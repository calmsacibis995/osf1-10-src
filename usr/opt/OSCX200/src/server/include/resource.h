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
/* $XConsortium: resource.h,v 1.12 91/05/03 17:57:10 keith Exp $ */
/***********************************************************
Copyright 1987, 1989 by Digital Equipment Corporation, Maynard, Massachusetts,
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
#ifndef RESOURCE_H
#define RESOURCE_H 1
#include "misc.h"

/*****************************************************************
 * STUFF FOR RESOURCES 
 *****************************************************************/

/* classes for Resource routines */

typedef unsigned long RESTYPE;

#define RC_VANILLA	((RESTYPE)0)
#define RC_CACHED	((RESTYPE)1<<31)
#define RC_DRAWABLE	((RESTYPE)1<<30)
#define RC_LASTPREDEF	RC_DRAWABLE
#define RC_ANY		(~(RESTYPE)0)

/* types for Resource routines */

#define RT_WINDOW	((RESTYPE)1|RC_CACHED|RC_DRAWABLE)
#define RT_PIXMAP	((RESTYPE)2|RC_CACHED|RC_DRAWABLE)
#define RT_GC		((RESTYPE)3|RC_CACHED)
#define RT_FONT		((RESTYPE)4)
#define RT_CURSOR	((RESTYPE)5)
#define RT_COLORMAP	((RESTYPE)6)
#define RT_CMAPENTRY	((RESTYPE)7)
#define RT_OTHERCLIENT	((RESTYPE)8)
#define RT_PASSIVEGRAB	((RESTYPE)9)
#define RT_LASTPREDEF	RT_PASSIVEGRAB
#define RT_NONE		((RESTYPE)0)

/* bits and fields within a resource id */
#define CLIENTOFFSET 22					/* client field */
#define RESOURCE_ID_MASK	0x3FFFFFL		/* low 22 bits */
#define CLIENT_BITS(id) ((id) & 0x1fc00000L)		/* hi 7 bits */
#define CLIENT_ID(id) ((int)(CLIENT_BITS(id) >> CLIENTOFFSET))
#define SERVER_BIT		0x20000000L		/* use illegal bit */

#ifdef INVALID
#undef INVALID	/* needed on HP/UX */
#endif

/* Invalid resource id */
#define INVALID	(0)

#define BAD_RESOURCE 0xe0000000L

RESTYPE CreateNewResourceType();
RESTYPE CreateNewResourceClass();
XID FakeClientID();
Bool AddResource();
void FreeResource();
void FreeClientResources();
Bool LegalNewID();
pointer LookupIDByType();
pointer LookupIDByClass();

#endif /* RESOURCE_H */
