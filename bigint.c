/*
 * Carolina de Assis Souza 2320860 - <Colocar Turma>
 *
 * Implementa números inteiros de 128 bits com sinal (complemento de dois),
 * armazenados em ordem little-endian (byte menos significativo primeiro).
 */

#include "bigint.h"
#include <string.h> 

#define NUM_BYTES (NUM_BITS / 8)  /* 16 bytes = 128 bits */

/* ============================================================
 * Funções auxiliares
 * ============================================================ */

/**
 * Função: sign_bit
 *
 * Retorna 1 se o número é negativo (bit de sinal = 1), ou 0 caso contrário.
 *
 * @param a Valor BigInt a ser analisado.
 * @return 1 se negativo, 0 se positivo.
 */
static int sign_bit(const BigInt a) {
    return (a[NUM_BYTES - 1] & 0x80) != 0;
}

/**
 * Função: add128
 *
 * Soma dois BigInts (a + b) módulo 2¹²⁸.
 * O overflow é descartado automaticamente, mantendo o resultado em 128 bits.
 *
 * @param r Resultado da soma.
 * @param a Primeiro operando.
 * @param b Segundo operando.
 */
static void add128(BigInt r, const BigInt a, const BigInt b) {
    unsigned int carry = 0;
    for (int i = 0; i < NUM_BYTES; i++) {
        unsigned int sum = (unsigned int)a[i] + (unsigned int)b[i] + carry;
        r[i] = (unsigned char)(sum & 0xFF);
        carry = sum >> 8;
    }
}

/**
 * Função: big_comp2
 *
 * Calcula o complemento de dois de um BigInt (r = -a),
 * invertendo os bits e somando 1.
 *
 * @param r Resultado (BigInt negado).
 * @param a Valor original.
 */
void big_comp2(BigInt r, BigInt a) {
    unsigned char tmp[NUM_BYTES];
    unsigned int carry = 1;

    for (int i = 0; i < NUM_BYTES; i++)
        tmp[i] = (unsigned char)(~a[i]);

    for (int i = 0; i < NUM_BYTES; i++) {
        unsigned int sum = (unsigned int)tmp[i] + carry;
        r[i] = (unsigned char)(sum & 0xFF);
        carry = sum >> 8;
    }
}

/* ============================================================
 * Deslocamentos de 1 bit
 * ============================================================ */

/* desloca à esquerda (shift lógico de 1 bit) */
static void shl1(BigInt out, const BigInt in) {
    unsigned int carry = 0;
    for (int i = 0; i < NUM_BYTES; i++) {
        unsigned int v = ((unsigned int)in[i] << 1) | carry;
        out[i] = (unsigned char)(v & 0xFF);
        carry = (v >> 8) & 1;
    }
}

/* desloca à direita (shift lógico de 1 bit) */
static void shr1(BigInt out, const BigInt in) {
    unsigned int carry = 0;
    for (int i = NUM_BYTES - 1; i >= 0; i--) {
        unsigned int v = ((unsigned int)in[i] >> 1) | (carry << 7);
        out[i] = (unsigned char)(v & 0xFF);
        carry = in[i] & 1;
    }
}

/* desloca à direita (shift aritmético de 1 bit, preservando sinal) */
static void sar1(BigInt out, const BigInt in) {
    unsigned int carry = 0;
    int sbit = sign_bit(in);
    for (int i = NUM_BYTES - 1; i >= 0; i--) {
        unsigned int msb = (i == NUM_BYTES - 1) ? (sbit << 7) : (carry << 7);
        unsigned int v = ((unsigned int)in[i] >> 1) | msb;
        out[i] = (unsigned char)(v & 0xFF);
        carry = in[i] & 1;
    }
}

/* ============================================================
 * Operações principais
 * ============================================================ */

/**
 * Função: big_val
 *
 * Inicializa um BigInt com o valor de um número long.
 * Os bytes são copiados em ordem little-endian e o sinal é estendido
 * até completar os 128 bits.
 *
 * @param r   Resultado (BigInt inicializado).
 * @param val Valor long a ser convertido.
 */
void big_val(BigInt r, long val) {
    unsigned long u = (unsigned long)val;
    int n = (sizeof(long) < NUM_BYTES) ? (int)sizeof(long) : NUM_BYTES;

    for (int i = 0; i < n; i++)
        r[i] = (unsigned char)((u >> (8 * i)) & 0xFF);

    unsigned char fill = (val < 0) ? 0xFF : 0x00;
    for (int i = n; i < NUM_BYTES; i++)
        r[i] = fill;
}

