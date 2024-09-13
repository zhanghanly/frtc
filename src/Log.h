#ifndef FRTC_LOG_H
#define FRTC_LOG_H

#include <cstdio>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <sys/time.h>
#include <sys/types.h> //getpid, gettid
#include <string>
#include <thread>
#include <mutex>
#include <android/log.h>
#include <condition_variable>

namespace frtc {

enum LogLevel {
    FATAL = 1,
    ERROR,
    WARN,
    INFO,
    DEBUG,
    TRACE,
};

class utcTimer {
public:    
    utcTimer() {
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        //set _sys_acc_sec, _sys_acc_min
        _sys_acc_sec = tv.tv_sec;
        _sys_acc_min = _sys_acc_sec / 60;
        //use _sys_acc_sec calc year, mon, day, hour, min, sec
        struct tm cur_tm;
        localtime_r((time_t*)&_sys_acc_sec, &cur_tm);
        year = cur_tm.tm_year + 1900;
        mon  = cur_tm.tm_mon + 1;
        day  = cur_tm.tm_mday;
        hour = cur_tm.tm_hour;
        min  = cur_tm.tm_min;
        sec  = cur_tm.tm_sec;
        resetUtcFmt();
    }

    uint64_t getCurrTime(int* p_msec = nullptr) {
        struct timeval tv;
        //get current ts
        gettimeofday(&tv, nullptr);
        if (p_msec) {
            *p_msec = tv.tv_usec / 1000;
        }
        //if not in same seconds
        if ((uint32_t)tv.tv_sec != _sys_acc_sec) {
            sec = tv.tv_sec % 60;
            _sys_acc_sec = tv.tv_sec;
            //or if not in same minutes
            if (_sys_acc_sec / 60 != _sys_acc_min) {
                //use _sys_acc_sec update year, mon, day, hour, min, sec
                _sys_acc_min = _sys_acc_sec / 60;
                struct tm cur_tm;
                localtime_r((time_t*)&_sys_acc_sec, &cur_tm);
                year = cur_tm.tm_year + 1900;
                mon  = cur_tm.tm_mon + 1;
                day  = cur_tm.tm_mday;
                hour = cur_tm.tm_hour;
                min  = cur_tm.tm_min;
                //reformat utc format
                resetUtcFmt();
            } else {
                //reformat utc format only sec
                resetUtcFmtSec();
            }
        }
        return tv.tv_sec;
    }

    int year, mon, day, hour, min, sec;
    char utc_fmt[20];

private:
    void resetUtcFmt() {
        snprintf(utc_fmt, 20, "%d-%02d-%02d %02d:%02d:%02d", year, mon, day, hour, min, sec);
    }
    
    void resetUtcFmtSec() {
        snprintf(utc_fmt + 17, 3, "%02d", sec);
    }

    uint64_t _sys_acc_min;
    uint64_t _sys_acc_sec;
};

class LogBuffer {
public:
    enum status {
        FREE,
        FULL
    };

    LogBuffer(uint32_t len):
    _status(FREE),
    _prev(nullptr),
    _next(nullptr),
    _totalLen(len),
    _usedLen(0) {
        _data = new char[len];
        if (!_data) {
            fprintf(stderr, "no space to allocate _data\n");
            exit(1);
        }
    }

    uint32_t availLen() const { 
        return _totalLen - _usedLen;
    }

    bool empty() const { 
        return _usedLen == 0;
    }

    void append(const char* log_line, uint32_t len) {
        if (availLen() < len) {
            return;
        }
        memcpy(_data + _usedLen, log_line, len);
        _usedLen += len;
    }

    void clear() {
        _usedLen = 0;
        _status = FREE;
    }

    void persist(FILE* fp) {
        uint32_t wt_len = fwrite(_data, 1, _usedLen, fp);
        if (wt_len != _usedLen) {
            fprintf(stderr, "write log to disk error, wt_len %u\n", wt_len);
        }
    }

private:
    LogBuffer(const LogBuffer&);
    LogBuffer& operator=(const LogBuffer&);

    uint32_t _totalLen;
    uint32_t _usedLen;
    char* _data;
    
public:
    status _status;
    LogBuffer* _prev;
    LogBuffer* _next;
};

class Log {
public:
    //for thread-safe singleton
    static Log* instance() {
        static Log log;
        return &log;
    }

