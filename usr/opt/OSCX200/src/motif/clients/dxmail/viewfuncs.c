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
static char rcs_id[] = "@(#)$RCSfile: viewfuncs.c,v $ $Revision: 1.1.6.8 $ (DEC) $Date: 1993/12/22 00:00:00 $";
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

/* viewfuncs.c -- handle viewing of a message */

#include "decxmail.h"
#include <Xm/CutPaste.h>	/* added for Motif 1.2 support */
#include <X11/Xatom.h>
#include <Xm/TextP.h>
#include "EDiskSrc.h"
#include "mlist.h"
#include "msg.h"
#include "toc.h"


static Msg ThisMsg(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    if (scrn->msg) return scrn->msg;
    if (scrn->toc && LastPopupMsg && MsgGetToc(LastPopupMsg) == scrn->toc)
	return LastPopupMsg;
    return NULL;
}



/*
 * Skip to the next (or previous, if lastwentbackwards) message.  Skips
 * over deleted messages if SkipDeleted is set, and over moved ones if
 * SkipMoved is set.
 */

void SkipToMsg(scrn, wantnew)
Scrn scrn;
Boolean wantnew;		/* True if we always want a new msg shown.  */
{
    Msg msg = scrn->msg;
    Toc toc = MsgGetToc(msg);
    FateType fate;
    Boolean lastdir;
    if (msg != NULL)
	fate = MsgGetFate(msg, (Toc *) NULL);
    while (msg != NULL && (wantnew || (SkipMoved && fate == Fmove) ||
			   (SkipDeleted && fate == Fdelete))) {
	if (scrn->lastwentbackwards) msg = TocMsgBefore(toc, msg);
	else msg = TocMsgAfter(toc, msg);
	if (msg != NULL)
	    fate = MsgGetFate(msg, (Toc *) NULL);
	wantnew = FALSE;
    }
    if (msg != NULL) {
	lastdir = scrn->lastwentbackwards;
	(void) MsgSetScrn(msg, scrn);
	scrn->lastwentbackwards = lastdir;
    }
}


void ExecViewBefore(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    TRACE(("@ExecViewBefore\n"));
    scrn->lastwentbackwards = TRUE;
    SkipToMsg(scrn, TRUE);
}


void ExecViewAfter(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    TRACE(("@ExecViewAfter\n"));
    scrn->lastwentbackwards = FALSE;
    SkipToMsg(scrn, TRUE);
}



/* ARGSUSED */
void ExecNextSelected(w, event, params, num_params)
Widget w;
XEvent *event;
char **params;
Cardinal *num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    MsgList mlist = GetSelectedMsgs(scrn);
    register int i;
    TRACE(("@ExecNextSelected\n"));
    if (!mlist || mlist->nummsgs == 0) return;
    scrn->lastwentbackwards = FALSE;
    if (scrn->msg) {
	for (i=0 ; i<mlist->nummsgs ; i++) {
	    if (mlist->msglist[i] == scrn->msg) {
		if (i < mlist->nummsgs - 1)
		    (void) MsgSetScrn(mlist->msglist[i+1], scrn);
		FreeMsgList(mlist);
		return;
	    }
	}
    }
    (void) MsgSetScrn(mlist->msglist[0], scrn);
    FreeMsgList(mlist);
}


/* ARGSUSED */
void ExecPrevSelected(w, event, params, num_params)
Widget w;
XEvent *event;
char **params;
Cardinal *num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    MsgList mlist = GetSelectedMsgs(scrn);
    register int i;
    TRACE(("@ExecPrevSelected\n"));
    if (!mlist || mlist->nummsgs == 0) return;
    scrn->lastwentbackwards = TRUE;
    if (scrn->msg) {
	for (i=mlist->nummsgs-1 ; i>=0 ; i--) {
	    if (mlist->msglist[i] == scrn->msg) {
		if (i > 0)
		    (void) MsgSetScrn(mlist->msglist[i-1], scrn);
		FreeMsgList(mlist);
		return;
	    }
	}
    }
    (void) MsgSetScrn(mlist->msglist[mlist->nummsgs - 1], scrn);
    FreeMsgList(mlist);
}



