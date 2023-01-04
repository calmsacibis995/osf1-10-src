/******************************************************************************
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
**      Perform an ordered dither on image.
**
**  ENVIRONMENT:
**
**      VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHOR(S):
**
**      Ken MacDonald, Digital Equipment Corp.
**	Rich Piccolo,  Digital Equipment Corp.
**	Modified for V3.0 by John Poltrack,  Digital Equipment Corp.
**
**
**  CREATION DATE:
**
**      January, 1988
**
************************************************************************/

/*
** Table of contents
*/
#ifdef NODAS_PROTO
long		_IpsDitherOrdered();	  /* frame to dithered frame        */
#endif

/*
**  Include files:
*/
#include <IpsDef.h>			    /* Image definitions	    */
#include <IpsStatusCodes.h>		    /* Status codes		    */
#include <IpsMemoryTable.h>		    /* IPS Memory Mgt. Functions    */
#include <IpsMacros.h>			    /* IPS Macros		    */
#ifndef NODAS_PROTO
#include <ipsprot.h>			    /* Ips prototypes		    */
#endif
#include <math.h>			    /* math routine definitions     */

/*
**  MACRO definitions:
*/

/*
**  Local variables
*/

/*
**  External References:
*/
#ifdef NODAS_PROTO
long _IpsVerifyNotInPlace();		    /* from Ips__udp_utils.c	    */
long _IpsBuildDstUdp();			    /* from IPS__UDP_UTILS.C	    */
#endif
/* 
** Internal routines
*/
#ifdef NODAS_PROTO
static long     _IpsOrderedByteBitonal(); /* Layer 2a dither bits           */
static long     _IpsOrderedByteByte();    /* Layer 2a dither bytes          */
static float Ips__cmatx();
static int   Ips__spiral();
static float Ips__dmatx();
#else
PROTO(static long _IpsOrderedByteBitonal, (struct UDP */*src_udp*/, struct UDP */*dst_udp*/, long /*dither_type*/, long /*threshold_spec*/));
PROTO(static long _IpsOrderedByteByte, (struct UDP */*src_udp*/, struct UDP */*dst_udp*/, long /*dither_type*/, long /*threshold_spec*/, long /*output_levels*/));
PROTO(static float Ips__cmatx, (int /*matrix*/[], int /*pxsize*/, int /*pysize*/));
PROTO(static int Ips__spiral, (int /*thresh*/[], int /*xsize*/, int /*ysize*/));
PROTO(static float Ips__dmatx, (int /*matrix*/[], int /*order*/, int /*size*/));
#endif

/************************************************************************
**
**  FUNCTIONAL DESCRIPTION:
**
**      Converts a frame to a dithered frame using the 
**         clustered dot or dispersed dot approach.
**
**  FORMAL PARAMETERS:
**
**	src_udp		- source udp
**	dst_udp		- destination udp
**	dither_type 	- clustered or dispersed ordered dither
**	threshold_spec 	- dither specification for the ordered dither algoritm
**	output_levels	- quantization levels for dithering
**	flags		- processing flag if dithering to 2 levels - default
**			  is keep dst same as src regarding dtype and class;
**			  otherwise IpsM_DitherToBitonal 
**
**  IMPLICIT INPUTS:
**
**       none
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
**	none
**
**  SIDE EFFECTS:
**
**      none
**
*************************************************************************/

/***************************************************************************/
/* Restrictions:                                                           */
/*    current restrictions:                                                */
/*                                                                         */
/*    byte image input only						   */
/*    decompression not performed                                          */
/*    grid type other than rectangular not supported                       */
/*    lookup tables not checked                                            */
/*    non 1:1 aspect ratio not supported                                   */
/*                                                                         */
/***************************************************************************/

long _IpsDitherOrdered(src_udp,dst_udp,dither_type,
			threshold_spec,output_levels,flags)
