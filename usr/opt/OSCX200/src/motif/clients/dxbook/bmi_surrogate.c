/*
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 * HISTORY
 */
/* DEC/CMS REPLACEMENT HISTORY, Element BMI_SURROGATE.C*/
/* *5     9-JUN-1992 10:00:51 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *4     3-MAR-1992 17:08:05 KARDON "UCXed"*/
/* *3    12-FEB-1992 12:16:56 PARMENTER "i18n support:  #ifdef'ed asian"*/
/* *2     1-NOV-1991 12:48:24 BALLENGER "Reintegrate  memex support"*/
/* *1    16-SEP-1991 12:42:42 PARMENTER "Routines for dealing with surrogates"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BMI_SURROGATE.C*/
/*
***************************************************************
**  Copyright (c) Digital Equipment Corporation, 1988, 1990  **
**  All Rights Reserved.  Unpublished rights reserved	     **
**  under the copyright laws of the United States.  	     **
**  	    	    	    	    	    	    	     **
**  The software contained on this media is proprietary	     **
**  to and embodies the confidential technology of  	     **
**  Digital Equipment Corporation.  Possession, use,	     **
**  duplication or dissemination of the software and	     **
**  media is authorized only pursuant to a valid written     **
**  license from Digital Equipment Corporation.	    	     **
**  	    	    	    	    	    	    	     **
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 	     **
**  disclosure by the U.S. Government is subject to 	     **
**  restrictions as set forth in Subparagraph (c)(1)(ii)     **
**  of DFARS 252.227-7013, or in FAR 52.227-19, as  	     **
**  applicable.	    	    	    	    	    	     **
***************************************************************
*/


/*
**++
**  FACILITY:
**
**      Bookreader Memex Interfaces Interface (bmi*)
**
**  ABSTRACT:
**
**	Routines for dealing with surrogates.
**
**  AUTHORS:
**
**      David L Ballenger
**
**  CREATION DATE:     02-Jul-1990
**
**  MODIFICATION HISTORY:
**
**  V03-0002 DLB0002     David L Ballenger           30-Apr-1991
**           Fix problems with surrogates in the library window.
**
**  V03-0001 DLB0001     David L Ballenger           01-Mar-1991
**           Fix problems with surrogate highlighting after the composite
**           network changes for QAR 807.  LinkWorks naming convention changes.
**
**--
**/



/*
** Include Files
*/
#include <stdio.h>
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <DXm/DECspecific.h>
#include "br_common_defs.h"  /* common BR #defines */ 
#include "br_meta_data.h"    /* typedefs and #defines for I/O of BR files */
#include "br_typedefs.h"     /* BR high-level typedefs and #defines */
#include "br_malloc.h"       /* BKR_MALLOC, etc defined here */
#include "br_globals.h"      /* BR external variables declared here */
#include "bmi_private_defs.h"
#include "bmi_surrogate_defs.h"
#include "bmi_surrogate.h"

/*
** Local Variables
*/
static char *bmi_surrogate_subtype = BMI_SURROGATE_SUBTYPE;
        

void
bmi_print_surrogate PARAM_NAMES((surrogate))
    lwk_surrogate surrogate PARAM_END
/*
 *
 * Function description:
 *
 *      Utility routine to print the information in a bookreader
 *      surrogate object.
 *
 * Arguments:
 *
 *      surrogate - the surrogate that we are to print
 *
 *
 * Return value:
 *
 *      None
 *
 * Side effects:
 *
 *      None
 *
 */

