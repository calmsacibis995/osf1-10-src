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
**
**  _IpsEncode
**
**  FACILITY:
**
**      Image Processing Services (IPS)
**
**  ABSTRACT:
**
**      Encode bitmapped frame into CCITT Group 3/1D encoded image buffer.
**
**  ENVIRONMENT:
**
**      VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHOR(S):
**
**      Ken MacDonald, Digital Equipment Corp.
**	Revised for V3.0 by Karen Rodwell, Digital Equipment Corp.
**
**  CREATION DATE:
**
**      October 16, 1989
**
************************************************************************/

/*
**  Table of contents:
*/
#ifdef NODAS_PROTO
long _IpsEncodeG31d();      /* encode image frame to G31D buffer routine */
long _IpsEncodeG32d();      /* encode image frame to G32D buffer routine */
long _IpsEncodeG42d();      /* encode image frame to G42D buffer routine */
#endif

/*
**  Include files:
*/
#include        <IpsDef.h>              /* image frame constants              */
#include 	<IpsStatusCodes.h>      /* IPS Status Codes                   */
#include        <IpsMemoryTable.h>      /* IPS Memory Mgt. Functions          */
#include	<IpsMacros.h>		/* IPS Macros			      */
#ifndef NODAS_PROTO
#include 	<ipsprot.h>		/* Ips prototypes 		      */
#endif
#include        <ips_fax_macros.h>      /* FAX macros like $GET_12_BITS       */
#include        <ips_fax_definitions.h> /* FAX definitions                    */
#include        <ips_fax_paramdef.h>    /* FAX routine parameter block defns. */
/*
**  Equated Symbols:
*/
#define BLACK_TO_WHITE     0
#define WHITE_TO_BLACK     1
#define WHITE 0
#define BLACK 1
#define MAXCODEWORDSIZE 14
#define BLACK_TABLE_SIZE 8192
#define WHITE_TABLE_SIZE 4096
#define DECODE_TABLE_SIZE 4096
#define ENCODE_TABLE_SIZE 2561
    
/* Image Services Library routines */
#ifdef NODAS_PROTO
void _IpsEncodeG31dScan();              /* encodes a scan in g31d format      */
void _IpsEncodeG42dScan();              /* encodes a scan in g42d format      */
long _IpsBuildChangelist();              /* builds changelist for 1 scanline   */
#endif

#if defined(__VAXC) || defined(VAXC)
globalref struct
   {
   short int length;                    /* codeword length                    */
   short int codeword;                  /* codeword                           */
   }
   IPS_AR_FAX1D_ENCODE_WHITE[ENCODE_TABLE_SIZE],
   IPS_AR_FAX1D_ENCODE_BLACK[ENCODE_TABLE_SIZE];
#else
extern struct
   {
   short int length;                     /* codeword length                   */
   short int codeword;                   /* codeword                          */
   }
   IPS_AR_FAX1D_ENCODE_WHITE[ENCODE_TABLE_SIZE],
   IPS_AR_FAX1D_ENCODE_BLACK[ENCODE_TABLE_SIZE];
#endif

/*
**  External references
*/
#ifdef NODAS_PROTO
long _IpsVerifyNotInPlace();		    /* from Ips__udp_utils.c	    */
#endif

/************************************************************************
**
**  _IpsEncodeG31d
**
**  FUNCTIONAL DESCRIPTION:
**
**      Implements compression of a raw bitmap into CCITT Group 4 encoded
**      format.
**
**  FORMAL PARAMETERS:
**      src_udp --> Pointer to source udp (initialized)
**      dst_udp --> Pointer to destination udp (uninitialized)
**      flags   --> Indicates that the encoded scanlines will be byte aligned.
**      compressed_data_size --> Indicates the number of bits needed for
**				  the compression.
**
**  IMPLICIT INPUTS:  none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE:   returns status
**  SIGNAL CODES:     none
**  SIDE EFFECTS:     none
**
************************************************************************/

