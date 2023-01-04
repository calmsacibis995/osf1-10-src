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
static char rcs_id[] = "@(#)$RCSfile: tocutil.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/08/03 00:02:37 $";
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

/* tocutil.c -- internal routines for toc stuff. */

#include <sys/file.h>
#include "decxmail.h"
#include <Xm/XmP.h>
#include <Xm/Label.h>
#include <X11/Xatom.h>
/* #include <DXm/DXmSvn.h> included in externs.h */
#include "msg.h"
#include "mlist.h"
#include "toc.h"
#include "tocutil.h"
#include "tocintrnl.h"



Toc TUMalloc()
{
    Toc toc;
    toc = XtNew(TocRec);
    bzero((char *)toc, (int) sizeof(TocRec));
    toc->msgs = (Msg *) XtMalloc((unsigned) 1);
    toc->seqlist = (Sequence *) XtMalloc((unsigned) 1);
    toc->validity = unknown;
    toc->subFolderCount = 0;
    toc->level = 0;
    return toc;
}


/* Returns TRUE if the scan file for the given toc is out of date. */

int TUScanFileOutOfDate(toc)
  Toc toc;
{
    return LastModifyDate(toc->path) > (toc->lastreaddate + SCAN_SLOP);
}



void TUScanFileForToc(toc)
  Toc toc;
{
    static Arg arglist[] = {
	{XmNlabelString, NULL},
	{XtNx, (XtArgVal) 0},
	{XtNy, (XtArgVal) 0},
	{XtNborderWidth, (XtArgVal) 1}
    };
    Scrn scrn;
    Widget label;
    char  **argv, str[100], str2[10];
    XEvent event;
    if (toc) {
	BeginLongOperation();
	TUGetFullFolderInfo(toc);
	if (toc->num_scrns) scrn = toc->scrn[0];
	else if (numScrns > 0) scrn = scrnList[0];
	else scrn = NULL;
	if (scrn) {
	    DEBUG(("Rescanning %s\n",toc->foldername));
	    (void) sprintf(str, "Rescanning %s", toc->foldername);
	    arglist[0].value = (XtArgVal) XmStringCreateSimple(str);
	    label = XmCreateLabel(scrn->tocwidget, "alert",
				   arglist, XtNumber(arglist));
	    XtManageChild(label);
	    XtRealizeWidget(label);
	    XmStringFree((char *) arglist[0].value);

	    /*
	     * This next bit hacks an expose event to the label.  It's really
	     * a gross bit of code...
	     */
	    bzero((char *) &event, sizeof(XEvent));
	    event.xexpose.type = Expose;
	    event.xexpose.display = theDisplay;
	    event.xexpose.window = XtWindow(label);
	    event.xexpose.width = GetWidth(label);
	    event.xexpose.height = GetHeight(label);

#ifdef IGNOREDDIF
	    XtDispatchEvent(&event); 
#else
/* Precede all calls to XtDispatchEvent() with a call to XDPSDispatchEvent(): */
	    if(!(dps_exists && XDPSDispatchEvent(&event)))
	      XtDispatchEvent(&event);
#endif /* IGNOREDDIF */	

	    							/* %%%Hack. */
	    XFlush(XtDisplay(label));
	}

	argv = MakeArgv(6);
	argv[0] = scanCmd;
	(void) sprintf(str, "+%s", toc->foldername);
	argv[1] = str;
	argv[2] = "-width";
	(void) sprintf(str2, "%d", defTocWidth);
	argv[3] = str2;
	if (ScanFormatFile && *ScanFormatFile) {
	    argv[4] = "-form";
	    argv[5] = ScanFormatFile;
	} else argv[4] = NULL;
	DoCommand(argv, (char *) NULL, toc->scanfile);
	XtFree((char *) argv);

	if (scrn)
#ifdef NODESTROY
	    XtUnmanageChild(label);
#else
	    XtDestroyWidget(label);
#endif
	toc->validity = valid;
	toc->curmsg = NULL;	/* Get cur msg somehow! %%% */
	EndLongOperation();
    }
}


