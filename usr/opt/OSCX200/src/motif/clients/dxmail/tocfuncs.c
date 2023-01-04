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
static char rcs_id[] = "@(#)$RCSfile: tocfuncs.c,v $ $Revision: 1.1.5.5 $ (DEC) $Date: 1993/12/21 23:59:54 $";
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

/* tocfuncs.c -- handle things in the toc widget. */

#include "decxmail.h"
#include <Xm/TextP.h>
#include "msg.h"
#include "mlist.h"
#include "radio.h"
#include "toc.h"
#include <DXm/DXmPrint.h>
#ifndef DXmNunmanageOnOk
#define DXmNunmanageOnOk "unmanageOnOk"
#define DXmNunmanageOnCancel "unmanageOnCancel"
#endif


static void printStrippedJob();

void ExecNextView(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    Toc toc = scrn->toc;
    MsgList mlist;
    FateType fate;
    Msg msg;
    TRACE(("@ExecNextView\n"));
    if (toc == NULL) return;
    mlist = TocCurMsgList(toc,scrn);
    if (mlist->nummsgs)
	msg = mlist->msglist[0];
    else {
	msg = TocGetCurMsg(toc);
	if (msg && msg == scrn->msg) msg = TocMsgAfter(toc, msg);
	if (msg) fate = MsgGetFate(msg, (Toc *)NULL);
	while (msg && ((SkipDeleted && fate == Fdelete)
		|| (SkipMoved && fate == Fmove))) {
	    msg = TocMsgAfter(toc, msg);
	    if (msg) fate = MsgGetFate(msg, (Toc *)NULL);
	}
    }
    if (msg) {
	if (!MsgSetScrn(msg, scrn)) {
	    TocUnsetSelection(toc);
	    TocSetCurMsg(toc, msg);
	}
    }
    FreeMsgList(mlist);
}



void ExecPrevView(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    Toc toc = scrn->toc;
    MsgList mlist;
    FateType fate;
    Msg msg;
    TRACE(("@ExecPrevView\n"));
    if (toc == NULL) return;
    mlist = TocCurMsgList(toc, scrn);
    if (mlist->nummsgs)
	msg = mlist->msglist[mlist->nummsgs - 1];
    else {
	msg = TocGetCurMsg(toc);
	if (msg && msg == scrn->msg) msg = TocMsgBefore(toc, msg);
	if (msg) fate = MsgGetFate(msg, (Toc *)NULL);
	while (msg && ((SkipDeleted && fate == Fdelete)
		|| (SkipMoved && fate == Fmove))) {
	    msg = TocMsgBefore(toc, msg);
	    if (msg) fate = MsgGetFate(msg, (Toc *)NULL);
	}
    }
    if (msg) {
	if (!MsgSetScrn(msg, scrn)) {
	    TocUnsetSelection(toc);
	    TocSetCurMsg(toc, msg);
	}
    }
    FreeMsgList(mlist);
}



/* ARGSUSED */
void ExecViewNew(w, event, params, num_params)
Widget w;
XEvent *event;
char **params;
Cardinal *num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    Toc toc = scrn->toc;
    Scrn vscrn;
    MsgList mlist;
    TRACE(("@ExecViewNew\n"));
    if (toc == NULL) return;
    mlist = CurMsgListOrCurMsg(toc,scrn);
    if (mlist->nummsgs) {
	vscrn = CreateReadScrn(params[0]);
	if (!MsgSetScrn(mlist->msglist[0], vscrn)) {
	    MapScrn(vscrn, 0);
	}
    }
    FreeMsgList(mlist);
}

/* ARGSUSED */
void ExecViewInSpecified(w, event, params, num_params)
Widget w;
XEvent *event;
char **params;
Cardinal *num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    Toc toc = scrn->toc;
    char *title;
    register int i, n;
    MsgList mlist;
    TRACE(("@ExecViewInSpecified\n"));
    if (toc == NULL) return;
    mlist = CurMsgListOrCurMsg(toc, scrn);
    if (mlist->nummsgs) {
	title = RadioGetCurrent(ReadWindowsRadio);
	while (*title && (*title < '0' || *title > '9'))
	    title++;
	n  = atoi(title);
	if (n <= 0) return;
	for (i=0 ; i<numScrns ; i++) {
	    if (scrnList[i]->readnum == n) {
		(void) MsgSetScrn(mlist->msglist[0], scrnList[i]);
		break;
	    }
	}
    }
    if (mlist) FreeMsgList(mlist);
}



