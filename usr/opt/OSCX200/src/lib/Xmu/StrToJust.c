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
/* $XConsortium: StrToJust.c,v 1.6 90/12/20 13:30:18 converse Exp $ */

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

/* ARGSUSED */
void
XmuCvtStringToJustify(args, num_args, fromVal, toVal)
    XrmValuePtr args;		/* unused */
    Cardinal	*num_args;	/* unused */
    XrmValuePtr fromVal;
    XrmValuePtr toVal;
{
    static XtJustify	e;
    static XrmQuark	XrmQEleft;
    static XrmQuark	XrmQEcenter;
    static XrmQuark	XrmQEright;
    static int		haveQuarks;
    XrmQuark    q;
    char	*s = (char *) fromVal->addr;
    char        lowerName[1000];

    if (s == NULL) return;

    if (!haveQuarks) {
	XrmQEleft   = XrmPermStringToQuark(XtEleft);
	XrmQEcenter = XrmPermStringToQuark(XtEcenter);
	XrmQEright  = XrmPermStringToQuark(XtEright);
	haveQuarks = 1;
    }

    XmuCopyISOLatin1Lowered(lowerName, s);

    q = XrmStringToQuark(lowerName);

    toVal->size = sizeof(XtJustify);
    toVal->addr = (caddr_t) &e;

    if (q == XrmQEleft)   { e = XtJustifyLeft;   return; }
    if (q == XrmQEcenter) { e = XtJustifyCenter; return; }
    if (q == XrmQEright)  { e = XtJustifyRight;  return; }

    toVal->size = 0;
    toVal->addr = NULL;
}
