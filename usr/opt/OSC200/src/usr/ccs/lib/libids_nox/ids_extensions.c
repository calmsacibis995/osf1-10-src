
/****************************************************************************
**
**  Copyright (c) Digital Equipment Corporation, 1990 All Rights Reserved.
**  Unpublished rights reserved under the copyright laws of the United States.
**  The software contained on this media is proprietary to and embodies the
**  confidential technology of Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and media is authorized only
**  pursuant to a valid written license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the U.S.
**  Government is subject to restrictions as set forth in Subparagraph
**  (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19, as applicable.
***************************************************************************/

/*****************************************************************************
**
**  FACILITY:
**
**      Image Display Services (IDS)
**
**  ABSTRACT:
**
**      IDS extensions to Image Services Library.  Note: these extensions are
**	written for the friendly environment of IDS, i.e. like IMG$$ routines.
**
**  ENVIRONMENT:
**
**      VAX/VMS/Ultrix
**
**  AUTHOR(S):
**
**      Bob Shelley
**      Subu Garikapati
**	Michael D. O'Connor (9-January-1991)
**
**  CREATION DATE:
**
**     November 11, 1988
**
**  MODIFICATION HISTORY:
**
*****************************************************************************/

/*
**  Include files
*/
    /*
    **  Standard C include files
    */
#include <img/ChfDef.h>			    /* Condition handling functions */
#include <math.h>			    /* math routines		    */
    /*
    **  ISL and IDS include files
    */
#include    <img/ImgDef.h>	    /* ISL public symbols		     */
#include    <img/ImgEntry.h>    /* ISL public entry points		    */
#include    <img/ImgStatusCodes.h>

#ifdef IDS_NOX
#include    <ids__widget_nox.h> /* IDS public/private without X11 references*/
#else
#include    <ids__widget.h> /* IDS public/private, Dwtoolkit, Xlib defs.    */
#endif
#ifndef NODAS_PROTO
#include <idsprot.h>            /* IDS prototypes */
#endif
/*
**  MACRO definitions
*/
    /* none */
/*
**  Equated Symbols
*/
#define Rshft	0	/* requantize using right shift			    */
#define Lshft	1	/* requantize using left  shift			    */
#define Fmult	2	/* requantize using floating multiplication	    */
#define F_RGB	3	/* requantize by converting RGB to grayscale	    */

typedef struct _    {
    int	     s_pstr,  d_pstr;		    /* src and dst pixel stride	    */
    int	     s_lstr,  d_lstr;		    /* src and dst scanline stride  */
    int	     s_off[Ids_MaxComponents],	    /* src and dst component offsets*/
	     d_off[Ids_MaxComponents];
    int	     s_msk[Ids_MaxComponents],	    /* src and dst component masks  */
	     d_msk[Ids_MaxComponents];
    int	     mode[Ids_MaxComponents];	    /* mode of component conversion */
    int	     shft[Ids_MaxComponents];	    /* component shift count	    */
    float    mult[Ids_MaxComponents];	    /* component multiplication fact*/
    } ToneRequantizeDataStruct, *ToneRequantizeData;


/*
**  Table of contents
*/
#ifdef NODAS_PROTO
IdsHistogramData    IdsHistogram();		/* histogram pixel usage    */
unsigned long	   *IdsConvertFrame(); 		/* set up  comp space conv  */
unsigned long	   *IdsConvertPlane(); 		/* set up  comp space conv  */
unsigned long 	   *IdsScale(); 		/* set up for aspect ratio. */
unsigned long	   *IdsPixelRemap();		/* special case of ISL ver. */
unsigned long	   *IdsPixelTrueRemap();	/* special case of ISL ver. */
unsigned long	   *IdsToneScale();		/* gray,color tonescale adj */
unsigned long	   *IdsSharpen();		/* high pass filtering      */
unsigned long	   *IdsDither();		/* call dither or requantize*/
unsigned long	    IdsRequantize();		/* requantize pixel values  */
void		    IdsPutItem();		/* append item to PUT_ITMLST*/
void		    IdsSetItmlst();		/* append item to ITMLST    */
void		    IdsFreePutList();		/* deallocate ISL PUT_ITMLST*/
void		    IdsFreeItmlst();		/* deallocate ISL ITMLST    */

static RemapLut     ToneScaleLUT();             /* LUT for tonescale funct  */
static float       *ObtainScanlinePixels();     /* enhance,high pass filter */
static void         Convolve();                 /* enhance,high pass filter */
static void	    RequantizeScanline();	/* requantize one scanline  */
static void	    RemapByLUT();		/* remap using LUT	    */
static void	    RemapByTruePixels();	/* remap using true pixels  */
static void	    RemapByColorLUTs();		/* remap using color LUT    */
static unsigned long  *CloneFid();			/* Clone fid for Requantize */
static void	    SetScanlineStride();	/* compute scanline stride  */

#else
PROTO(static RemapLut ToneScaleLUT, (float */*punch_1*/, float */*punch_2*/, unsigned long /*stride*/, unsigned long /*slev*/, unsigned long /*dlev*/));
PROTO(static void RequantizeScanline, (ToneRequantizeData /*rq*/, int /*pixels*/, int /*comps*/, char */*s_base*/, int /*s_beg*/, char */*d_base*/, int /*d_beg*/));
PROTO(static void RemapByLUT, (struct UDP */*src*/, struct UDP */*dst*/, RemapLut /*lut*/));
PROTO(static void RemapByTruePixels, (ToneRequantizeData /*tone*/, struct UDP */*src*/, struct UDP */*dst*/));
PROTO(static float *ObtainScanlinePixels, (ToneRequantizeData /*sharp*/, struct UDP */*src*/, int /*scanline*/, long int /*s_pos*/, int /*s_type*/));
PROTO(static void Convolve, (int /*pixels*/, float */*lines*/[], char */*kernel1*/, float */*pixelsret*/));
PROTO(static void RemapByColorLUTs, (ToneRequantizeData /*tone*/, struct UDP */*src*/, struct UDP */*dst*/, RemapLut /*lut_red*/, RemapLut /*lut_grn*/, RemapLut /*lut_blu*/));
PROTO(static unsigned long CloneFid, (unsigned long /*fid*/, struct PUT_ITMLST */*attr*/, unsigned long /*roi*/));
PROTO(static void SetScanlineStride, (unsigned long /*fid*/, struct PUT_ITMLST */*attr*/, struct UDP */*udp*/, unsigned long /*roi*/, int /*scnt*/));
PROTO(unsigned long IdsRequantize, (unsigned long /*fid*/, struct PUT_ITMLST */*attr*/, unsigned long /*roi*/, unsigned long */*levels*/));/*put in by Dhiren 10/28/93*/
PROTO(void IdsSetItmlst, (struct ITMLST **/*itmlst*/, unsigned long int /*code*/, unsigned long int /*value*/, unsigned long int /*index*/));/*put in by Dhiren 10/28/93*/
PROTO(void IdsFreeItmlst, (struct ITMLST **/*itmlst*/));/*put in by Dhiren 10/28/93*/
#endif

/*
**  External References:
**
*/
#ifdef NODAS_PROTO
extern int		 _ImgAllocateDataPlane();
extern char		*_ImgCalloc();
extern void		 _ImgCfree();
extern unsigned long	*_ImgCheckNormal();
extern unsigned long	*_ImgCloneFrame();
extern unsigned long	*_ImgErase();
extern void		 _IpsMovtcLong();
extern unsigned long	*_ImgPut();
extern char		*_ImgRealloc();
extern struct UDP	*_ImgSetRoi();
extern void		 _ImgStoreDataPlane();
#endif

/* mask array for the GetField_() and PutField_() MACROs */
#if ( ( defined(__VAXC) || defined(VAXC) ) && (!defined(__alpha) && !defined(ALPHA) ) ) 
globalref unsigned int IMG_AL_ONES_ASCENDING[33];
#else
extern    unsigned int IMG_AL_ONES_ASCENDING[33];
#endif

/*
**	Local Storage
*/
    /* none */

/*****************************************************************************
**  IdsHistogram
**
**  FUNCTIONAL DESCRIPTION:
**
**	Create a histogram of pixel value usage.
**
**  FORMAL PARAMETERS:
**
**      fid            - image frame ID
**      roi            - region of interest identifier
**
**  FUNCTION VALUE:
**
**	IdsHistogramData - address of structure containing count and pointer.
**
*******************************************************************************/
IdsHistogramData IdsHistogram( fid, roi )
 unsigned long    fid;			    /* input frame		    */
 unsigned long    roi;			    /* region of interest	    */
{
    char *base;
    int pixel_bits, pixel_stride, pixels, line_stride, lines, offset;
    int mask, pixel, line;
    struct UDP src;
    unsigned long *hist;
    IdsHistogramData histogram;

#ifdef TRACE
printf( "Entering Routine IdsHistogram in module IDS_EXTENSIONS \n");
#endif

    histogram =
             (IdsHistogramData) _ImgCalloc(1, sizeof(IdsHistogramDataStruct));
    /*
    **  Get a working copy of the image's UDP.
    */
    GetIsl_( fid, Img_Udp, src, 0 );
    if( roi != 0 )
        /*
        **  Apply the ROI to our copy of the UDP.
        */
        _ImgSetRoi( &src, roi );

    /*
    **  Get/calculate the parameters we need from the UDP.
    */
    pixel_bits   = src.UdpW_PixelLength;
    pixel_stride = src.UdpL_PxlStride;
    pixels       = src.UdpL_X2 - src.UdpL_X1 + 1;
    line_stride  = src.UdpL_ScnStride;
    lines        = src.UdpL_Y2 - src.UdpL_Y1 + 1;
    base         = (char *) src.UdpA_Base;
    offset       = src.UdpL_Pos;

    /*
    **  Allocate the histogram buffer: size depends on bits per pixel.
    */
    histogram->count = 1 << pixel_bits;
    hist = (unsigned long *) _ImgCalloc( histogram->count, sizeof(long) );
    histogram->pointer = hist;
         
    /*
    **  Mask for extracting pixel as a bit field from an unaligned bit stream.
    */
    mask = IMG_AL_ONES_ASCENDING[pixel_bits];

    /*
    **  Create a histogram of this image's pixel value usage.
    */
    for( line = 0; line < lines; offset = ++line * line_stride + src.UdpL_Pos )
        /*
        **  Scanline by scanline...
        */
        for( pixel = 0;  pixel < pixels; offset += pixel_stride, pixel++ )
            /*
            **  Pixel by pixel...
            */
            hist[ GetField_(base, offset, mask) ]++;
            
#ifdef TRACE
printf( "Leaving Routine IdsHistogram in module IDS_EXTENSIONS \n");
#endif
    return( histogram );
}

