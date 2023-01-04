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
**      _IpsFFT performs the 2D FFT on the the src udp data plane
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
**      October 11, 1990
**
**  COMMENTS:
**
**	This code represents the initial draft of the Fast Fourier Transform
**	implementation.
**
**	The 2-dimensional FFT is built upon the 1-dimensional FFT
**	The flow depends on which direction of the FFT is desired.  For Forward
**	transformations, the FFT of each row is taken then the resultant columns
**	are transformed.  The Inverse FFT is the same as the Forward except
**	that the conjugate of the incoming data and the resultant data is taken.
**	The last conjugate is performed while doing the column transforms.
**	In both cases the results are divided by the factor N - this maintains
**	symmetry in both directions and allows using the common 1D FFT.
**
**	These routines have several possibilities for performance improvements (but 
**	with tradeoffs):
**
**	- the taking of any data type for forward FFT taking advantage of
**	  Hermitian symmetry for real data.
**	- possible elimination of centering DC term which causes a complete
**	  pass through the data but allows for more visually useful power
**	  spec and magnitude images.  Note: elimination of the centered DC
**	  term will cause the filtering routines to change.
**
************************************************************************/

/*
**  Table of contents
*/
#ifdef NODAS_PROTO
long	_IpsForwardFFT();		/* IPS Forward FFT		      */
long	_IpsInverseFFT();		/* IPS Inverse FFT		      */
long	_IpsInverseRealFFT();		/* IPS Inverse FFT producing real     */
void	IpsFFT_1D();		/* straight 1 dimensional FFT		      */
void	IpsShiftForCenterDc();	/* shift data for centering DC term	      */
#endif

/*
** External References
*/
#ifdef NODAS_PROTO
long _IpsPromoteDataType();	/* from ips__promote_data_type		      */
long _IpsGetStartAddress();	/* from udp_utils			      */
long _IpsCopy();		
long _IpsBuildDstUdp();		/* from udp_utils			      */
long _IpsVerifyNotInPlace();	/* from udp_utils			      */
long _IpsVerifySizeFFT();	/* verify fft size, returned power of 2	      */
#endif
				/* from ips__freq_utils			      */
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
# define PI 3.141592654

