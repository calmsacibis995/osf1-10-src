
/***************************************************************************** 
**
**  Copyright (c) Digital Equipment Corporation, 1990 All Rights Reserved.
**  Unpublished rights reserved under the copyright laws of the United States.
**  
**  The software contained on this media is proprietary to and embodies the 
**  confidential technology of Digital Equipment Corporation. Possession, use,
**  duplication or dissemination of the software and media is authorized only 
**  pursuant to a valid written license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the 
**  U.S. Government is subject to restrictions as set forth in 
**  Subparagraph (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19, as
**  applicable.
**
*****************************************************************************/

/************************************************************************
**
**  FACILITY:
**
**      Image Services Library
**
**  ABSTRACT:
**
**      Convert an image frame into a sixel buffer
**
**  ENVIRONMENT:
**
**      VAX/VMS
**
**  AUTHOR(S):
**
**      Ken MacDonald, Digital Equipment Corp.
**
**  CREATION DATE:
**
**      August, 1986
**
**  MODIFICATION HISTORY:
**
**	Ken MacDonald, Oct. 86 - Converted to ISL calling standards
**
************************************************************************/


/*
**  Include files:
*/

/*  ISL IMGLIB modules                                                       */

#include        <img/ChfDef.h>
#include        <ImgMacros.h>           /* macros                            */
#include	<img/ImgDef.h>		/* Image Frame Attribute constants   */
#include	<ImgDefP.h>		/* Image Frame Attribute constants   */
#ifndef NODAS_PROTO
#include <imgprot.h>	    /* Img prototypes */
#endif

/*
**  Table of contents:
*/
#if defined(__VMS) || defined(VMS)
struct FCT *IMG$EXPORT_SIXELS();      /* image frame to sixel buffer routine */
#endif
#ifdef NODAS_PROTO
struct FCT *ImgExportSixels();        /* DECwindows style entry point        */
#endif

/* 
** module local routines
*/
#ifdef NODAS_PROTO
static int img_repeat_sequence();
#else
static int img_repeat_sequence(int /*repcnt*/, int /*value*/, unsigned char */*strptr*/);
#endif



/*
**  MACRO definitions:
*/

/*
**  Equated Symbols:
*/

#define ZERO_LEN   0
#define ZERO_ADR   0
#define ZERO_FILL  0
#define MAX_CODEWORD_SIZE 6
#define LUT_SIZE 256

/*
**  External References:
*/

#ifdef NODAS_PROTO
void ChfSignal();                       /* error signal routine             */
void ChfStop();                         /* signal and stop                  */

struct FCT	*ImgGetFrameAttributes();

unsigned char	*_ImgCalloc();		/* allocate & clear dyn memory	    */
void		 _ImgCfree();		/* free dyn memory from _ImgCalloc */
void		 _ImgGet();
void		 _ImgSetRoi();

void		_IpsMovc5Long();	/* move characters with fill        */
void		_IpsMovtcLong();        /* move translated characters       */
void		_IpsMovv5();            /* move bits routine                */
int		_IpsBuildChangelist();  /* build scanline changelist        */
#endif

/* 
** Global references to lookup tables
*/
#if defined(__VAXC) || defined(VAXC)
globalref unsigned char	IMG_AB_BIN_RVSD[LUT_SIZE]; /* bit reversed lut       */
#else
extern unsigned char	IMG_AB_BIN_RVSD[LUT_SIZE]; /* bit reversed lut       */
#endif

/*
**	Status codes
*/
#include <img/ImgStatusCodes.h>         /* ISL Status Codes             */

/*
**  Local Storage:
*/

