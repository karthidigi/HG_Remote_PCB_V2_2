
// Source: https://github.com/kokke/tiny-AES-c
// Modified for AES-128 in CTR mode only

#ifndef _AES_H_
#define _AES_H_

#include <stdint.h>
#include <stddef.h> // For size_t

#define CTR 1 // Enable CTR mode only

struct AES_ctx {
  uint8_t RoundKey[176]; // 128-bit key, 10 rounds + initial key
  uint8_t Iv[16]; // Initialization vector for CTR
};
typedef struct AES_ctx AES_ctx;

// Prevent C++ name mangling
#ifdef __cplusplus
extern "C" {
#endif

void AES_init_ctx(AES_ctx* ctx, const uint8_t* key);
void AES_init_ctx_iv(AES_ctx* ctx, const uint8_t* key, const uint8_t* iv);
void AES_ctx_set_iv(AES_ctx* ctx, const uint8_t* iv);
void AES_CTR_xcrypt_buffer(AES_ctx* ctx, uint8_t* buf, uint32_t length);

#ifdef __cplusplus
}
#endif

#endif // _AES_H_