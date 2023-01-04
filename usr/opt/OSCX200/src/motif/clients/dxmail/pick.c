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
static char rcs_id[] = "@(#)$RCSfile: pick.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/08/02 23:59:28 $";
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


/* pick.c -- handle a pick widget. */


#include "decxmail.h"
#include "toc.h"
#include <Xm/Label.h>
#include <Xm/ToggleB.h>
#include <Xm/PushB.h>
#include <Xm/Form.h>
#include <Xm/PanedW.h>
#include <Xm/ScrolledW.h>
#include <Vendor.h>

typedef void (*VoidProc)();

typedef enum _EntryType {
    ETlabel, ETtext, ETtoggle, ETcommand
} EntryType;

typedef struct _EntryRec {	/* Info about one entry in the adb. */
    char *name;			/* Name for this widget.  */
    EntryType type;		/* Type of widget. */
    Opaque addr;		/* Pointer to place to store info in widget.
				   If ETcommand, this is func to call. */
    Boolean newrow;		/* Whether this widget starts a new row. */
    Widget widget;		/* Widget id (once created). */
} EntryRec, *Entry;

typedef struct _PickRec {
    Boolean mapped;		/* Whether this record is currently in use. */
    Cursor sleepcursor;
    Widget shell;		/* Shell widget containing pick. */
    Widget pane;		/* Pane containing everything. */
    Widget topadb;		/* Adb for top half. */
    Entry top;			/* Description of top pick adb. */
    Cardinal num_top;		/* Number of entries in above. */
    Widget bottomadb;		/* Adb for bottom half. */
    Entry bottom;		/* Description of bottom pick adb. */
    Cardinal num_bottom;	/* Number of entries in above. */
} PickRec, *Pick;


static Pick *pickList = NULL;	/* Array of pointers to created picks. */
static Cardinal numPicks = 0;	/* Size of above. */

static int numPendingPicks = 0;	/* Number of picks currently running. */

static void LittleOr(), BigOr(), DoYes(), DoNo();

static char **argv = NULL;
static int argvsize = 0;

#define OneLine(name) \
    {"skip",		ETtoggle,	NULL,			TRUE,	NULL},\
    {name,		ETlabel,	NULL,			FALSE,	NULL},\
    {"text",		ETtext,		NULL,			FALSE,	NULL},\
    {"or",		ETcommand,	(Opaque)LittleOr,	FALSE,	NULL}


static EntryRec TopTemplate[] = {
    OneLine("from"),
    OneLine("to"),
    OneLine("cc"),
    OneLine("date"),
    OneLine("subject"),
    OneLine("search"),

    {"skip",		ETtoggle,	NULL,			TRUE,	NULL},
    {"other",		ETtext,		NULL,			FALSE,	NULL},
    {"text",		ETtext,		NULL,			FALSE,	NULL},
    {"or",		ETcommand,	(Opaque)LittleOr,	FALSE,	NULL},

    {"bigor",		ETcommand,	(Opaque)BigOr,		TRUE,	NULL},
};



static char *fromfolder, *toseq, *fromseq, *fromdate, *todate, *datefield;
static Boolean mergeseq = False;

static EntryRec BottomTemplate[] = {
    {"fromFolderPrompt",ETlabel,	NULL,			TRUE,	NULL},
    {"fromFolder",	ETtext,		(Opaque)&fromfolder,	FALSE,	NULL},

    {"toSeqPrompt",	ETlabel,	NULL,			TRUE,	NULL},
    {"toSeq",		ETtext,		(Opaque)&toseq,		FALSE,	NULL},
    {"fromSeqPrompt",	ETlabel,	NULL,			FALSE,	NULL},
    {"fromSeq",		ETtext,		(Opaque)&fromseq,	FALSE, 	NULL},

    {"dateRngePrompt",	ETlabel,	NULL,			TRUE,	NULL},
    {"fromDate",	ETtext,		(Opaque)&fromdate,	FALSE,	NULL},
    {"dateBorder",	ETlabel,	NULL,			FALSE,	NULL},
    {"toDate",		ETtext,		(Opaque)&todate,	FALSE,	NULL},
    {"dateFieldPrompt",	ETlabel,	NULL,			FALSE,	NULL},
    {"dateField",	ETtext,		(Opaque)&datefield,	FALSE,	NULL},

    {"mergeSeq",	ETtoggle,	(Opaque)&mergeseq,	TRUE,	NULL},

    {"no",		ETcommand, 	(Opaque)DoNo,		TRUE,	NULL},
    {"yes",		ETcommand, 	(Opaque)DoYes,		FALSE,	NULL},

    {"inProgress",	ETlabel,	NULL,			TRUE,	NULL},
};




