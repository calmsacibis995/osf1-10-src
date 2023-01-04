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
 * $XConsortium: mips3230.c,v 1.3 91/07/18 22:58:04 keith Exp $
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
#ident	"$Header: /usr/sde/x11/rcs/x11/src/./server/ddx/mips/mips3230.c,v 1.2 91/12/15 12:42:16 devrcs Exp $"

#include <sys/types.h>
#include <sysv/sys/grafreg.h>
#include <sysv/sys/termio.h>
#include <sysv/sys/kbd_ioctl.h>

#include <X.h>
#include <Xproto.h>
#include <misc.h>
#include <cursorstr.h>
#include <colormapst.h>
#include <input.h>
#include <scrnintstr.h>
#include <servermd.h>

#include "mipsFb.h"
#include "mips3230.h"
#include "mipsKbd.h"
#include "mipsIo.h"

extern char *mipsMapit();

static void Blank3230c();
static void WriteCMap3230c();
static void Close3230c();

static Bool RealizeCursor3230c();
static void SetCursor3230c();
static void MoveCursor3230c();
static void RecolorCursor3230c();

static void Blank3230m();

/*
 * Color frame buffer support
 */

Bool
mipsMap3230c(pm)
	MipsScreenPtr pm;
{
	if (!pm->fbreg && (
		!(pm->fbcache = (unsigned char *) mipsMapit((char *) 0,
			GBFCKEY, R3030_GRAPHICS_FRAME_SIZE)) ||
		!(pm->fbnocache = (unsigned char *) mipsMapit((char *) 0,
			GBUFKEY, R3030_GRAPHICS_FRAME_SIZE)) ||
		!(pm->fbspec = (unsigned char *) mipsMapit((char *) 0,
			GBFVKEY, R3030_GRAPHICS_VECTOR_FRAME_SIZE)) ||
		!(pm->fbreg = (unsigned char *) mipsMapit((char *) 0,
			GREGKEY, R3030_GRAPHICS_REG_SIZE))))
		return FALSE;

	pm->fbnorm = pm->fbcache;

	Close3230c(pm);
	mipsInitColor(pm);

	/*
	 * Disable monochrome video to avoid performance degradation
	 * when running color-only on two headed system.
	 *
	 * (Depends on monochrome video being enabled later for real
	 * two headed operation, keyboard being open by this point, and
	 * Blank3230m() not needing the pm struct contents.)
	 */
	Blank3230m((MipsScreenPtr) 0, SCREEN_SAVER_ON);

	pm->fb_width = RS3230C_BPSL;

	pm->cap = MIPS_SCR_CURSOR | MIPS_SCR_PACKED | MIPS_SCR_MASK;

	/* fill mode doesn't work on type 0 color boards */
	if (((struct rs3230c_reg *) pm->fbreg)->kernel &
                RS3230C_KERNEL_IDMASK)
		pm->cap |= MIPS_SCR_FILL;

	pm->Blank = Blank3230c;
	pm->WriteCMap = WriteCMap3230c;
	pm->Close = Close3230c;

	pm->RealizeCursor = RealizeCursor3230c;
	pm->SetCursor = SetCursor3230c;
	pm->MoveCursor = MoveCursor3230c;
	pm->RecolorCursor = RecolorCursor3230c;

	{
		struct bt459 *ramdac =
			&((struct rs3230c_reg *) pm->fbreg)->ramdac;
		int v;

		/*
		 * Set read mask for number of planes populated.
		 * XXX The PROM and/or kernel should take care of this.
		 * XXX All 3230s have 8 bit frame buffers anyway.
		 */
		BT459_SETADDR(ramdac, BT459_READMASK);
		ramdac->ctrl = (1 << pm->depth) - 1;

		/*
		 * Set HW cursor to "X mode"
		 */
		BT459_SETADDR(ramdac, BT459_CR2);
		v = ramdac->ctrl;
		ramdac->adlo = BT459_CR2;
		ramdac->ctrl = v | BT459_CR2_XCURS;
	}

	return TRUE;
}

static void
Blank3230c(pm, on)
	MipsScreenPtr pm;
	Bool on;
{
	struct rs3230c_reg *reg = (struct rs3230c_reg *) pm->fbreg;

	if (on != SCREEN_SAVER_ON)
		reg->xserver |= RS3230C_XSERVER_UNBLANK;
	else
		reg->xserver &= ~RS3230C_XSERVER_UNBLANK;
}

/*
 * KTCWRTCOLOR doesn't work on a 3230.  It doesn't even give an error,
 * just prints "WARNING: No IRQ5 Interrupt Available" on the console.
 */
