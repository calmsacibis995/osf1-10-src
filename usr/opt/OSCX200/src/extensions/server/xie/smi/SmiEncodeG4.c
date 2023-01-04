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
Copyright 1989, 1990 by Digital Equipment Corporation, Maynard, Massachusetts,
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

/******************************************************************************
**++
**  FACILITY:
**
**      X Imaging Extension
**      Sample Machine Independant DDX
**
**  ABSTRACT:
**
**      This module implements compression using G4 Facsimile encoding.
**
**      Since compression is driven by requests from transport, it is not
**      implemented as a standard pipeline element.  The usual pipeline
**      routines are stubbed in for consistency.
**      
**      
**
**  ENVIRONMENT:
**
**	VAX/VMS V5.3
**	ULTRIX  V3.1
**
**  AUTHORS:
**
**	Gary L. Grebus
**	Derived from code originally developed by Ken MacDonald
**
**  CREATION DATE:     
**
**	Mon Mar 26 15:28:21 1990
**
*******************************************************************************/

#include <stdio.h>

#include "SmiEncodeG4.h"

/*
**  Table of contents
*/
int		 SmiEncodeG4();

int              SmiCreateEncodeG4();
static int 	_EncodeG4Initialize();
static int 	_EncodeG4Activate();
static int 	_EncodeG4Abort();
static int 	_EncodeG4Destroy();

static int              _EncodeG4Segment();
static EncodeG4StatePtr _InitializeG4State();
static EncodeG4StatePtr _DestroyG4State();
static void		_EncodeG4Process();

/*
**  Equated Symbols
*/
#define WHITE 0
#define BLACK 1
#define BLACK_TABLE_SIZE 8192
#define WHITE_TABLE_SIZE 4096
#define DECODE_TABLE_SIZE 4096
#define MAXCODEWORDSIZE 14

/*                                                                         
 * 2 Dimensional codeword and codeword length definitions used to
 * encode image data using either G32D or G42D schemes.          
 *                                                               
 * NOTE: These are coded as literals rather than as              
 *	values in a lookup table because there aren't       
 *	very many of them, and the encoding code will       
 *	run a little faster.  Decoding codewords are        
 *	stored in a table found in the MiFAX_DECODE_TABLES  
 *	module.                                             
 */                                                         
#define MiK_FAX2D_CODEWORD_PASS 8
#define MiK_FAX2D_CODELEN_PASS 4
#define MiK_FAX2D_CODEWORD_HORIZ 4
#define MiK_FAX2D_CODELEN_HORIZ 3
#define MiK_FAX2D_CODEWORD_V0 1
#define MiK_FAX2D_CODELEN_V0 1
#define MiK_FAX2D_CODEWORD_VR1 6
#define MiK_FAX2D_CODELEN_VR1 3
#define MiK_FAX2D_CODEWORD_VR2 48
#define MiK_FAX2D_CODELEN_VR2 6
#define MiK_FAX2D_CODEWORD_VR3 96
#define MiK_FAX2D_CODELEN_VR3 7
#define MiK_FAX2D_CODEWORD_VL1 2
#define MiK_FAX2D_CODELEN_VL1 3
#define MiK_FAX2D_CODEWORD_VL2 16
#define MiK_FAX2D_CODELEN_VL2 6
#define MiK_FAX2D_CODEWORD_VL3 32
#define MiK_FAX2D_CODELEN_VL3 7

/* Constants for some of the 1D codewords/lengths */
#define MiK_FAX1D_CODEWORD_EOL 2048
#define MiK_FAX1D_CODELEN_EOL 12
#define MiK_FAX1D_CODEWORD_WHITE2560 3968
#define MiK_FAX1D_CODELEN_WHITE2560 12
#define MiK_FAX1D_CODEWORD_BLACK2560 3968
#define MiK_FAX1D_CODELEN_BLACK2560 12

/*
**  Macros
*/
#ifndef min
#define min(a,b) (a<b?a:b)
#endif
#ifndef max
#define max(a,b) (a>b?a:b)
#endif
/*
**  Macros used for performing G4 codeword manipulations.
**  For Ultrix (and the non-VAX architectures Ultrix supports),
**  don't access longwords on non-longword aligned boundaries.
*/

