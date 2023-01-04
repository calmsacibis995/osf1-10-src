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
**  _IpsRotateBitonal
**
**  FACILITY:
**
**      Image Processing Services (IPS)
**
**  ABSTRACT:
**
**      This module contains routines which create an image frame by
**	rotating all or a region of interest from a user supplied 
**	source image frame.
**
**	The routines in this module operate only on bitonal images.
**
**  ENVIRONMENT:
**
**      VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHOR(S):
**
**      Bob Shelley
**	Revised for V3.0 by Karen Rodwell 
**
**  CREATION DATE:
**
**	October 3, 1989
**
*****************************************************************************/

/*
**  Table of contents
*/
#ifdef NODAS_PROTO
long _IpsRotateBitonal();  /* ROTATE IMAGE			    */
#endif

/*
**  Include files 
*/

#include <IpsDef.h>                         /* Image definitions            */
#include <IpsStatusCodes.h>                 /* status codes                 */
#include <IpsMemoryTable.h>		    /* IPS Memory Mgt. Functions    */
#ifndef NODAS_PROTO
#include <ipsprot.h>				    /* Ips prototypes */
#endif
#include <math.h>                           /* math library                 */

/*
**  Equated Symbols
*/
#define	DEG_RAD	-0.0174532925199432959	/* Degrees to rads inverted Y axis  */

/*
**  External References from IPS		   <- from module ->
*/
#ifdef NODAS_PROTO
long		*_IpsBuildChangelist();	        /* IPS__CHANGE_LISTS	*/
void		 _IpsMovv7();			/* IPS__EXTEND_INSTRUCT	*/
long		_IpsGetStartAddress();		/* from Ips__udp_utils  */
long		_IpsLogicalBitonal();      /* from IPS__LOGICAL_BITONAL */
#endif
/* 
** Internal routines
*/
#ifdef NODAS_PROTO
static long *rotate_0();   /* write rotated scanlines		    */
static long *rotate_1();   /* write rotated scanlines & fill holes  */
#else
PROTO(static long *rotate_0, (unsigned char */*base*/, long /*line*/, long */*ref*/, long */*lst*/));
PROTO(static long *rotate_1, (unsigned char */*base*/, long /*line*/, long */*ref*/, long */*lst*/, long /*hole*/, long /*fill*/));
#endif

