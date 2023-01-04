#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Id: fileio.c,v 1.1.4.2 1993/06/25 22:46:08 Ronald_Hegli Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
/*
****************************************************************************
**                                                                          *
**  Copyright (c) 1987                                                      *
**  By DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                        *
**  All Rights Reserved
**                                                                          *
**  This software is furnished under a license and may be used and  copied  *
**  only  in  accordance  with  the  terms  of  such  license and with the  *
**  inclusion of the above copyright notice.  This software or  any  other  *
**  copies  thereof may not be provided or otherwise made available to any  *
**  other person.  No title to and ownership of  the  software  is  hereby  *
**  transferred.                                                            *
**                                                                          *
**  The information in this software is subject to change  without  notice  *
**  and  should  not  be  construed  as  a commitment by DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL assumes no responsibility for the use or  reliability  of  its  *
**  software on equipment which is not supplied by DIGITAL.                 *
**                                                                          *
*****************************************************************************
**++
**  FACILITY:
**
**   DECpaint - DECwindows paint program
**
**  AUTHOR
**
**   Daniel Latham, October 1987
**
**  ABSTRACT:
**
**   FILEIO contains routines that are called to read in and write out
**   a picture file.
**
**  ENVIRONMENT:
**
**   User mode, executable image.
**
**  MODIFICATION HISTORY:
**
** 	pjw - 22-nov-93 Add Dhiren's long vs. int bug fix to accomodate
**	      latest Image Services. ootb_bug 39
**
**      dl      10/6/88
**      add error handling on file save
**
**	jj	    10/13/88
**	when file succesfully read in set undo to NO_ACTION
**--
**/           
#include "paintrefs.h"

#include <cdaptp.h>
#include <cdadef.h>
#include <cdamsg.h>
#include <ddifdef.h>

/* macro for setting up item list for
 * calls to cda_convert
 */
#define MAKE_ITEM(item, i, code, length, address)			       \
{item[i].item_code    = (unsigned short int) code;			       \
 item[i].item_length  = (unsigned short int) length;			       \
 item[i].CDAitemparam.item_address = (char *) address;}

static Pixmap ret_pixmap;
static unsigned int ret_wd, ret_ht;
static char ddif_marker[] = "DECpaint";
static int new_file_color;

#define ss$_continue 1
static int unwind_depth;

/*
 *
 * ROUTINE:  Force_Return_On_Failure
 *
 * ABSTRACT: 
 *         
 *  Recover from an error
 *
 */
int Force_Return_On_Failure(sigargs, mechargs)
    int *sigargs;
    int *mechargs;
{
    failure_occurred = 1;

    unwind_depth = mechargs[2];

    ChfUnwind(&unwind_depth, 0);

    return(ss$_continue);
}


/*ram*/
unsigned char reverse_bits[0x100] = {
        0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
        0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
        0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
        0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
        0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
        0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
        0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
        0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
        0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
        0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
        0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
        0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
        0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
        0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
        0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
        0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
        0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
        0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
        0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
        0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
        0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
        0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
        0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
        0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
        0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
        0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
        0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
        0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
        0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
        0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
        0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
        0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff
};


/* Routine to convert image data to client native bit order *//*ram*/
ConvertImageNative (ximage)
XImage *ximage;
{
    register unsigned char *imgbyteptr = (unsigned char *) ximage->data;
    register unsigned char *rev = reverse_bits;
    int i, imgbytes;

    if (((ximage->format == ZPixmap) && (ximage->bits_per_pixel != 1)) ||
        (ximage->bitmap_bit_order == NATIVE_BIT_ORDER))
        return;

    imgbytes = ximage->height * ximage->bytes_per_line;
    for (i=0; i< imgbytes; i++) {
        *imgbyteptr = rev[*imgbyteptr];
        imgbyteptr++;
    }
}



/*
 *
 * ROUTINE:  Find_BMU_Scale_Factor
 *
 * ABSTRACT: 
 *
 *  Calculate basic measurement units for height and width
 *
 */
void	Find_BMU_Scale_Factor( 	WidthFactor, HeightFactor )
float	*WidthFactor;
float	*HeightFactor;
  { 
 
     *HeightFactor = (float)(BMU) / (float)resolution; /* BMU per Pixel */
     *WidthFactor =  (float)(BMU) / (float)resolution; /* BMU per Pixel */

     return;
  }

/*
 *
 * ROUTINE:  New_Image
 *
 * ABSTRACT: 
 *                                
 *   Return a copy of the current image updated..
 *
 */
XImage *New_Image ()
{
    XImage *ximage, *tmp_image;
    int bits_per_line, extra_bits, image_bytes, tmp = pixmap_changed;
    long bits_per_pixel;
    unsigned char *image_data;

    bits_per_pixel = (pdepth == 1) ? 1 : 8;
    tmp_image = picture_image;
    bits_per_line = pimage_wd * bits_per_pixel;
    if (extra_bits = bits_per_line % 32)
        bits_per_line = bits_per_line + 32 - extra_bits;
    image_bytes = (bits_per_line / 8) * pimage_ht;
    image_data = (unsigned char *) XtCalloc
	(image_bytes, sizeof(unsigned char));
    if (image_data == 0) 
	return (0);
    picture_image = XCreateImage
    (
	disp, visual_info->visual,
        pdepth, img_format, 0, (char *)image_data, pimage_wd,
        pimage_ht, 32, 0
    );
    picture_image->bitmap_bit_order = NATIVE_BIT_ORDER;    /*ram*/
    picture_image->byte_order = NATIVE_BYTE_ORDER;         /*ram*/
    Merge_Image(0, 0, pimage_wd, pimage_ht, 0, 0, tmp_image, picture_image,
		ImgK_Src);
    Update_Image (0, 0, picture_wd, picture_ht, picture_x, picture_y, picture,
		  ImgK_Src);
    ximage = picture_image;
    picture_image = tmp_image;
    pixmap_changed = tmp;
    return (ximage);
}

int Pad_Image(image_id, adjusted_image_id, new)
    unsigned long image_id;
    unsigned long *adjusted_image_id;
    int new;
/*
 * FUNCTION:
 *      Creates an ISL frame which has scanlines padded to be aligned on
 *      longword boundaries. Note that the original image may be returned
 *      if it is correctly formatted.
 * INPUTS:
 *	image_id = ISL frame identifier to adjust
 *      adjusted_image_id = ISL frame identifier of adjusted image
 * RESULTS:
 *	
 * NOTES:
 *      The adjusted image might be the original image (i.e. not a copy).
 *	if new is true, always create a new image id.
 */
{
    int off_modulo;
    int scanline_stride;
    static int scanline_pad = 32;

    struct PUT_ITMLST set_attributes[2];
    int set_index;
    struct GET_ITMLST get_attributes[2];
    int get_index;

    start_get_itemlist(get_attributes, get_index);
    put_get_item(get_attributes, get_index, Img_ScanlineStride, scanline_stride);
    end_get_itemlist(get_attributes, get_index);

    ImgGetFrameAttributes(image_id, get_attributes);

    off_modulo = scanline_stride % scanline_pad;
    if ((off_modulo == 0) && (!new))
       *adjusted_image_id = image_id;
    else {
	if (off_modulo) 
	    scanline_stride += scanline_pad - off_modulo;

	start_set_itemlist(set_attributes, set_index);
	put_set_item(set_attributes, set_index, Img_ScanlineStride, scanline_stride);
	end_set_itemlist(set_attributes, set_index);

	ISL_ERROR_HANDLER_SETUP
	*adjusted_image_id = ImgCopy(image_id, 0, set_attributes);
	if (failure_occurred) {
	    ISL_RECOVER_FROM_ERROR
	    return (0);
       }
   }
   return 1;
}

