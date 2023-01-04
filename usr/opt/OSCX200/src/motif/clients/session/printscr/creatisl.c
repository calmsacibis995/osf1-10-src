/*
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 * HISTORY
 */
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader = "$Header: /usr/sde/x11/rcs/x11/src/./motif/clients/session/printscr/creatisl.c,v 1.1 90/01/01 00:00:00 devrcs Exp $";
#endif		/* BuildSystemHeader */
/*
**++
**  COPYRIGHT (c) 1988 BY
**  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.
**  ALL RIGHTS RESERVED.
** 
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED
**  ONLY  IN  ACCORDANCE  OF  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER
**  COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
**  OTHER PERSON.  NO TITLE TO AND  OWNERSHIP OF THE  SOFTWARE IS  HEREBY
**  TRANSFERRED.
** 
**  THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE
**  AND  SHOULD  NOT  BE  CONSTRUED  AS A COMMITMENT BY DIGITAL EQUIPMENT
**  CORPORATION.
** 
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.
**--
**/


/*
**++
**  MODULE NAME:
**	creatisl.c
**
**  FACILITY:
**      PrintScreen
**
**  ABSTRACT:
**	Subroutines which converts an "X11" image into an ISL frame 
**      
**
**  ENVIRONMENT:
**
**	VMS V5, Ultrix V3  DW FT2, ISL V1
**
**  AUTHORS:
**      Kathy Robinson
**
**  RELEASER:
**	Kathy Robinson
**
**  CREATION DATE:     
**	April 11, 1988
**
**  MODIFICATION HISTORY:
**
**	18-APR-1988
**		Moved #include iprdw.h to top so that #define ISL could be put in iprdw.h
**
**	9-MAY-1988
**		Remove reference to SPECT_TYPE
**
**	1-JUL-1988
**		Fixed references to ISL constants which ISL renamed
**
**	14-Jun-1989	Barbara Bazemore
**		Add Brightness_Polarity attribute to Create_Frame
**		for reverse_image/toner_save support.
**
**	29-Jun-1989	B. Bazemore
**		Use DPI in ISL bounding box calculations for correct
**		output size.
**--
**/

#include <iprdw.h>

#ifdef ISL


#ifdef VMS
#include <img$def>
#else
#include <img_def.h>
#endif

#define BMUS_PER_INCH 1200

/* CONVERT_PIXEL_TO_BMU: Given a pixel value, returns the equivalent
   basic measuring unit value using the device pixel DPI (dots per inch) 
   value, treated as a linear measurement; parameter is an integer value */
/* [Assumption: options->dpy has been defined ] */

#define CONVERT_PIXEL_TO_BMU( pixels ) \
    ((int) (BMUS_PER_INCH * ((float)(pixels) / DPIof(options->dpy)) ))


typedef struct v_d_t
   {
    short string_length;
    unsigned char string_type;
    unsigned char string_class;
    unsigned char *string_address;           
   } vms_descriptor_type;

/*
  Itemlist handling for setting ISL attributes.
 */

typedef struct sie
   {
    int item_code;
    int component_length;
    int *component_address;
    int index;
   } set_itemlist_entry;

#define put_set_item(item_list, item_index, item_code_value, component)        \
   {                                                                           \
    item_index++;                                                              \
    item_list[item_index].item_code = item_code_value;                         \
    item_list[item_index].component_length = sizeof(component);                \
    item_list[item_index].component_address = &component;                      \
    item_list[item_index].index = 0;                                           \
   }

#define end_set_itemlist(item_list, item_index)                                \
   {                                                                           \
    item_index++;                                                              \
    item_list[item_index].item_code = 0;                                       \
    item_list[item_index].component_length = 0;                                \
    item_list[item_index].component_address = 0;                               \
    item_list[item_index].index = 0;                                           \
   }

#define start_set_itemlist(item_list, item_index)                              \
   {                                                                           \
    item_index = -1;                                                           \
    item_list[0].item_code = 0;                                                \
    item_list[0].component_length = 0;                                         \
    item_list[0].component_address = 0;                                        \
    item_list[0].index = 0;                                                    \
   }

#endif


/*
 *	creat_color_islframe( options, ximage, file )
 *		options - (RO) (*dxPrscOptions) options file, 
 *			reverse_image and aspect are referenced
 *		ximage 	- (RO) (*XImage) input image from fiximage
 *		isl - (WO) isl frame
 *
 *	returns:
 *		indications of success, see iprdw.h
 *	requires:
 * 		swap routines - negbits, invertbits
 *		Image Services Library routines
 * 	restrictions:
 *		image is a "corrected" not a real X11 image
 *	other:
 *		no  implicit parameters, etc.
 *
 */	