/**
 * Função: big_sum
 *
 * Soma dois BigInts (a + b) módulo 2¹²⁸.
 *
 * @param r Resultado da soma.
 * @param a Primeiro operando.
 * @param b Segundo operando.
 */
void big_sum(BigInt r, BigInt a, BigInt b) {
    add128(r, a, b);
}

/**
 * Função: big_sub
 *
 * Subtrai dois BigInts (a - b), implementada como a + (-b).
 *
 * @param r Resultado da subtração.
 * @param a Minuendo.
 * @param b Subtraendo.
 */
void big_sub(BigInt r, BigInt a, BigInt b) {
    unsigned char nb[NUM_BYTES];
    big_comp2(nb, b);
    add128(r, a, nb);
}

/**
 * Função: big_shl
 *
 * Desloca um BigInt para a esquerda em n bits (shift lógico).
 * Se n ≥ 128, o resultado é zero.
 *
 * @param r Resultado do deslocamento.
 * @param a Valor original.
 * @param n Quantidade de bits a deslocar.
 */
void big_shl(BigInt r, BigInt a, int n) {
    if (n <= 0) { memcpy(r, a, NUM_BYTES); return; }
    if (n >= 128) { memset(r, 0, NUM_BYTES); return; }

    unsigned char cur[NUM_BYTES], nxt[NUM_BYTES];
    memcpy(cur, a, NUM_BYTES);

    for (int i = 0; i < n; i++) {
        shl1(nxt, cur);
        memcpy(cur, nxt, NUM_BYTES);
    }
    memcpy(r, cur, NUM_BYTES);
}

/**
 * Função: big_shr
 *
 * Desloca um BigInt para a direita em n bits (shift lógico).
 * Se n ≥ 128, o resultado é zero.
 *
 * @param r Resultado do deslocamento.
 * @param a Valor original.
 * @param n Quantidade de bits a deslocar.
 */
void big_shr(BigInt r, BigInt a, int n) {
    if (n <= 0) { memcpy(r, a, NUM_BYTES); return; }
    if (n >= 128) { memset(r, 0, NUM_BYTES); return; }

    unsigned char cur[NUM_BYTES], nxt[NUM_BYTES];
    memcpy(cur, a, NUM_BYTES);

    for (int i = 0; i < n; i++) {
        shr1(nxt, cur);
        memcpy(cur, nxt, NUM_BYTES);
    }
    memcpy(r, cur, NUM_BYTES);
}

/**
 * Função: big_sar
 *
 * Desloca um BigInt para a direita em n bits (shift aritmético).
 * Preserva o bit de sinal. Se n ≥ 128, o resultado é preenchido
 * com 0x00 (para números positivos) ou 0xFF (para negativos).
 *
 * @param r Resultado do deslocamento.
 * @param a Valor original.
 * @param n Quantidade de bits a deslocar.
 */
void big_sar(BigInt r, BigInt a, int n) {
    if (n <= 0) { memcpy(r, a, NUM_BYTES); return; }
    if (n >= 128) {
        unsigned char fill = sign_bit(a) ? 0xFF : 0x00;
        for (int i = 0; i < NUM_BYTES; i++) r[i] = fill;
        return;
    }

    unsigned char cur[NUM_BYTES], nxt[NUM_BYTES];
    memcpy(cur, a, NUM_BYTES);

    for (int i = 0; i < n; i++) {
        sar1(nxt, cur);
        memcpy(cur, nxt, NUM_BYTES);
    }
    memcpy(r, cur, NUM_BYTES);
}

/**
 * Função: big_mul
 *
 * Multiplica dois BigInts de 128 bits (a * b),
 * utilizando o método clássico de soma e deslocamento ("shift and add").
 * O resultado é mantido módulo 2¹²⁸.
 *
 * @param r Resultado da multiplicação.
 * @param a Multiplicando.
 * @param b Multiplicador.
 */
void big_mul(BigInt r, BigInt a, BigInt b) {
    unsigned char acc[NUM_BYTES] = {0};
    unsigned char mulcand[NUM_BYTES], muler[NUM_BYTES], tmp[NUM_BYTES];

    memcpy(mulcand, a, NUM_BYTES);
    memcpy(muler, b, NUM_BYTES);

    for (int i = 0; i < 128; i++) {
        if (muler[0] & 1) {
            add128(tmp, acc, mulcand);
            memcpy(acc, tmp, NUM_BYTES);
        }
        shl1(tmp, mulcand);
        memcpy(mulcand, tmp, NUM_BYTES);

        shr1(tmp, muler);
        memcpy(muler, tmp, NUM_BYTES);
    }

    memcpy(r, acc, NUM_BYTES);
}
