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
static char *BuildSystemHeader = "$Header: /usr/sde/x11/rcs/x11/src/./motif/clients/session/printscr/dither.c,v 1.1 90/01/01 00:00:00 devrcs Exp $";
#endif		/* BuildSystemHeader */
/***********************************************************************
*
* COPYRIGHT (c) 1989 BY
* DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.
* ALL RIGHTS RESERVED.
*
* THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED
* ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE
* INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER
* COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
* OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY
* TRANSFERRED.
*
* THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE
* AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT
* CORPORATION.
*
* DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS
* SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.
***********************************************************************/

/***********************************************************************
*
*  MODULE NAME:
*	Dither.c
*
*  FACILITY:
*
*	PrintScreen
*
*  ABSTRACT:
*
*	This program converts a multi-plane DDIF image frame into a
*	1 plane, bitonal DDIF image frame.
*
*  ENVIRONMENT:
*
*      VMS V5.2, Ultrix V3.0  
*      ISL V1.1+
*
*  AUTHOR(S):
*
*      B. Bazemore
*	based on ISL example routine
*
*  CREATION DATE:
*
*      May 8, 1989
*
*  MODIFICATION HISTORY:
*
*	B. Bazemore	26-May-1989
*	    Mono and color frames have different numbers of components.
*	    Asking for all three RGB components on a mono frame causes 
*	    an ISL error.  Check # of components, then get component values.
*
***********************************************************************/

/*
**  VAX-C library
*/
#include <stdio.h>
#include <ChfDef.h>                            /* condition handler structs */
#include <ssdef.h>                             /* sys services messages    */
 
/*
**  ISL library
*/
#ifdef VMS
#include <img$def.h>
#include <img$entry.h>

#else
#include <img_def.h>
#include <img_entry.h>
#endif
/*
**  PrintScreen definitions
*/
#include        <iprdw.h>

/*
**  MACRO to define an ISL item list structure
*/
#define PUT_ITMLST_ENTRY(item,buffer,index)			\
    {								\
    image_deflst[put_index].PUT$L_CODE = item;			\
    image_deflst[put_index].PUT$L_LENGTH  = sizeof(buffer);	\
    image_deflst[put_index].PUT$A_BUFFER  = (char *) &buffer;	\
    image_deflst[put_index++].PUT$L_INDEX = index;		\
    }

#define END_PUT_ITMLST						\
    {								\
    image_deflst[put_index].PUT$L_CODE = NULL;			\
    image_deflst[put_index].PUT$L_LENGTH  = NULL;		\
    image_deflst[put_index].PUT$A_BUFFER  = (char *) NULL;	\
    image_deflst[put_index++].PUT$L_INDEX = NULL;		\
    }

/*  Get itemlist structure is a bit different from put 
*
*    image_itmlst[0].GET$L_CODE = IMG$_PIXELS_PER_LINE; 
*    image_itmlst[0].GET$L_LENGTH = 4;
*    image_itmlst[0].GET$A_BUFFER = (char *) &input_width;
*    image_itmlst[0].GET$A_RETLEN = 0;
*    image_itmlst[0].GET$L_INDEX = 0;
*/
#define GET_ITMLST_ENTRY(item,buffer,index)			\
    {								\
    image_getlst[get_index].GET$L_CODE = item;			\
    image_getlst[get_index].GET$L_LENGTH = sizeof(buffer);	\
    image_getlst[get_index].GET$A_BUFFER = (char *) &buffer;	\
    image_getlst[get_index].GET$A_RETLEN = 0;			\
    image_getlst[get_index++].GET$L_INDEX = index;		\
    }

#define END_GET_ITMLST						\
    {								\
    image_getlst[get_index].GET$L_CODE = NULL;			\
    image_getlst[get_index].GET$L_LENGTH = NULL;		\
    image_getlst[get_index].GET$A_BUFFER = (char *) NULL;	\
    image_getlst[get_index].GET$A_RETLEN = 0;			\
    image_getlst[get_index++].GET$L_INDEX = NULL;		\
    }



