/**
 * Arquivo: bigint.c
 * Aluno(s):
 *   Nome_do_Aluno1 - Matrícula - Turma
 *   Nome_do_Aluno2 - Matrícula - Turma
 *
 * Implementação de números inteiros de 128 bits com sinal (BigInt),
 * representados em complemento de dois (two's complement) e ordem
 * little-endian (byte menos significativo primeiro).
 *
 * Cada função abaixo implementa uma das operações básicas definidas
 * para o tipo BigInt.
 */

#include "bigint.h"
#include <string.h> /* memset, memcpy */

#define NUM_BYTES (NUM_BITS/8) /* 16 bytes = 128 bits */

/* ===========================================================
 * FUNÇÕES AUXILIARES INTERNAS
 * =========================================================== */

/**
 * Função: big_clear
 *
 * Zera todos os bytes do BigInt.
 *
 * @param x Variável BigInt a ser zerada.
 */
static void big_clear(BigInt x) {
    memset(x, 0, NUM_BYTES);
}

/**
 * Função: big_copy
 *
 * Copia o conteúdo de um BigInt para outro.
 *
 * @param dst Destino da cópia.
 * @param src Origem dos dados.
 */
static void big_copy(BigInt dst, const BigInt src) {
    memcpy(dst, src, NUM_BYTES);
}

/**
 * Função: sign_bit
 *
 * Retorna o bit de sinal de um BigInt (1 se negativo, 0 se positivo).
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
 * Soma dois BigInts de 128 bits (módulo 2¹²⁸).
 * O overflow é descartado automaticamente, mantendo o resultado em 128 bits.
 *
 * @param r Resultado da soma (a + b).
 * @param a Primeiro operando.
 * @param b Segundo operando.
 */
static void add128(BigInt r, const BigInt a, const BigInt b) {
    unsigned int carry = 0;
    for (int i = 0; i < NUM_BYTES; i++) {
        unsigned int sum = (unsigned int)a[i] + (unsigned int)b[i] + carry;
        r[i]   = (unsigned char)(sum & 0xFF);
        carry  = sum >> 8;
    }
}

/**
 * Função: big_comp2
 *
 * Calcula o complemento de dois de um BigInt, ou seja, r = -a.
 * O resultado é obtido invertendo todos os bits e somando 1.
 *
 * @param r Resultado (BigInt negado).
 * @param a Valor de entrada a ser negado.
 */
void big_comp2(BigInt r, BigInt a) {
    unsigned char tmp[NUM_BYTES];
    unsigned int carry = 1;

    /* 1. Inverte todos os bits */
    for (int i = 0; i < NUM_BYTES; i++) {
        tmp[i] = (unsigned char)(~a[i]);
    }

    /* 2. Soma 1 ao resultado invertido */
    for (int i = 0; i < NUM_BYTES; i++) {
        unsigned int sum = (unsigned int)tmp[i] + carry;
        r[i] = (unsigned char)(sum & 0xFF);
        carry = sum >> 8;
    }
}

/* ===========================================================
 * DESLOCAMENTOS DE 1 BIT
 * =========================================================== */

/**
 * Função: shl1
 *
 * Desloca o BigInt para a esquerda em 1 bit (shift lógico).
 *
 * @param out Resultado do deslocamento.
 * @param in  Valor original a ser deslocado.
 */
static void shl1(BigInt out, const BigInt in) {
    unsigned int carry = 0;
    for (int i = 0; i < NUM_BYTES; i++) {
        unsigned int value = ((unsigned int)in[i] << 1) | carry;
        out[i] = (unsigned char)(value & 0xFF);
        carry  = (value >> 8) & 0x01;
    }
}

/**
 * Função: shr1
 *
 * Desloca o BigInt para a direita em 1 bit (shift lógico),
 * preenchendo com zeros no bit mais significativo.
 *
 * @param out Resultado do deslocamento.
 * @param in  Valor original a ser deslocado.
 */
static void shr1(BigInt out, const BigInt in) {
    unsigned int carry = 0;
    for (int i = NUM_BYTES - 1; i >= 0; i--) {
        unsigned int v = ((unsigned int)in[i] >> 1) | (carry << 7);
        out[i]   = (unsigned char)(v & 0xFF);
        carry    = in[i] & 0x1;
    }
}

/**
 * Função: sar1
 *
 * Desloca o BigInt para a direita em 1 bit (shift aritmético),
 * preservando o bit de sinal.
 *
 * @param out Resultado do deslocamento.
 * @param in  Valor original a ser deslocado.
 */
static void sar1(BigInt out, const BigInt in) {
    unsigned int carry = 0;
    int sbit = sign_bit(in);
    for (int i = NUM_BYTES - 1; i >= 0; i--) {
        unsigned int msb_in = (i == NUM_BYTES - 1) ? (sbit << 7) : (carry << 7);
        unsigned int v = ((unsigned int)in[i] >> 1) | msb_in;
        out[i]   = (unsigned char)(v & 0xFF);
        carry    = in[i] & 0x1;
    }
}

/* ===========================================================
 * OPERAÇÕES PRINCIPAIS DA BIBLIOTECA
 * =========================================================== */

/**
 * Função: big_val
 *
 * Inicializa um BigInt com o valor de um número long.
 * Realiza a extensão de sinal para os 128 bits.
 *
 * @param r   Resultado (BigInt inicializado).
 * @param val Valor long de entrada.
 */
void big_val(BigInt r, long val) {
    unsigned long u = (unsigned long)val;
    int n = (sizeof(long) < NUM_BYTES) ? (int)sizeof(long) : NUM_BYTES;

    /* Copia os bytes do número original (little-endian) */
    for (int i = 0; i < n; i++) {
        r[i] = (unsigned char)((u >> (8 * i)) & 0xFF);
    }

    /* Extensão de sinal até 128 bits */
    unsigned char fill = (val < 0) ? 0xFF : 0x00;
    for (int i = n; i < NUM_BYTES; i++) {
        r[i] = fill;
    }
}

/**
 * Função: big_sum
 *
 * Soma dois BigInts (a + b), mantendo o resultado módulo 2¹²⁸.
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

/**
 * Função: big_sar
 *
 * Desloca um BigInt para a direita em n bits (shift aritmético).
 * Preserva o bit de sinal. Se n ≥ 128, preenche com 0x00 ou 0xFF.
 *
 * @param r Resultado do deslocamento.
 * @param a Valor original.
 * @param n Quantidade de bits a deslocar.
 */
void big_sar(BigInt r, BigInt a, int n) {
    if (n <= 0) { big_copy(r, a); return; }
    if (n >= 128) {
        unsigned char fill = (sign_bit(a)) ? 0xFF : 0x00;
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
    unsigned char acc[NUM_BYTES];      /* acumulador do resultado */
    unsigned char mulcand[NUM_BYTES];  /* multiplicando corrente */
    unsigned char muler[NUM_BYTES];    /* multiplicador corrente */
    unsigned char tmp[NUM_BYTES];

    big_clear(acc);
    big_copy(mulcand, a);
    big_copy(muler, b);

    for (int i = 0; i < 128; i++) {
        if (muler[0] & 0x01) {            /* se bit menos significativo de b é 1 */
            add128(tmp, acc, mulcand);    /* soma o multiplicando ao acumulador */
            big_copy(acc, tmp);
        }
        shl1(tmp, mulcand);               /* desloca multiplicando à esquerda */
        big_copy(mulcand, tmp);

        shr1(tmp, muler);                 /* desloca multiplicador à direita */
        big_copy(muler, tmp);
    }

    big_copy(r, acc);
}
