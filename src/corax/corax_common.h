/*
    Copyright (C) 2015-2020 Tomas Flouri, Diego Darriba, Alexey Kozlov

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Contact: Tomas Flouri <Tomas.Flouri@h-its.org>,
    Exelixis Lab, Heidelberg Instutute for Theoretical Studies
    Schloss-Wolfsbrunnenweg 35, D-69118 Heidelberg, Germany
*/

#ifndef CORAX_COMMON_H_
#define CORAX_COMMON_H_
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if (!defined(__clang__) && defined(__GNUC__)                                  \
     && (__GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 7)))
#if ((__GNUC__ == 4) && (__GNUC_MINOR__ == 6))
#if (defined(HAVE_AVX2))
#error "GCC 4.6.x. Please run ./configure --disable-avx2"
#endif
#else
#if (defined(HAVE_AVX2) || defined(HAVE_AVX))
#error "GCC < 4.6. Please run ./configure --disable-avx --disable-avx2"
#endif
#endif
#endif

#ifdef HAVE_X86INTRIN_H
#include <x86intrin.h>
#endif

/* platform specific */

#if (!defined(__APPLE__) && !defined(__WIN32__) && !defined(__WIN64__))
#include <sys/sysinfo.h>
#endif

#if (defined(__WIN32__) || defined(__WIN64__))
#define PLL_EXPORT __declspec(dllexport)
#else
#define PLL_EXPORT
#endif

/* macros */

#define PLL_MIN(a, b) ((a) < (b) ? (a) : (b))
#define PLL_MAX(a, b) ((a) > (b) ? (a) : (b))
#define PLL_SWAP(x, y)                                                         \
  do {                                                                         \
    __typeof__(x) _t = x;                                                      \
    x                = y;                                                      \
    y                = _t;                                                     \
  } while (0)
#define PLL_STAT(x)                                                            \
  ((pll_hardware.init || pll_hardware_probe()) && pll_hardware.x)
#define PLL_UNUSED(expr)                                                       \
  do {                                                                         \
    (void)(expr);                                                              \
  } while (0)

#define PLL_UTREE_IS_TIP(node) (node->next == NULL)

/** @defgroup pll_defines Constant Definitions 
 * @{ 
 */
/* constants */
#define PLL_FAILURE 0
#define PLL_SUCCESS 1

#define PLL_FALSE 0
#define PLL_TRUE 1

#define PLL_ALIGNMENT_CPU 8
#define PLL_ALIGNMENT_SSE 16
#define PLL_ALIGNMENT_AVX 32

#define PLL_LINEALLOC 2048

#define PLL_ASCII_SIZE 256

#define PLL_ERRMSG_LEN 200

#define PLL_SCALE_FACTOR                                                       \
  115792089237316195423570985008687907853269984665640564039457584007913129639936.0 /*  2**256 (exactly)  */
#define PLL_SCALE_THRESHOLD (1.0 / PLL_SCALE_FACTOR)
#define PLL_SCALE_FACTOR_SQRT                                                  \
  340282366920938463463374607431768211456.0 /* 2**128 */
#define PLL_SCALE_THRESHOLD_SQRT (1.0 / PLL_SCALE_FACTOR_SQRT)
#define PLL_SCALE_BUFFER_NONE -1

/* in per-rate scaling mode, maximum difference between scalers
 * please see https://github.com/xflouris/libpll/issues/44  */
#define PLL_SCALE_RATE_MAXDIFF 4

#define PLL_MISC_EPSILON 1e-8
#define PLL_ONE_EPSILON 1e-15
#define PLL_ONE_MIN (1 - PLL_ONE_EPSILON)
#define PLL_ONE_MAX (1 + PLL_ONE_EPSILON)
#define PLL_EIGEN_MINFREQ 1e-6

#define PLL_TREE_DEFAULT_BRANCH_LENGTH 0.1
/** @} */

/** @defgroup pll_attributes Attributes
 * These are flags which are used to control the behavior of a partition.
 * @{
 */

/* attribute flags */


/** Flag specifying no SIMD operations */
#define PLL_ATTRIB_ARCH_CPU 0
/** Flag specifying only SSE3 SIMD operations */
#define PLL_ATTRIB_ARCH_SSE (1 << 0)
/** Flag specifying only AVX SIMD operations */
#define PLL_ATTRIB_ARCH_AVX (1 << 1)
/** Flag specifying only AVX2 SIMD operations */
#define PLL_ATTRIB_ARCH_AVX2 (1 << 2)
/** Flag specifying only AVX512 SIMD operations */
#define PLL_ATTRIB_ARCH_AVX512 (1 << 3)
/** Mask for the CPU architecture attributes */
#define PLL_ATTRIB_ARCH_MASK 0xF

/** 
 * Flag which indicates the use of the pattern tip optimization. Mutually
 * exclusive with the `PLL_ATTRIB_SITE_REPEATS` flag.
 */
#define PLL_ATTRIB_PATTERN_TIP (1 << 4)

/* ascertainment bias correction */
#define PLL_ATTRIB_AB_LEWIS (1 << 5)
#define PLL_ATTRIB_AB_FELSENSTEIN (2 << 5)
#define PLL_ATTRIB_AB_STAMATAKIS (3 << 5)
#define PLL_ATTRIB_AB_MASK (7 << 5)
#define PLL_ATTRIB_AB_FLAG (1 << 8)

#define PLL_ATTRIB_RATE_SCALERS (1 << 9)

/* site repeats */

/** 
 * Flag indicating the use of the site repeats optimization. Mutually exclusive
 * with the `PLL_ATTRIB_PATTERN_TIP` flag.
 */
#define PLL_ATTRIB_SITE_REPEATS (1 << 10)

/** Mask for all the attributes currently defined */
#define PLL_ATTRIB_MASK ((1 << 11) - 1)

/** @} */

#define PLL_REPEATS_LOOKUP_SIZE 2000000

/* topological rearrangements */

#define PLL_UTREE_MOVE_SPR 1
#define PLL_UTREE_MOVE_NNI 2

#define PLL_UTREE_MOVE_NNI_LEFT 1
#define PLL_UTREE_MOVE_NNI_RIGHT 2

#define PLL_TREE_TRAVERSE_POSTORDER 1
#define PLL_TREE_TRAVERSE_PREORDER 2

/** @defgroup pll_errors Error Codes
 * Error codes for coraxlib.
 * @{
 */
/* error codes */
#define PLL_ERROR_NOT_IMPLEMENTED 13
#define PLL_ERROR_INVALID_RANGE 21
#define PLL_ERROR_INVALID_NODE_TYPE 22
#define PLL_ERROR_INVALID_INDEX 23
#define PLL_ERROR_INVALID_PARAM 24
#define PLL_ERROR_INVALID_TREE 25
#define PLL_ERROR_INVALID_TREE_SIZE 26
#define PLL_ERROR_INVALID_SPLIT 27
#define PLL_ERROR_INVALID_THRESHOLD 28

#define PLL_ERROR_FILE_OPEN 100
#define PLL_ERROR_FILE_SEEK 101
#define PLL_ERROR_FILE_EOF 102
#define PLL_ERROR_FASTA_ILLEGALCHAR 201
#define PLL_ERROR_FASTA_UNPRINTABLECHAR 202
#define PLL_ERROR_FASTA_INVALIDHEADER 203
#define PLL_ERROR_FASTA_NONALIGNED 204
#define PLL_ERROR_PHYLIP_SYNTAX 231
#define PLL_ERROR_PHYLIP_LONGSEQ 232
#define PLL_ERROR_PHYLIP_NONALIGNED 233
#define PLL_ERROR_PHYLIP_ILLEGALCHAR 234
#define PLL_ERROR_PHYLIP_UNPRINTABLECHAR 235
#define PLL_ERROR_NEWICK_SYNTAX 111
#define PLL_ERROR_MEM_ALLOC 112
#define PLL_ERROR_TIPDATA_ILLEGALSTATE 114
#define PLL_ERROR_TIPDATA_ILLEGALFUNCTION 115
#define PLL_ERROR_TREE_CONVERSION 116
#define PLL_ERROR_INVAR_INCOMPAT 117
#define PLL_ERROR_INVAR_PROPORTION 118
#define PLL_ERROR_INVAR_PARAMINDEX 119
#define PLL_ERROR_INVAR_NONEFOUND 120
#define PLL_ERROR_AB_INVALIDMETHOD 121
#define PLL_ERROR_AB_NOSUPPORT 122
#define PLL_ERROR_SPR_TERMINALBRANCH 123
#define PLL_ERROR_SPR_NOCHANGE 124
#define PLL_ERROR_NNI_INVALIDMOVE 125
#define PLL_ERROR_NNI_TERMINALBRANCH 126
#define PLL_ERROR_STEPWISE_STRUCT 127
#define PLL_ERROR_STEPWISE_TIPS 128
#define PLL_ERROR_STEPWISE_UNSUPPORTED 129
#define PLL_ERROR_EINVAL 130
#define PLL_ERROR_MSA_EMPTY 131
#define PLL_ERROR_MSA_MAP_INVALID 132
/** @} */

/* utree specific */

#define PLL_UTREE_SHOW_LABEL (1 << 0)
#define PLL_UTREE_SHOW_BRANCH_LENGTH (1 << 1)
#define PLL_UTREE_SHOW_CLV_INDEX (1 << 2)
#define PLL_UTREE_SHOW_SCALER_INDEX (1 << 3)
#define PLL_UTREE_SHOW_PMATRIX_INDEX (1 << 4)
#define PLL_UTREE_SHOW_DATA (1 << 5)

/* GAMMA discretization modes */
#define PLL_GAMMA_RATES_MEAN 0
#define PLL_GAMMA_RATES_MEDIAN 1

/* branch linkage modes */
#define PLL_BRLEN_LINKED    0
#define PLL_BRLEN_SCALED    1
#define PLL_BRLEN_UNLINKED  2

/* parallel reduction modes */
#define PLL_REDUCE_SUM     0
#define PLL_REDUCE_MAX     1
#define PLL_REDUCE_MIN     2

// TODO: this must be adapted for MSVC
#define PLL_POPCNT32 __builtin_popcount
#define PLL_POPCNT64 __builtin_popcountll
#define PLL_CTZ32 __builtin_ctz
#define PLL_CTZ64 __builtin_ctzll

/* structures and data types */

#define PLL_STATE_POPCNT PLL_POPCNT64
#define PLL_STATE_CTZ PLL_CTZ64

typedef unsigned long long pll_state_t;
typedef int                pll_bool_t;

typedef struct pll_hardware_s
{
  int init;
  /* cpu features */
  int altivec_present;
  int mmx_present;
  int sse_present;
  int sse2_present;
  int sse3_present;
  int ssse3_present;
  int sse41_present;
  int sse42_present;
  int popcnt_present;
  int avx_present;
  int avx2_present;

  /* TODO: add chip,core,mem info */
} pll_hardware_t;

struct pll_repeats;

/** @defgroup pll_partition_t pll_partition_t
 * Module concerning the pll_partition_t
 */

/**
 * A partition is a section of the genome for which all sites evolved under the
 * same model. Informally, one can think of a partition being a gene, but
 * understand that this is not always the case. In particular partitions of a
 * genome might not code for something, and there are no requirements that the
 * sites which make up a partition are even contiguous.
 * 
 * Here is a checklist for creating and using a new partition:
 * - Create the partition.
 * - Set the substitution parameters,
 * - Set the tip states:
 *   - Compress the MSA first.
 *   - Set the pattern weights as well.
 * - Set the frequencies.
 * - Set the proportion of invariant sites.
 * - Set the category rates.
 * - Set the category weights.
 * 
 * @ingroup pll_partition_t
 */
typedef struct pll_partition
{
  /**
   * Number of tips present in this partition. Also, the column length in the
   * MSA.
   */
  unsigned int tips;

  /**
   * Number of CLV buffers. Typically, this is the number of edges in the tree
   */
  unsigned int clv_buffers;

  /**
   * The number of "conceptual" nodes in the tree. This is to say, the number of
   * nodes in the tree, and not the number of `pll_unode_t` present in the tree
   * structure. Includes the tips.
   */
  unsigned int nodes; // tips + clv_buffer

  /**
   * Number of states the for the current partition model. For example, a DNA
   * model will use 4 states.
   */
  unsigned int states;

  /**
   * Number of sites in the MSA. Typically, this is the number of _unique_
   * sites, as there are functions to compress the MSA into only unique sites.
   */
  unsigned int sites;

  /**
   * The sum of the pattern weights. When an MSA is compressed, it is compressed
   * into a list of unique sites with each associated with a pattern weight.
   * Typically, this is the length of the uncompressed MSA. The exception is
   * when a weighted MSA is being used. In this case, it will be the sum of the
   * MSA weights.
   */
  unsigned int pattern_weight_sum;
  
  /**
   * How many rate matrices are present in the partition. This is different than
   * the number of rate _categories_. This is instead to be able to specify a
   * mixture model. Typically though, this is 1
   */
  unsigned int rate_matrices;

  /**
   * The number of probability matrices that will be used for computation of a
   * likelihood. Practically, this is going to be the number of edges in the
   * tree.
   */
  unsigned int prob_matrices;

  /**
   * Number of rate categories for the partition. When specifying other sizes,
   * the number of rate categories does not need to be accounted for, as it will
   * be tracked by the partition.
   */
  unsigned int rate_cats;

  /**
   * Number of scale buffers.
   */
  unsigned int scale_buffers;

  /**
   * Bitvector of the flags used for computation in the `pll_partition_t`. 
   *
   * @ingroup pll_attributes
   */
  unsigned int attributes;

  /* vectorization options */

  /**
   * One of three constants depending on what architecture is being used. At the
   * time of writing these are:
   * - `PLL_ALIGNMENT_CPU`
   * - `PLL_ALIGNMENT_SSE`
   * - `PLL_ALIGNMENT_AVX`
   */
  size_t       alignment;

  /**
   * How many states are used, after padding. This is also the size of an
   * individual CLV buffer.
   */
  unsigned int states_padded;

  double **      clv;
  double **      pmatrix;
  double *       rates;
  double *       rate_weights;
  double **      subst_params;
  unsigned int **scale_buffer;
  double **      frequencies;
  double *       prop_invar;
  int *          invariant;
  unsigned int * pattern_weights;

  int *    eigen_decomp_valid;
  double **eigenvecs;
  double **inv_eigenvecs;
  double **eigenvals;

  /* tip-tip precomputation data */
  unsigned int    maxstates;
  unsigned char **tipchars;
  unsigned char * charmap;
  double *        ttlookup;
  pll_state_t *   tipmap;

  /* ascertainment bias correction */
  int asc_bias_alloc;
  int asc_additional_sites; // partition->asc_bias_alloc ? states : 0

  /* site repeats */
  /** 
   * If repeats are disabled, repeats is set to NULL. Otherwise, it points to a
   * structure holding all information required to use the site repeats
   * optimization.
   */
  struct pll_repeats *repeats;
} pll_partition_t;

