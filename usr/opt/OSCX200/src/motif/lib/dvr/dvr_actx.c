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
#define Module DVR_ACTX
#define Ident  "V02-031"

/*
**++
**   COPYRIGHT (c) 1988, 1992 BY
**   DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.
**   ALL RIGHTS RESERVED.
**
**   THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED
**   ONLY  IN  ACCORDANCE  OF  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE
**   INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER
**   COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
**   OTHER PERSON.  NO TITLE TO AND  OWNERSHIP OF THE  SOFTWARE IS  HEREBY
**   TRANSFERRED.
**
**   THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE
**   AND  SHOULD  NOT  BE  CONSTRUED  AS A COMMITMENT BY DIGITAL EQUIPMENT
**   CORPORATION.
**
**    DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS
**    SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.
**
** FACILITY:
**
**	Compound Document Architecture (CDA)
**	Compound Document Viewers
**	DIGITAL Document Interchange Format (DDIF)
**
** ABSTRACT:
**	This module contains the action routines for the
**	ddif viewer widget; these are the routines which
**	are called to move through ddif documents.  It contains
**	the the public routines DvrNextWindow,
**	DvrPreviousWindow, DvrTopDocument,
**	and DvrBottomDocument; as well as private
**	routines to create and manipulate scroll bars.
**
** ENVIORNMENT:
**	VMS DECwindows and ultrix uws
**
** AUTHORS:
**      Dennis McEvoy
**
**
** MODIFIED BY:
**
**	V02-001		PBD0001		Peter B. Derr	1-Mar-1989
**		Revise #include files
**			DAM0001		Dennis McEvoy	12-may-1989
**		check for bad parameters in DvrGotoPage()
**	V02-002		DAM0002		Dennis McEvoy	15-jun-1989
**		check for NULL page before trying to scroll
**	V02-003		DAM0003		Dennis McEvoy	19-oct-1989
**		add smarts for ps viewing
**	V02-004		DAM0004		Dennis McEvoy	22-feb-1990
**		update page number resource
**	V02-005		DAM0005		Dennis McEvoy	05-mar-1990
**		changes for OS/2 port
**	V02-006		DAM0006		Dennis McEvoy	12-jul-1990
**		merge in changes for new callback support
**	V02-007		SJM0000		Stephen Munyan	20-Jun-1990
**		changes for Motif
**	V02-008		BFB0000		Barbara Bazemore 9-Nov-1990
**		move image widget no longer needed, switched to renderings.
**
**	V02-009		SJM0000		Stephen Munyan	28-Nov-1990
**		Incorporate prototype changes from XUI version
**
**		V02-007		JJT0001		Jeff Tancill	27-Nov-1990
**		V02-008		DAM0008		Dennis McEvoy	28-nov-1990
**			fix protos
**
**	V02-010		DAM0010		Dennis McEvoy	03-dec-1990
**		remove forward def of dvr_scroll_action_proc (use proto)
**	V02-011		DAM0011		Dennis McEvoy	05-feb-1991
**		fix end-of-doc callbacks
**	V02-012		DAM0012		Dennis McEvoy	01-mar-1991
**		convert to new typedefs
**	V02-013		RTG0001		Dick Gumbel	05-Mar-1991
**		cleanup #include's
**	V02-014		DAM0001		Dennis McEvoy	28-Mar-1991
**		switch to Xm scrollwindow widget for ps viewing
**	V02-015		DAM0001		Dennis McEvoy	03-apr-1991
**		cleanup typedefs
**	V02-016		DAM0001		Dennis McEvoy	06-may-1991
**		move dvr_int include to cleanup ultrix build
**	V02-017		DAM0001		Dennis McEvoy	06-jun-1991
**		fix accvio in DvrGotoPage
**	V02-018		SJM0000		Steve Munyan	24-Jun-1991
**		DEC C PROTO cleanups
**	V02-019		SJM0000		Steve Munyan	 2-Aug-1991
**		Fixed problem in horizontal and vertical scroll bar
**		calculations to prevent Motif diagnostics.  The
**		diagnostics were being generated because the slider
**		percentage was getting slightly to large due to
**		floating point round off errors and integer truncation.
**	V02-020		DAM0001		Dennis McEvoy	05-aug-1991
**		rename, rename headers, remove dollar signs
**	V02-021		DAM0001		Dennis McEvoy	01-oct-1991
**		remove extraneous scroll bar routines
**	V02-022		DAM0001		Dennis McEvoy	18-oct-1991
**		cleanup params to match protos
**	V02-023		DAM0001		Dennis McEvoy	28-oct-1991
**		add #ifdef DVR_PSVIEW around ps viewer related calls
**		DVR_PSVIEW will be defined in dvrwint.h for vms and ultrix 
**		and osf/1 but not defined for sun 
**	V02-024		CJR0001		Chris Ralto	19-Feb-1992
**		Move V1 entry point routines from this module
**		into new module dvr_olde.
**	V02-025		ECR0001		Elizabeth Rust	30-Mar-1992
**		Merge in audio code.
**	V02-026		RMM0001		Robert Meagher	07-Apr-1992
**		Switch to watch cursor for PS paging operations.
**	V02-027		DAM0000		Dennis McEvoy	01-may-1992
**		type cleanups for alpha/osf1
**	V02-028		KLM0001		Kevin McBride	12-Jun-1992
**		Alpha OSF/1 port
**	V02-029		RDH003		Don Haney	15-Jul-1992
**		Specify same-directory includes with "", not <>
**	V02-030		RJD000		Ronan Duke      30-Aug-1992
**		Include Xm/MAnagerP.h if linking against Motif V1.2
**	V02-031		RJD001		Ronan Duke       8-Sep-1992
**		Cast XmCR_* enum values to int when comparing with 
**	        the variable reason (an int) to avoid compiler warnings
**--
*/

/*
**
**  INCLUDE FILES
**
**/
#include <cdatrans.h>

#ifdef __vms__

#include <dvrint.h>				/* DVR internal definitions */

#pragma nostandard				/* turn off /stand=port for
						   "unclean" X include files */
#endif

#include <Xm/Xm.h>				/* Motif definitions */

/* RD:
 * if linking against V1.2 of Motif then need to explicitly include 
 * Xm/ManagerP.h to get XmManagerPart definition
 */
#ifdef MOTIF_V12
#include <Xm/ManagerP.h>	
#endif


#include <Mrm/MrmAppl.h>			/* Motif Resource Manager defintions */
#include <Xm/ScrollBar.h>			/* Scroll Bar definitions */

#ifdef __vms__
#pragma standard				/* turn /stand=port back on */
#else
#include <dvrint.h>				/* DVR internal definitions */
#endif

#include "dvrwdef.h"			/* Public Defns w/dvr_include */
#include "dvrwint.h"			/* Viewer Constants and structures  */
#include "dvrwptp.h"			/* viewer windowing prototypes */

#ifdef DVR_PSVIEW

#include <psviewwg.h>				/* PS Viewer Widget definitions */


/*
**
**  EXTERNAL and GLOBAL SPECIFIERS
**
**/


/* protos */
PROTO( void dvr_reset_ps_scrollbars,
    	(DvrViewerWidget) );

#endif  /* DVR_PSVIEW */


/*
 * Local PROTO definitions
*/

PROTO(CDAstatus dvr_drag_horizontal, (DvrViewerWidget, int));
PROTO(CDAstatus dvr_drag_vertical, (DvrViewerWidget, int));
PROTO(CDAstatus dvr_goto_next_window, (DvrViewerWidget, Boolean));
PROTO(CDAstatus dvr_goto_previous_window, (DvrViewerWidget, Boolean));
PROTO(CDAstatus dvr_left_inc, (DvrViewerWidget));
PROTO(CDAstatus dvr_left_window, (DvrViewerWidget));
PROTO(CDAstatus dvr_next_inc, (DvrViewerWidget));
PROTO(CDAstatus dvr_previous_inc, (DvrViewerWidget));
PROTO(CDAstatus dvr_right_inc, (DvrViewerWidget));
PROTO(CDAstatus dvr_right_window, (DvrViewerWidget));
PROTO(CDAstatus dvr_to_bottom_horizontal, (DvrViewerWidget, int));
PROTO(CDAstatus dvr_to_bottom_vertical, (DvrViewerWidget, int));
PROTO(CDAstatus dvr_to_top_horizontal, (DvrViewerWidget, int));
PROTO(CDAstatus dvr_to_top_vertical, (DvrViewerWidget, int));



/* define large number for going to the end of unstructured PS docs */
#define PS_LAST_PAGE 10000

/* defines for PS page movement */
#define PS_PREVIOUS	0
#define PS_NEXT         1
#define PS_GOTO         2

/*  this is the routine which updates the
 *  display of a ddif viewer widget;
 */



static int function_code = CDA_CONTINUE;



/*
**++
**  ROUTINE NAME:
**	dvr_call_eod_callbacks(vw)
**
**  FUNCTIONAL DESCRIPTION:
**	call end-of-doc callbacks specified in viewer
**
**  FORMAL PARAMETERS:
**	DvrViewerWidget vw;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

void dvr_call_eod_callbacks (vw)
    DvrViewerWidget vw;

{
    XEvent ev;    /* dummy event */

    dvr_call_callbacks(vw, DvrCRendDocument, &ev);

}


/*
**++
**  ROUTINE NAME:
**	adjust_slider(sb, value, percent)
**
**  FUNCTIONAL DESCRIPTION:
**	change the slider for the given scroll bar
**
**  FORMAL PARAMETERS:
**    	Widget 	sb; 		     scroll bar to change
**    	int    	value;		     value of slider
**    	int	percent_current;     % of doc displayed in current win
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

adjust_slider(sb,
	      value,
	      percent_current)

    Widget 	sb; 		    /* scroll bar to change   		 */
    int    	value;		    /* value of slider        		 */
    int		percent_current;    /* % of doc displayed in current win */

{
    /*  take inc down to mean 1 percent, page down to mean
     *  a window full
     */
    XmScrollBarSetValues(sb, value,
		percent_current, 0, 0, FALSE);

}

/*  convinience routine to get the width and height of a page;
 *  if the page is NULL, return 1 for width and height so scroll bars
 *  will fill.
 */

CDAstatus get_page_data(page_addr, width, height)
    void	  *page_addr;
    unsigned long *width;
    unsigned long *height;
{
    struct pag 		*current_page = (struct pag *) page_addr;

    if (current_page == NULL)
      {
	*width  = 1;
	*height = 1;
      }

    else
      {
        struct pag_private *page_data = (struct pag_private *) current_page->pag_user_private;

	if (page_data)
	  {
            *width  = page_data->width;
	    *height = page_data->height;
	  }
	else
	  {
	    *width  = 1;
	    *height = 1;
	  }
      }
}

