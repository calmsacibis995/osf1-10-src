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
/* $XConsortium: XQuTree.c,v 11.19 91/01/06 11:47:41 rws Exp $ */
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

Status XQueryTree (dpy, w, root, parent, children, nchildren)
    register Display *dpy;
    Window w;
    Window *root;	/* RETURN */
    Window *parent;	/* RETURN */
    Window **children;	/* RETURN */
    unsigned int *nchildren;  /* RETURN */
{
    long nbytes;
    xQueryTreeReply rep;
    register xResourceReq *req;

    LockDisplay(dpy);
    GetResReq(QueryTree, w, req);
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	UnlockDisplay(dpy);
	SyncHandle();
	return (0);
	}

    *children = (Window *) NULL; 
    if (rep.nChildren != 0) {
	nbytes = rep.nChildren * sizeof(Window);
	*children = (Window *) Xmalloc((unsigned) nbytes);
	nbytes = rep.nChildren << 2;
	if (! *children) {
	    _XEatData(dpy, (unsigned long) nbytes);
	    UnlockDisplay(dpy);
	    SyncHandle();
	    return (0);
	}
#if ( defined(__osf__) && defined(__alpha) )
	{
	  /* sizeof(Window) != sizeof(CARD32) */
	  int i ;
	  CARD32 value ;

	  for (i = 0 ; i < rep.nChildren ; i++)
	  {
	    _XRead32 (dpy, &value, 4) ;
	    (*children)[i] = (long) value ;
	  }
	}
#else
	_XRead32 (dpy, (char *) *children, nbytes);
       /* Note: won't work if sizeof(Window) is not 32 bits! */
#endif
    }
    *parent = rep.parent;
    *root = rep.root;
    *nchildren = rep.nChildren;
    UnlockDisplay(dpy);
    SyncHandle();
    return (1);
}

