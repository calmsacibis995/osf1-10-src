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
/* BuildSystemHeader added automatically */
/* $Id: paintdefs.h,v 1.1.4.2 1993/06/25 22:46:37 Ronald_Hegli Exp $ */
/*
#module PAINTDEFS "V1-000"
****************************************************************************
**                                                                          *
**  Copyright (c) 1987                                                      *
**  By DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                        *
**  All Rights Reserved                                                     *
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
**   This module contains global definitions for all global variables
**
**  ENVIRONMENT:
**
**   User mode, executable image.
**
**  MODIFICATION HISTORY:
**	23-May-88 John Hainsworth - added SQRT2.
**
**--
**/           
#include <stdio.h>
#include <math.h>

#if defined(VMS) && !defined(__DECC)
#pragma nostandard
#endif

/* jj-port -> */
#ifdef PAINT_REFS  /* defined in paintrefs.h */

#ifndef VMS
#define def_or_ref extern
#else
#define def_or_ref globalref
#endif

#else

#ifndef VMS
#define def_or_ref
#else
#define def_or_ref globaldef
#endif

#endif
/* jj-port <- */

#include "position.h"
#include "paintuil.h"

#include <Xm/XmP.h>
#include <Mrm/MrmAppl.h>
#include <X11/Vendor.h>
#include <DXm/DECspecific.h>
#include <img/ImgDef.h>
#include <img/ImgEntry.h>
#include <img/IdsImage.h>
#include <img/ChfDef.h>

#include "sysdep.h" /* jj-port */

/* Conditional Compilation switches */
/* Epic callable */
/*
#define EPIC_CALLABLE
*/

/* print available */
#define PRINT 


/* ISL available */
#define ISL_SUPPORT

/* Macros for Error Handler Setup and Recovery */
#define ISL_ERROR_HANDLER_SETUP \
    failure_occurred = 0; \
    ChfEstablish (Force_Return_On_Failure);

#define ISL_RECOVER_FROM_ERROR  ChfRevert ();


/*
 * PAINTDEFS.H
 *
 * This is the definition file for all global variables and constants
 *
 */
/* Default picture measurements in millimeters */
#define MM_PER_INCH 25.4
#define DEF_PICTURE_WD (8.0*MM_PER_INCH)
#define DEF_PICTURE_HT (10.0*MM_PER_INCH)
#define DEF_PICTURE_X (1.0*MM_PER_INCH)
#define DEF_PICTURE_Y (0.5*MM_PER_INCH)
#define DEF_MASTER_WD (10.0*MM_PER_INCH)
#define DEF_MASTER_HT (10.0*MM_PER_INCH)
#define DEF_MASTER_X 0                
#define DEF_MASTER_Y 0
#define DEFAULT_X 50
#define DEFAULT_Y 50
#define US_LETTER_WIDTH 8.5	/* inches */
#define US_LETTER_HEIGHT 11.0	/* inches */


/*ram*/
/* define bit order and byte order constants for different hardware */
#ifdef vax
#define NATIVE_BYTE_ORDER LSBFirst
#define NATIVE_BIT_ORDER LSBFirst
#endif
/* following for MIPS (little endian) or MIPEL */
#ifdef MIPSEL
#define NATIVE_BYTE_ORDER LSBFirst
#define NATIVE_BIT_ORDER LSBFirst
#endif
/* following for Alpha (little endian) */
#ifdef __alpha
#define NATIVE_BYTE_ORDER LSBFirst
#define NATIVE_BIT_ORDER LSBFirst
#endif
#ifdef sparc
#define NATIVE_BYTE_ORDER MSBFirst
#define NATIVE_BIT_ORDER MSBFirst
#endif

/* standard resolutions */
#define DPI75	75
#define DPI100	100
#define DPI300	300
#define RES_ERR_TOLERANCE 0.15

#define BMU 1200

/* Success and Failure */
#define SUCCESS 0
#define FAILURE 1

/* rectangle sides */
#define LEFT 0
#define RIGHT 1
#define TOP 2
#define BOTTOM 3

/* Actions */
#define NO_ACTION -1
#define LINE 0
#define RECTANGLE 1
#define STROKE 2
#define ELLIPSE 3
#define POLYGON 4
#define ERASE 5
#define SELECT_RECT 6
#define SELECT_AREA 7
#define MOVE 8
#define ZOOM 9
#define PENCIL 10
#define ARC 11
#define BRUSH 12
#define FLOOD 13
#define TEXT 14
#define CLEAR 15
#define SPRAYCAN 16
#define CIRCLE 17
#define SQUARE 18
#define ZOOM_MOVE 19
#define CUT 20
#define COPY 21
#define PASTE 22
#define INVERT 23
#define SCALE 24
#define CROP 25
#define DROPPER 26
#define CHANGE_PICTURE_SIZE 27
#define SCALE_PICTURE 28
#define FULLVIEW_CROP 29
#define QUICK_COPY 30
#define INCLUDE 31
#define CLEAR_WW 32
#define MAX_ACTION 33

#define UNDO_ACTION_LENGTH 40