/*
**++
**  ROUTINE NAME:
**	dvr_adjust_horizontal_slider(vw)
**
**  FUNCTIONAL DESCRIPTION:
**	adjust the horizontal scroll bar's slider to reflect
**	the current position within the document.
**
**  FORMAL PARAMETERS:
**    	DvrViewerWidget vw;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

CDAstatus dvr_adjust_horizontal_slider(vw)
    DvrViewerWidget vw;

{
    struct pag   	*current_page 	= vw->dvr_viewer.Dvr.current_page;
    unsigned long 	pag_width;
    unsigned long 	pag_height;
    int			win_width	= XtWidth(vw->dvr_viewer.work_window);
    int			win_height	= XtHeight(vw->dvr_viewer.work_window);
    float		temp_float;
    int			temp_int;

    DvrStructPtr	dvr_ptr 	= &(vw->dvr_viewer.Dvr);

    get_page_data(current_page, &pag_width, &pag_height);

    /* if there is a horizontal scroll bar, adjust it */
    if (vw->dvr_viewer.h_scroll != NULL)
      {
	/* check for a bad page size */
	if (pag_width == 0)
	  {
	    /*  if the page width is 0, fill the scroll
	     *	bar and return bad parameter status
	     */
	    adjust_slider(vw->dvr_viewer.h_scroll, 0, DVR_MAX_SCROLL_VALUE);
	    return(DVR_BADPARAM);
	  }

	else
	  {
	    /* compute the location of the scroll bar slider */
    	    temp_float = (float) ((float) vw->dvr_viewer.Dvr.Xtop /
				  (float) pag_width);
    	    temp_int = (int) (temp_float * (float) DVR_MAX_SCROLL_VALUE);
    	    dvr_ptr->HorizValue = MIN( DVR_MAX_SCROLL_VALUE,
					MAX(0, temp_int) ) ;

	    /* compute the size of the scroll bar slider */
    	    temp_float = (float) ((float) win_width / (float) pag_width);
    	    temp_int = (int) (temp_float * (float) DVR_MAX_SCROLL_VALUE);

    	    dvr_ptr->HorizPercentCurrent = MIN( DVR_MAX_SCROLL_VALUE,
					        MAX(0, temp_int) ) ;

	    /*
	     * Check to see if the resulting calculation is greater than
	     * the size of the scroll bar.  If so adjust the horizontal
	     * percentage to ensure that we don't overflow the slider
	     * and generate a Motif diagnostic.
	    */

	    if ((dvr_ptr->HorizPercentCurrent + dvr_ptr->HorizValue) > DVR_MAX_SCROLL_VALUE)
	        /* need to subtract current value of scroll bar */
	        dvr_ptr->HorizPercentCurrent = DVR_MAX_SCROLL_VALUE -
					       dvr_ptr->HorizValue;

	    adjust_slider(vw->dvr_viewer.h_scroll,
		      vw->dvr_viewer.Dvr.HorizValue,
		      vw->dvr_viewer.Dvr.HorizPercentCurrent);
	  }
       }
    return(DVR_NORMAL);
}

/*
**++
**  ROUTINE NAME:
**	dvr_adjust_vertical_slider(vw)
**
**  FUNCTIONAL DESCRIPTION:
**	adjust the vertical scroll bar's slider to reflect the
**	current position within the document.
**
**  FORMAL PARAMETERS:
**    	DvrViewerWidget vw;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

CDAstatus dvr_adjust_vertical_slider(vw)
    DvrViewerWidget vw;

{
    struct pag   	*current_page 	= vw->dvr_viewer.Dvr.current_page;
    unsigned long 	pag_width;
    unsigned long 	pag_height;
    int			win_width	= XtWidth(vw->dvr_viewer.work_window);
    int			win_height	= XtHeight(vw->dvr_viewer.work_window);
    float		temp_float;
    int			temp_int;

    DvrStructPtr	dvr_ptr 	= &(vw->dvr_viewer.Dvr);

    get_page_data(current_page, &pag_width, &pag_height);

    /* if there is a vertical scroll bar, adjust it */
    if (vw->dvr_viewer.v_scroll != NULL)
      {
	/* check for a bad page size */
	if (pag_height == 0)
	  {
	    /*  if the page height is 0, fill the scroll
	     *	bar and return bad parameter status
	     */
	    adjust_slider(vw->dvr_viewer.v_scroll, 0, DVR_MAX_SCROLL_VALUE);
	    return(DVR_BADPARAM);
	  }

	else
	  {
	    /* compute the location of the scroll bar slider */
    	    temp_float = (float) ((float) vw->dvr_viewer.Dvr.Ytop /
				  (float) pag_height);
    	    temp_int = (int) (temp_float * (float) DVR_MAX_SCROLL_VALUE);
    	    dvr_ptr->VertValue =  MIN( DVR_MAX_SCROLL_VALUE,
					MAX(0, temp_int) ) ;

	    /* compute the size of the scroll bar slider */
    	    temp_float = (float) ((float) win_height / (float) pag_height);
    	    temp_int = (int) (temp_float * (float) DVR_MAX_SCROLL_VALUE);
    	    dvr_ptr->VertPercentCurrent =  MIN( DVR_MAX_SCROLL_VALUE,
	        				MAX(0, temp_int) ) ;

	    /*
	     * Check to see if the resulting calculation is greater than
	     * the size of the scroll bar.  If so adjust the horizontal
	     * percentage to ensure that we don't overflow the slider
	     * and generate a Motif diagnostic.
	    */

	    if ((dvr_ptr->VertPercentCurrent + dvr_ptr->VertValue) > DVR_MAX_SCROLL_VALUE)
	        /* need to subtract current value of scroll bar */
	        dvr_ptr->VertPercentCurrent = DVR_MAX_SCROLL_VALUE -
					      dvr_ptr->VertValue;

	    adjust_slider(vw->dvr_viewer.v_scroll,
		      vw->dvr_viewer.Dvr.VertValue,
		      vw->dvr_viewer.Dvr.VertPercentCurrent);

          }
       }
    return(DVR_NORMAL);
}

/*
**++
**  ROUTINE NAME:
**	dvr_adjust_sliders(vw)
**
**  FUNCTIONAL DESCRIPTION:
**	adjust the sliders on the scroll bars to
**	correctly show the position relative to the
**	entire document. Use the values within the widget.
**
**  FORMAL PARAMETERS:
**    	DvrViewerWidget vw;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

CDAstatus dvr_adjust_sliders(vw)
    DvrViewerWidget vw;

{
    CDAstatus stat;
    /* adjust both scroll bars */

    stat = dvr_adjust_horizontal_slider(vw);
    stat = dvr_adjust_vertical_slider(vw);

    return(stat);
}

/*
**++
**  ROUTINE NAME:
**	dvr_scroll_action_proc(w, t, r)
**
**  FUNCTIONAL DESCRIPTION:
**      This routine is called when there is an
**      event on one of the scroll bars (page up,
**	increment down, etc).
**
**
**  FORMAL PARAMETERS:
**	Widget w;
**	char   *t;
**	char   *r;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

void dvr_scroll_action_proc(w, t, r)
	Widget  w;
	char    *t;
	XmScrollBarCallbackStruct *r;
{
    DvrViewerWidget vw;
    int action_type;
    CDAstatus stat;

    int reason = r->reason;


    /*  the scroll bar has to be one of the viewer widget's
     *  children so remember the viewer widget ID so we can
     *  pass it to dvr_viewer_action.
     */

    vw = (DvrViewerWidget) w->core.parent;

    /*  if there is no file open, then we should not be updating
     *  the scroll bars, return
     */
    if (!vw->dvr_viewer.Dvr.FileOpen)
	return;

    else

    /*
     *	else, file is open, make sure we have a current page;
     *  if we don't, then this means that the file is still open
     *  probably with some diagnostics; since we have no page, we
     *  do not allow any scrolling, return!
     */

	if (vw->dvr_viewer.Dvr.current_page == 0)
	    return;

/*AUDIO STUFF*/
#ifdef CDA_AUDIO_SUPPORT

        /*Take the buttons off the screen to make the scroll action smoother*/

        if ((reason != (int)XmCR_INCREMENT) && (reason != (int)XmCR_DECREMENT))
            dvr_unmanage_audio_buttons(vw->dvr_viewer.Dvr.current_page);

#endif
/*END AUDIO STUFF*/

    /*  is this the vertical scroll bar or the
     *	horizontal scroll bar?
     */

    if (w == vw->dvr_viewer.v_scroll)
      {

	/*  this is the vertical scroll bar;
	 *  call the appropriate routine;
	 */

    	switch(reason)
    	  {
	    case XmCR_INCREMENT:
	      vw->dvr_viewer.Dvr.WidgetStatus = dvr_next_inc(vw);
	      break;

	    case XmCR_DECREMENT:
	      vw->dvr_viewer.Dvr.WidgetStatus = dvr_previous_inc(vw);
	      break;

	    case XmCR_PAGE_INCREMENT:
	      vw->dvr_viewer.Dvr.WidgetStatus =
#ifdef TEST_WINDOW
			dvr_goto_next_window(vw, TRUE);
#else
			dvr_goto_next_window(vw, FALSE);
#endif
	      break;

	    case XmCR_PAGE_DECREMENT:
	      vw->dvr_viewer.Dvr.WidgetStatus =
#ifdef TEST_WINDOW
			dvr_goto_previous_window(vw, TRUE);
#else
			dvr_goto_previous_window(vw, FALSE);
#endif
	      break;

	    case XmCR_VALUE_CHANGED:
	      vw->dvr_viewer.Dvr.WidgetStatus =
			dvr_drag_vertical(vw, r->value);
	      break;

	    case XmCR_TO_TOP:
	      vw->dvr_viewer.Dvr.WidgetStatus =
			dvr_to_top_vertical(vw, r->pixel);
	      break;

	    case XmCR_TO_BOTTOM:
	      vw->dvr_viewer.Dvr.WidgetStatus =
			dvr_to_bottom_vertical(vw, r->pixel);
	      break;

	    default: return;
          }
      }

    else if (w == vw->dvr_viewer.h_scroll)
      {

	/*  this is the horizontal scroll bar; set the
	 *  action type according to the reason.
	 */

    	switch(reason)
    	  {
	    case XmCR_INCREMENT:
	      vw->dvr_viewer.Dvr.WidgetStatus = dvr_right_inc(vw);
	      break;

	    case XmCR_DECREMENT:
	      vw->dvr_viewer.Dvr.WidgetStatus = dvr_left_inc(vw);
	      break;

	    case XmCR_PAGE_INCREMENT:
	      vw->dvr_viewer.Dvr.WidgetStatus =
			dvr_right_window(vw);
	      break;

	    case XmCR_PAGE_DECREMENT:
	      vw->dvr_viewer.Dvr.WidgetStatus =
			dvr_left_window(vw);
	      break;

	    case XmCR_VALUE_CHANGED:
	      vw->dvr_viewer.Dvr.WidgetStatus =
			dvr_drag_horizontal(vw, r->value);
	      break;

	    case XmCR_TO_TOP:
	      vw->dvr_viewer.Dvr.WidgetStatus =
			dvr_to_top_horizontal(vw, r->pixel);
	      break;

	    case XmCR_TO_BOTTOM:
	      vw->dvr_viewer.Dvr.WidgetStatus =
			dvr_to_bottom_horizontal(vw, r->pixel);
	      break;

	    default: return;
          }
      }

}


