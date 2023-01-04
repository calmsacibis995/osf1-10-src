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
**  Copyright (c) Digital Equipment Corporation, 1990 All Rights Reserved.
**  Unpublished rights reserved under the copyright laws of the United States.
**  The software contained on this media is proprietary to and embodies the
**  confidential technology of Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and media is authorized only
**  pursuant to a valid written license from Digital Equipment Corporation.

**  RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the U.S.
**  Government is subject to restrictions as set forth in Subparagraph
**  (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19, as applicable.
**
**  Derived from work bearing the following copyright and permissions:
**
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
**      This module contains the support for the scale pipeline element.
**
**  ENVIRONMENT:
**
**      VAX/VMS V5.0
**
**  AUTHOR(S):
**
**      John Weber
**	Bob Shelley	(bitonal scale algorithms)
**	Gary Grebus	(cloned from SmiConvert)
**
**  CREATION DATE:
**
**      Mon Aug 27 14:01:43 1990
**
**  MODIFICATION HISTORY:
**
*****************************************************************************/

/*
**  Include files
*/
#include <stdio.h>

#include "DmiScale.h"

/*
**  Table of contents
*/
int	     DmiScale();
int          DmiCreateScale();

static int _ScaleInitialize();
static int _ScaleActivate();
static int _ScaleFlush();
static int _ScaleDestroy();

static ScaleProc _ScaleHorizontal();
static ScaleProc _ScaleVertical();
static ScaleProc _ScaleVerticalCl();

static int _ScaleVerticalUp();
static int _ScaleVerticalDown();
static int _ScaleVerticalUpCl();
static int _ScaleVerticalDownCl();
static int _ScaleVerticalNoneCl();
static int _ScaleVerticalDownClFast();

static int _ScaleHorizontalNone();
static int _ScaleHorizontalNoneCl();

static int _ScaleHorizontalDownBits();
static int _ScaleHorizontalDownBytes();
static int _ScaleHorizontalDownWords();
static int _ScaleHorizontalDownFloats();
static int _ScaleHorizontalDownCl();

static int _ScaleHorizontalUpBits();
static int _ScaleHorizontalUpBytes();
static int _ScaleHorizontalUpWords();
static int _ScaleHorizontalUpFloats();
static int _ScaleHorizontalUpCl();

static void _ScaleAnalyzeSrc();
static void _ScaleExtractDst();
static void _ScaleGetClUdp();
static void _ScaleDestroyClUdp();

static UdpPtr _ChkUdp();
static unsigned int *_BuildHrzMap();
static struct VPOINT *_BuildVRun();

/*
**  MACRO definitions
*/

/*
**  Equated Symbols
*/
#undef FAST_SCALE_DOWN			/* Select prettier but slower 
					 * form of changelist scale down */

#define	ALL0  8 /* VLIST window pre-set for NEXT == CURRENT == OUT == 0's   */
#define	ALL1  7 /* VLIST window pre-set for NEXT == CURRENT == OUT == 1's   */
#define OUT   1 /* VLIST window mask for OUT polarity bit		    */

/* Structure defining vertical run transition points. */
static struct VPOINT
    {
    long	flink;			/* link to next active column	    */
    int		window;			/* vertical window state	    */
    };

/*
**  External References
*/

/*
**	Local Storage
*/
    /*
    **  Scale Element Vector
    */
static PipeElementVector ScalePipeElement = {
    NULL,				/* FLINK			    */
    NULL,				/* BLINK			    */
    sizeof(PipeElementVector),		/* Size				    */
    TypePipeElementVector,		/* Structure type		    */
    StypeScaleElement,			/* Structure subtype		    */
    sizeof(ScalePipeCtx),		/* Context block size		    */
    0,					/* No input flag		    */
    0,					/* Reserved flags		    */
    _ScaleInitialize,			/* Initialize entry		    */
    _ScaleActivate,			/* Activate entry		    */
    _ScaleFlush,			/* Flush entry			    */
    _ScaleDestroy,			/* Destroy entry		    */
    NULL				/* Abort entry point		    */
    };

/*
**  Look-up tables for vertical scaling of bitonal images using changelists.
**
**  Source CHANGELISTs are processed in groups, by _ScaleAnalyzeSrc, to 
**  create destination CHANGELIST, via _ScaleExtractDst, which can then be
**  horizontally  scaled and written to the destination image.
**
**  Each SRC group consists of the N source scanlines required to make up a DST
**  scanline.  An array of VPOINT structures is treated as a linked list called
**  the VLIST.  The array elements represent vertical columns of the SRC bitmap
**  bracketed by column -1 (left of left) and X+1 (right of right).  The VLIST 
**  forms a sliding window which covers two groups of scanlines at a time.
**  Each VLIST element holds a 'flink' which links it to the next "active" 
**  column, and a 7 bit 'window' consisting of 3 fields:
**
**        +--6--+--5--+--4--+--3--+--2--+--1--+--0--+
**        |       NEXT      |     CURRENT     | OUT |
**        |< N ><N...1>< 0 >|< N ><N...1>< 0 >|     |
**        +-----+-----+-----+-----+-----+-----+-----+
**   - OUT	DST pixel polarity OUTput for the previous DST CHANGELIST,
**   - CURRENT  SRC group data for the CURRENT DST CHANGELIST being created,
**   - NEXT	SRC group data for the NEXT DST CHANGELIST to create.
**
**  The CURRENT and NEXT fields each consist of three bits (right to left):
**   - initial polarity of the group (ie. pixel polarity of CHANGELIST 0),
**   - first transition polarity within the group
**				     (ie. found in any CHANGELIST 1 thru N),
**   - final polarity of the group   (ie. pixel polarity of CHANGELIST N).
**	(If a group consists of a single CHANGELIST or no vertical transitions
**	      are encountered, all three bits will equal the initial polarity.)
**
**  Routine _ScaleAnalyzeSrc processes the NEXT SRC group.  Any newly "active"
**  column is linked in, and the contents of 'window' are used as an index
**  to look-up table 'next_0' or 'next_1' depending on whether the pixel
**  at the column being processed is a 0 or 1 respectively.  Only the NEXT
**  field is affected by the look-up as follows:
**
**	  previous  NEXT from	NEXT from
**	    NEXT      next_0	  next_1
**	    000		000	    110
**	    001		001	    101
**	    010		010	    110
**	    011		000	    111	    011 and 100 are initial values set
**	    100		000	    111	     by 'extract_dst' (see below).
**	    101		001	    101
**	    110		010	    110
**	    111		001	    111
**
**	(Entries containing ZERO in the following tables are never accessed.)
*/

#define ZERO 0
/*
**  0 bit read from next SRC CHANGELIST
*/
#ifdef VMS
static readonly int next_0[] = {
#else
static int next_0[] = {
#endif
/* 0    1    2    3    4    5    6    7    8    9    A    B    C    D    E   F*/
0x00,0x01,0x02,0x03,0x04,0x05,ZERO,0x0F,0x00,ZERO,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
0x10,0x11,0x12,0x13,0x14,0x15,ZERO,ZERO,ZERO,ZERO,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
0x20,0x21,0x22,0x23,0x24,0x25,ZERO,ZERO,ZERO,ZERO,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
0x00,0x01,0x02,0x03,0x04,0x05,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,
ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
0x10,0x11,0x12,0x13,0x14,0x15,ZERO,ZERO,ZERO,ZERO,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
0x20,0x21,0x22,0x23,0x24,0x25,ZERO,ZERO,ZERO,ZERO,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
0x10,0x11,0x12,0x13,0x14,0x15,ZERO,ZERO,ZERO,ZERO,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
				};

/*
**  1 bit read from next SRC CHANGELIST
*/
#ifdef VMS
static readonly int next_1[] = {
#else
static int next_1[] = {
#endif
/* 0    1    2    3    4    5    6    7    8    9    A    B    C    D    E   F*/
0x60,0x61,0x62,0x63,0x64,0x65,ZERO,0x7F,0x70,ZERO,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
0x50,0x51,0x52,0x53,0x54,0x55,ZERO,ZERO,ZERO,ZERO,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
0x60,0x61,0x62,0x63,0x64,0x65,ZERO,ZERO,ZERO,ZERO,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
0x70,0x71,0x72,0x73,0x74,0x75,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,
ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F,
0x50,0x51,0x52,0x53,0x54,0x55,ZERO,ZERO,ZERO,ZERO,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
0x60,0x61,0x62,0x63,0x64,0x65,ZERO,ZERO,ZERO,ZERO,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
0x70,0x71,0x72,0x73,0x74,0x75,ZERO,ZERO,ZERO,ZERO,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F
				};
/*
**  Routine _ScaleExtractDst reinitializes each "active" VLIST element by using
**  the contents of 'window' as the index to the look-up table.  The lookup
**  table can be chosen to further alter the algorithm so that perference is
**  given to increase the legibility of one pixel polarity over another. The
**  choice of look-up tables and their contents implements the following 
**  algorithm to determine the new value for OUT:
**
**  Note: currently the "PreferBoth" (i.e. prefer neither) mode is used.
**
**	IF (NOT( PreferBoth )
**	    AND( initial polarity of CURRENT equals OUT equals PREFERENCE )
**	    AND( there follows a SINGLE transition )
**	    AND( initial polarity of NEXT is not PREFERENCE )
**
**	    THEN    OUT will be of the PREFERENCE polarity,
**
**	    ELSE IF ( CURRENT contains any pixel not equal to OUT )
**
**		 THEN	complement OUT,
**
**		 ELSE	OUT remains unchanged.
**
**  Besides choosing the polarity of OUT, the table look-up also copies NEXT 
**  into CURRENT, and re-initializes NEXT to the special value, 011 or 100, 
**  depending on whether the 'final polarity' of NEXT had been 0 or 1.
**
**  If the new window state of a column is identical to the window state of 
**  the nearest "active" column to its left, the column is redundant, and 
**  therefore is unlinked within the VLIST (making it not "active").
*/

/* The prefer_0 and prefer_1 tables are not currently used since XIE protocol
** has no mechanism for expressing the preference.
*/

#ifdef NEVER_USED

/*
**  Decision table: PREFERENCE = 0
*/
#ifdef VMS
static readonly int prefer_0[] = {
#else
static int prefer_0[] = {
#endif
/* 0    1    2    3    4    5    6    7    8    9    A    B    C    D    E   F*/
ALL0,0x30,0x31,0x30,0x31,0x30,ZERO,ALL1,ALL0,ZERO,0x31,0x30,0x31,0x30,0x31,0x31,
0x32,0x32,0x33,0x32,0x33,0x32,ZERO,ZERO,ZERO,ZERO,0x33,0x32,0x32,0x32,0x33,0x33,
0x34,0x34,0x35,0x34,0x35,0x34,ZERO,ZERO,ZERO,ZERO,0x35,0x34,0x35,0x34,0x35,0x35,
ALL0,ALL0,ALL1,ALL0,ALL1,ALL0,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,
ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ALL1,ALL0,ALL0,ALL0,ALL1,ALL1,
0x4A,0x4A,0x4B,0x4A,0x4B,0x4A,ZERO,ZERO,ZERO,ZERO,0x4B,0x4A,0x4A,0x4A,0x4B,0x4B,
0x4C,0x4C,0x4D,0x4C,0x4D,0x4C,ZERO,ZERO,ZERO,ZERO,0x4D,0x4C,0x4D,0x4C,0x4D,0x4D,
0x4E,0x4E,0x4F,0x4E,0x4F,0x4E,ZERO,ZERO,ZERO,ZERO,0x4F,0x4E,0x4E,0x4E,0x4F,ALL1
				};

/*
**  Decision table: PREFERENCE = 1
*/
#ifdef VMS
static readonly int prefer_1[] = {
#else
static int prefer_1[] = {
#endif
/* 0    1    2    3    4    5    6    7    8    9    A    B    C    D    E   F*/
ALL0,0x30,0x31,0x31,0x31,0x30,ZERO,ALL1,ALL0,ZERO,0x31,0x30,0x31,0x30,0x31,0x31,
0x32,0x32,0x33,0x32,0x33,0x32,ZERO,ZERO,ZERO,ZERO,0x33,0x32,0x33,0x32,0x33,0x33,
0x34,0x34,0x35,0x35,0x35,0x34,ZERO,ZERO,ZERO,ZERO,0x35,0x34,0x35,0x34,0x35,0x35,
ALL0,ALL0,ALL1,ALL1,ALL1,ALL0,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,
ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ALL1,ALL0,ALL1,ALL0,ALL1,ALL1,
0x4A,0x4A,0x4B,0x4A,0x4B,0x4A,ZERO,ZERO,ZERO,ZERO,0x4B,0x4A,0x4B,0x4A,0x4B,0x4B,
0x4C,0x4C,0x4D,0x4D,0x4D,0x4C,ZERO,ZERO,ZERO,ZERO,0x4D,0x4C,0x4D,0x4C,0x4D,0x4D,
0x4E,0x4E,0x4F,0x4E,0x4F,0x4E,ZERO,ZERO,ZERO,ZERO,0x4F,0x4E,0x4F,0x4E,0x4F,ALL1
				};

#endif  /* NEVER_USED */
/*
**  Decision table: treat 0 and 1 equally
*/
#ifdef VMS
static readonly int prefer_both[] = {
#else
static int prefer_both[] = {
#endif
/* 0    1    2    3    4    5    6    7    8    9    A    B    C    D    E   F*/
ALL0,0x30,0x31,0x30,0x31,0x30,ZERO,ALL1,ALL0,ZERO,0x31,0x30,0x31,0x30,0x31,0x31,
0x32,0x32,0x33,0x32,0x33,0x32,ZERO,ZERO,ZERO,ZERO,0x33,0x32,0x33,0x32,0x33,0x33,
0x34,0x34,0x35,0x34,0x35,0x34,ZERO,ZERO,ZERO,ZERO,0x35,0x34,0x35,0x34,0x35,0x35,
ALL0,ALL0,ALL1,ALL0,ALL1,ALL0,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,
ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ZERO,ALL1,ALL0,ALL1,ALL0,ALL1,ALL1,
0x4A,0x4A,0x4B,0x4A,0x4B,0x4A,ZERO,ZERO,ZERO,ZERO,0x4B,0x4A,0x4B,0x4A,0x4B,0x4B,
0x4C,0x4C,0x4D,0x4C,0x4D,0x4C,ZERO,ZERO,ZERO,ZERO,0x4D,0x4C,0x4D,0x4C,0x4D,0x4D,
0x4E,0x4E,0x4F,0x4E,0x4F,0x4E,ZERO,ZERO,ZERO,ZERO,0x4F,0x4E,0x4F,0x4E,0x4F,ALL1
				};


/*****************************************************************************
**  DmiCreateScale
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function initializes the SCALE pipeline element
**
**  FORMAL PARAMETERS:
**
**      ctx - scale pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
int DmiCreateScale( pipe, srcsnk, dstsnk )
 Pipe		    pipe;
 PipeSinkPtr	    srcsnk;
 PipeSinkPtr	    dstsnk;
{
    ScalePipeCtxPtr ctx = (ScalePipeCtxPtr)
			   DdxCreatePipeCtx_( pipe, &ScalePipeElement, FALSE );
    if( !IsPointer_(ctx) ) return( (int) ctx );

    /*
    **	Copy parameters into context record
    */
    SclSrcSnk_(ctx) = srcsnk;
    SclDstSnk_(ctx) = dstsnk;
    /*
    **	Calculate scale factors
    */
    SclXScl_(ctx) = (double) SnkWidth_(dstsnk,0)  / SnkWidth_(srcsnk,0);
    SclYScl_(ctx) = (double) SnkHeight_(dstsnk,0) / SnkHeight_(srcsnk,0);
    /*
    **	Determine the image class.
    */
    SclIsBitonal_(ctx) = !IsPointer_(SnkUdpPtr_(srcsnk,1))
				     && SnkLvl_(srcsnk,0) == 2;

    /*
    **	Create a drain on the source sink to read all components.
    */
    SclSrcDrn_(ctx) = DdxRmCreateDrain_( srcsnk, -1 );
    if( !IsPointer_(SclSrcDrn_(ctx)) ) return( (int) SclSrcDrn_(ctx) );

    DdxRmSetQuantum_( SclSrcDrn_(ctx), SclYScl_(ctx) >= 1.0 ? 1
				     : (int)(1.0/SclYScl_(ctx)) );
    if( !SclIsBitonal_(ctx) )
	DdxRmSetDType_( SclSrcDrn_(ctx), UdpK_DTypeUndefined, DtM_Any );
    else
	{
	DdxRmSetDType_( SclSrcDrn_(ctx), UdpK_DTypeCL, DtM_CL );

	/* Precalculate the changelist horizontal scaling map */
	SclHrzMap_(ctx) = _BuildHrzMap( SnkUdpPtr_(SclSrcSnk_(ctx),0),
				        SclXScl_(ctx));
	if( SclHrzMap_(ctx) != NULL &&	/* Null means no hrz scale needed */
	    !IsPointer_(SclHrzMap_(ctx)) ) return( (int) SclHrzMap_(ctx) );

#ifndef FAST_SCALE_DOWN
	/* If scaling down, initialize the vertical run structure */
	if( SclYScl_(ctx) < 1.0 )
	    {
	    SclVRun_(ctx) = _BuildVRun( SnkUdpPtr_(SclSrcSnk_(ctx),0) );
	    if( !IsPointer_(SclVRun_(ctx)) ) return( (int) SclVRun_(ctx) );

	    /* Allocate the buffer for the composite scanline */
	    SclTmpCl_(ctx) =
		(unsigned *) DdxMalloc_( (SnkWidth_(SclSrcSnk_(ctx),0) + 2)
					   * sizeof(unsigned int) );
	    if( SclTmpCl_(ctx) == NULL ) return( BadAlloc );
	    } 
#endif
	}
    /*
    **  Set up Destination sink.
    */
    DdxRmSetQuantum_( dstsnk, 1 );
    if( SclIsBitonal_(ctx) )
	DdxRmSetDType_( dstsnk, UdpK_DTypeCL, DtM_CL );
    else
	DdxRmSetDType_( dstsnk, UdpK_DTypeUndefined, DtM_Any );

    return( Success );
}					/* end DmiCreateScale */

/*****************************************************************************
**  _ScaleInitialize
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function initializes the SCALE pipeline element
**
**  FORMAL PARAMETERS:
**
**      ctx - scale pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int	_ScaleInitialize( ctx )
ScalePipeCtxPtr	ctx;
{
    int status, dtype;

    /*
    **	Initialize input drain.
    */
    status = DdxRmInitializePort_( CtxHead_(ctx), SclSrcDrn_(ctx) );
    if( status != Success ) return( status );
    CtxInp_(ctx) = SclSrcDrn_(ctx);

    /*
    **	Initialize output sink (out dtype == in dtype).
    */
    dtype = DdxRmGetDType_( SclSrcDrn_(ctx) );
    DdxRmSetDType_( SclDstSnk_(ctx), dtype, 1<<dtype );
    status = DdxRmInitializePort_( CtxHead_(ctx), SclDstSnk_(ctx) );
    if( status != Success ) return( status );

    /*
    **	Determine horizontal scaling function.
    */
    SclHrzRou_(ctx) = _ScaleHorizontal(SclXScl_(ctx),dtype,SclIsBitonal_(ctx));
    if( !IsPointer_(SclHrzRou_(ctx)) ) return( (int) SclHrzRou_(ctx) );

    return( Success );
}

/*****************************************************************************
**  _ScaleActivate
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine controls data scaling
**
**  FORMAL PARAMETERS:
**
**      ctx - scale pipeline element context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int _ScaleActivate( ctx )
ScalePipeCtxPtr	ctx;
{
    PipeDataPtr copy, src, dst = NULL;
    int quantum, status, sy, c;

    for( status = Success; status == Success; src = DdxRmDeallocData_(src),
					      dst = DdxRmDeallocData_(dst) )
	{
	src = DdxRmGetData_( ctx, SclSrcDrn_(ctx) );
	if( !IsPointer_(src) ) return( (int) src );
	c  = DatCmpIdx_(src);

	if( SclVRun_(ctx) == NULL )
	    {	/* Use the normal scaling algorithm */
	    sy = (double) SclDstScn_(ctx,c) / SclYScl_(ctx);

            if( SclDstScn_(ctx,c) < SnkHeight_(SclDstSnk_(ctx),c)
		      && sy >= DatY1_(src) && sy <= DatY2_(src) )
		{
		dst = DdxRmAllocData_( SclDstSnk_(ctx), c,
				       SclDstScn_(ctx,c), 1 );
		status = (int) dst;
		if( !IsPointer_(dst) ) continue;

		if( DatDatTyp_(src) == UdpK_DTypeCL )
		    status = (*SclHrzRou_(ctx))( SclHrzMap_(ctx),
						 DatBase_(src),
						 DatBase_(dst) );
		else
		    status = (*SclHrzRou_(ctx))( DatUdpPtr_(src),
						 DatUdpPtr_(dst),
						 SclXScl_(ctx),
						 SclYScl_(ctx) );
	    
		for( sy = ++SclDstScn_(ctx,c) / SclYScl_(ctx);
		     status == Success && sy >= DatY1_(src) && sy <= DatY2_(src)
			  && SclDstScn_(ctx,c) < SnkHeight_(SclDstSnk_(ctx),c);
		     sy = ++SclDstScn_(ctx,c) / SclYScl_(ctx) )
		    {	/*
			**  Hand out reference copies to do vertical scale-up.
			*/
		    copy = DdxRmAllocRefDesc_( dst, DatY1_(dst), 1 );
		    if( IsPointer_(copy) )
			{
			status = DdxRmPutData_( SclDstSnk_(ctx), dst );
			dst    = copy;
			DatY1_(dst)++;
			DatY2_(dst)++;
			}
		    else
			status = (int) copy;
		    }
		if( status != Success ) continue;

		status = DdxRmPutData_( SclDstSnk_(ctx), dst );
		dst    = NULL;
		}
	    }

	else	/*  Special case for changelist scale down vertical */
	    {	/*
		**  Preload the vertical run structure.
		*/
	    _ScaleAnalyzeSrc( DatUdpPtr_(src), SclVRun_(ctx) );
	    _ScaleExtractDst( SclVRun_(ctx), DatWidth_(src), SclTmpCl_(ctx) );

	    if( SclDstScn_(ctx,c)++ > 0 )
		{   /*
		    **	Consume this new quantum, producing composite source
		    **	scanline from the scanlines represented in the VRun.
		    */
		dst = DdxRmAllocData_( SclDstSnk_(ctx), c,
				       SclDstScn_(ctx,c) - 2, 1 );
		status = (int) dst;
		if( !IsPointer_(dst) ) continue;

		status = (*SclHrzRou_(ctx))( SclHrzMap_(ctx),
					     SclTmpCl_(ctx),
					     DatBase_(dst) );
		if( status != Success ) continue;

		status = DdxRmPutData_( SclDstSnk_(ctx), dst );
		dst     = NULL;
		if( status != Success ) continue;
		/*
		**  Set the quantum for the next time around.
		*/
		quantum = (int)((SclDstScn_(ctx,c) + 1) / SclYScl_(ctx))
							- DatY2_(src) - 1;
		if( quantum + DatY2_(src) > SnkY2_(SclSrcSnk_(ctx),c) )
		    quantum = SnkY2_(SclSrcSnk_(ctx),c) - DatY2_(src);
		DdxRmSetQuantum_( SclSrcDrn_(ctx), quantum );
		}
	    }
	}
    return( status );
}

/*****************************************************************************
**  _ScaleFlush
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**      ctx - scale pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int	_ScaleFlush( ctx )
ScalePipeCtxPtr	ctx;
{
    int status = Success;
    PipeDataPtr dst;

    if( IsPointer_(SclVRun_(ctx)) )
	{   /*
	    **	The last scanline is still in the VRun structure.
	    */
	dst = DdxRmAllocData_( SclDstSnk_(ctx), 0, SclDstScn_(ctx,0) - 1, 1 );
	status = (int) dst;
	if( IsPointer_(dst) )
	    {
	    _ScaleExtractDst( SclVRun_(ctx),
			      SnkWidth_(SclSrcSnk_(ctx),0),
			      SclTmpCl_(ctx) );
	    status = (*SclHrzRou_(ctx))( SclHrzMap_(ctx),
					 SclTmpCl_(ctx),
				         DatBase_(dst) );
	    if( status == Success )
		status  = DdxRmPutData_( SclDstSnk_(ctx), dst );
	    else
		DdxRmDeallocData_( dst );
	    }
	}
    return( status );
}

