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
static char *rcsid = "@(#)$RCSfile: wgaprocs.c,v $ $Revision: 1.1.4.6 $ (DEC) $Date: 1993/11/22 17:35:52 $";
#endif

/*
 *	This module has most of the routines that are used
 *	to read/write the Compaq Qvision.
 *
 *	Written: 1-May-1993, Henry R. Tumblin
 */

#include "X.h"
#include "Xproto.h"
#include "input.h"
#include "scrnintstr.h"
#include "cursorstr.h"
#include "miscstruct.h"

#include "vga.h"
#include "vgaprocs.h"
#include "wga.h"
#include "wgaprocs.h"
#include "wgaio.h"
#include <stdio.h>
#include <c_asm.h>

#define NO_COLOR -1
#define NO_ROP -1

typedef unsigned char * BITMAP_UNIT_PTR;

extern int vgaScreenPrivateIndex;
extern int vgaScreenActive;

/*
 *	Static function prototypes
 */

static outblk(unsigned char *pSrc, int wSrc, int hSrc, int widthSrc, int mode, 
	      int xSrc, int ySrc);

/*
 *  This is a precomputed array of bit reversed bytes.  So reversing the
 *  bits can be reduced to a single table lookup instead of two table
 *  lookups, two shifts and an OR - or worse yet, the 8 bit loop shift and
 *  test from the rb() function.  It probably should be moved to a C
 *  module and made a extern array.
 *
 */
const unsigned char RBITS[] = {
0x00,0x80,0x40,0xc0,0x20,0xa0,0x60,0xe0,0x10,0x90,0x50,0xd0,0x30,0xb0,0x70,0xf0,
0x08,0x88,0x48,0xc8,0x28,0xa8,0x68,0xe8,0x18,0x98,0x58,0xd8,0x38,0xb8,0x78,0xf8,
0x04,0x84,0x44,0xc4,0x24,0xa4,0x64,0xe4,0x14,0x94,0x54,0xd4,0x34,0xb4,0x74,0xf4,
0x0c,0x8c,0x4c,0xcc,0x2c,0xac,0x6c,0xec,0x1c,0x9c,0x5c,0xdc,0x3c,0xbc,0x7c,0xfc,
0x02,0x82,0x42,0xc2,0x22,0xa2,0x62,0xe2,0x12,0x92,0x52,0xd2,0x32,0xb2,0x72,0xf2,
0x0a,0x8a,0x4a,0xca,0x2a,0xaa,0x6a,0xea,0x1a,0x9a,0x5a,0xda,0x3a,0xba,0x7a,0xfa,
0x06,0x86,0x46,0xc6,0x26,0xa6,0x66,0xe6,0x16,0x96,0x56,0xd6,0x36,0xb6,0x76,0xf6,
0x0e,0x8e,0x4e,0xce,0x2e,0xae,0x6e,0xee,0x1e,0x9e,0x5e,0xde,0x3e,0xbe,0x7e,0xfe,
0x01,0x81,0x41,0xc1,0x21,0xa1,0x61,0xe1,0x11,0x91,0x51,0xd1,0x31,0xb1,0x71,0xf1,
0x09,0x89,0x49,0xc9,0x29,0xa9,0x69,0xe9,0x19,0x99,0x59,0xd9,0x39,0xb9,0x79,0xf9,
0x05,0x85,0x45,0xc5,0x25,0xa5,0x65,0xe5,0x15,0x95,0x55,0xd5,0x35,0xb5,0x75,0xf5,
0x0d,0x8d,0x4d,0xcd,0x2d,0xad,0x6d,0xed,0x1d,0x9d,0x5d,0xdd,0x3d,0xbd,0x7d,0xfd,
0x03,0x83,0x43,0xc3,0x23,0xa3,0x63,0xe3,0x13,0x93,0x53,0xd3,0x33,0xb3,0x73,0xf3,
0x0b,0x8b,0x4b,0xcb,0x2b,0xab,0x6b,0xeb,0x1b,0x9b,0x5b,0xdb,0x3b,0xbb,0x7b,0xfb,
0x07,0x87,0x47,0xc7,0x27,0xa7,0x67,0xe7,0x17,0x97,0x57,0xd7,0x37,0xb7,0x77,0xf7,
0x0f,0x8f,0x4f,0xcf,0x2f,0xaf,0x6f,0xef,0x1f,0x9f,0x5f,0xdf,0x3f,0xbf,0x7f,0xff
};

/*
 *	Array of off-screen addresses used in outblk function and pointer
 *	to frame buffer.
 */

static unsigned int *bP[4];
static unsigned int *fP;

#define OUTBLK(_pSrc,_wSrc,_hSrc,_widthSrc,_mode,_xSrc,_ySrc) \
{					\
  int i,j;				\
  union {				\
    unsigned char ub[4];		\
    unsigned int ul;			\
  } _uu;				\
  if(_xSrc || _ySrc || (_wSrc > 8) || _mode) {	\
    outblk(_pSrc,_wSrc,_hSrc,_widthSrc,_mode,_xSrc,_ySrc); \
  } else {				\
    j = 0; i = 0; _uu.ul = 0;		\
    for (; _hSrc; _hSrc--) {		\
      _uu.ub[i++] = RBITS[*_pSrc];	\
      if (i == sizeof(_uu)){		\
	*bP[j++] = _uu.ul;		\
	if (j == 4)			\
	  j = 0;			\
	i = 0; _uu.ul = 0;		\
      }					\
      _pSrc += _widthSrc;		\
    }					\
    if (i) *bP[j] = _uu.ul;		\
  }					\
}


/***********************************************************************/

/*
 *  Setup registers for bitblt     
 */

