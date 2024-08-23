#ifndef FRTC_H264_RTP_H
#define FRTC_H264_RTP_H

#include <memory>
#include "RtpPacket.h"
#include "Frame.h"
#include "H264.h"
#include "Stamp.h"
#include "RtpDecoder.h"

namespace frtc {

/**
 * h264 rtp解码类
 * 将 h264 over rtsp-rtp 解复用出 h264-Frame
 * rfc3984
 */
class H264RtpDecoder : public RtpDecoder {
public:
    using Ptr = std::shared_ptr<H264RtpDecoder>;
    
    ~H264RtpDecoder() override = default;

    H264RtpDecoder();

    /**
     * 输入264 rtp包
     * @param rtp rtp包
     * @param key_pos 此参数忽略之
     */
    bool inputRtp(RtpPacket::Ptr rtp, bool key_pos = true);

private:
    bool singleFrame(const RtpPacket::Ptr &rtp, const uint8_t *ptr, ssize_t size, uint64_t stamp);
    bool unpackStapA(const RtpPacket::Ptr &rtp, const uint8_t *ptr, ssize_t size, uint64_t stamp);
    bool mergeFu(const RtpPacket::Ptr &rtp, const uint8_t *ptr, ssize_t size, uint64_t stamp, uint16_t seq);

    bool decodeRtp(const RtpPacket::Ptr &rtp);
    H264Frame::Ptr obtainFrame();
    void outputFrame(const RtpPacket::Ptr &rtp, const H264Frame::Ptr &frame);

private:
    bool _is_gop = false;
    bool _gop_dropped = false;
    bool _fu_dropped = true;
    uint16_t _last_seq = 0;
    H264Frame::Ptr _frame;
    DtsGenerator _dts_generator;
};

}

#endif
