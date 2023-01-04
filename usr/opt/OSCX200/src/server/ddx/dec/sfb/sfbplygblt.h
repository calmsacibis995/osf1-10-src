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

#ifndef SFBPLYGBLT_H
#define SFBPLYGBLT_H

/* Variable-pitch ImageText painter */
extern void sfbImageGlyphBlt();

extern void sfbPolyGlyphBlt();

#ifndef MITR5
#define FONTASCENT(font)		((font)->pCS->fontAscent)
#define FONTDESCENT(font)		((font)->pCS->fontDescent)
#define FONTMINBOUNDS(font, field)	((font)->pCS->minbounds.field)
#define TERMINALFONT(font)		((font)->pCS->terminalFont)
/* Temporary until someone resolves the calling interface to these 
 * routines
 */
#define SFBGLYPHBITS(ppci)	(Bits8 *)((ppci)->pPriv)
#else
#define SFBGLYPHBITS(ppci)	(Bits8 *)((ppci)->bits)
#endif

#endif
