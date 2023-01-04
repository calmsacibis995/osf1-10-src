/* #module dt_output "X3.0-7" */
/*
 *  Title:	DECterm Output Module
 *
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © Digital Equipment Corporation, 1988, 1993 All Rights	     |
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
 *	The DECterm widget includes the following components: control,
 *	source, input, and output.
 *
 *	The output component is entirely responsible for rendering the text
 *	in the window.
 *
 *  Procedures contained in this module:
 *
 *	o_inititialize		initialize the output component
 *	o_reconfigure		process reconfiguration event
 *	o_expose		process exposure event
 *	o_focus_in		called when focus received
 *	o_focus_out		called when focus relinquished
 *	o_realize		called when a new widget instance is created
 *	o_event_handler		output event handler
 *	o_set_initial_line	set initial top visible line number
 *	o_set_top_line		set top line number in backing store
 *	o_set_bottom_line	set bottom line number in backing store
 *	o_set_marker		set a marker between lines
 *	o_clear_marker		clear a marker between lines
 *	o_set_cursor_position	set output cursor position
 *	o_display_segment	display part of a line of text
 *	o_scroll_lines		scroll lines on the display
 *	o_erase_lines		erase one or more lines on the display
 *	o_clear_comm		reset flow control
 *	o_set_value_foreground	called when foreground resource changes
 *	o_set_value_background	called when background resource changes
 *	o_set_value_reverseVideo  called when reverseVideo resource changes
 *	o_set_value_cursorStyle	called when cursorStyle resource changes
 *	o_set_display_width	called to change the width of the display
 *	o_destroy		called when the widget is destroyed
 *	o_convert_XY_to_position
 *				convert pixels to character cells
 *	o_convert_XY_to_pixels	convert display pixels to source pixels
 *	o_convert_position_to_XY
 *				convert character cells to pixels
 *	o_display_region	display a region of text
 *	o_update_rectangle	refresh a rectangle from display list.  Update
 *				it to the window or pixmap parameter if later
 *				is not NULL
 *
 *  Author:	Robert Messenger
 *
 *  Modification history:
 *
 * Alfred von Campe     18-Dec-1993     BL-E
 *	- Change all occurrances of common.foreground to manager.foreground.
 *
 * Alfred von Campe     01-Dec-1993     BL-E
 *	- Add Bob's ANSI color text fix from V1.1 patch kit.  This fixes the
 *        problems with reverse ANSI color text that Aston tried to fix on
 *        15-Dec-1992, but which introduced another bug.  Reverse color should
 *        not depend on the previous segment.
 *
 * Alfred von Campe     04-Nov-1993     V1.2/BL3
 *	- Add Bob's workaround for Alpha compiler bug (ALPHA_FINE_FONT_BUG).
 *
 * Alfred von Campe     02-Nov-1993     BL-E
 *	- Use 10 and 14 point fonts on 100 DPI monitors.
 *
 * Eric Osman		30-Aug-1993	BL-D
 *	- Turn off output when doing resize, since upon client event, we clear
 *	  regis mode, and hence we want to wait until after this event before
 *	  letting a regis string start to be parsed.
 *
 * Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 * Alfred von Campe     21-May-1993     V1.2
 *	- Changed wrong usage of "short int" to REND to support new REND size.
 *
 * Dave Doucette	 7-APR-1993	V1.2/BL2
 *	- Added code for expose event routine to check for Regis rubberband
 *	  cursor refresh.
 *
 * Aston Chan		12-Mar-1993	V1.2/BL2
 *	- Fix the *life long* problem of scroll back couple of pages and
 *	  hit return will make current page off by 1 line.
 *	- Add Turkish/Greek support.  Changes include :
 *	  o set v1_encoding to False
 *	  o handle correct Supplemental Character Sets
 *	  o open correct fonts
 *
 * Aston Chan		15-Dec-1992	Post V3.1/SSB
 *	- Fix cld of reverse ansi color not working correctly.  Fix is to
 *	  remember the last changed foreground and background color in previous
 *	  segment and swap them when reverse sequence is sent.
 *
 * Alfred von Campe     07-Oct-1992     Ag/BL10
 *      - Added type casts to satisfy Alpha compiler and 64-bit cleanliness.
 *
 * Eric Osman		24-Jul-1992	VXT V12
 *	- Prevent crashes by using strncpy instead of strcpy to make sure we
 *	  don't overwrite our bounds on font name buffers.
 *
 * Aston Chan		29-May-1992	Post V3.1/SSB
 *	- If FAKE_VM is turned on, there is an error if we do 
 *	  mcr decw$terminal -number n  where 'n' is greater than 1.  Problem
 *	  is w->common.fontUsed not initialized in o_initialize().
 *
 * Alfred von Campe     26-May-1992     Post V3.1
 *      - Fix order of parameters in one DLFDSetFontNameField() call.
 *
 * Aston Chan		27-Mar-1992	V3.1/BL6
 *	- Merge in Tolung's fix of loading -*-dec-drcs fonts for Asian DECterm
 *
 * Alfred von Campe     13-Mar-1992     V3.1/BL6
 *      - Fix I18N problem where characters appear out of order (from To-lung).
 *      - Fix ULTRIX compiler problem with string initialization.
 *
 * Alfred von Campe     20-Feb-1992     V3.1
 *      - Add color text support.
 *
 * Aston Chan		08-Jan-1991	V3.1
 *	- Add static in front of noshare XtTranslations parsed_trans. 
 *	  Complained by VECTORTST.
 *
 * Eric Osman		23-Dec-1991	V3.1
 *	- Only create translation table for scrollbars once to avoid memory
 *	  leak.
 *
 * Aston Chan		19-Dec-1991	V3.1
 *	- I18n code merge
 *
 * Aston Chan		09-Oct-1991	V3.1
 *	- Fix the use of &font_name_list in one of the load_font().  Originally
 *	  the &font_name_resource is mistyped which causes AccVio when that
 *	  code is activated.
 *	- DECTERMV3 QAR #00045, #00023, #00024 or Print Graphics problem.
 *	  pixmap parameter is added to o_update_rectangle() and 
 *	  draw_segment() routines.  If this pixmap is not NULL then the
 *	  string/text will be drawn to pixmap instead of XtWindow(w).  The
 *	  o_update_rectangle() routine will be called by o_expose() to do the
 *	  update which means writing to pixmap is sharing same update code
 *	  with o_expose().
 *	- update_rectangle() is renamed to o_update_rectangle() and "static"
 *	  is removed from it.
 *	- Add XmCR_DRAG reason to scroll_callback_proc()'s case statement
 *	  in order to support "live scrolling".
 *
 * Aston Chan		01-Oct-1991	V3.1
 *	- QAR 0040 of MACAW QAR DB.  Clear lines off top can cause DECTerm
 *	  to exit.  Problem is call to XCopyArea() with a negative height.
 *	  Make sure non-negative height is passed.
 *
 * Alfred von Campe     14-Oct-1991     V3.1
 *	- Add a workaround for a toolkit bug that reports a negative
 *        core.height as a large positive number.
 *      - Merge Michele Lien's fix for the font fallback scheme which did not
 *        work corrrectly in open_font() and caused the VXT to crash if a bogus
 *        font name was supplied.
 *
 * Alfred von Campe     06-Oct-1991     Hercules/1 T0.7
 *      - Changed private #include file names to all lower case.
 *      - Added #extern definition for XmCreateScrollBar().
 *      - Added static to two functions.
 *
 * Michele Lien		3-Oct-1991	VXT V1.0
 *	- font fallback scheme did not work correctly in open_font(), which 
 *	  causes a crash when a bogus font name is entered.  This is discovered
 *	  when I implemented the customization UI for the font resources.
 *
 * Alfred von Campe     24-May-1991     V3.0
 *	- Remove help callbacks from the scrollbars so that we don't crash
 *        when we try to reference a NULL XmScrollBarCallbackStruct pointer
 *        in scroll_callback_proc().
 *
 * Eric Osman		27-Mar-1991	V3.0
 *	- Make so typing works even if mouse is on scroll bar.  Later, we'll
 *	want to revisit this when we implement scrolling keys (such as
 *	ctrl-next-screen key).
 *
 * Eric Osman		14-Jan-1991
 *	- Bob forgot to release GS font name in o_destroy.  Also, fontUsed
 *	  was never being freed.
 *
 * Bob Messenger	15-Sep-1990	X3.0-7
 *	- Add support for GS fonts.
 *
 * Bob Messenger	11-Sep-1990	X3.0-7
 *	- Initialize the STATUS_LINE_VISIBLE status bit correctly.
 *
 * Bob Messenger	10-Sep-1990	X3.0-7
 *	- If the family name is wild carded in the base font, wild card it
 *	  in the other fonts in the font set as well.  This fixes a problem
 *	  loading APL fonts.
 *
 *  Michele Lien    24-Aug-1990 VXT X0.0
 *  - Modify this module to work on VXT platform. Change #ifdef VMS to
 *    #ifdef VMS_DECTERM so that VXT specific code can be compiled under
 *    VMS or ULTRIX development environment.
 *
 * Bob Messenger	19-Jul-1990	X3.0-5
 *	- Change the default value for the bigFontSetName and littleFontSetName
 *	  resources at run time, depending on whether DECterm is displaying to
 *	  a 15 or 19 inch monitor.  This is so that DECterm can use an 18
 *	  point font on the 19 inch, 100 dpi monitor.
 *
 * Bob Messenger	17-Jul-1990	X3.0-5
 *	Merge in Toshi Tanimoto's changes to support Asian terminals:
 *	- two byte (double width) fonts drawing 
 *	- open fonts specific to Kanji terminal (Tomcat)
 *
 * Bob Messenger	13-Jul-1990	X3.0-5
 *	- Check for XLoadQuery font returning NULL.
 *
 * Bob Messenger	02-Ju1-1990	X3.0-5
 *	- Fix bug introduced in the 2-May-1990 font fallback change: for double
 *	  high fonts need to double point height and average width as well as
 *	  pixel height.
 *
 * Bill Matthews    June 1990
 * - Make ansi_color_entries globaldef
 *
 * Mark Woodbury	25-May-1990 X3.0-3M
 *  - Motif update
 *
 * Bob Messenger	 2-May-1990	X3.0-2
 *	- When we can't open a font, even after applying all fallbacks, call
 *	  widget_error with DECW$K_MSG_EXIT_NO_FONT instead of DECW$K_MSG_EXIT,
 *	  so applications using the DECterm widget won't have to link with
 *	  the DECterm message file.
 *
 * Bob Messenger	24-Apr-1990	X3.0-2
 *	- Use an improved font fallback strategy.
 *
 * Bob Messenger	22-Apr-1990	X3.0-2
 *	- Fix occluded scrolling problem.
 *
 * Bob Messenger	13-Apr-1990	X3.0-2
 *	- Fix problems of scrolling when there is a selection.
 *
 * Bob Messenger	05-Oct-1989	V2.0
 *	- Fix problem where the fontUsed resource is freed after being
 *	  allocated (because a set_value routine that calls open_font is
 *	  called before o_set_value_fontUsed).  Instead, move the code that
 *	  prevents fontUsed from being changed to c_set_values.
 *
 * Bob Messenger	11-Sep-1989	V2.0
 *	- Fix problem where the window freezes when scrolling and completely
 *	  occluded (because graphic expose handling was disabled when the
 *	  OUTPUT_DISABLED bit was set).
 *
 * Bob Messenger	23-Aug-1989	X2.0-20
 *	- Fix Ultrix compilation error (illegal character).
 *	- Resize scroll bars in o_reconfigure if they are enabled.
 *
 * Bob Messenger	13-Aug-1989	X2.0-19
 *	- New style flow control, using graphics exposures on XCopyArea.
 *
 * Bob Messenger	12-Aug-1989	X2.0-19
 *	_ Resize scroll bar when display size changes.
 *
 * Bob Messenger	 5-Aug-1989	X2.0-18
 *	- Fix up code for allocating and freeing ANSI colors.
 *	- Clear graphics before swapping foreground and background in
 *	  o_set_value_reverseVideo.
 *
 * Bob Messenger	23-Jul-1989	X2.0-16
 *	- Allow the status line to be redrawn after an expose event.
 *
 * Bob Messenger	22-Jun-1989	X2.0-15
 *	- Fix problem with double high font names, caused by the default
 *	  fonts match pixel size instead of point size.
 *
 * Bob Messenger	27-May-1989	X2.0-13
 *	- Call regis_resize_window when the window size changes.
 *	- Make the underscore cursor two pixels wide and put it at the
 *	  bottom of the character cell.
 *	- Use XInternAtom to find message codes, to avoid conflict with
 *	  selection ClientMessage's.
 *	- Don't wipe out markers when scrolling.
 *	- Replace bcopy with memcpy.
 *	- Add set_values procedure for fontUsed to prevent its value
 *	  from changing except from within the DECterm widget.
 *
 * Bob Messenger	22-May-1989	X2.0-12
 *	- Simplify the code to deal with occluded scrolling.  There is no
 *	  longer a scroll queue; instead we keep track of the number of lines
 *	  scrolled up and down for which there is no exposure event.  In
 *	  some cases this means that we redraw more than we really have to.
 *	- Change the baseline for the double high fonts, based on the
 *	  assumption that their ascent is twice the ascent of the single high
 *	  fonts, rather than the ascent plus the height.
 *	- Look at the maximum character number in the base font of the set
 *	  to see whether it's using the V1 character encodings (253 characters
 *	  in DEC Multinational) or the corrected V2 encodings.
 *
 * Bob Messenger	12-May-1989	X2.0-10
 *	- Convert XCopyArea to XClearArea if scrolling too many lines in
 *	  o_scroll_lines, to avoid X error.
 *	- Make MB2 and MB3 in scroll bars conform to style guide.
 * 	- Use widget_error or widget_message instead of printf.
 *
 * Bob Messenger	 8-May-1989	X2.0-10
 *	- Don't call regis_resize_terminal in o_adjust_display(), wait
 *	  for MESSAGE_REQUEST_RESIZE (avoids accvio, since otherwise we
 *	  try to access the condensed font before it's loaded when the
 *	  condensedFont and columns resources are changed in the same
 *	  XtSetValues call).
 *
 * Bob Messenger	18-Apr-1989	X2.0-6
 *	- Free fonts correctly.
 *
 * Bob Messenger	11-Apr-1989	X2.0-6
 *	- Call allocate_color_map instead of init_color_map.
 *	- Support quasi-"DRCS" font (i.e. a normal server font that the user
 *	  can create).
 *
 * Bob Messenger	11-Apr-1989	X2.0-6
 *	- Fix bug with selected text not being highlighted.
 *
 * Bob Messenger	 7-Apr-1989	X2.0-6
 *	- Convert to new widget bindings (DECw prefix).
 *
 * Bob Messenger	 7-Apr-1989	X2.0-6
 *	- Support variable number of bit planes.  Fix bug with reverse video.
 *
 * Bob Messenger	 4-Apr-1989	X2.0-5
 *	- Support ANSI color text.  Add normal_gc and reverse_gc
 *	  for use in cases where we want to be sure we have the actual
 *	  foreground and background colors and the font doesn't matter.
 *	- Don't allocate gc's until needed.
 *	- Free fonts that we've loaded.
 *
 * Bob Messenger	31-Mar-1989	X2.0-5
 *	- update the text cursor plane mask in o_set_foreground_color and
 *	  o_set_background_color.
 *
 * Bob Messenger	19-Mar-1989	X2.0-3
 *	- add APL support
 *
 * Bob Messenger	14-Mar-1989	X2.0-2
 *	- implement fontUsed resource
 *
 * Bob Messenger	18-Jan-1989	X1.1-1
 *	- regis_scroll_* now implies the erase rectangle
 *
 * Bob Messenger	16-Jan-1989	X1.1-1
 *	- Allow for negative computed display sizes (fixes QAR 61 in
 *	  DECTERM_V2).  The minimum number of rows is 1, even if we set
 *	  the window height to 0 and autoResizeTerminal is on.
 *
 * Bob Messenger	10-Jan-1989	X1.1-1
 *	- Preserve bottom visible line, not top visible line, when shrinking
 *	  window
 *
 * Bob Messenger	09-Jan-1989	X1.1-1
 *	- Don't write to status line unless it's actually visible (not just
 *	  enabled)
 *
 * Bob Messenger	09-Dec-1988	X1.1-1
 *	- Fix scroll_queue to adjust properly to panning (can't just put in
 *	  a dummy entry, have to put in real values)
 *
 * Eric Osman		27-Octo1988	X0.5-5
 *	- Fix scrolling to not crash server (was erroneously passing -15
 *	  as height)
 * Tom Porcher		18-Oct-1988	X0.5-4
 *	- disallow columns outside of the display to be returned by
 *	  o_convert_XY_to_position().  This fixes the bug where you can
 *	  select a blank line and get junk.
 *	- Don't do vertical cursor coupling into the STATUS_LINE!
 *	- Fix refresh test to include status line.
 *
 * Eric osman		30-Sep-1988	X0.5-1
 *	- add ampersand on font_info in open_font to fix memory corruption
 *	- use non-info flavor of XloadFont to avoid suspected xlib bug
 *	  whereby XloadFontsWithInfo neglects to add 1 to address to prepare
 *	  for XfreeFontNames subtracting 1.
 *
 * Bob Messenger	 6-Sep-1988	X0.5-1
 *	- implement scroll_queue ring list to fix scrolling exposure bug, and
 *	  take out exposure caching
 *
 * Tom Porcher		12-Aug-1988	X0.4-44
 *	- changes to allow foreground and background reflect real
 *	  foreground and background.  Basically, the core.background_pixel
 *	  common.foreground are always the text colors.  The screenMode
 *	  resource is now only historic, and a new resource, reverseVideo,
 *	  is a one-shot resource which when set to TRUE will swap the
 *	  current fg and bg colors.  Specific changes:
 *		- BLACK_TEXT_GC_MASK is now REVERSE_TEXT_GC_MASK, and
 *		  its meaning is reversed.
 *		- black/white_border_gcs are no longer used.
 *		- output.text_foreground/background have been replaced
 *		  with common.foreground and core.background_pixel.
 *		- removed o_set_value_screenMode() and added
 *		  o_set_value_reverseVideo().
 *	- the special NULL handler in display_segment() was neglecting the
 *	  last four pixels.  This was because the test did not take into
 *        account the borders are not included in display_width.  I fixed
 *	  the test, and also made it check for the left edge to correct
 *	  the problems with panning double-width lines.
 *
 * Mike Leibow		08-Aug-1988	X0.4-42
 *	- fixed the scrollbars so they are always the right size.
 *
 * Tom Porcher		 2-Aug-1988	X0.4-41
 *	- made compute_optimal_size() be called even if DISABLE_REDISPLAY
 *	  is set.  This is necessary in case multiple changes occur
 *	  to the size variables in a single call, such as in the DECCOLM
 *	  sequence.
 *
 * Tom Porcher		27-Jul-1988	X0.4-41
 *	- made compute_optimal_size() be called even before window is
 *	  initialized, so the size can be computed based on configuration
 *	  file resources and customization strings.
 *
 * Eric Osman		27-May-1988	X0.4-28
 *	- Use XtRemoveTimeOut instead of vms ast
 *
 * Tom Porcher		26-May-1988	X0.4-28
 *	- change name of dlfd.h to DLFD.h as it is in DLFD.c.
 *
 * Eric Osman		16-May-1988	X0.4-27
 *	- Change code to understand that blink timer isn't always on
 *
 * Tom Porcher		 5-Apr-1988	X0.4-10
 *	- Change o_create() to o_realize().
 *
 * Tom Porcher		 5-Apr-1988	X0.4-7
 *	- Added "region" parameter to o_expose().
 *	- Removed one callback from DwtScrollBar().
 *
 * Eric Osman		01-Apr-1988	X0.4
 *	- Add cursor blink code
 *
 * Tom Porcher		18-Mar-1988	X0.4
 *	- Added routine o_convert_position_to_XY().
 *
 * Bob Messenger	11-Jan-1988	X0.4
 *	- compute_optimal_size does a XSendEvent to request a resize instead
 *	  calling the resize callback directly; this avoids a bug where the
 *	  the window is resized incorrectly when scoll bars are enabled or
 *	  disabled.
 *
 * Tom Porcher		31-Dec-1987	X0.3
 *	- changed o_return_optimal_size() to compute_optimal_size(); there
 *	  are no longer any external callers.
 *	- Added o_destroy().
 *	- changed resizeCallback() to use XtCallCallbacks
 *	- Changed all o_set_value_xxx routines to new parameters.
 *	- resizeCallback now only called if widget realized, to prevent being
 *	  called before initialization is complete.
 *
 * Tom Porcher		23-Dec-1987	X0.3
 *	- Changed routines:
 *		o_convert_to_source_cells() --> o_convert_XY_to_position()
 *		    Now returns a DECtermPosition.
 *		o_convert_to_source_pixels() --> o_convert_XY_to_pixels()
 *		    Name change only.
 *	- Added o_display_region().
 *	- Changed draw_segment() to account for highlighted region.
 *
 *  R. Messenger	9-Sep-1987
 *	Module created.
 *
 */


#include "dectermp.h"

#include "dlfd.h"
#include "regstruct.h"

/* conditional compilation */

#if defined(ALPHA)
#define ALPHA_FINE_FONT_BUG	/* o_set_value_fineFontSetName bug hangs on VMS
				   DECterm with DEC C T1.1-048H-248T compiler */
#endif

#define DEBUG_DT_OUTPUT		/* if defined, can turn sync on/off */

#ifdef DEBUG_DT_OUTPUT
static DECtermData *output_widget;
#endif

#ifdef MEASURE_KEYBOARD_PERFORMANCE

#include timeb
int key_enabled, key_min, key_max, key_total, key_num;
struct timeb key_time;
char key_char;
#define KEYBOARD_ECHO 3
#endif

/* macro definitions */

/* macro max returns the larger of two integers */

#ifndef max
#define max(a,b) ( ( (a) > (b) ) ? (a) : (b) )
#endif

/* macro min returns the smaller of two integers */

#ifndef min
#define min(a,b) ( ( (a) < (b) ) ? (a) : (b) )
#endif

/* read-only data */

#ifdef VMS_DECTERM
globaldef readonly
#endif
#ifdef VXT_DECTERM
globaldef
#endif VXT_DECTERM

int ansi_color_entries[] = { 5, 10, 11, 14,
				    9, 12, 13, 6 };

/* external functions used by this module */
extern s_line_structure *s_read_data();  /* DT_source_subs.c	*/
extern int s_execute_deferred();	 /* DT_source_subs.c	*/
extern regis_cleanup_rband_expose();	 /* reg_ReGIS.c		*/
extern Widget XmCreateScrollBar();

/* externally callable functions */

void o_reconfigure(), o_expose(), o_realize(), o_set_initial_line(),
    o_display_cursor(), o_erase_cursor(), o_focus_in(), o_focus_out(),
    o_set_top_line(), o_set_bottom_line(), o_set_marker(), o_clear_marker(),
    o_set_cursor_position(), o_display_segment(), o_scroll_lines(),
    o_erase_lines(), o_clear_comm(), o_set_value_reverseVideo(),
    o_set_value_fontSetSelection(), o_set_value_bigFontSetName(),
    o_set_value_littleFontSetName(), o_set_value_cursorStyle(),
    o_set_value_bigFontOtherName(), o_set_value_littleFontOtherName(),
    o_set_value_gsFontOtherName(),
    o_set_display_width(), o_destroy(), o_set_value_gsFontSetName(),
    o_convert_XY_to_pixels(), o_convert_position_to_XY(),
    o_set_value_scrollHorizontal(), o_set_value_scrollVertical(),
    o_set_value_foreground(), o_set_value_background(),
    o_set_foreground_color(), o_set_background_color(), o_update_segment(),
    o_adjust_display(), o_disable_redisplay(), o_enable_redisplay(),
    o_set_value_fineFontSetName(),
    o_draw_intermediate_char(),
    o_update_rectangle(),
    o_set_value_useBoldFont(),
    o_set_value_textCursorEnable(), o_disable_cursor(), o_enable_cursor();

DECtermPosition o_convert_XY_to_position();

/* local procedures */

static void
    o_event_handler(),		/* event handler registered by o_realize */
    initialize_window(),	/* initialize window stuff */
    calculate_visible_area(),	/* compute what character cells are visible */
    erase_old_cursor(),		/* erase the cursor using previous style */
    complement_cursor(),	/* actually display or erase the cursor */
    repaint_border(),		/* redraw the border */
    repaint_left_border(),
    repaint_right_border(),
    clear_border(),		/* clear the border */
    erase_border(),		/* erase the border */
    draw_border(),		/* draw the border with a stated GC (color) */
    scroll_callback_proc(),	/* callback procedure for scroll bars */
    pan_horizontal(),		/* scroll the display horizontally */
    pan_vertical(),		/* scroll the display vertically */
    adjust_scroll_bars(),	/* update horizontal & vertical scroll bars */
    update_segment(),		/* refresh a line segment from display list */
    draw_segment();		/* actually display or update a line segment */


static Pixel
    get_ansi_color();

Bool
    row_is_marked();		/* returns True if a given row is markeed */

static void
    complement_row_marker();	/* draw or erase a marker */

XFontStruct
    *load_font();		/* load a font given a pattern that matches */

static int debug_flag=0;	/* True means synchronize X calls */

/*

o_initialize - initialize the output component

This routine is called during widget initialization to initialize the output
fields in the context block.  In particular it initializes the standard
character size fields for compute_optimal_size, and it initializes the
window background and border.

Input fields in context block:
	common.reverseVideo		True means reverse video screen

Output fields in context block:
	core.border_pixel		color of window border
	core.background_pixel		color of window background
	output.char_width		width of standard character cell
	output.char_height		height of standard character cell
	output.char_ascent		offset to baseline in character cell
	output.status			status bits (set to OUTPUT_DISABLED,
					  will be cleared when window realized)

*/

o_initialize( w )
    DECtermData *w;		/* widget context block */
{
    int i;

#ifdef DEBUG_DT_OUTPUT
    output_widget = w;
#endif

    w->output.status = OUTPUT_DISABLED;
    w->output.columns = w->common.columns;
    w->output.lines_scrolled_up = 0;
    w->output.lines_scrolled_down = 0;
    w->output.first_scroll = 0;
    w->output.scroll_direction = 0;
    w->output.exposures_pending = 0;

    w->output.scroll_count = 0;
    w->output.sync_serial = 0;
    w->output.sync_serial2 = 0;

    w->common.fontUsed = NULL;

/* initialize markers */

    w->output.marker_is_set = 0;

    w->common.original_foreground_color = w->manager.foreground;
    w->common.original_background_color = w->core.background_pixel;

/* initialize colors */

    for ( i = 0; i < NUM_TEXT_COLORS; i++ )
	{
	w->output.color_pixels[ i ] = NO_COLOR;
	w->output.color_pixel_allocated[ i ] = FALSE;
	}

/* mark all fonts as unloaded */

    for ( i = 0; i < FONTS_PER_SET; i++ )
	w->output.font_list[i] = NULL;

/* initialize gcs */

    for ( i = 0; i < NUM_TEXT_GCS; i++ )
	w->output.text_gc[ i ] = 0;		/* means not created */

    if ( w->common.fontUsed != NULL )		/* re-allocate if resource value */
	w->common.fontUsed = XtNewString( w->common.fontUsed );

    if ( w->common.bigFontOtherName != NULL )
	w->common.bigFontOtherName = XtNewString(w->common.bigFontOtherName);
    if ( w->common.littleFontOtherName != NULL )
	w->common.littleFontOtherName = XtNewString(w->common.littleFontOtherName);
    if ( w->common.gsFontOtherName != NULL )
	w->common.gsFontOtherName = XtNewString(w->common.gsFontOtherName);

    o_set_value_bigFontSetName( w, w ); /* this also computes optimal size */
    o_set_value_littleFontSetName( w, w );
    o_set_value_gsFontSetName( w, w );
    o_set_value_fineFontSetName( w, w );
    w->output.pixmap = NULL;
    w->output.pixmap_gc = NULL;

/* initialize blink timer for this terminal */

    w->output.blink_set_flag = 0;
    w->output.char_blink_phase = 0;
    w->output.some_blink_flag = 0;
}

/*

compute_optimal_size - find the optimum size for the terminal widget

Input fields in context block:
	output.columns		logical display width in character cells
	common.rows		logical display height in character cells
	common.scrollHorizontal	horizontal scroll bar enable
	common.scrollVertical	vertical scroll bar enable
	output.char_width	standard character width in pixels
	output.char_height	standard character height in pixels

*/

void compute_optimal_size( w )
    DECtermData *w;		/* widget context block */
{
    int width, height;

    width = w->output.columns * w->output.char_width + X_MARGIN * 2;
				/* +2 is a kludge... */
    if ( w->common.scrollVertical )
	width += SCROLL_BAR_WIDTH;
    height = w->common.rows * w->output.char_height + Y_MARGIN * 2;
				/* +2 is a kludge... */
    if ( w->common.scrollHorizontal )
	height += SCROLL_BAR_WIDTH;

    if ( w->common.statusDisplayEnable )
	height += STATUS_LINE_HEIGHT;

    if ( width != w->common.displayWidth || height != w->common.displayHeight
      || w->output.char_width != w->common.displayWidthInc
      || w->output.char_height != w->common.displayHeightInc )
	{
	XEvent event;
	w->common.displayWidth = width;
	w->common.displayHeight = height;
	w->common.displayWidthInc = w->output.char_width;
	w->common.displayHeightInc = w->output.char_height;
	if ( (w->output.status & WINDOW_INITIALIZED)
		&&
	     !(w->output.status & DISABLE_REDISPLAY_WINDOW) )
	    {
	    event.type = ClientMessage;
	    event.xclient.display = XtDisplay(w);
	    event.xclient.window = XtWindow(w);
	    event.xclient.format = 32;
	    event.xclient.data.l[0] =
		XInternAtom( XtDisplay(w), "MESSAGE_REQUEST_RESIZE", FALSE );
	    XSendEvent( XtDisplay(w), XtWindow(w), True, NoEventMask, &event );
	    w->output.status |= DISABLE_REDISPLAY_WINDOW;
/*
 * Turn off output when doing resize, since upon client event, we clear
 * regis mode, and hence we want to wait until after this event before
 * letting a regis string start to be parsed.
 */
	    s_stop_output( w, STOP_OUTPUT_RESIZE ); 
	    }
	}                 
/* figure out where the status line belongs */
    if ( w->common.statusDisplayEnable ) {
      w->output.status_line_top = ( w->core.height
        - ( w->common.scrollHorizontal ? SCROLL_BAR_WIDTH : 0 )
        - ( w->common.statusDisplayEnable ? STATUS_LINE_HEIGHT : 0)
        + 1) ;
      /* status line has moved, so it needs to be redisplayed. */
      w->output.status |= REDRAW_DISPLAY;
    }
}