int TUGetTocNumber(toc)
Toc toc;
{
    int i;

    for (i = 0; i < numFolders; i++) {
	if (folderList[i] == toc) return i;
    }
    return -1;
}

int TUGetMsgPosition(toc, msg)
  Toc toc;
  Msg msg;
{
    register int msgid, h, l, m;
    char str[100];
    if (msg == NULL) return -1;
    msgid = msg->msgid;
    l = 0;
    h = toc->nummsgs - 1;
    while (l < h - 1) {
	m = (l + h) / 2;
	if (toc->msgs[m]->msgid > msgid)
	    h = m;
	else
	    l = m;
    }
    if (toc->msgs[l] == msg) return l;
    if (toc->msgs[h] == msg) return h;
    (void) sprintf(str, "TUGetMsgPosition search failed! hi=%d, lo=%d, msgid=%d",
		   h, l, msgid);
    Punt(str);
    return 0; /* Keep lint happy. */
}

int TUGetMsgIndex(toc, msg)
  Toc toc;
  Msg msg;
{
    int pos;
    if (msg == NULL || toc == NULL ||
	toc->scrn == NULL || toc->scrn[0] == NULL) return -1;

    for (pos = 0; pos < toc->nummsgs; pos++) {
	if (toc->msgs[pos] == msg) return pos += 1;
    }
    return -1;
}


void TUResetTocLabel(scrn)
  Scrn scrn;
{
    char str[500];
    Toc toc;
    if (scrn && scrn->toclabel) {
	toc = scrn->toc;
	if (toc == NULL)
	    (void) strcpy(str, " ");
	else {
	    if (toc->stopupdate) {
		toc->needslabelupdate = TRUE;
		return;
	    }
	    (void) sprintf(str, "Folder: %s  Sequence: %s    %d messages",
			   toc->foldername, toc->viewedseq->name,
			   toc->viewedseq == toc->seqlist[0] ? toc->nummsgs
			   	: toc->viewedseq->mlist->nummsgs);
	    toc->needslabelupdate = FALSE;
	}
	ChangeLabel(scrn->toclabel, str);
    }
}


/* A major toc change has occured; redisplay it.  (This also should work even
   if we now have a new source to display stuff from.) */

