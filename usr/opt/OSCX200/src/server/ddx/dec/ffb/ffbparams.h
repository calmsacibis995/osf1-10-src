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
 * @(#)$RCSfile: ffbparams.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:12:00 $
 */
/*
 */

#ifndef FFBPARAMS_H
#define FFBPARAMS_H

/* These parameters determine everything else. */

/* Note that FFBPIXELBITS, FFBDEPTHBITS, FFBSRCPIXELBITS, and FFBSRCDEPTHBITS
   are usually defined using -D in the Makefile.  If not, we're looking at
   depth-independent code in ffbgenffb.c or ffb.c. ||| Would like to get rid
   of this hack. */

#ifndef FFBPIXELBITS
#define FFBPIXELBITS 8
#define FFBDEPTHBITS 8
#endif


/* Number of pixels that opaque and transparent stipple modes affect. */
#define FFBSTIPPLEBITS  32

/* Alignment for transparent and opaque stipples */     
#define FFBSTIPPLEALIGNMENT  (4 * FFBPIXELBITS / 8)

/* Macros that are highly dependent upon possible values for FFBPIXELBITS and
   the associated FFBSTIPPLELALIGNMENT. */

#define FFB_PIXELBITS_TO_X_SHIFT(ffbPixelBits) ((ffbPixelBits) >> 4)

#define FFB_PIXELBITS_TO_STIPPLE_ALIGNMASK(ffbPixelBits) \
    ((ffbPixelBits) >> 1) - 1)


/* Number of pixels that masked copies affect. */
#define FFBCOPYBITS     32

/* Size of on-chip copy buffer */
#define FFBCOPYBUFFERBYTES	64

/* The effective size of the copy pixel shifter for COPY, DMAREAD, and
   DMAWRITE modes.  These parameters precisely determine source and
   destination alignment constraints, except for a DMAWRITE destination.
*/
#define FFBCOPYSHIFTBYTES   8
#define FFBDMAWRITESHIFTBYTES   8
#define FFBDMAREADSHIFTBYTES    4

/* Number of words of copy buffer accessible directly from CPU */
#define FFBBUFFERWORDS  8


/* Maximum number of bits that Bresenham line-drawing engine can handle. */
#define FFBLINEBITS     16


/* Bits on bus. */
#define FFBBUSBITS      32


/* Bits to VRAM interface. */
#define FFBVRAMBITS     64


/* Offset in pixels from starting scan line */
#define FFBSTARTPIXELS     4096


/* Number of bytes used for cursor (if RAMDAC doesn't support it) */
#define FFBCURSORBYTES     1024

#endif /* FFBPARAMS_H */

/*
 * HISTORY
 */
