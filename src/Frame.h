#ifndef FRTC_FRAME_H
#define FRTC_FRAME_H

#include <memory>
#include <vector>
#include "MediaCodec.h"

namespace frtc {

class FrameBuffer {
public:
    char* data();

    int32_t size();

    bool empty();

    void append(char);

    void append(const char*, int32_t);

    void assign(const char*, int32_t);

    void clear();

private:
    std::vector<char> _buffer;
};

typedef std::shared_ptr<FrameBuffer> FrameBufferPtr;

class Frame : public MediaCodec {
public:
    virtual ~Frame() = default;

    virtual char* data() = 0;

    virtual int32_t size() = 0;

    virtual int32_t prefix() = 0;

    virtual uint64_t dts() = 0;

    virtual uint64_t pts() = 0;

    virtual bool keyFrame() = 0;

    virtual bool configFrame() = 0;

    virtual bool cacheAble();

    virtual bool dropAble();

    virtual bool decodeAble();
};

typedef std::shared_ptr<Frame> FramePtr;


/**
 * 通过Frame接口包装指针，方便使用者把自己的数据快速包装成frame
 */
class FrameFromPtr : public Frame {
public:
    ~FrameFromPtr() override = default;
    
    FrameFromPtr(
        CodecId codec_id, char* ptr, size_t size, uint64_t dts, uint64_t pts = 0, size_t prefix_size = 0,
        bool is_key = false)
        : FrameFromPtr(ptr, size, dts, pts, prefix_size, is_key) {
        _codec_id = codec_id;
    }

    FrameFromPtr(char* ptr, size_t size, uint64_t dts, uint64_t pts = 0, size_t prefix_size = 0, bool is_key = false) {
        _ptr = ptr;
        _size = size;
        _dts = dts;
        _pts = pts;
        _prefix_size = prefix_size;
        _is_key = is_key;
    }

    char* data() override { return _ptr; }
    int32_t size() override { return _size; }
    uint64_t dts() override { return _dts; }
    uint64_t pts() override { return _pts ? _pts : dts(); }
    int32_t prefix() override { return _prefix_size; }
    bool cacheAble() override { return false; }
    bool keyFrame() override { return _is_key; }
    bool configFrame() override { return false; }
    void setCodecId(CodecId codec_id) { _codec_id = codec_id; }
    CodecId codecId() override { return _codec_id;}

protected:
    FrameFromPtr() = default;

protected:
    bool _is_key;
    char* _ptr;
    uint64_t _dts;
    uint64_t _pts = 0;
    int32_t _size;
    int32_t _prefix_size;
    CodecId _codec_id;
};

class FrameImp : public Frame {
public:
    ~FrameImp() override = default;
    
    FrameImp() {
        _buffer = std::make_shared<FrameBuffer>();
    } 
    
    FrameImp(
        CodecId codec_id, char* ptr, size_t size, uint64_t dts, uint64_t pts = 0, size_t prefix_size = 0,
        bool is_key = false)
        : FrameImp(ptr, size, dts, pts, prefix_size, is_key) {
        _codec_id = codec_id;
    }

    FrameImp(char* ptr, size_t size, uint64_t dts, uint64_t pts = 0, size_t prefix_size = 0, bool is_key = false) {
        _buffer = std::make_shared<FrameBuffer>();
        _buffer->append(ptr, size);
        _dts = dts;
        _pts = pts;
        _prefix_size = prefix_size;
        _isKey = is_key;
    }

    template <typename C = FrameImp>
    static std::shared_ptr<C> create() {
#if 0
        static ResourcePool<C> packet_pool;
        static onceToken token([]() {
            packet_pool.setSize(1024);
        });
        auto ret = packet_pool.obtain2();
        ret->_buffer.clear();
        ret->_prefix_size = 0;
        ret->_dts = 0;
        ret->_pts = 0;
        return ret;
#else
        return std::shared_ptr<C>(new C());
#endif
    }

    char* data() override { return _buffer->data(); }
    int32_t size() override { return _buffer->size(); }
    uint64_t dts() override { return _dts; }
    uint64_t pts() override { return _pts ? _pts : dts(); }
    int32_t prefix() override { return _prefix_size; }
    bool cacheAble() override { return false; }
    bool keyFrame() override { return _isKey; }
    bool configFrame() override { return _isConfig; }
    void setCodecId(CodecId codec_id) { _codec_id = codec_id; }
    CodecId codecId() override { return _codec_id;}
    MediaType mediaType() override { return _type; }

public:
    bool _isKey;
    bool _isConfig;
    FrameBufferPtr _buffer;
    uint64_t _dts;
    uint64_t _pts = 0;
    int32_t _prefix_size;
    CodecId _codec_id;
    MediaType _type;
};

typedef std::shared_ptr<FrameImp> FrameImpPtr; 

}

#endif