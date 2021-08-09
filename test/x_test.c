/*
 * This program is released under the Common Public License V1.0
 *
 * You should have received a copy of Common Public License V1.0 along with
 * with this program.
 *
 * Copyright IBM Corp. 2019
 */

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <openssl/evp.h>

#include "ica_api.h"
#include "testcase.h"

#define ITERATIONS	1000

struct kat {
	const unsigned char *priv;
	const unsigned char *peer_pub;
	const unsigned char *shared_secret;
};

#ifndef NO_CPACF
static void check_functionlist(void);

static void x25519_kat(void);
static void x448_kat(void);
static void x25519_pc(void);
static void x448_pc(void);
#endif /* NO_CPACF */

int main(int argc, char *argv[])
{
#ifdef NO_CPACF
	UNUSED(argc);
	UNUSED(argv);
	printf("Skipping X25519/X448 test, because CPACF support disabled via config option.\n");
	return TEST_SKIP;
#else
	int i;

	set_verbosity(argc, argv);

	check_functionlist();

	VV_(printf("\n=== X25519 KAT ===\n"));
	x25519_kat();

	VV_(printf("\n=== X448 KAT ===\n"));
	x448_kat();

	VV_(printf("\n=== X25519 PC ===\n"));
	for (i = 0; i < ITERATIONS; i++)
		x25519_pc();

	VV_(printf("\n=== X448 PC ===\n"));
	for (i = 0; i < ITERATIONS; i++)
		x448_pc();
#endif /* NO_CPACF */
}

#ifndef NO_CPACF
static void check_functionlist(void)
{
	unsigned int i, listlen, func;
	libica_func_list_element *list;

	if (ica_get_functionlist(NULL, &listlen))
		EXIT_ERR("ica_get_functionlist failed.");

	func = 0;

	list = calloc(1, sizeof(*list) * listlen);
	if (list == NULL)
		EXIT_ERR("calloc failed.");

	if (ica_get_functionlist(list, &listlen))
		EXIT_ERR("ica_get_functionlist failed.");

	for (i = 0; i < listlen; i++) {
		if (list[i].mech_mode_id == X25519_KEYGEN
		    && (list[i].flags & 4))
			func |= 0x01;
		if (list[i].mech_mode_id == X25519_DERIVE
		    && (list[i].flags & 4))
			func |= 0x02;
		if (list[i].mech_mode_id == X448_KEYGEN
		    && (list[i].flags & 4))
			func |= 0x04;
		if (list[i].mech_mode_id == X448_DERIVE
		    && (list[i].flags & 4))
			func |= 0x08;
	}

	free(list);

	if (func != (0x01 | 0x02 | 0x04 | 0x08))
		exit(TEST_SKIP);
}

