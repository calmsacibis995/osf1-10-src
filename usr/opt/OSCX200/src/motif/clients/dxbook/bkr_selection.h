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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_SELECTION.H*/
/* *4    17-NOV-1992 22:51:35 BALLENGER "Special handling for Space and Return."*/
/* *3     3-MAR-1992 17:03:42 KARDON "UCXed"*/
/* *2    20-SEP-1991 16:13:08 BALLENGER " Fix problems with multiple, concurrent opens of a directory."*/
/* *1    16-SEP-1991 12:46:46 PARMENTER "Function Prototypes for bkr_selection.c"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_SELECTION.H*/
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
**      Bookreader User Interface (bkr)
**
**  ABSTRACT:
**
**	Function prototypes for 
**
**  AUTHORS:
**
**      David L Ballenger
**
**  CREATION DATE:     17-Apr-1990
**
**  MODIFICATION HISTORY:
**
**--
**/
#ifndef BKR_SELECTION_H
#define BKR_SELECTION_H

#include "br_prototype.h"



/*
** Routines defined in bkr_selection.c
*/
extern void
  bkr_selection_collapse_expand PROTOTYPE((Widget	       widget,
                                           caddr_t 	       data,
                                   XmAnyCallbackStruct *reason /* Not used */
                                          ));
extern void
  bkr_selection_detach_from_src PROTOTYPE((Widget	    	svn_widget,
                                           BKR_WINDOW	    	*window,
                                           DXmSvnCallbackStruct *data
                                           ));	
extern BKR_DIR_ENTRY
 *bkr_selection_display_entry PROTOTYPE((BKR_WINDOW	*window,
                                         BMD_OBJECT_ID dir_id,
                                         unsigned      entry_number
                                        ));
extern void
  bkr_selection_double_click PROTOTYPE((Widget	    	     svn_widget,
                                        BKR_WINDOW	     *window,
                                        DXmSvnCallbackStruct *data
                                       ));   
extern BKR_DIR_ENTRY
  *bkr_selection_find_entry_by_id PROTOTYPE((BKR_DIR_ENTRY *dir_list,
                                            BMD_OBJECT_ID dir_id_to_find
                                            ));
extern void
  bkr_selection_free_hierarchy PROTOTYPE((BKR_WINDOW	*window));

extern void
  bkr_selection_get_entry PROTOTYPE((Widget	    	  svn_widget,
                                     BKR_WINDOW	    	  *window,
                                     DXmSvnCallbackStruct *data
                                     ));	
extern void
  bkr_selection_highlight_entry PROTOTYPE((Widget  	    	    svn_widget,
                                  XButtonPressedEvent	    *xbutton_event,
                                  int	    	    	    position_number,
                                  Boolean 	    	    highlight
                                  ));
extern unsigned
  bkr_selection_new_hierarchy PROTOTYPE((BKR_WINDOW *window, 
                                         BMD_OBJECT_ID  dir_to_open));
extern void
  bkr_selection_popup_menu PROTOTYPE((Widget	    	   svn_widget,
                                      BKR_WINDOW 	   *window,
                                      DXmSvnCallbackStruct *data
                                     ));
extern void
  bkr_selection_position_entry PROTOTYPE((Widget	svn_widget,
                                          BKR_DIR_ENTRY *entry,
                                          int	    	new_position
                                         ));
extern void
  bkr_selection_transitions_done PROTOTYPE((Widget	    	   svn_widget,
                                            BKR_WINDOW 	   *window,
                                            DXmSvnCallbackStruct *data
                                           ));	
extern void
  bkr_selection_update_entry PROTOTYPE((Widget svn_widget,
                                         BKR_DIR_ENTRY *entry
                                       ));
extern void
  bkr_selection_view_directory PROTOTYPE((Widget	    	    widget,
                                          BKR_DIR_ENTRY   	    *entry,
                                          XmAnyCallbackStruct	    *reason
                                         ));

void 
bkr_selection_keyboard_actions PROTOTYPE((Widget    svn_widget,
                                          XKeyEvent *event,
                                          String    *params,
                                          Cardinal  *num_params
                                          ));
#endif 