/** @defgroup pll_repeats_t pll_repeats_t
 * Module relating to the repeats structure
 */

/**
 *  Site repeats is a technique that, for each node of a tree,
 *  compresses all the site CLVs that are expected to be equal,
 *  in order to save memory and computations.
 *
 *  Let u be a node, and let i and j be two sites. If the sites 
 *  i and j are equal in all the sequences under the node u, then
 *  their CLVs are also equal, and thus do not need to be computed/stored
 *  twice. We say that they belong to the same repeat class. 
 *
 *  For a given node, the site repeats technique identifies all the 
 *  different repeat classes, and associates to each of them a unique
 *  class identifier (starting from 1 for each node), which is required
 *  to find the location of the CLV of this repeat class
 *
 *  Note that, if scaling is enabled, scalers are also (similarly to
 *  clvs) compressed by site repeats
 *
 *  @ingroup pll_repeats_t
 */
typedef struct pll_repeats
{
  /**
   * `pernode_site_id[u->clv_index][i]` is equal to the class identifier of the
   * site `i` at node `u` (all the sites that have the same class identifier
   * under the same node belong to the same repeat class)
   */
  unsigned int **pernode_site_id;

  /**
   * `pernode_id_site[u->clv_index][id]` is the first site which belongs to the
   * repeat class with identifier `id` at node `u`.
   */
  unsigned int **pernode_id_site;

  /**
   * `pernode_ids[u->clv_index]` is the number of different repeat classes (and
   * thus of different class idenfiers) at node `u`
   */
  unsigned int *pernode_ids;

  /**
   * `perscale_ids[scaler_index]` is the number of different repeat classes for
   * the scaler associated with the `id` scaler_index.  For a given
   * `pll_operation_t *op`, if scalers are enabled:
   * `pernode_ids[op->parent_clv_index] == perscale_ids[op->parent_scaler_index]`
   */
  unsigned int *perscale_ids;

  /** `pernode_allocated_clvs[u->clv_index] * size_of_a_site_clv` is the total
   * size of the vector that was allocated for the (compressed) CLV of the node
   * `u`, where `size_of_a_site_clv` is the size of CLV chunk corresponding to
   * one site and one node. Note that this size might be larger than required
   * (its similar to the capacity of an STL container that can be larger than
   * its size)
   */
  unsigned int *pernode_allocated_clvs;

  /**
   * Returns `true` if site repeats compression should be applied on the parent
   * node of `left_clv` and `right_clv`. In particular, applying site repeats on
   * nodes that are close to the (virtual) root of the tree is often
   * counterproductive.  This function can be redefined, and its default
   * definition is is `pll_default_enable_repeats` 
   */
  unsigned int (*enable_repeats)(struct pll_partition *partition,
                                 unsigned int          left_clv,
                                 unsigned int          right_clv);

  /**
   * callback called when repeats are updated. Reallocate the CLV
   * and scaler vector for the node whose clv_index is parent. 
   * sites_to_alloc indicates the number of "unique sites", 
   * (or rather class identifiers) for this node.
   * 
   * This function can be redefined and its default definition
   * is pll_default_reallocate_repeats
   * 
   * By default, we always reallocate the exact required size, in
   * order to save memory. An alternative strategy could consist
   * in preallocating the maximum size (assuming that there is no
   * repeat) once at the first call, and then not doing anything
   * at the next calls (to avoid deallocation/reallocation).
   */
  void (*reallocate_repeats)(struct pll_partition *partition,
                             unsigned int          parent,
                             int                   scaler_index,
                             unsigned int          sites_to_alloc);

  /*
   * The `lookup_buffer` corresponds to the "matrix M" in the original site
   * repeats publication. It is used to compute the repeat classes at a given
   * node `u`. Let `v` and `w` be the children of `u`, and let `nv` and `nw` be
   * their respective numbers of class repeats. If `nv*nw > lookup_buffer_size`,
   * site repeats cannot be computed, and we disable site repeats for node `u`.
   * Thus, `lookup_buffer_size` should be large enough to allow as much nodes as
   * possible to benefit from site repeats, without costing too much memory
   * (remember, there might be many partitions...). Default size is
   * `PLL_REPEATS_LOOKUP_SIZE` and can be changed with
   * `pll_resize_repeats_lookup`
   */
  unsigned int *lookup_buffer;
  unsigned int  lookup_buffer_size;

  /**
   * Map each character (representing a state) to a unique identifier 
   */
  char *        charmap;
  
  /**
   * Those vectors are pre-allocated buffers for the site repeats algorithm (or
   * for using site repeats in some kernels functions). They are only relevant
   * in the scope of the function in which they are being used.
   */
  unsigned int *toclean_buffer;
  unsigned int *id_site_buffer;
  double *      bclv_buffer;

} pll_repeats_t;

/** @defgroup pll_operation_t pll_operation_t
 * Module containing structures and functions related to operations.
 */

/**
 * Structure for driving likelihood operations. In general should only be
 * created using `pll_utree_create_operations`.
 *
 * @ingroup pll_operation_t
 */
typedef struct pll_operation
{
  unsigned int parent_clv_index;
  int          parent_scaler_index;
  unsigned int child1_clv_index;
  unsigned int child1_matrix_index;
  int          child1_scaler_index;
  unsigned int child2_clv_index;
  unsigned int child2_matrix_index;
  int          child2_scaler_index;
} pll_operation_t;

/* Doubly-linked list */

typedef struct pll_dlist
{
  struct pll_dlist *next;
  struct pll_dlist *prev;
  void *            data;
} pll_dlist_t;

/* multiple sequence alignment */
typedef struct pll_msa_s
{
  int count;
  int length;

  char **sequence;
  char **label;
} pll_msa_t;

/* Simple structure for handling FASTA parsing */

typedef struct pll_fasta
{
  FILE *              fp;
  char                line[PLL_LINEALLOC];
  const unsigned int *chrstatus;
  long                no;
  long                filesize;
  long                lineno;
  long                stripped_count;
  long                stripped[256];
} pll_fasta_t;

/* Simple structure for handling PHYLIP parsing */
typedef struct pll_phylip_s
{
  FILE *              fp;
  char *              line;
  size_t              line_size;
  size_t              line_maxsize;
  char                buffer[PLL_LINEALLOC];
  const unsigned int *chrstatus;
  long                no;
  long                filesize;
  long                lineno;
  long                stripped_count;
  long                stripped[256];
} pll_phylip_t;

/**
 * A structure that is a fundamental element of `pll_utree_t`. It contains a
 * next and back pointer. For more information, please see docs/pll_utree_t.md
 *
 */
typedef struct pll_unode_s
{
  /**
   * Label for the tree. Optional. If not present, then should be set to
   * `nullptr`
   */
  char *              label;

  /**
   * Length of the edge, which is represented by the back pointer
   */
  double              length;

  /**
   * Index of this node in the `nodes` buffer of `pll_utree_t`. Each
   * "super"-node shares and index. I.E. the index is on the "tree node" level,
   * not on the pll_unode_t level.
   */
  unsigned int        node_index;

  /**
   * Index into the CLV buffer when computing a likelihood. For more
   * information, please see the documentation on `pll_partition_t`.
   */
  unsigned int        clv_index;

  /**
   * Index into the scalar array to represent the CLV scaler. Please see the
   * documentation on `pll_partition_t` for more information.
   */
  int                 scaler_index;

  /**
   * Index into the array of probability matrices. These probability matrices
   * will be computed based on the branch length `length`. For more information
   * please see the documentation on `pll_partition_t`.
   */
  unsigned int        pmatrix_index;

  /**
   * See the explaination in the concepts section of `docs/pll_utree_t.md`
   */
  struct pll_unode_s *next;

  /**
   * See the explaination in the concepts section of `docs/pll_utree_t.md`
   */
  struct pll_unode_s *back;

  /**
   * An extra pointer to store "user data". In praactice, this section can be
   * used for any task, but exsiting functions might use it, so be careful.
   */
  void *data;
} pll_unode_t;

/** @defgroup pll_utree_t pll_utree_t
 * Module for the `pll_utree_t` struct and associated functions
 */

/**
 * The data structure is made up of two different structs. The first,
 * pll_utree_t wraps the tree. In general, when a tree is used for a function,
 * it requires a pll_utree_t. Some important things to know about this
 * structure: the first inner_count nodes in the nodes array are assumed to be
 * "inner nodes". This means that they have a non-null next pointer. Several
 * functions that use pll_utree_ts don't check for this, so they may fail when
 * this assumption is violated. To avoid this, use the pll_utree_wraptree
 * function discussed below to create a pll_utree_t.
 *
 * @ingroup pll_utree_t
 */
typedef struct pll_utree_s
{
  /**
   * Number of tips in the tree
   */
  unsigned int tip_count;

  /**
   * Number of inner nodes. Not the number of `pll_unode_t` that make up the
   * tree, but the number of conceptual nodes on the phylogenetic tree.
   */
  unsigned int inner_count;

  /**
   * The number of edges in the tree
   */
  unsigned int edge_count;

  /**
   * Flag indicating if the tree is binary
   */
  int          binary;

  /**
   * An array of `pll_unode_t` pointers
   */
  pll_unode_t **nodes;

  /**
   * A pointer to the virtual root. By convention, this is always an inner node.
   * All tree manipulation functions such as `pll_utree_wraptree` follow this
   * convention.
   */
  pll_unode_t * vroot;
} pll_utree_t;

/* structures for parsimony */

typedef struct pll_parsimony_s
{
  /* common information */
  unsigned int tips;
  unsigned int inner_nodes;
  unsigned int sites;
  unsigned int states;
  unsigned int attributes;
  size_t       alignment;

  /* fast unweighted parsimony */
  unsigned int **packedvector;
  unsigned int * node_cost;
  unsigned int   packedvector_count;
  unsigned int   const_cost;
  int *          informative;
  unsigned int   informative_count;

  /* weighted parsimony */
  unsigned int   score_buffers;
  unsigned int   ancestral_buffers;
  double *       score_matrix;
  double **      sbuffer;
  unsigned int **anc_states;
} pll_parsimony_t;

typedef struct pll_pars_buildop_s
{
  unsigned int parent_score_index;
  unsigned int child1_score_index;
  unsigned int child2_score_index;
} pll_pars_buildop_t;

typedef struct pll_pars_recop_s
{
  unsigned int node_score_index;
  unsigned int node_ancestral_index;
  unsigned int parent_score_index;
  unsigned int parent_ancestral_index;
} pll_pars_recop_t;

/* structures for SVG visualization */

typedef struct pll_svg_attrib_s
{
  int    precision;
  long   width;
  long   font_size;
  long   tip_spacing;
  long   stroke_width;
  long   legend_show;
  long   legend_spacing;
  long   margin_left;
  long   margin_right;
  long   margin_bottom;
  long   margin_top;
  long   node_radius;
  double legend_ratio;
} pll_svg_attrib_t;

/* Reentrant versions of the `random' family of functions.
   These functions all use the following data structure to contain
   state, rather than global state variables. Taken and modified from
   glibc 2.23 */

struct pll_random_data
{
  int32_t *fptr;      /* Front pointer.  */
  int32_t *rptr;      /* Rear pointer.  */
  int32_t *state;     /* Array of state values.  */
  int      rand_type; /* Type of random number generator.  */
  int      rand_deg;  /* Degree of random number generator.  */
  int      rand_sep;  /* Distance between front and rear.  */
  int32_t *end_ptr;   /* Pointer behind state table.  */
};

typedef struct pll_random_state_s
{
  struct pll_random_data rdata;
  char *                 state_buf; /* Buffer to store state */
} pll_random_state;

/* common data */

PLL_EXPORT extern __thread int            pll_errno;
PLL_EXPORT extern __thread char           pll_errmsg[200];
PLL_EXPORT extern __thread pll_hardware_t pll_hardware;

PLL_EXPORT extern const pll_state_t  pll_map_bin[256];
PLL_EXPORT extern const pll_state_t  pll_map_nt[256];
PLL_EXPORT extern const pll_state_t  pll_map_aa[256];
PLL_EXPORT extern const pll_state_t  pll_map_gt10[256];
PLL_EXPORT extern const unsigned int pll_map_fasta[256];
PLL_EXPORT extern const unsigned int pll_map_phylip[256];
PLL_EXPORT extern const unsigned int pll_map_generic[256];

