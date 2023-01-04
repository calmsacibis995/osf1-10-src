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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BMI_USER_INTERFACE.C*/
/* *8    20-SEP-1992 17:55:03 BALLENGER "Turn off LinkWorks in Chracter cell mode"*/
/* *7     9-JUN-1992 10:01:11 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *6    10-MAY-1992 21:11:26 BALLENGER "Add linkworks not installed message."*/
/* *5     8-MAY-1992 20:27:13 BALLENGER "temporary fix for LinkWorks not installed...."*/
/* *4     3-MAR-1992 17:08:42 KARDON "UCXed"*/
/* *3    12-FEB-1992 12:17:00 PARMENTER "i18n support:  #ifdef'ed asian"*/
/* *2     1-NOV-1991 12:48:38 BALLENGER "Reintegrate  memex support"*/
/* *1    16-SEP-1991 12:42:35 PARMENTER "Linkworks Interfaces"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BMI_USER_INTERFACE.C*/
#ifndef VMS
 /*
#else
#module BMI_USER_INTERFACE "V03-0005"
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
**      Bookreader Memex Interfaces Interface (bmi*)
**
**  ABSTRACT:
**
**	Routines for dealing with the HIS User Interface
**
**  AUTHORS:
**
**      David L Ballenger
**
**  CREATION DATE:     02-Jul-1990
**
**  MODIFICATION HISTORY:
**
**  V03-0005  DLB0003     David L Ballenger           30-Apr-1991
**            Fix problems with surrogates in the library window.
**
**  V03-0004  DLB0002     David L Ballenger           01-Mar-1991
**            Fix problems with surrogate highlighting after the composite
**            network changes for QAR 807.  LinkWorks naming convention changes.
**      
**  V03-0003  DLB0001     David L Ballenger           8-Feb-1991
**            Add fixes for MEMEX related QARS.
**
**  V03-0002  JAF0002	    James A. Ferguson   	3-Oct-1990
**  	      Fix infinite loop in reset_surrogates_for_all_windows.
**
**  V03-0001  JAF0001	    James A. Ferguson   	14-Aug-1990
**  	      Add parameter to window->update_highlighting routine calls.
**
**--
**/


#include "br_common_defs.h"
#include "br_meta_data.h"
#include "br_typedefs.h"
#include "br_globals.h"
#include "br_malloc.h"
#include "bkr_error.h"
#include "bkr_fetch.h"
#include "bmi_private_defs.h"
#include "bmi_surrogate_defs.h"
#include "bmi_library.h"
#include "bmi_book.h"
#include <X11/Xlib.h>
#include <DXm/DECspecific.h>
#include <lwk_dxm_def.h>

#define SET_CALLBACK(window,prop,routine) \
        { \
            lwk_status set_cb_status; \
            lwk_callback cb = (lwk_callback)routine; \
            set_cb_status = lwk_set_value(window->memex_ui, \
                                   prop, \
                                   lwk_c_domain_routine, \
                                   &cb, \
                                   lwk_c_set_property \
                                   ); \
            DELETE_AND_RETURN_ON_ERROR(set_cb_status,window->memex_ui); \
        }

#ifdef MEMEX_DEBUG
#define CURRENCY_CHANGE(string) (void)fprintf(stderr,"Currency change for window %x:  %s\n",window,string)
#else
#define CURRENCY_CHANGE(string)
#endif 

static MrmRegisterArg  tag_reglist[] = { { "tag", (caddr_t) 0 } };
        

static lwk_status
get_default_highlighting PARAM_NAMES((window))
    BKR_WINDOW_PTR window PARAM_END
/*
 *
 * Function description:
 *
 *      Gets the default highlighting. This should only be called for
 *      the library window.
 *
 * Arguments:
 *
 *      window - The window context for the library window.
 *
 * Return value:
 *
 *      The status returned from lwk_get_value() or lwk_set_value().
 *
 * Side effects:
 *
 *      None
 *
 */
    
