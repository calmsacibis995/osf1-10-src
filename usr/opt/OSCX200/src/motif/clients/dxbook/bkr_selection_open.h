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
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_SELECTION_OPEN.H*/
/* *2     3-MAR-1992 17:04:07 KARDON "UCXed"*/
/* *1    16-SEP-1991 12:46:57 PARMENTER "Function Prototypes for bkr_selection_open.c"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_SELECTION_OPEN.H*/
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
#ifndef BKR_SELECTION_OPEN_H
#define BKR_SELECTION_OPEN_H

#include "br_prototype.h"



/*
** Routines defined in bkr_selection_open
*/

Boolean 
bkr_selection_open PROTOTYPE((
    BKR_SHELL *shell,
    BMD_OBJECT_ID dir_id_to_open));

BKR_SHELL *
bkr_selection_open_book PROTOTYPE((
    char         *filename,
    BMD_SHELF_ID shelf_id,
    unsigned     entry_id,
    Boolean      use_default_window,
    BKR_NODE     *library_node));

void	 
bkr_selection_open_default_dir PROTOTYPE((
    Widget		widget,
    BKR_WINDOW          *window,
    XmAnyCallbackStruct *callback_data));

#endif 

