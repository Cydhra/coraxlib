#ifndef CORAX_IO_UTREE_IO_H_
#define CORAX_IO_UTREE_IO_H_

#include "corax/corax_common.h"

/* functions in newick.cpp */

PLL_EXPORT pll_utree_t *pll_utree_parse_newick(const char *filename);

PLL_EXPORT pll_utree_t *pll_utree_parse_newick_rooted(const char *filename);

PLL_EXPORT pll_utree_t *pll_utree_parse_newick_unroot(const char *filename);

PLL_EXPORT pll_utree_t *pll_utree_parse_newick_string(const char *s);

PLL_EXPORT pll_utree_t *pll_utree_parse_newick_string_rooted(const char *s);

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