/******************************************************************************
**
**  FUNCTIONAL DESCRIPTION:
**
**	This function applies the two dimensional forward FFT to the src udp.  
**	The algorithm operates in place once a complex representation of
**	the source is achieved by either copying or promoting the src data
**	unless the src is already complex.  If the src is complex, the dst 
**	base would be the gating item whether or not a "user's in place" is 
**	done, otherwise a copy is performed prior to implementation.
**
**	NOTE: src udp dimensions must be equal and be powers of two otherwise 
**	      an error is returned
**
**  The option to center the DC term is applied to the incoming data.
**
**  The equations are:
**
**  F(u,v) = 1/ N (sigma_1 sigma_2 (f(x,y) * exp (-j * 2 * PI * (ux + vy)/N))
**
**  where:
**
**	sigma_1 = sum of x from 0 to N - 1
**	sigma_2 = sum of y from 0 to N - 1
**	f(x,y)  = original pixel at x,y
**	F(u,v)  = Fourier coefficient at u,v
**
**  NOTE: the equations are to illustrate how N and the use of conjugates
**	  apply.
**
**  FORMAL PARAMETERS:
**
**	src_udp		    source udp
**	dst_udp		    destination udp
**	flags		    processing flags
**			    IpsM_FFTCenterDC -> force centering of DC term
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
******************************************************************************/
long _IpsForwardFFT (src_udp, dst_udp, flags)
struct UDP	*src_udp;		    /* source udp		    */
struct UDP	*dst_udp;		    /* dst udp			    */
unsigned long	flags;			    /* forward or backward	    */
    {
    unsigned long 	mem_alloc = 0;
    long		status;
    unsigned long	ix, iy;
    COMPLEX		*dst_ptr;
    COMPLEX		*col_ptr;
    COMPLEX		*s_ptr, *d_ptr;
    unsigned long	n;
    long		nu;
    unsigned long	src_start, dst_start;

    /* 
    ** do error checking before commencing 
    */
    if  (src_udp->UdpB_Class != UdpK_ClassA)
	return (IpsX_UNSOPTION);

    /*
    ** if user has dst base set flag
    */
    if (dst_udp->UdpA_Base == 0)
	mem_alloc = 1;

    /* 
    ** check udp dimensions for equality and powers of two
    */
    if (src_udp->UdpL_PxlPerScn != src_udp->UdpL_ScnCnt)
	return (IpsX_INVDARG);

    nu = _IpsVerifySizeFFT (src_udp->UdpL_PxlPerScn);
    if (nu == -1)
	return (IpsX_UNSIMGDIM);
    n = src_udp->UdpL_PxlPerScn;  /* fft sample points	*/

    /* 
    ** convert to complex if not complex otherwise make copy of src unless 
    ** its in place (i.e. dst address = src address) in which case we just 
    ** "build" dst udp
    */
    if (src_udp->UdpB_DType != UdpK_DTypeC)
	/* 
	** non-complex in - promote 
	** Note: Promote will not allow in place operation thus it will
	** be properly flagged
	*/
	status = _IpsPromoteDataType (src_udp, dst_udp, IpsK_DTypeC, 0); 
    else
	{
	/* complex in */
	status = _IpsGetStartAddress (src_udp, &src_start);
	dst_start = (unsigned long)(COMPLEX *)dst_udp->UdpA_Base + 
	    (dst_udp->UdpL_Pos>>6);
	if (src_start != dst_start || dst_udp->UdpA_Base == 0)
	    /*
	    ** not in place requested
	    */
	    status = _IpsCopy (src_udp, dst_udp, 0);
	else
	    /* Define dst fields */
	    status = _IpsBuildDstUdp (src_udp,dst_udp,0,0,IpsK_InPlaceAllowed);
	}
    if (status != IpsX_SUCCESS)
	return;

    /* 
    ** See if caller wishes to center the DC term
    */
    if ((flags & IpsM_FFTCenterDC))
	IpsShiftForCenterDc (dst_udp);
		
    /* 
    ** take forward FFT of each row 
    */
    dst_ptr = (COMPLEX *)dst_udp->UdpA_Base + (dst_udp->UdpL_Pos>>6);
    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
	{
	IpsFFT_1D (dst_ptr, n, nu);
	dst_ptr += dst_udp->UdpL_PxlPerScn;
	}	/* end of row transform */

    /* 
    ** take forward FFT of each resultant column,
    ** allocate working buffer of size equal to number of rows - elininates
    ** need for strides in FFT algorithm
    */
    col_ptr = (COMPLEX *)(*IpsA_MemoryTable[IpsK_Alloc])(sizeof(COMPLEX) *
	dst_udp->UdpL_ScnCnt, 0, 0);
    if (!col_ptr)
	{
        if (mem_alloc == 1)
	    (*IpsA_MemoryTable[IpsK_FreeDataPlane]) (dst_udp->UdpA_Base);
	return (IpsX_INSVIRMEM);
	}

    dst_ptr = (COMPLEX *)dst_udp->UdpA_Base + (dst_udp->UdpL_Pos>>6);
    for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
	{
	/* 
	** move column into sequential working buffer 
	*/
	for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
	    *(col_ptr+iy) = *(dst_ptr+(iy*dst_udp->UdpL_PxlPerScn));
	/*
	** take fft
	*/
	IpsFFT_1D (col_ptr, dst_udp->UdpL_ScnCnt, nu);
	
	/* 
	** move results back from working buffer to column dividing by n on 
	** the fly
	*/
	for (s_ptr = dst_ptr, d_ptr = col_ptr,
	    iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++, 
		s_ptr += dst_udp->UdpL_PxlPerScn, d_ptr++)
		    {
		    s_ptr->real = d_ptr->real / (float)n;
		    s_ptr->imag = d_ptr->imag / (float)n;
		    }
	dst_ptr++;
	}	/* end of column transform */

    /* 
    ** free allocated working buffer
    */
    (*IpsA_MemoryTable[IpsK_Dealloc])(col_ptr);    
    return (IpsX_SUCCESS);
    }