void setBlt(pScreen, dsrc, expnd, fg, bg, rop)
  ScreenPtr pScreen;
  int dsrc, expnd;
  unsigned int fg, bg;
  int rop;
{
  int cntr=0;
  vgaScreenPrivPtr vgaPriv = (vgaScreenPrivPtr) (pScreen->devPrivates[vgaScreenPrivateIndex].ptr);
  wgaShadowRegPtr pShadow = (wgaShadowRegPtr) (vgaPriv->avail);

  if (mergexlate[rop] == SOURCE_DATA){
    dsrc |= (ROPSELECT_NO_ROPS | PIXELMASK_ONLY );
  }
  else if (dsrc & ROPSELECT_ALL_EXCPT_PRIMARY){
    dsrc |= (PIXELMASK_ONLY );
  }
  else if (dsrc & ROPSELECT_PRIMARY_ONLY){
    dsrc |= (PIXELMASK_ONLY );
  }
  else {
    dsrc |= (ROPSELECT_ALL | PIXELMASK_ONLY );
  }
  if (rop != NO_ROP)
    WGAUpdateROP_Az(pShadow, mergexlate[rop]);

  if (inp (CTRL_REG_1) != (expnd | BITS_PER_PIX_8 | ENAB_TRITON_MODE) )
    WGAUpdateCtrlReg1z (pShadow, expnd | BITS_PER_PIX_8 | ENAB_TRITON_MODE);

  WGAUpdateDataPathCtrlz(pShadow, dsrc);

  if (expnd & (EXPAND_TO_FG | EXPAND_TO_BG)){
    if (fg >= 0)
      WGAUpdateFGColor(pShadow, fg);
    if (bg >= 0)
      WGAUpdateBGColor(pShadow, bg);
  }
}



/*
 * Reset the BLT engine after a color-expand operation
 */
void resetBlt()
{

  outpw(GC_INDEX,BLT_CONFIG+(RESET_BLT<<8));
  outpw(GC_INDEX,BLT_CONFIG+(BLT_ENABLE<<8));
}



/*
 *	Perform a BLT operation on the screen
 */
void
pwgaBlit(pScreen, xSrc, ySrc, wSrc, hSrc, xDst, yDst, alu, planemask)
    ScreenPtr pScreen;
    int xSrc, ySrc, wSrc, hSrc;
    int xDst, yDst;
    unsigned int alu;
    unsigned int planemask;
{
  int overlap=0,i;
  vgaScreenPrivPtr vgaPriv = (vgaScreenPrivPtr) (pScreen->devPrivates[vgaScreenPrivateIndex].ptr);
  wgaShadowRegPtr pShadow = (wgaShadowRegPtr) (vgaPriv->avail);

#ifdef TRACE
  fprintf (stderr,"pwgaBlit: x:%d, y:%d, w:%d, h:%d, dx:%d, dy:%d, alu:%x, pm:%x\n",
	   xSrc,ySrc,wSrc,hSrc,xDst,yDst,alu,planemask);
#endif

  if (overlap = (yDst > ySrc) || ((yDst == ySrc) && (xSrc < xDst))){
    xDst += wSrc -1;
    yDst += hSrc -1;
    xSrc += wSrc -1;
    ySrc += hSrc -1;
  }

  GLOBALWAIT(pScreen->myNum, "pwgaBlit");
  setBlt (pScreen, SRC_IS_SCRN_LATCHES | ROPSELECT_ALL,
          PACKED_PIXEL_VIEW, NO_COLOR, NO_COLOR, alu);
  WGAUpdatePlaneMask(pShadow, planemask);
  outpiz (X0_SRC_ADDR_LO, xSrc, ySrc);
  outpiz (DEST_ADDR_LO, xDst, yDst);
  outpwz (BITMAP_WIDTH, wSrc);
  outpwz (BITMAP_HEIGHT, hSrc);

  if (overlap) {
    outpz (BLT_CMD_0,(BACKWARD | NO_BYTE_SWAP | NO_WRAP | START_BLT));
  } else {
    outpz (BLT_CMD_0,FORWARD | NO_BYTE_SWAP | NO_WRAP | START_BLT);
  }
}



/*
 *	Replicate scans on the screen. A special form of blt
 */
void
pwgaReplicateScans(pScreen, xSrc, ySrc, wSrc, hSrc, hDst, planemask)
    ScreenPtr pScreen;
    int xSrc, ySrc, wSrc, hSrc, hDst;
    unsigned int planemask;
{
  vgaScreenPrivPtr vgaPriv = (vgaScreenPrivPtr) (pScreen->devPrivates[vgaScreenPrivateIndex].ptr);
  wgaShadowRegPtr pShadow = (wgaShadowRegPtr) (vgaPriv->avail);

#ifdef TRACE
  fprintf (stderr,"pwgaReplicateScans: x:%d, y:%d, w:%d, h:%d, hd:%d, pm:%x\n",
	   xSrc,ySrc,wSrc,hSrc,hSrc,planemask);
#endif
  GLOBALWAIT(pScreen->myNum, "pwgaReplicateScans");

  WGAUpdatePlaneMask(pShadow, planemask&0xff);
  WGAUpdatePixelMask(pShadow, 0xff);
  WGAUpdateCtrlReg1z(pShadow, PACKED_PIXEL_VIEW | BITS_PER_PIX_8 | ENAB_TRITON_MODE);
  WGAUpdateDataPathCtrl(pShadow, (ROPSELECT_NO_ROPS | PLANARMASK_NONE_0XFF |
			 PIXELMASK_ONLY | SRC_IS_SCRN_LATCHES));

  outpiz (X0_SRC_ADDR_LO, xSrc, ySrc);

  outpiz (DEST_ADDR_LO, xSrc, (ySrc + hSrc));
  outpwz (BITMAP_WIDTH,wSrc);
  outpwz (BITMAP_HEIGHT,hDst);

  outpz (BLT_CMD_0,FORWARD | NO_BYTE_SWAP | WRAP | START_BLT);

}



/*
 *	Fill specified area with a solid color.
 *	Since this uses packed pixel view, no reset is necessary.
 */
