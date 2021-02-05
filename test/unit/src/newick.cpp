#include "corax.h"
#include <gtest/gtest.h>

TEST(NewickParser, simple0) {
  auto t = pll_utree_parse_newick_string("((a:1.0,b:1.0):1.0, c:1.0);");

  EXPECT_EQ(t->tip_count, 3);
  EXPECT_EQ(t->inner_count, 1);

  auto current = t->vroot;

  EXPECT_STREQ(current->back->label, "c");
  EXPECT_EQ(current->back->length, 2.0);
  EXPECT_EQ(current->length, 2.0);

  current = current->next;

  EXPECT_STREQ(current->back->label, "a");
  EXPECT_EQ(current->back->length, 1.0);
  EXPECT_EQ(current->length, 1.0);

  current = current->next;

  EXPECT_STREQ(current->back->label, "b");
  EXPECT_EQ(current->back->length, 1.0);
  EXPECT_EQ(current->length, 1.0);

  pll_utree_destroy(t, nullptr);
}

TEST(NewickParser, simple1) {
  auto t = pll_utree_parse_newick_string("((a,b)ab,(c,d)cd)root;");

  EXPECT_EQ(t->tip_count, 4);
  EXPECT_EQ(t->inner_count, 2);

  auto current = t->vroot;

  EXPECT_STREQ(current->label, "cd");
  EXPECT_EQ(current->length, 0.0);

  current = current->next;

  EXPECT_STREQ(current->back->label, "c");
  EXPECT_EQ(current->length, 0.0);

  current = current->next;

  EXPECT_STREQ(current->back->label, "d");
  EXPECT_EQ(current->length, 0.0);

  current = t->vroot->back;

  EXPECT_STREQ(current->label, "ab");
  EXPECT_EQ(current->length, 0.0);

  current = current->next;

  EXPECT_STREQ(current->back->label, "a");
  EXPECT_EQ(current->length, 0.0);

  current = current->next;

  EXPECT_STREQ(current->back->label, "b");
  EXPECT_EQ(current->length, 0.0);

  pll_utree_destroy(t, nullptr);
}

TEST(NewickParser, simple2) {
  auto t = pll_utree_parse_newick_string("((a,b)13,(c,d)4cd)root;");

  EXPECT_EQ(t->tip_count, 4);
  EXPECT_EQ(t->inner_count, 2);

  auto current = t->vroot;

  EXPECT_STREQ(current->label, "4cd");
  EXPECT_EQ(current->length, 0.0);

  current = current->next;

  EXPECT_STREQ(current->back->label, "c");
  EXPECT_EQ(current->length, 0.0);

  current = current->next;

  EXPECT_STREQ(current->back->label, "d");
  EXPECT_EQ(current->length, 0.0);

  current = t->vroot->back;

  EXPECT_STREQ(current->label, "13");
  EXPECT_EQ(current->length, 0.0);

  current = current->next;

  EXPECT_STREQ(current->back->label, "a");
  EXPECT_EQ(current->length, 0.0);

  current = current->next;

  EXPECT_STREQ(current->back->label, "b");
  EXPECT_EQ(current->length, 0.0);
  pll_utree_destroy(t, nullptr);
}

TEST(NewickParser, simple3) {
  auto t =
      pll_utree_parse_newick_string("((a:30.5,b:0.03):48.0,(c:0,d:3)cd)root;");

  EXPECT_EQ(t->tip_count, 4);
  EXPECT_EQ(t->inner_count, 2);

  auto current = t->vroot;

  EXPECT_STREQ(current->label, "cd");
  EXPECT_EQ(current->length, 48.0);
  EXPECT_EQ(current->back->length, 48.0);

  current = current->next;

  EXPECT_STREQ(current->back->label, "c");
  EXPECT_EQ(current->length, 0.0);
  EXPECT_EQ(current->back->length, 0.0);

  current = current->next;

  EXPECT_STREQ(current->back->label, "d");
  EXPECT_EQ(current->length, 3.0);
  EXPECT_EQ(current->back->length, 3.0);

  current = t->vroot->back;

  EXPECT_EQ(current->label, nullptr);
  EXPECT_EQ(current->length, 48.0);
  EXPECT_EQ(current->back->length, 48.0);

  current = current->next;

  EXPECT_STREQ(current->back->label, "a");
  EXPECT_EQ(current->length, 30.5);
  EXPECT_EQ(current->back->length, 30.5);

  current = current->next;

  EXPECT_STREQ(current->back->label, "b");
  EXPECT_EQ(current->length, 0.03);
  EXPECT_EQ(current->back->length, 0.03);

  pll_utree_destroy(t, nullptr);
}

