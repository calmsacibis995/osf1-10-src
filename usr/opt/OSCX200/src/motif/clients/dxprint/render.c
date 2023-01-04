/*
** COPYRIGHT (c) 1992 BY
** DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.
** ALL RIGHTS RESERVED.
**
** THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED
** ONLY  IN  ACCORDANCE  OF  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE
** INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER
** COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
** OTHER PERSON.  NO TITLE TO AND  OWNERSHIP OF THE  SOFTWARE IS  HEREBY
** TRANSFERRED.
**
** THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE
** AND  SHOULD  NOT  BE  CONSTRUED  AS A COMMITMENT BY DIGITAL EQUIPMENT
** CORPORATION.
**
** DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS
** SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.
*/

/*
**++
**  MODULE NAME:
**	render.c
**
**  FACILITY:
**      PrintScreen
**
**  ABSTRACT:
**	Subroutines which convert an X11 image or xwd file
**		to one of the supported output formats.
**
**  ENVIRONMENT:
**	VMS V5.4, Ultrix V4.2, OSF/1 1.0, Motif V1.1, ISL V3.0
**
**  AUTHOR:
**      Edward P Luwish
**
**  CREATION DATE:     
**	January 20, 1992
**
**  MODIFICATION HISTORY:
**
**	DATE		Developer
**		Description
**
**
**	30-Nov-93 pjw Add comments. Repair CvtDataClass crock to not
**		generate a useless PS buffer. 
**
**	17-Aug-1992	Edward P Luwish
**		Added CvtDataClass routine to convert multi-plane images to
**		grayscale or bitonal.
**--
*/

#include "iprdw.h"
#include <stdio.h>
#ifdef VMS
#include <descrip.h>
#endif

#if defined(VMS)
#include <ssdef.h>                             /* sys services messages    */
#endif /* VMS */

#if defined(VMS) && !defined(__DECC)
#pragma nostandard
#endif
#include <img/IdsImage.h>
#include <img/ImgDef.h>
#include <img/ImgEntry.h>
#include <Mrm/MrmAppl.h>
#if defined(VMS) && !defined(__DECC)
#pragma standard
#endif

/*
** These are two cutesy macros pulled from idsmacros.h, which is not shipped
** with the ISL kit.
*/
#define INIT_ITM_(struct,ctr,code,length,buffer,retlen,index) \
    struct[ctr].ItmL_Code   = (unsigned long int)    code;\
    struct[ctr].ItmL_Length = (unsigned long int)    length;\
    struct[ctr].ItmA_Buffer = (char *)         (buffer);\
    struct[ctr].ItmA_Retlen = (unsigned long int *) (retlen);\
    struct[ctr].ItmL_Index  = (unsigned long int)    index;\
    ctr++

/* short version */
#define END_ITEM_(struct,ctr)\
    struct[ctr].ItmL_Code = 0;\
    struct[ctr].ItmL_Length = 0;\
    struct[ctr].ItmA_Buffer = 0;\
    struct[ctr].ItmA_Retlen = 0;\
    struct[ctr].ItmL_Index = 0;\
    ctr++

#define I_I(code,buffer,retlen,ctr) \
    INIT_ITM_(itmlst,index,code,sizeof(buffer),&buffer,retlen,ctr)
#define E_I END_ITEM_(itmlst,index)

#ifdef VMS
#include <time.h>
#else
#include <sys/time.h>
#endif

#define BMUS_PER_INCH 1200

/*
** CONVERT_PIXEL_TO_BMU: Given a pixel value, returns the equivalent
** basic measuring unit value using the device pixel DPI (dots per inch)
** value, treated as a linear measurement; parameter is an integer value
*/
#define CONVERT_PIXEL_TO_BMU(pixels, screen) \
    ((BMUS_PER_INCH * pixels) / (DPIof(screen)))


static unsigned long BitonalXimageToFid
#if _PRDW_PROTO_
(
     Display		*display,
     Screen		*screen,
     XImage		*ximage,
     Visual		*visual,
     Colormap		cmap,
     unsigned long	output_class,
     int		polarity
)
#else
(display, screen, ximage, visual, cmap, output_class, polarity)
     Display           *display;
     Screen		*screen;
     XImage		*ximage;
     Visual		*visual;
     Colormap		cmap;
     unsigned long	output_class;
     int		polarity;
