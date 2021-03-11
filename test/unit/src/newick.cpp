#include "corax/corax.h"
#include <asm-generic/errno.h>
#include <gtest/gtest.h>
#include <stdexcept>

/*
 * This needs to be a macro because we need to inline the code to get google
 * test to print the trace corectly.
 */
#define check_node_attributes(node, expected_label, expected_length)           \
  {                                                                            \
    EXPECT_STREQ(node->label, expected_label);                                 \
    EXPECT_EQ(node->length, expected_length);                                  \
    EXPECT_EQ(node->length, node->back->length);                               \
  }

#define check_tree_attributes(                                                 \
    test_tree, expected_inner_count, expected_tip_count)                       \
  {                                                                            \
    EXPECT_EQ(test_tree->tip_count, expected_tip_count);                       \
    EXPECT_EQ(test_tree->inner_count, expected_inner_count);                   \
  }

TEST(NewickParser, simple0) {
  auto t = pll_utree_parse_newick_string_unroot("((a:1.0,b:1.0):1.0, c:1.0);");

  check_tree_attributes(t, 1, 3);

  auto current = t->vroot;

  check_node_attributes(current->back, "c", 2.0);

  current = current->next;

  check_node_attributes(current->back, "a", 1.0);

  current = current->next;

  check_node_attributes(current->back, "b", 1.0);

  pll_utree_destroy(t, nullptr);
}

TEST(NewickParser, simple0rooted) {
  auto t = pll_utree_parse_newick_string_rooted("((a:1.0,b:1.0):1.0, c:1.0);");

  EXPECT_EQ(t->tip_count, 3);
  EXPECT_EQ(t->inner_count, 2);

  auto current = t->vroot;

  check_node_attributes(current->back, "c", 1.0);

  current = current->next->back;

  check_node_attributes(current->back, nullptr, 1.0);

  current = current->next;

  check_node_attributes(current->back, "a", 1.0);

  current = current->next;

  check_node_attributes(current->back, "b", 1.0);

  pll_utree_destroy(t, nullptr);
}

TEST(NewickParser, simple1) {
  auto t = pll_utree_parse_newick_string_unroot("((a,b)ab,(c,d)cd)root;");

  EXPECT_EQ(t->tip_count, 4);
  EXPECT_EQ(t->inner_count, 2);

  auto current = t->vroot;

  check_node_attributes(current, "cd", 0.0);

  current = current->next;

  check_node_attributes(current->back, "c", 0.0);

  current = current->next;

  check_node_attributes(current->back, "d", 0.0);

  current = t->vroot->back;

  check_node_attributes(current, "ab", 0.0);

  current = current->next;

  check_node_attributes(current->back, "a", 0.0);

  current = current->next;

  check_node_attributes(current->back, "b", 0.0);

  pll_utree_destroy(t, nullptr);
}

TEST(NewickParser, simple1rooted) {
  auto t = pll_utree_parse_newick_string_rooted("((a,b)ab,(c,d)cd)root;");

  check_tree_attributes(t, 3, 4);

  auto current = t->vroot;

  check_node_attributes(current, "root", 0.0);

  current = current->next;

  check_node_attributes(current->back, "ab", 0.0);

  current = current->next;

  check_node_attributes(current->back, "cd", 0.0);

  current = t->vroot->back->next;

  check_node_attributes(current->back, "c", 0.0);

  current = current->next;

  check_node_attributes(current->back, "d", 0.0);

  current = t->vroot->next->back->next;

  check_node_attributes(current->back, "a", 0.0);

  current = current->next;

  check_node_attributes(current->back, "b", 0.0);

  pll_utree_destroy(t, nullptr);
}

TEST(NewickParser, simple2) {
  auto t = pll_utree_parse_newick_string_unroot("((a,b)13,(c,d)4cd)root;");

  check_tree_attributes(t, 2, 4);

  auto current = t->vroot;

  check_node_attributes(current, "4cd", 0.0);

  current = current->next;

  check_node_attributes(current->back, "c", 0.0);

  current = current->next;

  check_node_attributes(current->back, "d", 0.0);

  current = t->vroot->back;

  check_node_attributes(current, "13", 0.0);

  current = current->next;

  check_node_attributes(current->back, "a", 0.0);

  current = current->next;

  check_node_attributes(current->back, "b", 0.0);

  pll_utree_destroy(t, nullptr);
}

