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
static char rcs_id[] = "@(#)$RCSfile: button.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/08/02 23:55:37 $";
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


/* button.c -- Handle a button being pressed */

#include "decxmail.h"
#include "button.h"
#include "buttonint.h"
#include <Xm/PushB.h>
#include <Xm/PushBG.h>

static Button LastButtonPressed = NULL;
static void (*RedoProc)() = NULL;
static caddr_t RedoParam;

static void DoButtonPress();


/*
 * Create a button.
 */

Button ButtonCreate(scrn, parent, name)
Scrn scrn;
Widget parent;
char *name;
{
    static XtCallbackRec callbackstruct[2] = {DoButtonPress, NULL, NULL};
    static Arg arglist[] = {
	{XmNactivateCallback, (XtArgVal) callbackstruct},
    };
    Button button;
    button = (Button) XtNew(ButtonRec);
    button->scrn = scrn;
    button->name = MallocACopy(name);
    button->enabled = TRUE;
    button->num_funcs = 0;
    button->funcs = NULL;
    callbackstruct[0].closure = (caddr_t) button;
    button->widget = XmCreatePushButton(parent, name,
					       arglist, XtNumber(arglist));
    return button;
}



/*
 * Take the given widget, and treat it as a button.  This includes registering
 * the activate callback on it.
 */

Button ButtonMake(widget)
Widget widget;
{
    static XtCallbackRec callbackstruct[2] = {DoButtonPress, NULL, NULL};
    static Arg arglist[] = {
	{XmNactivateCallback, (XtArgVal) callbackstruct},
    };
    Button button;
    button = (Button) XtNew(ButtonRec);
    button->widget = widget;
    button->scrn = ScrnFromWidget(widget);
    button->name = MallocACopy(GetWidgetName(widget));
    button->enabled = TRUE;
    button->num_funcs = 0;
    button->funcs = NULL;
    callbackstruct[0].closure = (caddr_t) button;
    XtSetValues(widget, arglist, XtNumber(arglist));
    return button;
}


/*
 * Add a new function for this button to call when invoked.
 */

void ButtonAddFunc(button, func, params, num_params)
Button button;
XtActionProc func;
char **params;
Cardinal num_params;
{
    FuncInfo info;
    button->num_funcs++;
    button->funcs = (FuncInfo *)
	XtRealloc((char *) button->funcs,
		  (unsigned) button->num_funcs * sizeof(FuncInfo));
    button->funcs[button->num_funcs - 1] = info = XtNew(FuncInfoRec);
    info->func = func;
    info->params = params;
    info->num_params = num_params;
    TieButtonToFunc(button, func);
}
	

/*
 * Change whether the given button is enabled.
 */
void ButtonChangeEnabled(button, enabled)
Button button;
Boolean enabled;
{
    if (enabled != button->enabled) {
	XtSetSensitive(button->widget, enabled);
	button->enabled = enabled;
    }
}

Widget ButtonGetWidget(button)
Button button;
{
    return button->widget;
}


Scrn ButtonGetScrn(button)
Button button;
{
    return button->scrn;
}


/* The given button has just been pressed.  (This routine is usually called
   by the command button widget code.)   Call the functions registered for
   this button. */

/*ARGSUSED*/
static void DoButtonPress(w, param, data)
Widget w;
Opaque param, data;
{
    Button button = (Button) param;
    XEvent event;
    register int i;
    FuncInfo info;
    if (!button->enabled) return;
    if (button != LastButtonPressed) DestroyConfirmWindow();
    LastButtonPressed = button;
    DisableEnablingOfButtons();
    for (i=0 ; i<button->num_funcs ; i++) {
	info = button->funcs[i];
	(*info->func)(button->widget, &event, info->params, &info->num_params);
    }
    EnableEnablingOfButtons();
}


/* Act as if the last button pressed was pressed again. */

void RedoLastButton()
{
    if (LastButtonPressed)
	DoButtonPress(LastButtonPressed->widget, (Opaque) LastButtonPressed,
		      (Opaque) NULL);
    else if (RedoProc)
	(*RedoProc)(RedoParam);
}


/*
 * Call the given procedure when RedoLastButton() gets called.  This is for
 * modules that define they're own callbacks without making a Button structure.
 */

void ButtonSetRedo(proc, param)
void (*proc)();
caddr_t param;
{
    LastButtonPressed = NULL;
    RedoProc = proc;
    RedoParam = param;
}