/* ARGSUSED */
void ExecThisReply(w, event, params, num_params)
Widget w;
XEvent *event;
char **params;
Cardinal *num_params;
{
    Msg msg = ThisMsg(w);
    Msg newmsg;
    Scrn nscrn;
    TRACE(("@ExecThisReply\n"));
    if (msg == NULL) return;
    nscrn = CreateNewScrn(params[0]);
    newmsg = TocMakeNewMsg(DraftsFolder);
    MsgSetTemporary(newmsg);
    MsgLoadReply(newmsg, msg);
    (void) MsgSetScrnForComp(newmsg, nscrn);
    MsgSetChanged(msg, False);
    MapScrn(nscrn, event->xbutton.time);
}


/* ARGSUSED */
void ExecThisForward(w, event, params, num_params)
Widget w;
XEvent *event;
char **params;
Cardinal *num_params;
{
    Msg msg = ThisMsg(w);
    MsgList mlist;
    TRACE(("@ExecThisForward\n"));
    if (msg == NULL) return;
    mlist = MakeSingleMsgList(msg);
    CreateForward(mlist, params[0], event->xbutton.time);
    FreeMsgList(mlist);
}


/* ARGSUSED */
void ExecThisInDefault(w, event, params, num_params)
Widget w;
XEvent *event;
char **params;
Cardinal *num_params;
{
    Msg msg = ThisMsg(w);
    Scrn vscrn = GetDefaultViewScrn();
    TRACE(("@ExecThisInDefault\n"));
    if (msg == NULL) return;
    if (vscrn == NULL)
	vscrn = CreateReadScrn(params[0]);
    (void) MsgSetScrn(msg, vscrn);
    MapScrn(vscrn, 0);
    TocRedoButtonPixmaps(MsgGetToc(msg));
}

/* ARGSUSED */
void ExecThisInNew(w, event, params, num_params)
Widget w;
XEvent *event;
char **params;
Cardinal *num_params;
{
    Msg msg = ThisMsg(w);
    Scrn vscrn;
    TRACE(("@ExecThisInNew\n"));
    if (msg == NULL) return;
    vscrn = CreateReadScrn(params[0]);
    (void) MsgSetScrn(msg, vscrn);
    MapScrn(vscrn, 0);
    TocRedoButtonPixmaps(MsgGetToc(msg));
}


/* ARGSUSED */
void ExecThisUseAsComposition(w, event, params, num_params)
Widget w;
XEvent *event;
char **params;
Cardinal *num_params;
{
    Msg msg = ThisMsg(w);
    Msg newmsg;
    Scrn nscrn;
    int i;
    FILE *fid;
    Widget  tw;

    TRACE(("@ExecThisUseAsComposition\n"));
    if (msg == NULL) return;
    nscrn = CreateNewScrn(params[0]);

    tw = nscrn->viewwidget;
    if ( nscrn->ddifheaders && XtIsManaged(nscrn->ddifheaders) )
        tw = nscrn->ddifheaders;

         fid = myfopen(MsgFileName(msg), "r");
	 if (fid) {
	 	if (checkForPS(fid)) 
		    StopComposeWordWrap(tw);
	}
    if (MsgGetToc(msg) == DraftsFolder)
	newmsg = msg;
    else {
	newmsg = TocMakeNewMsg(DraftsFolder);
	MsgLoadCopy(newmsg, msg);
	MsgSetTemporary(newmsg);
    }
    (void) MsgSetScrnForComp(newmsg, nscrn);
    MapScrn(nscrn, event->xbutton.time);
}



void ExecEditView(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    TRACE(("@ExecEditView\n"));
    if (scrn->msg == NULL) return;
	XmTextSetEditable(scrn->viewwidget,True);
}
    


void ExecSaveView(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    if (scrn->msg == NULL) return;
    TRACE(("@ExecSaveView\n"));
    MsgSaveChanges2(scrn->msg, scrn->viewwidget);
    if (scrn->viewwidget != NULL)
		XmTextSetEditable(scrn->viewwidget,False);
/*
    MsgClearEditable(scrn->msg);
*/
}
    
