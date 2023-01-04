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
#define Module DVR_PS
#define Ident  "V02-017"

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
**	This module contains various routines for manipulating the
**	postscript previewer widget contained within the CDA viewer widget
**
** AUTHORS:
**      Dennis McEvoy
**
**
** MODIFIED BY:
**
**	V02-001		DAM0001		Dennis McEvoy	19-oct-1989
**		initial creation
**	V02-002		DAM0002		Dennis McEvoy	22-feb-1990
**		add smarts for dealing with button-row, page-number resources
**	V02-003		DAM0003		Dennis McEvoy	16-jul-1990
**		use y dpi from viewer
**	V02-004		SJM0000		Stephen Munyan	28-Jun-1990
**		Conversion to Motif
**	V02-005		DAM0005		Dennis McEvoy	23-jan-1991
**		fix ps ok callback
**	V02-006		DAM0005		Dennis McEvoy	05-feb-1991
**		fix end of doc callback
**	V02-007		RTG0001		Dick Gumbel	05-Mar-1991
**		Cleanup #include's
**	V02-008		DAM0001		Dennis McEvoy	28-Mar-1991
**		switch to Xm scrollwindow widget for ps scrolling
**	V02-009		DAM0001		Dennis McEvoy	03-apr-1991
**		cleanups
**	V02-010		DAM0001		Dennis McEvoy	04-apr-1991
**		fix accvio in ps error handler
**      V02-011	        DAM0001		Dennis McEvoy	06-may-1991
**		move dvr_int include to cleanup ultrix build
**      V02-012	        DAM0001		Dennis McEvoy	17-may-1991
**		fix proto to match decl
**      V02-013	        DAM0001		Dennis McEvoy	10-jun-1991
**		update error abort to match uws
**	V02-014		SJM0000		Stephen Munyan	26-Jun-1991
**		DEC C Cleanups to PROTO statements.
**      V02-015	        DAM0001		Dennis McEvoy	05-aug-1991
**		renamed header files, removed dollar signs
**      V02-016	        DAM0001		Dennis McEvoy	09-jan-1992
**		use exact dpi to make positioning exact on servers with
**		non standard dpi
**	V02-017		RMM0001		Bob Meagher	07-Apr-1992
**		Changes to support incremental processing of PS files.
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

#include <Xm/Xm.h>				/* Motif definitions */
#include <Mrm/MrmAppl.h>			/* Motif Resource Manager defintions */

#pragma standard				/* turn /stand=port back on */

#else	/* UNIX - and for all others, this is the best default */

#include <Xm/Xm.h>				/* Motif definitions */
#include <Mrm/MrmAppl.h>			/* Motif Resource Manager defintions */

#include <dvrint.h>				/* DVR internal definitions */


#endif

#include <dvrwdef.h>				/* Public Defns w/dvr_include */
#include <dvrwint.h>				/* Viewer Constants and structures  */
#include <psviewwg.h>

/*
 * EXTERNALS AND GLOBALS
 */

/* protos */
PROTO( void dvr_reset_ps_scrollbars,
    	(DvrViewerWidget) );

static void dvr_ps_status_proc();
void dvr_ps_text_proc();
void dvr_ps_error_proc();

DvrViewerWidget 	dvr_find_viewer_widget ();



/*
**++
**  ROUTINE NAME:
**	dvr_init_ps(w)
**
**  FUNCTIONAL DESCRIPTION:
**	initialize the postscript previewer widget for use within the
**	CDA viewer widget
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

void dvr_init_ps(vw)

    DvrViewerWidget vw;

{
    Display *dpy;
    Screen *screen;
    double xdpmm, ydpmm;
    int width, height;
    double current_width, current_height;
    double real_x_dpmm, real_y_dpmm;

    Widget w = PSwin(vw);

    /* initialize page number vars */
    vw->dvr_viewer.Ps.current_page = 0;
    vw->dvr_viewer.Ps.last_page    = 0;
    vw->dvr_viewer.Ps.waiting	   = FALSE;

    /*  set ps widget to be sized according to code copied from
     *  existing postscript previewer application
     */

    /*  need to set width/height of ps widget; if specified for cda
     *  viewer, use same; else default to letter size
     */

    if (vw->dvr_viewer.paper_width == 0)
	current_width = 215.90;
    else
	current_width = (double) vw->dvr_viewer.paper_width;

    if (vw->dvr_viewer.paper_height == 0)
	current_height = 279.40;
    else
	current_height = (double) vw->dvr_viewer.paper_height;


    /*  have to get the actual dpm (dots per mm) from the server; the dpi in
     *  the dvr struct is adjusted to match the X fonts used on a
     *  server; for ps viewing, we need to use the exact dpi to
     *  position things correctly
     */ 
    real_x_dpmm = (double) XWidthOfScreen(XDefaultScreenOfDisplay(XtDisplay(vw))) /
		  (double) XWidthMMOfScreen(XDefaultScreenOfDisplay(XtDisplay(vw)));
	          
    real_y_dpmm = (double) XHeightOfScreen(XDefaultScreenOfDisplay(XtDisplay(vw))) /
		  (double) XHeightMMOfScreen(XDefaultScreenOfDisplay(XtDisplay(vw))); 
	          
    width  = (int) (current_width   * real_x_dpmm);
    height = (int) (current_height  * real_y_dpmm);

    /* store values in our widget so we know when to change */
    vw->dvr_viewer.Ps.window_width  = width;
    vw->dvr_viewer.Ps.window_height = height;

    XtResizeWidget(w, width, height, XtBorderWidth(w));

    PSViewSetStatusProc(w, dvr_ps_status_proc, (Opaque) vw);
    PSViewSetProcs(w, dvr_ps_text_proc, dvr_ps_error_proc);

}


