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
static char *BuildSystemHeader= "$Id: differencebox.c,v 1.1.4.3 1993/09/21 21:48:07 Lynda_Rice Exp $";
#endif		/* BuildSystemHeader */
/*
 * Copyright 1988 by Digital Equipment Corporation, Maynard, Massachusetts.
 * 
 *                         All Rights Reserved
 * 
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
 *	differencebox.c - utility routines for difference box
 *
 *	Author:	Laurence P. G. Cable
 *
 *	Created : 24th March 1988
 *
 *
 *	Description
 *	-----------
 *
 *	Modification History
 *	------------ -------
 *	
 *
 *	14th April 1988		Laurence P. G. Cable
 *
 *	changed name of NumberSequence structure element from 'howmany'
 *	to 'oneortwo' to remove macro name clash with V2.4 sys/types.h
 *	macro.
 *
 *	16th April 1988		Laurence P. G. Cable
 *
 *	I have done a number of things :-
 *
 *		1) fixed DifferenceBoxExposeCallback to paint when getting
 *		   only one event ... (the sense of the return condition
 *		   was wrong!)
 *
 *		2) fixed the x co-ords for wide lines 
 *
 *
 *	16th April 1988		Laurence P. G. Cable
 *
 *	Added the line number display code to the difference box!
 *
 *	
 *	17th April 1988		Laurence P. G. Cable
 *
 *	Changed the pathetic resize handler code and modified 
 *	DifferenceBoxCreateXPointList to stop recursing infinitely
 *	which it was deliberately coding not to do in the first place!
 *	decided to change DifferenceBoxExposeHandler to call 
 *	UpdateAndPaintDiffsInDifferenceBox ...... 
 *
 *	22nd April 1988	Laurence P. G. Cable
 *
 *	Added 'Dragging' optimisation code and change line number painting
 *	so that its prettier on colour displays!
 *
 *	6th May 1988	Laurence P. G. Cable
 *
 *	changed interface to come into line with dxdiff
 *
 *	19th June 1988	Laurence P. G. Cable
 *
 *	Changed the drawing stuff to draw delete's and append's
 *	as triangles ....
 *
 *	14th July 1988	Laurence P. G. Cable
 *
 *	Fixed bug in Slave scrolling code where a pervious fix caused the 
 *	slave scroll to exit if no diff was there .... it now scrolls
 *	properly
 *
 *	16 Jan 1990	Colin Prosser
 *
 *	Move code to set differencebox->painting if diffregion is already
 *	mapped from CreateDifferenceBox() to LoadNewDifferenceBox().
 *	Initialize differencebox->painting to False in CreateDifferenceBox().
 *	Fixes UWS QAR 01568 where line numbers weren't drawn on initial startup
 *	when two files were specified on the command line or on initial display
 *	following "do differences in new".  Also fixes related UWS QAR 01558
 *	where the +/-ln option appeared not to function.
 *
 *	In LoadNewDifferenceBox() save current value of scrollmode before
 *	temporary change to ScrollOne while updating display of diffs and
 *	restore afterwards.  Fixes UWS QAR 01556 where -sv option didn't work.
 *
 *	1 Sep 1993	Lynda Rice
 *
 *	Made many changes to the scrolling and skipping (Next/Prev Diff) code.
 *	In general, corrected problems scrolling to the bottom of a file;
 *	scrollbar bouncing problems at the bottom of a file; screen boundary
 *	problems where a diff on the first line of the next screen would be
 *	considered visible on the bottom of the current screen; and many 
 *	unusual behaviours observed while testing files containing a single
 *	diff of various sizes and placements within files, which resulted
 *	in things like a diff being centered on one screen and at the bottom
 *	of the other, or "bounce-back" of text in a smaller file that was at
 *	the top of its last screen caused by quickly dragging the scrollbar
 *	towards the bottom of the larger file; not properly locating the
 *	next diff beyond the first screen when the first screen contains
 *	diffs, resulting in the diff being centered on one screen and dropped
 *	down on the next; etc., etc., etc..  A lot of the unusual behaviours
 *	were a result of code which attempts to center diffs at the center of
 *	the screens.  In order to anaylze the different behaviours, a lot of
 *	SCROLLDEBUG code was added.  The pointer to the DxDiffDisplay structure
 *	is now maintained in the differencebox structure in order to locate
 *	a scrollbar widget, now used to determine the number of rows on a
 *	screen.  And finally, some attempts to clean-up the code and add
 *	comments have been made, which are far too minimal I am sure.
 *
 *	10 Sep 1993	Lynda Rice
 *
 *	Fixed eof/eol handling in the Next/Prev Diff skipping code I added,
 *	which caused problems with the Prev Diff button.  Fixed DiffIsVisible()
 *	to really determine the visibility of a diff.  Also, if not scrolling,
 *	the nearest diff for the master file has already been located by the
 *	time ScrollDifferenceBox() is called, so bypassed the call in this
 *	function.
 */

static char sccsid[] = "@(#)differencebox.c	1.28	19:03:20 1/16/90";

#include <sys/types.h>
#include <sys/stat.h>

#ifdef	DEBUG
#include <stdio.h>
#endif	DEBUG
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/DrawingAP.h>

#include "y.tab.h"
#include "filestuff.h"
#include "arglists.h"
#include "parsediff.h"
#include "alloc.h"
#include "dxdiff.h"
#include "differencebox.h"
#include "menu.h"
#include "text.h"
#include "display.h"

extern XDrawImageString();
extern XDrawString();

/********************** Private Routines ************************/


/********************************
 *
 *     ResizeDifferenceBox 
 *
 ********************************/

Boolean
ResizeDifferenceBox(differencebox)
	register DifferenceBoxPtr differencebox;
{
	Arg	args;

	args.name = XmNheight;
	args.value = (XtArgVal)&CoreHeight(differencebox->core);
	XtGetValues(differencebox->window, &args, 1);

	if (differencebox->height != CoreHeight(differencebox->core)) {
		XtWidgetGeometry req,rep;

		
		req.height = differencebox->height;
		req.request_mode = CWHeight;
		XtMakeGeometryRequest(differencebox->window, &req, &rep);
		return True;
	}

	return False;
}



/********************************
 *
 *     CheckDifferenceBoxWindowDimensions 
 *
 ********************************/

static Boolean
CheckDifferenceBoxWindowDimensions(differencebox)
	register DifferenceBoxPtr differencebox;
{
	Arg		widthandheight[2];

	widthandheight[0].name  = XmNwidth;
	widthandheight[0].value = (XtArgVal)&CoreWidth(differencebox->core);
	widthandheight[1].name  = XmNheight;
	widthandheight[1].value = (XtArgVal)&CoreHeight(differencebox->core);

	
	XtGetValues(differencebox->window, widthandheight,
		    sizeof widthandheight / sizeof widthandheight[0]);

	if (CoreHeight(differencebox->core) != differencebox->height ||
	    CoreWidth(differencebox->core) != differencebox->width) {
		differencebox->height = CoreHeight(differencebox->core);
		differencebox->width = CoreWidth(differencebox->core);
		return True;
	} else
		return False;
}




/********************************
 *
 *      DifferenceBoxResizeHandler
 *
 ********************************/