/*
**++
**  ROUTINE NAME:
**	dvr_ps_change_page(vw, flag, page_num)
**
**  FUNCTIONAL DESCRIPTION:
**	call ps routines to change current page;
**	page_num parameter is used only for goto page
**
**  FORMAL PARAMETERS:
**    	DvrViewerWidget vw;
**    	unsigned short	flag;
**	unsigned long	page_num;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

void dvr_ps_change_page(vw, flag, page_num)
    DvrViewerWidget 	vw;
    unsigned short	flag;
    CDAcount		page_num;

{

#ifdef DVR_PSVIEW
    PSViewDisableRedisplay(PSwin(vw));
    dvr_reset_ps_scrollbars(vw);

    switch (flag)
      {
	case PS_NEXT:
		PSViewNextPage(PSwin(vw));
		break;
	case PS_GOTO:
    		(void) PSViewShowPage(PSwin(vw), page_num);
		break;
	case PS_PREVIOUS:
		PSViewPreviousPage(PSwin(vw));
		break;
      }

    PSViewEnableRedisplay(PSwin(vw));
#endif

}



/*
**++
**  ROUTINE NAME:
**	dvr_next_inc(vw)
**
**  FUNCTIONAL DESCRIPTION:
**	display the next increment of the current page; This
**	routine is called when the user clicks on the increment
**	down arrow in the vertical scroll bar.
**
**  FORMAL PARAMETERS:
**    	DvrViewerWidget vw;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

CDAstatus dvr_next_inc (vw)
    DvrViewerWidget 	vw;

{
    CDAstatus	 	stat;
    struct pag   	*current_page 	= vw->dvr_viewer.Dvr.current_page;
    unsigned long 	pag_width;
    unsigned long 	pag_height;
    int			win_width	= XtWidth(vw->dvr_viewer.work_window);
    int			win_height	= XtHeight(vw->dvr_viewer.work_window);

    get_page_data(current_page, &pag_width, &pag_height);

    /* move to next increment of this window */
    if ( vw->dvr_viewer.Dvr.Ytop + win_height >= pag_height)
      {
    	/* then we cannot step any further, we're all through */
    	return(DVR_NORMAL);
      }
    else
      {
	int save_y_top = vw->dvr_viewer.Dvr.Ytop;
	int actual_increment_size;

        /*  else, there is another increment within this
	 *  page, update the variables, scroll the window up, and
	 *  display the new section of the window
	 */
	 vw->dvr_viewer.Dvr.Ytop = vw->dvr_viewer.Dvr.Ytop + DVR_INC_SIZE;

	 /* always have a full screen if we can */
	 if (vw->dvr_viewer.Dvr.Ytop + win_height > pag_height)
	     vw->dvr_viewer.Dvr.Ytop = pag_height - win_height;

	 actual_increment_size = vw->dvr_viewer.Dvr.Ytop - save_y_top;
    	 if (XtIsManaged(vw))
	   {
	     int ul_x, ul_y, lr_x, lr_y;

             /* scroll up with XCopyArea */
	     XCopyArea(	XtDisplay(vw),
		     	XtWindow(vw->dvr_viewer.work_window),
		     	XtWindow(vw->dvr_viewer.work_window),
			vw->dvr_viewer.Dvr.GcId,
			0,
			actual_increment_size,
			XtWidth(vw->dvr_viewer.work_window),
			XtHeight(vw->dvr_viewer.work_window)-actual_increment_size,
			0,
			0);
	     ul_x = 0;
	     ul_y = win_height- actual_increment_size;
	     lr_x = win_width;
	     lr_y = win_height;

	     /* clear the area that needs to be repainted */
	     XFillRectangle(XtDisplay(vw), XtWindow(vw->dvr_viewer.work_window),
			vw->dvr_viewer.Dvr.erase_gc_id, ul_x, ul_y,
			lr_x, lr_y);

	     /* display the new area */

	     stat = dvr_display_page(vw, ul_x, ul_y, lr_x, lr_y);

	     if (DVRFailure(stat))
		return(stat);

	    /*
	    **  Invoke the application specific callback routine for the scroll
	    **  bar event
	    */
	    if (vw->dvr_viewer.scroll_bar_callback != NULL)			/* was .ecallback in XUI */
		dvr_call_scroll_bar_callbacks
			(vw
			, vw->dvr_viewer.Dvr.Xtop
			, vw->dvr_viewer.Dvr.Ytop
			, pag_width
			, pag_height
			, win_width
			, win_height
			);

	     /* adjust the scroll bar sliders */
	     return(dvr_adjust_vertical_slider(vw));
	   }
      }

    return(DVR_NORMAL);
}

/*
**++
**  ROUTINE NAME:
**	dvr_previous_inc(vw)
**
**  FUNCTIONAL DESCRIPTION:
**	display the previous increment of the current page; This
**	routine is called when the user clicks on the increment
**	up arrow in the vertical scroll bar.
**
**  FORMAL PARAMETERS:
**    	DvrViewerWidget vw;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

CDAstatus dvr_previous_inc (vw)
    DvrViewerWidget 	vw;

{
    CDAstatus	 	stat;
    struct pag   	*current_page 	= vw->dvr_viewer.Dvr.current_page;
    unsigned long 	pag_width;
    unsigned long 	pag_height;
    int			win_width	= XtWidth(vw->dvr_viewer.work_window);
    int			win_height	= XtHeight(vw->dvr_viewer.work_window);

    get_page_data(current_page, &pag_width, &pag_height);

    /* move to next increment of this window */
    if ( vw->dvr_viewer.Dvr.Ytop == 0 )
      {
	/* then we cannot step any further, we're all through */
	return(DVR_NORMAL);

      }
    else
      {
	int save_y_top = vw->dvr_viewer.Dvr.Ytop;
	int actual_increment_size;

	/*  else, there is another increment within this
	 *  page, update the variables, scroll the window down, and
	 *  display the new section of the window
	 */
	vw->dvr_viewer.Dvr.Ytop = MAX (0,
		(vw->dvr_viewer.Dvr.Ytop - DVR_INC_SIZE) );

	actual_increment_size = save_y_top - vw->dvr_viewer.Dvr.Ytop;

    	if (XtIsManaged(vw))
	  {
	     int ul_x, ul_y, lr_x, lr_y;

             /* scroll down with XCopyArea */
	     XCopyArea(	XtDisplay(vw),
		     	XtWindow(vw->dvr_viewer.work_window),
		     	XtWindow(vw->dvr_viewer.work_window),
			vw->dvr_viewer.Dvr.GcId,
			0,
			0,
			XtWidth(vw->dvr_viewer.work_window),
			XtHeight(vw->dvr_viewer.work_window)-actual_increment_size,
			0,
			actual_increment_size);

	     ul_x = 0;
	     ul_y = 0;
	     lr_x = win_width;
	     lr_y = actual_increment_size;

	     /* clear the area that needs to be repainted */
	     XFillRectangle(XtDisplay(vw), XtWindow(vw->dvr_viewer.work_window),
			vw->dvr_viewer.Dvr.erase_gc_id, ul_x, ul_y,
			lr_x, lr_y);

	     /* display the new area */

	     stat = dvr_display_page(vw, ul_x, ul_y, lr_x, lr_y);

	     if (DVRFailure(stat))
		return(stat);

	    /*
	    **  Invoke the application specific callback routine for the scroll
	    **  bar event
	    */
	    if (vw->dvr_viewer.scroll_bar_callback != NULL)			/* was .ecallback in XUI */
		dvr_call_scroll_bar_callbacks
			(vw
			, vw->dvr_viewer.Dvr.Xtop
			, vw->dvr_viewer.Dvr.Ytop
			, pag_width
			, pag_height
			, win_width
			, win_height
			);

	    /* adjust the scroll bar sliders */
	    return(dvr_adjust_vertical_slider(vw));
	  }

      }

    return(DVR_NORMAL);
}

/*
**++
**  ROUTINE NAME:
**	dvr_right_inc(vw)
**
**  FUNCTIONAL DESCRIPTION:
**	display the next increment to the right of the current page; This
**	routine is called when the user clicks on the increment
**	over right arrow in the horizontal scroll bar.
**
**  FORMAL PARAMETERS:
**    	DvrViewerWidget vw;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

CDAstatus dvr_right_inc (vw)
    DvrViewerWidget 	vw;

{
    CDAstatus		stat;
    struct pag   	*current_page 	= vw->dvr_viewer.Dvr.current_page;
    unsigned long 	pag_width;
    unsigned long	pag_height;
    int			win_width	= XtWidth(vw->dvr_viewer.work_window);
    int			win_height	= XtHeight(vw->dvr_viewer.work_window);

    get_page_data(current_page, &pag_width, &pag_height);

    /* move to next increment of this window */
    if ( (vw->dvr_viewer.Dvr.Xtop + win_width ) >= pag_width)
      {
    	/* then we cannot step any further, we're all through */
    	return(DVR_NORMAL);
      }
    else
      {
	int save_x_top = vw->dvr_viewer.Dvr.Xtop;
	int actual_increment_size;

        /*  else, there is another increment within this
	 *  page, update the variables, scroll the window up, and
	 *  display the new section of the window
	 */
	 vw->dvr_viewer.Dvr.Xtop = vw->dvr_viewer.Dvr.Xtop + DVR_INC_SIZE;

	 /* always have a full screen if we can */
	 if (vw->dvr_viewer.Dvr.Xtop + win_width > pag_width)
	     vw->dvr_viewer.Dvr.Xtop = pag_width - win_width;

	 actual_increment_size = vw->dvr_viewer.Dvr.Xtop - save_x_top;
    	 if (XtIsManaged(vw))
	   {
	     int ul_x, ul_y, lr_x, lr_y;

             /* scroll over with XCopyArea */
	     XCopyArea(	XtDisplay(vw),
		     	XtWindow(vw->dvr_viewer.work_window),
		     	XtWindow(vw->dvr_viewer.work_window),
			vw->dvr_viewer.Dvr.GcId,
			actual_increment_size,
			0,
			XtWidth(vw->dvr_viewer.work_window)-actual_increment_size,
			XtHeight(vw->dvr_viewer.work_window),
			0,
			0);

	     ul_x = win_width - actual_increment_size;
	     ul_y = 0;
	     lr_x = win_width;
	     lr_y = win_height;

	     /* clear the area that needs to be repainted */
	     XFillRectangle(XtDisplay(vw), XtWindow(vw->dvr_viewer.work_window),
			vw->dvr_viewer.Dvr.erase_gc_id, ul_x, ul_y,
			lr_x, lr_y);

	     /* display the new area */

	     stat = dvr_display_page(vw, ul_x, ul_y, lr_x, lr_y);

	     if(DVRFailure(stat))
		return(stat);

	    /*
	    **  Invoke the application specific callback routine for the scroll
	    **  bar event
	    */
	    if (vw->dvr_viewer.scroll_bar_callback != NULL)			/* was .ecallback in XUI */
		dvr_call_scroll_bar_callbacks
			(vw
			, vw->dvr_viewer.Dvr.Xtop
			, vw->dvr_viewer.Dvr.Ytop
			, pag_width
			, pag_height
			, win_width
			, win_height
			);

	     /* adjust the scroll bar sliders */
	     return(dvr_adjust_horizontal_slider(vw));
	   }

      }

    return(DVR_NORMAL);
}