void
pwgaFillSolid(pScreen, fg, alu, planemask, xDst, yDst, wDst, hDst, first)
    ScreenPtr pScreen;
    unsigned int fg;
    unsigned int alu;
    unsigned int planemask;
    int xDst, yDst, wDst, hDst, first;
{
  static union {
    unsigned char ub[4];
    unsigned int ul;
  } uu;
  vgaScreenPrivPtr vgaPriv;
  wgaShadowRegPtr pShadow;

#ifdef TRACE
  fprintf (stderr,"pwgaFillSolid: fg:%x, alu:%x, pm:%x, xd:%d, yd:%d, wd:%d, hd:%d\n",
	   fg,alu,planemask,xDst,yDst,wDst,hDst);
#endif
  
  if (first) {
    int i;

    for (i = 3; i >= 0; i--)
      uu.ub[i] = fg & 0xff;
    vgaPriv = (vgaScreenPrivPtr) (pScreen->devPrivates[vgaScreenPrivateIndex].ptr);
    pShadow = (wgaShadowRegPtr) (vgaPriv->avail);
    GLOBALWAIT(pScreen->myNum, "pwgaFillSolid");
    setBlt ( pScreen, SRC_IS_PATTERN_REGS|ROPSELECT_ALL,
             PACKED_PIXEL_VIEW,
             NO_COLOR, NO_COLOR, alu);
    WGAUpdatePlaneMask(pShadow, planemask);

    outpiz (X0_SRC_ADDR_LO, 0x00, 0x00);

    /* Set the pattern registers            */

    outpwz (PREG_4, uu.ul);
    outpwz (PREG_6, uu.ul);
    outpw  (PREG_0, uu.ul);
    outpwz (PREG_2, uu.ul);
  }

  if (wDst <= 20 && hDst == 1) {
    unsigned int *tP;
    unsigned int width;

    tP = (unsigned int *)((unsigned long) fP + ((1024 * yDst + xDst) << 7));
    width = 4 - (xDst & 3);
    if (width > wDst)
      width = wDst;
    tP = (unsigned int *)((unsigned long) tP + ((width - 1) << 5));
    *tP = uu.ul;
    wDst -= width;
    if (!wDst)
      return;
    tP = (unsigned int *)((unsigned long) tP + (width << 7));
    tP = (unsigned int *)((unsigned long) tP | 0x60);
    for (; wDst >= 4; wDst -= 4) {
      *tP = uu.ul;
      tP = (unsigned int *)((unsigned long) tP + (0x4 << 7));
    }
    if (wDst) {
      tP = (unsigned int *)(((unsigned long) tP & ~0x60) | ((wDst - 1) << 5));
      *tP = uu.ul;
    }
    return;
  }

  if (!first) {
    asm("mb");
    WGABUFFEREDWAIT("pwgaFillSolid");
  }
  outpiz (DEST_ADDR_LO, xDst, yDst);
  outpwz (BITMAP_WIDTH, wDst);
  outpwz (BITMAP_HEIGHT, hDst);

  /* Start the engine */

  outpz (BLT_CMD_0, FORWARD | NO_BYTE_SWAP | WRAP | START_BLT);

}



/*
 *	Draw a color image in packed pixel mode
 */
void pwgaDrawColorImage(ScreenPtr pScreen, int xDst, int yDst, int wDst, 
    int hDst, unsigned char *pSrc, int widthSrc, unsigned int alu,
    unsigned int pm, int first)
{

  unsigned int i,j,k,tmp,offs, offx, ob, bitnbr;
  int savedCtrl1,pn,mask,bitpos,plane;
  vgaScreenPrivPtr vgaPriv;
  wgaShadowRegPtr pShadow;

#ifdef TRACE
  fprintf (stderr,"pwgaDrawColorImage: xd:%d, yd:%d, wd:%d, hd:%d, ws:%d, alu:%x, pm:%x\n",
	   xDst,yDst,wDst,hDst,widthSrc,alu,pm);
#endif
/*
 * 	Set up to do the writes
 */
    
  if (first) {
    vgaPriv = (vgaScreenPrivPtr) (pScreen->devPrivates[vgaScreenPrivateIndex].ptr);
    pShadow = (wgaShadowRegPtr) (vgaPriv->avail);
    GLOBALWAIT(pScreen->myNum, "pwgaDrawColorImage");
    setBlt ( pScreen, SRC_IS_CPU_DATA |ROPSELECT_ALL,
	     PACKED_PIXEL_VIEW,
             NO_COLOR, NO_COLOR, alu);
    WGAUpdatePlaneMask(pShadow, (pm&0xff));
    outpiz (X0_SRC_ADDR_LO, 0x00, 0x00);
  } else {
    WGABUFFEREDWAIT("pwgaDrawColorImage");
  }

  outpiz (DEST_ADDR_LO, xDst, yDst);
  outpwz (BITMAP_WIDTH, wDst);
  outpwz (BITMAP_HEIGHT, hDst);
  outp (BLT_CMD_0, FORWARD | NO_BYTE_SWAP | WRAP | START_BLT);

  OUTBLK (pSrc, wDst, hDst, widthSrc, 2, 0, 0);

}



/*
 *	Read pack a packed pixel image from the frame buffer
 */
void
pwgaReadColorImage(pScreen, xSrc, ySrc, wSrc, hSrc, planemask, pDst, widthDst)
    ScreenPtr pScreen;
    int xSrc, ySrc, wSrc, hSrc;
    unsigned long planemask;
    BITMAP_UNIT_PTR pDst;
    int widthDst;
{

  unsigned int tmp;
  unsigned int offs;
  int row,col,j;
  unsigned int * dwP;
  vgaScreenPrivPtr vgaPriv = (vgaScreenPrivPtr) (pScreen->devPrivates[vgaScreenPrivateIndex].ptr);
  wgaShadowRegPtr pShadow = (wgaShadowRegPtr) (vgaPriv->avail);

#ifdef TRACE

  fprintf (stderr,"pwgaReadColorImage: x:%x, y:%x, w:%x, h:%x, pd:%x, wd:%x\n",
	   xSrc, ySrc, wSrc, hSrc, pDst, widthDst);
#endif

  GLOBALWAIT(pScreen->myNum, "pwgaReadColorImage");
  /*
   * Read from the frame in packed pixel mode
   */
  WGAUpdateCtrlReg1(pShadow, PACKED_PIXEL_VIEW + BITS_PER_PIX_8 + ENAB_TRITON_MODE);
  asm("mb");			/* Make sure it happens before reading */

  for (j=0,row=0; row<hSrc; row++){
    offs = (xSrc+((ySrc+row)*1024));
    for (col=0; col<wSrc; col++){
      dwP = (unsigned int *)((unsigned long) fP + (unsigned long)((offs+col) << 7));
      tmp = *dwP;
      pDst[j++] = ((tmp) >> ((offs+col)&0x03)*8) & planemask;
    }
    pDst += widthDst;
    j=0;
  }
}



/*
 *	Perform opaque stippling at the specified position for the
 *	pattern specified in psrcBase
 */