TEST(NewickParser, simple4) {
  auto t =
      pll_utree_parse_newick_string("((a:1e-10,b:0.03)ab,(c:0,d:3E-5)cd)root;");

  EXPECT_EQ(t->tip_count, 4);
  EXPECT_EQ(t->inner_count, 2);

  auto current = t->vroot;

  EXPECT_STREQ(current->label, "cd");
  EXPECT_DOUBLE_EQ(current->length, 0.0);
  EXPECT_DOUBLE_EQ(current->back->length, 0.0);

  current = current->next;

  EXPECT_STREQ(current->back->label, "c");
  EXPECT_DOUBLE_EQ(current->length, 0.0);
  EXPECT_DOUBLE_EQ(current->back->length, 0.0);

  current = current->next;

  EXPECT_STREQ(current->back->label, "d");
  EXPECT_DOUBLE_EQ(current->length, 3e-5);
  EXPECT_DOUBLE_EQ(current->back->length, 3e-5);

  current = t->vroot->back;

  EXPECT_STREQ(current->label, "ab");
  EXPECT_EQ(current->length, 0.0);
  EXPECT_EQ(current->back->length, 0.0);

  current = current->next;

  EXPECT_STREQ(current->back->label, "a");
  EXPECT_DOUBLE_EQ(current->length, 1e-10);
  EXPECT_DOUBLE_EQ(current->back->length, 1e-10);

  current = current->next;

  EXPECT_STREQ(current->back->label, "b");
  EXPECT_DOUBLE_EQ(current->length, 0.03);
  EXPECT_DOUBLE_EQ(current->back->length, 0.03);

  pll_utree_destroy(t, nullptr);
}

TEST(NewickParser, simple5) {
  auto t = pll_utree_parse_newick_string(
      "( (a : 30.5 , b : 0.03 ) ab , (c :0,d : 3 ) cd )root;");

  EXPECT_EQ(t->tip_count, 4);
  EXPECT_EQ(t->inner_count, 2);

  auto current = t->vroot;

  EXPECT_STREQ(current->label, "cd");
  EXPECT_DOUBLE_EQ(current->length, 0.0);
  EXPECT_DOUBLE_EQ(current->back->length, 0.0);

  current = current->next;

  EXPECT_STREQ(current->back->label, "c");
  EXPECT_DOUBLE_EQ(current->length, 0.0);
  EXPECT_DOUBLE_EQ(current->back->length, 0.0);

  current = current->next;

  EXPECT_STREQ(current->back->label, "d");
  EXPECT_DOUBLE_EQ(current->length, 3);
  EXPECT_DOUBLE_EQ(current->back->length, 3);

  current = t->vroot->back;

  EXPECT_STREQ(current->label, "ab");
  EXPECT_EQ(current->length, 0.0);
  EXPECT_EQ(current->back->length, 0.0);

  current = current->next;

  EXPECT_STREQ(current->back->label, "a");
  EXPECT_DOUBLE_EQ(current->length, 30.5);
  EXPECT_DOUBLE_EQ(current->back->length, 30.5);

  current = current->next;

  EXPECT_STREQ(current->back->label, "b");
  EXPECT_DOUBLE_EQ(current->length, 0.03);
  EXPECT_DOUBLE_EQ(current->back->length, 0.03);

  pll_utree_destroy(t, nullptr);
}