static void x25519_pc(void)
{
	ICA_X25519_CTX *ctx1 = NULL, *ctx2 = NULL;
	EVP_PKEY *pkey1 = NULL, *pkey2 = NULL;
	EVP_PKEY_CTX *pctx = NULL;
	unsigned char priv[32], pub[32], key1[32], key2[32];
	size_t keylen = 0;

	if (ica_x25519_ctx_new(&ctx1))
		EXIT_ERR("ica_x448_ctx_new failed.");
	if (ica_x25519_ctx_new(&ctx2))
		EXIT_ERR("ica_x448_ctx_new failed.");

	if (ica_x25519_key_gen(ctx1))
		EXIT_ERR("ica_x25519_key_gen failed.");
	if (ica_x25519_key_gen(ctx2))
		EXIT_ERR("ica_x25519_key_gen failed.");

	if (ica_x25519_key_get(ctx1, priv, NULL))
		EXIT_ERR("ica_x25519_key_get failed.");
	if (ica_x25519_key_get(ctx2, NULL, pub))
		EXIT_ERR("ica_x25519_key_get failed.");

	pkey1 = EVP_PKEY_new_raw_private_key(EVP_PKEY_X25519, NULL,
					     priv, sizeof(priv));
	if (pkey1 == NULL)
		EXIT_ERR("EVP_PKEY_new_raw_private_key failed.");

	pkey2 = EVP_PKEY_new_raw_public_key(EVP_PKEY_X25519, NULL, pub,
					    sizeof(pub));
	if (pkey2 == NULL)
		EXIT_ERR("EVP_PKEY_new_raw_private_key failed.");

	pctx = EVP_PKEY_CTX_new(pkey1, NULL);
	if (pctx == NULL)
		EXIT_ERR("EVP_PKEY_CTX_new failed.");

	if (EVP_PKEY_derive_init(pctx) != 1)
		EXIT_ERR("EVP_PKEY_derive_init failed.");

	if (EVP_PKEY_derive_set_peer(pctx, pkey2) != 1)
		EXIT_ERR("EVP_PKEY_derive_set_peer failed.");

	if (EVP_PKEY_derive(pctx, NULL, &keylen) != 1)
		EXIT_ERR("EVP_PKEY_derive failed.");

	if (EVP_PKEY_derive(pctx, key1, &keylen) != 1)
		EXIT_ERR("EVP_PKEY_derive failed.");

	if (ica_x25519_derive(ctx1, key2, pub) != 0)
		EXIT_ERR("ica_x25519_derive failed.");

	if (keylen != 32)
		EXIT_ERR("x25519 wrong shared secret size.");

	if (memcmp(key1, key2, keylen) != 0)
		EXIT_ERR("x25519 shared secrets do not match.");

	(void)ica_x25519_ctx_del(&ctx1);
	(void)ica_x25519_ctx_del(&ctx2);
	EVP_PKEY_free(pkey1);
	EVP_PKEY_free(pkey2);
	EVP_PKEY_CTX_free(pctx);
}

static void x448_pc(void)
{
	ICA_X448_CTX *ctx1 = NULL, *ctx2 = NULL;
	EVP_PKEY *pkey1 = NULL, *pkey2 = NULL;
	EVP_PKEY_CTX *pctx = NULL;
	unsigned char priv[56], pub[56], key1[56], key2[56];
	size_t keylen = 0;

	if (ica_x448_ctx_new(&ctx1))
		EXIT_ERR("ica_x448_ctx_new failed.");
	if (ica_x448_ctx_new(&ctx2))
		EXIT_ERR("ica_x448_ctx_new failed.");

	if (ica_x448_key_gen(ctx1))
		EXIT_ERR("ica_x448_key_gen failed.");
	if (ica_x448_key_gen(ctx2))
		EXIT_ERR("ica_x448_key_gen failed.");

	if (ica_x448_key_get(ctx1, priv, NULL))
		EXIT_ERR("ica_x448_key_get failed.");
	if (ica_x448_key_get(ctx2, NULL, pub))
		EXIT_ERR("ica_x448_key_get failed.");

	pkey1 = EVP_PKEY_new_raw_private_key(EVP_PKEY_X448, NULL,
					     priv, sizeof(priv));
	if (pkey1 == NULL)
		EXIT_ERR("EVP_PKEY_new_raw_private_key failed.");

	pkey2 = EVP_PKEY_new_raw_public_key(EVP_PKEY_X448, NULL, pub,
					    sizeof(pub));
	if (pkey2 == NULL)
		EXIT_ERR("EVP_PKEY_new_raw_private_key failed.");

	pctx = EVP_PKEY_CTX_new(pkey1, NULL);
	if (pctx == NULL)
		EXIT_ERR("EVP_PKEY_CTX_new failed.");

	if (EVP_PKEY_derive_init(pctx) != 1)
		EXIT_ERR("EVP_PKEY_derive_init failed.");

	if (EVP_PKEY_derive_set_peer(pctx, pkey2) != 1)
		EXIT_ERR("EVP_PKEY_derive_set_peer failed.");

	if (EVP_PKEY_derive(pctx, NULL, &keylen) != 1)
		EXIT_ERR("EVP_PKEY_derive failed.");

	if (EVP_PKEY_derive(pctx, key1, &keylen) != 1)
		EXIT_ERR("EVP_PKEY_derive failed.");

	if (ica_x448_derive(ctx1, key2, pub) != 0)
		EXIT_ERR("ica_x448_derive failed.");

	if (keylen != 56)
		EXIT_ERR("x448 wrong shared secret size.");

	if (memcmp(key1, key2, keylen) != 0)
		EXIT_ERR("x448 shared secrets do not match.");

	(void)ica_x448_ctx_del(&ctx1);
	(void)ica_x448_ctx_del(&ctx2);
	EVP_PKEY_free(pkey1);
	EVP_PKEY_free(pkey2);
	EVP_PKEY_CTX_free(pctx);
}

