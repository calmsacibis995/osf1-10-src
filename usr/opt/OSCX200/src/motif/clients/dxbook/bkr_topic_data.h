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
**	Routines for accessing topic data
**
**  AUTHORS:
**
**      David L Ballenger
**
**  CREATION DATE:     26-Feb-1992
**
**  MODIFICATION HISTORY:
**
**
**--
**/
#ifndef BKR_TOPIC_DATA_H
#define BKR_TOPIC_DATA_H

#include "br_prototype.h"

BKR_TOPIC_DATA *
bkr_topic_data_get PROTOTYPE((
    BKR_BOOK_CTX *book,
    BMD_OBJECT_ID page_id));

void
bkr_topic_data_free PROTOTYPE((
    BKR_BOOK_CTX *book,
    BMD_OBJECT_ID page_id));

void
bkr_topic_data_delete PROTOTYPE((
    BKR_BOOK_CTX *book, 
    BKR_TOPIC_DATA *topic));

BMD_CHUNK *
bkr_topic_chunk_get PROTOTYPE((
    BKR_BOOK_CTX *book,
    BMD_OBJECT_ID chunk_id));

void
bkr_topic_chunk_free PROTOTYPE((
    BKR_BOOK_CTX *book,
    BMD_OBJECT_ID chunk_id));

#endif 