/*****************************************************************************
**  IdsPixelRemap
**
**  FUNCTIONAL DESCRIPTION:
**
**	Remap image pixel values using supplied LUT.
**
**	The entire pixel is remapped including pad bits (if bits per pixel 
**	is less than pixel stride) -- CAVEAT EMPTOR --.
**
**  FORMAL PARAMETERS:
**
**      fid     - image frame ID
**	lut	- LUT structure
**      attr    - attribute itemlist applied to new frame
**      roi     - region of interest identifier
**
**  FUNCTION VALUE:
**
**	newfid	- new image frame ID
**
*******************************************************************************/
unsigned long IdsPixelRemap( fid, lut, attr, roi )
 unsigned long	    fid;		/* input frame			    */
 RemapLut	     lut;		/* lookup table descriptor	    */
 struct PUT_ITMLST  *attr;		/* attrib. for new frame	    */
 unsigned long	    roi;		/* region of interest		    */
{
    unsigned long   dst_fid;
    struct UDP src, dst;

#ifdef TRACE
printf( "Entering Routine IdsPixelRemap in module IDS_EXTENSIONS \n");
#endif

    /*
    **	Get src UDP, and handle ROI .
    */
    GetIsl_( fid, Img_Udp, src, 0 );
    if( roi != 0 )
	_ImgSetRoi( &src, roi );

    /*
    **	Create the new fid and fetch its UDP.
    */
    dst_fid = CloneFid( fid, attr, roi );
    GetIsl_( dst_fid, Img_Udp, dst, 0 );

    RemapByLUT( &src, &dst, lut );

#ifdef TRACE
printf( "Leaving Routine IdsPixelRemap in module IDS_EXTENSIONS \n");
#endif
    return( dst_fid ); 
}


/*****************************************************************************
**  IdsPixelTrueRemap
**
**  FUNCTIONAL DESCRIPTION:
**
**	Remap image pixel values using true colors
**
**
**  FORMAL PARAMETERS:
**
**      fid     - image frame ID
**      attr    - attribute itemlist applied to new frame
**      roi     - region of interest identifier
**
**  FUNCTION VALUE:
**
**	newfid	- new image frame ID
**
*******************************************************************************/
unsigned long IdsPixelTrueRemap( fid, attr, roi )
 unsigned long      fid;		/* input frame			    */
 struct PUT_ITMLST  *attr;		/* attrib. for new frame	    */
 unsigned long      roi;		/* region of interest		    */
{
    unsigned long dst_fid;
    struct UDP src, dst;
    unsigned long   src_bpc[Ids_MaxComponents];/* source bits per component */
    unsigned long   dst_bpc[Ids_MaxComponents];/* source bits per component */
    ToneRequantizeDataStruct tone;  
    int   spect_cnt, i;

#ifdef TRACE
printf( "Entering Routine IdsPixelTrueRemap in module IDS_EXTENSIONS \n");
#endif

    /*
    **  Make sure scanline stride is valid, if specified in the 'attr' list.
    **  (A hack to solve floating point discrepencies between VAX and PMAX)
    */
    SetScanlineStride( fid, attr, &src, roi, 0 );
   
    /*
    **	Get src UDP, and handle ROI .
    */
    GetIsl_( fid, Img_Udp, src, 0 );
    if( roi != 0 )
	_ImgSetRoi( &src, roi );

    /*
    **	Create the new fid and fetch its UDP.
    */
    dst_fid = CloneFid( fid, attr, roi );
    GetIsl_( dst_fid, Img_Udp, dst, 0 );

    /*
    ** Obtain spect count, bits/component, offset and mask of source fid
    */
    GetBitsPerComponent_( fid, spect_cnt, src_bpc );
    for( i = 0; i < Ids_MaxComponents; i++ )
        {
        tone.s_off[i] = i == 0 ? 0 : tone.s_off[i-1] + src_bpc[i-1];
        tone.s_msk[i] = (1 << src_bpc[i]) - 1;
        }
    /*
    ** Obtain spect count, bits/component, offset and mask of dest fid
    */
    GetBitsPerComponent_( dst_fid, spect_cnt, dst_bpc );
    for( i = 0; i < Ids_MaxComponents; i++ )
        {
        tone.d_off[i] = i == 0 ? 0 : tone.d_off[i-1] + dst_bpc[i-1];
        tone.d_msk[i] = (1 << dst_bpc[i]) - 1;
        }
    
    RemapByTruePixels( &tone, &src, &dst );

    return( dst_fid ); 
#ifdef TRACE
printf( "Leaving Routine IdsPixelTrueRemap in module IDS_EXTENSIONS \n");
#endif
}

/*****************************************************************************
**  IdsScale
**
**  FUNCTIONAL DESCRIPTION:
**
**   To accomidate for scale mode of  Pixel Aspect ratio  not equal to 1. If
**   the pixel aspect ratio of fid is not 1 then the image is scaled in 
**   correspondence to the aspect ratio. The pixel_path_dist and line_path_dist
**   frame attributes are set to 1 before the image is scaled. Then the above
**   mentioned frame attributes are set back to their orginal values. This is
**   all done to avoid ISL warning of "attribute does not have normalized value"
**
**  FORMAL PARAMETERS:
**
**      fid     - image frame ID
**      x_scale - ISL x scale factor
**      y_scale - ISL y scale factor
**      roi     - region of interest identifier
**      sflags  - ISL scale options flag
**      pad     -
**  FUNCTION VALUE:
**
**      newfid  - new image frame ID
**
*******************************************************************************/
unsigned long IdsScale( fid, x_scale, y_scale, roi, s_flags, pad )
 unsigned long         fid;               /* input frame                      */
 float              *x_scale;           /* x scale factor                   */
 float              *y_scale;           /* y scale factor                   */
 unsigned long      roi;               /* region of interest               */
 int                 s_flags;           /* flags for scale                  */
 int                 pad;
{
    int pp_dist, lp_dist;
    int ppp_dist, llp_dist;
    unsigned long d_fid;

#ifdef TRACE
printf( "Entering Routine IdsScale in module IDS_EXTENSIONS \n");
#endif

    /*
    ** Read the aspect value, the pixel and line path distances
    */
    GetIsl_( fid, Img_PPPixelDist, ppp_dist, 0 );
    GetIsl_( fid, Img_LPPixelDist, llp_dist,  0 );

    /*
    **  Set the Pixel and line path distances to 1. ie aspect ratio = 1
    */
    pp_dist = lp_dist = 1;
    fid = ImgSet(fid,Img_PPPixelDist,&pp_dist,sizeof(long),0);
    fid = ImgSet(fid,Img_LPPixelDist,&lp_dist,sizeof(long),0);

    /*
    **  Do the required scaling
    */
    d_fid = ImgScale( fid, x_scale, y_scale, (struct ROI *)roi, s_flags, pad);
            
    /*
    **  Set the Pixel and line path dist back to the actual values for old fid
    */
    fid = ImgSet(fid,Img_PPPixelDist,&ppp_dist,sizeof(long),0);
    fid = ImgSet(fid,Img_LPPixelDist,&llp_dist,sizeof(long),0);

#ifdef TRACE
printf( "Leaving Routine IdsScale in module IDS_EXTENSIONS \n");
#endif

    return( d_fid );
}
          

/*****************************************************************************
**  IdsConvertFrame
**
**  FUNCTIONAL DESCRIPTION:
**
**  Convert fid from BandIntrlvdByPlane to BandIntrlvdByPixel
**
**  FORMAL PARAMETERS:
**
**      fid     - image frame ID
**  FUNCTION VALUE:
**
**      newfid  - new image frame ID
**
*******************************************************************************/
unsigned long IdsConvertFrame( fid )
 unsigned long         fid;               /* input frame                      */
{
    struct ITMLST  *attr = 0;
    unsigned long d_fid = fid;
    unsigned long  new_cs;

#ifdef TRACE
printf( "Entering Routine IdsConvertFrame in module IDS_EXTENSIONS \n");
#endif

    GetIsl_(fid, Img_CompSpaceOrg, new_cs, 0);
    if ( new_cs == ImgK_BandIntrlvdByPlane )
     {
      new_cs = ImgK_BandIntrlvdByPixel;
      IdsSetItmlst(&attr, Img_CompSpaceOrg,     new_cs,         0);
      /*
      **  Do the required space conversion
      */
      d_fid = ImgConvertFrame(fid,attr,0);
      IdsFreeItmlst( &attr );
      GetIsl_(d_fid, Img_CompSpaceOrg, new_cs, 0);
     } 
#ifdef TRACE
printf( "Leaving Routine IdsConvertFrame in module IDS_EXTENSIONS \n");
#endif

    return( d_fid );
}

/*****************************************************************************
**  IdsConvertPlane
**
**  FUNCTIONAL DESCRIPTION:
**
**  Convert fid from BandIntrlvdByPixel to BandIntrlvdByPlane
**
**  FORMAL PARAMETERS:
**
**      fid     - image frame ID
**  FUNCTION VALUE:
**
**      newfid  - new image frame ID
**
*******************************************************************************/
unsigned long IdsConvertPlane( fid )
 unsigned long         fid;               /* input frame                      */
{
    struct ITMLST  *attr = 0;
    unsigned long d_fid = fid;
    unsigned long  new_cs, isl_parm;

#ifdef TRACE
printf( "Entering Routine IdsConvertPlane in module IDS_EXTENSIONS \n");
#endif

    GetIsl_( fid, Img_CompSpaceOrg, new_cs, 0);
    GetIsl_( fid, Img_CompressionType, isl_parm, 0 );
    if ( new_cs == ImgK_BandIntrlvdByPixel )
     {
      /*
      ** If comp space is BandByPixel and if image is not compressed then
      ** decompress and then convert to BandByPlane or native format. Note if
      ** the comp space is Band ByPlane then decompression happens in the pipe.
      */
      if( isl_parm != ImgK_PcmCompression )
        ImgDecompressFrame( fid, ImgM_InPlace, 0 );
  
      new_cs = ImgK_BandIntrlvdByPlane;
      IdsSetItmlst(&attr, Img_CompSpaceOrg,     new_cs,         0);
      /*
      **  Do the required space conversion
      */
      d_fid = ImgConvertFrame(fid,attr,0);
      IdsFreeItmlst( &attr );
      GetIsl_(d_fid, Img_CompSpaceOrg, new_cs, 0);

     } 
#ifdef TRACE
printf( "Leaving Routine IdsConvertPlane in module IDS_EXTENSIONS \n");
#endif
    return( d_fid );
}

