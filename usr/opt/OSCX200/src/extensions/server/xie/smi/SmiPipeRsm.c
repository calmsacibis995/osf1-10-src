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

/************************************************************************
**
**  FACILITY:
**
**      X Imaging Extension
**	Sample Machine Independant DDX
**
**  ABSTRACT:
**
**      This module contains resource manager utility routines
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
************************************************************************/

/*
**  Include files
*/
#include <stdio.h>

#include "SmiPipe.h"
#include <XieDdx.h>

/*
**  Table of contents
*/
PipeDataPtr	    RmAllocCmpDataDesc();
PipeDataPtr	    RmAllocCmpPermDesc();
PipeDataPtr	    RmAllocData();
int	    	    RmAllocDataArray();
PipeDataPtr	    RmAllocDataDesc();
PipeDataPtr	    RmAllocPermDesc();
PipeDataPtr	    RmAllocRefDesc();
char		    RmBestDType();
PipeDataPtr	    RmCreateDataDesc();
PipeDrainPtr	    RmCreateDrain();
PipeSinkPtr	    RmCreateSink();
PipeDataPtr	    RmDeallocData();
PipeDataPtr	    RmDeallocDataDesc();
PipeDrainPtr	    RmDestroyDrain();
PipeSinkPtr	    RmDestroySink();
PipeDataPtr	    RmGetData();
int		    RmGetDType();
int		    RmGetMaxQuantum();
int		    RmGetQuantum();
int		    RmHasQuantum();
int		    RmInitializePort();
PipeDataPtr	    RmMergeBuffers();
PipeDataPtr	    RmObtainQuantum();
int		    RmOwnData();
int		    RmPutData();
void		    RmSetAlignment();
void		    RmSetDType();
void		    RmSetQuantum();
PipeDataPtr	    RmSplitBuffers();

/*
**  MACRO definitions
*/
#define Min_(a,b) ((a) <= (b) ? (a) : (b))
#define Max_(a,b) ((a) >  (b) ? (a) : (b))
/*
**  Equated Symbols
*/

/*
**  External References
*/

/*
**	Local Storage
*/

/************************************************************************
**  RmAllocCmpDataDesc
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine allocates and initializes a pipe data descriptor
**      which can be used to describe an array of compressed data.
**
**  FORMAL PARAMETERS:
**
**      sink - sink which describes the data
**	plane - plane index for this segment
**      size - size in bits of the data segment
**
**  FUNCTION VALUE:
**
**      address of allocated data descriptor
**
************************************************************************/
PipeDataPtr  RmAllocCmpDataDesc( sink, plane, size )
 PipeSinkPtr sink;
 int	     plane;
 int	     size;

{
    PipeDataPtr data;

    data = DdxRmCreateDataDesc_( sink );
    if( data == NULL ) return( (PipeDataPtr) BadAlloc );
    /*
    **	Copy and init Udp
    */
    DatUdp_(data)    = SnkUdp_(sink,plane);
    DatY1_(data)     = 0;
    DatY2_(data)     = 0;
    DatHeight_(data) = 0;
    DatPos_(data)    = 0;
    DatBase_(data)   = NULL;
    DatDatTyp_(data) = SnkDtyp_(sink);

    DatArSize_(data) = size;

    /*
    **	Initialize remaining data fields
    */
    DatTyp_(data)     = TypePipeData;
    DatSiz_(data)     = sizeof(PipeDataRec);
    DatPrt_(data)     = (PipePortPtr) sink;
    DatRefPtr_(data)  = NULL;
    DatRefCnt_(data)  = 0;
    DatPrm_(data)     = FALSE;
    DatNoCache_(data) = TRUE;
    DatDelPnd_(data)  = FALSE;
    DatOwned_(data)   = FALSE;

    return( data );
}				    /* end of RmAllocCmpDataDesc */

/************************************************************************
**  RmAllocCmpPermDesc
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine allocates and initializes a pipe data descriptor
**      which points to data residing in a sink.
**
**  FORMAL PARAMETERS:
**
**      sink - sink which describes the data
**	plane - plane index for this segment
**      size - size in bits of the data segment
**
**  FUNCTION VALUE:
**
**      address of allocated data descriptor
**
************************************************************************/
PipeDataPtr  RmAllocCmpPermDesc( sink, plane, size )
 PipeSinkPtr sink;
 int	     plane;
 int	     size;

{
    PipeDataPtr data;

    data = DdxRmCreateDataDesc_( sink );
    if( data == NULL ) return( (PipeDataPtr) BadAlloc );
    /*
    **	Copy and init Udp
    */
    DatUdp_(data)    = SnkUdp_(sink,plane);
    DatY1_(data)     = 0;
    DatY2_(data)     = 0;
    DatHeight_(data) = 0;
    DatDatTyp_(data) = SnkDtyp_(sink);

    DatArSize_(data) = size;

    /*
    **	Initialize remaining data fields
    */
    DatTyp_(data)     = TypePipeData;
    DatSiz_(data)     = sizeof(PipeDataRec);
    DatPrt_(data)     = (PipePortPtr) sink;
    DatRefPtr_(data)  = NULL;
    DatRefCnt_(data)  = 0;
    DatPrm_(data)     = TRUE;
    DatNoCache_(data) = TRUE;
    DatDelPnd_(data)  = FALSE;
    DatOwned_(data)   = TRUE;
    SnkRefCnt_(sink,plane)++;

    return( data );
}				    /* end of RmAllocCmpPermDesc */

/************************************************************************
**  RmAllocData
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine allocates a pipe data descriptor and its associated
**      image data array.  The data must be uncompressed (PCM) format.
**
**      This is the routine which DDX modules should call to obtain
**      a destination data segment which they will later write to a
**      sink.
**
**  FORMAL PARAMETERS:
**
**      sink
**	comp
**	start
**	length
**
**  FUNCTION VALUE:
**
**      Pointer to Pipe Data Descriptor for allocated data.
**
************************************************************************/
PipeDataPtr  RmAllocData( sink, comp, start, length )
 PipeSinkPtr sink;
 int	     comp;
 int	     start;
 int	     length;
{
    PipeDataPtr data;
    int s;

    if( !SnkWrite_(sink)  || SnkDtyp_(sink) == UdpK_DTypeCL )
	data = DdxRmAllocDataDesc_( sink, comp, start, length );
    else
	data = DdxRmAllocPermDesc_( sink, comp, start, length );

    if( IsPointer_(data) && DatBase_(data) == NULL )
	{ 
	s = RmAllocDataArray( data );
	if (s != Success )
	    {
	    RmDeallocDataDesc( data );
	    data = (PipeDataPtr) s;
	    }
	else
	    {
	    if( DatPrm_(data) )
	        SnkBase_(sink,comp) = DatBase_(data);
	    else
		/* Since the cache is searched for exact fit, don't
		** bother caching segments that can't ever be used.
		*/
		DatNoCache_(data) =
		    (DatHeight_(data) > SnkHeight_(sink,comp)/2);
	    }
        }
    return( data );
}				    /* end of RmAllocData */

