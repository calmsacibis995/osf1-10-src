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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_BUTTON.C*/
/* *34   26-MAR-1993 15:39:03 BALLENGER "Fix compilation problems for VAX ULTRIX."*/
/* *33   22-MAR-1993 14:37:40 BALLENGER "Fix problems with polygon hotspots."*/
/* *32   24-FEB-1993 17:46:58 BALLENGER "Fixes for large topic and Region memory leak."*/
/* *31   21-OCT-1992 11:32:19 KLUM "fix accvio on cut-to-buffer"*/
/* *30   21-SEP-1992 22:11:04 BALLENGER "Fix hotspot selection for character cell."*/
/* *29    5-AUG-1992 21:40:58 BALLENGER "Hotspot traversal and handling for character cell support."*/
/* *28   24-JUL-1992 12:23:37 KLUM "repress print if NO_PRINT alt-prod-name"*/
/* *27   20-JUL-1992 13:33:19 BALLENGER "Character cell support."*/
/* *26   19-JUN-1992 13:26:58 BALLENGER "Conditionalize use of Rags."*/
/* *25    9-JUN-1992 11:16:12 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *24   21-MAY-1992 09:05:44 FITZELL "ultrix compiler is far stricter than vms :-)"*/
/* *23   20-MAY-1992 17:13:24 FITZELL " highlighting search hit across lines"*/
/* *22   19-MAY-1992 14:02:59 FITZELL "search highlighting"*/
/* *21   11-MAY-1992 14:25:56 FITZELL "putting back in a fix for last char hilight"*/
/* *20    8-MAY-1992 16:19:40 FITZELL "add initialize copy ctx routine"*/
/* *19    8-MAY-1992 12:32:13 FITZELL "fixes missing last char when highlightinga singel line"*/
/* *18   30-APR-1992 22:20:25 GOSSELIN "updating with RAGS animation fixes"*/
/* *17    6-APR-1992 13:53:43 FITZELL "wrong file last time"*/
/* *16    6-APR-1992 13:51:21 FITZELL "ultrix compiler warning so much for cc/portable"*/
/* *15    6-APR-1992 12:34:25 FITZELL "highlighting fixes"*/
/* *14    3-APR-1992 17:14:09 FITZELL "decworld hooks"*/
/* *13   29-MAR-1992 16:30:42 FITZELL "take care of multiple topics and next/prev topic whena selctions is active"*/
/* *12   29-MAR-1992 14:08:08 FITZELL "BL998 checkin - highlighting getting better"*/
/* *11   25-MAR-1992 12:43:51 FITZELL "need ifdef cOpy around highlight_text_word"*/
/* *10   24-MAR-1992 15:28:36 FITZELL "adding bkr_highlight_text_word for search"*/
/* *9    20-MAR-1992 14:15:11 FITZELL "fix compiler warnings on Ultrix"*/
/* *8    19-MAR-1992 12:43:00 FITZELL "put wrong file in  last time"*/
/* *7    19-MAR-1992 11:54:30 FITZELL "add highlighting/selecting text for cut and pasting"*/
/* *6    27-FEB-1992 08:42:54 FITZELL "fixing mb1_motion so that it will build on ultrix"*/
/* *5    26-FEB-1992 16:30:15 GOSSELIN "added bkr_mb1_motion routine"*/
/* *4     9-JAN-1992 14:09:49 FITZELL "fix btn3 accvio when no hotspots are present"*/
/* *3     2-JAN-1992 09:55:48 FITZELL "Xbook_references for OAF level B"*/
/* *2    17-SEP-1991 19:26:28 BALLENGER "include function prototype headers"*/
/* *1    16-SEP-1991 12:38:47 PARMENTER "Button press and release events"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_BUTTON.C*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_BUTTON.C*/
/* *31   21-OCT-1992 11:32:19 KLUM "fix accvio on cut-to-buffer"*/
/* *30   21-SEP-1992 22:11:04 BALLENGER "Fix hotspot selection for character cell."*/
/* *29    5-AUG-1992 21:40:58 BALLENGER "Hotspot traversal and handling for character cell support."*/
/* *28   24-JUL-1992 12:23:37 KLUM "repress print if NO_PRINT alt-prod-name"*/
/* *27   20-JUL-1992 13:33:19 BALLENGER "Character cell support."*/
/* *26   19-JUN-1992 13:26:58 BALLENGER "Conditionalize use of Rags."*/
/* *25    9-JUN-1992 11:16:12 GOSSELIN "updating with VAX/ULTRIX PROTOTYPE fixes"*/
/* *24   21-MAY-1992 09:05:44 FITZELL "ultrix compiler is far stricter than vms :-)"*/
/* *23   20-MAY-1992 17:13:24 FITZELL " highlighting search hit across lines"*/
/* *22   19-MAY-1992 14:02:59 FITZELL "search highlighting"*/
/* *21   11-MAY-1992 14:25:56 FITZELL "putting back in a fix for last char hilight"*/
/* *20    8-MAY-1992 16:19:40 FITZELL "add initialize copy ctx routine"*/
/* *19    8-MAY-1992 12:32:13 FITZELL "fixes missing last char when highlightinga singel line"*/
/* *18   30-APR-1992 22:20:25 GOSSELIN "updating with RAGS animation fixes"*/
/* *17    6-APR-1992 13:53:43 FITZELL "wrong file last time"*/
/* *16    6-APR-1992 13:51:21 FITZELL "ultrix compiler warning so much for cc/portable"*/
/* *15    6-APR-1992 12:34:25 FITZELL "highlighting fixes"*/
/* *14    3-APR-1992 17:14:09 FITZELL "decworld hooks"*/
/* *13   29-MAR-1992 16:30:42 FITZELL "take care of multiple topics and next/prev topic whena selctions is active"*/
/* *12   29-MAR-1992 14:08:08 FITZELL "BL998 checkin - highlighting getting better"*/
/* *11   25-MAR-1992 12:43:51 FITZELL "need ifdef cOpy around highlight_text_word"*/
/* *10   24-MAR-1992 15:28:36 FITZELL "adding bkr_highlight_text_word for search"*/
/* *9    20-MAR-1992 14:15:11 FITZELL "fix compiler warnings on Ultrix"*/
/* *8    19-MAR-1992 12:43:00 FITZELL "put wrong file in  last time"*/
/* *7    19-MAR-1992 11:54:30 FITZELL "add highlighting/selecting text for cut and pasting"*/
/* *6    27-FEB-1992 08:42:54 FITZELL "fixing mb1_motion so that it will build on ultrix"*/
/* *5    26-FEB-1992 16:30:15 GOSSELIN "added bkr_mb1_motion routine"*/
/* *4     9-JAN-1992 14:09:49 FITZELL "fix btn3 accvio when no hotspots are present"*/
/* *3     2-JAN-1992 09:55:48 FITZELL "Xbook_references for OAF level B"*/
/* *2    17-SEP-1991 19:26:28 BALLENGER "include function prototype headers"*/
/* *1    16-SEP-1991 12:38:47 PARMENTER "Button press and release events"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_BUTTON.C*/
#ifndef VMS
 /*
#else
#module BKR_BUTTON "V03-0000"
#endif
#ifndef VMS
  */
#endif

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
**	Button press and release event handling callbacks for the drawing window.
**
**  AUTHORS:
**
**      James A. Ferguson
**
**  CREATION DATE:     13-Apr-1990
**
**  MODIFICATION HISTORY:
**
**--
**/


/*
 * INCLUDE FILES
 */
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include "br_common_defs.h"   /* common BR #defines (SELECT/UNSELECT) */
#include "br_meta_data.h"     /* typedefs and #defines for I/O of BR files */
#include "br_typedefs.h"      /* BR high-level typedefs and #defines */
#include "br_globals.h"       /* BR external variables declared here */
#include "bkr_button.h"       /* function prototypes for .c module */
#include "bkr_object.h"       /* Object id routines */
#include "bkr_topic_create.h" /* Topic creation routines */
#include "bkr_topic_open.h"   /* Topic open routines */
#include "bkr_window.h"       /* Window access routines */
#include "br_malloc.h"	      /* memory allocation defines */
#ifdef DECWORLD
#include "bkr_decworld.h"
#endif

#define TEXT_PTR( pkt )	    ( (BMD_TEXT_PKT *) &( (BMD_FTEXT_PKT *) (pkt) )->value[0] )

#define DOWN_CLICK  	1
#define UP_CLICK    	2

