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
static char *rcsid = "@(#)$RCSfile: vram.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:48:55 $";
#endif
/*
 */

#include <stdio.h>

#ifdef SOFTWARE_MODEL
#include "model/types.h"
#include "model/defs.h"
#include "ffbvars.h"
#include "ffbparams.h"
#include "ffbvram.h"

#include "N_OS.h"			/* __MB() */

#define SCREEN_WIDTH    1280
#define SCREEN_HEIGHT   1024

#if 0
    From the spec:

	24-plane True Color:

	31    28 27   24 23           16 15            8 7             0
	+-------+-------+---------------+---------------+---------------+
	| 1000  | ovlay |      Red      |     Green     |     Blue      |
	+-------+-------+---------------+---------------+---------------+

	12-plane True Color Buffer 0:

	31    28 27   24 23   20 19   16 15   12 11    8 7     4 3     0
	+-------+-------+-------+-------+-------+-------+-------+-------+
	| 0011  | ovlay |  Red  | xxxxx | Green | xxxxx |  Blue | xxxxx |
	+-------+-------+-------+-------+-------+-------+-------+-------+

	12-plane True Color Buffer 1:

	31    28 27   24 23   20 19   16 15   12 11    8 7     4 3     0
	+-------+-------+-------+-------+-------+-------+-------+-------+
	| 0111  | ovlay | xxxxx |  Red  | xxxxx | Green | xxxxx |  Blue |
	+-------+-------+-------+-------+-------+-------+-------+-------+

	8-plane Colormap 0 Buffer 0:

	31    28 27   24 23           16 15            8 7             0
	+-------+-------+---------------+---------------+---------------+
	| 0000  | ovlay |     xxxxx     |     xxxxx     |     index     |
	+-------+-------+---------------+---------------+---------------+

	8-plane Colormap 0 Buffer 1:

	31    28 27   24 23           16 15            8 7             0
	+-------+-------+---------------+---------------+---------------+
	| 0001  | ovlay |     xxxxx     |     index     |     xxxxx     |
	+-------+-------+---------------+---------------+---------------+

	8-plane Colormap 0 Buffer 2:

	31    28 27   24 23           16 15            8 7             0
	+-------+-------+---------------+---------------+---------------+
	| 0010  | ovlay |     index     |     xxxxx     |     xxxxx     |
	+-------+-------+---------------+---------------+---------------+

	8-plane Colormap 1 Buffer 0:

	31    28 27   24 23           16 15            8 7             0
	+-------+-------+---------------+---------------+---------------+
	| 0100  | ovlay |     xxxxx     |     xxxxx     |     index     |
	+-------+-------+---------------+---------------+---------------+

	8-plane Colormap 1 Buffer 1:

	31    28 27   24 23           16 15            8 7             0
	+-------+-------+---------------+---------------+---------------+
	| 0101  | ovlay |     xxxxx     |     index     |     xxxxx     |
	+-------+-------+---------------+---------------+---------------+

	8-plane Colormap 1 Buffer 2:

	31    28 27   24 23           16 15            8 7             0
	+-------+-------+---------------+---------------+---------------+
	| 0110  | ovlay |     index     |     xxxxx     |     xxxxx     |
	+-------+-------+---------------+---------------+---------------+

Joel
#endif

/*
 * control bits for vdac to decode:
 *	_<planes><channel><cmap>_
 *
 *	cmap:	000-255 = true color	(#1)
 *		256-511 = pseudo color	(#2)
 *		512-527 = overlay
 */
#define _24___	8			/* 0:23 */
#define _12_1_	7			/* 0:3+8:11+16:19 */
#define _12_0_	3			/* 4:7+12:15+20:23*/
#define _08b0_	0			/* 0:7 */
#define _08g0_	1			/* 8:15 */
#define _08r0_	2			/* 16:23 */
#define _08b1_	4			/* 0:7 */
#define _08g1_	5			/* 8:15 */
#define _08r1_	6			/* 16:23 */