TEST(NewickParser, simple2rooted) {
  auto t = pll_utree_parse_newick_string_rooted("((a,b)13,(c,d)4cd)root;");

  check_tree_attributes(t, 3, 4);

  auto current = t->vroot;

  check_node_attributes(current, "root", 0.0);

  current = current->next;

  check_node_attributes(current->back, "13", 0.0);

  current = current->next;

  check_node_attributes(current->back, "4cd", 0.0);

  current = t->vroot->back->next;

  check_node_attributes(current->back, "c", 0.0);

  current = current->next;

  check_node_attributes(current->back, "d", 0.0);

  current = t->vroot->next->back->next;

  check_node_attributes(current->back, "a", 0.0);

  current = current->next;

  check_node_attributes(current->back, "b", 0.0);

  pll_utree_destroy(t, nullptr);
}

TEST(NewickParser, simple3) {
  auto t = pll_utree_parse_newick_string_unroot(
      "((a:30.5,b:0.03):48.0,(c:0,d:3)cd)root;");

  check_tree_attributes(t, 2, 4);

  auto current = t->vroot;

  check_node_attributes(current, "cd", 48.0);

  current = current->next;

  check_node_attributes(current->back, "c", 0.0);

  current = current->next;

  check_node_attributes(current->back, "d", 3.0);

  current = t->vroot->back;

  check_node_attributes(current, nullptr, 48.0);

  current = current->next;

  check_node_attributes(current->back, "a", 30.5);

  current = current->next;

  check_node_attributes(current->back, "b", 0.03);

  pll_utree_destroy(t, nullptr);
}

TEST(NewickParser, simple3rooted) {
  auto t = pll_utree_parse_newick_string_rooted(
      "((a:30.5,b:0.03):48.0,(c:0,d:3)cd)root;");

  check_tree_attributes(t, 3, 4);

  auto current = t->vroot;

  check_node_attributes(current, "root", 0.0);

  current = current->next;

  check_node_attributes(current->back, nullptr, 48.0);

  current = current->next;

  check_node_attributes(current->back, "cd", 0.0);

  current = t->vroot->back->next;

  check_node_attributes(current->back, "c", 0.0);

  current = current->next;

  check_node_attributes(current->back, "d", 3.0);

  current = t->vroot->next->back->next;

  check_node_attributes(current->back, "a", 30.5);

  current = current->next;

  check_node_attributes(current->back, "b", 0.03);

  pll_utree_destroy(t, nullptr);
}

TEST(NewickParser, simple4) {
  auto t = pll_utree_parse_newick_string_unroot(
      "((a:1e-10,b:0.03)ab,(c:0,d:3E-5)cd)root;");

  check_tree_attributes(t, 2, 4);

  auto current = t->vroot;

  check_node_attributes(current, "cd", 0.0);

  current = current->next;

  check_node_attributes(current->back, "c", 0.0);

  current = current->next;

  check_node_attributes(current->back, "d", 3e-5);

  current = t->vroot->back;

  check_node_attributes(current, "ab", 0.0);

  current = current->next;

  check_node_attributes(current->back, "a", 1e-10);

  current = current->next;

  check_node_attributes(current->back, "b", 0.03);

  pll_utree_destroy(t, nullptr);
}

TEST(NewickParser, simple4rooted) {
  auto t = pll_utree_parse_newick_string_rooted(
      "((a:1e-10,b:0.03)ab,(c:0,d:3E-5)cd)root;");

  check_tree_attributes(t, 3, 4);

  auto current = t->vroot;

  check_node_attributes(current, "root", 0.0);

  current = current->next;

  check_node_attributes(current->back, "ab", 0.0);

  current = current->next;

  check_node_attributes(current->back, "cd", 0.0);

  current = t->vroot->back->next;

  check_node_attributes(current->back, "c", 0.0);

  current = current->next;

  check_node_attributes(current->back, "d", 3e-5);

  current = t->vroot->next->back->next;

  check_node_attributes(current->back, "a", 1e-10);

  current = current->next;

  check_node_attributes(current->back, "b", 0.03);

  pll_utree_destroy(t, nullptr);
}

