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
 * @(#)$RCSfile: ffbfill.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:09:16 $
 */
/*
 */

/*
 *
 *
 * NAME           ffbfill.h
 *
 * Description    Stuff for 2D fills, stipples, etc.
 *                
 *
 *
 */
#ifndef _N_FFBFILL_H
#define _N_FFBFILL_H

/*
 * drawable depth	drawable bpp	fbdepth		packed
 *------------------------------------------------------------
 * 	8		8		8		n
 *	8		8		32		y
 *	8		32		32		n
 *	12		32		32		n
 *	24		32		32		n
 */
#define FFBBLOCKTILEPIXELS	8

#define FFBFILLMODE(pDraw, pGC, solid, maxPixels, BLEH, FOO, SWINE)	      \
{									      \
    ffbGCPrivPtr ffbPriv = FFBGCPRIV(pGC);				      \
    FFBMode ffbFillMode;						      \
    if (ffbPriv->canBlock) {						      \
	if (solid | (pDraw->depth == 8)) {				      \
	    maxPixels = FFBMAXBSWPIXELS;				      \
	} else {							      \
	    maxPixels = FFBMAXBPWPIXELS;				      \
	}								      \
	ffbFillMode = BLOCKFILL;					      \
	BLEH;								      \
    } else {								      \
	extern int ffbMaxFillPixels[2][16];				      \
	maxPixels = ffbMaxFillPixels[pDraw->depth != 8][pGC->alu];	      \
	if (pGC->fillStyle == FillOpaqueStippled) {			      \
	    ffbFillMode = OPAQUEFILL;					      \
	} else {							      \
	    ffbFillMode = TRANSPARENTFILL;				      \
	}								      \
	FOO;								      \
    }									      \
    SWINE;								      \
}


#define REGION_RECTS_SETUP(pbox,nrectFill,prectBase,xDraw,yDraw)	      \
	long int clipx1, clipx2, clipy1, clipy2;			      \
	long ul, lr;							      \
									      \
	long nrect = nrectFill;						      \
	xRectangle *prect = prectBase;					      \
									      \
	clipx1 = pbox->x1 - xDraw;					      \
	clipy1 = pbox->y1 - yDraw;					      \
	clipx2 = pbox->x2 - xDraw;					      \
	clipy2 = pbox->y2 - yDraw;					      \
									      \
	ul = (clipy1 << 16) | (clipx1 & 0xffff);			      \
        lr = ((clipy2-1) << 16) | ((clipx2-1) & 0xffff)


#define REGION_SPANS_SETUP(pbox,nInit,pptInit,pwidthInit,xDraw,yDraw)	      \
	long int clipx1, clipx2, clipy1, clipy2;			      \
	register long ul, lr;						      \
									      \
	register long n = nInit;					      \
	register DDXPointPtr ppt = pptInit;				      \
        register int *pwidth = pwidthInit;				      \
									      \
	clipx1 = pbox->x1 - xDraw;					      \
	clipy1 = pbox->y1 - yDraw;					      \
	clipx2 = pbox->x2 - xDraw;					      \
	clipy2 = pbox->y2 - yDraw;					      \
									      \
	ul = (clipy1 << 16) | (clipx1 & 0xffff);			      \
        lr = ((clipy2-1) << 16) | ((clipx2-1) & 0xffff)



#define xShort(yx)	(((yx) << (LONG_BIT-16)) >> (LONG_BIT-16))
#define yShort(yx)	((yx) >> 16)
#define wShort(hw)	((hw) & 0xffff)
#define hShort(hw)	(((hw) >> 16) & 0xffff)


#define RECT_COMPLETELY_CLIPPED	{ prect++; if (--nrect > 0) continue; break; }
#define SPAN_COMPLETELY_CLIPPED	{ ppt++; pwidth++; if (--n > 0) continue; break; }


#define CLIP_SPANS(ppt,dw,pw,ul,lr,sign,cx1,cx2,cy1,cy2,x,y,ymul,w,S)	      \
{									      \
    register long yx1_, yx2_;						      \
									      \
    yx1_ = ((int *) ppt)[0];						      \
    y = yShort(yx1_);							      \
    ymul = y * dw;							      \
    x = xShort(yx1_);							      \
    w = *(pw);								      \
    yx2_ = yx1_ + w;							      \
									      \
    if ((((yx1_ - ul) | (lr - yx2_) | yx1_ | w) & sign) | (w == 0))	      \
    {									      \
	/* Ick, we have to clip at least partially */			      \
	if (y < cy1 | y >= cy2) { S; }					      \
	w += x;								      \
	if (x < cx1) x = cx1;						      \
	if (w > cx2) w = cx2;						      \
	w -= x;								      \
	if (w <= 0) { S; }						      \
    }									      \
}



#define CLIP_RECT(prect,dw,ul,lr,signbits,cx1,cx2,cy1,cy2,x,y,ymul,w,h,S)     \
{									      \
    register long yx1_, yx2_, dim_;					      \
									      \
    yx1_ = ((int *) prect)[0];						      \
    y = yShort(yx1_);							      \
    ymul = y * dw;							      \
    x = xShort(yx1_);							      \
    dim_ = ((int *) prect)[1];						      \
    yx2_ = yx1_ + dim_;							      \
    w = wShort(dim_);							      \
    h = hShort(dim_);							      \
									      \
    if ((((yx1_ - ul) | (lr - yx2_) | yx1_ | dim_) & signbits)		      \
	| (w == 0) | (h == 0))						      \
    {									      \
	register long x2_, y2_;						      \
	/* Ick, we have to clip at least partially */			      \
	x2_ = x + w;							      \
	y2_ = y + h;							      \
	if (x < cx1) x = cx1;						      \
	if (y < cy1) { y = cy1; ymul = y * dw; }			      \
	if (x2_ > cx2) x2_ = cx2;					      \
	if (y2_ > cy2) y2_ = cy2;					      \
	w = x2_ - x;							      \
	h = y2_ - y;							      \
	if ((w <= 0) | (h <= 0)) { S; }					      \
    }									      \
}

#endif /* _N_FFBFILL_H */


/*
 * HISTORY
 */
