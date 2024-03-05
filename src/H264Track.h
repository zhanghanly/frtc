#ifndef FRTC_H264_TRACK_H
#define FRTC_H264_TRACK_H

#include <string>
#include "Track.h"

namespace frtc {

class H264Track : public VideoTrack {
public:
    ~H264Track() override = default;
    
    CodecId codecId() override;

    bool ready() override;

    void inputFrame(FramePtr) override;

    int32_t height() override;

    int32_t width() override;

    int32_t fps() override;

private:
    void inputFrame_1(FramePtr);

    void insertConfigFrame(FramePtr);

private:
    int32_t _height;
    int32_t _width;
    int32_t _fps;

    std::string _sps;
    std::string _pps;
};

}


#endif