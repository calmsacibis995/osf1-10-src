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
static char rcs_id[] = "@(#)$RCSfile: actions.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/08/02 23:55:26 $";
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

/* actions.c -- management of actions. */

#include "decxmail.h"
#include "button.h"

typedef struct _ActionRec {
    XtActionProc func;
    char *name;
    XrmQuark signature;
    Cardinal num_buttons;
    Button *button;
    Boolean enabled;		/* Whether this function is enabled. */
} ActionRec, *Action;

static XContext ActionContext;
static Action *actionList;
Cardinal num_actions;

InitActions(actions, num)
XtActionList actions;
Cardinal num;
{
    Action action;
    register int i;
    num_actions = num;
    actionList = (Action *) XtMalloc((unsigned) num * sizeof(Action));

    for (i=0 ; i<num ; i++, actions++) {
	actionList[i] = action = XtNew(ActionRec);
	action->name = actions->string;
	action->func = actions->proc;
	action->signature = XrmStringToQuark(action->name);
	action->num_buttons = 0;
	action->button = NULL;
	action->enabled = TRUE;
    }
}


/* Inform the action handler that a new screen has been created. */

ActionNewScrn(scrn)
Scrn scrn;
{
    register int i;
    Action action;

    scrn->actionContext = XUniqueContext();
    for (i=0 ; i<num_actions ; i++) {
	action = XtNew(ActionRec);
	*action = *actionList[i];
	XSaveContext(XtDisplay(scrn->widget), (Window) action->func, 				scrn->actionContext, (caddr_t) action);
    }
}


static Action FuncToAction(scrn, func)
Scrn scrn;
XtActionProc func;
{
    Action result;
    if (XFindContext(XtDisplay(scrn->widget), (Window)func, scrn->actionContext,
		     (caddr_t *)&result))
	    Punt("Couldn't find action!");
    return result;
}



/*
 * Associate the given button with the given function.  Enabling or disabling
 * the function will enable or disable the button.
 *
 */

void TieButtonToFunc(button, func)
Button button;
XtActionProc func;
{
    Action action;
    action = FuncToAction(ButtonGetScrn(button), func);
    action->num_buttons++;
    action->button = (Button *) XtRealloc((char *) action->button,
					  (unsigned)
					  action->num_buttons*sizeof(Button));
    action->button[action->num_buttons - 1] = button;
    ButtonChangeEnabled(button, action->enabled);
}


/*
 * Change whether all buttons associated with the given action in the given
 * scrn are enabled.
 */

void FuncChangeEnabled(func, scrn, enabled)
XtActionProc func;
Scrn scrn;
Boolean enabled;
{
    Action action;
    register int i;
    Button button;

    action = FuncToAction(scrn, func);
    if (enabled != action->enabled) {
	action->enabled = enabled;
	for (i=0 ; i<action->num_buttons ; i++) {
	    button = action->button[i];
	    ButtonChangeEnabled(button, enabled);
	}
    }
}

Boolean FuncGetEnabled(func, scrn)
XtActionProc func;
Scrn scrn;
{
    return FuncToAction(scrn, func)->enabled;
}


XtActionProc NameToFunc(name)
char *name;
{
    char str[500], *ptr;
    Cardinal i;
    XrmQuark signature = XrmStringToQuark(name);
    for (i=0 ; i<num_actions ; i++)
	if (actionList[i]->signature == signature)
	    return actionList[i]->func;
    if (debug) {
	(void) strcpy(str, XrmQuarkToString(signature));
	ptr = str;
	i = 1;
	XtWarningMsg("cantFindAction", "action", "decxmailError",
		     "Couldn't find action named %s.", &ptr, &i);
    }
    return ((XtActionProc)NoOp);
}
