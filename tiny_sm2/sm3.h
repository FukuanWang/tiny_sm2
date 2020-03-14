#include <stdint.h>

#ifndef _SM3_H_
#define _SM3_H_
/*
in: input data
in_len: input data length
out: output hash code
*/
void sm3(uint8_t* in, uint32_t in_len, uint8_t* out);

#endif  /* _SM3_H_  */
