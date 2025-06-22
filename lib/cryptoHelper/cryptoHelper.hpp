#include <mbedtls/md.h>

bool computeHmacSHA256(const uint8_t* key, size_t keyLen,
                       const uint8_t* data, size_t dataLen,
                       uint8_t* outHmac, size_t outLen = 32)
{
    const mbedtls_md_info_t* md = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    if (!md || outLen < 32) return false;

    mbedtls_md_context_t ctx;
    mbedtls_md_init(&ctx);

    if (mbedtls_md_setup(&ctx, md, 1) != 0) {
        mbedtls_md_free(&ctx);
        return false;
    }

    mbedtls_md_hmac_starts(&ctx, key, keyLen);
    mbedtls_md_hmac_update(&ctx, data, dataLen);
    mbedtls_md_hmac_finish(&ctx, outHmac);
    mbedtls_md_free(&ctx);
    return true;
}