#include "RtpDecoder.h"
#include "H264Rtp.h"
#include "H265Rtp.h"
#include "G711Rtp.h"

namespace frtc {

void RtpDecoder::outputFrame(FramePtr frame) {
    if (frame && _reciever) {
        _reciever(frame);
    }
}
    
void RtpDecoder::setRecieve(std::function<void(FramePtr)> reciever) {
    if (reciever) {
        _reciever = reciever;
    }
}

RtpDecoderSp createRtpDecoder(CodecId id) {
    switch (id) {
    case CodecId::h264:
        return std::make_shared<H264RtpDecoder>();
    
    case CodecId::h265:
        return std::make_shared<H265RtpDecoder>();

    case CodecId::g711a:
    case CodecId::g711u:
        return std::make_shared<G711RtpDecoder>(id); 

    default:
        return nullptr;
    }
}

}