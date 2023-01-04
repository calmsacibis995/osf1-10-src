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
 * $XConsortium: fontfile.c,v 1.10 92/11/20 15:30:50 gildea Exp $
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
/* $NCDId: @(#)fontfile.c,v 1.6 1991/07/02 17:00:46 lemke Exp $ */

#include    "fontfilest.h"

/*
 * Map FPE functions to renderer functions
 */

int
FontFileNameCheck (name)
    char    *name;
{
#ifndef NCD
    return *name == '/';
#else
    return ((strcmp(name, "built-ins") == 0) || (*name == '/'));
#endif
}

int
FontFileInitFPE (fpe)
    FontPathElementPtr	fpe;
{
    int			status;
    FontDirectoryPtr	dir;

    status = FontFileReadDirectory (fpe->name, &dir);
    if (status == Successful)
    {
	if (dir->nonScalable.used > 0)
	    if (!FontFileRegisterBitmapSource (fpe))
	    {
		FontFileFreeFPE (fpe);
		return AllocError;
	    }
	fpe->private = (pointer) dir;
    }
    return status;
}

/* ARGSUSED */
int
FontFileResetFPE (fpe)
    FontPathElementPtr	fpe;
{
    FontDirectoryPtr	dir;

    dir = (FontDirectoryPtr) fpe->private;
    if (FontFileDirectoryChanged (dir))
    {
	/* can't do it, so tell the caller to close and re-open */
	return FPEResetFailed;	
    }
    return Successful;
}

int
FontFileFreeFPE (fpe)
    FontPathElementPtr	fpe;
{
    FontFileUnregisterBitmapSource (fpe);
    FontFileFreeDir ((FontDirectoryPtr) fpe->private);
    return Successful;
}

