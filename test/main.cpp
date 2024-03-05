#include <iostream>
//#include "../Log.h"
#include "FrtcApi.h"

int main(int argc, char** argv) {
    //LOG_INIT("log", "myname", 3);
    //for (int i = 0; i < 3; i++) {
    //    LOG_ERROR("my number is number my number is my number is my number is my number is my number is my number is %d", i);
    //}

    void* handler = frtcCreateCtx();
    if (!handler) {
        return -1;
    }
    //frtcConnectSignalServer(handler, "https://video.gpsnow.net:9443/36.133.148.156-40443/index/api/webrtc?app=jtt1078&stream=078343253343_1_0&type=play");
    //frtcConnectSignalServer(handler, "https://video.gpsnow.net:9443/36.133.148.156-40443/index/api/webrtc?app=jtt1078&stream=078343375922_1_0&type=play");
    //frtcConnectSignalServer(handler, "https://video.whatsgps.com:1443/47.236.232.146-20443/index/api/webrtc?app=jtt1078&stream=055000018787_1_0&type=play");
    frtcConnectSignalServer(handler, "https://video.gpsnow.net:9443/36.133.148.156-40443/index/api/webrtc?app=jtt1078&stream=015610004169_1_0&type=play");
    while (true) {


    }


    return 0;
}