{
    lwk_status status;
    lwk_integer highlighting;

    /* Get the new default highlighting.
     */
    highlighting = lwk_c_hl_none;
    status = lwk_get_value(window->memex_ui,
                           lwk_c_p_default_highlight,
                           lwk_c_domain_integer,
                           &highlighting
                           );
    RETURN_ON_ERROR(status);

    if (window->type == BKR_LIBRARY) {
        /* This is the libray window so update the select_destination flag.
         */
        bmi_context.select_destination 
            = (highlighting & lwk_c_hl_select_destination);
    }

    if (window->using_default_highlighting) {
        /* Set the current highlighting in this window to the new default
         * highlighting.
         */
        status = lwk_set_value(window->memex_ui,
                               lwk_c_p_appl_highlight,
                               lwk_c_domain_integer,
                               &highlighting,
                               lwk_c_set_property
                               );
        RETURN_ON_ERROR(status);
        
        /* Since changing the current highlighting to the new default setting
         * will set this flag to FALSE we need to set it back to TRUE to
         * indicate that this window is still using the default highlighting.
         */
        window->using_default_highlighting = TRUE;
    }
    return status;
    
} /* end get_default_highlighting */

lwk_boolean
bmi_check_highlighting PARAM_NAMES((ui,surrogate_list))
    lwk_dxm_ui ui PARAM_SEP
    BMI_SURROGATE_LIST_PTR surrogate_list PARAM_END
/*
 *
 * Function description:
 *
 *      Scan through a surrogate list to see if the object associated
 *      with it should be highlighted.  
 *
 * Arguments:
 *
 *      ui - the HIS use interface for the window displaying the object.
 *
 *      surrogate_list - a linked list of surrogates to be checked
 *
 * Return value:
 *
 *      True if a surrogate meets the highlighting criteria for the ui.
 *
 * Side effects:
 *
 *      None
 *
 */

{
    lwk_status status;
    lwk_boolean highlighted = FALSE;

    /* The highlight state is set to false and then we loop through the
     * surrogates until we find one that indicates that the object that
     * these surrogates describe should be highlighted, or we hit the
     * end of the list.
     */
    while (surrogate_list && !highlighted) {
        status = lwk_surrogate_is_highlighted(ui,
                                              surrogate_list->surrogate,
                                              &highlighted
                                              );
        surrogate_list = surrogate_list->next;
    }
    return highlighted;

} /* end bmi_check_highlighting () */


static lwk_termination
BuildNetworkList PARAM_NAMES((closure,cnet,domain,network))
    lwk_closure closure PARAM_SEP
    lwk_composite_linknet cnet PARAM_SEP
    lwk_domain domain PARAM_SEP
    lwk_linknet *network PARAM_END

/*
 *
 * Function description:
 *
 *      Callback routine for lwk_iterate to delete the networks
 *      in a composite network.
 *
 * Arguments:
 *
 *      closure - the window context for the library window
 *
 *      object - the object that we are iterating on, not used
 *
 *      domain - domain of the value, should be lwk_c_domain_linknet
 *
 *      value  - the value of the network object to delete.
 *
 * Return value:
 *
 *      Termination value of 0 if successful, 1 otherwise.
 *
 * Side effects:
 *
 *      None
 *
 */

{
    BMI_NETWORK_LIST_PTR networks;

    networks = bmi_context.active_networks;

    while (networks != NULL) {
        if (networks->network == *network) {
            return (lwk_termination) 0;
        }
        networks = networks->next;
    }

    networks = (BMI_NETWORK_LIST_PTR)BKR_MALLOC(sizeof(BMI_NETWORK_LIST));
    if (networks == NULL) {
        return (lwk_termination) 1;
    }

    networks->network = *network;
    networks->next = bmi_context.active_networks;
    bmi_context.active_networks = networks;

    return (lwk_termination) 0;

} /* end BuildNetworkList () */

lwk_status
bmi_delete_composite_network(VOID_PARAM)
/*
 *
 * Function description:
 *
 *      Routine to delete the current composite network and the
 *      networks it contains.  This should be done when the current
 *      composite network chnages.
 *
 * Arguments:
 *
 *      None
 *
 * Return value:
 *
 *      An lwk_status value indicating the status of the delete.
 *
 * Side effects:
 *
 *      None
 *
 */

