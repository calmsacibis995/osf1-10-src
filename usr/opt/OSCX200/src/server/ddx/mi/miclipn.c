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
/*

Copyright 1990 by the Massachusetts Institute of Technology

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

/* $XConsortium: miclipn.c,v 5.0 90/09/24 09:41:16 rws Exp $ */

#include "X.h"
#include "windowstr.h"
#include "scrnintstr.h"

static void	(*clipNotify)() = 0;
static void	(*ClipNotifies[MAXSCREENS])();

static void
miClipNotifyWrapper(pWin, dx, dy)
    WindowPtr pWin;
    int dx, dy;
{
    if (clipNotify)
	(*clipNotify)(pWin, dx, dy);
    if (ClipNotifies[pWin->drawable.pScreen->myNum])
	(*ClipNotifies[pWin->drawable.pScreen->myNum])(pWin, dx, dy);
}

/*
 * miClipNotify --
 *	Hook to let DDX request notification when the clipList of
 *	a window is recomputed on any screen.  For R4 compatibility;
 *	better if you wrap the ClipNotify screen proc yourself.
 */

static unsigned long clipGeneration = 0;

void
miClipNotify (func)
    void (*func)();
{
    int i;

    clipNotify = func;
    if (clipGeneration != serverGeneration)
    {
	clipGeneration = serverGeneration;
	for (i = 0; i < screenInfo.numScreens; i++)
	{
	    ClipNotifies[i] = screenInfo.screens[i]->ClipNotify;
	    screenInfo.screens[i]->ClipNotify = miClipNotifyWrapper;
	}
    }
}