Scrn GetDefaultViewScrn()
{
    register int i;
    for (i=0 ; i<numScrns ; i++) {
	if (scrnList[i]->mapped && scrnList[i]->readnum == 1)
	    return scrnList[i];
    }
    for (i=0; i<numScrns; i++) {
	if (scrnList[i]->readnum == 1)
	    return scrnList[i];
    }
    return NULL;
}


/* ARGSUSED */
void ExecViewInDefault(w, event, params, num_params)
Widget w;
XEvent *event;
char **params;
Cardinal *num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    Toc toc = scrn->toc;
    Scrn vscrn;
    MsgList mlist;
    TRACE(("@ExecViewInDefault\n"));
    mlist = CurMsgListOrCurMsg(toc, scrn);
    if (mlist->nummsgs) {
	vscrn = GetDefaultViewScrn();
	if (vscrn == NULL)
	    vscrn = CreateReadScrn(params[0]);
	if (!MsgSetScrn(mlist->msglist[0], vscrn)) {
	    MapScrn(vscrn, 0);
	}
	TocRedoButtonPixmaps(toc);
    }
    if (mlist) FreeMsgList(mlist);
}


/* ARGSUSED */
void ExecViewMsg(w, msg)
Widget w;
Msg msg;
{
    Scrn scrn = ScrnFromWidget(w);
    Toc toc = scrn->toc;
    Scrn vscrn;
    TRACE(("@ExecViewMsg\n"));
    /*
     * This check has been put here to prevent crashes due to bad msg address.
     * This fix was based on the stack trace from a core file created by non
     * a non reproducable crash. After this fix, it does not crash during soak.
     * This check has to be removed once the actual problem has been traced.
     */
    if (msg == NULL || msg == (Msg) 0x1) {
      DEBUG(("tocfuncs.c: ExecViewMsg(): Invalid msg address.\n"));
      return;
    }
    vscrn = GetDefaultViewScrn();
    if (vscrn == NULL) {
	vscrn = CreateReadScrn("read");
    }
    if (!MsgSetScrn(msg, vscrn)) {
	MapScrn(vscrn, 0);
    }
    TocRedoButtonPixmaps(toc);
}


/* ARGSUSED */
void ExecTocForward(w, event, params, num_params)
Widget w;
XEvent *event;
char **params;
Cardinal *num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    Toc toc = scrn->toc;
    MsgList mlist;
    TRACE(("@ExecTocForward\n"));
    if (toc == NULL) return;
    mlist = CurMsgListOrCurMsg(toc, scrn);
    if (mlist->nummsgs)
	CreateForward(mlist, params[0], event->xbutton.time);
    FreeMsgList(mlist);
}


/* ARGSUSED */
void ExecTocUseAsComposition(w, event, params, num_params)
Widget w;
XEvent *event;
char **params;
Cardinal *num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    Toc toc = scrn->toc;
    Scrn vscrn;
    MsgList mlist;
    Msg msg;
    FILE * fid;
    int i;
    Widget  tw;

    TRACE(("@ExecTocUseAsComposition\n"));
    if (toc == NULL) return;
    mlist = CurMsgListOrCurMsg(toc, scrn);
    if (mlist->nummsgs) {
	vscrn = CreateNewScrn(params[0]);
        tw = vscrn->viewwidget;
        if ( vscrn->ddifheaders && XtIsManaged(vscrn->ddifheaders) )
            tw = vscrn->ddifheaders;
	ResetComposeScrnWordWrapSetting(tw);
	for (i=0 ; i<mlist->nummsgs ; i++) {
	  fid = myfopen(MsgFileName(mlist->msglist[i]), "r");
	  if (fid) {
	    if (checkForPS(fid)) {
	      StopComposeWordWrap(tw);
	      break;
	    }
	  }
	}
	
        if (DraftsFolder == toc) {
	    msg = mlist->msglist[0];
	} else {
	    msg = TocMakeNewMsg(DraftsFolder);
	    MsgLoadCopy(msg, mlist->msglist[0]);
	    MsgSetTemporary(msg);
	}
	(void)MsgSetScrnForComp(msg, vscrn);
	MapScrn(vscrn, event->xbutton.time);
    }
    FreeMsgList(mlist);
}