/*
**++
**  ROUTINE NAME:
**	dvr_view_ps_file(w, filename)
**
**  FUNCTIONAL DESCRIPTION:
**	call routines to associate file with ps viewer widget
**
**  FORMAL PARAMETERS:
**    	Widget w;
**      char   *filename;
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

void dvr_view_ps_file(vw, filename)
    DvrViewerWidget 	vw;
    char   		*filename;

{
    Widget w = PSwin(vw);

    /* stolen from OpenFileAndParseComments() */

    PSViewDisableRedisplay(w);

    /* place the scrollbars at the top and to the left */
    dvr_reset_ps_scrollbars(vw);

    PSViewSetFile(w, filename, 1, "");

    if (filename != NULL)
      {
	/* set all values for ps widget */
	double 	scale_val;
 	Arg 	ps_arg_list[5];
	int 	ps_arg_count= 0;
	double  double_width, double_height;
	int 	ps_width, ps_height;


	PSViewSetParseComments(w, vw->dvr_viewer.Ps.use_comments);
	PSViewBitmapWidths(w, vw->dvr_viewer.Ps.use_bitmaps);
        PSViewSetHeaderRequired(w, vw->dvr_viewer.Ps.header_required);

	PSViewFakeTrays(w, vw->dvr_viewer.Ps.use_trays);

	PSViewSetWindowDrawMode(w, vw->dvr_viewer.Ps.watch_progress);

	/*  the scale is stored as an integer in the widget; This
	 *  makes it so it corresponds directly to a scale widget;
	 *  need to divide by 10 before passing to ps widget
         */

	scale_val = (double) vw->dvr_viewer.Ps.scale_value/10;

	PSViewSetScale(w, scale_val);

	if (vw->dvr_viewer.Ps.scale_value != 10)
	  {
	    /* scaling in effect, scale paper size */
	    double_width = (double) vw->dvr_viewer.Ps.window_width *
			   scale_val;
	    ps_width = (int) double_width;

	    double_height = (double) vw->dvr_viewer.Ps.window_height *
			   scale_val;
	    ps_height = (int) double_height;

	  }
	else
	  {
            ps_width  = vw->dvr_viewer.Ps.window_width;
	    ps_height = vw->dvr_viewer.Ps.window_height;
       	  }

	PSViewSetOrientation(w, vw->dvr_viewer.Ps.orientation);

	if ( (vw->dvr_viewer.Ps.orientation == 90) ||
	     (vw->dvr_viewer.Ps.orientation == 270) )
	  {
	    /* flip width and height */
	    int temp;
	    temp      = ps_width;
	    ps_width  = ps_height;
	    ps_height = temp;
	  }

    	/*  the Ps window width and height get stored in our widget
    	 *  whenever they change via set values;
    	 *  for ps processing, we need to resize the ps widget whenever it's
    	 *  paper size changes
    	 */

    	if (ps_width != w->core.width)
      	  {
	    XtSetArg(ps_arg_list[ps_arg_count], XmNwidth,
			ps_width);
	    ps_arg_count++;
          }

    	if (ps_height != w->core.height)
          {
	    XtSetArg(ps_arg_list[ps_arg_count], XmNheight,
			ps_height);
	    ps_arg_count++;
          }

        if (ps_arg_count != 0)
          {
	    XtSetValues(PSwin(vw), ps_arg_list, ps_arg_count);
	    dvr_resize_ps_widget(vw);
          }
      }

    PSViewEnableRedisplay(w);

    if ( (filename != NULL) && (PSViewIsStructured(w)) )
      {
        vw->dvr_viewer.Ps.last_page = PSViewGetNumPages(w);
	if ( vw->dvr_viewer.Ps.last_page == (-1) )
	    vw->dvr_viewer.Ps.last_page = 0;
	else
	    vw->dvr_viewer.Dvr.DocRead = TRUE;
#ifdef DEBUG
	printf("\n know pages %d\n", vw->dvr_viewer.Ps.last_page);
#endif
      }
    else
        vw->dvr_viewer.Ps.last_page = 0;


}


