/*
 *  Title:	DT_sixel.h - DECterm Widget sixel definitions
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © 1988, 1993                                                 |
 *  | By Digital Equipment Corporation, Maynard, Mass.                       |
 *  | All Rights Reserved.                                                   |
 *  |                                                                        |
 *  | This software is furnished under a license and may be used and  copied |
 *  | only  in  accordance  with  the  terms  of  such  license and with the |
 *  | inclusion of the above copyright notice.  This software or  any  other |
 *  | copies  thereof may not be provided or otherwise made available to any |
 *  | other person.  No title to and ownership of  the  software  is  hereby |
 *  | transfered.                                                            |
 *  |                                                                        |
 *  | The information in this software is subject to change  without  notice |
 *  | and  should  not  be  construed  as  a commitment by Digital Equipment |
 *  | Corporation.                                                           |
 *  |                                                                        |
 *  | DIGITAL assumes no responsibility for the use or  reliability  of  its |
 *  | software on equipment which is not supplied by DIGITAL.                |
 *  +------------------------------------------------------------------------+
 *  
 *  Module Abstract:
 *
 *	Definitions for sixel emulation in DECterm Widget
 *
 *  Author:	Bob Messenger
 *
 *  Modification history:
 *
 *  Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 * Eric Osman		 4-Feb-1993
 *	- Add .pixel_count to keep track of width of current overlay.
 *
 * Eric Osman		19-Aug-1992	VXT V1.2
 *	- Add raster_change_ok to keep up with vms sixel bug fixes
 *
 * Aston Chan		17-Dec-1991	V3.1
 *	- I18n code merge
 *
 * Bob Messenger	23-May-1991	X3.0
 *	- Support overlay mode.
 *
 * Bob Messenger	 7-Apr-1989	X2.0-6
 *	- Removed single_plane, use the ONE_PLANE macro instead.
 *
 * Bob Messenger	31-Mar-1989	X2.0-5
 *	- Took out sixel_colors and next_color since we now use a modulo
 *	  function to map sixel colors to ReGIS colors.
 *
 * Bob Messenger	13-Jan-1989	X1.1-1
 *	- Add number parsing fields and macro
 */

/* writing modes */

#define REPLACE_MODE 0		/* write both foreground and background */
#define OVERLAY_MODE 1		/* write foreground only */

/* parsing states */

#define PROCESS_SIXEL 0		/* ready to parse a sixel character */
#define PROCESS_NUMBER 1	/* parsing a number */
#define PROCESS_REPEAT 2	/* parsing a repeat count */
#define PROCESS_RASTER 3	/* parsing a set raster attributes command */
#define PROCESS_RASTER_2 4	/* parsing aspect ratio denominator */
#define PROCESS_RASTER_3 5	/* parsing raster width */
#define PROCESS_RASTER_4 6	/* parsing raster height */
#define PROCESS_COLOR 7		/* parsing a color specifier */
#define PROCESS_RGBFLAG 8	/* parsing the RGB/HLS specifier */
#define PROCESS_RED 9		/* parsing the red color component */
#define PROCESS_GREEN 10	/* parsing the green color component */
#define PROCESS_BLUE 11		/* parsing the blue color component */
#define PROCESS_HUE 12		/* parsing the hue color component */
#define PROCESS_LIGHTNESS 13	/* parsing the lightness color component */
#define PROCESS_SATURATION 14	/* parsing the saturation color component */

/* number status values */

#define NO_NUMBER 0		/* no digits were seen */
#define GOOD_NUMBER 1		/* a number was seen between 0 and 32767 */
#define BIG_NUMBER 2		/* a number was seen but it was too big */

#define PARSE_NUMBER( next_state )					\
	{								\
	ld->sixel.number_value = 0;					\
	ld->sixel.number_next_state = next_state;			\
	ld->sixel.state = PROCESS_NUMBER;				\
	ld->sixel.number_status = NO_NUMBER;				\
	}

typedef struct
    {
    int state,			/* parsing state; see values above */
	x_origin,		/* origin of drawable area in pixels */
	y_origin,		/*   (x and y components) */
	width,			/* width of drawable area in pixels */
	height,			/* height of drawable area in pixels */
	wbyte,			/* width of each line of bitmap in bytes */
	size,			/* size of bitmap in bytes */
	byte,			/* offset of current byte within bitmap row */
	mask,			/* mask with current bit in bitmap */
	repeat,			/* repeat count for current sixel */
	pixel_count,		/* how many pixels wide current overlay is */
	multiplier,		/* multiplier for current repeat count */
	mode,			/* REPLACE_MODE or OVERLAY_MODE */
	aspect_numerator,	/* numerator of parsed aspect ratio */
	aspect_denominator,	/* denominator of parsed aspect ratio */
	horizontal_extent,	/* width of sixel area */
	vertical_extent;	/* height of sixel area */
    char *data,			/* address of start of bitmap */
	*row;			/* address of start of bitmap row */
    int color,			/* current sixel color */
	rgb_flag,		/* True if color is given in RGB form */
	red,			/* red component, 0 to 100 */
	green,			/* green component, 0 to 100 */
	blue,			/* blue component, 0 to 100 */
	hue,			/* hue component, 0 to 360 */
	lightness,		/* lightness component, 0 to 100 */
	saturation;		/* saturation component, 0 to 100 */
    int number_value,		/* value of number being parsed */
	number_status,		/* status of number being parsed; see above */
	number_next_state;	/* state to return to when number is parsed */
    GC gc,			/* graphics context for drawing sixels */
	bs_gc;			/* graphics context for backing store */
    Pixmap pixmap,		/* pixmap for overlay mode */
	bs_pixmap;		/* backing store pixmap for overlay mode */
    GC pixmap_gc,		/* graphics context for writing to pixmap */
	bs_pixmap_gc;		/* gc for writing to bs_pixmap */
    int pixmap_width,		/* width of pixmap */
	pixmap_height,		/* height of pixmap */
	bs_pixmap_width,	/* width of bs_pixmap */
	bs_pixmap_height,	/* height of bs_pixmap */
	one_plane_image;	/* True means image is one plane deep */
    int raster_change_ok;       /* Can I change raster aspect ratio ? */
    } SixelData;
