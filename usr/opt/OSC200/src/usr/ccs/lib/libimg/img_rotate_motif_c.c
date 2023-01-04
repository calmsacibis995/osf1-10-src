/************************************************************************
**
**  Copyright (c) Digital Equipment Corporation, 1991
**  All Rights Reserved.  Unpublished rights reserved
**  under the copyright laws of the United States.
**  
**  The software contained on this media is proprietary
**  to and embodies the confidential technology of 
**  Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and
**  media is authorized only pursuant to a valid written
**  license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
**  disclosure by the U.S. Government is subject to
**  restrictions as set forth in Subparagraph (c)(1)(ii)
**  of DFARS 252.227-7013, or in FAR 52.227-19, as
**  applicable.
**
************************************************************************/

/************************************************************************
**  img_rotate_motif_c.c
**
**  FACILITY:
**
**	DECimage Application Services, Image Services Library
**
**  ABSTRACT:
**
**	This program runs under Motif and demonstrates the use of the
**	ImgRotateFrame command.  It will rotate an image.
**
**	define:	$ rotate := $sys$disk:[]img_rotate_motif_c.exe
**	usage: 	$ rotate [-n] [-r] input_image_file_name [ouput_image_file_name]
**
**  ENVIRONMENT:
**
**	VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHOR(S):
**
**	Ray Giroux
**
**  CREATION DATE:
**
**	April, 1991
**
************************************************************************/

/*
**  Include files:
*/

#include <stdio.h>
#include <stdlib.h>
#if defined(__VMS) || defined(VMS)
#include <MrmAppl.h>
#else
#include <Mrm/MrmAppl.h>
#endif

#include <img/IdsImage.h>
#include <img/ImgDef.h>
#include <img/ImgEntry.h>

/*
**  Table of contents:
*/

static unsigned long GetFid ();
static void HelpCb ();
static unsigned long PutFid ();
static void ViewCb ();

/*
**  Literals
*/

#define INPUT_IMG_NAME	"Rotate Input Image"	/* input image name */
#define OUTPUT_IMG_NAME	"Rotate Output Image"	/* output image name */
#define IMG_CLASS_NAME	"Rotate Image"		/* image class name */
#define INPUT_IMG_X	0			/* input image X pos */
#define INPUT_IMG_Y	0			/* input image Y pos */
#define IMAGE_WIDTH	512			/* image width in pixels */
#define IMAGE_HEIGHT	512			/* image height in pixels */
 
/*
**  Local Storage
*/
 
/*
**	Callback routine structures allow us to register the callback
**	routines with the widget.
*/

static XtCallbackRec help_callback[] = {{(XmVoidProc)HelpCb, NULL}, NULL};
static XtCallbackRec view_callback[] = {{(XmVoidProc)ViewCb, NULL}, NULL};

/******************************************************************************
**  main
**
**  FUNCTIONAL DESCRIPTION:
**
**	Main entry point for the sample program.  It is responsible for:
**
**	o Parsing the command line
**	o Getting the ISL frame id of the input image
**	o Scaling the input image for output
**	o Saving the output image, if requested
**	o Creating the DECwindows top level application and shell
**	  widgets used to display the images
**	o Creating the image widgets using IDS
**	o Managing the image and realizing the application widgets
**	  to display the images
**
**  FORMAL PARAMETERS:
**
**	argc - argument count
**	argv - argument vector
**		-a - rotation angle
**		-r - reverse preference
**		-n - use nearest neighbor algorithm
**		input_image_file
**		output_image_file
**
******************************************************************************/

