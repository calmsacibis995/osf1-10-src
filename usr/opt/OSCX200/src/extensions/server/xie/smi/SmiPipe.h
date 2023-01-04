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
**	X Imaging Extensions
**      Sample Machine Independant DDX
**
**  ABSTRACT:
**
**      This is the common header file for all pipeline processing modules.
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
**      March 25, 1989
**
**  MODIFICATION HISTORY:
**
*****************************************************************************/
#ifndef	_SMIPIPE
#define _SMIPIPE

/*
**  Include files
*/

/* Fix a conflict that comes from the core X misc.h file */
#ifdef __alpha
#if defined(abs)
#undef abs
#endif
#endif
#if defined(fabs)
#undef fabs
#endif

#ifndef VXT
#include <math.h>
#else
#include <vxtmath.h>
#endif

#include <X.h>
#include <XieAppl.h>
#include <XieDdx.h>
#include <XieUdpDef.h>
#include <SmiMemMgt.h>

/*
**  Data Structures and Typedefs
*/
#ifndef externalref
#if (defined(VMS) && defined(VAXC))
#define externalref globalref
#else
#define externalref extern
#endif
#endif

#ifndef externaldef
#if (defined (VMS) && defined(VAXC))
#ifdef VXT
#define externaldef(psect) globaldef {"psect"}
#else
#define externaldef(psect) globaldef {"psect"} noshare
#endif

#else
#define externaldef(psect)
#endif
#endif
/*
**  Standard block header...begins all data structures to provide support for
**  linked lists, and to tag each data structure with a type and subtype. This
**  makes things easy to identify in memory.
*/
typedef struct _BHD {
    struct _BHD	*flink;
    struct _BHD *blink;
    short	 size;
    char	 type;
    char	 subtype;
} Bhd, *BhdPtr;

/*
**  A simple queue data structure with flink and blink
*/
typedef struct _Queue {
    struct _Queue *flink;
    struct _Queue *blink;
} Queue, *QueuePtr;

/*
**  Pipe Element Vector Flags
*/
typedef struct _PipeElementVectorFlags {
    unsigned    Pv_v_NoInput :   1;
    unsigned    reserved     :  31;
} PipeElementVectorFlags, *PipeElementVectorFlagsPtr;

#define		Pv_m_NoInput	    1

/*
**  Pipeline Element Vector Block
*/
typedef struct _PipeElementVector {
    Bhd			    bhd;
    short		    ctx_size;
    PipeElementVectorFlags  flags;
    int			    (*initialize)();
    int			    (*activate)();
    int			    (*flush)();
    int			    (*destroy)();
    int			    (*abort)();
} PipeElementVector, *PipeElementVectorPtr;

/*
**  Pipeline Head. 
**  This data structure is the root of the pipeline description.
*/
typedef struct _PipelineHead {
    Bhd				 bhd;
    struct _PipeElementCtxRec	*run_nxt;	/* next runnable element ctx*/
    unsigned			 pixel_count;	/* work count for yielding  */
    unsigned			 time_slice;    /* work limit for yielding  */
    Queue			 desc_cache;	/* queue of PipeDataRec	    */
    struct _PipelineHead	*optptr;	/* optimized pipe block ptr */
} PipelineHead, *Pipe;   

/*
**  Pipeline Element Context, Common Part. 
**  This struct defines the common part of the pipeline context data structure. 
**  All context structs contain this struct as the first part of the context.
*/
typedef struct _PipeElementCommonPart {
    Bhd			  bhd;
    PipeElementVectorPtr  vector;
    struct _PipeDrainRec *input_port;
    Pipe		  head;		/* Ptr back to head of this pipeline */
    int			  status;
} PipeElementCommonPart, *PipeElementCommonPartPtr;

/*
**  Generic Pipe Context Record
*/
typedef struct _PipeElementCtxRec {
    PipeElementCommonPart common;
} PipeElementCtxRec, *PipeElementCtxPtr;

/*
**  Pipe Element Vector Flags
*/
typedef struct _DataTypeMaskRec {
    unsigned	DtV_reserved_0	:   1;	/* Reserved			    */
    unsigned    DtV_BU		:   1;	/* Byte Unsigned                    */
    unsigned    DtV_WU		:   1;	/* Word Unsigned                    */
    unsigned    DtV_LU		:   1;	/* Longword Unsigned                */
    unsigned    DtV_F		:   1;	/* Single Precision Float           */
    unsigned    DtV_VU		:   1;	/* Unaligned Bitstream              */
    unsigned    DtV_V		:   1;	/* Aligned Bitstream                */
    unsigned    DtV_CL		:   1;  /* Change list                      */
    unsigned    reserved	:  24;	/* Reserved			    */
} DataTypeMaskRec, *DataTypeMaskPtr;

