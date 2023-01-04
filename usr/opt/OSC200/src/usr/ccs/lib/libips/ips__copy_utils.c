/******************************************************************************
**
**  Copyright (c) Digital Equipment Corporation, 1990-1991 All Rights Reserved.
**  Unpublished rights reserved under the copyright laws of the United States.
**  The software contained on this media is proprietary to and embodies the
**  confidential technology of Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and media is authorized only
**  pursuant to a valid written license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the U.S.
**  Government is subject to restrictions as set forth in Subparagraph
**  (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19, as applicable.
**/

/************************************************************************
**  IPS__COPY_UTILS
**
**  FACILITY:
**
**	Image Processing Services (IPS)
**
**  ABSTRACT:
**
**	This module contains utility routines used to dispatch to 
** 	the appropriate module for copying data from one format to
** 	another.
** 
**
**  ENVIRONMENT:
**
**	VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHOR(S):
**
** 	Mark Sornson
**	Revised by Karen D'Intino
**
**  CREATION DATE:
**
**	13-JULY-1991
**
************************************************************************/

/*
**  Table of contents:
**
**	Global routines
*/
long _IpsCopyData();	  	    /* Routine used by IPS & ISL to move data */
long _IpsGetUdpInfo();		    /* Used by ISL Only*/
long _IpsGetUdpElementAlignment();  /* Used by ISL (through GetUdpInfo) */
long _IpsGetUdpPadding();	    /* Used by (IPS) Copy data and BY ISL */
long _IpsGetUdpScanlineAlignment(); /* Used by ISL Only */
		    
/*
**	Module local routines
*/
#ifdef NODAS_PROTO
static void	Move_bits_by_plane();
static void	Move_bits_by_scanline();
#endif


/*
**  Include files:
*/
#include    <IpsDef.h>
#include    <IpsDefP.h>
#include    <IpsStatusCodes.h>
#include    <IpsMemoryTable.h>
#ifndef NODAS_PROTO
#include    <ipsprot.h>			    /* Ips prototypes */
#endif

/*
**  MACRO definitions:
**
**	Determine the alignment of a data element.
**	Mask out all but the lower 5 bits, and use the
**	result as an index into a static alignment tabel.
*/
#define DATA_ALIGNMENT_( data )	    alignment_table[(data & 0x0000001F)]

/*
**  Equated Symbols:
**
**	Constants
*/
#define	MOVE_BITS_BY_PLANE	1
#define MOVE_BITS_BY_LINE	2
#define MOVE_BITS_BY_PIXEL	3

/*
**  External References:
*/
#ifdef NODAS_PROTO
void	*memcpy();                                 /* from standard library */
#endif

/*
**	External function calls
*/
#ifdef NODAS_PROTO
long	_IpsMoveBits();	                /* from module IpsMoveBits */
					/* Depending on the platform */
					/* VMS -- In ips__move_bits.mar */
					/* Portable -- In Uips__move_bits.c */
#endif
/*
**  Local Storage:
**
**	Alignment table, used to lookup alignment any particular 
**	offset address.
*/
static long	alignment_table[32] = {
		    /* 0 0000 */    LONG_ALIGNED,
		    /* 0 0001 */    BIT_ALIGNED,
		    /* 0 0010 */    BIT_ALIGNED,
		    /* 0 0011 */    BIT_ALIGNED,
		    /* 0 0100 */    BIT_ALIGNED,
		    /* 0 0101 */    BIT_ALIGNED,
		    /* 0 0110 */    BIT_ALIGNED,
		    /* 0 0111 */    BIT_ALIGNED,
		    /* 0 1000 */    BYTE_ALIGNED,
		    /* 0 1001 */    BIT_ALIGNED,
		    /* 0 1010 */    BIT_ALIGNED,
		    /* 0 1011 */    BIT_ALIGNED,
		    /* 0 1100 */    BIT_ALIGNED,
		    /* 0 1101 */    BIT_ALIGNED,
		    /* 0 1110 */    BIT_ALIGNED,
		    /* 0 1111 */    BIT_ALIGNED,

		    /* 1 0000 */    WORD_ALIGNED,
		    /* 1 0001 */    BIT_ALIGNED,
		    /* 1 0010 */    BIT_ALIGNED,
		    /* 1 0011 */    BIT_ALIGNED,
		    /* 1 0100 */    BIT_ALIGNED,
		    /* 1 0101 */    BIT_ALIGNED,
		    /* 1 0110 */    BIT_ALIGNED,
		    /* 1 0111 */    BIT_ALIGNED,
		    /* 1 1000 */    BYTE_ALIGNED,
		    /* 1 1001 */    BIT_ALIGNED,
		    /* 1 1010 */    BIT_ALIGNED,
		    /* 1 1011 */    BIT_ALIGNED,
		    /* 1 1100 */    BIT_ALIGNED,
		    /* 1 1101 */    BIT_ALIGNED,
		    /* 1 1110 */    BIT_ALIGNED,
		    /* 1 1111 */    BIT_ALIGNED };