/************************************************************************
**
**  FUNCTIONAL DESCRIPTION:
**
**      Convert frame to sixel buffer routine
**
**  FORMAL PARAMETERS:
**
**      fid	    - pointer to the frame to be decompressed
**
**	out_buf_ptr - pointer to output buffer to be used. If not given,
**		      a suitable buffer will be internally allocated.
**
**      out_buf_len - length of buffer pointed to by buf_ptr; or if buf_ptr
**		      not specified this should be the size of the internally
**		      allocated buffer.
**
**      BYT_CNT	    - returns the number of bytes decoded by the routine
**
**      FLAGS	    - flags longword to define special processing
**
**	ACTION	    - pointer to an action routine to call if the output buffer
**		      is exhausted. Must be specified if internally allocated 
**		      buffers are used.
**
**	USERPARM    - application private value passed through into 
**		      action routine
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
**      returns input fid value
**
**  SIGNAL CODES:
**
**	ImgX_INVARGCNT - invalid argument count
**	ImgX_ACTNOTDEF - action routine not defined when bufadr is null
**      ImgX_BUFOVRFLW - buffer overflow detected
**	ImgX_INVLINPRG - invalid line prog attrib. (not verticle)
**	ImgX_INVPXLPTH - invalid pixel path attrib (not horizontal)
**      ImgX_UNSASPRAT - aspect ratio unsupported
**      ImgX_UNSCMPTYP - data compression unsupported
**      ImgX_UNSGRDTYP - grid type unsupported
**      ImgX_UNSPXLSTR - pixel stride unsupported
**	ImgX_UNSSPCTYP - unsupported spectral type (not bitonal)
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
/*    pixel stride must be 1                                               */
/*    only bitonal images are supported                                    */
/*    multiple data planes, content elements, etc. not handled             */
/*    decompression not performed                                          */
/*    grid type other than rectangular not supported                       */
/*    pixel path must be 0 or 180 degrees                                  */
/*    line progression must be 90 or 270 degrees                           */
/*    lookup tables not checked                                            */
/*    non 1:1 aspect ratio not supported                                   */
/*    standard codes for parameter values not defined yet                  */
/*                                                                         */
/***************************************************************************/

#if defined(__VMS) || defined(VMS)
struct FCT *IMG$EXPORT_SIXELS(fid,roi_id,out_buf_ptr,out_buf_len,byt_cnt,
        flags,action,userparm)
    struct FCT    *fid;                     /* frame id                       */
    struct ROI    *roi_id;		   /* roi id			     */
    unsigned char *out_buf_ptr;             /* output buffer                  */
    unsigned int  out_buf_len;              /* size of out buffer             */

    unsigned int  *byt_cnt;                 /* returned byte count (opt)      */
    unsigned int  flags;                    /* processing flags (opt)         */
    long           (*action)();              /* buffer full action routine(opt)*/
    long           userparm;                 /* user parameters                */
    {
    return(ImgExportSixels(fid,roi_id,out_buf_ptr,out_buf_len,byt_cnt,
        flags,action,userparm));
    }
#endif

