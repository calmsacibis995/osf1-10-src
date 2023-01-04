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
/* DEC/CMS REPLACEMENT HISTORY, Element BMI_LIBRARY.C*/
/* *9     9-JUN-1992 10:00:30 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *8    10-MAY-1992 21:11:17 BALLENGER "Add linkworks not installed message."*/
/* *7     8-MAY-1992 20:27:04 BALLENGER "temporary fix for LinkWorks not installed...."*/
/* *6     8-MAY-1992 17:58:28 BALLENGER "UCX$CONVERT"*/
/* *5     8-MAY-1992 16:52:08 BALLENGER "Fix QAR 127, Link menu craches if no library shef found."*/
/* *4     3-MAR-1992 17:07:20 KARDON "UCXed"*/
/* *3    12-DEC-1991 14:48:42 BALLENGER "Fix LinkWorks coldstart timing  problems."*/
/* *2     1-NOV-1991 12:48:05 BALLENGER "Reintegrate  memex support"*/
/* *1    16-SEP-1991 12:41:17 PARMENTER "Linkworks Interfaces"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BMI_LIBRARY.C*/
#ifndef VMS
 /*
#else
#module BMI_LIBRARY "V03-0003"
#endif
#ifndef VMS
  */
#endif
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
**      Bookreader Memex Interfaces (bmi*)
**
**  ABSTRACT:
**
**	Interfaces to the hyperinformation services for dealing
**      with libraries and shelves.
**
**  AUTHORS:
**
**      David L Ballenger
**
**  CREATION DATE:     02-Jul-1990
**
**  MODIFICATION HISTORY:
**
**
**  V03-0003 DLB0002		David L Ballenger	30-Apr-1991
**           Fix problems with handling of surrogate objects in the
**           library window.
**
**  V03-0002 DLB0001     David L Ballenger           01-Mar-1991
**           Fix problems with surrogate highlighting after the composite
**           network changes for QAR 807.  LinkWorks naming convention 
**           changes.
**
**  V03-0001 JAF0001	    James A. Ferguson   	14-Aug-1990
**  	     Modify update_library_highlighting and 
**           update_shelf_highlighting to add redisplay parameter.
**
**--
**/



/*
 * INCLUDE FILES
 */


#include "br_common_defs.h"  /* common BR #defines */ 
#include "br_meta_data.h"    /* typedefs and #defines for I/O of BR files */
#include "br_typedefs.h"     /* BR high-level typedefs and #defines */
#include "br_malloc.h"       /* BKR_MALLOC, etc defined here */
#include "br_globals.h"      /* BR external variables declared here */
#include "bkr_fetch.h"
#include "bkr_library.h"
#include "bmi_private_defs.h"
#include "bmi_surrogate_defs.h"
#include "bmi_library.h"
#include "bmi_query.h"
#include "bmi_surrogate.h"
#include "bmi_user_interface.h"

/*
 * DEFINES
 */
#define USE_FIRST_MATCH (-1)
#define DELIMITER_CHAR '\\'
#define DELIMITER_STR  "\\"
#define NULL_CHAR ((char)0)
#define SET_CHAR(ptr,chr) if (ptr) { *ptr = (chr) ; }


/*
** Global Variables
*/

BMI_GLOBAL_CONTEXT bmi_context;

static lwk_status
get_entry_info PARAM_NAMES((surrogate,entry_id,entry_type,entry_title))
    lwk_surrogate surrogate PARAM_SEP
    lwk_integer *entry_id PARAM_SEP
    lwk_integer *entry_type PARAM_SEP
    lwk_string  *entry_title PARAM_END

/*
 *
 * Function description:
 *
 *      Gets the entry id, type and title from a shelf entry surrogate
 *      object.
 *
 * Arguments:
 *
 *      surrogate   - a shelf entry surrogate object
 *      entry_id    - return address for the entry id
 *      entry_type  - return address for the entry type
 *      entry_title - return address for the entry title
 *
 * Return value:
 *
 *      The lwk_status from the lwk_get_value() calls.
 *
 * Side effects:
 *
 *      None
 *
 */

