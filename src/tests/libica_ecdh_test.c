/* This program is released under the Common Public License V1.0
 *
 * You should have received a copy of Common Public License V1.0 along with
 * with this program.
 */

/* Copyright IBM Corp. 2017 */

#include <fcntl.h>
#include <memory.h>
#include <sys/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "ica_api.h"
#include <sys/time.h>
#include "testcase.h"
#include <openssl/obj_mac.h>


#define NUM_ECDH_TESTS		(sizeof(ecdh_kats)/sizeof(ecdh_kat_t))
#define MAX_ECC_KEY_SIZE	66 /* 521 bits */

typedef struct {
	unsigned int nid;
	unsigned int privlen;
	unsigned char da[MAX_ECC_KEY_SIZE];
	unsigned char xa[MAX_ECC_KEY_SIZE];
	unsigned char ya[MAX_ECC_KEY_SIZE];
	unsigned char db[MAX_ECC_KEY_SIZE];
	unsigned char xb[MAX_ECC_KEY_SIZE];
	unsigned char yb[MAX_ECC_KEY_SIZE];
	unsigned char z[MAX_ECC_KEY_SIZE];
} ecdh_kat_t;

static ecdh_kat_t ecdh_kats[] = {
	/* Keys and shared secrets from RFC 5114 */
	{NID_X9_62_prime192v1, 24,
	 {0x32,0x3F,0xA3,0x16,0x9D,0x8E,0x9C,0x65,0x93,0xF5,0x94,0x76,0xBC,0x14,0x20,0x00,
	  0xAB,0x5B,0xE0,0xE2,0x49,0xC4,0x34,0x26,},
	 {0xCD,0x46,0x48,0x9E,0xCF,0xD6,0xC1,0x05,0xE7,0xB3,0xD3,0x25,0x66,0xE2,0xB1,0x22,
	  0xE2,0x49,0xAB,0xAA,0xDD,0x87,0x06,0x12,},
	 {0x68,0x88,0x7B,0x48,0x77,0xDF,0x51,0xDD,0x4D,0xC3,0xD6,0xFD,0x11,0xF0,0xA2,0x6F,
	  0x8F,0xD3,0x84,0x43,0x17,0x91,0x6E,0x9A,},
	 {0x63,0x1F,0x95,0xBB,0x4A,0x67,0x63,0x2C,0x9C,0x47,0x6E,0xEE,0x9A,0xB6,0x95,0xAB,
	  0x24,0x0A,0x04,0x99,0x30,0x7F,0xCF,0x62,},
	 {0x51,0x9A,0x12,0x16,0x80,0xE0,0x04,0x54,0x66,0xBA,0x21,0xDF,0x2E,0xEE,0x47,0xF5,
	  0x97,0x3B,0x50,0x05,0x77,0xEF,0x13,0xD5,},
	 {0xFF,0x61,0x3A,0xB4,0xD6,0x4C,0xEE,0x3A,0x20,0x87,0x5B,0xDB,0x10,0xF9,0x53,0xF6,
	  0xB3,0x0C,0xA0,0x72,0xC6,0x0A,0xA5,0x7F,},
	 {0xAD,0x42,0x01,0x82,0x63,0x3F,0x85,0x26,0xBF,0xE9,0x54,0xAC,0xDA,0x37,0x6F,0x05,
	  0xE5,0xFF,0x4F,0x83,0x7F,0x54,0xFE,0xBE,}
	},
	{NID_secp224r1, 28,
	 {0xB5,0x58,0xEB,0x6C,0x28,0x8D,0xA7,0x07,0xBB,0xB4,0xF8,0xFB,0xAE,0x2A,0xB9,0xE9,
	  0xCB,0x62,0xE3,0xBC,0x5C,0x75,0x73,0xE2,0x2E,0x26,0xD3,0x7F,},
	 {0x49,0xDF,0xEF,0x30,0x9F,0x81,0x48,0x8C,0x30,0x4C,0xFF,0x5A,0xB3,0xEE,0x5A,0x21,
	  0x54,0x36,0x7D,0xC7,0x83,0x31,0x50,0xE0,0xA5,0x1F,0x3E,0xEB,},
	 {0x4F,0x2B,0x5E,0xE4,0x57,0x62,0xC4,0xF6,0x54,0xC1,0xA0,0xC6,0x7F,0x54,0xCF,0x88,
	  0xB0,0x16,0xB5,0x1B,0xCE,0x3D,0x7C,0x22,0x8D,0x57,0xAD,0xB4,},
	 {0xAC,0x3B,0x1A,0xDD,0x3D,0x97,0x70,0xE6,0xF6,0xA7,0x08,0xEE,0x9F,0x3B,0x8E,0x0A,
	  0xB3,0xB4,0x80,0xE9,0xF2,0x7F,0x85,0xC8,0x8B,0x5E,0x6D,0x18,},
	 {0x6B,0x3A,0xC9,0x6A,0x8D,0x0C,0xDE,0x6A,0x55,0x99,0xBE,0x80,0x32,0xED,0xF1,0x0C,
	  0x16,0x2D,0x0A,0x8A,0xD2,0x19,0x50,0x6D,0xCD,0x42,0xA2,0x07,},
	 {0xD4,0x91,0xBE,0x99,0xC2,0x13,0xA7,0xD1,0xCA,0x37,0x06,0xDE,0xBF,0xE3,0x05,0xF3,
	  0x61,0xAF,0xCB,0xB3,0x3E,0x26,0x09,0xC8,0xB1,0x61,0x8A,0xD5,},
	 {0x52,0x27,0x2F,0x50,0xF4,0x6F,0x4E,0xDC,0x91,0x51,0x56,0x90,0x92,0xF4,0x6D,0xF2,
	  0xD9,0x6E,0xCC,0x3B,0x6D,0xC1,0x71,0x4A,0x4E,0xA9,0x49,0xFA,}
	 },
	{NID_X9_62_prime256v1, 32,
	 {0x81,0x42,0x64,0x14,0x5F,0x2F,0x56,0xF2,0xE9,0x6A,0x8E,0x33,0x7A,0x12,0x84,0x99,
	  0x3F,0xAF,0x43,0x2A,0x5A,0xBC,0xE5,0x9E,0x86,0x7B,0x72,0x91,0xD5,0x07,0xA3,0xAF,},
	 {0x2A,0xF5,0x02,0xF3,0xBE,0x89,0x52,0xF2,0xC9,0xB5,0xA8,0xD4,0x16,0x0D,0x09,0xE9,
	  0x71,0x65,0xBE,0x50,0xBC,0x42,0xAE,0x4A,0x5E,0x8D,0x3B,0x4B,0xA8,0x3A,0xEB,0x15,},
	 {0xEB,0x0F,0xAF,0x4C,0xA9,0x86,0xC4,0xD3,0x86,0x81,0xA0,0xF9,0x87,0x2D,0x79,0xD5,
	  0x67,0x95,0xBD,0x4B,0xFF,0x6E,0x6D,0xE3,0xC0,0xF5,0x01,0x5E,0xCE,0x5E,0xFD,0x85,},
	 {0x2C,0xE1,0x78,0x8E,0xC1,0x97,0xE0,0x96,0xDB,0x95,0xA2,0x00,0xCC,0x0A,0xB2,0x6A,
	  0x19,0xCE,0x6B,0xCC,0xAD,0x56,0x2B,0x8E,0xEE,0x1B,0x59,0x37,0x61,0xCF,0x7F,0x41,},
	 {0xB1,0x20,0xDE,0x4A,0xA3,0x64,0x92,0x79,0x53,0x46,0xE8,0xDE,0x6C,0x2C,0x86,0x46,
	  0xAE,0x06,0xAA,0xEA,0x27,0x9F,0xA7,0x75,0xB3,0xAB,0x07,0x15,0xF6,0xCE,0x51,0xB0,},
	 {0x9F,0x1B,0x7E,0xEC,0xE2,0x0D,0x7B,0x5E,0xD8,0xEC,0x68,0x5F,0xA3,0xF0,0x71,0xD8,
	  0x37,0x27,0x02,0x70,0x92,0xA8,0x41,0x13,0x85,0xC3,0x4D,0xDE,0x57,0x08,0xB2,0xB6,},
	 {0xDD,0x0F,0x53,0x96,0x21,0x9D,0x1E,0xA3,0x93,0x31,0x04,0x12,0xD1,0x9A,0x08,0xF1,
	  0xF5,0x81,0x1E,0x9D,0xC8,0xEC,0x8E,0xEA,0x7F,0x80,0xD2,0x1C,0x82,0x0C,0x27,0x88,},
	},
	{NID_secp384r1, 48,
	 {0xD2,0x73,0x35,0xEA,0x71,0x66,0x4A,0xF2,0x44,0xDD,0x14,0xE9,0xFD,0x12,0x60,0x71,
	  0x5D,0xFD,0x8A,0x79,0x65,0x57,0x1C,0x48,0xD7,0x09,0xEE,0x7A,0x79,0x62,0xA1,0x56,
	  0xD7,0x06,0xA9,0x0C,0xBC,0xB5,0xDF,0x29,0x86,0xF0,0x5F,0xEA,0xDB,0x93,0x76,0xF1,},
	 {0x79,0x31,0x48,0xF1,0x78,0x76,0x34,0xD5,0xDA,0x4C,0x6D,0x90,0x74,0x41,0x7D,0x05,
	  0xE0,0x57,0xAB,0x62,0xF8,0x20,0x54,0xD1,0x0E,0xE6,0xB0,0x40,0x3D,0x62,0x79,0x54,
	  0x7E,0x6A,0x8E,0xA9,0xD1,0xFD,0x77,0x42,0x7D,0x01,0x6F,0xE2,0x7A,0x8B,0x8C,0x66,},
	 {0xC6,0xC4,0x12,0x94,0x33,0x1D,0x23,0xE6,0xF4,0x80,0xF4,0xFB,0x4C,0xD4,0x05,0x04,
	  0xC9,0x47,0x39,0x2E,0x94,0xF4,0xC3,0xF0,0x6B,0x8F,0x39,0x8B,0xB2,0x9E,0x42,0x36,
	  0x8F,0x7A,0x68,0x59,0x23,0xDE,0x3B,0x67,0xBA,0xCE,0xD2,0x14,0xA1,0xA1,0xD1,0x28,},
	 {0x52,0xD1,0x79,0x1F,0xDB,0x4B,0x70,0xF8,0x9C,0x0F,0x00,0xD4,0x56,0xC2,0xF7,0x02,
	  0x3B,0x61,0x25,0x26,0x2C,0x36,0xA7,0xDF,0x1F,0x80,0x23,0x11,0x21,0xCC,0xE3,0xD3,
	  0x9B,0xE5,0x2E,0x00,0xC1,0x94,0xA4,0x13,0x2C,0x4A,0x6C,0x76,0x8B,0xCD,0x94,0xD2,},
	 {0x5C,0xD4,0x2A,0xB9,0xC4,0x1B,0x53,0x47,0xF7,0x4B,0x8D,0x4E,0xFB,0x70,0x8B,0x3D,
	  0x5B,0x36,0xDB,0x65,0x91,0x53,0x59,0xB4,0x4A,0xBC,0x17,0x64,0x7B,0x6B,0x99,0x99,
	  0x78,0x9D,0x72,0xA8,0x48,0x65,0xAE,0x2F,0x22,0x3F,0x12,0xB5,0xA1,0xAB,0xC1,0x20,},
	 {0xE1,0x71,0x45,0x8F,0xEA,0xA9,0x39,0xAA,0xA3,0xA8,0xBF,0xAC,0x46,0xB4,0x04,0xBD,
	  0x8F,0x6D,0x5B,0x34,0x8C,0x0F,0xA4,0xD8,0x0C,0xEC,0xA1,0x63,0x56,0xCA,0x93,0x32,
	  0x40,0xBD,0xE8,0x72,0x34,0x15,0xA8,0xEC,0xE0,0x35,0xB0,0xED,0xF3,0x67,0x55,0xDE,},
	 {0x5E,0xA1,0xFC,0x4A,0xF7,0x25,0x6D,0x20,0x55,0x98,0x1B,0x11,0x05,0x75,0xE0,0xA8,
	  0xCA,0xE5,0x31,0x60,0x13,0x7D,0x90,0x4C,0x59,0xD9,0x26,0xEB,0x1B,0x84,0x56,0xE4,
	  0x27,0xAA,0x8A,0x45,0x40,0x88,0x4C,0x37,0xDE,0x15,0x9A,0x58,0x02,0x8A,0xBC,0x0E,},
	},
	{NID_secp521r1, 66,
	 {0x01,0x13,0xF8,0x2D,0xA8,0x25,0x73,0x5E,0x3D,0x97,0x27,0x66,0x83,0xB2,0xB7,0x42,
	  0x77,0xBA,0xD2,0x73,0x35,0xEA,0x71,0x66,0x4A,0xF2,0x43,0x0C,0xC4,0xF3,0x34,0x59,
	  0xB9,0x66,0x9E,0xE7,0x8B,0x3F,0xFB,0x9B,0x86,0x83,0x01,0x5D,0x34,0x4D,0xCB,0xFE,
	  0xF6,0xFB,0x9A,0xF4,0xC6,0xC4,0x70,0xBE,0x25,0x45,0x16,0xCD,0x3C,0x1A,0x1F,0xB4,
	  0x73,0x62,},
	 {0x01,0xEB,0xB3,0x4D,0xD7,0x57,0x21,0xAB,0xF8,0xAD,0xC9,0xDB,0xED,0x17,0x88,0x9C,
	  0xBB,0x97,0x65,0xD9,0x0A,0x7C,0x60,0xF2,0xCE,0xF0,0x07,0xBB,0x0F,0x2B,0x26,0xE1,
	  0x48,0x81,0xFD,0x44,0x42,0xE6,0x89,0xD6,0x1C,0xB2,0xDD,0x04,0x6E,0xE3,0x0E,0x3F,
	  0xFD,0x20,0xF9,0xA4,0x5B,0xBD,0xF6,0x41,0x3D,0x58,0x3A,0x2D,0xBF,0x59,0x92,0x4F,
	  0xD3,0x5C,},
	 {0x00,0xF6,0xB6,0x32,0xD1,0x94,0xC0,0x38,0x8E,0x22,0xD8,0x43,0x7E,0x55,0x8C,0x55,
	  0x2A,0xE1,0x95,0xAD,0xFD,0x15,0x3F,0x92,0xD7,0x49,0x08,0x35,0x1B,0x2F,0x8C,0x4E,
	  0xDA,0x94,0xED,0xB0,0x91,0x6D,0x1B,0x53,0xC0,0x20,0xB5,0xEE,0xCA,0xED,0x1A,0x5F,
	  0xC3,0x8A,0x23,0x3E,0x48,0x30,0x58,0x7B,0xB2,0xEE,0x34,0x89,0xB3,0xB4,0x2A,0x5A,
	  0x86,0xA4,},
	 {0x00,0xCE,0xE3,0x48,0x0D,0x86,0x45,0xA1,0x7D,0x24,0x9F,0x27,0x76,0xD2,0x8B,0xAE,
	  0x61,0x69,0x52,0xD1,0x79,0x1F,0xDB,0x4B,0x70,0xF7,0xC3,0x37,0x87,0x32,0xAA,0x1B,
	  0x22,0x92,0x84,0x48,0xBC,0xD1,0xDC,0x24,0x96,0xD4,0x35,0xB0,0x10,0x48,0x06,0x6E,
	  0xBE,0x4F,0x72,0x90,0x3C,0x36,0x1B,0x1A,0x9D,0xC1,0x19,0x3D,0xC2,0xC9,0xD0,0x89,
	  0x1B,0x96,},
	 {0x01,0x0E,0xBF,0xAF,0xC6,0xE8,0x5E,0x08,0xD2,0x4B,0xFF,0xFC,0xC1,0xA4,0x51,0x1D,
	  0xB0,0xE6,0x34,0xBE,0xEB,0x1B,0x6D,0xEC,0x8C,0x59,0x39,0xAE,0x44,0x76,0x62,0x01,
	  0xAF,0x62,0x00,0x43,0x0B,0xA9,0x7C,0x8A,0xC6,0xA0,0xE9,0xF0,0x8B,0x33,0xCE,0x7E,
	  0x9F,0xEE,0xB5,0xBA,0x4E,0xE5,0xE0,0xD8,0x15,0x10,0xC2,0x42,0x95,0xB8,0xA0,0x8D,
	  0x02,0x35,},
	 {0x00,0xA4,0xA6,0xEC,0x30,0x0D,0xF9,0xE2,0x57,0xB0,0x37,0x2B,0x5E,0x7A,0xBF,0xEF,
	  0x09,0x34,0x36,0x71,0x9A,0x77,0x88,0x7E,0xBB,0x0B,0x18,0xCF,0x80,0x99,0xB9,0xF4,
	  0x21,0x2B,0x6E,0x30,0xA1,0x41,0x9C,0x18,0xE0,0x29,0xD3,0x68,0x63,0xCC,0x9D,0x44,
	  0x8F,0x4D,0xBA,0x4D,0x2A,0x0E,0x60,0x71,0x1B,0xE5,0x72,0x91,0x5F,0xBD,0x4F,0xEF,
	  0x26,0x95,},
	 {0x00,0xCD,0xEA,0x89,0x62,0x1C,0xFA,0x46,0xB1,0x32,0xF9,0xE4,0xCF,0xE2,0x26,0x1C,
	  0xDE,0x2D,0x43,0x68,0xEB,0x56,0x56,0x63,0x4C,0x7C,0xC9,0x8C,0x7A,0x00,0xCD,0xE5,
	  0x4E,0xD1,0x86,0x6A,0x0D,0xD3,0xE6,0x12,0x6C,0x9D,0x2F,0x84,0x5D,0xAF,0xF8,0x2C,
	  0xEB,0x1D,0xA0,0x8F,0x5D,0x87,0x52,0x1B,0xB0,0xEB,0xEC,0xA7,0x79,0x11,0x16,0x9C,
	  0x20,0xCC,},
	},
	/* Keys and shared secrets from RFC 5903 */
	{NID_X9_62_prime256v1, 32,
	 {0xC8,0x8F,0x01,0xF5,0x10,0xD9,0xAC,0x3F,0x70,0xA2,0x92,0xDA,0xA2,0x31,0x6D,0xE5,
	  0x44,0xE9,0xAA,0xB8,0xAF,0xE8,0x40,0x49,0xC6,0x2A,0x9C,0x57,0x86,0x2D,0x14,0x33,},
	 {0xDA,0xD0,0xB6,0x53,0x94,0x22,0x1C,0xF9,0xB0,0x51,0xE1,0xFE,0xCA,0x57,0x87,0xD0,
	  0x98,0xDF,0xE6,0x37,0xFC,0x90,0xB9,0xEF,0x94,0x5D,0x0C,0x37,0x72,0x58,0x11,0x80,},
	 {0x52,0x71,0xA0,0x46,0x1C,0xDB,0x82,0x52,0xD6,0x1F,0x1C,0x45,0x6F,0xA3,0xE5,0x9A,
	  0xB1,0xF4,0x5B,0x33,0xAC,0xCF,0x5F,0x58,0x38,0x9E,0x05,0x77,0xB8,0x99,0x0B,0xB3,},
	 {0xC6,0xEF,0x9C,0x5D,0x78,0xAE,0x01,0x2A,0x01,0x11,0x64,0xAC,0xB3,0x97,0xCE,0x20,
	  0x88,0x68,0x5D,0x8F,0x06,0xBF,0x9B,0xE0,0xB2,0x83,0xAB,0x46,0x47,0x6B,0xEE,0x53,},
	 {0xD1,0x2D,0xFB,0x52,0x89,0xC8,0xD4,0xF8,0x12,0x08,0xB7,0x02,0x70,0x39,0x8C,0x34,
	  0x22,0x96,0x97,0x0A,0x0B,0xCC,0xB7,0x4C,0x73,0x6F,0xC7,0x55,0x44,0x94,0xBF,0x63,},
	 {0x56,0xFB,0xF3,0xCA,0x36,0x6C,0xC2,0x3E,0x81,0x57,0x85,0x4C,0x13,0xC5,0x8D,0x6A,
	  0xAC,0x23,0xF0,0x46,0xAD,0xA3,0x0F,0x83,0x53,0xE7,0x4F,0x33,0x03,0x98,0x72,0xAB,},
	 {0xD6,0x84,0x0F,0x6B,0x42,0xF6,0xED,0xAF,0xD1,0x31,0x16,0xE0,0xE1,0x25,0x65,0x20,
	  0x2F,0xEF,0x8E,0x9E,0xCE,0x7D,0xCE,0x03,0x81,0x24,0x64,0xD0,0x4B,0x94,0x42,0xDE,}
	},
	{NID_secp384r1, 48,
	 {0x09,0x9F,0x3C,0x70,0x34,0xD4,0xA2,0xC6,0x99,0x88,0x4D,0x73,0xA3,0x75,0xA6,0x7F,
	  0x76,0x24,0xEF,0x7C,0x6B,0x3C,0x0F,0x16,0x06,0x47,0xB6,0x74,0x14,0xDC,0xE6,0x55,
	  0xE3,0x5B,0x53,0x80,0x41,0xE6,0x49,0xEE,0x3F,0xAE,0xF8,0x96,0x78,0x3A,0xB1,0x94,},
	 {0x66,0x78,0x42,0xD7,0xD1,0x80,0xAC,0x2C,0xDE,0x6F,0x74,0xF3,0x75,0x51,0xF5,0x57,
	  0x55,0xC7,0x64,0x5C,0x20,0xEF,0x73,0xE3,0x16,0x34,0xFE,0x72,0xB4,0xC5,0x5E,0xE6,
	  0xDE,0x3A,0xC8,0x08,0xAC,0xB4,0xBD,0xB4,0xC8,0x87,0x32,0xAE,0xE9,0x5F,0x41,0xAA,},
	 {0x94,0x82,0xED,0x1F,0xC0,0xEE,0xB9,0xCA,0xFC,0x49,0x84,0x62,0x5C,0xCF,0xC2,0x3F,
	  0x65,0x03,0x21,0x49,0xE0,0xE1,0x44,0xAD,0xA0,0x24,0x18,0x15,0x35,0xA0,0xF3,0x8E,
	  0xEB,0x9F,0xCF,0xF3,0xC2,0xC9,0x47,0xDA,0xE6,0x9B,0x4C,0x63,0x45,0x73,0xA8,0x1C,},
	 {0x41,0xCB,0x07,0x79,0xB4,0xBD,0xB8,0x5D,0x47,0x84,0x67,0x25,0xFB,0xEC,0x3C,0x94,
	  0x30,0xFA,0xB4,0x6C,0xC8,0xDC,0x50,0x60,0x85,0x5C,0xC9,0xBD,0xA0,0xAA,0x29,0x42,
	  0xE0,0x30,0x83,0x12,0x91,0x6B,0x8E,0xD2,0x96,0x0E,0x4B,0xD5,0x5A,0x74,0x48,0xFC,},
	 {0xE5,0x58,0xDB,0xEF,0x53,0xEE,0xCD,0xE3,0xD3,0xFC,0xCF,0xC1,0xAE,0xA0,0x8A,0x89,
	  0xA9,0x87,0x47,0x5D,0x12,0xFD,0x95,0x0D,0x83,0xCF,0xA4,0x17,0x32,0xBC,0x50,0x9D,
	  0x0D,0x1A,0xC4,0x3A,0x03,0x36,0xDE,0xF9,0x6F,0xDA,0x41,0xD0,0x77,0x4A,0x35,0x71,},
	 {0xDC,0xFB,0xEC,0x7A,0xAC,0xF3,0x19,0x64,0x72,0x16,0x9E,0x83,0x84,0x30,0x36,0x7F,
	  0x66,0xEE,0xBE,0x3C,0x6E,0x70,0xC4,0x16,0xDD,0x5F,0x0C,0x68,0x75,0x9D,0xD1,0xFF,
	  0xF8,0x3F,0xA4,0x01,0x42,0x20,0x9D,0xFF,0x5E,0xAA,0xD9,0x6D,0xB9,0xE6,0x38,0x6C,},
	 {0x11,0x18,0x73,0x31,0xC2,0x79,0x96,0x2D,0x93,0xD6,0x04,0x24,0x3F,0xD5,0x92,0xCB,
	  0x9D,0x0A,0x92,0x6F,0x42,0x2E,0x47,0x18,0x75,0x21,0x28,0x7E,0x71,0x56,0xC5,0xC4,
	  0xD6,0x03,0x13,0x55,0x69,0xB9,0xE9,0xD0,0x9C,0xF5,0xD4,0xA2,0x70,0xF5,0x97,0x46,}
	},
	{NID_secp521r1, 66,
	 {0x00,0x37,0xAD,0xE9,0x31,0x9A,0x89,0xF4,0xDA,0xBD,0xB3,0xEF,0x41,0x1A,0xAC,0xCC,
	  0xA5,0x12,0x3C,0x61,0xAC,0xAB,0x57,0xB5,0x39,0x3D,0xCE,0x47,0x60,0x81,0x72,0xA0,
	  0x95,0xAA,0x85,0xA3,0x0F,0xE1,0xC2,0x95,0x2C,0x67,0x71,0xD9,0x37,0xBA,0x97,0x77,
	  0xF5,0x95,0x7B,0x26,0x39,0xBA,0xB0,0x72,0x46,0x2F,0x68,0xC2,0x7A,0x57,0x38,0x2D,
	  0x4A,0x52,},
	 {0x00,0x15,0x41,0x7E,0x84,0xDB,0xF2,0x8C,0x0A,0xD3,0xC2,0x78,0x71,0x33,0x49,0xDC,
	  0x7D,0xF1,0x53,0xC8,0x97,0xA1,0x89,0x1B,0xD9,0x8B,0xAB,0x43,0x57,0xC9,0xEC,0xBE,
	  0xE1,0xE3,0xBF,0x42,0xE0,0x0B,0x8E,0x38,0x0A,0xEA,0xE5,0x7C,0x2D,0x10,0x75,0x64,
	  0x94,0x18,0x85,0x94,0x2A,0xF5,0xA7,0xF4,0x60,0x17,0x23,0xC4,0x19,0x5D,0x17,0x6C,
	  0xED,0x3E,},
	 {0x01,0x7C,0xAE,0x20,0xB6,0x64,0x1D,0x2E,0xEB,0x69,0x57,0x86,0xD8,0xC9,0x46,0x14,
	  0x62,0x39,0xD0,0x99,0xE1,0x8E,0x1D,0x5A,0x51,0x4C,0x73,0x9D,0x7C,0xB4,0xA1,0x0A,
	  0xD8,0xA7,0x88,0x01,0x5A,0xC4,0x05,0xD7,0x79,0x9D,0xC7,0x5E,0x7B,0x7D,0x5B,0x6C,
	  0xF2,0x26,0x1A,0x6A,0x7F,0x15,0x07,0x43,0x8B,0xF0,0x1B,0xEB,0x6C,0xA3,0x92,0x6F,
	  0x95,0x82,},
	 {0x01,0x45,0xBA,0x99,0xA8,0x47,0xAF,0x43,0x79,0x3F,0xDD,0x0E,0x87,0x2E,0x7C,0xDF,
	  0xA1,0x6B,0xE3,0x0F,0xDC,0x78,0x0F,0x97,0xBC,0xCC,0x3F,0x07,0x83,0x80,0x20,0x1E,
	  0x9C,0x67,0x7D,0x60,0x0B,0x34,0x37,0x57,0xA3,0xBD,0xBF,0x2A,0x31,0x63,0xE4,0xC2,
	  0xF8,0x69,0xCC,0xA7,0x45,0x8A,0xA4,0xA4,0xEF,0xFC,0x31,0x1F,0x5C,0xB1,0x51,0x68,
	  0x5E,0xB9,},
	 {0x00,0xD0,0xB3,0x97,0x5A,0xC4,0xB7,0x99,0xF5,0xBE,0xA1,0x6D,0x5E,0x13,0xE9,0xAF,
	  0x97,0x1D,0x5E,0x9B,0x98,0x4C,0x9F,0x39,0x72,0x8B,0x5E,0x57,0x39,0x73,0x5A,0x21,
	  0x9B,0x97,0xC3,0x56,0x43,0x6A,0xDC,0x6E,0x95,0xBB,0x03,0x52,0xF6,0xBE,0x64,0xA6,
	  0xC2,0x91,0x2D,0x4E,0xF2,0xD0,0x43,0x3C,0xED,0x2B,0x61,0x71,0x64,0x00,0x12,0xD9,
	  0x46,0x0F,},
	 {0x01,0x5C,0x68,0x22,0x63,0x83,0x95,0x6E,0x3B,0xD0,0x66,0xE7,0x97,0xB6,0x23,0xC2,
	  0x7C,0xE0,0xEA,0xC2,0xF5,0x51,0xA1,0x0C,0x2C,0x72,0x4D,0x98,0x52,0x07,0x7B,0x87,
	  0x22,0x0B,0x65,0x36,0xC5,0xC4,0x08,0xA1,0xD2,0xAE,0xBB,0x8E,0x86,0xD6,0x78,0xAE,
	  0x49,0xCB,0x57,0x09,0x1F,0x47,0x32,0x29,0x65,0x79,0xAB,0x44,0xFC,0xD1,0x7F,0x0F,
	  0xC5,0x6A,},
	 {0x01,0x14,0x4C,0x7D,0x79,0xAE,0x69,0x56,0xBC,0x8E,0xDB,0x8E,0x7C,0x78,0x7C,0x45,
	  0x21,0xCB,0x08,0x6F,0xA6,0x44,0x07,0xF9,0x78,0x94,0xE5,0xE6,0xB2,0xD7,0x9B,0x04,
	  0xD1,0x42,0x7E,0x73,0xCA,0x4B,0xAA,0x24,0x0A,0x34,0x78,0x68,0x59,0x81,0x0C,0x06,
	  0xB3,0xC7,0x15,0xA3,0xA8,0xCC,0x31,0x51,0xF2,0xBE,0xE4,0x17,0x99,0x6D,0x19,0xF3,
	  0xDD,0xEA,}
	},
	/* Keys and shared secrets from RFC 7027 */
	{NID_brainpoolP256r1, 32,
	 {0x81,0xDB,0x1E,0xE1,0x00,0x15,0x0F,0xF2,0xEA,0x33,0x8D,0x70,0x82,0x71,0xBE,0x38,
	  0x30,0x0C,0xB5,0x42,0x41,0xD7,0x99,0x50,0xF7,0x7B,0x06,0x30,0x39,0x80,0x4F,0x1D,},
	 {0x44,0x10,0x6E,0x91,0x3F,0x92,0xBC,0x02,0xA1,0x70,0x5D,0x99,0x53,0xA8,0x41,0x4D,
	  0xB9,0x5E,0x1A,0xAA,0x49,0xE8,0x1D,0x9E,0x85,0xF9,0x29,0xA8,0xE3,0x10,0x0B,0xE5,},
	 {0x8A,0xB4,0x84,0x6F,0x11,0xCA,0xCC,0xB7,0x3C,0xE4,0x9C,0xBD,0xD1,0x20,0xF5,0xA9,
	  0x00,0xA6,0x9F,0xD3,0x2C,0x27,0x22,0x23,0xF7,0x89,0xEF,0x10,0xEB,0x08,0x9B,0xDC,},
	 {0x55,0xE4,0x0B,0xC4,0x1E,0x37,0xE3,0xE2,0xAD,0x25,0xC3,0xC6,0x65,0x45,0x11,0xFF,
	  0xA8,0x47,0x4A,0x91,0xA0,0x03,0x20,0x87,0x59,0x38,0x52,0xD3,0xE7,0xD7,0x6B,0xD3,},
	 {0x8D,0x2D,0x68,0x8C,0x6C,0xF9,0x3E,0x11,0x60,0xAD,0x04,0xCC,0x44,0x29,0x11,0x7D,
	  0xC2,0xC4,0x18,0x25,0xE1,0xE9,0xFC,0xA0,0xAD,0xDD,0x34,0xE6,0xF1,0xB3,0x9F,0x7B,},
	 {0x99,0x0C,0x57,0x52,0x08,0x12,0xBE,0x51,0x26,0x41,0xE4,0x70,0x34,0x83,0x21,0x06,
	  0xBC,0x7D,0x3E,0x8D,0xD0,0xE4,0xC7,0xF1,0x13,0x6D,0x70,0x06,0x54,0x7C,0xEC,0x6A,},
	 {0x89,0xAF,0xC3,0x9D,0x41,0xD3,0xB3,0x27,0x81,0x4B,0x80,0x94,0x0B,0x04,0x25,0x90,
	  0xF9,0x65,0x56,0xEC,0x91,0xE6,0xAE,0x79,0x39,0xBC,0xE3,0x1F,0x3A,0x18,0xBF,0x2B,},
	},
	{NID_brainpoolP384r1, 48,
	 {0x1E,0x20,0xF5,0xE0,0x48,0xA5,0x88,0x6F,0x1F,0x15,0x7C,0x74,0xE9,0x1B,0xDE,0x2B,
	  0x98,0xC8,0xB5,0x2D,0x58,0xE5,0x00,0x3D,0x57,0x05,0x3F,0xC4,0xB0,0xBD,0x65,0xD6,
	  0xF1,0x5E,0xB5,0xD1,0xEE,0x16,0x10,0xDF,0x87,0x07,0x95,0x14,0x36,0x27,0xD0,0x42,},
	 {0x68,0xB6,0x65,0xDD,0x91,0xC1,0x95,0x80,0x06,0x50,0xCD,0xD3,0x63,0xC6,0x25,0xF4,
	  0xE7,0x42,0xE8,0x13,0x46,0x67,0xB7,0x67,0xB1,0xB4,0x76,0x79,0x35,0x88,0xF8,0x85,
	  0xAB,0x69,0x8C,0x85,0x2D,0x4A,0x6E,0x77,0xA2,0x52,0xD6,0x38,0x0F,0xCA,0xF0,0x68,},
	 {0x55,0xBC,0x91,0xA3,0x9C,0x9E,0xC0,0x1D,0xEE,0x36,0x01,0x7B,0x7D,0x67,0x3A,0x93,
	  0x12,0x36,0xD2,0xF1,0xF5,0xC8,0x39,0x42,0xD0,0x49,0xE3,0xFA,0x20,0x60,0x74,0x93,
	  0xE0,0xD0,0x38,0xFF,0x2F,0xD3,0x0C,0x2A,0xB6,0x7D,0x15,0xC8,0x5F,0x7F,0xAA,0x59,},
	 {0x03,0x26,0x40,0xBC,0x60,0x03,0xC5,0x92,0x60,0xF7,0x25,0x0C,0x3D,0xB5,0x8C,0xE6,
	  0x47,0xF9,0x8E,0x12,0x60,0xAC,0xCE,0x4A,0xCD,0xA3,0xDD,0x86,0x9F,0x74,0xE0,0x1F,
	  0x8B,0xA5,0xE0,0x32,0x43,0x09,0xDB,0x6A,0x98,0x31,0x49,0x7A,0xBA,0xC9,0x66,0x70,},
	 {0x4D,0x44,0x32,0x6F,0x26,0x9A,0x59,0x7A,0x5B,0x58,0xBB,0xA5,0x65,0xDA,0x55,0x56,
	  0xED,0x7F,0xD9,0xA8,0xA9,0xEB,0x76,0xC2,0x5F,0x46,0xDB,0x69,0xD1,0x9D,0xC8,0xCE,
	  0x6A,0xD1,0x8E,0x40,0x4B,0x15,0x73,0x8B,0x20,0x86,0xDF,0x37,0xE7,0x1D,0x1E,0xB4,},
	 {0x62,0xD6,0x92,0x13,0x6D,0xE5,0x6C,0xBE,0x93,0xBF,0x5F,0xA3,0x18,0x8E,0xF5,0x8B,
	  0xC8,0xA3,0xA0,0xEC,0x6C,0x1E,0x15,0x1A,0x21,0x03,0x8A,0x42,0xE9,0x18,0x53,0x29,
	  0xB5,0xB2,0x75,0x90,0x3D,0x19,0x2F,0x8D,0x4E,0x1F,0x32,0xFE,0x9C,0xC7,0x8C,0x48,},
	 {0x0B,0xD9,0xD3,0xA7,0xEA,0x0B,0x3D,0x51,0x9D,0x09,0xD8,0xE4,0x8D,0x07,0x85,0xFB,
	  0x74,0x4A,0x6B,0x35,0x5E,0x63,0x04,0xBC,0x51,0xC2,0x29,0xFB,0xBC,0xE2,0x39,0xBB,
	  0xAD,0xF6,0x40,0x37,0x15,0xC3,0x5D,0x4F,0xB2,0xA5,0x44,0x4F,0x57,0x5D,0x4F,0x42,},
	},
	{NID_brainpoolP512r1, 64,
	 {0x16,0x30,0x2F,0xF0,0xDB,0xBB,0x5A,0x8D,0x73,0x3D,0xAB,0x71,0x41,0xC1,0xB4,0x5A,
	  0xCB,0xC8,0x71,0x59,0x39,0x67,0x7F,0x6A,0x56,0x85,0x0A,0x38,0xBD,0x87,0xBD,0x59,
	  0xB0,0x9E,0x80,0x27,0x96,0x09,0xFF,0x33,0x3E,0xB9,0xD4,0xC0,0x61,0x23,0x1F,0xB2,
	  0x6F,0x92,0xEE,0xB0,0x49,0x82,0xA5,0xF1,0xD1,0x76,0x4C,0xAD,0x57,0x66,0x54,0x22,},
	 {0x0A,0x42,0x05,0x17,0xE4,0x06,0xAA,0xC0,0xAC,0xDC,0xE9,0x0F,0xCD,0x71,0x48,0x77,
	  0x18,0xD3,0xB9,0x53,0xEF,0xD7,0xFB,0xEC,0x5F,0x7F,0x27,0xE2,0x8C,0x61,0x49,0x99,
	  0x93,0x97,0xE9,0x1E,0x02,0x9E,0x06,0x45,0x7D,0xB2,0xD3,0xE6,0x40,0x66,0x8B,0x39,
	  0x2C,0x2A,0x7E,0x73,0x7A,0x7F,0x0B,0xF0,0x44,0x36,0xD1,0x16,0x40,0xFD,0x09,0xFD,},
	 {0x72,0xE6,0x88,0x2E,0x8D,0xB2,0x8A,0xAD,0x36,0x23,0x7C,0xD2,0x5D,0x58,0x0D,0xB2,
	  0x37,0x83,0x96,0x1C,0x8D,0xC5,0x2D,0xFA,0x2E,0xC1,0x38,0xAD,0x47,0x2A,0x0F,0xCE,
	  0xF3,0x88,0x7C,0xF6,0x2B,0x62,0x3B,0x2A,0x87,0xDE,0x5C,0x58,0x83,0x01,0xEA,0x3E,
	  0x5F,0xC2,0x69,0xB3,0x73,0xB6,0x07,0x24,0xF5,0xE8,0x2A,0x6A,0xD1,0x47,0xFD,0xE7,},
	 {0x23,0x0E,0x18,0xE1,0xBC,0xC8,0x8A,0x36,0x2F,0xA5,0x4E,0x4E,0xA3,0x90,0x20,0x09,
	  0x29,0x2F,0x7F,0x80,0x33,0x62,0x4F,0xD4,0x71,0xB5,0xD8,0xAC,0xE4,0x9D,0x12,0xCF,
	  0xAB,0xBC,0x19,0x96,0x3D,0xAB,0x8E,0x2F,0x1E,0xBA,0x00,0xBF,0xFB,0x29,0xE4,0xD7,
	  0x2D,0x13,0xF2,0x22,0x45,0x62,0xF4,0x05,0xCB,0x80,0x50,0x36,0x66,0xB2,0x54,0x29,},
	 {0x9D,0x45,0xF6,0x6D,0xE5,0xD6,0x7E,0x2E,0x6D,0xB6,0xE9,0x3A,0x59,0xCE,0x0B,0xB4,
	  0x81,0x06,0x09,0x7F,0xF7,0x8A,0x08,0x1D,0xE7,0x81,0xCD,0xB3,0x1F,0xCE,0x8C,0xCB,
	  0xAA,0xEA,0x8D,0xD4,0x32,0x0C,0x41,0x19,0xF1,0xE9,0xCD,0x43,0x7A,0x2E,0xAB,0x37,
	  0x31,0xFA,0x96,0x68,0xAB,0x26,0x8D,0x87,0x1D,0xED,0xA5,0x5A,0x54,0x73,0x19,0x9F,},
	 {0x2F,0xDC,0x31,0x30,0x95,0xBC,0xDD,0x5F,0xB3,0xA9,0x16,0x36,0xF0,0x7A,0x95,0x9C,
	  0x8E,0x86,0xB5,0x63,0x6A,0x1E,0x93,0x0E,0x83,0x96,0x04,0x9C,0xB4,0x81,0x96,0x1D,
	  0x36,0x5C,0xC1,0x14,0x53,0xA0,0x6C,0x71,0x98,0x35,0x47,0x5B,0x12,0xCB,0x52,0xFC,
	  0x3C,0x38,0x3B,0xCE,0x35,0xE2,0x7E,0xF1,0x94,0x51,0x2B,0x71,0x87,0x62,0x85,0xFA,},
	 {0xA7,0x92,0x70,0x98,0x65,0x5F,0x1F,0x99,0x76,0xFA,0x50,0xA9,0xD5,0x66,0x86,0x5D,
	  0xC5,0x30,0x33,0x18,0x46,0x38,0x1C,0x87,0x25,0x6B,0xAF,0x32,0x26,0x24,0x4B,0x76,
	  0xD3,0x64,0x03,0xC0,0x24,0xD7,0xBB,0xF0,0xAA,0x08,0x03,0xEA,0xFF,0x40,0x5D,0x3D,
	  0x24,0xF1,0x1A,0x9B,0x5C,0x0B,0xEF,0x67,0x9F,0xE1,0x45,0x4B,0x21,0xC4,0xCD,0x1F,},
	},
};


