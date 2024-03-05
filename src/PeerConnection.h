#ifndef FRTC_PEER_CONNECTION_H
#define FRTC_PEER_CONNECTION_H

#include "FrtcApi.h"
#include "RtcContext.h"
#include "RtcConfig.h"
#include "NetworkInterface.h"

namespace frtc {

class PeerConnection {
public:
    void initializer(RtcConfigSp); 

    void connnectSignalServer(const std::string&);

    void startEstablishConnection();

    void setVideoCallback(OnRecieveVideoFrame);

    void setAudioCallback(OnRecieveAudioFrame);

    void destory();

private:
    RtcContextSp _context;
    NetworkSp _signalClient;
};

}

#endif