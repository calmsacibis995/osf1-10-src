
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
**      Create printable PostScript buffer for an image frame
**
**  ENVIRONMENT:
**
**      VAX/VMS
**
**  AUTHOR(S):
**
**      Ken MacDonald, Digital Equipment Corp.
**
**
**  CREATION DATE:
**
**      June, 1986
**
**  MODIFICATION HISTORY:
**
**      KJM - October 1986 - Convert to ISL calling standards
**      KJM - November 1988 - Add support for ASCII-hex encoding
**
************************************************************************/

/*
**  Include files:
*/
#include	<stdio.h>
#include	<string.h>

#include        <img/ChfDef.h>
#include	<img/ImgDef.h>
#include        <ImgDefP.h>	
#include        <ImgMacros.h>		 /* macros                          */
#ifndef NODAS_PROTO
#include <imgprot.h>	    /* Img prototypes */
#endif
#include        <math.h>                 /* math functions                  */

/*
**  Table of contents:
*/
#if defined(__VMS) || defined(VMS)
struct FCT *IMG$EXPORT_PS();     /* frame to PostScript routine      */
#endif
#ifdef NODAS_PROTO
struct FCT *ImgExportPS();       /* DECwindows style entry point     */
#endif

/* 
** module local processing routines
*/
#ifdef NODAS_PROTO
static void IMG_ADD_PS_BUFF();	/* add string to PS buffer          */
#else
#if (0)
/* can't prototype variable args */
PROTO(static void IMG_ADD_PS_BUFF, 
	(unsigned char **/*workbufptr*/, 
	unsigned char */*bufptr*/, 
	int */*byt_cnt*/, 
	unsigned char /*string*/[], 
	long (*/*action*/)(), 
	long /*userparm*/, 
	unsigned char */*last_byte*/, 
	int /*opt1*/, 
	int /*opt2*/, 
	int /*opt3*/, 
	int /*opt4*/));
#else
static void IMG_ADD_PS_BUFF();	/* add string to PS buffer          */
#endif
#endif


/*
**  MACRO definitions:
*/
#define CHECK_BUFFER_(size) \
    if (work_buf_ptr + size > last_byte)\
        {\
        if (action != 0)\
            {\
            *byt_cnt += work_buf_ptr - buf_ptr;\
            action_status = (*action)(buf_ptr,work_buf_ptr-buf_ptr,userparm);\
            if ((action_status & 1) != 1)\
                ChfSignal( 1, action_status);\
            work_buf_ptr = buf_ptr;\
            }\
        else\
            {\
            ChfStop( 1, ImgX_BUFOVRFLW);\
            }\
        }
   

/*
**  Equated Symbols:
*/
#define MAXOUTCHAR 120
#define MAXOUTSIZE MAXOUTCHAR+1
#define LUT_SIZE   256
#define ZERO_FILL  0
#define LINE_FEED  10

/*
**  External References:
*/

#ifdef NODAS_PROTO
void ChfSignal();                       /* error signal routine             */
void ChfStop();                         /* signal and stop                  */

struct FCT	*ImgGetFrameAttributes();
unsigned char	*_ImgCalloc();		/* allocate & clear dyn memory	    */
void		 _ImgCfree();		/* free dyn memory from _ImgCalloc  */
void		 _ImgGet();
void		 _ImgSetRoi();

void		 _IpsMovtcLong();	/* move translated char		    */
void		 _IpsMovv5();		/* move bits			    */
#endif

/* 
** Lookup tables used for various output formats
*/
#if defined(__VAXC) || defined(VAXC)
globalref unsigned char		IMG_AB_LW_BIN[],
				IMG_AB_LW_BIN_RVSD[];
globalref unsigned short int	IMG_AW_ASCII_HEX[],
				IMG_AW_ASCII_HEX_RVSD[];
#else
extern unsigned char		IMG_AB_LW_BIN[],
				IMG_AB_LW_BIN_RVSD[];