/******************************************************************************
**  _IpsCopyData
**
**  FUNCTIONAL DESCRIPTION:
**
**	Move data from a source buffer to a destination buffer, both of
**	which are described with UDP descriptors.
**
**  FORMAL PARAMETERS:
**
**	srcudp	    Source buffer UDP.  Passed by reference.
**
**	dstudp	    Destination buffer UDP.  Passed by reference.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
long _IpsCopyData( srcudp, dstudp )
struct UDP	*srcudp;
struct UDP	*dstudp;
{
long	d_lincnt    = dstudp->UdpL_ScnCnt;
long	d_pxlcnt    = dstudp->UdpL_PxlPerScn;
long	s_lincnt    = srcudp->UdpL_ScnCnt;
long	s_pxlcnt    = srcudp->UdpL_PxlPerScn;
long	dstudp_padding;
long	move_type	    = MOVE_BITS_BY_PIXEL;
long	srcudp_padding;
long	srcpos, dstpos;
long    status = IpsX_SUCCESS;

if ( s_pxlcnt > d_pxlcnt )
    return ( IpsX_SPCGTRDPC );

if ( s_lincnt > d_lincnt )
    return ( IpsX_SLCGTRDLC );

if ( srcudp->UdpW_PixelLength == 0 )
    return ( IpsX_SRCLENZER );

if ( dstudp->UdpW_PixelLength == 0 )
    return ( IpsX_DSTLENZER );

if ( srcudp->UdpW_PixelLength > dstudp->UdpW_PixelLength )
    return ( IpsX_SPLGTRDPL );

if ( srcudp->UdpW_PixelLength > dstudp->UdpL_PxlStride )
    return ( IpsX_SPLGTRDPS );

if ( srcudp->UdpW_PixelLength > 64 )
    return ( IpsX_SLENGTR64 );

if ( dstudp->UdpW_PixelLength > 64 )
    return ( IpsX_DLENGTR64 );

/*
** Now move the bits.  This function will figure out the most optimal
** way to move the data.
** Figure out whether the data can be moved as an entire plane,
** or a scanline at a time.  The default method is a pixel at
** a time.
*/
dstudp_padding = _IpsGetUdpPadding( dstudp );
srcudp_padding = _IpsGetUdpPadding( srcudp );

/*
** Note that it's only possible to move larger blocks of data if
** the pixels in the source and the destination are the same size.
*/
srcpos = srcudp->UdpL_Pos + (srcudp->UdpL_Y1 * srcudp->UdpL_ScnStride) +
    srcudp->UdpL_X1 * srcudp->UdpL_PxlStride;
dstpos = dstudp->UdpL_Pos + (dstudp->UdpL_Y1 * dstudp->UdpL_ScnStride) +
    dstudp->UdpL_X1 * dstudp->UdpL_PxlStride;

