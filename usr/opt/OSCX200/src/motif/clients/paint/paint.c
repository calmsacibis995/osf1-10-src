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
static char *BuildSystemHeader= "$Header: /usr/sde/osf1/rcs/x11/src/motif/clients/paint/paint.c,v 1.1.4.2 1993/06/25 22:46:24 Ronald_Hegli Exp $";
#endif		/* BuildSystemHeader */
/*
****************************************************************************
**                                                                          *
**  Copyright (c) 1987                                                      *
**  By DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                        *
**  All rights reserved
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
**   DECpaint - VMS DECwindows paint program
**
**  AUTHOR
**
**   Daniel Latham, October 1987
**
**  ABSTRACT:
**
**   This program allowes the creation of bitmaps through a paint-style
**   interface.  
**
**  ENVIRONMENT:
**
**   User mode, executable image.
**
**  MODIFICATION HISTORY:
**
**      dl      10/5/88
**      set picture_changed variable
**
**      dl      10/6/88
**      change the default fill pattern to solid foreground
**
**      dl      10/11/88
**      call Change_Picture_Size always
**
**      dl      10/12/88
**      set flag when called from EPIC to read/not read a file
**
**      dl      10/25/88
**      take first argument from a command line as the file
**
**--       
**/           
#include "paintdefs.h"
#include "paint_icon.h"
#define PAINT_ICON XCreatePixmapFromBitmapData (XtDisplay(toplevel), \
    XDefaultRootWindow (XtDisplay(toplevel)), \
    paint_icon_bits, \
    paint_icon_width, \
    paint_icon_height, \
    BlackPixel (XtDisplay(toplevel), DefaultScreen(XtDisplay(toplevel))), \
    WhitePixel (XtDisplay(toplevel), DefaultScreen(XtDisplay(toplevel))), \
    1)
/*  Motif needs depth 1 for icon
    DefaultDepth (XtDisplay(toplevel), DefaultScreen(XtDisplay(toplevel))))
*/

#define WINDOW_ICON XCreatePixmapFromBitmapData (XtDisplay(toplevel), \
    XDefaultRootWindow (XtDisplay(toplevel)), \
    window_icon_bits, \
    window_icon_width, \
    window_icon_height, \
    BlackPixel (XtDisplay(toplevel), DefaultScreen(XtDisplay(toplevel))), \
    WhitePixel (XtDisplay(toplevel), DefaultScreen(XtDisplay(toplevel))), \
    1)
/*  Motif needs depth 1 for icon
    DefaultDepth (XtDisplay(toplevel), DefaultScreen(XtDisplay(toplevel))))
*/

/* treg -> */

#if defined(VMS) && !defined(__DECC)
#pragma nostandard
#endif
externalref (ptDrawingAreaWidgetClass);
#if defined(VMS) && !defined(__DECC)
#pragma standard
#endif

extern Widget WindowCreate();

/* <- treg */

#ifdef EPIC_CALLABLE
int AppAILError();
int AppInitializeSession();
int AppExecuteSession();
int AppTerminateSession();
int AppTerminateAppl();
int AppSetAttributes();
int AppSetInputFile();
unsigned long ul_zero; /* jj-port (unsigned long zero) */

#define NUMBER_ACTIONS 7
static AiAction action_list[] =
{   /* Common actions */
    { AI_AIL_ERROR,		AppAILError,		0 },
    { AI_SET_ATTRIBUTES,	AppSetAttributes,	0 },

    /* Actions to deal with the parent application */    
    { AI_INITIALIZE_SESSION,	AppInitializeSession,	0 },
    { AI_EXECUTE_SESSION,	AppExecuteSession,	0 },
    { AI_TERMINATE_SESSION,	AppTerminateSession,	0 },
    { AI_TERMINATE_APPL,	AppTerminateAppl,	0 },
    { AI_SET_INPUT_FILE,	AppSetInputFile,	0 }
};

#define CENTIPOINTS_PER_INCH 7200

#define CENTIPOINTS_TO_PIXELS(points, res)\
    ((int)(((float)((points) * (res)) / (float)CENTIPOINTS_PER_INCH) + 0.5))
#define PIXELS_TO_CENTIPOINTS(pixels, res)\
    (((pixels) * CENTIPOINTS_PER_INCH) / (res))

static int pic_width;
static int pic_height;
static int read_file;		    /* dl-10/12/88 */
static int no_respond_count = 0;    /* dl-10/12/88 */
static paint_window_mapped = FALSE;
#endif

static Position mx = 10;
static Position my = 10;
static Dimension mwd = 600;
static Dimension mht = 600;
static Dimension mmwd = 200;
static Dimension mmht = 200;

/*
 *
 * ROUTINE:  Set_Default_Stipple
 *
 * ABSTRACT: 
 *
 * Sets up default fill/outline stipple patterns
 *
 */                
