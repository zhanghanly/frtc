#include "MediaSource.h"
#include "RtcTransport.h"
#include "base/Log.h"

namespace frtc {

MediaSource::MediaSource() 
            : _allTrackReady(false) {}

void MediaSource::addTrack(TrackPtr track) { 
    //if (_allTrackReady) {
    //    LOGW("%s", "come too late, all tracks are ready"); 
    //    return;
    //}
    
    auto trackType = track->mediaType();
    _trackMap[trackType] = track;
    track->addObservre([this](FramePtr frame) {
        if (_allTrackReady) {
            onTrackFrame(frame);
        } else {
            LOGI("track is not ready, track map size=%lu", _trackMap.size());
            /*cache first*/
            auto& cacheLst = _cachedFrame[frame->mediaType()];
            while (cacheLst.size() > 50) {
                cacheLst.pop_front();
            }
            cacheLst.push_back(frame);
        }
    });
    checkTrackIfReady();
}
    
void MediaSource::checkTrackIfReady(void) {
    if (_allTrackReady) {
        return;
    }
    if (_trackMap.size() >= 1) {
        for (auto& item : _cachedFrame) {
            for (auto& frame : item.second) {
                _trackMap[item.first]->inputFrame(frame);
            }
            item.second.clear();
        }

        _allTrackReady = true;
    }
}

void MediaSource::inputFrame(FramePtr frame) {
    LOGI("%s", "media source input frame");
    auto frameType = frame->mediaType();
    if (_trackMap.find(frameType) == _trackMap.end()) {
        LOGW("%s", "not found track in map");
        return;
    }

    _trackMap[frameType]->inputFrame(frame);
}

TrackPtr MediaSource::getTrack(MediaType type) {
    for (auto& item : _trackMap) {
        if (item.first == type) {
            return item.second;
        }
    }

    return nullptr;
}

std::vector<TrackPtr> MediaSource::getAllTracks() {
    std::vector<TrackPtr> tracks;
    for (auto& item : _trackMap) {
        tracks.push_back(item.second);
    }

    return tracks;
}


RtcMediaSource::RtcMediaSource(RtcTransport* transport)
              : _transport(transport) {} 

void RtcMediaSource::onTrackFrame(FramePtr frame) {
    if (_transport) {
        _transport->onFrame(frame);
    }
}

void RtcMediaSource::onTrackReady() {

}

}