#ifndef FRTC_RTC_DEMUXER_H
#define FRTC_RTC_DEMUXER_H

#include <memory>
#include "rtp/RtpPacket.h"
#include "rtp/RtpDecoder.h"
#include "Track.h"
#include "Sdp.h"

namespace frtc {

class RtcTransport;

class RtcDemuxer {
public:
    RtcDemuxer(RtcTransport*);
    
    void loadSdp(SdpSp);    
    
    void inputRtp(RtpPacket::Ptr rtp);

private:
    void addTrack(TrackPtr);

    void addTrackComplete(); 

    void createAudioDecoder(MediaPayloadPtr);

    void createVideoDecoder(MediaPayloadPtr);

private:
    RtcTransport* _transport;
    RtpDecoderSp _videoDecoder;
    RtpDecoderSp _audioDecoder;
};

typedef std::shared_ptr<RtcDemuxer> RtcDemuxerSp;

}

#endif