/************************************************************************
**  RmAllocDataArray
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine allocates an image data array and associates it with the
**      specified data segment.  If possible, the request is satisfied from
**      the array cache of the port associated with the data segment.
**
**  FORMAL PARAMETERS:
**
**      data
**
**  FUNCTION VALUE:
**
**      Returns standard X status codes.
**
************************************************************************/
int  RmAllocDataArray( data )
    PipeDataPtr data;
{
    PipeDataPtr cache;
    PipePortPtr port = DatPrt_(data);

    /*
    **  See if we've cached a suitable image array for this request.
    */
    for( cache = PrtAryNxt_(port);
	   cache != PrtAryLst_(port);
	   cache = DatNxt_(cache) )
	{
	if( DatArSize_(cache)  < DatArSize_(data) )
	    continue;
	if( DatArSize_(cache) == DatArSize_(data) )
	    {   /*
		**  Use the cached array for the image data, and move
		**  its descriptor to the pipeline's descriptor cache.
		*/
	     DatBase_(data) = DatBase_(cache);
	     RemQue_(cache);
	     RmDeallocDataDesc(cache);
	     }
	     break;
	}

    if( DatBase_(data) == NULL )
	/*
	**  Allocate a new array for the image data.
	*/
	DatBase_(data) = DdxMallocBits_(DatArSize_(data));

    if ( DatBase_(data) == NULL )
        return( BadAlloc );  
    else
        {
        DatOwned_(data) = TRUE;
        return( Success );
        }
}					/* end of RmAllocDataArray */

/************************************************************************
**  RmAllocDataDesc
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine allocates and initializes an image data descriptor
**      which will be used to hold uncompressed (PCM) data.
**
**  FORMAL PARAMETERS:
**
**      port
**	comp
**	start
**	length
**
**  FUNCTION VALUE:
**
**      address of allocated data descriptor
**
************************************************************************/
PipeDataPtr  RmAllocDataDesc( port, comp, start, length )
 PipePortPtr port;
 int	     comp;
 int	     start;
 int	     length;
{
    PipeDataPtr data;
    PipeSinkPtr sink = PrtTyp_(port) == TypePipeSink ? (PipeSinkPtr) port
						     :  DrnSnkPtr_(port);

    data = DdxRmCreateDataDesc_( sink );
    if( data == NULL ) return( (PipeDataPtr) BadAlloc );
    /*
    **	Copy and init Udp
    */
    DatUdp_(data)    = SnkUdp_(sink,comp);
    DatY1_(data)     = start;
    DatY2_(data)     = DatY1_(data) + length - 1;
    DatHeight_(data) = length;
    DatBase_(data)   = NULL;
    DatPos_(data)    = 0;

    if( DatDatTyp_(data) != PrtDtyp_(port) )
	{   /*
	    **	Arrange for correct DType.
	    */
	DatDatTyp_(data)  = PrtDtyp_(port);

	switch( DatDatTyp_(data) )
	    {
	case UdpK_DTypeBU:
	    DatPxlLen_(data) = 8;
	    DatDatCls_(data) = UdpK_ClassA;
	    break;
	case UdpK_DTypeWU:
	    DatPxlLen_(data) = 16;
	    DatDatCls_(data) = UdpK_ClassA;
	    break;
	case UdpK_DTypeLU:
	case UdpK_DTypeF:
	    DatPxlLen_(data) = 32;
	    DatDatCls_(data) = UdpK_ClassA;
	    break;
	case UdpK_DTypeV:
	case UdpK_DTypeVU:
	    DatPxlLen_(data) = BitsFromLevels_(DatLvl_(data));
	    DatDatCls_(data) = UdpK_ClassUBA;
	    break;
	case UdpK_DTypeCL:
	    DatPxlLen_(data) = 0;
	    DatDatCls_(data) = UdpK_ClassCL;
	    break;
	default:
    	    RmDeallocDataDesc( data );
	    return( (PipeDataPtr) BadMatch );
	    }
	DatPxlStr_(data) = DatPxlLen_(data);
	DatScnStr_(data) = DatDatCls_(data) == UdpK_ClassCL ? 0
			 : DatPxlStr_(data) * DatWidth_(data);
	}
    /*
    **	Calculate default ArSize for the described data.
    */
    DatArSize_(data) = DatDatCls_(data) == UdpK_ClassCL
	? DatHeight_(data) * (DatWidth_(data) + 2) * sizeof(unsigned int) * 8
	: DatHeight_(data) * DatScnStr_(data) + DatPos_(data);

    if( IsPointer_(PrtAln_(port)) && PrtAln_(port) != SnkAln_(sink) )
	/*
	**  Arrange for Ddx specific alignment.
	*/
	(*PrtAln_(port))(DatUdpPtr_(data), PrtAlnDat_(port));

    /*
    **	Initialize remaining data fields
    */
    DatTyp_(data)     = TypePipeData;
    DatSiz_(data)     = sizeof(PipeDataRec);
    DatPrt_(data)     = port;
    DatRefPtr_(data)  = NULL;
    DatRefCnt_(data)  = 0;
    DatPrm_(data)     = FALSE;
    DatNoCache_(data) = FALSE;
    DatDelPnd_(data)  = FALSE;
    DatOwned_(data) = TRUE;

    return( data );
}				    /* end of RmAllocDataDesc */

/************************************************************************
**  RmAllocPermDesc
**
**  FUNCTIONAL DESCRIPTION:
**
**     This routine allocates a pipe data descriptor which points to
**     an image data array associated with the sink.  Only uncompressed
**     data is supported.
**
**  FORMAL PARAMETERS:
**	sink - pointer to the sink which describes the data
**      comp - component index for this segment of data
**      start - starting scanline number for this segment
**      length - number of scanlines in this segment
**	
**  FUNCTION VALUE:
**
**      address of allocated data descriptor
**
************************************************************************/
PipeDataPtr  RmAllocPermDesc( sink, comp, start, length )
 PipeSinkPtr sink;
 int	     comp;
 int	     start;
 int	     length;
{
    PipeDataPtr	data;

    data = DdxRmCreateDataDesc_( sink );
    if( data == NULL ) return( (PipeDataPtr) BadAlloc );
    /*
    **	Copy and init Udp
    */
    DatUdp_(data)    = SnkUdp_(sink,comp);
    DatPos_(data)   += DatScnStr_(data) * (start - DatY1_(data));
    DatY1_(data)     = start;
    DatY2_(data)     = start + length - 1;
    DatHeight_(data) = length;
    /*
    **	Initialize remaining data fields.
    */
    DatTyp_(data)     = TypePipeData;
    DatSiz_(data)     = sizeof(PipeDataRec);
    DatPrt_(data)     = (PipePortPtr) sink;
    DatRefPtr_(data)  = NULL;
    DatRefCnt_(data)  = 0;
    DatPrm_(data)     = TRUE;
    DatNoCache_(data) = FALSE;
    DatDelPnd_(data)  = FALSE;
    DatOwned_(data) = TRUE;
    SnkRefCnt_(sink,comp)++;

    return( data );
}				    /* end of RmAllocPermDesc */

