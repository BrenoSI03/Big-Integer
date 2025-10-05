/* testebigint.c
 * Aluno(s):
 *   Nome_do_Aluno1 Matricula Turma
 *   Nome_do_Aluno2 Matricula Turma
*/

#include "bigint.h"
#include <stdio.h>
#include <string.h>

static void dump_hex(const char *tag, const BigInt x) {
    printf("%s = 0x", tag);
    for (int i = NUM_BITS/8 - 1; i >= 0; --i) printf("%02X", x[i]);
    putchar('\n');
}

static int eq(const BigInt a, const BigInt b) { return memcmp(a,b,NUM_BITS/8)==0; }

#define ASSERT_TRUE(msg,cond) do{ if(!(cond)){printf("FAIL: %s\n",msg); return 1;} else printf("OK  : %s\n",msg);}while(0)

int main(void) {
    BigInt a,b,r,exp;

    /* big_val: 1 e -2 do enunciado */
    big_val(a, 1L);
    /* exp = {01,00,...,00} */
    memset(exp, 0, sizeof exp); exp[0]=0x01;
    ASSERT_TRUE("big_val(1)", eq(a, exp));

    big_val(a, -2L);
    /* exp = {FE,FF,...,FF} */
    for (int i=0;i<16;i++) exp[i]=0xFF; exp[0]=0xFE;
    ASSERT_TRUE("big_val(-2)", eq(a, exp));

    /* comp2: -1 = comp2(1) */
    big_val(a, 1L);
    big_comp2(r, a);
    for (int i=0;i<16;i++) exp[i]=0xFF;
    ASSERT_TRUE("comp2(1) = -1", eq(r, exp));

    /* sum: 1+2=3 */
    big_val(a, 1L);
    big_val(b, 2L);
    big_sum(r, a, b);
    big_val(exp, 3L);
    ASSERT_TRUE("1+2=3", eq(r, exp));

    /* sub: 3-5=-2 */
    big_val(a, 3L); big_val(b, 5L);
    big_sub(r, a, b);
    big_val(exp, -2L);
    ASSERT_TRUE("3-5=-2", eq(r, exp));

    /* shl: (1<<8) */
    big_val(a, 1L);
    big_shl(r, a, 8);
    memset(exp,0,16); exp[1]=0x01;
    ASSERT_TRUE("shl(1,8)", eq(r, exp));

    /* shr lógico: 0x8000.. >> 1 = 0x4000.. */
    memset(a,0,16); a[15]=0x80;
    big_shr(r, a, 1);
    memset(exp,0,16); exp[15]=0x40;
    ASSERT_TRUE("shr lógico", eq(r, exp));

    /* sar aritmético: (-2)>>1 = -1 */
    big_val(a, -2L);
    big_sar(r, a, 1);
    for (int i=0;i<16;i++) exp[i]=0xFF;
    ASSERT_TRUE("sar(-2,1)=-1", eq(r, exp));

    /* mul: 5 * (-3) = -15 */
    big_val(a, 5L);
    big_val(b, -3L);
    big_mul(r, a, b);
    big_val(exp, -15L);
    ASSERT_TRUE("5*(-3)=-15", eq(r, exp));

    /* mul: (1<<64) * 2 = (1<<65) */
    memset(a,0,16); a[8]=0x01; /* 2^64 */
    big_val(b, 2L);
    big_mul(r, a, b);
    memset(exp, 0, 16); 
    exp[8] = 0x02;
    ASSERT_TRUE("(1<<64)*2=(1<<65)", eq(r, exp));

    puts("Todos os testes básicos passaram.");
    return 0;
}