long _IpsEncodeG31d(src_udp, dst_udp, flags, compressed_data_size)
struct UDP	*src_udp;
struct UDP	*dst_udp;
long  		flags;
unsigned long	*compressed_data_size;
    {
    unsigned long  buf_size_bits;        /* output buffer size in bits      */
    long           scanline_index;       /* current scanline number         */
    long           srcbitptr;            /* bit pos. = start of current line*/
    long           i;                    /* for loop counter                */
    long           nocheck_point;        /*check buffer space after here    */
    long           max_bits_per_scan;    /* max possible bits in a scan     */
    struct 	   FAX_ENCODE_PARAMS enc_parm;
    long	   size;                    
    long	   pad;
    long	   mem_alloc = 0;
    long	   status;

    /*
    ** Initialize destination UDP fixed fields & data buffer.
    */
    dst_udp->UdpL_PxlPerScn = src_udp->UdpL_PxlPerScn;
    dst_udp->UdpL_ScnCnt = src_udp->UdpL_ScnCnt;
    dst_udp->UdpL_X1 = 0;
    dst_udp->UdpL_Y1 = 0;
    dst_udp->UdpL_X2 = src_udp->UdpL_PxlPerScn - 1;
    dst_udp->UdpL_Y2 = src_udp->UdpL_ScnCnt - 1;
    dst_udp->UdpL_ScnStride = src_udp->UdpL_PxlPerScn;
    dst_udp->UdpW_PixelLength = 1;
    dst_udp->UdpL_PxlStride = 1;
    dst_udp->UdpB_DType = UdpK_DTypeVU;
    dst_udp->UdpB_Class = UdpK_ClassUBS;
    dst_udp->UdpL_Levels = 2;
    dst_udp->UdpL_CompIdx = src_udp->UdpL_CompIdx;

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
 
    if(src_udp->UdpL_PxlStride != 1) 
        return (IpsX_UNSPXLSTR);

    /* 
    ** alloc current changelist: length is (scanline length + 5) longwords    
    */
    enc_parm.ccl = (long *) (*IpsA_MemoryTable[IpsK_Alloc])
    	((sizeof(int)*src_udp->UdpL_PxlPerScn + 5),IpsM_InitMem,0);
    enc_parm.buf_ptr = dst_udp->UdpA_Base;

    buf_size_bits = dst_udp->UdpL_ArSize;	/* buffer size in bits        */

    /* 
    ** figure on the very largest possible scanline encoding, and set the     
    ** "nocheck_point" - i.e. before this point we never have to check to see 
    ** that the next scanline encoded will overflow the buffer. If we go past 
    ** the "nocheck_point" it means we will potentially overflow during       
    ** processing the next scanline and we'll signal BUFOVRFLW.               
    */

    max_bits_per_scan = MAXCODEWORDSIZE * src_udp->UdpL_PxlPerScn +
	MAXCODEWORDSIZE;
    nocheck_point = buf_size_bits - max_bits_per_scan;
    scanline_index = 0; 
    srcbitptr = (src_udp->UdpL_Pos + (src_udp->UdpL_X1 * 
	src_udp->UdpL_PxlStride) + (src_udp->UdpL_Y1 * src_udp->UdpL_ScnStride))
	    - src_udp->UdpL_ScnStride; 
    enc_parm.bufbitptr = dst_udp->UdpL_Pos;

    ADD_CODEWORD_(enc_parm.buf_ptr,enc_parm.bufbitptr,
	IPS_K_FAX1D_CODEWORD_EOL,IPS_K_FAX1D_CODELEN_EOL);

    /* 
    ** encode until all scanlines are encoded
    */
    while(scanline_index < src_udp->UdpL_ScnCnt)
        {
        srcbitptr += src_udp->UdpL_ScnStride;
        _IpsBuildChangelist(src_udp->UdpA_Base, srcbitptr, 
	    src_udp->UdpL_PxlPerScn, enc_parm.ccl, src_udp->UdpL_PxlPerScn+5);
        if (enc_parm.bufbitptr > nocheck_point)
            {
            return( IpsX_BUFOVRFLW);
            }
        else /* don't check each codeword */
            {
	    _IpsEncodeG31dScan(&enc_parm);
            if (flags & IpsM_EncodeScanlineAlign)
                {
                pad = ((enc_parm.bufbitptr + 12 ) % 8);
                enc_parm.bufbitptr += (8 - pad);
                }

            ADD_CODEWORD_(enc_parm.buf_ptr,enc_parm.bufbitptr,
                IPS_K_FAX1D_CODEWORD_EOL,IPS_K_FAX1D_CODELEN_EOL);
            }
        scanline_index++;
        } /* end outer while */

    /* add five copies of EOL to form an EOB in the output */
    for (i=0; i < 5; i++)
        {
        ADD_CODEWORD_(enc_parm.buf_ptr,enc_parm.bufbitptr,
            IPS_K_FAX1D_CODEWORD_EOL,IPS_K_FAX1D_CODELEN_EOL);
        }
    /* 
    ** Note if user passes his own buffer then pos will be not be included in
    ** what we report back
    */

    *compressed_data_size = enc_parm.bufbitptr - dst_udp->UdpL_Pos;
    
    size = (enc_parm.bufbitptr + 7)/8;
    if (mem_alloc == 1)
        {
        dst_udp->UdpA_Base = (unsigned char *)
            (*IpsA_MemoryTable[IpsK_ReallocateDataPlane])
		(dst_udp->UdpA_Base,size);
	dst_udp->UdpL_ArSize = size * 8;
	}
    /*
    ** Free dynamic memory.
    */
    (*IpsA_MemoryTable[IpsK_Dealloc])(enc_parm.ccl);
    return (IpsX_SUCCESS);
    
    }/* end of _IpsEncodeG31d */

