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

#ifndef  BKR_SIMPLE_SEARCH_H
#define  BKR_SIMPLE_SEARCH_H

#include "br_prototype.h"
#ifdef __osf__
#include "dxmcbrwidget.h"
#else
#include <DXmCbr/dxmcbrwidget.h>
#endif


extern void bkr_setup_simple_search
    PROTOTYPE(( BKR_WINDOW 		*window,
    		Opaque			*tag ));

extern void bkr_simple_search_ok
    PROTOTYPE(( Widget			widget,
    		Opaque			*tag,
    		XmAnyCallbackStruct	*data ));

extern void bkr_simple_search_abort
    PROTOTYPE(( Widget			widget,
    		Opaque			*tag,
    		XmAnyCallbackStruct	*data ));

extern void bkr_simple_search_cancel 
    PROTOTYPE(( Widget			widget,
    		Opaque			*tag,
		XmAnyCallbackStruct	*data ));

extern void bkr_simple_results_select
    PROTOTYPE(( BKR_WINDOW 	*window,
		int              	position));

extern void bkr_simple_results_cancel 
    PROTOTYPE(( Widget			widget,
    		Opaque			*tag,
		DXmCbrCallbackStruct   	*data ));


extern void bkr_search_message_ok
    PROTOTYPE(( Widget  	    	widget,
    		Opaque			*tag,
		XmAnyCallbackStruct	*data ));

#endif /* BKR_SIMPLE_SEARCH_H */
                                 



