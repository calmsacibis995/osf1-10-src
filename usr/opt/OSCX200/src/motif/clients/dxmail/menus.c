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
static char rcs_id[] = "@(#)$RCSfile: menus.c,v $ $Revision: 1.1.6.3 $ (DEC) $Date: 1993/08/02 23:58:28 $";
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
/* menus.c -- management of menus. */

#include "decxmail.h"
#include "button.h"
#include "radio.h"
#include "toc.h"
#include "tocintrnl.h"
#include "actionprocs.h"
#include "Shell.h"
#include <Xm/Form.h>
#include <Xm/PanedW.h>
#include <Xm/Frame.h>
#include <Xm/RowColumn.h>
#include <Xm/CascadeB.h>
#include <Xm/Separator.h>
#include <Xm/SeparatoG.h>
#include <Mrm/MrmAppl.h>
#include <DXm/DXmHelpB.h>
/* #include <DXm/DXmSvn.h> included in externs.h */
#include <Xm/ScrolledW.h>
#include <Xm/Label.h>
#include <Xm/MainW.h>		/*SM*/
#include <X11/cursorfont.h>
#include <decwcursor.h>

typedef struct _LayoutRec {
    char *name;			/* Name of this layout rec. */
    int length;			/* Allocated string space for data. */
    char *data;			/* String for this layout rec. */
    char *ptr;			/* Where we currently are in the rec. */
    struct _LayoutRec *next;
} LayoutRec, *Layout;

static XrmQuark SMenu = 0, SButtonBox, STocLabel, SSeparator,
		SFolderRadio, SReadRadio, SSeqRadio, STocText, SViewText,
		SCompText, SPopUp, SColumns, SButton, SScroll, SHelpMenu,
		SOutline;

static XtTranslations resizeTranslations = (XtTranslations) NULL;
static char resize[] = "<ConfigureNotify>: resize-folder-box()";

extern Pixmap OpenFolderPixmap;
extern Pixmap ClosedFolderPixmap;
extern Pixmap OpenTrashPixmap;
extern Pixmap ClosedTrashPixmap;
extern Pixmap OpenDrawerPixmap;
extern Pixmap ClosedDrawerPixmap;

static char *SkipWhiteSpace(ptr)
char *ptr;
{
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    return ptr;
}

static Layout LoadLayout(name)
char *name;
{
    Layout layout, insert;
    char rname[100], rclass[100], temp[100];
    char *type, *ptr, *ptr2, *ptr3, *stuff;
    XrmValue value;

    layout = XtNew(LayoutRec);
    layout->name = MallocACopy(name);
    (void) sprintf(rname, "%s.layout.%s", progName, name);
    (void) sprintf(rclass, "%s.Layout.LayoutInfo", PROGCLASS);
    XrmGetResource(DefaultDB, rname, rclass, &type, &value);
    ptr = (char *) value.addr;
    if (ptr == NULL || *ptr == '\0') {
	(void) fprintf(stderr, "Can't find layout info.  Most likely, the initialization files have\n");
	(void) fprintf(stderr, "not been installed in /usr/lib/X11/app-defaults.\n");
	exit(1);
    }
    layout->data = MallocACopy(ptr);
    layout->length = strlen(layout->data);
    do {
	ptr = index(layout->data, '[');
	if (ptr) {
	    ptr2 = index(ptr, ']');
	    if (ptr2) {
		(void) strncpy(temp, ptr+1, ptr2 - ptr);
		temp[(ptr2 - ptr) - 2] = 0;
		insert = LoadLayout(temp);
		stuff = XtMalloc((unsigned) (layout->length + insert->length -
					     (ptr2 - ptr) + 10));
		ptr3 = stuff;
		bcopy(layout->data, ptr3, ptr - layout->data);
		ptr3 += ptr - layout->data;
		bcopy((char *) insert->data, ptr3, insert->length);
		ptr3 += insert->length;
		bcopy(ptr2+1, ptr3, layout->length - (ptr2+1 - layout->data));
		ptr3 += (ptr2+1 - layout->data);
		*ptr3 = 0;
		XtFree(layout->data);
		layout->data = stuff;
		XtFree(insert->data);
		XtFree(insert->name);
		XtFree((char *) insert);
	    }
	}
    } while (ptr && ptr2);
    layout->ptr = layout->data;
    layout->next = NULL;
    return layout;
}


static Layout FindLayout(name)
char *name;
{
    static Layout first = NULL;
    Layout result;
    for (result = first ; result != NULL ; result = result->next)
	if (strcmp(result->name, name) == 0) break;
    if (result == NULL) {
	result = LoadLayout(name);
	result->next = first;
	first = result;
    }
    result->ptr = result->data;
    return result;
}
      


static char *GetALine(layout)
Layout layout;
{
    static char *result = NULL;
    static int maxlength = 0;
    register char *ptr;
    int length;
    if (layout->ptr == NULL || *layout->ptr == '\0') return NULL;
    ptr = index(layout->ptr, '\n');
    if (ptr == NULL) {
	ptr = layout->ptr;
	layout->ptr = NULL;
	return ptr;
    }
    length = ptr - layout->ptr;
    if (maxlength <= length) {
	maxlength = length + 1;
	result = XtRealloc(result, (unsigned) maxlength);
    }
    bcopy(layout->ptr, result, length);
    result[length] = '\0';
    while (*ptr == '\n') ptr++;
    layout->ptr = ptr;
    return result;
}


/*
 * Find the next word as pointed by ptr.  The word is put into the word
 * parameter, and the function returns a pointer to the next word (or a
 * pointer to NULL if there are no more words.)
 */