/************************************************************************
**  RmAllocRefDesc
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
**      Pointer to Image Data Descriptor for allocated data.
**
************************************************************************/
PipeDataPtr  RmAllocRefDesc( data, start, length )
 PipeDataPtr data;
 int	     start;
 int	     length;
{
    PipeDataPtr	refdat;
    PipePortPtr port = DatPrt_(data);
    PipeSinkPtr sink = PrtTyp_(port) == TypePipeSink ? (PipeSinkPtr) port
						     :  DrnSnkPtr_(port);
    refdat = DdxRmCreateDataDesc_( sink );
    if( refdat == NULL ) return( (PipeDataPtr) BadAlloc );
    /*
    **	Copy and init Udp
    */
    DatUdp_(refdat) = DatUdp_(data);
    if( DatDatTyp_(data) == UdpK_DTypeCL )
	DatPos_(refdat)  =
		DdxPosCl_(DatBase_(data), DatPos_(data), start - DatY1_(data));
    else
	DatPos_(refdat) += DatScnStr_(data) * (start - DatY1_(data));
    DatY1_(refdat)       = start;
    DatY2_(refdat)       = start + length - 1;
    DatHeight_(refdat)   = length;
    /*
    **	Initialize remaining data fields.
    */
    DatTyp_(refdat)      = TypePipeData;
    DatSiz_(refdat)      = sizeof(PipeDataRec);
    DatRefPtr_(refdat)   = data;
    DatRefCnt_(refdat)   = 0;
    DatPrt_(refdat)      = port;
    DatPrm_(refdat)      = DatPrm_(data);
    DatDelPnd_(refdat)   = FALSE;
    DatNoCache_(refdat)  = FALSE;
    DatOwned_(refdat)    = DatOwned_(data);

    DatRefCnt_(data)++;

    return( refdat );
}				    /* end of RmAllocRefDesc */

/************************************************************************
**  RmBestDType
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine selects the best datatype to be used for a sink
**      or drain, given a mask of possible choices and the number of
**      levels which needed to be represented.
**
**  FORMAL PARAMETERS:
**
**      mask - mask of supported dtypes
**	levels - number of levels in data to be stored
**
**  FUNCTION VALUE:
**
**      selected DType value
**
************************************************************************/
char	RmBestDType( mask, levels )
 int	mask;
 int    levels;
{
    int *list;
    int i;
    int best;

    /* The possible data types for each range of levels, in decreasing order
    ** of preference.
    */
    static int list_32[] = {UdpK_DTypeLU, UdpK_DTypeVU, 0};
    static int list_16[] = {UdpK_DTypeWU, UdpK_DTypeVU, UdpK_DTypeLU, 0};
    static int list_8[] = { UdpK_DTypeBU, UdpK_DTypeVU,
			       UdpK_DTypeWU, UdpK_DTypeLU, 0 };
    static int list_1[] = {UdpK_DTypeCL, UdpK_DTypeVU, UdpK_DTypeBU,
			       UdpK_DTypeWU, UdpK_DTypeLU, 0};

    best = 0;
    if( levels > 65536 )
	list = &list_32[0];
    else if( levels > 256 )
	list = &list_16[0];
    else if( levels > 2 )
	list = &list_8[0];
    else {
	list = &list_1[0];
	best = 1;		 /*  Changelists are only best if explicitly
				 ** requested. */
	}

    for( i = 0; list[i] != 0; i++ )
        if( mask & 1<<(list[i]) )
            return( list[i] );

    /*
    ** There is no suitable data type in the mask.
    ** Return the best choice for the number of levels.
    */
    return( list[best] );
}				    /* end of RmBestDType */

/************************************************************************
**  RmCreateDataDesc
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
**      pointer to created drain
**
************************************************************************/
PipeDataPtr	RmCreateDataDesc( sink )
 PipeSinkPtr	sink;
{
    PipeDataPtr	data;
    Pipe pipe = SnkPipe_(sink);

    if( !IsPointer_(pipe) || QueueEmpty_(PipeDsc_(pipe)) )
	    /*
	    **	Allocate a new data descriptor.
	    */
	data = (PipeDataPtr) DdxMalloc_(sizeof(PipeDataRec));
    else
	{   /*
	    **	Re-use a data descriptor from the pipeline's data cache.
	    */
	data = PipeDscFlk_(pipe);
	RemQue_(data);
	}

    return( data );
}				    /* end of RmCreateDataDesc */

/************************************************************************
**  RmCreateDrain
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**      data queue
**	quantum
**
**  FUNCTION VALUE:
**
**      pointer to created drain
**
************************************************************************/
PipeDrainPtr	RmCreateDrain( sink, cmask )
 PipeSinkPtr	sink;
 int		cmask;
{
    PipeDrainPtr pred;
    PipeDrainPtr drain;
    int comp;

    if( cmask == 0 ) return( (PipeDrainPtr) BadValue );

    drain = (PipeDrainPtr) DdxCalloc_(1,sizeof(PipeDrainRec));
    if( drain == NULL ) return( (PipeDrainPtr) BadAlloc );
    /*
    **	Attach to end of Drain queue.
    */
    pred = SnkDrnBlk_(sink);
    InsQue_(drain,pred);
    /*
    **	Init block head fields
    */
    DrnTyp_(drain) = TypePipeDrain;
    DrnSiz_(drain) = sizeof(PipeDrainRec);
    /*
    **	Init Queues
    */
    IniQue_(PrtAryLst_(drain));

    for( comp = 0;  comp < XieK_MaxComponents;  comp++ )
	IniQue_(DrnDat_(drain,comp));

    DrnAln_(drain)    = DdxAlignDefault_;
    DrnDtyp_(drain)   = UdpK_DTypeUndefined;
    DrnCmpMsk_(drain) = cmask & (1<<XieK_MaxComponents)-1;
    /*
    **	Owning sink.
    */
    DrnSnkPtr_(drain) = sink;

    return( drain );
}				    /* end of RmCreateDrain */

/************************************************************************
**  RmCreateSink
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**      sink
**
**  FUNCTION VALUE:
**
**      pointer to created sink
**
************************************************************************/
PipeSinkPtr RmCreateSink( insink )
 PipeSinkPtr insink;
{
    PipeSinkPtr	sink = insink;

    if( insink == NULL )
	{
	sink = (PipeSinkPtr) DdxCalloc_(1,sizeof(PipeSinkRec));
	if( sink == NULL ) return( (PipeSinkPtr) BadAlloc );
	}
    else
	memset( sink, 0, sizeof(PipeSinkRec) );
    /*
    **	Init block head fields
    */
    SnkTyp_(sink) = TypePipeSink;
    SnkSiz_(sink) = sizeof(PipeSinkRec);
    /*
    **	Init queues.
    */
    IniQue_(SnkDrnLst_(sink));
    IniQue_(PrtAryLst_(sink));

    return( sink );
}				    /* end of RmCreateSink */