FILE *vramFile;
FILE *cmdFile;
int lastMatchLine;
     
extern FILE *vramOut;
extern FILE *cmdOut;
extern int logFlag;
extern int UsePxg;
/* extern */ int vramCompareFlag;   /* Don't have this in high-level model */
extern DEEPREG shadowDeep;
        
unsigned	char *vram;
unsigned	vram_bytes;

int		screen_width;
int		phys_screen_width;
int		screen_height;
unsigned char   *sfbVRAM;


extern struct model_colormap cmapModel;
#endif /*SOFTWARE_MODEL*/

/*
 * We should never call GetDmaLine. This is a place holder for the HW people's version.
*/

char *GetDmaLine ()
{
    printf("GetDmaLine called....abort()!\n");
    abort();
}


#ifdef SOFTWARE_MODEL
#define FFBPIXELBYTES (FFBPIXELBITS/8)

static long ibuf;
static int *poll, *cbuf;
static unsigned int pxgOpt, pxgX, pxgY, pxgFG;
pxInfo *pxInfoPtr;  /* Shared with ffbscrinit.c */


InitVRAM()
{
    int i;

    /*
     * initialize memory to a pseudo-random pattern.
     */
    srandom (1);

    if (UsePxg) {
	/* Set up pointers to PXG */
	poll =  PQO_POLL(pxInfoPtr);
	cbuf = _PQO_RAM(pxInfoPtr);
    }

    for (i = 0; i < vram_bytes/sizeof(unsigned) ; ++i)
	((unsigned *)vram)[i] = random();
}

/*
 * read or write the VRAM's.
 *
 * always returns 8 BYTES of data from the addressed (8-PIXEL) word.
 */

static int byteMaskExpand[16] = {
    0x00000000, 0x000000ff, 0x0000ff00, 0x0000ffff,
    0x00ff0000, 0x00ff00ff, 0x00ffff00, 0x00ffffff,
    0xff000000, 0xff0000ff, 0xff00ff00, 0xff00ffff,
    0xffff0000, 0xffff00ff, 0xffffff00, 0xffffffff
};