void ExecThisPrintStripped(w)
Widget w;
{
    Msg msg = ThisMsg(w);
    char str[200], check_prnt[100];
    char *tempPrintName;
    char *msgName;
    extern void stripMessage();

    TRACE(("@ExecThisPrintStripped\n"));
    if (msg == NULL) return;
    BeginLongOperation();
    msgName = MsgFileName(msg);
    tempPrintName = tempnam(NULL, "Print");
    stripMessage(msgName, tempPrintName);

    (void) sprintf(check_prnt, "%s", defPrintStrippedCommand); /* AJ 01*/

    if (strncmp(check_prnt , STRIPPEDPRINTDEFAULT, 
		strlen(STRIPPEDPRINTDEFAULT)) == 0)
      {
      (void) sprintf(check_prnt, "%s", defPrintCommand);
	  if ( strncmp ( check_prnt , PRINTDEFAULT, strlen(PRINTDEFAULT)) != 0)
	    (void) sprintf(str, "%s %s", check_prnt, tempPrintName);
	  else
	    (void) sprintf(str,"%s %s",defPrintStrippedCommand, tempPrintName);
      }
    else
      (void) sprintf(str, "%s %s", defPrintStrippedCommand, tempPrintName);

	
    (void) system(str);
    (void) unlink(tempPrintName);
    EndLongOperation();

}

void ExecThisPrint(w)
Widget w;
{
    Msg msg = ThisMsg(w);
    char str[200], check_prnt[100];

    TRACE(("@ExecThisPrint\n"));
    if (msg == NULL) return;
    if (MsgGetMsgType(msg) == MTps) {
	ExecThisPrintStripped(w);
	return;
    }
    BeginLongOperation();

    (void) sprintf(check_prnt, "%s", defPrintCommand); /* AJ 01 */

    if (strncmp(check_prnt , PRINTDEFAULT, strlen(PRINTDEFAULT)) == 0)
      {
      (void) sprintf(check_prnt, "%s", defPrintStrippedCommand);
	  if ( strncmp ( check_prnt ,STRIPPEDPRINTDEFAULT,
			strlen(STRIPPEDPRINTDEFAULT )) != 0)
	    (void) sprintf(str, "%s %s", check_prnt, MsgFileName(msg));
	  else
	    (void) sprintf(str, "%s %s", defPrintCommand, MsgFileName(msg));
      }
    else
    (void) sprintf(str, "%s %s", defPrintCommand, MsgFileName(msg));

    (void) system(str);
    EndLongOperation();
}

void ExecThisPrintSelective(w)
Widget w;
{
    Msg msg = ThisMsg(w);
    char str[200];

    TRACE(("@ExecThisPrintSelective\n"));
    if (msg == NULL) return;
    if (MsgGetMsgType(msg) == MTps) {
	ExecThisPrintStripped(w);
	return;
    }
    ExecThisPrint(w);
}

MsgHandle *dialogmsghandles = NULL;
char dialogfoldername[128];
int dialogmsghandlessize = 0;
Boolean dialogcopy;

static Boolean DoDialog(name, scrn)
char *name;
Scrn scrn;
{
    Toc toc = TocGetNamed(dialogfoldername);
    Toc desttoc = TocGetNamed(name);
    Scrn newscrn;

    MsgList mlist;
    Msg msg;
    register int i;
    if (strlen(name) == 0) {
	Error(scrn->parent, EFolderEmptyName, (char *) NULL);
	return FALSE;
    }
    if (desttoc == NULL) {
         newscrn = scrn;  /* initializing */
         for (i=0 ; i<numScrns ; i++) {
           if ( strcmp(scrnList[i]->name, "main") == 0)
             newscrn = scrnList[i];
         }
        if (!(CreateFolder (name, newscrn))) {
	  Error(scrn->parent, EFolderCantCreate, name);
	  return FALSE;
	}
	else
	  desttoc = TocGetNamed(name);
    }
    if (desttoc == toc) {
	Error (scrn->parent, EFolderCantMoveCopySelf, name);
	return FALSE;
    }
    mlist = MakeNullMsgList();
    for (i=0 ; i<dialogmsghandlessize ; i++) {
	msg = MsgFromHandle(dialogmsghandles[i]);
	if (msg && MsgGetToc(msg) == toc)
	    AppendMsgList(mlist, MsgFromHandle(dialogmsghandles[i]));
	MsgFreeHandle(dialogmsghandles[i]);
    }
    if (mlist->nummsgs == NULL) {
	Error(scrn->parent, EMessageNoneSpecified, (char *) NULL);
    } else {
	if (dialogcopy)
	    TocCopyMessages(toc, mlist, desttoc);
	else
	    MsgBatchSetFate(mlist, Fmove, desttoc);
    }
    FreeMsgList(mlist);
    dialogmsghandlessize = 0;
    /* TocForceRescan(toc); */ /* to ensure status update */
    return TRUE;
}