static char *FindOneWord(ptr, word)
char *ptr, *word;
{
    register int count;
    ptr = SkipWhiteSpace(ptr);
    if (*ptr == '"') {
	ptr++;
	while (*ptr && *ptr != '"') *word++ = *ptr++;
	if (*ptr) ptr++;
    } else {
	count = 0;
	while (*ptr && (count > 0 ||
			(*ptr != ' ' && *ptr != '\t' && *ptr != ':'))) {
	    if (*ptr == '(') count++;
	    if (*ptr == ')' && count > 0) count--;
	    *word++ = *ptr++;
	}
    }
    *word = 0;
    return SkipWhiteSpace(ptr);
}


ParseToFuncAndParams(ptr, func, params, num_params)
char *ptr;
XtActionProc *func;		/* RETURN */
char ***params;			/* RETURN */
Cardinal *num_params;		/* RETURN */
{
    char *ptr2, temp[50], **p;
    ptr2 = temp;
    *num_params = 0;
    while (*ptr && *ptr != '(') *ptr2++ = *ptr++;
    *ptr2 = 0;
    *func = NameToFunc(temp);
    p = NULL;
    if (*ptr) {
	ptr++;			/* Skip the paren */
	while (*ptr && *ptr != ')') {
	    ptr = SkipWhiteSpace(ptr);
	    ptr2 = temp;
	    while (*ptr && *ptr != ')' && *ptr != ',')
		if (*ptr != ' ' && *ptr != '\t') *ptr2++ = *ptr++;
		else ptr++;
	    if (ptr2 > temp) {
		(*num_params)++;
		p = (char **)
		    XtRealloc((char *) p,
			      (unsigned) *num_params * sizeof(char *));
		*ptr2 = 0;
		p[*num_params - 1] = MallocACopy(temp);
	    }
	    if (*ptr == ',') ptr++;
	}
    }
    *params = p;
}
    
static Boolean ResetSkipAdjust(w)
Widget	w;
{	
    static Arg skipArg[] = {
	{XmNskipAdjust, False},
    };

    XtSetValues(w, skipArg, 1);
    return True;
}

static void AttachSource(widget)
Widget widget;
{
    TagRec *tags;
    int i, pos, wastePos;

    InitialFolder->opened = True;
    if (defSVNStyle) {
	tags = (TagRec *) XtMalloc(numFolders * sizeof (TagRec));
	pos = 2;
	wastePos = 0;
	for (i = 0; i < numFolders; i++) {
	    if (folderList[i] != InitialFolder &&
		folderList[i] != DraftsFolder &&
		folderList[i] != WastebasketFolder) {
		if (folderList[i]->visible) {
		    tags[pos].tagFields.tocNumber = i;
		    tags[pos].tagFields.msgNumber = 0;
		    pos++;
		}
	    } else {
		if (folderList[i] == InitialFolder) {
		    tags[0].tagFields.tocNumber = i;
		    tags[0].tagFields.msgNumber = 0;
		}
		if (folderList[i] == DraftsFolder) {
		    tags[1].tagFields.tocNumber = i;
		    tags[1].tagFields.msgNumber = 0;
		}
		if (folderList[i] == WastebasketFolder) {
		    wastePos = i;
		}
	    }
 	}
	if (wastePos != 0) {
	    tags[pos].tagFields.tocNumber = wastePos;
	    tags[pos].tagFields.msgNumber = 0;
	    pos++;
	}
	DXmSvnAddEntries(widget, 0, pos, 0, tags, True);
	XtFree((char *)tags);
    }
}