{

    lwk_status status;
    lwk_string container = NULL;
    lwk_string type = NULL;
    lwk_string subtype;
    lwk_string string;
    lwk_integer id;

    /* Get the surrogate subtype property
     */
    status = GET_STRING(surrogate,lwk_c_p_surrogate_sub_type,&subtype);
    
    fprintf(stderr,"surrogate %8x:\n",surrogate);
    fprintf(stderr,"    subtype:  %s\n",subtype);

    /* Get the object type property so we will know what we ar dealing with.
     */
    status = GET_STRING(surrogate,BMI_PROP_OBJECT_TYPE,&type);
    fprintf(stderr,"    object type:  %s\n",type);

    /* All bookreader surrogate object have a container property, so go ahead
     * and get that.
     */
    status = GET_STRING(surrogate,BMI_PROP_CONTAINER,&container);
    fprintf(stderr,"    container:  %s\n",container);

    if (status != lwk_s_success) {

    } else if (strcmp(type,BMI_OBJ_NAME_BOOK) == 0) {
    } else if (strcmp(type,BMI_OBJ_NAME_DIRECTORY) == 0) {
        GET_STRING(surrogate,BMI_PROP_DIR_NAME,&string);
        fprintf(stderr,"    directory:  %s\n",string);
        GET_INTEGER(surrogate,BMI_PROP_DIR_ENTRY,&id);
        fprintf(stderr,"    entry id:  %u\n",id);
    } else if (strcmp(type,BMI_OBJ_NAME_CHUNK) == 0) {
        GET_STRING(surrogate,BMI_PROP_CHUNK_NAME,&string);
        fprintf(stderr,"    symbol:  %s\n",string);
        GET_INTEGER(surrogate,BMI_PROP_CHUNK_ID,&id);
        fprintf(stderr,"    chunk id:  %u\n",id);
    } else if (strcmp(type,BMI_OBJ_NAME_SHELF) == 0) {
        GET_STRING(surrogate,BMI_PROP_SHELF_PATH,&string);
        fprintf(stderr,"    shelf path:  %s\n",string);
        GET_INTEGER(surrogate,BMI_PROP_SHELF_ENTRY_ID,&id);
        fprintf(stderr,"    shelf entry id:     %d\n",id);
        GET_INTEGER(surrogate,BMI_PROP_SHELF_ENTRY_TYPE,&id);
        fprintf(stderr,"    shelf entry type:   %d\n",id);
        GET_STRING(surrogate,BMI_PROP_SHELF_ENTRY_TITLE,&string);
        fprintf(stderr,"    shelf entry title:  %s\n",string);
    } else {
        fprintf(stderr,"    ***** Unknown Type *****");
    }
} /* end apply */

lwk_status
bmi_save_surrogate_in_list PARAM_NAMES((list_head,surrogate))
    BMI_SURROGATE_LIST_PTR *list_head PARAM_SEP
    lwk_surrogate surrogate PARAM_END

/*
 *
 * Function description:
 *
 *      Saves a surrogate in a linked list of surrogates describing the
 *      same object.  This facilitates the get_surrogate callback and
 *      checking highlighting.
 *
 * Arguments:
 *
 *      list_head - the address of the a pointer to the first element
 *                  of the linked list.
 *
 *      surrogate - the surrogate to be stored in the list
 *
 * Return value:
 *
 *      Returns his_success on success, otherwise lwk_s_failure.
 *
 * Side effects:
 *
 *      Deletes the surrogate if it can' allocate memeory to save it
 *      in the list.
 */

{
    register BMI_SURROGATE_LIST_PTR list;
        
    /* Keep track of the new surrogate in the local surrogate list
     * to make checking some things easier.
     */
    list = (BMI_SURROGATE_LIST_PTR)BKR_MALLOC(sizeof(BMI_SURROGATE_LIST));
    if (list == NULL) {
        
        return lwk_s_failure;
    }

    /* Save the surrogate in the list and return the new list head.
     */
    list->surrogate = surrogate;
    if (*list_head == NULL) {
        list->next = NULL;
    } else {
        list->next = (*list_head)->next;
    }
    *list_head = list;
#ifdef MEMEX_DEBUG
    fprintf(stderr,"Saving surrogate %8x in list %8x\n",surrogate,list_head);
    bmi_print_surrogate(surrogate);
#endif 

    return lwk_s_success;
        
} /* end bmi_save_surrogate_in_list () */

lwk_status
bmi_save_surrogate_in_tree PARAM_NAMES((tree,surrogate,key))
    BMI_SURROGATE_TREE_PTR *tree PARAM_SEP
    lwk_surrogate surrogate PARAM_SEP
    unsigned key PARAM_END

/*
 *
 * Function description:
 *
 *      Saves a surrogate in a binary tree of surrogate lists.  This is
 *      useful for chunks and directory entries, where the number of
 *      objects is most likely much larger than the number of surrogates
 *      describing those objects.
 *
 * Arguments:
 *
 *      tree - The address of a pointer to the root of the tree, this may
 *             be updated if a newly created node becomes the root of the
 *             tree.
 *
 *      surrogate - the surrogate to be saved in the tree
 *
 *      key  - the key to base the tree insertion on.
 *
 * Return value:
 *
 *      Returns lwk_s_success if the insertion was successful, his_failure
 *      otherwise.
 *
 * Side effects:
 *
 *      None
 *
 */

