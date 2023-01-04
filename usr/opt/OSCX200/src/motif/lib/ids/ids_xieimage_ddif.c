
/***************************************************************************
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

/*******************************************************************************
**
**  FACILITY:
**
**      X Image Extension (XIE)
**
**  ABSTRACT:
**
**	This module contains XIE / ISL image format translation routines.
**
**  ENVIRONMENT:
**
**      VAX/VMS V5.3
**	ULTRIX  V3.1
**
**  AUTHOR(S):
**
**      Subu Garikapati
**      Robert NC Shelley
**
**  CREATION DATE:
**
**	September 13, 1989
**
**  MODIFICATION HISTORY:
**
*******************************************************************************/

/*
**  Definition to let include files know who's calling.
*/
#define _XieDDIF

/*
**  Include files
*/
    /*
    **  Standard C include files
    */
#include <errno.h>
#include <stdio.h>
    /*
    **  X11 and XIE include files
    */
#include    <ids__widget.h> /* IDS public/private, Dwtoolkit, Xlib defs.    */
    /*
    **  ISL include files
    */
#include <img/ImgDef.h>
#include <img/ImgEntry.h>
#ifndef NODAS_PROTO
#include <idsprot.h>            /* IDS prototypes */
#endif


/*
**  Table of contents
*/
#ifdef NODAS_PROTO
unsigned long   IdsDecToXieImage();   /* create XieImage from a DDIF file   */
unsigned long   IdsFidToXieImage();   /* create XieImage from an ISL fid    */
unsigned long   IdsPhotoToXieImage(); /* create XieImage from server photo  */
void            IdsPhotoToDec();      /* create DDIFF file from server photo*/
unsigned long   IdsXieImageToFid();   /* create ISL fid from XIE XieImage   */
unsigned long   IdsXieImageToDec();   /* create DDIF file from XIE XieImage */

DataForXie      IdsDataForXie();    /* allocate and get box dimensions      */
void            CollectRoiData();   /* Collect ISL ROI attribut */
unsigned long   ConvertRenderedFid(); /* convert fid as the way the appl spec*/
#endif

/*
**  Macro definitions
*/
    /*
    **  Simplify calling ImgGet(...)
    */
#define ImgGet_(fid,item,buffer,index) \
            ImgGet((fid),(item),(char *)&(buffer),(sizeof(buffer)),0,(index))


/*
**  Equated symbols
*/

/*
**  Externals
*/
XieImage XieCreateImage();
/*
**  Local storage
*/

/*******************************************************************************
**  IdsDecToXieImage
**
**  FUNCTIONAL DESCRIPTION:
**
**	Create an XieImageRec from a DDIF file.
**
**  FORMAL PARAMETERS:
**
**	name	- file name string
**
**  FUNCTION VALUE:
**
**	XieImage - pointer to XieImageRec
**
*******************************************************************************/
unsigned long IdsDecToXieImage(name )
 char *name;
{
    unsigned long ctx, fid;
    XieImage img;

#ifdef TRACE
printf( "Entering Routine IdsDecToXieImage in module IDS_XIEIMAGE_DDIF \n");
#endif
    /*
    **	Get image from DDIF stream file.
    */
    ctx = ImgOpenDDIFFile( ImgK_ModeImport, strlen(name), name, 0, 0, 0 );
    fid = ImgImportDDIFFrame( ctx, 0, 0, 0, 0, 0 );

    /*
    **	Create an XieImage from the fid.
    */
    img = (XieImage) IdsFidToXieImage( fid, True );

    ImgDeleteFrame( fid );
    ImgCloseDDIFFile( ctx, 0 );

#ifdef TRACE
printf( "Leaving Routine IdsDecToXieImage in module IDS_XIEIMAGE_DDIF \n");
#endif
    return (unsigned long) img;
}				    /* end IdsDecToXieImage*/

