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
**      This is the header file for the G4 compress pipeline element module.
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
**	Robert NC Shelley
**
**  CREATION DATE:
**
**      Mon Mar 26 15:24:07 1990
**
**  MODIFICATION HISTORY:
**
*****************************************************************************/

#ifndef _SMIENCODEG4
#define _SMIENCODEG4

#include <SmiPipe.h>

/*
**  Structure definitions and Typedefs
*/
    /*
    **	This is the structure allocated by SmiEncodeG4InitState(), and used to
    **	maintain the state of a segmented compression.
    */
typedef struct _EncodeG4State {
    int		  *ccl;		    /* Pointer to current changelist	    */
    int		  *rcl;		    /* Pointer to reference changelist	    */
    int		   pos;		    /* destination bit offset		    */
    int		   size;	    /* total buffer size		    */
    unsigned char *base;	    /* destination buffer address	    */
    UdpPtr	   cvt;		    /* optional format conversion Udp	    */
} EncodeG4State, *EncodeG4StatePtr;

    /*
    **  EncodeG4 pipeline context block.
    */
typedef struct _EncodeG4PipeCtx {
    PipeElementCommonPart   common;
    struct _EncodeG4Part {
    PipeSinkPtr		 src;
    PipeDrainPtr	 drn;
    UdpPtr		 dst;
    EncodeG4StatePtr	 state;
    unsigned int	*final;
    } EncodeG4Part;
} EncodeG4PipeCtx, *EncodeG4PipeCtxPtr;

    /*
    **  Encode table structure.
    */
typedef struct	_EncodeG4HuffTable {
   short length;		    /* codeword length			    */
   short codeword;		    /* codeword				    */
} EncodeG4HuffTableRec, *EncodeG4HuffTablePtr;

/*
**  MACRO definitions
*/
    /*
    **	Macros for accessing EncodeG4StatePtr fields.
    */
#define esG4Ccl_(state)	    ((state)->ccl)
#define esG4Rcl_(state)	    ((state)->rcl)
#define esG4Pos_(state)	    ((state)->pos)
#define esG4Siz_(state)	    ((state)->size)
#define esG4Buf_(state)	    ((state)->base)
#define esG4Cvt_(state)	    ((state)->cvt)
    /*
    **	Macros for accessing EncodeG4PipeCtxPtr fields.
    */
#define EG4Src_(ctx)	((ctx)->EncodeG4Part.src)
#define EG4Drn_(ctx)	((ctx)->EncodeG4Part.drn)
#define EG4Dst_(ctx)	((ctx)->EncodeG4Part.dst)
#define EG4State_(ctx)	((ctx)->EncodeG4Part.state)
#define EG4Final_(ctx)	((ctx)->EncodeG4Part.final)

/*
**  Equated Symbols
*/
#define ENCODE_TABLE_SIZE 2561
/*
**  External References
*/
extern int              SmiEncodeG4();
extern int              SmiCreateEncodeG4();

externalref EncodeG4HuffTableRec MiAR_Fax1dEncodeWhite[ENCODE_TABLE_SIZE];
externalref EncodeG4HuffTableRec MiAR_Fax1dEncodeBlack[ENCODE_TABLE_SIZE];

#endif
/* _SMIENCODEG4 */