void
pwgaOpaqueStipple(pScreen, xSrc, ySrc, wSrc, hSrc, psrcBase, widthSrc,
                  xDst, yDst, first, alu, planemask, fg, bg)
    ScreenPtr pScreen;
    int xSrc, ySrc, wSrc, hSrc;
    unsigned char *psrcBase;
    int widthSrc;
    int xDst, yDst, first;
    unsigned int alu;
    unsigned int planemask, fg, bg;
{
  vgaScreenPrivPtr vgaPriv;
  wgaShadowRegPtr pShadow;

#ifdef TRACE
  fprintf(stderr,"pwgaOpaqueStipple: x:%x,y:%x,w:%x,h:%x\n",
	  xSrc,ySrc,wSrc,hSrc);
  fprintf(stderr,"                 : pb:%x,ws:%x,xd:%d,yd:%x\n",
	  psrcBase,widthSrc,xDst,yDst);
  fprintf(stderr,"                 : alu:%x, pm:%x, fg:%x, bg:%x\n",
	  alu,planemask,fg,bg);
#endif

  if (first) {
    vgaPriv = (vgaScreenPrivPtr) (pScreen->devPrivates[vgaScreenPrivateIndex].ptr);
    pShadow = (wgaShadowRegPtr) (vgaPriv->avail);
    GLOBALWAIT(pScreen->myNum, "pwgaOpaqueStipple");
    setBlt (pScreen, SRC_IS_CPU_DATA|ROPSELECT_ALL,
             EXPAND_TO_BG,
             bg, fg, alu);
    WGAUpdatePlaneMask(pShadow, (planemask&0xff));

    outpiz (X0_SRC_ADDR_LO, 0x00, 0x00);
  } else {
    asm("mb");
    WGABUFFEREDWAIT("pwgaOpaqueStipple");
  }
  outpiz (DEST_ADDR_LO, xDst, yDst);
  outpwz (BITMAP_WIDTH, wSrc);
  outpwz (BITMAP_HEIGHT, hSrc);

  outp (BLT_CMD_0, FORWARD | NO_BYTE_SWAP | WRAP | START_BLT);

  OUTBLK (psrcBase, wSrc, hSrc, widthSrc, 0, xSrc, ySrc);
  asm("mb");			/* Make sure it finishes before going on */

}



/*
 *	Perform stipple operation
 */
void
pwgaStipple(pScreen, xSrc, ySrc, wSrc, hSrc, psrcBase, widthSrc,
            xDst, yDst, first, alu, planemask, fg)
    ScreenPtr pScreen;
    int xSrc, ySrc, wSrc, hSrc;
    unsigned char *psrcBase;
    int widthSrc;
    int xDst, yDst, first;
    unsigned int alu;
    unsigned int planemask, fg;
{
  vgaScreenPrivPtr vgaPriv;
  wgaShadowRegPtr pShadow;

#ifdef TRACE

  fprintf(stderr,"pwgaStipple: x:%d,y:%d,w:%d,h:%d\n",
	  xSrc,ySrc,wSrc,hSrc);
  fprintf(stderr,"           : pb:%x,ws:%d,xd:%d,yd:%d\n",
	  psrcBase,widthSrc,xDst,yDst);
  fprintf(stderr,"           : alu:%x, pm:%x, fg:%x\n",
	  alu,planemask,fg);
#endif

    if (first) {
      vgaPriv = (vgaScreenPrivPtr) (pScreen->devPrivates[vgaScreenPrivateIndex].ptr);
      pShadow = (wgaShadowRegPtr) (vgaPriv->avail);
      GLOBALWAIT(pScreen->myNum, "pwgaStipple");

      setBlt (pScreen, SRC_IS_CPU_DATA | ROPSELECT_PRIMARY_ONLY | PIXELMASK_AND_SRC_DATA, 
	       EXPAND_TO_FG,
               fg, NO_COLOR, alu);
      WGAUpdatePlaneMask(pShadow, planemask);

      outpiz (X0_SRC_ADDR_LO, 0x00, 0x00);
    } else {
      asm("mb");
      WGABUFFEREDWAIT("pwgaStipple");
    }

    outpiz (DEST_ADDR_LO, xDst, yDst);
    outpwz (BITMAP_WIDTH, wSrc);
    outpwz (BITMAP_HEIGHT, hSrc);

    outp (BLT_CMD_0, FORWARD | NO_BYTE_SWAP | WRAP | START_BLT);

    OUTBLK (psrcBase, wSrc, hSrc, widthSrc, 0, xSrc, ySrc);
    asm("mb");			/* Make sure it finishes before going on */

}



/*
 *	Outblk_init = Setup array of off-screen addresses to be used
 *	in outblk transfer functions.  This keeps the screen from getting
 *	trashed while transferring data to the blt engine.
 */

void outblk_init(void)
{
  int j;

  fP = (unsigned int *) GetFrameAddress(0);
  for (j = 3; j >= 0; j--)
    bP[j] = (unsigned int *) ((unsigned long) fP +	/* Get base address */
	         (((1024/8)*768+(j*8))<<7) + 		/* Off-screen offset */
		 0x60);					/* 32 bit write mode */
}


/*
 *	outblk - heart of cpu-qvision transfers. Make sure you
 *	understand what this does before you muck around with it
 *	because a lot of stuff will break from stupid mistakes made
 *	herein. (I know from experience!)
 */