/* if ret_img_id is passed in, then return the image id to that address */
/* otherwise, just write the DDIF to a file and delete the image id */
int Write_DDIF(ximage, ret_img_id)
    XImage *ximage;           
    long *ret_img_id;
{
    int i, j;
    unsigned char *image_data;
    unsigned long image_id, new_id;
    int set_index;
    struct PUT_ITMLST set_attributes[25];
    buf_descriptor_type array_descriptor;
    int status; /* dl- 10/6/88 */
    unsigned long context;
    int image_size;
    long bmu_x, bmu_y;
    long spectral_mapping;
    int new_image_data = FALSE;
    float width_factor, height_factor;
    static unsigned long zero = 0;
    long quant_levels_per_comp, bits_per_comp;
    int colors_used[MAX_COLORS];
    int num_colors_used;
    DDIF_COLOR_TYPE ddif_colors[MAX_COLORS];
    int im_stype;
    long int *pf_table;
    struct HCB *hist_id;
    int skip;
    unsigned char *Expand_Image_Data_Cmap_To_RGB();
    unsigned char *Convert_Image_Data_Cmap_To_BW();

/*
    if (pdepth != 1) {
	status = Write_Color_DDIF (ximage);
	return (status);
    }
*/

/*
    switch (visual_info->class) {
	case StaticGray :
	    file_color = SAVE_BW;
	    break;
	case GrayScale :
	    file_color = SAVE_GRAY;
	    break;
	case StaticColor :
	case PseudoColor :
	case TrueColor :
	case DirectColor :
	    file_color = SAVE_COLOR;
	    break;
    }   
*/

    if ( ximage->bitmap_bit_order != NATIVE_BIT_ORDER )	/*ram*/
	ConvertImageNative (ximage);			/*ram*/

    switch (file_color) {
	case SAVE_COLOR :
	    bits_per_pixel = 24;
	    spectral_mapping = ImgK_RgbMap;
	    image_data = Expand_Image_Data_Cmap_To_RGB (ximage->data,
							ximage->width,
							ximage->height,
							ximage->bytes_per_line,
							&bytes, colors_used,
							&num_colors_used);
	    if (image_data == 0) {
		return (K_FAILURE);
	    }
	    new_image_data = TRUE;
	    im_stype = ImgK_StypeMultispect;
	    break;
	case SAVE_GRAY :
	    bits_per_pixel = 8;
	    spectral_mapping = ImgK_MonochromeMap;
	    bytes = ximage->bytes_per_line;
	    image_data = (unsigned char *)ximage->data;
	    im_stype = ImgK_StypeGreyscale;
	    break;
	case SAVE_BW :
	    bits_per_pixel = 1;
	    spectral_mapping = ImgK_MonochromeMap;
	    if (pdepth != 1) {
		image_data = Convert_Image_Data_Cmap_To_BW (ximage->data,
							    ximage->width,
							    ximage->height,
							    ximage->bytes_per_line,
							    &bytes, 4, &status);
		if (status != K_SUCCESS) {
		    return (status);
		}
		new_image_data = TRUE;
	    }
	    else {
		bytes = ximage->bytes_per_line;
		image_data = (unsigned char *)ximage->data;
	    }
	    im_stype = ImgK_StypeBitonal;
	    break;
    }

    status = K_SUCCESS; /* dl - 10/6/88 assume success */
    pixel_order = ImgK_StandardPixelOrder; 

    pixels_per_scanline = ximage-> width;
    scanline_count = ximage -> height;
    scanline_stride = 8 * bytes;	/* 8 bits per byte */
    image_size = bytes * scanline_count;
    
    Find_BMU_Scale_Factor( &width_factor, &height_factor );
    bmu_x = pixels_per_scanline * width_factor;
    bmu_y = scanline_count * height_factor;
    start_set_itemlist(set_attributes, set_index);
    put_set_item(set_attributes, set_index, Img_FrmBoxLlX, zero );
    put_set_item(set_attributes, set_index, Img_FrmBoxLlY, zero );
    put_set_item(set_attributes, set_index, Img_FrmBoxUrX, bmu_x );
    put_set_item(set_attributes, set_index, Img_FrmBoxUrY, bmu_y );

    put_set_item(set_attributes, set_index, Img_PixelOrder, pixel_order);
    put_set_item(set_attributes, set_index, Img_BitsPerPixel, bits_per_pixel);
    put_set_item(set_attributes, set_index, Img_PixelsPerLine, pixels_per_scanline);
    put_set_item(set_attributes, set_index, Img_NumberOfLines, scanline_count);
    put_set_item(set_attributes, set_index, Img_ScanlineStride, scanline_stride);
    put_set_item(set_attributes, set_index, Img_SpectralMapping, spectral_mapping);

    if (file_color == SAVE_COLOR) {
	bits_per_comp = 8;
	quant_levels_per_comp = 256;

	put_set_item(set_attributes, set_index, Img_BitsPerComp, bits_per_comp);
	put_set_item(set_attributes, set_index, Img_BitsPerComp, bits_per_comp);
	set_attributes[set_index].PutL_Index = 1;
	put_set_item(set_attributes, set_index, Img_BitsPerComp, bits_per_comp);
	set_attributes[set_index].PutL_Index = 2;
	put_set_item(set_attributes, set_index, Img_PixelStride, bits_per_pixel);
	put_set_item(set_attributes, set_index, Img_PlaneBitsPerPixel, bits_per_pixel);
	put_set_item(set_attributes, set_index, Img_QuantLevelsPerComp, quant_levels_per_comp);
	put_set_item(set_attributes, set_index, Img_QuantLevelsPerComp, quant_levels_per_comp);
	set_attributes[set_index].PutL_Index = 1;
	put_set_item(set_attributes, set_index, Img_QuantLevelsPerComp, quant_levels_per_comp);
	set_attributes[set_index].PutL_Index = 2;

/* Get the colors actually used */
	for (i = 0; i < num_colors_used; i++) {
	    ddif_colors[i].red = colormap[colors_used[i]].i_red;
	    ddif_colors[i].green = colormap[colors_used[i]].i_green;
	    ddif_colors[i].blue = colormap[colors_used[i]].i_blue;
	}
    }

    end_set_itemlist(set_attributes, set_index);

    image_id = ImgCreateFrame(set_attributes, im_stype);

    ImgImportBitmap(image_id, (char *)image_data, image_size, 0, 0, 0, 0);

    if (file_color == SAVE_GRAY) {
/* Remap pixels so colormap indexes map to grey values */
	for (i = 0; i < MAX_COLORS; i++) {
	    colors_used[i] = FALSE;
	}
/* include black and white */
	colors_used[0] = TRUE;
	colors_used[MAX_COLORS - 1] = TRUE;
	num_colors_used = 2;
	for (i = 0; i < num_colors; i++) {
	    if (visual_info->class == GrayScale) {
		pixel_remap[colormap[i].pixel] = colormap[i].i_green >> 8;
	    }
	    else {				
		pixel_remap[colormap[i].pixel] = 
		    Convert_Color_To_Grey (&(colormap[i]));
	    }
	    if (!(colors_used[pixel_remap[colormap[i].pixel]])) {
		colors_used[pixel_remap[colormap[i].pixel]] = TRUE;
		num_colors_used++;
	    }
	}
	  
        array_descriptor.dsc$w_length = sizeof(unsigned char);
        array_descriptor.dsc$b_dtype = DSC_K_DTYPE_BU;
        array_descriptor.dsc$b_class = DSC_K_CLASS_A;
        array_descriptor.dsc$a_pointer = (char *)pixel_remap;

        array_descriptor.dsc$b_scale = 0;
        array_descriptor.dsc$b_digits = 0;
        array_descriptor.dsc$b_aflags = 0;

        array_descriptor.dsc$b_dimct = 1;
        array_descriptor.dsc$l_arsize = sizeof(pixel_remap);

	ISL_ERROR_HANDLER_SETUP
	new_id = ImgPixelRemap (image_id, (unsigned long)&array_descriptor, 0, 0, 0, 0, 0);
	if (failure_occurred) {
	    ISL_RECOVER_FROM_ERROR
	    ImgDeleteFrame (image_id);
	    return (K_FAILURE);
	}
	if (image_id != new_id)
	    ImgDeleteFrame (image_id);
	image_id = new_id;

/* create a historgram */
	ISL_ERROR_HANDLER_SETUP
	hist_id = (struct HCB *)ImgCreateHistogram (image_id, 0,
				    ImgK_HtypeImplicitIndex,
				    0, 0);

	if (failure_occurred)
	{
	    ISL_RECOVER_FROM_ERROR
	    hist_id = 0;
	}
	else {
	    pf_table = (long int *)hist_id->HcbA_TablePointer;
	}

/* find the colors actually used */
	for (i = 0, j = 0; i < MAX_COLORS; i++) {
	    if (colors_used[i]) {
		skip = FALSE;
		if (hist_id) {
		    if ((pf_table[i] == 0) &&
			(i != 0) && (i != (MAX_COLORS - 1))) {
			skip = TRUE;
			num_colors_used--;
		    }
		}
		if (!skip) {
		    ddif_colors[j].red = i << 8;
		    ddif_colors[j].green = i << 8;
		    ddif_colors[j].blue = i << 8;
		    j++;
		}
	    }
	}
    }

    if ((file_color == SAVE_GRAY) || (file_color == SAVE_COLOR)) {
/* put in the private data - # of colors and list of colors */
	start_set_itemlist(set_attributes, set_index);
/*jj*/
	set_index++;
	set_attributes[set_index].PutL_Code = Img_UserLabel;
	set_attributes[set_index].PutL_Length = sizeof(ddif_marker);
	set_attributes[set_index].PutA_Buffer = ddif_marker;
	set_attributes[set_index].PutL_Index = 0;
/*
	put_set_item(set_attributes, set_index, Img_UserLabel, ddif_marker);
*/
	put_set_item(set_attributes, set_index, Img_UserLabel, num_colors_used);
	set_attributes[set_index].PutL_Index = 1;
/*jj*/
	set_index++;
	set_attributes[set_index].PutL_Code = Img_UserLabel;
	set_attributes[set_index].PutL_Length = sizeof(ddif_colors);
	set_attributes[set_index].PutA_Buffer = (char *)ddif_colors;
	set_attributes[set_index].PutL_Index = 2;
/*
	put_set_item(set_attributes, set_index, Img_UserLabel, ddif_colors);
	set_attributes[set_index]PutL_Index = 2;
*/
	end_set_itemlist(set_attributes, set_index);
	ImgSetFrameAttributes (image_id, set_attributes);
    }

/* scanline count > 25 bit is a hack so ISL won't crash */
/*
    if ((file_color == SAVE_BW) && (scanline_count > 25)) 
	image_id = ImgCompress(image_id, 0, 0);
*/
#ifdef USE_COMPRESSION
    ISL_ERROR_HANDLER_SETUP
    ImgCompress (image_id, 0, 0);
    if (failure_occurred) {
/* just couldn't compress it no big deal, keep going */
	ISL_RECOVER_FROM_ERROR
    }
#endif /* USE_COMPRESSION */
                             
/* if ret_img_id then just return the image id, else, send it to the file. */
    if (ret_img_id) {
	*ret_img_id = image_id;
	return (status);
    }

/* dl - 10/6/88 */
    ISL_ERROR_HANDLER_SETUP
    context = ImgOpenFile(ImgK_ModeExport, ImgK_FtypeDDIF, 
    			  strlen(cur_file), cur_file, 0, 0);
    if (failure_occurred){
      ISL_RECOVER_FROM_ERROR
      status = K_CANNOT_OPEN;
    }
    else{
      ISL_ERROR_HANDLER_SETUP
      ImgExportFrame(image_id, context, 0);
      if ( failure_occurred )
      {
    	ISL_RECOVER_FROM_ERROR
    	status = K_CANNOT_WRITE;
      }

      ISL_ERROR_HANDLER_SETUP
      ImgCloseFile(context, 0);
      if ( failure_occurred )
      {
    	ISL_RECOVER_FROM_ERROR
    	status = K_CANNOT_CLOSE;
      }
    }
    ImgDeleteFrame( image_id );
    if (new_image_data)
    {
	XtFree ((char *)image_data);
    }
    return( status );  /* dl - 10/6/88 */
}

