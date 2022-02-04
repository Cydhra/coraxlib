/*
 Copyright (C) 2015-21 Diego Darriba, Alexey Kozlov

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

#include "corax/corax.h"
#include "opt_treeinfo.h"
#include "opt_branches.h"
#include <time.h>

// move structure
typedef struct move{

    int     type;
    double  initial_branch_lengths[5];
    unsigned int initial_pmatrix_indices[5];

} corax_nni_move;

// manual traversal implemented to update CLVs root to current note, before re-rooting the tree
static int traverse_to_root_recursive(corax_treeinfo_t *treeinfo, 
                                    corax_unode_t* node, 
                                    const corax_unode_t ** outbuffer, 
                                    unsigned int *traversal_size)
{
    
    if(CORAX_UTREE_IS_TIP(node)){ 
        return 0;
    } else {

        if(node == treeinfo->root || node->next == treeinfo->root || node->next->next == treeinfo->root){
            outbuffer[*traversal_size] = node->next->back;
            *traversal_size = *traversal_size + 1;

            outbuffer[*traversal_size] = node->next->next->back;
            *traversal_size = *traversal_size + 1;

            return 1;
        } else {

            if (traverse_to_root_recursive(treeinfo, node->next->back, outbuffer, traversal_size)){
                outbuffer[*traversal_size] = node->next->back;
                *traversal_size = *traversal_size + 1;

                outbuffer[*traversal_size] = node->next->next->back;
                *traversal_size = *traversal_size + 1;
                return 1;
            
            } else if (traverse_to_root_recursive(treeinfo, node->next->next->back, outbuffer, traversal_size)){
                outbuffer[*traversal_size] = node->next->next->back;
                *traversal_size = *traversal_size + 1;

                outbuffer[*traversal_size] = node->next->back;
                *traversal_size = *traversal_size + 1;
                return 1;

            } else {
                return 0;
            }
        }
    }

}

static void setup_move_info(corax_nni_move *move, corax_unode_t *node, int type){

    move->type = type;

    move->initial_branch_lengths[0] = node->next->length;
    move->initial_branch_lengths[1] = node->next->next->length;
    move->initial_branch_lengths[2] = node->length;
    move->initial_branch_lengths[3] = node->back->next->length;
    move->initial_branch_lengths[4] = node->back->next->next->length;

    move->initial_pmatrix_indices[0] = node->next->pmatrix_index;
    move->initial_pmatrix_indices[1] = node->next->next->pmatrix_index;
    move->initial_pmatrix_indices[2] = node->pmatrix_index;
    move->initial_pmatrix_indices[3] = node->back->next->pmatrix_index;
    move->initial_pmatrix_indices[4] = node->back->next->next->pmatrix_index;
    
}

// mode 0: traverse from node p to (previous) root and update CLVs - calculate likelihood
// mode 1: traverse local quartet - update local CLVs - calculate likelihood
// mode 2: traverse triplet (p->next->back, p->next->next->back, p->back), update CLV, calculate likelihood
static double compute_local_likelihood_treeinfo(corax_treeinfo_t* treeinfo, 
                                            corax_unode_t *p,
                                            int mode)
{
    
    assert(!CORAX_UTREE_IS_TIP(p));

    // declarations - initializations
    unsigned int ops_count,
                 matrix_count;
    unsigned int part;

    double       total_loglh          = 0.0;
    const int    old_active_partition = treeinfo->active_partition;
    
    corax_treeinfo_set_active_partition(treeinfo, CORAX_TREEINFO_PARTITION_ALL);

    // matrices
    corax_operation_t *operations_local;
    const corax_unode_t ** trav_buffer_local;
    unsigned int * matrix_indices;
    double *branch_lengths;
    unsigned int traversal_size = 0;
    unsigned int tip_nodes_count = treeinfo->partitions[0]->tips;
    
    // nodes
    corax_unode_t *q = p->back;

    if(mode == 0){

        trav_buffer_local = (const corax_unode_t **) malloc ((2*tip_nodes_count-2) * sizeof(corax_unode_t *));
        operations_local = (corax_operation_t*)malloc((tip_nodes_count-2)*sizeof(corax_operation_t));
        branch_lengths = (double*)malloc((2*tip_nodes_count-3)*sizeof(double));
        matrix_indices = (unsigned int*)malloc((2*tip_nodes_count-3)*sizeof(unsigned int));

        if (!traverse_to_root_recursive(treeinfo, p, trav_buffer_local, &traversal_size)){
            corax_set_error(CORAX_NNI_ROOT_NOT_FOUND,
                            "For some reason, the root of the tree in traverse_to_root_recursive() function was never found.\n");
            return CORAX_FAILURE;
            
        }

        trav_buffer_local[traversal_size] = p;
        traversal_size++;
    
    } else if(mode == 1){
        
        // manual traversal

        corax_unode_t *p_left = p->next->back;
        corax_unode_t *p_right = p->next->next->back;
        corax_unode_t *q_left = q->next->back;
        corax_unode_t *q_right = q->next->next->back;
        
        trav_buffer_local = (const corax_unode_t **) malloc (6* sizeof(corax_unode_t *));
        trav_buffer_local[0] = q_left;
        trav_buffer_local[1] = q_right;
        trav_buffer_local[2] = q;
        trav_buffer_local[3] = p_left;
        trav_buffer_local[4] = p_right;
        trav_buffer_local[5] = p;

        matrix_indices = (unsigned int*)malloc(5*sizeof(unsigned int));
        branch_lengths = (double*)malloc(5*sizeof(double));
        operations_local = (corax_operation_t*)malloc(6*sizeof(corax_operation_t));
        traversal_size = 6;

    } else if (mode == 2) {

        // manual traversal

        corax_unode_t *p_left = p->next->back;
        corax_unode_t *p_right = p->next->next->back;
        trav_buffer_local = (const corax_unode_t **) malloc (4* sizeof(corax_unode_t *));
        
        trav_buffer_local[0] = p_left;
        trav_buffer_local[1] = p_right;
        trav_buffer_local[2] = q;
        trav_buffer_local[3] = p;
        
        matrix_indices = (unsigned int*)malloc(3*sizeof(unsigned int));
        branch_lengths = (double*)malloc(3*sizeof(double));
        operations_local = (corax_operation_t*)malloc(4*sizeof(corax_operation_t));
        traversal_size = 4;
    }


    corax_utree_create_operations (trav_buffer_local, traversal_size, branch_lengths,
                                        matrix_indices, operations_local, &matrix_count,
                                        &ops_count);

    for (part = 0; part < treeinfo->partition_count; ++part){
        
        /* only selected partitioned will be affected */
        if (treeinfo->active_partition == CORAX_TREEINFO_PARTITION_ALL
            || treeinfo->active_partition == (int)part)
        {
            corax_update_prob_matrices (treeinfo->partitions[part], 
                                        treeinfo->param_indices[part], 
                                        matrix_indices, 
                                        branch_lengths,
                                        matrix_count);
        }
    }
    
    for (part = 0; part < treeinfo->partition_count; ++part)
    {
        if (!treeinfo->partitions[part])
        {
            /* this partition will be computed by another thread(s) */
            treeinfo->partition_loglh[part] = 0.0;
            continue;
        }

        /* all subsequent operation will affect current partition only */
        corax_treeinfo_set_active_partition(treeinfo, (int)part);

        /* use the operations array to compute all ops_count inner CLVs. Operations
        will be carried out sequentially starting from operation 0 towards
        ops_count-1 */
        corax_update_clvs(treeinfo->partitions[part], operations_local, ops_count);

        corax_treeinfo_validate_clvs(treeinfo, treeinfo->travbuffer, traversal_size);

        /* compute the likelihood on an edge of the unrooted tree by specifying
        the CLV indices at the two end-point of the branch, the probability
        matrix index for the concrete branch length, and the index of the model
        of whose frequency vector is to be used */
        treeinfo->partition_loglh[part] =
            corax_compute_edge_loglikelihood(treeinfo->partitions[part],
                                            p->clv_index,
                                            p->scaler_index,
                                            q->clv_index,
                                            q->scaler_index,
                                            p->pmatrix_index,
                                            treeinfo->param_indices[part],
                                            NULL);
    }

    free(trav_buffer_local);
    free(operations_local);
    free(branch_lengths);
    free(matrix_indices);

    if (treeinfo->parallel_reduce_cb)
    {
        treeinfo->parallel_reduce_cb(treeinfo->parallel_context,
                                    treeinfo->partition_loglh,
                                    part,
                                    CORAX_REDUCE_SUM);
    }

    for (part = 0; part < treeinfo->partition_count; ++part)
        total_loglh += treeinfo->partition_loglh[part];

    /* restore original active partition */
    corax_treeinfo_set_active_partition(treeinfo, old_active_partition);

    assert(total_loglh < 0.);

    return total_loglh;
}

