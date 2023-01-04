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
static char *BuildSystemHeader = "$Header: /usr/sde/x11/rcs/x11/src/./motif/clients/session/printscr/creatoutput.c,v 1.1 90/01/01 00:00:00 devrcs Exp $";
#endif		/* BuildSystemHeader */
/*
**++
**  COPYRIGHT (c) 1988, 1989 BY
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
**	creatoutput.c
**	(originally creatdd.c)
**
**  FACILITY:
**      PrintScreen
**
**  ABSTRACT:
**	Subroutines which convert an X11 image
**		to an output format.
**
**  ENVIRONMENT:
**
**	VMS V5.2, Ultrix V3  DW V1+, ISL V1.1+
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
**		Moved include iprdw.h to top so that define ISL could be put in iprdw.h
**
**	9-MAY-1988
**		Remove reference to SPECT_TYPE
**
**	1-JUL-1988
**		Fixed references to ISL constants which ISL renamed
**
**	11-MAY-1989	B. Bazemore
**		Use ISL's conversion to PS for PS output.
**
**	 1-JUN-1989	B. Bazemore
**		Use IDS for sixel generation
**
**	19-JUN-1989	B. Bazemore
**		Do not reset xsca,ysca,xpos, and ypos when
**		NoRotate (NoForm) is selected.  This caused 
**		a blank page as output.
**
**	29-JUN-1989	B. Bazemore
**		Use DPI info in PS scaling operation. Bump PS ident.
**
**	1-AUG-1989	K. Robinson
**              Cast for Ultrix, use CHF Establish on Ultrix
**
**	10-AUG-1989	K. Robinson
**              Add a begin stmt to ps output, so as to use our own dict
**
**	10-AUG-1989	K. Robinson
**		Use requantizing dither to get black&white ouput
**--
**/


extern int dither_signal_handler();

/*
 *	creatdd( options, ximage, file )
 *		options - (RO) (*dxPrscOptions) options file, 
 *			reverse_image and aspect are referenced
 *		ximage 	- (RO) (*XImage) input image
 *		file 	- (WO) (*FILE) opened output file
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
#include <iprdw.h>
#include <stdio.h>
#ifdef ISL



#ifdef VMS
#include <IDS$IMAGE>
#include <img$def>
#else
#include <ids_image.h>
#include <img_def.h>
#endif
#endif

/* Create PS specific includes */
#include	<ps.h>
#ifdef VMS
#include	<time.h>
#else
#include 	<sys/time.h>
#endif

#ifndef TYPE_U_ 
#define TYPE_U_(v)    (*((unsigned long int  *)&(v)))
#endif

#define BMUfac 16

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








creatdd(options, ximage, image_id)

dxPrscOptions *options;
XImage	*ximage;
int image_id;
{
#ifdef ISL
    char *image_data;
    int set_index;
    set_itemlist_entry set_attributes[32];

unsigned char filename[1024];
    vms_descriptor_type image_filename_descriptor;

    int pixels_per_scanline;
    int scanline_count;
    int bits_per_pixel = 8;
    int scanline_stride;
    int bmu_wd, bmu_ht, zero;
    int context;
    int bytes;
    int image_size;
    int status;
    int target_depth;
    int contrast;
    int output_image;



    output_image = image_id;
    if (options->print_color == dxPrscPrinterBW)
	{
    	target_depth = 1;
	contrast = TRUE;
    	status = dither_isl( image_id, 
			 &output_image, 
			 target_depth, 
			 contrast );

    	if (BAD(status))
		return (status);
	}
    if (options->print_color == dxPrscPrinterGrey)
	{
    	target_depth = 1;
	contrast = FALSE;
    	status = dither_isl( image_id, 
			 &output_image, 
			 target_depth, 
			 contrast );

    	if (BAD(status))
		return (status);
	}




    image_filename_descriptor.string_length  = strlen(options->print_dest);
    image_filename_descriptor.string_type    = 14;
    image_filename_descriptor.string_class   = 1;
    image_filename_descriptor.string_address = (unsigned char *)options->print_dest;

/* 
 * Write ISL frame as a DDIF file.  
 * (And you thought DDIF was hard)			
 */

    context = IMG$OPEN_DDIF_FILE(IMG$K_MODE_EXPORT, &image_filename_descriptor, 0, 0);

    IMG$EXPORT_DDIF_FRAME(output_image, 0, context, NULL, NULL, NULL, NULL, NULL);

    IMG$CLOSE_DDIF_FILE(context, NULL);

    IMG$DELETE_FRAME (output_image);
#endif
    return (1);
}