void MakeMoveOrCopyDialog(scrn, mlist, copy)
Scrn scrn;
MsgList mlist;
Boolean copy;			/* TRUE if this is a copy dialog box. */
{
    Toc toc;
    Toc desttoc = SelectedToc(scrnList[0]);
    register int i;
    if (mlist->nummsgs == 0) {
	Error(scrn->parent, EMessageNoneSpecified, (char *) NULL);
	return;
    }
    toc = MsgGetToc(mlist->msglist[0]);
    (void) strcpy(dialogfoldername, TocGetFolderName(toc));
    dialogmsghandlessize = mlist->nummsgs;
    dialogmsghandles = (MsgHandle *)
	XtRealloc((char *) dialogmsghandles,
		  mlist->nummsgs * sizeof(MsgHandle));
    for (i=0 ; i<mlist->nummsgs ; i++)
	dialogmsghandles[i] = MsgGetHandle(mlist->msglist[i]);
    MakePrompt(scrn,
	       (mlist->nummsgs == 1 ?
		(copy ? "copyMsg" : "moveMsg") :
		(copy ? "copyMsgs" : "moveMsgs")),
	       DoDialog,
	       (toc == desttoc) ? "" : TocGetFolderName(desttoc),
		"OnWindow OnFolders");
    dialogcopy = copy;
}


void ExecThisMoveDialog(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    MsgList mlist = MakeSingleMsgList(ThisMsg(w));
    TRACE(("@ExecThisMoveDialog\n"));
    MakeMoveOrCopyDialog(scrn, mlist, FALSE);
    FreeMsgList(mlist);
}


void ExecThisMove(w)
Widget w;
{
    Msg msg = ThisMsg(w);
    Toc desttoc = SelectedToc(scrnList[0]);
    MsgList mlist;
    TRACE(("@ExecThisMove\n"));
    if (msg == NULL) return;
    mlist = MakeSingleMsgList(msg);
    if (desttoc == MsgGetToc(msg) || desttoc == NULL) {
	MakeMoveOrCopyDialog(ScrnFromWidget(w), mlist, FALSE);
    } else {
	MsgBatchSetFate(mlist, Fmove, desttoc);
    }
    FreeMsgList(mlist);
}


void ExecThisCopyDialog(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    MsgList mlist = MakeSingleMsgList(ThisMsg(w));
    TRACE(("@ExecThisCopyDialog\n"));
    MakeMoveOrCopyDialog(scrn, mlist, TRUE);
    FreeMsgList(mlist);
}


void ExecThisCopy(w)
Widget w;
{
    Msg msg = ThisMsg(w);
    Toc desttoc = SelectedToc(scrnList[0]);
    MsgList mlist;
    TRACE(("@ExecThisCopy\n"));
    if (msg == NULL) return;
    mlist = MakeSingleMsgList(msg);
    if (desttoc == MsgGetToc(msg) || desttoc == NULL)
	MakeMoveOrCopyDialog(ScrnFromWidget(w), mlist, TRUE);
    else {
	TocCopyMessages(MsgGetToc(msg), mlist, desttoc);
    }
    FreeMsgList(mlist);
}


void ExecThisDelete(w)
Widget w;
{
    Msg msg = ThisMsg(w);
    MsgList mlist;
    TRACE(("@ExecThisDelete\n"));
    if (msg == NULL) return;
    mlist = MakeSingleMsgList(msg);
    MsgBatchSetFate(mlist, Fdelete, (Toc) NULL);
    FreeMsgList(mlist);
/* PJS: Make SURE that the buttons for the msg's screens are enabled... */
    ForceEnableScrnsDeleteButtons();
}


void ExecThisUnmark(w)
Widget w;
{
    Msg msg = ThisMsg(w);
    TRACE(("@ExecThisUnmark\n"));
    if (msg != NULL) {
	MsgSetFate(msg, Fignore, (Toc) NULL);
/* PJS: Make SURE that the buttons for the msg's screens are enabled... */
        ForceEnableScrnsDeleteButtons();
    }
}


