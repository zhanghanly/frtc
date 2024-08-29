#ifndef FRTC_API_H
#define FRTC_API_H

#include <stdint.h>

typedef enum {
    FRTC_UNKNOWN_A,
    FRTC_PCMA,
    FRTC_PCMU,
    FRTC_AAC,
    FRTC_OPUS
} FrtcAudioCodec;

typedef enum {
    FRTC_UNKNOWN_V,
    FRTC_H264,
    FRTC_H265,
    FRTC_VP8,
    FRTC_VP9
} FrtcVideoCodec;

typedef struct {
    int32_t frameType;
    int64_t pts;
    int64_t dts;
    int32_t size;
    uint8_t* data;
} FrtcFrame;

typedef struct {
    FrtcAudioCodec codec;
    int32_t samplerate;
    int32_t channels;
    int32_t samplerateDepth;
} FrtcAudioParam;

typedef struct {
    FrtcVideoCodec codec;
    char extra[1024];
    int32_t extraLen;
} FrtcVideoParam;

typedef void (*OnRecieveVideoParam)(void*, FrtcVideoParam*);

typedef void (*OnRecieveAudioParam)(void*, FrtcAudioParam*);

typedef void (*OnRecieveVideoFrame)(void*, FrtcFrame*);

typedef void (*OnRecieveAudioFrame)(void*, FrtcFrame*);

typedef struct {
    void* user_data;
    OnRecieveVideoParam video_param_cb;
    OnRecieveAudioParam audio_param_cb;
    OnRecieveVideoFrame video_frame_cb;
    OnRecieveAudioFrame audio_frame_cb;
} FrtcStreamConfig;


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief: 创建frtc的上下文 
 * @return: frtc的上下文句柄
*/
void* frtcCreateCtx(void);

/**
 * @brief: 设置接收流回调 
 * @param ctx: frtc的上下文句柄 
 * @param config: 接收流的配置项 
*/
void frtcSetStreamConfig(void* ctx, FrtcStreamConfig* config);

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