/************************************************************************
**  RmDeallocData
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine deallocates the data descriptor and associated data.
**
**  FORMAL PARAMETERS:
**
**	data - image data descriptor to deallocate
**
**  FUNCTION VALUE:  NULL
**
************************************************************************/
PipeDataPtr  RmDeallocData( data )
 PipeDataPtr data;
{
    PipeDataPtr	ref;
    PipeDataPtr pred;
    PipePortPtr	port;
    PipeSinkPtr sink;
    int		comp;

    if( IsPointer_(data) && DatRefCnt_(data) == 0 )
	/*
	**  Got some kind of valid data descriptor ready to be freed.
	*/
	if( IsPointer_(DatRefPtr_(data)) )
	    {	/*
		**  This is a reference descriptor. Free the descriptor and
		**  update the reference count on the parent descriptor. If
		**  the parent descriptor reference count goes to zero and
		**  its been marked for deletion, get rid of it as well...
		*/
	    ref = DatRefPtr_(data);
	    if( --DatRefCnt_(ref) == 0 && DatDelPnd_(ref) )
		/*
		**  Delete referenced descriptor.
		*/
		DdxRmDeallocData_(ref);
	    DdxRmDeallocDataDesc_(data);
	    }
	else if( DatPrm_(data) )
	    {	/*
		**  This is a data descriptor which points to a large
		**  temporary sink buffer. Deallocate the data only if
		**  there are no more references to the sink buffer.
		*/
	    sink = (PipeSinkPtr) DatPrt_(data);	
	    comp = DatCmpIdx_(data);
	    if( --SnkRefCnt_(sink,comp) == 0 && !SnkPrm_(sink) )
		/*
		**  Free ephemeral data from Sink.
		*/
		SnkBase_(sink,comp) = DdxFreeBits_(SnkBase_(sink,comp));
	    DdxRmDeallocDataDesc_(data);
	    }
	else if( IsPointer_(DatBase_(data)) && DatOwned_(data) )
            {
            if( DatNoCache_(data) )
		{
		DdxFreeBits_(DatBase_(data));
		DdxRmDeallocDataDesc_(data);
		}
            else
		{   /*
		    **  This descriptor is a segment of ephemeral data.
		    **	Cache its array on the port from whence it came.
		    */
		port = DatPrt_(data);
		pred = PrtAryLst_(port);
		for( ref = PrtAryNxt_(port);
			ref != PrtAryLst_(port);
			    ref = DatNxt_(ref) ) {
		    if( DatArSize_(ref) >= DatArSize_(data) )
			break;
		    pred = ref;
		    }
		InsQue_(data,pred);
		}
	    }
	else
	    DdxRmDeallocDataDesc_(data);

    else if( IsPointer_(data) )
	/*
	**  A valid descriptor, but with outstanding references....
	*/
	DatDelPnd_(data) = TRUE;

    return( NULL );
}				    /* end of RmDeallocData */

/************************************************************************
**  RmDeallocDataDesc
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine deallocates an image data descriptor
**
**  FORMAL PARAMETERS:
**
**	data - image data descriptor to deallocate
**
**  FUNCTION VALUE:  NULL
**
************************************************************************/
PipeDataPtr  RmDeallocDataDesc( data )
 PipeDataPtr data;
{
    PipeDataPtr queue;
    PipePortPtr port = DatPrt_(data);
    PipeSinkPtr sink = PrtTyp_(port) == TypePipeSink ? (PipeSinkPtr) port
						     :  DrnSnkPtr_(port);
    Pipe	pipe = SnkPipe_(sink);

    if( IsPointer_(data) )
	if( IsPointer_(pipe) )
	    {
	    DatBase_(data)  = NULL;
	    queue = PipeDsc_(pipe);
	    InsQue_(data,queue);
	    }
	else
	    DdxFree_(data);

    return( NULL );
}				    /* end of RmDeallocDataDesc */

/************************************************************************
**  RmDestroyDrain
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**      data queue
**	quantum
**
**  FUNCTION VALUE:  NULL
**
************************************************************************/
PipeDrainPtr	RmDestroyDrain( drain )
 PipeDrainPtr	drain;
{
    PipeDataPtr	data;
    PipeSinkPtr sink;
    int comp;

    if( IsPointer_(drain) )
	{
	sink = DrnSnkPtr_(drain);
	/*
	**  Remove Drain from queue.
	*/
	RemQue_(drain);
	/*
	**  Free anything lingering on the data queues.
	*/
	for( comp = 0;  comp < XieK_MaxComponents;  comp++ )
	    for( data = DrnDatFlk_(drain,comp); 
		    data != DrnDat_(drain,comp); 
			data = DrnDatFlk_(drain,comp) )
		{
		RemQue_(data);
		DrnCmpQnt_(drain,DatCmpIdx_(data)) -= DatHeight_(data);
		DdxRmDeallocData_(data);
		}
	/*
	**  Free cached image array data descriptors.
	*/
	for( data = PrtAryNxt_(drain);
		data != PrtAryLst_(drain);
		    data = PrtAryNxt_(drain) )
	    {
	    RemQue_(data);
	    DdxFreeBits_(DatBase_(data));
	    DdxFree_(data);
	    }
	/*
	**  Free Drain structure.
	*/
	DdxCfree_(drain);
	/*
	**  If this is the last Drain on the Sink queue, and this
	**  Sink wants to be deleted, then do it now.
	*/
	if( QueueEmpty_(SnkDrnLst_(sink)) && SnkDelete_(sink) )
	    DdxRmDestroySink_(sink);
	}

    return( NULL );
}				    /* end of RmDestroyDrain */

/************************************************************************
**  RmDestroySink
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**      sink
**
**  FUNCTION VALUE:  NULL
**
************************************************************************/
PipeSinkPtr  RmDestroySink( sink )
 PipeSinkPtr sink;
{
    int		i;
    PipeDataPtr	data;

    if( IsPointer_(sink) )
	{   /*
	    **  Free cached image array data descriptors.
	    */
	for( data = PrtAryNxt_(sink);
		data != PrtAryLst_(sink);
		    data = PrtAryNxt_(sink) )
	    {
	    RemQue_(data);
	    DdxFreeBits_(DatBase_(data));
	    DdxFree_(data);
	    }
	/*
	**  Free the Udps and their image data arrays.
	*/
	for( i = 0;  i < XieK_MaxComponents;  i++ )
	    if( IsPointer_(SnkUdpPtr_(sink,i)) )
		{
		if( IsPointer_(SnkBase_(sink,i)) )
		  DdxFreeBits_(SnkBase_(sink,i));

		DdxFree_(SnkUdpPtr_(sink,i));
		}
	DdxCfree_(sink);
	}
    return( NULL );
}				    /* end of RmDestroySink */

/************************************************************************
**  RmGetData
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
**	pointer to data
**
************************************************************************/
PipeDataPtr	    RmGetData( ctx, drain )
 PipeElementCtxPtr  ctx;
 PipeDrainPtr	    drain;
{
    int		comp;
    PipeDataPtr	data;

    /*
    **	Remember this was the last Drain read from.
    */
    CtxInp_(ctx) = drain;
    /*
    **	If the scheduler says its OK, return a data buffer (if available).
    */
    if( DrnRunOk_(drain) || DdxSchedOkToRun_(ctx) )
	{
	data = DdxRmObtainQuantum_(drain);
	if( IsPointer_(data) )
	    {   /*
		**  Update pixel count and component quantum for this buffer.
		*/
	    CtxPxlCnt_(ctx) += DatWidth_(data) * DatHeight_(data)
		               * DatPxlLen_(data);
	    comp = DatCmpIdx_(data);
	    DrnCmpQnt_(drain,comp) -= DatHeight_(data);
	    }
	DrnRunOk_(drain) = FALSE;
	}
    else
	data = NULL;

    return( data );
}				    /* end of RmGetData */