{
    lwk_status status;

    status = GET_INTEGER(surrogate,BMI_PROP_SHELF_ENTRY_ID,entry_id);
    if (status != lwk_s_success) {
        *entry_id = USE_FIRST_MATCH;
    }
    status = GET_INTEGER(surrogate,BMI_PROP_SHELF_ENTRY_TYPE,entry_type);
    if (status == lwk_s_success) {
        status = GET_STRING(surrogate,BMI_PROP_SHELF_ENTRY_TITLE,entry_title);
    }
    return status;

} /* end get_entry_info */


static BKR_NODE_PTR
find_entry_in_shelf PARAM_NAMES((shelf,entry_id,entry_type,entry_title))
    BKR_NODE_PTR shelf PARAM_SEP
    lwk_integer  entry_id PARAM_SEP
    lwk_integer  entry_type PARAM_SEP
    lwk_string   entry_title PARAM_END

/*
 *
 * Function description:
 *
 *      Looks for the entry specifed by the entry id, type, and file
 *      in a shelf.  If the entry represented by entry id, does not
 *      have the specifed type and file, it return the first one that
 *      does.
 *
 * Arguments:
 *
 *      shelf       - BKR_NODE_PTR for a shelf
 *      entry_id    - id of entry that we're looking for
 *      entry_type  - entry must have this type
 *      entry_title - entry must have this title
 *
 * Return value:
 *
 *      Pointer to the BKR_NODE for the entry, or NULL if the entry
 *      was not found.
 *
 * Side effects:
 *
 *      None
 *
 */

{
    register BKR_NODE_PTR entry = shelf->u.shelf.children;
    register BKR_NODE_PTR best_match = NULL;
    
    while (entry) {
        if (entry->entry_type == entry_type) {
            if (strcmp(entry->title,entry_title) == 0) {
                if ((entry->entry_id == entry_id)
                    || (entry_id == USE_FIRST_MATCH) 
                    ) {
                    best_match = entry;
                    break;
                } else if (best_match == NULL) {
                    best_match = entry;
                }
            }
        }
        entry = entry->sibling;
    }
    return best_match;
} /* end find_entry_in_shelf */


static lwk_termination
save_shelf_surrogates PARAM_NAMES((user_data,network,domain,surrogate))
    lwk_closure user_data PARAM_SEP
    lwk_composite_linknet network PARAM_SEP
    lwk_integer domain PARAM_SEP
    lwk_surrogate *surrogate PARAM_END

/*
 *
 * Function description:
 *
 *      Query callback routine to save shelf entry surrogate objects.
 *
 * Arguments:
 *
 *      user_data - a pointer to the BKR_NODE for the shelf.
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
    lwk_status status;
    lwk_integer entry_id;
    lwk_integer entry_type;
    lwk_string  entry_title;
    lwk_list    path;
    lwk_termination termination;
    BKR_NODE_PTR node;

    BKR_NODE_PTR shelf = (BKR_NODE_PTR)user_data;

    TRACE("save_shelf_surrogates()")

    /* Get the information to identify the specific entry in the shelf.
     */
    status = get_entry_info(*surrogate,&entry_id,&entry_type,&entry_title);
    if (status == lwk_s_success) {

        /* Is there matching entry?
         */
        node = find_entry_in_shelf(shelf,entry_id,entry_type,entry_title);
        if (node) {
            /* Found a match, save it in that entry's list
             * of surrogates.
             */
            bmi_save_surrogate_in_list(&node->surrogates,*surrogate);
        }
        (void)lwk_delete_string(&entry_title);
    } else {
        bmi_save_surrogate_in_list(&shelf->surrogates,*surrogate);
    }

    /* Return a termination of 0 to continue the query.
     */
    return 0;

} /* end save_shelf_surrogates */