PLL_EXPORT extern const double pll_aa_rates_dayhoff[190];
PLL_EXPORT extern const double pll_aa_rates_lg[190];
PLL_EXPORT extern const double pll_aa_rates_dcmut[190];
PLL_EXPORT extern const double pll_aa_rates_jtt[190];
PLL_EXPORT extern const double pll_aa_rates_mtrev[190];
PLL_EXPORT extern const double pll_aa_rates_wag[190];
PLL_EXPORT extern const double pll_aa_rates_rtrev[190];
PLL_EXPORT extern const double pll_aa_rates_cprev[190];
PLL_EXPORT extern const double pll_aa_rates_vt[190];
PLL_EXPORT extern const double pll_aa_rates_blosum62[190];
PLL_EXPORT extern const double pll_aa_rates_mtmam[190];
PLL_EXPORT extern const double pll_aa_rates_mtart[190];
PLL_EXPORT extern const double pll_aa_rates_mtzoa[190];
PLL_EXPORT extern const double pll_aa_rates_pmb[190];
PLL_EXPORT extern const double pll_aa_rates_hivb[190];
PLL_EXPORT extern const double pll_aa_rates_hivw[190];
PLL_EXPORT extern const double pll_aa_rates_jttdcmut[190];
PLL_EXPORT extern const double pll_aa_rates_flu[190];
PLL_EXPORT extern const double pll_aa_rates_stmtrev[190];
PLL_EXPORT extern const double pll_aa_rates_den[190];
PLL_EXPORT extern const double pll_aa_rates_lg4m[4][190];
PLL_EXPORT extern const double pll_aa_rates_lg4x[4][190];

PLL_EXPORT extern const double pll_aa_freqs_dayhoff[20];
PLL_EXPORT extern const double pll_aa_freqs_lg[20];
PLL_EXPORT extern const double pll_aa_freqs_dcmut[20];
PLL_EXPORT extern const double pll_aa_freqs_jtt[20];
PLL_EXPORT extern const double pll_aa_freqs_mtrev[20];
PLL_EXPORT extern const double pll_aa_freqs_wag[20];
PLL_EXPORT extern const double pll_aa_freqs_rtrev[20];
PLL_EXPORT extern const double pll_aa_freqs_cprev[20];
PLL_EXPORT extern const double pll_aa_freqs_vt[20];
PLL_EXPORT extern const double pll_aa_freqs_blosum62[20];
PLL_EXPORT extern const double pll_aa_freqs_mtmam[20];
PLL_EXPORT extern const double pll_aa_freqs_mtart[20];
PLL_EXPORT extern const double pll_aa_freqs_mtzoa[20];
PLL_EXPORT extern const double pll_aa_freqs_pmb[20];
PLL_EXPORT extern const double pll_aa_freqs_hivb[20];
PLL_EXPORT extern const double pll_aa_freqs_hivw[20];
PLL_EXPORT extern const double pll_aa_freqs_jttdcmut[20];
PLL_EXPORT extern const double pll_aa_freqs_flu[20];
PLL_EXPORT extern const double pll_aa_freqs_stmtrev[20];
PLL_EXPORT extern const double pll_aa_freqs_den[20];
PLL_EXPORT extern const double pll_aa_freqs_lg4m[4][20];
PLL_EXPORT extern const double pll_aa_freqs_lg4x[4][20];

