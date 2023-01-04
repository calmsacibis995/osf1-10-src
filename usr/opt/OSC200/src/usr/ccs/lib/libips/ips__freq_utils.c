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
**
*****************************************************************************/

/*****************************************************************************
**
**  FACILITY:
**
**      Image Processing Services
**
**  ABSTRACT:
**
**      IPS__FREQ_UTILS consists of utilities to facilitate the use of complex
**	data for frequency filtering.
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
**      October 22, 1990
**
************************************************************************/

/*
**  Table of contents
*/
#ifdef NODAS_PROTO
long	_IpsPowerSpec();		/* Generate power spectrum	    */
long	_IpsMagnitude();		/* Generate magnitude spectrum	    */
long	_IpsLogMagnitude();		/* Generate log magnitude spectrum  */
long	_IpsPhase();			/* Generate phase spectrum	    */	
long	_IpsExtractComp();		/* extract real or imag comp	    */
long	_IpsVerifySizeFFT();		/* verify FFT size, ret log base2   */
long	build_dst();			/* local common utility		    */
#endif
/*
** Include files
*/
#include <IpsDef.h>			    /* IPS definitions		    */
#include <IpsDefP.h>			    /* IPS Private definitions	    */
#include <IpsStatusCodes.h>		    /* IPS Status Codes		    */
#include <IpsMemoryTable.h>		    /* IPS Memory Vector Table      */
#include <IpsMacros.h>			    /* IPS Macros		    */
#ifndef NODAS_PROTO
#include <ipsprot.h>			    /* Ips prototypes */
#endif
#include <math.h>			    /* Math Runtime routines        */

/*
**  Equated Symbols
*/

/*
**  External references
*/
#ifdef NODAS_PROTO
long _IpsVerifyNotInPlace();                       /* from ips__udp_utils.c */
#endif

/******************************************************************************
**
**  FUNCTIONAL DESCRIPTION:
**
**	_IpsPowerSpec creates a float power spectrum from a complex src
**
**	Power spectrum is defined as:
**
**	    P[i] = real[i] ** 2 + imag[i] ** 2
**
**  FORMAL PARAMETERS:
**
**	src_udp		    source udp
**	dst_udp		    destination udp
**	flags		    not implemented
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
**	status
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/

long _IpsPowerSpec (src_udp, dst_udp, flags)
struct UDP	*src_udp;		    /* source udp		    */
struct UDP	*dst_udp;		    /* dst udp			    */
unsigned long	flags;			    /* forward or backward */
    {
    unsigned long	mem_alloc;
    long		status;
    unsigned long	ix, iy;
    COMPLEX		*src_ptr;
    float		*dst_ptr;

    if  (src_udp->UdpB_Class != UdpK_ClassA)
	return (IpsX_UNSOPTION);
 
    if (src_udp->UdpB_DType != UdpK_DTypeC)
	return (IpsX_INVDTYPE);

    status = build_dst (src_udp, dst_udp, &mem_alloc);
    if (status != IpsX_SUCCESS)
	return (status);

    src_ptr = (COMPLEX *)src_udp->UdpA_Base + (src_udp->UdpL_Pos>>6);
    dst_ptr = (float *)dst_udp->UdpA_Base + (dst_udp->UdpL_Pos>>5);
    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
	for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
	    {
	    *dst_ptr++ = src_ptr->real * src_ptr->real + 
		src_ptr->imag *	src_ptr->imag;
	    src_ptr++;
	    }

    return (IpsX_SUCCESS);
    }

