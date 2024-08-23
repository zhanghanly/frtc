#ifndef FRTC_H265_H
#define FRTC_H265_H

#include "Frame.h"

#define H265_TYPE(v) (((uint8_t)(v) >> 1) & 0x3f)

namespace frtc {

template<typename Parent>
class H265FrameHelper : public Parent {
public:
    friend class FrameImp;
    //friend class toolkit::ResourcePool_l<H265FrameHelper>;
    using Ptr = std::shared_ptr<H265FrameHelper>;

    enum {
        NAL_TRAIL_N = 0,
        NAL_TRAIL_R = 1,
        NAL_TSA_N = 2,
        NAL_TSA_R = 3,
        NAL_STSA_N = 4,
        NAL_STSA_R = 5,
        NAL_RADL_N = 6,
        NAL_RADL_R = 7,
        NAL_RASL_N = 8,
        NAL_RASL_R = 9,
        NAL_BLA_W_LP = 16,
        NAL_BLA_W_RADL = 17,
        NAL_BLA_N_LP = 18,
        NAL_IDR_W_RADL = 19,
        NAL_IDR_N_LP = 20,
        NAL_CRA_NUT = 21,
        NAL_RSV_IRAP_VCL22 = 22,
        NAL_RSV_IRAP_VCL23 = 23,

        NAL_VPS = 32,
        NAL_SPS = 33,
        NAL_PPS = 34,
        NAL_AUD = 35,
        NAL_EOS_NUT = 36,
        NAL_EOB_NUT = 37,
        NAL_FD_NUT = 38,
        NAL_SEI_PREFIX = 39,
        NAL_SEI_SUFFIX = 40,
    };

    template<typename ...ARGS>
    H265FrameHelper(ARGS &&...args): Parent(std::forward<ARGS>(args)...) {
        this->_codec_id = CodecId::h265;
    }

    bool keyFrame() override {
        auto nal_ptr = (uint8_t*) this->data() + this->prefix();
        auto type = H265_TYPE(*nal_ptr);
        // 参考自FFmpeg: IRAP VCL NAL unit types span the range
        // [BLA_W_LP (16), RSV_IRAP_VCL23 (23)].
        return (type >= NAL_BLA_W_LP && type <= NAL_RSV_IRAP_VCL23) && decodeAble() ;
    }

    bool configFrame() override {
        auto nal_ptr = (uint8_t*) this->data() + this->prefix();
        switch (H265_TYPE(*nal_ptr)) {
            case NAL_VPS:
            case NAL_SPS:
            case NAL_PPS : return true;
            default : return false;
        }
    }

    bool dropAble() override {
        auto nal_ptr = (uint8_t*) this->data() + this->prefix();
        switch (H265_TYPE(*nal_ptr)) {
            case NAL_AUD:
            case NAL_SEI_SUFFIX:
            case NAL_SEI_PREFIX: return true;
            default: return false;
        }
    }

    bool decodeAble() override {
        auto nal_ptr = (uint8_t*) this->data() + this->prefix();
        auto type = H265_TYPE(*nal_ptr);
        //多slice情况下, first_slice_segment_in_pic_flag 表示其为一帧的开始
        return type >= NAL_TRAIL_N && type <= NAL_RSV_IRAP_VCL23 && (nal_ptr[2] & 0x80);
    }
};

/**
 * 265帧类
 */
using H265Frame = H265FrameHelper<FrameImp>;

/**
 * 防止内存拷贝的H265类
 * 用户可以通过该类型快速把一个指针无拷贝的包装成Frame类
 */
using H265FrameNoCacheAble = H265FrameHelper<FrameFromPtr>;

}

#endif