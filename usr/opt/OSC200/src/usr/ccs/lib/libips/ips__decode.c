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
**  _IpsDecode
** 
**  FACILITY:
**
**      Image Processing Services (IPS)
**
**  ABSTRACT:
**
**      Decode frame with CCITT Group 3-1D encoded image into bitmap.
**
**  ENVIRONMENT:
**
**      VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**
**
**  AUTHOR(S):
**
**      Ken MacDonald, Digital Equipment Corp.
**      Revised for V3.0 by Karen Rodwell, Digital Equipment Corp.
**
**  CREATION DATE:
**
**      October 20, 1989
**
************************************************************************/

/*
**  Table of contents:
*/
#ifdef NODAS_PROTO
long _IpsDecodeG31d();  /* decode G31D image frame to buffer routine */
long _IpsDecodeG32d();  /* decode G32D image frame to buffer routine */
long _IpsDecodeG42d();  /* decode G42D image frame to buffer routine */
#endif

/*
**  Include files:
*/
#include        <IpsDef.h>              /* image frame constants             */
#include        <IpsStatusCodes.h>      /* IPS Status Codes                  */
#include        <IpsMemoryTable.h>      /* IPS Memory Mgt. Functions         */
#ifndef NODAS_PROTO
#include	<ipsprot.h>		/* Ips prototypes */
#endif
#include        <ips_fax_macros.h>      /* fax macros like $GET_12_BITS      */
#include        <ips_fax_definitions.h> /* FAX definitions                   */
#include        <ips_fax_paramdef.h>    /* FAX routine parameter block defns.*/

/* Image Services Library routines                                           */
#ifdef NODAS_PROTO
void _IpsPutRuns();                     /*  generates runs for 1 scanline    */
long _IpsDecodeG42dScan();              /*  routine decodes 1 scanline       */
long _IpsDecodeG31dScan();              /*  routine decodes 1 scanline       */
#endif

/*
**  External references
*/
#ifdef NODAS_PROTO
long _IpsVerifyNotInPlace();		/*  from IPS__UDP_UTILS.C            */
#endif

/*
**  Module local functions
*/
#ifdef NODAS_PROTO
static long _IpsFindNextEOL ();
static long _IpsFindNextG31DEOL ();
#else
PROTO( static long _IpsFindNextEOL, ( struct FAX_DECODE_PARAMS *) );
PROTO( static long _IpsFindNextG31DEOL, ( struct FAX_DECODE_PARAMS *) );
#endif

/************************************************************************
**
**  _IpsDecodeG31d
**
**  FUNCTIONAL DESCRIPTION:
**
**      Decodes an image frame which has been compressed using the CCITT
**	Group 3-1D standard compression technique.
**
**  FORMAL PARAMETERS:
**
**      src_udp --> Pointer to source udp (initialized)
**      dst_udp --> Pointer to destination udp (uninitialized)
**	flag --> Used to indicate if destination is padded to byte.
**
**
**  IMPLICIT INPUTS:  none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE:   returns status
**  SIGNAL CODES:     none
**  SIDE EFFECTS:     none
**
************************************************************************/

