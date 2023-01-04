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
/*  DEC/CMS REPLACEMENT HISTORY, Element SMICOMPARE.H */
/*  *3     1-NOV-1990 10:40:59 SHELLEY "first pass adding error handling" */
/*  *2    25-SEP-1990 10:49:15 HENNESSY "Add DIX -> DDX interface change" */
/*  *1    10-JUL-1990 16:48:42 GREBUS "Defns for Compare operator" */
/*  DEC/CMS REPLACEMENT HISTORY, Element SMICOMPARE.H */

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
**      This is the header file for the COMPARE pipeline element module.
**
**  ENVIRONMENT:
**
**      VAX/VMS V5.0
**	VAX ULTRIX V3.0
**	RISC ULTRIX V3.0
**
**  AUTHOR(S):
**
**      Gary L. Grebus
**
**  CREATION DATE:
**
**      Fri Jul  6 16:32:27 1990
**
**  MODIFICATION HISTORY:
**
*****************************************************************************/
#ifndef _SMICOMPARE
#define _SMICOMPARE

/*
**  Include files
*/
#include <SmiPipe.h>

/*	 
**  Function reference
*/	 
int             SmiCompare();
int             SmiCreateCompare();


/*
**  Structure definitions and Typedefs
*/

/*
**  Compare pipeline context block.
*/
typedef struct _ComparePipeCtx {
    PipeElementCommonPart   common;
    struct _ComparePart {
	PipeSinkPtr		srcsnk1;
	PipeSinkPtr		srcsnk2;
	PipeSinkPtr		dstsnk;
	PipeDrainPtr		srcdrn1[XieK_MaxComponents];
	PipeDrainPtr		srcdrn2[XieK_MaxComponents];
	PipeDataPtr             srcdata1[XieK_MaxComponents];
	PipeDataPtr             srcdata2[XieK_MaxComponents];
	unsigned long int       constant[XieK_MaxComponents];
	int                     ncmp_in;
	int                     ncmp_out;
	int                     combine_op;
	UdpPtr			cpp;
	long int		x1;
	long int		x2;
	long int		y1;
	long int		y2;
	void    		(*function)();
    } ComparePart;
} ComparePipeCtx, *ComparePipeCtxPtr;

/*
**  MACRO definitions
*/

/*
**  Pipeline context, Compare part, access MACROs
*/
#define CmpSrcSnk1_(ctx) \
		(((ComparePipeCtxPtr)(ctx))->ComparePart.srcsnk1)
#define CmpSrcSnk2_(ctx) \
		(((ComparePipeCtxPtr)(ctx))->ComparePart.srcsnk2)
#define CmpDstSnk_(ctx) (((ComparePipeCtxPtr)(ctx))->ComparePart.dstsnk)

#define CmpSrcDrn1_(ctx,comp) \
		(((ComparePipeCtxPtr)(ctx))->ComparePart.srcdrn1[comp])
#define CmpSrcDrn2_(ctx,comp) \
		(((ComparePipeCtxPtr)(ctx))->ComparePart.srcdrn2[comp])

#define CmpSrcData1_(ctx,comp) \
                (((ComparePipeCtxPtr)(ctx))->ComparePart.srcdata1[comp])
#define CmpSrcData2_(ctx,comp) \
                (((ComparePipeCtxPtr)(ctx))->ComparePart.srcdata2[comp])

#define CmpConst_(ctx,comp) \
		(((ComparePipeCtxPtr)(ctx))->ComparePart.constant[comp])
#define CmpCmpIn_(ctx)	  (((ComparePipeCtxPtr)(ctx))->ComparePart.ncmp_in)
#define CmpCmpOut_(ctx)	  (((ComparePipeCtxPtr)(ctx))->ComparePart.ncmp_out)
#define CmpCombOp_(ctx)   (((ComparePipeCtxPtr)(ctx))->ComparePart.combine_op)
#define CmpCpp_(ctx)	  (((ComparePipeCtxPtr)(ctx))->ComparePart.cpp)
#define CmpRoiX1_(ctx)	  (((ComparePipeCtxPtr)(ctx))->ComparePart.x1)
#define CmpRoiX2_(ctx)	  (((ComparePipeCtxPtr)(ctx))->ComparePart.x2)
#define CmpRoiY1_(ctx)	  (((ComparePipeCtxPtr)(ctx))->ComparePart.y1)
#define CmpRoiY2_(ctx)	  (((ComparePipeCtxPtr)(ctx))->ComparePart.y2)
#define CmpFunc_(ctx) \
		(((ComparePipeCtxPtr)(ctx))->ComparePart.function)

#endif
/* _SMICOMPARE */
