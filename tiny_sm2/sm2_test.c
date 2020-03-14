#include <stdio.h>
#include <string.h>
#include <time.h>

#include "sm2.h"
#include "sm3.h"

#define TFuc(X) void X##_TEST(void)
#define InvT(X) X##_TEST()

//extern eccpoint_t G;
const eccpoint_t G =
{
	0x6B17D1F2, 0xE12C4247, 0xF8BCE6E5, 0x63A440F2, 0x77037D81, 0x2DEB33A0, 0xF4A13945, 0xD898C296,
	0x4FE342E2, 0xFE1A7F9B, 0x8EE7EB4A, 0x7C0F9E16, 0x2BCE3357, 0x6B315ECE, 0xCBB64068, 0x37BF51F5
};

void show_sig(signature_t sig){
    int i;
    for(i = 0; i < 16; i++)
    {
        printf("%08x ", sig[i]);
        if(((i + 1) % 8) == 0){
            printf("\n");
        }
    }
}

TFuc(SM2_SIG){
    char *M = "message digest";
    static const bn_t dA = { 0x128B2FA8, 0xBD433C6C, 0x068C8D80, 0x3DFF7979, 0x2A519A55, 0x171B1B65, 0x0C23661D, 0x15897263 };
    static const bn_t ZA = { 0xF4A38489, 0xE32B45B6, 0xF876E3AC, 0x2168CA39, 0x2362DC8F, 0x23459C1D, 0x1146FC3D, 0xBFB7BC9A };

    int i = 0;
	signature_t sigA;

	sm2_ecdsa_sign((uint8_t *)M, strlen(M), ZA, dA, sigA);
	printf("\n");
	printf("signature of A:\n");
	show_sig(sigA);


    eccpoint_t EccA;
	eccpoint_mult_jacobian(EccA, G, dA);

	printf("\n");
    printf("verification:\n");
	int Verify = sm2_ecdsa_verify(EccA, (uint8_t *)M, strlen(M), ZA, sigA);

    if (Verify)
    {
    	printf("Verify OK!\n");
    }
}


TFuc(SM2_ENC){
    char *M_en = "encryption standard";
    const bn_t DB_en = { 0x1649AB77, 0xA00637BD, 0x5E2EFE28, 0x3FBF3535, 0x34AA7F7C, 0xB89463F2, 0x08DDBC29, 0x20BB0DA0 };

    uint32_t mlen = strlen(M_en);
	uint8_t C[0x100];

	eccpoint_t pb;
	eccpoint_mult_jacobian(pb, G, DB_en);

	printf("\n");
	printf("encryption:\n");
	sm2_encrypt(C, (uint8_t *)M_en, mlen, pb);

    printf("encrypted text:\n");
    int i;
    for(i = 0; i < mlen; i++)
    {
        printf("%02x ", C[i]);
        if(((i + 1) % 16) == 0){
            printf("\n");
        }
    }
    printf("\n");

	uint8_t M1[0x100];
	memset(M1, 0, 0x100);

	printf("\n");
	printf("decryption:\n");
	sm2_decrypt(M1, mlen, C, DB_en);

	printf("decrypted text: %s \n",M1);
}


int main(void)
{
	printf("==================Program Start==================\n");
	//srand(time(NULL));
	InvT(SM2_SIG);
	InvT(SM2_ENC);
	printf("\n==================Program End==================\n");
}
