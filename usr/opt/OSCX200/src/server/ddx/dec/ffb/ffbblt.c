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
static char *rcsid = "@(#)$RCSfile: ffbblt.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:07:18 $";
#endif
/*
 */

#include "X.h"
#include "Xprotostr.h"

#include "miscstruct.h"
#include "regionstr.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"

#include "ffb.h"
#include "ffbblt.h"
#include "bitblt.h"

#if FFBDEPTHBITS==32 && !defined(SCREEN_TO_MEM)
/* XXX: figure out how to make ffbgenffb do this */
/*
 * For 8-bit pix, hdwe reads lowest available byte of mask at a time and decodes lo-to-hi.
 * for 32-bit pix, it reads a lowest available nibble at a time, and decodes lo-to-hi.
 * vram writes are 64 bits; this is 2 big pixels, or 8 little guys.  Turn on the bit
 * corresponding to the pixel you want lit --- the screen is lit from left to right, 
 * from low bit enable to high.  So on a 32-bit screen, the pixels are numbered
 * {1,2,1,2,1,...}; on an 8-bit screen they go {1,2,4,8,1,2,4,8,1...8..}.
 */
CommandWord ffb32BackLeftMask[16] = {
    /* num masked pix to paint */	/* mask */
           /* 16*/			0xffff,
           /* 1	*/		    	0x2,
           /* 2 */                  	0x3,  		/* 0x2 | 0x1 */
           /* 3 */                  	0xb,		/* 0x8 | 0x3 */
           /* 4 */                  	0xf,		/* 0xc | 0x3 */
           /* 5 */                  	0x2f,		/* 0x20 | 0xf */
           /* 6 */                  	0x3f,		/* 0x20 | 0x10 | 0xf */
           /* 7 */                  	0xbf,		/* 0x80 | 0x30 | 0xf */
           /* 8 */                  	0xff,
           /* 9 */                  	0x2ff,
           /* 10*/                  	0x3ff,
           /* 11*/                  	0xbff,
           /* 12*/                  	0xfff,
           /* 13*/                  	0x2fff,
           /* 14*/                  	0x3fff,
           /* 15*/                  	0xbfff
	   };

CommandWord ffb32BackRightMask[3] = {
    /* pixel alignment of dst */	/* mask */
    /* after correction       */	/* always skip last pix */
           /* -2 */			0xfff4,	/* skip last 3 pix (2 + 1) */
	   /* -1 */			0xfffc, /* skip last 2 pix (1 + 1) */
	   /*  0 */			0xfffd  /* skip last 1 pix (0 + 1) */
	   };
#endif /* stuff for deep pixels */


#ifndef SCREEN_TO_MEM

/*
   In the copies below, we may need to read more data from the source
   than we write to the destination, in order to get the pump primed at
   the beginning of the copy, or to get the pump drained at the end of
   the copy.  We do this by (1) alway using a starting mask that includes
   the first word of the source, even though the destination mask says not
   to write the first word, and by (2) using an ending mask based upon the
   aligned destination width, which may be a little longer than what we'd
   get computing the mask based upon the source.
 */