/*****************************************************************************
**  _ScaleDestroy
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine deallocates all resources allocated by the scale
**	pipeline element.
**
**  FORMAL PARAMETERS:
**
**      ctx - scale pipeline context
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int	_ScaleDestroy( ctx )
ScalePipeCtxPtr	ctx;
{
    if( IsPointer_(SclHrzMap_(ctx)) )
	SclHrzMap_(ctx) = (unsigned int *) DdxFree_(SclHrzMap_(ctx));

#ifndef FAST_SCALE_DOWN
    if( IsPointer_(SclVRun_(ctx)) )
	{
	SclVRun_(ctx)  = (struct VPOINT *) DdxFree_(SclVRun_(ctx));
	SclTmpCl_(ctx) = (unsigned int *)  DdxFree_(SclTmpCl_(ctx));
	}
#endif
    SclSrcDrn_(ctx) = DdxRmDestroyDrain_(SclSrcDrn_(ctx));

    return( Success );
}

/*****************************************************************************
**  DmiScale
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
*****************************************************************************/
int	DmiScale( srcudp, dstudp )
UdpPtr	srcudp;
UdpPtr	dstudp;
{
    double xscale, yscale;
    int (*horizontal_scale)();
    int		      status = Success;
    unsigned int      *h_map = NULL;
    UdpPtr dst = NULL, cludp = NULL;

    if( dstudp->UdpA_Base == NULL )
	{   /*
	    **  Allocate dest image array.
	    */
	dstudp->UdpA_Base = DdxMallocBits_( dstudp->UdpL_ArSize );
	if( dstudp->UdpA_Base == NULL ) return( BadAlloc );
	}

    /*
    **	Calculate horizontal and vertical scale factors.
    */
    xscale = (double)dstudp->UdpL_PxlPerScn / (double)srcudp->UdpL_PxlPerScn;
    yscale = (double)dstudp->UdpL_ScnCnt    / (double)srcudp->UdpL_ScnCnt;
    /*
    **	Determine the proper horizontal scale routine to call based
    **	on source data type and scale factors.
    */
    horizontal_scale = _ScaleHorizontal( xscale, srcudp->UdpB_DType,
					 srcudp->UdpL_Levels == 2 );
    /*
    **	If destination is not a supported type, create an intermediate image.
    */
    dst = _ChkUdp( srcudp, dstudp );
    if( !IsPointer_(dst) )
	{
	status = (int) dst;
	goto cleanup;
	}
    /*
    **	Now, call the proper routine based on scale factors.
    */
    if( srcudp->UdpL_Levels != 2 )
	status = (*_ScaleVertical( yscale, srcudp ))
			    ( srcudp, dst, xscale, yscale, horizontal_scale );
    else
	{
	/* Scale bitonal images using changelists */
	_ScaleGetClUdp( srcudp, 1, &cludp );
	if( !IsPointer_(cludp) )
	    {
	    status = (int) cludp;
	    goto cleanup;
	    }
	h_map = _BuildHrzMap( srcudp, xscale );
	if( h_map != NULL && !IsPointer_(h_map) )
	    {
	    status = (int) h_map;
	    goto cleanup;
	    }
	status = (*_ScaleVerticalCl( yscale, srcudp ))
			( srcudp, dst, h_map, yscale, horizontal_scale, cludp );
	}
    if( status != Success ) goto cleanup;

    /*
    **	If a temporary image was necessary, convert to the desired data type.
    */
    if( dst != dstudp )
	status = DdxConvert_( dst, dstudp, XieK_MoveMode );

cleanup:
    if( dst != dstudp )
	{
	if( IsPointer_(dst->UdpA_Base) )
	    DdxFreeBits_(dst->UdpA_Base);
	DdxFree_(dst);
	}
    _ScaleDestroyClUdp( cludp );
    DdxFree_( h_map );
	
    return( status );
}