/*****************************************************************************
**  IdsToneScale
**
**  FUNCTIONAL DESCRIPTION:
**
**	Remap grayscale image pixel values as a function of the punch factors.
**
**  FORMAL PARAMETERS:
**
**      fid     - image frame ID
**      punch_1	- 
**      punch_2	- 
**      attr    - attribute itemlist applied to new frame
**      roi     - region of interest identifier
**	levels	- ptr to the number of output levels per comp
**
**  FUNCTION VALUE:
**
**	newfid	- new image frame ID
**
*******************************************************************************/
unsigned long IdsToneScale( fid, punch1, punch2, attr, roi, dlev )
 unsigned long	    fid;		/* input frame			    */
 float		    *punch1;		/*				    */
 float		    *punch2;		/*				    */
 struct PUT_ITMLST  *attr;		/* attrib. for new frame	    */
 unsigned long       roi;		/* region of interest		    */
 unsigned long	     dlev[];		/* number of dest output levels/comp*/
{
    unsigned long dst_fid;
    struct UDP src, dst;
    RemapLut lut_gray, lut_red, lut_grn, lut_blu;
    ToneRequantizeDataStruct tone;
    unsigned long   src_bpc[Ids_MaxComponents];/* source bits per component */
    unsigned long   dst_bpc[Ids_MaxComponents];/* source bits per component */
    unsigned long   stride;             /* num of bits making up an entry   */
    int   spect_type, spect_cnt, i;

#ifdef TRACE
printf( "Entering Routine IdsToneScale in module IDS_EXTENSIONS \n");
#endif

    /*
    **  Make sure scanline stride is valid, if specified in the 'attr' list.
    **  (A hack to solve floating point discrepencies between VAX and PMAX)
    */
    SetScanlineStride( fid, attr, &src, roi, 0 );

    /*
    **  Create the new fid and fetch its UDP.
    */
    dst_fid = CloneFid( fid, attr, roi );
    GetIsl_( dst_fid, Img_Udp, dst, 0 );
/*
    if( src.UdpW_PixelLength != dst.UdpW_PixelLength)
        ChfStop (1,ImgX_INVBITPXL);
*/  

    /*
    ** Obtain spect count, bits/component, offset and mask of source fid
    */
    GetBitsPerComponent_( fid, spect_cnt, src_bpc );
    for( i = 0; i < Ids_MaxComponents; i++ )
        {
        tone.s_off[i] = i == 0 ? 0 : tone.s_off[i-1] + src_bpc[i-1];
        tone.s_msk[i] = (1 << src_bpc[i]) - 1;
        }
    /*
    ** Obtain spect count, bits/component, offset and mask of dest fid
    */
    GetBitsPerComponent_( dst_fid, spect_cnt, dst_bpc );
    for( i = 0; i < Ids_MaxComponents; i++ )
        {
        tone.d_off[i] = i == 0 ? 0 : tone.d_off[i-1] + dst_bpc[i-1];
        tone.d_msk[i] = (1 << dst_bpc[i]) - 1;
        }

    GetIsl_(fid, Img_SpectType, spect_type, 0);
    switch( spect_type )
     {
    case ImgK_StypeGrayscale :
      /*
      **  Initialize total # of bits, for an entry
      */
      stride   = dst.UdpL_PxlStride == 8 ? 8 : dst.UdpW_PixelLength;

      /*
      **  Does the  gray tonescale.
      */
      lut_gray = ToneScaleLUT( punch1, punch2, stride, 1 << src_bpc[GRA],
                                                                    dlev[GRA] );
      RemapByLUT( &src, &dst, lut_gray );
       _ImgCfree( lut_gray->base );
       _ImgCfree( lut_gray );
      break;
    case ImgK_StypeMultispect :
      /*
      **  Does the color tonescale.
      */
      lut_red = ToneScaleLUT( punch1, punch2, dst_bpc[RED], 1 << src_bpc[RED],
                                                                   dlev[RED] ); 
      lut_grn = ToneScaleLUT( punch1, punch2, dst_bpc[GRN], 1 << src_bpc[GRN],
                                                                   dlev[GRN] );
      lut_blu = ToneScaleLUT( punch1, punch2, dst_bpc[BLU], 1 << src_bpc[BLU],
                                                                   dlev[BLU] );
      RemapByColorLUTs( &tone, &src, &dst, lut_red, lut_grn, lut_blu );
       _ImgCfree( lut_red->base );
       _ImgCfree( lut_red );
       _ImgCfree( lut_grn->base );
       _ImgCfree( lut_grn );
       _ImgCfree( lut_blu->base );
       _ImgCfree( lut_blu );
      break;
     }

#ifdef TRACE
printf( "Leaving Routine IdsToneScale in module IDS_EXTENSIONS \n");
#endif

    return( dst_fid ); 
}


/*****************************************************************************
**  ToneScaleLUT
**
**  FUNCTIONAL DESCRIPTION:
**
**	Remap grayscale or color/comp image pixel values as a function of 
**      the punch factors.
**
**  FORMAL PARAMETERS:
**
**      punch_1	- 
**      punch_2	- 
**      stride  - 
**	slev 	- 
**	dlev 	- 
**
**  FUNCTION VALUE:
**
**	RemapLut - new remap look up table 
**
*******************************************************************************/
static RemapLut  ToneScaleLUT( punch_1, punch_2, stride, slev, dlev )
 float		    *punch_1;		/*				    */
 float		    *punch_2;		/*				    */
 unsigned long       stride;            /* num of bits making up an entry   */
 unsigned long	     slev;		/* number of source levels / comp   */
 unsigned long	     dlev;		/* number of output levels / comp   */
{

/*******************************************************************************
** 
**  The following code courtesy of Bob Ulichney....
**
**	A subroutine to generate the aggregate tone scale table.
*/
    RemapLut       lut;
    float  punch1, punch2, range, val;
    int	   src_maxval, dst_maxval, i, imax, imin, hi, lo, neg=0;
    int    xyz, xzy; 

#ifdef TRACE
printf( "Entering Routine ToneScaleLUT in module IDS_EXTENSIONS \n");
#endif


    /*
    **  Initialize our LUT structure.
    */
    lut = (RemapLut) _ImgCalloc( 1, sizeof(RemapLutStruct) );
    lut->stride = stride;
    lut->mask     = IMG_AL_ONES_ASCENDING[stride];
    lut->bytes  = (lut->stride * slev + 7)/ 8;
    lut->base   = (unsigned char *) _ImgCalloc( lut->bytes, sizeof(char) );

    /* 
    ** Adjust Punch Factors
    */
    punch1 = *punch_1;	
    punch2 = *punch_2;
    dst_maxval =  dlev - 1;
    src_maxval =  slev - 1;

    if( punch1 > punch2 )
	neg = 1, val = punch1, punch1 = punch2, punch2 = val;

    punch2 *= src_maxval, punch1 *= src_maxval;
    lo = punch1 + (punch1 > 0.0 ? 0.5 : -0.5 );	/* round */
    hi = punch2 + (punch2 > 0.0 ? 0.5 : -0.5 );

    /* 
    ** Compute area of table between punch1 and punch2. 
    */
    imin  = MAX( lo, 0 );
    imax  = MIN( hi, src_maxval );
    range = hi - lo;

    /*
    ** Prepare the LUT for the tonescale func, x-axis input y-axis ouput levels
    */
    if( range != 0 )
      for( i = imin; i < imax; i++ )
	PutField_(lut->base, lut->stride * i, lut->mask,
			    (int)(dst_maxval * (i - lo) / range + 0.5));
    /* 
    ** Fill punch2 clip area 
    */
    for( i = imax; i <  slev; i++ )
	PutField_(lut->base, lut->stride * i, lut->mask, dst_maxval);

    /* 
    ** If necessary negate the input by reversing the order of the table
    */
    if( neg ) for( i = 0; i < src_maxval - i; i++ )
	{
	PutField_(lut->base, lut->stride * i, lut->mask,
	GetField_(lut->base, lut->stride * (src_maxval - i), lut->mask));

	PutField_(lut->base, lut->stride * (src_maxval - i), lut->mask,
	GetField_(lut->base, lut->stride * i, lut->mask));

	PutField_(lut->base, lut->stride * i, lut->mask,
	GetField_(lut->base, lut->stride * (src_maxval - i), lut->mask));
	}

#ifdef TRACE
printf( "Leaving Routine ToneScaleLUT in module IDS_EXTENSIONS \n");
#endif

    return( lut ); 

}  /* End of GetTone */

          