static void
WriteCMap3230c(pm, pmap)
	MipsScreenPtr pm;
	ColormapPtr pmap;
{
	struct bt459 *ramdac =
		&((struct rs3230c_reg *) pm->fbreg)->ramdac;
	int n;
	Entry *in;
	volatile DACBITS *out = &ramdac->cmap;

	n = pmap->pVisual->ColormapEntries;
	in = pmap->red;

	BT459_SETADDR(ramdac, BT459_CMAPRAM);

	while (--n >= 0) {
		*out = in->co.local.red >> 8;
		*out = in->co.local.green >> 8;
		*out = in->co.local.blue >> 8;
		in++;
	}
}

static void
Close3230c(pm)
	MipsScreenPtr pm;
{
	struct rs3230c_reg *reg = (struct rs3230c_reg *) pm->fbreg;

	/* clear mode bits in X server register */
	reg->xserver &= RS3230C_XSERVER_UNBLANK;

	/* disable plane mask */
	reg->mask = ~0;

	/* disable HW cursor */
	SetCursor3230c(pm, (CursorPtr) 0, (pointer) 0, 0, 0);
}

/*
 * Convert cursor source and mask planes into fixed size Bt459 format
 * buffer, so we can reload the cursor RAM more quickly.
 *
 * The Bt459 interleaves mask and source bits in each byte, with the leftmost
 * pixel as the MSB:  <M0 S0 M1 S1 M2 S2 M3 S3>
 */
static Bool
RealizeCursor3230c(pm, pCurs, pPriv)
	MipsScreenPtr pm;
	CursorPtr pCurs;
	pointer *pPriv;
{
	CursorBitsPtr bits = pCurs->bits;
	int w, h;
	int x, y;
	int bytes;
	int soffset;
	short *ram, rmask;
	unsigned char *source, *mask;
	int ramt, st, mt;
	int bit;

	ram = (short *) xalloc(BT459_CURSBYTES);
	*pPriv = (pointer) ram;
	if (!ram)
		return FALSE;

	h = bits->height;
	if (h > BT459_CURSMAX)
		h = BT459_CURSMAX;

	w = bits->width;

	/* line to line offset in source and mask bitmaps */
	soffset = ((w + BITMAP_SCANLINE_PAD - 1) &
		~(BITMAP_SCANLINE_PAD - 1)) >> 3;

	if (w > BT459_CURSMAX)
		w = BT459_CURSMAX;

	/* right edge mask for cursor RAM */
	rmask = 0xffff0000 >> ((w & 7) << 1);

	/* bytes per line actually used in source and mask bitmaps */
	bytes = (w + 7) >> 3;

	source = bits->source;
	mask = bits->mask;

	for (y = 0; y < h; y++) {
		for (x = 0; x < bytes; x++) {
			/*
			 * Repack 1 mask byte and 1 source byte into
			 * 2 Bt459 cursor RAM bytes.
			 */
			mt = mask[x] << 8;
			st = source[x] << 7;
			ramt = 0;
			bit = 0x8000;
			while (bit) {
				ramt |= (mt & bit);
				bit >>= 1;
				mt >>= 1;
				ramt |= (st & bit);
				bit >>= 1;
				st >>= 1;
			}
			*ram++ = ramt;
		}

		/*
		 * Mask off garbage bits of partial word on right edge of
		 * cursor (if any).
		 */
		if (rmask)
			ram[-1] &= rmask;

		/* zero out blank space to right of cursor */
		for (; x < BT459_CURSMAX / 8; x++)
			*ram++ = 0;

		source += soffset;
		mask += soffset;
	}
	/* zero out blank space below cursor */
	for (; y < BT459_CURSMAX; y++) {
		for (x = 0; x < BT459_CURSMAX / 8; x++)
			ram[x] = 0;
		ram += BT459_CURSMAX / 8;
	}

	return TRUE;
}