static void
query_shelves PARAM_NAMES((parent))
    BKR_NODE_PTR parent PARAM_END
/*
 *
 * Function description:
 *
 *      Does a bmi_query_shelf() to find surrogates for this shelf, and calls
 *      itself recursively to do the queries for its subshelves.
 *
 * Arguments:
 *
 *      parent - BKR_NODE_PTR for the shelf to query
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
    BKR_NODE_PTR child;

    /* Now query this shelf.
     */
    bmi_query_shelf(parent,save_shelf_surrogates);

    /* Now do the same thing for subshelves.
     */
    child = parent->u.shelf.children;
    while (child) {

        if (NODE_IS_EXPANDABLE(child) && (child->u.shelf.children != NULL)) {
            query_shelves(child);
        }
        child = child->sibling;
    }

} /* end query_shelves */


static void
update_shelf_highlighting PARAM_NAMES((ui,shelf,refresh))
    lwk_dxm_ui ui PARAM_SEP
    BKR_NODE_PTR shelf PARAM_SEP
    Boolean refresh PARAM_END

/*
 *
 * Function description:
 *
 *      Update the highlighting for a shelf and all of its descendants.
 *
 * Arguments:
 *
 *      shelf 	  - the shelf node to start with
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
    BKR_NODE_PTR child;
    lwk_status status;

    TRACE("update_shelf_highlighting()")

    /* Scan through all the child entries, setting the highlighting for each
     * entry, and if the entry is a shelf with children then recursively
     * call this routine to update the highlighting for the children.
     */
    child = shelf->u.shelf.children;
    while (child) {

        Boolean old_highlight;

        old_highlight = child->highlight;
        child->highlight = HIGHLIGHT_STATE(ui,child->surrogates);
        if (shelf->u.shelf.opened 
            && ((old_highlight != child->highlight) || refresh)
           ) {
            bkr_library_update_entry(child);
        }
        
        /* If the child is a shelf with children then update the
         * highlighting for the children of the child.
         */
        if (NODE_IS_EXPANDABLE(child) && (child->u.shelf.children != NULL)) {
            update_shelf_highlighting(ui,child,refresh);
        }

        /* Get the next child/entry in this shelf
         */
        child = child->sibling;
    }
} /* end update_shelf_highlighting */


static void
update_library_highlighting PARAM_NAMES((window,redisplay))
    BKR_WINDOW_PTR window PARAM_SEP
    Boolean redisplay PARAM_END

/*
 *
 * Function description:
 *
 *      Routine specified in the library window context to do highlighting
 *      for the whole library.  This only does anything if the library
 *      window has highlighting turned on.
 *
 * Arguments:
 *
 *      window - The window context for the library window.  This is
 *               not used, and is only present for compatibility with
 *               the equivalent routines for topic and directory windows.
 *
 * Return value:
 *
 *      None
 *
 * Side effects:
 *
 *      Will cause the library to be queried if highlighting is turned on
 *      and the repository has not been queried for shelf surrogates.
 *
 */

{
    BKR_NODE_PTR topshelf  = window->u.library.root_of_tree;
    Boolean refresh;
    lwk_integer old_highlighting;
    lwk_status status;

    TRACE("update_library_highlighting()")


    if ((window->memex_ui == NULL)
        || (! XtIsManaged(window->widgets[W_MAIN_WINDOW]))
        || (topshelf == NULL)
        || (topshelf->u.shelf.children == NULL)
       ) {
        /* Things aren't set up yet, just return.
         */
        return;
    }
    if (bmi_context.composite_net_valid && window->u.library.shelves_queried) {
        
        /* Everything is up to date so nothing needs to be refreshed
         * yet.
         */
        refresh = FALSE;
    } else {
        /* Delete any existing surrogates.
         */
        if (window->u.library.shelves_queried) {
            bmi_delete_shelf_surrogates(topshelf);
        }
        /* Query the whole library tree of shelves.
         */
        query_shelves(topshelf);
        window->u.library.shelves_queried = TRUE;
        refresh = TRUE;
    }

    old_highlighting = HIGHLIGHTING_ON(window);
    status = GET_HIGHLIGHTING(window);
    if (status != lwk_s_success) {
        return;
    }

    refresh |= (HIGHLIGHTING_ON(window) != old_highlighting);

    /* The real work is done by this routine.
     */
    update_shelf_highlighting(window->memex_ui,topshelf,refresh);

} /* end update_library_highlighting */