static Pick PickFromWidget(w)
Widget w;
{
    register int i;
    while (XtClass(w) != xmPanedWindowWidgetClass)
	w = GetParent(w);
    for (i=0 ; i<numPicks ; i++)
	if (pickList[i]->pane == w)
	    return pickList[i];
    Punt("Can't find it in PickFromWidget!");
    /*NOTREACHED*/
}


static void CreateAdb(pick, adb, entry, num_entry, adjust)
Pick pick;
Widget adb;
Entry entry;
Cardinal num_entry;
Boolean adjust;			/* Special hack.  If TRUE, makes all entries
				   in the second column have the same width. */
{
    Arg args[100];
    Cardinal num_args, c;
    Widget above = NULL, left = NULL;
    int lastrow = 0;
    register int i, j, maxwidth = 0;
    static WidgetList widgetlist = NULL;
    static Cardinal max = 0;
    WidgetClass class;
    static XtCallbackRec callbackstruct[2] = {NULL, NULL, NULL};
    static XtTranslations textoverride;

    if (num_entry > max) {
	max = num_entry;
	widgetlist = (WidgetList) XtMalloc((Cardinal) (max * sizeof(Widget)));
    }
    c = 0;
    for (i=0 ; i<num_entry ; i++) {
	num_args = 0;
	if (entry[i].newrow) {
	    left = NULL;
	    above = NULL;
	    for (j=lastrow ; j<i ; j++) {
		if (above == NULL ||
		        GetHeight(entry[j].widget) > GetHeight(above))
		    above = entry[j].widget;
	    }
	    lastrow = i;
	}
	if (above == NULL) {
	    XtSetArg(args[num_args], XmNtopAttachment, XmATTACH_FORM);
	    num_args++;
	} else {
	    XtSetArg(args[num_args], XmNtopAttachment, XmATTACH_WIDGET);
	    num_args++;
	    XtSetArg(args[num_args], XmNtopWidget, above);
	    num_args++;
	}
	if (left == NULL) {
	    XtSetArg(args[num_args], XmNleftAttachment, XmATTACH_FORM);
	    num_args++;
	} else {
	    XtSetArg(args[num_args], XmNleftAttachment, XmATTACH_WIDGET);
	    num_args++;
	    XtSetArg(args[num_args], XmNleftWidget, left);
	    num_args++;
	}
	switch (entry[i].type) {
	  case ETlabel:
	    class = (WidgetClass) xmLabelWidgetClass;
	    break;
	  case ETtext:
	    class = (WidgetClass) xmTextWidgetClass;
	    break;
	  case ETtoggle:
	    class = (WidgetClass) xmToggleButtonWidgetClass;
	    if (entry[i].addr) {
		XtSetArg(args[num_args], XmNset, (XtArgVal) (entry[i].addr));
		num_args++;
	    }
	    break;
	  case ETcommand:
	    class = (WidgetClass) xmPushButtonWidgetClass;
	    callbackstruct[0].callback = (VoidProc) entry[i].addr;
	    callbackstruct[0].closure = (caddr_t) pick;
	    XtSetArg(args[num_args], XmNactivateCallback, callbackstruct);
	    num_args++;
	    break;
	}
	if (entry[i].widget == NULL) {
	    if (adjust && c < i-1 && i == lastrow + 1) {
		XtSetArg(args[num_args], XmNwidth, maxwidth);
		num_args++;
	    }
	    entry[i].widget = XtCreateWidget(entry[i].name, class, adb,
					     args, num_args);
	    widgetlist[c++] = entry[i].widget;
	    if (entry[i].type == ETtext) {
		if (textoverride == NULL)
		    textoverride =
			XtParseTranslationTable("<Key>0xff0d: do-pick-yes()");
		XtOverrideTranslations(entry[i].widget, textoverride);
	    }
	} else {
	    static Arg getargs[] = {
		{XmNtopWidget, NULL},
		{XmNleftWidget, NULL},
	    };
	    if (c > 0) {
		XtManageChildren(widgetlist, c);
		c = 0;
	    }
	    getargs[0].value = getargs[1].value = NULL;
	    XtGetValues(entry[i].widget, getargs, XtNumber(getargs));
	    if ((above != NULL && getargs[0].value != (XtArgVal) above) ||
		  (left != NULL && getargs[1].value != (XtArgVal) left))
		XtSetValues(entry[i].widget, args, num_args);
	}
	if (adjust && i == lastrow + 1 &&
	      GetWidth(entry[i].widget) > maxwidth)
	    maxwidth = GetWidth(entry[i].widget);
	left = entry[i].widget;
    }
    if (adjust) {
	lastrow = 0;
	num_args = 0;
	XtSetArg(args[num_args], XmNwidth, maxwidth);
	num_args++;
	for (i=0 ; i<num_entry ; i++) {
	    if (entry[i].newrow) lastrow = i;
	    if (i == lastrow + 1) {
		if (GetWidth(entry[i].widget) != maxwidth)
		    XtSetValues(entry[i].widget, args, num_args);
	    }
	}
    }
    if (c != 0)
	XtManageChildren(widgetlist, c);
}


