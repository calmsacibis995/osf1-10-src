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
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_SCROLL.H*/
/* *3     5-AUG-1992 21:53:42 BALLENGER "add bkr_scroll_max_y routine"*/
/* *2     3-MAR-1992 17:03:16 KARDON "UCXed"*/
/* *1    16-SEP-1991 12:46:37 PARMENTER "Function Prototypes for bkr_scroll.c"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_SCROLL.H*/
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
#ifndef BKR_SCROLL_H
#define BKR_SCROLL_H

#include "br_prototype.h"


/*
** Routines defined in bkr_scroll.c
*/
void  	    	    	    
bkr_scroll_adjust_display PROTOTYPE((
    BKR_WINDOW *window));

void	    	    	    
bkr_scroll_adjust_scrollbars PROTOTYPE((
    BKR_WINDOW	    *window,
    int	    	    new_x,
    int	    	    new_y,
    int	    	    new_max_x,
    int	    	    new_max_y));

void	    	    	    
bkr_scroll_horizontal_callback PROTOTYPE((
    Widget    	    	    	hscrollbar,
    BKR_WINDOW	    	    	*window,
    XmScrollBarCallbackStruct	*callback_data));

void	    	    	    
bkr_scroll_vertical_callback PROTOTYPE((
    Widget    	    	    	vscrollbar,
    BKR_WINDOW	    	    	*window,
    XmScrollBarCallbackStruct	*callback_data));

int
bkr_scroll_max_y PROTOTYPE((
    int topic_height, 
    int window_height));
#endif 