TEST(NewickParser, simple6) {
  auto t =
      pll_utree_parse_newick_string("(\t(a\t:\t30.5\t,\tb\t:\t0.03\t)\tab\t,\t("
                                    "c\t:0,d\t:\t3\t)\tcd\t)root\t;");

  EXPECT_EQ(t->tip_count, 4);
  EXPECT_EQ(t->inner_count, 2);

  auto current = t->vroot;

  EXPECT_STREQ(current->label, "cd");
  EXPECT_DOUBLE_EQ(current->length, 0.0);
  EXPECT_DOUBLE_EQ(current->back->length, 0.0);

  current = current->next;

  EXPECT_STREQ(current->back->label, "c");
  EXPECT_DOUBLE_EQ(current->length, 0.0);
  EXPECT_DOUBLE_EQ(current->back->length, 0.0);

  current = current->next;

  EXPECT_STREQ(current->back->label, "d");
  EXPECT_DOUBLE_EQ(current->length, 3);
  EXPECT_DOUBLE_EQ(current->back->length, 3);

  current = t->vroot->back;

  EXPECT_STREQ(current->label, "ab");
  EXPECT_EQ(current->length, 0.0);
  EXPECT_EQ(current->back->length, 0.0);

  current = current->next;

  EXPECT_STREQ(current->back->label, "a");
  EXPECT_DOUBLE_EQ(current->length, 30.5);
  EXPECT_DOUBLE_EQ(current->back->length, 30.5);

  current = current->next;

  EXPECT_STREQ(current->back->label, "b");
  EXPECT_DOUBLE_EQ(current->length, 0.03);
  EXPECT_DOUBLE_EQ(current->back->length, 0.03);

  pll_utree_destroy(t, nullptr);
}

TEST(NewickParser, simple7) {
  auto t = pll_utree_parse_newick_string("((a,b)ab\n,(c,d\n)cd)root;");

  EXPECT_EQ(t->tip_count, 4);
  EXPECT_EQ(t->inner_count, 2);

  auto current = t->vroot;

  EXPECT_STREQ(current->label, "cd");
  EXPECT_DOUBLE_EQ(current->length, 0.0);
  EXPECT_DOUBLE_EQ(current->back->length, 0.0);

  current = current->next;

  EXPECT_STREQ(current->back->label, "c");
  EXPECT_DOUBLE_EQ(current->length, 0.0);
  EXPECT_DOUBLE_EQ(current->back->length, 0.0);

  current = current->next;

  EXPECT_STREQ(current->back->label, "d");
  EXPECT_DOUBLE_EQ(current->length, 0);
  EXPECT_DOUBLE_EQ(current->back->length, 0);

  current = t->vroot->back;

  EXPECT_STREQ(current->label, "ab");
  EXPECT_EQ(current->length, 0.0);
  EXPECT_EQ(current->back->length, 0.0);

  current = current->next;

  EXPECT_STREQ(current->back->label, "a");
  EXPECT_DOUBLE_EQ(current->length, 0.0);
  EXPECT_DOUBLE_EQ(current->back->length, 0.0);

  current = current->next;

  EXPECT_STREQ(current->back->label, "b");
  EXPECT_DOUBLE_EQ(current->length, 0.0);
  EXPECT_DOUBLE_EQ(current->back->length, 0.0);

  pll_utree_destroy(t, nullptr);
}

TEST(NewickParser, simple8) {
  auto t = pll_utree_parse_newick_string("((a,b)ab\r\n,(c,d\r\n)cd)root;");

  EXPECT_EQ(t->tip_count, 4);
  EXPECT_EQ(t->inner_count, 2);

  auto current = t->vroot;

  EXPECT_STREQ(current->label, "cd");
  EXPECT_DOUBLE_EQ(current->length, 0.0);
  EXPECT_DOUBLE_EQ(current->back->length, 0.0);

  current = current->next;

  EXPECT_STREQ(current->back->label, "c");
  EXPECT_DOUBLE_EQ(current->length, 0.0);
  EXPECT_DOUBLE_EQ(current->back->length, 0.0);

  current = current->next;

  EXPECT_STREQ(current->back->label, "d");
  EXPECT_DOUBLE_EQ(current->length, 0);
  EXPECT_DOUBLE_EQ(current->back->length, 0);

  current = t->vroot->back;

  EXPECT_STREQ(current->label, "ab");
  EXPECT_EQ(current->length, 0.0);
  EXPECT_EQ(current->back->length, 0.0);

  current = current->next;

  EXPECT_STREQ(current->back->label, "a");
  EXPECT_DOUBLE_EQ(current->length, 0.0);
  EXPECT_DOUBLE_EQ(current->back->length, 0.0);

  current = current->next;

  EXPECT_STREQ(current->back->label, "b");
  EXPECT_DOUBLE_EQ(current->length, 0.0);
  EXPECT_DOUBLE_EQ(current->back->length, 0.0);

  pll_utree_destroy(t, nullptr);
}

