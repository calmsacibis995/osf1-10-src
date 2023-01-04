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
 * $XConsortium: mips2030.c,v 1.2 91/07/18 22:57:54 keith Exp $
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
#ident	"$Header: /usr/sde/x11/rcs/x11/src/./server/ddx/mips/mips2030.c,v 1.2 91/12/15 12:42:16 devrcs Exp $"

#include <sys/types.h>
#include <sysv/sys/grafreg.h>

#include <X.h>
#include <Xproto.h>
#include <misc.h>
#include <colormapst.h>
#include <input.h>
#include <scrnintstr.h>

#include "mips.h"
#include "mipsFb.h"
#include "mips2030.h"

extern char *mipsMapit();

static void Blank2030();

Bool
mipsMap2030(pm)
	MipsScreenPtr pm;
{
	if (!pm->fbreg && (
		!(pm->fbnocache = (unsigned char *) mipsMapit((char *) 0,
			GBUFKEY, GRAPHICS_FRAME_SIZE)) ||
		!(pm->fbreg = (unsigned char *) mipsMapit((char *) 0,
			GREGKEY, GRAPHICS_REG_SIZE))))
		return FALSE;

	pm->fbnorm = pm->fbnocache;

	mipsInitColor(pm);

	pm->fb_width = RS2030_BPSL;
	pm->Blank = Blank2030;

	{
		struct bt458 *ramdac =
			&((struct rs2030_reg *) pm->fbreg)->ramdac;

		/*
		 * Set read mask for number of planes populated.
		 * XXX The PROM and/or kernel should take care of this.
		 */
		ramdac->addr = BT458_READMASK;
		ramdac->ctrl = (1 << pm->depth) - 1;
	}

	return TRUE;
}

static void
Blank2030(pm, on)
	MipsScreenPtr pm;
	Bool on;
{
	struct rs2030_reg *reg = (struct rs2030_reg *) pm->fbreg;

	if (on != SCREEN_SAVER_ON)
		reg->unblank = 0;
	else
		reg->blank = 0;
}
