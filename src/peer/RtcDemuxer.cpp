#include "media/H264Track.h"
#include "media/H265Track.h"
#include "media/G711Track.h"
#include "base/Log.h"
#include "RtcDemuxer.h"
#include "RtcTransport.h"

namespace frtc {
    
RtcDemuxer::RtcDemuxer(RtcTransport* transport)
          : _transport(transport) {}

void RtcDemuxer::loadSdp(SdpSp sdp) {
    MediaDescPtr video = sdp->videoDesc(); 
    if (video && !video->payloads.empty()) {
        createVideoDecoder(video->payloads[0]);
    }

    MediaDescPtr audio = sdp->audioDesc(); 
    if (audio && !audio->payloads.empty()) {
        createAudioDecoder(audio->payloads[0]);
    }
}    

void RtcDemuxer::inputRtp(RtpPacket::Ptr rtp) {
     switch (rtp->type) {
        case MediaType::video: {
            if (_videoDecoder) {
                LOGI("%s", "video rtp decoder input video packet");
                _videoDecoder->inputRtp(rtp, true);
            }
            break;
        }
        case MediaType::audio: {
            if (_audioDecoder) {
                LOGI("%s", "audio rtp decoder input audio packet");
                _audioDecoder->inputRtp(rtp, false);
            }
            break;
        }
        default: break;
    }
}

void RtcDemuxer::addTrack(TrackPtr track) {
    if (_transport) {
        _transport->addTrack(track);
    }
}

void RtcDemuxer::addTrackComplete() {

} 

void RtcDemuxer::createAudioDecoder(MediaPayloadPtr audioPayload) {
    if (_audioDecoder) {
        return;
    }
    
    TrackPtr track;
    CodecId id = getCodecId(audioPayload->encodingName);
    switch (id) {
    case CodecId::g711a:
    case CodecId::g711u:
        track = std::make_shared<G711Track>(id, 8000, 1, 16);
        break;
    default:
        break;
    }

    LOGI("create audio encoding name=%s", audioPayload->encodingName.c_str());
    _audioDecoder = createRtpDecoder(id);
    if (_audioDecoder) {
        _audioDecoder->setRecieve([this](FramePtr frame) {
            if (frame && _transport) {
                _transport->inputFrame(frame);
            }
        });
        if (track) {
            addTrack(track);
        }
    }
}

void RtcDemuxer::createVideoDecoder(MediaPayloadPtr videoPayload) {
    if (_videoDecoder) {
        return;
    }

    TrackPtr track;
    CodecId id = getCodecId(videoPayload->encodingName);
    switch (id) {
    case CodecId::h264:
        track = std::make_shared<H264Track>();
        break;
    case CodecId::h265:
        track = std::make_shared<H265Track>();
        break;
    default:
        break;
    }

    LOGI("create video encoding name=%s", videoPayload->encodingName.c_str());
    _videoDecoder = createRtpDecoder(id);
    if (_videoDecoder) {
        _videoDecoder->setRecieve([this](FramePtr frame) {
            if (frame && _transport) {
                _transport->inputFrame(frame);
            }
        });
        if (track) {
            addTrack(track);
        }
    }
}

}