main (argc, argv)
int argc;
char *argv[];
{
    char *input_filename = NULL; 	/* input image filename */
    char *output_filename = NULL;	/* output image filename */
    long input_fid;			/* ISL frame id of input image */
    long output_fid;			/* ISL frame id of output image */
    XtAppContext appctx;		/* application context */
    Widget input_widget;		/* the input widget */
    Widget output_widget;		/* the output widget */
    Widget input_img_widget;		/* the IDS input image widget */
    Widget output_img_widget;		/* the IDS output image widget */
    float angle = 0;			/* rotation angle */
    unsigned long rotate_flags = 0;	/* ISL rotate flags */
    char buffer[512];			/* general purpose character buffer */
    Cardinal targc;			/* temporary argument count */
    char *targv;			/* temporary argument vector */
    Arg al[10];				/* argument list */
    int ac;				/* argument count */
    int i;
    int error_flag = FALSE;		/* error flag; assume no usage error */


    /*
    ** Parse the command line.
    */

    for (i = 1; i < argc; i++)
    {
	if (strcmp (argv[i], "-n") == 0)	/* nearest neighbor */
	    rotate_flags |= ImgM_NearestNeighbor;
	else if (strcmp (argv[i], "-r") == 0)	/* reverse preference */
	    rotate_flags |= ImgM_ReversePreference;
	else if (strcmp (argv[i], "-a") == 0)	/* rotation angle */
	    angle = (float) atof (argv[++i]);
	else if (*argv[i] == '-')		/* command line error */
	{
	    error_flag = TRUE;
	    break;
	}
	else
	{
	    input_filename = argv[i];		/* input image file name */
	    if (i + 1 < argc)
		output_filename = argv[i+1];	/* output image filename */
	    break;
	}
    }

    /*
    ** It's an error not to have an image file or a rotation angle.
    */

    if (input_filename == NULL || angle == 0)
	error_flag = TRUE;

    /*
    ** A usage error occured; report the correct usage.
    */

    if (error_flag)
    {
	printf (
	    "usage:  rotate -a angle [-n] [-r] \
input_image_filename [output_image_filename]\n", 
	    argv[0]);
	exit (1);
    }

    /*
    ** Get the ISL input fid.
    */

    input_fid = GetFid (input_filename);

    /*
    ** Rotate the input frame.
    */

    output_fid = ImgRotateFrame (
			input_fid,		/* frame identifier to rotate */
			&angle,			/* rotation angle */
			rotate_flags);		/* flags */
    /*
    ** Save output image to the output file, if requested.
    */

    if (output_filename != NULL)
    {
	unsigned long byte_count;		/* number of bytes saved */

	byte_count = PutFid (output_fid, output_filename);
    }

    /*
    ** Create the DECwindows input and output widgets.
    ** The output widget is a child of the input widget.
    */

    targc = 0;
    targv = NULL;
    input_widget = XtAppInitialize (
		    &appctx, INPUT_IMG_NAME, NULL, 0, &targc, &targv,
		    NULL, NULL, (Cardinal) 0);
    output_widget = XtAppCreateShell (
		    OUTPUT_IMG_NAME, IMG_CLASS_NAME, topLevelShellWidgetClass,
		    XtDisplay (input_widget), NULL, 0);

    /*
    ** Create the IDS input and output image widgets; pan them so the
    ** user is able to see the entire image.
    ** These are children of the DECwindows input and output widgets.
    */

    input_img_widget = IdsXmPannedImage (
			input_widget,			/* parent */
			"",				/* name */
			0, 0,				/* x and y pos */
			IMAGE_WIDTH, IMAGE_HEIGHT,	/* width and height */
			input_fid,			/* input frame id */
			NULL,				/* render callback */
			view_callback,			/* view callback */
			NULL,				/* drag callback */
			help_callback);			/* help callback */
    output_img_widget = IdsXmPannedImage (
			output_widget,			/* parent */
			"",				/* name */
			0, 0,				/* x and y pos */
			IMAGE_WIDTH, IMAGE_HEIGHT,	/* width and height */
			output_fid,			/* output frame id */
			NULL,				/* render callback */
			view_callback,			/* view callback */
			NULL,				/* drag callback */
			help_callback);			/* help callback */


    /*
    ** Set the position and title of the input and output widgets.
    ** The output goes to the right of the input.
    */

    ac = 0;
    SETARG_ (al, ac, XmNx, INPUT_IMG_X);
    SETARG_ (al, ac, XmNy, INPUT_IMG_Y);
    sprintf (buffer, "%s:  %s\n", INPUT_IMG_NAME, input_filename);
    SETARG_ (al, ac, XmNtitle, buffer);
    SETARG_ (al, ac, XmNiconName, INPUT_IMG_NAME);
    XtSetValues (input_widget, al, ac);

    ac = 0;
    SETARG_ (al, ac, XmNx, INPUT_IMG_X + IMAGE_WIDTH);
    SETARG_ (al, ac, XmNy, INPUT_IMG_Y);
    if (output_filename != NULL)
	sprintf (buffer, "%s:  %s\n", OUTPUT_IMG_NAME, output_filename);
    else
	strcpy (buffer, OUTPUT_IMG_NAME);
    SETARG_ (al, ac, XmNtitle, buffer);
    SETARG_ (al, ac, XmNiconName, OUTPUT_IMG_NAME);
    XtSetValues (output_widget, al, ac);

    /*
    ** Manage the images and realize the applications.
    */

    XtManageChild (input_img_widget);
    XtManageChild (output_img_widget);
    XtRealizeWidget (input_widget);
    XtRealizeWidget (output_widget);

    /*
    ** Call the toolkit to loop forever, processing the application events.
    */

    XtAppMainLoop (appctx);
}