/* Utility: change the fate of the selected messages in the scrn. */

static MarkMessages(scrn, fate)
Scrn scrn;
FateType fate;
{
    Toc toc = scrn->toc;
    Toc desttoc;
    MsgList mlist;
    if (toc == NULL) return;
    if (fate == Fmove)
	desttoc = SelectedToc(scrn);
    else
	desttoc = NULL;
    TocStopUpdate(toc);
    mlist = CurMsgListOrCurMsg(toc, scrn);
    if (desttoc == toc || (desttoc == NULL && fate == Fmove))
	MakeMoveOrCopyDialog(scrn, mlist, FALSE);
    else
	MsgBatchSetFate(mlist, fate, desttoc);
    FreeMsgList(mlist);
    TocStartUpdate(toc);
}



void ExecMarkDelete(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    TRACE(("@ExecMarkDelete\n"));
    MarkMessages(scrn, Fdelete);
/* PJS: Grey out the DELETE button if we have deleted these msgs... */
    ForceEnableScrnsDeleteButtons();
}



void ExecMarkCopy(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    Toc toc = scrn->toc;
    Toc desttoc = SelectedToc(scrn);
    MsgList mlist = TocCurMsgList(toc, scrn);
    TRACE(("@ExecMarkCopy\n"));
    if (desttoc != NULL && desttoc != toc)
	TocCopyMessages(toc, mlist, desttoc);
    else
	MakeMoveOrCopyDialog(scrn, mlist, TRUE);
    FreeMsgList(mlist);
}


void ExecMarkMove(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    TRACE(("@ExecMarkMove\n"));
    MarkMessages(scrn, Fmove);
}


void ExecMarkCopyDialog(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    MsgList mlist = TocCurMsgList(scrn->toc, scrn);
    TRACE(("@ExecMarkCopyDialog\n"));
    MakeMoveOrCopyDialog(scrn, mlist, TRUE);
    FreeMsgList(mlist);
}


void ExecMarkMoveDialog(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    MsgList mlist = TocCurMsgList(scrn->toc, scrn);
    TRACE(("@ExecMarkMoveDialog\n"));
    MakeMoveOrCopyDialog(scrn, mlist, FALSE);
    FreeMsgList(mlist);
}


void ExecMarkUnmarked(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    TRACE(("@ExecMarkUnmarked\n"));
    MarkMessages(scrn, Fignore);
/* PJS: Ensure that the DELETE button is active again... */
    ForceEnableScrnsDeleteButtons();
}


void ExecCommitChanges(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    TRACE(("@ExecCommitChanges\n"));
    if (scrn->toc == NULL) return;
    TocCommitChanges(scrn->toc);
}


/* ARGSUSED */
void ExecEmptyWastebasket(w)
Widget w;
{
    TRACE(("@ExecEmptyWastebasket\n"));
    if (defUseWastebasket)
	TocEmptyWastebasket();
}

/* ARGSUSED */
static void printJob(w, event, reason)
Widget w;
XEvent *event;
XmAnyCallbackStruct reason;
{
    XmStringTable files;
    int		fileCount = 0;
    Arg		jobArgs[4];
    int		ac;

    if (w == (Widget) NULL)
	return;
    ac = 0;
    XtSetArg(jobArgs[ac], DXmNfileNameCount, &fileCount);ac++;
    XtSetArg(jobArgs[ac], DXmNfileNameList, &files);ac++;
    XtGetValues(w, jobArgs, ac);
    DXmPrintWgtPrintJob(w, files, fileCount); 
#ifdef notdef
    /* need to free the file list? */
#endif
}

void ExecPrintMessages(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    Toc toc = scrn->toc;
    MsgList mlist;
    char str[200],  check_prnt[100];
    register int i;
    TRACE(("@ExecPrintMessages\n"));
    if (toc == NULL) return;
    BeginLongOperation();
    mlist = CurMsgListOrCurMsg(toc, scrn);

    (void) sprintf(check_prnt, "%s", defPrintCommand); /* AJ 01 */

    if (strncmp(check_prnt , PRINTDEFAULT, strlen(PRINTDEFAULT)) == 0)
      {
	(void) sprintf(check_prnt, "%s", defPrintStrippedCommand);
	if ( strncmp ( check_prnt ,STRIPPEDPRINTDEFAULT,
		      strlen(STRIPPEDPRINTDEFAULT)) != 0)
	  (void) sprintf(check_prnt, "%s", defPrintStrippedCommand);
      }

    for (i = 0; i < mlist->nummsgs; i++) {
	(void) sprintf(str, "%s %s", check_prnt,
		MsgFileName(mlist->msglist[i]));
	(void) system(str);
    }
    FreeMsgList(mlist);
    EndLongOperation();
}