/*******************************************************************************
**  IdsFidToXieImage
**
**  FUNCTIONAL DESCRIPTION:
**
**	Create an XieImageRec from an ISL image frame.
**
**  FORMAL PARAMETERS:
**
**	fid	- ISL image frame id
**	copy	- Boolean: TRUE == copy image data, FALSE == use image pointer.
**
**  FUNCTION VALUE:
**
**	XieImage - pointer to XieImageRec
**
*******************************************************************************/
unsigned long IdsFidToXieImage(fid, copy )
 unsigned long	fid;
 unsigned char	copy;
{
    unsigned char cmpres, cmp_org, cmp_map, cmp_cnt;
    unsigned char *data, cmp_len[XieK_MaxComponents];
    unsigned long  i, pxl_bits, cmp_lvl[XieK_MaxComponents];
    unsigned long  numerator, denominator, width, height;
    unsigned long  imgCmpres, imgCmpOrg, imgCmpMap, imgCmpCnt, imgCmpLen;
    unsigned long  imgCmpLev;
    unsigned long  imgPxlProg, imgScnProg, imgPxlPol;
    XieImage img;

#ifdef TRACE
printf( "Entering Routine IdsFidToXieImage in module IDS_XIEIMAGE_DDIF \n");
#endif
    /*
    **	Get image dimension information.
    */
    ImgGet_( fid, Img_PixelsPerLine, width,       0 );
    ImgGet_( fid, Img_NumberOfLines, height,      0 );

    /*
    **	Get per-spectral-component information.
    */
    ImgGet_( fid, Img_NumberOfComp, imgCmpCnt, 0 );
    cmp_cnt = imgCmpCnt;

    for( pxl_bits = 0, i = 0; i < cmp_cnt; i++ )
	{
	ImgGet_( fid, Img_QuantLevelsPerComp, imgCmpLev, i );
	ImgGet_( fid, Img_ImgBitsPerComp, imgCmpLen, i );
	cmp_lvl[i] =    imgCmpLev;
	cmp_len[i] =    imgCmpLen;
	pxl_bits  +=    imgCmpLen;
	}

    /*
    **	Convert spectral component mapping.
    */
    ImgGet_( fid, Img_SpectralMapping, imgCmpMap, 0 );
    switch( imgCmpMap )
	{
    case ImgK_MonochromeMap :
        cmp_map = pxl_bits == 1 ? XieK_Bitonal : XieK_GrayScale;
	break;

    case ImgK_RGBMap :
	cmp_map = XieK_RGB;
	break;

    default : 
           /*
           ** Some time later this PixAllErr has to be named properly
           */
           XtWarningMsg("PixAllErr","XieImageFromFid","IdsImageError",
                         " (Img_SpectralMapping)",0,0);
	}

    /*
    **	Convert component space organization.
    */
    ImgGet_( fid, Img_CompSpaceOrg, imgCmpOrg, 0 );
    switch( imgCmpOrg )
	{
    case ImgK_OrgFulPxlCmp :
	cmp_org = XieK_BandByPixel;
	break;

    case ImgK_OrgParPxlExp :
	cmp_org = XieK_BandByPlane;
	break;

    case ImgK_OrgFulPxlExp :
	cmp_org = XieK_BitByPlane;
	break;

    default : 
           XtWarningMsg("PixAllErr","XieImageFromFid","IdsImageError",
                         "(Img_CompSpaceOrg)",0,0);
	}

    /*
    **	Convert compression type to compression scheme.
    */
    ImgGet_( fid, Img_CompressionType, imgCmpres, 0 );
    switch( imgCmpres )
	{
    case ImgK_PcmCompression :
	cmpres = XieK_PCM;
	break;

    case ImgK_G31dCompression :
	cmpres = XieK_G31D;
	break;

    case ImgK_G32dCompression :
	cmpres = XieK_G32D;
	break;

    case ImgK_G42dCompression :
	cmpres = XieK_G42D;
	break;

    case ImgK_DctCompression:
	cmpres = XieK_DCT;
	break;

    default : 
           XtWarningMsg("PixAllErr","XieImageFromFid","IdsImageError",
                         "(Img_CompressionType)",0,0);
	}

    /*
    **	Create XieImage structures per our specifications.
    */
    img = XieCreateImage( width, height,
			  cmpres, cmp_org, cmp_map, cmp_cnt, cmp_lvl, 0, 0 );

    /*
    **	Convert pixel path to pixel progression.
    */
    ImgGet_( fid, Img_PixelPath, imgPxlProg, 0 );
    if( imgPxlProg == 0 )
	PxlProg_(img) = XieK_PP0;

    else if( imgPxlProg == 90 )
	PxlProg_(img) = XieK_PP90;

    else if( imgPxlProg == 180 )
	PxlProg_(img) = XieK_PP180;

    else if( imgPxlProg == 270 )
	PxlProg_(img) = XieK_PP270;

    else
	XtWarningMsg("PixAllErr","XieImageFromFid","IdsImageError",
                         "(Img_PixelPath)",0,0);


    /*
    **	Convert line progression.
    */
    ImgGet_( fid, Img_LineProgression, imgScnProg, 0 );
    if( imgScnProg == 90 )
	ScnProg_(img) = XieK_LP90;

    else if( imgScnProg == 270 )
	ScnProg_(img) = XieK_LP270;

    else
	XtWarningMsg("PixAllErr","XieImageFromFid","IdsImageError",
                         "(Img_LineProgression)",0,0);

    /*
    **	Convert brightness polarity.
    */
    ImgGet_( fid, Img_BrtPolarity, imgPxlPol, 0 );
    switch( imgPxlPol )
	{
    case ImgK_ZeroMaxIntensity :
	PxlPol_(img) = XieK_ZeroBright;
	break;

    case ImgK_ZeroMinIntensity :
	PxlPol_(img) = XieK_ZeroDark;
	break;

    default : 
           XtWarningMsg("PixAllErr","XieImageFromFid","IdsImageError",
                         "(Img_BrtPolarity)",0,0);
	}

    /*
    **	Get image pixel aspect ratio.
    */
    ImgGet_( fid, Img_PPPixelDist,   numerator,   0 );
    ImgGet_( fid, Img_LPPixelDist,   denominator, 0 );

    if( numerator == 0  ||  denominator == 0 )
           XtWarningMsg("PixAllErr","XieImageFromFid","IdsImageError",
                         "(Img_PPPixelDist or Img_LPPixelDist)",0,0);

    PxlRatio_(img) = (double) numerator / denominator;

    /*
    **	Fill in the per-plane information.
    */
    for( i = 0; i < XieK_MaxPlanes; i++ )
	if( Plane_(img,i) != NULL )
	    {
	    ImgGet_( fid, Img_PixelStride,    uPxlStr_(img,i), i );
	    ImgGet_( fid, Img_ScanlineStride, uScnStr_(img,i), i );
	    ImgGet_( fid, Img_DataOffset,     uPos_(img,i),    i );
	    ImgGet_( fid, Img_PlaneDataBase,  data,	       i );
	    if( Cmpres_(img) == XieK_PCM )
		uArSize_(img,i) = uPos_(img,i) + uScnStr_(img,i) * height ;
	    else
		{
		ImgGet_( fid, Img_CdpArsize,  uArSize_(img,i), i );
		uArSize_(img,i) = uArSize_(img,i) * 8; /* in bits */
		}

	    if( copy )
		{
		uBase_(img,i) = XieMallocBits( uArSize_(img,i) );
		memcpy( uBase_(img,i), data, uArSize_(img,i) +  7 >> 3 );
		}
	    else
		uBase_(img,i) = data;
	    }
    OwnData_(img) = copy;

#ifdef TRACE
printf( "Leaving Routine IdsFidToXieImage in module IDS_XIEIMAGE_DDIF \n");
#endif
    return (unsigned long) img;
}				    /* end IdsFidToXieImage*/

