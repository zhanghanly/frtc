#ifndef FRTC_H264_H
#define FRTC_H264_H

#include "Frame.h"

#define H264_TYPE(v) ((uint8_t)(v) & 0x1F)

namespace frtc {

//void splitH264(const char *ptr, size_t len, size_t prefix, const std::function<void(const char *, size_t, size_t)> &cb);
//size_t prefixSize(const char *ptr, size_t len);

template<typename Parent>
class H264FrameHelper : public Parent {
public:
    friend class FrameImp;
    //friend class toolkit::ResourcePool_l<H264FrameHelper>;
    using Ptr = std::shared_ptr<H264FrameHelper>;

    enum {
        NAL_IDR = 5,
        NAL_SEI = 6,
        NAL_SPS = 7,
        NAL_PPS = 8,
        NAL_AUD = 9,
        NAL_B_P = 1,
    };

    template<typename ...ARGS>
    H264FrameHelper(ARGS &&...args): Parent(std::forward<ARGS>(args)...) {
        this->_codec_id = CodecId::h264;
    }

    bool keyFrame() override {
        auto nal_ptr = (uint8_t*) this->data() + this->prefix();
        return H264_TYPE(*nal_ptr) == NAL_IDR && decodeAble();
    }

    bool configFrame() override {
        auto nal_ptr = (uint8_t*) this->data() + this->prefix();
        switch (H264_TYPE(*nal_ptr)) {
            case NAL_SPS:
            case NAL_PPS: return true;
            default: return false;
        }
    }

    bool dropAble() override {
        auto nal_ptr = (uint8_t*) this->data() + this->prefix();
        switch (H264_TYPE(*nal_ptr)) {
            case NAL_SEI:
            case NAL_AUD: return true;
            default: return false;
        }
    }

    bool decodeAble() override {
        auto nal_ptr = (uint8_t*) this->data() + this->prefix();
        auto type = H264_TYPE(*nal_ptr);
        //多slice情况下, first_mb_in_slice 表示其为一帧的开始
        return type >= NAL_B_P && type <= NAL_IDR && (nal_ptr[1] & 0x80);
    }
};

/**
 * 264帧类
 */
using H264Frame = H264FrameHelper<FrameImp>;

/**
 * 防止内存拷贝的H264类
 * 用户可以通过该类型快速把一个指针无拷贝的包装成Frame类
 */
using H264FrameNoCacheAble = H264FrameHelper<FrameFromPtr>;

}

#endif