/******************************************************************************
**
**  FUNCTIONAL DESCRIPTION:
**
**	This function applies the two dimensional inverse FFT to the src udp.  
**	The algorithm operates in place once a complex representation of
**	the source is achieved by either copying or promoting the src data
**	unless the src is already complex.  If the src is complex, the dst 
**	base would be the gating item whether or not a "user's in place" is 
**	done, otherwise a copy is performed prior to implementation.
**
**	NOTE: src udp dimensions must be equal and be powers of two otherwise 
**	      an error is returned
**
**  The option to center the DC term is applied to the incoming data.
**
**  The equation is:
**
**  f_(x,y) = 1/ N (sigma_1 sigma_2 (F_(u,v) * exp (-j * 2 * PI * (ux + vy)/N))
**
**  where:
**
**	sigma_1 = sum of u from 0 to N - 1
**	sigma_2 = sum of v from 0 to N - 1
**	f_(x,y) = conjugate of pixel at (x,y)
**	F_(u,v) = conjugate of the Fourier coefficient at u,v
**
**  NOTE: the equation is to illustrate how N and the use of conjugates
**	  apply.
**
**  FORMAL PARAMETERS:
**
**	src_udp		    source udp
**	dst_udp		    destination udp
**	flags		    processing flags
**			    IpsM_FFTCenterDC -> force centering of DC term
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
******************************************************************************/
long _IpsInverseFFT (src_udp, dst_udp, flags)
struct UDP	*src_udp;		    /* source udp		    */
struct UDP	*dst_udp;		    /* dst udp			    */
unsigned long	flags;			    /* forward or backward	    */
    {
    unsigned long 	mem_alloc = 0;
    long		status;
    unsigned long	ix, iy;
    COMPLEX		*dst_ptr;
    COMPLEX		*col_ptr;
    COMPLEX		*s_ptr, *d_ptr;
    unsigned long	n;
    long		nu;
    unsigned long	src_start, dst_start;

    /* 
    ** do error checking before commencing 
    */
    if  (src_udp->UdpB_Class != UdpK_ClassA)
	return (IpsX_UNSOPTION);

    /*
    ** if user has dst base set flag
    */
    if (dst_udp->UdpA_Base == 0)
	mem_alloc = 1;

    /* 
    ** check udp dimensions for equality and powers of two
    */
    if (src_udp->UdpL_PxlPerScn != src_udp->UdpL_ScnCnt)
	return (IpsX_INVDARG);

    nu = _IpsVerifySizeFFT (src_udp->UdpL_PxlPerScn);
    if (nu == -1)
	return (IpsX_UNSIMGDIM);
    n = src_udp->UdpL_PxlPerScn;  /* fft sample points	*/

    /* 
    ** convert to complex if not complex otherwise make copy of src unless 
    ** its in place (i.e. dst address = src address) in which case we just 
    ** "build" dst udp
    */
    if (src_udp->UdpB_DType != UdpK_DTypeC)
	/* 
	** non-complex in - promote 
	** Note: Promote will not allow in place operation thus it will
	** be properly flagged
	*/
	status = _IpsPromoteDataType (src_udp, dst_udp, IpsK_DTypeC, 0); 
    else
	{
	/* complex in */
	status = _IpsGetStartAddress (src_udp, &src_start);
	dst_start = (unsigned long)(COMPLEX *)dst_udp->UdpA_Base + 
	    (dst_udp->UdpL_Pos>>6);
	if (src_start != dst_start || dst_udp->UdpA_Base == 0)
	    /*
	    ** not in place requested
	    */
	    status = _IpsCopy (src_udp, dst_udp, 0);
	else
	    /* Define dst fields */
	    status = _IpsBuildDstUdp (src_udp,dst_udp,0,0,IpsK_InPlaceAllowed);
	}
    if (status != IpsX_SUCCESS)
	return;

    /*
    ** First take conjugate 
    */
    dst_ptr = (COMPLEX *)dst_udp->UdpA_Base + (dst_udp->UdpL_Pos>>6);
    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
        for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++, dst_ptr++)
	    dst_ptr->imag *= -1;	    
	    
    /* 
    ** take inverse FFT of each row 
    */
    dst_ptr = (COMPLEX *)dst_udp->UdpA_Base + (dst_udp->UdpL_Pos>>6);
    for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
	{
	IpsFFT_1D (dst_ptr, n, nu);
	dst_ptr += dst_udp->UdpL_PxlPerScn;
	}	/* end of row transform */

    /* 
    ** take inverse FFT of each resultant column,
    ** allocate working buffer of size equal to number of rows - elininates
    ** need for strides in FFT algorithm
    */
    col_ptr = (COMPLEX *)(*IpsA_MemoryTable[IpsK_Alloc])(sizeof(COMPLEX) *
	dst_udp->UdpL_ScnCnt, 0, 0);
    if (!col_ptr)
	{
        if (mem_alloc == 1)
	    (*IpsA_MemoryTable[IpsK_FreeDataPlane]) (dst_udp->UdpA_Base);
	return (IpsX_INSVIRMEM);
	}

    dst_ptr = (COMPLEX *)dst_udp->UdpA_Base + (dst_udp->UdpL_Pos>>6);
    for (ix = 0; ix < dst_udp->UdpL_PxlPerScn; ix++)
	{
	/* 
	** move column into sequential working buffer 
	*/
	for (iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++)
	    *(col_ptr+iy) = *(dst_ptr+(iy*dst_udp->UdpL_PxlPerScn));
	/*
	** take fft
	*/
	IpsFFT_1D (col_ptr, dst_udp->UdpL_ScnCnt, nu);
	
	/* 
	** move results back from working buffer to column dividing by n on 
	** the fly, take conjugate on the fly as well
	*/
	for (s_ptr = dst_ptr, d_ptr = col_ptr,
	    iy = 0; iy < dst_udp->UdpL_ScnCnt; iy++, 
		s_ptr += dst_udp->UdpL_PxlPerScn, d_ptr++)
		    {
		    s_ptr->real = d_ptr->real / (float)n;
		    s_ptr->imag = -1.0 * d_ptr->imag / (float)n;
		    }
	    
	dst_ptr++;
	}	/* end of column transform */

    /* 
    ** if user had dc in center then shift back
    */
    if ((flags & IpsM_FFTCenterDC))
	IpsShiftForCenterDc (dst_udp);

    /* 
    ** free allocated working buffer
    */
    (*IpsA_MemoryTable[IpsK_Dealloc])(col_ptr);    
    return (IpsX_SUCCESS);
    }

