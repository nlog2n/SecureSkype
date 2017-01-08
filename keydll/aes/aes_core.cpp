
#include "stdafx.h"
#include "aes.h"
#include "aes_table.h"


void encipher(char *key, char *txt, int len)
{
	AES_KEY K;
	char cnt[17];

	expandEncryptionKey((u8*)key, 128, &K );
	for(int i=0; i<len; )
	{
		sprintf(cnt,"%016X", i);
		AES_encrypt((u8*)&cnt[0], (u8*)&cnt[0], &K );
		for(int j=0; (j<16) && (i<len); j++, i++) 
		{

			*txt ^= cnt[j];
			txt++;
		}

	}
}

void decipher(char *key, char *txt, int len)
{
	AES_KEY K;
	char cnt[17];

	expandEncryptionKey((u8*)key, 128, &K );
	for(int i=0; i<len; )
	{
		sprintf(cnt,"%016X", i);
		AES_encrypt((u8*)&cnt[0], (u8*)&cnt[0], &K );
		for(int j=0; (j<16) && (i<len); j++, i++) 
		{
			*txt ^= cnt[j];
			txt++;
		}

	}
}

int expandEncryptionKey(const u8 *userKey, const int bits, AES_KEY *key)
{
	u32 *rk;
	int i = 0;
	u32 temp;

	if (!userKey || !key)
		return -1;
	if (bits != 128 && bits != 192 && bits != 256)
		return -2;

	rk = key->rd_key;

	if (bits==128)
		key->rounds = 10;
	else if (bits==192)
		key->rounds = 12;
	else
		key->rounds = 14;

	rk[0] = GETU32(userKey     );
	rk[1] = GETU32(userKey +  4);
	rk[2] = GETU32(userKey +  8);
	rk[3] = GETU32(userKey + 12);
	if (bits == 128)
	{
		while (1)
		{
			temp  = rk[3];
			rk[4] = rk[0] ^
				(Te2[(temp >> 16) & 0xff] & 0xff000000) ^
				(Te3[(temp >>  8) & 0xff] & 0x00ff0000) ^
				(Te0[(temp      ) & 0xff] & 0x0000ff00) ^
				(Te1[(temp >> 24)       ] & 0x000000ff) ^
				rcon[i];
			rk[5] = rk[1] ^ rk[4];
			rk[6] = rk[2] ^ rk[5];
			rk[7] = rk[3] ^ rk[6];
			if (++i == 10) {
				return 0;
			}
			rk += 4;
		}
	}
	rk[4] = GETU32(userKey + 16);
	rk[5] = GETU32(userKey + 20);
	if (bits == 192) {
		while (1) {
			temp = rk[ 5];
			rk[ 6] = rk[ 0] ^
				(Te2[(temp >> 16) & 0xff] & 0xff000000) ^
				(Te3[(temp >>  8) & 0xff] & 0x00ff0000) ^
				(Te0[(temp      ) & 0xff] & 0x0000ff00) ^
				(Te1[(temp >> 24)       ] & 0x000000ff) ^
				rcon[i];
			rk[ 7] = rk[ 1] ^ rk[ 6];
			rk[ 8] = rk[ 2] ^ rk[ 7];
			rk[ 9] = rk[ 3] ^ rk[ 8];
			if (++i == 8)
			{
				return 0;
			}
			rk[10] = rk[ 4] ^ rk[ 9];
			rk[11] = rk[ 5] ^ rk[10];
			rk += 6;
		}
	}
	rk[6] = GETU32(userKey + 24);
	rk[7] = GETU32(userKey + 28);
	if (bits == 256)
	{
		while (1)
		{
			temp = rk[ 7];
			rk[ 8] = rk[ 0] ^
				(Te2[(temp >> 16) & 0xff] & 0xff000000) ^
				(Te3[(temp >>  8) & 0xff] & 0x00ff0000) ^
				(Te0[(temp      ) & 0xff] & 0x0000ff00) ^
				(Te1[(temp >> 24)       ] & 0x000000ff) ^
				rcon[i];
			rk[ 9] = rk[ 1] ^ rk[ 8];
			rk[10] = rk[ 2] ^ rk[ 9];
			rk[11] = rk[ 3] ^ rk[10];
			if (++i == 7)
			{
				return 0;
			}
			temp = rk[11];
			rk[12] = rk[ 4] ^
				(Te2[(temp >> 24)       ] & 0xff000000) ^
				(Te3[(temp >> 16) & 0xff] & 0x00ff0000) ^
				(Te0[(temp >>  8) & 0xff] & 0x0000ff00) ^
				(Te1[(temp      ) & 0xff] & 0x000000ff);
			rk[13] = rk[ 5] ^ rk[12];
			rk[14] = rk[ 6] ^ rk[13];
			rk[15] = rk[ 7] ^ rk[14];

			rk += 8;
		}
	}
	return 0;
}