struct UDP  *src_udp;			  /* source udp			     */
struct UDP  *dst_udp;			  /* destination udp		     */
long	    dither_type;    		  /* clustered or dispersed          */
long	    threshold_spec;    		  /* m-factor (if clustered          */
				          /* order (if dispersed)	     */
long	    output_levels;		  /* dither output level	     */
unsigned long	flags;			  /* dither to bitonal flag	    */

{
unsigned long	byte_boundary_padbits;	  /* pad of bitonal udp to byte	     */
unsigned long	size;			  /* size in bytes (for allocation)   */
long		status;
unsigned long	mem_alloc = 0;

/*
** Validate source and type
*/
switch (src_udp->UdpB_Class)
    {
    case UdpK_ClassUBA:				
	/*
	** Bitonal UDP
	*/
	return(IpsX_INVDTHBIT);
	break;

    case UdpK_ClassA:
	/*
	** Atomic array
	*/
	switch (src_udp->UdpB_DType)
	    {
	    case UdpK_DTypeBU:
	        break;
	    case UdpK_DTypeWU:
	    case UdpK_DTypeLU:
	    case UdpK_DTypeF:
	    case UdpK_DTypeVU:
	    case UdpK_DTypeV:
	    default:
		return(IpsX_UNSOPTION);
		break;
	    }/* end switch on DType */
	break;
    case UdpK_ClassCL:
    case UdpK_ClassUBS:
    default:
	return(IpsX_UNSOPTION);
    break;
    }/* end switch on class */

/*
** Initialize destination udp fixed fields , calculate size in bytes of
** new data buffer and allocate memory
*/
if (output_levels == 2 && flags == IpsM_DitherToBitonal)
    {
    /*
    ** Dither to bitonal, the destination udp is padded out to the nearest
    ** byte boundary and the other fields are set to bitonal values
    */
    dst_udp->UdpL_PxlPerScn = src_udp->UdpL_PxlPerScn;
    dst_udp->UdpL_ScnCnt = src_udp->UdpL_ScnCnt;
    dst_udp->UdpL_X1 = 0;
    dst_udp->UdpL_Y1 = 0;
    dst_udp->UdpL_X2 = src_udp->UdpL_PxlPerScn - 1;
    dst_udp->UdpL_Y2 = src_udp->UdpL_ScnCnt - 1;
    byte_boundary_padbits = (8 - (src_udp->UdpL_PxlPerScn % 8)) % 8;
    dst_udp->UdpL_ScnStride = src_udp->UdpL_PxlPerScn + byte_boundary_padbits;
    dst_udp->UdpW_PixelLength = 1;
    dst_udp->UdpL_PxlStride = 1;
    dst_udp->UdpB_DType = UdpK_DTypeVU;
    dst_udp->UdpB_Class = UdpK_ClassUBA;
    size = dst_udp->UdpL_ScnStride * dst_udp->UdpL_ScnCnt;
    if (dst_udp->UdpA_Base != 0)
	{
	/* No in place allowed */
	status = _IpsVerifyNotInPlace (src_udp, dst_udp);
	if (status != IpsX_SUCCESS) return (status);
	if ((dst_udp->UdpL_ArSize - dst_udp->UdpL_Pos) < size)
	    return (IpsX_INSVIRMEM);
	}
    else
	{
	dst_udp->UdpA_Base = (unsigned char *)
	    (*IpsA_MemoryTable[IpsK_AllocateDataPlane])
		(((size + 7) >> 3),IpsM_InitMem, 0);
	if (!dst_udp->UdpA_Base) return (IpsX_INSVIRMEM);
	dst_udp->UdpL_ArSize = size;
	dst_udp->UdpL_Pos = 0;
	mem_alloc = 1;
	}
    }
else	    /* don't dither to bitonal */
    {
    /* No retain, no in place, no mem initialization */
    if (dst_udp->UdpA_Base == 0) mem_alloc = 1;
    status = _IpsBuildDstUdp (src_udp, dst_udp, 0, 0, 0);
    if (status != IpsX_SUCCESS) 
	return (status);
    }

/*
** set output levels
*/
dst_udp->UdpL_Levels = output_levels;


/*****************************************************************************
**	    Dispatch to layer 2a                                            **
*****************************************************************************/

    switch (src_udp->UdpB_DType)
	{
	case UdpK_DTypeBU:
	    if (output_levels == 2 && flags == IpsM_DitherToBitonal)
	    /* 
	    ** Note: Use byte to bitonal even if dithering from byte to 
	    ** bitonal
	    */
		status = _IpsOrderedByteBitonal(src_udp,dst_udp,dither_type,
		    threshold_spec);
	    else
	        status = _IpsOrderedByteByte(src_udp,dst_udp,dither_type,
                                           threshold_spec,output_levels);
	break;
	case UdpK_DTypeWU:
	case UdpK_DTypeLU:
	case UdpK_DTypeF:
	default:
	    return(IpsX_UNSOPTION);
	break;
	}/* end switch on DType */

if ((status != IpsX_SUCCESS) && (mem_alloc == 1))
    (*IpsA_MemoryTable[IpsK_FreeDataPlane]) (dst_udp->UdpA_Base);

return (status);

}/* end _IpsDitherOrdered */

