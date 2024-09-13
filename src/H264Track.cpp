#include <iostream>
#include "H264Track.h"
#include "Log.h"

namespace frtc {

enum class H264FrameType {
    NAL_IDR = 5,
    NAL_SEI = 6,
    NAL_SPS = 7,
    NAL_PPS = 8,
    NAL_AUD = 9,
    NAL_B_P = 1
};

H264FrameType getH264FrameType(char nalu) {
    return (H264FrameType)((uint8_t)(nalu) & 0x1F);
}

CodecId H264Track::codecId() {
    return CodecId::h264;
}
    
bool H264Track::ready() {
    return !_sps.empty() && !_pps.empty();
}

void H264Track::inputFrame(FramePtr frame) {
    LOGI("%s", "h264 track input frame");
    H264FrameType type = getH264FrameType(frame->data()[frame->prefix()]);
    if (type == H264FrameType::NAL_B_P || type == H264FrameType::NAL_IDR) {
        if (ready()) {
            inputFrame_1(frame);
        } 
    } else {
        inputFrame_1(frame);
    }
}

void H264Track::inputFrame_1(FramePtr frame) {
    H264FrameType type = getH264FrameType(frame->data()[frame->prefix()]);
    switch (type) {
        case H264FrameType::NAL_SPS: {
            _sps = std::string(frame->data() + frame->prefix(), frame->size() - frame->prefix());
            //Track::inputFrame(frame);
            LOGI("%s", "h264 track recieve sps frame");
            break;
        }
        case H264FrameType::NAL_PPS: {
            _pps = std::string(frame->data() + frame->prefix(), frame->size() - frame->prefix());
            //Track::inputFrame(frame);
            LOGI("%s", "h264 track recieve pps frame");
            break;
        }
        default: {
            // 判断是否是I帧, 并且如果是,那判断前面是否插入过config帧, 如果插入过就不插入了
            if (frame->keyFrame()) {
                LOGI("%s", "h264 track recieve key frame");
                insertConfigFrame(frame);
            } else {
                LOGI("%s", "h264 track recieve p frame");
            }
            Track::inputFrame(frame);
        }
    }
}

void H264Track::insertConfigFrame(FramePtr frame) {
    if (!_sps.empty()) {
        FrameImpPtr spsFrame = std::make_shared<FrameImp>();
        spsFrame->_type = MediaType::video;
        spsFrame->_isConfig = true;
        spsFrame->_isKey = false;
        spsFrame->_pts = frame->pts();
        spsFrame->_prefix_size = 4;
        spsFrame->_buffer->append(0x00); 
        spsFrame->_buffer->append(0x00); 
        spsFrame->_buffer->append(0x00); 
        spsFrame->_buffer->append(0x01); 
        spsFrame->_buffer->append(_sps.c_str(), _sps.size());
        LOGI("%s", "out put sps frame");
        Track::inputFrame(spsFrame);
    }
    if (!_pps.empty()) {
        FrameImpPtr ppsFrame = std::make_shared<FrameImp>();
        ppsFrame->_type = MediaType::video;
        ppsFrame->_isConfig = true;
        ppsFrame->_isKey = false;
        ppsFrame->_pts = frame->pts();
        ppsFrame->_prefix_size = 4;
        ppsFrame->_buffer->append(0x00); 
        ppsFrame->_buffer->append(0x00); 
        ppsFrame->_buffer->append(0x00); 
        ppsFrame->_buffer->append(0x01); 
        ppsFrame->_buffer->append(_pps.c_str(), _pps.size());
        LOGI("%s", "out put pps frame");
        Track::inputFrame(ppsFrame);
    }
}

int32_t H264Track::height() {
    return _height;
}

int32_t H264Track::width() {
    return _width;
}

int32_t H264Track::fps() {
    return _fps;
}

}