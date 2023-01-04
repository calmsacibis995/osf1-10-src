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
 * $XConsortium: skyReg.h,v 1.2 91/07/16 13:16:14 jap Exp $
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
 * skyReg.h - hardware register support
 */

#ifndef SKYREG_H
#define SKYREG_H

#define SKYWAYSetMode(index,m)  \
	SKYWAY_MODE_REG(index) = (m) ; \
	SKYWAY_WINCTRL_REG(index) = 0x00

#define SKYWAYSetRGBColor(index,r,g,b)  \
	{ SKYWAY_SINDEX_REG(index)   =  (PALETTESEQ  << 8) ; \
	  SKYWAY_SINDEX_REG(index)   =  (PALETTEDATA << 8) | ((r) >> 8) ; \
	  SKYWAY_SINDEX_REG(index)  =  (PALETTEDATA << 8) | ((g) >> 8) ; \
	  SKYWAY_SINDEX_REG(index)  =  (PALETTEDATA << 8) | ((b) >> 8) ; }

#define SKYWAYSetColorIndex(index,n)            \
	{ SKYWAY_SINDEX_REG(index) = (PALETTEMASK << 8) | 0xff ; \
	  SKYWAY_SINDEX_REG(index) = (SPINDEXLO << 8) | (0xff & (n)) ; \
	  SKYWAY_SINDEX_REG(index) =  (SPINDEXHI << 8) | 0x00 ;}

#define SKYWAYSetWidth(index,d)         SKYWAY_DM1_REG(index) = (short)(d)
#define SKYWAYSetHeight(index,d)                SKYWAY_DM2_REG(index) = (short)(d)
#define SKYWAYSetForegroundColor(index,f)       SKYWAY_FC_REG(index) = (int)(f)
#define SKYWAYSetBackgroundColor(index,b)       SKYWAY_BC_REG(index) = (int)(b)
#define SKYWAYSetCarryChain(index,c)            SKYWAY_CC_REG(index) = (unsigned int)(c)
#define SKYWAYSetCCV(index,c)                   SKYWAY_CCV_REG(index) = (int) (c)
#define SKYWAYSetCCC(index,c)                   SKYWAY_CCC_REG(index) = (unsigned short)(c)

#define SKYWAYSetForegroundMix(index,f) SKYWAY_FM_REG(index) = (f)
#define SKYWAYSetBackgroundMix(index,b) SKYWAY_BM_REG(index) = (b)

#define SKYWAYSetPixmapWidth(index,w)     SKYWAY_PMW_REG(index) = (short)(w)
#define SKYWAYSetPixmapHeight(index,h)  SKYWAY_PMH_REG(index) = (short)(h)
#define SKYWAYSetPixmapBase(index,b)    SKYWAY_PMB_REG(index) = (unsigned int)(b)
#define SKYWAYSetPixmapIndex(index,i)   SKYWAY_PMI_REG(index) = (short)(i)
#define SKYWAYSetPixmapControl(index,c) SKYWAY_PMC_REG(index) = (unsigned char)(c)
#define SKYWAYSetPixmapFormat(index,f)  SKYWAY_PMF_REG(index) = (short)(f)

#define SKYWAYSetBresenhamError(index,e)    SKYWAY_BME_REG(index) = (int) (e)
#define SKYWAYSetBresenhamErrorK1(index,k1) SKYWAY_BMK1_REG(index) = (int) (k1)
#define SKYWAYSetBresenhamErrorK2(index,k2) SKYWAY_BMK2_REG(index) = (int) (k2)
#define SKYWAYSetDirectionSteps(index,d)    SKYWAY_DRT_REG(index) = (int) (d)
#define SKYWAYSetPixelOp(index,p)             SKYWAY_PO_REG(index) = (unsigned int) (p)

#define SKYWAYSetDimension(index,w,h) \
	{ SKYWAY_DM1_REG(index) = (short) (w) ; \
	  SKYWAY_DM2_REG(index) = (short) (h) ; }

#define SKYWAYSetPixmapMaskOffset(index,x,y) \
	{ SKYWAY_MASKX_REG(index) = (short) (x) ; \
	  SKYWAY_MASKY_REG(index) = (short) (y) ; }

#define SKYWAYSetPixmapSrcOffset(index,x,y) \
	{ SKYWAY_SRCX_REG(index) = (short) (x) ; \
	  SKYWAY_SRCY_REG(index) = (short) (y) ; }

#define SKYWAYSetPixmapDstOffset(index,x,y) \
	{ SKYWAY_DSTX_REG(index) = (short) (x) ; \
	  SKYWAY_DSTY_REG(index) = (short) (y) ; }

#define SKYWAYSetPixmapPatOffset(index,x,y) \
	{ SKYWAY_PATX_REG(index) = (short) (x) ; \
	  SKYWAY_PATY_REG(index) = (short) (y) ; }

#define SKYWAYSetPageDirBase(index,p)       SKYWAY_PD_REG(index) = (unsigned int) (p)
#define SKYWAYSetVirtualAddr(index,v)       SKYWAY_VA_REG(index) = (unsigned int) (v)
#define SKYWAYGetStateALength(index,a)    (a) = SKYWAY_LA_REG(index)
#define SKYWAYGetStateBLength(index,b)    (b) = SKYWAY_LB_REG (index)

#define SKYWAYSetPlaneMask(index,p)   SKYWAY_PM_REG (index)= (p)
#define SKYWAYSetALU(index,a)   \
	{ SKYWAYSetForegroundMix(index,a) ; \
	  SKYWAYSetBackgroundMix(index,a) ; }

#define SKYWAYSetupScreenPixmap(index,pixsize) \
 { SKYWAYSetPixmapIndex(index,PixMapC) ; \
   SKYWAYSetPixmapBase(index,SKYWAY_COP_START) ; \
   SKYWAYSetPixmapWidth(index,1280 - 1) ; \
   SKYWAYSetPixmapHeight(index,1024 - 1) ; \
   SKYWAYSetPixmapFormat(index,MI1 | pixsize) ; }

#endif /* SKYREG_H */
