#include <cstring>
#include <iostream>
#include <openssl/md5.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include "EncryptionAlgo.h"
#include "Log.h"

namespace frtc {

int32_t hmac_encode(const char* algo, const char* key, const int32_t key_length,
                    const char* input, const int32_t input_length, char* output, 
                    uint32_t* output_length) {
    const EVP_MD* engine = NULL;
    if (strcmp(algo , "sha512")==0) {
        engine = EVP_sha512();
    } else if(strcmp(algo , "sha256")==0) {
        engine = EVP_sha256();
    } else if(strcmp(algo ,  "sha1")==0) {
        engine = EVP_sha1();
    } else if(strcmp(algo ,  "md5")==0) {
        engine = EVP_md5();
    } else if(strcmp(algo ,  "sha224")==0) {
        engine = EVP_sha224();
    } else if(strcmp(algo ,  "sha384")==0) {
        engine = EVP_sha384();
    } else {
    	LOGE("ERROR_RTC_STUN unknown algo=%s", algo);
        return -1;
    }

#if !defined(OPENSSL_VERSION_NUMBER) || OPENSSL_VERSION_NUMBER < 0x10100000L
	HMAC_CTX hctx;
	HMAC_CTX_init(&hctx);
    if (&hctx == NULL) {
        LOGE("%s", "hmac init faied");
        return -1;
    }

    if (HMAC_Init_ex(&hctx, key, key_length, engine, NULL) < 0) {
        HMAC_CTX_cleanup(&hctx);
        LOGE("%s", "hmac init faied");
        return -1;
    }

    if (HMAC_Update(&hctx, (const uint8_t*)input, input_length) < 0) {
        HMAC_CTX_cleanup(&hctx);
        LOGE("%s", "hmac update faied");
        return -1;
    }

    if (HMAC_Final(&hctx, (uint8_t*)output, &output_length) < 0) {
        HMAC_CTX_cleanup(&hctx);
        LOGE("%s", "hmac final faied");
        return -1;
    }

    HMAC_CTX_cleanup(&hctx);

#else
    HMAC_CTX* ctx = HMAC_CTX_new();
    if (ctx == NULL) {
        LOGE("%s", "hmac init faied");
        return -1;
    }
    if (HMAC_Init_ex(ctx, key, key_length, engine, NULL) < 0) {
        HMAC_CTX_free(ctx);
        LOGE("%s", "hmac init faied");
        return -1;
    }
    if (HMAC_Update(ctx, (const uint8_t*)input, input_length) < 0) {
        HMAC_CTX_free(ctx);
        LOGE("%s", "hmac update faied");
        return -1;
    }
    if (HMAC_Final(ctx, (uint8_t*)output, output_length) < 0) {
        HMAC_CTX_free(ctx);
        LOGE("%s", "hmac final faied");
        return -1;
    }

    HMAC_CTX_free(ctx);
#endif
    return 0;
}

uint64_t __crc32_reflect(uint64_t data, int32_t width) {
    uint64_t res = data & 0x01;

    for (int32_t i = 0; i < (int)width - 1; i++) {
        data >>= 1;
        res = (res << 1) | (data & 0x01);
    }

    return res;
}

void __crc32_make_table(uint32_t t[256], uint32_t poly, bool reflect_in) {
    int32_t width = 32; // 32bits checksum.
    uint64_t msb_mask = (uint32_t)(0x01 << (width - 1));
    uint64_t mask = (uint32_t)(((msb_mask - 1) << 1) | 1);

    int32_t tbl_idx_width = 8; // table index size.
    int32_t tbl_width = 0x01 << tbl_idx_width; // table size: 256

    for (int32_t i = 0; i < (int)tbl_width; i++) {
        uint64_t reg = (uint64_t)i;

        if (reflect_in) {
            reg = __crc32_reflect(reg, tbl_idx_width);
        }

        reg = reg << (width - tbl_idx_width);
        for (int32_t j = 0; j < tbl_idx_width; j++) {
            if ((reg&msb_mask) != 0) {
                reg = (reg << 1) ^ poly;
            } else {
                reg = reg << 1;
            }
        }

        if (reflect_in) {
            reg = __crc32_reflect(reg, width);
        }

        t[i] = (uint32_t)(reg & mask);
    }
}

// @see pycrc table_driven at https://github.com/winlinvip/pycrc/blob/master/pycrc/algorithms.py#L207
uint32_t __crc32_table_driven(uint32_t* t, const void* buf, int32_t size, uint32_t previous, 
                              bool reflect_in, uint32_t xor_in, bool reflect_out, uint32_t xor_out) {
    int32_t width = 32; // 32bits checksum.
    uint64_t msb_mask = (uint32_t)(0x01 << (width - 1));
    uint64_t mask = (uint32_t)(((msb_mask - 1) << 1) | 1);
    int32_t tbl_idx_width = 8; // table index size.
    uint8_t* p = (uint8_t*)buf;
    uint64_t reg = 0;

    if (!reflect_in) {
        reg = xor_in;

        for (int32_t i = 0; i < (int)size; i++) {
            uint8_t tblidx = (uint8_t)((reg >> (width - tbl_idx_width)) ^ p[i]);
            reg = t[tblidx] ^ (reg << tbl_idx_width);
        }
    } else {
        reg = previous ^ __crc32_reflect(xor_in, width);

        for (int32_t i = 0; i < (int)size; i++) {
            uint8_t tblidx = (uint8_t)(reg ^ p[i]);
            reg = t[tblidx] ^ (reg >> tbl_idx_width);
        }

        reg = __crc32_reflect(reg, width);
    }

    if (reflect_out) {
        reg = __crc32_reflect(reg, width);
    }

    reg ^= xor_out;
    return (uint32_t)(reg & mask);
}

// @see pycrc https://github.com/winlinvip/pycrc/blob/master/pycrc/algorithms.py#L207
// IEEETable is the table for the IEEE polynomial.
static uint32_t __crc32_IEEE_table[256];
static bool __crc32_IEEE_table_initialized = false;

uint32_t crc32_ieee(const void* buf, int32_t size, uint32_t previous) {
    // @see golang IEEE of hash/crc32/crc32.go
    // IEEE is by far and away the most common CRC-32 polynomial.
    // Used by ethernet (IEEE 802.3), v.42, fddi, gzip, zip, png, ...
    // @remark The poly of CRC32 IEEE is 0x04C11DB7, its reverse is 0xEDB88320,
    //      please read https://en.wikipedia.org/wiki/Cyclic_redundancy_check
    uint32_t poly = 0x04C11DB7;
    bool reflect_in = true;
    uint32_t xor_in = 0xffffffff;
    bool reflect_out = true;
    uint32_t xor_out = 0xffffffff;

    if (!__crc32_IEEE_table_initialized) {
        __crc32_make_table(__crc32_IEEE_table, poly, reflect_in);
        __crc32_IEEE_table_initialized = true;
    }

    return __crc32_table_driven(__crc32_IEEE_table, buf, size, previous, reflect_in, xor_in, reflect_out, xor_out);
}

}