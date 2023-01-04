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
/* DEC/CMS REPLACEMENT HISTORY, Element BMI_DIRECTORY.C*/
/* *7     9-JUN-1992 10:00:18 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *6    10-MAY-1992 21:11:12 BALLENGER "Add linkworks not installed message."*/
/* *5     8-MAY-1992 20:26:59 BALLENGER "temporary fix for LinkWorks not installed...."*/
/* *4     3-MAR-1992 17:07:04 KARDON "UCXed"*/
/* *3    13-FEB-1992 13:26:52 BALLENGER "Fix ACCVIOs when deleting links or (de)activating link nets, etc."*/
/* *2     1-NOV-1991 12:47:59 BALLENGER "Reintegrate  memex support"*/
/* *1    16-SEP-1991 12:41:13 PARMENTER "Linkworks Interfaces"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BMI_DIRECTORY.C*/
#ifndef VMS
 /*
#else
#module BMI_DIRECTORY "V03-0008"
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
**      with directories.
**
**  AUTHORS:
**
**      David L Ballenger
**
**  CREATION DATE:     ??-Jul-1990
**
**  MODIFICATION HISTORY:
**
**  V03-0008  DLB0004     David L Ballenger           30-Apr-1991
**            Fix problems with select destination and visiting when
**            double-clicking on the LinkWorks icon.
**
**  V03-0007  DLB0003     David L Ballenger           07-Apr-1991
**            Fix problem with non-TOC directory highlighting and
**            incorrect return of error status when nothing is
**            selected on get/create surrogate callbacks.
**
**  V03-0006  DLB0002     David L Ballenger           01-Mar-1991
**            Fix problems with surrogate highlighting after the composite
**            network changes for QAR 807.  LinkWorks naming convention 
**            changes.
**
**  V03-0005  DLB0001     David L Ballenger           08-Feb-1991
**            Fix problems with surrogate highlighting after the composite
**            network changes.
**
**  V03-0004  JAF0004	    James A. Ferguson   	5-Dec-1990
**  	      Add bmi_directory_visit_entry to do a "Visit" operation
**  	      on a given directory entry.
**
**  V03-0003  JAF0003	    James A. Ferguson   	2-Dec-1990
**  	      Call bkr_selection_update_entry instead of 
**  	      bkr_selection_highlight_entry.
**
**  V03-0002  JAF0002	    James A. Ferguson   	14-Aug-1990
**  	      Modify bmi_update_dir_highlighting and 
**  	      clear_dir_highlighting to call bkr_selection_highlight_entry.
**  	      Also add redisplay parameter to bmi_update_dir_highlighting.
**
**  V03-0001  JAF0001	    James A. Ferguson   	27-Jul-1990
**  	      Update find_dir_entry, create_directory_surrogate
**  	      clear_dir_highlighting, and bmi_update_dir_highlighting
**  	      to support new SVN Selection window in BL5.
**
**--
**/


#include "br_common_defs.h"  /* common BR #defines */ 
#include "br_meta_data.h"    /* typedefs and #defines for I/O of BR files */
#include "br_typedefs.h"     /* BR high-level typedefs and #defines */
#include "br_malloc.h"       /* BKR_MALLOC, etc defined here */
#include "br_globals.h"      /* BR external variables declared here */
#include "bkr_selection.h"
#include "bmi_private_defs.h"
#include "bmi_surrogate_defs.h"
#include "bmi_book.h"
#include "bmi_directory.h"
#include "bmi_query.h"
#include "bmi_surrogate.h"
#include "bmi_user_interface.h"

static lwk_termination
save_directory_surrogates PARAM_NAMES((user_data,network,domain,surrogate))
    lwk_closure user_data PARAM_SEP
    lwk_composite_linknet network PARAM_SEP
    lwk_integer domain PARAM_SEP
    lwk_surrogate *surrogate PARAM_END

