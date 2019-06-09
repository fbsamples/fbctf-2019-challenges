#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <openssl/aes.h>
#include <openssl/bn.h>
#include <openssl/err.h>
#include <openssl/ec.h>
#include <openssl/evp.h>

static void fail(const char *file, int lineno, const char *line)
{
  fprintf(stderr, "%s:%d: %s\n", file, lineno, line);
  ERR_print_errors_fp(stderr);
  exit(1);
}

#ifdef DEBUG_ERROR
#define ENSURE(x) do { if ((x) <= 0) fail(__FILE__, __LINE__, #x); } while(0)
#else
#define ENSURE(x) x
#endif

struct file {
    uint8_t* buf;
    size_t size;
};

static EC_GROUP *group;
static BN_CTX *bnctx;

struct file load_file(const char* fname) {

    // open 'd' value
    FILE* fid = fopen(fname, "r");
    if (fid == NULL) {
        struct file out = { NULL, 0 };
        return out;
    }

    // get length
    fseek(fid, 0, SEEK_END);
    const long fsize = ftell(fid);
    fseek(fid, 0, SEEK_SET);

    // read in the file
    uint8_t* buf = calloc(fsize + 1, 1);
    fread(buf, fsize, 1, fid);
    fclose(fid);
    buf[fsize] = '\0'; // null terminate in case it's a text file

    // return the struct with the buf and size
    struct file out = { buf, fsize };
    return out;
}

void ec_init() {
    ERR_load_crypto_strings();
    ENSURE(group = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1));
    ENSURE(bnctx = BN_CTX_new());
}

EC_POINT* find_Q(BIGNUM* d) {
    // get e = d^(-1) mod n, where n is curve order
    BIGNUM *order;
    ENSURE(order = BN_new());
    ENSURE(EC_GROUP_get_order(group, order, bnctx));

    BIGNUM *e;
    ENSURE(e = BN_mod_inverse(NULL, d, order, bnctx));

    // calculate Q = e*P
    EC_POINT *Q;
    ENSURE(Q = EC_POINT_new(group));
    ENSURE(EC_POINT_mul(group, Q, e, NULL, NULL, bnctx));

    BN_free(order);
    BN_free(e);

    return Q;
}

void print_point(EC_POINT* Q) {
    BIGNUM *qx, *qy;
    ENSURE(qx = BN_new());
    ENSURE(qy = BN_new());

    ENSURE(EC_POINT_get_affine_coordinates_GFp(group, Q, qx, qy, bnctx));

    char *qx_hex = BN_bn2hex(qx);
    char *qy_hex = BN_bn2hex(qy);

    printf("x: %s\ny: %s\n", qx_hex, qy_hex);

    // cleanup
    OPENSSL_free(qx_hex);
    OPENSSL_free(qy_hex);
    BN_free(qx);
    BN_free(qy);
}

BIGNUM* get_first_30(struct file enc_file) {
    if (enc_file.size < 32) {
        fprintf(
            stderr,
            "error: enc file is too small (must be at least 32 bytes): %lu",
            enc_file.size
        );
        exit(3);
    }

    BIGNUM* iv = NULL;
    ENSURE(iv = BN_bin2bn(enc_file.buf, 30, NULL));
    return iv;
}

bool guess_is_correct(EC_POINT *S, uint8_t *next_two, EC_POINT *Q) {
    BIGNUM *s;
    ENSURE(s = BN_new());
    ENSURE(EC_POINT_get_affine_coordinates_GFp(group, S, s, NULL, bnctx));

    EC_POINT *R;
    ENSURE(R = EC_POINT_new(group));
    ENSURE(EC_POINT_mul(group, R, NULL, Q, s, bnctx));

    BIGNUM *r;
    ENSURE(r = BN_new());
    ENSURE(EC_POINT_get_affine_coordinates_GFp(group, R, r, NULL, bnctx));
    ENSURE(BN_mask_bits(r, 30*8));
    ENSURE(BN_rshift(r, r, 28*8));

    assert(BN_num_bytes(r) <= 2);
    uint8_t test_two[2];
    ENSURE(BN_bn2bin(r, test_two));

    return memcmp(next_two, test_two, 2) == 0;
}

