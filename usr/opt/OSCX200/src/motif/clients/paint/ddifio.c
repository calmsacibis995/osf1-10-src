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
/*
**************************************************************************
**                   DIGITAL EQUIPMENT CORPORATION                      **
**                         CONFIDENTIAL                                 **
**    NOT FOR MODIFICATION OR REDISTRIBUTION IN ANY MANNER WHATSOEVER   **
**************************************************************************
*/
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /usr/sde/x11/rcs/x11/src/./motif/clients/paint/ddifio.c,v 1.1.2.4 92/12/11 08:34:06 devrcs Exp $";	/* BuildSystemHeader */
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
**   Jonathan Joseph, Dec 1989
**
**  ABSTRACT:
**
**   Handles reading / writing of DDIF files
**
**  ENVIRONMENT:
**
**   User mode, executable image.
**
**  MODIFICATION HISTORY:
**
**--
**/           
#include <time.h>

#include "paintrefs.h"

#include <cdaptp.h>
#include <cdadef.h>
#include <cdamsg.h>
#include <ddifdef.h>

#define MAKE_ITEM(item, i, code, length, address)                              \
{item[i].item_code    = (unsigned short int) code;                             \
 item[i].item_length  = (unsigned short int) length;                           \
 item[i].CDAitemparam.item_address = (char *) address;}


int Convert_Color_F_To_I (f_color)
    float f_color;
{
    int i_color;

    i_color = (f_color * (float) MAX_COLOR_VALUE + 0.5);
    return (i_color);
}

float Convert_Color_I_To_F (i_color)
    int i_color;
{
    float f_color;

    f_color = (float) i_color / (float) MAX_COLOR_VALUE;
    return (f_color);
}

/* Color -> Mono intensity  Y = 0.587*G + 0.299*R + 0.114*B */
/* given a color in MyColor format, spit out a grey intensity between 0 and */
/* 255 */
int Convert_Color_To_Grey (color)
    MyColor *color;
{
    return ((int) (0.299 * (float) (color->i_red >> 8) +
		   0.587 * (float) (color->i_green >> 8) +
		   0.114 * (float) (color->i_blue >> 8) + 0.5));
}

#if 0
void Convert_Color_RGB_To_MyColor (rgb, rgb_info, color)
    unsigned char rgb;
    unsigned long *rgb_info;
    MyColor *color;
{
    int i, j, k, z;
    int i_color;
    unsigned char mask;
    unsigned short m2;

    i = 0;
    for (z = 0; z < 3; z ++) {
	mask = 0;
	for (j = 0; j < rgb_info[z]; j++) {
	    mask <<= 1;
	    mask += 1;
	}
	i += j;
	mask <<= (8 - i);
	m2 = rgb & mask;
	m2 <<= (8 + i - j);
	i_color = 0;
	for (k = 0; k < 16; k += j) {
	    i_color >>= j;
	    i_color += m2;
	}
	switch (z) {
	    case 0:
		color->f_red = Convert_Color_I_To_F (i_color);
		break;
	    case 1:
		color->f_green = Convert_Color_I_To_F (i_color);
		break;
	    case 2:
		color->f_blue = Convert_Color_I_To_F (i_color);
		break;
	}
    }
}
#endif