/* Miscellaneous constants */
#define MAXPTS 256	/* Max number of points used for drawing */
#define PI 3.14159					/* pi */
#define SQRT2 1.4142135
#define RADIANS_PER_DEGREE 57.2957795	/* Radians in one degreee */
#define DOUBLE_CLICK_TIME 1  /* The number of seconds between a double click */
#define DEGREES  180.0/PI
/* #define HORIZONTAL TRUE */ /* jj-port */
/* #define VERTICAL FALSE */ /* jj-port */
#define FATPIX_SIZE 16		/* The size of square to represent a pixel inzoom mode */ 
#define FATAL TRUE
#define NONFATAL FALSE
#define SENSITIVE TRUE  /* For setting widget sensitivity */
#define INSENSITIVE FALSE
#define SPRAY_RADIUS 20
#define SPRAY_DIAMETER 40
#define SPRAY_RADIUS_SQUARED 400
#define DDIF_FORMAT 0
#define XBITMAP_FORMAT 1
#define ICON_WIDTH 21
#define ICON_HEIGHT 21
#define MIN_HT_FOR_BIG_ICONS 550
#define UNDO_ACTION_LENGTH 40
        
/* Macro definitions */
#ifndef MAX
#define MAX( a, b ) ( (a) > (b) ? (a) : (b) )
#endif
#ifndef MIN
#define MIN( a, b ) ( (a) < (b) ? (a) : (b) )
#endif
#define MakeArg( n, v ) {	args[numargs].name = n;\
				args[numargs].value = v;\
				numargs++;\
			}
#define Inside_Rectangle( x1, y1, wd1, ht1, x2, y2, wd2, ht2)\
    ((((x1) >= (x2)) && ((y1) >= (y2)) && (((x1) + (wd1)) <= ((x2) + (wd2))) &&\
     (((y1) + (ht1)) <= ((y2) + (ht2)))) ? TRUE : FALSE)




/* given the function (f) from a gc  return the corresponding combination rule
   for an image combine */
#define COMBO_RULE(f) \
    ((f) == GXclear ? ImgK_ClrDst : \
     ((f) == GXand ? ImgK_SrcAndDst : \
      ((f) == GXandReverse ? ImgK_SrcAndNotDst : \
       ((f) == GXcopy ? ImgK_Src : \
	((f) == GXandInverted ? ImgK_NotSrcAndDst : \
	 ((f) == GXnoop ? ImgK_Dst : \
	  ((f) == GXxor ? ImgK_SrcXorDst : \
	   ((f) == GXor ? ImgK_SrcOrDst  : \
	    ((f) == GXnor ? ImgK_NotSrcAndNotDst : \
	     ((f) == GXequiv ? ImgK_NotSrcXorDst : \
	      ((f) == GXinvert ? ImgK_NotDst : \
	       ((f) == GXorReverse ? ImgK_SrcOrNotDst : \
		((f) == GXcopyInverted ? ImgK_NotSrc : \
		 ((f) == GXorInverted ? ImgK_NotSrcOrDst : \
		  ((f) == GXnand ? ImgK_NotSrcOrNotDst : \
		   ((f) == GXset ? ImgK_SetDst : (f)))))))))))))))))

/*									    
 *     DRM hierarchy and DRM type
 */
def_or_ref MrmHierarchy s_DRMHierarchy;   
def_or_ref MrmType dummy_class;

/*
 * Image variables
 */
def_or_ref XImage *picture_image;
def_or_ref long pimage_wd, pimage_ht;
def_or_ref int picture_x, picture_y;
#define MIN_PICTURE_WD 10
#define MIN_PICTURE_HT 10
def_or_ref float printer_page_wd, printer_page_ht;
def_or_ref int printer_resolution;

/*
 * Picture window variables 
 */
def_or_ref Position main_x, main_y;
def_or_ref Dimension main_wd, main_ht;
def_or_ref Dimension pwindow_wd, pwindow_ht;
def_or_ref Dimension main_min_wd, main_min_ht;

/* Picture location and size variables */
def_or_ref Dimension picture_wd, picture_ht;
def_or_ref Position pic_xorigin, pic_yorigin;
def_or_ref int max_picture_wd, max_picture_ht;

/* width and height of position window */
def_or_ref Dimension position_wd, position_ht;

/* Translate from picture window coordinates to image coordinates */
/* jj-really translating picture window to pixmap coordinates */
#define PWX_TO_IX( x ) ( (x) + pic_xorigin )
#define PWY_TO_IY( y ) ( (y) + pic_yorigin )

/* Translate from image coordinates to picture window coordinates */
/* jj-really translating pixmap to picture window coordinates */
#define IX_TO_PWX( x ) ( (x) - pic_xorigin )
#define IY_TO_PWY( y ) ( (y) - pic_yorigin )

/* Translate from pixmap coordinates to image coordinates */
#define  PMX_TO_IX( x ) ( (x) + picture_x )
#define  PMY_TO_IY( y ) ( (y) + picture_y )

/* Translate from image coordinates to pixmap coordinates */
#define  IX_TO_PMX( x ) ( (x) - picture_x )
#define  IY_TO_PMY( y ) ( (y) - picture_y )

/*
#define PWX_TO_ZX( x ) ( ((x) - zoom_xorigin) * zoom_pixsize )
#define PWY_TO_ZY( y ) ( ((y) - zoom_yorigin) * zoom_pixsize )
*/

/* PD stands for picture depth */
#define GC_PD_SOLID 0
#define GC_PD_ERASER 1
#define GC_PD_SQUARE_BRUSH 2
#define GC_PD_ROUND_BRUSH 3
#define GC_PD_OUTLINE 4
#define GC_PD_FILL 5
#define GC_PD_FLOOD 6
#define GC_PD_STROKE 7
#define GC_PD_COPY 8
#define GC_PD_FUNCTION 9
#define GC_PD_INVERT 10
#define GC_PD_SPRAY 11
#define GC_PD_MASK 23