/*
**++
**  ROUTINE NAME:
**	dvr_ps_error_proc(widget, string, numChars)
**
**  FUNCTIONAL DESCRIPTION:
**	error routine provided to ps widget; gets called with a
**	character string; pass to our error callback with a format
**	warning status;
**
**  FORMAL PARAMETERS:
**    	Widget widget;
**	char *string;
**	int numChars;
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

void dvr_ps_error_proc(widget, string, numChars)
    Widget widget;
    char *string;
    int numChars;
{
    char *str = (char *)XtMalloc(numChars + 1);
    Widget sw;
    DvrViewerWidget vw;

    /*  scroll widget is parent of ps widget;
     *  viewer widget is parent of scroll widget;
     */
    vw = dvr_find_viewer_widget(widget);
    sw = PSscroll(vw);

    strncpy(str, string, numChars);
    str[numChars] = 0;

#ifdef DEBUG
    printf("\nerror proc %s\n", str);
#endif

    dvr_error_callback(vw, 0, DVR_FORMATWARN, str, 0);

    if (vw->dvr_viewer.Ps.waiting)
      {
	/* if we've not yet reached a page; issue severe error */

	dvr_error_callback(vw, 0, DVR_FORMATERROR, 0, 0);

      }

    PSErrorAbort(PSwin(vw));

    XtFree(str);
}


/*
**++
**  ROUTINE NAME:
**	dvr_ps_text_proc(widget, string, numChars)
**
**  FUNCTIONAL DESCRIPTION:
**	text proc provided to ps widget; for us, does same as error proc
**	but does not abort
**
**  FORMAL PARAMETERS:
**    	Widget widget;
**   	char *string;
**    	int numChars;
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

void dvr_ps_text_proc(widget, string, numChars)
    Widget widget;
    char *string;
    int numChars;
{
    char *str = (char *)XtMalloc(numChars + 1);
    Widget sw;
    DvrViewerWidget vw;

    /*  scroll widget is parent of ps widget;
     *  viewer widget is parent of scroll widget;
     */
    vw = dvr_find_viewer_widget(widget);
    sw = PSscroll(vw);

    strncpy(str, string, numChars);
    str[numChars] = 0;

#ifdef DEBUG
    printf("\ntext proc %s\n", str);
#endif

    dvr_error_callback(vw, 0, DVR_FORMATWARN, str, 0);

    XtFree(str);

}


/*
**++
**  ROUTINE NAME:
**	dvr_ps_status_proc(data, status, page)
**
**  FUNCTIONAL DESCRIPTION:
**	status proc provided to ps widget; respond to status from ps
**	widget;
**
**  FORMAL PARAMETERS:
**	Opaque data;
**	StatusCode status;
**	int page;
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

static void dvr_ps_status_proc(data, status, page)
  Opaque data;
  enum StatusCode status;
  int page;
{
    DvrViewerWidget vw = (DvrViewerWidget) data;

    switch(status) {
      case starting:

	/* for starting, turn cursor to watch */

#ifdef DEBUG
	printf("\n starting ps status \n");
#endif

	/* enable abort */
	EnableAbort(vw, TRUE);

	if (!vw->dvr_viewer.Ps.waiting)
	  {
	    dvr_set_watch_cursor(vw);
	    vw->dvr_viewer.Ps.waiting= TRUE;
	  }
	break;

      case finished:

	/* for finished, reset cursor */

#ifdef DEBUG
	printf("\n finished ps status\n");
#endif

	/* disable abort */
	EnableAbort(vw, FALSE);

	if (vw->dvr_viewer.Ps.waiting)
	  {
	    dvr_reset_cursor(vw);
	    vw->dvr_viewer.Ps.waiting = FALSE;
	  }
	break;

      case badfile:

	/* issue open-fail callback */

#ifdef DEBUG
	printf("\nbad file ps status\n");
