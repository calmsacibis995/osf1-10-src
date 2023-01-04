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
static char rcs_id[] = "@(#)$RCSfile: radio.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/08/02 23:59:57 $";
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

/* radio.c -- management of radio boxes. */

#include "decxmail.h"
#include "radio.h"
#include "radioint.h"
#include "toc.h"
#include <Xm/PushB.h>
#include "item.h"
#include "ClosedFolder.bit"
#include "OpenFolder.bit"
#include "OpenTrash.bit"
#include "ClosedTrash.bit"
#include "OpenDrawer.bit"
#include "ClosedDrawer.bit"

Pixmap OpenFolderPixmap = (Pixmap) NULL;
Pixmap ClosedFolderPixmap = (Pixmap) NULL;
Pixmap OpenTrashPixmap = (Pixmap) NULL;
Pixmap ClosedTrashPixmap = (Pixmap) NULL;
Pixmap OpenDrawerPixmap = (Pixmap) NULL;
Pixmap ClosedDrawerPixmap = (Pixmap) NULL;

/*
 * Handle a radio button being pressed.
 */

/* ARGSUSED */
static void DoRadioPress(w, param, data)
Widget w;
Opaque param, data;
{
    Radio radio = (Radio) param;
    Widget widget;
    XEvent event;
    WidgetInfoRec *info;
    register int i;
    RadioSetCurrent(radio, GetWidgetName(w));
    widget = GetParent(w);
    for (i=0 ; i<radio->num_widgets ; i++) {
	if (widget == radio->widgetinfo[i].widget) {
	    info = radio->widgetinfo + i;
	    if (info->func)
		(*info->func)(w, &event, info->params, (Cardinal *)info->num_params);
	}
    }
}
/*
 * Handle double-click on a folder
 */

extern void ExecOpenFolder();
/* ARGSUSED */
static void DoRadioDoubleClick(w, param, data)
Widget w;
Opaque param, data;
{
    Radio radio = (Radio) param;
    Toc toc;
    char *name;
    static char *autoOpened = (char *) NULL;
    char currentParent[512], autoParent[512];
    char *ptr;

    name = GetWidgetName(w);
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
	    TocHideSubfolders(w, autoOpened);
	    XtFree(autoOpened);
	    autoOpened = (char *) NULL;
	}
    }
    RadioSetCurrent(radio, name);
    RadioSetOpened(radio, name);
    ExecOpenFolder(w);
    if (defOpenSubfolders) {
	toc = TocGetNamed(name);
	if (toc != NULL) {
	    if (TocSubFolderCount(toc)) {
		TocRevealSubfolders(w, name);
		if (autoOpened == NULL) autoOpened = XtNewString(name);
	    }
	}

    }
}


/*
 * Sort proc used when inserting children into a radio button.  By a
 * disgusting hack, we always return the right value by having the rest of the
 * code set SortPosition just before this routine would be called.
 */

static Cardinal SortPosition;

/* ARGSUSED */
static Cardinal SortProc(widget)
Widget widget;
{
    return SortPosition;
}


/*
 * Create a new radio box.
 */

Radio RadioCreate()
{
    Radio radio;

    radio = XtNew(RadioRec);
    radio->num_widgets = 0;
    radio->widgetinfo = NULL;
    radio->num_children = 0;
    radio->children = NULL;
    radio->names = NULL;
    radio->current = -1;
    radio->opened= -1;
    radio->highlight = NULL;
    radio->itemBox = False;
    return radio;
}


/* Highlight or unhighlight the given button.  What a hack...%%% */

static FlipColors(widget)
Widget widget;
{
    static Arg arglist[] = {
	{XtNforeground, NULL},
	{XtNbackground, NULL},
    };
    XtArgVal temp;
    arglist[0].value = arglist[1].value = NULL;
    XtGetValues(widget, arglist, XtNumber(arglist));
    temp = arglist[0].value;
    arglist[0].value = arglist[1].value;
    arglist[1].value = temp;
    XtSetValues(widget, arglist, XtNumber(arglist));
    DEBUG(("Flipping %s\n",GetWidgetName(widget)));
}



static void IncrementBorderWidth(widget, delta)
Widget widget;
int delta;
{
    int border_width;
    static Arg args[] = {
	{XtNborderWidth, (XtArgVal) NULL},
    };
    DEBUG(("Highlighting %s\n",GetWidgetName(widget)));
    border_width = GetBorderWidth(widget) + delta;
    if (border_width >= 0) {
	args[0].value = (XtArgVal) border_width;
	XtSetValues(widget, args, XtNumber(args));
    }
}

