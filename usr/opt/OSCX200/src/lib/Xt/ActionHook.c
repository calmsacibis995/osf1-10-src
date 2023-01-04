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
/* $XConsortium: ActionHook.c,v 1.4 90/12/03 16:30:40 converse Exp $ */

/*LINTLIBRARY*/

/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/* 
 * Contains XtAppAddActionHook, XtRemoveActionHook
 */

#include "IntrinsicI.h"


/*ARGSUSED*/
static void FreeActionHookList( widget, closure, call_data )
    Widget widget;		/* unused (and invalid) */
    XtPointer closure;		/* ActionHook* */
    XtPointer call_data;	/* unused */
{
    ActionHook list = *(ActionHook*)closure;
    while (list != NULL) {
	ActionHook next = list->next;
	XtFree( (XtPointer)list );
	list = next;
    }
}


XtActionHookId XtAppAddActionHook( app, proc, closure )
    XtAppContext app;
    XtActionHookProc proc;
    XtPointer closure;
{
    ActionHook hook = XtNew(ActionHookRec);
    hook->next = app->action_hook_list;
    hook->app = app;
    hook->proc = proc;
    hook->closure = closure;
    if (app->action_hook_list == NULL) {
	_XtAddCallback( &app->destroy_callbacks,
		        FreeActionHookList,
		        (XtPointer)&app->action_hook_list
		      );
    }
    app->action_hook_list = hook;
    return (XtActionHookId)hook;
}


void XtRemoveActionHook( id )
    XtActionHookId id;
{
    ActionHook *p, hook = (ActionHook)id;
    XtAppContext app = hook->app;
    for (p = &app->action_hook_list; p != NULL && *p != hook; p = &(*p)->next);
    if (p == NULL) {
#ifdef DEBUG
	XtAppWarningMsg(app, "badId", "xtRemoveActionHook", XtCXtToolkitError,
			"XtRemoveActionHook called with bad or old hook id",
			(String*)NULL, (Cardinal*)NULL);
#endif /*DEBUG*/	
	return;
    }
    *p = hook->next;
    XtFree( (XtPointer)hook );
}
