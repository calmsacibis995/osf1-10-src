/*
 *  Title:	DECtermP.h - DECterm Widget common definitions and data structures
 *
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © Digital Equipment Corporation, 1988, 1993 All Rights       |
 *  | Reserved.  Unpublished rights reserved under the copyright laws of     |
 *  | the United States.                                                     |
 *  |                                                                        |
 *  | The software contained on this media is proprietary to and embodies    |
 *  | the confidential technology of Digital Equipment Corporation.          |
 *  | Possession, use, duplication or dissemination of the software and      |
 *  | media is authorized only pursuant to a valid written license from      |
 *  | Digital Equipment Corporation.                                         |
 *  |                                                                        |
 *  | RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the      |
 *  | U.S. Government is subject to restrictions as set forth in             |
 *  | Subparagraph (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19,    |
 *  | as applicable.                                                         |
 *  |                                                                        |
 *  | The information in this software is subject to change  without  notice |
 *  | and  should  not  be  construed  as  a commitment by Digital Equipment |
 *  | Corporation.                                                           |
 *  |                                                                        |
 *  | DIGITAL assumes no responsibility for the use or  reliability  of  its |
 *  | software on equipment which is not supplied by DIGITAL.                |
 *  +------------------------------------------------------------------------+
 *  
 *  
 *  Module Abstract:
 *
 *	Definitions common to all DECterm widget modules
 *
 *  Author:	Tom Porcher
 *
 *  Modification history:
 *
 *  Alfred von Campe    07-Dec-1993     BL-E
 *	- Subclass DECterm widget off of XmManager instead of just Composite.
 *
 *  Alfred von Campe    02-Nov-1993     BL-E
 *	- Use 10 and 14 point fonts on 100 DPI monitors.
 *
 *  Eric Osman		30-Aug-1993	BL-D
 *	- Add STOP_OUTPUT_RESIZE to fix problem of regis commands appearing
 *	  on screen instead of being interpreted properly as regis, which
 *	  could occur if one command string both changed terminal size and
 *	  then invoked regis.
 *
 *  Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 *  Alfred von Campe    21-May-1993     V1.2
 *      - Include dt_wv_hdr.h instead of the obsolete dt_source.h
 *
 *  Alfred von Campe    07-Oct-1992     Ag/BL10
 *      - Changed #include syntax
 *
 *  Alfred von Campe    20-Feb-1992     V3.1
 *      - Add color text support.
 *
 *  Aston Chan		19-Dec-1991	V3.1
 *	- I18n code merge
 *
 *  Alfred von Campe    06-Oct-1991     Hercules/1 T0.7
 *      - Changed private #include file names to all lower case.
 *      - Removed FMT8BIT altogether.
 *
 *  Aston Chan		1-Sep-1991	Alpha
 *	- DECC complained FMT8BIT being multiply defined.  Comment out for
 *	  Alpha because FMT8BIT is not being used anyway.
 *
 *  Jim Bay		 2-Dec-1990
 *	- Temporarily removed #define for ASIAN_SUPPORT to fix problem
 *	  with GS fonts not being displayed
 *
 *  Bob Messenger	15-Sep-1990	X3.0-7
 *	- Add support for GS font.
 *
 *  Michele Lien    24-Aug-1990 VXT X0.0
 *  - Modify this module to work on VXT platform. Change #ifdef VMS to
 *    #ifdef VMS_DECTERM so that VXT specific code can be compiled under
 *    VMS or ULTRIX development environment.
 *  
 *  Bob Messenger	19-Jul-1990	X3.0-5
 *	- Specify the default font set names in decipoints (180 and 140)
 *	  instead of pixels, as part of support for 18 point fonts on
 *	  100 dpi systems.
 *
 *  Bob Messenger	17-Jul-1990	X3.0-5
 *	- Enable Asian terminal support by defining ASIAN_SUPPORT.
 *
 *  Mark Woodbury   25-May-1990 X3.0-3M
 *  - Motif update
 *
 *  Bob Messenger	31-May-1989	X2.0-13
 *	- Look for 14 and 18 pixels for the default font sets, rather than
 *	  140 and 180 decipoints.
 *
 *  Bob Messenger	26-Apr-1989	X2.0-12
 *	- Support V1 font encodings (in addition to V2 encodings).
 *
 *  Bob Messenger	 7-Apr-1989	X2.0-6
 *	- Support variable number of bit planes.
 *
 *  Bob Messenger	 4-Apr-1989	X2.0-5
 *	- Support backing store enable.
 *
 *  Bob Messenger	31-Mar-1989	X2.0-5
 *	- Support shared colormap entries
 *
 *  Tom Porcher         22-Apr-1988     X0.4-11
 *      - #undef'ed the RESOURCE and C_RESOURCE macros so they don't get
 *        redefined warnings.
 *
 *  Bob Messenger	18-Feb-1988	X0.4-0
 *	Added ReGIS and sixel context blocks
 */