#define NONE     0
#define DOWN	 1
#define UP	 2
#define RIGHT    3
#define LEFT	 4
#define RIGHTOFF 5
#define LEFTOFF  6

#define BKR_COPY_BUFFER_SIZE 2048
#define SECONDARY_SEL_ATOM_NAME "STUFF_SELECTION"

static int no_print = FALSE;


/*
 * LOCAL ROUTINES
 */

static void process_topic_window_btn1
    PROTOTYPE((Widget		widget,
	       BKR_WINDOW    	*window,
               XButtonEvent    	*event,
               Boolean 	    	 up_click
               ));

/* extern routines */
XtConvertSelectionProc  bkr_convert_selection_cbk ();
XtLoseSelectionProc     bkr_lose_selection_cbk ();
XtSelectionDoneProc     bkr_selection_done_cbk ();



/*
 * FORWARD DEFINITIONS 
 */

static Time	    	    mb1_click2_delay = 250;	    /* in milliseconds */
static Widget	    	    *topic_popup_widgets = NULL;


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_button_btn1
**
** 	Button 1 action handler routine for the drawing window
**
**  FORMAL PARAMETERS:
**
**	widget      	- id of the widget that caused the event
**	xbutton_event	- X event associated with the Btn1Up event
**	params	    	- address of a list of strings passed as arguments
**	num_params  	- address of the number of arguments passed
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
void
bkr_button_btn1 PARAM_NAMES((widget,xbutton_event,params,num_params))
    Widget		     widget PARAM_SEP
    XButtonEvent    	    *xbutton_event PARAM_SEP
    String  	    	    *params PARAM_SEP
    Cardinal	    	    *num_params PARAM_END
{
    BKR_WINDOW 	            *window = NULL;
    int	    	    	    argument;

#ifdef DECWORLD
    bkr_decworld_reset_timer = TRUE;
#endif

    window = bkr_window_find( widget );
    if ( (window == NULL) || (*num_params != 1)) {
    	return;
    }
    if (num_params == NULL) {
        return;
    }
    if ((*num_params < 0) || (params[0] == NULL)) {
        return;
    }

    argument = atoi( params[0] );

    switch ( window->type )
    {
    	case BKR_SELECTION :
    	    return;

    	case BKR_STANDARD_TOPIC :
    	case BKR_FORMAL_TOPIC :
    	    if ( argument == UP_CLICK )
    	    	process_topic_window_btn1( widget,window, xbutton_event, TRUE );
    	    else if ( argument == DOWN_CLICK )
            {
                no_print = bri_book_no_print(window->shell->book->book_id);
                process_topic_window_btn1( widget,window, xbutton_event,FALSE );
#ifndef NO_RAGS
                bkr_rags_wevent( window, 0, xbutton_event);
#endif /* NO_RAGS */
            }
    	    break;
    }

};  /* end of bkr_button_btn1 */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_button_btn2
**
** 	Button 2 action handler routine for the drawing window
**
**  FORMAL PARAMETERS:
**
**	widget      	- id of the widget that caused the event
**	xbutton_event	- X event associated with the Btn1Up event
**	params	    	- address of a list of strings passed as arguments
**	num_params  	- address of the number of arguments passed
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
void
bkr_button_btn2 PARAM_NAMES((widget,xbutton_event,params,num_params))
    Widget		     widget PARAM_SEP
    XButtonEvent    	    *xbutton_event PARAM_SEP
    String  	    	    *params PARAM_SEP
    Cardinal	    	    *num_params PARAM_END

{
    BKR_WINDOW 	            *window = NULL;
    int	    	    	    argument;
    Atom		    secondary_sel_atom;
    XClientMessageEvent	    client_message_event;
    Window		    focus_window;
    int			    focus_state;


    if( bkrplus_g_allow_copy && !no_print ) {
#ifdef COPY
	window = bkr_window_find( widget );

	if ( (window == NULL) || (*num_params != 1)) 
	    return;
	    
	argument = atoi( params[0] );

	switch ( window->type )
	{
	    case BKR_SELECTION :
		return;

	    case BKR_STANDARD_TOPIC :
	    case BKR_FORMAL_TOPIC :

		if(( argument == DOWN_CLICK) && ( bkr_copy_ctx.startline))
		    bkr_un_highlight(bkr_copy_ctx.window);
		else if (( argument == UP_CLICK ) && ( bkr_copy_ctx.active )) { 
		    bkr_highlight_text_get(window);
		    /* Get ID of window with input focus */
		    XGetInputFocus (bkr_display, &focus_window, &focus_state); 
		    /* Get atom associated with STUFF_SELECTION event */
		    secondary_sel_atom = XInternAtom (bkr_display,
						      SECONDARY_SEL_ATOM_NAME,
						      FALSE);
		    /* Fill out client event structure */
		    client_message_event.type = ClientMessage;
		    client_message_event.serial = 0;
		    client_message_event.send_event = TRUE;
		    client_message_event.display = bkr_display;
		    client_message_event.window = focus_window;
		    client_message_event.message_type = secondary_sel_atom;
		    client_message_event.format = 32;
		    client_message_event.data.l[0]= XA_SECONDARY;
		    client_message_event.data.l[1]= xbutton_event->time;

		/* Send stuff_selection event to window with input focus */
		    XSendEvent (bkr_display, focus_window, TRUE, 
				NoEventMask, &client_message_event);
		    }
		break;
	    }
#endif
       }
};  /* end of bkr_button_btn2 */


