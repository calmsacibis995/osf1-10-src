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
 * $XConsortium: skyHdwr.c,v 1.4 91/09/13 14:28:20 rws Exp $
 *
 * Copyright IBM Corporation 1987,1988,1989,1990,1991
 *
 * All Rights Reserved
 *
 * License to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of IBM not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS, AND 
 * NONINFRINGEMENT OF THIRD PARTY RIGHTS, IN NO EVENT SHALL
 * IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
*/

/*
 * skyHdwr.c - initialize hardware registers
 */

#include <sys/types.h>
#include <sys/hft.h>
#include <sys/entdisp.h>

#include "X.h"
#include "screenint.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "cursorstr.h"
#include "ibmScreen.h"
#include "ibmTrace.h"

#include "skyHdwr.h"
#include "skyReg.h"

void SkywayFillSolid();
static void SkywaySetCoprocessorInfo();
volatile struct SkyCopRegs
{
        unsigned int  a;
        unsigned int  b;
	char          c;      
	unsigned char d;    
	short         e;   
	short         f;   
        unsigned char g;
        unsigned char h;
        short         pix_index; 
        unsigned char i;
        unsigned char j;   
	char          *pixmap_base;
	ulong         k;    
};

int skyHdwrInit(index)
int index ;
{

   TRACE(("skyHdwrInit\n"));

   skywayWaitFifo2(index);
   SKYWAY_CCC_REG(index) = Color_Cmp_Fal;
   SKYWAY_MEM_REG(index) = 0x0b ;   /* Motorola order, 8 bits/pixel */
   SkywaySetCoprocessorInfo(index);

   SkywayFillSolid(0,0x3,0xff,0,0,1280,1024,index);

   /* Set the io register pointers					*/
   SKYWAY_SINDEX_REG(index) = 0x5600;/* disable the cursor in the DAC   */
   SKYWAY_SINDEX_REG(index) = 0x6c04;
   SKYWAY_SINDEX_REG(index) = 0x5061; /* set CRTC to prepare for reset  */
   SKYWAY_SINDEX_REG(index) = 0x5060; /* set CRTC to reset              */

   SKYWAY_INT_REG(index) = 0x00 ;
   SKYWAY_INS_REG(index) = 0xff ;    /* clear all interrupt status bits */
   SKYWAY_VMC_REG(index) = 0x00 ;    /* disable virtual memory interrupts */
   SKYWAY_VMS_REG(index) = 0xff ;    /* clear virtual memory interrupts */

   /* native motorola mode ; memory decoding enabled */

   SKYWAYSetMode(index,0x0c);

   SKYWAY_WINCTRL_REG(index) = 0;    /* No pc window */

   /* set the memory configuration register to 64 bit serializer width,
      256K x 4 module type, and 64 bit physical width for 64 bit wide
      serializer                                                        */

   SKYWAY_SINDEX_REG(index) = 0x009e;

   /* initialize the DAC                                                */

SKYWAY_SINDEX_REG(index) = 0x6006; /* Put '01000011'b into the Brooktree */
SKYWAY_SINDEX_REG(index) = 0x6443; /* command register to initialize it. */
SKYWAY_SINDEX_REG(index) = 0x6004; /* Turn on all bits in the read mask. */
SKYWAY_SINDEX_REG(index) = 0x64FF;
SKYWAY_SINDEX_REG(index) = 0x6005;      /*      60  INDX REG = 05 */
SKYWAY_SINDEX_REG(index) = 0x6400;      /*      64  BLNK MSK = 00 */
SKYWAY_SINDEX_REG(index) = 0x6007;      /*      60  INDX REG = 07 */
SKYWAY_SINDEX_REG(index) = 0x6400;      /*      64  TEST REG = 00 */

   /* The following values are taken from the Addendum to the SKYWAY1
    Video Subsystem Hardware Workbook dated October 3, 1988.    However */
   /* some values are taken from swstew.c, and don't match the addendum */

SKYWAY_SINDEX_REG(index) = 0x5402;/* Clock Frequency must be set to 0x02 */
SKYWAY_SINDEX_REG(index) = 0x3600;/* turn off the sprite control register */
SKYWAY_SINDEX_REG(index) = 0x6400;/* use Brooktree Palette DAC control  */
	                        /* register to turn off palette mask    */
SKYWAY_SINDEX_REG(index) = 0x5103;/* 8 bits/pixel, x1 scaling factors   */
/* pass two start */
SKYWAY_SINDEX_REG(index) = 0x0102;/* Set 0.25 clock increment on        */
SKYWAY_SINDEX_REG(index) = 0x16db;
SKYWAY_SINDEX_REG(index) = 0x18ac;
SKYWAY_SINDEX_REG(index) = 0x1a93;
SKYWAY_SINDEX_REG(index) = 0x1cc5;
SKYWAY_SINDEX_REG(index) = 0x1e06;
SKYWAY_SINDEX_REG(index) = 0x2a04;
/* pass two end */

SKYWAY_SINDEX_REG(index) = 0x10db;/* 1760 pixels per scan line          */
SKYWAY_SINDEX_REG(index) = 0x129f;/* 1280 pixels in active picture area */
SKYWAY_SINDEX_REG(index) = 0x149f;/* 1280 is end of picture border area */
SKYWAY_SINDEX_REG(index) = 0x201f;/* vertical total regs set to 0x41f,  */
SKYWAY_SINDEX_REG(index) = 0x2104;/* which is 1056 lines                */
SKYWAY_SINDEX_REG(index) = 0x22ff;/* vertical display end registers are */
SKYWAY_SINDEX_REG (index)= 0x2303;/* set to 0x3ff, which is 1024 lines  */
SKYWAY_SINDEX_REG(index) = 0x241f;/* vertical blanking start registers  */
SKYWAY_SINDEX_REG(index) = 0x2504;/* are set to 0x041f                  */
SKYWAY_SINDEX_REG(index) = 0x26ff;/* vertical blanking end registers are */
SKYWAY_SINDEX_REG(index) = 0x2703;/* set to 0x03ff                      */
SKYWAY_SINDEX_REG(index) = 0x2801;/* vertical sync pulse start registers*/
SKYWAY_SINDEX_REG(index) = 0x2904;/* are set to 0x0401                  */
SKYWAY_SINDEX_REG(index) = 0x4000;/* set the Start Address registers to */
SKYWAY_SINDEX_REG(index) = 0x4100;/* define the start address of the    */
SKYWAY_SINDEX_REG(index) = 0x4200;/* active picture to address 0        */
SKYWAY_SINDEX_REG(index) = 0x43a0;/* buffer pitch registers set to 1280 */
SKYWAY_SINDEX_REG(index) = 0x4400;/* pixels per scan line               */
SKYWAY_SINDEX_REG(index) = 0x64ff;/* turn on palette mask               */
SKYWAY_SINDEX_REG(index) = 0x2cff;/* vertical line compare lo           */
SKYWAY_SINDEX_REG(index) = 0x2d07;/* vertical line compare hi           */
SKYWAY_SINDEX_REG(index) = 0x5063;/* set Display Mode 1 register to:    */
	                        /*      Normal operation                */
	                        /*      No clock error                  */
	                        /*      Non interlaced                  */
	                        /*      Video Feature Interface disabled*/
	                        /*      Composite syncs enabled         */
	                        /*      + Vertical, - Horizontal        */
	                        /*         sync polarity                */


   return( 0 );

}

