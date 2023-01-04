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
**      Blue Noise dithering routine adapted from work done by Bob Ulichney,
**      DIGITAL.
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
**  CREATION DATE:     February, 1988
**
************************************************************************/
/*
**  Table of contents:
*/
#ifdef NODAS_PROTO
long	    _IpsDitherBluenoise();         /* Layer 2  dither routine	    */
#endif

/*
**  Include files:
*/

#include <IpsDef.h>			    /* IPS Definitions              */
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

/* inline random number generator ... generates floating number between      */
/* min and max arguments specified                                           */

#define RANDOM_(min,max) \
    (seed.integer = 69069 * seed.integer + 1, \
    (float)(seed.fields.high_bits) * (float)5.9604645E-08 * ((max) - (min)) + (min))

/*
** seed integer structure for the RANDOM_ macro
*/
union RANDOM_SEED
    {
    unsigned int  integer;
    struct
         {
         unsigned : 8;
         unsigned high_bits : 24;
         } fields;
    } ;

/* constants from Bob Ulichney's Blue Noise routines                         */

#define W1   .4375                       /* 7./16.                           */
#define W2   .1875                       /* 3./16.                           */
#define W3   .3125                       /* 5./16.                           */
#define W4   .0625                       /* 1./16.                           */

#define MAX_NOISE_B 0.15625              /* max value of noise_b == 5/32     */
#define MAX_NOISE_S 0.03125              /* max value of noise_s == 1/32     */

/*
**  External References:
*/
#ifdef NODAS_PROTO
long _IpsBuildDstUdp();			    /*	from Ips__udp_utils.c       */
void _IpsMovc5Long();			    /*	from UIPS__MOVC5.C          */
long _IpsVerifyNotInPlace();		    /*	from Ips__udp_utils.c	    */
#endif

/*
** Local references
*/
#ifdef NODAS_PROTO
static long _IpsBluenoiseByteBitonal();    /* Layer 2a dither bits          */
static long _IpsBluenoiseByteByte();       /* Layer 2a dither bytes         */
#else
static long _IpsBluenoiseByteBitonal(struct UDP */*src_udp*/, struct UDP */*dst_udp*/);
static long _IpsBluenoiseByteByte(struct UDP */*src_udp*/, struct UDP */*dst_udp*/, unsigned long /*output_levels*/);
#endif


/************************************************************************
**
**  _IpsDitherBluenoise()
**
**  FUNCTIONAL DESCRIPTION:
**
**      Performs "blue noise" dithering algorithm. 
**
**  FORMAL PARAMETERS:
**
**	src_udp		- source udp
**	dst_udp		- destination udp
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
**      long status
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

/***************************************************************************/
/* Restrictions:                                                           */
/*    current restrictions:                                                */
/*                                                                         */
/*    byte image input only						   */
/***************************************************************************/

