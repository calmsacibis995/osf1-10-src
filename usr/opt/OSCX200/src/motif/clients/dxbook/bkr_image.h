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
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_IMAGE.H*/
/* *3    19-JUN-1992 20:12:52 BALLENGER "Cleanup for Alpha/OSF port"*/
/* *2     3-MAR-1992 17:00:06 KARDON "UCXed"*/
/* *1    16-SEP-1991 12:45:56 PARMENTER "Function Prototypes for bkr_image.c"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_IMAGE.H*/
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
**	Function prototypes for bkr_image.h
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
#ifndef BKR_IMAGE_H
#define BKR_IMAGE_H

#include "br_prototype.h"



/*
** Routines defined in bkr_image.c
*/
void	    	    	    
bkr_image_close PROTOTYPE((
    BMD_CHUNK	*chunk));

void		    	    
bkr_image_display PROTOTYPE((
    Window		window,
    BR_UINT_8		*data_addr,
    BR_UINT_32		data_len,
    int 		xoff,
    int 		yoff,
    int 		xclip,
    int 		yclip,
    int 		wclip,
    int 		hclip,
    int			im_width,
    int			im_height,
    BR_HANDLE		handle));

BR_HANDLE    	    	    
bkr_image_init PROTOTYPE((
    BR_UINT_8		*data_addr,
    BR_UINT_32		data_len,
    short int		width,
    short int		height,
    short int		x_res,
    short int		y_res,
    BR_UINT_32	        data_type));

#ifdef BKR_SCALING
XImage *    	    	    
bkr_image_scale PROTOTYPE((
    XImage	*ximage,
    float	*xscale,
    float	*yscale));
#endif

#endif 