/************************************************************************
**
**  _IpsEncodeG32d
**
**  FUNCTIONAL DESCRIPTION:
**
**      Implements compression of a raw bitmap into CCITT Group 4 encoded
**      format.
**
**  FORMAL PARAMETERS:
**      src_udp --> Pointer to source udp (initialized)
**      dst_udp --> Pointer to destination udp (uninitialized)
**      k_factor --> Used for G32d encoding.
**      flags   --> Indicates that the encoded scanlines will be byte aligned.
**      compressed_data_size --> Indicates the number of bits needed for
**				  the compression.
**
**  IMPLICIT INPUTS:  none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE:   return status
**  SIGNAL CODES:     none
**  SIDE EFFECTS:     none
**
************************************************************************/

long _IpsEncodeG32d(src_udp, dst_udp, k_factor, flags, compressed_data_size)
struct UDP *src_udp;			  /* Source data plane descriptor    */
struct UDP *dst_udp;			  /* Destination data plane descrip  */
long       k_factor;                 	  /* k factor for g32d encoding      */
long 	   flags;			  /* flag used for scanline padding  */
unsigned long *compressed_data_size;
    {
    long           scanline_index;        /* current scanline number         */
    long           srcbitptr;             /* bit pos. = start of current line*/
    unsigned long  buf_size_bits;         /* output buffer size in bits      */
    long           cidx, ridx;            /* change list indices             */
    long           *temp_ptr;             /* temp. change list pointer       */
    long           k_count=0;             /* counter for K lines             */
    long           i;                     /* for loop variable               */
    long           nocheck_point;         /* point where buffer check starts */
    long           max_bits_per_scan;     /* max bits in one encoded scan    */
    long	   size;		  /* Data plane size		     */
    long	   pad;
    long	   mem_alloc=0;
    long	   status;
    struct FAX_ENCODE_PARAMS enc_parm;
    /*
    ** Initialize destination UDP fixed fields & data buffer.
    */
    dst_udp->UdpL_PxlPerScn = src_udp->UdpL_PxlPerScn;
    dst_udp->UdpL_ScnCnt = src_udp->UdpL_ScnCnt;
    dst_udp->UdpL_X1 = 0;
    dst_udp->UdpL_Y1 = 0;
    dst_udp->UdpL_X2 = src_udp->UdpL_PxlPerScn - 1;
    dst_udp->UdpL_Y2 = src_udp->UdpL_ScnCnt - 1;
    dst_udp->UdpL_ScnStride = src_udp->UdpL_PxlPerScn;
    dst_udp->UdpW_PixelLength = 1;
    dst_udp->UdpL_PxlStride = 1;
    dst_udp->UdpB_DType = UdpK_DTypeVU;
    dst_udp->UdpB_Class = UdpK_ClassUBS;
    dst_udp->UdpL_Levels = 2;
    dst_udp->UdpL_CompIdx = src_udp->UdpL_CompIdx;

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
 
    if (k_factor <= 0)
        return (IpsX_INVARGCON);
    if(src_udp->UdpL_PxlStride != 1)  /* pixel stride not 1  */
        return(IpsX_UNSPXLSTR);

    /* 
    ** alloc curr/ref changelists: length is (scanline length + 5) longwords  
    ** In the 2D CCITT encodings, the codes specify offsets to bit positions  
    ** in reference to the previous line, the "reference" line. When starting 
    ** up, an "imaginary" first reference line changelist of all zeros is     
    ** created.                                                               
    */
    enc_parm.ccl = (long *) (*IpsA_MemoryTable[IpsK_Alloc])
			((sizeof(long)*src_udp->UdpL_PxlPerScn + 5),
				IpsM_InitMem,0);
    enc_parm.rcl = (long *) (*IpsA_MemoryTable[IpsK_Alloc])
			((sizeof(long)*src_udp->UdpL_PxlPerScn + 5),
			IpsM_InitMem,0);
    enc_parm.buf_ptr = dst_udp->UdpA_Base;

    /* buffer size in bits              */
    buf_size_bits = dst_udp->UdpL_ArSize; 

    /* 
    ** figure on the very largest possible scanline encoding, and set the     
    ** "nocheck_point" - i.e. before this point we never have to check to see 
    ** that the next scanline encoded will overflow the buffer. If we go past 
    ** the "nocheck_point" it means we will potentially overflow during       
    ** processing the next scanline and we'll signal BUFOVRFLW.               
    */
    max_bits_per_scan = MAXCODEWORDSIZE * src_udp->UdpL_PxlPerScn + 
	MAXCODEWORDSIZE;
    nocheck_point = buf_size_bits - max_bits_per_scan;
    scanline_index = 0;		    /* scan index starts at low bound */
    srcbitptr = (src_udp->UdpL_Pos + (src_udp->UdpL_X1 * 
	src_udp->UdpL_PxlStride) + (src_udp->UdpL_Y1 * src_udp->UdpL_ScnStride))
	    - src_udp->UdpL_ScnStride; 
    enc_parm.bufbitptr = dst_udp->UdpL_Pos;

    /* 
    ** make imaginary reference line changelist (makes it appear that a line  
    ** of all zeroes preceded this one).                                      
    */
    enc_parm.rcl[0] = 1;
    enc_parm.rcl[1] = src_udp->UdpL_PxlPerScn;
    enc_parm.rcl[2] = src_udp->UdpL_PxlPerScn;
    enc_parm.rcl[3] = src_udp->UdpL_PxlPerScn;
    enc_parm.rcl[4] = src_udp->UdpL_PxlPerScn;
 
    ADD_CODEWORD_(enc_parm.buf_ptr,enc_parm.bufbitptr,IPS_K_FAX1D_CODEWORD_EOL,
        IPS_K_FAX1D_CODELEN_EOL);           /* G32D image starts with EOL     */
    /* 
    ** encode until every scanline is encoded                                
    */
    while(scanline_index < src_udp->UdpL_ScnCnt)
        {
        srcbitptr += src_udp->UdpL_ScnStride;
        _IpsBuildChangelist(src_udp->UdpA_Base,
	    srcbitptr,src_udp->UdpL_PxlPerScn,
            enc_parm.ccl,src_udp->UdpL_PxlPerScn+5);

        /* 
	** after building the changelist, add a couple of extra "stoppers"    
        ** at the end, since part of the algorithm can, under certain         
        ** conditions, look one or two elements ahead in the changelist. This 
        ** prevents the algorithm from finding leftover values from previous  
        ** scans and doing very poor things.                                 
	*/
        enc_parm.ccl[enc_parm.ccl[0]+1] = src_udp->UdpL_PxlPerScn;
        enc_parm.ccl[enc_parm.ccl[0]+2] = src_udp->UdpL_PxlPerScn;
        enc_parm.ccl[enc_parm.ccl[0]+3] = src_udp->UdpL_PxlPerScn;

        /* 
	** in G32D encoding, every K-th line is encoded in the G31D format    
        ** and the intervening lines are encoded in G42D format.              
        ** for more information, see the CCITT T.4 and T.6 specs.            
	*/
        if (k_count == 0)                    /* is next line the 1D line?  */
            {
	    /*
	    **  Add the 1D tag
            **  *((long *)(enc_parm.buf_ptr + (enc_parm.bufbitptr >> 3))) ^= 
            **    TAG_K_1D << (enc_parm.bufbitptr & 0x7);
            */
	    XOR_WRITE32_(enc_parm.buf_ptr + (enc_parm.bufbitptr >> 3),
                    TAG_K_1D << (enc_parm.bufbitptr & 0x7));
            enc_parm.bufbitptr++;
            _IpsEncodeG31dScan(&enc_parm);
            if (flags & IpsM_EncodeScanlineAlign)
                {
                pad = ((enc_parm.bufbitptr + 12 + 1) % 8);
                enc_parm.bufbitptr += (8 - pad);
                }

            ADD_CODEWORD_(enc_parm.buf_ptr,enc_parm.bufbitptr,
                IPS_K_FAX1D_CODEWORD_EOL,IPS_K_FAX1D_CODELEN_EOL);
            }
        else
            {
            /* 
	    ** NOTE that since TAG_K_2D == 0,                                 
            ** it is unnecessary to write a zero bit into a zeroed buffer     
	    */
            enc_parm.bufbitptr++;                   /* add 1 for the tag bit..*/
            if (enc_parm.bufbitptr > nocheck_point) /* check buffer space?    */
               {
               return(IpsX_BUFOVRFLW);
               }
            else  /* no, plenty of room    */
               {
               _IpsEncodeG42dScan(&enc_parm);
                if (flags & IpsM_EncodeScanlineAlign)
                    {
		    pad = ((enc_parm.bufbitptr + 12 + 1) % 8);
                    enc_parm.bufbitptr += (8 - pad);
                    }
               ADD_CODEWORD_(enc_parm.buf_ptr,enc_parm.bufbitptr,
                   IPS_K_FAX1D_CODEWORD_EOL,IPS_K_FAX1D_CODELEN_EOL);
               }
            }
        if (++k_count == k_factor) /* reset k_count if == k   */
            k_count = 0;

        /* swap pointers for change and reference lines */
        temp_ptr = enc_parm.rcl;
        enc_parm.rcl = enc_parm.ccl;
        enc_parm.ccl = temp_ptr;
        scanline_index++;
        } /* end outer while */
    /* 
    **  Add five copies of EOL and 1D tagbit to form an EOB in the output      
    **  *(enc_parm.buf_ptr + (enc_parm.bufbitptr >> 3)) ^= TAG_K_1D << 
    **  (enc_parm.bufbitptr & 0x7);
    */
    XOR_WRITE32_(enc_parm.buf_ptr + (enc_parm.bufbitptr >> 3),
            TAG_K_1D << (enc_parm.bufbitptr & 0x7));
    enc_parm.bufbitptr++;

    for(i = 0;i < 5; i++)
        {
        ADD_CODEWORD_(enc_parm.buf_ptr,enc_parm.bufbitptr,
            IPS_K_FAX1D_CODEWORD_EOL,IPS_K_FAX1D_CODELEN_EOL);
        /* 
	**  Add the 1D tagbit to buffer
        **  *((enc_parm.buf_ptr + (enc_parm.bufbitptr >> 3))) ^= TAG_K_1D << 
	**	(enc_parm.bufbitptr & 0x7);
	*/
        XOR_WRITE32_(enc_parm.buf_ptr + (enc_parm.bufbitptr >> 3),
                TAG_K_1D << (enc_parm.bufbitptr & 0x7));
        enc_parm.bufbitptr++;
        }
    /* 
    ** Note if user passes his own buffer then pos will be not be included in
    ** what we report back
    */
    *compressed_data_size = enc_parm.bufbitptr - dst_udp->UdpL_Pos;
    size = (long)(enc_parm.bufbitptr + 7) / 8;

    if (mem_alloc == 1)
	{
	dst_udp->UdpA_Base = (unsigned char *)
	    (*IpsA_MemoryTable[IpsK_ReallocateDataPlane])(dst_udp->UdpA_Base,size);
	dst_udp->UdpL_ArSize = size * 8;
	}
    /*
    ** Deallocate dynamic memory.
    */
    (*IpsA_MemoryTable[IpsK_Dealloc])(enc_parm.ccl);
    (*IpsA_MemoryTable[IpsK_Dealloc])(enc_parm.rcl);
    return (IpsX_SUCCESS);
    }/*  end of _IpsEncodeG32d routine */