static void EraseAdb(entry, num_entry)
Entry entry;
Cardinal num_entry;
{
    int i;
    static Arg args[] = {
	{XmNvalue, (XtArgVal) FALSE},
    };
    for (i=0 ; i<num_entry ; i++, entry++) {
	switch (entry->type) {
	  case ETtext:
	    if (entry->addr == NULL)
		XmTextSetString(entry->widget, "");
	    else
		XmTextSetString(entry->widget, *((char **) entry->addr));
	    break;
	  case ETtoggle:
	    XtSetValues(entry->widget, args, XtNumber(args));
	    break;
	}
    }
}


/*
 * Insert the given entries into the top adb after the given widget.
 */
static void InsertEntries(pick, widget, new, num_new)
Pick pick;
Widget widget;
Entry new;
Cardinal num_new;
{
    Entry top = pick->top;
    Cardinal num_top = pick->num_top;
    Entry result;
    register int i, j, k;
    result = (Entry)
	XtMalloc((Cardinal) (num_top + num_new) * sizeof(EntryRec));
    j = 0;
    for (i=0 ; i<num_top ; i++) {
	if (top[i].widget == widget) {
	    result[j] = top[i];
	    result[j].type = ETlabel;
	    result[j].widget = NULL;
	    j++;
	    for (k=0 ; k<num_new ; k++)
		result[j++] = new[k];
	} else
	    result[j++] = top[i];
    }
    XtFree((char *) pick->top);
    pick->top = result;
    pick->num_top += num_new;
    CreateAdb(pick, pick->topadb, pick->top, pick->num_top, TRUE);
#ifdef NODESTROY
    XtUnmanageChild(widget);
#else
    XtDestroyWidget(widget);
#endif
}




/* ARGSUSED */
static void LittleOr(widget, param, data)
Widget widget;
Opaque param, data;
{
    static EntryRec new[] = {
	{"text",	ETtext,		NULL,			FALSE,	NULL},
	{"or",		ETcommand,	(Opaque)LittleOr,	FALSE,	NULL}
    };
    Pick pick = (Pick) param;
    InsertEntries(pick, widget, new, XtNumber(new));
}

