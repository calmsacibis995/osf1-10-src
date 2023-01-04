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
**	bkr_pr_resolve_range header file
**
**
**  AUTHORS:
**
**      Frank Klum
**
**  CREATION DATE:     30-Nov-1991
**
**  MODIFICATION HISTORY:
**
**--
**/

#ifndef  BKR_PR_RESOLVE_RANGE_H
#define  BKR_PR_RESOLVE_RANGE_H
#ifdef PRINT

#include "br_prototype.h"

extern range_t *resolve_range
    PROTOTYPE((cdp_t               *cdp,
               range_t             **first_range,
               pglist_t            *first_page,
               topic_t             *first_topic,
               char                *start_string,
               char                *end_string,
               int                 type,
               int                 *status));

extern void condense_ranges
    PROTOTYPE((cdp_t               *cdp,
               range_t             **first_range));

extern void delete_range
    PROTOTYPE((cdp_t               *cdp,
               range_t             **first_range,
               XmString            xstring));

#endif /* PRINT */
#endif /* BKR_PR_RESOLVE_RANGE_H */
