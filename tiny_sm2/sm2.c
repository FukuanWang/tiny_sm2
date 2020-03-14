#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "sm2.h"
#include "sm3.h"


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

#define curve_v (0x100)

typedef struct
{
	uint32_t low;
	uint32_t high;
} uint64;


#ifdef NIST_CURVE
const bn_t curve_p = { 0xFFFFFFFF, 0x00000001, 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };
const bn_t curve_b = { 0x5AC635D8, 0xAA3A93E7, 0xB3EBBD55, 0x769886BC, 0x651D06B0, 0xCC53B0F6, 0x3BCE3C3E, 0x27D2604B };
const bn_t curve_n = { 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xBCE6FAAD, 0xA7179E84, 0xF3B9CAC2, 0xFC632551 };

const eccpoint_t G =
{
	0x6B17D1F2, 0xE12C4247, 0xF8BCE6E5, 0x63A440F2, 0x77037D81, 0x2DEB33A0, 0xF4A13945, 0xD898C296,
	0x4FE342E2, 0xFE1A7F9B, 0x8EE7EB4A, 0x7C0F9E16, 0x2BCE3357, 0x6B315ECE, 0xCBB64068, 0x37BF51F5
};

#else
const bn_t curve_p = { 0xFFFFFFFE, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF };
const bn_t curve_b = { 0x28E9FA9E, 0x9D9F5E34, 0x4D5A9E4B, 0xCF6509A7, 0xF39789F5, 0x15AB8F92, 0xDDBCBD41, 0x4D940E93 };
const bn_t curve_n = { 0xFFFFFFFE, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x7203DF6B, 0x21C6052B, 0x53BBF409, 0x39D54123 };

const eccpoint_t G =
{
	0x32C4AE2C, 0x1F198119, 0x5F990446, 0x6A39C994, 0x8FE30BBF, 0xF2660BE1, 0x715A4589, 0x334C74C7,
	0xBC3736A2, 0xF4F6779C, 0x59BDCEE3, 0x6B692153, 0xD0A9877C, 0xC62A4740, 0x02DF32E5, 0x2139F0A0
};

const bn_t CURVE_P1[11] =
{
	{ 0xfffffffe, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff, 0xffffffff },
	{ 0xfffffffd, 0xffffffff, 0xffffffff, 0xffffffff, 0xfffffffe, 0x00000001, 0xffffffff, 0xfffffffe },
	{ 0xfffffffc, 0xffffffff, 0xffffffff, 0xffffffff, 0xfffffffd, 0x00000002, 0xffffffff, 0xfffffffd },
	{ 0xfffffffb, 0xffffffff, 0xffffffff, 0xffffffff, 0xfffffffc, 0x00000003, 0xffffffff, 0xfffffffc },
	{ 0xfffffffa, 0xffffffff, 0xffffffff, 0xffffffff, 0xfffffffb, 0x00000004, 0xffffffff, 0xfffffffb },
	{ 0xfffffff9, 0xffffffff, 0xffffffff, 0xffffffff, 0xfffffffa, 0x00000005, 0xffffffff, 0xfffffffa },
	{ 0xfffffff8, 0xffffffff, 0xffffffff, 0xffffffff, 0xfffffff9, 0x00000006, 0xffffffff, 0xfffffff9 },
	{ 0xfffffff7, 0xffffffff, 0xffffffff, 0xffffffff, 0xfffffff8, 0x00000007, 0xffffffff, 0xfffffff8 },
	{ 0xfffffff6, 0xffffffff, 0xffffffff, 0xffffffff, 0xfffffff7, 0x00000008, 0xffffffff, 0xfffffff7 },
	{ 0xfffffff5, 0xffffffff, 0xffffffff, 0xffffffff, 0xfffffff6, 0x00000009, 0xffffffff, 0xfffffff6 },
	{ 0xfffffff4, 0xffffffff, 0xffffffff, 0xffffffff, 0xfffffff5, 0x0000000a, 0xffffffff, 0xfffffff5 }
};

const eccpoint_t GPP[16] =
{
	{
		0x32c4ae2c, 0x1f198119, 0x5f990446, 0x6a39c994, 0x8fe30bbf, 0xf2660be1, 0x715a4589, 0x334c74c7,
		0xbc3736a2, 0xf4f6779c, 0x59bdcee3, 0x6b692153, 0xd0a9877c, 0xc62a4740, 0x02df32e5, 0x2139f0a0
	},
	{
		0xa97f7cd4, 0xb3c993b4, 0xbe2daa8c, 0xdb41e24c, 0xa13f6bd9, 0x45302244, 0xe26918f1, 0xd0509ebf,
		0x530b5dd8, 0x8c688ef5, 0xccc5cec0, 0x8a72150f, 0x7c400ee5, 0xcd045292, 0xaaacdd03, 0x7458f6e6
	},
	{
		0xc7490616, 0x68652e26, 0x040e008f, 0xdd5eb77a, 0x344a417b, 0x7fce19db, 0xa575da57, 0xcc372a9e,
		0xf2df5db2, 0xd144e945, 0x4504c622, 0xb51cf38f, 0x5006206e, 0xb579ff7d, 0xa6976eff, 0x5fbe6480
	},
	{
		0xddf09255, 0x5409c19d, 0xfdbe86a7, 0x5c139906, 0xa8019833, 0x7744ee78, 0xcd27e384, 0xd9fcaf15,
		0x847d18ff, 0xb38e8706, 0x5cd6b6e9, 0xc12d2922, 0x03793770, 0x7d6a49a2, 0x223b9496, 0x57e52bc1
	},
	{
		0xa27233f3, 0xa5959508, 0x0b4a2444, 0xa46a74c5, 0xfe8d59cb, 0x43619e4f, 0x173472a5, 0x8cca247e,
		0x379e72f6, 0x3722c924, 0x768f7689, 0xb210f45f, 0xc3a84331, 0x40d1ebca, 0x85227940, 0x922c02e9
	},
	{
		0x04b3cb10, 0xc9c6d8e2, 0x7c1aab77, 0x0f67f543, 0x125dcdd5, 0x89c2ff82, 0x668c74d7, 0x8ce20ace,
		0x63516355, 0x287e39fe, 0x4918e5c0, 0x2e2b0b93, 0x0c94816e, 0x63c4bc72, 0x739a8fd8, 0x05174a4b
	},
	{
		0x952072d6, 0xff9c65bd, 0xfa804513, 0x275f58aa, 0x7bea65c6, 0x421e189e, 0x2b834f1d, 0x509b9cd0,
		0xe6bb9804, 0x458bb70f, 0x1a473f8d, 0x9748f238, 0x58d1434a, 0xe934ba75, 0x03891e10, 0x5e009b00
	},
	{
		0xf73b839f, 0x13912c1a, 0x3291676c, 0x38d39324, 0x3b424f35, 0xf0ecce4c, 0x461b1bbc, 0xb80f829c,
		0x32ec7722, 0x695dc7cf, 0x5ee9fab9, 0x85c12455, 0xdc2e788f, 0xb170aa14, 0x4c353377, 0x1db0955e
	},
	{
		0xdd18aa4a, 0xec26eac4, 0x1c993f01, 0x115c57f7, 0xbf5113ab, 0x85b5ed44, 0x36969f8b, 0x77125e6a,
		0x161e5851, 0x969da822, 0x3361493f, 0x76fac50f, 0x6cce6001, 0xd7c0b853, 0xe0180d54, 0xd4ae2221
	},
	{
		0xa68b2ec4, 0x9d1921d0, 0xe2e0586b, 0xb5dc9419, 0xf7c96c55, 0x940a4726, 0x1323f252, 0xe493952a,
		0x96f361a2, 0x3b3deb99, 0x933ef442, 0xfca7a734, 0xb95f2de6, 0x9200269e, 0xfd4520a6, 0x3f5cc585
	},
	{
		0x6407be63, 0x9bc5b738, 0x137cbf4e, 0x8aa50535, 0x5b551c86, 0xa8976933, 0xe1de5f10, 0x0950c8ab,
		0x141caee6, 0x12e7ab07, 0x12a9423f, 0xd80dea54, 0x60fcfd98, 0x8fff6dec, 0xf9453211, 0x48e90344
	},
	{
		0xac8df677, 0x299e8288, 0x15c07a8c, 0x04226fe3, 0xbb570a9f, 0x430ea7f6, 0x4dbb2fcd, 0x069d45db,
		0xaef82cf3, 0x61db1c71, 0x30c43be6, 0x9b23cc14, 0x0e9b9643, 0xd766da33, 0x7bd8b7c8, 0x47718ce5
	},
	{
		0x178d1f6b, 0xd584afcc, 0xce1564c3, 0xdf7d67ea, 0x7e8581fb, 0xbcf5de01, 0x0263b20d, 0x539e4139,
		0xf575f34e, 0x5f6c63c6, 0x53da45ce, 0xc1caada1, 0x7021aafc, 0x29ac8e86, 0x9b499a7d, 0xa7820bf9
	},
	{
		0xec6f34ce, 0xacaa73a0, 0xd7a1f169, 0x475b4ddf, 0x02a85a74, 0x10754529, 0xce37e84f, 0xc9541b4b,
		0x3c85cf62, 0xa744817e, 0x6f251c63, 0xfbbd2927, 0xe607f33b, 0xe20135ea, 0x73306a0f, 0xf0485872
	},
	{
		0x28221c36, 0x398c4c3a, 0x625773ca, 0xe69bbe05, 0xc9816be0, 0x68be66db, 0x9dd4ef42, 0x1635b9dc,
		0x3b4d172d, 0x6963510b, 0xab2012c4, 0x1627cd81, 0x64c77a81, 0x858d4ca1, 0x2995225e, 0x66b162ba
	},
	{
		0x08daae84, 0x0b2a43eb, 0xd6b48259, 0xb69e8530, 0x27d053c0, 0xf150fe10, 0xac995c50, 0x66bdaaa5,
		0x80cc0a98, 0x0495af87, 0xc1b8e581, 0x50da662d, 0xf7dc3105, 0x60bbe83a, 0x2458b9e3, 0xc73bf657
	},
};
#endif

