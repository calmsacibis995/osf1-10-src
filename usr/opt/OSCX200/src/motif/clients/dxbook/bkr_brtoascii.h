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
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_BRTOASCII.H*/
/* *5     7-MAY-1992 14:23:31 KLUM "better error handling"*/
/* *4     3-MAR-1992 16:56:28 KARDON "UCXed"*/
/* *3     7-FEB-1992 16:15:09 KLUM "changed to bkr_book_ascii, bkr_topic_ascii"*/
/* *2     2-JAN-1992 13:34:58 KLUM "missed this one on integration"*/
/* *1    16-SEP-1991 12:44:48 PARMENTER "Function Prototypes for bkr_brtoascii.c"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_BRTOASCII.H*/
/*
***************************************************************
**  Copyright (c) Digital Equipment Corporation, 1988, 1991  **
**  All Rights Reserved.  Unpublished rights reserved	     **
**  under the copyright laws of the United States.  	     **
**  	    	    	    	    	    	    	     **
**  The software contained on this media is proprietary	     **
**  to and embodies the confidential technology of  	     **
**  Digital Equipment Corporation.  Possession, use, 	     **
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
**      Bookreader User Interface ( bkr )
**
**  ABSTRACT:
**
**	bkr_brtoascii header file
**
**
**--
**/
#include "br_prototype.h"

#ifndef BKR_BRTOASCII_H
#define BKR_BRTOASCII_H

/*
** FUNCTION PROTOTYPES
*/
extern int bkr_book_ascii
    PROTOTYPE((BKR_BOOK_CTX        *book,         /* Bookreader book context */
               char                *out_fname,
               FILE                *fp,           /* ascii output file ptr */
               Widget              parent_w));

extern int bkr_topic_ascii
    PROTOTYPE((BKR_BOOK_CTX  *book,         /* Bookreader book context */
               BMD_OBJECT_ID page_id,       /* page id for this topic */
               char          *out_fname,
               FILE          *fp,           /* ascii output file ptr */
               Widget        parent_w));


#endif /*  BKR_BRTOASCII_H */