/*************************************************************************
**  _IpsRotateBitonal
**
**  FUNCTIONAL DESCRIPTION:
**
**      Return an image frame to the user which has been rotated from
**	a supplied image frame.
**
**  FORMAL PARAMETERS:
**
**	src_udp	    - udp of source image
**	dst_udp	    - udp of destination image
**	angle	    - angle of rotation (degrees)
**	flags 	    - flag for reverse edge fill
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
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**	none
**
*****************************************************************************/
long _IpsRotateBitonal( src_udp, dst_udp, angle,flags)
struct UDP *src_udp;                    /* Working src UDP descriptor */
struct UDP *dst_udp;                    /* Working dst UDP descriptor */
float      *angle;			/* Angle of rotation	      */
long	   flags;			/* Flag for ReverseEdgeFill   */
{
unsigned char *d_base;	        /* dst buffer address			    */
double sina, cosa;		/* sin(angle), cos(angle)		    */
long *clst, csiz, ccnt = 0;	/* CHANGELIST, size, and count field index  */
long *rmap, rsiz;		/* pre-rotated reference line and size	    */
long dx, dy = 1;		/* delta X & Y (distance to next pixel)	    */
long shft_h, shft_v, shft_hv;   /* scanline shift: horizontal/vertical/both */
long hole, fill;		/* hole detect, fill offset		    */
long s_pos, d_pos = 0;	        /* src and dst start of scanline position   */
long s_xcnt, s_ycnt;  	        /* src pixel counts in X and Y directions   */
long status = IpsX_SUCCESS;
long tmp1, tmp2, *ptr;	        /* temps				    */
long edge = flags & IpsM_ReverseEdgeFill ? 1 : ccnt;
unsigned long src_start_addr;	/* used to figure actual src start addr	    */

/*
** validate source class and type
*/
switch (src_udp->UdpB_Class)
    {
    case UdpK_ClassUBA:
        switch (src_udp->UdpB_DType)
            {
            case UdpK_DTypeVU: break;
            case UdpK_DTypeBU:
            case UdpK_DTypeWU:
            case UdpK_DTypeLU:
            case UdpK_DTypeF:
            case UdpK_DTypeV:
            default: return(IpsX_UNSOPTION); break;
            }; break;
    case UdpK_ClassA:
    case UdpK_ClassUBS:
    case UdpK_ClassCL:
    default: return(IpsX_UNSOPTION); break;
    };

/*
** Initialize destination UDP 
*/
sina = sin( *angle * DEG_RAD );
cosa = cos( *angle * DEG_RAD );
/*
**	Calculate the source dimensions:
**	    upper limits of X (ie. 0...tmp1),
**	    upper limits of Y (ie. 0...tmp2),
**	    pixels per scanline and scanline count.
*/
tmp1 = src_udp->UdpL_X2 - src_udp->UdpL_X1;
tmp2 = src_udp->UdpL_Y2 - src_udp->UdpL_Y1;
s_xcnt = tmp1 + 1;
s_ycnt = tmp2 + 1;

/*
**  Compute the working values for destination parameters X1, 
**  X2, Y1, Y2.
**  (based on src parameters X1,Y1 translated to 0,0), and the values for
**  delta-X and horizontal scanline shifts ("ISL normal" inverted Y axis).
*/
if( sina >= 0 )
    if( cosa >= 0 )
	{						/* Quadrant I 	    */
	dx =  1;					/* X increasing	    */
	shft_h = 1;					/* lines shift right*/
	dst_udp->UdpL_X1 = 0;				/* use upper left   */
	dst_udp->UdpL_X2 = tmp1 * cosa + tmp2 * sina;   /* use lower right  */
	dst_udp->UdpL_Y1 = -( tmp1 * sina );		/* use upper right  */
	dst_udp->UdpL_Y2 = tmp2 * cosa;			/* use lower left   */
	}
    else
	{						/* Quadrant II	    */
	dx = -1;					/* X decreasing	    */
	shft_h = 1;					/* lines shift right*/
	dst_udp->UdpL_X1 = tmp1 * cosa;			/* use upper right  */
	dst_udp->UdpL_X2 = tmp2 * sina;			/* use lower left   */
	dst_udp->UdpL_Y1 = tmp2 * cosa - tmp1 * sina;   /* use lower right  */
	dst_udp->UdpL_Y2 = 0;				/* use upper left   */
	}
else if( cosa < 0 )
	{						/* Quadrant III	    */
	dx = -1;					/* X decreasing	    */
	shft_h = -1;					/* lines shift left */
	dst_udp->UdpL_X1 = tmp1 * cosa + tmp2 * sina;   /* use lower right  */
	dst_udp->UdpL_X2 = 0;				/* use upper left   */
	dst_udp->UdpL_Y1 = tmp2 * cosa;			/* use lower left   */
	dst_udp->UdpL_Y2 = -( tmp1 * sina );		/* use upper right  */
	}
    else
	{						/* Quadrant IV	    */
	dx = 1;						/* X increasing	    */
	shft_h = -1;					/* lines shift left */
	dst_udp->UdpL_X1 = tmp2 * sina;			/* use lower left   */
	dst_udp->UdpL_X2 = tmp1 * cosa;			/* use upper right  */
	dst_udp->UdpL_Y1 = 0;				/* use upper left   */
	dst_udp->UdpL_Y2 = tmp2 * cosa - tmp1 * sina;   /* use lower right  */
	}

/*
**	Compute the working values for destination parameters Scan stride
**	and Pos.
*/
dst_udp->UdpW_PixelLength = src_udp->UdpW_PixelLength;
dst_udp->UdpL_PxlPerScn = dst_udp->UdpL_X2 - dst_udp->UdpL_X1 + 1;
dst_udp->UdpL_ScnCnt = dst_udp->UdpL_Y2 - dst_udp->UdpL_Y1 + 1;
dst_udp->UdpL_ScnStride = dst_udp->UdpL_PxlPerScn;
dst_udp->UdpL_ScnStride += ((8 - (dst_udp->UdpL_ScnStride % 8))) % 8;
dst_udp->UdpL_CompIdx = src_udp->UdpL_CompIdx;
dst_udp->UdpL_Levels = src_udp->UdpL_Levels;
dst_udp->UdpB_Class = src_udp->UdpB_Class;
dst_udp->UdpB_DType = src_udp->UdpB_DType;
dst_udp->UdpL_PxlStride = src_udp->UdpL_PxlStride;

if (dst_udp->UdpA_Base != 0)
    {
    /* 
    ** No in place allowed - verify 
    */
/*
**	Since x1, x2 y1 and y2 are used as the old model we can't call this
**	x1, x2 etc. can be negative (a no-no for IPS)
**    status = _IpsVerifyNotInPlace (src_udp, dst_udp);
*/
/*
**  Get nearest byte boundary of src
*/
    status = _IpsGetStartAddress (src_udp, &src_start_addr);
    if (status != IpsX_SUCCESS) return (status);
    if (src_start_addr == 
	(unsigned long) dst_udp->UdpA_Base + (dst_udp->UdpL_Pos>>3))
	    return (IpsX_NOINPLACE);
    if ((dst_udp->UdpL_ArSize - dst_udp->UdpL_Pos) < (dst_udp->UdpL_ScnStride * 
	dst_udp->UdpL_ScnCnt))
	    return (IpsX_INSVIRMEM);
    }
else
    {
    dst_udp->UdpA_Base = (unsigned char *)
	(*IpsA_MemoryTable[IpsK_AllocateDataPlane])
	    (((dst_udp->UdpL_ScnStride * dst_udp->UdpL_ScnCnt+7)/8),
		IpsM_InitMem,0);
    if (!dst_udp->UdpA_Base) 
	return (IpsX_INSVIRMEM);
            dst_udp->UdpL_ArSize = dst_udp->UdpL_ScnStride * 
		dst_udp->UdpL_ScnCnt;
    dst_udp->UdpL_Pos = 0;
    }

/*
**	Allocate working storage required:
**	    - space for the CHANGELIST array,
**	    - space for the pre-rotated reference line map.
*/
rsiz = s_xcnt > s_ycnt			    /* reference line map size	    */
	 ? s_xcnt + 1 : s_ycnt + 1;	    /* ...use largest src dimension */
csiz = s_xcnt + 2;			    /* CHANGELIST size		    */
clst = (long *) (*IpsA_MemoryTable[IpsK_Alloc])
    ((sizeof(long)*(rsiz + csiz + 1)),IpsM_InitMem,0);

d_base = (unsigned char *)dst_udp->UdpA_Base;

/*
**  Compute the horizontal/vertical reference line which maps source 
**	pixel positions to pre-rotated destination positions.
*/
rmap = clst + csiz;
for( ptr = rmap, tmp1 = 0; tmp1 < rsiz; ++ptr, ++tmp1 )
    *ptr = (int)(tmp1*cosa) - (int)(tmp1*sina)*dst_udp->UdpL_ScnStride +
	(dst_udp->UdpL_Pos - ( dst_udp->UdpL_PxlStride * dst_udp->UdpL_X1
				+ dst_udp->UdpL_ScnStride * dst_udp->UdpL_Y1));

/*
**  Setup values for:
**	- vertical scanline shifts,
**	- combined horizontal/vertical shifts,
**	- hole (detect intersections of combined horizontal/vertical shifts),
**	- fill (offset from previous pixel to hole to fill).
*/
shft_v  = dx * dst_udp->UdpL_ScnStride; /* vertical change		    */
shft_hv = shft_v + shft_h;		    /* horizontal + vertical change */
hole = dx - shft_h * 
	            dst_udp->UdpL_ScnStride;/* hole to fill when both change*/
fill = dx + shft_h == 0 ? dx : hole-dx; /* pixel offset to fill hole    */
/*
**	MAIN LOOP: stay here until rotation is complete.
*/
for( ptr = rmap, 
    s_pos = src_udp->UdpL_Pos + src_udp->UdpL_ScnStride * src_udp->UdpL_Y1 +
	    src_udp->UdpL_X1;  s_ycnt--;
		dy = *( ptr + 1 ) - *ptr,	/* delta-Y to next scanline */
		++ptr,				/* next ref line point	    */
		s_pos += src_udp->UdpL_ScnStride,/* next src position	    */
		d_pos +=  dy == 0    ? 0	/* no change		    */
			: dy == dx   ? shft_v	/* vertical shift	    */
			: dy == hole ? shft_hv	/* both shift		    */
				     : shft_h )	/* horizontal shift	    */
    {
    _IpsBuildChangelist(src_udp->UdpA_Base, s_pos, s_xcnt, 
				&clst[edge], csiz);
    if( edge == 1 )
	{
	/*
	**	Modify the CHANGELIST so that we set dst bits where zeros are.
	*/
	ccnt = clst[edge + 1] == 0 ? 2 : 0;
	clst[ccnt] = clst[edge] - ccnt + 1;
	clst[edge] = 0;
	}		
    if( dy == hole )
	/*
	**  Delta-Y for this scanline indicates a combined horizontal and
	**  vertical shift, so we must prevent "holes" at the intersections
	**  of combined horizontal/vertical shifts along this scanline.
	*/
	rotate_1( d_base, d_pos, rmap, &clst[ccnt], hole, fill);
    else if( dy != 0 || edge != 1 )
	    rotate_0( d_base, d_pos, rmap, &clst[ccnt] );
    }
/*
**	Translate destination parameters (L1,L2) to (0,0).
*/
dst_udp->UdpL_X2 = dst_udp->UdpL_X2 - dst_udp->UdpL_X1;
dst_udp->UdpL_X1 = 0;
dst_udp->UdpL_Y2 = dst_udp->UdpL_Y2 - dst_udp->UdpL_Y1;
dst_udp->UdpL_Y1 = 0;
dst_udp->UdpL_ScnCnt = dst_udp->UdpL_Y2 + 1;
dst_udp->UdpL_PxlPerScn = dst_udp->UdpL_X2 + 1;

if( edge == 1 )
    /*
    **  Invert the entire image (just to accomplish reverse edge fill).
    */
    status = _IpsLogicalBitonal (dst_udp, dst_udp, dst_udp, 0, 
				 IpsK_SetToNotSrc2);

/*
**	Deallocate working storage and return.
*/
(*IpsA_MemoryTable[IpsK_Dealloc])( clst ); 
return (status);
} /* end of _IpsRotateBitonal */

