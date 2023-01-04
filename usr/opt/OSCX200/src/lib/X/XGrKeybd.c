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
/* $XConsortium: XGrKeybd.c,v 11.16 91/01/06 11:46:25 rws Exp $ */
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

#define NEED_REPLIES
#include "Xlibint.h"
int XGrabKeyboard (dpy, window, ownerEvents, pointerMode, keyboardMode, time)
    register Display *dpy;
    Window window;
    Bool ownerEvents;
    int pointerMode, keyboardMode;
    Time time;
{
        xGrabKeyboardReply rep;
	register xGrabKeyboardReq *req;
	register int status;
	LockDisplay(dpy);
        GetReq(GrabKeyboard, req);
	req->grabWindow = window;
	req->ownerEvents = ownerEvents;
	req->pointerMode = pointerMode;
	req->keyboardMode = keyboardMode;
	req->time = time;

       /* if we ever return, suppress the error */
	if (_XReply (dpy, (xReply *) &rep, 0, xTrue) == 0) 
		rep.status = GrabSuccess;
	status = rep.status;
	UnlockDisplay(dpy);
	SyncHandle();
	return (status);
}