static void GetSvnEntry(widget, closure, data)
Widget widget;
int closure;
DXmSvnCallbackStruct *data;
{
#define pix_width 16
#define pix_offset 2
    int entry_number = data->entry_number;
    Scrn scrn = ScrnFromWidget(widget);
    Toc toc;
    Msg msg;
    XmString str;
    static Pixmap pixmap, lastPixmap = (Pixmap) NULL;
    int i, pos;
    TagRec tag;
    char tocName[1024];
    char *ptr;

    InitFolderPixmaps(widget);
    tag.tagValue = data->entry_tag;

    toc = folderList[tag.tagFields.tocNumber];

    if (tag.tagFields.msgNumber != 0) {
	msg = toc->msgs[tag.tagFields.msgNumber - 1];
	        if (msg == NULL) {
		  DEBUG(("menus.c:GetSVNentry(): Invalid message address.\n"));
		  return ;
	} 
	toc = msg->toc;
	msg->index = entry_number;
	DXmSvnSetEntryNumComponents(widget, entry_number, 2);
	pixmap = MsgGetPixmap(toc, msg);
	if (pixmap != NULL && lastPixmap == NULL) lastPixmap = pixmap;
	if (pixmap == NULL) pixmap = lastPixmap;
	msg->buttonPixmap = pixmap;
	DXmSvnSetComponentPixmap(widget, entry_number, 1,
#ifdef notdef
		pix_offset, pix_offset, pixmap, pix_width, pix_width);
#else
		0, 0, pixmap, pix_width, pix_width);
#endif
	ptr = strchr(msg->curbuf, '\n');
	if (ptr)
	    *ptr = '\0';
	str = XmStringCreateSimple(msg->curbuf);
	if (ptr)
	    *ptr = '\n';
	DXmSvnSetComponentText(widget, entry_number, 2,
		pix_width + pix_offset, 0, str, NULL);
	XmStringFree(str);
    } else { /* not, thus toc */
	toc->index = entry_number;
	DXmSvnSetEntryNumComponents(widget, data->entry_number, 2);
	if (toc == WastebasketFolder) {
	    if (toc == scrn->toc) {
		DXmSvnSetComponentPixmap(widget, data->entry_number, 1,
			0, 0, OpenTrashPixmap, pix_width, pix_width);
	    } else {
		DXmSvnSetComponentPixmap(widget, data->entry_number, 1,
			0, 0, ClosedTrashPixmap, pix_width, pix_width);
	    } 
	} else {
	    if (toc == scrn->toc) {
		if (TocSubFolderCount(toc)) {
		    DXmSvnSetComponentPixmap(widget, data->entry_number, 1,
			0, 0, OpenDrawerPixmap, pix_width, pix_width);
		} else {
		    DXmSvnSetComponentPixmap(widget, data->entry_number, 1,
			0, 0, OpenFolderPixmap, pix_width, pix_width);
		}
	    } else {
		if (TocSubFolderCount(toc)) {
		    DXmSvnSetComponentPixmap(widget, data->entry_number, 1,
			0, 0, ClosedDrawerPixmap, pix_width, pix_width);
		} else {
		    DXmSvnSetComponentPixmap(widget, data->entry_number, 1,
			0, 0, ClosedFolderPixmap, pix_width, pix_width);
		}
	    }
	}
	if (toc->mailpending) {
	    sprintf(tocName, "%s -- New Mail -- ", TocGetFolderName(toc));
	    str = XmStringCreateSimple(tocName);
	} else {
	    str = XmStringCreateSimple(TocGetFolderName(toc));
	}
	DXmSvnSetComponentText(widget, data->entry_number, 2,
		pix_width + 4, 0, str, NULL);
	XmStringFree(str);
    }
}
static void EntryDoubleClick(widget, closure, data)
Widget widget;
int closure;
DXmSvnCallbackStruct *data;
{
    Msg msg;
    TagRec tag;
    Scrn scrn = ScrnFromWidget(widget);
    Toc toc;
    char *name;
    static char *autoOpened = (char *) NULL;
    char currentParent[512], autoParent[512];
    char *ptr;
    TagRec changeTag;
    int pos;
    int msgNumber;

    tag.tagValue = data->entry_tag;
    toc = folderList[tag.tagFields.tocNumber];
    if (tag.tagFields.msgNumber != 0) {
	msgNumber = tag.tagFields.msgNumber - 1;
	if (msgNumber >= toc->nummsgs) msgNumber = toc->nummsgs -1;
	msg = toc->msgs[msgNumber];
	ExecViewMsg(widget, msg);
    } else {
	if (scrn->toc && scrn->toc == toc) {
	    toc->opened = False;
	    TocSetScrn(NULL, scrn);
	    pos = DXmSvnGetEntryNumber(scrn->tocwidget, data->entry_tag);
	    if (pos != 0)
		DXmSvnInvalidateEntry(scrn->tocwidget, pos);
	    return;
	}
	if (scrn->toc) {
	    TocSetScrn(NULL, scrn);
	    pos = DXmSvnGetEntryNumber(scrn->tocwidget, data->entry_tag);
	    if (pos != 0)
		DXmSvnInvalidateEntry(scrn->tocwidget, pos);
	    if (scrn->tocwidget) {
	        changeTag.tagFields.tocNumber = TUGetTocNumber(scrn->toc);
	        changeTag.tagFields.msgNumber = 0;
	        pos = DXmSvnGetEntryNumber(scrn->tocwidget, changeTag.tagValue);
		if (pos != 0)
		    DXmSvnInvalidateEntry(scrn->tocwidget, pos);
	    }
	    TocSetScrn(scrn->toc, NULL);
	}
	name = TocGetFolderName(toc);
	if (toc == scrn->toc) {
	    TocSetScrn(toc, NULL);
	    toc->opened = False;
	    if (scrn->tocwidget) {
	        changeTag.tagFields.tocNumber = TUGetTocNumber(toc);
	        changeTag.tagFields.msgNumber = 0;
	        pos = DXmSvnGetEntryNumber(scrn->tocwidget, changeTag.tagValue);
		if (pos != 0)
		    DXmSvnInvalidateEntry(scrn->tocwidget, pos);
	    }
	} else {
	    toc->opened = True;
	    if (scrn->tocwidget) {
	        changeTag.tagFields.tocNumber = TUGetTocNumber(toc);
	        changeTag.tagFields.msgNumber = 0;
	        pos = DXmSvnGetEntryNumber(scrn->tocwidget, changeTag.tagValue);
		if (pos != 0)
		    DXmSvnInvalidateEntry(scrn->tocwidget, pos);
	    }
	    if (defCloseSubfolders && autoOpened != NULL) {
	        strcpy(currentParent, name);
	        if ((ptr = index(currentParent, '/')) != NULL) {
		    *ptr = '\0';
	        }
	        strcpy(autoParent, autoOpened);
	        if ((ptr = index(autoParent, '/')) != NULL) {
		    *ptr = '\0';
	        }
	        if (strcmp(autoParent, currentParent) != 0) {
		    TocHideSubfolders(widget, autoOpened);
		    XtFree(autoOpened);
		    autoOpened = (char *) NULL;
	        }
	    }
	    ExecOpenFolder(widget);
	    TocChangeViewedSeq(toc, TocGetSeqNamed(toc, "all"));
	    if (defOpenSubfolders) {
	        if (TocSubFolderCount(toc)) {
		    TocRevealSubfolders(widget, name);
		    if (autoOpened == NULL) autoOpened = XtNewString(name);
	        }
	    }
	}
    }
}
static void SelectedCallback(widget, closure, data)
Widget widget;
int closure;
DXmSvnCallbackStruct *data;
{
    Scrn scrn = ScrnFromWidget(widget);
    TagRec tag;

    tag.tagValue = data->entry_tag;
    if (tag.tagFields.msgNumber != 0)
	EnableProperButtons(scrn);
}
static void DraggedCallback(widget, closure, data)
Widget widget;
int closure;
DXmSvnCallbackStruct *data;
{
    Scrn scrn = ScrnFromWidget(widget);
    TagRec tag;
    Toc toc;

    tag.tagValue = DXmSvnGetEntryTag(widget, data->entry_number);
    toc = folderList[tag.tagFields.tocNumber];

    if (toc == WastebasketFolder) {
	ExecMarkDelete(widget);
    } else {
	LastPopupToc = toc;
	ExecMarkMove(widget);
	LastPopupToc = (Toc) NULL;
    }
}
static Widget CreateTocText(scrn, name)
Scrn scrn;
char *name;
{
    static XtCallbackRec SvnHelpCB[2] =
	{(XtCallbackProc)ExecPopupHelpMenu, "OnWindow OnMainOutline",  NULL, NULL};
    static XtCallbackRec SvnConfirmedCB[2] =
	{(XtCallbackProc)EntryDoubleClick, NULL, NULL, NULL};
    static XtCallbackRec SvnAttachCB[2] =
	{(XtCallbackProc)AttachSource, NULL, NULL, NULL};
    static XtCallbackRec SvnGetEntryCB[2] =
	{(XtCallbackProc)GetSvnEntry, NULL,  NULL, NULL};
    static XtCallbackRec SvnExtendedCB[2] =
	{(XtCallbackProc)SelectedCallback, NULL, NULL, NULL};
    static XtCallbackRec SvnSelectedCB[2] =
	{(XtCallbackProc)SelectedCallback, NULL, NULL, NULL};
    static XtCallbackRec SvnUnselectedCB[2] =
	{(XtCallbackProc)SelectedCallback, NULL, NULL, NULL};
    static XtCallbackRec SvnDraggedCB[2] =
	{(XtCallbackProc)DraggedCallback, NULL, NULL, NULL};
    static XtCallbackRec SvnPopupMenuCB[2] =
	{(XtCallbackProc)PopupSvnMenu, NULL, NULL, NULL};

    static Arg panedSvnArgs[] = {
	{DXmSvnNuseScrollButtons,		False},
	{DXmSvnNexpectHighlighting,		False},
	{DXmSvnNdisplayMode,			DXmSvnKdisplayOutline},
	{DXmSvnNselectionMode,			DXmSvnKselectEntry},
	{DXmSvnNselectAndConfirmCallback,	(XtArgVal) SvnConfirmedCB},
	{DXmSvnNattachToSourceCallback,		(XtArgVal) SvnAttachCB},
	{DXmSvnNgetEntryCallback,		(XtArgVal) SvnGetEntryCB},
	{XmNhelpCallback,			(XtArgVal) SvnHelpCB},
	{DXmSvnNhelpRequestedCallback,		(XtArgVal) SvnHelpCB},
	{DXmSvnNextendConfirmCallback,		(XtArgVal) SvnExtendedCB},
	{DXmSvnNentrySelectedCallback,		(XtArgVal) SvnSelectedCB},
	{DXmSvnNentryUnselectedCallback,	(XtArgVal) SvnUnselectedCB},
	{DXmSvnNpopupMenuCallback,		(XtArgVal) SvnPopupMenuCB},
    };

    static Arg outlineSvnArgs[] = {
	{DXmSvnNuseScrollButtons,		True},
	{DXmSvnNexpectHighlighting,		False},
	{DXmSvnNdisplayMode,			DXmSvnKdisplayOutline},
	{DXmSvnNselectionMode,			DXmSvnKselectEntry},
	{DXmSvnNselectAndConfirmCallback,	(XtArgVal) SvnConfirmedCB},
	{DXmSvnNattachToSourceCallback,		(XtArgVal) SvnAttachCB},
	{DXmSvnNgetEntryCallback,		(XtArgVal) SvnGetEntryCB},
	{DXmSvnNhelpRequestedCallback,		(XtArgVal) SvnHelpCB},
	{XmNhelpCallback,			(XtArgVal) SvnHelpCB},
	{DXmSvnNextendConfirmCallback,		(XtArgVal) SvnExtendedCB},
	{DXmSvnNentrySelectedCallback,		(XtArgVal) SvnSelectedCB},
	{DXmSvnNentryUnselectedCallback,	(XtArgVal) SvnUnselectedCB},
	{DXmSvnNselectionsDraggedCallback,	(XtArgVal) SvnDraggedCB},
	{DXmSvnNpopupMenuCallback,		(XtArgVal) SvnPopupMenuCB},
    };

    Widget svn;
    Widget frame;

    frame = XmCreateFrame(scrn->widget, "tocFrame", NULL, 0);
    XtManageChild(frame);
    if (!defSVNStyle) {
	svn = DXmCreateSvn (frame, "tocList",
			panedSvnArgs, XtNumber(panedSvnArgs));
    } else {
	svn = DXmCreateSvn (frame, "outlineList",
			outlineSvnArgs, XtNumber(outlineSvnArgs));
    }
    XtAddWorkProc((XtWorkProc)ResetSkipAdjust, frame);
    scrn->tocwidget = svn;
    XtManageChild(svn);
    CreateWidgetFromLayout(scrn, svn, "tocbuttonpopup");
    if (defSVNStyle)
	CreateWidgetFromLayout(scrn, svn, "folderbuttonpopup");
    return svn;
}

