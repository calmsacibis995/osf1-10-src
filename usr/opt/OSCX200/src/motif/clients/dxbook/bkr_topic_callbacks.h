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
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_TOPIC_CALLBACKS.H*/
/* *3     5-AUG-1992 21:47:07 BALLENGER "Hotspot traversal and handling for character cell support."*/
/* *2     3-MAR-1992 17:05:08 KARDON "UCXed"*/
/* *1    16-SEP-1991 12:47:15 PARMENTER "Function Prototypes for bkr_topic_callbacks.c"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_TOPIC_CALLBACKS.H*/
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
**	Function prototypes for bkr_topic_callbacks.c
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
#ifndef BKR_TOPIC_CALLBACKS_H
#define BKR_TOPIC_CALLBACKS_H

#include "br_prototype.h"



/*
** Routines defined in bkr_topic_callbacks.c
*/

extern void	 bkr_topic_bottom PROTOTYPE ((Widget	      widget,
                                  BKR_WINDOW	      *window,
                                  XmAnyCallbackStruct *callback_data));
extern void	 bkr_topic_extensions_onoff PROTOTYPE ((Widget  	    	    	 widget,
                                            BKR_WINDOW	    	    	 *window,
                                            XmToggleButtonCallbackStruct *callback_data));
extern void	 bkr_topic_goback PROTOTYPE ((Widget	      widget,
                                  caddr_t 	      data,
                                  XmAnyCallbackStruct *callback_data));
extern void	 bkr_topic_hot_spots_onoff PROTOTYPE ((Widget		    	widget,
                                           BKR_WINDOW	    	    	*window,
                                           XmToggleButtonCallbackStruct *callback_data));
extern void	 bkr_topic_next PROTOTYPE ((Widget		    widget,
                                caddr_t 	    data,
                                XmAnyCallbackStruct *callback_data));
extern void	 bkr_topic_next_screen PROTOTYPE ((Widget		   widget,
                                       BKR_WINDOW	   *window,
                                       XmAnyCallbackStruct *callback_data));
extern void	 bkr_topic_previous PROTOTYPE ((Widget		widget,
                                    caddr_t 	    	data,
                                    XmAnyCallbackStruct *callback_data));
extern void	 bkr_topic_previous_screen PROTOTYPE ((Widget	       widget,
                                           BKR_WINDOW	       *window,
                                           XmAnyCallbackStruct *callback_data));
extern void	 bkr_topic_top PROTOTYPE ((Widget		   widget,
                               BKR_WINDOW	   *window,
                               XmAnyCallbackStruct *callback_data));
extern unsigned bkr_find_last_screen_top PROTOTYPE ((BKR_WINDOW *window, 
                                         int *last_screen_top_rtn));

void bkr_topic_keyboard_actions 
    PROTOTYPE((Widget	 widget,
               XKeyEvent *event,
               String  	 *params, 
               Cardinal	 *num_params
               ));
void
bkr_topic_focus_highlight 
    PROTOTYPE((BKR_WINDOW *window,
               Boolean highlight
               ));

void bkr_topic_focus
    PROTOTYPE((Widget	widget,
               XEvent   *event,
               String  	*params, 
               Cardinal *num_params
               ));
void bkr_topic_btn_box_focus
    PROTOTYPE((Widget	widget,
               XEvent   *event,
               String  	*params, 
               Cardinal *num_params
               ));
#endif 