#ifdef COPY
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_highlight_exposure
**
** 	turns off highlightin on an exposure event then rehighlights after
**	 everything is exposed
**
**  FORMAL PARAMETERS:
**
**	window	- bookreader window that the highlighting is in
**	
**	
**	
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
void bkr_highlight_exposure(window)
    BKR_WINDOW      *window;
{
  int           width;
  Widget        drawing_window;
  Window        drawing_window_id;
  int           i;
  BKR_TEXT_LINE *templine;
  BKR_TEXT_LINE *startline;
  BKR_TEXT_LINE *endline;
  int           start;
  int           x_offset;
  int           y_offset;


  if((!bkr_copy_ctx.startline) || (bkr_copy_ctx.window != window))
        return;


  if(bkr_copy_ctx.active) {
    x_offset = window->u.topic.x; /* offset is a neg value so add it to the x's*/
    y_offset = window->u.topic.y; /* offset is a neg value so add it to the y's*/

    startline = bkr_copy_ctx.startline;
    endline = bkr_copy_ctx.endline;

    /* get the window and it's id for doing the graphics operations */
    drawing_window = window->widgets[W_WINDOW];
    drawing_window_id = XtWindow( drawing_window );

    if(startline == endline) {

        if(bkr_copy_ctx.x_end < bkr_copy_ctx.x_start)
            start = bkr_copy_ctx.x_end;
        else
            start = bkr_copy_ctx.x_start;

	    XFillRectangle(bkr_display, drawing_window_id, bkr_xor_gc,
                           (start + x_offset),
                           (bkr_copy_ctx.startline->y + y_offset),
                           bkr_copy_ctx.start_width,
                           bkr_copy_ctx.startline->height);
        }
    else {

        if(startline->y > endline->y) {
            templine = startline;
            startline = endline;
            endline = templine;
            i = bkr_copy_ctx.x_start;
            bkr_copy_ctx.x_start = bkr_copy_ctx.x_end;
            bkr_copy_ctx.x_end = i;
            }

        /* starting at x_pos highlight to the end of line*/
        width = startline->width - bkr_copy_ctx.x_start;
    
	if(startline->exposed) {
	    XFillRectangle(bkr_display, drawing_window_id, bkr_xor_gc,
                           (bkr_copy_ctx.x_start + x_offset), 
                           (startline->y + y_offset), 
                           width,
                           startline->height);

	    startline->exposed = FALSE;
	    }
	
	templine = startline->next;

	while( templine != endline ) {

	    if(templine->exposed) {
		XFillRectangle(bkr_display, drawing_window_id, bkr_xor_gc, 
                               (templine->x + x_offset), 
                               (templine->y + y_offset), 
                               templine->width, 
                               templine->height);

		templine->exposed = FALSE;
		}

	    if(templine->next)
	        templine = templine->next;
	    else
	        return;
	    } /* end while(templine != endline) */

		/* last line now find out how far to the right to highlight*/

	width = bkr_copy_ctx.x_end;

        if( templine->exposed ) {
	    XFillRectangle(bkr_display, drawing_window_id, bkr_xor_gc, 
			   (templine->x + x_offset), 
                           (templine->y + y_offset), 
                           width, 
			   templine->height);

	    templine->exposed = FALSE;
            }
        }
      } /* end if( bkr_copy_ctx.active) */

} /* end bkr_highlight_exposure */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      bkr_copy_ctx_init
**	
**	initializes to zero/NULL the copy ctx struct
**
**  FORMAL PARAMETERS:
**
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
void bkr_copy_ctx_init()
{

    bkr_copy_ctx.window = NULL;
    bkr_copy_ctx.chunk = NULL;
    bkr_copy_ctx.startline = NULL;
    bkr_copy_ctx.endline = NULL;
    bkr_copy_ctx.bkr_copy_buffer_len = 0;
    bkr_copy_ctx.x_start = 0;
    bkr_copy_ctx.start_index = 0;
    bkr_copy_ctx.start_width = 0;
    bkr_copy_ctx.x_end = 0;
    bkr_copy_ctx.end_index = 0;
    bkr_copy_ctx.end_width =  0;
    bkr_copy_ctx.y_pos = 0;
    bkr_copy_ctx.y_pos_prev = 0;
    bkr_copy_ctx.active = FALSE;
    bkr_copy_ctx.direction = 0;
    bkr_copy_ctx.onoff = 0;
    bkr_copy_ctx.enabled = 0;
#ifdef TRACE
    printf("BKR_COPY_CTX CLEARED\n");
#endif
} /* bkr_copy_ctx_init */
 
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_un_highlight
**
** 	turns off highlighting and re-initializes everything
**
**  FORMAL PARAMETERS:
**
**	window	- bookreader window that the highlighting is in
**	
**	
**	
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
void bkr_un_highlight(window)
    BKR_WINDOW	    *window;
{
  int           width;
  Widget        drawing_window;
  Window	drawing_window_id;
  int           i;
  BKR_TEXT_LINE *templine;
  BKR_TEXT_LINE *startline;
  BKR_TEXT_LINE *endline;
  int		start;
  int		offset;


  if(!bkr_copy_ctx.startline)
	return;


  if(bkr_copy_ctx.active) {
    offset = window->u.topic.y; /* offset is a neg value so add it to the y's*/

    startline = bkr_copy_ctx.startline;
    endline = bkr_copy_ctx.endline;

    /* get the window and it's id for doing the graphics operations */
    drawing_window = window->widgets[W_WINDOW];
    drawing_window_id = XtWindow( drawing_window );

    if(startline == endline) {

	if(bkr_copy_ctx.x_end < bkr_copy_ctx.x_start)
	    start = bkr_copy_ctx.x_end;
        else
            start = bkr_copy_ctx.x_start;

	XFillRectangle(bkr_display, drawing_window_id, bkr_xor_gc, start,
			   (bkr_copy_ctx.startline->y + offset), 
			   bkr_copy_ctx.start_width, 
			   bkr_copy_ctx.startline->height);
#ifdef TRACE
    	      printf("ONE LINE CLEAR x = %d, y = %d, w = %d, h = %d \n", start, 
	           bkr_copy_ctx.startline->y, bkr_copy_ctx.start_width,
		   bkr_copy_ctx.startline->height);
#endif
    

	}
    else {
 
	if(startline->y > endline->y) {
	    templine = startline;
	    startline = endline;
	    endline = templine;
	    i = bkr_copy_ctx.x_start;
	    bkr_copy_ctx.x_start = bkr_copy_ctx.x_end;	
	    bkr_copy_ctx.x_end = i;
	    }

	/* starting at x_pos highlight to the end of line*/
	width = startline->width - bkr_copy_ctx.x_start;

	XFillRectangle(bkr_display, drawing_window_id, bkr_xor_gc, 
		  bkr_copy_ctx.x_start, (startline->y + offset), width, 
		  startline->height);

	templine = startline->next;
    
	while( templine != endline ) {
	    XFillRectangle(bkr_display, drawing_window_id, bkr_xor_gc, templine->x,
		       (templine->y + offset), templine->width, templine->height);

	    if(templine->next)
		templine = templine->next;
	    else
		return;
	    } /* end while(templine != endline) */

	    /* last line now find out how far to the right to highlight*/

	width = bkr_copy_ctx.x_end; 
	XFillRectangle(bkr_display, drawing_window_id, bkr_xor_gc, templine->x,
		       (templine->y + offset), width, templine->height);
	}
      } /* end if( bkr_copy_ctx.active) */

    /* set everything to null*/
    bkr_copy_ctx_init();
} /* bkr_un_highlight */



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      bkr_highlight_text_get
**	
**	gets the highlighted text and puts it in the global buffer 	
**
**  FORMAL PARAMETERS:
**
**	window	- bookreader window that the highlighting is in
**	
**	
**	
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
void bkr_highlight_text_get(window)
    BKR_WINDOW	    *window;
{
  int           width, new_size;
  int           i,num_chars;
  char		*char_ptr;
  char		*global_buffer_ptr;
  BKR_TEXT_LINE *templine;
  BKR_TEXT_LINE *startline;
  BKR_TEXT_LINE *endline;
  Boolean	one_line;
    
    startline = bkr_copy_ctx.startline;
    endline = bkr_copy_ctx.endline;

    if(bkr_copy_ctx.bkr_copy_buffer == NULL) {
	bkr_copy_ctx.bkr_copy_buffer = ( char *) BKR_MALLOC(BKR_COPY_BUFFER_SIZE);
	bkr_copy_ctx.bkr_copy_buffer_size = BKR_COPY_BUFFER_SIZE;
	}
    global_buffer_ptr = bkr_copy_ctx.bkr_copy_buffer;

    if(endline == startline) {
	one_line = TRUE;
	if(bkr_copy_ctx.start_index > bkr_copy_ctx.end_index){
	    i = bkr_copy_ctx.start_index;
            bkr_copy_ctx.start_index = bkr_copy_ctx.end_index;
            bkr_copy_ctx.end_index = i;
	    }
	}
    else {
	one_line = FALSE;
	/* check to see if we have to flip the starting and ending points*/
	if(startline->y > endline->y) {
	    templine = startline;
	    startline = endline;
	    endline = templine;
	    i = bkr_copy_ctx.start_index;
	    bkr_copy_ctx.start_index = bkr_copy_ctx.end_index;	
	    bkr_copy_ctx.end_index = i;
	    }
	}

    /* get the number of chars that are highlighted on this line */
    
    if(one_line)
	num_chars = (bkr_copy_ctx.end_index - bkr_copy_ctx.start_index) ;
    else
	num_chars =  (startline->n_bytes - bkr_copy_ctx.start_index) + 1 ;


/****HACK alert need to find the real reason that the above else clause sometimes
/***** gives a neg number 
*****/
    if( num_chars > 0) {
	bkr_copy_ctx.bkr_copy_buffer_len = num_chars;
    
	char_ptr = (char *) &startline->chars[bkr_copy_ctx.start_index - 1];

	memcpy(global_buffer_ptr, char_ptr, num_chars);

	global_buffer_ptr += num_chars;
	}

#ifdef TRACE_TEXT_GET
/*    bkr_copy_ctx.bkr_copy_buffer[bkr_copy_ctx.bkr_copy_buffer_len +1] = 0; */
    printf(" Copy buffer holds %s OK\n",bkr_copy_ctx.bkr_copy_buffer);
#endif

    if(one_line) {
	return;
	}
    else {
	    
	templine = startline->next;
    
	while( templine != endline ) {
	    num_chars = templine->n_bytes;
	    bkr_copy_ctx.bkr_copy_buffer_len += num_chars;
	    if(bkr_copy_ctx.bkr_copy_buffer_len > 
					    bkr_copy_ctx.bkr_copy_buffer_size)
	    {
		new_size = (int ) (bkr_copy_ctx.bkr_copy_buffer_size + BKR_COPY_BUFFER_SIZE);
		bkr_copy_ctx.bkr_copy_buffer = ( char *)
		      BKR_REALLOC(bkr_copy_ctx.bkr_copy_buffer, new_size);
		}
	
	    memcpy(global_buffer_ptr,templine->chars,num_chars);
	    global_buffer_ptr += num_chars;
#ifdef TRACE_TEXT_GET
/*    bkr_copy_ctx.bkr_copy_buffer[bkr_copy_ctx.bkr_copy_buffer_len +1] = 0; */
    printf(" Copy buffer holds %s OK\n",bkr_copy_ctx.bkr_copy_buffer);
#endif

	    if(templine->next)
		templine = templine->next;
	    else
		return;
	    } /* end while(templine != endline) */

	/* last line now find out how far to the right to highlight*/
	num_chars = bkr_copy_ctx.end_index - 1;

        if(num_chars > 0) {
	    bkr_copy_ctx.bkr_copy_buffer_len += num_chars;

	    char_ptr = (char *) templine->chars;

	    memcpy(global_buffer_ptr, char_ptr, num_chars);
            }

	}
#ifdef TRACE_TEXT_GET
  /*  bkr_copy_ctx.bkr_copy_buffer[bkr_copy_ctx.bkr_copy_buffer_len +1] = 0; */
 printf(" END GET TEXT Copy buffer holds %s OK\n",bkr_copy_ctx.bkr_copy_buffer);
#endif
} /* bkr_highlight_text_get */
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**   get_first_box 
**
**    Gets the bounding box start and end positions of the 
**    character the sprite is in 
**	
**
**  FORMAL PARAMETERS:
**
**	window	- bookreader window that the highlighting is in
**	
**	
**	int		real_x;		  x of the motion event *
**	int		*rtn_x;		 * rtn left side x of bounding box *
**	int		*rtn_x_next;	 * rtn right side x of bounding box * 
**	int             *rtn_x_prev;	 * rtn left side of prev character *
**	BKR_TEXT_LINE	*line;		 * text line we are dealing with *
**	int		*rtn_char_index; * index *
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/