/*

o_reconfigure - process reconfiguration event

This procedure is called when any reconfiguration event occurs, such as a
window resize or move.  It doesn't refresh the display because it assumes
the toolkit will call o_expose after o_reconfigure returns.

Input fields in context block:
	core.width		new window width in pixels
	core.height		new window height in pixels

Output fields in context block:
	output.left_visible_column	leftmost character in window
	output.visible_columns		number characters visible in each line
	output.display_width		visible logical display width (pixels)
	output.top_visible_line		top line in window
	output.visible_rows		number of lines in window
	output.display height		visible logical display height (pixels)

*/

void o_reconfigure( w )
    DECtermData *w;	/* context pointer for widget */
{
    int row;		/* loop index for lines of characters */
    int rows, columns;
    Arg arglist[2];

/* make sure o_realize and o_set_initial_line have been called */

    if ( ! (w->output.status & WINDOW_INITIALIZED) )
	return;

/* need to move or resize the scroll bars if they are enabled */

    if ( w->common.scrollVertical || w->common.scrollHorizontal )
	w->output.status |= RESIZE_SCROLL_BARS;

/* if auto resize is enabled, resize the logical display */

    if ( w->common.autoResizeTerminal )
	{
	rows = w->core.height - ( w->common.scrollHorizontal ?
	  SCROLL_BAR_WIDTH : 0 ) - ( w->common.statusDisplayEnable ?
	  STATUS_LINE_HEIGHT : 0) - 2 * Y_MARGIN;

/* If rows (height in pizels) is greater than 65000 assume that w->core.height
   was supposed to be negative.  This is to fix the QAR where resizing a
   DECterm to less than the work area would make it "spring" to full size.
   This is a workaround for a toolkit bug (which I have QARed) */

	if ( rows < w->output.char_height || rows > 65000)
	    rows = 1;
	else
	    rows /= w->output.char_height;
	columns = w->core.width - ( w->common.scrollVertical ?
	  SCROLL_BAR_WIDTH : 0 ) - 2 * X_MARGIN;
	if ( columns < w->output.char_width )
	    columns = 1;
	else
	    columns /= w->output.char_width;
	if ( rows != w->common.rows || columns != w->common.columns )
	    {
	    w->output.status |= DISABLE_REDISPLAY_TERMINAL;
	    XtSetArg( arglist[0], DECwNrows, rows );
	    XtSetArg( arglist[1], DECwNcolumns, columns );
	    XtSetValues( (Widget)w, arglist, 2 );
	    w->output.status &= ~DISABLE_REDISPLAY_TERMINAL;
	    }
	}	
    o_adjust_display( w );
}

/*

o_expose - process exposure event

This procedure is called when an expose event occurs: this can happen when
part of the window is uncovered or when the source rectangle for a copy area
is partially obscured (the latter is called a graphics exposure event).  It
refreshes the rectangle that was exposed by calling the s_read_data routine
in the source component (via update_segment).

*/

void o_expose( w, event, region )
    DECtermData *w;	/* context pointer for widget */
    XEvent *event;	/* data associated with event */
    Region region;	/* region that has been exposed, if we enable
			   exposure compression */
{
    int x,		/* upper left x of exposed rectangle (pixels) */
	y,		/* upper left y of exposed rectangle (pixels) */
	width,		/* width of exposed rectangle (pixels) */
	height,		/* height of exposed rectangle (pixels) */
	first_row,	/* top line that was exposed (character cells) */
	last_row,	/* bottom line that was exposed (character cells) */
	first_column,	/* first character exposed in each row (cells) */
	length,		/* number of characters exposed in each row */
	row,		/* loop index for charater rows */
	dont_update,	/* TRUE means don't update this row */
	row_adjust_up,	/* number of lines to extend rectangle upward */
	row_adjust_down;/* number of lines to extend rectangle downward */

struct	regis_cntx
       *rs;		/* Pointer to the ReGIS data structure. */

    if ( event->type == GraphicsExpose || event->type == NoExpose )
	{

/*
 * New-style flow control: if the serial number for this exposure event
 * is the one we are waiting for, resume output.
 *
 * There is a server/Xlib problem that we have to work around: the protocol
 * only passes 16 bits for serial numbers, so Xlib has to guess at the top
 * 16 bits.  For this reason, we should only check the bottom 16 bits.
 */
	if ( (event->xgraphicsexpose.serial & 0xffff) ==
		(w->output.sync_serial & 0xffff) )
	    {
	    w->output.sync_serial = w->output.sync_serial2;
	    if ( w->output.sync_serial2 != 0 )
		{
		w->output.sync_serial2 = 0;
		s_start_output( w, STOP_OUTPUT_SCROLL );
		}
	    }
	if ( ! ( w->output.status & PARTIALLY_OBSCURED ) )
	    return;	/* this event was for flow control purposes only */
	}

/* make sure this is an event we care about and output is not disabled */

    if ( event->type != Expose && event->type != GraphicsExpose
      && event->type != NoExpose
      || w->output.status & ( OUTPUT_DISABLED | REDRAW_DISPLAY
       | DISABLE_REFRESH | RESIZE_SCROLL_BARS ) )
	return;

/*
 * Simplified algorithm for GraphicsExpose and NoExpose events:
 *
 * We keep track of the number of lines the display has scrolled up an/or
 * down: we will end up refreshing a rectangle that includes every line
 * in the reactangle we were told about PLUS some number of lines above or
 * below that rectangle.  Every time we scroll a line up or down we increment
 * (by the number of lines scrolled) the counter for scrolls in that
 * direction.  Every time we receive a GraphicsExpose or NoExpose event we
 * decrement (by one) the up counter if every scroll has been up, decrement
 * the down counter is every scroll has been down, or do nothing if scrolls
 * have been in both directions (that's the part of the price of not keeping
 * a scroll queue: we don't know if this was an up scroll or a down scroll).
 *
 * There is special handling of the first scroll that's done from the idle
 * state.  w->output.first_scroll contains the number of lines scrolled in
 * the first request: positive for scrolling up and negative for scrolling
 * down.  If a graphics expose event comes in and there is only one scroll
 * outstanding there's no need to expand the exposed rectangle.  The second
 * and subsequent scroll requests are added into lines_scrolled_up
 * or lines_scrolled down.
 */

    if ( event->type == GraphicsExpose || event->type == NoExpose )
	{
/*
 * Adjust area to be redrawn according to how much intervening scrolling
 * has been done.
 */
	row_adjust_up = w->output.lines_scrolled_up;
	row_adjust_down = w->output.lines_scrolled_down;
	if ( event->type == NoExpose || event->xgraphicsexpose.count == 0 )
	    {  /* last event for this XCopyArea */
	    if ( w->output.first_scroll > 0 )
		w->output.lines_scrolled_up -= w->output.first_scroll;
	    else if ( w->output.first_scroll < 0 )
		w->output.lines_scrolled_down += w->output.first_scroll;
	    else if ( w->output.scroll_direction > 0 )
		--w->output.lines_scrolled_up;
	    else if ( w->output.scroll_direction < 0 )
		--w->output.lines_scrolled_down;
	    w->output.first_scroll = 0;
	    if ( --w->output.exposures_pending <= 0 )
		{
		if ( w->output.exposures_pending < 0 )
		    widget_message( w,
			"Warning: too many graphics expose events\n" );
		w->output.lines_scrolled_up = 0;
		w->output.lines_scrolled_down = 0;
		w->output.scroll_direction = 0;
		}
	    }
	}
    else
	{
	row_adjust_up = 0;
	row_adjust_down = 0;
	}

    if ( event->type == NoExpose )
	return;

/* find the exposed rectangle, depending on what kind of exposure event
   this is */

    if ( event->type == Expose )
	{
	x = event->xexpose.x;
	y = event->xexpose.y;
	width = event->xexpose.width;
	height = event->xexpose.height;
	}
    else
	{	/* GraphicsExpose */
	x = event->xgraphicsexpose.x;
	y = event->xgraphicsexpose.y;
	width = event->xgraphicsexpose.width;
	height = event->xgraphicsexpose.height;
	}

/* make sure the exposed rectangle is at least partially within the
   display rectangle, including the border */

    if ( x + width <= 0 || x >= w->common.display_width + 2 * X_MARGIN
         || y + height <= 0 || y >= ((w->output.status & STATUS_LINE_VISIBLE) ?
		w->core.height : w->common.display_height + 2 * Y_MARGIN ))
	return;

/* redraw the border if any part of the border was exposed */

    if ( x < X_MARGIN || x + width >= w->common.display_width
      || y < Y_MARGIN || y + height >= w->common.display_height )
	repaint_border( w );

    if ( w->output.status & STATUS_LINE_VISIBLE
	&&  y <= (w->output.status_line_top + STATUS_LINE_HEIGHT)
	&& y + height >= w->output.status_line_top )
	  w->output.status |= UPDATE_STATUS_LINE;

/* find the top exposed row, but make sure it isn't above the top line in
   the display.  Also apply the row adjustment. */

    first_row = ( y - Y_MARGIN ) / w->output.char_height +
      w->output.top_visible_line - row_adjust_up;
    if ( first_row < w->output.top_visible_line )
	first_row = w->output.top_visible_line;

/* find the bottom exposed row, making sure it isn't below the bottom line
   in the display.  Also apply the row adjustment. */

    last_row = ( y + height - 1 - Y_MARGIN )
      / w->output.char_height + w->output.top_visible_line + row_adjust_down;
    if ( last_row > w->output.visible_rows + w->output.top_visible_line - 1 )
	last_row = w->output.visible_rows + w->output.top_visible_line - 1;

/* find the leftmost exposed column */

    first_column = ( x - X_MARGIN ) / w->output.char_width +
      w->output.left_visible_column;
    if ( first_column < w->output.left_visible_column )
	first_column = w->output.left_visible_column;

/* find the number of exposed columns, but not past the number of columns
   in the display */

    length = ( x + width - 1 - X_MARGIN )
      / w->output.char_width - first_column + 1
      + w->output.left_visible_column;
    if ( first_column + length > w->output.left_visible_column
      + w->output.visible_columns )
	length = w->output.left_visible_column + w->output.visible_columns
	  - first_column;

/* see whether the cursor is within the exposed area */

    if ( first_row <= w->output.cursor_row && w->output.cursor_row <= last_row
      && first_column <= w->output.cursor_right
      && w->output.cursor_left < first_column + length )
	{  /* yes, so mark the cursor as not visible */
	w->output.status &= ~CURSOR_IS_VISIBLE;
	}

/* Erase the ReGIS rubberband cursor only if one has already
 * been drawn.
 * */

    rs = (struct regis_cntx *)w->regis;

    if ( rs != NULL ) 			/* Make sure pointer is not null */
   	if ( rs->cs_rband_active )	/* before we reference it.	 */
	    {
	    regis_erase_rband_expose();
	    }

/* do any deferred scrolling */

    s_execute_deferred( w );

/* update the exposed rectangle and possibly the status line */

    o_update_rectangle( w, first_row, last_row, first_column, length, NULL );

    if ( w->output.status & UPDATE_STATUS_LINE ) {
	w->output.status &= ~UPDATE_STATUS_LINE;
	update_segment( w, STATUS_LINE, first_column, length );
    }

/* update the cursor */

    o_display_cursor( w );

/* and update the ReGIS cursor if active */

    if ( rs != NULL ) 			/* Make sure pointer is not null */
    	if ( rs->cs_rband_active )
	    {
	    (rs->cursor_motion_handler)();
	    }


}

/*

o_focus_in - keyboard focus has been given to us

This routine calls any specific output functions that may want to know
that focus has arrived.

*/

void o_focus_in (w, tag, event)
    DECtermData *w;
{

/* Tell cursor it may now blink */

    start_blink_timer (w, 0);
}

/*

o_focus_out - keyboard focus has been taken from us

This routine calls any specific output functions that may want to know
that focus has been taken away.

*/

void o_focus_out (w, tag, event)
    DECtermData *w;
{
}

/*

o_realize - a new widget instance has been created

This routine is called when the widget is realized.  It initializes the
output portion of the widget context block and registers an event handler
for visibility notification events with the toolkit.  Note that initialization
isn't complete until o_set_initial_line is called to set the top visible line
in the display.


*/

void o_realize( w )
    DECtermData *w;	/* context pointer for widget */
{

/* in debugging environment, synchronize events so they are executed
   immediately rather than buffered */

    if ( debug_flag )
	XSynchronize( XtDisplay(w), True );

/* register handler for visibility notification and non-maskable events */

    XtAddEventHandler( (Widget)w, VisibilityChangeMask, True,
		      o_event_handler, 0 );

    o_set_value_reverseVideo( w, w );	/* in case it starts at 1 */
    o_set_value_useBoldFont( w, w );    /* initialize really_use_bold_font */

/* initialize window border and background */

    XSetWindowBorder( XtDisplay(w), XtWindow(w),
      w->manager.foreground );

/* compute common size data */

    w->common.logical_width = w->common.columns * w->output.char_width;
    w->common.logical_height = w->common.rows * w->output.char_height;
    w->common.display_width = w->common.logical_width;
    w->common.display_height = w->common.logical_height;
    w->common.origin_x = 0;
    w->common.origin_y = 0;

/* decide whether or not to draw the status line */

    if ( w->common.statusDisplayEnable )
	w->output.status |= STATUS_LINE_VISIBLE;
    else
	w->output.status &= ~(STATUS_LINE_VISIBLE);

    if ( w->output.status & INITIAL_LINE_SET )
	initialize_window( w );		/* do what o_set_initial_line used to do */
}

/*

o_event_handler - output event handler

This procedure handles visibility exposure events and non-maskable events
(i.e. GraphicsExpose and NoExpose events).  It is registered with the toolkit
by o_realize.  Note that reconfiguration events and expose events are dispatched
directly by the toolkit to o_reconfigure and o_expose because they are
registered as part of the widget class record; they are not dispatched through
o_event_handler.

GraphicsExpose events are forwarded to o_expose.  Visibility notification
is used to keep track of whether the window is unobscured or is fully or
partially obscured (covered by another window).  If it is fully obscured
output is disabled; the toolkit will generate an expose event when the
window is visible again.  If the window is partially obscured, we need to
ask for graphics exposure events so when we scroll a line onto the display
from an obscured part of the display list, the line will get drawn.

*/

static void o_event_handler( w, closure, event)
    DECtermData *w;	/* context pointer for widget */
    caddr_t closure;	/* tag value (ignored) */
    XEvent *event;	/* data associated with event */
{
    XGCValues xgcv;	/* graphics context values to be changed */
    int row;

    if ( event->type == GraphicsExpose || event->type == NoExpose )
	o_expose( w, event );	/* treat GraphicsExpose as Expose */
    else if ( event->type == VisibilityNotify )
	{  /* our window visibility has changed */
	if ( event->xvisibility.state == VisibilityPartiallyObscured )
	    w->output.status |= PARTIALLY_OBSCURED;
	else
	    w->output.status &= ~PARTIALLY_OBSCURED;

/* if the window is fully obscured, turn off output */

	if ( event->xvisibility.state == VisibilityFullyObscured )
	    w->output.status |= OUTPUT_DISABLED;

	else
	    {  /* fully visible or partially obscured */
	    w->output.status &= ~OUTPUT_DISABLED;

/* If the window is fully visible, turn off graphics exposures.  If the
   window is partilly obscured, turn them on, so when a line is scrolled
   onto the display from an obscured part of the logical display, it
   will be drawn by o_expose */

	    if ( event->xvisibility.state == VisibilityPartiallyObscured )
		{
		xgcv.graphics_exposures = TRUE;
/*		s_disable_batch_scrolling( w );	*/
		}
	    else
		{
		xgcv.graphics_exposures = FALSE;
/*		s_enable_batch_scrolling( w );  */
		}
	    XChangeGC( XtDisplay(w), w->output.copy_area_gc,
	      GCGraphicsExposures, &xgcv );
	    }
	}
	
#ifdef MEASURE_KEYBOARD_PERFORMANCE
    else if ( ( event->type & 127 ) == ClientMessage
      && event->xclient.data.l[0] == KEYBOARD_ECHO
      && key_enabled && (char) event->xclient.data.l[1] == key_char )
	{
	struct timeb time_str;
	int diff;
	ftime( &time_str );
	diff = ( time_str.time - key_time.time ) * 1000 +
	  time_str.millitm - key_time.millitm;
	if ( diff < key_min )
	    key_min = diff;
	if ( diff > key_max )
	    key_max = diff;
	key_total += diff;
	key_num++;
	}
#endif

    else if ( ( event->type & 127 ) == ClientMessage
      && event->xclient.data.l[0] == XInternAtom( XtDisplay(w),
		"MESSAGE_REQUEST_RESIZE", FALSE ) )
	{
	int reason = DECwCRResize;
	regis_resize_terminal( w );
	if ( w->output.status & WINDOW_INITIALIZED )
	    XtCallCallbacks( (Widget)w, DECwNresizeCallback, &reason );
	w->output.status &= ~DISABLE_REDISPLAY_WINDOW;
	o_adjust_display( w );
/*
 * Now that we've called regis_resize_terminal, which clears regis mode,
 * we can safely restart output.  If user's string both did a resize and
 * called regis, it will now do its regis.
 */
        s_start_output( w, STOP_OUTPUT_RESIZE );
	}
    else if ( ( event->type & 127 ) == ClientMessage
      && event->xclient.data.l[0] == XInternAtom( XtDisplay(w),
		"MESSAGE_REDRAW_DISPLAY", FALSE ) )
	{
	if ( w->common.statusDisplayEnable )
	    w->output.status |= STATUS_LINE_VISIBLE;
	else
	    w->output.status &= ~(STATUS_LINE_VISIBLE);

	o_update_rectangle( w, w->output.top_visible_line,
	  w->output.top_visible_line + w->output.visible_rows + 1,
	  w->output.left_visible_column, w->output.visible_columns, NULL );

	update_segment( w, STATUS_LINE, w->output.left_visible_column,
	    w->output.visible_columns );
	o_display_cursor( w );
	repaint_border( w );
	adjust_scroll_bars( w );
	w->output.status &= ~DISABLE_REFRESH;
	}
}

/* o_set_initial_line - set initial top visible line number

This routine sets the line to be displayed at the top of the window.  Since
nothing can be drawn until the top line is known, this routine also creates
the scroll bars and graphics contexts and draws the border.

*/

void o_set_initial_line( w, line )
    DECtermData *w;	/* context pointer for widget */
    int line;		/* line number of top line in window */
{
    w->output.initial_line = line;
    w->output.status |= INITIAL_LINE_SET;

    if ( XtIsRealized(w) )
	initialize_window( w );
}

/* initialize_window - setup window-related stuff

This routine creates the scroll bars and graphics contexts and draws the border.
This routine can only be called when both the initial line has been set and
the widget is realized.  This routine can only be called once.

Input fields in graphics context:

	core.width		window width in pixels
	core.height		window height in pixels
	common.columns		logical display width in character cells
	common.rows		logical display height in character cells
	output.char_width	character width in pixels
	output.char_height	character height in pixels

Output fields in context block:

	output.left_visible_column	leftmost visible character in each row
	output.visible_columns		number visible characters in each row
	output.display_width		visible display width in pixels
	output.top_visible_line		top visible row of characters
	output.visible_rows		number of visible rows of characters
	output.display_height		visible display height in pixels
	output.initial_line		top line in logical display
	output.copy_area_gc,		GC for copying areas (scrolling)
	output.black_text_gc		GC for drawing black text
	output.white_text_gc		GC for drawing white text
	output.complement_gc		GC for drawing cursors
	output.h_scroll			horizontal scroll bar widget
	output.v_scroll			vertical scroll bar widget

*/

/*
 * The  following translation table is not a local variable because we don't
 * want to leak memory every time we make a new widget.
 */

#ifdef VMS_DECTERM
static noshare
#endif
    XtTranslations parsed_trans = NULL;

static
void initialize_window( w )
    DECtermData *w;	/* context pointer for widget */
    {
	XGCValues xgcv;	/* values to change in context block */
	int i;		/* loop index for text GCs */
	int hw, hh, vw, vh;
	Arg	al[22];
	int	ac;
/*
	The callbacks are each a zero-terminated list of procedure, tag pairs.
	One callback is used for the horizontal scroll bar and the other for
	the vertical scroll bar 
*/
	static XtCallbackRec 
		scroll_callback[] = {{scroll_callback_proc, 0}, 0 };

	char translation_table[30];
	strcpy( translation_table, "<Key>: InputFromWidgets()" );
/* 
	This routine can only be called once per DECterm.
*/
	if ( ! ( w->output.status & WINDOW_INITIALIZED ) ){
/*
 * We parse the translation table exactly *once* rather than for each widget,
 * so that we don't leak memory.  If there were a way to free up the translation
 * table when we delete the widget, we'd create the table once per widget.
 */
	if (! parsed_trans)
	parsed_trans = XtParseTranslationTable (translation_table);

/* 
	    Initialize the visible area with the indicated line on top 
*/
	    w->output.top_visible_line = w->output.initial_line;
	    w->output.left_visible_column = 0;
	    w->output.status |= WINDOW_INITIALIZED;
	    w->output.status &= ~OUTPUT_DISABLED;
	    calculate_visible_area( w );
/* 
	    Create graphics contexts:
	    The copy area GC sets the graphics exposures flag if the window is
	    partially visible, so if a line is scrolled onto the window from an
	    obscured part of the logical display it will generate a graphics
	    exposure event.  Initially the window is unobscured, so the graphics
	    exposure flag is false. 
*/
	    xgcv.graphics_exposures = False;
	    w->output.copy_area_gc = XCreateGC( XtDisplay(w), XtWindow(w),
		                     GCGraphicsExposures, &xgcv );
/* 
	    GC for XCopyArea with graphics exposures always set, for flow 
	    control 
*/
	    xgcv.graphics_exposures = True;
	    w->output.copy_area_graphics_expose_gc = XCreateGC( XtDisplay(w),
			XtWindow(w), GCGraphicsExposures, &xgcv );
/* 
	    Create GC's for normal and reverse in default colors 
*/
	    xgcv.foreground = w->manager.foreground;
	    xgcv.background = w->core.background_pixel;
	    w->output.normal_gc = XCreateGC( XtDisplay(w), XtWindow(w),
			GCForeground | GCBackground, &xgcv );
	    xgcv.foreground = w->core.background_pixel;
	    xgcv.background = w->manager.foreground;
	    w->output.reverse_gc = XCreateGC( XtDisplay(w), XtWindow(w),
			GCForeground | GCBackground, &xgcv );
/* 
	    Create the complement GC (with GXinvert drawing function) for 
	    drawing cursors.  The plane mask is set to the XOR of the 
	    foreground and background colors 
*/
	    w->output.complement_gc = XCreateGC( XtDisplay(w), 
			XtWindow(w), 0, 0 );
	    XSetFunction( XtDisplay(w), w->output.complement_gc, GXinvert );
	    XSetPlaneMask( XtDisplay(w), w->output.complement_gc,
	    		w->manager.foreground ^ w->core.background_pixel );
/* 
	    Draw the initial window 
*/
	    repaint_border( w );
	    o_display_cursor( w );
/* 
	    Create the horizontal scroll bar at the bottom of the window 
*/
	    find_optimal_scrollbar_sizes(w, &hw, &hh, &vw, &vh);

#ifdef ORIGINAL_CODE
/* Motif Transition : High-level widget routine conversion */
 *        w->output.h_scroll.widget =  DwtScrollBar( w, "hsb",
 * 	  0, w->core.height - SCROLL_BAR_WIDTH,
 * 	  hw, hh,
 * 	  SCROLL_BAR_HORIZONTAL_INCREMENT, w->output.visible_columns - 1,
 * 	  w->output.visible_columns, 0, 0, w->output.columns,
 * 	  XmHORIZONTAL,
 * 	  scroll_callback, scroll_callback, scroll_callback,
 * 	  scroll_callback, scroll_callback, scroll_callback,
 * 	  scroll_callback, scroll_callback, scroll_callback )
#endif

	    ac = 0;
	    XtSetArg(al[ac], XmNx, 0); ac++;
	    XtSetArg(al[ac], XmNy, w->core.height - SCROLL_BAR_WIDTH); ac++;
	    XtSetArg(al[ac], XmNwidth, hw); ac++;
	    XtSetArg(al[ac], XmNheight, hh); ac++;
	    XtSetArg(al[ac], XmNincrement, SCROLL_BAR_HORIZONTAL_INCREMENT); ac++;
	    XtSetArg(al[ac], XmNpageIncrement, w->output.visible_columns - 1); ac++;
	    XtSetArg(al[ac], XmNsliderSize, w->output.visible_columns); ac++;
	    XtSetArg(al[ac], XmNvalue, 0); ac++;
	    XtSetArg(al[ac], XmNminimum, 0); ac++;
	    XtSetArg(al[ac], XmNmaximum, w->output.columns); ac++;
	    XtSetArg(al[ac], XmNorientation, XmHORIZONTAL); ac++;
	    XtSetArg(al[ac], XmNvalueChangedCallback, scroll_callback); ac++;
/*	    XtSetArg(al[ac], XmNhelpCallback, scroll_callback); ac++; */
	    XtSetArg(al[ac], XmNincrementCallback, scroll_callback); ac++;
	    XtSetArg(al[ac], XmNdecrementCallback, scroll_callback); ac++;
	    XtSetArg(al[ac], XmNpageIncrementCallback, scroll_callback); ac++;
	    XtSetArg(al[ac], XmNpageDecrementCallback, scroll_callback); ac++;
	    XtSetArg(al[ac], XmNtoTopCallback, scroll_callback); ac++;
	    XtSetArg(al[ac], XmNtoBottomCallback, scroll_callback); ac++;
	    XtSetArg(al[ac], XmNdragCallback, scroll_callback); ac++;
	    w->output.h_scroll.widget = XmCreateScrollBar(w, "hsb", al, ac);

	    w->output.h_scroll.value = 0;
	    w->output.h_scroll.maxValue = w->output.columns;
	    w->output.h_scroll.minValue = 0;
	    w->output.h_scroll.shown = w->output.visible_columns;
	    XtRealizeWidget( w->output.h_scroll.widget );
	    XtOverrideTranslations( w->output.h_scroll.widget, parsed_trans );
	    if ( w->common.scrollHorizontal ) {
		XtManageChild( w->output.h_scroll.widget );
		w->output.status |= HORIZONTAL_SCROLL_BAR_MAPPED;
	    }
/* 
	    Create the vertical scroll bar at the right of the window 
*/
#ifdef ORIGINAL_CODE
/* Motif Transition : High-level widget routine conversion */
 *	w->output.v_scroll.widget = * DwtScrollBar( w, "vsb",
 * 	  w->core.width - SCROLL_BAR_WIDTH, 0,
 * 	  vw, vh,
 * 	  SCROLL_BAR_VERTICAL_INCREMENT, w->output.visible_rows - 1,
 * 	  w->output.visible_rows,
 * 	  w->output.top_visible_line,
 * 	  w->output.top_line,
 * 	  w->output.bottom_line + 1,
 * 	  XmVERTICAL,
 * 	  scroll_callback, scroll_callback, scroll_callback,
 * 	  scroll_callback, scroll_callback, scroll_callback,
 * 	  scroll_callback, scroll_callback, scroll_callback )
#endif
	    ac = 0;
	    XtSetArg(al[ac], XmNx, w->core.width - SCROLL_BAR_WIDTH); ac++;
	    XtSetArg(al[ac], XmNy, 0); ac++;
	    XtSetArg(al[ac], XmNwidth, vw); ac++;
	    XtSetArg(al[ac], XmNheight, vh); ac++;
	    XtSetArg(al[ac], XmNincrement, SCROLL_BAR_VERTICAL_INCREMENT); ac++;
	    XtSetArg(al[ac], XmNpageIncrement, w->output.visible_rows - 1); ac++;
	    XtSetArg(al[ac], XmNsliderSize, w->output.visible_rows); ac++;
	    XtSetArg(al[ac], XmNvalue, w->output.top_visible_line); ac++;
	    XtSetArg(al[ac], XmNminimum, w->output.top_line); ac++;
	    XtSetArg(al[ac], XmNmaximum, w->output.bottom_line + 1); ac++;
	    XtSetArg(al[ac], XmNorientation, XmVERTICAL); ac++;
	    XtSetArg(al[ac], XmNvalueChangedCallback, scroll_callback); ac++;
/*	    XtSetArg(al[ac], XmNhelpCallback, scroll_callback); ac++; */
	    XtSetArg(al[ac], XmNincrementCallback, scroll_callback); ac++;
	    XtSetArg(al[ac], XmNdecrementCallback, scroll_callback); ac++;
	    XtSetArg(al[ac], XmNpageIncrementCallback, scroll_callback); ac++;
	    XtSetArg(al[ac], XmNpageDecrementCallback, scroll_callback); ac++;
	    XtSetArg(al[ac], XmNtoTopCallback, scroll_callback); ac++;
	    XtSetArg(al[ac], XmNtoBottomCallback, scroll_callback); ac++;
	    XtSetArg(al[ac], XmNdragCallback, scroll_callback); ac++;
	    w->output.v_scroll.widget = XmCreateScrollBar(w, "vsb", al, ac);

	    w->output.v_scroll.value = w->output.top_visible_line;
	    w->output.v_scroll.maxValue = w->output.bottom_line + 1;
	    w->output.v_scroll.minValue = w->output.top_line;
	    w->output.v_scroll.shown = w->output.bottom_line -
	    w->output.top_line + 1;
	    XtRealizeWidget( w->output.v_scroll.widget );
	    XtOverrideTranslations( w->output.v_scroll.widget, parsed_trans );
	    if ( w->common.scrollVertical ) {
		XtManageChild( w->output.v_scroll.widget );
		w->output.status |= VERTICAL_SCROLL_BAR_MAPPED;
	    }
	}
}

/*
find_optimal_scrollbar_sizes - I bet you can figure out what this does
*/
find_optimal_scrollbar_sizes(w, hw, hh, vw, vh)
DECtermData *w;
int *hw, *hh, *vw, *vh;
{
    *hw = (w->common.display_width) + 2 * X_MARGIN - 2;
    *hh = SCROLL_BAR_WIDTH - 2;
    *vw = SCROLL_BAR_WIDTH - 2;
    *vh = (w->common.display_height) + 2 * Y_MARGIN - 2;
}