long _IpsDecodeG31d(src_udp, dst_udp,flag)
struct UDP *src_udp;
struct UDP *dst_udp;
unsigned long flag;
    {
    struct FAX_DECODE_PARAMS dec_parm;      /* parm block for DECODE_xxxxSCAN */
    long           bufbitptr=0;             /* ptrs to source/output bits     */
    unsigned long  buf_size_bits;           /* # bits in output buffer        */
    long           scanline_index;          /* current scanline number        */
    long           eol_count = 0;           /* end of line count              */
    long	   should_be_eol = 0;	    /* 1st 12 bits of buf must be EOL */
    long	   size;		    /* buffer size		      */
    long	   status;                
    long	   i;
    unsigned long  result_flag = IpsX_SUCCESS;
    
    if(src_udp->UdpL_PxlStride != 1)            
        return (IpsX_UNSPXLSTR);
    if ((src_udp->UdpL_X1 != 0) || (src_udp->UdpL_Y1 != 0))
        return (IpsX_INCNSARG);

    /*
    ** Initialize destination UDP fixed fields & data buffer.
    */
    dst_udp->UdpL_PxlPerScn = src_udp->UdpL_PxlPerScn;
    dst_udp->UdpL_ScnCnt = src_udp->UdpL_ScnCnt;
    dst_udp->UdpL_X1 = 0;
    dst_udp->UdpL_Y1 = 0;
    dst_udp->UdpL_X2 = src_udp->UdpL_PxlPerScn - 1;
    dst_udp->UdpL_Y2 = src_udp->UdpL_ScnCnt - 1;
/*    dst_udp->UdpL_ScnStride = dst_udp->UdpL_X2 - dst_udp->UdpL_X1 + 1;  */
    dst_udp->UdpL_ScnStride = src_udp->UdpL_ScnStride;
    if (flag != IpsK_DTypeIntBits)
        dst_udp->UdpL_ScnStride += ((32 - (dst_udp->UdpL_ScnStride % 32))) % 32;
    dst_udp->UdpW_PixelLength = 1;
    dst_udp->UdpL_PxlStride = 1;
    dst_udp->UdpB_DType = UdpK_DTypeVU;
    dst_udp->UdpB_Class = UdpK_ClassUBA;
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
        dst_udp->UdpL_ArSize = size;
        size = ((dst_udp->UdpL_ArSize + 7)/8);
        dst_udp->UdpA_Base = (unsigned char *)
           (*IpsA_MemoryTable[IpsK_AllocateDataPlane])(size,IpsM_InitMem,0);
        if (!dst_udp->UdpA_Base) return (IpsX_INSVIRMEM);
        dst_udp->UdpL_Pos = 0;
        }
    
    /* alloc current changelist: length is (scanline length + 4) longwords    */
    dec_parm.pixels_per_line = src_udp->UdpL_PxlPerScn;/* scanline_size (bits)*/
    dec_parm.ccl = (long *) (*IpsA_MemoryTable[IpsK_Alloc])
	(((src_udp->UdpL_PxlPerScn + 4) * sizeof(long)), IpsM_InitMem,0);

    /* buffer size in bits         */
    buf_size_bits = dst_udp->UdpL_ArSize - dst_udp->UdpL_Pos;
    dec_parm.base = src_udp->UdpA_Base;
    scanline_index = 0;                        /* first scanline              */

    /* skip leading EOL in the image.                                         */
    dec_parm.srcbitptr = src_udp->UdpL_Pos + IPS_K_FAX1D_CODELEN_EOL;

    /* go until 5 EOL's found, or scanline upper bound is reached             */
    while(eol_count < 5 && scanline_index < src_udp->UdpL_ScnCnt)
        {
        dec_parm.ccl[0] = 0;
        /* 
	** check for being less than one scan away from the end of the buffer 
        ** if so, we'll call the action routine. If not specified, signal     
	** a buffer overflow                                                  
	*/
	if (bufbitptr + dst_udp->UdpL_ScnStride > buf_size_bits)
	    return(IpsX_BUFOVRFLW);
 
	/* get one scanline. changes stored in dec_parm.ccl[]                 */
	status = _IpsDecodeG31dScan(&dec_parm);
    
	/*
	**  If _IpsDecodeG31dScan complains about invalid scanline
	**  length, run through the current change list and fix up the
	**  change list entries that are invalid (meaning they go beyond
	**  the expected number of pixels per line).
	*/
	if ( status == IpsX_INVSCNLEN )
	    {
	    result_flag = IpsX_INVSCNLEN;
	    /* 
	    ** reject offsets > pxl per scn in the changelist and
	    ** adjust accordingly
	    */
	    i = dec_parm.ccl[0];
	    while (dec_parm.ccl[i] > dst_udp->UdpL_PxlPerScn)
		i--;
	    dec_parm.ccl[0] = i;
	    dec_parm.ccl[i + 1] = dst_udp->UdpL_PxlPerScn;
	    dec_parm.ccl[i + 2] = dst_udp->UdpL_PxlPerScn;
	    dec_parm.ccl[i + 3] = dst_udp->UdpL_PxlPerScn;
	    }

	/*
	**  If the scanline decompress failed with invalid code type,
	**  attempt to find the next valid EOL and start again (at the 
	**  top of the loop) ...
	*/
	if ( status == IpsX_INVCODTYP )
	    {
	    status = _IpsFindNextEOL( &dec_parm );
	    if ( !(status&1) )
		return (IpsX_FAILURE);
	    }
	/*
	**  ... else fill runs by passing the changelist to _IpsPutRuns
	*/
	else
	    {
	    _IpsPutRuns(dst_udp->UdpA_Base, bufbitptr, dec_parm.ccl);
	    }

	/* check for empty lines (ccl[0] == 0); if not empty increment        */
	/* buffer ptr by the scanline stride                                  */
	if (dec_parm.ccl[0] == 0) 
	    eol_count++;
	else
	    {
	    scanline_index++;
	    bufbitptr += dst_udp->UdpL_ScnStride;
	    }

	} /* end outer while */

    (*IpsA_MemoryTable[IpsK_Dealloc])(dec_parm.ccl);
    return (result_flag);
    } /* End _IpsDecodeG31d */

