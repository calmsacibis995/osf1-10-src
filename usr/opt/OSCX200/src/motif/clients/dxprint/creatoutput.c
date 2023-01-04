/* 
**++
**  COPYRIGHT (c) 1988, 1989, 1990, 1991, 1992 BY
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
*/

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
**	30-Nov-93	pjw
**		Add comments.
**
**	29-Jan-1992	Edward P Luwish
**		Many experimental changes to support new interfaces.  To
**		be replaced by RENDER.C
**
**	 07-Jan-1991	Ed Luwish
**		Corrected problem with B/W PostScript looking the same as
**		Grayscale PostScript (i.e. with dithered gray values rather
**		than high contrast black and white).
**
**	 30-Nov-1990	Kathy Robinson
**		Put the dithers back in, Thank You
**		Don't use DECimage to scale rotate when postscript is *much*
**			faster
**		Use spiffy new low-res color sixel template from DECimage
**
**	05-Nov-1990	Krishna Mangipudi
**		1. creatdd - Modified to call ids
**		2. creatsx - Deleted call to dither
**		3. creatps - Deleted call to dither
**			   - Changed over to new postscript templates
**
**	 14-Aug-1990	Kathy Robinson
**		creatdd was not returning a success code
**
**	 3-Aug-1990	Barbara Bazemore
**		Revamp action routine to obey carriage returns in buffer data
**		and return a value.
**
**	30-JUN-1990	KMR
**		Shorten ps and sixel records to 255 for edt and other old stuff
**		Fix version info in ps comments
**
**	27-JUN-1990	KMR
**		Clear up sixel memory leak!
**
**	13-JUN-1990	KMR
**		Fix aspect ratio and landscape problem
**		Don't let ps records get too big
**		Fix ps norotate logic
**
**	17-MAY-1990	KMR
**		Fix noscale problem
**
**      6-FEB-1990
**              Added extra %! for Ultrix and Fixed sixel noscale option
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
**	07-DEC-1992	Dhiren M Patel
**		Allow image services to write correct header on sixel
**		data. Enable line breaks in sixel data on Unix.
**		Fix seg faulting.
**	10-DEC-1992	Dhiren M Patel
**		Fix ootb_bug 381. Added initialization for variables
**		xpos and ypos in creatps ().
**		Fix ootb_bug 476. Added code to set the default values
**		for command line captures and put in code so that the
**		code for gui captures is not affected by this change.
**		Change made in creatps ().
**	20-DEC-1992	Dhiren M Patel
**		In creatps changed long if else sequence to a switch.
**--
*/

#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /b5/aguws3.0/aguws3.0_rcs/src/dec/clients/print/creatoutput.c,v 1.2 91/12/30 12:48:20 devbld Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */

#include "iprdw.h"

#include <stdio.h>
#include "smshare.h"

#ifdef VMS
#include <time.h>
#else
#include <sys/time.h>
#endif

#if defined(VMS) && !defined(__DECC)
#pragma nostandard
#endif
#include <Xm/Xm.h>
#include <img/IdsImage.h>
#include <img/ImgDef.h>
#include <img/ImgEntry.h>
#if defined(VMS) && !defined(__DECC)
#pragma standard
#endif

#include "prdw_entry.h"

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
int creatdd
#if _PRDW_PROTO_
(
    dxPrscOptions	*options,
    int			height,
    int			width,
    XImage		*ximage,
    unsigned long	image_id,
    unsigned int	converted,
    Display		*dpy,
    int			dfs
)
#else
(options, height, width, ximage, image_id, converted, dpy, dfs)
    dxPrscOptions	*options;
    int			height;
    int			width;
    XImage		*ximage;
    unsigned long	image_id;
    unsigned int	converted;
    Display		*dpy;
    int			dfs;
