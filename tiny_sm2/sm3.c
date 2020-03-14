#include "sm3.h"
#include <string.h>

#define T0 (0x79cc4519)
#define T1 (0x7a879d8a)

#define FF0(x,y,z) ((x) ^ (y) ^ (z))
#define FF1(x,y,z) (((x) & (y)) | ( (x) & (z)) | ( (y) & (z)))

#define GG0(x,y,z) ((x) ^ (y) ^ (z))
#define GG1(x,y,z) (((x) & (y)) | ((~(x)) & (z)))

#define ROTL(x,n) ((((x) << (n)) | ((x) >> (32 - (n)))))

#define P0(x) ((x) ^  ROTL((x),9) ^ ROTL((x),17))
#define P1(x) ((x) ^  ROTL((x),15) ^ ROTL((x),23))

/*
 transfer from uint_8 array to uint_32 array
*/
#ifndef UC_TO_UL
#define UC_TO_UL(BN,S,i)								  \
{														  \
    (BN[i]) = ( (uint32_t) (S)[((i) << 2)    ] << 24 )    \
			| ( (uint32_t) (S)[((i) << 2) + 1] << 16 )    \
			| ( (uint32_t) (S)[((i) << 2) + 2] <<  8 )    \
			| ( (uint32_t) (S)[((i) << 2) + 3]       );   \
}
#endif

/*
 transfer from uint_32 array to uint_8 array
*/
#ifndef UL_TO_UC
#define UL_TO_UC(S,BN,i)												  \
{												    					  \
    (S)[((i) << 2)    ] = (uint8_t) ( ((BN[i]) & 0xFF000000) >> 24 );     \
    (S)[((i) << 2) + 1] = (uint8_t) ( ((BN[i]) & 0x00FF0000) >> 16 );     \
    (S)[((i) << 2) + 2] = (uint8_t) ( ((BN[i]) & 0x0000FF00) >>  8 );     \
    (S)[((i) << 2) + 3] = (uint8_t) ( ((BN[i]) & 0x000000FF)       );     \
}
#endif


typedef struct{
    uint32_t iv[8];
    uint8_t buffer[64];
} sm3_ctx;

static void sm3_init(sm3_ctx *ctx){
    ctx->iv[0] = 0x7380166F;
    ctx->iv[1] = 0x4914B2B9;
    ctx->iv[2] = 0x172442D7;
    ctx->iv[3] = 0xDA8A0600;
    ctx->iv[4] = 0xA96F30BC;
    ctx->iv[5] = 0x163138AA;
    ctx->iv[6] = 0xE38DEE4D;
    ctx->iv[7] = 0xB0FB0E4E;
}

static void sm3_message_expand(uint8_t* B, uint32_t* W0, uint32_t* W1){
    uint32_t j;
    for(j = 0; j < 16; j++){
        UC_TO_UL(W0, B, j);
    }

    for(j = 16; j < 68; j++){
        W0[j] = P1(W0[j - 16] ^ W0[j - 9] ^ ROTL(W0[j - 3], 15)) ^ ROTL(W0[j - 13], 7) ^ W0[j - 6];
    }

    for(j = 0; j < 64; j++){
        W1[j] = W0[j] ^ W0[j + 4];
    }
}


static void sm3_CF(uint32_t* IV, uint32_t* W0, uint32_t* W1){
    uint32_t A, B, C, D, E, F, G, H, SS1, SS2, TT1, TT2;
    A = IV[0];
    B = IV[1];
    C = IV[2];
    D = IV[3];
    E = IV[4];
    F = IV[5];
    G = IV[6];
    H = IV[7];

    int j;
    for(j = 0; j < 16; j++){
        SS1 = ROTL((ROTL(A, 12) + E + ROTL(T0, j)), 7);
        SS2 = SS1 ^ ROTL(A, 12);
		TT1 = FF0(A, B, C) + D + SS2 + W1[j];
		TT2 = GG0(E, F, G) + H + SS1 + W0[j];
		D = C;
		C = ROTL(B, 9);
		B = A;
		A = TT1;
		H = G;
		G = ROTL(F, 19);
		F = E;
		E = P0(TT2);
    }

    for(j = 16; j < 64; j++)
	{
		SS1 = ROTL((ROTL(A,12) + E + ROTL(T1,j)), 7);
		SS2 = SS1 ^ ROTL(A,12);
		TT1 = FF1(A,B,C) + D + SS2 + W1[j];
		TT2 = GG1(E,F,G) + H + SS1 + W0[j];
		D = C;
		C = ROTL(B,9);
		B = A;
		A = TT1;
		H = G;
		G = ROTL(F,19);
		F = E;
		E = P0(TT2);
	}

    IV[0] ^= A;
    IV[1] ^= B;
    IV[2] ^= C;
    IV[3] ^= D;
    IV[4] ^= E;
    IV[5] ^= F;
    IV[6] ^= G;
    IV[7] ^= H;
}

static void sm3_process(sm3_ctx* ctx, uint8_t* data, uint32_t data_len){
    uint32_t W0[68];
    uint32_t W1[64];
    uint32_t left = data_len;
    uint8_t *pdata;
    for(pdata = data; left >= 48; pdata += 64){
        sm3_message_expand(pdata, W0, W1);
        sm3_CF(ctx->iv, W0, W1);
        left -= 64;
    }

    memset(ctx->buffer, 0, 64);
    memcpy(ctx->buffer, pdata, left);
    ctx->buffer[left] = 0x80;
    uint32_t data_len_bit = data_len * 8;
    ctx->buffer[60] = (uint8_t)(data_len_bit >> 24);
    ctx->buffer[61] = (uint8_t)(data_len_bit >> 16);
    ctx->buffer[62] = (uint8_t)(data_len_bit >>  8);
    ctx->buffer[63] = (uint8_t)(data_len_bit      );
    sm3_message_expand(ctx->buffer, W0, W1);
    sm3_CF(ctx->iv, W0, W1);
}

void sm3(uint8_t* in, uint32_t len, uint8_t* out){
    sm3_ctx ctx;
    sm3_init(&ctx);
    sm3_process(&ctx, in, len);
    int j;
    for(j = 0; j < 8; j++){
        UL_TO_UC(out, ctx.iv, j);
    }
}