void
bmi_create_shelf_context PARAM_NAMES((window,shelf))
    BKR_WINDOW_PTR window  PARAM_SEP
    BKR_NODE_PTR shelf PARAM_END

/*
 *
 * Function description:
 *
 *      This routine should be called whenever a shelf is opened.
 *      It updates the MEMEX information in the BKR_NODE for the
 *      shelf.
 *
 * Arguments:
 *
 *      shelf - the shelf node that has just been opened
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
    BKR_NODE_PTR child;
    int path_length;
    
    TRACE("bmi_create_shelf_context()")

    if (shelf->parent) {
        path_length = strlen(shelf->parent->u.shelf.path) + strlen(shelf->title) + 2;
        shelf->u.shelf.path = (char *)BKR_MALLOC(path_length);
        if (shelf->u.shelf.path) {
            strcpy(shelf->u.shelf.path,shelf->parent->u.shelf.path);
            strcat(shelf->u.shelf.path,DELIMITER_STR);
            strcat(shelf->u.shelf.path,shelf->title);
        }
    } else {
        path_length = strlen(window->u.library.name) + 2;
        shelf->u.shelf.path = (char *)BKR_MALLOC(path_length);
        if (shelf->u.shelf.path) {
            strcpy(shelf->u.shelf.path,window->u.library.name);
        }
    }
    
    if (bmi_context.composite_net_valid && window->u.library.shelves_queried) {

        /* We need to query for surrogates for this shelf, and update its
         * highlighting.
         */
        query_shelves(shelf);
        update_shelf_highlighting(window->memex_ui,shelf,TRUE);
    } else {
        update_library_highlighting(window,TRUE);        
    }

} /* end bmi_create_shelf_context */

void 
bmi_delete_shelf_surrogates PARAM_NAMES((shelf))
    BKR_NODE_PTR shelf PARAM_END
/*
 *
 * Function description:
 *
 *      Deletes all of the surroagtes for the specified shelf and
 *      all of its child shelves. This will typically be called after
 *      the current composite network has changed or when some other
 *      event cause the library to be requeried.
 *
 * Arguments:
 *
 *      shelf - the BKR_NODE_PTR for the shelf
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
    register BKR_NODE_PTR child;

    TRACE("bmi_delete_shelf_surrogates()")

    child = shelf->u.shelf.children;
    while (child) {

        /* First delete any old surrogate lists for the entries.
         */
        if (child->surrogates) {
            bmi_clear_surrogate_list(&child->surrogates,1);
        }
        child->highlight = FALSE;

        /* If the child entry is a shelf with children, delete
         * its surrogate lists.
         */
        if (NODE_IS_EXPANDABLE(child) && (child->u.shelf.children != NULL)) {
            bmi_delete_shelf_surrogates(child);
        }
        child = child->sibling;
    }

} /* end bmi_delete_shelf_surrogates */



static lwk_status
create_shelf_surrogate PARAM_NAMES((ui,reason,dxm_info,user_data,surrogate_rtn))
    lwk_ui ui PARAM_SEP 
    lwk_reason reason PARAM_SEP
    XmAnyCallbackStruct *dxm_info PARAM_SEP
    lwk_closure user_data PARAM_SEP
    lwk_surrogate *surrogate_rtn PARAM_END