/****************************************************************************
 * 	CMATX returns a theshold matrix specified in a file to be used 
 * 	to halftone by ordered dither.
 *      Comments and routine adapted from Bob Ulichney, DEC by Ken MacDonald
 *****************************************************************************/
static float Ips__cmatx(matrix,pxsize,pysize)
int           matrix[],pxsize,pysize;
    {
    int           max;
    int           i,j,ix,iy,icode,x,y,xsize,ysize;
    float         outval;
   
    xsize = pxsize;
    ysize = pysize;
   
    /* Adjust values to be returned */
    pxsize = 2 * xsize;
    pysize = 2 * ysize;

    /* Generate the threshold matrix */
    max = Ips__spiral(matrix,xsize,ysize);
    outval = 2 * max + 1;

    /* Now replicate other 3 quadrants */

    for(x=0; x<xsize; x++)
        for(y=0; y<ysize; y++)
            {
	    matrix[(x+xsize)+(y*pxsize)] = outval - matrix[x+(y*pxsize)];
	    matrix[x+((y+ysize)*pxsize)] = matrix[(x+xsize)+ (y*pxsize)];
	    matrix[(x+xsize)+((y+ysize)*pxsize)] = matrix[x+(y*pxsize)];
            }

    return(outval);
    }/* end Ips_cmatx */	

/****************************************************************************
 *      Ips__spiral()
 *      A subroutine to create a threshold matrix that grows like a
 *      spiral from the center of the matrix "thresh".  "xsize" and "ysixe"
 *      must be even numbers.  The subroutine returns the maximum threshold
 *      entered.
 *      Note that the aspect ratio for a hexagonal array should be X/Y = 6/5.
 *
 *      Adapted from routine by Bob Ulichney, DEC by Ken MacDonald.
 *      Original comments retained.
 *****************************************************************************/
#define	GROWTH 5 /* cycles per unit radius increase */
#define	DEL    2 /* steps per unit circumfrance */

