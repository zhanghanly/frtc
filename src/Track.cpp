#include "Track.h"

namespace frtc {

void Track::addObservre(std::function<void(FramePtr)> observer) {
    if (observer) {
        _observer = observer;
    }
}

void Track::inputFrame(FramePtr frame) {
    if (_observer) {
        _observer(frame);
    }
}

MediaType AudioTrack::mediaType() {
    return MediaType::audio;
}

MediaType VideoTrack::mediaType() {
    return MediaType::video;
}

}