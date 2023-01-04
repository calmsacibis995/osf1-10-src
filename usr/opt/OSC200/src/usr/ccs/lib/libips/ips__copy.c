/************************************************************************
**
**  Copyright (c) Digital Equipment Corporation, 1990-1991 All Rights Reserved.
**  Unpublished rights reserved under the copyright laws of the United States.
**  The software contained on this media is proprietary to and embodies the
**  confidential technology of Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and media is authorized only
**  pursuant to a valid written license from Digital Equipment Corporation.
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the U.S.
**  Government is subject to restrictions as set forth in Subparagraph
**  (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19, as applicable.
**/
/******************************************************************************
**
**  FACILITY:
**
**      Image Processing Services (IPS)
**
**  ABSTRACT:
**
**      Core routine to perform a copy.
**
**  ENVIRONMENT:
**
**      VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHOR(S):
**
**      Richard Piccolo, Digital Equipment Corp.
**
**  CREATION DATE:
**
**      Nov, 1989
**
*******************************************************************************/
/*
**  Table of contents:
*/
#ifdef NODAS_PROTO
long _IpsCopyBitonal();			    /* copy bitonal udp to another */
long _IpsCopy();			    /* copy udp to another */
#endif

/*
**  Include files:
*/
#include    <IpsDef.h>
#include    <IpsStatusCodes.h>
#include    <IpsMemoryTable.h>
#ifndef NODAS_PROTO
#include    <ipsprot.h>			    /* Ips prototypes */
#endif
/*
**  Equated Symbols:
*/

/*
**  External References:
*/
#ifdef NODAS_PROTO
long _IpsCopyData ();		/* copy udp data - from Ips__udp_utils */
long _IpsBuildDstUdp();		/* from udp_utils */
#endif

/*
**  _IpsCopy
**
**  FUNCTIONAL DESCRIPTION:
**
**      Copy from source image array to destination image array.  The 
**	area copied is defined by the lower and upper bounds in the 
**	supplied source UDP descriptor.  The supplied destination UDP 
**	descriptor describes the output desired.
**
**  FORMAL PARAMETERS:
**
**      src_udp - pointer to the source UDP descriptor 
**	    UdpA_Base, UdpL_ArSize - array containing complete image,
**	    UdpL_ScnStride  - scanline stride,
**	    UdpL_X1  - lower boundary in X direction,
**	    UdpL_X2  - upper boundary in X direction,
**	    UdpL_Y1  - lower boundary in Y direction,
**	    UdpL_Y2  - upper boundary in Y direction,
**	    UdpL_Pos - offset to data for element ( X1, X2 ) + initial offset.
**
**      dst_udp - pointer to the empty destination UDP descriptor 
**	flags   --> retain outer dimension option
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      status
**
**  SIGNAL CODES:
**
**	IpsX_INVBUFLEN	- image data falls outside of source image array
**			    or destination image array.
**
**  SIDE EFFECTS:
**
**      none
**
*****************************************************************************/

long _IpsCopyBitonal ( src_udp, dst_udp , flags)
struct UDP	*src_udp;	    /* source UDP descriptor pointer	    */
struct UDP	*dst_udp;	    /* destination UDP descriptor pointer   */
unsigned long	flags;		    /* retain option	        */
{
return (_IpsCopy( src_udp, dst_udp, flags));
}

long _IpsCopy( src_udp, dst_udp, flags)
struct UDP	*src_udp;	    /* source UDP descriptor pointer	    */
struct UDP	*dst_udp;	    /* destination UDP descriptor pointer   */
unsigned long	flags;		    /* retain option	        */

    {
    int size;
    long	status;

    /*
    ** Initialize destination udp fixed fields , calculate size in bytes of
    ** new data buffer and allocate memory
    */
    status = _IpsBuildDstUdp (src_udp, dst_udp, 0, flags, 0);
    if (status != IpsX_SUCCESS) 
        return (status);

    /*
    **  Copy data using optimal method
    */
    status = _IpsCopyData (src_udp, dst_udp);
    return (status);
}/* end _IpsCopy */