static void x25519_kat(void)
{
	struct kat vec[] = {
	{
	/* some wycheproof test vectors */
	.priv = (const unsigned char[]) {
	0x28, 0x87, 0x96, 0xbc, 0x5a, 0xff, 0x4b, 0x81, 0xa3, 0x75, 0x01, 0x75,
	0x7b, 0xc0, 0x75, 0x3a, 0x3c, 0x21, 0x96, 0x47, 0x90, 0xd3, 0x86, 0x99,
	0x30, 0x8d, 0xeb, 0xc1, 0x7a, 0x6e, 0xaf, 0x8d
	},
	.peer_pub = (const unsigned char[]) {
	0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f
	},
	.shared_secret = (const unsigned char[]) {
	0xb4, 0xe0, 0xdd, 0x76, 0xda, 0x7b, 0x07, 0x17, 0x28, 0xb6, 0x1f, 0x85,
	0x67, 0x71, 0xaa, 0x35, 0x6e, 0x57, 0xed, 0xa7, 0x8a, 0x5b, 0x16, 0x55,
	0xcc, 0x38, 0x20, 0xfb, 0x5f, 0x85, 0x4c, 0x5c
	},
	},
	{
	.priv = (const unsigned char[]) {
	0x60, 0x88, 0x7b, 0x3d, 0xc7, 0x24, 0x43, 0x02, 0x6e, 0xbe, 0xdb, 0xbb,
	0xb7, 0x06, 0x65, 0xf4, 0x2b, 0x87, 0xad, 0xd1, 0x44, 0x0e, 0x77, 0x68,
	0xfb, 0xd7, 0xe8, 0xe2, 0xce, 0x5f, 0x63, 0x9d
	},
	.peer_pub = (const unsigned char[]) {
	0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
	},
	.shared_secret = (const unsigned char[]) {
	0x38, 0xd6, 0x30, 0x4c, 0x4a, 0x7e, 0x6d, 0x9f, 0x79, 0x59, 0x33, 0x4f,
	0xb5, 0x24, 0x5b, 0xd2, 0xc7, 0x54, 0x52, 0x5d, 0x4c, 0x91, 0xdb, 0x95,
	0x02, 0x06, 0x92, 0x62, 0x34, 0xc1, 0xf6, 0x33
	},
	},
	{
	.priv = (const unsigned char[]) {
	0xa0, 0xa4, 0xf1, 0x30, 0xb9, 0x8a, 0x5b, 0xe4, 0xb1, 0xce, 0xdb, 0x7c,
	0xb8, 0x55, 0x84, 0xa3, 0x52, 0x0e, 0x14, 0x2d, 0x47, 0x4d, 0xc9, 0xcc,
	0xb9, 0x09, 0xa0, 0x73, 0xa9, 0x76, 0xbf, 0x63
	},
	.peer_pub = (const unsigned char[]) {
	0x0a, 0xb4, 0xe7, 0x63, 0x80, 0xd8, 0x4d, 0xde, 0x4f, 0x68, 0x33, 0xc5,
	0x8f, 0x2a, 0x9f, 0xb8, 0xf8, 0x3b, 0xb0, 0x16, 0x9b, 0x17, 0x2b, 0xe4,
	0xb6, 0xe0, 0x59, 0x28, 0x87, 0x74, 0x1a, 0x36
	},
	.shared_secret = (const unsigned char[]) {
	0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	},
	},
	{
	.priv = (const unsigned char[]) {
	0xa0, 0xa4, 0xf1, 0x30, 0xb9, 0x8a, 0x5b, 0xe4, 0xb1, 0xce, 0xdb, 0x7c,
	0xb8, 0x55, 0x84, 0xa3, 0x52, 0x0e, 0x14, 0x2d, 0x47, 0x4d, 0xc9, 0xcc,
	0xb9, 0x09, 0xa0, 0x73, 0xa9, 0x76, 0xbf, 0x63
	},
	.peer_pub = (const unsigned char[]) {
	0x89, 0xe1, 0x0d, 0x57, 0x01, 0xb4, 0x33, 0x7d, 0x2d, 0x03, 0x21, 0x81,
	0x53, 0x8b, 0x10, 0x64, 0xbd, 0x40, 0x84, 0x40, 0x1c, 0xec, 0xa1, 0xfd,
	0x12, 0x66, 0x3a, 0x19, 0x59, 0x38, 0x80, 0x00
	},
	.shared_secret = (const unsigned char[]) {
	0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	},
	},
	{
	.priv = (const unsigned char[]) {
	0xa0, 0xa4, 0xf1, 0x30, 0xb9, 0x8a, 0x5b, 0xe4, 0xb1, 0xce, 0xdb, 0x7c,
	0xb8, 0x55, 0x84, 0xa3, 0x52, 0x0e, 0x14, 0x2d, 0x47, 0x4d, 0xc9, 0xcc,
	0xb9, 0x09, 0xa0, 0x73, 0xa9, 0x76, 0xbf, 0x63
	},
	.peer_pub = (const unsigned char[]) {
	0x2b, 0x55, 0xd3, 0xaa, 0x4a, 0x8f, 0x80, 0xc8, 0xc0, 0xb2, 0xae, 0x5f,
	0x93, 0x3e, 0x85, 0xaf, 0x49, 0xbe, 0xac, 0x36, 0xc2, 0xfa, 0x73, 0x94,
	0xba, 0xb7, 0x6c, 0x89, 0x33, 0xf8, 0xf8, 0x1d
	},
	.shared_secret = (const unsigned char[]) {
	0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	},
	},
	};
	ICA_X25519_CTX *ctx = NULL;
	unsigned char shared_secret[32];
	size_t i = 0;

	if (ica_x25519_ctx_new(&ctx))
		EXIT_ERR("ica_x448_ctx_new failed.");

	for (i = 0; i < sizeof(vec) / sizeof(vec[0]); i++) {
		if (ica_x25519_key_set(ctx, vec[i].priv, NULL) != 0)
			EXIT_ERR("ica_x25519_key_set failed.");

		if (ica_x25519_derive(ctx,
				      shared_secret, vec[i].peer_pub) != 0)
			EXIT_ERR("ica_x25519_derive failed.");

		if (memcmp(shared_secret, vec[i].shared_secret, 32) != 0)
			EXIT_ERR("x25519 shared secrets do not match.");
	}

	(void)ica_x25519_ctx_del(&ctx);
}