void ExecPrintWidget(w)
Widget w;
{
    Scrn 	    scrn = ScrnFromWidget(w);
    Toc 	    toc = scrn->toc;
    Msg		    msg = scrn->msg;
    MsgList 	    mlist;
    register int    i;
    XmStringTable   files;
    Arg		    printArgs[10];
    int		    ac;
    static Widget   printWidget = (Widget) NULL;
    static XtCallbackRec printCallback[2] = {(XtCallbackProc)printJob, 
					       NULL, NULL, NULL};

    TRACE(("@ExecPrintWidget\n"));
    if (toc == NULL && msg == NULL) return;
    BeginLongOperation();
    if (toc == NULL) {
	files = (XmStringTable) XtMalloc(sizeof (XmString));
	files[0] = XmStringCreateSimple(MsgFileName(msg));
	ac = 0;
	XtSetArg(printArgs[ac], DXmNunmanageOnOk, True);ac++;
	XtSetArg(printArgs[ac], DXmNunmanageOnCancel, True);ac++;
	XtSetArg(printArgs[ac], DXmNfileNameList, files);ac++;
	XtSetArg(printArgs[ac], DXmNfileNameCount, 1); ac++;
        XtSetArg(printArgs[ac], DXmNsuppressOptionsMask,DXmSUPPRESS_DELETE_FILE); ac++;

	XtSetArg(printArgs[ac], XmNokCallback, printCallback);ac++;
    } else {
	mlist = CurMsgListOrCurMsg(toc, scrn);
	files = (XmStringTable) XtMalloc(mlist->nummsgs * (sizeof (XmString)));
	for (i = 0; i < mlist->nummsgs; i++) {
	    files[i] = XmStringCreateSimple(MsgFileName(mlist->msglist[i]));
	}
	ac = 0;
	XtSetArg(printArgs[ac], DXmNunmanageOnOk, True);ac++;
	XtSetArg(printArgs[ac], DXmNunmanageOnCancel, True);ac++;
	XtSetArg(printArgs[ac], DXmNfileNameList, files);ac++;
	XtSetArg(printArgs[ac], DXmNfileNameCount, mlist->nummsgs); ac++;
        XtSetArg(printArgs[ac], DXmNsuppressOptionsMask,DXmSUPPRESS_DELETE_FILE); ac++;

	XtSetArg(printArgs[ac], XmNokCallback, printCallback);ac++;
    }
    
    if (printWidget == (Widget) NULL) {
	printWidget = DXmCreatePrintDialog(toplevel, "printDialog",
				printArgs, ac);
    } else {
	XtSetValues(printWidget, printArgs, ac);
    }
    XtManageChild(printWidget);
    if (toc == NULL) {
	XmStringFree(files[0]);
    } else {
	for (i = 0; i < mlist->nummsgs; i++) {
	    XmStringFree(files[i]);
	}
	FreeMsgList(mlist);
    }
    XtFree((char *)files);
    EndLongOperation();
}