typedef unsigned short DataTypeMask;

#define DtM_BU (1<<UdpK_DTypeBU)		/* Byte Unsigned	    */
#define DtM_WU (1<<UdpK_DTypeWU)		/* Word Unsigned	    */
#define DtM_LU (1<<UdpK_DTypeLU)		/* Longword Unsigned	    */
#define DtM_F  (1<<UdpK_DTypeF)			/* Single Precision Float   */
#define DtM_VU (1<<UdpK_DTypeVU)		/* PCM Bitstream	    */
#define DtM_V  (1<<UdpK_DTypeV)			/* Compressed		    */
#define DtM_CL (1<<UdpK_DTypeCL)		/* Change List		    */
#define DtM_Any		(DtM_BU|DtM_WU|DtM_VU|DtM_F)
#define DtM_Constrained (DtM_BU|DtM_WU|DtM_VU)

/*
**  The data pipe port definition structure.
*/
typedef struct _PipePortRec {
    Bhd		    bhd;
    char	    data_type;
    DataTypeMask    data_type_mask;
    int		    quantum;
    Fcard32	    alignment;
    unsigned long  *align_dat;
    Queue	    array_cache;
} PipePortRec, *PipePortPtr;

/*
**  The data pipe Sink definition structure.
*/
typedef struct _PipeSinkRec {
    PipePortRec	    port;
    Pipe	    pipe_head;
    Queue	    drain_list;
    int		    max_quantum;
    int		    udp_final_mask;
    int		    reference_count[XieK_MaxComponents];
    UdpPtr	    udp[XieK_MaxComponents];
    struct {
	unsigned initialized    :  1;
	unsigned permanent	:  1;
	unsigned delete		:  1;
	unsigned write		:  1;
	unsigned full		:  1;
	unsigned reserved	: 27;
    } flags;
} PipeSinkRec, *PipeSinkPtr;

/*
**  The data drain definition structure.
*/
typedef struct _PipeDrainRec {
    PipePortRec	     port;
    PipeSinkPtr	     sink;
    int		     comp_mask;
    int		     comp_quantum[XieK_MaxComponents];
    Queue	     data[XieK_MaxComponents];
    struct {
	unsigned clog		:  1;
	unsigned convert	:  1;
	unsigned run_ok		:  1;
	unsigned reserved	: 29;
    } flags;
} PipeDrainRec, *PipeDrainPtr;

/*
**  PipeDataRec
**  Image data descriptor. This data structure describes image data that may
**  by placed onto pipeline element queues.
*/
typedef struct _PipeDataRec {
    Bhd			    bhd;
    PipePortPtr		    port;
    struct _PipeDataRec    *reference_ptr;
    int			    reference_count;
    UdpRec		    udp;
    struct {
	unsigned permanent	:  1;
	unsigned delete_pending	:  1;
	unsigned dont_cache	:  1;
	unsigned owned          :  1;
	unsigned reserved	: 28;
    } flags;	
} PipeDataRec, *PipeDataPtr;

/*
**  MACRO definitions
*/
    /*
    ** Block Head Access Macros
    */
#define BhdNxt_(bhd)	(((BhdPtr)(bhd))->flink)
#define BhdPrv_(bhd)	(((BhdPtr)(bhd))->blink)
#define BhdSiz_(bhd)	(((BhdPtr)(bhd))->size)
#define BhdTyp_(bhd)	(((BhdPtr)(bhd))->type)
#define BhdSubTyp_(bhd)	(((BhdPtr)(bhd))->subtype)

    /*
    ** Pipeline Head Access Macros
    */
