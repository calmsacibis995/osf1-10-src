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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_CURSOR.H*/
/* *3     8-NOV-1992 19:21:33 BALLENGER "Use work in progress box instead of wait cursor on character cell."*/
/* *2     3-MAR-1992 16:57:52 KARDON "UCXed"*/
/* *1    16-SEP-1991 12:45:11 PARMENTER "Function Prototypes for bkr_cursor.c"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_CURSOR.H*/
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
**	Function prototypes for bkr_cursor.c
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
#ifndef BKR_CURSOR_H
#define BKR_CURSOR_H

#include "br_prototype.h"

/*
** Routines defined in bkr_cursor.c
*/
extern void	bkr_cursor_create_cursors PROTOTYPE((void));
extern void	bkr_cursor_display_wait PROTOTYPE((Boolean onoff));
extern void	bkr_cursor_display_inactive
                  PROTOTYPE((Widget shell_id, Boolean onoff));
extern void	bkr_cursor_display_inactive_all
                  PROTOTYPE((Boolean onoff));
extern void	bkr_cc_working PROTOTYPE((Boolean onoff));

#endif 