/*
 set the big number with 0
*/
static void bn_clear(uint32_t *x)
{
    memset(x, 0, array_len * 4);
}

/*
 set big number x with y
*/
static void bn_set(uint32_t*x, const uint32_t*y)
{
    memcpy(x, y, array_len * 4);
}


/*
 judge if x is zero,
 - return 1, if true
 - return 0, if not true
*/
static int bn_is_zero(const bn_t x)
{
	int i = 0;
	do
	{
		if (x[i])
		{
			return 0;
		}
		i++;
	} while (i < array_len);
	return 1;

}


/*
 compare two big numbers
 - return 1 if x > y
 - return -1 if x < y
 - return 0 if x == y
*/
static int bn_cmp(const bn_t x, const bn_t y)
{
	int i = 0;
	do
	{
		if (x[i] > y[i])
		{
			return 1;
		}
		else if (x[i] < y[i])
		{
			return -1;
		}
		i++;
	} while (i < array_len);
	return 0;
}


/*
 big number calculation: z = x + y
 - return the carry 0 or 1
*/
static uint32_t bn_add(uint32_t *x, const uint32_t *y, const uint32_t *z)
{
	uint32_t l_carry = 0;
	int i;
	for (i = 7; i >= 0; --i)
	{
		uint32_t l_sum = y[i] + z[i] + l_carry;
		if (l_sum != y[i])
		{
			l_carry = (l_sum < y[i]);
		}
		x[i] = l_sum;
	}
	return l_carry;
}


/*
 big number calculation: z = x - y
 -return the borrow 0 or 1
*/
static uint32_t bn_sub(bn_t x, const bn_t y, const bn_t z)
{
	uint32_t l_borrow = 0;
	int i;

	for (i = 7; i >= 0; i--)
	{
		uint32_t l_diff = y[i] - z[i] - l_borrow;
		if (l_diff != y[i])
		{
			l_borrow = (l_diff > y[i]);
		}
		x[i] = l_diff;
	}
	return l_borrow;
}


/*
 big number left shrift 1 bit
*/
static void bn_rshift1(uint32_t*bn)
{
	bn[7] = bn[7] >> 1 | (bn[6] << 31);
	bn[6] = bn[6] >> 1 | (bn[5] << 31);
	bn[5] = bn[5] >> 1 | (bn[4] << 31);
	bn[4] = bn[4] >> 1 | (bn[3] << 31);
	bn[3] = bn[3] >> 1 | (bn[2] << 31);
	bn[2] = bn[2] >> 1 | (bn[1] << 31);
	bn[1] = bn[1] >> 1 | (bn[0] << 31);
	bn[0] >>= 1;
}

/*
 big number left shrift n bit
*/
static void bn_rshiftn(uint32_t *bn, int n)
{
	if (n)
	{
		bn[7] = (bn[7] >> (n)) | bn[6] << (32 - (n));
		bn[6] = (bn[6] >> (n)) | bn[5] << (32 - (n));
		bn[5] = (bn[5] >> (n)) | bn[4] << (32 - (n));
		bn[4] = (bn[4] >> (n)) | bn[3] << (32 - (n));
		bn[3] = (bn[3] >> (n)) | bn[2] << (32 - (n));
		bn[2] = (bn[2] >> (n)) | bn[1] << (32 - (n));
		bn[1] = (bn[1] >> (n)) | bn[0] << (32 - (n));
		bn[0] >>= n;
	}
}

/*
 512 bits big number left shrift 1 bit
*/
static void bn_512rshift1(uint32_t*bn)
{
	bn[0 ] = bn[0 ] << 1 | bn[1 ] >> 31;
	bn[1 ] = bn[1 ] << 1 | bn[2 ] >> 31;
	bn[2 ] = bn[2 ] << 1 | bn[3 ] >> 31;
	bn[3 ] = bn[3 ] << 1 | bn[4 ] >> 31;
	bn[4 ] = bn[4 ] << 1 | bn[5 ] >> 31;
	bn[5 ] = bn[5 ] << 1 | bn[6 ] >> 31;
	bn[6 ] = bn[6 ] << 1 | bn[7 ] >> 31;
	bn[7 ] = bn[7 ] << 1 | bn[8 ] >> 31;
	bn[8 ] = bn[8 ] << 1 | bn[9 ] >> 31;
	bn[9 ] = bn[9 ] << 1 | bn[10] >> 31;
	bn[10] = bn[10] << 1 | bn[11] >> 31;
	bn[11] = bn[11] << 1 | bn[12] >> 31;
	bn[12] = bn[12] << 1 | bn[13] >> 31;
	bn[13] = bn[13] << 1 | bn[14] >> 31;
	bn[14] = bn[14] << 1 | bn[15] >> 31;
	bn[15] <<= 1;
}

/*

*/
static void bn_rshift(uint32_t *bn_result, const uint32_t *bn, uint32_t bn_shift)
{
	int i = 0;
	uint32_t k = bn_shift >> 5;
	bn_t btemp;
	bn_clear(btemp);

	for (i = k; i < array_len; i++)
	{
		btemp[i] = bn[i - k];
	}
	bn_set(bn_result, btemp);
	bn_rshiftn(bn_result, bn_shift % 32);
}


static uint64 mult_32_32(uint32_t bn_x, uint32_t bn_y)
{
	uint64 l_result;
	uint32_t a0 = bn_x & 0x0000fffful;
	uint32_t a1 = bn_x >> 16;
	uint32_t b0 = bn_y & 0x0000fffful;
	uint32_t b1 = bn_y >> 16;
	uint32_t m0 = a0 * b0;
	uint32_t m1 = a0 * b1;
	uint32_t m2 = a1 * b0;
	uint32_t m3 = a1 * b1;

	m2 += (m0 >> 16);
	m2 += m1;
	if (m2 < m1)
	{
		m3 += 0x10000ul;
	}
	l_result.low = bn_x * bn_y;
	l_result.high = m3 + (m2 >> 16);
	return l_result;
}

static uint64 add_64_64(uint64 a, uint64 b)
{
	uint64 l_result;
	l_result.low = a.low + b.low;
	l_result.high = a.high + b.high + (l_result.low < a.low);
	return l_result;
}

static void mult_64_64(uint32_t *bn_result, uint64 bn_x, uint64 bn_y)
{
	//uint64
	uint32_t a0 = bn_x.low;
	uint32_t a1 = bn_x.high;
	uint32_t b0 = bn_y.low;
	uint32_t b1 = bn_y.high;

	uint64 m0 = mult_32_32(a0, b0);
	uint64 m3 = mult_32_32(a1, b1);

	uint32_t m1 = a0 + a1;
	uint32_t m1_carry = (m1 < a0);
	uint32_t m2 = b0 + b1;
	uint32_t m2_carry = (m2 < b0);

	uint64 m12 = mult_32_32(m1, m2);


	uint32_t m12_carry;

	if (m1_carry && !m2_carry)
	{
		m12.high += m2;
		m12_carry = (m12.high < m2);

		uint64 m12t = m12;
		uint32_t borrow = 0;

		m12.low = m12t.low - m0.low;
		borrow = m12.low > m12t.low;
		m12.high = m12t.high - m0.high - borrow;
		if (m12.high != m12t.high)
			borrow = (m12.high > m12t.high);
		if (borrow)
			m12_carry--;
		borrow = 0;
		m12t = m12;
		m12.low = m12t.low - m3.low;
		borrow = m12.low > m12t.low;
		m12.high = m12t.high - m3.high - borrow;
		if (m12.high != m12t.high)
			borrow = (m12.high > m12t.high);
		if (borrow)
			m12_carry--;
	}
	else if (!m1_carry && m2_carry)
	{
		m12.high += m1;
		m12_carry = (m12.high < m1);

		uint64 m12t = m12;
		uint32_t borrow = 0;

		m12.low = m12t.low - m0.low;
		borrow = m12.low > m12t.low;
		m12.high = m12t.high - m0.high - borrow;
		if (m12.high != m12t.high)
			borrow = (m12.high > m12t.high);
		if (borrow)
			m12_carry--;

		borrow = 0;
		m12t = m12;
		m12.low = m12t.low - m3.low;
		borrow = m12.low > m12t.low;
		m12.high = m12t.high - m3.high - borrow;
		if (m12.high != m12t.high)
			borrow = (m12.high > m12t.high);
		if (borrow)
			m12_carry--;
	}
	else if (m1_carry && m2_carry)
	{
		m12_carry = 1;
		uint64 m12t = m12;
		m12.high = m12t.high + m1;
		m12_carry += (m12.high < m12t.high);
		m12t = m12;
		m12.high = m12t.high + m2;
		m12_carry += (m12.high < m12t.high);


		m12t = m12;
		uint32_t borrow = 0;

		m12.low = m12t.low - m0.low;
		borrow = m12.low > m12t.low;
		m12.high = m12t.high - m0.high - borrow;
		if (m12.high != m12t.high)
			borrow = (m12.high > m12t.high);
		if (borrow)
			m12_carry--;

		borrow = 0;
		m12t = m12;
		m12.low = m12t.low - m3.low;
		borrow = m12.low > m12t.low;
		m12.high = m12t.high - m3.high - borrow;
		if (m12.high != m12t.high)
			borrow = (m12.high > m12t.high);
		if (borrow)
			m12_carry--;
	}
	else
	{
		m12_carry = 0;
		uint64 m12t = m12;
		uint32_t borrow = 0;

		m12.low = m12t.low - m0.low;
		borrow = m12.low > m12t.low;
		m12.high = m12t.high - m0.high - borrow;
		if (m12.high != m12t.high)
			borrow = (m12.high > m12t.high);

		borrow = 0;
		m12t = m12;
		m12.low = m12t.low - m3.low;
		borrow = m12.low > m12t.low;
		m12.high = m12t.high - m3.high - borrow;
		if (m12.high != m12t.high)
			borrow = (m12.high > m12t.high);
	}
	uint32_t carry = 0;
	bn_result[3] = m0.low;
	bn_result[2] = m0.high + m12.low;
	carry = (bn_result[2] < m0.high);
	bn_result[1] = m3.low + m12.high + carry;
	if (bn_result[1] != m3.low)
		carry = (bn_result[1] < m3.low);
	bn_result[0] = m3.high + m12_carry + carry;
}


