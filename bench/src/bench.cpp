#include <benchmark/benchmark.h>

#include <corax/corax.h>
#include <iostream>
#include "PLLTreeInfo.hpp"
#include <string>
#include <vector>

#define STRING(s) #s
#define STRINGIFY(s) STRING(s)
#define GET_DATA(s) STRINGIFY(BENCH_DATA/s)


template <class ...Args>
void BM_Parse_Newick(benchmark::State& state, Args&&... args) {
  auto args_tuple = std::make_tuple(std::move(args)...);
  auto treePath = std::get<0>(args_tuple);
  std::vector<corax_utree_t *> toDestroy;
  for (auto _ : state) {
    auto tree = corax_utree_parse_newick_rooted(treePath);
    corax_utree_destroy(tree, nullptr);
  }
}

template <class ...Args>
void BM_kernel_likelihood(benchmark::State& state, Args&&... args) {
  auto args_tuple = std::make_tuple(std::move(args)...);
  auto treePath = std::get<0>(args_tuple);
  auto msaPath= std::get<1>(args_tuple);
  auto modelStr = std::get<2>(args_tuple);
  auto repeats = std::get<3>(args_tuple);
  auto vectorization = std::get<4>(args_tuple);
  PLLTreeInfo treeInfoWrapper(treePath,
      false,
      msaPath,
      modelStr,
      repeats,
      vectorization);
  
  auto treeinfo = treeInfoWrapper.getTreeInfo();
  double ll = corax_treeinfo_compute_loglh(treeinfo, false);
  for (auto _ : state) {
      auto v = corax_compute_edge_loglikelihood(treeinfo->partitions[0],
                                       treeinfo->root->clv_index,
                                       treeinfo->root->scaler_index,
                                       treeinfo->root->back->clv_index,
                                       treeinfo->root->back->scaler_index,
                                       treeinfo->root->pmatrix_index,
                                       treeinfo->param_indices[0],
                                       nullptr);
      assert(v == ll);
  }
}


#define GET_TREE(DATA) GET_DATA(trees/DATA ## .newick)
#define GET_MSA(DATA) GET_DATA(msas/DATA ## .phy)

#define BENCH_KERNEL(KER, VEC, MODEL, DATA) \
  BENCHMARK_CAPTURE(BM_kernel_ ## KER, \
    DNA_ ## DATA  ## _ ## MODEL ## _1_ ## VEC,\
    GET_TREE(DATA), \
    GET_MSA(DATA),\
    STRINGIFY(MODEL),\
    1,\
    CORAX_ATTRIB_ARCH_ ## VEC \
    );


  
BENCH_KERNEL(likelihood, AVX, GTR, 128)
BENCH_KERNEL(likelihood, SSE, GTR, 128)
BENCH_KERNEL(likelihood, AVX, LG, 94)
BENCH_KERNEL(likelihood, SSE, LG, 94)

#define BENCH_TREE(TREE) BENCHMARK_CAPTURE(BM_Parse_Newick, \
    tree_ ## TREE ## _taxa,  \
    GET_TREE(TREE));
 
BENCH_TREE(94)
BENCH_TREE(128)
BENCH_TREE(286)
BENCH_TREE(10575)


/*
BENCHMARK_CAPTURE(BM_Parse_Newick, 
    tree_128_taxa, 
    GET_DATA(trees/128.newick));
BENCHMARK_CAPTURE(BM_Parse_Newick, 
    tree_94_taxa, 
    GET_DATA(trees/94.newick));
BENCHMARK_CAPTURE(BM_Parse_Newick, 
    tree_286_taxa, 
    GET_DATA(trees/vertebrates_286.newick));
BENCHMARK_CAPTURE(BM_Parse_Newick, 
    tree_10575_taxa, 
    GET_DATA(trees/wol_10575.newick));
*/
BENCHMARK_MAIN();