/*****************************************************************************
**  
**  _ScaleHorizontal
**  _ScaleVertical
**  _ScaleVerticalCl
**
**  FUNCTIONAL DESCRIPTION:
**
**	Return a pointer to the appropriate scaling routine, based on
**      the source image data type.
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static ScaleProc _ScaleHorizontal(xscale, dtype, bitonal)
double		xscale;
int		dtype;
int		bitonal;
{
    if( xscale < 1.0 )
	/*
	 **  Scaling down
	 */
	if( bitonal )
	    /* Handle bitonal images using change lists */
	    return( _ScaleHorizontalDownCl );
	else
	    switch( dtype )
		{
	    case UdpK_DTypeBU:
		return( _ScaleHorizontalDownBytes );

	    case UdpK_DTypeWU:
		return( _ScaleHorizontalDownWords );

	    case UdpK_DTypeF:
		return( _ScaleHorizontalDownFloats );

	    case UdpK_DTypeVU:
		return( _ScaleHorizontalDownBits );

	    case UdpK_DTypeUndefined:
	    case UdpK_DTypeLU:
	    default:
		return( NULL );
		}
    else if( xscale > 1.0 )
	/*
	**  Scaling up
	*/
	if( bitonal ) 
	    /* Handle bitonal images using change lists */
	    return( _ScaleHorizontalUpCl );
	else
	    switch( dtype )
		{
	    case UdpK_DTypeBU:
		return( _ScaleHorizontalUpBytes );

	    case UdpK_DTypeWU:
		return( _ScaleHorizontalUpWords );

	    case UdpK_DTypeF:
		return( _ScaleHorizontalUpFloats );

	    case UdpK_DTypeVU:
		return( _ScaleHorizontalUpBits );

	    case UdpK_DTypeLU:
	    case UdpK_DTypeUndefined:
	    default:
		return( NULL );
		} 
    else
	/*
	**  No scale
	*/
	if( bitonal )
	    /* Handle bitonal images using change lists */
	    return( _ScaleHorizontalNoneCl );
	else
	    return( _ScaleHorizontalNone );
}