/*****************************************************************************
**  IdsPhotoToXieImage()
**
**  FUNCTIONAL DESCRIPTION:
**
**   The Compression parameter is used by the DCT compression scheme only and
**   is chosen as 100.
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
**
*****************************************************************************/
unsigned long IdsPhotoToXieImage(map,cmpres,cmap)
    XiePhotomap	    map;
    unsigned long   cmpres;
    unsigned long   cmap;
{
     XieImage      img;
     unsigned long cpar = 100; /*compression parameter for DCT cimages   */

#ifdef TRACE
printf( "Entering Routine IdsPhotoToXieImage in module IDS_XIEIMAGE_DDIF \n");
#endif

     switch( cmpres )
        {
     case Ids_UnCompress :
	switch( cmap )
	    {
	case Ids_BandByPixel :	    
	    img = XieGetImage( map, XieK_PCM, 0, XieK_BandByPixel, 0, 0, 
	    							XieK_DataCopy );
	    break;
	case Ids_BandByPlane :	    
          img = XieGetImage( map, XieK_PCM, 0, XieK_BandByPlane, 0, 0, 
								XieK_DataCopy );
	  break;  
        case Ids_BitByPlane  :
          img = XieGetImage( map, XieK_PCM, 0, XieK_BitByPlane, 0, 0, 
								XieK_DataCopy );
	  break;
	default  :
	  printf("\t not implemented\n");
	  break;
	    }

     case Ids_CompressG42D :
	switch( cmap )
	    {
	case Ids_BandByPixel :	    
          img = XieGetImage( map, XieK_G42D, 0, XieK_BandByPixel, 0, 0, 
								XieK_DataCopy );
	  if( !StreamFinal_(img) )
	    printf("\t Photomap did not compress\n");
	  break;

        case Ids_BandByPlane :
          img = XieGetImage( map, XieK_G42D, 0, XieK_BandByPlane, 0, 0, 
								XieK_DataCopy );
	  if( !StreamFinal_(img) )
	    printf("\t Photomap did not compress\n");
	  break;

        case Ids_BitByPlane :
          img = XieGetImage( map, XieK_G42D, 0, XieK_BitByPlane, 0, 0, 
								XieK_DataCopy );
	  if( !StreamFinal_(img) )
	    printf("\t Photomap did not compress\n");
	  break;
	    
	default  :
        printf("\t not implemented\n");
	break;
	    }
     case Ids_CompressDCT :
	switch( cmap )
	    {
	case Ids_BandByPixel :	    
          img = XieGetImage( map, XieK_G42D, cpar, XieK_BandByPixel, 0, 0, 
								XieK_DataCopy );
	  if( !StreamFinal_(img) )
	    printf("\t Photomap did not compress\n");
	  break;

        case Ids_BandByPlane :
          img = XieGetImage( map, XieK_G42D, cpar, XieK_BandByPlane, 0, 0, 
								XieK_DataCopy );
	  if( !StreamFinal_(img) )
	    printf("\t Photomap did not compress\n");
	  break;

        case Ids_BitByPlane :
        printf("\t BitByPlane compress image not implemented\n");
	break;

	default  :
        printf("\t not implemented\n");
	break;
	    }    
     default :
        printf("\t not implemented\n");
	break;
     }
#ifdef TRACE
printf( "Leaving Routine IdsPhotoToXieImage in module IDS_XIEIMAGE_DDIF \n");
#endif
    return (unsigned long) img;
}

