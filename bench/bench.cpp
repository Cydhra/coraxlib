#include <benchmark/benchmark.h>

#include <corax/corax.h>
#include <iostream>

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

BENCHMARK_CAPTURE(BM_Parse_Newick, 
    tree_10575_taxa, 
    GET_DATA(trees/wol_10575.newick));
BENCHMARK_CAPTURE(BM_Parse_Newick, 
    tree_286_taxa, 
    GET_DATA(trees/vertebrates_286.newick));


BENCHMARK_MAIN();