if ( srcudp->UdpW_PixelLength == dstudp->UdpW_PixelLength )
    {
    switch( srcudp_padding )
	{
	case LINE_PACKED_PIXEL_PACKED:
	    switch ( dstudp_padding )
		{
		case LINE_PACKED_PIXEL_PACKED:
		    if ( (srcpos & BIT_ALIGNED) == 0 &&
		         (dstpos & BIT_ALIGNED) == 0 )
			move_type = MOVE_BITS_BY_PLANE;
		    break;
		case LINE_PADDED_PIXEL_PACKED:
		    if ( (DATA_ALIGNMENT_(srcpos) != BIT_ALIGNED &&
		          DATA_ALIGNMENT_(srcudp->UdpL_ScnStride) != BIT_ALIGNED )
					    &&
		         (DATA_ALIGNMENT_(dstpos) != BIT_ALIGNED &&
		          DATA_ALIGNMENT_(dstudp->UdpL_ScnStride) != BIT_ALIGNED ) )
			move_type = MOVE_BITS_BY_LINE;
		    break;
		default:
		    break;
		} /* end switch */
	    break;
	case LINE_PADDED_PIXEL_PACKED:
	    switch ( dstudp_padding )
		{
		case LINE_PACKED_PIXEL_PACKED:
		case LINE_PADDED_PIXEL_PACKED:
		    if ( (DATA_ALIGNMENT_(srcpos) != BIT_ALIGNED &&
			  DATA_ALIGNMENT_(srcudp->UdpL_ScnStride) != BIT_ALIGNED )
			    	    &&
		         (DATA_ALIGNMENT_(dstpos) != BIT_ALIGNED &&
		          DATA_ALIGNMENT_(dstudp->UdpL_ScnStride) != BIT_ALIGNED ) )
			move_type = MOVE_BITS_BY_LINE;
		    break;
		default:
		    break;
		} /* end switch */
	    break;
	default:
	    break;
	} /* end switch */
    } /* end if */

/*
** If both src and dst are fully packed, but the destination pixel count is 
** larger than the src pixel count, set the move type to be MOVE_BITS_BY_PIXEL.
*/
if ( move_type == MOVE_BITS_BY_PLANE )
    if ( dstudp->UdpL_PxlPerScn > srcudp->UdpL_PxlPerScn )
	move_type = MOVE_BITS_BY_PIXEL;

/* If we have a roi and this is a bitonal image, should move data 
** by pixel to avoid incorrect move by  line.
*/

if ((move_type == MOVE_BITS_BY_LINE) && (srcudp->UdpL_PxlStride == 1) && 
		(srcudp->UdpL_PxlPerScn < srcudp->UdpL_ScnStride ))
    move_type = MOVE_BITS_BY_PIXEL;
/*
** Switch to the appropriate move routine
*/
switch ( move_type )
    {
    case MOVE_BITS_BY_PLANE:
	Move_bits_by_plane( srcudp, dstudp );
	break;
    case MOVE_BITS_BY_LINE:
	Move_bits_by_scanline( srcudp, dstudp );
	break;
    case MOVE_BITS_BY_PIXEL:
    default:
	status = _IpsMoveBits( srcudp, dstudp );
	break;
    }
return (status);
} /* end of _IpsCopyData */


/******************************************************************************
**  _IpsGetUdpInfo
**
**  FUNCTIONAL DESCRIPTION:
**
**	[@tbs@]
**
**  FORMAL PARAMETERS:
**
**	[@description_or_none@]
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	[@description_or_none@]
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
long _IpsGetUdpInfo( udp )
struct UDP	*udp;
{
long	data_alignment;
long	descriptor_alignment;

data_alignment = _IpsGetUdpElementAlignment( udp );
switch ( udp->UdpW_PixelLength )
    {
    case 8:
	descriptor_alignment = (data_alignment << 16) + 8;
	break;
    case 16:
	descriptor_alignment = (data_alignment << 16) + 16;
	break;
    case 24:
	descriptor_alignment = (data_alignment << 16) + 24;
	break;
    case 32:
	descriptor_alignment = (data_alignment << 16) + 32;
	break;
    default:
	descriptor_alignment = (data_alignment << 16) + NBITS;
	break;
    }

return descriptor_alignment;
} /* end of _IpsGetUdpInfo */