void TURedisplayToc(scrn)
  Scrn scrn;
{
    Toc toc;
    int entries;
    TagRec *msgs;
    int i, j, numDisplayed;
    Arg arg[1];
    int tocNumber;
    TagRec tag, findTag;
    int pos;

    if (scrn != NULL && scrn->tocwidget != NULL) {
	toc = scrn->toc;
	tag.tagFields.tocNumber = tocNumber = TUGetTocNumber(toc);
 	if (toc && toc->nummsgs > 0) {
	    if (toc->stopupdate) {
		toc->needsrepaint = TRUE;
		return;
	    }
	    msgs = (TagRec *) XtMalloc(toc->nummsgs * sizeof (TagRec));
	    numDisplayed = 0;
	    for (i = 0; i < toc->nummsgs; i++) {
		if (toc->msgs[i]->visible) {
		    msgs[numDisplayed].tagFields.tocNumber = tocNumber;
		    msgs[numDisplayed].tagFields.msgNumber = i + 1;
		    numDisplayed++;
		}
	    }
	    TocDisableRedisplay(scrn->tocwidget);
	    XtSetArg(arg[0], DXmSvnNnumberOfEntries, &entries);
	    XtGetValues(scrn->tocwidget, arg, 1);
	    if (!defSVNStyle) {
		XtSetArg(arg[0], DXmSvnNnumberOfEntries, &entries);
		XtGetValues(scrn->tocwidget, arg, 1);
		if (entries != 0)
		    DXmSvnDeleteEntries(scrn->tocwidget, 0, entries);
	    } else {
		tag.tagFields.msgNumber = 0;
		pos = toc->index = DXmSvnGetEntryNumber(scrn->tocwidget, tag.tagValue);
		if (pos != 0)
		    DXmSvnInvalidateEntry(scrn->tocwidget, pos);
		entries = 0;
		for (pos = pos + 1;;pos++) {
		   findTag.tagValue = DXmSvnGetEntryTag(scrn->tocwidget, pos);
		   if (findTag.tagFields.msgNumber == 0) break;
		   entries++;
		}
		if (entries > 0) {
		    DXmSvnDeleteEntries(scrn->tocwidget, toc->index, entries);
		}
	    }
	    if (toc == scrn->toc && numDisplayed) {
	        DXmSvnAddEntries(scrn->tocwidget, toc->index, numDisplayed, 
			(defSVNStyle ? toc->level+1 : 0), msgs,
			(defSVNStyle ? False : True));
	    }
	    XtFree((char *)msgs);
	    TocSetCurMsg(toc, TocGetCurMsg(toc));
	    TocEnableRedisplay(scrn->tocwidget);
	    TocCheckSeqButtons(toc);
	    toc->needsrepaint = FALSE;
	} else {
	    toc = scrn->lasttoc;
	    tag.tagFields.tocNumber = tocNumber = TUGetTocNumber(toc);
	    TocDisableRedisplay(scrn->tocwidget);
	    XtSetArg(arg[0], DXmSvnNnumberOfEntries, &entries);
	    XtGetValues(scrn->tocwidget, arg, 1);
	    if (defSVNStyle && toc) {
		tag.tagFields.msgNumber = 0;
		pos = toc->index = DXmSvnGetEntryNumber(scrn->tocwidget, tag.tagValue);
		if (pos != 0)
		    DXmSvnInvalidateEntry(scrn->tocwidget, pos);
		entries = 0;
		for (pos = pos + 1;;pos++) {
		   findTag.tagValue = DXmSvnGetEntryTag(scrn->tocwidget, pos);
		   if (findTag.tagFields.msgNumber == 0) break;
		   entries++;
		}
	    }
	    if (toc && entries > 0)
		    DXmSvnDeleteEntries(scrn->tocwidget, toc->index, entries);
	    TocEnableRedisplay(scrn->tocwidget);
	}
    }
    scrn->lasttoc = NULL;
}