{
    lwk_status status;
    BMI_NETWORK_LIST_PTR networks;
    BKR_BOOK_CTX_PTR book;

    TRACE("bmi_delete_composite_network");

    /* Delete the old composite network.
     */
    if (bmi_context.current_composite_net != lwk_c_null_object) {
        RETURN_ON_ERROR(lwk_delete(&bmi_context.current_composite_net) );
        bmi_context.current_composite_net = lwk_c_null_object;
    }

    /* Delete its component networks.
     */
    networks = bmi_context.active_networks;
    while (networks != NULL) {
        BMI_NETWORK_LIST_PTR temp;
        (void)lwk_delete(&networks->network);
        temp = networks;
        networks = networks->next;
        BKR_FREE(temp);
    }
    bmi_context.active_networks = NULL;

    /* Get rid of all of the cached surrogates from the old composite
     * network.
     */
    if (bkr_library->u.library.shelves_queried) {
        bmi_delete_shelf_surrogates(bkr_library->u.library.root_of_tree);
        bkr_library->u.library.shelves_queried = FALSE;
    }

    /* Now loop through all the open books and do the same for any
     * associated windows.
     */
    book = bkr_open_books;
    while (book) {
        if (book->queried_chunks) {
            bmi_delete_surrogate_tree(&book->chunk_surrogates);
            book->queried_chunks = FALSE;
        }
        if (book->queried_directories) {

            BKR_DIR_ENTRY_PTR dirctx = book->directories;

            while (dirctx) {
                bmi_delete_surrogate_tree(&dirctx->u.directory.surrogates);
                dirctx = dirctx->sibling;
            }
            book->queried_directories = FALSE;
        }
    	book = book->next;
    }

    return lwk_s_success;

} /* end delete_current_composite_net () */

lwk_status
bmi_update_composite_network(VOID_PARAM)
/*
 *
 * Function description:
 *
 *      Routine to delete the current composite network and the
 *      networks it contains.  This should be done when the current
 *      composite network chnages.
 *
 * Arguments:
 *
 *      none
 *
 * Return value:
 *
 *      An lwk_status value indicating the status of the delete.
 *
 * Side effects:
 *
 *      None
 *
 */
{
    lwk_status status;
    lwk_termination termination;
    BMI_NETWORK_LIST_PTR networks;

    TRACE("bmi_update_composite_network");

    /* First delete the old composite network and associated objects.
     */
    bmi_delete_composite_network();

    /* Now get the new composite network.
     */
    status = lwk_get_value(bkr_library->memex_ui,
                           lwk_c_p_active_comp_linknet,
                           lwk_c_domain_comp_linknet,
                           &bmi_context.current_composite_net
                          );
    RETURN_ON_ERROR(status);

    /* Get the component networks and build our list of networks.
     */
    if (bmi_context.current_composite_net != lwk_c_null_object) {
        lwk_termination termination;
        status = lwk_iterate(bmi_context.current_composite_net,
                             lwk_c_domain_linknet,
                             (lwk_closure)0,
                             BuildNetworkList,
                             &termination);
        RETURN_ON_ERROR(status);
    }

    /* Flag that the composite network is valid
     */
    bmi_context.composite_net_valid = TRUE;
    return lwk_s_success;

} /* end bmi_update_composite_network () */



static lwk_status
apply_callback PARAM_NAMES((ui,reason,dxm_info,user_data,surrogate,operation,follow_type))
    lwk_ui ui PARAM_SEP
    lwk_reason reason PARAM_SEP
    XmAnyCallbackStruct *dxm_info PARAM_SEP
    lwk_closure user_data PARAM_SEP
    lwk_surrogate surrogate PARAM_SEP
    lwk_string operation PARAM_SEP
    lwk_follow_type follow_type PARAM_END

/*
 *
 * Function description:
 *
 *      The apply callback for the hyperinformation services. This is the only
 *      one for the bookreader and is associated with the lwk_ui for the
 *      library window.  This handles applys for all bookreader surrogate 
 *      objects.
 *
 * Arguments:
 *
 *      ui - the user interface that generated the callback
 *
 *      user_data - the user_data specified in the ui, not used
 *
 *      surrogate - the surrogate that we are to operate on
 *
 *      operation - this should only be "View", not currently checked
 *
 *      follow_type - not currently used
 *
 * Return value:
 *
 *      An lwk_status value indicating the success or failure of the apply.
 *
 * Side effects:
 *
 *      None
 *
 */

