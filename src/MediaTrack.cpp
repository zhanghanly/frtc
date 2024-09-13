#include <arpa/inet.h>
#include "MediaTrack.h"
#include "RtcTransport.h"

namespace frtc {

std::shared_ptr<RtpChannel> MediaTrack::getRtpChannel(uint32_t ssrc) const {
    auto it_chn = rtp_channel.find(rtp_ext_ctx->getRid(ssrc));
    if (it_chn == rtp_channel.end()) {
        return nullptr;
    }
    
    return it_chn->second;
}


void WrappedRtpTrack::inputRtp(const char* buf, size_t len, uint64_t stamp_ms, RtpHeader* rtp) {
#if 0
    auto seq = ntohs(rtp->seq);
    if (track->media->type == TrackVideo && seq % 100 == 0) {
        //此处模拟接受丢包
        return;
    }
#endif

    auto ssrc = ntohl(rtp->ssrc);
    // 修改ext id至统一
    std::string rid;
    auto twcc_ext = track->rtp_ext_ctx->changeRtpExtId(rtp, true, &rid, RtpExtType::transport_cc);
    if (twcc_ext) {
        _twcc_ctx.onRtp(ssrc, twcc_ext.getTransportCCSeq(), stamp_ms);
    }

    auto& ref = track->rtp_channel[rid];
    if (!ref) {
        _transport.createRtpChannel(rid, ssrc, *track);
    }

    // 解析并排序rtp
    ref->inputRtp(track->track_type, track->rtp_payload->clockRate, (uint8_t*)buf, len, false);
}

void WrappedRtxTrack::inputRtp(const char* buf, size_t len, uint64_t stamp_ms, RtpHeader* rtp) {
    // 修改ext id至统一
    std::string rid;
    track->rtp_ext_ctx->changeRtpExtId(rtp, true, &rid, RtpExtType::transport_cc);

    auto& ref = track->rtp_channel[rid];
    if (!ref) {
        // 再接收到对应的rtp前，丢弃rtx包
        //WarnL << "unknown rtx rtp, rid:" << rid << ", ssrc:" << ntohl(rtp->ssrc) << ", codec:" << track->plan_rtp->codec
        //      << ", seq:" << ntohs(rtp->seq);
        return;
    }

    // 这里是rtx重传包
    //  https://datatracker.ietf.org/doc/html/rfc4588#section-4
    auto payload = rtp->getPayloadData();
    auto size = rtp->getPayloadSize(len);
    if (size < 2) {
        return;
    }

    // 前两个字节是原始的rtp的seq
    auto origin_seq = payload[0] << 8 | payload[1];
    // rtx 转换为 rtp
    rtp->pt = track->rtp_payload->payloadType;
    rtp->seq = htons(origin_seq);
    rtp->ssrc = htonl(ref->getSSRC());

    memmove((uint8_t*)buf + 2, buf, payload - (uint8_t*)buf);
    buf += 2;
    len -= 2;
    ref->inputRtp(track->track_type, track->rtp_payload->clockRate, (uint8_t*)buf, len, true);
}

}