static Boolean WorkDialogBoxUngrabTab(data)
Opaque data;
{
    Widget widget = (Widget) data;
    XUngrabKey(XtDisplay(widget),
	       XKeysymToKeycode(XtDisplay(widget), 0xFF09), /* tab */
	       AnyModifier,
	       XtWindow(widget));
    return TRUE;
}

static Widget CreateViewText(scrn, name)
Scrn scrn;
char *name;
{
    Widget adb;
    static Arg textargs[] = {
	{XmNleftAttachment, (XtArgVal) XmATTACH_FORM},
	{XmNrightAttachment, (XtArgVal) XmATTACH_FORM},
	{XmNtopAttachment, (XtArgVal) XmATTACH_FORM},
	{XmNbottomAttachment, (XtArgVal) XmATTACH_FORM},
	{XmNeditable, (XtArgVal) True},
	{XmNeditMode, (XtArgVal) XmMULTI_LINE_EDIT},
	{XmNpendingDelete, (XtArgVal) True},
    };
    adb = XmCreateForm(scrn->widget, "messageArea", NULL, (Cardinal) 0);
    XtAddWorkProc(WorkDialogBoxUngrabTab, (Opaque) adb);
    scrn->viewwidget = XmCreateScrolledText(adb, name,
				      textargs, XtNumber(textargs));
    XtManageChild(scrn->viewwidget);
    return adb;
}
    