{

    lwk_status status;
    lwk_status confirm_status;
    lwk_string container = NULL;
    lwk_string type = NULL;

    TRACE("apply_callback");

    /* Get the object type property so we will know what we are dealing with.
     */
    status = GET_STRING(surrogate,BMI_PROP_OBJECT_TYPE,&type);

    /* All bookreader surrogate object have a container property, so go ahead
     * and get that.
     */
    status = GET_STRING(surrogate,BMI_PROP_CONTAINER,&container);

    if ((status != lwk_s_success) || (container == lwk_c_null_object)) {

    } else if (strcmp(type,BMI_OBJ_NAME_BOOK) == 0) {
        status = bmi_book_open_to_target(container,BMI_OBJ_BOOK,surrogate,operation);
    } else if (strcmp(type,BMI_OBJ_NAME_DIRECTORY) == 0) {
        status = bmi_book_open_to_target(container,BMI_OBJ_DIRECTORY,surrogate,operation);
    } else if (strcmp(type,BMI_OBJ_NAME_CHUNK) == 0) {
        status = bmi_book_open_to_target(container,BMI_OBJ_CHUNK,surrogate,operation);
    } else if (strcmp(type,BMI_OBJ_NAME_SHELF) == 0) {

        status = bmi_shelf_open_to_entry((BKR_WINDOW_PTR)user_data,container,surrogate);
    } else {
        status = lwk_s_failure;
    }
    (void)lwk_delete_string(&container);
    (void)lwk_delete_string(&type);
    
    /* Now confirm to the HIS that we did the apply.
     */
    confirm_status = lwk_confirm_apply(ui,surrogate);
    if (confirm_status != lwk_s_success) {
        return confirm_status;
    }
    return status;

} /* end apply */

static lwk_status
close_view_callback PARAM_NAMES((ui,reason,dxm_info,user_data))
    lwk_ui ui PARAM_SEP
    lwk_reason reason PARAM_SEP
    XmAnyCallbackStruct *dxm_info PARAM_SEP
    lwk_closure user_data PARAM_END

/*
 *
 * Function description:
 *
 *      The generic HIS close view callback.  This routine handles
 *      closing  all types of bookreader views/windows.
 *
 * Arguments:
 *
 *      ui - The HIS user interface, not used in this case.
 *
 *      user_data - The user data specified when the user interface
 *                  was created.  In this case it is the address of
 *                  a BKR_WINDOW structure.
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
    register BKR_WINDOW *window = (BKR_WINDOW *)user_data;

    if (window->appl_shell_id != NULL)
    {
    	ICONIFY_WINDOW(window->appl_shell_id);
    }

    return lwk_s_success;

} /* end close_view */


void
bmi_insert_highlight_icon PARAM_NAMES((svn,entry_num,component,x_pos))
    Widget svn PARAM_SEP
    int    entry_num PARAM_SEP
    int    component PARAM_SEP
    int    x_pos PARAM_END

/*
 *
 * Function description:
 *
 *      Utility routine to insert the LWK icon in an SVN entry.
 *
 * Arguments:
 *
 *      svn       - the SVN widget containing the entryto behighlighted
 *      entry_num - number of the entry
 *      component - which component in the entry
 *      x_pos     - X offset in the entry
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
    	DXmSvnSetComponentPixmap(svn, 
                                 entry_num, 
                                 component,
                                 x_pos,
                                 0, 	       	  /* y position       */
                                 bmi_context.highlight_icon,
                                 bmi_context.highlight_icon_width,
                                 bmi_context.highlight_icon_height
                                 );

} /* end bmi_insert_highlight_icon */


static void
update_all_highlighting(VOID_PARAM)
/*
 *
 * Function description:
 *
 *      This routine updates the highlighting for all windows
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
 *      
 *
 */

{
    BKR_SHELL_PTR  shell;
    BKR_WINDOW_PTR window;

    TRACE("update_all_highlighting");

    /* First do the library window.
     */
    (*bkr_library->update_highlighting)(bkr_library,TRUE);

    /* Now loop through all the book shells and do the same for any
     * associated windows.
     */
    shell = bkr_all_shells;
    while (shell) {

        window = shell->other_topics;
        while (window) {
            (*window->update_highlighting)(window,TRUE);
            window = window->u.topic.sibling;
        }

        window = shell->default_topic;
        if (window) {
            (*window->update_highlighting)(window,TRUE);
        }

        window = shell->selection;
        if (window) {
            (*window->update_highlighting)(window,TRUE);
        }

    	shell = shell->all_shells;
    }

} /* end update_all_highlighting() */



