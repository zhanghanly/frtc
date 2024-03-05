#ifndef FRTC_MEDIA_SOURCE_H
#define FRTC_MEDIA_SOURCE_H

#include <vector>
#include <map>
#include <list>
#include "Track.h"
#include "Frame.h"

namespace frtc {

class MediaSource {
public:
    MediaSource();    

    virtual ~MediaSource() = default;    

    void inputFrame(FramePtr);

    void addTrack(TrackPtr);

    virtual void onTrackFrame(FramePtr) = 0;

    virtual void onTrackReady() = 0;

    void checkTrackIfReady(void);

    TrackPtr getTrack(MediaType);

    std::vector<TrackPtr> getAllTracks();

private:
    bool _allTrackReady;
    std::map<MediaType, TrackPtr> _trackMap;
    std::map<MediaType, std::list<FramePtr>> _cachedFrame;
};

class RtcMediaSource : public MediaSource {
public:
    ~RtcMediaSource() override = default;
    virtual void onTrackFrame(FramePtr);

    virtual void onTrackReady();
};

}

#endif