#endif
{
    IdsItmlst2		ps_itmlst[10];
    IdsItmlst2		rn_itmlst[10];
    IdsRendering	*rendering;	
    unsigned long	context;
    unsigned long	psid;
    unsigned long	output_image;
    int			lstindex;
    unsigned long	tmplt,rendc;
    unsigned int	system_color;

    output_image = image_id;

    lstindex = 0;

    ps_itmlst[lstindex].item_name = (char *) IdsNtemplate;
    system_color = determine_system_color (dpy, dfs);
    switch (system_color)
    {
    case color_system:
    case gray_system:
	ps_itmlst[lstindex++].value = Ids_TmpltVsGPX;
	break;
    case black_white_system:
	ps_itmlst[lstindex++].value = Ids_TmpltVsII;
	break;
    }

    ps_itmlst[lstindex].item_name = (char *) IdsNrenderingClass;
    if (command.options.print_color == dxPrscPrinterBW) 
	rendc = Ids_Bitonal;
    else if (command.options.print_color == dxPrscPrinterGrey)
	rendc = Ids_GrayScale;
    else if (command.options.print_color == dxPrscPrinterColor)
	rendc = Ids_Color;
    ps_itmlst[lstindex++].value = rendc;

    ps_itmlst[lstindex].item_name = (char *)IdsNdisplayHeight;
    ps_itmlst[lstindex++].value = (unsigned long)height;

    ps_itmlst[lstindex].item_name = (char *)IdsNdisplayWidth;
    ps_itmlst[lstindex++].value = (unsigned long)width;

    ps_itmlst[lstindex].item_name = (char *) NULL;
    ps_itmlst[lstindex++].value = 0L;

    Dprintf(("Before CreatePresentSurface (DDIF)\n"),0);
    psid = IdsCreatePresentSurface (ps_itmlst);

    /* 
     * Set up Rendering item list   
     */
    lstindex = 0;
    rn_itmlst[lstindex].item_name = (char *) IdsNprotocol;
    rn_itmlst[lstindex++].value = Ids_Fid;

    /* Terminate the rendering item list */
    rn_itmlst[lstindex].item_name = (char *) NULL;
    rn_itmlst[lstindex++].value = 0;

    Dprintf(("Before CreateRendering (DDIF)\n"),0);
    rendering = IdsCreateRendering (output_image, psid, rn_itmlst);

    Dprintf(("Before DeleteFrame (DDIF)\n"),0);
    ImgDeleteFrame (output_image);

    Dprintf(("Before OpenFile (DDIF)\n"),0);
    context = ImgOpenFile
    (
	(unsigned long)ImgK_ModeExport,
	(unsigned long)ImgK_FtypeDDIF,
	(unsigned long)strlen(command.print_dest),
	(char *)command.print_dest,
	NULL,
	0L
    );

    Dprintf(("Before ExportFrame (DDIF)\n"),0);
    ImgExportFrame (rendering->rndfid, context, 0L);

    Dprintf(("Before CloseFile (DDIF)\n"),0);
    ImgCloseFile (context, 0L);

    Dprintf(("Before DeletePresentSurface (DDIF)\n"),0);
    IdsDeletePresentSurface (psid);

    Dprintf(("Before DeleteRendering (DDIF)\n"),0);
    IdsDeleteRendering (rendering);

    Dprintf(("After DeleteRendering (DDIF)\n"),DbgIslMeter);
    Dprintf(("End of DDIF rendering operation\n"),DbgMsgOnly);

    /* If we're still here, everything must be ok :-) */

    return (Normal);
}
                                                                
/* 
** Create Sixel Output
*/
int creatsx
#if _PRDW_PROTO_
(
    dxPrscOptions	*options,
    int			height,
    int			width,
    FILE		*file,
    unsigned long	image_id,
    unsigned int	converted,
    Display		*dpy,
    int			dfs
)
#else
(options, height, width, file, image_id, converted, dpy, dfs)
    dxPrscOptions	*options;
    int			height;
    int			width;
    FILE		*file;
    unsigned long	image_id;
    unsigned int	converted;
    Display		*dpy;
    int			dfs;