/************************************************************************
**  RmGetDType
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**	port
**
**  FUNCTION VALUE:
**
**      value of DType
**
************************************************************************/
int		RmGetDType( port )
 PipePortPtr	port;
{
    return( PrtDtyp_(port) );
}				    /* end of RmGetDType */

/************************************************************************
**  RmGetQuantum
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
**      current quantum value
**
************************************************************************/
int	    RmGetQuantum( port )
 PipePortPtr port;
{
    return( PrtQnt_(port) );
}				    /* end of RmGetQuantum */

/************************************************************************
**  RmGetMaxQuantum
**
**  FUNCTIONAL DESCRIPTION:
**
**	Return the maximum quantum requested for any Drain attached to a Sink.
**	If that result is 0, return the Sink's quantum instead.
**	If that result is 0, return the Sink's height.
**
**  FORMAL PARAMETERS:
**
**  FUNCTION VALUE:
**
**      value of maximum quantum found
**
************************************************************************/
int	    RmGetMaxQuantum( sink )
 PipeSinkPtr sink;
{
    return( SnkMaxQnt_(sink) > 0 ? SnkMaxQnt_(sink) : SnkHeight_(sink,0) );
}				    /* end of RmGetMaxQuantum */

/************************************************************************
**  RmHasQuantum
**
**  FUNCTIONAL DESCRIPTION:
**
**     Examines the specified drain and returns the index of the component
**     or compressed data plane with enough data available to satisfy
**     the drain quantum.  If multiple components satisfy quantum, the
**     one with the most data available is returned.
**     
**
**  FORMAL PARAMETERS:
**
**      drain	- drain to test for quantum
**
**  FUNCTION VALUE:
**
**	index of component which as quantum (or XieK_MaxComponents if none do)
**
************************************************************************/
int		RmHasQuantum( drain )
 PipeDrainPtr	drain;
{
    int i, mask, available = 0, component = XieK_MaxComponents,
			          quantum = abs(DrnQnt_(drain));

    if( DrnDtyp_(drain) != UdpK_DTypeV ) 
	{
	for( i = 0, mask = DrnCmpMsk_(drain);  mask;  i++, mask >>= 1 )
	    /*
	    **  Choose the component with the largest quantum available.
	    */
	    if( mask & 1 && DrnCmpQnt_(drain,i) >  available
			 && DrnCmpQnt_(drain,i) >= quantum ) 
		{
		component = i;
		available = DrnCmpQnt_(drain,i);
		}
	}
    else
	for( i =0, mask = DrnCmpMsk_(drain); mask; i++, mask >>= 1 )
	    /*
	    ** Choose the compressed data plane with data available.
	    ** Only two quantum values are allowed: 0 and SnkHeight
	    ** (ie. the whole image)
	    */
	    if( mask & 1     && !QueueEmpty_(DrnDat_(drain,i)) &&
	      ( quantum == 0 ||  SnkUdpFinalMsk_(DrnSnkPtr_(drain)) & 1<<i ) )
		component = i;

    DrnRunOk_(drain) = component < XieK_MaxComponents;
    
    return( component );
}				    /* end of RmHasQuantum */