/**********************************************************************
**  rotate_0
**
**  FUNCTIONAL DESCRIPTION:
**
**	Convert CHANGELIST into rotated scanline.
**
**  FORMAL PARAMETERS:
**
**	base	- base address for image data
**	line	- scanline offset from reference line
**	ref	- reference line rotation position map
**	lst	- address of CHANGELIST
**
**  FUNCTION VALUE:
**
**	lst	- address of CHANGELIST passed in
**
*****************************************************************************/
static long *rotate_0( base, line, ref, lst )
unsigned char   *base;		/* base address of destination image	    */
long		 line;		/* offset from reference line		    */
long		*ref;		/* reference line destination map	    */
long		*lst;		/* pointer to CHANGELIST		    */

{
    int	 delta,			    /* distance to next position	    */
	 d_pos = 1 << 30,	    /* dst position (init = impossible pos) */
    	 i,			    /* loop count			    */
	 length,		    /* src run length			    */
         startlen,endlen;           /* variables to calculate run length */
    long *run = lst,		    /* dyanmic lst pointer		    */
	*x_pos;			    /* src scanline position map pointer    */

    /*
    **  Write a mapped string of ones for each run position pair.
    */
    for( i = *run++ / 2; i > 0; --i )	/* for pairs of CHANGELIST points...*/
        {
        x_pos    = *run + ref;             /* map position of start of run */
        delta    = *x_pos++ - d_pos;        /* distance to start position */
        startlen = *run++;
        endlen   = *run++;
	for(length = -( startlen - endlen);     /* run length		    */
		   --length >= 0;		/* for length of run...	    */
			delta = *x_pos++ - d_pos)   /* distance to next pos */

	    /*
	    **	If there's no change in X or Y we've already written this bit.
	    */
	    if( delta != 0 )
		{
		d_pos += delta;		    /* position of next pixel	    */
		*( base + ( d_pos + line >> 3 )) |= 1 << ( d_pos + line & 7 );
		}
        }
    return (long *) ( lst );			    /* return list pointer	    */
} /* end of rotate */