/******************************************************************************
**  _IpsGetUdpElementAlignment
**
**  FUNCTIONAL DESCRIPTION:
**
**	Get the alignment of an element in a UDP.
**
**  FORMAL PARAMETERS:
**
**	udp	Uncompressed data plane descriptor.  Passed by reference.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	udp_alignment	longword, by value
**			Alignment indicator of the following types:
**
**			    BIT_ALIGNED
**			    BYTE_ALIGNED
**			    WORD_ALIGNED
**			    LONG_ALIGNED
**
**  SIGNAL CODES:
**
**	[@description_or_none@]
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
long _IpsGetUdpElementAlignment( udp )
struct UDP	*udp;
{
long	udp_alignment	= 0;
long	pos;

pos = udp->UdpL_Pos + (udp->UdpL_Y1 * udp->UdpL_ScnStride) +
    udp->UdpL_X1 * udp->UdpL_PxlStride;

switch ( DATA_ALIGNMENT_(pos) )
    {
    /*
    ** If the initial offset (POS) is BIT_ALIGNED...
    */
    case BIT_ALIGNED:
	udp_alignment = BIT_ALIGNED;
	break;
    /*
    ** If the initial offset (POS) is BYTE_ALIGNED...
    */
    case BYTE_ALIGNED:
	switch ( DATA_ALIGNMENT_( udp->UdpL_ScnStride ) )
	    {
	    /*
	    ** ... and if the scanline stride (S2) is BIT_ALIGNED ...
	    */
	    case BIT_ALIGNED:
		udp_alignment = BIT_ALIGNED;
		break;
	    /*
	    ** ... and if the scanline stride (S2) is any other alignment ...
	    */
	    case BYTE_ALIGNED:
	    case WORD_ALIGNED:
	    case LONG_ALIGNED:
		switch ( DATA_ALIGNMENT_( udp->UdpL_PxlStride ) )
		    {
		    /*
		    ** ... and if the pixel stride (S1) is BIT_ALIGNED ...
		    */
		    case BIT_ALIGNED:
			udp_alignment = BIT_ALIGNED;
			break;
		    /*
		    ** ... and if the pixel stride (S1) is any other 
		    ** alignment, then BYTE alignment is the best we can do.
		    */
		    case BYTE_ALIGNED:
		    case WORD_ALIGNED:
		    case LONG_ALIGNED:
			udp_alignment = BYTE_ALIGNED;
		    default:
			break;
		    } /* end of S1 switch */
	    default:
		break;
	    } /* end of S2 switch */
	break;
    /*
    ** If the initial offset (POS) is WORD_ALIGNED...
    */
    case WORD_ALIGNED:
	switch ( DATA_ALIGNMENT_( udp->UdpL_ScnStride ) )
	    {
	    /*
	    ** ... and if the scanline stride (S2) is BIT_ALIGNED ...
	    */
	    case BIT_ALIGNED:
		udp_alignment = BIT_ALIGNED;
		break;
	    /*
	    ** ... and if the scanline stride (S2) is BYTE_ALIGNED ...
	    */
	    case BYTE_ALIGNED:
		switch ( DATA_ALIGNMENT_( udp->UdpL_PxlStride ) )
		    {
		    /*
		    ** ... and if the pixel stride (S1) is BIT_ALIGNED ...
		    */
		    case BIT_ALIGNED:
			udp_alignment = BIT_ALIGNED;
			break;
		    /*
		    ** ... and if the pixel stride (S1) is anything else,
		    ** BYTE alignment is the best we can do.
		    */
		    case BYTE_ALIGNED:
		    case WORD_ALIGNED:
		    case LONG_ALIGNED:
			udp_alignment = BYTE_ALIGNED;
		    default:
			break;
		    } /* end of S1 switch */
	    /*
	    ** ... and if the scanline stride (S2) is any other alignment ...
	    */
	    case WORD_ALIGNED:
	    case LONG_ALIGNED:
		switch ( DATA_ALIGNMENT_( udp->UdpL_PxlStride ) )
		    {
		    /*
		    ** ... if the pixel stride (S1) is BIT_ALIGNED ...
		    */
		    case BIT_ALIGNED:
			udp_alignment = BIT_ALIGNED;
			break;
		    /*
		    ** ... if the pixel stride (S1) is BYTE_ALIGNED ...
		    */
		    case BYTE_ALIGNED:
			udp_alignment = BYTE_ALIGNED;
			break;
		    /*
		    ** ... if the pixel stride (S1) is anything else, WORD
		    ** alignment is the best we can do.
		    */
		    case WORD_ALIGNED:
		    case LONG_ALIGNED:
			udp_alignment = WORD_ALIGNED;
		    default:
			break;
		    } /* end of S1 switch */
	    default:
		break;
	    } /* end of S2 switch */
	break;
    /*
    ** If the initial offset (POS) is LONG_ALIGNED...
    */
    case LONG_ALIGNED:
	switch ( DATA_ALIGNMENT_( udp->UdpL_ScnStride ) )
	    {
	    /*
	    ** ... and if the scanline stride (S2) is BIT_ALIGNED ...
	    */
	    case BIT_ALIGNED:
		udp_alignment = BIT_ALIGNED;
		break;
	    /*
	    ** ... and if the scanline stride (S2) is BYTE_ALIGNED ...
	    */
	    case BYTE_ALIGNED:
		switch ( DATA_ALIGNMENT_( udp->UdpL_PxlStride ) )
		    {
		    /*
		    ** ... and if the pixel stride (S1) is BIT_ALIGNED ...
		    */
		    case BIT_ALIGNED:
			udp_alignment = BIT_ALIGNED;
			break;
		    /*
		    ** ... and if the pixel stride (S1) is any other 
		    ** alignment, then BYTE alignment is the best we can do.
		    */
		    case BYTE_ALIGNED:
		    case WORD_ALIGNED:
		    case LONG_ALIGNED:
			udp_alignment = BYTE_ALIGNED;
		    default:
			break;
		    } /* end of S1 switch */
	    /*
	    ** ... and if the scanline stride (S2) is WORD_ALIGNED ...
	    */
	    case WORD_ALIGNED:
		switch ( DATA_ALIGNMENT_( udp->UdpL_PxlStride ) )
		    {
		    /*
		    ** ... and if the pixel stride (S1) is BIT_ALIGNED ...
		    */
		    case BIT_ALIGNED:
			udp_alignment = BIT_ALIGNED;
			break;
		    /*
		    ** ... and if the pixel stride (S1) is BYTE_ALIGNED ...
		    */
		    case BYTE_ALIGNED:
			udp_alignment = BYTE_ALIGNED;
			break;
		    /*
		    ** ... and if the pixel stride (S1) is anything else,
		    ** WORD alignment is the best we can do.
		    */
		    case WORD_ALIGNED:
		    case LONG_ALIGNED:
			udp_alignment = WORD_ALIGNED;
		    default:
			break;
		    } /* end of S1 switch */
	    /*
	    ** ... and if the scanline stride (S2) is LONG_ALIGNED ...
	    */
	    case LONG_ALIGNED:
		switch ( DATA_ALIGNMENT_( udp->UdpL_PxlStride ) )
		    {
		    /*
		    ** ... if the pixel stride (S1) is BIT_ALIGNED ...
		    */
		    case BIT_ALIGNED:
			udp_alignment = BIT_ALIGNED;
			break;
		    /*
		    ** ... if the pixel stride (S1) is BYTE_ALIGNED ...
		    */
		    case BYTE_ALIGNED:
			udp_alignment = BYTE_ALIGNED;
			break;
		    /*
		    ** ... if the pixel stride (S1) is WORD_ALIGNED ...
		    */
		    case WORD_ALIGNED:
			udp_alignment = WORD_ALIGNED;
			break;
		    /*
		    ** ... if the pixel stride (S1) is LONG_ALIGNED ...
		    */
		    case LONG_ALIGNED:
			udp_alignment = LONG_ALIGNED;
		    default:
			break;
		    } /* end of S2 switch */
	    default:
		break;
	    } /* end of S1 switch */
    default:
	break;
    } /* end of POS switch */