/************************************************************************
**  RmInitializePort
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**	pipe	-
**	port	-
**
**  FUNCTION VALUE:
**
**      status
**
************************************************************************/
int RmInitializePort( pipe, port )
 Pipe		pipe;
 PipePortPtr    port;
{
    int i, comps, dt_and, dt_or, in_use, status = Success;
    UdpRec	  drain_udp;
    PipeDataPtr	  data, queue;
    PipeDrainPtr  drain;
    int maxlvl;
    
    PipeSinkPtr	  sink = PrtTyp_(port) == TypePipeSink ? (PipeSinkPtr) port
						       :  DrnSnkPtr_(port);
    if( !SnkInited_(sink) )
	{
	/*
	**  See if the Sink is "in use" (ie. it already contains some data).
	**  Also determine the max number of levels required by any component.
	*/
	maxlvl = 0;
	in_use = FALSE;
	for( comps = 0;
	      comps < XieK_MaxComponents && IsPointer_(SnkUdpPtr_(sink,comps));
	      comps++ )
	    {
	    in_use |= IsPointer_(SnkBase_(sink,comps));
	    maxlvl = Max_(maxlvl, SnkLvl_(sink,comps));
	    }

	SnkInited_(sink) = TRUE;
	SnkPipe_(sink)	 = pipe;
	SnkWrite_(sink)  = SnkPrm_(sink) || !SnkFull_(sink) &&
			  (in_use || SnkMaxQnt_(sink) == SnkHeight_(sink,0));
	/*
	**  Establish the DType for the Sink.
	*/
	if( SnkDtyp_(sink) == UdpK_DTypeUndefined )
	    if( in_use )
		SnkDtyp_(sink) = SnkDatTyp_(sink,0);
	    else
		{   /*
		    **  Calculate Sink/Drains DType intersection and union.
		    */
		for( dt_and = SnkDtypMsk_(sink), dt_or  = 0,
		     drain  = SnkDrnFlk_(sink);
			drain != SnkDrnLst_(sink);
			    drain = DrnNxt_(drain) )
		    {
		    dt_and &= DrnDtyp_(drain) == UdpK_DTypeUndefined
			   ?  DrnDtypMsk_(drain) : 1<<DrnDtyp_(drain);
		    dt_or  |= DrnDtyp_(drain) == UdpK_DTypeUndefined
			   ?  DrnDtypMsk_(drain) : 1<<DrnDtyp_(drain);
		    }
		dt_or &= SnkDtypMsk_(sink);
		/*
		**  Make the "best" choice for the Sink's DType from:
		**	- the Sink & Drains DType intersection,
		**	- the Sink | Drains DType union,
		**	- the Sink's requirement.
		*/
		SnkDtyp_(sink) = dt_and ?
		                     DdxRmBestDType_(dt_and, maxlvl)
			       : dt_or  ? DdxRmBestDType_(dt_or, maxlvl)
			       : DdxRmBestDType_(SnkDtypMsk_(sink), maxlvl);
		}

	if( SnkDtyp_(sink) != UdpK_DTypeCL && SnkDtyp_(sink) != UdpK_DTypeV )
	    for( i = 0;  i < comps;  i++ )
		if( SnkDtyp_(sink) != SnkDatTyp_(sink,i) )
		    {	/*
			**  Make the Sink match the DType we'll be generating.
			*/
		    data = DdxRmAllocDataDesc_(sink, i, 0, SnkHeight_(sink,i));
		    if( !IsPointer_(data) ) return( (int) data );
		    SnkUdp_(sink,i) = DatUdp_(data);
		    DdxRmDeallocDataDesc_( data );
		    }
	/*
	**  Establish the DType and alignment requirements for each Drain.
	*/
	for( drain = SnkDrnFlk_(sink);
		drain != SnkDrnLst_(sink);
		    drain = DrnNxt_(drain) )
	    {
	    if( DrnDtypMsk_(drain) != 1<<DrnDtyp_(drain) )
		/*
		**  Since this is a friendly Drain (ie. supports multi-DTypes)
		**  make the "best" choice for the Drain's DType from:
		**	- the Sink's DType,
		**	- the Drain's preferred DType,
		**	- the "best" choice from the Drain's DType mask.
		*/
		DrnDtyp_(drain) = DrnDtypMsk_(drain) & 1<<SnkDtyp_(sink)
				? SnkDtyp_(sink)
				: DrnDtyp_(drain) != UdpK_DTypeUndefined
				? DrnDtyp_(drain)
				: DdxRmBestDType_(DrnDtypMsk_(drain),maxlvl);

	    if( !in_use &&  SnkDatTyp_(sink,0) == DrnDtyp_(drain)
			&& !IsPointer_(SnkAln_(sink))
			&&  IsPointer_(DrnAln_(drain)) )
		{   /*
		    **  Make Sink comply with Drain's alignment requirements.
		    */
		SnkAln_(sink)    = DrnAln_(drain);
                SnkAlnDat_(sink) = DrnAlnDat_(drain);
		for( i = 0;  i < comps;  i++ )
		    (*SnkAln_(sink))(SnkUdpPtr_(sink,i), SnkAlnDat_(sink));
		}
	    /*
	    **	See if data conversion is needed between the Sink and Drain.
	    */
	    DrnConvert_(drain) = DrnDtyp_(drain) != SnkDtyp_(sink);

	    if( !DrnConvert_(drain) && IsPointer_(DrnAln_(drain))
				    && DrnAln_(drain) != SnkAln_(sink) )
		/*
		**  See if the Drain's alignment function produces
		**  results that are compatible with the Sink.
		*/
		for( i = 0;  i < comps && !DrnConvert_(drain);  i++ )
		    if( 1<<i & DrnCmpMsk_(drain) )
			{
			drain_udp = SnkUdp_(sink,i);
			(*DrnAln_(drain))(&drain_udp, DrnAlnDat_(drain));
			DrnConvert_(drain) |=
				    SnkArSize_(sink,i) != urArSize_(drain_udp)
				 || SnkScnStr_(sink,i) != urScnStr_(drain_udp)
				 || SnkPxlStr_(sink,i) != urPxlStr_(drain_udp);
			}

	    /*
	    ** If either port contains compressed data and a conversion
	    ** is necessary, something is very confused.
	    */
	    if (DrnDtyp_(drain) == UdpK_DTypeV ||
		SnkDtyp_(sink) == UdpK_DTypeV )
	      for( i = 0; i < comps; i++)
	        if (DrnConvert_(drain) )
		  return( BadImplementation );

	    /*
	    **  Determine if data can "back up" in the Sink for this Drain.
	    */
	    DrnClog_(drain) = DrnQnt_(drain)  == SnkHeight_(sink,0)
			   && DrnDtyp_(drain) != UdpK_DTypeCL;
	    if( DrnClog_(drain) )
		for( i = 0; i < comps; i++ )
		    if( 1<<i & DrnCmpMsk_(drain) )
			{   /*
			    ** Making a reference here prevents the sink's
			    ** buffers from being deallocated if the sink isn't
			    ** permanent.  When the sink is full, RmPutData
			    ** will put the data directly into this descriptor.
			    */
			data = SnkDtyp_(sink) == UdpK_DTypeV
			     ? DdxRmAllocCmpPermDesc_(sink,i,SnkArSize_(sink,i))
			     : DdxRmAllocPermDesc_(sink,i,0,DrnQnt_(drain));
			if( !IsPointer_(data) ) return( (int) data );
			queue = DrnDatBlk_(drain,i);
			InsQue_(data,queue);
			}
	    }

	if( SnkFull_(sink) )
	    for( i = 0;  i < comps && status == Success;  i++ )
		{   /*
		    **  Place the full image on the Drains.
		    */
		data = SnkDtyp_(sink) == UdpK_DTypeV
		     ? DdxRmAllocCmpPermDesc_( sink, i, SnkArSize_(sink,i) )
		     : DdxRmAllocPermDesc_( sink, i, SnkY1_(sink,i),
						 SnkHeight_(sink,i) );
		status = IsPointer_(data) ? DdxRmPutData_( sink, data )
					  : (int) data;
		}
	}
    return( status );
}				    /* end of RmInitializePort */

/************************************************************************
**  RmMergeBuffers
**
**  FUNCTIONAL DESCRIPTION:
**	Merges and splits data segments on the drain to obtain one of
**      the correct size to satisfy quantum.
**
**      This routine supports only uncompressed data.
**
**  FORMAL PARAMETERS:
**
**      drain
**      src_data
**	quantum
**
**  FUNCTION VALUE:
**
**      pointer to merged data
**
************************************************************************/
PipeDataPtr	    RmMergeBuffers( drain, src_data, quantum )
 PipeDrainPtr	    drain;
 PipeDataPtr	    src_data;
 int		    quantum;
{
    PipeDataPtr	quantum_data = NULL;
    PipeDataPtr	  next, data = src_data;
    PipeSinkPtr		sink = DrnSnkPtr_(drain);
    int remaining, count, status, comp = DatCmpIdx_(data), y1 = DatY1_(data);
	    
    if( SnkWrite_(sink) )
	{
	if( DrnConvert_(drain) || DrnDtyp_(drain) == UdpK_DTypeCL )
	    {
	    quantum_data = DdxRmAllocDataDesc_( drain, comp, y1, quantum );
	    if( IsPointer_(quantum_data) )
		{
		status = DdxRmAllocDataArray_( quantum_data );
		if( status == Success )
		  status = DdxConvert_( SnkUdpPtr_(sink,comp),
				      DatUdpPtr_(quantum_data), XieK_MoveMode );
		if( status != Success )
		    {
		    DdxRmDeallocData_(quantum_data);
		    quantum_data = (PipeDataPtr) status;
		    }
		}
	    }
	else
	    quantum_data = DdxRmAllocPermDesc_( sink, comp, y1, quantum );

	if( IsPointer_(quantum_data) )
	    for( remaining = quantum;  remaining > 0;  remaining -= count )
		{
		if( DatHeight_(data) > remaining )
		    {
		    data = DdxRmSplitBuffers_( drain, data, remaining );
		    next = data;
		    if( !IsPointer_(data) ) break;
		    }
		else
		    {
		    next = DatNxt_(data);
		    RemQue_(data);
		    }
		count = DatHeight_(data);
		DdxRmDeallocData_(data);
		data = next;
		}
	}
    else
	{
	quantum_data = DdxRmAllocDataDesc_( drain, comp, y1, quantum );
	if( IsPointer_(quantum_data) )
            {
	    status = DdxRmAllocDataArray_( quantum_data );
	    if( status != Success )
	      {
	      DdxRmDeallocData_(quantum_data);
	      quantum_data = (PipeDataPtr) status;
	      }
	    }

	if( IsPointer_(quantum_data) )
	    for( remaining = quantum; remaining > 0; remaining -= count )
		{
		if( DatHeight_(data) > remaining )
		    {
		    data = DdxRmSplitBuffers_( drain, data, remaining );
		    next = data;
		    if( !IsPointer_(data) ) break;
		    }
		else
		    {
		    next = DatNxt_(data);
		    RemQue_(data);
		    }
		count  = DatHeight_(data);
		status = DdxConvert_( DatUdpPtr_(data),
				      DatUdpPtr_(quantum_data), XieK_MoveMode );
		if( status != Success )
		    {
		    DdxRmDeallocData_(quantum_data);
		    quantum_data = (PipeDataPtr) status;
		    }
		else
		    {
		    DdxRmDeallocData_(data);
		    data = next;
		    }
		}
        }

    if( !IsPointer_(quantum_data) || !IsPointer_(data) )
	{
	if( IsPointer_(quantum_data) )
	    {
	    DdxRmDeallocData_(quantum_data);
	    quantum_data = data;
	    }
	if( IsPointer_(data) )
	    DdxRmDeallocData_(data);
	}
    return( quantum_data );
}				    /* end of RmMergeBuffers */