/* ARGSUSED */
static void BigOr(widget, param, data)
Widget widget;
Opaque param, data;
{
    Pick pick = (Pick) param;
    InsertEntries(pick, widget, TopTemplate, XtNumber(TopTemplate));
}



/* ARGSUSED */
static void DoNo(widget, param, data)
Widget widget;
Opaque param, data;
{
    Pick pick = (Pick) param;
    XtPopdown(pick->shell);
    pick->mapped = FALSE;
    DestroyConfirmWindow();
}



static void StoreValue(entry, addr)
Entry entry;
Opaque addr;
{
    char **ptr, *temp;
    Boolean *b;
    switch(entry->type) {
      case ETtext:
	ptr = (char **) addr;
	*ptr = XmTextGetString(entry->widget);
	while (**ptr == '-') {
	    temp = *ptr;
	    *ptr = MallocACopy(temp + 1);
	    XtFree(temp);
	}
	break;
      case ETtoggle:
	b = (Boolean *) addr;
	*b = (Boolean) XmToggleButtonGetState(entry->widget);
	break;
    }
}

static void DumpArgv()
{
    register int i;
    if (debug) {
	for (i=0 ; i<argvsize ; i++)
	    (void) fprintf(stderr, "%s ", argv[i]);
	(void) fprintf(stderr, "\n");
    }
}

static void AppendArgv(ptr)
char *ptr;
{
    argvsize++;
    argv = ResizeArgv(argv, argvsize);
    argv[argvsize - 1] = MallocACopy(ptr);
}

static void DeleteLastArgv()
{
    argvsize--;
    XtFree(argv[argvsize]);
    argv[argvsize] = NULL;
}

#define GetLastArgv() (argv[argvsize - 1])
    

static void LBrace()
{
    AppendArgv("-lbrace");
}


/*
 * When adding a right brace, we backtrack and remove null expressions and
 * dangling operators.  
 */

static void RBrace()
{
    char *last;
    Boolean addit = TRUE;
    for (;;) {
	if (argvsize == 0) return;
	last = GetLastArgv();
	if (strcmp(last, "-lbrace") == 0) {
	    if (!addit) break;
	    addit = FALSE;
	}
	if (last[0] == '-' && strcmp(last, "-rbrace") != 0) DeleteLastArgv();
	else {
	    if (addit) AppendArgv("-rbrace");
	    break;
	}
    }
    return;
}



static void UpdatePendingPickDisplay()
{
    Pick pick;
    Entry entry;
    char str[100];
    register int i, j;
    if (numPendingPicks == 0) {
	strcpy(str, " ");
    } else if (numPendingPicks == 1) {
	strcpy(str, "(1 pick in progress)");
    } else {
	sprintf(str, "(%d picks in progress)", numPendingPicks);
    }
    for (i=0 ; i<numPicks ; i++) {
	pick = pickList[i];
	if (pick->mapped) {
	    for (j=0, entry=pick->bottom ; j<pick->num_bottom ; j++, entry++) {
		if (entry->type == ETlabel &&
		        strcmp(entry->name, "inProgress") == 0) {
		    ChangeLabel(entry->widget, str);
		}
	    }
	}
    }
}



typedef struct _PendingPickRec {
    Toc toc;
    char *toseq;
} PendingPickRec, *PendingPick;


Boolean FinishPick(pending)
PendingPick pending;
{
    Toc toc = pending->toc;
    TocReloadSeqLists(toc);
    TocChangeViewedSeq(toc, TocGetSeqNamed(toc, pending->toseq));
    XtFree(pending->toseq);
    XtFree((char *) pending);
    numPendingPicks--;
    UpdatePendingPickDisplay();
    return TRUE;
}