void InitFolderPixmaps(widget)
Widget widget;
{
    static Arg colorargs[] = {
	{XtNforeground, NULL},
	{XtNbackground, NULL},
    };
    if (OpenFolderPixmap == (Pixmap) NULL) {
	colorargs[0].value = colorargs[1].value = NULL;
	XtGetValues(widget, colorargs, XtNumber(colorargs));

	OpenFolderPixmap = XCreatePixmapFromBitmapData(XtDisplay(widget),
				XtWindow(widget), openfolder_bits,
				openfolder_width, openfolder_height,	
				colorargs[0].value, colorargs[1].value,
				DefaultDepth(XtDisplay(widget),
					DefaultScreen(XtDisplay(widget))));

	ClosedFolderPixmap = XCreatePixmapFromBitmapData(XtDisplay(widget),
				XtWindow(widget), closedfolder_bits,
				closedfolder_width, closedfolder_height,	
				colorargs[0].value, colorargs[1].value,
				DefaultDepth(XtDisplay(widget),
					DefaultScreen(XtDisplay(widget))));
	OpenTrashPixmap = XCreatePixmapFromBitmapData(XtDisplay(widget),
				XtWindow(widget), opentrash_bits,
				opentrash_width, opentrash_height,	
				colorargs[0].value, colorargs[1].value,
				DefaultDepth(XtDisplay(widget),
					DefaultScreen(XtDisplay(widget))));
	ClosedTrashPixmap = XCreatePixmapFromBitmapData(XtDisplay(widget),
				XtWindow(widget), closedtrash_bits,
				closedtrash_width, closedtrash_height,	
				colorargs[0].value, colorargs[1].value,
				DefaultDepth(XtDisplay(widget),
					DefaultScreen(XtDisplay(widget))));
	OpenDrawerPixmap = XCreatePixmapFromBitmapData(XtDisplay(widget),
				XtWindow(widget), opendrawer_bits,
				opendrawer_width, opendrawer_height,	
				colorargs[0].value, colorargs[1].value,
				DefaultDepth(XtDisplay(widget),
					DefaultScreen(XtDisplay(widget))));
	ClosedDrawerPixmap = XCreatePixmapFromBitmapData(XtDisplay(widget),
				XtWindow(widget), closeddrawer_bits,
				closeddrawer_width, closeddrawer_height,	
				colorargs[0].value, colorargs[1].value,
				DefaultDepth(XtDisplay(widget),
					DefaultScreen(XtDisplay(widget))));
    }
}
/*
 * Add a new master widget which is to display this radio box.  Any button
 * selected in this widget will then call the specified func and params.
 */

