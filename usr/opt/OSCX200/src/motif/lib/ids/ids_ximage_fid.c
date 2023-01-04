/***********************************************************************
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
**      Image Display Services   (IDS) 
**
**  ABSTRACT:
**	Module to convert a X bitmap to a FID
**
**  ENVIRONMENT:
**
**      VAX/VMS V5.0, DECwindows V1.0
**
**  AUTHOR(S):
**
**      Krishna Mangipudi
**
**  CREATION DATE:
**
**	October 11, 1990
**
**  MODIFICATION HISTORY:
**
**	
**
*****************************************************************************/

/*
**  Include files
*/
#include <stdio.h>

#if defined(__VMS) || defined(VMS)
#include <xlib.h> 
#include <xatom.h>
#else        
#include <X11/Xlib.h> 
#include <X11/Xatom.h>
#endif

#include <ids__macros.h> 
#include <img/IdsImage.h>
#include <img/ImgDef.h>
#include <img/ImgEntry.h>
#ifndef NODAS_PROTO
#include <idsprot.h>		/* IDS prototypes */
#endif

/*
**  Table of contents
*/
#ifdef NODAS_PROTO
#if defined(__VMS) || defined(VMS)
int			 ids$_ximagetofid();
#endif
#endif


#define X_DISPLAY_(lst) (lst[0].value)
#define X_WINDOW_(lst)  (lst[1].value)
/*
**  Equated symbols
*/