/*****************************************************************************
**  IdsSharpen
**
**  FUNCTIONAL DESCRIPTION:
**
**	Sharpen the Image
**
**  FORMAL PARAMETERS:
**
**      fid     - image frame ID
**      roi     - region of interest identifier
**	beta 	- sharpening factor
**
**  FUNCTION VALUE:
**
**	newfid	- new image frame ID
**
*******************************************************************************/
unsigned long IdsSharpen( fid, beta, roi )
 unsigned long	    fid;		/* input frame			    */
 float        	    *beta;		/* sharpening knob                  */
 unsigned long	    roi;		/* region of interest		    */
{
    char  *s_base, *d_base;
    char   *kernel1;
    unsigned long   src_bpc[Ids_MaxComponents];/* source bits per component */
    unsigned long   dst_bpc[Ids_MaxComponents];/* source bits per component */
    int   spect_cnt, i;
    int cur; 
    int s_pos, d_pos, s_mask, d_mask, s_pstr, d_pstr, s_lstr, d_lstr;
    int t_pos, s_type;
    int pixel, pixels, lines;
    int   max_pixel_out =  0;           /* 2 ** m where n is bits/pixel dst */
    int   line = 0; 
    float  *data;
    float value;
    float   ratio = 1.0;
    float  *float_pixels;               /* Scanlines of float pixel values  */ 
    float  *scanlines_pixels[3];        /* 3 float ptrs to hold array pixels*/
    unsigned long dst_fid;
    struct UDP src, dst;                /* source and dest UDP's            */
    ToneRequantizeDataStruct sharp;

#ifdef TRACE
printf( "Entering Routine IdsSharpen in module IDS_EXTENSIONS \n");
#endif

    /*
    ** Build Laplacian filter
    */
    kernel1 = (char *)_ImgCalloc( 9, sizeof(float) );
    data = (float *) kernel1;

    /*
    ** Initialize Laplacian filter
    */
    value = 1.0 / 8.0 * (*beta);
    *data++ = value;     /* [-1, -1] */
    *data++ = value;     /* [ 0, -1] */
    *data++ = value;     /* [ 1, -1] */
    *data++ = value;     /* [-1,  0] */
    *data++ = -*beta;    /* [ 0,  0] */
    *data++ = value;     /* [ 0,  1] */
    *data++ = value;     /* [ 1, -1] */
    *data++ = value;     /* [ 1,  0] */
    *data++ = value;     /* [ 1,  1] */
     
   /*
   **  Get the src UDP descriptor, handle ROI, and figure image dimensions.
   */
    GetIsl_( fid, Img_Udp, src, 0);
    if( roi != 0 )
        _ImgSetRoi( &src, roi );

    
    /*
    **  Create the new fid and fetch its UDP. Only for Gray scale images
    */
    dst_fid = CloneFid( fid, 0, roi );
    GetIsl_( dst_fid, Img_Udp, dst, 0 );
/*
    if( src.UdpW_PixelLength != dst.UdpW_PixelLength)
        ChfStop (1,ImgX_INVBITPXL);
*/
    sharp.s_pstr    = src.UdpL_PxlStride;
    sharp.s_lstr    = src.UdpL_ScnStride;
    pixels          = src.UdpL_X2 - src.UdpL_X1 + 1;
    lines           = src.UdpL_Y2 - src.UdpL_Y1 + 1;
    s_base          = (char *) src.UdpA_Base;
    s_pos           = src.UdpL_Pos;
    s_mask          = IMG_AL_ONES_ASCENDING[src.UdpW_PixelLength];
    d_pstr          = dst.UdpL_PxlStride;
    d_lstr          = dst.UdpL_ScnStride;
    d_base          = (char *) dst.UdpA_Base;
    d_pos           = dst.UdpL_Pos;

    GetIsl_(fid, Img_SpectType, s_type, 0);
    switch( s_type )
     {
    case ImgK_StypeMultispect :
      sharp.mult[RED] = 0.299; 
      sharp.mult[GRN] = 0.587; 
      sharp.mult[BLU] = 0.114; 
      /*
      ** Obtain spect count, bits/component, offset and mask of source fid
      */
      GetBitsPerComponent_( fid, spect_cnt, src_bpc );
      for( i = 0; i < Ids_MaxComponents; i++ )
         {
          sharp.s_off[i] = i == 0 ? 0 : sharp.s_off[i-1] + src_bpc[i-1];
          sharp.s_msk[i] = (1 << src_bpc[i]) - 1;
         }
      /*
      ** Obtain spect count, bits/component, offset and mask of dest fid
      */
      GetBitsPerComponent_( dst_fid, spect_cnt, dst_bpc );
      for( i = 0; i < Ids_MaxComponents; i++ )
          {
          sharp.d_off[i] = i == 0 ? 0 : sharp.d_off[i-1] + dst_bpc[i-1];
          sharp.d_msk[i] = (1 << dst_bpc[i]) - 1;
          }
      break;
    case ImgK_StypeGrayscale :
      /*
      ** Calculate the maximum pixel value of dst
      */
      max_pixel_out = (1 << d_pstr) - 1;
      d_mask    = IMG_AL_ONES_ASCENDING[dst.UdpW_PixelLength];    
      break;
     }/* end of switch */

    /*
    ** Save the first pixel position ptr
    */
    t_pos     = s_pos;

    /*
    ** Initially scanline1 contains pixels all 0, scanline1 is the first line
    ** int of the image.
    */
    scanlines_pixels[1] = (float *)_ImgCalloc(pixels + 2, sizeof(float));
    scanlines_pixels[2] = (float *)ObtainScanlinePixels(&sharp, &src, 
                                                          line, s_pos, s_type );
    /*
    ** For loop to convert integer image to float image, calculate convolution,
    ** perform subtraction of laplacian filtered image from the origanal image
    ** for each  scanline by scanline,evaluate the boundary values of each pixel
    */
    /*
    ** When procesing the last line of image another scanline filled with pixel
    ** values 0 is needed. ie why the loop is forced to go one more time;
    */
    /*
    ** Position to the first pixel value of each scanline starting from the 
    ** second scanline.
    */ 
    for(line = 1, cur = 0, s_pos = line * sharp.s_lstr + src.UdpL_Pos; 
                          line <= lines; 
                          line++, cur++,
                          s_pos = line * sharp.s_lstr + src.UdpL_Pos,
                          t_pos = line * sharp.s_lstr + src.UdpL_Pos,  
                          d_pos = cur * d_lstr + dst.UdpL_Pos )
     {
      /*
      ** Make present scanline2 as the  scanline1, scanline3 as scanline2  
      */
      scanlines_pixels[0] = (float *)scanlines_pixels[1];
      scanlines_pixels[1] = (float *)scanlines_pixels[2];

      /*
      ** When procesing n'th line get pixel values for n + 1'th line. 0 -- 1
      */
      scanlines_pixels[2]=(float *)ObtainScanlinePixels(&sharp, &src, 
                                                       line, s_pos, s_type );
      /*
      ** Allocate memory to hold float pixel values for one scanline at a time
      */
      float_pixels     = (float *) _ImgCalloc(pixels, sizeof(float));
      
      /*
      ** Perform convolution on current scanline, function returns float_pixels
      ** would contain float values for pixels ranging from -large to +large 
      */ 
      Convolve( pixels, scanlines_pixels, kernel1, float_pixels );

      /*
      ** Sharpening is achieved by subtracting the laplacian filtered image
      ** scanline from the original image scanline.  -large to +large values.
      */

    switch( s_type )
      {      
    case ImgK_StypeMultispect :
      for(pixel = 0; pixel < pixels; pixel++, t_pos += sharp.s_pstr, 
                                                             d_pos += d_pstr )
       {
        /*
        ** sharpening is achieved by subtracting Laplacian filtered image from
        ** the original image
        */
        *( float_pixels + pixel ) = *( scanlines_pixels[1] + pixel+1 ) -
                                       *( float_pixels + pixel );

        /*
        ** Find luminance ratio of Y(new)/Y(old) ie ( -1 to +2 ) / ( 0 -- 1 )
        */
        if ( *( scanlines_pixels[1] + pixel+1) != 0 )
         ratio = *( float_pixels + pixel ) / *( scanlines_pixels[1] + pixel+1 );

        /*
        ** Multiply the pixel red comp with the luminance ratio 
        */
        value = ratio * ((float)GetField_(s_base, t_pos + sharp.s_off[RED],
                                                             sharp.s_msk[RED]));
        /*
        ** Evaluate the boundary values
        */
        if( value  > sharp.s_msk[RED] ) 
                 *( float_pixels + pixel) = sharp.s_msk[RED];
        else if( value < 0 ) 
                 *( float_pixels + pixel) = 0;
        else 
                 *( float_pixels + pixel) = value;

        /*
        ** Insert the red comp pixel value in dst fid
        */
        PutField_( d_base, d_pos + sharp.d_off[RED], sharp.d_msk[RED], 
                                ( int )( *( float_pixels + pixel )  + 0.5 ));
        /*
        ** Multiply the pixel grn comp with the luminance ratio 
        */
        value =  ratio * ((float)GetField_(s_base, t_pos + sharp.s_off[GRN],
                                                            sharp.s_msk[GRN])); 
        /*
        ** Evaluate the boundary values
        */
        if( value > sharp.s_msk[GRN] )
                  *( float_pixels + pixel ) = sharp.s_msk[GRN];
        else if( value < 0 ) 
                  *( float_pixels + pixel ) = 0;
        else 
                  *( float_pixels + pixel ) =  value;
   
        /*
        ** Insert the grn comp pixel value in dst fid
        */
        PutField_( d_base, d_pos + sharp.d_off[GRN], sharp.d_msk[GRN], 
                               ( int )( *( float_pixels + pixel )  + 0.5 ));

        /*
        ** Multiply the pixel blu comp with the luminance ratio 
        */
        value =  ratio * ((float)GetField_(s_base, t_pos + sharp.s_off[BLU], 
                                                            sharp.s_msk[BLU])); 
        /*
        ** Evaluate the boundary values
        */
        if( value  > sharp.s_msk[BLU] )
                  *( float_pixels + pixel ) = sharp.s_msk[BLU];
        else if( value < 0 ) 
                  *( float_pixels + pixel ) = 0;
        else 
                  *( float_pixels + pixel ) =  value;

        /*
        ** Insert the blu comp pixel value in dst fid
        */
         PutField_( d_base, d_pos + sharp.d_off[BLU], sharp.d_msk[BLU], 
                                  ( int )( *( float_pixels + pixel )  + 0.5 ));
       }
      break;
    case ImgK_StypeGrayscale :
      for(pixel = 0; pixel < pixels; pixel++, t_pos += sharp.s_pstr, 
                                                             d_pos += d_pstr )
       {
        *( float_pixels + pixel ) = *( scanlines_pixels[1] + pixel )   
                                      - *( float_pixels + pixel );
        /*
        ** Evaluate the boundary values
        */
        if ( *( float_pixels + pixel) > 1.0 )
                  *( float_pixels + pixel ) = 1.0 ;
        else if( *( float_pixels + pixel) < 0.0 )
                  *( float_pixels + pixel ) = 0.0;

        /*
        ** Put the new pixel value in the dest fid
        */
        PutField_(d_base, d_pos, d_mask, 
              ( int )( *( float_pixels +  pixel ) * max_pixel_out + 0.5 ));

       }
      break; 
      }/* end of switch */

     /*
     ** Clearthe array of temp float image pixels     
     */
      memset( float_pixels, (int) NULL, pixels );
     }

    /*
    ** Free the array of temp scanlines  pixels     
    */
     _ImgCfree( scanlines_pixels[0] );     
     _ImgCfree( scanlines_pixels[1] );     
     _ImgCfree( scanlines_pixels[2] );     

#ifdef TRACE
printf( "Leaving Routine IdsSharpen in module IDS_EXTENSIONS \n");
#endif

    return( dst_fid ); 
}

/*****************************************************************************
**  IdsDither
**
**  FUNCTIONAL DESCRIPTION:
**
**	Call IdsRequantize or ImgDither depending on the dither_algorithm.
**
**  FORMAL PARAMETERS:
**
**      fid     - image frame ID
**      attr    - attribute itemlist applied to new frame
**      algor	- dither/requantize algorithm
**      thresh	- dither threshold
**      roi     - region of interest identifier
**	levels	- ptr to the number of output levels per comp
**
**  FUNCTION VALUE:
**
**	newfid	- new image frame ID
**
*******************************************************************************/
unsigned long IdsDither( fid, attr, algor, thresh, roi, levels )
 unsigned long	    fid;		/* input frame			    */
 struct PUT_ITMLST  *attr;		/* attrib. for new frame	    */
 unsigned long	     algor;		/* dither/requantize algorithm	    */
 unsigned long	     thresh;		/* dither threshold		    */
 unsigned long 	    roi;		/* region of interest		    */
 unsigned long	    *levels;		/* number of output levels / comp   */
{
 unsigned long newfid;
 struct UDP src;
 int i, cnt;

#ifdef TRACE
printf( "Entering Routine IdsDither in module IDS_EXTENSIONS \n");
#endif


    /*
    **	Make sure scanline stride is valid, if specified in the 'attr' list.
    **	(A hack to solve floating point discrepencies between VAX and PMAX)
    */
   GetIsl_( fid, Img_NumberOfComp, cnt, 0);
    for(i = 0; i < cnt; ++i)
      SetScanlineStride( fid, attr, &src, roi, i );

    /*
    **	Call IdsRequantize or ImgDither depending on the dither algorithm.
    */
    if( algor == Ids_Requantize )
	newfid = IdsRequantize( fid, attr, roi, levels );
    else
	newfid = ImgDither( fid, attr, algor, thresh, 0, (struct ROI *)roi, levels );

#ifdef TRACE
printf( "Leaving Routine IdsDither in module IDS_EXTENSIONS \n");
#endif

    return( newfid );
}

