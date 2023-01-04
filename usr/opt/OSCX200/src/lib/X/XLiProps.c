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
/* $XConsortium: XLiProps.c,v 11.20 91/01/06 11:46:49 rws Exp $ */
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

Atom *XListProperties(dpy, window, n_props)
register Display *dpy;
Window window;
int *n_props;  /* RETURN */
{
    long nbytes;
    xListPropertiesReply rep;
    Atom *properties;
    register xResourceReq *req;

    LockDisplay(dpy);
    GetResReq(ListProperties, window, req);
    if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	*n_props = 0;
	UnlockDisplay(dpy);
        SyncHandle();
	return ((Atom *) NULL);
    }

    if (rep.nProperties) {
#if ( defined (__osf__) && defined(__alpha) )
	int ret_bytes, i;
	CARD32 * values; 

	ret_bytes = rep.nProperties * sizeof(Atom);
	properties = (Atom *) Xmalloc ((unsigned) ret_bytes);

	nbytes = rep.nProperties * sizeof(CARD32);
	values = (CARD32 *) _XAllocScratch(dpy, (unsigned long) nbytes);
	nbytes = rep.nProperties << 2;
	if (! properties) {
	    _XEatData(dpy, (unsigned long) nbytes);
	    UnlockDisplay(dpy);
	    SyncHandle();
	    return (Atom *) NULL;
	}
	_XRead32 (dpy, (char *) values, nbytes);
	for ( i = 0; i < rep.nProperties; i++ )
	    properties[i] = (Atom)values[i];
#else
	nbytes = rep.nProperties * sizeof(Atom);
	properties = (Atom *) Xmalloc ((unsigned) nbytes);
	nbytes = rep.nProperties << 2;
	if (! properties) {
	    _XEatData(dpy, (unsigned long) nbytes);
	    UnlockDisplay(dpy);
	    SyncHandle();
	    return (Atom *) NULL;
	}
	_XRead32 (dpy, (char *) properties, nbytes);
#endif
    }
    else properties = (Atom *) NULL;

    *n_props = rep.nProperties;
    UnlockDisplay(dpy);
    SyncHandle();
    return (properties);
}