static lwk_status
currency_change_callback PARAM_NAMES((ui,reason,dxm_info,user_data,currency))
    lwk_ui ui  PARAM_SEP
    lwk_reason reason PARAM_SEP
    XmAnyCallbackStruct *dxm_info PARAM_SEP
    lwk_closure user_data PARAM_SEP
    lwk_environment_change currency PARAM_END

/*
 *
 * Function description:
 *
 *      The generic HIS currency change callback.  The same routine is
 *      used for all of the different window/object types, but the UI
 *      for the library window may be the only one to take action on
 *      certain currency changes.
 *
 * Arguments:
 *
 *      ui - The HIS user interface, not used in this case.
 *
 *      user_data - The user data specified when the user interface
 *                  was created.  In this case it is the address of
 *                  a BKR_WINDOW structure.
 *
 *      currency - what changed.
 *
 * Return value:
 *
 *      Returns lwk_s_success if we ignore the change or we can respond
 *      to it correctly, otherwise return an appropriate error (probably
 *      from some HIS call).
 *
 * Side effects:
 *
 *      May change current networks, highlighting, etc.
 *
 */
{
    lwk_status status = lwk_s_success;
    register BKR_WINDOW_PTR window = (BKR_WINDOW_PTR)user_data;

    switch (currency) {

        case lwk_c_env_active_comp_linknet: {
            CURRENCY_CHANGE("current composite network");
            /* The current composite network changed so delete
             * the old composite network associated with this UI,
             * get the new one, and then call the query routine specified
             * in the window context for this UI to find the appropriate
             * surrogates in the new composite network.
             *
             * Only the library window will handle this currency change.
             */
            if (window->type == BKR_LIBRARY) {
                bmi_context.composite_net_valid = FALSE;
                update_all_highlighting();
                break ;
            }
        }

        case lwk_c_env_recording_linknet: {
            CURRENCY_CHANGE("current network");
            break;
        }

        case lwk_c_env_appl_highlight: {
            CURRENCY_CHANGE("current highlighting");

            /* The current highlighting changed so get the new value
             * and update the highlighting for the objects associated
             * with this window context.
             */
            (*window->update_highlighting)(window,TRUE);

            window->using_default_highlighting = FALSE;
            break;
        }
        case lwk_c_env_new_link: {
            CURRENCY_CHANGE("current connection");

            /* Update the highlighting for the the objects assocated with
             * this memex context, if we are supposed to highlight the
             * current connection.
             */
            if (HIGHLIGHTING_ON(window)) {
                (*window->update_highlighting)(window,TRUE);
            }
            break;
        }
        case lwk_c_env_follow_destination: {
            CURRENCY_CHANGE("current destination");
            
            /* Update the highlighting for the the objects assocated with
             * this memex context, if we are supposed to highlight the
             * current destination.
             */
            if (HIGHLIGHT(window,lwk_c_hl_destination_of_follow)) {
                (*window->update_highlighting)(window,TRUE);
            }
            break;
        }
        case lwk_c_env_pending_source: {
            CURRENCY_CHANGE("current source");
            
            /* Update the highlighting for the the objects assocated with
             * this memex context, if we are supposed to highlight the
             * current source.
             */
            if (HIGHLIGHT(window,lwk_c_hl_pending_source)) {
                (*window->update_highlighting)(window,TRUE);
            }
            break;
        }
        case lwk_c_env_pending_target: {
            CURRENCY_CHANGE("current target");
            
            /* Update the highlighting for the the objects assocated with
             * this memex context, if we are supposed to highlight the
             * current target.
             */
            if (HIGHLIGHT(window,lwk_c_hl_pending_target)) {
                (*window->update_highlighting)(window,TRUE);
            }
            break;
        }
        case lwk_c_env_default_highlight: {
            CURRENCY_CHANGE("default highlighting");
            
            /* The library window always repsonds by getting the
             * select destination setting, any other window will
             * respond and change it's highlighting if it is still
             * using the default highlighting.
             */
            if ((window->using_default_highlighting) 
                || (window->type == BKR_LIBRARY)) {
                status = get_default_highlighting(window);
            }
            break;
        }
        
        /**********************************************
         * We don't do anything with these right now. *
         ********************************************** 
         */
        case lwk_c_env_active_comp_path:
            CURRENCY_CHANGE("current composite path");
            (*window->update_highlighting)(window,TRUE);
            break;
        case lwk_c_env_active_path:
            CURRENCY_CHANGE("current path");
            (*window->update_highlighting)(window,TRUE);
            break;
        case lwk_c_env_followed_link:
            CURRENCY_CHANGE("current follow");
            (*window->update_highlighting)(window,TRUE);
            break;
        case lwk_c_env_followed_step:
            CURRENCY_CHANGE("current step");
            (*window->update_highlighting)(window,TRUE);
            break;
        case lwk_c_env_recording_path:
            CURRENCY_CHANGE("current trail");
            (*window->update_highlighting)(window,TRUE);
            break;
        case lwk_c_env_default_relationship:
            CURRENCY_CHANGE("default connection type");
            break;
        case lwk_c_env_default_operation:
            CURRENCY_CHANGE("default operation");
            break;
        case lwk_c_env_default_retain_source:
            CURRENCY_CHANGE("default retain source");
            break;
        case lwk_c_env_default_retain_target:
            CURRENCY_CHANGE("default retain target");
            break;
    }
    return lwk_s_success;
}