/*****************************************************************************
**  IdsRequantize
**
**  FUNCTIONAL DESCRIPTION:
**
**	Re-quantize image pixel values to a (usually lesser) different number
**	of levels per spectral component.  For sake of IDS convenience we also
**	do spectral conversion from COLOR (IDS only accepts RGB) to GRAYSCALE.
**	Spectral conversion is specified by way of the item list "attr".
**
**  FORMAL PARAMETERS:
**
**      fid     - image frame ID
**      attr    - attribute itemlist applied to new frame
**      roi     - region of interest identifier
**	levels	- ptr to the number of output levels per comp
**
**  FUNCTION VALUE:
**
**	newfid	- new image frame ID
**
**  SIGNAL CODES:
**
**	ImgX_INVBITPXL	- invalid bits per pixel.
**      ImgX_INCFRMATT	- inconsistent frame attributes, old vs new.
**
*******************************************************************************/
unsigned long IdsRequantize( fid, attr, roi, levels )
 unsigned long	    fid;		/* input frame			    */
 struct PUT_ITMLST  *attr;		/* attrib. for new frame	    */
 unsigned long 	    roi;		/* region of interest		    */
 unsigned long	    *levels;		/* number of output levels / comp   */
{


    static RemapLutStruct lut = {8,0xff,256,0};
    static float RGB_mult[]   = {0.299,0.587,0.114};
    char  *s_base, *d_base;
    unsigned char  s_map[256], d_map[256];
    int s_pos, d_pos, s_cmps, d_cmps, s_lstr, d_lstr, s_type, d_type;
    int s_bpc[Ids_MaxComponents], d_bpc[Ids_MaxComponents];
    int d_bpp, r_bpp, levs, pixels, line, lines, i;
    ToneRequantizeDataStruct rq;
    unsigned long dst_fid;
    struct UDP  src, dst;
    int just_copy, copy_status;

#ifdef TRACE
printf( "Entering Routine IdsRequantize in module IDS_EXTENSIONS \n");
#endif

    /*
    **	Get the src UDP descriptor etc.
    */
    GetBitsPerComponent_( fid, s_cmps, s_bpc );
    GetIsl_( fid, Img_SpectType, s_type, 0 );
    if (s_type == ImgK_StypeBitonal) s_type = ImgK_StypeGrayscale;
    GetIsl_( fid, Img_Udp,	   src,	   0 );
    if( roi != 0 )
	_ImgSetRoi( &src, roi );
    rq.s_pstr = src.UdpL_PxlStride;
    s_lstr    = src.UdpL_ScnStride;
    pixels    = src.UdpL_X2 - src.UdpL_X1 + 1;
    lines     = src.UdpL_Y2 - src.UdpL_Y1 + 1;
    s_base    = (char *) src.UdpA_Base;
    s_pos     = src.UdpL_Pos;

    /*
    **	Create our dst Fid and retrieve its attributes.
    */
    dst_fid = CloneFid( fid, attr, roi );
    GetBitsPerComponent_( dst_fid, d_cmps, d_bpc );
    GetIsl_( dst_fid, Img_SpectType, d_type, 0 );
    if (d_type == ImgK_StypeBitonal) d_type = ImgK_StypeGrayscale;
    GetIsl_( dst_fid, Img_Udp,	       dst,    0 );
    rq.d_pstr = dst.UdpL_PxlStride;
    d_lstr    = dst.UdpL_ScnStride;
    d_base    = (char *) dst.UdpA_Base;
    d_pos     = dst.UdpL_Pos;

    for( d_bpp = 0, levs = 1, i = 0; i < d_cmps; i++ )
	{
	SetBitsPerComponent_(r_bpp, levels[i]);
	if( r_bpp != d_bpc[i] )
	    ChfStop (1,ImgX_INVBITPXL);
	d_bpp += r_bpp;
	levs  *= levels[i];
	}
     if( d_bpp  != dst.UdpW_PixelLength || levs < 1 || d_type > s_type)
	ChfStop (1,ImgX_INCFRMATT);

    /*
    **	Compute all the factors needed for the requantization.
    */
    just_copy = TRUE;
    for( i = 0; i < s_cmps; i++ )
	{
	rq.s_off[i] = i == 0 ? 0 : rq.s_off[i-1] + s_bpc[i-1];
	rq.d_off[i] = i == 0 ? 0 : rq.d_off[i-1] + d_bpc[i-1];
	rq.s_msk[i] = IMG_AL_ONES_ASCENDING[s_bpc[i]];
	rq.d_msk[i] = IMG_AL_ONES_ASCENDING[d_bpc[i]];
	/*
	**  If source spectral type is equal to destination spectral type, then
	**  no spectral class conversion is taking place. In this case, if the
	**  number of destination bits per component equals the number of source
	**  bits per component, simple shifting may be used. Otherwise, a 
	**  floating point scale/requantize is called for.
	**
	**  If spectral class conversion is taking place, convert RGB pixels to
	**  greyscale values.
	*/
	rq.mode[i]  = s_type == d_type ?
			(1<<d_bpc[i] == levels[i]) ?
			    (d_bpc[i] < s_bpc[i] ? Rshft : Lshft ) :
			    Fmult :
			F_RGB;

	switch( rq.mode[i] )
	  {
	  case Rshft :
	  case Lshft :
	    rq.shft[i] = abs(s_bpc[i] - d_bpc[i]);
	    if (rq.shft[i] != 0)
	      just_copy = FALSE;
	    break;
	  case Fmult :
	    rq.mult[i] = (float) (levels[i]-1) / rq.s_msk[i];
	    if (rq.mult[i] != 1.0)
	      just_copy = FALSE;
	    break;
	  case F_RGB :
	    rq.mult[i] = RGB_mult[i] * (levels[GRA]-1) / rq.s_msk[i];
	    if (rq.mult[i] != 1.0)
	      just_copy = FALSE;
	    break;
	  }
      }

    if (just_copy)
      {
	copy_status = _IpsCopy(&src,&dst,0);
	_ImgPut(dst_fid,(long)Img_Udp,&dst,(long)sizeof(dst),(long)0);
      }
    else
      if(( s_lstr % 8 | d_lstr % 8 | src.UdpL_Pos | dst.UdpL_Pos) == 0
	 && rq.s_pstr == 8 && rq.d_pstr == 8 )
	{
	  /*
	   **  Src and dst are byte aligned in byte size chunks, so use a LUT.
	   */
	  for( i = 0; i < 256; s_map[i] = i, d_map[i++] = 0 );
	  RequantizeScanline( &rq, 256, d_cmps, s_map, 0, d_map, 0 );
	  lut.base = d_map;
	  RemapByLUT( &src, &dst, &lut );
	}
      else
	/*
	 **  Requantize the hard way, a scanline at a time.
	 */
	for( line = 0; line++ < lines; s_pos = line * s_lstr + src.UdpL_Pos,
	    d_pos = line * d_lstr + dst.UdpL_Pos )
	  RequantizeScanline(&rq, pixels, d_cmps, s_base,s_pos, d_base,d_pos);

#ifdef TRACE
printf( "Leaving Routine IdsRequantize in module IDS_EXTENSIONS \n");
#endif

return( dst_fid ); 
}

/*******************************************************************************
**  IdsPutItem
**
**  FUNCTIONAL DESCRIPTION:
**
**	Append an ISL item list name/value/index triplet to a dynamic item list.
**	If the item list doesn't exist, one is allocated.
**
**  FORMAL PARAMETERS:
**
**	itmlst	- pointer to the address of the dynamic item list.
**	code 	- item code .
**	value	- item value.
**	index	- item index.
**
*******************************************************************************/
void IdsPutItem( itmlst, code, value, index )
 struct PUT_ITMLST **itmlst;
 unsigned long int code;
 unsigned long int value;
 unsigned long int index;
{
    int i;
    struct PUT_ITMLST *lst = *itmlst;

#ifdef TRACE
printf( "Entering Routine IdsPutItem in module IDS_EXTENSIONS \n");
#endif

    if( lst == NULL )
	/*
	**  We need to allocate an item list.
	*/
	lst = (struct PUT_ITMLST *) _ImgCalloc( 1, sizeof(struct PUT_ITMLST));

    /*
    **	Find the end of the item list.
    */
    for( i = 0; lst[i].PutL_Code != 0; i++ );

    /*
    **	Now stuff in the new entry.
    */
    lst[i].PutL_Code   = code;
    lst[i].PutL_Length = sizeof(long);
    lst[i].PutA_Buffer = (char *) _ImgCalloc(1, sizeof(long));
    lst[i].PutL_Index  = index;
    *(unsigned long int *)(lst[i].PutA_Buffer) = value;

    /*
    **	Append a terminator and return the new item list address.
    */
    *itmlst = (struct PUT_ITMLST *)
	_ImgRealloc( lst, (i + 2) * sizeof(struct PUT_ITMLST));
    (*itmlst)[i+1].PutL_Code   = (long) NULL;
    (*itmlst)[i+1].PutL_Length = (long) NULL;
    (*itmlst)[i+1].PutA_Buffer = NULL;
    (*itmlst)[i+1].PutL_Index  = (long) NULL;
    
#ifdef TRACE
printf( "Leaving Routine IdsPutItem in module IDS_EXTENSIONS \n");
#endif

}

/*******************************************************************************
**  IdsSetItmlst
**
**  FUNCTIONAL DESCRIPTION:
**
**	Append an ISL item list name/value/index triplet to a dynamic item list.
**	This routine is for the generic 5 element ISL ITMLST.
**	If the item list doesn't exist, one is allocated.
**
**  FORMAL PARAMETERS:
**
**	itmlst	- pointer to the address of the dynamic item list.
**	code 	- item code .
**	value	- item value.
**	index	- item index.
**
*******************************************************************************/
void IdsSetItmlst( itmlst, code, value, index )
 struct ITMLST **itmlst;
 unsigned long int code;
 unsigned long int value;
 unsigned long int index;
{
    int i;
    struct ITMLST *lst = *itmlst;

#ifdef TRACE
printf( "Entering Routine IdsSetItmlst in module IDS_EXTENSIONS \n");
#endif

    if( lst == NULL )
	/*
	**  We need to allocate an item list.
	*/
	lst = (struct ITMLST *) _ImgCalloc( 1, sizeof(struct ITMLST));

    /*
    **	Find the end of the item list.
    */
    for( i = 0; lst[i].ItmL_Code != 0; i++ );

    /*
    **	Now stuff in the new entry.
    */
    lst[i].ItmL_Code   = code;
    lst[i].ItmL_Length = sizeof(long);
    lst[i].ItmA_Buffer = (char *) _ImgCalloc(1, sizeof(long));
    lst[i].ItmA_Retlen = NULL;
    lst[i].ItmL_Index  = index;
    *(unsigned long int *)(lst[i].ItmA_Buffer) = value;

    /*
    **	Append a terminator and return the new item list address.
    */
    *itmlst = (struct ITMLST *)
	_ImgRealloc( lst, (i + 2) * sizeof(struct ITMLST));
    (*itmlst)[i+1].ItmL_Code   = (long) NULL;
    (*itmlst)[i+1].ItmL_Length = (long) NULL;
    (*itmlst)[i+1].ItmA_Buffer = NULL;
    (*itmlst)[i+1].ItmA_Retlen = NULL;
    (*itmlst)[i+1].ItmL_Index  = (long) NULL;
    
#ifdef TRACE
printf( "Leaving Routine IdsSetItmlst in module IDS_EXTENSIONS \n");
#endif

}

/*******************************************************************************
**  IdsFreePutList
**
**  FUNCTIONAL DESCRIPTION:
**
**	Deallocate a dynamic ISL Put item list.
**
**  FORMAL PARAMETERS:
**
**	itmlst	- pointer to the address of the dynamic put item list.
**
*******************************************************************************/
void IdsFreePutList( itmlst )
 struct PUT_ITMLST **itmlst;
{
    int i;

#ifdef TRACE
printf( "Entering Routine IdsFreePutList in module IDS_EXTENSIONS \n");
#endif