{
    register BMI_SURROGATE_TREE_PTR node;
        
    /* Look for a node in the tree with a matching key.
     */
    node = *tree;
#ifdef MEMEX_DEBUG
    fprintf(stderr,
            "Saving surrogate %8x in tree %8x, key = %u, ",
            surrogate,tree,key);
#endif 
    while (node) {
        
        if (node->key == key) {

            /* There is a node with a matching key, so save the surrogate
             * in a linked list of surrogates with the same key and return
             * the status from that operation.
             */
#ifdef MEMEX_DEBUG
    fprintf(stderr,"node = %8x\n",node);
#endif 
            return bmi_save_surrogate_in_list(&node->list,surrogate);

        } else {

            /* If the key is les than the key of the current node then
             * follow the less pointer, else follow the more pointer.
             * Note that the "root" of the tree now becomes the followed
             * pointer.
             */
            if (node->key > key) {
                tree = &node->less;
            } else {    
                tree = &node->more;
            }
            node = *tree;
        }
    }
        
    /* No match found so create a new node
     */
    node = (BMI_SURROGATE_TREE*)BKR_MALLOC(sizeof(BMI_SURROGATE_TREE));
    if (node == NULL) {

        return lwk_s_failure;
    }

#ifdef MEMEX_DEBUG
    fprintf(stderr,"node = %8x\n",node);
#endif 
    /* Initialize the node.
     */
    node->key = key;
    node->list = NULL;
    node->less = NULL;
    node->more = NULL;

    /* Update the pointer that should point to this node.
     */
    *tree = node;

    /* Start a linked list of surrogates with the same key and return the
     * status from that operation.
     */
    return bmi_save_surrogate_in_list(&node->list,surrogate);

} /* end bmi_save_surrogate_in_tree () */

BMI_SURROGATE_LIST_PTR
bmi_find_surrogate_in_tree PARAM_NAMES((tree,key))
    BMI_SURROGATE_TREE_PTR tree PARAM_SEP
    unsigned key PARAM_END

/*
 *
 * Function description:
 *
 *      Finds a linked list of surrogates for a key in a tree.
 *
 * Arguments:
 *
 *      tree - the tree to search
 *
 *      key - the key to match
 *
 * Return value:
 *
 *      Pointer to the linked list of surrogates that match the specified
 *      key, or NULL.
 *
 * Side effects:
 *
 *      None
 *
 */

{
    /* Search the tree.
     */
    while (tree != NULL) {

        if (tree->key == key) {

            /* Found a match, return the surrogate list pointer.
             */
#ifdef MEMEX_DEBUG
    fprintf(stderr,"Found key %u in tree node %8x\n",key,tree);
#endif 
            return tree->list;

        } else {
            /* Follow the less branch or more branch appropriately
             */
            if (tree->key > key) {
                tree = tree->less;
            } else {
                tree = tree->more;
            }
        }
           }

    /* Didn't find a node with a matching key, return null.
     */
    return NULL;
        
} /* end bmi_find_surrogate_in_tree () */

void
bmi_clear_surrogate_list PARAM_NAMES((list_array,n_elements))
    BMI_SURROGATE_LIST_PTR *list_array PARAM_SEP
    unsigned n_elements PARAM_END

/*
 *
 * Function description:
 *
 *      
 *
 * Arguments:
 *
 *      None
 *
 * Return value:
 *
 *      None
 *
 * Side effects:
 *
 *      None
 *
 */

{
    register BMI_SURROGATE_LIST_PTR list;
    register BMI_SURROGATE_LIST_PTR next;
    register unsigned j;

    for (j = 0 ; j < n_elements ; j++) {
        if (list_array[j]) {
            list = list_array[j];
            while (list) {
                next = list->next;
                BKR_FREE(list);
                list = next;
            }
            list_array[j] = NULL;
        }
    }

} /* end bmi_clear_surrogate_list () */
void
bmi_delete_surrogate_tree PARAM_NAMES((tree))
    BMI_SURROGATE_TREE_PTR *tree PARAM_END
/*
 *
 * Function description:
 *
 *      
 *
 * Arguments:
 *
 *      None
 *
 * Return value:
 *
 *      None
 *
 * Side effects:
 *
 *      None
 *
 */