/* ARGSUSED */
int
FontFileOpenFont (client, fpe, flags, name, namelen, format, fmask,
		  id, pFont, aliasName)
    pointer		client;
    FontPathElementPtr	fpe;
    int			flags;
    char		*name;
    int			namelen;
    fsBitmapFormat	format;
    fsBitmapFormatMask	fmask;
    XID			id;
    FontPtr		*pFont;
    char		**aliasName;
{
    FontDirectoryPtr	dir;
    char		lowerName[MAXFONTNAMELEN];
    char		fileName[MAXFONTFILENAMELEN*2 + 1];
    FontNameRec		tmpName;
    FontEntryPtr	entry;
    FontScalableRec	vals;
    FontScalableEntryPtr   scalable;
    FontScaledPtr	scaled;
    FontBitmapEntryPtr	bitmap;
    FontAliasEntryPtr	alias;
    FontBCEntryPtr	bc;
    int			ret;
    Bool		noSpecificSize;
    
    if (namelen >= MAXFONTNAMELEN)
	return AllocError;
    dir = (FontDirectoryPtr) fpe->private;
    CopyISOLatin1Lowered (lowerName, name, namelen);
    lowerName[namelen] = '\0';
    tmpName.name = lowerName;
    tmpName.length = namelen;
    tmpName.ndashes = FontFileCountDashes (lowerName, namelen);

    /* Look for any bitmap fonts that match pattern (either XLFD or non-XLFD)
	first, before looking at any scalable fonts
    */
    if (entry = FontFileFindNameInDir (&dir->nonScalable, &tmpName))
    {
	switch (entry->type) {
	case FONT_ENTRY_BITMAP:
	    bitmap = &entry->u.bitmap;
	    if (bitmap->pFont)
	    {
	    	*pFont = bitmap->pFont;
	    	ret = Successful;
	    }
	    else
	    {
		ret = FontFileOpenBitmap (fpe, pFont, flags, entry, format, fmask);
	    }
	    break;
	case FONT_ENTRY_ALIAS:
	    alias = &entry->u.alias;
	    *aliasName = alias->resolved;
	    ret = FontNameAlias;
	    break;
	case FONT_ENTRY_BC:
	    bc = &entry->u.bc;
	    entry = bc->entry;
	    ret = (*scalable->renderer->OpenScalable)
		    (fpe, pFont, flags, entry, &bc->vals, format, fmask);
	    break;
	default:
	    ret = BadFontName;
	}
	if (ret == Successful)
	    (*pFont)->info.cachable = TRUE;
	return ret;
    }

    /* No bitmap font in this FPE matches so now look for any matching
	scalable fonts.
    */
    if (tmpName.ndashes == 14 &&
	FontParseXLFDName (lowerName, &vals, FONT_XLFD_REPLACE_ZERO))
    {
	/* pattern is XLFD, so do special XLFD processing: */
	tmpName.length = strlen (lowerName);
	entry = FontFileFindNameInDir (&dir->scalable, &tmpName);
	noSpecificSize = vals.point <= 0 && vals.pixel <= 0;
    	if (entry && entry->type == FONT_ENTRY_SCALABLE &&
	    FontFileCompleteXLFD (&vals, &entry->u.scalable.extra->defaults))
	{
	    scalable = &entry->u.scalable;
	    scaled = FontFileFindScaledInstance (entry, &vals, noSpecificSize);
	    /*
	     * A scaled instance can occur one of two ways:
	     *
	     *  Either the font has been scaled to this
	     *   size already, in which case scaled->pFont
	     *   will point at that font.
	     *
	     *  Or a bitmap instance in this size exists,
	     *   which is handled as if we got a pattern
	     *   matching the bitmap font name.
	     */
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
			ret = FontFileOpenBitmap (fpe, pFont, flags, entry,
						  format, fmask);
		    }
		}
		else /* "cannot" happen */
		{
		    ret = BadFontName;
		}
	    }
	    else
	    {
		ret = FontFileMatchBitmapSource (fpe, pFont, flags, entry, &tmpName, &vals, format, fmask, noSpecificSize);
		if (ret != Successful)
		{
		    /* Make a new scaled instance */
	    	    strcpy (fileName, dir->directory);
	    	    strcat (fileName, scalable->fileName);
	    	    ret = (*scalable->renderer->OpenScalable) (fpe, pFont,
				flags, entry, fileName, &vals, format, fmask);
		    /* Save the instance */
		    if (ret == Successful)
		    	if (!FontFileAddScaledInstance (entry, &vals,
						    *pFont, (char *) 0))
			    (*pFont)->fpePrivate = (pointer) 0;
		}
	    }
	} else {
	    ret = BadFontName;
	}
    }
    else
    {
	/* non-XLFD pattern: the R5 sample code does not try and match any
	    scalable fontnames in this case so we won't either.
	    Note, however, ListFonts will match scalable fontnames for the
	    non-XLFD pattern case.
	*/
	ret = BadFontName;
    }
    if (ret == Successful)
	(*pFont)->info.cachable = TRUE;
    return ret;
}

/* ARGSUSED */
FontFileCloseFont (fpe, pFont)
    FontPathElementPtr	fpe;
    FontPtr		pFont;
{
    FontEntryPtr    entry;

    if (entry = (FontEntryPtr) pFont->fpePrivate) {
	switch (entry->type) {
	case FONT_ENTRY_SCALABLE:
	    FontFileRemoveScaledInstance (entry, pFont);
	    break;
	case FONT_ENTRY_BITMAP:
	    entry->u.bitmap.pFont = 0;
	    break;
	default:
	    /* "cannot" happen */
	    break;
	}
	pFont->fpePrivate = 0;
    }
    (*pFont->unload_font) (pFont);
}

