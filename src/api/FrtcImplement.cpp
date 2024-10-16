#include "peer/PeerConnection.h"
#include "base/Log.h"
#include "FrtcApi.h"

void* frtcCreateCtx(void) {
    frtc::PeerConnection* peer = new frtc::PeerConnection();
    peer->initializer(nullptr);
    LOGI("%s", "create peer connection finished");

    return peer;
}

void frtcSetStreamConfig(void* ctx, FrtcStreamConfig* config) {
    frtc::PeerConnection* peer = (frtc::PeerConnection*)ctx;
    if (peer) {
        peer->setStreamConfig(config);
    }
}

int frtcConnectSignalServer(void* ctx, const char* url) {
    LOGI("%s", "connect signal server start");
    frtc::PeerConnection* peer = (frtc::PeerConnection*)ctx;
    if (peer) {
        peer->connnectSignalServer(url);
        peer->startEstablishConnection();
        LOGI("%s", "connect signal server complete");
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