/* ARGSUSED */
static void DoYes(widget, param, data)
Widget widget;
Opaque param, data;
{
    Pick pick = (Pick) param;
    Toc toc;
    Entry entry;
    Cardinal num_entry;
    char *ptr, kind[500], str[500];
    register int i;
    Boolean b;
    PendingPick pending;

    DestroyConfirmWindow();
    if (!pick->mapped) return;
    entry = pick->bottom;
    num_entry = pick->num_bottom;
    for (i=0 ; i<num_entry ; i++) {
	if (entry[i].addr && entry[i].type != ETcommand)
	    StoreValue(entry+i, entry[i].addr);
    }

    toc = TocGetNamed(fromfolder);
    TocRecheckValidity(toc);

    if (toc == NULL) {
	Error(pick->shell, EFolderNoSuch, fromfolder);
	return;
    }
    if (TocGetSeqNamed(toc, fromseq) == NULL) {
	Error(pick->shell, ESequenceNoSuch, fromseq);
	return;
    }
    if (toseq[0] == NULL) {
	Error(pick->shell, ESequenceEmptyName, (char *) NULL);
	return;
    }
    if (strcmp(toseq, "all") == 0) {
	Error(pick->shell, ESequenceNoAll, (char *) NULL);
	return;
    }


    BeginLongOperation();
    argv = MakeArgv(1);
    argvsize = 0;
    AppendArgv(pickCmd);
    (void) sprintf(str, "+%s", TocGetFolderName(toc));
    AppendArgv(str);
    AppendArgv(fromseq);
    AppendArgv("-sequence");
    AppendArgv(toseq);
    if (mergeseq) AppendArgv("-nozero");
    else AppendArgv("-zero");
    if (debug) AppendArgv("-list");
    if (*datefield) {
	AppendArgv("-datefield");
	AppendArgv(datefield);
    }
    if (*fromdate) {
	AppendArgv("-after");
	AppendArgv(fromdate);
	AppendArgv("-and");
    }
    if (*todate) {
	AppendArgv("-before");
	AppendArgv(todate);
	AppendArgv("-and");
    }
    LBrace();
    LBrace();			/* Gets closed by any BigOr's. */
    LBrace();
    for (i=0, entry=pick->top ; i<pick->num_top ; i++, entry++) {
	switch(entry->type) {
	  case ETtoggle:
	    RBrace();
	    if (strcmp(GetLastArgv(), "-rbrace") == 0)
		AppendArgv("-and");
	    StoreValue(entry, (Opaque) &b);
	    if (b) AppendArgv("-not");
	    LBrace();
	    break;
	  case ETlabel:
	    if (strcmp(entry->name, "bigor") == 0) {
		RBrace();
		RBrace();
		if (strcmp(GetLastArgv(), "-rbrace") == 0)
		    AppendArgv("-or");
		LBrace();
		LBrace();
	    } else if (strcmp(entry->name, "or") != 0) {
		(void) sprintf(kind, "-%s", entry->name);
	    }
	    break;
	  case ETtext:
	    StoreValue(entry, (Opaque) &ptr);
	    if (strcmp(entry->name, "other") == 0) {
		if (*ptr == 0) {
		    while (entry->type != ETcommand) i++, entry++;
		} else
		    (void) sprintf(kind, "--%s", ptr);
	    } else if (*ptr) {
		AppendArgv(kind);
		AppendArgv(ptr);
		AppendArgv("-or");
	    }
	    XtFree(ptr);
	    break;
	}
    }
    RBrace();
    RBrace();
    RBrace();
		
    if (debug) DumpArgv();
    TocCreateNullSequence(toc, toseq);
    pending = XtNew(PendingPickRec);
    pending->toc = toc;
    pending->toseq = MallocACopy(toseq);
    numPendingPicks++;
    UpdatePendingPickDisplay();
    DoCommandInBackground(argv, (char *) NULL, debug ? NULL : "/dev/null",
			  (XtWorkProc) FinishPick, (Opaque) pending);

    for (i=0 ; i<argvsize ; i++)
	XtFree(argv[i]);
    XtFree((char *) argv);

    EndLongOperation();
}


/* ARGSUSED */
void DoPickYes(w, event, params, num_params)
Widget w;
XEvent *event;
char **params;
Cardinal *num_params;
{
    Pick pick = PickFromWidget(w);
    TRACE(("@DoPickYes\n"));
    DoYes(w, (Opaque) pick, (Opaque) NULL);
}