static int optimize_branch_lengths_quartet(corax_treeinfo_t* treeinfo,
                                            corax_unode_t *p,
                                            double lh_epsilon,
                                            int smoothings)
{   
    int retval = CORAX_SUCCESS;
    double logl;
    
    logl = compute_local_likelihood_treeinfo(treeinfo, p, 1);

    // new optimization function - we need to hide that from the user I guess (or maybe create a wrapper function)
    logl = -1*corax_opt_optimize_branch_lengths_local_multi_quartet( treeinfo->partitions,
                                                                    treeinfo->partition_count,
                                                                    treeinfo->root,
                                                                    treeinfo->param_indices,
                                                                    treeinfo->deriv_precomp,
                                                                    treeinfo->branch_lengths,
                                                                    treeinfo->brlen_scalers,
                                                                    CORAX_OPT_MIN_BRANCH_LEN,
                                                                    CORAX_OPT_MAX_BRANCH_LEN,
                                                                    lh_epsilon,
                                                                    smoothings,
                                                                    1, // keep_update
                                                                    CORAX_OPT_BLO_NEWTON_FAST,
                                                                    treeinfo->brlen_linkage,
                                                                    treeinfo->parallel_context,
                                                                    treeinfo->parallel_reduce_cb);
    
    if (logl == CORAX_FAILURE){
        printf("Branch Length Optimization Error\n");
        printf("Corax Error num = %d\n", corax_errno);
        printf("Corax Error msg = %s\n", corax_errmsg);
        retval = CORAX_FAILURE;
    }

    return retval;
}

