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
 * @(#)$RCSfile: ffbtile8.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:18:25 $
 */
/*
 */
/*
 * Given rotate-right amount in pixels and tile params, expand tile
 * scanline to 8 pixels properly rotated.
 */

#define FFB8PRETILELOOP(ix)	{ ix <<= 3; }

#if LONG_BIT==32

#define FFB8STARTTILELOOP(_pSrc, width, _bits, ix)			      \
{									      \
    unsigned int r0, r1;						      \
    if (ix < LONG_BIT) {						      \
	r0 = _pSrc[0]; r1 = _pSrc[1];					      \
    } else {								      \
	r0 = _pSrc[1]; r1 = _pSrc[0]; ix -= LONG_BIT;			      \
    }									      \
    _bits[0] = ((r0 >> ix) | (r1 << (LONG_BIT-ix)));			      \
    _bits[1] = ((r1 >> ix) | (r0 << (LONG_BIT-ix)));			      \
}
#define FFB8TILESTRIDE(W)	(((W) + sizeof(int)-1) / sizeof(int))
#define FFB32TILESTRIDE(W)	(W)

#elif LONG_BIT==64

#define FFB8STARTTILELOOP(_pSrc, width, _bits, ix)			      \
{									      \
    register unsigned long qw = *(unsigned long *)_pSrc;		      \
    /* Do alignment of _bits */						      \
    *(unsigned long *)_bits = (qw << (LONG_BIT-ix)) | (qw >> ix);	      \
}
#define FFB8TILESTRIDE(W)	(2)
#define FFB32TILESTRIDE(W)	((W) + ((W)&1))
#endif /*LONG_BIT*/

#define FFB8LOADCOLORTILE(ffb,_bits,wid)				      \
{									      \
    FFBCOLOR0(ffb,_bits[0]);						      \
    FFBCOLOR1(ffb,_bits[1]);						      \
}

#define FFB32PRETILELOOP(ix)	{ /*nada*/ }

#define FFB32STARTTILELOOP(_pSrc, width, _bits, ix)			      \
{									      \
    _bits[(0-ix)&7] = _pSrc[0];						      \
    _bits[(1-ix)&7] = _pSrc[1];						      \
    _bits[(2-ix)&7] = _pSrc[2];						      \
    _bits[(3-ix)&7] = _pSrc[3];						      \
    _bits[(4-ix)&7] = _pSrc[4];						      \
    _bits[(5-ix)&7] = _pSrc[5];						      \
    _bits[(6-ix)&7] = _pSrc[6];						      \
    _bits[(7-ix)&7] = _pSrc[7];						      \
}

#define FFB32LOADCOLORTILE(ffb,_bits,wid)				      \
{									      \
    FFBCOLOR0(ffb, _bits[0]|wid);					      \
    FFBCOLOR1(ffb, _bits[1]|wid);					      \
    FFBCOLOR2(ffb, _bits[2]|wid);					      \
    FFBCOLOR3(ffb, _bits[3]|wid);					      \
    FFBCOLOR4(ffb, _bits[4]|wid);					      \
    FFBCOLOR5(ffb, _bits[5]|wid);					      \
    FFBCOLOR6(ffb, _bits[6]|wid);					      \
    FFBCOLOR7(ffb, _bits[7]|wid);					      \
}

#if FFBPIXELBITS==8
#define STARTTILELOOP(S,W,B,R,D)	     FFB8STARTTILELOOP(S,W,B,R)
#define LOADCOLORTILE(ffb,_bits,depth,wid)   FFB8LOADCOLORTILE(ffb,_bits,wid)

#elif FFBPIXELBITS==32
#define STARTTILELOOP(S,W,B,R,D)	{				      \
    if (D != 8) {							      \
	FFB32STARTTILELOOP(S,W,B,R);					      \
    } else {								      \
	FFB8STARTTILELOOP(S,W,B,R);					      \
    }									      \
}
#define LOADCOLORTILE(ffb,_bits,depth,wid)				      \
{									      \
    if (depth != 8) {							      \
	FFB32LOADCOLORTILE(ffb,_bits,wid);				      \
    } else {								      \
	FFB8LOADCOLORTILE(ffb,_bits,wid);				      \
    }									      \
} /*see also FFBLOADCOLORREGS()*/

#endif

/*
 * this stuff works for any depth
 */

#define ENDTILELOOP(pdst, dstwidth, psrcBase, _pSrc, iy,		      \
		    tileStride, tileHeight, height)			      \
{									      \
    pdst += dstwidth;							      \
    _pSrc += tileStride;						      \
    if (++iy == tileHeight) {						      \
	iy = 0;								      \
	_pSrc = psrcBase;						      \
    }									      \
    height--;								      \
}

#define SRCTILESTRIDE(pD, tileWidth) (pD->depth == 8 ?			      \
				      FFB8TILESTRIDE(tileWidth) :	      \
				      FFB32TILESTRIDE(tileWidth))