static Boolean WorkRealizeWidget(data)
Opaque data;
{
    if (!CheckWorkOK()) return FALSE;
    XtRealizeWidget((Widget) data);
    return TRUE;
}


static Boolean WorkAddFolderPopup(data)
Opaque data;
{
    Widget parent = (Widget) data;
    CreateWidgetFromLayout(ScrnFromWidget(parent), parent,
			   "folderbuttonpopup");
    return TRUE;
}

/* Expect a number of forms:   (whitespace as '___')
 *   either a comment line: first non-white-space-character '!' or '#',
 *    or    a line as  '___name___[:]___type___[:]___action()'
 *    or    a line as  '___name___[:]___type()'
 *           (for which 'action' becomes 'type' and 'type' becomes "button")
 */
static Widget CreateWidget(scrn, layout, parent, ptr)
Scrn scrn;
Layout layout;
Widget parent;
char *ptr;
{
    char name[500], type[500], action[500];
    XrmQuark signature;
    Widget widget, subwidget;
    Popup popup;
    Button button;
    XtActionProc func;
    char **params;
    Cardinal num_params;
    Arg	al[13];
    int	ac = 0;
    Dimension height;
    static XtCallbackRec folderHelpCallback[2] =
	{(XtCallbackProc)ExecPopupHelpMenu, "OnFolders",
	 NULL, NULL};

    if (resizeTranslations == (XtTranslations) NULL) {
	resizeTranslations = XtParseTranslationTable(resize);
    }
    ptr = SkipWhiteSpace(ptr);
    if (*ptr == '#' || * ptr == '!') return NULL;
    ptr = FindOneWord(ptr, name);
    ptr = SkipWhiteSpace(ptr);
    if (*ptr == ':') ptr++;
    ptr = FindOneWord(ptr, type);
    if (index(type, '(')) {		/* SHORT-CUT FOR 'Button' WIDGETS! */
	(void) strcpy(action, type);
	(void) strcpy(type, "button");
    } else {
	if (*ptr == ':') ptr++;
	ptr = FindOneWord(ptr, action);
    }
    if (name[0] == 0 || type[0] == 0)
	return NULL;

    signature = XrmStringToQuark(type);
    if (signature == SMenu || signature == SHelpMenu ||
	signature == SButtonBox || signature == SPopUp ||
	signature == SColumns || signature == SScroll) {

/* If the TYPE is a MENU, create the PULL-RIGHT menu WIDGET. */
	if (signature == SMenu || signature == SHelpMenu) {

/* Make all PULL-DOWN menus whose parent is the SCREEN'S main WORK-AREA
 * belong instead to the SCREEN's main MENU-BAR.
 */
	    if (parent == scrn->widget)
		parent = scrn->menu;

	    ac = 0;
	    XtSetArg(al[ac], XmNx, 0); ac++;
	    XtSetArg(al[ac], XmNy, 0); ac++;
	    XtSetArg(al[ac], XmNrowColumnType, XmMENU_PULLDOWN); ac++;
	    XtSetArg(al[ac], XmNorientation, XmVERTICAL); ac++;
	    XtSetArg(al[ac], XmNentryCallback, NULL); ac++;
	    XtSetArg(al[ac], XmNmapCallback, NULL); ac++;
	    XtSetArg(al[ac], XmNhelpCallback, NULL); ac++;
	    widget = XmCreatePulldownMenu(parent,
			name,
			al, ac);
	    XtAddWorkProc(WorkRealizeWidget, (Opaque) widget);
	} else if (signature == SPopUp) {
	    if (!XtIsRealized(parent)) {
		if (!XtIsRealized(scrn->parent))
		    XtRealizeWidget(scrn->parent);
		if (!XtIsRealized(parent))
		    XtRealizeWidget(parent);
	    }
	    ac = 0;
	    XtSetArg(al[ac], XmNx, 0); ac++;
	    XtSetArg(al[ac], XmNy, 0); ac++;
	    XtSetArg(al[ac], XmNorientation, XmVERTICAL); ac++;
	    XtSetArg(al[ac], XmNentryCallback, NULL); ac++;
	    XtSetArg(al[ac], XmNmapCallback, NULL); ac++;
	    XtSetArg(al[ac], XmNhelpCallback, NULL); ac++;
	    XtSetArg(al[ac], XmNrowColumnType, XmMENU_POPUP);ac++;
	    widget = XmCreatePopupMenu(parent,
			name,
			al, ac);
	    XtAddWorkProc(WorkRealizeWidget, (Opaque) widget);
	    XtAddCallback(GetParent(widget), XtNpopdownCallback,
			  (XtCallbackProc)PopupFinished, NULL);
	} else if (signature == SButtonBox) {
	    ac = 0;
	    XtSetArg(al[ac], XmNresizeHeight, True);ac++;
	    XtSetArg(al[ac], XmNresizeWidth, False);ac++;
	    XtSetArg(al[ac], XmNadjustLast, False);ac++;
	    XtSetArg(al[ac], XmNorientation, XmHORIZONTAL);ac++;
	    XtSetArg(al[ac], XmNnumColumns, 1);ac++;
	    XtSetArg(al[ac], XmNpacking, XmPACK_TIGHT);ac++;
	    XtSetArg(al[ac], XmNisHomogeneous, False);ac++;
	    widget = XmCreateRowColumn(parent, name, al, ac);
	    height = GetHeight(widget);
	    height *= 2;
	    ac = 0;
	    XtSetArg(al[ac], XmNpaneMinimum, height);ac++;
	    XtSetArg(al[ac], XmNpaneMaximum, height);ac++;
	    XtSetValues(widget, al, ac);
	} else if (signature == SColumns) {
	    ac = 0;
	    XtSetArg(al[ac], XmNborderWidth, 0);ac++;
	    XtSetArg(al[ac], XmNorientation, XmVERTICAL);ac++;
	    XtSetArg(al[ac], XmNpacking, XmPACK_COLUMN);ac++;
	    XtSetArg(al[ac], XmNresizeWidth, True);ac++;
	    XtSetArg(al[ac], XmNresizeHeight, True);ac++;
	    XtSetArg(al[ac], XmNnumColumns, 5);ac++;
	    XtSetArg(al[ac], XmNadjustLast, False);ac++;
	    XtSetArg(al[ac], XmNhelpCallback, folderHelpCallback);ac++;
	    scrn->folderwidget = widget =
		XtCreateWidget(name, xmRowColumnWidgetClass, parent, al, ac);
	    XtAddWorkProc((XtWorkProc)ResizeFolderBox, parent);
	} else if (signature == SScroll) {
	    Widget hScrollBar;
	    ac = 0;
	    XtSetArg(al[ac], XmNborderWidth, 0);ac++;
	    XtSetArg(al[ac], XmNscrollingPolicy, XmAUTOMATIC);ac++;
	    XtSetArg(al[ac], XmNvisualPolicy, XmCONSTANT);ac++;
	    XtSetArg(al[ac], XmNscrollBarDisplayPolicy, XmSTATIC);ac++;
	    widget = XtCreateWidget(name, xmScrolledWindowWidgetClass,
					parent, al, ac);
	    XtOverrideTranslations(widget, resizeTranslations);
	    ac = 0;
	    XtSetArg(al[ac], XmNhorizontalScrollBar, &hScrollBar); ac++;
	    XtGetValues(widget, al, ac);
	    XtUnmanageChild(hScrollBar);
	}
/* For these menu-type TYPES, read and process LAYOUT lines until a matching
 * END type is found: note the recursive call to CreateWidget for this!
 */
	for (;;) {
	    ptr = GetALine(layout);
	    if (ptr == NULL) return widget;
	    ptr = SkipWhiteSpace(ptr);

/* If the TYPE was a MENU, create a MENU-ENTRY in the PARENT that will
 * PULL-RIGHT the above-created PULL-RIGHT menu when MB1-ACTIVATED.
 */
	    if ((signature == SMenu || signature == SHelpMenu) &&
		strncmp(ptr, "endmenu", 7) == 0) {
		static XtCallbackRec helpCallback[2] =
			{(XtCallbackProc)ExecPopupHelpMenu, "OnPromptDialog",
			NULL, NULL};
		static Arg arglist[] = {
		    {XmNsubMenuId, (XtArgVal) NULL},
		    {XmNhelpCallback, (XtArgVal) helpCallback},
		};
		char rname[100];
		sprintf(rname, "%smenubar.%s.helpTopic", scrn->name,
					name);
		helpCallback[0].closure = (caddr_t)
			GetApplicationResourceAsString(rname, "Help Topic");
		arglist[0].value = (XtArgVal) widget;
		widget = XmCreateCascadeButton(parent, name, arglist,
						   (int)XtNumber(arglist));
		if (signature == SHelpMenu) {
		    static Arg helparg[] = {
			{XmNmenuHelpWidget, (XtArgVal) NULL},
		    };
		    helparg[0].value = (XtArgVal) widget;
		    XtSetValues(parent, helparg, XtNumber(helparg));
		}
		if (action[0]) {
		    button = ButtonMake(widget);
		    ParseToFuncAndParams(action, &func, &params, &num_params);
		    ButtonAddFunc(button, func, params, num_params);
		}
		break;
	    }
	    if (signature == SPopUp && strncmp(ptr, "endpopup", 8) == 0) {
		popup = XtNew(PopupRec);
		popup->name = MallocACopy(name);
		popup->widget = widget;
		popup->fixed = FALSE;
		popup->next = scrn->firstpopup;
		scrn->firstpopup = popup;
		widget = NULL; /* We don't want this managed yet. */
		break;
	    }
	    if ((signature == SScroll && strncmp(ptr, "endscroll", 9) == 0)   ||
		(signature == SColumns && strncmp(ptr,"endcolumns", 10) == 0) ||
	        (signature == SButtonBox && strncmp(ptr,"endbuttonbox",12) ==0))
	    {
		XtManageChild(widget);
		break;
	    }
	    subwidget = CreateWidget(scrn, layout, widget, ptr);
	    if (subwidget) {
		if (signature == SButtonBox) {
		    ac = 0;
		    height = GetHeight(subwidget) + 5;
		    XtSetArg(al[ac], XmNpaneMinimum, height);ac++;
		    XtSetArg(al[ac], XmNpaneMaximum, height);ac++;
		    XtSetValues(widget, al, ac);
		}
		XtManageChild(subwidget);
	    }
	} /* END-OF-for-LOOP */

    } /* END-OF-menu/buttonBox/popUp/column/scroll-LAYOUT */

    else if (signature == STocLabel) {
	widget = CreateTitleBar(scrn, name);
	scrn->toclabel = widget;
    } else if (signature == SFolderRadio) {
	RadioAddWidget(scrn->folderradio, parent, (XtActionProc) NULL,
			(char **) NULL, (Cardinal) NULL, True);
	XtAddWorkProc(WorkAddFolderPopup, (Opaque) parent);
	widget = NULL;
    } else if (signature == SSeqRadio) {
	ParseToFuncAndParams(action, &func, &params, &num_params);
	RadioAddWidget(scrn->seqradio, parent, func, params, num_params, False);
	widget = NULL;
    } else if (signature == SReadRadio) {

/* Add this parent widget as a new parent widget for the Read-Windows
 * RADIO box.
 */
	ParseToFuncAndParams(action, &func, &params, &num_params);
	RadioAddWidget(ReadWindowsRadio, parent, func, params, num_params,False);
	widget = NULL;
    } else if (signature == STocText) {
	widget = CreateTocText(scrn, name); 
    } else if (signature == SOutline) {
	widget = CreateTocText(scrn, name); 
    } else if (signature == SViewText || signature == SCompText) {
	widget = CreateViewText(scrn, name);
    } else if (signature == SSeparator) {
	widget = XmCreateSeparator(parent, name, NULL, 0);
    } else if (signature == SButton) {
	button = ButtonCreate(scrn, parent, name);
	do {
	    ParseToFuncAndParams(action, &func, &params, &num_params);
	    ButtonAddFunc(button, func, params, num_params);
	    ptr = SkipWhiteSpace(ptr);
	    ptr = FindOneWord(ptr, action);
	} while (action[0]);
	widget = ButtonGetWidget(button);
    }
    return widget;
}


