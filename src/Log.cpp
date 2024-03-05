#include <errno.h>
#include <unistd.h>     //access, getpid
#include <assert.h>     //assert
#include <stdarg.h>     //va_list
#include <sys/stat.h>   //mkdir
#include <sys/syscall.h>//system call
#include "Log.h"

namespace frtc {

#define MEM_USE_LIMIT (3u * 1024 * 1024 * 1024) //3GB
#define LOG_USE_LIMIT (1u * 1024 * 1024 * 1024) //1GB
#define LOG_LEN_LIMIT (4 * 1024) //4K
#define RELOG_THRESOLD 5
#define BUFF_WAIT_TIME 1

pid_t gettid() {
    return syscall(__NR_gettid);
}

Log* Log::_instance = nullptr;
uint32_t Log::_oneBuffLen = 30*1024*1024;//30MB

Log::Log():
    _buffCnt(3),
    _currBuf(nullptr),
    _prstBuf(nullptr),
    _fp(nullptr),
    _logCnt(0),
    _envOk(false),
    _level(INFO),
    _lst_lts(0),
    _tm() {
    //create double linked list
    LogBuffer* head = new LogBuffer(_oneBuffLen);
    if (!head) {
        fprintf(stderr, "no space to allocate cell_buffer\n");
        exit(1);
    }
    
    LogBuffer* current;
    LogBuffer* prev = head;
    for (int i = 1;i < _buffCnt; ++i) {
        current = new LogBuffer(_oneBuffLen);
        if (!current) {
            fprintf(stderr, "no space to allocate cell_buffer\n");
            exit(1);
        }
        current->_prev = prev;
        prev->_next = current;
        prev = current;
    }
    prev->_next = head;
    head->_prev = prev;

    _currBuf = head;
    _prstBuf = head;

    _pid = getpid();
}
    
Log::~Log() {
    if (_thread.joinable()) {
        _thread.join();
    }
}

void Log::initialize(const char* log_dir, const char* prog_name, int level) {
    _logDir = log_dir;
    _progName = prog_name;

    mkdir(_logDir.c_str(), 0777);
    //查看是否存在此目录、目录下是否允许创建文件
    if (access(_logDir.c_str(), F_OK | W_OK) == -1) {
        fprintf(stderr, "logdir: %s error: %s\n", _logDir.c_str(), strerror(errno));
    } else {
        _envOk = true;
    }
    if (level > TRACE) {
        level = TRACE;
    }
    if (level < FATAL) {
        level = FATAL;
    }
    _level = level;

    _thread = std::thread(&Log::persist, this);
}

void Log::persist() {
    while (true) {
        std::unique_lock<std::mutex> ul(_mutex);
        while (_prstBuf->empty()) {
            if (_cond.wait_for(ul, std::chrono::seconds(1)) == std::cv_status::timeout) {
                continue;
            }
        }
        if (_prstBuf->_status == LogBuffer::FREE) {
            assert(_currBuf == _prstBuf);  //to test
            _currBuf->_status = LogBuffer::FULL;
            _currBuf = _currBuf->_next;
        }
        ul.unlock();

        int year = _tm.year;
        int mon = _tm.mon;
        int day = _tm.day;
        //decision which file to write
        if (!decisFile(year, mon, day)) {
            continue;
        }
        //write
        _prstBuf->persist(_fp);
        fflush(_fp);
        _prstBuf->clear();
        _prstBuf = _prstBuf->_next;
    }
}

void Log::append(const char* lvl, const char* format, ...) {
    int ms;
    uint64_t curr_sec = _tm.getCurrTime(&ms);
    if (_lst_lts && curr_sec - _lst_lts < RELOG_THRESOLD) {
        return;
    }

    char log_line[LOG_LEN_LIMIT];
    //int prev_len = snprintf(log_line, LOG_LEN_LIMIT, "%s[%d-%02d-%02d %02d:%02d:%02d.%03d]", lvl, _tm.year, _tm.mon, _tm.day, _tm.hour, _tm.min, _tm.sec, ms);
    int prev_len = snprintf(log_line, LOG_LEN_LIMIT, "%s[%s.%03d]", lvl, _tm.utc_fmt, ms);

    va_list arg_ptr;
    va_start(arg_ptr, format);

    //TO OPTIMIZE IN THE FUTURE: performance too low here!
    int main_len = vsnprintf(log_line + prev_len, LOG_LEN_LIMIT - prev_len, format, arg_ptr);

    va_end(arg_ptr);

    uint32_t len = prev_len + main_len;

    _lst_lts = 0;
    bool tell_back = false;

    std::lock_guard<std::mutex> lg(_mutex);
    if (_currBuf->_status == LogBuffer::FREE && _currBuf->availLen() >= len) {
        _currBuf->append(log_line, len);
    } else {
        if (_currBuf->_status == LogBuffer::FREE) {
            _currBuf->_status = LogBuffer::FULL;//set to FULL
            LogBuffer* next_buf = _currBuf->_next;
            //tell backend thread
             tell_back = true;

            if (next_buf->_status == LogBuffer::FULL) {
                //if mem use < MEM_USE_LIMIT, allocate new cell_buffer
                if (_oneBuffLen * (_buffCnt + 1) > MEM_USE_LIMIT) {
                    fprintf(stderr, "no more log space can use\n");
                    _currBuf = next_buf;
                    _lst_lts = curr_sec;
                } else {
                    LogBuffer* new_buffer = new LogBuffer(_oneBuffLen);
                    _buffCnt += 1;
                    new_buffer->_prev = _currBuf;
                    _currBuf->_next = new_buffer;
                    new_buffer->_next = next_buf;
                    next_buf->_prev = new_buffer;
                    _currBuf = new_buffer;
                }
            } else {
                //next buffer is free, we can use it
                _currBuf = next_buf;
            } if (!_lst_lts) {
                _currBuf->append(log_line, len);
            }
        } else { //_curr_buf->status == cell_buffer::FULL, assert persist is on here too!
            _lst_lts = curr_sec;
        }
    }
    if (tell_back) {
        _cond.notify_one();
    }
}

bool Log::decisFile(int year, int mon, int day) {
    //TODO: 是根据日志消息的时间写时间？还是自主写时间？  I select 自主写时间
    if (!_envOk) {
        if (_fp) {
            fclose(_fp);
        }
        _fp = fopen("/dev/null", "w");
        return _fp != nullptr;
    }
    if (!_fp) {
        _year = year, _mon = mon, _day = day;
        char log_path[1024] = {};
        sprintf(log_path, "%s/%s.%d%02d%02d.%u.log", _logDir.c_str(), _progName.c_str(), _year, _mon, _day, _pid);
        _fp = fopen(log_path, "w");
        if (_fp) {
            _logCnt += 1;
        }
    } else if (_day != day) {
        fclose(_fp);
        char log_path[1024] = {};
        _year = year, _mon = mon, _day = day;
        sprintf(log_path, "%s/%s.%d%02d%02d.%u.log", _logDir.c_str(), _progName.c_str(), _year, _mon, _day, _pid);
        _fp = fopen(log_path, "w");
        if (_fp) {
            _logCnt = 1;
        }
    } else if (ftell(_fp) >= LOG_USE_LIMIT) {
        fclose(_fp);
        char old_path[1024] = {};
        char new_path[1024] = {};
        //mv xxx.log.[i] xxx.log.[i + 1]
        for (int i = _logCnt - 1;i > 0; --i) {
            sprintf(old_path, "%s/%s.%d%02d%02d.%u.log.%d", _logDir.c_str(), _progName.c_str(), _year, _mon, _day, _pid, i);
            sprintf(new_path, "%s/%s.%d%02d%02d.%u.log.%d", _logDir.c_str(), _progName.c_str(), _year, _mon, _day, _pid, i + 1);
            rename(old_path, new_path);
        }
        //mv xxx.log xxx.log.1
        sprintf(old_path, "%s/%s.%d%02d%02d.%u.log", _logDir.c_str(), _progName.c_str(), _year, _mon, _day, _pid);
        sprintf(new_path, "%s/%s.%d%02d%02d.%u.log.1", _logDir.c_str(), _progName.c_str(), _year, _mon, _day, _pid);
        rename(old_path, new_path);
        _fp = fopen(old_path, "w");
        if (_fp) {
            _logCnt += 1;
        }
    }
    return _fp != nullptr;
}

}