void TULoadSeqLists(toc, curmsg)
  Toc toc;
  Msg *curmsg;			/* RETURN - message specified by 'cur' */
{
    Sequence seq, *oldseqs;
    FILE *fid;
    char str[500], *ptr, *ptr2, viewed[500];
    register int i, j, oldnumsequences;

    if (curmsg)
	*curmsg = NULL;
    if (toc->viewedseq) (void) strcpy(viewed, toc->viewedseq->name);
    else *viewed = 0;
    oldseqs = toc->seqlist;
    oldnumsequences = toc->numsequences;
    toc->numsequences = 1;
    toc->seqlist = (Sequence *) XtMalloc((Cardinal) sizeof(Sequence));
    seq = toc->seqlist[0] = XtNew(SequenceRec);
    bzero((char *) seq, sizeof(SequenceRec));
    seq->name = MallocACopy("all");
    seq->mlist = NULL;
    toc->viewedseq = seq;
    (void) sprintf(str, "%s/.mh_sequences", toc->path);
    fid = myfopen(str, "r");
    if (fid) {
	while (ptr = ReadLine(fid)) {
	    ptr2 = index(ptr, ':');
	    if (ptr2) {
		*ptr2++ = 0;
		if (strcmp(ptr, "cur") == 0) {
		    if (curmsg)
			*curmsg = TocMsgFromId(toc, atoi(ptr2));
		} else if (strcmp(ptr, "all") != 0) {
		    for (i=1 ; i<toc->numsequences ; i++)
			if (strcmp(toc->seqlist[i]->name, ptr) > 0)
			    break;
		    toc->numsequences++;
		    toc->seqlist = (Sequence *)
			XtRealloc((char *) toc->seqlist,
				  (unsigned) toc->numsequences * sizeof(Sequence));
		    for (j=toc->numsequences - 1; j > i ; j--)
			toc->seqlist[j] = toc->seqlist[j-1];
		    seq = toc->seqlist[i] = XtNew(SequenceRec);
		    bzero((char *) seq, sizeof(SequenceRec));
		    seq->name = MallocACopy(ptr);
		    seq->mlist = StringToMsgList(toc, ptr2);
		    if (strcmp(seq->name, viewed) == 0) {
			toc->viewedseq = seq;
			*viewed = 0;
		    }
		}
	    }
	}
	(void) myfclose(fid);
    }
    if (curmsg && *curmsg == NULL && toc->nummsgs > 0)
	*curmsg = toc->msgs[toc->nummsgs - 1];
    for (i = 0; i < oldnumsequences; i++) {
	seq = oldseqs[i];
	if (seq) {
	    if (i > 0) {
		if (TocCreateNullSequence(toc, seq->name) &&
		    strcmp(seq->name, viewed) == 0) {
		    toc->viewedseq = TocGetSeqNamed(toc, seq->name);
		    *viewed = 0;
		}
	    }
	    XtFree((char *) seq->name);
	    if (seq->mlist) FreeMsgList(seq->mlist);
	    XtFree((char *)seq);
	}
    }
    XtFree((char *) oldseqs);
}



/* Refigure what messages are visible.  Also makes sure we're displaying the
   correct set of seq buttons. */

void TURefigureWhatsVisible(toc)
Toc toc;
{
    MsgList mlist = NULL;
    Msg msg;
    register int     i, w;
    int changed, newval, msgid;
    Sequence seq = toc->viewedseq;

    if (seq != NULL) mlist = seq->mlist;
    w = 0;
    changed = FALSE;
    TocCheckSeqButtons(toc);
    for (i = 0; i < toc->nummsgs; i++) {
	msg = toc->msgs[i];
	msgid = msg->msgid;
	while (mlist && mlist->msglist[w] && mlist->msglist[w]->msgid < msgid
	       && w < mlist->nummsgs)
	    w++;
	newval = (!mlist || (w < mlist->nummsgs &&
			     mlist->msglist[w]->msgid == msgid));
	if (newval != msg->visible) {
	    if (!changed) {
		TocUnsetSelection(toc);
		changed = TRUE;
	    }
	    msg->visible = newval;
	}
    }
    if (changed) {
	TURefigureTocPositions(toc);
	for (i=0 ; i<toc->num_scrns ; i++) {
	    TocDisableRedisplay(toc->scrn[i]->tocwidget);
	    TURedisplayToc(toc->scrn[i]);
	    TocEnableRedisplay(toc->scrn[i]->tocwidget);
	}
    }
    for (i=0 ; i<toc->num_scrns ; i++)
	TUResetTocLabel(toc->scrn[i]);
}

/*
 * (Re)load the toc from the scanfile.  If reloading, this makes efforts to
 * keep the fates of msgs, and to keep msgs that are being edited.  Note that
 * this routine must know of all places that msg ptrs are stored; it expects
 * them to be kept only in tocs, in scrns, in msg sequences, and in msg
 * handles.
 */

#define SeemsIdentical(msg1, msg2) ((msg1)->msgid == (msg2)->msgid &&	      \
				    ((msg1)->temporary || (msg2)->temporary ||\
			       strcmp((msg1)->origbuf, (msg2)->origbuf) == 0))

