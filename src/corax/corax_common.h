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
#define CORAX_EXPORT __declspec(dllexport)
#else
#define CORAX_EXPORT
#endif

/* macros */

#define CORAX_MIN(a, b) ((a) < (b) ? (a) : (b))
#define CORAX_MAX(a, b) ((a) > (b) ? (a) : (b))
#define CORAX_SWAP(x, y)                                                       \
  do {                                                                         \
    __typeof__(x) _t = x;                                                      \
    x                = y;                                                      \
    y                = _t;                                                     \
  } while (0)
#define CORAX_STAT(x)                                                          \
  ((corax_hardware.init || corax_hardware_probe()) && corax_hardware.x)
#define CORAX_UNUSED(expr)                                                     \
  do {                                                                         \
    (void)(expr);                                                              \
  } while (0)

#define CORAX_UTREE_IS_TIP(node) (node->next == NULL)

/** @defgroup corax_defines Constant Definitions
 * @{
 */
/* constants */
#define CORAX_FAILURE 0
#define CORAX_SUCCESS 1

#define CORAX_FALSE 0
#define CORAX_TRUE 1

#define CORAX_ALIGNMENT_CPU 8
#define CORAX_ALIGNMENT_SSE 16
#define CORAX_ALIGNMENT_AVX 32

#define CORAX_LINEALLOC 2048

#define CORAX_ASCII_SIZE 256

#define CORAX_ERRMSG_LEN 200

#define CORAX_SCALE_FACTOR                                                     \
  115792089237316195423570985008687907853269984665640564039457584007913129639936.0 /*  2**256 (exactly)  */
#define CORAX_SCALE_THRESHOLD (1.0 / CORAX_SCALE_FACTOR)
#define CORAX_SCALE_FACTOR_SQRT                                                \
  340282366920938463463374607431768211456.0 /* 2**128 */
#define CORAX_SCALE_THRESHOLD_SQRT (1.0 / CORAX_SCALE_FACTOR_SQRT)
#define CORAX_SCALE_BUFFER_NONE -1

/* in per-rate scaling mode, maximum difference between scalers
 * please see https://github.com/xflouris/libpll/issues/44  */
#define CORAX_SCALE_RATE_MAXDIFF 4

#define CORAX_MISC_EPSILON 1e-8
#define CORAX_ONE_EPSILON 1e-15
#define CORAX_ONE_MIN (1 - CORAX_ONE_EPSILON)
#define CORAX_ONE_MAX (1 + CORAX_ONE_EPSILON)
#define CORAX_EIGEN_MINFREQ 1e-6

#define CORAX_TREE_DEFAULT_BRANCH_LENGTH 0.1
/** @} */

/** @defgroup corax_attributes Attributes
 * These are flags which are used to control the behavior of a partition.
 * @{
 */

/* attribute flags */

/** Flag specifying no SIMD operations */
#define CORAX_ATTRIB_ARCH_CPU 0
/** Flag specifying only SSE3 SIMD operations */
#define CORAX_ATTRIB_ARCH_SSE (1 << 0)
/** Flag specifying only AVX SIMD operations */
#define CORAX_ATTRIB_ARCH_AVX (1 << 1)
/** Flag specifying only AVX2 SIMD operations */
#define CORAX_ATTRIB_ARCH_AVX2 (1 << 2)
/** Flag specifying only AVX512 SIMD operations */
#define CORAX_ATTRIB_ARCH_AVX512 (1 << 3)
/** Mask for the CPU architecture attributes */
#define CORAX_ATTRIB_ARCH_MASK 0xF

/**
 * Flag which indicates the use of the pattern tip optimization. Mutually
 * exclusive with the `CORAX_ATTRIB_SITE_REPEATS` flag.
 */
#define CORAX_ATTRIB_PATTERN_TIP (1 << 4)

/* ascertainment bias correction */
#define CORAX_ATTRIB_AB_LEWIS (1 << 5)
#define CORAX_ATTRIB_AB_FELSENSTEIN (2 << 5)
#define CORAX_ATTRIB_AB_STAMATAKIS (3 << 5)
#define CORAX_ATTRIB_AB_MASK (7 << 5)
#define CORAX_ATTRIB_AB_FLAG (1 << 8)

#define CORAX_ATTRIB_RATE_SCALERS (1 << 9)

/* site repeats */

/**
 * Flag indicating the use of the site repeats optimization. Mutually exclusive
 * with the `CORAX_ATTRIB_PATTERN_TIP` flag.
 */
#define CORAX_ATTRIB_SITE_REPEATS (1 << 10)

/** Mask for all the attributes currently defined */
#define CORAX_ATTRIB_MASK ((1 << 11) - 1)

/** @} */

#define CORAX_REPEATS_LOOKUP_SIZE 2000000

/* topological rearrangements */

#define CORAX_UTREE_MOVE_SPR 1
#define CORAX_UTREE_MOVE_NNI 2

#define CORAX_UTREE_MOVE_NNI_LEFT 1
#define CORAX_UTREE_MOVE_NNI_RIGHT 2

#define CORAX_TREE_TRAVERSE_POSTORDER 1
#define CORAX_TREE_TRAVERSE_PREORDER 2

/** @defgroup corax_errors Error Codes
 * Error codes for coraxlib.
 * @{
 */
/* error codes */
#define CORAX_ERROR_NOT_IMPLEMENTED 13
#define CORAX_ERROR_INVALID_RANGE 21
#define CORAX_ERROR_INVALID_NODE_TYPE 22
#define CORAX_ERROR_INVALID_INDEX 23
#define CORAX_ERROR_INVALID_PARAM 24
#define CORAX_ERROR_INVALID_TREE 25
#define CORAX_ERROR_INVALID_TREE_SIZE 26
#define CORAX_ERROR_INVALID_SPLIT 27
#define CORAX_ERROR_INVALID_THRESHOLD 28

#define CORAX_ERROR_FILE_OPEN 100
#define CORAX_ERROR_FILE_SEEK 101
#define CORAX_ERROR_FILE_EOF 102
#define CORAX_ERROR_FASTA_ILLEGALCHAR 201
#define CORAX_ERROR_FASTA_UNPRINTABLECHAR 202
#define CORAX_ERROR_FASTA_INVALIDHEADER 203
#define CORAX_ERROR_FASTA_NONALIGNED 204
#define CORAX_ERROR_PHYLIP_SYNTAX 231
#define CORAX_ERROR_PHYLIP_LONGSEQ 232
#define CORAX_ERROR_PHYLIP_NONALIGNED 233
#define CORAX_ERROR_PHYLIP_ILLEGALCHAR 234
#define CORAX_ERROR_PHYLIP_UNPRINTABLECHAR 235
#define CORAX_ERROR_NEWICK_SYNTAX 111
#define CORAX_ERROR_MEM_ALLOC 112
#define CORAX_ERROR_TIPDATA_ILLEGALSTATE 114
#define CORAX_ERROR_TIPDATA_ILLEGALFUNCTION 115
#define CORAX_ERROR_TREE_CONVERSION 116
#define CORAX_ERROR_INVAR_INCOMPAT 117
#define CORAX_ERROR_INVAR_PROPORTION 118
#define CORAX_ERROR_INVAR_PARAMINDEX 119
#define CORAX_ERROR_INVAR_NONEFOUND 120
#define CORAX_ERROR_AB_INVALIDMETHOD 121
#define CORAX_ERROR_AB_NOSUPPORT 122
#define CORAX_ERROR_SPR_TERMINALBRANCH 123
#define CORAX_ERROR_SPR_NOCHANGE 124
#define CORAX_ERROR_NNI_INVALIDMOVE 125
#define CORAX_ERROR_NNI_TERMINALBRANCH 126
#define CORAX_ERROR_STEPWISE_STRUCT 127
#define CORAX_ERROR_STEPWISE_TIPS 128
#define CORAX_ERROR_STEPWISE_UNSUPPORTED 129
#define CORAX_ERROR_EINVAL 130
#define CORAX_ERROR_MSA_EMPTY 131
#define CORAX_ERROR_MSA_MAP_INVALID 132
/** @} */

/* utree specific */