void RadioAddWidget(radio, widget, func, params, num_params, isFolder)
Radio radio;
Widget widget;
XtActionProc func;
char **params;
Cardinal num_params;
Boolean isFolder;
{
    register int i, ac;
    int which = radio->num_widgets;
    static XtCallbackRec callbackstruct[2] = {
      {DoRadioPress,  NULL},
      {NULL, NULL}
    };
    static XtCallbackRec doubleclickcallback[2] = {
      {DoRadioDoubleClick, NULL},
      {NULL, NULL}
    };

    Arg arglist[5];
    char *nameStr;
    
    
    InitFolderPixmaps(widget);
    ac = 0;
    callbackstruct[0].closure = (XtPointer) radio;
    doubleclickcallback[0].closure = (XtPointer) radio;

    arglist[0].value = (XtArgVal) callbackstruct;
    radio->num_widgets++;
    radio->widgetinfo = (WidgetInfoRec *)
	XtRealloc((char *) radio->widgetinfo,
		  (unsigned) radio->num_widgets * sizeof(WidgetInfoRec));
    radio->widgetinfo[which].widget = widget;
    radio->widgetinfo[which].func = func;
    radio->widgetinfo[which].params = params;
    radio->widgetinfo[which].num_params = num_params;
    radio->itemBox = isFolder;
    SetInsertPosition((CompositeWidget) widget, SortProc);
    radio->children = (Widget **)
	XtRealloc((char *) radio->children,
		  (unsigned) radio->num_widgets * sizeof(Widget *));
    if (radio->num_children)
	radio->children[which] = (Widget *)
	    XtMalloc((unsigned) radio->num_children * sizeof(Widget));
    else
	radio->children[which] = NULL;
    for (i=0 ; i<radio->num_children ; i++) {
	SortPosition = i;
	if (isFolder) {
	    ac = 0;
	    if ((nameStr = index(radio->names[i], '/')) == NULL)
		nameStr = radio->names[i];
	    XtSetArg(arglist[ac], XmNlabelString, 
		     XmStringCreateSimple(nameStr));ac++;

	    XtSetArg(arglist[ac], XmNdefaultActionCallback,
		(XtArgVal) doubleclickcallback);ac++;

	    XtSetArg(arglist[ac], XmNsingleSelectionCallback,
		(XtArgVal)callbackstruct);ac++;

	    if (!strcmp(radio->names[i], wastebasketFolderName)) {
		if (radio->opened == i) {
		    XtSetArg(arglist[ac], XmNlabelPixmap, OpenTrashPixmap);ac++;
		} else {
		    XtSetArg(arglist[ac], XmNlabelPixmap, ClosedTrashPixmap);ac++;
		}
	    } else {
		if (radio->opened == i) {
		    XtSetArg(arglist[ac], XmNlabelPixmap,
			(TocSubFolderCount(TocGetNamed(radio->names[i])) ?
				OpenDrawerPixmap : OpenFolderPixmap));ac++;
		} else {
		    XtSetArg(arglist[ac], XmNlabelPixmap,
			(TocSubFolderCount(TocGetNamed(radio->names[i])) ?
				ClosedDrawerPixmap : ClosedFolderPixmap));ac++;
		}
	    }
/** commented out 'coz itemwidgetclass gives problems now...
    To be enabled once the Toolkit fix is done...
***/
	    radio->children[which][i] = 
		XtCreateWidget(radio->names[i], itemwidgetclass, widget,
				arglist, ac);


	    XmStringFree(arglist[0].value);

	} else {
	    ac = 0;
	    XtSetArg(arglist[ac], XmNlabelString, 
	        (XtArgVal) XmStringCreateSimple(radio->names[i]));ac++;
	    XtSetArg(arglist[ac], XmNactivateCallback, callbackstruct);ac++;
	    radio->children[which][i] = 
	      XtCreateWidget(radio->names[i], xmPushButtonWidgetClass, widget,
			     arglist, ac);
	    XmStringFree(arglist[0].value);
	    if (radio->highlight[i]) {
		IncrementBorderWidth(radio->children[which][i],
			defNewMailBorderWidth);
	    }
	}
    }
    if (radio->current >= 0)
	FlipColors(radio->children[which][radio->current]);
    XtManageChildren(radio->children[which], radio->num_children);
    if (isFolder && radio->num_children)
	XmProcessTraversal(radio->children[which][0], XmTRAVERSE_CURRENT);
}




/* Set the current button in a radio box. */

void RadioSetCurrent(radio, name)
Radio radio;
char *name;
{
    register int i,j, k;
    Arg arglist[2];

    i = -1;
    if (name)
	for (i=0 ; i<radio->num_children ; i++)
	    if (strcmp(radio->names[i], name) == 0) break;
    if (i < 0 || i >= radio->num_children) i = -1;
    if (radio->itemBox) {
	if (radio->current != i) {
	    for (j=0 ; j<radio->num_widgets ; j++) {
		for (k = 0;k < radio->num_children; k++) {
		    if (!strcmp(radio->names[k], wastebasketFolderName)) {
			if (k == radio->opened) {
			    XtSetArg(arglist[0], XmNlabelPixmap, OpenTrashPixmap);
			} else {
			    XtSetArg(arglist[0], XmNlabelPixmap, ClosedTrashPixmap);
			}
			XtSetValues(radio->children[j][k], arglist, 1);
			DXmItemSetState(radio->children[j][k], (k == i), False);		    } else {
			if (k == radio->opened) {
		    	XtSetArg(arglist[0], XmNlabelPixmap, 
			    (TocSubFolderCount(TocGetNamed(radio->names[k])) ?
				OpenDrawerPixmap : OpenFolderPixmap));
			} else {
		    	XtSetArg(arglist[0], XmNlabelPixmap, 
			    (TocSubFolderCount(TocGetNamed(radio->names[k])) ?
				ClosedDrawerPixmap : ClosedFolderPixmap));
			}
			XtSetValues(radio->children[j][k], arglist, 1);
			DXmItemSetState(radio->children[j][k], (k == i), False);
			}
		}
	    }
	    radio->current = i;
	}
    } else {
	if (radio->current != i) {
	    for (j=0 ; j<radio->num_widgets ; j++) {
		if (radio->current >= 0)
		    FlipColors(radio->children[j][radio->current]);
		if (i >= 0)
		    FlipColors(radio->children[j][i]);
	    }
	    radio->current = i;
	}
    }
}