TEST(NewickParser, simple5) {
  auto t = pll_utree_parse_newick_string_unroot(
      "( (a : 30.5 , b : 0.03 ) ab , (c :0,d : 3 ) cd )root;");

  check_tree_attributes(t, 2, 4);

  auto current = t->vroot;

  check_node_attributes(current, "cd", 0.0);

  current = current->next;

  check_node_attributes(current->back, "c", 0.0);

  current = current->next;

  check_node_attributes(current->back, "d", 3.0);

  current = t->vroot->back;

  check_node_attributes(current, "ab", 0.0);

  current = current->next;

  check_node_attributes(current->back, "a", 30.5);

  current = current->next;

  check_node_attributes(current->back, "b", 0.03);

  pll_utree_destroy(t, nullptr);
}

TEST(NewickParser, simple5rooted) {
  auto t = pll_utree_parse_newick_string_rooted(
      "( (a : 30.5 , b : 0.03 ) ab , (c :0,d : 3 ) cd )root;");

  check_tree_attributes(t, 3, 4);

  auto current = t->vroot;

  check_node_attributes(current, "root", 0.0);

  current = current->next;

  check_node_attributes(current->back, "ab", 0.0);

  current = current->next;

  check_node_attributes(current->back, "cd", 0.0);

  current = t->vroot->back->next;

  check_node_attributes(current->back, "c", 0.0);

  current = current->next;

  check_node_attributes(current->back, "d", 3.0);

  current = t->vroot->next->back->next;

  check_node_attributes(current->back, "a", 30.5);

  current = current->next;

  check_node_attributes(current->back, "b", 0.03);

  pll_utree_destroy(t, nullptr);
}

TEST(NewickParser, simple6) {
  auto t = pll_utree_parse_newick_string_unroot(
      "(\t(a\t:\t30.5\t,\tb\t:\t0.03\t)\tab\t,\t(c\t:0,d\t:\t3\t)\tcd\t)"
      "root\t;");

  check_tree_attributes(t, 2, 4);

  auto current = t->vroot;

  check_node_attributes(current, "cd", 0.0);

  current = current->next;

  check_node_attributes(current->back, "c", 0.0);

  current = current->next;

  check_node_attributes(current->back, "d", 3.0);

  current = t->vroot->back;

  check_node_attributes(current, "ab", 0.0);

  current = current->next;

  check_node_attributes(current->back, "a", 30.5);

  current = current->next;

  check_node_attributes(current->back, "b", 0.03);

  pll_utree_destroy(t, nullptr);
}

TEST(NewickParser, simple6rooted) {
  auto t = pll_utree_parse_newick_string_rooted(
      "(\t(a\t:\t30.5\t,\tb\t:\t0.03\t)\tab\t,\t(c\t:0,d\t:\t3\t)\tcd\t)"
      "root\t;");

  check_tree_attributes(t, 3, 4);

  auto current = t->vroot;

  check_node_attributes(current, "root", 0.0);

  current = current->next;

  check_node_attributes(current->back, "ab", 0.0);

  current = current->next;

  check_node_attributes(current->back, "cd", 0.0);

  current = t->vroot->back->next;

  check_node_attributes(current->back, "c", 0.0);

  current = current->next;

  check_node_attributes(current->back, "d", 3.0);

  current = t->vroot->next->back->next;

  check_node_attributes(current->back, "a", 30.5);

  current = current->next;

  check_node_attributes(current->back, "b", 0.03);

  pll_utree_destroy(t, nullptr);
}

TEST(NewickParser, simple7) {
  auto t = pll_utree_parse_newick_string_unroot("((a,b)ab\n,(c,d\n)cd)root;");

  check_tree_attributes(t, 2, 4);

  auto current = t->vroot;

  check_node_attributes(current, "cd", 0.0);

  current = current->next;

  check_node_attributes(current->back, "c", 0.0);

  current = current->next;

  check_node_attributes(current->back, "d", 0.0);

  current = t->vroot->back;

  check_node_attributes(current, "ab", 0.0);

  current = current->next;

  check_node_attributes(current->back, "a", 0.0);

  current = current->next;

  check_node_attributes(current->back, "b", 0.0);

  pll_utree_destroy(t, nullptr);
}