void 
bmi_select_svn_entry PARAM_NAMES((window,tag))
    BKR_WINDOW_PTR window PARAM_SEP
    unsigned tag PARAM_END
{
    unsigned             svn_entry_num;
    DXmSvnCallbackStruct unused_callback_data;
    
    DXmSvnDisableDisplay(window->widgets[W_SVN]);
    svn_entry_num = DXmSvnGetEntryNumber(window->widgets[W_SVN],tag);
    DXmSvnClearSelections(window->widgets[W_SVN]);
    DXmSvnSelectEntry(window->widgets[W_SVN],svn_entry_num);
    if (window->type == BKR_SELECTION) {
        bkr_selection_transitions_done(window->widgets[W_SVN],
                                       window,
                                       &unused_callback_data
                                       );
    }
    else 
    {
        int                 unused_tag;

        bkr_library_transitions_done(window->widgets[W_SVN],
                                     &unused_tag,
                                     &unused_callback_data);
    }
    DXmSvnEnableDisplay(window->widgets[W_SVN]);
}

void
bmi_save_connection_menu PARAM_NAMES((connection_menu,user_data,callback_data))
    Widget connection_menu PARAM_SEP
    caddr_t user_data PARAM_SEP
    XmAnyCallbackStruct *callback_data PARAM_END

/*
 *
 * Function description:
 *
 *      Routine to save the connection menu widget and create the
 *      BMI window context when a directory or topic window is
 *      created.
 *
 * Arguments:
 *
 *      connection_menu - the connection menu widget
 *
 *      user_data - the pointer to the BKR_WINDOW containing the
 *                  connection menu
 *
 *      callback_data - not used 
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
    BKR_WINDOW_PTR window = (BKR_WINDOW_PTR)user_data;

    window->highlighting = lwk_c_hl_none;
    window->connection_menu = connection_menu;

} /* end bmi_save_connection_menu */


void
bmi_delete_ui PARAM_NAMES((window))
    BKR_WINDOW_PTR window PARAM_END
/*
 *
 * Function description:
 *
 *     This deletes the DECwindows User Interface object for a window context
 *     which breaks the connection to the hyperinformation services
 *     for that context, i.e. it will receive no more callbacks.
 *
 * Arguments:
 *
 *      window - the memex context
 *
 * Return value:
 *
 *      The status from lwk_delete().
 *
 * Side effects:
 *
 *      The composite network and network objects will be deleted too.
 *
 */

{
    if (window && (window->memex_ui))
    {
        (void)lwk_delete(&window->memex_ui);
        window->memex_ui = NULL;
    }
} /* end bmi_delete_ui () */


lwk_status
bmi_create_memex_ui PARAM_NAMES((window,create_surrogate_cb,get_surrogate_cb))
    BKR_WINDOW *window  PARAM_SEP
    lwk_callback create_surrogate_cb PARAM_SEP
    lwk_callback get_surrogate_cb PARAM_END

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
    static XmString cs_name = NULL;
#ifdef I18N_MULTIBYTE
    long cs_length;
    long cs_status;