void
get_first_box(real_x, rtn_x , rtn_x_next, rtn_x_prev, line, rtn_char_index)
int		real_x;		 /* x of the motion event */
int		*rtn_x;		 /* rtn left side x of bounding box */
int		*rtn_x_next;	 /* rtn right side x of bounding box */ 
int             *rtn_x_prev;	 /* rtn left side of prev character */
BKR_TEXT_LINE	*line;		 /* text line we are dealing with */
int		*rtn_char_index; /* index */
{
    int     ix, right_side, index;

    /* check the x position against lines starting x then start adding*/
    /* char widths until we hit it or pass it */

    for(ix = 0, right_side = line->x;
         (real_x > right_side) && (ix < line->n_bytes); ++ix)
        {
        right_side += line->char_widths[ix];
        }

    *rtn_char_index = ix;

    if(ix == 0)
      ix = 1;

    *rtn_x_next = right_side;
    *rtn_x = right_side - line->char_widths[ix - 1];
    index = (ix == 1)? 0: ix - 2;
    *rtn_x_prev = *rtn_x - line->char_widths[index];

}
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**   bkr_highlight_text
**
**    
**    text highlighting determined by position of sprite
**	
**
**  FORMAL PARAMETERS:
**
**    Widget	    widget,
**    BKR_WINDOW    *window,
**    Time	    event_time,
**    int	    event_x,
**    int	    event_y,
**    Atom	    selection_atom
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/