BIGNUM* recover_state_from_iv(BIGNUM *r, uint8_t *next_two, EC_POINT *Q, BIGNUM* d) {
    BIGNUM *inc;
    ENSURE(inc = BN_new());
    ENSURE(BN_zero(inc));
    ENSURE(BN_set_bit(inc, 30*8));

    EC_POINT *R, *S;
    ENSURE(R = EC_POINT_new(group));
    ENSURE(S = EC_POINT_new(group));

    bool found = false;
    for (size_t i = 0; i < 0x10000; ++i, BN_add(r, r, inc)) {
        if(!EC_POINT_set_compressed_coordinates_GFp(group, R, r, 0, bnctx)) {
            // i||r isn't a valid x coordinate, try the next one.
            continue;
        }
        // R now holds a potential valid pre-state value
        ENSURE(EC_POINT_mul(group, S, NULL, R, d, bnctx));
        if (guess_is_correct(S, next_two, Q)) {
            found = true;
            break;
        }
    }

    BIGNUM *out = NULL;
    if (found) {
        ENSURE(out = BN_new());
        ENSURE(EC_POINT_get_affine_coordinates_GFp(group, S, out, NULL, bnctx));
    }

    BN_free(inc);
    EC_POINT_free(R);
    EC_POINT_free(S);

    return out;
}

BIGNUM* dualec_churn_state(BIGNUM *s) {
    EC_POINT *S1;
    ENSURE(S1 = EC_POINT_new(group));
    ENSURE(EC_POINT_mul(group, S1, s, NULL, NULL, bnctx));

    BIGNUM *s1;
    ENSURE(s1 = BN_new());
    ENSURE(EC_POINT_get_affine_coordinates_GFp(group, S1, s1, NULL, bnctx));

    return s1;
}

BIGNUM* dualec_generate(BIGNUM *s, EC_POINT *Q) {
    EC_POINT *R = EC_POINT_new(group);
    BIGNUM *r = BN_new();
    ENSURE(EC_POINT_mul(group, R, NULL, Q, s, bnctx));
    ENSURE(EC_POINT_get_affine_coordinates_GFp(group, R, r, NULL, bnctx));
    ENSURE(BN_mask_bits(r, 30*8));
    return r;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s d enc\n", argv[0]);
        return 1;
    }

    struct file d_hex_file = load_file(argv[1]);
    struct file enc_file = load_file(argv[2]);

    ec_init();

    BIGNUM *d = NULL;
    ENSURE(BN_hex2bn(&d, (const char*)d_hex_file.buf));

    EC_POINT *Q = find_Q(d);

    printf("point Q:\n");
    print_point(Q);

    BIGNUM *r = get_first_30(enc_file);
    uint8_t *next_two = &enc_file.buf[30];

    BIGNUM* s = recover_state_from_iv(r, next_two, Q, d);

    if (s == NULL) {
        fprintf(stderr, "could not recover state\n");
        exit(4);
    }

    // s is pointing to the last two of the iv, lets churn and point it to key
    s = dualec_churn_state(s);
    BIGNUM *r1 = dualec_generate(s, Q); // 30 bytes
    s = dualec_churn_state(s);
    BIGNUM *r2 = dualec_generate(s, Q); // 30 bytes
    BN_rshift(r2, r2, 28*8); // get the 2 msbytes

    uint8_t key[32];
    assert(BN_num_bytes(r2) <= 30);
    ENSURE(BN_bn2bin(r1, key));
    assert(BN_num_bytes(r2) <= 2);
    ENSURE(BN_bn2bin(r2, &key[30]));

    // decrypt
    AES_KEY dec_key;
    AES_set_decrypt_key(key, 32*8, &dec_key);

    uint8_t* out = malloc(enc_file.size - 32 + 1);
    AES_ige_encrypt(
        &enc_file.buf[32],
        out,
        enc_file.size - 32,
        &dec_key,
        enc_file.buf,
        AES_DECRYPT
    );
    out[enc_file.size - 32] = '\0';

    printf("%s\n", out);

    // cleanup and exit
    free(d_hex_file.buf);
    free(enc_file.buf);
    return 0;
}