#endif
{
    int			count;
    unsigned long int	surface_id;	    /* IDS presentation surface	*/
    IdsItmlst2		item_list_2[17];
    IdsRendering	*rendering;	    /* Rendering of image for device */
    int			lstindex, i, len;
    char		*ptr, *insert_ptr;
    int			devtype;	    /* Device type - LA100	    */
    float		punch1, punch2, grays; /* Grayscale adjustment params */
    float		rotate_angle;	    /* Rotate to landscape mode	    */

    /*
    ** Establish the condition handler here to specifically take
    ** care of image services routines which may signal an error.
    ** If one of the image routines signal an error then this routine
    ** will return with that error status.
    */

    ChfEstablish (img_signal_handler);

    /* Create an item list for Create Presentation Surface		*/

    lstindex = 0;

    /* This has been put in as the UI defaults to "VT or DECTERM" and
     * this value is defined bitonal by image services. Even if the user
     * selects color sixels, the value which by default gets passed
     * to image services is "bitonal" and no color info. is produced.
     * The only case where image services will get correct info. is when
     * the user explicitly selects Lj250, Lj250Lr or Lcg01 as printer device.
     * The UI doesn't check to see what the user has selected and doesn't
     * accordingly change over to the appropriate color default (even if
     * the user selected color sixels i.e. the default remains VT !!!
     * The following hack checks the users selection and passes appropriate
     * template to image services.
     */
    if ((command.options.print_color == dxPrscPrinterColor) &&
                (command.options.sixel_device != Ids_TmpltLj250Lr)
                && (command.options.sixel_device != Ids_TmpltLj250)
                && (command.options.sixel_device != Ids_TmpltLcg01))
        command.options.sixel_device = Ids_TmpltLj250Lr;


    /* Get device template from the user-entered sixel device code
     * Basically, this tells IDS that we want (color or b&w) sixels 
     */    
    item_list_2[lstindex].item_name = (char *) IdsNtemplate;
    item_list_2[lstindex++].value = 
			(unsigned long int)(command.options.sixel_device);

    /* Override default size values with screen dimensions */

    item_list_2[lstindex].item_name = (char *) IdsNdisplayWidth;
    item_list_2[lstindex++].value = (unsigned long int) UPSPAGEWID;

    item_list_2[lstindex].item_name = (char *) IdsNdisplayHeight;
    item_list_2[lstindex++].value = (unsigned long int) UPSPAGELEN;

    item_list_2[lstindex].item_name = NULL;	/* Terminate itemlist */
    item_list_2[lstindex++].value = 0;


    Dprintf(("Before CreatePresentSurface (Sixel)\n"),0);
    surface_id  = IdsCreatePresentSurface (item_list_2);

    /* 
    ** Set up Rendering item list   
    */
    lstindex = 0;
    /* Rotate and scale the image if user requested it */

    if ((command.options.orient == dxPrscLandscape) ||
	((command.options.orient == dxPrscBestFit) && (width > height)))
    {
	/*
	** Rotate the image for landscape mode.
	** Update local height and width to reflect the change
	** You MUST use  to cast rotate angle or it's
	** type will be wrong, or else it won't be a float 
	*/
	item_list_2[lstindex].item_name = (char *)IdsNrotateMode;
	item_list_2[lstindex++].value = Ids_Rotate;
	rotate_angle = 90.0;
	item_list_2[lstindex].item_name = (char *) IdsNrotateAngle;
	item_list_2[lstindex++].value = TYPE_U_(rotate_angle);

	i = height;
	height = width;
	width  = i;
    }
    else
    {
	item_list_2[lstindex].item_name = (char *) IdsNrotateMode;
	item_list_2[lstindex++].value = Ids_NoRotate;
    }

    if (command.options.fit == dxPrscCrop)
    {
	/* User didn't request any scaling */

	item_list_2[lstindex].item_name = (char *) IdsNscaleMode;
	item_list_2[lstindex++].value = Ids_NoScale;
    }
    else if (command.options.fit == dxPrscReduce)
    {
	if (command.options.sixel_device == Ids_TmpltLj250)
	{
	    width = width / 2;
	    height = height / 2;
	}
	/*
	** If either the height or width of the image will overflow
	** the page, scale it down to fit.  Otherwise, leave it
	** the original size.
	*/
	if ((width > UPSPAGEWID) || (height > UPSPAGELEN))
	{
	    item_list_2[lstindex].item_name = (char *) IdsNscaleMode;
	    item_list_2[lstindex++].value = Ids_FitWithin;
	}
	else
	{
	    item_list_2[lstindex].item_name = (char *) IdsNscaleMode;
	    item_list_2[lstindex++].value = Ids_NoScale;
	}
    } 
    else if (command.options.fit == dxPrscScale)
    {
	if (command.options.sixel_device == Ids_TmpltLj250)
	{
	    width = width / 2;
	    height = height / 2;
	}
	if ((width > UPSPAGEWID) || (height > UPSPAGELEN))
	{
	    item_list_2[lstindex].item_name = (char *) IdsNscaleMode;
	    item_list_2[lstindex++].value = Ids_FitWithin;
	}
	else
	{
	    float   h_scale, w_scale;
	    unsigned long int *scale;

	    w_scale = (((float) UPSPAGEWID) / ((float) width));
	    h_scale = (((float) UPSPAGELEN) / ((float) height));

	    item_list_2[lstindex].item_name = (char *) IdsNscaleMode;
	    item_list_2[lstindex++].value = Ids_Scale;

	    /*
	    ** to fool IDS - give it a longword item
	    ** value with the same bits as float for the scale.
	    */
	    if (w_scale < h_scale)
		scale = (unsigned long int *) &w_scale;
	    else
		scale = (unsigned long int *) &h_scale;

	    item_list_2[lstindex].item_name = (char *) IdsNxScale;
	    item_list_2[lstindex++].value = *scale;
	    item_list_2[lstindex].item_name = (char *) IdsNyScale;
	    item_list_2[lstindex++].value = *scale;
	}
    }
    else if (command.options.fit == dxPrscGrow ||
	command.options.fit == dxPrscShrink)
    {
	unsigned long int	*scale;
	float			f_scale = 2.0;

	if (command.options.fit == dxPrscGrow)
	    f_scale = 2.0;
	else
	    f_scale = 0.5;

	item_list_2[lstindex].item_name = (char *) IdsNscaleMode;
	item_list_2[lstindex++].value = Ids_Scale;

	/*
	** to fool IDS - give it a longword item
	** value with the same bits as float 2.0
	*/
	scale = (unsigned long int *) &f_scale;

	item_list_2[lstindex].item_name = (char *) IdsNxScale;
	item_list_2[lstindex++].value = *scale;
	item_list_2[lstindex].item_name = (char *) IdsNyScale;
	item_list_2[lstindex++].value = *scale;

    }

    /*
    ** Bluenoise algorithm is most CPU intensive, but gives
    ** the best results.  Dither dispered is faster and sometime looks better
    ** with syntehtic graphics such as most printscreens are, requantize does
    ** thresholding.
    */
    item_list_2[lstindex].item_name = (char *) IdsNditherAlgorithm;

    if (command.options.print_color == dxPrscPrinterBW)
	item_list_2[lstindex++].value = Ids_Requantize;
    else
	item_list_2[lstindex++].value = Ids_Dispersed;

    /* Terminate the rendering item list    */

    item_list_2[lstindex].item_name = NULL;
    item_list_2[lstindex++].value = 0;

    /* This creates the sixel data; does all dithering, etc too! */

    Dprintf(("Before CreateRendering (Sixel)\n"),0);
    rendering = IdsCreateRendering (image_id, surface_id, item_list_2);

    Dprintf(("Before DeleteFrame (Sixel)\n"),0);
    ImgDeleteFrame (image_id);

    /* Get the size and address of the rendered sixel data   */

    count = rendering->type_spec_data.sixel.bytcnt;
    ptr = rendering->type_spec_data.sixel.bufptr;

    /*
    ** dp: The following code conforming to the comment below
    **     was putting its own generic header instead of leaving
    **     the image services header on the sixel data produced
    **     by image services.
    **
    **     Put the code in a "if 0", to leave the image services
    **     header on. This code is being removed as image services
    **     puts a header with correct device specific aspect ratios
    **     (IT SHOULD !!) and the generic header may not be correct
    **     for all devices.
    */
#if 0
    /* 
    ** Skip past IDS's "[ESC]P{formatting info}q" sixel header, 
    ** so we can insert our own, more generic, "[ESC]P9q"
    **
    ** The sixel header always begins with a "[ESC]P" string.
    ** The formatting information is represented by one or more
    ** Device Control String parameters separated by semicolons ";".
    ** The final character of the sixel header is always "q".
    **
    ** The following code could have just searched for the 
    ** first occurrence of "q" and reference the sixel data 
    ** from there.
    */
    if (command.options.print_color != dxPrscPrinterColor) 
    {
	insert_ptr = (char *)strchr( ptr, '\033');	    
	insert_ptr = (char *)strchr( insert_ptr, 'P');
	insert_ptr = (char *)strchr( insert_ptr, 'q');
	insert_ptr++;
        /*
        ** Deduct from "count" the number of bytes we skipped
	** in IDS's sixel header "[ESC]P{formatting info}q"
        */
	count -= insert_ptr - ptr;
	ptr = insert_ptr;
	fprintf( file, "\033P9q");
    }
#endif

    /* Now we write the sixels to an output file */
    
    for (i = 0; i < count; i += 500, ptr += 500)
    {
	len = (count - i > 500) ? (500) : (count - i);  
	fwrite( ptr, len, 1, file);

/*#ifdef vms*/
/* dp:
 * Line breaks are needed on Unix too for editors like vi to be able
 * to read this file. Need to read this (sixel file) in order to
 * check that the data is not garbage.
 */
    	    fprintf(file, "\n"); /* don't let the records get too big */
/*#endif*/
    }

    Dprintf(("Before DeletePresentSurface (Sixel)\n"),0);
    IdsDeletePresentSurface (surface_id);

    Dprintf(("Before DeleteRendering (Sixel)\n"),0);
    IdsDeleteRendering (rendering);

    Dprintf(("After DeleteRendering (Sixel)\n"),DbgIslMeter);
    Dprintf(("End of Sixel rendering operation\n"),DbgMsgOnly);

    return (Normal);	
}