/* SD stands for screen depth */
#define GC_SD_SOLID 12
#define GC_SD_ERASER 13
#define GC_SD_SQUARE_BRUSH 14
#define GC_SD_ROUND_BRUSH 15
#define GC_SD_OUTLINE 16
#define GC_SD_FILL 17
#define GC_SD_STROKE 18
#define GC_SD_COPY 19

/* D1 stands for depth 1 (bitmap) */
#define GC_D1_COPY 20
#define GC_D1_INVERT 21
#define GC_D1_SOLID 24
#define GC_D1_FUNCTION 25
#define GC_D1_GRAY 30

#define GC_RUBBERBAND 22

#define GC_MD_COPY 26
#define GC_MD_SOLID 27
#define GC_MD_OUTLINE 28
#define GC_MD_FILL 29

#define NUMBER_OF_GCS 31
def_or_ref GC GCs[NUMBER_OF_GCS];
/* jj-port */
#define Get_GC(g) ( (int)GCs[g] ? (GC)(GCs[g]) : (GC)(GCs[g]=(GC)Create_GC(g)) )

/* display information */
def_or_ref float wd_pix_per_mm, ht_pix_per_mm;

/* State Variables */
def_or_ref int button_down_mode;/* If TRUE, the use the click and drag model,
                           * otherwise uses click,move,click for the mouse*/
def_or_ref int rbanding;				/* If TRUE, the user is currently rubberbanding*/
def_or_ref int first_time;	/* first time rubberband flag */
def_or_ref int current_action;/* What the current user action is */
def_or_ref int prv_action;		/* What the previous user action is */
def_or_ref int use_brush;			/* If TRUE, use a brush to draw shapes outlines*/
def_or_ref int select_on;			/* If TRUE, the user has selected a part of the
                           * picture */
def_or_ref int alt_shape;			/* If TRUE, the alternate of the shape is drawn.
                           * ie. A rectangle would be drawn as a square. */
def_or_ref int resolution;
#define MIN_RESOLUTION 1
def_or_ref int screen_resolution;
def_or_ref int screen_wd;
def_or_ref int screen_ht;
def_or_ref int shift_key_down;/* If TRUE, the shift key is depressed */
def_or_ref int ctrl_key_down;/* If TRUE, the ctrl key is depressed */
def_or_ref int alt_key_down;/* If TRUE, the compose key is depressed */
def_or_ref int quit;				/* If TRUE, exit the program */
def_or_ref int first_move;	/* If TRUE, user has not moved the selected region*/
def_or_ref int select_rectangle; /* If TRUE, rectangle is selected, else area */
def_or_ref int highlight_on; /* If TRUE, highlight is on, else it is off */
def_or_ref long pencil_value; /* current pencil color */
def_or_ref int opaque_fill;
def_or_ref int screen_depth;
def_or_ref int screen;
def_or_ref int pdepth;
def_or_ref int stipple_sz;
def_or_ref int icon_sz;
def_or_ref unsigned long bit_plane; /* specifies bit_plane */
def_or_ref Visual *cur_visual;
def_or_ref Widget ifocus;	/* widget being used for input */
def_or_ref int img_format;	/* format to be used for images */
def_or_ref int image_stype;	/* spectral type for imaage */
def_or_ref int grid_on;	/* flag for grid on/off */
def_or_ref int grid_size; /* grid size in pixels */
def_or_ref XtIntervalId spray_timer;
def_or_ref Pixmap spraymap;
def_or_ref Widget scroll_window;
def_or_ref int xscroll, yscroll;
def_or_ref int undo_available;  /* undo function is available */
def_or_ref int exiting_paint;  /* not enough memory for startup */

/* variables for selecting part of the picture */
def_or_ref int select_x, select_y; /* Contains x,y pair defining a select rectangle*/
def_or_ref int select_width, select_height;
def_or_ref int undo_move_wd, undo_move_ht;
def_or_ref int undo_move_x, undo_move_y; /* jj-11/09/88 in image coords */
def_or_ref int undo_picture_x, undo_picture_y;
def_or_ref XPoint *select_pts;
def_or_ref XPoint *select_area_points;                
def_or_ref int num_selpts;
def_or_ref int num_sapts;
def_or_ref Region select_region;
def_or_ref int snafu; 
def_or_ref int easy_move; 
def_or_ref int refresh_zoom_from;
def_or_ref int failure_occurred;     /* ISL failure was falgged */
def_or_ref int flood_same_pattern;   /* true if flood is B on B or W on W. */
#define FROM_PICTURE 0
#define FROM_COPYMAP 1

def_or_ref int moved_only;
def_or_ref int no_distortion;
def_or_ref int no_net_move;
def_or_ref int deselected;

def_or_ref int orig_select_x, orig_select_y;
def_or_ref int orig_select_wd, orig_select_ht;
def_or_ref int orig_select_rect;
def_or_ref XPoint *orig_hi_points;
def_or_ref int orig_num_hipts;
def_or_ref Region orig_select_region;