/******************************************************************************
**
**  FUNCTIONAL DESCRIPTION:
**
**	This function applies the two dimensional inverse FFT to the src udp.  
**	The src must be complex.  The dst will always be float.  In place is
**	not supported.  The src will be destroyed.  
**
**	This routine is utilized for frequency filtering for higher efficiency
**	when it is assured that the result is real.
**
**  The option to center the DC term is applied to the incoming data.
**
**  The equation is:
**
**  f_(x,y) = 1/ N (sigma_1 sigma_2 (F_(u,v) * exp (-j * 2 * PI * (ux + vy)/N))
**
**  where:
**
**	sigma_1 = sum of u from 0 to N - 1
**	sigma_2 = sum of v from 0 to N - 1
**	f_(x,y) = conjugate of pixel at (x,y)
**	F_(u,v) = conjugate of the Fourier coefficient at u,v
**
**  NOTE: the equation is to illustrate how N and the use of conjugates
**	  apply.
**
**  FORMAL PARAMETERS:
**
**	src_udp		    source udp
**	dst_udp		    destination udp
**	flags		    processing flags
**			    IpsM_FFTCenterDC -> force centering of DC term
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
******************************************************************************/
long _IpsInverseRealFFT (src_udp, dst_udp, flags)
struct UDP	*src_udp;		    /* source udp		    */
struct UDP	*dst_udp;		    /* dst udp			    */
unsigned long	flags;			    /* forward or backward	    */
    {
    unsigned long 	mem_alloc = 0;
    unsigned long	size;
    long		status;
    unsigned long	ix, iy;
    float		*dst_ptr, *d2_ptr;
    COMPLEX		*src_ptr,*col_ptr;
    COMPLEX		*s_ptr, *d_ptr;
    unsigned long	n;
    long		nu;
    unsigned long	src_start, dst_start;

    /* 
    ** do error checking before commencing 
    */
    if  ((src_udp->UdpB_Class != UdpK_ClassA) || 
	(src_udp->UdpB_DType != UdpK_DTypeC))
	    return (IpsX_UNSOPTION);

    /* 
    ** check udp dimensions for equality and powers of two
    */
    if (src_udp->UdpL_PxlPerScn != src_udp->UdpL_ScnCnt)
	return (IpsX_INVDARG);

    nu = _IpsVerifySizeFFT (src_udp->UdpL_PxlPerScn);
    if (nu == -1)
	return (IpsX_UNSIMGDIM);
    n = src_udp->UdpL_PxlPerScn;  /* fft sample points	*/

    /*
    ** build float udp
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

    /*
    ** First take conjugate 
    */
    src_ptr = (COMPLEX *)src_udp->UdpA_Base + (src_udp->UdpL_Pos>>6);
    for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
        for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++, src_ptr++)
	    src_ptr->imag *= -1;	    
	    
    /* 
    ** take inverse FFT of each row 
    */
    src_ptr = (COMPLEX *)src_udp->UdpA_Base + (src_udp->UdpL_Pos>>6);
    for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
	{
	IpsFFT_1D (src_ptr, n, nu);
	src_ptr += src_udp->UdpL_PxlPerScn;
	}		/* end of row transform */

    /* 
    ** take inverse FFT of each resultant column,
    ** allocate working buffer of size equal to number of rows - elininates
    ** need for strides in FFT algorithm
    */
    col_ptr = (COMPLEX *)(*IpsA_MemoryTable[IpsK_Alloc])(sizeof(COMPLEX) *
	src_udp->UdpL_ScnCnt, 0, 0);
    if (!col_ptr)
	{
        if (mem_alloc == 1)
	    (*IpsA_MemoryTable[IpsK_FreeDataPlane]) (src_udp->UdpA_Base);
	return (IpsX_INSVIRMEM);
	}

    src_ptr = (COMPLEX *)src_udp->UdpA_Base + (src_udp->UdpL_Pos>>6);
    dst_ptr = (float *)dst_udp->UdpA_Base + (dst_udp->UdpL_Pos>>6);

    for (ix = 0; ix < src_udp->UdpL_PxlPerScn; ix++)
	{
	/* 
	** move column into sequential working buffer 
	*/
	for (iy = 0; iy < src_udp->UdpL_ScnCnt; iy++)
	    *(col_ptr+iy) = *(src_ptr+(iy*src_udp->UdpL_PxlPerScn));
	/*
	** take fft
	*/
	IpsFFT_1D (col_ptr, src_udp->UdpL_ScnCnt, nu);
	
	/* 
	** move results back to float dst udp from working buffer to column 
	** dividing by n on the fly
	*/
	for (d2_ptr = dst_ptr, d_ptr = col_ptr,
	    iy = 0; iy < src_udp->UdpL_ScnCnt; iy++, 
		d2_ptr += dst_udp->UdpL_PxlPerScn, d_ptr++)
		    *d2_ptr = d_ptr->real / (float)n;
	    
	src_ptr++;			/* next column */
	dst_ptr++;
	}				/* end of column transform */

    /* 
    ** if user had dc in center then shift back
    */
    if ((flags & IpsM_FFTCenterDC))
	IpsShiftForCenterDc (dst_udp);

    /* 
    ** free allocated working buffer
    */
    (*IpsA_MemoryTable[IpsK_Dealloc])(col_ptr);    
    return (IpsX_SUCCESS);
    }

