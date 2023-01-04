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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_LIBRARY.H*/
/* *4    17-NOV-1992 22:51:21 BALLENGER "Special handling for Space and Return."*/
/* *3    29-OCT-1992 16:13:52 KLUM "make some modules extern to share with bkr_simple_search"*/
/* *2     3-MAR-1992 17:00:28 KARDON "UCXed"*/
/* *1    16-SEP-1991 12:45:59 PARMENTER "Function Prototypes for bkr_library.c"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_LIBRARY.H*/
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
**	Function prototypes for bkr_library.c
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
#ifndef BKR_LIBRARY_H
#define BKR_LIBRARY_H

#include "br_prototype.h"


/*
** Routines defined in bkr_library.c
*/
void	 bkr_library_attach_to_source PROTOTYPE((Widget	 widget,
                                      int 	    	 unused_tag,
                                      DXmSvnCallbackStruct *data
                                      ));
void	 bkr_library_collapse_expand PROTOTYPE((Widget 	    widget,
                                     int 	    	    *tag,
                                     XmAnyCallbackStruct    *reason
                                     ));
void 	 bkr_library_detach_from_source PROTOTYPE((Widget    widget,
                                        int 	    	     unused_tag,
                                        DXmSvnCallbackStruct *data
                                        ));
void	 bkr_library_double_click PROTOTYPE((Widget    widget,
                                  int 	    	       unused_tag,
                                  DXmSvnCallbackStruct *data
                                  ));
void	 bkr_library_free_hierarchy PROTOTYPE((BKR_NODE_PTR *node_ptr,
                                    Boolean 	    close_books
                                    ));
void 	 bkr_library_update_entry PROTOTYPE((BKR_NODE *entry));

void 	 bkr_library_get_entry PROTOTYPE((Widget    widget,
                               int 	    	    unused_tag,
                               DXmSvnCallbackStruct *data
                               ));
int 	 bkr_library_highlight_entry PROTOTYPE((Widget 	    widget,
                                     XButtonPressedEvent    *xbutton_event,
                                     int	    	    position_number,
                                     Boolean 	    	    highlight
                                     ));

int 	 bkr_library_number_books_open PROTOTYPE((void));

void	 bkr_library_open_close_node PROTOTYPE((Widget   widget,
                                     int	    	 *tag,
                                     XmAnyCallbackStruct *data
                                     ));

unsigned bkr_library_open_new_library PROTOTYPE((char *new_library_name));

void	 bkr_library_transitions_done PROTOTYPE((Widget	   widget,
                                      int 	    	   unused_tag,
                                      DXmSvnCallbackStruct *data
                                      ));

void	 bkr_library_free_node PROTOTYPE((BKR_NODE *parent,
                                          Boolean  close_open_books)); 

unsigned bkr_library_sibling_initialize PROTOTYPE((BKR_NODE_PTR node));

void	 bkr_library_update_node PROTOTYPE((BKR_NODE *node, 
                                 BKR_SHELL *shell_id, 
                                 Boolean   open
                                 ));

void 
bkr_library_keyboard_actions PROTOTYPE((Widget    svn_primary_window,
                                        XKeyEvent *event,
                                        String    *params,
                                        Cardinal  *num_params
                                          ));
#endif 

