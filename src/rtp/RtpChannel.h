#ifndef FRTC_RTP_CHANNEL_H
#define FRTC_RTP_CHANNEL_H

#include <functional>
#include "RtpTrack.h"
#include "rtcp/RtcpFCI.h"
#include "rtcp/RtcpContext.h"
#include "rtcp/RtcpNack.h"

namespace frtc {

class RtpChannel : public RtpTrackImp, public std::enable_shared_from_this<RtpChannel> {
public:
    RtpChannel(RtpTrackImp::OnSorted cb, std::function<void(const FCI_NACK& nack)> on_nack) {
        _on_nack = std::move(on_nack);
        setOnSorted(std::move(cb));
        //设置jitter buffer参数
        //GET_CONFIG(uint32_t, nack_maxms, Rtc::kNackMaxMS);
        uint32_t nack_maxms = 5000;
        RtpTrackImp::setParams(1024, nack_maxms, 512);
        _nack_ctx.setOnNack([this](const FCI_NACK& nack) { onNack(nack); });
    }

    RtpPacket::Ptr inputRtp(MediaType type, int sample_rate, uint8_t* ptr, size_t len, bool is_rtx) {
        auto rtp = RtpTrack::inputRtp(type, sample_rate, ptr, len);
        if (!rtp) {
            return rtp;
        }
        auto seq = rtp->getSeq();
        _nack_ctx.received(seq, is_rtx);
        if (!is_rtx) {
            // 统计rtp接受情况，便于生成nack rtcp包
            _rtcp_context.onRtp(seq, rtp->getStamp(), rtp->ntp_stamp, sample_rate, len);
        }
        return rtp;
    }

    Buffer::Ptr createRtcpRR(RtcpHeader* sr, uint32_t ssrc) {
        _rtcp_context.onRtcp(sr);
        return _rtcp_context.createRtcpRR(ssrc, getSSRC());
    }
    
    Buffer::Ptr createRtcpRR(uint32_t ssrc) {
        return _rtcp_context.createRtcpRR(ssrc, getSSRC());
    }

    float getLossRate() {
        auto expected = _rtcp_context.getExpectedPacketsInterval();
        if (!expected) {
            return -1;
        }
        return _rtcp_context.getLostInterval() * 100 / expected;
    }

private:
    void starNackTimer() {
        //if (_delay_task) {
        //    return;
        //}
        //std::weak_ptr<RtpChannel> weak_self = shared_from_this();
        //_delay_task = _poller->doDelayTask(10, [weak_self]() -> uint64_t {
        //    auto strong_self = weak_self.lock();
        //    if (!strong_self) {
        //        return 0;
        //    }
        //    auto ret = strong_self->_nack_ctx.reSendNack();
        //    if (!ret) {
        //        strong_self->_delay_task = nullptr;
        //    }
        //    return ret;
        //});
    }

    void onNack(const FCI_NACK& nack) {
        _on_nack(nack);
        starNackTimer();
    }

private:
    NackContext _nack_ctx;
    RtcpContextForRecv _rtcp_context;
    std::function<void(const FCI_NACK& nack)> _on_nack;
};

}

#endif