#define SCAN_NONE "scan: no messages in"
#define SCAN_EMPTY ": empty"
void TULoadTocFile(toc)
  Toc toc;
{
    int maxmsgs, l, orignummsgs, origcurmsgid;
    static int	failures = 0;
    int msgid;
    register int i, j;
    FILE *fid;
    XmTextPosition position;
    char *ptr, *ptr2;
    Msg msg, curmsg, lastmsg;
    Msg *origmsgs;

    if (toc->updatepending)
	return;
    toc->updatepending = True;
    DisableEnablingOfButtons();
    TocStopUpdate(toc);
    TocUnsetSelection(toc);
    toc->lastreaddate = LastModifyDate(toc->path);
    if (toc->curmsg) {
	origcurmsgid = toc->curmsg->msgid;
	TocSetCurMsg(toc, (Msg)NULL);
    } else origcurmsgid = 0;
    fid = myfopen(toc->scanfile, "r");
    if (fid == NULL) {
	TUScanFileForToc(toc);
	fid = FOpenAndCheck(toc->scanfile, "r");
    }
    maxmsgs = 10;
    orignummsgs = toc->nummsgs;
    toc->nummsgs = 0;
    origmsgs = toc->msgs;
    toc->msgs = (Msg *) XtMalloc((unsigned) maxmsgs * sizeof(Msg));
    position = 0;
    i = 0;
    curmsg = NULL;
    lastmsg = NULL;
    while (ptr = ReadLineWithCR(fid)) {
	if (*ptr == NULL) continue;
	if (strncmp(ptr, SCAN_NONE, strlen(SCAN_NONE)) == 0) continue;
	if (strstr(ptr, SCAN_EMPTY, strlen(SCAN_EMPTY)) != NULL) continue;
/* PJS: If this line does not have a msg-number in the right place, then
 * try to rescan once, else give a warning message.
 */
	msgid = atoi(ptr + ScanIDCols[0]);
	if (msgid <= 0 || (lastmsg && msgid <= lastmsg->msgid)) {
	    if (failures == 0) {
		/* Something screwy happened; rescan and start again. */
		DEBUG(("Inconsistant cache found; rescanning.\n"));
		(void) myfclose(fid);
/* PJS: Set the failure-to-read-scan flag. */
		failures = 1;
		for (i=0 ; i<toc->nummsgs ; i++)
		    MsgFree(toc->msgs[i]);
		XtFree((char *) toc->msgs);
		toc->msgs = origmsgs;
		toc->nummsgs = orignummsgs;
		toc->updatepending = False;
		TUScanFileForToc(toc);
		TULoadTocFile(toc);
		TocStartUpdate(toc);
		EnableEnablingOfButtons();
		return;
	    } else {
		ptr2 = XtMalloc(strlen(ptr) + strlen("Scan Failure: ")+1);
		sprintf(ptr2,"Scan Failure: %s",ptr);
		Warning(toplevel,NULL,ptr2);
		XtFree(ptr2);
		continue;
	    }
	}
	toc->msgs[toc->nummsgs++] = msg = MsgMalloc();
	l = strlen(ptr);
	msg->toc = toc;
	msg->position = position;
	msg->length = l;
	msg->curbuf = msg->origbuf = MallocACopy(ptr);
	msg->msgid = msgid;
	if (msg->msgid == origcurmsgid)
	    curmsg = msg;
	position++;
	msg->changed = FALSE;
	msg->fate = Fignore;
	msg->desttoc = NULL;
	msg->visible = TRUE;
	msg->msgtype = MTunknown;
	if (toc->nummsgs >= maxmsgs) {
	    maxmsgs += 100;
	    toc->msgs = (Msg *) XtRealloc((char *) toc->msgs,
					  (unsigned) maxmsgs * sizeof(Msg));
	}
	while (i < orignummsgs && origmsgs[i]->msgid < msg->msgid) i++;
	if (i < orignummsgs && SeemsIdentical(origmsgs[i], msg)) {
	    MsgSetFate(msg, origmsgs[i]->fate, origmsgs[i]->desttoc);
	    if (origmsgs[i]->handle) {
		msg->handle = origmsgs[i]->handle;
		msg->handle->msg = msg;
		origmsgs[i]->handle = NULL;
	    }
	}
	lastmsg = msg;
    }

/* PJS: Reset the failure-to-read-scan flag. */
    failures = 0;
    toc->lastPos = position;
    toc->msgs = (Msg *) XtRealloc((char *) toc->msgs,
				  (unsigned) toc->nummsgs * sizeof(Msg));
    (void) myfclose(fid);

    for (i=0 ; i<numScrns ; i++) {
	msg = scrnList[i]->msg;
	if (msg && msg->toc == toc) {
	    for (j=0 ; j<toc->nummsgs ; j++) {
		if (SeemsIdentical(toc->msgs[j], msg)) {
		    msg->position = toc->msgs[j]->position;
		    msg->visible = TRUE;
		    ptr = toc->msgs[j]->curbuf;
		    ptr2 = toc->msgs[j]->origbuf;
		    *(toc->msgs[j]) = *msg;
		    msg->anno = NULL;  /* aj 01 */
		    toc->msgs[j]->curbuf = ptr;
		    toc->msgs[j]->origbuf = ptr2;
		    scrnList[i]->msg = toc->msgs[j];
		    break;
		}
	    }
	    if (j >= toc->nummsgs) {
		DEBUG(("Adding message\n"));
		msg->temporary = FALSE;	/* Don't try to auto-delete msg. */
		MsgSetScrnForce(msg, (Scrn) NULL);
	    }
	}
	if (scrnList[i]->toc == toc) {
	    TURedisplayToc(scrnList[i]);
	    TUResetTocLabel(scrnList[i]);
	}
    }

    for (i=0 ; i<orignummsgs ; i++)
	MsgFree(origmsgs[i]);
    XtFree((char *)origmsgs);
    TULoadSeqLists(toc, &msg);
    TocCheckSeqButtons(toc);
    if (curmsg == NULL) curmsg = msg;
    TocSetCurMsg(toc, curmsg);
    toc->updatepending = False;
    TocStartUpdate(toc);
    EnableEnablingOfButtons();
}


