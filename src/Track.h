#ifndef FRTC_TRACK_H
#define FRTC_TRACK_H

#include <functional>
#include <memory>
#include <list>
#include "Frame.h"
#include "MediaCodec.h"

namespace frtc {

class Track : public MediaCodec {
public:
    virtual ~Track() = default;
    
    virtual bool ready() = 0;

    //virtual int32_t getSSRC() = 0;

    virtual void inputFrame(FramePtr);

    void addObservre(std::function<void(FramePtr)>);

protected:
    std::function<void(FramePtr)> _observer;
};

typedef std::shared_ptr<Track> TrackPtr;


class AudioTrack : public Track {
public:
    virtual ~AudioTrack() = default;
    
    MediaType mediaType() override;

    virtual int32_t channels() = 0;

    virtual int32_t sampleRate() = 0;

    virtual int32_t sampleBytes() = 0;
};

class VideoTrack : public Track {
public:
    virtual ~VideoTrack() = default;

    MediaType mediaType() override;
    
    virtual int32_t height() = 0;

    virtual int32_t width() = 0;

    virtual int32_t fps() = 0;
};

}

#endif