/*
 *
 * Function description:
 *
 *      His_query callback routine to save directory entry surrogates
 *      that the query found.
 *
 * Arguments:
 *
 *      All arguments are specified by the lwk_query() routine.  Only the
 *      user_data argument is used to pass the book context;
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
    BKR_DIR_ENTRY_PTR dir = book->directories;
    lwk_integer entry;
    lwk_string name;
    lwk_status status;

    /* Get the directory name from the surrogate
     */
    status = GET_STRING(*surrogate,BMI_PROP_DIR_NAME,&name);
    if (status != lwk_s_success) {
        return 0;
    }

    /* See if we have a directory that has the same name.
     */
    while (dir && (strcmp(dir->title,name) != 0))
    {
        dir = dir->sibling;
    }
    (void)lwk_delete_string(&name);

    if (dir) {
        
        /* Have a matching directory, get the entry number and save
         * the surrogate if the directory entry is specified.
         */
        status = GET_INTEGER(*surrogate,BMI_PROP_DIR_ENTRY,&entry);
        if (status == lwk_s_no_such_property) {
            entry = 0;
        } else {
            if (status == lwk_s_success) {
                status = bmi_check_book_timestamp(*surrogate,book);
                if ((status != lwk_s_no_such_property) 
                    && (status != lwk_s_success)
                    ) {
                    entry = 0;
                }
            } else {
                return 0;
            }
        }
        bmi_save_surrogate_in_tree(&dir->u.directory.surrogates,*surrogate,entry);     
    }
    return 0;

} /* end save_directory_surrogates () */


static Boolean
query_for_directories PARAM_NAMES((window))
    BKR_WINDOW_PTR window PARAM_END
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
    register BKR_BOOK_CTX_PTR book;
    register unsigned j;
    register BKR_DIR_ENTRY_PTR dir;
    lwk_status status;

    book = window->shell->book;

    if (bmi_context.composite_net_valid && book->queried_directories) {
        return FALSE;
    }

    dir = book->directories;
    while (dir) {
        if (dir->u.directory.surrogates) {
            bmi_delete_surrogate_tree(&dir->u.directory.surrogates);
            dir->u.directory.surrogates = NULL;
        }
        dir = dir->sibling;
    }

    status = bmi_query_book(book,
                            BMI_OBJ_NAME_DIRECTORY,
                            (lwk_callback *)save_directory_surrogates);
    if (status == lwk_s_success) {
        book->queried_directories = TRUE;
    }
    return TRUE;

} /* end query_for_directories () */

static BKR_DIR_ENTRY_PTR
find_selected_dir_entry PARAM_NAMES((window))
    BKR_WINDOW_PTR window PARAM_END
/*
 *
 * Function description:
 *
 *      Find the selected directory entry in the selection window.
 *
 * Arguments:
 *
 *      window       - The window context for the selection window
 * 
 * Return value:
 *
 *      Returns the pointer to the first selected directory entry.
 *
 * Side effects:
 *
 *      None
 *
 */

{
    if (window->u.selection.num_selected == 0)
    {
        return NULL;
    } 
    else 
    {
        return (BKR_DIR_ENTRY_PTR)window->u.selection.selected_entry_tags[0];
    }
} /* end find_dir_entry () */


