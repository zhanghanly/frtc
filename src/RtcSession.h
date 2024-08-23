#ifndef FRTC_RTC_SESSION_H
#define FRTC_RTC_SESSION_H

#include <string>
#include "Sdp.h"
#include "Ticker.h"

namespace frtc {

class RtcSession {
public:
    RtcSession(const std::string&);
    
    std::string createLocalSdp();

    void setRemoteSdp(const std::string&);

    SdpSp getRtmoteSdp();

    std::string getRemoteFingerprint();

    CandidatePtr getCandidate();

    void setLocalIceInfo(const SessionInfo&);
    
    SessionInfoPtr getLocalIceInfo();

    SessionInfoPtr getRemoteIceInfo();

    bool alive();

    void updateTicker();

private:
    SdpSp _localSdp;    
    SdpSp _remoteSdp;
    TickerSp _ticker;
    std::string _localFingerprint;
};

typedef std::shared_ptr<RtcSession> RtcSessionSp;

}

#endif