/*
 * Change a layout name into actual widgets.
 */

void CreateWidgetFromLayout(scrn, parent, name)
Scrn scrn;
Widget parent;
char *name;
{
    Layout layout;
    char *ptr;
    Widget widget;
    layout = FindLayout(name);
    do {
	ptr = GetALine(layout);
	if (ptr != NULL) {
	    widget = CreateWidget(scrn, layout, parent, ptr);
	    if (widget)
		XtManageChild(widget);
	}
    } while (ptr != NULL);
}


/*
 * Fill in the 'menu' field in the given screen from a menu created according
 * to the layout stored under name.
 */

void MakeMenu(scrn, name)
     Scrn scrn;
     char *name;
{
  Toc toc;
  char **names;	/* Names for creating radio buttons */
  char *snames[2];
  char rname[100];
  register int i, p;
  static Radio folderradio = NULL;
  
  DEBUG(("In MakeMenu...\n"));
  DEBUG(("Quarking..."));
  if (SMenu == 0) {
    SMenu = XrmStringToQuark("menu");
    SHelpMenu = XrmStringToQuark("helpmenu");
    SPopUp = XrmStringToQuark("popup");
    SButtonBox = XrmStringToQuark("buttonbox");
    STocLabel = XrmStringToQuark("toclabel");
    SFolderRadio = XrmStringToQuark("folderradio");
    SReadRadio = XrmStringToQuark("readradio");
    SSeparator = XrmStringToQuark("separator");
    SSeqRadio = XrmStringToQuark("seqradio");
    STocText = XrmStringToQuark("toctext");
    SViewText = XrmStringToQuark("viewtext");
    SCompText = XrmStringToQuark("comptext");
    SColumns = XrmStringToQuark("columns");
    SButton = XrmStringToQuark("button");
    SScroll = XrmStringToQuark("scroll");
    SOutline = XrmStringToQuark("outline");
  }
  DEBUG(("Done.\n"));
  names = (char **) XtMalloc((unsigned) numFolders * sizeof(char *));
  p = 0;
  names[p++] = TocGetFolderName(InitialFolder);
  names[p++] = TocGetFolderName(DraftsFolder);
  for (i=0 ; i<numFolders ; i++) {
    toc = folderList[i];
    if (toc != InitialFolder && toc != DraftsFolder &&
	(!defUseWastebasket || toc != WastebasketFolder) &&
	TocGetVisible(toc)) {
      names[p++] = TocGetFolderName(toc);
    }
  }
  if (defUseWastebasket) {
    names[p++] = TocGetFolderName(WastebasketFolder);
  }
  DEBUG(("Radio Create...\n"));
  folderradio = RadioCreate();
  if (!defSVNStyle)
    RadioAddFolders(folderradio, names, p, 0);
  XtFree((char *) names);
  
  scrn->folderradio = folderradio;
  snames[0] = "all";
  scrn->seqradio = RadioCreate();
  DEBUG(("Radio adding buttons...\n"));
  RadioAddButtons(scrn->seqradio, snames, 1, 0);
  
  {
    Arg	al[10];
    int	ac = 0;
    static  XtCallbackRec helpCallback[2] = {(XtCallbackProc)ExecPopupHelpMenu, NULL,NULL};
    
    sprintf(rname, "%smenubar.helpTopic", name);
    helpCallback[0].closure =
      (caddr_t) GetApplicationResourceAsString(rname, "Help Topic");
    XtSetArg(al[ac], XmNentryCallback, "menuBar"); ac++;
    XtSetArg(al[ac], XmNhelpCallback, helpCallback); ac++;
    DEBUG(("Creating menubar...\n"));
    scrn->menu = XmCreateMenuBar(scrn->main,
				 "menuBar",
				 al, ac);
  };
  DEBUG(("Managing Child - menu & widget...\n"));
  XtManageChild(scrn->menu);
  XtManageChild(scrn->widget);
  DEBUG(("Setting areas..\n"));
  XmMainWindowSetAreas(scrn->main, scrn->menu, NULL,NULL,NULL, scrn->widget);
  DEBUG(("Creating  widget from layout...\n"));
  CreateWidgetFromLayout(scrn, scrn->widget, name);
  DEBUG(("Setting areas again..\n"));
  XmMainWindowSetAreas(scrn->main, scrn->menu, NULL,NULL,NULL, scrn->widget);
  DEBUG(("Return from MenuMake.\n"));

}