Restore_Colormap (cmap, num, private)
    MyColor *cmap;
    int num;
    int private;
{
    int i, j, k, kk;
    unsigned long extra_pixels[256], used_pixels[256], pixels[2], dummy;
    XColor xcolor;

    Get_Default_Colormap ();
    if (!private) {
	k = kk = 0;
	for (i = 2; i < num; ) {
	    if (!XAllocColorCells (disp, paint_colormap, 0, &dummy, 0, pixels, 1)) {
		private = TRUE;
		break;
	    }
	    else {
		for (j = 2; j < num; j++) {
		    if (cmap[j].pixel == pixels[0]) {
			used_pixels[kk] = pixels[0];
			kk++;
			i++;
			break;
		    }
		}
		if (j >= num) {
		    extra_pixels[k] = pixels[0];
		    k++;
		}
	    }
	}
	
	if (k > 0) {
	    XFreeColors (disp, default_colormap, extra_pixels, k, 0);
	}
	if (private && (kk > 0)) {
	    XFreeColors (disp, default_colormap, used_pixels, kk, 0);
	}
    }

    if (private) {
	Create_Private_Colormap ();
    }
    
    for (i = 2; i < num; i++) {
	xcolor.pixel = cmap[i].pixel;
        xcolor.flags = DoRed | DoGreen | DoBlue;
	xcolor.red = cmap[i].i_red;
	xcolor.green = cmap[i].i_green;
	xcolor.blue = cmap[i].i_blue;

	XStoreColor (disp, paint_colormap, &xcolor);

	colormap[i].f_red = cmap[i].f_red;	
	colormap[i].f_green = cmap[i].f_green;	
	colormap[i].f_blue = cmap[i].f_blue;	
	colormap[i].i_red = cmap[i].i_red;	
	colormap[i].i_green = cmap[i].i_green;	
	colormap[i].i_blue = cmap[i].i_blue;	
	colormap[i].pixel = cmap[i].pixel;	
    }
    num_colors = num;
}

Save_Colormap (cmap, num, private)
    MyColor *cmap;
    int *num;
    int *private;
{
    int i;

    for (i = 2; i < num_colors; i++)
    {
	cmap[i].pixel = colormap[i].pixel;
	cmap[i].i_red = colormap[i].i_red;
	cmap[i].i_green = colormap[i].i_green;
	cmap[i].i_blue = colormap[i].i_blue;
	cmap[i].f_red = colormap[i].f_red;
	cmap[i].f_green = colormap[i].f_green;
	cmap[i].f_blue = colormap[i].f_blue;
    }
    *num = num_colors;
    if (paint_colormap == default_colormap) {
	*private = FALSE;
    }
    else {
	*private = TRUE;
    }
}

