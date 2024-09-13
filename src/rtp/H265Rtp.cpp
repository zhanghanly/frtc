#include <iostream>
#include "H265Rtp.h"
#include "Log.h"

namespace frtc {

//https://datatracker.ietf.org/doc/rfc7798/
//H265 nalu 头两个字节的定义
/*
 0               1
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |F|    Type   |  LayerId  | TID |
 +-------------+-----------------+
 Forbidden zero(F) : 1 bit
 NAL unit type(Type) : 6 bits
 NUH layer ID(LayerId) : 6 bits
 NUH temporal ID plus 1 (TID) : 3 bits
*/

H265RtpDecoder::H265RtpDecoder() {
    _frame = obtainFrame();
}

H265Frame::Ptr H265RtpDecoder::obtainFrame() {
    auto frame = FrameImp::create<H265Frame>();
    frame->_prefix_size = 4;
    frame->_type = MediaType::video;
    frame->_codec_id = CodecId::h265;
    
    return frame;
}

#define AV_RB16(x)                           \
    ((((const uint8_t*)(x))[0] << 8) |          \
      ((const uint8_t*)(x))[1])

#define CHECK_SIZE(total, size, ret) \
        if (total < size) {     \
            std::cout << "invalid rtp data size:" << total << " < " << size << ",rtp:\r\n" << rtp->dumpString(); _gop_dropped = true;  return ret; \ 
        }

// 4.4.2. Aggregation Packets (APs) (p25)
/*
 0               1               2               3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          RTP Header                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|      PayloadHdr (Type=48)     |           NALU 1 DONL         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|           NALU 1 Size         |            NALU 1 HDR         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
|                         NALU 1 Data . . .                     |
|                                                               |
+     . . .     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|               |  NALU 2 DOND  |            NALU 2 Size        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|          NALU 2 HDR           |                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+            NALU 2 Data        |
|                                                               |
|         . . .                 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                               :    ...OPTIONAL RTP padding    |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
bool H265RtpDecoder::unpackAp(const RtpPacket::Ptr &rtp, const uint8_t *ptr, ssize_t size, uint64_t stamp){
    bool have_key_frame = false;
    //忽略PayloadHdr
    CHECK_SIZE(size, 2, have_key_frame);
    ptr += 2;
    size -= 2;

    while (size) {
        if (_using_donl_field) {
            CHECK_SIZE(size, 2, have_key_frame);
            uint16_t donl = AV_RB16(ptr);
            size -= 2;
            ptr += 2;
        }
        CHECK_SIZE(size, 2, have_key_frame);
        uint16_t nalu_size = AV_RB16(ptr);
        size -= 2;
        ptr += 2;
        CHECK_SIZE(size, nalu_size, have_key_frame)
        if (singleFrame(rtp, ptr, nalu_size, stamp)) {
            have_key_frame = true;
        }
        size -= nalu_size;
        ptr += nalu_size;
    }
    return have_key_frame;
}

// 4.4.3. Fragmentation Units (p29)
/*
 0               1               2               3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|     PayloadHdr (Type=49)      |    FU header  |  DONL (cond)  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-|
|  DONL (cond)  |                                               |
|-+-+-+-+-+-+-+-+                                               |
|                           FU payload                          |
|                                                               |
|                               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                               :    ...OPTIONAL RTP padding    |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

+---------------+
|0|1|2|3|4|5|6|7|
+-+-+-+-+-+-+-+-+
|S|E|   FuType  |
+---------------+
*/

bool H265RtpDecoder::mergeFu(const RtpPacket::Ptr &rtp, const uint8_t *ptr, ssize_t size, uint64_t stamp, uint16_t seq){
    //CHECK_SIZE(size, 4, false);
    auto s_bit = ptr[2] >> 7;
    auto e_bit = (ptr[2] >> 6) & 0x01;
    auto type = ptr[2] & 0x3f;
    if (s_bit) {
        //该帧的第一个rtp包
        _frame->_buffer->assign("\x00\x00\x00\x01", 4);
        _frame->_buffer->append((type << 1) | (ptr[0] & 0x81));
        _frame->_buffer->append(ptr[1]);
        _frame->_pts = stamp;
        _fu_dropped = false;
    }

    if (_fu_dropped) {
        //该帧不完整
        return false;
    }

    if (!s_bit && seq != (uint16_t) (_last_seq + 1)) {
        //中间的或末尾的rtp包，其seq必须连续，否则说明rtp丢包，那么该帧不完整，必须得丢弃
        _fu_dropped = true;
        _frame->_buffer->clear();
        return false;
    }

    //跳过PayloadHdr +  FU header
    ptr += 3;
    size -= 3;
    if (_using_donl_field) {
        //DONL确保不少于2个字节
        //CHECK_SIZE(size, 2, false);
        uint16_t donl = AV_RB16(ptr);
        size -= 2;
        ptr += 2;
    }

    //CHECK_SIZE(size, 1, false);
    //后面追加数据
    _frame->_buffer->append((char*)ptr, size);

    if (!e_bit) {
        //非末尾包
        return s_bit ? (_frame->keyFrame() || _frame->configFrame()) : false;
    }

    //确保下一次fu必须收到第一个包
    _fu_dropped = true;
    //该帧最后一个rtp包
    outputFrame(rtp, _frame);
    return false;
}

bool H265RtpDecoder::inputRtp(RtpPacket::Ptr rtp, bool) {
    auto seq = rtp->getSeq();
    auto last_is_gop = _is_gop;
    _is_gop = decodeRtp(rtp);
    if (!_gop_dropped && seq != (uint16_t) (_last_seq + 1) && _last_seq) {
        _gop_dropped = true;
        //WarnL << "start drop h265 gop, last seq:" << _last_seq << ", rtp:\r\n" << rtp->dumpString();
    }
    _last_seq = seq;
    // 确保有sps rtp的时候，gop从sps开始；否则从关键帧开始
    return _is_gop && !last_is_gop;
}

bool H265RtpDecoder::decodeRtp(const RtpPacket::Ptr &rtp) {
    auto payload_size = rtp->getPayloadSize();
    if (payload_size <= 0) {
        //无实际负载
        return false;
    }
    auto frame = rtp->getPayload();
    auto stamp = rtp->getStampMS();
    auto seq = rtp->getSeq();
    int nal = H265_TYPE(frame[0]);

    switch (nal) {
        case 48:
            // aggregated packet (AP) - with two or more NAL units
            return unpackAp(rtp, frame, payload_size, stamp);

        case 49:
            // fragmentation unit (FU)
            return mergeFu(rtp, frame, payload_size, stamp, seq);

        default: {
            if (nal < 48) {
                // Single NAL Unit Packets (p24)
                return singleFrame(rtp, frame, payload_size, stamp);
            }
            _gop_dropped = true;
            //WarnL << "不支持该类型的265 RTP包, nal type" << nal << ", rtp:\r\n" << rtp->dumpString();
            return false;
        }
    }
}

bool H265RtpDecoder::singleFrame(const RtpPacket::Ptr &rtp, const uint8_t *ptr, ssize_t size, uint64_t stamp){
    _frame->_buffer->assign("\x00\x00\x00\x01", 4);
    _frame->_buffer->append((char *)ptr, size);
    _frame->_pts = stamp;
    auto key = _frame->keyFrame() || _frame->configFrame();
    outputFrame(rtp, _frame);
    return key;
}

void H265RtpDecoder::outputFrame(const RtpPacket::Ptr &rtp, const H265Frame::Ptr &frame) {
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
         LOGI("%s", "h265 RtpDecoder output video frame"); 
        RtpDecoder::outputFrame(frame);
    }
    _frame = obtainFrame();
}

}