static void
DifferenceBoxResizeHandler(differencebox) /* this is a misnomer */
	register DifferenceBoxPtr differencebox;
{	
	int oldrows = differencebox->rows;
	int t;

	/* The call to GetVerticalSliderSize() has replaced the previous
	 * calculation of "abs((height - 2 * vmargins) / fontheight)",
	 * because the resulting number of rows was sometimes off by one.
	 */
	differencebox->rows = GetVerticalSliderSize(differencebox) ;

	if (differencebox->resizenotify != (void (*)())NULL) {
		if (differencebox->prevdb != (DifferenceBoxPtr)NULL) {
			XtFree((char *)differencebox->prevdb);
		}

		if ((differencebox->prevdb = (DifferenceBoxPtr)XtMalloc(sizeof (DifferenceBox))) == (DifferenceBoxPtr)NULL) {
			return;	/* error */
		}

		bcopy((char *)differencebox, (char *)differencebox->prevdb, sizeof (DifferenceBox));

		differencebox->prevdb->rows = oldrows;

		differencebox->resizenotifypending = True;
	}

	if (((t = differencebox->rflen - oldrows) >= 0 && differencebox->toplnorf >= t) ||
	    ((t = differencebox->lflen - oldrows) >= 0 && differencebox->toplnolf >= t)) {
		int delta = oldrows - differencebox->rows;

		differencebox->ptoplnorf = differencebox->toplnorf;
		differencebox->ptoplnolf = differencebox->toplnolf;

		differencebox->toplnorf = differencebox->toplnorf + delta <= 0 ? 1 : differencebox->toplnorf + delta;
		differencebox->toplnolf = differencebox->toplnolf + delta <= 0 ? 1 : differencebox->toplnolf + delta;

	}
}



/********************************
 *
 *      DifferenceBoxMapEventHandler
 *
 ********************************/

static void
DifferenceBoxMapEventHandler(w, clientd, event)
	register Widget	w;
	caddr_t		clientd;
	register XEvent	*event;
{
	register DifferenceBoxPtr differencebox = (DifferenceBoxPtr)clientd;
	extern	 Widget		  XtWindowToWidget();

	if (XtWindowToWidget(event->xany.display, event->xany.window) != differencebox->window)
		return;

	if (event->type == MapNotify) {
		differencebox->painting = True;
	} else {
		if (event->type == UnmapNotify) {
			differencebox->painting = False;
		}
	}
}

/********************************
 *
 *      DifferenceBoxExposeCallback
 *
 ********************************/

static void
DifferenceBoxExposeCallback(w, clientd, calld)
	Widget	w;
	caddr_t	clientd;
	caddr_t	calld;
{
	extern	 void		  DifferenceBoxPaintLineNumbers();
	extern	 void		  DifferenceBoxDrawDifferences();
	extern	 void		  UpdateAndPaintDiffsInDifferenceBox();
	register DifferenceBoxPtr differencebox = (DifferenceBoxPtr)clientd;
	register XExposeEvent	  *xevent; 

	xevent = (XExposeEvent *)((XmDrawingAreaCallbackStruct *)calld)->event;

	if (xevent->count)
		return;

	if (CheckDifferenceBoxWindowDimensions(differencebox)) {
		DifferenceBoxResizeHandler(differencebox);
		UpdateAndPaintDiffsInDifferenceBox(differencebox); /* changed */

		if (differencebox->resizenotifypending) {
			differencebox->resizenotifypending = False;
			(*differencebox->resizenotify)(differencebox, differencebox->prevdb,
						      differencebox->clientdata);

			XtFree((char *)differencebox->prevdb);
			differencebox->prevdb = (DifferenceBoxPtr)NULL;
		}
	} else {	/* just exposed */
		DifferenceBoxDrawDifferences(differencebox);
		DifferenceBoxPaintLineNumbers(differencebox);
	}

	XFlush(XtDisplay(differencebox->window));
}

static XtCallbackRec exposecallbacklist[] = {
			{ (VoidProc) DifferenceBoxExposeCallback, 0 },
			{ (VoidProc) NULL, NULL }
		   };


/********************************
 *
 *      DifferenceBoxDestroyCallback
 *
 ********************************/

static void
DifferenceBoxDestroyCallback(w, clientd, calld)
	Widget	w;
	caddr_t	clientd;
	caddr_t	calld;
{
	register DifferenceBoxPtr differencebox = (DifferenceBoxPtr)clientd;
	register Display	  *d = XtDisplay(differencebox->window);


	XtRemoveEventHandler(differencebox->window, 
		StructureNotifyMask | SubstructureNotifyMask, False, 
		(XtEventHandler)DifferenceBoxMapEventHandler, 
		differencebox);

	if (differencebox->npoints != 0)
		XtFree((char *)differencebox->points);

	XtDestroyGC(differencebox->gc);
	if (differencebox->draggc != differencebox->gc)
		XtDestroyGC(differencebox->draggc);

	if (differencebox->lnos != (LineNumberPtr)NULL)
		XtFree((char *)differencebox->lnos);

	if (differencebox->lnfgc != (GC)NULL)
		XtDestroyGC(differencebox->lnfgc);

	if (differencebox->prevdb != (DifferenceBoxPtr)NULL)
		XtFree((char *)differencebox->prevdb);

	XtFree((char *)differencebox);
}

static XtCallbackRec destroycallbacklist[] = {
	{ (VoidProc)DifferenceBoxDestroyCallback, 0 },
	{ (VoidProc)NULL, 0 }
};


/********************************
 *
 *      DifferenceBoxDrawDifferences
 *
 ********************************/

static void
DifferenceBoxDrawDifferences(differencebox)
	DifferenceBoxPtr differencebox;
{
	register int		npointlist = differencebox->dllen;
	register PointListPtr	points = differencebox->points;
	register GC		gc = (!differencebox->dragging) ?
						differencebox->gc :
						differencebox->draggc;

	register Window		w = XtWindow(differencebox->window);
	register Display	*d = XtDisplay(differencebox->window);

	if (!differencebox->painting)
		return;

	XClearWindow(d, w); /* probably cause flicker ? */

	if (differencebox->drawdiffsas == DrawDiffsAsLines ||
	    differencebox->dragging) {
		while(npointlist--) {
			XDrawLines(d, w, gc, points->points, points->numpoints, CoordModeOrigin);
			points++;
		}
	} else {	/* DrawDiffsAsFilledPolygons */
		while(npointlist--) {
			XFillPolygon(d, w, gc, points->points, points->numpoints, Nonconvex, CoordModeOrigin);
			points++;
		}
	}
}




/********************************
 *
 *      DifferenceBoxFindNearestDiffToLine
 *
 ********************************/


#define Numseq(ptr,offset)	*((NumberSequencePtr *)((long)(ptr) + offset))

#define	neg(a)		((a) < 0)
#define abs(a)		(!neg(a) ? (a) : -(a))
#define	min(a, b)	((a) < (b) ? (a) : (b))
#define	amin(a, b)	(abs(a) < abs(b) ? (a) : (b))
#define aamin(a, b)	min(abs(a),abs(b))