static ScaleProc _ScaleVertical(yscale,src)
double		    yscale;
UdpPtr		    src;
{
    if( yscale <= 1.0 )
	/*
	**  Scaling down or none
	*/
	return( _ScaleVerticalDown );
    else
	/*
	**  Scaling up
	*/
	return( _ScaleVerticalUp );
}


static ScaleProc _ScaleVerticalCl(yscale,src)
double		    yscale;
UdpPtr		    src;
{
    if( yscale == 1.0 )
	/*
	**  No vertical scaling
	*/
	return( _ScaleVerticalNoneCl );

    else if( yscale < 1.0 ) 
	/*
	**  Scaling down
	*/
#ifdef FAST_SCALE_DOWN
	return( _ScaleVerticalDownClFast );
#else
	return( _ScaleVerticalDownCl );
#endif /* FAST_SCALE_DOWN */
    else 
	/*
	**  Scaling up
	*/
	return( _ScaleVerticalUpCl );
}

/*****************************************************************************
**
**  _ScaleVerticalUp
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int _ScaleVerticalUp( srcudp, dstudp, xscale, yscale, hrz_scale )
UdpPtr	srcudp;
UdpPtr	dstudp;
double	xscale;
double	yscale;
int   (*hrz_scale)();
{
    UdpRec   dscn;		    /* This is a UDP that will describe a   */
				    /* single destination scanline	    */
    int	     sx, sy, dx, dy, status;

    dscn = *dstudp;
    /*
    **	Set up a temporary UDP to describe a single scanline
    */
    dscn.UdpL_ScnCnt = 1;
    dscn.UdpL_Y1 = dscn.UdpL_Y2 = dstudp->UdpL_Y1;

    while( dscn.UdpL_Y1 <= dstudp->UdpL_Y2 )
	{
	sy = (int) ((double) dscn.UdpL_Y1 / yscale );

	status = (*hrz_scale)( srcudp, &dscn, xscale, yscale );
	if( status != Success ) return( status );

	dscn.UdpL_Y1   = ++dscn.UdpL_Y2;
	dscn.UdpL_Pos +=   dscn.UdpL_ScnStride;

	while( (int) ((double) dscn.UdpL_Y1 / yscale ) == sy && 
			       dscn.UdpL_Y1 <= dstudp->UdpL_Y2 )
	    {
	    memcpy( dscn.UdpA_Base +  dscn.UdpL_Pos / 8,
		    dscn.UdpA_Base + (dscn.UdpL_Pos-dscn.UdpL_ScnStride)/8,
		    dscn.UdpL_ScnStride / 8);
	    dscn.UdpL_Y1   = ++dscn.UdpL_Y2;
	    dscn.UdpL_Pos +=   dscn.UdpL_ScnStride;
	    }	    
	}
    return( Success );
}

/*****************************************************************************
**
**  _ScaleVerticalDown
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int _ScaleVerticalDown( srcudp, dstudp, xscale, yscale, hrz_scale )
UdpPtr	srcudp;
UdpPtr	dstudp;
double	xscale;
double	yscale;
int   (*hrz_scale)();
{
    UdpRec   dscn;		    /* This is a UDP that will describe a   */
				    /* single destination scanline	    */
    int	     sx, sy, dx, dy, status;

    dscn = *dstudp;
    /*
    **	Set up a temporary UDP to describe a single scanline
    */
    dscn.UdpL_ScnCnt = 1;
    dscn.UdpL_Y1 = dscn.UdpL_Y2 = dstudp->UdpL_Y1;

    while( dscn.UdpL_Y1 <= dstudp->UdpL_Y2 )
	{
	status = (*hrz_scale)( srcudp, &dscn, xscale, yscale );
	if( status != Success ) return( status );

	dscn.UdpL_Y1   = ++dscn.UdpL_Y2;
	dscn.UdpL_Pos +=   dscn.UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  
**  _ScaleVerticalNoneCl
**
**  FUNCTIONAL DESCRIPTION:
**
**	Perform only horizontal scaling for a bitonal image, using the
**      changelist scaling algorithm.
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int _ScaleVerticalNoneCl(srcudp, dstudp, h_map, yscale, hrz_scale, cludp)
UdpPtr	srcudp;
UdpPtr	dstudp;
unsigned int *h_map;
double	yscale;
int   (*hrz_scale)();
UdpPtr	cludp;
{
    UdpRec   tmpudp;			/* Udp for one source scanline */
    int status;

    int scanline;

    if( h_map == NULL )
	/* No horizontal scaling either. Just copy the image */
	status = DdxConvert_( srcudp, dstudp, XieK_MoveMode );

    else
	{
	status = Success;
	tmpudp = *srcudp;
	tmpudp.UdpL_ScnCnt = 1;
	for( scanline = srcudp->UdpL_Y1;
	     scanline <= srcudp->UdpL_Y2 && status == Success;
	     scanline++ )
	     {
	    /* Convert one input scanline to a change list */
	    tmpudp.UdpL_Y1 = tmpudp.UdpL_Y2 = scanline;
	    cludp->UdpL_Y1 = cludp->UdpL_Y2 = scanline;
	    status = DdxConvert_( &tmpudp, cludp, XieK_MoveMode );
	
	    /* Scale the scanline horizontally */
	    if( status == Success )
		status  = (*hrz_scale)(h_map, (unsigned int *)cludp->UdpA_Base,
				    	      (unsigned int *)cludp->UdpA_Base);
	
	    /* Convert the scaled changelist to the desired destination type */
	    if( status == Success )
		status = DdxConvert_( cludp, dstudp, XieK_MoveMode );
	    tmpudp.UdpL_Pos += tmpudp.UdpL_ScnStride;
	    }
	}					/* end _ScaleVerticalNoneCl */
    return( status );
}

/*****************************************************************************
**  
**  _ScaleVerticalUpCl
**
**  FUNCTIONAL DESCRIPTION:
**
**	Scale up vertically a bitonal image, using the changelist
**      scaling algorithm.  Each scanline is replicated y_scale times.
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int _ScaleVerticalUpCl(srcudp, dstudp, h_map, yscale, hrz_scale, cludp)
UdpPtr	srcudp;
UdpPtr	dstudp;
unsigned int *h_map;
double	yscale;
int   (*hrz_scale)();
UdpPtr	cludp;
{
    UdpRec   tmpudp;			/* Udp for one source scanline */

    int scanline;
    int dline;

    int start_scan;
    int end_scan;
    int status;

    tmpudp = *srcudp;
    tmpudp.UdpL_ScnCnt = 1;
    status = Success;

    if( h_map == NULL )
	/* No horizontal scaling - no need to use changelists after all */
	for( scanline = 0;
	     scanline <= srcudp->UdpL_Y2 - srcudp->UdpL_Y1 && status == Success;
	     scanline++ )
	    {
	    start_scan = (double)(scanline)   * yscale;
	    end_scan   = (double)(scanline+1) * yscale;

	    for( dline = start_scan;
		 dline < end_scan && status == Success;
		 dline++ )
		{
		/* Issue required number of copies of the scanline */
		tmpudp.UdpL_Y1 = tmpudp.UdpL_Y2 = dline;
		status = DdxConvert_( &tmpudp, dstudp, XieK_MoveMode );
		}
	    tmpudp.UdpL_Pos += tmpudp.UdpL_ScnStride;
	    }
    else
	for( scanline = srcudp->UdpL_Y1;
	     scanline <= srcudp->UdpL_Y2 && status == Success;
	     scanline++ )
	    {
	    /* Convert one input scanline to a change list */
	    tmpudp.UdpL_Y1 = tmpudp.UdpL_Y2 = scanline;
	    cludp->UdpL_Y1 = cludp->UdpL_Y2 = scanline;
	    status = DdxConvert_( &tmpudp, cludp, XieK_MoveMode );
	    if( status == Success )
		status  = (*hrz_scale)(h_map, (unsigned int *)cludp->UdpA_Base,
					      (unsigned int *)cludp->UdpA_Base);
	    if( status != Success ) continue;

	    start_scan = (double)(scanline - srcudp->UdpL_Y1)   * yscale;
	    end_scan   = (double)(scanline - srcudp->UdpL_Y1+1) * yscale;

	    for( dline = start_scan;
		 dline < end_scan && status == Success;
		 dline++ )
		{
		/* Issue required number of copies of the scanline */
		cludp->UdpL_Y1 = cludp->UdpL_Y2 = dline;
		status = DdxConvert_( cludp, dstudp, XieK_MoveMode );
		}
	    tmpudp.UdpL_Pos += tmpudp.UdpL_ScnStride;
	    }
    return( status );
}					/* end _ScaleVerticalUpCl */