static struct { unsigned short r,g,b;} installed[256];

skySetColor(number,red,green,blue,index)
    register unsigned   number, red, green, blue;
    int index ;
{

    TRACE(("SkywaySetColor(%d,%d,%d,%d,%d)\n",number,red,green,blue,index));

    installed[number].r = red;
    installed[number].g = green;
    installed[number].b = blue;

    /* I don't know what this does, but it  */
    /* prevents blinking from occurring     */

    SKYWAY_SINDEX_REG(index) = (SPINDEXLO << 8 ) | 0x07 ;
    /* turn off mask */
    SKYWAY_SINDEX_REG(index) = (PALETTEMASK << 8 ) | 0x00 ;

    SKYWAYSetColorIndex(index,number) ;
    SKYWAYSetRGBColor(index,red,green,blue) ;

   return;

}

void
SkywayTileRect(pTile, alu, pm, x, y, w, h, xSrc, ySrc )
    PixmapPtr pTile;
    int alu ;
    unsigned long int pm ;
    int x, y, w, h, xSrc, ySrc;
{

 register unsigned char	*psrc, *pSRC ;
 register volatile unsigned char *pdst, *pDST ;
 int	xoff, yoff, i,j ;
 int    index ;

 TRACE(("SkywayTileRect(0x%x,%d,0x%x,%d,%d,%d,%d,0x%x,0x%x)\n",
		pTile,alu,pm,x,y,w,h,xSrc,ySrc));

 index = pTile->drawable.pScreen->myNum ;
 skywayWaitFifo2(index);

 SKYWAYSetCCC(index,Color_Cmp_Fal);
 SKYWAYSetALU(index,alu);
 SKYWAYSetPlaneMask(index,pm & 0xffff);

 SKYWAYSetupScreenPixmap(index,PixSize8) ;  /* PixSize4 for skymono */

 SKYWAYSetPixmapDstOffset(index,x,y);
 SKYWAYSetWidth(index,w - 1) ;
 SKYWAYSetHeight(index,h - 1) ;

 /* Move data into adapter ; wait for DMA */

 skywayWaitFifo2(index);

 pSRC = (unsigned char *) pTile->devPrivate.ptr ;
 pDST = (unsigned char *) SKYWAY_TILEOFFSET[index] ; 

 TRACE((" pTile->devKind is %d \n",pTile->devKind));

 for ( i = 0 ; i < pTile->drawable.height ; i++  )
 {
	psrc = pSRC ;
	pdst = pDST ;

	for ( j =0 ; j < pTile->drawable.width ; j++)
	    *pdst++ = *psrc++ ;

	    pSRC += pTile->devKind ;
	    pDST += pTile->drawable.width ;

 }

 /* src pattern */

 xoff = (x - xSrc) % pTile->drawable.width ;
 yoff = (y - ySrc) % pTile->drawable.height ;
 TRACE(("xoff is %d yoff is %d \n",xoff,yoff));
 TRACE(("xSrc is %d ySrc is %d \n",xSrc,ySrc));

 SKYWAYSetPixmapIndex(index,PixMapA) ;
 SKYWAYSetPixmapBase(index,SC_INVBASEOFFSET) ; 
 SKYWAYSetPixmapWidth(index,pTile->drawable.width - 1) ;
 SKYWAYSetPixmapHeight(index,pTile->drawable.height - 1) ;
 SKYWAYSetPixmapFormat(index,MI1 | PixSize8) ;
 SKYWAYSetPixmapSrcOffset(index,xoff, yoff);

 SKYWAY_PO_REG(index)   =   POSrcA | POForeSrc | POStepBlt |
		     PODestC | POPatFore |
		     POMaskDis | POModeAll | POOct0 ;

 skywayWaitFifo2(index);

}