/*

o_set_top_line - set top line number in backing store

This routine defines the top line in the display list, including the
transcript.  It must be called before o_set_initial_line.

Input fields in context block:
	output.status & INITIAL_LINE_SET
					True if o_set_initial_line was called
	output.top_visible_line		top line in window
	output.char_height		height of standard character in pixels
	output.bottom_line		bottom line in logical display
	output.display_height		height of visible text area in pixels
	core.height			window height in pixels

Output fields in context block:
	output.top_line			top line in logical display

*/

void o_set_top_line( w, line )
    DECtermData *w;	/* context pointer for widget */
    int line;		/* line number of top line in backing store */
{
    int move_distance,		/* amount to scroll window by */
	unclipped_height,	/* window height in pixels not including
				   partial lines */
	old_display_height,	/* display height when called */
	old_bottom_line,	/* bottom line in window before called */
	new_bottom_line,	/* bottom line in window after called */
	old_top_visible_line,	/* top visible line when called */
	row,			/* loop index for text lines */
	height;			/* for XCopyArea */

    if ( w->output.status & TOP_LINE_SET && line == w->output.top_line )
	return;
    w->output.top_line = line;
    w->output.status |= TOP_LINE_SET;
    if ( w->output.status & WINDOW_INITIALIZED )
	{  /* moving the top line, so possibly adjust the window */
/*
/*	w->output.status &= ~DISABLE_REDISPLAY;
/*	o_adjust_display( w );	/* execute any deferred operations */
/*
 */

/* if the new top line is below the top of the window we need to scroll
   the window upwards and put the new top line at the top of the window */

	if ( line > w->output.top_visible_line )
	    {  /* the top of the window is no longer part of the l.d. */

/* move the window contents to put the new top line at the top of the
   window */

	    if ( row_is_marked( w, line ) )
		complement_row_marker( w, line, w->output.left_visible_column,
		  w->output.visible_columns );
	    move_distance = ( line - w->output.top_visible_line ) *
	      w->output.char_height;
	    unclipped_height = ( w->common.display_height /
	      w->output.char_height ) * w->output.char_height;
	    /* make sure the height is non-negative which will cause an error
	     * QAR 00040 of MACAW QAR DB    a.c. 01/10/91
	     */
	    height = (unclipped_height > move_distance) ?
		     (unclipped_height - move_distance) : 0;
	    XCopyArea( XtDisplay(w), XtWindow(w), XtWindow(w),
	      w->output.copy_area_gc, X_MARGIN, Y_MARGIN + move_distance,
	      w->common.display_width, height, X_MARGIN, Y_MARGIN );
	    if ( w->output.status & PARTIALLY_OBSCURED )
		add_scroll_entry( w, line - w->output.top_visible_line );
	    old_bottom_line = w->output.top_visible_line
	      + w->output.visible_rows - 1;
	    w->output.top_visible_line = line;
	    adjust_origin( w );

/* if the display area has to shrink, set the new clipping rectangle and
   clear the area under the display area */

	    if ( ( w->output.bottom_line - line + 1 ) * w->output.char_height <
	      w->common.display_height )
		{  /* area has to shrink */
		o_erase_cursor( w );
		erase_border( w );
		calculate_visible_area( w );
		old_display_height = w->common.display_height;
		XClearArea( XtDisplay(w), XtWindow(w), 0,
		  w->common.display_height + Y_MARGIN,
		  w->common.display_width + 2 * X_MARGIN,
		  old_display_height - w->common.display_height, 0 );
		}

/* refresh lines scrolled onto the display as needed */

	    new_bottom_line = w->output.visible_rows +
	      w->output.top_visible_line - 1;

	    o_update_rectangle( w, old_bottom_line + 1, new_bottom_line,
	      w->output.left_visible_column, w->output.visible_columns, NULL );

/* redraw the border and make sure the cursor is visible */

	    repaint_border( w );
	    o_display_cursor( w );
	    }

/* if the new top line is above the top of the window and there is room to
   expand the display area within the window, scroll the window downwards
   so we can fill it as completely as possible */

	else if ( line < w->output.top_visible_line
	  && w->common.display_height + w->output.char_height <=
	  w->core.height - ( w->common.scrollHorizontal ?
	  SCROLL_BAR_WIDTH : 0 ) - (w->common.statusDisplayEnable ?
	  STATUS_LINE_HEIGHT : 0) - 2 * Y_MARGIN )
	    {  /* the window must expand and scroll downwards */
	    o_erase_cursor( w );
	    erase_border( w );

/* set the new clipping rectangle and clear the old bottom border */

	    old_display_height = w->common.display_height;
	    old_top_visible_line = line;
	    XClearArea( XtDisplay(w), XtWindow(w), 0,
	      old_display_height + Y_MARGIN + 2, w->common.display_width +
	      2 * X_MARGIN, 2, 0 );
	    calculate_visible_area( w );

/* scroll the window downwards */

	    move_distance = w->common.display_height - old_display_height;
	    XCopyArea( XtDisplay(w), XtWindow(w), XtWindow(w),
	      w->output.copy_area_gc, X_MARGIN, Y_MARGIN,
	      w->common.display_width, old_display_height,
	      X_MARGIN, Y_MARGIN + move_distance );
	    if ( w->output.status & PARTIALLY_OBSCURED )
		add_scroll_entry( w, move_distance / w->output.char_height );

/* refresh the top of the display */

	    o_update_rectangle( w, line, old_top_visible_line - 1,
	      w->output.left_visible_column, w->output.visible_columns, NULL );

/* redraw the border and make sure the cursor is visible */

	    repaint_border( w );
	    o_display_cursor( w );
	    }

/* update the scroll bar */

	adjust_scroll_bars( w );
	}
    o_adjust_display( w );
}

/*

o_set_bottom_line - set bottom line number in backing store

This routine defines the bottom line in the display list, including the
status line.  It must be called before o_set_initial_line.

*/

void o_set_bottom_line( w, line )
    DECtermData *w;	/* context pointer for widget */
    int line;		/* line number of bottom line in backing store */
{
    int old_display_height,	/* display height when called */
	old_top_visible_line,	/* top visible line when called */
	old_bottom_complete_line,
				/* bottom line in window when called,
				   not including partial lines */
	move_distance,		/* number of pixels to scroll by */
	row;			/* loop index for character rows */

    if ( w->output.status & BOTTOM_LINE_SET && line == w->output.bottom_line )
	return;
    w->output.bottom_line = line;
    w->output.status |= BOTTOM_LINE_SET | COMPUTE_OPTIMAL_SIZE ;
    if ( w->output.status & WINDOW_INITIALIZED )
	{  /* moving the bottom line, so possibly adjust the window */

/* if the bottom line moves above the bottom of the visible area, either
   truncate the vertical area or pan vertically so the bottom line is
   at the bottom of the visible area. */

	if ( line < w->output.top_visible_line + w->output.visible_rows - 1 )
	    {  /* bottom line moved above bottom of visible area */
	    if ( line - w->output.top_line + 1 >= w->output.visible_rows )
		{  /* l.d. at least as tall as window, so put bottom line at
		      bottom of window */
		pan_vertical( w, line - w->output.visible_rows + 1 );
		}
	    else
		{  /* l.d. smaller than window */
		w->output.status |= REDRAW_DISPLAY;
		}
	    }

/* if the logical display has become taller and the logical display was
   shorter than the window, expand the window downwards */

	else if ( line >= w->output.top_visible_line + w->output.visible_rows )
	    {
	    w->output.status |= REDRAW_DISPLAY;
	    }
	}
    o_adjust_display( w );
}

/*

o_set_marker - set a marker between lines

*/

void o_set_marker( w, number, line )
    DECtermData *w;	/* context pointer for widget */
    int number,		/* number of marker, from 0 to 7 */
	line;		/* line number above which the marker will appear */
{
    if ( w->output.marker_is_set & ( 1 << number ) )
	complement_row_marker( w, w->output.marker_row[ number ],
	  w->output.left_visible_column, w->output.visible_columns );
    else
	w->output.marker_is_set |= ( 1 << number );
    w->output.marker_row[ number ] = line;
    complement_row_marker( w, line, w->output.left_visible_column,
      w->output.visible_columns );
}

/*

o_clear_marker - clear a marker between lines
         
*/

void o_clear_marker( w, number )
    DECtermData *w;	/* context pointer for widget */
    int number;		/* number of marker, from 0 to 7 */
{
    if ( w->output.marker_is_set & ( 1 << number ) )
	{
	complement_row_marker( w, w->output.marker_row[ number ],
	  w->output.left_visible_column, w->output.visible_columns );
	w->output.marker_is_set &= ~( 1 << number );
	}
}

void o_draw_intermediate_char( w, string16 )
    DECtermData *w;
    XChar2b *string16;
{
    static unsigned char code[2];
    static REND rend[2];
    static EXT_REND ext_rend[2];
    static s_line_structure ls_data = { code, rend, ext_rend, 2, 0, 0, 0 }; 
    int x, y, cwidth, gc_mask, color_index;
    REND attribute;
    Display *dpy = XtDisplay( w );
    Pixel foreground, background;
    unsigned char b_rendits =
	w->source.SpecificSourceData->wvt$b_rendits[w->output.cursor_row];

    code[0] = string16->byte1;
    code[1] = string16->byte2;
    rend[0] = rend[1] = 
	TWO_BYTE_SET | w->source.SpecificSourceData->wvt$w_actv_rendition;
    ext_rend[0] = w->source.SpecificSourceData->wvt$w_actv_ext_rendition |
	FIRST_OF_TWO;
    ext_rend[1] = w->source.SpecificSourceData->wvt$w_actv_ext_rendition |
	LAST_OF_TWO;
    attribute = ( rend[0] & csa_M_BLINK  && w->output.char_blink_phase & 2 ) ?
	rend[0] : rend[0] & ~csa_M_BLINK;
    gc_mask = HANGUL_TEXT_GC_MASK;
    if ((( attribute & csa_M_REVERSE ) ^ ( attribute & csa_M_BLINK )) & 1 )
	gc_mask |= REVERSE_TEXT_GC_MASK;
    ls_data.b_rendits = b_rendits;
    if ( b_rendits & csa_M_DOUBLE_HIGH )
	gc_mask |= DOUBLE_SIZE_TEXT_GC_MASK;
    if ( b_rendits & csa_M_DOUBLE_WIDTH )
	gc_mask |= DOUBLE_WIDE_TEXT_GC_MASK;
    if ( !w->output.text_gc[gc_mask] ) {
	w->output.text_gc[gc_mask] =
	    XCreateGC( dpy, XtWindow( w ), 0, 0 );
	XFlush( dpy );
	w->output.foreground_pixel[gc_mask] = 0;
	w->output.background_pixel[gc_mask] = 1;
	w->output.gc_font_valid[gc_mask] = FALSE;
    }
    if ( !w->output.gc_font_valid[gc_mask] ) {
	if ( !w->output.font_list[gc_mask >> 1] )
	    open_font( w, gc_mask );
	XSetFont( dpy, w->output.text_gc[gc_mask],
	    w->output.font_list[gc_mask >> 1]->fid );
	XFlush( dpy );
	w->output.gc_font_valid[gc_mask] = TRUE;
    }
/* handle color display... 910716 */
    if ( !( attribute & csa_M_NODEFAULT_TEXT ))
	foreground = ( gc_mask & REVERSE_TEXT_GC_MASK ) ?
	    w->core.background_pixel : w->manager.foreground;
    else {
	color_index = ( attribute & MASK_TEXT ) >> MASK_TEXT_SHIFT;
	foreground = get_ansi_color( w, color_index );
    }
    if ( w->output.foreground_pixel[gc_mask] != foreground ) {
	XSetForeground( dpy, w->output.text_gc[gc_mask], foreground );
	w->output.foreground_pixel[gc_mask] = foreground;
    }
    if ( !( attribute & csa_M_NODEFAULT_TEXT_BCK ))
	background = ( gc_mask & REVERSE_TEXT_GC_MASK ) ?
	    w->manager.foreground : w->core.background_pixel;
    else {
	color_index = ( attribute & MASK_TEXT_BCK ) >> MASK_TEXT_BCK_SHIFT;
	background = get_ansi_color( w, color_index );
    }
    if ( w->output.background_pixel[gc_mask] != background ) {
	XSetBackground( dpy, w->output.text_gc[gc_mask], background );
	w->output.background_pixel[gc_mask] = background;
    }
/* ....................... 910716 */
    if ( b_rendits & ( csa_M_DOUBLE_WIDTH | csa_M_DOUBLE_HIGH )) {
	cwidth = 4;
	x = (( w->output.cursor_column / 2 ) -
	     ( w->output.left_visible_column / 2 )) *
	     ( w->output.char_width * 2 ) + X_MARGIN;
	if ( w->output.left_visible_column & 1 )
	    x -= w->output.char_width;
    } else {
	cwidth = 2;
	x = ( w->output.cursor_column -
	    w->output.left_visible_column ) *
	    w->output.char_width + X_MARGIN;
    }
    if ( w->output.cursor_row == STATUS_LINE ) {
	y = w->output.status_line_top +
	    w->output.char_ascent;
    } else {
	y = ( w->output.cursor_row -
	    w->output.top_visible_line ) *
	    w->output.char_height +
	    w->output.char_ascent + Y_MARGIN;
    }
    if ( b_rendits & csa_M_DOUBLE_HIGH ) {
	y += w->output.char_ascent;
	if ( b_rendits & csa_M_DOUBLE_BOTTOM ) {
	    y -= w->output.char_height;
	}
    }
    o_erase_cursor( w );
/* clear intermediate char... 910718 */ 
    if ( !code[0] && !code[1] ) {
	XFillRectangle( dpy, XtWindow( w ),
	    (( gc_mask & REVERSE_TEXT_GC_MASK ) ? w->output.normal_gc
						: w->output.reverse_gc ),
	    x, y - w->output.char_ascent, cwidth * w->output.char_width,
	    w->output.char_height );
	if ( w->output.cursor_column + cwidth >
	    w->output.left_visible_column + w->output.visible_columns )
	    repaint_right_border( w, w->output.cursor_row );
    } else {
/* .......................... 910718 */ 
	draw_image_string( w, gc_mask, x, y, w->output.cursor_row, &ls_data, 0,
	    2 , NULL );
    }
    o_display_cursor( w );
}

/*

o_set_cursor_position - set output cursor position

This routine sets the output cursor position and it also erases the cursor
and redraws it at its new position.

Output fields in context block:
	output.cursor_row	cursor y position (character cells)
	output.cursor_column	cursor x position (character cells)

*/

void o_set_cursor_position( w, line, index )
    DECtermData *w;	/* context pointer for widget */
    int line,		/* line number */
	index;		/* column in line (0 based) */
{
    s_line_structure *ls;

    o_erase_cursor( w );			/* remove cursor from old location */
    w->output.cursor_row = line;	/* update the cursor position */
    w->output.cursor_column = index;
    ls = s_read_data( w, line );
    if ( ls->b_rendits & ( csa_M_DOUBLE_WIDTH | csa_M_DOUBLE_HIGH ) )
	{
	w->output.cursor_double = True;
	w->output.cursor_left = index * 2;
	w->output.cursor_right = index * 2 + 1;
	}
    else
	{
	w->output.cursor_double = False;
	w->output.cursor_left = index;
	w->output.cursor_right = index;
	}
    o_display_cursor( w );		/* draw cursor in new position */
}

/*

o_display_segment - part of a line in the backing store has been changed

This routine updates one or more contiguous characters on the same line
by calling s_read_data to read from the display list, then drawing the
characters on the display applying the font and attributes for each
character.

Input fields in context block:
	output.status & OUTPUT_DISABLED	means don't draw anything
	output.top_visible_line		top row displayed in window
	output.visible_rows		window height (character cells)
	output.left_visible_column	left column displayed in window
	output.visible_columns		window width (character cells)
	output.cursor_row		row containing cursor
	output.cursor_column		column containing cursor
*/

void o_display_segment( w, line, index, count )
    DECtermData *w;	/* context pointer for widget */
    int line,		/* line number */
	index,		/* starting column in line (0 based) */
	count;		/* count of characters */
{
    s_line_structure *ls;
    int left_x, left_visible_column, right_visible_column, char_width;

/* make sure at least part of the segment is visible in the window and that
   output isn't disabled */

    if ( w->output.status & OUTPUT_DISABLED )
	return;
    if ( line != STATUS_LINE && (
      line < w->output.top_visible_line
      || line >= w->output.top_visible_line + w->output.visible_rows
      || line < w->output.top_line
      || line > w->output.bottom_line ))
        return;
    if ( line == STATUS_LINE && !(w->output.status & STATUS_LINE_VISIBLE))
	return;
    if ( count <= 0 )
	return;

/* read from the display list to see whether this is a double width lline */

    ls = s_read_data( w, line );
    if ( ls->b_rendits & ( csa_M_DOUBLE_WIDTH | csa_M_DOUBLE_HIGH ) )
	{
	left_visible_column = ( w->output.left_visible_column - 1 ) / 2;
	right_visible_column = ( w->output.left_visible_column +
	  w->output.visible_columns - 1 ) / 2;
	char_width = w->output.char_width * 2;
	}
    else
	{
	left_visible_column = w->output.left_visible_column;
	right_visible_column = w->output.left_visible_column +
	  w->output.visible_columns - 1;
	char_width = w->output.char_width;
	}

    if ( index + count <= left_visible_column || index > right_visible_column )
	return;

/* if the left end of the segment is outside the window, clip the block to the
   left edge of the window */

    if ( index < left_visible_column )
	{
	count -= left_visible_column - index;
	index = left_visible_column;
	}

/* if the right end of the segment is outside the window, clip the segment to
   the right edge of the window */

    if ( index + count >= right_visible_column )
	count = right_visible_column - index + 1;

    left_x = ( index - left_visible_column ) * char_width + X_MARGIN;

    draw_segment( w, line, index, count, left_x, ls, NULL );
}

/*

o_scroll_lines - the backing store has been scrolled

This routine is called to scroll lines vertically on the screen in response to
lines being scrolled in the backing store.  It is different from the
pan_vertical routine, which scroll lines vertically when the vertical scroll
bar is activated.  o_scroll_lines scrolls only a selected region of the
window, and it erases lines that are scrolled into.

See the comments in o_expose for a description of the simplified algorithm
for handling occluded scrolling.

Note that this routine should be called after scrolling the display list
but before adjusting the selection, because s_set_selection calls
o_display_segment, which won't update the right lines unless the scrolling
has been done in the window (i.e. the window contents match the display list).

Input fields in context block:
    output.top_visible_line	line at top of window
    output.visible_rows		number of lines in window
    output.left_visible_column	column at left of window
    output.visible_columns	number of columns in window
    output.char_height		standard character height in current font
    output.copy_area_gc		GC for copying areas.  The clip rectangle
				is set to the display area (i.e. not including
				the border or scroll bars) and graphics
				exposure events may or may not be enabled,
				depending whether the window is partially
				obscured or not.
    output.display_height	height of display area in pixels
    output.display_width	width of display area in pixels

*/

void o_scroll_lines( w, count, line, numlines )
    DECtermData *w;		/* context pointer for widget */
    int count,			/* count of lines to scroll:
					positive to move upward
					negative to move downward */
	line,			/* starting line of region that has moved */
	numlines;		/* number of lines to move */
{
    int area_top,		/* top pixel in scroll region in logical
				   display space (i.e. it might be above or
				   below the window) */
	area_bottom,		/* one past bottom pixel in scroll region
				   in logical display space */
	move_distance,		/* number of pixels to scroll by */
	scroll_top,		/* top pixel to scroll in window */
	scroll_bottom,		/* one past bottom pixel to scroll in window */
	erase_top,		/* top pixel to erase in window */
	erase_bottom,		/* one past bottom pixel to erase in window */
	refresh_top,		/* top pixel to refresh (from display list) */
	refresh_bottom,		/* one past bottom pixel to refresh */
	refresh_first,		/* first line to refresh (character cells) */
	refresh_last,		/* one past last line to refresh */
	row,			/* loop index for character cell rows */
	marker,			/* loop index for markers */
	marker_was_set,		/* contents of w->output.marker_is_set
				   when routine was called */
	highlighted;		/* true if erased lines are highlighted */
    GC gc;			/* GC to use for scrolling */
    unsigned long serial;	/* serial number of XCopyArea */
    s_line_structure *ls;	/* pointer into the display list */

    area_top = ( line - w->output.top_visible_line ) * w->output.char_height
      + w->common.origin_y;
    area_bottom = area_top + numlines * w->output.char_height;
    if ( area_top < 0 )
	area_top = 0;
    move_distance = count * w->output.char_height;
    if ( w->common.backing_store_active && area_top < area_bottom )
	if ( count > 0 )
	    {
	    regis_scroll_up( w, move_distance, 0, area_top,
	      w->common.logical_width, area_bottom - area_top );
	    }
	else
	    {
	    regis_scroll_down( w, -move_distance, 0, area_top,
	      w->common.logical_width, area_bottom - area_top );
	    }

/* make sure at least part of the scrolling region is visible */

    if ( w->output.status & OUTPUT_DISABLED
      || line + numlines <= w->output.top_visible_line
      || line >= w->output.top_visible_line + w->output.visible_rows
      || count == 0 || numlines <= 0 )
	return;

/* erase the cursor if it is within the scrolling region */

    if ( line <= w->output.cursor_row
      && w->output.cursor_row <= line + numlines )
	o_erase_cursor(w);

/* erase any markers that are within the scrolling region */

    marker_was_set = w->output.marker_is_set;
    for ( marker = 0; marker < NUMBER_OF_MARKERS; marker++ )
	if ( w->output.marker_is_set & ( 1 << marker )
	  && line <= w->output.marker_row[ marker ]
	  && w->output.marker_row[ marker ] < line + numlines
	  && w->output.marker_row[ marker ] != w->output.top_visible_line )
	    complement_row_marker( w, w->output.marker_row[ marker ],
	      w->output.left_visible_column, w->output.visible_columns );
    w->output.marker_is_set = 0;	/* prevent markers from being drawn
					   when refreshing display */

/* find the top and bottom of the scrolling region relative to the top of
   the window, without worrying yet about what part of it is visible */

    area_top = ( line - w->output.top_visible_line ) * w->output.char_height
      + Y_MARGIN;
    area_bottom = area_top + numlines * w->output.char_height;

/* find what part of the scrolling region is visible */

    scroll_top = max( area_top, Y_MARGIN );
    scroll_bottom = min( area_bottom, w->common.display_height + Y_MARGIN );

    move_distance = abs( count ) * w->output.char_height;

    if ( w->output.status & PARTIALLY_OBSCURED )
	add_scroll_entry( w, count );
		/* put in scroll_queue to adjust graphics expose rectangle */

/* decide which gc to use for the XCopyArea depending on whether we need to
   synchronize */

    if ( ++w->output.scroll_count >= w->common.syncFrequency )
	{
	gc = w->output.copy_area_graphics_expose_gc;
	w->output.scroll_count = 0;
	}
    else
	gc = w->output.copy_area_gc;
    serial = 0;

/* decide which whether the area to be erased is highlighted or not */

    ls = s_read_data( w, count > 0 ? line + numlines - 1 : line );
    highlighted = ( ls->highlight_begin < ls->highlight_end );

    if ( count > 0 )
	{  /* scroll up */

/* scrolling up: adjust the bottom of the visible scrolling region so we
   don't copy a partially clipped line at the bottom of the window */

	scroll_bottom = ( ( scroll_bottom - Y_MARGIN ) / w->output.char_height )
	  * w->output.char_height + Y_MARGIN;

/* move all but the last "count" lines up by "count" lines */

	if ( scroll_bottom > scroll_top + move_distance )
	    {
	    if ( gc == w->output.copy_area_graphics_expose_gc )
		serial = XNextRequest( XtDisplay(w) );
	    XCopyArea( XtDisplay(w), XtWindow(w), XtWindow(w), gc,
	      X_MARGIN, scroll_top + move_distance,
	      w->common.display_width, scroll_bottom - scroll_top -
	      move_distance, X_MARGIN, scroll_top );
	    }

/* calculate what lines have to be refreshed: these are lines that are scrolled
   into from outside the window, including the partial line at the bottom
   of the window (if any).  We could have instead relied on a graphics
   exposure event to refresh these lines. */

	erase_top = area_bottom - move_distance;
	refresh_top = scroll_bottom - move_distance;
	if ( refresh_top < erase_top )
	    {  /* make sure at least one line needs to be refreshed */
	    refresh_first = ( refresh_top - Y_MARGIN ) / w->output.char_height +
	      w->output.top_visible_line;
	    refresh_bottom = min( scroll_bottom, erase_top );
	    refresh_last = ( refresh_bottom - Y_MARGIN + w->output.char_height
	      - 1 ) / w->output.char_height + w->output.top_visible_line;

/* refresh those lines from the display list */

	    o_update_rectangle( w, refresh_first, refresh_last,
	      w->output.left_visible_column, w->output.visible_columns, NULL );
	    }

/* calculate the lines that need to be erased: these are the last "count"
   lines in the scrolling region, regardless of what lines were visible.
   We only actually erase the lines that are visible, however. */

	erase_top = max( erase_top, Y_MARGIN );
	erase_bottom = min( area_bottom, w->common.display_height +
	  Y_MARGIN );

/* if there are any lines that need to be erased, erase them */

	if ( erase_top < erase_bottom )
	    if ( highlighted )
		XFillRectangle( XtDisplay(w), XtWindow(w), w->output.normal_gc,
		  X_MARGIN, erase_top,
		  w->common.display_width, erase_bottom - erase_top );
	    else
		XClearArea( XtDisplay(w), XtWindow(w), X_MARGIN, erase_top,
		  w->common.display_width, erase_bottom - erase_top, 0 );
	}
    else
	{  /* scroll down */

/* move all but the first "count" lines down by "count" lines.  We don't
   have to worry about partial lines in this case because we know the
   window starts at a line boundary (guaranteed by calculate_display_area),
   and the clipping rectangle ensures that if only part of the bottom line
   is visible we won't write over the window border */

	if ( scroll_bottom > scroll_top + move_distance )
	    {
	    if ( gc == w->output.copy_area_graphics_expose_gc )
		serial = XNextRequest( XtDisplay(w) );
	    XCopyArea( XtDisplay(w), XtWindow(w), XtWindow(w), gc,
	      X_MARGIN, scroll_top,
	      w->common.display_width, scroll_bottom - scroll_top -
	      move_distance, X_MARGIN, scroll_top + move_distance );
	    }

/* calculate the region that needs to be refreshed, i.e. lines that were
   scrolled into from outside the window, and are not going to be erased. */

	erase_bottom = area_top + move_distance;
	refresh_bottom = min( scroll_top + move_distance, scroll_bottom );
	if ( refresh_bottom > erase_bottom )
	    {  /* make sure at least part of the region is visible */
	    refresh_top = max( scroll_top, erase_bottom );
	    refresh_first = ( refresh_top - Y_MARGIN ) / w->output.char_height +
	      w->output.top_visible_line;
	    refresh_last = ( refresh_bottom - Y_MARGIN + w->output.char_height
	      - 1 ) / w->output.char_height + w->output.top_visible_line;

/* refresh this region from the display list */

	    o_update_rectangle( w, refresh_first, refresh_last,
	      w->output.left_visible_column, w->output.visible_columns, NULL );
	    }

/* calculate the region to be erased: the top "count" lines in the scrolling
   region, clipped to the window boundary */

	erase_top = max( area_top, Y_MARGIN );
	erase_bottom = min( erase_bottom, w->common.display_height +
	  Y_MARGIN );

/* if at least part of the region to be erased is visible, erase it */

	if ( erase_top < erase_bottom )
	    if ( highlighted )
		XFillRectangle( XtDisplay(w), XtWindow(w), w->output.normal_gc,
		  X_MARGIN, erase_top,
		  w->common.display_width, erase_bottom - erase_top );
	    else
		XClearArea( XtDisplay(w), XtWindow(w), X_MARGIN, erase_top,
		  w->common.display_width, erase_bottom - erase_top, 0 );
	}

/* re-draw any markers that are within the scrolling region */

    w->output.marker_is_set = marker_was_set;
    for ( marker = 0; marker < NUMBER_OF_MARKERS; marker++ )
	if ( w->output.marker_is_set & ( 1 << marker )
	  && line <= w->output.marker_row[ marker ]
	  && w->output.marker_row[ marker ] < line + numlines
	  && w->output.marker_row[ marker ] != w->output.top_visible_line )
	    complement_row_marker( w, w->output.marker_row[ marker ],
	      w->output.left_visible_column, w->output.visible_columns );

/* if we asked for graphics exposures, remember the serial number and
   possibly stop output */

    if ( serial != 0 )
	if ( w->output.sync_serial == 0 )
	    w->output.sync_serial = serial;
	else
	    {
	    w->output.sync_serial2 = serial;
	    s_stop_output( w, STOP_OUTPUT_SCROLL );
	    }
}

/*

o_erase_lines - lines in the backing store are now blank

This routine is called to erase one or more lines on the window.

Input fields in context block:
	output.status & OUTPUT_DISABLED		If True, don't draw anything
	output.top_visible_line		top line in window
	output.visible_rows		number of lines in window
	output.visible_columns		number of characters in each line
	output.char_height		standard character height in pixels
	output.char_width		standard character width in pixels

*/

void o_erase_lines( w, line, numlines )
    DECtermData *w;	/* context pointer for widget */
    int line,		/* starting line of region that is erased */
	numlines;	/* number of lines to erase */
{
    int erase_top,	/* top pixel to erase */
	erase_bottom,	/* one past bottom pixel to erase */
	marker;		/* loop index for markers */

/* make sure output isn't disabled and there is at least one line to erase */

    if ( w->output.status & OUTPUT_DISABLED )
	return;
    if (line != STATUS_LINE && (
      line >= w->output.top_visible_line + w->output.visible_rows
      || line + numlines <= w->output.top_visible_line || numlines <= 0 ))
	return;
    if ( line == STATUS_LINE && !(w->output.status & STATUS_LINE_VISIBLE))
	return;

/* erase the cursor if it's in one of the lines to be erased */

    if ( line <= w->output.cursor_row
      && w->output.cursor_row < line + numlines )
	o_erase_cursor( w );

/* erase the area */

    if (line == STATUS_LINE) {
        erase_top = w->output.status_line_top;
	erase_bottom = erase_top + STATUS_LINE_HEIGHT - 1;
    } else {
        erase_top = ( line - w->output.top_visible_line )
	  * w->output.char_height + Y_MARGIN;
        if ( erase_top < Y_MARGIN )
            erase_top = Y_MARGIN;
        erase_bottom = ( line - w->output.top_visible_line + numlines ) *
          w->output.char_height + Y_MARGIN;
        if ( erase_bottom > w->common.display_height + Y_MARGIN )
	  erase_bottom = w->common.display_height + Y_MARGIN;
    }
    XClearArea( XtDisplay(w), XtWindow(w), X_MARGIN, erase_top,
      w->output.visible_columns * w->output.char_width,
      erase_bottom - erase_top, 0 );

/* tell ReGIS to erase its backing store */

    if ( w->common.backing_store_active )
	regis_erase_rectangle( w, w->core.background_pixel, 0,
	  erase_top - w->common.origin_y - Y_MARGIN,
	  w->common.logical_width, erase_bottom - erase_top );

/* re-draw any markers that are within the scrolling region */

      for ( marker = 0; marker < NUMBER_OF_MARKERS; marker++ )
	if ( w->output.marker_is_set & ( 1 << marker )
	  && line <= w->output.marker_row[ marker ]
	  && w->output.marker_row[ marker ]< line + numlines )
	    complement_row_marker( w, line, w->output.left_visible_column,
	      w->output.visible_columns );
}