static	EdcPtr
DifferenceBoxFindNearestDiffToLine(differencebox, line, file)
	DifferenceBoxPtr differencebox;
	register int	 line;
	WhichFile	 file;
{
	register EdcPtr			p,
					limit,
					last;

	register int			nsoffset,
					edcpoffset;

	register NumberSequencePtr 	nsp;

	EdcPtr				viewprt;

	int				headd,
					taild,
					viewprtd;

	register int			lastd,
					currd;

	if (differencebox->head == (EdcPtr)NULL ||
	    differencebox->tail == (EdcPtr)NULL)
		return (EdcPtr)NULL;

	if (file == LeftFile) {
		viewprt = differencebox->viewprtl;
		nsoffset = XtOffset(EdcPtr, ns1);
	} else { /* RightFile */
		viewprt = differencebox->viewprtr;
		nsoffset = XtOffset(EdcPtr, ns2);
	}

	nsp = Numseq(differencebox->head, nsoffset);
	headd = line - nsp->numbers[0];
	if (nsp->oneortwo == (unsigned char)2)
		headd = amin(headd, line - nsp->numbers[1]);
	if (neg(headd))
		return differencebox->head;

	nsp = Numseq(differencebox->tail, nsoffset);
	taild = line - nsp->numbers[0];
	if (nsp->oneortwo == (unsigned char)2)
		taild = amin(taild, line - nsp->numbers[1]);
	if (!neg(taild))
		return differencebox->tail;

	if (viewprt == (EdcPtr)NULL) {
		if (abs(headd) < abs(taild)) {
			p = differencebox->head;
			limit = differencebox->tail;
			edcpoffset = XtOffset(EdcPtr, next);
			lastd = headd;
		} else { 
			p = differencebox->tail;
			limit = differencebox->head;
			edcpoffset = XtOffset(EdcPtr,prev);
			lastd = taild;
		}
	} else {
		nsp = Numseq(viewprt, nsoffset);
		viewprtd = line - nsp->numbers[0];

		if (nsp->oneortwo == (unsigned char)2)
			viewprtd = amin(viewprtd, nsp->numbers[1]);

		if (neg(viewprtd)) {
			if (abs(viewprtd) < abs(headd)) {
				p = viewprt;
				limit = differencebox->head;
				edcpoffset = XtOffset(EdcPtr, prev);
				lastd = viewprtd;
			} else {
				p = differencebox->head;
				limit = viewprt;
				edcpoffset = XtOffset(EdcPtr, next);
				lastd = headd;
			}
		} else {
			if (abs(viewprtd) < abs(taild)) {
				p = viewprt;
				limit = differencebox->tail;
				edcpoffset = XtOffset(EdcPtr, next);
				lastd = viewprtd;
			} else {
				p = differencebox->tail;
				limit = viewprt;
				edcpoffset = XtOffset(EdcPtr, prev);
				lastd = taild;
			}
		}
	}

	for (last = p; p && p != limit; last = p, lastd = currd, 
					p = *(EdcPtr *)((long)p + edcpoffset)) {
		nsp = Numseq(p, nsoffset);
		currd = line - nsp->numbers[0];
		if (nsp->oneortwo == (unsigned char)2)
			currd = amin( currd, line - nsp->numbers[1]);

		if (abs(lastd) < abs(currd))
			return last;
	}

	return (abs(lastd) <= abs(currd) ? last : p); /* was the one before the limt o.k ?? */
}

/********************************
 *
 *      DifferenceBoxEstablishVisibleDiffs
 *
 ********************************/

	/* The BelowDifferenceBox macro returns true if the last lines of the
	 * specified diff are less than the top lines of the current screen
	 * for both the left and right files.
	 */

#define	BelowDifferenceBox(p,db)				\
		(((p)->ns1->numbers[1] < (db)->toplnolf) &&	\
		 ((p)->ns2->numbers[1] < (db)->toplnorf))

	/* The AboveDifferenceBox macro returns true if the first lines of the
	 * specified diff are greater than the bottom lines of the current
	 * screen for both the left and right files.
	 *
	 * The calculation of the last row on the screen has been corrected
	 * to prevent a diff on the top line of the next screen from being
	 * considered visible on the current screen.
	 */
#define	AboveDifferenceBox(p,db)					 \
		(((p)->ns1->numbers[0] > ((db)->toplnolf + (db)->rows - 1)) && \
		 ((p)->ns2->numbers[0] > ((db)->toplnorf + (db)->rows - 1)))

static void
DifferenceBoxEstablishVisibleDiffs(differencebox)
	register DifferenceBoxPtr differencebox;
{
	register EdcPtr	p,
			q,
			vpl = differencebox->viewprtl,
			vpr = differencebox->viewprtr,
			head = differencebox->head,
			tail = differencebox->tail;
	EdcPtr		top, bottom;
	register int	c = 0,
			t;

	differencebox->dllen = 0; /* assume the worst ! */

	if (head == (EdcPtr)NULL) {
		top = bottom = (EdcPtr)NULL;
		goto wayout;
	}

	/* 1st - find out which diff is nearest the start of the list ! */

	if (vpl == vpr) {
		top = bottom = vpl;
	} else {
		p = vpl;
		q = vpr;

		while ((p != head && q != head) && !(p == vpr || q == vpl)) {
			p = p->prev;
			q = q->prev;
		}

		if (p == head || q == vpl) {
			top = vpl;
			bottom = vpr;
		} else {
			top = vpr;
			bottom = vpl;
		}
	}

	/* the hard way !! - never heard of sequence numbering lists ?? */


	/* now run up and down the chain to determine visible differences */

	/* find the 'bottom' visible diff */

	for (p = bottom;
	     (c = AboveDifferenceBox(p, differencebox)) && p != head;
	     p = p->prev)
	;

	if (p == head && c) {	/* differences not visible in viewport */
		top = bottom = (EdcPtr)NULL;
		goto wayout;		
	} else { /* !c ==> not above viewport, who cares about p == head */
		for (q = p;
		     !AboveDifferenceBox(p, differencebox) && p != tail;
		     q = p, p = p->next)
		;

		if ((t = p == tail) && BelowDifferenceBox(p, differencebox)) {
			top = bottom = (EdcPtr)NULL;
			goto wayout;		
		} else 
			bottom = (t) ? tail : q;
	}

	/* now find the 'top' one !!! */

	for (p = top;
	     BelowDifferenceBox(p, differencebox) && p != tail;
	     p = p->next)
	;
	
	if (p == tail && !AboveDifferenceBox(p, differencebox)) {
		top = tail;
	} else { /* !c ==> not below viewport, who cares about p == tail */
		for (q = p;
		     !BelowDifferenceBox(p, differencebox) && p != head;
		     q = p, p = p->prev)
		;

		top = (p == head) ? head : q;
	}
	     
	/* find out how many there are on the list */

	for ( p = top, q = bottom->next, c = 0;
	      p != q;
	      c++, p = p->next)
	;

	differencebox->dllen = c;

wayout:	/* well you have to have one dont you !!! */

	differencebox->vdtop = top;
	differencebox->vdbottom = bottom;
}





/********************************
 *
 *	DifferenceBoxCreateLineNumberList
 *
 ********************************/

static int
cvtitoa(i, s)		/* convert int to str and return length */
	register int  i;
	register char *s;
{
	char          b[9];
	register char *p,*t;
	int	      len;

	for (p = b + sizeof (b) - 1; i > 0;
		*p-- = "0123456789"[i % 10], i /= 10)
	;
	
	len = (t = b + sizeof(b)) - ++p;

	while (p < t)
		*s++ = *p++;

	return len;
}

static void
DifferenceBoxCreateLineNumberList(differencebox)
	DifferenceBoxPtr differencebox;
{
	PointListPtr		pointlist = differencebox->points;
	register XPoint		*points;
	register int		npoints = differencebox->npoints;
	register EdcPtr		edcptr = differencebox->vdtop;
	register LineNumberPtr 	lnptr;
	register XFontStruct	*fsptr = differencebox->font;

	if (!(differencebox->painting && !differencebox->dragging &&
	      differencebox->linenumbers) || differencebox->dllen == 0)
		return;	/* not displaying or no differences */


	if (differencebox->lnos != (LineNumberPtr)NULL)
		XtFree((char *)differencebox->lnos);

	differencebox->nstrs = 0;

	differencebox->lnos = lnptr = 
		(LineNumberPtr)XtMalloc(differencebox->dllen * 4 * sizeof(LineNumber)); /* max required */

	if (lnptr == (LineNumberPtr)NULL)
		return;	/* give up and go home! */
	
	for (points = pointlist->points; 
	     npoints;
	     (npoints -= pointlist->numpoints), pointlist++, (points = pointlist->points), (edcptr = edcptr->next)) {
		XCharStruct	bbox;
		int		dummy;

		differencebox->nstrs += (edcptr->et == EChange) ? 4 : 3;

		/* top left */

		if (edcptr->et != EAppend) {
			lnptr->x = points[0].x;
			lnptr->len = cvtitoa(edcptr->ns1->numbers[0],
					     lnptr->number);
			XTextExtents(fsptr, lnptr->number, lnptr->len, &dummy, &dummy,
				     &dummy, &bbox);
			(lnptr++)->y = points[0].y + (bbox.ascent + bbox.descent) / 2;
		}

		/* bottom left */

		{
			int idx = (edcptr->et == EChange || edcptr->et == EAppend) ? 3 : 2;

			lnptr->x = points[idx].x;
			lnptr->len = cvtitoa(edcptr->ns1->numbers[1] + 1,
					     lnptr->number);
			XTextExtents(fsptr, lnptr->number, lnptr->len, &dummy, &dummy,
				     &dummy, &bbox);
			(lnptr++)->y = points[idx].y + (bbox.ascent + bbox.descent) / 2;
		}

		/* top right */

		lnptr->len = cvtitoa(edcptr->ns2->numbers[0],
				     lnptr->number);
		XTextExtents(fsptr, lnptr->number, lnptr->len, &dummy, &dummy,
			     &dummy, &bbox);
		lnptr->x = points[1].x - (bbox.lbearing + bbox.rbearing);
		(lnptr++)->y = points[1].y + (bbox.ascent + bbox.descent) / 2;

		/* bottom right */

		if (edcptr->et != EDelete) {
			lnptr->len = cvtitoa(edcptr->ns2->numbers[1] + 1,
					     lnptr->number);
			XTextExtents(fsptr, lnptr->number, lnptr->len, &dummy, &dummy, 
				     &dummy, &bbox);
			lnptr->x = points[2].x - (bbox.lbearing + bbox.rbearing);
			(lnptr++)->y = points[2].y + (bbox.ascent + bbox.descent) / 2;
		}
	}
}


