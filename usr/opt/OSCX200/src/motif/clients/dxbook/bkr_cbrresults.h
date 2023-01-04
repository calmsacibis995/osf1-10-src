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
 * @(#)$RCSfile: bkr_cbrresults.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/08/24 15:36:52 $
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
**	bkr_cbrresults header file
**
**
**  AUTHORS:
**
**      Frank Klum
**
**  CREATION DATE:     23-Sep-1992
**
**  MODIFICATION HISTORY:
**
**
**--
**/

#ifndef  BKR_CBRRESULTS_H
#define  BKR_CBRRESULTS_H


extern void BkrCbrOK_button
    PROTOTYPE((Widget		   widget,
               Opaque   	   tag,
               XmAnyCallbackStruct *data));

extern void BkrCbrApply_button
    PROTOTYPE((Widget		   widget,
               Opaque   	   tag,
               XmAnyCallbackStruct *data));

extern void BkrCbrCancel_button
    PROTOTYPE((Widget		   widget,
               Opaque   	   tag,
               XmAnyCallbackStruct *data));

extern void BkrCbrSelect_list
    PROTOTYPE((Widget		   widget,
               Opaque   	   tag,
               XmListCallbackStruct *data));

extern void BkrCbr_double_click
    PROTOTYPE((Widget		   widget,
               Opaque   	   tag,
               XmAnyCallbackStruct *data));

extern void BKrCbrResultsDeleteAllItems
    PROTOTYPE((Widget              scrolled_list_w,
               int                 n_list_items));

extern void BkrWidthButtonsEqually
    PROTOTYPE((Widget		   *widgets,
               int      	   num_widgets));

extern void BkrSpaceButtonsEqually
    PROTOTYPE((Widget		   *widgets,
               int       	   num_widgets));

#endif /* BKR_CBRRESULTS_H */
