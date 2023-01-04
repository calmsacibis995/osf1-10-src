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
static char *BuildSystemHeader= "$Header: /alphabits/u3/x11/ode/rcs/x11/src/motif/clients/dxdiff/displaymenucbs.c,v 1.1.2.2 92/08/03 09:47:57 Dave_Hill Exp $";	/* BuildSystemHeader */
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
 *	displaymenucbs.c - display menu callbacks
 *
 *	Author:	Laurence P. G. Cable
 *
 *	Created : 5th May 1988
 *
 *
 *	Description
 *	-----------
 *
 *
 *	Modification History
 *	------------ -------
 *	
 *	31st May 1988	Laurence P. G. Cable
 *
 *	Pulled the edit button sorry didnt have the time!
 */

static char sccsid[] = "@(#)displaymenucbs.c	1.5	17:45:09 2/21/89";

#include <sys/types.h>
#include <sys/stat.h>

#ifdef  DEBUG
#include <stdio.h>
#endif  DEBUG
#include <X11/Xlib.h>
#include <Xm/Xm.h>
#include "dxdiff.h"
#include "arglists.h"
#include "y.tab.h"
#include "filestuff.h"
#include "parsediff.h"
#include "alloc.h"
#include "differencebox.h"
#include "menu.h"
#include "text.h"
#include "display.h"
#include "displaymenu.h"

/*******************************
 *******************************
 **
 ** Text Display Menu menu callbacks
 **
 *******************************
 *******************************/


/********************************
 *
 *      SkipToNextPrevDiffActivateCallback
 *
 ********************************/

extern	void	ScrollToNextDiffInDifferenceBox(), ScrollToPrevDiffInDifferenceBox();

static VoidProc
SkipToNextPrevDiffActivateCallback(w, clientd, calld)
	Widget	w;
	caddr_t	clientd;
	caddr_t	calld;
{
	DxDiffDisplayPtr	display = (DxDiffDisplayPtr)clientd;
	DifferenceBoxPtr	differencebox = DiffRegionADBPtrDifferenceBox(DxDiffDisplayPtrDiffRegionADB(display));
	TextDisplayADBPtr	ltext = DxDiffDisplayPtrLeftTextADB(display),
				rtext = DxDiffDisplayPtrRightTextADB(display),
				text, othertext;
	MenuEntryPtr		*lentries = AMenuBarPtrEntries(TextDisplayADBPtrMenuBar(ltext)),
				*rentries = AMenuBarPtrEntries(TextDisplayADBPtrMenuBar(rtext));
	Boolean			isleft;
	int			*top, *othertop, last, otherlast;
	void			(*scrollftn)();

	if ((isleft = (PushButtonEntryPtrWidget(MenuEntryPtrPushButtonEntryPtr(lentries[(int)NextDiffButton])) == w)) ||
	    PushButtonEntryPtrWidget(MenuEntryPtrPushButtonEntryPtr(rentries[(int)NextDiffButton])) == w) {
		scrollftn = ScrollToNextDiffInDifferenceBox;
	} else 
		if ((isleft = (PushButtonEntryPtrWidget(MenuEntryPtrPushButtonEntryPtr(lentries[(int)PrevDiffButton])) == w)) ||
		    PushButtonEntryPtrWidget(MenuEntryPtrPushButtonEntryPtr(rentries[(int)PrevDiffButton])) == w) {
			scrollftn = ScrollToPrevDiffInDifferenceBox;
		}

	if (isleft) {
		last = *(top = &(differencebox->toplnolf));
		otherlast = *(othertop = &(differencebox->toplnorf));
		text = ltext;
		othertext = rtext;
	} else {
		last = *(top = &(differencebox->toplnorf));
		otherlast = *(othertop = &(differencebox->toplnolf));
		text = rtext;
		othertext = ltext;
	}

	(*scrollftn)(differencebox, isleft ? LeftFile : RightFile);

	_XmTextDisableRedisplay(TextDisplayPtrWidget(TextDisplayADBPtrTextDisplay(text)), False);
	XmTextScroll(TextDisplayPtrWidget(TextDisplayADBPtrTextDisplay(text)), *top - last);

	if (differencebox->scrollmode == ScrollBoth) {
		_XmTextDisableRedisplay(TextDisplayPtrWidget(TextDisplayADBPtrTextDisplay(othertext)), False);
		XmTextScroll(TextDisplayPtrWidget(TextDisplayADBPtrTextDisplay(othertext)), *othertop - otherlast);
	}

	UpdateTextHighLights(TextDisplayADBPtrTextDisplay(ltext), TextDisplayADBPtrTextDisplay(rtext), differencebox);

	_XmTextEnableRedisplay(TextDisplayPtrWidget(TextDisplayADBPtrTextDisplay(text)));
	if (differencebox->scrollmode == ScrollBoth) {
		_XmTextEnableRedisplay(TextDisplayPtrWidget(TextDisplayADBPtrTextDisplay(othertext)));
	}
}

XtCallbackRec	SkipToNextPrevDiffCallbackList[] = {
			{ (XtCallbackProc)SkipToNextPrevDiffActivateCallback, 0 },
			{ (VoidProc)NULL, 0 }
		};



/********************************
 *
 *      EditActivateCallback
 *
 ********************************/

#ifdef	EDITBTN
static VoidProc
EditActivateCallback(w, clientd, calld)
	Widget	w;
	caddr_t	clientd;
	caddr_t	calld;
{
}

XtCallbackRec	EditCallbackList[] = {
			{ EditActivateCallback, 0 },
			{ (VoidProc)NULL, 0 }
		 };
#endif	EDITBTN