/******************************************************************************
**
**  FUNCTIONAL DESCRIPTION:
**
**	IpsFFT_1D takes a (one dimensional) Fast Fourier Transform
**	of a complex data stream.  Results overwrite the input.
**	The FFT is a fast method to finding the Discrete Fourier Transform 
**	which (for N point transform) is defined as:
**
**	F(s) = sigma (f(x) * (e ** (-j * 2 * PI * s * x)/N))
**
**	where
**	
**	    sigma   = sum of x from 0 to N-1 
**	    f(x)    = the input data
**	    s	    = 0,1,...N-1
**	    F(s)    = the complex coefficent of the resultant transform
**
**	Note:
**	    e ** ((-j * 2 * PI * s * x) = cos(2*PI*s*x) - j*sin(2*PI*s*x)
**	    
**	This routine is independent of any IPS constructs.
**
**	For reference see any book on Imaging Processing using Frequency
**	Filtering.  This routine implements the Cooley and Tukey algorithm
**	of finding the Discrete Fourier Transform.
**
**  FORMAL PARAMETERS:
**
**	data 		    column or row of data 
**	n		    number of points of data (power of two)
**	ln		    2**ln = n
**
******************************************************************************/
void IpsFFT_1D (data, n, ln)
COMPLEX		data[];		    /* incoming data stream */
unsigned long	n;		    /* number of points	    */
unsigned long	ln;		    /* power of 2 for n	    */
    {
    COMPLEX u,w,t,temp;		    /* working variables    */
    long    i,j,k,l,le, le1, ip;
    long    mid, nm1;

    mid = n >> 1;
    nm1 = n - 1;

    /*
    ** reorder the data for successive doubling (2,4,8..n transforms)
    */
    j = 0;
    for (i = 0; i < nm1; i++)
	{
	if ( i < j)
	    {
	    t = data[j];
	    data[j] = data[i];
	    data[i] = t;
	    }
	k = mid;
	while (k <= j)
	    {
	    j = j - k;
	    k = k>>1;
	    }
	j += k;
	}

    /*
    ** Implement successive doubling
    */
    for (le = 2, l = 0; l < ln; l++, le*=2)
	{
	le1 = le >> 1;

	u.real = 1.0;
	u.imag = 0.0;

	w.real = cos (PI/(double)le1);
	w.imag = -1.0 * (sin (PI/(double)le1));

	for (j = 0; j < le1; j++)
	    {
	    for (i = j; i < n; i+=le)
		{
		ip = i + le1;
		COMPLEX_MULTIPLY_(data[ip],u,t);
		COMPLEX_SUBTRACT_(data[i],t,data[ip]);
		COMPLEX_ADD_(data[i],t,data[i]);
		}
	    COMPLEX_MULTIPLY_(u,w,temp);
	    u = temp;
	    }
	COMPLEX_MULTIPLY_(u,w,temp);
	u = temp;
	}
    return;
    }

