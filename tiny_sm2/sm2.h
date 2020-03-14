#include <stdint.h>

#ifndef _SM2_H_
#define _SM2_H_

#include <stdio.h>
#include "sm3.h"

#define NIST_CURVE
/*there are still some problem without using NIST_CURVE*/

#define array_len_256  8
#define array_len_double_256 16
#define array_len_triple_256 24
#define bn_zero_256     { 0x0 ,0x0,0x0,0x0,0x0 ,0x0 ,0x0, 0x0 }
#define bn_one_256      { 0x0 ,0x0,0x0,0x0,0x0 ,0x0 ,0x0, 0x1 }



typedef uint32_t bn_256_t[array_len_256];
typedef uint32_t eccpoint256_t[array_len_double_256];
typedef uint32_t signature256_t[array_len_double_256];
typedef uint32_t eccpoint256J_t[array_len_triple_256];

#define bn_t			    bn_256_t
#define bn_zero		        bn_zero_256
#define bn_one		        bn_one_256
#define array_len	        array_len_256



#define array_len_double    array_len_double_256

#define eccpoint_t          eccpoint256_t
#define signature_t         signature256_t
#define eccpointJ_t         eccpoint256J_t



void eccpoint_mult_jacobian(eccpoint_t P_result,const eccpoint_t P_point, const bn_t P_scalar);


/*
 generate the signature(sm2 algorithm)
 -M: message to be signed
 -mlen: message length
 -z: user-identifiable idenification
 -Pri_key: private key
 -sig: output signature
*/
extern int sm2_ecdsa_sign(const uint8_t *M, const uint32_t mlen, const bn_t Z, const bn_t Pri_key, signature_t Sig);


/*
 verify the signature(sm2 algorithm)
 -P_publicKey: public key
 -M: message to be verified
 -mlen: message length
 -z: user-identifiable idenification
 -sig: input signature
*/
extern int sm2_ecdsa_verify(const eccpoint_t P_publicKey, const uint8_t *M, const uint32_t mlen, const bn_t Z, const signature_t Sig);


/*
 generate the signature(ecc algorithm)
 -M: message to be signed
 -mlen: message length
 -z: user-identifiable idenification
 -Pri_key: private key
 -sig: output signature
*/
extern int ecc_ecdsa_sign(const uint8_t *M, const uint32_t mlen, const bn_t Z, const bn_t Pri_key, signature_t Sig);



/*
 verify the signature(ecc algorithm)
 -P_publicKey: public key
 -M: message to be verified
 -mlen: message length
 -z: user-identifiable idenification
 -sig: input signature
*/
extern int ecc_ecdsa_verify(const eccpoint_t P_publicKey, const uint8_t *M, const uint32_t mlen, const bn_t Z, const signature_t Sig);


/*
 sm2 encryption 
 -C: output cipher text
 -M: plain text
 -mlen: message length
 -PB: public key
*/
extern int sm2_encrypt(uint8_t *C, const uint8_t *M, const uint32_t mlen, const eccpoint_t PB);


/*
 sm2 decryption
 -M: output plain text
 -mlen: message length
 -C: cipher text
 -d: private key
 */
int sm2_decrypt(uint8_t *M, uint32_t mlen, const uint8_t *C, const bn_t d);




#endif  /* _SM2_H_  */
