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

/***********************************************************
Copyright 1990 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/************************************************************************
**
**  FACILITY:
**
**      XIE Sample Machine Independant DDX
**	DCT decompression routine
**		  
**  ABSTRACT:
**
**	This module contains definitions required by the XIE SMI DCT 
**	decompression routines.
**	
**  ENVIRONMENT:
**
**	VAX/VMS V5.3
**	ULTRIX  V4.0
**
**  AUTHOR(S):
**
**	John Nadai
**	John Weber
**	Robert NC Shelley
**
**  CREATION DATE:
**
**      July 31, 1990
**
************************************************************************/

    /*
    **	Allows to be included multiple times.
    */
#ifndef _DECODEDCT
#define _DECODEDCT

/*
**  Include Files
*/
#if	defined(XIESMI)
#include <SmiPipe.h>
#include <SmiDctDef.h>
#endif

/*
**  Equated symbols
*/
    /*
    **	Action routine status values
    */
#define DcdK_ActionSuccess  0
#define DcdK_ActionError    1
    /*
    **  Initial size and increment of unstuffed source data buffer.
    */
#define DcdK_DataSize       1024

/*
**  DCT decompression state structure.
*/
typedef struct _DecodeDctState {
    /*
    **	Decode state variable.
    */
    struct _DecDctStatus {
	char		state;
	struct _DctStatusFlags {
	    unsigned	data_seen	: 1;
	    unsigned	marker_seen	: 1;
	    unsigned	input_exhausted : 1;
	    unsigned	reserved	: 5;
	} flags;
        union {
            int		dst_full;
	    char	udp_full[sizeof(int)];
        } full;
    } status;
    /*
    **	The current source buffer.
    */
    int		   current_len;		/* Length of current data	    */
    int		   current_pos;		/* Position within current data	    */
    unsigned char *current_ptr;		/* Pointer to current data	    */
    /*
    **	Token type and data pointer/length.
    */
    int		   token_len;		/* Length of token data		    */
    int		   token_pos;		/* Position of token specific data  */
    unsigned char *token_ptr;		/* Pointer to token data	    */
    /*
    **	Data buffer -- for tokens that span source buffers and unstuffing data.
    */
    unsigned char *data_ptr;		/* Pointer to data buffer	    */
    int		   data_siz;		/* Size of data buffer		    */
    /*
    **	DCT frame variables.
    */
    union {
        char	mode;
	struct _DctMode {			/* DCT mode flags	    */
	    unsigned reserved_1	    : 1;	/*  Unused		    */
	    unsigned arith_coding   : 1;	/*  Arithmetic encoding	    */
	    unsigned progressive    : 1;	/*  Progressive	xmisson	    */
	    unsigned hierarchical   : 1;	/*  Hierarchical encoding   */
	    unsigned differential   : 1;	/*  Differential encoding   */
	    unsigned spatial	    : 1;	/*  Spatial encoding	    */
	    unsigned extended	    : 1;	/*  Extended version	    */
	    unsigned reserved_2	    : 1;	/*  Unused		    */
	} mode_bits;
    }	    mode_union;
    char    precision;			/* Data precision, 0 = 8 bits	    */
    short   frame_width;		/* Pels/scan for frame		    */
    short   frame_height;		/* Scan count for frame		    */
    char    component_count;		/* Components per scan		    */
    short   restart_interval;		/* Restart interval		    */
    char    restart_enable;		/* Restart enabled for current scan */
    /*
    **	DCT per component variables.
    */
    UdpPtr  dstlst[DctK_MaxComponents];	/* Destination UDPs		    */
    struct _DecDctComponent {
	char	component_index;	/* Component index value	    */
	char	dstidx;			/* Destination UDP index	    */
	char	vertical_samples;	/* Vertical sampling rate	    */
	char	horizontal_samples;	/* Horizontal sampling rate	    */
	char	quant_table;		/* Quantization table to use	    */
	int	component_width;	/* Pels/scan for component	    */
	int	component_height;	/* Scan count for component	    */
	int	component_stride;	/* Minimum stride required per scan */
	int	component_arsize;	/* Minimum scans required for image */
	char	component_huff_ac;	/* Huffman AC table for this comp.  */
	char	component_huff_dc;	/* Huffman DC table for this comp.  */
	int	scans_per_segment;	/* Number of scans between restarts */
	int	current_x;		/* Current X location for decode    */
	int	current_y;		/* Current Y location for decode    */
    } component[DctK_MaxComponents];
    /*
    **	Quantization tables.
    */
    char		quant_default[DctK_MaxQuantTables];
    QuantTablePtr	quant_table[DctK_MaxQuantTables];
    /*
    **	Huffman tables.
    */
    AcSpecTablePtr	ac_spec_table[DctK_MaxHuffTables];
    AcDecodeTablePtr	ac_huff_table[DctK_MaxHuffTables];
    DcSpecTablePtr	dc_spec_table[DctK_MaxHuffTables];
    DcDecodeTablePtr	dc_huff_table[DctK_MaxHuffTables];
    /*
    **	Decompression state variables.
    */
    int			interleaves_per_line;
    int			interleave_lines;
    int			horz_interleave_count;
    int			vert_interleave_counter;
    int			horz_interleave_counter;
} DecodeDctStateRec, *DecodeDctStatePtr;

