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
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_DISPLAY.H*/
/* *4    19-JUN-1992 20:12:44 BALLENGER "Cleanup for Alpha/OSF port"*/
/* *3     3-MAR-1992 16:58:15 KARDON "UCXed"*/
/* *2     1-NOV-1991 13:04:51 BALLENGER "reintegrate  memex support"*/
/* *1    16-SEP-1991 12:45:20 PARMENTER "Function Prototypes for bkr_display.c"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_DISPLAY.H*/
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
**	Display routines for text, images, and RAGS graphics.
**
**  AUTHORS:
**
**      David L Ballenger
**
**  CREATION DATE:     15-Jul-1991
**
**  MODIFICATION HISTORY:
**
**
**--
**/
#ifndef BKR_DISPLAY_H
#define BKR_DISPLAY_H
#include "br_prototype.h"

extern void	 bkr_display_data 
PROTOTYPE((Window       window_id,
           BMD_CHUNK    *chunk,
           int	       vwx,
           int	       vwy,
           XExposeEvent *expose,
           BKR_WINDOW   *window
           ));

extern BR_HANDLE bkr_display_ftext 
PROTOTYPE((Window      window_id,
           BR_UINT_8   *data_addr,
           unsigned    data_len,
           int 	       xoff,
           int 	       yoff,
           int 	       xclip,
           int 	       yclip,
           int 	       wclip,
           int 	       hclip,
           BKR_WINDOW  *window,
           BR_HANDLE   handle
           ));

#endif /* BKR_DISPLAY_H */
