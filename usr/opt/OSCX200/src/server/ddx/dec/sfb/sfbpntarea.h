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
/* Solid painters */
extern void sfbPolyFillRectSolid(); /* Solid rectangles w/clip code	      */
extern void sfbSolidFillSpans();    /* Solid spans			      */

/* Transparent stipple painters */
extern void sfbTSFillArea();	    /* TS rectangles, w != SFBBUSBITS         */
extern void sfbTSFillAreaWord();    /* TS rectangles, w = SFBBUSBITS          */
extern void sfbTSFillAreaWord2();   /* TS rectangles, w = SFBBUSBITS, h = 2^n */
extern void sfbTSFillSpans();       /* TS spans, w != SFBBUSBITS	      */
extern void sfbTSFillSpansWord();   /* TS spans, w = SFBBUSBITS	              */

/* Opaque stipple painters */
extern void sfbOSFillArea();	    /* OS rectangles, w != SFBBUSBITS         */
extern void sfbOSFillAreaWord();    /* OS rectangles, w = SFBBUSBITS	      */
extern void sfbOSFillAreaWord2();   /* OS rectangles, w = SFBBUSBITS, h = 2^n */
extern void sfbOSFillSpans();       /* OS spans, w != SFBBUSBITS	      */
extern void sfbOSFillSpansWord();   /* OS spans, w = SFBBUSBITS	              */

/* Tile painters */
extern void sfbTileFillArea();      /* Tile rectangles, w != SFBBUSBITS       */
extern void sfbTileFillAreaWord();  /* Tile rectangles, w = SFBBUSBITS        */
extern void sfbTileFillAreaWord2(); /* Tile rectangles, w = SFBBUSBITS, h=2^n */
extern void sfbTileFillSpans();     /* Tile spans, w != SFBBUSBITS	      */
extern void sfbTileFillSpansWord(); /* Tile spans, w = SFBBUSBITS	      */

/* SetSpans painter */
extern void sfbSetSpans();

/* Opaque stipple CopyPlane painter */
extern void sfbOSPlane();