static void
bkr_highlight_text PARAM_NAMES((widget,window,event_time,event_x,event_y,selection_atom))
    Widget	widget PARAM_SEP
    BKR_WINDOW  *window PARAM_SEP
    Time	event_time PARAM_SEP
    int		event_x PARAM_SEP
    int		event_y PARAM_SEP
    Atom	selection_atom PARAM_END
{
  int		    real_x,real_y;
  int		    width,height,offset;
  Widget	    drawing_window;
  Window	    drawing_window_id;
  int		    i, numlines;
  BKR_TEXT_LINE	    *templine;
  BKR_TOPIC_DATA    *td;
  int		    v_direction, h_direction;
  int		    dropping, climbing;
  int		    highlight_on;
  int		    start_x;
  
  BKR_TEXT_LINE *startline = NULL;
  static int x_pos = 0;
  static int x_pos_prev = 0;
  static int x_pos_next = 0;
  static int x_width_next = 0;
  static int x_width_prev = 0;
  static int char_index = 0;

  /* check the sprites y value against the baseline values in the text lines 
     whatever two baselines it falls between the higher value ie lower on the 
     screen baseline will be the line to start on*/
			       
    offset = window->u.topic.y; /* offset is a neg value so add it to the y's*/

    if(!bkr_copy_ctx.startline) { 
#ifdef TRACE
 	printf("RESET EVRYTHING\n");
#endif
	td =  window->u.topic.data;

	startline = td->text_lines;
	
	if( !startline )
	    return;

	/* check to make sure we are not in a graphic */
	if( !startline->parent_chunk) {
	    if(td->chunk_list->data_type != BMD_CHUNK_FTEXT) {
		startline = NULL;
		return;
		}
	    }
	else {
	    if(startline->parent_chunk->data_type != BMD_CHUNK_FTEXT) {
		startline = NULL;
		return;
		}
	    }

	while (((startline->baseline + offset)  < event_y) && startline->next)
	    startline = startline->next;
	
	bkr_copy_ctx.y_pos_prev = startline->y ;
	bkr_copy_ctx.y_pos = startline->y + startline->height;
#ifdef TRACE
	printf("NEW Y's y_pos = %d y_prev = %d\n",
		bkr_copy_ctx.y_pos,bkr_copy_ctx.y_pos_prev);
#endif
	/* we've got the line now lets get the x position */
	get_first_box(event_x, &x_pos, &x_pos_next, &x_pos_prev, 
		      startline,&char_index);
	
	bkr_copy_ctx.startline = bkr_copy_ctx.endline = startline;
	bkr_copy_ctx.start_index = char_index;
	bkr_copy_ctx.x_start = x_pos;
        bkr_copy_ctx.start_width =  0;
	bkr_copy_ctx.direction = 0;
	bkr_copy_ctx.end_width = startline->y;
	x_width_prev = x_pos - x_pos_prev;
	x_width_next = x_pos_next - x_pos;
	} /* end if (!startline) */
    else {
	startline = bkr_copy_ctx.endline;
	highlight_on = FALSE;
	numlines = 0;
	/* okay we already have a starting line and an x_pos to check against */

	/* get the window and it's id for doing the graphics operations */
	drawing_window = window->widgets[W_WINDOW];
	drawing_window_id = XtWindow( drawing_window );

#ifdef TRACE
	printf("UP?DOWN E->y = %d y_pos = %d\n",event_y,(bkr_copy_ctx.y_pos+offset));
#endif
	/*are we going up or down ?? */
	if(event_y > (bkr_copy_ctx.y_pos + offset)) {
	    v_direction = DOWN;
	    dropping = TRUE;

	    if(startline->next)	  
	        templine = startline->next;
	    else {
	    /* we might have un-highlighted chars to the right */

		if(startline->width > x_pos_next) {

		    /* starting at x_pos highlight to the end of line*/
		    width = startline->width - x_pos;
		    
		    highlight_on = TRUE;
		    XFillRectangle(bkr_display, drawing_window_id, bkr_xor_gc,
			       x_pos, (startline->y + offset), 
			       width, startline->height);
#ifdef TRACE
		    printf("DOwn LL FINISH x = %d y = %d w = %d h = %d \n", x_pos,
			startline->y - offset,width,startline->height);
#endif

		    /* fix up the various variables */
		    get_first_box( (startline->width + 1), &x_pos, &x_pos_next, 
				    &x_pos_prev, startline, &char_index);
	
		    bkr_copy_ctx.end_index = char_index;
		    bkr_copy_ctx.x_end = x_pos;
		    bkr_copy_ctx.start_width =  0;
		    x_width_prev = x_pos - x_pos_prev;
		    x_width_next = x_pos_next - x_pos;
		    }
		return;
	        }

	    /* how many lines down */
	    while(dropping) {
		if(event_y < ((templine->y  + templine->height) + offset)) {
#ifdef TRACE
		    printf("DOWN  E->Y = %d < next y_pos %d numlines %d\n",
		            event_y,(templine->y  + templine->height),numlines);
#endif
		    dropping = FALSE;
		    }
		else {
		    if(templine->next) {
		         numlines++;
			 templine = templine->next;
			 }
		    else 
			dropping = FALSE;
		    }
		}
	    bkr_copy_ctx.y_pos_prev = bkr_copy_ctx.y_pos;
	    bkr_copy_ctx.y_pos = templine->y + templine->height;
	    bkr_copy_ctx.endline = templine;
#ifdef TRACE
	    printf("end DROPPING prev = %d y = %d\n",
	    (bkr_copy_ctx.y_pos_prev + offset),(bkr_copy_ctx.y_pos+offset));
#endif
	    }	    
	else if(event_y < (bkr_copy_ctx.y_pos_prev + offset)) {
#ifdef TRACE
            printf("goingUP prev = %d y = %d\n",
            (bkr_copy_ctx.y_pos_prev + offset),(bkr_copy_ctx.y_pos+offset));
#endif
	    v_direction = UP;
	    climbing = TRUE;

	    if(startline->prev)
		templine = startline->prev;
	    else 
		return;

            /* how many lines up */
            while(climbing) {
                if(event_y > (templine->y + offset)){
#ifdef TRACE
		    printf("UP  E->Y = %d is < templine->y %d \n",
		            event_y,(templine->y + offset));
#endif
                    climbing = FALSE;
		    }
                else {
		    if(templine->prev) {
			numlines++;
			templine = templine->prev;
			}
		    else 
			climbing = FALSE;
		    }
                }

	    bkr_copy_ctx.y_pos = bkr_copy_ctx.y_pos_prev;
	    bkr_copy_ctx.y_pos_prev = templine->y;
	    bkr_copy_ctx.endline = templine;
            }

	else
	    v_direction = NONE;


	if(event_x > x_pos_next)
	    h_direction = RIGHT;
	else if (event_x < x_pos) 
	    h_direction = LEFT;
	else 
	    h_direction = NONE;
#ifdef TRACE
	printf(" event x = %d x_pos = %d x_pos_next = %d x_pos_prev = %d\n",
		event_x, x_pos, x_pos_next, x_pos_prev);
#endif	
	/* we know how far either up or down and which way on the horizontal */
	/* plane. First highlight complete lines then the partial lines a    */
	/* character at a time . On the horizontal plane we always will     */
	/* have the right and left sides of the bounding box so once the     */
	/* bounding box sides have been passed highlight that character and  */
	/* get the next characters B-BOX */

	switch(v_direction) {
	    case NONE:

		switch(h_direction) {
		    case NONE:
			return;

		    case RIGHT:
			while( (event_x > x_pos_next) && 
					       (x_pos_next <= startline->width) ) {

			    highlight_on = TRUE;

			    /*switched dir need to wait until be go thru right*/
			    /* side of highlighted char before doing anything */
			    if(bkr_copy_ctx.direction != LEFT) {
				XFillRectangle(bkr_display, drawing_window_id,
					bkr_xor_gc, x_pos, (startline->y + offset),
				        x_width_next, startline->height);

				if(startline == bkr_copy_ctx.endline) {
				    if(x_pos >= bkr_copy_ctx.x_start)
					bkr_copy_ctx.start_width += x_width_next;
				    else 
					bkr_copy_ctx.start_width -= x_width_next;
				    }
				else {
				    if(bkr_copy_ctx.onoff != RIGHTOFF)	
					bkr_copy_ctx.start_width += x_width_next;
				    else
					bkr_copy_ctx.start_width -= x_width_next;
				    }
				}
			    if(bkr_copy_ctx.direction == UP)
				bkr_copy_ctx.onoff = RIGHTOFF;

			    bkr_copy_ctx.direction = RIGHT;

#ifdef TRACE
		  printf("RIGHT x_pos = %d y = %d x_width_next = %d h = %d \n", 
			x_pos, startline->y, x_width_next,startline->height);
#endif

			    x_pos_prev = x_pos;
			    x_pos = x_pos_next;
			    x_width_prev = x_width_next;
			    x_width_next = startline->char_widths[char_index++];
			    x_pos_next += x_width_next;
			    bkr_copy_ctx.end_index = char_index;
			    bkr_copy_ctx.x_end = x_pos;
			    }
			break;
		    case LEFT:		
			while( event_x < x_pos ) {	

			    highlight_on = TRUE;

			    if( (bkr_copy_ctx.direction == LEFT) || 
					    (bkr_copy_ctx.direction == NONE)) {
				XFillRectangle(bkr_display, drawing_window_id,
                                    bkr_xor_gc, x_pos, (startline->y + offset),
                                    x_width_next,startline->height);
				

				if(( x_pos > bkr_copy_ctx.x_start) || 
					     (bkr_copy_ctx.onoff == LEFTOFF )) {
				    bkr_copy_ctx.start_width -= x_width_prev;
				    bkr_copy_ctx.onoff = LEFTOFF;
				    }
				else {
                                    bkr_copy_ctx.onoff = LEFT;
				    bkr_copy_ctx.start_width += x_width_next;
				    }
				}
			    if(bkr_copy_ctx.direction == DOWN)
				bkr_copy_ctx.onoff = LEFTOFF;

			    bkr_copy_ctx.direction = LEFT;

#ifdef TRACE
	       printf("LEFT x_pos = %d y = %d x_width_next = %d h = %d \n",
			x_pos, startline->y, x_width_next,startline->height);
#endif

			    bkr_copy_ctx.x_end = x_pos;
			    x_pos_next = x_pos;
			    x_pos = x_pos_prev;
			    x_width_next = x_width_prev;
			    char_index--;
			    x_width_prev = startline->char_widths[char_index];
			    x_pos_prev -= x_width_prev;
			    bkr_copy_ctx.end_index = char_index;
			    }
			break;
		     }			    
	        break;

	    case DOWN:
		/* starting at x_pos highlight to the end of line*/
			    
		highlight_on = TRUE;
		
		if(bkr_copy_ctx.direction == LEFT) {

		    if(bkr_copy_ctx.end_width == startline->y)
			bkr_copy_ctx.x_start = bkr_copy_ctx.x_end;

		    if(bkr_copy_ctx.onoff != LEFTOFF) 
			start_x = x_pos_next + bkr_copy_ctx.start_width;
		    else
			start_x = x_pos_next;
		    }
		else
		    start_x = x_pos;

		width = startline->width - start_x;

		XFillRectangle(bkr_display, drawing_window_id, bkr_xor_gc,
			       start_x, (startline->y + offset), width, startline->height);
#ifdef TRACE
	printf("DOWN First line x = %d y = %d w = %d h = %d \n", x_pos,
			startline->y,width,startline->height);
#endif
		templine = startline->next;
		for (i = 0;i < numlines; i++) {
		    XFillRectangle(bkr_display, drawing_window_id, 
			           bkr_xor_gc, templine->x, (templine->y + offset),
				   templine->width,templine->height);

		    templine = templine->next;
		    }

		/* last line now find out how far to the right to highlight*/

	/* we've got the line now lets get the x position */
		get_first_box(event_x, &x_pos, &x_pos_next, &x_pos_prev, 
			      templine,&char_index);
	
		bkr_copy_ctx.endline = templine;
		bkr_copy_ctx.end_index = char_index;
		bkr_copy_ctx.x_end = x_pos;
		bkr_copy_ctx.start_width =  x_pos;
		x_width_prev = x_pos - x_pos_prev;
		x_width_next = x_pos_next - x_pos;
		
		width = x_pos;

	        XFillRectangle(bkr_display, drawing_window_id, bkr_xor_gc,
			     templine->x, (templine->y + offset), width, 
			     templine->height);

#ifdef TRACE
		 printf("DOWN last line x = %d y = %d w = %d h = %d \n", 
			templine->x, (templine->y + offset),width,templine->height);
#endif
		bkr_copy_ctx.direction = DOWN;
		break;

	    case UP:
	        /* starting at x_pos_next highlight to beginning */
		if(bkr_copy_ctx.direction == RIGHT) {
		    if(bkr_copy_ctx.onoff == RIGHTOFF)
			width = x_pos;
		    else
			width = bkr_copy_ctx.x_end - bkr_copy_ctx.start_width;
		    }
		else if((bkr_copy_ctx.direction == DOWN) ||
			( bkr_copy_ctx.direction == UP)  ||
			( bkr_copy_ctx.direction == NONE))
		    width = x_pos;
		else
		    width =  x_pos_next;

		bkr_copy_ctx.start_width = width;
		
/* - startline->x; */

		XFillRectangle(bkr_display, drawing_window_id, bkr_xor_gc,
			  startline->x, (startline->y + offset), width, 
			  startline->height);
#ifdef TRACE
		printf("UP 1st line x = %d y = %d w = %d h = %d \n", startline->x,
			startline->y, width,startline->height);
#endif
		/*go up a line  and starting from the end*/
		templine = startline->prev;
                for (i = 0;i < numlines; i++) {
                    XFillRectangle(bkr_display, drawing_window_id,
                                   bkr_xor_gc, templine->x, (templine->y + offset),
                                   templine->width,templine->height);

                    templine = templine->prev;
                    }

		get_first_box(event_x, &x_pos, &x_pos_next, &x_pos_prev, 
			      templine,&char_index);
	
		bkr_copy_ctx.endline = templine;
		bkr_copy_ctx.end_index = char_index;
		bkr_copy_ctx.x_end = x_pos; /* was x_pos_next */
		x_width_prev = x_pos - x_pos_prev;
		x_width_next = x_pos_next - x_pos;

		width = templine->width - x_pos;

/* was x_pos_next for for start x */
		XFillRectangle(bkr_display, drawing_window_id,bkr_xor_gc,
			       x_pos, (templine->y + offset), width, 
			       templine->height);
		
#ifdef TRACE
		printf("UP LAst line x = %d y = %d w = %d h = %d \n",
                      x_pos,(templine->y + offset),width,templine->height);
#endif
		bkr_copy_ctx.direction = UP;
		highlight_on = TRUE;

		break;

	} /* end of switch(v_direction) */

  }
  
  if( highlight_on ) {
      if(bkr_copy_ctx.active != TRUE) {
	  bkr_copy_ctx.active = TRUE;
	  bkr_copy_ctx.enabled = TRUE;
	  bkr_copy_ctx.window = window;
	  XtOwnSelection (widget, selection_atom, event_time, 
			    bkr_convert_selection_cbk,
			    bkr_lose_selection_cbk, 
			    bkr_selection_done_cbk);
	  }	
      }
  
  return;


};  /* end of bkr_highlight_text */

