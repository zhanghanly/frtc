#include <iostream>
#include "PeerConnection.h"
#include "FrtcApi.h"

void* frtcCreateCtx(void) {
    frtc::PeerConnection* peer = new frtc::PeerConnection();
    peer->initializer(nullptr);

    return peer;
}

void frtcSetStreamConfig(void* ctx, FrtcStreamConfig* config) {
    frtc::PeerConnection* peer = (frtc::PeerConnection*)ctx;
    if (peer) {
        peer->setStreamConfig(config);
    }
}

int frtcConnectSignalServer(void* ctx, const char* url) {
    std::cout << "connect signal server start" << std::endl;
    frtc::PeerConnection* peer = (frtc::PeerConnection*)ctx;
    if (peer) {
        peer->connnectSignalServer(url);
        peer->startEstablishConnection();
        std::cout << "connect signal server complete" << std::endl;
    }
    
    return 0;
}

void frtcCodeToString(int32_t code, char* buffer, int32_t len) {


}

void frtcDestoryCtx(void* ctx) {
    frtc::PeerConnection* peer = (frtc::PeerConnection*)ctx;
    if (peer) {
        peer->destory();
        delete peer;
    }
}