/*

o_clear_comm - reset flow control

This routine resets the communication path to the X server.  In the old flow
control method, this means we clear the synchronization count.

*/

void o_clear_comm( w )
    DECtermData *w;	/* context pointer for widget */
{
    w->output.scroll_count = 0;
    w->output.sync_serial = 0;
    w->output.sync_serial2 = 0;
    s_start_output( w, STOP_OUTPUT_SCROLL );
}

/*

o_set_value_fg_bg - called when either the foreground or background
		    resources change

*/

void o_set_value_fg_bg( oldw, w )
    DECtermWidget oldw, w;	/* context pointer for widget */
{

    w->common.original_foreground_color = w->manager.foreground;
    w->common.original_background_color = w->core.background_pixel;
    find_default_foreground( w );
    find_default_background( w );
    w->common.screenMode = ( w->common.default_foreground_mono <
	w->common.default_background_mono );
    if ( ! w->common.screenMode && w->common.text_background_index != 0 )
	{
	w->common.text_foreground_index = w->common.text_background_index;
	w->common.text_background_index = 0;
	}
    else if ( w->common.screenMode && w->common.text_foreground_index != 0 )
	{
	w->common.text_background_index = w->common.text_foreground_index;
	w->common.text_foreground_index = 0;
	}
    set_default_foreground( w );
    set_default_background( w );

/* redraw the border and window context in their new colors unless output
   is disabled */

    if ( w->output.status & OUTPUT_DISABLED )
	return;
    w->output.status |= REDRAW_DISPLAY;
    o_adjust_display( w );
}


/*

o_set_value_foreground - called when foreground resource changes

This routine is called to notify the output component that the value of the
foreground resource has changed.  It sets the RGB value of ReGIS color 7 and
sets the foreground resource to the pixel value ReGIS has allocated for
color 7.

*/

void o_set_value_foreground( oldw, w )
    DECtermWidget oldw, w;	/* context pointer for widget */
{
    o_set_value_fg_bg( oldw, w );
}


/*

o_set_value_background - called when background resource changes

This routine is called to notify the output component that the value of the
background resource has changed.  It sets the RGB value of ReGIS color 0 and
sets the background resource to the pixel value ReGIS has allocated for
color 0.

*/

void o_set_value_background( oldw, w )
    DECtermWidget oldw, w;	/* context pointer for widget */
{
    o_set_value_fg_bg( oldw, w );
}

/*

o_set_value_reverseVideo - called when reverseVideo resource changes

This routine is called to notify the output component that the value of
the reverseVideo resource has changed.  Its effect is to swap the foreground
anf background resources.  It resets reverseVideo to FALSE so it can be
set again to TRUE.
It redraws the borders and refreshes the screen using the new background
and foreground.

Input fields in context block:
	common.reverseVideo

*/

void o_set_value_reverseVideo( oldw, w )
    DECtermWidget oldw,w;	/* context pointer for widget */
{
    Pixel temp;

    if ( !w->common.reverseVideo ) return;
    w->common.reverseVideo = FALSE;

/* reset the colormap, clearing the graphics visible bit (the screen will
   be redraw later).  This restores foreground and background to their
   true pixel values */

    w->common.graphics_visible = FALSE;
    WVT$RESET_COLORMAP( w );

/* swap the foreground and background colors */

    temp = w->core.background_pixel;
    w->core.background_pixel = w->manager.foreground;
    w->manager.foreground = temp;

/* set the new colors in graphics contexts */

    o_set_value_foreground( w, w );
    o_set_value_background( w, w );

/* clear the graphics backing store */

    destroy_backing_store( w );

/* redraw the border and window context in their new colors unless output
   is disabled */

    if ( w->output.status & OUTPUT_DISABLED )
	return;
    w->output.status |= REDRAW_DISPLAY;
    o_adjust_display( w );
}

/*

o_set_value_statusDisplayEnable - called when status display resource changes.

This routine will make the output erase or draw the status display depending
on what value the resource is.

*/

void o_set_value_statusDisplayEnable( oldw, neww )
    DECtermWidget oldw, neww;	/* widget context block */
{
    neww->output.status |= REDRAW_DISPLAY | COMPUTE_OPTIMAL_SIZE;
    o_adjust_display( neww );
}

/*

o_set_value_fontSetSelection - called when fontSetSelection resource changes

This routine is called to notify the output component that the font set
has changed.  It loads the fonts in the font set and sets the font size
fields in the context block.

*/

void o_set_value_fontSetSelection( oldw, w )
    DECtermWidget oldw,w;	/* widget context block */
{
    XFontStruct *font_struct;	/* font structure for standard font */
    int row,			/* loop index for text rows */
	font_index,		/* default font (normal or compressed) */
	i;			/* generic loop index */
    int j;

/* free all fonts and mark them as unloaded */

    for ( i = 0; i < FONTS_PER_SET; i++ )
	{
	if ( w->output.font_list[i] )
	    {
	    j = i & ~(( BOLD_TEXT_GC_MASK | DOUBLE_WIDE_TEXT_GC_MASK |
			DOUBLE_SIZE_TEXT_GC_MASK ) >> 1 );
	    if (( i != j ) &&
		( w->output.font_list[i] == w->output.font_list[j] ))
		w->output.font_list[i] = NULL;
	    }
	}
    for ( i = 0; i < FONTS_PER_SET; i++ ) {
	if ( w->output.font_list[i] ) {
	    XFreeFont( XtDisplay(w), w->output.font_list[i] );
	    w->output.font_list[i] = NULL;
	    }
	}

    for ( i = 0; i < NUM_TEXT_GCS; i++ )
	w->output.gc_font_valid[i] = FALSE;

/* load the normal font */

    font_index = FONT_NORMAL;

    open_font( w, font_index << 1 );

/* erase cursor with old character sizes so flag gets cleared */

    erase_old_cursor( oldw, w );

/* set standard character dimensions */

    font_struct = w->output.font_list[font_index];
    w->output.char_width = font_struct->max_bounds.width;
/* in Kanji ReGIS mode, the aspect retio of each character cell is kept 1:2.5 
   to make sure DECterm emulates VT284/6 exactly */

    w->output.real_char_height = font_struct->ascent + font_struct->descent;

    if (( w->source.wvt$l_ext_flags & vte1_m_asian_common ) && w->common.regisScreenMode )
        w->output.char_height = w->output.char_width * 2.5;
    else
        w->output.char_height = w->output.real_char_height;
    w->output.char_ascent = font_struct->ascent;

/* see if this is an old-style font or a new-style font */

    if ( w->source.wvt$l_ext_flags & vte1_m_asian_common )
	w->common.v1_encodings = TRUE;
    else if ( w->source.wvt$l_ext_flags & vte1_m_turkish ||
	      w->source.wvt$l_ext_flags & vte1_m_greek )
	w->common.v1_encodings = FALSE;
    else
        w->common.v1_encodings = ( font_struct->max_char_or_byte2 == 253 );

/* recompute screen dimensions */

    w->output.status |= REDRAW_DISPLAY | COMPUTE_OPTIMAL_SIZE;
    o_adjust_display( w );

/* tell ReGIS about the change */

    regis_change_font( w );
}

/* o_set_value_bigFontSetName - called when bigFontSetName resource changes */

void o_set_value_bigFontSetName( oldw, w )
    DECtermWidget oldw, w;
{
    int len, count;
    char *ournew;		/* our own copy of new font set name */
    char **font_name_list;
    char default_name[MAX_FONT_NAME_SIZE+1];
/*
 * Guarantee a null at the end of whatever string we're dealt.
 */
    default_name[MAX_FONT_NAME_SIZE] = 0;

/* if the font name is null, calculate it based on the screen size */

    if ( w->common.bigFontSetName[0] == '\0' )
	{
	if ( w->source.wvt$l_ext_flags & vte1_m_asian_common )
	    strncpy( default_name, DEFAULT_ASIAN_BIG_FONT_SET_NAME,
		        MAX_FONT_NAME_SIZE );
	else
	    strncpy( default_name, DEFAULT_BIG_FONT_SET_NAME,
	                MAX_FONT_NAME_SIZE );

/* Use a 14 point font instead of an 18 point font on 100 DPI monitors */
	if ( GetDisplayDPI(XtDisplay(w)) > 90 )
	    DLFDSetFontNameField( DLFD_POINT_SIZE, default_name, 3, "140" );

	w->common.bigFontSetName = default_name;
	}

/* make sure the font set exists - if not, don't change the font set name */

    font_name_list = XListFonts( XtDisplay(w), w->common.bigFontSetName, 1,
      &count );
    XFreeFontNames( font_name_list );
    if ( count == 0 )
	{
	if ( w->common.fontSetSelection == DECwBigFont )
	    widget_error( w, DECW$K_MSG_CANT_FIND_FONT, 0,
		    w->common.bigFontSetName );
	if ( w->output.status & BIG_FONT_SET_NAME_ALLOCATED )
	    {
	    w->common.bigFontSetName = oldw->common.bigFontSetName;
	    return;
	    }
	else
	    if ( w->source.wvt$l_ext_flags & vte1_m_asian_common )
		w->common.bigFontSetName = DEFAULT_ASIAN_BIG_FONT_SET_NAME;
	    else
		w->common.bigFontSetName = DEFAULT_BIG_FONT_SET_NAME;
	}
    
/* copy the new font set name into space we allocate and free the old name.
   The order is important because new might point to the old name (e.g.
   during initialization) */

    len = strlen( w->common.bigFontSetName ) + 1;
    ournew = XtMalloc( len );
    if ( ournew == NULL )
	{
	widget_message( w,
	  "Can't allocate space for new big font name, length is %d bytes\n",
	  len );
	widget_message( w, "BigFontSetName not changed\n" );
	return;
	}
    memcpy( ournew, w->common.bigFontSetName, len );
    if ( w->output.status & BIG_FONT_SET_NAME_ALLOCATED )
	XtFree( oldw->common.bigFontSetName );
    else
	w->output.status |= BIG_FONT_SET_NAME_ALLOCATED;
    w->common.bigFontSetName = ournew;
    if ( w->common.fontSetSelection == DECwBigFont )
	o_set_value_fontSetSelection( oldw, w );
}

/* o_set_value_littleFontSetName - called when littleFontSetName resource
changes */

void o_set_value_littleFontSetName( oldw, w )
    DECtermWidget oldw, w;
{
    int len, count;
    char *ournew;		/* our own copy of new font set name */
    char **font_name_list;
    char default_name[MAX_FONT_NAME_SIZE+1];
/*
 * Guarantee a null at the end of whatever string we're dealt.
 */
    default_name[MAX_FONT_NAME_SIZE] = 0;

/* if the font name is null, calculate it based on the screen size */

    if ( w->common.littleFontSetName[0] == '\0' )
	{
	if ( w->source.wvt$l_ext_flags & vte1_m_asian_common )
	    strncpy( default_name, DEFAULT_ASIAN_LITTLE_FONT_SET_NAME,
		        MAX_FONT_NAME_SIZE );
	else
	    strncpy( default_name, DEFAULT_LITTLE_FONT_SET_NAME,
	                MAX_FONT_NAME_SIZE );

/* Use a 10 point font instead of a 14 point font on 100 DPI monitors */
	if ( GetDisplayDPI(XtDisplay(w)) > 90 )
	    DLFDSetFontNameField( DLFD_POINT_SIZE, default_name, 3, "100" );

	w->common.littleFontSetName = default_name;
	}

/* make sure the font set exists - if not, don't change the font set name */

    font_name_list = XListFonts( XtDisplay(w), w->common.littleFontSetName,
      1, &count );
    XFreeFontNames( font_name_list );
    if ( count == 0 )
	{
	if ( w->common.fontSetSelection == DECwLittleFont )
	    widget_error( w, DECW$K_MSG_CANT_FIND_FONT, 0,
		    w->common.littleFontSetName );
	if ( w->output.status & LITTLE_FONT_SET_NAME_ALLOCATED )
	    {
	    w->common.littleFontSetName = oldw->common.littleFontSetName;
	    return;
	    }
	else
	    if ( w->source.wvt$l_ext_flags & vte1_m_asian_common )
		w->common.littleFontSetName =
		    DEFAULT_ASIAN_LITTLE_FONT_SET_NAME;
	    else
		w->common.littleFontSetName = DEFAULT_LITTLE_FONT_SET_NAME;
	}
    
/* copy the new font set name into space we allocate and free the old name.
   The order is important because new might point to the old name (e.g.
   during initialization) */

    len = strlen( w->common.littleFontSetName ) + 1;
    ournew = XtMalloc( len );
    if ( ournew == NULL )
	{
	widget_message( w,
	  "Can't allocate space for new little font name, length is %d bytes\n",
	  len );
	widget_message( w, "LittleFontSetName not changed\n" );
	return;
	}
    memcpy( ournew, w->common.littleFontSetName, len );
    if ( w->output.status & LITTLE_FONT_SET_NAME_ALLOCATED )
	XtFree( oldw->common.littleFontSetName );
    else
	w->output.status |= LITTLE_FONT_SET_NAME_ALLOCATED;
    w->common.littleFontSetName = ournew;
    if ( w->common.fontSetSelection == DECwLittleFont )
	o_set_value_fontSetSelection( oldw, w );
}

/* o_set_value_gsFontSetName - called when gsFontSetName resource changes */

void o_set_value_gsFontSetName( oldw, w )
    DECtermWidget oldw, w;
{
    int len, count;
    char *ournew;		/* our own copy of new font set name */
    char **font_name_list;
    char font_name_buffer[MAX_FONT_NAME_SIZE+1];
    char *font_name;
/*
 * Guarantee a null at the end of whatever string we're dealt.
 */
    font_name_buffer[MAX_FONT_NAME_SIZE] = 0;

/* make sure the font set exists - if not, don't change the font set name */

    font_name = w->common.gsFontSetName;
    font_name_list = XListFonts( XtDisplay(w), font_name, 1, &count );
    XFreeFontNames( font_name_list );

/* if the GS font doesn't exist, see if we can load the non-GS font */

    if ( count == 0 )
	{
	widget_message( w, "Can't find GS font %s\n", font_name );
	strncpy( font_name_buffer, font_name, MAX_FONT_NAME_SIZE );
	DLFDSetFontNameField( DLFD_ADD_STYLE_NAME, font_name_buffer, 0, "" );
	font_name_list = XListFonts( XtDisplay(w), font_name_buffer, 1,
	  &count );
	XFreeFontNames( font_name_list );
	font_name = font_name_buffer;
	}

    if ( count == 0 )
	{
	if ( w->common.fontSetSelection == DECwGSFont )
	    widget_error( w, DECW$K_MSG_CANT_FIND_FONT, 0,
		    w->common.gsFontSetName );
	if ( w->output.status & GS_FONT_SET_NAME_ALLOCATED )
	    {
	    w->common.gsFontSetName = oldw->common.gsFontSetName;
	    return;
	    }
	else
	    w->common.gsFontSetName = DEFAULT_GS_FONT_SET_NAME;
	}
    
/* copy the new font set name into space we allocate and free the old name.
   The order is important because new might point to the old name (e.g.
   during initialization) */

    len = strlen( font_name ) + 1;
    ournew = XtMalloc( len );
    if ( ournew == NULL )
	{
	widget_message( w,
	  "Can't allocate space for new GS font name, length is %d bytes\n",
	  len );
	widget_message( w, "GSFontSetName not changed\n" );
	return;
	}
    memcpy( ournew, font_name, len );
    if ( w->output.status & GS_FONT_SET_NAME_ALLOCATED )
	XtFree( oldw->common.gsFontSetName );
    else
	w->output.status |= GS_FONT_SET_NAME_ALLOCATED;
    w->common.gsFontSetName = ournew;
    if ( w->common.fontSetSelection == DECwGSFont )
	o_set_value_fontSetSelection( oldw, w );
}

void o_set_value_bigFontOtherName ( oldw, neww )
    DECtermWidget oldw, neww;
{
    if ( oldw->common.bigFontOtherName != NULL )
	XtFree( oldw->common.bigFontOtherName );

    if ( neww->common.bigFontOtherName != NULL )
	neww->common.bigFontOtherName =
		XtNewString( neww->common.bigFontOtherName );
}

void o_set_value_littleFontOtherName ( oldw, neww )
    DECtermWidget oldw, neww;
{
    if ( oldw->common.littleFontOtherName != NULL )
	XtFree( oldw->common.littleFontOtherName );

    if ( neww->common.littleFontOtherName != NULL )
	neww->common.littleFontOtherName =
		XtNewString( neww->common.littleFontOtherName );
}

void o_set_value_gsFontOtherName ( oldw, neww )
    DECtermWidget oldw, neww;
{
    if ( oldw->common.gsFontOtherName != NULL )
	XtFree( oldw->common.gsFontOtherName );

    if ( neww->common.gsFontOtherName != NULL )
	neww->common.gsFontOtherName =
		XtNewString( neww->common.gsFontOtherName );
}


void o_set_value_fineFontSetName( oldw, w )
    DECtermWidget oldw, w;
{
#if !defined(ALPHA_FINE_FONT_BUG)
    int len, count;
    char *ournew;		/* our own copy of new font set name */
    char **font_name_list;
    char default_name[MAX_FONT_NAME_SIZE];

/*
 * Guarantee a null at the end of whatever string we're dealt.
 */
    default_name[MAX_FONT_NAME_SIZE] = 0;

    if ( w->common.fineFontSetName[0] == '\0' )
	{
	strncpy( default_name, DEFAULT_ASIAN_FINE_FONT_SET_NAME,
	    MAX_FONT_NAME_SIZE );
	w->common.fineFontSetName = default_name;
	}

/* make sure the font set exists - if not, don't change the font set name */

    font_name_list = XListFonts( XtDisplay(w), w->common.fineFontSetName,
      1, &count );
    XFreeFontNames( font_name_list );
    if ( count == 0 )
	{
	if ( w->source.wvt$l_ext_flags & vte1_m_tomcat )
	widget_error( w, DECW$K_MSG_CANT_FIND_FONT, 0,
		w->common.fineFontSetName );
	if ( w->output.status & FINE_FONT_SET_NAME_ALLOCATED )
	    {
	    w->common.fineFontSetName = oldw->common.fineFontSetName;
	    return;
	    }
	else
	    w->common.fineFontSetName = DEFAULT_ASIAN_FINE_FONT_SET_NAME;
	}
    
/* copy the new font set name into space we allocate and free the old name.
   The order is important because new might point to the old name (e.g.
   during initialization) */

    len = strlen( w->common.fineFontSetName ) + 1;
    ournew = XtMalloc( len );
    if ( ournew == NULL )
	{
	widget_message( w,
	  "Can't allocate space for new fine font name, length is %d bytes\n",
	  len );
	widget_message( w, "FineFontSetName not changed\n" );
	return;
	}
    memcpy( ournew, w->common.fineFontSetName, len );
    if ( w->output.status & FINE_FONT_SET_NAME_ALLOCATED )
	XtFree( oldw->common.fineFontSetName );
    else
	w->output.status |= FINE_FONT_SET_NAME_ALLOCATED;
    w->common.fineFontSetName = ournew;
    if ( w->source.wvt$l_ext_flags & vte1_m_tomcat &&
	 w->common.fontSetSelection == DECwFineFont )
	o_set_value_fontSetSelection( oldw, w );
#endif /* ALPHA_FINE_FONT_BUG */
}

/*

o_set_value_condensedFont - called when the condensedFont resource changes

*/

void o_set_value_condensedFont( oldw, w )
    DECtermWidget oldw, w;
{
    o_set_value_fontSetSelection( oldw, w );
}

/*

o_set_value_textCursorEnable - called when textCursorEnable resource changes

*/

void o_set_value_textCursorEnable( oldw, w )
    DECtermWidget oldw, w;
{
    if ( w->common.textCursorEnable )
	{
	o_enable_cursor( w, CURSOR_DISABLED_RESOURCE );
	if ( w->input.has_focus && w->common.cursorBlinkEnable )
	    start_blink_timer( w, 0 );
	}
    else
	o_disable_cursor( w, CURSOR_DISABLED_RESOURCE );
}	

/*

o_set_value_cursorStyle - called when cursorStyle resource changes

This routine is called to notify the output component that the cursor style
has changed.  It sets the new resource value in the context block and redraws
the cursor.

Output fields in context block:
	common.cursorStyle	set to new cursor style

*/

void o_set_value_cursorStyle( oldw, w )
    DECtermWidget oldw,w;		/* context pointer for widget */
{
    erase_old_cursor( oldw, w );
    o_display_cursor( w );
}

/*

o_set_value_scrollHorizontal - called when horizontal scroll bar is enabled
			       or disabled

*/

void o_set_value_scrollHorizontal( oldw, w )
    DECtermWidget oldw,w;
{
    w->output.status |= RESIZE_SCROLL_BARS | COMPUTE_OPTIMAL_SIZE;
    o_adjust_display( w );
}

/*

o_set_value_scrollVertical - called when vertical scroll bar is enabled or
			     disabled

*/

void o_set_value_scrollVertical( oldw, w )
    DECtermWidget oldw,w;
{
    w->output.status |= RESIZE_SCROLL_BARS | COMPUTE_OPTIMAL_SIZE;
    o_adjust_display( w );
}

/*

o_set_value_fontUsed - called when someone tries to change fontUsed.  Don't
		       let them!

*/

o_set_value_fontUsed( oldw, neww )
    DECtermWidget oldw, neww;
{
}


/*

o_set_value_useBoldFont - draw bold text using a bold font

*/

void o_set_value_useBoldFont( oldw, w )
    DECtermWidget oldw, w;
{
    w->common.really_use_bold_font = w->common.useBoldFont;
    /*
     * On a single plane system, use the bold font even if the user didn't
     * ask for it.
     */
    if ( DisplayPlanes( XtDisplay(w), DefaultScreen( XtDisplay(w) ) ) == 1 )
        w->common.really_use_bold_font = TRUE;
    /*
     * If the window has been initialized, redraw the display so that bold
     * text will be displayed according the new value of really_use_bold_font.
     */
    if ( w->output.status & WINDOW_INITIALIZED )
        o_repaint_display(w);
}

/*

o_set_display_width - called when the logical display width changes

This routine is called to notify the output component that the logical display
width has changed.  

Output fields in context block:
	output.top_visible_line		top line in window
	output.visible_rows		number of lines in window
	output.left_visible_column	left character in each line
	output.visible_columns		number of characters per line
	output.display_height		height of visible rows in pixels
	output.display_width		width of visible rows in pixels

*/

void o_set_display_width( w, new_columns )
    DECtermData *w;		/* widget context block */
    int new_columns;		/* new value of w->common.columns */
{
    int old_display_width,	/* display width when called */
	old_left_visible_column,
				/* left visible column when called */
	old_right_complete_column,
				/* rightmost column when called, not
				   including partial columns */
	move_distance,		/* number of pixels to scroll by */
	row;			/* loop index for character rows */

    w->output.columns = new_columns;
    w->output.status |= REDRAW_DISPLAY | COMPUTE_OPTIMAL_SIZE;
    o_adjust_display( w );
}

/*

o_destroy - called when widget is destroyed

This routine is called to notify the output component that the widget is being
destroyed.

*/

void o_destroy( w )
    DECtermData *w;
{
    int i;
    int j;

/*
 * Remove any lurking blink timer for this terminal.
 */

    if ( w->output.blink_set_flag) XtRemoveTimeOut (w->output.blink_id);

    if ( w->output.status & BIG_FONT_SET_NAME_ALLOCATED )
	XtFree( w->common.bigFontSetName );

    if ( w->output.status & LITTLE_FONT_SET_NAME_ALLOCATED )
	XtFree( w->common.littleFontSetName );

    if ( w->output.status & GS_FONT_SET_NAME_ALLOCATED )
	XtFree( w->common.gsFontSetName );

    if ( w->common.fontUsed != NULL )
    {
	XtFree( w->common.fontUsed );
	w->common.fontUsed = NULL;
    }

    if ( w->common.bigFontOtherName != NULL )
	XtFree( w->common.bigFontOtherName );

    if ( w->common.littleFontOtherName != NULL )
	XtFree( w->common.littleFontOtherName );

    if ( w->common.gsFontOtherName != NULL )
	XtFree( w->common.gsFontOtherName );

    if ( w->output.status & FINE_FONT_SET_NAME_ALLOCATED )
	XtFree( w->common.fineFontSetName );

    if ( w->output.status & WINDOW_INITIALIZED )
	{
	XFreeGC( XtDisplay(w), w->output.copy_area_gc );
	XFreeGC( XtDisplay(w), w->output.copy_area_graphics_expose_gc );
	XFreeGC( XtDisplay(w), w->output.normal_gc );
	XFreeGC( XtDisplay(w), w->output.reverse_gc );
	for ( i = 0; i < NUM_TEXT_GCS; i++ )
	    if ( w->output.text_gc[i] )
		XFreeGC( XtDisplay(w), w->output.text_gc[i] );
	XFreeGC( XtDisplay(w), w->output.complement_gc );
	}

    o_free_ansi_colors( w );

    for ( i = 0; i < FONTS_PER_SET; i++ )
	if ( w->output.font_list[i] )
	    {
	    j = i & ~(( BOLD_TEXT_GC_MASK | DOUBLE_WIDE_TEXT_GC_MASK |
			DOUBLE_SIZE_TEXT_GC_MASK ) >> 1 );
	    if (( i != j ) &&
		( w->output.font_list[i] == w->output.font_list[j] ))
		w->output.font_list[i] = NULL;
	    }
    for ( i = 0; i < FONTS_PER_SET; i++ )
	if ( w->output.font_list[i] ) {
	    XFreeFont( XtDisplay(w), w->output.font_list[i] );
	    w->output.font_list[i] = NULL;
	    }
	    
    w->output.status &= ~(BIG_FONT_SET_NAME_ALLOCATED |
      LITTLE_FONT_SET_NAME_ALLOCATED | WINDOW_INITIALIZED);

    XtRemoveEventHandler((Widget)w, VisibilityChangeMask, True,
			 (XtEventHandler)o_event_handler, 0 );
}

/*

o_convert_XY_to_position - convert window pixels to source cells

This routine converts a pixel coordinate relative to the window origin to
a cell coordinate in the logical display.  It purposely does NOT check that
the character cell is visible.

Input fields in context block:
	output.top_visible_line		top character line in window
	output.left_visible_column	left character column in window
	output.char_height		height of standard character in pixels
	output.char_width		width of standard character in pixels

*/

DECtermPosition o_convert_XY_to_position( w, x, y )
    DECtermWidget w;		/* widget context block */
    int x,			/* horizontal pixels from left of window */
	y;			/* vertical pixels from top of window */
{
    int row, column;
    s_line_structure *ls;

    row = ( y - Y_MARGIN ) / w->output.char_height + w->output.top_visible_line;
    column = ( x - X_MARGIN ) / w->output.char_width +
      w->output.left_visible_column;
    if ( column < w->output.left_visible_column )
	column = w->output.left_visible_column;
    if ( column > w->output.left_visible_column + w->output.visible_columns )
	column = w->output.left_visible_column + w->output.visible_columns;
    ls = s_read_data( w, row );
    if ( ls->b_rendits & ( csa_M_DOUBLE_WIDTH | csa_M_DOUBLE_HIGH ) )
	column /= 2;

    return ( DECtermPosition_position( column, row ) );
}


/*

o_convert_position_to_XY - convert source position to window pixels

This routine converts a pixel coordinate relative to the window origin to
a cell coordinate in the logical display.  It purposely does NOT check that
the character cell is visible.

Input fields in context block:
	output.top_visible_line		top character line in window
	output.left_visible_column	left character column in window
	output.char_height		height of standard character in pixels
	output.char_width		width of standard character in pixels

*/

void o_convert_position_to_XY( w, pos, xp, yp )
    DECtermWidget w;		/* widget context block */
    DECtermPosition pos;	/* source cell position */
    int *xp,			/* horizontal pixels from left of window */
	*yp;			/* vertical pixels from top of window */
{
    int row, column;
    s_line_structure *ls;

    row = DECtermPosition_row( pos );
    column = DECtermPosition_column( pos );
    ls = s_read_data( w, row );

    *xp = ( DECtermPosition_column( pos ) - w->output.left_visible_column )
	    * w->output.char_width
	    * ( ( ls->b_rendits & ( csa_M_DOUBLE_HIGH | csa_M_DOUBLE_WIDTH ) )
	    ? 2 : 1 ) + X_MARGIN;

    *yp = ( DECtermPosition_row( pos ) - w->output.top_visible_line )
	    * w->output.char_height + Y_MARGIN;
}

/*

o_convert_XY_to_pixels - convert display pixels to source pixels

This routine converts a pixel coordinate relative to the window origin to
a pixel coordinate relative to row 0, column 0.  It also returns the line
spacing in pixels, so the caller can convert the y coordinate to a character
line number.  It purposely does NOT check that the pixel is visible.

Input fields in context block:
	output.top_visible_line		top character line in window
	output.left_visible_column	left character column in window
	output.char_height		height of standard character in pixels
	output.char_width		width of standard character in pixels

*/

void o_convert_XY_to_pixels( w, x, y, sx, sy, line_spacing )
    DECtermData *w;		/* widget context block */
    int x,			/* horizontal pixels from left of window */
	y,			/* vertical pixels from top of window */
	*sx,			/* OUTPUT: horizontal pixels from column 0 */
	*sy,			/* OUTPUT: vertical pixels from row 0 */
	*line_spacing;		/* OUTPUT: standard character height */
{
    *sx = x - X_MARGIN + w->output.left_visible_column *
      w->output.char_width;
    *sy = y - Y_MARGIN + w->output.top_visible_line * w->output.char_height;
    *line_spacing = w->output.char_height;
}