static outblk (pSrc, wSrc, hSrc, widthSrc, mode, xSrc, ySrc)
unsigned char * pSrc;
int wSrc, hSrc, widthSrc, mode, xSrc, ySrc;
{
  int i,j,k;
  int col;
  union {
    unsigned char ub[4];
    unsigned int ul;
  } uu;
  unsigned char *pSrcptr;
  unsigned char shifted[240];
  unsigned char *tempSrc = shifted;

#ifdef TRACE
  fprintf (stderr,"outblk: w:%d, h:%d, width:%d, mode:%d\n",
	   wSrc,hSrc,widthSrc,mode);
#endif
#if defined(TRACE) && TRACE > 1
  displayBitmap(pSrc,wSrc,hSrc,widthSrc);
#endif
/*
 *	Mode = 0, 8 pixels/byte of source
 *	Mode = 2, 1 pixel/byte of source(bitonal)
 */

  if (mode == 4){
    i=0;k=0;uu.ul=0;j=0;
    for (; hSrc; hSrc--){
      for (col = wSrc; col; col--){
        uu.ub[i++] = RBITS[pSrc[k++]];
	if (i == sizeof(uu)){
	  *bP[j++] = uu.ul;
	  if (j==4)
	    j=0;
	  i=0; uu.ul=0;
        }
      }
    pSrc += widthSrc;
    k=0;
    }
  return;
  }
/*
 *	Mode 2, output a single pixel for 8 plane's worth of data
 *	with each write, unfortunately, this can't be buffered. 
 */
  if (mode == 2){
    j=0; i=0; uu.ul=0;
    for (; hSrc; hSrc--) {
      pSrcptr = pSrc;
      for (col = wSrc; col; col--) {
        uu.ub[i++] = *pSrcptr++;
	if (i == sizeof(uu)){
	  *bP[j++] = uu.ul;
	  if (j==4) j=0;
	  i=0; uu.ul=0;
        }
      }
      /*
       *	Must flush at end of line
       */
      if (i){
        *bP[j++] = uu.ul;
        if(j==4) j=0;
        i = 0; uu.ul=0;
      }
      pSrc += widthSrc;
    }
  return;
  }
/*
 *	Mode 0 - Write out 8 pixels with each byte
 */
  pSrc += ySrc * widthSrc;			/* Skip first ySrc rows */
  if (xSrc) {
    int shift;

    if ((xSrc + wSrc) <= 8) {			/* All within one byte */
      j = 0; i = 0; uu.ul = 0;
      for (; hSrc; hSrc--) {
        uu.ub[i++] = RBITS[*pSrc >> xSrc];
        if (i == sizeof(uu)){
	  *bP[j++] = uu.ul;			/* To avoid using MB here */
	  if (j == 4)
	    j = 0;
	  i = 0; uu.ul = 0;
        }
        pSrc += widthSrc;
      }
      if (i)					/* Put out any trailing data */
	*bP[j] = uu.ul;
      return;
    }
    shift = xSrc & 0x7;
    if (shift) {
      int wSrcbytes, alloc_size;
      unsigned char *tempSrcptr;
      
      switch (widthSrc) {
	case sizeof(long) :
	  if (hSrc > (sizeof(shifted) / sizeof(long)))
	    tempSrc = (unsigned char *) Xalloc(hSrc * sizeof(long));
	  tempSrcptr = tempSrc;
	  for (i = hSrc; i; i--) {
	    *(unsigned long *) tempSrcptr = (*(unsigned long *) pSrc) >> shift;
	    tempSrcptr += sizeof(long);
	    pSrc += sizeof(long);
	  }
	  break;
	case sizeof(int) :
	  if (hSrc > (sizeof(shifted) / sizeof(int)))
	    tempSrc = (unsigned char *) Xalloc(hSrc * sizeof(int));
	  tempSrcptr = tempSrc;
	  for (i = hSrc; i; i--) {
	    *(unsigned int *) tempSrcptr = (*(unsigned int *) pSrc) >> shift;
	    tempSrcptr += sizeof(int);
	    pSrc += sizeof(int);
	  }
	  break;
	case sizeof(short) :
	  if (hSrc > (sizeof(shifted) / sizeof(short)))
	    tempSrc = (unsigned char *) Xalloc(hSrc * sizeof(short));
	  tempSrcptr = tempSrc;
	  for (i = hSrc; i; i--) {
	    *(unsigned short *)tempSrcptr = (*(unsigned short *) pSrc) >> shift;
	    tempSrcptr += sizeof(short);
	    pSrc += sizeof(short);
	  }
	  break;
	default :
	  wSrcbytes = (wSrc-1)/8 + 1;
	  alloc_size = hSrc * wSrcbytes;
	  if (alloc_size > sizeof(shifted))
	    tempSrc = (unsigned char *) Xalloc(hSrc * wSrcbytes);
	  tempSrcptr = tempSrc;
	  for (; xSrc >= 8; xSrc -= 8)		/* Eliminate whole-byte shift */
	    pSrc++;
	  for (i = hSrc; i; i--) {
	    pSrcptr = pSrc;
	    for (j = wSrc; j > 0; j -= 8, pSrcptr++)
	      if ((j + shift) > 8)
	        *tempSrcptr++ = (*pSrcptr>>shift) | (*(pSrcptr+1) << (8-shift));
	      else
	        *tempSrcptr++ = *pSrcptr >> shift;
	    pSrc += widthSrc;
	  }
	  widthSrc = wSrcbytes;
      }
      pSrc = tempSrc;				/* Write from tempSrc area */
      xSrc &= ~0x7;
    }
    for (; xSrc; xSrc -= 8)			/* Eliminate whole-byte shift */
      pSrc++;
  }
  if (wSrc <= 8) {				/* All within one byte */
    j = 0; i = 0; uu.ul = 0;
    for (; hSrc; hSrc--) {
      uu.ub[i++] = RBITS[*pSrc];
      if (i == sizeof(uu)){
	*bP[j++] = uu.ul;			/* To avoid using MB here */
	if (j == 4)
	  j = 0;
	i = 0; uu.ul = 0;
      }
      pSrc += widthSrc;
    }
  } else {
    wSrc = ((wSrc-1)/8+1)*8;
    j = 0; i = 0; uu.ul = 0;
    for (; hSrc; hSrc--) {
      pSrcptr = pSrc;
      for (col = wSrc; col > 0; col -= 8) {
        uu.ub[i++] = RBITS[*pSrcptr++];
        if (i == sizeof(uu)) {
	  *bP[j++] = uu.ul;			/* To avoid using MB here */
	  if (j == 4)
	    j = 0;
	  i = 0; uu.ul = 0;
        }
      }
      pSrc += widthSrc;
    }
  }
  if (i)					/* Put out any trailing data */
    *bP[j] = uu.ul;
  if (tempSrc != shifted)
    Xfree(tempSrc);
}



/*
 *	Bresenham engine -  Draw a single line 
 */

