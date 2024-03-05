#include "FrtcApi.h"
#include "PeerConnection.h"

void* frtcCreateCtx(void) {
    frtc::PeerConnection* peer = new frtc::PeerConnection();
    peer->initializer(nullptr);
    
    return peer;
}

void frtcSetAudioCallback(OnRecieveAudioFrame callback) {


}

void frtcSetVideoCallback(OnRecieveVideoFrame callback) {


}

int frtcConnectSignalServer(void* ctx, const char* url) {
    frtc::PeerConnection* peer = (frtc::PeerConnection*)ctx;
    peer->connnectSignalServer(url);
    peer->startEstablishConnection();

    return 0;
}


void frtcCodeToString(int32_t code, char* buffer, int32_t len) {


}

void frtcDestoryCtx(void* ctx) {
    if (ctx) {
        frtc::PeerConnection* peer = (frtc::PeerConnection*)ctx;
        delete peer;
    }
}