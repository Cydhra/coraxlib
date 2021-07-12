/**
 * File containing the input and output operation for trees.
 */
#ifndef CORAX_IO_UTREE_IO_H_
#define CORAX_IO_UTREE_IO_H_

#include "corax/corax_common.h"

/**
 * This function will create a pll_utree_t from a newick _file_. If there is an
 * error, the function will return PLL_ERROR (which happens to be 0). If a file
 * containing a rooted tree is passed to this function, an error will be
 * generated, and parsing will fail.
 *
 * @ingroup pll_utree_t
 */
PLL_EXPORT pll_utree_t *pll_utree_parse_newick(const char *filename);

/**
 * This function will create a pll_utree_t from a newick _file_. If there is an
 * error, the function will return PLL_ERROR (which happens to be 0). The
 * tree contained in the file is allowed to be rooted or unrooted.
 *
 * @ingroup pll_utree_t
 */
PLL_EXPORT pll_utree_t *pll_utree_parse_newick_rooted(const char *filename);

/**
 * This function will create a pll_utree_t from a newick _file_. If there is an
 * error, the function will return PLL_ERROR (which happens to be 0). If a file
 * containing a rooted tree is passed to this function, it will **unroot** the
 * tree, and return a rooted tree.
 *
 * @ingroup pll_utree_t
 */
PLL_EXPORT pll_utree_t *pll_utree_parse_newick_unroot(const char *filename);

/**
 * This function will create a pll_utree_t from a newick _string_. If there is
 * an error, the function will return PLL_ERROR (which happens to be 0). If a
 * string containing a rooted tree is passed to this function, an error will be
 * generated, and parsing will fail.
 *
 * @ingroup pll_utree_t
 */
PLL_EXPORT pll_utree_t *pll_utree_parse_newick_string(const char *s);

/**
 * This function will create a pll_utree_t from a newick _string_. If there is
 * an error, the function will return PLL_ERROR (which happens to be 0). The
 * tree contained in the string is allowed to be rooted or unrooted.
 *
 * @ingroup pll_utree_t
 */
PLL_EXPORT pll_utree_t *pll_utree_parse_newick_string_rooted(const char *s);

/**
 * @ingroup pll_utree_t
 * This function will create a pll_utree_t from a newick _string_. If there is
 * an error, the function will return PLL_ERROR (which happens to be 0). If the
 * tree contained in the string is rooted, it will **unroot** the tree, and
 *
 * @ingroup pll_utree_t
 * return a rooted tree.
 */
PLL_EXPORT pll_utree_t *pll_utree_parse_newick_string_unroot(const char *s);

/* functions in utree_newick.c */

PLL_EXPORT char *
pll_utree_export_newick(const pll_unode_t *root,
                        char *(*cb_serialize)(const pll_unode_t *));

PLL_EXPORT char *pll_utree_export_newick_rooted(const pll_unode_t *root,
                                                double             root_brlen);

/* functions in utree_ascii.c */

PLL_EXPORT void pll_utree_show_ascii(const pll_unode_t *tree, int options);

/* functions in utree_svg.c */

PLL_EXPORT pll_svg_attrib_t *pll_svg_attrib_create(void);

PLL_EXPORT void pll_svg_attrib_destroy(pll_svg_attrib_t *attrib);

PLL_EXPORT int pll_utree_export_svg(pll_utree_t *           tree,
                                    pll_unode_t *           root,
                                    const pll_svg_attrib_t *attribs,
                                    const char *            filename);

#endif /* CORAX_IO_UTREE_IO_H_ */
