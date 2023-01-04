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
static char rcs_id[] = "@(#)$RCSfile: widgetinfo.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/08/03 00:03:34 $";
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

/*
 * Routines to get info from widgets.  Most of this should probably be
 * provided for me by the toolkit...
 */


#include <stdio.h>
#include <Xm/XmP.h>
#include <X11/IntrinsicP.h>
#include <Xm/ScrollBarP.h>
#include <Xm/LabelGP.h>
#include <Xm/LabelP.h>
#include <Xm/CascadeBGP.h>
#include <Xm/CascadeBP.h>

int
GetShadowThickness(widget)
Widget widget;
{
int		thickness = 0;
static Arg	args[] = {
		    {XmNshadowThickness, (XtArgVal)NULL},
		};

    args[0].value = (XtArgVal)&thickness;
    XtGetValues(widget, args, XtNumber(args));
    return thickness;
}

char *GetWidgetName(widget)
Widget widget;
{
    return widget->core.name;
}


unsigned int GetBorderWidth(widget)
Widget widget;
{
    return widget->core.border_width;
}


Widget GetParent(widget)
Widget widget;
{
    return widget->core.parent;
}


unsigned int GetHeight(widget)
Widget widget;
{
    return widget->core.height;
}


unsigned int GetWidth(widget)
Widget widget;
{
    return widget->core.width;
}


int GetX(widget)
Widget widget;
{
    return widget->core.x;
}


int GetY(widget)
Widget widget;
{
    return widget->core.y;
}


void SetInsertPosition(widget, func)
CompositeWidget widget;
XtOrderProc func;
{
    static Arg arg[1] = {
	{XmNinsertPosition, (XtArgVal) 0},
    };
    arg[0].value = (XtArgVal) func;
    XtSetValues((Widget) widget, arg, 1);
}


/*
 * The below routines shouldn't be used unless you really know what you're
 * doing...
 */

void SetX(widget, x)
Widget widget;
int x;
{
    widget->core.x = x;
}

void SetY(widget, y)
Widget widget;
int y;
{
    widget->core.y = y;
}

void SetBorderPixmap(widget, border_pixmap)
Widget widget;
Pixmap border_pixmap;
{
    widget->core.border_pixmap = border_pixmap;
}

void SetFocus(widget, focusTime)
Widget widget;
Time focusTime;
{
    if (widget->core.widget_class->core_class.accept_focus)
	(*widget->core.widget_class->core_class.accept_focus)(widget, &focusTime);
}

Boolean IsScrollbarWidget(widget)
Widget widget;
{
    return (XtClass(widget) == ((WidgetClass)xmScrollBarWidgetClass)
	    || XtClass(XtParent(widget)) == ((WidgetClass)xmScrollBarWidgetClass));
}


/*
 * Dump the hierarchy of widget names and classes from the given widget,
 * indented by the given amount.
 */

void DumpIt(fid, widget, indent)
FILE *fid;
Widget widget;
int indent;
{
    register int i;
    char *name;
    for (i=0 ; i<indent ; i++)
	(void) fprintf(fid, " ");
    name = XtName(widget);
    (void) fprintf(fid, "%s (%x)", name, widget);
    for (i=strlen(name) + indent ; i<40 ; i++)
	(void) fprintf(fid, " ");
    (void) fprintf(fid, "%s", XtClass(widget)->core_class.class_name);
    (void) fprintf(fid, " (%d,%d)",GetX(widget),GetY(widget));
    (void) fprintf(fid, " %dx%d\n",GetWidth(widget),GetHeight(widget));


    /* Test to see whether widget or gadget CascadeButton */

    if (strcmp(XtClass(widget)->core_class.class_name, "XmCascadeButton") == 0)
      {
	if (XmIsGadget (widget))
	  {
	    widget = (Widget) (((XmCascadeButtonGadget) widget)->cascade_button.submenu);
	  }
	else
	  {
	    widget = (Widget) (((XmCascadeButtonWidget) widget)->cascade_button.submenu);
	  }
      }

    if (XtIsSubclass(widget, compositeWidgetClass)) {
	CompositeWidget c = (CompositeWidget) widget;
	for (i=0 ; i<c->composite.num_children ; i++)
	    DumpIt(fid, c->composite.children[i], indent + 2);
    }
}
/*
 * Return the first child of a composite widget (or NULL if not composite).
 * Used during folder-box resizing to find the size of the buttons.
 */

Widget GetFirstChild(widget)
Widget widget;
{
    if (XtIsSubclass(widget, compositeWidgetClass)) {
	CompositeWidget c = (CompositeWidget) widget;
	if (c->composite.num_children > 0) {
	    return c->composite.children[0];
	} else {
	    return (Widget) NULL;
	}
    } else {
	return (Widget) NULL;
    }
}