ULONG64
do_rams(uaddr, data, planemask, bytemask, we, depth32)
    unsigned uaddr;		/* 8-PIXEL ``word'' address		*/
    ULONG64 data;		/* 8-BYTE data to write (if any)	*/
    unsigned planemask;		/* planemask				*/
    unsigned bytemask;		/* 8 bits of byte enable		*/
    unsigned we;		/* write enable	(0 means write)		*/
    unsigned depth32;		/* non zero if 32 bit system		*/
{
    int i, mask;
    int *wAddr;
    ULONG64 vram_return;
    int addr, addr_adj;
    int byte_shift;

    addr = (int)uaddr;
    bytemask &= 0xff;		/* ||| Shouldn't need this, right? */

    if (depth32) {
	addr &= 0x1fffff;
        byte_shift = 2;
    } else {
        addr &= 0xfffff;                    /* (8MP/8)-1 */
        byte_shift = 0;
    }
    if (addr >= (vram_bytes/8)) 
        abort();

    if (we == 0) {			/** asserted low, so write it **/
	if (logFlag)
	    fprintf (vramOut, "%x %x %x %x %x\n", 
		     addr, data.lo, data.hi, planemask, bytemask);
    }
    addr *= 8; 

    /* This code deals display a virtual display of stride 1284 pixels on a 
       display with stride 1280 pixels */

    addr_adj = addr;
    addr -= (FFBSTARTPIXELS << byte_shift);
    if (screen_width != phys_screen_width
	    && addr > 0 
	    && addr < (screen_height * (phys_screen_width << byte_shift))) {
	/* Remap weird virtual screen size onto actual screen size */
	addr_adj = (addr/(phys_screen_width << byte_shift)) 
		    * (screen_width << byte_shift);
	addr_adj += addr % (phys_screen_width << byte_shift);
	if((addr % (phys_screen_width << byte_shift)) 
		    >= (screen_width << byte_shift)) 
	    addr_adj -= 4;
    addr_adj += (FFBSTARTPIXELS << byte_shift);
    }
    wAddr = (int *) (vram + addr_adj);

    if (we == 0) {			/** asserted low, so write it **/
	mask = byteMaskExpand[bytemask & 0xf] & planemask;
	i = 4;				/* Hint about # pixels to redisplay */
	if (mask == 0xffffffff) {
	    wAddr[0] = data.lo;
	} else if (mask == 0) {
	    i = 0;
	    bytemask = (bytemask & 0xf0) | (bytemask >> 4);
	    addr_adj += 4;
	} else {
	    wAddr[0] = (wAddr[0] & ~mask) | (data.lo & mask);
	}
	mask = byteMaskExpand[bytemask >> 4] & planemask;
	if (mask == 0xffffffff) {
	    wAddr[1] = data.hi;
	    i += 4;
	} else {
	    wAddr[1] = (wAddr[1] & ~mask) | (data.hi & mask);
	    if (mask != 0) i += 4;
	}
	/*
	 * now transfer the pixels in vram[] to the pxg fb:
	 *	1. map vram[] to display[]
	 *	2. write_spans
	 */

	if (i && (planemask & 0x0fffffff)) {
	    if (UsePxg) {
		if (depth32) {
		    mapVramToDisplay(&vram[addr_adj], 
				     (addr_adj >> byte_shift) - FFBSTARTPIXELS, 
				     i >> byte_shift, depth32,
				     &vram[addr_adj], bytemask);
		} else {
		    mapVramToDisplay(&vram[addr_adj],
				     addr_adj - FFBSTARTPIXELS, i, depth32,
				     &data, bytemask);
		}
	    } else {
		/* HX */
		*((int *)(sfbVRAM + addr_adj)) = *((int *)(vram + addr_adj));
		*((int *)(sfbVRAM + addr_adj + 4)) = 
		    *((int *)(vram + addr_adj + 4));
	    }
	}
    }
    vram_return.lo = wAddr[0]; 
    vram_return.hi = wAddr[1]; 
    return (vram_return);
}

vdac459(src, dst, len)
    unsigned char *src;
    unsigned int *dst;
    int len;
{
    unsigned int pixel;
    unsigned int red;
    unsigned int blu;
    unsigned int gre;

    while (len--) {
	pixel = *src++;
	red = cmapModel.palette[2][pixel];
	gre = cmapModel.palette[1][pixel];
	blu = cmapModel.palette[0][pixel];
	*dst++ = (red << 16) | (gre << 8) | blu;
    }
}


vdac463(src, dst, len)
    unsigned int *src;
    unsigned int *dst;
    int len;
{
    unsigned int pixel, format;
    unsigned int red;
    unsigned int blu;
    unsigned int gre;

    int off = 0;

    while (len--) {
	pixel = 0xf0ffffff &  *src++;	/* ||| */

	if (pixel & 0x0f000000) {
	    /* no overlay masking supported */
	    red = gre = blu = ((pixel & 0xf000000) >> 24) + 512;
	}
	else {
	    format = (pixel & 0xf0000000) >> 28;
	    switch (format) {
	     /* There are lots of abort() calls in here that eventually go
	        away.  But for now, they help catch errors in setting window
		id's, as the only valid id's we should see are 24-plane and
		8-bit buffer 0, colormap 0. ||| */
	    
	     case _24___:
		red = (pixel >> 16) & 0xff;
		gre = (pixel >> 8)  & 0xff;
		blu = (pixel)       & 0xff;
		off = -1;
		break;
	     case _12_1_:
		red = (pixel & 0x0f0000) >> 12;
		gre = (pixel & 0x000f00) >> 4;
		blu = (pixel & 0x00000f) << 4;
		off = -1;
		break;
	     case _12_0_:
		red = (pixel & 0xf00000) >> 16;
		gre = (pixel & 0x00f000) >> 8;
		blu = (pixel & 0x0000f0);
		off = -1;
		break;
	     case _08b1_:
		off = 256;
	     case _08b0_:
		red = gre = blu = ((pixel >> 0)  & 0xff) + off;
		break;
	     case _08g1_:
		off = 256;
	     case _08g0_:
		red = gre = blu = ((pixel >> 8)  & 0xff) + off;
		break;
	     case _08r1_:
		off = 256;
	     case _08r0_:
		red = gre = blu = ((pixel >> 16) & 0xff) + off;
		break;
    
	     default:
		abort();
	    }
	}
	if (off >= 0)
	{
	    red = cmapModel.palette[2][red];
	    gre = cmapModel.palette[1][gre];
	    blu = cmapModel.palette[0][blu];
	}
	*dst++ = ((red << 16) | (gre << 8) | blu);
    }
}


