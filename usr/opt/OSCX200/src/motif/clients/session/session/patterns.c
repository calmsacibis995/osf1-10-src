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
#include "paintdefs.h"
#include "smconstants.h"
#include "smdata.h"
#include <Xm/PushB.h>

extern	GC  Create_GC();
Pixmap Create_Pixmap();
Pixmap Create_Bitmap_Menu();

static struct {
	Widget dialog;
	Widget fill;
	Widget fg;
	Widget bg;
	Widget anone;
	Widget window;
	Widget dismiss;
	} w_sample;

Pixmap fill_sample;

/* These variables are to keep track of the current pattern indices
that are being used for the fill and outline stipple */
static int fill_index = -1;

extern void Set_Sample();			/* forward dec's */
extern void Set_Sample_Pattern();			/* forward dec's */


void Set_No_Stipple( p )
Pixmap p;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Sets the box which shows the selected pattern to have an X in
**	it.  Which signifies the server default has been selected.
**
**  FORMAL PARAMETERS:
**
**	p - The pixmap for the selected pattern push button.  We will
**	    draw the x into that pixmap.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/
{
GC gc_solid;

gc_solid = Get_GC( GC_SD_SOLID );
gc_values.line_width = 2;
gc_values.foreground = window_fg;
XChangeGC( display_id, gc_solid, GCForeground|GCLineWidth, &gc_values );
XDrawLine( display_id, p, gc_solid, 0, 0, 32, 32 );
XDrawLine( display_id, p, gc_solid, 32, 0, 0, 32 );
gc_values.line_width = 0;
XChangeGC( display_id, gc_solid, GCLineWidth, &gc_values );
}

void Set_Fill_Sample()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Sets the box which shows the selected pattern to have the
**	slected pattern.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/
{
Arg args[10];
int argcnt;

/* clear pixmap- it may have contained the no stipple symbol */
XSetForeground( display_id, Get_GC(GC_SD_SOLID), window_bg );
XFillRectangle( display_id, fill_sample, Get_GC(GC_SD_SOLID), 0, 0, 32, 32 );
if( fill_stipple )
	/* Fill the pixmap with the correct pattern */
	XFillRectangle( display_id, fill_sample, Get_GC(GC_SD_FILL),
		2, 2, 28, 28 );
else
	{   
	/* fill the pixmap with the default X symbol */
	Set_No_Stipple( fill_sample );
	}
argcnt = 0;
XtSetArg(args[argcnt], XmNlabelPixmap, fill_sample); argcnt++;
XtSetArg(args[argcnt], XmNselectPixmap, fill_sample); argcnt++;
XtSetArg(args[argcnt], XmNlabelPixmap, fill_sample); argcnt++;
XtSetValues(w_sample.fill, args, argcnt);
}

void Create_Samples()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Creates the sample box which shows which pattern is currently
**	selected.  Also, the boxes to select solid foreground, solid
**	background, and server default patterns.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/
{
Arg args[10];
int argcnt;
Pixmap fg_pattern, bg_pattern;

/* Creates the pixmap for the button which will show the currently selected
   pattern.  Set the initial pattern, and create the button with the
   pixmap inside the button */
fill_sample = Create_Pixmap( window_bg, 32, 32 );
w_sample.fill = windowsetup.patterncurrent_id;
Set_Fill_Sample();
argcnt = 0;
XtSetArg(args[argcnt], XmNlabelPixmap, fill_sample); argcnt++;
XtSetArg(args[argcnt], XmNselectPixmap, fill_sample); argcnt++;
XtSetArg(args[argcnt], XmNlabelPixmap, fill_sample); argcnt++;
XtSetArg(args[argcnt], XmNforeground, window_fg); argcnt++;
XtSetArg(args[argcnt], XmNbackground, window_bg); argcnt++;
XtSetValues(w_sample.fill, args, argcnt);

/* Create the pixmap and push button for the selection of solid foreground
   pattern.  */
fg_pattern = Create_Pixmap( window_fg, 16, 16 );
w_sample.fg = windowsetup.patternfg_id;
argcnt = 0;
XtSetArg(args[argcnt], XmNlabelPixmap, fg_pattern); argcnt++;
XtSetArg(args[argcnt], XmNselectPixmap, fg_pattern); argcnt++;
XtSetArg(args[argcnt], XmNlabelPixmap, fg_pattern); argcnt++;
XtSetArg(args[argcnt], XmNforeground, window_fg); argcnt++;
XtSetArg(args[argcnt], XmNbackground, window_bg); argcnt++;
XtSetValues(w_sample.fill, args, argcnt);

/* Create the pixmap and push button for the selection of solid background
   pattern.  */
bg_pattern = Create_Pixmap( window_bg, 16, 16 );
w_sample.bg = windowsetup.patternbg_id;
argcnt = 0;
XtSetArg(args[argcnt], XmNlabelPixmap, bg_pattern); argcnt++;
XtSetArg(args[argcnt], XmNselectPixmap, bg_pattern); argcnt++;
XtSetArg(args[argcnt], XmNlabelPixmap, bg_pattern); argcnt++;
XtSetArg(args[argcnt], XmNforeground, window_fg); argcnt++;
XtSetArg(args[argcnt], XmNbackground, window_bg); argcnt++;
XtSetValues(w_sample.fill, args, argcnt);

/* Create the push button for server default */
w_sample.anone = windowsetup.patterndefault_id;
}

