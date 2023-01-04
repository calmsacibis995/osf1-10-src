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
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_PRINTEXTRACT.H*/
/* *8     4-JUN-1992 16:27:55 KLUM "fixes to printing from popup windows"*/
/* *7    20-MAY-1992 11:26:00 KLUM "UI rework + printing from topic windows"*/
/* *6     3-MAR-1992 17:01:46 KARDON "UCXed"*/
/* *5    21-JAN-1992 14:49:39 KLUM "bl1"*/
/* *4     8-JAN-1992 16:26:37 KLUM "new ui with toggles"*/
/* *3     2-JAN-1992 09:32:54 KLUM "PS print work"*/
/* *2    13-NOV-1991 14:12:28 KLUM "Green development work"*/
/* *1    16-SEP-1991 12:46:23 PARMENTER "Function Prototypes for bkr_printextract.c"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_PRINTEXTRACT.H*/
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
**	bkr_printextract header file
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
**           Cleanup for integration with main main Bookreader code.
**
**--
**/

#ifndef  BKR_PRINTEXTRACT_H
#define  BKR_PRINTEXTRACT_H
#ifdef PRINT


extern void bkr_print_book
    PROTOTYPE((Widget		   widget,
               int   		   *tag,
               XmAnyCallbackStruct *data));

extern void bkr_print_topic
    PROTOTYPE((Widget		   widget,
               int   		   *tag,
               XmAnyCallbackStruct *data));

extern void bkr_printtopic_window
    PROTOTYPE((Widget		   widget,
               int   		   *tag,
               XmAnyCallbackStruct *data));

extern void bkr_printpopup
    PROTOTYPE((Widget		   widget,
               int   		   *tag,
               XmAnyCallbackStruct *data));

extern void bkr_print_otherps
    PROTOTYPE((Widget		   widget,
               int   		   *tag,
               XmAnyCallbackStruct *data));

extern void bkr_pe_options_button
    PROTOTYPE((Widget              widget,
               int		   *tag,
               XmAnyCallbackStruct *reason));

extern void bkr_pe_ok_button
    PROTOTYPE((Widget              widget,
               int		   *tag,
               XmAnyCallbackStruct *reason));

extern void bkr_pe_cancel_button
    PROTOTYPE((Widget              widget,
               int		   *tag,
               XmAnyCallbackStruct *reason));

extern void bkr_pe_printdialog_ok
    PROTOTYPE((Widget              widget,
               int		   *tag,
               XmAnyCallbackStruct *reason));

extern void bkr_pe_infodialog_ok
    PROTOTYPE((Widget		  w,
               int		  *tag,
               XmAnyCallbackStruct *reason));

extern void bkr_pe_append_tb
    PROTOTYPE((Widget		  w,
               int		  *tag,
               XmToggleButtonCallbackStruct *toggle));

extern void bkr_pe_format_radiobox
    PROTOTYPE((Widget		  w,
               int		  *tag,
               XmAnyCallbackStruct *reason));

extern void bkr_pe_radio2
    PROTOTYPE((Widget		  w,
               int		  *tag,
               XmAnyCallbackStruct *reason));

extern void bkr_pe_filename_text
    PROTOTYPE((Widget		  w,
               int		  *tag,
               XmAnyCallbackStruct *reason));

extern void bkr_pe_printers_buttons
    PROTOTYPE((Widget		  w,
               int		  *tag,
               XmAnyCallbackStruct *reason));

#if FALSE
void bkr_book_extract_cb
    PROTOTYPE((Widget			widget,
               Opaque			*tag,
               XmAnyCallbackStruct      *reason));
#endif 

#else  /* PRINT */

#endif /* PRINT */
#endif /* BKR_PRINTEXTRACT_H */