/*

calculate_visible_area - determine the visible portion of the logical display

This routine is called internally when the window is created or the window
or logical display size changes, and calculates what I call the "display
area".  This is that area of the window that contains visible character cells:
it does not include the scroll bars or border, and it does not include
blank space between the last visible character in a row or column and the
border when the logical display is smaller than the window in that dimension.
Normally it preserves the character origin and adjusts the width and height;
however, it adjusts the character origin (top visible line and left visible
column) if needed so there is no blank space between the display area and the
border unless the entire logical display is visible in that dimension.

Note that we don't use common.rows and common.columns because these still
have their old values when the logical display is resized; see o_resize_display.

Input fields in context block:
	core.width			window width in pixels
	core.height			window height in pixels
	output.char_width		standard character width in pixels
	output.char_height		standard character height in pixels

Output fields in context block:
	output.initial_line		top character line in logical display
	output.top_visible_line		top character line in window
	output.visible_rows		number of character lines in window
	output.display_height		height of display area in pixels
	output.left_visible_column	left character in each line
	output.visible_columns		number of characters in each line
	output.display_width		width of display area in pixels

*/

static void calculate_visible_area( w )
    DECtermData *w;		/* widget context block */
{
    int rows,			/* logical display height in characters */
	visible_rows,		/* new visible display height in characters */
	old_display_width,	/* display width when called */
	old_display_height;	/* display height when called */

    old_display_width = w->common.display_width;
    old_display_height = w->common.display_height;

    rows = w->output.bottom_line - w->output.top_line + 1;
    w->common.logical_width = w->common.columns * w->output.char_width;
    w->common.logical_height = w->common.rows * w->output.char_height;

/* initially assume the display area is simply the window area without
   the border and scroll bars, rounded down to the next smaller character
   increment */

    w->common.display_width = w->core.width
      - ( w->common.scrollVertical ? SCROLL_BAR_WIDTH : 0 )
      - 2 * X_MARGIN;
    if ( w->common.display_width < 0 )
	w->common.display_width = 0;
    else
	w->common.display_width = ( w->common.display_width /
	  w->output.char_width ) * w->output.char_width;
    w->common.display_height = w->core.height
      - ( w->common.scrollHorizontal ? SCROLL_BAR_WIDTH : 0 )
      - ( w->common.statusDisplayEnable ? STATUS_LINE_HEIGHT : 0)
      - 2 * Y_MARGIN;
    if ( w->common.display_height < 0 )
	w->common.display_height = 0;
    else
	w->common.display_height = ( w->common.display_height /
	  w->output.char_height ) * w->output.char_height;

/* find how many characters are visible horizontally, possibly including a
   partial character at the right edge of the display area */

    w->output.visible_columns = ( w->common.display_width +
      w->output.char_width - 1 ) / w->output.char_width;

/* if the logical display doesn't reach the right edge of window, adjust the
   display area and/or the left visible column */

    if ( w->output.left_visible_column + w->output.visible_columns >
      w->output.columns )
	{  /* visible area is wider than l.d., so narrow the visible area */

/* adjust the left visible column so the right column of the logical display
   is at the right edge of the window */

	w->output.left_visible_column = w->output.columns
	  - w->output.visible_columns;
	if ( w->output.left_visible_column < 0 )
	    w->output.left_visible_column = 0;

/* if the window is still wider than the logical display, reduce the display
   area so it only includes visible character cells */

	if ( w->output.left_visible_column + w->output.visible_columns >
	  w->output.columns )
	    {  /* visible area is wider than l.d., so narrow the visible area */
	    w->output.visible_columns = w->output.columns
	      - w->output.left_visible_column;
	    w->common.display_width = w->output.visible_columns *
	      w->output.char_width;
	    }
	}

/* find how many characters are visible vertically, possibly including a
   partial character at the bottom edge of the display area */

    visible_rows = ( w->common.display_height +
      w->output.char_height - 1 ) / w->output.char_height;

/* if the visible area is shrinking, adjust the top visible line to
   keep the same bottom visible line as before */

    if ( visible_rows < w->output.visible_rows )
	w->output.top_visible_line += w->output.visible_rows - visible_rows;
    w->output.visible_rows = visible_rows;

/* if the logical display doesn't reach the bottom edge of the window,
   adjust the display area and/or the top visible line */

    if ( w->output.top_visible_line - w->output.top_line +
      w->output.visible_rows > rows )
	{  /* visible area is taller than l.d., so shorten the visible area */

/* adjust the top visible line so the bottom line of the logical display
   is at the bottom edge of the window */

	w->output.top_visible_line = rows - w->output.visible_rows +
	  w->output.top_line;
	if ( w->output.top_visible_line < w->output.top_line )
	    w->output.top_visible_line = w->output.top_line;

/* if the window is still taller than the logical display, reduce the display
   area so it only includes visible character cells */

	if ( w->output.top_visible_line - w->output.top_line +
	  w->output.visible_rows > rows )
	    {  /* visible area is taller than l.d.,
	          so shorten the visible area */
	    w->output.visible_rows = rows - w->output.top_visible_line +
	      w->output.top_line;
	    w->common.display_height = w->output.visible_rows *
	      w->output.char_height;
	    }
	}
    adjust_origin( w );

/* resize the scroll bars of the size has changed */

    if ( w->common.display_width != old_display_width
		&& w->common.scrollHorizontal
	    || w->common.display_height != old_display_height
		&& w->common.scrollVertical )
	{
	w->output.status |= RESIZE_SCROLL_BARS;
	}
}

/*

o_display_cursor - draw the cursor at the current position

This routine draws the cursor unless it is already visible.

Input/Output fields in context block:

	output.status & CURSOR_IS_VISIBLE	true if cursor has been drawn
						(set to True)

*/

void o_display_cursor( w )
    DECtermData *w;	/* context pointer for widget */
{
    if ( ! ( w->output.status & ( CURSOR_IS_VISIBLE | CURSOR_DISABLED ) ) )
	{  /* cursor not already visible, so draw it */
	complement_cursor( w );
	w->output.status |= CURSOR_IS_VISIBLE;
	}
}

/*

o_erase_cursor - erase the cursor

This routine erases the cursor if it is visible.

Note that the cursor is drawn in complement (GXinvert) mode, and is erased
by complementing it again.  That's why it's important to keep track of
whether the cursor is visible or not.

Input/Output fields in context block:

	output.status & CURSOR_IS_VISIBLE	true if cursor has been drawn
						(set to False)

*/

void o_erase_cursor( w )
    DECtermData *w;	/* context pointer for widget */
{
    if ( w->output.status & CURSOR_IS_VISIBLE
      && ! ( w->output.status & CURSOR_DISABLED ) )
	{  /* cursor has been drawn, so erase it */
	complement_cursor( w );
	w->output.status &= ~CURSOR_IS_VISIBLE;
	}
}

/*
                                  
erase_old_cursor -- erase cursor using old cursor style

*/

static void
erase_old_cursor( oldw, w )
    DECtermWidget oldw,w;
{
    DECwCursorStyle new_style = w->common.cursorStyle;

    w->common.cursorStyle = oldw->common.cursorStyle;
    o_erase_cursor( w );
    w->common.cursorStyle = new_style;
}

/*

complement cursor - display or erase the cursor

This routine actually draws or erases the cursor, using the current cursor
style.

Input fields in context block:
	output.status
		& OUTPUT_DISABLED	if True don't draw anything
	output.cursor_row		character row with cursor
	output.cursor_column		character column with cursor
	output.top_visible_line		top row in window
	output.left_visible_column	left column in window
	output.char_height		height of standard character in pixels
	output.char_ascent		position of baseline in standard char
	output.char_width		width of standard character in pixels
	output.cursorStyle		type of cursor to draw
	output.complement_gc		GC with GXinvert and clipping rectangle

*/
               
static void complement_cursor( w )
    DECtermData *w;		/* widget context block */
{
    int x, y;			/* character cell origin in pixels */
    int cursor_width;

/* make sure output isn't disabled */

    if ( w->output.status & OUTPUT_DISABLED )
	return;

/* find the character's coordinate */

    x = ( w->output.cursor_left - w->output.left_visible_column )
      * w->output.char_width + X_MARGIN;
    if (w->output.cursor_row == STATUS_LINE) {
        if (!(w->output.status & STATUS_LINE_VISIBLE))
	  return;
	y = w->output.status_line_top;
    } else {
        y = ( w->output.cursor_row - w->output.top_visible_line )
          * w->output.char_height + Y_MARGIN;
    }

/* make sure the character is at least partially visible */

    if ( w->output.cursor_double )
	cursor_width = w->output.char_width * 2;
    else
	cursor_width = w->output.char_width;
    if ( w->output.cursor_row == STATUS_LINE) {
      if (x >= w->common.display_width + X_MARGIN
	|| x + cursor_width <= X_MARGIN )
	  return;
    } else {
      if ( x >= w->common.display_width + X_MARGIN
        || x + cursor_width <= X_MARGIN
        || y >= w->common.display_height + Y_MARGIN
        || y + w->output.char_height <= Y_MARGIN )
	  return;
    }

/* underline cursor: draw a line two pixels wide at the bottom of the
   character cell (to avoid having to change the GC we'll just draw two
   lines) */

    if ( w->common.cursorStyle == DECwCursorUnderline )
	{
	y += w->output.char_height - 2;
	XDrawLine( XtDisplay(w), XtWindow(w), w->output.complement_gc,
	  x, y, x + cursor_width - 1, y );
	XDrawLine( XtDisplay(w), XtWindow(w), w->output.complement_gc,
	  x, y+1, x + cursor_width - 1, y+1 );
	}

/* block cursor: complement the entire character cell */

    else
	XFillRectangle( XtDisplay(w), XtWindow(w), w->output.complement_gc,
	  x, y, cursor_width, w->output.char_height );
}

/*

repaint_border - local routine to redraw the border

This routine draws a border around the display area.  The border is four
pixels wide: the outer two pixels in the foreground color and the inner two
pixels in the background color.  A special graphics context has to be used
because the border is outside the normal clipping rectangle.

Input fields in context block:
	output.status
		& OUTPUT_DISABLED	if True don't draw anything
	output.display_width		width of display area
	output.display_height		height of display area
	
*/

static void repaint_border( w )
    DECtermData *w;		/* context pointer for widget */
{

/* choose the appropriate colors: if screenMode is set the border is black
   and the background is white, and if screenMode is reset the border is white
   and the background is black.  This is easy to do:  we just use the
   NORMAL text GC. */

/* two byte (double width) characters may have been drawn between the external
   border and the internal one, so we have to clear them */

    if ( w->source.wvt$l_ext_flags & vte1_m_asian_common )
	clear_border(w);
    draw_border( w, w->output.normal_gc, w->output.reverse_gc,
      w->common.display_width + 2 * X_MARGIN,
      w->common.display_height + 2 * Y_MARGIN );
}

/*

erase_border - erase the border

This routine draws the border using the background color.

*/

static void erase_border( w )
    DECtermData *w;		/* widget context block */
{

/* choose the appropriate colors: if screenMode is set the border is black
   and the background is white, and if screenMode is reset the border is white
   and the background is black */

    draw_border( w, w->output.reverse_gc, w->output.normal_gc,
      w->common.display_width + 2 * X_MARGIN,
      w->common.display_height + 2 * Y_MARGIN );
}

/*

draw_border - draw the border using a given GC (color)

This routine is called by repaint_border and erase_border to actually draw
or erase the order.

*/

static void draw_border( w, fgc, bgc, width, height )
    DECtermData *w;		/* widget context block */
    GC fgc,			/* foreground graphics context */
	bgc;			/* background graphics context */
    int width,			/* width of display area including border */
	height;			/* height of display area including border */
{
    XPoint points[9];		/* array of endpoints of lines to draw */

/* make sure output isn't disabled */

    if ( w->output.status & OUTPUT_DISABLED )
	return;

/* set up the point array to draw a two pixel wide border in a spiral pattern */

    points[0].x = 0;
    points[0].y = height - 1;
    points[1].x = 0;
    points[1].y = 0;
    points[2].x = width - 1;
    points[2].y = 0;
    points[3].x = width - 1;
    points[3].y = height - 1;
    points[4].x = 1;
    points[4].y = height - 1;
    points[5].x = 1;
    points[5].y = 1;
    points[6].x = width - 2;
    points[6].y = 1;
    points[7].x = width - 2;
    points[7].y = height - 2;
    points[8].x = 2;
    points[8].y = height - 2;

/* draw the outer part of the border */

    XDrawLines( XtDisplay(w), XtWindow(w), fgc, points, 9, CoordModeOrigin );

/* on a pseudo color system the inner border is assumed to have been cleared
   to the background color, but on a system with shared colors the inner
   border needs to be drawn as well, in case the background color was changed
   with a ReGIS command */

    if ( w->common.graphics_mode == SINGLE_PLANE
      || w->common.graphics_mode == ALLOCATED_COLORS )
	{

/* set up the point array to draw a two pixel wide border in a spiral pattern */

	points[0].x = 2;
	points[0].y = height - 3;
	points[1].x = 2;
	points[1].y = 2;
	points[2].x = width - 3;
	points[2].y = 2;
	points[3].x = width - 3;
	points[3].y = height - 3;
	points[4].x = 3;
	points[4].y = height - 3;
	points[5].x = 3;
	points[5].y = 3;
	points[6].x = width - 4;
	points[6].y = 3;
	points[7].x = width - 4;
	points[7].y = height - 4;
	points[8].x = 4;
	points[8].y = height - 4;

/* draw the inner part of the border */

	XDrawLines( XtDisplay(w), XtWindow(w), bgc, points, 9, CoordModeOrigin );

	}
}


static void repaint_left_border( w, line )
    DECtermData *w;
    int line;
{
    GC gcf,gcb;
    int x, y1, y2;

    gcf = w->output.normal_gc;
    gcb = w->output.reverse_gc;

    y1 = (line - w->output.top_visible_line) * 
	    w->output.char_height + Y_MARGIN;
    y2 = y1 + w->output.char_height;
    XDrawLine( XtDisplay(w), XtWindow(w), gcf, 0, y1, 0, y2 );
    XDrawLine( XtDisplay(w), XtWindow(w), gcf, 1, y1, 1, y2 );
    for ( x = 2; x < X_MARGIN; x++ )
        XDrawLine( XtDisplay(w), XtWindow(w), gcb, x, y1, x, y2 );
}                   

static void repaint_right_border( w, line )
    DECtermData *w;
    int line;
{
    GC gcf,gcb;
    int x, y1, y2;
    int width, char_width;

    gcf = w->output.normal_gc;
    gcb = w->output.reverse_gc;

    y1 = (line - w->output.top_visible_line) * 
	    w->output.char_height + Y_MARGIN;
    y2 = y1 + w->output.char_height;
    width = w->common.display_width + 2 * X_MARGIN;
    char_width = w->output.char_width;
    x = width - 2;
    XDrawLine( XtDisplay(w), XtWindow(w), gcf, x, y1, x, y2 );
    XDrawLine( XtDisplay(w), XtWindow(w), gcf, x + 1, y1, x + 1, y2 );
    for ( x = width - X_MARGIN; x < width - 2; x++ )
        XDrawLine( XtDisplay(w), XtWindow(w), gcb, x, y1, x, y2 );
    for ( x = width; x < width - X_MARGIN + char_width; x++ )
        XDrawLine( XtDisplay(w), XtWindow(w), gcb, x, y1, x, y2 );
}

static void clear_border( w )
    DECtermData *w;
{
    GC gc;
    int x, y1, y2;
    int width, height, char_width;

    if ( w->output.status & OUTPUT_DISABLED )
	return;

    gc = w->output.reverse_gc;

    width = w->common.display_width + 2 * X_MARGIN;
    height = w->common.display_height + 2 * Y_MARGIN;
    char_width = w->output.char_width;
    y1 = 2;
    y2 = height - 3;
    for ( x = 2; x < X_MARGIN; x++ )
	XDrawLine( XtDisplay(w), XtWindow(w), gc, x, y1, x, y2 );
    for ( x = width - X_MARGIN; x < width - 2; x++ )
	XDrawLine( XtDisplay(w), XtWindow(w), gc, x, y1, x, y2 );
    for ( x = width; x < width - X_MARGIN + char_width; x++ )
	XDrawLine( XtDisplay(w), XtWindow(w), gc, x, y1, x, y2 );
}

/*

scroll_callback_proc - callback procedure for scroll bars

This routine is passed to the scroll bar widget as the callback procedure for
all scroll bar actions for both the horizontal and vertical scroll bars.  It
scrolls the display area as directed by user input.

Input fields in context block:
	output.h_scroll			horizontal scroll bar widget context
	output.v_scroll			vertical scroll bar widget context

Output fields in context block:
	output.top_visible_line		top line in window
	output.left_visible_column	left column in window

These fields are changed in pan_vertical and pan_horizontal, and changing
them can also affect other fields (such as the display width and height).

*/

static void scroll_callback_proc( scroll_widget, tag, data )
    Widget scroll_widget;	/* scroll bar widget context block */
    int tag;			/* not used */
    XmScrollBarCallbackStruct *data;
				/* data passed by scroll bar widget */
{
    DECtermWidget w;		/* DECterm widget context block */
    int row, column;		/* calculated character row/column number */

    w = (DECtermWidget)scroll_widget->core.parent;
				/* point to the DECterm widget */
    if ( w->output.status & SCROLL_ADJUST_IN_PROGRESS )
	return;  /* emulator, not user, asked for scroll bars to change */

    switch ( data->reason )
	{
	case XmCR_VALUE_CHANGED:
	case XmCR_INCREMENT:
	case XmCR_DECREMENT:
	case XmCR_PAGE_DECREMENT:
	case XmCR_PAGE_INCREMENT:
	case XmCR_DRAG:
	    /* all these reasons change the value */
	    if ( scroll_widget == w->output.h_scroll.widget )
		{  /* scrolling horizontally */
		pan_horizontal( w, min( data->value, w->output.columns -
	          w->output.visible_columns ) );
		}
	    else if ( scroll_widget == w->output.v_scroll.widget )
		{  /* scrolling vertically */
		pan_vertical( w, min( data->value, w->output.bottom_line -
		  w->output.visible_rows + 1 ) );
		}
	    else
		widget_message( w,
		  "Received unknown scroll widget context address %d!\n",
		  scroll_widget );
	    break;
	case XmCR_TO_TOP:
	case XmCR_TO_BOTTOM:
	    if ( scroll_widget == w->output.h_scroll.widget )
		{  /* scrolling horizontally */
		column = ( data->pixel - X_MARGIN ) / w->output.char_width +
			w->output.left_visible_column;
		if ( data->reason == XmCR_TO_TOP )
		    pan_horizontal( w, column );
		else
		    pan_horizontal( w, column - w->output.visible_columns + 1 );
		}
	    else
		{  /* scrolling vertically */
		row = ( data->pixel - Y_MARGIN ) / w->output.char_height +
			w->output.top_visible_line;
		if ( data->reason == XmCR_TO_TOP )
		    pan_vertical( w, row );
		else
		    pan_vertical( w, row - w->output.visible_rows + 1 );
		}
	    break;
        default:	/* XmCR_DRAG, XmCR_HELP */
	    /* don't do anything while output is dragged */
	    /* don't provide any help (yet?) */
	    break;
	}
    adjust_scroll_bars( w );	/* make sure scroll bars reflect reality */
}

/*

pan_horizontal - scroll the display horizontally

This routine is called to scroll the display horizontally in response to
scroll bar input.

Input fields in context block:
	output.char_width		width of standard character in pixels
	output.copy_area_gc		GC with clipping rectangle and
					possibly graphics exposures enabled
	output.display_width		width of display area in pixels
	output.display_height		height of display area in pixels
	output.top_visible_line		top text line in window
	output.visible_rows		number of text rows in window

Input/Output fields in context block:
	output.left_visible_column	leftmost text column in window

*/

static void pan_horizontal( w, column )
    DECtermData *w;	/* context block */
    int column;		/* column number to put at left edge */
{
    int unclipped_width;	/* width not including partial characters */
    int move_distance;		/* number of pixels to move by */
    int row;			/* loop index for character rows */
    int left_exposed_column;	/* leftmost character to repaint in each row */
    int exposed_columns;	/* number of characters to repaint each row */
    int old_left_column;	/* left visible column when called */

    if ( column == w->output.left_visible_column )
	return;			/* already at that position */
    old_left_column = w->output.left_visible_column;
    if ( column + w->output.visible_columns > w->common.columns )
	column = w->common.columns - w->output.visible_columns + 1;
    if ( column < 0 )
	column = 0;
    w->output.left_visible_column = column;
    adjust_origin( w );

    if ( column < old_left_column )
	{  /* panning to the right */

/* panning to the right (e.g. hit the left scroll button):
   first move the window contents to the right */

	move_distance = ( old_left_column - column ) *
	  w->output.char_width;
	if ( move_distance < w->common.display_width ) {
	    XCopyArea( XtDisplay(w), XtWindow(w), XtWindow(w),
	      w->output.copy_area_gc, X_MARGIN, Y_MARGIN,
	      w->common.display_width - move_distance, w->common.display_height,
	      X_MARGIN + move_distance, Y_MARGIN );
	    if ( w->output.status & PARTIALLY_OBSCURED )
		add_scroll_entry( w, 0 );
			/* dummy entry since there is no vertical movement */
	    if (w->output.status & STATUS_LINE_VISIBLE) {
	      XCopyArea( XtDisplay(w), XtWindow(w), XtWindow(w),
	        w->output.copy_area_gc, X_MARGIN, w->output.status_line_top,
	        w->common.display_width - move_distance,
	        STATUS_LINE_HEIGHT, X_MARGIN + move_distance,
	        w->output.status_line_top);
	     if ( w->output.status & PARTIALLY_OBSCURED )
	     	add_scroll_entry( w, 0 );
	    }
	}

/* update the left portion of the window (scrolled into from outside the
   window) from the display list */

	o_update_rectangle( w, w->output.top_visible_line,
	  w->output.top_visible_line + w->output.visible_rows - 1,
	  column, old_left_column - column, NULL );

	if (w->output.status & STATUS_LINE_VISIBLE) {
	    update_segment( w, STATUS_LINE, column, old_left_column - column );
	}
	}
    else
	{  /* panning to the left */

/* panning to the left (e.g. hit the right scroll button):
   first move the window to the right, but don't bother to copy partial
   characters at the right edge of the window */

	move_distance = ( column - old_left_column ) * w->output.char_width;
	if ( move_distance < w->common.display_width ) {
	    unclipped_width = ( w->common.display_width / w->output.char_width )
	      * w->output.char_width;
	    XCopyArea( XtDisplay(w), XtWindow(w), XtWindow(w),
	      w->output.copy_area_gc, X_MARGIN + move_distance, Y_MARGIN,
	      unclipped_width - move_distance, w->common.display_height,
	      X_MARGIN, Y_MARGIN );
	    if ( w->output.status & PARTIALLY_OBSCURED )
		add_scroll_entry( w, 0 );
			/* dummy entry since there's no vertical movement */
	    if (w->output.status & STATUS_LINE_VISIBLE) {
	      XCopyArea( XtDisplay(w), XtWindow(w), XtWindow(w),
	        w->output.copy_area_gc, X_MARGIN + move_distance,
	        w->output.status_line_top, unclipped_width - move_distance,
	        STATUS_LINE_HEIGHT, X_MARGIN,
	        w->output.status_line_top );
	      if ( w->output.status & PARTIALLY_OBSCURED )
		add_scroll_entry( w, 0 );
	    }
	}

/* now update the right part of the window (which was scrolled into from
   outside the window) from the display list */

	left_exposed_column = w->common.display_width / w->output.char_width +
	  old_left_column;
	exposed_columns = column - old_left_column +
          ( ( w->common.display_width % w->output.char_width == 0 ) ? 0 : 1 );
	o_update_rectangle( w, w->output.top_visible_line,
	  w->output.top_visible_line + w->output.visible_rows - 1,
	  left_exposed_column, exposed_columns, NULL );
	if (w->output.status & STATUS_LINE_VISIBLE) {
	    update_segment( w, STATUS_LINE, left_exposed_column, exposed_columns );
	}
	}
/* two byte (double width) characters may have corrupted the border, so we
   have to repaint it */

    if ( w->source.wvt$l_ext_flags & vte1_m_asian_common )
	repaint_border( w );
    o_display_cursor( w );
}

/*

pan_vertical - scroll the display vertically

This routine is called to scroll the display vertically in response to
scroll bar input, in contrast to o_scroll_lines which is called when the
display list is scrolled in response to output from the application (line
feeds).

Input fields in context block:
	output.char_height		height of standard character in pixels
	output.copy_area_gc		GC with clipping rectangle and
					possibly graphics exposures enabled
	output.display_height		height of display area in pixels
	output.display_width		width of display area in pixels
	output.left_visible_column	leftmost text column in window
	output.visible_columns		number of text columns in window

Input/Output fields in context block:
	output.top_visible_line		top text row in window

*/

static void pan_vertical( w, row )
    DECtermData *w;		/* context block */
    int row;			/* row to put at top of window */
{
    int unclipped_height;	/* height not including partial characters */
    int move_distance;		/* number of pixels to move by */
    int old_top_row;		/* top visible line when called */

    if ( row == w->output.top_visible_line )
	return;			/* already at that position */
    old_top_row = w->output.top_visible_line;
    if ( row + w->output.visible_rows - 1 > w->output.bottom_line )
	row = w->output.bottom_line - w->output.visible_rows + 1;
    if ( row < w->output.top_line )
	row = w->output.top_line;

    if ( row < old_top_row )
	{  /* panning down */

/* panning down (e.g. hit the top scroll buttom):
   first move the window contents downwards */

	w->output.top_visible_line = row;
	adjust_origin( w );
	move_distance = ( old_top_row - row ) * w->output.char_height;
	if ( move_distance < w->common.display_height ) {
	    XCopyArea( XtDisplay(w), XtWindow(w), XtWindow(w),
	      w->output.copy_area_gc, X_MARGIN, Y_MARGIN,
	      w->common.display_width, w->common.display_height - move_distance,
	      X_MARGIN, Y_MARGIN + move_distance );
	    if ( w->output.status & PARTIALLY_OBSCURED )
		add_scroll_entry( w, old_top_row - row );
	    if ( row_is_marked( w, old_top_row ) )
	         complement_row_marker( w, old_top_row,
	         w->output.left_visible_column, w->output.visible_columns );
	}
/* update the top portion of the window (scrolled into from outside the
   window) from the display list */

	o_update_rectangle( w, row, old_top_row - 1,
	  w->output.left_visible_column, w->output.visible_columns, NULL );
	}
    else
	{  /* panning up */

/* panning up (e.g. hit the bottom scroll button):
   first move the window upwards, but don't bother to copy partial
   characters at the bottom edge of the window */

	if ( row_is_marked( w, row ) )
	    complement_row_marker( w, row,
	      w->output.left_visible_column, w->output.visible_columns );
	w->output.top_visible_line = row;
	adjust_origin( w );
	move_distance = ( row - old_top_row ) * w->output.char_height;
	if ( move_distance < w->common.display_height ) {
	    unclipped_height = ( w->common.display_height /
	      w->output.char_height ) * w->output.char_height;
	    XCopyArea( XtDisplay(w), XtWindow(w), XtWindow(w),
	      w->output.copy_area_gc, X_MARGIN, Y_MARGIN + move_distance,
	      w->common.display_width, unclipped_height - move_distance,
	      X_MARGIN, Y_MARGIN );
	    if ( w->output.status & PARTIALLY_OBSCURED )
		add_scroll_entry( w, row - old_top_row );
	}
/* now update the bottom part of the window (which was scrolled into from
   outside the window) from the display list */

	o_update_rectangle( w, w->common.display_height / w->output.char_height
	  + old_top_row, row + w->output.visible_rows - 1,
	  w->output.left_visible_column, w->output.visible_columns, NULL );
	}
    o_display_cursor( w );
}

/*
adjust_scroll_bars - update horizontal and vertical scroll bars

This routine sets the value, maxValue, minValue, shown, inc, and pageInc
resources in the horizontal and vertical scroll bars, depending on what
values have changed since the last update.  It does not change the size
or position of the scroll bars.

*/

static void adjust_scroll_bars( w )
    DECtermData *w;		/* DECterm widget context block */
{
    Arg arg_list[5];		/* argument list for XtSetValues */
    int num_values;		/* number of values to change */

    w->output.status |= SCROLL_ADJUST_IN_PROGRESS;
				/* don't want the callbacks to scroll the
				   screen AGAIN! */

/* update horizontal scroll bar */

    if ( w->common.scrollHorizontal )
	{
	num_values = 0;
	if ( w->output.h_scroll.value != w->output.left_visible_column )
	    {
	    w->output.h_scroll.value = w->output.left_visible_column;
	    arg_list[num_values].name = XmNvalue;
	    arg_list[num_values++].value = w->output.h_scroll.value;
	    }
	if ( w->output.h_scroll.maxValue != w->output.columns )
	    {
	    w->output.h_scroll.maxValue = w->output.columns;
	    arg_list[num_values].name = XmNmaximum;
	    arg_list[num_values++].value = w->output.h_scroll.maxValue;
	    }
	if ( w->output.h_scroll.shown != w->output.visible_columns )
	    {
	    w->output.h_scroll.shown = w->output.visible_columns;
	    arg_list[num_values].name = XmNsliderSize;
	    arg_list[num_values++].value = w->output.h_scroll.shown;
	    arg_list[num_values].name = XmNpageIncrement;
	    arg_list[num_values++].value = w->output.h_scroll.shown - 1;
	    }
	if ( num_values > 0 )
	    XtSetValues( w->output.h_scroll.widget, arg_list, num_values );
	}

/* update vertical scroll bar */

    if ( w->common.scrollVertical )
	{
	num_values = 0;
	if ( w->output.v_scroll.value != w->output.top_visible_line )
	    {
	    w->output.v_scroll.value = w->output.top_visible_line;
	    arg_list[num_values].name = XmNvalue;
	    arg_list[num_values++].value = w->output.v_scroll.value;
	    }
	if ( w->output.v_scroll.maxValue != w->output.bottom_line + 1 )
	    {
	    w->output.v_scroll.maxValue = w->output.bottom_line + 1;
	    arg_list[num_values].name = XmNmaximum;
	    arg_list[num_values++].value = w->output.v_scroll.maxValue;
	    }
	if ( w->output.v_scroll.minValue != w->output.top_line )
	    {
	    w->output.v_scroll.minValue = w->output.top_line;
	    arg_list[num_values].name = XmNminimum;
	    arg_list[num_values++].value = w->output.v_scroll.minValue;
	    }
	if ( w->output.v_scroll.shown != w->output.visible_rows )
	    {
	    w->output.v_scroll.shown = w->output.visible_rows;
	    arg_list[num_values].name = XmNsliderSize;
	    arg_list[num_values++].value = w->output.v_scroll.shown;
	    arg_list[num_values].name = XmNpageIncrement;
	    arg_list[num_values++].value = w->output.v_scroll.shown - 1;
	    }
	if ( num_values > 0 )
	    XtSetValues( w->output.v_scroll.widget, arg_list, num_values );
	}

    w->output.status &= ~SCROLL_ADJUST_IN_PROGRESS;
}

