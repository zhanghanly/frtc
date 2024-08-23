#include <iostream>
#include "H265Track.h"

namespace frtc {

enum class H265FrameType {
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

H265FrameType getH265FrameType(char nalu) {
    return (H265FrameType)(((uint8_t)(nalu) >> 1) & 0x3F);
}

CodecId H265Track::codecId() {
    return CodecId::h265;
}

bool H265Track::ready() {
    return !_sps.empty() && !_pps.empty() && !_vps.empty();
}

void H265Track::inputFrame(FramePtr frame) {
    std::cout << "h265 track input frame" << std::endl;
    H265FrameType type = getH265FrameType(frame->data()[frame->prefix()]);
    if (!frame->configFrame() && type != H265FrameType::NAL_SEI_PREFIX) {
        if (ready()) {
            inputFrame_1(frame);
        } else {
            return;
        }
    }

    inputFrame_1(frame);
}

int32_t H265Track::height() {
    return _height;
}

int32_t H265Track::width() {
    return _width;
}

int32_t H265Track::fps() {
    return _fps;
}

void H265Track::inputFrame_1(FramePtr frame) {
    if (frame->keyFrame()) {
        insertConfigFrame(frame);
        return Track::inputFrame(frame);
    }
    //非idr帧
    switch (getH265FrameType(frame->data()[frame->prefix()])) {
        case H265FrameType::NAL_VPS: {
            _vps = std::string(frame->data() + frame->prefix(), frame->size() - frame->prefix());
            break;
        }
        case H265FrameType::NAL_SPS: {
            _sps = std::string(frame->data() + frame->prefix(), frame->size() - frame->prefix());
            break;
        }
        case H265FrameType::NAL_PPS: {
            _pps = std::string(frame->data() + frame->prefix(), frame->size() - frame->prefix());
            break;
        }
        default: {
            Track::inputFrame(frame);
            break;
        }
    }
}

void H265Track::insertConfigFrame(FramePtr frame) {
    if (!_vps.empty()) {
        FrameImpPtr vpsFrame = std::make_shared<FrameImp>();
        vpsFrame->_isConfig = true;
        vpsFrame->_pts = frame->pts();
        vpsFrame->_prefix_size = 4;
        vpsFrame->_buffer->append(0x00); 
        vpsFrame->_buffer->append(0x00); 
        vpsFrame->_buffer->append(0x00); 
        vpsFrame->_buffer->append(0x01); 
        vpsFrame->_buffer->append(_vps.c_str(), _vps.size());
        Track::inputFrame(vpsFrame);
    }
    if (!_sps.empty()) {
        FrameImpPtr spsFrame = std::make_shared<FrameImp>();
        spsFrame->_isConfig = true;
        spsFrame->_pts = frame->pts();
        spsFrame->_prefix_size = 4;
        spsFrame->_buffer->append(0x00); 
        spsFrame->_buffer->append(0x00); 
        spsFrame->_buffer->append(0x00); 
        spsFrame->_buffer->append(0x01); 
        spsFrame->_buffer->append(_sps.c_str(), _sps.size());
        Track::inputFrame(spsFrame);
    }
    if (!_pps.empty()) {
        FrameImpPtr ppsFrame = std::make_shared<FrameImp>();
        ppsFrame->_isConfig = true;
        ppsFrame->_pts = frame->pts();
        ppsFrame->_prefix_size = 4;
        ppsFrame->_buffer->append(0x00); 
        ppsFrame->_buffer->append(0x00); 
        ppsFrame->_buffer->append(0x00); 
        ppsFrame->_buffer->append(0x01); 
        ppsFrame->_buffer->append(_pps.c_str(), _pps.size());
        Track::inputFrame(ppsFrame);
    }
}

}
