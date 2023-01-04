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
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_CLIENT_SERVER.H*/
/* *3     3-MAR-1992 16:57:19 KARDON "UCXed"*/
/* *2    14-OCT-1991 12:10:49 BALLENGER " Fix synchronization problems."*/
/* *1    16-SEP-1991 12:44:57 PARMENTER "Function Prototypes for bkr_client_server.c"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_CLIENT_SERVER.H*/
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
**	bkr_client_server.h
**
**  ABSTRACT:
**
**	Routines for dealing with the client/server access through the
**      Bookreader API.
**
**  AUTHORS:
**
**      David L Ballenger
**
**  CREATION DATE:    1-July-1991
**
**  MODIFICATION HISTORY:
**
**--
**/
#ifndef BKR_CLIENT_SERVER_H
#define BKR_CLIENT_SERVER_H
#include "br_prototype.h"

extern void bkr_server_initialize
    PROTOTYPE((void));

extern Boolean bkr_server_coldstart
    PROTOTYPE((void));

/* Don't put anything after this endif
 */
#endif /* BKR_CLIENT_SERVER_H */
