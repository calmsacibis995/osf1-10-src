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
static char rcs_id[] = "@(#)$RCSfile: customize.c,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/12/21 14:34:35 $";
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


/* customize.c --  let the user customize things. */

#include "decxmail.h"
#include "radio.h"
#include "toc.h"
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/Separator.h>

typedef struct _RadioDesc {
    int value;
    char *str;
} RadioDescRec, *RadioDesc;

typedef struct _CustomRec {
    caddr_t addr;
    String type;
    String resource;
    RadioDesc desc;
} CustomRec, *Custom;


char *agingString;

static RadioDescRec alldef[] = {
    {FALSE, "off"},
    {TRUE, "on"},
    {DEFAULT, "default"},
    {0, NULL},
};

static RadioDescRec layoutdef[] = {
    {FALSE, "paned"},
    {TRUE, "outline"},
    {0, NULL},
};

static CustomRec customlist[] = {
    {NULL, NULL, NULL, NULL},
    {(caddr_t)&SkipDeleted, XtRBoolean, "SkipDeleted", NULL},
    {(caddr_t)&SkipMoved, XtRBoolean, "SkipMoved", NULL},
    {NULL, NULL, NULL, NULL},
    {(caddr_t)&AutoCommit, XtRBoolean, "AutoCommit", NULL},
    {(caddr_t)&AutoPack, XtRBoolean, "AutoPack", NULL},
    {NULL, NULL, NULL, NULL},
    {(caddr_t)&defUseWastebasket, XtRBoolean, "UseWastebasket", NULL},
    {NULL, NULL, NULL, NULL},
    {(caddr_t)&defAffectCurIfNullSelection, XtRBoolean,
	 "affectCurrentMsgIfNullSelection", NULL},
    {NULL, NULL, NULL, NULL},
    {(caddr_t)&defOpenSubfolders, XtRBoolean, "openSubfoldersWithDrawer", NULL},
    {(caddr_t)&defCloseSubfolders, XtRBoolean, "autoCloseSubfolders", NULL},
    {NULL, NULL, NULL, NULL},
    {(caddr_t)&defBeepIfNoMail, XtRBoolean, "BeepIfNoMail", NULL},
    {NULL, NULL, NULL, NULL},
    {(caddr_t)&defReplyCCAll, RYesNoDefault, "ReplyCCAll", alldef},
    {(caddr_t)&defReplyCCMe, RYesNoDefault, "ReplyCCMe", alldef},
    {NULL, NULL, NULL, NULL},
    {(caddr_t)&defInitialState, XtRInitialState, "InitialState", NULL},
    {NULL, NULL, NULL, NULL},
    {(caddr_t)&defAnnotateReplies, XtRBoolean, "AnnotateReplies"},
    {(caddr_t)&defAnnotateForwards, XtRBoolean, "AnnotateForwards"},
    {NULL, NULL, NULL, NULL},
    {(caddr_t)&newSVNStyle, RYesNoDefault, "LayoutStyle", layoutdef},
    {NULL, NULL, NULL, NULL},

/*    {(caddr_t)&defIncOnShowUnopened, XtRBoolean, "IncOnShowUnopened"}, */
/*    {(caddr_t)&defGrabFocus, XtRBoolean, "GrabFocus"}, */
/*    {(caddr_t)&agingString, XtRString, "WastebasketDaysToKeep"}, */
/*    {(caddr_t)&initialFolderName, XtRString, "InitialFolder"}*/
};

static Widget customwidgetlist[XtNumber(customlist)];

static Widget *customwidgetsublist[XtNumber(customlist)] = {NULL};

/*static Widget customshell = NULL; /* Shell widget for customizing. */
static Widget customadb = NULL; /* Adb containing customizing stuff. */

static void CustomizeDelete()
{
    if (customadb) {
	XtPopdown(GetParent(customadb));
#ifdef NODESTROY
	XtUnmanageChild(GetParent(customadb));
#else
	XtDestroyWidget(GetParent(customadb));
#endif
	customadb = NULL;
    }
}