#define PipeCtxNxt_(pipe)   ((PipeElementCtxPtr)(((Pipe)(pipe))->bhd.flink))
#define PipeCtxPrv_(pipe)   ((PipeElementCtxPtr)(((Pipe)(pipe))->bhd.blink))
#define PipeCtxRun_(pipe)   (((Pipe)(pipe))->run_nxt)
#define PipePxlCnt_(pipe)   (((Pipe)(pipe))->pixel_count)
#define PipeTimeSlice_(pipe) (((Pipe)(pipe))->time_slice)
#define PipeDsc_(pipe)	    ((PipeDataPtr)(&(((Pipe)(pipe))->desc_cache)))
#define PipeDscFlk_(pipe)   ((PipeDataPtr)(((Pipe)(pipe))->desc_cache.flink))
#define PipeDscBlk_(pipe)   ((PipeDataPtr)(((Pipe)(pipe))->desc_cache.blink))
#define PipeOptPtr_(pipe)   (((Pipe)(pipe))->optptr)
#define PipeOptFlk_(pipe)\
		  ((PipeElementCtxPtr)(((Pipe)(pipe))->optptr->bhd.flink))
#define PipeOptBlk_(pipe)\
		  ((PipeElementCtxPtr)(((Pipe)(pipe))->optptr->bhd.blink))

    /*
    **  Pipeline Element Context Access Macros
    */
#define CtxNxt_(ctx)\
	    ((PipeElementCtxPtr)(((PipeElementCtxPtr)(ctx))->common.bhd.flink))
#define CtxPrv_(ctx)\
	    ((PipeElementCtxPtr)(((PipeElementCtxPtr)(ctx))->common.bhd.blink))
#define CtxSiz_(ctx)	(((PipeElementCtxPtr)(ctx))->common.bhd.size)
#define CtxTyp_(ctx)	(((PipeElementCtxPtr)(ctx))->common.bhd.type)
#define CtxSubTyp_(ctx)	(((PipeElementCtxPtr)(ctx))->common.bhd.subtype)
#define CtxVecPtr_(ctx) (((PipeElementCtxPtr)(ctx))->common.vector)
#define CtxInp_(ctx)	(((PipeElementCtxPtr)(ctx))->common.input_port)
#define CtxHead_(ctx)   (((PipeElementCtxPtr)(ctx))->common.head)
#define CtxStatus_(ctx)	(((PipeElementCtxPtr)(ctx))->common.status)
#define CtxPxlCnt_(ctx) (PipePxlCnt_(((PipeElementCtxPtr)(ctx))->common.head))
#define CtxInit_(ctx)	(((PipeElementCtxPtr)(ctx))->common.vector->initialize)
#define CtxActivate_(ctx) (((PipeElementCtxPtr)(ctx))->common.vector->activate)
#define CtxFlush_(ctx)	(((PipeElementCtxPtr)(ctx))->common.vector->flush)
#define CtxDestroy_(ctx) (((PipeElementCtxPtr)(ctx))->common.vector->destroy)
#define CtxAbort_(ctx) (((PipeElementCtxPtr)(ctx))->common.vector->abort)

    /*
    **  Pipe element vector macros
    */
#define VecSubTyp_(vec)	    ((vec)->bhd.subtype)
#define VecCtxSiz_(vec)	    ((vec)->ctx_size)
#define VecFlags_(vec)	    ((vec)->flags)
#define VecNoInpt_(vec)	    ((vec)->flags.Pv_v_NoInput)
#define VecInit_(vec)	    ((vec)->initialize)
#define VecActive_(vec)	    ((vec)->activate)
#define VecDestroy_(vec)    ((vec)->destroy)

    /*
    **  Port access macros
    */
#define PrtFlk_(port)	    (((PipePortPtr)(port))->bhd.flink)
#define PrtBlk_(port)	    (((PipePortPtr)(port))->bhd.blink)
#define PrtTyp_(port)	    (((PipePortPtr)(port))->bhd.type)
#define PrtDtyp_(port)	    (((PipePortPtr)(port))->data_type)
#define PrtDtypMsk_(port)   (((PipePortPtr)(port))->data_type_mask)
#define PrtQnt_(port)	    (((PipePortPtr)(port))->quantum)
#define PrtAln_(port)	    (((PipePortPtr)(port))->alignment)
#define PrtAlnDat_(port)    (((PipePortPtr)(port))->align_dat)

#define PrtAryLst_(port)\
	    ((PipeDataPtr)(&(((PipePortPtr)(port))->array_cache)))
#define PrtAryNxt_(port)\
	      ((PipeDataPtr)(((PipePortPtr)(port))->array_cache.flink))
#define PrtAryPrv_(port)\
	      ((PipeDataPtr)(((PipePortPtr)(port))->array_cache.blink))

    /*
    **  Data Sink access macros.
    */