void
pwgaBresS(pScreen, fg, alu, planemask, signdx, signdy, axis, 
	  x1, y1, e, e1, e2, len)
    ScreenPtr pScreen;
    unsigned int fg;
    unsigned int alu;
    unsigned int planemask;
    int signdx, signdy, axis;
    int x1, y1, e, e1, e2, len;
{
  int i=0;
  vgaScreenPrivPtr vgaPriv = (vgaScreenPrivPtr) (pScreen->devPrivates[vgaScreenPrivateIndex].ptr);
  wgaShadowRegPtr pShadow = (wgaShadowRegPtr) (vgaPriv->avail);

#if defined(TRACE)
  fprintf(stderr,"pwgaBresS: fg:%x,alu:%x,pm:%x,sx:%d,sy:%d,axis:%d\n",
	  fg,alu,planemask,signdx,signdy,axis);
  fprintf(stderr,"             : x1:%d, y1:%d, e:%d, e1:%d, e2:%d, len:%d\n",
	  x1,y1,e,e1,e2,len);
#endif

  GLOBALWAIT(pScreen->myNum, "pwgaBresS");

  /* Sanity Check */
  if (len <= 0)
    return;

  if (axis)
    i |= Y_MAJOR;

  if (signdx < 0)
    i |= SIGN_X_NEG;

  if (signdy < 0)
    i |= SIGN_Y_NEG;

  WGAUpdateCtrlReg1(pShadow, EXPAND_TO_FG | BITS_PER_PIX_8 | ENAB_TRITON_MODE);

  WGAUpdateROP_A(pShadow, mergexlate[alu]);
  WGAUpdatePlaneMask(pShadow, planemask);
  WGAUpdateDataPathCtrl(pShadow, (SRC_IS_LINE_PATTERN | ROPSELECT_ALL |
				  PIXELMASK_ONLY | PLANARMASK_NONE_0XFF));

  WGAUpdateFGColor(pShadow, fg);

  outpw (GC_INDEX,LINE_CMD+((LINE_RESET)<<8));
  outpw (GC_INDEX,LINE_CMD+((CALC_ONLY)<<8));

  outpw (GC_INDEX,LINE_ERR_TERM+((e&0xff)<<8));
  outpw (GC_INDEX,(LINE_ERR_TERM+1)+(((e>>8)&0xff)<<8));

  outpw (GC_INDEX,K1_CONST+((e1&0xff)<<8));
  outpw (GC_INDEX,(K1_CONST+1)+(((e1>>8)&0xff)<<8));

  outpw (GC_INDEX,K2_CONST+((e2&0xff)<<8));
  outpw (GC_INDEX,(K2_CONST+1)+(((e2>>8)&0xff)<<8));

  outpw (GC_INDEX,LINE_PIX_CNT+((len-1&0xff)<<8));
  outpw (GC_INDEX,(LINE_PIX_CNT+1)+(((len-1>>8)&0xff)<<8));

  outpw (GC_INDEX,SIGN_CODES+(i<<8));

  outpi (X0_SRC_ADDR_LO, x1, y1);

  outpw (GC_INDEX,LINE_CMD+((START_LINE | KEEP_X0_Y0)<<8));

}



/* Draw a dashed line		*/

void
pwgaBresOnOff(pScreen, fg, alu, planemask, pdashIndex, pDash, numInDashList, 
	      pdashOffset, signdx, signdy, axis, x1, y1, e, e1, e2, len)
    ScreenPtr pScreen;
    unsigned int fg;
    unsigned int alu;
    unsigned int planemask;
    int *pdashIndex;
    unsigned char *pDash;
    int numInDashList;
    int *pdashOffset;
    int signdx, signdy, axis;
    int x1, y1, e, e1, e2, len;
{
  int dashIndex;
  int dashRemaining;
  unsigned int i = 0;
  unsigned int cmd;
  unsigned int pattern = 0xffffffff;
  unsigned int patt_len = 0;
  int express = FALSE;
  vgaScreenPrivPtr vgaPriv = (vgaScreenPrivPtr) (pScreen->devPrivates[vgaScreenPrivateIndex].ptr);
  wgaShadowRegPtr pShadow = (wgaShadowRegPtr) (vgaPriv->avail);

#ifdef TRACE
  fprintf(stderr,"pwgaBresOnOff: fg:%x,alu:%x,pm:%x,pdi:%x,pd:%x\n",
	  fg,alu,planemask,pdashIndex,pDash);
  fprintf(stderr,"             : nid:%x,pdo:%x,sx:%d,sy:%x,axis:%x\n",
	  numInDashList, pdashOffset, signdx, signdy, axis );
  fprintf(stderr,"             : x1:%x, y1:%x, e:%x, e1:%x, e2:%x, len:%x\n",
	  x1,y1,e,e1,e2,len);
#endif

  /* Sanity Check */
  if (len <= 0)
    return;

  if (axis)
    i |= Y_MAJOR;

  if (signdx < 0)
    i |= SIGN_X_NEG;

  if (signdy < 0)
    i |= SIGN_Y_NEG;

  GLOBALWAIT(pScreen->myNum, "pwgaBresOnOff");
  outpw (GC_INDEX,LINE_CMD+((LINE_RESET)<<8));
  outpw (GC_INDEX,LINE_CMD+((CALC_ONLY)<<8));

  for (dashIndex = numInDashList-1; dashIndex>=0 && patt_len<=32; dashIndex--)
    patt_len += pDash[dashIndex];

  express = (patt_len <= 32);

  if (express)
    for (dashIndex = numInDashList - 1; dashIndex >= 0; dashIndex--) {
      pattern >>= pDash[dashIndex];
      if (!(dashIndex & 1))
        pattern |= (1 << 32) - (1 << (32 - pDash[dashIndex]));
    }

  WGAUpdateCtrlReg1(pShadow, EXPAND_TO_FG | BITS_PER_PIX_8 | ENAB_TRITON_MODE);

  WGAUpdateROP_A(pShadow, mergexlate[alu]);
  WGAUpdatePlaneMask(pShadow, planemask);
  WGAUpdateDataPathCtrl(pShadow, (SRC_IS_LINE_PATTERN | ROPSELECT_PRIMARY_ONLY |
				  PIXELMASK_ONLY | PLANARMASK_NONE_0XFF));

  WGAUpdateFGColor(pShadow, fg);

  if (express) {
    int startPoint;
    
    dashIndex = *pdashIndex;
    startPoint = *pdashOffset;
    while (--dashIndex >= 0)
      startPoint += pDash[dashIndex];
    startPoint %= 32;
    outpw (GC_INDEX,PATTERN_POINTER+((31 - startPoint) << 8));
  } else {
    outpw (GC_INDEX,PATTERN_POINTER+(31 << 8));
  }
  
  outpw (GC_INDEX,PATTERN_END+(((32 - patt_len) % 32) << 8));
  outpi (LINE_PATTERN, pattern, 0);

  outpi (X0_SRC_ADDR_LO, x1, y1);

  outpw (GC_INDEX,LINE_ERR_TERM+((e&0xff)<<8));
  outpw (GC_INDEX,(LINE_ERR_TERM+1)+(((e>>8)&0xff)<<8));

  outpw (GC_INDEX,K1_CONST+((e1&0xff)<<8));
  outpw (GC_INDEX,(K1_CONST+1)+(((e1>>8)&0xff)<<8));

  outpw (GC_INDEX,K2_CONST+((e2&0xff)<<8));
  outpw (GC_INDEX,(K2_CONST+1)+(((e2>>8)&0xff)<<8));

  outpw (GC_INDEX,SIGN_CODES+(i<<8));

  cmd = KEEP_X0_Y0+LAST_PIXEL_NULL;
  outpw (GC_INDEX,LINE_CMD+(cmd<<8));

  cmd |= START_BIT;

  dashIndex = *pdashIndex;
  dashRemaining = pDash[dashIndex] - *pdashOffset;

  if (express) {
    outpw (GC_INDEX,LINE_PIX_CNT+((len&0xff)<<8));
    outpw (GC_INDEX,(LINE_PIX_CNT+1)+(((len>>8)&0xff)<<8));
    outpw (GC_INDEX,LINE_CMD+(cmd<<8));
    while (len >= dashRemaining) {
      len -= dashRemaining;
      if (++dashIndex == numInDashList)
        dashIndex = 0;
      dashRemaining = pDash[dashIndex];
    }
    *pdashIndex = dashIndex;
    *pdashOffset = len;
    return;
  }

  if (dashIndex & 1)
    cmd = CALC_ONLY; /* Don't draw ODD dashes */

  for (;;) {
    if (len < dashRemaining) {
      outp (GC_INDEX,LINE_CMD);
      while (inp(GC_DATA) & START_BIT)
	usleep(10) ;
      outpw (GC_INDEX,LINE_PIX_CNT+((len&0xff)<<8));
      outpw (GC_INDEX,(LINE_PIX_CNT+1)+(((len>>8)&0xff)<<8));
      outpw (GC_INDEX,LINE_CMD+(cmd<<8));
      dashRemaining -= len;
      *pdashIndex = dashIndex;
      *pdashOffset = pDash[dashIndex] - dashRemaining;
      break;
    }
    else if (len == dashRemaining) {
      outp (GC_INDEX,LINE_CMD);
      while (inp(GC_DATA) & START_BIT)
	usleep(10) ;
      outpw (GC_INDEX,LINE_PIX_CNT+((len&0xff)<<8));
      outpw (GC_INDEX,(LINE_PIX_CNT+1)+(((len>>8)&0xff)<<8));
      outpw (GC_INDEX,LINE_CMD+(cmd<<8));
      if (++dashIndex == numInDashList)
        dashIndex = 0;
      *pdashIndex = dashIndex;
      *pdashOffset = 0;
      break;
    }
    else {
      outp (GC_INDEX,LINE_CMD);
      while (inp(GC_DATA) & START_BIT)
	usleep(10) ;
      outpw (GC_INDEX,LINE_PIX_CNT+((dashRemaining&0xff)<<8));
      outpw (GC_INDEX,(LINE_PIX_CNT+1)+(((dashRemaining>>8)&0xff)<<8));
      outpw (GC_INDEX,LINE_CMD+(cmd<<8));

      len -= dashRemaining;
      if (++dashIndex == numInDashList)
        dashIndex = 0;
      dashRemaining = pDash[dashIndex];
      cmd = START_BIT;
    }
  }
}