#define FFBPRETILELOOP(D,ix)		{				      \
    if (D != 8) {							      \
	FFB32PRETILELOOP(ix);						      \
    } else {								      \
	FFB8PRETILELOOP(ix);						      \
    }									      \
}

#define FFBTILERECT(pDraw,ffb,wid,ix,iy,madmax,				      \
		    psrcBase,tileWidth,tileHeight,tileStride,		      \
		    pdst,dstwidth,width,height)				      \
{									      \
    CommandWord *_pSrc = psrcBase + tileStride * iy;			      \
    CommandWord _bits[8]; /*Actual source bits for scanline*/		      \
									      \
    /* Offset pdst back so FFB-aligned. */				      \
    long _dalign = (long)pdst & FFBBUSBYTESMASK;			      \
    long _patAlign = (long)pdst & FFBSTIPPLEALIGNMASK;			      \
									      \
    pdst -= _dalign;							      \
									      \
    FFBBYTESTOPIXELS(_patAlign);					      \
    ix -= _patAlign;							      \
    ix &= 7;								      \
									      \
    FFBPRETILELOOP(pDraw->depth,ix);					      \
									      \
    if (width <= madmax) {						      \
	/* One write per scan line */					      \
	CommandWord mask = FFBLOADBLOCKDATA(_dalign,width);		      \
	do { /*For each line of height*/				      \
	    STARTTILELOOP(_pSrc, tileWidth, _bits, ix, pDraw->depth);	      \
	    CYCLE_REGS(ffb);						      \
	    LOADCOLORTILE(ffb, _bits, pDraw->depth, wid);		      \
	    FFBWRITE(pdst, mask);					      \
	    ENDTILELOOP(pdst, dstwidth, psrcBase, _pSrc, iy,		      \
			tileStride, tileHeight, height);		      \
	} while (height != 0);						      \
    } else {								      \
	/* Multi-word mask */						      \
	do { /*For each line of height*/				      \
	    Pixel8 *_p = pdst;						      \
	    int _w = width;						      \
	    CommandWord mask = FFBLOADBLOCKDATA(_dalign, madmax);	      \
	    STARTTILELOOP(_pSrc, tileWidth, _bits, ix, pDraw->depth);	      \
	    CYCLE_REGS(ffb);						      \
	    LOADCOLORTILE(ffb, _bits, pDraw->depth, wid);		      \
	    while (_w > madmax) {					      \
		FFBWRITE(_p, mask);					      \
		_p += madmax * FFBPIXELBYTES;				      \
		_w -= madmax;						      \
	    }								      \
	    FFBWRITE(_p, FFBLOADBLOCKDATA(_dalign,_w));			      \
	    ENDTILELOOP(pdst, dstwidth, psrcBase, _pSrc, iy,		      \
			tileStride, tileHeight, height);		      \
	} while (height != 0);						      \
    } /*if skinny else fat rectangle*/					      \
}


/*
 * Same as TILERECT, but without the height loop
 */
#define FFBTILESPAN(pDraw,ffb,wid,ix,iy,madmax,				      \
		    psrcBase,tileWidth,tileHeight,tileStride,		      \
		    pdst,dstwidth,width)				      \
{									      \
    CommandWord *_pSrc = psrcBase + tileStride * iy;			      \
    CommandWord _bits[8]; /*Actual source bits for scanline*/		      \
									      \
    /* Offset pdst back so FFB-aligned. */				      \
    long _dalign = (long)pdst & FFBBUSBYTESMASK;			      \
    long _patAlign = (long)pdst & FFBSTIPPLEALIGNMASK;			      \
									      \
    pdst -= _dalign;							      \
									      \
    FFBBYTESTOPIXELS(_patAlign);					      \
    ix -= _patAlign;							      \
    ix &= 7;								      \
									      \
    FFBPRETILELOOP(pDraw->depth,ix);					      \
    CYCLE_REGS(ffb);							      \
									      \
    if (width <= madmax) {						      \
	/* One write per scan line */					      \
	STARTTILELOOP(_pSrc, tileWidth, _bits, ix, pDraw->depth);	      \
	LOADCOLORTILE(ffb, _bits, pDraw->depth, wid);			      \
	FFBWRITE(pdst, FFBLOADBLOCKDATA(_dalign,width));		      \
    } else {								      \
	/* Multi-word mask */						      \
	Pixel8 *_p = pdst;						      \
	int _w = width;							      \
	CommandWord mask = FFBLOADBLOCKDATA(_dalign, madmax);		      \
	STARTTILELOOP(_pSrc, tileWidth, _bits, ix, pDraw->depth);	      \
	LOADCOLORTILE(ffb, _bits, pDraw->depth, wid);			      \
	while (_w > madmax) {						      \
	    FFBWRITE(_p, mask);						      \
	    _p += madmax * FFBPIXELBYTES;				      \
	    _w -= madmax;						      \
	}								      \
	FFBWRITE(_p, FFBLOADBLOCKDATA(_dalign,_w));			      \
    } /*if skinny else fat rectangle*/					      \
}

/*
 * HISTORY
 */