/*****************************************************************************
**  GetFid
**
**  FUNCTIONAL DESCRIPTION:
**
**	Examine all the environment variables and set up the proper
**	variables for monitoring the file open and the display of the
**	image
**
**	
**  FORMAL PARAMETERS:
**
**	filename - string with the ddif image file to open
**
**
**  FUNCTION VALUE:
**
**      ISL Frame Identifer of fid of ddif file opened
**
**
**  Set the logicals (on VMS) or environment variable (Unix)
**
**	NOSTANDARDIZE	    undefined	Frames are standardized automatically
**			    0		"	"	"	"
**			    not zero	Frames are not standardized
**
**	ACCESS_TYPE	    undefined	Cda is used
**			    1		Cda is used
**			    2		Rms open, Qio read is used
**			    3		Rms open, Rms BIO read is used
**			    others	Cda is used.
**
**	IO_BUF_SIZE	    undefined	127 blocks by default
**			    defined	size in blocks.  Any size is valid
**					for QIO; 127 max is used by RMS (which
**					means that values larger are clipped
**					to 127).
**
**					NOTE: this is ignored if access type
**					is Cda.
**
**	IO_ASYNCH	    undefined	asynchronous I/O for RMS and BIO
**			    0		"	    "	"   "	"   "
**			    not zero	synchronous I/O for RMS and QIO
**
**	IO_STATS	    undefined	DO NOT SHOW I/O Stats
**			    0		"	    "	"   "	"   "
**			    not zero	Show I/O stats
**
**
*****************************************************************************/

static unsigned long GetFid (filename)
char *filename;
{
    char *access_type_str;
    char *io_stats_str;
    char *io_asynch_flag_str;
    char *io_buf_size_str;
    char *nostandardize_flag_str;
    long access_type = ImgK_Cda;
    long env_access_type;
    long env_io_stats;
    long env_io_asynch_flag;
    long env_nostandardize_flag;
    long io_buf_size = 127;
    unsigned long ctx;
    unsigned long fid;
    unsigned long file_open_flags = 0;
    unsigned long import_flags = 0;
    unsigned long io_stats_flag	= FALSE;
    struct ITMLST itmlst[3];


    /*
    ** Get environment variables (logicals or symbols) for run-time
    ** tuning of the import I/O.
    */

    access_type_str = getenv ("ACCESS_TYPE");
    if (access_type_str != NULL)
    {
	env_access_type = atoi (access_type_str);
	switch (env_access_type)
	{
	    case 1:
		access_type = ImgK_Cda;
		break;
	    case 2:
		access_type = ImgK_Qio;
		break;
	    case 3:
		access_type = ImgK_RmsBio;
		break;
	    default:
		access_type = ImgK_Qio;
	}
    }

    io_buf_size_str = getenv ("IO_BUF_SIZE");
    if (io_buf_size_str != NULL)
	io_buf_size = atoi (io_buf_size_str);
    else
	io_buf_size = 64;

    io_asynch_flag_str = getenv ("IO_ASYNCH");
    if (io_asynch_flag_str != NULL)
    {
	env_io_asynch_flag = atoi (io_asynch_flag_str);
	if (env_io_asynch_flag != 0)
	    file_open_flags = 0;
	else
	    file_open_flags = ImgM_Asynchronous;
    }

    io_stats_str = getenv ("IO_STATS");
    if (io_stats_str != NULL)
    {
        env_io_stats = atoi (io_stats_str);
        if (env_io_asynch_flag != 0)
	    io_stats_flag = TRUE;
        else
	    io_stats_flag = FALSE;
    }

    nostandardize_flag_str = getenv ("NOSTANDARDIZE");
    if (nostandardize_flag_str != NULL)
    {
        env_nostandardize_flag = atoi (nostandardize_flag_str);
        if (env_nostandardize_flag != 0)
	    import_flags = ImgM_NoStandardize;
        else
	    import_flags = 0;
    }

    /*
    ** Set up the ITMLST struct for I/O variables based on the 
    ** environment variable values.
    */

    itmlst[0].ItmL_Code		= Img_AccessType;
    itmlst[0].ItmL_Length	= sizeof (access_type);
    itmlst[0].ItmA_Buffer	= (char *) &access_type;
    itmlst[0].ItmA_Retlen	= 0;
    itmlst[0].ItmL_Index	= 0;

    itmlst[1].ItmL_Code		= Img_IoBufSize;
    itmlst[1].ItmL_Length	= sizeof (io_buf_size);
    itmlst[1].ItmA_Buffer	= (char *) &io_buf_size;
    itmlst[1].ItmA_Retlen	= 0;
    itmlst[1].ItmL_Index	= 0;

    itmlst[2].ItmL_Code		= 0;
    itmlst[2].ItmL_Length	= 0;
    itmlst[2].ItmA_Buffer	= (char *) 0;
    itmlst[2].ItmA_Retlen	= 0;
    itmlst[2].ItmL_Index	= 0;

    /*
    ** Open the file and import the frame.
    */

    ctx = ImgOpenFile (
		ImgK_ModeImport,			/* file open mode */
		ImgK_FtypeDDIF,				/* file type */
		strlen (filename),			/* file name length */
		filename,				/* file name */
		itmlst,					/* item list */
		file_open_flags);			/* file flags */

    fid = ImgImportFrame (ctx, import_flags);

    /*
    ** Close the image file.
    */

    ImgCloseFile (ctx, 0);

    /*
    ** Return the frame id.
    */

    return (fid);
}

