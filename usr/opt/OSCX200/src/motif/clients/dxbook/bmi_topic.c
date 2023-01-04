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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BMI_TOPIC.C*/
/* *9    24-FEB-1993 17:41:21 BALLENGER "Fix problems with large topics."*/
/* *8    13-AUG-1992 15:10:41 GOSSELIN "updating with necessary A/OSF changes"*/
/* *7     9-JUN-1992 10:01:01 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *6    10-MAY-1992 21:11:22 BALLENGER "Add linkworks not installed message."*/
/* *5     8-MAY-1992 20:27:09 BALLENGER "temporary fix for LinkWorks not installed...."*/
/* *4     3-MAR-1992 17:08:26 KARDON "UCXed"*/
/* *3    13-FEB-1992 13:26:56 BALLENGER "Fix ACCVIOs when deleting links or (de)activating link nets, etc."*/
/* *2     1-NOV-1991 12:48:33 BALLENGER "Reintegrate  memex support"*/
/* *1    16-SEP-1991 12:42:46 PARMENTER "Linkworks Interfaces for topic window and surrogate objects"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BMI_TOPIC.C*/
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
**      Bookreader Memex Interface (bmi_*)
**
**  ABSTRACT:
**
**	Routines for dealing with the topic window and its
**      HIS user interface and surrogate objects.
**
**  AUTHORS:
**
**      David L Ballenger
**
**  CREATION DATE:     02-Jul-1990
**
**  MODIFICATION HISTORY:
**
**      DLB0002     David L Ballenger           01-Mar-1991
**                  Fix problems with surrogate highlighting after the composite
**                  network changes for QAR 807.  LinkWorks naming
**                  convention changes.
**
**      DLB0001     David L Ballenger           08-Feb-1991
**                  Fix problems with surrogate highlighting after the composite
**                  network changes.
**
**	JAF0001	    James A. Ferguson   	14-Aug-1990
**  	    	    Fix bug in find_selected_chunk when searching for 
**  	    	    first chunk in window.  Modify bmi_update_chunk_highlighting 
**  	    	    and clear_chunk_highlighting to redisplay the Topic after
**  	    	    the highlighting has been changed.  Also add redisplay 
**  	    	    parameter to bmi_update_chunk_highlighting.
**
**--
**/

#include "br_common_defs.h"  /* common BR #defines */ 
#include "br_meta_data.h"    /* typedefs and #defines for I/O of BR files */
#include "bods_private_def.h"
#include "br_typedefs.h"     /* BR high-level typedefs and #defines */
#include "br_malloc.h"       /* BKR_MALLOC, etc defined here */
#include "br_globals.h"      /* BR external variables declared here */
#include "bkr_selection.h"
#include "bmi_private_defs.h"
#include "bmi_surrogate_defs.h"
#include "bmi_book.h"
#include "bmi_query.h"
#include "bmi_surrogate.h"
#include "bmi_topic.h"
#include "bmi_user_interface.h"

/*
** External References
*/
extern char *bri_page_chunk_symbol();


lwk_termination
save_chunk_surrogates PARAM_NAMES((user_data,network,domain,surrogate))
    lwk_closure user_data PARAM_SEP
    lwk_composite_linknet network PARAM_SEP
    lwk_integer domain PARAM_SEP
    lwk_surrogate *surrogate PARAM_END

/*
 *
 * Function description:
 *
 *      Query callback routine to save chunk surrogate objects.
 *
 * Arguments:
 *
 *      user_data - the book context for the book being queried
 *
 *      network   - Not used.
 *
 *      domain    - Not used.
 *
 *      surrogate - The surrogate to be saved.
 *
 * Return value:
 *
 *      0
 *
 * Side effects:
 *
 *      None
 *
 */
{
    BKR_BOOK_CTX_PTR book = (BKR_BOOK_CTX_PTR)user_data;
    lwk_status status;
    BMD_OBJECT_ID ckid;
    char *chunk_symbol;

    status = GET_INTEGER(*surrogate,BMI_PROP_CHUNK_ID,&ckid);
    if (status != lwk_s_success) {
        if (status == lwk_s_no_such_property) {
            ckid = 0;
        } else {
            return 0;
        }
    }
#ifdef MEMEX_DEBUG
    fprintf(stderr,"Query found chunk id %u\n",ckid);
#endif 
    status = bmi_check_book_timestamp(*surrogate,book);
    if ((status == lwk_s_success) && (ckid != 0)) {
        bmi_save_surrogate_in_tree(&book->chunk_surrogates,*surrogate,ckid);
        return 0;
    }

    status = GET_STRING(*surrogate,BMI_PROP_CHUNK_NAME,&chunk_symbol);
    if (status != lwk_s_success) {
        return 0;
    }
#ifdef MEMEX_DEBUG
    fprintf(stderr,"Query found chunk symbol %s\n",chunk_symbol);
#endif 

    ckid = bri_symbol_to_chunk(book->book_id,chunk_symbol);
    (void)lwk_delete_string(&chunk_symbol);
#ifdef MEMEX_DEBUG
    fprintf(stderr,"Query found chunk id %u\n",ckid);
#endif 

    if (ckid) {
        bmi_save_surrogate_in_tree(&book->chunk_surrogates,*surrogate,ckid);
    }
    return 0;

} /* end save_chunk_surrogates () */