XImage *Read_DDIF(status, picture_altered, context)
    int *status;
    int *picture_altered;
    unsigned long context;
{                 
    unsigned char *image_data;
    buf_descriptor_type array_descriptor;
	
    int big_dim = printer_resolution * MAX (printer_page_wd, printer_page_ht);
    int small_dim = printer_resolution * MIN (printer_page_wd, printer_page_ht);

    float Convert_Color_I_To_F ();
    int tmp_num_colors;
    int tmp_private;
    MyColor tmp_colormap[MAX_COLORS], color;
    XColor xcolor, xcolors[MAX_COLORS];
    PF_ENTRY_TYPE *pf_table;
    int pf_entries;
    int remap_pixels = FALSE;
    int color_entry;
    char marker[10];
    int num_colors_used;
    DDIF_COLOR_TYPE ddif_colors[MAX_COLORS];
    int ddif_remap[MAX_COLORS];

    int restore_paint_saved_image = FALSE;
    int i, j, k;
    int best, best_ind, tmp;
    unsigned long image_id;
    unsigned long new_id;
    int get_index, set_index, requantize_index;
    struct GET_ITMLST get_attributes[20];
    struct PUT_ITMLST set_attributes[5];
    int context_supplied = FALSE;
    XImage *ximage, *img;
    float scale_factor;
    float xscale, yscale;
    int compress;
    int spectral_mapping;
    int brt_polarity, brt_polarity_in;
    int ll_x_in, ll_y_in, ur_x_in, ur_y_in;
    int ll_x, ll_y, ur_x, ur_y;
    int bytes, image_size;
    unsigned char *plane_data, *dp_addr;
    unsigned long ul_zero = 0;
    int ss;
    int fg;
    int pixel_stride;

    int num_to_change, num_already_used;
    int special[NUM_SPECIAL_COLORS], already_used[NUM_SPECIAL_COLORS + 2];
    int num_special;
    int go_for_it, extra_bw;

/*    int psid;*/
    unsigned long psid; /* put in on 11/16/93 as IdsCreateRendering defines it
				as a pointer */

    IdsRendering *rendering;
    IdsItmlst2 ps_itmlst[4];
    IdsItmlst2 rnd_itmlst[4];

    struct HCB *hist_id;
    
    unsigned char *new_image_data;
    int bytes_per_line;

    int AC_status;
    int save_colormap_size;

    unsigned char *Convert_Image_Data_RGB_To_Cmap();
    unsigned char *Expand_Image_Data();
/* end of variable section */

/* Assume success */
    *status = K_SUCCESS;

/* Assume the picture will not have to be altered */
    *picture_altered = FALSE;

    if (context) {
	context_supplied = TRUE;
    }

    start_get_itemlist(get_attributes, get_index);
    put_get_item(get_attributes, get_index, Img_BitsPerPixel, bits_per_pixel);
    put_get_item(get_attributes, get_index, Img_PixelsPerLine, pixels_per_scanline);
    put_get_item(get_attributes, get_index, Img_NumberOfLines, scanline_count);
    put_get_item(get_attributes, get_index, Img_ScanlineStride, scanline_stride);
    put_get_item(get_attributes, get_index, Img_CompressionType, compress );
    put_get_item(get_attributes, get_index, Img_SpectralMapping, spectral_mapping);
    put_get_item(get_attributes, get_index, Img_BrtPolarity, brt_polarity);
    put_get_item(get_attributes, get_index, Img_DataPlaneBase, plane_data);
    put_get_item(get_attributes, get_index, Img_PixelStride, pixel_stride);

    put_get_item(get_attributes, get_index, Img_FrmBoxLlX, ll_x);
    put_get_item(get_attributes, get_index, Img_FrmBoxLlY, ll_y);
    put_get_item(get_attributes, get_index, Img_FrmBoxUrX, ur_x);
    put_get_item(get_attributes, get_index, Img_FrmBoxUrY, ur_y);

/*jj*/
    get_index++;
    get_attributes[get_index].GetL_Code = Img_UserLabel;
    get_attributes[get_index].GetL_Length = sizeof(marker);
    get_attributes[get_index].GetA_Buffer = marker;
    get_attributes[get_index].GetA_Retlen = 0; 
    get_attributes[get_index].GetL_Index = 0;
/*
    put_get_item(get_attributes, get_index, Img_UserLabel, marker);
*/
    put_get_item(get_attributes, get_index, Img_UserLabel, num_colors_used);
    get_attributes[get_index].GetL_Index = 1;

/*jj*/
    get_index++;
    get_attributes[get_index].GetL_Code = Img_UserLabel;
    get_attributes[get_index].GetL_Length = sizeof(ddif_colors);
    get_attributes[get_index].GetA_Buffer = (char *)ddif_colors;
    get_attributes[get_index].GetA_Retlen = 0; 
    get_attributes[get_index].GetL_Index = 2;
/*
    put_get_item(get_attributes, get_index, Img_UserLabel, ddif_colors);
    get_attributes[get_index].index = 2;
*/
    end_get_itemlist(get_attributes, get_index);

    if (!context_supplied) {
	ISL_ERROR_HANDLER_SETUP
	context = ImgOpenDDIFFile(ImgK_ModeImport, strlen(temp_file), temp_file,
				  strlen(temp_file), temp_file, 0);
	if (failure_occurred) {
	    ISL_RECOVER_FROM_ERROR
	    *status = K_ISL_FAILURE;
	    return (0);
	}
    }

    ISL_ERROR_HANDLER_SETUP
    image_id = ImgImportDDIFFrame(context, 0, 0, 0, 0, 0);

    if (!context_supplied) {
	ImgCloseDDIFFile(context, 0);
    }

    if (failure_occurred)
    {
	ISL_RECOVER_FROM_ERROR
	*status = K_ISL_FAILURE;
	return (0);
    }

    if (image_id == 0)
    {
	*status = K_NO_IMAGE;
	return (0);
    }

    ImgGetFrameAttributes(image_id, get_attributes); /* jj-port */
    brt_polarity_in = brt_polarity;
    ll_x_in = ll_x;
    ll_y_in = ll_y;
    ur_x_in = ur_x;
    ur_y_in = ur_y;

    switch (visual_info->class) {
	case StaticGray :
	    new_file_color = SAVE_BW;
	    break;
	case GrayScale :
	    if (bits_per_pixel == 1) {
		new_file_color = SAVE_BW;
	    }
	    else {
		new_file_color = SAVE_GRAY;
	    }
	    break;
	default :
	    if (spectral_mapping == ImgK_MonochromeMap) {
		if (bits_per_pixel == 1) {
		    new_file_color = SAVE_BW;
		}
		else {
		    new_file_color = SAVE_GRAY;
		}
	    }
	    else {
		new_file_color = SAVE_COLOR;
	    }
	    break;
    }


/* see if image is too big */
    if ((pixels_per_scanline > big_dim) || (scanline_count > big_dim) ||
	((pixels_per_scanline > small_dim) && (scanline_count > small_dim)) ||
	(context_supplied &&
	 ((pixels_per_scanline > screen_wd) ||(scanline_count > screen_ht)))) 
    {
	*status = K_IMAGE_TOO_BIG;
	ImgDeleteFrame (image_id);
	return (0);
    }

/* decompress if necessary */
    if( compress != ImgK_PcmCompression )
    {
	ISL_ERROR_HANDLER_SETUP

	image_id = ImgDecompress (image_id);

	if (failure_occurred)
	{
	    ISL_RECOVER_FROM_ERROR
	    *status = K_NO_DECOMPRESS;
	    return (0);
	}
    }

    if ((Pad_Image (image_id, &new_id, FALSE)) == 0)
    {
	*status = K_NO_PAD_IMAGE;
	ImgDeleteFrame (image_id);
	return (0);
    }

    if (image_id != new_id)
    {
	ImgDeleteFrame (image_id);
    }
    image_id = new_id;

    if ((pdepth > 1) && (!context_supplied))
    {
/* save the colormap */
	Save_Colormap (tmp_colormap, &tmp_num_colors, &tmp_private);
	Get_Default_Colormap ();
    }

/* set up the pixel_remap to default to nothing */
    for (i = 0; i < MAX_COLORS; i++)
	pixel_remap[i] = i;

    if (strcmp (marker, ddif_marker) == 0)
    {
	if ((num_colors_used <= colormap_size) && 
	    (pdepth != 1))
	{
	    if ((visual_info->class != GrayScale) ||
		(spectral_mapping == ImgK_MonochromeMap))
	    {
		restore_paint_saved_image = TRUE;
	    }
	}
    }

    if (!restore_paint_saved_image)
    {
	if (spectral_mapping == ImgK_ColorMap)
	{
	    if (pdepth > 1)
	    {
		*status = Read_Color_File ();
		if (*status != K_SUCCESS)
		{
		    if (!context_supplied) {
			Restore_Colormap (tmp_colormap, tmp_num_colors,
					  tmp_private);
		    }
		    return (0);
		}
		remap_pixels = TRUE;
	    }
	}
	else
	{
	    if (bits_per_pixel != 1)
	    {
/* Create the Present Surface */
		ps_itmlst[0].item_name= (char *) IdsNprotocol;
		ps_itmlst[0].value = (unsigned long int) Ids_Fid;
/*		ps_itmlst[0].value = (unsigned long int) Ids_XImage; */
		ps_itmlst[1].item_name = (char *) IdsNworkstation;
		ps_itmlst[1].value = (unsigned long int) disp;
		ps_itmlst[2].item_name = (char *) IdsNwsWindow;
		ps_itmlst[2].value = (unsigned long int) XDefaultRootWindow (disp);
		ps_itmlst[3].item_name = (char *) NULL;
		ps_itmlst[3].value = (unsigned long int) NULL;

		ISL_ERROR_HANDLER_SETUP
		psid = IdsCreatePresentSurface (ps_itmlst);

		if (failure_occurred)
		{
		    ISL_RECOVER_FROM_ERROR
		    if (pdepth > 1)
		    {
			if (!context_supplied) {
			    Restore_Colormap (tmp_colormap, tmp_num_colors,
					      tmp_private);
			}
		    }
		    *status = K_NO_PRESENT_SURFACE;
		    return (0);
		}

/* Create the Rendering */
		rnd_itmlst[0].item_name = (char *) IdsNscaleMode;
		rnd_itmlst[0].value = (unsigned long int) Ids_NoScale;
		rnd_itmlst[1].item_name = (char *) NULL;
		rnd_itmlst[1].value = (unsigned long int) NULL;

		ISL_ERROR_HANDLER_SETUP
		rendering = IdsCreateRendering (image_id, psid, rnd_itmlst);
		if (failure_occurred)
		{
		    ISL_RECOVER_FROM_ERROR
		    if (pdepth > 1)
		    {
			if (!context_supplied) {
			    Restore_Colormap (tmp_colormap, tmp_num_colors,
					      tmp_private);
			}
		    }
		    IdsDeleteRendering (rendering);
		    *status = K_NO_RENDERING;
		    return (0);
		}

		new_id = rendering->rndfid;
		if (image_id != new_id) {
		    ImgDeleteFrame (image_id);
/* pad the new image */
		    if ((Pad_Image (new_id, &image_id, TRUE)) == 0)
		    {
			*status = K_NO_PAD_IMAGE;
			ImgDeleteFrame (new_id);
			IdsDeletePresentSurface (psid);
			IdsDeleteRendering (rendering);
			return (0);
		    }
		}

		*picture_altered = TRUE;

		IdsDeletePresentSurface (psid);
		IdsDeleteRendering (rendering);

		if (pdepth > 1)
		{
		    ImgGetFrameAttributes(image_id, get_attributes);

/* make sure it pixel stride is 8 bits */
		    if (pixel_stride != 8) {
			bytes_per_line = pixels_per_scanline;
			while (bytes_per_line % 4) {
			    bytes_per_line++;
			}
			bytes = bytes_per_line * scanline_count;
			ss = bytes_per_line * 8;

			ISL_ERROR_HANDLER_SETUP
			dp_addr = (unsigned char *) ImgAllocateDataPlane (bytes, ul_zero);

			if (failure_occurred)
			{
			    ISL_RECOVER_FROM_ERROR
			    if (!context_supplied) {
				Restore_Colormap (tmp_colormap, tmp_num_colors,
						  tmp_private);
			    }
			    ImgDeleteFrame (image_id);
			    *status = K_NO_CLIENT_MEMORY;
			    return (0);
			}

			if (pixel_stride == 4)
			    Remap_4_To_8 (plane_data, dp_addr,
					  scanline_stride / 8,
					  bytes_per_line,
					  pixels_per_scanline,
					  scanline_count);
			else if (pixel_stride == 2)
			    Remap_2_To_8 (plane_data, dp_addr,
					  scanline_stride / 8,
					  bytes_per_line,
					  pixels_per_scanline,
					  scanline_count);
			else {
			    ImgDeleteFrame (image_id);
			    if (!context_supplied) {
				Restore_Colormap (tmp_colormap, tmp_num_colors,
						  tmp_private);
			    }
			    *status = K_UNSUPPORTED_FORMAT;
			    return (0);
			}

			pixel_stride = 8;
			start_set_itemlist (set_attributes, set_index);
			put_set_item (set_attributes, set_index,
				      Img_ScanlineStride, ss);
			put_set_item (set_attributes, set_index,
				      Img_PixelStride, pixel_stride);
			end_set_itemlist(set_attributes, set_index);

			XtFree ((char *)plane_data);
			plane_data = NULL;
			ImgSetFrameAttributes (image_id, set_attributes);
			ImgStoreDataPlane (image_id, (char *)dp_addr);
			ImgGetFrameAttributes(image_id, get_attributes);
		    }

/* create a historgram */
		    ISL_ERROR_HANDLER_SETUP
		    hist_id = (struct HCB *)ImgCreateHistogram (image_id, 0,
						ImgK_HtypeExplicitIndex,
						ImgM_SortByFreq, 0);

		    if (failure_occurred)
		    {
			ISL_RECOVER_FROM_ERROR
			if (!context_supplied) {
			    Restore_Colormap (tmp_colormap, tmp_num_colors,
					      tmp_private);
			}
			*status = K_NO_HISTOGRAM;
			return (0);
		    }

		    pf_entries = hist_id->HcbL_EntryCount;
		    pf_table = (PF_ENTRY_TYPE *)hist_id->HcbA_TablePointer;

/* go backwards so we will get the most frequently used colors first */
		    ImgGetFrameAttributes(image_id, get_attributes);
		    for (i = pf_entries -1, j = 0; i >= 0; i--, j++)
		    {
			Convert_Pixel_To_DDIF_Color (pf_table[i].pixel,
						     &(ddif_colors[j]),
						     spectral_mapping,
						     bits_per_pixel,
						     brt_polarity);
		    }
		    num_colors_used = pf_entries;
		}
	    } /* end if (bits_per_pixel != 1) [ */
	} /* end if (spectral_mapping == ImgK_ColorMap) [] else [ */
    } /* end if (!restore_paint_saved_image) [ */

    if ((pdepth > 1) && (bits_per_pixel > 1))
    {
/* if we need to use some special indexes */
	if ((!context_supplied) &&
	    (num_colors_used >
	     (visual_info->colormap_size - num_special_indexes)))
	{
	    num_to_change = num_colors_used -
			    (visual_info->colormap_size - num_special_indexes);
	    num_already_used = 0;
	    /* find black and white indexes */
	    for (i = 0;
		 ((i < num_colors_used) && (num_already_used < 2));
		 i++)
	    {
		if (((ddif_colors[i].red >> 8  == 0xff) &&
		     (ddif_colors[i].green >> 8  == 0xff) &&
		     (ddif_colors[i].blue >> 8  == 0xff)) ||
		    ((ddif_colors[i].red >> 8  == 0) &&
		     (ddif_colors[i].green >> 8  == 0) &&
		     (ddif_colors[i].blue >> 8  == 0)))
		{
		    already_used[num_already_used] = i;
		    num_already_used++;
		}
	    }
	    for (i = (num_special_indexes - 1), j = 0;
		 j < num_to_change;
		 i--, j++)
	    {
		xcolor.pixel = special_indexes[i];
		XQueryColor (disp, default_colormap, &xcolor);
		special[j] = Find_Closest_Color_2 (&xcolor, ddif_colors,
						 num_colors_used, already_used,
						 num_already_used);
		already_used[num_already_used] = special[j];
		num_already_used++;
	    }
	}
	else
	{
	    num_to_change = 0;
	}

/* if reading in an image for a paste, do not stomp on the special indexes */
	if (context_supplied) {
	    save_colormap_size = colormap_size;
	    colormap_size = 
		MAX(visual_info->colormap_size - num_special_indexes,
		    num_colors);
	}

/* set up new colors */
	for (i = 0; i < num_colors_used; i++)
	{
	    go_for_it = TRUE;
	    for (j = 0; ((j < num_to_change) && (go_for_it)); j++)
	    {
		if (i == special[j])
		{
		    go_for_it = FALSE;
		}
	    }
	    if (go_for_it)
	    {
		color.i_red = ddif_colors[i].red;
		color.i_green = ddif_colors[i].green;
		color.i_blue = ddif_colors[i].blue;
		AC_status = Add_Color (&color, FALSE, &color_entry);
		if (AC_status == K_CLOSEST_COLOR) {
		    *picture_altered = TRUE;
		}
		ddif_remap[i] = color_entry;
	    }
	}
/* fill in special colors */
	for (i = 0; i < num_to_change; i++)
	{
	    color.i_red = ddif_colors[special[i]].red;
	    color.i_green = ddif_colors[special[i]].green;
	    color.i_blue = ddif_colors[special[i]].blue;
	    AC_status = Add_Color (&color, FALSE, &color_entry);
	    if (AC_status == K_CLOSEST_COLOR) {
		*picture_altered = TRUE;
	    }
	    ddif_remap[special[i]] = color_entry;
	}

/* reset the colormap size */
	if (context_supplied) {
	    colormap_size = save_colormap_size;
	}

/* if the image has been fed throught IDS or */
/* if restoring to a greyscale server */
	if ((!restore_paint_saved_image ) ||
	    (spectral_mapping == ImgK_MonochromeMap))
	{
	    for (i = 0; i < num_colors_used; i++)
	    {
		if (restore_paint_saved_image)
		{
		    j = ddif_colors[i].green >> 8;
		}
		else
		{
		    j = pf_table[num_colors_used - i - 1].pixel;
		}
		pixel_remap[j] = colormap[ddif_remap[i]].pixel;
	    }
	    remap_pixels = TRUE;
	}
    } /* end if (pdepth > 1) [ */

/* remap pixels if necessary */
    if (remap_pixels)
    {
	if (bits_per_pixel != 8) {
	    bits_per_pixel = 8;
	    start_set_itemlist (set_attributes, set_index);
	    put_set_item (set_attributes, set_index,
			  Img_BitsPerPixel, bits_per_pixel);
	    end_set_itemlist(set_attributes, set_index);
	    ImgSetFrameAttributes (image_id, set_attributes);
	}

        array_descriptor.dsc$w_length = sizeof(unsigned char);
        array_descriptor.dsc$b_dtype = DSC_K_DTYPE_BU;
        array_descriptor.dsc$b_class = DSC_K_CLASS_A;
        array_descriptor.dsc$a_pointer = (char *)pixel_remap;

        array_descriptor.dsc$b_scale = 0;
        array_descriptor.dsc$b_digits = 0;
        array_descriptor.dsc$b_aflags = 0;

        array_descriptor.dsc$b_dimct = 1;
        array_descriptor.dsc$l_arsize = sizeof(pixel_remap);
	ISL_ERROR_HANDLER_SETUP
	new_id = ImgPixelRemap (image_id, (unsigned long)&array_descriptor, 0, 0, 0, 0, 0);
	if (failure_occurred)
	{
	    ISL_RECOVER_FROM_ERROR
/* get old colors back */
	    if (!context_supplied) {
		Restore_Colormap (tmp_colormap, tmp_num_colors, tmp_private);
	    }
	    *status = K_NO_COLOR_REMAP;
	    return (0);
	}
	if (image_id != new_id)
	    ImgDeleteFrame (image_id);
	image_id = new_id;
    } /* end if (remap_pixels) [ */

    ImgGetFrameAttributes(image_id, get_attributes);
    bytes = scanline_stride/8;
    image_data = (unsigned char *) XtMalloc (scanline_count * bytes);

    if (image_data == 0)
    {
	*status = K_NO_CLIENT_MEMORY;
	ImgDeleteFrame (image_id);
	if (pdepth > 1)
	{
	    if (!context_supplied) {
		Restore_Colormap (tmp_colormap, tmp_num_colors, tmp_private);
	    }
	}
	return (0);
    }
    ImgExportBitmap(image_id, 0, (char *)image_data, (scanline_count * bytes),
		      0, 0, 0, 0);

    if ((restore_paint_saved_image) &&
	(spectral_mapping != ImgK_MonochromeMap))
    {
	new_image_data = Convert_Image_Data_RGB_To_Cmap (image_data,
							 pixels_per_scanline,
							 scanline_count,
							 bytes, &bytes,
							 ddif_colors,
							 num_colors_used,
							 ddif_remap,
							 status);
	if (new_image_data == NULL)
	{
	    ImgDeleteFrame (image_id);
	    XtFree ((char *)image_data);
	    image_data = NULL;
	    if (!context_supplied)
	    {
		Restore_Colormap (tmp_colormap, tmp_num_colors, tmp_private);
	    }
	    return (0);
	}
	else
	{
	    XtFree ((char *)image_data);
	    image_data = new_image_data;
	}
    }
    else
    {
/* sort of a hack. If the incoming image has brightness polarity of 
   zero_min_intensity and either of the display or the incoming image is 
   monochrome, need to flip the bits of the image */

	if ((brt_polarity_in == ImgK_ZeroMinIntensity) && (pdepth == 1))
	{
	    Invert_Image_Data (image_data, pixels_per_scanline,
			       scanline_count, bytes);
	}					    

	if ((bits_per_pixel == 1) && (pdepth > 1))
	{
	    if (brt_polarity_in == ImgK_ZeroMinIntensity)
	    {
		fg = FALSE;
	    }
	    else
	    {
		fg = TRUE;
	    }
	    new_image_data = Expand_Image_Data (image_data,
						pixels_per_scanline,
						scanline_count, bytes,
						&bytes_per_line, fg);
	
            if (new_image_data == 0)
	    {
                *status = K_NO_CLIENT_MEMORY;
                ImgDeleteFrame (image_id);
                XtFree ((char *)image_data);
		image_data = NULL;
                if (!context_supplied)
		{
                    Restore_Colormap (tmp_colormap, tmp_num_colors,
                                      tmp_private);
                }
                return (0);
            }
 
	    XtFree ((char *)image_data);
	    image_data = new_image_data;
	    bits_per_pixel = 8;
	    bytes = bytes_per_line;
	}
    } /* end if ((restore_paint_saved_image) && ()) [] else [ */

    ximage = XCreateImage (disp, visual_info->visual,
			   (bits_per_pixel == 1) ? 1 : pdepth, 
			   img_format, 0, (char *)image_data,
			   pixels_per_scanline, scanline_count, 32,
			   bytes);
    ximage->bitmap_bit_order = NATIVE_BIT_ORDER;    /*ram*/
    ximage->byte_order = NATIVE_BYTE_ORDER;	    /*ram*/

/* set the resolution */
    if (!context_supplied) {
	if (((ur_x_in - ll_x_in) <= 0) ||
	    ((pixels_per_scanline * BMU) < (ur_x_in - ll_x_in)))
	{
	    resolution = screen_resolution;
	    Display_Message ("T_NO_RESOLUTION");
	}
	else
	{
	    resolution = (pixels_per_scanline * BMU) / (ur_x_in - ll_x_in);
	}
    }

    ImgDeleteFrame (image_id);
    return (ximage);
}