/*****************************************************************************
**  HelpCb
**
**  FUNCTIONAL DESCRIPTION:
**
**	The image widget calls this routine when help has been requested. This
**	routine prints a helpful message on the user's terminal.
**
**  FORMAL PARAMETERS:
**
**	iw  - image widget identifier
**	tag - user data that was supplied in the callback list structure
**	      when this routine was registered
**	cbs - address of help callback structure
**
*****************************************************************************/

static void HelpCb (iw, tag, cbs)
Widget iw;
caddr_t tag;
XmAnyCallbackStruct *cbs;
{
    printf("rotate -a angle [-n] [-r] \
input-image-filename [output-image-filename]\n");
    printf("\t\"Click-and-drag\" to pan.\n");
    printf("\t\"CONTROL C\" to exit.\n");
}

/*****************************************************************************
**  PutFid
**
**  FUNCTIONAL DESCRIPTION:
**
**	Examine all the environment variables and set up the proper
**	variables for monitoring the file open and the saving of the
**	image
**
**	
**  FORMAL PARAMETERS:
**
**	fid	 - ISL Frame Identifer of image to be saved
**	filename - string with the ddif image file to save to
**
**
**  FUNCTION VALUE:
**
**      The number of bytes successfully written to the ddif file
**
**
**  Set the logicals (on VMS) or environment variable (Unix)
**
**	PAGEBREAK	    undefined	Frames are not preceded by a page break
**			    0		"	"	"	"
**			    not zero	Precede frame by a page break
**
**	SOFTPAGEBREAK	    undefined	Default page breaks are hard
**			    0		"	"	"	"
**			    not zero	Page breaks are soft
**
**	ACCESS_TYPE	    undefined	Cda is used
**			    1		Cda is used
**			    2		Rms open, Qio read is used
**			    3		Rms open, Rms BIO read is used
**			    others	Cda is used.
**
**	IO_BUF_SIZE	    undefined	127 blocks by default
**			    defined	size in blocks.  Any size is valid
**					for QIO; 127 max is used by RMS (which
**					means that values larger are clipped
**					to 127).
**
**					NOTE: this is ignored if access type
**					is Cda.
**
**	IO_ASYNCH	    undefined	asynchronous I/O for RMS and BIO
**			    0		"	    "	"   "	"   "
**			    not zero	synchronous I/O for RMS and QIO
**
**	IO_STATS	    undefined	DO NOT SHOW I/O Stats
**			    0		"	    "	"   "	"   "
**			    not zero	Show I/O stats
**
**
*****************************************************************************/