return udp_alignment;
} /* end of _IpsGetUdpElementAlignment */


/******************************************************************************
**  _IpsGetUdpPadding
**
**  FUNCTIONAL DESCRIPTION:
**
**	Determine (and return a value that signifies) the type of pixel
**	and scanline padding that's specified for a given UDP.
**
**  FORMAL PARAMETERS:
**
**	udp	Uncompressed Data Plane descriptor.  Passed by reference.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	padding_type	Padding type code:
**
**			    LINE_PACKED_PIXEL_PACKED
**			    LINE_PACKED_PIXEL_PADDED
**			    LINE_PADDED_PIXEL_PACKED
**			    LINE_PADDED_PIXEL_PADDED
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
long _IpsGetUdpPadding( udp )
struct UDP	*udp;
{
long	padding_type;
long	pixel_count	    = udp->UdpL_PxlPerScn;
long	pixel_padding;
long	scanline_padding;

pixel_padding = udp->UdpL_PxlStride - udp->UdpW_PixelLength;
scanline_padding = udp->UdpL_ScnStride - (pixel_count * udp->UdpL_PxlStride );

if ( scanline_padding == 0 )
    {
    if ( pixel_padding == 0 )
	padding_type = LINE_PACKED_PIXEL_PACKED;
    else
	padding_type = LINE_PACKED_PIXEL_PADDED;
    }
else
    {
    if ( pixel_padding == 0 )
	padding_type = LINE_PADDED_PIXEL_PACKED;
    else
	padding_type = LINE_PADDED_PIXEL_PADDED;
    }
		    
return padding_type;
} /* end of _IpsGetUdpPadding */


