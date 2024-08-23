#include "H264Rtp.h"

namespace frtc {

#pragma pack(push, 1)

class FuFlags {
public:
#if __BYTE_ORDER == __BIG_ENDIAN
    unsigned start_bit: 1;
    unsigned end_bit: 1;
    unsigned reserved: 1;
    unsigned nal_type: 5;
#else
    unsigned nal_type: 5;
    unsigned reserved: 1;
    unsigned end_bit: 1;
    unsigned start_bit: 1;
#endif
};

#pragma pack(pop)

H264RtpDecoder::H264RtpDecoder() {
    _frame = obtainFrame();
}

H264Frame::Ptr H264RtpDecoder::obtainFrame() {
    auto frame = FrameImp::create<H264Frame>();
    frame->_prefix_size = 4;
    frame->_type = MediaType::video;
    frame->_codec_id = CodecId::h264;

    return frame;
}

bool H264RtpDecoder::inputRtp(RtpPacket::Ptr rtp, bool key_pos) {
    auto seq = rtp->getSeq();
    auto last_is_gop = _is_gop;
    _is_gop = decodeRtp(rtp);
    if (!_gop_dropped && seq != (uint16_t)(_last_seq + 1) && _last_seq) {
        _gop_dropped = true;
        //WarnL << "start drop h264 gop, last seq:" << _last_seq << ", rtp:\r\n" << rtp->dumpString();
    }
    _last_seq = seq;
    // 确保有sps rtp的时候，gop从sps开始；否则从关键帧开始
    return _is_gop && !last_is_gop;
}

/*
RTF3984 5.2节  Common Structure of the RTP Payload Format
Table 1.  Summary of NAL unit types and their payload structures

   Type   Packet    Type name                        Section
   ---------------------------------------------------------
   0      undefined                                    -
   1-23   NAL unit  Single NAL unit packet per H.264   5.6
   24     STAP-A    Single-time aggregation packet     5.7.1
   25     STAP-B    Single-time aggregation packet     5.7.1
   26     MTAP16    Multi-time aggregation packet      5.7.2
   27     MTAP24    Multi-time aggregation packet      5.7.2
   28     FU-A      Fragmentation unit                 5.8
   29     FU-B      Fragmentation unit                 5.8
   30-31  undefined                                    -
*/

bool H264RtpDecoder::singleFrame(const RtpPacket::Ptr &rtp, const uint8_t *ptr, ssize_t size, uint64_t stamp){
    _frame->_buffer->assign("\x00\x00\x00\x01", 4);
    _frame->_buffer->append((char *) ptr, size);
    _frame->_pts = stamp;
    auto key = _frame->keyFrame() || _frame->configFrame();
    outputFrame(rtp, _frame);
    return key;
}

bool H264RtpDecoder::unpackStapA(const RtpPacket::Ptr &rtp, const uint8_t *ptr, ssize_t size, uint64_t stamp) {
    //STAP-A 单一时间的组合包
    auto have_key_frame = false;
    auto end = ptr + size;
    while (ptr + 2 < end) {
        uint16_t len = (ptr[0] << 8) | ptr[1];
        if (!len || ptr + len > end) {
            //WarnL << "invalid rtp data size:" << len << ",rtp:\r\n" << rtp->dumpString();
            _gop_dropped = true;
            break;
        }
        ptr += 2;
        if (singleFrame(rtp, ptr, len, stamp)) {
            have_key_frame = true;
        }
        ptr += len;
    }
    return have_key_frame;
}

bool H264RtpDecoder::mergeFu(const RtpPacket::Ptr &rtp, const uint8_t *ptr, ssize_t size, uint64_t stamp, uint16_t seq){
    auto nal_suffix = *ptr & (~0x1F);
    FuFlags *fu = (FuFlags *) (ptr + 1);
    if (fu->start_bit) {
        //该帧的第一个rtp包
        _frame->_buffer->assign("\x00\x00\x00\x01", 4);
        _frame->_buffer->append(nal_suffix | fu->nal_type);
        _frame->_pts = stamp;
        _fu_dropped = false;
    }

    if (_fu_dropped) {
        //该帧不完整
        return false;
    }

    if (!fu->start_bit && seq != (uint16_t) (_last_seq + 1)) {
        //中间的或末尾的rtp包，其seq必须连续，否则说明rtp丢包，那么该帧不完整，必须得丢弃
        _fu_dropped = true;
        _frame->_buffer->clear();
        return false;
    }

    //后面追加数据
    _frame->_buffer->append((char*) ptr + 2, size - 2);
    if (!fu->end_bit) {
        //非末尾包
        return fu->start_bit ? (_frame->keyFrame() || _frame->configFrame()) : false;
    }

    //确保下一次fu必须收到第一个包
    _fu_dropped = true;
    //该帧最后一个rtp包,输出frame
    outputFrame(rtp, _frame);
    return false;
}

bool H264RtpDecoder::decodeRtp(const RtpPacket::Ptr &rtp) {
    auto payload_size = rtp->getPayloadSize();
    if (payload_size <= 0) {
        //无实际负载
        return false;
    }
    auto frame = rtp->getPayload();
    auto stamp = rtp->getStampMS();
    auto seq = rtp->getSeq();
    int nal = H264_TYPE(frame[0]);

    switch (nal) {
        case 24:
            // 24 STAP-A Single-time aggregation packet 5.7.1
            return unpackStapA(rtp, frame + 1, payload_size - 1, stamp);

        case 28:
            // 28 FU-A Fragmentation unit
            return mergeFu(rtp, frame, payload_size, stamp, seq);

        default: {
            if (nal < 24) {
                //Single NAL Unit Packets
                return singleFrame(rtp, frame, payload_size, stamp);
            }
            _gop_dropped = true;
            //WarnL << "不支持该类型的264 RTP包, nal type:" << nal << ", rtp:\r\n" << rtp->dumpString();
            return false;
        }
    }
}

void H264RtpDecoder::outputFrame(const RtpPacket::Ptr &rtp, const H264Frame::Ptr &frame) {
    if (frame->dropAble()) {
        //不参与dts生成
        frame->_dts = frame->_pts;
    } else {
        //rtsp没有dts，那么根据pts排序算法生成dts
        _dts_generator.getDts(frame->_pts, frame->_dts);
    }

    if (frame->keyFrame() && _gop_dropped) {
        _gop_dropped = false;
        //InfoL << "new gop received, rtp:\r\n" << rtp->dumpString();
    }
    if (!_gop_dropped) {
        //RtpCodec::inputFrame(frame);
        RtpDecoder::outputFrame(frame); 
    }
    _frame = obtainFrame();
}

}
