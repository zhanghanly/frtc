#include "PeerConnection.h"
#include "Log.h"

namespace frtc {

void PeerConnection::initializer(RtcConfigSp config) {
    if (!_context) {
        _context = std::make_shared<RtcContext>();
    }
}

void PeerConnection::connnectSignalServer(const std::string& url) {
    if (!_signalClient) {
        _signalClient = createNetworkClient(parseUrlType(url));
        _signalClient->setReadCb([this](SignalErr code, const std::string& sdp) {
            if (code == SignalErr::SUCCESS) {
                _context->setRemoteSdp(sdp);
                std::cout << "set remote sdp" << std::endl;

            } else {
                LOG_ERROR("%s", "connect server failed");
            }
        });
    }
    std::string localSdp = _context->createLocalSdp();
    _signalClient->connectPeer(url);
    _signalClient->sendReq(localSdp);

    std::cout << "localSdp=" << localSdp << std::endl;
}
    
void PeerConnection::startEstablishConnection() {
    _context->startConnectPeer();
    std::cout << "start connect peer" << std::endl;
}

void PeerConnection::setVideoCallback(OnRecieveVideoFrame callback) {


}

void PeerConnection::setAudioCallback(OnRecieveAudioFrame callback) {


}
    
void PeerConnection::destory() {


}

}