TEST(NewickParser, simple7rooted) {
  auto t = pll_utree_parse_newick_string_rooted("((a,b)ab\n,(c,d\n)cd)root;");

  check_tree_attributes(t, 3, 4);

  auto current = t->vroot;

  check_node_attributes(current, "root", 0.0);

  current = current->next;

  check_node_attributes(current->back, "ab", 0.0);

  current = current->next;

  check_node_attributes(current->back, "cd", 0.0);

  current = t->vroot->back->next;

  check_node_attributes(current->back, "c", 0.0);

  current = current->next;

  check_node_attributes(current->back, "d", 0.0);

  current = t->vroot->next->back->next;

  check_node_attributes(current->back, "a", 0.0);

  current = current->next;

  check_node_attributes(current->back, "b", 0.0);

  pll_utree_destroy(t, nullptr);
}

TEST(NewickParser, simple8) {
  auto t =
      pll_utree_parse_newick_string_unroot("((a,b)ab\r\n,(c,d\r\n)cd)root;");

  check_tree_attributes(t, 2, 4);

  auto current = t->vroot;

  check_node_attributes(current, "cd", 0.0);

  current = current->next;

  check_node_attributes(current->back, "c", 0.0);

  current = current->next;

  check_node_attributes(current->back, "d", 0.0);

  current = t->vroot->back;

  check_node_attributes(current, "ab", 0.0);

  current = current->next;

  check_node_attributes(current->back, "a", 0.0);

  current = current->next;

  check_node_attributes(current->back, "b", 0.0);

  pll_utree_destroy(t, nullptr);
}

TEST(NewickParser, simple8rooted) {
  auto t =
      pll_utree_parse_newick_string_rooted("((a,b)ab\r\n,(c,d\r\n)cd)root;");

  check_tree_attributes(t, 3, 4);

  auto current = t->vroot;

  check_node_attributes(current, "root", 0.0);

  current = current->next;

  check_node_attributes(current->back, "ab", 0.0);

  current = current->next;

  check_node_attributes(current->back, "cd", 0.0);

  current = t->vroot->back->next;

  check_node_attributes(current->back, "c", 0.0);

  current = current->next;

  check_node_attributes(current->back, "d", 0.0);

  current = t->vroot->next->back->next;

  check_node_attributes(current->back, "a", 0.0);

  current = current->next;

  check_node_attributes(current->back, "b", 0.0);

  pll_utree_destroy(t, nullptr);
}

TEST(NewickParser, simple9) {
  auto t = pll_utree_parse_newick_string_unroot(
      "((!a+7=5,b^o&)ab,($$£*c,d/\\?!_-|)cd)ro#~ot;");

  check_tree_attributes(t, 2, 4);

  auto current = t->vroot;

  check_node_attributes(current, "cd", 0.0);

  current = current->next;

  check_node_attributes(current->back, "$$£*c", 0.0);

  current = current->next;

  /* Note that the \\ is a backslash only. */
  check_node_attributes(current->back, "d/\\?!_-|", 0.0);

  current = t->vroot->back;

  check_node_attributes(current, "ab", 0.0);

  current = current->next;

  check_node_attributes(current->back, "!a+7=5", 0.0);

  current = current->next;

  check_node_attributes(current->back, "b^o&", 0.0);

  pll_utree_destroy(t, nullptr);
}

TEST(NewickParser, simple9rooted) {
  auto t = pll_utree_parse_newick_string_rooted(
      "((!a+7=5,b^o&)ab,($$£*c,d/\\?!_-|)cd)ro#~ot;");

  check_tree_attributes(t, 3, 4);

  auto current = t->vroot;

  check_node_attributes(current, "ro#~ot", 0.0);

  current = current->next;

  check_node_attributes(current->back, "ab", 0.0);

  current = current->next;

  check_node_attributes(current->back, "cd", 0.0);

  current = current->next;

  current = t->vroot->back->next;

  check_node_attributes(current->back, "$$£*c", 0.0);

  current = current->next;

  /* Note that the \\ is a backslash only. */
  check_node_attributes(current->back, "d/\\?!_-|", 0.0);

  current = t->vroot->next->back->next;

  check_node_attributes(current->back, "!a+7=5", 0.0);

  current = current->next;

  check_node_attributes(current->back, "b^o&", 0.0);

  pll_utree_destroy(t, nullptr);
}