/*****************************************************************************
**  
**  _ScaleVerticalDownCl
**
**  FUNCTIONAL DESCRIPTION:
**
**	Scale down vertically a bitonal image, by locating and scaling
**	vertical runs.
**
**	Vertical runs may shrink out of existance (e.g. thin horizontal lines,
**	black or white).  To prevent the loss of such detail we will detect
**	vertical transitions within the group of source scanlines which will
**	become the next destination scanline.  Any transition will result in
**	a destination pixel polarity change from that of the pixel in the
**	same column on the preceeding destination scanline.  If more than
**	two transitions occur the additional detail must be discarded.
**
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int _ScaleVerticalDownCl(srcudp, dstudp, h_map, yscale, hrz_scale,dst_cl)
UdpPtr	srcudp;
UdpPtr	dstudp;
unsigned int *h_map;
double	yscale;
int   (*hrz_scale)();
UdpPtr	dst_cl;
{
    struct VPOINT *v_run;		/* Linked VLIST array */

    int  dstline;
    int  count;
    int       quantum;
    int       status;

    UdpRec tmpudp;			/* Temp UDP to describe source */
    UdpPtr src_cl;			/* Temp UDP to hold source changelist*/

    v_run = NULL;
    src_cl = NULL;
    status = Success;

    /* Allocate the vertical changelist structure */
    v_run = _BuildVRun( srcudp );
    if( !IsPointer_(v_run) )
	{
	status = (int) v_run;
	goto cleanup;
	}
    tmpudp = *srcudp;

    dstline = dstudp->UdpL_Y1;
    quantum = (dstline + 1) / yscale - tmpudp.UdpL_Y1;

    /* Allocate and init a buffer to hold converted quanta of the source */
    _ScaleGetClUdp( &tmpudp, quantum + 1, &src_cl );
    if( !IsPointer_(src_cl) )
	{
	status = (int) src_cl;
	goto cleanup;
	}
    tmpudp.UdpL_ScnCnt = quantum;
    tmpudp.UdpL_Y2 = tmpudp.UdpL_Y1 + quantum - 1;

    src_cl->UdpL_ScnCnt = quantum;
    src_cl->UdpL_Y1 = tmpudp.UdpL_Y1;
    src_cl->UdpL_Y2 = src_cl->UdpL_Y1 + quantum - 1;

    status = DdxConvert_( &tmpudp, src_cl, XieK_MoveMode );
    if( status != Success ) goto cleanup;

    /* Pre-load the vertical run structure */
    _ScaleAnalyzeSrc(src_cl,  v_run);
    _ScaleExtractDst(v_run, tmpudp.UdpL_PxlPerScn,
		    (unsigned int *)dst_cl->UdpA_Base);

    tmpudp.UdpL_Y1  += quantum;
    tmpudp.UdpL_Pos += (quantum * tmpudp.UdpL_ScnStride);

    /*
    **  Process the remaining src scanlines and write dst scanlines.
    */
    for( count = dstudp->UdpL_ScnCnt - 1; count > 0; count-- )
	{
	dstline++;
	quantum = (dstline + 1) / yscale - tmpudp.UdpL_Y1;

	tmpudp.UdpL_ScnCnt = quantum;
	tmpudp.UdpL_Y2 = tmpudp.UdpL_Y1 + quantum - 1;

	src_cl->UdpL_ScnCnt = quantum;
	src_cl->UdpL_Y1 = tmpudp.UdpL_Y1;
	src_cl->UdpL_Y2 = src_cl->UdpL_Y1 + quantum - 1;

	status = DdxConvert_( &tmpudp, src_cl, XieK_MoveMode );
	if( status != Success ) goto cleanup;

	_ScaleAnalyzeSrc(src_cl, v_run);
	_ScaleExtractDst(v_run, tmpudp.UdpL_PxlPerScn,
			 (unsigned int *)dst_cl->UdpA_Base);

	if( h_map != NULL )
	    status = (*hrz_scale)( h_map, (unsigned int *)dst_cl->UdpA_Base,
					  (unsigned int *)dst_cl->UdpA_Base);
	dst_cl->UdpL_Y1 = dst_cl->UdpL_Y2 = dstline;
	if( status == Success )
	    status  = DdxConvert_( dst_cl, dstudp, XieK_MoveMode );
	if( status != Success ) goto cleanup;
	    
	tmpudp.UdpL_Y1  += quantum;
	tmpudp.UdpL_Pos += (quantum * tmpudp.UdpL_ScnStride);
	}

    /*
    **  Extract and write the final dst scanline.
    */
    _ScaleExtractDst(v_run, tmpudp.UdpL_PxlPerScn,
		    (unsigned int *)dst_cl->UdpA_Base);

    if( h_map != NULL )
	status = (*hrz_scale)( h_map, (unsigned int *)dst_cl->UdpA_Base,
				      (unsigned int *)dst_cl->UdpA_Base);

    dst_cl->UdpL_Y1 = dst_cl->UdpL_Y2 = dstline;
    if( status == Success )
        status  = DdxConvert_( dst_cl, dstudp, XieK_MoveMode );

cleanup:
    if( IsPointer_(v_run) )
	  DdxFree_(v_run);
    _ScaleDestroyClUdp(src_cl);

    return(status);
}					/* end _ScaleVerticalDownCl */

#ifdef FAST_SCALE_DOWN
/*****************************************************************************
**
**  _ScaleVerticalDownClFast
**
**  FUNCTIONAL DESCRIPTION:
**
**	Scale a bitonal image vertically by using only every (1/y_scale)th
**      scanline.  The image is processed using changelist scaling algorithms.
**      This is an alternative to _ScaleVerticalDownCl(), giving up
**      enhanced detail to gain speed.
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int
_ScaleVerticalDownClFast(srcudp, dstudp, h_map, yscale, hrz_scale, cludp)

UdpPtr	srcudp;
UdpPtr	dstudp;
unsigned int *h_map;
double	yscale;
int   (*hrz_scale)();
UdpPtr	cludp;
{
    UdpRec   tmpudp;			/* Udp for one source scanline */

    int scanline;
    int sline;			/* source scanline number */
    int      status;

    tmpudp = *srcudp;
    tmpudp.UdpL_ScnCnt = 1;

    status = Success;
    if( h_map == NULL )
	/* No horizontal scaling - no need to use changelists after all */
	for( scanline = dstudp->UdpL_Y1;
	     scanline <= dstudp->UdpL_Y2 && status == Success;
	     scanline++ )
	    {
	    sline = (int)((scanline - dstudp->UdpL_Y1) / yscale);
	    tmpudp.UdpL_Pos = sline * srcudp->UdpL_ScnStride +
		srcudp->UdpL_Pos;
	    tmpudp.UdpL_Y1 = tmpudp.UdpL_Y2 = scanline;
	    status = DdxConvert_( &tmpudp, dstudp, XieK_MoveMode );
	    }
    else
	for( scanline = dstudp->UdpL_Y1;
	     scanline <= dstudp->UdpL_Y2 && status == Success;
	     scanline++ )
	    {
	    sline = (int)((scanline - dstudp->UdpL_Y1) / yscale);

	    /* Convert required input scanline to a change list */
	    tmpudp.UdpL_Y1 = tmpudp.UdpL_Y2 = sline + srcudp->UdpL_Y1;
	    cludp->UdpL_Y1 = cludp->UdpL_Y2 = sline + srcudp->UdpL_Y1;
	    tmpudp.UdpL_Pos = sline * srcudp->UdpL_ScnStride +
		                srcudp->UdpL_Pos;
	    status = DdxConvert_( &tmpudp, cludp, XieK_MoveMode );

	    if( status == Success)
		{
		status = (*hrz_scale)( h_map, (unsigned int *)cludp->UdpA_Base,
					      (unsigned int *)cludp->UdpA_Base);
		cludp->UdpL_Y1 = cludp->UdpL_Y2 = scanline;
		if( status == Success )
		    status  = DdxConvert_( cludp, dstudp, XieK_MoveMode );
		}
	    }
}       				/* end _ScaleVerticalDownClFast */
#endif  /* FAST_SCALE_DOWN */

/*****************************************************************************
**  
**  _ScaleHorizontalNone
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int _ScaleHorizontalNone( srcudp, dstudp, xscale, yscale )
UdpPtr	srcudp;
UdpPtr	dstudp;
double	xscale;
double	yscale;
{
    return( DdxCopy_( srcudp, dstudp,
		      dstudp->UdpL_X1,
		      (int) ((double) dstudp->UdpL_Y1 / yscale)
				    - srcudp->UdpL_Y1,
		      dstudp->UdpL_X1, dstudp->UdpL_Y1,
		      dstudp->UdpL_PxlPerScn, dstudp->UdpL_ScnCnt ) );
}

/*****************************************************************************
**  
**  _ScaleHorizontalNoneCl
**
**  FUNCTIONAL DESCRIPTION:
**
**    Copy the input changelist to the output changelist without
**    performing any horizontal scaling.
**
**  FORMAL PARAMETERS:
**
**    map	- address of pre-scaled transition point array
**    inlst	- Ptr to the input changelist data.
**    outlst  -   Ptr to buffer to receive the output changelist data.
**                (could be the same as inlst).
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int _ScaleHorizontalNoneCl( map, inlst, outlst )
unsigned int *map;
unsigned int *inlst;
unsigned int *outlst;
{
    if( inlst != outlst )
	memcpy( outlst, inlst, (*inlst + 2) * sizeof(unsigned int) );

    return( Success );
}					/* end _ScaleHorizontalNoneCl */