    if( *itmlst != NULL )
	{
	/*
	**  First free all the item value buffers.
	*/
	for( i = 0; (*itmlst)[i].PutA_Buffer != NULL; i++ )
	     _ImgCfree( (*itmlst)[i].PutA_Buffer );

	/*
	**  Now free the item list ...
	*/
	 _ImgCfree( *itmlst );
	/*
	**  ... and remove its pointer from our caller.
	*/
	*itmlst = NULL;
	}
#ifdef TRACE
printf( "Leaving Routine IdsFreePutList in module IDS_EXTENSIONS \n");
#endif
}

/*******************************************************************************
**  IdsFreeItmlst
**
**  FUNCTIONAL DESCRIPTION:
**
**	Deallocate a dynamic ISL ITMLST.
**
**  FORMAL PARAMETERS:
**
**	itmlst	- pointer to the address of the dynamic put item list.
**
*******************************************************************************/
void IdsFreeItmlst( itmlst )
 struct ITMLST **itmlst;
{
    int i;

#ifdef TRACE
printf( "Entering Routine IdsFreeItmlst in module IDS_EXTENSIONS \n");
#endif

    if( *itmlst != NULL )
	{
	/*
	**  First free all the item value buffers.
	*/
	for( i = 0; (*itmlst)[i].ItmA_Buffer != NULL; i++ )
	     _ImgCfree( (*itmlst)[i].ItmA_Buffer );

	/*
	**  Now free the item list ...
	*/
	 _ImgCfree( *itmlst );
	/*
	**  ... and remove its pointer from our caller.
	*/
	*itmlst = NULL;
	}
#ifdef TRACE
printf( "Leaving Routine IdsFreeItmlst in module IDS_EXTENSIONS \n");
#endif
}

/*****************************************************************************
**  RequantizeScanline
**
**  FUNCTIONAL DESCRIPTION:
**
**	Requantize a single scanline by component by pixel.
**
**  FORMAL PARAMETERS:
**
**      rq	- address of ToneRequantizeDataStruct
**	pixels	- number of pixels per scanline
**	comps	- number of components per pixel
**	s_base	- src base address of image
**	s_beg	- src offset to beginning of scanline
**	d_base	- dst base address of image
**	d_beg	- dst offset to beginning of scanline
**
*******************************************************************************/
static void RequantizeScanline(rq, pixels, comps, s_base, s_beg, d_base, d_beg)
 ToneRequantizeData	rq;
 int	 pixels;
 int	 comps;
 char	*s_base;
 int	 s_beg;
 char	*d_base;
 int	 d_beg;
{
    int s_pos, d_pos, pixel, comp;

#ifdef TRACE
printf( "Entering Routine RequantizeScanline in module IDS_EXTENSIONS \n");
#endif

    /*
    **	Processing on a component by component basis is more efficient since
    **	we only have to choose a requantization mode (switch statement) once
    **	for each component per scanline.
    */
    for( comp = 0; comp < comps; comp++ )
	{
	s_pos = s_beg + rq->s_off[comp];
	d_pos = d_beg + rq->d_off[comp];

	/*
	**  Pixel by pixel ...
	*/
	switch( rq->mode[comp] )
	    {
	case Rshft :
	    /*
	    **	Requantize using right shift.
	    */
	    for( pixel = 0; pixel < pixels; s_pos += rq->s_pstr,
					    d_pos += rq->d_pstr, pixel++ )
		PutField_(d_base,d_pos,rq->d_msk[comp],
		GetField_(s_base,s_pos,rq->s_msk[comp]) >> rq->shft[comp]);
		break;

	case Lshft :
	    /*
	    **	Requantize using left shift.
	    */
	    for( pixel = 0; pixel < pixels; s_pos += rq->s_pstr,
					    d_pos += rq->d_pstr, pixel++ )
		PutField_(d_base,d_pos,rq->d_msk[comp],
		GetField_(s_base,s_pos,rq->s_msk[comp]) << rq->shft[comp]);
		break;

	case Fmult :
	    /*
	    **	Requantize using floating multiplication with rounding.
	    */
	    for( pixel = 0; pixel < pixels; s_pos += rq->s_pstr,
					    d_pos += rq->d_pstr, pixel++ )
		PutField_(d_base,d_pos,rq->d_msk[comp], Round_(
		GetField_(s_base,s_pos,rq->s_msk[comp]) * rq->mult[comp]));
		break;

	case F_RGB :
	    /*
	    **	Requantize converting RGB to grayscale.
	    */
	    for( pixel = 0; pixel < pixels; s_pos += rq->s_pstr,
					    d_pos += rq->d_pstr, pixel++ )
		PutField_(d_base,d_pos,rq->d_msk[comp], Round_(
		GetField_(s_base,s_pos+rq->s_off[RED],rq->s_msk[RED])
						     * rq->mult[RED] +
		GetField_(s_base,s_pos+rq->s_off[GRN],rq->s_msk[GRN])
						     * rq->mult[GRN] +
		GetField_(s_base,s_pos+rq->s_off[BLU],rq->s_msk[BLU])
						     * rq->mult[BLU]));
		break;
	    }
	}
#ifdef TRACE
printf( "Leaving Routine RequantizeScanline in module IDS_EXTENSIONS \n");
#endif

}

/*****************************************************************************
**  RemapByLUT
**
**  FUNCTIONAL DESCRIPTION:
**
**	Remap image pixel values using supplied LUT.  Uses MOVTC if the LUT, 
**	src, and dst images are byte aligned and have byte size pixel strides.
**
**	When MOVTC is used, the entire pixel is remapped (including pad bits,
**	    if bits per pixel is less than pixel stride) -- CAVEAT EMPTOR --.
**
**  FORMAL PARAMETERS:
**
**      src	- src UDP desc
**      dst	- dst UDP desc
**	lut	- LUT descriptor
**
*******************************************************************************/
static void RemapByLUT( src, dst, lut )
 struct UDP    *src;
 struct UDP    *dst;
 RemapLut       lut;
{
    char  *s_base, *d_base;
    int s_pos, d_pos, s_mask, d_mask, s_pstr, d_pstr, s_lstr, d_lstr;
    int pixel, pixels, line, lines;

    s_pstr    = src->UdpL_PxlStride;
    s_lstr    = src->UdpL_ScnStride;
    pixels    = src->UdpL_X2 - src->UdpL_X1 + 1;
    lines     = src->UdpL_Y2 - src->UdpL_Y1 + 1;
    s_base    = (char *) src->UdpA_Base;
    s_pos     = src->UdpL_Pos;
    s_mask    = IMG_AL_ONES_ASCENDING[src->UdpW_PixelLength];
    d_pstr    = dst->UdpL_PxlStride;
    d_lstr    = dst->UdpL_ScnStride;
    d_base    = (char *) dst->UdpA_Base;
    d_pos     = dst->UdpL_Pos;
    d_mask    = IMG_AL_ONES_ASCENDING[dst->UdpW_PixelLength];

#ifdef TRACE
printf( "Entering Routine RemapByLUT in module IDS_EXTENSIONS \n");
#endif

    if( s_pstr == 8 && d_pstr == 8 && lut->stride == 8 &&
      ( s_lstr %  8  | d_lstr  % 8  | src->UdpL_Pos | dst->UdpL_Pos ) == 0 )
	for( line = 0; line++ < lines; s_pos = line * s_lstr + src->UdpL_Pos,
				       d_pos = line * d_lstr + dst->UdpL_Pos )
	    _IpsMovtcLong( pixels, s_base+(s_pos >> 3), 0, lut->base,
			     pixels, d_base+(d_pos >> 3));
    else
	for(line = 0; line++ < lines; s_pos = line * s_lstr + src->UdpL_Pos,
				      d_pos = line * d_lstr + dst->UdpL_Pos)
	    for(pixel = 0; pixel++ < pixels; s_pos += s_pstr, d_pos += d_pstr)
		PutField_(d_base, d_pos, d_mask,
		    GetField_(lut->base,
		    GetField_(s_base, s_pos, s_mask) * lut->stride, lut->mask));
#ifdef TRACE
printf( "Leaving Routine RemapByLUT in module IDS_EXTENSIONS \n");
#endif

}

/*****************************************************************************
**  RemapByTruePixels
**
**  FUNCTIONAL DESCRIPTION:
**
**	Remap image pixel values using true colors.  
**
**  FORMAL PARAMETERS:
**
**      src	- src UDP desc
**      dst	- dst UDP desc
**	lut	- LUT descriptor
**
*******************************************************************************/
static void RemapByTruePixels( tone, src, dst )
 ToneRequantizeData  tone;
 struct UDP    *src;
 struct UDP    *dst;
{
    char  *s_base, *d_base;
    int s_pos, d_pos, s_mask, d_mask, s_pstr, d_pstr, s_lstr, d_lstr;
    int pixel, pixels, line, lines;
    unsigned long red_val, grn_val, blu_val, tot_val;

#ifdef TRACE
printf( "Entering Routine RemapByTruePixels in module IDS_EXTENSIONS \n");
#endif

    s_pstr    = src->UdpL_PxlStride;
    s_lstr    = src->UdpL_ScnStride;
    pixels    = src->UdpL_X2 - src->UdpL_X1 + 1;
    lines     = src->UdpL_Y2 - src->UdpL_Y1 + 1;
    s_base    = (char *) src->UdpA_Base;
    s_pos     = src->UdpL_Pos;
    s_mask    = IMG_AL_ONES_ASCENDING[src->UdpW_PixelLength];
    d_pstr    = dst->UdpL_PxlStride;
    d_lstr    = dst->UdpL_ScnStride;
    d_base    = (char *) dst->UdpA_Base;
    d_pos     = dst->UdpL_Pos;
    d_mask    = IMG_AL_ONES_ASCENDING[dst->UdpW_PixelLength];