/********************************
 *
 *      DifferenceBoxPaintLineNumbers
 *
 ********************************/

static void
DifferenceBoxPaintLineNumbers(differencebox)
	DifferenceBoxPtr differencebox;
{
	register int		nstr = differencebox->nstrs;
	register LineNumberPtr	lnptr = differencebox->lnos;
	register GC		gc = differencebox->lnfgc;
	register Display	*d = XtDisplay(differencebox->window);
	register Window		w = XtWindow(differencebox->window);
	register void		(*drawstring)() = differencebox->drawstring;

	if (!(differencebox->painting && !differencebox->dragging &&
	      differencebox->linenumbers) || differencebox->dllen == 0)
		return;

	while (nstr--) {
		(*drawstring)(d, w, gc, lnptr->x, lnptr->y, lnptr->number,
		              lnptr->len);
		lnptr++;
	}
}




/********************************
 *
 *      DifferenceBoxCreateXPointList
 *
 ********************************/

static void
DifferenceBoxCreateXPointList(differencebox)
	DifferenceBoxPtr differencebox;
{
	register XPoint *points;
	register int    npointlist = differencebox->dllen; /* assume the maximum possible */
	PointListPtr	pointlist;
	register EdcPtr p;
	register int	width = differencebox->width,
			vmargins,
			lvt,
			rvt,
			rows,
			font,
			lw;


	/* assume that the geometry of the differencebox is up to date */
	/* because - we have called a CheckDifferenceBoxWindowGeometry */
	/*           and a DifferenceBoxResizeHandler  !!! 	       */

	vmargins = differencebox->vmargins;
	lvt = differencebox->toplnolf;
	rvt = differencebox->toplnorf;
	rows = differencebox->rows;
	font = differencebox->fontheight;
	lw = differencebox->lw;

	differencebox->npoints = 0;

		
	if (differencebox->points != (PointListPtr)NULL) {
		XtFree((char *)differencebox->points);
		differencebox->points = (PointListPtr)NULL;
	}

	if (differencebox->vdtop == (EdcPtr)NULL) {
		return;
	}

	differencebox->points = pointlist = (PointListPtr)XtCalloc(npointlist, sizeof (PointList));


	if (pointlist == (PointListPtr)NULL) { /* error */
		return;			/* for the moment */
	}

	for ((p = differencebox->vdtop), (points = pointlist->points);
	     npointlist;
	     (p = p->next), pointlist++, npointlist--, (points = pointlist->points)) {
		register int t;
		Boolean	     hasfivepoints,point2xleft = False;

		if (hasfivepoints = (p->et == EChange)) {
			differencebox->npoints += (pointlist->numpoints = 5);
		} else {
			differencebox->npoints += (pointlist->numpoints = 4);
		}
		
		points[0].x = points[3].x = (short)(lw / 2 + 1);
		points[1].x = (short)(width - lw / 2 - 1);
		if (p->et != EAppend)
			points[0].y = (short)((p->ns1->numbers[0] - lvt) * font);
		if (p->et != EDelete) {
			points[2].x = points[1].x;
			points[1].y = (short)((p->ns2->numbers[0] - rvt) * font);
			points[2].y = (short)((p->ns2->numbers[1] - rvt + 1) * font);
		}

		switch	(p->et)	{
			case EChange:
				points[4].x = points[0].x;
				points[4].y = points[0].y;
				points[3].y = (short)((p->ns1->numbers[1] - lvt + 1) * font);
				break;

			case EAppend:	/* arrow right to left */
				points[3].y = points[0].y = (short)((p->ns1->numbers[0] - lvt + 1) * font);
				break;

			case EDelete:	/* arrow left to right */
				point2xleft = True;
				points[2].x = points[0].x;
				points[1].y = (short)((p->ns2->numbers[1] - rvt + 1) * font);
				points[2].y = (short)((p->ns1->numbers[1] - lvt + 1) * font);
				points[3].y = points[0].y;
				break;
		}

		/* now adjust for vertical margins !!! */

		if (!vmargins) continue;

		if (p->ns1->numbers[0] >= lvt) {
			points[0].y += vmargins;
			points[3].y += vmargins;
			if (hasfivepoints)
				points[4].y += vmargins;
			else
				if (point2xleft)
					points[2].y += vmargins;

			if (p->ns1->numbers[0] > lvt + rows) {
				points[0].y += vmargins;
				points[3].y += vmargins;
				if (hasfivepoints)
					points[4].y += vmargins;
				else
					if (point2xleft)
						points[2].y += vmargins;
			} else
				if (p->ns1->numbers[1] > lvt + rows)
					if (hasfivepoints || !point2xleft)
						points[3].y += vmargins;
					else
						points[2].y += vmargins;
		}
 
		if (p->ns2->numbers[0] >= rvt) {
			points[1].y += vmargins;
			if (!point2xleft)
				points[2].y += vmargins;

			if (p->ns2->numbers[0] > rvt + rows) {
				points[1].y += vmargins;
				if (!point2xleft)
					points[2].y += vmargins;
			} else
				if (!point2xleft && p->ns2->numbers[1] > rvt + rows)
					points[2].y += vmargins;
		}
	}
}

	
/********************************
 *
 *      GetTopOfLastScreen
 *
 ********************************/

int
GetTopOfLastScreen(differencebox, file)
        DifferenceBoxPtr	differencebox;
        WhichFile		file;
{
	int	lasttopLine;

	/* This function returns the top line of the last screen for the
	 * specified file.  dxdiff uses the top line of a screen as a basis
	 * for all scrolling/skipping operations; end-of-file checks are
	 * based on the top line of the last screen.
	 *
	 * This function replaces in-line computations of "flen - rows + 1".
	 * The addition of "+ 2" compensates for a blank row at the bottom of
	 * the screen.
	 */

	if (file == LeftFile)
		lasttopLine = differencebox->lflen - differencebox->rows + 2 ;
	else
		lasttopLine = differencebox->rflen - differencebox->rows + 2 ;

	if(lasttopLine < 1) lasttopLine = 1 ;

	return lasttopLine ;
}


/********************************
 *
 *      SetTopLine
 *
 ********************************/

