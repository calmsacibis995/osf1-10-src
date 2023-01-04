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
/* $XConsortium: misc.h,v 1.4 92/11/18 21:00:41 gildea Exp $ */
/*
 * Copyright 1990, 1991 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices, Digital or
 * M.I.T. not be used in advertising or publicity pertaining to distribution
 * of the software without specific, written prior permission.
 *
 * NETWORK COMPUTING DEVICES, DIGITAL AND M.I.T. DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES,
 * DIGITAL OR M.I.T. BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 */

#ifndef _MISC_H_
#define _MISC_H_

#include	<X11/Xosdefs.h>
#include	<X11/Xfuncs.h>

#include	"assert.h"	/* so its everywhere */

#ifndef NULL

#ifndef X_NOT_STDC_ENV
#include	<stddef.h>
#else
#define	NULL	0
#endif

#endif

#define	MAXCLIENTS	128

typedef unsigned char *pointer;
typedef int Bool;
typedef unsigned long Atom;

#define	MILLI_PER_SECOND	(1000)
#define	MILLI_PER_MINUTE	(1000 * 60)

#ifndef TRUE
#define	TRUE 1
#define	FALSE 0
#endif

#define	min(a, b)	(((a) < (b)) ? (a) : (b))
#define	max(a, b)	(((a) > (b)) ? (a) : (b))
#define	abs(a)		((a) > 0 ? (a) : -(a))

#include	"os.h"

#define	lowbit(x)	((x) & (~(x) + 1))

/* byte swapping helpers */

/* byte swap a long literal */
#define lswapl(x) ((((x) & 0xff) << 24) |\
		   (((x) & 0xff00) << 8) |\
		   (((x) & 0xff0000) >> 8) |\
		   (((x) >> 24) & 0xff))

/* byte swap a short literal */
#define lswaps(x) ((((x) & 0xff) << 8) | (((x) >> 8) & 0xff))


/* byte swap a long */
#define swapl(x, n) n = ((char *) (x))[0];\
		 ((char *) (x))[0] = ((char *) (x))[3];\
		 ((char *) (x))[3] = n;\
		 n = ((char *) (x))[1];\
		 ((char *) (x))[1] = ((char *) (x))[2];\
		 ((char *) (x))[2] = n;

/* byte swap a short */
#define swaps(x, n) n = ((char *) (x))[0];\
		 ((char *) (x))[0] = ((char *) (x))[1];\
		 ((char *) (x))[1] = n

/* copy long from src to dst byteswapping on the way */
#define cpswapl(src, dst) \
                 ((char *)&(dst))[0] = ((char *) &(src))[3];\
                 ((char *)&(dst))[1] = ((char *) &(src))[2];\
                 ((char *)&(dst))[2] = ((char *) &(src))[1];\
                 ((char *)&(dst))[3] = ((char *) &(src))[0];

/* copy short from src to dst byteswapping on the way */
#define cpswaps(src, dst)\
		 ((char *) &(dst))[0] = ((char *) &(src))[1];\
		 ((char *) &(dst))[1] = ((char *) &(src))[0];


extern void SwapLongs();
extern void SwapShorts();

extern void CopyIsoLatin1Lowered();
extern void NoopDDA();
extern char *NameForAtom();
extern void BitOrderInvert();
extern void TwoByteInvert();
extern void FourByteInvert();

extern long GetTimeInMillis();


#if __STDC__ && !defined(UNIXCPP)
#define fsCat(x,y) x##_##y
#else
#define fsCat(x,y) x/**/_/**/y
#endif

/* copy a xCharInfo into a XCharInfo */

#define fsPack_XCharInfo(structure, packet) \
    fsCat(packet,left) = (structure)->leftSideBearing; \
    fsCat(packet,right) = (structure)->rightSideBearing; \
    fsCat(packet,width) = (structure)->characterWidth; \
    fsCat(packet,ascent) = (structure)->ascent; \
    fsCat(packet,descent) = (structure)->descent; \
    fsCat(packet,attributes) = (structure)->attributes


/* copy a FontInfoRec into a XFontInfoHeader */

#define fsPack_XFontInfoHeader(structure, packet, clientversion) \
    (packet)->font_header_flags = ((structure)->allExist) ? FontInfoAllCharsExist : 0; \
    (packet)->font_header_draw_direction = ((structure)->drawDirection == LeftToRight) \
               ? LeftToRightDrawDirection : RightToLeftDrawDirection; \
 \
    if ((structure)->inkInside) \
	(packet)->font_header_flags |= FontInfoInkInside; \
 \
    if (clientversion > 1) { \
	(packet)->font_header_char_range_min_char_high = (structure)->firstRow; \
        (packet)->font_header_char_range_min_char_low = (structure)->firstCol; \
        (packet)->font_header_char_range_max_char_high = (structure)->lastRow; \
        (packet)->font_header_char_range_max_char_low = (structure)->lastCol; \
        (packet)->font_header_default_char_high = (structure)->defaultCh >> 8; \
        (packet)->font_header_default_char_low = (structure)->defaultCh & 0xff; \
    } else { \
	(packet)->font_header_char_range_min_char_high = (structure)->firstCol; \
	(packet)->font_header_char_range_min_char_low = (structure)->firstRow; \
	(packet)->font_header_char_range_max_char_high = (structure)->lastCol; \
	(packet)->font_header_char_range_max_char_low = (structure)->lastRow; \
	(packet)->font_header_default_char_high = (structure)->defaultCh & 0xff; \
	(packet)->font_header_default_char_low = (structure)->defaultCh >> 8; \
    } \
 \
    fsPack_XCharInfo(&(structure)->ink_minbounds, (packet)->font_header_min_bounds); \
    fsPack_XCharInfo(&(structure)->ink_maxbounds, (packet)->font_header_max_bounds); \
 \
    (packet)->font_header_font_ascent = (structure)->fontAscent; \
    (packet)->font_header_font_descent = (structure)->fontDescent

#endif				/* _MISC_H_ */
