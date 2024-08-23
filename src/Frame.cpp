#include "Frame.h"

namespace frtc {

char* FrameBuffer::data() {
    if (_buffer.empty()) {
        return nullptr;
    }

    return &(_buffer[0]);
}

int32_t FrameBuffer::size() {
    return _buffer.size();
}
    
bool FrameBuffer::empty() {
    return _buffer.empty();
}


void FrameBuffer::append(char data) {
    _buffer.push_back(data);
}

void FrameBuffer::append(const char* data, int32_t size) {
    _buffer.insert(_buffer.end(), data, data + size);
}
    
void FrameBuffer::assign(const char* data, int32_t size) {
    _buffer.clear();
    append(data, size);
}

void FrameBuffer::clear() {
    _buffer.clear();
}


bool Frame::cacheAble() {
    return true;
}

bool Frame::dropAble() {
    return false;
}

bool Frame::decodeAble() {
    if (mediaType() != MediaType::video) {
        return true;
    }

    return !configFrame();
}

}
