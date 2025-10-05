/* bigint.c
 * Aluno(s):
 *   Nome_do_Aluno1 Matricula Turma
 *   Nome_do_Aluno2 Matricula Turma
 *
 * Implementa BigInt (128-bit signed, twos complement, little-endian).
 */

#include "bigint.h"
#include <string.h> /* memset, memcpy */

#define NUM_BYTES (NUM_BITS/8) /* expected 16 for 128-bit */

/* --------------------------- tiny helpers --------------------------- */

static void big_clear(BigInt x) {
    memset(x, 0, NUM_BYTES);
}

static void big_copy(BigInt dst, const BigInt src) {
    memcpy(dst, src, NUM_BYTES);
}

static int sign_bit(const BigInt a) {
    /* 1 if negative, 0 otherwise */
    return (a[NUM_BYTES - 1] & 0x80) != 0;
}

/* Get bit i (0..127), little-endian addressing */
static int get_bit(const BigInt a, int i) {
    int byte_index = i >> 3;       /* i/8 */
    int bit_index  = i & 7;        /* i%8 */
    return (a[byte_index] >> bit_index) & 1;
}

/* r = a + b (mod 2^128) */
static void add128(BigInt r, const BigInt a, const BigInt b) {
    unsigned int carry = 0;
    for (int i = 0; i < NUM_BYTES; i++) {
        unsigned int sum = (unsigned int)a[i] + (unsigned int)b[i] + carry;
        r[i]   = (unsigned char)(sum & 0xFF);
        carry  = sum >> 8;
    }
}

/* r = -a (two's complement: invert bits, then add 1) */
void big_comp2(BigInt r, BigInt a) {
    unsigned char tmp[NUM_BYTES];
    unsigned int carry = 1;

    /* 1. Invert all bits */
    for (int i = 0; i < NUM_BYTES; i++) {
        tmp[i] = (unsigned char)(~a[i]);
    }

    /* 2. Add 1 to the inverted number */
    for (int i = 0; i < NUM_BYTES; i++) {
        unsigned int sum = (unsigned int)tmp[i] + carry;
        r[i] = (unsigned char)(sum & 0xFF);
        carry = sum >> 8;
    }
}


/* ----------------------- 1-bit shift helpers ------------------------ */
/* These keep the code extremely easy to reason about. */

/* out = in << 1 (logical) */
static void shl1(BigInt out, const BigInt in) {
    unsigned int carry = 0; /* carry from lower to higher byte */

    for (int i = 0; i < NUM_BYTES; i++) {
        unsigned int value = ((unsigned int)in[i] << 1) | carry;
        out[i] = (unsigned char)(value & 0xFF);
        carry  = (value >> 8) & 0x01;  /* carry out = bit 8 of shifted value */
    }
}


/* out = in >> 1 (logical, fills with 0 on the left) */
static void shr1(BigInt out, const BigInt in) {
    unsigned int carry = 0; /* from higher byte to lower byte */
    for (int i = NUM_BYTES - 1; i >= 0; i--) {
        unsigned int v = ((unsigned int)in[i] >> 1) | (carry << 7);
        out[i]   = (unsigned char)(v & 0xFF);
        carry    = in[i] & 0x1; /* LSB becomes next carry for the lower byte */
    }
}

/* out = in >> 1 (arithmetic, keeps sign bit) */
static void sar1(BigInt out, const BigInt in) {
    unsigned int carry = 0;
    int sbit = sign_bit(in); /* 1 if negative */
    for (int i = NUM_BYTES - 1; i >= 0; i--) {
        unsigned int msb_in = (i == NUM_BYTES - 1) ? (sbit << 7) : (carry << 7);
        unsigned int v = ((unsigned int)in[i] >> 1) | msb_in;
        out[i]   = (unsigned char)(v & 0xFF);
        carry    = in[i] & 0x1;
    }
}

/* ----------------------------- API --------------------------------- */

void big_val(BigInt r, long val) {
    /* Fill with the bytes of 'val' (little-endian), then sign-extend. */
    unsigned long u = (unsigned long)val;
    int n = (sizeof(long) < NUM_BYTES) ? (int)sizeof(long) : NUM_BYTES;

    for (int i = 0; i < n; i++) {
        r[i] = (unsigned char)((u >> (8 * i)) & 0xFF);
    }
    /* sign-extend to 128 bits */
    {
        unsigned char fill;
        if (val < 0) fill = 0xFF; else fill = 0x00;
        for (int i = n; i < NUM_BYTES; i++) r[i] = fill;
    }
}

void big_sum(BigInt r, BigInt a, BigInt b) {
    add128(r, a, b);
}

void big_sub(BigInt r, BigInt a, BigInt b) {
    unsigned char nb[NUM_BYTES];
    big_comp2(nb, b);     /* nb = -b */
    add128(r, a, nb);     /* r  = a + (-b) */
}

/* r = a << n (logical). If n>=128 -> r = 0. */
void big_shl(BigInt r, BigInt a, int n) {
    if (n <= 0) { big_copy(r, a); return; }
    if (n >= 128) { big_clear(r); return; }

    unsigned char cur[NUM_BYTES], nxt[NUM_BYTES];
    big_copy(cur, a);

    for (int i = 0; i < n; i++) {
        shl1(nxt, cur);
        big_copy(cur, nxt);
    }
    big_copy(r, cur);
}

/* r = a >> n (logical). If n>=128 -> r = 0. */
void big_shr(BigInt r, BigInt a, int n) {
    if (n <= 0) { big_copy(r, a); return; }
    if (n >= 128) { big_clear(r); return; }

    unsigned char cur[NUM_BYTES], nxt[NUM_BYTES];
    big_copy(cur, a);

    for (int i = 0; i < n; i++) {
        shr1(nxt, cur);
        big_copy(cur, nxt);
    }
    big_copy(r, cur);
}

/* r = a >> n (arithmetic). If n>=128 -> r = 0x00.. or 0xFF.. depending on sign. */
void big_sar(BigInt r, BigInt a, int n) {
    if (n <= 0) { big_copy(r, a); return; }
    if (n >= 128) {
        unsigned char fill;
        if (sign_bit(a)) fill = 0xFF; else fill = 0x00;
        for (int i = 0; i < NUM_BYTES; i++) r[i] = fill;
        return;
    }

    unsigned char cur[NUM_BYTES], nxt[NUM_BYTES];
    big_copy(cur, a);

    for (int i = 0; i < n; i++) {
        sar1(nxt, cur);
        big_copy(cur, nxt);
    }
    big_copy(r, cur);
}

/* r = a * b (mod 2^128), classic "shift-and-add":
 * For each bit of b:
 *   if bit == 1: acc += multiplicand
 *   multiplicand <<= 1
 *   b >>= 1 (logical)
 */
void big_mul(BigInt r, BigInt a, BigInt b) {
    unsigned char acc[NUM_BYTES];
    unsigned char mulcand[NUM_BYTES];
    unsigned char muler[NUM_BYTES];
    unsigned char tmp[NUM_BYTES];

    big_clear(acc);
    big_copy(mulcand, a);
    big_copy(muler, b);

    for (int i = 0; i < 128; i++) {
        if (muler[0] & 0x01) {
            add128(tmp, acc, mulcand);
            big_copy(acc, tmp);
        }
        /* mulcand <<= 1; muler >>= 1 (logical) */
        shl1(tmp, mulcand);
        big_copy(mulcand, tmp);

        shr1(tmp, muler);
        big_copy(muler, tmp);
    }

    big_copy(r, acc);
}