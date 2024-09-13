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
    //frtcConnectSignalServer(handler, "https://video.whatsgps.com:1443/47.236.232.146-20443/index/api/webrtc?app=jtt1078&stream=055000018798_1_0&type=play");
    frtcConnectSignalServer(handler, "webrtc://video.whatsgps.com:1443/47.236.232.146-20443/index/api/webrtc?app=jtt1078&stream=055000018791_1_0&type=play");
    while (true) {

    }

    return 0;
}
