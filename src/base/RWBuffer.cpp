#include <cstring>
#include "RWBuffer.h"

namespace frtc {

void RWBuffer::init(char* b, int32_t nn){
	_head = _data = b;
	_size = nn;
}

void RWBuffer::destroy(){
}

int32_t RWBuffer::pos() {
    return (int)(_head - _data);
}
    
char* RWBuffer::data() {
    return _data;
}
    
int32_t RWBuffer::size() {
    return _size;
}

int32_t RWBuffer::left() {
    return _size - (int)(_head - _data);
}

int32_t RWBuffer::empty() {
    return !_data || (_head >= _data + _size);
}

int32_t RWBuffer::require(int32_t required_size) {
    if (required_size < 0) {
        return 0;
    }

    return required_size <= _size - (_head - _data);
}

void RWBuffer::skip(int32_t size) {
    _head += size;
}

char RWBuffer::read1bytes() {
    return *_head++;
}

int16_t RWBuffer::read2bytes() {
    int16_t value;
    char* pp = (char*)&value;
    pp[1] = *_head++;
    pp[0] = *_head++;

    return value;
}

uint16_t RWBuffer::readChar2bytes(char* buf) {
    uint16_t value=0;
    char* pp = (char*)&value;
    pp[1] = buf[0];
    pp[0] = buf[1];

    return value;
}

int16_t RWBuffer::readLe2bytes() {
    int16_t value;
    char* pp = (char*)&value;
    pp[1] = *_head++;
    pp[0] = *_head++;

    return value;
}

int32_t RWBuffer::read3bytes() {
    int32_t value = 0x00;
    char* pp = (char*)&value;
    pp[2] = *_head++;
    pp[1] = *_head++;
    pp[0] = *_head++;

    return value;
}

int32_t RWBuffer::readLe3bytes() {
    int32_t value = 0x00;
    char* pp = (char*)&value;
    pp[0] = *_head++;
    pp[1] = *_head++;
    pp[2] = *_head++;

    return value;
}

int32_t RWBuffer::read4bytes() {
    int32_t value=0;
    char* pp = (char*)&value;
    pp[3] = *_head++;
    pp[2] = *_head++;
    pp[1] = *_head++;
    pp[0] = *_head++;

    return value;
}

uint32_t RWBuffer::readChar4bytes(char* buf) {
    uint32_t value;
    char* pp = (char*)&value;
    pp[3] = buf[0];
    pp[2] = buf[1];
    pp[1] = buf[2];
    pp[0] = buf[3];

    return value;
}

int32_t RWBuffer::readLe4bytes() {
    int32_t value;
    char* pp = (char*)&value;
    pp[0] = *_head++;
    pp[1] = *_head++;
    pp[2] = *_head++;
    pp[3] = *_head++;

    return value;
}

int64_t RWBuffer::read8bytes() {
    int64_t value;
    char* pp = (char*)&value;
    pp[7] = *_head++;
    pp[6] = *_head++;
    pp[5] = *_head++;
    pp[4] = *_head++;
    pp[3] = *_head++;
    pp[2] = *_head++;
    pp[1] = *_head++;
    pp[0] = *_head++;

    return value;
}

int64_t RWBuffer::readLe8bytes() {
    int64_t value;
    char* pp = (char*)&value;
    pp[0] = *_head++;
    pp[1] = *_head++;
    pp[2] = *_head++;
    pp[3] = *_head++;
    pp[4] = *_head++;
    pp[5] = *_head++;
    pp[6] = *_head++;
    pp[7] = *_head++;

    return value;
}

void RWBuffer::readBytes(char* data, int32_t size) {
    memcpy(data, _head, size);
    _head += size;
}

void RWBuffer::write1bytes(char value) {
    *_head++ = value;
}

void RWBuffer::write2bytes(int16_t value) {
    char* pp = (char*)&value;
    *_head++ = pp[1];
    *_head++ = pp[0];
}

void RWBuffer::writeLe2bytes(int16_t value) {
    char* pp = (char*)&value;
    *_head++ = pp[0];
    *_head++ = pp[1];
}

void RWBuffer::write4bytes(int32_t value) {
    char* pp = (char*)&value;
    *_head++ = pp[3];
    *_head++ = pp[2];
    *_head++ = pp[1];
    *_head++ = pp[0];
}

void RWBuffer::writeLe4bytes(int32_t value) {
    char* pp = (char*)&value;
    *_head++ = pp[0];
    *_head++ = pp[1];
    *_head++ = pp[2];
    *_head++ = pp[3];
}

void RWBuffer::write3bytes(int32_t value) {
    char* pp = (char*)&value;
    *_head++ = pp[2];
    *_head++ = pp[1];
    *_head++ = pp[0];
}

void RWBuffer::writeLe3bytes(int32_t value) {
    char* pp = (char*)&value;
    *_head++ = pp[0];
    *_head++ = pp[1];
    *_head++ = pp[2];
}

void RWBuffer::write8bytes(int64_t value) {
    char* pp = (char*)&value;
    *_head++ = pp[7];
    *_head++ = pp[6];
    *_head++ = pp[5];
    *_head++ = pp[4];
    *_head++ = pp[3];
    *_head++ = pp[2];
    *_head++ = pp[1];
    *_head++ = pp[0];
}

void RWBuffer::writeLe8bytes(int64_t value) {
    char* pp = (char*)&value;
    *_head++ = pp[0];
    *_head++ = pp[1];
    *_head++ = pp[2];
    *_head++ = pp[3];
    *_head++ = pp[4];
    *_head++ = pp[5];
    *_head++ = pp[6];
    *_head++ = pp[7];
}

void RWBuffer::writeBytes(const char* data, int32_t size) {
    memcpy(_head, data, size);
    _head += size;
}

void RWBuffer::writeCString(const char* data) {
	int32_t datasize=strlen(data);
    memcpy(_head, data, datasize);
    _head += datasize;
}

}