/* Add a codeword of given length to the buffer */
#define ADD_CODEWORD_(base,bitptr,code,len) \
   XOR_WRITE32_(base + (bitptr >> 3),code << (bitptr & 0x7));\
   bitptr += len

/* Add a complete white run (makeup + term codes) to buffer (no check)
 * the rl (runlength parameter) is destroyed during this macro            
 */
#define ENCODE_1D_WHITERUN_(base,bitptr,rl) \
   while (rl >= 2560)\
      {\
      XOR_WRITE32_(base + (bitptr >> 3), \
      MiK_FAX1D_CODEWORD_WHITE2560 << (bitptr & 0x7));\
      bitptr += MiK_FAX1D_CODELEN_WHITE2560;\
      rl -= 2560;\
      }\
   if (rl >= 64)\
      {\
      working_run_length = rl & 0xFFFFFFC0;\
      rl &= 0x3F;\
      XOR_WRITE32_(base + (bitptr >> 3), \
         MiAR_Fax1dEncodeWhite[working_run_length].codeword << \
         (bitptr & 0x7));\
      bitptr += MiAR_Fax1dEncodeWhite[working_run_length].length;\
      }\
   XOR_WRITE32_(base + (bitptr >> 3), \
      MiAR_Fax1dEncodeWhite[rl].codeword << (bitptr & 0x7));\
   bitptr += MiAR_Fax1dEncodeWhite[rl].length

/* Add a complete black run (makeup + term codes) to buffer (no check) 
 * the rl (runlength parameter) is destroyed during this macro           
 */
#define ENCODE_1D_BLACKRUN_(base,bitptr,rl) \
   while (rl >= 2560)\
      {\
      XOR_WRITE32_(base + (bitptr >> 3), \
         MiK_FAX1D_CODEWORD_BLACK2560 << (bitptr & 0x7));\
      bitptr += MiK_FAX1D_CODELEN_BLACK2560;\
      rl -= 2560;\
      }\
   if (rl >= 64)\
      {\
      working_run_length = rl & 0xFFFFFFC0;\
      rl &= 0x3F;\
      XOR_WRITE32_(base + (bitptr >> 3),\
         MiAR_Fax1dEncodeBlack[working_run_length].codeword << \
         (bitptr & 0x7));\
      bitptr += MiAR_Fax1dEncodeBlack[working_run_length].length;\
      }\
   XOR_WRITE32_(base + (bitptr >> 3), \
      MiAR_Fax1dEncodeBlack[rl].codeword << (bitptr & 0x7));\
   bitptr += MiAR_Fax1dEncodeBlack[rl].length

/*
**  External References
*/
extern int 		*MiBuildChangeList();

/*
**  Local storage
*/
    /*
    **  EncodeG4 Element Vector
    */
externaldef(EncodeG4PipeElement) PipeElementVector EncodeG4PipeElement = {
    NULL,				/* FLINK			    */
    NULL,				/* BLINK			    */
    sizeof(PipeElementVector),		/* Size				    */
    TypePipeElementVector,		/* Structure type		    */
    StypeEncodeG4Element,		/* Structure subtype		    */
    sizeof(EncodeG4PipeCtx),		/* Context block size		    */
    0,					/* No input flag		    */
    0,					/* Reserved flags		    */
    _EncodeG4Initialize,		/* Initialize entry		    */
    _EncodeG4Activate,			/* Activate entry		    */
    _EncodeG4Abort,			/* Flush entry			    */
    _EncodeG4Destroy,			/* Destroy entry		    */
    _EncodeG4Abort			/* Abort entry point		    */
    };

/*****************************************************************************
**  SmiEncodeG4
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine performs CCITT G4 compression of an entire image.
**
**  FORMAL PARAMETERS:
**
**	src	- Pointer to the Udp which will feed the compression
**      dst	- Pointer to the Udp which will receive the compressed data
**
**  FUNCTION VALUE:
**
**      status
**
*****************************************************************************/
int	SmiEncodeG4( src, dst, size_ret )
 UdpPtr		src;
 UdpPtr		dst;
 unsigned long *size_ret;
{
    EncodeG4StatePtr state;
    int status, final = FALSE;

    state = _InitializeG4State( dst );
    if( state == NULL ) return( BadAlloc );

    status = _EncodeG4Segment( src, dst, state, &final, size_ret );

    _DestroyG4State( state );

    return( status == BadLength ? Success : status );
}                               /* end SmiEncodeG4 */

