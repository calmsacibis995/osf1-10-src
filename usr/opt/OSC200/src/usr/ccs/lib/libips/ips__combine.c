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

/************************************************************************
**
**  FACILITY:
**
**      Image Processing Services (IPS)
**
**  ABSTRACT:
**
**      This module contains the user level service and support routines
**	for IpsCombine. This service provides logical combination of bitmap
**	image data.
**
**  ENVIRONMENT:
**
**      VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHOR(S):
**
**      John Weber
**	Modified for V3.0 by John Poltrack
**
**  CREATION DATE:
**
**      11-JUN-1986
**
**
************************************************************************/

/*
**  Table of contents
*/
#ifdef NODAS_PROTO
long _IpsCombine();
#endif

/*
**  Include files
*/

#include <IpsDef.h>
#include <IpsStatusCodes.h>
#ifndef NODAS_PROTO
#include <ipsprot.h>			    /* Ips prototypes */
#endif

/*
**  MACRO definitions
*/

/*
** Minimum value function macro
*/
#define MIN_(a,b)\
    (a) < (b) ? (a) : (b)

/*
** Effective bit offset of udp using x and y coordinates
*/
#define EB(udp,x,y) \
((udp)->UdpL_Pos + ((udp)->UdpL_PxlStride * ((x) - (udp)->UdpL_X1)) + \
((udp)->UdpL_ScnStride * ((y) - (udp)->UdpL_Y1)))

/*
**  Equated Symbols
*/
#define MASK_SIZE 8			/* Number of elements in MASK array */

/*
**  External references
*/
#ifdef NODAS_PROTO
long _IpsCombineBits();                     /* from UIPS__OLD_COMBINE_BITS  */
#endif

/*****************************************************************************
**  _IpsCombine - Move block of bits with combination rules
**
**  FUNCTIONAL DESCRIPTION:
**
**      This function implements the traditional BITBLT operation between
**	ISL frames. Bits described by the source UDP are moved to the 
**	destination UDP after being masked and combined in the specified
**	fashion.
**
**  FORMAL PARAMETERS:
**
**      srcudp
**	dstudp
**	mask
**	rule
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
**      none
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
************************************************************************/
long _IpsCombine(srcudp, dstudp, mask, rule)
struct UDP *srcudp;
struct UDP *dstudp;
char mask[];
long rule;
{
    long bits_per_scan, scanline_count;
    long scanline;
    long i, j;
    unsigned char *src,*dst;
    long mask_array[8];
    /*
    **	Turn the byte array mask into a longword array mask more suitable for 
    **	use with _IpsCombineBits. If mask is not specified, build a mask array
    **	with all ones.
    */
    for (i = 0;  i < MASK_SIZE;  i++)
	{
	    char *tmp = (char *) &mask_array[i];
	    for (j = 0;  j < sizeof(long);  j++)
		*tmp++ = mask ? mask[i] : -1;
	}
    /*
    **	Start by determining the size of the region to be BLTed.
    */
    bits_per_scan = MIN_(
       (srcudp->UdpL_X2 - srcudp->UdpL_X1 + 1) * srcudp->UdpL_PxlStride,
       (dstudp->UdpL_X2 - dstudp->UdpL_X1 + 1) * dstudp->UdpL_PxlStride);

    scanline_count = MIN_(
        srcudp->UdpL_Y2 - srcudp->UdpL_Y1 + 1,
        dstudp->UdpL_Y2 - dstudp->UdpL_Y1 + 1);
    /*
    **	If regions overlap, decide in which direction the data should
    **	be moved so as not to destroy anything needed later.
    */
    src = srcudp->UdpA_Base + (EB(srcudp,srcudp->UdpL_X1,srcudp->UdpL_Y1)/8);
    dst = dstudp->UdpA_Base + (EB(dstudp,dstudp->UdpL_X1,dstudp->UdpL_Y1)/8);
   
    if ((rule < IpsK_ComboMin) || (rule > IpsK_ComboMax))
        return (IpsX_INVDARG);
        
    if (dst > src)
	/*
	**  Move scanlines from high addresses to low addresses
	*/
	for (scanline = scanline_count - 1;  scanline >= 0;  scanline--)
	    _IpsCombineBits(
		bits_per_scan,  		    /* Bit count	    */
		EB(srcudp,srcudp->UdpL_X1,scanline + srcudp->UdpL_Y1),
						    /* Source bit offset    */
		srcudp->UdpA_Base,		    /* Source buffer adr    */
		EB(dstudp,dstudp->UdpL_X1,(scanline + dstudp->UdpL_Y1)),
						    /* Dest bit offset	    */
		dstudp->UdpA_Base,		    /* Dest buffer address  */
		mask_array[scanline%MASK_SIZE],	    /* Source mask	    */
		rule);				    /* Combination rule	    */
    else
	/*
	**  Move scanlines from low addresses to high addresses
	*/
	for (scanline = 0;  scanline < scanline_count;  scanline++)
	    _IpsCombineBits(
		bits_per_scan,  		    /* Bit count	    */
		EB(srcudp,srcudp->UdpL_X1,scanline + srcudp->UdpL_Y1),
						    /* Source bit offset    */
		srcudp->UdpA_Base,		    /* Source buffer adr    */
		EB(dstudp,dstudp->UdpL_X1,scanline + dstudp->UdpL_Y1),
						    /* Dest bit offset	    */
		dstudp->UdpA_Base,		    /* Dest buffer address  */
		mask_array[scanline%MASK_SIZE],	    /* Source mask	    */
		rule);				    /* Combination rule	    */

    return (IpsX_SUCCESS);
}