creat_color_islframe(options, ximage, vis, isl)
dxPrscOptions *options;
XImage	*ximage;                          
Visual		*vis;
int *isl;
{
#ifdef ISL
    char *image_data;
    int image_id;
    int set_index;
    set_itemlist_entry set_attributes[32];


    int pixels_per_scanline;
    int scanline_count;
    int bits_per_pixel;
    int scanline_stride;
    int spectral_type = IMG$K_RGB_MAP;
    int pixel_order = IMG$K_STANDARD_PIXEL_ORDER;
    int bmu_wd, bmu_ht, zero;
    int context;
    int bytes;
    int image_size;
    int num_comp = 3;
    int pix_stride;
    int comp_array[3];
    int toner_save;			    /* reverse image option */




/* store image metrics						
 * some of these variables are redunant.  		
 */

    bits_per_pixel = ximage->depth;
    pix_stride = ximage->bits_per_pixel;
    bytes = ximage->bytes_per_line;
    pixels_per_scanline = ximage-> width;
    scanline_count = ximage -> height;
    scanline_stride = BITSINBYTE * bytes;	/*  BITSINBYTE = 8 */
    image_data = ximage -> data;
    image_size = bytes * scanline_count;

    /* Calculate the bounding box information.  Determine actual
     * physical width and height.  This will vary with different 
     * DPI displays.
     */
    bmu_wd =  CONVERT_PIXEL_TO_BMU( ximage->width );
    bmu_ht =  CONVERT_PIXEL_TO_BMU( ximage->height );
    zero = 0;

    if (bits_per_pixel == 24)
 	{
    	comp_array[0] = 8;      /* Really should look in visual for these */
    	comp_array[1] = 8;    
    	comp_array[2] = 8;    
        }
    if (bits_per_pixel == 8)
	{
    	comp_array[0] = 3;      
    	comp_array[1] = 3;
    	comp_array[2] = 2;
	}



    /* Set postive or reverse image based on user option    */
    if (options->reverse_image == dxPrscNegative)
	toner_save = IMG$K_ZERO_MAX_INTENSITY;
    else
	toner_save = IMG$K_ZERO_MIN_INTENSITY;


/*  Massage image data into an LSB-byte ordered, LSB bit-ordered scheme 
 */
 
    if( ximage->byte_order != LSBFirst )
    {
	switch( ximage->bitmap_unit )
 	{
		case 8: 
			break;
		case 16: 
			swapshort( image_data, image_size);
		    	break;
		default:
			swaplong( image_data, image_size);
	}
    }
    if( ximage->bitmap_bit_order != LSBFirst ) /* ISL wants it backwards (LSB)*/
	{  
	swapbits( image_data, image_size);
	}
		    


/* 
 * set attributes so ISL can parse image data			
 * Also set DDIF bounding box info so real world size of image      	
 * can be detremined by readers of DDIF					
 */



    start_set_itemlist(set_attributes, set_index);
    put_set_item(set_attributes,set_index,IMG$_PIXELS_PER_LINE, pixels_per_scanline);
    put_set_item(set_attributes,set_index,IMG$_NUMBER_OF_LINES, scanline_count);
    put_set_item(set_attributes,set_index,IMG$_PIXEL_STRIDE, pix_stride);
    put_set_item(set_attributes,set_index,IMG$_BITS_PER_PIXEL, bits_per_pixel);
    put_set_item(set_attributes,set_index,IMG$_SCANLINE_STRIDE, scanline_stride);
    put_set_item(set_attributes,set_index,IMG$_SPECTRAL_MAPPING, spectral_type);
    put_set_item(set_attributes,set_index,IMG$_NUMBER_OF_COMP, num_comp);
    put_set_item(set_attributes,set_index,IMG$_PIXEL_ORDER, pixel_order);
    put_set_item(set_attributes,set_index,IMG$_BRT_POLARITY, toner_save);

    /* Let them know which bits are r, which g, which b */
    put_set_item(set_attributes,set_index,IMG$_IMG_BITS_PER_COMP, comp_array[0]);
    put_set_item(set_attributes,set_index,IMG$_IMG_BITS_PER_COMP, comp_array[1]);
    set_attributes[set_index].index = 1;
    put_set_item(set_attributes,set_index,IMG$_IMG_BITS_PER_COMP, comp_array[2]);
    set_attributes[set_index].index = 2;
  
    /* It's our responsibility, not ISL's to put a bounding box in the DDIF */
    put_set_item(set_attributes,set_index,IMG$_FRM_BOX_LL_X, zero);
    put_set_item(set_attributes,set_index,IMG$_FRM_BOX_LL_Y, zero);
    put_set_item(set_attributes,set_index,IMG$_FRM_BOX_UR_X, bmu_wd);
    put_set_item(set_attributes,set_index,IMG$_FRM_BOX_UR_Y, bmu_ht);
    end_set_itemlist(set_attributes, set_index);


/* 
 * create ISL frame from color "X image" data, which we ourselves created in
 * fiximage, and isn't 100% like real Ximage data.
 */
    image_id = IMG$CREATE_FRAME(set_attributes, IMG$K_STYPE_MULTISPECT);

    IMG$IMPORT_BITMAP(image_id, image_data, image_size, NULL, NULL, NULL, NULL);

    /* Return isl frame id */
    *isl = image_id;
#endif

    }
                                
