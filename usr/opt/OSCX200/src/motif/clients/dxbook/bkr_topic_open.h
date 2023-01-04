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
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_TOPIC_OPEN.H*/
/* *5     5-AUG-1992 21:49:15 BALLENGER "Hotspot traversal and handling for character cell support."*/
/* *4    24-APR-1992 16:43:30 BALLENGER "Support window positioning through the API."*/
/* *3    14-MAR-1992 14:20:21 BALLENGER "Have bkr_topic_data_get open the page"*/
/* *2     3-MAR-1992 17:05:55 KARDON "UCXed"*/
/* *1    16-SEP-1991 12:47:31 PARMENTER "Function Prototypes for bkr_topic_open.c"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_TOPIC_OPEN.H*/
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
#ifndef BKR_TOPIC_OPEN_H
#define BKR_TOPIC_OPEN_H

#include "br_prototype.h"



/*
** Routines defined in bkr_topic_open.c
*/
void       
bkr_topic_open_in_default PROTOTYPE((
    Widget		 widget,
    caddr_t 	    	 data,
    XmAnyCallbackStruct *callback_data)); 	/* unused */

void       
bkr_topic_open_in_new PROTOTYPE((
    Widget		widget,
    caddr_t 	        data,
    XmAnyCallbackStruct *callback_data)); 	/* unused */

BKR_WINDOW *
bkr_topic_open_to_chunk PROTOTYPE((
    BKR_BOOK_CTX  *book,
    BMD_OBJECT_ID page_id,
    BMD_OBJECT_ID chunk_id));
    
BKR_WINDOW *
bkr_topic_open_to_position PROTOTYPE((
    BKR_SHELL	  *shell,
    BKR_WINDOW    *window,
    BMD_OBJECT_ID pg_id,
    int	          x_pos,
    int	          y_pos,
    BMD_OBJECT_ID chunk_id,
    Boolean 	  save_for_history));

void bkr_display_chunk_at_top PROTOTYPE((
    BKR_WINDOW    *window,
    BMD_OBJECT_ID chunk_id));

#endif 

