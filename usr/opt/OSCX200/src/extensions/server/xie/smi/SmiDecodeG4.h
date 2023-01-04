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
**      This is the header file for the G4 decompress pipeline element module.
**
**  ENVIRONMENT:
**
**      VAX/VMS V5.0
**	VAX ULTRIX V3.0
**	RISC ULTRIX V3.0
**
**  AUTHOR(S):
**
**      John Weber
**	Robert NC Shelley
**
**  CREATION DATE:
**
**      February 27, 1990
**
**  MODIFICATION HISTORY:
**
*****************************************************************************/
#ifndef _SMIDECODEG4
#define _SMIDECODEG4

/*
**  Include files
*/
#include <SmiPipe.h>

/*
**  Equated Symbols
*/

/*
**  State record for decompression
*/
typedef struct _DecodeG4State {
    int		    srcpos;
    int		    ppl;
    int		    mode;
    int 	   *rcl_desc;
    int		   *rcl;
    int		   *ccl;
    int		    ridx;
    int		    cidx;
    int		    a0;
    int		    a0prime;
    int		    a1;
    int		    b1;
    int		    rlen;
    int		    remdat;
    int		    remlen;
    int		    nxtscn;
    int		    lstscn;
    int             pad_flag;
} DecodeG4StateRec, *DecodeG4StatePtr;

/*
**  Huffman decode table definitions
*/
typedef struct _DecodeG4HuffTable {
    short	    value;
    unsigned char   length;
    char	    type;
} DecodeG4HuffTableRec, *DecodeG4HuffTablePtr;

    /*
    **  DecodeG4 pipeline context block.
    */
typedef struct _DecodeG4PipeCtx {
    PipeElementCommonPart   common;
    struct _DecodeG4Part {
    PipeSinkPtr		 Src;
    PipeSinkPtr		 Dst;
    PipeDrainPtr	 Drn;
    PipeDataPtr		 SrcDat;
    PipeDataPtr		 DstDat;
    DecodeG4StatePtr	 SteBlk;
    int                  NewDst;
    } DecodeG4Part;
} DecodeG4PipeCtx, *DecodeG4PipeCtxPtr;

/*
**  MACROs to access pipeline context structure.
*/
#define DG4SrcSnk_(ctx)		((ctx)->DecodeG4Part.Src)
#define DG4DstSnk_(ctx)		((ctx)->DecodeG4Part.Dst)
#define DG4SrcDrn_(ctx)		((ctx)->DecodeG4Part.Drn)
#define DG4SrcDat_(ctx)		((ctx)->DecodeG4Part.SrcDat)
#define DG4DstDat_(ctx)		((ctx)->DecodeG4Part.DstDat)
#define DG4SteBlk_(ctx)		((ctx)->DecodeG4Part.SteBlk)
#define DG4NewDst_(ctx)		((ctx)->DecodeG4Part.NewDst)

/*
**  Code table constants
*/
#define G4K_BlackTableSize	 8192
#define G4K_WhiteTableSize	 4096
#define G4K_DecodeTableSize	 4096
#define G4K_MinSrcLen		 14	    /* Defines the maximum codeword */
					    /* size in bits		    */
#define G4M_WhiteRun		 0x0FFF
#define G4M_BlackRun		 0x1FFF
#define G4M_Codeword		 0x0FFF
/*
**  Decoding mode constants
*/
#define G4K_ModeNewScan		 1
#define G4K_ModeCodeword	 2
#define G4K_ModeFirstRun	 3
#define G4K_ModeSecondRun	 4
/*
**  These constants define the usage of the change lists. If the change list
**  index is even, a white run is described. If a black run is being defined,
**  the changelist index will be odd.
*/
#define	G4K_WhiteRun		 1
#define G4K_BlackRun		 0
/*
**  Code type constants
*/
#define G4K_CodetypeTerminator	 2
#define G4K_CodetypeEol		-1
#define G4K_CodetypeMakeup	 1
/*
**  Code value constants
*/
#define G4K_CodevaluePass	 4
#define G4K_CodevalueHoriz	 5
#define G4K_CodevalueV0		 0
#define G4K_CodevalueVR1	 1
#define G4K_CodevalueVL1	-1
#define G4K_CodevalueVR2	 2
#define G4K_CodevalueVL2	-2
#define G4K_CodevalueVR3	 3
#define G4K_CodevalueVL3	-3
/*
**  Status code definitions
*/
#define G4K_StsEol		-1
#define G4K_StsDcdErr		-2
#define G4K_StsSrcExh		-3
#define G4K_StsDstFul		-4
#define G4K_StsEoi	        -5

/*
**  External References
*/
extern int	     SmiDecodeG4();
extern int	     SmiCreateDecodeG4();

externalref DecodeG4HuffTableRec MiArFax1DDecodeWhite[G4K_WhiteTableSize];
externalref DecodeG4HuffTableRec MiArFax1DDecodeBlack[G4K_BlackTableSize]; 
externalref DecodeG4HuffTableRec MiArFax2DDecodeTable[G4K_DecodeTableSize];

#endif
/* _SMIDECODEG4 */
