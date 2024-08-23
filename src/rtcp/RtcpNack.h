#ifndef FRTC_RTCP_NACK_H
#define FRTC_RTCP_NACK_H

#include <set>
#include <map>
#include <deque>
#include <functional>
#include <unordered_map>
#include "RtcpFCI.h"
#include "rtp/RtpPacket.h"

namespace frtc {

class NackList {
public:
    void pushBack(RtpPacket::Ptr rtp);
    void forEach(const FCI_NACK &nack, const std::function<void(const RtpPacket::Ptr &rtp)> &cb);

private:
    void popFront();
    uint32_t getCacheMS();
    int64_t getRtpStamp(uint16_t seq);
    RtpPacket::Ptr *getRtp(uint16_t seq);

private:
    uint32_t _cache_ms_check = 0;
    std::deque<uint16_t> _nack_cache_seq;
    std::unordered_map<uint16_t, RtpPacket::Ptr> _nack_cache_pkt;
};

class NackContext {
public:
    using Ptr = std::shared_ptr<NackContext>;
    using onNack = std::function<void(const FCI_NACK &nack)>;

    NackContext();

    void received(uint16_t seq, bool is_rtx = false);
    void setOnNack(onNack cb);
    uint64_t reSendNack();

private:
    void eraseFrontSeq();
    void doNack(const FCI_NACK &nack, bool record_nack);
    void recordNack(const FCI_NACK &nack);
    void clearNackStatus(uint16_t seq);
    void makeNack(uint16_t max, bool flush = false);

private:
    bool _started = false;
    int _rtt = 50;
    onNack _cb;
    std::set<uint16_t> _seq;
    // 最新nack包中的rtp seq值
    uint16_t _nack_seq = 0;

    struct NackStatus {
        uint64_t first_stamp;
        uint64_t update_stamp;
        int nack_count = 0;
    };
    std::map<uint16_t /*seq*/, NackStatus> _nack_send_status;
};

}

#endif