static lwk_status
create_directory_surrogate 
    PARAM_NAMES((ui,reason,dxm_info,user_data,surrogate_rtn))
    lwk_ui ui PARAM_SEP 
    lwk_reason reason PARAM_SEP
    XmAnyCallbackStruct *dxm_info PARAM_SEP
    lwk_closure user_data PARAM_SEP
    lwk_surrogate *surrogate_rtn PARAM_END
{

    BKR_WINDOW_PTR window;
    BKR_BOOK_CTX_PTR book;
    BMI_SURROGATE_LIST_PTR list;
    BKR_DIR_ENTRY_PTR selected_entry;
    BKR_DIR_ENTRY_PTR parent_dir;
    BMD_OBJECT_TYPE entry_type;
    unsigned entry_num;
    lwk_status status;
    lwk_string string;
    lwk_surrogate surrogate;
    char desc[255];
    

    window = (BKR_WINDOW_PTR)user_data;
    book = window->shell->book;

    (void)query_for_directories(window);

    /* Find the current directory and the selected entry in it.
     */
    selected_entry = find_selected_dir_entry(window);
    if (selected_entry == NULL) {

        /* Nothing was selected, so return the NULL object.
         */
        *surrogate_rtn = lwk_c_null_object;
        return lwk_s_success;
    }

    if (selected_entry->entry_type == DIRECTORY) 
    {
        parent_dir = selected_entry;
    }
    else 
    {
        parent_dir = bkr_selection_find_entry_by_id(book->directories,
                                                    BRI_DIRECTORY_ID(selected_entry->object_id));
    }

    entry_num = BRI_DIRECTORY_ENTRY_NUMBER(selected_entry->object_id);
    list = bmi_find_surrogate_in_tree(parent_dir->u.directory.surrogates,entry_num);

    /* See if we have any existing surrogates.
     */
    surrogate = bmi_find_existing_surrogate(window,list,&status);
    if (surrogate) {
        *surrogate_rtn = surrogate;
        return lwk_s_success;
    } else {
        RETURN_ON_ERROR(status);
    }

    /*  No known Surrogate, so create one.
     */
    if (parent_dir == selected_entry) 
    {
        /* A Directory itself was selected not a directory entry.
         */
        sprintf(desc,"%s:  %s",book->title,parent_dir->title);

    } 
    else
    {
        /* Describe the specified directory entry.
         */
        sprintf(desc,"%s:  %s:  %s",book->title,parent_dir->title, selected_entry->title);
    }

    /* Create the basic surrogate for a directory object.
     */
    status = bmi_create_book_surrogate(book,
                                       BMI_OBJ_NAME_DIRECTORY,
                                       desc,
                                       &surrogate
                                       );
    RETURN_ON_ERROR(status);

    /* Set the directory name property.
     */
    status = lwk_set_value(surrogate,
                           BMI_PROP_DIR_NAME,
                           lwk_c_domain_string,
                           &parent_dir->title,
                           lwk_c_set_property
                           );
    RETURN_ON_ERROR(status);

    if (selected_entry != parent_dir) {

        /* An entry was selected so add the directory entry property.
         */
        status = lwk_set_value(surrogate,
                               BMI_PROP_DIR_ENTRY,
                               lwk_c_domain_integer,
                               &entry_num,
                               lwk_c_set_property
                               );
        RETURN_ON_ERROR(status);
    }

    /* Save the surrogate.
     */
    status = bmi_save_surrogate_in_tree(&parent_dir->u.directory.surrogates,surrogate,entry_num);
    RETURN_ON_ERROR(status);

    *surrogate_rtn = surrogate;
    return lwk_s_success;

} /* end create_directory_surrogate */

static lwk_status  
get_directory_surrogate PARAM_NAMES((ui,reason,dxm_info,user_data,surrogates))
    lwk_ui ui PARAM_SEP 
    lwk_reason reason PARAM_SEP
    XmAnyCallbackStruct *dxm_info PARAM_SEP
    lwk_closure user_data PARAM_SEP
    lwk_surrogate *surrogates PARAM_END

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
    BKR_WINDOW_PTR window = (BKR_WINDOW_PTR)user_data;
    BKR_DIR_ENTRY_PTR selected_entry;
    BKR_DIR_ENTRY_PTR parent_dir;
    BMI_SURROGATE_LIST_PTR list;

    if (window->active) {
        
        (void)query_for_directories(window);
        
        /* Find the current directory and the selected entry in it.
         */
        selected_entry = find_selected_dir_entry(window);
        if (selected_entry == NULL) {
            
            /* Nothing was selected, so return the NULL object.
             */
            *surrogates = lwk_c_null_object;
            return lwk_s_success;
        }
        
        if (selected_entry->entry_type == DIRECTORY) 
        {
            parent_dir = selected_entry;
        }
        else 
        {
            parent_dir = bkr_selection_find_entry_by_id(window->shell->book->directories,
                                                        BRI_DIRECTORY_ID(selected_entry->object_id));
        }
        
        list = bmi_find_surrogate_in_tree(parent_dir->u.directory.surrogates,
                                          BRI_DIRECTORY_ENTRY_NUMBER(selected_entry->object_id));
        
        return bmi_get_surrogates(list,surrogates);
    } else {

        /* The window is not active, i.e. its cached, so just return a
         * null object and a success status.
         */
        *surrogates = (lwk_object) lwk_c_null_object;
        return lwk_s_success;
        
    }
   
} /* end get_directory_surrogate */

