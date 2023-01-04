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
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /usr/sde/osf1/rcs/x11/src/motif/clients/dxdiff/scrolling.c,v 1.1.4.3 1993/09/21 21:48:17 Lynda_Rice Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
/*
 * Copyright 1988 by Digital Equipment Corporation, Maynard, Massachusetts.
 * 
 *                         All Rights Reserved
 * 
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/*
 *
 *	dxdiff
 *
 *	scrolling.c - scrolling support
 *
 *	Author:	Laurence P. G. Cable
 *
 *	Created : 19th May 1988
 *
 *
 *	Description
 *	-----------
 *
 *
 *	Modification History
 *	------------ -------
 *	
 *	1 Sep 1993	Lynda Rice
 *
 *	Refer to the history in differencebox.c.
 *
 *	10 Sep 1993	Lynda Rice
 *
 *	Fixed problem in DoHorizontalScrollingFromCallBack() that was causing
 *	X Toolkit Warnings to be generated when horizontally scrolling two
 *	lines of different lengths.  If the scrollbar of the longer line was
 *	in use and its value exceeded the maximum for the shorter line, the
 *	shorter line's value was incorrectly being truncated using the limits
 *	of the longer line.
 */

static char sccsid[] = "@(#)scrolling.c	1.4	12:14:58 7/3/88";

#include <sys/types.h>
#include <sys/stat.h>

#ifdef  DEBUG
#include <stdio.h>
#endif  DEBUG
#include <X11/Xlib.h>
#include <Xm/Xm.h>
#include <Xm/TextP.h>
#include "arglists.h"
#include "y.tab.h"
#include "filestuff.h"
#include "parsediff.h"
#include "alloc.h"
#include "differencebox.h"
#include "menu.h"
#include "text.h"
#include "display.h"


/********************************
 *
 *     GetVerticalSliderSize
 *
 ********************************/

int
GetVerticalSliderSize(differencebox)
	register DifferenceBoxPtr differencebox ;
{
	Widget	scrollbar ;
	int scroll_value, slider_size, incr, page_incr ;

	/* This function gets the scrollbar values from the toolkit and returns
	 * the slider_size, which is used as the screen row count.
	 */

	scrollbar = (Widget)TextDisplayADBPtrVScroll
		(DxDiffDisplayPtrLeftTextADB
		((DxDiffDisplayPtr)differencebox->display));

	XmScrollBarGetValues(scrollbar,
			     &scroll_value, &slider_size, &incr, &page_incr) ;

#ifdef SCROLLDEBUG
	fprintf(stderr,"GetVerticalSliderSize()\n") ;
	fprintf(stderr,"scroll_value = %d slider_size = %d incr = %d page_incr = %d\n", scroll_value, slider_size, incr, page_incr) ;
#endif

	return slider_size ;
}

/********************************
 *
 *     GetScrollDelta
 *
 ********************************/

#define	neg(a)		((a) < 0)
#define abs(a)		(!neg(a) ? (a) : -(a))

int
GetScrollDelta(db, delta, file)
	DifferenceBoxPtr	db ;
	int			delta ;
	WhichFile		file ;
{
	int	top = (file == LeftFile) ? db->toplnolf : db->toplnorf ;
	int	lasttop = GetTopOfLastScreen(db, file) ;
	int	position = top + delta ;

	/* This function range checks the in-coming scroll delta and makes
	 * adjustments to compensate for the start/end of a file as needed.
	 * The appropriate delta is then returned.
	 *
	 * DoScrollFromCallBack() now calls this function before each call to
	 * XmTextScroll() to assist with the linked scrolling feature.  This
	 * was to prevent some text "bounce-back" problems, noticed in the
	 * case of a smaller file that was at the top of its last screen and
	 * the quick dragging of the scrollbar towards the bottom of a larger
	 * file.
	 */

	if((top <= 1 && delta < 0) || (top == lasttop && delta > 0))
		return 0 ;
	else if(delta < 0 && position <= 0)
		return ((delta + abs(position)) + 1) ;
	else if(delta > 0 && position > lasttop)
		return (delta - (position - lasttop)) ;
	else
		return delta ;
}