/*****************************************************************************
**  SmiCreateEncodeG4
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function creates the ENCODE G4 pipeline element
**
**  FORMAL PARAMETERS:
**
**      pipe	    - pipeline pointer
**      src_sink    - the sink from which to read PCM data
**	dst_udps    - pointer to destination Udps for G42D data
**	final	    - pointer to flag to set when finished
**
*****************************************************************************/
int SmiCreateEncodeG4( pipe, src_sink, dst_udps, final )
 Pipe		 pipe;
 PipeSinkPtr	 src_sink;
 UdpPtr		*dst_udps;
 unsigned int	*final;
{
    EncodeG4PipeCtxPtr ctx = (EncodeG4PipeCtxPtr)
			DdxCreatePipeCtx_( pipe, &EncodeG4PipeElement, FALSE );
    if( !IsPointer_(ctx) ) return( (int) ctx );

    /*
    **  Build our EncodeG4 Pipe Ctx from the arguments passed in.
    */
    EG4Src_(ctx)   = src_sink;
    EG4Dst_(ctx)   = dst_udps[0];
    EG4Final_(ctx) = final;
   *final &= ~(1<<upCmpIdx_(EG4Dst_(ctx)));
    /*
    **  Create drain.
    */
    EG4Drn_(ctx) = DdxRmCreateDrain_( src_sink, 1 );
    if( !IsPointer_(EG4Drn_(ctx)) ) return( (int) EG4Drn_(ctx) );

    DdxRmSetDType_( EG4Drn_(ctx), UdpK_DTypeVU, DtM_VU );
    DdxRmSetQuantum_( EG4Drn_(ctx), 0 );

    return( Success );
}                               /* end SmiCreateEncodeG4 */

/*****************************************************************************
**  _EncodeG4Initialize
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function initializes the ENCODE G4 pipeline element
**
**  FORMAL PARAMETERS:
**
**      ctx - encode G4 pipeline context
**
*****************************************************************************/
static int 	    _EncodeG4Initialize(ctx)
 EncodeG4PipeCtxPtr  ctx;
{
    int status = DdxRmInitializePort_( CtxHead_(ctx), EG4Drn_(ctx) );
    if( status == Success )
	{
	EG4State_(ctx) = _InitializeG4State( EG4Dst_(ctx) );
	if( EG4State_(ctx) == NULL ) return( BadAlloc );
	}
   *EG4Final_(ctx) &= ~(1<<upCmpIdx_(EG4Dst_(ctx)));
    CtxInp_(ctx)    = EG4Drn_(ctx);

    return( status );
}                               /* end _EncodeG4Initialize */

/*****************************************************************************
**  _EncodeG4Activate
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine performs G4 compression on a segment of the image.
**
**  FORMAL PARAMETERS:
**
**      ctx - encode G4 pipeline element context
**
*****************************************************************************/
static int 	    _EncodeG4Activate(ctx)
 EncodeG4PipeCtxPtr  ctx;
{
    PipeDataPtr   src;
    UdpPtr	  dst = EG4Dst_(ctx);
    unsigned long bits, status;

    for( status = Success; status == Success; src = DdxRmDeallocData_(src) )
	{
	src = DdxRmGetData_( ctx, EG4Drn_(ctx) );
	if( !IsPointer_(src) ) return( (int) src );

	if( !(*EG4Final_(ctx) & 1<<DatCmpIdx_(src))
	       && DatY2_(src) >= upY1_(dst) && DatY1_(src) <= upY2_(dst) )
	    {
	    status = _EncodeG4Segment( DatUdpPtr_(src),	dst,
				       EG4State_(ctx), EG4Final_(ctx), &bits );
	    if( status != Success )
		{   /*
		    **  Image didn't compress.
		    */
		upArSize_(dst) = 0;
		upPos_(dst)    = 0;
		if( status == BadLength )
		    status  = Success;
		}
	    else
		/*
		**  Use ArSize to point to the end of the compressed data.
		*/
		upArSize_(dst) += bits;
	    }
	}
    return( status );
}                               /* end _EncodeG4Activate */