void stripMessage(from, to)
char *from;
char *to;
{
    FILE *in, *out;
    char *buf;
    Boolean foundHeaderEnd = False;
    Boolean foundFirstNonBlank = False;
    int lineCount = 0;

    in = myfopen(from, "r");
    if (in == NULL) {
	Punt("Can't open message file to print\n");
	return;
    }
    out = myfopen(to, "a");
    if (out == NULL) {
	Punt("Can't open message file to print\n");
	return;
    }

    for(;;) {
	if ((buf = ReadLineWithCR(in)) == NULL) break;
	if (foundHeaderEnd && !foundFirstNonBlank) {
	    if (strlen(buf) > 1) foundFirstNonBlank = True;
	}
	if (foundFirstNonBlank) {
	    /*
	     * Special hack for postscript messages - if the line
	     * starts right, re-open the file, tossing out
	     * the preceeding stuff. But only for the first 10 lines.
	     */

	    if (lineCount < PSLIMIT && strncmp(buf, "%!", 2) == 0) {
		(void) fclose(out);
		out = myfopen(to, "w");
	    }
	    (void) fwrite(buf, 1, strlen(buf), out);
	    lineCount++;
	}
	if (strlen(buf) == 1 && *buf == '\n')
	    foundHeaderEnd = True;
    }
    (void) fclose(in);
    (void) fclose(out);
}
void ExecPrintStripped(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    Toc toc = scrn->toc;
    MsgList mlist;
    char *tempPrintName;
    char *msgName;
    char str[200], check_prnt[100];
    register int i;
    TRACE(("@ExecPrintStripped\n"));
    if (toc == NULL) return;
    BeginLongOperation();
    mlist = CurMsgListOrCurMsg(toc, scrn);
	
    (void) sprintf(check_prnt, "%s", defPrintStrippedCommand); /* AJ 01 */

    if (strncmp(check_prnt , STRIPPEDPRINTDEFAULT, strlen 
		(STRIPPEDPRINTDEFAULT))  == 0)
      {
	(void) sprintf(check_prnt, "%s", defPrintCommand);
	if ( strncmp ( check_prnt , PRINTDEFAULT, strlen(PRINTDEFAULT)) == 0)
	  (void) sprintf(check_prnt, "%s", defPrintStrippedCommand);
      }

    for (i = 0; i < mlist->nummsgs; i++) {
	msgName = MsgFileName(mlist->msglist[i]);
	tempPrintName = tempnam(NULL, "Print");
	stripMessage(msgName, tempPrintName);
	(void) sprintf(str, "%s %s", check_prnt, tempPrintName);
	(void) system(str);
	(void) unlink(tempPrintName);
    }
    FreeMsgList(mlist);
    EndLongOperation();
}

static char **tempPrintFiles;
static int tempPrintCount;

/* ARGSUSED */
static void printStrippedJob(w, event, reason)
Widget w;
XEvent *event;
XmAnyCallbackStruct reason;
{
    XmStringTable files;
    int		fileCount = 0;
    Arg		jobArgs[4];
    int		ac;
    int		i;

    if (w == (Widget) NULL)
	return;
    ac = 0;
    XtSetArg(jobArgs[ac], DXmNfileNameCount, &fileCount);ac++;
    XtSetArg(jobArgs[ac], DXmNfileNameList, &files);ac++;
    XtGetValues(w, jobArgs, ac);
    DXmPrintWgtPrintJob(w, files, fileCount); 
    for (i = 0; i < tempPrintCount; i++) {
/* allow Print Widget to remove the file not DXMail
	(void) unlink(tempPrintFiles[i]);
*/
	XtFree(tempPrintFiles[i]);
    }
    XtFree((char *)tempPrintFiles);
    tempPrintCount = 0;
}


