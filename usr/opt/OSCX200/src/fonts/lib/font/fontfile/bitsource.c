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
/* $NCDId: @(#)bitsource.c,v 1.4 1991/07/02 17:00:59 lemke Exp $ */
/*
 * $XConsortium: bitsource.c,v 1.5 91/07/17 14:29:54 keith Exp $
 *
 * Copyright 1991 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

#include    "fontfilest.h"

BitmapSourcesRec	FontFileBitmapSources;

Bool
FontFileRegisterBitmapSource (fpe)
    FontPathElementPtr	fpe;
{
    FontPathElementPtr	*new;
    int			i;
    int			newsize;

    for (i = 0; i < FontFileBitmapSources.count; i++)
	if (FontFileBitmapSources.fpe[i] == fpe)
	    return TRUE;
    if (FontFileBitmapSources.count == FontFileBitmapSources.size)
    {
	newsize = FontFileBitmapSources.size + 4;
	new = (FontPathElementPtr *) xrealloc (FontFileBitmapSources.fpe, newsize * sizeof *new);
	if (!new)
	    return FALSE;
	FontFileBitmapSources.size = newsize;
	FontFileBitmapSources.fpe = new;
    }
    FontFileBitmapSources.fpe[FontFileBitmapSources.count++] = fpe;
    return TRUE;
}

void
FontFileUnregisterBitmapSource (fpe)
    FontPathElementPtr	fpe;
{
    int	    i;

    for (i = 0; i < FontFileBitmapSources.count; i++)
	if (FontFileBitmapSources.fpe[i] == fpe)
	{
	    FontFileBitmapSources.count--;
	    if (FontFileBitmapSources.count == 0)
	    {
		FontFileBitmapSources.size = 0;
		xfree (FontFileBitmapSources.fpe);
		FontFileBitmapSources.fpe = 0;
	    }
	    else
	    {
	    	for (; i < FontFileBitmapSources.count; i++)
		    FontFileBitmapSources.fpe[i] = FontFileBitmapSources.fpe[i+1];
	    }
	    break;
	}
}

FontFileMatchBitmapSource (fpe, pFont, flags, entry, zeroPat, vals, format, fmask, noSpecificSize)
    FontPathElementPtr	fpe;
    FontPtr		*pFont;
    int			flags;
    FontEntryPtr	entry;
    FontNamePtr		zeroPat;
    FontScalablePtr	vals;
    fsBitmapFormat	format;
    fsBitmapFormatMask	fmask;
    Bool		noSpecificSize;
{
    int			source;
    FontEntryPtr	zero;
    FontBitmapEntryPtr	bitmap;
    int			ret;
    FontDirectoryPtr	dir;
    FontScaledPtr	scaled;

    /*
     * Look through all the registered bitmap sources for
     * the same zero name as ours; entries along that one
     * can be scaled as desired.
     */
    ret = BadFontName;
    for (source = 0; source < FontFileBitmapSources.count; source++)
    {
    	if (FontFileBitmapSources.fpe[source] == fpe)
	    continue;
	dir = (FontDirectoryPtr) FontFileBitmapSources.fpe[source]->private;
	zero = FontFileFindNameInDir (&dir->scalable, zeroPat);
	if (!zero)
	    continue;
    	scaled = FontFileFindScaledInstance (zero, vals, noSpecificSize);
    	if (scaled)
    	{
	    if (scaled->pFont)
	    {
		*pFont = scaled->pFont;
		ret = Successful;
	    }
	    else if (scaled->bitmap)
	    {
		entry = scaled->bitmap;
		bitmap = &entry->u.bitmap;
		if (bitmap->pFont)
		{
		    *pFont = bitmap->pFont;
		    ret = Successful;
		}
		else
		{
		    ret = FontFileOpenBitmap (
				FontFileBitmapSources.fpe[source],
				pFont, flags, entry, format, fmask);
		}
	    }
	    else /* "cannot" happen */
	    {
		ret = BadFontName;
	    }
	    break;
    	}
    }
    return ret;
}
