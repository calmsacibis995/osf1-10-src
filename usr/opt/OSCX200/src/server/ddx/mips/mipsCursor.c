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
 * $XConsortium: mipsCursor.c,v 1.2 91/07/18 22:58:13 keith Exp $
 *
 * Copyright 1991 MIPS Computer Systems, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of MIPS not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  MIPS makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * MIPS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL MIPS
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#ident	"$Header: /usr/sde/x11/rcs/x11/src/./server/ddx/mips/mipsCursor.c,v 1.2 91/12/15 12:42:16 devrcs Exp $"

/*
 * Device independent (?) part of HW cursor support
 */

#ifndef X11R4

#include <signal.h>

#include <X.h>
#define NEED_EVENTS
#include <misc.h>
#include <input.h>
#include <cursorstr.h>
#include <mipointer.h>
#include <regionstr.h>
#include <scrnintstr.h>
#include <servermd.h>
#include <windowstr.h>

#include "mipsFb.h"

static Bool mipsRealizeCursor();
static Bool mipsUnrealizeCursor();
static void mipsSetCursor();
static void mipsMoveCursor();
static void mipsRecolorCursor();

static miPointerSpriteFuncRec mipsPointerSpriteFuncs = {
	mipsRealizeCursor,
	mipsUnrealizeCursor,
	mipsSetCursor,
	mipsMoveCursor,
};

extern miPointerScreenFuncRec mipsPointerScreenFuncs;

Bool
mipsCursorInit(pm, pScr)
	MipsScreenPtr pm;
	ScreenPtr pScr;
{
	if ((pm->cap & MIPS_SCR_CURSOR) && pm->RealizeCursor) {
		if (!(miPointerInitialize(pScr, &mipsPointerSpriteFuncs,
			&mipsPointerScreenFuncs, FALSE)))
			return FALSE;

		pScr->RecolorCursor = mipsRecolorCursor;
	}
	else
		return miDCInitialize(pScr, &mipsPointerScreenFuncs);
}

static Bool
mipsRealizeCursor(pScr, pCurs)
	ScreenPtr pScr;
	CursorPtr pCurs;
{
	if (pCurs->bits->refcnt <= 1) {
		int index = pScr->myNum;
		MipsScreenPtr pm = MipsScreenNumToPriv(index);
		pointer *pPriv = &pCurs->bits->devPriv[index];

		return (*pm->RealizeCursor)(pm, pCurs, pPriv);
	}

	return TRUE;
}

static Bool
mipsUnrealizeCursor(pScr, pCurs)
	ScreenPtr pScr;
	CursorPtr pCurs;
{
	pointer priv;

	if (pCurs->bits->refcnt <= 1 &&
		(priv = pCurs->bits->devPriv[pScr->myNum]))
		xfree(priv);
	return TRUE;
}

static void
mipsSetCursor(pScr, pCurs, x, y)
	ScreenPtr pScr;
	CursorPtr pCurs;
	int x, y;
{
	int index = pScr->myNum;
	MipsScreenPtr pm = MipsScreenNumToPriv(index);
	pointer priv = 	pCurs ? pCurs->bits->devPriv[index] : 0;

#ifdef SYSV
	void (*poll) ();

	poll = sigset(SIGPOLL, SIG_HOLD);
#else /* SYSV */
	int block;

	block = sigblock(sigmask(SIGIO));
#endif /* SYSV */

	(*pm->SetCursor)(pm, pCurs, priv, x, y);

#ifdef SYSV
	(void) sigset(SIGPOLL, poll);
#else /* SYSV */
	(void) sigsetmask(block);
#endif /* SYSV */
}

static void
mipsMoveCursor(pScr, x, y)
	ScreenPtr pScr;
	int x, y;
{
	MipsScreenPtr pm = MipsScreenToPriv(pScr);

	(*pm->MoveCursor)(pm, x, y);
}

static void
mipsRecolorCursor(pScr, pCurs, displayed)
	ScreenPtr pScr;
	CursorPtr pCurs;
	Bool displayed;
{
	MipsScreenPtr pm = MipsScreenToPriv(pScr);

	if (displayed)
		(*pm->RecolorCursor)(pm, pCurs);
}

#endif /* not X11R4 */