#endif
    lwk_status status;
    
    if (cs_name == NULL) {
#ifdef I18N_MULTIBYTE
        cs_name = DXmCvtFCtoCS( BKR_APPLICATION_CLASS, &cs_length, &cs_status );
#else
        cs_name = XmStringCreateSimple( BKR_APPLICATION_CLASS );
#endif
    }

    if (window->memex_ui) {
#ifdef MEMEX_DEBUG
        fprintf(stderr,"UI already active\n");
#endif
        bmi_delete_ui(window);
    }

    status = lwk_create_dxm_ui(cs_name,
                              lwk_c_true,
                              lwk_c_true,
                              window->widgets[W_MAIN_WINDOW],
                              window->connection_menu,
                              &window->memex_ui
                              );
    RETURN_ON_ERROR(status);

    status = lwk_set_value(window->memex_ui, 
                           lwk_c_p_user_data,
                           lwk_c_domain_closure,
                           &window,
                           lwk_c_set_property
                           );
    RETURN_ON_ERROR(status);

    /* We need to do extra setup of the library window user interface.
     */
    if (window->type == BKR_LIBRARY) {

        lwk_string string;

        /* Add the Bookreader's supported surrogates.
         */
        string = BMI_SURROGATE_SUBTYPE;
        status = lwk_set_value(window->memex_ui,
                               lwk_c_p_supported_surrogates,
                               lwk_c_domain_string,
                               &string,
                               lwk_c_add_property
                               );
        RETURN_ON_ERROR(status);

        /* Add the supported operations, only view.
         */
        string = BMI_OPERATION_VIEW;
        status = lwk_set_value(window->memex_ui,
                               lwk_c_p_supported_operations,
                               lwk_c_domain_string,
                               &string,
                               lwk_c_add_property
                               );
        RETURN_ON_ERROR(status);
        string = BMI_OPERATION_VIEW_NEW;
        status = lwk_set_value(window->memex_ui,
                               lwk_c_p_supported_operations,
                               lwk_c_domain_string,
                               &string,
                               lwk_c_add_property
                               );
        RETURN_ON_ERROR(status);
        string = BMI_OPERATION_VIEW_DEFAULT;
        status = lwk_set_value(window->memex_ui,
                               lwk_c_p_supported_operations,
                               lwk_c_domain_string,
                               &string,
                               lwk_c_add_property
                               );
        RETURN_ON_ERROR(status);
        string = BMI_OPERATION_CLOSE;
        status = lwk_set_value(window->memex_ui,
                               lwk_c_p_supported_operations,
                               lwk_c_domain_string,
                               &string,
                               lwk_c_add_property
                               );
        RETURN_ON_ERROR(status);

        /* Set the apply callback.
         */
        SET_CALLBACK(window,lwk_c_p_apply_cb,apply_callback);
    }

    RETURN_ON_ERROR(status);


    /* Set the callbacks for this user interface.
     */
    SET_CALLBACK(window,lwk_c_p_environment_change_cb,currency_change_callback);
    SET_CALLBACK(window,lwk_c_p_close_view_cb,close_view_callback);
    SET_CALLBACK(window,lwk_c_p_create_surrogate_cb,create_surrogate_cb);
    SET_CALLBACK(window,lwk_c_p_get_surrogate_cb,get_surrogate_cb);

    /* Now that the callbacks, particularly the currency change
     * callback is setup, set the current highlighting for this
     * user interface.  This is initially the default highlighting.
     *
     * This will cause the currency change to be invoked and the 
     * highlighting for the surrogates to be set if necessary.
     */
    window->using_default_highlighting = TRUE;
    status = get_default_highlighting(window);

    return status;

} /* end bmi_create_memex_ui () */

void
bmi_visit PARAM_NAMES((window))
    BKR_WINDOW_PTR window PARAM_END
/*
 *
 * Function description:
 *
 *      Perform a "Visit" operation on a window.
 *
 * Arguments:
 *
 *      window - pointer to the BKR_WINDOW structure that operation is
 *               being preformed on
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
    /* The UI was not set up correctly, just return.
     */
    if (window && window->memex_ui)
    {
        lwk_do_dxm_menu_action(window->memex_ui,lwk_c_dxm_menu_visit);
    }

}   /* end of bmi_visit */

Boolean
bmi_initialize_his(VOID_PARAM)
/*
 *
 * Function description:
 *
 *      Initialize the context for the Bookreader Memex Interfaces,
 *      then  initialize the hyperinformation services.
 *
 * Arguments:
 *
 *      None
 *
 * Return value:
 *
 *      Returns the state of the hyperinvoked parameter to lwk_initialize()
 *      if that succeeds, FALSE otherwise.
 *
 * Side effects:
 *
 *      None
 *
 */

