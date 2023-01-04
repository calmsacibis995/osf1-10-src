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
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_RAGS.H*/
/* *4    19-JUN-1992 20:12:58 BALLENGER "Cleanup for Alpha/OSF port"*/
/* *3    19-MAR-1992 13:23:31 GOSSELIN "added new RAGS support"*/
/* *2     3-MAR-1992 17:02:46 KARDON "UCXed"*/
/* *1    16-SEP-1991 12:46:28 PARMENTER "Function Prototypes for bkr_rags.c"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_RAGS.H*/
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
**	Function prototypes for bkr_rags.c
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
#ifndef BKR_RAGS_H
#define BKR_RAGS_H

#include "br_prototype.h"



/*
** Routines defined in bkr_rags.c
*/
extern BR_HANDLE bkr_rags_read PROTOTYPE((
    Window		window,		/*  window to display in	    */
    BR_UINT_8		*data_addr,	/*  address of data to display	    */
    BR_UINT_32		data_len,	/*  length of data to display	    */
    int 		xoff,		/*  x offset of origin in window    */
    int 		yoff,		/*  y offset of origin in window    */
    int 		xclip,		/*  x value of clip rectangle	    */
    int 		yclip,		/*  y value of clip rectangle	    */
    int 		wclip,		/*  width of clip rectangle	    */
    int 		hclip,		/*  height of clip rectangle	    */
					/*  NOTE: clip rect. is wrt window  */
					/*  origin, NOT wrt (xoff, yoff)    */
    BR_HANDLE	    	display_handle, /*  Rags segment handle	    	    */
    BR_UINT_32	    	data_type));    /*  data type                       */

extern BR_HANDLE bkr_rags_display PROTOTYPE((
    Window		window,		/*  window to display in	    */
    BR_UINT_8		*data_addr,	/*  address of data to display	    */
    BR_UINT_32		data_len,	/*  length of data to display	    */
    int 		xoff,		/*  x offset of origin in window    */
    int 		yoff,		/*  y offset of origin in window    */
    int 		xclip,		/*  x value of clip rectangle	    */
    int 		yclip,		/*  y value of clip rectangle	    */
    int 		wclip,		/*  width of clip rectangle	    */
    int 		hclip,		/*  height of clip rectangle	    */
					/*  NOTE: clip rect. is wrt window  */
					/*  origin, NOT wrt (xoff, yoff)    */
    BR_HANDLE	    	display_handle, /*  Rags segment handle	    	    */
    BR_UINT_32	    	data_type));    /*  data type                       */

extern void bkr_rags_close
    PROTOTYPE((BR_HANDLE));

BR_HANDLE
bkr_rags_file PROTOTYPE((BR_UINT_8 * data_addr ));

void
bkr_rags_init PROTOTYPE((void));

BR_HANDLE
bkr_rags_mem PROTOTYPE((
    BR_UINT_8		*data_addr,	/*  address of data buffer	    */
    BR_UINT_32		data_len,	/*  length of data buffer	    */
    BR_UINT_32 		type ));

void
bkr_rags_term PROTOTYPE((void));

#endif 




