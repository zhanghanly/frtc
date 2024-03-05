#include <cstring>
#include "Buffer.h"

namespace frtc {

void Buffer::init(char* b, int32_t nn){
	_head = _data = b;
	_size = nn;
}

void Buffer::destroy(){
}

int32_t Buffer::pos() {
    return (int)(_head - _data);
}
    
char* Buffer::data() {
    return _data;
}
    
int32_t Buffer::size() {
    return _size;
}

int32_t Buffer::left() {
    return _size - (int)(_head - _data);
}

int32_t Buffer::empty() {
    return !_data || (_head >= _data + _size);
}

int32_t Buffer::require(int32_t required_size) {
    if (required_size < 0) {
        return 0;
    }

    return required_size <= _size - (_head - _data);
}

void Buffer::skip(int32_t size) {
    _head += size;
}

char Buffer::read1bytes() {
    return *_head++;
}

int16_t Buffer::read2bytes() {
    int16_t value;
    char* pp = (char*)&value;
    pp[1] = *_head++;
    pp[0] = *_head++;

    return value;
}

uint16_t Buffer::readChar2bytes(char* buf) {
    uint16_t value=0;
    char* pp = (char*)&value;
    pp[1] = buf[0];
    pp[0] = buf[1];

    return value;
}

int16_t Buffer::readLe2bytes() {
    int16_t value;
    char* pp = (char*)&value;
    pp[1] = *_head++;
    pp[0] = *_head++;

    return value;
}

int32_t Buffer::read3bytes() {
    int32_t value = 0x00;
    char* pp = (char*)&value;
    pp[2] = *_head++;
    pp[1] = *_head++;
    pp[0] = *_head++;

    return value;
}

int32_t Buffer::readLe3bytes() {
    int32_t value = 0x00;
    char* pp = (char*)&value;
    pp[0] = *_head++;
    pp[1] = *_head++;
    pp[2] = *_head++;

    return value;
}

int32_t Buffer::read4bytes() {
    int32_t value=0;
    char* pp = (char*)&value;
    pp[3] = *_head++;
    pp[2] = *_head++;
    pp[1] = *_head++;
    pp[0] = *_head++;

    return value;
}

uint32_t Buffer::readChar4bytes(char* buf) {
    uint32_t value;
    char* pp = (char*)&value;
    pp[3] = buf[0];
    pp[2] = buf[1];
    pp[1] = buf[2];
    pp[0] = buf[3];

    return value;
}

int32_t Buffer::readLe4bytes() {
    int32_t value;
    char* pp = (char*)&value;
    pp[0] = *_head++;
    pp[1] = *_head++;
    pp[2] = *_head++;
    pp[3] = *_head++;

    return value;
}

int64_t Buffer::read8bytes() {
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

int64_t Buffer::readLe8bytes() {
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

void Buffer::readBytes(char* data, int32_t size) {
    memcpy(data, _head, size);
    _head += size;
}

void Buffer::write1bytes(char value) {
    *_head++ = value;
}

void Buffer::write2bytes(int16_t value) {
    char* pp = (char*)&value;
    *_head++ = pp[1];
    *_head++ = pp[0];
}

void Buffer::writeLe2bytes(int16_t value) {
    char* pp = (char*)&value;
    *_head++ = pp[0];
    *_head++ = pp[1];
}

void Buffer::write4bytes(int32_t value) {
    char* pp = (char*)&value;
    *_head++ = pp[3];
    *_head++ = pp[2];
    *_head++ = pp[1];
    *_head++ = pp[0];
}

void Buffer::writeLe4bytes(int32_t value) {
    char* pp = (char*)&value;
    *_head++ = pp[0];
    *_head++ = pp[1];
    *_head++ = pp[2];
    *_head++ = pp[3];
}

void Buffer::write3bytes(int32_t value) {
    char* pp = (char*)&value;
    *_head++ = pp[2];
    *_head++ = pp[1];
    *_head++ = pp[0];
}

void Buffer::writeLe3bytes(int32_t value) {
    char* pp = (char*)&value;
    *_head++ = pp[0];
    *_head++ = pp[1];
    *_head++ = pp[2];
}

void Buffer::write8bytes(int64_t value) {
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

void Buffer::writeLe8bytes(int64_t value) {
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

void Buffer::writeBytes(const char* data, int32_t size) {
    memcpy(_head, data, size);
    _head += size;
}

void Buffer::writeCString(const char* data) {
	int32_t datasize=strlen(data);
    memcpy(_head, data, datasize);
    _head += datasize;
}

}