#endif
/*****************************************************************************
**  XimageToFid
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**     display	    Structure describing a display connection
**     ximage	    The bitmap to be converted to a FID
**     visual	    Visual class of the drawable
**     cmap	    Color map
**     output_class Output class of the FID
**     polarity     Negative or positive tone scaling
**
*****************************************************************************/

{
    unsigned long	fid;
    unsigned long	data_plane_size;
    unsigned long	flag;
    struct ITMLST	itmlst[25];
    unsigned short      index;
    unsigned long	bits_per_pixel;
    unsigned long	scanline_stride;
    unsigned long	spectral_mapping;
    unsigned long	comp_org_space;
    unsigned long	data_class;
    unsigned long	width;
    unsigned long	height;
    unsigned long	bmu_width, bmu_height;
    unsigned long	zero = 0;

    unsigned long	brt_polarity;
    int			len, pad;

    width = ximage->width;
    height = ximage->height;

    /* Define Item list paramaters */
    comp_org_space = ImgK_BandIntrlvdByPixel;
    bits_per_pixel = 1;
    spectral_mapping = ImgK_MonochromeMap;
    data_class = ImgK_ClassBitonal;

    if (polarity == dxPrscNegative)
	brt_polarity = ImgK_ZeroMaxIntensity;
    else
	brt_polarity = ImgK_ZeroMinIntensity;

    len = width;
    pad = ximage->bitmap_pad;
    scanline_stride = len + ((pad - len % pad) % pad);

    bmu_width = CONVERT_PIXEL_TO_BMU (width, screen);
    bmu_height = CONVERT_PIXEL_TO_BMU (height, screen);

    index = 0;
    I_I(Img_NumberOfLines,height,0,0);

    I_I(Img_PixelsPerLine,width,0,0);

    I_I(Img_BitsPerPixel,bits_per_pixel,0,0);
    I_I(Img_SpectralMapping,spectral_mapping,0,0);
    I_I(Img_ImageDataClass,data_class,0,0);
    I_I(Img_BrtPolarity,brt_polarity,0,0);

    I_I(Img_ScanlineStride,scanline_stride,0,0);

    I_I (Img_FrmBoxLLX, zero, 0, 0);
    I_I (Img_FrmBoxLLY, zero, 0, 0);
    I_I (Img_FrmBoxURX, bmu_width, 0, 0);
    I_I (Img_FrmBoxURY, bmu_height, 0, 0);

    E_I;

    output_class =  ImgK_ClassBitonal; 
    flag = ImgM_NoDataPlaneAlloc;
    fid = ImgAllocateFrame (output_class, itmlst, 0, flag);

    data_plane_size =  width * height;
    ImgImportDataPlane (fid, 0, ximage->data, data_plane_size, 0, 0, 0);

    return (fid);
}

unsigned long XimageToFid
#if _PRDW_PROTO_
(
     Display		*display,
     Screen		*screen,
     XImage		*ximage,
     Visual		*visual,
     Colormap		cmap,
     unsigned long	output_class,
     int		polarity
)
#else
(display, screen, ximage, visual, cmap, output_class, polarity)
     Display           *display;
     Screen		*screen;
     XImage		*ximage;
     Visual		*visual;
     Colormap		cmap;
     unsigned long	output_class;
     int		polarity;
#endif
/*****************************************************************************
**  XimageToFid
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**     display	    Structure describing a display connection
**     ximage	    The bitmap to be converted to a FID
**     visual	    Visual class of the drawable
**     cmap	    Color map
**     output_class Output class of the FID
**     polarity     Negative or positive tone scaling
**
*****************************************************************************/