action(ptr,len,usr)
int *ptr,len,usr;
{
static FILE *fileptr;
    fileptr = (FILE *)usr;
    fwrite(ptr,len,1,fileptr);
#ifdef VMS
    fprintf(fileptr, "\n"); /* don't let the records get too big */
#endif
}

                                                                
/* 
** Create Sixel Output
*/
int
creatsx(options, ximage, file, image_id)
dxPrscOptions *options;
XImage	*ximage;
FILE *file;
int image_id;
{
#ifdef ISL
    char		*image_data;
    set_itemlist_entry	set_attributes[7];
    vms_descriptor_type image_filename_descriptor;

    int pixels_per_scanline;
    int scanline_count;
    int bits_per_pixel = 1;
    int scanline_stride;
    int count;
    int bytes;
    int image_size;

 /*   int	bitonal_image, gray_image, color_image; */
 /*    int target_depth;    */

    int	    surface_id;			    /* IDS presentation surface	*/
    unsigned long   item_list_2[30];
    struct IDS$R_RENDERING  *rendering;	    /* Rendering of image for device */
    int	    lstindex, i, len;
    char    *ptr, *insert_ptr;
    int	    devtype;			    /* Device type - LA100	    */
    float   punch1, punch2, grays;	    /* Grayscale adjustment params  */
    float   rotate_angle;		    /* Rotate to landscape mode	    */
    int     x_scale,y_scale;		    /* Device res. measurements	    */
    int	    image_height, image_width;

/* Here I declare and initalize the sixel introducer sequence - the single
 * most often changed code in this project.  This particular introducer
 * works the same on LA50s and LN03s. The problem, however, is that it
 * doesn't take advantage of any nifty sixel features - like better scaling.
 * If someday, there is a way to detirmine whether the printer is an LA50
 * or not, a more sophistcated sequence can be used.  In particular, the
 * DPI information could be used to decide to scale to 90 DPI instead of 72
 * Note that ISL provides scaling routines, but these LOSE information and
 * as of ISL V1, are not acceptable
 */

/*** unsigned char ansi_head[16];
unsigned char sixel_head[4];  
unsigned char sixel_tail[6];
***/
/* 
 *	ansi_head
 *       	CSI!p * reset the printer
 *       	ESC[1r * !Set margin to top
 *       	ESC[d * Move to top
 *		CSI 2 w CSI 3 z  set horz and vertical pitch (DECSHORP DECVERP)
 *	sixel_head
 *       	ESCP9q * start sixels
 *		assumes 72 DPI
 *	sixel_tail
 *       	ESC\ end sixel stream
 *  		 CSI2 I set to decipoints
 */
/****
i = 0;
ansi_head[i++] =  155;
ansi_head[i++] =  33;
ansi_head[i++] =  112;

ansi_head[i++] =  27;
ansi_head[i++] =  91;
ansi_head[i++] =  49;
ansi_head[i++] =  114;

ansi_head[i++] =  27;
ansi_head[i++] =  91;
ansi_head[i++] =  100;

ansi_head[i++] =  155;
ansi_head[i++] =  50;
ansi_head[i++] =  119;
ansi_head[i++] =  155;
ansi_head[i++] =  51;
ansi_head[i++] =  122;
***/
/*      = {  155, 33, 112,
        27, 91, 49, 114,
        27, 91, 100,
	155,  50, 119, 155, 51, 122 };  */
/***
i = 0;
sixel_head[i++] =  27;
sixel_head[i++] =  80;
sixel_head[i++] =  57;
sixel_head[i++] =  113;
***/
/* 	= {
        27, 80, 57, 113 };              */
/***
i = 0;
sixel_tail[i++] =  27;
sixel_tail[i++] =  92;

sixel_tail[i++] =  155;
sixel_tail[i++] =  50;
sixel_tail[i++] =  32;
sixel_tail[i++] =  73;
***/
/*	= {
                27, 92,
                155, 50, 32, 73 };      */


/*
 * store image metrics						
 */
    bytes = ximage->bytes_per_line;
    pixels_per_scanline = ximage-> width;
    scanline_count = ximage -> height;
    scanline_stride = BITSINBYTE * bytes;	/*  BITSINBYTE = 8 */
    image_data = ximage -> data;
    image_size = bytes * scanline_count;

    image_height = ximage->height;
    image_width  = ximage->width;

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

    /* Create an item list for Create Presentation Surface		*/

    lstindex = 0;

    /* Establish sixel type - color or generic B&W - by specifying
     * a device template.

     */
    item_list_2[lstindex++] = (unsigned)(IDS$C_NTEMPLATE);	    /* Item code	*/

    if (options->print_color == dxPrscPrinterColor) 
	{
	item_list_2[lstindex++] = IDS$C_TMPLT_LJ250; /* Companion Color Printer	*/
	}
    else
	{
	item_list_2[lstindex++] = IDS$C_TMPLT_LA100; /* B&W Printer	*/

	/* If the user asked for 1 to 1 aspect ratio, make sure they
	 * get it.  This involves modifying the presentation surface 
	 * resolution.
	 */
	if (options->aspect != dxPrscPixAsp2)
	    {
	    /* Use Basic Measuring Unit scale - 1/1200 of an inch   */
	    item_list_2[lstindex++] = (unsigned)(IDS$C_NUNITS);
	    item_list_2[lstindex++] = IDS$C_UNITS_BMU;

	    y_scale = 17;		/* BMUs between pixels		*/
	    x_scale = 17;		/* normally x = 34
		*/
	    item_list_2[lstindex++] = (unsigned)(IDS$C_NY_DISTANCE);    /* Item code	*/
	    item_list_2[lstindex++] = y_scale;
	    item_list_2[lstindex++] = (unsigned)(IDS$C_NX_DISTANCE);    /* Item code	*/
	    item_list_2[lstindex++] = x_scale;
	    }   
	}

    item_list_2[lstindex++] = 0;		    /* Null terminate	*/
    item_list_2[lstindex++] = 0;

    /* Basically, this tells IDS that we want (color or b&w) sixels */    
    surface_id  = IDS$CREATE_PRESENT_SURFACE( item_list_2);
    /* 
     * Set up Rendering item list   
     */
    lstindex = 0;

    /* Bluenoise algorithm is most CPU intensive, but gives
     * the best results.
     */    
    item_list_2[lstindex++] = (unsigned)(IDS$C_NDITHER_ALGORITHM);

    if (options->print_color == dxPrscPrinterBW)
    	item_list_2[lstindex++] = IDS$K_REQUANTIZE;
    else
    	item_list_2[lstindex++] = IMG$K_DITHER_DISPERSED;



    /* Rotate and scale the image if user requested it */

    if (options->form_feed == dxPrscNoForm)
	{
	/* User didn't request any rotation or scaling 	*/

	item_list_2[lstindex++] = (unsigned)(IDS$C_NROTATE_MODE);
	item_list_2[lstindex++] = IDS$C_NO_ROTATE;

	item_list_2[lstindex++] = (unsigned)(IDS$C_NSCALE_MODE);
/*	item_list_2[lstindex++] = IDS$C_PHYSICAL; */
	item_list_2[lstindex++] = IDS$C_NO_SCALE; 
	}
    else
	{
	/* User asked that image fit on a printed page, rotate and
	 * scale accordingly.
	 */
	if ((image_width > UPSPAGEWID) && (image_width > image_height))
	    {
	    /* 
	     * Rotate the image for landscape mode.
	     * Update local height and width to reflect the change
	     * You MUST use  to cast rotate angle or it's
	     * type will be wrong, or else it won't be a float 
	     */
	    item_list_2[lstindex++] = (unsigned)(IDS$C_NROTATE_MODE);
	    item_list_2[lstindex++] = IDS$C_ROTATE;
	    rotate_angle = 90.0;
	    item_list_2[lstindex++] = (unsigned)(IDS$C_NROTATE_ANGLE);
	    item_list_2[lstindex++] = TYPE_U_(rotate_angle);

	    i = image_height;
	    image_height = image_width;
	    image_width  = i;
	    }
	/*
	 * If either the height or width of the image will overflow
	 * the page, scale it down to fit.  Otherwise, leave it
	 * the original size.
	 */
	if ((image_width > UPSPAGEWID) || (image_height > UPSPAGELEN))
	    {
	    item_list_2[lstindex++] = (unsigned)(IDS$C_NSCALE_MODE);
	    item_list_2[lstindex++] = IDS$C_FIT_WITHIN;
	    }
	else
	    {
	    item_list_2[lstindex++] = (unsigned)(IDS$C_NSCALE_MODE);
/*	    item_list_2[lstindex++] = IDS$C_PHYSICAL; */
	    item_list_2[lstindex++] = IDS$C_NO_SCALE;
	    }
	
	} 


    /* Terminate the rendering item list    */

    item_list_2[lstindex++] = 0;
    item_list_2[lstindex++] = 0;

    /* This creates the sixel data; does all dithering, etc too! */

    rendering = IDS$CREATE_RENDERING( image_id, surface_id, item_list_2);

    /* Get the size and address of the rendered sixel data   */

    count =  rendering->RND$R_TYPE_SPEC_DATA.RND$R_SIXEL.RND$L_SXL_BYTCNT;
    ptr	  =  rendering->RND$R_TYPE_SPEC_DATA.RND$R_SIXEL.RND$A_SXL_BUFPTR;

    
    /* 
     * Skip past IDS's "[ESC]Pq" sixel header, so we can insert our own, 
     * more generic, "[ESC]P9q"
     */
    if (options->print_color != dxPrscPrinterColor) 
	{
    	insert_ptr = (char *)strchr( ptr, '\033');	    
    	insert_ptr = (char *)strchr( insert_ptr, 'P');
    	insert_ptr = (char *)strchr( insert_ptr, 'q');
    	insert_ptr++;
    	ptr = insert_ptr;
    	fprintf( file, "\033P9q");
	}

    /* Now we write the sixels to an output file */
    
    for (i = 0; i < count; i += 500, ptr += 500)
	{
    	len = (count - i > 500) ? (500) : (count - i);  
    	fwrite( ptr, len, 1, file);

#	ifdef vms
    	    fprintf(file, "\n"); /* don't let the records get too big */
#	endif
        }

#endif

IDS$DELETE_PRESENT_SURFACE(surface_id);
IDS$DELETE_RENDERING( rendering);
IMG$DELETE_FRAME(image_id);

return (Normal);	
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	psbe_emit_image_line is called from the ISL IMG$EXPORT_PS
**	routine once for each line of PS image data that has
**	been converted from DDIF image data.  This routine will
**	emit a line of PS image data.
**
**	This routine is only called during the do_emit pass.
**
**  FORMAL PARAMETERS:
**
**	buf_ptr	- IN: pointer to PS image data
**	buf_len	- IN: number of bytes of PS image data.
**	file    - IN: PS file to write image to
**
**  IMPLICIT INPUTS:
**
**	None
**
**  IMPLICIT OUTPUTS:
**
**	None
**
**  FUNCTION VALUE: 
**
**	None
**
**  SIDE EFFECTS:
**
**	None
**
**--
**/

int	prsc_emit_image_line(buf_ptr, buf_len, file)

unsigned char *buf_ptr;
int	      buf_len;
FILE	      *file;
{
unsigned char		*img_ptr1;	/* pointers to image content	*/
unsigned char		*img_ptr2;
int			line_length;
	/*
	* Break the PS image content into line size chunks
	* and put them out to the file
	*/
	img_ptr1 = buf_ptr;	/* current start of line ptr	*/
	img_ptr2 = img_ptr1;	/* current end of line ptr	*/

	/*
	* Loop until we run out of image data
	*/
	while (TRUE)
	    {
	    /* Check for end of buffer */
	    if (img_ptr2 >= (unsigned char *)(buf_ptr + buf_len))
		break;

	    line_length = img_ptr2 - img_ptr1 + 1;

	    /* Advance and check for end of line	*/
	    if ((*img_ptr2++ == '\n')
		|| (line_length == MAX_OUTPUT_BUFFER))
		{

		/* Write a line of image data, terminated with an EOL	*/

		fwrite(img_ptr1,line_length,1, file);

		if (line_length == MAX_OUTPUT_BUFFER)
 		    fprintf(file, "\n");

		/* Start new line where we left off */
		img_ptr1 = img_ptr2;
		}
	    }

        return SS$_NORMAL;

} /* end prsc_emit_image_line */

/*
 *	creatps( options, ximage, file, image_id )
 *		options - (RO) (*dxPrscOptions) options structure, only
 *			reverse_images is referenced
 *		ximage 	- (RO) (*Ximage) image to be converted
 *		file 	- (WO) (*FILE) opened output file
 *	        image_id - (RO) (int) Image Services frame ident
 *
 *	requires:
 *
 * 	restrictions:
 *		
 *	other:
 *		no  implicit parameters, etc.
 *
 */

int	creatps( options, ximage, file, image_id )
dxPrscOptions *options;
XImage	*ximage;
FILE	*file;
int	image_id;
{
	int	bitonal_image;	    /* ISL frame ident	*/
	int	DPI;
	int	xsca, ysca;	/* x and y scale factors		*/
	int	xpos, ypos;	/* coordinates of image origin		*/
	int	target_depth;	/* number of planes in PostScript image	*/
	int	contrast;	/* punch to black and white if true    	*/
	bool	toobig;		/* true when image is too big for page	*/
	bool	rotate;		/* true when image should be rotated	*/
	int	status;		/* return status			*/
	struct tm *timest;
	int time_val;
static char *month[12]= {"JAN","FEB", "MAR", "APR", "MAY", "JUN", "JUL",
			 "AUG","SEP","OCT","NOV","DEC"};


/*
 * Find amounts to scale, translate and rotate image for best
 * fit on 8.5 x 11 piece of paper
 * These numbers are also used to calculate the bounding box
 * for the header comments.
 */
    DPI = DPIof(options->dpy);      /* Get resolution of display device	*/
    toobig = False;
    xsca = ximage->width;   /* scale factor includes wid and hei because by */
    ysca = ximage->height;  /* convention, Postscript images are 1 x 1      */


/* Only rotate if  1a) the image is wider that the paper
 * AND 1b) The image is wider than it is long.
 * 
 * Do not rotate if NoRotate (NoForm) is selected.
 */
    if ( (( ximage->width <=  ximage->height) || (ximage->width < UPSPAGEWID))
      || (options->form_feed == dxPrscNoForm))
    {
		/* UNROTATED CASE */
		if (options->form_feed == DECW$C_PRSC_NOFORM )
		{
			/* don't scale either */
			xsca = ximage->width * 72/DPI;
			ysca = ximage->height * 72/DPI;
			/* center if it's smaller than the page 
			 * otherwise use upper left corner 
			 */
			if (xsca > UPSPAGEWID) 
				xpos = (PSPAGEWID - UPSPAGEWID)/2;
			else xpos = (PSPAGEWID - xsca)/2;
			if (ysca > UPSPAGELEN)
				ypos = (PSPAGELEN - UPSPAGELEN)/2;
			else ypos = (PSPAGELEN - ysca)/2;
		}
		else 
		{
			/* scale if necessary to fit page */
			if(ximage->width > UPSPAGEWID)
			{
				ysca *= UPSPAGEWID;
				ysca /= xsca;
				xsca = UPSPAGEWID;
				toobig = True;
			}
			if( ysca > UPSPAGELEN)
			{
				xsca *= UPSPAGELEN;
				xsca /= ysca;
				ysca = UPSPAGELEN;
				toobig = True;
			}
			if(!toobig)
			{
				/* Convert to Centipoints, then scale */
				xsca *= 72;
				xsca /= DPI;
				ysca *= 72;
				ysca /= DPI;
			}
			xpos = (PSPAGEWID - xsca)/2;
			ypos = (PSPAGELEN - ysca) /2;
		}
		rotate = FALSE;
	}
	else
	{
		/* ROTATED CASE */
		if(ximage->width > UPSPAGELEN)
		{
			ysca *= UPSPAGELEN;
			ysca /= xsca;
			xsca = UPSPAGELEN;
			toobig = True;
		}
		if( ysca > UPSPAGEWID)
		{
			xsca *= UPSPAGEWID;
			xsca /= ysca;
			ysca = UPSPAGEWID;
			toobig = True;
		}
		if(!toobig)
		{
			xsca *= 72;
			xsca /= DPI;
			ysca *= 72;
			ysca /= DPI;
		}

		xpos = PSPAGEWID-((PSPAGEWID-ysca)/2);
		ypos = ((PSPAGELEN-xsca)/2);
		rotate = TRUE;

    }
/*
 * Header as described in Encapulated Postscript File specification
 * Header info includes: Bounding box of image (in standard PS coord system)
 * Creator, creation date, and file name.
 */
    time(&time_val);
    timest = localtime(&time_val);

#ifndef VMS
    fprintf(file, "%%!");
#endif
    fprintf(file, "%%!PS-Adobe-2.0 EPSF-1.2\n");
    fprintf(file, "%%%%Title: %s\n", (options->print_dest == NULL) ?
 		"stdout" : options->print_dest );
    fprintf(file, "%%%%Creator: DECW$PRINTSCREEN V2 EFT-1 \n");
    fprintf(file, "%%%%CreationDate: %d-%s-19%d  \n",
		timest->tm_mday,month[timest->tm_mon], timest->tm_year);
    fprintf(file, "%%%%Pages: 1 \n");
    fprintf(file, "%%%%BoundingBox:");
    if (!rotate)
		fprintf(file, "%d %d %d %d\n",
				xpos, ypos,
				xpos + xsca, ypos + ysca);
    else
		fprintf(file, "%d %d %d %d\n",
				xpos- ysca, ypos,
				xpos, ypos + xsca);

    fprintf(file, "%%%%EndComments \n");
    fprintf(file, "%%%%EndProlog \n");
    fprintf(file, "%%%%Page: 1 1 \n");



    /*
    * Save our state in a temporary.
    */
    fprintf(file, "15 dict begin\n");
    fprintf(file, "/temp-save save def\n");

/*
 * Write out scaling and rotating comands
 */
    fprintf(file,"%d %d translate\n",xpos, ypos);
    if (rotate)
	fprintf(file, "90 rotate\n");
    fprintf(file, "%d %d scale\n", xsca, ysca );



    /*
    * Scale for the entire $I frame size.  Then
    * re-define "scale" so that any scale done
    * by ISL will be a no-operation.  This assumes
    * that ISL generated code will use the unit
    * square technique...
    */
/*^&    frame_x = frame->frm_bbox_ur_x - frame->frm_bbox_ll_x;
    frame_y = frame->frm_bbox_ur_y - frame->frm_bbox_ll_y;
    if ((frame_x != 0) && (frame_y != 0))
	{
	EMIT_MEASURE (frame_x);
	EMIT_MEASURE (frame_y);
	EMIT_TOKEN ("scale");
	}
 ^&*/
    /*
    * Redefine "scale" so that any scale done by ISL will 
    * be a no-operation.  Also redefine "showpage" to be a no-op.
    */
    fprintf(file, "/scale { pop pop } bind def\n");
    fprintf(file, "/showpage {} def\n");

    /*
    * Emit an Encapsulated PS comment so ISL's Header and Trailer
    * comments can be parsed by others.
    */
    fprintf(file, "%%%%BeginDocument: Image Services\n");

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

    /*
    * Dither the image down to a bitonal for ISL, if it isn't 
    * bitonal already.
    *
    * Eventually the dither_isl and IMG$EXPORT_PS calls will
    * be replaced by IDS calls which will handle color PS.
    */
    target_depth = 1;
    if (options->print_color == dxPrscPrinterBW)
	contrast = TRUE;
    else
	contrast = FALSE;


    status = dither_isl( image_id, 
			 &bitonal_image, 
			 target_depth, 
			 contrast );

    if (BAD(status))
	return (status);
 
    /*
    * Convert the DDIF image to a PS image format.
    * This routine will call psbe_emit_image_line
    * for each line of image data.
    */
    IMG$EXPORT_PS (bitonal_image,
		   0,			/* region of interest	*/
		   0,			/* buffer pointer	*/
		   0,			/* buffer length	*/
		   0,			/* # of bytes exported	*/
		   0,			/* flags		*/
		   prsc_emit_image_line,/* action routine	*/
		   file);		/* action routine param	*/

    IMG$DELETE_FRAME (bitonal_image);

    /*
    * Encapsulated comments for end of document
    */
    fprintf(file, "\n%%%%EndDocument\n");

    /*
    * Restore our state from the temporary.
    */
    fprintf(file, "temp-save restore\n");

    /*
    * Wrap up the document
    */
    fprintf(file, "end\n");
    fprintf(file, "showpage\n");
    fprintf(file, "%%%%Trailer \n");

    /*
    * Announce our end... (DEC-specific comment)
    */
    fprintf(file, "%%End-of-file\n");
    return( SS$_NORMAL );

}   /* end of creatps	*/