/*
 *
 * ROUTINE:  Find_Format
 *
 * ABSTRACT: 
 *         
 *  Determine the file format of the file to be read in.
 *
 */
int Find_Format()
{
    int type;
    int x_hot, y_hot;
    int status;
    unsigned long context;
    int frame_id;
    CDAitemlist	    process_option_list [3];
    unsigned int    cda_status;
    CDAaggtype	    aggregate_type;
    CDAsize	    file_name_len;
    DDISstreamhandle stream_handle;
    CDAfilehandle    file_handle;
    CDArootagghandle root_handle;

/* First check to see if it is a DDIF file */
    file_name_len = strlen (temp_file);

    MAKE_ITEM (process_option_list, 0 ,DDIF_INHERIT_ATTRIBUTES, 0, 0);
    MAKE_ITEM (process_option_list, 1, DDIF_EVALUATE_CONTENT, 0, 0);
    MAKE_ITEM (process_option_list, 2, 0, 0, 0);

    aggregate_type = DDIF_DDF;  /* Root aggregate type */
    cda_status = cda_open_file (&file_name_len, temp_file,
                        NULL,  /* Default file specification length - optional */
                        NULL,  /* Default file specification - optional */
                        NULL,  /* Default memory_alloc_proc        */
                        NULL,  /* Default memory_dealloc_proc      */
                        NULL,  /* Default alloc_dealloc_context    */
                        &aggregate_type,
                        process_option_list,
                        NULL,  /* Default result_filename_len      */
                        NULL,  /* Default result_filename          */
                        NULL,  /* Default result_filename_ret_len  */
                        &stream_handle,
                        &file_handle,
                        &root_handle);

    if (cda_status == CDA_NORMAL) {
	cda_status = cda_close_file (&stream_handle, &file_handle);
	ISL_ERROR_HANDLER_SETUP
	context = ImgOpenDDIFFile(ImgK_ModeImport, strlen(temp_file), temp_file,
				  strlen(temp_file), temp_file, 0);

	if (failure_occurred) {
	    ISL_RECOVER_FROM_ERROR
	}
	else 
	    ImgCloseDDIFFile(context, 0);
    }
    else
	failure_occurred = TRUE;


/* If a failure did not occur, it is a DDIF file, otherwise it may be an
 * XBitmap file
 */
    type = K_UNKNOWN_FILE_FORMAT;
    if( !failure_occurred ) {
      type = DDIF_FORMAT;
      Set_Default_Filetype( DDIF_DEFAULT_TYPE );
    }
    else {
        status = XReadBitmapFile (disp, pwindow, temp_file, &ret_wd, &ret_ht,
                                  &ret_pixmap, &x_hot, &y_hot);
        if( status == BitmapSuccess ) {
            type = XBITMAP_FORMAT;
            if ((ret_wd >= MIN_PICTURE_WD) && (ret_ht >= MIN_PICTURE_HT) &&
                (ret_wd <= XDisplayWidth (disp, screen)) &&
                (ret_ht <= XDisplayHeight (disp, screen))) {
                Set_Default_Filetype( XBITMAP_DEFAULT_TYPE );
            }
        }
    }
    return(type);
}

