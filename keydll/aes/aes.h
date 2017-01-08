/**
 * Excerpted from OpenSSL 1.0.1c crypto/aes/aes_core.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


//#define FULL_UNROLL


/* Because array size can't be a const in C, the following two are macros. Both sizes are in bytes. */
#define AES_Nr_MAX     14
#define AES_BLOCK_SIZE 16

/* This should be a hidden type, but EVP requires that the size be known */
typedef struct
{
    unsigned int rd_key[4 *(AES_Nr_MAX + 1)];
    int rounds;
} AES_KEY;

typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

// swap ends for integer operations. use fast instruction _lrotl _lrotr if provided
#if defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_AMD64) || defined(_M_X64))
  #define SWAP(x) (_lrotl(x, 8) & 0x00ff00ff | _lrotr(x, 8) & 0xff00ff00)
  #define GETU32(p) SWAP(*((u32 *)(p)))
  #define PUTU32(ct, st) { *((u32 *)(ct)) = SWAP((st)); }
#else   
  #define GETU32(pt) (((u32)(pt)[0] << 24) ^ ((u32)(pt)[1] << 16) ^ ((u32)(pt)[2] <<  8) ^ ((u32)(pt)[3]))
  #define PUTU32(ct, st) { (ct)[0] = (u8)((st) >> 24); (ct)[1] = (u8)((st) >> 16); (ct)[2] = (u8)((st) >>  8); (ct)[3] = (u8)(st); }
#endif


typedef void (*block_f)( const u8 *in, u8 *out, const AES_KEY *key );


int expandEncryptionKey(const u8 *userKey, const int bits, AES_KEY *key);
int expandDecryptionKey(const u8 *userKey, const int bits, AES_KEY *key);

void AES_encrypt(const u8 *in, u8 *out, const AES_KEY *key);
void AES_decrypt(const u8 *in, u8 *out, const AES_KEY *key);

void ctr128_encrypt( const u8 *in, u8 *out, size_t len,
                     const void *key, u8 ivec[16], u8 ecount_buf[16],
                     unsigned int *num, block_f encryptor );