    /*
     ** Display of GrayScale image on true color workstation
     */
    /*
     ** In the case where the # of levels in the source is less than
     ** the # of levels of the presentation surface, each image pixel
     ** must be requantized by shifting the pixel val to the high bits
     ** of the pixel.
     ** The val of the red component is copied to the green and blue 
     ** components because grey is represented by equal levels of red
     ** green and blue.
     */
    if ((( s_lstr % 8 | d_lstr % 8 | s_pos | d_pos) == 0) &&
	(s_pstr == 8) && (d_pstr == 32) )
      {
	unsigned int i,tmpi;
	unsigned int TrueColor_gray_lut[256];

	unsigned char *src_pixel = (unsigned char *) (s_base + (s_pos >> 3));
	unsigned int  *dst_pixel = (unsigned int *) (d_base + (d_pos >> 5));
	int src_line_pad = (s_lstr - (pixels * s_pstr)) >> 3;
	int dst_line_pad = (d_lstr - (pixels * d_pstr)) >> 5;
	int shift_amt = 8 - BitsFromLevels_(src->UdpL_Levels);

	if (shift_amt == 0)
	  for (i = 0; i < src->UdpL_Levels; i++)
#ifdef sparc
	    TrueColor_gray_lut[i] = (i << 8) | (i << 16) | (i << 24);
#else
	    TrueColor_gray_lut[i] = i | (i << 8) | (i << 16);
#endif
	else
	  for (i = 0; i < src->UdpL_Levels; i++)
	    {
	      tmpi = i << shift_amt;
#ifdef sparc
	      TrueColor_gray_lut[i] = (tmpi << 8) | (tmpi << 16 ) | (tmpi << 24);
#else
	      TrueColor_gray_lut[i] = tmpi | (tmpi << 8 ) | (tmpi << 16);
#endif
	    }

	for (line = 0; 
	     line++ < lines;
	     src_pixel += src_line_pad, dst_pixel += dst_line_pad)
	    for (pixel = 0; 
		 pixel++ < pixels; 
		 src_pixel++, dst_pixel++)
	      *dst_pixel = TrueColor_gray_lut[*src_pixel];
      }
    else
      { 
	if( src->UdpL_Levels < 256 )
	  {
	    int shift_amt = 8 - BitsFromLevels_(src->UdpL_Levels);
	    for(line = 0; line++ < lines; s_pos = line * s_lstr + src->UdpL_Pos,
		d_pos = line * d_lstr + dst->UdpL_Pos)
	      for(pixel = 0; pixel++ < pixels; s_pos += s_pstr, d_pos += d_pstr)
		{
		  red_val = GetField_(s_base, s_pos, s_mask);
		  red_val <<= shift_amt;
		  grn_val = ( red_val << 8 );
		  blu_val = ( red_val << 16 );
		  tot_val = red_val | grn_val | blu_val;
		  PutField_(d_base, d_pos, d_mask, tot_val );
		}
	  }
	else
	  {
	    for(line = 0; line++ < lines; s_pos = line * s_lstr + src->UdpL_Pos,
		d_pos = line * d_lstr + dst->UdpL_Pos)
	      for(pixel = 0; pixel++ < pixels; s_pos += s_pstr, d_pos += d_pstr)
		{
		  red_val = GetField_(s_base, s_pos, s_mask);
		  grn_val = ( red_val << 8 );
		  blu_val = ( red_val << 16 );
		  tot_val = red_val | grn_val | blu_val;
		  PutField_(d_base, d_pos, d_mask, tot_val );
		}
	  }
      }
#ifdef TRACE
printf( "Leaving Routine RemapByTruePixels in module IDS_EXTENSIONS \n");
#endif
  }


/*****************************************************************************
**  ObtainScanlinePixels
**
**  FUNCTIONAL DESCRIPTION:
**
**    Obtains an array of float pixel values for one scan line
**
**  FORMAL PARAMETERS:
**
**      sharp     - ToneRequantize struct
**      src       - src UDP desc
**      scanline  - scanline number
**      s_pos     - pos at the beign of new scanline
**      s_type    - gray or multi spect
**
**  FUNCTION VALUE:
**
**      float_pixels - an array of float pixel values
**      
**
*******************************************************************************/
static float *ObtainScanlinePixels( sharp, src, scanline, s_pos, s_type )
 ToneRequantizeData  sharp;
 struct UDP         *src;
 int                 scanline;
 long int            s_pos;
 int                 s_type;
{
   char        *s_base;
   int          pixel, pixels, s_mask, s_pstr;
   int          max_pixel_int =  0;       /* 2 ** n where n is bits/pixel src */
   float       *scanline_pixels;
 
#ifdef TRACE
printf( "Entering Routine ObtainScanlinePixels in module IDS_EXTENSIONS \n");
#endif

   s_base    = (char *) src->UdpA_Base; 
   s_pstr    = src->UdpL_PxlStride;
   pixels    = src->UdpL_X2 - src->UdpL_X1 + 1;
   max_pixel_int = (1 << s_pstr) - 1;
   s_mask    = IMG_AL_ONES_ASCENDING[src->UdpW_PixelLength];
 
   /*
   ** Beside the pixels of image the scanline contains right and left pixels
   ** with values 0
   */ 

   scanline_pixels = (float *) _ImgCalloc(pixels + 2, sizeof(float)); 
   pixels++;                     /* helps to count 1 to x+1 instead of 0 to x */

    switch( s_type )
     {
    case ImgK_StypeGrayscale :
      if( !( scanline > src->UdpL_Y2 || scanline < src->UdpL_Y1 ) )
        {
         for(pixel = 1; pixel++ < pixels; s_pos += s_pstr )
          {
           *(scanline_pixels + pixel) =
                   ( (float)GetField_(s_base, s_pos, s_mask) / max_pixel_int );
          }
        }
      break;
    case ImgK_StypeMultispect :
      if( !( scanline > src->UdpL_Y2 || scanline < src->UdpL_Y1 ) )
      { 
        for(pixel = 1; pixel < pixels; pixel++, s_pos += s_pstr )
        {
         *(scanline_pixels + pixel) = ( (float)
               ( GetField_(s_base, s_pos + sharp->s_off[RED], sharp->s_msk[RED])
                                   * sharp->mult[RED] / sharp->s_msk[RED] ) +
               ( GetField_(s_base, s_pos + sharp->s_off[GRN], sharp->s_msk[GRN])
                                   * sharp->mult[GRN] / sharp->s_msk[GRN] ) +
               ( GetField_(s_base, s_pos + sharp->s_off[BLU], sharp->s_msk[BLU])
                                   * sharp->mult[BLU] / sharp->s_msk[BLU] ));
         }
      }      
      break;
     }/* end of switch */ 

#ifdef TRACE
printf( "Leaving Routine ObtainScanlinePixels in module IDS_EXTENSIONS \n");
#endif

   return( scanline_pixels );
}

/*****************************************************************************
**  Convolve
**
**  FUNCTIONAL DESCRIPTION:
**
**   The Property of the kernel is that the sum of their elements is zero.
**   This Laplacian when applied to an image produces large amplitudes at edge
**   loctions and zero in constant or uniformy varying regions. The amplitude
**   of each pixel in the scanline is calculated by convolving with the kernel
**   matrix multilplied by the beta factor. This beta factor controls the
**   affective amplitude of the edge information
**
**  FORMAL PARAMETERS:
**
**      pixels    - num of pixels in scanline
**      lines     - an array of three integer pixel value scan lines
**      kernel    - kernel used for convolution
**      pixelsret - ptr to float pixel values after convolution
**
**
*******************************************************************************/
static void Convolve( pixels, lines, kernel1, pixelsret )
 int          pixels;
 float       *lines[];
 char        *kernel1;
 float       *pixelsret;
{
   int          j,pixel, line;
   float       *temp, midval;
   float       *kernel, kerval;

#ifdef TRACE
printf( "Entering Routine Convolve in module IDS_EXTENSIONS \n");
#endif

  kernel  = (float *)kernel1;

   /*
   ** pixel by pixel
   */
   for( pixel = 1, line = 1 ; pixel <= pixels; pixel++, pixelsret++ )
    /*
    ** Computiation of pixel value to achieve sharpening
    */ 
    {           
     /*
     ** Compute the address of the next pixel,  0 0 position in 3 * 3 matrix
     */
     *pixelsret += *kernel * (*(lines[line-1] + pixel - 1));

     /*
     ** 0 1 position in 3 * 3 matrix
     */
      kernel++;
     *pixelsret += *kernel * (*(lines[line-1] + pixel));
 
     /*
     ** 0 2 position in 3 * 3 matrix
     */
     kernel++;
     *pixelsret += *kernel * (*(lines[line-1] + pixel + 1));

     /*
     ** 1 0 position in 3 * 3 matrix
     */
      kernel++;
     *pixelsret += *kernel * (*(lines[line]+ pixel-1));

     /*
     ** 1 1 position in 3 * 3 matrix
     */
      kernel++;
     *pixelsret += *kernel * (*(lines[line] + pixel));

     /*
     ** 1 2 position in 3 * 3 matrix
     */
      kernel++; 
     *pixelsret += *kernel * (*(lines[line] + pixel + 1));

     /*
     ** 2 0 position in 3 * 3 matrix
     */
      kernel++;
     *pixelsret += *kernel * (*(lines[line+1] + pixel - 1));

     /*
     ** 2 1 position in 3 * 3 matrix
     */ 
      kernel++; 
     *pixelsret += *kernel * (*(lines[line+1] + pixel));

     /*
     ** 2 2 position in 3 * 3 matrix
     */
      kernel++;  
     *pixelsret += *kernel * (*(lines[line+1] + pixel + 1));
      
     kernel  = (float *)kernel1;
    }      
#ifdef TRACE
printf( "Leaving Routine Convolve in module IDS_EXTENSIONS \n");
#endif

}

/*****************************************************************************
**  RemapByColorLUTs
**
**  FUNCTIONAL DESCRIPTION:
**
**	Remap image pixel values using supplied LUT.  Uses MOVTC if the LUT, 
**	src, and dst images are byte aligned and have byte size pixel strides.
**
**	When MOVTC is used, the entire pixel is remapped (including pad bits,
**	    if bits per pixel is less than pixel stride) -- CAVEAT EMPTOR --.
**
**  FORMAL PARAMETERS:
**
**      tone    - ToneRequantize struct
**      src	- src UDP desc
**      dst	- dst UDP desc
**	lut_red	- Red   LUT descriptor
**	lut_grn	- Green LUT descriptor
**	lut_blu	- Blue  LUT descriptor
**
*******************************************************************************/
static void RemapByColorLUTs( tone, src, dst, lut_red, lut_grn, lut_blu )
 ToneRequantizeData  tone;
 struct UDP         *src;
 struct UDP         *dst;
 RemapLut	     lut_red;
 RemapLut	     lut_grn;
 RemapLut	     lut_blu;
{
    char  *s_base, *d_base;
    int s_pos, d_pos;
    int pixel, pixels, line, lines;
    int value;
    int tmp;
    tone->s_pstr  = src->UdpL_PxlStride;             /* pixel stride        */
    tone->s_lstr  = src->UdpL_ScnStride;             /* scanline stride     */
    pixels        = src->UdpL_X2 - src->UdpL_X1 + 1; /* pixels per scanline */
    lines         = src->UdpL_Y2 - src->UdpL_Y1 + 1; /* scanlines in image  */
    s_base        = (char *) src->UdpA_Base; 
    s_pos         = src->UdpL_Pos;                    /* init bit pos offset*/
    tone->d_pstr  = dst->UdpL_PxlStride;
    tone->d_lstr  = dst->UdpL_ScnStride;
    d_base        = (char *) dst->UdpA_Base;
    d_pos         = dst->UdpL_Pos;

#ifdef TRACE
printf( "Entering Routine RemapByColorLUTs in module IDS_EXTENSIONS \n");
#endif

    /* for each scanline by scanline */ 
    for( line = 0; line++ < lines; s_pos = line * tone->s_lstr + src->UdpL_Pos,
  			         d_pos = line * tone->d_lstr + dst->UdpL_Pos)
     /* for each pixel by pixel */
     for( pixel = 0; pixel++ < pixels; s_pos += tone->s_pstr,
                                                         d_pos += tone->d_pstr)
      {
       /* 
       ** take the red comp offset bits of src fid, grab  the  entry in LUT
       ** from this offset and put this value in red comp offset of dest fid
       */
       tmp = GetField_(s_base, s_pos + tone->s_off[RED], tone->s_msk[RED]);
       tmp = GetField_(lut_red->base, tmp * lut_red->stride, lut_red->mask);
       PutField_( d_base, d_pos + tone->d_off[RED], tone->d_msk[RED], tmp);
       /* 
       ** take the grn comp offset bits of src fid, grab  the  entry in LUT
       ** from this offset and put this value in grn comp offset of dest fid
       */
       tmp = GetField_(s_base, s_pos + tone->s_off[GRN], tone->s_msk[GRN]);
       tmp = GetField_(lut_grn->base, tmp * lut_grn->stride, lut_grn->mask);
       PutField_( d_base, d_pos + tone->d_off[GRN], tone->d_msk[GRN], tmp);
       /* 
       ** take the blu comp offset bits of src fid, grab  the  entry in LUT
       ** from this offset and put this value in blu comp offset of dest fid
       */
       tmp = GetField_(s_base, s_pos + tone->s_off[BLU], tone->s_msk[BLU]);
       tmp = GetField_(lut_blu->base, tmp * lut_blu->stride, lut_blu->mask);
       PutField_( d_base, d_pos + tone->d_off[BLU], tone->d_msk[BLU], tmp);
      }
#ifdef TRACE
printf( "Leaving Routine RemapByColorLUTs in module IDS_EXTENSIONS \n");
#endif

}

