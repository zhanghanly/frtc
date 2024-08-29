#include "Track.h"
#include "Utility.h"

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

void splitH264(const char* ptr, size_t len, size_t prefix, 
               const std::function<void(const char*, size_t, size_t)>& cb) {
    auto start = ptr + prefix;
    auto end = ptr + len;
    size_t next_prefix;
    while (true) {
        auto next_start = memfind(start, end - start, "\x00\x00\x01", 3);
        if (next_start) {
            //找到下一帧
            if (*(next_start - 1) == 0x00) {
                //这个是00 00 00 01开头
                next_start -= 1;
                next_prefix = 4;
            } else {
                //这个是00 00 01开头
                next_prefix = 3;
            }
            //记得加上本帧prefix长度
            cb(start - prefix, next_start - start + prefix, prefix);
            //搜索下一帧末尾的起始位置
            start = next_start + next_prefix;
            //记录下一帧的prefix长度
            prefix = next_prefix;
            continue;
        }
        //未找到下一帧,这是最后一帧
        cb(start - prefix, end - start + prefix, prefix);
        break;
    }
}

}