void CreatePick(foldername, from, to)
char *foldername, *from, *to;
{
    register int i;
    Pick pick;
    Widget scrollw;
    static Arg args[] = {
	{XtNiconPixmap, (XtArgVal) NULL},
	{XtNiconName, (XtArgVal) "Mail: pick"},
	{XtNtitle, (XtArgVal) "Mail: pick"},
    };
    BeginLongOperation();
    for (i=0 ; i<numPicks ; i++)
	if (pickList[i]->mapped == FALSE) break;
    if (i < numPicks) {
	pick = pickList[i];
    } else {
	numPicks++;
	pickList = (Pick *)
	    XtRealloc((char *) pickList, (unsigned) numPicks * sizeof(Pick));
	pick = pickList[numPicks - 1] = XtNew(PickRec);
	bzero((char *)pick, sizeof(PickRec));
	args[0].value = (XtArgVal) GetBitmapNamed("pick");
	pick->shell =
	    XtCreatePopupShell("pick", topLevelShellWidgetClass,
			       toplevel, args, XtNumber(args));
	pick->pane = XtCreateWidget("pane", xmPanedWindowWidgetClass,
				    pick->shell,
				    NULL, (Cardinal) 0);
	scrollw = XtCreateWidget("topscroll", xmScrolledWindowWidgetClass,
				 pick->pane, NULL, (Cardinal) 0);
	pick->topadb = XmCreateForm(scrollw, "top",
					   NULL, (Cardinal) 0);
	XtManageChild(pick->topadb);
	XtManageChild(scrollw);
	pick->bottomadb = XmCreateForm(pick->pane, "bottom",
					      NULL, (Cardinal) 0);
	XtManageChild(pick->bottomadb);
	pick->top = (Entry)
	    XtMalloc((unsigned) (sizeof(EntryRec) * XtNumber(TopTemplate)));
	pick->num_top = XtNumber(TopTemplate);
	bcopy((char *) TopTemplate, (char *) pick->top,
	      sizeof(EntryRec) * XtNumber(TopTemplate));
	pick->bottom = (Entry)
	    XtMalloc((unsigned) (sizeof(EntryRec) * XtNumber(BottomTemplate)));
	pick->num_bottom = XtNumber(BottomTemplate);
	bcopy((char *) BottomTemplate, (char *) pick->bottom,
	      sizeof(EntryRec) * XtNumber(BottomTemplate));
	CreateAdb(pick, pick->topadb, pick->top, pick->num_top, TRUE);
	CreateAdb(pick, pick->bottomadb, pick->bottom, pick->num_bottom,
		   FALSE);
	XtManageChild(pick->pane);
	XtRealizeWidget(pick->shell);
	pick->sleepcursor = MakeCursor(pick->shell, SleepCursorName);
    }
    EraseAdb(pick->top, pick->num_top);
    fromfolder = foldername;
    fromseq = from;
    toseq = to;
    datefield = fromdate = todate = "";
    EraseAdb(pick->bottom, pick->num_bottom);
    XtPopup(pick->shell, XtGrabNone);
    pick->mapped = TRUE;
    UpdatePendingPickDisplay();
    EndLongOperation();
}


/* Change cursors in pick windows for long operation. */

void PickBeginLongOperation()
{
    register int i;
    Pick pick;
    for (i=0 ; i<numPicks ; i++) {
	pick = pickList[i];
	if (pick->mapped)
	    XDefineCursor(XtDisplay(pick->shell), XtWindow(pick->shell),
			  pick->sleepcursor);
    }
}


/* Change cursors in pick windows after long operation. */

void PickEndLongOperation()
{
    register int i;
    Pick pick;
    for (i=0 ; i<numPicks ; i++) {
	pick = pickList[i];
	if (pick->mapped)
	    XDefineCursor(XtDisplay(pick->shell), XtWindow(pick->shell),
			  NULL);
    }
}