/**
* Corresponds to private_AES_set_decrypt_key(.)
* Expand the cipher key into the decryption key schedule.
*/
int expandDecryptionKey(const u8 *userKey, const int bits, AES_KEY *key)
{
	u32 *rk;
	int i, j, status;
	u32 temp;

	/* first, start with an encryption schedule */
	status = expandEncryptionKey(userKey, bits, key);
	if (status < 0)
		return status;

	rk = key->rd_key;

	/* invert the order of the round keys: */
	for (i = 0, j = 4*(key->rounds); i < j; i += 4, j -= 4)
	{
		temp = rk[i    ]; rk[i    ] = rk[j    ]; rk[j    ] = temp;
		temp = rk[i + 1]; rk[i + 1] = rk[j + 1]; rk[j + 1] = temp;
		temp = rk[i + 2]; rk[i + 2] = rk[j + 2]; rk[j + 2] = temp;
		temp = rk[i + 3]; rk[i + 3] = rk[j + 3]; rk[j + 3] = temp;
	}
	/* apply the inverse MixColumn transform to all round keys but the first and the last: */
	for (i = 1; i < (key->rounds); i++)
	{
		rk += 4;
		rk[0] =
			Td0[Te1[(rk[0] >> 24)       ] & 0xff] ^
			Td1[Te1[(rk[0] >> 16) & 0xff] & 0xff] ^
			Td2[Te1[(rk[0] >>  8) & 0xff] & 0xff] ^
			Td3[Te1[(rk[0]      ) & 0xff] & 0xff];
		rk[1] =
			Td0[Te1[(rk[1] >> 24)       ] & 0xff] ^
			Td1[Te1[(rk[1] >> 16) & 0xff] & 0xff] ^
			Td2[Te1[(rk[1] >>  8) & 0xff] & 0xff] ^
			Td3[Te1[(rk[1]      ) & 0xff] & 0xff];
		rk[2] =
			Td0[Te1[(rk[2] >> 24)       ] & 0xff] ^
			Td1[Te1[(rk[2] >> 16) & 0xff] & 0xff] ^
			Td2[Te1[(rk[2] >>  8) & 0xff] & 0xff] ^
			Td3[Te1[(rk[2]      ) & 0xff] & 0xff];
		rk[3] =
			Td0[Te1[(rk[3] >> 24)       ] & 0xff] ^
			Td1[Te1[(rk[3] >> 16) & 0xff] & 0xff] ^
			Td2[Te1[(rk[3] >>  8) & 0xff] & 0xff] ^
			Td3[Te1[(rk[3]      ) & 0xff] & 0xff];
	}
	return 0;
}