{
    int			i, j;
    unsigned long	fid;
    XColor		*color_table;
    int			ct_sz;
    int			rshf, gshf, bshf;
    unsigned long	rmask, gmask, bmask;
    unsigned long	data_plane_size;
    char	 	*dp0, *temp_dp0;
    char	 	*dp1, *temp_dp1;
    char	 	*dp2, *temp_dp2;
    unsigned long	flag;
    int			GreyScale_Image = TRUE;
    struct ITMLST	itmlst[30];
    unsigned short      index;
    unsigned long	planes_per_pixel;
    unsigned long	bits_per_comp;
    unsigned long	bits_per_pixel;	
    unsigned long	bits_per_pixel_total;	
    unsigned long	scanline_stride;
    unsigned long	spectral_mapping;
    unsigned long	comp_org_space;
    unsigned long	data_class;
    unsigned long	width;
    unsigned long	height;
    unsigned long	bmu_width, bmu_height;
    unsigned long	zero = 0;

    unsigned long	brt_polarity;
    int			len, pad;

    width = ximage->width;
    height = ximage->height;

    /*
    ** Bitonal image is a special case.
    */
    if (ximage->depth == 1)
    {
	fid = BitonalXimageToFid
	    (display, screen, ximage, visual, cmap, output_class, polarity);
	return (fid);
    }

    bmu_width = CONVERT_PIXEL_TO_BMU (width, screen);
    bmu_height = CONVERT_PIXEL_TO_BMU (height, screen);

    if (polarity == dxPrscNegative)
	brt_polarity = ImgK_ZeroMaxIntensity;
    else
	brt_polarity = ImgK_ZeroMinIntensity;

    /*
    ** Get a color table from the selected Colormap and Visual.
    ** The call to this routin should really be moved inside the GrabScreen.
    */
    ct_sz = getColors (display, cmap, visual, &color_table);

    /*
    ** Change the 16 bit pixel values into 8 bit.
    */
    for (i = 0; i < (ct_sz); i++)
    {
	color_table[i].red >>= 8;
	color_table[i].green >>= 8;
	color_table[i].blue >>= 8;
    }

    /*
    ** compute size of the data plane
    */
    data_plane_size = width * height * sizeof(char);

    /*
    ** Define Item list parameters
    */
    comp_org_space = ImgK_BandIntrlvdByPlane;
    bits_per_comp = sizeof(char)*8;
    bits_per_pixel = bits_per_comp;

    /*
    ** Create the data plane. Use the new entry point
    ** (ImgAlloc vs. ImgAllocate) which takes three arguments. 
    */
    dp0 = ImgAllocDataPlane (data_plane_size, 0L, (unsigned char)0);
    dp1 = ImgAllocDataPlane (data_plane_size, 0L, (unsigned char)1);
    dp2 = ImgAllocDataPlane (data_plane_size, 0L, (unsigned char)2);

    temp_dp0 = dp0;
    temp_dp1 = dp1;
    temp_dp2 = dp2;


    /*
    ** Get the information from the visual for manipulating the color table.
    */
    if ((visual->class == TrueColor) || (visual->class == DirectColor))
    {
	Pixel r1, g1, b1;
	/*
	** just of "correctness" could simply be done by saying 32, since
	** the real values are going to 32 for I14y.
	*/
	int max_shift = sizeof(long) * 8;

	/*
	** The mask values for the components.
	*/
	rmask = visual->red_mask;
	gmask = visual->green_mask;
	bmask = visual->blue_mask;

/*
** This macro defines the least significant pixel in a mask.  This is a
** multiple of 2 and can be used to divide the masked value, generating
** the equivalent of a right shift.
**
** Example:
**  pixel = 0x0f0e03
**  rmask = 0xff
**  gmask = 0xff00
**  bmask = 0xff0000
**  rshf = 0
**  gshf = 0x100
**  bshf = 0x10000
**  (pixel & gmask) = 0x0e00
**  (pixel & gmask) / gshf = 0x0e
**	(ie: this is the value of the green component)
**  (pixel & rmask) = 0x03
**  (pixel & rmask) / rshf = 0x03
**  (pixel & bmask) = 0x0f0000
**  (pixel & bmask) / bshf = 0x0f
*/
#define lowbit(x) ((x) & (~(x) + 1))

	rshf = lowbit(rmask);
	gshf = lowbit(gmask);
	bshf = lowbit(bmask);
    }

    /*
    ** Loop through the pixels in the image and pull in the actual rgb values.
    */
    for (i = 0; i < height; i++)
    {
	for (j = 0; j < width; j++)
	{
	    unsigned long   pixel;
	    unsigned short  red, blue, green;

	    /*
	    ** 8 bit per component value for the specified X,Y location.
	    */
	    pixel = XGetPixel (ximage, j, i);
	    if ((visual->class == TrueColor) || (visual->class == DirectColor))
	    {
		/*
		** Decompose the pixel value into separate red, green and
		** blue indices into the color table.
		*/
		red =   color_table[(pixel & rmask) / rshf].red;
		green = color_table[(pixel & gmask) / gshf].green;
		blue =  color_table[(pixel & bmask) / bshf].blue;
	    }
	    else
	    {
		red =   color_table[pixel].red;
		green = color_table[pixel].green;
		blue =  color_table[pixel].blue;
	    }

	    /*
	    ** If the components aren't equal, then we aren't greyscale.
	    */
	    if (((red != green) || (green != blue)) && GreyScale_Image)
	    {
		GreyScale_Image = FALSE;
	    }

	    /*
	    ** Put the values into the data planes.
	    */
	    memcpy (temp_dp0++, &red,   sizeof(char));	
	    memcpy (temp_dp1++, &green, sizeof(char));
	    memcpy (temp_dp2++, &blue,  sizeof(char)); 
	}
    }
    XtFree ((char *)color_table);
    
    if (GreyScale_Image == FALSE)
    {
	planes_per_pixel = 3;
	spectral_mapping = ImgK_RGBMap;
	data_class = ImgK_ClassMultispect;
	output_class = data_class; 
    } 
    else
    {
	planes_per_pixel = 1;
	spectral_mapping = ImgK_MonochromeMap;
	data_class = ImgK_ClassGreyscale;
	output_class = data_class;
    }
    bits_per_pixel_total = bits_per_comp * planes_per_pixel;

    /* 
    ** Setup the item list to create an image frame 
    */
    index = 0;
    I_I(Img_NumberOfLines,height,0,0);
    I_I(Img_PixelsPerLine,width,0,0);
    I_I(Img_BitsPerComp,bits_per_comp,0,0);
    I_I(Img_PlaneBitsPerPixel,bits_per_pixel,0,0);
    if (GreyScale_Image == FALSE)
    {
	I_I(Img_NumberOfLines,height,0,1);
	I_I(Img_NumberOfLines,height,0,2);

	I_I(Img_PixelsPerLine,width,0,1);
	I_I(Img_PixelsPerLine,width,0,2);

	I_I(Img_BitsPerComp,bits_per_comp,0,1);
	I_I(Img_BitsPerComp,bits_per_comp,0,2);

	I_I(Img_PlaneBitsPerPixel,bits_per_pixel,0,1);
	I_I(Img_PlaneBitsPerPixel,bits_per_pixel,0,2);
    } 

    I_I(Img_CompSpaceOrg,comp_org_space,0,0);

    I_I(Img_PlanesPerPixel,planes_per_pixel,0,0);


    I_I(Img_TotalBitsPerPixel,bits_per_pixel_total,0,0);
    I_I(Img_SpectralMapping,spectral_mapping,0,0);
    I_I(Img_ImageDataClass,data_class,0,0);
    I_I(Img_BrtPolarity,brt_polarity,0,0);

    /*
    ** Specify the image bounding box.  Particularly important for creating
    ** DDIF as the images need the frame when sent to DECwrite.
    */
    I_I (Img_FrmBoxLLX, zero, 0, 0);
    I_I (Img_FrmBoxLLY, zero, 0, 0);
    I_I (Img_FrmBoxURX, bmu_width, 0, 0);
    I_I (Img_FrmBoxURY, bmu_height, 0, 0);

    E_I;

    flag = ImgM_NoDataPlaneAlloc;
    fid = ImgAllocateFrame(output_class,itmlst,0,flag);

    /* 
    ** Attach dataplane to the image frame and return the fid 
    */
    fid = ImgAttachDataPlane (fid, dp0, 0L);
    if (GreyScale_Image == FALSE)
    {
	fid = ImgAttachDataPlane (fid, dp1, 1L);
	fid = ImgAttachDataPlane (fid, dp2, 2L);
    } 
    else
    {
	/*
	** If the image is all gray, we don't need all three planes.
	*/
	ImgFreeDataPlane(dp1);
	ImgFreeDataPlane(dp2);
    }
    return( fid );
}