int Write_Color_DDIF (ximage)
    XImage *ximage;
{

    time_t date;
    struct tm *local_date;

    CDAsize filename_length;
    CDAagghandle aggregate_handle_stack [5];

    unsigned long value_constant = DDIF_K_VALUE_CONSTANT;
    CDAsize  value_constant_length = sizeof (value_constant);

    unsigned long integer_value;
    CDAsize integer_value_length = sizeof (integer_value);

    unsigned long dsc_major_version = 1;
    unsigned long dsc_minor_version = 0;

    static unsigned char dsc_product_identifier[] = "Img_";
    CDAsize dsc_product_identifier_length =
			sizeof (dsc_product_identifier) - 1;

    static unsigned char dsc_product_name[] = "DECpaint V3.0";
    CDAsize dsc_product_name_length = sizeof (dsc_product_name) - 1;

    static unsigned char dhd_title[] = "DECpaint File";
    CDAsize dhd_title_length = sizeof (dhd_title) - 1;

    static unsigned char dhd_author[] = "DECpaint";
    CDAsize dhd_author_length = sizeof (dhd_author) - 1;

    static unsigned char dhd_version[] = "3.0";
    CDAsize dhd_version_length = sizeof (dhd_version) - 1;

    char *dhd_date;
    CDAsize dhd_date_length;

    unsigned long img_lut_c = DDIF_K_RGB_LUT;
    CDAsize img_lut_c_length = sizeof (img_lut_c);
    
    static unsigned char content_category[] = "$I";
    CDAsize content_category_length = sizeof (content_category) - 1;

    unsigned long img_pixel_path = 0;
    unsigned long img_line_progression = 270;
    unsigned long img_pp_pixel_dist = 1;
    unsigned long img_lp_pixel_dist = 1;

    unsigned long img_brt_polarity = DDIF_K_ZERO_MIN_INTENSITY;
    CDAsize img_brt_polarity_length = sizeof (img_brt_polarity);

    unsigned long img_grid_type = DDIF_K_RECTANGULAR_GRID;
    CDAsize img_grid_type_length = sizeof (img_grid_type);

    unsigned long img_spectral_mapping = DDIF_K_LUT_MAP;
    CDAsize img_spectral_mapping_length = sizeof (img_spectral_mapping);

    unsigned long img_comp_space_org = DDIF_K_FULL_COMPACTION;
    CDAsize  img_comp_space_org_length = sizeof (img_comp_space_org);

    unsigned long img_planes_per_pixel = 1;

    unsigned long img_plane_signif = DDIF_K_LSB_MSB;
    CDAsize  img_plane_signif_length = sizeof (img_plane_signif);

    unsigned long img_number_of_comp = 1;
    unsigned long img_bits_per_comp = (pdepth == 1) ? 1 : 8;
    unsigned long frm_box_ll_x = 0;
    unsigned long frm_box_ll_y = 0;

    unsigned long frm_position_c = DDIF_K_FRAME_FIXED;
    CDAsize  frm_position_c_length = sizeof (frm_position_c);

    unsigned long frmfxd_position_x = 0;
    unsigned long frmfxd_position_y = 0;

    unsigned long idu_compression_type = DDIF_K_PCM_COMPRESSION;
    CDAsize  idu_compression_type_length= sizeof (idu_compression_type);

    unsigned long idu_pixel_order = DDIF_K_STANDARD_PIXEL_ORDER;
    CDAsize  idu_pixel_order_length = sizeof (idu_pixel_order);

    unsigned char *idu_plane_data;
    CDAsize  idu_plane_data_length;

    CDAaggtype aggregate_type;
    CDAconstant aggregate_item;
    CDAindex aggregate_index;
    CDAconstant add_info;
    unsigned long status;
    DDISstreamhandle stream_handle;
    CDAfilehandle file_handle;
    CDArootagghandle root_aggregate_handle;
    unsigned long aggregate_handle;
    CDAagghandle prv_aggregate_handle;
    CDAsize aggregate_handle_length = sizeof (aggregate_handle);

    unsigned long ahs_index = 0;

    float color_value;
    CDAsize color_value_length = sizeof (color_value);
    int i;
    unsigned long lut_index;
    CDAsize lut_index_length = sizeof (lut_index);	 


/* Create the root aggregate */
    aggregate_type =DDIF_DDF;
    status = cda_create_root_aggregate (NULL, NULL, NULL, NULL,
					&aggregate_type,
					&root_aggregate_handle);
    
    if (status != CDA_NORMAL) {
	return (K_FAILURE);
    }
    
/* Create the DDF Descriptor agrregate */
    aggregate_type = DDIF_DSC;
    status = cda_create_aggregate (&root_aggregate_handle,
                                   &aggregate_type,
                                   &aggregate_handle_stack[ahs_index]);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

/* Attach it to the root aggregate */
    aggregate_item = DDIF_DDF_DESCRIPTOR;
    status = cda_store_item (&root_aggregate_handle,
                             (CDAagghandle*)&root_aggregate_handle,
                             &aggregate_item,
                             &aggregate_handle_length,
                             &aggregate_handle_stack[ahs_index], NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

/* Fill in the Major Version item of the Descriptor aggregate. */
    aggregate_item = DDIF_DSC_MAJOR_VERSION;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &integer_value_length,
                             &dsc_major_version, NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

/* Fill in the Minor Version item of the Descriptor aggregate. */
    aggregate_item = DDIF_DSC_MINOR_VERSION;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &integer_value_length,
                             &dsc_minor_version, NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

/* Fill in the Product Identifier item of the Descriptor aggregate */
    aggregate_item = DDIF_DSC_PRODUCT_IDENTIFIER;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &dsc_product_identifier_length,
                              dsc_product_identifier, NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

/* Fill in the Product Name item of the Descriptor aggregate. */
    aggregate_index = 0;
    aggregate_item = DDIF_DSC_PRODUCT_NAME;
    add_info = CDA_K_ISO_LATIN1;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &dsc_product_name_length,
                              dsc_product_name,
                             &aggregate_index,
                             &add_info);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

/* Create the Header aggregate */
    aggregate_type = DDIF_DHD;
    status = cda_create_aggregate (&root_aggregate_handle,
                                   &aggregate_type,
                                   &aggregate_handle_stack[ahs_index]);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

/* Attach it to the Root aggregate */
    aggregate_item = DDIF_DDF_HEADER;
    status = cda_store_item (&root_aggregate_handle,
                             (CDAagghandle*)&root_aggregate_handle,
                             &aggregate_item,
                             &aggregate_handle_length,
                             &aggregate_handle_stack[ahs_index], NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

/* Fill in the Title item of the Descriptor aggregate. */
    aggregate_index = 0;
    aggregate_item = DDIF_DHD_TITLE;
    add_info = CDA_K_ISO_LATIN1;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &dhd_title_length,
                              dhd_title,
                             &aggregate_index,
                             &add_info);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    aggregate_index = 0;
    aggregate_item = DDIF_DHD_AUTHOR;
    add_info = CDA_K_ISO_LATIN1;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &dhd_author_length,
                              dhd_author,
                             &aggregate_index,
                             &add_info);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    aggregate_index = 0;
    aggregate_item = DDIF_DHD_VERSION;
    add_info = CDA_K_ISO_LATIN1;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &dhd_version_length,
                              dhd_version,
                             &aggregate_index,
                             &add_info);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    aggregate_item = DDIF_DHD_DATE;
    time (&date);
    local_date = localtime (&date);
    dhd_date = asctime (local_date);
    dhd_date[24] = '.';
    dhd_date_length = 25;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &dhd_date_length,
                             dhd_date,
                             &aggregate_index, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