void ExecPrintWidgetStripped(w)
Widget w;
{
    Scrn 	    scrn = ScrnFromWidget(w);
    Toc 	    toc = scrn->toc;
    Msg		    msg = scrn->msg;
    MsgList 	    mlist;
    register int    i;
    XmStringTable   files;
    Arg		    printArgs[15];
    int		    ac;
    static Widget   printWidget = (Widget) NULL;
    static XtCallbackRec printCallback[2] = {(XtCallbackProc)printStrippedJob,
					       NULL, NULL, NULL};
    char *tempPrintName;
    char *msgName;

    TRACE(("@ExecPrintWidgetStripped\n"));
    if (toc == NULL && msg == NULL) return;
    BeginLongOperation();
    tempPrintCount = 0;
    if (toc == NULL) {
	files = (XmStringTable) XtMalloc(sizeof (XmString));
	tempPrintFiles = (char ** ) XtMalloc(sizeof (char *));
	msgName = MsgFileName(msg);
	tempPrintName = tempnam(NULL, "Print");
	stripMessage(msgName, tempPrintName);
	files[0] = XmStringCreateSimple(tempPrintName);
	ac = 0;
	XtSetArg(printArgs[ac], DXmNunmanageOnOk, True);ac++;
	XtSetArg(printArgs[ac], DXmNunmanageOnCancel, True);ac++;
	XtSetArg(printArgs[ac], DXmNfileNameList, files);ac++;
	XtSetArg(printArgs[ac], DXmNfileNameCount, 1); ac++;
        XtSetArg(printArgs[ac], DXmNdeleteFile, True);ac++;
      XtSetArg(printArgs[ac], DXmNsuppressOptionsMask,DXmSUPPRESS_DELETE_FILE); ac++;

	XtSetArg(printArgs[ac], XmNokCallback, printCallback);ac++;
	tempPrintFiles[tempPrintCount++] = XtNewString(tempPrintName);
    } else {
	mlist = CurMsgListOrCurMsg(toc, scrn);
	files = (XmStringTable) XtMalloc(mlist->nummsgs * (sizeof (XmString)));
	tempPrintFiles = (char ** ) XtMalloc(mlist->nummsgs * (sizeof (char *)));
	files = (XmStringTable) XtMalloc(mlist->nummsgs * (sizeof (XmString)));
	for (i = 0; i < mlist->nummsgs; i++) {
	    msgName = MsgFileName(mlist->msglist[i]);
	    tempPrintName = tempnam(NULL, "Print");
	    stripMessage(msgName, tempPrintName);
	    files[i] = XmStringCreateSimple(tempPrintName);
	    tempPrintFiles[tempPrintCount++] = XtNewString(tempPrintName);
	}
	ac = 0;
	XtSetArg(printArgs[ac], DXmNunmanageOnOk, True);ac++;
	XtSetArg(printArgs[ac], DXmNunmanageOnCancel, True);ac++;
	XtSetArg(printArgs[ac], DXmNfileNameList, files);ac++;
	XtSetArg(printArgs[ac], DXmNfileNameCount, mlist->nummsgs); ac++;
        XtSetArg(printArgs[ac], DXmNdeleteFile, True);ac++;
      XtSetArg(printArgs[ac], DXmNsuppressOptionsMask,DXmSUPPRESS_DELETE_FILE); ac++;

	XtSetArg(printArgs[ac], XmNokCallback, printCallback);ac++;
    }
    
    if (printWidget == (Widget) NULL) {
	printWidget = DXmCreatePrintDialog(toplevel, "printDialog",
				printArgs, ac);
    } else {
	XtSetValues(printWidget, printArgs, ac);
    }
    XtManageChild(printWidget);
    if (toc == NULL) {
	XmStringFree(files[0]);
    } else {
	for (i = 0; i < mlist->nummsgs; i++) {
	    XmStringFree(files[i]);
	}
	FreeMsgList(mlist);
    }
    XtFree((char *)files);
    EndLongOperation();
}

void ExecPack(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    Toc toc = scrn->toc;
    TRACE(("@ExecPack\n"));
    if (AutoCommit)
	TocCommitChanges(toc);
    TocPack(toc);
}



void ExecSort(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    Toc toc = scrn->toc;
    char **argv, str[100];
    TRACE(("@ExecSort\n"));
    if (toc == NULL) return;
    if (TocConfirmCataclysm(toc)) return;
    TocSaveCurMsg(toc);
    argv = MakeArgv(3);
    argv[0] = sortmCmd;
    (void) sprintf(str, "+%s", TocGetFolderName(toc));
    argv[1] = str;
    argv[2] = "-noverbose";
    DoCommand(argv, (char *) NULL, "/dev/null");
    XtFree((char *) argv);
    TocForceRescan(toc);
}




void ExecForceRescan(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    Toc toc = scrn->toc;
    TRACE(("@ExecForceRescan\n"));
    if (toc == NULL) return;
    TocSaveCurMsg(toc);
    TocForceRescan(toc);
}



/* Incorporate new mail. */

void ExecIncorporate(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    Toc toc;
    Msg msg;
    register int i;
    TRACE(("@ExecIncorporate\n"));
    BeginLongOperation();
    toc = scrn->toc;
    if (toc == NULL || !TocCanIncorporate(toc)) {
	toc = InitialFolder;
    }
    TocStopUpdate(toc);
    msg = TocIncorporate(toc);
    if (msg) {
	if (toc != scrn->toc) {
	    if (scrn->toc != NULL && AutoCommit)
		TocCommitChanges(scrn->toc);

	    TocStartUpdate(toc);
	    TocSetScrn(toc, scrn);
	    TocStopUpdate(toc);
	}
	TocMakeVisible(toc, msg);
    } 
    TocStartUpdate(toc);
    for (i=0 ; i<numScrns ; i++)
	EnableProperButtons(scrnList[i]);
    TocCheckForNewMail();
    EndLongOperation();
}