/*****************************************************************************
**  IdsPhotoToFid()
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
**
*****************************************************************************/
unsigned long  IdsPhotoToFid(map,cmpres,cmap)
    XiePhotomap	    map;
    unsigned long   cmpres;
    unsigned long   cmap;
{
    unsigned long   fid;
    XieImage	    img;

#ifdef TRACE
printf( "Entering Routine IdsPhotoToFid in module IDS_XIEIMAGE_DDIF \n");
#endif
    /*
    ** transport server photomap data to client's xieimage
    */
    img = (XieImage) IdsPhotoToXieImage(map,cmpres,cmap);
    /*
    ** Convert xieimage to fid 
    */
    fid = IdsXieImageToFid(img);
    /*
    ** Free the xieimage
    */
    XieFreeImage( img );

#ifdef TRACE
printf( "Leaving Routine IdsPhotoToFid in module IDS_XIEIMAGE_DDIF \n");
#endif
    return(fid);
}

/*****************************************************************************
**  IdsXieImageToFid()
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
**
*****************************************************************************/
unsigned long   IdsXieImageToFid(img)
    XieImage	    img;
{
    struct ITMLST       itmlst[25];
    unsigned short      index;
    unsigned long  i, pxl_bits, output_class, flag;
    int                 sizeint  = sizeof(int);
    int                 data_plane_size;
    unsigned long       cmpres, cmp_org, cmp_map, cmp_cnt, 
			imgPxlPol, imgPxlProg, imgScnProg;
    unsigned long       fid;
    char               *dp;

#ifdef TRACE
printf( "Entering Routine IdsXieImageToFid in module IDS_XIEIMAGE_DDIF \n");
#endif
    index = 0;
    INIT_ITM_(itmlst, index, Img_PixelsPerLine, sizeint, &Width_(img), 0, 0);
    INIT_ITM_(itmlst, index, Img_NumberOfLines, sizeint, &Height_(img), 0, 0);
    for( pxl_bits = 0, i = 0; i < CmpCnt_(img); i++ )
        {
	INIT_ITM_(itmlst, index,  Img_QuantLevelsPerComp, sizeint,
						  CmpLvl_(img,i), 0, i);
	INIT_ITM_(itmlst, index,  Img_ImgBitsPerComp, sizeint,
						  CmpLen_(img,i), 0, i);
        pxl_bits  +=     CmpLen_(img,i);
        }
    /*
    **  Convert spectral component mapping.
    */
    switch( CmpMap_(img) )
        {
    case XieK_Bitonal   :
	output_class =  ImgK_ClassBitonal;
	cmp_map      = ImgK_MonochromeMap;

    case XieK_GrayScale :
	output_class =   ImgK_ClassMultispect;
	cmp_map      =   ImgK_MonochromeMap;
        break;

    case  XieK_RGB :
	output_class =   ImgK_ClassMultispect;
	cmp_map      =   Img_SpectralMapping;
        break;
             
   default :
           /*
           ** Some time later this PixAllErr has to be named properly
           */
           XtWarningMsg("PixAllErr","XieImageFromFid","IdsImageError",
                         " (Img_SpectralMapping)",0,0);
        }
   INIT_ITM_(itmlst, index, Img_SpectralMapping, sizeint, &cmp_map, 0, 0);
        
    /*
    **  Convert component space mapping.
    */
    switch( CmpOrg_(img) )
        {
    case XieK_BandByPixel :
	cmp_org  =  ImgK_OrgFulPxlCmp;
        break;

    case XieK_BandByPlane :
	cmp_org  =  ImgK_OrgParPxlExp;
        break;

    case XieK_BitByPlane  :
	cmp_org  =  ImgK_OrgFulPxlExp;
        break;

    default :
           XtWarningMsg("PixAllErr","XieImageFromFid","IdsImageError",
                         "(Img_CompSpaceOrg)",0,0);
        }
    INIT_ITM_(itmlst, index, Img_CompSpaceOrg,sizeint,
						 &cmp_org, 0, 0);

    /*
    **  Convert compression type to compression scheme.
    */
    switch( Cmpres_(img) )
        {
    case  XieK_PCM :
        break;

    case  XieK_G42D :
	if( CmpOrg_(img) == XieK_BandByPlane )
	    cmpres = ImgK_G42dCompression;
	else
           XtWarningMsg("NotSupported","XieImageFromFid","IdsImageError",
                         "(Img_CompressionType)",0,0);
        break;

    case XieK_DCT :
	if( CmpOrg_(img) == XieK_BandByPlane )
	    cmpres = ImgK_DctCompression;
	else
           XtWarningMsg("NotSupported","XieImageFromFid","IdsImageError",
                         "(Img_CompressionType)",0,0);
        break;

    default :
           XtWarningMsg("PixAllErr","XieImageFromFid","IdsImageError",
                         "(Img_CompressionType)",0,0);
        }
    INIT_ITM_(itmlst, index, Img_CompressionType, sizeint, &cmpres, 0, 0);

    /*
    **  Convert pixel path to pixel progression.
    */
    switch( PxlProg_(img) )
        {
    case  XieK_PP0 :
        imgPxlProg = 0;
        break;

    case  XieK_PP90 :
        imgPxlProg = 90;
        break;

    case  XieK_PP180 :
        imgPxlProg = 180;
        break;

    case  XieK_PP270 :
        imgPxlProg = 270;
        break;

    default :
        XtWarningMsg("PixAllErr","XieImageFromFid","IdsImageError",
                         "(Img_PixelPath)",0,0);
        }
    INIT_ITM_(itmlst, index,  Img_PixelPath, sizeint, & imgPxlProg, 0, 0);
      
    /*
    **  Convert line progression.
    */
    switch( ScnProg_(img) )
        {
    case  XieK_LP90:
	imgScnProg =  90;
        break;

    case  XieK_LP270 :
	imgScnProg =  270;
        break;

    default :
           XtWarningMsg("PixAllErr","XieImageFromFid","IdsImageError",
                         "(Img_BrtPolarity)",0,0);
        }
    INIT_ITM_(itmlst, index,  Img_BrtPolarity , sizeint, &imgPxlPol, 0, 0);

   switch( imgPxlPol )
        {
    case  XieK_ZeroBright :
        imgPxlPol = ImgK_ZeroMaxIntensity;
        break;

    case XieK_ZeroDark :
        imgPxlPol = ImgK_ZeroMinIntensity;
        break;

    default :
           XtWarningMsg("PixAllErr","XieImageFromFid","IdsImageError",
                         "(Img_BrtPolarity)",0,0);
        }
    INIT_ITM_(itmlst, index,  Img_BrtPolarity , sizeint, &imgPxlPol, 0, 0);
      
    /*
    **  Get image pixel aspect ratio.
    */
    INIT_ITM_(itmlst, index, Img_PxlAspectRatio, sizeint, &PxlRatio_(img),0,0);

    /*
    ** Pixel stride, scanline stride, offset
    */
    for( i = 0; i < CmpCnt_(img); i++ )
        {
        INIT_ITM_(itmlst, index, Img_PixelStride, sizeint, 
						     &uPxlStr_(img,i), 0, i);
        INIT_ITM_(itmlst, index,  Img_ScanlineStride, sizeint, 
						     & uScnStr_(img,i), 0, i);
        INIT_ITM_(itmlst, index,  Img_DataOffset, sizeint, 
						     &uPos_(img,i), 0, i);
        }
    END_ITEM_(itmlst,index);

    flag = ImgM_NoDataPlaneAlloc;
    fid = ImgAllocateFrame(output_class,itmlst,0,flag);
   
    /*
    **  Fill in the per-plane information.
    */
   for( i = 0; i < XieK_MaxPlanes; i++ )
        if( Plane_(img,i) != NULL )
            {
	    data_plane_size = (uArSize_(img,i) +  7 >> 3);
	    dp = (char *)ImgAllocateDataPlane(data_plane_size,0);
            memcpy( dp, uBase_(img,i), data_plane_size );
	    fid = ImgAttachDataPlane( fid, dp, i );	   
	    }

#ifdef TRACE
printf( "Leaving Routine IdsXieImageToFid in module IDS_XIEIMAGE_DDIF \n");
#endif
    return(fid);
}	    

