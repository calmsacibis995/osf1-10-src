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

/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_ERROR.H*/
/* *3    17-NOV-1992 22:49:53 BALLENGER "Fix error dialog so that previous window regains focus."*/
/* *2     3-MAR-1992 16:58:39 KARDON "UCXed"*/
/* *1    16-SEP-1991 12:45:30 PARMENTER "Function Prototypes for bkr_error.c"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_ERROR.H*/
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
**	Function prototypes for bkr_error.c
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
#ifndef BKR_ERROR_H
#define BKR_ERROR_H

#include "br_prototype.h"

/*
** Routines defined in bkr_error.c
*/
#ifdef BR_TYPEDEFS_H
void
bkr_error_simple_msg PROTOTYPE((
    BKR_WINDOW	*window,
    char 	*error));
#endif 

void	    	    	    
bkr_error_modal PROTOTYPE((
    char    	    *error_msg,
    Window  	    window_id));

void	    	    	    
bkr_error_set_parent_shell PROTOTYPE((
    Widget  shell_id));

void	    	    	    
bkr_error_unmap PROTOTYPE((
    Widget  	    	widget,
    caddr_t 	    	tag,
    XmAnyCallbackStruct	*reason));

int 	    	    	    
bkr_error_xlib_non_fatal PROTOTYPE((
    Display	    *display,
    XErrorEvent	    *event));

void	    	    	    
bkr_error_xt_warning_handler PROTOTYPE((
    char    *error_msg));

#endif 

