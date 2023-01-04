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
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_ICON.C*/
/* *4    17-JUN-1992 20:37:02 BALLENGER "Include br_common_defs.h"*/
/* *3     9-JUN-1992 09:56:38 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *2     3-MAR-1992 16:59:43 KARDON "UCXed"*/
/* *1    16-SEP-1991 12:39:33 PARMENTER "Icons"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_ICON.C*/
#ifndef VMS
 /*
#else
# module BKR_ICON "V03-0000"
#endif
#ifndef VMS
  */
#endif

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
**	Routine and data for creating Extensions stipple pixmap.
**
**  AUTHORS:
**
**      James A. Ferguson
**
**  CREATION DATE:     25-Apr-1990
**
**  MODIFICATION HISTORY:
**
**--
**/


/*
 * INCLUDE FILES
 */

#include    <X11/Xlib.h>
#include "br_common_defs.h"
#include "br_meta_data.h"    /* typedefs and #defines for I/O of BR files */
#include "br_typedefs.h"     /* BR high-level typedefs and #defines */
#include "br_globals.h"      /* BR external variables declared here */
#include "bkr_icon.h"        /* function prototypes for .c module */

/*
 *  Extension stipple pixmap data
 */

#define STIPPLE_WIDTH 16
#define STIPPLE_HEIGHT 16
static char stipple_bits[] = {
   0x00, 0x00, 0x22, 0x22, 0x00, 0x00, 0x88, 0x88, 0x00, 0x00, 0x22, 0x22,
   0x00, 0x00, 0x88, 0x88, 0x00, 0x00, 0x22, 0x22, 0x00, 0x00, 0x88, 0x88,
   0x00, 0x00, 0x22, 0x22, 0x00, 0x00, 0x88, 0x88};



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_icon_create_stipple_pixmap
**
** 	Creates the stipple pixmap used in the Extensions GC.
**
**  FORMAL PARAMETERS:
**
**	none
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	Virtual memory is allocated.
**
**--
**/
Pixmap
bkr_icon_create_stipple_pixmap(VOID_PARAM)
{

    return ( XCreateBitmapFromData( 
    	    	bkr_display,
    	    	XDefaultRootWindow( bkr_display ), 
    	    	stipple_bits,
    	    	STIPPLE_WIDTH,
    	    	STIPPLE_HEIGHT ) );

};  /* end of bkr_icon_create_stipple_pixmap */