/************************************************************************
**  RmObtainQuantum
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine extracts a quantum of data from the drain data queue.
**
**  FORMAL PARAMETERS:
**
**      drain
**
**  FUNCTION VALUE:
**
**      pointer to the data
**
************************************************************************/
PipeDataPtr	RmObtainQuantum( drain )
 PipeDrainPtr	drain;
{
    PipeDataPtr new, data;
    int	     comp, status;

    if( !SnkInited_(DrnSnkPtr_(drain)) )
	return( NULL );
    /*
    **	Determine which component to use next.
    */
    comp = DdxRmHasQuantum_( drain );
    if( comp == XieK_MaxComponents )
	return( NULL );				/* sorry, try again later   */

    /*
    **	Beginning of the data queue for this component.
    */
    data = DrnDatFlk_(drain,comp);
    /*
    **	If the Drain quantum is zero, or the first data element has 
    **	correct quantum, no need to jumble stuff around.  Never have to
    **  split/merge for compressed data.
    */
    if( DrnQnt_(drain) == 0
       || DatHeight_(data) == DrnQnt_(drain)
       || DrnQnt_(drain) <  0 && DatHeight_(data) == DrnCmpQnt_(drain,comp)
       || DrnDtyp_(drain) == UdpK_DTypeV )
	{
	RemQue_(data);
	if( DrnConvert_(drain) )
	    {	/*
		**  The DType or alignment doesn't match what the Drain wants.
		*/
	    new = DdxRmAllocDataDesc_( drain, DatCmpIdx_(data),
				       DatY1_(data), DatHeight_(data) );
	    if( IsPointer_(new) )
		{
		status = DdxRmAllocDataArray_( new );
		if ( status == Success )
		    status = DdxConvert_( DatUdpPtr_(data),
				      DatUdpPtr_(new), XieK_MoveMode );
		if( status != Success )
		    {
		    DdxRmDeallocData_(new);
		    new = (PipeDataPtr) status;
		    }
		}
	    DdxRmDeallocData_(data);
	    data = new;
	    }
	}
    else if( DrnQnt_(drain) < 0 )
	/*
	**  Merge together all buffers for this component.
	*/
	data = DdxRmMergeBuffers_( drain, data, DrnCmpQnt_(drain,comp) );

    else if( DatHeight_(data) < DrnQnt_(drain) )
	/*
	**  Merge together enough buffers to satisfy quantum for this component.
	*/
	data = DdxRmMergeBuffers_( drain, data, DrnQnt_(drain) );

    else
	/*
	**  Extract a quantum of data from this buffer leaving the rest behind.
	*/
	data = DdxRmSplitBuffers_( drain, data, DrnQnt_(drain) );

    return( data );
}				    /* end of RmObtainQuantum */

/************************************************************************
**  RmOwnData
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine replaces references to a specified data arrays
**      on the drains associated with a sink.  The contents of the
**      array is copied to storage that is known to be owned by
**      XIE.
**
**  FORMAL PARAMETERS:
**
**      sink - sink to be searched
**	wire - address of the array to replace.
**      comp - component/plane index of the data buffer
**
**  FUNCTION VALUE:
**
**      status
**
************************************************************************/
int	    RmOwnData( sink, wire, comp )
 PipeSinkPtr sink;
 unsigned char *wire;
 int comp;
{
    PipeDrainPtr drain;
    PipeDataPtr  data;
    unsigned char *owned = NULL;
    
    if (!QueueEmpty_(SnkDrnLst_(sink)) )
	for( drain = SnkDrnFlk_(sink); drain != SnkDrnLst_(sink);
	    drain = DrnNxt_(drain))
	    {
	    if (1<<comp & DrnCmpMsk_(drain))
		{
		if (!QueueEmpty_(DrnDat_(drain,comp)))
		    {
		    for( data = DrnDatFlk_(drain, comp);
			data != DrnDat_(drain, comp);
			data = DatNxt_(data))
			{
			if (DatBase_(data) == wire)
			    {
			    if( owned == NULL )
				{
				owned = (unsigned char *)
				    DdxMallocBits_( DatArSize_(data) );
				if( owned == NULL ) return( BadAlloc );
				memcpy( owned, DatBase_(data),
				       DatArSize_(data)>>3 );
				}
			    DatBase_(data) = owned;
			    DatOwned_(data) = TRUE;
			    }
			}
		    }
		}
	    }
    return( Success );
}

