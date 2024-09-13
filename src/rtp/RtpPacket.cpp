#include <arpa/inet.h>
#include <sstream>
#include "RtpPacket.h"
#include "Log.h"

namespace frtc {

#define AV_RB16(x) ((((const uint8_t *)(x))[0] << 8) | ((const uint8_t *)(x))[1])

size_t RtpHeader::getCsrcSize() const {
    // 每个csrc占用4字节
    return csrc << 2;
}

uint8_t* RtpHeader::getCsrcData() {
    if (!csrc) {
        return nullptr;
    }
    return &payload;
}

size_t RtpHeader::getExtSize() const {
    // rtp有ext
    if (!ext) {
        return 0;
    }
    auto ext_ptr = &payload + getCsrcSize();
    // uint16_t reserved = AV_RB16(ext_ptr);
    // 每个ext占用4字节
    return AV_RB16(ext_ptr + 2) << 2;
}

uint16_t RtpHeader::getExtReserved() const {
    // rtp有ext
    if (!ext) {
        return 0;
    }
    auto ext_ptr = &payload + getCsrcSize();
    return AV_RB16(ext_ptr);
}

uint8_t* RtpHeader::getExtData() {
    if (!ext) {
        return nullptr;
    }
    auto ext_ptr = &payload + getCsrcSize();
    // 多出的4个字节分别为reserved、ext_len
    return ext_ptr + 4;
}

size_t RtpHeader::getPayloadOffset() const {
    // 有ext时，还需要忽略reserved、ext_len 4个字节
    return getCsrcSize() + (ext ? (4 + getExtSize()) : 0);
}

uint8_t* RtpHeader::getPayloadData() {
    return &payload + getPayloadOffset();
}

size_t RtpHeader::getPaddingSize(size_t rtp_size) const {
    if (!padding) {
        return 0;
    }
    auto end = (uint8_t*)this + rtp_size - 1;
    return *end;
}

ssize_t RtpHeader::getPayloadSize(size_t rtp_size) const {
    auto invalid_size = getPayloadOffset() + getPaddingSize(rtp_size);
    return (ssize_t)rtp_size - invalid_size - RtpPacket::kRtpHeaderSize;
}

std::string RtpHeader::dumpString(size_t rtp_size) const {
    std::stringstream printer;
    printer << "version:" << (int)version << "\r\n";
    printer << "padding:" << getPaddingSize(rtp_size) << "\r\n";
    printer << "ext:" << getExtSize() << "\r\n";
    printer << "csrc:" << getCsrcSize() << "\r\n";
    printer << "mark:" << (int)mark << "\r\n";
    printer << "pt:" << (int)pt << "\r\n";
    printer << "seq:" << ntohs(seq) << "\r\n";
    printer << "stamp:" << ntohl(stamp) << "\r\n";
    printer << "ssrc:" << ntohl(ssrc) << "\r\n";
    printer << "rtp size:" << rtp_size << "\r\n";
    printer << "payload offset:" << getPayloadOffset() << "\r\n";
    printer << "payload size:" << getPayloadSize(rtp_size) << "\r\n";
    
    return printer.str();
}

RtpHeader* RtpPacket::getHeader() {
    // 需除去rtcp over tcp 4个字节长度
    return (RtpHeader*)(data() + RtpPacket::kRtpTcpHeaderSize);
}

const RtpHeader* RtpPacket::getHeader() const {
    return (RtpHeader*)(data() + RtpPacket::kRtpTcpHeaderSize);
}

std::string RtpPacket::dumpString() const {
    return ((RtpPacket*)this)->getHeader()->dumpString(size() - RtpPacket::kRtpTcpHeaderSize);
}

uint16_t RtpPacket::getSeq() const {
    return ntohs(getHeader()->seq);
}

uint32_t RtpPacket::getStamp() const {
    return ntohl(getHeader()->stamp);
}

uint64_t RtpPacket::getStampMS(bool ntp) const {
    LOGI("stamp=%u sample_rate=%u", getStamp(), sample_rate);
    return ntp ? ntp_stamp : getStamp() * uint64_t(1000) / sample_rate;
}

uint32_t RtpPacket::getSSRC() const {
    return ntohl(getHeader()->ssrc);
}

uint8_t* RtpPacket::getPayload() {
    return getHeader()->getPayloadData();
}

size_t RtpPacket::getPayloadSize() const {
    // 需除去rtcp over tcp 4个字节长度
    return getHeader()->getPayloadSize(size() - kRtpTcpHeaderSize);
}

RtpPacket::Ptr RtpPacket::create() {
#if 0
    static ResourcePool<RtpPacket> packet_pool;
    static onceToken token([]() {
        packet_pool.setSize(1024);
    });
    auto ret = packet_pool.obtain2();
    ret->setSize(0);
    return ret;
#else
    return Ptr(new RtpPacket);
#endif
}

}