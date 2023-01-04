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
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_BUTTON.H*/
/* *9    20-JUL-1992 13:33:30 BALLENGER "Character cell support."*/
/* *8     9-JUN-1992 11:01:29 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *7    20-MAY-1992 08:46:42 FITZELL "adding num_chars param to bkr_highlight_text_word"*/
/* *6    29-MAR-1992 13:49:00 FITZELL "add bkr_highlight_text_word function prototype"*/
/* *5    19-MAR-1992 11:55:43 FITZELL "new func prototype for highlighting text"*/
/* *4     3-MAR-1992 16:56:36 KARDON "UCXed"*/
/* *3    26-FEB-1992 17:00:50 GOSSELIN "changed XButtonEvent to XMotionEvent"*/
/* *2    19-FEB-1992 16:14:58 FITZELL "fix intermediate build problem"*/
/* *1    16-SEP-1991 12:44:53 PARMENTER "Function Prototypes for bkr_button.c"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_BUTTON.H*/
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
**	All global variable definitions for the Bookreader.
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
#ifndef _BKR_BUTTON_H
#define _BKR_BUTTON_H
/*
** Routines defined in bkr_button.c
*/
extern Boolean	bkr_button_click_on_hot_spot
	PROTOTYPE((BKR_WINDOW	  *window,
                   XButtonEvent *event,
                   BMD_CHUNK	  **chunk_rtn
                   ));
extern void     bkr_arrow_key
	PROTOTYPE((Widget	     widget,
                   XButtonEvent *xbutton_event,
                   String       *params,
                   Cardinal     *num_params
                   ));
extern void     bkr_button_btn1
	PROTOTYPE((Widget	     widget,
                   XButtonEvent *xbutton_event,
                   String       *params,
                   Cardinal     *num_params
                   ));
extern void	bkr_button_btn3_popup
	PROTOTYPE((Widget		    widget,
                   XButtonEvent    	    *event,
                   String  	    	    *params,
                   Cardinal	    	    *num_params
                   ));
extern Boolean	bkr_button_click_on_hot_spot
	PROTOTYPE((BKR_WINDOW	  *window,
                   XButtonEvent *event,
                   BMD_CHUNK	  **chunk_rtn
                   ));
extern void	bkr_button_select_hot_spot
	PROTOTYPE((BKR_WINDOW	        *window,
                   BMD_CHUNK   		*chunk,
                   Boolean 	    	select
                   ));
extern void     bkr_mb1_motion
	PROTOTYPE((Widget                  widget,
                   XMotionEvent            *event,
                   String                  *params,
                   Cardinal                *num_params
                   ));

extern void     bkr_mb2_motion
	PROTOTYPE((Widget                  widget,
                   XMotionEvent            *event,
                   String                  *params,
                   Cardinal                *num_params
                   ));

extern void     bkr_button_btn2
	PROTOTYPE((Widget	     widget,
                   XButtonEvent *xbutton_event,
                   String       *params,
                   Cardinal     *num_params
                   ));

extern void	bkr_highlight_text_get
	PROTOTYPE((BKR_WINDOW *window));

extern void	bkr_highlight_text
	PROTOTYPE(( Widget	widget,
                   BKR_WINDOW  *window,
                   Time	event_time,
                   int		x,
                   int		y,    
                   Atom         selection_atom));

extern void    bkr_un_highlight
	PROTOTYPE(( BKR_WINDOW *window ));

extern void    bkr_highlight_text_word
	PROTOTYPE(( BKR_WINDOW *window,
                   int    line_index,
                   int    char_index,
		   int	  num_chars));



#endif /* _BKR_BUTTON_H */