/*****************************************************************************
/* given the window, the line number ie starting from the top count the number
/* of lines and the offset into the char array to the first charaacter to 
/* highlight. Highligt until the first space char.
/*
/*
/****************************************************************************/

void 
bkr_highlight_text_word PARAM_NAMES((window,line_index,char_index,num_chars)) 
    BKR_WINDOW *window PARAM_SEP
    int    line_index PARAM_SEP
    int    char_index PARAM_SEP
    int    num_chars PARAM_END
{
  BKR_TOPIC_DATA   *topic_data;
  BKR_TEXT_LINE    *line;
  int		    i,c;
  int           width;
  Widget        drawing_window;
  Window	drawing_window_id;
  int		x_start,x_end,count;
  char		*word;
  char		*end_word;
  unsigned char	*spaces;
  Boolean	multiline = FALSE;
	
    /* make sure nothing else is highlighted 
    bkr_un_highlight(bkr_copy_ctx.window); 
    */

   /* get the window and it's id for doing the graphics operations */
    drawing_window = window->widgets[W_WINDOW];
    drawing_window_id = XtWindow( drawing_window );

   
    topic_data  =  window->u.topic.data;
    line = topic_data->text_lines;

    for(i=1; i < line_index;i++)
	line = line->next;


    if(!line) {
#ifdef TRACE
	printf("BOGUS LINE_INDEX\n");
#endif
	return;
	}
    else {

	line->exposed = TRUE;

	/* find starting point to highlight */
	for (i=0, x_start = 0; i<char_index - 1; i++) 
	    x_start += line->char_widths[i];
    

	bkr_copy_ctx.window = window;
	bkr_copy_ctx.startline = line;
        bkr_copy_ctx.x_start = x_start;
        bkr_copy_ctx.start_index = char_index;
	bkr_copy_ctx.end_index = 0;
            
	x_end = x_start;

	/* use count to flag when we go pasta line break*/
	count = char_index + num_chars;

	/* if we have more chars than are left on the line then we */
	/* have a multi line hit */
	if(line->n_bytes < count) {
	    count = line->n_bytes - char_index;
	    multiline = TRUE;
	    }
	else 
	    count = num_chars;

	for (c = 0; c < count; c++)
	    x_end += line->char_widths[i++];
	    
        bkr_copy_ctx.start_width = x_end - x_start;
	    	
	if(multiline) {
	    line = line->next;
	    line->exposed = TRUE;
	    count = num_chars - count;

	    /*check how many leading spaces to add we have stripped off*/
	    /* all but one when doing the search */
	    spaces = line->chars;
	    while(*spaces == ' ') {
		spaces++;
		count++; 
		}

	    x_end = line->x;
	    for (i = 0; i < count; i++)
		x_end += line->char_widths[i];
 
	    /* get width  to highlight */
	    bkr_copy_ctx.end_width = x_end - line->x;
	    bkr_copy_ctx.end_index = i;
	    }


	bkr_copy_ctx.endline = line;
	bkr_copy_ctx.x_end = x_end;
	bkr_copy_ctx.active = TRUE;
	}

}

#endif  /*COPY*/


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_mb1_motion
**
** 	Button 1 motion action handler routine for the drawing window
**
**  FORMAL PARAMETERS:
**
**	widget      	- id of the widget that caused the event
**	xbutton_event	- X event associated with the Btn1Motion
**	params	    	- address of a list of strings passed as arguments
**	num_params  	- address of the number of arguments passed
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
void
bkr_mb1_motion PARAM_NAMES((widget,event,params,num_params))
    Widget		     widget PARAM_SEP
    XMotionEvent    	    *event PARAM_SEP
    String  	    	    *params PARAM_SEP
    Cardinal	    	    *num_params PARAM_END

{
    BKR_WINDOW 	            *window;
    BMD_CHUNK		    *chunk = NULL;

    if( bkrplus_g_allow_copy && !no_print ) {
#ifdef COPY
	window = bkr_window_find( widget );
	if ( window == NULL )
	    return;

	switch ( window->type ) {

	    case BKR_SELECTION :
		return;

	    case BKR_STANDARD_TOPIC :
	    case BKR_FORMAL_TOPIC :

	    /* See if we are over a hot spot that is already highlighted */
		if (  window->u.topic.selected_chunk != NULL ) 
		    bkr_button_select_hot_spot( window, 
						window->u.topic.selected_chunk, 
						UNSELECT );
		bkr_highlight_text( widget, window, event->time, event->x,
				    event->y, XA_PRIMARY);
		bkr_copy_ctx.atom = XA_PRIMARY;

		break;
	    }
#endif
	} /* end if( bkrplus_g_allow_copy && !no_print ) */
};  /* end of bkr_mb1_motion */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_mb2_motion
**
** 	Button 1 motion action handler routine for the drawing window
**
**  FORMAL PARAMETERS:
**
**	widget      	- id of the widget that caused the event
**	xmotion_event	- X event associated with the Btn1Motion
**	params	    	- address of a list of strings passed as arguments
**	num_params  	- address of the number of arguments passed
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
void
bkr_mb2_motion PARAM_NAMES((widget,event,params,num_params))
    Widget		     widget PARAM_SEP
    XMotionEvent    	    *event PARAM_SEP
    String  	    	    *params PARAM_SEP
    Cardinal	    	    *num_params PARAM_END

{
    BKR_WINDOW 	            *window;
    BMD_CHUNK		    *chunk = NULL;
    Boolean		    chunk_target_exists = FALSE;
    
    if( bkrplus_g_allow_copy && !no_print ) {
#ifdef COPY
	window = bkr_window_find( widget );
	if ( window == NULL )
	    return;

	switch ( window->type )
	{
	    case BKR_SELECTION :
		return;

	    case BKR_STANDARD_TOPIC :
	    case BKR_FORMAL_TOPIC :

		if ( window->u.topic.selected_chunk != NULL ) 
		    bkr_button_select_hot_spot( window, 
					    window->u.topic.selected_chunk, 
					    UNSELECT );
		bkr_highlight_text( widget, window, event->time, event->x,
				    event->y, XA_SECONDARY);
		bkr_copy_ctx.atom = XA_SECONDARY ;
		break;
	    }
#endif
	} /* end if( bkrplus_g_allow_copy && !no_print ) */

};  /* end of bkr_mb2_motion */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_button_btn3_popup
**
** 	Creates and displays the appropriate popup menu for either
**  	    the SELECTION, or STANDARD or FORMAL Topic windows.
**
**  FORMAL PARAMETERS:
**
**	widget      - id of the widget that caused the event
**	event	    - X event associated with the Btn3Down event
**	params	    - address of a list of strings passed as arguments
**	num_params  - address of the number of arguments passed
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
void
bkr_button_btn3_popup PARAM_NAMES((widget,event,params,num_params))
    Widget		     widget PARAM_SEP
    XButtonEvent    	    *event PARAM_SEP
    String  	    	    *params PARAM_SEP
    Cardinal	    	    *num_params PARAM_END