/******************************************************************************
**
**  FUNCTIONAL DESCRIPTION:
**
**	_IpsMagnitude creates a float magnitude spectrum from a complex src
**
**	Magnitude spectrum is defined as:
**
**	M[i] = sqrt (real[i] ** 2 + imag[i] ** 2)
**
**  FORMAL PARAMETERS:
**
**	src_udp		    source udp
**	dst_udp		    destination udp
**	flags		    not implemented
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
**	status
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/

long _IpsMagnitude (src_udp, dst_udp, flags)
struct UDP	*src_udp;		    /* source udp		    */
struct UDP	*dst_udp;		    /* dst udp			    */
unsigned long	flags;			    /* none defined		    */
    {
    unsigned long	mem_alloc;
    long		status;
    unsigned long	ix, iy;
    COMPLEX		*src_ptr;
    float		*dst_ptr;

    if  (src_udp->UdpB_Class != UdpK_ClassA)
	return (IpsX_UNSOPTION);
 
    if (src_udp->UdpB_DType != UdpK_DTypeC)
	return (IpsX_INVDTYPE);

    status = build_dst (src_udp, dst_udp, &mem_alloc);
    if (status != IpsX_SUCCESS)
	return (status);

    src_ptr = (COMPLEX *)src_udp->UdpA_Base + (src_udp->UdpL_Pos>>6);
    dst_ptr = (float *)dst_udp->UdpA_Base + (dst_udp->UdpL_Pos>>5);
    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
	for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
	    {
	    *dst_ptr++ = sqrt ((double)(src_ptr->real * src_ptr->real + 
		src_ptr->imag *	src_ptr->imag));
	    src_ptr++;
	    }

    return (IpsX_SUCCESS);
    }

/******************************************************************************
**
**  FUNCTIONAL DESCRIPTION:
**
**	_IpsLogMagnitude creates a float log magnitude spectrum from a complex 
**	src
**
**	Log Magnitude spectrum is defined as:
**
**	LM[i] = log (sqrt (real[i] ** 2 + imag[i] ** 2) + 1)
**
**  FORMAL PARAMETERS:
**
**	src_udp		    source udp
**	dst_udp		    destination udp
**	flags		    not implemented
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
**	status
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/

long _IpsLogMagnitude (src_udp, dst_udp, flags)
struct UDP	*src_udp;		    /* source udp		    */
struct UDP	*dst_udp;		    /* dst udp			    */
unsigned long	flags;			    /* none defined		    */
    {
    unsigned long	mem_alloc;
    long		status;
    unsigned long	ix, iy;
    COMPLEX		*src_ptr;
    float		*dst_ptr;
    double		z;

    if  (src_udp->UdpB_Class != UdpK_ClassA)
	return (IpsX_UNSOPTION);
 
    if (src_udp->UdpB_DType != UdpK_DTypeC)
	return (IpsX_INVDTYPE);

    status = build_dst (src_udp, dst_udp, &mem_alloc);
    if (status != IpsX_SUCCESS)
	return (status);

    src_ptr = (COMPLEX *)src_udp->UdpA_Base + (src_udp->UdpL_Pos>>6);
    dst_ptr = (float *)dst_udp->UdpA_Base + (dst_udp->UdpL_Pos>>5);
    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
	for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
	    {
	    z = sqrt ((double)(src_ptr->real * src_ptr->real + 
		src_ptr->imag *	src_ptr->imag));
	    *dst_ptr++ = (float)log(z + 1.0);
	    src_ptr++;
	    }

    return (IpsX_SUCCESS);
    }

/******************************************************************************
**
**  FUNCTIONAL DESCRIPTION:
**
**	_IpsPhase creates the phase from a complex src
**
**	Phase spectrum is defined as:
**
**	Phase[i] = atan (imag[i] / real[i]);
**
**  FORMAL PARAMETERS:
**
**	src_udp		    source udp
**	dst_udp		    destination udp
**	flags		    not implemented
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
**	status
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/

long _IpsPhase (src_udp, dst_udp, flags)
struct UDP	*src_udp;		    /* source udp		    */
struct UDP	*dst_udp;		    /* dst udp			    */
unsigned long	flags;			    /* none defined		    */
    {
    unsigned long	mem_alloc;
    long		status;
    unsigned long	ix, iy;
    COMPLEX		*src_ptr;
    float		*dst_ptr;

    if  (src_udp->UdpB_Class != UdpK_ClassA)
	return (IpsX_UNSOPTION);
 
    if (src_udp->UdpB_DType != UdpK_DTypeC)
	return (IpsX_INVDTYPE);

    status = build_dst (src_udp, dst_udp, &mem_alloc);
    if (status != IpsX_SUCCESS)
	return (status);

    src_ptr = (COMPLEX *)src_udp->UdpA_Base + (src_udp->UdpL_Pos>>6);
    dst_ptr = (float *)dst_udp->UdpA_Base + (dst_udp->UdpL_Pos>>5);
    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
	for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
	    {
	    *dst_ptr++ = atan ((double)(src_ptr->imag / src_ptr->real));
	    src_ptr++;
	    }

    return (IpsX_SUCCESS);
    }