static void mult_128_128(bn_t bn_result, uint32_t *bn_x, uint32_t *bn_y)
{
	int i;
	uint64 a0, a1, b0, b1;
	a0.low = bn_x[3];
	a0.high = bn_x[2];
	a1.low = bn_x[1];
	a1.high = bn_x[0];

	b0.low = bn_y[3];
	b0.high = bn_y[2];
	b1.low = bn_y[1];
	b1.high = bn_y[0];

	uint32_t m0[4];
	uint32_t m3[4];
	mult_64_64(m0, a0, b0);
	mult_64_64(m3, a1, b1);

	uint64 m1, m2;
	uint32_t m1_carry = 0;
	m1.low = a0.low + a1.low;
	m1_carry = m1.low < a0.low;
	m1.high = a0.high + a1.high + m1_carry;
	if (m1.high != a0.high)
		m1_carry = (m1.high < a0.high);

	uint32_t m2_carry = 0;
	m2.low = b0.low + b1.low;
	m2_carry = m2.low < b0.low;
	m2.high = b0.high + b1.high + m2_carry;
	if (m2.high != b0.high)
		m2_carry = (m2.high < b0.high);

	uint32_t m12[4];
	mult_64_64(m12, m1, m2);

	uint32_t m12_carry;
    uint32_t borrow;
	if (m1_carry && !m2_carry)
	{
		m12[1] += m2.low;
		m12_carry = (m12[1] < m2.low);
		m12[0] = m2.high + m12[0] + m12_carry;
		if (m12[0] != m2.high)
			m12_carry = (m12[0] < m2.high);

		uint32_t m12t[4];
		m12t[0] = m12[0];
		m12t[1] = m12[1];
		m12t[2] = m12[2];
		m12t[3] = m12[3];
		m12[3] = m12t[3] - m0[3];
		borrow = (m12[3] > m12t[3]);
		for (i = 0; i < 3; i++)
		{
			m12[2 - i] = m12t[2 - i] - m0[2 - i] - borrow;
			if (m12[2 - i] != m12t[2 - i])
			{
				borrow = (m12[2 - i] > m12t[2 - i]);
			}
		}
		if (borrow)
		{
			m12_carry--;
		}

		m12t[0] = m12[0];
		m12t[1] = m12[1];
		m12t[2] = m12[2];
		m12t[3] = m12[3];
		borrow = 0;

		m12[3] = m12t[3] - m3[3];
		borrow = (m12[3] > m12t[3]);

		for (i = 0; i < 3; i++)
		{
			m12[2 - i] = m12t[2 - i] - m3[2 - i] - borrow;
			if (m12[2 - i] != m12t[2 - i])
			{
				borrow = (m12[2 - i] > m12t[2 - i]);
			}
		}
		if (borrow)
		{
			m12_carry--;
		}
	}

	else if (!m1_carry && m2_carry)
	{
		m12[1] += m1.low;
		m12_carry = (m12[1] < m1.low);
		m12[0] = m1.high + m12[0] + m12_carry;
		if (m12[0] != m1.high)
			m12_carry = (m12[0] < m1.high);

		uint32_t m12t[4];
		m12t[0] = m12[0];
		m12t[1] = m12[1];
		m12t[2] = m12[2];
		m12t[3] = m12[3];
		m12[3] = m12t[3] - m0[3];
		borrow = (m12[3] > m12t[3]);

		for (i = 0; i < 3; i++)
		{
			m12[2 - i] = m12t[2 - i] - m0[2 - i] - borrow;
			if (m12[2 - i] != m12t[2 - i])
			{
				borrow = (m12[2 - i] > m12t[2 - i]);
			}
		}
		if (borrow)
		{
			m12_carry--;
		}
		m12t[0] = m12[0];
		m12t[1] = m12[1];
		m12t[2] = m12[2];
		m12t[3] = m12[3];
		m12[3] = m12t[3] - m3[3];
		borrow = (m12[3] > m12t[3]);
		for (i = 0; i < 3; i++)
		{
			m12[2 - i] = m12t[2 - i] - m3[2 - i] - borrow;
			if (m12[2 - i] != m12t[2 - i])
			{
				borrow = (m12[2 - i] > m12t[2 - i]);
			}
		}
		if (borrow)
		{
			m12_carry--;
		}
	}
	else if (m1_carry && m2_carry)
	{
		m12_carry = 1;
		uint32_t carry_temp;
		m12[1] += m1.low;
		carry_temp = (m12[1] < m1.low);
		m12[0] = m1.high + m12[0] + carry_temp;
		if (m12[0] != m1.high)
			carry_temp = (m12[0] < m1.high);
		if (carry_temp)
			m12_carry++;

		m12[1] += m2.low;
		carry_temp = (m12[1] < m2.low);
		m12[0] = m2.high + m12[0] + carry_temp;
		if (m12[0] != m2.high)
			carry_temp = (m12[0] < m2.high);
		if (carry_temp)
			m12_carry++;
		uint32_t m12t[4];
		m12t[0] = m12[0];
		m12t[1] = m12[1];
		m12t[2] = m12[2];
		m12t[3] = m12[3];
		m12[3] = m12t[3] - m0[3];
		borrow = (m12[3] > m12t[3]);

		for (i = 0; i < 3; i++)
		{
			m12[2 - i] = m12t[2 - i] - m0[2 - i] - borrow;
			if (m12[2 - i] != m12t[2 - i])
			{
				borrow = (m12[2 - i] > m12t[2 - i]);
			}
		}
		if (borrow)
		{
			m12_carry--;
		}

		m12t[0] = m12[0];
		m12t[1] = m12[1];
		m12t[2] = m12[2];
		m12t[3] = m12[3];
		m12[3] = m12t[3] - m3[3];
		borrow = (m12[3] > m12t[3]);
		for (i = 0; i < 3; i++)
		{
			m12[2 - i] = m12t[2 - i] - m3[2 - i] - borrow;
			if (m12[2 - i] != m12t[2 - i])
			{
				borrow = (m12[2 - i] > m12t[2 - i]);
			}
		}
		if (borrow)
		{
			m12_carry--;
		}
	}
	else
	{
		m12_carry = 0;
		uint32_t m12t[4];
		m12t[0] = m12[0];
		m12t[1] = m12[1];
		m12t[2] = m12[2];
		m12t[3] = m12[3];
		m12[3] = m12t[3] - m0[3];
		borrow = (m12[3] > m12t[3]);
		for (i = 0; i < 3; i++)
		{
			m12[2 - i] = m12t[2 - i] - m0[2 - i] - borrow;
			if (m12[2 - i] != m12t[2 - i])
			{
				borrow = (m12[2 - i] > m12t[2 - i]);
			}
		}

		m12t[0] = m12[0];
		m12t[1] = m12[1];
		m12t[2] = m12[2];
		m12t[3] = m12[3];
		borrow = 0;
		m12[3] = m12t[3] - m3[3];
		borrow = (m12[3] > m12t[3]);

		for (i = 0; i < 3; i++)
		{
			m12[2 - i] = m12t[2 - i] - m3[2 - i] - borrow;
			if (m12[2 - i] != m12t[2 - i])
			{
				borrow = (m12[2 - i] > m12t[2 - i]);
			}
		}
	}

	uint32_t carry = 0;
	bn_result[7] = m0[3];
	bn_result[6] = m0[2];

	bn_result[5] = m0[1] + m12[3];
	carry = (bn_result[5] < m0[1]);
	bn_result[4] = m0[0] + m12[2] + carry;
	if (bn_result[4] != m0[0])
		carry = (bn_result[4] < m0[0]);
	bn_result[3] = m12[1] + m3[3] + carry;
	if (bn_result[3] != m12[1])
		carry = (bn_result[3] < m12[1]);
	bn_result[2] = m12[0] + m3[2] + carry;
	if (bn_result[2] != m12[0])
		carry = (bn_result[2] < m12[0]);
	bn_result[1] = m3[1] + m12_carry + carry;
	if (bn_result[1] != m3[1])
		carry = (bn_result[1] < m3[1]);
	bn_result[0] = m3[0] + carry;
}

/*
 big number calculation bn_result = bn_x * bn_y
*/

