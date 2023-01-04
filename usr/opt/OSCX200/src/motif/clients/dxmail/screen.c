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
static char rcs_id[] = "@(#)$RCSfile: screen.c,v $ $Revision: 1.1.6.7 $ (DEC) $Date: 1994/01/11 22:06:39 $";
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

/* screen.c -- management of scrns. */

#include "decxmail.h"
#include "msg.h"
#include "radio.h"
#include "toc.h"
#include "Shell.h"
#include "Vendor.h"
#include <Xm/PanedW.h>
#include <Xm/MainW.h>
#include <Xm/ScrolledW.h>
#include "actionprocs.h"
#include <Xatom.h>
#include "cursorfont.h"
#include "EDiskSrc.h"
#include "capsar.h"
#ifndef IGNOREDDIF
#include <cda_def.h>
/*
#include <dots_def.h>
*/
#include <cda_msg.h>
#include "dvr_msg.h"
#include "dvr_decw_def.h"
#undef Message
#endif /* IGNOREDDIF */
static int disableEnable = 0;


void DisableEnablingOfButtons()
{
    disableEnable++;
}

void EnableEnablingOfButtons()
{
    if (disableEnable)
	if (--disableEnable == 0)
	    EnableScrnsButtons();
}

/* PJS: Use this to reset ALL buttons of ALL screens. */
void EnableScrnsButtons()
{
register int i;
Scrn scrn;

    for (i=0 ; i<numScrns ; i++) {
	scrn = scrnList[i];
	if (scrn->mapped && scrn->neednewenabled)
	    EnableProperButtons(scrn);
    }
}

/* PJS: Use this to DEFINITELY reset ALL buttons of ALL screens. */
void ForceEnableScrnsButtons()
{
register int i;
Scrn scrn;

    for (i=0 ; i<numScrns ; i++) {
	scrn = scrnList[i];
	if (scrn->mapped)
	    EnableProperButtons(scrn);
    }
}

/* PJS: Use this to DEFINITELY reset the DELETE button on the screens. */
void ForceEnableScrnsDeleteButtons()
{
register int	i;
Scrn		scrn;
Boolean		msgsselected;

    for (i=0 ; i<numScrns ; i++) {
	scrn = scrnList[i];
	msgsselected = scrn->toc != NULL &&
	    (defAffectCurIfNullSelection || TocHasSelection(scrn->toc, scrn));
	if (scrn->mapped)
	    if (scrn->readnum > 0)
	        FuncChangeEnabled((XtActionProc)ExecThisDelete,scrn,
				  IsDeleteGrey(scrn,msgsselected,scrn->msg));
	    else
	        FuncChangeEnabled((XtActionProc)ExecMarkDelete,scrn,
				  IsDeleteGrey(scrn,msgsselected,scrn->msg));
    }
}

/* Figure out which buttons should and shouldn't be enabled in the given
   screen.  This should be called whenever something major happens to the
   screen. */