/************************************************************************
**  RmPutData
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine puts data on the pipeline element's output port
**
**  FORMAL PARAMETERS:
**
**      sink
**	data - image data descriptor
**
**  FUNCTION VALUE:
**
**      status
**
************************************************************************/
int	    RmPutData( sink, data )
 PipeSinkPtr sink;
 PipeDataPtr data;
{
    PipeDrainPtr drain;
    PipeDataPtr	 copy, queue;
    UdpRec	 src;
    int pos, status = Success, reference = 0, comp = DatCmpIdx_(data);

    if( SnkWrite_(sink) )
	/*
	**  Make sure data in the Sink reflects this request.
	*/
	if( SnkBase_(sink,comp) != DatBase_(data) )
	    {   /*
		**  Copy external data into the Sink.
		*/
	    if( DatDatTyp_(data) != UdpK_DTypeV )
	        {
		status = DdxConvert_( DatUdpPtr_(data),
				      SnkUdpPtr_(sink,comp), XieK_MoveMode );
	        if( status == Success  &&  DatDatTyp_(data) != UdpK_DTypeCL )
		    {   /*
		        **  Redirect data to be referenced from the Sink.
		        */
		    copy = DdxRmAllocPermDesc_( sink, comp, DatY1_(data),
							DatHeight_(data) );
		    if( IsPointer_(copy) )
		        {
		        DdxRmDeallocData_(data);
		        data = copy;
		        }
		    else
		        status = (int) copy;
		    }
	        }
	    else
	        {   /*
		    **	Append new compressed data to the Sink.  ArSize records
		    **	how much data is present (ie. the current end of data).
		    */
		pos = SnkPos_(sink,comp) + SnkArSize_(sink,comp);
		SnkArSize_(sink,comp)   += DatArSize_(data);
		SnkBase_(sink,comp) = SnkBase_(sink,comp) != NULL
		  ? DdxReallocBits_(SnkBase_(sink,comp),SnkArSize_(sink,comp))
		  : DdxMallocBits_(SnkArSize_(sink,comp));
		if( SnkBase_(sink,comp) == NULL )
		    status = BadAlloc;
		else
		    memcpy( SnkBase_(sink,comp) + (pos>>3),
			    DatBase_(data), DatArSize_(data)>>3 );

		if( status == Success )
	            {
		    copy = DdxRmAllocCmpPermDesc_( sink, comp,
						   SnkArSize_(sink,comp) );
		    if( IsPointer_(copy) )
		        {
		        DatPos_(copy) = pos;
		        DdxRmDeallocData_(data);
		        data = copy;
		        }
		    else
		        status = (int) copy;
		    }
		}
	    }
	else if( IsPointer_(DatRefPtr_(data)) )
	    {	/*
		**  If this reference descriptor is pointing back at preceeding 
		**  scanlines, copy them to the new location within the Sink.
		*/
	    pos = SnkPos_(sink,comp) + DatScnStr_(data) * DatY1_(data);
	    if( pos > DatPos_(data) )
		{
		src = DatUdp_(data);
		DatPos_(data) = pos;
		status = DdxConvert_( &src, DatUdpPtr_(data), XieK_MoveMode );
		}
	    }
    if( DatY2_(data) == SnkY2_(sink,comp) )
	/*
	**  We've reached the last scanline -- set final for the associated Udp.
	*/
	SnkUdpFinalMsk_(sink) |= 1<<comp;
    /*
    **	Hand this data over to the interested Drains.
    */
    for( drain = SnkDrnFlk_(sink);
	    drain != SnkDrnLst_(sink) && status == Success;
		drain = DrnNxt_(drain) )
	{
	if( !(DrnCmpMsk_(drain) & 1<<comp) )
	    continue;	/* This Drain is not interested in this component   */

	if( !DrnClog_(drain) )
	    {	/*
		**  Give this Drain this data segment.
		*/
	    copy = reference++ == 0 ? data
		 : DdxRmAllocRefDesc_(data, DatY1_(data), DatHeight_(data));
	    if( IsPointer_(copy) )
		{
		queue = DrnDatBlk_(drain,comp);
		InsQue_(copy,queue);
		DrnCmpQnt_(drain,comp) += DatHeight_(copy);
		}
	    else
		status = (int) copy;
	    }
	else if( SnkUdpFinalMsk_(sink) & 1<<comp )
	    {   /*
		**  If all the data is here make the descriptor which was
		**  put on the drain by RmInitializePort reference the 
		**  completed Udp from the Sink.
		*/
	    copy = DrnDatFlk_(drain,comp);
	    DatBase_(copy) = SnkBase_(sink,comp);
	    DatArSize_(copy) = SnkArSize_(sink,comp);
	    if( DrnDtyp_(drain) != UdpK_DTypeV )
	        DrnCmpQnt_(drain,comp) = SnkHeight_(sink,comp);
	    }
	}
    if( reference == 0 )
	DdxRmDeallocData_(data);    /* No Drain was given the original data */

    return( status );
}				    /* end of RmPutData */


/************************************************************************
**  RmSetAlignment
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**      port	   -
**	align	   -  pointer to alignment function
**	align_dat  -  pointer to alignment function private data structure
**
************************************************************************/
void		RmSetAlignment( port, align, align_dat )
 PipePortPtr	port;
 Fcard32	align;
 unsigned long *align_dat;
{
    PrtAln_(port)    = align;
    PrtAlnDat_(port) = align_dat;
}				    /* end of RmSetAlignment */


/************************************************************************
**  RmSetDType
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**      data queue
**	quantum
**
************************************************************************/
void		RmSetDType( port, dtype, dtype_mask )
 PipePortPtr	port;
 int		dtype;
 DataTypeMask	dtype_mask;
{
    PrtDtyp_(port)    = dtype;
    PrtDtypMsk_(port) = dtype_mask;
}				    /* end of RmSetDType */

/************************************************************************
**  RmSetQuantum
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**      data queue
**	quantum
**
************************************************************************/
void	    RmSetQuantum( port, quantum )
 PipePortPtr port;
 int	     quantum;
{
    int max_quantum = 0;
    PipeDrainPtr  drain;
    PipeSinkPtr	   sink = PrtTyp_(port) == TypePipeSink ? (PipeSinkPtr) port
							:  DrnSnkPtr_(port);
    /*
    **	Stash the new quantum.
    */
    PrtQnt_(port) = quantum;
    /*
    **	Search through the Drains to find the maximum quantum requested.
    */
    for( drain = SnkDrnFlk_(sink);
	    drain != SnkDrnLst_(sink);
		drain = DrnNxt_(drain) )
	if( max_quantum < abs( DrnQnt_(drain)) )
	    max_quantum = abs( DrnQnt_(drain));
    /*
    **  Remember the largest quantum required by any Drain
    **      -- or the Sink if the Drains don't care.
    */
    SnkMaxQnt_(sink) = max_quantum > 0 ? max_quantum : SnkQnt_(sink);
}				    /* end of RmSetQuantum */

/************************************************************************
**  RmSplitBuffers
**
**  FUNCTIONAL DESCRIPTION:
**
**    Split a quantum sized piece off the front of a pipe data segment.
**    Data segments containing compressed data are not supported.
**
**  FORMAL PARAMETERS:
**
**      data queue
**	quantum
**
**  FUNCTION VALUE:
**
**      pointer to data
**
************************************************************************/
PipeDataPtr	RmSplitBuffers( drain, data, quantum )
 PipeDrainPtr	drain;
 PipeDataPtr	data;
 int		quantum;
{
    PipeDataPtr	new;
    int status, y1 = DatY1_(data);

    if( DrnConvert_(drain) && quantum == DrnQnt_(drain) )
	{   /*
	    **	Create quantum scanlines of the Drain's data type.
	    */
	new = DdxRmAllocDataDesc_( drain,
			      DatCmpIdx_(data), y1, quantum );
	if( IsPointer_(new) )
	    {
	    status = DdxRmAllocDataArray_( new );
	    if( status == Success )
	        status = DdxConvert_( DatUdpPtr_(data),
				  DatUdpPtr_(new), XieK_MoveMode );
	    if( status != Success )
		{
		DdxRmDeallocData_(new);
		new = (PipeDataPtr) status;
		}
	    }
	}
    else
	new = DdxRmAllocRefDesc_( data, y1, quantum );

    if( IsPointer_(new) )
	{   /*
	    **  Adjust data to describe only the remant beyond quantum.
	    */
	DatHeight_(data)  -= quantum;
	DatY1_(data)      += quantum;
	if( DatDatTyp_(data) == UdpK_DTypeCL )
	    DatPos_(data)  = DdxPosCl_(DatBase_(data), DatPos_(data), quantum);
	else
	    DatPos_(data) += quantum * DatScnStr_(data);
	}
    return( new );
}				    /* end of RmSplitBuffers */
/* end of module SmiPipeRsm.c */