/********************************
 *
 *     UpdateTextHighLights
 *
 ********************************/

void
UpdateTextHighLights(ltext, rtext, differencebox)
	TextDisplayPtr		ltext, rtext;
	DifferenceBoxPtr	differencebox;
{
	register EdcPtr	head = differencebox->vdtop,
			tail = differencebox->vdbottom,
			edcp;

	for (edcp = head; edcp; edcp = edcp->next) {
		if (!RightHighLighted(edcp) && edcp->highr) {
			XmTextSetHighlight(TextDisplayPtrWidget(rtext),
					    edcp->highr->left, edcp->highr->right, edcp->highr->mode);
			HighLightRight(edcp);
		}

		if (!LeftHighLighted(edcp) && edcp->highl) {
			XmTextSetHighlight(TextDisplayPtrWidget(ltext),
					    edcp->highl->left, edcp->highl->right, edcp->highl->mode);
			HighLightLeft(edcp);
		}

		if (edcp == tail) break;	/* all done */
	}
}

	

/********************************
 *
 *     DoScrollFromCallBack
 *
 ********************************/

void
DoScrollFromCallBack(w, clientd, calld)
	Widget	w;
	caddr_t	clientd;
	XmScrollBarCallbackStruct *calld;
{
	extern void			UpdateAndPaintDiffsInDifferenceBox();
	DxDiffDisplayPtr 		display = (DxDiffDisplayPtr)clientd;
	DifferenceBoxPtr 		differencebox = (DifferenceBoxPtr)
						DiffRegionADBPtrDifferenceBox(DxDiffDisplayPtrDiffRegionADB(display));
	TextDisplayPtr	 		text, othertext;
	WhichFile	 		master, slave;
	HVScrollBarPtr			scrollbar, otherscrollbar;
	XmScrollBarCallbackStruct      *cbsp =
                                        (XmScrollBarCallbackStruct *)calld;
	int				topLine, otherTopLine;
	XmTextWidget			tw, otherTw;
	int				min = 0, othermin = 0;
	int				max = 0, othermax = 0;
	int				othervalue;
	int				sliderSize = 0;
	Arg				arg[3];
	static int			position;
	static Arg scrollArgs[] = {
		{XmNvalue, (XtArgVal)&position},
	};
	int				*top, *othertop, last, otherlast;
	int				delta;

	/* This function is entered whenever the scrollbar is used.  By the
	 * time it is entered, the toolkit has scrolled the selected/master
	 * file.  If the linked vertical scrolling option is on, this function
	 * will coordinate movement of the slave screen.
	 */

#ifdef	SCROLLDEBUG
	fprintf(stderr,"DoScrollFromCallBack(calld->value = %d)\n",
		calld->value);
	ScrollDebug(differencebox) ;
#endif
	differencebox->scrolling = True ;

	if (w == (Widget)TextDisplayADBPtrVScroll(DxDiffDisplayPtrRightTextADB(display))) {
		master = RightFile;
		text = TextDisplayADBPtrTextDisplay(DxDiffDisplayPtrRightTextADB(display));
		othertext = TextDisplayADBPtrTextDisplay(DxDiffDisplayPtrLeftTextADB(display));
		scrollbar = TextDisplayADBPtrVScroll(DxDiffDisplayPtrRightTextADB(display));
		otherscrollbar = TextDisplayADBPtrVScroll(DxDiffDisplayPtrLeftTextADB(display));
		last = *(top = &(differencebox->toplnorf));
		otherlast = *(othertop = &(differencebox->toplnolf));
		tw = (XmTextWidget) TextDisplayPtrWidget(
			TextDisplayADBPtrTextDisplay(
			    DxDiffDisplayPtrRightTextADB(display)));
		delta = calld->value - differencebox->toplnorf + 1;
		differencebox->toplnorf =  topLine = calld->value + 1;
		otherTw = (XmTextWidget) TextDisplayPtrWidget(
			TextDisplayADBPtrTextDisplay(
			    DxDiffDisplayPtrLeftTextADB(display)));
		position = 0;
		XtGetValues((Widget)otherscrollbar, scrollArgs, 1);
		differencebox->toplnolf = otherTopLine = position + 1;
	} else {
		master = LeftFile;
		text = TextDisplayADBPtrTextDisplay(DxDiffDisplayPtrLeftTextADB(display));
		othertext = TextDisplayADBPtrTextDisplay(DxDiffDisplayPtrRightTextADB(display));
		scrollbar = TextDisplayADBPtrVScroll(DxDiffDisplayPtrLeftTextADB(display));
		otherscrollbar = TextDisplayADBPtrVScroll(DxDiffDisplayPtrRightTextADB(display));
		last = *(top = &(differencebox->toplnolf));
		otherlast = *(othertop = &(differencebox->toplnorf));
		tw = (XmTextWidget) TextDisplayPtrWidget(
			TextDisplayADBPtrTextDisplay(
			    DxDiffDisplayPtrLeftTextADB(display)));
		delta = calld->value - differencebox->toplnolf + 1;
		differencebox->toplnolf = topLine = calld->value + 1;
		otherTw = (XmTextWidget) TextDisplayPtrWidget(
			TextDisplayADBPtrTextDisplay(
			    DxDiffDisplayPtrRightTextADB(display)));
		position = 0;
		XtGetValues((Widget)otherscrollbar, scrollArgs, 1);
		differencebox->toplnorf = otherTopLine = position + 1;
	}

	slave = Slave(master) ;

#ifdef SCROLLDEBUG
	fprintf(stderr,"delta = %d\n", delta) ;
#endif

	/* If the linked vertical scrolling option is on, scroll the slave's
	 * screen now.
	 */

	if (differencebox->scrollmode == ScrollBoth) {
		if((delta = GetScrollDelta(differencebox, delta, slave)) != 0)
			XmTextScroll(otherTw, delta ) ;
		if (slave == RightFile)
			differencebox->toplnorf = otherTw->text.top_line + 1;
		else
			differencebox->toplnolf = otherTw->text.top_line + 1;

#ifdef SCROLLDEBUG
	fprintf(stderr,"delta = %d\n", delta) ;
#endif


	}

	ScrollDifferenceBox(differencebox, topLine, master);

	/* If the linked vertical scrolling option is currently turned-on,
	 * DifferenceBoxSlaveScrollOtherDiffs() may have attempted to
	 * center a diff in the slave's screen.  If so, more adjustments
	 * will be made here.
	 */

	_XmTextDisableRedisplay(tw, False);
	if (differencebox->scrollmode == ScrollBoth)
		_XmTextDisableRedisplay(otherTw, False);

	if (differencebox->scrollmode == ScrollBoth) {
		if (slave == RightFile) {
		    delta = differencebox->toplnorf - differencebox->ptoplnorf;
		    if((delta = GetScrollDelta(differencebox, delta, slave))
				!= 0)
			XmTextScroll(otherTw, delta);
		    differencebox->toplnorf = otherTw->text.top_line + 1;

		    delta = differencebox->toplnolf - differencebox->ptoplnolf;
		    if((delta = GetScrollDelta(differencebox, delta, master))
				!= 0)
			XmTextScroll(tw, delta);
		    differencebox->toplnolf = tw->text.top_line + 1 ;
		} else {
		    delta = differencebox->toplnolf - differencebox->ptoplnolf;
		    if((delta = GetScrollDelta(differencebox, delta, slave))
				!= 0)
			XmTextScroll(otherTw, delta);
		    differencebox->toplnolf = otherTw->text.top_line + 1;

		    delta = differencebox->toplnorf - differencebox->ptoplnorf;
		    if((delta = GetScrollDelta(differencebox, delta, master))
				!= 0)
			XmTextScroll(tw, delta);
		    differencebox->toplnorf = tw->text.top_line + 1 ;
		}

#ifdef SCROLLDEBUG
	fprintf(stderr,"delta = %d\n", delta) ;
#endif

	}


	UpdateTextHighLights(TextDisplayADBPtrTextDisplay(DxDiffDisplayPtrLeftTextADB(display)),
			     TextDisplayADBPtrTextDisplay(DxDiffDisplayPtrRightTextADB(display)),
			     differencebox);

	_XmTextEnableRedisplay(tw);
	if (differencebox->scrollmode == ScrollBoth) {
	    _XmTextEnableRedisplay(otherTw);
	}

	/*
	 * This code was added to move-up the difference box for a file that
	 * ends exactly on the last row of a screen.
	 */
	if(TopOfLastScreen(differencebox,LeftFile) &&
	   TopOfLastScreen(differencebox,RightFile))
		UpdateAndPaintDiffsInDifferenceBox(differencebox) ;

	differencebox->scrolling = False ;

#ifdef SCROLLDEBUG
	ScrollDebug(differencebox) ;
#endif

}