#define CORAX_UTREE_SHOW_LABEL (1 << 0)
#define CORAX_UTREE_SHOW_BRANCH_LENGTH (1 << 1)
#define CORAX_UTREE_SHOW_CLV_INDEX (1 << 2)
#define CORAX_UTREE_SHOW_SCALER_INDEX (1 << 3)
#define CORAX_UTREE_SHOW_PMATRIX_INDEX (1 << 4)
#define CORAX_UTREE_SHOW_DATA (1 << 5)

/* GAMMA discretization modes */
#define CORAX_GAMMA_RATES_MEAN 0
#define CORAX_GAMMA_RATES_MEDIAN 1

/* branch linkage modes */
#define CORAX_BRLEN_LINKED 0
#define CORAX_BRLEN_SCALED 1
#define CORAX_BRLEN_UNLINKED 2

/* parallel reduction modes */
#define CORAX_REDUCE_SUM 0
#define CORAX_REDUCE_MAX 1
#define CORAX_REDUCE_MIN 2

// TODO: this must be adapted for MSVC
#define CORAX_POPCNT32 __builtin_popcount
#define CORAX_POPCNT64 __builtin_popcountll
#define CORAX_CTZ32 __builtin_ctz
#define CORAX_CTZ64 __builtin_ctzll

/* structures and data types */

#define CORAX_STATE_POPCNT CORAX_POPCNT64
#define CORAX_STATE_CTZ CORAX_CTZ64

typedef unsigned long long corax_state_t;
typedef int                corax_bool_t;

typedef struct corax_hardware_s
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
} corax_hardware_t;

struct corax_repeats;

/** @defgroup corax_partition_t corax_partition_t
 * Module concerning the corax_partition_t
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
 * @ingroup corax_partition_t
 */
typedef struct corax_partition
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
   * nodes in the tree, and not the number of `corax_unode_t` present in the
   * tree structure. Includes the tips.
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
   * Bitvector of the flags used for computation in the `corax_partition_t`.
   *
   * @ingroup corax_attributes
   */
  unsigned int attributes;

  /* vectorization options */

  /**
   * One of three constants depending on what architecture is being used. At the
   * time of writing these are:
   * - `CORAX_ALIGNMENT_CPU`
   * - `CORAX_ALIGNMENT_SSE`
   * - `CORAX_ALIGNMENT_AVX`
   */
  size_t alignment;

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
  corax_state_t * tipmap;

  /* ascertainment bias correction */
  int asc_bias_alloc;
  int asc_additional_sites; // partition->asc_bias_alloc ? states : 0

  /* site repeats */
  /**
   * If repeats are disabled, repeats is set to NULL. Otherwise, it points to a
   * structure holding all information required to use the site repeats
   * optimization.
   */
  struct corax_repeats *repeats;
} corax_partition_t;

/** @defgroup corax_repeats_t corax_repeats_t
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
 *  @ingroup corax_repeats_t
 */
typedef struct corax_repeats
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
   * `corax_operation_t *op`, if scalers are enabled:
   * `pernode_ids[op->parent_clv_index] ==
   * perscale_ids[op->parent_scaler_index]`
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
   * definition is is `corax_default_enable_repeats`
   */
  unsigned int (*enable_repeats)(struct corax_partition *partition,
                                 unsigned int            left_clv,
                                 unsigned int            right_clv);

  /**
   * callback called when repeats are updated. Reallocate the CLV
   * and scaler vector for the node whose clv_index is parent.
   * sites_to_alloc indicates the number of "unique sites",
   * (or rather class identifiers) for this node.
   *
   * This function can be redefined and its default definition
   * is corax_default_reallocate_repeats
   *
   * By default, we always reallocate the exact required size, in
   * order to save memory. An alternative strategy could consist
   * in preallocating the maximum size (assuming that there is no
   * repeat) once at the first call, and then not doing anything
   * at the next calls (to avoid deallocation/reallocation).
   */
  void (*reallocate_repeats)(struct corax_partition *partition,
                             unsigned int            parent,
                             int                     scaler_index,
                             unsigned int            sites_to_alloc);

  /*
   * The `lookup_buffer` corresponds to the "matrix M" in the original site
   * repeats publication. It is used to compute the repeat classes at a given
   * node `u`. Let `v` and `w` be the children of `u`, and let `nv` and `nw` be
   * their respective numbers of class repeats. If `nv*nw > lookup_buffer_size`,
   * site repeats cannot be computed, and we disable site repeats for node `u`.
   * Thus, `lookup_buffer_size` should be large enough to allow as much nodes as
   * possible to benefit from site repeats, without costing too much memory
   * (remember, there might be many partitions...). Default size is
   * `CORAX_REPEATS_LOOKUP_SIZE` and can be changed with
   * `corax_resize_repeats_lookup`
   */
  unsigned int *lookup_buffer;
  unsigned int  lookup_buffer_size;

  /**
   * Map each character (representing a state) to a unique identifier
   */
  char *charmap;

  /**
   * Those vectors are pre-allocated buffers for the site repeats algorithm (or
   * for using site repeats in some kernels functions). They are only relevant
   * in the scope of the function in which they are being used.
   */
  unsigned int *toclean_buffer;
  unsigned int *id_site_buffer;
  double *      bclv_buffer;

} corax_repeats_t;

/** @defgroup corax_operation_t corax_operation_t
 * Module containing structures and functions related to operations.
 */

/**
 * Structure for driving likelihood operations. In general should only be
 * created using `corax_utree_create_operations`.
 *
 * @ingroup corax_operation_t
 */
typedef struct corax_operation
{
  unsigned int parent_clv_index;
  int          parent_scaler_index;
  unsigned int child1_clv_index;
  unsigned int child1_matrix_index;
  int          child1_scaler_index;
  unsigned int child2_clv_index;
  unsigned int child2_matrix_index;
  int          child2_scaler_index;
} corax_operation_t;

/* Doubly-linked list */

typedef struct corax_dlist
{
  struct corax_dlist *next;
  struct corax_dlist *prev;
  void *              data;
} corax_dlist_t;

/* multiple sequence alignment */
typedef struct corax_msa_s
{
  int count;
  int length;

  char **sequence;
  char **label;
} corax_msa_t;

/* Simple structure for handling FASTA parsing */

typedef struct corax_fasta
{
  FILE *              fp;
  char                line[CORAX_LINEALLOC];
  const unsigned int *chrstatus;
  long                no;
  long                filesize;
  long                lineno;
  long                stripped_count;
  long                stripped[256];
} corax_fasta_t;

/* Simple structure for handling PHYLIP parsing */
typedef struct corax_phylip_s
{
  FILE *              fp;
  char *              line;
  size_t              line_size;
  size_t              line_maxsize;
  char                buffer[CORAX_LINEALLOC];
  const unsigned int *chrstatus;
  long                no;
  long                filesize;
  long                lineno;
  long                stripped_count;
  long                stripped[256];
} corax_phylip_t;

/**
 * A structure that is a fundamental element of `corax_utree_t`. It contains a
 * next and back pointer. For more information, please see docs/corax_utree_t.md
 *
 */
typedef struct corax_unode_s
{
  /**
   * Label for the tree. Optional. If not present, then should be set to
   * `nullptr`
   */
  char *label;

  /**
   * Length of the edge, which is represented by the back pointer
   */
  double length;

  /**
   * Index of this node in the `nodes` buffer of `corax_utree_t`. Each
   * "super"-node shares and index. I.E. the index is on the "tree node" level,
   * not on the corax_unode_t level.
   */
  unsigned int node_index;

  /**
   * Index into the CLV buffer when computing a likelihood. For more
   * information, please see the documentation on `corax_partition_t`.
   */
  unsigned int clv_index;

  /**
   * Index into the scalar array to represent the CLV scaler. Please see the
   * documentation on `corax_partition_t` for more information.
   */
  int scaler_index;

  /**
   * Index into the array of probability matrices. These probability matrices
   * will be computed based on the branch length `length`. For more information
   * please see the documentation on `corax_partition_t`.
   */
  unsigned int pmatrix_index;

  /**
   * See the explaination in the concepts section of `docs/corax_utree_t.md`
   */
  struct corax_unode_s *next;

  /**
   * See the explaination in the concepts section of `docs/corax_utree_t.md`
   */
  struct corax_unode_s *back;

  /**
   * An extra pointer to store "user data". In praactice, this section can be
   * used for any task, but exsiting functions might use it, so be careful.
   */
  void *data;
} corax_unode_t;