/*
 * SM:	Help Related Routines       
 */

static Widget helpDialog = NULL;
static Widget main_window = NULL;
static Widget app_shell = NULL;

Widget
GetAppShell (client)
Widget client;
{
	for (app_shell = client;
	     app_shell &&
	       (XtClass(app_shell) != (WidgetClass) topLevelShellWidgetClass); ) {
	    app_shell = GetParent(app_shell);
    }
return app_shell;
}
/* ARGSUSED */
void
ExecPopupHelpMenu(client, closure, call_data)
Widget client;
char *closure;
int call_data;
{
    static int param_count = 1;
    ExecCreateHelpMenu(client, NULL, &closure, &param_count);
}
/* ARGSUSED */
void 
ExecCreateHelpMenu(client, event, params, num_params)
Widget client;
XEvent *event;
char **params;
int  *num_params;
{
XmString topic;

    Arg	help_args[5];
    int ac = 0;
    static XmString help_librarySpec = (XmString) NULL;
    static XmString help_applicationName;

    DEBUG(("Creating help menu widget...\n"));
    TRACE(("@ExecCreateHelpMenu\n"));
    if (num_params == NULL || *num_params <= 0) {
	DEBUG(("No parameters sent to function...\n"));
	return;
    }
    topic    = XmStringCreateSimple(*params);

    if (!help_librarySpec) {
	help_librarySpec = XmStringCreateSimple("/usr/lib/X11/help/dxmail");
	help_applicationName = XmStringCreateSimple("dxmail");
    }
    XtSetArg(help_args[ac], DXmNfirstTopic, topic);ac++;
    XtSetArg(help_args[ac], DXmNlibraryType, DXmTextLibrary);ac++;
    XtSetArg(help_args[ac], DXmNlibrarySpec, help_librarySpec);ac++;
    XtSetArg(help_args[ac], DXmNapplicationName, help_applicationName);ac++;

    BeginLongOperation();

    if ( !helpDialog ) {
	GetAppShell(client);
	helpDialog = DXmCreateHelpDialog(app_shell, "helpDialog",
					help_args, ac);
    }

    XmUpdateDisplay(app_shell);

    if ( helpDialog )
    {
	XtSetValues(helpDialog, help_args, ac);
	XtManageChild(helpDialog);
    }

    EndLongOperation();
}

