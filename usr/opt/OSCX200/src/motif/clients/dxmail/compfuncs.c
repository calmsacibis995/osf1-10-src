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
static char rcs_id[] ="@(#)$RCSfile: compfuncs.c,v $ $Revision: 1.1.6.6 $ (DEC) $Date: 1993/12/21 14:34:32 $";
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

/* compfuncs.c -- handle composition buttons. */

#include "decxmail.h"
#include "msg.h"
#include "toc.h"

Boolean orig_wordwrap, changed_wordwrap = False;

/* Reset this composition widget to be one with just a blank message
   template. */

void ExecCompReset(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    Msg msg;
    TRACE(("@ExecCompReset\n"));
    if (MsgSetScrn((Msg) NULL, scrn))
	return;
    msg = TocMakeNewMsg(DraftsFolder);
    MsgLoadComposition(msg);
    MsgSetTemporary(msg);
    MsgSetReapable(msg);
    ResetComposeScrnWordWrapSetting(scrn->viewwidget);
    (void) MsgSetScrn(msg, scrn);
}


/* Compose a new message. */

/* ARGSUSED */
void ExecComposeMessage(w, event, params, num_params)
Widget w;
XEvent *event;
char **params;
Cardinal *num_params;
{
    Widget tw;

    Scrn scrn = ScrnFromWidget(w);
    Msg msg;
    TRACE(("@ExecComposeMessage\n"));

    scrn = CreateNewScrn(params[0]);
    tw = scrn->viewwidget;
    if (scrn->ddifheaders && XtIsManaged(scrn->ddifheaders))
       tw = scrn->ddifheaders;
    ResetComposeScrnWordWrapSetting(tw);
    msg = TocMakeNewMsg(DraftsFolder);
    MsgLoadComposition(msg);
    MsgSetTemporary(msg);
    MsgSetReapable(msg);
    (void) MsgSetScrnForComp(msg, scrn);
    MapScrn(scrn, event->xbutton.time);
}


/* Send the message in this widget.  Avoid sending the same message twice.
   (Code elsewhere actually makes sure this button is disabled to avoid
   sending the same message twice, but it doesn't hurt to be safe here.) */


void ExecSendDraft(w)
Widget w;
{
    Widget tw;

    Scrn scrn = ScrnFromWidget(w);
    TRACE(("@ExecSendDraft\n"));

    tw = scrn->viewwidget;
    if (scrn->ddifheaders && XtIsManaged(scrn->ddifheaders))
       tw = scrn->ddifheaders;
    if (scrn->msg == NULL) return;
    if (!MsgGetReapable(scrn->msg)) {
	MsgSend2(scrn->msg, tw);
	MsgSetReapable(scrn->msg);
    }
}


/* Save any changes to the message.  This also makes this message permanent. */

void ExecSaveDraft(w)
Widget w;
{
    Scrn scrn = ScrnFromWidget(w);
    TRACE(("@ExecSaveDraft\n"));
    if (scrn->msg == NULL) return;
    MsgSetPermanent(scrn->msg);
    MsgSaveChanges2(scrn->msg, scrn->viewwidget);
    MsgClearReapable(scrn->msg);
}


/* Utility routine; creates a composition screen containing a forward message
   of the messages in the given msglist. */

CreateForward(mlist, name, eventtime)
MsgList mlist;
char *name;
Time eventtime;
{
    Scrn scrn;
    Msg msg;
    FILE * fid;
    int i;
    Widget tw;

    scrn = CreateNewScrn(name);   
    tw = scrn->viewwidget;
    if (scrn->ddifheaders && XtIsManaged(scrn->ddifheaders))
       tw = scrn->ddifheaders;
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
    msg = TocMakeNewMsg(DraftsFolder);
    MsgLoadForward(msg, mlist,scrn);
    MsgSetTemporary(msg);
    (void) MsgSetScrn(msg, scrn);
    if (scrn->ddifheaders && XtIsManaged(scrn->ddifheaders))
        XmTextSetEditable(scrn->ddifheaders,True);
    MapScrn(scrn, eventtime);
}



/*
 * Compose using a file, which we prompt for.
 */

static char screenname[100];

static Boolean DoExecComposeUsingFile(scrn, name)
Scrn scrn;
char *name;
{
    Msg msg;
    FILE *fid;
    Boolean GetComposeScrnWordWrapSetting();
    Widget tw;

    tw = scrn->viewwidget;
    if (scrn->ddifheaders && XtIsManaged(scrn->ddifheaders))
       tw = scrn->ddifheaders;

    msg = TocMakeNewMsg(DraftsFolder);


/* PJS: Added next test to stop garbage out on garbaage in... */
    if (!name || !(*name))
	MsgLoadComposition(msg);
    else {
/*  call CreateNewScrn to have a scrn struct to pass to MsgLoadFromFile 
*/
    	scrn = CreateNewScrn(screenname);
	if (!MsgLoadFromFile(msg, name,scrn)) {
	    Error(scrn->parent, EFileCantOpen, name);
	    (void) unlink(MsgFileName(msg));
	    TocRemoveMsg(DraftsFolder, msg);
	    MsgFree(msg);
	    return FALSE;
	}
    }
    MsgSetTemporary(msg);
    MsgSetReapable(msg);
    ResetComposeScrnWordWrapSetting(tw);
    fid = myfopen(name, "r");
    if (fid) {
	if (checkForPS(fid)) {
	    StopComposeWordWrap(tw);
	}
    }
    (void) MsgSetScrnForComp(msg, scrn);
    MapScrn(scrn, XtLastTimestampProcessed(theDisplay));
    return TRUE;
}
    

/* ARGSUSED */
void ExecComposeUsingFile(w, event, params, num_params)
Widget w;
XEvent *event;
char **params;
Cardinal *num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    TRACE(("@ExecComposeUsingFile\n"));
    strcpy(screenname, params[0]);
    MakeFileSelect(scrn, DoExecComposeUsingFile,
		   "Choose file for composition",
	"OnWindow OnCreating_and_Sending OnIncluding_a_file");
}




Boolean
GetComposeScrnWordWrapSetting(widget)
     Widget widget;
{
  Boolean         wordwrap = False;
  static Arg      args[] = {
    {XmNwordWrap, (XtArgVal)NULL},
  };
  args[0].value = (XtArgVal)&wordwrap;
  XtGetValues(widget, args, XtNumber(args));
  return wordwrap;
}

SetComposeScrnWordWrapSetting(widget, setting)
     Widget widget;
     Boolean setting;
{
  Arg      argset[1];

  if (setting) {
    XtSetArg(argset[0], XmNwordWrap, (XtArgVal) True );
  }
  else {
    XtSetArg(argset[0], XmNwordWrap, (XtArgVal) False );
  }
  XtSetValues(widget, argset, 1);
}


ResetComposeScrnWordWrapSetting(widget)
Widget widget;
{

    if (changed_wordwrap){
      SetComposeScrnWordWrapSetting( widget, orig_wordwrap);
      changed_wordwrap = False;
    }

}

StopComposeWordWrap(widget)
Widget widget;
{
  if ( GetComposeScrnWordWrapSetting(widget) ) {
    orig_wordwrap = True;
    SetComposeScrnWordWrapSetting(widget, False);
    changed_wordwrap = True;
    Warning(toplevel, NULL, "Auto-wordwrap disabled to preserve line-breaks");
  }
}





