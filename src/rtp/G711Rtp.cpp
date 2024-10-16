#include "G711Rtp.h"
#include "base/Log.h"

namespace frtc {

    G711RtpDecoder::G711RtpDecoder(CodecId id, size_t max_frame_size) {
        _codec = id;
        _max_frame_size = max_frame_size;
        obtainFrame();
    }

    bool G711RtpDecoder::inputRtp(RtpPacket::Ptr rtp, bool key_pos) {
        auto payload_size = rtp->getPayloadSize();
        if (payload_size <= 0) {
            //无实际负载
            return false;
        }

        auto payload = rtp->getPayload();
        auto stamp = rtp->getStamp();
        auto seq = rtp->getSeq();
        if (_last_stamp != stamp || _frame->_buffer->size() > _max_frame_size) {
            //时间戳发生变化或者缓存超过MAX_FRAME_SIZE，则清空上帧数据
            if (!_frame->_buffer->empty()) {
                //有有效帧，则输出
                //RtpCodec::inputFrame(_frame);
                RtpDecoder::outputFrame(_frame);
            }

            //新的一帧数据
            obtainFrame();
            _frame->_dts = rtp->getStampMS();
            _last_stamp = stamp;
            _drop_flag = false;
        } else if (_last_seq != 0 && (uint16_t)(_last_seq + 1) != seq) {
            //时间戳未发生变化，但是seq却不连续，说明中间rtp丢包了，那么整帧应该废弃
            LOGI("rtp is lost: %d -> %d", _last_seq, seq);
            _drop_flag = true;
            _frame->_buffer->clear();
        }

        if (!_drop_flag) {
            _frame->_buffer->append((char*)payload, payload_size);
        }

        _last_seq = seq;
        return false;
    }

    void G711RtpDecoder::obtainFrame() {
        _frame = FrameImp::create();
        _frame->_codec_id = _codec;
        _frame->_type = MediaType::audio;
    }

}