static Boolean
query_for_chunks PARAM_NAMES((window))
    BKR_WINDOW *window PARAM_END
/*
 *
 * Function description:
 *
 *      Query the composite network for chunk surrogates
 *
 * Arguments:
 *
 *      window - the window context
 *
 * Return value:
 *
 *      Returns the status from the query or success if we didn't
 *      need to do the query.
 *
 * Side effects:
 *
 *      None
 *
 */

{
    register BKR_BOOK_CTX_PTR book;
    lwk_status status;

    TRACE("query for chunks");

    book = window->shell->book;

    if (bmi_context.composite_net_valid && book->queried_chunks) {
        return FALSE;
    }

    status = bmi_query_book(book,BMI_OBJ_NAME_CHUNK,(lwk_callback *)save_chunk_surrogates);
    if (status == lwk_s_success) {
        book->queried_chunks = TRUE;
    }
    return TRUE;

} /* end query_for_chunks () */


static BMD_OBJECT_ID 
find_selected_chunk PARAM_NAMES((window))
    BKR_WINDOW *window PARAM_END

/*
 *
 * Function description:
 *
 *      Find the "selected" chunk in a topic window and return the
 *      chunk id.  Since there is currently no way to select a chunk
 *      from the topic window, the routine finds the parent chunk of
 *      the top most chunk visible in the window.
 *
 * Arguments:
 *
 *      shell - the shell for the topic window
 *
 * Return value:
 *
 *      The chunk id of the "selected" chunk.
 *
 * Side effects:
 *
 *      None
 *
 */

{
    register BMD_CHUNK *chunk = window->u.topic.chunk_list;
    register unsigned cknum;

    /* Find the "selected" chunk.  Since a user can't "select" a chunk
     * in a topic window we use the parent chunk of the chunk at the
     * top of the window.
     */
    cknum = 0;
    while ( (cknum < window->u.topic.num_chunks)
           && ( (abs(window->u.topic.y) > chunk->rect.top)
    	      && (abs(window->u.topic.y) > chunk->rect.bottom) )
           ) {
        cknum++;
        chunk++;
    }
    if (chunk->parent) {
        return (chunk->parent);
    } else {
        return (chunk->id);
    }

} /* end find_selected_chunk () */


static lwk_status
create_chunk_surrogate PARAM_NAMES((ui,reason,dxm_info,user_data,surrogate_rtn))
    lwk_ui ui PARAM_SEP 
    lwk_reason reason PARAM_SEP
    XmAnyCallbackStruct *dxm_info PARAM_SEP
    lwk_closure user_data PARAM_SEP
    lwk_surrogate *surrogate_rtn PARAM_END

/*
 *
 * Function description:
 *
 *      The create surrogate call back for the topic window HIS
 *      user interface.
 *
 * Arguments:
 *
 *      ui - the HIS user interface for the topic window
 *
 *      user_data - the window context for the topic window
 *
 *      surrogate_rtn - return parameter for the "created" surrogate
 *
 * Return value:
 *
 *      Returns lwk_s_success if sucessful, otherwise an apporpiate
 *      lwk_status error value.
 *
 * Side effects:
 *
 *      None
 *
 */