static double apply_move(corax_treeinfo_t* treeinfo,
                        corax_unode_t *node, 
                        corax_nni_move *move, 
                        int smoothings,
                        double lh_epsilon)
{

    if (move->type == CORAX_UTREE_MOVE_NNI_LEFT || move->type == CORAX_UTREE_MOVE_NNI_RIGHT){
        
        if (CORAX_UTREE_IS_TIP(node) || CORAX_UTREE_IS_TIP(node->back))
        {
            /* invalid move */
            corax_set_error(CORAX_TREE_ERROR_NNI_LEAF,
                            "Attempting to apply NNI on a leaf branch\n");
            return CORAX_FAILURE;
        }

        corax_utree_nni(node, move->type, NULL);
        
        if(optimize_branch_lengths_quartet(treeinfo, node, lh_epsilon, smoothings) != CORAX_FAILURE)
            return compute_local_likelihood_treeinfo(treeinfo, node, 2);
       
        else{
            printf("Something went wrong with the branch-length optimization. Exit..\n");
            return CORAX_FAILURE;
        }


    } else {

        /* invalid move */
        corax_set_error(CORAX_TREE_ERROR_NNI_INVALID_MOVE, "Invalid NNI move type\n");
        return CORAX_FAILURE;
        
    }

}


static int undo_move(corax_treeinfo_t* treeinfo, corax_unode_t *node, corax_nni_move *move){
    
    
    int retval = CORAX_SUCCESS;

    retval = corax_utree_nni(node, move->type, NULL);

    node->next->pmatrix_index = node->next->back->pmatrix_index = move->initial_pmatrix_indices[0];
    node->next->length = node->next->back->length = move->initial_branch_lengths[0];
    treeinfo->branch_lengths[0][node->next->pmatrix_index] = move->initial_branch_lengths[0];

    node->next->next->pmatrix_index = node->next->next->back->pmatrix_index = move->initial_pmatrix_indices[1];
    node->next->next->length = node->next->next->back->length = move->initial_branch_lengths[1];
    treeinfo->branch_lengths[0][node->next->next->pmatrix_index] = move->initial_branch_lengths[1];

    node->length = node->back->length = move->initial_branch_lengths[2];
    treeinfo->branch_lengths[0][node->pmatrix_index] = move->initial_branch_lengths[2];

    node->back->next->pmatrix_index = node->back->next->back->pmatrix_index = move->initial_pmatrix_indices[3];
    node->back->next->length = node->back->next->back->length = move->initial_branch_lengths[3];
    treeinfo->branch_lengths[0][node->back->next->pmatrix_index] = move->initial_branch_lengths[3];

    node->back->next->next->pmatrix_index = node->back->next->next->back->pmatrix_index = move->initial_pmatrix_indices[4];
    node->back->next->next->length = node->back->next->next->back->length = move->initial_branch_lengths[4];
    treeinfo->branch_lengths[0][node->back->next->next->pmatrix_index] = move->initial_branch_lengths[4];

    return retval;
}