static int Ips__spiral(thresh,xsize,ysize)
int thresh[],xsize,ysize;

    {
    int          x,y,ix,iy,max,maxval;
    double       PI,idelr,x0,y0,r,t,A,Anorm;
    int		 row_inc;
    
    PI = acos(-1.0);

    idelr = 1.0/(DEL*GROWTH*2*PI);       /* Note that there are 2*PI*DEL*r
					**   steps per cycle */
    x0 = (xsize/2.);                     /* center of matrix in coord. units */
    y0 = (ysize/2.);

    A = ((float)xsize)/ysize;            /* eccentricity factor */
    Anorm = (A<1.0)? 1.0 : A;            /* Adjustment of step size to 
					**  compensate radius increase */

    maxval = xsize*ysize;

    /* Generate the spiral. 
    **  Delta-t = 2PI/(#steps/cycle),  and  Delta-r = 1/(GROWTH*(#steps/cycle))
    */

    /* since we are creating one quadrant the array is incremented by 2*xsize */

    row_inc = 2 * xsize;

    for(t=0,r=.1*A,max=0; max<maxval; r+=idelr/r,t+=1/(DEL*r*Anorm))
        {
        x = r*A*sin(t) + x0;
        y = r*cos(t) + y0;
        if((x<xsize)&&(y<ysize)&&(thresh[x + (row_inc * y)]==0)) 
            thresh[x+ (row_inc * y)] = (++max);
        }

    return(max);
    }/* end Ips__spiral */

/****************************************************************************
*	Ips__dmatx()
*	DMATX returns a "homogeneous" threshold matrix to be used
*	to halftone by ordered dither on a rectangular grid.
 *****************************************************************************/
static float Ips__dmatx (matrix,order,size)
int  matrix[], order, size;

    {
    int xoff,yoff,eqnum,xnew,ynew;
    int	i,j,ix,iy,mcode,maxstp;
    float outval;

    /* set up variables */

    mcode = (order+1)/2;
    maxstp = 2*mcode;				  /* number of steps */
						  /* # of rows and/or columns */
    outval = size*size+1;			  /* available otput densities*/
    yoff = size/2;
    xoff = yoff;
    eqnum = 1;

    /* initalize the matrix */

    for (i=0; i<outval-1; i++) matrix[i] = -1;
    matrix[0] = 1;

    /* generate the matrix */

    for (i=0;i<maxstp; i++)
	{
	for (ix=0; ix<size; ix++)
	    for (iy=0; iy <size; iy++)
		if (matrix[ix + (iy * size)] >= 0)
		    {
		    xnew = (ix+xoff) % size;
		    ynew = (iy+yoff) % size;
		    if (matrix[xnew + (ynew * size)] < 0) 
			matrix[xnew+(ynew*size)] = matrix[ix+(iy*size)] + eqnum;
		    }
	if (xoff == 0) xoff = (yoff /=2);
	else xoff = 0;
	eqnum *= 2;
    }

    /* adjust for "half squares (if order is odd) */

    if (order & 1)
	{
	for (j= 0; j <size; j++)
	    for (i= 0; i <size/2; i++) ++matrix[i+j*size];

	for (i= 0; i <size; i++)
	    for (j = 0; j<size; j++) matrix[i+j*size] /= 2;

	outval = (size*size/2) + 1;
	}

    return (outval);
    }/* end Ips__dmatx */


/******************************************************************************
**
**  _IpsOrderedByteBitonal
**
**  FUNCTIONAL DESCRIPTION:
**
**      Ordered dither from byte to bit
**
**  FORMAL PARAMETERS:
**
**	src_udp -->	pointer to source	    udp (initialized)
**	dst_udp -->	pointer to destination  udp (uninitialized)
**	dither_type	clustered or dispersed
**	threshold	m-factor(if clustered) or order(if dispersed)
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      long status
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**      none
**
*******************************************************************************/

/******************************************************************************/
/* RESTRICTIONS:                                                              */
/*    current restrictions:						      */
/*                                                                            */
/*    byte udp's only. class is UBA					      */
/*                                                                            */
/******************************************************************************/

static long _IpsOrderedByteBitonal(src_udp,dst_udp,dither_type,threshold_spec)
struct UDP  *src_udp;			  /* source udp			     */
struct UDP  *dst_udp;			  /* destination udp		     */
long	    dither_type;    		  /* clustered or dispersed          */
long	    threshold_spec;    		  /* m-factor (if clustered          */
				          /* order (if dispersed)	     */

