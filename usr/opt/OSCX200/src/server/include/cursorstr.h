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
/* $XConsortium: cursorstr.h,v 1.6 89/08/04 18:23:09 rws Exp $ */
/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
#ifndef CURSORSTRUCT_H
#define CURSORSTRUCT_H 

#include "cursor.h"
#include "pixmap.h"
#include "misc.h"
/* 
 * device-independent cursor storage
 */

/*
 * source and mask point directly to the bits, which are in the server-defined
 * bitmap format.
 */
typedef struct _CursorBits {
    unsigned char *source;			/* points to bits */
    unsigned char *mask;			/* points to bits */
    unsigned short width, height, xhot, yhot;	/* metrics */
    int refcnt;					/* can be shared */
    pointer devPriv[MAXSCREENS];		/* set by pScr->RealizeCursor*/
} CursorBits, *CursorBitsPtr;

typedef struct _Cursor {
    CursorBitsPtr bits;
    unsigned short foreRed, foreGreen, foreBlue; /* device-independent color */
    unsigned short backRed, backGreen, backBlue; /* device-independent color */
    int refcnt;
    pointer devPriv[MAXSCREENS];		/* set by pScr->RealizeCursor*/
} CursorRec;

typedef struct _CursorMetric {
    unsigned short width, height, xhot, yhot;
} CursorMetricRec;

extern int		FreeCursor();
extern CursorPtr	AllocCursor();		/* also realizes it */
extern int		AllocGlyphCursor();	/* also realizes it */
				/* created from default cursor font */
extern CursorPtr	CreateRootCursor();

#endif /* CURSORSTRUCT_H */