#define SnkNxt_(sink)	((PipeSinkPtr)(((PipeSinkPtr)(sink))->port.bhd.flink))
#define SnkPrv_(sink)	((PipeSinkPtr)(((PipeSinkPtr)(sink))->port.bhd.blink))
#define SnkTyp_(sink)		(((PipeSinkPtr)(sink))->port.bhd.type)
#define SnkSiz_(sink)		(((PipeSinkPtr)(sink))->port.bhd.size)
#define SnkDtyp_(sink)		(((PipeSinkPtr)(sink))->port.data_type)
#define SnkDtypMsk_(sink)	(((PipeSinkPtr)(sink))->port.data_type_mask)
#define SnkQnt_(sink)		(((PipeSinkPtr)(sink))->port.quantum)
#define	SnkAln_(sink)		(((PipeSinkPtr)(sink))->port.alignment)
#define SnkAlnDat_(sink)        (((PipeSinkPtr)(sink))->port.align_dat)
#define SnkPipe_(sink)		(((PipeSinkPtr)(sink))->pipe_head)
#define SnkDrnLst_(sink)\
		 ((PipeDrainPtr)(&(((PipeSinkPtr)(sink))->drain_list)))
#define SnkDrnFlk_(sink)\
		 ((PipeDrainPtr)(((PipeSinkPtr)(sink))->drain_list.flink))
#define SnkDrnBlk_(sink)\
		 ((PipeDrainPtr)(((PipeSinkPtr)(sink))->drain_list.blink))
#define SnkMaxQnt_(sink)	(((PipeSinkPtr)(sink))->max_quantum)
#define SnkUdpFinalMsk_(sink)	(((PipeSinkPtr)(sink))->udp_final_mask)
#define SnkRefCnt_(sink,comp)	(((PipeSinkPtr)(sink))->reference_count[(comp)])
#define	SnkUdp_(sink,comp)	(*((PipeSinkPtr)(sink))->udp[(comp)])
#define SnkUdpPtr_(sink,comp)	(((PipeSinkPtr)(sink))->udp[(comp)])
#define SnkPxlLen_(sink,comp)\
		    (((PipeSinkPtr)(sink))->udp[(comp)]->UdpW_PixelLength)
#define SnkDatTyp_(sink,comp)	(((PipeSinkPtr)(sink))->udp[(comp)]->UdpB_DType)
#define SnkDatCls_(sink,comp)	(((PipeSinkPtr)(sink))->udp[(comp)]->UdpB_Class)
#define SnkBase_(sink,comp)	(((PipeSinkPtr)(sink))->udp[(comp)]->UdpA_Base)
#define SnkArSize_(sink,comp)\
		    (((PipeSinkPtr)(sink))->udp[(comp)]->UdpL_ArSize)
#define SnkPxlStr_(sink,comp)\
		    (((PipeSinkPtr)(sink))->udp[(comp)]->UdpL_PxlStride)
#define SnkScnStr_(sink,comp)\
		    (((PipeSinkPtr)(sink))->udp[(comp)]->UdpL_ScnStride)
#define SnkX1_(sink,comp)	(((PipeSinkPtr)(sink))->udp[(comp)]->UdpL_X1)
#define SnkX2_(sink,comp)	(((PipeSinkPtr)(sink))->udp[(comp)]->UdpL_X2)
#define SnkY1_(sink,comp)	(((PipeSinkPtr)(sink))->udp[(comp)]->UdpL_Y1)
#define SnkY2_(sink,comp)	(((PipeSinkPtr)(sink))->udp[(comp)]->UdpL_Y2)
#define SnkWidth_(sink,comp)\
		    (((PipeSinkPtr)(sink))->udp[(comp)]->UdpL_PxlPerScn)
#define SnkHeight_(sink,comp)\
		    (((PipeSinkPtr)(sink))->udp[(comp)]->UdpL_ScnCnt)
#define SnkPos_(sink,comp)	(((PipeSinkPtr)(sink))->udp[(comp)]->UdpL_Pos)
#define SnkCmpIdx_(sink,comp)\
		    (((PipeSinkPtr)(sink))->udp[(comp)]->UdpL_CompIdx)
#define SnkLvl_(sink,comp)\
		    (((PipeSinkPtr)(sink))->udp[(comp)]->UdpL_Levels)