long _IpsDitherBluenoise(src_udp, dst_udp, output_levels, flags)
struct UDP	*src_udp;		  /* source udp			    */
struct UDP	*dst_udp;		  /* destination udp		    */
unsigned long	output_levels;		  /* quantization output level	    */
unsigned long	flags;			  /* dither to bitonal flag	    */
{
unsigned long byte_boundary_padbits;	  /* pad of bitonal udp to byte     */
unsigned long size;			  /* size in bytes for allocation   */
long	      status;
unsigned long mem_alloc = 0;

/*
** Validate source and type
*/
switch (src_udp->UdpB_Class)
    {
    case UdpK_ClassUBA:				
	/*
	** Bitonal source UDP
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
else
    {	/* don't dither to bitonal */
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
	    status = _IpsBluenoiseByteBitonal(src_udp,dst_udp);
	else
	    status = _IpsBluenoiseByteByte(src_udp,dst_udp,output_levels);
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
}/* end _IpsDitherBluenoise */

/******************************************************************************
**
**  _IpsBluenoiseByteBitonal
**
**  FUNCTIONAL DESCRIPTION:
**
**      Bluenoise dither from byte to bit
**
**  FORMAL PARAMETERS:
**
**	src_udp --> pointer to source	    udp (initialized)
**	dst_udp --> pointer to destination  udp (initialized)
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
/*    bitonal udp's only. class is UBA                                        */
/*                                                                            */
/******************************************************************************/

static long _IpsBluenoiseByteBitonal(src_udp, dst_udp)
struct UDP	*src_udp;		  /* source udp			    */
struct UDP	*dst_udp;		  /* destination udp		    */
{
unsigned char *src_ptr, *dst_ptr;	/* udp byte pointers for loop	    */
unsigned long src_pad;			/* pad variable			    */
unsigned long src_bytes_per_line;
union RANDOM_SEED seed;

int	ix,iy;			        /* loop counters		    */
int 	out_val, temp;			/* output value locations	    */
int	offset, new_offset;		/* inner loop offsets		    */
int     new_number_of_lines;		/* output frame's no. of lines	    */
int     new_pixels_per_line;		/* output frame's pix/line	    */
int	scan_width, new_scan_width;	/* offset last pixel on line	    */
int	data_bit_offset;		/* outer loop offsets		    */
int	new_data_bit_offset;		/*  "     "      "		    */

float   new_val;			/* input value, +error	            */
float   scale_factor,bias;		/* interval determination           */
float   *err_line,*e_curr;		/* ptrs. to error accum. lines      */
float	*e_next,*e_temp;		/*  "	 "    "	    "	   "        */
float   error;			        /* error accum. term                */
float   noise;	                        /* THE famous noise		    */

unsigned long size;		        /* size in bytes for allocation	    */  

seed.integer = 0; 

src_bytes_per_line = src_udp->UdpL_ScnStride>>3;

new_number_of_lines = dst_udp->UdpL_ScnCnt;
new_pixels_per_line = dst_udp->UdpL_PxlPerScn;

/* 
** Alloc lines to store error accumulations plus an extra pixel on each
** end of the line sp we don't have to check for overflow.
*/
size = sizeof(float) * (2 * (new_pixels_per_line + 2));
err_line = (float *) (*IpsA_MemoryTable[IpsK_Alloc])(size,IpsM_InitMem,0);
if (!err_line)
    return (IpsX_INSVIRMEM);
e_curr   = err_line + 1;
e_next   = e_curr + (new_pixels_per_line + 2);

/* scan width is used to point to last pixel in the line */
new_scan_width = (dst_udp->UdpL_PxlPerScn - 1) * dst_udp->UdpL_PxlStride;
new_data_bit_offset = dst_udp->UdpL_Pos;

/*
** set up pointers to source and destination
*/
src_ptr = (unsigned char *)(src_udp->UdpA_Base + (src_udp->UdpL_Pos>>3) +
    ((src_udp->UdpL_ScnStride>>3) * src_udp->UdpL_Y1) + src_udp->UdpL_X1);
src_pad = (src_udp->UdpL_ScnStride>>3) - src_udp->UdpL_PxlPerScn;

/*
** set up scale factor (input levels -> output levels) and bias
*/
scale_factor = (float)src_udp->UdpL_Levels-1; 
bias = scale_factor / 2.0;

for (iy = 0; iy < new_number_of_lines; iy++)
        {
        /* 
	** process lines alternately right->left, then left->right
	*/
        if ((iy & 0x1) == 0)                  /* process l-> r        */
	    {
	    /* compute bit offset to beginning of this scan in new 
	    /* & old image planes.                                    */
    
	    new_offset = new_data_bit_offset;
     
	    for (ix = 0; ix < new_pixels_per_line; ix++)
	        {
	        /* get the original data value                        */

	        /* add previous error to this pixel for new value */
	        new_val = *src_ptr++ + (*(e_curr + ix));

	        /* determine grey scale interval it falls in and 
	        /* calculate its error */

	        out_val = (new_val+bias) / scale_factor;
	        temp = ((float)out_val * scale_factor) + 0.5;
	        error = (float)new_val - (float)temp;

	        PUT_BIT_VALUE_(dst_udp->UdpA_Base,new_offset,out_val);

                /* distribute err out to the rest of the neighborhood */
                noise = RANDOM_(-MAX_NOISE_B,MAX_NOISE_B);
                *(e_curr + ix + 1) += error * (W1 + noise);
                *(e_next + ix)     += error * (W3 - noise);
                noise = RANDOM_(-MAX_NOISE_S,MAX_NOISE_S);
                *(e_next + ix - 1) += error * (W2 + noise);
                *(e_next + ix + 1) += error * (W4 - noise);
    	        /*position next pixel*/
                new_offset += dst_udp->UdpL_PxlStride; 
	        }					  /* end for l -> r */
	    src_ptr += src_pad;
	    }						  /* end if  l -> r */
        else		                                  /* process r -> l */
            {
	    /* compute bit offset to END of this scan in new & old    */
            /* image planes.                                          */
	    src_ptr += src_udp->UdpL_PxlPerScn-1;
	    new_offset = new_data_bit_offset + new_scan_width;
	    for (ix = new_pixels_per_line - 1; ix >= 0; ix--)
	        {

	        /* add previous error to this pixel for new value */
	        new_val =  *src_ptr-- + (*(e_curr + ix));
	        /* determine grey scale interval it falls in and 
	        /* calculate its error */

	        out_val = (new_val+bias) / scale_factor;
	        temp = ((float)out_val * scale_factor) + 0.5;
	        error = (float)new_val - (float)temp;
	        PUT_BIT_VALUE_(dst_udp->UdpA_Base,new_offset,out_val);

	        /* distribute err out to the rest of the neighborhood */
                noise = RANDOM_(-MAX_NOISE_B,MAX_NOISE_B);
                *(e_curr + ix - 1) += error * (W1 + noise);
                *(e_next + ix)     += error * (W3 - noise);
                noise = RANDOM_(-MAX_NOISE_S,MAX_NOISE_S);
                *(e_next + ix + 1) += error * (W2 + noise);
                *(e_next + ix - 1) += error * (W4 - noise);

	        /* position next pixel  */
                new_offset -= dst_udp->UdpL_PxlStride; 
                }
	    src_ptr += src_bytes_per_line+1;
            }       			/* end of if-else  */

	    /* update next line	*/
	    new_data_bit_offset += dst_udp->UdpL_ScnStride;

	    /* swap the error buffer lines current <---> next, */
	    /* zero the new "next"*/
	    e_temp = e_curr;
	    e_curr = e_next;
	    e_next = e_temp;
	    _IpsMovc5Long(0,0,0,sizeof(float)*(new_pixels_per_line + 2),
                         (unsigned char *)(e_next - 1));

        }           /* end of for iy == 0; etc. loop */

    _IpsMovc5Long(0,0,0,sizeof(float)*(new_pixels_per_line + 2),
             (unsigned char *)(e_curr- 1));

/*
** Deallocate buffer
*/
(*IpsA_MemoryTable[IpsK_Dealloc])(err_line);
return (IpsX_SUCCESS);
}/* end _IpsBluenoiseByteBitonal */
/******************************************************************************
**
**  _IpsBluenoiseByteByte
**
**  FUNCTIONAL DESCRIPTION:
**
**      Bluenoise dither from byte to bit
**
**  FORMAL PARAMETERS:
**
**	src_udp -->	pointer to source	udp
**	dst_udp -->	pointer to destination  udp
**	output_levels	quantization output levels
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
/*    byte udp's only class is UBA                                            */
/*                                                                            */
/******************************************************************************/


static long _IpsBluenoiseByteByte(src_udp, dst_udp, output_levels)
struct UDP	*src_udp;		  /* source udp			    */
struct UDP	*dst_udp;		  /* destination udp		    */
unsigned long	output_levels;		  /* quantization output level	    */
{
unsigned char *src_ptr, *dst_ptr;	/* udp byte pointers for loop	    */
unsigned long src_pad;			/* source pad variable		    */
unsigned long src_bytes_per_line;
unsigned long dst_bytes_per_line;
union RANDOM_SEED seed;

int	ix,iy;			        /* loop counters		    */
int 	out_val, temp;			/* output value locations	    */
int     new_number_of_lines;		/* output frame's no. of lines	    */
int     new_pixels_per_line;		/* output frame's pix/line	    */
int	scan_width, new_scan_width;	/* offset last pixel on line	    */

float   new_val;			/* input value, +error	            */
float   scale_factor,bias;		/* interval determination           */
float   *err_line,*e_curr;		/* ptrs. to error accum. lines      */
float	*e_next,*e_temp;		/*  "	 "    "	    "	   "        */
float   error;			        /* error accum. term                */
float   noise;	                        /* THE famous noise		    */

unsigned long size;		        /* size in bytes for allocation	    */  

seed.integer = 0; 

new_number_of_lines = dst_udp->UdpL_ScnCnt;
new_pixels_per_line = dst_udp->UdpL_PxlPerScn;

src_bytes_per_line = src_udp->UdpL_ScnStride>>3;
dst_bytes_per_line = dst_udp->UdpL_ScnStride>>3;

/* 
** Alloc lines to store error accumulations plus an extra pixel on each
** end of the line sp we don't have to check for overflow.
*/
size = sizeof(float) * (2 * (new_pixels_per_line + 2));
err_line = (float *) (*IpsA_MemoryTable[IpsK_Alloc])(size,IpsM_InitMem,0);
if (!err_line)
    return (IpsX_INSVIRMEM);

e_curr   = err_line + 1;
e_next   = e_curr + (new_pixels_per_line + 2);

/* compute bit distance from start of line to last pixel on the line      */
new_scan_width = (dst_udp->UdpL_PxlPerScn - 1) * dst_udp->UdpL_PxlStride;

/*
** set up pointers to source and destination
*/
src_ptr = (unsigned char *)(src_udp->UdpA_Base + (src_udp->UdpL_Pos>>3) +
    ((src_udp->UdpL_ScnStride>>3) * src_udp->UdpL_Y1) + src_udp->UdpL_X1);
src_pad = (src_udp->UdpL_ScnStride>>3) - src_udp->UdpL_PxlPerScn;

dst_ptr = (unsigned char *)(dst_udp->UdpA_Base + (dst_udp->UdpL_Pos>>3));

/*
** set up scale factor (input levels -> output levels) and bias
*/
scale_factor = output_levels - 1; 
scale_factor = (float)(src_udp->UdpL_Levels-1) / scale_factor; 
bias = scale_factor / 2.0;

for (iy = 0; iy < new_number_of_lines; iy++)
        {
        /* 
	** process lines alternately right->left, then left->right
	*/
        if ((iy & 0x1) == 0)                  /* process l-> r        */
	    {
     
	    for (ix = 0; ix < new_pixels_per_line; ix++)
	        {

	        /* add previous error to this pixel for new value */
	        new_val = *src_ptr++ + (*(e_curr + ix));

	        /* determine grey scale interval it falls in and 
	        /* calculate its error */

	        out_val = (new_val+bias) / scale_factor;
	        temp = ((float)out_val * scale_factor) + 0.5;
	        error = (float)new_val - (float)temp;

		*dst_ptr++ = out_val;

                /* distribute err out to the rest of the neighborhood */
                noise = RANDOM_(-MAX_NOISE_B,MAX_NOISE_B);
                *(e_curr + ix + 1) += error * (W1 + noise);
                *(e_next + ix)     += error * (W3 - noise);
                noise = RANDOM_(-MAX_NOISE_S,MAX_NOISE_S);
                *(e_next + ix - 1) += error * (W2 + noise);
                *(e_next + ix + 1) += error * (W4 - noise);
	        }					  /* end for l -> r */
	    src_ptr += src_pad;
	    }						  /* end if  l -> r */
        else		                                  /* process r -> l */
            {
	    /* compute bit offset to END of this scan in new & old    */
            /* image planes.                                          */
	    src_ptr += src_udp->UdpL_PxlPerScn-1;
	    dst_ptr += dst_udp->UdpL_PxlPerScn-1;
	    for (ix = new_pixels_per_line - 1; ix >= 0; ix--)
	        {

	        /* add previous error to this pixel for new value */
	        new_val = *src_ptr-- + (*(e_curr + ix));

	        /* determine grey scale interval it falls in and 
	        /* calculate its error */

	        out_val = (new_val+bias) / scale_factor;
	        temp = ((float)out_val * scale_factor) + 0.5;
	        error = (float)new_val - (float)temp;
		*dst_ptr-- = out_val;

	        /* distribute err out to the rest of the neighborhood */
                noise = RANDOM_(-MAX_NOISE_B,MAX_NOISE_B);
                *(e_curr + ix - 1) += error * (W1 + noise);
                *(e_next + ix)     += error * (W3 - noise);
                noise = RANDOM_(-MAX_NOISE_S,MAX_NOISE_S);
                *(e_next + ix + 1) += error * (W2 + noise);
                *(e_next + ix - 1) += error * (W4 - noise);
                }
	    src_ptr += src_bytes_per_line+1;
	    dst_ptr += dst_bytes_per_line+1;
            }       			/* end of if-else  */

	    /* swap the error buffer lines current <---> next, */
	    /* zero the new "next"*/
	    e_temp = e_curr;
	    e_curr = e_next;
	    e_next = e_temp;
	    _IpsMovc5Long(0,0,0,sizeof(float)*(new_pixels_per_line + 2),
                         (unsigned char *)(e_next - 1));

        }           /* end of for iy == 0; etc. loop */

    _IpsMovc5Long(0,0,0,sizeof(float)*(new_pixels_per_line + 2),
             (unsigned char *)(e_curr- 1));
/*
** Deallocate buffer
*/
(*IpsA_MemoryTable[IpsK_Dealloc])(err_line);
return (IpsX_SUCCESS);
}/* end _IpsBluenoiseByteByte */