void EnableProperButtons(scrn)
Scrn scrn;
{
    Boolean value, editable, changed, reapable, msgsselected;
    Msg msg, msg2;
    MsgList mlist;
    FateType fate = Fignore;
    Toc toc, msgtoc;

    if (scrn) {
	if (disableEnable > 0 || !scrn->mapped) {
	    scrn->neednewenabled = TRUE;
	    return;
	}
	scrn->neednewenabled = FALSE;
	toc = scrn->toc;
	msg = scrn->msg;
	if (msg) msgtoc = MsgGetToc(msg);
	msgsselected = toc != NULL &&
	    (defAffectCurIfNullSelection || TocHasSelection(toc, scrn));
	FuncChangeEnabled((XtActionProc)NoOp, scrn, FALSE);
	FuncChangeEnabled((XtActionProc)ExecShowUnseen, scrn, TocCanIncorporate(toc));
	FuncChangeEnabled((XtActionProc)ExecPack, scrn, TocNeedsPacking(toc));

	value = TocHasSequences(toc);
	FuncChangeEnabled((XtActionProc)ExecOpenSeq, scrn, value);
	FuncChangeEnabled((XtActionProc)ExecAddToSeq, scrn, value && msgsselected);
	FuncChangeEnabled((XtActionProc)ExecRemoveFromSeq, scrn, value && msgsselected);
	FuncChangeEnabled((XtActionProc)ExecDeleteSeq, scrn, value);

	FuncChangeEnabled((XtActionProc)ExecEmptyWastebasket, scrn, defUseWastebasket);

	FuncChangeEnabled((XtActionProc)ExecViewNew, scrn, msgsselected);
	FuncChangeEnabled((XtActionProc)ExecViewInDefault, scrn, msgsselected);
	FuncChangeEnabled((XtActionProc)ExecTocForward, scrn, msgsselected);
	FuncChangeEnabled((XtActionProc)ExecTocUseAsComposition, scrn, msgsselected);

/* PJS: If the message is deleted, grey out the delete box. */
	FuncChangeEnabled((XtActionProc)ExecMarkDelete,scrn,
			  IsDeleteGrey(scrn,msgsselected,msg));

	FuncChangeEnabled((XtActionProc)ExecMarkCopy, scrn, msgsselected);
	FuncChangeEnabled((XtActionProc)ExecMarkMove, scrn, msgsselected);
	FuncChangeEnabled((XtActionProc)ExecMarkUnmarked, scrn, msgsselected);
	FuncChangeEnabled((XtActionProc)ExecThisPrint, scrn, (msg != NULL));
	FuncChangeEnabled((XtActionProc)ExecThisPrintStripped, scrn, (msg != NULL));
	FuncChangeEnabled((XtActionProc)ExecPrintMessages, scrn, 
			(msgsselected || (msg !=NULL)));
	FuncChangeEnabled((XtActionProc)ExecPrintStripped, scrn, 
			(msgsselected || (msg !=NULL)));
	FuncChangeEnabled((XtActionProc)ExecPrintWidget, scrn,
			(msgsselected || (msg !=NULL)));
	FuncChangeEnabled((XtActionProc)ExecPrintWidgetStripped, scrn,
			(msgsselected || (msg !=NULL)));
	FuncChangeEnabled((XtActionProc)ExecExtractMsg, scrn,
			(msgsselected || (msg !=NULL)));
	FuncChangeEnabled((XtActionProc)ExecExtractMsgStrip, scrn,
			(msgsselected || (msg !=NULL)));
	FuncChangeEnabled((XtActionProc)ExecTocReply, scrn, msgsselected);

	FuncChangeEnabled((XtActionProc)ExecViewInSpecified, scrn,
			  msgsselected &&
			      RadioGetNumChildren(ReadWindowsRadio) != 0);

/*	editable = ((msg != NULL && !MsgGetEditable(msg));
 */
        editable = ( !(scrn->ddifbody != NULL && XtIsManaged(scrn->ddifbody))
                    && (msg != NULL && !MsgGetEditable(msg)) );
	changed =  ( !(scrn->ddifbody != NULL && XtIsManaged(scrn->ddifbody))
                    && (msg != NULL && MsgChanged(msg)) );
	FuncChangeEnabled((XtActionProc)ExecEditView, scrn, editable);
	FuncChangeEnabled((XtActionProc)ExecSaveView, scrn, changed);

	if (msg != NULL) {  /* ps orientation */
            FuncChangeEnabled(ExecPSOrient,scrn, 
		(MsgGetMsgType(msg) == MTps));
	    changed = MsgChanged(msg);
	    if (!changed)
		MsgSetCallOnChange(msg,EnableProperButtons,(Opaque)scrn);
	    else
		MsgClearCallOnChange(msg);
	} else {
	    FuncChangeEnabled((XtActionProc)ExecEditView, scrn, True);
	}

        if (scrn->ddifbody != NULL && XtIsManaged(scrn->ddifbody)) {
	    FuncChangeEnabled((XtActionProc)ExecViewInDDIFViewer, scrn, True);
	    FuncChangeEnabled((XtActionProc)ExecCustomEditor, scrn, FALSE);
        }

	if (msg != NULL && msgtoc == DraftsFolder) {
	    changed = MsgChanged(msg);
	    reapable = MsgGetReapable(msg);
	    FuncChangeEnabled((XtActionProc)ExecSendDraft, scrn, changed || !reapable);
	    FuncChangeEnabled((XtActionProc)ExecSaveDraft, scrn,
			      (changed || reapable) &&
			          MsgGetMsgType(msg) == MTtext);
	    if (!changed) MsgSetCallOnChange(msg,
					     EnableProperButtons,
					     (Opaque) scrn);
	    else MsgClearCallOnChange(msg);
	} else {
	    FuncChangeEnabled((XtActionProc)ExecSendDraft, scrn, FALSE);
	    FuncChangeEnabled((XtActionProc)ExecSaveDraft, scrn, FALSE);
	}
	if (scrn->readnum > 0) {
	    FuncChangeEnabled((XtActionProc)ExecMakeDefaultView, scrn, scrn->readnum != 1);
	    mlist = GetSelectedMsgs(scrn);
	    FuncChangeEnabled((XtActionProc)ExecNextSelected, scrn,
			      mlist != NULL && mlist->nummsgs > 0 &&
			          mlist->msglist[mlist->nummsgs - 1] != msg);
	    FuncChangeEnabled((XtActionProc)ExecPrevSelected, scrn,
			      mlist != NULL && mlist->nummsgs > 0 &&
			          mlist->msglist[0] != msg);
	    if (mlist) FreeMsgList(mlist);
	    msg2 = msg;
	    while (msg2 && (msg2 == msg || (SkipDeleted && fate == Fdelete) ||
			    (SkipMoved && fate == Fmove))) {
		msg2 = TocMsgAfter(msgtoc, msg2);
		if (msg2) fate = MsgGetFate(msg2, (Toc *) NULL);
	    }
	    FuncChangeEnabled((XtActionProc)ExecViewAfter, scrn, msg2 != NULL);
	    msg2 = msg;
	    while (msg2 && (msg2 == msg || (SkipDeleted && fate == Fdelete) ||
			    (SkipMoved && fate == Fmove))) {
		msg2 = TocMsgBefore(msgtoc, msg2);
		if (msg2) fate = MsgGetFate(msg2, (Toc *) NULL);
	    }
	    FuncChangeEnabled((XtActionProc)ExecViewBefore, scrn, msg2 != NULL);
/* PJS: If the message is deleted, grey out the delete box. */
 	    FuncChangeEnabled((XtActionProc)ExecThisDelete,scrn,
			      IsDeleteGrey(scrn,msgsselected,msg));
	}
    }
}



/* Create a scrn of the given type. */