/*
 *
 * Function description:
 *
 *      Create surrogate callback routine for creating shelf_surrogates
 *
 * Arguments:
 *
 *      ui            - the LinkWorks user interface object for the 
 *                      library window
 * 
 *      reason        - not used
 * 
 *      dxm_info      - not used
 * 
 *      user_data     - pointer to the library window
 * 
 *      surrogate_rtn - the return parameter for the created or found
 *                      surrogate
 *
 * Return value:
 *
 *      An lwk_status value indicating success or failure.
 *
 * Side effects:
 *
 *      None
 *
 */

{
    BKR_WINDOW_PTR window = (BKR_WINDOW_PTR)user_data;
    BKR_NODE_PTR selected;
    lwk_status status;
    lwk_surrogate surrogate;
    char *path;
    char desc[255];


    /* Find the selected entry in the library window. If there is no selected entry
     * we do an implicit selection of the whole library.
     */
    selected = window->u.library.root_of_tree;
    if (window->u.library.selected_entry_tags && window->u.library.selected_entry_tags[0]) {
        selected = (BKR_NODE *)window->u.library.selected_entry_tags[0];
    }

    if (selected->parent) {
        path = selected->parent->u.shelf.path;
    } else {
        path = window->u.library.name;
    }
    
    /* Attempt to return any existing surrogates
     */
    if (selected->surrogates) {
        return bmi_get_surrogates(selected->surrogates,surrogate_rtn);
    }

    /*	No known Surrogate, so create one.
     */
    sprintf(desc,"%s:  %s",window->u.library.name,selected->title);
    status = bmi_create_surrogate(path,BMI_OBJ_NAME_SHELF,desc,&surrogate);
    RETURN_ON_ERROR(status);

    if (selected->parent) {

        /* Set the SHELF_ENTRY_ID property using the entry id
         */
        status = SET_INTEGER(surrogate,BMI_PROP_SHELF_ENTRY_ID,&selected->entry_id);
        DELETE_AND_RETURN_ON_ERROR(status,surrogate);
        
        /* Set the SHELF_ENTRY_TYPE property using the entry type
         */
        status = SET_INTEGER(surrogate,BMI_PROP_SHELF_ENTRY_TYPE,&selected->entry_type);
        DELETE_AND_RETURN_ON_ERROR(status,surrogate);

        /* Set the SHELF_ENTRY_TITLE property using the entry title
         */
        status = SET_STRING(surrogate,BMI_PROP_SHELF_ENTRY_TITLE,&selected->title);
        DELETE_AND_RETURN_ON_ERROR(status,surrogate);
    }

    /* Save it in list for this entry.
     */
    status = bmi_save_surrogate_in_list(&selected->surrogates,surrogate);
    if (status == lwk_s_success) {
        *surrogate_rtn = surrogate;
    }
    return status;

} /* end bmi_create_shelf_surrogate () */

static lwk_status  
get_shelf_surrogate PARAM_NAMES((ui,reason,dxm_info,user_data,surrogates))
    lwk_ui ui PARAM_SEP 
    lwk_reason reason PARAM_SEP
    XmAnyCallbackStruct *dxm_info PARAM_SEP
    lwk_closure user_data PARAM_SEP
    lwk_surrogate *surrogates PARAM_END

/*
 *
 * Function description:
 *
 *     Get surrogate callback routine for shelf surrogates 
 *
 * Arguments:
 *
 *      ui         - the LinkWorks user interface object for the 
 *                   library window
 * 
 *      reason     - not used
 * 
 *      dxm_info   - not used
 * 
 *      user_data  - not used
 * 
 *      surrogates - the return parameter for any found surrogates
 *
 * Return value:
 *
 *      An lwk_status value indicating success or failure.
 *
 * Side effects:
 *
 *      None
 *
 */
{
    BKR_WINDOW_PTR window = (BKR_WINDOW_PTR)user_data;
    BKR_NODE_PTR selected;
    lwk_status status;
    lwk_surrogate surrogate;
    char desc[255];


    if (window && (window->type == BKR_LIBRARY) && (window->u.library.root_of_tree) ) 
    {
        /* Find the selected entry in the library window. If there is no selected entry
         * we do an implicit selection of the whole library.
         */
        selected = window->u.library.root_of_tree;
        if (window->u.library.selected_entry_tags 
            && window->u.library.selected_entry_tags[0]
            ) 
        {
            selected = (BKR_NODE *)window->u.library.selected_entry_tags[0];
        }

        if (selected && selected->surrogates) 
        {
            /* Attempt to get the surrogates for the selection or the
             * whole library.
             */
            return bmi_get_surrogates(selected->surrogates,surrogates);
        }
    }
    *surrogates = lwk_c_null_object;
    return lwk_s_success;

} /* end get_shelf_surrogate */

