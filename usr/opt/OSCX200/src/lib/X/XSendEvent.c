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
/* $XConsortium: XSendEvent.c,v 11.11 91/01/06 11:47:51 rws Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

/*
Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in advertising or
publicity pertaining to distribution of the software without specific,
written prior permission.  M.I.T. makes no representations about the
suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.
*/

#define NEED_EVENTS
#include "Xlibint.h"

extern Status _XEventToWire();
/*
 * In order to avoid all images requiring _XEventToWire, we install the
 * event converter here if it has never been installed.
 */
Status
XSendEvent(dpy, w, propagate, event_mask, event)
    register Display *dpy;
    Window w;
    Bool propagate;
    long event_mask;
    XEvent *event;
{
    register xSendEventReq *req;
    xEvent ev;
    register Status (**fp)();
    Status status;

    LockDisplay (dpy);

    /* call through display to find proper conversion routine */

    fp = &dpy->wire_vec[event->type & 0177];
    if (*fp == NULL) *fp = _XEventToWire;
    status = (**fp)(dpy, event, &ev);

    if (status) {
	GetReq(SendEvent, req);
	req->destination = w;
	req->propagate = propagate;
	req->eventMask = event_mask;
#ifdef WORD64
	/* avoid quad-alignment problems */
	bcopy ((char *) &ev, (char *) req->eventdata, SIZEOF(xEvent));
#else
	req->event = ev;
#endif /* WORD64 */
    }

    UnlockDisplay(dpy);
    SyncHandle();
    return(status);
}