{

    BKR_WINDOW_PTR window;
    BKR_BOOK_CTX_PTR book;
    BMI_SURROGATE_LIST_PTR surrogate_list;
    lwk_status status;
    lwk_string string;
    lwk_surrogate surrogate;
    BMD_OBJECT_ID ckid;
    char desc[255];

    /* Get our window context for the topic from the user_data, and then
     * the local book context from the window context.
     */
    window = (BKR_WINDOW_PTR)user_data;
    book = window->shell->book;

    (void)query_for_chunks(window);

    /* Find the selected chunk in the topic window.
     */
    ckid = find_selected_chunk(window);

    /* Get the surrogate list for this chunk from the book context
     */
    surrogate_list = bmi_find_surrogate_in_tree(book->chunk_surrogates,ckid);

    /* See if the list contains any approriate surrogates
     */
    surrogate = bmi_find_existing_surrogate(window,surrogate_list,&status);
    if (surrogate) {

        /* Yep, so return this surrogate.
         */
        *surrogate_rtn = surrogate;
        return lwk_s_success;

    } else {

        /* No. Return the error status if an error occured, otherwise
         * create a new surrogate.
         */
        RETURN_ON_ERROR(status);
    }

    /*	No known Surrogate, so create one.
     */

    /* Create a description for the surrogate.
     */
    sprintf(desc,
            "%s:  %s",
            book->title,
            bri_page_chunk_title(book->book_id,ckid)
            );

    /* Call a routine to create a basic book surrogate of the specfied
     * type.
     */
    status = bmi_create_book_surrogate(book,
                                       BMI_OBJ_NAME_CHUNK,
                                       desc,
                                       &surrogate);
    RETURN_ON_ERROR(status);

    /* If the chunk has a symbol then but it in the surrogate.  If an
     * error occurs delete the partially defined surrogate and return
     * the error status.
     */
    string = bri_page_chunk_symbol(book->book_id,ckid);
    if (string) {
        status = SET_STRING(surrogate,BMI_PROP_CHUNK_NAME,&string);
        DELETE_AND_RETURN_ON_ERROR(status,surrogate);
#ifdef MEMEX_DEBUG
        fprintf(stderr,"Creating chunk symbol %s\n",string);
#endif 
    }

    /* Always put the chunk id in the surrogate.  If an error occurs delete
     * the partially defined surrogate and return the error status.
     */
    status = SET_INTEGER(surrogate,BMI_PROP_CHUNK_ID,&ckid);
#ifdef MEMEX_DEBUG
        fprintf(stderr,"Creating chunk id %u\n",ckid);
#endif 
    DELETE_AND_RETURN_ON_ERROR(status,surrogate);

    /* Keep track of the new surrogate in the local surrogate list
     * to make checking some things easier.
     */
    status = bmi_save_surrogate_in_tree(&book->chunk_surrogates,surrogate,ckid);
    RETURN_ON_ERROR(status);


    /* Return the new surrogate and success,
     */
    *surrogate_rtn = surrogate;
    return lwk_s_success;

} /* end create_chunk_surrogate */

static lwk_status  
get_chunk_surrogate PARAM_NAMES((ui,reason,dxm_info,user_data,object_rtn))
    lwk_ui ui PARAM_SEP 
    lwk_reason reason PARAM_SEP
    XmAnyCallbackStruct *dxm_info PARAM_SEP
    lwk_closure user_data PARAM_SEP
    lwk_object *object_rtn PARAM_END

/*
 *
 * Function description:
 *
 *      The get surrogate callback for the topic window HIS user interface.
 *
 * Arguments:
 *
 *      ui - the HIS user interface for the topic window
 *
 *      user_data - the window context for the topic window
 *
 *      object_rtn - return parameter for the found surrogates
 *
 * Return value:
 *
 *      Returns lwk_s_succes unless an error occurs when creating and
 *      populating and lwk_list object for multiple surrogates.
 *
 * Side effects:
 *
 *      None
 *
 */
{
    BKR_WINDOW_PTR window = (BKR_WINDOW_PTR)user_data;
    BKR_BOOK_CTX_PTR book  = window->shell->book;
    BMI_SURROGATE_LIST_PTR list;
    lwk_status status;
    BMD_OBJECT_ID ckid;


    TRACE("get_chunk_surrogate()")
    
    if (window->active) {

        (void)query_for_chunks(window);
        
        /* Find the selected chunk.
         */
        ckid = find_selected_chunk(window);
        
        /* Find the local linked list of surrogates for the selected chunk.
         */
        list = bmi_find_surrogate_in_tree(book->chunk_surrogates,ckid);
#ifdef MEMEX_DEBUG
        fprintf(stderr,"book->chunk_surrogates = %x\n",book->chunk_surrogates);
#endif 
        
        /* Return a null object, the single surrogate for the chunk, or
         * an lwk_list containing all the surrogates for the chunk.
         */
        status = bmi_get_surrogates(list,object_rtn);
#ifdef MEMEX_DEBUG
    fprintf(stderr,"Returned object %8x to address %8x\n",
            *object_rtn,object_rtn);
#endif 
      
    return status;
   
    } else {

        /* The window is not active, i.e. its cached, so just return a
         * null object and a success status.
         */
        *object_rtn = (lwk_object) lwk_c_null_object;
        return lwk_s_success;
    }
} /* end get_chunk_surrogate */

