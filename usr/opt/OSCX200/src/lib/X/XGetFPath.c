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
/* $XConsortium: XGetFPath.c,v 11.15 91/01/06 11:45:55 rws Exp $ */
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

char **XGetFontPath(dpy, npaths)
register Display *dpy;
int *npaths;	/* RETURN */
{
	xGetFontPathReply rep;
	register long nbytes;
	char **flist;
	char *ch;
	register unsigned i;
	register int length;
	register xReq *req;

	LockDisplay(dpy);
	GetEmptyReq (GetFontPath, req);
	(void) _XReply (dpy, (xReply *) &rep, 0, xFalse);

	if (rep.nPaths) {
	    flist = (char **)
		Xmalloc((unsigned) rep.nPaths * sizeof (char *));
	    nbytes = (long)rep.length << 2;
	    ch = (char *) Xmalloc ((unsigned) (nbytes + 1));
                /* +1 to leave room for last null-terminator */

	    if ((! flist) || (! ch)) {
		if (flist) Xfree((char *) flist);
		if (ch) Xfree(ch);
		_XEatData(dpy, (unsigned long) nbytes);
		UnlockDisplay(dpy);
		SyncHandle();
		return (char **) NULL;
	    }

	    _XReadPad (dpy, ch, nbytes);
	    /*
	     * unpack into null terminated strings.
	     */
	    length = *ch;
	    for (i = 0; i < rep.nPaths; i++) {
		flist[i] = ch+1;  /* skip over length */
		ch += length + 1; /* find next length ... */
		length = *ch;
		*ch = '\0'; /* and replace with null-termination */
	    }
	}
	else flist = NULL;
	*npaths = rep.nPaths;
	UnlockDisplay(dpy);
	SyncHandle();
	return (flist);
}

XFreeFontPath (list)
char **list;
{
	if (list != NULL) {
		Xfree (list[0]-1);
		Xfree ((char *)list);
	}
}