CORAX_EXPORT double corax_algo_nni_local(corax_treeinfo_t *treeinfo,
                                          int    smoothings,
                                          double lh_epsilon)
{

    double tree_logl;
    double test_logl;
    double new_logl;

    corax_unode_t *q = treeinfo->root;
    corax_nni_move *move = (corax_nni_move *)malloc(sizeof(corax_nni_move));

    // setup move
    if(optimize_branch_lengths_quartet(treeinfo, q, lh_epsilon, smoothings) != CORAX_FAILURE){
        tree_logl = compute_local_likelihood_treeinfo(treeinfo, q, 2);
    }else {
        printf("Something went wrong with the branch-length optimization. Exit..\n");
        return CORAX_FAILURE;
    }
    
    // setup move
    setup_move_info(move, q, CORAX_UTREE_MOVE_NNI_LEFT);

    // apply nni move
    new_logl = apply_move(treeinfo, q, move, smoothings, lh_epsilon);
    //printf("New logl = %.5f\n", new_logl);

    if (new_logl == CORAX_FAILURE){

        printf("Failed to apply NNI move. \n");
        printf("Corax error number %d \n", corax_errno);
        printf("Corax error msg: %s\n", corax_errmsg);
        free(move);
        return CORAX_FAILURE;
    }

    if (new_logl > tree_logl){
        
        // update move structure
        setup_move_info(move, q, CORAX_UTREE_MOVE_NNI_RIGHT);
        tree_logl = new_logl;

        new_logl = apply_move(treeinfo, q, move, smoothings, lh_epsilon);

        if (new_logl == CORAX_FAILURE){

            printf("Failed to apply NNI move. \n");
            printf("Error number %d \n", corax_errno);
            printf("Error msg: %s\n", corax_errmsg);
            free(move);
            return CORAX_FAILURE;
        }

        if (new_logl > tree_logl){
            
            tree_logl = new_logl;
            free(move);

        } else {

            if(!undo_move(treeinfo, q, move)){
                corax_set_error(CORAX_NNI_ROUND_UNDO_MOVE_ERROR,
                        "Error in undoing move that generates a tree of worst likelihood ...\n");
                return CORAX_FAILURE;

            }

            test_logl = compute_local_likelihood_treeinfo(treeinfo, q, 1);
            assert(fabs(test_logl - tree_logl) < 1e-5);
            tree_logl = test_logl;
            free(move);

        }
    
    } else {

        if(!undo_move(treeinfo, q, move)){
                corax_set_error(CORAX_NNI_ROUND_UNDO_MOVE_ERROR,
                        "Error in undoing move that generates a tree of worst likelihood ...\n");
                return CORAX_FAILURE;
        }

        test_logl = compute_local_likelihood_treeinfo(treeinfo, q, 1);
        assert(fabs(test_logl - tree_logl) < 1e-5);
        tree_logl = test_logl;

        // setup_move_info(move, q, new_logl, CORAX_UTREE_MOVE_NNI_RIGHT);
        move->type = CORAX_UTREE_MOVE_NNI_RIGHT;
        new_logl = apply_move(treeinfo, q, move, smoothings, lh_epsilon);

        if (new_logl == CORAX_FAILURE){
            
            printf("Failed to apply NNI move. \n");
            printf("Error number %d \n", corax_errno);
            printf("Error msg: %s\n", corax_errmsg);
            free(move);
            return CORAX_FAILURE;
        }

        if (new_logl > tree_logl){
            
            tree_logl = new_logl;
            free(move);
        
        } else {

            if(!undo_move(treeinfo, q, move)){
                corax_set_error(CORAX_NNI_ROUND_UNDO_MOVE_ERROR,
                        "Error in undoing move that generates a tree of worst likelihood ...\n");
                return CORAX_FAILURE;

            }

            test_logl = compute_local_likelihood_treeinfo(treeinfo, q, 1);
            assert(fabs(test_logl - tree_logl) < 1e-5);
            tree_logl = test_logl;
            free(move);

        }
    }

    return tree_logl;

}


