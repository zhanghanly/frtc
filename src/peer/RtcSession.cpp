#include <cstdlib>
#include "RtcSession.h"
#include "base/Utility.h"

namespace frtc {

RtcSession::RtcSession(const std::string& fingerprint)
    : _localFingerprint(fingerprint) {
    _ticker = std::make_shared<Ticker>();
    _localSdp = std::make_shared<Sdp>();
    _remoteSdp = std::make_shared<Sdp>();
}

std::string RtcSession::createLocalSdp() {
    return _localSdp->create(_localFingerprint);
}

void RtcSession::setRemoteSdp(const std::string& sdp) {
    _remoteSdp->parse(sdp);
}
    
SdpSp RtcSession::getRtmoteSdp() {
    return _remoteSdp;
}

CandidatePtr RtcSession::getCandidate() {
    MediaDescPtr video = _remoteSdp->videoDesc();
    if (!video) {
        return nullptr;
    }
    
    return video->candidate;
}
    
void RtcSession::setLocalIceInfo(const SessionInfo& info) {
    _localSdp->sessionInfo = info;
}

SessionInfoPtr RtcSession::getLocalIceInfo() {
    MediaDescPtr video = _localSdp->videoDesc();
    if (!video) {
        return nullptr;
    }

    return video->sessionInfo;
}

SessionInfoPtr RtcSession::getRemoteIceInfo() {
    MediaDescPtr video = _remoteSdp->videoDesc();
    if (!video) {
        return nullptr;
    }
    
    return video->sessionInfo; 
}

std::string RtcSession::getRemoteFingerprint() {
    MediaDescPtr video = _remoteSdp->videoDesc();
    if (!video) {
        return "";
    }
    
    return video->sessionInfo->fingerprint;
}

bool RtcSession::alive() {
    return _ticker->elapsedTime() < 10000;
}

void RtcSession::updateTicker() {
    _ticker->resetTime();
}

}