#ifndef _DECtermP_h
#define _DECtermP_h

#if !defined(VMS_DECTERM) && !defined(VXT_DECTERM)
#include <Mrm/MrmAppl.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <Xm/XmP.h>
#include <Xm/ManagerP.h>
#else
#include "MrmAppl.h"
#include "IntrinsicP.h"
#include "CoreP.h"
#include "XmP.h"
#include "ManagerP.h"
#include "Xutil.h"
#include "Xatom.h"
#endif

#ifndef VAXC
#define globaldef
#define globalref	extern
#endif


/*
 * Conditional compilation.
 */

/* Removed ASIAN_SUPPORT */

#define VT340_COLOR_TEXT
                        /* VT340-style color text: include support for the
                           useBoldFont resource. */

#include "decterm.h"


/*
 * Miscellaneous definitions
 */

typedef int DECtermPosition;
typedef struct {		/* DECtermPosition_struct */
    unsigned int column :  8;
             int row    : 24;
} DECtermPosition_struct ;

#define DECtermPosition_column( p ) ((unsigned char) (p))
#define DECtermPosition_row( p ) ((p)>>8)
#define DECtermPosition_position( c, r ) (DECtermPosition) (((r)<<8)+(c))
#define MAX_POSITION ((1<<(8*( sizeof(DECtermPosition)-1 )-1 ))-1) /* 2^23-1 */

#define DEFAULT_BIG_FONT_SET_NAME "-*-Terminal-*-*-*--*-180-*-*-*-*-*-*"
#define DEFAULT_LITTLE_FONT_SET_NAME "-*-Terminal-*-*-*--*-140-*-*-*-*-*-*"
#define DEFAULT_GS_FONT_SET_NAME "-*-Terminal-*-*-*-GS-*-140-*-*-*-*-*-*"
#define DEFAULT_ASIAN_BIG_FONT_SET_NAME \
	"-ADECW-Screen-Medium-R-Normal--24-*-*-*-*-*-*-*"
#define DEFAULT_ASIAN_LITTLE_FONT_SET_NAME \
	"-ADECW-Screen-Medium-R-Normal--18-*-*-*-*-*-*-*"
#define DEFAULT_ASIAN_FINE_FONT_SET_NAME \
	"-JDECW-Screen-Medium-R-Normal--14-*-*-*-*-*-*-*"

/*
 * The following macro finds the approximate DPI of a display using integer
 * arithmetic.  To get the exact DPI it would need to use floating point and
 * a conversion factor of 25.4 (the number of millimeters in an inch) instead
 * of just 25.  Since we can assume we have a 100 DPI monitor if the DPI is
 * greater than 90, we don't need to be too accurate in determining the DPI of
 * the display.
 */
#define GetDisplayDPI(dpy) (XDisplayWidth((dpy), XDefaultScreen((dpy))) * 25 / \
                           XDisplayWidthMM((dpy), XDefaultScreen((dpy))))
/*
 * Common data structure for DECterm Widget data record
 */