Scrn CreateNewScrn(name)
char *name;
{
    register int i;
    Scrn scrn;
    char *realName;
    char rname[500];
    static XtCallbackRec helpCallback[2] = 
	{(XtCallbackProc)ExecPopupHelpMenu, "OnMainWindow", NULL, NULL};
    static Arg args[] = {
	{XtNiconPixmap, NULL},
	{XmNhelpCallback, (XtArgVal) helpCallback},
	{XmNdeleteResponse, XmDO_NOTHING},
    };

    Pixmap iconifyPixmap;
    DEBUG(("In CreateNewScreen...\n"));
    if (defSVNStyle) {
	if (strcmp(name, "main") == 0) {
	    realName = "mainoutline";
	} else {
	    realName = name;
	}
    } else {
	realName = name;
    }
    if (realName == NULL) Punt("No name passed to CreateNewScrn!");
    for (i=0 ; i<numScrns ; i++) {
	if (!scrnList[i]->mapped && strcmp(scrnList[i]->name, realName) == 0)
	    return scrnList[i];
    }
    BeginLongOperation();

    sprintf(rname, "%sicon", realName);
    args[0].value = (XtArgVal)
	GetBitmapNamed(GetApplicationResourceAsString(rname, "Icon"));

    sprintf(rname, "%siconifyButton", realName);
    iconifyPixmap =
	(Pixmap) GetBitmapNamed(GetApplicationResourceAsString(rname, "Icon"));

    numScrns++;
    scrnList = (Scrn *)
	XtRealloc((char *) scrnList, (unsigned) numScrns*sizeof(Scrn));
    scrn = scrnList[numScrns - 1] = XtNew(ScrnRec);
    bzero((char *)scrn, sizeof(ScrnRec));
    scrn->lastwentbackwards = defDefaultViewBackwards;
    scrn->name = MallocACopy(realName);
/*    if (numScrns == 1) {
	scrn->parent = toplevel;
	XtSetValues(scrn->parent, args, XtNumber(args));
    } else */
    scrn->parent = XtCreatePopupShell(realName, topLevelShellWidgetClass,
				      toplevel, args, XtNumber(args));

    SetIconifyIcon(scrn->parent, iconifyPixmap);
    (void) sprintf(rname, "%sWindow", realName);

    scrn->main = XmCreateMainWindow(scrn->parent, rname, NULL, (Cardinal) 0);

    XtManageChild(scrn->main);

    scrn->widget = XtCreateWidget("workArea", xmPanedWindowWidgetClass,
				  scrn->main,
				  NULL, (Cardinal) 0);

    ActionNewScrn(scrn);

    MakeMenu(scrn, realName);

/* PJS - PMAX doesn't like this - It's new functionality anyway...
    XtInstallAllAccelerators(scrn->widget, scrn->main);

    if (scrn->viewwidget)
        XtInstallAllAccelerators(scrn->viewwidget, scrn->main);
    if (scrn->tocwidget)
        XtInstallAllAccelerators(scrn->tocwidget, scrn->main);
*/

    DEBUG(("Realizing..."));
    XtRealizeWidget(scrn->parent);
    scrn->normalcursor = MakeCursor(scrn->parent, NormalCursorName);
    scrn->sleepcursor = MakeCursor(scrn->parent, SleepCursorName);
    AddProtocols(scrn->parent, (XtCallbackProc) ExecCloseScrn, NULL);
    DEBUG(("done\n"));

    scrn->mapped = FALSE;
    EndLongOperation();
    return scrn;
}



static Boolean WorkCreateReadPopup(data)
Opaque data;
{
    Scrn scrn = (Scrn) data;
    static XtTranslations textoverride = NULL;
    if (textoverride == NULL)
	textoverride =
	    XtParseTranslationTable("<Btn3Down>: do-popup(readPopup)");
    XtOverrideTranslations(scrn->viewwidget, textoverride);
    CreateWidgetFromLayout(scrn, scrn->viewwidget, "readpopup");
    return TRUE;
}    

static Boolean WorkCreateDDIFReadPopup(data) /* ps orient popup */
Opaque data;
{
    Scrn scrn = (Scrn) data;
    static XtTranslations ddifoverride = NULL;

    if (ddifoverride == NULL) 
	ddifoverride =
	    XtParseTranslationTable("<Btn3Down>: do-popup(ddifPopup)");

    if ( !scrn->ddifbody )
        return FALSE;
    if (scrn->ddifbody)
	XtOverrideTranslations(scrn->ddifbody, ddifoverride);
    CreateWidgetFromLayout(scrn, scrn->ddifbody, "ddifpopup");
    return TRUE;
}    

/*
 * Create a "reading" screen.  This is the same as CreateNewScrn, but it also
 * sets the readnum for that scrn, and creates the read popup menu.
 */