/* Read new mail, if any. */

/* ARGSUSED */
void ExecReadNewMail(w, event, params, num_params)
Widget w;
XEvent *event;
char **params;
Cardinal *num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    Toc toc;
    Scrn vscrn;
    Msg msg;
    TRACE(("@ExecReadNewMail\n"));
    BeginLongOperation();
    if (TocCanIncorporate(scrn->toc))
	toc = scrn->toc;
    else
	toc = InitialFolder;
    if (toc != scrn->toc) {
	if (!defSVNStyle) {
	    RadioSetCurrent(scrn->folderradio, TocGetFolderName(toc));
	    RadioSetOpened(scrn->folderradio, TocGetFolderName(toc));
	}
	ExecOpenFolder(w);
    }	
    TocStopUpdate(toc);
    msg = TocIncorporate(toc);
    if (msg) {
	vscrn = GetDefaultViewScrn();
	if (vscrn == NULL)
	    vscrn = CreateReadScrn(params[0]);
	if (defDefaultViewBackwards)
	    msg = TocGetLastMsg(toc);
	if (!MsgSetScrn(msg, vscrn)) {
	    MapScrn(vscrn, 0);
	}
    } else
	NoNewMailWarning(scrn);
    TocStartUpdate(toc);
    TocCheckForNewMail();
    EndLongOperation();
}


/* Read new mail (if any) in a new view window. */

/* ARGSUSED */
void ExecReadNewMailInNew(w, event, params, num_params)
Widget w;
XEvent *event;
char **params;
Cardinal *num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    Msg msg;
    Scrn vscrn;
    Toc toc;
    TRACE(("@ExecReadNewMailInNew\n"));
    BeginLongOperation();
    if (TocCanIncorporate(scrn->toc))
	toc = scrn->toc;
    else
	toc = InitialFolder;
    msg = TocIncorporate(toc);
    if (msg) {
	if (defDefaultViewBackwards)
	    msg = TocGetLastMsg(toc);
	vscrn = CreateReadScrn(params[0]);
	if (!MsgSetScrn(msg, vscrn)) {
	    MapScrn(vscrn, 0);
	}
    } else
	NoNewMailWarning(scrn);
    TocCheckForNewMail();
    EndLongOperation();
}


void ExecShowUnseen(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    Toc toc = scrn->toc;
    TRACE(("@ExecShowUnseen\n"));
    if (toc == NULL || unseenSeqName == NULL || !TocCanIncorporate(toc))
	return;
    TocStopUpdate(toc);
    if (defIncOnShowUnopened)
	(void) TocIncorporate(toc);
    TocChangeViewedSeq(toc, TocGetSeqNamed(toc, unseenSeqName));
    TocStartUpdate(toc);
    TocCheckForNewMail();
}
    

/* ARGSUSED */
void ExecTocReply(w, event, params, num_params)
Widget w;
XEvent *event;
char **params;
Cardinal *num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    Toc toc = scrn->toc;
    Scrn nscrn;
    MsgList mlist;
    Msg msg;
    TRACE(("@ExecTocReply\n"));
    if (toc == NULL) return;
    mlist = CurMsgListOrCurMsg(toc, scrn );
    if (mlist->nummsgs) {
	nscrn = CreateNewScrn(params[0]);
	msg = TocMakeNewMsg(DraftsFolder);
	MsgSetTemporary(msg);
	MsgLoadReply(msg, mlist->msglist[0]);
	(void)MsgSetScrnForComp(msg, nscrn);
	MapScrn(nscrn, event->xbutton.time);
    }
    FreeMsgList(mlist);
}


void ExecPick(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    Toc toc = scrn->toc;
    TRACE(("@ExecPick\n"));
    if (toc == NULL) return;
    CreatePick(TocGetFolderName(toc), TocViewedSequence(toc)->name, "temp");
}


void ExecPickInSelected(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    Toc toc = SelectedToc(scrn);
    TRACE(("@ExecPickInSelected\n"));
    if (toc == NULL) {
	toc = scrn->toc;
	if (toc == NULL) return;
    }
    CreatePick(TocGetFolderName(toc), "all", "temp");
}