TEST(NewickParser, simple9) {
  auto t = pll_utree_parse_newick_string(
      "((!a+7=5,b^o&)ab,($$£*c,d/\\?!_-|)cd)ro#~ot;");

  EXPECT_EQ(t->tip_count, 4);
  EXPECT_EQ(t->inner_count, 2);

  auto current = t->vroot;

  EXPECT_STREQ(current->label, "cd");
  EXPECT_DOUBLE_EQ(current->length, 0.0);
  EXPECT_DOUBLE_EQ(current->back->length, 0.0);

  current = current->next;

  EXPECT_STREQ(current->back->label, "$$£*c");
  EXPECT_DOUBLE_EQ(current->length, 0.0);
  EXPECT_DOUBLE_EQ(current->back->length, 0.0);

  current = current->next;

  /* Note that the \\ is a backslash only. */
  EXPECT_STREQ(current->back->label, "d/\\?!_-|");
  EXPECT_DOUBLE_EQ(current->length, 0);
  EXPECT_DOUBLE_EQ(current->back->length, 0);

  current = t->vroot->back;

  EXPECT_STREQ(current->label, "ab");
  EXPECT_EQ(current->length, 0.0);
  EXPECT_EQ(current->back->length, 0.0);

  current = current->next;

  EXPECT_STREQ(current->back->label, "!a+7=5");
  EXPECT_DOUBLE_EQ(current->length, 0.0);
  EXPECT_DOUBLE_EQ(current->back->length, 0.0);

  current = current->next;

  EXPECT_STREQ(current->back->label, "b^o&");
  EXPECT_DOUBLE_EQ(current->length, 0.0);
  EXPECT_DOUBLE_EQ(current->back->length, 0.0);

  pll_utree_destroy(t, nullptr);
}

TEST(NewickParser, simple10) {
  auto t = pll_utree_parse_newick_string("((a[comment],b),(c,(d, e):0.5));");

  EXPECT_EQ(t->tip_count, 5);
  EXPECT_EQ(t->inner_count, 3);

  auto current = t->vroot;

  EXPECT_STREQ(current->label, nullptr);
  EXPECT_DOUBLE_EQ(current->length, 0.0);
  EXPECT_DOUBLE_EQ(current->back->length, 0.0);

  current = current->next;

  EXPECT_STREQ(current->back->label, "c");
  EXPECT_DOUBLE_EQ(current->length, 0.0);
  EXPECT_DOUBLE_EQ(current->back->length, 0.0);

  current = current->next;

  EXPECT_STREQ(current->back->label, nullptr);
  EXPECT_DOUBLE_EQ(current->length, 0.5);
  EXPECT_DOUBLE_EQ(current->back->length, 0.5);

  current = current->back->next;

  EXPECT_STREQ(current->back->label, "d");
  EXPECT_DOUBLE_EQ(current->length, 0.0);
  EXPECT_DOUBLE_EQ(current->back->length, 0.0);

  current = current->next;

  EXPECT_STREQ(current->back->label, "e");
  EXPECT_DOUBLE_EQ(current->length, 0.0);
  EXPECT_DOUBLE_EQ(current->back->length, 0.0);

  current = t->vroot->back->next;

  EXPECT_STREQ(current->back->label, "a");
  EXPECT_DOUBLE_EQ(current->length, 0.0);
  EXPECT_DOUBLE_EQ(current->back->length, 0.0);

  current = current->next;

  EXPECT_STREQ(current->back->label, "b");
  EXPECT_DOUBLE_EQ(current->length, 0.0);
  EXPECT_DOUBLE_EQ(current->back->length, 0.0);

  pll_utree_destroy(t, nullptr);
}

