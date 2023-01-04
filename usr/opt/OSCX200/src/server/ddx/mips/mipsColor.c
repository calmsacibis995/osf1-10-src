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
 * $XConsortium: mipsColor.c,v 1.5 91/07/18 22:58:10 keith Exp $
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
#ident	"$Header: /usr/sde/x11/rcs/x11/src/./server/ddx/mips/mipsColor.c,v 1.2 91/12/15 12:42:16 devrcs Exp $"

#include <sys/types.h>

#include "X.h"
#include "Xproto.h"
#include "scrnintstr.h"
#include "colormapst.h"
#include "windowstr.h"
#include "input.h"

#include "mips.h"
#include "mipsFb.h"

int
mipsListInstalledColormaps(pScreen, pmaps)
	ScreenPtr pScreen;
	Colormap *pmaps;
{
	MipsScreenPtr pm = MipsScreenToPriv(pScreen);

	/* By the time we are processing requests, we can guarantee that
	 * there is always a colormap installed */

	*pmaps = pm->InstalledMap->mid;
	return 1;
}

void
mipsStoreColors(pmap, ndef, pdefs)
	ColormapPtr pmap;
	int ndef;
	xColorItem *pdefs;
{
	MipsScreenPtr pm = MipsScreenToPriv(pmap->pScreen);

	if (pmap != pm->InstalledMap)
		return;

	switch (pmap->class) {
	case GrayScale:
	case PseudoColor:
	case StaticColor:
	case StaticGray:
		if (pm->WriteCMap)
			(*pm->WriteCMap)(pm, pmap);
		break;
	case DirectColor:
	default:
		ErrorF("illegal colormap class (%d)\n", pmap->class);
		break;
	}
}

void
mipsInstallColormap(pmap)
	ColormapPtr pmap;
{
	ScreenPtr pScreen = pmap->pScreen;
	int index = pScreen->myNum;
	MipsScreenPtr pm = MipsScreenNumToPriv(index);
	ColormapPtr oldpmap = pm->InstalledMap;

	if (pmap == oldpmap)
		return;

	/* Uninstall pInstalledMap. No hardware changes required, just
	 * notify all interested parties. */

	if (oldpmap != (ColormapPtr) None)
		WalkTree(oldpmap->pScreen, TellLostMap,
			(pointer) &oldpmap->mid);

	/* Install pmap */

	pm->InstalledMap = pmap;
	switch (pmap->class) {
	case StaticGray:
	case GrayScale:
	case PseudoColor:
	case StaticColor:
		(*pScreen->StoreColors)(pmap, 0, NULL);
		break;
	default:
		ErrorF("illegal colormap class (%d)\n", pmap->class);
		break;
	}
	WalkTree(pScreen, TellGainedMap, (pointer) &pmap->mid);
}

void
mipsUninstallColormap(pmap)
	ColormapPtr pmap;
{
	ScreenPtr pScreen = pmap->pScreen;
	MipsScreenPtr pm = MipsScreenToPriv(pScreen);

	if (pmap != pm->InstalledMap)
		return;

	/* Install default map */
	pmap = (ColormapPtr) LookupIDByType(pScreen->defColormap,
		RT_COLORMAP);
	(*pScreen->InstallColormap)(pmap);
}


/*
 * taken from X11R5 ddx/dec/ws/cfbinit.c -- thank you DEC!
 */
static void
colorNameToColor(index, name, red, green, blue)
	int index;
	char *name;
	unsigned short *red, *green, *blue;
{
	/* hex color */
	if (name[0] == '#') {
		int value;
		if (sscanf(&name[1], "%6x", &value) == 1) {
			*blue = value << 8;
			*green = (value >>= 8) << 8;
			*red = (value >>= 8) << 8;
		}
	}
	/* named color */
	else
		(void) OsLookupColor(index, name, strlen(name),
			red, green, blue);
}

/* based on cfb/cfbcmap.c:cfbCreateDefColormap() */
Bool
mipsCreateDefColormap(index, pScreen, white, black)
	int index;
	ScreenPtr pScreen;
	char *white, *black;
{
	VisualPtr pVisual;
	ColormapPtr cmap;
	unsigned short red, green, blue;

	for (pVisual = pScreen->visuals;
		pVisual->vid != pScreen->rootVisual;
		pVisual++)
		 /* nothing */ ;

	if (CreateColormap(pScreen->defColormap, pScreen, pVisual, &cmap,
		(pVisual->nplanes == 1 || (pVisual->class & DynamicClass)) ?
			AllocNone : AllocAll, 0))
		return FALSE;

	red = green = blue = 0xffff;
	colorNameToColor(index, white, &red, &green, &blue);
	if (AllocColor(cmap, &red, &green, &blue, &pScreen->whitePixel, 0))
		return FALSE;

	red = green = blue = 0;
	colorNameToColor(index, black, &red, &green, &blue);
	if (AllocColor(cmap, &red, &green, &blue, &pScreen->blackPixel, 0))
		return FALSE;

	(*pScreen->InstallColormap) (cmap);
	return TRUE;
}