/* Set the opened button in a radio box. */

void RadioSetOpened(radio, name)
Radio radio;
char *name;
{
    register int i,j, k;
    Arg arglist[2];
    Toc toc;

    if (defSVNStyle) {
	toc = TocGetNamed(name);
	if (toc) TocSetOpened(toc, True);
	return;
    }
    i = -1;
    if (name)
	for (i=0 ; i<radio->num_children ; i++)
	    if (strcmp(radio->names[i], name) == 0) break;
    if (i < 0 || i >= radio->num_children) i = -1;
    if (i >= 0) radio->opened = i;
    for (j=0 ; j<radio->num_widgets ; j++) {
	for (k = 0;k < radio->num_children; k++) {
	    if (!strcmp(radio->names[k], wastebasketFolderName)) {
		if (k == radio->opened) {
		    XtSetArg(arglist[0], XmNlabelPixmap, OpenTrashPixmap);
		} else {
		    XtSetArg(arglist[0], XmNlabelPixmap, ClosedTrashPixmap);
		}
		XtSetValues(radio->children[j][k], arglist, 1);
		DXmItemSetState(radio->children[j][k], (k == i), False);
		if (k == i) XmProcessTraversal(radio->children[j][k],
						XmTRAVERSE_CURRENT);
	    } else {
		if (k == radio->opened) {
		    XtSetArg(arglist[0], XmNlabelPixmap, 
			(TocSubFolderCount(TocGetNamed(radio->names[k])) ?
				OpenDrawerPixmap : OpenFolderPixmap));
		} else {
		    XtSetArg(arglist[0], XmNlabelPixmap, 
			(TocSubFolderCount(TocGetNamed(radio->names[k])) ?
				ClosedDrawerPixmap : ClosedFolderPixmap));
		}
		XtSetValues(radio->children[j][k], arglist, 1);
		DXmItemSetState(radio->children[j][k], (k == i), False);
		if (k == i) XmProcessTraversal(radio->children[j][k],
						XmTRAVERSE_CURRENT); 
	    }
	}
    } 
}
/*
 * Fix the pixmaps for the folders. (Folders added or deleted)
 */
void RadioFixFolders(radio)
Radio radio;
{
    register int i,j;
    Arg arglist[2];

    if (!radio || !radio->num_children) return;
    for (i=0 ; i<radio->num_widgets ; i++) {
	for (j = 0;j < radio->num_children; j++) {
	    if (!strcmp(radio->names[j], wastebasketFolderName)) {
		if (j == radio->opened) {
		    XtSetArg(arglist[0], XmNlabelPixmap, OpenTrashPixmap);
		} else {
		    XtSetArg(arglist[0], XmNlabelPixmap, ClosedTrashPixmap);
		}
		XtSetValues(radio->children[i][j], arglist, 1);
	    } else {
		if (j == radio->opened) {
		    XtSetArg(arglist[0], XmNlabelPixmap, 
			(TocSubFolderCount(TocGetNamed(radio->names[j])) ?
				OpenDrawerPixmap : OpenFolderPixmap));
		} else {
		    XtSetArg(arglist[0], XmNlabelPixmap, 
			(TocSubFolderCount(TocGetNamed(radio->names[j])) ?
				ClosedDrawerPixmap : ClosedFolderPixmap));
		}
		XtSetValues(radio->children[i][j], arglist, 1);
	    }
	}
    } 
}

/*
 * Create a bunch of new buttons, and add them to a radio box.
 */

