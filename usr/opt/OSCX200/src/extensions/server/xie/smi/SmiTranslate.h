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
/*******************************************************************************
**  Copyright 1989 by Digital Equipment Corporation, Maynard, Massachusetts,
**  and the Massachusetts Institute of Technology, Cambridge, Massachusetts.
**  
**                          All Rights Reserved
**  
**  Permission to use, copy, modify, and distribute this software and its 
**  documentation for any purpose and without fee is hereby granted, 
**  provided that the above copyright notice appear in all copies and that
**  both that copyright notice and this permission notice appear in 
**  supporting documentation, and that the names of Digital or MIT not be
**  used in advertising or publicity pertaining to distribution of the
**  software without specific, written prior permission.  
**  
**  DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
**  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
**  DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
**  ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
**  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
**  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
**  SOFTWARE.
**  
*******************************************************************************/

/*****************************************************************************
**
**  FACILITY:
**
**	X Imaging Extension
**      Sample Machine Independant DDX
**
**  ABSTRACT:
**
**      This is the header file for the TRANSLATE pipeline element module.
**
**  ENVIRONMENT:
**
**      VAX/VMS V5.0
**	VAX ULTRIX V3.1
**	RISC ULTRIX V3.1
**
**  AUTHOR(S):
**
**      Gary Grebus
**
**  CREATION DATE:
**
**      Wed Jul 11 10:29:06 1990
**
**  MODIFICATION HISTORY:
**
*****************************************************************************/
#ifndef _SMITRANSLATE
#define _SMITRANSLATE

/*
**  Include files
*/
#include <SmiPipe.h>

    /*	 
    **  Function reference
    */	 
int             SmiCreateTranslate();
int             SmiTranslate();

/*
**  Structure definitions and Typedefs
*/

/*
**  Translate pipeline context block.
*/
typedef struct _TranslatePipeCtx {
    PipeElementCommonPart   common;
    struct _TranslatePart {
	PipeSinkPtr	     src1;
	PipeSinkPtr	     src2;
	PipeSinkPtr	     dst;
	UdpPtr               srcidc;
	UdpPtr               dstidc;
	int                  srcx1;	
	int		     srcy1;
	int		     srcy2;
	int                  x1;	
	int                  x2;
	int		     y1;
	int		     y2;
	int                  src1state[XieK_MaxComponents];
	int                  ncmp;
	UdpPtr		     cpp;
	PipeDataPtr          srcdat1[XieK_MaxComponents];
	PipeDataPtr          srcdat2;
	PipeDrainPtr	     srcdrn1[XieK_MaxComponents];
	PipeDrainPtr	     srcdrn2;
    } TranslatePart;
} TranslatePipeCtx, *TranslatePipeCtxPtr;

/*
**  MACRO definitions
*/

/*
**  Pipeline context, Translate part, access MACROs
*/
#define TrnSrcDrn1_(ctx,cmp) \
		(((TranslatePipeCtxPtr)(ctx))->TranslatePart.srcdrn1[cmp])
#define TrnSrcDrn2_(ctx) (((TranslatePipeCtxPtr)(ctx))->TranslatePart.srcdrn2)
/*
**  Translate Parameters
*/
#define TrnSrcSnk1_(ctx)    (((TranslatePipeCtxPtr)(ctx))->TranslatePart.src1)
#define TrnSrcSnk2_(ctx)    (((TranslatePipeCtxPtr)(ctx))->TranslatePart.src2)
#define TrnDstSnk_(ctx)	    (((TranslatePipeCtxPtr)(ctx))->TranslatePart.dst)
#define TrnSrcX1_(ctx)	    (((TranslatePipeCtxPtr)(ctx))->TranslatePart.srcx1)
#define TrnSrcY1_(ctx)	    (((TranslatePipeCtxPtr)(ctx))->TranslatePart.srcy1)
#define TrnSrcY2_(ctx)	    (((TranslatePipeCtxPtr)(ctx))->TranslatePart.srcy2)
#define TrnRoiX1_(ctx)	    (((TranslatePipeCtxPtr)(ctx))->TranslatePart.x1)
#define TrnRoiX2_(ctx)	    (((TranslatePipeCtxPtr)(ctx))->TranslatePart.x2)
#define TrnRoiY1_(ctx)	    (((TranslatePipeCtxPtr)(ctx))->TranslatePart.y1)
#define TrnRoiY2_(ctx)	    (((TranslatePipeCtxPtr)(ctx))->TranslatePart.y2)
#define TrnSrc1State_(ctx, cmp)\
		 (((TranslatePipeCtxPtr)(ctx))->TranslatePart.src1state[cmp])

#define TrnNcmp_(ctx)      (((TranslatePipeCtxPtr)(ctx))->TranslatePart.ncmp)
#define TrnSrcIdc_(ctx)    (((TranslatePipeCtxPtr)(ctx))->TranslatePart.srcidc)
#define TrnDstIdc_(ctx)    (((TranslatePipeCtxPtr)(ctx))->TranslatePart.dstidc)
#define TrnCpp_(ctx)	    (((TranslatePipeCtxPtr)(ctx))->TranslatePart.cpp)
#define TrnDat1_(ctx,cmp) \
		(((TranslatePipeCtxPtr)(ctx))->TranslatePart.srcdat1[cmp])
#define TrnDat2_(ctx)	  (((TranslatePipeCtxPtr)(ctx))->TranslatePart.srcdat2)

#endif
/* _SMITRANSLATE */
