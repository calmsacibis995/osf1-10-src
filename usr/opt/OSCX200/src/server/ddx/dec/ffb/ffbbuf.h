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
 * @(#)$RCSfile: ffbbuf.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:08:13 $
 */
/*
 */

/*
 *
 *
 * NAME           ffbbuf.h
 *
 * Description    Defines data structures, etc. used in common by drawlib (and hence
 *		  lgi) and ddx.
 *                
 *
 * Notes
 *
 */

#ifndef FFBBUF_H
#define FFBBUF_H

#if FFBPIXELBITS==32
#  define WID_MASK	    	0xf0000000
#  define FFB_OVRLY_PLANES    	0x0f000000
#  define FFBISOVERLAYWINDOW(W)	((W)->drawable.depth == 4)                                                 
#else
#  define WID_MASK	0x00000000
#endif
#define WID_8		0x00000000
#define WID_8_B1	0x10000000
#define WID_8_B2	0x20000000
#define WID_8_C0	0x00000000	/* 0-2 */
#define WID_8_C1	0x40000000	/* 4-6 */

#define WID_12		0x30000000	/* 3: TrueColor */
#define WID_12_B1	0x70000000	/* 7: TrueColor */

#define WID_24		0x80000000	/* 8: TrueColor */
#define WID_24_C0	0x10000000	/* 9: DirectColor12/24 */
#define WID_24_C1	0x20000000	/* 10:DirectColor12/24 */

#define WID_8S		0xb0000000
#define WID_8S_B1	0xc0000000
#define WID_8S_B2	0xd0000000


/* We use distinct src and dst rotate and visual fields because the values loaded
   into rop and mode registers are different depending on whether the buffer is
   a src or a dst */
typedef struct {
    unsigned int	physDepth;	/* of frame buffer                        */
    unsigned int	depthbits;	/* logical depth of pixels in this buffer */
    unsigned int	pixelbits;	/* stride from pixel to pixel             */
    unsigned int	rotateDst;	/* which byte to rotate data into/out of  */
                                        /* meaningful only for 8-bit unpacked     */
    unsigned int        rotateSrc;      /* which byte to rotate data into/out of  */
    unsigned int        visualSrc;      /* specifies drawable depth if chip'ese   */
    unsigned int	visualDst;	/* specifies drawable depth if chip'ese   */
    unsigned int	planemask;	/* massaged version of what's in gc       */
    unsigned int	wid;		/* 0xf0000000 window type bits		  */
    unsigned int	clientPlaneMask;/* modified planemask */
} ffbBufferDescriptor;

typedef ffbBufferDescriptor ffbBufDesc, *ffbBufDPtr;

#endif /* FFBBUF_H */

/*
 * HISTORY
 */