/*
**++
**  ROUTINE NAME:
**	dvr_left_inc(vw)
**
**  FUNCTIONAL DESCRIPTION:
**	display the previous increment to the left of the current page; This
**	routine is called when the user clicks on the increment
**	over arrow in the horizontal scroll bar.
**
**  FORMAL PARAMETERS:
**    	DvrViewerWidget vw;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

CDAstatus dvr_left_inc (vw)
    DvrViewerWidget 	vw;

{
    CDAstatus	 	stat;
    struct pag   	*current_page 	= vw->dvr_viewer.Dvr.current_page;
    unsigned long 	pag_width;
    unsigned long	pag_height;
    int			win_width	= XtWidth(vw->dvr_viewer.work_window);
    int			win_height	= XtHeight(vw->dvr_viewer.work_window);

    get_page_data(current_page, &pag_width, &pag_height);

    /* move to next increment to the left of this window */
    if ( vw->dvr_viewer.Dvr.Xtop == 0 )
      {
	/* then we cannot step any further, we're all through */
	return(DVR_NORMAL);

      }
    else
      {
	int save_x_top = vw->dvr_viewer.Dvr.Xtop;
	int actual_increment_size;

	/*  else, there is another increment within this
	 *  page, update the variables, scroll the window down, and
	 *  display the new section of the window
	 */
	vw->dvr_viewer.Dvr.Xtop = MAX (0,
		(vw->dvr_viewer.Dvr.Xtop - DVR_INC_SIZE) );

	actual_increment_size = save_x_top - vw->dvr_viewer.Dvr.Xtop;

    	if (XtIsManaged(vw))
	  {
	     int ul_x, ul_y, lr_x, lr_y;

             /* scroll over with XCopyArea */
	     XCopyArea(	XtDisplay(vw),
		     	XtWindow(vw->dvr_viewer.work_window),
		     	XtWindow(vw->dvr_viewer.work_window),
			vw->dvr_viewer.Dvr.GcId,
			0,
			0,
			XtWidth(vw->dvr_viewer.work_window)-actual_increment_size,
			XtHeight(vw->dvr_viewer.work_window),
			actual_increment_size,
			0);

	     ul_x = 0;
	     ul_y = 0;
	     lr_x = actual_increment_size;
	     lr_y = win_height;

	     /* clear the area that needs to be repainted */
	     XFillRectangle(XtDisplay(vw), XtWindow(vw->dvr_viewer.work_window),
			vw->dvr_viewer.Dvr.erase_gc_id, ul_x, ul_y,
			lr_x, lr_y);

	     /* display the new area */

	     stat = dvr_display_page(vw, ul_x, ul_y, lr_x, lr_y);

	     if(DVRFailure(stat))
		return(stat);

	    /*
	    **  Invoke the application specific callback routine for the scroll
	    **  bar event
	    */
	    if (vw->dvr_viewer.scroll_bar_callback != NULL)			/* was .ecallback in XUI */
		dvr_call_scroll_bar_callbacks
			(vw
			, vw->dvr_viewer.Dvr.Xtop
			, vw->dvr_viewer.Dvr.Ytop
			, pag_width
			, pag_height
			, win_width
			, win_height
			);

	    /* adjust the scroll bar sliders */
	    return(dvr_adjust_horizontal_slider(vw));
	  }

      }

    return(DVR_NORMAL);
}

/*
**++
**  ROUTINE NAME:
**	dvr_goto_next_window(vw, advance_page_flag)
**
**  FUNCTIONAL DESCRIPTION:
**	display the next window of the current page;
**	if the advance_page_flag is set, then goto the
**	next page if we're at the end of the current page.
**
**  FORMAL PARAMETERS:
**    	DvrViewerWidget vw;
**    	Boolean		advance_page_flag;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

#if CDA_EXPAND_PROTO == 1

CDAstatus dvr_goto_next_window (DvrViewerWidget	vw,
				Boolean		advance_page_flag)

#else

CDAstatus dvr_goto_next_window (vw, advance_page_flag)
    DvrViewerWidget 	vw;
    Boolean		advance_page_flag;

#endif

{
    /* if widget passed in is zero, return error */
    if (vw == 0) return (DVR_INVADDR);

    /* if a file is open, display the next window
     */

    if (vw->dvr_viewer.Dvr.FileOpen)
      {
	CDAstatus 	stat;
    	struct pag   	*current_page 	= vw->dvr_viewer.Dvr.current_page;
    	unsigned long   pag_width;
    	unsigned long 	pag_height;
    	int		win_width	= XtWidth(vw->dvr_viewer.work_window);
    	int		win_height	= XtHeight(vw->dvr_viewer.work_window);

        get_page_data(current_page, &pag_width, &pag_height);

	/* move to next window of page or next page if we have to */
	if ( vw->dvr_viewer.Dvr.Ytop + win_height >= pag_height)
	  {
	    /*  if the advance page flag is not set, then we're all
	     *  through, return.
	     */
	    if (!advance_page_flag)
		return(DVR_NORMAL);

	    /* if the flag is set, try to go to the next page;*/
	    /* we are already in the final window of this page
	     * goto the next page if this is not the last page
	     */
	    if ( (vw->dvr_viewer.Dvr.DocRead) &&
		 ((struct pag *) vw->dvr_viewer.Dvr.page_list.que_blink ==
			(struct pag *) &(vw->dvr_viewer.Dvr.current_page) ) )
		{
		  /* then this is the last page, return EOD */
		  vw->dvr_viewer.Dvr.WidgetStatus = DVR_EOD;
		  return(DVR_EOD);
	        }
	    else
                {
		  /* get the next page */
		  return(DvrNextPage(vw));
		}
	  }
	else
	  {
	    /* else, the next window is within this page, update pointers
	     * and redisplay
	     */
	    vw->dvr_viewer.Dvr.Ytop = vw->dvr_viewer.Dvr.Ytop + win_height;

	    /* always have a full screen if we can */
	    if (vw->dvr_viewer.Dvr.Ytop + win_height > pag_height)
	      vw->dvr_viewer.Dvr.Ytop = pag_height - win_height;

    	    if (XtIsManaged(vw))
	      {
	    	/* clear window */
	    	XFillRectangle(XtDisplay(vw), XtWindow(vw->dvr_viewer.work_window),
			vw->dvr_viewer.Dvr.erase_gc_id, 0, 0,
			win_width,
			win_height);

	    	stat = dvr_display_page(vw, 0, 0, win_width, win_height);

	        if(DVRFailure(stat))
		  return(stat);

		/*
		**  Invoke the application specific callback routine for the scroll
		**  bar event
		*/
		if (vw->dvr_viewer.scroll_bar_callback != NULL)			/* was .ecallback in XUI */
		    dvr_call_scroll_bar_callbacks
			    (vw
			    , vw->dvr_viewer.Dvr.Xtop
			    , vw->dvr_viewer.Dvr.Ytop
			    , pag_width
			    , pag_height
			    , win_width
			    , win_height
			    );

	    	return(dvr_adjust_vertical_slider(vw));
	      }

	    return(DVR_NORMAL);
      	  }
      }
    else
	return(DVR_FILENOTOPEN);
}