/* variables for undo'ing scale */
def_or_ref int prv_select_x, prv_select_y;
def_or_ref Region prv_select_region;
def_or_ref int prv_num_hipts;
def_or_ref XPoint *prv_hi_points;
def_or_ref int prv_select_width;
def_or_ref int prv_select_height;
def_or_ref int prv_select_rectangle;
def_or_ref int prv_opaque_fill;
def_or_ref int first_scale_undo_after_deselect;
def_or_ref XImage *copymap_image;
def_or_ref Pixmap prv_copymap;
def_or_ref Pixmap prv_selmap;
def_or_ref Pixmap prv_cutmap;

def_or_ref int qc_x;
def_or_ref int qc_y;
def_or_ref int qc_redo;

/* Array to store pixel values in zoom mode */
def_or_ref long *bitvals;
def_or_ref short rband_x, rband_y;
def_or_ref short anchor_x, anchor_y;
def_or_ref int xdist, ydist;
def_or_ref int cur_x, cur_y;		/* Contains current pointer position */
def_or_ref int true_ptr_x, true_ptr_y;   /* true coords of pointer position */
def_or_ref int cur_fill_style; /* current fill style */

/* Variables to keep last computed arc */
def_or_ref int arc_x;
def_or_ref int arc_y;
def_or_ref int arc_wd;
def_or_ref int arc_ht;
def_or_ref int arc_beg_angle;
def_or_ref int arc_end_angle;
def_or_ref int chord_x1, chord_y1;
def_or_ref int chord_x2, chord_y2;

/* Variables to keep last computed rectangle */
def_or_ref int rect_x, rect_y;
def_or_ref int rect_wd, rect_ht;

/* Variables to keep track of crop */
def_or_ref int crop_rectangle;
def_or_ref int crop_x, crop_y;
def_or_ref int crop_wd, crop_ht;
def_or_ref Widget crop_dialog;
def_or_ref Widget crop_button;
def_or_ref int prv_picture_wd, prv_picture_ht;
def_or_ref int prv_pic_xorigin, prv_pic_yorigin;
def_or_ref int prv_picture_x, prv_picture_y;
def_or_ref int prv_pimage_wd, prv_pimage_ht;

/* Spray variables */
def_or_ref Pixmap spraymask;
#define SPRAY_SZ 16
/* #define SPRAY_RADIUS 8 */ /* jj-port */

/* X window variables */
def_or_ref Display *disp;
def_or_ref Window pwindow;
def_or_ref Window buttons;
def_or_ref Window master;
def_or_ref Window zoom;
def_or_ref Window select_window; /* point to window where selection occured*/
/*
def_or_ref GC gc_pencil;
def_or_ref GC gc_solid;
def_or_ref GC gc_opaque;
def_or_ref GC gc_copy;
def_or_ref GC gc_draw;
def_or_ref GC gc_bitmap;
*/
def_or_ref GC gc_rband;
def_or_ref GC gc_highlight;
def_or_ref GC cur_gc;
def_or_ref XGCValues gc_values;
def_or_ref Pixmap picture;
def_or_ref Pixmap erasermap;
def_or_ref Pixmap copymap;
def_or_ref Pixmap cutmap;
def_or_ref Pixmap selmap;
def_or_ref Pixmap undo_move_map;
def_or_ref Pixmap clip_mask;
def_or_ref Pixmap cur_drawable;
def_or_ref Pixmap undomap;
def_or_ref Pixmap scratchmap;
def_or_ref Pixmap pxmap;
def_or_ref Pixmap btmap;
def_or_ref Pixmap icon_pixmap;
def_or_ref Pixmap outline_stipple;
def_or_ref Pixmap fill_stipple;
def_or_ref Pixmap solid_stipple;
def_or_ref Pixmap solid_fg_stipple;
def_or_ref Pixmap solid_bg_stipple;

/* array to store widget id's of push_buttons, toggle_buttons, windows, etc */
/* which need to be accessed independently */
def_or_ref Widget widget_ids [200];

/* Variables for the event queue */
def_or_ref int event_mask;
def_or_ref XEvent event, *event_ptr;
def_or_ref XEvent nextevent;

typedef struct {
    int x, y;
    int wd, ht;
} MyRect;

/* Variables that store the last affected portion of the picture */
def_or_ref int undo_x;
def_or_ref int undo_y;
def_or_ref int undo_width;
def_or_ref int undo_height;
def_or_ref int undo_action;
def_or_ref int prv_undo_action;
def_or_ref int UG_wd;		    /* width of undo grid box */
def_or_ref int UG_ht;		    /* height of undo grid box */
def_or_ref int UG_rows;		    /* number of rows in undo grid */
def_or_ref int UG_cols;		    /* number of columns in undo grid */
def_or_ref int UG_last;		    /* number of undo boxes */
def_or_ref int UG_xwd;		    /* width of last box in row if not full */
def_or_ref int UG_xht;		    /* width of last box in colum if not full */
def_or_ref int UG_num;		    /* max number of boxes for screen */
def_or_ref int UG_num_used;	    /* number of boxes used for undo */
def_or_ref int *UG_used;		    /* array of used boxes */
def_or_ref XImage **UG_image;	    /* array of all boxes */
def_or_ref MyRect UG_extra[5];	    /* coords for boxes outside pixmap */

/* Variables that store the min and maxes for shapes */
def_or_ref int shape_xmin;
def_or_ref int shape_xmax;
def_or_ref int shape_ymin;
def_or_ref int shape_ymax;
def_or_ref int shape_wd;
def_or_ref int shape_ht;

