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
**  _IpsDecodeUtil
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
**      Karen D'Intino, Digital Equipment Corp.
**
**  CREATION DATE:
**
**      December 6, 1991
**
************************************************************************/

/*
**  Table of contents:
*/
long IpsCorrectScanlineCount();  /* utility entry point to return the scanline ct */
long _IpsDecodeG31dUtils();  /* decode G31D image frame to buffer routine */
long _IpsDecodeG32dUtils();  /* decode G32D image frame to buffer routine */
long _IpsDecodeG42dUtils();  /* decode G42D image frame to buffer routine */

/*
**  Include files:
*/
#include <img/ImgDef.h>                         /* Image definitions     */
#include <img/ImgDefP.h>                        /* Image Private definitions    */

#include        <IpsDef.h>              /* image frame constants             */
#include        <IpsStatusCodes.h>      /* IPS Status Codes                  */
#include        <IpsMemoryTable.h>      /* IPS Memory Mgt. Functions         */
#include        <ips_fax_macros.h>      /* fax macros like $GET_12_BITS      */
#include        <ips_fax_definitions.h> /* FAX definitions                   */
#include        <ips_fax_paramdef.h>    /* FAX routine parameter block defns.*/

/* Image Services Library routines                                           */
long _IpsDecodeG42dScan();              /*  routine decodes 1 scanline       */
long _IpsDecodeG31dScan();              /*  routine decodes 1 scanline       */

/*
**  External references
*/
long _IpsVerifyNotInPlace();		/*  from IPS__UDP_UTILS.C            */


long IpsCorrectScanlineCount(src_udp, compression_type, scanline_count)
struct  UDP *src_udp;
long compression_type;
long *scanline_count;
    {
    long status;
    unsigned long   bitonal_flags   = 0;

    if (compression_type == ImgK_PcmCompression)
        return (TRUE);
    /*
    ** Set the class field in the udp
    */
    src_udp->UdpB_Class = UdpK_ClassUBA;
    switch(compression_type)
        {
        case ImgK_G31dCompression:
            status =_IpsDecodeG31dUtils
                        (src_udp, bitonal_flags, scanline_count);
            break;
        case ImgK_G32dCompression:
            status = _IpsDecodeG32dUtils
                  (src_udp, bitonal_flags, scanline_count);
	    break;
        case ImgK_G42dCompression:
            status = _IpsDecodeG42dUtils
                    (src_udp, bitonal_flags,scanline_count);
            break;

        default: /* unknown or unsupported type requested     */
            break;
        }/* END switch */

    return(status);
    }

/************************************************************************
**
**  _IpsDecodeG31dUtils
**
**  FUNCTIONAL DESCRIPTION:
**
**      Decodes an image frame which has been compressed using the CCITT
**	Group 3-1D standard compression technique.
**
**  FORMAL PARAMETERS:
**
**      src_udp --> Pointer to source udp (initialized)
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

long _IpsDecodeG31dUtils(src_udp,flag, scanline_count)
struct UDP *src_udp;
unsigned long flag;
long *scanline_count;
    {
    struct FAX_DECODE_PARAMS dec_parm;      /* parm block for DECODE_xxxxSCAN */
    long           bufbitptr=0;             /* ptrs to source/output bits     */
    unsigned long  buf_size_bits;           /* # bits in output buffer        */
    long           eol_count = 0;           /* end of line count              */
    long	   should_be_eol = 0;	    /* 1st 12 bits of buf must be EOL */
    long	   size;		    /* buffer size		      */
    long	   status;                
    long	   i;
    unsigned long  result_flag = IpsX_SUCCESS;
    long	   scanline_index;
    if(src_udp->UdpL_PxlStride != 1)            
        return (IpsX_UNSPXLSTR);
    if ((src_udp->UdpL_X1 != 0) || (src_udp->UdpL_Y1 != 0))
        return (IpsX_INCNSARG);

    size = (src_udp->UdpL_PxlPerScn) *  src_udp->UdpL_ScnCnt;

    if (flag != IpsK_DTypeIntBits)
        size += ((8 - (size % 8))) % 8;

    /* alloc current changelist: length is (scanline length + 4) longwords    */
    dec_parm.pixels_per_line = src_udp->UdpL_PxlPerScn;/* scanline_size (bits)*/
    dec_parm.ccl = (long *) (*IpsA_MemoryTable[IpsK_Alloc])
	((src_udp->UdpL_PxlPerScn + 4 * sizeof(long)), IpsM_InitMem,0);

    /* buffer size in bits         */
    buf_size_bits = size;
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
	if (bufbitptr + src_udp->UdpL_ScnStride > buf_size_bits)
	    return(IpsX_BUFOVRFLW);
 
	/* get one scanline. changes stored in dec_parm.ccl[]                 */
	status = _IpsDecodeG31dScan(&dec_parm);
    
	if (status != IpsX_SUCCESS)
	    {
	    result_flag = IpsX_INVSCNLEN;
	    /* 
	    ** reject offsets > pxl per scn in the changelist and
	    ** adjust accordingly
	    */
	    i = dec_parm.ccl[0];
	    while (dec_parm.ccl[i] > src_udp->UdpL_PxlPerScn)
		i--;
	    dec_parm.ccl[0] = i;
	    dec_parm.ccl[i + 1] = src_udp->UdpL_PxlPerScn;
	    dec_parm.ccl[i + 2] = src_udp->UdpL_PxlPerScn;
	    dec_parm.ccl[i + 3] = src_udp->UdpL_PxlPerScn;
	    }


	/* check for empty lines (ccl[0] == 0); if not empty increment        */
	/* buffer ptr by the scanline stride                                  */
	if (dec_parm.ccl[0] == 0) 
	    eol_count++;
	else
	    {
	    scanline_index++;
	    bufbitptr += src_udp->UdpL_ScnStride;
	    }
	} /* end outer while */

    (*IpsA_MemoryTable[IpsK_Dealloc])(dec_parm.ccl);
    *scanline_count = scanline_index;
    return (IpsX_SUCCESS);
    } /* End _IpsDecodeG31d */

