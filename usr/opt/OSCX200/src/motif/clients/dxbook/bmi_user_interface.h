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
**      Bookreader Memex Interface (bmi)
**
**  ABSTRACT:
**
**	Function prototypes for bmi_user_interface.c
**
**  AUTHORS:
**
**      David L Ballenger
**
**  CREATION DATE:     18-Oct-1991
**
**  MODIFICATION HISTORY:
**
**--
**/
#ifndef BMI_USER_INTERFACE_H
#define BMI_USER_INTERFACE_H

#include "br_prototype.h"


/*
** Routines defined in bmi_user_interface.c
*/
extern lwk_boolean bmi_check_highlighting
    PROTOTYPE((lwk_dxm_ui ui,
               BMI_SURROGATE_LIST_PTR surrogate_list));

extern lwk_status bmi_delete_composite_network
    PROTOTYPE((void));

extern lwk_status bmi_update_composite_network
    PROTOTYPE((void));

extern void bmi_insert_highlight_icon
    PROTOTYPE((Widget svn,
               int    entry_num,
               int    component,
               int    x_pos));

extern void bmi_select_svn_entry
    PROTOTYPE((BKR_WINDOW_PTR window,
               unsigned tag));

extern void bmi_save_connection_menu 
    PROTOTYPE((Widget connection_menu, 
               caddr_t user_data, 
               XmAnyCallbackStruct *callback_data));

extern void bmi_delete_ui 
    PROTOTYPE((BKR_WINDOW_PTR window));

extern lwk_status bmi_create_memex_ui
    PROTOTYPE((BKR_WINDOW *window,
               lwk_callback create_surrogate_cb,
               lwk_callback get_surrogate_cb));

void bmi_visit
    PROTOTYPE((BKR_WINDOW_PTR window));

Boolean bmi_initialize_his
    PROTOTYPE((void));

void bmi_install_no_linkworks_menu
    PROTOTYPE((BKR_WINDOW *window));

void bmi_no_linkworks_msg_display
    PROTOTYPE((Widget menu_item,
               caddr_t user_data,
               XmAnyCallbackStruct *callback_data));
#endif 