void
SkywayFillSolid( color, alu, pm, x, y, w, h,index)
     int   color ;
     unsigned int alu, pm ;
     short  x, y ;
     short  w, h ;
     int    index ;
{

 TRACE(("SkywayFillSolid: color %d, alu: %d, pm: %d xy(%d, %d) %dx%d\n",
	color, alu, pm, x, y, w, h));

 skywayWaitFifo2(index);

 SKYWAYSetCCC(index,Color_Cmp_Fal);
 SKYWAYSetALU(index,alu);
 SKYWAYSetPlaneMask(index,pm & 0xffff);
 SKYWAYSetForegroundColor(index,color);	

 /* Set the foreground field for the Pixel Operations Register to */
 /* indicate the color is coming from a register */

 SKYWAYSetupScreenPixmap(index,PixSize8) ;

 SKYWAYSetPixmapDstOffset(index,x,y);
 SKYWAYSetWidth(index,w - 1) ;
 SKYWAYSetHeight(index,h - 1) ;

 SKYWAY_PO_REG(index)   =   POForeReg | POStepBlt |
		     PODestC | POPatFore |
		     POMaskDis | POModeAll | POOct0 ;

 skywayWaitFifo2(index);

} 

skywaySetupMask(xoff,yoff,width,height,pmask,index)
int xoff,yoff,width,height,pmask ;
int index ;
{

	SKYWAY_PMI_REG(index) = PixMapD ;
        SKYWAY_PMB_REG(index) = SKYWAY_MASKMAP_START ;
        SKYWAY_PMW_REG(index) = width -  1 ;
        SKYWAY_PMH_REG(index) = height - 1 ;
        SKYWAY_PMF_REG(index) = MI1 | PixSize1 ;
  	SKYWAY_DSTX_REG(index) = xoff ;
  	SKYWAY_DSTY_REG(index) = yoff ;
	SKYWAYSetPlaneMask(index,pmask & 0xffff);

  	SKYWAY_PO_REG(index) |= POMaskEn ;   /* Dangerous */

}