void RadioAddButtons(radio, names, num_names, position)
Radio radio;
char **names;
int num_names;
int position;
{
    static XtCallbackRec callbackstruct[2] = {DoRadioPress, NULL, NULL};
    static XtCallbackRec doubleclickcallback[2] = {DoRadioDoubleClick, NULL, NULL};
    static Arg arglist[] = {
	{XmNactivateCallback, NULL},
    };
    register int i, w;
    int oldcur = radio->current;

    if (num_names == 0) return;
    RadioSetCurrent(radio, (char *) NULL);
    if (position < 0) position = 0;
    if (position > radio->num_children) position = radio->num_children;

    callbackstruct[0].closure = (caddr_t) radio;
    arglist[0].value = (XtArgVal) callbackstruct;
    doubleclickcallback[0].closure = (caddr_t) radio;

/* Resize the radio arrays containing the button names etc... */
    radio->num_children += num_names;
    radio->highlight = (Boolean *)
	XtRealloc((char *) radio->highlight,
		  radio->num_children * sizeof(Boolean));
    radio->names = (char **)
	XtRealloc((char *) radio->names,
		  (unsigned) radio->num_children * sizeof(char *));

/* shift all button names down that exist  below the insertion point... */
    for (i=radio->num_children - 1 ; i >= position + num_names ; i--)
	radio->names[i] = radio->names[i - num_names];

/* copy and unhighlight the new button names at the insertion point... */
    for (i=0 ; i<num_names ; i++) {
	radio->names[i + position] = MallocACopy(names[i]);
	radio->highlight[i + position] = FALSE;
    }

/* For every parent of this radio box... */
    for (w=0 ; w<radio->num_widgets ; w++) {
	radio->children[w] = (Widget *)
	    XtRealloc((char *) radio->children[w],
		      (unsigned) radio->num_children * sizeof(Widget));
	for (i=radio->num_children - 1 ; i >= position + num_names ; i--)
	    radio->children[w][i] = radio->children[w][i - num_names];
	for (i=position ; i<position + num_names ; i++) {
	    SortPosition = i;
	    if (radio->itemBox) {
		Punt("Should not have called RadioAddButtons");
	    } else {
		radio->children[w][i] =
		    XmCreatePushButton(radio->widgetinfo[w].widget,
				       radio->names[i],
				       arglist, XtNumber(arglist));
	    }
	}
	XtManageChildren(radio->children[w] + position, (Cardinal) num_names);
	XtAddWorkProc((XtWorkProc)ResizeFolderBox, XtParent(radio->widgetinfo[w].widget));
    }
    if (oldcur >= 0) {
/* PJS: Only increment the old current position if it was after the position
 * at which the new buttons were installed.
 */
	if (oldcur >= position)
	    oldcur += num_names;
	RadioSetCurrent(radio, radio->names[oldcur]);
    }
}
void RadioAddFolders(radio, names, num_names, position)
Radio radio;
char **names;
int num_names;
int position;
{
    static XtCallbackRec callbackstruct[2] = {DoRadioPress, NULL, NULL};
    static XtCallbackRec doubleclickcallback[2] = {DoRadioDoubleClick, NULL, NULL};
    static Arg arglist[] = {
	{XmNactivateCallback, NULL},
    };
    register int i, w, ac;
    Arg itemArglist[5];
    int oldcur;
    char *nameStr;

    oldcur = radio->current;
    RadioSetCurrent(radio, (char *) NULL);
    if (position < 0) position = 0;
    if (position > radio->num_children) position = radio->num_children;

    callbackstruct[0].closure = (caddr_t) radio;
    arglist[0].value = (XtArgVal) callbackstruct;
    doubleclickcallback[0].closure = (caddr_t) radio;

/* Resize the radio arrays containing the button names etc... */
    radio->num_children += num_names;
    radio->highlight = (Boolean *)
	XtRealloc((char *) radio->highlight,
		  radio->num_children * sizeof(Boolean));
    radio->names = (char **)
	XtRealloc((char *) radio->names,
		  (unsigned) radio->num_children * sizeof(char *));

/* shift all button names down that exist  below the insertion point... */
    for (i=radio->num_children - 1 ; i >= position + num_names ; i--) {
	radio->names[i] = radio->names[i - num_names];
    }

/* copy and unhighlight the new button names at the insertion point... */
    for (i=0 ; i<num_names ; i++) {
	radio->names[i + position] = MallocACopy(names[i]);
	radio->highlight[i + position] = FALSE;
    }

/* For every parent of this radio box... */
    for (w=0 ; w<radio->num_widgets ; w++) {
	radio->children[w] = (Widget *)
	    XtRealloc((char *) radio->children[w],
		      (unsigned) radio->num_children * sizeof(Widget));
	for (i=radio->num_children - 1 ; i >= position + num_names ; i--)
	    radio->children[w][i] = radio->children[w][i - num_names];
	for (i=position ; i<position + num_names ; i++) {
	    SortPosition = i;
	    if (!defSVNStyle && radio->itemBox) {
		ac = 0;
		if ((nameStr = index(radio->names[i],'/')) == NULL) {
		    nameStr = radio->names[i];
		}
		XtSetArg(itemArglist[ac], XmNlabelString, 
	        	(XtArgVal) XmStringCreateSimple(nameStr));ac++;
		XtSetArg(itemArglist[ac], XmNdefaultActionCallback,
			(XtArgVal) doubleclickcallback);ac++;
		XtSetArg(itemArglist[ac], XmNsingleSelectionCallback,
			(XtArgVal)callbackstruct);ac++;
		if (!strcmp(radio->names[i], wastebasketFolderName)) {
		    if (radio->opened == i) {
		    	XtSetArg(itemArglist[ac], XmNlabelPixmap,
				OpenTrashPixmap);ac++;
		    } else {
			XtSetArg(itemArglist[ac], XmNlabelPixmap,
				ClosedTrashPixmap);ac++;
		    }
		} else {
		    if (radio->opened == i) {
			if (TocSubFolderCount(TocGetNamed(radio->names[i]))) {
			    XtSetArg(itemArglist[ac], XmNlabelPixmap,
				OpenDrawerPixmap);ac++;
			} else {
			    XtSetArg(itemArglist[ac], XmNlabelPixmap,
				OpenFolderPixmap);ac++;
			}
		    } else {
			if (TocSubFolderCount(TocGetNamed(radio->names[i]))) {
			    XtSetArg(itemArglist[ac], XmNlabelPixmap,
				ClosedDrawerPixmap);ac++;
			} else {
			    XtSetArg(itemArglist[ac], XmNlabelPixmap,
				ClosedFolderPixmap);ac++;
			}
		    }
		}
		radio->children[w][i] = 
		    XtCreateWidget(radio->names[i], itemwidgetclass,
				radio->widgetinfo[w].widget,
				itemArglist, ac); 
		XmStringFree(itemArglist[0].value);
	    } else {
		if (!defSVNStyle)
		    radio->children[w][i] =
		        XmCreatePushButton(radio->widgetinfo[w].widget,
				       radio->names[i],
				       arglist, XtNumber(arglist));
	    }
	}
	if (!defSVNStyle)
	    XtManageChildren(radio->children[w] + position, (Cardinal) num_names);
    }
    if (oldcur >= 0) {
/* PJS: Only increment the old current position if it was after the position
 * at which the new buttons were installed.
 */
	if (oldcur >= position)
	    oldcur += num_names;
	RadioSetCurrent(radio, radio->names[oldcur]);
    }
}