/*****************************************************************************
**  IdsXimageToFid
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**     display	    Structure describing a display connection
**     ximage	    The bitmap to be converted to a FID
**     visual	    Visual class of the drawable
**     cmap	    Color map
**     output_class Output class of the FID
**
*****************************************************************************/
/*DECwindows entry point definition                                           */
#if defined(__VMS) || defined(VMS)
unsigned long IDS$XIMAGETO_FID(display,ximage,visual,cmap,output_class)
     Display           *display;
     XImage             *ximage;
     Visual             *visual;
     Colormap           cmap;
     unsigned long      output_class;
{
    IdsXimageToFid(display,ximage,visual,cmap,output_class);
}
#endif
unsigned long IdsXimageToFid(display,ximage,visual,cmap,output_class)
     Display           *display;
     XImage		*ximage;
     Visual		*visual;
     Colormap		cmap;
     unsigned long	output_class;
{
    unsigned long fid;
    XColor		*color_ptr, *tmp_clr;
    int			i,j,k;
    int			ncolors;
    int			data_plane_size;
    char		*dp0, *temp_dp0;
    char		*dp1, *temp_dp1;
    char		*dp2, *temp_dp2;
    unsigned long	flag;
    int			GreyScale_Image = TRUE;
    struct ITMLST	itmlst[25];
    unsigned short      index;
    int			planes_per_pixel;
    int			bits_per_comp;
    int			bits_per_pixel;	
    int			bits_per_pixel_total;	
    int			scanline_stride;
    int			spectral_mapping;
    int			comp_org_space;
    int			data_class;
    int			sizechar = sizeof(char);
    int			sizeint	 = sizeof(int);
    int			brt_polarity;
    int			len, pad;

#ifdef TRACE
printf( "Entering Routine IdsXimageToFid in module IDS_XIMAGE_FID \n");
#endif

    if (ximage->depth == 1) /* Image is Bitonal */
    {
	/* Define Item list paramaters */
	comp_org_space = ImgK_BandIntrlvdByPixel;
	bits_per_pixel = 1;
	spectral_mapping = ImgK_MonochromeMap;
	data_class = ImgK_ClassBitonal;
	brt_polarity = ImgK_ZeroMaxIntensity;
	len = ximage->width;
	pad = ximage->bitmap_pad;
	scanline_stride = len + ((pad - len % pad) % pad);

	index = 0;
	INIT_ITM_(itmlst,index,Img_NumberOfLines,sizeint,&ximage->height,0,0);

	INIT_ITM_(itmlst,index,Img_PixelsPerLine,sizeint,&ximage->width,0,0);

	INIT_ITM_(itmlst,index,Img_BitsPerPixel,sizeint,&bits_per_pixel,0,0);
	INIT_ITM_(itmlst,index,Img_SpectralMapping,sizeint,&spectral_mapping,
									0,0);
	INIT_ITM_(itmlst,index,Img_ImageDataClass,sizeint,&data_class,0,0);
	INIT_ITM_(itmlst,index,Img_BrtPolarity,sizeint,&brt_polarity,0,0);

	INIT_ITM_(itmlst,index,Img_ScanlineStride,sizeint,&scanline_stride,0,0);
	END_ITEM_(itmlst,index);

	output_class =  ImgK_ClassBitonal; 
	flag = ImgM_NoDataPlaneAlloc;
	fid = ImgAllocateFrame(output_class,itmlst,0, flag);

	data_plane_size =  ximage->width * ximage->height;
	ImgImportDataPlane(fid, 0, (unsigned char *)ximage->data,data_plane_size, 0, 0, 0);
    }
    else /* Image is Multispectral or Greyscale */
    {
	/* Allocate memory for one scanline */
	color_ptr = (XColor *) calloc(ximage->width, sizeof(XColor));
	tmp_clr = color_ptr;

	/* compute size of the data plane */
	data_plane_size = ximage->width * ximage->height * sizechar;

	/* Define Item list parameters */
	comp_org_space = ImgK_BandIntrlvdByPlane;
	bits_per_comp = sizechar*8;
	bits_per_pixel = bits_per_comp;

	/* Create the data plane */
	dp0 = ImgAllocateDataPlane(data_plane_size,0);
	dp1 = ImgAllocateDataPlane(data_plane_size,0);
	dp2 = ImgAllocateDataPlane(data_plane_size,0);

	ncolors = 0;
	temp_dp0 = dp0;
	temp_dp1 = dp1;
	temp_dp2 = dp2;
	/* If visual class is either DirectColor or TrueColor then extract
	** the RGB components of the pixel using the visual component masks.
	** For all other visual types extract the RGB components from the
	** color map.
	*/
	if ( (visual->class == DirectColor) ||(visual->class == TrueColor) )
	{
	    for ( i=0; i<ximage->height; i++ )
	    {
		for ( j=0; j<ximage->width; j++ )
		{
		    if (ncolors < ximage->width-1)
		    {
			tmp_clr->pixel  =  XGetPixel( ximage, j, i);
			ncolors++; tmp_clr++;
		    }
		    else
		    {
			tmp_clr->pixel  =  XGetPixel( ximage, j, i);
			ncolors++; tmp_clr++;
			tmp_clr = color_ptr;
			for( k=0; k<ximage->width; k++, tmp_clr++ )
			{
			    tmp_clr->red = (tmp_clr->pixel & visual->red_mask) ;
			    tmp_clr->green = (tmp_clr->pixel & 
						visual->green_mask) >> 8;
			    tmp_clr->blue  = (tmp_clr->pixel & 
						visual->blue_mask)  >> 16;
			    if ( ((tmp_clr->red   != tmp_clr->green) ||
				  (tmp_clr->green != tmp_clr->blue )) &&
				   GreyScale_Image )
				      GreyScale_Image = FALSE; 
			    memcpy(temp_dp0++,&tmp_clr->red, sizechar);	
			    memcpy(temp_dp1++,&tmp_clr->green,sizechar);
			    memcpy(temp_dp2++,&tmp_clr->blue, sizechar); 
			}
			ncolors = 0; tmp_clr = color_ptr;
		    }
		}
	    }
	}
	else /* Neither DirectColor nor TrueColor visual */ 
	{
	    for ( i=0; i<ximage->height; i++ )
	    {
		for ( j=0; j<ximage->width; j++ )
		{
		    if ( ncolors < ximage->width-1 ) /* create one scanline */
		    {
			tmp_clr->pixel  =  XGetPixel( ximage, j, i);
			ncolors++; tmp_clr++;
		    }
		    else   /* Copy the RGB values into dataplane */
		    {
			tmp_clr->pixel  =  XGetPixel( ximage, j, i);
			ncolors++; tmp_clr++;
			tmp_clr = color_ptr;
			XQueryColors( display, cmap, tmp_clr, ncolors );
			for( k=0; k<ximage->width; k++, tmp_clr++ )
			{
			    tmp_clr->red   = tmp_clr->red   >> 8;
			    tmp_clr->green = tmp_clr->green >> 8;
			    tmp_clr->blue  = tmp_clr->blue  >> 8;
			    if ( ((tmp_clr->red   != tmp_clr->green) ||
				  (tmp_clr->green != tmp_clr->blue )) &&
				   GreyScale_Image )
				      GreyScale_Image = FALSE; 
			    memcpy(temp_dp0++,&tmp_clr->red,  sizechar);	
			    memcpy(temp_dp1++,&tmp_clr->green,sizechar);
			    memcpy(temp_dp2++,&tmp_clr->blue, sizechar); 
			}
			ncolors = 0; tmp_clr = color_ptr;
		    }
		}
	    }
	}
	free( color_ptr );
	
	if (GreyScale_Image == FALSE)
	{
		planes_per_pixel = 3;
		bits_per_pixel_total = bits_per_comp*3;
		spectral_mapping = ImgK_RGBMap;
		data_class = ImgK_ClassMultispect;
		output_class = ImgK_ClassMultispect;  
	} 
	else
	{
		planes_per_pixel = 1;
		bits_per_pixel_total = bits_per_comp;
		spectral_mapping = ImgK_MonochromeMap;
		data_class = ImgK_ClassGreyscale;
		output_class = ImgK_ClassGreyscale;
	}
   /* 
    ** Setup the item list to create an image frame 
    */
	index = 0;
	INIT_ITM_(itmlst,index,Img_NumberOfLines,sizeint,&ximage->height,0,0);
	INIT_ITM_(itmlst,index,Img_PixelsPerLine,sizeint,&ximage->width,0,0);
	INIT_ITM_(itmlst,index,Img_BitsPerComp,sizeint,&bits_per_comp,0,0);
	INIT_ITM_(itmlst,index,Img_PlaneBitsPerPixel,sizeint,&bits_per_pixel,
									0,0);
	if (GreyScale_Image == FALSE)
	{
	    INIT_ITM_(itmlst,index,Img_NumberOfLines,sizeint,
			    &ximage->height,0,1);
	    INIT_ITM_(itmlst,index,Img_NumberOfLines,sizeint,
			    &ximage->height,0,2);

	    INIT_ITM_(itmlst,index,Img_PixelsPerLine,sizeint,
			    &ximage->width,0,1);
	    INIT_ITM_(itmlst,index,Img_PixelsPerLine,sizeint,
			    &ximage->width,0,2);

	    INIT_ITM_(itmlst,index,Img_BitsPerComp,sizeint,
			    &bits_per_comp,0,1);
	    INIT_ITM_(itmlst,index,Img_BitsPerComp,sizeint,
			    &bits_per_comp,0,2);

	    INIT_ITM_(itmlst,index,Img_PlaneBitsPerPixel,sizeint,
			    &bits_per_pixel,0,1);
	    INIT_ITM_(itmlst,index,Img_PlaneBitsPerPixel,sizeint,
			    &bits_per_pixel,0,2);
	} 

	INIT_ITM_(itmlst,index,Img_CompSpaceOrg,sizeint,&comp_org_space,0,0);

	INIT_ITM_(itmlst,index,Img_PlanesPerPixel,sizeint,&planes_per_pixel,
									0,0);


	INIT_ITM_(itmlst,index,Img_TotalBitsPerPixel,sizeint,
						  &bits_per_pixel_total,0,0);
	INIT_ITM_(itmlst,index,Img_SpectralMapping,sizeint,&spectral_mapping,
									0,0);
	INIT_ITM_(itmlst,index,Img_ImageDataClass,sizeint,&data_class,0,0);
	END_ITEM_(itmlst,index);

	flag = ImgM_NoDataPlaneAlloc;
	fid = ImgAllocateFrame(output_class,itmlst,0,flag);

	/* 
	** Attach dataplane to the image frame and return the fid 
	*/
	fid = ImgAttachDataPlane( fid, dp0, 0 );
	if (GreyScale_Image == FALSE)
	{
	    fid = ImgAttachDataPlane( fid, dp1, 1 );
	    fid = ImgAttachDataPlane( fid, dp2, 2 );
	} 
	else
	{
	    ImgFreeDataPlane(dp1);
	    ImgFreeDataPlane(dp2);
	}
    }

#ifdef TRACE
printf( "Leaving Routine IdsXimageToFid in module IDS_XIMAGE_FID \n");
#endif

    return( fid );
}