/* Create the Segment aggregate */
    aggregate_type = DDIF_SEG;
    status = cda_create_aggregate (&root_aggregate_handle,
                                   &aggregate_type,
                                   &aggregate_handle_stack[ahs_index]);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

/* Attach it to the Root aggregate */
    aggregate_item = DDIF_DDF_CONTENT;
    status = cda_store_item (&root_aggregate_handle,
                             (CDAagghandle*)&root_aggregate_handle,
                             &aggregate_item,
                             &aggregate_handle_length,
                             &aggregate_handle_stack[ahs_index], NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }
    
/* Create a Segment aggregate */
    ahs_index++;
    aggregate_type = DDIF_SEG;
    status = cda_create_aggregate (&root_aggregate_handle,
                                   &aggregate_type,
                                   &aggregate_handle_stack[ahs_index]);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

/* Attach it to the DDF_CONTENT Segment aggregate */
    aggregate_item = DDIF_SEG_CONTENT;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index - 1],
                             &aggregate_item,
                             &aggregate_handle_length,
                             &aggregate_handle_stack[ahs_index], NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    ahs_index++;
    aggregate_type = DDIF_SGA;
    status = cda_create_aggregate (&root_aggregate_handle,
                                   &aggregate_type,
                                   &aggregate_handle_stack[ahs_index]);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    aggregate_item = DDIF_SEG_SPECIFIC_ATTRIBUTES;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index - 1],
                             &aggregate_item,
                             &aggregate_handle_length,
                             &aggregate_handle_stack[ahs_index], NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }


    aggregate_item = DDIF_SGA_CONTENT_CATEGORY;
    aggregate_index = 0;
    add_info = DDIF_K_I_CATEGORY;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &content_category_length,
                              content_category,
			     &aggregate_index,
			     &add_info);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    aggregate_item = DDIF_SGA_IMG_PIXEL_PATH;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &integer_value_length,
                             &img_pixel_path, NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    aggregate_item = DDIF_SGA_IMG_LINE_PROGRESSION;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &integer_value_length,
                             &img_line_progression, NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    aggregate_item = DDIF_SGA_IMG_PP_PIXEL_DIST;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &integer_value_length,
                             &img_pp_pixel_dist, NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    aggregate_item = DDIF_SGA_IMG_LP_PIXEL_DIST;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &integer_value_length,
                             &img_lp_pixel_dist, NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    aggregate_item = DDIF_SGA_IMG_BRT_POLARITY;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &img_brt_polarity_length,
                             &img_brt_polarity, NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    aggregate_item = DDIF_SGA_IMG_GRID_TYPE;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &img_grid_type_length,
                             &img_grid_type, NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    aggregate_item = DDIF_SGA_IMG_SPECTRAL_MAPPING;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &img_spectral_mapping_length,
                             &img_spectral_mapping, NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    aggregate_item = DDIF_SGA_IMG_LOOKUP_TABLES_C;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &img_lut_c_length,
                             &img_lut_c, NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    ahs_index++;

    for (i = 0; i < num_colors; i++) {
	aggregate_type = DDIF_RGB;
	status = cda_create_aggregate (&root_aggregate_handle,
				       &aggregate_type,
				       &aggregate_handle_stack[ahs_index]);
	if (status != CDA_NORMAL) {
	    cda_delete_root_aggregate (&root_aggregate_handle);
	    return (K_FAILURE);
	}

	if (i == 0) {
	    aggregate_index = i;
	    aggregate_item = DDIF_SGA_IMG_LOOKUP_TABLES;
	    status = cda_store_item (&root_aggregate_handle,
				     &aggregate_handle_stack[ahs_index - 1],
				     &aggregate_item,
				     &aggregate_handle_length,
				     &aggregate_handle_stack[ahs_index],
				     &aggregate_index, NULL);
	    if (status != CDA_NORMAL) {
		cda_delete_root_aggregate (&root_aggregate_handle);
		return (K_FAILURE);
	    }
	}
	else {
	    status = cda_insert_aggregate (&aggregate_handle_stack[ahs_index],
					   &prv_aggregate_handle);
	    if (status != CDA_NORMAL) {
		cda_delete_root_aggregate (&root_aggregate_handle);
		return (K_FAILURE);
	    }
	}
	prv_aggregate_handle = aggregate_handle_stack[ahs_index];

	lut_index = colormap[i].pixel;
	aggregate_item = DDIF_RGB_LUT_INDEX;
	status = cda_store_item (&root_aggregate_handle,
				 &aggregate_handle_stack[ahs_index],
				 &aggregate_item,
				 &lut_index_length,
				 &lut_index, NULL, NULL);
	if (status != CDA_NORMAL) {
	    cda_delete_root_aggregate (&root_aggregate_handle);
	    return (K_FAILURE);
	}

	color_value = colormap[i].f_red;
	aggregate_item = DDIF_RGB_RED_VALUE;
	status = cda_store_item (&root_aggregate_handle,
				 &aggregate_handle_stack[ahs_index],
				 &aggregate_item,
				 &color_value_length,
				 &color_value, NULL, NULL);
	if (status != CDA_NORMAL) {
	    cda_delete_root_aggregate (&root_aggregate_handle);
	    return (K_FAILURE);
	}

	color_value = colormap[i].f_green;
	aggregate_item = DDIF_RGB_GREEN_VALUE;
	status = cda_store_item (&root_aggregate_handle,
				 &aggregate_handle_stack[ahs_index],
				 &aggregate_item,
				 &color_value_length,
				 &color_value, NULL, NULL);
	if (status != CDA_NORMAL) {
	    cda_delete_root_aggregate (&root_aggregate_handle);
	    return (K_FAILURE);
	}

	color_value = colormap[i].f_blue;
	aggregate_item = DDIF_RGB_BLUE_VALUE;
	status = cda_store_item (&root_aggregate_handle,
				 &aggregate_handle_stack[ahs_index],
				 &aggregate_item,
				 &color_value_length,
				 &color_value, NULL, NULL);
	if (status != CDA_NORMAL) {
	    cda_delete_root_aggregate (&root_aggregate_handle);
	    return (K_FAILURE);
	}
    }

    ahs_index--;

    aggregate_item = DDIF_SGA_IMG_COMP_SPACE_ORG;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &img_comp_space_org_length,
                             &img_comp_space_org, NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    aggregate_item = DDIF_SGA_IMG_PLANES_PER_PIXEL;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &integer_value_length,
                             &img_planes_per_pixel, NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    aggregate_item = DDIF_SGA_IMG_PLANE_SIGNIF;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &img_plane_signif_length,
                             &img_plane_signif, NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    aggregate_item = DDIF_SGA_IMG_NUMBER_OF_COMP;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &integer_value_length,
                             &img_number_of_comp, NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    aggregate_index = 0;
    aggregate_item = DDIF_SGA_IMG_BITS_PER_COMP;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &integer_value_length,
                             &img_bits_per_comp,
			     &aggregate_index, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    aggregate_item = DDIF_SGA_FRM_BOX_LL_X_C;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &value_constant_length,
                             &value_constant, NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    aggregate_item = DDIF_SGA_FRM_BOX_LL_X;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &integer_value_length,
                             &frm_box_ll_x, NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    aggregate_item = DDIF_SGA_FRM_BOX_LL_Y_C;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &value_constant_length,
                             &value_constant, NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    aggregate_item = DDIF_SGA_FRM_BOX_LL_Y;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &integer_value_length,
                             &frm_box_ll_y, NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    aggregate_item = DDIF_SGA_FRM_BOX_UR_X_C;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &value_constant_length,
                             &value_constant, NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    aggregate_item = DDIF_SGA_FRM_BOX_UR_X;
    integer_value = (float)(ximage->width * BMU) / resolution;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &integer_value_length,
                             &integer_value, NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    aggregate_item = DDIF_SGA_FRM_BOX_UR_Y_C;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &value_constant_length,
                             &value_constant, NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    aggregate_item = DDIF_SGA_FRM_BOX_UR_Y;
    integer_value = (float)(ximage->height * BMU) / resolution;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &integer_value_length,
                             &integer_value, NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    aggregate_item = DDIF_SGA_FRM_POSITION_C;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &frm_position_c_length,
                             &frm_position_c, NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    aggregate_item = DDIF_SGA_FRMFXD_POSITION_X_C;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &value_constant_length,
                             &value_constant, NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    aggregate_item = DDIF_SGA_FRMFXD_POSITION_X;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &integer_value_length,
                             &frmfxd_position_x, NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    aggregate_item = DDIF_SGA_FRMFXD_POSITION_Y_C;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &value_constant_length,
                             &value_constant, NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    aggregate_item = DDIF_SGA_FRMFXD_POSITION_Y;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &integer_value_length,
                             &frmfxd_position_y, NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