/** @defgroup corax_utree_t corax_utree_t
 * Module for the `corax_utree_t` struct and associated functions
 */

/**
 * The data structure is made up of two different structs. The first,
 * corax_utree_t wraps the tree. In general, when a tree is used for a function,
 * it requires a corax_utree_t. Some important things to know about this
 * structure: the first inner_count nodes in the nodes array are assumed to be
 * "inner nodes". This means that they have a non-null next pointer. Several
 * functions that use corax_utree_ts don't check for this, so they may fail when
 * this assumption is violated. To avoid this, use the corax_utree_wraptree
 * function discussed below to create a corax_utree_t.
 *
 * @ingroup corax_utree_t
 */
typedef struct corax_utree_s
{
  /**
   * Number of tips in the tree
   */
  unsigned int tip_count;

  /**
   * Number of inner nodes. Not the number of `corax_unode_t` that make up the
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
  int binary;

  /**
   * An array of `corax_unode_t` pointers
   */
  corax_unode_t **nodes;

  /**
   * A pointer to the virtual root. By convention, this is always an inner node.
   * All tree manipulation functions such as `corax_utree_wraptree` follow this
   * convention.
   */
  corax_unode_t *vroot;
} corax_utree_t;

/* structures for parsimony */

typedef struct corax_parsimony_s
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
} corax_parsimony_t;

typedef struct corax_pars_buildop_s
{
  unsigned int parent_score_index;
  unsigned int child1_score_index;
  unsigned int child2_score_index;
} corax_pars_buildop_t;

typedef struct corax_pars_recop_s
{
  unsigned int node_score_index;
  unsigned int node_ancestral_index;
  unsigned int parent_score_index;
  unsigned int parent_ancestral_index;
} corax_pars_recop_t;

/* structures for SVG visualization */

typedef struct corax_svg_attrib_s
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
} corax_svg_attrib_t;

/* Reentrant versions of the `random' family of functions.
   These functions all use the following data structure to contain
   state, rather than global state variables. Taken and modified from
   glibc 2.23 */

struct corax_random_data
{
  int32_t *fptr;      /* Front pointer.  */
  int32_t *rptr;      /* Rear pointer.  */
  int32_t *state;     /* Array of state values.  */
  int      rand_type; /* Type of random number generator.  */
  int      rand_deg;  /* Degree of random number generator.  */
  int      rand_sep;  /* Distance between front and rear.  */
  int32_t *end_ptr;   /* Pointer behind state table.  */
};

typedef struct corax_random_state_s
{
  struct corax_random_data rdata;
  char *                   state_buf; /* Buffer to store state */
} corax_random_state;

/* common data */

CORAX_EXPORT extern __thread int              corax_errno;
CORAX_EXPORT extern __thread char             corax_errmsg[200];
CORAX_EXPORT extern __thread corax_hardware_t corax_hardware;

CORAX_EXPORT extern const corax_state_t corax_map_bin[256];
CORAX_EXPORT extern const corax_state_t corax_map_nt[256];
CORAX_EXPORT extern const corax_state_t corax_map_aa[256];
CORAX_EXPORT extern const corax_state_t corax_map_gt10[256];
CORAX_EXPORT extern const unsigned int  corax_map_fasta[256];
CORAX_EXPORT extern const unsigned int  corax_map_phylip[256];
CORAX_EXPORT extern const unsigned int  corax_map_generic[256];

CORAX_EXPORT extern const double corax_aa_rates_dayhoff[190];
CORAX_EXPORT extern const double corax_aa_rates_lg[190];
CORAX_EXPORT extern const double corax_aa_rates_dcmut[190];
CORAX_EXPORT extern const double corax_aa_rates_jtt[190];
CORAX_EXPORT extern const double corax_aa_rates_mtrev[190];
CORAX_EXPORT extern const double corax_aa_rates_wag[190];
CORAX_EXPORT extern const double corax_aa_rates_rtrev[190];
CORAX_EXPORT extern const double corax_aa_rates_cprev[190];
CORAX_EXPORT extern const double corax_aa_rates_vt[190];
CORAX_EXPORT extern const double corax_aa_rates_blosum62[190];
CORAX_EXPORT extern const double corax_aa_rates_mtmam[190];
CORAX_EXPORT extern const double corax_aa_rates_mtart[190];
CORAX_EXPORT extern const double corax_aa_rates_mtzoa[190];
CORAX_EXPORT extern const double corax_aa_rates_pmb[190];
CORAX_EXPORT extern const double corax_aa_rates_hivb[190];
CORAX_EXPORT extern const double corax_aa_rates_hivw[190];
CORAX_EXPORT extern const double corax_aa_rates_jttdcmut[190];
CORAX_EXPORT extern const double corax_aa_rates_flu[190];
CORAX_EXPORT extern const double corax_aa_rates_stmtrev[190];
CORAX_EXPORT extern const double corax_aa_rates_den[190];
CORAX_EXPORT extern const double corax_aa_rates_lg4m[4][190];
CORAX_EXPORT extern const double corax_aa_rates_lg4x[4][190];

CORAX_EXPORT extern const double corax_aa_freqs_dayhoff[20];
CORAX_EXPORT extern const double corax_aa_freqs_lg[20];
CORAX_EXPORT extern const double corax_aa_freqs_dcmut[20];
CORAX_EXPORT extern const double corax_aa_freqs_jtt[20];
CORAX_EXPORT extern const double corax_aa_freqs_mtrev[20];
CORAX_EXPORT extern const double corax_aa_freqs_wag[20];
CORAX_EXPORT extern const double corax_aa_freqs_rtrev[20];
CORAX_EXPORT extern const double corax_aa_freqs_cprev[20];
CORAX_EXPORT extern const double corax_aa_freqs_vt[20];
CORAX_EXPORT extern const double corax_aa_freqs_blosum62[20];
CORAX_EXPORT extern const double corax_aa_freqs_mtmam[20];
CORAX_EXPORT extern const double corax_aa_freqs_mtart[20];
CORAX_EXPORT extern const double corax_aa_freqs_mtzoa[20];
CORAX_EXPORT extern const double corax_aa_freqs_pmb[20];
CORAX_EXPORT extern const double corax_aa_freqs_hivb[20];
CORAX_EXPORT extern const double corax_aa_freqs_hivw[20];
CORAX_EXPORT extern const double corax_aa_freqs_jttdcmut[20];
CORAX_EXPORT extern const double corax_aa_freqs_flu[20];
CORAX_EXPORT extern const double corax_aa_freqs_stmtrev[20];
CORAX_EXPORT extern const double corax_aa_freqs_den[20];
CORAX_EXPORT extern const double corax_aa_freqs_lg4m[4][20];
CORAX_EXPORT extern const double corax_aa_freqs_lg4x[4][20];

