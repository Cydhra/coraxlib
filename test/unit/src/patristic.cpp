#include <gtest/gtest.h>

#include "corax/corax.h"
#include "corax/tree/utree.h"
#include <string>

TEST(PATRISTIC, correctness) {
    corax_utree_t* tree = corax_utree_parse_newick_string_unroot("((a:0.1,b:0.2)ab:0.9,(c:0.1,d:0.11)cd:0.5, e:1.9);");
    std::vector matrix(25, 0.0);

    ASSERT_TRUE(corax_utree_binary_patristic_distances(tree->vroot, 5, matrix.data()));

    // tips are left to right from the parser
    ASSERT_TRUE(matrix[0 * 5 + 1] - 0.3     < 1E-12);
    ASSERT_TRUE(matrix[0 * 5 + 2] - 1.6     < 1E-12);
    ASSERT_TRUE(matrix[0 * 5 + 3] - 1.61    < 1E-12);
    ASSERT_TRUE(matrix[0 * 5 + 4] - 2.9     < 1E-12);

    ASSERT_TRUE(matrix[1 * 5 + 0] - 0.3     < 1E-12);
    ASSERT_TRUE(matrix[1 * 5 + 2] - 1.7     < 1E-12);
    ASSERT_TRUE(matrix[1 * 5 + 3] - 1.71    < 1E-12);
    ASSERT_TRUE(matrix[1 * 5 + 4] - 3.0     < 1E-12);

    ASSERT_TRUE(matrix[2 * 5 + 0] - 1.6     < 1E-12);
    ASSERT_TRUE(matrix[2 * 5 + 1] - 1.7     < 1E-12);
    ASSERT_TRUE(matrix[2 * 5 + 3] - 0.21    < 1E-12);
    ASSERT_TRUE(matrix[2 * 5 + 4] - 2.5     < 1E-12);

    ASSERT_TRUE(matrix[3 * 5 + 0] - 1.61    < 1E-12);
    ASSERT_TRUE(matrix[3 * 5 + 1] - 1.71    < 1E-12);
    ASSERT_TRUE(matrix[3 * 5 + 2] - 0.21    < 1E-12);
    ASSERT_TRUE(matrix[3 * 5 + 4] - 2.51    < 1E-12);

    ASSERT_TRUE(matrix[4 * 5 + 0] - 2.9     < 1E-12);
    ASSERT_TRUE(matrix[4 * 5 + 1] - 3.0     < 1E-12);
    ASSERT_TRUE(matrix[4 * 5 + 2] - 2.5     < 1E-12);
    ASSERT_TRUE(matrix[4 * 5 + 3] - 2.51    < 1E-12);
}

TEST(PATRISTIC, stability) {
    corax_utree_t* tree = corax_utree_parse_newick_string_unroot("((a:0.1,b:0.2)ab:0.9,(c:0.1,d:0.11)cd:0.5, e:1.9);");
    std::vector matrix(25, 0.0);
    std::vector other_root(25, 0.0);

    ASSERT_TRUE(corax_utree_binary_patristic_distances(tree->vroot, 5, matrix.data()));
    ASSERT_TRUE(corax_utree_binary_patristic_distances(tree->vroot->next->back, 5, other_root.data()));

    for (unsigned int i = 0; i < 5; ++i) {
        for (unsigned int j = 0; i < 5; ++i) {
            if (i != j) {
                ASSERT_TRUE(matrix[i * 5 + j] - other_root[i * 5 + j] < 1E-12);
            }
        }
    }
}