/* DECwindows style entry point                                              */
struct FCT *ImgExportSixels(fid,roi_id,out_buf_ptr,out_buf_len,byt_cnt,
        flags,action,userparm)
    struct FCT    *fid;                     /* frame id                       */
    struct ROI    *roi_id;		   /* roi id			     */
    unsigned char *out_buf_ptr;             /* output buffer                  */
    unsigned int  out_buf_len;              /* size of out buffer             */

    unsigned int  *byt_cnt;                 /* returned byte count (opt)      */
    unsigned int  flags;                    /* processing flags (opt)         */
    long           (*action)();              /* buffer full action routine(opt)*/
    long           userparm;                 /* user parameters                */

    {
    int           srcbitptr;               /* bit pointer to input buffer     */
    unsigned char *xlate_buffer;           /* buffer for use in translation   */
    int           buf_len;                 /* length of sixel buffer          */
    unsigned char *sixel_temp;             /* ptr to temp storage for sixels  */
    unsigned char *sixel_out_ptr;          /* ptr to output sixel array       */
    unsigned int  dummy_byt_cnt;           /* local var. if byt_cnt not passed*/
    unsigned char *sixel_byte;             /* base pointer to area to sixelize*/
    int           sixel_bit;               /* bit offset   ""  ""  ""   ""    */
    int           num_lines;               /* number scans to sixelize        */
    unsigned char *buf_ptr;
    int           i,n,m,k;                 /* counters, etc.                  */
    int           npix,nscans;             /* num. of pixels and scanlines    */
    int           bytes_per_scan;          /* whole bytes / scan              */
    int lines_left;
    int repeat_cnt;
    unsigned char *rpt_char_ptr,*new_char_ptr;
    unsigned char *lut_ptr;                      /* lookup table to be used   */

    unsigned char *byte_ptr;                    /* ptr to current byte        */
    unsigned char *near_last_byte;              /* near last byte in buffer   */
    long          *ccl;                         /* pointer to changelist      */
    int           cl_len;                       /* changelist length          */

    long           action_status;                /* action routine return      */

#if defined(__VAXC) || defined(VAXC)
    readonly static unsigned char sixel_mask[] = 
        {1,2,4,8,16,32};
    readonly static unsigned char byte_mask[] = 
        {1,2,4,8,16,32,64,128};
#else
    static unsigned char sixel_mask[] = 
        {1,2,4,8,16,32};
    static unsigned char byte_mask[] = 
        {1,2,4,8,16,32,64,128};
#endif


    /* define return areas for IMG$GET_FRAME_ATTRIBUTES                       */

    int           spect_type;
    int           grid_type;
    int           pxl_path_dist;
    int           line_prog_dist;
    int           pxl_path,line_prog;
    int           cmprs_type;
    int           pxl_order;
    int           brt_polarity;
 
    unsigned char initial_sixel;
    struct UDP	  udp;
    int           row = 0;
    int           zero = 0;

    /* define masks for looking at each pixel                                 */

    unsigned char six_mask;

    struct GET_ITMLST itmlst[15];

    if (byt_cnt == 0)                           /* byt_cnt not specified      */
       byt_cnt = &dummy_byt_cnt;                /* give it dummy internal addr*/

    /*
    ** The user should have passed a zero or a pointer to a valid ROI.
    ** In either case, set the udp dimensions to fit the ROI.
    */
    _ImgGet(fid, Img_Udp, &udp, sizeof(struct UDP), 0, 0);
    if (roi_id != 0)
        _ImgSetRoi(&udp,roi_id);

    /* initialize the GET_ITMLST                                             */

    row = 0;
    INIT_GET_(itmlst,row,Img_ImageDataClass, sizeof(int), &spect_type, 0,0);
    INIT_GET_(itmlst,row,Img_GridType, sizeof(int), &grid_type, 0,0);
    INIT_GET_(itmlst,row,Img_PPPixelDist, sizeof(int), &pxl_path_dist, 0,0);
    INIT_GET_(itmlst,row,Img_LPPixelDist, sizeof(int), &line_prog_dist, 0,0);
    INIT_GET_(itmlst,row,Img_PixelPath, sizeof(int), &pxl_path, 0,0);
    INIT_GET_(itmlst,row,Img_LineProgression, sizeof(int), &line_prog, 0,0);
    INIT_GET_(itmlst,row,Img_CompressionType, sizeof(int), &cmprs_type, 0,0);
    INIT_GET_(itmlst,row,Img_BitOrder, sizeof(int), &pxl_order, 0,0);
    INIT_GET_(itmlst,row,Img_BrtPolarity, sizeof(int), &brt_polarity, 0,0);
    END_GET_(itmlst,row);

    /* retrieve useful data items from the data structures                    */
    ImgGetFrameAttributes(fid,itmlst);
   
    /* check for parameters not handled in this version                       */

    if((pxl_path != ImgK_RightToLeft) && 
        (pxl_path != ImgK_LeftToRight))              /* bad pixel path      */
            ChfSignal( 1, ImgX_INVPXLPTH);

    if((line_prog != ImgK_TopToBottom) && 
        (line_prog != ImgK_BottomToTop))             /* bad line progression*/
            ChfSignal( 1, ImgX_INVLINPRG);

    if(spect_type != ImgK_ClassBitonal)              /* non-bitonal         */
        ChfSignal( 1, ImgX_UNSSPCTYP);

    if (pxl_path_dist != line_prog_dist)               /* not 1:1 aspect ratio*/
        ChfSignal( 1, ImgX_UNSASPRAT);

    if(grid_type != ImgK_RectangularGrid)            /* non-rectangular grid*/
        ChfSignal( 1, ImgX_UNSGRDTYP);

    if(cmprs_type != ImgK_PcmCompression)            /* image is compressed */
        ChfSignal( 1, ImgX_UNSCMPTYP);

    if(udp.UdpL_PxlStride != 1)                              /* pixel stride not 1  */
        ChfSignal( 1, ImgX_UNSPXLSTR);

    if(out_buf_ptr == 0 && action == 0)                /* if no user buffer,  */
        ChfStop( 1, ImgX_ACTNOTDEF);                       /* action routine req'd*/

    /* assign number lines, number scans, bits/pixel to local variables       */

    nscans = udp.UdpL_Y2 - udp.UdpL_Y1 + 1;
    lines_left = nscans;
    npix = udp.UdpL_X2 - udp.UdpL_X1 + 1;
    bytes_per_scan = (npix + 7) / 8;
    if (pxl_order == ImgK_ReversePixelOrder)   /* assign correct lookup tbl*/
        lut_ptr = IMG_AB_BIN_RVSD;

    /* set the initial sixel value to either all one's or all zero's based    */
    /* on the brightness polarity                                             */

    if (brt_polarity == ImgK_ZeroMaxIntensity)
        initial_sixel = 0x40;               /* initially 01000000B            */
    else
        initial_sixel = 0x7F;               /* initially 01111111B            */

    /* allocate one temporary line of sixels for work area                    */

    *byt_cnt = 0;
    sixel_temp = (unsigned char *)_ImgCalloc( sizeof(char) ,npix+1 );

    /* allocate one scan worth of buffer in case byte alignment is needed     */

    xlate_buffer = (unsigned char *)_ImgCalloc( sizeof(char) ,bytes_per_scan);

    /* allocate a changelist 4 entries bigger than a scanline                 */

    cl_len = npix + 4;
    ccl = (long *)_ImgCalloc( sizeof(long), cl_len );

    /* allocate output area if buf_ptr not supplied. Length will be buf_len   */

    buf_len = out_buf_len;
    if (out_buf_ptr == 0)
        {
        if (buf_len == 0)              /* allocate 1/4 size of the image       */
            buf_len = (udp.UdpL_X2 - udp.UdpL_X1 + 1) * 
                (udp.UdpL_Y2 - udp.UdpL_Y1 + 1) / 32;
        buf_ptr = (unsigned char *)(unsigned char *)_ImgCalloc(sizeof(char), buf_len );
        }
    else
        buf_ptr = out_buf_ptr;

    near_last_byte = buf_ptr + buf_len - 1 - MAX_CODEWORD_SIZE;

    /* start pointer out one scanline BEFORE the buffer, since S2 is added on */
    /* at the head of the loop                                                */

    srcbitptr = udp.UdpL_Pos - udp.UdpL_ScnStride;
    sixel_out_ptr = buf_ptr;

    while (lines_left > 0)
        {
        /* initialize the temp buffer to either ones or zeros (initial_sixel)  */

        _IpsMovc5Long(ZERO_LEN,(unsigned char *)&zero,initial_sixel,npix,sixel_temp);
        num_lines = lines_left > 6 ? 6 : lines_left;
        for (n = 0; n < num_lines; n++)
            {
            srcbitptr += udp.UdpL_ScnStride;
            if (pxl_order == ImgK_ReversePixelOrder)
                {
                sixel_byte = xlate_buffer;
                sixel_bit = 0;
                if (srcbitptr & 7 != 0)            /* byte align before translate*/
                    {
                    _IpsMovv5(npix,srcbitptr,udp.UdpA_Base,0,xlate_buffer);
                    _IpsMovtcLong(bytes_per_scan,xlate_buffer,ZERO_FILL,lut_ptr,
                        bytes_per_scan,xlate_buffer);
                     }
                else                                /* else just translate       */
                    _IpsMovtcLong(bytes_per_scan,udp.UdpA_Base+(srcbitptr >> 3),
                        ZERO_FILL,lut_ptr,bytes_per_scan,xlate_buffer);
                }
            else
                {
                sixel_byte = (unsigned char *) udp.UdpA_Base;
                sixel_bit  = srcbitptr;
                }

            /* convert line to changelist, then OR in the proper bits           */
            /* with the appropriate sixel_mask into the sixel temporary line    */

            _IpsBuildChangelist(sixel_byte,sixel_bit,npix,ccl,cl_len);
            ccl[ccl[0]+1] = npix;
            six_mask = sixel_mask[n];
            for (m = 1; m < ccl[0]; m += 2)
                for (k = ccl[m]; k < ccl[m+1]; k++)
                    sixel_temp[k] ^= six_mask;
            lines_left--;
            } /* end of for loop */

	/*
	** If we have unused pixels in the last line of sixels AND the
	** sixel initializer sets all sixels to black by default, clear
	** the unset rows ...
	**
	**	Clear the rows by clearing the unused bits in
	**	each sixel byte.   Use a lookup table which
	**	maps the number used lines into a mask which
	**	will clear the unused lines.
	*/
	if ( ((num_lines % 6) != 0) && (initial_sixel == 0x7F) )
	    {
	    static long	clear_bits[6] = {    0x00
					    ,0x3E	/* 0011 1110	*/
					    ,0x3C	/* 0011 1100	*/
					    ,0x38	/* 0011 1000	*/
					    ,0x30	/* 0011 0000	*/
					    ,0x20 };	/* 0010 0000	*/
	    long    idx;
	    long    used_lines;

	    used_lines = num_lines % 6;
	    for ( idx = 0; idx < npix; ++idx )
		{
		sixel_temp[idx] ^= clear_bits[used_lines];
		}
	    }

        /* six scan lines have now been converted into sixels. Compress them   */
        /* into sixel output area                                              */

        rpt_char_ptr = sixel_temp;           /* set to beginning of buffer     */
        new_char_ptr = sixel_temp;
        while (new_char_ptr < (npix + sixel_temp))
            {
            while (*rpt_char_ptr == *++new_char_ptr);
         
            repeat_cnt = new_char_ptr - rpt_char_ptr;

            /* encode runlength if > 3; else stringcopy to output area          */

            if (repeat_cnt > 3)
                {
                sixel_out_ptr += 
                    img_repeat_sequence(repeat_cnt,--*rpt_char_ptr,sixel_out_ptr);
                }
            else
                {
                --*rpt_char_ptr;            /* subtract 1 to get real sixel value*/
                for (n=0; n < repeat_cnt; n++)
                    *sixel_out_ptr++ = *rpt_char_ptr;
                } 

            /* invoke action routine if closer than 1 codeword to end of buffer */

            if (sixel_out_ptr > near_last_byte)
                {
                if (action != 0)
                    {
                    *byt_cnt += sixel_out_ptr - buf_ptr;
                    action_status = (*action)(buf_ptr,sixel_out_ptr-buf_ptr,userparm);
                    if ((action_status & 1) != 1)
                        ChfSignal( 1, action_status);
                    sixel_out_ptr = buf_ptr;
                    }
                else
                    ChfStop( 1, ImgX_BUFOVRFLW);
                }

            /* and continue as if nothing had happened                          */

            rpt_char_ptr = new_char_ptr;
            }

        /* six lines converted to sixels. add the sixel graphics CR/LF         */

        *sixel_out_ptr++ = '-';
        }                                 /* end outer while                   */

    if (action != 0)                     /* output last batch of sixels       */
        {
        action_status = (*action)(buf_ptr,sixel_out_ptr-buf_ptr,userparm);
        if ((action_status & 1) != 1)
            ChfSignal( 1, action_status);
	}

    *byt_cnt += sixel_out_ptr - buf_ptr;

    _ImgCfree(sixel_temp);		/* free internal work areas          */
    _ImgCfree(xlate_buffer);
    _ImgCfree(ccl);
    if (out_buf_ptr == 0)                /* if we allocated internal area     */
        _ImgCfree(buf_ptr);

    return (fid);                    /* all detected errors are signalled */
    }

/****************************************************************************
**
** internal routine to parse the sixel repeat count and stuff the result
** onto the end of the string. This simulates an "sprintf("!%d",repcnt);"
**
****************************************************************************/
static int img_repeat_sequence(repcnt,value,strptr)
    int repcnt;
    int value;
    unsigned char *strptr;
    {
    int magnitude;
    int temp = repcnt;
    unsigned char *ptr = strptr;

    for (magnitude = 1; magnitude <= repcnt; magnitude *= 10);
    
    *ptr++ = '!';

    for (magnitude /= 10; magnitude >= 1; magnitude /= 10)
	{
        *ptr++ = '0' + (temp / magnitude);
        temp = temp % magnitude;
	}

    *ptr++ = value;

    return(ptr-strptr);	    
    }
