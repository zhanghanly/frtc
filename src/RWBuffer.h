#ifndef FRTC_RW_BUFFER_H
#define FRTC_RW_BUFFER_H

#include <cstdint>
#include <memory>

namespace frtc {

class RWBuffer {
public:
    void init(char* b, int32_t nn);
    void destroy();
    int32_t pos();
    char* data();
    int32_t size();
    // Left bytes in buffer, total size() minus the current pos().
    int32_t left();
    // Whether buffer is empty.
    int32_t empty();
    // Whether buffer is able to supply required size of bytes.
    // @remark User should check buffer by require then do read/write.
    // @remark Assert the required_size is not negative.
    int32_t require(int32_t required_size);
    void skip(int32_t size);
    // Write 1bytes char to buffer.
    void write1bytes(char value);
    // Write 2bytes int32_t to buffer.
    void write2bytes(int16_t value);
    void writeLe2bytes(int16_t value);
    // Write 4bytes int32_t to buffer.
    void write4bytes(int32_t value);
    void writeLe4bytes(int32_t value);
    // Write 3bytes int32_t to buffer.
    void write3bytes(int32_t value);
    void writeLe3bytes(int32_t value);
    // Write 8bytes int32_t to buffer.
    void write8bytes(int64_t value);
    void writeLe8bytes(int64_t value);
    // Write string to buffer
    //void write_string(Buffer* buf,std::string value);
    // Write bytes to buffer
    void writeBytes(const char* data, int32_t size);
    void writeCString(const char* data);
    // Read 1bytes char from buffer.
    char read1bytes();
    // Read 2bytes int32_t from buffer.
    int16_t read2bytes();
    int16_t readLe2bytes();
    // Read 3bytes int32_t from buffer.
    int32_t read3bytes();
    int32_t readLe3bytes();
    // Read 4bytes int32_t from buffer.
    int32_t read4bytes();
    int32_t readLe4bytes();
    // Read 8bytes int32_t from buffer.
    int64_t read8bytes();
    int64_t readLe8bytes();
    // Read bytes from buffer, length specifies by param len.
    void readBytes(char* data, int32_t size);

    uint16_t readChar2bytes(char* buf);
    uint32_t readChar4bytes(char* buf);

private:
    // current position at bytes.
    char* _head;
    // the bytes data for buffer to read or write.
    char* _data;
    // the total number of bytes.
    int32_t _size;
};

typedef std::shared_ptr<RWBuffer> RWBufferSp;

}

#endif