static unsigned long PutFid (fid, filename)
unsigned long fid;
char *filename;
{
    char *access_type_str;
    char *io_stats_str;
    char *io_asynch_flag_str;
    char *io_buf_size_str;
    char *pagebreak_flag_str;
    char *softpagebreak_flag_str;
    long access_type = ImgK_Cda;
    long env_access_type;
    long env_io_stats;
    long env_io_asynch_flag;
    long env_pagebreak_flag;
    long env_softpagebreak_flag;
    long io_buf_size = 127;
    unsigned long byte_count;
    unsigned long ctx;
    unsigned long file_open_flags = 0;
    unsigned long export_flags = 0;
    unsigned long io_stats_flag	= FALSE;
    struct ITMLST itmlst[3];


    /*
    ** Get environment variables (logicals or symbols) for run-time
    ** tuning of the export I/O.
    */

    access_type_str = getenv ("ACCESS_TYPE");
    if (access_type_str != NULL)
    {
	env_access_type = atoi (access_type_str);
	switch (env_access_type)
	{
	    case 1:
		access_type = ImgK_Cda;
		break;
	    case 2:
		access_type = ImgK_Qio;
		break;
	    case 3:
		access_type = ImgK_RmsBio;
		break;
	    default:
		access_type = ImgK_Qio;
	}
    }

    io_buf_size_str = getenv ("IO_BUF_SIZE");
    if (io_buf_size_str != NULL)
	io_buf_size = atoi (io_buf_size_str);
    else
	io_buf_size = 64;

    io_asynch_flag_str = getenv ("IO_ASYNCH");
    if (io_asynch_flag_str != NULL)
    {
	env_io_asynch_flag = atoi (io_asynch_flag_str);
	if (env_io_asynch_flag != 0)
	    file_open_flags = 0;
	else
	    file_open_flags = ImgM_Asynchronous;
    }

    io_stats_str = getenv ("IO_STATS");
    if (io_stats_str != NULL)
    {
        env_io_stats = atoi (io_stats_str);
        if (env_io_asynch_flag != 0)
	    io_stats_flag = TRUE;
        else
	    io_stats_flag = FALSE;
    }

    pagebreak_flag_str = getenv ("PAGEBREAK");
    if (pagebreak_flag_str != NULL)
    {
        env_pagebreak_flag = atoi (pagebreak_flag_str);
        if (env_pagebreak_flag != 0)
	{
	    export_flags = ImgM_PageBreak;

	    softpagebreak_flag_str = getenv ("SOFTPAGEBREAK");
	    if (softpagebreak_flag_str != NULL)
	    {
		env_softpagebreak_flag = atoi (softpagebreak_flag_str);
		if (env_softpagebreak_flag != 0)
		    export_flags |= ImgM_SoftPageBreak;
	    }
	}
        else
	    export_flags = 0;
    }

    /*
    ** Set up the ITMLST struct for I/O variables based on the 
    ** environment variable values.
    */

    itmlst[0].ItmL_Code		= Img_AccessType;
    itmlst[0].ItmL_Length	= sizeof (access_type);
    itmlst[0].ItmA_Buffer	= (char *) &access_type;
    itmlst[0].ItmA_Retlen	= 0;
    itmlst[0].ItmL_Index	= 0;

    itmlst[1].ItmL_Code		= Img_IoBufSize;
    itmlst[1].ItmL_Length	= sizeof (io_buf_size);
    itmlst[1].ItmA_Buffer	= (char *) &io_buf_size;
    itmlst[1].ItmA_Retlen	= 0;
    itmlst[1].ItmL_Index	= 0;

    itmlst[2].ItmL_Code		= 0;
    itmlst[2].ItmL_Length	= 0;
    itmlst[2].ItmA_Buffer	= (char *) 0;
    itmlst[2].ItmA_Retlen	= 0;
    itmlst[2].ItmL_Index	= 0;

    /*
    ** Open the file and export the frame.
    */

    ctx = ImgOpenFile (
		ImgK_ModeExport,			/* file open mode */
		ImgK_FtypeDDIF,				/* file type */
		strlen (filename),			/* file name length */
		filename,				/* file name */
		itmlst,					/* item list */
		file_open_flags);			/* file flags */

    byte_count = ImgExportFrame (fid, ctx, export_flags);

    /*
    ** Close the image file.
    */

    ImgCloseFile (ctx, 0);

    /*
    ** Return the number of bytes saved.
    */

    return (byte_count);
}

/*****************************************************************************
**  ViewCb
**
**  FUNCTIONAL DESCRIPTION:
**
**	The image widget calls this function when the portion of the
**	image being viewed in the window has changed. This function
**	prints the updated image view coordinates.
**
**  FORMAL PARAMETERS:
**
**	iw  - image widget identifier
**	tag - user data that was supplied in the callback list structure
**	      when this routine was registered
**	cbs - address of view callback structure
**
*****************************************************************************/

static void ViewCb (iw, tag, cbs)
Widget iw;
caddr_t tag;
IdsViewCallback cbs;
{
    /*
    ** Nothing interesting to do here.
    */
}
