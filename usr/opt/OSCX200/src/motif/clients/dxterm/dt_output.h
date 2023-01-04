/*
 *  Title:	DT_output.h
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © 1988,1993                                                  |
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
 *	<short description of module contents>
 *
 *  Procedures contained in this module:
 *
 *	<list of procedure names and abstracts>
 *
 *  Author:	<original author>
 *
 *  Modification history:
 *
 *  Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 * Aston Chan		17-Dec-1991	V3.1
 *	- I18n code merge
 *
 * Bob Messenger	15-Sep-1990	X3.0-7
 *	- Add support for GS fonts.
 *
 * Bob Messenger	17-Jul-1990	X3.0-5
 *	Merge in Toshi Tanimoto's changes to support Asian terminals:
 *	- definitions of masks for characater set bits (gc_mask)
 *
 * Bob Messenger	24-Apr-1990	X3.0-2
 *	- Remove special gc_mask and font_index bits for condensed fonts, since
 *	  these are now treated the same way as normal fonts.
 *
 * Bob Messenger	13-Aug-1989	X2.0-19
 *	- New style flow control, using exposure events from XCopyArea.
 *
 * Bob Messenger	 5-Aug-1989	X2.0-18
 *	- Added color_pixel_allocated, to clean up color text support.
 *
 * Bob Messenger	22-May-1989	X2.0-12
 *	- Changeover to simplified occluded scrolling.
 *
 * Bob Messenger	12-Apr-1989	X2.0-6
 *	- Added quasi-"DRCS" font support.
 *
 * Bob Messenger	 4-Apr-1989	X2.0-5
 *	- Added ANSI color text support.
 *
 * Bob Messenger	22-Mar-1989	X2.0-3
 *	- Added APL font support.
 *
 * Bob Messenger	12-Sep-1988	X1.1-1
 *	- increase scroll_queue size to 48 to help prevent overflow
 *
 * Bob Messenger	 6-Sep-1988	X0.5-1
 *	- add scroll_queue ring list to fix scrolling exposure bug, and
 *	  take out exposure caching
 *
 * Tom Porcher		12-Aug-1988	X0.4-44
 *	- changes for new interpretation of foreground/background:
 *	    - replaced BLACK_TEXT_GC_MASK with REVERSE_TEXT_GC_MASK.
 *	    - removed text_foreground and text_background.
 *	    - removed black_border_gc and white_border_gc.
 */


/* definitions used by DECterm output module */

#ifndef _DT_OUTPUT_H
#define _DT_OUTPUT_H

#define X_MARGIN 4
#define Y_MARGIN 4

#define SCROLL_BAR_WIDTH 19	/* 2 more than usual to account for border */
#define SCROLL_BAR_HORIZONTAL_INCREMENT 1
#define SCROLL_BAR_VERTICAL_INCREMENT 1

#define STATUS_LINE_HEIGHT (3 + w->output.char_height)

#define NUMBER_OF_MARKERS 8

#define NUM_TEXT_COLORS 8	/* number of ANSI text colors */

#define NO_COLOR (-1)	/* flag for a pixel not being allocated */

/* indices into font_list */

#define FONT_NORMAL			0

/* the other definitions aren't actually needed, and some exceed 31 chars */

#define FONTS_PER_SET 128

/* indices into text_gc */

#define REVERSE_TEXT_GC_MASK		1
#define BOLD_TEXT_GC_MASK		2
/* Bit 7-4 is used as value, so at most 16 fonts can be defined */
#define DOUBLE_WIDE_TEXT_GC_MASK	4
#define DOUBLE_SIZE_TEXT_GC_MASK	8
#define TECH_TEXT_GC_MASK		16
#define APL_TEXT_GC_MASK		32
#define DRCS_TEXT_GC_MASK		64
#define ROMAN_TEXT_GC_MASK		128	/* 10000000, kanji */
#define KANJI_TEXT_GC_MASK		144	/* 10010000, kanji */
#define HANZI_TEXT_GC_MASK		160	/* 10100000, hanzi */
#define HANGUL_TEXT_GC_MASK		176	/* 10110000, hangul */
#define KS_ROMAN_TEXT_GC_MASK		192	/* 11000000, hangul */
#define HANYU_TEXT_GC_MASK		208	/* 11010000, hanyu */
#define HANYU_4_TEXT_GC_MASK		224	/* 11100000, hanyu */
#define HEB_TEXT_GC_MASK		240	/* 11110000, hebrew */
#define CHAR_SET_GC_MASK		0xf0
#define NUM_TEXT_GCS 256