/* Create the Image aggregate */
    aggregate_type = DDIF_IMG;
    status = cda_create_aggregate (&root_aggregate_handle,
                                   &aggregate_type,
                                   &aggregate_handle_stack[ahs_index]);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    aggregate_item = DDIF_SEG_CONTENT;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index - 1],
                             &aggregate_item,
                             &aggregate_handle_length,
                             &aggregate_handle_stack[ahs_index], NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    ahs_index++;

/* create the IDU aggregate */
    aggregate_type = DDIF_IDU;
    status = cda_create_aggregate (&root_aggregate_handle,
                                   &aggregate_type,
                                   &aggregate_handle_stack[ahs_index]);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    aggregate_item = DDIF_IMG_CONTENT;
    aggregate_index = 0;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index - 1],
                             &aggregate_item,
                             &aggregate_handle_length,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_index, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

/* populate the IDU aggregate */
    aggregate_item = DDIF_IDU_PIXELS_PER_LINE;
    integer_value = ximage->width;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &integer_value_length,
                             &integer_value, NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    aggregate_item = DDIF_IDU_NUMBER_OF_LINES;
    integer_value = ximage->height;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &integer_value_length,
                             &integer_value, NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    aggregate_item = DDIF_IDU_COMPRESSION_TYPE;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &idu_compression_type_length,
                             &idu_compression_type, NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    aggregate_item = DDIF_IDU_DATA_OFFSET;
    integer_value = 0;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &integer_value_length,
                             &integer_value, NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    aggregate_item = DDIF_IDU_PIXEL_STRIDE;
    integer_value = ximage->bits_per_pixel;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &integer_value_length,
                             &integer_value, NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    aggregate_item = DDIF_IDU_SCANLINE_STRIDE;
    integer_value = ximage->bytes_per_line * 8;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &integer_value_length,
                             &integer_value, NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    aggregate_item = DDIF_IDU_PIXEL_ORDER;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &idu_pixel_order_length,
                             &idu_pixel_order, NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    aggregate_item = DDIF_IDU_BITS_PER_PIXEL;
    integer_value = ximage->bits_per_pixel;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &integer_value_length,
                             &integer_value, NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

    aggregate_item = DDIF_IDU_PLANE_DATA;
    idu_plane_data = (unsigned char *)ximage->data;
    idu_plane_data_length = ximage->bytes_per_line * ximage->height;
    status = cda_store_item (&root_aggregate_handle,
                             &aggregate_handle_stack[ahs_index],
                             &aggregate_item,
                             &idu_plane_data_length,
                              idu_plane_data, NULL, NULL);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

