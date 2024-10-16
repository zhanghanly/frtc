#ifndef FRTC_TWCC_CONTEXT_H
#define FRTC_TWCC_CONTEXT_H

#include <stdint.h>
#include <map>
#include <functional>
#include <string>

namespace frtc {

class TwccContext {
public:
    using onSendTwccCB = std::function<void(uint32_t ssrc, std::string fci)>;
    //每个twcc rtcp包最多表明的rtp ext seq增量
    static constexpr size_t kMaxSeqSize = 20;
    //每个twcc rtcp包发送的最大时间间隔，单位毫秒
    static constexpr size_t kMaxTimeDelta = 256;

    void onRtp(uint32_t ssrc, uint16_t twcc_ext_seq, uint64_t stamp_ms);
    void setOnSendTwccCB(onSendTwccCB cb);

private:
    void onSendTwcc(uint32_t ssrc);
    bool needSendTwcc() const;
    int checkSeqStatus(uint16_t twcc_ext_seq) const;
    void clearStatus();

private:
    uint64_t _min_stamp = 0;
    uint64_t _max_stamp;
     /*twcc_ext_seq -> recv time in ms*/
    std::map<uint32_t, uint64_t> _rtp_recv_status;
    uint8_t _twcc_pkt_count = 0;
    onSendTwccCB _cb;
};

}

#endif