{

    if (*tree == NULL) {
        return;
    }
    if ((*tree)->less) {
        bmi_delete_surrogate_tree(&(*tree)->less);
    }
    if ((*tree)->more) {
        bmi_delete_surrogate_tree(&(*tree)->more);
    }

    if ((*tree)->list) {
        register BMI_SURROGATE_LIST_PTR ptr = (*tree)->list;
        register BMI_SURROGATE_LIST_PTR next;
        while (ptr) {
            next = ptr->next;
            BKR_FREE(ptr);
            ptr = next;
        }
    }
    BKR_FREE(*tree);
} /* end bmi_delete_surrogate_tree () */


lwk_status
bmi_create_surrogate PARAM_NAMES((container,object_type,description,surrogate_rtn))
    char *container PARAM_SEP
    char *object_type PARAM_SEP
    char *description PARAM_SEP
    lwk_surrogate *surrogate_rtn PARAM_END

/*
 *
 * Function description:
 *
 *      Create a basic bookreader surrogate object.
 *
 * Arguments:
 *
 *      container - the value for the container property
 *
 *      object_type - the type of object that this surrogate describes,
 *                    i.e. the value of the object type property
 *
 *      description - the value for the description property
 *
 *      surrogate_rtn - parameter for returning the create surrogate object
 *
 * Return value:
 *
 *      An lwk_status value inidicating the success or failure of the
 *      object creation.
 *
 * Side effects:
 *
 *      None
 *
 */

{
    lwk_status status;
    lwk_surrogate surrogate;
    XmString cstring;
    long cvt_status;
    long byte_cnt;
    Opaque ddif_string;

    RETURN_ON_ERROR( lwk_create(lwk_c_domain_surrogate, &surrogate) );
#ifdef MEMEX_DEBUG
    fprintf(stderr,
            "Created surrogate %8x\n	container: %s\n	object type: %s\n	description: %s\n",
            surrogate,container,object_type,description);
#endif 

    status = SET_STRING(surrogate,lwk_c_p_surrogate_sub_type,&bmi_surrogate_subtype);
    DELETE_AND_RETURN_ON_ERROR(status,surrogate);

    status = SET_STRING(surrogate,BMI_PROP_OBJECT_TYPE,&object_type);
    DELETE_AND_RETURN_ON_ERROR(status,surrogate);

    status = SET_STRING(surrogate,BMI_PROP_CONTAINER,&container);
    DELETE_AND_RETURN_ON_ERROR(status,surrogate);

#ifdef I18N_MULTIBYTE
    cstring = DXmCvtFCtoCS(description,&byte_cnt,&cvt_status);
#else
    cstring = XmStringCreateSimple(description);
#endif
    ddif_string = DXmCvtCStoDDIF(cstring,&byte_cnt,&cvt_status);
    if (cvt_status != 1) {
    	status = lwk_s_alloc_error;
    	DELETE_AND_RETURN_ON_ERROR(status,surrogate);
    }
    status = SET_CSTRING(surrogate,lwk_c_p_description,&ddif_string);
    COMPOUND_STRING_FREE(cstring);
    XtFree(ddif_string);
    DELETE_AND_RETURN_ON_ERROR(status,surrogate);

    *surrogate_rtn = surrogate;
    return lwk_s_success;

} /* end bmi_create_surrogate () */

lwk_surrogate
bmi_find_existing_surrogate PARAM_NAMES((winctx,surrogate_list,status_rtn))
    BKR_WINDOW_PTR winctx PARAM_SEP
    BMI_SURROGATE_LIST_PTR surrogate_list PARAM_SEP
    lwk_status             *status_rtn PARAM_END

/*
 *
 * Function description:
 *
 *      Attempt to find a suitable  existing surrogate in a surrogate list
 *
 * Arguments:
 *
 *      winctx - the window context for the surrogate
 *
 *      surrogate_list - the surrogate linked list to search
 *
 *      status_rtn - return parameter for the status
 * 
 *
 * Return value:
 *
 *      The found surrogate or lwk_c_null_object.
 *
 * Side effects:
 *
 *      None
 *
 */

