#include <sys/time.h>
#include <cstring>
#include "Utility.h"

namespace frtc {

static char alphapt[] = {
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
    'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
    'u', 'v', 'w', 'x', 'y', 'z',
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
    'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
    'U', 'V', 'W', 'X', 'Y', 'Z',
    '0', '1', '2', '3', '4', '5' , '6' ,'7','8','9'
}; 

static char digit[] = {
    '0', '1', '2', '3', '4', '5' , '6' ,'7','8','9'
}; 

std::string generateRandDigit(int32_t len) {
    std::string randDigitStr;
    for (int i = 0; i < len; i++) {
        int index = rand() % 10;
        randDigitStr.push_back(digit[index]); 
    }

    return randDigitStr;
}

std::string generateRandStr(int32_t len) {
    std::string randStr;
    for (int i = 0; i < len; i++) {
        int index = rand() % 52;
        randStr.push_back(alphapt[index]);
    }

    return randStr;
}

std::string generateRandMixedStr(int32_t len) {
    std::string randMixedStr;
    for (int i = 0; i < len; i++) {
        int index = rand() % 62;
        randMixedStr.push_back(alphapt[index]);
    }

    return randMixedStr;
}

std::vector<std::string> splitStrWithSeparator(const std::string& src, const std::string& sep) {
    std::vector<std::string> result;
    std::size_t pos = src.find(sep);    
    std::size_t index = 0;

    while (pos != std::string::npos) {
        result.emplace_back(src.c_str() + index,  pos-index);
        index = pos + sep.size(); 
        pos = src.find(sep, pos + sep.size()); 
    }
    if (index < src.size()) {
        result.emplace_back(src.c_str() + index);
    }

    return result;
}

uint64_t getCurrentMillisecond() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);

    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}


const char* memfind(const char* buf, ssize_t len, const char* subbuf, ssize_t sublen) {
    for (auto i = 0; i < len - sublen; ++i) {
        if (memcmp(buf + i, subbuf, sublen) == 0) {
            return buf + i;
        }
    }
    
    return nullptr;
}

}