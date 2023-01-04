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
static char rcs_id[] = "@(#)$RCSfile: popup.c,v $ $Revision: 1.1.5.4 $ (DEC) $Date: 1994/01/11 22:06:36 $";
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


/* popup.c -- Handle pop-up widgets. */

#include "decxmail.h"
#include "toc.h"
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/MessageB.h>
#include <Xm/FileSB.h>
#include <Xm/SelectioB.h>
#include <Xm/DialogS.h>
#include <Xm/TextP.h>
#include <Xm/PanedW.h>
#include <Xm/PushB.h>
#include <Xm/Frame.h>
#include <Xm/RowColumn.h>
/* #include <DXm/DXmSvn.h> included in externs.h */
#include "actionprocs.h"
#include "tocintrnl.h"

static Widget curparent = NULL;
static char curstr[1000];

static Widget confirmwidget = NULL;
static Widget promptwidget = NULL;
static Widget prompttext;
static char error_buffer[2048];

static Widget MesgTopLevel = (Widget) 0;
static Widget MesgText = (Widget) 0;
static Widget dismiss;

static Boolean MesgMapped = False;

#define MESG_SIZE 4096
static char MesgString[MESG_SIZE];
static char InfoString[512];

static int delay_redisplay = 0;

Boolean (*promptfunc)();

static void DestroyPromptWindow()
{
    if (promptwidget != NULL) {
#ifndef NODESTROY
	XtDestroyWidget(promptwidget);
#else
	XtUnmanageChild(promptwidget);
#endif
	promptwidget = NULL;
    }
}

static void DoPromptNo()
{
    DestroyPromptWindow();
}

void DoPromptYes()
{
    char *ptr;
    TRACE(("@DoPromptYes\n"));
    if (promptwidget != NULL) {
	BeginLongOperation();
	ButtonSetRedo(DoPromptYes, NULL);
	ptr = XmTextGetString(prompttext);
	if ((*promptfunc)(ptr, ScrnFromWidget(promptwidget)))
	    DestroyPromptWindow();
	XtFree(ptr);
	EndLongOperation();
    }
}

void MakePrompt(scrn, name, func, initvalue, helpString)
Scrn scrn;
char *name;
Boolean (*func)();
char *initvalue;
char *helpString;
{
    static XtCallbackRec yescallback[2] = {(XtCallbackProc)DoPromptYes, NULL, NULL};
    static XtCallbackRec nocallback[2] = {(XtCallbackProc)DoPromptNo, NULL, NULL};
    static XtCallbackRec helpCallback[2] =
	{(XtCallbackProc)ExecPopupHelpMenu, NULL,
	 NULL, NULL};
    static Arg promptArgs[] = {
	{XmNokCallback, (XtArgVal) yescallback},
	{XmNcancelCallback, (XtArgVal) nocallback},
	{XmNhelpCallback, (XtArgVal) helpCallback},
    };

    DestroyConfirmWindow();
    DestroyPromptWindow();
    promptfunc = func;
    helpCallback[0].closure = (caddr_t) helpString;
    promptwidget = XmCreatePromptDialog(scrn->parent, name, promptArgs,
					XtNumber(promptArgs));
    prompttext = XmSelectionBoxGetChild(promptwidget, XmDIALOG_TEXT);
    XmTextSetString(prompttext, (initvalue ? initvalue : ""));

    XtRealizeWidget(promptwidget);
    XtManageChild(promptwidget);
}


static Scrn filescrn = NULL;
static Widget filewidget = NULL;
static Boolean (*filefunc)() = NULL;

static void DestroyFileSelect()
{
    if (filewidget)
#ifndef NODESTROY
	XtDestroyWidget(filewidget);
#else
	XtUnmanageChild(filewidget);
#endif
    filewidget = NULL;
}


