#include <benchmark/benchmark.h>

#include <corax/corax.h>
#include <iostream>
#include "PLLTreeInfo.hpp"

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
void BM_likelihood(benchmark::State& state, Args&&... args) {
  auto args_tuple = std::make_tuple(std::move(args)...);
  auto treePath = std::get<0>(args_tuple);
  auto msaPath= std::get<1>(args_tuple);
  auto modelStr = std::get<2>(args_tuple);
  PLLTreeInfo treeInfoWrapper(treePath,
      false,
      msaPath,
      modelStr);
  
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

BENCHMARK_CAPTURE(BM_likelihood, 
    DNA_128, 
    GET_DATA(trees/128.newick),
    GET_DATA(msas/128.phy),
    "GTR+G"
    );


BENCHMARK_CAPTURE(BM_Parse_Newick, 
    tree_128_taxa, 
    GET_DATA(trees/128.newick));
BENCHMARK_CAPTURE(BM_Parse_Newick, 
    tree_10575_taxa, 
    GET_DATA(trees/wol_10575.newick));
BENCHMARK_CAPTURE(BM_Parse_Newick, 
    tree_286_taxa, 
    GET_DATA(trees/vertebrates_286.newick));

BENCHMARK_MAIN();

