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
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_CLOSE.H*/
/* *2     3-MAR-1992 16:57:27 KARDON "UCXed"*/
/* *1    16-SEP-1991 12:45:04 PARMENTER "Function Prototypes for bkr_close.c"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_CLOSE.H*/
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
**	Function prototypes for bkr_close.c
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
#ifndef BKR_CLOSE_H
#define BKR_CLOSE_H

#include "br_prototype.h"

/* 
** Routines defined in bkr_close.c
*/
extern void	 
bkr_close_all_but_default_topic PROTOTYPE((
    BKR_SHELL *shell));

extern void	 
bkr_close_shell PROTOTYPE((
    BKR_SHELL *shell,
    Boolean unconditonal_free));

extern void	 
bkr_close_selection_window PROTOTYPE((
    Widget                     widget,
    BKR_WINDOW	    	       *sel_window,
    XmPushButtonCallbackStruct *callback_data));

extern void	 
bkr_close_topic_data PROTOTYPE((
    BKR_WINDOW *topic_window));

extern void	 
bkr_close_topic_window PROTOTYPE((
    Widget		      widget,
    caddr_t 	    	      data,
    XmPushButtonCallbackStruct *callback_data));

extern void	
bkr_close_quit_callback PROTOTYPE((
    void));

extern void          
bkr_close_window PROTOTYPE((
    BKR_WINDOW *window,
    Boolean    unconditional_destroy));

#endif 