/*

o_update_rectangle - refresh line segments from several lines from the
		   display list

This routine is equivalent to making several calls to update_segment
for consecutive rows.  It makes one call to regis_update_rectangle
instead of a call per line.

This routine is originally named update_rectangle.  "o_" is added and
"static" is removed so that DT_PRINTER can call it.

*/

void o_update_rectangle (w, top, bottom, index, count, pixmap)
    DECtermWidget w;
    int top, bottom, index, count;
    Pixmap pixmap;
{
    int row;

    if ( (w->output.status & OUTPUT_DISABLED) && pixmap == NULL )
	return;
    if ( top < w->output.top_visible_line )
	top = w->output.top_visible_line;
    if ( bottom >= w->output.top_visible_line + w->output.visible_rows )
	bottom = w->output.top_visible_line + w->output.visible_rows - 1;
    if ( top > bottom )
	return;
    if ( w->common.backing_store_active )
	regis_update_rectangle( w, ( index - w->output.left_visible_column )
	  * w->output.char_width + X_MARGIN,
	  ( top - w->output.top_visible_line ) * w->output.char_height
	  + Y_MARGIN, count * w->output.char_width, ( bottom - top + 1 )
	  * w->output.char_height, pixmap );
    for ( row = top; row <= bottom; row++ )
	o_update_segment( w, row, index, count, FALSE, pixmap );
}

/*

update_segment - refresh a line segment from the display list

This routine is similar to o_display_segment except that the character index
and count are always in terms of standard character cells, regardless of the
line renditions (i.e. the index and count are twice what they should be in
double width lines).  If graphics are visible they are refreshed from the
pixmap backing store and then over-written with text.

*/

static void update_segment (w, line, index, count)
     DECtermData *w ;
     int line, index, count ;
    {o_update_segment (w, line, index, count, 1, NULL);}

void o_update_segment( w, line, index, count, update_regis_flag, pixmap )
    DECtermData *w;
    int line, index, count;
    Boolean update_regis_flag;
    Pixmap pixmap;
{
    s_line_structure *ls;
    int left_x, right, char_width, x, y, width, height, left_visible_column;

/* make sure at least part of the segment is visible in the window and that
   output isn't disabled */

    if ( (w->output.status & OUTPUT_DISABLED) && pixmap == NULL )
	return;
    if ( line != STATUS_LINE) {
      if ( line < w->output.top_visible_line
        || line >= w->output.top_visible_line + w->output.visible_rows
        || line < w->output.top_line
        || line > w->output.bottom_line )
	  return;
    }
    if ( line == STATUS_LINE && !(w->output.status & STATUS_LINE_VISIBLE))
	return;
    if (index + count <= w->output.left_visible_column
        || index >= w->output.left_visible_column + w->output.visible_columns
        || count <= 0 )
    	  return;

/* if the left end of the segment is outside the window, clip the block to the
   left edge of the window */

    if ( index < w->output.left_visible_column )
	{
	count -= w->output.left_visible_column - index;
	index = w->output.left_visible_column;
	}

/* if the right end of the segment is outside the window, clip the segment to
   the right edge of the window */

    if ( index + count > w->output.left_visible_column +
      w->output.visible_columns )
	count = w->output.left_visible_column + w->output.visible_columns -
	  index;

    x = ( index - w->output.left_visible_column ) * w->output.char_width
      + X_MARGIN;
    if ( line == STATUS_LINE ) {         
	y = w->output.status_line_top;
    } else {
        y = ( line - w->output.top_visible_line ) * w->output.char_height
          + Y_MARGIN;
    }
    width = count * w->output.char_width;
    height = w->output.char_height;

/* draw the graphics under the exposed area */

    if (update_regis_flag)
	if ( w->common.regisScreenMode && w->common.backing_store_active )
	    regis_update_rectangle( w, x, y, width, height, pixmap );

    ls = s_read_data( w, line );
    right = index + count - 1;
    char_width = w->output.char_width;
    left_visible_column = w->output.left_visible_column;
    if ( ls->b_rendits & ( csa_M_DOUBLE_WIDTH | csa_M_DOUBLE_HIGH ) )
	{
	index = index / 2;	/* round down */
	right = (right + 1) / 2;		/* round up */
	left_visible_column /= 2;
	char_width *= 2;
	left_x = ( index - left_visible_column ) * char_width + X_MARGIN;
	if ( w->output.left_visible_column & 1) left_x -= char_width / 2;
	}
    else {
	left_x = ( index - left_visible_column ) * char_width + X_MARGIN;
    }
    count = right - index + 1;
    if ( w->common.backing_store_active )
	w->output.status |= UPDATE_IN_PROGRESS;
    draw_segment( w, line, index, count, left_x, ls, pixmap );
    w->output.status &= ~UPDATE_IN_PROGRESS;
}


Pixel rendition_color(w, attribute, foreground_request)
DECtermData *w;
REND attribute;
Boolean foreground_request;
{
int color_index;

/*
 * VT340-style color text.  Choose a ReGIS color index
 * based on the bold, reverse and blink attributes.
 */
if ( attribute & csa_M_BLINK )
    {
    if ( attribute & csa_M_BOLD )
	{
	if (foreground_request)	/* for foreground */
	    {
	    if ( attribute & csa_M_REVERSE )
		color_index = 2; /* blink/bold/reverse foreground = normal*/
	    else
		color_index = 1; /* blink/bold foreground = dim */
	    }
	else			/* for background */
	    {
	    if ( attribute & csa_M_REVERSE )
		color_index = 1; /* blink/bold/reverse background = dim */
	    else
		color_index = 2; /* blink/bold background = normal */
	    }
	}
    else
	{
	if (foreground_request) /* for foreground */
	    {
	    if ( attribute & csa_M_REVERSE )
		color_index = 2; /* blink/reverse foreground = normal */
	    else
		color_index = 0; /* blink/normal foreground = black */
	    }
	else			/* for background */
	    {
	    if ( attribute & csa_M_REVERSE )
		color_index = 0; /* blink/reverse background = black */
	    else
		color_index = 2; /* blink/normal background = normal */
	    }
	}
    }
else
    {
    if ( attribute & csa_M_BOLD )
	{
	if (foreground_request)
	    {
	    if ( attribute & csa_M_REVERSE )
		color_index = 0;  /* bold/reverse foreground = black */
	    else
		color_index = 3; /* bold foreground = bright */
	    }
	else
	    {
	    if ( attribute & csa_M_REVERSE )
		color_index = 3; /* bold/reverse background = bright */
	    else
		color_index = 0; /* bold background = black */
	    }
	}
    else
	{
	if (foreground_request)
	    {
	    if ( attribute & csa_M_REVERSE )
		color_index = 0; /* reverse foreground = black */
	    else
		color_index = 2; /* normal forground = normal */
	    }
	else
	    {
	    if ( attribute & csa_M_REVERSE )
		color_index = 2; /* reverse background = normal */
	    else
		color_index = 0; /* normal background = black */
	    }
	}
    }

/*
 * If we're using a color other than the normal text
 * foreground or background color, make sure the
 * graphics color map is allocated.
 */
if ( color_index == 1 || color_index == 3 )
    {
    w->common.graphics_visible = TRUE;
    if ( ! w->common.color_map_allocated )
	allocate_color_map( w );
    }

/*
 * If we're emulating 4 (or more -- currently not
 * allowed) bit planes, convert from VT330 to VT340
 * color map indices.
 */
if ( w->common.bitPlanes >= 4 )
    switch ( color_index )
	{
	case 1:     /* dim */
	    color_index = 8;
	    break;
	case 2:     /* normal */
	    color_index = 7;
	    break;
	case 3:     /* bright */
	    color_index = 15;
	    break;
	}

/*
 * Convert from a color map index to a pixel value.
 */

return (regis_fetch_color( w, color_index ));

}


static void draw_segment( w, line, index, count, left_x, ls, pixmap )
    DECtermData *w;
    int line, index, count, left_x;
    s_line_structure *ls; /* characters and attributes from display list */
    Pixmap pixmap;       /* draw to this pixmap if pixmap != NULL */
{
    Boolean some_blink_flag = 0;
			/* indicates whether any characters are blinking */
    int start_column,	/* first column in a group with the same attributes */
	column,		/* loop index for character columns */
	char_width,	/* character width in pixels (possibly doubled) */
	baseline_y,	/* baseline (in pixels) for drawing characters */
	char_is_null,	/* True means the characters in this block are nulls */
	gc_mask,	/* index of text GC to use */
	ewidth,		/* width of area to erase */
	y,
	color_index;	/* index for ANSI colors (0=black, etc.) */

    Boolean highlight
	= FALSE;	/* TRUE if this block is highlighted */
    GC gc;		/* graphics context block to use */
    REND attribute;	/* attribute for current character block */
    XRectangle rect;	/* clipping rectangle for double high characters */
    XGCValues values;	/* values block for resetting clipping rectangle */
    Pixel foreground, background, temp_color;
    EXT_REND ext_attribute;	/* extended attribute for current block  */
    Drawable drawable;

    /* draw the image to drawable instead of XtWindow(w) */
    
    drawable = (Drawable) ((pixmap == NULL) ? XtWindow(w) : pixmap);

    if ( ls->b_rendits & ( csa_M_DOUBLE_WIDTH | csa_M_DOUBLE_HIGH ) )
	char_width = w->output.char_width * 2;
    else
	char_width = w->output.char_width;

    baseline_y = 0;

/* If there is a highlight transition in this character block, then
   split this into two calls to draw_segment */

        {

        if ( index + count > ls->highlight_begin &&
             index < ls->highlight_end )
	    {	/* segment is completely or paritally highlighted */
	    int left_index = index;
	    int right_index = index + count;

	    if ( left_index < ls->highlight_begin )
		{			/* do left unhighlighted portion */
		left_index = ls->highlight_begin;
		draw_segment( w, line, index, left_index-index, left_x, ls,
			      pixmap );
		}

	    if ( right_index > ls->highlight_end )
		{			/* do right unhighlighted portion */
		right_index = ls->highlight_end;
		draw_segment( w, line, right_index, index + count - right_index,
			      left_x + (right_index-index) * char_width, ls,
			      pixmap );
		}

	    left_x += (left_index-index) * char_width;
	    index = left_index;
	    count = right_index - left_index;
	    highlight = TRUE;	/* do remaining highlighted portion */

	    }
	}

/* erase the cursor if it is within the character block.  */

    if ( w->output.cursor_row == line && index <= w->output.cursor_right
      && w->output.cursor_left < index + count )
	o_erase_cursor( w );

/* outer loop: find each block of contiguous characters with the same
   attributes */

    for ( start_column = index; start_column < index + count;
       left_x += ( column - start_column ) * char_width, start_column = column )
	{
/*
 * Define "r" macro, which yields a rendition value with the blink bit
 * xored with current blink phase.  This allows characters which are
 * blinking but currently off to appear as the same rendition as abutting
 * normal characters, hence allowing them to be drawn together.
 */
#define r(v) (((v)&csa_M_BLINK && w->output.char_blink_phase&2) ? \
		(v) : (v)&~csa_M_BLINK)

/* inner loop: for each character with the same attribute.  Null (zero)
   characters are a special case: they are treated as having their own
   attribute, i.e. if a block includes any null characters all the characters
   in the block must be null.  This is because the null characters are
   drawn by erasing a rectangle instead of being drawn as text. */

      if (ls->w_widths == 0) char_is_null = TRUE;
      else

	for ( column = start_column, attribute=r(ls->a_rend_base[start_column]),
	  ext_attribute=(ls->a_ext_rend_base[start_column] & ~csa_M_BYTE_POSITION),
	  char_is_null = ( ls->a_code_base[start_column] == NULL );
	  column < index + count && r(ls->a_rend_base[column]) == attribute
	  && (ls->a_ext_rend_base[column] & ~csa_M_BYTE_POSITION) == ext_attribute
	  && ( ls->a_code_base[column] == NULL ) == char_is_null; column++ )
	    {
	    }

/* if this set of characters includes any blinking ones, remember */

	some_blink_flag |= ls->a_rend_base[start_column] & csa_M_BLINK ? 1 : 0;

/* now we've reached the end of a block of characters, so draw it.  If the
   block contains null characters, erase the rectangle of character cells */

	if ( char_is_null )
	    {  /* for nulls, just erase a rectangle */
	    if ( ! ( w->output.status & UPDATE_IN_PROGRESS ) )
		{  /* make sure we aren't refreshing a rectangle */
		if ( ! highlight )
		    gc = w->output.reverse_gc;
		else
		    gc = w->output.normal_gc;
		if ( line == STATUS_LINE ) {
		    if ( line == STATUS_LINE && !(w->output.status & STATUS_LINE_VISIBLE))
			return;
		    y = w->output.status_line_top;
		} else {
		    y = (line - w->output.top_visible_line)
		      * w->output.char_height + Y_MARGIN;
		}
		ewidth = ( column - start_column ) * char_width;

/* clip the region for the case of double-wide lines */

		if ( left_x < X_MARGIN ) {
		    ewidth -= X_MARGIN - left_x;
		    left_x = X_MARGIN;
		}
		if ( left_x + ewidth > X_MARGIN + w->common.display_width ) {
		    ewidth = X_MARGIN + w->common.display_width - left_x;
		}

		XFillRectangle( XtDisplay(w), drawable, gc, left_x,
				y, ewidth, w->output.char_height );
		}
		if ( w->source.wvt$l_ext_flags & vte1_m_asian_common ) {
		    if ( column >= ( w->output.left_visible_column
				   + w->output.visible_columns ))
			repaint_right_border( w, line );
		}
	    }
	else
	    { /* otherwise, draw the characters in replace mode */

/* determine which graphics context to use.  Normally we use white text on
   a black background, but reverse these if in reverse video screen mode and
   reverse again if
   blink mode (which if on at this point means blink phase is currently
   such that character should be reversed),
   and reverse them again if the reverse video attribute is set. */

	    if ( (   ( (attribute & csa_M_REVERSE) != 0)
		   ^ ( (attribute & csa_M_BLINK) != 0)
		   ^ highlight
		 ) & 1 )
			/* if reverse video attribute XOR reverse screen
			      XOR highlight */
		gc_mask = REVERSE_TEXT_GC_MASK;
	    else
		gc_mask = 0;
	    if ( attribute & csa_M_BOLD )
		gc_mask |= BOLD_TEXT_GC_MASK;
	    if ( ls->b_rendits & csa_M_DOUBLE_HIGH )
		gc_mask |= DOUBLE_SIZE_TEXT_GC_MASK;
	    if ( ls->b_rendits & csa_M_DOUBLE_WIDTH )
		gc_mask |= DOUBLE_WIDE_TEXT_GC_MASK;

/* mapping of character sets to fonts is terminal-specific */

	    if ( w->source.wvt$l_ext_flags & vte1_m_asian_common ) {
		switch ( ext_attribute & csa_M_EXT_CHAR_SET )
		    {
		    case STANDARD_SET:
			switch ( attribute & csa_M_CHAR_SET )
			    {
			    case ASCII:
				break;
			    case LINE_DRAWING:
				break;
			    case TECHNICAL:
				gc_mask |= TECH_TEXT_GC_MASK;
				break;
			    case APL:
				break;
			    case DRCS_FONT:  
				gc_mask |= DRCS_TEXT_GC_MASK;
				break;
			    case ISO_LATIN_1:
				break;
			    case SUPPLEMENTAL:
				break;
			    }
			break;

		    case ONE_BYTE_SET:
			switch ( attribute & csa_M_CHAR_SET )
			    {
			    case JIS_ROMAN:
				gc_mask |= ROMAN_TEXT_GC_MASK;
				break;
			    case JIS_KATAKANA:
				gc_mask |= ROMAN_TEXT_GC_MASK;
				break;
			    case CRM_FONT_L:
				gc_mask |= TECH_TEXT_GC_MASK;
				break;
			    case CRM_FONT_R:
				gc_mask |= ROMAN_TEXT_GC_MASK;
				break;
			    case KS_ROMAN:
				gc_mask |= KS_ROMAN_TEXT_GC_MASK;
				break;
			    }
			break;

		    case TWO_BYTE_SET:
			switch ( attribute & csa_M_CHAR_SET )
			    {
       			    case DEC_KANJI:
				gc_mask |= KANJI_TEXT_GC_MASK;
				break;           
       			    case DEC_HANZI:
				gc_mask |= HANZI_TEXT_GC_MASK;
				break;           
       			    case DEC_HANGUL:
				gc_mask |= HANGUL_TEXT_GC_MASK;
				break;           
			    }
			break;
		    case FOUR_BYTE_SET:
			switch ( attribute & csa_M_CHAR_SET )
			    {
       			    case DEC_HANYU:
				gc_mask |= HANYU_TEXT_GC_MASK;
				break;
			    case DEC_HANYU_4:
				gc_mask |= HANYU_4_TEXT_GC_MASK;
				break;           
			    }
			break;

		    default:
			break;
		    }
	    } else {
	    switch ( ext_attribute & csa_M_EXT_CHAR_SET )
	    {

	    case ONE_BYTE_SET:
	    switch ( attribute & csa_M_CHAR_SET ) {
		case ISO_LATIN_8:
		case HEB_SUPPLEMENTAL:
		    gc_mask |= HEB_TEXT_GC_MASK;
		    break;
	    }
	    break;

	    case STANDARD_SET:
	    if ( ( attribute & csa_M_CHAR_SET ) == TECHNICAL )
		gc_mask |= TECH_TEXT_GC_MASK;
	    if ( ( attribute & csa_M_CHAR_SET ) == APL )
		gc_mask |= APL_TEXT_GC_MASK;
	    if ( ( attribute & csa_M_CHAR_SET ) == DRCS_FONT )
		gc_mask |= DRCS_TEXT_GC_MASK;

	    break;
	    default:
	    break;
	    }
	    }

/* if we're using the VT340 method of drawing bold, toggle the reverse
   attribute if screen mode is set (i.e. we're in reverse video mode).  Note
   that if really_use_bold_font is true (i.e. if we're not in VT340 color
   text mode) and screen mode is set, the foreground and background color
   resource values have been exchanged, so we shouldn't toggle the
   reverse attribute. */

            if ( ( w->common.screenMode ^ highlight ) != 0
              && ! w->common.really_use_bold_font )
                attribute ^= csa_M_REVERSE;

/* create the GC if it doesn't exist */

	    if ( ! w->output.text_gc[ gc_mask ] )
		{
		w->output.text_gc[ gc_mask ] =
			XCreateGC( XtDisplay(w), drawable, 0, 0 );
		w->output.foreground_pixel[ gc_mask ] = 0;	/* default */
		w->output.background_pixel[ gc_mask ] = 1;
		w->output.gc_font_valid[ gc_mask ] = FALSE;
		}
	    gc = w->output.text_gc[ gc_mask ];

/* Ansi color is specified */
	    if (attribute & csa_M_NODEFAULT_TEXT ||
		attribute & csa_M_NODEFAULT_TEXT_BCK)
	        {

/* First, set foreground and background dis-regarding the REVERSE_TEXT_GC_MASK
 * at this point.
 */

		if (attribute & csa_M_NODEFAULT_TEXT)
		    {
		    color_index	= ( attribute & MASK_TEXT ) >> MASK_TEXT_SHIFT;
		    foreground = get_ansi_color( w, color_index );
		    }
		else
		    foreground = w->manager.foreground;

		if (attribute & csa_M_NODEFAULT_TEXT_BCK)
		    {
		    color_index	= ( attribute & MASK_TEXT_BCK ) >>
						MASK_TEXT_BCK_SHIFT;
		    background = get_ansi_color( w, color_index );
		    }
		else
		    background = w->core.background_pixel;

/* if REVERSE is requested then swap foreground and background
 */
		if ( (gc_mask & REVERSE_TEXT_GC_MASK) )
		    {
		    temp_color = foreground;
		    foreground = background;
		    background = temp_color;
		    }
                }
	    else  /* normal case when no ansi color is set */

		/* set the colors if they aren't what we want */
		{   
                    /*
                     * If the foreground color hasn't been over-ridden by
                     * an ANSI color, see if we have to use a color from
                     * the graphics color map.
                     */
                if ( w->common.really_use_bold_font )
                    foreground = ( gc_mask & REVERSE_TEXT_GC_MASK ) ?
                      w->core.background_pixel : w->manager.foreground;
                else
		    foreground = rendition_color(w, attribute, TRUE);
                    /*
                     * If the background color hasn't been over-ridden by
                     * an ANSI color, see if we have to use a color from
                     * the graphics color map.
                     */
                if ( w->common.really_use_bold_font )
                    background = ( gc_mask & REVERSE_TEXT_GC_MASK ) ?
                      w->manager.foreground : w->core.background_pixel;
                else
		    background = rendition_color(w, attribute, FALSE);

	        } /* else.  No Ansi color is specified */

/* reset the GC's foreground and background color if needed
 */
	    if ( w->output.foreground_pixel[ gc_mask ] != foreground )
		{
		XSetForeground( XtDisplay(w), w->output.text_gc[ gc_mask ],
				    foreground );
		w->output.foreground_pixel[ gc_mask ] = foreground;
		}

	    if ( w->output.background_pixel[ gc_mask ] != background )
		{
		XSetBackground( XtDisplay(w), w->output.text_gc[ gc_mask ],
			            background );
		w->output.background_pixel[ gc_mask ] = background;
		}

/* make sure the font we want has been loaded */

	    if ( ! w->output.gc_font_valid[ gc_mask ] )
		{
		if ( ! w->output.font_list[ gc_mask >> 1 ] )
		    open_font( w, gc_mask );
		XSetFont( XtDisplay(w), w->output.text_gc[ gc_mask ],
			w->output.font_list[ gc_mask >> 1 ]->fid );
		w->output.gc_font_valid[ gc_mask ] = TRUE;
		}

/* find the x and y positions (in pixels) to draw the block of characters */

	    if (line == STATUS_LINE) {
		baseline_y = w->output.status_line_top + w->output.char_ascent;
	    } else {
	        baseline_y = ( line - w->output.top_visible_line ) *
	          w->output.char_height + w->output.char_ascent + Y_MARGIN;
	    }

/* assume the double high fonts have twice the ascent of the base font */

	    if ( ls->b_rendits & csa_M_DOUBLE_HIGH ) {
		baseline_y += w->output.char_ascent;
		if ( ls->b_rendits & csa_M_DOUBLE_BOTTOM ) {
		    baseline_y -= w->output.char_height;
		}
	    }

#ifdef MEASURE_KEYBOARD_PERFORMANCE
	    if ( key_enabled )
		{
		XEvent event;
		event.type = ClientMessage;
		event.xclient.display = XtDisplay(w);
		event.xclient.window = XtWindow(w);
		event.xclient.format = 32;
		event.xclient.data.l[0] = KEYBOARD_ECHO;
		event.xclient.data.l[1] = (long) ls->a_code_base[start_column];
		XSendEvent( XtDisplay(w), XtWindow(w), True, NoEventMask,
		  &event );
		}
#endif

/* for double high/wide lines, set the clipping rectangle */

	    if ( ls->b_rendits & ( csa_M_DOUBLE_HIGH | csa_M_DOUBLE_WIDTH ) )
		{
		rect.x = X_MARGIN;
		if (line == STATUS_LINE) {
		    rect.y = w->output.status_line_top;
		} else {
		    rect.y = ( line - w->output.top_visible_line ) *
		      w->output.char_height + Y_MARGIN;
		}
		rect.width = w->common.display_width;
		rect.height = w->output.char_height;
		XSetClipRectangles( XtDisplay(w), gc, 0, 0, &rect,1,YXBanded );
		}

/* draw the characters, including the cell backgrounds */

	    /* draw to pixmap if it is non_-NULL */
	    if ( w->source.wvt$l_ext_flags & vte1_m_asian_common )
		draw_image_string( w, gc_mask, left_x,
		  baseline_y, line, ls, start_column, column, pixmap );
	    else

	    /* drawable can be XtWindow(w) or pixmap */
	    XDrawImageString( XtDisplay(w), drawable, gc, left_x,
	      baseline_y, (char *)&ls->a_code_base[start_column],
	      column - start_column );

/* restore the clipping rectangle */

	    if ( ls->b_rendits & ( csa_M_DOUBLE_HIGH | csa_M_DOUBLE_WIDTH ) )
		{
		values.clip_mask = None;
		XChangeGC( XtDisplay(w), gc, GCClipMask, &values );
		}

/* underline attribute: draw a line under the character block.  Note that
   we draw 2 pixels beneath the baseline because we have to be beneath the
   underscore character, which is 1 pixel beneath the baseline */

	    if ( attribute & csa_M_UNDERLINE )

		/* draw underline to drawable instead */
		if ( w->source.wvt$l_ext_flags & vte1_m_asian_common )
		    XDrawLine( XtDisplay(w), drawable, gc, left_x, 
		      baseline_y - w->output.char_ascent + w->output.char_height - 1,
		      left_x + ( column - start_column ) * char_width - 1,
		      baseline_y - w->output.char_ascent + w->output.char_height - 1 );
		else

		/* draw underline to drawable instead */
		XDrawLine( XtDisplay(w), drawable, gc, left_x, baseline_y,
		  left_x + ( column - start_column ) * char_width - 1,
		  baseline_y );
	    }

	}

/* draw a marker if this line is marked */

    if ( row_is_marked( w, line ) )
	{
	if ( ls->b_rendits & ( csa_M_DOUBLE_WIDTH | csa_M_DOUBLE_HIGH ) )
	    complement_row_marker( w, line, index * 2, count * 2 );
	else
	    complement_row_marker( w, line, index, count );
	}

/* if any characters are blinking, make sure timer is set, and remember
 * that this DECterm contains some blinking characters */

	if (some_blink_flag)
	    {
	    w->output.some_blink_flag = 1;
	    start_blink_timer (w, 0);
	    }
}

/*

row_is_marked - determine whether a marker has been set at a text row

*/

Bool row_is_marked( w, line )
    DECtermData *w;
    int line;
{
    int marker;

    for ( marker = 0; marker < NUMBER_OF_MARKERS; marker++ )
	if ( w->output.marker_is_set & ( 1 << marker )
	  && w->output.marker_row[ marker ] == line )
	    return True;
    return False;
}

/*

complement_row_marker - draw or erase a marker at a text row

*/
static void complement_row_marker( w, line, index, count )
    DECtermData *w;
    int line, index, count;
{
    int x1, x2, y;

    if ( w->output.status & OUTPUT_DISABLED)
	return;
    if ( line != STATUS_LINE && (
         line <= w->output.top_visible_line	/* don't draw line at top */
      || line >= w->output.top_visible_line + w->output.visible_rows ) )
	return;
    if ( line == STATUS_LINE && !(w->output.status & STATUS_LINE_VISIBLE))
	return;
    if ( line == STATUS_LINE ) {
	y = w->output.status_line_top;
    } else {
        y = ( line - w->output.top_visible_line ) * w->output.char_height +
          Y_MARGIN;
    }
    x1 = ( index - w->output.left_visible_column ) * w->output.char_width +
      X_MARGIN;
    x2 = x1 + count * w->output.char_width - 1;

    XDrawLine( XtDisplay(w), XtWindow(w), w->output.complement_gc,
      x1, y, x2, y );
}

/*
 * o_display_region -- display a region of the display
 *
 * This is like o_display_segment but it takes DECtermPositions as
 * arguments, and it can cross lines.
 *
 */

void o_display_region( w, begin, end )
    DECtermWidget w;
    DECtermPosition begin, end;
{
    int line, column;
    DECtermPosition cursor;

    if ( begin == end ) return;
    if ( begin > end )
	{
	DECtermPosition temp = begin;
	begin = end;
	end = temp;
	}

    cursor = DECtermPosition_position( w->output.cursor_column,
      w->output.cursor_row );
    if ( begin <= cursor && cursor <= end )
	o_erase_cursor( w );

    for ( line = DECtermPosition_row(begin),
	  column = DECtermPosition_column(begin);
	  line < DECtermPosition_row(end);
          line++,
	  column = 0 )
    {
	o_display_segment( w, line, column, MAX_COLUMN );
    }

    o_display_segment( w, line, column,
		       DECtermPosition_column(end)-column );

    if ( begin <= cursor && cursor <= end )
	o_display_cursor( w );
}

/*

o_set_foreground_color - change the value of the foreground resource

ReGIS calls this routine when it allocates color map entry 7.  The routine
updates the appropriate GC values.

*/

void o_set_foreground_color( w, pixel )
    DECtermWidget w;		/* DECterm widget context */
    Pixel pixel;		/* new foreground color */
{
    int i;		/* loop index for GCs */

    w->manager.foreground = pixel;
    w->core.border_pixel = pixel;
    if ( ! ( w->output.status & WINDOW_INITIALIZED ) )
	return;
    XSetWindowBorder( XtDisplay(w), XtWindow(w), w->manager.foreground );
    XSetForeground( XtDisplay(w), w->output.normal_gc, pixel );
    XSetBackground( XtDisplay(w), w->output.reverse_gc, pixel );
    o_set_cursor_plane_mask( w );
    repaint_border( w );
}

/*

o_set_background_color - change the value of the background resource

ReGIS calls this routine when it allocates color map entry 0.  The routine
updates the appropriate GC values.

*/

void o_set_background_color( w, pixel )
    DECtermWidget w;		/* DECterm widget context */
    Pixel pixel;		/* new background color */
{
    int i;		/* loop index for GCs */
    int dim;            /* color map index for dim */

    w->core.background_pixel = pixel;
    if ( ! ( w->output.status & WINDOW_INITIALIZED ) )
	return;

/* if we're in reverse mode and we're emulating VT340 color text, set
   the window background to graphics color 8 instead of to the background
   resource. */

    if ( w->common.screenMode && ! w->common.really_use_bold_font
      && w->common.color_map_allocated )
        {
        if ( w->common.bitPlanes < 4 )
            dim = 1;
        else
            dim = 8;
        pixel = w->common.color_map[dim].pixel;
        }

    XSetWindowBackground( XtDisplay(w), XtWindow(w), pixel );
    XSetBackground( XtDisplay(w), w->output.normal_gc, pixel );
    XSetForeground( XtDisplay(w), w->output.reverse_gc, pixel );
    o_set_cursor_plane_mask( w );
    repaint_border( w );
}