void
pwgaBresDouble(pScreen, fg, bg, alu, planemask, pdashIndex, pDash,
             numInDashList, pdashOffset,
             signdx, signdy, axis, x1, y1, e, e1, e2, len)
    ScreenPtr pScreen;
    unsigned int fg, bg;
    unsigned int alu;
    unsigned int planemask;
    int *pdashIndex;
    unsigned char *pDash;
    int numInDashList;
    int *pdashOffset;
    int signdx, signdy, axis;
    int x1, y1, e, e1, e2, len;
{
  int dashIndex;
  int i = 0;
  int dashRemaining;
  unsigned int cmd;
  unsigned int pattern = 0xffffffff;
  unsigned int patt_len = 0;
  int express = FALSE;
  vgaScreenPrivPtr vgaPriv = (vgaScreenPrivPtr) (pScreen->devPrivates[vgaScreenPrivateIndex].ptr);
  wgaShadowRegPtr pShadow = (wgaShadowRegPtr) (vgaPriv->avail);

#ifdef TRACE
  fprintf(stderr,"pwgaBresDouble: fg:%x,alu:%x,pm:%x,pdi:%x,pd:%x\n",
	  fg,alu,planemask,pdashIndex,pDash);
  fprintf(stderr,"          : nid:%x,pdo:%x,sx:%d,sy:%x,axis:%x\n",
	  numInDashList, pdashOffset, signdx, signdy, axis );
  fprintf(stderr,"          : x1:%x, y1:%x, e:%x, e1:%x, e2:%x, len:%x\n",
	  x1,y1,e,e1,e2,len);
#endif

  /* Sanity Check */
  if (len <= 0)
    return;

  GLOBALWAIT(pScreen->myNum, "pwgaBresDouble");

  if (axis)
    i |= Y_MAJOR;

  if (signdx < 0)
    i |= SIGN_X_NEG;

  if (signdy < 0)
    i |= SIGN_Y_NEG;

  outpw (GC_INDEX,LINE_CMD+((LINE_RESET)<<8));
  outpw (GC_INDEX,LINE_CMD+((CALC_ONLY)<<8));

  for (dashIndex = numInDashList-1; dashIndex>=0 && patt_len<=32; dashIndex--)
    patt_len += pDash[dashIndex];
  
  express = (patt_len <= 32);

  if (express) {
    for (dashIndex = numInDashList - 1; dashIndex >= 0; dashIndex--) {
      pattern >>= pDash[dashIndex];
      if (!(dashIndex & 1))
        pattern |= (1 << 32) - (1 << (32 - pDash[dashIndex]));
    }
    WGAUpdateBGColor(pShadow, bg);
  }
  
  WGAUpdateCtrlReg1(pShadow, EXPAND_TO_FG | BITS_PER_PIX_8 | ENAB_TRITON_MODE);

  WGAUpdateROP_A(pShadow, mergexlate[alu]);
  WGAUpdatePlaneMask(pShadow, planemask);
  WGAUpdateDataPathCtrl(pShadow, (SRC_IS_LINE_PATTERN | ROPSELECT_ALL |
				  PIXELMASK_ONLY | PLANARMASK_NONE_0XFF));

  WGAUpdateFGColor(pShadow, fg);
  
  if (express) {
    int startPoint;
    
    dashIndex = *pdashIndex;
    startPoint = *pdashOffset;
    while (--dashIndex >= 0)
      startPoint += pDash[dashIndex];
    startPoint %= 32;
    outpw (GC_INDEX,PATTERN_POINTER+((31 - startPoint) << 8));
  } else {
    outpw (GC_INDEX,PATTERN_POINTER+(31 << 8));
  }
  
  outpw (GC_INDEX,PATTERN_END+(((32 - patt_len) % 32) << 8));
  outpi (LINE_PATTERN, pattern, 0);

  outpi (X0_SRC_ADDR_LO, x1, y1);

  outpw (GC_INDEX,LINE_ERR_TERM+((e&0xff)<<8));
  outpw (GC_INDEX,(LINE_ERR_TERM+1)+(((e>>8)&0xff)<<8));

  outpw (GC_INDEX,K1_CONST+((e1&0xff)<<8));
  outpw (GC_INDEX,(K1_CONST+1)+(((e1>>8)&0xff)<<8));

  outpw (GC_INDEX,K2_CONST+((e2&0xff)<<8));
  outpw (GC_INDEX,(K2_CONST+1)+(((e2>>8)&0xff)<<8));

  outpw (GC_INDEX,SIGN_CODES+(i<<8));

  cmd = LAST_PIXEL_NULL;
  outpw (GC_INDEX,LINE_CMD+(cmd<<8));

  cmd |= START_BIT | LAST_PIXEL_NULL | AXIAL_WHEN_0 | REVERSIBLE_LINE ;

  dashIndex = *pdashIndex;
  dashRemaining = pDash[dashIndex] - *pdashOffset;

  if (express) {
    outpw (GC_INDEX,LINE_PIX_CNT+((len&0xff)<<8));
    outpw (GC_INDEX,(LINE_PIX_CNT+1)+(((len>>8)&0xff)<<8));
    outpw (GC_INDEX,LINE_CMD+(cmd<<8));
    while (len >= dashRemaining) {
      len -= dashRemaining;
      if (++dashIndex == numInDashList)
        dashIndex = 0;
      dashRemaining = pDash[dashIndex];
    }
    *pdashIndex = dashIndex;
    *pdashOffset = len;
    return;
  }

  for (;;) {
    if (len < dashRemaining) {
      outp (GC_INDEX,LINE_CMD);
      while (inp(GC_DATA) & START_BIT)
	usleep(10) ;

      if (dashIndex & 1) {
	WGAUpdateFGColor(pShadow, bg);
      } else {
	WGAUpdateFGColor(pShadow, fg);
      }
      outpw (GC_INDEX,LINE_PIX_CNT+((len&0xff)<<8));
      outpw (GC_INDEX,(LINE_PIX_CNT+1)+(((len>>8)&0xff)<<8));
      outpw (GC_INDEX,LINE_CMD+(cmd<<8));

      dashRemaining -= len;
      *pdashIndex = dashIndex;
      *pdashOffset = pDash[dashIndex] - dashRemaining;
      break;
    }
    else if (len == dashRemaining) {
      outp (GC_INDEX,LINE_CMD);
      while (inp(GC_DATA) & START_BIT)
	usleep(10) ;

      if (dashIndex & 1) {
	WGAUpdateFGColor(pShadow, bg);
      } else {
	WGAUpdateFGColor(pShadow, fg);
      }
      outpw (GC_INDEX,LINE_PIX_CNT+((len&0xff)<<8));
      outpw (GC_INDEX,(LINE_PIX_CNT+1)+(((len>>8)&0xff)<<8));
      outpw (GC_INDEX,LINE_CMD+(cmd<<8));
      if (++dashIndex == numInDashList)
        dashIndex = 0;
      *pdashIndex = dashIndex;
      *pdashOffset = 0;
      break;
    }
    else {
      outp (GC_INDEX,LINE_CMD);
      while (inp(GC_DATA) & START_BIT)
	usleep(10) ;

      if (dashIndex & 1) {
	WGAUpdateFGColor(pShadow, bg);
      } else {
	WGAUpdateFGColor(pShadow, fg);
      }
      outpw (GC_INDEX,LINE_PIX_CNT+((dashRemaining&0xff)<<8));
      outpw (GC_INDEX,(LINE_PIX_CNT+1)+(((dashRemaining>>8)&0xff)<<8));
      outpw (GC_INDEX,LINE_CMD+(cmd<<8));
      len -= dashRemaining;
      if (++dashIndex == numInDashList)
        dashIndex = 0;
      dashRemaining = pDash[dashIndex];
    }
  }
}



