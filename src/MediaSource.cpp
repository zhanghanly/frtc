#include "MediaSource.h"
#include "RtcTransport.h"
#include "Log.h"

namespace frtc {

MediaSource::MediaSource() 
            : _allTrackReady(false) {}

void MediaSource::addTrack(TrackPtr track) { 
    if (_allTrackReady) {
        LOG_WARN("come too late, all tracks are ready"); 
        return;
    }
    
    auto trackType = track->mediaType();
    _trackMap[trackType] = track;
    track->addObservre([this](FramePtr frame) {
        if (_allTrackReady) {
            std::cout << "track is ready" << std::endl;
            onTrackFrame(frame);
        } else {
            std::cout << "track map size=" <<  _trackMap.size() << std::endl;
            std::cout << "track is not ready" << std::endl;
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
    if (_trackMap.size() == 2) {
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
    std::cout << "media source input frame" << std::endl;
    auto frameType = frame->mediaType();
    if (_trackMap.find(frameType) == _trackMap.end()) {
        std::cout << "not found track in map" << std::endl;
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