/* ARGSUSED */
static void FileSelectYes(w, param, data)
Widget w;
Opaque param, data;
{
    XmFileSelectionBoxCallbackStruct *info =
	(XmFileSelectionBoxCallbackStruct *) data;
    char *filename = ExtractStringFromCompoundString(info->value);
    if (filescrn && filefunc) {
	if ((*filefunc)(filescrn, filename))
	    DestroyFileSelect();
    } else {
	DestroyFileSelect();
    }
}



/*
 * Make a file selection widget.  When "OK" is pressed in it, call the given
 * function with the scrn and the selected filename.  If the function returns
 * TRUE, then remove the widget.
 */

void MakeFileSelect(scrn, func, label, helpString)
Scrn scrn;
Boolean (*func)();
char *label;
char *helpString;
{
    static XtCallbackRec activatecallback[2] = {(XtCallbackProc)FileSelectYes, NULL, NULL};
    static XtCallbackRec cancelcallback[2] = {(XtCallbackProc)DestroyFileSelect, NULL, NULL};
    static XtCallbackRec helpCallback[2] = {(XtCallbackProc)ExecPopupHelpMenu, NULL,NULL, NULL};
    static XmString cstring = NULL;
    static Arg args[] = {
	{XmNdirMask, (XtArgVal) NULL},
	{XmNselectionLabelString, (XtArgVal) NULL},
	{XmNdialogStyle, (XtArgVal) XmDIALOG_MODELESS},
	{XmNokCallback, (XtArgVal) activatecallback},
	{XmNcancelCallback, (XtArgVal) cancelcallback},
	{XmNautoUnmanage, (XtArgVal) True},
	{XmNhelpCallback, (XtArgVal) helpCallback},
    };
    helpCallback[0].closure = (caddr_t) helpString;
    DestroyFileSelect();
    if (scrn == NULL) return;
    if (cstring == NULL) {
	cstring = XmStringCreateSimple("*");
	args[0].value = (XtArgVal) cstring;
    }
    args[1].value = (XtArgVal) XmStringCreateSimple(label);
    filescrn = scrn;
    filefunc = func;
    filewidget = XmCreateFileSelectionDialog(scrn->parent, "fileselect",
					args, (int) XtNumber(args));
    XtManageChild(filewidget);
    XtFree((char *) args[1].value);
}


void DestroyConfirmWindow() 
{
    if (curparent == NULL) return;
    curparent = NULL;
    XtUnmanageChild(confirmwidget);
#ifndef NODESTROY
    XtDestroyWidget(confirmwidget);
#endif
    confirmwidget = NULL;
}


static void DestroyMesgTopLevel() 
{
    if (MesgTopLevel == NULL) return;
    XtUnmanageChild(MesgTopLevel);
#ifndef NODESTROY
    XtDestroyWidget(MesgTopLevel);
#endif

    MesgTopLevel = NULL;
    MesgMapped = False;
}


static void DoYes()
{
    if (curparent == NULL) return;
    RedoLastButton();
    DestroyConfirmWindow();
}


static void DoNo()
{
    DestroyConfirmWindow();
}



Boolean Confirm(scrn, str, helpString)
Scrn scrn;
char *str;
char *helpString;
{
    static XtCallbackRec yescallback[2] = {(XtCallbackProc)DoYes, NULL, NULL};
    static XtCallbackRec nocallback[2] = {(XtCallbackProc)DoNo, NULL, NULL};
    static XtCallbackRec helpCallback[2] =
	{(XtCallbackProc)ExecPopupHelpMenu, NULL,
	 NULL, NULL};
    static Arg args[] = {
	{XmNmessageString , (XtArgVal) NULL},
	{XmNcancelLabelString, (XtArgVal) NULL},
	{XmNokCallback, (XtArgVal) yescallback},
	{XmNcancelCallback, (XtArgVal) nocallback},
	{XmNdialogStyle, (XtArgVal) XmDIALOG_MODELESS},
	{XmNhelpCallback, (XtArgVal) helpCallback},
    };
    if (scrn->parent == curparent && strcmp(str, curstr) == 0) {
	DestroyConfirmWindow();
	return TRUE;
    }
    DestroyConfirmWindow();
    helpCallback[0].closure = helpString;
    (void) strcpy(curstr, str);
    args[0].value = (XtArgVal) XmStringCreateSimple(curstr);
    confirmwidget = XmCreateWarningDialog(scrn->parent, "confirm",
					args, (int) XtNumber(args));
    XtMapWidget(scrn->parent);
    XtManageChild(confirmwidget);
    confirmwidget = GetParent(confirmwidget);
    curparent = scrn->parent;
    XtRealizeWidget(confirmwidget);
    XtManageChild(confirmwidget);
    XtFree((char *) args[0].value);
    return FALSE;
}