/*
 *	creat_mono_islframe( options, ximage, file )
 *		options - (RO) (*dxPrscOptions) options file, 
 *			reverse_image and aspect are referenced
 *		ximage 	- (RO) (*XImage) input image
 *		isl - (WO) isl frame
 *
 *	returns:
 *		indications of success, see iprdw.h
 *	requires:
 * 		swap routines - negbits, invertbits
 *		Image Services Library routines
 * 	restrictions:
 *		image MUST be black and white (that is, have a depth of 1)
 *	other:
 *		no  implicit parameters, etc.
 *
 */	
creat_mono_islframe(options, ximage, isl)
dxPrscOptions *options;
XImage	*ximage;
int * isl;
{
#ifdef ISL
    char *image_data;
    int image_id;
    int set_index;
    set_itemlist_entry set_attributes[32];


    int pixels_per_scanline;
    int scanline_count;
    int bits_per_pixel = 1;
    int scanline_stride;
    int spectral_type = IMG$K_STYPE_BITONAL;
    int pixel_order = IMG$K_STANDARD_PIXEL_ORDER;
    int bmu_wd, bmu_ht;
    int zero, one;
    int context;
    int bytes;
    int image_size;
    int toner_save;			    /* Reverse image	*/



/* store image metrics						
 * some of these variables are redunant.  		
 */

    bytes = ximage->bytes_per_line;
    pixels_per_scanline = ximage-> width;
    scanline_count = ximage -> height;
    scanline_stride = bytes * BITSINBYTE;	/*  BITSINBYTE = 8 */
    image_data = ximage -> data;
    image_size = bytes * scanline_count;
    bmu_wd =  CONVERT_PIXEL_TO_BMU( ximage->width );
    bmu_ht =  CONVERT_PIXEL_TO_BMU( ximage->height );
    zero = 0;
    one = 1;

    /* Set postive or reverse image based on user option    */
    if (options->reverse_image == dxPrscNegative)
	toner_save = IMG$K_ZERO_MAX_INTENSITY;
    else
	toner_save = IMG$K_ZERO_MIN_INTENSITY;

/*  Massage image data into an LSB-byte ordered, LSB bit-ordered scheme 
 *  Complement all bits if reverse option is specified		    
 */
 
    if( ximage->byte_order != LSBFirst )
    {
	switch( ximage->bitmap_unit )
 	{
		case 8: 
			break;
		case 16: 
			swapshort( image_data, image_size);
		    	break;
		default:
			swaplong( image_data, image_size);
	}
    }
    if( ximage->bitmap_bit_order != LSBFirst ) /* ISL wants it backwards (LSB)*/
	{  
	swapbits( image_data, image_size);
	}
		    


/* 
 * set attributes so ISL can parse image data			
 * Also set DDIF bounding box info so real world size of image      	
 * can be detremined by readers of DDIF					
 */
    start_set_itemlist(set_attributes, set_index);
    put_set_item(set_attributes,set_index,IMG$_PIXEL_ORDER, pixel_order);
    put_set_item(set_attributes,set_index,IMG$_BITS_PER_PIXEL, bits_per_pixel);
    put_set_item(set_attributes,set_index,IMG$_NUMBER_OF_LINES, scanline_count);
    put_set_item(set_attributes,set_index,IMG$_SCANLINE_STRIDE, scanline_stride);
    put_set_item(set_attributes,set_index,IMG$_PIXELS_PER_LINE, pixels_per_scanline);
    put_set_item(set_attributes,set_index,IMG$_BRT_POLARITY, toner_save);
    put_set_item(set_attributes,set_index,IMG$_NUMBER_OF_COMP, one);
    put_set_item(set_attributes,set_index,IMG$_IMG_BITS_PER_COMP, one);
  
    /* It's our responsibility, not ISL's to put a bounding box in the DDIF */
    put_set_item(set_attributes,set_index,IMG$_FRM_BOX_LL_X, zero);
    put_set_item(set_attributes,set_index,IMG$_FRM_BOX_LL_Y, zero);
    put_set_item(set_attributes,set_index,IMG$_FRM_BOX_UR_X, bmu_wd);
    put_set_item(set_attributes,set_index,IMG$_FRM_BOX_UR_Y, bmu_ht);
    end_set_itemlist(set_attributes, set_index);


/* 
 * create ISL frame from 1 plane (mono) X image data returned by server, then 
 * write ISL frame as a sixel file.  If necessary, rotate the frame first. If 
 * the option is set, scale the frame in the x direction by 2 - so as to 
 * change the aspect ratio.  (Scaling up by integers works OK.  It's the 
 * scaling down, or up by a fraction, that is ugly.)
 */


    image_id = IMG$CREATE_FRAME(set_attributes, spectral_type);

    IMG$IMPORT_BITMAP(image_id, image_data, image_size, NULL, NULL, NULL, NULL);

    
    /* Return isl frame id */
    *isl = image_id;

#endif
    }