/*
 * Remove the given button widget from the radio box.  The button widget is
 * destroyed.  If it was the current button in a radio buttonbox, then the
 * current button becomes unset.
 */

void RadioDeleteButton(radio, name)
Radio radio;
char *name;
{
    register int i, w;
    if (radio->current >= 0 && strcmp(name, radio->names[radio->current]) == 0)
	RadioSetCurrent(radio, (char *) NULL);
    for (i=0 ; i<radio->num_children ; i++) {
	if (strcmp(radio->names[i], name) == 0) {
	    XtFree(radio->names[i]);
	    radio->num_children--;
	    for (w=0 ; w<radio->num_widgets ; w++) {
		/*
		 * The next line is an incredibly disgusting hack.  The
		 * intrinsics have a bug where it tries to free the border
		 * pixmap when it destroys a widget.  This is completely wrong,
		 * and the next line defeats it by brute force. %%%
		 */
		SetBorderPixmap(radio->children[w][i], (Pixmap) 2);
#ifndef NODESTROY
		XtDestroyWidget(radio->children[w][i]);
#else
		XtUnmanageChild(radio->children[w][i]);
#endif
	    }
	    for ( ; i<radio->num_children ; i++) {
		radio->names[i] = radio->names[i+1];
		for (w=0 ; w<radio->num_widgets ; w++) {
		    radio->children[w][i] = radio->children[w][i+1];
		}
	    }
	    break;
	}
    }
}



char *RadioGetCurrent(radio)
Radio radio;
{
    return (radio->current >= 0) ?radio->names[radio->current] : NULL;
}


Cardinal RadioGetNumChildren(radio)
Radio radio;
{
    return radio->num_children;
}


char *RadioGetName(radio, i)
Radio radio;
int i;
{
    return radio->names[i];
}


extern void RadioSetHighlight(radio, name, value)
Radio radio;
char *name;
Boolean value;
{
    register int i, w, delta;
    for (i=0 ; i<radio->num_children ; i++) {
	if (strcmp(radio->names[i], name) == 0) {
		radio->highlight[i] = value;
		delta = value ? defNewMailBorderWidth : -defNewMailBorderWidth;
		for (w=0 ; w<radio->num_widgets ; w++) {
		    IncrementBorderWidth(radio->children[w][i], delta);
		}
	}
    }
}
