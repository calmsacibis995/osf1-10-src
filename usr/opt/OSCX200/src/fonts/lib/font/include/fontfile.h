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
 * $XConsortium: fontfile.h,v 1.1 91/05/11 09:11:58 rws Exp $
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

#ifndef _FONTFILE_H_
#define _FONTFILE_H_
typedef struct _FontEntry	    *FontEntryPtr;
typedef struct _FontTable	    *FontTablePtr;
typedef struct _FontName	    *FontNamePtr;
typedef struct _FontScaled	    *FontScaledPtr;
typedef struct _FontScalableExtra   *FontScalableExtraPtr;
typedef struct _FontScalableEntry   *FontScalableEntryPtr;
typedef struct _FontScaleAliasEntry *FontScaleAliasEntryPtr;
typedef struct _FontBitmapEntry	    *FontBitmapEntryPtr;
typedef struct _FontAliasEntry	    *FontAliasEntryPtr;
typedef struct _FontBCEntry	    *FontBCEntryPtr;
typedef struct _FontDirectory	    *FontDirectoryPtr;
typedef struct _FontRenderer	    *FontRendererPtr;

#define NullFontEntry		    ((FontEntryPtr) 0)
#define NullFontTable		    ((FontTablePtr) 0)
#define NullFontName		    ((FontNamePtr) 0)
#define NullFontScaled		    ((FontScaled) 0)
#define NullFontScalableExtra	    ((FontScalableExtra) 0)
#define NullFontscalableEntry	    ((FontScalableEntry) 0)
#define NullFontScaleAliasEntry	    ((FontScaleAliasEntry) 0)
#define NullFontBitmapEntry	    ((FontBitmapEntry) 0)
#define NullFontAliasEntry	    ((FontAliasEntry) 0)
#define NullFontBCEntry		    ((FontBCEntry) 0)
#define NullFontDirectory	    ((FontDirectoryPtr) 0)
#define NullFontRenderer	    ((FontRendererPtr) 0)

#define FONT_ENTRY_SCALABLE	0
#define FONT_ENTRY_SCALE_ALIAS	1
#define FONT_ENTRY_BITMAP	2
#define FONT_ENTRY_ALIAS	3
#define FONT_ENTRY_BC		4

#define MAXFONTNAMELEN	    1024
#define MAXFONTFILENAMELEN  1024

#define FontDirFile	    "fonts.dir"
#define FontAliasFile	    "fonts.alias"
#define FontScalableFile    "fonts.scale"

extern FontEntryPtr	FontFileFindNameInDir ();
extern FontDirectoryPtr	FontFileMakeDir ();
extern FontRendererPtr	FontFileMatchRenderer ();
extern char		*SaveString ();
extern FontScaledPtr	FontFileFindScaledInstance ();
#endif /* _FONTFILE_H_ */