extern unsigned short int	IMG_AW_ASCII_HEX[],
				IMG_AW_ASCII_HEX_RVSD[];
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
**      Converts an image frame to printable PostScript and returns a buffer
**         containing the PostScript
**
**  FORMAL PARAMETERS:
**
**      fid	- pointer to the frame to be converted to PostScript
**
**	buf_ptr - pointer to output buffer to be used. If not given,
**	          a buffer of length buflen will be internally allocated.
**
**      buflen	- length of buffer pointed to by buf_ptr; or if buf_ptr
**                not specified this should be the size of the internally
**                allocated buffer.
**
**      BYT_CNT	- returns the number of bytes decoded by the routine
**
**      FLAGS	- longword of flags to define special processing
**
**	ACTION	- pointer to an action routine to call if the output buffer
**                is exhausted. Must be specified if internally allocated 
**	          buffers are used.
**
**	USERPARM    - private user parameter passed through to the user
**		      action routine.
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
**      returns the input frame identifier
**
**  SIGNAL CODES:
**
**	ImgX_INVARGCNT - invalid argument count
**	ImgX_ACTNOTDEF - action routine not specified when required
**	ImgX_BUFOVRFLW - buffer overflowed with no action routine specified
**	ImgX_INVBUFLEN - invalid buffer len
**	ImgX_INVLINPRG - invalid line progression attr (notvertical)
**	ImgX_INVPXLPTH - invalid pixel path progr. attr (not horizontal)
**      ImgX_UNSASPRAT - aspect ratio unsupported
**      ImgX_UNSCMPTYP - data compression unsupported
**      ImgX_UNSGRDTYP - grid type unsupported
**      ImgX_UNSPXLSTR - pixel stride unsupported
**      ImgX_UNSSPCTYP - unsupported spectral type - only bitonal allowed
**
**  SIDE EFFECTS:
**
**      none
**
***************************************************************************/

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
/*    image data is transformed into "special" binary for the LaserWriter  */
/*       (control characters are not allowed). All control characters in   */
/*       output are converted to harmless characters.                      */
/*       This format may be specified by a flag.                           */
/*                                                                         */
/***************************************************************************/

/***** The PostScript code actually generated by this module will be 
** similar to the following example. Variables actually inserted by the
** routine into the PostScript code are denoted by asterisks: e.g. *300*
** in this example; the asterisks are NOT part of the actual code.

%!PSAdobe-1.0 
%%DocumentFonts: 
%%Title: Image 
%%Creator: IMG$EXPORT_PS
%%CreationDate: ??????????? 
%%For: Image Services Library 
%%Pages: 1 
%%EndComments 

/inch {72 mul} def 
/pic 512 string def 
{1 exch sub} settransfer 

/afterimage { 
   } def 
/btimage 
{npix nscans nbits 
[npix 0 0 nscans neg 0 nscans] 
{currentfile pic readline pop} 

image grestore afterimage showpage} def 

%%EndProlog 

%%Page: 1 1 

save 
/npix *2560* def % pixels per line 

/nscans *3300* def % number of scans in image 

/nbits *1* def % number bits/pix 

gsave 
npix 300 div inch nscans 300 div inch scale 

btimage


******* binary image data is inserted at this point **********

restore 
%%Trailer 

******* End of Example PostScript output *********************
**/

#if defined(__VMS) || defined(VMS)
struct FCT *IMG$EXPORT_PS(fid,roi_id,out_buf_ptr,out_buflen,
    byt_cnt,flags,action,userparm)
    struct FCT    *fid;                 /* frame context block pointer        */
    struct ROI    *roi_id;	       /* region of interest identifier	     */
    unsigned char *out_buf_ptr;	       /* Output buffer address              */
    int           out_buflen;	       /* Output buffer length               */

    int           *byt_cnt;	       /* Optional byte count                */
    int           flags;	       /* Optional user flags                */
    long           (*action)();	       /* Optional user action routine       */
    long           userparm;           /* optional user parameters           */
    {

    return (ImgExportPS(fid,roi_id,out_buf_ptr,out_buflen,
        byt_cnt,flags,action,userparm));
    }
#endif