Scrn CreateReadScrn(name)
char *name;
{
    Scrn scrn;
    register int i, n;
    char title[100], *ptr;
    Boolean reused = False;

    scrn = NULL;

    for (i = 0; i < numScrns; i++) {
	if (!scrnList[i]->mapped && scrnList[0]->readnum != 0) {
	    scrn = scrnList[i];
	    reused = True;
	    break;
	}
    }
    if (scrn == NULL) {
	for (i = 0; i < numScrns; i++) {
	    if (!scrnList[i]->mapped && strcmp(scrnList[i]->name, "read") == 0){
		scrn = scrnList[i];
		reused = True;
		break;
	    }
	}
    }
    if (scrn == NULL)
	scrn = CreateNewScrn(name);
    n = 1;
    for (i=0 ; i<numScrns; i++) {
	if (scrnList[i]->mapped && scrnList[i]->readnum == n) {
	    n++;
	    i = -1;		/* Start over. */
	}
    }
    scrn->readnum = n;
    (void) sprintf(title, "Window #%d", n);
    if (!reused) {
	ptr = title;
	RadioAddButtons(ReadWindowsRadio, &ptr, 1, n - 1);
	if (defDoReadPopups)
	    XtAddWorkProc(WorkCreateReadPopup, (Opaque) scrn);
    }
    if (n == 1)
	for (i=0 ; i<numScrns ; i++)
	    EnableProperButtons(scrnList[i]);
    return scrn;
}    




/* Destroy the screen.  If unsaved changes are in a msg, too bad. */

void DestroyScrn(scrn)
  Scrn scrn;
{
    char title[100];
    register int i,n;
    if (scrn->mapped) {
	XtPopdown(scrn->parent);
	DestroyConfirmWindow();
	TocSetScrn((Toc) NULL, scrn);
	MsgSetScrnForce((Msg) NULL, scrn);
    }
    scrn->mapped = FALSE;
    if (scrn->readnum != 0) {
	(void) sprintf(title, "Window #%d", scrn->readnum);
	RadioDeleteButton(ReadWindowsRadio, title);
	scrn->readnum = 0;
	if (RadioGetNumChildren(ReadWindowsRadio) == 0)
	    for (i=0 ; i<numScrns ; i++)
		EnableProperButtons(scrnList[i]);

    }

/* new code Motif 1.2 clean out the scrnlist don't reuse */

/* Also, Destroy widgets not being reused. */


   if ( scrn->parent ) {

       DEBUG(( "DestroyScrn: Icing viewwidget %lx !\n", scrn->viewwidget ));

       IcePopupChildren( scrn->parent );

       if ( scrn->ddifbody && XtIsManaged(scrn->ddifbody) ) {
          DEBUG(( "DestroyScrn: Icing ddifheaders %lx !\n", scrn->ddifheaders ));
          /* Close "DVR" file before destroying widget. */
          DvrCloseFile( scrn->ddifbody );
          XtDestroyWidget( scrn->ddifbody );
          scrn->ddifbody = NULL;
       }
       if ( scrn->ddifpane ) {
          XtDestroyWidget( scrn->ddifpane );
          scrn->ddifpane = NULL;
          scrn->ddifheaders = NULL;
       }
       XtDestroyWidget( scrn->parent );
       scrn->parent = NULL;
       scrn->main = NULL;
   }

   scrn->viewwidget= NULL;
    for (i = 0; i < numScrns; i++) {
	if (!scrnList[i]->mapped && scrnList[0]->readnum == 0) {
	   if ( scrn == scrnList[i] )
	   {
             scrnList[i] = NULL;
             for (n = i+1; n < numScrns; n++)
             {
              scrnList[n-1]=scrnList[n];
              scrnList[n] = NULL;
             }
             numScrns--;
	     break;
	  }
	}
    }
}



static void GrabFocusOnMap(widget, data, event)
Widget widget;
Opaque data;
XEvent *event;
{
    Time focusTime = (Time) data;
    Scrn scrn;
    if (event->xany.type == MapNotify) {
	scrn = ScrnFromWidget(widget);
	SetFocus(scrn->widget, focusTime);
	XtRemoveEventHandler(widget, StructureNotifyMask, FALSE,
			     (XtEventHandler)GrabFocusOnMap, data);
    }
}



void MapScrn(scrn, mapTime)
Scrn scrn;
Time mapTime;
{
    Boolean WorkChangeTitles();
    if (!scrn->mapped) {
	BeginLongOperation();	/* Hack to force cursor to be right. */
	if (mapTime > 0 && defGrabFocus) {
	    XtAddEventHandler(scrn->parent, StructureNotifyMask, FALSE,
		      (XtEventHandler) GrabFocusOnMap, (Opaque) mapTime);
	}
	scrn->mapped = TRUE;
	(void) WorkChangeTitles((Opaque) scrn);
	    /* Change titles *now*, before the window manager
	       even sees this window. */
	XtPopup(scrn->parent, XtGrabNone);
	XmProcessTraversal(scrn->viewwidget, XmTRAVERSE_CURRENT);
	EnableProperButtons(scrn);
	EndLongOperation();
    }
}


Scrn ScrnFromWidget(w)
Widget w;
{
    register int i;
    while (w && XtClass(w) != (WidgetClass) topLevelShellWidgetClass)
	w = GetParent(w);
    if (w) {
	for (i=0 ; i<numScrns ; i++) {
	    if (w == scrnList[i]->parent)
		return scrnList[i];
	}
    }
    DEBUG(("ScrnFromWidget w=%lx numScrns = %d\n",w,numScrns));
    Punt("ScrnFromWidget failed!");
    /*NOTREACHED*/
}
extern char user_title[];