/**************************************************************************
**  rotate_1
**
**  FUNCTIONAL DESCRIPTION:
**
**	Convert CHANGELIST into rotated scanline while filling holes.
**
**  FORMAL PARAMETERS:
**
**	base	- base address for image data
**	line	- scanline offset from reference line
**	ref	- reference line rotation position map
**	lst	- address of CHANGELIST
**	hole	- reference line delta when both X and Y change
**	fill	- offset when hole to fill
**
**  FUNCTION VALUE:
**
**	lst	- address of CHANGELIST passed in
**
*****************************************************************************/
static long *rotate_1( base, line, ref, lst, hole, fill )
unsigned char   *base;		/* base address of destination image	    */
long		 line;		/* offset from reference line		    */
long		*ref;		/* reference line destination map	    */
long		*lst;		/* pointer to CHANGELIST		    */
long		 hole;		/* value of simultaneous XY change 	    */
long		 fill;		/* offset to hole to fill 		    */

{
    int	 delta,			    /* distance to next position	    */
	 d_pos = 1 << 30,	    /* dst position (init = impossible pos) */
	 i,			    /* loop count			    */
	 length,		    /* src run length			    */
         startlen,endlen;           /* variables to calculate run length */
    long *run = lst,		    /* dyanmic lst pointer		    */
	*x_pos;			    /* src scanline position map pointer    */

    /*
    **  Write a mapped string of ones for each run position pair.
    */
    for( i = *run++ / 2; i > 0; --i )	/* for pairs of CHANGELIST points...*/
        {
        x_pos    = *run + ref;             /* map position of start of run */
        delta    = *x_pos++ - d_pos;        /* distance to start position */
        startlen = *run++;
        endlen   = *run++;
	for(length = -( startlen - endlen);     /* run length		    */
		   length > 0; --length )	/* for length of run...	    */
	    {

	    /*
	    **	If there's no change in X or Y we've already written this bit.
	    */
	    if( delta != 0 )
		{
		d_pos += delta;
		*( base + ( d_pos + line >> 3 )) |= 1 << ( d_pos + line & 7 );
		}
	    delta = *x_pos++ - d_pos;	    /* distance to next position    */

	    /*
	    **	We have a hole to fill when ever there is a simultaneous
	    **	change in X and Y pending.
	    */
	    if( delta == hole )
		*(base + (d_pos+line+fill >> 3)) |= 1 <<(d_pos+line+fill & 7);
	    }
        }
    return (long *) ( lst );			    /* return list pointer	    */
} /* end of rotate_fill */