/* Variables for zoom mode */
def_or_ref int zoom_xorigin;		/* The coordinate in the picture that the */
def_or_ref int zoom_yorigin;		/* zoom window uses as its origin */
def_or_ref int zoom_width;
def_or_ref int zoom_height;
def_or_ref int zoom_pixsize;		/* The square size of a pixel in zoom mode */
def_or_ref int zoom_pixborder;	/* The border surrounding the square pixsize*/
def_or_ref int zoomed;		/* Display is now in zoom mode */
def_or_ref Region zoom_region;
def_or_ref int zoom_move_xdist, zoom_move_ydist; /* distance to move zoom */
def_or_ref int old_zoom_wd;
def_or_ref int old_zoom_ht;

/* convert from zoom window coordinates to image coordinates */
#define ZX_TO_IX(x) (zoom_xorigin + ((x) < 0 ? \
    ((((x) + 1) / zoom_pixsize) - 1) : ((x) / zoom_pixsize)))
#define ZY_TO_IY(y) (zoom_yorigin + ((y) < 0 ? \
    ((((y) + 1) / zoom_pixsize) - 1) : ((y) / zoom_pixsize)))

#define ZX_TO_PWX(x) (zoom_xorigin - pic_xorigin + ((x) < 0 ? \
    ((((x) + 1) / zoom_pixsize) - 1) : ((x) / zoom_pixsize)))
#define ZY_TO_PWY(y) (zoom_yorigin - pic_yorigin + ((y) < 0 ? \
    ((((y) + 1) / zoom_pixsize) - 1) : ((y) / zoom_pixsize)))

#define IX_TO_ZX(x) (((x) - zoom_xorigin) * zoom_pixsize)
#define IY_TO_ZY(y) (((y) - zoom_yorigin) * zoom_pixsize)

/* Variables for text entry */
def_or_ref int entering_text;
def_or_ref int text_x;
def_or_ref int text_y;
def_or_ref int text_xmin, text_xmax, text_ymin, text_ymax;
#ifdef I18N_MULTIBYTE
/* Asian require more than 1 font */
#define MAXFONTS 3
def_or_ref XFontStruct *cur_font[MAXFONTS];
#else
def_or_ref XFontStruct *cur_font;
#endif /* I18N_MULTIBYTE */
Widget text_widget;

/* Widget variables */
def_or_ref Widget toplevel, main_widget, picture_widget;
def_or_ref Widget zoom_dialog, zoom_widget;
def_or_ref Widget scroll_widget;
def_or_ref Widget work_box;
def_or_ref Widget pic_hbar, pic_vbar;

/* dialog boxes */
def_or_ref Widget msgbox;
def_or_ref Widget help_widget;
def_or_ref Widget default_question_dialog;
def_or_ref Widget ai_error_caution_box;
def_or_ref Widget pic_shape_dialog;
def_or_ref Widget scale_dialog;
def_or_ref Widget color_mix_apply_cb;
def_or_ref Widget grid_size_dialog;

#define NUM_DIALOG_BOXES 22

/* Variables for menubar */
#define FILE_PULLDOWN 0
#define EDIT_PULLDOWN 1
#define OPTIONS_PULLDOWN 2
#define FONT_PULLDOWN 3
#define CUSTOMIZE_PULLDOWN 4
#define HELP_PULLDOWN 5
#define NUM_MBAR_ENTRIES 6
def_or_ref Widget menubar;
def_or_ref Widget mbar[NUM_MBAR_ENTRIES];


typedef struct {
    unsigned long pixel;
    float f_red, f_green, f_blue;
    unsigned short int i_red, i_green, i_blue;
} MyColor;

typedef struct {
    unsigned short int red;
    unsigned short int green;
    unsigned short int blue;
} DDIF_COLOR_TYPE;

/* Miscellaneous globals */
def_or_ref XColor foreground;
def_or_ref XColor background;
def_or_ref XColor cursor_fg, cursor_bg;
def_or_ref long window_fg, window_bg, paint_color, paint_bg_color;
def_or_ref long picture_fg, picture_bg;
def_or_ref unsigned char picture_bg_byte, picture_fg_byte;
def_or_ref long fground;
def_or_ref long bground;
def_or_ref Widget color_dialog;
def_or_ref Widget color_mix_dialog;
def_or_ref XVisualInfo *visual_info;
def_or_ref int refresh_picture_pending;
def_or_ref int window_exposure;

#define K_ALLOC_COLOR_FAILED -1
#define MAX_COLORS 256
#define MAX_COLOR_VALUE 65535
def_or_ref MyColor colormap[MAX_COLORS];
def_or_ref int num_colors;
def_or_ref MyColor new_colors[MAX_COLORS];
def_or_ref int num_new_colors;

def_or_ref unsigned char pixel_remap[MAX_COLORS];

def_or_ref int gpx;
def_or_ref int gpx_reduction;
def_or_ref int colormap_size;
def_or_ref unsigned long black_pixel;
def_or_ref unsigned long white_pixel;

#define BLACK 0
#define WHITE 1

#define CURSOR_FG_INDEX 0 
#define CURSOR_BG_INDEX 1 
#define WINDOW_FG_INDEX 2
#define WINDOW_BG_INDEX 3
#define WINDOW_BORDER_INDEX 4
#define WINDOW_HIGHLIGHT_INDEX 5
#define NUM_SPECIAL_COLORS 6
def_or_ref unsigned long special_colors[NUM_SPECIAL_COLORS];
def_or_ref num_special_indexes;
def_or_ref unsigned long special_indexes[NUM_SPECIAL_COLORS];