/*****************************************************************************
**   ConvertRenderedFid()
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
**
*****************************************************************************/
unsigned long    ConvertRenderedFid(fid,cmpres,cmap)
    unsigned long   fid;
    unsigned long   cmpres;
    unsigned long   cmap;
{
    unsigned long   ret_fid;
    unsigned long   new_cs;
    unsigned long   old_cs;
    unsigned long   data_class;


#ifdef TRACE
printf( "Entering Routine ConvertRenderedFid in module IDS_XIEIMAGE_DDIF \n");
#endif
    switch( cmpres )
        {
    case Ids_UnCompress :
	    /*
	    ** If the data is uncompressed, see if we REALLY have to 
	    ** convert the comp space org (i.e., reinterleave the data), or
	    ** see if all we really have to do is set the attribute in
	    ** the fid to make the caller happy (i.e., make it think
	    ** the frame is in the org. that it asked for).
	    **
	    **	Bitonal data never has to be reinterleaved, and greyscale
	    **	data never unless the output org is bit by plane.
	    */
	    _ImgGet( fid, Img_CompSpaceOrg, &old_cs, sizeof(old_cs), 0, 0 );
	    _ImgGet( fid, Img_ImageDataClass, &data_class, sizeof(data_class),
			0, 0 );

	    if ( (data_class == ImgK_ClassBitonal ) ||
		 (data_class == ImgK_ClassGrayscale && 
		  old_cs != ImgK_BitIntrlvdByPlane && 
		  cmap == ImgK_BitIntrlvdByPlane)  ||
		 (data_class == ImgK_ClassMultispect &&
		  old_cs == cmap) )
		{
		/*
		** Don't have to reinterleave it ... it's sufficient
		** just to change the csorg attribute.
		*/
		_ImgPut( fid, Img_CompSpaceOrg, &cmap, sizeof(cmap), 0 );
		ret_fid = fid;
		}
	    else
		{
		/*
		** Force a reinterleave of the data ...
		*/
		switch( cmap )
		    {
		case Ids_BandByPixel :	    
		    ret_fid = ImgCvtCompSpaceOrg( fid, ImgK_BandIntrlvdByPixel,
							ImgK_LsbitFirst, 0);
		    break;
		case Ids_BandByPlane :	    
		    ret_fid = ImgCvtCompSpaceOrg( fid, ImgK_BandIntrlvdByPlane,
							ImgK_LsbitFirst, 0);
		    break;  
		case Ids_BitByPlane  :
		    ret_fid = ImgCvtCompSpaceOrg( fid, ImgK_BitIntrlvdByPlane,
							ImgK_LsbitFirst, 0);
		    break;
		default  :
		break;
		    } /* end inner switch */
		} /* end if */
	break; /* end case Ids_UnCompress */

     case Ids_CompressG31D :
	switch( cmap )
	    {
	case Ids_BandByPixel :	    
        case Ids_BandByPlane :
        case Ids_BitByPlane :
	    ret_fid = ImgCompressFrame( fid,  ImgK_G31dCompression, 0, 0);
	    break;
	default  :
	    break;
	    }
        break;

     case Ids_CompressG32D :
	switch( cmap )
	    {
	case Ids_BandByPixel :	    
        case Ids_BandByPlane :
        case Ids_BitByPlane :
	    ret_fid = ImgCompressFrame( fid,  ImgK_G32dCompression, 0, 0);
	    break;
	default  :
	    break;
	    }
	break;

     case Ids_CompressG42D :
	switch( cmap )
	    {
	case Ids_BandByPixel :	    
        case Ids_BandByPlane :
        case Ids_BitByPlane :
	    ret_fid = ImgCompressFrame( fid,  ImgK_G32dCompression, 0, 0);
	    break;
	default  :
	    break;
	    }
        break;

     case Ids_CompressDCT :
	switch( cmap )
	    {
        case Ids_BandByPixel :
        case Ids_BandByPlane :
        case Ids_BitByPlane  :
	    ret_fid = ImgCompressFrame( fid, ImgK_DctCompression, 0, 0);
	    break;
	default  :
	    break;
	    } 
        break;

     default :
	break;
      }

#ifdef TRACE
printf( "Leaving Routine ConvertRenderedFid in module IDS_XIEIMAGE_DDIF \n");
#endif
    return ret_fid;
}

