#include "MediaCodec.h"

namespace frtc {

std::string getMediaType(MediaType type) {
    switch (type) {
    case MediaType::video:
        return "video";    
    case MediaType::audio:
        return "audio";
    default:
        return "none";
    }
}

std::string getCodecName(CodecId id) {
    switch (id) {
    case CodecId::h264:
        return "h264";
    case CodecId::h265:
        return "h265";
    case CodecId::g711a:
        return "g711a";
    case CodecId::g711u:
        return "g711u";
    case CodecId::opus:
        return "opus";
    case CodecId::aac:
        return "aac";
    default:
        return "none";
    }
}

CodecId getCodecId(std::string& name) {
    if (name == "h264" || name == "H264") {
        return CodecId::h264;
    } else if (name == "h265" || name == "H265") {
        return CodecId::h265;
    } else if (name == "pcma" || name == "PCMA") {
        return CodecId::g711a;
    } else if (name == "pcmu" || name == "PCMU") {
        return CodecId::g711u;
    } else if (name == "aac" || name == "AAC") {
        return CodecId::aac;
    } else if (name == "opus" || name == "OPUS") {
        return CodecId::opus;
    } else {
        return CodecId::none;
    }
}

}