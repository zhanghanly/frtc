#include <iostream>
#include "PeerConnection.h"
#include "Log.h"

namespace frtc {

void PeerConnection::initializer(RtcConfigSp config) {
    if (!_context) {
        _context = std::make_shared<RtcContext>();
        _context->setFrameCallback([this](FramePtr frame) {
            if (frame) {
                FrtcFrame rtcFrame;
                rtcFrame.data = (uint8_t*)(frame->data());
                rtcFrame.size = frame->size();
                rtcFrame.pts = frame->pts();
                rtcFrame.dts = frame->dts();

                if (frame->mediaType() == MediaType::video) {
                    if (frame->keyFrame() && !_setVideoParam) {
                        std::cout << "call video param callback" << std::endl;
                        if (frame->codecId() == CodecId::h264) {
                            _videoParam.codec = FRTC_H264;
                        } else if (frame->codecId() == CodecId::h265) {
                            _videoParam.codec = FRTC_H265;
                        }
                        if (_config.video_param_cb) {
                            _config.video_param_cb(_config.user_data, &_videoParam);
                            _setVideoParam = true;
                        }
                    }
                    if (frame->configFrame()) {
                        if (!_setVideoParam) {
                            std::cout << "config frame" << std::endl;
                            memcpy(_videoParam.extra + _videoParam.extraLen, frame->data(), frame->size()); 
                            _videoParam.extraLen += frame->size();
                        }
                    } else {
                        if (_config.video_frame_cb) {
                            _config.video_frame_cb(_config.user_data, &rtcFrame);
                        }
                    }
                
                } else if (frame->mediaType() == MediaType::audio) {
                    if (!_setAudioParam && _config.audio_param_cb) {
                        FrtcAudioParam param;
                        param.codec = FRTC_PCMA; 
                        param.channels = 1;
                        param.samplerate = 8000;
                        param.samplerateDepth = 16;

                        _config.audio_param_cb(_config.user_data, &param);
                        _setAudioParam = true;
                    }
                    if (_config.audio_frame_cb) {
                        _config.audio_frame_cb(_config.user_data, &rtcFrame);
                    }
                }
            }
        });
        _videoParam.extraLen = 0;
    }
}

void PeerConnection::connnectSignalServer(const std::string& url) {
    std::string realUrl = replaceUrl(url);
    if (!_signalClient) {
        _signalClient = createNetworkClient(parseUrlType(realUrl));
        _signalClient->setReadCb([this](SignalErr code, const std::string& sdp) {
            if (code == SignalErr::SUCCESS) {
                _context->setRemoteSdp(sdp);
                std::cout << "set remote sdp" << std::endl;

            } else {
                std::cout << "connect server failed" << std::endl;
            }
        });
    }
    
    std::string localSdp = _context->createLocalSdp();
    _signalClient->connectPeer(realUrl);
    _signalClient->sendReq(localSdp);
}
    
void PeerConnection::startEstablishConnection() {
    _context->startConnectPeer();
    std::cout << "start connect peer" << std::endl;
}

void PeerConnection::setStreamConfig(FrtcStreamConfig* config) {
    if (config) {
        _config.user_data = config->user_data;
        _config.video_param_cb = config->video_param_cb;
        _config.audio_param_cb = config->audio_param_cb;
        _config.video_frame_cb = config->video_frame_cb;
        _config.audio_frame_cb = config->audio_frame_cb;
    }
}

std::string PeerConnection::replaceUrl(const std::string& webrtcUrl) {
    std::cout << "old url=" << webrtcUrl << std::endl;
    
    if (webrtcUrl.find("webrtc://") != std::string::npos) {
        std::string httpsUrl = "https"; 
        httpsUrl += webrtcUrl.c_str() + 6;
        std::cout << "new url=" << httpsUrl << std::endl;

        return httpsUrl;
    } 

    return webrtcUrl;
}

void PeerConnection::destory() {
    _context->stop();
}

}