/*****************************************************************************
**  
**  _ScaleHorizontalDownBits
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int _ScaleHorizontalDownBits( srcudp, dstudp, xscale, yscale )
UdpPtr	    srcudp;
UdpPtr	    dstudp;
double	    xscale;
double	    yscale;
{
    unsigned int   s_cmask, d_cmask, value;
    int		    s_scan_offset, s_offset;
    int		    d_scan_offset, d_offset;
    int		    sx, sy, dx, dy;

    s_cmask = srcudp->UdpL_Levels - 1;
    d_cmask = dstudp->UdpL_Levels - 1;

    d_scan_offset = dstudp->UdpL_Pos;

    for( dy = dstudp->UdpL_Y1;  dy <= dstudp->UdpL_Y2;  dy++ )
	{
	sy = (int)((double)dy / yscale );

	s_scan_offset = srcudp->UdpL_Pos + 
			((sy - srcudp->UdpL_Y1) * srcudp->UdpL_ScnStride);
	d_offset = d_scan_offset;

	for( dx = dstudp->UdpL_X1;  dx <= dstudp->UdpL_X2;  dx++ )
	    {
	    sx = (int)((double)dx / xscale );

	    s_offset = s_scan_offset + (srcudp->UdpL_PxlStride * sx);

	    value = GET_VALUE_(srcudp->UdpA_Base,s_offset,s_cmask);
	    PUT_VALUE_(dstudp->UdpA_Base, d_offset, value,d_cmask);

	    d_offset += dstudp->UdpL_PxlStride;
	    }
	d_scan_offset += dstudp->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  
**  _ScaleHorizontalDownBytes
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int _ScaleHorizontalDownBytes( srcudp, dstudp, xscale, yscale )
UdpPtr	srcudp;
UdpPtr	dstudp;
double	xscale;
double	yscale;
{
    int	     s_scan_offset, s_offset;
    int	     d_scan_offset, d_offset;
    int	     sx, sy, dx, dy;

    d_scan_offset = dstudp->UdpL_Pos;

    for( dy = dstudp->UdpL_Y1;  dy <= dstudp->UdpL_Y2;  dy++ )
	{
	sy = (int)((double)dy / yscale );

	s_scan_offset = srcudp->UdpL_Pos + 
			((sy - srcudp->UdpL_Y1) * srcudp->UdpL_ScnStride);
	d_offset = d_scan_offset;

	for( dx = dstudp->UdpL_X1;  dx <= dstudp->UdpL_X2;  dx++ )
	    {
	    sx = (int)((double)dx / xscale );

	    s_offset = s_scan_offset + (srcudp->UdpL_PxlStride * sx);

	    *(unsigned char *)(dstudp->UdpA_Base + d_offset / 8) =
	    *(unsigned char *)(srcudp->UdpA_Base + s_offset / 8);

	    d_offset += dstudp->UdpL_PxlStride;
	    }
	d_scan_offset += dstudp->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  
**  _ScaleHorizontalDownWords
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int _ScaleHorizontalDownWords( srcudp, dstudp, xscale, yscale )
UdpPtr	srcudp;
UdpPtr	dstudp;
double	xscale;
double	yscale;
{
    int	     s_scan_offset, s_offset;
    int	     d_scan_offset, d_offset;
    int	     sx, sy, dx, dy;

    d_scan_offset = dstudp->UdpL_Pos;

    for( dy = dstudp->UdpL_Y1;  dy <= dstudp->UdpL_Y2;  dy++ )
	{
	sy = (int)((double)dy / yscale );

	s_scan_offset = srcudp->UdpL_Pos + 
			((sy - srcudp->UdpL_Y1) * srcudp->UdpL_ScnStride);
	d_offset = d_scan_offset;

	for( dx = dstudp->UdpL_X1;  dx <= dstudp->UdpL_X2;  dx++ )
	    {
	    sx = (int)((double)dx / xscale );

	    s_offset = s_scan_offset + (srcudp->UdpL_PxlStride * sx);

	    *(unsigned short *)(dstudp->UdpA_Base + d_offset / 8) =
		*(unsigned short *)(srcudp->UdpA_Base + s_offset/8);

	    d_offset += dstudp->UdpL_PxlStride;
	    }
	d_scan_offset += dstudp->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  
**   _ScaleHorizontalDownFloats
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int _ScaleHorizontalDownFloats( srcudp, dstudp, xscale, yscale )
UdpPtr	srcudp;
UdpPtr	dstudp;
double	xscale;
double	yscale;
{
    int	     s_scan_offset, s_offset;
    int	     d_scan_offset, d_offset;
    int	     sx, sy, dx, dy;

    d_scan_offset = dstudp->UdpL_Pos;

    for( dy = dstudp->UdpL_Y1;  dy <= dstudp->UdpL_Y2;  dy++ )
	{
	sy = (int)((double)dy / yscale );

	s_scan_offset = srcudp->UdpL_Pos + 
			((sy - srcudp->UdpL_Y1) * srcudp->UdpL_ScnStride);
	d_offset = d_scan_offset;

	for( dx = dstudp->UdpL_X1;  dx <= dstudp->UdpL_X2;  dx++ )
	    {
	    sx = (int)((double)dx / xscale );

	    s_offset = s_scan_offset + (srcudp->UdpL_PxlStride * sx);

	    *(float *)(dstudp->UdpA_Base + d_offset / 8) =
		*(float *)(srcudp->UdpA_Base + s_offset/8);

	    d_offset += dstudp->UdpL_PxlStride;
	    }
	d_scan_offset += dstudp->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  
**  _ScaleHorizontalDownCl
**
**  FUNCTIONAL DESCRIPTION:
**
**	Scale down CHANGELIST transition points (i.e. shrink scanline width).
**
**	When scaling down, runs may shrink out of existance (black or white).
**	This results in the disappearance of thin vertical lines.
**	Under these circumstances, for the sake of retaining detail, we will
**	borrow a pixel from the beginning of the next run, if the combined 
**	scaled length of the current run and next run will result in at least 
**	two pixels.  Otherwise we will discard the current run or the next
**	run depending on run length (a run is a pair of transition points).
**
**  FORMAL PARAMETERS:
**
**	map	- address of pre-scaled transition point array
**	inlst	- pointer to the input changelist data
**      outlst  - pointer to output changelist buffer (could be the same
**                as inlst).
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int _ScaleHorizontalDownCl( map, inlst, outlst )
int *map;
unsigned int *inlst;
unsigned int *outlst;
{
    unsigned int beg;		    /* Scaled begin point of current run    */
    unsigned int end;	            /* Scaled end point of current run	    */
    int cnt = *inlst;		    /* Number of transitions in CHANGELIST  */
    unsigned int *cr =  outlst + 1; /* Current run pointer		    */
    unsigned int *nr =  inlst  + 1; /* Next run pointer			    */

    beg = map[*nr++];		    /* scale begin of initial run of ones   */

    while( --cnt > 0 )		    /* if at least a pair of points exists  */
	{
	end = map[*nr++];		/* scale end of current run	    */
	if( beg < end )			/* if run is at least one pixel long*/
	    *cr++ = beg;		/* ...save scaled begin point	    */

	else if( --cnt > 0 )		/* else, if another point exists    */
	    {
	    end = map[*nr++];		/* ...scale end of next run	    */
	    if( end - beg >= 2 )	/* if combined length of current    */
		{			/* ...and next run is >= 2 pixels   */
		*cr++ = beg;		/* ...save single pixel run	    */
		*cr++ = beg + 1;	/* ...and begin of next run	    */
		}
	    else if(  nr[-1] - nr[-2]	/* else, if next run is shorter	    */
		   <= nr[-2] - nr[-3] )	/* ...than its predecessor	    */
		continue;		/* ...discard the next run	    */
	    }				/* (else discard current run)	    */

	beg = end;			/* current end point is next begin  */
	}

    *cr = beg;			    /* scale scanline length		    */
    *outlst = cr - outlst;	    /* update size of CHANGELIST	    */

    return( Success );
}					/* end _ScaleHorizontalDownCl */