void Warning(parent, name, ptr)
Widget parent;
char *name, *ptr;
{
static XtCallbackRec	yescallback[2] = {(XtCallbackProc)DoNo, NULL, NULL};
static Arg	args[] = {
	{XmNmessageString , (XtArgVal) NULL},
	{XmNokCallback, (XtArgVal) yescallback},
	{XmNdialogStyle, (XtArgVal) XmDIALOG_MODELESS},
};
char	*format, *str;

    if (name == (char *)NULL && ptr == (char *)NULL)
	return;

    if (!ptr)
	ptr = "";

    if (name == (char *)NULL)
	str = ptr;
    else {
	format = GetApplicationResourceAsString(name, "WarningText");
	if (format == NULL) {
	    str = XtMalloc(strlen(name)+strlen(ptr) + 20);
	    if (ptr)
		(void) sprintf(str, "Errorcode: %s (%s)", name, ptr);
	    else
		(void) sprintf(str, "Errorcode: %s", name);
	} else {
	    str = XtMalloc(strlen(format)+strlen(ptr) + 2);
	    (void) sprintf(str, format, ptr);
	}
    }
    DestroyConfirmWindow();
    args[0].value = (XtArgVal) XmStringCreateSimple(str);
    confirmwidget = XmCreateInformationDialog(parent, "warning",
					args, (int) XtNumber(args));
    XtUnmanageChild(XmMessageBoxGetChild(confirmwidget, XmDIALOG_CANCEL_BUTTON));
    XtUnmanageChild(XmMessageBoxGetChild(confirmwidget, XmDIALOG_HELP_BUTTON));
    XtManageChild(confirmwidget);
    confirmwidget = GetParent(confirmwidget);
    curparent = parent;
    XtRealizeWidget(confirmwidget);
    XtManageChild(confirmwidget);

    if (str != ptr)
	XtFree(str);
}


void Error(parent, name, ptr)
Widget parent;
char *name, *ptr;
{
    Feep();
    Warning(parent, name, ptr);
}


void NoNewMailWarning(scrn)
Scrn scrn;
{
    if (defBeepIfNoMail)
	Feep();
    else
	Warning(scrn->parent, WNoNewMail, (char *)NULL);
}