/*****************************************************************************
**  _EncodeG4Abort
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine deallocates all resources allocated by the encode G4
**	pipeline element initialize and/or activate routines.
**
**  FORMAL PARAMETERS:
**
**      ctx - encode G4 pipeline context
**
*****************************************************************************/
static int 	    _EncodeG4Abort(ctx)
 EncodeG4PipeCtxPtr  ctx;
{
    EG4State_(ctx) = _DestroyG4State( EG4State_(ctx) );

    return( Success );
}                               /* end _EncodeG4Abort */

/*****************************************************************************
**  _EncodeG4Destroy
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine deallocates all resources allocated by the encode G4
**	pipeline element create routine.
**
**  FORMAL PARAMETERS:
**
**      ctx - encode G4 pipeline context
**
*****************************************************************************/
static int 	    _EncodeG4Destroy(ctx)
 EncodeG4PipeCtxPtr  ctx;
{
    EG4Drn_(ctx)   =  DdxRmDestroyDrain_( EG4Drn_(ctx) );

    return( Success );
}                               /* end _EncodeG4Destroy */

/*****************************************************************************
**  _EncodeG4Segment
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine performs CCITT G4 compression on a segment of an image.
**
**  FORMAL PARAMETERS:
**
**	src	 - Pointer to the Udp containing segment to be compressed
**      dst	 - Pointer to the Udp which will receive the compressed data
**      state	 - Pointer to the compression state information
**      final	 - Pointer to a flag to set when finished
**      size_ret - Pointer to where to stash number of bits encoded
**
**  FUNCTION VALUE:
**
**      status
**
*****************************************************************************/
static int _EncodeG4Segment( src, dst, state, final, size_ret )
 UdpPtr		     src;
 UdpPtr		     dst;
 EncodeG4StatePtr    state;
 int		    *final;
 unsigned long	    *size_ret;
{
    long int x1, y1, x2, y2, width, height;
    long int pos, used, size, line, status;
    int *tmp_ptr;
    UdpPtr   cvt = esG4Cvt_(state);

    *size_ret = 0;
    x1 = max( upX1_(src), upX1_(dst) );
    x2 = min( upX2_(src), upX2_(dst) );
    y1 = max( upY1_(src), upY1_(dst) );
    y2 = min( upY2_(src), upY2_(dst) );
    width  = x2 - x1 + 1;
    height = y2 - y1 + 1;

    *final |= width > 0  && height > 0 ? 1<<upCmpIdx_(src) : 0;
    if( upLvl_(src) != 2 ||  !( *final & 1<<upCmpIdx_(src) ) )
	return( BadMatch );

    if( upPxlStr_(src) == 1 )
	pos = upPos_(src) + x1-upX1_(src) + (y1-upY1_(src)) * upScnStr_(src);
    else if( cvt == NULL )
	{   /* create a scanline conversion Udp */
	cvt = (UdpPtr) DdxMalloc_(sizeof(UdpRec));
	if( cvt == NULL ) return( BadAlloc );
	esG4Cvt_(state)   = cvt;
        upPxlLen_(cvt)	  = 1;
        upDType_(cvt)	  = UdpK_DTypeVU;
        upClass_(cvt)	  = UdpK_ClassUBA;
        upX1_(cvt)	  = x1;
        upX2_(cvt)	  = x2;
        upWidth_(cvt)	  = width;
        upHeight_(cvt)	  = 1;
        upPxlStr_(cvt)	  = 1;
        upScnStr_(cvt)	  = width;
        upArSize_(cvt)	  = width;
        upBase_(cvt)	  = NULL;
        upPos_(cvt)	  = 0;
        upCmpIdx_(cvt)	  = upCmpIdx_(src);
        upLvl_(cvt)	  = 2;
	}
    if( upBase_(dst) == NULL )
	{
	upBase_(dst)  = DdxCallocBits_( esG4Siz_(state) - esG4Pos_(state) );
	esG4Buf_(state)   = NULL;
	if( upBase_(dst) == NULL ) return( BadAlloc );
	}
    if( upBase_(dst) != esG4Buf_(state) )
	{   /*
	    **	New buffer -- save its size and reset Pos.
	    */
	esG4Siz_(state) -= esG4Pos_(state);
	esG4Pos_(state)  = upPos_(dst);
	}
    esG4Buf_(state) = upBase_(dst);
    used = esG4Pos_(state);
    size = esG4Siz_(state) - used - (MAXCODEWORDSIZE * width + 1);

    for( line = y1; line <= y2; line++ )
	{   /*
	    **	Convert the scanline into a changelist.
	    */
	if( cvt == NULL )
	    {
	    MiBuildChangeList_(upBase_(src),pos,width,esG4Ccl_(state),width+2);
	    pos += upScnStr_(src);
	    }
	else
	    {
	    upY1_(cvt) = line;
	    upY2_(cvt) = line;
	    status = DdxConvert_( src, cvt, XieK_MoveMode );
	    if( status != Success ) return( status );
	    MiBuildChangeList_(upBase_(cvt), 0, width, esG4Ccl_(state), width+2);
	    }
	/*
	**  After building the changelist, add a few extra "stoppers".
	*/
	esG4Ccl_(state)[*esG4Ccl_(state)+1] = width;
	esG4Ccl_(state)[*esG4Ccl_(state)+2] = width;
	esG4Ccl_(state)[*esG4Ccl_(state)+3] = width;

	if( esG4Pos_(state) > size )  return( BadLength );
	/*
	**  Convert changelist into CCITT G42D compressed data.
	*/
	_EncodeG4Process( state );
	/*
	**  Swap pointers for current and reference change lists.
	*/
	tmp_ptr	        = esG4Rcl_(state);
	esG4Rcl_(state) = esG4Ccl_(state);
	esG4Ccl_(state) = tmp_ptr;
	}				/* end for */

    if( line < upY2_(dst) )
	*final &= ~(1<<upCmpIdx_(src));

    else if( esG4Pos_(state) + 2 * MiK_FAX1D_CODELEN_EOL <= esG4Siz_(state) )
	{   /*
	    **  Insert two copies of EOL to form an EOB in the output.
	    */
	ADD_CODEWORD_(esG4Buf_(state), esG4Pos_(state),
		      MiK_FAX1D_CODEWORD_EOL, MiK_FAX1D_CODELEN_EOL);
	ADD_CODEWORD_(esG4Buf_(state), esG4Pos_(state),
		      MiK_FAX1D_CODEWORD_EOL, MiK_FAX1D_CODELEN_EOL);
	esG4Pos_(state) += 2 * MiK_FAX1D_CODELEN_EOL;
	}
    else
	return( BadLength );

    *size_ret = esG4Pos_(state) - used;

    return( Success );
}                               /* end _EncodeG4Segment */