static void
SetCursor3230c(pm, pCurs, priv, x, y)
	MipsScreenPtr pm;
	CursorPtr pCurs;
	pointer priv;
	int x, y;
{
	struct bt459 *ramdac =
		&((struct rs3230c_reg *) pm->fbreg)->ramdac;
	unsigned char *ram, c;
	int i;

	/* turn cursor off */
	BT459_SETADDR(ramdac, BT459_CURSCMD);
	ramdac->ctrl = 0;

	if (!pCurs)
		return;

	/* adjust hot spot values using magic constants from databook */
	pm->xhot = BT459_CURSFIXX(pCurs->bits->xhot);
	pm->yhot = BT459_CURSFIXY(pCurs->bits->yhot);

	/* position cursor */
	MoveCursor3230c(pm, x, y);

	/* load colormap */
	RecolorCursor3230c(pm, pCurs);

	ram = (unsigned char *) priv;

	/*
	 * Load cursor RAM from preformatted buffer
	 */
	BT459_SETADDR(ramdac, BT459_CURSRAM);
	for (i = 0; i < BT459_CURSBYTES; i++)
		ramdac->ctrl = ram[i];

	/*
	 * Verify cursor RAM contents and correct any errors caused by
	 * internal bus contention in Bt459 Rev A parts...
	 */
	BT459_SETADDR(ramdac, BT459_CURSRAM);
	for (i = 0; i < BT459_CURSBYTES; i++)
		while ((c = (unsigned char) ramdac->ctrl) != ram[i]) {
			BT459_SETADDR(ramdac, BT459_CURSRAM + i);
			ramdac->ctrl = (DACBITS) ram[i];
			BT459_SETADDR(ramdac, BT459_CURSRAM + i);
		}

	/* turn cursor on */
	BT459_SETADDR(ramdac, BT459_CURSCMD);
	ramdac->ctrl = BT459_CURSCMD_CURSEN;
}

static void
MoveCursor3230c(pm, x, y)
	MipsScreenPtr pm;
	int x, y;
{
	struct bt459 *ramdac =
		&((struct rs3230c_reg *) pm->fbreg)->ramdac;

	x += pm->xhot;
	y += pm->yhot;

	BT459_SETADDR(ramdac, BT459_CURSXLO);
	ramdac->ctrl = x;
	ramdac->ctrl = x >> 8;
	ramdac->ctrl = y;
	ramdac->ctrl = y >> 8;
}

static void
RecolorCursor3230c(pm, pCurs)
	MipsScreenPtr pm;
	CursorPtr pCurs;
{
	struct bt459 *ramdac =
		&((struct rs3230c_reg *) pm->fbreg)->ramdac;

	BT459_SETADDR(ramdac, BT459_CURSCOL2);
	ramdac->ctrl = pCurs->backRed >> 8;
	ramdac->ctrl = pCurs->backGreen >> 8;
	ramdac->ctrl = pCurs->backBlue >> 8;
	ramdac->ctrl = pCurs->foreRed >> 8;
	ramdac->ctrl = pCurs->foreGreen >> 8;
	ramdac->ctrl = pCurs->foreBlue >> 8;
}


/*
 * Monochrome frame buffer support
 */

Bool
mipsMap3230m(pm)
	MipsScreenPtr pm;
{
	/* XXX should skip uncached mapping if unused */
	if (!pm->fbcache && (
		!(pm->fbcache = (unsigned char *) mipsMapit((char *) 0,
		GBMONCH, MONO_FRAME_SIZE)) ||
		!(pm->fbnocache = (unsigned char *) mipsMapit((char *) 0,
		GBMNUNC, MONO_FRAME_SIZE))))
		return FALSE;

	pm->fbnorm = pm->fbcache;

	pm->bitsPerPixel = 1;
	pm->depth = 1;
	pm->dpi = 100;	/* measured */
	pm->scr_width = RS3230M_VISW;
	pm->scr_height = RS3230M_VISH;
	pm->fb_width = RS3230M_BPSL * 8;
	pm->Blank = Blank3230m;

	/* need to open keyboard for blanking control */
	openKeybd();

	return TRUE;
}

static void
Blank3230m(pm, on)
	MipsScreenPtr pm;
	Bool on;
{
	int value;

	value = on == SCREEN_SAVER_ON ? 1 : 0;

	if (keybdPriv.cap & DEV_BLANK)
		if (sysvIoctl(keybdPriv.fd, KTMBLANK, &value) < 0) {
			keybdPriv.cap &= ~DEV_BLANK;
			Error("KTMBLANK ioctl failed");
		}
}

#define	RS3230_LINESIZE		32
#define	RS3230_CACHESIZE	RS3230C_VISW
#define	RS3230_ALIGN		(64 * 1024)

/* flush frame buffer data from cache */
/* XXX method for computing flush address is hokey */
mipsFlush3230()
{
	static volatile char *flush;
	extern char *sbrk();
	volatile char *p;
	int i;

	if ((p = flush) == 0)
		flush = p = (volatile char *)
			(((int) sbrk(0) & ~(RS3230_ALIGN - 1)) - RS3230_ALIGN);

	for (i = 0; i < RS3230_CACHESIZE; i += RS3230_LINESIZE)
		p[i];
}