TEST(NewickParser, simple10) {
  auto t =
      pll_utree_parse_newick_string_unroot("((a[comment],b),(c,(d, e):0.5));");

  check_tree_attributes(t, 3, 5);

  auto current = t->vroot;

  check_node_attributes(current, nullptr, 0.0);

  current = current->next;

  check_node_attributes(current->back, "c", 0.0);

  current = current->next;

  check_node_attributes(current->back, nullptr, 0.5);

  current = current->back->next;

  check_node_attributes(current->back, "d", 0.0);

  current = current->next;

  check_node_attributes(current->back, "e", 0.0);

  current = t->vroot->back->next;

  check_node_attributes(current->back, "a", 0.0);

  current = current->next;

  check_node_attributes(current->back, "b", 0.0);

  pll_utree_destroy(t, nullptr);
}

TEST(NewickParser, simple11) {
  auto t =
      pll_utree_parse_newick_string_unroot("((a,b),(c,(d, e)hello world));");

  check_tree_attributes(t, 3, 5);

  auto current = t->vroot;

  check_node_attributes(current, nullptr, 0.0);

  current = current->next;

  check_node_attributes(current->back, "c", 0.0);

  current = current->next;

  check_node_attributes(current->back, "hello world", 0.0);

  current = current->back->next;

  check_node_attributes(current->back, "d", 0.0);

  current = current->next;

  check_node_attributes(current->back, "e", 0.0);

  current = t->vroot->back->next;

  check_node_attributes(current->back, "a", 0.0);

  current = current->next;

  check_node_attributes(current->back, "b", 0.0);

  pll_utree_destroy(t, nullptr);
}

TEST(NewickParser, unrooted0) {
  auto t = pll_utree_parse_newick_string("(a:1.0,b:1.0,c:1.0);");

  check_tree_attributes(t, 1, 3);

  auto current = t->vroot;

  check_node_attributes(current->back, "c", 1.0);

  current = current->next;

  check_node_attributes(current->back, "a", 1.0);

  current = current->next;

  check_node_attributes(current->back, "b", 1.0);

  pll_utree_destroy(t, nullptr);
}

TEST(NewickParser, unrooted1) {
  auto t =
      pll_utree_parse_newick_string("((a:1.0,b:1.0)ab:1.0,c:1.0, d:1.0)r;");

  check_tree_attributes(t, 2, 4);

  auto current = t->vroot;

  check_node_attributes(current, "r", 1.0);
  check_node_attributes(current->back, "d", 1.0);

  current = current->next;

  check_node_attributes(current->back, "ab", 1.0);

  current = current->back->next;

  check_node_attributes(current->back, "a", 1.0);

  current = current->next;

  check_node_attributes(current->back, "b", 1.0);

  current = t->vroot->next->next;

  check_node_attributes(current->back, "c", 1.0);

  pll_utree_destroy(t, nullptr);
}

TEST(NewickParser, unicode0) {
  auto t = pll_utree_parse_newick_string("(鼠:1.0, 牛:2.0, 虎:3.0);");

  check_tree_attributes(t, 1, 3);

  auto current = t->vroot;

  check_node_attributes(current->back, "虎", 3.0);

  current = current->next;

  check_node_attributes(current->back, "鼠", 1.0);

  current = current->next;

  check_node_attributes(current->back, "牛", 2.0);

  pll_utree_destroy(t, nullptr);
}

TEST(NewickParser, unicode1) {
  auto t = pll_utree_parse_newick_string("(วัว:1.0, หมา:2.0, ค้างคาว:3.0);");

  check_tree_attributes(t, 1, 3);

  auto current = t->vroot;

  check_node_attributes(current->back, "ค้างคาว", 3.0);

  current = current->next;

  check_node_attributes(current->back, "วัว", 1.0);

  current = current->next;

  check_node_attributes(current->back, "หมา", 2.0);

  pll_utree_destroy(t, nullptr);
}

TEST(NewickParser, unicode2) {
  auto t = pll_utree_parse_newick_string("(소:1.0, 개:2.0, 박쥐:3.0);");

  check_tree_attributes(t, 1, 3);

  auto current = t->vroot;

  check_node_attributes(current->back, "박쥐", 3.0);

  current = current->next;

  check_node_attributes(current->back, "소", 1.0);

  current = current->next;

  check_node_attributes(current->back, "개", 2.0);

  pll_utree_destroy(t, nullptr);
}

