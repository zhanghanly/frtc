#ifndef FRTC_RTP_DECODER_H
#define FRTC_RTP_DECODER_H

#include <functional>
#include "RtpPacket.h" 
#include "media/Frame.h"

namespace frtc {

class RtpDecoder {
public:
    virtual ~RtpDecoder() = default;

    virtual bool inputRtp(RtpPacket::Ptr, bool key_pos = true) = 0;

    virtual void outputFrame(FramePtr);

    void setRecieve(std::function<void(FramePtr)>);

private:
    std::function<void(FramePtr)> _reciever;
};

typedef std::shared_ptr<RtpDecoder> RtpDecoderSp;

RtpDecoderSp createRtpDecoder(CodecId);

}

#endif