/* DECwindows entry point definition                                         */
struct FCT *ImgExportPS(fid,roi_id,out_buf_ptr,out_buflen,
    byt_cnt,flags,action,userparm)
    struct FCT    *fid;                /* frame context block pointer        */
    struct ROI    *roi_id;	       /* region of interest identifier	     */
    unsigned char *out_buf_ptr;	       /* Output buffer address              */
    int           out_buflen;	       /* Output buffer length               */

    int           *byt_cnt;	       /* Optional byte count                */
    int           flags;	       /* Optional user flags                */
    long           (*action)();	       /* Optional user action routine       */
    long           userparm;           /* optional user parameters           */
    {
    unsigned char *lut;                /* pointer to correct lookup table    */
                                       /* for binary output                  */
    unsigned short int *hex_lut;       /* pointer to ASCII-hex lookup tables */

    int           i,n;                 /* counters, etc.                     */
    int           last;                /* last byte in buffer                */
    int           npix,nscans;         /* # pixels, # scans                  */
    int           bufcount,datacount,tran_len;/* counters                     */
    unsigned char *last_byte,*work_buf_ptr;
    int           buflen;              /* internal var. for buffer length    */
    unsigned char *buf_ptr;            /* internal buffer pointer            */
    int           dummy_byt_cnt;       /* used if byt_cnt not specified      */
    int           srcbitptr;           /* pointer to current bit position    */
    int           bytes_left_in_scan;  /* bytes in current scan              */
    unsigned char *temp_scan_buffer;   /* temporary storage for 1 scan       */
    unsigned char *scan_bufptr;        /* pointer to current scan position   */

    long           action_status;       /* ret. value from user action rtn.   */

    /* define return areas for IMG$GET_FRAME_ATTRIBUTES                       */
    struct UDP    udp;
    int           spect_type;
    int           grid_type;
    int           pp_pixel_dist;
    int           lp_pixel_dist;
    int           compression_type;
    int           bits_per_pixel;
    int           pixel_order;
    int           pixel_path,line_progression;
    int           brt_polarity;

    int           boundbox_urx,boundbox_ury;      /* BoundingBox defns.      */

    struct GET_ITMLST itmlst[11];
    int index = 0;


    INIT_GET_(itmlst,index,Img_ImageDataClass,sizeof(int),&spect_type,0,0);
    INIT_GET_(itmlst,index,Img_GridType,sizeof(int),&grid_type,0,0);
    INIT_GET_(itmlst,index,Img_PPPixelDist,sizeof(int),&pp_pixel_dist,0,0);
    INIT_GET_(itmlst,index,Img_LPPixelDist, sizeof(int),&lp_pixel_dist,0,0);
    INIT_GET_(itmlst,index,Img_CompressionType,sizeof(int),&compression_type,0,0);
    INIT_GET_(itmlst,index,Img_LineProgression,sizeof(int),&line_progression,0,0);
    INIT_GET_(itmlst,index,Img_PixelPath,sizeof(int),&pixel_path,0,0);
    INIT_GET_(itmlst,index,Img_BitsPerPixel,sizeof(int),&bits_per_pixel,0,0);
    INIT_GET_(itmlst,index,Img_BitOrder,sizeof(int),&pixel_order,0,0);
    INIT_GET_(itmlst,index,Img_BrtPolarity,sizeof(int),&brt_polarity,0,0);
    END_GET_(itmlst,index);

    if (byt_cnt == 0)                           /* byt_cnt not specified      */
        byt_cnt = &dummy_byt_cnt;                /* give it dummy internal addr*/

    /*
    ** The user should have passed a zero or a pointer to a valid ROI.
    ** In either case, set the udp dimensions to fit the ROI then get
    ** the UDP values.  This is NOT put back in the frame because this
    ** is an export/translation service and should not change the source
    ** frame.
    */
    _ImgGet(fid, Img_Udp, &udp, sizeof(struct UDP), 0, 0);
    if (roi_id != 0)
        _ImgSetRoi(&udp,roi_id);
 
    /* retrieve data items from the frame                                    */
    ImgGetFrameAttributes(fid,itmlst);

    /* check for parameters not handled in this version                      */

    if((pixel_path != ImgK_RightToLeft) && 
        (pixel_path != ImgK_LeftToRight))	    /* bad pixel path       */
            ChfSignal( 1, ImgX_INVPXLPTH);

    if((line_progression != ImgK_TopToBottom) && 
        (line_progression != ImgK_BottomToTop))    /* bad line progressio  */
            ChfSignal( 1, ImgX_INVLINPRG);

    if(spect_type != ImgK_ClassBitonal)            /* non-bitonal	    */
        ChfSignal( 1, ImgX_UNSSPCTYP);

    if (pp_pixel_dist != lp_pixel_dist)              /*1:1 aspect ratio      */
        ChfSignal( 1, ImgX_UNSASPRAT);

    if(grid_type != ImgK_RectangularGrid)          /* non-rectangular grid */
        ChfSignal( 1, ImgX_UNSGRDTYP);

    if(compression_type != ImgK_PcmCompression)    /* image is compressed  */
        ChfSignal( 1, ImgX_UNSCMPTYP);

    if(udp.UdpL_PxlStride != bits_per_pixel)	             /* pixel stride not 1   */
        ChfSignal( 1, ImgX_UNSPXLSTR);

    if(out_buflen < 0)
        ChfStop( 1, ImgX_INVBUFLEN);			    /* inv. buffer size     */

    if(out_buf_ptr != 0 && out_buflen < MAXOUTSIZE)
        ChfStop( 1, ImgX_INVBUFLEN);			    /* inv. buffer size     */

    if(out_buf_ptr == 0 && action == 0)              /* action rtn. req'd,   */
        ChfStop( 1, ImgX_ACTNOTDEF);                     /* not specified        */

    /* assign number pixels, number scans                                    */

    nscans = udp.UdpL_Y2 - udp.UdpL_Y1 + 1;
    npix = udp.UdpL_X2 - udp.UdpL_X1 + 1;
    /*
    ** compute PS "BoundingBox" - the maximum rectangle of the image in points
    ** (point = 1/72 inch). .24 == 72 (points/inch) / 300 (dots/inch printer)
    ** as we are scaling to the exact raster size of a 300 dpi printer.
    ** As the Box must be an integer yet contain the ENTIRE image, the "ceil"
    ** function is used to compute the max x and max y dimensions. The
    ** "floor" function is implicit in the constants 0 and 0 for the lower
    ** bounds of the box.
    */
    boundbox_urx = ceil((double)(.24 * npix));
    boundbox_ury = ceil((double)(.24 * nscans));

    /* set up buffer of length buflen if necessary; set internal flag       */

    buf_ptr = out_buf_ptr;
    buflen = out_buflen;
    if (out_buf_ptr == 0)
        {

        /* if buffer must be allocated, get one big enough for the header and  */
        /* linefeeds necessary, plus the size of the image                     */
        /* also, must get 2x image size if encoding in hex...                 */

        if(buflen == 0)
            {
            if (flags & ImgM_SerialBinaryEncoding)
                buflen = 1000 + ((npix + 7) / 8 / MAXOUTCHAR + 1) * nscans +
                    (npix + 7) / 8 * nscans;
            else
                buflen = 1000 + (npix + 7) / 8 * nscans * 2;
            }
        buf_ptr = (unsigned char *)_ImgCalloc(1,buflen);
        }
    temp_scan_buffer = (unsigned char *)_ImgCalloc(1,(npix+7)/8);

    work_buf_ptr = buf_ptr;			    /* current output ptr.  */
    last_byte = buf_ptr + buflen - 1;		    /* last byte in buffer  */
    *byt_cnt = 0;

    /* pick correct lookup table and assign pointer to it.                    */
    /* NOTE: since PostScript requires its bytes to be in REVERSE order,      */
    /* bytes that are in NORMAL order must be REVERSED!                       */

    if (pixel_order != ImgK_ReversePixelOrder )
        lut = IMG_AB_LW_BIN_RVSD;
    if (pixel_order == ImgK_ReversePixelOrder )
        lut = IMG_AB_LW_BIN;

    /* if serial line binary flag not specified, use ASCII hex encoding       */
    /* and assign proper LUT to reference the ASCII-hex tables                */

    if (pixel_order != ImgK_ReversePixelOrder )
        hex_lut = IMG_AW_ASCII_HEX_RVSD;
    if (pixel_order == ImgK_ReversePixelOrder )
        hex_lut = IMG_AW_ASCII_HEX;

    /* generate a PostScript comment section                                  */

    IMG_ADD_PS_BUFF(&work_buf_ptr,buf_ptr,byt_cnt,"%%!PS-Adobe-2.0 EPSF-1.2\n",
        action,userparm,last_byte);
    IMG_ADD_PS_BUFF(&work_buf_ptr,buf_ptr,byt_cnt,"%%%%Title: Bitmap Image \n",
        action,userparm,last_byte);
    IMG_ADD_PS_BUFF(&work_buf_ptr,buf_ptr,byt_cnt,"%%%%Creator: VAS V1.1 \n",
        action,userparm,last_byte);
/*
**  creation date will be left out for now...
**    IMG_ADD_PS_BUFF(&work_buf_ptr,buf_ptr,byt_cnt,"%%%%CreationDate: **** \n",
**        action,userparm,last_byte);
*/
    IMG_ADD_PS_BUFF(&work_buf_ptr,buf_ptr,byt_cnt,
        "%%%%BoundingBox: 0 0 %d %d \n",action,userparm,last_byte,
        boundbox_urx,boundbox_ury);
    IMG_ADD_PS_BUFF(&work_buf_ptr,buf_ptr,byt_cnt,"%%%%Pages: 1 1 \n",
        action,userparm,last_byte);
    IMG_ADD_PS_BUFF(&work_buf_ptr,buf_ptr,byt_cnt,"%%%%EndComments \n",
        action,userparm,last_byte);

    /* write text of the PostScript prologue code                             */

    IMG_ADD_PS_BUFF(&work_buf_ptr,buf_ptr,byt_cnt,"/DEC_IMG_dict 50 dict def \n",
        action,userparm,last_byte);
    IMG_ADD_PS_BUFF(&work_buf_ptr,buf_ptr,byt_cnt,"DEC_IMG_dict begin \n",
        action,userparm,last_byte);
    IMG_ADD_PS_BUFF(&work_buf_ptr,buf_ptr,byt_cnt,"/inch {72 mul} def end \n",
        action,userparm,last_byte);
    IMG_ADD_PS_BUFF(&work_buf_ptr,buf_ptr,byt_cnt,"%%%%EndProlog \n",
        action,userparm,last_byte);

    /* write text of PostScript page                                          */

    IMG_ADD_PS_BUFF(&work_buf_ptr,buf_ptr,byt_cnt,"%%%%Page: 1 1 \n",
        action,userparm,last_byte);
    IMG_ADD_PS_BUFF(&work_buf_ptr,buf_ptr,byt_cnt,"save \n",
        action,userparm,last_byte);
    IMG_ADD_PS_BUFF(&work_buf_ptr,buf_ptr,byt_cnt,"DEC_IMG_dict begin \n",
        action,userparm,last_byte);

    if (flags & ImgM_SerialBinaryEncoding)
        IMG_ADD_PS_BUFF(&work_buf_ptr,buf_ptr,byt_cnt,"/pic 512 string def \n",
            action,userparm,last_byte);
    else
        IMG_ADD_PS_BUFF(&work_buf_ptr,buf_ptr,byt_cnt,"/pic %d string def \n",
            action,userparm,last_byte,(npix + 7) / 8);
    if (brt_polarity == ImgK_ZeroMaxIntensity) /*reversed polarity */
        IMG_ADD_PS_BUFF(&work_buf_ptr,buf_ptr,byt_cnt,
            "{1 exch sub} settransfer \n",action,userparm,last_byte);
    IMG_ADD_PS_BUFF(&work_buf_ptr,buf_ptr,byt_cnt,"/afterimage { \n",
        action,userparm,last_byte);
    IMG_ADD_PS_BUFF(&work_buf_ptr,buf_ptr,byt_cnt,"} def \n",
        action,userparm,last_byte);
    IMG_ADD_PS_BUFF(&work_buf_ptr,buf_ptr,byt_cnt,"/btimage \n",
        action,userparm,last_byte);
    IMG_ADD_PS_BUFF(&work_buf_ptr,buf_ptr,byt_cnt,"{npix nscans nbits ",
        action,userparm,last_byte);

    /* check orientation of input data. generate correct PS transform matrix  */

    if (pixel_path == (ImgK_LeftToRight) && 
        line_progression == (ImgK_TopToBottom))
            IMG_ADD_PS_BUFF(&work_buf_ptr,buf_ptr,byt_cnt,
                "[npix 0 0 nscans neg 0 nscans] \n",action,userparm,last_byte);
    else if (pixel_path == (ImgK_RightToLeft) && 
        line_progression == (ImgK_TopToBottom))
            IMG_ADD_PS_BUFF(&work_buf_ptr,buf_ptr,byt_cnt,
                "[npix neg 0 0 nscans neg npix nscans] \n",action,userparm,last_byte);
    else if (pixel_path == (ImgK_RightToLeft) && 
        line_progression == (ImgK_BottomToTop))
            IMG_ADD_PS_BUFF(&work_buf_ptr,buf_ptr,byt_cnt,
                "[npix neg 0 0 nscans npix 0] \n",action,userparm,last_byte);
    else if (pixel_path == (ImgK_LeftToRight) && 
        line_progression == (ImgK_BottomToTop))
            IMG_ADD_PS_BUFF(&work_buf_ptr,buf_ptr,byt_cnt,
                "[npix 0 0 nscans 0 0] \n",action,userparm,last_byte);

    if (flags & ImgM_SerialBinaryEncoding)
        IMG_ADD_PS_BUFF(&work_buf_ptr,buf_ptr,byt_cnt,
            "{currentfile pic readline pop} \n",action,userparm,last_byte);
    else
        IMG_ADD_PS_BUFF(&work_buf_ptr,buf_ptr,byt_cnt,
            "{currentfile pic readhexstring pop} \n",action,userparm,last_byte);
    IMG_ADD_PS_BUFF(&work_buf_ptr,buf_ptr,byt_cnt,
        "image afterimage} bind def \n",action,userparm,last_byte);

    IMG_ADD_PS_BUFF(&work_buf_ptr,buf_ptr,byt_cnt,
        "/npix %d def \n",action,userparm,last_byte,npix);
    IMG_ADD_PS_BUFF(&work_buf_ptr,buf_ptr,byt_cnt,
        "/nscans %d def \n",action,userparm,last_byte,nscans);
    IMG_ADD_PS_BUFF(&work_buf_ptr,buf_ptr,byt_cnt,
        "/nbits %d def \n",action,userparm,last_byte,bits_per_pixel);
    IMG_ADD_PS_BUFF(&work_buf_ptr,buf_ptr,byt_cnt,
        "npix inch 300 div nscans inch 300 div scale \n",
            action,userparm,last_byte);

    /* note in the following line that NO spaces are allowed between "btimage"*/
    /* and the \n that follows. Image gets unexpected end-of-data if space is */
    /* present.                                                               */

    IMG_ADD_PS_BUFF(&work_buf_ptr,buf_ptr,byt_cnt,"btimage\n",
        action,userparm,last_byte);

    /* write out the image data, translated by the lookup table               */
    /* use MOVV5 to align each scan to byte boundary if necessary             */

    srcbitptr = udp.UdpL_Pos - udp.UdpL_ScnStride;
    for (i = 1; i <= nscans; i++)
        {
        srcbitptr += udp.UdpL_ScnStride;
        bytes_left_in_scan = (npix + 7) / 8;

        if ((srcbitptr & 0x7) == 0)                 /* buffer on byte boundary?*/
            scan_bufptr = (unsigned char *)(udp.UdpA_Base + (srcbitptr >> 3));
        else                                        /*No. Move to byte boundary*/
            {
            _IpsMovv5(npix,srcbitptr,udp.UdpA_Base,0,temp_scan_buffer);
            scan_bufptr = temp_scan_buffer;
            }

        if (flags & ImgM_SerialBinaryEncoding)
            {
            while (bytes_left_in_scan > 0)              /* encode bytes in 1 scan  */
                {
                tran_len = (bytes_left_in_scan < MAXOUTCHAR ? bytes_left_in_scan :
                    MAXOUTCHAR);
    
                /* check for room enough for this buffer                            */
    
                CHECK_BUFFER_(tran_len+1);
    
                _IpsMovtcLong(tran_len,scan_bufptr,LINE_FEED,lut,tran_len+1,
                    work_buf_ptr);                         /* adds <LF> as last char */
                scan_bufptr += tran_len;
                work_buf_ptr += tran_len + 1;
                bytes_left_in_scan -= tran_len;
                }
            }
        else                    /* ASCII hex encoding                         */
            {
            CHECK_BUFFER_( bytes_left_in_scan * 2 );
            for( n = 0; n < bytes_left_in_scan; n++)
                {
                WRITE16_(work_buf_ptr,hex_lut[*scan_bufptr]);
		scan_bufptr++;
                /* *((short int *)work_buf_ptr) = hex_lut[*scan_bufptr++]; */
                work_buf_ptr += 2;
                
                }
            }
        }

    /* write PostScript trailer code                                          */

    IMG_ADD_PS_BUFF(&work_buf_ptr,buf_ptr,byt_cnt,"\nrestore showpage end \n",
        action,userparm,last_byte);
    IMG_ADD_PS_BUFF(&work_buf_ptr,buf_ptr,byt_cnt,"%%%%Trailer \n",
        action,userparm,last_byte);

    /* last call for action if action routine has been specified              */

    if (action != 0)
        {
        *byt_cnt += work_buf_ptr - buf_ptr;
        action_status = (*action)(buf_ptr,work_buf_ptr-buf_ptr,userparm);
        if ((action_status & 1) != 1)
            ChfSignal( 1, action_status);
        work_buf_ptr = buf_ptr;
        }
    else
        *byt_cnt = work_buf_ptr - buf_ptr;

    if(out_buf_ptr == 0)               /* clean up allocated storage          */
        _ImgCfree(buf_ptr);
    _ImgCfree(temp_scan_buffer);

    return (fid);                      /* all detected errors are signalled   */
    }

