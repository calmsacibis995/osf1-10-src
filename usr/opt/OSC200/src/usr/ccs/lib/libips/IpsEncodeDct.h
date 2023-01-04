/*  DEC/CMS REPLACEMENT HISTORY, Element SMIENCODEDCT.H */
/*  *7     2-NOV-1990 13:37:12 SHELLEY "add SmiCreateEncodeDct entry point" */
/*  *6     1-NOV-1990 10:42:38 SHELLEY "first pass adding error handling" */
/*  *5    23-OCT-1990 15:23:44 HENNESSY "Finish DIX->DDX interface changes" */
/*   2A1  23-OCT-1990 15:21:28 HENNESSY "Merge changes" */
/*  *4    12-OCT-1990 16:53:16 WEBER "IPS integration" */
/*  *3     8-OCT-1990 15:12:04 WEBER "Merge changes" */
/*   1A1   8-OCT-1990 14:29:18 WEBER "Add pipeline support" */
/*  *2    25-SEP-1990 10:51:35 HENNESSY "Add DIX -> DDX interface change" */
/*  *1    18-SEP-1990 08:02:04 WEBER "DCT compression/decompression sources" */
/*  DEC/CMS REPLACEMENT HISTORY, Element SMIENCODEDCT.H */

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
**	DCT compression routine
**		  
**  ABSTRACT:
**
**	This module contains definitions required by the XIE SMI DCT 
**	compression routines.
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
**
**  CREATION DATE:
**
**      July 31, 1990
**
************************************************************************/

#ifndef ENCODEDCT
#define ENCODEDCT	/* the "endif" MUST be the last line of this file   */

/*
**  DCT decompression state structure.
*/
typedef struct _EncodeDctState {
    /*
    **	Decode state variable.
    */
    char    current_state;
    struct _EnDctStatus {
	unsigned	input_exhausted : 1;
	unsigned	new_data	: 1;
	unsigned	end_frame	: 1;
	unsigned	end_image	: 1;
	unsigned	reserved	: 4;
    } status;
    /*
    **	Information about where we are in destination data buffer.
    */
    unsigned char *current_ptr;
    int		   current_len;
    int		   current_remaining;
    /*
    **	Information about last token parsed.
    */
    int		   token;		/* Token type			    */
    /*
    **	DCT frame variables.
    */
    short   frame_width;		/* Pels/scan for frame		    */
    short   frame_height;		/* Scan count for frame		    */
    char    component_count;		/* Components per scan		    */
    short   restart_interval;		/* Restart interval		    */
    char    restart_enable;		/* Restart enabled for current scan */
    int	    restart_count;		/* Restart counter		    */
    int	    restart_lines;		/* Lines per restart		    */
    /*
    **	DCT per component variables.
    */
    struct _EnDctComponent {
	UdpRec  udp;			/* Source component UDP		    */
	char	vertical_samples;	/* Vertical sampling rate	    */
	char	horizontal_samples;	/* Horizontal sampling rate	    */
	char	quant_table;		/* Quantization table to use	    */
	char	huff_ac;		/* Huffman AC table for this comp.  */
	char	huff_dc;		/* Huffman DC table for this comp.  */
	int	scans_per_segment;	/* Number of scans between restarts */
	int	current_x;		/* Current X location for decode    */
	int	current_y;		/* Current Y location for decode    */
	short	previous_dc;		/* Previous DC value		    */
    } component[DctK_MaxComponents];
    /*
    **	Quantization tables.
    */
    char		quant_default[DctK_MaxQuantTables];
    QuantTablePtr	quant_table[DctK_MaxQuantTables];
    /*
    **	Huffman tables
    */
    AcEncodeTablePtr	huff_ac_table[DctK_MaxHuffTables];
    DcEncodeTablePtr	huff_dc_table[DctK_MaxHuffTables];
    /*
    **	Compression state variables.
    */
    int			interleaves_per_line;
    int			interleave_lines;
    int			horz_interleave_count;
    int			vert_interleave_counter;
    int			horz_interleave_counter;
} EncodeDctStateRec, *EncodeDctStatePtr;

#if defined(XIESMI)
    /*
    **  EncodeG4 pipeline context block.
    */