typedef struct
    {
    int top_line;		/* top line number in logical display */
    int bottom_line;		/* bottom line number in logical display */
    int initial_line;		/* top line in display list (one past last
				   line in transcript) */
    int top_visible_line;	/* top line number in window */
    int columns;		/* number of columns in logical display */
    int left_visible_column;	/* leftmost character cell in window */
    int visible_rows;		/* number of lines in window */
    int visible_columns;	/* number of characters visible per line */
    long status;		/* status bits, see below */
    XtIntervalId blink_id;	/* timer for cursor and character blinker  */
    char char_blink_phase;	/* phase of blinking characters.  B0 says  */
				/* whether it's time to reverse them yet.  */
				/* B1 says which phase to draw them as.	   */
    Boolean blink_set_flag;	/* on if blink timer is set */
    Boolean some_blink_flag;	/* some blinking characters exist */
    GC normal_gc;		/* graphics context for drawing borders etc */
    GC reverse_gc;		/* graphics context for erasing borders etc */
    GC copy_area_gc;		/* graphics context for copying areas */
    GC copy_area_graphics_expose_gc;	/* graphics context for copying areas
				   with graphics exposures set for flow
				   control */
    GC complement_gc;		/* graphics context for complementing */
    GC text_gc[ NUM_TEXT_GCS ];	/* graphics contexts for drawing text, or
    				   0 if not created */
    Pixel color_pixels[ NUM_TEXT_COLORS ];
				/* -1 if color not allocated, else pixel */
    Boolean color_pixel_allocated[ NUM_TEXT_COLORS ];
				/* TRUE means the color was specifically
				   allocated (not just part of a color map) */
    Pixel foreground_pixel[ NUM_TEXT_GCS ];
				/* last foreground pixel set in GC */
    Pixel background_pixel[ NUM_TEXT_GCS ];
				/* last background pixel set in GC */
    Boolean gc_font_valid[ NUM_TEXT_GCS ];
    XFontStruct	*font_list[FONTS_PER_SET];
				/* fonts in current font set, or 0 if not
				   loaded */
    int status_line_top;	/* status line location in pixels */
    int char_width;		/* character width in pixels */
    int char_height;		/* character height in pixels */
    int char_ascent;		/* character ascent above baseline */
    int scroll_count;		/* number of scroll requests since last
				   sync */
    unsigned long sync_serial;	/* serial number for flow control: next
				   exposure event expected */
    unsigned long sync_serial2;	/* serial number for flow control: the
				   XCopyArea that blocked output */
    int cursor_row;		/* cursor line number */
    int cursor_column;		/* cursor column number */
    Bool cursor_double;		/* True if cursor is double wide */
    int cursor_left;		/* left cursor column in standard characters */
    int cursor_right;		/* right cursor column in standard characters */
    struct scroll_state		/* resources last set in scroll bars */
	{
	Widget widget;		/* widget context block */
	int value;		/* value resource  */
	int maxValue;		/* maxValue resource */
	int minValue;		/* minValue resource */
	int shown;		/* shown resource */
	} h_scroll, v_scroll;	/* horizontal and vertical scroll bars */
    int marker_is_set;		/* bit array: True means the corresponding
				   marker was set */
    int marker_row[ NUMBER_OF_MARKERS ];
				/* text line number for each marker */
    int lines_scrolled_up;	/* net lines scrolled up for pending
				   graphics exposures */
    int lines_scrolled_down;	/* net lines scrolled down for pending
				   graphics exposures */
    int first_scroll;		/* lines scrolled up or down for first
				   request made since idle state */
    int scroll_direction;	/* positive means all scrolls since idle
				   state have been up, negative means
				   down, zero means horizontal or mixed */
    int exposures_pending;	/* number of graphics exposure events
				   that we're waiting for */
    int real_char_height;	/* real character height in pixels used in
				   Kanji ReGIS emulation */
    Pixmap pixmap;		/* simulate double height/width adecw font */
    GC pixmap_gc;
    } OutputData;

/* status bits */