/************************************************************************
**
** _IpsDecodeG32d
**
**  FUNCTIONAL DESCRIPTION:
**
**      Decodes an image frame which has been compressed using the CCITT
**	Group 3/2D standard compression technique.
**
**  FORMAL PARAMETERS:
**
**      src_udp --> Pointer to source udp (initialized)
**      dst_udp --> Pointer to destination udp (uninitialized)
**	flag --> Used to indicate if destination is padded to byte.
**
**  IMPLICIT INPUTS:  none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE:   returns status 
**  SIDE EFFECTS:     none
**  SIGNAL CODES:     none
**
************************************************************************/

long _IpsDecodeG32d(src_udp, dst_udp,flag)
struct UDP *src_udp;
struct UDP *dst_udp;
unsigned long flag;
    {
    long size;				    /* buffer size		      */
    struct FAX_DECODE_PARAMS dec_parm;      /* parm block for DECODE_xxxxSCAN */
    long           bufbitptr=0;		    /* ptrs to source/output bits     */
    unsigned long  buf_size_bits;           /* # bits in output buffer        */
    long           *temp_ptr;               /* temp. list pointer             */
    long           scanline_index;          /* current scanline number        */
    long           eol_count = 0;           /* end of line count              */
    long	   status;

    /*
    ** Initialize destination UDP fixed fields & data buffer.
    */
    if(src_udp->UdpL_PxlStride != 1)            
        return (IpsX_UNSPXLSTR);

    if ((src_udp->UdpL_X1 != 0) || (src_udp->UdpL_Y1 != 0))
        return (IpsX_INCNSARG);

    dst_udp->UdpL_PxlPerScn = src_udp->UdpL_PxlPerScn;
    dst_udp->UdpL_ScnCnt = src_udp->UdpL_ScnCnt;
    dst_udp->UdpL_X1 = 0;
    dst_udp->UdpL_Y1 = 0;
    dst_udp->UdpL_X2 = src_udp->UdpL_PxlPerScn - 1;
    dst_udp->UdpL_Y2 = src_udp->UdpL_ScnCnt - 1;
/*    dst_udp->UdpL_ScnStride = dst_udp->UdpL_X2 - dst_udp->UdpL_X1 + 1;    */
    dst_udp->UdpL_ScnStride = src_udp->UdpL_ScnStride;
    if (flag != IpsK_DTypeIntBits)
        dst_udp->UdpL_ScnStride += ((32 - (dst_udp->UdpL_ScnStride % 32))) % 32;
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
        dst_udp->UdpL_ArSize = size;
        size = ((dst_udp->UdpL_ArSize + 7)/8);
        dst_udp->UdpA_Base = (unsigned char *)
           (*IpsA_MemoryTable[IpsK_AllocateDataPlane])(size,IpsM_InitMem,0);
        if (!dst_udp->UdpA_Base) return (IpsX_INSVIRMEM);
        dst_udp->UdpL_Pos = 0;
        }
    
    /* alloc curr/ref changelists: length is (scanline length + 4) longwords  */
    dec_parm.pixels_per_line = src_udp->UdpL_PxlPerScn;/* scanline_size (bits)*/
    dec_parm.base = src_udp->UdpA_Base;
    dec_parm.ccl = (long *) (*IpsA_MemoryTable[IpsK_Alloc])
	((sizeof(long) * (dec_parm.pixels_per_line + 4)), IpsM_InitMem, 0);
    dec_parm.rcl = (long *)(*IpsA_MemoryTable[IpsK_Alloc])
	((sizeof(long) * (dec_parm.pixels_per_line + 4)),IpsM_InitMem,0);
    /* buffer size in bits         */
    buf_size_bits = dst_udp->UdpL_ArSize - dst_udp->UdpL_Pos;
    scanline_index = 0;                        /* first scanline              */

    /* In the 2D CCITT encodings, the codes specify offsets to bit positions  */
    /* in reference to the previous line, the "reference" line. When starting */
    /* up, an "imaginary" first reference line changelist of all zeros is     */
    /* created.                                                               */
    dec_parm.rcl[0] = 1;
    dec_parm.rcl[1] = dec_parm.pixels_per_line;
    dec_parm.rcl[2] = dec_parm.pixels_per_line;
    dec_parm.rcl[3] = dec_parm.pixels_per_line;
    dec_parm.rcl[4] = dec_parm.pixels_per_line;

    /* start srcbitptr to skip leading EOL in the image...                    */
    dec_parm.srcbitptr = src_udp->UdpL_Pos + IPS_K_FAX1D_CODELEN_EOL;

    /* go until 5 EOL's found, or scanline upper bound is reached             */
    while(eol_count < 5 && scanline_index < src_udp->UdpL_ScnCnt)
        {
        dec_parm.ccl[0] = 0;
        /* check for being less than one scan away from the end of the buffer */
        /* if so, we'll call the action routine. If not specified, signal     */
        /* a buffer overflow                                                  */
 
        if (bufbitptr + dst_udp->UdpL_ScnStride > buf_size_bits)
            return(IpsX_BUFOVRFLW);

        /* get one scanline. changes stored in dec_parm.ccl[]                 */

        if ((GET_1_BIT_(dec_parm.base,dec_parm.srcbitptr)) == TAG_K_1D)
            {
            dec_parm.srcbitptr++;
            /* EOL is skipped by IpsDecodeG31dScan */
            status = _IpsDecodeG31dScan(&dec_parm);

            }
        else
            {
            dec_parm.srcbitptr++;
            status = _IpsDecodeG42dScan(&dec_parm);
            /*
            **  cycle through any number of fill bits that may be optionally
            **  present at the end of a g4-2d encoded line (but ONLY in
            **  g3-2d images!). This 'while' will stop when it gets to a 
            **  bit, which will then be the EOL.
            **  Note that the extra set of parentheses in the while is needed
            **  so that the GET_12 macro is correctly evaluated...
            */
	    if ( status&1)
		{
		while ((GET_12_BITS_(dec_parm.base,dec_parm.srcbitptr)) == 0)
                    dec_parm.srcbitptr++;
		dec_parm.srcbitptr += IPS_K_FAX1D_CODELEN_EOL; /* skip EOL  */
		}
            }
    
	/*
        **  Fill runs by passing the changelist to _IpsPutRuns
	**  if the scanline decoded successfully
	*/
	if ( status&1 )
	    {
	    _IpsPutRuns(dst_udp->UdpA_Base,bufbitptr,dec_parm.ccl);
	    temp_ptr = dec_parm.rcl;               /* swap curr/reference line ptr*/
	    dec_parm.rcl = dec_parm.ccl;
	    dec_parm.ccl = temp_ptr;
	    }
	else
	/*
	**  Attempt to error correct by finding next EOL + G31D Tag
	**  so that we continue as if nothing happened (except that some
	**  scanline data dropped out -- how much dropped out we won't
	**  worry about).
	*/
	    {
	    status = _IpsFindNextG31DEOL( &dec_parm );
	    if ( !(status&1) )
		return (IpsX_FAILURE);
	    }
    
        /* 
	** check for EOL codes on empty lines else inc return ptr by the stride
	*/
        if (dec_parm.rcl[0] == 0) 
            eol_count++;
        else
            {
            scanline_index++;
            bufbitptr += dst_udp->UdpL_ScnStride;
            }
        } /* end outer while */

    (*IpsA_MemoryTable[IpsK_Dealloc])(dec_parm.ccl);
    (*IpsA_MemoryTable[IpsK_Dealloc])(dec_parm.rcl);
    return (IpsX_SUCCESS);
    }/* end _IpsDecodeG32d */

