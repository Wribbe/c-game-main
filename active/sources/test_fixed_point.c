#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

int main(void)
{
    int16_t ia = 10;
    float fa = 10.0f;

    int divisor = 3;
    int shift = 4;

    int16_t shifted_i = (ia << shift)/divisor;
    int16_t shifted_sum = 0;
    for (int i=0; i<5; i++) {
        shifted_sum += shifted_i;
    }
    int16_t idiv = (shifted_sum + (1 << (shift - 1))) >> shift;
    //int16_t idiv = shifted_sum >> shift;
    float fdiv = fa/divisor;

    printf("fdiv: %f\n", fdiv*5);
    printf("idiv: %"PRIi16"\n", idiv);

    // Try convert from negative to positive.
    int16_t sia = -4;
    uint16_t sia_sum = 0;

    uint16_t adder = 0x11/2;

    uint16_t sia_pos = sia + adder;
    size_t num_add = 4;
    for (size_t i=0; i<num_add; i++) {
        sia_sum += sia_pos;
        printf("sia_pos: %"PRIu16"\n", sia_pos);
    }

    int16_t sia_reverted = (int16_t)sia_sum - (adder * num_add);
    printf("sia: %"PRIi16" sia_reverted_sum: %"PRIi16" sia_reverted: %"PRIi16"\n", sia, sia_reverted, (int16_t)(sia_reverted/num_add));

    // Try some shifting and neg -> pos -> neg.
    uint16_t pos_shift = (0xffff >> 2)+1;
    int16_t original = -33;
    size_t shifts = 8;
    uint32_t pos_original = (original + pos_shift) << shifts;
    printf("pos_original: %"PRIu16"\n", pos_original);
    uint64_t pos_sum = 0;
    float test_sum = 0;
    uint16_t num_elements = 93;
    for(size_t i=0; i<num_elements; i++) {
        pos_sum += pos_original/num_elements;
        test_sum += (float)original/num_elements;
    }
    int16_t neg_sum = 0;
    // Round up on last decimal if > 0.5.
    pos_sum += 1 << (shifts - 1);
    printf("pos_sum: %"PRIu64"\n", pos_sum);
    printf("positive shift: %"PRIu16"\n", pos_shift);
    neg_sum = (pos_sum >> shifts) - pos_shift;

    printf("test_sum: %f\n", test_sum);
    printf("pos_sum: %"PRIu64", original: %"PRIi16", neg_sum: %"PRIi16"\n", pos_sum, original, neg_sum);

    // Convert from int16_t to float.
    int16_t conv = -0xffff+0xffff/2;
    float fconv = (float)conv/(0xffff);
    printf("conv: %"PRIi16" float: %f\n", conv, fconv);
}