TEST(NewickParser, unicode3) {
  auto t = pll_utree_parse_newick_string("(🐕:1.0, 🐄:2.0, 🦇:3.0);");

  check_tree_attributes(t, 1, 3);

  auto current = t->vroot;

  check_node_attributes(current->back, "🦇", 3.0);

  current = current->next;

  check_node_attributes(current->back, "🐕", 1.0);

  current = current->next;

  check_node_attributes(current->back, "🐄", 2.0);

  pll_utree_destroy(t, nullptr);
}

TEST(NewickParser, badtrees1) {
  EXPECT_THROW(pll_utree_parse_newick_string_unroot("((a,b),(c,(d, e)))"),
               std::runtime_error);
}

TEST(NewickParser, badtrees2) {
  EXPECT_THROW(pll_utree_parse_newick_string_unroot("((a,b);,(c,(d, e)))"),
               std::runtime_error);
}

TEST(NewickParser, badtrees3) {
  EXPECT_THROW(pll_utree_parse_newick_string_unroot("((a,b)(c,(d, e):0.5));"),
               std::runtime_error);
}

TEST(NewickParser, badtrees4) {
  EXPECT_THROW(pll_utree_parse_newick_string_unroot("((a,b),(c,(d, e)));wtf"),
               std::runtime_error);
}

TEST(NewickParser, badtrees5) {
  EXPECT_THROW(pll_utree_parse_newick_string_unroot("((a,b),(c,(d, e:0.1));"),
               std::runtime_error);
}

TEST(NewickParser, badtrees6) {
  EXPECT_THROW(pll_utree_parse_newick_string_unroot("(a,b),(c,(d, e:0.1)));"),
               std::runtime_error);
}

TEST(NewickParser, badtrees7) {
  EXPECT_THROW(pll_utree_parse_newick_string_unroot("((a,b),(c,(d, ())));"),
               std::runtime_error);
}

TEST(NewickParser, badtrees8) {
  EXPECT_THROW(pll_utree_parse_newick_string_unroot("((a,b),(c,(d, (e))));"),
               std::runtime_error);
}

TEST(NewickParser, badtrees9) {
  EXPECT_THROW(
      pll_utree_parse_newick_string_unroot("((a,b),(c,(d, e):0.5 label));"),
      std::runtime_error);
}

TEST(NewickParser, badtrees10) {
  EXPECT_THROW(pll_utree_parse_newick_string_unroot("((a,b),(c,(d, e):0.a5));"),
               std::runtime_error);
}

TEST(NewickParser, badtrees11) {
  EXPECT_THROW(
      pll_utree_parse_newick_string_unroot("((a,b),(c,(d, e:0.0:0.1)));"),
      std::runtime_error);
}

TEST(NewickParser, rooted_as_unrooted0) {
  EXPECT_THROW(pll_utree_parse_newick_string("(a,(c,d));"),
               std::invalid_argument);
}

/*
TEST(TreeReader, manytrees) {
  auto        treefile    = env->get_datafile();
  size_t      line_number = 1;
  std::string line;
  while (std::getline(treefile, line)) {
    if (!treefile) { break; }
    auto tr = TreeReader();
    try {
      auto t = tr.readTree(line);
      EXPECT_GT(t->getExternalNodeCount(), 0);
    } catch (const std::exception &e) {
      throw std::runtime_error{std::string("Got error on line ") +
                               std::to_string(line_number) +
                               " with the error: \n" + e.what()};
    }
    line_number++;
    line.clear();
  }
}

TEST(TreeReader, DISABLED_pathologic0) {
  auto treefile = env->get_pathological_data();

  size_t      line_number = 1;
  std::string line;
  while (std::getline(treefile, line)) {
    if (!treefile) { break; }
    auto tr = TreeReader();
    try {
      auto t = tr.readTree(line);
      EXPECT_GT(t->getExternalNodeCount(), 0);
    } catch (const std::exception &e) {
      throw std::runtime_error{std::string("Got error on line ") +
                               std::to_string(line_number) +
                               " with the error: \n" + e.what()};
    }
    line_number++;
    line.clear();
  }
}
*/
