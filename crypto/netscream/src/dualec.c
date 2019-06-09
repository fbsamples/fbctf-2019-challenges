
#include <openssl/bn.h>
#include <openssl/err.h>
#include <openssl/ec.h>
#include <openssl/evp.h>

#include "dualec.h"

static void fail(const char *file, int lineno, const char *line)
{
  fprintf(stderr, "%s:%d: %s\n", file, lineno, line);
  ERR_print_errors_fp(stderr);
  exit(1);
}

#ifdef DEBUG_ERROR
#define ENSURE(x) do { if ((x) <= 0) fail("drbg.c", __LINE__, #x); } while(0)
#else
#define ENSURE(x) x
#endif

static const uint8_t qx[32] =
{
  0xf6,0xc4,0xf7,0x66,0xb3,0xc6,0x1f,0x09,0xe6,0x09,0x58,0x22,
  0x24,0xcc,0x8e,0xbc,0xcf,0x4c,0xd4,0x96,0x1e,0xf7,0x80,0xcc,
  0x02,0xe8,0xf0,0x9a,0x0e,0xfa,0x7c,0xa5,
};

static const uint8_t qy[32] = {
  0xf2,0x12,0xc5,0x76,0x8d,0x46,0x71,0x6c,0x6c,0xac,0x4d,0x23,
  0xff,0x12,0xe8,0xae,0x89,0xfd,0x9e,0xee,0xc8,0x3a,0x0e,0x83,
  0xe3,0x5d,0xb3,0xaa,0xde,0x0c,0xcb,0x5b,
};

static EC_GROUP *group;
static EC_POINT *Q, *R;
static BIGNUM   *s, *x;
static BN_CTX   *bnctx;

void dualec_init(FILE* random) {
	ERR_load_crypto_strings();

	ENSURE(group = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1));

	ENSURE(bnctx = BN_CTX_new());

	ENSURE(Q = EC_POINT_new(group));
	ENSURE(R = EC_POINT_new(group));

	ENSURE(x = BN_new());
	ENSURE(s = BN_new());

	ENSURE(BN_bin2bn(qx, 32, x));
	ENSURE(BN_bin2bn(qy, 32, s));

	ENSURE(EC_POINT_set_affine_coordinates_GFp(group, Q, x, s, bnctx));

	uint8_t seed[32];
	fread(&seed, 1, sizeof(seed), random);
	ENSURE(BN_bin2bn(seed, 32, s));

}

void dualec_generate(uint8_t* buf, size_t size) {
	for(size_t i = 0; i < size;) {
		ENSURE(EC_POINT_mul(group, R, s, NULL, NULL, bnctx));
		ENSURE(EC_POINT_get_affine_coordinates_GFp(group, R, s, NULL, bnctx));

		ENSURE(EC_POINT_mul(group, R, NULL, Q, s, bnctx));
		ENSURE(EC_POINT_get_affine_coordinates_GFp(group, R, x, NULL, bnctx));

		if (BN_num_bytes(x) > 30) {
			ENSURE(BN_mask_bits(x, 30*8));
		}

		size_t nleft = size - i;

		if (nleft < 30) {
			ENSURE(BN_rshift(x, x, (30 - nleft) * 8));
			BN_bn2bin(x, &buf[i + (nleft - BN_num_bytes(x))]);
			i = size;
		} else {
			BN_bn2bin(x, &buf[i + (30 - BN_num_bytes(x))]);
			i += 30;
		}
	}
}