int main(int argc, char **argv)
{
	ica_adapter_handle_t adapter_handle;
	unsigned int i, rc;
	unsigned int errors=0, test_failed=0;
	unsigned char shared_secret[MAX_ECC_KEY_SIZE];
	unsigned int privlen = 0;
	ICA_EC_KEY *eckey_A, *eckey_B;


	set_verbosity(argc, argv);

	rc = ica_open_adapter(&adapter_handle);
	if (rc != 0) {
		V_(printf("ica_open_adapter failed and returned %d (0x%x).\n", rc, rc));
		return 8;
	}

	/* Iterate over curves */
	for (i = 0; i < NUM_ECDH_TESTS; i++) {

		V_(printf("Testing curve %d \n", ecdh_kats[i].nid));

		test_failed = 0;
		memset(shared_secret, 0, sizeof(shared_secret));

		eckey_A = ica_ec_key_new(ecdh_kats[i].nid, &privlen);
		rc = ica_ec_key_init(ecdh_kats[i].xa, ecdh_kats[i].ya, ecdh_kats[i].da, eckey_A);

		eckey_B = ica_ec_key_new(ecdh_kats[i].nid, &privlen);
		rc = ica_ec_key_init(ecdh_kats[i].xb, ecdh_kats[i].yb, ecdh_kats[i].db, eckey_B);

		/* calculate shared secret with priv_A, pub_B */
		rc = ica_ecdh_derive_secret(adapter_handle, eckey_A, eckey_B,
								shared_secret, privlen);
		if (rc) {
			V_(printf("Shared secret could not be derived, rc=%i.\n",rc));
			test_failed = 1;
		} else {

			/* compare result with known result */
			if (memcmp(shared_secret, ecdh_kats[i].z, ecdh_kats[i].privlen) != 0) {
				V_(printf("Check 1: priv_A, pub_B: Results do not match.\n"));
				VV_(printf("Expected result:\n"));
				dump_array(ecdh_kats[i].z, privlen);
				VV_(printf("Calculated result:\n"));
				dump_array(shared_secret, privlen);
				test_failed = 1;
			}

			/* calculate shared secret with priv_B, pub_A */
			rc = ica_ecdh_derive_secret(adapter_handle, eckey_B, eckey_A,
									shared_secret, privlen);
			if (rc) {
				V_(printf("Shared secret could not be derived, rc=%i.\n",rc));
				test_failed = 1;
			} else {

				/* compare result with known result */
				if (memcmp(shared_secret, ecdh_kats[i].z, ecdh_kats[i].privlen) != 0) {
					V_(printf("Check 2: pub(B), priv(A): Results do not match.\n"));
					VV_(printf("Expected result:\n"));
					dump_array(ecdh_kats[i].z, privlen);
					VV_(printf("Calculated result:\n"));
					dump_array(shared_secret, privlen);
					test_failed = 1;
				}
			}
		}

		if (test_failed)
			errors++;

		ica_ec_key_free(eckey_A);
		ica_ec_key_free(eckey_B);

	}

	ica_close_adapter(adapter_handle);

	if (errors)
		printf("%i of %li ECDH tests failed.\n", errors, NUM_ECDH_TESTS);
	else
		printf("All ECDH tests passed.\n");

	return 0;
}
