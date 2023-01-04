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
 * @(#)$RCSfile: ffbteglyph.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:18:03 $
 */
/*
 */

#ifdef Bits64
/* Fixed-pitch PolyText glyph painters */
void ffbTESplatGlyphs6();
void ffbTESplatGlyphs8();
void ffbTESplatGlyphs10();
void ffbTESplatGlyphs15();
void ffbTESplatGlyphs16();
void ffbTESplatGlyphs20();
void ffbTESplatGlyphs30();
void ffbTESplatGlyphs32();

/* Fixed-pitch ImageText glyph painters */
void ffbTEImageGlyphs6();
void ffbTEImageGlyphs8();
void ffbTEImageGlyphs10();
void ffbTEImageGlyphs15();
void ffbTEImageGlyphs16();
void ffbTEImageGlyphs20();
void ffbTEImageGlyphs30();
void ffbTEImageGlyphs32();
#else
/* Fixed-pitch PolyText glyph painters */
void ffbTESplatGlyphs7();
void ffbTESplatGlyphs8();
void ffbTESplatGlyphs14();
void ffbTESplatGlyphs16();
void ffbTESplatGlyphs32();

/* Fixed-pitch ImageText glyph painters */
void ffbTEImageGlyphs7();
void ffbTEImageGlyphs8();
void ffbTEImageGlyphs14();
void ffbTEImageGlyphs16();
void ffbTEImageGlyphs32();
#endif

#include "ffbplygblt.h"

/*
 * HISTORY
 */