FontFileOpenBitmap (fpe, pFont, flags, entry, format, fmask)
    FontPathElementPtr	fpe;
    int			flags;
    FontEntryPtr	entry;
    FontPtr		*pFont;
{
    FontBitmapEntryPtr	bitmap;
    char		fileName[MAXFONTFILENAMELEN*2+1];
    int			ret;
    FontDirectoryPtr	dir;

    dir = (FontDirectoryPtr) fpe->private;
    bitmap = &entry->u.bitmap;
    strcpy (fileName, dir->directory);
    strcat (fileName, bitmap->fileName);
    ret = (*bitmap->renderer->OpenBitmap) 
			(fpe, pFont, flags, entry, fileName, format, fmask);
    if (ret == Successful)
    {
	bitmap->pFont = *pFont;
	(*pFont)->fpePrivate = (pointer) entry;
    }
    return ret;
}

FontFileGetInfoBitmap (fpe, pFontInfo, entry)
    FontPathElementPtr	fpe;
    FontInfoPtr		pFontInfo;
    FontEntryPtr	entry;
{
    FontBitmapEntryPtr	bitmap;
    char		fileName[MAXFONTFILENAMELEN*2+1];
    int			ret;
    FontDirectoryPtr	dir;

    dir = (FontDirectoryPtr) fpe->private;
    bitmap = &entry->u.bitmap;
    strcpy (fileName, dir->directory);
    strcat (fileName, bitmap->fileName);
    ret = (*bitmap->renderer->GetInfoBitmap) (fpe, pFontInfo, entry, fileName);
    return ret;
}

/* XXX code (from R5) does not check for memory alloc errors */
/* ARGSUSED */
FontFileListFonts (client, fpe, pat, len, max, names)
    pointer     client;
    FontPathElementPtr fpe;
    char       *pat;
    int         len;
    int         max;
    FontNamesPtr names;
{
    FontDirectoryPtr	dir;
    char		lowerChars[MAXFONTNAMELEN], zeroChars[MAXFONTNAMELEN];
    FontNameRec		lowerName;
    FontNameRec		zeroName;
    FontNamesPtr	scaleNames;
    FontScalableRec	vals, zeroVals, tmpVals;
    int			i;
    int			oldnnames;
    int			ret;

    if (len >= MAXFONTNAMELEN)
	return AllocError;
    dir = (FontDirectoryPtr) fpe->private;
    CopyISOLatin1Lowered (lowerChars, pat, len);
    lowerChars[len] = '\0';
    lowerName.name = lowerChars;
    lowerName.length = len;
    lowerName.ndashes = FontFileCountDashes (lowerChars, len);

    /* Add matching bitmap fonts to list before scalable fonts */
    oldnnames = names->nnames;
    ret = FontFileFindNamesInDir (&dir->nonScalable, &lowerName, max, names);
    max -= names->nnames - oldnnames;
    if (ret != Successful || max <= 0) {
	return (ret);
    }

    /* Now add matching scalable fonts to list: */
    strcpy (zeroChars, lowerChars);
    if (lowerName.ndashes == 14 &&
	FontParseXLFDName (zeroChars, &vals, FONT_XLFD_REPLACE_ZERO))
    {
	/* pattern is XLFD so do special XLFD processing: */
	scaleNames = MakeFontNamesRecord (0);
	if (!scaleNames)
	    return AllocError;
	zeroName.name = zeroChars;
	zeroName.length = strlen (zeroChars);
	zeroName.ndashes = lowerName.ndashes;
	FontFileFindNamesInDir (&dir->scalable, &zeroName, max, scaleNames);
	for (i = 0; i < scaleNames->nnames; i++)
	{
	    FontParseXLFDName (scaleNames->names[i], &zeroVals, FONT_XLFD_REPLACE_NONE);
	    tmpVals = vals;
	    if (FontFileCompleteXLFD (&tmpVals, &zeroVals))
	    {
		strcpy (zeroChars, scaleNames->names[i]);
		if (vals.pixel <= 0)
		    tmpVals.pixel = 0;
		if (vals.point <= 0)
		    tmpVals.point = 0;
		if (vals.width <= 0)
		    tmpVals.width = 0;
		if (vals.x == 0)
		    tmpVals.x = 0;
		if (vals.y == 0)
		    tmpVals.y = 0;
		FontParseXLFDName (zeroChars, &tmpVals, FONT_XLFD_REPLACE_VALUE);
		(void) AddFontNamesName (names, zeroChars, strlen (zeroChars));
	    }
	}
	FreeFontNames (scaleNames);
    }
    else
    {
	/* pattern is non-XLFD: */
    	FontFileFindNamesInDir (&dir->scalable, &lowerName, max, names);
    }
    return (Successful);
}

