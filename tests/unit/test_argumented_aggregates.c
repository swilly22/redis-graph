#include <stdio.h>
#include "assert.h"
#include "../../src/arithmetic/arithmetic_expression.h"
#include "../../src/arithmetic/agg_funcs.h"

void _test_ar_func(AR_ExpNode *root, SIValue expected) {
    SIValue res = AR_EXP_Evaluate(root);

    if(res.doubleval != expected.doubleval) {
        printf("res.doubleval: %lf expected.doubleval: %lf\n", res.doubleval, expected.doubleval);
    }

    assert(res.doubleval == expected.doubleval);
}

void test_percentile_disc() {
    // Percentiles to check
    AR_ExpNode *zero = AR_EXP_NewConstOperandNode(SI_DoubleVal(0));
    AR_ExpNode *dot_one = AR_EXP_NewConstOperandNode(SI_DoubleVal(0.1));
    AR_ExpNode *one_third = AR_EXP_NewConstOperandNode(SI_DoubleVal(0.33));
    AR_ExpNode *half = AR_EXP_NewConstOperandNode(SI_DoubleVal(0.5));
    AR_ExpNode *one = AR_EXP_NewConstOperandNode(SI_DoubleVal(1));

    AR_ExpNode *test_percentiles[5] = {zero, dot_one, one_third, half, one};
    // percentile_disc should always return a value actually contained in the set
    int expected[5] = {0, 0, 1, 2, 4};
    /* AR_ExpNode *expected[5] = {children[0], children[0], */
        /* children[1], children[2], children[4]}; */

    AR_ExpNode *perc;
    for (int i = 0; i < 5; i ++) {
        perc = AR_EXP_NewOpNode("percentileDisc", 6);
        for (int j = 1; j <= 5; j ++) {
            perc->op.children[j - 1] = AR_EXP_NewConstOperandNode(SI_DoubleVal(j * 2));
        }
        // The last child of this node will be the requested percentile
        perc->op.children[5] = test_percentiles[i];
        AR_EXP_Aggregate(perc);
        // Reduce sorts the list and applies the percentile formula
        AR_EXP_Reduce(perc);
        _test_ar_func(perc, AR_EXP_Evaluate(perc->op.children[expected[i]]));
        AR_EXP_Free(perc);
    }

    printf("test_percentile_disc - PASS!\n");
}

int main(int argc, char **argv) {
    AR_RegisterFuncs();
    Agg_RegisterFuncs();

    test_percentile_disc();

    return 0;
}