void TUSaveTocFile(toc)
  Toc toc;
{
    extern long lseek();
    Msg msg;
    int fid;
    register int i;
    XmTextPosition position;
    off_t offset_t;
    if (toc->validity != valid) return;
    if (toc->stopupdate) {
	toc->needscachesave = TRUE;
	return;
    }
    fid = -1;
    position = 0;
    for (i = 0; i < toc->nummsgs; i++) {
	msg = toc->msgs[i];
	if (fid < 0 && msg->changed) {
	    fid = myopen(toc->scanfile, O_RDWR, 0666);
	    offset_t = position;
	    (void) lseek(fid, offset_t,  0); /* 64bit port */
	}
	if (fid >= 0) {
	    (void) write(fid, msg->origbuf, msg->length);
	}
	position += msg->length;
    }
    if (fid < 0)
	fid = myopen(toc->scanfile, O_RDWR, 0666);
    if (fid >= 0) {
	(void) ftruncate(fid, position);
	(void) myclose(fid);
    } else
	(void) utime(toc->scanfile, (time_t *)NULL);
    toc->needscachesave = FALSE;
    toc->lastreaddate = LastModifyDate(toc->path);
}


void TUEnsureScanIsValidAndOpen(toc)
  Toc toc;
{
    if (toc) {
	TUGetFullFolderInfo(toc);
	if (toc->validity == invalid || TUScanFileOutOfDate(toc)) {
	    TUScanFileForToc(toc);
	    TULoadTocFile(toc);
	}
	toc->validity = valid;
    }
}



/* Refigure all the positions, based on which lines are visible. */