typedef struct _LFWIData {
    FontNamesPtr    names;
    int		    current;
} LFWIDataRec, *LFWIDataPtr;

FontFileStartListFontsWithInfo(client, fpe, pat, len, max, privatep)
    pointer     client;
    FontPathElementPtr fpe;
    char       *pat;
    int         len;
    int         max;
    pointer    *privatep;
{
    LFWIDataPtr	data;
    int		ret;

    data = (LFWIDataPtr) xalloc (sizeof *data);
    if (!data)
	return AllocError;
    data->names = MakeFontNamesRecord (0);
    if (!data->names)
    {
	xfree (data);
	return AllocError;
    }
    ret = FontFileListFonts (client, fpe, pat, len, max, data->names);
    if (ret != Successful)
    {
	FreeFontNames (data->names);
	xfree (data);
	return ret;
    }
    data->current = 0;
    *privatep = (pointer) data;
    return Successful;
}

/* ARGSUSED */
static int
FontFileListOneFontWithInfo (client, fpe, namep, namelenp, pFontInfo)
    pointer		client;
    FontPathElementPtr	fpe;
    char		**namep;
    int			*namelenp;
    FontInfoPtr		*pFontInfo;
{
    FontDirectoryPtr	dir;
    char		lowerName[MAXFONTNAMELEN];
    char		fileName[MAXFONTFILENAMELEN*2 + 1];
    FontNameRec		tmpName;
    FontEntryPtr	entry;
    FontScalableRec	vals;
    FontScalableEntryPtr   scalable;
    FontScaledPtr	scaled;
    FontBitmapEntryPtr	bitmap;
    FontAliasEntryPtr	alias;
    int			ret;
    char		*name = *namep;
    int			namelen = *namelenp;
    Bool		noSpecificSize;
    
    if (namelen >= MAXFONTNAMELEN)
	return AllocError;
    dir = (FontDirectoryPtr) fpe->private;
    CopyISOLatin1Lowered (lowerName, name, namelen);
    lowerName[namelen] = '\0';
    tmpName.name = lowerName;
    tmpName.length = namelen;
    tmpName.ndashes = FontFileCountDashes (lowerName, namelen);
    /* Match XLFD patterns */
    if (tmpName.ndashes == 14 &&
	FontParseXLFDName (lowerName, &vals, FONT_XLFD_REPLACE_ZERO))
    {
	tmpName.length = strlen (lowerName);
	entry = FontFileFindNameInDir (&dir->scalable, &tmpName);
	noSpecificSize = vals.point <= 0 && vals.pixel <= 0;
    	if (entry && entry->type == FONT_ENTRY_SCALABLE &&
	    FontFileCompleteXLFD (&vals, &entry->u.scalable.extra->defaults))
	{
	    scalable = &entry->u.scalable;
	    scaled = FontFileFindScaledInstance (entry, &vals, noSpecificSize);
	    /*
	     * A scaled instance can occur one of two ways:
	     *
	     *  Either the font has been scaled to this
	     *   size already, in which case scaled->pFont
	     *   will point at that font.
	     *
	     *  Or a bitmap instance in this size exists,
	     *   which is handled as if we got a pattern
	     *   matching the bitmap font name.
	     */
	    if (scaled)
	    {
		if (scaled->pFont)
		{
		    *pFontInfo = &scaled->pFont->info;
		    ret = Successful;
		}
		else if (scaled->bitmap)
		{
		    entry = scaled->bitmap;
		    bitmap = &entry->u.bitmap;
		    if (bitmap->pFont)
		    {
			*pFontInfo = &bitmap->pFont->info;
			ret = Successful;
		    }
		    else
		    {
			ret = FontFileGetInfoBitmap (fpe, *pFontInfo, entry);
		    }
		}
		else /* "cannot" happen */
		{
		    ret = BadFontName;
		}
	    }
	    else
	    {
#ifdef NOTDEF
		/* no special case yet */
		ret = FontFileMatchBitmapSource (fpe, pFont, flags, entry, &vals, format, fmask, noSpecificSize);
		if (ret != Successful)
#endif
		{
		    /* Make a new scaled instance */
	    	    strcpy (fileName, dir->directory);
	    	    strcat (fileName, scalable->fileName);
	    	    ret = (*scalable->renderer->GetInfoScalable)
			(fpe, *pFontInfo, entry, &tmpName, fileName, &vals);
		}
	    }
	    return ret;
	}
	CopyISOLatin1Lowered (lowerName, name, namelen);
	tmpName.length = namelen;
    }
    /* Match non XLFD pattern */
    if (entry = FontFileFindNameInDir (&dir->nonScalable, &tmpName))
    {
	switch (entry->type) {
	case FONT_ENTRY_BITMAP:
	    bitmap = &entry->u.bitmap;
	    if (bitmap->pFont)
	    {
	    	*pFontInfo = &bitmap->pFont->info;
	    	ret = Successful;
	    }
	    else
	    {
		ret = FontFileGetInfoBitmap (fpe, *pFontInfo, entry);
	    }
	    break;
	case FONT_ENTRY_ALIAS:
	    alias = &entry->u.alias;
	    *(char **)pFontInfo = name;
	    *namelenp = strlen (*namep = alias->resolved);
	    ret = FontNameAlias;
	    break;
	case FONT_ENTRY_BC:
#ifdef NOTYET
	    /* no LFWI for this yet */
	    bc = &entry->u.bc;
	    entry = bc->entry;
	    /* Make a new scaled instance */
    	    strcpy (fileName, dir->directory);
    	    strcat (fileName, scalable->fileName);
	    ret = (*scalable->renderer->GetInfoScalable)
		    (fpe, *pFontInfo, entry, tmpName, fileName, &bc->vals);
#endif
	    break;
	default:
	    ret = BadFontName;
	}
    }
    else
    {
	ret = BadFontName;
    }
    return ret;
}