{
int	    *mat;			  /* start of dither matrix	      */
int	    *mat_row_start;		  /* start of current dither mtrx row */
int	    *mat_row_end;		  /* end of current dither mtrx row   */
int	    *mat_val;			  /* current pointer to dither matrix */
int	    *mat_last;		  	  /* end of matrix 		      */
int	    tot_cnt;		          /* number of values in thresh matrix*/
int	    xsize, ysize;		  /* matrix dimensions		    */

float   scale_factor;			  /* scaling for grey levels output   */
float   unit;				  /* step for thresholding	      */
float	levels;			          /* set by threshold parameter       */
  
unsigned int  new_number_of_lines;	  /* loop condtion (output udp)	      */
unsigned int  new_pixels_per_line;	  /* loop condition (output udp)      */
unsigned int  new_data_bit_offset;	  /* dithered bit offset	      */
unsigned int  new_line_offset;		  /* dithered line offset	      */
unsigned int  orig_val, new_val;	  /* pixel values		      */
unsigned char *src_ptr, *dst_ptr;	  /* byte pointers to udp             */
unsigned long src_pad, dst_pad;		  /* pad variable		      */
int		ix,iy;                    /* loop counters                    */
unsigned long	size;			  /* size in bytes (for allocation)   */

/* 
** Set up pointers and loop conditionals for dithering to 1 bit
*/
src_ptr = (unsigned char *)(src_udp->UdpA_Base + (src_udp->UdpL_Pos>>3) +
    ((src_udp->UdpL_ScnStride>>3) * src_udp->UdpL_Y1) + src_udp->UdpL_X1);
src_pad = (src_udp->UdpL_ScnStride>>3) - src_udp->UdpL_PxlPerScn;

new_number_of_lines = dst_udp->UdpL_ScnCnt;
new_pixels_per_line = dst_udp->UdpL_PxlPerScn;
new_line_offset = dst_udp->UdpL_Pos;				 /* in bits */

new_val = 1;			              /* output if threshold is met */

if (dither_type == IpsK_DitherClustered) 
    {
    /* 
    ** Generate the threshold matrix. matrix dimensions will be 
    ** 2*xsize and 2*ysize
    */
    xsize = threshold_spec;
    ysize = threshold_spec;

    /*
    ** allocate array of xsize*ysize*4  (doubled)
    */
    tot_cnt = xsize*ysize*4;
    size = sizeof(int) * (2*tot_cnt);
    mat = (int *)(*IpsA_MemoryTable[IpsK_Alloc])(size,IpsM_InitMem,0);
    if (!mat)
        return (IpsX_INSVIRMEM);

    levels = Ips__cmatx(mat+tot_cnt,xsize,ysize);
    xsize *= 2;                       /* double the original matrix size  */
    ysize *= 2;
    }
else
    {
    ix = (threshold_spec+1) / 2;
    xsize = ysize = 1<<ix;
    tot_cnt = xsize*ysize;
    size = sizeof(int) * (2*tot_cnt);
    mat = (int *)(*IpsA_MemoryTable[IpsK_Alloc])(size,IpsM_InitMem,0);
    if (!mat)
        return (IpsX_INSVIRMEM);

    levels = Ips__dmatx (mat+tot_cnt,threshold_spec,xsize);
    }

/*
** Assign actual values to the matrix --
** scale to range (1,max_pxl)    
*/

/* step for thresholding */
unit = src_udp->UdpL_Levels/ (levels);

for (ix = 0; ix < tot_cnt; ix++) 
    *(mat+ix) = *(mat+ix+tot_cnt) * unit + 0.5;

mat_row_start = mat;
mat_last = mat + tot_cnt;

for (iy = 0; iy < new_number_of_lines; iy++)
    {
    mat_val = mat_row_start;
    mat_row_end = mat_row_start + xsize;
		
    /* 
    ** compute bit offset to beginning of this scan in new & old
    ** image planes.
    */
    new_data_bit_offset = new_line_offset;

    for (ix = 1; ix < new_pixels_per_line+1; ix++)
	{
	/* get the original data value */

	orig_val = *src_ptr++;

	if (orig_val >= *mat_val++)
	    /* > threshold, set output to all one's pixel  */
	    PUT_BIT_VALUE_(dst_udp->UdpA_Base,new_data_bit_offset,new_val);

	    /* position to next dst pixel				      */
	    new_data_bit_offset += dst_udp->UdpL_PxlStride; 

	    if (mat_val == mat_row_end) mat_val = mat_row_start;
	}/* end of row */

    /* 
    ** bump up to next line
    */
    src_ptr += src_pad;
    new_line_offset += dst_udp->UdpL_ScnStride; /* next dst line */
    mat_row_start += xsize;
    if (mat_row_start == mat_last) 
        mat_row_start = mat;

    }/* end of number of lines */

/* 
** free allocated memory
*/
(*IpsA_MemoryTable[IpsK_Dealloc])(mat);
return (IpsX_SUCCESS);

}/* end _IpsOrderedByteBitonal */

