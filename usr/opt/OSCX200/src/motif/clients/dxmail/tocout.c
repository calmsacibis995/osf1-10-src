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
#ifndef lint
static char rcs_id[] = "@(#)$RCSfile: tocout.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/08/03 00:02:21 $";
#endif

/*
 *                     Copyright (c) 1987, 1991 by
 *              Digital Equipment Corporation, Maynard, MA
 *                      All rights reserved.
 *
 *   This software is furnished under a license and may be used and
 *   copied  only  in accordance with the terms of such license and
 *   with the  inclusion  of  the  above  copyright  notice.   This
 *   software  or  any  other copies thereof may not be provided or
 *   otherwise made available to any other person.  No title to and
 *   ownership of the software is hereby transferred.
 *
 *   The information in this software is subject to change  without
 *   notice  and should not be construed as a commitment by Digital
 *   Equipment Corporation.
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
 */

/* File: tocout.c -- toc output module */

#include "decxmail.h"
#include <Xm/XmP.h>
#include <Xm/PushB.h>
#include "toc.h"
#include "tocintrnl.h"

static Pixmap NormalPixmap = NULL;
static Pixmap CurrentPixmap = NULL;
static Pixmap UnopenedPixmap = NULL;
static Pixmap ShownPixmap = NULL;
static Widget lastwidget = NULL;
static Scrn scrn = NULL;
static haschanged = FALSE;
static int disabledepth = 0;

#define CheckNewScrn(widget) 						\
    if (widget != lastwidget) {					\
	FlushChanges();						\
	lastwidget = widget;					\
	scrn = ScrnFromWidget((Widget) widget);		\
    }

static void FlushChanges()
{
    disabledepth = 0;
    if (scrn == NULL || !haschanged) return;
    haschanged = FALSE;
    CreateWidgetFromLayout(scrn, scrn->tocwidget,
					   "tocbuttonpopup");
    RedoButtonPixmaps(scrn);
}


Pixmap MsgGetPixmap(toc, msg)
Toc toc;
Msg msg;
{
    Pixmap pixmap;
    register int j, l;
    Boolean caninc = TocCanIncorporate(toc);
    Sequence seq;
    MsgList mlist;
    static Scrn defscrn = NULL;

    if (toc == NULL || msg == NULL) return NULL;
    if (NormalPixmap == NULL) {
	NormalPixmap = GetPixmapNamed("tocbutton");
	CurrentPixmap = GetPixmapNamed("currentbutton");
	UnopenedPixmap = GetPixmapNamed("unopenedbutton");
	ShownPixmap = GetPixmapNamed("shownbutton");
    }
    if (caninc) {
	seq = TocGetSeqNamed(toc, unseenSeqName);
	if (seq) mlist = seq->mlist;
	else caninc = FALSE;
    }
    l = 0;
    pixmap = NormalPixmap;
    if (msg == toc->curmsg) {
	pixmap = CurrentPixmap;
	if (defscrn == NULL || !defscrn->mapped || defscrn->readnum != 1) {
	    defscrn = NULL;
	    for (j=0 ; j<numScrns ; j++) {
		if (scrnList[j]->mapped && scrnList[j]->readnum == 1) {
		    defscrn = scrnList[j];
		    break;
		}
	    }
	}
	if (defscrn && msg == defscrn->msg)
	    pixmap = ShownPixmap;
    } else if (caninc && l < mlist->nummsgs) {
	while (l < mlist->nummsgs && mlist->msglist[l]->msgid < msg->msgid)
	    l++;
	if (l < mlist->nummsgs && mlist->msglist[l] == msg)
	    pixmap = UnopenedPixmap;
    }
    return pixmap;
}

/*
 * Work proc to actually recalculate what pixmaps to display for each button.
 * Modified to speed up the reading of messages using Prev - Next.
 */