int
SetTopLine(differencebox, topLine, file)
        DifferenceBoxPtr	differencebox;
	int			topLine;
        WhichFile		file;
{
	int	lasttopLine;

	/* This function helped replace some pretty nasty looking in-line code.
	 * The in-coming topLine argument is range checked for the start/end
	 * of file, and adjusted if necessary.  This value is then saved in
	 * the differencebox structure.  If an end-of-file adjustment was made,
	 * the top line of the last screen is returned; otherwise, a zero is
	 * returned.  Also, the previous top (ptop) value in the differencebox
	 * structure is set to either the current top value upon entry or
	 * the top line of the last screen if an adjustment was made.  dxdiff
	 * uses the top line of a screen as a basis for all scrolling/skipping
	 * operations; end-of-file checks are based on the top line of the last
	 * screen.
	 */

	if (topLine < 1) topLine = 1;
	
	if (file == LeftFile) {
		differencebox->ptoplnolf = differencebox->toplnolf;
		lasttopLine = GetTopOfLastScreen(differencebox, file) ;
		if(topLine >= lasttopLine) {
			differencebox->ptoplnolf = topLine = lasttopLine ;
		} else {
			lasttopLine = 0 ;
		}
		differencebox->toplnolf = topLine ;
	} else {
		differencebox->ptoplnorf = differencebox->toplnorf;
		lasttopLine = GetTopOfLastScreen(differencebox, file) ;
		if(topLine >= lasttopLine) {
			differencebox->ptoplnorf = topLine = lasttopLine ;
		} else {
			lasttopLine = 0 ;
		}
		differencebox->toplnorf = topLine ;
	}

	return lasttopLine ;
}


/********************************
 *
 *      TopOfLastScreen
 *
 ********************************/

int
TopOfLastScreen(differencebox, file)
        DifferenceBoxPtr	differencebox;
        WhichFile		file;
{
	int	topLine, lasttopLine;

	/* This function returns a true or false indication if the current
	 * top line of the specified file is the last top line of the file
	 * (an eof check).  dxdiff uses the top line of a screen as a basis
	 * for all scrolling/skipping operations; end-of-file checks are
	 * based on the top line of the last screen.
	 */

	if (file == LeftFile) {
		topLine = differencebox->toplnolf ;
		lasttopLine = GetTopOfLastScreen(differencebox, file) ;
	} else {
		topLine = differencebox->toplnorf ;
		lasttopLine = GetTopOfLastScreen(differencebox, file) ;
	}

#ifdef SCROLLDEBUG
	fprintf(stderr,"topLine = %d, lasttopLine = %d\n", topLine,lasttopLine);
#endif

	if(topLine >= lasttopLine)
		return True ;
	else
		return False ;
}


/********************************
 *
 *      DiffIsVisible
 *
 ********************************/

#define	BelowDiffBox(p,db,file)					\
		((file == LeftFile) ?				\
		((p)->ns1->numbers[1] < (db)->toplnolf) :	\
		((p)->ns2->numbers[1] < (db)->toplnorf))

#define	AboveDiffBox(p,db,file)						     \
		((file == LeftFile) ?					     \
		((p)->ns1->numbers[0] > ((db)->toplnolf + (db)->rows - 1)) : \
		((p)->ns2->numbers[0] > ((db)->toplnorf + (db)->rows - 1)))

Boolean
DiffIsVisible(db, p, file)
	DifferenceBoxPtr db ;
	EdcPtr p ;
	WhichFile file ;
{
	Boolean lVisible, rVisible ;
	int lDiffTop, lDiffBot, lViewportTop, lViewportBot,
	    rDiffTop, rDiffBot, rViewportTop, rViewportBot ;

	/* This function returns true if the specified diff is visible or
	 * false if not. It takes linked scrolling into account.  If linked
	 * scrolling is on, the diff must be at least partially visible in
	 * both the left and right screens to be counted as visible.
	 */

	lVisible = False ;
	lDiffTop = p->ns1->numbers[0] ;
	lDiffBot = p->ns1->numbers[1] ;
	lViewportTop = db->toplnolf ;
	lViewportBot = lViewportTop + db->rows - 1 ;

	rVisible = False ;
	rDiffTop = p->ns2->numbers[0] ;
	rDiffBot = p->ns2->numbers[1] ;
	rViewportTop = db->toplnorf ;
	rViewportBot = rViewportTop + db->rows - 1 ;

	if(db->scrollmode == ScrollBoth)
		if(BelowDifferenceBox(p, db) || AboveDifferenceBox(p, db)) 
			return False ;
	else
		if(BelowDiffBox(p, db, file) || AboveDiffBox(p, db, file))
			return False ;

	if((lViewportTop >= lDiffTop && lViewportTop <= lDiffBot) ||
	   (lViewportBot >= lDiffTop && lViewportBot <= lDiffBot) ||
	   (lDiffTop >= lViewportTop && lDiffTop <= lViewportBot) ||
	   (lDiffBot >= lViewportTop && lDiffBot <= lViewportBot))
		lVisible = True ;

	if((rViewportTop >= rDiffTop && rViewportTop <= rDiffBot) ||
	   (rViewportBot >= rDiffTop && rViewportBot <= rDiffBot) ||
	   (rDiffTop >= rViewportTop && rDiffTop <= rViewportBot) ||
	   (rDiffBot >= rViewportTop && rDiffBot <= rViewportBot))
		rVisible = True ;

	if(db->scrollmode == ScrollBoth) {
		if(lVisible == True && rVisible == True)
			return True ;
		else
			return False ;
	} else {
		if(file == LeftFile && lVisible == True)
			return True ;
		else
		if(file == RightFile && rVisible == True)
			return True ;
		else
			return False ;
	}
}


/********************************
 *
 *      DifferenceBoxSlaveScrollOtherDiffs
 *
 ********************************/

