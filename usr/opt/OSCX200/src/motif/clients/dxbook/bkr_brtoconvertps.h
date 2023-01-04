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
**	bkr_brtoconvertps header file
**
**
**--
**/
#include "br_prototype.h"

#ifndef BKR_BRTOCONVERTPS_H
#define BKR_BRTOCONVERTPS_H

/*
** FUNCTION PROTOTYPES
*/
extern int bkr_ps_begindoc
    PROTOTYPE((char  *ps_fname,
               FILE  *fp,
               Widget parent_widget));

extern int bkr_ps_enddoc
    PROTOTYPE((char  *ps_fname,
               FILE  *fp,
               Widget parent_widget));

extern int bkr_book_convertps
    PROTOTYPE((BKR_BOOK_CTX        *book,         /* Bookreader book context */
               char                *ps_fname,
               FILE                *fp,
               Widget              parent_widget));

extern int bkr_topic_convertps
    PROTOTYPE((BKR_BOOK_CTX  *book,         /* Bookreader book context */
               BMD_OBJECT_ID page_id,       /* page id for this topic */
               char          *ps_fname,
               FILE          *fp,           /* convert ps output file ptr */
               int           entire_book,   /* true if printing entire book */
               Widget        parent_widget,
               BKR_TOPIC_DATA  *topic));

extern int bkr_psdocsetup
    PROTOTYPE((BKR_BOOK_CTX  *book,
               char          *ps_fname,
               FILE          *fp,
               Widget        parent_widget));

extern int includeprolog
    PROTOTYPE((char          *in_area,
               char          *in_fnam,
               char          *ps_fname,
               FILE          *out_fp,
               Widget        parent_widget));

extern void setup_paper_scaling
    PROTOTYPE((int           page_width,
               int           page_height));

#endif /*  BKR_BRTOCONVERTPS_H */