/************************************************************************
**
**  _IpsDecodeG42d
**
**  FUNCTIONAL DESCRIPTION:
**
**      Decodes an image frame which has been compressed using the CCITT
**	Group 4 standard compression technique.
**
**  FORMAL PARAMETERS:
**      src_udp --> Pointer to source udp (initialized)
**      dst_udp --> Pointer to destination udp (uninitialized)
**	flag --> Used to indicate if destination is padded to byte.
**
**  IMPLICIT INPUTS:  none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE:   returns status
**  SIDE EFFECTS:     none
**  SIGNAL CODES:     none
**
************************************************************************/

long _IpsDecodeG42d(src_udp, dst_udp,flag)
struct UDP *src_udp;
struct UDP *dst_udp;
unsigned long flag;
    {
    struct FAX_DECODE_PARAMS dec_parm;      /* parm block for DECODE_xxxxSCAN */
    long	   size;		        /* buffer size		          */
    long           bufbitptr=0;             /* ptrs to source/output bits     */
    unsigned long  buf_size_bits;           /* # bits in output buffer        */
    long           *temp_ptr;               /* temp. change list pointer      */
    long           scanline_index;          /* current scanline number        */
    long           eol_count = 0;           /* end of line count              */
    long	   status;
    long	   retStatus	= IpsX_SUCCESS;
        
    /*
    ** Initialize destination UDP fixed fields & data buffer.
    */
    if(src_udp->UdpL_PxlStride != 1)            
        return (IpsX_UNSPXLSTR);
    
    if ((src_udp->UdpL_X1 != 0) || (src_udp->UdpL_Y1 != 0))
        return (IpsX_INCNSARG);

    dst_udp->UdpL_PxlPerScn = src_udp->UdpL_PxlPerScn;
    dst_udp->UdpL_ScnCnt = src_udp->UdpL_ScnCnt;
    dst_udp->UdpL_X1 = 0;
    dst_udp->UdpL_Y1 = 0;
    dst_udp->UdpL_X2 = src_udp->UdpL_PxlPerScn - 1;
    dst_udp->UdpL_Y2 = src_udp->UdpL_ScnCnt - 1;
/*    dst_udp->UdpL_ScnStride = dst_udp->UdpL_X2 - dst_udp->UdpL_X1 + 1;    */
    dst_udp->UdpL_ScnStride = src_udp->UdpL_ScnStride;
    if (flag != IpsK_DTypeIntBits)
        dst_udp->UdpL_ScnStride += ((32 - (dst_udp->UdpL_ScnStride % 32))) % 32;
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
        dst_udp->UdpL_ArSize = size;
        size = ((dst_udp->UdpL_ArSize + 7)/8);
        dst_udp->UdpA_Base = (unsigned char *)
               (*IpsA_MemoryTable[IpsK_AllocateDataPlane])(size,IpsM_InitMem,0);
        if (!dst_udp->UdpA_Base) return (IpsX_INSVIRMEM);
        dst_udp->UdpL_Pos = 0;
        }
    
    /* alloc curr/ref changelists: length is (scanline length + 4) longwords  */
    dec_parm.pixels_per_line = src_udp->UdpL_PxlPerScn;/* scanline_size (bits)*/
    dec_parm.ccl = (long *) (*IpsA_MemoryTable[IpsK_Alloc])
	((sizeof(long)* (dec_parm.pixels_per_line + 4)),IpsM_InitMem,0);
    dec_parm.rcl = (long *) (*IpsA_MemoryTable[IpsK_Alloc])
	((sizeof(long)* (dec_parm.pixels_per_line + 4)),IpsM_InitMem,0);

    dec_parm.base = src_udp->UdpA_Base;
    /* buffer size in bits         */
    buf_size_bits = dst_udp->UdpL_ArSize - dst_udp->UdpL_Pos;
    scanline_index = 0;                        /* first scanline              */
    dec_parm.srcbitptr = src_udp->UdpL_Pos;    /* starting bit position       */

    /* In the 2D CCITT encodings, the codes specify offsets to bit positions  */
    /* in reference to the previous line, the "reference" line. When starting */
    /* up, an "imaginary" first reference line changelist of all zeros is     */
    /* created.                                                               */
    dec_parm.rcl[0] = 1;
    dec_parm.rcl[1] = dec_parm.pixels_per_line;
    dec_parm.rcl[2] = dec_parm.pixels_per_line;
    dec_parm.rcl[3] = dec_parm.pixels_per_line;
    dec_parm.rcl[4] = dec_parm.pixels_per_line;
    
    /* go until 2 EOL's found, or scanline upper bound is reached             */
    while(eol_count < 2 && scanline_index < src_udp->UdpL_ScnCnt)
        {
        dec_parm.ccl[0] = 0;
        /* 
	** check for being less than one scan away from the end of the buffer 
        ** signal a buffer overflow                                           
	*/
        if (bufbitptr + dst_udp->UdpL_ScnStride > buf_size_bits)
            return(IpsX_BUFOVRFLW);

        /* get one scanline. changes stored in dec_parm.ccl[]                 */
        status = _IpsDecodeG42dScan(&dec_parm);
	if ( !(status&1) )
	    {
	    retStatus = status;
	    break;
	    }
	    
    
        /* fill runs by passing the changelist to _IpsPutRuns                 */
        _IpsPutRuns(dst_udp->UdpA_Base,bufbitptr,dec_parm.ccl);
        temp_ptr = dec_parm.rcl;               /* swap curr/reference line ptr*/
        dec_parm.rcl = dec_parm.ccl;
        dec_parm.ccl = temp_ptr;
    
        /* check for EOL codes on empty lines else inc return ptr by 
        ** the stride*/
        if (dec_parm.rcl[0] == 0) 
            eol_count++;
        else
            {
            scanline_index++;
            bufbitptr += dst_udp->UdpL_ScnStride;
            }
        } /* end outer while */
    (*IpsA_MemoryTable[IpsK_Dealloc]) (dec_parm.ccl);
    (*IpsA_MemoryTable[IpsK_Dealloc]) (dec_parm.rcl);
    return (retStatus);
    } /* end of _IpsDecodeG42d */


