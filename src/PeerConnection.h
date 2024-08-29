#ifndef FRTC_PEER_CONNECTION_H
#define FRTC_PEER_CONNECTION_H

#include "NetworkInterface.h"
#include "FrtcApi.h"
#include "RtcContext.h"
#include "RtcConfig.h"

namespace frtc {

class PeerConnection {
public:
    void initializer(RtcConfigSp); 

    void connnectSignalServer(const std::string&);

    void startEstablishConnection();

    void setStreamConfig(FrtcStreamConfig*);

    void destory();

private:
    std::string replaceUrl(const std::string&);

private:
    RtcContextSp _context;
    NetworkSp _signalClient;
    FrtcStreamConfig _config;
    bool _setAudioParam = false;
    bool _setVideoParam = false;
    FrtcVideoParam _videoParam;
};

}

#endif