void Set_Default_Stipple()
{
/* solid patterns */
static char solidfg_bits[] = {
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

static char solidbg_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


	solid_fg_stipple = XCreatePixmapFromBitmapData( disp, btmap,
	 solidfg_bits, 16, 16, 1, 0, 1 );

	solid_bg_stipple = XCreatePixmapFromBitmapData( disp, btmap,
	 solidbg_bits, 16, 16, 1, 0, 1 );

	outline_stipple = solid_fg_stipple;
	fill_stipple = solid_bg_stipple; /* jj - 3/27/89 */
/*	fill_stipple = solid_fg_stipple; *//* dl - 10/6/88 */

			
}



/*
 *
 * ROUTINE:  Set_Screen_Resolution
 *
 * ABSTRACT:
 *
 *  Find the resolution of the display.  If within error tolerance of a standard
 *  resolution then round off to that resolution.
 */
void Set_Screen_Resolution()
{
#define PIX_ROUNDOFF 0.5
    float res;
    float r1,r2;

    r1 = XDisplayWidth (disp, screen);
    r2 = XDisplayWidthMM (disp, screen);
    res = (r1 * MM_PER_INCH ) / r2;
/*    res = (XDisplayWidth (disp, screen) * MM_PER_INCH ) /
	  XDisplayWidthMM (disp, screen); */
    if (fabs (DPI75 - res) < (DPI75 * RES_ERR_TOLERANCE))
	screen_resolution = DPI75;
    else
	if (fabs (DPI100 - res) < (DPI100 * RES_ERR_TOLERANCE))
	    screen_resolution = DPI100;
	else
	    screen_resolution = res + PIX_ROUNDOFF;
}

  
/*
 *
 * ROUTINE:  Initialize_Globals
 *
 * ABSTRACT: 
 *
 *  Initialize necessary global variables
 *
 */
void Initialize_Globals()
{              
    int i, j;
    int nVis;
    static int usableClasses[3] = {PseudoColor, GrayScale, StaticGray};
    XVisualInfo vInfo_dummy;

    if (startup_initialize) {
	disp = XtDisplay (toplevel);
	screen = DefaultScreen (disp);	
/*
        vInfo_dummy.visualid = (XDefaultVisual (disp, screen))->visualid;
        visual_info = XGetVisualInfo (disp, VisualIDMask, &vInfo_dummy, &nVis);
*/
	visual_info = 0;
	vInfo_dummy.screen = screen;
	for (i = 0; i < 3; i++) {	
	    vInfo_dummy.class = usableClasses[i];
	    visual_info = XGetVisualInfo (disp,
					  VisualClassMask | VisualScreenMask,
					  &vInfo_dummy, &nVis);
	    if (visual_info) {
		break;
	    }				
	}	    	

	if (!visual_info) {
	}

	switch (visual_info->class) {
	    case StaticGray :
		file_color = SAVE_BW;
		break;
	    case GrayScale :	
		file_color = SAVE_GRAY;
		break;
	    default :
		file_color = SAVE_COLOR;
		break;
	}

/* determine if running on an 8 plane GPX/VsII - this is a hack */
/* Only on GPX/vSII are black and white pixel == 252 and 253 */
	if ((BlackPixel (disp, screen) == 252) &&
	    (WhitePixel (disp, screen) == 253)) {
	    gpx = TRUE;
	}
	else {
	    gpx = FALSE;
	}
	gpx_reduction = (gpx ? 2 : 0);
	colormap_size = visual_info->colormap_size - gpx_reduction;
	
	screen_wd = XDisplayWidth (disp, screen);
	screen_ht = XDisplayHeight (disp, screen);

	position_ht = (screen_ht < MIN_HT_FOR_BIG_ICONS) ? 36 : 72;

	Set_Screen_Resolution ();
	printer_page_wd = US_LETTER_WIDTH;
	printer_page_ht = US_LETTER_HEIGHT;
	printer_resolution = DPI300;
    }

    resolution = screen_resolution;
    pimage_wd = resolution * 8.0; 
    pimage_ht = resolution * 10.0;
    pic_xorigin = 0;
    pic_yorigin = 0;
    picture_x = 0;
    picture_y = 0;
    pixmap_changed = FALSE;		/* dl - 10/5/88 */
    picture_changed = FALSE;
    refresh_zoom_from = FROM_PICTURE;
    refresh_picture_pending = TRUE;
    window_exposure = FALSE;
    paint_view = NORMAL_VIEW;
    exiting_paint = FALSE;
	
    undo_x = 0;
    undo_y = 0;
    undo_width = 0;
    undo_height = 0;
    undo_picture_x = picture_x;
    undo_picture_y = picture_y;

    hi_points = 0;
    orig_hi_points = 0;
    prv_hi_points = 0;
    num_hipts = 0;
    orig_num_hipts = 0;
    prv_num_hipts = 0;

    if (startup_initialize) {
	UG_wd = 64;
        UG_ht = 64;

	UG_last = ((screen_wd / UG_wd) + (((screen_wd % UG_wd) == 0) ? 0 : 1)) *
		  ((screen_ht / UG_ht) + (((screen_ht % UG_ht) == 0) ? 0 : 1));

	UG_num = UG_last + 5;
	UG_image = (XImage **) XtCalloc (UG_num, sizeof(XImage *));
	UG_used = (int *) XtCalloc (UG_num, sizeof(int));
    }

    if (!startup_initialize)
    {
	if (copymap_image != NULL)
	{
#if 0
	    XtFree (copymap_image->data);
#endif
	    XDestroyImage (copymap_image);
	}
	copymap_image = NULL;
    }

    if (startup_initialize) {
	Init_Dialog_Boxes ();


/* Initialize line width buffer */
	j = 1;
	for (i = 0; i < NUM_LINE_SIZES; ++i) {
	    line_width[i] = j;
	    j += 2;
	}

	cur_line_wd = line_width[1];

/* Initialize default brush */
	cur_brush = SQUARE_BRUSH;
	cur_brush_index = 1;
	brushes[cur_brush][cur_brush_index].wd = 8;

/* Initialize state variables */
	button_down_mode = TRUE;
	current_action = BRUSH;     
	rbanding = FALSE;     
	use_brush = FALSE;
	select_on = FALSE;
	sp_select_on = FALSE;
	highlight_on = FALSE;
	first_move = FALSE;
	opaque_fill = TRUE;
	cur_fill_style = FillOpaqueStippled;
	crop_rectangle = FALSE;
	grid_on = FALSE;
	grid_size = DEFAULT_GRID_SIZE;
	zoom_pixsize = 8;
	zoom_pixborder = 1;
	pindex = -1;
	entering_text = FALSE;
	file_format = DDIF_FORMAT;
	event_mask = ButtonPressMask | ButtonReleaseMask | 
			Button1MotionMask | ExposureMask | 
 			KeyPressMask | KeyReleaseMask;
#ifdef I18N_MULTIBYTE
/* Asain requires at most MAXFONTS */
	for (i = 0; i < MAXFONTS; i++)
	    cur_font[i] = NULL;
#else
	cur_font = NULL;
#endif /* I18N_MULTIBYTE */
        pdepth = visual_info->depth;
	screen_depth = XDefaultDepth (disp, screen);
    }

    default_colormap = XDefaultColormap (disp, screen);

    if (visual_info->visual != XDefaultVisual (disp, screen)) {
	paint_colormap = XCreateColormap (disp, XDefaultRootWindow (disp),
					  visual_info->visual, AllocAll);
	for (i = 0; i < visual_info->colormap_size; i++) {
	    colormap[i].pixel = i;
	}
    }
    else {
	paint_colormap = default_colormap;
    }

/* Main width and height shouldn't be larger than the screen */
    if (main_wd > screen_wd)
	main_wd = screen_ht;
    if (main_ht > screen_wd)
	main_ht = screen_ht;

    if (startup_initialize) {
	Build_Main (toplevel);

	Get_Attribute (main_widget, XmNforeground, &window_fg);
	Get_Attribute (main_widget, XmNbackground, &window_bg);
    }		

    Init_Color_Table ();
    Init_Font_Attributes ();

    if (startup_initialize) {
	if (pdepth == 1) {
	    img_format = XYPixmap;
            picture_fg = 1;
            picture_bg = 0;
            bit_plane = 1;
	    image_stype = ImgK_StypeBitonal; 
	}
        else {
	    img_format = ZPixmap;
            picture_fg = colormap[paint_color].pixel;
            picture_bg = colormap[paint_bg_color].pixel;
            bit_plane = AllPlanes;
	    image_stype = ImgK_StypeMultispect;
/*	    image_stype = ImgK_StypeGreyscale; */
	}

/* if pdepth = 0, picture_bg_byte = 00000000 = 0 = picture_bg */
/* if pdepth = 4, picture_bg_byte = 0000XXXX = picture_bg */
/* if pdepth = 8, picture_bg_byte = XXXXXXXX = picture_bg */	
	picture_bg_byte = picture_bg;
    }

/*  main_x = 100; */
/*  main_y = 100; */
/*  window_fg = BlackPixel (disp, screen); */
/*  window_bg = WhitePixel (disp, screen); */

    if (startup_initialize) {
	main_min_wd = MIN (main_min_wd, main_wd);
	main_min_wd = MAX (main_min_wd, mmwd);
	main_min_ht = MIN (main_min_ht, main_ht);
	main_min_ht = MAX (main_min_ht, mmht);

/* initialize points buffer */
	points = (XPoint *) XtMalloc(sizeof(XPoint)*ALLOC_NUM); /* jj-port */
	numpts = 0;
	num_alloc = ALLOC_NUM;

/* drawable's for GC creation */
	btmap = XCreatePixmap( disp, DefaultRootWindow(disp), 1, 1, 1 );
	pxmap = XCreatePixmap( disp, DefaultRootWindow(disp), 1, 1, pdepth);

/* initialize GC array */
	for (i = 0; i < NUMBER_OF_GCS; ++i)
	    GCs[i] = NULL;

/* setup mandatory stipple patterns */
	Set_Default_Stipple();
    }

    startup_initialize = FALSE;
}                                               


/* JJ-MOTIF ->

Set_Focus(w)
Widget	w;
{
Time CT;
CT = CurrentTime;

  *                
  *	Set focus to the widget, have to make sure the widget is visible
  *	before we set focus to it. Otherwise ---> ERROR.
  *
       
  XSync(disp, 0);
  if (XtIsRealized (w))
	(*w->core.widget_class->core_class.accept_focus)(w, &CT);
}

<- JJ-MOTIF */

Create_Pixmaps (bg, wd, ht)
    long bg;
    int  wd, ht;
{
    picture = Create_Pdepth_Pixmap (bg, wd, ht);
    if (picture) {
/* change undo parameters */
        UG_cols = (wd / UG_wd);
        UG_xwd = wd % UG_wd;
        if (UG_xwd != 0)
            UG_cols += 1;
        UG_rows = (ht / UG_ht);
        UG_xht = ht % UG_ht;
        if (UG_xht != 0)
            UG_rows += 1;
        UG_num_used = 0;
        undo_available = TRUE;
    }
    else {
        exiting_paint = TRUE;
        Display_Message ("T_EXIT_NO_SERVER_MEMORY");
    }
}


/*
 *
 * ROUTINE:  main
 *
 * ABSTRACT: 
 *
 *  main entry point - when compiling for ULTRIX, paint is bound
 * with other applications, so the entry point is not main but
 * paint_main.
 *
 */
#ifdef COMBINE
int paint_main(argc, argv)
#else
int main(argc, argv)
#endif
    int argc;
    char **argv;              
{
    Dimension wd, ht;
    int i, j;
    Arg args[7];
    int argcnt;
    int bits_per_pixel, bits_per_line, extra_bits, image_bytes;
    char *image_data;
    extern int Error_Handler();

    static XtAppContext app_context;

/*
**  Storage for the resources to get
*/

typedef struct {
	Position x, y;
	Dimension width, height;
	Pixel foreground, background;
	Pixel pointer_fg, pointer_bg;
	Dimension min_wd, min_ht;
} ResDatarec, *ResData ;

/*
**  Resource list
*/
static XtResource resources[] = {
    {XtNx, XtCX, XtRShort, sizeof(Position),
        XtOffset (ResData, x), XtRShort, (caddr_t) &mx},
    {XtNy, XtCY, XtRShort, sizeof(Position),
        XtOffset (ResData, y), XtRShort, (caddr_t) &my},
    {XtNwidth, XtCWidth, XtRShort, sizeof(Dimension),
        XtOffset (ResData, width), XtRShort, (caddr_t) &mwd},
    {XtNheight, XtCHeight, XtRShort, sizeof(Dimension),
        XtOffset (ResData, height), XtRShort, (caddr_t) &mht},
    {XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
        XtOffset (ResData, foreground), XtRString, "Black" },
    {XtNbackground, XtCBackground, XtRPixel, sizeof(Pixel),
        XtOffset (ResData, background), XtRString, "White" },
    {"pointer_foreground", "Pointer_foreground", XtRPixel, sizeof(Pixel),
	XtOffset (ResData, pointer_fg), XtRString, "Black" },
    {"pointer_background", "Pointer_background", XtRPixel, sizeof(Pixel),
	XtOffset (ResData, pointer_bg), XtRString, "White" },
    {"min_width", "Min_width", XtRShort, sizeof(Dimension),
	XtOffset (ResData, min_wd), XtRShort, (caddr_t) &mmwd},
    {"min_height", "Min_height", XtRShort, sizeof(Dimension),
	XtOffset (ResData, min_ht), XtRShort, (caddr_t) &mmht}
};

ResDatarec resdata;

#ifdef EPIC_CALLABLE
char *my_name;
int my_name_len;
AiEnvirDECWTK envir_info;
int num_action;
#endif

#ifdef R5_XLIB
/*  XtSetLanguageProc() should be called before XtAppInitialize() */
    XtSetLanguageProc(NULL, NULL, NULL);
#endif /* R5_XLIB */

    MrmInitialize ();
    DXmInitialize ();

/* treg -> */
    if (MrmRegisterClass (MrmwcUnknown, "Different_visual_window",
			  "WindowCreate", WindowCreate, (WidgetClass)ptDrawingAreaWidgetClass)
	!= MrmSUCCESS) {
	DRM_Error ("can't register DRM class.");
    }
/* <- treg */

/* Initalize toolkit and allow children to resize */
#if !defined(VMS)
    toplevel = XtInitialize(
    		NULL, 
    		"DXpaint", 
    		NULL, 
    		0, 
    		&argc, 
    		argv );
/*
    		NULL, 
    		NULL, 
    		0);
*/

#else
    toplevel = XtInitialize(
    		NULL, 
    		"DECW$PAINT", 
    		NULL, 
    		0, 
    		&argc, 
    		argv );
/*
    		NULL, 
    		NULL, 
    		0);
*/
#endif


/* XtSetLanguageProc set's a locale for the toolkit in Motif 1.2 or later */

/*
#ifdef R5_XLIB
    XtSetLanguageProc(app_context, NULL, app_context);
#endif
*/



    argcnt = 0;
/* not needed for motif - and causes trouble
    XtSetArg( args[argcnt], XtNallowShellResize, TRUE );
    ++argcnt;
*/
/*
    XtSetArg( args[argcnt], XtNiconName, T_DECPAINT );
    ++argcnt;
*/
    XtSetArg( args[argcnt], XtNiconPixmap, PAINT_ICON );
    ++argcnt;

/* JJ-MOTIF -> 
   XtSetArg( args[argcnt], XtNiconifyPixmap, WINDOW_ICON );
    ++argcnt;
<- JJ-MOTIF */

/*
    XtSetArg( args[argcnt], XtNtitle, T_DECPAINT );
    ++argcnt;
*/

    XtSetValues( toplevel, args, argcnt );

    run_as_child = FALSE;
#ifdef EPIC_CALLABLE
/*
 * Check to see whether this application is invoked by 
 * another application and process accordingly.
 */
    my_name = "DECPaint V1.0";
    my_name_len = strlen(my_name);
    envir_info.type = AI_ENVIR_DECWTK;
    envir_info.widget = (struct _WidgetRec *) toplevel; /* jj-port */
    session = AiStartup (my_name, &my_name_len, &envir_info, &argc, argv, 
			 &parent_hnd );

    session_active = FALSE;
    if( session ) {	/* Make sure Applications Interface is started OK */

	/* Check to see whether to run as a child application */
	if((parent_hnd != (AiAppl)0) && (parent_hnd != (AiAppl)1)) /* jj-port */
	    run_as_child = TRUE;

	/* Set up and declare action routines. */
	num_action = NUMBER_ACTIONS;
	AiSetActions( &session, &ul_zero, action_list, &num_action ); /* jj-port */
    }

    widget_realized = FALSE;
#endif


/* Check to see if file has been specified on the command line -dl 10/25/88 */
/* assume first argument is the file spec */
    cfile[0] = '\0';
    cur_file = &cfile[0];
    cname [0] = '\0';
    cur_name = &cname[0];
    tfile[0] = '\0';
    temp_file = &tfile[0];
    lfile[0] = '\0';
    last_file_name = &lfile[0];
    ifile[0] = '\0';
    include_file_name = &ifile[0];
    if( argc > 1 ) {
/*	strcpy( cur_file, argv[1] ); */
        strcpy( temp_file, argv[1] );
    } 

    startup_initialize = TRUE;

/* Set from resource file  */
    XtGetApplicationResources(toplevel, &resdata, resources,
                              XtNumber(resources),NULL, 0);
    main_x = resdata.x;
    main_y = resdata.y;
    main_wd = resdata.width;
    main_ht = resdata.height;
    window_fg = resdata.foreground;
    window_bg = resdata.background;	
    cursor_fg.pixel = resdata.pointer_fg;
    cursor_bg.pixel = resdata.pointer_bg;
    main_min_wd = resdata.min_wd;
    main_min_ht = resdata.min_ht;

    Initialize_Globals();

/* create the GC for rubberbanding */
    gc_rband = Get_GC( GC_RUBBERBAND );
    gc_highlight = gc_rband;

/* Create the main window for the widget */
/*  Build_Main(toplevel); */

/* Set up error handler */
    XSetErrorHandler( Error_Handler );

/*
    XSetInputFocus( disp, pwindow, RevertToParent, CurrentTime);
    Set_Focus( main_widget );
    XSelectInput( disp, XtWindow(XtParent(toplevel)),StructureNotifyMask );
*/

    Display_Main();

/* Display the UI now if not run as a child application */
    if( !run_as_child ) {
/* Create the pixmaps that contains the picture */
	Create_Pixmaps (picture_bg, picture_wd, picture_ht);
	if (picture) {
/* jj-11/16/88 - create the picture image */	    
/*
	    picture_image = XGetImage (disp, picture, 0, 0, picture_wd,
				       picture_ht, bit_plane, img_format);
	    if (picture_image->bitmap_bit_order != NATIVE_BIT_ORDER)
		ConvertImageNative (picture_image);                  
*/
	    bits_per_pixel = (pdepth == 1) ? 1 : 8;
	    bits_per_line = pimage_wd * bits_per_pixel;
	    if (extra_bits = bits_per_line % 32)
		bits_per_line += 32 - extra_bits;
	    image_bytes = (bits_per_line / 8) * pimage_ht;
	    image_data = (char *) XtCalloc (image_bytes, sizeof (char));

            if (image_data == 0) {
		exiting_paint = TRUE;
		Display_Message ("T_EXIT_NO_CLIENT_MEMORY");
            }
            else {
/* might only be good under VMS *???* */
		if (picture_bg_byte != 0) {
		    memset (image_data, picture_bg_byte, image_bytes);
		}

		picture_image = 
		    XCreateImage (disp, visual_info->visual,
				  (bits_per_pixel == 1) ? 1 : pdepth,
				  img_format, 0, image_data, 
				  pimage_wd, pimage_ht, 32, 0);
		picture_image->bitmap_bit_order = NATIVE_BIT_ORDER;    /*ram*/
		picture_image->byte_order = NATIVE_BYTE_ORDER;         /*ram*/
	    }
	}

	if (!exiting_paint) {
	    if( strlen( temp_file ))  { 
		Extract_Filename ();  /* Set titlebar in case read fails. */
		Set_Titlebar ();
		Read_File();
	    }
	    if (!exiting_paint) {
		Refresh_Picture (pic_xorigin, pic_yorigin, pwindow_wd,
				 pwindow_ht);
	    }
	}
    }

#if !defined(HYPERHELP)
#define HYPERHELP 1
#endif

#if HYPERHELP
    if (!hyperhelp_context)
    {
	extern void help_error();
        DXmHelpSystemOpen
	    (&hyperhelp_context, toplevel, PAINT_HELP, help_error, NoPaintHelp);
    }
#endif

/* Enter main event loop, never return. */
/*    XtAppMainLoop(app_context); */
    XtMainLoop();
}
          
/*
 *
 * ROUTINE:  Exit_Paint
 *
 * ABSTRACT: 
 *
 *  terminate paint
 *
 */
void Exit_Paint( status )
    int status;
{
    if (entering_text)
	End_Text ();
    if (select_on)
	DeSelect(FALSE);
    if (zoomed)
	End_Zoom();

    if (picture)
	XFreePixmap (disp, picture);
/* dl - 10/11/88 reset pixmap variables */
    picture = 0;

#ifdef EPIC_CALLABLE
    /* Check whether this is in the middle of terminating a session */
    if (run_as_child) {
	if (session_active)
	    /* Tell parent that we want to close the current session. */
	    AiSessionExit (&parent_hnd, 0, 0);

	if (terminate_hnd) 
	    AiActionAck (&parent_hnd, &terminate_hnd, AI_SUCCESS);

	AppUnmapUI();
	return;
    }
#endif

    XtDestroyWidget( toplevel );
    exit(status);
}

#ifdef EPIC_CALLABLE
/*
 * Action and callback routines 
 */

/*
 *
 * ROUTINE:  AppUnmapUI
 *
 * ABSTRACT: 
 *
 *	Make user interface disappear.
 *
 */
int AppUnmapUI()
{
    Display *dpy;
    Window win;

    /*
     * Make the window/UI invisible
     */
    dpy = XtDisplay( toplevel );
    win = XtWindow( toplevel );
    XUnmapWindow( dpy, win );
    paint_window_mapped = FALSE;
    XFlush( dpy );
}

/*
 *
 * ROUTINE:  AppAILError
 *
 * ABSTRACT: 
 *
 *	Trap errors detected by AIL
 *
 */
int AppAILError( param, from_appl, error_code, level )
    long	    *param;
    AiAppl	    *from_appl;
    unsigned long   *error_code;
    unsigned long   *level;
{
    switch (*error_code) {
	case AI_ERR_NO_RESPOND :
	    no_respond_count++;
	    if( no_respond_count >= 10 ) {
		no_respond_count = 0;
		if (paint_window_mapped) {
		    Create_AI_Error_Caution_Box (AI_ERR_NO_RESPOND);
		}
		else {
		    Exit_Paint( *error_code );
		}
	    }
	    break;
	case AI_ERR_BROKEN_LINK :
	    if (paint_window_mapped) {
		Create_AI_Error_Caution_Box (AI_ERR_BROKEN_LINK);
	    }
	    else {
		Exit_Paint( *error_code );
	    }
	    break;
    }
}

/*
 *
 * ROUTINE:  AppInitialSession
 *
 * ABSTRACT: 
 *
 *	Call by parent to initialize a new session.
 *
 */
int AppInitializeSession( param, from_appl, action_hnd )
    long	    *param;
    AiAppl	    *from_appl;
    unsigned long   *action_hnd;
{
    no_respond_count = 0;

     /*
     * Do session initialization (if any)
     */
    Initialize_Globals();
    pic_width = pimage_wd;
    pic_height = pimage_ht;

    read_file = FALSE;		/* dl-10/12/88 */
    new_file = TRUE;		/* bc-1/5/89 */
    almost_new_file = FALSE;

    terminate_hnd = 0;		/* for defer acknowledgement */

    return AI_SUCCESS;
}

/*
 *
 * ROUTINE:  AppExecuteSession
 *
 * ABSTRACT: 
 *
 *	Call by parent to start executing a session.
 *
 */
int AppExecuteSession( param, from_appl, action_hnd )
    long	    *param;
    AiAppl	    *from_appl;
    unsigned long   *action_hnd;
{
    int bits_per_pixel, bits_per_line, extra_bits, image_bytes;
    char *image_data;

    no_respond_count = 0;

    /* Create the pixmaps that contains the picture */
/*
    picture = Create_Pdepth_Pixmap( picture_bg, picture_wd, picture_ht );
    undomap = Create_Pdepth_Pixmap( picture_bg, picture_wd, picture_ht );
*/
/* jj-01/06/89 - create the picture image */	    
    bits_per_pixel = (pdepth == 1) ? 1 : 8;
    bits_per_line = pic_width * bits_per_pixel;
    if (extra_bits = bits_per_line % 32)
	bits_per_line += 32 - extra_bits;
    image_bytes = (bits_per_line / 8) * pic_height;
    image_data = (char *) XtCalloc (image_bytes, sizeof (char));

    if (image_data) {
/* might only be good under VMS *???* */
	if (picture_bg_byte != 0) {
	    memset (image_data, picture_bg_byte, image_bytes);
	}

	picture_image = 
	    XCreateImage (disp, visual_info->visual,
			  (bits_per_pixel == 1) ? 1 : pdepth, 
			  img_format, 0, image_data, pic_width,
			  pic_height, 32, 0);
	picture_image->bitmap_bit_order = NATIVE_BIT_ORDER;    /*ram*/
	picture_image->byte_order = NATIVE_BYTE_ORDER;         /*ram*/
    }

/*
    if (!widget_realized) {
	 *
	 *  Realize the toplevel widget.
	 *
	Display_Main();
	widget_realized = TRUE;
    }
    else 
*/
    {
	Display *dpy;
	Window win;

	/*
	 *  Map the window again to make it visible.
	 */
	dpy = XtDisplay( toplevel );
	win = XtWindow( toplevel );
	XMapWindow( dpy, win );
	XFlush( dpy );
    }
    paint_window_mapped = TRUE;

    if (image_data == 0) {
	exiting_paint = TRUE;
	Display_Message ("T_EXIT_NO_CLIENT_MEMORY");
    }

    if (!exiting_paint) {
/* jj - 01/06/89 resize regardless of whether it is realized */
	Change_Picture_Size( pic_width, pic_height );
	if (exiting_paint)
            return AI_SUCCESS;

	if ((strlen (cur_file) > 0) && read_file) {    /* dl-10/12/88 */
	    Read_File();
	    if (exiting_paint)
		return AI_SUCCESS;
	}
	else {
	    Extract_Filename ();
	    Set_Titlebar ();
	}

/* Keep track of width and height in case it changes */
/* jj-01/06/89 use image dimensions */
	pic_width = pimage_wd;
	pic_height = pimage_ht;
    }

    session_active = TRUE;

    return AI_SUCCESS;
}

/*
 *
 * ROUTINE:  AppTerminateSession
 *
 * ABSTRACT: 
 *
 *	Call by parent when it wants to terminate a session.
 *
 */
int AppTerminateSession( param, from_appl, action_hnd, situation )
    long	    *param;
    AiAppl	    *from_appl;
    unsigned long   *action_hnd;
    unsigned long   *situation;
{
    char *name, *format;
    int name_len, format_len;
    int status;

    no_respond_count = 0;

    /*
     * If normal termination, tell parent about revised input file and 
     * final output file.
     */
    if( *situation == AI_EXIT_NORMAL && session_active ) {
	if( strlen( cur_file ) == 0 ) {
	    Create_Write_Dialog();
	    terminate_hnd = *action_hnd;
	    return AI_DEFER_ACKNOWLEDGEMENT;
	} else
	    Write_File();
    }

    session_active = FALSE;

    /*
     * Hide the user interface
     */
    AppUnmapUI();

    return AI_SUCCESS;
}

/*
 *
 * ROUTINE:  AppTerminateAppl
 *
 * ABSTRACT: 
 *
 *	Call by parent when it wants to terminate the application.
 *
 */
int AppTerminateAppl( param, from_appl, action_hnd )
    long	    *param;
    AiAppl	    *from_appl;
    unsigned long   *action_hnd;
{
    int status;

    /*
     * Tell parent all-done before exit.
     */
    AiActionAck( from_appl, action_hnd, AI_SUCCESS );

    XtDestroyWidget( toplevel );
    exit(0);
}

/*
 *
 * ROUTINE:  AppSetAttributes
 *
 * ABSTRACT: 
 *
 *	Call by parent when attributes are to be updated.
 *
 */
int AppSetAttributes( param, from_appl, item, count )
    long	    *param;
    AiAppl	    *from_appl;
    AiItem	    *item;
    int		    *count;
{
    int status = AI_SUCCESS;
    int i;
    int resize = FALSE;
    int width = 0, height = 0;
    int nat_width = 0, nat_height = 0;
    int phys_width = 0, phys_height = 0;

    no_respond_count = 0;

    for( i = *count; i > 0; --i ) {
	switch( item->attr_id ) {
	    case AI_ID_NATURAL_WIDTH:
		nat_width = CENTIPOINTS_TO_PIXELS (item->value, resolution);
		resize = TRUE;
		break;
			
	    case AI_ID_NATURAL_HEIGHT:
		nat_height = CENTIPOINTS_TO_PIXELS (item->value, resolution);
		resize = TRUE;
		break;

	    case AI_ID_PHYSICAL_WIDTH:
		phys_width = CENTIPOINTS_TO_PIXELS (item->value, resolution);
		resize = TRUE;
		break;
			
	    case AI_ID_PHYSICAL_HEIGHT:
		phys_height = CENTIPOINTS_TO_PIXELS (item->value, resolution);
		resize = TRUE;
		break;

	    default:
		status = AI_ERR_DATA_IGNORED;
	}
    item++;
    }

    /*
     * Change picture size if width or height changed
     */


/* jj-01/06/89 use image dimensions */
    if( resize ) {
	if ((nat_width != 0) && (nat_height != 0)) {
	    width = nat_width;
	    height = nat_height;
	}
	else {
	    if ((phys_width != 0) && (phys_height != 0)) {
		width = phys_width;
		height = phys_height;
	    }
	}
    	if( width == 0 ) width = pimage_wd;
    	if( height == 0 ) height = pimage_ht;

/* dl - 10/11/88 resize regardless of whether it is realized 
    Change_Picture_Size( width, height ); 
*/
/*
    	if( widget_realized )
    	    Change_Picture_Size( width, height );
    	else {
    	    picture_wd = width;
    	    picture_ht = height;
    	}
*/
        pic_width = width;
        pic_height = height;
    }

    return status;
}

/*
 *
 * ROUTINE:  AppSetInputFile
 *
 * ABSTRACT: 
 *
 *	Call be parent regarding input file to use.
 *
 */
int AppSetInputFile( param, from_appl, name, name_len, flags )
    long	    *param;
    AiAppl	    *from_appl;
    char	    *name;
    int		    *name_len;
    unsigned	    *flags;
{
    no_respond_count = 0;

    /*
     * Save input file name
     */
    if( *name_len == 0 )
	*cur_file = '\0';
    else 
	strcpy( cur_file, name );

    if( (*flags & AI_FLAG_CREATE) )	/* dl-10/12/88 */
        read_file = FALSE;
    
    else {
        read_file = TRUE;
	strcpy( temp_file, cur_file );
    }

    return AI_SUCCESS;
}

/*
 *
 * ROUTINE:  AppUpdateParent
 *
 * ABSTRACT: 
 *
 *	Tell parent about result and updated data.
 *
 */
AppUpdateParent()
{
    int status;
    char *format;
    int len, format_len;

    /*
     * If the picture size has changed, tell the parent 
     * Or even if it hasn't tell parent width and height
     */
/*
    if ((((pimage_wd * screen_resolution) / resolution) != pic_width) || 
	(((pimage_ht * screen_resolution) / resolution) != pic_height)) {
*/
    	AiItem item_list[2];
    	int item_count;

    	item_count = 2;
    	item_list[0].attr_id = AI_ID_NATURAL_WIDTH;
    	item_list[0].value = PIXELS_TO_CENTIPOINTS (pimage_wd, resolution);
/* (pimage_wd * screen_resolution) / resolution; */
    	item_list[1].attr_id = AI_ID_NATURAL_HEIGHT;
    	item_list[1].value = PIXELS_TO_CENTIPOINTS (pimage_ht, resolution);
/* (pimage_ht * screen_resolution) / resolution; */
    	AiSetAttributes( &parent_hnd, item_list, &item_count, 0, 0 );
    	pic_width = pimage_wd;
    	pic_height = pimage_ht;
/*
    }
*/

    /*
     * Tell parent about the updated image file
     * (The final output and revised input files are the same for DECW$paint)
     */
    len = strlen( cur_file );
    format = "DDIF_IMAGE";
    format_len = strlen( format );
    status = AiSetRevisedInputFile( &parent_hnd, cur_file, &len, 0, 0 );
    status = AiSetFinalOutputFile( &parent_hnd, cur_file, &len, format, 
		    &format_len, &ul_zero, 0, 0 ); /* jj-port */
}
#endif