/*
 *
 * ROUTINE:  Write_File
 *
 * ABSTRACT: 
 *         
 *  Writes the picture out to a file.  Inputs are: cur_file and
 *  file_format
 *
 */
void Write_File()
{
    int dummy;
    int status;
    int i, j;
    int hl = FALSE;
    XImage *ximage;
    Pixmap tmp_picture;
    unsigned char *image_data;
    unsigned char *Convert_Image_Data_Cmap_To_BW();
    FILE *fp;

    if (select_on) {
	Increase_Change_Pix (select_x, select_y, select_width, select_height);
    }
    if (entering_text)
	End_Text ();

    if (write_dialog != 0) {
	if (XtIsManaged (write_dialog)) {
	    Set_Cursor_Watch (XtWindow (write_dialog));
	}
    }
    Set_Cursor_Watch (pwindow);

/*strip off the version number */
    i = strcspn( cur_file, ";" );
    cfile[i] = '\0';

/* This was a bad idea, it created an empty file */
/* see if we can open the file for a write */
/*
    fp = fopen (cur_file, "w");
    if (fp == NULL) {
	status = K_CANNOT_OPEN;
    }
    else {
	fclose (fp);
*/

/*
 * Write to DDIF file
 */
	if( file_format == DDIF_FORMAT ){
	    Set_Default_Filetype( DDIF_DEFAULT_TYPE );

/* Update the image */
	    if (paint_view == NORMAL_VIEW) {
		Update_Image (0, 0, picture_wd, picture_ht, picture_x,
			      picture_y, picture, ImgK_Src);
	    }
	    ximage = picture_image;

	    if (ximage == 0) {
		status = K_NO_CLIENT_MEMORY;
	    }
	    else {
		status = Write_DDIF (ximage, 0L);
		if (ximage != picture_image)
		{
#if 0
		    XtFree (ximage->data);
#endif
		    XDestroyImage (ximage);
		    ximage = NULL;
		    picture_image = NULL;
		}

#ifdef EPIC_CALLABLE
/* If there's a parent, tell the parent about updated result */
		if (run_as_child)
		    AppUpdateParent();
#endif

	    }
	}
	else{
/*
 * Write to XBitmap file 
 */
	    if ((pimage_wd > XDisplayWidth (disp, screen)) ||
		(pimage_ht > XDisplayHeight (disp, screen))) {
		status = K_BITMAP_TOO_LARGE;
	    }
	    else {
		status = K_SUCCESS;
		if (select_on && highlight_on) {
		    UnHighlight_Select ();
		    hl = TRUE;
		}
		Set_Default_Filetype (XBITMAP_DEFAULT_TYPE);
		if (pdepth != 1) {
		    if (paint_view == NORMAL_VIEW) {
			Update_Image (0, 0, picture_wd, picture_ht, picture_x,
				      picture_y, picture, ImgK_Src);
		    }
		    image_data = Convert_Image_Data_Cmap_To_BW (
				    picture_image->data,
				    pimage_wd, pimage_ht,
				    picture_image->bytes_per_line,
				    &dummy, 1, &status);
		    if (status == K_SUCCESS) {
			tmp_picture = XCreateBitmapFromData (disp,
					DefaultRootWindow(disp), (char*)image_data,
					pimage_wd, pimage_ht);
			if (image_data != NULL)
			{
			    XtFree ((char *)image_data);
			    image_data = NULL;
			}
			if (tmp_picture == 0)
			{
			    status = K_FAILURE;
			}
		    }
		}
		else {
		    if (paint_view == FULL_VIEW) {
			tmp_picture = Create_Pdepth_Pixmap (picture_bg,
							    picture_wd,
							    picture_ht);
			if (tmp_picture == 0) {
			    status = K_FAILURE;
			}
			else {
			    MY_XPutImage (disp, tmp_picture, Get_GC(GC_PD_COPY),
					  picture_image, picture_x, picture_y,
					  0, 0, picture_wd, picture_ht);
			}
		    }
		    else {
			tmp_picture = picture;
		    }
		}

		if (status == K_SUCCESS) {
		    status = XWriteBitmapFile (disp, cur_file, tmp_picture,
					       pimage_wd, pimage_ht, 0, 0);
		    if (status != BitmapSuccess) {
			status = K_FAILURE;
		    }
		    else {
			status = K_SUCCESS;
		    }
		    if ((picture != tmp_picture) && (tmp_picture != 0)) {
			XFreePixmap (disp, tmp_picture);
		    }
		}

		if (hl) {
		    Highlight_Select ();
		}
	    }
	}

/*
    }
*/

/* dl - 10/6/88 if the write fails, display a message */
    switch (status) {
        case K_SUCCESS :
/* picture has not been changed since last save - dl 10/5/88 */
            picture_changed = FALSE;
            Extract_Filename();
            Set_Titlebar();
	    if (write_dialog) {
		if (XtIsManaged (write_dialog)) {
		    XtUnmanageChild (write_dialog);
		}
	    }
            break;
        case K_FAILURE :
            Display_Message( "T_FILE_WRITE_ERROR" );
	    /* restore the previous file name */
	    strcpy (cur_file, last_file_name);
	    break;
        case K_CANNOT_OPEN :
            Display_Message( "T_CANNOT_OPEN_WRITE_FILE" );
	    /* restore the previous file name */
	    strcpy (cur_file, last_file_name);
    	    exiting_paint = FALSE;
            break;
        case K_CANNOT_WRITE :
            Display_Message( "T_CANNOT_WRITE_FILE" );
    	    exiting_paint = FALSE;
            break;
        case K_CANNOT_CLOSE :
            Display_Message( "T_CANNOT_CLOSE_WRITE_FILE" );
            break;
        case K_BITMAP_TOO_LARGE :
            Display_Message( "T_BITMAP_TOO_LARGE_WRITE");
	    /* restore the previous file name */
	    strcpy (cur_file, last_file_name);
            break;
        case K_PICTURE_NOT_BW:
            Display_Message ("T_PICTURE_NOT_BW");
	    /* restore the previous file name */
	    strcpy (cur_file, last_file_name);
            break;
	case K_NO_CLIENT_MEMORY :
	    Display_Message ("T_NO_MEM_FOR_SAVE");
	    break;
    }
    if (write_dialog != 0)
	XUndefineCursor (disp, XtWindow (write_dialog));
    Set_Cursor (pwindow, current_action);
}