static int nni_recursive(corax_treeinfo_t* treeinfo,
                        corax_unode_t *node,
                        unsigned int *interchagnes,
                        int smoothings,
                        double lh_epsilon,
                        int update_to_root)
{
    
    int retval = CORAX_SUCCESS;
    corax_unode_t *q =  node->back;
    corax_unode_t *pb1 = node->next->back, 
                    *pb2 = node->next->next->back;
    
    double tree_logl;
    double new_logl;

    if(!CORAX_UTREE_IS_TIP(q)){
        
        if (update_to_root){
            //tree_logl = corax_treeinfo_compute_loglh(treeinfo, 0);
            tree_logl = compute_local_likelihood_treeinfo(treeinfo, q, 0);
            treeinfo->root = q;
            //tree_logl = compute_local_likelihood(treeinfo, q, 2);
        } else {
            treeinfo->root = q;
            tree_logl = compute_local_likelihood_treeinfo(treeinfo, q, 2);
        }

        corax_unode_t *test_node = q->next->back;
        new_logl = corax_algo_nni_local(treeinfo, smoothings, lh_epsilon);
        
        if(new_logl == CORAX_FAILURE)
            return CORAX_FAILURE;

        assert(new_logl - tree_logl > -1e-5); // to avoid numerical errors
        
        if(q->next->back != test_node) (*interchagnes)++;

    }

    // pb2->back is gonna be the next root 
    // so the second time we call the function, we have to update CLVs from the previous root
    // up to the new root
    if(!CORAX_UTREE_IS_TIP(pb1) && retval)
        retval = nni_recursive(treeinfo, pb1, interchagnes, smoothings, lh_epsilon, false);
    
    if(!CORAX_UTREE_IS_TIP(pb2) && retval)
        retval = nni_recursive(treeinfo, pb2, interchagnes, smoothings, lh_epsilon, true);

    return retval;
}