/*****************************************************************************
**  IdsPhotoToDec()
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
**
*****************************************************************************/
void   IdsPhotoToDec(map,cmpres,cmap,name)
    XiePhotomap	    map;
    unsigned long   cmpres;
    unsigned long   cmap;
    char           *name;
{
    unsigned long   fid;

#ifdef TRACE
printf( "Entering Routine IdsPhotoToDec in module IDS_XIEIMAGE_DDIF \n");
#endif

    fid  = IdsPhotoToFid(map,cmpres,cmap);
    IdsFidToDec(fid,cmpres,cmap,name);

#ifdef TRACE
printf( "Leaving Routine IdsPhotoToDec in module IDS_XIEIMAGE_DDIF \n");
#endif
}

/*****************************************************************************
**  IdsFidToDec()
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
**
*****************************************************************************/
void  IdsFidToDec(fid,cmpres,cmap,name)
    unsigned long   fid;
    unsigned long   cmpres;
    unsigned long   cmap;
    char           *name;
{
#ifdef TRACE
printf( "Entering Routine IdsFidToDec in module IDS_XIEIMAGE_DDIF \n");
#endif

#ifdef TRACE
printf( "Leaving Routine IdsFidToDec in module IDS_XIEIMAGE_DDIF \n");
#endif
}