static void
DifferenceBoxSlaveScrollOtherDiffs(differencebox, inwhichfile)
	register DifferenceBoxPtr differencebox;
	WhichFile		  inwhichfile; /* to slave scroll ?? */
{
	register EdcPtr	p;
	register int	top,
			ptop,
			stop,
			rows = differencebox->rows,
			mmiddle,
			lasttopLine;
	register int	mnsoffset,
			snsoffset;
	WhichFile	master,
			slave ;

	int		scroll;

#ifdef	SCROLLDEBUG
	fprintf(stderr,"DifferenceBoxSlaveScrollOtherDiffs()\n") ;
	ScrollDebug(differencebox) ;
#endif

	/* This purpose of this function is to scroll the slave screen, based
	 * upon the number of rows that the master screen has been scrolled.
	 * However, an attempt is made to locate the closest diff to the center
	 * of the master screen.  If a diff is found and the middle of the diff
	 * is still within the boundaries of the master's screen, the top of
	 * the slave screen is adjusted such that the corresponding diff in
	 * the slave's file is near the center of the slave's screen.  If the
	 * middle of the diff was not within the bounds of the master's screen,
	 * then the slave's top line is adjusted by the number of rows that
	 * the master had scrolled upon entry.
	 *
	 * Interestingly, this centering action not only makes the diff look
	 * good, it can also help to bring the master and slave files back in
	 * sync with each other, depending upon the number of and location of
	 * diffs throughout the files.
	 *
	 * However, there are some cases where the attempt to center the diffs
	 * resulted in some pretty bizzare behaviour, especially for files
	 * containing only one diff.  An attempt has been made to prevent these
	 * misbehaviors by including additional conditions that must be met
	 * before attempting to center the diffs.  These conditionas may need
	 * further adjustment to handle other cases that show unusual behaviour.
	 * This is a hard call to make, because everything depends upon the
	 * nature of the files being diff'd.
	 */

	if (inwhichfile == LeftFile) {
		top = differencebox->toplnorf;
		ptop = differencebox->ptoplnorf;
		stop = differencebox->toplnolf ;
		mnsoffset = XtOffset(EdcPtr, ns2);
		snsoffset = XtOffset(EdcPtr, ns1);
		master = RightFile;
		slave = LeftFile ;
	} else {
		top = differencebox->toplnolf;
		ptop = differencebox->ptoplnolf;
		stop = differencebox->toplnorf ;
		mnsoffset = XtOffset(EdcPtr, ns1);
		snsoffset = XtOffset(EdcPtr, ns2);
		master = LeftFile;
		slave = RightFile ;
	}

	mmiddle = (top + rows / 2) - 1 ;

	if ((p = DifferenceBoxFindNearestDiffToLine(differencebox, mmiddle, master)) != (EdcPtr)NULL) {
		mmiddle = ((Numseq(p, mnsoffset))->numbers[0] + 
			   (Numseq(p, mnsoffset))->numbers[1]) / 2;
	}

#ifdef SCROLLDEBUG
	fprintf(stderr,"master:  n0 = %d, n1 = %d\n",
		(Numseq(p, mnsoffset))->numbers[0],
		(Numseq(p, mnsoffset))->numbers[1]) ;
	fprintf(stderr,"slave:  n0 = %d, n1 = %d\n",
		(Numseq(p, snsoffset))->numbers[0],
		(Numseq(p, snsoffset))->numbers[1]) ;
#endif

	if (p != (EdcPtr)NULL &&
	   ((mmiddle >= top) && (mmiddle < top + rows)) &&
	   (!differencebox->scrolling || (differencebox->scrolling &&
	   	((top < ptop && stop > 1) ||
		 (top > ptop && !TopOfLastScreen(differencebox, slave))) &&
		(differencebox->head != differencebox->tail)))) {
		int	smiddle;

		smiddle = ((Numseq(p, snsoffset))->numbers[0] + 
			   (Numseq(p, snsoffset))->numbers[1]) / 2;

		scroll = smiddle - (mmiddle - top);

#ifdef SCROLLDEBUG
		fprintf(stderr,"scroll = smiddle - (mmiddle - top):  %d = %d - (%d - %d)\n",
			scroll, smiddle, mmiddle, top) ;
#endif


	} else { /* no visible differences in difference box */
		 /* so scroll the other one by the same number of lines */

		scroll = ((inwhichfile == LeftFile) ? differencebox->toplnolf
						    : differencebox->toplnorf)
			 + (top - ptop); 
	}

	if((lasttopLine = SetTopLine(differencebox,scroll,slave)) != 0)
		p = DifferenceBoxFindNearestDiffToLine
			(differencebox,lasttopLine,slave) ;

	if(inwhichfile == LeftFile)
		differencebox->viewprtl = p ;
	else
		differencebox->viewprtr = p ;
}



/********************************
 *
 *      SkipToDiffInDifferenceBox
 *
 ********************************/

static void
SkipToDiffInDifferenceBox(differencebox, whichfile, poffset)
	register DifferenceBoxPtr differencebox;
	WhichFile		  whichfile;
	int		  	  poffset;
{
	register int	nsoffset, line, top ;
	EdcPtr		p;
	Boolean	 	isnext = (poffset == XtOffset(EdcPtr, next)), eol;

	/* This function skips to the next or previous diff in a file, based
	 * upon which button was selected (left or right file; next or prev
	 * diff).  An attempt is made to locate a diff closest to the center of
	 * the master/selected file's screen.  If linked scrolling is on,
	 * DifferenceBoxSlaveScrollOtherDiffs() will attempt to center the diff
	 * on the slave's screen.
	 *
	 * Aside from some outright bugs, several misbehaviors were present in
	 * this "skipping" code, which I have tried to correct.  Some of these
	 * actions would not have been encountered had dxdiff been designed to
	 * stipple the Next/Prev diff buttons when the Tail/Head of the diff
	 * list is encountered.  Instead, this function may act repeatedly on
	 * the first/last diff in the diff list.  The combination of the lack
	 * of this feature, the attempts to center the diffs on the screens,
	 * logic errors, and the nature of the files being diff'd resulted in
	 * some unusual behaviours, which I have attempted to correct by adding
	 * some beginning-of-file/end-of-file/end-of-list (eol) processing.
	 */
	


#ifdef	SCROLLDEBUG
	fprintf(stderr,"SkipToDiffInDifferenceBox(poffset = %d, isnext = %d)\n",
		poffset, isnext) ;
	ScrollDebug(differencebox) ;
#endif


	if (whichfile ==  LeftFile) {
		line = (differencebox->toplnolf + differencebox->rows / 2) - 1;
		nsoffset = XtOffset(EdcPtr, ns1);
		top = differencebox->toplnolf;
	} else {
		line = (differencebox->toplnorf + differencebox->rows / 2) - 1;
		nsoffset = XtOffset(EdcPtr, ns2);
		top = differencebox->toplnorf;
	}

	if ((isnext && TopOfLastScreen(differencebox, whichfile)) ||
	   (!isnext && top == 1)) {
		differencebox->lastscrolled = whichfile;

#ifdef SCROLLDEBUG
	ScrollDebug(differencebox) ;
#endif
		return ;
	}

	p = DifferenceBoxFindNearestDiffToLine(differencebox,line,whichfile);

	while (p != (EdcPtr)NULL) {
		int	n0, n1 ;

		eol = ((!isnext && p == differencebox->head) ||
		      (isnext && p == differencebox->tail));

		n0 = (Numseq(p, nsoffset))->numbers[0]; 
		n1 = (Numseq(p, nsoffset))->numbers[1];

		line = ((n0 + n1 - differencebox->rows) / 2) - 1 ;

#ifdef	SCROLLDEBUG
	fprintf(stderr,"p = %x, head = %x, tail = %x\n",
		p, differencebox->head, differencebox->tail) ;
	fprintf(stderr,"eol = %d, n0 = %d, n1 = %d, line = %d\n",
		eol, n0, n1, line) ;
#endif

		if (!eol &&
		   ((isnext && line <= top) || (!isnext && line >= top))) {
			p = (EdcPtr)(*(EdcPtr *)((long)p + poffset));	/* try next/prev one */

#ifdef	SCROLLDEBUG
	fprintf(stderr,"trying next...p = %x\n", p) ;
#endif
			continue;
		} else {
		if (eol &&
		  ((isnext && line <= top) || (!isnext && line >= top)))
			break ;
		else
		if (eol && DiffIsVisible(differencebox, p, whichfile))
			break ;
		}

		if(whichfile == LeftFile)
			differencebox->viewprtl = p ;
		else
			differencebox->viewprtr = p ;
		differencebox->scrolling = False ;

		ScrollDifferenceBox(differencebox, line, whichfile);

		break;	/* got one */
	}

	differencebox->lastscrolled = whichfile;

#ifdef SCROLLDEBUG
	ScrollDebug(differencebox) ;
#endif
}




/********************************
 *
 *	UpdateAndPaintDiffsInDifferenceBox
 *
 ********************************/

void
UpdateAndPaintDiffsInDifferenceBox(differencebox)
	register DifferenceBoxPtr differencebox;
{
	DifferenceBoxEstablishVisibleDiffs(differencebox);
	DifferenceBoxCreateXPointList(differencebox);
	DifferenceBoxDrawDifferences(differencebox);
	DifferenceBoxCreateLineNumberList(differencebox);
	DifferenceBoxPaintLineNumbers(differencebox);

	if (differencebox->resizenotifypending) {
		differencebox->resizenotifypending = False;
		(*differencebox->resizenotify)(differencebox, differencebox->prevdb, 
					       differencebox->clientdata);

		XtFree((char *)differencebox->prevdb);
		differencebox->prevdb = (DifferenceBoxPtr)NULL;
	}
}



/********************** Public Routines ************************/








/********************************
 *
 *      CreateDifferenceBox
 *
 ********************************/