def_or_ref Colormap default_colormap;
def_or_ref Colormap paint_colormap;

def_or_ref int file_color;

/* allocat 128 points at a time */
#define ALLOC_NUM 128
def_or_ref XPoint *points;                                        
def_or_ref int numpts;
def_or_ref int num_alloc;

def_or_ref XPoint *hi_points;
def_or_ref int num_hipts;
def_or_ref char *geostr;                       
def_or_ref int moved_xdist;
def_or_ref int moved_ydist;


/* File spec variables */
/* dl - 10/19/88 add default types */
/* change DIR_MASK (in translaute.uil) if modified */
#define DDIF_DEFAULT_TYPE ".img" 
#define XBITMAP_DEFAULT_TYPE ".dat"
def_or_ref char cfile[256];
def_or_ref char *cur_file;	/* the name of the current picture file */
def_or_ref char cname[256];
def_or_ref char *cur_name;
def_or_ref char tfile[256];
def_or_ref char *temp_file;	/* the name of the current picture file */
def_or_ref char lfile[256];
def_or_ref char *last_file_name; /* the name of the current picture file */
def_or_ref char ifile[256];
def_or_ref char *include_file_name; /* the name of the file to be included */
def_or_ref int file_format;
def_or_ref int new_file;
def_or_ref int almost_new_file;
def_or_ref int picture_changed;
def_or_ref int pixmap_changed;

/* file status codes */
#define K_SUCCESS 1
#define K_NOT_BITONAL 2
#define K_IMAGE_TOO_SMALL 3
#define K_ISL_FAILURE 4
#define K_CANNOT_OPEN 5
#define K_NO_IMAGE 6
#define K_IMAGE_TOO_BIG 7
#define K_NO_PAD_IMAGE 8
#define K_NO_CLIENT_MEMORY 9
#define K_NO_DECOMPRESS 10
#define K_BITMAP_TOO_LARGE 11
#define K_FAILURE 12
#define K_GREY_SCALE 13
#define K_NO_COLOR_REMAP 14
#define K_NO_HISTOGRAM 15
#define K_NO_PRESENT_SURFACE 16
#define K_NO_RENDERING 17
#define K_DDIF_NO_GOOD 18
#define K_PICTURE_NOT_BW 19
#define K_NO_CONVERT_IMAGE_DATA 20
#define K_CLIPBOARD_LOCKED 21
#define K_UNSUPPORTED_FORMAT 22
#define K_CANNOT_WRITE 23
#define K_CANNOT_CLOSE 24
#define K_UNKNOWN_FILE_FORMAT -1

/* add color status codes */
#define K_NEW_COLOR 0
#define K_MATCH_COLOR 1
#define K_CLOSEST_COLOR 2

def_or_ref int change_pix_x;
def_or_ref int change_pix_y;
def_or_ref int change_pix_wd;
def_or_ref int change_pix_ht;


def_or_ref Widget write_dialog;
def_or_ref Widget read_dialog;
def_or_ref Widget include_dialog;
def_or_ref Widget print_dialog;
def_or_ref Widget print_2_dialog;


/* fullview variables */
#define NORMAL_VIEW 0
#define FULL_VIEW 1
def_or_ref int paint_view;
def_or_ref int select_portion_wd;
def_or_ref int select_portion_ht;
def_or_ref int sp_select_on;

/* Line width variables */
def_or_ref int cur_line_wd;
def_or_ref int line_width[NUM_LINE_SIZES];
def_or_ref Pixmap line_map[NUM_LINE_SIZES];
def_or_ref Widget line_wd_button[NUM_LINE_SIZES];
def_or_ref Widget line_dialog;

/* Icon variables */
def_or_ref Widget icon_window;

/* Brush shape variables */
#define SQUARE_BRUSH 0
#define ROUND_BRUSH 1
#define R45_BRUSH 2
#define L45_BRUSH 3
#define HORIZ_BRUSH 4
#define VERT_BRUSH 5
typedef struct {
	int wd;
	int x1, y1;
	int x2, y2;
	} brush_shape;
def_or_ref int cur_brush;
def_or_ref int cur_brush_index;
def_or_ref int brush_wd;
def_or_ref int brush_half_wd;
def_or_ref int brush_x1, brush_y1;
def_or_ref int brush_x2, brush_y2;

def_or_ref brush_shape brushes[ NUM_BRUSH_SHAPES ][ NUM_BRUSH_SIZES ];
def_or_ref Widget brush_button[ NUM_BRUSH_SIZES ][ NUM_BRUSH_SHAPES ];
def_or_ref Widget bshape_dismiss_button;
def_or_ref Widget brush_dialog;

/* Pattern variables */
#define NUM_PATTERNS 64
def_or_ref Widget edit_pat_dialog;
def_or_ref int pindex;
def_or_ref Widget pattern_dialog;
def_or_ref Pixmap patterns[NUM_PATTERNS];
def_or_ref Pixmap pattern_pixmap;
def_or_ref int pattern_sz, pattern_space;
def_or_ref int patcol, patrow;
def_or_ref Window pattern_window;

/*        
 * Event translation table for picture and zoom windows
 */    
def_or_ref XtTranslations main_translation; /* parsed Translation table */
def_or_ref XtTranslations motion_translation;
def_or_ref XtTranslations resize_translation;

