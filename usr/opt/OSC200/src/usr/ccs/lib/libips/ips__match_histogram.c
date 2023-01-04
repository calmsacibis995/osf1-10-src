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
**      Image Processing Services
**
**  ABSTRACT:
**
**      _IpsMatchHistogram matches an image plane's histogram the specified
**	shape.
**
**  ENVIRONMENT:
**
**      VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHOR(S):
**
**      Richard Piccolo
**
**  CREATION DATE:
**
**      April 3, 1990
**
************************************************************************/

/*
**  Table of contents
*/
#ifdef NODAS_PROTO
long  _IpsMatchHistogram();
#endif

/*
**  Include files
*/
#include <IpsDef.h>		    /* IPS Image Definitions	    */
#include <IpsStatusCodes.h>	    /* IPS Status Codes		    */
#include <IpsMemoryTable.h>	    /* IPS Memory Mgt. Functions    */
#ifndef NODAS_PROTO
#include <ipsprot.h>			    /* IPS Prototypes		    */
#endif
#include <math.h>		    /* Math Runtime Library	    */
/*
**  MACRO definitions
*/

/*
**  Equated Symbols
*/

/*
**  External References
*/
#ifdef NODAS_PROTO
long _IpsRemapUdp();
long  _IpsCreateLut_MH();
#endif

/******************************************************************************
**
**  FUNCTIONAL DESCRIPTION:
**
**      Takes a UDP, matches its histogram to the specified shape by deriving
**	a lookup table and applies the lookup table to the source UDP to
**	create a destination UDP.
**
**  FORMAL PARAMETERS:
**
**	src_udp --> pointer to source      udp (initialized)
**	dst_udp --> pointer to destination udp (uninitialized)
**	cpp    -->  control processing plane
**	density_type --> indicator for Gaussian, Gamma, Exponential, Flat
**	param1 -->  general argument (depending on the density type)
**	param2 -->  general argument (depending on the density type)
**	user_pd --> user density function (optional)
**	lut     --> returned LUT pointer (optional)
**
**  IMPLICIT INPUTS:	none
**  IMPLICIT OUTPUTS:	Lookup Table Structure
**  FUNCTION VALUE:	status return
**  SIGNAL CODES:	none
**  SIDE EFFECTS:	none
**                                                                            
*******************************************************************************/

long _IpsMatchHistogram (src_udp, dst_udp, cpp, density_type, param1, 
				  param2, user_pd, lut_ptr, flags)
struct UDP *src_udp;
struct UDP *dst_udp;
struct UDP *cpp;
unsigned long density_type;
double param1;
double param2;
float *user_pd;
unsigned long **lut_ptr;
unsigned long flags;
{
    long   status;			/* Returned status		    */
    unsigned long *remap_base;		/* Lookup Table For Match Histogram */

    /*
    ** Check class now before remap to determine if caller has specified the
    ** an allowable src udp.  There is no sense in waiting til remap
    */
    if  (src_udp->UdpB_Class != UdpK_ClassA) return (IpsX_INVDARG);

    /* 
    ** Create Lookup Table From Src Udp using Match Histogram Routine
    */
    status = _IpsCreateLut_MH (src_udp, cpp, density_type, param1, param2,
	user_pd, &remap_base);
    if (status != IpsX_SUCCESS)
	return (status);

    status = _IpsRemapUdp (src_udp, dst_udp, cpp, remap_base, 
	src_udp->UdpL_Levels, IpsK_DTypeLU, flags);

    /* If user does not want the LUT deallocate it */
    if (lut_ptr == 0)
        (*IpsA_MemoryTable[IpsK_Dealloc]) (remap_base);
    else
	*lut_ptr = (unsigned long *)remap_base;

    return (status);
}