void
bmi_create_library_ui PARAM_NAMES((window))
    BKR_WINDOW_PTR window PARAM_END
/*
 *
 * Function description:
 *
 *      Create the Memex User Inerface for the library window.
 *
 * Arguments:
 *
 *      shell - the library shell 
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
    Pixel   foreground_pixel;
    Pixel   background_pixel;
    Arg     arglist[5];
    int	    argcnt;
    caddr_t data;

    TRACE("bmi_create_library_ui()")
    if (bmi_context.initialized == FALSE) {

        /* This shouldn't happen, just return for now.
         */
        bmi_install_no_linkworks_menu(window);
        return;
    }
    if (window->type != BKR_LIBRARY) {
        return;
    }

    window->update_highlighting = (void *)update_library_highlighting;

#ifdef MEMEX_DEBUG
    fprintf(stderr,"Creating Dw Ui for library\n");
#endif 
    /* Now create the MEMEX user interface for the library window.
     */
    status = bmi_create_memex_ui(window,
                                 (lwk_callback)create_shelf_surrogate,
                                 (lwk_callback)get_shelf_surrogate);


    /* Get the foreground/background color for the icons */

    argcnt = 0;
    SET_ARG( XmNforeground, &foreground_pixel );
    SET_ARG( XmNbackground, &background_pixel );
    XtGetValues( window->widgets[W_SVN], arglist, argcnt );

    /* Get the LWK icon information for highlighting surrogates.
     */
    bmi_context.highlight_icon = bkr_fetch_icon_literal( "MEMEX_HIGHLIGHT_ICON",
                                                        foreground_pixel, 
                                                        background_pixel );
    data = bkr_fetch_literal( "HIGHLIGHT_PIXMAP_WIDTH", MrmRtypeInteger );
    bmi_context.highlight_icon_width = *( (int *) data );
    data = bkr_fetch_literal( "HIGHLIGHT_PIXMAP_HEIGHT", MrmRtypeInteger );
    bmi_context.highlight_icon_height = *( (int *) data );


} /* end bmi_create_library_ui */

bmi_apply_coldstart_surrogate()
/*
 *
 * Function description:
 *
 *      Will do a lwk_apply for the surrogate returned via
 *      his_initialize if Bookreader was coldstarted, i.e.
 *      hyperinvoked. 
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
    /* If the user interface was created and the bookreader was hyperinvoked,
     * then an initial operation and surrogate were returned from the call
     * to lwk_initialize().  Apply the operation to the surrogate.
     */
    if ((bmi_context.initialized) 
        && (bmi_context.hyperinvoked)
        && (bkr_library->memex_ui)
        ) {

        (void)lwk_apply(bkr_library->memex_ui,
                        bmi_context.operation,
                        bmi_context.surrogate
                        );
    }
} /* end bmi_apply_coldstart_surrogate */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bmi_shelf_open_to_entry
**
**      
**
**  FORMAL PARAMETERS:
**
**  	container    	 - the path to the shelf entry, comes from the
**                         container property in the surrogate object
**      surrogate        - the surrogate describing the shelf entry
**
**  IMPLICIT INPUTS:
**
**	bmi_context
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	Returns:    status of the open shelf entry operation.
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
lwk_status
bmi_shelf_open_to_entry PARAM_NAMES((window,container,surrogate))
    BKR_WINDOW_PTR window PARAM_SEP
    lwk_string    container PARAM_SEP
    lwk_surrogate surrogate PARAM_END

