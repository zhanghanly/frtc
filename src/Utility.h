#ifndef FRTC_UTILITY_H
#define FRTC_UTILITY_H

#include <string>
#include <vector>

namespace frtc {

/**
 * @brief: 生成随机的数字字符串 
 * @param callback: 要生成字符串的长度 
 * @return: 生成的随机数字字符串
*/
std::string generateRandDigit(int32_t len);

/**
 * @brief: 生成随机的字符字符串 
 * @param len: 要生成字符串的长度
 * @return: 生成的随机字符字符串
*/
std::string generateRandStr(int32_t len);

/**
 * @brief: 生成随机的数字和字符混合字符串 
 * @param len: 要生成字符串的长度
 * @return: 生成的随机数字和字符混合字符串
*/
std::string generateRandMixedStr(int32_t len);

/**
 * @brief: 用给定的字符串切割字符串 
 * @param len: 要被切割的字符串
 * @param len: 分割字符串
 * @return: 切割后的单词 
*/
std::vector<std::string> splitStrWithSeparator(const std::string& src, const std::string& sep);

/**
 * @return: 返回当前系统时间戳(单位：毫秒) 
*/
uint64_t getCurrentMillisecond();

const char* memfind(const char* buf, ssize_t len, const char* subbuf, ssize_t sublen);

}

#endif