/*
**++
**  ROUTINE NAME:
**	dvr_goto_previous_window(w, advance_page_flag)
**
**  FUNCTIONAL DESCRIPTION:
**	display the previous window of the current page;
**	if the advance_page_flag is set, then goto the
**	previous page if we're at the top of the current page.
**
**  FORMAL PARAMETERS:
**    	DvrViewerWidget vw;
**    	Boolean		advance_page_flag;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

#if CDA_EXPAND_PROTO == 1

CDAstatus dvr_goto_previous_window(DvrViewerWidget	vw,
				   Boolean		advance_page_flag)

#else

CDAstatus dvr_goto_previous_window(vw, advance_page_flag)
    DvrViewerWidget	vw;
    Boolean		advance_page_flag;

#endif

{
    /* if widget passed in is zero, return error */
    if (vw == 0) return (DVR_INVADDR);

    /* if a file is open, display the next window
     */

    if (vw->dvr_viewer.Dvr.FileOpen)
      {
	CDAstatus 	stat;
    	struct pag   	*current_page 	= vw->dvr_viewer.Dvr.current_page;
    	unsigned long 	pag_width;
    	unsigned long 	pag_height;
    	int		win_width	= XtWidth(vw->dvr_viewer.work_window);
    	int		win_height	= XtHeight(vw->dvr_viewer.work_window);

        get_page_data(current_page, &pag_width, &pag_height);

	/* move to previous window of page or previous page if we have to */
	if (vw->dvr_viewer.Dvr.Ytop == 0)
	  {
	    /*  if the advance page flag is not set, then we're all
	     *  through, return.
	     */
	    if (!advance_page_flag)
		return(DVR_NORMAL);

	    /* if the flag is set, try to go to the previous page;
	     * we are already in the first window of this page
	     * goto the previous page if this is not the first page
	     */
	    else if ( (struct pag *) current_page == (struct pag *) &(vw->dvr_viewer.Dvr.page_list.que_flink) )
		{
		  /* then this is the first page, return top-document */
		  vw->dvr_viewer.Dvr.WidgetStatus = DVR_TOPOFDOC;
		  return(DVR_TOPOFDOC);
	        }
	    else
                {
		  /* get the previous page and move to it's last window */
		  stat = dvr_goto_previous_page(vw, TRUE);

		  if (DVRFailure(stat))
		      dvr_error_callback(vw, 0, stat, 0, 0);

		  else
		    {
		      current_page 	= vw->dvr_viewer.Dvr.current_page;
    		      get_page_data(current_page, &pag_width, &pag_height);
		      vw->dvr_viewer.Dvr.Ytop = MAX(0,
						(pag_height - win_height) );

		      dvr_set_page_number(vw);
    	    	      if (XtIsManaged(vw))
	      	        {
	    	          /* clear window */
	    	          XFillRectangle(XtDisplay(vw), XtWindow(vw->dvr_viewer.work_window),
				vw->dvr_viewer.Dvr.erase_gc_id, 0, 0,
				win_width,
				win_height);

	    	          stat = dvr_display_page(vw, 0, 0, win_width, win_height);

	     	          if(DVRFailure(stat))
			    {
			      return(stat);
			    }

			    /*
			    **  Invoke the application specific callback routine for the scroll
			    **  bar event
			    */
			    if (vw->dvr_viewer.scroll_bar_callback != NULL)	/* was .ecallback in XUI */
				dvr_call_scroll_bar_callbacks
					(vw
					, vw->dvr_viewer.Dvr.Xtop
					, vw->dvr_viewer.Dvr.Ytop
					, pag_width
					, pag_height
					, win_width
					, win_height
					);
		        }
	    	      return(dvr_adjust_sliders(vw));

	      	    }
		}
	  }
	else
	  {
	    /* else, the previous window is within this page, update pointers
	     * and redisplay
	     */
	    vw->dvr_viewer.Dvr.Ytop = MAX(0,
			vw->dvr_viewer.Dvr.Ytop - win_height);

    	    if (XtIsManaged(vw))
	      {
	    	/* clear window */
	    	XFillRectangle(XtDisplay(vw), XtWindow(vw->dvr_viewer.work_window),
			vw->dvr_viewer.Dvr.erase_gc_id, 0, 0,
			win_width,
			win_height);

	    	stat = dvr_display_page(vw, 0, 0, win_width, win_height);

	        if(DVRFailure(stat))
		  {
		    return(stat);
		  }

		/*
		**  Invoke the application specific callback routine for the scroll
		**  bar event
		*/
		if (vw->dvr_viewer.scroll_bar_callback != NULL)			/* was .ecallback in XUI */
		    dvr_call_scroll_bar_callbacks
			    (vw
			    , vw->dvr_viewer.Dvr.Xtop
			    , vw->dvr_viewer.Dvr.Ytop
			    , pag_width
			    , pag_height
			    , win_width
			    , win_height
			    );

	    	return(dvr_adjust_vertical_slider(vw));
	      }

	    return(DVR_NORMAL);
      	  }
      }
    else
	return(DVR_FILENOTOPEN);
}

/*
**++
**  ROUTINE NAME:
**	dvr_right_window(vw)
**
**  FUNCTIONAL DESCRIPTION:
**	display the window to the right of the current window in the
**	current page. This routine is called when the user pages forward
**	with the horizontal scroll bar.
**
**  FORMAL PARAMETERS:
**    	DvrViewerWidget vw;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

CDAstatus dvr_right_window (vw)
    DvrViewerWidget 	vw;

{
    /* if widget passed in is zero, return error */
    if (vw == 0) return (DVR_INVADDR);

    /* if a file is open, move to the next window to the right
     */

    if (vw->dvr_viewer.Dvr.FileOpen)
      {
	CDAstatus	stat;
    	struct pag   	*current_page 	= vw->dvr_viewer.Dvr.current_page;
    	unsigned long 	pag_width;
    	unsigned long 	pag_height;
    	int		win_width	= XtWidth(vw->dvr_viewer.work_window);
    	int		win_height	= XtHeight(vw->dvr_viewer.work_window);

    	get_page_data(current_page, &pag_width, &pag_height);

	/* move to next window to the right of page */
	if ( vw->dvr_viewer.Dvr.Xtop + win_width >=  pag_width )
	  {
	    /* then we are already in the final window of this page;
	     * we're all done, return.
	     */
		  return(DVR_NORMAL);
	  }
	else
	  {
	    /* else, the next window is within this page, update pointers
	     * and redisplay
	     */
	    vw->dvr_viewer.Dvr.Xtop = vw->dvr_viewer.Dvr.Xtop + win_width;

	    /* always have a full screen if we can */
	    if (vw->dvr_viewer.Dvr.Xtop + win_width > pag_width)
	      vw->dvr_viewer.Dvr.Xtop = pag_width - win_width;

    	    if (XtIsManaged(vw))
	      {
	    	/* clear window */
	    	XFillRectangle(XtDisplay(vw), XtWindow(vw->dvr_viewer.work_window),
			vw->dvr_viewer.Dvr.erase_gc_id, 0, 0,
			win_width,
			win_height);

	    	stat = dvr_display_page(vw, 0, 0, win_width, win_height);

	        if(DVRFailure(stat))
		  return(stat);

		/*
		**  Invoke the application specific callback routine for the scroll
		**  bar event
		*/
		if (vw->dvr_viewer.scroll_bar_callback != NULL)			/* was .ecallback in XUI */
		    dvr_call_scroll_bar_callbacks
			    (vw
			    , vw->dvr_viewer.Dvr.Xtop
			    , vw->dvr_viewer.Dvr.Ytop
			    , pag_width
			    , pag_height
			    , win_width
			    , win_height
			    );

	    	return(dvr_adjust_horizontal_slider(vw));
	      }

	    return(DVR_NORMAL);
      	  }
      }
    else
	return(DVR_FILENOTOPEN);
}

/*
**++
**  ROUTINE NAME:
**	dvr_left_window(vw)
**
**  FUNCTIONAL DESCRIPTION:
**	display the window to the left of the current window in the
**	current page. This routine is called when the user pages back
**	with the horizontal scroll bar.
**
**  FORMAL PARAMETERS:
**    	DvrViewerWidget vw;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/
CDAstatus dvr_left_window(vw)
    DvrViewerWidget vw;

{
    /* if widget passed in is zero, return error */
    if (vw == 0) return (DVR_INVADDR);

    /* if a file is open, move to the next window to the left
     */

    if (vw->dvr_viewer.Dvr.FileOpen)
      {
	CDAstatus	stat;
    	struct pag   	*current_page 	= vw->dvr_viewer.Dvr.current_page;
    	unsigned long 	pag_width;
    	unsigned long 	pag_height;
    	int		win_width	= XtWidth(vw->dvr_viewer.work_window);
    	int		win_height	= XtHeight(vw->dvr_viewer.work_window);

    	get_page_data(current_page, &pag_width, &pag_height);

	/* move to next window to the left of the current page */
	if (vw->dvr_viewer.Dvr.Xtop == 0)
	  {
	    /* then we are already in the first window of this page;
	     * we're all done, return.
	     */
	    return(DVR_NORMAL);
	  }
	else
	  {
	    /* else, the previous window is within this page, update pointers
	     * and redisplay
	     */
	    vw->dvr_viewer.Dvr.Xtop = MAX(0,
			vw->dvr_viewer.Dvr.Xtop - win_width);

    	    if (XtIsManaged(vw))
	      {
	    	/* clear window */
	    	XFillRectangle(XtDisplay(vw), XtWindow(vw->dvr_viewer.work_window),
			vw->dvr_viewer.Dvr.erase_gc_id, 0, 0,
			win_width,
			win_height);

	    	stat = dvr_display_page(vw, 0, 0, win_width, win_height);

	        if(DVRFailure(stat))
		  return(stat);

		/*
		**  Invoke the application specific callback routine for the scroll
		**  bar event
		*/
		if (vw->dvr_viewer.scroll_bar_callback != NULL)			/* was .ecallback in XUI */
		    dvr_call_scroll_bar_callbacks
			    (vw
			    , vw->dvr_viewer.Dvr.Xtop
			    , vw->dvr_viewer.Dvr.Ytop
			    , pag_width
			    , pag_height
			    , win_width
			    , win_height
			    );

	    	return(dvr_adjust_horizontal_slider(vw));
	      }

	    return(DVR_NORMAL);
      	  }
      }
    else
	return(DVR_FILENOTOPEN);
}

/*
**++
**  ROUTINE NAME:
** 	dvr_to_top_vertical (vw, pixel_value)
**
**  FUNCTIONAL DESCRIPTION:
**	this routine is called when the user clicks on MB2 in the
**	vertical scroll bar. It scrolls the window so that the position
**	clicked in the scroll bar (pixel_value) becomes the top of the window.
**
**  FORMAL PARAMETERS:
**    DvrViewerWidget 	vw;
**    int		pixel_value;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

CDAstatus dvr_to_top_vertical (vw, pixel_value)
    DvrViewerWidget 	vw;
    int			pixel_value;

{
    CDAstatus		stat;
    struct pag   	*current_page 	= vw->dvr_viewer.Dvr.current_page;
    unsigned long	pag_width;
    unsigned long 	pag_height;
    int			win_width	= XtWidth(vw->dvr_viewer.work_window);
    int			win_height	= XtHeight(vw->dvr_viewer.work_window);


    get_page_data(current_page, &pag_width, &pag_height);

    if (pixel_value > ( pag_height-(vw->dvr_viewer.Dvr.Ytop) ))
	/* then the value is below any displayed ddif, return */
	return(DVR_NORMAL);

    else
      {
	/* value is valid, scroll the window */
	vw->dvr_viewer.Dvr.Ytop = vw->dvr_viewer.Dvr.Ytop + pixel_value;

	/* copy from pixel_value to the end up to the top */
	XCopyArea(XtDisplay(vw),
		  XtWindow(vw->dvr_viewer.work_window),
		  XtWindow(vw->dvr_viewer.work_window),
		  vw->dvr_viewer.Dvr.GcId,
		  0,
		  pixel_value,
		  XtWidth(vw->dvr_viewer.work_window),
		  XtHeight(vw->dvr_viewer.work_window)-pixel_value,
		  0,
		  0);

	/* erase from bottom-value to bottom */
	XFillRectangle(XtDisplay(vw), XtWindow(vw->dvr_viewer.work_window),
			vw->dvr_viewer.Dvr.erase_gc_id,
			0,
			win_height-pixel_value,
			win_width,
			win_height);

	/* repaint from bottom-value to bottom, adjust scroll bars */
	stat = dvr_display_page(vw, 0, win_height-pixel_value,
				win_width, win_height);

	if(DVRFailure(stat))
	  return(stat);

	/*
	**  Invoke the application specific callback routine for the scroll
	**  bar event
	*/
	if (vw->dvr_viewer.scroll_bar_callback != NULL)				/* was .ecallback in XUI */
	    dvr_call_scroll_bar_callbacks
		    (vw
		    , vw->dvr_viewer.Dvr.Xtop
		    , vw->dvr_viewer.Dvr.Ytop
		    , pag_width
		    , pag_height
		    , win_width
		    , win_height
		    );

	return(dvr_adjust_vertical_slider(vw));

      }
}