#endif

	dvr_error_callback(vw, 0, DVR_OPENFAIL, 0, 0);
	break;

      case ok:
	{
	XEvent        ev;
	unsigned long save_current_page;

	/*  ok status is issued when a page is reached;
         *  set current page number
 	 */

#ifdef DEBUG
	printf("\n ok ps status, gives a page number %d \n", page);
#endif

	save_current_page = vw->dvr_viewer.Ps.current_page;
  	vw->dvr_viewer.Ps.current_page = page;
	dvr_set_page_number(vw);
	dvr_update_page_number(vw);

	/* issue callback to calling application */
	vw->dvr_viewer.Dvr.WidgetStatus = DVR_NORMAL;

	dvr_call_callbacks(vw, DvrCRpsOK, &ev);
	if ( (vw->dvr_viewer.Ps.current_page == vw->dvr_viewer.Ps.last_page) &&
	     (vw->dvr_viewer.Ps.current_page != 0) &&
	     (vw->dvr_viewer.Ps.current_page != save_current_page) )
	    dvr_call_callbacks(vw, DvrCRendDocument, &ev);
	}

	break;

      case noPage:

	/* status issed when we know how many pages are in file;
	 * update our widget field;
	 */

#ifdef DEBUG
	printf("\nnopage ps status, now know end, page %d\n", page);
#endif

	if (page == 0)
	  {
	    /* error: file has no pages */
    	    dvr_error_callback(vw, 0, DVR_NOPAGE, 0, 0);
	    vw->dvr_viewer.Ps.current_page = 0;
#ifdef DEBUG
	    printf("\nerror file has no pages\n");
#endif

	  }
	else
	  {
	    if ( (!vw->dvr_viewer.Ps.use_comments) &&
		 (page == vw->dvr_viewer.Ps.current_page) )
	      {
	    	XEvent ev;
	    	dvr_call_callbacks(vw, DvrCRendDocument, &ev);
	      }
	  }

	vw->dvr_viewer.Ps.last_page = page;
	if ( page == (-1) )
	  vw->dvr_viewer.Ps.last_page = 0;
	else
	  vw->dvr_viewer.Dvr.DocRead  = TRUE;
	dvr_set_page_number(vw);

	break;

      case badComments:

	/* issue callback, ignore for this file */

	dvr_error_callback(vw, 0, DVR_BADCOMMENTS, 0, 0);
	PSViewSetParseComments(PSwin(vw), FALSE);

	break;

      case noHeader:

	/*  if this file has no header, and it is required, issue
	 *  callback; set widget fields, file is automatically closed.
	 */

	{
	  if (  (vw->dvr_viewer.Ps.current_page == 0) &&
		(vw->dvr_viewer.Ps.header_required) )
	    {
	      /* file is automatically closed, set bits, issue callback */
	      vw->dvr_viewer.Dvr.FileOpen = FALSE;
	      vw->dvr_viewer.Ps.current_page = 0;
	      vw->dvr_viewer.Ps.last_page = 0;
	      dvr_error_callback(vw, 0, DVR_NOPSHEAD, 0, 0);
	    }

	  break;
	}


    }
}


/*
**++
**  ROUTINE NAME:
**	dvr_set_viewer_normal(vw);
**
**  FUNCTIONAL DESCRIPTION:
**	if the ps widget is currently managed, take it down and put up the
**	normal viewer work window; This makes it so we do not have to worry
**	about having an empty ps viewer window displayed; (or filling it's
**	scrollbars.)
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

CDAstatus dvr_set_viewer_normal(vw)
    DvrViewerWidget vw;

{
    /*  if the PS window is managed, take it down and bring back normal
     *  windows
     */
    if (VIEWING_PS(vw))
      {
	if (vw->dvr_viewer.button_box_flag)
	    XtUnmanageChild(CancelBut(vw));

	XtUnmanageChild(PSscroll(vw));

	XtManageChild(Work(vw));

	if (Vscr(vw) != NULL)
	    XtManageChild(Vscr(vw));

	if (Hscr(vw) != NULL)
	    XtManageChild(Hscr(vw));

	dvr_resize(vw);

      }

    return(DVR_NORMAL);

}



/*
**++
**  ROUTINE NAME:
**	EnableAbort(vw, flag);
**
**  FUNCTIONAL DESCRIPTION:
**	set Cancel Button sensitivity to TRUE or FALSE
**
**  FORMAL PARAMETERS:
**	DvrViewerWidget vw;
**	Boolean flag;
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

void EnableAbort(DvrViewerWidget	vw,
		 Boolean		flag)

#else

void EnableAbort(vw, flag)
    DvrViewerWidget vw;
    Boolean flag;

#endif

{
    if (vw->dvr_viewer.button_box_flag)
      {
    	Arg arg_list[3];

    	XtSetArg(arg_list[0], XmNsensitive, flag);
    	XtSetValues(CancelBut(vw), arg_list, 1);
      }

}


/*
**++
**  ROUTINE NAME:
**	dvr_abort_ps_view(vw);
**
**  FUNCTIONAL DESCRIPTION:
**	call ps routine to abort processing
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

void dvr_abort_ps_view(vw)
    DvrViewerWidget vw;

{
    PSViewAbort(PSwin(vw));
}
