#ifndef FRTC_H265_RTP_H
#define FRTC_H265_RTP_H

#include <memory>
#include "H265.h"
#include "Stamp.h"
#include "RtpPacket.h"
#include "Frame.h"
#include "RtpDecoder.h"

namespace frtc {

/**
 * h265 rtp解码类
 * 将 h265 over rtsp-rtp 解复用出 h265-Frame
 * 《草案（H265-over-RTP）draft-ietf-payload-rtp-h265-07.pdf》
 */
class H265RtpDecoder : public RtpDecoder {
public:
    using Ptr = std::shared_ptr<H265RtpDecoder>;

    ~H265RtpDecoder() override = default;
    
    H265RtpDecoder();

    /**
     * 输入265 rtp包
     * @param rtp rtp包
     * @param key_pos 此参数忽略之
     */
    bool inputRtp(RtpPacket::Ptr rtp, bool key_pos = true) override;

private:
    bool unpackAp(const RtpPacket::Ptr& rtp, const uint8_t* ptr, ssize_t size, uint64_t stamp);
    bool mergeFu(const RtpPacket::Ptr& rtp, const uint8_t* ptr, ssize_t size, uint64_t stamp, uint16_t seq);
    bool singleFrame(const RtpPacket::Ptr& rtp, const uint8_t* ptr, ssize_t size, uint64_t stamp);

    bool decodeRtp(const RtpPacket::Ptr& rtp);
    H265Frame::Ptr obtainFrame();
    void outputFrame(const RtpPacket::Ptr& rtp, const H265Frame::Ptr& frame);

private:
    bool _is_gop = false;
    bool _using_donl_field = false;
    bool _gop_dropped = false;
    bool _fu_dropped = true;
    uint16_t _last_seq = 0;
    H265Frame::Ptr _frame;
    DtsGenerator _dts_generator;
};

}

#endif