void Display_Read_File_Message (status)
    int status;
{
    switch (status) {
	case K_NOT_BITONAL :
	    Display_Message( "T_FILE_NOT_BITONAL" );
	    break;
	case K_GREY_SCALE:
	    Display_Message( "T_FILE_GREY_SCALE" );
	    break;
	case K_IMAGE_TOO_SMALL :
	    Display_Message( "T_IMAGE_TOO_SMALL" );
	    break;
        case K_IMAGE_TOO_BIG :
            Display_Message( "T_IMAGE_TOO_BIG" );
            break;
	case K_UNKNOWN_FILE_FORMAT :
	    Display_Message ("T_ILLEGAL_FILE_FORMAT");
	    break;
	case K_CANNOT_OPEN :
	    Display_Message ("T_CANNOT_OPEN_FILE");
	    break;
	case K_ISL_FAILURE :
	    Display_Message ("T_FILE_READ_ERROR");
	    break;
	case K_NO_IMAGE :
	    Display_Message ("T_NO_IMAGE");
	    break;
	case K_NO_PAD_IMAGE :
	    Display_Message ("T_NO_PAD_IMAGE");
	    break;
	case K_NO_CLIENT_MEMORY :
	    Display_Message ("T_NO_MEM_FOR_READ");
	    break;
	case K_NO_DECOMPRESS :
	    Display_Message ("T_NO_DECOMPRESS");
	    break;
	case K_DDIF_NO_GOOD :
	    Display_Message ("T_DDIF_NO_GOOD");
	    break;
	case K_NO_COLOR_REMAP :
	case K_NO_HISTOGRAM :
	case K_NO_PRESENT_SURFACE :
	case K_NO_RENDERING :
	case K_NO_CONVERT_IMAGE_DATA :    	    
	    Display_Message ("T_NO_CONVERT_IMAGE");
	    break;
        case K_BITMAP_TOO_LARGE :
            Display_Message ("T_BITMAP_TOO_LARGE_READ");
            break;
	case K_UNSUPPORTED_FORMAT :
            Display_Message ("T_UNSUPPORTED_FORMAT");
            break;
	default :
            Display_Message ("T_FILE_READ_ERROR");
            break;
    }
}

/*
 *
 * ROUTINE:  Read_File
 *
 * ABSTRACT: 
 *
 *  Reads a file.  Inputs are: cur_file and file_format
 *
 */
