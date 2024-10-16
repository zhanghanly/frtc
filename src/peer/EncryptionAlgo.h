#ifndef FRTC_ENCRYPTION_ALGO_H
#define FRTC_ENCRYPTION_ALGO_H

#include <cstdint>

namespace frtc {

int32_t hmac_encode(const char* algo, const char* key, const int32_t key_length,
                    const char* input, const int32_t input_length, char* output,
                    uint32_t* output_length);


uint32_t crc32_ieee(const void* buf, int32_t size, uint32_t previous);

}

#endif