/*
* Encrypt a single block
* in and out can overlap
*/
void AES_encrypt(const u8 *in, u8 *out, const AES_KEY *key)
{

	const u32 *rk;
	u32 s0, s1, s2, s3, t0, t1, t2, t3;

	int r;


	assert(in && out && key);
	rk = key->rd_key;

	/*
	* map byte array block to cipher state
	* and add initial round key:
	*/
	s0 = GETU32(in     ) ^ rk[0];
	s1 = GETU32(in +  4) ^ rk[1];
	s2 = GETU32(in +  8) ^ rk[2];
	s3 = GETU32(in + 12) ^ rk[3];


	/*
	* Nr - 1 full rounds:
	*/
	r = key->rounds >> 1;
	for (;;)
	{
		t0 =
			Te0[(s0 >> 24)       ] ^
			Te1[(s1 >> 16) & 0xff] ^
			Te2[(s2 >>  8) & 0xff] ^
			Te3[(s3      ) & 0xff] ^
			rk[4];
		t1 =
			Te0[(s1 >> 24)       ] ^
			Te1[(s2 >> 16) & 0xff] ^
			Te2[(s3 >>  8) & 0xff] ^
			Te3[(s0      ) & 0xff] ^
			rk[5];
		t2 =
			Te0[(s2 >> 24)       ] ^
			Te1[(s3 >> 16) & 0xff] ^
			Te2[(s0 >>  8) & 0xff] ^
			Te3[(s1      ) & 0xff] ^
			rk[6];
		t3 =
			Te0[(s3 >> 24)       ] ^
			Te1[(s0 >> 16) & 0xff] ^
			Te2[(s1 >>  8) & 0xff] ^
			Te3[(s2      ) & 0xff] ^
			rk[7];

		rk += 8;
		if (--r == 0)
		{
			break;
		}

		s0 =
			Te0[(t0 >> 24)       ] ^
			Te1[(t1 >> 16) & 0xff] ^
			Te2[(t2 >>  8) & 0xff] ^
			Te3[(t3      ) & 0xff] ^
			rk[0];
		s1 =
			Te0[(t1 >> 24)       ] ^
			Te1[(t2 >> 16) & 0xff] ^
			Te2[(t3 >>  8) & 0xff] ^
			Te3[(t0      ) & 0xff] ^
			rk[1];
		s2 =
			Te0[(t2 >> 24)       ] ^
			Te1[(t3 >> 16) & 0xff] ^
			Te2[(t0 >>  8) & 0xff] ^
			Te3[(t1      ) & 0xff] ^
			rk[2];
		s3 =
			Te0[(t3 >> 24)       ] ^
			Te1[(t0 >> 16) & 0xff] ^
			Te2[(t1 >>  8) & 0xff] ^
			Te3[(t2      ) & 0xff] ^
			rk[3];
	}


	/*
	* apply last round and
	* map cipher state to byte array block:
	*/
	s0 =
		(Te2[(t0 >> 24)       ] & 0xff000000) ^
		(Te3[(t1 >> 16) & 0xff] & 0x00ff0000) ^
		(Te0[(t2 >>  8) & 0xff] & 0x0000ff00) ^
		(Te1[(t3      ) & 0xff] & 0x000000ff) ^
		rk[0];
	PUTU32(out     , s0);
	s1 =
		(Te2[(t1 >> 24)       ] & 0xff000000) ^
		(Te3[(t2 >> 16) & 0xff] & 0x00ff0000) ^
		(Te0[(t3 >>  8) & 0xff] & 0x0000ff00) ^
		(Te1[(t0      ) & 0xff] & 0x000000ff) ^
		rk[1];
	PUTU32(out +  4, s1);
	s2 =
		(Te2[(t2 >> 24)       ] & 0xff000000) ^
		(Te3[(t3 >> 16) & 0xff] & 0x00ff0000) ^
		(Te0[(t0 >>  8) & 0xff] & 0x0000ff00) ^
		(Te1[(t1      ) & 0xff] & 0x000000ff) ^
		rk[2];
	PUTU32(out +  8, s2);
	s3 =
		(Te2[(t3 >> 24)       ] & 0xff000000) ^
		(Te3[(t0 >> 16) & 0xff] & 0x00ff0000) ^
		(Te0[(t1 >>  8) & 0xff] & 0x0000ff00) ^
		(Te1[(t2      ) & 0xff] & 0x000000ff) ^
		rk[3];
	PUTU32(out + 12, s3);
}