{
    BKR_WINDOW	    	*window;
    Boolean 	    	click_on_hot_spot;
    BMD_CHUNK	    	*chunk = NULL;
    Arg	    	    	arglist[10];
    int	    	    	argcnt;
    Boolean 	    	sensitive;

    window = bkr_window_find( widget );

    if ( (window == NULL) || (window->type == BKR_SELECTION) )
    	return;


    if ( topic_popup_widgets == NULL )
	bkr_topic_create_popups( &topic_popup_widgets );
    if ( topic_popup_widgets == NULL )
	return;

    /* Manage/Unmanage the appropriate push buttons for this mapping */

    click_on_hot_spot = bkr_button_click_on_hot_spot( window, event, &chunk );


    if ( click_on_hot_spot )
    {
	/*  hot_spots with no target are xbook references and they */
	/*  don't need this particular popup			   */
	if(chunk->target == 0)
	    return;
	window->u.topic.btn3down_popup_chunk = chunk;
	if ( ! XtIsManaged( topic_popup_widgets[W_HOT_SPOT_POPUP_ENTRY] ) )
	    XtManageChild( topic_popup_widgets[W_HOT_SPOT_POPUP_ENTRY] );
	if ( ! XtIsManaged( topic_popup_widgets[W_HOT_SPOT_IN_NEW_POPUP_ENTRY] ) )
	    XtManageChild( topic_popup_widgets[W_HOT_SPOT_IN_NEW_POPUP_ENTRY] );
	if ( XtIsManaged( topic_popup_widgets[W_PREV_TOPIC_POPUP_ENTRY] ) )
	    XtUnmanageChild( topic_popup_widgets[W_PREV_TOPIC_POPUP_ENTRY] );
	if ( XtIsManaged( topic_popup_widgets[W_NEXT_TOPIC_POPUP_ENTRY] ) )
	    XtUnmanageChild( topic_popup_widgets[W_NEXT_TOPIC_POPUP_ENTRY] );
	if ( XtIsManaged( topic_popup_widgets[W_GOBACK_POPUP_ENTRY] ) )
	    XtUnmanageChild( topic_popup_widgets[W_GOBACK_POPUP_ENTRY] );
	if ( XtIsManaged( topic_popup_widgets[W_CLOSE_TOPIC_POPUP_ENTRY] ) )
	    XtUnmanageChild( topic_popup_widgets[W_CLOSE_TOPIC_POPUP_ENTRY] );
    }
    else
    {
	window->u.topic.btn3down_popup_chunk = NULL;
	if ( XtIsManaged( topic_popup_widgets[W_HOT_SPOT_POPUP_ENTRY] ) )
	    XtUnmanageChild( topic_popup_widgets[W_HOT_SPOT_POPUP_ENTRY] );
	if ( XtIsManaged( topic_popup_widgets[W_HOT_SPOT_IN_NEW_POPUP_ENTRY] ) )
	    XtUnmanageChild( topic_popup_widgets[W_HOT_SPOT_IN_NEW_POPUP_ENTRY] );
	if ( ! XtIsManaged( topic_popup_widgets[W_CLOSE_TOPIC_POPUP_ENTRY] ) )
	    XtManageChild( topic_popup_widgets[W_CLOSE_TOPIC_POPUP_ENTRY] );

	if ( window->type == BKR_FORMAL_TOPIC )
	{
	    if ( XtIsManaged( topic_popup_widgets[W_PREV_TOPIC_POPUP_ENTRY] ) )
		XtUnmanageChild( topic_popup_widgets[W_PREV_TOPIC_POPUP_ENTRY] );
	    if ( XtIsManaged( topic_popup_widgets[W_NEXT_TOPIC_POPUP_ENTRY] ) )
		XtUnmanageChild( topic_popup_widgets[W_NEXT_TOPIC_POPUP_ENTRY] );
	    if ( XtIsManaged( topic_popup_widgets[W_GOBACK_POPUP_ENTRY] ) )
		XtUnmanageChild( topic_popup_widgets[W_GOBACK_POPUP_ENTRY] );
	}
	else    /* STANDARD_TOPIC */
	{
	    if ( ! XtIsManaged( topic_popup_widgets[W_PREV_TOPIC_POPUP_ENTRY] ) )
		XtManageChild( topic_popup_widgets[W_PREV_TOPIC_POPUP_ENTRY] );
	    if ( ! XtIsManaged( topic_popup_widgets[W_NEXT_TOPIC_POPUP_ENTRY] ) )
		XtManageChild( topic_popup_widgets[W_NEXT_TOPIC_POPUP_ENTRY] );
	    if ( ! XtIsManaged( topic_popup_widgets[W_GOBACK_POPUP_ENTRY] ) )
		XtManageChild( topic_popup_widgets[W_GOBACK_POPUP_ENTRY] );
	}
    }

    /* Set the userData resource so the callback routines can access the window */

    sensitive = bri_page_previous(window->shell->book->book_id,window->u.topic.page_id ) != 0;
    argcnt = 0;
    SET_ARG( XmNuserData,   (caddr_t) window );
    SET_ARG( XmNsensitive,  sensitive );
    XtSetValues( topic_popup_widgets[W_PREV_TOPIC_POPUP_ENTRY], arglist, argcnt );
    sensitive = bri_page_next(window->shell->book->book_id,window->u.topic.page_id ) != 0;
    argcnt = 0;
    SET_ARG( XmNuserData,   (caddr_t) window );
    SET_ARG( XmNsensitive,  sensitive );
    XtSetValues( topic_popup_widgets[W_NEXT_TOPIC_POPUP_ENTRY], arglist, argcnt );
    argcnt = 0;
    SET_ARG( XmNuserData,   (caddr_t) window );
    SET_ARG( XmNsensitive,  ( window->u.topic.back_topic != NULL ) );
    XtSetValues( topic_popup_widgets[W_GOBACK_POPUP_ENTRY], 	arglist, argcnt );

    XtSetArg( arglist[0], XmNuserData, (caddr_t) window );
    XtSetValues( topic_popup_widgets[W_TOPIC_POPUP], 	     	     arglist, 1 );
    XtSetValues( topic_popup_widgets[W_HOT_SPOT_POPUP_ENTRY], 	     arglist, 1 );
    XtSetValues( topic_popup_widgets[W_HOT_SPOT_IN_NEW_POPUP_ENTRY], arglist, 1 );
    XtSetValues( topic_popup_widgets[W_CLOSE_TOPIC_POPUP_ENTRY],     arglist, 1 );

    /* Position the popup menu then manage it. */

    if ( ! XtIsRealized( topic_popup_widgets[W_TOPIC_POPUP] ) )
    	XtRealizeWidget( topic_popup_widgets[W_TOPIC_POPUP] );
    XmMenuPosition( topic_popup_widgets[W_TOPIC_POPUP], event );
    XtManageChild( topic_popup_widgets[W_TOPIC_POPUP] );

};  /* end of bkr_button_btn3_popup */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_button_click_on_hot_spot 
**
** 	Determines if the down click button event was over a hot spot in a 
**  	Topic window.
**
**  FORMAL PARAMETERS:
**
**	topic	    - pointer to Topic window to check for the down click event.
**  	event	    - X event associated with the Btn1Down.
**	chunk_rtn   - address of a pointer to a Chunk that describes the hot spot.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	Returns:    TRUE    - if the down click occurred over a hot spot.
**  	    	    FALSE   - all other cases.
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
Boolean
bkr_button_click_on_hot_spot PARAM_NAMES((window,event,chunk_rtn))
    BKR_WINDOW	 *window PARAM_SEP
    XButtonEvent *event PARAM_SEP
    BMD_CHUNK	 **chunk_rtn PARAM_END

{
    BMD_CHUNK	*chunk = window->u.topic.data->hot_spots;

    /* See if the click occurred over a hot spot.
     * Calculate the x and y for the point relative
     * to what part of thetopic is being displayed.
     */
    int x = event->x - window->u.topic.x;
    int y = event->y - window->u.topic.y;

    while (chunk)
    {
        if (chunk->region) 
        {
            if (XPointInRegion(chunk->region,x-chunk->rect.left,y-chunk->rect.top))
            {
                *chunk_rtn = chunk;
                return (TRUE);
            }
        }
        else
        {
            if ((x >= chunk->rect.left) && (x <= chunk->rect.right)
                && (y >= chunk->rect.top) && (y <= chunk->rect.bottom)
                ) 
            {
                *chunk_rtn = chunk;
                return TRUE; 
            }
        }
        chunk = chunk->next;
    }
    return ( FALSE );

};  /* end of bkr_button_click_on_hot_spot */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	bkr_button_select_hot_spot
**
** 	Reverse videos a hot spot in a Topic window.
**
**  FORMAL PARAMETERS:
**
**  	topic	- pointer to the Topic window.
**  	chunk	- pointer to the chunk.
**  	select	- Boolean: whether to SELECT or UNSELECT an entry.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
void
bkr_button_select_hot_spot PARAM_NAMES((window,chunk,selecths))
    BKR_WINDOW	        *window PARAM_SEP
    BMD_CHUNK   	*chunk PARAM_SEP
    Boolean 	    	selecths PARAM_END

