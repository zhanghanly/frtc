#include <cstdlib>
#include "RtcSession.h"
#include "Function.h"

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

Candidate RtcSession::getCandidate() {
    return (_remoteSdp->videoDesc).candidate;
}
    
void RtcSession::setLocalIceInfo(const SessionInfo& info) {
    _localSdp->sessionInfo = info;
}

SessionInfo RtcSession::getLocalIceInfo() {
    return (_localSdp->videoDesc).sessionInfo;
}

SessionInfo RtcSession::getRemoteIceInfo() {
    return (_remoteSdp->videoDesc).sessionInfo;
}

std::string RtcSession::getRemoteFingerprint() {
    return (_remoteSdp->videoDesc).sessionInfo.fingerprint;
}

bool RtcSession::alive() {
    return _ticker->elapsedTime() < 10000;
}

void RtcSession::updateTicker() {
    _ticker->resetTime();
}

}