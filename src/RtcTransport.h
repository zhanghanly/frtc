#ifndef RTC_TRANSPORT_H
#define RTC_TRANSPORT_H

#include <cstdint>
#include "TwccContext.h"
#include "MediaTrack.h"
#include "MediaSource.h"
#include "RtcDemuxer.h"
#include "Ticker.h"
#include "Frame.h"
#include "Sdp.h"

namespace frtc {

class RtcContext;

class RtcTransport : public std::enable_shared_from_this<RtcTransport> {
public:
    RtcTransport(RtcContext*); 
    
    void loadSdp(SdpSp);

    void onRtp(const char*, uint32_t);

    void onRtcp(const char*, uint32_t);

    void createRtpChannel(const std::string& rid, uint32_t ssrc, MediaTrack& track);

    void addTrack(TrackPtr);

    void inputFrame(FramePtr);

    void onFrame(FramePtr);

private:
    void onSortedRtp(MediaTrack& track, const std::string& rid, RtpPacket::Ptr rtp);

    void onSendNack(MediaTrack& track, const FCI_NACK& nack, uint32_t ssrc);
    
    void onSendTwcc(uint32_t ssrc, const std::string& twcc_fci);

private:
    //用掉的总流量
    uint64_t _bytes_usage = 0;
    //刷新计时器
    TickerSp _alive_ticker;
    //pli rtcp计时器
    //Ticker _pli_ticker;
    //rr rtcp计时器
    TickerSp _rr_ticker;
    //twcc rtcp发送上下文对象
    TwccContext _twcc_ctx;
    //根据发送rtp的track类型获取相关信息
    //MediaTrack::Ptr _type_to_track[2];
    //根据rtcp的ssrc获取相关信息，收发rtp和rtx的ssrc都会记录
    std::unordered_map<uint32_t/*ssrc*/, MediaTrack::Ptr> _ssrc_to_track;
    //根据接收rtp的pt获取相关信息
    std::unordered_map<uint8_t/*pt*/, std::unique_ptr<WrappedMediaTrack>> _pt_to_track;
    // recv video and audio frame
    MediaSourceSp _source;
    // rtc rtp demuxer
    RtcDemuxerSp _demuxer;
    RtcContext* _context;
};

typedef std::shared_ptr<RtcTransport> RtcTransportSp;

}

#endif