/********************************
 *
 *	DoHorizontalScrollingFromCallBack
 *
 ********************************/


DoHorizontalScrollingFromCallBack(w, clientd, calld)
	Widget	w;
	caddr_t	clientd,
		calld;
{
	DxDiffDisplayPtr 		display = (DxDiffDisplayPtr)clientd;
	HVScrollBarPtr			scrollbar, otherscrollbar;
	XmScrollBarCallbackStruct      *cbsp =
                                        (XmScrollBarCallbackStruct *)calld;
	int				min = 0, othermin = 0;
	int				max = 0, othermax = 0;
	int				othervalue;
	int				sliderSize = 0;
	Arg				arg[3];
	float				percent;

	if (w == (Widget)TextDisplayADBPtrHScroll(DxDiffDisplayPtrRightTextADB(display))) {
		scrollbar = TextDisplayADBPtrHScroll(DxDiffDisplayPtrRightTextADB(display));
		otherscrollbar = TextDisplayADBPtrHScroll(DxDiffDisplayPtrLeftTextADB(display));
		DxDiffDisplayPtrHorizontalScrolledLast(display) = RightFile;
	} else {
		scrollbar = TextDisplayADBPtrHScroll(DxDiffDisplayPtrLeftTextADB(display));
		otherscrollbar = TextDisplayADBPtrHScroll(DxDiffDisplayPtrRightTextADB(display));
		DxDiffDisplayPtrHorizontalScrolledLast(display) = LeftFile;
	}

	/* do slave scrolling */

	if (DxDiffDisplayPtrHorizontalSlaveScroll(display)) {
		XtSetArg(arg[0], XmNminimum, &min);
		XtSetArg(arg[1], XmNmaximum, &max);
		XtGetValues((Widget)scrollbar, arg, 2);

		XtSetArg(arg[0], XmNminimum, &othermin);
		XtSetArg(arg[1], XmNmaximum, &othermax);
		XtSetArg(arg[2], XmNsliderSize, &sliderSize);
		XtGetValues((Widget)otherscrollbar, arg, 3);

		percent = cbsp->value;
		percent = percent / (max - min);
		othervalue = (percent * (othermax - othermin));

		/*
		 * Check the other scrollbar value against it's limits and
		 * truncate it if necessary to prevent X Toolkit Warnings.
		 */
		if (othervalue >= (othermax - sliderSize))
		    othervalue = othermax - sliderSize;
		XmScrollBarSetValues(otherscrollbar, othervalue, sliderSize,
				     0, 0, True);
	}
}

#ifdef	SCROLLDEBUG
ScrollDebug(differencebox)
DifferenceBoxPtr	differencebox ;
{
	fprintf(stderr,"toplnolf = %d, ptoplnolf = %d\n", 
		differencebox->toplnolf, differencebox->ptoplnolf) ;
	fprintf(stderr,"toplnorf = %d, ptoplnorf = %d\n",
		differencebox->toplnorf, differencebox->ptoplnorf) ;
}
#endif