/******************************************************************************
**
**  _IpsExtractComp
**
**  FUNCTIONAL DESCRIPTION:
**
**      Create a float udp which is either the real or imaginary component of 
**	the src udp
**
**  FORMAL PARAMETERS:
**
**	src_udp     --> src plane (complex)
**	dst_udp	    --> dst plane (float)
**	comp flag   --> IpsK_Real, IpsK_Imag
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**       none
**
**  FUNCTION VALUE:
**
**      long status
**
**  RESTRICTIONS:
**
**	ROIs not supported for frequency domain routines
**
*******************************************************************************/

long _IpsExtractComp (src_udp, dst_udp, flags)
struct UDP  *src_udp;		    /* complex src input    */
struct UDP  *dst_udp;		    /* float output	    */
unsigned long	flags;		    /* real or imag	    */
    {
    float	    *f_ptr;
    COMPLEX	    *src_ptr;
    float	    *dst_ptr;
    long	    status;
    unsigned long   i, j;
    unsigned long   size;
    unsigned long   mem_alloc=0;

    if  (src_udp->UdpB_Class != UdpK_ClassA)
	return (IpsX_UNSOPTION);
 
    if (src_udp->UdpB_DType !=  UdpK_DTypeC)
	return (IpsX_INVDTYPE);

    /* fill in the destination UDP of type FLOAT (no retain) */

    dst_udp->UdpB_Class = src_udp->UdpB_Class;
    dst_udp->UdpB_DType = UdpK_DTypeF;
    dst_udp->UdpW_PixelLength = sizeof(float) << 3;
    dst_udp->UdpL_PxlStride = dst_udp->UdpW_PixelLength;
    dst_udp->UdpL_ScnCnt = src_udp->UdpL_ScnCnt;
    dst_udp->UdpL_PxlPerScn = src_udp->UdpL_PxlPerScn;
    dst_udp->UdpL_ScnStride = src_udp->UdpL_PxlPerScn * dst_udp->UdpL_PxlStride;
    dst_udp->UdpL_X1 = 0;
    dst_udp->UdpL_Y1 = 0;
    dst_udp->UdpL_X2 = src_udp->UdpL_PxlPerScn - 1;
    dst_udp->UdpL_Y2 = src_udp->UdpL_ScnCnt - 1;
    dst_udp->UdpL_CompIdx = 0;
    /* levels is not used for floating point */
    dst_udp->UdpL_Levels = 0;
    size = src_udp->UdpL_PxlPerScn * src_udp->UdpL_ScnCnt * 
	dst_udp->UdpL_PxlStride;
    if (dst_udp->UdpA_Base == 0)
        {
        dst_udp->UdpA_Base = (unsigned char *)
	    (*IpsA_MemoryTable[IpsK_AllocateDataPlane])
		((size >> 3),0,0);
	if (!dst_udp->UdpA_Base) return (IpsX_INSVIRMEM);
	dst_udp->UdpL_ArSize = size;
	dst_udp->UdpL_Pos = 0;
	mem_alloc = 1;
	}
    else
        {
	if (src_udp->UdpB_DType != UdpK_DTypeF)
	    {
	    /* No in place allowed */
	    status = _IpsVerifyNotInPlace (src_udp, dst_udp);
	    if (status != IpsX_SUCCESS) return (status);
	    }
	if ((dst_udp->UdpL_ArSize - dst_udp->UdpL_Pos) < size)
	    return (IpsX_INSVIRMEM);
        }

    src_ptr = (COMPLEX *)src_udp->UdpA_Base + (src_udp->UdpL_Pos>>6);
    dst_ptr = (float *)dst_udp->UdpA_Base + (dst_udp->UdpL_Pos>>5); 

    if (flags == IpsK_Real)
	{
	for (j=0; j < dst_udp->UdpL_ScnCnt; j++) 
	    {
	    for (i=0; i< dst_udp->UdpL_PxlPerScn; i++) 
		{
		*dst_ptr++ = src_ptr[i].real;
		}
	    src_ptr += src_udp->UdpL_PxlPerScn;
	    }
	}
    else
	{
	for (j=0; j < dst_udp->UdpL_ScnCnt; j++) 
	    {
	    for (i=0; i< dst_udp->UdpL_PxlPerScn; i++) 
		{
		*dst_ptr++ = src_ptr[i].imag;
		}
	    src_ptr += src_udp->UdpL_PxlPerScn;
	    }
	}

    return (IpsX_SUCCESS);
    }