static Boolean WorkChangeTitles(data)
Opaque data;
{
    Scrn scrn = (Scrn) data;
    Msg msg = scrn->msg;
    FateType fate;
    Toc desttoc;
    char title[200], icon[200];
    Arg args[2];
    Cardinal num_args = 0;
    if (scrn->mapped) {

	if (scrn->neednewtitle) {
	    if (scrn->viewwidget) {
		if (scrn->readnum == 0) {
		    (void) strcpy(title, "Mail:  Create");
		    if (msg && !MsgGetTemporary(msg)) {
			(void) strcat(title, "    ");
			(void) strcat(title, MsgGetName(msg));
		    }
		} else {
		    (void) sprintf(title, "Mail: Read - %d   ", scrn->readnum);
		    if (msg) {
			(void) strcat(title, MsgGetName(msg));

/* PJS: Add the sequence name to the READ window title: Useful, really... */
			(void) strcat(title, " (Sequence: ");
			(void) strcat(title, SelectedSeqName(scrn));
			(void) strcat(title, ")");

			fate = MsgGetFate(msg, &desttoc);
			switch (fate) {
			  case Fdelete:
			    (void) strcat(title, " Deleted");
			    break;
			  case Fmove:
			    (void) strcat(title, " Moved to ");
			    (void) strcat(title, TocGetFolderName(desttoc));
			    break;
			}
		    }
		}
	    } else {
                if (user_title[0])
                      (void) strcpy(title, user_title);
                else
		      (void) strcpy(title, "Mail: Main");
		if (scrn->toc) {
		    strcat(title, "   ");
		    strcat(title, TocGetFolderName(scrn->toc));
		}
	    }
	    XtSetArg(args[num_args], XtNtitle, (XtArgVal) title);
	    num_args++;
	}

	if (scrn->neednewiconname) {
	    strcpy(icon, progName);
	    if (scrn->viewwidget) {
		if (msg) strcpy(icon, MsgGetName(msg));
	    } else if (scrn->toc)
		strcpy(icon, TocGetFolderName(scrn->toc));
	    XtSetArg(args[num_args], XtNiconName, (XtArgVal) icon);
	    num_args++;
	}
		
    }
    if (num_args > 0)
	XtSetValues(scrn->parent, args, num_args);
    scrn->neednewtitle = scrn->neednewiconname = FALSE;
    return TRUE;
}

void ScrnNeedsTitleBarChanged(scrn)
Scrn scrn;
{
    if (scrn) {
	if (!scrn->neednewtitle && !scrn->neednewiconname)
	    XtAddWorkProc(WorkChangeTitles, (Opaque) scrn);
	scrn->neednewtitle = TRUE;
    }
}

void ScrnNeedsIconNameChanged(scrn)
Scrn scrn;
{
    if (scrn) {
	if (!scrn->neednewtitle && !scrn->neednewiconname)
	    XtAddWorkProc(WorkChangeTitles, (Opaque) scrn);
	scrn->neednewiconname = TRUE;
    }
}

#ifndef IGNOREDDIF 

int DXM_in_DDIFerrorHandler;