void ffbBitbltScrScr(pSrcDraw, pDstDraw, prgnDst, pptSrc)
    DrawablePtr	pSrcDraw;
    DrawablePtr	pDstDraw;
    RegionPtr	prgnDst;
    DDXPointPtr pptSrc;
{
    /* 
     * Many of these variables come in editions of 2, as we need 1 set for
     * even numbered scanlines, one set for odd.
     */
    int			xdir;		/* 0: left to right, 1: right to left */
    int			ydir;		/* 0: top to bottom, 1: bottom to top */
    int			dstAlign0, dstAlign1;	/* Last few bits of destination ptr */
    int			srcAlign0, srcAlign1;    /* last few bits of source ptr      */
    int			shift;	/* Mostly dstAlign-srcAlign	    */
    int			width0, width1;	/* width to blt			    */
    register int	h;		/* height to blt		    */
    register BoxPtr	pbox;		/* current box to blt to	    */
    BoxPtr		pboxInit;       /* starting box of region	    */
    int 		nbox;

    Pixel8		*psrcBase, *pdstBase;	/* start of src, dst	    */
    int			 widthSrc, widthDst;	/* add to get to same position
    						   in next line		    */
    int			wS, wD;			/* for next even/odd line   */
    DDXPointPtr		ppt;			/* source location	    */
    Bool		careful;		/* Use sorted rects?	    */
    Pixel8		*psrcLine0, *psrcLine1;		/* Current source scanline  */
    Pixel8		*pdstLine0, *pdstLine1; 		/* Current dest scanline    */
    CommandWord		ones = ffbCopyAll1;
    CommandWord		mask0, mask1, leftMask0, leftMask1, rightMask0, rightMask1;
    FFB			ffb;
    int			i;
    CommandWord         rotdepthSrc;

    /* Have to be careful about overlapping copies if pDstDraw = pSrcDraw, or
       if both are windows. */
    DrawableBaseAndWidthPlus(pSrcDraw, psrcBase, widthSrc,
	careful = TRUE  /* if window */,
	careful = FALSE /* if pixmap */);

    DrawableBaseAndWidthPlus(pDstDraw, pdstBase, widthDst,
	/* Nothing */   /* if window */,
	careful = FALSE /* if pixmap */);

    if (pSrcDraw == pDstDraw) careful = TRUE;

    pboxInit = REGION_RECTS(prgnDst);
    nbox = REGION_NUM_RECTS(prgnDst);

    if (nbox == 0) return;

    /* Decide what direction to do copies in, so as not to lose data if the
       source and destination overlap. */
    xdir = 0;
    ydir = 0;
    if (careful) {
	xdir = (pptSrc->x < pboxInit->x1);
	ydir = (pptSrc->y < pboxInit->y1);
	if (nbox == 1  ||  (xdir | ydir) == 0) {
	    /* Process rectangle(s) in existing order. */
	    careful = FALSE;
	} else {
	    /* Yuck, gotta rearrange them. */
	    if (!SortRectsAndPoints(&pboxInit, &pptSrc, nbox, xdir, ydir))
		return;
	}
	if (ydir != 0) {
	    /* Walk source bottom to top */
	    widthSrc = -widthSrc;
	    widthDst = -widthDst;
	}
    }

    /* we advance 2 sets of pointers, so double the strides */
    wS = widthSrc << 1;
    wD = widthDst << 1;
    
    

    ffb = FFBSCREENPRIV(pDstDraw->pScreen)->ffb;
    FFB_SRC_ROTATEDEPTH(pSrcDraw, rotdepthSrc);
    WRITE_MEMORY_BARRIER();
    FFBMODE(ffb, COPY | rotdepthSrc);

    pbox = pboxInit;
    ppt = pptSrc;

    /* Put dst and src in different frame buffer aliases */
    pdstBase = CYCLE_FB(pdstBase);
    i = 0;
    do { 
	h = pbox->y2 - pbox->y1; 
        width0 = width1 = pbox->x2 - pbox->x1;

	/* i tells us which frame buffer aliases in which we left the source
	   and destination pointers, so we can make sure that the new pointers
	   start in different frame buffer aliases. */
	if (i) {
	    /* Oops, did an odd number of scanlines last rectangle. */
	    pdstBase = CYCLE_FB_DOUBLE(pdstBase);
	    psrcBase = CYCLE_FB_DOUBLE(psrcBase);
	}
	i = h & 1;
	if (ydir == 0) { 
	    psrcLine0 = psrcBase + (ppt->y * widthSrc); 
	    pdstLine0 = pdstBase + (pbox->y1 * widthDst); 
	} else { 
	    /* we negated widthSrc and widthDst earlier */
	    psrcLine0 = psrcBase - ((ppt->y+h-1) * widthSrc); 
	    pdstLine0 = pdstBase - ((pbox->y2-1) * widthDst); 
	} 
	/* It's possible that the src or dst ending mask address on one line
	   is very close to the src or dst starting mask address on the next
	   line. */
	psrcLine1 = CYCLE_FB_DOUBLE(psrcLine0 + widthSrc);
	pdstLine1 = CYCLE_FB_DOUBLE(pdstLine0 + widthDst);


	if (xdir == 0) { 
	    /* Forward copy */
	    /* figure first set of masks, etc */
	    CONJUGATE_FORWARD_ARGUMENTS(psrcLine0, pdstLine0,
		  srcAlign0,dstAlign0,shift,width0,
		  leftMask0,rightMask0,ppt->x, pbox->x1,
	          FFBCOPYALL1_SCRSCR, FFBMASKEDCOPYPIXELSMASK_SCRSCR);

	    /* do second set of masks */
	    CONJUGATE_FORWARD_ARGUMENTS(psrcLine1, pdstLine1,
                  srcAlign1,dstAlign1,shift,width1,
		  leftMask1,rightMask1,ppt->x, pbox->x1,
                  FFBCOPYALL1_SCRSCR, FFBMASKEDCOPYPIXELSMASK_SCRSCR);
	    
	    CYCLE_REGS(ffb);
	    FFBSHIFT(ffb, shift);
	    if ((width0 <= FFBCOPYPIXELS_SCRSCR) && 
		(width1 <= FFBCOPYPIXELS_SCRSCR)) {
		/* Copy fits into a single word; combine masks.	*/
		mask0 = leftMask0 & rightMask0;
                mask1 = leftMask1 & rightMask1;
		h -= 2;
		while (h >= 0) {
		    FFBWRITE(psrcLine0, rightMask0);
		    FFBWRITE(pdstLine0, mask0);
		    psrcLine0 += wS;
		    pdstLine0 += wD;
		    FFBWRITE(psrcLine1, rightMask1);
		    FFBWRITE(pdstLine1, mask1);
		    psrcLine1 += wS;
		    pdstLine1 += wD;
		    h-=2;
		} 
		h += 2;
		if (h){
		    FFBWRITE(psrcLine0, rightMask0);
		    FFBWRITE(pdstLine0, mask0);
		    psrcLine0 += wS;
		    pdstLine0 += wD;
		}
	    } else {
		/* At least even or odd rows require multiple words/row */
		mask0 = leftMask0 & rightMask0;
		mask1 = leftMask1 & rightMask1;
                h-=2;
                while (h >= 0){
		    if (width0 <= FFBCOPYPIXELS_SCRSCR) {
			FFBWRITE(psrcLine0, rightMask0);
			FFBWRITE(pdstLine0, mask0);
		    } else {
			COPY_MASKED_AND_UNMASKED(ffb, psrcLine0, pdstLine0,
			    shift, width0,
			    leftMask0, rightMask0,
			    FFBCOPYBYTESDONE_SCRSCR, FFBSRCCOPYBYTESDONE_SCRSCR,
			    FFBCOPYBYTESDONEUNMASKED, 
			    FFBSRCCOPYBYTESDONEUNMASKED);
		    }
		    psrcLine0 += wS;
		    pdstLine0 += wD;
		    if (width1 <= FFBCOPYPIXELS_SCRSCR) {
			FFBWRITE(psrcLine1, rightMask1);
			FFBWRITE(pdstLine1, mask1);
		    } else {
			COPY_MASKED_AND_UNMASKED(ffb, psrcLine1, pdstLine1,
			    shift, width1,
			    leftMask1, rightMask1,
			    FFBCOPYBYTESDONE_SCRSCR, FFBSRCCOPYBYTESDONE_SCRSCR,
			    FFBCOPYBYTESDONEUNMASKED, 
			    FFBSRCCOPYBYTESDONEUNMASKED);
		    }
		    psrcLine1+= wS;
		    pdstLine1 += wD;
                    h-=2;
                } 
		h += 2;
		if (h){
		    if (width0 <= FFBCOPYPIXELS_SCRSCR) {
			FFBWRITE(psrcLine0, rightMask0);
			FFBWRITE(pdstLine0, mask0);
		    } else {
			COPY_MASKED_AND_UNMASKED(ffb, psrcLine0, pdstLine0,
			    shift, width0,
			    leftMask0, rightMask0,
			    FFBCOPYBYTESDONE_SCRSCR, FFBSRCCOPYBYTESDONE_SCRSCR,
			    FFBCOPYBYTESDONEUNMASKED, 
			    FFBSRCCOPYBYTESDONEUNMASKED);
		    }
		    psrcLine0 += wS;
		    pdstLine0 += wD;
		}
	    }

	} else {
	    /* Backward copy */
	    CONJUGATE_BACKWARD_ARGUMENTS(psrcLine0, pdstLine0,
                  srcAlign0,dstAlign0,shift,width0,
                  leftMask0,rightMask0,ppt->x, pbox->x2,
		  FFBCOPYALL1_SCRSCR,
                  FFBMASKEDCOPYPIXELSMASK_SCRSCR);
	    CONJUGATE_BACKWARD_ARGUMENTS(psrcLine1, pdstLine1,
                  srcAlign1,dstAlign1,shift,width1,
                  leftMask1,rightMask1,ppt->x, pbox->x2,
                  FFBCOPYALL1_SCRSCR,
                  FFBMASKEDCOPYPIXELSMASK_SCRSCR);


	    CYCLE_REGS(ffb);
	    FFBSHIFT(ffb, shift);
	    if ((width0 <= FFBCOPYPIXELS_SCRSCR) &&
		(width1 <= FFBCOPYPIXELS_SCRSCR)) {
                /*
                 * Copy fits into a single word; combine masks.
                 */
		mask0 = leftMask0 & rightMask0;
                mask1 = leftMask1 & rightMask1;
		h -= 2;
	 	while (h >= 0){
		    FFBWRITE(psrcLine0, leftMask0);
		    FFBWRITE(pdstLine0, mask0);
		    psrcLine0 += wS;
		    pdstLine0 += wD;
		    FFBWRITE(psrcLine1, leftMask1);
		    FFBWRITE(pdstLine1, mask1);
		    psrcLine1 += wS;
		    pdstLine1 += wD;
                    h-=2;
		} 
		h += 2;
		if (h){
		    FFBWRITE(psrcLine0, leftMask0);
		    FFBWRITE(pdstLine0, mask0);
		    psrcLine0 += wS;
		    pdstLine0 += wD;
		}
	    } else {
		/* 
		 * At least even or odd rows require multiple words/row
		 */
		mask0 = leftMask0 & rightMask0;
		mask1 = leftMask1 & rightMask1;
		h -= 2;
		while (h >= 0) {
		    if (width0 <= FFBCOPYPIXELS_SCRSCR) {
			FFBWRITE(psrcLine0, leftMask0);
			FFBWRITE(pdstLine0, mask0);
		    } else {
			COPY_MASKED_AND_UNMASKED(ffb, psrcLine0, pdstLine0,
			    shift,width0,
			    rightMask0, leftMask0,
			    -FFBCOPYBYTESDONE_SCRSCR, 
			    -FFBSRCCOPYBYTESDONE_SCRSCR,
			    -FFBCOPYBYTESDONEUNMASKED,
			    -FFBSRCCOPYBYTESDONEUNMASKED);
		    }
		    psrcLine0 += wS;
		    pdstLine0 += wD;
		    if (width1 <= FFBCOPYPIXELS_SCRSCR) {
			FFBWRITE(psrcLine1, leftMask1);
			FFBWRITE(pdstLine1, mask1);
		    } else {
			COPY_MASKED_AND_UNMASKED(ffb, psrcLine1, pdstLine1,
			    shift,width1,
			    rightMask1, leftMask1,
			    -FFBCOPYBYTESDONE_SCRSCR, 
			    -FFBSRCCOPYBYTESDONE_SCRSCR,
			    -FFBCOPYBYTESDONEUNMASKED,
			    -FFBSRCCOPYBYTESDONEUNMASKED);
		    }
		    psrcLine1 += wS;
		    pdstLine1 += wD;
		    h-=2;
		} 
		h += 2;
		if (h){
		    if (width0 <= FFBCOPYPIXELS_SCRSCR) {
			FFBWRITE(psrcLine0, leftMask0);
			FFBWRITE(pdstLine0, mask0);
		    } else {
			COPY_MASKED_AND_UNMASKED(ffb, psrcLine0, pdstLine0,
			    shift,width0,
			    rightMask0, leftMask0,
			    -FFBCOPYBYTESDONE_SCRSCR, 
			    -FFBSRCCOPYBYTESDONE_SCRSCR,
			    -FFBCOPYBYTESDONEUNMASKED,
			    -FFBSRCCOPYBYTESDONEUNMASKED);
		    }
		    psrcLine0 += wS;
		    pdstLine0 += wD;
		}
	    }
	} /* end if forward copy else backward copy */

	pbox++;
	ppt++;
	nbox--;
    } while (nbox != 0);
    if (careful) {			/* ||| VMS? */
	xfree(pboxInit);
	xfree(pptSrc);
    }
}