void ExecPopupMenu(w, event, params, num_params)
Widget w;
XEvent *event;
char **params;
Cardinal *num_params;
{
    Scrn scrn = ScrnFromWidget(w);
    Popup popup;
    Widget widget, subwidget;
    TRACE(("@ExecPopupMenu\n"));
    if (*num_params == 0)
	Punt("ExecPopupMenu called with no args!");
    PopupWidget = w;
    subwidget = NULL;
    if (event->xbutton.subwindow)
	subwidget = XtWindowToWidget(XtDisplay(w), event->xbutton.subwindow);
    if (subwidget == NULL)
	subwidget = w;
    if (subwidget == w && *num_params >= 2 &&
	  strcmp(params[1], "childonly") == 0) {
	XUngrabPointer(theDisplay, event->xbutton.time);
	return;
    }
    if (IsScrollbarWidget(subwidget)) {
	XUngrabPointer(theDisplay, event->xbutton.time);
	return;
    }
	
    for (popup = scrn->firstpopup; popup != NULL ; popup = popup->next) {
	if (strcmp(popup->name, params[0]) == 0) {
	    widget = popup->widget;
	    XmMenuPosition(widget, event);
	    if (GetX(widget) < 0) SetX(widget, -1);
	    XtManageChild(widget);
	    if (popup->fixed) {
		XAllowEvents(theDisplay, AsyncPointer, event->xbutton.time);
	    }
	    break;
	}
    }
    LastPopupMsg = NULL;
    if (strcmp(GetWidgetName(w), "innerFolderArea") == 0) {
	LastPopupToc = TocGetNamed(GetWidgetName(subwidget));
	scrn = GetDefaultViewScrn();
	if (scrn) LastPopupMsg = scrn->msg;
    } else
	LastPopupToc = NULL;
}

/* ARGSUSED */
void PopupSvnMenu(w, closure, data)
Widget w;
int closure;
DXmSvnCallbackStruct *data;
{
    Scrn scrn = ScrnFromWidget(w);
    XEvent *event = data->event;
    Popup popup;
    Widget widget, subwidget;
    int entry;
    char *popupName;
    Toc toc;
    TagRec tag;

    TRACE(("@PopupSvnMenu\n"));
    PopupWidget = w;
    subwidget = NULL;
    if (event->xbutton.subwindow)
	subwidget = XtWindowToWidget(XtDisplay(w), event->xbutton.subwindow);
    if (subwidget == NULL)
	subwidget = w;
    if (IsScrollbarWidget(subwidget)) {
	XUngrabPointer(theDisplay, event->xbutton.time);
	return;
    }
	
    DXmSvnClearSelections(w);
    DXmSvnMapPosition(w, event->xbutton.x, event->xbutton.y,
	&entry, NULL, (unsigned int *) &tag);
    if (entry == 0) /* aju */
       {
          XUngrabPointer (theDisplay, event->xbutton.time);
          return;
       }

    DXmSvnSelectEntry(w, entry);
    if (tag.tagFields.msgNumber == 0) {
	popupName = "folderPopup";
	LastPopupToc = folderList[tag.tagFields.tocNumber];
	LastPopupMsg = NULL;
    } else {
	popupName = "messagePopup";
	toc = folderList[tag.tagFields.tocNumber];
	LastPopupMsg = toc->msgs[tag.tagFields.msgNumber - 1];
	LastPopupToc = NULL;
    }
    for (popup = scrn->firstpopup; popup != NULL ; popup = popup->next) {
	if (strcmp(popup->name, popupName) == 0) {
	    widget = popup->widget;
	    XmMenuPosition(widget, event);
	    if (GetX(widget) < 0) SetX(widget, -1);
	    XtManageChild(widget);
	    if (popup->fixed) {
		XAllowEvents(theDisplay, AsyncPointer, event->xbutton.time);
	    }
	    break;
	}
    }
}


static Boolean WorkPopupFinished()
{
    LastPopupMsg = NULL;
    LastPopupToc = NULL;
    return TRUE;
}

void PopupFinished()
{
    XtAddWorkProc((XtWorkProc)WorkPopupFinished, (Opaque) NULL);
    /* Doing this in a workproc is a real hack.  The stuff should get done
     after the current event is handled, but before any other events are
     handled.  I don't know how to do this. %%% */
}


void InitPopup()
{
    PopupWidget = NULL;
    LastPopupMsg = NULL;
    LastPopupToc = NULL;
}
/*ARGSUSED*/
static void
dismissFunction(widget, client_data, call_data)
Widget widget;
caddr_t client_data;
caddr_t call_data;
{
    MesgMapped = False;
    XtUnmapWidget(MesgTopLevel);
    return;
}