static void DDIFerrorHandler(w, tag, dvr_reason)
Widget w;
caddr_t *tag;
DvrCallbackStruct *dvr_reason;
{
#define MAX_MESSAGE_LENGTH 256

    Scrn scrn = ScrnFromWidget(w);
    int message_length;
    char message_buffer[MAX_MESSAGE_LENGTH];
    char *old_buffer;				/* current diag text contents */
    int  old_buffer_length;
    int ret_status;
    char *msgString;

    if (dvr_reason->reason == DvrCRpsOK)
	return;

    /* Just let "RedisplayMsg" know it's being called from
     * "DDIFerrorHandler" as a callback ... so it doesn't
     * close DVR files.
     */

    DXM_in_DDIFerrorHandler = 1;

    if (dvr_reason->reason == DvrCRcdaError) {
	if (dvr_reason->status == DVR$_NOPSHEAD) {
	    Message(scrn->parent, "Cannot view - not a ps file\n");
	    if (scrn->msg) MsgSetMsgText(scrn, scrn->msg);
	}
    }

    message_buffer[0] = '\0';

    /*
     * If a message text has been supplied, stick it in the
     * message buffer
     */

    /*if (message even AND there is a non-NULL text pointer..)*/
    if (dvr_reason->string_ptr != NULL &&
	    ((dvr_reason->status & 1) == 0)) {
	message_length = strlen(dvr_reason->string_ptr);
	if ((message_length + 2) >= MAX_MESSAGE_LENGTH)
	    message_length = MAX_MESSAGE_LENGTH - 2;
	strncpy(message_buffer, dvr_reason->string_ptr, message_length);
	if (message_buffer[message_length-1] == '\n')
	    message_buffer[message_length-1] = ' ';
    } else { /*if (message even AND there is a non-NULL text pointer..)*/
	if (dvr_reason->status != 0 &&
	    ((dvr_reason->status & 1) == 0)) {
	    switch (dvr_reason->status) {
		case DVR$_ALREADYWIDGET:
		    msgString = NULL;
		    break;
		case DVR$_BADCOMMENTS:
		    msgString = NULL;
		    break;
		case DVR$_BADFRAMETYPE:
		    msgString = "Bad Frame Type\n";
		    break;
		case DVR$_DDIFERR:
		    msgString = "DDIF Error\n";
		    break;
		case DVR$_DEFAULTFONT:
		    msgString = NULL;
		    break;
		case DVR$_EOC:
		    msgString = NULL;
		    break;
		case DVR$_EOD:
		    msgString = NULL;
		    break;
		case DVR$_FATALERROR:
		    msgString = "Fatal error\n";
		    break;
		case DVR$_FILENOTFOUND:
		    msgString = "File not found\n";
		    break;
		case DVR$_FILENOTOPEN:
		    msgString = "File not open\n";
		    break;
		case DVR$_FORMATERROR:
		    msgString = NULL;
		    break;
		case DVR$_FORMATINFO:
		    msgString = NULL;
		    break;
		case DVR$_FORMATWARN:
		    msgString = "Formatting warning\n";
		    break;
		case DVR$_GRAPHICFAIL:
		    msgString = "Error processing graphic information, cannot display\n";
		    break;
		case DVR$_IMAGEFAIL:
		    msgString = "Error processing image information, cannot display\n";
		    break;
		case DVR$_INTERNALERROR:
		    msgString = "Internal error\n";
		    break;
		case DVR$_INVALREQ:
		    msgString = NULL;
		    break;
		case DVR$_INVFILETYPE:
		    msgString = "Invalid file type\n";
		    break;
	        case DVR$_MEMALLOFAIL:
		    msgString = "Memory allocation failed\n";
		    break;
		case DVR$_MEMDEALLOFAIL:
		    msgString = "Memory deallocation failed\n";
		    break;
		case DVR$_NOCONVERTER:
		    msgString = "No converter\n";
		    break;
		case DVR$_NODISPCONT:
		    msgString = "No displayable content\n";
		    break;
		case DVR$_NODPSEXT:
		    msgString = "Server does not have Postscript Extension\n";
		    break;
		case DVR$_NOFONT:
		    msgString = NULL;
		    break;
		case DVR$_NOPAGE:
		    msgString = "Cannot view document\n";
		    break;
		case DVR$_NOTBITONAL:
		    msgString = "Image is not bitonal - cannot display\n";
		    break;
		case DVR$_NOTDDIFDOC:
		    msgString = "Message is not a DDIF document\n";
		    break;
		case DVR$_OPENFAIL:
		    msgString = "Message file open failed\n";
		    break;
		case DVR$_PAGENOTFOUND:
		    msgString = NULL;
		    break;
		case DVR$_SCRFULL:
		    msgString = "Screen is full\n";
		    break;
		case DVR$_TEXTFAIL:
		    msgString = "Error processing text string, cannot display\n";
		    break;
		case DVR$_TOPOFDOC:
		    msgString = NULL;
		    break;
		case DVR$_UNKOBJTYPE:
		    msgString = "Unknown internal object type\n";
		    break;
		case DVR$_UNKSTRTYPE:
		    msgString = "Unknown internal structure type\n";
		    break;
		case CDA$_OPENFAIL:
		    msgString = "Could not open file\n";
		    break;
		case CDA$_INVDOC:
		    msgString = "CDA Invalid document\n";
		    break;
		case CDA$_INVITMLST:
		    msgString = "CDA Invalid item list\n";
		    break;
		case CDA$_UNSUPFMT:
		case CDA$_ICVNOTFND:
		    msgString = "Unsupported format\n";
		    break;
		case CDA$_READFAIL:
		    msgString = "Read failed\n";
		    break;
	
		default:
		    sprintf(message_buffer, "Document viewer returned error status %d\n"
			,dvr_reason->status);
		    msgString = message_buffer;
		    break;
	    }
	    if (msgString) {
		strcpy(message_buffer, msgString);
	    } else {
		message_buffer[0] = '\0';
	    }
	}
    }
    if (message_buffer[0] != 0) {
	Message(scrn->parent, message_buffer);
	if (scrn->msg) MsgSetMsgText(scrn, scrn->msg);
    }

    DXM_in_DDIFerrorHandler = 0;

}

#endif IGNOREDDIF

void MakeDDIFWidgets(scrn)
Scrn scrn;
{
    extern WidgetClass xmTextWidgetClass;	/* %%% Blech!  Hack! */
    static Arg args[] = {
	{XmNleftAttachment, (XtArgVal) XmATTACH_FORM},
	{XmNrightAttachment, (XtArgVal) XmATTACH_FORM},
	{XmNtopAttachment, (XtArgVal) XmATTACH_FORM},
	{XmNbottomAttachment, (XtArgVal) XmATTACH_FORM},
    };
    static Arg textArgs[] = {
	{XmNrows, (XtArgVal) 2},
	{XmNeditMode, (XtArgVal) XmMULTI_LINE_EDIT},
	{XmNeditable, (XtArgVal) False},
    };
#ifndef IGNOREDDIF 
    static XtCallbackRec errorCallback[2] = 
	{(XtCallbackProc)DDIFerrorHandler, NULL, NULL, NULL};
    static Arg viewerArgs[] = {
	{XmNactivateCallback, (XtArgVal) errorCallback},
	{DvrNorientation, (XtArgVal) 0},
    };
    if (scrn != NULL && scrn->ddifpane)  {
	if (scrn->ddifbody)
	    XtSetValues(scrn->ddifbody, viewerArgs, XtNumber(viewerArgs));
	return;
    }
#endif IGNOREDDIF
    scrn->ddifpane = XtCreateWidget("ddifArea", xmPanedWindowWidgetClass,
				    GetParent(GetParent(scrn->viewwidget)),
				    args, XtNumber(args));
     scrn->ddifheaders = XmCreateScrolledText(scrn->ddifpane, "ddifHeaders",
					textArgs, XtNumber(textArgs));	
#ifdef IGNOREDDIF
    scrn->ddifbody = (Widget) XtCreateWidget("ddifBody", xmTextWidgetClass,
				    scrn->ddifpane, NULL, (Cardinal) 0);
#else
    scrn->ddifbody = (Widget)DvrViewerCreate(scrn->ddifpane, "ddifBody",
				     viewerArgs, XtNumber(viewerArgs));
    XtAddWorkProc(WorkCreateDDIFReadPopup, (Opaque) scrn);
#endif IGNOREDDIF

    XtManageChild(scrn->ddifpane);
    XtManageChild(scrn->ddifheaders);
    XtManageChild(scrn->ddifbody);
}