#if (!((FFBSRCPIXELBITS==32) && (FFBSRCDEPTHBITS==8)))
/* Shouldn't ever have an unpacked 8-bit pixmap in main memory */
void ffbBitbltMemScr(pSrcDraw, pDstDraw, prgnDst, ppt)
    DrawablePtr	pSrcDraw;
    DrawablePtr	pDstDraw;
    RegionPtr	prgnDst;
    register DDXPointPtr ppt;
{
    register int	dstAlign;	/* Last few bits of destination ptr */
    register int	srcAlign;       /* last few bits of source ptr      */
    register int	shift;		/* Mostly dstAlign-srcAlign	    */
    register Pixel8	*psrc;		/* pointer to current src longword  */
    register Pixel8	*pdst;		/* pointer to current dst longword  */
    register int	width, wSav;	/* width to blt			    */
    register int	h, hSav;	/* height to blt		    */
    register BoxPtr	pbox;		/* current box to blt to	    */
    int 		nbox;

    Pixel8		*psrcBase, *pdstBase;	/* start of src, dst	    */
    int			 widthSrc, widthDst;	/* add to get to same position
    						   in next line		    */
    Pixel8		*psrcLine;		/* Current source scanline  */
    Pixel8		*pdstLine; 		/* Current dest scanline    */
    CommandWord		startmask,endmask,mask;
    CommandWord		ones = ffbSimpleAll1;
    FFB			ffb;
    CommandWord         rotdepthSrc;

    /* We know that the source is memory, and thus a pixmap.  We know that the
       destination is on the screen.  So we know that source and destination
       can't overlap. */

    ffb = FFBSCREENPRIV(pDstDraw->pScreen)->ffb;

    psrcBase = (Pixel8 *)(((PixmapPtr)pSrcDraw)->devPrivate.ptr);
    widthSrc = (int)(((PixmapPtr)pSrcDraw)->devKind);

    DrawableBaseAndWidth(pDstDraw, pdstBase, widthDst);

    nbox = REGION_NUM_RECTS(prgnDst);
    if (nbox == 0) return;
    pbox = REGION_RECTS(prgnDst);

    FFB_SRC_ROTATEDEPTH_MAINMEM(pSrcDraw, rotdepthSrc);

    /* Don't need to worry about src and dst in different frame buffer
       aliases...src is in main memory. */
    WRITE_MEMORY_BARRIER();

    /* ffbDoDMA will lock pages if it says okay for DMA */
    if (ffbDoDMA(psrcBase + ppt->y * widthSrc + ppt->x * FFBSRCPIXELBYTES,
	    widthSrc, FFB_PIXELBITS_TO_X_SHIFT(FFBSRCPIXELBITS), prgnDst,
	    DMAREADCOPY)) {
    
	FFBMODE(ffb, DMAREADCOPY | rotdepthSrc);
	do { 
	    ffbBitbltMemScrDMA(ffb, pbox, ppt, widthSrc, widthDst,
			       psrcBase, pdstBase);
	    pdstBase = CYCLE_FB(pdstBase);
            pbox++;
            ppt++;
            nbox--;
	} while (nbox != 0);
	/* Unlock pages */
	FFBSYNC(ffb);
	ffbUnwirePages();
	
    } else {

#ifndef USE_64
	FFBMODE(ffb, SIMPLE | rotdepthSrc);
	do { /* Per rectangle */
	    PixelWord   sA, sA1, sB, sC, sD, sE, sF, sG, sH, sI;
	    PixelWord   dA, dB;	
	    int		rotate,crotate,align,alignSav;

	    wSav = pbox->x2 - pbox->x1;
	    h = pbox->y2 - pbox->y1;
	    psrcLine = psrcBase + ppt->y * widthSrc + ppt->x * FFBSRCPIXELBYTES;
	    pdstLine =
		      pdstBase + pbox->y1 * widthDst + pbox->x1 * FFBPIXELBYTES;
	    dstAlign = (long)pdstLine & ((FFBBUSBYTES * FFBUNPACKED)-1);
	    srcAlign = (long)psrcLine & (FFBBUSBYTES - 1);
	    pdstLine -= dstAlign;
	    psrcLine -= srcAlign;
	    dstAlign /= FFBUNPACKED;
	    align = alignSav = dstAlign  - srcAlign;
#ifdef ndef         /* ||| for now aligned and unaligned will use same code. */ 
	    if (alignSav) {  /* unaligned */
#endif
		if(align < 0)	
		    align += 4;
		rotate = align << 3;
		crotate = 32 - rotate;
		startmask = FFBLEFTSIMPLEMASK(dstAlign,ones);
		endmask = FFBRIGHTSIMPLEMASK(wSav+dstAlign,ones);
		if (wSav + dstAlign <= (FFBBUSBYTES/FFBSRCPIXELBYTES)){
		    mask = startmask & endmask;
	            FFBPERSISTENTPIXELMASK(ffb,mask); 
		    if (alignSav >= 0) {
			do {
			    dA = (*((Pixel32 *)psrcLine)) << rotate;
			    FFBWRITE((Pixel32 *)pdstLine, dA); 
			    psrcLine += widthSrc;
			    pdstLine += widthDst;
			    h--;
			} while (h != 0);
		    } else {
			do {
		            sA1 = *((Pixel32 *)psrcLine) >> crotate;
		            sA  = *((Pixel32 *)(psrcLine + 4));
		            dA = (sA << rotate) | sA1;
			    FFBWRITE((Pixel32 *)pdstLine, dA); 
			    psrcLine += widthSrc;
			    pdstLine += widthDst;
			    h--;
			} while (h != 0);
		    }
		    pdstBase = CYCLE_FB(pdstBase);
		} else {
	            wSav -= FFBBUSBYTES/FFBSRCPIXELBYTES -  dstAlign;
		    hSav = h;
		    do {
		        psrc = psrcLine;
		        pdst = pdstLine;
	                width = wSav;
		        if (alignSav >= 0) {
		            sA  = *((Pixel32 *)psrc);
		            dA = sA << rotate;
			    psrc += 4;
		        } else {
		            sA1 = *((Pixel32 *)psrc) >> crotate;
		            sA  = *((Pixel32 *)(psrc + 4));
		            dA = (sA << rotate) | sA1;
			    psrc += 8;
		        }
			CYCLE_REGS(ffb);
		        FFBPIXELMASK(ffb,startmask);
		        FFBWRITE((Pixel32 *)pdst, dA);
			pdst += 4 * FFBUNPACKED;
			width -= 8 * (FFBBUSBYTES/FFBSRCPIXELBYTES);
		        while (width > 0){
			    READMAINMEM8;
 			    SIMPLE_COPY_COMPUTE_AND_WRITE8;  
		            sA = sI;
		            width -= 8 * FFBBUSBYTES/FFBSRCPIXELBYTES;
			    psrc += 4 * 8;
			    pdst += 4 * FFBUNPACKED * 8;
		        }
			width += (8-4) * FFBBUSBYTES/FFBSRCPIXELBYTES;  
			if (width >= 0){
			    READMAINMEM4;
 			    SIMPLE_COPY_COMPUTE_AND_WRITE4;  
		            sA = sE;
		            width -= 4 * FFBBUSBYTES/FFBSRCPIXELBYTES;
			    psrc += 4 * 4;
			    pdst += 4 * FFBUNPACKED * 4;
			}
			width += (4-1) * FFBBUSBYTES/FFBSRCPIXELBYTES;  
			while (width >= 0) {
		            sB = *((Pixel32 *)psrc);
		            dB = (sA >> crotate) | (sB << rotate);
		            FFBWRITE((Pixel32 *)pdst, dB);
		            sA = sB;
		            width -= FFBBUSBYTES/FFBSRCPIXELBYTES;
			    psrc += 4;
			    pdst += 4 * FFBUNPACKED;
			}
			width += FFBBUSBYTES/FFBSRCPIXELBYTES;  
			if (width) {
	                    sB = *((Pixel32 *)psrc);
		            dB = (sA >> crotate) | (sB << rotate);
			    CYCLE_REGS(ffb);
		            FFBPIXELMASK(ffb,endmask);
			    pdst = CYCLE_FB(pdst);
		            FFBWRITE((Pixel32 *)pdst, dB);
			}
		        psrcLine += widthSrc;
		        pdstLine += widthDst;
			pdstLine = CYCLE_FB_DOUBLE(pdstLine);
		        h--;
		    } while (h != 0);  	
		    if (hSav & 0x1){
			pdstBase = CYCLE_FB_DOUBLE(pdstBase);
		    }
		} /* end if super narrow else */  
#ifdef ndef 
	     } else { /* aligned */
		    ErrorF("unwritten code");
	     }  /* if unaligned else aligned copy */
#endif
	    pbox++;
	    ppt++;
	    nbox--;
	} while (nbox != 0);
    }
#else
        FFBMODE(ffb, SIMPLE | rotdepthSrc);
        do { /* Per rectangle */
            Pixel64     sA, sA1, sB, sC, sD, sE, sF, sG, sH, sI;
            Pixel64     dA,  dB;
            unsigned int      siA, siA1, siB, siC, siD, siE;
            unsigned int      diA, diB;
            int               rotate,crotate,align,alignSav,introtate,intcrotate;
            register int      addr_dstAlign;	/* Last few bits of destination ptr */
            register int      addr_srcAlign;       /* last few bits of source ptr      */
   
            wSav = pbox->x2 - pbox->x1;
            h = pbox->y2 - pbox->y1;
            psrcLine = psrcBase + ppt->y * widthSrc + ppt->x * FFBSRCPIXELBYTES;
            pdstLine =
                      pdstBase + pbox->y1 * widthDst + pbox->x1 * FFBPIXELBYTES;
            dstAlign = (long)pdstLine & 7; 
            srcAlign = (long)psrcLine & 7; 
            dstAlign /= FFBUNPACKED;
            addr_dstAlign = dstAlign & ((FFBBUSBYTES * FFBUNPACKED)-1);
            addr_srcAlign = srcAlign & ((FFBBUSBYTES * FFBUNPACKED)-1);
            pdstLine -= addr_dstAlign;
            psrcLine -= addr_srcAlign;
            align = alignSav = dstAlign  - srcAlign;
            startmask = FFBLEFTSIMPLEMASK(addr_dstAlign,ones);
            endmask = FFBRIGHTSIMPLEMASK(wSav+addr_dstAlign,ones);
  
            if (alignSav) {  /* unaligned */
                if(align < 0)
                    align += 8;
                rotate = align  << 3;
                crotate = 64 - rotate;
                introtate = (align &  ((FFBBUSBYTES * FFBUNPACKED)-1)) << 3;
                intcrotate = 32 - introtate;
                if ((wSav + addr_dstAlign) <= (FFBBUSBYTES/FFBSRCPIXELBYTES)){
                    mask = startmask & endmask;
                    FFBPERSISTENTPIXELMASK(ffb,mask);
		    do {
                        dA = (*((Pixel32 *)psrcLine)) << introtate;
                        FFBWRITE((Pixel32 *)pdstLine, dA);
                        psrcLine += widthSrc;
                        pdstLine += widthDst;
                        h--;
                    } while (h != 0);
                    pdstBase = CYCLE_FB(pdstBase);
                } else {
                    do {
                        psrc = psrcLine;
                        pdst = pdstLine;
                        width = wSav;
                        CYCLE_REGS(ffb);
                        FFBPIXELMASK(ffb,startmask);
                        if (dstAlign >= 4 || srcAlign >= 4){
                            if ( alignSav > 0){
                                if (srcAlign >= 4) {
                                    siA  = *((Pixel32 *)psrc);
                                    diA = siA << introtate;
                                    psrc += 4;
                                    sA = (Pixel64)siA << 32 | siA;
                                } else {
                                    siA  = *((Pixel32 *)psrc);
                                    siB  = *((Pixel32 *)(psrc + 4));
                                    diA = (siA << introtate) | (siB >> intcrotate);
                                    psrc += 8;
                                    sA = (Pixel64)siB << 32 | siA;
                                }
                                FFBWRITE((Pixel32 *)pdst, diA);
                                pdst += 4 * FFBUNPACKED;
                                width -= FFBBUSBYTES/FFBSRCPIXELBYTES -  addr_dstAlign;
                            } else {
                                if (dstAlign < 4){
                                    sB  = *((Pixel32 *)psrc);
                                    sA  = *((Pixel32 *)(psrc + 4)) << 32;
                                    dA = (sB | sA) << rotate;
                                    FFBWRITE64((Pixel64 *)pdst, dA);
                                    psrc += 4;
                                    pdst += 8 * FFBUNPACKED;
                                    width -= FFBBUSBYTES/FFBSRCPIXELBYTES -  addr_dstAlign +
                                                                (FFBBUSBYTES/FFBSRCPIXELBYTES);
                                } else {
                                    siA  = *((Pixel32 *)psrc);
                                    siB  = *((Pixel32 *)(psrc + 4));
                                    diA = (siA << introtate) | (siB >> intcrotate);
                                    FFBWRITE((Pixel32 *)pdst, diA);
                                    psrc += 4;
                                    pdst += 4 * FFBUNPACKED;
                                    sA = (Pixel64)siB << 32 | siB;
                                    width -= FFBBUSBYTES/FFBSRCPIXELBYTES -  addr_dstAlign;
                                }
                            }
                            width -= 8 * (FFBBUSBYTES/FFBSRCPIXELBYTES);
                        } else {
                            width -= (7 * (FFBBUSBYTES/FFBSRCPIXELBYTES)) +
                                        (FFBBUSBYTES/FFBSRCPIXELBYTES - addr_dstAlign);
                        }
                        while (width > 0){
                            READMAINMEM8;
                            SIMPLE_COPY_COMPUTE_AND_WRITE8;
                            sA = sE;
                            width -= 8 * FFBBUSBYTES/FFBSRCPIXELBYTES;
                            psrc += 4 * 8;
                            pdst += 4 * FFBUNPACKED * 8;
                        }
                        width += (8-4) * FFBBUSBYTES/FFBSRCPIXELBYTES;
                        if (width >= 0){
                            READMAINMEM4;
                            SIMPLE_COPY_COMPUTE_AND_WRITE4;
                            sA = sC;
                            width -= 4 * FFBBUSBYTES/FFBSRCPIXELBYTES;
                            psrc += 4 * 4;
                            pdst += 4 * FFBUNPACKED * 4;
                        }
                        width += (4-2) * FFBBUSBYTES/FFBSRCPIXELBYTES;
                        if (width >= 0) {
                            sB = *((Pixel64 *)psrc);
                            dB = (sA >> crotate) | (sB << rotate);
                            FFBWRITE((Pixel32 *)pdst, dB);
                            FFBWRITE((Pixel32 *)(pdst + 4), dB >> 32);
                            sA = sB;
                            psrc += 8;
                            pdst += 8 * FFBUNPACKED;
                        }
                        width -= FFBBUSBYTES/FFBSRCPIXELBYTES;
                        if (width >  0) {
                            sB = *((Pixel64 *)psrc);
                            dB = (sA >> crotate) | (sB << rotate);
                            FFBWRITE((Pixel32 *)pdst, dB);
                            CYCLE_REGS(ffb);
                            FFBPIXELMASK(ffb,endmask);
                            pdst = CYCLE_FB(pdst);
                            FFBWRITE((Pixel32 *)(pdst + 4), dB >> 32);
                        } else {
                            width += FFBBUSBYTES/FFBSRCPIXELBYTES;
                            if( width){
                                sB = *((Pixel64 *)psrc);
                                dB = (sA >> crotate) | (sB << rotate);
                                CYCLE_REGS(ffb);
                                FFBPIXELMASK(ffb,endmask);
                                pdst = CYCLE_FB(pdst);
                                FFBWRITE((Pixel32 *)pdst, dB);
                            }
                        }
                        psrcLine += widthSrc;
                        pdstLine += widthDst;
                        pdstLine = CYCLE_FB_DOUBLE(pdstLine);
                        h--;
                    } while (h != 0);
                    if (hSav & 0x1){
                        pdstBase = CYCLE_FB_DOUBLE(pdstBase);
                    }
                } /* end if super narrow else */
             } else { /* aligned */
                    if (wSav + addr_dstAlign <= (FFBBUSBYTES/FFBSRCPIXELBYTES)){
                        mask = startmask & endmask;
                        FFBPERSISTENTPIXELMASK(ffb, mask);
                        do {
                            FFBWRITE((Pixel32 *)pdstLine, (*((Pixel32 *)psrcLine)));
                            pdstLine += widthDst;
                            psrcLine += widthSrc;
                        } while (h != 0);
                        pdstBase = CYCLE_FB(pdstBase);
                    } else {
                        wSav -= FFBBUSBYTES/FFBSRCPIXELBYTES - addr_dstAlign;
                        do {
                            psrc = psrcLine;
                            pdst = pdstLine;
                            width = wSav;
                            CYCLE_REGS(ffb);
                            FFBPIXELMASK(ffb, startmask);
                            FFBWRITE((Pixel32 *)pdst, (*((Pixel32 *)psrc)));
                            pdst += 4 * FFBUNPACKED;
                            psrc += 4;
                            if (srcAlign < 4){
                                width -= 1 * (FFBBUSBYTES/FFBSRCPIXELBYTES);
                                if (width > 0){
                                    FFBWRITE((Pixel32 *)pdst, (*((Pixel32 *)psrc)));
                                    psrc += 4;
                                    pdst += 4 * FFBUNPACKED;
                                }
                            }
                            width -= 8 * (FFBBUSBYTES/FFBSRCPIXELBYTES);
                            while (width > 0){
                                READMAINMEM8;
                                FFBWRITE64((Pixel64 *)pdst, sB);
                                FFBWRITE64((Pixel64 *)(pdst + 8 * FFBUNPACKED), sC);
                                FFBWRITE64((Pixel64 *)(pdst + 16 * FFBUNPACKED), sD);
                                FFBWRITE64((Pixel64 *)(pdst + 24 * FFBUNPACKED), sE);
                                width -= 8 * (FFBBUSBYTES/FFBSRCPIXELBYTES);
                                psrc += 4 * 8;
                                pdst += 4 * FFBUNPACKED * 8;
                            }
                            width += (8 - 4) * FFBBUSBYTES/FFBSRCPIXELBYTES;
                            if (width > 0){
                                READMAINMEM4;
                                FFBWRITE64((Pixel64 *)pdst, sB);
                                FFBWRITE64((Pixel64 *)(pdst + 8 * FFBUNPACKED), sC);
                                width -= 4 * (FFBBUSBYTES/FFBSRCPIXELBYTES);
                                psrc += 4 * 4;
                                pdst += 4 * FFBUNPACKED * 4;
                            }
                            width += (4-1) * FFBBUSBYTES/FFBSRCPIXELBYTES;
                            while (width > 0){
                                FFBWRITE((Pixel32 *)pdst, (*((Pixel32 *)psrc)));
                                width -= FFBBUSBYTES/FFBSRCPIXELBYTES;
                                psrc += 4;
                                pdst += 4 * FFBUNPACKED;
                            }
                            width += FFBBUSBYTES/FFBSRCPIXELBYTES;
                            if (width){
                                CYCLE_REGS(ffb);
                                FFBPIXELMASK(ffb,endmask);
                                pdst = CYCLE_FB(pdst);
                                FFBWRITE((Pixel32 *)pdst, (*((Pixel32 *)psrc)));
                            }
                            psrcLine += widthSrc;
                            pdstLine += widthDst;
                            pdstLine = CYCLE_FB_DOUBLE(pdstLine);
                            h--;
                        } while (h != 0);
                        if (hSav & 0x1)
                            pdstBase = CYCLE_FB_DOUBLE(pdstBase);
                    }
             }  /* if unaligned else aligned copy */
            pbox++;
            ppt++;
            nbox--;
        } while (nbox != 0);
    } 