/*
**  MACROs to access state structure variables.
*/
#define DcdState_(state_ptr)	(state_ptr)->status.state
#define DcdData_(state)		(state)->status.flags.data_seen
#define DcdMarker_(state)	(state)->status.flags.marker_seen
#define DcdInpExh_(state)	(state)->status.flags.input_exhausted
#define DcdDstFul_(state)	(state)->status.full.dst_full
#define DcdUdpFul_(state,i)	(state)->status.full.udp_full[(i)]

#define DcdCurLen_(state)	(state)->current_len
#define DcdCurPos_(state)	(state)->current_pos
#define DcdCurPtr_(state)	(state)->current_ptr

#define DcdTokLen_(state)	(state)->token_len
#define DcdTokPos_(state)	(state)->token_pos
#define DcdTokPtr_(state)	(state)->token_ptr

#define DcdDatPtr_(state)       (state)->data_ptr
#define DcdDatSiz_(state)       (state)->data_siz

#define DcdMode_(state)		(state)->mode_union.mode
#define DcdPrec_(state)		(state)->precision
#define DcdFrmWid_(state)	(state)->frame_width
#define DcdFrmHgt_(state)	(state)->frame_height
#define DcdCmpCnt_(state)	(state)->component_count
#define DcdRstInt_(state)	(state)->restart_interval
#define DcdRstEnb_(state)	(state)->restart_enable

#define DcdCmpIdx_(state,i)	(state)->component[(i)].component_index
#define DcdCmpVrt_(state,i)	(state)->component[(i)].vertical_samples
#define DcdCmpHrz_(state,i)	(state)->component[(i)].horizontal_samples
#define DcdCmpQnt_(state,i)	(state)->component[(i)].quant_table
#define DcdCmpWid_(state,i)	(state)->component[(i)].component_width
#define DcdCmpHgt_(state,i)	(state)->component[(i)].component_height
#define DcdCmpStr_(state,i)	(state)->component[(i)].component_stride
#define DcdCmpLen_(state,i)	(state)->component[(i)].component_arsize
#define DcdCmpHac_(state,i)	(state)->component[(i)].component_huff_ac
#define DcdCmpHdc_(state,i)	(state)->component[(i)].component_huff_dc
#define DcdScnSeg_(state,i)	(state)->component[(i)].scans_per_segment
#define DcdCurX_(state,i)	(state)->component[(i)].current_x
#define DcdCurY_(state,i)	(state)->component[(i)].current_y
#define DcdDstIdx_(state,i)	(state)->component[(i)].dstidx

#define	DcdDstLst_(state,i)	(state)->dstlst[(i)]

#define DcdQntTbl_(state,i)	(state)->quant_table[(i)]
#define DcdQntDef_(state,i)	(state)->quant_default[(i)]

#define DcdHacTbl_(state,i)	(state)->ac_huff_table[(i)]
#define DcdHdcTbl_(state,i)	(state)->dc_huff_table[(i)]
#define DcdSacTbl_(state,i)	(state)->ac_spec_table[(i)]
#define DcdSdcTbl_(state,i)	(state)->dc_spec_table[(i)]

#define DcdVrtCnt_(state)	(state)->vert_interleave_counter
#define DcdHrzCnt_(state)	(state)->horz_interleave_counter

#define DcdIntCol_(state)	(state)->interleaves_per_line
#define DcdIntLin_(state)	(state)->interleave_lines

#if defined(XIESMI)
    /*
    **  DecodeDct pipeline context block.
    */
typedef struct _DecodeDctPipeCtx {
    PipeElementCommonPart   common;
    struct _DecodeDctPart {
    PipeSinkPtr		 Src;
    PipeSinkPtr		 Dst;
    PipeDrainPtr	 Drn;
    PipeDataPtr		 SrcDat;
    PipeDataPtr		 DstDat[XieK_MaxComponents];
    UdpPtr		 DstUdp[XieK_MaxComponents];
    DecodeDctStatePtr	 SteBlk[XieK_MaxComponents];
    int			 SrcCnt;
    int			 DstCnt;
    int			 DcdCnt;
    } DecodeDctPart;
} DecodeDctPipeCtx, *DecodeDctPipeCtxPtr;

/*
**  MACROs to access pipeline context structure.
*/
#define DcdSrc_(ctx)		((ctx)->DecodeDctPart.Src)
#define DcdDst_(ctx)		((ctx)->DecodeDctPart.Dst)
#define DcdDrn_(ctx)		((ctx)->DecodeDctPart.Drn)
#define DcdSrcDat_(ctx)		((ctx)->DecodeDctPart.SrcDat)
#define DcdDstDat_(ctx,comp)	((ctx)->DecodeDctPart.DstDat[(comp)])
#define DcdDstUdp_(ctx,comp)	((ctx)->DecodeDctPart.DstUdp[(comp)])
#define DcdSteBlk_(ctx,comp)	((ctx)->DecodeDctPart.SteBlk[(comp)])
#define DcdSrcCnt_(ctx)		((ctx)->DecodeDctPart.SrcCnt)
#define DcdDstCnt_(ctx)		((ctx)->DecodeDctPart.DstCnt)
#define DcdDcdCnt_(ctx)		((ctx)->DecodeDctPart.DcdCnt)
#endif

    /*	 
    **  Function reference at end to allow ptr defs
    */	 
#ifdef XIESMI
extern int                  SmiDecodeDct();
extern int		    SmiCreateDecodeDct();
#endif

/*
**  The following line MUST be the last line of this file.
*/
#endif
/* _DECODEDCT */