/************************************************************************
**
**  _IpsEncodeG42d
**
**  FUNCTIONAL DESCRIPTION:
**
**      Implements compression of a raw bitmap into CCITT Group 4 encoded
**      format.
**
**  FORMAL PARAMETERS:
**
**      src_udp --> Pointer to source udp (initialized)
**      dst_udp --> Pointer to destination udp (uninitialized)
**      compressed_data_size --> Indicates the number of bits needed for
**				  the compression.
**
**  IMPLICIT INPUTS:  none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE:   returns status
**  SIDE EFFECTS:     none
**  SIGNAL CODES:     none
**
************************************************************************/

long _IpsEncodeG42d(src_udp, dst_udp, compressed_data_size)
struct UDP      *src_udp;
struct UDP      *dst_udp;
unsigned long   *compressed_data_size;
    {
    long	       size;
    long           scanline_index;          /* current scanline number         */
    long           srcbitptr;               /* bit pos. = start of current line*/
    unsigned long  buf_size_bits;           /* output buffer size in bits      */
    long           cidx, ridx;              /* change list indices             */
    long           *temp_ptr;               /* temp change list pointer        */
    long           max_bits_per_scan;       /* max possible bits encoded/scan  */
    long           nocheck_point;           /* buf_size_bits-max_bits_per_scan */
    long	       status;			/* status returned by verifynoinplace*/
    long	       mem_alloc = 0;
    struct FAX_ENCODE_PARAMS enc_parm;      /* the encoding param descriptor   */

    /*
    ** Initialize destination UDP fixed fields & data buffer
    */
    dst_udp->UdpL_PxlPerScn = src_udp->UdpL_PxlPerScn;
    dst_udp->UdpL_ScnCnt = src_udp->UdpL_ScnCnt;
    dst_udp->UdpL_X1 = 0;
    dst_udp->UdpL_Y1 = 0;
    dst_udp->UdpL_X2 = src_udp->UdpL_PxlPerScn - 1;
    dst_udp->UdpL_Y2 = src_udp->UdpL_ScnCnt - 1;
    dst_udp->UdpL_ScnStride = src_udp->UdpL_PxlPerScn;
    dst_udp->UdpW_PixelLength = 1;
    dst_udp->UdpL_PxlStride = 1;
    dst_udp->UdpB_DType = UdpK_DTypeVU;
    dst_udp->UdpB_Class = UdpK_ClassUBS;
    dst_udp->UdpL_Levels = 2;
    dst_udp->UdpL_CompIdx = src_udp->UdpL_CompIdx;
    
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
        dst_udp->UdpL_Pos = 0;
        dst_udp->UdpL_ArSize = size;
        mem_alloc = 1;
        }
 
    if (src_udp->UdpL_PxlStride != 1) 
       return (IpsX_UNSPXLSTR);

    /* 
    ** alloc curr/ref changelists: length is (pixels_per_line + 5) longwords  
    ** current and reference changelists are pixels_per_line+5 integers       
    ** In the 2D CCITT encodings, the codes specify offsets to bit positions  
    ** in reference to the previous line, the "reference" line. When starting 
    ** up, an "imaginary" first reference line changelist of all zeros is     
    ** created.                                                               
    */
    enc_parm.ccl = (long *) (*IpsA_MemoryTable[IpsK_Alloc])
	((sizeof(long)*src_udp->UdpL_PxlPerScn + 5),IpsM_InitMem,0);
    enc_parm.rcl = (long *) (*IpsA_MemoryTable[IpsK_Alloc])
	((sizeof(long) * src_udp->UdpL_PxlPerScn + 5),IpsM_InitMem,0);
    enc_parm.buf_ptr = dst_udp->UdpA_Base;

    /* buffer size in bits              */
    buf_size_bits = dst_udp->UdpL_ArSize;

    /* 
    ** figure on the very largest possible scanline encoding, and set the     
    ** "nocheck_point" - i.e. before this point we never have to check to see 
    ** that the next scanline encoded will overflow the buffer. If we go past 
    ** the "nocheck_point" it means we will potentially overflow during       
    ** processing the next scanline and we'll signal BUFOVRFLW.               
    */

    max_bits_per_scan = MAXCODEWORDSIZE * src_udp->UdpL_PxlPerScn + MAXCODEWORDSIZE;
    nocheck_point = buf_size_bits - max_bits_per_scan;
    scanline_index = 0;
    srcbitptr = (src_udp->UdpL_Pos + (src_udp->UdpL_X1 * 
	src_udp->UdpL_PxlStride) + (src_udp->UdpL_Y1 * src_udp->UdpL_ScnStride))
		- src_udp->UdpL_ScnStride; 

    /* 
    ** start position of line      
    */
    enc_parm.bufbitptr = dst_udp->UdpL_Pos;

    /* 
    ** make imaginary reference line changelist (makes it appear that a line  
    ** of all zeroes preceded this one).                                      
    */
    enc_parm.rcl[0] = 1;
    enc_parm.rcl[1] = src_udp->UdpL_PxlPerScn;
    enc_parm.rcl[2] = src_udp->UdpL_PxlPerScn;
    enc_parm.rcl[3] = src_udp->UdpL_PxlPerScn;
    enc_parm.rcl[4] = src_udp->UdpL_PxlPerScn;

    /* 
    ** encode until scanlines 0 to number_of_lines are encoded 
    */

    while(scanline_index < src_udp->UdpL_ScnCnt)
        {
        srcbitptr += src_udp->UdpL_ScnStride;
        _IpsBuildChangelist(src_udp->UdpA_Base,srcbitptr,src_udp->UdpL_PxlPerScn,
             enc_parm.ccl, src_udp->UdpL_PxlPerScn+5);

        /* 
	** after building the changelist, add a couple of extra "stoppers"    
	** at the end, since part of the algorithm can, under certain         
        ** conditions, look one or two elements ahead in the changelist. This 
	** prevents the algorithm from finding leftover values from previous  
        ** scans and doing very poor things.                                  
        ** add extras to prevent an  overrun condition from occurring        
	*/
        enc_parm.ccl[enc_parm.ccl[0]+1] = src_udp->UdpL_PxlPerScn;
        enc_parm.ccl[enc_parm.ccl[0]+2] = src_udp->UdpL_PxlPerScn; 
        enc_parm.ccl[enc_parm.ccl[0]+3] = src_udp->UdpL_PxlPerScn; 
        if (enc_parm.bufbitptr > nocheck_point)
           return (IpsX_BUFOVRFLW);
        else
           _IpsEncodeG42dScan(&enc_parm);
	/* swap pointers for change and reference lines */
        temp_ptr = enc_parm.rcl;
	enc_parm.rcl = enc_parm.ccl;
	enc_parm.ccl = temp_ptr;
	scanline_index++;
	} /* end outer while */

    /* 
    ** add two copies of EOL to form an EOB in the output 
    */
    ADD_CODEWORD_(enc_parm.buf_ptr,enc_parm.bufbitptr,
	IPS_K_FAX1D_CODEWORD_EOL,IPS_K_FAX1D_CODELEN_EOL);
    ADD_CODEWORD_(enc_parm.buf_ptr,enc_parm.bufbitptr,
	IPS_K_FAX1D_CODEWORD_EOL,IPS_K_FAX1D_CODELEN_EOL);

    /* 
    ** Note if user passes his own buffer then pos will be not be included in
    ** what we report back
    */
    *compressed_data_size = enc_parm.bufbitptr - dst_udp->UdpL_Pos;
    size = (long)(enc_parm.bufbitptr + 7) / 8;

    if (mem_alloc == 1)
        {
        dst_udp->UdpA_Base = (unsigned char *)
            (*IpsA_MemoryTable[IpsK_ReallocateDataPlane])(dst_udp->UdpA_Base,size);
        dst_udp->UdpL_ArSize = size * 8;
        }

    /*
    ** Free dynamic memory.
    */
    (*IpsA_MemoryTable[IpsK_Dealloc])(enc_parm.ccl);
    (*IpsA_MemoryTable[IpsK_Dealloc])(enc_parm.rcl);
    return (IpsX_SUCCESS);
    }/* end _IpsEncodeG42d routine */