/*****************************************************************************
**  _InitializeG4State
**
**  FUNCTIONAL DESCRIPTION:
**
**	Create a context which will be used to compress image data in segments.
**
**  FORMAL PARAMETERS:
**
**	dst	- pointer to the Udp which will contain the compressed data
**
**  FUNCTION VALUE:
**
**      Returns a pointer to an EncodeG4State context (if successful).
**
*****************************************************************************/
static EncodeG4StatePtr _InitializeG4State(dst)
 UdpPtr dst;
{
    EncodeG4StatePtr state;

    /*
    **	Allocate a state structure
    */
    state = (EncodeG4StatePtr) DdxCalloc_(1, sizeof(EncodeG4State));
    if( state == NULL )
	return(  NULL );
    
    /*
    **	Allocate buffers to hold the change lists.
    */
    esG4Ccl_(state) = (int *) DdxMalloc_((upWidth_(dst) + 5) * sizeof(int));
    if( esG4Ccl_(state) == NULL )
	{
	_DestroyG4State(state);
	return( NULL );
	}
    esG4Rcl_(state) = (int *) DdxMalloc_((upWidth_(dst) + 5) * sizeof(int));
    if( esG4Rcl_(state) == NULL )
	{
	_DestroyG4State(state);
	return( NULL );
	}
    /*
    **	Init the reference changelist to the imaginary line used to start
    **	the G4 compression algorithm
    */
    esG4Rcl_(state)[0] = 1;
    esG4Rcl_(state)[1] = upWidth_(dst);
    esG4Rcl_(state)[2] = upWidth_(dst);
    esG4Rcl_(state)[3] = upWidth_(dst);
    esG4Rcl_(state)[4] = upWidth_(dst);
    /*
    **	Limit size of compressed data to size of PCM data.
    **	If the data expands beyond this limit it obviously can't be compressed.
    */
    esG4Siz_(state) = upWidth_(dst) * upHeight_(dst);
    upArSize_(dst)  = 0;

    return( state );
}                               /* end _InitializeG4State */