#ifdef __STDC__
#define BUTTON(nm,lbl)				\
static XtCallbackRec nm##Callbacks[] = {	\
    {nm##Function, NULL},			\
    {NULL, NULL}				\
};						\
static Arg nm##Args[] = {			\
    {XtNname, (XtArgVal) #nm},			\
    {XtNlabel, (XtArgVal) #lbl},		\
    {XmNactivateCallback, (XtArgVal) nm##Callbacks} \
};
#else
#define BUTTON(nm,lbl)				\
static XtCallbackRec nm/**/Callbacks[] = {	\
    {(XtCallbackProc)nm/**/Function, NULL},			\
    {NULL, NULL}				\
};						\
static Arg nm/**/Args[] = {			\
    {XtNname, (XtArgVal) "nm"},			\
    {XtNlabel, (XtArgVal) "lbl"},		\
    {XmNactivateCallback, (XtArgVal) nm/**/Callbacks} \
};
#endif


BUTTON(dismiss,dismiss);

static void displayMesgString()
{
    int newlen = strlen(MesgString) + 1;
    Arg args[2];

    if (delay_redisplay || (! MesgText))
	return;

    XmTextSetString(MesgText, MesgString);
    XmTextShowPosition(MesgText, (XmTextPosition) strlen(MesgString));
}

void mesgDisableRedisplay()
{
    delay_redisplay = 1;
    return;
}

void mesgEnableRedisplay()
{
    delay_redisplay = 0;
    displayMesgString();
    return;
}

Message (parent, newText)
Widget parent;
char *newText;
/*
 * brings up a new vertical pane, not moded
 *
 * the pane consists of 3 parts: title bar, scrollable text window,
 * button box
 */
{
    Widget pane, buttonBox, label, frame;
    Arg fontArgs[1];
    Arg paneArgs[4];
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    XmString labelString;
    static Arg labelArgs[] = {
	{XmNlabelString, (XtArgVal) NULL},
	{XmNskipAdjust, (XtArgVal) True},
    };
    static Arg boxArgs[] = {
	{XmNnumColumns,		(XtArgVal) 1},
	{XmNadjustLast, 	(XtArgVal) False},
	{XmNorientation,	(XtArgVal) XmHORIZONTAL},
	{XmNpacking,		(XtArgVal) XmPACK_COLUMN},
	{XmNallowResize,	(XtArgVal) True},
	{XmNskipAdjust, 	(XtArgVal) True},
    };
    static Arg shellArgs[] = {
	{XtNinput, (XtArgVal) True},
	{XtNsaveUnder, (XtArgVal) False},
    };
    static Arg textArgs[] = {
	{XmNrows,		(XtArgVal) 10},
	{XmNcolumns,		(XtArgVal) 80},
	{XmNwordWrap, 		(XtArgVal) TRUE},
	{XmNscrollVertical, 	(XtArgVal) TRUE},
	{XmNeditMode,  		(XtArgVal) XmMULTI_LINE_EDIT},
	{XmNeditable,		(XtArgVal) FALSE},
    };

    if (MesgTopLevel != (Widget) 0 && !MesgMapped) {
	(void) strcpy(MesgString, newText);
	XmTextSetString(MesgText, newText);
	XmTextShowPosition (MesgText, (XmTextPosition) strlen(MesgString));
	XtMapWidget(MesgTopLevel);
	XmProcessTraversal(dismiss, XmTRAVERSE_CURRENT);
	MesgMapped = True;
	return;
    }
    if (MesgTopLevel == (Widget) 0) {
	MesgTopLevel = XtCreatePopupShell("Mail: Viewer Information", topLevelShellWidgetClass,
					  parent, shellArgs, XtNumber(shellArgs));

	XtSetArg(paneArgs[0], XtNx, &x);
	XtSetArg(paneArgs[1], XtNy, &y);
	XtSetArg(paneArgs[2], XtNwidth, &width);
	XtSetArg(paneArgs[3], XtNheight, &height);
	XtGetValues(parent, paneArgs, XtNumber(paneArgs));
	XtSetArg(paneArgs[0], XtNx, x);
	XtSetArg(paneArgs[1], XtNy, y);
	XtSetArg(paneArgs[2], XtNwidth, width);
	XtSetArg(paneArgs[3], XtNheight, height);
	pane = XtCreateManagedWidget("pane", xmPanedWindowWidgetClass,
				    MesgTopLevel, paneArgs, XtNumber(paneArgs));

	(void) strcpy(MesgString, newText);

	if (labelArgs[0].value == NULL) {
	    labelArgs[0].value = (XtArgVal) XmStringLtoRCreate(
		"Viewer Error Information",
		XmSTRING_DEFAULT_CHARSET);
	}
	label = XtCreateManagedWidget("label", xmLabelWidgetClass, pane,
				      labelArgs, XtNumber(labelArgs));

/* *\	XmStringFree(labelArgs[0].value);
\* */
	frame = XtCreateManagedWidget("mesgFrame", xmFrameWidgetClass,
					pane, NULL, 0);
	MesgText = XmCreateScrolledText(frame, "text", 
					 textArgs, XtNumber(textArgs));
	XtManageChild(MesgText);

	buttonBox = XtCreateManagedWidget("box", xmRowColumnWidgetClass, pane,
					  boxArgs, XtNumber(boxArgs));
	dismissArgs[1].value = (XtArgVal) XmStringLtoRCreate(
						dismissArgs[0].value,
						XmSTRING_DEFAULT_CHARSET);
	dismiss = XtCreateManagedWidget("dismiss", xmPushButtonWidgetClass,
			      buttonBox, dismissArgs, XtNumber(dismissArgs));
    
	XmStringFree(dismissArgs[1].value);
	XtRealizeWidget(MesgTopLevel);

	XtPopup(MesgTopLevel, XtGrabNone);
	XmProcessTraversal(dismiss, XmTRAVERSE_CURRENT);
	MesgMapped = True;

	displayMesgString();
    } else {
	long len;
	long newlen;
	char addBuff[MESG_SIZE];

	(void) strcpy(addBuff, newText);
	len = strlen(MesgString);
	newlen = strlen(addBuff);

	if ((len + 10 + newlen) > MESG_SIZE) {
	    (void) strcpy(MesgString, addBuff);
	} else {	
#ifdef notdef
		(void) strcat(&MesgString[len], "\n");
#endif
		(void) strcat(&MesgString[len], addBuff);
	}

	displayMesgString();
    }
    
    return;
}


extern Widget DXMailConfirmBox;

void DestroyConfirmBox()
{
    if (DXMailConfirmBox != NULL) {
#ifndef NODESTROY
	XtDestroyWidget(DXMailConfirmBox);
#else
	XtUnmanageChild(DXMailConfirmBox);
#endif
	DXMailConfirmBox = NULL;
    }
}

void  IcePopupChildren( scrnparent )

 Widget  scrnparent;
{
   if ( scrnparent == NULL )
      return;
   if ( (filewidget) &&
        ( (Widget)(GetParent(XtParent(filewidget))) == scrnparent) )
      DestroyFileSelect();
   if ( (promptwidget) &&
        ( (Widget)(GetParent(XtParent(promptwidget))) == scrnparent) )
      DestroyPromptWindow();
   if ( (confirmwidget) &&
        ( (Widget)(GetParent(confirmwidget)) == scrnparent) )
      DestroyConfirmWindow();
   if ( (DXMailConfirmBox) &&
        ( (Widget)(GetParent(DXMailConfirmBox)) == scrnparent) )
      DestroyConfirmBox();
   if ( (MesgTopLevel) &&
        ( (Widget)(XtParent(MesgTopLevel)) == scrnparent) )
      DestroyMesgTopLevel();
}
