#ifndef FRTC_RTP_PACKET_H
#define FRTC_RTP_PACKET_H

#include <cstdint>
#include <string>
#include "RawBuffer.h"
#include "MediaCodec.h"

namespace frtc {

class RtpHeader {
public:
#if __BYTE_ORDER == __BIG_ENDIAN
    //版本号，固定为2
    uint32_t version: 2;
    //padding
    uint32_t padding: 1;
    //扩展
    uint32_t ext: 1;
    //csrc
    uint32_t csrc: 4;
    //mark
    uint32_t mark: 1;
    //负载类型
    uint32_t pt: 7;
#else
    //csrc
    uint32_t csrc: 4;
    //扩展
    uint32_t ext: 1;
    //padding
    uint32_t padding: 1;
    //版本号，固定为2
    uint32_t version: 2;
    //负载类型
    uint32_t pt: 7;
    //mark
    uint32_t mark: 1;
#endif
    //序列号
    uint32_t seq: 16;
    //时间戳
    uint32_t stamp;
    //ssrc
    uint32_t ssrc;
    //负载，如果有csrc和ext，前面为 4 * csrc + (4 + 4 * ext_len)
    uint8_t payload;

public:
    //返回csrc字段字节长度
    size_t getCsrcSize() const;
    //返回csrc字段首地址，不存在时返回nullptr
    uint8_t* getCsrcData();

    //返回ext字段字节长度
    size_t getExtSize() const;
    //返回ext reserved值
    uint16_t getExtReserved() const;
    //返回ext段首地址，不存在时返回nullptr
    uint8_t* getExtData();

    //返回有效负载指针,跳过csrc、ext
    uint8_t* getPayloadData();
    //返回有效负载总长度,不包括csrc、ext、padding
    ssize_t getPayloadSize(size_t rtp_size) const;
    //打印调试信息
    std::string dumpString(size_t rtp_size) const;

private:
    //返回有效负载偏移量
    size_t getPayloadOffset() const;
    //返回padding长度
    size_t getPaddingSize(size_t rtp_size) const;
};

// 此rtp为rtp over tcp形式，需要忽略前4个字节
class RtpPacket : public BufferRaw {
public:
    using Ptr = std::shared_ptr<RtpPacket>;
    enum { kRtpVersion = 2, kRtpHeaderSize = 12, kRtpTcpHeaderSize = 4 };

    // 获取rtp头
    RtpHeader* getHeader();
    const RtpHeader* getHeader() const;

    // 打印调试信息
    std::string dumpString() const;

    // 主机字节序的seq
    uint16_t getSeq() const;
    uint32_t getStamp() const;
    // 主机字节序的时间戳，已经转换为毫秒
    uint64_t getStampMS(bool ntp = true) const;
    // 主机字节序的ssrc
    uint32_t getSSRC() const;
    // 有效负载，跳过csrc、ext
    uint8_t* getPayload();
    // 有效负载长度，不包括csrc、ext、padding
    size_t getPayloadSize() const;

    // 音视频类型
    MediaType type;
    // 音频为采样率，视频一般为90000
    uint32_t sample_rate;
    // ntp时间戳
    uint64_t ntp_stamp;

    int track_index;

    static Ptr create();

private:
    RtpPacket() = default;
};

}

#endif