void ExecNewMailInView(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    Msg msg = scrn->msg;
    Msg newmsg;
    Toc toc;
    TRACE(("@ExecNewMailInView\n"));
    if (msg && MsgChanged(msg))
	if (MsgSetScrn((Msg) NULL, scrn)) return;
    if (msg && TocCanIncorporate(MsgGetToc(msg)))
	toc = MsgGetToc(msg);
    else
	toc = InitialFolder;
    BeginLongOperation();
    TocStopUpdate(toc);
    newmsg = TocIncorporate(toc);
    if (newmsg)
	(void) MsgSetScrn(newmsg, scrn);
    else {
	NoNewMailWarning(scrn);
#ifdef notdef
	(void) MsgSetScrn(msg, scrn);
#endif
    }
    TocStartUpdate(toc);
    TocCheckForNewMail();
    EndLongOperation();
}


void ExecMsgAddToSeq(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    TRACE(("@ExecMsgAddToSeq\n"));
    TwiddleSequence(scrn, MakeSingleMsgList(scrn->msg), ADD);
}

void ExecMsgRemoveFromSeq(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    TRACE(("@ExecMsgRemoveFromSeq\n"));
    TwiddleSequence(scrn, MakeSingleMsgList(scrn->msg), REMOVE);
}


void ExecMakeDefaultView(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    Toc toc;
    register int i;
    TRACE(("@ExecMakeDefaultView\n"));
    if (scrn->readnum > 1) {
	for (i=0 ; i<numScrns ; i++) {
	    if (scrnList[i]->readnum == 1) {
		scrnList[i]->readnum = scrn->readnum;
		TocRedoButtonPixmaps(MsgGetToc(scrnList[i]->msg));
		ScrnNeedsTitleBarChanged(scrnList[i]);
		EnableProperButtons(scrnList[i]);
		break;
	    }
	}
    }
    scrn->readnum = 1;
    if (scrn->msg) {
	toc = MsgGetToc(scrn->msg);
	TocSetCurMsg(toc, scrn->msg);
	TocRedoButtonPixmaps(toc);
    }
    ScrnNeedsTitleBarChanged(scrn);
    EnableProperButtons(scrn);
}


static Boolean DoInsertFile(scrn, name)
Scrn scrn;
char *name;
{
    Widget widget = scrn->viewwidget;
    Msg msg = scrn->msg;
    XmTextPosition position;
    char *ptr;
    FILE *fid;

    TRACE(("@DoInsertFile\n"));

    if ( scrn->ddifheaders && XtIsManaged(scrn->ddifheaders) )
        widget = scrn->ddifheaders;

    if (!scrn->mapped || widget == NULL || !MsgGetEditable(msg)) {
	Error(scrn->parent, EMessageNoneSpecified, (char *) NULL);
	return TRUE;
    }

/* PJS: Can't insert Directories into the dxmail windows. */
    if (IsDirectory(name)) {
	Error(scrn->parent, ESelectDirectory, name);
	return(FALSE);
    }

    DEBUG(("Inserting %s\n", name));
    fid = myfopen(name, "r");
    if (!fid) {
	Error(scrn->parent, EFileCantOpen, name);
	return FALSE;
    }
    if (checkForPS(fid)) 
		    StopComposeWordWrap(widget);
    _XmTextDisableRedisplay(widget, TRUE);
    position = XmTextGetInsertionPosition(widget);
    while (ptr = ReadLineWithCR(fid)) {
	XmTextReplace(widget, position, position, ptr);
	position += strlen(ptr);
    }
    myfclose(fid);
    _XmTextEnableRedisplay(widget);
    return TRUE;
}
    

void ExecInsertFile(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    MakeFileSelect(scrn, DoInsertFile, "Choose file to insert",
		    "OnWindow OnCreating_and_sending OnIncluding_a_file");
}


