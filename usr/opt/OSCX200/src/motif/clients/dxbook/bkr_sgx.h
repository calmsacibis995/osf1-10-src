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
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_SGX.H*/
/* *3    19-JUN-1992 20:13:04 BALLENGER "Cleanup for Alpha/OSF port"*/
/* *2     3-MAR-1992 17:04:26 KARDON "UCXed"*/
/* *1    16-SEP-1991 12:47:01 PARMENTER "Function Prototypes for bkr_sgx.c"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_SGX.H*/
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
#ifndef BKR_SGX_H
#define BKR_SGX_H

#include "br_prototype.h"



/*
** Routines defined in bkr_sgx.c
*/
void	    	    	    
bkr_sgx_init PROTOTYPE((void));

BR_HANDLE	    	    
bkr_sgx_display PROTOTYPE((
    Window		window,
    BMD_CHUNK		*chunk,
    int 		xoff,
    int 		yoff,
    int 		xclip,
    int 		yclip,
    int 		wclip,
    int 		hclip));

void		    	    
bkr_sgx_close PROTOTYPE((BR_HANDLE));

void		    	    
bkr_sgx_exit PROTOTYPE((void));

#endif 

