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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: vgapix.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/08/02 20:39:12 $";
#endif

#include "X.h"
#include "Xproto.h"
#include "Xmd.h"
#include "servermd.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "vga.h"
#include "vgaprocs.h"
#ifndef __alpha
#include "extender.h"
#endif
#define LARGEST_NONHUGE_PIXMAP 0x7FFF

PixmapPtr vgaCreatePixmap (pScreen, width, height, depth)
	ScreenPtr   pScreen;
	int         width;
	int         height;
	int         depth;
	{
	register PixmapPtr pPixmap;
	int byte_width;
	long size;

	byte_width = PixmapBytePad(width, depth);

	size = sizeof (PixmapRec) + (byte_width * (unsigned long)height);

#ifdef DWDOS286
	if (size > LARGEST_NONHUGE_PIXMAP)
		pPixmap = (PixmapPtr)E_huge_allocate(size);
	else
#endif
		pPixmap = (PixmapPtr) Xalloc(size);

	if (!pPixmap)
		return NullPixmap;
	pPixmap->drawable.type = DRAWABLE_PIXMAP;
	pPixmap->drawable.class = 0;
	pPixmap->drawable.pScreen = pScreen;
	pPixmap->drawable.depth = (unsigned char) depth;
	pPixmap->drawable.bitsPerPixel = (unsigned char)((depth == 1) ? 1 : 8);
	pPixmap->drawable.id = 0;
	pPixmap->drawable.serialNumber = NEXT_SERIAL_NUMBER;
	pPixmap->drawable.x = 0;
	pPixmap->drawable.y = 0;
	pPixmap->drawable.width = width;
	pPixmap->drawable.height = height;
	pPixmap->devKind = byte_width;
	pPixmap->refcnt = 1;
	pPixmap->devPrivate.ptr = (pointer)(pPixmap + 1);
	return pPixmap;
	}

Bool vgaDestroyPixmap(pPixmap)
	PixmapPtr pPixmap;
	{
	long size;
	if(--pPixmap->refcnt)
		return TRUE;

#ifdef DWDOS286
	size = sizeof (PixmapRec) + 
	    (pPixmap->devKind * (unsigned long)(pPixmap->drawable.height));
	if (size > LARGEST_NONHUGE_PIXMAP)
		E_huge_free((PMaddress)pPixmap);
	else
#endif
		Xfree((char *)pPixmap);

	return TRUE;
	}