DifferenceBoxPtr
CreateDifferenceBox(parent, name, core, constraints, lw, fontheight,
		    vmargins, lflen, rflen,lnforeground, font, display)
	register Widget		parent;
	char 			*name;
	register CoreArgListPtr	core;
	ADBConstraintArgListPtr	constraints;
	int			lw,
				fontheight,
				vmargins,
				lflen,
				rflen;
	XColor			*lnforeground;
	register XFontStruct	*font;
	DxDiffDisplayPtr	display;
{
	
	unsigned int			depth;
	Arg				deptharg;
	register DifferenceBoxPtr	differencebox;		


	differencebox = (DifferenceBoxPtr)XtMalloc(sizeof (DifferenceBox));

	if (differencebox == (DifferenceBoxPtr)NULL) { /* error */
		return (DifferenceBoxPtr)NULL;
	}


	if (core != (CoreArgListPtr)NULL) {
		bcopy((char *)core, (char *)&(differencebox->core),
		      sizeof (CoreArgList));
	}

	if (constraints != (ADBConstraintArgListPtr)NULL) {
		bcopy((char *)constraints, (char *)&(differencebox->constraints),
		      sizeof (ADBConstraintArgList));
	}
	
	differencebox->display = (caddr_t)display ;

	differencebox->lnos = (LineNumberPtr)NULL;
	differencebox->linenumbers = (font == (XFontStruct *)NULL);
	differencebox->font = font;

	differencebox->width = CoreWidth(differencebox->core);
	differencebox->rows = GetVerticalSliderSize(differencebox) ;
	differencebox->fontheight = fontheight;
	differencebox->vmargins = vmargins;
	differencebox->height = CoreHeight(differencebox->core) =
		fontheight * differencebox->rows + 2 * vmargins;
	
	differencebox->toplnolf = differencebox->toplnorf = 0;

	differencebox->npoints = 0;
	differencebox->points = (PointListPtr)NULL;
	differencebox->nstrs = 0;

	SetDifferenceBoxScrollOff(differencebox);
	SetDifferenceBoxDraggingOff(differencebox);
	SetDifferenceBoxDrawDiffsOff(differencebox);
	differencebox->drawdiffsas = DrawDiffsAsLines;
	
	differencebox->lastscrolled = (WhichFile)(-1);	/* uugh! */

	differencebox->draggc = (GC)NULL;
	differencebox->lnfgc = (GC)NULL;

	differencebox->head = differencebox->tail = (EdcPtr)NULL;
	differencebox->vdtop = differencebox->vdbottom = (EdcPtr)NULL;
	differencebox->viewprtl = differencebox->viewprtr = (EdcPtr)NULL;
	differencebox->dllen = 0;

	differencebox->lflen = lflen;
	differencebox->rflen = rflen;


	differencebox->resizenotifypending = False;
	differencebox->resizenotify = (void (*)())NULL;
	differencebox->clientdata = (caddr_t)NULL;
	differencebox->prevdb = (DifferenceBoxPtr)NULL;
	
	
	differencebox->exposecallback.name = XmNexposeCallback;

	CoreDestroyCallBack(differencebox->core) = (XtArgVal)destroycallbacklist;
	differencebox->exposecallback.value = (XtArgVal)exposecallbacklist;

	destroycallbacklist[0].closure = exposecallbacklist[0].closure = 
		(caddr_t)differencebox;

	differencebox->window = XmCreateDrawingArea(parent, name,
		(ArgList)&(differencebox->core),
		NumberOfArgsBetween(&(differencebox->core),
		    PointerToArg(differencebox->exposecallback)) + 1);

	if (differencebox->window == (Widget)NULL) {
		XtFree((char *)differencebox);
		return ((DifferenceBoxPtr)NULL);
	}

	XtAddEventHandler(differencebox->window, 
		StructureNotifyMask | SubstructureNotifyMask, False,
		(XtEventHandler)DifferenceBoxMapEventHandler,
		differencebox);

	differencebox->painting = False;

	depth = DefaultDepthOfScreen(DefaultScreenOfDisplay(XtDisplay(parent)));
	deptharg.name = XmNdepth;
	deptharg.value = (XtArgVal)&depth;
	XtGetValues(differencebox->window, &deptharg, 1);

	differencebox->lnforeground = lnforeground;

	if (depth < 2)
		differencebox->drawstring = (void(*))XDrawImageString;
	else
		differencebox->drawstring = (void(*))XDrawString;

	{
		XtGCMask	mask =	( GCLineWidth	|
					  GCGraphicsExposures |
					  GCForeground	|
					  GCBackground
					);
		XGCValues	values;
		Arg		args[2];
		XColor		fg,bg;


		args[0].name = XmNforeground;
		args[0].value = (XtArgVal)&fg;

		args[1].name = XmNbackground;
		args[1].value = (XtArgVal)&bg;

		XtGetValues(differencebox->window, args, 2);

		values.line_width = lw;
		values.graphics_exposures = False;
		values.foreground = fg.pixel;
		values.background = bg.pixel;

		differencebox->lw = lw ? lw : 1;
		differencebox->gc = XtGetGC(differencebox->window,
					    mask, &values);

		if (differencebox->gc == (GC)NULL) { /* error */
		}


		if (lw > 0) {
			values.line_width = 0;
			values.graphics_exposures = False;

			differencebox->draggc = XtGetGC(differencebox->window,
						        mask, &values);

			if (differencebox->draggc == (GC)NULL) { /* error */
				differencebox->draggc = differencebox->gc;
			}
		} else
				differencebox->draggc = differencebox->gc;

		if (differencebox->font != (XFontStruct *)NULL) {

			mask = GCFont | GCGraphicsExposures;

			values.font = differencebox->font->fid;
			values.graphics_exposures = False;

			if (depth >= 2 && differencebox->lnforeground != (XColor *)NULL) {
				mask |= GCForeground;
				values.foreground = differencebox->lnforeground->pixel;
			}

			differencebox->lnfgc = XtGetGC(differencebox->window,
						       mask, &values);
		}
	}

	for (differencebox->tfw = 2; lflen /= 10 ; differencebox->tfw++)
	; /* how many digits wide (at most) will the line number be ? */

	for (; rflen /= 10; differencebox->tfw++)
	; /* how many digits wide (at most) will the line number be ? */


	return differencebox;
}



/********************************
 *
 *      ScrollDifferenceBox
 *
 ********************************/

ScrollDifferenceBox(differencebox, scroll, inwhichfile)
	register DifferenceBoxPtr differencebox;
	int 			  scroll; 	/* absolute line number */
	WhichFile 		  inwhichfile;
{

	EdcPtr	viewptr ;

#ifdef	SCROLLDEBUG
	fprintf(stderr, "ScrollDifferenceBox():  scroll = %d\n", scroll) ;
#endif

	if (differencebox->scrollmode == NotScrolling)
		return;

	(void)SetTopLine(differencebox, scroll, inwhichfile) ;

	if(differencebox->scrolling) {
		viewptr = DifferenceBoxFindNearestDiffToLine
			(differencebox,scroll,inwhichfile) ;
		if(inwhichfile == LeftFile)
			differencebox->viewprtl = viewptr ;
		else
			differencebox->viewprtr = viewptr ;
	}

#ifdef	SCROLLDEBUG
	{
		int nsoffset, n0, n1 ;

		if(differencebox->scrolling) {
			if(inwhichfile == LeftFile)
				nsoffset = XtOffset(EdcPtr, ns1) ;
			else
				nsoffset = XtOffset(EdcPtr, ns2) ;
			n0 = (Numseq(viewptr, nsoffset))->numbers[0];
			n1 = (Numseq(viewptr, nsoffset))->numbers[1];

			fprintf(stderr, "n0 = %d, n1 = %d\n", n0, n1) ;
		}
	}
#endif

	if (differencebox->scrollmode == ScrollBoth) {
		DifferenceBoxSlaveScrollOtherDiffs(differencebox, 
			Slave(inwhichfile)) ;
	}

	differencebox->lastscrolled = inwhichfile;

	UpdateAndPaintDiffsInDifferenceBox(differencebox);
}


