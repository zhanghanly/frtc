#ifndef FRTC_G711_RTP_H
#define FRTC_G711_RTP_H

#include <memory>
#include "RtpDecoder.h"
#include "Frame.h"

namespace frtc {

class G711RtpDecoder : public RtpDecoder {
public:
    using Ptr = std::shared_ptr<G711RtpDecoder>;

    ~G711RtpDecoder() override = default;

    /**
     * 构造函数
     * @param codec 编码id
     * @param max_frame_size 允许的最大帧大小
     */
    G711RtpDecoder(CodecId id, size_t max_frame_size = 2 * 1024);

    /**
     * 输入rtp并解码
     * @param rtp rtp数据包
     * @param key_pos 此参数内部强制转换为false,请忽略之
     */
    bool inputRtp(RtpPacket::Ptr rtp, bool key_pos = false) override;

private:
    void obtainFrame();

private:
    bool _drop_flag = false;
    uint16_t _last_seq = 0;
    uint64_t _last_stamp = 0;
    size_t _max_frame_size;
    CodecId _codec;
    FrameImpPtr _frame;
};

}

#endif