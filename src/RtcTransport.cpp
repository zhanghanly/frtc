#include <cassert>
#include <arpa/inet.h>
#include "RtcTransport.h"
#include "MediaTrack.h"
#include "RtcContext.h"

namespace frtc {
    
RtcTransport::RtcTransport(RtcContext* context) {
    _context = context;
    _alive_ticker = std::make_shared<Ticker>();
    _rr_ticker = std::make_shared<Ticker>();
    _source = std::make_shared<RtcMediaSource>(this);
    _demuxer = std::make_shared<RtcDemuxer>(this);
    // twcc callback
    _twcc_ctx.setOnSendTwccCB([this](uint32_t ssrc, std::string fci) { 
        onSendTwcc(ssrc, fci);
    });
}

void RtcTransport::loadSdp(SdpSp sdp) {
    // 获取ssrc和pt相关信息,届时收到rtp和rtcp时分别可以根据pt和ssrc找到相关的信息
    for (auto& media : sdp->medias) {
        auto track = std::make_shared<MediaTrack>();
        if (media->type == "video") {
            track->track_type = MediaType::video;
        } else if (media->type == "audio") {
            track->track_type = MediaType::audio;
        } else {
            continue;
        }
        
        track->answer_rtp_ssrc = media->ssrc->ssrc; 
        track->rtp_payload = media->payloads[0]; 
        if (media->support_rtx) {
            track->answer_rtx_ssrc = media->ssrc->rtx_ssrc;
            track->rtx_payload = media->payloads[1]; 
        } 
        // recv ssrc --> MediaTrack
        _ssrc_to_track[track->answer_rtp_ssrc] = track;
        _ssrc_to_track[track->answer_rtx_ssrc] = track;

        std::cout << "track ssrc=" << track->answer_rtp_ssrc << " rtx ssrc=" << track->answer_rtx_ssrc << std::endl;    
        std::cout << "create track->rtp_payload->payloadType " << (track->rtp_payload)->payloadType << std::endl;
        // rtp pt --> MediaTrack
        _pt_to_track.emplace(
            (track->rtp_payload)->payloadType, std::unique_ptr<WrappedMediaTrack>(new WrappedRtpTrack(track, _twcc_ctx, *this)));
        if (media->support_rtx) {
            // rtx pt --> MediaTrack
            _pt_to_track.emplace((track->rtx_payload)->payloadType, std::unique_ptr<WrappedMediaTrack>(new WrappedRtxTrack(track)));
            std::cout << "create rtx track, ssrc=" << track->answer_rtx_ssrc << " pt=" << track->rtx_payload->payloadType << std::endl;
        }
        // 记录rtp ext类型与id的关系，方便接收或发送rtp时修改rtp ext id
        track->rtp_ext_ctx = std::make_shared<RtpExtContext>();
        std::weak_ptr<MediaTrack> weak_track = track;
        track->rtp_ext_ctx->setOnGetRtp([this, weak_track](uint8_t pt, uint32_t ssrc, const std::string& rid) {
            // ssrc --> MediaTrack
            auto track = weak_track.lock();
            assert(track);
            _ssrc_to_track[ssrc] = std::move(track);
            //InfoL << "get rtp, pt:" << (int)pt << ", ssrc:" << ssrc << ", rid:" << rid;
        });
    }
    // init demuxer audio and vieo decoder
    _demuxer->loadSdp(sdp);
} 


void RtcTransport::onRtp(const char* data, uint32_t size) {
    _alive_ticker->resetTime();
    RtpHeader* rtp = (RtpHeader*)data;
    // 根据接收到的rtp的pt信息，找到该流的信息
    auto it = _pt_to_track.find(rtp->pt);
    if (it == _pt_to_track.end()) {
        std::cout << "unknown rtp pt: " << (int)rtp->pt << std::endl;
        return;
    }
        
    uint64_t stamp_ms = getCurrentMillisecond();
    it->second->inputRtp(data, size, stamp_ms, rtp);
    //std::cout << "revieve rtp pt: " << (int)rtp->pt << std::endl;
}

void RtcTransport::onRtcp(const char* data, uint32_t size) {
    auto rtcps = RtcpHeader::loadFromBytes((char*)data, size);
    for (auto rtcp : rtcps) {
        switch ((RtcpType)rtcp->pt) {
        case RtcpType::RTCP_SR: {
            _alive_ticker->resetTime();
            // 对方汇报rtp发送情况
            RtcpSR* sr = (RtcpSR*)rtcp;
            auto it = _ssrc_to_track.find(sr->ssrc);
            if (it != _ssrc_to_track.end()) {
                auto& track = it->second;
                auto rtp_chn = track->getRtpChannel(sr->ssrc);
                if (!rtp_chn) {
                    std::cout << "unknown sr rtcp packet:" << rtcp->dumpString() << std::endl;
                } else {
                    // 设置rtp时间戳与ntp时间戳的对应关系
                    rtp_chn->setNtpStamp(sr->rtpts, sr->getNtpUnixStampMS());
                    auto rr = rtp_chn->createRtcpRR(sr, track->answer_rtp_ssrc);
                    if (_context) {
                        _context->sendRtcpPacket(rr->data(), rr->size());
                    }
                }
            } else {
                std::cout << "unknown  ssrc  sr rtcp packet:" << rtcp->dumpString() << std::endl;
            }
            break;
        }
        case RtcpType::RTCP_RR: {
            _alive_ticker->resetTime();
            // 对方汇报rtp接收情况
            RtcpRR* rr = (RtcpRR*)rtcp;
            for (auto item : rr->getItemList()) {
                auto it = _ssrc_to_track.find(item->ssrc);
                if (it != _ssrc_to_track.end()) {
                    auto& track = it->second;
                    track->rtcp_context_send->onRtcp(rtcp);
                    //auto sr = track->rtcp_context_send->createRtcpSR(track->answer_ssrc_rtp);
                    //sendRtcpPacket(sr->data(), sr->size(), true);
                } else {
                    //WarnL << "未识别的rr rtcp包:" << rtcp->dumpString();
                }
            }
            break;
        }
        case RtcpType::RTCP_BYE: {
            // 对方汇报停止发送rtp
            RtcpBye* bye = (RtcpBye*)rtcp;
            for (auto ssrc : bye->getSSRC()) {
                auto it = _ssrc_to_track.find(*ssrc);
                if (it == _ssrc_to_track.end()) {
                    //WarnL << "未识别的bye rtcp包:" << rtcp->dumpString();
                    continue;
                }
                _ssrc_to_track.erase(it);
            }
            //onRtcpBye();
            // bye 会在 sender audio track mute 时出现, 因此不能作为 shutdown 的依据
            break;
        }
        case RtcpType::RTCP_PSFB:
        case RtcpType::RTCP_RTPFB: {
            if ((RtcpType)rtcp->pt == RtcpType::RTCP_PSFB) {
                break;
            }
            // RTPFB
            switch ((RTPFBType)rtcp->report_count) {
            case RTPFBType::RTCP_RTPFB_NACK: {
                RtcpFB* fb = (RtcpFB*)rtcp;
                auto it = _ssrc_to_track.find(fb->ssrc_media);
                if (it == _ssrc_to_track.end()) {
                    //WarnL << "未识别的 rtcp包:" << rtcp->dumpString();
                    return;
                }
                auto& track = it->second;
                auto& fci = fb->getFci<FCI_NACK>();
                track->nack_list.forEach(fci, [&](const RtpPacket::Ptr& rtp) {
                    // rtp重传
                    //onSendRtp(rtp, true, true);
                });
                break;
            }
            default:
                break;
            }
            break;
        }
        case RtcpType::RTCP_XR: {
            RtcpXRRRTR* xr = (RtcpXRRRTR*)rtcp;
            if (xr->bt != 4) {
                break;
            }
            auto it = _ssrc_to_track.find(xr->ssrc);
            if (it == _ssrc_to_track.end()) {
                //WarnL << "未识别的 rtcp包:" << rtcp->dumpString();
                return;
            }
            auto& track = it->second;
            track->rtcp_context_send->onRtcp(rtcp);
            //auto xrdlrr = track->rtcp_context_send->createRtcpXRDLRR(track->answer_ssrc_rtp, track->answer_ssrc_rtp);
            //sendRtcpPacket(xrdlrr->data(), xrdlrr->size(), true);
            break;
        }
        default:
            break;
        }
    }
}
    
void RtcTransport::createRtpChannel(const std::string& rid, uint32_t ssrc, MediaTrack& track) {
     // rid --> RtpReceiverImp
    auto& ref = track.rtp_channel[rid];
    std::weak_ptr<RtcTransport> weak_self = shared_from_this();
    ref = std::make_shared<RtpChannel>(
        [&track, this, rid](RtpPacket::Ptr rtp) mutable { 
            onSortedRtp(track, rid, std::move(rtp));
        },
        [&track, weak_self, ssrc](const FCI_NACK& nack) mutable {
            // nack发送可能由定时器异步触发
            auto strong_self = weak_self.lock();
            if (strong_self) {
                strong_self->onSendNack(track, nack, ssrc);
            }
        
            std::cout << "send rtcp nack packet" << std::endl;
        });
    std::cout << "create rtp receiver of ssrc:" << ssrc << ", rid:" << rid << ", codec:" << std::endl;
}

void RtcTransport::onSortedRtp(MediaTrack& track, const std::string& rid, RtpPacket::Ptr rtp) {
    std::cout << "recieve sorted rtp packet, pt=" << (int)rtp->getHeader()->pt << std::endl;
    if (_demuxer) {
        _demuxer->inputRtp(rtp);
    }
    if (_rr_ticker->elapsedTime() >= 5000) {
        if (track.track_type == MediaType::video) {
            auto& ref = track.rtp_channel[rid];
            if (!ref) {
                return;
            }

            auto rr = ref->createRtcpRR(track.answer_rtp_ssrc);
            if (_context) {
                _context->sendRtcpPacket(rr->data(), rr->size());
                std::cout << "send rtcp rr packet" << std::endl;
            }
            _rr_ticker->resetTime();
        }
    }
}
    
void RtcTransport::onSendNack(MediaTrack& track, const FCI_NACK& nack, uint32_t ssrc) {
    auto rtcp = RtcpFB::create(RTPFBType::RTCP_RTPFB_NACK, &nack, FCI_NACK::kSize);
    rtcp->ssrc = htonl(track.answer_rtp_ssrc);
    rtcp->ssrc_media = htonl(ssrc);
    if (_context) {
        _context->sendRtcpPacket((char*)rtcp.get(), rtcp->getSize());
    }

    std::cout << "send rtcp nack request" << std::endl;
}
    
void RtcTransport::onSendTwcc(uint32_t ssrc, const std::string& twcc_fci) {
    auto rtcp = RtcpFB::create(RTPFBType::RTCP_RTPFB_TWCC, twcc_fci.data(), twcc_fci.size());
    rtcp->ssrc = htonl(0);
    rtcp->ssrc_media = htonl(ssrc);
    if (_context) {
        _context->sendRtcpPacket((char*)rtcp.get(), rtcp->getSize());
    }
    
    std::cout << "send rtcp twcc request" << std::endl;
}
    
void RtcTransport::addTrack(TrackPtr track) {
    std::cout << __FUNCTION__ << " add track" << std::endl;
    if (_source && track) {
        _source->addTrack(track);
    }
}
    
void RtcTransport::inputFrame(FramePtr frame) {
    std::cout << __FUNCTION__ << " input frame" << std::endl;
    if (_source && frame) {
        _source->inputFrame(frame);
    }
}

void RtcTransport::onFrame(FramePtr frame) {
    if (frame) {
        if (frame->mediaType() == MediaType::video) {
            std::cout << __FUNCTION__ << " on video frame" << std::endl;

        } else if (frame->mediaType() == MediaType::audio) {
            std::cout << __FUNCTION__ << " on audio frame" << std::endl;
        }

        if (_frame_cb) {
            _frame_cb(frame);
        }
    }
}
    
void RtcTransport::setFrameCallback(std::function<void(FramePtr)> cb) {
    if (cb) {
        _frame_cb = cb;
    }
}

}