/*
**++
**  ROUTINE NAME:
** 	dvr_to_bottom_vertical (vw, pixel_value)
**
**  FUNCTIONAL DESCRIPTION:
**	this routine is called when the user clicks on MB3 in the
**	vertical scroll bar. It scrolls the window so that the position
**	clicked in the scroll bar (pixel_value) becomes the bottom of the window.
**
**  FORMAL PARAMETERS:
**    DvrViewerWidget 	vw;
**    int		pixel_value;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

CDAstatus dvr_to_bottom_vertical (vw, pixel_value)
    DvrViewerWidget 	vw;
    int			pixel_value;

{
    CDAstatus		stat;
    struct pag   	*current_page 	= vw->dvr_viewer.Dvr.current_page;
    unsigned long 	pag_width;
    unsigned long	pag_height;
    int			win_width	= XtWidth(vw->dvr_viewer.work_window);
    int			win_height	= XtHeight(vw->dvr_viewer.work_window);

    get_page_data(current_page, &pag_width, &pag_height);

    if (pixel_value > ( pag_height-(vw->dvr_viewer.Dvr.Ytop) ))
	/* then the value is below any displayed ddif, return */
	return(DVR_NORMAL);

    else
      {
	/* value is valid, scroll the window */
	int save_y_top, actual_value;

	/*  save the Ytop value in case we cannot move the value requested
	 *  all the way to the bottom (not far enough into page); in this case,
	 *  move to the top of the page
	 */
	save_y_top = vw->dvr_viewer.Dvr.Ytop;

	vw->dvr_viewer.Dvr.Ytop = MAX(0,
			(vw->dvr_viewer.Dvr.Ytop - (win_height-pixel_value)) );

	actual_value = save_y_top - vw->dvr_viewer.Dvr.Ytop;

	/* copy from the top to the pixel_value to the bottom */
	XCopyArea(XtDisplay(vw),
		  XtWindow(vw->dvr_viewer.work_window),
		  XtWindow(vw->dvr_viewer.work_window),
		  vw->dvr_viewer.Dvr.GcId,
		  0,
		  0,
		  XtWidth(vw->dvr_viewer.work_window),
		  XtHeight(vw->dvr_viewer.work_window) - actual_value,
		  0,
		  actual_value);

	/* erase from the top to the actual value used */
	XFillRectangle(XtDisplay(vw), XtWindow(vw->dvr_viewer.work_window),
			vw->dvr_viewer.Dvr.erase_gc_id,
			0,
			0,
			win_width,
			actual_value);

	/* repaint from bottom-value to bottom, adjust scroll bars */
	stat = dvr_display_page(vw, 0, 0,
				win_width, actual_value);
	if(DVRFailure(stat))
	  return(stat);

	/*
	**  Invoke the application specific callback routine for the scroll
	**  bar event
	*/
	if (vw->dvr_viewer.scroll_bar_callback != NULL)				/* was .ecallback in XUI */
	    dvr_call_scroll_bar_callbacks
		    (vw
		    , vw->dvr_viewer.Dvr.Xtop
		    , vw->dvr_viewer.Dvr.Ytop
		    , pag_width
		    , pag_height
		    , win_width
		    , win_height
		    );

	return(dvr_adjust_vertical_slider(vw));

      }
}

/*
**++
**  ROUTINE NAME:
** 	dvr_to_top_horizontal (vw, pixel_value)
**
**  FUNCTIONAL DESCRIPTION:
**	this routine is called when the user clicks on MB2 in the
**	horizontal scroll bar. It scrolls the window so that the position
**	clicked in the scroll bar (pixel_value) becomes the far left of the window.
**
**  FORMAL PARAMETERS:
**    DvrViewerWidget 	vw;
**    int		pixel_value;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

CDAstatus dvr_to_top_horizontal (vw, pixel_value)
    DvrViewerWidget 	vw;
    int			pixel_value;

{
    CDAstatus		stat;
    struct pag   	*current_page 	= vw->dvr_viewer.Dvr.current_page;
    unsigned long 	pag_width;
    unsigned long 	pag_height;
    int			win_width	= XtWidth(vw->dvr_viewer.work_window);
    int			win_height	= XtHeight(vw->dvr_viewer.work_window);

    get_page_data(current_page, &pag_width, &pag_height);

    if (pixel_value > ( pag_width-(vw->dvr_viewer.Dvr.Xtop) ))
	/* then the value is to the right of any displayed ddif, return */
	return(DVR_NORMAL);

    else
      {
	/* value is valid, scroll the window */
	vw->dvr_viewer.Dvr.Xtop = vw->dvr_viewer.Dvr.Xtop + pixel_value;

	/* copy from pixel_value to the far right over to the far left */
	XCopyArea(XtDisplay(vw),
		  XtWindow(vw->dvr_viewer.work_window),
		  XtWindow(vw->dvr_viewer.work_window),
		  vw->dvr_viewer.Dvr.GcId,
		  pixel_value,
		  0,
		  XtWidth(vw->dvr_viewer.work_window)-pixel_value,
		  XtHeight(vw->dvr_viewer.work_window),
		  0,
		  0);

	/* erase from far_right minus pixel_value to the far right */
	XFillRectangle(XtDisplay(vw), XtWindow(vw->dvr_viewer.work_window),
			vw->dvr_viewer.Dvr.erase_gc_id,
			win_width-pixel_value,
			0,
			win_width,
			win_height);

	/* repaint cleared area , adjust scroll bars */
	stat = dvr_display_page(vw, win_width- pixel_value, 0,
				win_width, win_height);
	if(DVRFailure(stat))
	  return(stat);

	/*
	**  Invoke the application specific callback routine for the scroll
	**  bar event
	*/
	if (vw->dvr_viewer.scroll_bar_callback != NULL)				/* was .ecallback in XUI */
	    dvr_call_scroll_bar_callbacks
		    (vw
		    , vw->dvr_viewer.Dvr.Xtop
		    , vw->dvr_viewer.Dvr.Ytop
		    , pag_width
		    , pag_height
		    , win_width
		    , win_height
		    );

	return(dvr_adjust_horizontal_slider(vw));

      }
}

/*
**++
**  ROUTINE NAME:
** 	dvr_to_bottom_horizontal (vw, pixel_value)
**
**  FUNCTIONAL DESCRIPTION:
**	this routine is called when the user clicks on MB3 in the
**	horizontal scroll bar. It scrolls the window so that the position
**	clicked in the scroll bar (pixel_value) becomes the right edge of
**	the window.
**
**  FORMAL PARAMETERS:
**    DvrViewerWidget 	vw;
**    int		pixel_value;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

CDAstatus dvr_to_bottom_horizontal (vw, pixel_value)
    DvrViewerWidget 	vw;
    int			pixel_value;

{
    CDAstatus		stat;
    struct pag   	*current_page 	= vw->dvr_viewer.Dvr.current_page;
    unsigned long 	pag_width;
    unsigned long 	pag_height;
    int			win_width	= XtWidth(vw->dvr_viewer.work_window);
    int			win_height	= XtHeight(vw->dvr_viewer.work_window);

    get_page_data(current_page, &pag_width, &pag_height);

    if (pixel_value > ( pag_width-(vw->dvr_viewer.Dvr.Xtop) ))
	/* then the value is to the right of any displayed ddif, return */
	return(DVR_NORMAL);

    else
      {
	/* value is valid, scroll the window */
	int save_x_top, actual_value;

	/*  save the Xtop value in case we cannot move the value requested
	 *  all the way to the right (page not wide enough); in this case,
	 *  move to the far left of the page
	 */
	save_x_top = vw->dvr_viewer.Dvr.Xtop;

	vw->dvr_viewer.Dvr.Xtop = MAX(0,
			(vw->dvr_viewer.Dvr.Xtop - (win_width-pixel_value)) );

	actual_value = save_x_top - vw->dvr_viewer.Dvr.Xtop;

	/* copy from the far left to the pixel_value to the far right */
	XCopyArea(XtDisplay(vw),
		  XtWindow(vw->dvr_viewer.work_window),
		  XtWindow(vw->dvr_viewer.work_window),
		  vw->dvr_viewer.Dvr.GcId,
		  0,
		  0,
		  XtWidth(vw->dvr_viewer.work_window) - actual_value,
		  XtHeight(vw->dvr_viewer.work_window),
		  actual_value,
		  0);

	/* erase from the top to the actual value used */
	XFillRectangle(XtDisplay(vw), XtWindow(vw->dvr_viewer.work_window),
			vw->dvr_viewer.Dvr.erase_gc_id,
			0,
			0,
			actual_value,
			win_height);

	/* repaint from bottom-value to bottom, adjust scroll bars */
	stat = dvr_display_page(vw, 0, 0,
				actual_value, win_height);
	if(DVRFailure(stat))
	  return(stat);

	/*
	**  Invoke the application specific callback routine for the scroll
	**  bar event
	*/
	if (vw->dvr_viewer.scroll_bar_callback != NULL)				/* was .ecallback in XUI */
	    dvr_call_scroll_bar_callbacks
		    (vw
		    , vw->dvr_viewer.Dvr.Xtop
		    , vw->dvr_viewer.Dvr.Ytop
		    , pag_width
		    , pag_height
		    , win_width
		    , win_height
		    );

	return(dvr_adjust_horizontal_slider(vw));

      }
}

/*
**++
**  ROUTINE NAME:
** 	dvr_drag_vertical (vw, slider_value)
**
**  FUNCTIONAL DESCRIPTION:
**	this routine is called when the user drags the
**	vertical scroll bar. It scrolls the percentage indicated
**	by slider_value.
**
**  FORMAL PARAMETERS:
**    DvrViewerWidget 	vw;
**    int		slider_value;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

CDAstatus dvr_drag_vertical (vw, slider_value)
    DvrViewerWidget 	vw;
    int			slider_value;

{
    CDAstatus		stat;
    struct pag   	*current_page 	= vw->dvr_viewer.Dvr.current_page;
    unsigned long 	pag_width;
    unsigned long 	pag_height;
    int			win_width	= XtWidth(vw->dvr_viewer.work_window);
    int			win_height	= XtHeight(vw->dvr_viewer.work_window);
    float		percentage;

    get_page_data(current_page, &pag_width, &pag_height);

    /* determine the new Ytop value */
    percentage = (float) ((float) slider_value / (float) DVR_MAX_SCROLL_VALUE);

    vw->dvr_viewer.Dvr.Ytop = (int) (pag_height * (float) percentage);

    /* erase the window, and redraw, we should not have to adjust sliders */
    XFillRectangle(XtDisplay(vw), XtWindow(vw->dvr_viewer.work_window),
			vw->dvr_viewer.Dvr.erase_gc_id,
			0,
			0,
			win_width,
			win_height);

    stat = dvr_display_page(vw, 0, 0,
    		win_width, win_height);

    /*
    **  Invoke the application specific callback routine for the scroll
    **  bar event
    */
    if (vw->dvr_viewer.scroll_bar_callback != NULL)				/* was .ecallback in XUI */
	dvr_call_scroll_bar_callbacks
		(vw
		, vw->dvr_viewer.Dvr.Xtop
		, vw->dvr_viewer.Dvr.Ytop
		, pag_width
		, pag_height
		, win_width
		, win_height
		);

    return(stat);
}