/* Create an output file to receive the DDIF stream */
    filename_length = strlen (cur_file);
    status = cda_create_file (&filename_length, cur_file,
			      NULL, NULL, NULL, NULL, NULL,
			      &root_aggregate_handle,
			      NULL,  /* Default result_filename_len	*/
			      NULL,  /* Default result_filename		*/
			      NULL,  /* Default result_filename_ret_len	*/
			      &stream_handle,
			      &file_handle);
    if (status != CDA_NORMAL) {
	cda_delete_root_aggregate (&root_aggregate_handle);
	return (K_FAILURE);
    }

/* Write the DDIF stream to the output file */
    status = cda_put_document (&root_aggregate_handle, &stream_handle);

/* Close the output file */
    cda_close_file (&stream_handle, &file_handle);

/* Delete the Root aggregate */    
    cda_delete_root_aggregate (&root_aggregate_handle);

    if (status != CDA_NORMAL) {
	return (K_FAILURE);
    }
    else {
	return (K_SUCCESS);
    }
}

int Read_LUT_Entry (aggregate_handle, root_aggregate_handle)
    CDAagghandle *aggregate_handle;
    CDArootagghandle *root_aggregate_handle;
{
    int color_entry;
    int pixel;
    MyColor color;
    unsigned long integer_value, *integer_value_addr;
    CDAsize integer_value_length = sizeof (integer_value);

    float float_value, *float_value_addr;
    CDAsize float_value_length = sizeof (float_value);

    CDAconstant aggregate_item;
    unsigned long status;

    aggregate_item = DDIF_RGB_LUT_INDEX;
    status = cda_locate_item (root_aggregate_handle,
			      aggregate_handle,
			      &aggregate_item,
			      (CDAaddress*)&integer_value_addr,
			      &integer_value_length, NULL, NULL);
    if (status != CDA_NORMAL) {
	return (K_DDIF_NO_GOOD);
    }
    new_colors[num_new_colors].pixel = pixel = *integer_value_addr;

    aggregate_item = DDIF_RGB_RED_VALUE;
    status = cda_locate_item (root_aggregate_handle,
			      aggregate_handle,
			      &aggregate_item,
			      (CDAaddress*)&float_value_addr,
			      &float_value_length, NULL, NULL);
    if (status != CDA_NORMAL) {
	return (K_DDIF_NO_GOOD);
    }
    new_colors[num_new_colors].f_red = color.f_red = *float_value_addr;

    aggregate_item = DDIF_RGB_GREEN_VALUE;
    status = cda_locate_item (root_aggregate_handle,
			      aggregate_handle,
			      &aggregate_item,
			      (CDAaddress*)&float_value_addr,
			      &float_value_length, NULL, NULL);
    if (status != CDA_NORMAL) {
	return (K_DDIF_NO_GOOD);
    }
    new_colors[num_new_colors].f_green = color.f_green = *float_value_addr;

    aggregate_item = DDIF_RGB_BLUE_VALUE;
    status = cda_locate_item (root_aggregate_handle,
			      aggregate_handle,
			      &aggregate_item,
			      (CDAaddress*)&float_value_addr,
			      &float_value_length, NULL, NULL);
    if (status != CDA_NORMAL) {
	return (K_DDIF_NO_GOOD);
    }
    new_colors[num_new_colors].f_blue = color.f_blue = *float_value_addr;

    Add_Color (&color, TRUE, &color_entry);
    pixel_remap [pixel] = colormap[color_entry].pixel;


    num_new_colors++;
    return (K_SUCCESS);
}