/*
* Decrypt a single block
* in and out can overlap
*/
void AES_decrypt(const u8 *in, u8 *out, const AES_KEY *key)
{

	const u32 *rk;
	u32 s0, s1, s2, s3, t0, t1, t2, t3;
#ifndef FULL_UNROLL
	int r;
#endif /* !FULL_UNROLL */

	assert(in && out && key);
	rk = key->rd_key;

	/*
	* map byte array block to cipher state
	* and add initial round key:
	*/
	s0 = GETU32(in     ) ^ rk[0];
	s1 = GETU32(in +  4) ^ rk[1];
	s2 = GETU32(in +  8) ^ rk[2];
	s3 = GETU32(in + 12) ^ rk[3];

	/*
	* Nr - 1 full rounds:
	*/
	r = key->rounds >> 1;
	for (;;)
	{
		t0 =
			Td0[(s0 >> 24)       ] ^
			Td1[(s3 >> 16) & 0xff] ^
			Td2[(s2 >>  8) & 0xff] ^
			Td3[(s1      ) & 0xff] ^
			rk[4];
		t1 =
			Td0[(s1 >> 24)       ] ^
			Td1[(s0 >> 16) & 0xff] ^
			Td2[(s3 >>  8) & 0xff] ^
			Td3[(s2      ) & 0xff] ^
			rk[5];
		t2 =
			Td0[(s2 >> 24)       ] ^
			Td1[(s1 >> 16) & 0xff] ^
			Td2[(s0 >>  8) & 0xff] ^
			Td3[(s3      ) & 0xff] ^
			rk[6];
		t3 =
			Td0[(s3 >> 24)       ] ^
			Td1[(s2 >> 16) & 0xff] ^
			Td2[(s1 >>  8) & 0xff] ^
			Td3[(s0      ) & 0xff] ^
			rk[7];

		rk += 8;
		if (--r == 0)
		{
			break;
		}

		s0 =
			Td0[(t0 >> 24)       ] ^
			Td1[(t3 >> 16) & 0xff] ^
			Td2[(t2 >>  8) & 0xff] ^
			Td3[(t1      ) & 0xff] ^
			rk[0];
		s1 =
			Td0[(t1 >> 24)       ] ^
			Td1[(t0 >> 16) & 0xff] ^
			Td2[(t3 >>  8) & 0xff] ^
			Td3[(t2      ) & 0xff] ^
			rk[1];
		s2 =
			Td0[(t2 >> 24)       ] ^
			Td1[(t1 >> 16) & 0xff] ^
			Td2[(t0 >>  8) & 0xff] ^
			Td3[(t3      ) & 0xff] ^
			rk[2];
		s3 =
			Td0[(t3 >> 24)       ] ^
			Td1[(t2 >> 16) & 0xff] ^
			Td2[(t1 >>  8) & 0xff] ^
			Td3[(t0      ) & 0xff] ^
			rk[3];
	}

	/*
	* apply last round and
	* map cipher state to byte array block:
	*/
	s0 =
		(Td4[(t0 >> 24)       ] << 24) ^
		(Td4[(t3 >> 16) & 0xff] << 16) ^
		(Td4[(t2 >>  8) & 0xff] <<  8) ^
		(Td4[(t1      ) & 0xff])       ^
		rk[0];
	PUTU32(out     , s0);
	s1 =
		(Td4[(t1 >> 24)       ] << 24) ^
		(Td4[(t0 >> 16) & 0xff] << 16) ^
		(Td4[(t3 >>  8) & 0xff] <<  8) ^
		(Td4[(t2      ) & 0xff])       ^
		rk[1];
	PUTU32(out +  4, s1);
	s2 =
		(Td4[(t2 >> 24)       ] << 24) ^
		(Td4[(t1 >> 16) & 0xff] << 16) ^
		(Td4[(t0 >>  8) & 0xff] <<  8) ^
		(Td4[(t3      ) & 0xff])       ^
		rk[2];
	PUTU32(out +  8, s2);
	s3 =
		(Td4[(t3 >> 24)       ] << 24) ^
		(Td4[(t2 >> 16) & 0xff] << 16) ^
		(Td4[(t1 >>  8) & 0xff] <<  8) ^
		(Td4[(t0      ) & 0xff])       ^
		rk[3];
	PUTU32(out + 12, s3);
}