void Set_Stipple()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Whenever we click on a pattern, we need to store that patter
**	into a number of GCs that we use.  Also we need to set the
**	box which displays the current pattern to have the selected
**	pattern.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/
{
/* Modify all of the necesarry GCs */
if( fill_stipple ){
	XSetStipple( display_id, Get_GC(GC_PD_FLOOD), fill_stipple );
	XSetStipple( display_id, Get_GC(GC_PD_FILL), fill_stipple );
	XSetStipple( display_id, Get_GC(GC_SD_FILL), fill_stipple );
	}
/* Fill the box which displays the current pattern */
Set_Fill_Sample();
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Handles ButtonDown events in the pattern selection window.
**	The buttons in the pattern selection window are a certain distance
**	apart.  We can determine the pattern selected by looking at
**	the x and y of the button down event.  Then we need to change
**	the box which shows the current pattern to have the correct pattern.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/
void Clicked_On_Pattern( w, event, params, num_params )
	Widget  w;
	XButtonPressedEvent *event;
	char **params;
	int	num_params;

{
int row, col;
int argcnt;
Arg args[3];
unsigned int	pindex;

/* determine which pattern was clicked on */
row = event->y/(pattern_sz+ pattern_space);
col = event->x/(pattern_sz+ pattern_space);

/* change current pattern */
pindex = row + col*(PATTERN_ROWS);
/* account for solid fg, solid bg, and default index */
pindex = pindex + 3;
if (pindex > LAST_PATTERN) return;

/* Get the pattern and soter in fill_stipple */
fill_stipple = patterns[pindex];
/* Store the index into the array */
fill_index = pindex;
windowsetup.pattern_selected = fill_index;
/* Set the box with the default pattern correctly */
Set_Stipple();
/* Get push button pattern to change */
if (XtIsRealized(w_sample.fill))
  XClearArea(XtDisplay(w_sample.fill), XtWindow(w_sample.fill),
	     0, 0, 0, 0, True);
}

void Refresh_Pattern_Window( w, event, params, num_params )
	Widget  w;
	XExposeEvent *event;
	char **params;
	int	num_params;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Called on an expose event to the pattern window.
**	Draws all of the little patterns into the pattern selection
**	window (I think).
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/
{    
XCopyPlane( display_id, pattern_pixmap, XtWindow(w_sample.window), 
	    Get_GC(GC_SD_COPY), event->x, event->y, event->width, 
	    event->height, event->x, event->y, 1 );
}
Create_Patterns()
{
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Creates the pattern selection window.  This window has each
**	type of pattern displayed in a small push button.  The rows
**	and columns are layed out in a certain pattern so that the
**	x and y of the mouse press can be used to index into the
**	pattern array.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/
int i;
int argcnt;
Arg args[6];
static int num_actions = 2;
static XtActionsRec action_table [] =
{
	{"Clicked_On_Pattern", (XtActionProc) Clicked_On_Pattern},
	{"Refresh_Pattern_Window", (XtActionProc) Refresh_Pattern_Window}
};
static char pattern_translation_table[] =
	"<Btn1Down>: Clicked_On_Pattern()\n\
	<Expose>: Refresh_Pattern_Window()";

pattern_sz = 16;
pattern_space = 1;

/* create pattern bitmaps */
for( i = 0; i < NUM_PATTERNS; ++i )
	patterns[i] = XCreatePixmapFromBitmapData( display_id, btmap,
	 pattern_bits[i], 16, 16, 1, 0, 1 );
/* Create the small buttons with the pixmaps in them */
pattern_pixmap = Create_Bitmap_Menu( &patterns[3], PATTERN_ROWS,
		 PATTERN_COLS, pattern_sz, pattern_space, pattern_space );
XtAddActions ( action_table, num_actions );		
w_sample.window = windowsetup.palette_id;
argcnt = 0;
XtSetArg( args[argcnt], XmNwidth, 
          (Dimension)(pattern_sz+pattern_space)*PATTERN_COLS);  ++argcnt;
XtSetArg( args[argcnt], XmNheight, 
          (Dimension)(pattern_sz+pattern_space)*PATTERN_ROWS );  ++argcnt;
XtSetArg( args[argcnt], XmNtranslations,
	  XtParseTranslationTable(pattern_translation_table) );  ++argcnt;
XtSetValues(w_sample.window, args, argcnt);
}
int Create_Pattern_Dialog()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Creates the total pattern dialog box.  This includes the window
**	with all of the small pattern push-buttons as well as the
**	button which displays the currently selected pattern, and the
**	buttons for solid foreground, solid background, and server
**	default
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/
{
unsigned     long   black_p;
unsigned     long   white_p;
unsigned    int	i;

/* Save the pixel values for white and black.  Also initialize the
   globals window_fg and _bg and picture_fg and _bg to be black and
   white.  When the user selects colors for display foreground/background
   we will change these global values and reset GCs so that things are
   displayed in the correct colors */
white_p = (unsigned long)WhitePixel(display_id,screen);
black_p = (unsigned long)BlackPixel(display_id,screen);
window_fg = black_p;
picture_fg = black_p;
window_bg = white_p;
picture_bg = white_p;     

/* drawable's for GC creation */
btmap = XCreatePixmap( display_id, DefaultRootWindow(display_id), 1, 1, 1 );
pxmap = XCreatePixmap( display_id, DefaultRootWindow(display_id), 1, 1, 1);

/* initialize GC array and create gc for rubberbanding */
for( i = 0; i < NUMBER_OF_GCS; ++i)
    GCs[i] = NULL;

/* Initialize the fill_stipple */
fill_stipple = XCreatePixmapFromBitmapData( display_id, btmap,
	       pattern_bits[solid_background], 16, 16, 1, 0, 1 );

/* Create the buttons for solid foreground/background/default, and the
   window which shows the currently selected pattern */
Create_Samples();           

/* Create the pattern menu */
Create_Patterns();

/* Mark that nothing has been selected yet */
windowsetup.pattern_selected = -1;
}
void
Set_Sample_Pattern( w, tag, r )
    Widget w;					/* pulldown menu */
    caddr_t tag;
    XtCallbackList *r;		/* just pick up the boolean */
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Called when the user selects either Solid foreground button,
**	solid background button, or server default button.  It sets
**	the sample window to look correct, and marks the particular
**	button which was selected.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/
{
fill_index = -1;
if( w == w_sample.fg )
	{
	/* solid foreground selected */
	fill_stipple = patterns[solid_foreground];
	windowsetup.pattern_selected = solid_foreground;
	}
else
if( w == w_sample.bg )
	{
	/* solid background selected */
	fill_stipple = patterns[solid_background];
	windowsetup.pattern_selected = solid_background;
	}
else
	{
	/* server default selected */
	windowsetup.pattern_selected = default_pattern;
	fill_stipple = 0;
	}
/* Set the window which shows the current pattern */
Set_Stipple();
/* Get push button pattern to change */
if (XtIsRealized(w_sample.fill))
  XClearArea(XtDisplay(w_sample.fill), XtWindow(w_sample.fill),
	     0, 0, 0, 0, True);
}
void
Set_Sample( w, tag, r )
    Widget w;					/* pulldown menu */
    caddr_t tag;
    XtCallbackList *r;		/* just pick up the boolean */
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Change the border width on the widget in order to force it
**	to redraw.  Not sure why we need to do this.  It only seems
**	to be called when the user clicks on the button which displays
**	the current pattern. 
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/
{
if (XtIsRealized(w_sample.fill))
  XClearArea(XtDisplay(w_sample.fill), XtWindow(w_sample.fill),
	     0, 0, 0, 0, True);
}

GC Create_GC( gc_id )
int gc_id;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Create all of the GCs that are needed.  This was taken directly
**	from the paint code.  We don't actually use all of these
**	GCs in the session manager pattern box.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/
{
XGCValues values;     
int value_mask;
GC new_gc;

switch( gc_id ){
    case GC_PD_SOLID :{
	    values.foreground = picture_fg;
	    values.background = picture_bg;
	    values.fill_style = FillSolid;
	    values.graphics_exposures = FALSE;
	    value_mask = GCForeground | GCBackground | 
	      GCFillStyle | GCGraphicsExposures;
	    new_gc =XCreateGC( display_id, pxmap, value_mask, &values);
	    break;
	    }

    case  GC_PD_ERASER :{
	    values.foreground = picture_bg;
	    values.background = picture_fg;
	    values.fill_style = FillSolid;
	    values.graphics_exposures = FALSE;
	    value_mask = GCForeground | GCBackground | 
	      GCFillStyle | GCGraphicsExposures;
	    new_gc =XCreateGC( display_id, pxmap, value_mask, &values);
	    break;
	    }


    case  GC_PD_FILL :{
	    values.foreground = picture_fg;
	    values.background = picture_bg;
	    values.fill_style = FillOpaqueStippled;
	    values.stipple = fill_stipple;
	    values.graphics_exposures = FALSE;
	    value_mask = GCForeground | GCBackground | 
			 GCFillStyle | GCStipple | GCGraphicsExposures;

	    new_gc =XCreateGC( display_id, pxmap, value_mask, &values);
	    break;
	    }

    case  GC_PD_FLOOD :{
	    values.foreground = picture_fg;
	    values.background = picture_bg;
	    values.fill_style = FillOpaqueStippled;
	    values.stipple = fill_stipple;
	    values.graphics_exposures = FALSE;
	    values.cap_style = CapNotLast;
	    value_mask = GCForeground | GCBackground | 
			 GCFillStyle | GCStipple | GCCapStyle |
			 GCGraphicsExposures;

	    new_gc =XCreateGC( display_id, pxmap, value_mask, &values);
	    break;
	    }

    case  GC_PD_COPY :{
	    values.foreground = picture_fg;
	    values.background = picture_bg;
	    values.graphics_exposures = FALSE;
	    value_mask = GCForeground | GCBackground | 
	      GCGraphicsExposures;
	    new_gc =XCreateGC( display_id, pxmap, value_mask, &values);
	    break;
	    }

    case GC_PD_FUNCTION :{
	    values.foreground = picture_fg;
	    values.background = picture_bg;
	    values.graphics_exposures = FALSE;
	    value_mask = GCForeground | GCBackground | 
	      GCGraphicsExposures;
	    new_gc =XCreateGC( display_id, pxmap, value_mask, &values);
	    break;
	    }

    case GC_PD_INVERT :{
	    values.function = GXinvert;
	    values.foreground = picture_fg;
	    values.background = picture_bg;
	    values.graphics_exposures = FALSE;
	    value_mask = GCFunction | GCForeground | 
	      GCBackground | GCGraphicsExposures;
	    new_gc =XCreateGC( display_id, pxmap, value_mask, &values);
	    break;
	    }


    case GC_SD_SOLID :{
	    values.foreground = window_fg;
	    values.background = window_bg;
	    values.fill_style = FillSolid;
	    values.graphics_exposures = FALSE;
	    value_mask = GCForeground | GCBackground | 
	      GCFillStyle | GCGraphicsExposures;
	    new_gc =XCreateGC( display_id, DefaultRootWindow(display_id), value_mask, &values);
	    break;
	    }

    case GC_SD_ERASER :{
	    values.foreground = window_bg;
	    values.background = window_fg;
	    values.fill_style = FillSolid;
	    values.graphics_exposures = FALSE;
	    value_mask = GCForeground | GCBackground | 
	      GCFillStyle | GCGraphicsExposures;
	    new_gc =XCreateGC( display_id, DefaultRootWindow(display_id), value_mask, &values);
	    break;
	    }


    case  GC_SD_FILL :{
	    values.foreground = window_fg;
	    values.background = window_bg;
	    values.fill_style = FillOpaqueStippled;
	    values.stipple = fill_stipple;
	    values.graphics_exposures = FALSE;
	    value_mask = GCForeground | GCBackground | 
			 GCFillStyle | GCStipple | GCGraphicsExposures;
	    new_gc =XCreateGC( display_id, DefaultRootWindow(display_id), value_mask, &values);
	    break;
	    }

    case GC_SD_COPY :{
	    values.foreground = WhitePixel(display_id,screen);
	    values.background = BlackPixel(display_id,screen);
	    values.graphics_exposures = FALSE;
	    value_mask = GCForeground | GCBackground | 
	      GCGraphicsExposures;
	    new_gc =XCreateGC( display_id, DefaultRootWindow(display_id), value_mask, &values);
	    break;
	    }

    case GC_RUBBERBAND :{
	    values.function = GXinvert;
	    values.foreground = window_fg;
	    values.background = window_bg;
	    values.plane_mask = window_fg ^ window_bg;
	    values.graphics_exposures = FALSE;
	    value_mask = GCFunction | GCForeground | GCPlaneMask |
	      GCBackground | GCGraphicsExposures;
	    new_gc =XCreateGC( display_id, DefaultRootWindow(display_id), value_mask, &values);
	    break;
	    }

    case GC_D1_COPY :{
	    values.foreground = 1;
	    values.background = 0;
	    values.graphics_exposures = FALSE;
	    value_mask = GCForeground | GCBackground | 
	      GCGraphicsExposures;
	    new_gc =XCreateGC( display_id, btmap, value_mask, &values);
	    break;
	    }

    case GC_D1_INVERT :{
	    values.function = GXinvert;
	    values.foreground = 1;
	    values.background = 0;
	    values.graphics_exposures = FALSE;
	    value_mask = GCFunction | GCForeground | 
	     GCBackground | GCGraphicsExposures;
	    new_gc =XCreateGC( display_id, btmap, value_mask, &values);
	    break;
	    }
    }
    return( new_gc );
}

Pixmap Create_Pixmap( value, width, height )
long value;
int width, height;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Creates a pixmap with the default screen depth 
**	Sets the foreground to the given color pixel index and fills
**	the pixmap with a solid rectangle of the given width and
**	height.
**
**  FORMAL PARAMETERS:
**
**	value - The pixel value for the foreground color
**	width - The width of the pixmap to be created
**	height - The height of the pixmap created.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/
{
Pixmap tmpmap;
GC gc_solid;

tmpmap = XCreatePixmap( display_id, 
			DefaultRootWindow(display_id), width, height, 
			(unsigned int)DefaultDepth(display_id,screen));
gc_solid = Get_GC( GC_SD_SOLID );
XSetForeground( display_id, gc_solid, value );
XFillRectangle( display_id, tmpmap, gc_solid, 0, 0, width, height );
return(tmpmap);
}

Pixmap Create_Bitmap( pixval, width, height )
int pixval;
int width, height;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Create and initialize a pixmap with a depth of 1, pixval must
**	be 1 or 0.
**	Sets the foreground to the given color pixel index and fills
**	the pixmap with a solid rectangle of the given width and
**	height.
**
**  FORMAL PARAMETERS:
**
**	pixval - The pixel value for the foreground color
**	width - The width of the pixmap to be created
**	height - The height of the pixmap created.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/
{
Pixmap tmpmap;
GC gc_bitmap;

tmpmap = XCreatePixmap( display_id, 
	 DefaultRootWindow(display_id), width, height, 1);
gc_bitmap = Get_GC( GC_D1_COPY );
XSetForeground( display_id, gc_bitmap, pixval );
XFillRectangle( display_id, tmpmap, gc_bitmap, 0, 0, width, height );
return(tmpmap);
}

Pixmap Create_Bitmap_Menu( parray, rows, cols, sz, row_space, col_space )
Pixmap parray[];
int rows, cols;
int sz;
int row_space, col_space;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**
**	Creates a pixmap that contains a menu.  The menu is formed by
**	the input pixmaps, size and spacing information.  The resulting
**	pixmap is returned.
**
**  FORMAL PARAMETERS:
**
**	parray - An array of pixmaps that will form the menu
**	rows - The number of rows for the pixmap menu
**	cols - The number of columns for the pixmap menu
**	sz - The width and height of the pixmap menu
**	row_space - The space between rows
**	col_space - The space between columns
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/
{
Pixmap menu;
int wd, ht;
int i, j;

wd = cols*sz + (cols+1)*col_space;
ht = rows*sz + (rows+1)*row_space;
menu = Create_Bitmap( 0, wd, ht );
for( i = 0; i < cols; ++i )
	for( j = 0; j < rows; ++j )
		{
		XCopyArea( display_id, parray[i*rows+j], menu, 
		  Get_GC(GC_D1_COPY), 0, 0, sz, sz, 
		  col_space+(i*(sz+col_space)), 
		  row_space+(j*(sz+row_space)) );
		}
return( menu );	
}

set_widget_color(dofore, doback, fore_pixel,back_pixel)
unsigned int dofore,doback;
unsigned long	fore_pixel,back_pixel;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	When the colors are changes for display foreground and background
**	we need to change the way the pattern box looks.  This routine
**	will change the sample button to display the selected pattern
**	in the correct colors, and change the foreground/background
**	solid buttons to have the correct colors.
**
**  FORMAL PARAMETERS:
**
**	dofore - If set to 1 the foreground color has changed
**	doback - If set to 1 the background color has changed
**	fore_pixel - The pixel value for foreground color
**	back_pixel - The pixel value for background color
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/
{
Arg args[7];
int argcnt = 0;
Pixmap	p,p1;
GC gc_solid;
unsigned int mask = 0;

if (dofore)
    {
    /* If the foreground color changed, then create a new pixmap with
       the correct colors and set the solid foreground button to have that
       pixmap */
    XtSetArg( args[argcnt], XmNforeground, fore_pixel);
    ++argcnt;
    window_fg = fore_pixel;
    p	= Create_Pixmap( window_fg, 16, 16 );
    XtSetArg( args[argcnt], XmNlabelPixmap, p );
    ++argcnt;
    XtSetArg( args[argcnt], XmNselectPixmap, p );
    ++argcnt;
    XtSetArg( args[argcnt], XmNlabelPixmap, p );
    ++argcnt;
    XtSetValues (w_sample.fg, args, argcnt);
    }
if (doback)
    {
    /* If the background color changed, then create a new pixmap with
       the correct colors and set the solid background  button to have that
       pixmap */
    argcnt = 0;
    XtSetArg( args[argcnt], XmNbackground, back_pixel);
    ++argcnt;
    window_bg = back_pixel;
    p1 = Create_Pixmap( window_bg, 16, 16 );
    XtSetArg( args[argcnt], XmNlabelPixmap, p1 );
    ++argcnt;
    XtSetArg( args[argcnt], XmNselectPixmap, p1 );
    ++argcnt;
    XtSetArg( args[argcnt], XmNlabelPixmap, p1 );
    ++argcnt;
    XtSetValues (w_sample.bg, args, argcnt);
    }

/* change some GC's to have correct color */
if (dofore)
    {
    gc_values.foreground = window_fg;
    mask = GCForeground;
    }
if (doback)
    {
    gc_values.background = window_bg;
    mask = mask | GCBackground;
    }

gc_solid = Get_GC( GC_SD_SOLID );
XChangeGC( display_id, gc_solid, mask, &gc_values );
gc_solid = Get_GC( GC_PD_FLOOD);
XChangeGC( display_id, gc_solid, mask, &gc_values );
gc_solid = Get_GC( GC_PD_FILL);
XChangeGC( display_id, gc_solid, mask, &gc_values );
gc_solid = Get_GC( GC_SD_FILL);
XChangeGC( display_id, gc_solid, mask, &gc_values );
/* Now set the sample pattern box to look correct */
Set_Stipple();
/* Get push button pattern to change */
if (XtIsRealized(w_sample.fill))
  XClearArea(XtDisplay(w_sample.fill), XtWindow(w_sample.fill),
	     0, 0, 0, 0, True);
}

set_correct_pattern()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	If the user changes their default pattern by using other
**	resource files, we will need to change the sample button
**	to have the correct pattern.   This routine initializes
**	the fill_stipple global to be the correct pattern and
**	calls the routines to reset the pattern.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/
{
fill_index = -1;
if( windowsetup.pattern_index  == solid_foreground)
    {
    fill_stipple = patterns[solid_foreground];
    }
else
    {    
    if( windowsetup.pattern_index == solid_background)
	{
	fill_stipple = patterns[solid_background];
	}
    else
	{
	if (windowsetup.pattern_index == default_pattern)
	    {
	    fill_stipple = 0;
	    }
	else
	    {
	    fill_index = windowsetup.pattern_index;
	    fill_stipple = patterns[windowsetup.pattern_index];
	    }
	}
    }
/* Change the button which displays the current selection */
Set_Stipple();
/* Get push button pattern to change */
if (XtIsRealized(w_sample.fill))
  XClearArea(XtDisplay(w_sample.fill), XtWindow(w_sample.fill),
	     0, 0, 0, 0, True);
}