mapVramToDisplay(vram, addr, i, depth32, ppixel, align)
    void *vram;
    unsigned addr;
    int i;
    unsigned depth32;
    void *ppixel;
    unsigned align;
{
    unsigned y = addr / screen_width;
    unsigned x = addr % screen_width;
    int k, j;

    if (y >= SCREEN_HEIGHT)		/* visible height */
	return;
    if (x >= SCREEN_WIDTH)		/* taken care of by addr_adj */
	return;				/* code for 8-bit case? */

    switch (pxgOpt) {
     case 0:
	break;
     case 1:
	pxgX = x + ffs(align)-1;
	pxgY = y;
	if (depth32)
	    vdac463(ppixel, &pxgFG, 1);
	else
	    vdac459(ppixel, &pxgFG, 1);
     default:
	pxgOpt += 1;
	return;
    }

    if (ibuf) {
	ibuf ^= 0x100;
    }
    else {
	/* init */
	ibuf = (long)cbuf + 0x1000;
	cbuf[0] = 7;
	cbuf[1] = 0x1ffffff;
	cbuf[2] = 0;
	cbuf[3] = (0x20<<12) | (1<<19) | 1;
	/* assume idle 1st time through */
	PQO_STIC(pxInfoPtr)->ipdvint = STIC_INT_P_WE|STIC_INT_P;
    }

    if (depth32)
        vdac463(vram, ibuf, i);
    else
	vdac459(vram, ibuf, i);

    pxgIdle();
    cbuf[4] = ibuf & 0x1100;
    cbuf[5] = i<<3;
    cbuf[6] = (x<<19) | (y<<3);
    cbuf[7] = 0;
    cbuf[8] = 0;			/* these additional 0's */
    cbuf[9] = 0;			/* are to flush the MIPS */
    cbuf[10] = 0;			/* write buffer, before */
    cbuf[11] = 0;			/* the poll read below. */
    cbuf[12] = 0;
    cbuf[13] = 0;
    __MB();

    if (*poll != STAMP_GOOD) abort(); 
}


#define FFBVRAMALIGNMENT    (FFBVRAMBITS / 8)
#define FFBVRAMALIGNMASK    (FFBVRAMALIGNMENT - 1)
#define ALL1		0xff
/*
 * Fill VRAM with a pixel value
 */