#define SnkInited_(sink)	(((PipeSinkPtr)(sink))->flags.initialized)
#define SnkDelete_(sink)	(((PipeSinkPtr)(sink))->flags.delete)
#define SnkFull_(sink)		(((PipeSinkPtr)(sink))->flags.full)
#define SnkWrite_(sink)		(((PipeSinkPtr)(sink))->flags.write)
#define SnkPrm_(sink)		(((PipeSinkPtr)(sink))->flags.permanent)

    /*
    **  Data Drain access macros.
    */
#define DrnNxt_(drain) ((PipeDrainPtr)(((PipeDrainPtr)(drain))->port.bhd.flink))
#define DrnPrv_(drain) ((PipeDrainPtr)(((PipeDrainPtr)(drain))->port.bhd.blink))
#define	DrnTyp_(drain)		(((PipeDrainPtr)(drain))->port.bhd.type)
#define DrnSiz_(drain)		(((PipeDrainPtr)(drain))->port.bhd.size)
#define	DrnQnt_(drain)		(((PipeDrainPtr)(drain))->port.quantum)
#define	DrnAln_(drain)		(((PipeDrainPtr)(drain))->port.alignment)
#define DrnAlnDat_(drain)       (((PipeDrainPtr)(drain))->port.align_dat)
#define DrnSnkPtr_(drain)	(((PipeDrainPtr)(drain))->sink)
#define DrnDtyp_(drain)		(((PipeDrainPtr)(drain))->port.data_type)
#define DrnDtypMsk_(drain)	(((PipeDrainPtr)(drain))->port.data_type_mask)
#define DrnCmpMsk_(drain)	(((PipeDrainPtr)(drain))->comp_mask)
#define DrnCmpQnt_(drain,i)	(((PipeDrainPtr)(drain))->comp_quantum[(i)]) 
#define DrnDat_(drain,i)\
		  ((PipeDataPtr)(&(((PipeDrainPtr)(drain))->data[(i)])))
#define DrnDatFlk_(drain,i)\
		  ((PipeDataPtr)(((PipeDrainPtr)(drain))->data[(i)].flink))
#define DrnDatBlk_(drain,i)\
		  ((PipeDataPtr)(((PipeDrainPtr)(drain))->data[(i)].blink))
#define DrnClog_(drain)		(((PipeDrainPtr)(drain))->flags.clog)
#define DrnConvert_(drain)	(((PipeDrainPtr)(drain))->flags.convert)
#define DrnRunOk_(drain)	(((PipeDrainPtr)(drain))->flags.run_ok)

    /*
    **  Image data access macros.
    */
#define DatNxt_(data)	((PipeDataPtr)(((PipeDataPtr)(data))->bhd.flink))
#define DatPrv_(data)	((PipeDataPtr)(((PipeDataPtr)(data))->bhd.blink))
#define DatTyp_(data)		(((PipeDataPtr)(data))->bhd.type)
#define DatSiz_(data)		(((PipeDataPtr)(data))->bhd.size)
#define DatPrt_(data)		(((PipeDataPtr)(data))->port)
#define DatRefCnt_(data)	(((PipeDataPtr)(data))->reference_count)
#define DatRefPtr_(data)	(((PipeDataPtr)(data))->reference_ptr)
#define DatUdp_(data)		(((PipeDataPtr)(data))->udp)
#define DatUdpPtr_(data)	(&((PipeDataPtr)(data))->udp)
#define DatPxlLen_(data)	(((PipeDataPtr)(data))->udp.UdpW_PixelLength)
#define DatDatCls_(data)	(((PipeDataPtr)(data))->udp.UdpB_Class)
#define DatDatTyp_(data)	(((PipeDataPtr)(data))->udp.UdpB_DType)
#define DatDatCls_(data)	(((PipeDataPtr)(data))->udp.UdpB_Class)
#define DatBase_(data)		(((PipeDataPtr)(data))->udp.UdpA_Base)
#define DatArSize_(data)	(((PipeDataPtr)(data))->udp.UdpL_ArSize)
#define DatPxlStr_(data)	(((PipeDataPtr)(data))->udp.UdpL_PxlStride)
#define DatScnStr_(data)	(((PipeDataPtr)(data))->udp.UdpL_ScnStride)
#define DatX1_(data)		(((PipeDataPtr)(data))->udp.UdpL_X1)
#define DatX2_(data)		(((PipeDataPtr)(data))->udp.UdpL_X2)
#define DatY1_(data)		(((PipeDataPtr)(data))->udp.UdpL_Y1)
#define DatY2_(data)		(((PipeDataPtr)(data))->udp.UdpL_Y2)
#define DatWidth_(data)		(((PipeDataPtr)(data))->udp.UdpL_PxlPerScn)
#define DatHeight_(data)	(((PipeDataPtr)(data))->udp.UdpL_ScnCnt)
#define DatPos_(data)		(((PipeDataPtr)(data))->udp.UdpL_Pos)
#define DatCmpIdx_(data)	(((PipeDataPtr)(data))->udp.UdpL_CompIdx)
#define DatLvl_(data)		(((PipeDataPtr)(data))->udp.UdpL_Levels)
#define DatPrm_(data)		(((PipeDataPtr)(data))->flags.permanent)
#define DatDelPnd_(data)	(((PipeDataPtr)(data))->flags.delete_pending)
#define DatNoCache_(data)	(((PipeDataPtr)(data))->flags.dont_cache)
#define DatOwned_(data)		(((PipeDataPtr)(data))->flags.owned)

    /*
    **  Queue operation macros
    */