#define INITIAL_LINE_SET	0x1	/* TRUE if o_set_initial_line called */
#define WINDOW_INITIALIZED	0x2	/* TRUE if both initial line set and
					   window is realized */
/* removed duplicate OUTPUT_DISABLED */
#define CURSOR_IS_VISIBLE	0x4	/* TRUE if cursor was drawn */
#define OUTPUT_DISABLED		0x8	/* TRUE means don't draw anything */
#define TOP_LINE_SET		0x10	/* TRUE means o_set_top_line called */
#define BOTTOM_LINE_SET		0x20	/* TRUE means o_set_bottom_line
					   called */
#define BIG_FONT_SET_NAME_ALLOCATED 0x40 /* TRUE if space has been allocated
					   for the big font set name (will be
					   true after first call to
					   o_set_value_bigFontSetName */
#define LITTLE_FONT_SET_NAME_ALLOCATED 0x80 /* TRUE if space has been allocated
					   for the little font set name (will be
					   true after first call to
					   o_set_value_littleFontSetName */
#define SCROLL_ADJUST_IN_PROGRESS 0x100	/* TRUE if adjust_scroll_bars
					   generated scroll bar change */

#define USER_RESIZE_IN_PROGRESS 0x200	/* TRUE if changing rows and columns
					   because user resized window */

#define UPDATE_IN_PROGRESS 0x400	/* TRUE means don't refresh nulls */

#define UPDATE_STATUS_LINE 0x800	 /* true means update the status line
					    during an expose event */

#define DISABLE_REDISPLAY_CONTROL 0x1000 /* TRUE means don't redraw display
					   until o_adjust_display is called;
					   c_set_values in progress */

#define DISABLE_REDISPLAY_WINDOW 0x2000 /* TRUE means don't redraw display
					   until o_adjust_display is called;
					   auto-resize window in progress */

#define DISABLE_REDISPLAY_TERMINAL 0x4000 /* TRUE means don't redraw display
					  until o_adjust_display is called;
					  auto-resize terminal in progress */

#define DISABLE_REDISPLAY ( DISABLE_REDISPLAY_CONTROL |		\
	DISABLE_REDISPLAY_WINDOW |				\
	DISABLE_REDISPLAY_TERMINAL )	/* all bits must be clear in order
					   to redraw the display */

#define DISABLE_REFRESH 0X8000		/* TRUE means ignore exosure events */

#define REDRAW_DISPLAY 0x10000		/* TRUE means redraw display the next
					   time o_adjust_display is called */

#define RESIZE_SCROLL_BARS 0x20000	/* TRUE means move and resize the
					   scroll bars the next time
					   o_adjust_display is called */

#define COMPUTE_OPTIMAL_SIZE 0x40000	/* TRUE means call compute_optimal_size
					   the next time o_adjust_display
					   is called */

#define HORIZONTAL_SCROLL_BAR_MAPPED 0x80000 /* TRUE means the horizontal
					   scroll bar is mapped */

#define VERTICAL_SCROLL_BAR_MAPPED 0x100000 /* TRUE means the vertical scroll
					   bar is mapped */

#define CURSOR_DISABLED_RESOURCE 0x200000   /* TRUE means cursor disabled
					   because textCursorEnabled resource
					   is off */

#define CURSOR_DISABLED_DCS	0x400000    /* TRUE means cursor disabled
					   because in device control string */

#define CURSOR_DISABLED_SOURCE	0x800000    /* TRUE means cursor disabled
					   by DwtDECtermPutData */

#define PARTIALLY_OBSCURED	0x1000000   /* TRUE means window is partially
					   obscured by another window */

#define CURSOR_DISABLED ( CURSOR_DISABLED_RESOURCE | CURSOR_DISABLED_DCS \
    | CURSOR_DISABLED_SOURCE )		/* TRUE means don't draw cursor */

#define STATUS_LINE_VISIBLE	0x2000000	/* TRUE means status line
					   is actually visible, not just
					   enabled */
#define FINE_FONT_SET_NAME_ALLOCATED 0x4000000 /* TRUE if space has been allocated
					   for the fine font set name (will be
					   true after first call to
					   o_set_value_fineFontSetName */

#define GS_FONT_SET_NAME_ALLOCATED 0x8000000
					/* TRUE if space has been allocated for
					   the GS font set name */
#endif