void TURefigureTocPositions(toc)
  Toc toc;
{
    register int i;
    Msg msg;
    int position;
    position = 0;
    for (i=0; i<toc->nummsgs ; i++) {
	msg = toc->msgs[i];
	msg->position = position;
	if (msg->visible) position++;
    }
    toc->lastPos = position;
}



/* Make sure we've loaded ALL the folder info for this toc, including its
   path and sequence lists. */

void TUGetFullFolderInfo(toc)
  Toc toc;
{
    char str[500];
    if (toc->scanfile == NULL || toc->path == NULL) {
	if (toc->path == NULL) {
	    (void) sprintf(str, "%s/%s", mailDir, toc->foldername);
	    toc->path = MallocACopy(str);
	}
	if (toc->scanfile == NULL) {
	(void) sprintf(str, "%s/.decxmailcache", toc->path);
	    toc->scanfile = MallocACopy(str);
	}
	toc->lastreaddate = LastModifyDate(toc->scanfile);
	if (TUScanFileOutOfDate(toc)) {
	    DEBUG(("TUGetFullFolderInfo setting invalid\n"));
	    toc->validity = invalid;
	} else {
	    toc->validity = valid;
	    TULoadTocFile(toc);
	}
    }
}

/* Append a message to the end of the toc.  It has the given scan line.  This
   routine will figure out the message number, and change the scan line
   accordingly. */

Msg TUAppendToc(toc, ptr)
  Toc toc;
  char *ptr;
{
    char str[10], format[10];
    Msg msg;
    register int msgid, i, j;
    int addPos;
    TagRec msgTag[1];
    Arg arg[1];

    TUGetFullFolderInfo(toc);
    if (toc->validity != valid)
	return NULL;
	    
    TocUnsetSelection(toc);
    if (toc->nummsgs > 0)
	msgid = toc->msgs[toc->nummsgs - 1]->msgid + 1;
    else
	msgid = 1;
    (toc->nummsgs)++;
    toc->msgs = (Msg *) XtRealloc((char *) toc->msgs,
				  (unsigned) toc->nummsgs * sizeof(Msg));
    toc->msgs[toc->nummsgs - 1] = msg = MsgMalloc();
    msg->toc = toc;
    msg->curbuf = msg->origbuf = MallocACopy(ptr);
    (void) sprintf(format, "%%%dd", ScanIDCols[1]);
    (void)sprintf(str, format, msgid);
    for (i=0; i<ScanIDCols[1] ; i++)
	msg->origbuf[i + ScanIDCols[0]] = str[i];
    msg->msgid = msgid;
    msg->position = toc->lastPos;
    msg->length = strlen(ptr);
    msg->changed = TRUE;
    msg->fate = Fignore;
    msg->desttoc = NULL;
    msg->msgtype = MTunknown;
    if (toc->viewedseq == toc->seqlist[0]) {
	msg->visible = TRUE;
	toc->lastPos += msg->length;
    }
    else
	msg->visible = FALSE;

    TUSaveTocFile(toc);
    for (i=0 ; i<toc->num_scrns ; i++) {
	msgTag[0].tagFields.tocNumber = TUGetTocNumber(toc);
	msgTag[0].tagFields.msgNumber = toc->nummsgs;
	TocDisableRedisplay(toc->scrn[i]->tocwidget);
	if (!defSVNStyle) {
	    XtSetArg(arg[0], DXmSvnNnumberOfEntries, &addPos);
	    XtGetValues(toc->scrn[i]->tocwidget, arg, 1);
	} else {
	    addPos = toc->index - 1;
	    for (j = 0; j < toc->nummsgs; j++)  {
		if (toc->msgs[j]->visible) addPos++;
	    }
	}
	DXmSvnAddEntries(toc->scrn[i]->tocwidget, addPos, 1,
		(defSVNStyle ? toc->level+1 : 0), msgTag, True);
	TocEnableRedisplay(toc->scrn[i]->tocwidget);
	TUResetTocLabel(toc->scrn[i]);
    }
    return msg;
}