static Boolean WorkRedoButtonPixmaps(data)
Opaque data;
{
    Scrn thisscrn = (Scrn) data;
    Toc toc = thisscrn->toc;
    Msg msg;
    Pixmap pixmap;
    register int i, j, p, l, pos;
    Boolean caninc = TocCanIncorporate(toc);
    Sequence seq;
    MsgList mlist;
    static Arg args[] = {
	{XmNlabelPixmap, NULL},
    };
    TagRec tag;
    static Scrn defscrn = NULL;

    DEBUG(("Called WorkRedoButtonPixmaps.\n"));
    thisscrn->neednewpixmaps = FALSE;
    if (toc == NULL || toc->nummsgs == 0) return TRUE;
    if (caninc) {
	seq = TocGetSeqNamed(toc, unseenSeqName);
	if (seq) mlist = seq->mlist;
	else caninc = FALSE;
    }
    tag.tagFields.tocNumber = TUGetTocNumber(toc);
    l = 0;
    for (i=0; i < toc->nummsgs; i++) {
	msg = toc->msgs[i];
	pixmap = NormalPixmap;
	if (msg == toc->curmsg) {
	    pixmap = CurrentPixmap;
	    if (defscrn == NULL || !defscrn->mapped || defscrn->readnum != 1) {
		defscrn = NULL;
		for (j=0 ; j<numScrns ; j++) {
		    if (scrnList[j]->mapped && scrnList[j]->readnum == 1) {
			defscrn = scrnList[j];
			break;
		    }
		}
	    }
	    if (defscrn && msg == defscrn->msg)
		pixmap = ShownPixmap;
	} else if (caninc && l < mlist->nummsgs) {
	    while (l < mlist->nummsgs && mlist->msglist[l]->msgid < msg->msgid)
		l++;
	    if (l < mlist->nummsgs && mlist->msglist[l] == msg)
		pixmap = UnopenedPixmap;
	}
	if (msg->visible && pixmap != msg->buttonPixmap) {
	    msg->buttonPixmap = pixmap; /* AJ 01 */
	    TocDisableRedisplay(thisscrn->tocwidget);
	    tag.tagFields.msgNumber = TUGetMsgIndex(toc, msg);
	    pos = DXmSvnGetEntryNumber(thisscrn->tocwidget, tag.tagValue);
	    if (pos != 0)
	        DXmSvnInvalidateEntry(thisscrn->tocwidget, pos);
	    TocEnableRedisplay(thisscrn->tocwidget);
	}
    }
    return TRUE;
}

/*
 * Recalculate what pixmaps to display for each button.  Just sets up a work
 * proc to actually do it.
 */

void RedoButtonPixmaps(scrn)
Scrn scrn;
{
    DEBUG(("Called RedoButtonPixmaps.\n"));
    if (scrn && !scrn->neednewpixmaps) {
	if (scrn->mapped) {
	    scrn->neednewpixmaps = TRUE;
	    XtAddWorkProc(WorkRedoButtonPixmaps, (Opaque) scrn);
	} else {
	    (void) WorkRedoButtonPixmaps((Opaque) scrn);
	}
    }
}


void TocRedoButtonPixmaps(toc)
Toc toc;
{
register int i;

    DEBUG(("Called TocRedoButtonPixmaps\n"));
    if (toc != NULL)
	for (i=0 ; i<toc->num_scrns ; i++)
	    RedoButtonPixmaps(toc->scrn[i]);
}


/* ARGSUSED */
void ExecSelectThisMsg(w, event, params, num_params)
Widget w;
XEvent *event;
char **params;
Cardinal *num_params;
{
    Scrn thisscrn = ScrnFromWidget(w);
    Toc toc = thisscrn->toc;
    Msg msg;
    Widget svn = thisscrn->tocwidget;
    unsigned int entry;
    TRACE(("@ExecSelectThisMsg\n"));
    
    TocDisableRedisplay(svn);
    if (toc != NULL) {
	DXmSvnClearSelections(svn);
	entry = -1;
	DXmSvnMapPosition(svn, event->xbutton.x, event->xbutton.y,
	    &entry, NULL, (unsigned int *) &msg);
	if (entry > 0)
	    DXmSvnSelectEntry(svn, entry);
    }
    TocEnableRedisplay(svn);
    DestroyConfirmWindow();
}

/* ARGSUSED */
void ExecExtendThisMsg(w, event, params, num_params)
Widget w;
XEvent *event;
char **params;
Cardinal *num_params;
{
    TRACE(("@ExecExtendThisMsg\n"));
    ExecSelectThisMsg(w, event, params, num_params);
}


/* ARGSUSED */
void ExecExtendThisToc(w, event, params, num_params)
Widget w;
XEvent *event;
char **params;
Cardinal *num_params;
{
    Msg msg;

    TRACE(("@ExecExtendThisToc\n"));
    ExecSelectThisMsg(w, event, params, num_params);
}

void TocDisableRedisplay(w)
Widget w;
{
    CheckNewScrn(w);
    DXmSvnDisableDisplay(w);
    disabledepth++;
}

void TocEnableRedisplay(w)
Widget w;
{

    if (disabledepth > 0) {
	if (--disabledepth == 0) {
	    FlushChanges();
	}
    }
    DXmSvnEnableDisplay(w);
    CheckNewScrn(w);
}