static Boolean DoExtractFile(scrn, name)
Scrn scrn;
char *name;
{
    Msg msg = scrn->msg;
    char *ptr, **argv;
    FILE *tmp;
    XmTextSource	source = MsgGetSource(msg);
    Widget  tw;

    TRACE(("@DoInsertFile\n"));

    tw = scrn->viewwidget;
    if ( scrn->ddifheaders && XtIsManaged(scrn->ddifheaders) )
        tw = scrn->ddifheaders;


    if (!scrn->mapped || msg == NULL || source == NULL) {
	Error(scrn->parent, EMessageNoneSpecified, (char *) NULL);
	return TRUE;
    }
    if (MsgGetMsgType(msg) == MTddif) {
	if (!IsDirectory(name)) {
	    ptr = rindex(name, '/');
	    if (ptr) *ptr = '\0';
	    if (!ptr || !IsDirectory(name)) {
		Error(scrn->parent, EFileBadDirectory, name);
		return FALSE;
	    }
	}
	BeginLongOperation();
	ptr = MsgGetDDIFFile(msg);
	argv = MakeArgv(3);
	argv[0] = "dtoc";
	argv[1] = ptr;
	argv[2] = name;
	DoCommand(argv, NULL, "/dev/null");
	DeleteFileAndCheck(ptr);
	XtFree((char *) argv);
	XtFree(ptr);
	EndLongOperation();
    } else {
	if (MsgGetMsgType(msg) == MTps) {
	    tmp = myfopen(name, "w");
	    if (tmp == NULL) {
		Error(scrn->parent, EFileCantOpen, name);
		return FALSE;
	    }
	    (void) fclose(tmp);
	    CopyFileAndCheck(MsgFileName(msg), name);
	} else {
	    if (!XtEDiskSaveAsFile(source, name, True, tw)) {
		Error(scrn->parent, EFileCantOpen, name);
		return FALSE;
	    }
	}
    }
    return TRUE;
}

static Boolean DoExtractFileStrip(scrn, name)
Scrn scrn;
char *name;
{
    Msg msg = scrn->msg;
    char *tempName;
    char *ptr, **argv;
    XmTextSource	source = MsgGetSource(msg);
    FILE *tmp;
    Widget  tw;

    tw = scrn->viewwidget;
    if ( scrn->ddifheaders && XtIsManaged(scrn->ddifheaders) )
        tw = scrn->ddifheaders;

    if (!scrn->mapped || msg == NULL || source == NULL) {
	Error(scrn->parent, EMessageNoneSpecified, (char *) NULL);
	return TRUE;
    }
    if (MsgGetMsgType(msg) == MTddif) {
	if (!IsDirectory(name)) {
	    ptr = rindex(name, '/');
	    if (ptr) *ptr = '\0';
	    if (!ptr || !IsDirectory(name)) {
		Error(scrn->parent, EFileBadDirectory, name);
		return FALSE;
	    }
	}
	BeginLongOperation();
	ptr = MsgGetDDIFFile(msg);
	argv = MakeArgv(3);
	argv[0] = "dtoc";
	argv[1] = ptr;
	argv[2] = name;
	DoCommand(argv, NULL, "/dev/null");
	DeleteFileAndCheck(ptr);
	XtFree((char *) argv);
	XtFree(ptr);
	EndLongOperation();
    } else {
	if (MsgGetMsgType(msg) == MTps) {
	    tmp = myfopen(name, "w");
	    if (tmp == NULL) {
		Error(scrn->parent, EFileCantOpen, name);
		return FALSE;
	    }
	    (void) fclose(tmp);
	    stripMessage(MsgFileName(msg), name);
	} else {
	    tmp = myfopen(name, "w");
	    if (tmp == NULL) {
		Error(scrn->parent, EFileCantOpen, name);
		return FALSE;
	    }
	    (void) fclose(tmp);
	    tempName = tempnam(NULL, "Mext");
	    if (!XtEDiskSaveAsFile(source, tempName, True, tw)) {
		Error(scrn->parent, EFileCantOpen, name);
		return FALSE;
	    }
	    stripMessage(tempName, name);
	    (void) unlink(tempName);
	}
    }
    return TRUE;
}
    