static void bn_mult(uint32_t *bn_result, const bn_t bn_x, const bn_t bn_y)
{
	uint32_t a0[4], a1[4], b0[4], b1[4];
	a0[3] = bn_x[7];
	a0[2] = bn_x[6];
	a0[1] = bn_x[5];
	a0[0] = bn_x[4];

	a1[3] = bn_x[3];
	a1[2] = bn_x[2];
	a1[1] = bn_x[1];
	a1[0] = bn_x[0];

	b0[3] = bn_y[7];
	b0[2] = bn_y[6];
	b0[1] = bn_y[5];
	b0[0] = bn_y[4];

	b1[3] = bn_y[3];
	b1[2] = bn_y[2];
	b1[1] = bn_y[1];
	b1[0] = bn_y[0];


	uint32_t m0[8];
	uint32_t m3[8];
	mult_128_128(m0, a0, b0);
	mult_128_128(m3, a1, b1);

	int i;
	uint32_t m1[4], m2[4];
	uint32_t m1_carry = 0;
	m1[3] = a0[3] + a1[3];
	m1_carry = (m1[3] < a0[3]);
	for (i = 2; i >= 0; i--)
	{
		m1[i] = a0[i] + a1[i] + m1_carry;
		if (m1[i] != a0[i])
			m1_carry = (m1[i] < a0[i]);
	}

	uint32_t m2_carry = 0;
	m2[3] = b0[3] + b1[3];
	m2_carry = (m2[3] < b0[3]);
	for (i = 2; i >= 0; i--)
	{
		m2[i] = b0[i] + b1[i] + m2_carry;
		if (m2[i] != b0[i])
			m2_carry = (m2[i] < b0[i]);
	}

	bn_t m12;
	mult_128_128(m12, m1, m2);
	uint32_t m12_carry;
    uint32_t borrow = 0;
	if (m1_carry && !m2_carry)
	{
		m12[3] += m2[3];
		m12_carry = (m12[3] < m2[3]);
		for (i = 2; i >= 0; i--)
		{
			m12[i] = m2[i] + m12[i] + m12_carry;
			if (m12[i] != m2[i])
				m12_carry = (m12[i] < m2[i]);
		}
		borrow = bn_sub(m12, m12, m0);
		if (borrow)
		{
			m12_carry--;
		}
		borrow = bn_sub(m12, m12, m3);
		if (borrow)
		{
			m12_carry--;
		}
	}
	else if (!m1_carry && m2_carry)
	{
		m12[3] += m1[3];
		m12_carry = (m12[3] < m1[3]);
		for (i = 2; i >= 0; i--)
		{
			m12[i] = m1[i] + m12[i] + m12_carry;
			if (m12[i] != m1[i])
				m12_carry = (m12[i] < m1[i]);
		}
		borrow = bn_sub(m12, m12, m0);
		if (borrow)
		{
			m12_carry--;
		}

		borrow = bn_sub(m12, m12, m3);
		if (borrow)
		{
			m12_carry--;
		}
	}
	else if (m1_carry && m2_carry)
	{
		m12_carry = 1;
		m12[3] += m1[3];
		uint32_t carry_temp;
		carry_temp = (m12[3] < m1[3]);
		for (i = 2; i >= 0; i--)
		{
			m12[i] = m1[i] + m12[i] + carry_temp;
			if (m12[i] != m1[i])
				carry_temp = (m12[i] < m1[i]);
		}
		if (carry_temp)
			m12_carry++;

		m12[3] += m2[3];
		carry_temp = (m12[3] < m2[3]);
		for (i = 2; i >= 0; --i)
		{
			m12[i] = m2[i] + m12[i] + carry_temp;
			if (m12[i] != m2[i])
				carry_temp = (m12[i] < m2[i]);
		}
		if (carry_temp)
			m12_carry++;
		borrow = bn_sub(m12, m12, m0);
		if (borrow)
		{
			m12_carry--;
		}

		borrow = bn_sub(m12, m12, m3);
		if (borrow)
		{
			m12_carry--;
		}
	}
	else
	{
		m12_carry = 0;
		borrow = bn_sub(m12, m12, m0);
		borrow = bn_sub(m12, m12, m3);
	}
	uint32_t carry;
	for (i = 4; i < 8; i++)
	{
		bn_result[i + 8] = m0[i];
	}

	bn_result[11] = m0[3] + m12[7];
	carry = (bn_result[11] < m0[3]);
	bn_result[10] = m0[2] + m12[6] + carry;
	if (bn_result[10] != m0[2])
		carry = (bn_result[10] < m0[2]);
	bn_result[9] = m0[1] + m12[5] + carry;
	if (bn_result[9] != m0[1])
		carry = (bn_result[9] < m0[1]);
	bn_result[8] = m0[0] + m12[4] + carry;
	if (bn_result[8] != m0[0])
		carry = (bn_result[8] < m0[0]);
	bn_result[7] = m3[7] + m12[3] + carry;
	if (bn_result[7] != m3[7])
		carry = (bn_result[7] < m3[7]);
	bn_result[6] = m3[6] + m12[2] + carry;
	if (bn_result[6] != m3[6])
		carry = (bn_result[6] < m3[6]);
	bn_result[5] = m3[5] + m12[1] + carry;
	if (bn_result[5] != m3[5])
		carry = (bn_result[5] < m3[5]);
	bn_result[4] = m3[4] + m12[0] + carry;
	if (bn_result[4] != m3[4])
		carry = (bn_result[4] < m3[4]);
	bn_result[3] = m3[3] + m12_carry + carry;
	if (bn_result[3] != m3[3])
		carry = (bn_result[3] < m3[3]);
	bn_result[2] = m3[2] + carry;
	carry = (bn_result[2] < m3[2]);
	bn_result[1] = m3[1] + carry;
	carry = (bn_result[1] < m3[1]);
	bn_result[0] = m3[0] + carry;
}



/*
 big number calculation: bn_result = bn^2
*/
static void bn_square(uint32_t *bn_result, const uint32_t *bn)
{
	uint64 r01 = { 0, 0 };
	uint32_t r2 = 0;

	uint32_t i, k;
	for (k = 0; k < array_len_double - 1; ++k)
	{
		uint32_t l_min = (k < array_len ? 0 : (k + 1) - array_len);
		for (i = l_min; i <= k && i <= k - i; ++i)
		{
			uint64 l_product = mult_32_32(bn[array_len - 1 - i], bn[array_len - 1 + i - k]);
			if ((i << 1) < k)
			{
				r2 += l_product.high >> 31;
				l_product.high = (l_product.high << 1) | (l_product.low >> 31);
				l_product.low <<= 1;
			}
			r01 = add_64_64(r01, l_product);
			r2 += (r01.high < l_product.high);
		}
		bn_result[15 - k] = r01.low;
		r01.low = r01.high;
		r01.high = r2;
		r2 = 0;
	}
	bn_result[0] = r01.low;
}

/*
 judge if the bn_bit-th bit of bn is zero
 -return 0, if zeros
 -return non-zeros, if not zero
*/
#define bn_testBit(bn,bn_bit)									\
(bn[array_len - ((bn_bit) >> 5) -1] & ((uint32_t)1 << ((bn_bit) % 32)))


/*
 calculate a 260 bit number module curve_p
*/
static void bn_260mod256(bn_t bn_result, const bn_t bn, uint32_t bn_carry)
{
	bn_t temp;
	bn_set(temp, bn);
	while (bn_carry > 0)
	{
		while (!(bn_cmp(temp, curve_p) == -1))
		{
			bn_sub(temp, temp, curve_p);
		}
		bn_sub(temp, temp, curve_p);
		bn_carry--;
	}
	while (!(bn_cmp(temp, curve_p) == -1))
	{
		bn_sub(temp, temp, curve_p);
	}
	bn_set(bn_result, temp);
}

/*
 calculate a 260 bit number module curve_n
*/
static void bn_260modn256(bn_t bn_result, const bn_t bn, uint32_t bn_carry)
{
	bn_t temp;
	bn_set(temp, bn);
	while (bn_carry > 0)
	{
		while (!(bn_cmp(temp, curve_n) == -1))
		{
			bn_sub(temp, temp, curve_n);
		}
		bn_sub(temp, temp, curve_n);
		bn_carry--;
	}
	while (!(bn_cmp(temp, curve_n) == -1))
	{
		bn_sub(temp, temp, curve_n);
	}
	bn_set(bn_result, temp);
}


/*
 module p calculation: bn_result = bn_x + bn_y module curve_p
*/
static void bn_modAdd(uint32_t *bn_result, const uint32_t *bn_x, const uint32_t *bn_y)
{
	if (bn_add(bn_result, bn_x, bn_y) || bn_cmp(bn_result, curve_p) >= 0)
	{
		bn_sub(bn_result, bn_result, curve_p);
	}
}

/*
 module n calculation: bn_result = bn_x + bn_y module curve_n
*/
static void bn_modnAdd(uint32_t *bn_result, const uint32_t *bn_x, const uint32_t *bn_y)
{
	if (bn_add(bn_result, bn_x, bn_y) || bn_cmp(bn_result, curve_n) >= 0)
	{
		bn_sub(bn_result, bn_result, curve_n);
	}

}

/*
 module p calculation: bn_result = bn_left - bn_left module curve_p
*/
static void bn_modSub(uint32_t *bn_result, const uint32_t *bn_left, const uint32_t *bn_right)
{
	if (bn_sub(bn_result, bn_left, bn_right))
	{
		bn_add(bn_result, bn_result, curve_p);
	}
}

/*
 module n calculation: bn_result = bn_left - bn_left module curve_n
*/
static void bn_modnSub(uint32_t *bn_result, const uint32_t *bn_left, const uint32_t *bn_right)
{
	if (bn_sub(bn_result, bn_left, bn_right))
	{
		bn_add(bn_result, bn_result, curve_n);
	}
}



#ifdef NIST_CURVE
/*
 512 bits number module curve_p
*/
static void bn_512mod256(bn_t bn_result, const uint32_t *bn)
{
	bn_t s, temp;
	int carry = 0;

	bn_set(s, bn + 8);
	temp[0] = bn[0];
	temp[1] = bn[1];
	temp[2] = bn[2];
	temp[3] = bn[3];
	temp[4] = bn[4];
	temp[5] = 0;
	temp[6] = 0;
	temp[7] = 0;
	carry += bn_add(s, s, temp);
	carry += bn_add(s, s, temp);


	temp[2] = 0;
	temp[3] = 0;
	temp[4] = 0;
	temp[5] = bn[5];
	temp[6] = bn[6];
	temp[7] = bn[7];
	carry += bn_add(s, s, temp);

	temp[0] = 0;
	temp[1] = bn[0];
	temp[2] = bn[1];
	temp[3] = bn[2];
	temp[4] = bn[3];
	temp[5] = 0;
	temp[6] = 0;
	temp[7] = 0;
	carry += bn_add(s, s, temp);
	carry += bn_add(s, s, temp);

	temp[0] = bn[7];
	temp[1] = bn[2];
	temp[2] = bn[0];
	temp[3] = bn[1];
	temp[4] = bn[2];
	temp[5] = bn[4];
	temp[6] = bn[5];
	temp[7] = bn[6];
	carry += bn_add(s, s, temp);

	temp[0] = bn[5];
	temp[1] = bn[7];
	temp[2] = 0;
	temp[3] = 0;
	temp[4] = 0;
	temp[5] = bn[2];
	temp[6] = bn[3];
	temp[7] = bn[4];
	carry -= bn_sub(s, s, temp);

	temp[0] = bn[4];
	temp[1] = bn[6];
	temp[2] = 0;
	temp[3] = 0;
	temp[4] = bn[0];
	temp[5] = bn[1];
	temp[6] = bn[2];
	temp[7] = bn[3];
	carry -= bn_sub(s, s, temp);

	temp[0] = bn[3];
	temp[1] = 0;
	temp[2] = bn[5];
	temp[3] = bn[6];
	temp[4] = bn[7];
	temp[5] = bn[0];
	temp[6] = bn[1];
	temp[7] = bn[2];
	carry -= bn_sub(s, s, temp);

	temp[0] = bn[2];
	temp[1] = 0;
	temp[2] = bn[4];
	temp[3] = bn[5];
	temp[4] = bn[6];
	temp[5] = 0;
	temp[6] = bn[0];
	temp[7] = bn[1];
	carry -= bn_sub(s, s, temp);

	if (carry < 0)
	{
		do
		{
			carry += bn_add(s, s, curve_p);
		} while (carry < 0);
		bn_set(bn_result, s);
	}
	else
		bn_260mod256(bn_result, s, carry);
}