/******************************************************************************
**
**  build_dst
**
**  FUNCTIONAL DESCRIPTION:
**
**      Fills dst udp based on the src udp
**
**  FORMAL PARAMETERS:
**
**	src_udp     --> src plane 
**	dst_udp	    --> dst plane 
**	mem_alloc   --> returned value to indicate if any memory was allocated
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**       none
**
**  FUNCTION VALUE:
**
**      long status
**
**  RESTRICTIONS:
**
**	ROIs not supported for frequency domain routines
**
*******************************************************************************/

long build_dst (src_udp, dst_udp, mem_alloc)
struct UDP	*src_udp;
struct UDP	*dst_udp;
unsigned long	*mem_alloc;
    {
    unsigned long   size;
    long	    status;

    *mem_alloc= 0;

    /*
    ** create float output dst udp - no retain
    */
    dst_udp->UdpB_Class = src_udp->UdpB_Class;
    dst_udp->UdpB_DType = UdpK_DTypeF;
    dst_udp->UdpW_PixelLength = sizeof(float) << 3;
    dst_udp->UdpL_PxlStride = dst_udp->UdpW_PixelLength;
    dst_udp->UdpL_ScnCnt = src_udp->UdpL_ScnCnt;
    dst_udp->UdpL_PxlPerScn = src_udp->UdpL_PxlPerScn;
    dst_udp->UdpL_ScnStride = src_udp->UdpL_PxlPerScn * dst_udp->UdpL_PxlStride;
    dst_udp->UdpL_X1 = 0;
    dst_udp->UdpL_Y1 = 0;
    dst_udp->UdpL_X2 = src_udp->UdpL_PxlPerScn - 1;
    dst_udp->UdpL_Y2 = src_udp->UdpL_ScnCnt - 1;
    dst_udp->UdpL_CompIdx = 0;
	/* levels is not used for floating point */
    size = src_udp->UdpL_PxlPerScn * src_udp->UdpL_ScnCnt * 
	dst_udp->UdpL_PxlStride;

    if (dst_udp->UdpA_Base == 0)
        {
        dst_udp->UdpA_Base = (unsigned char *)
	    (*IpsA_MemoryTable[IpsK_AllocateDataPlane])
		((size >> 3),0,0);
	if (!dst_udp->UdpA_Base) 
	    return (IpsX_INSVIRMEM);
	dst_udp->UdpL_ArSize = size;
	dst_udp->UdpL_Pos = 0;
	*mem_alloc = 1;
	}
    else
        {
	/* No in place allowed */
	status = _IpsVerifyNotInPlace (src_udp, dst_udp);
	if (status != IpsX_SUCCESS) 
	    return (status);
	if ((dst_udp->UdpL_ArSize - dst_udp->UdpL_Pos) < size)
	    return (IpsX_INSVIRMEM);
        }
    return (IpsX_SUCCESS);

    }

/******************************************************************************
**
**  _IpsVerifySizeFFT
**
**  FUNCTIONAL DESCRIPTION:
**
**      Verifies that the FFT size is a power of two and returns the power
**
**  FORMAL PARAMETERS:
**
**	size --> dimension to check
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**       none
**
**  FUNCTION VALUE:
**
**      long nu --> log 2
**
*******************************************************************************/

long _IpsVerifySizeFFT (size)
long	size;
    {
    unsigned long n, nu;

    for (n = size, nu = -1; n != 0; nu++)
	n = n >> 1;
    if (((1<<nu) != size) || (nu == -1))
	return (-1);
    else
	return (nu);
    }
