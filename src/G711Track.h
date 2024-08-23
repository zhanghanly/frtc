#ifndef FRTC_G711A_TRACK_H
#define FRTC_G711A_TRACK_H

#include "Track.h"

namespace frtc {

class G711Track : public AudioTrack {
public:
    ~G711Track() override = default;
    
    /**
    * @param codecId 编码类型
    * @param sample_rate 采样率(HZ)
    * @param channels 通道数
    * @param sample_bit 采样位数，一般为16
    */
    G711Track(CodecId codecId, int sample_rate, int channels, int sampleBytes) {
        _codecId = codecId;
        _sampleRate = sample_rate;
        _channels = channels;
        _sampleBytes = sampleBytes;
    }

    /**
     * 返回编码类型
     */
    CodecId codecId() override {
        return _codecId;
    }

    /**
     * 是否已经初始化
     */
    bool ready() override {
        return true;
    }

    /**
     * 返回音频采样率
     */
    int sampleRate() override {
        return _sampleRate;
    }

    /**
     * 返回音频采样位数，一般为16或8
     */
    int sampleBytes() override {
        return _sampleBytes;
    }

    /**
     * 返回音频通道数
     */
    int channels() override {
        return _channels;
    }

private:
    CodecId _codecId;
    int _sampleRate;
    int _channels;
    int _sampleBytes;
};

}

#endif