/**************************************************************************
**  _IpsFindNextEOL 
**
**	Find the next EOL (end of line) in a G31D image
**	and set up the FAX_DECODE_PARAMS block to point to first
**	bit of the next line.
**
**	This routine is for G31D data.
**
***************************************************************************/
static long _IpsFindNextEOL (
    struct FAX_DECODE_PARAMS	*dec_parm
    )
{
long	bit;
long	eol_found   = 0;
long	eolt1_found = 0;
long	status	    = IpsX_SUCCESS;
long	zero_cnt    = 0;
long	x;

/*
**  Loop until EOL is found
**
**	NOTE:	This loop assumes this will be found.  If it isn't
**		the loop will eventually run off the end of the image
**		buffer and ACCVIO.
*/
while ( !eol_found )
    {
    bit = GET_1_BIT_( dec_parm->base, dec_parm->srcbitptr );
    ++dec_parm->srcbitptr;
    if ( bit )
	{
	if ( zero_cnt >= 11 )
	    eol_found = 1;	/* eol found	*/
	zero_cnt = 0;
	}
    else
	{
	++zero_cnt;
	}
    } /* end of while loop  */

return status;
} /* end of _IpsFindNextEOL */


/**************************************************************************
**  _IpsFindNextG31DEOL 
**
**	Find the next EOL (end of line) and G31D tag bit combination,
**	and set up the FAX_DECODE_PARAMS block to point to the tag
**	bit (which will be re-read by the calling routine).
**
**	This routine is for G32D data.
**
**	Tag scheme:
**	    1	signifies following line is 1D encoded 
**		(we're looking for this)
**	    0	signifies following line is 2D encoded
**		(we'll skip these)
***************************************************************************/
static long _IpsFindNextG31DEOL (
    struct FAX_DECODE_PARAMS	*dec_parm
    )
{
long	bit;
long	eol_found   = 0;
long	eolt1_found = 0;
long	status	    = IpsX_SUCCESS;
long	zero_cnt    = 0;
long	x;

/*
**  Loop until EOL + 1_TAG_BIT found
**
**	NOTE:	This loop assumes this will be found.  If it isn't
**		the loop will eventually run off the end of the image
**		buffer and ACCVIO.
*/
while ( !eolt1_found )
    {
    bit = GET_1_BIT_( dec_parm->base, dec_parm->srcbitptr );
    ++dec_parm->srcbitptr;
    if ( bit )
	{
	if ( eol_found )
	    eolt1_found = 1;	/* this is the eol + tag we want    */
	else
	    if ( zero_cnt >= 11 )
		eol_found = 1;	/* eol found	*/
	zero_cnt = 0;
	}
    else
	{
	++zero_cnt;
	if ( eol_found )
	    {
	    eol_found = 0;  /* wrong eol; this one is followed by a 2D line */
	    }
	}
    } /* end of while loop  */

/*
**  Back off src bit ptr by one bit (which unreads the tag bit)
**
**	This will allow the regular decode loop to pick up as though
**	it's starting again with a 1D line.
*/
--dec_parm->srcbitptr;

return status;
} /* end of _IpsFindNextG31DEOL */