/* isl macros and variables for use with ISL */
#define DDIF_BUFFER_LENGTH 1024

static int ddif_buf;
static int isl_ctx;
static long item_id;
static long pixels_per_scanline;
static long scanline_count;
static long bits_per_pixel;
static long scanline_stride;
static int spectral_type; 
static long pixel_order; 
static int bytes;

/*
   DSC_K values (to go with buf_descriptor_type structure)
*/

#define DSC_K_DTYPE_Z	0		/* unspecified */
#define DSC_K_DTYPE_BU	2		/* byte (unsigned);  8-bit unsigned quantity */
#define DSC_K_DTYPE_WU	3		/* word (unsigned);  16-bit unsigned quantity */
#define DSC_K_DTYPE_LU	4		/* longword (unsigned);  32-bit unsigned quantity */
#define DSC_K_DTYPE_QU	5		/* quadword (unsigned);  64-bit unsigned quantity */
#define DSC_K_DTYPE_OU	25		/* octaword (unsigned);  128-bit unsigned quantity */
#define DSC_K_DTYPE_B	6		/* byte integer (signed);  8-bit signed 2's-complement integer */
#define DSC_K_DTYPE_W	7		/* word integer (signed);  16-bit signed 2's-complement integer */
#define DSC_K_DTYPE_L	8		/* longword integer (signed);  32-bit signed 2's-complement integer */
#define DSC_K_DTYPE_Q	9		/* quadword integer (signed);  64-bit signed 2's-complement integer */
#define DSC_K_DTYPE_O	26		/* octaword integer (signed);  128-bit signed 2's-complement integer */
#define DSC_K_DTYPE_F	10		/* F_floating;  32-bit single-precision floating point */
#define DSC_K_DTYPE_D	11		/* D_floating;  64-bit double-precision floating point */
#define DSC_K_DTYPE_G	27		/* G_floating;  64-bit double-precision floating point */
#define DSC_K_DTYPE_H	28		/* H_floating;  128-bit quadruple-precision floating point */
#define DSC_K_DTYPE_FC	12		/* F_floating complex */
#define DSC_K_DTYPE_DC	13		/* D_floating complex */
#define DSC_K_DTYPE_GC	29		/* G_floating complex */
#define DSC_K_DTYPE_HC	30		/* H_floating complex */
#define DSC_K_DTYPE_CIT	31		/* COBOL Intermediate Temporary */
/*
 *	String data types:
 */
#define DSC_K_DTYPE_T	14		/* character string;  a single 8-bit character or a sequence of characters */
#define DSC_K_DTYPE_VT	37		/* varying character string;  16-bit count, followed by a string */
#define DSC_K_DTYPE_NU	15		/* numeric string, unsigned */
#define DSC_K_DTYPE_NL	16		/* numeric string, left separate sign */
#define DSC_K_DTYPE_NLO	17		/* numeric string, left overpunched sign */
#define DSC_K_DTYPE_NR	18		/* numeric string, right separate sign */
#define DSC_K_DTYPE_NRO	19		/* numeric string, right overpunched sign */
#define DSC_K_DTYPE_NZ	20		/* numeric string, zoned sign */
#define DSC_K_DTYPE_P	21		/* packed decimal string */
#define DSC_K_DTYPE_V	1		/* aligned bit string */
#define DSC_K_DTYPE_VU	34		/* unaligned bit string */
/*
 *	Miscellaneous data types:
 */
#define DSC_K_DTYPE_ZI	22		/* sequence of instructions */
#define DSC_K_DTYPE_ZEM	23		/* procedure entry mask */
#define DSC_K_DTYPE_DSC	24		/* descriptor */
#define DSC_K_DTYPE_BPV	32		/* bound procedure value */
#define DSC_K_DTYPE_BLV	33		/* bound label value */
#define DSC_K_DTYPE_ADT	35		/* absolute date and time */
/*
 *	Reserved data type codes:
 *	codes 38-191 are reserved to DIGITAL;
 *	codes 160-191 are reserved to DIGITAL facilities for facility-specific purposes;
 *	codes 192-255 are reserved for DIGITAL's Computer Special Systems Group
 *	  and for customers for their own use.
 */


/*
 *	Codes for DSC_b_class:
 */
#define DSC_K_CLASS_S	1		/* fixed-length descriptor */
#define DSC_K_CLASS_D	2		/* dynamic string descriptor */
/*	DSC_K_CLASS_V			** variable buffer descriptor;  reserved for use by DIGITAL */
#define DSC_K_CLASS_A	4		/* array descriptor */
#define DSC_K_CLASS_P	5		/* procedure descriptor */
/*	DSC_K_CLASS_PI			** procedure incarnation descriptor;  obsolete */
/*	DSC_K_CLASS_J			** label descriptor;  reserved for use by the VMS Debugger */
/*	DSC_K_CLASS_JI			** label incarnation descriptor;  obsolete */
#define DSC_K_CLASS_SD	9		/* decimal string descriptor */
#define DSC_K_CLASS_NCA	10		/* noncontiguous array descriptor */
#define DSC_K_CLASS_VS	11		/* varying string descriptor */
#define DSC_K_CLASS_VSA	12		/* varying string array descriptor */
#define DSC_K_CLASS_UBS	13		/* unaligned bit string descriptor */
#define DSC_K_CLASS_UBA	14		/* unaligned bit array descriptor */
#define DSC_K_CLASS_SB	15		/* string with bounds descriptor */
#define DSC_K_CLASS_UBSB 16		/* unaligned bit string with bounds descriptor */
/*
 *	Reserved descriptor class codes:
 *	codes 15-191 are reserved to DIGITAL;
 *	codes 160-191 are reserved to DIGITAL facilities for facility-specific purposes;
 *	codes 192-255 are reserved for DIGITAL's Computer Special Systems Group
 *	  and for customers for their own use.
 */