typedef struct _EncodeDctPipeCtx {
    PipeElementCommonPart   common;
    struct _EncodeDctPart {
    PipeSinkPtr		 src;
    int			 cmpmsk;
    PipeDrainPtr	 drn[XieK_MaxComponents];
    PipeDataPtr		 srcdat[XieK_MaxComponents];
    UdpPtr		 dst;
    EncodeDctStatePtr	 state;
    int			 factor;
    unsigned long	*final;
    } EncodeDctPart;
} EncodeDctPipeCtx, *EncodeDctPipeCtxPtr;
#endif

/*
**  MACROs to access state structure variables.
*/
#define DceState_(state)	(state)->current_state
#define DceInpExh_(state)	(state)->status.input_exhausted
#define DceNewDat_(state)	(state)->status.new_data
#define DceEndFrm_(state)	(state)->status.end_frame
#define DceEndImg_(state)	(state)->status.end_image

#define DceCurPtr_(state)	(state)->current_ptr
#define DceCurLen_(state)	(state)->current_len
#define DceCurRem_(state)	(state)->current_remaining

#define DceCurTok_(state)	(state)->token

#define DceMode_(state)		(state)->mode_union.mode
#define DceFrmWid_(state)	(state)->frame_width
#define DceFrmHgt_(state)	(state)->frame_height
#define DceCmpCnt_(state)	(state)->component_count
#define DceRstInt_(state)	(state)->restart_interval
#define DceRstEnb_(state)	(state)->restart_enable
#define DceRstCnt_(state)	(state)->restart_count

#define DceCmpVrt_(state,i)	(state)->component[(i)].vertical_samples
#define DceCmpHrz_(state,i)	(state)->component[(i)].horizontal_samples
#define DceCmpQnt_(state,i)	(state)->component[(i)].quant_table
#define DceCmpHac_(state,i)	(state)->component[(i)].huff_ac
#define DceCmpHdc_(state,i)	(state)->component[(i)].huff_dc
#define DceScnSeg_(state,i)	(state)->component[(i)].scans_per_segment
#define DceCurX_(state,i)	(state)->component[(i)].current_x
#define DceCurY_(state,i)	(state)->component[(i)].current_y
#define DceCmpPdc_(state,i)	(state)->component[(i)].previous_dc

#define	DceSrcUdp_(state,i)	(state)->component[(i)].udp
#define DceCmpIdx_(state,i)	(state)->component[(i)].udp.UdpL_CompIdx
#define DceCmpWid_(state,i)	(state)->component[(i)].udp.UdpL_PxlPerScn
#define DceCmpHgt_(state,i)	(state)->component[(i)].udp.UdpL_ScnCnt

#define DceQntTbl_(state,i)	(state)->quant_table[(i)]
#define DceQntDef_(state,i)	(state)->quant_default[(i)]

#define DceHacTbl_(state,i)	(state)->huff_ac_table[(i)]
#define DceHdcTbl_(state,i)	(state)->huff_dc_table[(i)]

#define DceVrtCnt_(state)	(state)->vert_interleave_counter
#define DceHrzCnt_(state)	(state)->horz_interleave_counter

#define DceIntCol_(state)	(state)->interleaves_per_line
#define DceIntLin_(state)	(state)->interleave_lines

    /*	 
    **  Function reference at end to allow for ptr defs
    */	 
#if defined(XIESMI)
unsigned long               SmiEncodeDct();
unsigned long               SmiEncodeDctSegment();
EncodeDctStatePtr           SmiEncodeDctInitializeState();
void                        SmiEncodeDctDestroyState();
int                         SmiCreateEncodeDct();
#endif

/*
**  MACROs to access pipeline context structure.
*/
#define DceSrcSnk_(ctx)		(ctx)->EncodeDctPart.src
#define DceDstUdp_(ctx)		(ctx)->EncodeDctPart.dst
#define DceFinal_(ctx)		(ctx)->EncodeDctPart.final
#define DceCmpMsk_(ctx)		(ctx)->EncodeDctPart.cmpmsk
#define DceCmpFct_(ctx)		(ctx)->EncodeDctPart.factor
#define DceSrcDrn_(ctx,i)	(ctx)->EncodeDctPart.drn[(i)]
#define DceSteBlk_(ctx)		(ctx)->EncodeDctPart.state
#define DceSrcDat_(ctx,i)	(ctx)->EncodeDctPart.srcdat[(i)]

/*
**  The following line MUST be the last line of this file.
*/
#endif