{
    lwk_status status;
    char *uid_file_path;

    TRACE("bmi_initialize_his")

    /* Initialize the context that the bookreader memex interfaces use.
     */
    bmi_context.initialized = FALSE;
    bmi_context.hyperinvoked = FALSE;
    bmi_context.operation = NULL;
    bmi_context.surrogate = lwk_c_null_object;
    bmi_context.composite_net_valid = FALSE;
    bmi_context.current_composite_net = lwk_c_null_object;
    bmi_context.active_networks = NULL;

    if (bkrplus_g_charcell_display) 
    {
        /* We currently don't support this in character cell mode
         */
        return FALSE;
    }

#ifndef VMS
    uid_file_path = XtResolvePathname(bkr_display,
                                      "uid",
                                      "lwk_dxmui",
                                      ".uid",
                                      NULL,
                                      NULL,
                                      0,
                                      NULL);

    if (uid_file_path == NULL) {
        return FALSE;
    }
#endif 

    /* Initialize the hyperinformation services.  Not that the return
     * parameters are store in the context for later use after the library
     * window lwk_dxm_ui is created.
     */
    status = lwk_initialize(&bmi_context.hyperinvoked,
                            &bmi_context.operation,
                            &bmi_context.surrogate);

    if (status == lwk_s_success) {

        /* The initialization was successful,  not that the services are
         * initialized and return whether bookreader was hyperinvoked.
         */
        bmi_context.initialized = TRUE;
        return (Boolean)bmi_context.hyperinvoked;

    } else {

        /* The initialization did not succeed
         */
        PRINT_STATUS("lwk_initialize",status)
        bmi_context.initialized = FALSE;
        return FALSE;
    }

} /* end bmi_initialize_his */

void
bmi_install_no_linkworks_menu PARAM_NAMES((window))
    BKR_WINDOW *window PARAM_END
{
    unsigned        status;
    MrmType	    dummy_class;
    Widget          no_linkworks_menu;

    if ((window == NULL) || (window->connection_menu == NULL)) {
        return;
    }

    if (bkrplus_g_charcell_display)
    {
        /* Since there ar no other LinkWorks clients available, we
         * make the Link menu unavailable by unmanaging it.
         */
        if (XtIsManaged(window->connection_menu)) 
        {
            XtUnmanageChild(window->connection_menu);
        }
        return;
    }
    /* Update the DRM tag and fetch the FILE pulldown menu */

    BKR_UPDATE_TAG( window );

    status = MrmFetchWidget(
    	    	bkr_hierarchy_id,
    	    	"noLinkWorksMenu",    	    	    /* index into UIL */
    	    	window->widgets[W_MENU_BAR],	    /* parent widget  */
                &no_linkworks_menu,	            /* widget fetched */
    	    	&dummy_class);	    	    	    /* unused class   */
    if ( status != MrmSUCCESS )
    {
    	char	*error_string;
	error_string = (char *) bkr_fetch_literal( "FETCH_WIDGET_ERROR", ASCIZ );
    	if ( error_string != NULL )
    	{
	    sprintf( errmsg, error_string, "noLinkWorksMenu" );
	    bkr_error_modal( errmsg, NULL );
	    XtFree( error_string );
    	}
    }
    else 
    {
        /* Associate the pulldown menu with the pulldown menu entry 
         */
        Arg arglist[5];
        int argcnt;

        argcnt = 0;
        SET_ARG( XmNsubMenuId, no_linkworks_menu);
        XtSetValues( window->connection_menu, arglist, argcnt );
    }
}

void 
bmi_no_linkworks_msg_display PARAM_NAMES((menu_item,user_data,callback_data))
    Widget menu_item PARAM_SEP
    caddr_t user_data PARAM_SEP
    XmAnyCallbackStruct *callback_data  PARAM_END

{
    BKR_WINDOW *window = (BKR_WINDOW *)user_data;
    char *error_string = (char *)bkr_fetch_literal("NO_LINKWORKS_MSG", ASCIZ );
    
    if (error_string) {
        bkr_error_modal(error_string,XtWindow(window->appl_shell_id ));
        XtFree(error_string);
    }
}


