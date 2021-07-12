Concepts
================================================================================

A partition is a section of the genome for which all sites evolved under the same model. Informally, one can think of a
partition being a gene, but understand that this is not always the case. In particular partitions of a genome might not
code for something, and there are no requirements that the sites which make up a partition are even _contiguous_.

Structure
================================================================================

```
typedef struct pll_partition
{
  unsigned int tips;
  unsigned int clv_buffers;
  unsigned int nodes; // tips + clv_buffer
  unsigned int states;
  unsigned int sites;
  unsigned int pattern_weight_sum;
  unsigned int rate_matrices;
  unsigned int prob_matrices;
  unsigned int rate_cats;
  unsigned int scale_buffers;
  unsigned int attributes;

  /* vectorization options */
  size_t alignment;
  unsigned int states_padded;

  double ** clv;
  double ** pmatrix;
  double * rates;
  double * rate_weights;
  double ** subst_params;
  unsigned int ** scale_buffer;
  double ** frequencies;
  double * prop_invar;
  int * invariant;
  unsigned int * pattern_weights;

  int * eigen_decomp_valid;
  double ** eigenvecs;
  double ** inv_eigenvecs;
  double ** eigenvals;

  /* tip-tip precomputation data */
  unsigned int maxstates;
  unsigned char ** tipchars;
  unsigned char * charmap;
  double * ttlookup;
  pll_state_t * tipmap;

  /* ascertainment bias correction */
  int asc_bias_alloc;
  int asc_additional_sites; // partition->asc_bias_alloc ? states : 0

  /* site repeats */
  struct pll_repeats *repeats;
} pll_partition_t;
```

This is generally organized into the following sections:

- Buffer counts and sizes
- Memory alignment state variables
- Model parameter buffers
- Buffers for the eigen[values|vectors]
- Sequence meta information
- Runtime flags
- Site repeat buffers

## Buffer Counts and Sizes

- `tips`: The number of tips present in this partition.
- `clv_buffers`: The number of CLVs, generally the number of edges in the tree.
- `nodes`: Node count. This includes the tips count.
- `states`: The number of states of the underlying data.
- `rate_matrices`: How many rate matrices present in the partition.
- `prob_matrices`: The number of edges in the tree. Practically this is the
  number of edges
- `scale_buffers`: The number of scale buffers.

None of these parameters are calculated in any way. Instead, they are specified in the `pll_partition_create` function.
This means that even non-binary trees can be (ostensibly) represented by this structure.

## Memory Alignment State Variables

- `alignment`: One of three constants depending on what architecture is being used. At the time of writing these are:
    - `PLL_ALIGNMENT_CPU`
    - `PLL_ALIGNMENT_SSE`
    - `PLL_ALIGNMENT_AVX`
- `states_padded`: How many states are used, after padding. This is also the size an individual CLV buffer.

## Attributes

The `attributes` field is a bitset that has the is combination of the following flags.

- Architecture attributes: Only one may be set
  - `PLL_ATTRIB_ARCH_CPU`
  - `PLL_ATTRIB_ARCH_SSE`
  - `PLL_ATTRIB_ARCH_AVX`
  - `PLL_ATTRIB_ARCH_AVX2`
- Ascertainment Bias: Only one of the "types" may be set, and if they are,
  `PLL_ATTRIB_AB_FLAG` must also be set
  - `PLL_ATTRIB_AB_LEWIS`
  - `PLL_ATTRIB_AB_FELSENSTEIN`
  - `PLL_ATTRIB_AB_STAMATAKIS`
  - `PLL_ATTRIB_AB_FLAG`
- Scalers
  - `PLL_ATTRIB_RATE_SCALERS`
- Optimizations: Only one may be set
  - `PLL_ATTRIB_PATTERN_TIP`
  - `PLL_ATTRIB_SITE_REPEATS`

## `clv`

In any given run, this will probably be the biggest allocation of memory, and will be involved in almost all of the
computation that `libpll` performs. As such, it can be useful to understand what CLVs are, and how the `clv` buffer
relates to them.

A conditional likelihood vector (CLV) is an intermediate calculation which informally represents the likelihood of a
subtree. Conceptually, every node (not a `pll_unode_t`) has a CLV, which is oriented with respect to the virtual root.
For more information please see the general documentation [here](./libpll.md).