void ExecOpenSeq(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    Toc toc = scrn->toc;
    TRACE(("@ExecOpenSeq\n"));
    if (toc == NULL) return;
    TocChangeViewedSeq(toc, TocGetSeqNamed(toc, SelectedSeqName(scrn)));
}



void TwiddleSequence(scrn, mlist, op)
Scrn scrn;
MsgList mlist;
TwiddleOperation op;
{
    Toc toc;
    char **argv, plus[100], str[100], seqname[100];
    register int i;
    toc = scrn->toc;
    if (toc == NULL) toc = MsgGetToc(scrn->msg);
    if (toc == NULL) return;
    (void) strcpy(seqname, SelectedSeqName(scrn));
    if (strcmp(seqname, "all") == 0) {
	Error(scrn->parent, ESequenceNoAll, (char *) NULL);
	return;
    }
    if (op != DELETE && mlist->nummsgs == 0) {
	FreeMsgList(mlist);
	Feep();
	return;
    }
    BeginLongOperation();
    argv = MakeArgv(6 + mlist->nummsgs);
    argv[0] = markCmd;
    (void) sprintf(plus, "+%s", TocGetFolderName(toc));
    argv[1] = plus;
    argv[2] = "-sequence";
    argv[3] = seqname;
    switch (op) {
      case ADD:
	argv[4] = "-add";
	argv[5] = "-nozero";
	break;
      case REMOVE:
	argv[4] = "-delete";
	argv[5] = "-nozero";
	break;
      case DELETE:
	argv[4] = "-delete";
	argv[5] = "all";
	break;
    }
    for (i = 0; i < mlist->nummsgs; i++) {
	(void) sprintf(str, "%d", MsgGetId(mlist->msglist[i]));
	argv[6 + i] = MallocACopy(str);
    }
    DoCommand(argv, (char *) NULL, "/dev/null");
    for (i = 0; i < mlist->nummsgs; i++)
        XtFree((char *) argv[6 + i]);
    XtFree((char *) argv);
    FreeMsgList(mlist);
    TocReloadSeqLists(toc);
    if (op == DELETE)
	(void) TocDeleteNullSequence(toc, seqname);
    TocRedoButtonPixmaps(toc);
    EndLongOperation();
}

    


void ExecAddToSeq(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    TRACE(("@ExecAddToSeq\n"));
    if (scrn->toc)
	TwiddleSequence(scrn, CurMsgListOrCurMsg(scrn->toc, scrn), ADD);
}



void ExecRemoveFromSeq(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    TRACE(("@ExecRemoveFromSeq\n"));
    if (scrn->toc)
	TwiddleSequence(scrn, CurMsgListOrCurMsg(scrn->toc, scrn), REMOVE);
}



void ExecDeleteSeq(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    TRACE(("@ExecDeleteSeq\n"));
    TwiddleSequence(scrn, MakeNullMsgList(), DELETE);
}


static Toc createtoc;

static Boolean CreateSequence(name, scrn)
char *name;
Scrn scrn;
{
    if (!TocCreateNullSequence(createtoc, name)) {
	Error(scrn->parent, ESequenceCantCreate, name);
	return FALSE;
    }
    return TRUE;
}


void ExecCreateSeq(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    TRACE(("@ExecCreateSeq\n"));
    createtoc = scrn->toc;
    if (createtoc)
	MakePrompt(scrn, "createSeq", CreateSequence, "",
		   "OnWindow Picking Sequences");
}



/* ARGSUSED */
void ExecSelectAll(w, event, params, num_params)
Widget w;
XEvent *event;
char **params;
Cardinal *num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    Widget widget;
    XmTextSource source;
    XmTextPosition first, last;
    widget = scrn->tocwidget;
    TRACE(("@ExecSelectAll\n"));
    if (widget == NULL) {
	widget = scrn->viewwidget;
        if ( scrn->ddifheaders && XtIsManaged(scrn->ddifheaders) )
            widget = scrn->ddifheaders;
	source = (XmTextSource) XmTextGetSource(widget);
	first = (*source->Scan)(source, 0, XmSELECT_ALL, XmsdLeft, 1, TRUE);
	last = (*source->Scan)(source, 0, XmSELECT_ALL, XmsdRight, 1, TRUE);
	(*source->SetSelection)(source, first, last, event->xbutton.time);
    } else {
	TocDisableRedisplay(widget);
	DXmSvnSelectAll(widget);
	TocEnableRedisplay(widget);
    }
}