typedef struct {		/* CommonData */

    /* Resource values:  common widget resources */

#define RESOURCE( p1, p2, p3, p4, p5, p6, p7, p8 ) p4 p5;
#define C_RESOURCE( p1, p2, p3, p4, p5, p6, p7, p8 )

#include "dt_resources.h"
#undef RESOURCE
#undef C_RESOURCE

	Boolean color_map_allocated;	/* TRUE means the graphics color
					   map has been allocated */
	XColor *color_map;		/* shared color map for graphics */
	unsigned long *color_map_mono;	/* monochrome value of each color */

	XColor pure_color[8];		/* best rgb values for hardware */

#define COLORS_PER_LIST_ELEMENT 10

	struct color_list_element
	    {
	    struct color_list_element *next;
	    int n;
	    Pixel pixels[COLORS_PER_LIST_ELEMENT];
	    } *free_color_list;

	Boolean *pixel_valid;		/* for each color */
	Boolean *pixel_allocated;	/* for each color */

	Pixel base_color;		/* allocated color */
	unsigned long *plane_masks;	/* allocated planes */
	unsigned long allocated_plane_mask;	/* mask of allocated planes */
	unsigned long cursor_plane_mask; /* planes to complement for cursor */
	int cursor_planes_affected;	/* number of emulated planes to
					   include in cursor_plane_mask */
	Pixel original_foreground_color,
	      original_background_color;
					/* foreground and background colors
					   in unallocated color cells */
	int text_foreground_index;	/* color table index for text fg */
	int text_background_index;	/* color table index for text bg */
	unsigned long default_foreground_red;
	unsigned long default_foreground_green;
	unsigned long default_foreground_blue;
	unsigned long default_foreground_mono;
	unsigned long default_background_red;
	unsigned long default_background_green;
	unsigned long default_background_blue;
	unsigned long default_background_mono;

	Pixel black_pixel, white_pixel;

	int hardware_planes;		/* number of planes in workstation */
	int graphics_mode;

#define SINGLE_PLANE 0			/* 1 plane, black and white */
#define ALLOCATED_COLORS 1		/* shared cells in default color map */
#define ALLOCATED_PLANES 2		/* exclusive planes in default cm */
#define ALLOCATED_COLORMAP 3		/* private colormap */

	Visual *visual;			/* visual for this window */
	Colormap default_colormap;	/* default colormap for screen */

	int logical_width;		/* width of logical (underlying)
					   display in pixels */
	int logical_height;		/* height of logical (underlying)
					   display in pixels */
	int display_width;		/* width of visible display in
					   pixels */
	int display_height;		/* height of visible display in
					   pixels */
	int origin_x;			/* horizontal offset of visible display
					   within logical display (in pixels) */
	int origin_y;			/* vertical offset of visible display
					   within logical display (in pixels) */
	Boolean backing_store_active;	/* TRUE means display must be
					   updated from backing store */
	Boolean graphics_visible;	/* TRUE means graphics are
					   potentially visible on the screen */
	Boolean v1_encodings;		/* TRUE means the fonts on this server
					   are using the V1 character
					   encodings, with DEC Multinational
					   in the ISO Latin 1 fonts */
#ifdef VT340_COLOR_TEXT
        Boolean really_use_bold_font;   /* TRUE if the useBoldFont resource
                                           is TRUE or if this is a single
                                           plane system. */
#endif
} CommonData;

/*
 * Widget data record for a DECterm widget instance
 *
 * To reference data in the Widget data record, procedures must declare the
 * parameter w to be of type DECtermWidget:
 *		DECtermWidget	w;
 *
 * Example references to fields in the widget data record:
 *		w->common.rows
 *		w->source.wv.vt200_flags
 */
#include "dt_control.h"
#include "dt_wv_hdr.h"
#include "dt_output.h"
#include "dt_input.h"
#include "dt_regis.h"
#include "dt_sixel.h"

typedef struct {		/* DECtermData */
    CorePart    core;
    CompositePart composite;
    ConstraintPart constraint;
    XmManagerPart manager;
    CommonData  common;
    ControlData control;
    SourceData  source;
    OutputData  output;
    InputData   input;
    RegisData	regis;
    SixelData	sixel;
} DECtermData, *DECtermWidget;

#define STOP_OUTPUT_TEMP 1<<0
#define STOP_OUTPUT_HOLD 1<<1
#define STOP_OUTPUT_SCROLL 1<<2
#define STOP_OUTPUT_REPORT 1<<3
#define STOP_OUTPUT_OTHER 1<<4
#define STOP_OUTPUT_PRINTER 1<<5
#define STOP_OUTPUT_RESIZE 1<<6

#define ALLOC_ARRAY( type, n ) ( type * ) XtMalloc( (n) * sizeof( type ) )

#define ONE_PLANE( w ) ( (w)->common.graphics_mode == SINGLE_PLANE )

#endif _DECtermP_h
/* End of DECtermP.h */