/*****************************************************************************
**  
** _ScaleHorizontalUpBits
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int _ScaleHorizontalUpBits( srcudp, dstudp, xscale, yscale )
UdpPtr	srcudp;
UdpPtr	dstudp;
double	xscale;
double	yscale;
{
    unsigned int  s_cmask, d_cmask, value;
    int		    s_scan_offset, s_offset;
    int		    d_scan_offset, d_offset;
    int		    sx, sy, dx, dy;

    s_cmask = srcudp->UdpL_Levels - 1;
    d_cmask = dstudp->UdpL_Levels - 1;

    d_scan_offset = dstudp->UdpL_Pos;

    for( dy = dstudp->UdpL_Y1; dy <= dstudp->UdpL_Y2; dy++ )
	{
	sy = (int)((double)dy / yscale );

	s_scan_offset = srcudp->UdpL_Pos + 
			((sy - srcudp->UdpL_Y1) * srcudp->UdpL_ScnStride);
	d_offset = d_scan_offset;

	for( dx = dstudp->UdpL_X1;  dx <= dstudp->UdpL_X2; )
	    {
	    sx = (int)((double)dx / xscale );

	    s_offset = s_scan_offset + (srcudp->UdpL_PxlStride * sx);

	    value = GET_VALUE_(srcudp->UdpA_Base,s_offset,s_cmask);

	    do	{
		PUT_VALUE_(dstudp->UdpA_Base, d_offset, value, d_cmask);
		dx++;
		d_offset += dstudp->UdpL_PxlStride;
		}
	    while( sx == (int)((double)dx / xscale ) && dx <= dstudp->UdpL_X2);
	    }
	d_scan_offset += dstudp->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  _ScaleHorizontalUpBytes
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int _ScaleHorizontalUpBytes(srcudp,dstudp,xscale,yscale)
UdpPtr	srcudp;
UdpPtr	dstudp;
double	xscale;
double	yscale;
{
    int	     s_scan_offset, s_offset;
    int	     d_scan_offset, d_offset;
    int	     sx, sy, dx, dy;

    unsigned char value;

    d_scan_offset = dstudp->UdpL_Pos;

    for( dy = dstudp->UdpL_Y1; dy <= dstudp->UdpL_Y2; dy++ )
	{
	sy = (int)((double)dy / yscale );

	s_scan_offset = srcudp->UdpL_Pos + 
			((sy - srcudp->UdpL_Y1) * srcudp->UdpL_ScnStride);
	d_offset = d_scan_offset;

	for( dx = dstudp->UdpL_X1;  dx <= dstudp->UdpL_X2; )
	    {
	    sx = (int)((double)dx / xscale );

	    s_offset = s_scan_offset + (srcudp->UdpL_PxlStride * sx);

	    value = *(unsigned char *) (srcudp->UdpA_Base + s_offset/8);

	    do	{
		*(unsigned char *)(dstudp->UdpA_Base + d_offset / 8) = value;
		dx++;
		d_offset += dstudp->UdpL_PxlStride;

		} while( sx == (int)((double)dx / xscale ) && 
			 dx <= dstudp->UdpL_X2);
	    }
	d_scan_offset += dstudp->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  _ScaleHorizontalUpWords
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int _ScaleHorizontalUpWords( srcudp, dstudp, xscale, yscale )
UdpPtr	srcudp;
UdpPtr	dstudp;
double	xscale;
double	yscale;
{
    int	     s_scan_offset, s_offset;
    int	     d_scan_offset, d_offset;
    int	     sx, sy, dx, dy;

    unsigned short value;

    d_scan_offset = dstudp->UdpL_Pos;

    for( dy = dstudp->UdpL_Y1; dy <= dstudp->UdpL_Y2; dy++ )
	{
	sy = (int)((double)dy / yscale );

	s_scan_offset = srcudp->UdpL_Pos + 
			((sy - srcudp->UdpL_Y1) * srcudp->UdpL_ScnStride);
	d_offset = d_scan_offset;

	for( dx = dstudp->UdpL_X1;  dx <= dstudp->UdpL_X2; )
	    {
	    sx = (int)((double)dx / xscale );

	    s_offset = s_scan_offset + (srcudp->UdpL_PxlStride * sx);

	    value = *(unsigned short *) (srcudp->UdpA_Base + s_offset/8);

	    do	{
		*(unsigned short *)(dstudp->UdpA_Base + d_offset / 8) = value;
		dx++;
		d_offset += dstudp->UdpL_PxlStride;
		}
	    while( sx == (int)((double)dx / xscale ) && dx <= dstudp->UdpL_X2 );
	    }
	d_scan_offset += dstudp->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  _ScaleHorizontalUpFloats
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int _ScaleHorizontalUpFloats( srcudp, dstudp, xscale, yscale )
UdpPtr	srcudp;
UdpPtr	dstudp;
double	xscale;
double	yscale;
{
    int	     s_scan_offset, s_offset;
    int	     d_scan_offset, d_offset;
    int	     sx, sy, dx, dy;
    float    value;

    d_scan_offset = dstudp->UdpL_Pos;

    for( dy = dstudp->UdpL_Y1; dy <= dstudp->UdpL_Y2; dy++ )
	{
	sy = (int)((double)dy / yscale );

	s_scan_offset = srcudp->UdpL_Pos + 
			((sy - srcudp->UdpL_Y1) * srcudp->UdpL_ScnStride);
	d_offset = d_scan_offset;

	for( dx = dstudp->UdpL_X1;  dx <= dstudp->UdpL_X2; )
	    {
	    sx = (int)((double)dx / xscale );

	    s_offset = s_scan_offset + (srcudp->UdpL_PxlStride * sx);

	    value = *(float *)
		    (srcudp->UdpA_Base + s_offset/8);

	    do	{
		*(float *)(dstudp->UdpA_Base + d_offset / 8) = value;
		dx++;
		d_offset += dstudp->UdpL_PxlStride;

		} while( sx == (int)((double)dx / xscale ) && 
			 dx <= dstudp->UdpL_X2);
	    }
	d_scan_offset += dstudp->UdpL_ScnStride;
	}
    return( Success );
}

/*****************************************************************************
**  _ScaleHorizontalUpCl
**
**  FUNCTIONAL DESCRIPTION:
**
**	Scale up CHANGELIST transition points (i.e. expand scanline width).
**
**  FORMAL PARAMETERS:
**
**	map	- address of pre-scaled transition point array
**	inlst	- Ptr to the input changelist data.
**      outlst  - Ptr to buffer to receive the output changelist data.
**               (could be the same as inlst).
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
*****************************************************************************/
static int _ScaleHorizontalUpCl( map, inlst, outlst )
unsigned int *map;
unsigned int *inlst;
unsigned int *outlst;
{
    unsigned int   i;		    /* point counter			    */
    unsigned int   *icr;	    /* input pointer                        */
    unsigned int   *ocr;	    /* output pointer                       */

    /*
    **	Scale up CHANGELIST transition points.
    */
    ocr = outlst;
    icr = inlst;
    *ocr++ = *icr;

    for( i = *icr++; i > 0; --i )
	*ocr++ = map[*icr++];			/* scale current point	    */

    return( Success );
}					/* end _ScaleHorizontalUpCl */

/******************************************************************************
**  _ScaleAnalyzeSrc
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine is responsible for detecting polarity changes in each 
**	column of pixels by analyzing the NEXT group of SRC CHANGELISTs.
**	New "active" columns are linked in, then the contents of 'window' are 
**	used as an index to look-up table 'next_0' or 'next_1' depending on 
**	whether the pixel at the column being processed is a '0' or '1' 
**	respectively.  Only the NEXT field is actually affected by the look-up.
**
**  FORMAL PARAMETERS:
**
**      srcudp  - Pointer to a UDP describing the next quantum
**                of the source image in changelist form.
**      v_run   - Pointer to vertical changelist (VLIST) array.
**
******************************************************************************/
static void _ScaleAnalyzeSrc( srcudp,  v_run)
    UdpPtr       srcudp;
    struct VPOINT *v_run;
{
    UdpRec	inudp;			/* Temporary descriptor */
    int append;	
    unsigned int *clst;			/* address of CHANGELIST	     */
    int	cind;			        /* index into CHANGELIST 	     */
    int vold;			        /* window state from prev scanline   */
    struct VPOINT *cnxt;	        /* next CHANGELIST column within VLIST*/
    struct VPOINT *v0;	                /* VLIST pointer to column 0	    */
    struct VPOINT *vcur;	        /* current VLIST column		    */
    struct VPOINT *vnxt;		/* next active VLIST column	    */
    struct VPOINT *vend;                /* end of VLIST			    */

    inudp = *srcudp;

    v0 = v_run + 1;
    vend = v0 + inudp.UdpL_PxlPerScn;

    /*
    **	Iterate through the scanlines in this quantum, appending their
    **	contents into the v_run.
    */
    for( append = 0; append < inudp.UdpL_ScnCnt; append++ )
	{
	clst = (unsigned int *)(inudp.UdpA_Base + (inudp.UdpL_Pos >> 3));
	vcur = v_run;
    	vnxt = (struct VPOINT *) vcur->flink;
	vold = vcur->window;
	cind =  1;
	cnxt = v0 + clst[cind++];

	while( vnxt < vend || cnxt < vend )
	    {
	    if( vnxt > cnxt )
		{			    /* CHANGELIST has next column   */
		vcur->flink = (long) cnxt;    /* link in new column	    */
		vcur = cnxt;
		vcur->flink = (long)vnxt;
		vcur->window = vold;	    /* state from previous scanline */
		cnxt = v0 + clst[cind++];   /* get next CHANGELIST column   */
		}
	    else
		{			    /* VLIST has next column	    */
		if( vnxt == cnxt )
		    cnxt = v0+clst[cind++]; /* get next CHANGELIST column   */
		vcur = vnxt;		    /* current column		    */
		vold = vcur->window;	    /* save state of prev scanline  */
		vnxt = (struct VPOINT *) vcur->flink;   /* next column	    */
		}
	    if(( cind & 1 ) == 0 )
		vcur->window = 
		    next_0[vcur->window];   /* update NEXT field with a 0   */
	    else
		vcur->window = 
		    next_1[vcur->window];   /* update NEXT field with a 1   */
	    }
	inudp.UdpL_Pos = DdxPosCl_( inudp.UdpA_Base, inudp.UdpL_Pos, 1 );
	inudp.UdpL_Y1++;
	}
}					/* end _ScaleAnalyzeSrc */

