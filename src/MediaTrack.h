#ifndef FRTC_MEDIA_TRACK_H
#define FRTC_MEDIA_TRACK_H

#include <memory>
#include <cstdint>
#include <string>
#include <unordered_map>
#include "TwccContext.h"
#include "rtp/RtpChannel.h"
#include "rtp/RtpExt.h"
#include "rtcp/RtcpNack.h"
#include "Sdp.h"

namespace frtc {

class MediaTrack {
public:
    using Ptr = std::shared_ptr<MediaTrack>;
    MediaType track_type;
    //const RtcCodecPlan* plan_rtp;
    //const RtcCodecPlan* plan_rtx;
    MediaPayloadPtr rtp_payload;
    MediaPayloadPtr rtx_payload;
    uint32_t offer_rtp_ssrc = 0;
    uint32_t offer_rtx_ssrc = 0;
    uint32_t answer_rtp_ssrc = 0;
    uint32_t answer_rtx_ssrc = 0;
    //Const RtcMedia* media;
    RtpExtContext::Ptr rtp_ext_ctx;
    //for send rtp
    NackList nack_list;
    RtcpContext::Ptr rtcp_context_send;
    //for recv rtp
    std::unordered_map<std::string/*rid*/, std::shared_ptr<RtpChannel>> rtp_channel;
    std::shared_ptr<RtpChannel> getRtpChannel(uint32_t ssrc) const;
};

struct WrappedMediaTrack {
    MediaTrack::Ptr track;
    explicit WrappedMediaTrack(MediaTrack::Ptr ptr): track(ptr) {}
    virtual ~WrappedMediaTrack() {}
    virtual void inputRtp(const char* buf, size_t len, uint64_t stamp_ms, RtpHeader* rtp) = 0;
};

struct WrappedRtxTrack: public WrappedMediaTrack {
    explicit WrappedRtxTrack(MediaTrack::Ptr ptr)
        : WrappedMediaTrack(std::move(ptr)) {}
    void inputRtp(const char* buf, size_t len, uint64_t stamp_ms, RtpHeader* rtp) override;
};

class RtcTransport;

struct WrappedRtpTrack : public WrappedMediaTrack {
    explicit WrappedRtpTrack(MediaTrack::Ptr ptr, TwccContext& twcc, RtcTransport& t)
        : WrappedMediaTrack(std::move(ptr))
        , _twcc_ctx(twcc)
        , _transport(t) {}
    TwccContext& _twcc_ctx;
    RtcTransport& _transport;
    void inputRtp(const char* buf, size_t len, uint64_t stamp_ms, RtpHeader* rtp) override;
};

}

#endif