void Read_File()
{
    int file_type;
    int status;
    XImage *temp_image;
    FILE *fp;
    unsigned char *image_data;
    int bytes;
    int tmp_num_colors = 0;
    int tmp_private;
    MyColor tmp_colormap[MAX_COLORS];
    int picture_altered = FALSE;
    
    unsigned char *Expand_Image_Data();

    if (read_dialog != 0) {
	if (XtIsManaged (read_dialog)) {
	    Set_Cursor_Watch (XtWindow (read_dialog));
	}
    }
    Set_Cursor_Watch (pwindow);
    status = K_SUCCESS; /* assume success */

    fp = fopen (temp_file, "r");
    if (fp == NULL) {
	status = K_CANNOT_OPEN;
    }
    else {
	fclose (fp);
	file_type = Find_Format();
	if (file_type == DDIF_FORMAT) {
	    Exit_Select_Portion ();
	    if (exiting_paint)
		return;
	    temp_image = Read_DDIF (&status, &picture_altered, 0);

/*
 * If the size of the picture has changed, resize the picture widget,
 * and create new pixmaps.
 */
	    if( status == K_SUCCESS ) {
		if ((temp_image->width < MIN_PICTURE_WD) ||
		    (temp_image->height < MIN_PICTURE_HT)) {
		    status = K_IMAGE_TOO_SMALL;
		}
		else {
		    picture_x = 0;
		    picture_y = 0;
		    pic_xorigin = 0;
		    pic_yorigin = 0;
		    if( (temp_image->width > XDisplayWidth(disp, screen)) ||
			(temp_image->height > XDisplayHeight(disp,screen))  ) {
			Display_Message ("T_PICTURE_NO_FIT");
		    }
		    if (picture_image != NULL)
		    {
#if 0
			XtFree (picture_image->data);
#endif
			XDestroyImage (picture_image);
		    }
		    picture_image = temp_image;
	 
		    new_file = TRUE;
		    Change_Picture_Size (temp_image->width, temp_image->height);
		    if (exiting_paint)
			return;
/*
		MY_XPutImage (disp, picture, Get_GC (GC_PD_COPY), picture_image,
			      0, 0, 0, 0, picture_wd, picture_ht);
		Refresh_Picture (0, 0, picture_wd, picture_ht);
*/
		    Set_File_Format (DDIF_FORMAT, FALSE);
		    Set_File_Color (new_file_color, FALSE);
		}
	    }
	}
	else {
	  if (file_type == XBITMAP_FORMAT) {
	    Exit_Select_Portion ();
	    if (exiting_paint)
		return;
/* The bitmap was already read into ret_pixmap, so just copy it */
	    if ((ret_wd < MIN_PICTURE_WD) || (ret_ht < MIN_PICTURE_HT)) {
	      status = K_IMAGE_TOO_SMALL;
	    }
            else {
	      if ((ret_wd > XDisplayWidth (disp, screen)) ||
		  (ret_ht > XDisplayHeight (disp, screen))) {
		status = K_BITMAP_TOO_LARGE;
              }
	      else {
		temp_image = XGetImage (disp, ret_pixmap, 0, 0, ret_wd, ret_ht,
                                        1, XYPixmap);
                if (temp_image->bitmap_bit_order != NATIVE_BIT_ORDER)
		    ConvertImageNative (temp_image);

		if (pdepth == 1) {
		  if (picture_image)
		  {
#if 0
		    XtFree (picture_image->data);
#endif
		    XDestroyImage (picture_image);
		  }
		  picture_image = temp_image;
		}
		else {
		  Save_Colormap (tmp_colormap, &tmp_num_colors, &tmp_private);
		  Get_Default_Colormap ();

		  image_data = Expand_Image_Data (temp_image->data,
						  ret_wd, ret_ht,
						  temp_image->bytes_per_line,
						  &bytes, TRUE);
		  XDestroyImage (temp_image);
		  if (image_data == 0) {
		    status = K_NO_CLIENT_MEMORY;
		  }
		  else {
		    temp_image = XCreateImage (disp, visual_info->visual,
					       pdepth, img_format, 0,
					       (char*)image_data, ret_wd, ret_ht, 32,
					       bytes);
		    if (temp_image != 0) {
		      temp_image->bitmap_bit_order = NATIVE_BIT_ORDER;
		      temp_image->byte_order = NATIVE_BYTE_ORDER;
		      if (picture_image != NULL)
		      {
#if 0
			XtFree (picture_image->data);
#endif
			XDestroyImage (picture_image);
		      }
		      picture_image = temp_image;
		    }
		    else {
		      status = K_FAILURE;
		    }
		  }
		  if (status != K_SUCCESS) {
		    Restore_Colormap (tmp_colormap, tmp_num_colors,
				      tmp_private);
		  }
		}
		
		if (status == K_SUCCESS) {
		  new_file = TRUE;
		  Change_Picture_Size (ret_wd, ret_ht);
		  if (exiting_paint)
		    return;
		  Set_File_Format (XBITMAP_FORMAT, FALSE);
	        }
	      }
	    }
            if (ret_pixmap) {
              XFreePixmap( disp, ret_pixmap );
	    }
	  }
	  else {
	    status = K_UNKNOWN_FILE_FORMAT;
	  }
	}
    }

    if (color_dialog)
    {
	if (XtIsManaged (color_dialog))
	{
	    Redraw_Color_Dialog_Window (0, 0, COLOR_WINDOW_WD, COLOR_WINDOW_HT);
	}
    }

/*
 * Do error checking
 */
    switch (status) {
	case K_SUCCESS :
	    if (picture_altered) {
		Display_Message ("T_IMAGE_ALTERED");
	    }
	    strcpy (cur_file, temp_file);
	    Extract_Filename();
	    Set_Titlebar();
	    Set_Undo_Button (NO_ACTION);
	    pixmap_changed = FALSE;
	    picture_changed = FALSE;
	    if (read_dialog) {
		if (XtIsManaged (read_dialog)) {
		    XtUnmanageChild (read_dialog);
		}
	    }
	    break;
	default :
	    Display_Read_File_Message (status);
	    break;
    }
    if (read_dialog != 0)
	XUndefineCursor (disp, XtWindow (read_dialog));
    Set_Cursor (pwindow, current_action);
}

/*
 *
 * ROUTINE:  Include_File
 *
 * ABSTRACT: 
 *
 *  Reads a file for inclusion.  Inputs are: cur_file and file_format
 *
 */
void Include_File()
{
    int file_type;
    int status, st;
    XImage *temp_image;
    FILE *fp;
    unsigned char *image_data;
    int bytes;
    int picture_altered = FALSE;
    int prv_num_colors = num_colors;
    unsigned long context;
    int tmp_undo_action;
    
    unsigned char *Expand_Image_Data();

    if (include_dialog != 0) {
	if (XtIsManaged (include_dialog)) {
	    Set_Cursor_Watch (XtWindow (include_dialog));
	}
    }
    Set_Cursor_Watch (pwindow);
    status = K_SUCCESS; /* assume success */

    fp = fopen (temp_file, "r");
    if (fp == NULL) {
	status = K_CANNOT_OPEN;
    }
    else {
	if (entering_text)
	    End_Text ();
	fclose (fp);
	file_type = Find_Format();
	if (file_type == DDIF_FORMAT) {

	    ISL_ERROR_HANDLER_SETUP
	    context = ImgOpenDDIFFile(ImgK_ModeImport, strlen(temp_file), 
				      temp_file, strlen(temp_file), temp_file,
				      0);
	    if (failure_occurred) {
		ISL_RECOVER_FROM_ERROR
		status = K_ISL_FAILURE;
	    }
	    else {
		Exit_Select_Portion ();
		if (exiting_paint)
		    return;
		temp_image = Read_DDIF (&status, &picture_altered, context);
	    }
	}
	else {
	    if (file_type == XBITMAP_FORMAT) {
		Exit_Select_Portion ();
		if (exiting_paint)
		    return;
		if ((ret_wd > XDisplayWidth (disp, screen)) ||
		    (ret_ht > XDisplayHeight (disp, screen))) {
		    status = K_IMAGE_TOO_BIG;
		}
		else {
		    temp_image = XGetImage (disp, ret_pixmap, 0, 0, ret_wd,
					    ret_ht, 1, XYPixmap);
		    if (temp_image->bitmap_bit_order != NATIVE_BIT_ORDER)
			ConvertImageNative (temp_image);

		    if (pdepth > 1) {
			image_data = 
			    Expand_Image_Data (temp_image->data,
					       ret_wd, ret_ht,
					       temp_image->bytes_per_line,
					       &bytes, TRUE);
			XDestroyImage (temp_image);
			if (image_data == 0) {
			    status = K_NO_CLIENT_MEMORY;
			}
			else {
			    temp_image =
				XCreateImage (disp, visual_info->visual,
					      pdepth, img_format, 0,
					      (char*)image_data, ret_wd, ret_ht,
					      32, bytes);
			    if (temp_image != 0) {
				temp_image->bitmap_bit_order = NATIVE_BIT_ORDER;
				temp_image->byte_order = NATIVE_BYTE_ORDER;
			    }
			    else {
				status = K_FAILURE;
			    }
			}
		    }
		}
		if (ret_pixmap) {
		    XFreePixmap (disp, ret_pixmap);
		}
	    }
	    else {
		status = K_UNKNOWN_FILE_FORMAT;
	    }
	}
    }
    
/*
 * Do error checking
 */
    switch (status) {
	case K_SUCCESS :
/* Do same as paste does */
	    tmp_undo_action = undo_action;
	    undo_action = INCLUDE;
	    st = Paste_Into_Select (temp_image);
	    if (st == K_SUCCESS) {
		if (picture_altered) {
		    Display_Message ("T_PASTE_IMAGE_ALTERED");
		}
		if (include_dialog) {
		    if (XtIsManaged (include_dialog)) {
			XtUnmanageChild (include_dialog);
		    }
		}
	    }
	    else {
		undo_action = tmp_undo_action;
		XDestroyImage (temp_image);
		Display_Message ("T_NO_MEM_FOR_INCLUDE");
	    }
	    break;
	case K_IMAGE_TOO_BIG :
	    Display_Message ("T_IMG_TOO_BIG_FOR_INCLUDE");
	    break;
	default :
	    Display_Read_File_Message (status);
	    break;
    }

    if ((status != K_SUCCESS) || (st != K_SUCCESS)) {
	Clear_Color_Table (prv_num_colors);
    }
    else {
	strcpy (include_file_name, temp_file);
	Set_Undo_Button (INCLUDE);
	Set_Edit_Buttons (SENSITIVE);
	if (color_dialog)
	{
	    if (XtIsManaged (color_dialog))
	    {
		Redraw_Color_Dialog_Window (0, 0, COLOR_WINDOW_WD,
					    COLOR_WINDOW_HT);
	    }
	}
    }

    if (include_dialog != 0)
	XUndefineCursor (disp, XtWindow (include_dialog));
    Set_Cursor (pwindow, current_action);
}