void
bmi_update_dir_highlighting PARAM_NAMES((window,redisplay))
    BKR_WINDOW_PTR window PARAM_SEP
    Boolean redisplay PARAM_END

/*
 *
 * Function description:
 *
 *      Updates the highlighting of the current directory.
 *
 * Arguments:
 *
 *      window 	  - The address of the window context block for the bookreader
 *                  	selection shell.
 *  	redisplay - update the highlighting of Selection window entries.
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
    BKR_DIR_ENTRY_PTR dir;
    Widget svn_widget;
    BKR_DIR_ENTRY *entry;
    lwk_integer old_highlighting;
    lwk_status status;
    Boolean refresh;

    if ((window == NULL) 
        || (window->active == FALSE)
        || (window->memex_ui == NULL)
        || (!XtIsManaged(window->widgets[W_MAIN_WINDOW]))
       ) {
        
        /* The UI was not set up correctly or the book context is not
         * set up, or the window isn't mmanaged, just return.
         */
        return;
    }

    /* Get the book context and then the directory context from that.
     */
    book = window->shell->book;

    refresh = query_for_directories(window);

    old_highlighting = HIGHLIGHTING_ON(window);
    status = GET_HIGHLIGHTING(window);
    if (status != lwk_s_success) {
        return;
    }

    refresh |= (HIGHLIGHTING_ON(window) != old_highlighting);

    dir = book->directories;

    /* This shouldn't happen, but if there are no directories just return.
     */
    if (dir == NULL) {
        return;
    }

    svn_widget = window->widgets[W_SVN];
    if (svn_widget == NULL) {
        return;
    }

    /* Loop through the directory entries and update the highlighting flag.
     */
    while (dir)
    {
    	register BKR_DIR_ENTRY *entries = dir->children;
        
        if (entries != NULL)
    	{
    	    register unsigned num_entries = dir->u.directory.num_entries;
            register unsigned entry_num;
            
            /* Update the highlight flag for each entry.
             * 
             * NOTE: entries is a 0-based array indexed by object_id - 1.
             */
            if (redisplay) {
                for (entry_num = 1 ; entry_num <= num_entries ; entry_num++ ) {
                    register BMI_SURROGATE_LIST_PTR list;
                    Boolean  current_highlight = entries[entry_num - 1].highlight;
                    
                    list = bmi_find_surrogate_in_tree(dir->u.directory.surrogates,entry_num);
                    entries[entry_num - 1].highlight = HIGHLIGHT_STATE(window->memex_ui,list);
                    if ((current_highlight != entries[entry_num - 1].highlight) 
                        || refresh
                        ) {
                        bkr_selection_update_entry(svn_widget,&entries[entry_num - 1]);
                    }
                }
            } else {
                for (entry_num = 1 ; entry_num <= num_entries ; entry_num++ ) {
                    register BMI_SURROGATE_LIST_PTR list;
                    
                    list = bmi_find_surrogate_in_tree(dir->u.directory.surrogates,entry_num);
                    entries[entry_num - 1].highlight = HIGHLIGHT_STATE(window->memex_ui,list);
                }
            }
        }
        dir = dir->sibling;
    }
}   /* end of bmi_update_dir_highlighting */


void
bmi_create_directory_ui PARAM_NAMES((window))
    BKR_WINDOW_PTR window PARAM_END
/*
 *
 * Function description:
 *
 *      Initialize the HIS User Interface for a directory window.
 *
 * Arguments:
 *
 *      shell - the BKR_SHELL (selection shell) for the window.
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

    if (bmi_context.initialized == FALSE) {

        /* This shouldn't happen, just return for now.
         */
        bmi_install_no_linkworks_menu(window);
        return;
    }
    if (window->type != BKR_SELECTION) {
        return;
    }

    window->update_highlighting = (void *)bmi_update_dir_highlighting;

    bmi_create_memex_ui(window,
                        (lwk_callback)create_directory_surrogate,
                        (lwk_callback)get_directory_surrogate);

} /* end bmi_create_directory_ui */