{
    Widget  drawing_window = window->widgets[W_WINDOW];
    GC hotspot_text_gc;
    int x_offset = window->u.topic.x + chunk->rect.left;
    int y_offset = window->u.topic.y + chunk->rect.top;

    if ( selecths )
    {
        /*  If a different hot spot is selected, unselect it  */

    	if (window->u.topic.selected_chunk
            && ( window->u.topic.selected_chunk != chunk ) 
            ) 
        {
    	    bkr_button_select_hot_spot(window, 
                                       window->u.topic.selected_chunk, 
                                       UNSELECT );
        }
    	window->u.topic.selected_chunk = chunk;
        hotspot_text_gc 
        = bkrplus_g_charcell_display ? bkr_reverse_text_gc : bkr_xor_gc;
    }
    else
    {
    	window->u.topic.selected_chunk = NULL;
        hotspot_text_gc 
        = bkrplus_g_charcell_display ? bkr_text_gc : bkr_xor_gc;
    }

    if (chunk->region) 
    {
        XOffsetRegion(chunk->region,x_offset,y_offset);
        XSetRegion(bkr_display,hotspot_text_gc,chunk->region);
        XOffsetRegion(chunk->region,-x_offset,-y_offset);
    }

    if (bkrplus_g_charcell_display) 
    {
        bkr_window_draw_cc_text(window,hotspot_text_gc,&chunk->rect);

        XSetClipMask(bkr_display,hotspot_text_gc,None);

        if ((selecths == FALSE) && window->u.topic.show_hot_spots) 
        {
            bkr_pointer_outline_hot_spot( window, chunk, ON );
        }
    } 
    else 
    {
        XFillRectangle(bkr_display,
                       XtWindow(drawing_window),
                       bkr_xor_gc,
                       x_offset, 
                       y_offset, 
                       chunk->rect.width, chunk->rect.height);

        XSetClipMask(bkr_display,hotspot_text_gc,None);
    }

    /* Set the appropriate pulldown menu entries */

    if ( window->widgets[W_FILE_MENU] != NULL )
    {
    	if ( selecths )
    	{
    	    XtSetSensitive( window->widgets[W_OPEN_TOPIC_IN_DEFAULT], TRUE );
    	    XtSetSensitive( window->widgets[W_OPEN_TOPIC_IN_NEW], TRUE );
    	}
    	else
    	{
    	    XtSetSensitive( window->widgets[W_OPEN_TOPIC_IN_DEFAULT], FALSE );
    	    XtSetSensitive( window->widgets[W_OPEN_TOPIC_IN_NEW], FALSE );
    	}
    }

};  /* end of bkr_button_select_hot_spot */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	process_topic_window_btn1
**
** 	Determines which hot spot was click on for Btn1Down or Btn1Up events
**  	in a Topic window and preforms the appropriate action.
**
**  FORMAL PARAMETERS:
**
**	window	    - pointer to Topic window which received the event.
**  	event	    - X event associated with the Btn1Down.
**  	up_click    - Boolean: whether to process up or down click events.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  COMPLETION CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
static void
process_topic_window_btn1 PARAM_NAMES((widget,window,event,up_click))
    Widget		    widget PARAM_SEP
    BKR_WINDOW	    	    *window PARAM_SEP
    XButtonEvent    	    *event PARAM_SEP
    Boolean 	    	    up_click PARAM_END

{
    BMD_CHUNK	    	*chunk = NULL;
    Boolean 	    	chunk_target_exists = FALSE;
    BMD_OBJECT_ID       target_id;
#ifdef TRACE
    int line;
    int word;
#endif

    if ( window->u.topic.chunk_list == NULL )
    	return;

    /* See if the down click was over a hot spot */

    chunk_target_exists = bkr_button_click_on_hot_spot( window, event, &chunk );

    if ( up_click )
    {
    	if ( ! window->u.topic.mb1_is_down )   /* MB1 NOT down */
    	    return;

    	/* Completing a double click, so open the target of the hot spot    */ 
	/* or highligh the word the pointer is in when the double click was */
	/* initiated */

    	if ( window->u.topic.mb1_double_click )
    	{
    	    if ( chunk_target_exists ) {
                
                BKR_SHELL *parent = window->shell;

                /*  Hotspots in FORMAL topics are always opened in the "default"
                 *  Standard Topic window.
                 */
		if(chunk->target == 0)
		    target_id = chunk->data_type; /* it's a xbook ref */
		else
		    target_id = chunk->target;

                if ( window->type == BKR_FORMAL_TOPIC ) {
		    bkr_object_id_dispatch(parent,parent->default_topic,target_id);
                } else if ( window->type == BKR_STANDARD_TOPIC ) {
                    bkr_object_id_dispatch(parent,window,target_id);
                }
            }
/*	    	    
#ifdef COPY
	    
	    if( bkrplus_g_allow_copy && !no_print ) 
		bkr_highlight_double_click(widget, window, event->time, 
					   event->x, event->y, 
					   bkr_copy_ctx.atom);
#endif
*/
	    window->u.topic.mb1_double_click = FALSE;
	    window->u.topic.mb1_up_time = 0;
	    window->u.topic.mb1_is_down = FALSE;
	    return;
    	}
#ifdef COPY
	if( bkr_copy_ctx.active &&  bkrplus_g_allow_copy && !no_print ) {
#ifdef TRACE
	    printf("Upclick catch up x = %d y = %d\n",event->x,event->y);
#endif
	    
	    bkr_highlight_text (widget, window, event->time, event->x, 
				event->y, bkr_copy_ctx.atom); 
	    bkr_highlight_text_get(window);
	    }
	else {
#endif
	    if ( chunk_target_exists ) {
		if (! (( window->u.topic.selected_chunk != NULL ) 
			      && ( window->u.topic.selected_chunk == chunk )) )
			bkr_button_select_hot_spot( window, chunk, SELECT );
		}
	    else	/* Down click wasn't over a hotspot, so deselect */
		{
		    if ( window->u.topic.selected_chunk != NULL ) 
			bkr_button_select_hot_spot( window, 
				    window->u.topic.selected_chunk, UNSELECT );
		}
#ifdef COPY
	    }
#endif
    	/*
     	 *  This was a normal downclick, so if the upclick was over the same line
     	 *  as the downclick, save the upclick event time, else disable doubleclick.
     	 */

    	if ( ( window->u.topic.selected_chunk != NULL ) 
    	    && ( window->u.topic.selected_chunk == chunk ) )
	    window->u.topic.mb1_up_time = event->time;
    	else
	    window->u.topic.mb1_up_time = 0;

    	/* This up click is over */

    	window->u.topic.mb1_is_down = FALSE;

    }
    else    	/* Process down click events */
    {
#ifdef COPY
	if(bkr_copy_ctx.startline) {
#ifdef TRACE
	    printf("DOWNCLICK - ACTIVE SELECTION\n");
#endif
	    bkr_un_highlight(bkr_copy_ctx.window);
	    }
#endif
    	window->u.topic.mb1_is_down = TRUE;


    	/*
     	*  If this down click occurred within the double click delay time,
     	*  a hot spot is selected, and the hot spot is already selected from the 
     	*  last down click, start a double click instead of a selection.
     	*/

    	if ( ( event->time <= ( window->u.topic.mb1_up_time + mb1_click2_delay ) ) 
    	    	&& ( window->u.topic.selected_chunk != NULL ) 
    	    	&& ( window->u.topic.selected_chunk == chunk ) )
    	{
	    window->u.topic.mb1_double_click = TRUE;
	    return;
    	}

    	/*
     	*  Down click was over a hot spot. If the hot spot (chunk) is already 
        *  selected
     	*  just return otherwise select the hot spot.
     	*/


    }	/* end if up_click */

};  /* end of process_topic_window_btn1 */