{
    lwk_integer  entry_id;
    lwk_integer  entry_type;
    lwk_string   entry_title;
    char         *delimiter;
    char         *title;
    lwk_status	 status;
    BKR_NODE_PTR entry;
    Widget       svn_widget;
    int          tag = W_EXPAND_ENTRY;
    XmAnyCallbackStruct reason;

    bkr_cursor_display_wait( ON );

    /* The library name / file spec is the first component in the
     * container string.  Replace the delimiter with a NUL.
     */
    delimiter = strchr(container,DELIMITER_CHAR);
    SET_CHAR(delimiter,NULL_CHAR);

    /* If the first component of the container does not match the
     * current library name, the switch libraries.
     */
    if (strcmp(container,window->u.library.name)) {
        if (!bkr_library_open_new_library(container)) {
            SET_CHAR(delimiter,DELIMITER_CHAR);
            bkr_cursor_display_wait( OFF );
            BKR_FLUSH_EVENTS;
            return lwk_s_failure;
        }
    }

    /* The rest of the components in the container are the entry title strings
     * which define the path down to the specific entry defined by specified
     * by this surrogate.
     */
    entry = window->u.library.root_of_tree;
    svn_widget = window->widgets[W_SVN];

    /* While we can find title strings in the container string,
     * attempt to find and open the shelf entries leading to the
     * entry we want.
     */
    while (entry && delimiter) {
        *delimiter = DELIMITER_CHAR;
        title = &delimiter[1];
        delimiter = strchr(title,DELIMITER_CHAR);
        SET_CHAR(delimiter,NULL_CHAR);
        if (*title) {
            entry = find_entry_in_shelf(entry,USE_FIRST_MATCH,BKR_SHELF_FILE,title);
            if (entry) {
                bkr_library_open_entry(entry);
            }
        }
    }
    SET_CHAR(delimiter,DELIMITER_CHAR);

    /* Could we find all of the shelf entry specified by the path.
     */
    if (entry == NULL) {
        bkr_cursor_display_wait( OFF );
        BKR_FLUSH_EVENTS;
        return lwk_s_failure;
    }

    /* Get the information to identify the specific entry in the shelf.
     */
    status = get_entry_info(surrogate,&entry_id,&entry_type,&entry_title);
    if (status == lwk_s_success) {

        /* Find that entry in the last open shelf
         */
        entry = find_entry_in_shelf(entry,entry_id,entry_type,entry_title);
        (void)lwk_delete_string(&entry_title);

        if (entry == NULL) {

            /* Didn't find the entry.
             */
            bkr_cursor_display_wait( OFF );
            BKR_FLUSH_EVENTS;
            status = lwk_s_failure;

        } else {

            /* We actually found the entry. Position it so that it is
             * at the "top" of the library window, then select it if
             * select destination is turned on.
             */

            int                 unused_tag;
            int                 entry_num;
            XmAnyCallbackStruct unused_callback_data;

            DXmSvnDisableDisplay(svn_widget);
            entry_num = DXmSvnGetEntryNumber(svn_widget,(unsigned)entry);
            DXmSvnPositionDisplay(svn_widget,entry_num,DXmSvnKpositionTop);
            if (bmi_context.select_destination) {
                bmi_select_svn_entry(bkr_library,(unsigned)entry);
            }
            DXmSvnEnableDisplay(svn_widget);
        }
    } else {
        if (status == lwk_s_no_such_property) {
            status = lwk_s_success;
        }
    }
    /* Make sure the library window is on not iconified.
     */
    RAISE_WINDOW(window->appl_shell_id);
    bkr_cursor_display_wait( OFF );
    BKR_FLUSH_EVENTS;
    return status;

};  /* end of bmi_shelf_open_to_entry */