static void x448_kat(void)
{
	struct kat vec[] = {
	{
	/* openssl test vectors */
	.priv = (const unsigned char[]) {
	0x9a, 0x8f, 0x49, 0x25, 0xd1, 0x51, 0x9f, 0x57, 0x75, 0xcf, 0x46, 0xb0,
	0x4b, 0x58, 0x00, 0xd4, 0xee, 0x9e, 0xe8, 0xba, 0xe8, 0xbc, 0x55, 0x65,
	0xd4, 0x98, 0xc2, 0x8d, 0xd9, 0xc9, 0xba, 0xf5, 0x74, 0xa9, 0x41, 0x97,
	0x44, 0x89, 0x73, 0x91, 0x00, 0x63, 0x82, 0xa6, 0xf1, 0x27, 0xab, 0x1d,
	0x9a, 0xc2, 0xd8, 0xc0, 0xa5, 0x98, 0x72, 0x6b
	},
	.peer_pub = (const unsigned char[]) {
	0x3e, 0xb7, 0xa8, 0x29, 0xb0, 0xcd, 0x20, 0xf5, 0xbc, 0xfc, 0x0b, 0x59,
	0x9b, 0x6f, 0xec, 0xcf, 0x6d, 0xa4, 0x62, 0x71, 0x07, 0xbd, 0xb0, 0xd4,
	0xf3, 0x45, 0xb4, 0x30, 0x27, 0xd8, 0xb9, 0x72, 0xfc, 0x3e, 0x34, 0xfb,
	0x42, 0x32, 0xa1, 0x3c, 0xa7, 0x06, 0xdc, 0xb5, 0x7a, 0xec, 0x3d, 0xae,
	0x07, 0xbd, 0xc1, 0xc6, 0x7b, 0xf3, 0x36, 0x09
	},
	.shared_secret = (const unsigned char[]) {
	0x07, 0xff, 0xf4, 0x18, 0x1a, 0xc6, 0xcc, 0x95, 0xec, 0x1c, 0x16, 0xa9,
	0x4a, 0x0f, 0x74, 0xd1, 0x2d, 0xa2, 0x32, 0xce, 0x40, 0xa7, 0x75, 0x52,
	0x28, 0x1d, 0x28, 0x2b, 0xb6, 0x0c, 0x0b, 0x56, 0xfd, 0x24, 0x64, 0xc3,
	0x35, 0x54, 0x39, 0x36, 0x52, 0x1c, 0x24, 0x40, 0x30, 0x85, 0xd5, 0x9a,
	0x44, 0x9a, 0x50, 0x37, 0x51, 0x4a, 0x87, 0x9d
	}
	},
	{
	.priv = (const unsigned char[]) {
	0x9a, 0x8f, 0x49, 0x25, 0xd1, 0x51, 0x9f, 0x57, 0x75, 0xcf, 0x46, 0xb0,
	0x4b, 0x58, 0x00, 0xd4, 0xee, 0x9e, 0xe8, 0xba, 0xe8, 0xbc, 0x55, 0x65,
	0xd4, 0x98, 0xc2, 0x8d, 0xd9, 0xc9, 0xba, 0xf5, 0x74, 0xa9, 0x41, 0x97,
	0x44, 0x89, 0x73, 0x91, 0x00, 0x63, 0x82, 0xa6, 0xf1, 0x27, 0xab, 0x1d,
	0x9a, 0xc2, 0xd8, 0xc0, 0xa5, 0x98, 0x72, 0x6b
	},
	.peer_pub = (const unsigned char[]) {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
	},
	.shared_secret = (const unsigned char[]) {
	0x66, 0xe2, 0xe6, 0x82, 0xb1, 0xf8, 0xe6, 0x8c, 0x80, 0x9f, 0x1b, 0xb3,
	0xe4, 0x06, 0xbd, 0x82, 0x69, 0x21, 0xd9, 0xc1, 0xa5, 0xbf, 0xbf, 0xcb,
	0xab, 0x7a, 0xe7, 0x2f, 0xee, 0xce, 0xe6, 0x36, 0x60, 0xea, 0xbd, 0x54,
	0x93, 0x4f, 0x33, 0x82, 0x06, 0x1d, 0x17, 0x60, 0x7f, 0x58, 0x1a, 0x90,
	0xbd, 0xac, 0x91, 0x7a, 0x06, 0x49, 0x59, 0xfb
	}
	}
	};
	ICA_X448_CTX *ctx = NULL;
	unsigned char shared_secret[56];
	size_t i = 0;

	if (ica_x448_ctx_new(&ctx))
		EXIT_ERR("ica_x448_ctx_new failed.");

	for (i = 0; i < sizeof(vec) / sizeof(vec[0]); i++) {
		if (ica_x448_key_set(ctx, vec[i].priv, NULL) != 0)
			EXIT_ERR("ica_x448_key_set failed.");

		if (ica_x448_derive(ctx,
				      shared_secret, vec[i].peer_pub) != 0)
			EXIT_ERR("ica_x448_derive failed.");

		if (memcmp(shared_secret, vec[i].shared_secret, 56) != 0)
			EXIT_ERR("x448 shared secrets do not match.");
	}

	(void)ica_x448_ctx_del(&ctx);
}
#endif /* NO_CPACF */