static Boolean DoExtractSelected(scrn, name)
Scrn scrn;
char *name;
{
    Toc toc = scrn->toc;
    Msg msg;
    MsgList mlist;
    register int i, ac;
    char **argv;
    FILE *tmp;
    char testfile[1024];

    if (toc == NULL) return TRUE;
    BeginLongOperation();
    mlist = TocCurMsgList(toc, scrn);
    if (mlist->nummsgs == 0) {
	Error(scrn->parent, EMessageNoneSpecified, (char *) NULL);
	FreeMsgList(mlist);
	EndLongOperation();
	return TRUE;
    }

    ac = 0;
    if (IsDirectory(name)) {
	argv = MakeArgv(mlist->nummsgs + 2);
	argv[ac] = XtNewString("cp");
	msg = mlist->msglist[0];
	sprintf(testfile, "%s/%d", name, MsgGetId(msg));
	tmp = myfopen(testfile, "w");
	if (tmp == NULL) {
	    Error(scrn->parent, EFileCantOpen, testfile);
	    EndLongOperation();
	    return FALSE;
	}
	(void) fclose(tmp);
	unlink(testfile);
    } else {
	tmp = myfopen(name, "w");
	if (tmp == NULL) {
	    Error(scrn->parent, EFileCantOpen, name);
	    EndLongOperation();
	    return FALSE;
	}
	(void) fclose(tmp);
	unlink(name);
	argv = MakeArgv(mlist->nummsgs + 1);
	argv[ac] = XtNewString("cat");
    }
    ac++;
    for (i = 0; i < mlist->nummsgs; i++) {
	msg = mlist->msglist[i];
	argv[ac] = XtNewString(MsgFileName(msg));
	ac++;
    }
    if (IsDirectory(name)) {
	argv[ac] = XtNewString(name);
	ac++;
	DoCommand(argv, NULL, "/dev/null");
    } else {
	DoCommand(argv, NULL, name);
    }
    for (i = 0; i < ac; i++)
	XtFree(argv[i]);
    XtFree((char *) argv);
    FreeMsgList(mlist);
    EndLongOperation();
    return TRUE;
}

void ExecExtractMsg(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    Msg msg = scrn->msg;
    Toc toc = scrn->toc;
    MsgList mlist;

    TRACE(("@ExecExtractMsg\n"));
    if (msg == NULL && toc == NULL) {
	Error(scrn->parent, EMessageNoneSpecified, (char *) NULL);
	return;
    }
    if (msg == NULL) {
	mlist = TocCurMsgList(toc, scrn);
	if (mlist == NULL || mlist->nummsgs == 0) {
	    Error(scrn->parent, EMessageNoneSpecified, (char *) NULL);
	    FreeMsgList(mlist);
	    return;
	}
	FreeMsgList(mlist);
	MakeFileSelect(scrn, DoExtractSelected, 
		    "Choose file to contain extraction",
		    "OnWindow OnCreating_a_file");
    } else {
	MakeFileSelect(scrn, DoExtractFile,
		       (MsgGetMsgType(msg) == MTddif ?
		       "Choose directory to contain ddif files" :
		       "Choose file to contain extraction"),
		    "OnWindow OnCreating_a_file");
    }
}

void ExecExtractMsgStrip(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    Msg msg = scrn->msg;
    Toc toc = scrn->toc;
    MsgList mlist;

    TRACE(("@ExecExtractMsgStrip\n"));

    if (msg == NULL && toc == NULL) {
	Error(scrn->parent, EMessageNoneSpecified, (char *) NULL);
	return;
    }
    if (msg == NULL) {
	mlist = TocCurMsgList(toc, scrn);
	if (mlist == NULL || mlist->nummsgs == 0) {
	    Error(scrn->parent, EMessageNoneSpecified, (char *) NULL);
	    FreeMsgList(mlist);
	    return;
	}
	FreeMsgList(mlist);
	MakeFileSelect(scrn, DoExtractSelected, 
		    "Choose file to contain extraction",
		    "OnWindow OnCreating_a_file");
    } else {
	MakeFileSelect(scrn, DoExtractFileStrip,
		       (MsgGetMsgType(msg) == MTddif ?
		       "Choose directory to contain ddif files" :
		       "Choose file to contain extraction"),
		    "OnWindow OnCreating_a_file");
    }
}

static int clipItemID;
static int clipDataID;

static Widget clipwidget;

/* ARGSUSED */
static void callClosureWithSelection(w, closure, selection, type,
				     value, length, format)
Widget w;
Opaque closure;
Atom *selection;
Atom *type;
char *value;
int *length;
int *format;
{
    (*(void(*)())closure)(value, *length);
    XtFree(value);
}