/* adjust_origin - adjust the origin of the logical display */

adjust_origin( w )
    DECtermWidget w;
{
    w->common.origin_x = w->output.left_visible_column * w->output.char_width;
    w->common.origin_y = ( w->output.top_visible_line - w->output.initial_line )
      * w->output.char_height;
    regis_adjust_origin( w );
}

/* open_font - open a font file */

open_font( w, gc_mask )
    DECtermWidget w;
    int gc_mask;
{
    int height_points, height_pixels, height_size, font_index, average_width,
      family_name_size;
    char the_name[ MAX_FONT_NAME_SIZE + 1 ], *font_name_resource, *base_font,
      height_str[ 10 ], *height_ptr, **font_name_list, *family_name_ptr;
    XFontStruct *font_info;
/*
 * Guarantee a null at the end of whatever string we're dealt.
 */
    the_name[MAX_FONT_NAME_SIZE] = 0;

    font_index = gc_mask >> 1;

/* font sets are terminal-specific */

    if ( w->source.wvt$l_ext_flags & vte1_m_asian_common ) {
    if (( gc_mask & BOLD_TEXT_GC_MASK ||
		( gc_mask & DOUBLE_SIZE_TEXT_GC_MASK ) ||
		( gc_mask & DOUBLE_WIDE_TEXT_GC_MASK )))
	{
	int real_font_index = 0;

	switch ( gc_mask & CHAR_SET_GC_MASK ) {
	    case TECH_TEXT_GC_MASK: 
	    case ROMAN_TEXT_GC_MASK: 
	    case KANJI_TEXT_GC_MASK: 
	    case HANZI_TEXT_GC_MASK: 
	    case HANGUL_TEXT_GC_MASK: 
	    case HANYU_TEXT_GC_MASK: 
	    case HANYU_4_TEXT_GC_MASK:
	    case KS_ROMAN_TEXT_GC_MASK: 
		real_font_index = ( gc_mask & CHAR_SET_GC_MASK ) >> 1;
		break;
	    default:
		break; 
	}
	if ( !w->output.font_list[ real_font_index ] )
	    open_font( w, real_font_index << 1 );

	w->output.font_list[ font_index ] =
		w->output.font_list[ real_font_index ];
	return;
        }

    if ( w->common.fontSetSelection == DECwBigFont )
	font_name_resource = w->common.bigFontSetName;
    else if ( w->common.fontSetSelection == DECwFineFont )
	font_name_resource = w->common.fineFontSetName;
    else if ( w->common.fontSetSelection == DECwGSFont )
	font_name_resource = w->common.gsFontSetName;
    else
	font_name_resource = w->common.littleFontSetName;

    strcpy( the_name, font_name_resource );

    switch ( gc_mask & CHAR_SET_GC_MASK ) {
    case TECH_TEXT_GC_MASK:
	DLFDSetFontNameField( DLFD_CHARSET_REGISTRY, the_name, 3, "DEC" );
        DLFDSetFontNameField( DLFD_CHARSET_ENCODING, the_name, 7, "DECtech" );
	break;
    case DRCS_TEXT_GC_MASK:
	DLFDSetFontNameField( DLFD_CHARSET_REGISTRY, the_name, 3, "DEC" );
        DLFDSetFontNameField( DLFD_CHARSET_ENCODING, the_name, 4, "DRCS" );
	break;
    case ROMAN_TEXT_GC_MASK:
        DLFDSetFontNameField( DLFD_FOUNDRY, the_name, 5, "JDECW" );
        DLFDSetFontNameField( DLFD_CHARSET_REGISTRY, the_name, 8, "JISX0201" );
	DLFDSetFontNameField( DLFD_CHARSET_ENCODING, the_name, 9, "RomanKana" );
	break;
    case KANJI_TEXT_GC_MASK:
        DLFDSetFontNameField( DLFD_FOUNDRY, the_name, 5, "JDECW" );
        DLFDSetFontNameField( DLFD_CHARSET_REGISTRY, the_name, 8, "JISX0208" );
        DLFDSetFontNameField( DLFD_CHARSET_ENCODING, the_name, 7, "Kanji11" );
	break;
    case HANZI_TEXT_GC_MASK:
        DLFDSetFontNameField( DLFD_FOUNDRY, the_name, 5, "ADECW" );
        DLFDSetFontNameField( DLFD_CHARSET_REGISTRY, the_name, 11,
	"GB2312.1980" );
        DLFDSetFontNameField( DLFD_CHARSET_ENCODING, the_name, 1, "1" );
	break;
    case HANGUL_TEXT_GC_MASK:
        DLFDSetFontNameField( DLFD_FOUNDRY, the_name, 5, "ADECW" );
        DLFDSetFontNameField( DLFD_CHARSET_REGISTRY, the_name, 12,
	"KSC5601.1987" );
        DLFDSetFontNameField( DLFD_CHARSET_ENCODING, the_name, 1, "1" );
	break;
    case KS_ROMAN_TEXT_GC_MASK:
        DLFDSetFontNameField( DLFD_FOUNDRY, the_name, 5, "ADECW" );
        DLFDSetFontNameField( DLFD_CHARSET_REGISTRY, the_name, 2, "ks" );
        DLFDSetFontNameField( DLFD_CHARSET_ENCODING, the_name, 5, "roman" );
	break;
    case HANYU_TEXT_GC_MASK:
        DLFDSetFontNameField( DLFD_FOUNDRY, the_name, 5, "ADECW" );
        DLFDSetFontNameField( DLFD_CHARSET_REGISTRY, the_name, 17,
	"DEC.CNS11643.1986" );
        DLFDSetFontNameField( DLFD_CHARSET_ENCODING, the_name, 1, "2" );
	break;
    case HANYU_4_TEXT_GC_MASK:
        DLFDSetFontNameField( DLFD_FOUNDRY, the_name, 5, "ADECW" );
        DLFDSetFontNameField( DLFD_CHARSET_REGISTRY, the_name, 14,
	"DEC.DTSCS.1990" );
        DLFDSetFontNameField( DLFD_CHARSET_ENCODING, the_name, 1, "2" );
	break;
    default:
	DLFDSetFontNameField( DLFD_CHARSET_REGISTRY, the_name, 3, "DEC" );
	DLFDSetFontNameField( DLFD_CHARSET_ENCODING, the_name, 8, "DECsuppl" );
	break;
    }
    } else {
    if ( w->common.fontSetSelection == DECwBigFont )
	base_font = w->common.bigFontSetName;
    else if ( w->common.fontSetSelection == DECwGSFont )
	base_font = w->common.gsFontSetName;
    else
	base_font = w->common.littleFontSetName;
    if ( font_index == FONT_NORMAL )
	{
	font_name_resource = base_font;
	strncpy( the_name, font_name_resource, MAX_FONT_NAME_SIZE );
	}
    else
	{
	/*
	 * If this is not the base font in the font set, start with fontUsed.
	 * This is the actual name used to open the base font.  That way if
	 * we fallback to a different font set we can still pick up bold etc.
	 * fonts in that font set.
	 *
	 * For Hebrew, do not use the w->common.fontUsed (may be 
	 * a Bitstream for example).
	 */
        if (( gc_mask & CHAR_SET_GC_MASK ) == HEB_TEXT_GC_MASK )
	    font_name_resource = base_font;
	else
	font_name_resource = w->common.fontUsed;
	strncpy( the_name, font_name_resource, MAX_FONT_NAME_SIZE );

	/*
	 * If the font family was wild carded in the base font, wild card it
	 * in the other fonts as well.
	 */
	family_name_ptr = DLFDGetFontNameField( DLFD_FAMILY_NAME, base_font,
	  &family_name_size );
	if ( family_name_size == 1 && family_name_ptr[0] == '*' )
	    {
	    DLFDSetFontNameField( DLFD_FAMILY_NAME, the_name, 1, "*" );
	    }
	}

    /*
     * Use gc_mask to set fields in the font name.
     */
    height_ptr = DLFDGetFontNameField( DLFD_POINT_SIZE, the_name,
      &height_size );
    strncpy( height_str, height_ptr, height_size );
    height_str[ height_size ] = '\0';
    height_points = atoi( height_str );
    height_ptr = DLFDGetFontNameField( DLFD_PIXEL_SIZE, the_name,
      &height_size );
    strncpy( height_str, height_ptr, height_size );
    height_str[ height_size ] = '\0';
    height_pixels = atoi( height_str );
    height_ptr = DLFDGetFontNameField( DLFD_AVERAGE_WIDTH, the_name,
      &height_size );
    strncpy( height_str, height_ptr, height_size );
    height_str[ height_size ] = '\0';
    average_width = atoi( height_str );
    height_ptr = DLFDGetFontNameField( DLFD_AVERAGE_WIDTH, the_name,
      &height_size );
    strncpy( height_str, height_ptr, height_size );
    height_str[ height_size ] = '\0';
    average_width = atoi( height_str );

    if ( gc_mask & BOLD_TEXT_GC_MASK && w->common.really_use_bold_font )
	DLFDSetFontNameField( DLFD_WEIGHT_NAME, the_name, 4, "Bold" );
    else
	DLFDSetFontNameField( DLFD_WEIGHT_NAME, the_name, 6, "Medium" );

    if ( w->common.condensedFont )
	if ( gc_mask & DOUBLE_WIDE_TEXT_GC_MASK
	  && ! ( gc_mask & DOUBLE_SIZE_TEXT_GC_MASK ) )
	    DLFDSetFontNameField( DLFD_SETWIDTH_NAME, the_name, 4, "Wide" );
	else
	    DLFDSetFontNameField( DLFD_SETWIDTH_NAME, the_name, 6, "Narrow" );
    else if ( gc_mask & DOUBLE_WIDE_TEXT_GC_MASK
      && ! ( gc_mask & DOUBLE_SIZE_TEXT_GC_MASK ) )
	DLFDSetFontNameField( DLFD_SETWIDTH_NAME, the_name, 11,
	  "Double Wide" );
    else
	DLFDSetFontNameField( DLFD_SETWIDTH_NAME, the_name, 6, "Normal" );

    /*
     * Specifying the slant speeds up XOpenFont.
     */
    DLFDSetFontNameField( DLFD_SLANT, the_name, 1, "R" );

    if ( gc_mask & (TECH_TEXT_GC_MASK | APL_TEXT_GC_MASK | DRCS_TEXT_GC_MASK) )
	DLFDSetFontNameField( DLFD_CHARSET_REGISTRY, the_name, 3, "DEC" );
    else
	DLFDSetFontNameField( DLFD_CHARSET_REGISTRY, the_name, 7, "ISO8859" );

    if (( gc_mask & CHAR_SET_GC_MASK ) == HEB_TEXT_GC_MASK ) {
	DLFDSetFontNameField( DLFD_CHARSET_REGISTRY, the_name, 7, "ISO8859" );
	DLFDSetFontNameField( DLFD_CHARSET_ENCODING, the_name, 1, "8" );
    } else
    if ( gc_mask & TECH_TEXT_GC_MASK )
	DLFDSetFontNameField( DLFD_CHARSET_ENCODING, the_name, 7, "DECtech" );
    else if ( gc_mask & APL_TEXT_GC_MASK )
	DLFDSetFontNameField( DLFD_CHARSET_ENCODING, the_name, 18,
		"DECAPLSupplemental" );
    else if ( gc_mask & DRCS_TEXT_GC_MASK )
	DLFDSetFontNameField( DLFD_CHARSET_ENCODING, the_name, 4, "DRCS" );
    else if (w->common.terminalType == DECwTurkish) /* iso8859-9 for Turkish */
	DLFDSetFontNameField( DLFD_CHARSET_ENCODING, the_name, 1, "9" );
    else if (w->common.terminalType == DECwGreek)   /* iso8859-7 for Greek */
	DLFDSetFontNameField( DLFD_CHARSET_ENCODING, the_name, 1, "7" );
    else
	DLFDSetFontNameField( DLFD_CHARSET_ENCODING, the_name, 1, "1" );

    if ( gc_mask & DOUBLE_SIZE_TEXT_GC_MASK )
	{
	if ( height_points > 0 )
	    {
	    sprintf( height_str, "%d", height_points * 2 );
	    DLFDSetFontNameField( DLFD_POINT_SIZE, the_name,
	      strlen( height_str ), height_str );
	    }
	if ( height_pixels > 0 )
	    {
	    sprintf( height_str, "%d", height_pixels * 2 );
	    DLFDSetFontNameField( DLFD_PIXEL_SIZE, the_name,
	      strlen( height_str ), height_str );
	    }
	}
    }

    if ( gc_mask & (DOUBLE_WIDE_TEXT_GC_MASK | DOUBLE_SIZE_TEXT_GC_MASK) )
	{
	if ( average_width > 0 )
	    {
	    sprintf( height_str, "%d", average_width * 2 );
	    DLFDSetFontNameField( DLFD_AVERAGE_WIDTH, the_name,
	      strlen( height_str ), height_str );
	    }
	}

    if ( gc_mask & (DOUBLE_WIDE_TEXT_GC_MASK | DOUBLE_SIZE_TEXT_GC_MASK) )
	{
	if ( average_width > 0 )
	    {
	    sprintf( height_str, "%d", average_width * 2 );
	    DLFDSetFontNameField( DLFD_AVERAGE_WIDTH, the_name,
	      strlen( height_str ), height_str );
	    }
	}

    font_info = load_font( XtDisplay(w), the_name, &font_name_list );

    /*
     * If the load failed AND the foundry name is not wildcarded, try 
     * wildcarding it
     */

    if ( font_info == NULL && *(strchr( the_name, '-') + 1) != '*')
      {
      DLFDSetFontNameField( DLFD_FOUNDRY, the_name, 1, "*" );
      font_info = load_font( XtDisplay(w), the_name, &font_name_list );
      };

    /*
     * If we're trying to load a GS font and it fails, try again without
     * the -GS-.  Don't display an error message unless this second attempt
     * also fails.
     */

    if ( font_info == NULL && w->common.fontSetSelection == DECwGSFont )
	{
	DLFDSetFontNameField( DLFD_ADD_STYLE_NAME, the_name, 0, "" );
	font_info = load_font( XtDisplay(w), the_name, &font_name_list );
	}

    /*
     * If the load failed AND the foundry name is not wildcarded, try 
     * wildcarding it
     */

    if ( font_info == NULL && *(strchr( the_name, '-') + 1) != '*')
      {
      DLFDSetFontNameField( DLFD_FOUNDRY, the_name, 1, "*" );
      font_info = load_font( XtDisplay(w), the_name, &font_name_list );
      };

    if ( font_info == NULL )
	{
	widget_error( w, DECW$K_MSG_CANT_FIND_FONT, 0, the_name );
	/*
	 * We weren't able to open the font, so choose a fallback font.  If
	 * this is not the base font in the font set, then we should be able
	 * to use the base font.  Don't just re-use the font info from
	 * font_list, though, because that would give us an error later when
	 * we tried to free the base font twice.
	 */
	font_info = load_font( XtDisplay(w), font_name_resource,
		&font_name_list );	
	if ( font_info != NULL )
	    widget_message( w, "Using fallback font %s\n", font_name_list[0] );
	else
	    {
	    if ( w->common.fontSetSelection == DECwBigFont )
		font_name_resource = DEFAULT_BIG_FONT_SET_NAME;
	    else if ( w->common.fontSetSelection == DECwGSFont )
		font_name_resource = DEFAULT_GS_FONT_SET_NAME;
	    else
		font_name_resource = DEFAULT_LITTLE_FONT_SET_NAME;
	    font_info = load_font( XtDisplay(w), font_name_resource,
			&font_name_list );
	    if ( font_info != NULL )
		widget_message( w, "Using fallback font %s\n",
			font_name_list[0] );
	    }
	if ( font_info == NULL )
	    {
	    strncpy( the_name, "-*-Courier-*-*-*--*-*-*-*-*-*-*-*",
		MAX_FONT_NAME_SIZE );
	    if ( height_points > 0 )
		{
		sprintf( height_str, "%d", height_points );
		DLFDSetFontNameField( DLFD_POINT_SIZE, the_name,
			strlen( height_str ), height_str );
		}
	    else
		{
		sprintf( height_str, "%d", height_pixels );
		DLFDSetFontNameField( DLFD_PIXEL_SIZE, the_name,
			strlen( height_str ), height_str );
		}
	    font_info = load_font( XtDisplay(w), the_name, &font_name_list );
	    if ( font_info != NULL )
		widget_message( w, "Using fallback font %s\n",
			font_name_list[0] );
	    }
	if ( font_info == NULL )
	    {
	    font_info = load_font( XtDisplay(w), "*-Courier-*",
			&font_name_list );
	    if ( font_info != NULL )
		widget_message( w, "Using fallback font %s\n",
			font_name_list[0] );
	    }
	if ( font_info == NULL )
	    {
	    font_info = load_font( XtDisplay(w), "Fixed", &font_name_list );
	    if ( font_info != NULL )
		widget_message( w, "Using fallback font %s\n",
			font_name_list[0] );
	    }
	if ( font_info == NULL )
	    {
	    /*
	     * All our fallback attempts have failed.  This is obviously a
	     * brain-dead system, so we'll just gracefully exit.
	     */
	    widget_error( w, DECW$K_MSG_EXIT_NO_FONT, 0, 0 );
	    }
	}

    /*
     * At this point we've successfully opened a font, so store its font_info
     * structure and possibly its name.  Don't forget to free font_name_list.
     */
    w->output.font_list[ font_index ] = font_info;
    if ( font_index == FONT_NORMAL )
	{
	/*
	 * We're loading the base font, so set its name in the fontUsed
	 * resource.
	 */
	if ( w->common.fontUsed != NULL )
	{
	    XtFree( w->common.fontUsed );
	    w->common.fontUsed = NULL;
	}

	w->common.fontUsed = XtMalloc( strlen( font_name_list[0] ) + 1 );
	strcpy( w->common.fontUsed, font_name_list[0] );
	}
    XFreeFontNames( font_name_list );
}	

/*
 * load_font - load a font given a pattern that matches it
 *
 * This routine also provides a list of fonts that match the pattern; the
 * font actually opened is the first one in this list.  The list should be
 * freed with XFreeFontNames.  The list will be NULL if the font couldn't
 * be loaded.
 */

XFontStruct *load_font( display, pattern, font_name_list )
    Display *display;
    char *pattern, ***font_name_list;
{
    int actual_count;
    XFontStruct *font_info;

    /*
     * First find a font that matches the pattern.
     */
    *font_name_list = XListFonts( display, pattern, 1, &actual_count );
    if ( actual_count == 0 )
	return NULL;			/* no font matches that pattern */
    /*
     * Open the font.
     */
    font_info = XLoadQueryFont( display, (*font_name_list)[0] );
    /*
     * If the font couldn't be opened, free the font name list.
     */
    if ( font_info == NULL )
	{
	XFreeFontNames( *font_name_list );
	*font_name_list = NULL;
	}
    return font_info;
}

void o_adjust_display( w )
    DECtermWidget w;
{
    int row, hw, hh, vw, vh;
    XEvent event;

/* the only reason o_adjust_display is called is because the window is about
   to change size for some reason.  Calculate the new size immediately. */
	calculate_visible_area( w );

/* figure out where the status line belongs */
    if ( w->common.statusDisplayEnable ) {
      w->output.status_line_top = ( w->core.height
        - ( w->common.scrollHorizontal ? SCROLL_BAR_WIDTH : 0 )
        - ( w->common.statusDisplayEnable ? STATUS_LINE_HEIGHT : 0)
        + 1) ;
    }

    if ( w->output.status & COMPUTE_OPTIMAL_SIZE )
	{
	w->output.status &= ~COMPUTE_OPTIMAL_SIZE;
	compute_optimal_size( w );
	}

    if ( w->output.status & DISABLE_REDISPLAY )
	return;

/* Don't do any of the window stuff now if not initialized; but clear
   flags since it will be done when window is initialized.  Otherwise,
   it will never get cleared. */

    if ( !(w->output.status & WINDOW_INITIALIZED) )
	{
	w->output.status &= ~( RESIZE_SCROLL_BARS | REDRAW_DISPLAY );
	return;
	}

/* make the scroll bars invisible if they have been disabled.  This is also
   when we have to recompute the clipping rectangles for ReGIS */

    if ( w->output.status & RESIZE_SCROLL_BARS )
	{
	if ( ( ! w->common.scrollHorizontal )
	  && ( w->output.status & HORIZONTAL_SCROLL_BAR_MAPPED ) )
	    {
	    w->output.status &= ~HORIZONTAL_SCROLL_BAR_MAPPED;
	    XtUnmanageChild( w->output.h_scroll.widget );
	    }
	if ( ( ! w->common.scrollVertical )
	  && ( w->output.status & VERTICAL_SCROLL_BAR_MAPPED ) )
	    {
	    w->output.status &= ~VERTICAL_SCROLL_BAR_MAPPED;
	    XtUnmanageChild( w->output.v_scroll.widget );
	    }
	}

    if ( w->output.status & RESIZE_SCROLL_BARS )
	{
	w->output.status &= ~RESIZE_SCROLL_BARS;
	w->output.status |= REDRAW_DISPLAY;

	find_optimal_scrollbar_sizes(w, &hw, &hh, &vw, &vh);

/* move and resize the horizontal scroll bar */

	if ( w->common.scrollHorizontal )
	    {
	    XtMoveWidget( w->output.h_scroll.widget, 0,
	      w->core.height - SCROLL_BAR_WIDTH );
	    XtResizeWidget( w->output.h_scroll.widget, hw,
		hh, w->output.h_scroll.widget->core.border_width );
	    }

/* move and resize the vertical scroll bar */

	if ( w->common.scrollVertical )
	    {
	    XtMoveWidget( w->output.v_scroll.widget,
	      w->core.width - SCROLL_BAR_WIDTH, 0 );
	    XtResizeWidget( w->output.v_scroll.widget, vw,
	      vh, w->output.v_scroll.widget->core.border_width );
	    }

/* make the scroll bars visible if they have been enabled */

	if ( w->common.scrollHorizontal
	  && ! ( w->output.status & HORIZONTAL_SCROLL_BAR_MAPPED ) )
	    {
	    w->output.status |= HORIZONTAL_SCROLL_BAR_MAPPED;
	    XtManageChild( w->output.h_scroll.widget );
	    }
	if ( w->common.scrollVertical
	  && ! ( w->output.status & VERTICAL_SCROLL_BAR_MAPPED ) )
	    {
	    w->output.status |= VERTICAL_SCROLL_BAR_MAPPED;
	    XtManageChild( w->output.v_scroll.widget );
	    }
	regis_resize_window( w );	/* reset ReGIS clipping rectangles */
	}

    if ( w->output.status & REDRAW_DISPLAY )
	{
	w->output.status &= ~REDRAW_DISPLAY;
	XClearWindow( XtDisplay(w), XtWindow(w) );
	w->output.status &= ~CURSOR_IS_VISIBLE;
	event.type = ClientMessage;
	event.xclient.display = XtDisplay(w);
	event.xclient.window = XtWindow(w);
	event.xclient.format = 32;
	event.xclient.data.l[0] = XInternAtom( XtDisplay(w),
		"MESSAGE_REDRAW_DISPLAY", FALSE );
	XSendEvent( XtDisplay(w), XtWindow(w), True, NoEventMask,
	  &event );
	w->output.status |= DISABLE_REFRESH;
	}
}

/* o_disable_redisplay - disable redrawing the window until o_adjust_display
			 is called */

void o_disable_redisplay( w )
    DECtermWidget w;
{
    w->output.status |= DISABLE_REDISPLAY_CONTROL;
}

/* o_enable_redisplay - re-enable redrawing the window */

void o_enable_redisplay( w )
    DECtermWidget w;
{
    w->output.status &= ~DISABLE_REDISPLAY_CONTROL;
}

/* o_disable_cursor - turn off the text cursor */

void o_disable_cursor( w, reason )
    DECtermWidget w;
    int reason;
{
    o_erase_cursor( w );
    w->output.status |= reason;
}

/* o_enable_cursor - turn on the text cursor */

void o_enable_cursor( w, reason )
    DECtermWidget w;
    int reason;
{
    w->output.status &= ~reason;
    if ( ! ( w->output.status & CURSOR_DISABLED ) )
	o_display_cursor( w );
}

/* o_repaint_display - redraw the display, including the background */

o_repaint_display( w )
    DECtermWidget w;
{
    int row, dwidth, dheight, owidth, oheight;
    XPoint points[9];

    o_update_rectangle( w, w->output.top_visible_line,
      w->output.top_visible_line + w->output.visible_rows - 1,
      w->output.left_visible_column, w->output.visible_columns, NULL );

    update_segment(w, STATUS_LINE, w->output.left_visible_column,
	  w->output.visible_columns );
    o_display_cursor( w );
    repaint_border( w );

    dwidth = w->common.display_width + X_MARGIN * 2;
    dheight = w->common.display_height + Y_MARGIN * 2;
    owidth = w->core.width - dwidth;
    oheight = w->core.height - dheight;

/* clear the inner border */

    points[0].x = 2;
    points[0].y = dheight - 3;
    points[1].x = 2;
    points[1].y = 2;
    points[2].x = dwidth - 3;
    points[2].y = 2;
    points[3].x = dwidth - 3;
    points[3].y = dheight - 3;
    points[4].x = 3;
    points[4].y = dheight - 3;
    points[5].x = 3;
    points[5].y = 3;
    points[6].x = dwidth - 4;
    points[6].y = 3;
    points[7].x = dwidth - 4;
    points[7].y = dheight - 4;
    points[8].x = 4;
    points[8].y = dheight - 4;

    XDrawLines( XtDisplay(w), XtWindow(w), w->output.reverse_gc, points, 9,
      CoordModeOrigin );

/* clear the areas outside the display area */

    if ( owidth > 0 )
	XClearArea( XtDisplay(w), XtWindow(w), dwidth, 0, owidth, dheight,
	  False );
    if ( oheight > 0 )
	XClearArea( XtDisplay(w), XtWindow(w), 0, dheight, w->core.width,
	  oheight, False );
}

/* o_set_cursor_plane_mask - set plane mask in complement GCs */

o_set_cursor_plane_mask( w )
    DECtermWidget w;
{
    Pixel background, mask;
    int dim;

    if ( w->common.screenMode && ! w->common.really_use_bold_font
      && w->common.color_map_allocated )
        {
        /*
         * Reverse video and emulating VT340 color text, so xor the
         * foreground with the dim color instead of with the background_pixel
         * resource.
         */
        if ( w->common.bitPlanes < 4 )
            dim = 1;
        else
            dim = 8;
        background = w->common.color_map[dim].pixel;
        }
    else
        background = w->core.background_pixel;
    mask = background ^ w->manager.foreground;
    XSetPlaneMask( XtDisplay(w), w->output.complement_gc, mask );
}

#ifdef DEBUG_DT_OUTPUT

void debug( onoff )
    int onoff;
{
    XSynchronize( XtDisplay(output_widget), onoff );
}

#endif

o_cursor_couple( w )
    DECtermWidget w;
{
    register int i, adjust=0;

    if ( w->common.couplingHorizontal ) {
	i = w->output.cursor_column;
	if (i < w->output.left_visible_column) {
	    pan_horizontal(w, i );
	    adjust = 1;
	}
	if (i  > (w->output.left_visible_column + w->output.visible_columns - 1) ) {
		i -= w->output.visible_columns - 1;
		if (i < 0) i = 0;
		pan_horizontal(w, i);
		adjust = 1;
	}
    }
    if ( w->common.couplingVertical && w->output.cursor_row != STATUS_LINE ) {
	i = w->output.cursor_row;
	if (i < w->output.top_visible_line ) {
	    pan_vertical( w, i );
	    adjust = 1;
	}
	if (i > (w->output.top_visible_line + w->output.visible_rows - 1) ) {
		i -= w->output.visible_rows - 1;
		if (i <= 0) i = 1;
		pan_vertical( w, i );
		adjust = 1;
	}
    }
    if (adjust) adjust_scroll_bars( w );
}

/* add_scroll_entry
 *
 * This routine formerly added an entry in the scroll queue: each XCopyArea
 * request added an entry to the queue and each GraphicsExpose or NoExpose
 * event removed one.  This algorithm could still lead to holes in the
 * display when the scroll queue overflowed.
 *
 * The simplified algorithm is to maintain counts of the number of lines
 * scrolled up and down and the number of outstanding expose events.  See
 * the comments in o_expose for details.
 */

add_scroll_entry( w, count )
    DECtermWidget w;
    int count;		/* number of lines scrolled:
				positive for scrolling up
				negative for scrolling down
				zero for scrolling horizontally */
{
    if ( w->output.exposures_pending <= 0 )
	{  /* first scroll from the idle state */
	w->output.first_scroll = count;
	if ( count > 0 )
	    w->output.lines_scrolled_up = count;
	else
	    w->output.lines_scrolled_down = (-count);
	w->output.scroll_direction = count;
	}
    else
	{
	if ( count > 0 )
	    w->output.lines_scrolled_up += count;
	else
	    w->output.lines_scrolled_down -= count;
	if ( count * w->output.scroll_direction <= 0 )
	    w->output.scroll_direction = 0;	/* changed direction */
	}
    ++w->output.exposures_pending;
}
/* get_ansi_color - return a pixel to use for ANSI color text */

static Pixel get_ansi_color( w, index )
    DECtermWidget w;
    int index;
{
    int entry;
    XColor screen_def_return, exact_def_return;

    w->common.graphics_visible = TRUE;

    if ( w->output.color_pixels[ index ] == NO_COLOR )
	{  /* need to allocate a color */

/*
 * If we've allocated a private colormap, use entries from the real
 * colormap.  Otherwise, try to allocate a shared, read-only color.
 */

	if ( w->common.color_map_allocated
	  && w->common.graphics_mode == ALLOCATED_COLORMAP )
	    {  /* already created private colormap, so use color map entry */
	    entry = ansi_color_entries[ index ];
	    w->output.color_pixels[ index ] =
		w->common.color_map[ entry ].pixel;
	    }
	else
	    /* try to allocate a named color from default colormap */
	    {
#if defined(VMS_DECTERM)
	    noshare
#endif
		extern char *color_name[];
	    if ( XAllocNamedColor( XtDisplay(w), w->core.colormap,
			color_name[ index ], &screen_def_return,
			&exact_def_return ) )
		{  /* use allocated color */
		w->output.color_pixels[ index ] = exact_def_return.pixel;
		w->output.color_pixel_allocated[ index ] = TRUE;
		}
	    else
		{  /* have to use color map entry */
		if ( ! w->common.color_map_allocated ) allocate_color_map( w );
		entry = ansi_color_entries[ index ];
		w->output.color_pixels[ index ] =
		    w->common.color_map[ entry ].pixel;
		}
	    }
	}
    return w->output.color_pixels[ index ];
}