Widget
GetMainWindow (client)
Widget client;
{

	for (main_window = client;
	     main_window &&
	       (XtClass(main_window) != (WidgetClass) xmMainWindowWidgetClass); ) {
	    main_window = GetParent(main_window);
    }
return main_window;
}


/* ARGSUSED */
void 
ExecOnContext(client, event, params, num_params)
Widget client;
XEvent *event;
char **params;
Cardinal *num_params;
{
    Cursor cursor = XCreateFontCursor(XtDisplay(client), XC_question_arrow);

    TRACE(("@ExecOnContext\n"));
    if ( client = XmTrackingLocate(GetMainWindow(client), cursor, False) )
    {
	for ( ; client != NULL; client = XtParent(client) )
	{
	    if ( XtHasCallbacks(client, XmNhelpCallback) == XtCallbackHasSome )
	    {
		XtCallCallbacks(client, XmNhelpCallback, NULL);
		break;
	    }
	}
    }
}

/* ARGSUSED */
void 
destroyHelpDialog(client, cb)
Widget client;
XmAnyCallbackStruct *cb;

{
#ifdef NODESTROY
	if ( helpDialog ) XtUnmanageChild( helpDialog );
#else
	if ( helpDialog ) XtDestroyWidget( helpDialog );
#endif
	helpDialog = NULL;
}

InitMenu()
{
}
/*
 * Procedure for resizing the button box.
 * Input is the widget ID of the scrolled window; using the size
 * of the scrolled window and the width of the children, the folder
 * RowColumn is resized to accomodate the buttons.
 */
/* ARGSUSED */
Boolean
ResizeFolderBox(widget, event)
    Widget		widget;
    XConfigureEvent	*event;
{
    Scrn		scrn;
    Dimension 		width;
    Arg	    		arglist[10];
    int	    		ac;
    static int		change = 30;
    int			i;
    Widget		child;
    int			childWidth;
    static int		marginWidth = 0;
    static int		spacing = 0;
    static Arg		getArgs[] = {
	{XmNmarginWidth, (XtArgVal) &marginWidth},
	{XmNspacing,	(XtArgVal) &spacing},
    };

/* Find which main window this is in */
    for (i = 0; i < numScrns; i++) {
	scrn = scrnList[i];
	if (XtParent(widget) == scrn->widget)
	    break;
    }
    if (XtParent(widget) != scrn->widget)
	return True;
    if (change == 30)
	change = 29;
    else
	change = 30;
    width = GetWidth(scrn->main) - change;
    ac = 0;
    XtSetArg(arglist[ac], XmNwidth, width);ac++;
/*
 * Get the width of the children so that the new number of
 * columns can be figured out.
 * Include the margin width on the left and right, and
 * the spacing between the buttons.
 */

    child = GetFirstChild(scrn->folderwidget);
    if (child != (Widget) NULL) {
	int columnCount;
	childWidth = GetWidth(child);
	XtGetValues(scrn->folderwidget, getArgs, XtNumber(getArgs));
	width = width - 2 * marginWidth;	/* Left and right margins */
	childWidth = childWidth + 2 * spacing;	/* Left and right edges */
	columnCount = width / childWidth;
	XtSetArg(arglist[ac], XmNnumColumns, columnCount);ac++;
    }
    XtSetValues(scrn->folderwidget, arglist, ac);

    return True;		/* work proc - don't re-execute */
}
