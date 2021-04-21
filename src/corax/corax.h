/*
    coraxlib - The Core RAxML Library

    Copyright (C) 2015-2021 Tomas Flouri, Diego Darriba, Alexey Kozlov,
                            Benoit Morel, Pierre Barbera, Ben Bettisworth
                            Sarah Lutteropp, Lukas Huebner

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

    Contact: Alexey Kozlov <Alexey.Kozlov@h-its.org>,
    Exelixis Lab, Heidelberg Instutute for Theoretical Studies
    Schloss-Wolfsbrunnenweg 35, D-69118 Heidelberg, Germany
*/

#ifndef CORAX_H_
#define CORAX_H_

#include "corax/corax_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include "corax/io/binary.h"
#include "corax/io/utree_io.h"

#include "corax/model/modutil.h"

#include "corax/optimize/opt_branches.h"
#include "corax/optimize/opt_model.h"
#include "corax/optimize/opt_treeinfo.h"

#include "corax/tree/treeinfo.h"
#include "corax/tree/utree.h"
#include "corax/tree/utree_moves.h"
#include "corax/tree/utree_traverse.h"
#include "corax/tree/utree_ops.h"
#include "corax/tree/utree_random.h"
#include "corax/tree/utree_split.h"
#include "corax/tree/utree_compare.h"
#include "corax/tree/utree_parsimony.h"
#include "corax/tree/utree_tbe.h"

#include "corax/util/msa.h"

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CORAX_H_ */