/* internal function to add a string to the PostScript buffer, optional parms*/
/* must be integers, and may be specified if the string (as input to sprintf)*/
/* has any "%d"'s in it.                                                     */
/* action routine, if nonzero will be called for buffer mgmt.                */

static void IMG_ADD_PS_BUFF(workbufptr,bufptr,byt_cnt,string,action,
    userparm,last_byte,opt1,opt2,opt3,opt4)
    unsigned char **workbufptr;
    unsigned char *bufptr;
    unsigned char string[];
    unsigned char *last_byte;
    int           *byt_cnt;
    long           (*action)();
    long           userparm;
    int           opt1,opt2,opt3,opt4;  /* opt. integer arguments to sprintf  */
    {
    long           action_status;
    char temp_buffer[100];     /* temp. buffer to hold string before */
                                        /* moving to actual buffer            */
    int           temp_size;

    /* produce the string in a temporary buffer to determine exact size...    */

    sprintf(temp_buffer,(char *) string,opt1,opt2,opt3,opt4);
    temp_size = strlen(temp_buffer);

    if (*workbufptr + temp_size > last_byte) /* too long for buffer left?     */
        {
        if (action != 0)
            {
            *byt_cnt += *workbufptr - bufptr;
            action_status = (*action)(bufptr,*workbufptr - bufptr,userparm);
            if ((action_status & 1) != 1)
                ChfSignal( 1, action_status);
            *workbufptr = bufptr;
            }
        else
            ChfStop( 1, ImgX_BUFOVRFLW);
        }

    memcpy(*workbufptr, temp_buffer, temp_size);
    *workbufptr += temp_size;
    return;
    }
