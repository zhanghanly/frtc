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

}