#else

/*
 512 bits number module curve_p
*/
static void bn_512mod256(bn_t bn_result, const uint32_t *bn)
{
	uint32_t X1[array_len_double];
	memcpy(X1, bn, array_len_double * 4);
	int i;
	uint32_t carry = 0;
	uint32_t borrow = 0;

	uint64 X[array_len_double];
	for (i = 0; i < array_len_double; i++)
	{
		X[i].low = X1[i];
		X[i].high = 0;
	}
	uint64 t1, t2, t3, s1, s2, d1, d2, d3;
	uint64 F[8];
	t1 = add_64_64(X[1], X[2]);
	t2 = add_64_64(X[3], X[4]);
	t2 = add_64_64(X[5], t2);
	t3 = add_64_64(X[6], X[7]);

	s1 = add_64_64(t1, t2);
	s2 = add_64_64(t1, t3);

	d1 = add_64_64(X[0], X[1]);
	d2 = add_64_64(X[0], X[0]);
	d3 = add_64_64(X[0], X[3]);

	uint64 g1, g2, g3;
	g1 = add_64_64(s1, d2);
	g2 = add_64_64(X[1], X[6]);
	g3 = add_64_64(d3, t1);

	F[0] = add_64_64(g1, s2);
	F[0] = add_64_64(F[0], X[15]);

	F[1] = add_64_64(g1, g2);
	F[1] = add_64_64(F[1], X[14]);

	F[2].low = X[13].low - s2.low;
	borrow = (F[2].low > X[13].low);
	F[2].high = X[13].high - s2.high - borrow;
	if (X[13].high != s2.high)
		borrow = (F[2].high > X[13].high);

	F[3] = add_64_64(g3, X[2]);
	F[3] = add_64_64(F[3], X[4]);
	F[3] = add_64_64(F[3], X[7]);
	F[3] = add_64_64(F[3], X[12]);

	F[4] = add_64_64(g3, g2);
	F[4] = add_64_64(F[4], X[11]);

	F[5] = add_64_64(d2, t1);
	F[5] = add_64_64(F[5], X[5]);
	F[5] = add_64_64(F[5], X[10]);

	F[6] = add_64_64(d1, X[4]);
	F[6] = add_64_64(F[6], X[9]);

	F[7] = add_64_64(g1, d3);
	F[7] = add_64_64(F[7], s2);
	F[7] = add_64_64(F[7], X[8]);
	bn_t R;

	R[7] = F[0].low;
	for (i = 6; i >= 0; i--)
	{
		R[i] = F[6 - i].high + F[7 - i].low + carry;
		if (F[6 - i].high != R[i])
			carry = (R[i] < F[7 - i].low);
	}
	carry += F[7].high;

	if (borrow)
	{
		if (R[3])
		{
			R[3]--;
		}
		else
		{
			R[3]--;
			if (R[2])
			{
				R[2]--;
			}
			else
			{
				R[2]--;
				if (R[1])
				{
					R[1]--;
				}
				else
				{
					R[1]--;
					if (R[0])
					{
						R[0]--;
					}
					else
					{
						R[0]--;
						if (carry)
							carry--;
					}
				}
			}
		}
	}
	borrow = 0;
	if (carry > 0)
	{
		borrow = bn_sub(R, R, CURVE_P1[carry - 1]);
		if (borrow)
		{
			if (!(bn_cmp(R, curve_p) == -1))
			{
				bn_sub(R, R, curve_p);
			}
		}
		else
		{
			bn_260mod256(R, R, 1);
		}
	}
	if (!(bn_cmp(R, curve_p) == -1))
	{
		bn_sub(R, R, curve_p);
	}
	bn_set(bn_result, R);
}

#endif

/*
 512 bits number module curve_n
*/
static void bn_512modn256(bn_t bn_result, const uint32_t *bn)
{
	int i = 0;
	uint32_t temp[array_len_double];
	for (i = 0; i < array_len_double; i++)
	{
		temp[i] = bn[i];
	}

	uint32_t bn_carry = 0;

	for (i = 0; i < 256; i++)
	{
		bn_260modn256(temp, temp, bn_carry);
		bn_carry = (temp[0] >> 31);
		bn_512rshift1(temp);
	}
	bn_260modn256(temp, temp, bn_carry);
	bn_set(bn_result, temp);
}


/*
 module p calculation: bn_result = bn_left * bn_left module curve_p
*/
static inline void bn_modMult(uint32_t *bn_result, const uint32_t *bn_x, const uint32_t *bn_y)
{
	uint32_t l_product[array_len_double];
	bn_mult(l_product, bn_x, bn_y);
	bn_512mod256(bn_result, l_product);
}

/*
 module n calculation: bn_result = bn_left * bn_left module curve_n
*/
static void bn_modnMult(uint32_t *bn_result, const uint32_t *bn_x, const uint32_t *bn_y)
{
	uint32_t l_product[array_len_double];
	bn_mult(l_product, bn_x, bn_y);
	bn_512modn256(bn_result, l_product);
}

/*
 module p calculation: bn_result = bn^2 module curve_p
*/
static inline void bn_modSquare(uint32_t *bn_result, const uint32_t *bn)
{
	uint32_t l_product[array_len_double];
	bn_square(l_product, bn);
	bn_512mod256(bn_result, l_product);
}

/*
 judge if bn a even number
 -return 1, if even number
 -return 0, if odd number
*/
#define EVEN(bn) (!(bn[array_len - 1] & 1))

/*
 module p calculation: bn_result = bn^(-1) module curve_p
*/
static void bn_modInv(uint32_t *bn_result, const uint32_t *bn)
{
	bn_t a, b, u, v;
	uint32_t l_carry;
	int l_cmpResult;
	if (bn_is_zero(bn))
	{
		bn_clear(bn_result);
		return;
	}

	bn_set(a, bn);
	bn_set(b, curve_p);
	bn_clear(u);
	u[array_len - 1] = 1;
	bn_clear(v);

	while ((l_cmpResult = bn_cmp(a, b)) != 0)
	{
		l_carry = 0;
		if (EVEN(a))
		{
			bn_rshift1(a);
			if (!EVEN(u))
			{
				l_carry = bn_add(u, u, curve_p);
			}
			bn_rshift1(u);
			if (l_carry)
			{
				u[0] |= 0x80000000ul;
			}
		}
		else if (EVEN(b))
		{
			bn_rshift1(b);
			if (!EVEN(v))
			{
				l_carry = bn_add(v, v, curve_p);
			}
			bn_rshift1(v);
			if (l_carry)
			{
				v[0] |= 0x80000000ul;
			}
		}
		else if (l_cmpResult > 0)
		{
			bn_sub(a, a, b);
			bn_rshift1(a);
			if (bn_cmp(u, v) < 0)
			{
				bn_add(u, u, curve_p);
			}
			bn_sub(u, u, v);
			if (!EVEN(u))
			{
				l_carry = bn_add(u, u, curve_p);
			}
			bn_rshift1(u);
			if (l_carry)
			{
				u[0] |= 0x80000000ul;
			}
		}
		else
		{
			bn_sub(b, b, a);
			bn_rshift1(b);
			if (bn_cmp(v, u) < 0)
			{
				bn_add(v, v, curve_p);
			}
			bn_sub(v, v, u);
			if (!EVEN(v))
			{
				l_carry = bn_add(v, v, curve_p);
			}
			bn_rshift1(v);
			if (l_carry)
			{
				v[0] |= 0x80000000ul;
			}
		}
	}

	bn_set(bn_result, u);
}


/*
 module n calculation: bn_result = bn^(-1) module curve_n
*/
static void bn_modnInv(uint32_t *bn_result, const uint32_t *bn)
{
	bn_t a, b, u, v;
	uint32_t l_carry;
	int l_cmpResult;
	if (bn_is_zero(bn))
	{
		bn_clear(bn_result);
		return;
	}

	bn_set(a, bn);
	bn_set(b, curve_n);
	bn_clear(u);
	u[array_len - 1] = 1;
	bn_clear(v);

	while ((l_cmpResult = bn_cmp(a, b)) != 0)
	{
		l_carry = 0;
		if (EVEN(a))
		{
			bn_rshift1(a);
			if (!EVEN(u))
			{
				l_carry = bn_add(u, u, curve_n);
			}
			bn_rshift1(u);
			if (l_carry)
			{
				u[0] |= 0x80000000ul;
			}
		}
		else if (EVEN(b))
		{
			bn_rshift1(b);
			if (!EVEN(v))
			{
				l_carry = bn_add(v, v, curve_n);
			}
			bn_rshift1(v);
			if (l_carry)
			{
				v[0] |= 0x80000000ul;
			}
		}
		else if (l_cmpResult > 0)
		{
			bn_sub(a, a, b);
			bn_rshift1(a);
			if (bn_cmp(u, v) < 0)
			{
				bn_add(u, u, curve_n);
			}
			bn_sub(u, u, v);
			if (!EVEN(u))
			{
				l_carry = bn_add(u, u, curve_n);
			}
			bn_rshift1(u);
			if (l_carry)
			{
				u[0] |= 0x80000000ul;
			}
		}
		else
		{
			bn_sub(b, b, a);
			bn_rshift1(b);
			if (bn_cmp(v, u) < 0)
			{
				bn_add(v, v, curve_n);
			}
			bn_sub(v, v, u);
			if (!EVEN(v))
			{
				l_carry = bn_add(v, v, curve_n);
			}
			bn_rshift1(v);
			if (l_carry)
			{
				v[0] |= 0x80000000ul;
			}
		}
	}

	bn_set(bn_result, u);
}