/******************************************************************************
**
**  _IpsOrderedByteByte
**
**  FUNCTIONAL DESCRIPTION:
**
**      Ordered dither from byte to bit
**
**  FORMAL PARAMETERS:
**
**	src_udp -->	pointer to source	    udp (initialized)
**	dst_udp -->	pointer to destination  udp (uninitialized)
**	dither_type	clustered or dispersed
**	threshold	m-factor(if clustered) or order(if dispersed)
**	output_levels	quantization levels
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      long status
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**      none
**
*******************************************************************************/

/******************************************************************************/
/* RESTRICTIONS:                                                              */
/*    current restrictions:						      */
/*                                                                            */
/*    byte udp's only. class is UBA					      */
/*                                                                            */
/******************************************************************************/

static long _IpsOrderedByteByte(src_udp,dst_udp,dither_type,threshold_spec,
                            output_levels)
struct UDP  *src_udp;			  /* source udp			     */
struct UDP  *dst_udp;			  /* destination udp		     */
long	    dither_type;    		  /* clustered or dispersed          */
long	    threshold_spec;    		  /* m-factor (if clustered          */
				          /* order (if dispersed)	     */
long	    output_levels;		  /* dither output level	     */

{
int	    *mat;			  /* start of dither matrix	      */
int	    *mat_row_start;		  /* start of current dither mtrx row */
int	    *mat_row_end;		  /* end of current dither mtrx row   */
int	    *mat_val;			  /* current pointer to dither matrix */
int	    *mat_last;		  	  /* end of matrix 		      */
int	    tot_cnt;		          /* number of values in thresh matrix*/
int	    xsize, ysize;		  /* matrix dimensions		    */

int	    new_number_of_lines;
int	    new_pixels_per_line;

float   *fmat;				  /* start pf f.p. dither matrix      */
float   *fmat_row_start;		  /* row start of f.p. dither matrix  */
float   *fmat_row_end;			  /* row end of f.p. dither matrix    */
float   *fmat_val;			  /* current ptr to f.p. dither matrx */
float   *fmat_last;		  	  /* end of f.p. dither matrix        */
float   scale_factor;			  /* scaling for grey levels output   */
float   unit;				  /* step for thresholding	      */
float	levels;			          /* set by threshold parameter       */
  
unsigned int  new_data_bit_offset;	
unsigned int  new_line_offset;
unsigned int  orig_val, new_val;
unsigned char *src_ptr, *dst_ptr;	/* byte pointers to udp		    */
unsigned long src_pad, dst_pad;		/* pad variable			    */
int		ix,iy;                  /* loop counters                    */
unsigned long	size;			/* size in bytes (for allocation)   */

{
/*
** Set up pointers and loop conditionals
*/
src_ptr = (unsigned char *)(src_udp->UdpA_Base + (src_udp->UdpL_Pos>>3) +
    ((src_udp->UdpL_ScnStride>>3) * src_udp->UdpL_Y1) + src_udp->UdpL_X1);
src_pad = (src_udp->UdpL_ScnStride>>3) - src_udp->UdpL_PxlPerScn;

dst_ptr = (unsigned char *)(dst_udp->UdpA_Base + (dst_udp->UdpL_Pos>>3));
dst_pad = (dst_udp->UdpL_ScnStride>>3) - dst_udp->UdpL_PxlPerScn;

new_number_of_lines = dst_udp->UdpL_ScnCnt;
new_pixels_per_line = dst_udp->UdpL_PxlPerScn;

/*
** Generate the threshold matrix based on dither type
*/
if (dither_type == IpsK_DitherClustered) 
    {
    xsize = threshold_spec;
    ysize = threshold_spec;

    /*
    ** allocate array of xsize*ysize*4  (doubled)
    */
    tot_cnt = xsize*ysize*4;
    size = sizeof(int) * (2*tot_cnt);
    mat = (int *)(*IpsA_MemoryTable[IpsK_Alloc])(size,IpsM_InitMem,0);
    if (!mat)
        return (IpsX_INSVIRMEM);

    levels = Ips__cmatx(mat+tot_cnt,xsize,ysize);
    xsize *= 2;                       /* double the original matrix size  */
    ysize *= 2;
    }
else
    {
    ix = (threshold_spec+1) / 2;
    xsize = ysize = 1<<ix;
    tot_cnt = xsize*ysize;
    size = sizeof(int) * (2*tot_cnt);
    mat = (int *)(*IpsA_MemoryTable[IpsK_Alloc])(size,IpsM_InitMem,0);
    if (!mat)
        return (IpsX_INSVIRMEM);

    levels = Ips__dmatx (mat+tot_cnt,threshold_spec,xsize);
    }

    /*  slower loop for > 1 bit output */

/*
** Allocate floating buffer of size tot_cnt for dither > 1 bit
*/
size = sizeof(float) * tot_cnt;
fmat = (float*)(*IpsA_MemoryTable[IpsK_Alloc])(size,IpsM_InitMem,0);
    if (!fmat)
	return (IpsX_INSVIRMEM);

/* 
** new output = (int) input_normalized * max_pix_out + matrix_normalized
*/

    /* calculate scale factor */
    scale_factor = output_levels - 1;
    scale_factor /= src_udp->UdpL_Levels-1;

    /* normalize values to f.p. matrix */
    for (ix = 0; ix < tot_cnt; ix++) 
	*(fmat+ix) = (float)*(mat+ix+tot_cnt) / levels;

    fmat_row_start = fmat;
    fmat_last = fmat + tot_cnt;

    for (iy = 0; iy < new_number_of_lines; iy++)
	{

	fmat_val = fmat_row_start;
	fmat_row_end = fmat_row_start + xsize;

	/* compute bit offset to beginning of this scan in new & old  */
	/* image planes.                                              */

	for (ix = 1; ix < new_pixels_per_line+1; ix++)
	    {
	    /* get the original data value                            */

	    orig_val = *src_ptr++;

	    *dst_ptr++ = ((float)orig_val*scale_factor) + *fmat_val++;

	    if (fmat_val == fmat_row_end) fmat_val = fmat_row_start;
	    }				  /* end of row		      */

	/* next line */
	src_ptr += src_pad;
	dst_ptr += dst_pad;

	fmat_row_start += xsize;
	if (fmat_row_start == fmat_last) fmat_row_start = fmat;

	}				  /* end of number of lines   */
    }					  /* end of > 1 bit loop      */
/* 
** free allocated memory
*/
(*IpsA_MemoryTable[IpsK_Dealloc])(mat);
(*IpsA_MemoryTable[IpsK_Dealloc])(fmat);

return (IpsX_SUCCESS);

}/* end _IpsOrderedByteByte */