/*
  Pixel remap descriptor and stuff
*/

typedef struct buf_descriptor_struct {
    unsigned short  dsc$w_length;
    unsigned char   dsc$b_dtype;
    unsigned char   dsc$b_class;
    char            *dsc$a_pointer;
    char            dsc$b_scale;
    unsigned char   dsc$b_digits;
    unsigned char   dsc$b_aflags;
    unsigned char   dsc$b_dimct;
    unsigned long   dsc$l_arsize;
} buf_descriptor_type;

typedef struct  item_list_struct {
    unsigned short int  item_length;
    unsigned short int  item_code;
    int                 item_address;
} ITEM_LIST_TYPE;

typedef struct pf_entry_struct {
    long int pixel, freq;
} PF_ENTRY_TYPE;


/*
  Itemlist handling for setting ISL attributes.
 */

/* jj-port */
#define put_set_item(item_list, item_index, item_code_value, component)        \
   {                                                                           \
    item_index++;                                                              \
    item_list[item_index].PutL_Code = (unsigned long) item_code_value;         \
    item_list[item_index].PutL_Length = (unsigned long) sizeof(component);     \
    item_list[item_index].PutA_Buffer = (char *) &component;                   \
    item_list[item_index].PutL_Index = 0;                                      \
   }

#define end_set_itemlist(item_list, item_index)                                \
   {                                                                           \
    item_index++;                                                              \
    item_list[item_index].PutL_Code = 0;                                       \
    item_list[item_index].PutL_Length = 0;                                     \
    item_list[item_index].PutA_Buffer = 0;                                     \
    item_list[item_index].PutL_Index = 0;                                      \
   }

#define start_set_itemlist(item_list, item_index)                              \
   {                                                                           \
    item_index = -1;                                                           \
    item_list[0].PutL_Code = 0;                                                \
    item_list[0].PutL_Length = 0;                                              \
    item_list[0].PutA_Buffer = 0;                                              \
    item_list[0].PutL_Index = 0;                                               \
   }

/*
  Itemlist handling for retrieving ISL attributes.
 */

/* jj-port */
#define put_get_item(item_list, item_index, item_code_value, component)        \
   {                                                                           \
    item_index++;                                                              \
    item_list[item_index].GetL_Code = (unsigned long) item_code_value;         \
    item_list[item_index].GetL_Length = (unsigned long) sizeof(component);     \
    item_list[item_index].GetA_Buffer = (char *) &component;                   \
    item_list[item_index].GetA_Retlen = 0;                                     \
    item_list[item_index].GetL_Index = 0;                                      \
   }

#define end_get_itemlist(item_list, item_index)                                \
   {                                                                           \
    item_index++;                                                              \
    item_list[item_index].GetL_Code = 0;                                       \
    item_list[item_index].GetL_Length = 0;                                     \
    item_list[item_index].GetA_Buffer = 0;                                     \
    item_list[item_index].GetA_Retlen = 0;                                     \
    item_list[item_index].GetL_Index = 0;                                      \
   }

#define start_get_itemlist(item_list, item_index)                              \
   {                                                                           \
    item_index = -1;                                                           \
    item_list[0].GetL_Code = 0;                                                \
    item_list[0].GetL_Length = 0;                                              \
    item_list[0].GetA_Buffer = 0;                                              \
    item_list[0].GetA_Retlen = 0;                                              \
    item_list[0].GetL_Index = 0;                                               \
   }


#define make_rect_buf(rbuf, x, y, wd, ht)                                      \
   {                                                                           \
    rbuf[0] = x;							       \
    rbuf[1] = y;							       \
    rbuf[2] = wd;							       \
    rbuf[3] = ht;							       \
   }

/* 
 * vms string descriptor type 
 */
typedef struct v_d_t
   {
    short string_length;
    unsigned char string_type;
    unsigned char string_class;
    unsigned char *string_address;
   } vms_descriptor_type;

typedef struct li
	{
	unsigned short int len; /* component length */
	unsigned short int code; /* the item code */
	unsigned long int str; /* address of string */
	} item_desc;

/*
 * EPIC Applications Interface related stuffs
 */
def_or_ref int run_as_child;
def_or_ref int startup_initialize;

#ifdef EPIC_CALLABLE
/* jj-port -> */
#ifdef ULTRIX
#ifndef no_proto
#define no_proto
#endif
#endif
/* jj-port <- */
#include <ail.h>

def_or_ref AiSession session;
def_or_ref AiAppl parent_hnd;
def_or_ref int widget_realized;
def_or_ref unsigned long terminate_hnd;
def_or_ref int session_active;
#endif

def_or_ref Opaque hyperhelp_context;
#define NoPaintHelp  "Can't fetch help message \n"
#if defined(VMS)
#define PAINT_HELP "decw$paint"
#else
#define PAINT_HELP "dxpaint"
#endif

#if defined(VMS) && !defined(__DECC)
#pragma standard
#endif