void
SkywayBitBlt(alu,pScreen,pmask,sx,sy,dx,dy,w,h)
     int alu,pmask ;
     ScreenPtr pScreen ;
     int sx,sy,dx,dy,w,h;	
{
 short over_y, over_x;
 unsigned int octant;
 int index ;

TRACE(("SkywayBitBltXXXXXXX(0x%x,0x%x,%d,%d,%d,%d,%d,%d)\n",
	alu,pmask,sx,sy,dx,dy,w,h));

index = pScreen->myNum ;

  /* BLT overlap codes */

#define D_NOT_OVER 0            /* no overlap */
#define D_TOP_EDGE_OVER 1       /* source overlaps top of destination */
#define D_BOTTOM_EDGE_OVER 2    /* source overlaps bottom of destination */
#define D_LEFT_EDGE_OVER 1      /* source overlaps left of destination */
#define D_RIGHT_EDGE_OVER 2     /* source overlaps right of destination */


   /* Set up the source & destination offsets. */

    /* First determine whether the source and destination overlap. */
    over_x = over_y = D_NOT_OVER;    /* initialize to no-overlap code */

    /* Check the y values. */
    if (dy >= sy) {	   /* maybe top of d is in s */
	if (dy < (sy + h))    /* top of d is in s */
	    over_y = D_TOP_EDGE_OVER;
    } else {	    /* maybe bottom of d is in s */
	if (sy < (dy + h))    /* bottom of d is in s */
	    over_y = D_BOTTOM_EDGE_OVER;
    }

    /* Check the x values. */
    if (dx >= sx) {	   /* maybe left of d is in s */
	if (dx < (sx + w))    /* left of d is in s */
	    over_x = D_LEFT_EDGE_OVER;
    } else {	    /* maybe right of d is in s */
	if (sx < (dx + w))    /* right of d is in s */
	    over_x = D_RIGHT_EDGE_OVER;
    }

    octant = POOct0;	/* in case no corners overlap */
    if (over_y == D_TOP_EDGE_OVER) {
	if (over_x == D_LEFT_EDGE_OVER) {
	    octant = POOct6;	/* upper left of d is in s */

	    /* Point to the lower-right corner */
	    sx += (w - 1);
	    sy += (h - 1);
	    dx += (w - 1);
	    dy += (h - 1);
	} else if (over_x == D_RIGHT_EDGE_OVER) {
	    octant = POOct2;	/* upper right of d is in s */

	    /* Point to the bottom left corner */
	    sy += (h - 1);
	    dy += (h - 1);
	}
    } else if (over_y == D_BOTTOM_EDGE_OVER) {
	if (over_x == D_LEFT_EDGE_OVER) {
	    octant = POOct4;	/* lower left of d is in s */

	    /* Point to the upper right corner */
	    sx += (w - 1);
	    dx += (w - 1);
	} else if (over_x == D_RIGHT_EDGE_OVER) {
	    octant = POOct0;	/* lower right of d is in s */

	    /* Already pointing to the upper left corner... */
	}
    }

skywayWaitFifo2(index);
SKYWAYSetupScreenPixmap(index,PixSize8) ;

SKYWAYSetPixmapSrcOffset(index,sx,sy);
SKYWAYSetPixmapDstOffset(index,dx,dy);
SKYWAYSetWidth(index,w - 1);
SKYWAYSetHeight(index,h - 1);

/* Set up the plane mask and alu. */

SKYWAYSetPlaneMask(index,pmask & 0xffff);
SKYWAYSetALU(index,alu);
SKYWAYSetCCC(index,Color_Cmp_Fal); /* Disable */

/* Set up the pixel operations register and do the actual BLT... */

SKYWAY_PO_REG(index) = POForeSrc | POBackSrc | POStepBlt | POSrcC | PODestC |
		POPatFore | POMaskDis | POModeAll | octant;

skywayWaitFifo2(index);

}

static void
SkywaySetCoprocessorInfo(index)
int index;
{
	volatile struct SkyCopRegs *cop;

	cop = (struct SkyCopRegs *) COPREG[index];
	cop->pix_index	= PixMapC;
	cop->pixmap_base= SKYWAY_VRAM_START[index];
}