/************************************************************************
**
** _IpsDecodeG32dUtils
**
**  FUNCTIONAL DESCRIPTION:
**
**      Decodes an image frame which has been compressed using the CCITT
**	Group 3/2D standard compression technique.
**
**  FORMAL PARAMETERS:
**
**      src_udp --> Pointer to source udp (initialized)
**	flag --> Used to indicate if destination is padded to byte.
**	scanline_count -> the true scanline count in the compressed image.
**
**  IMPLICIT INPUTS:  none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE:   returns status 
**  SIDE EFFECTS:     none
**  SIGNAL CODES:     none
**
************************************************************************/

long _IpsDecodeG32dUtils(src_udp,flag,scanline_count)
struct UDP *src_udp;
unsigned long flag;
long *scanline_count;
    {
    long size;				    /* buffer size		      */
    struct FAX_DECODE_PARAMS dec_parm;      /* parm block for DECODE_xxxxSCAN */
    long           bufbitptr=0;		    /* ptrs to source/output bits     */
    unsigned long  buf_size_bits;           /* # bits in output buffer        */
    long           *temp_ptr;               /* temp. list pointer             */
    long           eol_count = 0;           /* end of line count              */
    long	   status;
    long	   scanline_index;
    /*
    ** Initialize destination UDP fixed fields & data buffer.
    */
    if(src_udp->UdpL_PxlStride != 1)            
        return (IpsX_UNSPXLSTR);

    if ((src_udp->UdpL_X1 != 0) || (src_udp->UdpL_Y1 != 0))
        return (IpsX_INCNSARG);

    size = (src_udp->UdpL_PxlPerScn) *  src_udp->UdpL_ScnCnt;

    if (flag != IpsK_DTypeIntBits)
        size += ((8 - (size % 8))) % 8;

    /* alloc curr/ref changelists: length is (scanline length + 4) longwords  */
    dec_parm.pixels_per_line = src_udp->UdpL_PxlPerScn;/* scanline_size (bits)*/
    dec_parm.base = src_udp->UdpA_Base;
    dec_parm.ccl = (long *) (*IpsA_MemoryTable[IpsK_Alloc])
	((sizeof(long) * dec_parm.pixels_per_line + 4), IpsM_InitMem, 0);
    dec_parm.rcl = (long *)(*IpsA_MemoryTable[IpsK_Alloc])
	((sizeof(long) * dec_parm.pixels_per_line + 4),IpsM_InitMem,0);
    /* buffer size in bits         */
    buf_size_bits = size;

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
 
        if (bufbitptr + src_udp->UdpL_ScnStride > buf_size_bits)
            return(IpsX_BUFOVRFLW);

        /* get one scanline. changes stored in dec_parm.ccl[]                 */

        if ((GET_1_BIT_(dec_parm.base,dec_parm.srcbitptr)) == TAG_K_1D)
            {
            dec_parm.srcbitptr++;
            /* EOL is skipped by IpsDecodeG31dScan */
            _IpsDecodeG31dScan(&dec_parm);
            }
        else
            {
            dec_parm.srcbitptr++;
            _IpsDecodeG42dScan(&dec_parm);
            /*
            **  cycle through any number of fill bits that may be optionally
            **  present at the end of a g4-2d encoded line (but ONLY in
            **  g3-2d images!). This 'while' will stop when it gets to a 
            **  bit, which will then be the EOL.
            **  Note that the extra set of parentheses in the while is needed
            **  so that the GET_12 macro is correctly evaluated...
            */
            while ((GET_12_BITS_(dec_parm.base,dec_parm.srcbitptr)) == 0)
                    dec_parm.srcbitptr++;
            dec_parm.srcbitptr += IPS_K_FAX1D_CODELEN_EOL; /* skip EOL        */
            }
    
        temp_ptr = dec_parm.rcl;               /* swap curr/reference line ptr*/
        dec_parm.rcl = dec_parm.ccl;
        dec_parm.ccl = temp_ptr;
    
        /* 
	** check for EOL codes on empty lines else inc return ptr by the stride
	*/
        if (dec_parm.rcl[0] == 0) 
            eol_count++;
        else
            {
            scanline_index++;
            bufbitptr += src_udp->UdpL_ScnStride;
            }
        } /* end outer while */
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

        /* 
	** check for being less than one scan away from the end of the buffer 
        ** if so, we'll call the action routine. If not specified, signal     
        ** a buffer overflow                                                  
	*/
        if (bufbitptr + src_udp->UdpL_ScnStride > buf_size_bits)
            return(IpsX_BUFOVRFLW);
    
        /* get one scanline. changes stored in dec_parm.ccl[]                 */
    
        if ((GET_1_BIT_(dec_parm.base,dec_parm.srcbitptr)) == TAG_K_1D)
            {
            dec_parm.srcbitptr++;
            /* EOL is skipped by IpsDecodeG31dScan                            */
            _IpsDecodeG31dScan(&dec_parm);
            }
        else
            {
            dec_parm.srcbitptr++;
            _IpsDecodeG42dScan(&dec_parm);
            /*
            **  cycle through any number of fill bits that may be optionally
            **  present at the end of a g4-2d encoded line (but ONLY in
            **  g3-2d images!). This 'while' will stop when it gets to a 
            **  bit, which will then be the EOL.
            **  Note that the extra set of parentheses in the while is needed
            **  so that the GET_12 macro is correctly evaluated...
            */
            while ((GET_12_BITS_(dec_parm.base,dec_parm.srcbitptr)) == 0)
                dec_parm.srcbitptr++;
            dec_parm.srcbitptr += IPS_K_FAX1D_CODELEN_EOL; /* skip EOL        */
            }
    
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
            bufbitptr += src_udp->UdpL_ScnStride;
            }
        } /* end outer while */

    (*IpsA_MemoryTable[IpsK_Dealloc])(dec_parm.ccl);
    (*IpsA_MemoryTable[IpsK_Dealloc])(dec_parm.rcl);
    *scanline_count = scanline_index;
    return (IpsX_SUCCESS);
    }/* end _IpsDecodeG32d */