/******************************************************************************
**  _IpsGetUdpScanlineAlignment
**
**  FUNCTIONAL DESCRIPTION:
**
**	[@tbs@]
**
**  FORMAL PARAMETERS:
**
**	[@description_or_none@]
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	[@description_or_none@]
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
long _IpsGetUdpScanlineAlignment( udp )
struct UDP  *udp;
{
long	scanline_alignment;
long	pos;

pos = udp->UdpL_Pos + (udp->UdpL_Y1 * udp->UdpL_ScnStride) +
    udp->UdpL_X1 * udp->UdpL_PxlStride;

switch ( DATA_ALIGNMENT_(pos) )
    {
    /*
    ** If the initial offset (POS) is BIT_ALIGNED...
    */
    case BIT_ALIGNED:
	scanline_alignment = BIT_ALIGNED;
	break;
    /*
    ** If the initial offset (POS) is BYTE_ALIGNED...
    */
    case BYTE_ALIGNED:
	switch ( DATA_ALIGNMENT_( udp->UdpL_ScnStride ) )
	    {
	    /*
	    ** ... and if the scanline stride (S2) is BIT_ALIGNED ...
	    */
	    case BIT_ALIGNED:
		scanline_alignment = BIT_ALIGNED;
		break;
	    /*
	    ** ... and if the scanline stride (S2) is any other alignment ...
	    */
	    case WORD_ALIGNED:
	    case LONG_ALIGNED:
	    case BYTE_ALIGNED:
	    default:
		scanline_alignment = BYTE_ALIGNED;
		break;
	    } /* end of S2 switch */
	break;
    /*
    ** If the initial offset (POS) is WORD_ALIGNED...
    */
    case WORD_ALIGNED:
	switch ( DATA_ALIGNMENT_( udp->UdpL_ScnStride ) )
	    {
	    /*
	    ** ... and if the scanline stride (S2) is BIT_ALIGNED ...
	    */
	    case BIT_ALIGNED:
		scanline_alignment = BIT_ALIGNED;
		break;
	    case WORD_ALIGNED:
	    case LONG_ALIGNED:
		scanline_alignment = WORD_ALIGNED;
		break;
	    case BYTE_ALIGNED:
	    default:
		scanline_alignment = BYTE_ALIGNED;
		break;
	    } /* end of S2 switch */
	break;
    /*
    ** If the initial offset (POS) is LONG_ALIGNED...
    */
    case LONG_ALIGNED:
	switch ( DATA_ALIGNMENT_( udp->UdpL_ScnStride ) )
	    {
	    /*
	    ** ... and if the scanline stride (S2) is BIT_ALIGNED ...
	    */
	    case BIT_ALIGNED:
		scanline_alignment = BIT_ALIGNED;
		break;
	    /*
	    ** ... and if the scanline stride (S2) is BYTE_ALIGNED ...
	    */
	    case WORD_ALIGNED:
		scanline_alignment = WORD_ALIGNED;
		break;
	    case LONG_ALIGNED:
		scanline_alignment = LONG_ALIGNED;
		break;
	    case BYTE_ALIGNED:
	    default:
		scanline_alignment = BYTE_ALIGNED;
		break;
	    } /* end of S1 switch */
    default:
	break;
    } /* end of POS switch */

return scanline_alignment;
} /* end of _IpsGetUdpScanlineAlignment */


/******************************************************************************
**  Move_bits_by_plane()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Move data from a source buffer to a destination buffer all in
**	one chunk (i.e., treat the data as a contiguous plane).
**
**  FORMAL PARAMETERS:
**
**	srcudp	Source buffer UDP.  Passed by reference.
**
**	dstudp	Destination buffer UDP.  Passed by reference.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
static void Move_bits_by_plane( srcudp, dstudp )
struct UDP	*srcudp;
struct UDP	*dstudp;
{
unsigned char	*d_base	= dstudp->UdpA_Base;
unsigned char	*s_base	= srcudp->UdpA_Base;

long	 	lincnt		= srcudp->UdpL_ScnCnt;
unsigned long   *dstPlaneSizePtr;
unsigned long	dstPlaneSize;
unsigned long	planeSize;
unsigned long   *srcPlaneSizePtr;
unsigned long	srcPlaneSize;

/*
** Get the addresses of the first byte in each array.
*/
s_base = srcudp->UdpA_Base + ((srcudp->UdpL_Pos +
			    (srcudp->UdpL_X1 * srcudp->UdpL_PxlStride) +
			    (srcudp->UdpL_Y1 * srcudp->UdpL_ScnStride))/8);

