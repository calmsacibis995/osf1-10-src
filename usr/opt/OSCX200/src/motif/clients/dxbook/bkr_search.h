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
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_SEARCH.H*/
/* *6    16-APR-1992 17:52:38 FITZELL "bkr_book_open_to_chunk now returns a window pointer"*/
/* *5     5-MAR-1992 14:25:46 PARMENTER "adding simple search"*/
/* *4     3-MAR-1992 17:03:31 KARDON "UCXed"*/
/* *3    23-JAN-1992 16:38:57 PARMENTER "fixing search cb's"*/
/* *2     7-JAN-1992 16:50:34 PARMENTER "adding CBR/Search"*/
/* *1    16-SEP-1991 12:46:41 PARMENTER "Function Prototypes for bkr_search.c"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_SEARCH.H*/
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
**	bkr_search header file
**
**
**  AUTHORS:
**
**      Frank Klum
**
**  CREATION DATE:     30-May-1991
**
**  MODIFICATION HISTORY:
**
**  V03 0001 David L Ballenger 15-Aug-1991
**
**           Cleanup for integration with main Bookreader code.
**
**--
**/

#ifndef  BKR_SEARCH_H
#define  BKR_SEARCH_H

#include "br_prototype.h"



extern void bkr_no_op_cb 
    PROTOTYPE(( Widget widget,
		Opaque *tag,
		XmAnyCallbackStruct *data ));

extern void bkr_search_everything_cb
    PROTOTYPE(( Widget widget,
		Opaque *tag,
		XmAnyCallbackStruct *data ));

extern void bkr_search_concept_list_cb
    PROTOTYPE(( Widget widget,
		Opaque *tag,
		XmAnyCallbackStruct *data ));

extern void bkr_search_edit_concept_cb
    PROTOTYPE(( Widget widget,
		Opaque *tag,
		XmAnyCallbackStruct *data ));

#ifdef SEARCH

extern void bkr_cbr_results_cb
    PROTOTYPE(( char *locator ));

extern void bkr_cbr_get_topic
    PROTOTYPE(( char *locator ));


extern void bkr_initialize_search_context
    PROTOTYPE(( BKR_WINDOW *window ));


extern void bkr_delete_search_context
    PROTOTYPE(( BKR_WINDOW *window));


extern void bkr_reset_search_context
    PROTOTYPE(( BKR_WINDOW *window));


extern BKR_WINDOW *bkr_book_open_to_chunk 
    PROTOTYPE(( char *filename,
		BMD_OBJECT_ID chunk_id,
		Boolean	view_in_default ));

#endif /* SEARCH */
#endif /* BKR_SEARCH_H */