/*
judge if eccpoint is zero
 - return 1: if true
 - return 0: if not true
*/
static int eccpoint_is_zero(const eccpoint_t P)
{
	int i = 0;
	do
	{
		if (P[i])
		{
			return 0;
		}
		i++;
	} while (i < array_len_double);
	return 1;
}


/*
 jacobian transformation: (X, Y) => (x, y, z)
*/
static void jacobian_transformation(eccpointJ_t p, const eccpoint_t P)
{
	bn_t t1;
	uint32_t *pz;
	pz = p + array_len_double;
	bn_modSquare(t1, pz);								    //z^2
	bn_modMult(p, P, t1);								    //x=X*z^2
	bn_modMult(t1, t1, pz);									//z^3
	bn_modMult(p + array_len, P + array_len, t1);			//y=Y*z^3
}


/*
 jacobian inv transformation (x, y, z) => (X, Y)
*/
static void jacobian_invtransformation(eccpoint_t P, const eccpointJ_t p)
{
	bn_t t1;
	bn_t zinv;
	bn_modInv(zinv, p + array_len_double);					//1/z
	bn_modSquare(t1, zinv);									//1/(z^2)
	bn_modMult(P, p, t1);									//x/(z^2)
	bn_modMult(t1, t1, zinv);								//1/(z^3)
	bn_modMult(P + array_len, p + array_len, t1);			//y/(z^3)
}

/*
 curve calculation: p_result = 2 * p (p is not a jacobian transmitted eccpoint)
*/
static void eccpoint_double(eccpoint_t p_result,const eccpoint_t p)
{
	bn_t kx, prkx;
	bn_set(kx, p);

	bn_t s, temp1;
	bn_modSquare(temp1, kx);											//x^2
	bn_modAdd(s, temp1, temp1);										    //2*x^2
	bn_modAdd(s, s, temp1);											    //3*x^2
	bn_clear(temp1);
	temp1[7] = 3;
	bn_modSub(s, s, temp1);											    //3*x^2+a
	bn_modAdd(temp1, p + array_len, p + array_len);					    //2*y
	bn_modInv(temp1, temp1);											//1/2*y
	bn_modMult(s, s, temp1);											//(3*x^2+a)/2*y

	bn_modSquare(prkx, s);												//s^2
	bn_modSub(prkx, prkx, kx);					 					    //s^2-x
	bn_modSub(p_result, prkx, kx);			    						//s^2-2*x

	bn_modSub(temp1, kx, p_result);									    //x-x3
	bn_modMult(temp1, temp1, s);										//s*(x-x3)
	bn_modSub(p_result + array_len, temp1, p + array_len);			    //s*(x-x3)-y
}


#define AEQUAL_3                 							 //if curve_a = -3
/*
 curve calculation: p_result = 2 * p (p is a jacobian transmitted eccpoint)
*/
static void eccpoint_add_eq_jacobian(eccpointJ_t p_result,const eccpointJ_t p)
{
	bn_t prx, pry, prz;
	const uint32_t *x = p;
	const uint32_t *y = p + array_len;
	const uint32_t *z = p + array_len_double;

	if (bn_is_zero(x) && bn_is_zero(y))
	{
		bn_clear(prz);
		prz[7] = 1;
		bn_set(p_result + array_len_double, prz);
		return;
	}

	bn_t s1;
	bn_t s2;
	bn_t s3;
	bn_t temp1;
	bn_t temp2;
#ifdef AEQUAL_3
	bn_modSquare(s3, z);												//z^2
	bn_modAdd(temp1, x, s3);											//x+z^2
	bn_modSub(temp2, x, s3);											//x-z^2
	bn_modMult(s3, temp1, temp2);										//(x+z^2)*(x-z^2)
	bn_modAdd(s1, s3, s3);												//2*(x+z^2)*(x-z^2)
	bn_modAdd(s1, s1, s3);												//3*(x+z^2)*(x-z^2)
#else
	bn_modSquare(s1, p);												//x^2
	bn_modAdd(temp1, s1, s1);											//2*x^2
	bn_modAdd(s1, temp1, s1);											//3*x^2
	bn_modSquare(s3, p + array_len_double);							    //z^2
	bn_modSquare(s3, s3);												//z^4
	bn_clear(temp1);
	temp1[7] = 3;
	bn_modMult(s3, temp1, s3);											//-a*z^4
	bn_modSub(s1, s1, s3);												//3*x^2+a*z^4

#endif

	bn_modSquare(temp1, y);											    //y^2
	bn_modMult(s2, temp1, x);											//x*y^2
	bn_modAdd(s2, s2, s2);												//2*x*y^2
	bn_modAdd(s2, s2, s2);												//4*x*y^2

	bn_modSquare(temp1, temp1);										    //y^4
	bn_modAdd(s3, temp1, temp1);										//2*y^4
	bn_modAdd(s3, s3, s3);												//4*y^4
	bn_modAdd(s3, s3, s3);												//8*y^4

	bn_modSquare(temp1, s1);											//s1^2
	bn_modAdd(temp2, s2, s2);											//2*s2
	bn_modSub(prx, temp1, temp2);										//s1^2-2*s2

	bn_modSub(temp1, s2, prx);											//s2-x3
	bn_modMult(temp1, temp1, s1);										//s1*(s2-x3)
	bn_modSub(pry, temp1, s3);											//s1*(s2-x3)-s3

	bn_modMult(temp2, y, z);											//y*z
	bn_modAdd(prz, temp2, temp2);										//2*y*z

	bn_set(p_result, prx);
	bn_set(p_result + array_len, pry);
	bn_set(p_result + array_len_double, prz);
}

/*
 curve calculation: p_result = p1 + p2 (p2 is not a jacobian transmitted eccpoint)
*/
static void eccpoint_add_neq(eccpointJ_t p_result,const eccpointJ_t p1,const eccpoint_t p2)
{
	bn_t ONE = bn_one;

	if (bn_is_zero(p1) == 1 && bn_is_zero(p1 + array_len) == 1)
	{
		bn_set(p_result, p2);
		bn_set(p_result + array_len, p2 + array_len);
		bn_set(p_result + array_len_double, ONE);
		return;
	}

	if (bn_is_zero(p2) == 1 && bn_is_zero(p2 + array_len) == 1)
	{
		bn_set(p_result, p1);
		bn_set(p_result + array_len, p1 + array_len);
		bn_set(p_result + array_len_double, p1 + array_len_double);
		return;
	}

	if (bn_cmp(p2, p1) == 0 && bn_cmp(ONE, p1 + array_len_double) == 0)
	{
		bn_clear(p_result);
		bn_clear(p_result + array_len);
		bn_set(p_result + array_len_double, ONE);
		return;
	}

	const uint32_t *p1x = p1;
	const uint32_t *p1y = p1 + array_len;
	const uint32_t *p1z = p1 + array_len_double;
	const uint32_t *p2kx = p2;
	const uint32_t *p2ky = p2 + array_len;

	bn_t A, B, C, D, temp1, temp2, temp3, temp4;

	bn_modSquare(temp1, p1z);											//z1^2
	bn_modMult(A, p2kx, temp1);										    //x2*z1^2

	bn_modMult(B, temp1, p1z);											//z1^3
	bn_modMult(B, B, p2ky);											    //y2*z1^3

	bn_modSub(C, A, p1x);												//A-x1

	bn_modSub(D, B, p1y);												//B-y1

	bn_modSquare(temp1, C);											    //C^2
	bn_modMult(temp2, temp1, p1x);									    //x1*C^2-use
	bn_modAdd(temp3, temp2, temp2);									    //2*(x1*C^2)
	bn_modMult(temp1, temp1, C);									    //C^3
	bn_modAdd(temp3, temp3, temp1);									    //C^3+2*(x1*C^2)
	bn_modSquare(temp4, D);											    //D^2
	bn_modSub(p_result, temp4, temp3);									//D^2-(C^3+2*(x1*C^2))

	bn_modSub(temp3, temp2, p_result);									//x1*C^2-x3
	bn_modMult(temp4, D, temp3);										//D*(x1*C^2-x3)
	bn_modMult(temp2, temp1, p1y);										//y1*C^3
	bn_modSub(p_result + array_len, temp4, temp2);					    //D*(x1*C^2-x3)-y1*C^3
	bn_modMult(p_result + array_len_double, p1z, C);					//z1*C
}