d_base = dstudp->UdpA_Base + ((dstudp->UdpL_Pos +
                            (dstudp->UdpL_X1 * dstudp->UdpL_PxlStride) +
                            (dstudp->UdpL_Y1 * dstudp->UdpL_ScnStride))/8);
/*
** Get the size in bytes of the source plane.
**
**	NOTE:	compare the source plane size with the destination
**		plane size.  If the destination size is smaller,
**		use it instead (i.e., copy as much as will fit, and
**		assume the higher level code will handle it OK).
**
**		Sometimes, source data-planes can be larger than really
**		necessary.  In the case of copies, it's really the
**		destination size that is most important.
*/
srcPlaneSizePtr = (unsigned long *)(s_base - sizeof(srcPlaneSize));
srcPlaneSize = *srcPlaneSizePtr;
dstPlaneSizePtr = (unsigned long *)(d_base - sizeof(dstPlaneSize));
dstPlaneSize = *dstPlaneSizePtr;

if ( dstPlaneSize < srcPlaneSize )
    planeSize = dstPlaneSize;
else
    planeSize = srcPlaneSize;

/*
** Copy the plane.
*/
memcpy( d_base, s_base, planeSize );

return;
} /* end of Move_bits_by_plane */


/******************************************************************************
**  Move_bits_by_scanline()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Move data from a source buffer to a destination buffer a scanline
**	at a time.
**
**  FORMAL PARAMETERS:
**
**	srcudp	Source buffer UDP.  Passed by reference.
**
**	dstudp	Destination buffer UDP.  Passed by reference.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
static void Move_bits_by_scanline( srcudp, dstudp )
struct UDP	*srcudp;
struct UDP	*dstudp;
{
unsigned char	*d_base		= dstudp->UdpA_Base;
unsigned char	*d_ptr;
unsigned char	*s_base		= srcudp->UdpA_Base;
unsigned char	*s_ptr;

long	 d_line_len;
long	 d_line_stride	= dstudp->UdpL_ScnStride;
long	 lincnt		= srcudp->UdpL_ScnCnt;
long	 s_line_stride	= srcudp->UdpL_ScnStride;
long	 s_line_len;
long	 working_line_len;
long d_pixels, s_pixels;
/*
** Get the addresses of the first byte in each array.
*/
s_base = srcudp->UdpA_Base + ((srcudp->UdpL_Pos + 
			    (srcudp->UdpL_X1 * srcudp->UdpL_PxlStride) +
			    (srcudp->UdpL_Y1 * srcudp->UdpL_ScnStride))/8);
d_base = dstudp->UdpA_Base + ((dstudp->UdpL_Pos +
                            (dstudp->UdpL_X1 * dstudp->UdpL_PxlStride) +
                            (dstudp->UdpL_Y1 * dstudp->UdpL_ScnStride))/8);

/*
** Get the length in bytes of each line (to the full amount of stride).
** Use the shorter length as the number of bytes to move.  (Assume the
** larger length indicates more padding.)
*/

d_pixels = (dstudp->UdpL_PxlPerScn * dstudp->UdpL_PxlStride)/8;
s_pixels = (srcudp->UdpL_PxlPerScn * srcudp->UdpL_PxlStride)/8;

d_line_len = d_line_stride/8;
s_line_len = s_line_stride/8;

if (d_pixels <= s_pixels )
    working_line_len = d_pixels;
else
    working_line_len = s_pixels;

/*
** Set up the scanline line and pixel loops, and move the bits a line at
** a time.
*/
s_ptr = s_base;
d_ptr = d_base;

/*
** Loop that steps through scanlines
*/
for ( ; 0 < lincnt; --lincnt )
    {
    /*
    ** Move a scanline from the source to the destination
    */
    memcpy( d_ptr, s_ptr, working_line_len );

    /*
    ** Move the pointers
    */
    d_ptr += d_line_len;
    s_ptr += s_line_len;
    }

return;
} /* end of Move_bits_by_scanline */
