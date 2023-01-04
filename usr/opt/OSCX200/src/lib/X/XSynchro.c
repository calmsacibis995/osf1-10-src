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
/* $XConsortium: XSynchro.c,v 11.9 91/01/24 11:10:12 rws Exp $ */
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


int _XSyncFunction(dpy)
register Display *dpy;
{
	XSync(dpy,0);
	return 0;
}

int (*XSynchronize(dpy,onoff))()
     register Display *dpy;
     int onoff;
{
        int (*temp)();

	LockDisplay(dpy);
	temp = dpy->synchandler;
	if (onoff) dpy->synchandler = _XSyncFunction;
	else dpy->synchandler = NULL;
	UnlockDisplay(dpy);
	return (temp);
}

int (*XSetAfterFunction(dpy,func))()
     register Display *dpy;
     int (*func)(
#if NeedNestedPrototypes
		 Display*
#endif
		 );
{
        int (*temp)();

	LockDisplay(dpy);
	temp = dpy->synchandler;
	dpy->synchandler = func;
	UnlockDisplay(dpy);
	return (temp);
}