static double algo_nni_round(corax_treeinfo_t *treeinfo,
                                double tolerance,
                                int    smoothings,
                                double lh_epsilon)
{
    
    int retval = CORAX_SUCCESS;
    unsigned int nniRounds = 0;
    int total_interchanges = 0;
    unsigned int interchanges;
    double tree_logl, diff, new_logl;
    unsigned int tip_index = 0;

    
    corax_utree_t *tree = treeinfo->tree;
    corax_unode_t *start_node;
    
    printf("\n\nNNI Round:\n");
    

    clock_t begin = clock(); // calculate nni time
    
    // define start node (the back node from leaf 0)
    start_node = tree->nodes[tip_index]->back;
    if (!CORAX_UTREE_IS_TIP(start_node->back))
    {
        corax_set_error(CORAX_NNI_ROUND_LEAF_ERROR,
                        "The NNI round is defined to start from a leaf node, and in particular this node is the first one in the `**nodes` list of your tree. "
                        "The first elements of this list are expected to be leaf nodes, but in this case, this does not seem to hold.\n");
        printf("Eroor: %s\n", corax_errmsg);
        return CORAX_FAILURE;
    }

    // set root, calculate likelihood, update CLVs in general
    treeinfo->root = start_node;
    tree_logl = corax_treeinfo_compute_loglh(treeinfo, 0);
    
    do{
        
        interchanges = 0;
        

        // perform nni moves
        retval = nni_recursive(treeinfo, start_node, &interchanges, smoothings, lh_epsilon, true);
        
        if (retval == CORAX_FAILURE){
            printf("\nSomething went wrong, the return value of NNI round is CORAX_FAILURE. Exit...\n");
            return CORAX_FAILURE;
        }
        total_interchanges += interchanges;

        // during the nni round, the root of the tree has changed
        // so we first traverse tree to its root, update clvs, and then set again root equal to start node.
        start_node = tree->nodes[tip_index]->back;
        new_logl = compute_local_likelihood_treeinfo(treeinfo, start_node, 0);
        treeinfo->root = start_node;

        diff = new_logl - tree_logl;
        
        // we check if (diff < -1e-7) and not (diff < 0) to avoid numerical errors
        if (diff < -1e-7) {
            printf("ERROR! It seems that tree likelihood after the NNI round is smaller than the initial likelihood\n");
            printf("Init logl = %.5f and new logl = %.5f \n", tree_logl, new_logl);
            printf("Round Interchanges = %d\n", interchanges);
            corax_set_error(CORAX_NNI_DIFF_NEGATIVE_ERROR,
                            "The logl after an NNI round was smaller than the initial likelihood.\n");
            return CORAX_FAILURE;
        }

        tree_logl = new_logl;
        nniRounds ++;
        printf("Round %d, interchanges = %d, logl = %.5f\n", nniRounds, interchanges, tree_logl);

    } while((diff > tolerance || nniRounds < 10) && interchanges != 0);
    
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    
    printf("\nCompleted.\nTotal interchanges = %d, Time = %.3f secs\n", total_interchanges, time_spent);
    printf("Final logl = %.5f\n\n", tree_logl);

    return tree_logl;

}

CORAX_EXPORT double corax_algo_nni_round(corax_treeinfo_t *treeinfo,
                            double tolerance,
                            int    smoothings,
                            double lh_epsilon)
{

    // Integrity check
    if(!corax_utree_check_integrity(treeinfo->tree)){
        corax_set_error(CORAX_NNI_ROUND_INTEGRITY_ERROR,
                        "Tree is not consistent...\n");
        return CORAX_FAILURE;
    }
    
    // If the tree is a triplet, NNI moves cannot be done
    if(treeinfo->tip_count == 3){
        corax_set_error(CORAX_NNI_ROUND_TRIPLET_ERROR,
                    "Tree is a triplet, NNI moves are not allowed ...\n");
        return CORAX_FAILURE;   
    }

    return algo_nni_round(treeinfo, 
                        tolerance, 
                        smoothings,
                        lh_epsilon);

}