static void copyToClipboard(sel, count)
  char *sel;
  int count;
{
    if (ClipboardSuccess !=
	    
	XmClipboardCopy (XtDisplay(clipwidget), XtWindow(clipwidget),
			       clipItemID, "STRING", sel, count,
			       NULL, &clipDataID)) {
	Punt("Clipboard copy failed");
        /* Feep(); */
    }
    if (ClipboardSuccess != XmClipboardEndCopy (XtDisplay(clipwidget), 
				  XtWindow(clipwidget),
				  clipItemID)) {
	Punt("Clipboard end copy failed");
	/* Feep(); */
    }
}


void ExecCopy(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    Msg msg = scrn->msg;
    XmTextSource source;
    XmTextPosition left, right;
    static XmString nameString = (XmString) NULL;

    TRACE(("@ExecCopy\n"));
    if (nameString == (XmString) NULL) {
	nameString = XmStringCreateSimple(progName);
    }
    if (msg == NULL) return;
    clipwidget = scrn->viewwidget;
    if ( scrn->ddifheaders && XtIsManaged(scrn->ddifheaders) )
        clipwidget = scrn->ddifheaders;
    source = MsgGetSource(msg);
    if (!(*source->GetSelection)(source, &left, &right))
        return;
    if (ClipboardSuccess ==
	    
	XmClipboardStartCopy (XtDisplay(clipwidget),
				XtWindow(clipwidget),
				nameString, 
				XtLastTimestampProcessed(theDisplay),
				w, 0,
				&clipItemID)) {
	XtGetSelectionValue(toplevel, XA_PRIMARY, FMT8BIT, 
	 (XtSelectionCallbackProc) callClosureWithSelection, copyToClipboard,
			    XtLastTimestampProcessed(theDisplay));
    } else {
	Punt("Clipboard start copy failed");
    }
}


void ExecCut(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    Msg msg = scrn->msg;
    XmTextBlockRec block;
    XmTextPosition left, right;
    XmTextSource source; 
    int i=0;

    TRACE(("@ExecCut\n"));
    if (msg == NULL) return;
    clipwidget = scrn->viewwidget;
    if ( scrn->ddifheaders && XtIsManaged(scrn->ddifheaders) )
        clipwidget = scrn->ddifheaders;

    source = MsgGetSource(msg);
    ExecCopy(w);
    block.length = 0;
    block.ptr = NULL;
    block.format = FMT8BIT;

	/* call the Motif cut procedure to handle the cut of the text instead
	   of the internal routines
	*/

	i = XmTextCut(clipwidget,CurrentTime);
	if ( i == False)
		DEBUG(("There is no selection.\n"));

/*	original code for Motif 1.1.3 version of DXMail
 */
/*
 *    if ((*source->GetSelection)(source, &left, &right)){
 *
 *	while (source->data->numwidgets > i && source->data->widgets[i]->text.source != source )
 *		i++;
 *
 *	if ( source->data->numwidgets == i)
 *		exit(0);
 *	else {
 *        if ((*source->Replace)(source, NULL, left, right, &block) != EditDone) {
 *	}	
 *    }
 *   }
 *   }
 */
}


void ExecPaste(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    Msg msg = scrn->msg;
    int len = 0, outlen, private_id, pos;
    char *buf;
    Widget  tw;

    TRACE(("@ExecPaste\n"));
    if (msg == NULL) return;
    tw = scrn->viewwidget;
    if ( scrn->ddifheaders && XtIsManaged(scrn->ddifheaders) )
        tw = scrn->ddifheaders;
    XmClipboardInquireLength(XtDisplay(tw),
			      XtWindow(tw),
			      "STRING", &len);
    DEBUG(("ExecPaste: len=%d\n",len));
    if (len <= 0)
	return;
    buf = XtMalloc(len+1);
    if (XmClipboardStartRetrieve(XtDisplay(tw),
				 XtWindow(tw),
				 XtLastTimestampProcessed(theDisplay)
				 ) == ClipboardSuccess) {
        if (XmClipboardRetrieve(XtDisplay(tw),
			     XtWindow(tw),
			     "STRING", buf, len,
			     &outlen, &private_id) == ClipboardSuccess) {
	    pos = XmTextGetInsertionPosition(tw);
	    buf[outlen] = '\0';
	    XmTextReplace(tw, pos, pos, buf);
	}
	XmClipboardEndRetrieve(XtDisplay(tw),
			       XtWindow(tw));
    }

    XtFree(buf);
}