/*
 curve calculation: p_result = p1 + p2 (p2 is a jacobian transmitted eccpoint)
*/
static void eccpoint_add_neq_jacobian(eccpointJ_t p_result,const eccpointJ_t p1,const eccpointJ_t p2)
{
	if (bn_is_zero(p1) == 1 && bn_is_zero(p1 + array_len) == 1)
	{
		bn_set(p_result, p2);
		bn_set(p_result + array_len, p2 + array_len);
		bn_set(p_result + array_len_double, p2 + array_len_double);
		return;
	}

	if (bn_is_zero(p2) == 1 && bn_is_zero(p2 + array_len) == 1)
	{
		bn_set(p_result, p1);
		bn_set(p_result + array_len, p1 + array_len);
		bn_set(p_result + array_len_double, p1 + array_len_double);
		return;
	}

	if (bn_cmp(p2, p1) == 0 && bn_cmp(p2 + array_len_double, p1 + array_len_double) == 0)
	{
		bn_clear(p_result);
		bn_clear(p_result + array_len);
		bn_clear(p_result + array_len_double);
		(p_result + array_len_double)[7] = 1;
		return;
	}

	bn_t p1x, p1y, p1z, p2x, p2y, p2z;
	bn_set(p1x, p1);
	bn_set(p2x, p2);
	bn_set(p1y, p1 + array_len);
	bn_set(p2y, p2 + array_len);
	bn_set(p1z, p1 + array_len_double);
	bn_set(p2z, p2 + array_len_double);

	bn_t A, B, C, D, temp1, temp2, temp3, temp4, temp5;

	bn_modSquare(temp1, p2z);											//z2^2
	bn_modMult(A, temp1, p1x);											//x1*z2^2

	bn_modMult(temp1, temp1, p2z);										//z2^3
	bn_modMult(B, p1y, temp1);											//y1*z2^3

	bn_modSquare(temp2, p1z);											//z1^2
	bn_modMult(C, p2x, temp2);											//x2*z1^2

	bn_modSub(C, C, A);												    //x2*z1^2-x1*z2^2

	bn_modMult(D, temp2, p1z);											//z1^3
	bn_modMult(D, D, p2y);												//y2*z1^3
	bn_modSub(D, D, B);												    //y2*z1^3-y1*z2^3

	bn_modSquare(temp1, D);											    //D^2
	bn_modSquare(temp2, C);											    //C^2
	bn_modMult(temp3, temp2, A);										//A*C^2
	bn_modAdd(temp4, temp3, temp3);									    //2*A*C^2
	bn_modMult(temp5, temp2, C);										//C^3

	bn_modSub(temp1, temp1, temp4);									    //D^2-2*A*C^2
	bn_modSub(temp4, temp1, temp5);									    //D^2-2*A*C^2-C^3

	bn_set(p_result, temp4);
	bn_modSub(temp3, temp3, temp4);									    //A*C^2-x3
	bn_modMult(temp3, D, temp3);										//D*(A*C^2-x3)
	bn_modMult(temp5, temp5, B);										//B*C^3
	bn_modSub(p_result + array_len, temp3, temp5);					    //D*(A*C^2-x3)-B*C^3

	bn_modMult(temp2, p1z, C);											//z1*C
	bn_modMult(p_result + array_len_double, temp2, p2z);				//z1*z2*C

}


#ifndef NIST_CURVE
static void eccpoint_mult_G_jacobian(eccpoint_t P_result, const bn_t P_scalar)
{
	int i;
	int j = 255;
	bn_t k;
	bn_set(k, P_scalar);
	int t;
	uint32_t h;
	bn_t ktemp;

	eccpointJ_t p = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 };

	while (j >= 0)
	{
		if (!bn_testBit(k, j))
		{
			eccpoint_add_eq_jacobian(p, p);
			j--;
		}
		else
		{
			t = (j > 4) ? j - 4 : 0;
			while (!bn_testBit(k, t))
			{
				t++;
			}

			bn_rshift(ktemp, k, t);
			h = ktemp[7] & (0xFFFFFFFFul >> (32 - (j - t + 1) % 32));

			for (i = 0; i < j - t + 1; i++)
			{
				eccpoint_add_eq_jacobian(p, p);
			}
			eccpoint_add_neq(p, p, GPP[h >> 1]);
			j = t - 1;
		}
	}
	jacobian_invtransformation(P_result, p);
}
#endif

void eccpoint_mult_jacobian(eccpoint_t P_result,const eccpoint_t P_point, const bn_t P_scalar)
{
	int i;
	eccpointJ_t p = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 };
	eccpointJ_t pp[16] = { { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 } };
	jacobian_transformation(pp[0], P_point);

	int j = 255;
	eccpoint_t P2;
	eccpoint_double(P2, P_point);
	for (i = 0; i < 15; i++)
	{
		eccpoint_add_neq(pp[i + 1], pp[i], P2);
	}
	bn_t k;
	bn_set(k, P_scalar);
	int t;
	uint32_t h;
	bn_t ktemp;

	while (j >= 0)
	{
		if (!bn_testBit(k, j))
		{
			eccpoint_add_eq_jacobian(p, p);
			j--;
		}
		else
		{
			t = (j > 4) ? j - 4 : 0;
			while (!bn_testBit(k, t))
			{
				t++;
			}
			bn_rshift(ktemp, k, t);
			h = ktemp[7] & (0xFFFFFFFFul >> (32 - (j - t + 1) % 32));
			for (i = 0; i < j - t + 1; i++)
			{
				eccpoint_add_eq_jacobian(p, p);
			}
			eccpoint_add_neq_jacobian(p, p, pp[h >> 1]);
			j = t - 1;
		}
	}
	jacobian_invtransformation(P_result, p);
}

/*
 get a random big number
*/
static void getRandnum(bn_t bn)
{
    int i;
    do{
        for(i = 0; i < array_len; i++){
            bn[i] = (uint32_t)rand() | ((uint32_t)rand() << 15) | ((uint32_t)rand() << 30);
        }
    }while(bn_cmp(bn, curve_p) > 0);
}

/*
 judge if p is a eccpoint in this curve.
 -return 1, if true
 -return 0, if zero
*/
static int is_eccpoint(eccpoint_t p)
{
	bn_t x;
	bn_t y;
	bn_set(x, p);
	bn_set(y, p + array_len);

	bn_t y2_bar;
	bn_t ax;
	bn_t x2;
	bn_t x3;
	bn_t y2;

	bn_clear(ax);
	ax[7] = 3;
	bn_modMult(ax, x, ax);
	bn_modSquare(x2, x);
	bn_modMult(x3, x2, x);
	bn_modSub(y2_bar, x3, ax);
	bn_modAdd(y2_bar, y2_bar, curve_b);
	bn_modSquare(y2, y);
	return !bn_cmp(y2, y2_bar);
}


static void bn_to_str(uint8_t *s, const bn_t x)
{
	int i;
	for (i = 0; i < array_len; i++)
	{
		UL_TO_UC(s, x, i);
	}
}

static void str_to_bn(bn_t x, uint8_t *s)
{
	int i;
	for (i = 0; i < array_len; i++)
	{
		UC_TO_UL(x, s, i);
	}
}

static int str_is_zero(const uint8_t *S, const uint32_t slen)
{
	uint32_t i;
	for (i = 0; i < slen; i++)
	{
		if (S[i])
			return 0;
	}
	return 1;
}


static uint32_t TopXdivv(uint32_t X)
{
	if (X % curve_v == 0)
	{
		return X >> 8;
	}
	else
	{
		return (X >> 8) + 1;
	}
}

/*
 KDF function
*/
static int KDF(uint8_t *R, const uint8_t *S, const uint32_t slen, const uint32_t klen)
{
	uint32_t ct = 0x00000001;
	uint32_t i;
	uint8_t H[0x10][0x20];
	uint32_t imax = TopXdivv(klen);
	for (i = 0; i < imax; i++)
	{
		uint8_t ctc[4];
		ctc[0] = (uint8_t)(ct >> 24);
		ctc[1] = (uint8_t)(ct >> 16);
		ctc[2] = (uint8_t)(ct >> 8 );
		ctc[3] = (uint8_t)(ct      );

		uint8_t *Zct;
		if ((Zct = (uint8_t *)malloc(sizeof(uint8_t) * (slen+4))) == NULL){
            return -1;
        }
        memcpy(Zct, S, slen);
        memcpy(Zct + slen, ctc, 4);
		sm3(Zct, slen + 4, H[i]);
		ct++;
	}
	uint8_t *H_end;
	H_end = H[imax - 1];

    uint8_t *K;
    if ((K = (uint8_t *)malloc(sizeof(uint8_t) * ((imax - 1) << 5))) == NULL){
        return -1;
    }

	for (i = 0; i < imax - 1; i++)
	{
	    memcpy(K, K, i << 5);
	    memcpy(K + (i<<5), H[i], 32);
	}
	memcpy(R, K, (imax - 1) << 5);
	memcpy(R + ((imax - 1) << 5), H_end, 32);
	return 0;
}


static void str_xor(uint8_t *R, const uint8_t *X, const uint8_t *Y, uint32_t klen)
{
	uint32_t i;
	for (i = 0; i < klen; i++)
	{
		R[i] = X[i] ^ Y[i];
	}
}


/*
 generate the signature(sm2 algorithm)
*/
int sm2_ecdsa_sign(const uint8_t *M, const uint32_t mlen, const bn_t Z, const bn_t Pri_key, signature_t Sig)
{
	int i = 0;
	uint8_t ZC[32];
	for (i = 0; i < array_len; i++)
	{
		UL_TO_UC(ZC, Z, i);
	}

	uint8_t *Mbar;
	if ((Mbar = (uint8_t *)malloc(sizeof(uint8_t) * (mlen + 32)))==NULL){
        return -1;
	}

	memcpy(Mbar, ZC, 32);
	memcpy(Mbar + 32, M, mlen);

	uint8_t e[32];
	sm3(Mbar, 32 + mlen, e);

	bn_t e1;
	for (i = 0; i < array_len; i++)
	{
		UC_TO_UL(e1, e, i);
	}
	eccpoint_t dbd;
	bn_t n_test;
	bn_t k;
	do
	{
		do
		{
			do
			{
				getRandnum(k);
				if (bn_cmp(k, curve_n) != -1)
					bn_sub(k, k, curve_n);
			} while (bn_is_zero(k));

#ifdef NIST_CURVE
			eccpoint_mult_jacobian(dbd, G, k);
#else
			eccpoint_mult_G_jacobian(dbd, k);
#endif

			bn_modnAdd(Sig, e1, dbd);
			bn_modnAdd(n_test, Sig, k);

		} while (bn_is_zero(Sig) == 1 || bn_is_zero(n_test) == 1);
		bn_t dp1 = bn_one;
		bn_modnAdd(dp1, dp1, Pri_key);
		bn_modnInv(dp1, dp1);
		bn_t krda;
		bn_modnMult(krda, Sig, Pri_key);
		bn_modnSub(krda, k, krda);
		bn_modnMult(Sig + array_len, dp1, krda);
	} while (bn_is_zero(Sig + array_len) != 0);
	return 0;
}