void FillMemory(
                unsigned addr,          /* Byte address             */
                int     width,          /* width in pixels          */
                int     height,         /* height in scanlines      */
                int     scanlineWidth,  /* # bytes to next scanline */
                int     pixel,          /* 32-bit pixel value       */
                unsigned planemask      /* 32-bit planemask         */
                )
{
    extern FILE *cmdOut;
    extern int  logFlag;
    extern UsePxg;
    ULONG64     data;
    unsigned    p, a;
    unsigned    leftMask, rightMask, mask;
    int         savevramCompareFlag;
    int		saveLogFlag;
    int		m, align, w;
    unsigned int depth32;

    int pxgH = height;
    int pxgW = width;

    MakeIdle();
    if (logFlag)
      fprintf (cmdOut, "2 %x %x %x %x %x %x\n",
               addr, width, height, scanlineWidth, pixel, planemask);

    depth32 = shadowDeep.reg.deep;

    data.lo = pixel;
    data.hi = pixel;
    saveLogFlag = logFlag;
    logFlag = FALSE;
    savevramCompareFlag = vramCompareFlag;
    vramCompareFlag = FALSE;

    if (depth32) width *= 4;

    /*
     * if this is PXG/GXcopy/solid/"visible", then instead of doing itty-bitty image
     * transfers which take forever... just draw a solid rectangle.
     */
    if (UsePxg) {
	/* make sure this is a solid fill */
	if (depth32) {
	    pxgOpt = ((planemask & 0xffffff) == 0xffffff);
	} else {
	    int b0 = (pixel & 0xff);
	    int b1 = (pixel & 0xff00) >> 8;
	    int b2 = (pixel & 0xff0000) >> 16;
	    int b3 = (pixel & 0xff000000) >> 24;
	    if (b0 == b1 && b0 == b2 && b0 == b3) {
		pxgOpt = ((planemask & 0xffffffff) == 0xffffffff);
	    }
	}
    }

    do {
	/* Recompute alignment each time in case scanlineWidth mod 8 = 4 */
	align = addr & FFBVRAMALIGNMASK;
	a = addr - align;
	w = width + align;
	leftMask = (ALL1 << align) & ALL1;
	rightMask = ALL1 >> ((-w) & FFBVRAMALIGNMASK);
    
	if (w <= FFBVRAMALIGNMENT) {
	    mask = leftMask & rightMask;
	    do_rams(a/FFBVRAMALIGNMENT, data, planemask, mask, 0, depth32);
	} else {
	    p = a/FFBVRAMALIGNMENT;
	    do_rams(p, data, planemask, leftMask, 0, depth32);
	    for (m = w - 2*FFBVRAMALIGNMENT; m > 0; m -= FFBVRAMALIGNMENT) {
		p += 1;
		do_rams(p, data, planemask, ALL1, 0, depth32);
	    }
	    do_rams(p+1, data, planemask, rightMask, 0, depth32);
	}
	addr += scanlineWidth;
	height--;
    } while (height > 0);
    logFlag = saveLogFlag;
    vramCompareFlag = savevramCompareFlag;

    if (pxgOpt) {
	int lw = (pxgH << 2) -1;
	int y = (pxgY << 3) + lw;
	pxgOpt = ibuf = 0;
	pxgIdle();
	cbuf[0] = 1|(1<<4)|(2<<10);	/* line */
	cbuf[1] = 0x1ffffff;
	cbuf[2] = 0;
	cbuf[3] = 1|(0x20<<12);		/* GXcopy */
	cbuf[4] = pxgFG;		/* fg */
	cbuf[5] = (pxgX << 19) | y;
	cbuf[6] = ((((pxgX+pxgW) << 3) -1) << 16) | y;
	cbuf[7] = lw;
	cbuf[8] = 0;
	cbuf[9] = 0;
	cbuf[10] = 0;
	cbuf[11] = 0;
	cbuf[12] = 0;
	cbuf[13] = 0;
	__MB();
	if (*poll != STAMP_GOOD) abort();
	pxgIdle();
    }
}


pxgIdle()
{
    int k, j;

    for ( k=0; k<1000; k++ ) {
	if (PQO_STIC(pxInfoPtr)->ipdvint & STIC_INT_P)
	    break;
	for ( j=0; j<10000; j++ ) {}
    }
    PQO_STIC(pxInfoPtr)->ipdvint = STIC_INT_P_WE;
}
#endif /*SOFTWARE_MODEL*/

/*
 * HISTORY
 */