/* color to gray lookup table, array descriptor */
#ifdef VMS
globalref IMG$R_RGB332_Y8_LUT;

#else 
 struct IMG_LUT_DSC
     {
     unsigned short int length;
     unsigned char      dtype;
     unsigned char      class;
     unsigned char      *addr;
     unsigned char      scale;
     unsigned char      digits;
     unsigned char      aflags;
     unsigned char      dimct;
     unsigned long int  arsize;
     };
 
extern	struct IMG_LUT_DSC IMG$R_RGB332_Y8_LUT;

#endif


/*
 * Condition handler for signals received from Image Services.
 */
long dither_signal_handler (sigarg, mcharg)
    struct chf$signal_array     *sigarg;
    struct chf$mech_array       *mcharg;
{

    if (sigarg->chf$l_sig_name == SS$_UNWIND)
        return SS$_RESIGNAL;

    if (sigarg->chf$l_sig_name == SS$_DEBUG)
        return SS$_RESIGNAL;

    if (!BAD(sigarg->chf$l_sig_name))
        return SS$_CONTINUE;

#ifdef VMS
    /*
     * Return with status to whoever called the establisher
     * of this condition handler.
     */
    return LIB$SIG_TO_RET (sigarg, mcharg);
#else
    /*
     * Nothing fancy for non-VMS systems.
     */
    return sigarg->chf$l_sig_name;
#endif  /* not vms              */
}



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	dither_isl accepts an ISL (Image Services Library) frame 
**	identifier, and dithers the image as necessary to get it 
**	down to a 1 plane bitonal image.  
**  
**	Each time the image is dithered down, the previous image
**	frame is deallocated to conserve memory.
**
**  FORMAL PARAMETERS:
**
**	input_frame  - IN: a valid ISL frame ident, 24 planes or less
**			   by value
**	output_frame - OUT: pointer to a valid ISL frame ident, 1 plane
**			    by reference
**	target_depth - IN: the number of planes in the output_frame image
**			    by value
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	status - Normal or image services status
**
**  SIDE EFFECTS:
**
**	Do not pass the same pointer for the input frame and the 
**	output frame, since these frames may be operated on simultaneously.
**
**	The input_frame pointer will be invalid after this routine is called.
**
**	ISL routines will signal errors.  This routine establishes a
**	condition handler to catch any ISL errors and return a generic
**	image dither error.
**
**--
**/

long dither_isl( input_frame, output_frame, target_depth, contrast )

