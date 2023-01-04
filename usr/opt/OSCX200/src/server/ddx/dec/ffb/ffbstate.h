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
 * @(#)$RCSfile: ffbstate.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:17:53 $
 */
/*
 */
/*
 *
 *
 * NAME           ffbstate.h
 *
 * Description    Defines stuff for checking and loading hardware state.
 *                /
 *
 */

#ifndef _N_FFBSTATE_H
#define _N_FFBSTATE_H

/****************************************************************************
 *                     Ensure Hardware State Matches GC                     *
 ***************************************************************************/

/*
 * Source depth and rotate info is not set by 'CHECKSTATE' macro.
 * Most of the time, source depth specification is a compile-time
 * constant.  But for 12-bit sources it's not.  And source rotate
 * information is interesting only for 8-bit code. 
 * But these distinctions only save 1 instruction in each case, so
 * for now we just use 2 macros: src in main mem and src in hdwe.
 */
#define FFB_SRC_ROTATEDEPTH(psrc,spec)  			\
{								\
    ffbBufDesc  *bdptr;						\
    bdptr = FFBBUFDESCRIPTOR(psrc);				\
    spec = (bdptr->visualSrc | bdptr->rotateSrc);		\
}

#if FFBSRCDEPTHBITS==8
/* must be packed since in main memory */
#define FFB_SRC_ROTATEDEPTH_MAINMEM(psrc, spec)			\
{                                                               \
    spec = (PACKED_EIGHT_SRC << SRC_VISUAL_SHIFT) | ROTATE_SOURCE_0;	\
}
#else
/* some 32-bit pntarea code uses this branch (for unpacked) */
/* could be 24 or 12 depth, or unpacked; since in mainmem, rotate  */
/* is a don't care; can't use the version that uses the bdptr,     */
/* because might be called from cfb code with fake pixmap.	   */
#define FFB_SRC_ROTATEDEPTH_MAINMEM(psrc, spec)                 \
{                                                               \
    if (((DrawablePtr)(psrc))->depth == 8) {			\
        spec = PACKED_EIGHT_SRC << SRC_VISUAL_SHIFT;		\
    } else if (((DrawablePtr)(psrc))->depth == 12) {		\
        spec = TWELVE_BIT_BUF0_SRC << SRC_VISUAL_SHIFT;		\
    } else {							\
        spec = TWENTYFOUR_BIT_SRC << SRC_VISUAL_SHIFT;		\
    }								\
}
#endif


#define LOADSTATE(pdraw, scrPriv, ffb, pGC)				    \
{									    \
    ffbBufDPtr pbufD;							    \
    ffbGCPrivPtr ffbGCPriv;						    \
									    \
    ffbGCPriv = FFBGCPRIV(pGC);						    \
    pbufD = FFBBUFDESCRIPTOR(pdraw);					    \
    CYCLE_REGS(ffb);							    \
    FFBFOREGROUND(ffb, pGC->fgPixel);					    \
    FFBBACKGROUND(ffb, pGC->bgPixel);					    \
    FFBROP(ffb, pGC->alu, pbufD->rotateDst, pbufD->visualDst);		    \
    CYCLE_REGS(ffb);							    \
    FFBPLANEMASK(scrPriv, ffb, ffbGCPriv->planemask & pbufD->planemask);    \
    scrPriv->lastGC = pGC;						    \
} /* LOADSTATE */
    
#define CHECKSTATE(pScreen, pdraw, scrPriv, ffb, pGC)  \
{						\
    scrPriv = FFBSCREENPRIV(pScreen);		\
    ffb = scrPriv->ffb;				\
    if (pGC != scrPriv->lastGC) {		\
	LOADSTATE(pdraw, scrPriv, ffb, pGC);	\
    }						\
} /* CHECKSTATE */

/* see ffbpixmap.h for macro that accesses the buf descriptor */


#endif /* _N_FFBSTATE_H */

/*
 * HISTORY
 */