int creatps
#if _PRDW_PROTO_
(
    dxPrscOptions	*options,
    int			height,
    int			width,
    FILE		*file,
    unsigned long	image_id,
    unsigned int	converted,
    Display		*dpy,
    int			dfs
)
#else
(options, height, width, file, image_id, converted, dpy, dfs)
    dxPrscOptions	*options;
    int			height;
    int			width;
    FILE		*file;
    unsigned long	image_id;
    unsigned int	converted;
    Display		*dpy;
    int			dfs;
#endif
/*
**	creatps( options, height, width, file, image_id )
**		options - (RO) (*dxPrscOptions) options structure, only
**			reverse_images is referenced
**		height  - (RO) (int) screen y-dimension, in pixels
**		width   - (RO) (int) screen x-dimension, in pixels
**		file 	- (WO) (*FILE) opened output file
**	        image_id - (RO) (int) Image Services frame ident
**
**	requires:
**
** 	restrictions:
**		
**	other:
**		no  implicit parameters, etc.
**
*/
{
	struct tm		*timest;
        IdsItmlst2		ps_itmlst[10];
	IdsRendering		*rendering; /* Rendering of image for device */
	unsigned long		rendc;
        char			*image_data;
	bool			toobig;	/* true = image is too big for page */
	bool			rotate;	/* true = image should be rotated */
	long			foo;	/* force one_half to be aligned */
	float			one_half = .5;	/* the constant .5 (32-bit) */
	unsigned long int 	*punch;	/* punch value			*/
	unsigned long int	output_frame;	/* ISL frame ident	*/
	int			DPI;
	int			xsca, ysca;	/* x and y scale factors */
	int			xpos, ypos;	/* coords of image origin */
	int			status;		/* return status */
	int			lstindex;
	time_t	 		time_val;
	int			i;
	static	char 		*month[12]=
	{
	    "JAN", "FEB", "MAR",
	    "APR", "MAY", "JUN",
	    "JUL", "AUG", "SEP",
	    "OCT", "NOV", "DEC"
	};

/*
 * Find amounts to scale, translate and rotate image for best
 * fit on 8.5 x 11 piece of paper
 * These numbers are also used to calculate the bounding box
 * for the header comments.
 */
    DPI = DPIof (ScreenOfDisplay(dpy, dfs));  /* Get resolution of display device */
    toobig = False;

    xpos = 0;
    ypos = 0;

    if ((command.options.orient == dxPrscLandscape) ||
	((command.options.orient == dxPrscBestFit) && (width > height)))
    {
	rotate = True;
    }
    else
    {
	rotate = False;
    }

    switch (command.options.fit)
    {
	case dxPrscCrop:
	  /* Leave it alone */
	  xsca = width * 72/DPI;
	  ysca = height * 72/DPI;
	  xpos = ypos = 0;
	  break; 
	case dxPrscReduce:
	case dxPrscScale:
	  {
	    int target_width, target_height;
	    enum
	    {
	        scaleNone,
	        scaleWidth,
	        scaleHeight
	    } scale_type = scaleNone;

	    xsca = width;
	    ysca = height;

	    if (rotate)
	    {
	        target_width = UPSPAGELEN;
	        target_height = UPSPAGEWID;
	    }
	    else
	    {
	        target_width = UPSPAGEWID;
	        target_height = UPSPAGELEN;
	    }

	    if ((width <= target_width) && (height <= target_height) &&
	        (command.options.fit == dxPrscReduce))
	    {
	        scale_type = scaleNone;
	    }
	    else
	    {
	        if (((width * target_height) / height) > target_width)
	        {
		    scale_type = scaleWidth;
	        }
	        else
	        {
		    scale_type = scaleHeight;
	        }
	    }
	    if (scale_type == scaleHeight)
	    {
	        xsca *= target_height;
	        xsca /= ysca;
	        ysca = target_height;
	    }
	    else if (scale_type == scaleWidth)
	    {
	        ysca *= target_width;
	        ysca /= xsca;
	        xsca = target_width;
	    }
	    else
	    {
	        /* Convert to Centipoints, then scale */
	        xsca *= 72;
	        xsca /= DPI;
	        ysca *= 72;
	        ysca /= DPI;
	    }

	    ypos = (target_height - ysca) / 2;
	    xpos = (target_width - xsca) / 2;
	  } 
	  break;
	case dxPrscGrow:
	  /* Scale by 2 */
	  xsca = width * 2 * 72 / DPI;
	  ysca = height * 2 * 72 / DPI;
	  xpos = ypos = 0;
	  break;
	case dxPrscShrink:
	  /* Scale by .5 */
	  xsca = width * 72 / (DPI * 2);
	  ysca = height * 72 / (DPI * 2);
	  xpos = ypos = 0;
	  break;
    }	/* end switch */

    if (rotate)
    {
	int i;

	i = xsca;
	xsca = ysca;
	ysca = i;
	i = xpos;
	xpos = ypos;
	ypos = i;
    }
    /*
    ** Header as described in Encapulated Postscript File specification
    ** Header info includes: Bounding box of image (in standard PS coord system)
    ** Creator, creation date, and file name.
    */
    time(&time_val);
    timest = localtime(&time_val);

    fprintf (file, "%%!PS-Adobe-2.0 EPSF-1.2\n");
    fprintf
    (
	file,
	"%%%%Title: %s\n",
	(command.print_dest[0] == 0) ? 	"stdout" : command.print_dest
    );
    fprintf (file, "%%%%Creator: DECW$PRINTSCREEN ");
#ifdef VMS
    fprintf (file, "Version Date = %s", __DATE__);
#endif
    fprintf
    (
	file,
	"\n%%%%CreationDate: %d-%s-19%d  \n",
	timest->tm_mday,
	month[timest->tm_mon],
	timest->tm_year
    );
    fprintf (file, "%%%%Pages: 1 \n");
    fprintf (file, "%%%%BoundingBox:");
    fprintf (file, "%d %d %d %d\n", xpos, ypos, xpos + xsca, ypos + ysca);

    fprintf(file, "%%%%EndComments \n");
    fprintf(file, "%%%%EndProlog \n");
    fprintf(file, "%%%%Page: 1 1 \n");

    /*
    ** Save our state in a temporary.
    */
    fprintf (file, "15 dict begin\n");
    fprintf (file, "/temp-save save def\n");

    /*
    ** Write out scaling and rotating comands
    */
    if (rotate)
    {
	fprintf (file,"%d %d translate\n", xsca, ypos);
	fprintf (file, "90 rotate\n");
	fprintf (file, "%d %d scale\n", ysca, xsca);
    }
    else
    {
	fprintf (file,"%d %d translate\n", xpos, ypos);
	fprintf (file, "%d %d scale\n", xsca, ysca);
    }

    /*
    ** Scale for the entire $I frame size.  Then
    ** re-define "scale" so that any scale done
    ** by ISL will be a no-operation.  This assumes
    ** that ISL generated code will use the unit
    ** square technique...
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
    ** Redefine "scale" so that any scale done by ISL will 
    ** be a no-operation.  Also redefine "showpage" to be a no-op.
    */
    fprintf (file, "/scale { pop pop } bind def\n");
    fprintf (file, "/showpage {} def\n");

    /*
    ** Emit an Encapsulated PS comment so ISL's Header and Trailer
    ** comments can be parsed by others.
    */
    fprintf(file, "%%%%BeginDocument: Image Services\n");

    /*
    ** Establish the condition handler here to specifically take
    ** care of image services routines which may signal an error.
    ** If one of the image routines signal an error then this routine
    ** will return with that error status.
    */
    ChfEstablish (img_signal_handler);

    /* Get basic info about the input image, height, width, number of planes */

    lstindex = 0;

    ps_itmlst[lstindex].item_name = (char *) IdsNtemplate;
    if (command.options.print_color != dxPrscPrinterColor) 

/* It is critical to use the MonoPs and ColorPS templates here. 
 * They cause IDS to generate very vanilla PS that can be enclosed
 * in our wrapper. Using any other templates (Lps20 for example) generates
 * much more PS and it cannot be enclosed in out wrapper
 * There is an image services bug/gotcha here as well. MonoPs
 * allows bitonal AND greyscale images to be processed. You would think
 */

	ps_itmlst[lstindex++].value = Ids_TmpltMonoPs; /* B&W */
    else
	ps_itmlst[lstindex++].value = Ids_TmpltColorPs; /* color */

    ps_itmlst[lstindex].item_name = (char *) IdsNrenderingClass;
    if (command.options.print_color == dxPrscPrinterBW) 
	rendc = Ids_Bitonal;
    else if (command.options.print_color == dxPrscPrinterGrey)
	rendc = Ids_GrayScale;
    else if (command.options.print_color == dxPrscPrinterColor)
	rendc = Ids_Color;
    ps_itmlst[lstindex++].value = rendc;

    ps_itmlst[lstindex].item_name = (char *) IdsNdisplayWidth;
    ps_itmlst[lstindex++].value = (unsigned long int) width;

    ps_itmlst[lstindex].item_name = (char *) IdsNdisplayHeight;
    ps_itmlst[lstindex++].value = (unsigned long int) height;

    ps_itmlst[lstindex].item_name = (char *) NULL;
    ps_itmlst[lstindex++].value = 0;

    Dprintf(("Before CreatePresentSurface (PS)\n"),0);
    output_frame = IdsCreatePresentSurface (ps_itmlst);

    /* 
    ** Set up Rendering item list   
    */
    lstindex = 0;

    /* Rotation and scaling done w/ native PS, not DECimage  */

    ps_itmlst[lstindex].item_name = (char *) IdsNrotateMode;
    ps_itmlst[lstindex++].value = Ids_NoRotate;

    ps_itmlst[lstindex].item_name = (char *) IdsNscaleMode;
    ps_itmlst[lstindex++].value = Ids_NoScale;


    /* Bluenoise algorithm is most CPU intensive, but gives
    ** the best results.  Dither dispered is faster and sometime looks better
    ** with syntehtic graphics such as most printscreens are, requantize does
    ** thresholding.
    */
    if (command.options.print_color != dxPrscPrinterColor)
        ps_itmlst[lstindex].item_name = (char *) IdsNditherAlgorithm;

    if (command.options.print_color == dxPrscPrinterBW)
	ps_itmlst[lstindex++].value = Ids_Requantize;

    if (command.options.print_color == dxPrscPrinterGrey)
        ps_itmlst[lstindex++].value = Ids_Dispersed;

    if (command.options.print_color == dxPrscPrinterBW)
	/* Ensure that black and white is produced from continuous tone image */
    {
	/*
	** to fool IDS - give it a longword item
	** value with the same bits as float .5
	*/
	punch = (unsigned long int *) &one_half;

	ps_itmlst[lstindex].item_name = (char *) IdsNpunch1;

	/* < .5 = black */
	ps_itmlst[lstindex++].value = *punch;

	ps_itmlst[lstindex].item_name = (char *) IdsNpunch2;

	/* > .5 = wh */
	ps_itmlst[lstindex++].value = *punch;
    }

    /* Terminate the rendering item list    */

    ps_itmlst[lstindex].item_name = NULL;
    ps_itmlst[lstindex++].value = 0;

    /* This creates the ps data; does all dithering, etc too! */

    Dprintf(("Before CreateRendering (PS)\n"),0);
    rendering = IdsCreateRendering (image_id, output_frame, ps_itmlst);

    Dprintf(("Before DeleteFrame\n"),0);
    ImgDeleteFrame (image_id);

    status = prsc_emit_image_line
    (
	(unsigned char *)rendering->type_spec_data.postscript.bufptr,
	rendering->type_spec_data.postscript.bytcnt,
	file
    );

/* see if this is the garbage problem with color PS

    fwrite
    (
	rendering->RND$R_TYPE_SPEC_DATA.RND$R_POSTSCRIPT.RND$A_PSC_BUFPTR,
	rendering->RND$R_TYPE_SPEC_DATA.RND$R_POSTSCRIPT.RND$L_PSC_BYTCNT,
	1,
	file
     );

*/
    /*
    ** Encapsulated comments for end of document
    */
    fprintf(file, "\n%%%%EndDocument\n");

    /*
    ** Restore our state from the temporary.
    */
    fprintf(file, "temp-save restore\n");

    /*
    ** Wrap up the document
    */
    fprintf(file, "end\n");
    fprintf(file, "showpage\n");
    fprintf(file, "%%%%Trailer \n");

    /*
    ** Announce our end... (DEC-specific comment)
    */
    fprintf(file, "%%End-of-file\n");

    Dprintf(("Before DeletePresentSurface (PS)\n"),0);
    IdsDeletePresentSurface(output_frame);

    Dprintf(("Before DeleteRendering (PS)\n"),0);
    IdsDeleteRendering( rendering);

    Dprintf(("After DeleteRendering (PS)\n"),DbgIslMeter);
    Dprintf(("End of PostScript rendering operation\n"),DbgMsgOnly);

    return( SS$_NORMAL );

}   /* end of creatps	*/

int prsc_emit_image_line
#if _PRDW_PROTO_
(
    unsigned char	*buf_ptr,
    int			buf_len,
    FILE		*file
)
#else
(buf_ptr, buf_len, file)
    unsigned char	*buf_ptr;
    int			buf_len;
    FILE		*file;
#endif
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
*/
{
unsigned char		*img_ptr1;	/* pointers to image content	    */
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

        return Normal;

} /* end prsc_emit_image_line */