/*****************************************************************************
**  CloneFid
**
**  FUNCTIONAL DESCRIPTION:
**
**	Clone a Fid for use by IdsRequantize routines.
**
**  FORMAL PARAMETERS:
**
**      fid            - image frame ID
**      attr           - attribute itemlist applied to new frame
**      roi            - region of interest identifier
**
**  FUNCTION VALUE:
**
**	dst_fid	    - ID of cloned image frame 
**
**  SIGNAL CODES:
**
**      ImgX_PXLTOOLRG	- bits_per_pixel or bits_per_component too large (>25).
**      ImgX_INCFRMATT	- inconsistent frame attributes, old vs new.
**
*******************************************************************************/
static unsigned long CloneFid( fid, attr, roi )
 unsigned long       fid;		/* input frame			    */
 struct PUT_ITMLST  *attr;		/* attrib. for new frame	    */
 unsigned long	     roi;		/* region of interest		    */
{
    int pixels, lines, stride, s_comps, d_comps, bpc[Ids_MaxComponents], i;
    unsigned long dst_fid;
    struct UDP   src, dst;

#ifdef TRACE
printf( "Entering Routine CloneFid in module IDS_EXTENSIONS \n");
#endif

    /*
    **	Verified source frame.
    */
    /*    IMG_VERIFY_FRAME_CONTENT_( _ImgCheckNormal( fid )); */

    /*
    **	Get the src UDP descriptor, handle ROI, and figure image dimensions.
    */
    GetIsl_( fid, Img_Udp, src, 0);
    if( roi != 0 )
	_ImgSetRoi( &src, roi );
    pixels = src.UdpL_X2 - src.UdpL_X1 + 1;
    lines  = src.UdpL_Y2 - src.UdpL_Y1 + 1;

    /*
    **	Clone a new frame structure.
    */
    dst_fid = _ImgCloneFrame( fid );
    if( roi != 0 )
	{
	/*
	**  Apply ROI to dst frame.
	*/
	stride = pixels * src.UdpL_PxlStride;
	_ImgPut( dst_fid, (long)Img_PixelsPerLine,  &pixels, (long)sizeof(int), (long)0);
	_ImgPut( dst_fid, (long)Img_NumberOfLines,  &lines,  (long)sizeof(int), (long)0);
	_ImgPut( dst_fid, (long)Img_ScanlineStride, &stride, (long)sizeof(int), (long)0);
	}
    if( attr != 0 )
	/*
	**  Apply caller's item list to dst frame.
	*/
	ImgSetFrameAttributes( dst_fid, attr );
    /*
    **	Test src and dst for reasonable bits_per_component (ie. <= 25).
    */
    GetBitsPerComponent_( fid, s_comps, bpc );
    for( i = 0; i < s_comps; ++i )
	if( bpc[i] > 25 )
	    ChfStop (1,ImgX_PXLTOOLRG);
    GetBitsPerComponent_( dst_fid, d_comps, bpc );
    for( i = 0; i < d_comps; ++i )
	if( bpc[i] > 25 )
	    ChfStop (1,ImgX_PXLTOOLRG);
    /*
    **	If the new fid has a lower number of components, we have erasing to do.
    */
    while( d_comps < s_comps )
	{
	_ImgErase( dst_fid, (long)Img_ImgBitsPerComp, (long)--s_comps );
        _ImgErase( dst_fid, (long)Img_QuantLevelsPerComp, (long)s_comps );
        }

    /*
    **	Make sure that the quantization levels in the dst frame
    **	correspond to the number of levels implied by the bits per
    **	comp attribute(s).  This is necessary because in some cases,
    **	the caller of CloneFid only passes in a change of bits per
    **	comp, and doesn't include a specific change to the number of
    **	levels.  (When IDS was first written, the Img_QuantLevelsPerComp
    **	attribute didn't exist.)
    */
    for ( i = 0; i < d_comps; ++i )
	{
	int bpc_tmp;	/* bits per comp	*/
	int lpc_tmp;	/* levels per comp	*/
	int qbpc_tmp;	/* quant bits per comp	*/
	_ImgGet( dst_fid, Img_ImgBitsPerComp, &bpc_tmp, sizeof(bpc_tmp),0,i);
	_ImgGet( dst_fid, Img_QuantBitsPerComp, &qbpc_tmp,sizeof(bpc_tmp),0,i);

	/*
	** If the bits per comp are less than the quant bits per comp
	** (i.e., the min. # of bits needed to represent the specified
	** number of quant levels), assume the stored quant levels are 
	** wrong and fix them by lowering them to (2 ** bpc_tmp).
	*/
	if ( bpc_tmp < qbpc_tmp )
	    {
	    lpc_tmp = 1 << bpc_tmp;
	    _ImgPut( dst_fid, Img_QuantLevelsPerComp, &lpc_tmp,
			sizeof(lpc_tmp), i );
	    }
	}

    /*
    **	Get the dst UDP descriptor and test for inconsistencies.
    **	    (This should stop on separate error codes.)
    */
    GetIsl_(dst_fid, Img_Udp, dst, 0);
    if( pixels *  dst.UdpL_PxlStride > dst.UdpL_ScnStride
     ||	pixels != dst.UdpL_X2 - dst.UdpL_X1 + 1
     || lines  != dst.UdpL_Y2 - dst.UdpL_Y1 + 1
     || dst.UdpW_PixelLength > dst.UdpL_PxlStride )
	ChfStop (1,ImgX_INCFRMATT);
    /*
    **	Allocate and store a data plane for the dst frame.
    */
    _ImgStoreDataPlane( dst_fid,
        _ImgAllocateDataPlane((lines*dst.UdpL_ScnStride+dst.UdpL_Pos+7)/8,0));

#ifdef TRACE
printf( "Leaving Routine CloneFid in module IDS_EXTENSIONS \n");
#endif

    return( dst_fid ); 
}

/*****************************************************************************
**  SetScanlineStride
**
**  FUNCTIONAL DESCRIPTION:
**
**	Apply ROI and compute scanline stride, if requested in the 'attr' list.
**
**  FORMAL PARAMETERS:
**
**      fid     - image frame ID
**      attr    - attribute itemlist applied to new frame
**      udp	- address of where to put image UDP
**      roi     - region of interest identifier
**
*******************************************************************************/
static void SetScanlineStride( fid, attr, udp, roi, scnt )
 unsigned long	    fid;		/* input frame			    */
 struct PUT_ITMLST  *attr;		/* attrib. for new frame	    */
 struct UDP         *udp;		/* where to place image UDP	    */
 unsigned long	    roi;		/* region of interest		    */
 int                 scnt;              /* component number                 */
{
    unsigned long pixels, stride, item, *align;

#ifdef TRACE
printf( "Entering Routine SetScanlineStride in module IDS_EXTENSIONS \n");
#endif

    /*
    **	Get src UDP, and handle ROI .
    */
    _ImgGet( fid, Img_Udp, udp, sizeof(struct UDP), 0, scnt );
    if( roi != 0 )
	_ImgSetRoi( udp, roi );
    pixels = udp->UdpL_X2 - udp->UdpL_X1 + 1;
    stride = udp->UdpL_PxlStride;

    if( attr != NULL )
	{
	/*
	**  Search thru attr list for PIXEL_STRIDE and SCANLINE_STRIDE.
	*/
	for( align = NULL, item = 0;  attr[item].PutL_Code != 0; ++item )
	    if( attr[item].PutL_Code == Img_PixelStride )
		stride = *(unsigned long *)(attr[item].PutA_Buffer);
	    else if( attr[item].PutL_Code == Img_ScanlineStride )
		align  =  (unsigned long *)(attr[item].PutA_Buffer);

	if( align != NULL )
	    {
	    /*
	    **	Replace the scanline pad modulo with the scanline stride.
	    */
	    stride *= pixels;
	    stride += AlignBits_(stride, *align);
	    *align  = stride;
	    }
	}
#ifdef TRACE
printf( "Leaving Routine SetScanlineStride in module IDS_EXTENSIONS \n");
#endif

}

/*****************************************************************************
**  IdsExportSixels
**
**  FUNCTIONAL DESCRIPTION:
**
**      Export either color or bitonal sixels based on spectral type of input
**	image.
**
**  FORMAL PARAMETERS:
**
**      fid	    ISL image frame identifier of image to be exported.
**	roi	    Optional region of interest. If value zero, the entire 
**		    image is exported.
**	bufadr	    Address of buffer to receive sixel data.
**	buflen	    Length of buffer to receive sixel data.
**	bytcnt	    Count of total bytes returned to caller.
**	flags	    Processing flags.
**	action	    Application action routine.
**	usrprm	    Application action routine parameter.
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
**      fid - frame id which was exported from
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
*****************************************************************************/
unsigned long	 IdsExportSixels(fid,roi,bufadr,buflen,bytcnt,flags,action,usrprm)
unsigned long	 fid;
unsigned long	 roi;
char	*bufadr;
int	 buflen;
int	*bytcnt;
int	 flags;
long	(*action)();
long	 usrprm;
{
    int spectral_type;
#ifdef NODAS_PROTO
    unsigned long IdsExportHrColorSixels();
#endif

#ifdef TRACE
printf( "Entering Routine IdsExportSixels in module IDS_EXTENSIONS \n");
#endif

    GetIsl_(fid,Img_SpectType,spectral_type,0);
    switch (spectral_type)
    {
    case ImgK_StypeBitonal:
	return(ImgExportSixels(fid,(struct ROI *)roi,bufadr,buflen,bytcnt,flags,action,
				 usrprm));
	break;
    case ImgK_StypeMultispect:
        /* LJ250 - low res or high res ? */
	if (flags == Ids_Lj250lr_Mode)
	    return(IdsExportLrColorSixels(fid,roi,bufadr,buflen,bytcnt,flags,
			    action,usrprm));
	else
	    return(IdsExportHrColorSixels(fid,roi,bufadr,buflen,bytcnt,flags,
			    action,usrprm));
	break;
    default:
	ChfSignal(1,ImgX_UNSSPCTYP);
	return(fid);
    }
#ifdef TRACE
printf( "Leaving Routine IdsExportSixels in module IDS_EXTENSIONS \n");
#endif

}