FontFileListNextFontWithInfo(client, fpe, namep, namelenp, pFontInfo,
			     numFonts, private)
    pointer		client;
    FontPathElementPtr	fpe;
    char		**namep;
    int			*namelenp;
    FontInfoPtr		*pFontInfo;
    int			*numFonts;
    pointer		private;
{
    LFWIDataPtr	data = (LFWIDataPtr) private;
    int		ret;
    char	*name;
    int		namelen;

    if (data->current == data->names->nnames)
    {
	FreeFontNames (data->names);
	xfree (data);
	return BadFontName;
    }
    name = data->names->names[data->current];
    namelen = data->names->length[data->current];
    ret = FontFileListOneFontWithInfo (client, fpe, &name, &namelen, pFontInfo);
    if (ret == BadFontName)
	ret = AllocError;
    *namep = name;
    *namelenp = namelen;
    ++data->current;
    *numFonts = data->names->nnames - data->current;
    return ret;
}

typedef int (*IntFunc) ();
static int  font_file_type;

FontFileRegisterFpeFunctions()
{
    static Bool beenhere = FALSE;

    if (!beenhere) {
	FontFileRegisterFontFileFunctions ();
	beenhere = TRUE;
    }
    font_file_type = RegisterFPEFunctions(FontFileNameCheck,
					  FontFileInitFPE,
					  FontFileFreeFPE,
					  FontFileResetFPE,
					  FontFileOpenFont,
					  FontFileCloseFont,
					  FontFileListFonts,
					  FontFileStartListFontsWithInfo,
					  FontFileListNextFontWithInfo,
					  (IntFunc) 0,
					  (IntFunc) 0);
}