/*
**++
**  ROUTINE NAME:
** 	dvr_drag_horizontal (vw, slider_value)
**
**  FUNCTIONAL DESCRIPTION:
**	this routine is called when the user drags the
**	horizontal scroll bar. It scrolls the percentage indicated
**	by slider_value.
**
**  FORMAL PARAMETERS:
**    DvrViewerWidget 	vw;
**    int		slider_value;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

CDAstatus dvr_drag_horizontal (vw, slider_value)
    DvrViewerWidget 	vw;
    int			slider_value;

{
    CDAstatus	 	stat;
    struct pag   	*current_page 	= vw->dvr_viewer.Dvr.current_page;
    unsigned long 	pag_width;
    unsigned long 	pag_height;
    int			win_width	= XtWidth(vw->dvr_viewer.work_window);
    int			win_height	= XtHeight(vw->dvr_viewer.work_window);
    float		percentage;

    get_page_data(current_page, &pag_width, &pag_height);

    /* determine the new Xtop value */
    percentage = (float) ((float) slider_value / (float) DVR_MAX_SCROLL_VALUE);

    vw->dvr_viewer.Dvr.Xtop = (int) (pag_width * (float) percentage);

    /* erase the window, and redraw, we should not have to adjust sliders */
    XFillRectangle(XtDisplay(vw), XtWindow(vw->dvr_viewer.work_window),
			vw->dvr_viewer.Dvr.erase_gc_id,
			0,
			0,
			win_width,
			win_height);

    /* repaint from bottom-value to bottom, adjust scroll bars */
    stat = dvr_display_page(vw, 0, 0,
    		win_width, win_height);

    /*
    **  Invoke the application specific callback routine for the scroll
    **  bar event
    */
    if (vw->dvr_viewer.scroll_bar_callback != NULL)				/* was .ecallback in XUI */
	dvr_call_scroll_bar_callbacks
		(vw
		, vw->dvr_viewer.Dvr.Xtop
		, vw->dvr_viewer.Dvr.Ytop
		, pag_width
		, pag_height
		, win_width
		, win_height
		);

    return(stat);
}


/*
**++
**  ROUTINE NAME:
** 	dvr_reset_ps_scrollbars (vw)
**
**  FUNCTIONAL DESCRIPTION:
**	this routine is called when the user goes to a new ps
**	page to view; call Xt to reset the scroll bars to the upper left
**
**  FORMAL PARAMETERS:
**    DvrViewerWidget 	vw;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

#ifdef DVR_PSVIEW

void dvr_reset_ps_scrollbars(vw)
    DvrViewerWidget vw;

{
    int	value_return;			/* ScrollBar's slider position		*/
    int	slider_size_return;		/* Size of the slider			*/
    int	increment_return;		/* Returns the amount of button inc/dec */
    int	page_increment_return;		/* Returns the amount of page   inc/dec */

    /*
     * The first step is to find out what values are legal so we can put them
     * back with only the value changed.
    */

    if (PSvbar(vw) != NULL)
      {
    	XmScrollBarGetValues(PSvbar(vw),
    			 &value_return,
			 &slider_size_return,
			 &increment_return,
			 &page_increment_return);

    	/*
    	 * Now put back the same information with the value changed so the slider
    	 * will move back to it's original value (and notify the value changed callback.
    	*/

    	XmScrollBarSetValues(PSvbar(vw),
    			 0,					/* Note we're resetting the scroll bar */
			 slider_size_return,
			 increment_return,
			 page_increment_return,
			 TRUE);					/* Notify XmNvalueChanged callback routines */
      }

    /*
     * Now do the same thing for the horizontal slider.
    */

    if (PShbar(vw) != NULL)
      {
    	XmScrollBarGetValues(PShbar(vw),
    			 &value_return,
			 &slider_size_return,
			 &increment_return,
			 &page_increment_return);

    	XmScrollBarSetValues(PShbar(vw),
    			 0,					/* Note we're resetting the scroll bar */
			 slider_size_return,
			 increment_return,
			 page_increment_return,
			 TRUE);					/* Notify XmNvalueChanged callback routines */
      }

}

#endif



/*
 *  C BINDINGS FOR EXTERNAL ENTRY POINTS BEGIN HERE
 */

/*
**++
**  ROUTINE NAME:
**	DvrNextWindow(w)
**
**  FUNCTIONAL DESCRIPTION:
**	This routine calls the routine to display
**	the next page for a viewer widget.
**
**  FORMAL PARAMETERS:
**    	Widget w;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

CDAstatus DvrNextWindow(w)
    Widget w;

{
    CDAstatus stat;

    stat = dvr_goto_next_window((DvrViewerWidget) w, TRUE);

    if (DVRFailure(stat))
        dvr_error_callback((DvrViewerWidget) w, 0L, stat, 0, 0L);

    return(stat);
}


/*
**++
**  ROUTINE NAME:
**	DvrPreviousWindow(w)
**
**  FUNCTIONAL DESCRIPTION:
**	This routine calls the routine to display
**	the previous page for a viewer widget.
**
**  FORMAL PARAMETERS:
**    	Widget w;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

CDAstatus DvrPreviousWindow(w)
    Widget w;

{
    CDAstatus stat;

    stat = dvr_goto_previous_window((DvrViewerWidget) w, TRUE);

    if (DVRFailure(stat))
        dvr_error_callback((DvrViewerWidget) w, 0L, stat, 0, 0L);

    return(stat);
}

/*
**++
**  ROUTINE NAME:
**	DvrTopDocument(w)
**
**  FUNCTIONAL DESCRIPTION:
**	This routine calls the routine to display
**	the top of a document for a ddif viewer widget.
**
**  FORMAL PARAMETERS:
**    	DvrViewerWidget w;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

CDAstatus DvrTopDocument(w)
    DvrViewerWidget w;

{
    DvrViewerWidget vw = w;

    /* if widget passed in is zero, return error */
    if (vw == 0) return (DVR_INVADDR);

    /* if a file is open, loop until we get to the top
     * of the document.
     */

    if (vw->dvr_viewer.Dvr.FileOpen)
      {

#ifdef DVR_PSVIEW
	if (VIEWING_PS(vw)) /* viewing PS */
	  {
	    if (vw->dvr_viewer.Ps.current_page == 1)
	   	return(DVR_TOPOFDOC);
	    else
	      {
		dvr_set_watch_cursor(vw);
		dvr_ps_change_page(vw, PS_GOTO, 1);
		dvr_update_page_number(vw);
		dvr_reset_cursor(vw);
		return(DVR_NORMAL);
	      }
	  }

	else /* normal CDA viewing */
#endif

	  {
	    CDAstatus stat = 0;

            /*  if the document is open, check to see if we're
	     *  already at the top; if so, return
	     */

    	    struct pag  *current_page 	= vw->dvr_viewer.Dvr.current_page;
    	    int	    	page_num;

	    struct 	pag *first_page =
		  	  (struct pag *) vw->dvr_viewer.Dvr.page_list.que_flink;
 	    int  	first_page_num;

            if (current_page == NULL)
	        page_num = 1;
	    else
	        page_num = current_page->pag_page_number;

            if ( (first_page == NULL) || (current_page == NULL) )
	        first_page_num = 1;
	    else
	        first_page_num = first_page->pag_page_number;

	    if (page_num == first_page_num)
	        return(DVR_TOPOFDOC);

	    dvr_set_watch_cursor(vw);
	    while (stat != DVR_TOPOFDOC)
	      {
	        stat = dvr_goto_previous_page(vw, FALSE);
	        if(DVRFailure(stat))
	          {
		    dvr_error_callback(vw, 0, stat, 0, 0);
  		    dvr_reset_cursor(vw);
		    return(stat);
	          }
	      }
	    dvr_initialize_window(vw);

	    vw->dvr_viewer.Dvr.WidgetStatus = stat;

	    dvr_reset_cursor(vw);
	    dvr_update_page_number(vw);
	    return(vw->dvr_viewer.Dvr.WidgetStatus);
	  }

      }
    else
	return(DVR_FILENOTOPEN);
}

/*
**++
**  ROUTINE NAME:
**	DvrBottomDocument(w)
**
**  FUNCTIONAL DESCRIPTION:
**	This routine calls the routine to display
**	the bottom of a document for a ddif viewer widget.
**
**  FORMAL PARAMETERS:
**    	DvrViewerWidget w;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/

CDAstatus DvrBottomDocument(w)
    DvrViewerWidget w;

{
    DvrViewerWidget vw = w;

    /* if widget passed in is zero, return error */
    if (vw == 0) return (DVR_INVADDR);

    /* if a file is open, loop until we get to the bottom
     * of the document.
     */

    if (vw->dvr_viewer.Dvr.FileOpen)
      {
#ifdef DVR_PSVIEW
	if (VIEWING_PS(vw)) /* viewing ps ? */
	  {
	    if (vw->dvr_viewer.Ps.last_page != 0)
	      {
		/* if we know how many pages we have, go to end if
		 * we're not already there
		 */
		if (vw->dvr_viewer.Ps.current_page ==
		    vw->dvr_viewer.Ps.last_page)
		    /* already at bottom */
		    return(DVR_EOD);
		else
		  {
		    dvr_set_watch_cursor(vw);
		    dvr_ps_change_page(vw, PS_GOTO,
				       vw->dvr_viewer.Ps.last_page);
		    dvr_update_page_number(vw);
		    dvr_reset_cursor(vw);
		    return(DVR_NORMAL);
		  }
	      }

	    else
	      {
		/* not sure how many pages we have, go to large number */
		dvr_set_watch_cursor(vw);
		dvr_ps_change_page(vw, PS_GOTO, PS_LAST_PAGE);
		dvr_update_page_number(vw);
		dvr_reset_cursor(vw);
	  	return(DVR_NORMAL);
	      }
	  }

	else
#endif

	  {
	     /* normal CDA Viewing */

	    CDAstatus stat = 0;

    	    if (vw->dvr_viewer.Dvr.DocRead)
              {
                /*  if the document is fully read, check to see if we're
	         *  already at the bottom; if so, return
	         */

    	        struct pag  *current_page = vw->dvr_viewer.Dvr.current_page;
    	        int	    page_num;

	        struct pag  *last_page =
		  	  (struct pag *) vw->dvr_viewer.Dvr.page_list.que_blink;
 	        int  	    last_page_num;

                if (current_page == NULL)
	    	    page_num = 1;
	        else
	    	    page_num = current_page->pag_page_number;

                if ( (last_page == NULL) || (current_page == NULL) )
	            last_page_num = 1;
	        else
	            last_page_num = last_page->pag_page_number;

	        if (page_num == last_page_num)
	    	    return(DVR_EOD);

              }

	    dvr_set_watch_cursor(vw);

	    stat = dvr_goto_next_page(vw, FALSE);

	    if (stat != DVR_EOD)
	      {
	        while (stat != DVR_EOD)
	          {
	            stat = dvr_goto_next_page(vw, FALSE);

	            if (DVRFailure(stat))
	              {
		        dvr_error_callback(vw, 0, stat, 0, 0);
		        dvr_reset_cursor(vw);
		        return(stat);
	              }
	          }
	        dvr_initialize_window(vw);
	      }
	    else
	        dvr_set_page_number(vw);

	    vw->dvr_viewer.Dvr.WidgetStatus = stat;

	    dvr_reset_cursor(vw);
	    dvr_update_page_number(vw);
	    dvr_call_eod_callbacks(vw);

	    return(vw->dvr_viewer.Dvr.WidgetStatus);
          }

      }
    else
	return(DVR_FILENOTOPEN);
}