/*
 verify the signature(sm2 algorithm)
*/
int sm2_ecdsa_verify(const eccpoint_t P_publicKey, const uint8_t *M, const uint32_t mlen, const bn_t Z, const signature_t Sig)
{
	bn_t t;

	if (bn_is_zero(Sig) == 1 || bn_is_zero(Sig + array_len) == 1)
	{
		return 0;
	}

	if (bn_cmp(Sig, curve_n) != -1 || bn_cmp(Sig + array_len, curve_n) != -1)
	{
		return 0;
	}

	bn_modnAdd(t, Sig, Sig + array_len);
	int i;
	uint8_t ZC[32];
	for (i = 0; i < array_len; i++)
	{
		UL_TO_UC(ZC, Z, i);
	}

	uint8_t *Mbar = (uint8_t *)malloc(sizeof(uint8_t) * (mlen + 32));
	memcpy(Mbar, ZC, 32);
	memcpy(Mbar + 32, M, mlen);

	uint8_t e[32];
	sm3(Mbar, 32 + mlen, e);

	bn_t e1;
	for (i = 0; i < array_len; i++)
	{
		UC_TO_UL(e1, e, i);
	}

	if (bn_is_zero(t))
	{
		return 0;
	}
	eccpoint_t P0;

#ifdef NIST_CURVE
	eccpoint_mult_jacobian(P0, G, Sig + array_len);
#else
	eccpoint_mult_G_jacobian(P0, Sig + array_len);
#endif

	eccpoint_t P00;
	eccpoint_mult_jacobian(P00, P_publicKey, t);
	eccpoint_t P1;
	eccpointJ_t p1;
	eccpointJ_t p0 = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 };
	eccpointJ_t p00 = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 };

	jacobian_transformation(p0, P0);
	jacobian_transformation(p00, P00);

	eccpoint_add_neq_jacobian(p1, p0, p00);
	jacobian_invtransformation(P1, p1);

	bn_t R;
	bn_modnAdd(R, e1, P1);

	if (bn_cmp(Sig, R) != 0)
	{
		return 0;
	}
	return 1;
}



/*
 generate the signature(ecc algorithm)
*/
int ecc_ecdsa_sign(const uint8_t *M, const uint32_t mlen, const bn_t Z, const bn_t Pri_key, signature_t Sig)
{
	int i = 0;
	uint8_t ZC[32];
	for (i = 0; i < array_len; i++)
	{
		UL_TO_UC(ZC, Z, i);
	}

    uint8_t *Mbar = (uint8_t *)malloc(sizeof(uint8_t) * (mlen + 32));
	memcpy(Mbar, ZC, 32);
	memcpy(Mbar + 32, M, mlen);

	uint8_t e[32];
	sm3(Mbar, 32 + mlen, e);

	bn_t e1;
	for (i = 0; i < array_len; i++)
	{
		UC_TO_UL(e1, e, i);
	}

	eccpoint_t dbd;
	bn_t n_test;
	bn_t k;
	do
	{
		do
		{
			do
			{
				getRandnum(k);
				if (bn_cmp(k, curve_n) != -1)
					bn_sub(k, k, curve_n);
			} while (bn_is_zero(k));
#ifdef NIST_CURVE
			eccpoint_mult_jacobian(dbd, G, k);
#else
			eccpoint_mult_G_jacobian(dbd, k);
#endif

			bn_set(Sig, dbd);
			bn_modnAdd(n_test, Sig, k);

		} while (bn_is_zero(Sig) == 1 || bn_is_zero(n_test) == 1);
		bn_t dp1;
		bn_modnMult(dp1, Pri_key, Sig);
		bn_modnAdd(dp1, dp1, e1);
		bn_t dp2;
		bn_modnInv(dp2, k);
		bn_modnMult(Sig + array_len, dp1, dp2);
	} while (bn_is_zero(Sig + array_len) != 0);

	return 1;
}


/*
 verify the signature(ecc algorithm)
*/
int ecc_ecdsa_verify(const eccpoint_t P_publicKey, const uint8_t *M, const uint32_t mlen, const bn_t Z, const signature_t Sig)
{
	bn_t w;
	if (bn_is_zero(Sig) == 1 || bn_is_zero(Sig + array_len) == 1)
	{
		return -2;
	}

	if (bn_cmp(Sig, curve_n) != -1 || bn_cmp(Sig + array_len, curve_n) != -1)
	{
		return -2;
	}

	bn_modnInv(w, Sig + 8);
	int i;
	uint8_t ZC[32];
	for (i = 0; i < array_len; i++)
	{
		UL_TO_UC(ZC, Z, i);
	}

    uint8_t *Mbar;
    if ((Mbar = (uint8_t *)malloc(sizeof(uint8_t) * (mlen + 32))) == NULL){
        return -1;
    }
	memcpy(Mbar, ZC, 32);
	memcpy(Mbar + 32, M, mlen);

	uint8_t e[32];
	sm3(Mbar, 32 + mlen, e);
	bn_t e1;
	for (i = 0; i < array_len; i++)
	{
		UC_TO_UL(e1, e, i);
	}

	if (bn_is_zero(w))
	{
		return 0;
	}

	eccpoint_t P1, P2;
	bn_t u1, u2;
	bn_modnMult(u1, w, e1);
	bn_modnMult(u2, w, Sig);

#ifdef NIST_CURVE
	eccpoint_mult_jacobian(P1, G, u1);
#else
	eccpoint_mult_G_jacobian(P1, u1);
#endif
	eccpoint_mult_jacobian(P2, P_publicKey, u2);

	eccpoint_t P;

	eccpointJ_t p;
	eccpointJ_t p1 = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 };
	eccpointJ_t p2 = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 };

	jacobian_transformation(p1, P1);
	jacobian_transformation(p2, P2);

	eccpoint_add_neq_jacobian(p, p1, p2);
	jacobian_invtransformation(P, p);

	if (bn_cmp(Sig, P) != 0)
	{
		return 0;
	}
	return 1;
}


/*
sm2 encryption
*/
int sm2_encrypt(uint8_t *C, const uint8_t *M, const uint32_t mlen, const eccpoint_t PB)
{
	uint8_t x2[32];
	uint8_t y2[32];
	uint8_t *t;
    uint8_t *C1;

	if ((t = (uint8_t *)malloc(sizeof(uint8_t) * mlen)) == NULL)
        return -1;

    if ((C1 = (uint8_t *)malloc(sizeof(uint8_t) * 65)) == NULL)
        return -1;

	do
	{
		bn_t k;
		getRandnum(k);
		eccpoint_t P1, P2;
#ifdef NIST_CURVE
		eccpoint_mult_jacobian(P1, G, k);
#else
		eccpoint_mult_G_jacobian(P1, k);
#endif
		uint8_t x1[32];
		uint8_t y1[32];
		bn_to_str(x1, P1);
		bn_to_str(y1, P1 + array_len);
        C1[0] = 4;//PC
        memcpy(C1 + 1, x1, 32);
		memcpy(C1 + 33, y1, 32);
		if (eccpoint_is_zero(PB))
		{
			return 0;
		}
		eccpoint_mult_jacobian(P2, PB, k);
		bn_to_str(x2, P2);
		bn_to_str(y2, P2 + array_len);
		uint8_t xy2[64];
		uint8_t *pxy2 = xy2;
		memcpy(pxy2, x2, 32);
		memcpy(pxy2 + 32, y2, 32);
		if(KDF(t, xy2, 64, mlen << 3)){
			return -1;
		}
	} while (str_is_zero(t, mlen));

	uint8_t *C2;
	if ((C2 = (uint8_t *)malloc(sizeof(uint8_t) * mlen)) == NULL)
        return -1;
	str_xor(C2, M, t, mlen);

	uint8_t *H3;
	if ((H3 = (uint8_t *)malloc(sizeof(uint8_t) * (mlen + 64))) == NULL)
        return -1;
	uint8_t C3[32];
	memcpy(H3, x2, 32);
	memcpy(H3 + 32, M, mlen);
	memcpy(H3 + 32 + mlen, y2, 32);
	sm3(H3, mlen + 64, C3);
	memcpy(C, C1, 65);
	memcpy(C + 65, C2, mlen);
	memcpy(C + 65 + mlen, C3, mlen);
	return 0;
}


/*
 sm2 decryption
*/
int sm2_decrypt(uint8_t *M, uint32_t mlen, const uint8_t *C, const bn_t d)
{
	int i;
	uint8_t C1[65];
	uint8_t *t;
	uint8_t *C2;
	uint8_t x2[32];
	uint8_t y2[32];
	uint8_t u[32];
	uint8_t C3[32];

    if ((t = (uint8_t *)malloc(sizeof(uint8_t) * mlen)) == NULL)
        return -1;

    if ((C2 = (uint8_t *)malloc(sizeof(uint8_t) * mlen)) == NULL)
        return -1;

	for (i = 0; i < 65; i++)
	{
		C1[i] = C[i];
	}
	bn_t x1;
	bn_t y1;
	uint8_t *c1;
	c1 = C1 + 1;
	str_to_bn(x1, c1);
	c1 += 32;
	str_to_bn(y1, c1);
	eccpoint_t p1;
	bn_set(p1, x1);
	bn_set(p1 + array_len, y1);

	if (!is_eccpoint(p1))
	{
		return -2;
	}

	if (eccpoint_is_zero(p1))
	{
		return -2;
	}
	eccpoint_t p2;
	eccpoint_mult_jacobian(p2, p1, d);
	bn_to_str(x2, p2);
	bn_to_str(y2, p2 + array_len);

	uint8_t xy2[64];
    uint8_t *pxy2 = xy2;
    memcpy(pxy2, x2, 32);
    memcpy(pxy2 + 32, y2, 32);
	if(KDF(t, xy2, 64, mlen << 3)){
		return -1;
	}

	if (str_is_zero(t, mlen))
	{
		return 0;
	}

	for (i = 0; i < mlen; i++)
	{
		C2[i] = C[i + 65];
	}
	str_xor(M, C2, t, mlen);

    uint8_t *H3;
	if ((H3 = (uint8_t *)malloc(sizeof(uint8_t) * (mlen + 64))) == NULL)
        return -1;
	memcpy(H3, x2, 32);
	memcpy(H3 + 32, M, mlen);
	memcpy(H3 + 32 + mlen, y2, 32);

	sm3(H3, 64 + mlen, u);

	for (i = 0; i < 32; i++)
	{
		C3[i] = C[i + 65 + mlen];
	}

	if (0 != memcmp(u, C3, 32))
	{
		return -2;
	}

	return 0;
}