#ifdef __cplusplus
extern "C"
{
#endif

  /* functions in common.c */

  void pll_set_error(int _errno, const char *errmsg_fmt, ...);
  void pll_reset_error();

  /* functions in partition.c */

  /**
   * Creates a partition. The checklist for creating a new partition is:
   *
   * @param tips The number of tips of the tree. In phylogenetic terms, this is
   * the number of taxa.
   *
   * @param clv_buffers This is the number of CLVs that will be required to
   * compute the tree. Practically, this is the number of edges, or the number
   * of inner nodes. The number of rate categories is automatically accounted
   * for, so no need to add it.
   *
   * @param states The number of states that the model has, I.E. the type of
   * sequence data that is being worked on. Practically, this is:
   * - 2 for binary data,
   * - 4 for nucleotide data,
   * - 20 for amino acid data,
   * - 61 for codon data,
   *
   * @param sites How long is the alignment. Note that, this is going to be the
   * post compressed length of the sequence, i.e. the length that is from
   * pll_compress_site_patterns, or the number of unique site patterns.
   *
   * @param rate_matrices The number of rate matrices that are allocated. In a
   * simple and standard model, this is 1. In the case of a mixture model, this
   * should be equal to the number of classes of models. Note that having rate
   * categories still only requires 1 rate matrix, as those are computed based
   * on the single rate matrix.
   *
   * @param prob_matrices  The number of probability matrices need for
   * calculation. This will almost always be equal to the number of branches in
   * the tree. **IMPORTANT**: the number of rate categories is automatically
   * accounted for.
   *
   * @param rate_cats Number of different rate categories to consider.
   *
   * @param scale_buffers Number of scaling buffers to allocate. Practically,
   * this is equal to the number of inner nodes in the tree.
   *
   * @param attributes Please see the Attributes module.
   *
   * @return The created partition.
   *
   * @ingroup pll_partition_t
   */
  PLL_EXPORT pll_partition_t *pll_partition_create(unsigned int tips,
                                                   unsigned int clv_buffers,
                                                   unsigned int states,
                                                   unsigned int sites,
                                                   unsigned int rate_matrices,
                                                   unsigned int prob_matrices,
                                                   unsigned int rate_cats,
                                                   unsigned int scale_buffers,
                                                   unsigned int attributes);

  /** 
   * Destroys the partition, deallocating the memory.
   *
   * @param partition The partition to be destroyed.
   *
   * @ingroup pll_partition_t
   */
  PLL_EXPORT void pll_partition_destroy(pll_partition_t *partition);

  /**
   * Set the tip states based on an MSA for a partition. This will initialize
   * the tip CLVs based on the MSA that is passed.
   *
   * @param partition Partition to set the tips for.
   *
   * @param tip_index Index of the tip to be initialized.
   *
   * @param map A predefined map from `char` to `int` The choices are:
   * - pll_map_bin: For binary data with an alphabet of 0 and 1.
   * - pll_map_nt: For nucleotide data.
   * - pll_map_aa: For amino acid data.
   *
   * @param sequence The sequence associated with that tip.
   *
   * @ingroup pll_partition_t
   */
  PLL_EXPORT int pll_set_tip_states(pll_partition_t *  partition,
                                    unsigned int       tip_index,
                                    const pll_state_t *map,
                                    const char *       sequence);

  PLL_EXPORT int pll_set_tip_clv(pll_partition_t *partition,
                                 unsigned int     tip_index,
                                 const double *   clv,
                                 int              padding);

  /**
   * Sets the pattern weights for a partition.
   *
   * @param partition The partition to set the weights on.
   *
   * @param pattern_weights The array of weights. While you could set this
   * yourself, it should typically be the output of `pll_compress_site_patterns`
   *
   * @ingroup pll_partition_t
   */
  PLL_EXPORT void pll_set_pattern_weights(pll_partition_t *   partition,
                                          const unsigned int *pattern_weights);

  PLL_EXPORT int pll_set_asc_bias_type(pll_partition_t *partition,
                                       int              asc_bias_type);

  PLL_EXPORT void pll_set_asc_state_weights(pll_partition_t *   partition,
                                            const unsigned int *state_weights);

  /* functions in list.c */

  PLL_EXPORT int pll_dlist_append(pll_dlist_t **dlist, void *data);
  PLL_EXPORT int pll_dlist_remove(pll_dlist_t **dlist, void *data);
  PLL_EXPORT int pll_dlist_prepend(pll_dlist_t **dlist, void *data);

  PLL_EXPORT void pll_fill_parent_scaler(unsigned int        scaler_size,
                                         unsigned int *      parent_scaler,
                                         const unsigned int *left_scaler,
                                         const unsigned int *right_scaler);

  /* functions in repeats.c */

#define PLL_GET_ID(site_id, site) ((site_id) ? ((site_id)[(site)]) : (site))
#define PLL_GET_SITE(id_site, site) ((id_site) ? ((id_site)[(site)]) : (site))

  PLL_EXPORT int pll_repeats_enabled(const pll_partition_t *partition);

  PLL_EXPORT void pll_resize_repeats_lookup(pll_partition_t *partition,
                                            unsigned int     size);

  PLL_EXPORT unsigned int pll_get_sites_number(const pll_partition_t *partition,
                                               unsigned int clv_index);

  PLL_EXPORT unsigned int *pll_get_site_id(const pll_partition_t *partition,
                                           unsigned int           clv_index);

  PLL_EXPORT unsigned int *pll_get_id_site(const pll_partition_t *partition,
                                           unsigned int           clv_index);

  PLL_EXPORT unsigned int pll_get_clv_size(const pll_partition_t *partition,
                                           unsigned int           clv_index);

  PLL_EXPORT unsigned int pll_default_enable_repeats(pll_partition_t *partition,
                                                     unsigned int     left_clv,
                                                     unsigned int right_clv);

  PLL_EXPORT unsigned int pll_no_enable_repeats(pll_partition_t *partition,
                                                unsigned int     left_clv,
                                                unsigned int     right_clv);

  PLL_EXPORT void pll_default_reallocate_repeats(pll_partition_t *partition,
                                                 unsigned int     parent,
                                                 int              scaler_index,
                                                 unsigned int sites_to_alloc);

  PLL_EXPORT int pll_repeats_initialize(pll_partition_t *partition);

  PLL_EXPORT int pll_update_repeats_tips(pll_partition_t *  partition,
                                         unsigned int       tip_index,
                                         const pll_state_t *map,
                                         const char *       sequence);

  PLL_EXPORT void pll_update_repeats(pll_partition_t *      partition,
                                     const pll_operation_t *op);

  PLL_EXPORT void pll_disable_bclv(pll_partition_t *partition);

  PLL_EXPORT void
  pll_fill_parent_scaler_repeats(unsigned int        sites,
                                 unsigned int *      parent_scaler,
                                 const unsigned int *psites,
                                 const unsigned int *left_scaler,
                                 const unsigned int *lids,
                                 const unsigned int *right_scaler,
                                 const unsigned int *rids);

  PLL_EXPORT void
  pll_fill_parent_scaler_repeats_per_rate(unsigned int        sites,
                                          unsigned int        rates,
                                          unsigned int *      parent_scaler,
                                          const unsigned int *psites,
                                          const unsigned int *left_scaler,
                                          const unsigned int *lids,
                                          const unsigned int *right_scaler,
                                          const unsigned int *rids);

  /* functions in models.c */

  PLL_EXPORT unsigned int pll_subst_rate_count(unsigned int states);

  /**
   * Sets a substitution matrix for a partition.
   *
   * @param partition Partition for which the substitution matrix will be set.
   *
   * @param params_index Index of which rate matrix to use.
   *
   * @param params An array of substitution parameters. If we wanted to use the
   * following matrix
   * ```
   * *  a  b  c
   * a  *  d  e
   * b  d  *  f
   * c  e  f  *
   * ```
   * Then we would use the array
   * ```
   * double subst_params[] = {a, b, c, d, e, f}
   * ```
   *
   * @ingroup pll_partition_t
   */
  PLL_EXPORT void pll_set_subst_params(pll_partition_t *partition,
                                       unsigned int     params_index,
                                       const double *   params);

  /**
   * Sets the based distribution frequencies for a partition. This needs to be
   * done before a likelihood can be computed.
   *
   * @param partition The partition for which the frequencies will be set for.
   *
   * @param params_index The model index to set the frequencies for.
   *
   * @params frequencies The array of frequencies which will be used to compute
   * a likelihood.
   *
   * @ingroup pll_partition_t
   */
  PLL_EXPORT void pll_set_frequencies(pll_partition_t *partition,
                                      unsigned int     params_index,
                                      const double *   frequencies);

  PLL_EXPORT void pll_set_category_rates(pll_partition_t *partition,
                                         const double *   rates);

  PLL_EXPORT void pll_set_category_weights(pll_partition_t *partition,
                                           const double *   rate_weights);

  PLL_EXPORT int pll_update_eigen(pll_partition_t *partition,
                                  unsigned int     params_index);

  /**
   * Update the probability matrices of partition. 
   *
   * @param params_index Index of the parameters to use, as in rate categories.
   *
   * @param matrix_indices An array of indices into the `prob_matrices` in
   * `pll_partition_t`. These are the locations in which the matrices will be
   * stored. The best way to get these is via `pll_utree_create_operations`.
   *
   * @param branch_lengths A list of branch lengths which will be used for
   * computing the probability matrices. The best way to get these is via
   * `pll_utree_create_operations`.
   *
   * @param count The number of matrices to update.
   *
   * @ingroup pll_partition_t
   */
  PLL_EXPORT int pll_update_prob_matrices(pll_partition_t *   partition,
                                          const unsigned int *params_index,
                                          const unsigned int *matrix_indices,
                                          const double *      branch_lengths,
                                          unsigned int        count);

  PLL_EXPORT unsigned int
  pll_count_invariant_sites(pll_partition_t *partition,
                            unsigned int *   state_inv_count);

  PLL_EXPORT int pll_update_invariant_sites(pll_partition_t *partition);

  PLL_EXPORT int pll_update_invariant_sites_proportion(
      pll_partition_t *partition, unsigned int params_index, double prop_invar);

  PLL_EXPORT void *pll_aligned_alloc(size_t size, size_t alignment);

  PLL_EXPORT void pll_aligned_free(void *ptr);

  /* functions in likelihood.c */

  /**
   * Computes the likelihood given a single CLV. This is intended to be the CLV
   * of the "root" of the tree.
   *
   * @param partition The partition to compute the likelihood for.
   *
   * @param clv_index Index of the root CLV.
   *
   * @param scaler_index Index of the scalar buffer for the root CLV.
   *
   * @param freqs_indices An array of indices which indicate the per site base
   * distribution of states.
   *
   * @param[out] persite_lnl Buffer to store the individual site likelihoods.
   * Optional. Set to `nullptr` to ignore.
   *
   * @return The total likelihood of the partition.
   *
   * @ingroup pll_partition_t
   */
  PLL_EXPORT double
  pll_compute_root_loglikelihood(pll_partition_t *   partition,
                                 unsigned int        clv_index,
                                 int                 scaler_index,
                                 const unsigned int *freqs_indices,
                                 double *            persite_lnl);

  /**
   * Computes the likelihood of an edge. It does this by "rootinng" the tree at
   * parent, and computing the likelihood from there.
   *
   * @param partition The partition to compute the likelihood for.
   *
   * @param parent_clv_index Index of the parent CLV
   *
   * @param parent_scaler_index Index of the parent CLV scaler.
   *
   * @param child_clv_index Index of the child CLV
   *
   * @param child_parent_scaler_index Index of the child CLV scaler.
   *
   * @param matrix_index Index of the probability matrix between `parent` and
   * `child`.
   *
   * @param freqs_indices An array of indices which indicate the per site base
   * distribution of states.
   *
   * @param[out] persite_lnl Buffer to store the individual site likelihoods.
   * Optional. Set to `nullptr` to ignore.
   *
   * @return The total likelihood of the partition.
   *
   * @ingroup pll_partition_t
   */
  PLL_EXPORT double
  pll_compute_edge_loglikelihood(pll_partition_t *   partition,
                                 unsigned int        parent_clv_index,
                                 int                 parent_scaler_index,
                                 unsigned int        child_clv_index,
                                 int                 child_scaler_index,
                                 unsigned int        matrix_index,
                                 const unsigned int *freqs_indices,
                                 double *            persite_lnl);

  PLL_EXPORT int pll_compute_node_ancestral(pll_partition_t *partition,
                                            unsigned int     node_clv_index,
                                            int              node_scaler_index,
                                            unsigned int     other_clv_index,
                                            int              other_scaler_index,
                                            unsigned int     matrix_index,
                                            const unsigned int *freqs_indices,
                                            double *            ancestral);

  PLL_EXPORT int
  pll_compute_node_ancestral_extbuf(pll_partition_t *   partition,
                                    unsigned int        node_clv_index,
                                    int                 node_scaler_index,
                                    unsigned int        other_clv_index,
                                    int                 other_scaler_index,
                                    unsigned int        pmatrix_index,
                                    const unsigned int *freqs_indices,
                                    double *            ancestral,
                                    double *            temp_clv,
                                    unsigned int *      temp_scaler,
                                    double *            ident_pmat);

  /* functions in clvs.c */

  /**
   * Computes the CLVS for all the trees in the nodes specified in the
   * `operations` array.
   *
   * @param[in,out] partition The partition for which the operations will be
   * computed.
   *
   * @param operations The list of operations. Typically this will be generated
   * using pll_utree_create_operations
   *
   * @param count Number of elements in the operations buffer.
   *
   * @ingroup pll_partition_t
   * @ingroup pll_operation_t
   */ 
  PLL_EXPORT void pll_update_clvs(pll_partition_t *      partition,
                                  const pll_operation_t *operations,
                                  unsigned int           count);

  PLL_EXPORT void pll_update_clvs_rep(pll_partition_t *      partition,
                                      const pll_operation_t *operations,
                                      unsigned int           count,
                                      unsigned int           update_repeats);

  /* functions in derivatives.c */

  /**
   * Function which computes a "sumtable". This sumtable can be used to compute
   * the derivative of the likelihood with respect to a branch length.
   *
   * @param partition The partition for which the sumtable will be computed.
   *
   * @param parent_clv_index Parent CLV index of the edge in question.
   *
   * @param child_clv_index Child CLV index of the edge in question.
   *
   * @param params_indices A list of the indices for each rate category present
   * in the partition.
   *
   * @param[out] sumtable Buffer for the resulting sumtable. Should be allocated
   * with `rates * states_padded` elements.
   *
   * @ingroup pll_partition_t
   */
  PLL_EXPORT int pll_update_sumtable(pll_partition_t *   partition,
                                     unsigned int        parent_clv_index,
                                     unsigned int        child_clv_index,
                                     int                 parent_scaler_index,
                                     int                 child_scaler_index,
                                     const unsigned int *params_indices,
                                     double *            sumtable);

  /**
   * Computes the first and second derivative with respect to a specific branch
   * length.
   *
   * @param partition Partition that the derivative is computed for.
   *
   * @param parent_scaler_index Scaler index for the parent of the edge in
   * question.
   *
   * @param child_scaler_index Scaler index for the child of the edge in
   * question.
   *
   * @param branch_length Value at which to evaluate the derivative at.
   *
   * @param sumtable Sumbtable from `pll_udate_sumtable`.
   *
   * @param[out] d_f Buffer to store the first derivative. Only a single double.
   *
   * @param[out] dd_f Buffer to store the second derivative. Only a single
   * double.
   *
   * @ingroup pll_partition_t
   */
  PLL_EXPORT int
  pll_compute_likelihood_derivatives(pll_partition_t *   partition,
                                     int                 parent_scaler_index,
                                     int                 child_scaler_index,
                                     double              branch_length,
                                     const unsigned int *params_indices,
                                     const double *      sumtable,
                                     double *            d_f,
                                     double *            dd_f);

  /* functions in gamma.c */

  PLL_EXPORT int pll_compute_gamma_cats(double       alpha,
                                        unsigned int categories,
                                        double *     output_rates,
                                        int          rates_mode);

  /* functions in output.c */

  PLL_EXPORT void pll_show_pmatrix(const pll_partition_t *partition,
                                   unsigned int           index,
                                   unsigned int           float_precision);

  PLL_EXPORT void pll_show_clv(const pll_partition_t *partition,
                               unsigned int           clv_index,
                               int                    scaler_index,
                               unsigned int           float_precision);

  /* functions in fasta.c */

  PLL_EXPORT pll_fasta_t *pll_fasta_open(const char *        filename,
                                         const unsigned int *map);

  PLL_EXPORT int pll_fasta_getnext(pll_fasta_t *fd,
                                   char **      head,
                                   long *       head_len,
                                   char **      seq,
                                   long *       seq_len,
                                   long *       seqno);

  PLL_EXPORT void pll_fasta_close(pll_fasta_t *fd);

  PLL_EXPORT long pll_fasta_getfilesize(const pll_fasta_t *fd);

  PLL_EXPORT long pll_fasta_getfilepos(pll_fasta_t *fd);

  PLL_EXPORT int pll_fasta_rewind(pll_fasta_t *fd);

  pll_msa_t *pll_fasta_load(const char *fname);

  /* functions in phylip.c */

  PLL_EXPORT void pll_msa_destroy(pll_msa_t *msa);

  PLL_EXPORT pll_phylip_t *pll_phylip_open(const char *        filename,
                                           const unsigned int *map);

  PLL_EXPORT int pll_phylip_rewind(pll_phylip_t *fd);

  PLL_EXPORT void pll_phylip_close(pll_phylip_t *fd);

  PLL_EXPORT pll_msa_t *pll_phylip_parse_interleaved(pll_phylip_t *fd);

  PLL_EXPORT pll_msa_t *pll_phylip_parse_sequential(pll_phylip_t *fd);

  PLL_EXPORT pll_msa_t *pll_phylip_load(const char *fname,
                                        pll_bool_t  interleaved);

  PLL_EXPORT int pll_phylip_save(const char *out_fname, const pll_msa_t *msa);

  /* functions in core_clvs.c */

  PLL_EXPORT void pll_core_create_lookup(unsigned int       states,
                                         unsigned int       rate_cats,
                                         double *           lookup,
                                         const double *     left_matrix,
                                         const double *     right_matrix,
                                         const pll_state_t *tipmap,
                                         unsigned int       tipmap_size,
                                         unsigned int       attrib);

  PLL_EXPORT void pll_core_update_clv_tt(unsigned int         states,
                                         unsigned int         sites,
                                         unsigned int         rate_cats,
                                         double *             parent_clv,
                                         unsigned int *       parent_scaler,
                                         const unsigned char *left_tipchars,
                                         const unsigned char *right_tipchars,
                                         const pll_state_t *  tipmap,
                                         unsigned int         tipmap_size,
                                         const double *       lookup,
                                         unsigned int         attrib);

  PLL_EXPORT void pll_core_update_clv_ti(unsigned int         states,
                                         unsigned int         sites,
                                         unsigned int         rate_cats,
                                         double *             parent_clv,
                                         unsigned int *       parent_scaler,
                                         const unsigned char *left_tipchars,
                                         const double *       right_clv,
                                         const double *       left_matrix,
                                         const double *       right_matrix,
                                         const unsigned int * right_scaler,
                                         const pll_state_t *  tipmap,
                                         unsigned int         tipmap_size,
                                         unsigned int         attrib);

  PLL_EXPORT void pll_core_update_clv_ii(unsigned int        states,
                                         unsigned int        sites,
                                         unsigned int        rate_cats,
                                         double *            parent_clv,
                                         unsigned int *      parent_scaler,
                                         const double *      left_clv,
                                         const double *      right_clv,
                                         const double *      left_matrix,
                                         const double *      right_matrix,
                                         const unsigned int *left_scaler,
                                         const unsigned int *right_scaler,
                                         unsigned int        attrib);

  PLL_EXPORT void
  pll_core_update_clv_repeats(unsigned int        states,
                              unsigned int        parent_sites,
                              unsigned int        left_sites,
                              unsigned int        right_sites,
                              unsigned int        rate_cats,
                              double *            parent_clv,
                              unsigned int *      parent_scaler,
                              const double *      left_clv,
                              const double *      right_clv,
                              const double *      left_matrix,
                              const double *      right_matrix,
                              const unsigned int *left_scaler,
                              const unsigned int *right_scaler,
                              const unsigned int *parent_id_site,
                              const unsigned int *left_site_id,
                              const unsigned int *right_site_id,
                              double *            bclv_buffer,
                              unsigned int        attrib);

  PLL_EXPORT void
  pll_core_update_clv_repeats_generic(unsigned int        states,
                                      unsigned int        parent_sites,
                                      unsigned int        left_sites,
                                      unsigned int        right_sites,
                                      unsigned int        rate_cats,
                                      double *            parent_clv,
                                      unsigned int *      parent_scaler,
                                      const double *      left_clv,
                                      const double *      right_clv,
                                      const double *      left_matrix,
                                      const double *      right_matrix,
                                      const unsigned int *left_scaler,
                                      const unsigned int *right_scaler,
                                      const unsigned int *parent_id_site,
                                      const unsigned int *left_site_id,
                                      const unsigned int *right_site_id,
                                      double *            bclv_buffer,
                                      unsigned int        attrib);

  PLL_EXPORT void
  pll_core_update_clv_repeatsbclv_generic(unsigned int        states,
                                          unsigned int        parent_sites,
                                          unsigned int        left_sites,
                                          unsigned int        right_sites,
                                          unsigned int        rate_cats,
                                          double *            parent_clv,
                                          unsigned int *      parent_scaler,
                                          const double *      left_clv,
                                          const double *      right_clv,
                                          const double *      left_matrix,
                                          const double *      right_matrix,
                                          const unsigned int *left_scaler,
                                          const unsigned int *right_scaler,
                                          const unsigned int *parent_id_site,
                                          const unsigned int *left_site_id,
                                          const unsigned int *right_site_id,
                                          double *            bclv_buffer,
                                          unsigned int        attrib);

  PLL_EXPORT void pll_core_create_lookup_4x4(unsigned int  rate_cats,
                                             double *      lookup,
                                             const double *left_matrix,
                                             const double *right_matrix);

  PLL_EXPORT void
  pll_core_update_clv_tt_4x4(unsigned int         sites,
                             unsigned int         rate_cats,
                             double *             parent_clv,
                             unsigned int *       parent_scaler,
                             const unsigned char *left_tipchars,
                             const unsigned char *right_tipchars,
                             const double *       lookup,
                             unsigned int         attrib);

  PLL_EXPORT void pll_core_update_clv_ti_4x4(unsigned int         sites,
                                             unsigned int         rate_cats,
                                             double *             parent_clv,
                                             unsigned int *       parent_scaler,
                                             const unsigned char *left_tipchars,
                                             const double *       right_clv,
                                             const double *       left_matrix,
                                             const double *       right_matrix,
                                             const unsigned int * right_scaler,
                                             unsigned int         attrib);

  /* functions in core_derivatives.c */

  PLL_EXPORT int
  pll_core_update_sumtable_repeats(unsigned int        states,
                                   unsigned int        sites,
                                   unsigned int        parent_sites,
                                   unsigned int        rate_cats,
                                   const double *      clvp,
                                   const double *      clvc,
                                   const unsigned int *parent_scaler,
                                   const unsigned int *child_scaler,
                                   double *const *     eigenvecs,
                                   double *const *     inv_eigenvecs,
                                   double *const *     freqs,
                                   double *            sumtable,
                                   const unsigned int *parent_site_id,
                                   const unsigned int *child_site_id,
                                   double *            bclv_buffer,
                                   unsigned int        inv,
                                   unsigned int        attrib);

  PLL_EXPORT int
  pll_core_update_sumtable_repeats_generic(unsigned int        states,
                                           unsigned int        sites,
                                           unsigned int        parent_sites,
                                           unsigned int        rate_cats,
                                           const double *      clvp,
                                           const double *      clvc,
                                           const unsigned int *parent_scaler,
                                           const unsigned int *child_scaler,
                                           double *const *     eigenvecs,
                                           double *const *     inv_eigenvecs,
                                           double *const *     freqs,
                                           double *            sumtable,
                                           const unsigned int *parent_site_id,
                                           const unsigned int *child_site_id,
                                           double *            bclv_buffer,
                                           unsigned int        inv,
                                           unsigned int        attrib);
  PLL_EXPORT int
  pll_core_update_sumtable_ti_4x4(unsigned int         sites,
                                  unsigned int         rate_cats,
                                  const double *       parent_clv,
                                  const unsigned char *left_tipchars,
                                  const unsigned int * parent_scaler,
                                  double *const *      eigenvecs,
                                  double *const *      inv_eigenvecs,
                                  double *const *      freqs,
                                  double *             sumtable,
                                  unsigned int         attrib);

  PLL_EXPORT int pll_core_update_sumtable_ii(unsigned int        states,
                                             unsigned int        sites,
                                             unsigned int        rate_cats,
                                             const double *      parent_clv,
                                             const double *      child_clv,
                                             const unsigned int *parent_scaler,
                                             const unsigned int *child_scaler,
                                             double *const *     eigenvecs,
                                             double *const *     inv_eigenvecs,
                                             double *const *     freqs,
                                             double *            sumtable,
                                             unsigned int        attrib);

  PLL_EXPORT int pll_core_update_sumtable_ti(unsigned int         states,
                                             unsigned int         sites,
                                             unsigned int         rate_cats,
                                             const double *       parent_clv,
                                             const unsigned char *left_tipchars,
                                             const unsigned int * parent_scaler,
                                             double *const *      eigenvecs,
                                             double *const *      inv_eigenvecs,
                                             double *const *      freqs,
                                             const pll_state_t *  tipmap,
                                             unsigned int         tipmap_size,
                                             double *             sumtable,
                                             unsigned int         attrib);

  PLL_EXPORT int
  pll_core_likelihood_derivatives(unsigned int        states,
                                  unsigned int        sites,
                                  unsigned int        rate_cats,
                                  const double *      rate_weights,
                                  const unsigned int *parent_scaler,
                                  const unsigned int *child_scaler,
                                  unsigned int        parent_ids,
                                  unsigned int        child_ids,
                                  const int *         invariant,
                                  const unsigned int *pattern_weights,
                                  double              branch_length,
                                  const double *      prop_invar,
                                  double *const *     freqs,
                                  const double *      rates,
                                  double *const *     eigenvals,
                                  const double *      sumtable,
                                  double *            d_f,
                                  double *            dd_f,
                                  unsigned int        attrib);

  PLL_EXPORT int
  pll_core_update_sumtable_repeats_avx(unsigned int        states,
                                       unsigned int        sites,
                                       unsigned int        parent_sites,
                                       unsigned int        rate_cats,
                                       const double *      clvp,
                                       const double *      clvc,
                                       const unsigned int *parent_scaler,
                                       const unsigned int *child_scaler,
                                       double *const *     eigenvecs,
                                       double *const *     inv_eigenvecs,
                                       double *const *     freqs,
                                       double *            sumtable,
                                       const unsigned int *parent_site_id,
                                       const unsigned int *child_site_id,
                                       double *            bclv_buffer,
                                       unsigned int        inv,
                                       unsigned int        attrib);

  /* functions in core_likelihood.c */

  PLL_EXPORT double
  pll_core_edge_loglikelihood_ii(unsigned int        states,
                                 unsigned int        sites,
                                 unsigned int        rate_cats,
                                 const double *      parent_clv,
                                 const unsigned int *parent_scaler,
                                 const double *      child_clv,
                                 const unsigned int *child_scaler,
                                 const double *      pmatrix,
                                 double *const *     frequencies,
                                 const double *      rate_weights,
                                 const unsigned int *pattern_weights,
                                 const double *      invar_proportion,
                                 const int *         invar_indices,
                                 const unsigned int *freqs_indices,
                                 double *            persite_lnl,
                                 unsigned int        attrib);

  PLL_EXPORT double
  pll_core_edge_loglikelihood_ti(unsigned int         states,
                                 unsigned int         sites,
                                 unsigned int         rate_cats,
                                 const double *       parent_clv,
                                 const unsigned int * parent_scaler,
                                 const unsigned char *tipchars,
                                 const pll_state_t *  tipmap,
                                 unsigned int         tipmap_size,
                                 const double *       pmatrix,
                                 double *const *      frequencies,
                                 const double *       rate_weights,
                                 const unsigned int * pattern_weights,
                                 const double *       invar_proportion,
                                 const int *          invar_indices,
                                 const unsigned int * freqs_indices,
                                 double *             persite_lnl,
                                 unsigned int         attrib);

  PLL_EXPORT double
  pll_core_edge_loglikelihood_ti_4x4(unsigned int         sites,
                                     unsigned int         rate_cats,
                                     const double *       parent_clv,
                                     const unsigned int * parent_scaler,
                                     const unsigned char *tipchars,
                                     const double *       pmatrix,
                                     double *const *      frequencies,
                                     const double *       rate_weights,
                                     const unsigned int * pattern_weights,
                                     const double *       invar_proportion,
                                     const int *          invar_indices,
                                     const unsigned int * freqs_indices,
                                     double *             persite_lnl,
                                     unsigned int         attrib);

  PLL_EXPORT double
  pll_core_root_loglikelihood_repeats(unsigned int        states,
                                      unsigned int        sites,
                                      unsigned int        rate_cats,
                                      const double *      clv,
                                      const unsigned int *site_id,
                                      const unsigned int *scaler,
                                      double *const *     frequencies,
                                      const double *      rate_weights,
                                      const unsigned int *pattern_weights,
                                      const double *      invar_proportion,
                                      const int *         invar_indices,
                                      const unsigned int *freqs_indices,
                                      double *            persite_lnl,
                                      unsigned int        attrib);

  PLL_EXPORT double
  pll_core_edge_loglikelihood_repeats(unsigned int        states,
                                      unsigned int        sites,
                                      const unsigned int  child_sites,
                                      unsigned int        rate_cats,
                                      const double *      parent_clv,
                                      const unsigned int *parent_scaler,
                                      const double *      child_clv,
                                      const unsigned int *child_scaler,
                                      const double *      pmatrix,
                                      double **           frequencies,
                                      const double *      rate_weights,
                                      const unsigned int *pattern_weights,
                                      const double *      invar_proportion,
                                      const int *         invar_indices,
                                      const unsigned int *freqs_indices,
                                      double *            persite_lnl,
                                      const unsigned int *parent_site_id,
                                      const unsigned int *child_site_id,
                                      double *            bclv,
                                      unsigned int        attrib);

  PLL_EXPORT double pll_core_edge_loglikelihood_repeats_generic(
      unsigned int        states,
      unsigned int        sites,
      const unsigned int  child_sites,
      unsigned int        rate_cats,
      const double *      parent_clv,
      const unsigned int *parent_scaler,
      const double *      child_clv,
      const unsigned int *child_scaler,
      const double *      pmatrix,
      double **           frequencies,
      const double *      rate_weights,
      const unsigned int *pattern_weights,
      const double *      invar_proportion,
      const int *         invar_indices,
      const unsigned int *freqs_indices,
      double *            persite_lnl,
      const unsigned int *parent_site_id,
      const unsigned int *child_site_id,
      double *            bclv,
      unsigned int        attrib);

  PLL_EXPORT double
  pll_core_root_loglikelihood(unsigned int        states,
                              unsigned int        sites,
                              unsigned int        rate_cats,
                              const double *      clv,
                              const unsigned int *scaler,
                              double *const *     frequencies,
                              const double *      rate_weights,
                              const unsigned int *pattern_weights,
                              const double *      invar_proportion,
                              const int *         invar_indices,
                              const unsigned int *freqs_indices,
                              double *            persite_lnl,
                              unsigned int        attrib);

  /* functions in core_clvs_sse.c */

#ifdef HAVE_SSE3
  PLL_EXPORT void pll_core_create_lookup_sse(unsigned int       states,
                                             unsigned int       rate_cats,
                                             double *           ttlookup,
                                             const double *     left_matrix,
                                             const double *     right_matrix,
                                             const pll_state_t *tipmap,
                                             unsigned int       tipmap_size);

  PLL_EXPORT void pll_core_create_lookup_4x4_sse(unsigned int  rate_cats,
                                                 double *      lookup,
                                                 const double *left_matrix,
                                                 const double *right_matrix);

  PLL_EXPORT void
  pll_core_update_clv_tt_sse(unsigned int         states,
                             unsigned int         sites,
                             unsigned int         rate_cats,
                             double *             parent_clv,
                             unsigned int *       parent_scaler,
                             const unsigned char *left_tipchars,
                             const unsigned char *right_tipchars,
                             const double *       lookup,
                             unsigned int         tipstates_count,
                             unsigned int         attrib);

  PLL_EXPORT void
  pll_core_update_clv_tt_4x4_sse(unsigned int         sites,
                                 unsigned int         rate_cats,
                                 double *             parent_clv,
                                 unsigned int *       parent_scaler,
                                 const unsigned char *left_tipchars,
                                 const unsigned char *right_tipchars,
                                 const double *       lookup,
                                 unsigned int         attrib);

  PLL_EXPORT void pll_core_update_clv_ti_sse(unsigned int         states,
                                             unsigned int         sites,
                                             unsigned int         rate_cats,
                                             double *             parent_clv,
                                             unsigned int *       parent_scaler,
                                             const unsigned char *left_tipchars,
                                             const double *       right_clv,
                                             const double *       left_matrix,
                                             const double *       right_matrix,
                                             const unsigned int * right_scaler,
                                             const pll_state_t *  tipmap,
                                             unsigned int         tipmap_size,
                                             unsigned int         attrib);

  PLL_EXPORT void
  pll_core_update_clv_ti_4x4_sse(unsigned int         sites,
                                 unsigned int         rate_cats,
                                 double *             parent_clv,
                                 unsigned int *       parent_scaler,
                                 const unsigned char *left_tipchar,
                                 const double *       right_clv,
                                 const double *       left_matrix,
                                 const double *       right_matrix,
                                 const unsigned int * right_scaler,
                                 unsigned int         attrib);

  PLL_EXPORT void pll_core_update_clv_ii_sse(unsigned int        states,
                                             unsigned int        sites,
                                             unsigned int        rate_cats,
                                             double *            parent_clv,
                                             unsigned int *      parent_scaler,
                                             const double *      left_clv,
                                             const double *      right_clv,
                                             const double *      left_matrix,
                                             const double *      right_matrix,
                                             const unsigned int *left_scaler,
                                             const unsigned int *right_scaler,
                                             unsigned int        attrib);

  PLL_EXPORT void
  pll_core_update_clv_ii_4x4_sse(unsigned int        sites,
                                 unsigned int        rate_cats,
                                 double *            parent_clv,
                                 unsigned int *      parent_scaler,
                                 const double *      left_clv,
                                 const double *      right_clv,
                                 const double *      left_matrix,
                                 const double *      right_matrix,
                                 const unsigned int *left_scaler,
                                 const unsigned int *right_scaler,
                                 unsigned int        attrib);

  PLL_EXPORT void
  pll_core_update_clv_repeats_generic_sse(unsigned int        states,
                                          unsigned int        parent_sites,
                                          unsigned int        left_sites,
                                          unsigned int        right_sites,
                                          unsigned int        rate_cats,
                                          double *            parent_clv,
                                          unsigned int *      parent_scaler,
                                          const double *      left_clv,
                                          const double *      right_clv,
                                          const double *      left_matrix,
                                          const double *      right_matrix,
                                          const unsigned int *left_scaler,
                                          const unsigned int *right_scaler,
                                          const unsigned int *parent_id_site,
                                          const unsigned int *left_site_id,
                                          const unsigned int *right_site_id,
                                          double *            bclv_buffer,
                                          unsigned int        attrib);
#endif

  /* functions in core_clvs_avx.c */

#ifdef HAVE_AVX
  PLL_EXPORT void pll_core_create_lookup_avx(unsigned int       states,
                                             unsigned int       rate_cats,
                                             double *           lookup,
                                             const double *     left_matrix,
                                             const double *     right_matrix,
                                             const pll_state_t *tipmap,
                                             unsigned int       tipmap_size);

  PLL_EXPORT void pll_core_create_lookup_4x4_avx(unsigned int  rate_cats,
                                                 double *      lookup,
                                                 const double *left_matrix,
                                                 const double *right_matrix);

  PLL_EXPORT void pll_core_create_lookup_20x20_avx(unsigned int  rate_cats,
                                                   double *      ttlookup,
                                                   const double *left_matrix,
                                                   const double *right_matrix,
                                                   const pll_state_t *tipmap,
                                                   unsigned int tipmap_size);

  PLL_EXPORT void
  pll_core_update_clv_tt_avx(unsigned int         states,
                             unsigned int         sites,
                             unsigned int         rate_cats,
                             double *             parent_clv,
                             unsigned int *       parent_scaler,
                             const unsigned char *left_tipchars,
                             const unsigned char *right_tipchars,
                             const double *       lookup,
                             unsigned int         tipstates_count,
                             unsigned int         attrib);

  PLL_EXPORT void
  pll_core_update_clv_tt_4x4_avx(unsigned int         sites,
                                 unsigned int         rate_cats,
                                 double *             parent_clv,
                                 unsigned int *       parent_scaler,
                                 const unsigned char *left_tipchars,
                                 const unsigned char *right_tipchars,
                                 const double *       lookup,
                                 unsigned int         attrib);

  PLL_EXPORT void pll_core_update_clv_ti_avx(unsigned int         states,
                                             unsigned int         sites,
                                             unsigned int         rate_cats,
                                             double *             parent_clv,
                                             unsigned int *       parent_scaler,
                                             const unsigned char *left_tipchars,
                                             const double *       right_clv,
                                             const double *       left_matrix,
                                             const double *       right_matrix,
                                             const unsigned int * right_scaler,
                                             const pll_state_t *  tipmap,
                                             unsigned int         tipmap_size,
                                             unsigned int         attrib);

  PLL_EXPORT void
  pll_core_update_clv_ti_4x4_avx(unsigned int         sites,
                                 unsigned int         rate_cats,
                                 double *             parent_clv,
                                 unsigned int *       parent_scaler,
                                 const unsigned char *left_tipchar,
                                 const double *       right_clv,
                                 const double *       left_matrix,
                                 const double *       right_matrix,
                                 const unsigned int * right_scaler,
                                 unsigned int         attrib);

  PLL_EXPORT void
  pll_core_update_clv_ti_20x20_avx(unsigned int         sites,
                                   unsigned int         rate_cats,
                                   double *             parent_clv,
                                   unsigned int *       parent_scaler,
                                   const unsigned char *left_tipchar,
                                   const double *       right_clv,
                                   const double *       left_matrix,
                                   const double *       right_matrix,
                                   const unsigned int * right_scaler,
                                   const pll_state_t *  tipmap,
                                   unsigned int         tipmap_size,
                                   unsigned int         attrib);

  PLL_EXPORT void pll_core_update_clv_ii_avx(unsigned int        states,
                                             unsigned int        sites,
                                             unsigned int        rate_cats,
                                             double *            parent_clv,
                                             unsigned int *      parent_scaler,
                                             const double *      left_clv,
                                             const double *      right_clv,
                                             const double *      left_matrix,
                                             const double *      right_matrix,
                                             const unsigned int *left_scaler,
                                             const unsigned int *right_scaler,
                                             unsigned int        attrib);

  PLL_EXPORT void
  pll_core_update_clv_ii_4x4_avx(unsigned int        sites,
                                 unsigned int        rate_cats,
                                 double *            parent_clv,
                                 unsigned int *      parent_scaler,
                                 const double *      left_clv,
                                 const double *      right_clv,
                                 const double *      left_matrix,
                                 const double *      right_matrix,
                                 const unsigned int *left_scaler,
                                 const unsigned int *right_scaler,
                                 unsigned int        attrib);

  PLL_EXPORT void
  pll_core_update_clv_repeats_generic_avx(unsigned int        states,
                                          unsigned int        parent_sites,
                                          unsigned int        left_sites,
                                          unsigned int        right_sites,
                                          unsigned int        rate_cats,
                                          double *            parent_clv,
                                          unsigned int *      parent_scaler,
                                          const double *      left_clv,
                                          const double *      right_clv,
                                          const double *      left_matrix,
                                          const double *      right_matrix,
                                          const unsigned int *left_scaler,
                                          const unsigned int *right_scaler,
                                          const unsigned int *parent_id_site,
                                          const unsigned int *left_site_id,
                                          const unsigned int *right_site_id,
                                          double *            bclv_buffer,
                                          unsigned int        attrib);

  PLL_EXPORT void
  pll_core_update_clv_repeats_4x4_avx(unsigned int        states,
                                      unsigned int        parent_sites,
                                      unsigned int        left_sites,
                                      unsigned int        right_sites,
                                      unsigned int        rate_cats,
                                      double *            parent_clv,
                                      unsigned int *      parent_scaler,
                                      const double *      left_clv,
                                      const double *      right_clv,
                                      const double *      left_matrix,
                                      const double *      right_matrix,
                                      const unsigned int *left_scaler,
                                      const unsigned int *right_scaler,
                                      const unsigned int *parent_id_site,
                                      const unsigned int *left_site_id,
                                      const unsigned int *right_site_id,
                                      double *            bclv_buffer,
                                      unsigned int        attrib);

  PLL_EXPORT void
  pll_core_update_clv_repeatsbclv_4x4_avx(unsigned int        states,
                                          unsigned int        parent_sites,
                                          unsigned int        left_sites,
                                          unsigned int        right_sites,
                                          unsigned int        rate_cats,
                                          double *            parent_clv,
                                          unsigned int *      parent_scaler,
                                          const double *      left_clv,
                                          const double *      right_clv,
                                          const double *      left_matrix,
                                          const double *      right_matrix,
                                          const unsigned int *left_scaler,
                                          const unsigned int *right_scaler,
                                          const unsigned int *parent_id_site,
                                          const unsigned int *left_site_id,
                                          const unsigned int *right_site_id,
                                          double *            bclv_buffer,
                                          unsigned int        attrib);

  PLL_EXPORT void pll_core_update_clv_repeatsbclv_generic_avx(
      unsigned int        states,
      unsigned int        parent_sites,
      unsigned int        left_sites,
      unsigned int        right_sites,
      unsigned int        rate_cats,
      double *            parent_clv,
      unsigned int *      parent_scaler,
      const double *      left_clv,
      const double *      right_clv,
      const double *      left_matrix,
      const double *      right_matrix,
      const unsigned int *left_scaler,
      const unsigned int *right_scaler,
      const unsigned int *parent_id_site,
      const unsigned int *left_site_id,
      const unsigned int *right_site_id,
      double *            bclv_buffer,
      unsigned int        attrib);
#endif

  /* functions in core_clvs_avx2.c */

#ifdef HAVE_AVX2
  PLL_EXPORT void
  pll_core_update_clv_ti_avx2(unsigned int         states,
                              unsigned int         sites,
                              unsigned int         rate_cats,
                              double *             parent_clv,
                              unsigned int *       parent_scaler,
                              const unsigned char *left_tipchars,
                              const double *       right_clv,
                              const double *       left_matrix,
                              const double *       right_matrix,
                              const unsigned int * right_scaler,
                              const pll_state_t *  tipmap,
                              unsigned int         tipmap_size,
                              unsigned int         attrib);

  PLL_EXPORT
  void pll_core_update_clv_ti_20x20_avx2(unsigned int         sites,
                                         unsigned int         rate_cats,
                                         double *             parent_clv,
                                         unsigned int *       parent_scaler,
                                         const unsigned char *left_tipchar,
                                         const double *       right_clv,
                                         const double *       left_matrix,
                                         const double *       right_matrix,
                                         const unsigned int * right_scaler,
                                         const pll_state_t *  tipmap,
                                         unsigned int         tipmap_size,
                                         unsigned int         attrib);

  PLL_EXPORT void pll_core_update_clv_ii_avx2(unsigned int        states,
                                              unsigned int        sites,
                                              unsigned int        rate_cats,
                                              double *            parent_clv,
                                              unsigned int *      parent_scaler,
                                              const double *      left_clv,
                                              const double *      right_clv,
                                              const double *      left_matrix,
                                              const double *      right_matrix,
                                              const unsigned int *left_scaler,
                                              const unsigned int *right_scaler,
                                              unsigned int        attrib);

  PLL_EXPORT void
  pll_core_update_clv_repeats_generic_avx2(unsigned int        states,
                                           unsigned int        parent_sites,
                                           unsigned int        left_sites,
                                           unsigned int        right_sites,
                                           unsigned int        rate_cats,
                                           double *            parent_clv,
                                           unsigned int *      parent_scaler,
                                           const double *      left_clv,
                                           const double *      right_clv,
                                           const double *      left_matrix,
                                           const double *      right_matrix,
                                           const unsigned int *left_scaler,
                                           const unsigned int *right_scaler,
                                           const unsigned int *parent_id_site,
                                           const unsigned int *left_site_id,
                                           const unsigned int *right_site_id,
                                           double *            bclv_buffer,
                                           unsigned int        attrib);
#endif

  /* functions in core_derivatives_sse.c */

#ifdef HAVE_SSE3
  PLL_EXPORT int
  pll_core_update_sumtable_ii_sse(unsigned int        states,
                                  unsigned int        sites,
                                  unsigned int        rate_cats,
                                  const double *      parent_clv,
                                  const double *      child_clv,
                                  const unsigned int *parent_scaler,
                                  const unsigned int *child_scaler,
                                  double *const *     eigenvecs,
                                  double *const *     inv_eigenvecs,
                                  double *const *     freqs,
                                  double *            sumtable,
                                  unsigned int        attrib);

  PLL_EXPORT int
  pll_core_update_sumtable_ti_sse(unsigned int         states,
                                  unsigned int         sites,
                                  unsigned int         rate_cats,
                                  const double *       parent_clv,
                                  const unsigned char *left_tipchars,
                                  const unsigned int * parent_scaler,
                                  double *const *      eigenvecs,
                                  double *const *      inv_eigenvecs,
                                  double *const *      freqs,
                                  const pll_state_t *  tipmap,
                                  double *             sumtable,
                                  unsigned int         attrib);

  PLL_EXPORT int pll_core_update_sumtable_repeats_generic_sse(
      unsigned int        states,
      unsigned int        sites,
      unsigned int        parent_sites,
      unsigned int        rate_cats,
      const double *      clvp,
      const double *      clvc,
      const unsigned int *parent_scaler,
      const unsigned int *child_scaler,
      double *const *     eigenvecs,
      double *const *     inv_eigenvecs,
      double *const *     freqs,
      double *            sumtable,
      const unsigned int *parent_site_id,
      const unsigned int *child_site_id,
      double *            bclv_buffer,
      unsigned int        inv,
      unsigned int        attrib);
#endif

  /* functions in core_derivatives_avx.c */

#ifdef HAVE_AVX

  PLL_EXPORT int
  pll_core_update_sumtable_ii_avx(unsigned int        states,
                                  unsigned int        sites,
                                  unsigned int        rate_cats,
                                  const double *      clvp,
                                  const double *      clvc,
                                  const unsigned int *parent_scaler,
                                  const unsigned int *child_scaler,
                                  double *const *     eigenvecs,
                                  double *const *     inv_eigenvecs,
                                  double *const *     freqs,
                                  double *            sumtable,
                                  unsigned int        attrib);

  PLL_EXPORT int
  pll_core_update_sumtable_ti_avx(unsigned int         states,
                                  unsigned int         sites,
                                  unsigned int         rate_cats,
                                  const double *       parent_clv,
                                  const unsigned char *left_tipchars,
                                  const unsigned int * parent_scaler,
                                  double *const *      eigenvecs,
                                  double *const *      inv_eigenvecs,
                                  double *const *      freqs,
                                  const pll_state_t *  tipmap,
                                  unsigned int         tipmap_size,
                                  double *             sumtable,
                                  unsigned int         attrib);

  PLL_EXPORT int
  pll_core_likelihood_derivatives_avx(unsigned int        states,
                                      unsigned int        states_padded,
                                      unsigned int        rate_cats,
                                      unsigned int        ef_sites,
                                      const unsigned int *pattern_weights,
                                      const double *      rate_weights,
                                      const int *         invariant,
                                      const double *      prop_invar,
                                      double *const *     freqs,
                                      const double *      sumtable,
                                      const double *      diagptable,
                                      double *            d_f,
                                      double *            dd_f);

  PLL_EXPORT int pll_core_update_sumtable_repeats_generic_avx(
      unsigned int        states,
      unsigned int        sites,
      unsigned int        parent_sites,
      unsigned int        rate_cats,
      const double *      clvp,
      const double *      clvc,
      const unsigned int *parent_scaler,
      const unsigned int *child_scaler,
      double *const *     eigenvecs,
      double *const *     inv_eigenvecs,
      double *const *     freqs,
      double *            sumtable,
      const unsigned int *parent_site_id,
      const unsigned int *child_site_id,
      double *            bclv_buffer,
      unsigned int        inv,
      unsigned int        attrib);
  PLL_EXPORT int
                 pll_core_update_sumtable_repeats_4x4_avx(unsigned int        states,
                                                          unsigned int        sites,
                                                          unsigned int        parent_sites,
                                                          unsigned int        rate_cats,
                                                          const double *      clvp,
                                                          const double *      clvc,
                                                          const unsigned int *parent_scaler,
                                                          const unsigned int *child_scaler,
                                                          double *const *     eigenvecs,
                                                          double *const *     inv_eigenvecs,
                                                          double *const *     freqs,
                                                          double *            sumtable,
                                                          const unsigned int *parent_site_id,
                                                          const unsigned int *child_site_id,
                                                          double *            bclv_buffer,
                                                          unsigned int        inv,
                                                          unsigned int        attrib);
  PLL_EXPORT int pll_core_update_sumtable_repeatsbclv_4x4_avx(
      unsigned int        states,
      unsigned int        sites,
      unsigned int        parent_sites,
      unsigned int        rate_cats,
      const double *      clvp,
      const double *      clvc,
      const unsigned int *parent_scaler,
      const unsigned int *child_scaler,
      double *const *     eigenvecs,
      double *const *     inv_eigenvecs,
      double *const *     freqs,
      double *            sumtable,
      const unsigned int *parent_site_id,
      const unsigned int *child_site_id,
      double *            bclv_buffer,
      unsigned int        inv,
      unsigned int        attrib);
#endif

  /* functions in core_derivatives_avx2.c */

#ifdef HAVE_AVX2

  PLL_EXPORT int
  pll_core_update_sumtable_ii_avx2(unsigned int        states,
                                   unsigned int        sites,
                                   unsigned int        rate_cats,
                                   const double *      clvp,
                                   const double *      clvc,
                                   const unsigned int *parent_scaler,
                                   const unsigned int *child_scaler,
                                   double *const *     eigenvecs,
                                   double *const *     inv_eigenvecs,
                                   double *const *     freqs,
                                   double *            sumtable,
                                   unsigned int        attrib);

  PLL_EXPORT int
  pll_core_update_sumtable_ti_avx2(unsigned int         states,
                                   unsigned int         sites,
                                   unsigned int         rate_cats,
                                   const double *       parent_clv,
                                   const unsigned char *left_tipchars,
                                   const unsigned int * parent_scaler,
                                   double *const *      eigenvecs,
                                   double *const *      inv_eigenvecs,
                                   double *const *      freqs,
                                   const pll_state_t *  tipmap,
                                   unsigned int         tipmap_size,
                                   double *             sumtable,
                                   unsigned int         attrib);

  PLL_EXPORT
  int pll_core_likelihood_derivatives_avx2(unsigned int        states,
                                           unsigned int        states_padded,
                                           unsigned int        rate_cats,
                                           unsigned int        ef_sites,
                                           const unsigned int *pattern_weights,
                                           const double *      rate_weights,
                                           const int *         invariant,
                                           const double *      prop_invar,
                                           double *const *     freqs,
                                           const double *      sumtable,
                                           const double *      diagptable,
                                           double *            d_f,
                                           double *            dd_f);

  PLL_EXPORT int pll_core_update_sumtable_repeats_generic_avx2(
      unsigned int        states,
      unsigned int        sites,
      unsigned int        parent_sites,
      unsigned int        rate_cats,
      const double *      clvp,
      const double *      clvc,
      const unsigned int *parent_scaler,
      const unsigned int *child_scaler,
      double *const *     eigenvecs,
      double *const *     inv_eigenvecs,
      double *const *     freqs,
      double *            sumtable,
      const unsigned int *parent_site_id,
      const unsigned int *child_site_id,
      double *            bclv_buffer,
      unsigned int        inv,
      unsigned int        attrib);
#endif

  /* functions in core_likelihood_sse.c */

#ifdef HAVE_SSE3
  PLL_EXPORT
  double pll_core_edge_loglikelihood_ii_sse(unsigned int        states,
                                            unsigned int        sites,
                                            unsigned int        rate_cats,
                                            const double *      parent_clv,
                                            const unsigned int *parent_scaler,
                                            const double *      child_clv,
                                            const unsigned int *child_scaler,
                                            const double *      pmatrix,
                                            double *const *     frequencies,
                                            const double *      rate_weights,
                                            const unsigned int *pattern_weights,
                                            const double *invar_proportion,
                                            const int *   invar_indices,
                                            const unsigned int *freqs_indices,
                                            double *            persite_lnl,
                                            unsigned int        attrib);

  PLL_EXPORT
  double
  pll_core_edge_loglikelihood_ii_4x4_sse(unsigned int        sites,
                                         unsigned int        rate_cats,
                                         const double *      parent_clv,
                                         const unsigned int *parent_scaler,
                                         const double *      child_clv,
                                         const unsigned int *child_scaler,
                                         const double *      pmatrix,
                                         double *const *     frequencies,
                                         const double *      rate_weights,
                                         const unsigned int *pattern_weights,
                                         const double *      invar_proportion,
                                         const int *         invar_indices,
                                         const unsigned int *freqs_indices,
                                         double *            persite_lnl,
                                         unsigned int        attrib);

  PLL_EXPORT
  double pll_core_edge_loglikelihood_ti_sse(unsigned int         states,
                                            unsigned int         sites,
                                            unsigned int         rate_cats,
                                            const double *       parent_clv,
                                            const unsigned int * parent_scaler,
                                            const unsigned char *tipchars,
                                            const pll_state_t *  tipmap,
                                            const double *       pmatrix,
                                            double *const *      frequencies,
                                            const double *       rate_weights,
                                            const unsigned int *pattern_weights,
                                            const double *invar_proportion,
                                            const int *   invar_indices,
                                            const unsigned int *freqs_indices,
                                            double *            persite_lnl,
                                            unsigned int        attrib);

  PLL_EXPORT
  double
  pll_core_edge_loglikelihood_ti_4x4_sse(unsigned int         sites,
                                         unsigned int         rate_cats,
                                         const double *       parent_clv,
                                         const unsigned int * parent_scaler,
                                         const unsigned char *tipchars,
                                         const double *       pmatrix,
                                         double *const *      frequencies,
                                         const double *       rate_weights,
                                         const unsigned int * pattern_weights,
                                         const double *       invar_proportion,
                                         const int *          invar_indices,
                                         const unsigned int * freqs_indices,
                                         double *             persite_lnl,
                                         unsigned int         attrib);

  PLL_EXPORT double
  pll_core_root_loglikelihood_4x4_sse(unsigned int        sites,
                                      unsigned int        rate_cats,
                                      const double *      clv,
                                      const unsigned int *scaler,
                                      double *const *     frequencies,
                                      const double *      rate_weights,
                                      const unsigned int *pattern_weights,
                                      const double *      invar_proportion,
                                      const int *         invar_indices,
                                      const unsigned int *freqs_indices,
                                      double *            persite_lnl);

  PLL_EXPORT double
  pll_core_root_loglikelihood_sse(unsigned int        states,
                                  unsigned int        sites,
                                  unsigned int        rate_cats,
                                  const double *      clv,
                                  const unsigned int *scaler,
                                  double *const *     frequencies,
                                  const double *      rate_weights,
                                  const unsigned int *pattern_weights,
                                  const double *      invar_proportion,
                                  const int *         invar_indices,
                                  const unsigned int *freqs_indices,
                                  double *            persite_lnl);

  PLL_EXPORT double
  pll_core_root_loglikelihood_repeats_sse(unsigned int        states,
                                          unsigned int        sites,
                                          unsigned int        rate_cats,
                                          const double *      clv,
                                          const unsigned int *site_id,
                                          const unsigned int *scaler,
                                          double *const *     frequencies,
                                          const double *      rate_weights,
                                          const unsigned int *pattern_weights,
                                          const double *      invar_proportion,
                                          const int *         invar_indices,
                                          const unsigned int *freqs_indices,
                                          double *            persite_lnl);

  PLL_EXPORT double pll_core_edge_loglikelihood_repeats_generic_sse(
      unsigned int        states,
      unsigned int        sites,
      const unsigned int  child_sites,
      unsigned int        rate_cats,
      const double *      parent_clv,
      const unsigned int *parent_scaler,
      const double *      child_clv,
      const unsigned int *child_scaler,
      const double *      pmatrix,
      double **           frequencies,
      const double *      rate_weights,
      const unsigned int *pattern_weights,
      const double *      invar_proportion,
      const int *         invar_indices,
      const unsigned int *freqs_indices,
      double *            persite_lnl,
      const unsigned int *parent_site_id,
      const unsigned int *child_site_id,
      double *            bclv,
      unsigned int        attrib);
#endif

  /* functions in core_likelihood_avx.c */

#ifdef HAVE_AVX
  PLL_EXPORT double
  pll_core_edge_loglikelihood_ii_avx(unsigned int        states,
                                     unsigned int        sites,
                                     unsigned int        rate_cats,
                                     const double *      parent_clv,
                                     const unsigned int *parent_scaler,
                                     const double *      child_clv,
                                     const unsigned int *child_scaler,
                                     const double *      pmatrix,
                                     double *const *     frequencies,
                                     const double *      rate_weights,
                                     const unsigned int *pattern_weights,
                                     const double *      invar_proportion,
                                     const int *         invar_indices,
                                     const unsigned int *freqs_indices,
                                     double *            persite_lnl,
                                     unsigned int        attrib);

  PLL_EXPORT double
  pll_core_edge_loglikelihood_ii_4x4_avx(unsigned int        sites,
                                         unsigned int        rate_cats,
                                         const double *      parent_clv,
                                         const unsigned int *parent_scaler,
                                         const double *      child_clv,
                                         const unsigned int *child_scaler,
                                         const double *      pmatrix,
                                         double *const *     frequencies,
                                         const double *      rate_weights,
                                         const unsigned int *pattern_weights,
                                         const double *      invar_proportion,
                                         const int *         invar_indices,
                                         const unsigned int *freqs_indices,
                                         double *            persite_lnl,
                                         unsigned int        attrib);

  PLL_EXPORT double
  pll_core_edge_loglikelihood_ti_4x4_avx(unsigned int         sites,
                                         unsigned int         rate_cats,
                                         const double *       parent_clv,
                                         const unsigned int * parent_scaler,
                                         const unsigned char *tipchars,
                                         const double *       pmatrix,
                                         double *const *      frequencies,
                                         const double *       rate_weights,
                                         const unsigned int * pattern_weights,
                                         const double *       invar_proportion,
                                         const int *          invar_indices,
                                         const unsigned int * freqs_indices,
                                         double *             persite_lnl,
                                         unsigned int         attrib);

  PLL_EXPORT double
  pll_core_edge_loglikelihood_ti_20x20_avx(unsigned int         sites,
                                           unsigned int         rate_cats,
                                           const double *       parent_clv,
                                           const unsigned int * parent_scaler,
                                           const unsigned char *tipchars,
                                           const pll_state_t *  tipmap,
                                           unsigned int         tipmap_size,
                                           const double *       pmatrix,
                                           double *const *      frequencies,
                                           const double *       rate_weights,
                                           const unsigned int * pattern_weights,
                                           const double *      invar_proportion,
                                           const int *         invar_indices,
                                           const unsigned int *freqs_indices,
                                           double *            persite_lnl,
                                           unsigned int        attrib);

  PLL_EXPORT double
  pll_core_edge_loglikelihood_ti_avx(unsigned int         states,
                                     unsigned int         sites,
                                     unsigned int         rate_cats,
                                     const double *       parent_clv,
                                     const unsigned int * parent_scaler,
                                     const unsigned char *tipchars,
                                     const pll_state_t *  tipmap,
                                     const double *       pmatrix,
                                     double *const *      frequencies,
                                     const double *       rate_weights,
                                     const unsigned int * pattern_weights,
                                     const double *       invar_proportion,
                                     const int *          invar_indices,
                                     const unsigned int * freqs_indices,
                                     double *             persite_lnl,
                                     unsigned int         attrib);

  PLL_EXPORT double
  pll_core_root_loglikelihood_4x4_avx(unsigned int        sites,
                                      unsigned int        rate_cats,
                                      const double *      clv,
                                      const unsigned int *scaler,
                                      double *const *     frequencies,
                                      const double *      rate_weights,
                                      const unsigned int *pattern_weights,
                                      const double *      invar_proportion,
                                      const int *         invar_indices,
                                      const unsigned int *freqs_indices,
                                      double *            persite_lnl);

  PLL_EXPORT double
  pll_core_root_loglikelihood_avx(unsigned int        states,
                                  unsigned int        sites,
                                  unsigned int        rate_cats,
                                  const double *      clv,
                                  const unsigned int *scaler,
                                  double *const *     frequencies,
                                  const double *      rate_weights,
                                  const unsigned int *pattern_weights,
                                  const double *      invar_proportion,
                                  const int *         invar_indices,
                                  const unsigned int *freqs_indices,
                                  double *            persite_lnl);

  PLL_EXPORT double
  pll_core_root_loglikelihood_repeats_avx(unsigned int        states,
                                          unsigned int        sites,
                                          unsigned int        rate_cats,
                                          const double *      clv,
                                          const unsigned int *site_id,
                                          const unsigned int *scaler,
                                          double *const *     frequencies,
                                          const double *      rate_weights,
                                          const unsigned int *pattern_weights,
                                          const double *      invar_proportion,
                                          const int *         invar_indices,
                                          const unsigned int *freqs_indices,
                                          double *            persite_lnl);

  PLL_EXPORT double pll_core_edge_loglikelihood_repeats_generic_avx(
      unsigned int        states,
      unsigned int        sites,
      const unsigned int  child_sites,
      unsigned int        rate_cats,
      const double *      parent_clv,
      const unsigned int *parent_scaler,
      const double *      child_clv,
      const unsigned int *child_scaler,
      const double *      pmatrix,
      double **           frequencies,
      const double *      rate_weights,
      const unsigned int *pattern_weights,
      const double *      invar_proportion,
      const int *         invar_indices,
      const unsigned int *freqs_indices,
      double *            persite_lnl,
      const unsigned int *parent_site_id,
      const unsigned int *child_site_id,
      double *            bclv,
      unsigned int        attrib);

  PLL_EXPORT double pll_core_edge_loglikelihood_repeats_4x4_avx(
      unsigned int        states,
      unsigned int        sites,
      const unsigned int  child_sites,
      unsigned int        rate_cats,
      const double *      parent_clv,
      const unsigned int *parent_scaler,
      const double *      child_clv,
      const unsigned int *child_scaler,
      const double *      pmatrix,
      double **           frequencies,
      const double *      rate_weights,
      const unsigned int *pattern_weights,
      const double *      invar_proportion,
      const int *         invar_indices,
      const unsigned int *freqs_indices,
      double *            persite_lnl,
      const unsigned int *parent_site_id,
      const unsigned int *child_site_id,
      double *            bclv,
      unsigned int        attrib);

  PLL_EXPORT double pll_core_edge_loglikelihood_repeatsbclv_4x4_avx(
      unsigned int        states,
      unsigned int        sites,
      const unsigned int  child_sites,
      unsigned int        rate_cats,
      const double *      parent_clv,
      const unsigned int *parent_scaler,
      const double *      child_clv,
      const unsigned int *child_scaler,
      const double *      pmatrix,
      double **           frequencies,
      const double *      rate_weights,
      const unsigned int *pattern_weights,
      const double *      invar_proportion,
      const int *         invar_indices,
      const unsigned int *freqs_indices,
      double *            persite_lnl,
      const unsigned int *parent_site_id,
      const unsigned int *child_site_id,
      double *            bclv,
      unsigned int        attrib);
#endif

  /* functions in core_likelihood_avx2.c */

#ifdef HAVE_AVX2
  PLL_EXPORT
  double pll_core_root_loglikelihood_avx2(unsigned int        states,
                                          unsigned int        sites,
                                          unsigned int        rate_cats,
                                          const double *      clv,
                                          const unsigned int *scaler,
                                          double *const *     frequencies,
                                          const double *      rate_weights,
                                          const unsigned int *pattern_weights,
                                          const double *      invar_proportion,
                                          const int *         invar_indices,
                                          const unsigned int *freqs_indices,
                                          double *            persite_lnl);

  PLL_EXPORT
  double
  pll_core_edge_loglikelihood_ti_20x20_avx2(unsigned int         sites,
                                            unsigned int         rate_cats,
                                            const double *       parent_clv,
                                            const unsigned int * parent_scaler,
                                            const unsigned char *tipchars,
                                            const pll_state_t *  tipmap,
                                            unsigned int         tipmap_size,
                                            const double *       pmatrix,
                                            double *const *      frequencies,
                                            const double *       rate_weights,
                                            const unsigned int *pattern_weights,
                                            const double *invar_proportion,
                                            const int *   invar_indices,
                                            const unsigned int *freqs_indices,
                                            double *            persite_lnl,
                                            unsigned int        attrib);

  PLL_EXPORT
  double
  pll_core_edge_loglikelihood_ii_avx2(unsigned int        states,
                                      unsigned int        sites,
                                      unsigned int        rate_cats,
                                      const double *      parent_clv,
                                      const unsigned int *parent_scaler,
                                      const double *      child_clv,
                                      const unsigned int *child_scaler,
                                      const double *      pmatrix,
                                      double *const *     frequencies,
                                      const double *      rate_weights,
                                      const unsigned int *pattern_weights,
                                      const double *      invar_proportion,
                                      const int *         invar_indices,
                                      const unsigned int *freqs_indices,
                                      double *            persite_lnl,
                                      unsigned int        attrib);

  PLL_EXPORT
  double
  pll_core_root_loglikelihood_repeats_avx2(unsigned int        states,
                                           unsigned int        sites,
                                           unsigned int        rate_cats,
                                           const double *      clv,
                                           const unsigned int *site_id,
                                           const unsigned int *scaler,
                                           double *const *     frequencies,
                                           const double *      rate_weights,
                                           const unsigned int *pattern_weights,
                                           const double *      invar_proportion,
                                           const int *         invar_indices,
                                           const unsigned int *freqs_indices,
                                           double *            persite_lnl);

  PLL_EXPORT
  double pll_core_edge_loglikelihood_repeats_generic_avx2(
      unsigned int        states,
      unsigned int        sites,
      const unsigned int  child_sites,
      unsigned int        rate_cats,
      const double *      parent_clv,
      const unsigned int *parent_scaler,
      const double *      child_clv,
      const unsigned int *child_scaler,
      const double *      pmatrix,
      double **           frequencies,
      const double *      rate_weights,
      const unsigned int *pattern_weights,
      const double *      invar_proportion,
      const int *         invar_indices,
      const unsigned int *freqs_indices,
      double *            persite_lnl,
      const unsigned int *parent_site_id,
      const unsigned int *child_site_id,
      double *            bclv,
      unsigned int        attrib);

#endif

  /* functions in core_pmatrix.c */

  PLL_EXPORT int pll_core_update_pmatrix(double **           pmatrix,
                                         unsigned int        states,
                                         unsigned int        rate_cats,
                                         const double *      rates,
                                         const double *      branch_lengths,
                                         const unsigned int *matrix_indices,
                                         const unsigned int *params_indices,
                                         const double *      prop_invar,
                                         double *const *     eigenvals,
                                         double *const *     eigenvecs,
                                         double *const *     inv_eigenvecs,
                                         unsigned int        count,
                                         unsigned int        attrib);

  /* functions in core_pmatrix_avx2.c */

#ifdef HAVE_AVX2
  PLL_EXPORT int
  pll_core_update_pmatrix_20x20_avx2(double **           pmatrix,
                                     unsigned int        rate_cats,
                                     const double *      rates,
                                     const double *      branch_lengths,
                                     const unsigned int *matrix_indices,
                                     const unsigned int *params_indices,
                                     const double *      prop_invar,
                                     double *const *     eigenvals,
                                     double *const *     eigenvecs,
                                     double *const *     inv_eigenvecs,
                                     unsigned int        count);
#endif

  /* functions in core_pmatrix_avx.c */

#ifdef HAVE_AVX
  PLL_EXPORT int
  pll_core_update_pmatrix_4x4_avx(double **           pmatrix,
                                  unsigned int        rate_cats,
                                  const double *      rates,
                                  const double *      branch_lengths,
                                  const unsigned int *matrix_indices,
                                  const unsigned int *params_indices,
                                  const double *      prop_invar,
                                  double *const *     eigenvals,
                                  double *const *     eigenvecs,
                                  double *const *     inv_eigenvecs,
                                  unsigned int        count);

  PLL_EXPORT int
  pll_core_update_pmatrix_20x20_avx(double **           pmatrix,
                                    unsigned int        rate_cats,
                                    const double *      rates,
                                    const double *      branch_lengths,
                                    const unsigned int *matrix_indices,
                                    const unsigned int *params_indices,
                                    const double *      prop_invar,
                                    double *const *     eigenvals,
                                    double *const *     eigenvecs,
                                    double *const *     inv_eigenvecs,
                                    unsigned int        count);
#endif

  /* functions in core_pmatrix_sse.c */

#ifdef HAVE_SSE3
  PLL_EXPORT int
  pll_core_update_pmatrix_4x4_sse(double **           pmatrix,
                                  unsigned int        rate_cats,
                                  const double *      rates,
                                  const double *      branch_lengths,
                                  const unsigned int *matrix_indices,
                                  const unsigned int *params_indices,
                                  const double *      prop_invar,
                                  double *const *     eigenvals,
                                  double *const *     eigenvecs,
                                  double *const *     inv_eigenvecs,
                                  unsigned int        count);

  PLL_EXPORT int
  pll_core_update_pmatrix_20x20_sse(double **           pmatrix,
                                    unsigned int        rate_cats,
                                    const double *      rates,
                                    const double *      branch_lengths,
                                    const unsigned int *matrix_indices,
                                    const unsigned int *params_indices,
                                    const double *      prop_invar,
                                    double *const *     eigenvals,
                                    double *const *     eigenvecs,
                                    double *const *     inv_eigenvecs,
                                    unsigned int        count);
#endif

  /* functions in compress.c */

  /**
   * Compresses the MSA in place. This is to say, the buffer `sequence` is
   * changed to store the compressed alignment.
   *
   * @param[in,out] sequence The alignment to compress, should be the one from a
   * `pll_msa_t`.
   *
   * @param map The sequence encoding map. For example, `pll_map_nt`.
   *
   * @param count The number of sequences, also the number of tips, also the
   * number of taxa.
   *
   * @param[out] length The length of the compressed alignment.
   *
   * @ingroup pll_partition_t
   */
  PLL_EXPORT unsigned int *pll_compress_site_patterns(char **sequence,
                                                      const pll_state_t *map,
                                                      int                count,
                                                      int *length);

  PLL_EXPORT
  unsigned int *pll_compress_site_patterns_msa(pll_msa_t *        msa,
                                               const pll_state_t *map,
                                               unsigned int *site_pattern_map);

  /* functions in parsimony.c */

  PLL_EXPORT int pll_set_parsimony_sequence(pll_parsimony_t *  pars,
                                            unsigned int       tip_index,
                                            const pll_state_t *map,
                                            const char *       sequence);

  PLL_EXPORT pll_parsimony_t *
             pll_parsimony_create(unsigned int  tips,
                                  unsigned int  states,
                                  unsigned int  sites,
                                  const double *score_matrix,
                                  unsigned int  score_buffers,
                                  unsigned int  ancestral_buffers);

  PLL_EXPORT double pll_parsimony_build(pll_parsimony_t *         pars,
                                        const pll_pars_buildop_t *operations,
                                        unsigned int              count);

  PLL_EXPORT void pll_parsimony_reconstruct(pll_parsimony_t *       pars,
                                            const pll_state_t *     map,
                                            const pll_pars_recop_t *operations,
                                            unsigned int            count);

  PLL_EXPORT double pll_parsimony_score(pll_parsimony_t *pars,
                                        unsigned int     score_buffer_index);

  PLL_EXPORT void pll_parsimony_destroy(pll_parsimony_t *pars);

  /* functions in fast_parsimony.c */

  PLL_EXPORT pll_parsimony_t *
             pll_fastparsimony_init(const pll_partition_t *partition);

  PLL_EXPORT void
  pll_fastparsimony_update_vectors(pll_parsimony_t *         parsimony,
                                   const pll_pars_buildop_t *ops,
                                   unsigned int              count);

  PLL_EXPORT unsigned int
  pll_fastparsimony_root_score(const pll_parsimony_t *parsimony,
                               unsigned int           root_index);

  PLL_EXPORT unsigned int
  pll_fastparsimony_edge_score(const pll_parsimony_t *parsimony,
                               unsigned int           node1_score_index,
                               unsigned int           node2_score_index);

  PLL_EXPORT void
  pll_fastparsimony_update_vector_4x4(pll_parsimony_t *         parsimony,
                                      const pll_pars_buildop_t *op);

  PLL_EXPORT unsigned int
  pll_fastparsimony_edge_score_4x4(const pll_parsimony_t *parsimony,
                                   unsigned int           node1_score_index,
                                   unsigned int           node2_score_index);

  PLL_EXPORT void pll_fastparsimony_update_vector(pll_parsimony_t *parsimony,
                                                  const pll_pars_buildop_t *op);

  /* functions in fast_parsimony_sse.c */

  PLL_EXPORT void
  pll_fastparsimony_update_vector_4x4_sse(pll_parsimony_t *         parsimony,
                                          const pll_pars_buildop_t *op);

  PLL_EXPORT unsigned int
  pll_fastparsimony_edge_score_4x4_sse(const pll_parsimony_t *parsimony,
                                       unsigned int           node1_score_index,
                                       unsigned int node2_score_index);

  PLL_EXPORT unsigned int
  pll_fastparsimony_edge_score_sse(const pll_parsimony_t *parsimony,
                                   unsigned int           node1_score_index,
                                   unsigned int           node2_score_index);

  PLL_EXPORT void
  pll_fastparsimony_update_vector_sse(pll_parsimony_t *         parsimony,
                                      const pll_pars_buildop_t *op);

  /* functions in fast_parsimony_avx.c */

  PLL_EXPORT void
  pll_fastparsimony_update_vector_4x4_avx(pll_parsimony_t *         parsimony,
                                          const pll_pars_buildop_t *op);

  PLL_EXPORT unsigned int
  pll_fastparsimony_edge_score_4x4_avx(const pll_parsimony_t *parsimony,
                                       unsigned int           node1_score_index,
                                       unsigned int node2_score_index);

  PLL_EXPORT void
  pll_fastparsimony_update_vector_avx(pll_parsimony_t *         parsimony,
                                      const pll_pars_buildop_t *op);

  PLL_EXPORT unsigned int
  pll_fastparsimony_edge_score_avx(const pll_parsimony_t *parsimony,
                                   unsigned int           node1_score_index,
                                   unsigned int           node2_score_index);

  /* functions in fast_parsimony_avx2.c */

  PLL_EXPORT void
  pll_fastparsimony_update_vector_4x4_avx2(pll_parsimony_t *         parsimony,
                                           const pll_pars_buildop_t *op);

  PLL_EXPORT unsigned int
  pll_fastparsimony_edge_score_4x4_avx2(const pll_parsimony_t *parsimony,
                                        unsigned int node1_score_index,
                                        unsigned int node2_score_index);

  PLL_EXPORT void
  pll_fastparsimony_update_vector_avx2(pll_parsimony_t *         parsimony,
                                       const pll_pars_buildop_t *op);

  PLL_EXPORT unsigned int
  pll_fastparsimony_edge_score_avx2(const pll_parsimony_t *parsimony,
                                    unsigned int           node1_score_index,
                                    unsigned int           node2_score_index);

  /* functions in stepwise.c */

  PLL_EXPORT pll_utree_t *pll_fastparsimony_stepwise(pll_parsimony_t **list,
                                                     char *const *     labels,
                                                     unsigned int *    score,
                                                     unsigned int      count,
                                                     unsigned int      seed);

  /* functions in random.c */

  PLL_EXPORT extern int pll_random_r(struct pll_random_data *__buf,
                                     int32_t *               __result);

  PLL_EXPORT extern int pll_srandom_r(unsigned int            __seed,
                                      struct pll_random_data *__buf);

  PLL_EXPORT extern int pll_initstate_r(unsigned int            __seed,
                                        char *                  __statebuf,
                                        size_t                  __statelen,
                                        struct pll_random_data *__buf);

  PLL_EXPORT extern int pll_setstate_r(char *                  __statebuf,
                                       struct pll_random_data *__buf);

  PLL_EXPORT pll_random_state *pll_random_create(unsigned int seed);

  PLL_EXPORT int pll_random_getint(pll_random_state *rstate, int maxval);

  PLL_EXPORT void pll_random_destroy(pll_random_state *rstate);

  /* functions in hardware.c */

  PLL_EXPORT int pll_hardware_probe(void);

  PLL_EXPORT void pll_hardware_dump(void);

  PLL_EXPORT void pll_hardware_ignore(void);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
