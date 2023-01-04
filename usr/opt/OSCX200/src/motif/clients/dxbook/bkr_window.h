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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_WINDOW.H*/
/* *7    24-FEB-1993 17:48:12 BALLENGER "Fixes for large topic and Region memory leak."*/
/* *6    17-NOV-1992 22:48:26 BALLENGER "Distinguish active/inactive windows."*/
/* *5    21-SEP-1992 22:12:40 BALLENGER "    (1)   BALLENGER  4       6-SEP-1992 16:46:05 ""Fix hotspot selection for character*/
/*cell."""*/
/* *4    19-JUN-1992 20:13:08 BALLENGER "Cleanup for Alpha/OSF port"*/
/* *3    14-MAR-1992 14:14:45 BALLENGER "Fix problems with window and icon titles..."*/
/* *2     3-MAR-1992 17:06:44 KARDON "UCXed"*/
/* *1    16-SEP-1991 12:47:42 PARMENTER "Function Prototypes for bkr_window.c"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_WINDOW.H*/
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
**	Function prototypes for bkr_window.h
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
#ifndef BKR_WINDOW_H
#define BKR_WINDOW_H

#include "br_prototype.h"



/*
** Routines defined in bkr_window.c
*/
BKR_WINDOW *
bkr_window_set_names PROTOTYPE((
    BKR_WINDOW *window));

void
bkr_window_expose PROTOTYPE((
    Widget		    	widget,
    BKR_WINDOW                  *window,
    XmDrawingAreaCallbackStruct *callback_data));

void
bkr_window_graphics_expose PROTOTYPE((
    Widget		    widget,
    XEvent  	    	    *xevent,
    String  	    	    *params,
    Cardinal	    	    *num_params));

void
bkr_window_initialize_gc PROTOTYPE((
    BKR_WINDOW *window));

void
bkr_window_free_pixmaps PROTOTYPE((
    BKR_WINDOW *window));
void
bkr_window_set_icons PROTOTYPE((
    BKR_WINDOW *window));

void 
bkr_window_initialize_icons PROTOTYPE((Widget     appl_shell,
                                       XtPointer  user_data,
                                       XEvent     *event,
                                       Boolean    *continue_to_dispatch
                                       ));

extern void
bkr_window_client_message PROTOTYPE((Widget  	appl_shell,
                                     XtPointer  user_data,
                                     XEvent  	*event,
                                     Boolean    *continue_to_dispatch
                                     ));
extern char
*bkr_window_get_icon_index_name PROTOTYPE((char    	    *root_index_name,
                                           unsigned int    *icon_size_rtn
                                           ));


void 	 
bkr_window_resize_work_area PROTOTYPE((
    Widget	    widget,
    XConfigureEvent *event,
    String  	    *params,	    /* not used */
    Cardinal	    *num_params));   /* not used */

void	 
bkr_window_set_iconify_pixmap PROTOTYPE((
    Widget  widget,
    Pixmap  pixmap));

void	 
bkr_window_setup PROTOTYPE((
    Widget	    widget,
    Widget  	    parent,
    Boolean 	    raise_window
));

BKR_WINDOW *
bkr_window_find PROTOTYPE((
    Widget	    widget_id
));

void
bkr_window_draw_cc_text PROTOTYPE((
    BKR_WINDOW *window,
    GC         gc,
    BMD_RECTANGLE *event_rect
));

void
bkr_window_setup_wm_protocols PROTOTYPE((
    BKR_WINDOW *window
));

#endif 