/* o_free_ansi_colors - free all ANSI color text colors */

o_free_ansi_colors( w )
    DECtermWidget w;
{
    int i;

    for ( i = 0; i < NUM_TEXT_COLORS; i++ )
	{
	if ( w->output.color_pixel_allocated[i] )
	    XFreeColors( XtDisplay(w), w->core.colormap,
		    &w->output.color_pixels[i], 1, 0 );
	w->output.color_pixels[i] = NO_COLOR;
	w->output.color_pixel_allocated[i] = FALSE;
	}
}

#define valid_2_byte(index) \
    ((ext_attrib[(index)] == FIRST_OF_FOUR) && \
     (ext_attrib[(index+1)] == SECOND_OF_FOUR))
#define valid_4_byte_nolc(index) \
    ((ext_attrib[(index)] == THIRD_OF_FOUR) && \
     (ext_attrib[(index+1)] == FOURTH_OF_FOUR))
#define valid_4_lc(index) \
    ((ext_attrib[(index)] == FIRST_OF_FOUR_LC) && \
     (ext_attrib[(index+1)] == SECOND_OF_FOUR_LC))
#define valid_4_content(index) \
    ((ext_attrib[(index)] == THIRD_OF_FOUR_LC) && \
     (ext_attrib[(index+1)] == FOURTH_OF_FOUR_LC))
#define valid_4_byte_lc(index) \
    (valid_4_lc((index)) && valid_4_content((index)+2))

static XImage *get_scale_image();
static void draw_image();
static void scale_bitmap();

/* draw_image_string - draw text string of mixed 1/2-byte characters */
/* draw to the drawable instead of XtWindow(w) if p_map != NULL.  This
 * is for Print Graphics support.  also, XtFreePixmap() is added   a.c.
 */

draw_image_string( w, gc_mask, x, y, line, ls, start_column, column, p_map )
    DECtermData *w;
    int gc_mask;
    int x, y;
    int line;
    s_line_structure *ls;
    int start_column, column;
    Pixmap p_map;   /* draw to this if not NULL */
{
    char *string;
    REND *attrib;
    EXT_REND *ext_attrib;             
    unsigned long and_mask, or_mask, sav_mask;
    int length, ini, i, ii, sav_func;
    Bool sav_exp;
    int xi, yi, char_width, char_height, char_ascent;
    GC gc;
    XGCValues gcvalues;	/* values block to retrieve gc values data */
    XChar2b string16[ MAX_COLUMN ];
    Display *dpy;                                 
    Window wid;
    int src_width, src_height;
    int dest_x, dest_y, dest_width, dest_height, dest_shift;
    XImage *dest;
    Drawable drawable;
    GC pgc;
    Boolean double_mode = False;
    int px, py;
    int ini1;
    Drawable des_drawable;

    string = (char *)ls->a_code_base + start_column;
    attrib = (REND *)ls->a_rend_base + start_column;
    ext_attrib = ls->a_ext_rend_base + start_column;
    length = column - start_column;
    char_width = w->output.char_width;
    char_height = w->output.char_height;
    char_ascent = w->output.char_ascent;
    gc = w->output.text_gc[ gc_mask ];
    dpy = XtDisplay( w );

    des_drawable = (Drawable) ((p_map == NULL) ? XtWindow(w) : p_map);

    wid = (Window) des_drawable;

/* if double size/wide */

    if (( gc_mask & DOUBLE_SIZE_TEXT_GC_MASK ) || ( gc_mask & DOUBLE_WIDE_TEXT_GC_MASK ))
	{
	double_mode = True;
	{
	    int width = char_width * ( length + 6 );
	    if ( width > DisplayWidth( dpy, DefaultScreen( dpy ))) {
		width =  DisplayWidth( dpy, DefaultScreen( dpy ));
		px = 0;
	    } else
		px = char_width * 3;
	    if ( w->output.pixmap )
		XFreePixmap( dpy, w->output.pixmap );
	    if ( w->output.pixmap_gc )
		XFreeGC( dpy, w->output.pixmap_gc );
	    w->output.pixmap = XCreatePixmap( dpy, wid, width, char_height, 1 );
	    w->output.pixmap_gc = XCreateGC( dpy, w->output.pixmap, 0, NULL );
	    XSetForeground( dpy, w->output.pixmap_gc, 1 );
	    XSetBackground( dpy, w->output.pixmap_gc, 0 );
	}
	XGetGCValues(dpy, gc, GCFont, &gcvalues) ;
	
	XSetFont( dpy, w->output.pixmap_gc, gcvalues.font );
	drawable = w->output.pixmap;
	pgc = w->output.pixmap_gc;
	py = char_ascent;
	} else {
	drawable = wid;
	pgc = gc;
	px = x;
	py = y;
	}                  

/* write string */

    if (( ext_attrib[0] & csa_M_EXT_CHAR_SET ) == TWO_BYTE_SET )
	{
        if (( start_column != 0 ) && 
	        (( ext_attrib[-1] & csa_M_BYTE_OF_CHAR ) == FIRST_OF_TWO ) &&
	        (( ext_attrib[0] & csa_M_BYTE_OF_CHAR ) == LAST_OF_TWO ))
	    {
	    if ( string[-1] & 128 )
		{
		string16[0].byte1 = string[-1];
		string16[0].byte2 = string[0];
		}
	    else
		{    
		string16[0].byte1 = string[-1] | 128;
		string16[0].byte2 = string[0] | 128;    
		}
	    x -= char_width;
	    if ( w->common.regisScreenMode && !double_mode )
		clear_background( w, dpy, drawable, gc_mask, 
		    px, py - char_ascent, char_width, char_height);
	    XDrawImageString16( dpy, drawable, pgc, px - char_width, py,
		    string16, 1 );
	    ini = 1;
	    }
	else
	    if (( ext_attrib[0] & csa_M_BYTE_OF_CHAR ) == LAST_OF_TWO )
		ini = 1;
	    else
		ini = 0;
	i = ini;                   
	ii = 0;
	while ( i < length )           
	    {
	    if ( string[i] & 128 )
		{
	        if (( ext_attrib[i] & csa_M_BYTE_OF_CHAR ) == FIRST_OF_TWO )
		    {
	            if (( ext_attrib[i+1] & csa_M_BYTE_OF_CHAR ) == LAST_OF_TWO )
			{
			string16[ii].byte1 = string[i++];
			string16[ii++].byte2 = string[i++];
			}
		    else
			{
			if ( w->common.regisScreenMode && !double_mode )
			   clear_background( w, dpy, drawable, gc_mask, 
				px + char_width * ini, py - char_ascent, char_width * ii * 2, char_height);
			XDrawImageString16( dpy, drawable, pgc,
			    px + char_width * ini, py, string16, ii );
			ini += 2 * ii;
			i = ++ini;
			ii = 0;
			}
		    }
		else
		    {
		    if ( w->common.regisScreenMode && !double_mode )
			clear_background( w, dpy, drawable, gc_mask, 
			   px + char_width * ini, py - char_ascent, char_width * ii * 2, char_height);
		    XDrawImageString16( dpy, drawable, pgc,
			px + char_width * ini, py, string16, ii );
		    ini += 2 * ii;
		    i = ++ini;
		    ii = 0;
		    }
		}
	    else
		{
	        if (( ext_attrib[i] & csa_M_BYTE_OF_CHAR ) == FIRST_OF_TWO )
		    {
	            if (( ext_attrib[i+1] & csa_M_BYTE_OF_CHAR ) == LAST_OF_TWO )
			{
			string16[ii].byte1 = string[i++] | 128;
			string16[ii++].byte2 = string[i++] | 128;
			}
		    else
			{
			if ( w->common.regisScreenMode && !double_mode )
			   clear_background( w, dpy, drawable, gc_mask, 
				px + char_width * ini, py - char_ascent, char_width * ii * 2, char_height);
			XDrawImageString16( dpy, drawable, pgc,
			    px + char_width * ini, py, string16, ii );
			ini += 2 * ii;
			i = ++ini;
			ii = 0;
			}
		    }
		else
		    {
		    if ( w->common.regisScreenMode && !double_mode )
			clear_background( w, dpy, drawable, gc_mask, 
			   px + char_width * ini, py - char_ascent, char_width * ii * 2, char_height);
		    XDrawImageString16( dpy, drawable, pgc,
			px + char_width * ini, py, string16, ii );
		    ini += 2 * ii;
		    i = ++ini;
		    ii = 0;
    		    }
		}
	    }
	if ( w->common.regisScreenMode && !double_mode )
	    clear_background( w, dpy, drawable, gc_mask, 
		px + char_width * ini, py - char_ascent, char_width * ii * 2, char_height);
	XDrawImageString16( dpy, drawable, pgc,
	    px + char_width * ini, py, string16, ii );
        }

    else if (( ext_attrib[0] & csa_M_EXT_CHAR_SET ) == FOUR_BYTE_SET )
	{
        if (( start_column != 0 ) && 
	        ( valid_2_byte(-1) || valid_4_byte_nolc(-1) )) {
	    if ( string[-1] & 128 ) {
		string16[0].byte1 = string[-1];
		string16[0].byte2 = string[0];
	    } else {
		string16[0].byte1 = string[-1] | 128;
		string16[0].byte2 = string[0] | 128;    
	    }
	    x -= char_width;
	    if ( w->common.regisScreenMode && !double_mode )
		clear_background( w, dpy, drawable, gc_mask, 
		px - char_width, py - char_ascent, char_width * 2, char_height);
	    XDrawImageString16( dpy, drawable, pgc, 
				px - char_width, py, string16, 1 );
	    ini = ini1 = 1;
	} else if ( ext_attrib[0] & csa_M_LEADING_CODE_MODE ) {
	    if (( start_column > 2 ) && valid_4_byte_lc(-3))
		ini = 3;
	    else if (( start_column > 1 ) && valid_4_byte_lc(-2))
		ini = 2;
	    else if (( start_column > 0 ) && valid_4_byte_lc(-1))
		ini = 1;
	    else
		ini = 0;
	    ini1 = 0;
	    if (ini) {
		if ( string[2-ini] & 128 ) {
		    string16[0].byte1 = string[2-ini];
		    string16[0].byte2 = string[3-ini];
		} else {
		    string16[0].byte1 = string[2-ini] | 128;
		    string16[0].byte2 = string[3-ini] | 128;    
		}
		x -= char_width * ini;	/* 910704, TN */
		if ( w->common.regisScreenMode && !double_mode )
		    clear_background( w, dpy, drawable, gc_mask, 
		    px - char_width * ini, py - char_ascent, char_width * 4, char_height);
		DrawLeadingCode( dpy, drawable, pgc, char_width,
		    char_height, px - char_width * ini, py - char_ascent,
		    double_mode, ( gc_mask & REVERSE_TEXT_GC_MASK ) ?
			w->output.normal_gc : w->output.reverse_gc );
		XDrawImageString16( dpy, drawable, pgc,
		    px + char_width * ( 2 - ini ), py, string16, 1 );
		ini1 = 4 - ini;
	    }
	} else {
	    ini = ini1 = 0;
	}
	i = ini1;                   
	ii = 0;
	while ( i < length ) {          
	    if ( valid_2_byte(i) || valid_4_byte_nolc(i) ||
		 valid_4_content(i) ) {
		if ( string[i] & 128 ) {
		    string16[ii].byte1 = string[i++];
		    string16[ii++].byte2 = string[i++];
		} else {
		    string16[ii].byte1 = string[i++] | 128;
		    string16[ii++].byte2 = string[i++] | 128;
		}
	    } else {
		if ( w->common.regisScreenMode && !double_mode )
		    clear_background( w, dpy, drawable, gc_mask, 
		    px + char_width * ini1, py - char_ascent,
		    char_width * 2, char_height);
		XDrawImageString16( dpy, drawable, pgc, 
		    px + char_width * ini1, py, string16, ii );
		ini1 += 2 * ii;
		if ( valid_4_byte_lc(i) ) {
		    if ( w->common.regisScreenMode && !double_mode )
			clear_background( w, dpy, drawable, gc_mask, 
			px - char_width * ini1, py - char_ascent,
			char_width * 2, char_height);
		    DrawLeadingCode( dpy, drawable, pgc, char_width, char_height,
			px + char_width * ini1, py - char_ascent,
			double_mode, ( gc_mask & REVERSE_TEXT_GC_MASK ) ?
			    w->output.normal_gc : w->output.reverse_gc );
		    ini1 += 1;
		}
		i = ++ini1;
		ii = 0;
	    }
	}
	if ( w->common.regisScreenMode && !double_mode )
	    clear_background( w, dpy, drawable, gc_mask, 
	    px + char_width * ini1, py - char_ascent, char_width * ii * 2, char_height);
	XDrawImageString16( dpy, drawable, pgc, 
	    px + char_width * ini1, py, string16, ii );
        }

    else
	{
	i = length;
	ini = 0;
	if ( w->common.regisScreenMode && !double_mode )
	    clear_background( w, dpy, drawable, gc_mask, px,
		py - char_ascent, char_width * i, char_height );
	XDrawImageString( dpy, drawable, pgc, px, py, (char *)string, i );
	}

/* simulate double size/width rendition */

    if ( double_mode )
	{
	int tag = 0;

	dest_x = x - char_width * ini;
	dest_y = y - char_ascent;
	dest_width = ( src_width = char_width * ( i + ini )) << 1;
	dest_height = src_height = char_height;
	dest_shift = 2;

	if ( ls->b_rendits & csa_M_DOUBLE_HIGH ) {
	    if ( ls->b_rendits & csa_M_DOUBLE_BOTTOM ) {
		tag = 2;
		dest_y += char_height - char_ascent;
	    } else {
		tag = 1;
		dest_y -= char_ascent;
	    }
	}
	dest = get_scale_image( dpy, w->output.pixmap, px - char_width * ini,
	    py - char_ascent, src_width, src_height, tag );

	sav_exp = gcvalues.graphics_exposures;
	XSetGraphicsExposures( dpy, gc, False );

	draw_image( dpy, wid, gc, dest, dest_x, dest_y,
	    dest_width, dest_height );

	XSetGraphicsExposures( dpy, gc, sav_exp );

	XFreePixmap( dpy, w->output.pixmap );
	XFreeGC( dpy, w->output.pixmap_gc );
	w->output.pixmap = NULL;
	w->output.pixmap_gc = NULL;
	} else {	/* !double_mode	*/
	    dest_x = x - char_width * ini;
	    dest_y = y - char_ascent;
	    dest_width = char_width * ( i + ini );
	    dest_height = char_height;
	    dest_shift = 1;
	}

/* simulate bold rendition (supported double size/width text) */
    if ( gc_mask & BOLD_TEXT_GC_MASK ) {
	if ( gc_mask & REVERSE_TEXT_GC_MASK ) {
	    and_mask = w->manager.foreground;
	    or_mask = ~and_mask;
	} else {
	    or_mask = w->manager.foreground;
	    and_mask = ~or_mask;
	}
	XGetGCValues(dpy, gc, GCFunction | GCPlaneMask | GCGraphicsExposures,
		     &gcvalues) ;
	sav_func = gcvalues.function;
	sav_mask = gcvalues.plane_mask;
	sav_exp = gcvalues.graphics_exposures;
	XSetGraphicsExposures( dpy, gc, False );

	if ( XPlanesOfScreen( XDefaultScreenOfDisplay( dpy )) > 1 ||
		( and_mask & 1 )) {
	    if (( w->core.background_pixel == 1 ) ||
	       !( attrib[i + ini] & csa_M_NODEFAULT_TEXT )) {
	        XSetFunction( dpy, gc, GXand );
	        XSetPlaneMask( dpy, gc, and_mask );
                XCopyArea( dpy, wid, wid, gc, dest_x, dest_y,
		    dest_width - dest_shift, dest_height,
		    dest_x + dest_shift, dest_y );
	    } else if (( attrib[i + ini] & csa_M_NODEFAULT_TEXT_BCK )) {
	        XSetFunction( dpy, gc, GXor );
	        XSetPlaneMask( dpy, gc, and_mask & get_ansi_color( w,
		    (attrib[i + ini] & MASK_TEXT) >> MASK_TEXT_SHIFT ));
                XCopyArea( dpy, wid, wid, gc, dest_x, dest_y,
		    dest_width - dest_shift, dest_height,
		    dest_x + dest_shift, dest_y );
	        XSetFunction( dpy, gc, GXand );
	        XSetPlaneMask( dpy, gc, and_mask & get_ansi_color( w,
		    (attrib[i + ini] & MASK_TEXT_BCK) >> MASK_TEXT_BCK_SHIFT));
                XCopyArea( dpy, wid, wid, gc, dest_x, dest_y,
		    dest_width - dest_shift, dest_height,
		    dest_x + dest_shift, dest_y );
	    } else {
	        XSetFunction( dpy, gc, GXand );
	        XSetPlaneMask( dpy, gc, and_mask & w->core.background_pixel );
                XCopyArea( dpy, wid, wid, gc, dest_x, dest_y,
		    dest_width - dest_shift, dest_height,
		    dest_x + dest_shift, dest_y );
  	    	XSetFunction( dpy, gc, GXor );
	        XSetPlaneMask( dpy, gc, and_mask & ~w->core.background_pixel );
                XCopyArea( dpy, wid, wid, gc, dest_x, dest_y,
		    dest_width - dest_shift, dest_height, 
		    dest_x + dest_shift, dest_y );
	    }
	}

	if ( XPlanesOfScreen( XDefaultScreenOfDisplay( dpy )) > 1 ||
		( or_mask & 1 )) {
	    if (( w->core.background_pixel == 1 ) ||
	        !( attrib[i + ini] & csa_M_NODEFAULT_TEXT_BCK )) {
	        XSetFunction( dpy, gc, GXor );
	        XSetPlaneMask( dpy, gc, or_mask );
                XCopyArea( dpy, wid, wid, gc, dest_x, dest_y,
		    dest_width - dest_shift, dest_height,
		    dest_x + dest_shift, dest_y );
	    } else if (( attrib[i + ini] & csa_M_NODEFAULT_TEXT )) {
	        XSetFunction( dpy, gc, GXor );
	        XSetPlaneMask( dpy, gc, or_mask & get_ansi_color( w,
		    (attrib[i + ini] & MASK_TEXT) >> MASK_TEXT_SHIFT ));
                XCopyArea( dpy, wid, wid, gc, dest_x, dest_y,
		    dest_width - dest_shift, dest_height,
		    dest_x + dest_shift, dest_y );
	        XSetFunction( dpy, gc, GXand );
	        XSetPlaneMask( dpy, gc, or_mask & get_ansi_color( w,
		    (attrib[i + ini] & MASK_TEXT_BCK) >> MASK_TEXT_BCK_SHIFT));
                XCopyArea( dpy, wid, wid, gc, dest_x, dest_y,
		    dest_width - dest_shift, dest_height,
		    dest_x + dest_shift, dest_y );
	    } else {
	        XSetFunction( dpy, gc, GXor );
	        XSetPlaneMask( dpy, gc, or_mask & w->core.background_pixel );
                XCopyArea( dpy, wid, wid, gc, dest_x, dest_y,
		    dest_width - dest_shift, dest_height,
		    dest_x + dest_shift, dest_y );
  	    	XSetFunction( dpy, gc, GXand );
	        XSetPlaneMask( dpy, gc, or_mask & ~w->core.background_pixel );
                XCopyArea( dpy, wid, wid, gc, dest_x, dest_y,
		    dest_width - dest_shift, dest_height,
		    dest_x + dest_shift, dest_y );
	    }
	}

	XSetFunction( dpy, gc, sav_func );
	XSetPlaneMask( dpy, gc, sav_mask );
	XSetGraphicsExposures( dpy, gc, sav_exp );
    }

/* repaint both borders if necessary */

    if ( start_column == w->output.left_visible_column )
	repaint_left_border( w, line );

    if ( i == length + 1 && column == w->output.left_visible_column 
	+ w->output.visible_columns )
	repaint_right_border( w, line );                 

}

/* draw leading code */
int DrawLeadingCode( dpy, drawable, gc, width, height, x, y, double_mode, bgc )
Display *dpy;
Drawable drawable;
GC gc, bgc;
int x, y, width, height;
Boolean double_mode;
{
    XPoint points[12];
    int sp = (((width/3+1)*2+2)>width)? width/3 : width/3 + 1;

    if ( double_mode ) {
	XSetForeground( dpy, gc, 0 );
	XFillRectangle( dpy, drawable, gc, x, y, width << 1, height );
	XSetForeground( dpy, gc, 1 );
    } else
	XFillRectangle( dpy, drawable, bgc, x, y, width << 1, height );
    if ( width < 10 ) {
	points[2].x = ( points[1].x = ( points[0].x = x + width ) + sp ) + sp;
	points[0].y = points[1].y = points[2].y = y + height - 2;
	XDrawPoints( dpy, drawable, gc, points, 3, CoordModeOrigin );
    } else {
	points[8].x = points[11].x =
	( points[4].x = points[7].x =
	( points[0].x = points[3].x = x + width ) + sp ) + sp;
	points[9].x = points[10].x =
	( points[5].x = points[6].x =
	( points[1].x = points[2].x = x + width + 1 ) + sp ) + sp;
	points[0].y = points[1].y = points[6].y = points[7].y = points[8].y =
	points[10].y =
	( points[2].y = points[3].y = points[4].y = points[5].y = points[9].y =
		      points[11].y = y + height - 2 ) - 1;
	XDrawPoints( dpy, drawable, gc, points, 12, CoordModeOrigin );
    }
}

/* scale table */
    static short int scale_2[256] = {
	0x0000,	0x0003,	0x000c,	0x000f,	0x0030,	0x0033,	0x003c,	0x003f,
	0x00c0,	0x00c3,	0x00cc,	0x00cf,	0x00f0,	0x00f3,	0x00fc,	0x00ff,
	0x0300,	0x0303,	0x030c,	0x030f,	0x0330,	0x0333,	0x033c,	0x033f,
	0x03c0,	0x03c3,	0x03cc,	0x03cf,	0x03f0,	0x03f3,	0x03fc,	0x03ff,
	0x0c00,	0x0c03,	0x0c0c,	0x0c0f,	0x0c30,	0x0c33,	0x0c3c,	0x0c3f,
	0x0cc0,	0x0cc3,	0x0ccc,	0x0ccf,	0x0cf0,	0x0cf3,	0x0cfc,	0x0cff,
	0x0f00,	0x0f03,	0x0f0c,	0x0f0f,	0x0f30,	0x0f33,	0x0f3c,	0x0f3f,
	0x0fc0,	0x0fc3,	0x0fcc,	0x0fcf,	0x0ff0,	0x0ff3,	0x0ffc,	0x0fff,
	0x3000,	0x3003,	0x300c,	0x300f,	0x3030,	0x3033,	0x303c,	0x303f,
	0x30c0,	0x30c3,	0x30cc,	0x30cf,	0x30f0,	0x30f3,	0x30fc,	0x30ff,
	0x3300,	0x3303,	0x330c,	0x330f,	0x3330,	0x3333,	0x333c,	0x333f,
	0x33c0,	0x33c3,	0x33cc,	0x33cf,	0x33f0,	0x33f3,	0x33fc,	0x33ff,
	0x3c00,	0x3c03,	0x3c0c,	0x3c0f,	0x3c30,	0x3c33,	0x3c3c,	0x3c3f,
	0x3cc0,	0x3cc3,	0x3ccc,	0x3ccf,	0x3cf0,	0x3cf3,	0x3cfc,	0x3cff,
	0x3f00,	0x3f03,	0x3f0c,	0x3f0f,	0x3f30,	0x3f33,	0x3f3c,	0x3f3f,
	0x3fc0,	0x3fc3,	0x3fcc,	0x3fcf,	0x3ff0,	0x3ff3,	0x3ffc,	0x3fff,
	0xc000,	0xc003,	0xc00c,	0xc00f,	0xc030,	0xc033,	0xc03c,	0xc03f,
	0xc0c0,	0xc0c3,	0xc0cc,	0xc0cf,	0xc0f0,	0xc0f3,	0xc0fc,	0xc0ff,
	0xc300,	0xc303,	0xc30c,	0xc30f,	0xc330,	0xc333,	0xc33c,	0xc33f,
	0xc3c0,	0xc3c3,	0xc3cc,	0xc3cf,	0xc3f0,	0xc3f3,	0xc3fc,	0xc3ff,
	0xcc00,	0xcc03,	0xcc0c,	0xcc0f,	0xcc30,	0xcc33,	0xcc3c,	0xcc3f,
	0xccc0,	0xccc3,	0xcccc,	0xcccf,	0xccf0,	0xccf3,	0xccfc,	0xccff,
	0xcf00,	0xcf03,	0xcf0c,	0xcf0f,	0xcf30,	0xcf33,	0xcf3c,	0xcf3f,
	0xcfc0,	0xcfc3,	0xcfcc,	0xcfcf,	0xcff0,	0xcff3,	0xcffc,	0xcfff,
	0xf000,	0xf003,	0xf00c,	0xf00f,	0xf030,	0xf033,	0xf03c,	0xf03f,
	0xf0c0,	0xf0c3,	0xf0cc,	0xf0cf,	0xf0f0,	0xf0f3,	0xf0fc,	0xf0ff,
	0xf300,	0xf303,	0xf30c,	0xf30f,	0xf330,	0xf333,	0xf33c,	0xf33f,
	0xf3c0,	0xf3c3,	0xf3cc,	0xf3cf,	0xf3f0,	0xf3f3,	0xf3fc,	0xf3ff,
	0xfc00,	0xfc03,	0xfc0c,	0xfc0f,	0xfc30,	0xfc33,	0xfc3c,	0xfc3f,
	0xfcc0,	0xfcc3,	0xfccc,	0xfccf,	0xfcf0,	0xfcf3,	0xfcfc,	0xfcff,
	0xff00,	0xff03,	0xff0c,	0xff0f,	0xff30,	0xff33,	0xff3c,	0xff3f,
	0xffc0,	0xffc3,	0xffcc,	0xffcf,	0xfff0,	0xfff3,	0xfffc,	0xffff
    };

/* draw scaled image */
static void draw_image( dpy, wid, gc, image, x, y, width, height )
Display *dpy;
Window wid;
GC gc;
XImage *image;
int x, y, width, height;
{
    image->width = width;
    image->height = height;
    XPutImage( dpy, wid, gc, image, 0, 0, x, y, width, height );
    if ( image->data )
	XtFree( image->data );
}

/* get and scale bitmap pattern from the pixmap */
static XImage *get_scale_image( dpy, pixmap, x, y, width, height, tag )
Display *dpy;
Pixmap pixmap;
int x, y, width, height, tag;
{
    static XImage image = { 0, 0, 0, XYBitmap, NULL, 0, 16, 0, 16, 1, 0, 1,
				0, 0, 0, NULL };
    XImage *g_image = NULL;
    int	src_bpl,
	des_wpl = (( width << 1 ) + 0x0f ) >> 4,
	scan = ( width + 7 ) >> 3,
	half_height = height >> 1;
    char *src_bitmap;
    short int *des_bitmap, *bitmap;

    g_image = XGetImage( dpy, pixmap, x, y, width, height, 1, XYPixmap );
    if ( !g_image ) {
	image.data = NULL;
	return( &image );
    }
    bitmap = (short int *)XtCalloc(( des_wpl * height ) << 1 , 1 );
    src_bpl = g_image->bytes_per_line;
    src_bitmap = (char *)g_image->data;
    des_bitmap = (short int *)bitmap;
    if ( tag ) {
	if ( tag == 2 ) {
	    src_bitmap += ( height - half_height ) * src_bpl;
	    if ( height & 0x01 )
		des_bitmap += des_wpl;
	}
	scale_bitmap( des_bitmap, des_wpl, src_bitmap, src_bpl, scan,
	    half_height, 2 );
	if ( height &= (int)0x01 )
	    src_bitmap = (char *)( g_image->data + half_height * src_bpl );
	    des_bitmap = (short int *)( bitmap + (( tag == 2 ) ? 0 :
				(( half_height << 1 ) * des_wpl )));
    }
    scale_bitmap( des_bitmap, des_wpl, src_bitmap, src_bpl, scan,
	height, 1 );

    image.data = (char *)bitmap;
    image.bytes_per_line = des_wpl << 1;
    image.bitmap_bit_order = g_image->bitmap_bit_order;
    image.byte_order = g_image->byte_order;

    XDestroyImage( g_image );
    return( &image );
}

/* scale directly from XImage, assume LSBFirst in byte_order & bit_order */
static void
scale_bitmap( des, des_wpl, src, src_bpl, src_w, src_h, scale )
char *src;
short int *des;
int des_wpl, src_bpl, src_w, src_h, scale;
{
    register unsigned char *sptr = (unsigned char *)src;
    register short int *dptr1 = (short int *)des,
		       *dptr2 = (short int *)( des + des_wpl );
    register int j;
    int i;
    char *sptr_begin = src;

    if ( scale == 1 ) {
	for ( i = src_h; i; i-- ) {
	    for ( j = src_w; j; j-- )
		*dptr1++ = scale_2[*sptr++];
	    sptr = (unsigned char *)( sptr_begin += src_bpl );
	}
    } else if ( scale == 2 ) {
	for ( i = src_h; i; i-- ) {
	    for ( j = src_w; j; j-- )
		*dptr2++ = *dptr1++ = scale_2[*sptr++];
	    sptr = (unsigned char *)( sptr_begin += src_bpl );
	    dptr1 += des_wpl;
	    dptr2 += des_wpl;
	}
    }
}


/* clear background ( needed in ReGIS screen mode ) */

int clear_background( w, dpy, wid, gc_mask, x, y, width, height )
DECtermData *w;
Display *dpy;
Window wid;
int gc_mask;
int x, y;
int width, height;
{
    GC gc;

    if ( gc_mask & REVERSE_TEXT_GC_MASK )
	{
	gc = w->output.normal_gc;
	XFillRectangle( dpy, wid, gc, x, y, width, height);
	}
    else
	{
	gc = w->output.reverse_gc;
	XFillRectangle( dpy, wid, gc, x, y, width, height);
	}
}

#undef valid_2_byte(index)
#undef valid_4_byte_nolc(index)
#undef valid_4_lc(index)
#undef valid_4_content(index)
#undef valid_4_byte_lc(index)
