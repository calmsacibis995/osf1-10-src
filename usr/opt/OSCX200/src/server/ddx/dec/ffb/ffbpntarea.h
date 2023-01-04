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
 * @(#)$RCSfile: ffbpntarea.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:13:18 $
 */
/*
 */

#ifndef FFBPNTAREA_H
#define FFBPNTAREA_H

#include "ffbfill.h"

/* Multipurpose painters */
extern void ffbFillSpansWord();
extern void ffbFillAreaWord();
extern void ffbFillAreaWord2();

/* Solid painters */
extern void ffbPolyFillRectSolid();	/* Solid rectangles w/clip code	     */
extern void ffbSolidFillSpans();	/* Solid spans			     */

/* Transparent stipple painters */
extern void ffbTSFillArea();		/* TS rectangles, w != FFBBUSBITS    */
extern void ffbTSFillSpans();		/* TS spans, w != FFBBUSBITS	     */
extern void ffbTSSpansWord();		/* TS spans, w = FFBBUSBITS	     */
extern void ffbTSFillAreaWord();	/* TS rectangles, w = FFBBUSBITS     */
extern void ffbTSFillAreaWord2();	/* TS rectangles, w = FFBBUSBITS, h=2^n*/

/* Opaque stipple painters */
extern void ffbOSFillArea();		/* OS rectangles, w != FFBBUSBITS    */
extern void ffbOSFillSpans();		/* OS spans, w != FFBBUSBITS	     */
extern void ffbOSSpansWord();		/* OS spans, w = FFBBUSBITS	     */
extern void ffbOSFillAreaWord();	/* OS rectangles, w = FFBBUSBITS     */
extern void ffbOSFillAreaWord2();	/* OS rectangles, w = FFBBUSBITS, h=2^n*/

/* Tile painters */
extern void ffbTileFillArea();		/* Tile rectangles, w != FFBBUSBITS  */
extern void ffbTileFillSpans();		/* Tile spans, w != FFBBUSBITS	     */
extern void ffbTileSpansWord();		/* Tile spans, w = FFBBUSBITS	     */
extern void ffbTilePolyAreaWord();	/* Tile rectangles, w = 8, clipped   */
extern void ffbTileFillAreaWord();	/* Tile rectangles, w = 8	     */

/* SetSpans painter */
extern void ffbSetSpans();

/* Opaque stipple CopyPlane painter */
extern void ffbOSPlane();

#endif /* FFBPNTAREA_H */

/*
 * HISTORY
 */