/*****************************************************************************
**   IdsDataForXie()
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
**
*****************************************************************************/
DataForXie      IdsDataForXie(fid)
 unsigned long	fid;
{
    DataForXie   xiedat;

#ifdef TRACE
printf( "Entering Routine IdsDataForXie in module IDS_XIEIMAGE_DDIF \n");
#endif
    /*
    ** Allocate DataForXie struct and fill up it fields
    */
    xiedat = (DataForXie) _ImgCalloc(1, sizeof(DataForXieStruct) ); 

    /*
    ** Fill up the ISL frame box's coordinates
    */
    if( fid != 0 )
	{
	GetIsl_( fid, Img_FrmBoxLLX,    Cllx_(xiedat), 0 );
	GetIsl_( fid, Img_FrmBoxLLY,    Clly_(xiedat), 0 );
	GetIsl_( fid, Img_FrmBoxURX,    Curx_(xiedat), 0 );
	GetIsl_( fid, Img_FrmBoxURY,    Cury_(xiedat), 0 );
        GetIsl_( fid, Img_PPPixelDist,  Cppd_(xiedat), 0 );
        GetIsl_( fid, Img_LPPixelDist,  Clpd_(xiedat), 0 );
	}

#ifdef TRACE
printf( "Leaving Routine IdsDataForXie in module IDS_XIEIMAGE_DDIF \n");
#endif
    return( (DataForXie) xiedat );
}

/*****************************************************************************
**   CollectRoiData()
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**
**  FUNCTION VALUE:
**
**
*****************************************************************************/
void  CollectRoiData(pridat, roi_id)
DataForXie pridat;
unsigned long roi_id;
{
    struct ROI *roi = (struct ROI *)roi_id;

#ifdef TRACE
printf( "Entering Routine CollectRoiData in module IDS_XIEIMAGE_DDIF \n");
#endif
    /*
    ** Grab roi info from fid and store in the XIE private data structure
    */
    if (roi_id == 0)
	{
        Croix_(pridat) = 0;
        Croiy_(pridat) = 0;
        Croiw_(pridat) = 0;
        Croih_(pridat) = 0;
	}
    else
	{
        Croix_(pridat) = roi->RoiR_BoundingRect.BrectL_Ulx;
        Croiy_(pridat) = roi->RoiR_BoundingRect.BrectL_Uly;
        Croiw_(pridat) = roi->RoiR_BoundingRect.BrectL_XPixels;
        Croih_(pridat) = roi->RoiR_BoundingRect.BrectL_YPixels;
	}
#ifdef TRACE
printf( "Leaving Routine CollectRoiData  in module IDS_XIEIMAGE_DDIF \n");
#endif
}
