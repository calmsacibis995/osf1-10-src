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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: ffbcopy.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:08:29 $";
#endif
/*
 */
/*
 *
 *
 * NAME           ffbcopy.c
 *
 * Description    Declares a table of all depth variants of all copy procedures which
 *                compile more than twice (for 8- and 32-bit depths) and are directly
 *		  callable by non-copy code (so dma variants are excluded).  Intended 
 *		  to make selection of the appropriate procedure easier and more 
 *		  uniform.
 *
 */

#include "ffb.h"
#include "ffbblt.h"	

/*
 * Each line has 4 rop possibilities for scr-mem copies;
 * a single mem-scr copier;
 * a single scr-scr copier; and a
 * fillarea variant.
 *
 * Each line handles one
 * src/dst depth combination. A depth combination is specified by 4 numbers
 * from {8,32}.  
 * The numbers correspond to
 *              o src physical pixel depth
 *              o src logical pixel depth
 *              o dst physical pixel depth
 *              o dst logical pixel depth
 * in that order.
 * ffbbitblt.c just uses the rop to index into this table, so the scr-mem
 * variants must come first in rop order on each line.
 */

void UnpackedMainMemError()
{
    /* We should never get here.  But I'd rather see this message than jump
       through NULL ! */
    ErrorF("Trying to copy unpacked 8-bit data in main memory\n");
}

VoidProc ffbCopyTab[6][_TILE_FILL_AREA+1] = {
    /* _PACKED_TO_PACKED */
                /* scr-mem */
    ffb8888BitbltScrMemCopy,	    ffb8888BitbltScrMemCopySPM,
    ffb8888BitbltScrMemXor,         ffb8888BitbltScrMemGeneral,
    ffb8888_GetSpans,
                /* mem-scr */
    ffb8888BitbltMemScr,
                /* scr-scr */
    ffb8888BitbltScrScr,
                /* fillspans */
    ffb8888TileFillSpans,
                /* fillarea */
    ffb8888TileFillArea,

    /* _PACKED_TO_UNPACKED */
                /* scr-mem */
		/* invalid direction/format */
    UnpackedMainMemError,	    UnpackedMainMemError,
    UnpackedMainMemError,	    UnpackedMainMemError,
    UnpackedMainMemError,
                /* mem-scr */
    ffb88328BitbltMemScr,
                /* scr-scr */
    ffb88328BitbltScrScr,
                /* fillspans */
    ffb88328TileFillSpans,
                /* fillarea */
    ffb88328TileFillArea,

    /* _UNPACKED_TO_PACKED */
                /* scr-mem */
    ffb32888BitbltScrMemCopy,       ffb32888BitbltScrMemCopySPM,
    ffb32888BitbltScrMemXor,	    ffb32888BitbltScrMemGeneral,
    ffb32888_GetSpans,
                /* mem-scr */
    UnpackedMainMemError,
                /* scr-scr */
    ffb32888BitbltScrScr,
                /* fillspans */
    ffb32888TileFillSpans,
                /* fillarea */
    ffb32888TileFillArea,

    /* _UNPACKED_TO_UNPACKED */
                /* scr-mem */
		/* invalid direction/format */
    UnpackedMainMemError,	    UnpackedMainMemError,
    UnpackedMainMemError,	    UnpackedMainMemError,
    UnpackedMainMemError,
                /* mem-scr */
    UnpackedMainMemError,
                /* scr-scr */
    ffb328328BitbltScrScr,
                /* fillspans */
    ffb328328TileFillSpans,
                /* fillarea */
    ffb328328TileFillArea,

    /* _THIRTYTWO_BITS_DEEP */
                /* scr-mem */
    ffb32323232BitbltScrMemCopy,    ffb32323232BitbltScrMemCopySPM,
    ffb32323232BitbltScrMemXor,     ffb32323232BitbltScrMemGeneral,
    ffb32323232_GetSpans,
                /* mem-scr */
    ffb32323232BitbltMemScr,
                /* scr-scr */
    ffb32323232BitbltScrScr,
                /* fillspans */
    ffb32323232TileFillSpans,
                /* fillarea */
    ffb32323232TileFillArea,

    /* _TWELVE_BITS_DEEP: same copiers as for 32 and 24, but visual
       stuffed into rop register is different; this makes the code
       simpler */
                /* scr-mem */
    ffb32323232BitbltScrMemCopy,    ffb32323232BitbltScrMemCopySPM,
    ffb32323232BitbltScrMemXor,     ffb32323232BitbltScrMemGeneral,
    ffb32323232_GetSpans,
                /* mem-scr */
    ffb32323232BitbltMemScr,
                /* scr-scr */
    ffb32323232BitbltScrScr,
                /* fillspans */
    ffb32323232TileFillSpans,
                /* fillarea */
    ffb32323232TileFillArea
    };

/*
 * HISTORY
 */
