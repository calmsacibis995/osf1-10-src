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
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_FILE_DIALOG.H*/
/* *2     3-MAR-1992 16:59:01 KARDON "UCXed"*/
/* *1    16-SEP-1991 12:45:39 PARMENTER "Function Prototypes for bkr_file_dialog.c"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_FILE_DIALOG.H*/
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
**	Function prototypes for bkr_file_dialog
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
#ifndef BKR_FILE_DIALOG_H
#define BKR_FILE_DIALOG_H

#include "br_prototype.h"

/*
** Routines defined in bkr_file_dialog.c
*/
extern void	    	    	    
bkr_file_dialog_cancel PROTOTYPE((
    Widget		    	     widget,
    int	    	    	    	     *tag,
    XmFileSelectionBoxCallbackStruct *fileselect));

extern void	    	    	    
bkr_file_dialog_confirm_ok PROTOTYPE((
    Widget		    widget,
    int	    	    	    *tag,   	    	/* unused */
    XmAnyCallbackStruct     *callback_data));

extern void	    	    	    
bkr_file_dialog_create PROTOTYPE((
    Widget		    widget,
    int	    	    	    *tag,
    XmAnyCallbackStruct     *callback_data));

extern void	    	    	    
bkr_file_dialog_ok PROTOTYPE((
    Widget		    	     widget,
    int	    	    	    	     *tag,
    XmFileSelectionBoxCallbackStruct *fileselect));

extern void	    	    	    
bkr_file_dialog_open_default PROTOTYPE((
    Widget		    widget,
    int	    	    	    *tag,
    XmAnyCallbackStruct     *callback_data)); 	/* unused */


#endif 