/************************************************************************
**
**  _IpsDecodeG42dUtils
**
**  FUNCTIONAL DESCRIPTION:
**
**      Decodes an image frame which has been compressed using the CCITT
**	Group 4 standard compression technique.
**
**  FORMAL PARAMETERS:
**      src_udp --> Pointer to source udp (initialized)
**	flag --> Used to indicate if destination is padded to byte.
**
**  IMPLICIT INPUTS:  none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE:   returns status
**  SIDE EFFECTS:     none
**  SIGNAL CODES:     none
**
************************************************************************/

long _IpsDecodeG42dUtils(src_udp, flag, scanline_count)
struct UDP *src_udp;
unsigned long flag;
long *scanline_count;
    {
    struct FAX_DECODE_PARAMS dec_parm;      /* parm block for DECODE_xxxxSCAN */
    long	       size;		        /* buffer size		          */
    long           bufbitptr=0;             /* ptrs to source/output bits     */
    unsigned long  buf_size_bits;           /* # bits in output buffer        */
    long           *temp_ptr;               /* temp. change list pointer      */
    long           eol_count = 0;           /* end of line count              */
    long	   status;
    long	   scanline_index;
    /*
    ** Initialize destination UDP fixed fields & data buffer.
    */
    if(src_udp->UdpL_PxlStride != 1)            
        return (IpsX_UNSPXLSTR);
    
    if ((src_udp->UdpL_X1 != 0) || (src_udp->UdpL_Y1 != 0))
        return (IpsX_INCNSARG);

    size = (src_udp->UdpL_PxlPerScn) *  src_udp->UdpL_ScnCnt;

    if (flag != IpsK_DTypeIntBits)
        size += ((8 - (size % 8))) % 8;


    /* alloc curr/ref changelists: length is (scanline length + 4) longwords  */
    dec_parm.pixels_per_line = src_udp->UdpL_PxlPerScn;/* scanline_size (bits)*/
    dec_parm.ccl = (long *) (*IpsA_MemoryTable[IpsK_Alloc])
	((sizeof(long)*dec_parm.pixels_per_line + 4),IpsM_InitMem,0);
    dec_parm.rcl = (long *) (*IpsA_MemoryTable[IpsK_Alloc])
	((sizeof(long)*dec_parm.pixels_per_line + 4),IpsM_InitMem,0);

    dec_parm.base = src_udp->UdpA_Base;
    /* buffer size in bits         */
    buf_size_bits = size;

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
        if (bufbitptr + src_udp->UdpL_ScnStride > buf_size_bits)
            return(IpsX_BUFOVRFLW);

        /* get one scanline. changes stored in dec_parm.ccl[]                 */
        _IpsDecodeG42dScan(&dec_parm);
    
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
            bufbitptr += src_udp->UdpL_ScnStride;
            }
        } /* end outer while */
    (*IpsA_MemoryTable[IpsK_Dealloc]) (dec_parm.ccl);
    (*IpsA_MemoryTable[IpsK_Dealloc]) (dec_parm.rcl);
    *scanline_count = scanline_index;
    return (IpsX_SUCCESS);
    } /* end of _IpsDecodeG42d */