/******************************************************************************
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine is called when the user chooses to center the DC term
**	Used for both forward and inverse FFTs.  The routines implements:
**
**	    new_data = old_data * (-1.0)**(ix+iy)
**
**		where i and j are the columns and rows
**
**  FORMAL PARAMETERS:
**
**	udp		    pointer to incoming data for centering DC
**
******************************************************************************/
void IpsShiftForCenterDc (udp)
struct UDP *udp;		    /* src udp	*/
    {
    COMPLEX	    *c1_ptr, *c2_ptr;
    float	    *f1_ptr, *f2_ptr;
    unsigned long   ix, iy;
    long	    start = 1;

    if (udp->UdpB_DType == UdpK_DTypeC)
	{
	c1_ptr = (COMPLEX *)udp->UdpA_Base + (udp->UdpL_Pos>>6);
	for (iy = 0; iy < udp->UdpL_ScnCnt; iy++, start ^= 1, 
	    c1_ptr+=udp->UdpL_PxlPerScn)
	    {
	    c2_ptr = c1_ptr + start;
	    for (ix = start; ix < udp->UdpL_PxlPerScn; ix+=2, c2_ptr+=2)
		{
		c2_ptr->real *= -1.0;
		c2_ptr->imag *= -1.0;
		}
	    }
	}
    else
	{
	f1_ptr = (float *)udp->UdpA_Base + (udp->UdpL_Pos>>5);
	for (iy = 0; iy < udp->UdpL_ScnCnt; iy++, start ^= 1,
	    f1_ptr+=udp->UdpL_PxlPerScn)
	    {
	    f2_ptr = f1_ptr + start;
	    for (ix = start; ix < udp->UdpL_PxlPerScn; ix+=2, f2_ptr+=2)
		*f2_ptr *= -1.0;
	    }
	}

    return;
    }