int Read_Lookup_Table (aggregate_handle_in, root_aggregate_handle)
    CDAagghandle *aggregate_handle_in;
    CDArootagghandle *root_aggregate_handle;
{
    unsigned long integer_value;
    CDAsize       integer_value_length = sizeof (integer_value);

    unsigned long aggregate_type;
    CDAindex      aggregate_index;
    CDAconstant   aggregate_item;
    unsigned long add_info;
    unsigned long status;
    unsigned long *integer_value_addr;
    CDAaddress    aggregate_handle_addr;
    CDAagghandle  aggregate_handle, prv_aggregate_handle;
    CDAsize aggregate_handle_length = sizeof (aggregate_handle);
    int status_int;

    aggregate_item = DDIF_SEG_SPECIFIC_ATTRIBUTES;
    status = cda_locate_item (root_aggregate_handle,
			      aggregate_handle_in,
			      &aggregate_item,
			      &aggregate_handle_addr,
			      &aggregate_handle_length, NULL, NULL);
    if (status != CDA_NORMAL) {
	return (K_DDIF_NO_GOOD);
    }

    aggregate_handle = aggregate_handle_addr;

    aggregate_item = DDIF_SGA_IMG_LOOKUP_TABLES;
    aggregate_index = 0;
    status = cda_locate_item (root_aggregate_handle,
			      &aggregate_handle,
			      &aggregate_item,
			      &aggregate_handle_addr,
			      &aggregate_handle_length,
			      &aggregate_index, NULL);
    if (status != CDA_NORMAL) {
	return (K_DDIF_NO_GOOD);
    }

    aggregate_handle = aggregate_handle_addr;
    status_int = Read_LUT_Entry (&aggregate_handle, root_aggregate_handle);
    if (status_int != K_SUCCESS)
	return (status_int);

    prv_aggregate_handle = aggregate_handle;
    while ((status = cda_next_aggregate (&prv_aggregate_handle,
					 &aggregate_handle)) != CDA_ENDOFSEQ) {
	status_int = Read_LUT_Entry (&aggregate_handle, root_aggregate_handle);
	if (status_int != K_SUCCESS)
	    return (status_int);
	prv_aggregate_handle = aggregate_handle;
    }

    return (status_int);
}