{
    lwk_status status;
    lwk_surrogate surrogate;

    register BMI_SURROGATE_LIST_PTR list = surrogate_list;

    TRACE("bmi_find_existing_surrogates()")
    if (list == NULL) {
        
        /* No known surrogates, so return a NULL surrogate but a success
         * status, since we want the calling routine to continue.
         */
        *status_rtn = lwk_s_success;
        return lwk_c_null_object;
    }

    /*
    **  Look for a known Surrogate in the Current Network
    */
    while ((list != NULL) && (bmi_context.active_networks != NULL)){

        lwk_linknet network;
        BMI_NETWORK_LIST_PTR net_list = bmi_context.active_networks;

        status = lwk_get_value(list->surrogate,
                               lwk_c_p_container,
                               lwk_c_domain_persistent,
                               &network
                               );

        if (status != lwk_s_success) {
            *status_rtn = status;
            return lwk_c_null_object;
        }

        while (net_list) {
            if (network == net_list->network) { 
#ifdef MEMEX_DEBUG
                fprintf(stderr,"Found surrogate %8x in list %8x\n",
                        list->surrogate,surrogate_list);
                bmi_print_surrogate(list->surrogate);
#endif 
                *status_rtn = lwk_s_success;
                return list->surrogate;
            }
        }
	list = list->next;
    }

    /*
     * None in Current Network -- any other known Surrogate with the
     * same target type will do.  
     */
#ifdef MEMEX_DEBUG
    fprintf(stderr,"Found surrogate %8x in list %8x\n",
            surrogate_list->surrogate,surrogate_list);
    bmi_print_surrogate(list->surrogate);
#endif 
    *status_rtn = lwk_s_success;
    return surrogate_list->surrogate;

} /* end bmi_find_existing_surrogate () */

lwk_status
bmi_get_surrogates PARAM_NAMES((list,return_object))
    BMI_SURROGATE_LIST *list PARAM_SEP
    lwk_object *return_object PARAM_END

/*
 *
 * Function description:
 *
 *      Generic routine to get any existing surrogates for an object
 *      for the UI specific get_surrogate callback routines.  If multiple
 *      surrogates exist they will be returned as an lwk_list object.
 *
 * Arguments:
 *
 *      list - linked list of surrogates for the object
 *
 *      return_object - address of the lwk_object to get the surrogate
 *                      or list
 *
 * Return value:
 *
 *      Returns the status from creating the lwk_list and populating it
 *      if there is an error during thos operations otherwise, returns
 *      lwk_s_success.
 *
 * Side effects:
 *
 *      None
 *
 */

{       
    lwk_status status;
    lwk_list surrogate_list;

    TRACE("bmi_get_surrogates()")
    if (list == NULL) {
        
        /* Return a null object and a success status
         */
	*return_object = (lwk_object) lwk_c_null_object;
#ifdef MEMEX_DEBUG
        fprintf(stderr,"bmi_get_surrogates() returning lwk_c_null_object\n");
#endif 

    } else if (list->next == NULL) {

        /* Only one surrogate in the local linked list, so return it
         * and a succcess status.
         */
#ifdef MEMEX_DEBUG
    fprintf(stderr,"Found surrogate %8x in list %8x\n",list->surrogate,list);
#endif 
        *return_object = (lwk_object)list->surrogate;
#ifdef MEMEX_DEBUG
    fprintf(stderr,"Returned object %8x @ %8x to address %8x\n",
            *return_object,&list->surrogate,return_object);
    bmi_print_surrogate(list->surrogate);
#endif 

    } else {

        /* There are multiple surrogates in the local linked list.  Create
         * an lwk_list object, and put the surrogates in the list.  Then
         * return the list object and a success status, unless an error
         * occurs.
         */
        status = lwk_create_list(lwk_c_domain_surrogate, 5, &surrogate_list);
        RETURN_ON_ERROR(status);

        while (list) {
#ifdef MEMEX_DEBUG
    fprintf(stderr,"Found surrogate %8x in list %8x\n",list->surrogate,list);
    bmi_print_surrogate(list->surrogate);
#endif 
            status = lwk_add_element(surrogate_list, 
                                     lwk_c_domain_surrogate, 
                                     &list->surrogate
                                     );
            DELETE_AND_RETURN_ON_ERROR(status,surrogate_list);

            list = list->next;
        }
        *return_object = (lwk_object)surrogate_list;
#ifdef MEMEX_DEBUG
    fprintf(stderr,"Returned object %8x @ %8x to address %8x\n",
            *return_object,&surrogate_list,return_object);
#endif 
    }


    /* Success
     */
    return lwk_s_success;
        
} /* end bmi_get_surrogates () */
