typedef struct _DDIFFileInfoRec {
    char *dirname;
    char *filename;
    short refcount;
} DDIFFileInfoRec;


static DDIFFileInfo MakeDDIFFileInfo(dir, file)
char *dir, *file;
{
    DDIFFileInfo info;
    info = XtNew(DDIFFileInfoRec);
    info->dirname = MallocACopy(dir);
    info->filename = MallocACopy(file);
    info->refcount = 1;
    return info;
}

void FreeDDIFInfo(info)
DDIFFileInfo info;
{
    if (info != NULL && --info->refcount <= 0) {
	XtFree(info->dirname);
	XtFree(info->filename);
	XtFree((char *) info);
        info = NULL;
    }
}

static void FreeDDIFFileInfo(info)
DDIFFileInfo info;
{
    if (info != NULL && --info->refcount <= 0) {
	NukeDirectory(info->dirname);
	XtFree(info->dirname);
	XtFree(info->filename);
	XtFree((char *) info);
        info = NULL;
    }
}


static Boolean WorkFreeDDIFFileInfo(data)
Opaque data;
{
    FreeDDIFFileInfo((DDIFFileInfo) data);
    return TRUE;
}



void DDIFShowFile(scrn, filename)
Scrn scrn;
char *filename;
{
#ifndef IGNOREDDIF
    int status;
    int i, magic;
    char tempdir[300], *firstfile, **argv;


    if ( DXM_in_DDIFerrorHandler ) return;

    if (scrn->msg != NULL && MsgGetMsgType(scrn->msg) == MTps)
	MsgDestroyPS(scrn->msg);
    FreeDDIFFileInfo(scrn->ddifinfo);
    scrn->ddifinfo = NULL;
    magic = GetMagicNumber(filename);
    if (magic == DOTS_MAGICNO) {
        DEBUG(("DOTS magic number detected. "));
	strcpy(tempdir, MakeNewTempFileName());
	MkDirAndCheck(tempdir, 0700);
#ifdef NOTDEF
    struct item_list input_item_list[4];	/* Item list for dots$unpack */
    struct item_list *output_item_list;		/* Output item list */
    struct item_list *itm;			/* Item list cursor */
	input_item_list[0].cda$w_item_length = strlen(filename);
	input_item_list[0].cda$w_item_code = DOTS$_INPUT_FILE;
	input_item_list[0].cda$a_item_address = filename;
	input_item_list[1].cda$w_item_length = strlen(tempdir);
	input_item_list[1].cda$w_item_code = DOTS$_OUTPUT_DEFAULT;
	input_item_list[1].cda$a_item_address = tempdir;
	input_item_list[2].cda$w_item_code = 0;
	output_item_list = NULL;
	status = dots$unpack(&input_item_list[0], &output_item_list);
	if (output_item_list != NULL) {
	    status = DvrViewerFile(scrn->ddifbody,
				   output_item_list->cda$a_item_address,
				   "ddif", NULL, 0, 0);
	    if (status != DVR$_NORMAL)
		DEBUG(("DOTS Viewer returned error code %x.", status));
	    status = dots$free_output_list(output_item_list);
	} else {
	    DEBUG(("dots$unpack returned error code %x.\n", status));
	}
#endif NOTDEF
	argv = MakeArgv(10);
	argv[0] = "dtoc";
	argv[1] = "-p";
	argv[2] = filename;
	argv[3] = tempdir;
	argv[4] = 0;
	firstfile = DoCommandToString(argv);
	i = strlen(firstfile);
	while (i>0 && firstfile[i-1] == '\n') firstfile[--i] = 0;
	scrn->ddifinfo = MakeDDIFFileInfo(tempdir, firstfile);
	if (i > 0) {
	    magic = GetMagicNumber(firstfile);
            if (magic == DDIF_MAGICNO) {
	      status = DvrViewerFile(scrn->ddifbody, firstfile, "ddif",
				   NULL, 0, 0);
	    }
	    else if (magic == DTIF_MAGICNO) {
	      status = DvrViewerFile(scrn->ddifbody, firstfile, "dtif",
				   NULL, 0, 0);
	    }
	    if (status != DVR$_NORMAL)
		DEBUG(("DDIF Viewer returned error code %x.(dots file)\n",status));
	} else {
	    DEBUG(("Can't convert dots file %s.\n",filename));
	}
	DeleteFileAndCheck(filename);
	XtFree(firstfile);
	XtFree((char *) argv);
      } 
    else 
      if (magic == DDIF_MAGICNO) {
        DEBUG(("DDIF magic number detected. \n"));
	status = DvrViewerFile(scrn->ddifbody, filename, "ddif", NULL, 0, 0);
	if (status != DVR$_NORMAL)
	  DEBUG(("DDIF Viewer returned error code %x.(ddif file)\n", status));
      } 
      else 
	if (magic == DTIF_MAGICNO) {
	  DEBUG (( " Detected a DTIF magic number in file\n"));
	  status = DvrViewerFile(scrn->ddifbody, filename, "dtif", NULL, 0, 0);
	  if (status != DVR$_NORMAL)
	    DEBUG(("DTIF Viewer returned error code %x.(dtif file)\n", status));
	}
	else
	{
	 DEBUG(("DDIFShowFile called w/o CDA file\n"));
	}
	

#else
static XmTextSource	*sourcelist = NULL;
static int		length = 0;
XmTextSource		source;
register int		i;

    source = XtCreateEDiskSource(filename, FALSE);

    XmTextSetSource(scrn->ddifbody, source, (XmTextPosition) 0, 
			(XmTextPosition) 0);

    if (numScrns > length) {
	sourcelist = (XmTextSource *)
	    XtRealloc((char *) sourcelist, numScrns * sizeof(XmTextSource));
	for (; length < numScrns ; length++)
	    sourcelist[length] = NULL;
    }

    for (i=0 ; i<numScrns ; i++)
	if (scrnList[i] == scrn) break;
    if (i >= numScrns)
	Punt("DDIF Show File failed");
    if (sourcelist[i])
	XtDestroyEDiskSource(sourcelist[i]);
    sourcelist[i] = source;
#endif IGNOREDDIF
}

