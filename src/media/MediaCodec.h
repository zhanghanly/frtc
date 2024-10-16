#ifndef FRTC_MEDIA_CODEC_H
#define FRTC_MEDIA_CODEC_H

#include <string>

namespace frtc {

enum class MediaType {
    video,
    audio
};

enum class CodecId {
    h264,
    h265,
    g711a,
    g711u,
    opus,
    aac,
    none
};

class MediaCodec {
public:
    virtual ~MediaCodec() = default;
    
    virtual MediaType mediaType() = 0;

    virtual CodecId codecId() = 0;
};

std::string getMediaType(MediaType);

std::string getCodecName(CodecId);

CodecId getCodecId(std::string&);

}

#endif