#define HEADER "# Customizations saved here. Don't add anything after this line"
static void CustomizeSave()
{
    register int i, j;
    Custom custom;
    RadioDesc desc;
    FILE *fid, *fromfid;
    Widget widget, *subwidget;
    char backupName[1024];
    char *buf;

    sprintf(backupName, "%s~", customfile);
    (void) unlink(backupName);
    (void) rename(customfile, backupName);
    if ((fid = myfopen(customfile, "a")) == NULL) {
	Error(scrnList[0]->parent, EFileCantOpen, customfile);
	return;
    }
    fromfid = fopen(backupName, "r");
    if (fromfid != NULL) {
	do {
	    buf = ReadLine(fromfid);
	    if (buf) {
		if (strncmp(buf, HEADER, sizeof(HEADER)) == 0) {
		    buf = NULL;
		    continue;
		}
		(void) fprintf(fid, "%s\n", buf);
	    }
	} while (buf);
	(void) myfclose(fromfid);
    }
    (void) fprintf(fid, "%s\n", HEADER);
    for (i=0, custom = customlist ; i<XtNumber(customlist); i++, custom++) {
	widget = customwidgetlist[i];
	if (custom->type == NULL) continue;
	if (custom->desc) {
	    subwidget = customwidgetsublist[i];
	    for (j=0, desc = custom->desc; desc->str ; j++, desc++) {
		if (XmToggleButtonGetState(subwidget[j])) {
		    *((int *) custom->addr) = desc->value;
		    fprintf(fid, "*%s: %s\n", custom->resource, desc->str);
		    break;
		}
	    }
	} else if (strcmp(custom->type, XtRBoolean) == 0) {
	    *((Boolean *)custom->addr) = XmToggleButtonGetState(widget);
	    (void) fprintf(fid, "*%s: %s\n", custom->resource,
			   (*((Boolean *)custom->addr) ? "on" : "off"));
	} else if (strcmp(custom->type, XtRInitialState) == 0) {
	    if (XmToggleButtonGetState(widget)) {
		*((int *)custom->addr) = IconicState;
	    } else {
		*((int *)custom->addr) = NormalState;
	    }
	    (void) fprintf(fid, "*main.%s: %d\n", custom->resource,
			   *((int *)custom->addr));
	} else if (strcmp(custom->type, XtRString) == 0) {
	    *((char **)custom->addr) = XmTextGetString(widget);
	    (void) fprintf(fid, "*%s: %s\n", custom->resource,
			   *((char **)custom->addr));
	}
    }
    if (newSVNStyle != defSVNStyle)
	Warning(scrnList[0]->parent, WMustRestart, NULL);
    CustomizeDelete();
    (void) myfclose(fid);
    if (defUseWastebasket && TocGetNamed(wastebasketFolderName) == NULL) {
	(void) TocCreateFolder(wastebasketFolderName);
	WastebasketFolder = TocGetNamed(wastebasketFolderName);
	RadioAddFolders(scrnList[0]->folderradio, &wastebasketFolderName, 1,
		       numFolders - 1);
    }
    if (atoi(agingString) != defWastebasketDaysToKeep) {
	defWastebasketDaysToKeep = atoi(agingString);
	if (defUseWastebasket && defWastebasketDaysToKeep)
	    TocExpungeOldMessages(WastebasketFolder, defWastebasketDaysToKeep);
    }
    for (i=0 ; i<numScrns ; i++)
	EnableProperButtons(scrnList[i]);
}


/* ARGSUSED */
static void ToggleCallback(w, param, data)
Widget w;
Opaque param, data;
{
    register int i, j;
    Custom custom;
    RadioDesc desc;
    Widget *subwidget;
    for (i=0, custom = customlist ; i<XtNumber(customlist); i++, custom++) {
	subwidget = customwidgetsublist[i];
	if (subwidget != NULL) {
	    for (j=0, desc = custom->desc; desc->str ; j++, desc++) {
		if (subwidget[j] == w) {
		    for (j=0, desc = custom->desc; desc->str ; j++, desc++) {
			XmToggleButtonSetState(subwidget[j],
						subwidget[j] == w,
						FALSE);
		    }
		    return;
		}
	    }
	}
    }
}
		