void PSShowFile(scrn, filename)
Scrn scrn;
char *filename;
{
#ifndef IGNOREDDIF
    int status;
    char *origName;

    origName = XtNewString(filename);
    FreeDDIFFileInfo(scrn->ddifinfo);
    scrn->ddifinfo = NULL;
    status = DvrViewerFile(scrn->ddifbody, filename, "ps",
				   NULL, 0, 0);
    if (status == DVR$_NORMAL) {
	DeleteFileAndCheck(origName);
    } else {
	DEBUG(("DDIF Viewer returned error code %x.(ps file)\n",status));
    }
    XtFree(origName);
#else
static XmTextSource	*sourcelist = NULL;
static int		length = 0;
XmTextSource		source;
register int		i;

    source = XtCreateEDiskSource(filename, FALSE);

    XmTextSetSource(scrn->ddifbody, source, (XmTextPosition) 0, 
			(XmTextPosition) 0);

    if (numScrns > length) {
	sourcelist = (XmTextSource *)
	    XtRealloc((char *) sourcelist, numScrns * sizeof(XmTextSource));
	for (; length < numScrns ; length++)
	    sourcelist[length] = NULL;
    }

    for (i=0 ; i<numScrns ; i++)
	if (scrnList[i] == scrn) break;
    if (i >= numScrns)
	Punt("PS Show File failed");
    if (sourcelist[i])
	XtDestroyEDiskSource(sourcelist[i]);
    sourcelist[i] = source;
#endif IGNOREDDIF
}


void ExecViewInDDIFViewer(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    char **argv;
    register int c = 0;
    if (scrn->ddifinfo == NULL) return;
    scrn->ddifinfo->refcount++;
    argv = MakeArgv(4);
    argv[c++] = "dxvdoc";
    argv[c++] = scrn->ddifinfo->filename;
/*
    argv[c++] = "-display";
    argv[c++] = DisplayString(theDisplay);
*/
    argv[c++] = 0;
    DoCommandInBackground(argv, NULL, NULL, NULL, NULL );

/* *		  WorkFreeDDIFFileInfo, (Opaque) scrn->ddifinfo);
 * */
    XtFree((char *) argv);
}

/* PJS: Returns TRUE if any selected msgs are not deleted, or, 
 * if none selected, TRUE if there is a not deleted viewed message.
 */
Boolean IsDeleteGrey(scrn, msgsselected, msg)
Scrn scrn;
Boolean msgsselected;
Msg msg;
{
register int i;
Boolean value = FALSE;
MsgList mlist;

    mlist = GetSelectedMsgs(scrn);
    if (msgsselected && mlist != NULL && mlist->nummsgs > 0) {
	for (i=0; i<mlist->nummsgs; i++) {
	    if (MsgGetFate(mlist->msglist[i],(Toc *)NULL) != Fdelete) {
		value = TRUE;      /* A msg in the list ain't deleted... */
		break;
	    }
	}
    }
    else { /* No messages selected: does the TOC have one? */
	if (defAffectCurIfNullSelection || (msg != NULL && MsgGetFate(msg,(Toc *)NULL) != Fdelete))
	    value = TRUE;
    } 
    return value;
}


void ExecPSOrient(w, event, params, num_params)
Widget w;
XEvent *event;
char **params;
Cardinal *num_params;

#ifndef IGNOREDDIF

{
    Scrn scrn = ScrnFromWidget(w);
    Arg arg[1];
    char *filename;

    if (scrn == NULL) return;
    if (scrn->ddifbody == NULL || !XtIsManaged(scrn->ddifbody)) return;
    if (scrn->msg == NULL) return;

    XtSetArg(arg[0], DvrNorientation, atoi(params[0]));
    XtSetValues(scrn->ddifbody, arg, 1);
    
    RedisplayPSFile(scrn, scrn->msg);
} 

#else
{
  /* dummy function */
}
#endif IGNOREDDIF