    ~Log();

    void initialize(const char* log_dir, const char* prog_name, int level);

    int getLevel() const { 
        return _level; 
    }

    void persist();

    void append(const char* lvl, const char* format, ...);

private:
    Log();

    bool decisFile(int year, int mon, int day);

    Log(const Log&);

    const Log& operator=(const Log&);

    int _buffCnt;

    LogBuffer* _currBuf;
    LogBuffer* _prstBuf;
    LogBuffer* lastBuf;

    FILE* _fp;
    pid_t _pid;
    int _year, _mon, _day, _logCnt;
    std::string _progName;
    std::string _logDir;

    bool _envOk;       //if log dir ok
    int _level;
    uint64_t _lst_lts; //last can't log error time(s)
    
    utcTimer _tm;

    std::mutex _mutex;
    std::thread _thread;
    std::condition_variable _cond;
    static uint32_t _oneBuffLen;
    //singleton
    static Log* _instance;
};

pid_t gettid();

}

#define LOG_INIT(log_dir, prog_name, level) \
    do \
    { \
        frtc::Log::instance()->initialize(log_dir, prog_name, level); \
    } while (0)

//format: [LEVEL][yy-mm-dd h:m:s.ms][tid]file_name:line_no(func_name):content
#define LOG_TRACE(fmt, args...) \
    do \
    { \
        if (frtc::Log::instance()->getLevel() >= frtc::TRACE) \
        { \
            frtc::Log::instance()->append("[TRACE]", "[%u]%s:%d(%s): " fmt "\n", \
                    frtc::gettid(), __FILE__, __LINE__, __FUNCTION__, ##args); \
        } \
    } while (0)

#define LOG_DEBUG(fmt, args...) \
    do \
    { \
        if (frtc::Log::instance()->getLevel() >= frtc::DEBUG) \
        { \
            frtc::Log::instance()->append("[DEBUG]", "[%u]%s:%d(%s): " fmt "\n", \
                    frtc::gettid(), __FILE__, __LINE__, __FUNCTION__, ##args); \
        } \
    } while (0)

#define LOG_INFO(fmt, args...) \
    do \
    { \
        if (frtc::Log::instance()->getLevel() >= frtc::INFO) \
        { \
            frtc::Log::instance()->append("[INFO]", "[%u]%s:%d(%s): " fmt "\n", \
                    frtc::gettid(), __FILE__, __LINE__, __FUNCTION__, ##args); \
        } \
    } while (0)

#define LOG_NORMAL(fmt, args...) \
    do \
    { \
        if (frtc::Log::instance()->getLevel() >= frtc::INFO) \
        { \
            frtc::Log::instance()->append("[INFO]", "[%u]%s:%d(%s): " fmt "\n", \
                    frtc::gettid(), __FILE__, __LINE__, __FUNCTION__, ##args); \
        } \
    } while (0)

#define LOG_WARN(fmt, args...) \
    do \
    { \
        if (frtc::Log::instance()->getLevel() >= frtc::WARN) \
        { \
            frtc::Log::instance()->append("[WARN]", "[%u]%s:%d(%s): " fmt "\n", \
                    frtc::gettid(), __FILE__, __LINE__, __FUNCTION__, ##args); \
        } \
    } while (0)

#define LOG_ERROR(fmt, args...) \
    do \
    { \
        if (frtc::Log::instance()->getLevel() >= frtc::ERROR) \
        { \
            frtc::Log::instance()->append("[ERROR]", "[%u]%s:%d(%s): " fmt "\n", \
                frtc::gettid(), __FILE__, __LINE__, __FUNCTION__, ##args); \
        } \
    } while (0)

#define LOG_FATAL(fmt, args...) \
    do \
    { \
        frtc::Log::instance()->append("[FATAL]", "[%u]%s:%d(%s): " fmt "\n", \
            frtc::gettid(), __FILE__, __LINE__, __FUNCTION__, ##args); \
    } while (0)



#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, "frtc", __VA_ARGS__)

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , "frtc", __VA_ARGS__)

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO   , "frtc", __VA_ARGS__)

#define LOGW(...) __android_log_print(ANDROID_LOG_WARN   , "frtc", __VA_ARGS__)

#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , "frtc", __VA_ARGS__)

#endif