void CustomizeCreate()
{
    static Arg titleargs[] = {
	{XmNtopAttachment, (XtArgVal) XmATTACH_FORM},
	{XmNleftAttachment, (XtArgVal) XmATTACH_FORM},
	{XmNrightAttachment, (XtArgVal) XmATTACH_FORM},
    };
    static XtCallbackRec nocallback[2] = {(XtCallbackProc)CustomizeDelete, NULL, NULL};
    static Arg noargs[] = {
	{XmNtopWidget, NULL}, 
	{XmNtopAttachment, (XtArgVal) XmATTACH_WIDGET},
	{XmNleftAttachment, (XtArgVal) XmATTACH_FORM},
	{XmNactivateCallback, (XtArgVal) nocallback},
    };
    static Arg separatorargs[] = {
	{XmNtopWidget, NULL},
	{XmNtopAttachment, (XtArgVal) XmATTACH_WIDGET},
	{XmNleftAttachment, (XtArgVal) XmATTACH_FORM},
	{XmNrightAttachment, (XtArgVal) XmATTACH_FORM},
    };
    static XtCallbackRec yescallback[2] = { (XtCallbackProc)CustomizeSave, NULL, NULL};
    static Arg yesargs[] = {
	{XmNtopWidget, NULL},
	{XmNtopAttachment, (XtArgVal) XmATTACH_WIDGET},
	{XmNrightAttachment, (XtArgVal) XmATTACH_FORM},
	{XmNactivateCallback, (XtArgVal) yescallback},
    };
    static Arg labelargs[] = {
	{XmNtopWidget, NULL},
	{XmNtopAttachment, (XtArgVal) XmATTACH_WIDGET},
	{XmNleftAttachment, (XtArgVal) XmATTACH_FORM},
    };
    static XtCallbackRec togglecallback[2] = {ToggleCallback, NULL, NULL};
    static Arg toggleargs[] = {
	{XmNset, NULL},
	{XmNtopWidget, NULL},
	{XmNindicatorType, NULL},
	{XmNtopAttachment, (XtArgVal) XmATTACH_WIDGET},
	{XmNleftAttachment, (XtArgVal) XmATTACH_FORM},
	{XmNvalueChangedCallback, (XtArgVal) togglecallback},
    };

    Widget widget, lastwidget;
    char str[500];
    register int i, j;
    Custom custom;
    RadioDesc desc;

    TRACE(("@CustomizeCreate\n"));
    CustomizeDelete();

    (void) sprintf(str, "%d", defWastebasketDaysToKeep);
    agingString = MallocACopy(str);

    customadb = XmCreateFormDialog(toplevel, "customize",
					 NULL, (Cardinal) 0);


    lastwidget = XmCreateLabel(customadb, "title",
				titleargs, XtNumber(titleargs));
    XtManageChild(lastwidget);
    for (i=0, custom = customlist ; i<XtNumber(customlist); i++, custom++) {
	if (custom->desc != NULL) {
	    labelargs[0].value = (XtArgVal) lastwidget;
	    widget = XmCreateLabel(customadb, custom->resource,
				    labelargs, XtNumber(labelargs));
	    if (customwidgetsublist[i] == NULL) {
		for (j=0, desc = custom->desc; desc->str ; j++, desc++);
		customwidgetsublist[i] = (Widget *)
		    XtMalloc((unsigned) (sizeof(Widget) * j));
	    }
	    for (j=0, desc = custom->desc; desc->str ; j++, desc++) {
		XtManageChild(widget);
		sprintf(str, "%s%s", custom->resource, desc->str);
		toggleargs[0].value = (XtArgVal)
		    (*((int *) custom->addr) == (desc->value));
		toggleargs[1].value = (XtArgVal) widget;
		toggleargs[2].value = (XtArgVal) XmONE_OF_MANY;
		widget = (Widget) XmCreateToggleButton(customadb, str,
				 toggleargs, XtNumber(toggleargs));
		customwidgetsublist[i][j] = widget;
	    }
	} else if (custom->addr == NULL) {
	    separatorargs[0].value = (XtArgVal) lastwidget;
	    widget = XmCreateSeparator(customadb, "separator",
					separatorargs,
					XtNumber(separatorargs));
	} else if (strcmp(custom->type, XtRBoolean) == 0) {
	    toggleargs[0].value = (XtArgVal) *((Boolean *) custom->addr);
	    toggleargs[1].value = (XtArgVal) lastwidget;
	    toggleargs[2].value = (XtArgVal) XmN_OF_MANY;
	    widget = XmCreateToggleButton(customadb, custom->resource,
					   toggleargs, XtNumber(toggleargs));
	} else if (strcmp(custom->type, XtRInitialState) == 0) {
	    if ((*(int *) custom->addr) == IconicState) {
		toggleargs[0].value = True;
	    } else {
		toggleargs[0].value = False;
	    }
	    toggleargs[0].value = (XtArgVal) *((int *) custom->addr) == IconicState;
	    toggleargs[1].value = (XtArgVal) lastwidget;
	    toggleargs[2].value = (XtArgVal) XmN_OF_MANY;
	    widget = XmCreateToggleButton(customadb, custom->resource,
					   toggleargs, XtNumber(toggleargs));
	} else if (strcmp(custom->type, XtRString) == 0) {
	    static Arg textargs[] = {
		{XmNvalue, NULL},
		{XmNtopWidget, NULL},
		{XmNtopAttachment, (XtArgVal) XmATTACH_WIDGET},
		{XmNleftAttachment, (XtArgVal) XmATTACH_FORM},
/*		{XmNresizable, (XtArgVal) TRUE},   %%% */
	    };
	    labelargs[0].value = (XtArgVal) lastwidget;
	    widget = XmCreateLabel(customadb, custom->resource,
				    labelargs, XtNumber(labelargs));
	    XtManageChild(widget);

	    (void) sprintf(str, "%sText", custom->resource);
	    textargs[0].value = (XtArgVal) *((char **) custom->addr);
	    textargs[1].value = (XtArgVal) widget;
	    widget = (Widget) XmCreateText(customadb, str,
				    textargs, XtNumber(textargs));
	} else Punt("Unknown type in CustomizeCreate!");
	customwidgetlist[i] = widget;
	XtManageChild(widget);
	lastwidget = widget;
    }
    noargs[0].value = (XtArgVal) lastwidget;
    widget = XmCreatePushButton(customadb, "no", noargs, XtNumber(noargs));
    XtManageChild(widget);
    yesargs[0].value = (XtArgVal) lastwidget;
    widget = XmCreatePushButton(customadb, "yes", yesargs, XtNumber(yesargs));
    XtManageChild(widget);

    XtManageChild(customadb);
    widget = GetParent(customadb);
    XtRealizeWidget(widget);
    XtPopup(widget, XtGrabNone);

    /* *  Can't free and reuse!
     * *    XtFree(agingString);
     * */
}