/******************************************************************************
**  _ScaleExtractDst
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine extracts a DST CHANGELIST from the VLIST window state and 
**	re-initializes the VLIST by unlinking redundant columns and updating
**	the window state of "active" columns.
**
**  FORMAL PARAMETERS:
**
**      v_run   - Point to VLIST (vertical changelist) array.
**      src_width - Number of pixels in source scanline used to build
**                the changelist to be extracted.
**	lst	- Pointer to buffer to receive the changelist
**
******************************************************************************/
static void _ScaleExtractDst( v_run, src_width, lst )
    struct VPOINT *v_run;
    int 	src_width;
    unsigned int  *lst;
{
    int cind;			        /* index into CHANGELIST	    */
    int vnew;				/* temporary VLIST window state	    */
    struct VPOINT *v0;			/* VLIST pointer to column 0        */
    struct VPOINT *prev;		/* previous active column	    */
    struct VPOINT *curr;		/* current column	   	    */
    struct VPOINT *vend;		/* end of VLIST			    */

    cind = 1;
    v0   = v_run + 1;
    prev = v_run;
    curr = (struct VPOINT *)prev->flink;
    vend = v0 + src_width;

    while( curr < vend )
	{
	vnew = prefer_both[curr->window];	/* choose new OUT, copy NEXT*/
						/*  to CURRENT, re-init NEXT*/
	if( prev->window + vnew & OUT )		/* X polarity change ?	    */
	    lst[cind++] = curr - v0;		/*  create CHANGELIST entry */
	else if( prev->window == vnew )		/* is this column redundant?*/
	    {
	    curr = (struct VPOINT *)curr->flink;/* unlink current column    */
	    prev->flink = (long) curr;
	    continue;
	    }
	curr->window = vnew;			/* save new window state    */
	prev = curr;				/* link to next column	    */
	curr = (struct VPOINT *) curr->flink;
	}
    lst[cind] = curr - v0;			/* stash scanline length    */
    *lst = cind;				/* stash CHANGELIST size    */
}					/* end _ScaleExtractDst */

/******************************************************************************
**  _ScaleGetClUdp
**
**  FUNCTIONAL DESCRIPTION:
**
**	Allocate a UDP to hold count worst-case changelists for a scanline
**      from the source image.
**
**  FORMAL PARAMETERS:
**
**      src     - Point to source image UDP
**      count   - Number of changelists to allocate.
**	dstptr	- Pointer to location to receive changelist UDP address.
**
******************************************************************************/
static void _ScaleGetClUdp( src, count, dstptr )
    UdpPtr       src;
    int          count;
    UdpPtr	 *dstptr;
{
    UdpPtr	c;

    c = (UdpPtr) DdxMalloc_(sizeof(UdpRec));
    if( c == NULL )
	*dstptr = (UdpPtr) BadAlloc;

    else
	{
	c->UdpB_DType	    = UdpK_DTypeCL;
	c->UdpB_Class	    = UdpK_ClassCL;
	c->UdpW_PixelLength = 0;
	c->UdpL_PxlStride   = 0;
	c->UdpL_ScnStride   = 0;
	c->UdpL_X1          = src->UdpL_X1;
	c->UdpL_X2          = src->UdpL_X2;
	c->UdpL_Y1          = src->UdpL_Y1;
	c->UdpL_Y2          = c->UdpL_Y1 + count - 1;
	c->UdpL_ScnCnt      = count;
	c->UdpL_PxlPerScn   = src->UdpL_PxlPerScn;
	c->UdpL_Pos         = 0;
	c->UdpL_Levels      = 2;
	c->UdpL_CompIdx     = src->UdpL_CompIdx;
	c->UdpL_ArSize      = count * (c->UdpL_PxlPerScn + 2) *
				       sizeof(unsigned int) * 8;
	c->UdpA_Base	    = DdxMallocBits_( c->UdpL_ArSize );
	*dstptr = c;
	}
}				    /* end _ScaleGetClUdp */

/******************************************************************************
**  _ScaleDestroyClUdp
**
**  FUNCTIONAL DESCRIPTION:
**
**	Deallocate a changelist UDP allocated by _ScaleGetClUdp.
**
**  FORMAL PARAMETERS:
**
**      udp     - Pointer to the changelist UDP
**
******************************************************************************/
static void _ScaleDestroyClUdp( udp )
    UdpPtr       udp;
{
    if( IsPointer_(udp) )
	{
	if( IsPointer_(udp->UdpA_Base) )
	    DdxFreeBits_( udp->UdpA_Base );
	DdxFree_( udp );
	}
}

/******************************************************************************
**  _BuildHrzMap
**
**  FUNCTIONAL DESCRIPTION:
**
**	Build the lookup table of precalculated scaled run lengths used
**      by the changelist horizontal scale routines.
**
**  FORMAL PARAMETERS:
**
**	src	- Pointer to src udp
**      xscale  - Horizontal scale factor
**
**  FUNCTION VALUE:
**      Pointer to the allocated map
**
******************************************************************************/
static unsigned int * _BuildHrzMap( src, xscale )
    UdpPtr	src;
    double	xscale;
{
    unsigned int *h_map;
    unsigned int *ptmp;
    unsigned int   tmp;

    if( xscale != 1.0 )
	{
	/* Pre-calculate the horizontal scaling map */
	h_map = (unsigned *)DdxMalloc_((src->UdpL_PxlPerScn + 2)
				      * sizeof(unsigned int));
	if( h_map == NULL ) return( (unsigned int *) BadAlloc );

	ptmp = h_map;
	for( tmp = 0; tmp <= src->UdpL_PxlPerScn; tmp++ )
	    *(ptmp++) = tmp * xscale;
        }
    else
	h_map = NULL;

    return( h_map );
}					/* end _BuildHrzScale */

/******************************************************************************
**  _BuildVRun
**
**  FUNCTIONAL DESCRIPTION:
**
**	Build the data structure used to keep track of vertical runs
**      when using the version of changelist scale that attempts to
**      preserve detail.
**
**  FORMAL PARAMETERS:
**
**	src	- Pointer to src udp
**
**  FUNCTION VALUE:
**      Pointer to the allocated structure
**
******************************************************************************/
static struct VPOINT * _BuildVRun( src )
    UdpPtr	src;
{
    struct VPOINT *v_run;

    /* Worst-case changelist contains entry for each pixel + 2 overhead */
    v_run = (struct VPOINT *) DdxMalloc_((src->UdpL_PxlPerScn + 2)
					* sizeof(struct VPOINT));
    if( !IsPointer_(v_run) ) return( (struct VPOINT *) BadAlloc );
    /*
    **  Initialize VLIST to contain single "end of scanline" element.
    **	- The first VLIST element is initialized as an imaginary SRC scanline
    **	  column left of the left edge (ie. it's treated as VLIST[-1] ).
    **	- The last VLIST element is initialized as an imaginary SRC scanline 
    **	  column right of the right edge (ie. it's treated as VLIST[N+1] ).
    */
					    /* link VLIST[-1] to VLIST[N+1] */
    v_run->flink = (long)(v_run+src->UdpL_PxlPerScn+1);
    v_run->window = ALL0;		    /* preset window for VLIST[-1]  */
    v_run[src->UdpL_PxlPerScn+1] = *v_run;  /* link VLIST[N+1] to itself    */

    return (v_run);
}					/* end _BuildVRun */

/******************************************************************************
**  _ChkUdp
**
**  FUNCTIONAL DESCRIPTION:
**
**	Check dst udp and convert to one of our supported types if needed.
**
**  FORMAL PARAMETERS:
**
**      in      - Point to input udp, which drives the type selection.
**	out	- Pointer to out udp
**
******************************************************************************/
static UdpPtr _ChkUdp( in, out )
    UdpPtr       in;
    UdpPtr	 out;
{
    int	itype, otype;
    UdpPtr dst = out;

    /* For bitonal, the type conversion is handled by the scale routines. */
    if( in->UdpL_Levels == 2 )
	return( dst );

    itype = DdxGetTyp_( in );
    otype = DdxGetTyp_( out );

    if( itype != otype )
	{
	dst = (UdpPtr) DdxMalloc_(sizeof(UdpRec));
	if( !IsPointer_(dst) ) return( (UdpPtr) BadAlloc );

	*dst = *out;

	switch( itype )
	    {
	case XieK_AF:
	    dst->UdpB_DType	  = UdpK_DTypeF;
	    dst->UdpW_PixelLength = sizeof(float) * 8;
	    break;
	case XieK_AWU:
	    dst->UdpB_DType	  = UdpK_DTypeWU;
	    dst->UdpW_PixelLength = sizeof(short) * 8;
	    break;
	case XieK_ABU:
	    dst->UdpB_DType	  = UdpK_DTypeBU;
	    dst->UdpW_PixelLength = sizeof(char) * 8;
	    break;
	default: 
	    dst->UdpB_DType	  = UdpK_DTypeVU;
	    dst->UdpW_PixelLength = 1;
	    break;
	    }
	dst->UdpB_Class	    = dst->UdpB_DType == UdpK_DTypeVU ? UdpK_ClassUBA
							      : UdpK_ClassA;
	dst->UdpL_PxlStride = dst->UdpW_PixelLength;
	dst->UdpL_ScnStride = dst->UdpL_PxlStride * out->UdpL_PxlPerScn;
	dst->UdpL_ArSize    = dst->UdpL_ScnStride * out->UdpL_ScnCnt;
	dst->UdpA_Base	    = DdxMallocBits_( dst->UdpL_ArSize );

	if( dst->UdpA_Base == NULL )
	    dst = (UdpPtr) DdxFree_( dst );
	}
    return( dst );
}				    /* end _ChkUdp */
/* end module DmiScale */