/********************************
 *
 *      ScrollToPrevDiffInDifferenceBox
 *
 ********************************/

void
ScrollToPrevDiffInDifferenceBox(differencebox, inwhichfile)
	register DifferenceBoxPtr differencebox;
	WhichFile		  inwhichfile;
{
	SkipToDiffInDifferenceBox(differencebox, inwhichfile,
				  XtOffset(EdcPtr, prev));
}


/********************************
 *
 *      ScrollToNextDiffInDifferenceBox
 *
 ********************************/

void
ScrollToNextDiffInDifferenceBox(differencebox, inwhichfile)
	register DifferenceBoxPtr differencebox;
	WhichFile		  inwhichfile;
{
	SkipToDiffInDifferenceBox(differencebox, inwhichfile,
				  XtOffset(EdcPtr, next));
}


/********************************
 *
 *      LoadNewDifferenceBox
 *
 ********************************/

void
LoadNewDifferenceBox(differencebox, head, tail)
	register DifferenceBoxPtr differencebox;
	EdcPtr			  head,tail;	/* the diff list */
{
	DiffScrollMode scrollmode = differencebox->scrollmode;
	differencebox->vdtop = differencebox->vdbottom = (EdcPtr)NULL;
	differencebox->viewprtl = differencebox->viewprtr = (EdcPtr)NULL;
	differencebox->head = head;
	differencebox->tail = tail;
	differencebox->toplnolf = differencebox->toplnorf = 1;
	differencebox->ptoplnorf = differencebox->ptoplnolf = 0;
	differencebox->dllen = 0;

	if (XtIsRealized(differencebox->window)) {	/* there is a chance we're mapped already */
		XWindowAttributes	attrs;

		XGetWindowAttributes(XtDisplay(differencebox->window), XtWindow(differencebox->window), &attrs);

		differencebox->painting = (attrs.map_state != IsUnmapped);
	}

	SetDifferenceBoxScrollOne(differencebox);

	differencebox->viewprtl = 
		DifferenceBoxFindNearestDiffToLine(differencebox, 1, 
						   LeftFile);

	differencebox->viewprtr = 
		DifferenceBoxFindNearestDiffToLine(differencebox, 1,
						   RightFile);

	if (CheckDifferenceBoxWindowDimensions(differencebox))
		DifferenceBoxResizeHandler(differencebox);
	UpdateAndPaintDiffsInDifferenceBox(differencebox);

	differencebox->scrollmode = scrollmode;
}


/********************************
 *
 *      ReLoadDifferenceBox
 *
 ********************************/

void
ReLoadDifferenceBox(differencebox, head, tail, rflen, lflen)
	register DifferenceBoxPtr differencebox;
	EdcPtr			  head,tail;	/* the diff list */
{
	differencebox->rflen = rflen;
	differencebox->lflen = lflen;

	for (differencebox->tfw = 2; lflen /= 10 ; differencebox->tfw++)
	; /* how many digits wide (at most) will the line number be ? */

	for (; rflen /= 10; differencebox->tfw++)
	; /* how many digits wide (at most) will the line number be ? */


	LoadNewDifferenceBox(differencebox, head, tail);
}





/********************************
 *
 *      SetDifferenceBoxViewportSize
 *
 ********************************/

void
SetDifferenceBoxViewportSize(differencebox, rows)
	register DifferenceBoxPtr differencebox;
	int			  rows;
{
	differencebox->rows = rows;
	
	CheckDifferenceBoxWindowDimensions(differencebox);
	DifferenceBoxResizeHandler(differencebox);

	UpdateAndPaintDiffsInDifferenceBox(differencebox);
}




/********************************
 *
 *      SetDifferenceBoxFontHeight
 *
 ********************************/

void
SetDifferenceBoxFontHeight(differencebox, fontheight)
	register DifferenceBoxPtr differencebox;
	int			  fontheight;
{
	differencebox->fontheight = fontheight;

	CheckDifferenceBoxWindowDimensions(differencebox);
	DifferenceBoxResizeHandler(differencebox);

	UpdateAndPaintDiffsInDifferenceBox(differencebox);
}





/********************************
 *
 *      SetDifferenceBoxVMargins
 *
 ********************************/

void
SetDifferenceBoxVMargins(differencebox, vmargins)
	register DifferenceBoxPtr differencebox;
	int			  vmargins;
{
	differencebox->vmargins = vmargins;

	CheckDifferenceBoxWindowDimensions(differencebox);
	DifferenceBoxResizeHandler(differencebox);

	UpdateAndPaintDiffsInDifferenceBox(differencebox);
}


/********************************
 *
 *      SetDifferenceBoxDragging
 *
 ********************************/

void
SetDifferenceBoxDragging(differencebox, state)
	register DifferenceBoxPtr differencebox;
	Boolean			  state;
{
	if (state) {
		SetDifferenceBoxDraggingOn(differencebox);
	} else {
		SetDifferenceBoxDraggingOff(differencebox);
		UpdateAndPaintDiffsInDifferenceBox(differencebox);

		XFlush(XtDisplay(differencebox->window));
	}

}

/********************************
 *
 *      SetDifferenceBoxResizeNotify
 *
 ********************************/

void
SetDifferenceBoxResizeNotify(differencebox, ftn, closure)
	register DifferenceBoxPtr differencebox;
	void			  (*ftn)();
	caddr_t			  closure;
{
	differencebox->resizenotify = ftn;
	differencebox->clientdata = closure;
}

/********************************
 *
 *      SetDifferenceBoxDisplayLineNumbers
 *
 ********************************/

Boolean
SetDifferenceBoxDisplayLineNumbers(differencebox, state)
	register DifferenceBoxPtr differencebox;
	Boolean			  state;
{
	Boolean oldstate = differencebox->linenumbers;

	differencebox->linenumbers = state;

	if (oldstate != state)
		UpdateAndPaintDiffsInDifferenceBox(differencebox);

	return state;
}


/********************************
 *
 *      SetDifferenceBoxScrollingMode
 *
 ********************************/

void
SetDifferenceBoxScrollingMode(differencebox, scrollmode)
	register DifferenceBoxPtr differencebox;
	DiffScrollMode		  scrollmode;
{
	WhichFile	master,
			slave ;

#ifdef SCROLLDEBUG
	ScrollDebug(differencebox) ;
#endif

	differencebox->scrollmode = scrollmode;

	if (scrollmode == ScrollBoth && differencebox->lastscrolled != (WhichFile)(-1)) {
		master = differencebox->lastscrolled ;
		slave = (master == LeftFile) ? RightFile : LeftFile ;

		if(master == LeftFile)
			differencebox->viewprtl =
				DifferenceBoxFindNearestDiffToLine
				(differencebox,differencebox->toplnolf,master);
		else
			differencebox->viewprtr =
				DifferenceBoxFindNearestDiffToLine
				(differencebox,differencebox->toplnorf,master);

		differencebox->scrolling = False ;
		DifferenceBoxSlaveScrollOtherDiffs(differencebox,slave);

		UpdateAndPaintDiffsInDifferenceBox(differencebox);
	}

#ifdef SCROLLDEBUG
	ScrollDebug(differencebox) ;
#endif

}
	


/********************************
 *
 *      SetDifferenceBoxPaintingStyle
 *
 ********************************/

void
SetDifferenceBoxPaintingStyle(differencebox, drawmode)
	register DifferenceBoxPtr differencebox;
	DiffDrawMode		  drawmode;
{
	differencebox->drawdiffsas = drawmode;

	if (differencebox->painting) {
		UpdateAndPaintDiffsInDifferenceBox(differencebox);
	}
}

/********************************
 *
 *      DestroyDifferenceBox
 *
 ********************************/

void
DestroyDifferenceBox(differencebox)
	register DifferenceBoxPtr differencebox;
{
	XtDestroyWidget(differencebox->window); /* let the callback do the work */
}
