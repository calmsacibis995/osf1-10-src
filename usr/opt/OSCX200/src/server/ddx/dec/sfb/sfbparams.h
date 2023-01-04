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
/* These parameters determine everything else. */

/* Bits per pixel. */
#define SFBPIXELBITS     8

/* Number of pixels that opaque and transparent stipple modes affect. */
#define SFBSTIPPLEBITS  32

/* Number of pixels that copy mode affects. */
#define SFBCOPYBITS     32

/* Maximum number of bits that Bresenham line-drawing engine can handle. */
#define SFBLINEBITS     16

/* Bits on bus. */
#define SFBBUSBITS      32

/* Bits to VRAM interface. */
#define SFBVRAMBITS     64

/* Offset in pixels from starting scan line */
#define SFBSTARTPIXELS     4096