TEST(NewickParser, simple11) {
  auto t = pll_utree_parse_newick_string("((a,b),(c,(d, e)hello world));");

  EXPECT_EQ(t->tip_count, 5);
  EXPECT_EQ(t->inner_count, 3);

  auto current = t->vroot;

  EXPECT_STREQ(current->label, nullptr);
  EXPECT_DOUBLE_EQ(current->length, 0.0);
  EXPECT_DOUBLE_EQ(current->back->length, 0.0);

  current = current->next;

  EXPECT_STREQ(current->back->label, "c");
  EXPECT_DOUBLE_EQ(current->length, 0.0);
  EXPECT_DOUBLE_EQ(current->back->length, 0.0);

  current = current->next;

  EXPECT_STREQ(current->back->label, "hello world");
  EXPECT_DOUBLE_EQ(current->length, 0.0);
  EXPECT_DOUBLE_EQ(current->back->length, 0.0);

  current = current->back->next;

  EXPECT_STREQ(current->back->label, "d");
  EXPECT_DOUBLE_EQ(current->length, 0.0);
  EXPECT_DOUBLE_EQ(current->back->length, 0.0);

  current = current->next;

  EXPECT_STREQ(current->back->label, "e");
  EXPECT_DOUBLE_EQ(current->length, 0.0);
  EXPECT_DOUBLE_EQ(current->back->length, 0.0);

  current = t->vroot->back->next;

  EXPECT_STREQ(current->back->label, "a");
  EXPECT_DOUBLE_EQ(current->length, 0.0);
  EXPECT_DOUBLE_EQ(current->back->length, 0.0);

  current = current->next;

  EXPECT_STREQ(current->back->label, "b");
  EXPECT_DOUBLE_EQ(current->length, 0.0);
  EXPECT_DOUBLE_EQ(current->back->length, 0.0);

  pll_utree_destroy(t, nullptr);
}

TEST(TreeReader, badtrees1) {
  EXPECT_THROW(pll_utree_parse_newick_string("((a,b),(c,(d, e)))"),
               std::runtime_error);
}

TEST(TreeReader, badtrees2) {
  EXPECT_THROW(pll_utree_parse_newick_string("((a,b);,(c,(d, e)))"),
               std::runtime_error);
}

TEST(TreeReader, badtrees3) {
  EXPECT_THROW(pll_utree_parse_newick_string("((a,b)(c,(d, e):0.5));"),
               std::runtime_error);
}

TEST(TreeReader, badtrees4) {
  EXPECT_THROW(pll_utree_parse_newick_string("((a,b),(c,(d, e)));wtf"),
               std::runtime_error);
}

TEST(TreeReader, badtrees5) {
  EXPECT_THROW(pll_utree_parse_newick_string("((a,b),(c,(d, e:0.1));"),
               std::runtime_error);
}

TEST(TreeReader, badtrees6) {
  EXPECT_THROW(pll_utree_parse_newick_string("(a,b),(c,(d, e:0.1)));"),
               std::runtime_error);
}

TEST(TreeReader, badtrees7) {
  EXPECT_THROW(pll_utree_parse_newick_string("((a,b),(c,(d, ())));"),
               std::runtime_error);
}

TEST(TreeReader, badtrees8) {
  EXPECT_THROW(pll_utree_parse_newick_string("((a,b),(c,(d, (e))));"),
               std::runtime_error);
}

TEST(TreeReader, badtrees9) {
  EXPECT_THROW(pll_utree_parse_newick_string("((a,b),(c,(d, e):0.5 label));"),
               std::runtime_error);
}

TEST(TreeReader, badtrees10) {
  EXPECT_THROW(pll_utree_parse_newick_string("((a,b),(c,(d, e):0.a5));"),
               std::runtime_error);
}

TEST(TreeReader, badtrees11) {
  EXPECT_THROW(pll_utree_parse_newick_string("((a,b),(c,(d, e:0.0:0.1)));"),
               std::runtime_error);
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
