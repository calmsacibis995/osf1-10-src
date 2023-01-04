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
/*****************************************************************************
Copyright 1988, 1989 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************************/

#ifndef VMS
#include <X11/Xatom.h>
#else
#include <decw$include/Xatom.h>
#endif

#include "x11perf.h"

static Atom XA_PK_TEMP;
static Window root;

void DoNoOp(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    int     i;

    for (i = 0; i != reps; i++) {
	XNoOp(xp->d);
    }
}


void DoGetAtom(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    char    *atom;
    int     i;

    for (i = 0; i != reps; i++) {
	atom = XGetAtomName (xp->d, 1);
    }
}

int InitGetProperty(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    unsigned long foo = 41;

    root = RootWindow (xp->d, 0);
    XA_PK_TEMP = XInternAtom (xp->d, "_PK_TEMP", False);
    XChangeProperty (
	    xp->d, root, XA_PK_TEMP, XA_INTEGER, 32,
	    PropModeReplace, (unsigned char *)&foo, 1);
    return reps;
}

void DoGetProperty(xp, p, reps)
    XParms  xp;
    Parms   p;
    int     reps;
{
    char   *atom;
    int     i, status;
    int     actual_format;
    unsigned long actual_length, bytes_remaining;
    unsigned char *prop;
    
    Atom actual_type;

    for (i = 0; i != reps; i++) {
	status = XGetWindowProperty (
		xp->d, root, XA_PK_TEMP, 0, sizeof (int),
		False, AnyPropertyType, &actual_type, &actual_format,
		&actual_length, &bytes_remaining, &prop);
    }
}