#ifdef __cplusplus
extern "C"
{
#endif

  /* functions in common.c */

  void corax_set_error(int _errno, const char *errmsg_fmt, ...);
  void corax_reset_error();

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
   * corax_compress_site_patterns, or the number of unique site patterns.
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
   * @ingroup corax_partition_t
   */
  CORAX_EXPORT corax_partition_t *
               corax_partition_create(unsigned int tips,
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
   * @ingroup corax_partition_t
   */
  CORAX_EXPORT void corax_partition_destroy(corax_partition_t *partition);

  /**
   * Set the tip states based on an MSA for a partition. This will initialize
   * the tip CLVs based on the MSA that is passed.
   *
   * @param partition Partition to set the tips for.
   *
   * @param tip_index Index of the tip to be initialized.
   *
   * @param map A predefined map from `char` to `int` The choices are:
   * - corax_map_bin: For binary data with an alphabet of 0 and 1.
   * - corax_map_nt: For nucleotide data.
   * - corax_map_aa: For amino acid data.
   *
   * @param sequence The sequence associated with that tip.
   *
   * @ingroup corax_partition_t
   */
  CORAX_EXPORT int corax_set_tip_states(corax_partition_t *  partition,
                                        unsigned int         tip_index,
                                        const corax_state_t *map,
                                        const char *         sequence);

  CORAX_EXPORT int corax_set_tip_clv(corax_partition_t *partition,
                                     unsigned int       tip_index,
                                     const double *     clv,
                                     int                padding);

  /**
   * Sets the pattern weights for a partition.
   *
   * @param partition The partition to set the weights on.
   *
   * @param pattern_weights The array of weights. While you could set this
   * yourself, it should typically be the output of
   * `corax_compress_site_patterns`
   *
   * @ingroup corax_partition_t
   */
  CORAX_EXPORT void
  corax_set_pattern_weights(corax_partition_t * partition,
                            const unsigned int *pattern_weights);

  CORAX_EXPORT int corax_set_asc_bias_type(corax_partition_t *partition,
                                           int                asc_bias_type);

  CORAX_EXPORT void
  corax_set_asc_state_weights(corax_partition_t * partition,
                              const unsigned int *state_weights);

  /* functions in list.c */

  CORAX_EXPORT int corax_dlist_append(corax_dlist_t **dlist, void *data);
  CORAX_EXPORT int corax_dlist_remove(corax_dlist_t **dlist, void *data);
  CORAX_EXPORT int corax_dlist_prepend(corax_dlist_t **dlist, void *data);

  CORAX_EXPORT void corax_fill_parent_scaler(unsigned int        scaler_size,
                                             unsigned int *      parent_scaler,
                                             const unsigned int *left_scaler,
                                             const unsigned int *right_scaler);

  /* functions in repeats.c */

#define CORAX_GET_ID(site_id, site) ((site_id) ? ((site_id)[(site)]) : (site))
#define CORAX_GET_SITE(id_site, site) ((id_site) ? ((id_site)[(site)]) : (site))

  CORAX_EXPORT int corax_repeats_enabled(const corax_partition_t *partition);

  CORAX_EXPORT void corax_resize_repeats_lookup(corax_partition_t *partition,
                                                unsigned int       size);

  CORAX_EXPORT unsigned int
  corax_get_sites_number(const corax_partition_t *partition,
                         unsigned int             clv_index);

  CORAX_EXPORT unsigned int *
  corax_get_site_id(const corax_partition_t *partition, unsigned int clv_index);

  CORAX_EXPORT unsigned int *
  corax_get_id_site(const corax_partition_t *partition, unsigned int clv_index);

  CORAX_EXPORT unsigned int
  corax_get_clv_size(const corax_partition_t *partition,
                     unsigned int             clv_index);

  CORAX_EXPORT unsigned int
  corax_default_enable_repeats(corax_partition_t *partition,
                               unsigned int       left_clv,
                               unsigned int       right_clv);

  CORAX_EXPORT unsigned int
  corax_no_enable_repeats(corax_partition_t *partition,
                          unsigned int       left_clv,
                          unsigned int       right_clv);

  CORAX_EXPORT void
  corax_default_reallocate_repeats(corax_partition_t *partition,
                                   unsigned int       parent,
                                   int                scaler_index,
                                   unsigned int       sites_to_alloc);

  CORAX_EXPORT int corax_repeats_initialize(corax_partition_t *partition);

  CORAX_EXPORT int corax_update_repeats_tips(corax_partition_t *  partition,
                                             unsigned int         tip_index,
                                             const corax_state_t *map,
                                             const char *         sequence);

  CORAX_EXPORT void corax_update_repeats(corax_partition_t *      partition,
                                         const corax_operation_t *op);

  CORAX_EXPORT void corax_disable_bclv(corax_partition_t *partition);

  CORAX_EXPORT void
  corax_fill_parent_scaler_repeats(unsigned int        sites,
                                   unsigned int *      parent_scaler,
                                   const unsigned int *psites,
                                   const unsigned int *left_scaler,
                                   const unsigned int *lids,
                                   const unsigned int *right_scaler,
                                   const unsigned int *rids);

  CORAX_EXPORT void
  corax_fill_parent_scaler_repeats_per_rate(unsigned int        sites,
                                            unsigned int        rates,
                                            unsigned int *      parent_scaler,
                                            const unsigned int *psites,
                                            const unsigned int *left_scaler,
                                            const unsigned int *lids,
                                            const unsigned int *right_scaler,
                                            const unsigned int *rids);

  /* functions in models.c */

  CORAX_EXPORT unsigned int corax_subst_rate_count(unsigned int states);

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
   * @ingroup corax_partition_t
   */
  CORAX_EXPORT void corax_set_subst_params(corax_partition_t *partition,
                                           unsigned int       params_index,
                                           const double *     params);

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
   * @ingroup corax_partition_t
   */
  CORAX_EXPORT void corax_set_frequencies(corax_partition_t *partition,
                                          unsigned int       params_index,
                                          const double *     frequencies);

  CORAX_EXPORT void corax_set_category_rates(corax_partition_t *partition,
                                             const double *     rates);

  CORAX_EXPORT void corax_set_category_weights(corax_partition_t *partition,
                                               const double *     rate_weights);

  CORAX_EXPORT int corax_update_eigen(corax_partition_t *partition,
                                      unsigned int       params_index);

  /**
   * Update the probability matrices of partition.
   *
   * @param params_index Index of the parameters to use, as in rate categories.
   *
   * @param matrix_indices An array of indices into the `prob_matrices` in
   * `corax_partition_t`. These are the locations in which the matrices will be
   * stored. The best way to get these is via `corax_utree_create_operations`.
   *
   * @param branch_lengths A list of branch lengths which will be used for
   * computing the probability matrices. The best way to get these is via
   * `corax_utree_create_operations`.
   *
   * @param count The number of matrices to update.
   *
   * @ingroup corax_partition_t
   */
  CORAX_EXPORT int
  corax_update_prob_matrices(corax_partition_t * partition,
                             const unsigned int *params_index,
                             const unsigned int *matrix_indices,
                             const double *      branch_lengths,
                             unsigned int        count);

  CORAX_EXPORT unsigned int
  corax_count_invariant_sites(corax_partition_t *partition,
                              unsigned int *     state_inv_count);

  CORAX_EXPORT int corax_update_invariant_sites(corax_partition_t *partition);

  CORAX_EXPORT int
  corax_update_invariant_sites_proportion(corax_partition_t *partition,
                                          unsigned int       params_index,
                                          double             prop_invar);

  CORAX_EXPORT void *corax_aligned_alloc(size_t size, size_t alignment);

  CORAX_EXPORT void corax_aligned_free(void *ptr);

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
   * @ingroup corax_partition_t
   */
  CORAX_EXPORT double
  corax_compute_root_loglikelihood(corax_partition_t * partition,
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
   * @ingroup corax_partition_t
   */
  CORAX_EXPORT double
  corax_compute_edge_loglikelihood(corax_partition_t * partition,
                                   unsigned int        parent_clv_index,
                                   int                 parent_scaler_index,
                                   unsigned int        child_clv_index,
                                   int                 child_scaler_index,
                                   unsigned int        matrix_index,
                                   const unsigned int *freqs_indices,
                                   double *            persite_lnl);

  CORAX_EXPORT int
  corax_compute_node_ancestral(corax_partition_t * partition,
                               unsigned int        node_clv_index,
                               int                 node_scaler_index,
                               unsigned int        other_clv_index,
                               int                 other_scaler_index,
                               unsigned int        matrix_index,
                               const unsigned int *freqs_indices,
                               double *            ancestral);

  CORAX_EXPORT int
  corax_compute_node_ancestral_extbuf(corax_partition_t * partition,
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
   * using corax_utree_create_operations
   *
   * @param count Number of elements in the operations buffer.
   *
   * @ingroup corax_partition_t
   * @ingroup corax_operation_t
   */
  CORAX_EXPORT void corax_update_clvs(corax_partition_t *      partition,
                                      const corax_operation_t *operations,
                                      unsigned int             count);

  CORAX_EXPORT void corax_update_clvs_rep(corax_partition_t *      partition,
                                          const corax_operation_t *operations,
                                          unsigned int             count,
                                          unsigned int update_repeats);

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
   * @ingroup corax_partition_t
   */
  CORAX_EXPORT int corax_update_sumtable(corax_partition_t *partition,
                                         unsigned int       parent_clv_index,
                                         unsigned int       child_clv_index,
                                         int                parent_scaler_index,
                                         int                child_scaler_index,
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
   * @param sumtable Sumbtable from `corax_udate_sumtable`.
   *
   * @param[out] d_f Buffer to store the first derivative. Only a single double.
   *
   * @param[out] dd_f Buffer to store the second derivative. Only a single
   * double.
   *
   * @ingroup corax_partition_t
   */
  CORAX_EXPORT int
  corax_compute_likelihood_derivatives(corax_partition_t * partition,
                                       int                 parent_scaler_index,
                                       int                 child_scaler_index,
                                       double              branch_length,
                                       const unsigned int *params_indices,
                                       const double *      sumtable,
                                       double *            d_f,
                                       double *            dd_f);

  /* functions in gamma.c */

  CORAX_EXPORT int corax_compute_gamma_cats(double       alpha,
                                            unsigned int categories,
                                            double *     output_rates,
                                            int          rates_mode);

  /* functions in output.c */

  CORAX_EXPORT void corax_show_pmatrix(const corax_partition_t *partition,
                                       unsigned int             index,
                                       unsigned int float_precision);

  CORAX_EXPORT void corax_show_clv(const corax_partition_t *partition,
                                   unsigned int             clv_index,
                                   int                      scaler_index,
                                   unsigned int             float_precision);

  /* functions in fasta.c */

  CORAX_EXPORT corax_fasta_t *corax_fasta_open(const char *        filename,
                                               const unsigned int *map);

  CORAX_EXPORT int corax_fasta_getnext(corax_fasta_t *fd,
                                       char **        head,
                                       long *         head_len,
                                       char **        seq,
                                       long *         seq_len,
                                       long *         seqno);

  CORAX_EXPORT void corax_fasta_close(corax_fasta_t *fd);

  CORAX_EXPORT long corax_fasta_getfilesize(const corax_fasta_t *fd);

  CORAX_EXPORT long corax_fasta_getfilepos(corax_fasta_t *fd);

  CORAX_EXPORT int corax_fasta_rewind(corax_fasta_t *fd);

  corax_msa_t *corax_fasta_load(const char *fname);

  /* functions in phylip.c */

  CORAX_EXPORT void corax_msa_destroy(corax_msa_t *msa);

  CORAX_EXPORT corax_phylip_t *corax_phylip_open(const char *        filename,
                                                 const unsigned int *map);

  CORAX_EXPORT int corax_phylip_rewind(corax_phylip_t *fd);

  CORAX_EXPORT void corax_phylip_close(corax_phylip_t *fd);

  CORAX_EXPORT corax_msa_t *corax_phylip_parse_interleaved(corax_phylip_t *fd);

  CORAX_EXPORT corax_msa_t *corax_phylip_parse_sequential(corax_phylip_t *fd);

  CORAX_EXPORT corax_msa_t *corax_phylip_load(const char * fname,
                                              corax_bool_t interleaved);

  CORAX_EXPORT int corax_phylip_save(const char *       out_fname,
                                     const corax_msa_t *msa);

  /* functions in core_clvs.c */

  CORAX_EXPORT void corax_core_create_lookup(unsigned int         states,
                                             unsigned int         rate_cats,
                                             double *             lookup,
                                             const double *       left_matrix,
                                             const double *       right_matrix,
                                             const corax_state_t *tipmap,
                                             unsigned int         tipmap_size,
                                             unsigned int         attrib);

  CORAX_EXPORT void
  corax_core_update_clv_tt(unsigned int         states,
                           unsigned int         sites,
                           unsigned int         rate_cats,
                           double *             parent_clv,
                           unsigned int *       parent_scaler,
                           const unsigned char *left_tipchars,
                           const unsigned char *right_tipchars,
                           const corax_state_t *tipmap,
                           unsigned int         tipmap_size,
                           const double *       lookup,
                           unsigned int         attrib);

  CORAX_EXPORT void corax_core_update_clv_ti(unsigned int         states,
                                             unsigned int         sites,
                                             unsigned int         rate_cats,
                                             double *             parent_clv,
                                             unsigned int *       parent_scaler,
                                             const unsigned char *left_tipchars,
                                             const double *       right_clv,
                                             const double *       left_matrix,
                                             const double *       right_matrix,
                                             const unsigned int * right_scaler,
                                             const corax_state_t *tipmap,
                                             unsigned int         tipmap_size,
                                             unsigned int         attrib);

  CORAX_EXPORT void corax_core_update_clv_ii(unsigned int        states,
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

  CORAX_EXPORT void
  corax_core_update_clv_repeats(unsigned int        states,
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

  CORAX_EXPORT void
  corax_core_update_clv_repeats_generic(unsigned int        states,
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

  CORAX_EXPORT void
  corax_core_update_clv_repeatsbclv_generic(unsigned int        states,
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

  CORAX_EXPORT void corax_core_create_lookup_4x4(unsigned int  rate_cats,
                                                 double *      lookup,
                                                 const double *left_matrix,
                                                 const double *right_matrix);

  CORAX_EXPORT void
  corax_core_update_clv_tt_4x4(unsigned int         sites,
                               unsigned int         rate_cats,
                               double *             parent_clv,
                               unsigned int *       parent_scaler,
                               const unsigned char *left_tipchars,
                               const unsigned char *right_tipchars,
                               const double *       lookup,
                               unsigned int         attrib);

  CORAX_EXPORT void
  corax_core_update_clv_ti_4x4(unsigned int         sites,
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

  CORAX_EXPORT int
  corax_core_update_sumtable_repeats(unsigned int        states,
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

  CORAX_EXPORT int
  corax_core_update_sumtable_repeats_generic(unsigned int        states,
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
  CORAX_EXPORT int
  corax_core_update_sumtable_ti_4x4(unsigned int         sites,
                                    unsigned int         rate_cats,
                                    const double *       parent_clv,
                                    const unsigned char *left_tipchars,
                                    const unsigned int * parent_scaler,
                                    double *const *      eigenvecs,
                                    double *const *      inv_eigenvecs,
                                    double *const *      freqs,
                                    double *             sumtable,
                                    unsigned int         attrib);

  CORAX_EXPORT int
  corax_core_update_sumtable_ii(unsigned int        states,
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

  CORAX_EXPORT int
  corax_core_update_sumtable_ti(unsigned int         states,
                                unsigned int         sites,
                                unsigned int         rate_cats,
                                const double *       parent_clv,
                                const unsigned char *left_tipchars,
                                const unsigned int * parent_scaler,
                                double *const *      eigenvecs,
                                double *const *      inv_eigenvecs,
                                double *const *      freqs,
                                const corax_state_t *tipmap,
                                unsigned int         tipmap_size,
                                double *             sumtable,
                                unsigned int         attrib);

  CORAX_EXPORT int
  corax_core_likelihood_derivatives(unsigned int        states,
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

  CORAX_EXPORT int
  corax_core_update_sumtable_repeats_avx(unsigned int        states,
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

  CORAX_EXPORT double
  corax_core_edge_loglikelihood_ii(unsigned int        states,
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

  CORAX_EXPORT double
  corax_core_edge_loglikelihood_ti(unsigned int         states,
                                   unsigned int         sites,
                                   unsigned int         rate_cats,
                                   const double *       parent_clv,
                                   const unsigned int * parent_scaler,
                                   const unsigned char *tipchars,
                                   const corax_state_t *tipmap,
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

  CORAX_EXPORT double
  corax_core_edge_loglikelihood_ti_4x4(unsigned int         sites,
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

  CORAX_EXPORT double
  corax_core_root_loglikelihood_repeats(unsigned int        states,
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

  CORAX_EXPORT double
  corax_core_edge_loglikelihood_repeats(unsigned int        states,
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

  CORAX_EXPORT double corax_core_edge_loglikelihood_repeats_generic(
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

  CORAX_EXPORT double
  corax_core_root_loglikelihood(unsigned int        states,
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
  CORAX_EXPORT void corax_core_create_lookup_sse(unsigned int  states,
                                                 unsigned int  rate_cats,
                                                 double *      ttlookup,
                                                 const double *left_matrix,
                                                 const double *right_matrix,
                                                 const corax_state_t *tipmap,
                                                 unsigned int tipmap_size);

  CORAX_EXPORT void
  corax_core_create_lookup_4x4_sse(unsigned int  rate_cats,
                                   double *      lookup,
                                   const double *left_matrix,
                                   const double *right_matrix);

  CORAX_EXPORT void
  corax_core_update_clv_tt_sse(unsigned int         states,
                               unsigned int         sites,
                               unsigned int         rate_cats,
                               double *             parent_clv,
                               unsigned int *       parent_scaler,
                               const unsigned char *left_tipchars,
                               const unsigned char *right_tipchars,
                               const double *       lookup,
                               unsigned int         tipstates_count,
                               unsigned int         attrib);

  CORAX_EXPORT void
  corax_core_update_clv_tt_4x4_sse(unsigned int         sites,
                                   unsigned int         rate_cats,
                                   double *             parent_clv,
                                   unsigned int *       parent_scaler,
                                   const unsigned char *left_tipchars,
                                   const unsigned char *right_tipchars,
                                   const double *       lookup,
                                   unsigned int         attrib);

  CORAX_EXPORT void
  corax_core_update_clv_ti_sse(unsigned int         states,
                               unsigned int         sites,
                               unsigned int         rate_cats,
                               double *             parent_clv,
                               unsigned int *       parent_scaler,
                               const unsigned char *left_tipchars,
                               const double *       right_clv,
                               const double *       left_matrix,
                               const double *       right_matrix,
                               const unsigned int * right_scaler,
                               const corax_state_t *tipmap,
                               unsigned int         tipmap_size,
                               unsigned int         attrib);

  CORAX_EXPORT void
  corax_core_update_clv_ti_4x4_sse(unsigned int         sites,
                                   unsigned int         rate_cats,
                                   double *             parent_clv,
                                   unsigned int *       parent_scaler,
                                   const unsigned char *left_tipchar,
                                   const double *       right_clv,
                                   const double *       left_matrix,
                                   const double *       right_matrix,
                                   const unsigned int * right_scaler,
                                   unsigned int         attrib);

  CORAX_EXPORT void
  corax_core_update_clv_ii_sse(unsigned int        states,
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

  CORAX_EXPORT void
  corax_core_update_clv_ii_4x4_sse(unsigned int        sites,
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

  CORAX_EXPORT void
  corax_core_update_clv_repeats_generic_sse(unsigned int        states,
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
  CORAX_EXPORT void corax_core_create_lookup_avx(unsigned int  states,
                                                 unsigned int  rate_cats,
                                                 double *      lookup,
                                                 const double *left_matrix,
                                                 const double *right_matrix,
                                                 const corax_state_t *tipmap,
                                                 unsigned int tipmap_size);

  CORAX_EXPORT void
  corax_core_create_lookup_4x4_avx(unsigned int  rate_cats,
                                   double *      lookup,
                                   const double *left_matrix,
                                   const double *right_matrix);

  CORAX_EXPORT void
  corax_core_create_lookup_20x20_avx(unsigned int         rate_cats,
                                     double *             ttlookup,
                                     const double *       left_matrix,
                                     const double *       right_matrix,
                                     const corax_state_t *tipmap,
                                     unsigned int         tipmap_size);

  CORAX_EXPORT void
  corax_core_update_clv_tt_avx(unsigned int         states,
                               unsigned int         sites,
                               unsigned int         rate_cats,
                               double *             parent_clv,
                               unsigned int *       parent_scaler,
                               const unsigned char *left_tipchars,
                               const unsigned char *right_tipchars,
                               const double *       lookup,
                               unsigned int         tipstates_count,
                               unsigned int         attrib);

  CORAX_EXPORT void
  corax_core_update_clv_tt_4x4_avx(unsigned int         sites,
                                   unsigned int         rate_cats,
                                   double *             parent_clv,
                                   unsigned int *       parent_scaler,
                                   const unsigned char *left_tipchars,
                                   const unsigned char *right_tipchars,
                                   const double *       lookup,
                                   unsigned int         attrib);

  CORAX_EXPORT void
  corax_core_update_clv_ti_avx(unsigned int         states,
                               unsigned int         sites,
                               unsigned int         rate_cats,
                               double *             parent_clv,
                               unsigned int *       parent_scaler,
                               const unsigned char *left_tipchars,
                               const double *       right_clv,
                               const double *       left_matrix,
                               const double *       right_matrix,
                               const unsigned int * right_scaler,
                               const corax_state_t *tipmap,
                               unsigned int         tipmap_size,
                               unsigned int         attrib);

  CORAX_EXPORT void
  corax_core_update_clv_ti_4x4_avx(unsigned int         sites,
                                   unsigned int         rate_cats,
                                   double *             parent_clv,
                                   unsigned int *       parent_scaler,
                                   const unsigned char *left_tipchar,
                                   const double *       right_clv,
                                   const double *       left_matrix,
                                   const double *       right_matrix,
                                   const unsigned int * right_scaler,
                                   unsigned int         attrib);

  CORAX_EXPORT void
  corax_core_update_clv_ti_20x20_avx(unsigned int         sites,
                                     unsigned int         rate_cats,
                                     double *             parent_clv,
                                     unsigned int *       parent_scaler,
                                     const unsigned char *left_tipchar,
                                     const double *       right_clv,
                                     const double *       left_matrix,
                                     const double *       right_matrix,
                                     const unsigned int * right_scaler,
                                     const corax_state_t *tipmap,
                                     unsigned int         tipmap_size,
                                     unsigned int         attrib);

  CORAX_EXPORT void
  corax_core_update_clv_ii_avx(unsigned int        states,
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

  CORAX_EXPORT void
  corax_core_update_clv_ii_4x4_avx(unsigned int        sites,
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

  CORAX_EXPORT void
  corax_core_update_clv_repeats_generic_avx(unsigned int        states,
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

  CORAX_EXPORT void
  corax_core_update_clv_repeats_4x4_avx(unsigned int        states,
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

  CORAX_EXPORT void
  corax_core_update_clv_repeatsbclv_4x4_avx(unsigned int        states,
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

  CORAX_EXPORT void corax_core_update_clv_repeatsbclv_generic_avx(
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
  CORAX_EXPORT void
  corax_core_update_clv_ti_avx2(unsigned int         states,
                                unsigned int         sites,
                                unsigned int         rate_cats,
                                double *             parent_clv,
                                unsigned int *       parent_scaler,
                                const unsigned char *left_tipchars,
                                const double *       right_clv,
                                const double *       left_matrix,
                                const double *       right_matrix,
                                const unsigned int * right_scaler,
                                const corax_state_t *tipmap,
                                unsigned int         tipmap_size,
                                unsigned int         attrib);

  CORAX_EXPORT
  void corax_core_update_clv_ti_20x20_avx2(unsigned int         sites,
                                           unsigned int         rate_cats,
                                           double *             parent_clv,
                                           unsigned int *       parent_scaler,
                                           const unsigned char *left_tipchar,
                                           const double *       right_clv,
                                           const double *       left_matrix,
                                           const double *       right_matrix,
                                           const unsigned int * right_scaler,
                                           const corax_state_t *tipmap,
                                           unsigned int         tipmap_size,
                                           unsigned int         attrib);

  CORAX_EXPORT void
  corax_core_update_clv_ii_avx2(unsigned int        states,
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

  CORAX_EXPORT void
  corax_core_update_clv_repeats_generic_avx2(unsigned int        states,
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
  CORAX_EXPORT int
  corax_core_update_sumtable_ii_sse(unsigned int        states,
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

  CORAX_EXPORT int
  corax_core_update_sumtable_ti_sse(unsigned int         states,
                                    unsigned int         sites,
                                    unsigned int         rate_cats,
                                    const double *       parent_clv,
                                    const unsigned char *left_tipchars,
                                    const unsigned int * parent_scaler,
                                    double *const *      eigenvecs,
                                    double *const *      inv_eigenvecs,
                                    double *const *      freqs,
                                    const corax_state_t *tipmap,
                                    double *             sumtable,
                                    unsigned int         attrib);

  CORAX_EXPORT int corax_core_update_sumtable_repeats_generic_sse(
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

  CORAX_EXPORT int
  corax_core_update_sumtable_ii_avx(unsigned int        states,
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

  CORAX_EXPORT int
  corax_core_update_sumtable_ti_avx(unsigned int         states,
                                    unsigned int         sites,
                                    unsigned int         rate_cats,
                                    const double *       parent_clv,
                                    const unsigned char *left_tipchars,
                                    const unsigned int * parent_scaler,
                                    double *const *      eigenvecs,
                                    double *const *      inv_eigenvecs,
                                    double *const *      freqs,
                                    const corax_state_t *tipmap,
                                    unsigned int         tipmap_size,
                                    double *             sumtable,
                                    unsigned int         attrib);

  CORAX_EXPORT int
  corax_core_likelihood_derivatives_avx(unsigned int        states,
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

  CORAX_EXPORT int corax_core_update_sumtable_repeats_generic_avx(
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
  CORAX_EXPORT int
  corax_core_update_sumtable_repeats_4x4_avx(unsigned int        states,
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
  CORAX_EXPORT int corax_core_update_sumtable_repeatsbclv_4x4_avx(
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

  CORAX_EXPORT int
  corax_core_update_sumtable_ii_avx2(unsigned int        states,
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

  CORAX_EXPORT int
  corax_core_update_sumtable_ti_avx2(unsigned int         states,
                                     unsigned int         sites,
                                     unsigned int         rate_cats,
                                     const double *       parent_clv,
                                     const unsigned char *left_tipchars,
                                     const unsigned int * parent_scaler,
                                     double *const *      eigenvecs,
                                     double *const *      inv_eigenvecs,
                                     double *const *      freqs,
                                     const corax_state_t *tipmap,
                                     unsigned int         tipmap_size,
                                     double *             sumtable,
                                     unsigned int         attrib);

  CORAX_EXPORT
  int corax_core_likelihood_derivatives_avx2(
      unsigned int        states,
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

  CORAX_EXPORT int corax_core_update_sumtable_repeats_generic_avx2(
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
  CORAX_EXPORT
  double
  corax_core_edge_loglikelihood_ii_sse(unsigned int        states,
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

  CORAX_EXPORT
  double
  corax_core_edge_loglikelihood_ii_4x4_sse(unsigned int        sites,
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

  CORAX_EXPORT
  double
  corax_core_edge_loglikelihood_ti_sse(unsigned int         states,
                                       unsigned int         sites,
                                       unsigned int         rate_cats,
                                       const double *       parent_clv,
                                       const unsigned int * parent_scaler,
                                       const unsigned char *tipchars,
                                       const corax_state_t *tipmap,
                                       const double *       pmatrix,
                                       double *const *      frequencies,
                                       const double *       rate_weights,
                                       const unsigned int * pattern_weights,
                                       const double *       invar_proportion,
                                       const int *          invar_indices,
                                       const unsigned int * freqs_indices,
                                       double *             persite_lnl,
                                       unsigned int         attrib);

  CORAX_EXPORT
  double
  corax_core_edge_loglikelihood_ti_4x4_sse(unsigned int         sites,
                                           unsigned int         rate_cats,
                                           const double *       parent_clv,
                                           const unsigned int * parent_scaler,
                                           const unsigned char *tipchars,
                                           const double *       pmatrix,
                                           double *const *      frequencies,
                                           const double *       rate_weights,
                                           const unsigned int * pattern_weights,
                                           const double *      invar_proportion,
                                           const int *         invar_indices,
                                           const unsigned int *freqs_indices,
                                           double *            persite_lnl,
                                           unsigned int        attrib);

  CORAX_EXPORT double
  corax_core_root_loglikelihood_4x4_sse(unsigned int        sites,
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

  CORAX_EXPORT double
  corax_core_root_loglikelihood_sse(unsigned int        states,
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

  CORAX_EXPORT double
  corax_core_root_loglikelihood_repeats_sse(unsigned int        states,
                                            unsigned int        sites,
                                            unsigned int        rate_cats,
                                            const double *      clv,
                                            const unsigned int *site_id,
                                            const unsigned int *scaler,
                                            double *const *     frequencies,
                                            const double *      rate_weights,
                                            const unsigned int *pattern_weights,
                                            const double *invar_proportion,
                                            const int *   invar_indices,
                                            const unsigned int *freqs_indices,
                                            double *            persite_lnl);

  CORAX_EXPORT double corax_core_edge_loglikelihood_repeats_generic_sse(
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
  CORAX_EXPORT double
  corax_core_edge_loglikelihood_ii_avx(unsigned int        states,
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

  CORAX_EXPORT double
  corax_core_edge_loglikelihood_ii_4x4_avx(unsigned int        sites,
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

  CORAX_EXPORT double
  corax_core_edge_loglikelihood_ti_4x4_avx(unsigned int         sites,
                                           unsigned int         rate_cats,
                                           const double *       parent_clv,
                                           const unsigned int * parent_scaler,
                                           const unsigned char *tipchars,
                                           const double *       pmatrix,
                                           double *const *      frequencies,
                                           const double *       rate_weights,
                                           const unsigned int * pattern_weights,
                                           const double *      invar_proportion,
                                           const int *         invar_indices,
                                           const unsigned int *freqs_indices,
                                           double *            persite_lnl,
                                           unsigned int        attrib);

  CORAX_EXPORT double corax_core_edge_loglikelihood_ti_20x20_avx(
      unsigned int         sites,
      unsigned int         rate_cats,
      const double *       parent_clv,
      const unsigned int * parent_scaler,
      const unsigned char *tipchars,
      const corax_state_t *tipmap,
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

  CORAX_EXPORT double
  corax_core_edge_loglikelihood_ti_avx(unsigned int         states,
                                       unsigned int         sites,
                                       unsigned int         rate_cats,
                                       const double *       parent_clv,
                                       const unsigned int * parent_scaler,
                                       const unsigned char *tipchars,
                                       const corax_state_t *tipmap,
                                       const double *       pmatrix,
                                       double *const *      frequencies,
                                       const double *       rate_weights,
                                       const unsigned int * pattern_weights,
                                       const double *       invar_proportion,
                                       const int *          invar_indices,
                                       const unsigned int * freqs_indices,
                                       double *             persite_lnl,
                                       unsigned int         attrib);

  CORAX_EXPORT double
  corax_core_root_loglikelihood_4x4_avx(unsigned int        sites,
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

  CORAX_EXPORT double
  corax_core_root_loglikelihood_avx(unsigned int        states,
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

  CORAX_EXPORT double
  corax_core_root_loglikelihood_repeats_avx(unsigned int        states,
                                            unsigned int        sites,
                                            unsigned int        rate_cats,
                                            const double *      clv,
                                            const unsigned int *site_id,
                                            const unsigned int *scaler,
                                            double *const *     frequencies,
                                            const double *      rate_weights,
                                            const unsigned int *pattern_weights,
                                            const double *invar_proportion,
                                            const int *   invar_indices,
                                            const unsigned int *freqs_indices,
                                            double *            persite_lnl);

  CORAX_EXPORT double corax_core_edge_loglikelihood_repeats_generic_avx(
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

  CORAX_EXPORT double corax_core_edge_loglikelihood_repeats_4x4_avx(
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

  CORAX_EXPORT double corax_core_edge_loglikelihood_repeatsbclv_4x4_avx(
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
  CORAX_EXPORT
  double corax_core_root_loglikelihood_avx2(unsigned int        states,
                                            unsigned int        sites,
                                            unsigned int        rate_cats,
                                            const double *      clv,
                                            const unsigned int *scaler,
                                            double *const *     frequencies,
                                            const double *      rate_weights,
                                            const unsigned int *pattern_weights,
                                            const double *invar_proportion,
                                            const int *   invar_indices,
                                            const unsigned int *freqs_indices,
                                            double *            persite_lnl);

  CORAX_EXPORT
  double corax_core_edge_loglikelihood_ti_20x20_avx2(
      unsigned int         sites,
      unsigned int         rate_cats,
      const double *       parent_clv,
      const unsigned int * parent_scaler,
      const unsigned char *tipchars,
      const corax_state_t *tipmap,
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

  CORAX_EXPORT
  double
  corax_core_edge_loglikelihood_ii_avx2(unsigned int        states,
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

  CORAX_EXPORT
  double corax_core_root_loglikelihood_repeats_avx2(
      unsigned int        states,
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

  CORAX_EXPORT
  double corax_core_edge_loglikelihood_repeats_generic_avx2(
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

  CORAX_EXPORT int corax_core_update_pmatrix(double **           pmatrix,
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
  CORAX_EXPORT int
  corax_core_update_pmatrix_20x20_avx2(double **           pmatrix,
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
  CORAX_EXPORT int
  corax_core_update_pmatrix_4x4_avx(double **           pmatrix,
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

  CORAX_EXPORT int
  corax_core_update_pmatrix_20x20_avx(double **           pmatrix,
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
  CORAX_EXPORT int
  corax_core_update_pmatrix_4x4_sse(double **           pmatrix,
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

  CORAX_EXPORT int
  corax_core_update_pmatrix_20x20_sse(double **           pmatrix,
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
   * `corax_msa_t`.
   *
   * @param map The sequence encoding map. For example, `corax_map_nt`.
   *
   * @param count The number of sequences, also the number of tips, also the
   * number of taxa.
   *
   * @param[out] length The length of the compressed alignment.
   *
   * @ingroup corax_partition_t
   */
  CORAX_EXPORT unsigned int *corax_compress_site_patterns(
      char **sequence, const corax_state_t *map, int count, int *length);

  CORAX_EXPORT
  unsigned int *
  corax_compress_site_patterns_msa(corax_msa_t *        msa,
                                   const corax_state_t *map,
                                   unsigned int *       site_pattern_map);

  /* functions in parsimony.c */

  CORAX_EXPORT int corax_set_parsimony_sequence(corax_parsimony_t *  pars,
                                                unsigned int         tip_index,
                                                const corax_state_t *map,
                                                const char *         sequence);

  CORAX_EXPORT corax_parsimony_t *
               corax_parsimony_create(unsigned int  tips,
                                      unsigned int  states,
                                      unsigned int  sites,
                                      const double *score_matrix,
                                      unsigned int  score_buffers,
                                      unsigned int  ancestral_buffers);

  CORAX_EXPORT double
  corax_parsimony_build(corax_parsimony_t *         pars,
                        const corax_pars_buildop_t *operations,
                        unsigned int                count);

  CORAX_EXPORT void
  corax_parsimony_reconstruct(corax_parsimony_t *       pars,
                              const corax_state_t *     map,
                              const corax_pars_recop_t *operations,
                              unsigned int              count);

  CORAX_EXPORT double corax_parsimony_score(corax_parsimony_t *pars,
                                            unsigned int score_buffer_index);

  CORAX_EXPORT void corax_parsimony_destroy(corax_parsimony_t *pars);

  /* functions in fast_parsimony.c */

  CORAX_EXPORT corax_parsimony_t *
               corax_fastparsimony_init(const corax_partition_t *partition);

  CORAX_EXPORT void
  corax_fastparsimony_update_vectors(corax_parsimony_t *         parsimony,
                                     const corax_pars_buildop_t *ops,
                                     unsigned int                count);

  CORAX_EXPORT unsigned int
  corax_fastparsimony_root_score(const corax_parsimony_t *parsimony,
                                 unsigned int             root_index);

  CORAX_EXPORT unsigned int
  corax_fastparsimony_edge_score(const corax_parsimony_t *parsimony,
                                 unsigned int             node1_score_index,
                                 unsigned int             node2_score_index);

  CORAX_EXPORT void
  corax_fastparsimony_update_vector_4x4(corax_parsimony_t *         parsimony,
                                        const corax_pars_buildop_t *op);

  CORAX_EXPORT unsigned int
  corax_fastparsimony_edge_score_4x4(const corax_parsimony_t *parsimony,
                                     unsigned int             node1_score_index,
                                     unsigned int node2_score_index);

  CORAX_EXPORT void
  corax_fastparsimony_update_vector(corax_parsimony_t *         parsimony,
                                    const corax_pars_buildop_t *op);

  /* functions in fast_parsimony_sse.c */

  CORAX_EXPORT void
  corax_fastparsimony_update_vector_4x4_sse(corax_parsimony_t *parsimony,
                                            const corax_pars_buildop_t *op);

  CORAX_EXPORT unsigned int
  corax_fastparsimony_edge_score_4x4_sse(const corax_parsimony_t *parsimony,
                                         unsigned int node1_score_index,
                                         unsigned int node2_score_index);

  CORAX_EXPORT unsigned int
  corax_fastparsimony_edge_score_sse(const corax_parsimony_t *parsimony,
                                     unsigned int             node1_score_index,
                                     unsigned int node2_score_index);

  CORAX_EXPORT void
  corax_fastparsimony_update_vector_sse(corax_parsimony_t *         parsimony,
                                        const corax_pars_buildop_t *op);

  /* functions in fast_parsimony_avx.c */

  CORAX_EXPORT void
  corax_fastparsimony_update_vector_4x4_avx(corax_parsimony_t *parsimony,
                                            const corax_pars_buildop_t *op);

  CORAX_EXPORT unsigned int
  corax_fastparsimony_edge_score_4x4_avx(const corax_parsimony_t *parsimony,
                                         unsigned int node1_score_index,
                                         unsigned int node2_score_index);

  CORAX_EXPORT void
  corax_fastparsimony_update_vector_avx(corax_parsimony_t *         parsimony,
                                        const corax_pars_buildop_t *op);

  CORAX_EXPORT unsigned int
  corax_fastparsimony_edge_score_avx(const corax_parsimony_t *parsimony,
                                     unsigned int             node1_score_index,
                                     unsigned int node2_score_index);

  /* functions in fast_parsimony_avx2.c */

  CORAX_EXPORT void
  corax_fastparsimony_update_vector_4x4_avx2(corax_parsimony_t *parsimony,
                                             const corax_pars_buildop_t *op);

  CORAX_EXPORT unsigned int
  corax_fastparsimony_edge_score_4x4_avx2(const corax_parsimony_t *parsimony,
                                          unsigned int node1_score_index,
                                          unsigned int node2_score_index);

  CORAX_EXPORT void
  corax_fastparsimony_update_vector_avx2(corax_parsimony_t *         parsimony,
                                         const corax_pars_buildop_t *op);

  CORAX_EXPORT unsigned int
  corax_fastparsimony_edge_score_avx2(const corax_parsimony_t *parsimony,
                                      unsigned int node1_score_index,
                                      unsigned int node2_score_index);

  /* functions in stepwise.c */

  CORAX_EXPORT corax_utree_t *
               corax_fastparsimony_stepwise(corax_parsimony_t **list,
                                            char *const *       labels,
                                            unsigned int *      score,
                                            unsigned int        count,
                                            unsigned int        seed);

  /* functions in random.c */

  CORAX_EXPORT extern int corax_random_r(struct corax_random_data *__buf,
                                         int32_t *                 __result);

  CORAX_EXPORT extern int corax_srandom_r(unsigned int              __seed,
                                          struct corax_random_data *__buf);

  CORAX_EXPORT extern int corax_initstate_r(unsigned int __seed,
                                            char *       __statebuf,
                                            size_t       __statelen,
                                            struct corax_random_data *__buf);

  CORAX_EXPORT extern int corax_setstate_r(char *                    __statebuf,
                                           struct corax_random_data *__buf);

  CORAX_EXPORT corax_random_state *corax_random_create(unsigned int seed);

  CORAX_EXPORT int corax_random_getint(corax_random_state *rstate, int maxval);

  CORAX_EXPORT void corax_random_destroy(corax_random_state *rstate);

  /* functions in hardware.c */

  CORAX_EXPORT int corax_hardware_probe(void);

  CORAX_EXPORT void corax_hardware_dump(void);

  CORAX_EXPORT void corax_hardware_ignore(void);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