#define IniQue_(head) \
   (((QueuePtr)(head))->flink = ((QueuePtr)(head)), \
    ((QueuePtr)(head))->blink = ((QueuePtr)(head)))

#define InsQue_(entry,pred) \
   (((QueuePtr)(entry))->flink = ((QueuePtr)(pred))->flink, \
    ((QueuePtr)(entry))->blink = ((QueuePtr)(pred)),\
    ((QueuePtr)(pred))->flink->blink = ((QueuePtr)(entry)),\
    ((QueuePtr)(pred))->flink = ((QueuePtr)(entry)))

#define RemQue_(entry) \
    (((QueuePtr)(entry))->blink->flink = ((QueuePtr)(entry))->flink, \
    ((QueuePtr)(entry))->flink->blink = ((QueuePtr)(entry))->blink, \
    ((QueuePtr)(entry)))

#define QueueEmpty_(head) (((QueuePtr)(head))->flink == (QueuePtr)(head))

    /*
    **  Equated Symbols
    */
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE  1
#endif

    /*
    **  List of block types
    **
    **	NOTE:	If you add to the element stype list, remember that other
    **		DDX'x have allocated stypes for their elements in their
    **		element associated .h files. The StypeLast is used as the
    **		base for these additions.
    */
#define TypePipeElementVector	1
#define TypePipeElementCtx	2
#define     StypeAreaElement			 1
#define     StypeAreaStatisticsElement           2
#define     StypeArithmeticElement		 3
#define     StypeChromeComElement		 4
#define     StypeChromeSepElement		 5
#define     StypeCompareElement			 6
#define     StypeConstrainElement		 7
#define	    StypeCropElement			 8
#define	    StypeDecodeDctElement		 9
#define	    StypeDecodeG4Element		10
#define	    StypeDitherElement			11
#define	    StypeEncodeDctElement		12
#define     StypeEncodeG4Element		13
#define	    StypeExportElement			14
#define     StypeFillElement                    15
#define     StypeGetStreamElement		16
#define     StypeLogicalElement			17
#define	    StypeLuminanceElement		18
#define     StypeMatchHistogramElement          19
#define     StypeMathElement			20
#define	    StypeMirrorElement			21
#define	    StypePointElement			22
#define     StypePointStatisticsElement         23
#define     StypePutStreamElement		24
#define	    StypeRotateElement			25
#define	    StypeScaleElement			26
#define     StypeTranslateElement		27
#define	    StypeLast				28
#define TypePipe		3
#define TypePipeData		4
#define TypePipeSink		5
#define TypePipeDrain		6

    /*
    **  Pipeline element Status Types
    */
#define StatusNew	1
#define StatusReady	2
#define StatusActive	3
#define StatusBlocked	4
#define StatusComplete	5


    /*
    **  Pipeline Entry Points
    */
extern int		    SmiAbortPipeline();
extern PipeElementCtxPtr    SmiCreatePipeCtx();
extern Pipe		    SmiCreatePipeline();
extern int		    SmiDestroyPipeline();
extern int		    SmiFlushPipeline();
extern int		    SmiInitiatePipeline();
extern int		    SmiPipeHasYielded();
extern int		    SmiResumePipeline();

    /*
    **  Include pipeline entry points from other modules.
    */
#include <SmiPipeRsm.h>
#include <SmiPipeSched.h>

#endif
/* _SMIPIPE */