/*
 * 	Set the hardware color map entry 
 */

void pwgaSetPalette (
	Pixel pixel,
	unsigned short red,
	unsigned short green,
	unsigned short blue)
{

#if defined(TRACE) && TRACE > 1
  fprintf (stderr,"pwgaSetPalette: pixel: 0x%0.4x, r:%d, g: %d, b:%d\n",
	pixel,red,green,blue);
#endif

  outp( PALETTE_WRITE, pixel&0xff);
  outp( PALETTE_DATA, red>>8&0xff);     /* red */
  outp( PALETTE_DATA, green>>8&0xff);     /* green */
  outp( PALETTE_DATA, blue>>8&0xff);     /* blue */
}

/*
 *	Close the video down
 */
void VideoClose()
{
}


 
/* I presume this will clip the deltas */
void LimitMotionOutsideScreen(int clocx, int clocy, int *deltax, int *deltay)
{ 
  clocx += *deltax;
  if (clocx < 0) *deltax += (0 - clocx);
  if (clocx >= 1024) *deltax -= (clocx - 1024);
  clocy += *deltay;
  if (clocy < 0) *deltay += (0 - clocy);
  if (clocy >= 768) *deltay -= (clocy - 768);
}



void ResetScreenSaverTimer()
{
}



#ifdef TRACE
#if TRACE > 1
static unsigned char masktable[] = {
      0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
static unsigned char revtable[] = {
      0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};



static displayBitmap(bits, w, h, pad)
char * bits;
int w,h,pad;
{
int row, column;
static char chars[] = {'-','#'};
    
    putc ('\n',stderr);
    
    for (row = 0; row < h; row++) {
	fprintf (stderr,"%0.2x:",bits[0]&0xff);
        for (column = 0; column < w; column++) {
            int i = (column & 7);
    
            if ((bits[0] & masktable[i])) {
                putc (chars[1], stderr);
            } else {
                putc (chars[0], stderr);
            }
   
            if (i == 7) bits++;
        }
	bits+=pad;
        putc ('\n', stderr);

    }
    
  return;
}
#endif
#endif

