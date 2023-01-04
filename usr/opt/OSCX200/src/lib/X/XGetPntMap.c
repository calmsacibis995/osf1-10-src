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
/* $XConsortium: XGetPntMap.c,v 1.14 91/01/06 11:46:14 rws Exp $ */
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

#ifdef MIN		/* some systems define this in <sys/param.h> */
#undef MIN
#endif
#define MIN(a, b) ((a) < (b) ? (a) : (b))

int XGetPointerMapping (dpy, map, nmaps)
    register Display *dpy;
    unsigned char *map;	/* RETURN */
    int nmaps;

{
    unsigned char mapping[256];	/* known fixed size */
    long nbytes;
    xGetPointerMappingReply rep;
    register xReq *req;

    LockDisplay(dpy);
    GetEmptyReq(GetPointerMapping, req);
    (void) _XReply(dpy, (xReply *)&rep, 0, xFalse);

    nbytes = (long)rep.length << 2;
    _XRead (dpy, (char *)mapping, nbytes);
    /* don't return more data than the user asked for. */
    if (rep.nElts) {
	    bcopy ((char *) mapping, (char *) map, 
		MIN((int)rep.nElts, nmaps) );
	}
    UnlockDisplay(dpy);
    SyncHandle();
    return ((int) rep.nElts);
}

#if NeedFunctionPrototypes
KeySym *XGetKeyboardMapping (Display *dpy,
#if NeedWidePrototypes
			     unsigned int first_keycode,
#else
			     KeyCode first_keycode,
#endif
			     int count,
			     int *keysyms_per_keycode)
#else
KeySym *XGetKeyboardMapping (dpy, first_keycode, count, keysyms_per_keycode)
    register Display *dpy;
    KeyCode first_keycode;
    int count;
    int *keysyms_per_keycode;		/* RETURN */
#endif
{
    long nbytes;
    unsigned long nkeysyms;
    register KeySym *mapping = NULL;
    xGetKeyboardMappingReply rep;
    register xGetKeyboardMappingReq *req;

    LockDisplay(dpy);
    GetReq(GetKeyboardMapping, req);
    req->firstKeyCode = first_keycode;
    req->count = count;
    if (! _XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	UnlockDisplay(dpy);
	SyncHandle();
	return (KeySym *) NULL;
    }

    nkeysyms = (unsigned long) rep.length;
    if (nkeysyms > 0) {
	nbytes = nkeysyms * sizeof (KeySym);
	mapping = (KeySym *) Xmalloc ((unsigned) nbytes);
	nbytes = nkeysyms << 2;
	if (! mapping) {
	    _XEatData(dpy, (unsigned long) nbytes);
	    UnlockDisplay(dpy);
	    SyncHandle();
	    return (KeySym *) NULL;
	}
	_XRead32 (dpy, (char *) mapping, nbytes);
#ifndef CRAY
#ifdef MUSTCOPY
	{
	int i;
	if (sizeof(KeySym) > sizeof(CARD32))
	  for (i = nkeysyms-1; i >= 0; i -= 1)
	    mapping[i] = ((CARD32 *) mapping)[i];
	if (sizeof(KeySym) < sizeof(CARD32))
	  for (i = 0; i < nkeysyms; i += 1)
	    mapping[i] = ((CARD32 *) mapping)[i];
	}
#endif /*MUSTCOPY*/
#endif /*CRAY*/
    }
    *keysyms_per_keycode = rep.keySymsPerKeyCode;
    UnlockDisplay(dpy);
    SyncHandle();
    return (mapping);
}