int input_frame;		    /* ISL frame identifiers	*/
int *output_frame;
int target_depth;		    /* # planes desired		*/
int contrast;			    /* gray dithering or black&white */
{
    int	out_frame;		    /* local copy of output_frame	      */
    int tmp_fid;		    /* temporary ISL frame pointer	      */


    /*
    **	Variables that will receive image attributes and the ISL item list
    **	to be passed to IMG$GET_FRAME_ATTRIBUTES that describe them.
    */
#define MAX_NUM_COMPONENTS  3
    int component_bits[MAX_NUM_COMPONENTS]; /* RGB components	    */
    int input_width, input_height;  /* Image width and height in pixels	    */
    int number_of_components;	    /* number of components per pixel	    */
    int pixel_stride;		    /* #bits allocated per pixel	    */
    int pixel_bits;		    /* # bits per pixel, 1, 8, 24	    */
    int scanline_stride;	    /* pixel_stride x input_width	    */
    int spectral_type, spectral_map;/* bitonal, gray, color    */    
    int component_index = 8;	    /* index of component 1 in item lists   */
    int put_index = 0;
    int get_index = 0;
    int	comp_index;		    /* loop index   */
    float punch1, punch2;
  /* 
   * always keep component bits as last entries in the item lists.  If the
   * position of the component bit entries change, change component_index
   * value as well.
   *
   * image_deflst is used for defining image frame attributes.
   * image_itmlst is used for obtaining image frame attribute values.
   */
    struct PUT_ITMLST image_deflst[12];
    struct GET_ITMLST image_getlst[12];

 PUT_ITMLST_ENTRY(IMG$_PIXELS_PER_LINE,   input_width,	        0)
 PUT_ITMLST_ENTRY(IMG$_NUMBER_OF_LINES,   input_height,         0)
 PUT_ITMLST_ENTRY(IMG$_PIXEL_STRIDE,	  pixel_stride,         0)
 PUT_ITMLST_ENTRY(IMG$_BITS_PER_PIXEL,	  pixel_bits,	        0)
 PUT_ITMLST_ENTRY(IMG$_SCANLINE_STRIDE,   scanline_stride,      0)
 PUT_ITMLST_ENTRY(IMG$_SPECTRAL_MAPPING,  spectral_map,        0)
 PUT_ITMLST_ENTRY(IMG$_NUMBER_OF_COMP,    number_of_components, 0)
 PUT_ITMLST_ENTRY(IMG$_IMG_BITS_PER_COMP, component_bits[0],    0)
 PUT_ITMLST_ENTRY(IMG$_IMG_BITS_PER_COMP, component_bits[1],    1)
 PUT_ITMLST_ENTRY(IMG$_IMG_BITS_PER_COMP, component_bits[2],    2)
 END_PUT_ITMLST

 GET_ITMLST_ENTRY(IMG$_PIXELS_PER_LINE,   input_width,	        0)
 GET_ITMLST_ENTRY(IMG$_NUMBER_OF_LINES,   input_height,         0)
 GET_ITMLST_ENTRY(IMG$_PIXEL_STRIDE,	  pixel_stride,         0)
 GET_ITMLST_ENTRY(IMG$_BITS_PER_PIXEL,	  pixel_bits,	        0)
 GET_ITMLST_ENTRY(IMG$_SCANLINE_STRIDE,   scanline_stride,      0)
 GET_ITMLST_ENTRY(IMG$_SPECT_TYPE,        spectral_type,        0)
 GET_ITMLST_ENTRY(IMG$_SPECTRAL_MAPPING,  spectral_map,        0)
 GET_ITMLST_ENTRY(IMG$_NUMBER_OF_COMP,    number_of_components, 0)
 END_GET_ITMLST
    
    /*
    * Establish the condition handler here to specifically take
    * care of image services routines which may signal an error.
    * If one of the image routines signal an error then this routine
    * will return with that error status.
    */
#ifdef VMS
    LIB$ESTABLISH (dither_signal_handler);
#else
    ChfEstablish (dither_signal_handler);
#endif

    /* Get basic info about the input image, height, width, number of planes */

    IMG$GET_FRAME_ATTRIBUTES( input_frame, image_getlst);

    /*
     * Now that we know the number of components we have, get the
     * value of each component.  Create a new Get item list.
     */
    if (number_of_components > MAX_NUM_COMPONENTS)
	/* We don't handle this many color components	*/
	return ( FUNERROR );

    get_index = 0;
    for (comp_index = 0;  comp_index < number_of_components;  comp_index++)
	{
	GET_ITMLST_ENTRY(  IMG$_IMG_BITS_PER_COMP 
			 , component_bits[comp_index]
			 , comp_index)
	}
    END_GET_ITMLST
    IMG$GET_FRAME_ATTRIBUTES( input_frame, image_getlst);

    /*
    **  Dither the input image down to 8-plane RGB
    **	    red = 3 plane, green = 3, blue =2   (3+3+2 = 8)
    **
    */
    if ((target_depth <= 8) && (component_bits[0] > 3))
	{
	pixel_stride = 8;
	scanline_stride = pixel_stride * input_width;
	pixel_bits = 8;

	component_bits[0] = 3;
	component_bits[1] = 3;
	component_bits[2] = 2;

	out_frame = IMG$DITHER(
			 input_frame,	    /* FID, input image frame id    */
			 image_deflst,	    /* ITMLST, put options	    */
			 IMG$K_DITHER_DISPERSED, /* ALGORITHM, dither algo	    */
			 0,		    /* THRESHOLD SPEC, not used for blue */
			 0,		    /* FLAGS, processing flags	    */
			 0,		    /* ROI, region of interest	    */
			 0 );		    /* LEVELS, for each spectral comp	*/  

       /* Free up memory from previous version of the image	*/

       IMG$DELETE_FRAME( input_frame );
       }
    else
       /* image is already 8 or less planes */
       out_frame = input_frame;

   /*
    * There is only one component per pixel when we go to B&W. Null
    * out the two extra components by erasing them from the 
    * current frame, and terminating the put-item-list early
    * so subsequent dithered images won't have the components 
    * specified.
    */
    if (number_of_components > 2)
	{
	out_frame = IMG$ERASE(
			    out_frame
			  , image_deflst[component_index].PUT$L_CODE
			  , 2		    /* component_bit[2] */
			  );
	}

    if (number_of_components > 1)
	{
	out_frame = IMG$ERASE(
			    out_frame
			  , image_deflst[component_index].PUT$L_CODE
			  , 1		    /* component_bit[1] */
			  );
	}

    /* Terminate put-list at first component item   */

    image_deflst[component_index].PUT$L_CODE   = 0;
    image_deflst[component_index].PUT$L_LENGTH = 0;
    image_deflst[component_index].PUT$A_BUFFER = 0;


    /*
    ** Convert from color to grayscale if necessary.
    */
    if (   (target_depth < 8) 
        && (spectral_type != IMG$K_STYPE_BITONAL)
	&& (spectral_type != IMG$K_STYPE_GREYSCALE)
	)
	{
	/* 8 shades of gray vs. Red-Green-Blue		    */
	pixel_stride = 8;
	pixel_bits   = 8;
	scanline_stride = pixel_stride * input_width;
	spectral_map   = IMG$K_MONOCHROME_MAP;

	number_of_components = 1;
	component_bits[0] = 8;
	
        /* Do the conversion from color to grayscale	*/

	tmp_fid = IMG$PIXEL_REMAP(
			out_frame,
			&IMG$R_RGB332_Y8_LUT,	/* lookup table descrip	*/
			image_deflst,		/* image put itmlist	*/
			0,			/* flags, MBZ		*/
			0,			/* user remap routine	*/
			0,			/* user routine param	*/
			0			/* region of interest	*/
			);

	/* Do some cleanup		*/
	IMG$DELETE_FRAME( out_frame );
	out_frame = tmp_fid;
	}

  /*
  ** If requested, increase the contrast as much as possible 
  */

    if ( (target_depth == 1) && (pixel_stride > 1) && contrast )
	{

	punch1 = 0.5;
	punch2 = 0.50001;

	tmp_fid = IMG$TONESCALE_ADJUST(
				out_frame,
				&punch1,
				&punch2,
				0,0,0);
	IMG$DELETE_FRAME( out_frame );
	out_frame = tmp_fid;
	}

    

    /*
    ** Get down to bitonal.  At this point we have a bitonal or grayscale
    ** image.  If it is grayscale dither it down from 8 shades of gray
    ** to black and white.
    */

    if ( (target_depth == 1) && (pixel_stride > 1))
	{

	/* Set up for bitonal dither	*/
	pixel_stride    = 1;
	pixel_bits	= 1;
	scanline_stride = pixel_stride * input_width;
	spectral_map   = IMG$K_MONOCHROME_MAP;

	/* Black or white pixels    */
	component_bits[0] = 1;


	tmp_fid = IMG$DITHER(
		out_frame,		    /* FID, input image frame id    */
		image_deflst,		    /* ITMLST, put-list options	    */
		IMG$K_DITHER_DISPERSED,	    /* ALGORITHM, dither algorithm  */
		0,			    /* THRESHOLD SPEC, not used for blue */
		0,			    /* FLAGS, processing flags	    */
		0,			    /* ROI, region of interest	    */
		0 );			    /* LEVELS, for each spectral comp	*/  

	IMG$DELETE_FRAME( out_frame );
	out_frame = tmp_fid;
	}

    *output_frame = out_frame;
    return ( Normal );
}
