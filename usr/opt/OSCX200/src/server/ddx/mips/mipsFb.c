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
 * $XConsortium: mipsFb.c,v 1.4 91/07/18 22:58:16 keith Exp $
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
#ident	"$Header: /usr/sde/x11/rcs/x11/src/./server/ddx/mips/mipsFb.c,v 1.2 91/12/15 12:42:16 devrcs Exp $"

#include <sys/types.h>
#include <sysv/sys/grafreg.h>
#include <sysv/sys/termio.h>
#include <sysv/sys/kbd_ioctl.h>

#include <X.h>
#include <Xproto.h>
#include <misc.h>
#include <colormapst.h>
#include <input.h>
#include <scrnintstr.h>

#include "mips.h"
#include "mipsFb.h"
#include "mipsKbd.h"
#include "mipsIo.h"

extern int defaultColorVisualClass;
extern int mipsMonitorSize;

static void mipsWriteCMapIoctl();
static int mcfbCheckDepth();

/* generic initialization for color FBs */
mipsInitColor(pm)
	MipsScreenPtr pm;
{
	pm->bitsPerPixel = 8;
	pm->depth = mipsCheckDepth(pm->fbnorm);
	/* measured on 19" and 16" Sony monitors */
	pm->dpi = mipsMonitorSize ? 96 : 110;
	pm->scr_width = 1280;
	pm->scr_height = 1024;

	/* need to open keyboard for colormap access */
	openKeybd();

	pm->WriteCMap = mipsWriteCMapIoctl;
}

/*ARGSUSED*/
static void
mipsWriteCMapIoctl(pm, pmap)
	MipsScreenPtr pm;
	ColormapPtr pmap;
{
	int n;
	Entry *in;
	unsigned char *out;
	struct colorm cm;

	n = pmap->pVisual->ColormapEntries;
	in = pmap->red;

	cm.cmstart = 0;
	cm.cmcount = n;
	out = cm.cmap;

	while (--n >= 0) {
		out[0] = in->co.local.red >> 8;
		out[1] = in->co.local.green >> 8;
		out[2] = in->co.local.blue >> 8;
		in++;
		out += 3;
	}

	if (keybdPriv.cap & DEV_COLORMAP)
		if (sysvIoctl(keybdPriv.fd, KTCWRTCOLOR, &cm) < 0) {
			keybdPriv.cap &= ~DEV_COLORMAP;
			Error("KTCWRTCOLOR ioctl failed");
		}
}

/*
 * Check 8 bit frame buffer to see how many planes are populated.
 */
static int
mipsCheckDepth(fb)
	volatile u_char *fb;
{
	int depth = 8;
	u_char fb0, fb1;

	fb0 = fb[0];
	fb1 = fb[1];
	fb[0] = 0x5a;
	fb[1] = 0xa5;

	if (fb[0] != 0x5a || fb[1] != 0xa5)
		depth = 4;

	fb[0] = fb0;
	fb[1] = fb1;

	return depth;
}

/*
 * Special CFB initialization code...
 *
 * To support 4 bit gray scale systems, we overwrite the visuals table
 * in cfb/cfbscrinit.c.  This is not elegant but at least we don't
 * have to modify the MIT code.
 */

#define	PSZ	4
#define _BP 8
#define _RZ ((PSZ + 2) / 3)
#define _RS 0
#define _RM ((1 << _RZ) - 1)
#define _GZ ((PSZ - _RZ + 1) / 2)
#define _GS _RZ
#define _GM (((1 << _GZ) - 1) << _GS)
#define _BZ (PSZ - _RZ - _GZ)
#define _BS (_RZ + _GZ)
#define _BM (((1 << _BZ) - 1) << _BS)
#define _CE (1 << _RZ)

static VisualRec visuals[] = {
/* vid  class        bpRGB cmpE nplan rMask gMask bMask oRed oGreen oBlue */
#ifndef STATIC_COLOR
    0,  PseudoColor, _BP,  1<<PSZ,   PSZ,  0,   0,   0,   0,   0,   0,
    0,  DirectColor, _BP, _CE,       PSZ,  _RM, _GM, _BM, _RS, _GS, _BS,
    0,  GrayScale,   _BP,  1<<PSZ,   PSZ,  0,   0,   0,   0,   0,   0,
    0,  StaticGray,  _BP,  1<<PSZ,   PSZ,  0,   0,   0,   0,   0,   0,
#endif
    0,  StaticColor, _BP,  1<<PSZ,   PSZ,  _RM, _GM, _BM, _RS, _GS, _BS,
    0,  TrueColor,   _BP, _CE,       PSZ,  _RM, _GM, _BM, _RS, _GS, _BS
};

#define	NUMVISUALS	((sizeof visuals)/(sizeof visuals[0]))

mipsFixScreen4(pScreen)
	ScreenPtr pScreen;
{
	VisualPtr oldvis;
	int i;

	if (defaultColorVisualClass < 0)
		defaultColorVisualClass = GrayScale;

	oldvis = pScreen->visuals;

	for (i = 0; i < NUMVISUALS; i++) {
		oldvis[i].bitsPerRGBValue =
			visuals[i].bitsPerRGBValue;
		oldvis[i].ColormapEntries =
			visuals[i].ColormapEntries;
		oldvis[i].nplanes = visuals[i].nplanes;
		oldvis[i].redMask = visuals[i].redMask;
		oldvis[i].greenMask = visuals[i].greenMask;
		oldvis[i].blueMask = visuals[i].blueMask;
		oldvis[i].offsetRed = visuals[i].offsetRed;
		oldvis[i].offsetGreen = visuals[i].offsetGreen;
		oldvis[i].offsetBlue = visuals[i].offsetBlue;
	}
}