unsigned long CvtDataClass
#if _PRDW_PROTO_
(
    unsigned long	srcfid,
    int			inclass,
    int			outclass
)
#else
(srcfid, inclass, outclass)
    unsigned long	srcfid;
    int			inclass;
    int			outclass;
#endif
/*
 * Function:
 * 	CvtDataClass - converts an image from one "visual type" to another
 * 	i.e. from color to greyscale, ccolor/greyscale to b&w. 
 *	
 * Inputs:
 * 	scrfid 	  - pointer to orignal image frame
 * 	inclass   - the type of the image
 * 	outclass  - the type of image you want to convert to 
 *	
 * Outputs:
 *	
 * Notes:
 * 
 * pjw - GAG! This entire routine is a stupid gross hack. In it's previous
 * pathetic state it was basically  asking image services to generate PS
 * output in order that it  get the resulting "transformed" fid. IOW, as a
 * side-effect of asking image services to generate PS for a particular
 * device, it would also generate the equivalent image frame. The PS is
 * discarded and the resulting image frame is then fed the routines which
 * generate  PS and sixels!! IOW, the code if generating the output twice! The
 * idiocy of it all is that IDS can do this in a single pass just by
 * specifying the proper attributes in the the creatps and creatsx routines.
 * This code is totally unnecessary!  However, it would require too much
 * testing to yank it at this  late stage of the game so I'll do the minumum
 * hacks to make it  work as is. 
 * 
 */
{
    unsigned long	psid;
    unsigned long	retfid = 0;
    IdsItmlst2		ps_itmlst[20];
    IdsItmlst2		*ps_ptr = ps_itmlst;
    IdsItmlst2		rn_itmlst[20];
    IdsItmlst2		*rn_ptr = rn_itmlst;
    IdsRendering	*rendering;


    ps_ptr->item_name = IdsNtemplate;
    switch (inclass)
    {
    case dxPrscPrinterColor:		/* Map color to greyscale or b&w */
	ps_ptr->value = Ids_TmpltColorPs;
	break;
    case dxPrscPrinterGrey:		/* Map greyscale to b&w */

/* pjw: I think it would be sufficient to simply use Ids_TmplLps20 here
 * to force bitonal and not have to specify the rendering class. 
 * I'll leave it this way for now 
 */
	ps_ptr->value = Ids_TmpltMonoPs;
	++ps_ptr;
	ps_ptr->item_name = IdsNrenderingClass;
	ps_ptr->value = Ids_Bitonal;	/* force B&W */
	break;
    }
/* Make sure that we do NOT generate any actual PS buffers. We just
 * want the resulting fid back from IDS
 */
    ++ps_ptr;
    ps_ptr->item_name = IdsNprotocol;
    ps_ptr->value = Ids_Fid;

    ++ps_ptr;
    ps_ptr->item_name = NULL;		/* End of item list */
    ps_ptr->value = 0;

    psid = IdsCreatePresentSurface (ps_itmlst);
    if (psid == 0L)
    {
	ImgDeleteFrame (srcfid);
	return (retfid);
    }



    if (outclass == dxPrscPrinterGrey)	/* Convert from color to greyscale */
    {
	rn_ptr->item_name = IdsNlevelsGray;
	rn_ptr->value = 256;
	++rn_ptr;

	rn_ptr->item_name = IdsNfitLevels;
	rn_ptr->value = 256;
	++rn_ptr;
#if 0
	rn_ptr->item_name = IdsNditherAlgorithm;
	rn_ptr->value = Ids_Dispersed;
	++rn_ptr;
#endif
    }
    else				/* convert to B&W (bitonal) */
    {
	rn_ptr->item_name = IdsNlevelsGray;
	rn_ptr->value = 2;
	++rn_ptr;

	rn_ptr->item_name = IdsNfitLevels;
	rn_ptr->value = 2;
	++rn_ptr;
#if 0
	rn_ptr->item_name = IdsNditherAlgorithm;
	rn_ptr->value = Ids_Requantize;
	++rn_ptr;
#endif
    }

    rn_ptr->item_name = NULL;
    rn_ptr->value = 0;

    rendering = IdsCreateRendering (srcfid, psid, rn_itmlst);
    if (rendering != NULL)
    {
	retfid = ImgCopyFrame (rendering->rndfid, 0L);
#if 0
	if (outclass != dxPrscPrinterGrey)
	{
	    int	index = 0;
	    int bits = 1;
	    struct ITMLST	itmlst[2];
	    I_I(Img_BitsPerPixel,bits,0,0);
	    E_I;
	    retfid = ImgSetFrameAttributes (retfid, itmlst);
	}
#endif
	IdsDeleteRendering (rendering);
    }

    ImgDeleteFrame (srcfid);
    IdsDeletePresentSurface (psid);

    return (retfid);
}

/*
 * Condition handler for signals received from Image Services.
 */
long img_signal_handler
#if _PRDW_PROTO_
(
    ChfSigVecPtr	sigarg,
    ChfMchArgsPtr	mcharg
)
#else
(sigarg, mcharg)
    ChfSigVecPtr	sigarg;
    ChfMchArgsPtr	mcharg;
#endif
{
#ifdef VMS

    if (sigarg->name == SS$_UNWIND)
        return SS$_RESIGNAL;

    if (sigarg->name == SS$_DEBUG)
        return SS$_RESIGNAL;

    if (!BAD(sigarg->name))
        return SS$_CONTINUE;

    return (dxPrscFatErrLib);
#endif
}