int Read_Color_File (w, stuff, reason)
    Widget   w;
    caddr_t  stuff;
    int	     reason;
{

    CDAsize filename_length;
    CDAaggtype aggregate_type;
    unsigned long aggregate_item, aggregate_index;
    unsigned long status;
    DDISstreamhandle stream_handle;
    CDAfilehandle file_handle;
    CDArootagghandle root_aggregate_handle;
    CDAagghandle aggregate_handle, prv_aggregate_handle;
    int status_int;

    CDAitemlist process_option_list [3];
    int done = FALSE;


    MAKE_ITEM (process_option_list, 0 ,DDIF_INHERIT_ATTRIBUTES, 0, NULL);
    MAKE_ITEM (process_option_list, 1, DDIF_EVALUATE_CONTENT, 0, NULL);
    MAKE_ITEM (process_option_list, 2, 0, 0, NULL);

    aggregate_type = DDIF_DDF;
    filename_length = strlen (temp_file);
    status = cda_open_file (&filename_length, temp_file,
			    NULL, NULL, NULL, NULL, NULL,
			    &aggregate_type,
			    process_option_list,
			    NULL,  /* Default result_filename_len	*/
			    NULL,  /* Default result_filename		*/
			    NULL,  /* Default result_filename_ret_len	*/
			    &stream_handle,
			    &file_handle,
			    &root_aggregate_handle);
    if (status != CDA_NORMAL) {
	return (K_CANNOT_OPEN);
    }

    while (!done) {
	status = cda_get_aggregate (&root_aggregate_handle,
				    &stream_handle,
				    &aggregate_handle,
				    &aggregate_type);
	if (status != CDA_NORMAL) {
	    status_int = K_DDIF_NO_GOOD;
	    done = TRUE;
	}
	else {
	    switch (aggregate_type) {
		case DDIF_SEG :
		    prv_aggregate_handle = aggregate_handle;
		    break;
		case DDIF_IMG :
		    status_int = Read_Lookup_Table (&prv_aggregate_handle,
						    &root_aggregate_handle);
		    done = TRUE;
		    break;
		default : 
		    break;
	    }
	}
    }

/* Close the file */
    cda_close_file (&stream_handle, &file_handle);

/* Delete the Root aggregate */    
    cda_delete_root_aggregate (&root_aggregate_handle);

    return (status_int);
}