/*
**++
**  ROUTINE NAME:
**	DvrNextPage(w)
**
**  FUNCTIONAL DESCRIPTION:
**	This routine displays the next page of a ddif document
**
**  FORMAL PARAMETERS:
**    	DvrViewerWidget w;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/
CDAstatus DvrNextPage(w)

    DvrViewerWidget w;

{
    DvrViewerWidget vw = w;
    Boolean	    call_eod_callbacks = FALSE;

    /* if widget passed in is zero, return error */
    if (vw == 0) return (DVR_INVADDR);

    /* if a file is open, get the next page
     */

    if (vw->dvr_viewer.Dvr.FileOpen)
      {
#ifdef DVR_PSVIEW
        if (VIEWING_PS(vw))
	  {
	    /* if the ps window is managed, deal with it specially */

	    if (vw->dvr_viewer.Ps.current_page ==
		vw->dvr_viewer.Ps.last_page)
		return(DVR_EOD);

	    else
	      {
		dvr_set_watch_cursor(vw);
		dvr_ps_change_page(vw, PS_NEXT, 0);
		dvr_update_page_number(vw);
		dvr_reset_cursor(vw);
	        return(DVR_NORMAL);
	      }
 	  }
    	else
#endif
	  /* normal CDA viewing */
	  {
	    if (vw->dvr_viewer.Dvr.DocRead)
              {
                /*  if the document is fully read, check to see if we're
	         *  already at the bottom; if so, return
	         */

    	        struct pag  *current_page = vw->dvr_viewer.Dvr.current_page;
    	        int	   page_num;

	        struct 	pag *last_page =
		  	(struct pag *) vw->dvr_viewer.Dvr.page_list.que_blink;
 	        int  	last_page_num;

                if (current_page == NULL)
	    	    page_num = 1;
	        else
	    	    page_num = current_page->pag_page_number;

                if ( (last_page == NULL) || (current_page == NULL) )
	            last_page_num = 1;
	        else
	            last_page_num = last_page->pag_page_number;

	        if (page_num == last_page_num)
	    	    return(DVR_EOD);

		call_eod_callbacks = (last_page_num == page_num+1);
              }

	    dvr_set_watch_cursor(vw);
	    vw->dvr_viewer.Dvr.WidgetStatus = dvr_goto_next_page(vw, TRUE);

	    if (DVRFailure(vw->dvr_viewer.Dvr.WidgetStatus))
	      {
	        dvr_error_callback(vw, 0, vw->dvr_viewer.Dvr.WidgetStatus, 0, 0);
	        dvr_reset_cursor(vw);
	        return(vw->dvr_viewer.Dvr.WidgetStatus);
	      }

     	    if (vw->dvr_viewer.Dvr.WidgetStatus != DVR_EOD)
	      {
	        dvr_initialize_window(vw);
		if (call_eod_callbacks)
		    dvr_call_eod_callbacks(vw);
	      }
	    else
	      {
	        dvr_set_page_number(vw);
		dvr_call_eod_callbacks(vw);
	      }

	    dvr_reset_cursor(vw);
	    dvr_update_page_number(vw);
	    return(vw->dvr_viewer.Dvr.WidgetStatus);

	  }
      }
    else
	return(DVR_FILENOTOPEN);
}

/*
**++
**  ROUTINE NAME:
**	DvrPreviousPage(w)
**
**  FUNCTIONAL DESCRIPTION:
**	This routine displays the previous page of a ddif
**	document if there is one.
**
**  FORMAL PARAMETERS:
**    	DvrViewerWidget w;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/
CDAstatus DvrPreviousPage(w)

    DvrViewerWidget w;

{
    DvrViewerWidget vw = w;

    /* if widget passed in is zero, return error */
    if (vw == 0) return (DVR_INVADDR);

    /* if a file is open, get the next page
     */

    if (vw->dvr_viewer.Dvr.FileOpen)
      {
#ifdef DVR_PSVIEW
	if (VIEWING_PS(vw))
	  {
	    /* if ps window is managed, deal with specially */

	    if (vw->dvr_viewer.Ps.current_page == 1)
		return(DVR_TOPOFDOC);
	    else
	      {
		dvr_set_watch_cursor(vw);
		dvr_ps_change_page(vw, PS_PREVIOUS, 0);
		dvr_update_page_number(vw);
		dvr_reset_cursor(vw);
	    	return(DVR_NORMAL);
	      }
	  }
	else
#endif
	  /* normal CDA viewing */
	  {
            /*  if the document is open, check to see if we're
	     *  already at the top; if so, return
	     */

    	    struct pag  *current_page 	= vw->dvr_viewer.Dvr.current_page;
    	    int	    page_num;

	    struct 	pag *first_page =
		  	(struct pag *) vw->dvr_viewer.Dvr.page_list.que_flink;
 	    int  	first_page_num;

            if (current_page == NULL)
	        page_num = 1;
	    else
	        page_num = current_page->pag_page_number;

            if ( (first_page == NULL) || (current_page == NULL) )
	        first_page_num = 1;
	    else
	        first_page_num = first_page->pag_page_number;

	    if (page_num == first_page_num)
	        return(DVR_TOPOFDOC);

	    dvr_set_watch_cursor(vw);

	    vw->dvr_viewer.Dvr.WidgetStatus = dvr_goto_previous_page(vw, TRUE);

	    if (DVRFailure(vw->dvr_viewer.Dvr.WidgetStatus))
	      {
	        dvr_error_callback(vw, 0, vw->dvr_viewer.Dvr.WidgetStatus, 0, 0);
	        dvr_reset_cursor(vw);
	        return(vw->dvr_viewer.Dvr.WidgetStatus);
	      }

	    if (vw->dvr_viewer.Dvr.WidgetStatus != DVR_TOPOFDOC)
	        dvr_initialize_window(vw);
	    else
	        dvr_set_page_number(vw);

	    dvr_reset_cursor(vw);
	    dvr_update_page_number(vw);
	    return(vw->dvr_viewer.Dvr.WidgetStatus);
	  }

      }
    else
	return(DVR_FILENOTOPEN);
}

/*
**++
**  ROUTINE NAME:
**	DvrGotoPage(w, page_num)
**
**  FUNCTIONAL DESCRIPTION:
**	this routine will attempt to move the document to the page number
**	passed in
**  FORMAL PARAMETERS:
**    	DvrViewerWidget w;
**	unsigned long page_num;
**
**  IMPLICIT INPUTS:
** 	none
**
**  IMPLICIT OUTPUTS:
**	none
**
**  FUNCTION VALUE:
**	none
**
**  SIDE EFFECTS:
**	none
**--
**/
CDAstatus DvrGotoPage(w, page_num)

    DvrViewerWidget 	w;
    CDAconstant 	page_num;

{
    DvrViewerWidget vw = w;
    CDAstatus stat;

    /* if widget passed in is zero, return error */
    if (vw == 0) return (DVR_INVADDR);

    /* if a file is open, get the next page
     */

    if (vw->dvr_viewer.Dvr.FileOpen)
      {
#ifdef DVR_PSVIEW
	if (VIEWING_PS(vw)) /* ps viewing */
	  {
	    if (vw->dvr_viewer.Ps.current_page != page_num)
	        /* make sure we're not already on requested page */
	      {
		dvr_set_watch_cursor(vw);
		dvr_ps_change_page(vw, PS_GOTO, page_num);
		dvr_update_page_number(vw);
		dvr_reset_cursor(vw);
	      }
	    return(DVR_NORMAL);
	  }

	else
#endif
	  {
	    unsigned long pag_width, pag_height;
	    struct pag    *current_page   = vw->dvr_viewer.Dvr.current_page;
	    CDAconstant   save_page_num   = page_num;

    	    dvr_set_watch_cursor(vw);
    	    stat = dvr_goto_page_number(vw, TRUE, &page_num);
    	    if (DVRSuccess(stat))
	      {
	        current_page = vw->dvr_viewer.Dvr.current_page;
	        dvr_initialize_window(vw);
		if (
		     (stat == DVR_EOD) ||
		     (  (page_num != save_page_num) &&
			(vw->dvr_viewer.Dvr.DocRead) )
		   )
		    dvr_call_eod_callbacks(vw);
	      }
    	    else
	       dvr_error_callback(vw, 0, stat, 0, 0);

	    get_page_data(current_page, &pag_width, &pag_height);
	    /*
	    **  Invoke the application specific callback routine for the scroll
	    **  bar event
	    */
	    if (vw->dvr_viewer.scroll_bar_callback != NULL)			/* was .ecallback in XUI */
		dvr_call_scroll_bar_callbacks
			(vw
			, vw->dvr_viewer.Dvr.Xtop
			, vw->dvr_viewer.Dvr.Ytop
			, pag_width
			, pag_height
			, XtWidth(vw->dvr_viewer.work_window)
			, XtHeight(vw->dvr_viewer.work_window)
			);

    	    dvr_reset_cursor(vw);
	    dvr_update_page_number(vw);
    	    return(stat);
	  }
      }
    else
	return(DVR_FILENOTOPEN);

}