/*****************************************************************************
**  _DestroyG4State
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine frees all the dynamic storage associated with
**      the compression state pointer.
**
**  FORMAL PARAMETERS:
**
**      state  - Compression state pointer.
**
*****************************************************************************/
static EncodeG4StatePtr _DestroyG4State(state)
 EncodeG4StatePtr state;
{
    if( IsPointer_(state) )
	{
	if( IsPointer_(esG4Ccl_(state)) )
	       DdxFree_(esG4Ccl_(state));

	if( IsPointer_(esG4Rcl_(state)) )
	       DdxFree_(esG4Rcl_(state));

	if( IsPointer_(esG4Cvt_(state)) )
	    {
	    if( IsPointer_(upBase_(esG4Cvt_(state))) )
	       DdxFreeBits_(upBase_(esG4Cvt_(state)));
	    DdxFree_(esG4Cvt_(state));	    
	    }
	DdxFree_(state);
	}
    return( NULL );
}                               /* end _DestroyG4State */

/*****************************************************************************
**  _EncodeG4Process
**
**  FUNCTIONAL DESCRIPTION:
**
**      Used by the FAX encoding routines for Group 3/2D and Group 4.
**      Encodes a single scan line in Group 4 CCITT format, and creates
**      a reference changelist for use in the encoding of subsequent scans.
**
**      This version encodes the scanline without checking for end-of-buffer
**      conditions, assuming that the parent routine has checked that enough
**      room for the maximum size possible scanline is available.
**
**      Group 4 coding is always done by encoding the changes (black to white,
**      or vice-versa) with references to the changes on the previous line.
**      The image is assumed to start with an imaginary white line. Several
**      modes are used depending on how well the current line changes
**      correspond to the previous line. These are Horizontal mode and
**      Pass mode, used when the lines do not match well, and seven Vertical
**      modes, used when a change on the current line is within plus or
**      minus 3 pixels of the reference change.
**
**	The change positions in the current line are compared to the change
**	positions in the reference line, and the mode is identified. A
**	codeword identifying the appropriate mode is written to the encoded
**	output buffer. In Pass and Vertical modes, this is all that is done;
**	in Horizontal mode, FAX Group 3 length codewords are now added to
**	identify the change position. Both a black and a white set of Group 3
**	codewords are always written.
**
**	The logic of the program is concerned mainly with keeping track of
**	a variety of change positions denoted by a0, a0prime, a1, a2, b0, b1
**	and keeping proper position in the current and reference changelists
**	(ccl[] and rcl[] ).
**
**  FORMAL PARAMETERS:
**
**      state	- EncodeG4 state information.
**
************************************************************************/
static void _EncodeG4Process(state)
 EncodeG4StatePtr state;
{
   int a0 = -1, a0prime = -1, a1, b1, a2, b2;	    /* CCITT pointer names  */
   int run_length, working_run_length, i, curr_pos = 0, ridx = 1;
   int           *ccl = esG4Ccl_(state);
   int           *rcl = esG4Rcl_(state);
   int            pos = esG4Pos_(state);
   unsigned char *buf = esG4Buf_(state);
   unsigned char code_len, run_color = WHITE;
   unsigned int  code_word;
   short int     code_val;

   for( i = 1; i <= *ccl; )
      {
      run_length = ccl[i] - curr_pos;
      curr_pos   = ccl[i];
      a1 = a0prime + run_length + (a0prime < 0 ? 1 : 0);

      /* detect b1, b2                                                    */
      /* backup ridx to prevent a specific pass mode condition from       */
      /* going undetected                                                 */
      if( ridx > 1 && rcl[ridx-1] > a0prime )
          ridx--;
      while( rcl[ridx] <= a0prime || (ridx & 1) == run_color )
         ridx++;
      b1 = rcl[ridx];
      b2 = rcl[ridx+1];

      /* detect proper coding mode                                        */
      if( b2 < a1 )                       /* pass mode                    */
         {
         ADD_CODEWORD_(buf,pos,MiK_FAX2D_CODEWORD_PASS,MiK_FAX2D_CODELEN_PASS);
         a0prime = b2;

         /* must back up bit pointers to reclaim difference between       */
         /* the new value of a0 (a0prime) and a1                          */
         curr_pos -= a1 - a0prime;        /* cp = cp - (a1-a0')           */
         }
      else
         {
         if( abs(a1 - b1) <= 3 )          /* one of the vertical modes    */
            {
            switch( a1 - b1 )
               {
               case  0 :
                  ADD_CODEWORD_(buf,pos,MiK_FAX2D_CODEWORD_V0,
					MiK_FAX2D_CODELEN_V0);
                  break;
               case -1 :
                  ADD_CODEWORD_(buf,pos,MiK_FAX2D_CODEWORD_VL1,
					MiK_FAX2D_CODELEN_VL1);
                  break;
               case  1 :
                  ADD_CODEWORD_(buf,pos,MiK_FAX2D_CODEWORD_VR1,
					MiK_FAX2D_CODELEN_VR1);
                  break;
               case -2 :
                  ADD_CODEWORD_(buf,pos,MiK_FAX2D_CODEWORD_VL2,
					MiK_FAX2D_CODELEN_VL2);
                  break;
               case  2 :
                  ADD_CODEWORD_(buf,pos,MiK_FAX2D_CODEWORD_VR2,
					MiK_FAX2D_CODELEN_VR2);
                  break;
               case -3 :
                  ADD_CODEWORD_(buf,pos,MiK_FAX2D_CODEWORD_VL3,
					MiK_FAX2D_CODELEN_VL3);
                  break;
               case  3 :
                  ADD_CODEWORD_(buf,pos,MiK_FAX2D_CODEWORD_VR3,
					MiK_FAX2D_CODELEN_VR3);
                  break;
               }
            if( a0 < 0 )
               {
               a0 = 0;
               if( a0prime < 0 )
                   a0prime = 0;
               }
            run_color ^= 1;               /* change run color                */

            a0prime = a1;
            a0 = a0prime;
            }
         else                             /* horizontal mode                 */
            {
            ADD_CODEWORD_(buf,pos,MiK_FAX2D_CODEWORD_HORIZ,
				  MiK_FAX2D_CODELEN_HORIZ);
            /* update change list and index                                  */
            if( a0 < 0 )
               {
               a0 = 0;
               if( a0prime < 0 )
                   a0prime = 0;
               }
            i++;
            if( run_color == WHITE )
               {ENCODE_1D_WHITERUN_(buf,pos,run_length);}
            else
               {ENCODE_1D_BLACKRUN_(buf,pos,run_length);}

            /* detect a2; get next runlength                              */
            run_color ^= 1;
	    run_length = ccl[i] - curr_pos;
            curr_pos   = ccl[i];
            a2 = a1 + run_length;

            if( run_color == WHITE )
               {ENCODE_1D_WHITERUN_(buf,pos,run_length);}
            else
               {ENCODE_1D_BLACKRUN_(buf,pos,run_length);}

            run_color ^= 1;
            a0prime    = a2;
            a0 = a0prime;
            }
         }
      if( curr_pos == ccl[i] )
          i++;
      }				/* end for */
   /*
   **	Update number of bits used.
   */
   esG4Pos_(state) = pos;
}                               /* end _EncodeG4Process */
/* end module SmiEncodeG4 */
