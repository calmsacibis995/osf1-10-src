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
/* $XConsortium: StrToBS.c,v 1.2 90/12/20 13:27:50 converse Exp $ */

/* 
 * Copyright 1988 by the Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific, 
 * written prior permission. M.I.T. makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 */

#include <X11/Intrinsic.h>
#include "Converters.h"
#include "CharSet.h"

#define	done(address, type) \
	{ (*toVal).size = sizeof(type); (*toVal).addr = (caddr_t) address; }

/* ARGSUSED */
void
XmuCvtStringToBackingStore (args, num_args, fromVal, toVal)
    XrmValue	*args;		/* unused */
    Cardinal	*num_args;	/* unused */
    XrmValuePtr fromVal;
    XrmValuePtr toVal;
{
    char	lowerString[1024];
    XrmQuark	q;
    static int	backingStoreType;
    static XrmQuark XtQEnotUseful, XtQEwhenMapped, XtQEalways, XtQEdefault;
    static int haveQuarks = 0;

    if (*num_args != 0)
        XtWarning("String to BackingStore conversion needs no extra arguments");
    if (!haveQuarks) {
	XmuCopyISOLatin1Lowered (lowerString, XtEnotUseful);
	XtQEnotUseful = XrmStringToQuark(lowerString);
	XmuCopyISOLatin1Lowered (lowerString, XtEwhenMapped);
	XtQEwhenMapped = XrmStringToQuark(lowerString);
	XmuCopyISOLatin1Lowered (lowerString, XtEalways);
	XtQEalways = XrmStringToQuark(lowerString);
	XmuCopyISOLatin1Lowered (lowerString, XtEdefault);
	XtQEdefault = XrmStringToQuark(lowerString);
	haveQuarks = 1;
    }
    XmuCopyISOLatin1Lowered (lowerString, (char *) fromVal->addr);
    q = XrmStringToQuark (lowerString);
    if (q == XtQEnotUseful) {
	backingStoreType = NotUseful;
	done (&backingStoreType, int);
    } else if (q == XtQEwhenMapped) {
    	backingStoreType = WhenMapped;
	done (&backingStoreType, int);
    } else if (q == XtQEalways) {
	backingStoreType = Always;
	done (&backingStoreType, int);
    } else if (q == XtQEdefault) {
    	backingStoreType = Always + WhenMapped + NotUseful;
	done (&backingStoreType, int);
    } else {
        XtStringConversionWarning((char *) fromVal->addr, "BackingStore");
    }
}
