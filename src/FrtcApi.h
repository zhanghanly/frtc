#ifndef FRTC_API_H
#define FRTC_API_H

#include <stdint.h>

enum FrtcAudioCodec {
    FRTC_UNKNOWN_A,
    FRTC_PCMA,
    FRTC_PCMU,
    FRTC_AAC,
    FRTC_OPUS
};

enum FrtcVideoCodec {
    FRTC_UNKNOWN_V,
    FRTC_H264,
    FRTC_H265,
    FRTC_VP8,
    FRTC_VP9
};

typedef struct {
    FrtcAudioCodec codec;
    int32_t samplerate;
    int32_t channels;
    int32_t samplerateDepth;
    int64_t pts;
    int32_t size;
    uint8_t* data;
} FrtcAudioFrame;

typedef struct {
    FrtcVideoCodec codec;
    int32_t frameType;
    int64_t pts;
    int64_t dts;
    int32_t size;
    uint8_t* data;
} FrtcVideoFrame;


typedef struct {






    /* data */
} FrtcAudioConfig;

typedef void (*OnRecieveVideoFrame)(void*, FrtcVideoFrame);
typedef void (*OnRecieveAudioFrame)(void*, FrtcAudioFrame);


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief: 创建frtc的上下文 
 * @return: frtc的上下文句柄
*/
void* frtcCreateCtx(void);

/**
 * @brief: 设置接收音频帧的回调函数 
 * @param callback: 接收音频帧的回调函数 
*/
void frtcSetAudioCallback(OnRecieveAudioFrame callback);

/**
 * @brief: 设置接收视频帧的回调函数 
 * @param callback: 接收视频帧的回调函数 
*/
void frtcSetVideoCallback(OnRecieveVideoFrame callback);

/**
 * @brief: 连接信令服务器 
 * @param ctx: frtc的上下文句柄 
 * @param url: 信令服务器的地址
 * @return: 大于0表示是空包，等于0表示不是空包，小于0表示出错 
*/
int frtcConnectSignalServer(void* ctx, const char* url);

/**
 * @brief: 将错误码转化为对应的字符串描述 
 * @param code: 错误码 
 * @param buffer: 写入字符串的buffer
 * @param len: buffer的大小
*/
void frtcCodeToString(int32_t code, char* buffer, int32_t len);

/**
 * @brief: 释放frtc上下文句柄 
 * @param ctx: frtc的上下文句柄 
*/
void frtcDestoryCtx(void* ctx);

#ifdef __cplusplus
}
#endif

#endif