void
bmi_update_chunk_highlighting PARAM_NAMES((window,redisplay))
    BKR_WINDOW_PTR window PARAM_SEP
    Boolean redisplay PARAM_END

/*
 *
 * Function description:
 *
 *      Update the highlighting for the chunks in a topic window.  This
 *      routine is called whenever a new topic is displayed in the topic
 *      window or the current compoiste network changes.  I may also be
 *      called when other currency changes that could affect highlighting.
 *
 * Arguments:
 *
 *      window    - the window context
 *  	redisplay - redraw the contents of the Topic window
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
    BKR_BOOK_CTX_PTR book;
    lwk_integer old_highlighting;
    lwk_status status;
    Boolean state_changed;


    TRACE("bmi_update_chunk_highlighting()")

    if ((window == NULL) || (window->active == FALSE)) {
        return;
    }
    book = window->shell->book;

    if (book == NULL) {
        return;
    }

    state_changed = query_for_chunks(window);
#ifdef MEMEX_DEBUG
    fprintf(stderr,"State_changed is %s after %s\n",(state_changed) ? "TRUE" : "FALSE","query");
#endif 

    old_highlighting = HIGHLIGHTING_ON(window);
    if (GET_HIGHLIGHTING(window) != lwk_s_success) {
        return;
    }
    
    state_changed |= (HIGHLIGHTING_ON(window) != old_highlighting);
#ifdef MEMEX_DEBUG
    fprintf(stderr,"State_changed is %s after %s\n",(state_changed) ? "TRUE" : "FALSE",
            "checking previous highlighting");
#endif 

    if (book->chunk_surrogates) {

        BMI_SURROGATE_TREE_PTR tree = book->chunk_surrogates;
        register unsigned num_chunks = window->u.topic.num_chunks;
        register BMD_CHUNK *chunk = window->u.topic.chunk_list;

        while (num_chunks) {
            register BMI_SURROGATE_LIST_PTR surrogate_list;
            Boolean new_highlight_state;
            
            surrogate_list = bmi_find_surrogate_in_tree(tree,chunk->id);
            new_highlight_state = HIGHLIGHT_STATE(window->memex_ui,surrogate_list);
            if (chunk->highlight != new_highlight_state) {
                state_changed = TRUE;
                chunk->highlight = new_highlight_state;
            }
            chunk++;
            num_chunks--;
        }
    } else {

        register unsigned num_chunks = window->u.topic.num_chunks;
        register BMD_CHUNK *chunk = window->u.topic.chunk_list;

        while (num_chunks) {
            if (chunk->highlight) {
                state_changed = TRUE;
                chunk->highlight = FALSE;
            }
            chunk++;
            num_chunks--;
        }
    }
#ifdef MEMEX_DEBUG
    fprintf(stderr,"State_changed is %s after %s\n",(state_changed) ? "TRUE" : "FALSE","checking chunks");
#endif 
    
    /* Redraw the Topic window so the highlighting will be updated.
     */
    if (state_changed && redisplay) {
    	bkr_window_expose( window->widgets[W_MAIN_WINDOW],window,NULL);
    }

}   /* end bmi_update_chunk_highlighting */

void
bmi_create_topic_ui PARAM_NAMES((window))
    BKR_WINDOW_PTR window PARAM_END
/*
 *
 * Function description:
 *
 *      This routine initializes the HIS user interface and light bulb menu
 *      for a topic window. It should be called from bmi_topic_open_to_position
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
    BKR_BOOK_CTX_PTR book;
    lwk_status status;

    TRACE("bmi_create_topic_ui()")

    if (bmi_context.initialized == FALSE) {

        /* This shouldn't happen, just return for now.
         */
        bmi_install_no_linkworks_menu(window);
        return;
    }
    if ((window->type != BKR_STANDARD_TOPIC) && (window->type != BKR_FORMAL_TOPIC)) {
        return;
    }

    /* Set up the surrogate query and highlight updating routines for
     * dealing with chunk surrogates.
     */
    window->update_highlighting = (void *)bmi_update_chunk_highlighting;

    status = bmi_create_memex_ui(window,
                                 (lwk_callback)create_chunk_surrogate,
                                 (lwk_callback)get_chunk_surrogate);

} /* end bmi_create_topic_ui */












