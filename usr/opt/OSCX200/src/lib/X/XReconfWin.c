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
/* $XConsortium: XReconfWin.c,v 11.14 91/01/06 11:47:45 rws Exp $ */
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

#include "Xlibint.h"

#define AllMaskBits (CWX|CWY|CWWidth|CWHeight|\
		     CWBorderWidth|CWSibling|CWStackMode)

XConfigureWindow(dpy, w, mask, changes)
    register Display *dpy;
    Window w;
    unsigned int mask;
    XWindowChanges *changes;
    {
    CARD32 values[7];
    register CARD32 *value = values;
    long nvalues;
    register xConfigureWindowReq *req;

    LockDisplay(dpy);
    GetReq(ConfigureWindow, req);
    req->window = w;
    mask &= AllMaskBits;
    req->mask = mask;

    /* Warning!  This code assumes that "unsigned long" is 32-bits wide */

    if (mask & CWX)
	*value++ = changes->x;
	
    if (mask & CWY)
    	*value++ = changes->y;

    if (mask & CWWidth)
    	*value++ = changes->width;

    if (mask & CWHeight)
    	*value++ = changes->height;

    if (mask & CWBorderWidth)
    	*value++ = changes->border_width;

    if (mask & CWSibling)
	*value++ = changes->sibling;

    if (mask & CWStackMode)
        *value++ = changes->stack_mode;

    req->length += (nvalues = value - values);

    nvalues <<= 2;			/* watch out for macros... */
    Data32 (dpy, (long *) values, nvalues);
    UnlockDisplay(dpy);
    SyncHandle();

    }