#endif
    CYCLE_REGS(ffb);
    FFBPIXELMASK(ffb,ones);  /* safety measure to flip back to one shot mode */
}
#endif

#else /* SCREEN_TO_MEM */

/* MYCFBCOPY is used in ffbbufdrain macro down inside the higher level macros */
#define MYCFBCOPY(src, pdst) \
    CFBCOPY(src, pdst, andbits1, xorbits1, andbits2, xorbits2)

#if !((FFBPIXELBITS==32) && (FFBDEPTHBITS==8))
/* Shouldn't ever have an unpacked 8-bit pixmap in main memory */
void FFBBITBLTSCRMEM(pSrcDraw, pDstDraw, prgnDst, ppt,
	    andbits1, xorbits1, andbits2, xorbits2)
    DrawablePtr	pSrcDraw;
    DrawablePtr	pDstDraw;
    RegionPtr	prgnDst;
    register DDXPointPtr ppt;
    PixelWord	andbits1;
    PixelWord	xorbits1;
    PixelWord	andbits2;
    PixelWord	xorbits2;
{
    register int	dstAlign;	/* Last few bits of destination ptr */
    register int	srcAlign;       /* last few bits of source ptr      */
    register int	shift;		/* Mostly dstAlign-srcAlign	    */
    register Pixel8	*psrc;		/* pointer to current src longword  */
    register Pixel8	*pdst;		/* pointer to current dst longword  */
    register int	width, wSav;	/* width to blt			    */
    register int	h;		/* height to blt		    */
    register BoxPtr	pbox;		/* current box to blt to	    */
    int 		nbox;

    Pixel8		*psrcBase, *pdstBase;	/* start of src, dst	    */
    int			 widthSrc, widthDst;	/* add to get to same position
    						   in next line		    */
    Pixel8		*psrcLine;		/* Current source scanline  */
    Pixel8		*pdstLine; 		/* Current dest scanline    */
    CommandWord		ones = ffbCopyAll1;
    CommandWord		mask, leftMask, rightMask, bits_mask = 0;
    int			m;
    FFB			ffb;
    Bool                doneFirstSet; /* even/odd scanline processing */
    int                 wS, wD;                 /* for next even/odd line   */
    register int        numS;           /* even/odd height                  */
    CommandWord         rotdepthSrc;

    /* 
     * We know that the destination is memory, and thus a pixmap.  We know that
     * the source is on the screen.  So we know that source and destination
     * can't overlap.
     */

    DrawableBaseAndWidth(pSrcDraw, psrcBase, widthSrc);
    pdstBase = (Pixel8 *)(((PixmapPtr)pDstDraw)->devPrivate.ptr);
    widthDst = (int)(((PixmapPtr)pDstDraw)->devKind);

    pbox = REGION_RECTS(prgnDst);
    nbox = REGION_NUM_RECTS(prgnDst);

    if (nbox == 0) return;

    ffb = FFBSCREENPRIV(pDstDraw->pScreen)->ffb;
    FFB_SRC_ROTATEDEPTH(pSrcDraw, rotdepthSrc);
    WRITE_MEMORY_BARRIER();

#ifdef DMAOK
    /* Can only do DMA if GXcopy */
    if (ffbDoDMA(pdstBase + pbox->y1*widthDst + pbox->x1*FFBPIXELBYTES,
	    widthDst, FFB_PIXELBITS_TO_X_SHIFT(FFBPIXELBITS), prgnDst,
	    DMAWRITECOPY)) {
	FFBMODE(ffb, DMAWRITECOPY | rotdepthSrc);
	/* andbits2 is effectively the planemask.  Do this, rather than
	   ~0, for GetImage ZPixmap semantics. */
	FFBDATA(ffb, andbits2);
	do { 
	    psrcBase = CYCLE_FB(psrcBase);
            ffbBitbltScrMemDMA(ffb, pbox, ppt, widthSrc, widthDst,
                               psrcBase, pdstBase);
	    pbox++;
	    ppt++;
	    nbox--;
	} while (nbox != 0);
	FFBSYNC(ffb);
	ffbUnwirePages();

    } else {
#endif /* DMAOK */

	FFBMODE(ffb, COPY | rotdepthSrc);
#if FFBDEPTHBITS == 32		/* mask extra bits for 12 and 24 visuals */
	COMPUTE_EXTRA_BITS_MASK(pDstDraw->depth,bits_mask);
#endif

	do { /* For each rectangle */
	    wSav = pbox->x2 - pbox->x1;
	    h = pbox->y2 - pbox->y1;

	    psrcBase = CYCLE_FB(psrcBase);
	    CYCLE_REGS(ffb);

	    /* process even numbered scanlines separately from odd */
	    doneFirstSet = 0;
	    
	    /*
	     * Need 2x widths and .5 h to implement even/odd scanline sets.
	     */
	    numS = (h >> 1) + (h & 1);
	    wS = widthSrc << 1;
	    wD = widthDst << 1;
    
	    do {
		width = wSav;
	       
		psrcLine = psrcBase + ((ppt->y + doneFirstSet) * widthSrc); 
		pdstLine = pdstBase + ((pbox->y1 + doneFirstSet) * widthDst); 
	
		CONJUGATE_FORWARD_ARGUMENTS(psrcLine,pdstLine,srcAlign,dstAlign,
					    shift,width,leftMask,rightMask,
					    ppt->x, pbox->x1,
					    FFBCOPYALL1, 
					    FFBMASKEDCOPYPIXELSMASK);
		CYCLE_REGS(ffb);
		FFBSHIFT(ffb, shift);
	
		if (width <= FFBCOPYPIXELS) {
		    /* The mask fits into a single word */
		    mask = leftMask & rightMask;
		    do {
			CYCLE_REGS(ffb);
			COPY_ONE_SCRMEM(ffb,psrcLine,pdstLine,rightMask,mask,
					bits_mask, wS, wD);
			numS--;
		    } while (numS != 0);
		} else {
		    /* Mask requires multiple words */
		    do {
			CYCLE_REGS(ffb);
			COPY_MULTIPLE_SCRMEM(ffb,psrcLine,pdstLine,width,
				    wS, wD, leftMask,rightMask, bits_mask);
			numS--;
		    } while (numS != 0);
		} /* if small copy else big copy */
	
		doneFirstSet ^= 1;              /* toggle sets */
		numS = h >> 1;
	    } while (doneFirstSet && numS);
	    pbox++;
	    ppt++;
	    nbox--;
	} while (nbox != 0);
#ifdef DMAOK
    }
#endif /* DMAOK */
}
#endif

#endif /* SCREEN_TO_MEM */

/*
 * HISTORY
 */
