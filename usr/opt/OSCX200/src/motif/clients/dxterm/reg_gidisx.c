/*
 *  Title:	GidisX
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
 *	This contains the code for ReGIS to draw in an X window as part of the
 *	DECterm widget.  It parallels the Gidis interface in earlier ReGIS
 *	products.
 *
 *  Author:	Bob Messenger
 *
 *  Modified by:
 *
 * Alfred von Campe     18-Dec-1993     BL-E
 *	- Change all occurrances of common.foreground to manager.foreground.
 *
 * Eric Osman		 4-Oct-1993	BL-E
 *	- Don't let characters be so big that XPutImage uses up regis stack
 *	  and crashes decterm controller and vxt.
 *
 * Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 * Dave Doucette	 7-Apr-1993	V1.2/BL2
 *	- Added initialization of rubberband cursor information
 *	  to G3_INITIALIZE()
 *
 * Alfred von Campe     08-Oct-1992     Ag/BL10
 *      - Added typecasts to satisfy Alpha compiler.
 *
 * Eric Osman		21-Jul-1992	 VXT V12
 *	- Don't crash or hang regarding timers (regis pause command).
 *
 * Eric Osman           11-June-1992     Sun
 *      - Change a "char" to "unsigned char" to make C compiler happy
 *      - In get_glyph declare ch as unsigned char instead of int.  Makes
 *        a difference on machines that put single-characters on left end
 *        of integers instead of right.
 *
 * Aston Chan		12-Mar-1992	V3.1/BL6
 *	- Fix CLD IPO_05703.  In G68_LOAD_CHARACTER_CELL(), rs->font_glyphs[][]
 *	  may be used before malloc.  Fix is to make sure the old memory is
 *	  free and new memory malloc.
 *
 * Alfred von Campe     20-Feb-1992     V3.1
 *      - Add color text support.
 *
 * Aston Chan		19-Dec-1991	V3.1
 *	- I18n code merge
 *
 * Aston Chan		17-Dec-1991	V3.1
 *	- rs' x_offset and y_offset have to be initialized to 0 while x_scale
 *	  and y_scale have to be initialized to 1 in G3_INITIALIZE().
 *	  Solve the AccVio of divided by zero in UNMAP_X[Y]_* macros.
 *
 * Aston Chan		28-Oct-1991	V3.1
 *	- rs' is being used inside create_backing_store() before it got
 *	  initialized.  G3_INITIALIZE() is called if rs->initialize is False.
 *
 * Eric Osman		24-Oct-1991	V3.1
 *	- Don't leave allocated_glyph in unknown state in end_filled_figure.
 *	- Fix a place where glyph was being erroneously freed twice because
 *	  someone (Bob) forgot to make sure glyph!=glyph2
 *
 * Aston Chan		16-Oct-1991	V3.1
 *	- Add pixmap parameter to regis_update_rectangle().
 *
 * Michele Lien		12-Sep-1991     VXT V1.0
 *	- Fix memory corruption in draw_character().  It causes VXT to crash
 *	  when issuing regis text command with negative pattern control.
 *
 * Aston Chan		1-Sep-1991	Alpha
 *	- Change readonly to const for dec_supplemental, and iso_latin_1.
 *	  Usage of "const" is recommended in VAXC manual and is a must for
 *	  DECC.
 *
 * Eric Osman		21-Aug-1991	V3.1
 *	- Fix memory leak (that made all DECterms go away after long regis
 *	  usage) in fill code.
 *
 * Bob Messenger	18-Apr-1991	T3.0
 *	- Fix two polygon fill bugs:
 *	    - Update the plane mask correctly to fix a problem (CLD MUH01682)
 *	      where a filled area is drawn in a different color after an
 *	      expose event.
 *	    - Call fetch_color instead of regis_fetch_color to fetch the
 *	      foreground color, so that the plane mask will be applied
 *	      correctly when sharing color map entries.
 * Bob Messenger	 6-Sep-1990	X2.0-7
 *	- Send button and keypress reports as unsolicited reports, to avoid
 *	  a deadlock on VMS when doing multiple-shot input in one window
 *	  and output in another.
 *	- Update RSTRUCT in check_backing_store.
 *	- Add from_regis parameter to print_graphics_screen to support
 *	  Graphics To Host.
 *
 * Bob Messenger	26-Aug-1990	X2.0-6
 *	- Allow S(H...) to work, by having G140_PRINT_SCREEN() call
 *	  print_graphics_screen().
 *
 *  Michele Lien    24-Aug-1990 VXT X0.0
 *  - Modify this module to work on VXT platform. Change #ifdef VMS to
 *    #ifdef VMS_DECTERM so that VXT specific code can be compiled under
 *    VMS or ULTRIX development environment.
 *
 * Bob Messenger	31-Jul-1990	X3.0-6
 *	- Fix the bug in regis_input_handler where it doesn't set RSTRUCT
 *	  to point to the ReGIS context block for the current widget.  The
 *	  effect of this bug was that if the host was doing ReGIS output in
 *	  one window and ReGIS input in another, and the user pressed a
 *	  key in the second window, DECterm would go into a tight CPU loop.
 *
 * Bob Messenger        17-Jul-1990     X3.0-5
 *      Merge in Toshi Tanimoto's changes to support Asian terminals -
 *	- two byte (double width) font drawing
 *	- VT286 specific default colormap
 *
 * Bill Matthews	May 1990
 * - make dec_supplemental and iso_latin_1 globalref
 *
 * Bob Messenger	24-Apr-1990	X3.0-2
 *	- Always load the FONT_NORMAL font, since condensed fonts are now
 *	  treated as FONT_NORMAL.
 *
 * Bob Messenger	26-Mar-1990	V2.1
 *	- Choose higher intensities for the default monochrome colormap, so
 *	  colors are clearly distinguishable.
 *	  Old: 0, 13, 26, 40, 6, 20, 33, 46 (as on VT340).
 *	  New: 0, 33, 66, 100, 25, 50, 75, 85.
 *
 * Bob Messenger	 12-Mar-1990	V2.1
 *	- Fix locator reporting when screen addressing is inverted.  If the
 *	  locator position is not valid, send a report with omitted
 *	  coordinates.
 *	- Free the load font pixmap when changing fonts, to avoid an X error
 *	  because the pixmap size is wrong.
 *
 * Bob Messenger	 4-Aug-1989	X2.0-18
 *	- Test for error allocating color planes.
 *	- Fix bug where inverting shading pattern didn't redraw pixmap.
 *
 * Bob Messenger	22-Jul-1989	X2.0-16
 *	- Test for error creating the backing store pixmap.
 *
 * Bob Messenger	28-May-1989	X2.0-13
 *	- Clip to addressable area.
 *	- Disable graphics exposures for sixel.bs_gc.
 *	- Use user preference for 8 bit characters.
 *	- Fix plane mask on GPX systems.
 *	- Replace bcopy and bzero with memcpy and memset.
 *	- In draw_character when testing for the display size being the
 *	  same as the escapement, use hard coordinates instead of soft
 *	  coordinates to increase the chance of a match.  This is necessary
 *	  because some applications (e.g. VAXsim-PLUS) assume (in effect) that
 *	  S(A[0,0][767,479]) produces the same scaling as S(A[0,0][799,479]).
 *
 * Bob Messenger	11-May-1989	X2.0-10
 *	- Use VT340-compatible character sizes.
 *	- Fix the problem of gaps between adjacent characters.
 *	- Fix the plane mask on single plane / shared color systems.
 *
 * Bob Messenger	 1-May-1989	X2.0-9
 *	- Don't call XStoreColors if we're using read-only colors.  The
 *	  code to allocate a color map was testing for ALLOCATE_COLORS
 *	  instead of ALLOCATE_COLORMAP, which broke both the share colors
 *	  and private colormap code paths.
 *
 * Bob Messenger	21-Apr-1989	X2.0-7
 *	- Assume eight bit characters are ISO Latin 1 (eliminates
 *	  undefined references to iso_latin_1[].
 *
 * Bob Messenger	11-Apr-1989	X2.0-6
 *	- Decouple WVT$RESET_COLORMAP() from allocate_color_map(), so that
 *	  DECCTR can report the contents of the colormap even if graphics is
 *	  not visible.
 *
 * Bob Messenger	 5-Apr-1989	X2.0-6
 *	- Support variable number of bit planes
 *
 * Bob Messenger	 4-Apr-1989	X2.0-5
 *	- Support backing store enable.
 *
 * Bob Messenger	 4-Apr-1989	X2.0-5
 *	- Support ANSI color text: use new default colormap and deallocate
 *	  ANSI colors when freeing private colormap.
 *
 * Bob Messenger	31-Mar-1989	X2.0-5
 *	- Support shared colormap entries
 *
 * Bob Messenger	28-Mar-1989	V1.1-2
 *	- In get_glyph check for using condensed font (fixes crash on PMAX).
 *
 * Bob Messenger	15-Feb-1989	X1.1-1
 *	- Conditional compilation for 4-plane gpx colormap support, so it
 *	  can be taken out in uws 2.1.  FOUR_PLANE_COLORMAP != FALSE means
 *	  4-plane GPX's have their own colormap.  SINGLE_PLANE is the loweest
 *	  number of planes that is treated as a single plane system.
 *	- Fix Ultrix compilation problems
 *	- Fix tests for mouse coordinates being in window
 *
 * Bob Messenger	24-Jan-1989	X1.1-1
 *	- Make locator support conform to the functional spec:
 *		o locator button definition support
 *		o disable output during one shot locator input
 *		o send report for non-cursor keys in one shot mode
 *		o concurrent ANSI and ReGIS locator
 *
 * Bob Messenger	18-Jan-1989	X1.1-1
 *	- "prune" the deferred scroll queue to prevent unnecessary scrolling
 *	  of the backing store and to deallocate the backing store if
 *	  possible.
 *
 * Tom Porcher		19-Oct-1988	X0.5-4
 *	- correct draw_character() arg to unsigned char; it's used as an
 *	  array index and causes an ACCVIO (DSG #420).
 *
 * Tom Porcher		 9-Sep-1988	X0.5-1
 *	- choose correct font if condensed font is selected in
 *	  regis_create_display().
 *
 * Bob Messenger	 8-Sep-1988	X0.5-1
 *	- add in character ascent when using fast path
 *
 * Tom Porcher		16-Aug-1988	X0.4-43
 *	- fix typo in set_screen_background; mono was never set.
 *
 * Tom Porcher		12-Aug-1988	X0.4-43
 *	- changes for new true fg/bg color scheme.
 *
 * Tom Porcher		26-Apr-1988	X0.4-12
 *	- Fix type declaration of new_gc for Ultrix.
 *
 * Tom Porcher		21-Apr-1988	X0.4-10
 *	- duplicate font_glyphs initialization code in regis_create_display()
 *	  so it is always initialized.
 *
 *  Bob Messenger	20-Aug-1987
 *	First release.
 *
 */

#include "regstruct.h"
#include "gidcalls.h"

/*
 * The following SHIFT_LEFT macro implements
 *
 *            a << b
 *
 * For positive b, it's EXACTLY a << b.  For negative b, it's a >> (-b), which
 * is what many machines would do anyway, but some don't, such as Sun Sparc.
 */
#define SHIFT_LEFT_IN_PLACE(a,b) (((b)>0) ? ((a)<<=(b)) : ((a)>>=(-(b))))

#define SHIFT_LEFT(a,b) (((b)>0)?((a)<<(b)):((a)>>(-(b))))

#define MAP_X_ABSOLUTE( x ) ( (int) ( x * rs->x_scale + rs->x_offset ) )
#define MAP_Y_ABSOLUTE( y ) ( (int) ( y * rs->y_scale + rs->y_offset ) )
#define MAP_X_RELATIVE( x ) ( (int) ( x * rs->x_scale ) )
#define MAP_Y_RELATIVE( y ) ( (int) ( y * rs->y_scale ) )

#define UNMAP_X_ABSOLUTE( x ) ((int) ((x - rs->x_offset) / rs->x_scale + 0.5 ))
#define UNMAP_Y_ABSOLUTE( y ) ((int) ((y - rs->y_offset) / rs->y_scale + 0.5))
#define UNMAP_X_RELATIVE( x ) ( (int) ( x / rs->x_scale + 0.5 ) )
#define UNMAP_Y_RELATIVE( y ) ( (int) ( y / rs->y_scale + 0.5 ) )

#ifndef min
#define min(x,y) (((x)<(y))?(x):(y))
#endif

static char solid[MAX1_ALPH_CELL_HEIGHT] =
		{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0, 0 },
	dark_gray[MAX1_ALPH_CELL_HEIGHT - 2] =
		{ 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55 },
	light_gray[MAX1_ALPH_CELL_HEIGHT - 2] =
		{ 0xaa, 0xff, 0xaa, 0xff, 0xaa, 0xff, 0xaa, 0xff };

static char solid_wide[MAX1_ALPH_WIDE_CELL_HEIGHT * 2] =
		{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		  0xff, 0xff, 0xff, 0xff, 0, 0, 0, 0 },
	dark_gray_wide[(MAX1_ALPH_WIDE_CELL_HEIGHT - 2) * 2] =
		{ 0xaa, 0xaa, 0x55, 0x55, 0xaa, 0xaa, 0x55, 0x55, 0xaa, 0xaa,
		  0x55, 0x55, 0xaa, 0xaa, 0x55, 0x55, 0xaa, 0xaa, 0x55, 0x55,
		  0xaa, 0xaa, 0x55, 0x55, 0xaa, 0xaa, 0x55, 0x55, 0xaa, 0xaa,
		  0x55, 0x55, 0xaa, 0xaa, 0x55, 0x55, 0xaa, 0xaa, 0x55, 0x55,
		  0xaa, 0xaa, 0x55, 0x55 },
	light_gray_wide[(MAX1_ALPH_WIDE_CELL_HEIGHT - 2) * 2] =
		{ 0xaa, 0xaa, 0xff, 0xff, 0xaa, 0xaa, 0xff, 0xff, 0xaa, 0xaa,
		  0xff, 0xff, 0xaa, 0xaa, 0xff, 0xff, 0xaa, 0xaa, 0xff, 0xff,
		  0xaa, 0xaa, 0xff, 0xff, 0xaa, 0xaa, 0xff, 0xff, 0xaa, 0xaa,
		  0xff, 0xff, 0xaa, 0xaa, 0xff, 0xff, 0xaa, 0xaa, 0xff, 0xff,
		  0xaa, 0xaa, 0xff, 0xff };

Boolean glyph_is_halftone();
static Boolean regis_input_handler(), regis_button_handler();
Pixel fetch_color(), regis_fetch_color();
static Pixmap create_pixmap();
static void cp_error_handler();

#ifdef VMS_DECTERM
globalref const
#else
#ifdef VXT_DECTERM
globalref
#else VXT_DECTERM
extern
#endif VXT_DECTERM
#endif
char dec_supplemental[], iso_latin_1[];

#ifdef VMS_DECTERM
noshare
#endif
static int cp_error;	/* error flag for create_pixmap */

extern int s_return_lkd();

FLUSH_GIDIS()
{
}

G1_NOP()
{
}

G2_END_LIST()
{
    struct regis_cntx *rs = RSTRUCT;
    DECtermWidget w = rs->widget;
    XGCValues values;
    int line_style, foreground, background, pattern, mode, i, single_point;
    int bitno, dash_size, multiplier, dashno, pat, even_odd, dash_offset;

    if ( rs->sc_current_opcode == G58_OP_DRAW_LINES && rs->numpoints > 1 )
	{ /* end of DRAW_LINES_END_LIST command, so do actual drawing */
	check_backing_store( w );
	mode = rs->writing_mode & ~1;
	pattern = rs->gid_pattern;
	if ( rs->writing_mode & 1 )
	    pattern = (~pattern) & 0xffff;
	if ( pattern == 0 && ( mode == G42_MODE_OVERLAY
	  || mode == G40_MODE_COMPLEMENT ) || mode == G38_MODE_TRANSPARENT )
	    {
	    rs->sc_current_opcode = 0;
	    return;	/* nothing to draw */
	    }
	if ( pattern == 0 || pattern == 0xffff
	  || mode == G46_MODE_ERASE )
	    {  /* switch to solid lines */
	    if ( rs->gc_line_style != LineSolid )
		{
		XSetLineAttributes( rs->display, rs->gc, 0,
		  LineSolid, CapButt, JoinMiter );

		/* hack to work around server bug */
		XSetFillStyle( rs->display, rs->gc, FillOpaqueStippled );

		XSetFillStyle( rs->display, rs->gc, FillSolid );

		if ( w->common.backing_store_active )
		    {
		    XSetLineAttributes( rs->display, rs->bs_gc, 0,
		      LineSolid, CapButt, JoinMiter );
		    XSetFillStyle( rs->display, rs->bs_gc, FillOpaqueStippled );
		    XSetFillStyle( rs->display, rs->bs_gc, FillSolid );
		    }

		rs->gc_line_style = LineSolid;
		}
	    }
	else
	    {  /* switch to non-solid lines */
	    if ( mode == G44_MODE_REPLACE )
		line_style = LineDoubleDash;
	    else
		line_style = LineOnOffDash;
	    if ( line_style != rs->gc_line_style )
		{
		XSetLineAttributes( rs->display, rs->gc, 0, line_style,
		  CapButt, JoinMiter );
		if ( w->common.backing_store_active )
		    XSetLineAttributes( rs->display, rs->bs_gc, 0, line_style,
		      CapButt, JoinMiter );
		rs->gc_line_style = line_style;
		}
	    }
	if ( rs->gc_line_style != LineSolid
	  || mode == G42_MODE_OVERLAY
	  || mode == G44_MODE_REPLACE && pattern == 0xffff
	  || rs->writing_mode == G47_MODE_ERASE_NEGATE )
	    foreground = fetch_color( rs->foreground_index );
	else
	    foreground = fetch_color( rs->background_index );
	if ( foreground != rs->gc_foreground_color
	  && mode != G40_MODE_COMPLEMENT )
	    {
	    XSetForeground( rs->display, rs->gc, foreground );
	    if ( w->common.backing_store_active )
		XSetForeground( rs->display, rs->bs_gc, foreground );
	    rs->gc_foreground_color = foreground;
	    }
	if ( ( w->common.graphics_mode == ALLOCATED_PLANES
	    || w->common.graphics_mode == ALLOCATED_COLORMAP )
	  && rs->plane_mask != rs->gc_plane_mask )
	    {
	    XSetPlaneMask( rs->display, rs->gc, rs->plane_mask );
	    if ( w->common.backing_store_active )
		XSetPlaneMask( rs->display, rs->bs_gc, rs->plane_mask );
	    rs->gc_plane_mask = rs->plane_mask;
	    }
	if ( rs->gc_line_style == LineDoubleDash )
	    {
	    background = fetch_color( rs->background_index );
	    if ( rs->gc_background_color != background )
		{
		XSetBackground( rs->display, rs->gc, background );
		if ( w->common.backing_store_active )
		    XSetBackground( rs->display, rs->bs_gc,
		      background );
		rs->gc_background_color = background;
		}
	    }
	if ( rs->gc_function != rs->function )
	    {
	    XSetFunction( rs->display, rs->gc, rs->function );
	    if ( w->common.backing_store_active )
		XSetFunction( rs->display, rs->bs_gc, rs->function );
	    rs->gc_function = rs->function;
	    }
	if ( rs->gc_line_style != LineSolid && ( rs->gc_pattern !=
	  pattern || rs->gc_multiplier != rs->gid_multiplier ) )
	    {
	    bitno = 0;
	    pat = pattern;
	    even_odd = 1;

	    for ( dashno = 0; bitno < 16; dashno++ )
		{
		dash_size = 0;
		for ( ; bitno < 16; bitno++, pat >>= 1 )
		    {
		    if ( ( pat & 1 ) == even_odd )
			++dash_size;
		    else
			break;
		    }
		rs->dash_list[dashno] = dash_size * rs->gid_multiplier;
		dash_size = 0;
		even_odd ^= 1;
		}	
	    if ( dashno & 1 )
		rs->dash_list[ dashno++ ] = 0;
	    if ( rs->dash_list[ 0 ] == 0 )
		{
		dash_offset = rs->dash_list[ dashno - 2 ] +
		  rs->dash_list[ dashno - 1 ];
		rs->dash_list[ 0 ] += rs->dash_list[ dashno - 2 ];
		rs->dash_list[ 1 ] += rs->dash_list[ dashno - 1 ];
		dashno -= 2;
		}
	    else
		dash_offset = 0;
	    if ( rs->dash_list[ dashno - 1 ] == 0 )
		{
		dash_offset += rs->dash_list[ dashno - 2 ];
		rs->dash_list[ 0 ] += rs->dash_list[ dashno - 2 ];
		dashno -= 2;
		}
	    rs->dashno = dashno;
	    XSetDashes( rs->display, rs->gc, dash_offset, rs->dash_list,
	      rs->dashno );
	    if ( w->common.backing_store_active )
		XSetDashes( rs->display, rs->bs_gc, dash_offset, rs->dash_list,
		  rs->dashno );
	    rs->gc_pattern = rs->gid_pattern;
	    rs->gc_multiplier = rs->gid_multiplier;
	    }
		/* hack to work around server bug */
		XSetFillStyle( rs->display, rs->gc, FillOpaqueStippled );

		XSetFillStyle( rs->display, rs->gc, FillSolid );

		if ( w->common.backing_store_active )
		    {
		    XSetFillStyle( rs->display, rs->bs_gc, FillOpaqueStippled );
		    XSetFillStyle( rs->display, rs->bs_gc, FillSolid );
		    }

	if (rs->kanji_regis)
	    if ( rs->sc_current_opcode == G94_OP_DRAW_CHARACTERS )
		rs->char_stack_empty = True;

/*
 * This doesn't seem to be documented, but the server won't draw a line
 * that consists of a single point; this means we need to check to see if
 * all the points in our point list are the same.
 */
	single_point = True;
	for ( i = 1; i < rs->numpoints; i++ )
	    if ( rs->point_list[i].x != rs->point_list[0].x
	      || rs->point_list[i].y != rs->point_list[0].y )
		{
		single_point = False;
		break;
		}
	if ( single_point )
	    {
	    XDrawPoint( rs->display, rs->window, rs->gc, rs->point_list[0].x,
	      rs->point_list[0].y );
	    if ( w->common.backing_store_active )
		{
		XDrawPoint( rs->display, rs->bs_pixmap, rs->bs_gc,
		  rs->point_list[0].x - rs->x_offset,
		  rs->point_list[0].y - rs->y_offset );
		}
	    }
	else
	    {
	    XDrawLines( rs->display, rs->window, rs->gc, rs->point_list,
	      rs->numpoints, CoordModeOrigin );
	    if ( w->common.backing_store_active )
		{
		adjust_point_list();
		XDrawLines( rs->display, rs->bs_pixmap, rs->bs_gc,
		  rs->point_list,
		  rs->numpoints, CoordModeOrigin );
		}
	    }
	}

    if ( rs->sc_current_opcode != G64_OP_BEGIN_FILLED_FIGURE )
	rs->sc_current_opcode = 0;
}

G3_INITIALIZE()
{
    struct regis_cntx *rs = RSTRUCT;
    DECtermWidget w = rs->widget;
    int alph, ch;
    XGCValues values, rband_values;

    rs->sh_glyph_changed = FALSE;

    values.function   = GXcopy;
    values.plane_mask = (-1);
    values.foreground = w->manager.foreground;
    values.background = w->core.background_pixel;
    values.line_style = LineSolid;

    rband_values.function   = GXxor;
    rband_values.fill_style = 0;
    rband_values.line_style = LineSolid;

    XChangeGC( rs->display, rs->gc, GCFunction | GCPlaneMask | GCForeground |
      GCBackground | GCLineStyle, &values );
    if ( w->common.backing_store_active )
	XChangeGC( rs->display, rs->bs_gc, GCFunction | GCPlaneMask |
	  GCForeground | GCBackground | GCLineStyle, &values );

    rs->gc_function = values.function;
    rs->gc_plane_mask = values.plane_mask;
    rs->gc_foreground_color = values.foreground;
    rs->gc_background_color = values.background;
    rs->gc_line_style = values.line_style;
    rs->gc_pattern = 1;
    rs->gc_multiplier = -1;	/* impossible value */

    rs->cursor_motion_handler = NULL;	/* Start with no motion handler */
    rs->cs_height_rband	      = 0;	/* Initialize rubberband coord. */
    rs->cs_width_rband	      = 0;
    rs->cs_rband_active       = FALSE;

    /* The next two calls makes the rubberband Graphic context based on the
     * current graphic context.
     */

    rs->cs_gc_rband = XCreateGC( rs->display, rs->window, 
				 GCFunction | GCLineStyle | GCFillStyle, 
				&rband_values );

    XCopyGC( rs->display, rs->gc, GCPlaneMask | GCForeground | GCBackground,
	 rs->cs_gc_rband );

    values.fill_style = FillStippled;
    XChangeGC( rs->display, rs->text_gc, GCFunction | GCPlaneMask |
      GCForeground | GCBackground | GCFillStyle, &values );
    if ( w->common.backing_store_active )
	XChangeGC( rs->display, rs->bs_text_gc, GCFunction | GCPlaneMask |
	  GCForeground | GCBackground | GCFillStyle, &values );
    rs->text_gc_foreground_color = values.foreground;
    rs->text_gc_background_color = values.background;
    rs->text_gc_function = values.function;
    rs->text_gc_plane_mask = values.plane_mask;

    values.fill_style = FillSolid;

    XChangeGC( rs->display, rs->shade_gc, GCForeground | GCBackground |
      GCFunction | GCFillStyle | GCPlaneMask, &values );
    if ( w->common.backing_store_active )
	XChangeGC( rs->display, rs->bs_shade_gc, GCForeground | GCBackground |
	  GCFunction | GCFillStyle | GCPlaneMask, &values );
    rs->shade_gc_foreground_color = values.foreground;
    rs->shade_gc_background_color = values.background;
    rs->shade_gc_function = values.function;
    rs->shade_gc_fill_style = values.fill_style;
    rs->shade_gc_plane_mask = values.plane_mask;

    XChangeGC( rs->display, rs->text_background_gc, GCForeground |
      GCFunction | GCPlaneMask, &values );
    if ( w->common.backing_store_active )
	XChangeGC( rs->display, rs->bs_text_background_gc, GCForeground |
	  GCFunction | GCPlaneMask, &values );

    rs->text_background_gc_foreground = values.foreground;
    rs->text_background_gc_function = values.function;
    rs->text_background_gc_plane_mask = values.plane_mask;

    rs->writing_mode = G42_MODE_OVERLAY;
    rs->sc_current_opcode = 0;
    for ( alph = 0; alph < TOTAL3_NUMBER_OF_ALPHABETS; alph++ )
	{
	if ( rs->kanji_regis )
	    {
	    rs->al_width[ alph ] = W_ALPH_CELL_WIDTH_DEFAULT;
	    rs->al_height[ alph ] = H_ALPH_CELL_HEIGHT_DEFAULT;
	    }
	for ( ch = 0; ch < 256; ch++ )
	    if ( alph == 0 )
#ifdef CANNED_TEXT_FONT
		rs->font_glyphs[ alph ][ ch ] = alph0[ ch ];
#else
		rs->font_glyphs[ alph ][ ch ] = NULL;
#endif
	    else if ( rs->kanji_regis )		
		rs->font_glyphs[ alph ][ ch ] = solid_wide;
	    else
		rs->font_glyphs[ alph ][ ch ] = solid;
	}

#ifdef CANNED_TEXT_FONT
    rs->char_width = 8;
    rs->char_height = 10;
    rs->char_ascent = 0;
#else
    rs->char_width = w->output.char_width;
  if ( rs->kanji_regis ) 
    rs->char_height = w->output.real_char_height;
  else
    rs->char_height = w->output.char_height;
    rs->char_ascent = w->output.char_ascent;
#endif

    rs->tpx_pixmap = 0;
    rs->tpx_gc = 0;
    rs->tpx_width = 0;
    rs->tpx_height = 0;
    rs->tpx_x_origin = 0;
    rs->tpx_y_origin = 0;

    rs->spx_pixmap = 0;
    rs->spx_gc = 0;
    rs->spx_width = 0;
    rs->spx_height = 0;
    rs->spx_alphabet = -1;
    rs->spx_character = 0;
    rs->spx_inverted = FALSE;

    if ( rs->kanji_regis )
	{
	if (w->source.wvt$l_ext_specific_flags & vte2_m_jisroman_mode)
	    rs->g_char_set[0] = REG_JIS_ROMAN;
	else
	    rs->g_char_set[0] = REG_ASCII;

	if (w->source.wvt$l_ext_specific_flags & vte2_m_kanji_mode)
	    rs->g_char_set[1] = REG_DEC_KANJI;        
	else
	    rs->g_char_set[1] = REG_JIS_KATAKANA;
 	{
	int g_set, gc_mask;
	for (g_set = 0; g_set < 2; g_set++)
	    {
   	    gc_mask = 0;
	    switch (rs->g_char_set[g_set])
		{
		case REG_ASCII:
		    break;
		case REG_JIS_ROMAN:
		    gc_mask |= ROMAN_TEXT_GC_MASK;
		    break;
		case REG_JIS_KATAKANA:
		    gc_mask |= ROMAN_TEXT_GC_MASK;
		    break;
		case REG_DEC_KANJI:
		    gc_mask |= KANJI_TEXT_GC_MASK;
		    break;           
		}
	    if (!w->output.font_list[gc_mask >> 1])
		open_font(w, gc_mask);

	    rs->g_text_font[g_set] = w->output.font_list[gc_mask >> 1];
	    }
	}
	rs->char_stack_empty = True;
	}

    rs->x_scale	    = 1;   /* scales have to be initialized to 1, *not* 0 */
    rs->y_scale     = 1;   /* or else UNMAP_X[Y]_*(x) macro will AccVio   */
    rs->x_offset    = 0;
    rs->y_offset    = 0;

    rs->initialized = TRUE;
}

G11_NEW_PICTURE()
{
}

G12_END_PICTURE()
{
    struct regis_cntx *rs = RSTRUCT;
    DECtermWidget w = rs->widget;
    Pixel background_color;

    WVT$ERASE_DISPLAY_LIST( w );
    o_erase_cursor( w );
    rs->defer_count = 0;	/* get rid of queued erasures */
    background_color = fetch_color( rs->background_index );
    if ( rs->background_index == w->common.text_background_index )
	{
	destroy_backing_store( w );
	w->common.graphics_visible = FALSE;
	}
    else
	check_backing_store( w );
    if ( rs->screen_erase_gc_foreground != background_color )
	{
	XSetForeground( rs->display, rs->screen_erase_gc, background_color );
	if ( w->common.backing_store_active )
	    XSetForeground( rs->display, rs->bs_screen_erase_gc,
	      background_color );
	rs->screen_erase_gc_foreground = background_color;
	}
    XFillRectangle( rs->display, rs->window, rs->screen_erase_gc,
      X_MARGIN, Y_MARGIN, w->common.display_width, w->common.display_height );
    if ( w->common.backing_store_active )
	XFillRectangle( rs->display, rs->bs_pixmap, rs->bs_screen_erase_gc,
	  0, 0, w->common.logical_width,
	  w->common.logical_height );
    if ( ONE_PLANE(w) )
	set_screen_background( background_color );
}

G13_FLUSH_BUFFER()
{
}

G14_SET_NUMBER_COLORS( n )
    int n;
{
}

G15_SET_OUTPUT_CLIPPING_REGION( x, y, width, height )
    int x, y, width, height;
{
    XRectangle rectangle;
    struct regis_cntx *rs = RSTRUCT;
    DECtermWidget w = rs->widget;

    if ( width > w->common.display_width )
	width = w->common.display_width;
    if ( height > w->common.display_height )
	height = w->common.display_height;

    if ( ! XtIsRealized(w) || rs == NULL || ! rs->initialized
      || ( rs->window_width == width && rs->window_height == height ) )
	return;

    rs->window_width = width;
    rs->window_height = height;

    rectangle.x = x;
    rectangle.y = y;
    rectangle.width = width;
    rectangle.height = height;

    XSetClipRectangles( rs->display, rs->gc, 0, 0,
      &rectangle, 1, YXBanded );
    XSetClipRectangles( rs->display, rs->text_gc, 0, 0,
      &rectangle, 1, YXBanded );
    XSetClipRectangles( rs->display, rs->text_clear_gc, 0, 0,
      &rectangle, 1, YXBanded );
    XSetClipRectangles( rs->display, rs->text_background_gc, 0, 0,
      &rectangle, 1, YXBanded );
    XSetClipRectangles( rs->display, rs->shade_gc, 0, 0,
      &rectangle, 1, YXBanded );

    if ( w->common.backing_store_active )
	set_bs_clipping_rectangles();
}

set_bs_clipping_rectangles()
{
    XRectangle rectangle;
    struct regis_cntx *rs = RSTRUCT;
    DECtermWidget w = rs->widget;
    int width, height;

    width = MAP_X_RELATIVE( rs->output_ids_width );
    height = MAP_Y_RELATIVE( rs->output_ids_height );

    if ( width > w->common.display_width )
	width = w->common.display_width;
    if ( height > w->common.display_height )
	height = w->common.display_height;

    rectangle.x = 0;
    rectangle.y = 0;
    rectangle.width = width;
    rectangle.height = height;

    XSetClipRectangles( rs->display, rs->bs_gc, 0, 0,
      &rectangle, 1, YXBanded );
    XSetClipRectangles( rs->display, rs->bs_text_gc, 0, 0,
      &rectangle, 1, YXBanded );
    XSetClipRectangles( rs->display, rs->bs_text_clear_gc, 0, 0,
      &rectangle, 1, YXBanded );
    XSetClipRectangles( rs->display, rs->bs_text_background_gc, 0, 0,
      &rectangle, 1, YXBanded );
    XSetClipRectangles( rs->display, rs->bs_shade_gc, 0, 0,
      &rectangle, 1, YXBanded );
}

GSET_OUTPUT_CURSOR( alphabet, character, width, height, x_offset, y_offset )
    int alphabet, character, width, height, x_offset, y_offset;
{
}

G24_SET_CURSOR_BASE( x, y )
    int x, y;
{
}

G25_SET_IN_CLIP_REGION( x, y, width, height )
    int x, y, width, height;
{
}

G26_SET_ECHO_CURSOR( alphabet, character, width, height, x_offset, y_offset )
    int alphabet, character, width, height, x_offset, y_offset;
{
}

G27_SET_ECHO_CURSOR_BASE( x, y )
    int x, y;
{
}

G28_SET_OUTPUT_IDS( width, height )
    int width, height;
{
    struct regis_cntx *rs = RSTRUCT;
    DECtermWidget w = rs->widget;

    if ( width <= 0 || height <= 0 )
	return;		/* illegal */
    rs->output_ids_width = width;
    rs->output_ids_height = height;
    rs->x_scale = w->common.logical_width;
    rs->x_scale /= width;
    rs->y_scale = w->common.logical_height;
    rs->y_scale /= height;
    if ( rs->x_scale > rs->y_scale )
	{  /* use whichever scale factor is smaller */
	rs->x_scale = rs->y_scale;
	}
    else
	{
	rs->y_scale = rs->x_scale;
	}
    G29_SET_OUTPUT_VIEWPORT( X_MARGIN, Y_MARGIN, MAP_X_RELATIVE( width ),
      MAP_Y_RELATIVE( height ) );
}

G29_SET_OUTPUT_VIEWPORT( x, y, width, height )
    int x, y, width, height;
{
    struct regis_cntx *rs = RSTRUCT;
    DECtermWidget w = rs->widget;

    rs->x_offset = w->common.origin_x + x;
    rs->y_offset = w->common.origin_y + y;
    G15_SET_OUTPUT_CLIPPING_REGION( x, y, width, height );
}

G30_SET_GIDIS_OUTPUT_SPACE( x, y, width, height )
    int x, y, width, height;
{
}

G31_SET_INPUT_IDS( width, height )
    int width, height;
{
}

G32_SET_INPUT_VIEWPORT( x, y, width, height )
    int x, y, width, height;
{
}

G33_SET_GIDIS_INPUT_SPACE( x, y, width, height )
    int x, y, width, height;
{
}

G34_SET_ECHO_VIEWPORT( x, y, width, height )
    int x, y, width, height;
{
}

G35_SET_PRIMARY_COLOR( color )
    int color;
{
    struct regis_cntx *rs = RSTRUCT;

    rs->foreground_index = color;
}

G36_SET_SECONDARY_COLOR( color )
    int color;
{
    struct regis_cntx *rs = RSTRUCT;

    rs->background_index = color;
}

G37_SET_WRITING_MODE( mode )
    int mode;
{
    struct regis_cntx *rs = RSTRUCT;

    rs->writing_mode = mode;
    if ( ( mode & ~1 ) == G40_MODE_COMPLEMENT )
	{
	rs->function = GXinvert;
	rs->plane_mask = rs->dynamic_plane_mask;
	}
    else
	{
	rs->function = GXcopy;
	rs->plane_mask = rs->dynamic_plane_mask | rs->all_plane_mask;
	}
}

G48_SET_PLANE_MASK( mask )
    int mask;
{
    struct regis_cntx *rs = RSTRUCT;
    DECtermWidget w = rs->widget;
    int plane, plane_mask;

    if ( ONE_PLANE(w)
      || w->common.graphics_mode == ALLOCATED_COLORS )
	{
	rs->plane_mask = mask;
	rs->dynamic_plane_mask = mask;
	rs->all_plane_mask = mask;
	return;		/* no need for plane mask if sharing colormap */
	}
    plane_mask = 0;
    for ( plane = 0; plane < w->common.bitPlanes; plane++ )
	{
	if ( mask & ( 1 << plane ) )
	    plane_mask |= w->common.plane_masks[ plane ];
	}
    rs->dynamic_plane_mask = plane_mask;
    rs->all_plane_mask = XAllPlanes()
      & ~w->common.allocated_plane_mask;
    if ( ( rs->writing_mode & ~1 ) == G40_MODE_COMPLEMENT )
	rs->plane_mask = plane_mask;
    else
	rs->plane_mask = plane_mask | rs->all_plane_mask;
}

G49_SET_LINE_TEXTURE( length, pattern, multiplier )
    int length, pattern, multiplier;
{
    struct regis_cntx *rs;

    rs = RSTRUCT;

    rs->gid_pattern = pattern & 0xffff;
    rs->gid_multiplier = multiplier;
}

G50_SET_LINE_WIDTH( width )
    int width;
{
}

G51_SET_PIXEL_SIZE( width, height, x_offset, y_offset )
    int width, height, x_offset, y_offset;
{
}

G52_SET_AREA_TEXTURE( alphabet, character )
    int alphabet, character;
{
    struct regis_cntx *rs = RSTRUCT;

    rs->gid_sh_alphabet = alphabet;
    rs->gid_sh_character = character;
    rs->gid_sh_pattern = rs->gid_pattern & 0xffff;
    rs->gid_sh_multiplier = rs->gid_multiplier;
}

G53_SET_AREA_SIZE( width, height )
    int width, height;
{
    struct regis_cntx *rs = RSTRUCT;

    rs->gid_sh_width = MAP_X_RELATIVE( width );
    if ( rs->gid_sh_width < 8 )
	rs->gid_sh_width = 8;
    rs->gid_sh_height = MAP_Y_RELATIVE( height );
    if ( rs->gid_sh_height < 8 )
	rs->gid_sh_height = 8;
/*    XQueryBestSize( rs->display, StippleShape, rs->window, rs->gid_sh_width,
      rs->gid_sh_height, &rs->gid_sh_width, &rs->gid_sh_height );
*/
}

G54_SET_AREA_CELL_SIZE( width, height )
    int width, height;
{
}

G55_SET_COLOR_MAP_ENTRY( color, r, g, b, mono )
    int color, r, g, b, mono;
{
    struct regis_cntx *rs = RSTRUCT;
    DECtermWidget w = rs->widget;
    Pixel pixel;

    if ( ! w->common.color_map )
	return;

    w->common.color_map_mono[color] = mono;
    if ( ! rs->color_monitor )
	{
	r = mono;
	g = mono;
	b = mono;
	}

    if ( ONE_PLANE(w) )
	{
	if ( rs->screen_background_mono < 0x8000 )
	    if ( mono >= 0x4000 )
		pixel = w->common.white_pixel;
	    else
		pixel = w->common.black_pixel;
	else
	    if ( mono >= 0xc000 )
		pixel = w->common.white_pixel;
	    else
		pixel = w->common.black_pixel;
	w->common.color_map[color].pixel = pixel;
	}
	
    else
	{  /* on a multi-plane system, need to allocate/store color */
	if ( w->common.graphics_mode == ALLOCATED_COLORS
	  && w->common.pixel_allocated[ color ]
	  && w->common.color_map_allocated )
	    {  /* already an allocated pixel there, so add to free list */
	    regis_bump_color( w, color );
	    }
	w->common.color_map[color].red = r;
	w->common.color_map[color].green = g;
	w->common.color_map[color].blue = b;
	w->common.color_map[color].flags = DoRed | DoGreen | DoBlue;
	if ( ! w->common.color_map_allocated )
	    return;
	if ( w->common.graphics_mode == ALLOCATED_COLORS )
	    {  /* allocate a shared color */
	    if ( color == w->common.text_foreground_index
	      || color == w->common.text_background_index )
		{
		XAllocColor( rs->display, w->core.colormap,
		  &w->common.color_map[ color ] );
				/* assume success for now */
		if ( color == w->common.text_foreground_index )
		    o_set_foreground_color( w,
		      w->common.color_map[color].pixel );
		else if ( color == w->common.text_background_index )
		    o_set_background_color( w,
		      w->common.color_map[color].pixel );
		w->common.pixel_allocated[color] = TRUE;
		w->common.pixel_valid[color] = TRUE;
		}
	    else
		{
		w->common.pixel_allocated[color] = FALSE;
		w->common.pixel_valid[color] = FALSE;
		}
	    }
	else
	    XStoreColor( rs->display, w->core.colormap,
	      &w->common.color_map[ color ] );
	}
    o_set_cursor_plane_mask( w );
}

G56_SET_POSITION( x, y )
    int x, y;
{
    struct regis_cntx *rs;

    rs = RSTRUCT;
    rs->x_soft_pos = x;
    rs->y_soft_pos = y;
}

G58_DRAW_LINES( length, base_of_array )
    int length, *base_of_array;
{
    int i;
    struct regis_cntx *rs;

    rs = RSTRUCT;

    if ( rs->sc_current_opcode == 0 || rs->numpoints == 0 )
	{
	rs->point_list[0].x = MAP_X_ABSOLUTE( rs->x_soft_pos );
	rs->point_list[0].y = MAP_Y_ABSOLUTE( rs->y_soft_pos );
	rs->numpoints = 1;
	}
    if ( rs->sc_current_opcode == 0 )
	{
	for ( i = 0; i < length; i += 2 )
	    {
	    rs->point_list[rs->numpoints].x =
	      MAP_X_ABSOLUTE( base_of_array[i] );
	    rs->point_list[rs->numpoints].y =
	      MAP_Y_ABSOLUTE( base_of_array[i+1] );
	    rs->numpoints++;
	    }
	rs->x_soft_pos = base_of_array[length-2];
	rs->y_soft_pos = base_of_array[length-1];
	rs->sc_current_opcode = G58_OP_DRAW_LINES;
	G2_END_LIST();
	}
    else if ( rs->sc_current_opcode == G64_OP_BEGIN_FILLED_FIGURE )
	rgid_process( base_of_array, length );
}

G59_DRAW_LINES_END_LIST()
{
    struct regis_cntx *rs;

    rs = RSTRUCT;

    if ( rs->sc_current_opcode == G64_OP_BEGIN_FILLED_FIGURE
      && rs->numpoints > 0)
	return;		/* already in a filled figure */
    rs->point_list[0].x = MAP_X_ABSOLUTE( rs->x_soft_pos );
    rs->point_list[0].y = MAP_Y_ABSOLUTE( rs->y_soft_pos );
    rs->numpoints = 1;
    if ( rs->sc_current_opcode == 0 )
	rs->sc_current_opcode = G58_OP_DRAW_LINES;
}

G64_BEGIN_FILLED_FIGURE()
{
    struct regis_cntx *rs;

    rs = RSTRUCT;
    rs->sc_current_opcode = G64_OP_BEGIN_FILLED_FIGURE;
    rs->numpoints = 0;
}

G65_END_FILLED_FIGURE()
{
    struct regis_cntx *rs = RSTRUCT;
    DECtermWidget w = rs->widget;
    int fill_style, foreground, background, pattern, mode, i, j, cell_byte;
    int storage_width, storage_height, cell_width, cell_height, glyph_size;
    unsigned long foreground_mono, background_mono;
    char *glyph, *glyph2, *gptr;
    Boolean allocated_glyph = False;
    Boolean inverted;
    XImage image;

    rs = RSTRUCT;
    if ( rs->sc_current_opcode != G64_OP_BEGIN_FILLED_FIGURE )
	return;
    check_backing_store( w );
    inverted = ( ( rs->writing_mode & 1 ) != 0 );
    pattern = rs->gid_sh_pattern;
    if ( inverted )
	pattern = (~pattern) & 0xffff;
    mode = rs->writing_mode & ~1;

    if ( rs->numpoints >= 3 && ! ( rs->gid_sh_character <= 0
      && ( mode == G42_MODE_OVERLAY || mode == G40_MODE_COMPLEMENT )
      && pattern == 0 || mode == G38_MODE_TRANSPARENT ) )
	{
	if ( rs->gid_sh_character <= 0 && ( pattern == 0 || pattern == 0xffff )
	  || mode == G46_MODE_ERASE
	  || rs->foreground_index == rs->background_index )
	    fill_style = FillSolid;
	else if ( mode == G44_MODE_REPLACE )
	    fill_style = FillOpaqueStippled;
	else
	    fill_style = FillStippled;
	if ( rs->writing_mode == G46_MODE_ERASE
	  || ( rs->gid_sh_character <= 0
	  && mode == G44_MODE_REPLACE && pattern == 0 ) )
	    {
	    foreground = fetch_color( rs->background_index );
	    foreground_mono = w->common.color_map[rs->background_index].green;
	    }
	else
	    {
	    foreground = fetch_color( rs->foreground_index );
	    foreground_mono = w->common.color_map[rs->foreground_index].green;
	    }
	if ( fill_style == FillOpaqueStippled )
	    {
	    background = fetch_color( rs->background_index );
	    background_mono = w->common.color_map[rs->background_index].green;
	    }
	if ( ONE_PLANE(w)
	  && ( fill_style == FillSolid
	  && mode != G40_MODE_COMPLEMENT
	  && 0x4000 <= foreground_mono && foreground_mono < 0xc000
	  || fill_style == FillOpaqueStippled
	  && rs->gid_sh_alphabet > 0 && foreground != background
	  && glyph_is_halftone( rs->font_glyphs[ rs->gid_sh_alphabet ]
	  [ rs->gid_sh_character ], MAX2_ALPH_CELL_WIDTH,
	  MAX1_ALPH_CELL_HEIGHT - 2 ) ) )
	    {  /* on a single plane system, simulate solid gray with halftone */
	    if ( fill_style == FillOpaqueStippled )
		foreground_mono = ( foreground_mono + background_mono ) / 2;
	    if ( 0x4000 <= foreground_mono && foreground_mono < 0x8000 )
		glyph = dark_gray;
	    else if ( 0x8000 <= foreground_mono && foreground_mono < 0xc000 )
		glyph = light_gray;
	    else
		{
		glyph = NULL;
		fill_style = FillSolid;
		if ( foreground_mono < 0x4000 )
		    foreground = w->common.black_pixel;
		else
		    foreground = w->common.white_pixel;
		}
	    if ( glyph != NULL )
		{
		allocated_glyph = False;
		cell_width = 8;
		cell_height = 8;
		foreground = w->common.white_pixel;
		background = w->common.black_pixel;
		fill_style = FillOpaqueStippled;
		}
	    }
	else if ( fill_style != FillSolid )
	    {
	    if ( rs->gid_sh_character > 0 )
		{
		cell_width = rs->gid_sh_width;
		cell_height = rs->gid_sh_height;
		if ( rs->gid_sh_alphabet == 0 )
		    {
		    storage_width = rs->char_width;
		    storage_height = rs->char_height - 2;
		    }
		else
		    {
		    if ( rs->kanji_regis )
			{
			storage_width = rs->al_width[rs->gid_alphabet];
			storage_height = rs->al_height[rs->gid_alphabet] - 2;
			}
		    else
			{
			storage_width = rs->max2_alph_cell_width;
			storage_height = rs->max1_alph_cell_height - 2;
			}
		    }
		if ( rs->font_glyphs[ rs->gid_sh_alphabet ]
		  [ rs->gid_sh_character ] == NULL )
		    get_glyph( rs->gid_sh_alphabet, rs->gid_sh_character );
		glyph = rs->font_glyphs[ rs->gid_sh_alphabet]
		  [ rs->gid_sh_character ];
		glyph_size = ( storage_width + 7 ) / 8 * storage_height;
		allocated_glyph = False;

#ifdef CANNED_TEXT_FONT
		if ( rs->gid_sh_alphabet == 0 )
		    {
		    glyph2 = XtMalloc( glyph_size );
		    reverse_character( glyph, storage_width, storage_height,
		      glyph2 );
		    if ( allocated_glyph )
			XtFree( glyph );
		    glyph = glyph2;
		    allocated_glyph = True;
		    }
#endif

		if ( inverted )
		    {
		    if ( ! allocated_glyph )
			glyph2 = XtMalloc( glyph_size );
		    else
			glyph2 = glyph;
		    negate_character( glyph, storage_width, storage_height,
		      glyph2 );
		    glyph = glyph2;
		    allocated_glyph = True;
		    }

		if ( cell_width != storage_width
		  || cell_height != storage_height )
		    {
		    glyph_size = ( cell_width + 7 ) / 8 * cell_height;
		    glyph2 = XtMalloc( glyph_size );
		    scale_character( glyph, storage_width, storage_height,
		      glyph2, cell_width, cell_height, cell_width );
		    if ( allocated_glyph )
			XtFree( glyph );
		    glyph = glyph2;
		    allocated_glyph = True;
		    }
		}
	    else
		{  /* use a line pattern instead */
		cell_width = 8;
		cell_height = 16 * rs->gid_sh_multiplier;
		glyph = XtMalloc( cell_height );
		allocated_glyph = True;
		gptr = glyph;
		for ( i = 0; i < 16; i++ )
		    {
		    if ( pattern & 1 )
			cell_byte = 0xff;
		    else
			cell_byte = 0;
		    pattern >>= 1;
		    for ( j = 0; j < rs->gid_sh_multiplier; j++ )
			*gptr++ = cell_byte;
		    }
		}
	    }
	if ( fill_style != rs->shade_gc_fill_style )
	    {
	    XSetFillStyle( rs->display, rs->shade_gc, fill_style );
	    if ( w->common.backing_store_active )
		XSetFillStyle( rs->display, rs->bs_shade_gc, fill_style );
	    rs->shade_gc_fill_style = fill_style;
	    }
	if ( foreground != rs->shade_gc_foreground_color
	  && mode != G40_MODE_COMPLEMENT )
	    {
	    XSetForeground( rs->display, rs->shade_gc, foreground );
	    if ( w->common.backing_store_active )
		XSetForeground( rs->display, rs->bs_shade_gc, foreground );
	    rs->shade_gc_foreground_color = foreground;
	    }
	if ( rs->shade_gc_fill_style == FillOpaqueStippled
	  && rs->shade_gc_background_color != background )
	    {
	    XSetBackground( rs->display, rs->shade_gc, background );
	    if ( w->common.backing_store_active )
		XSetBackground( rs->display, rs->bs_shade_gc, background );
	    rs->shade_gc_background_color = background;
	    }
	if ( rs->shade_gc_function != rs->function )
	    {
	    XSetFunction( rs->display, rs->shade_gc, rs->function );
	    if ( w->common.backing_store_active )
		XSetFunction( rs->display, rs->bs_shade_gc, rs->function );
	    rs->shade_gc_function = rs->function;
	    }
	if ( ( w->common.graphics_mode == ALLOCATED_PLANES
	    || w->common.graphics_mode == ALLOCATED_COLORMAP )
	  && rs->shade_gc_plane_mask != rs->plane_mask )
	    {
	    XSetPlaneMask( rs->display, rs->shade_gc, rs->plane_mask );
	    if ( w->common.backing_store_active )
		XSetPlaneMask( rs->display, rs->bs_shade_gc, rs->plane_mask );
	    rs->shade_gc_plane_mask = rs->plane_mask;
	    }
	if ( fill_style != FillSolid && ! ( rs->gid_sh_character > 0 &&
	  rs->gid_sh_alphabet == rs->spx_alphabet && rs->gid_sh_character ==
	  rs->spx_character && cell_width == rs->spx_width && cell_height ==
	  rs->spx_height && ! rs->sh_glyph_changed &&
	  inverted == rs->spx_inverted ) )
	    {
	    rs->sh_glyph_changed = FALSE;
#if 0
	    if ( cell_width != rs->spx_width || cell_height != rs->spx_height )
#endif
		{
		if ( rs->spx_pixmap != NULL )
		    {
		    XFreeGC( rs->display, rs->spx_gc );
#if 1
if ( cell_width || rs->spx_width || cell_height != rs->spx_height )
#endif
		    XFreePixmap( rs->display, rs->spx_pixmap );
		    }
#if 1
if ( cell_width || rs->spx_width || cell_height != rs->spx_height )
#endif
		rs->spx_pixmap = XCreatePixmap( rs->display, rs->window,
		  cell_width, cell_height, 1 );
		rs->spx_width = cell_width;
		rs->spx_height = cell_height;
		rs->spx_gc = XCreateGC( rs->display, rs->spx_pixmap, 0, NULL );
		XSetForeground( rs->display, rs->spx_gc, 1 );
		XSetBackground( rs->display, rs->spx_gc, 0 );
		}
	    image.width = cell_width;
	    image.height = cell_height;
	    image.xoffset = 0;
	    image.format = XYBitmap;
	    image.data = glyph;
	    image.byte_order = LSBFirst;
	    image.bitmap_unit = 8;
	    image.bitmap_bit_order = LSBFirst;
	    image.bitmap_pad = 8;
	    image.depth = 1;
	    image.bytes_per_line = ( cell_width + 7 ) / 8;
	    image.bits_per_pixel = 1;
	    image.obdata = NULL;
	    XPutImage( rs->display, rs->spx_pixmap, rs->spx_gc, &image,
	      0, 0, 0, 0, cell_width, cell_height );
	    XSetStipple( rs->display, rs->shade_gc, rs->spx_pixmap );
	    if ( w->common.backing_store_active )
		XSetStipple( rs->display, rs->bs_shade_gc, rs->spx_pixmap );
	    rs->spx_alphabet = rs->gid_sh_alphabet;
	    rs->spx_character = rs->gid_sh_character;
	    rs->spx_inverted = inverted;
	    }
	if ( allocated_glyph )
	    XtFree( glyph );
	XFillPolygon( rs->display, rs->window, rs->shade_gc, rs->point_list,
	  rs->numpoints, Complex, CoordModeOrigin );
	if ( w->common.backing_store_active )
	    {
	    adjust_point_list();
	    XFillPolygon( rs->display, rs->bs_pixmap, rs->bs_shade_gc,
	      rs->point_list, rs->numpoints, Complex, CoordModeOrigin );
	    }
	}
    rs->numpoints = 0;
    rs->sc_current_opcode = 0;
}

G66_SET_ALPHABET( alphabet )
    int alphabet;
{
    struct regis_cntx *rs = RSTRUCT;

    rs->gid_alphabet = alphabet;
}

G67_CREATE_ALPHABET( width, height, extent, type )
    int width, height, extent, type;
{
}

G68_LOAD_CHARACTER_CELL( character, length, base_of_array )
    int character, length, *base_of_array;
{
    struct regis_cntx *rs = RSTRUCT;
    int i;
    int j, width, height;
    char *char_base_of_array = (char *)base_of_array;

    if ( rs->gid_alphabet <= 0
      || rs->gid_alphabet >= TOTAL3_NUMBER_OF_ALPHABETS )
	return;
    rs->sh_glyph_changed = TRUE;	/* signals that we have to redraw
					   shading pixmap */
  if ( rs->kanji_regis ) {

    width = ( rs->al_width[ rs->gid_alphabet ] + 7  ) / 8;
    height = rs->al_height[ rs->gid_alphabet ];

    if ( rs->font_glyphs[ rs->gid_alphabet ][ character ] != solid_wide &&
	 rs->font_glyphs[ rs->gid_alphabet ][ character ] != NULL )
	XtFree( rs->font_glyphs[ rs->gid_alphabet ][ character ] );

    rs->font_glyphs[ rs->gid_alphabet][ character ] = 
	XtMalloc( width * height );

    for ( i = 0; i < height; i++ )
	for ( j = 0; j < width; j++ ) 
	    rs->font_glyphs[ rs->gid_alphabet ][ character ][ j + i * width ] =
		char_base_of_array[ j + i * sizeof( int ) ];
  } else {
    if ( rs->font_glyphs[ rs->gid_alphabet ][ character ] != solid &&
	 rs->font_glyphs[ rs->gid_alphabet ][ character ] != NULL )
	XtFree( rs->font_glyphs[rs->gid_alphabet][character] );

    rs->font_glyphs[ rs->gid_alphabet][ character ] =
	  XtMalloc( H_ALPH_CELL_HEIGHT_DEFAULT );

    for ( i = 0; i < length; i++ )
	rs->font_glyphs[ rs->gid_alphabet ][ character ][ i ] =
	  base_of_array[ i ];
    for (; i < H_ALPH_CELL_HEIGHT_DEFAULT; i++ )
	rs->font_glyphs[ rs->gid_alphabet ][ character ][ i ] = 0;
  }
}

G71_BEGIN_DEFINE_CHARACTER( character, width )
    int character, width;
{
}

G72_END_DEFINE_CHARACTER()
{
}

G73_LOAD_BY_NAME( length, base_of_array )
    int length, *base_of_array;
{
}

G74_LOAD_BY_NAME_END_LIST()
{
}

G75_SET_CELL_DISPLAY_SIZE( width, height )
    int width, height;
{
    struct regis_cntx *rs = RSTRUCT;

/*
 * We keep the display size in both soft and hard units.  The soft units
 * are needed so when the display size is the same as the escapement in the
 * x and/or y directions we can re-calculate the hard display size before
 * drawing each character; this prevents gaps from being drawn between
 * characters.
 */
    rs->gid_x_soft_display_size = width;
    rs->gid_y_soft_display_size = height;
    rs->gid_x_hard_display_size = MAP_X_RELATIVE( width );
    rs->gid_y_hard_display_size = MAP_Y_RELATIVE( height );
}

G76_SET_CELL_UNIT_SIZE( width, height )
    int width, height;
{
    struct regis_cntx *rs = RSTRUCT;

    rs->gid_x_hard_unit_size = MAP_X_RELATIVE( width );
    rs->gid_y_hard_unit_size = MAP_Y_RELATIVE( height );
}

G77_SET_CELL_MOVEMENT_MODE( flag )
    int flag;
{
}

G83_SET_CELL_EXPLICIT_MOVEMENT( dx, dy )
    int dx, dy;
{
    struct regis_cntx *rs = RSTRUCT;

    rs->gid_x_soft_escapement = dx;
    rs->gid_y_soft_escapement = dy;
}

GSET_CELL_RENDITION( flags )
    int flags;
{
}

G90_SET_CELL_ROTATION( angle )
    int angle;
{
    struct regis_cntx *rs = RSTRUCT;

    rs->gid_direction = angle;
}

G91_SET_CELL_OBLIQUE( angle )
    int angle;
{
    struct regis_cntx *rs = RSTRUCT;

    rs->gid_oblique = angle;
}

G92_SET_CELL_ORIGIN( ox, oy )
    int ox, oy;
{
}

G93_SET_CELL_ALIGNMENT( x_offset, y_offset )
    int x_offset, y_offset;
{
}

G94_DRAW_CHARACTERS( length, base_of_array )
    int length, *base_of_array;
{
    struct regis_cntx *rs;

    rs = RSTRUCT;
    rs->sc_current_opcode = G94_OP_DRAW_CHARACTERS;
    rgid_process( base_of_array, length );
    G2_END_LIST();
}

G95_DRAW_CHARACTERS_END_LIST()
{
    struct regis_cntx *rs;

    rs = RSTRUCT;
    rs->sc_current_opcode = G94_OP_DRAW_CHARACTERS;
}

G96_ERASE_CLIPPING_REGION()
{
}

G97_SCROLL_CLIPPING_REGION( dx, dy )
    int dx, dy;
{
}

G98_COPY_RECTANGLE( src_x, src_y, dest_x, dest_y, width, height )
    int src_x, src_y, dest_x, dest_y, width, height;
{
}

G99_FLOOD_INTERIOR()
{
}

G100_FLOOD_TO_BORDER( color )
    int color;
{
}

G101_REQUEST_STATUS()
{
}

G104_REQUEST_CURRENT_POSITION()
{
}

G105_REQUEST_CELL_STANDARD()
{
}

G106_REQUEST_INPUT_SIZE()
{
}

G107_REQUEST_OUTPUT_SIZE()
{
}

G108_REQUEST_DEVICE_CHAR()
{
}

G116_SET_INPUT_TRIGGER( type, data )
    int type, data;
{
}

/*
 * Not sure what this is, but I'm using it to enable button handling.  Bob.
 */

G121_ENABLE_INPUT()
{
    struct regis_cntx *rs = RSTRUCT;
    DECtermWidget w = rs->widget;

    rs->text_button_handler = w->input.button_handler;
    i_enable_button( w, regis_button_handler );
}

/*
 * Disable button handling.
 */

G122_DISABLE_INPUT()
{
    struct regis_cntx *rs = RSTRUCT;
    DECtermWidget w = rs->widget;

    i_enable_button( w, rs->text_button_handler );
}

/* I'm not sure what this was originally intended to do, but I'm using it to
 * block the emulator while it's waiting for input in one-shot input mode. Bob.
 */

G123_REQUEST_INPUT()
{
    struct regis_cntx *rs = RSTRUCT;
    DECtermWidget w = rs->widget;

    rs->input_pending = TRUE;
    i_enable_input( w, regis_input_handler );
    G121_ENABLE_INPUT();	/* enable button handling */
    s_stop_output( w, STOP_OUTPUT_OTHER );
    rs->paused = TRUE;
    if ( rs->sc_source == 0 )
	{
	rs->paused_data = rs->sc_pointer;
	rs->sc_pointer = "";
	}
    else
	{
	rs->paused_data = rs->sc_stack[0];
	rs->sc_stack[0] = "";
	}
    rs->bufpos = NULL;
    rs->bufpos = ( char * ) getl_regis();
	/* this will block until input comes in */
}

/* I'm not sure what this is for either, but I'm using it to request the
 * locator position in multiple input mode.  Bob.
 */

G124_TRIGGER_INPUT( flags )
    int flags;	/* ignored, at least for now */
{
    struct regis_cntx *rs = RSTRUCT;
    DECtermWidget w = rs->widget;
    int bad_position = FALSE;
    int x, y, rx, ry, dummy, loc_length;
    char *loc_data;
    Window wdummy;

    if (!XQueryPointer( XtDisplay(w), XtWindow(w), &wdummy, &wdummy,
		&dummy, &dummy, &x, &y, (unsigned int *)&dummy )
	     || x < X_MARGIN || x >= X_MARGIN + w->common.display_width
	     || y < Y_MARGIN || y >= Y_MARGIN + w->common.display_height )
	{	/* locator is off screen or outside display area */
	rx = ( -1 );	/* special error indicator */
	ry = ( -1 );
	}
    else
	{	/* in display area, so convert to coordinates */
	rx = UNMAP_X_ABSOLUTE( x );
	ry = UNMAP_Y_ABSOLUTE( y );
	if ( rx < 0 || rx >= rs->output_ids_width
	  || ry < 0 || ry >= rs->output_ids_height )
	    {	/* locator is outside addressable area */
	    rx = ( -1 );
	    ry = ( -1 );
	    }
	}

    loc_length = s_return_lkd( w, 0, &loc_data );
			/* look up LKD button 0 down */
    if ( loc_length != 0 )
	PUT_REPORT( loc_data, loc_length );
    if ( rx < 0 || ry < 0 )
	PUT_REPORT( "[]\r", 3 );	/* error indicator */
    else
	rprt2_position( rx, ry, TRUE );	/* solicited report */
}

G127_SET_FILL_OFF()
{
    struct regis_cntx *rs = RSTRUCT;

    rs->numpoints = 0;
    rs->sc_current_opcode = 0;
}

G139_WAIT( ticks )
    int ticks;
{
    int milliseconds;
    struct regis_cntx *rs = RSTRUCT;
    DECtermWidget w = rs->widget;
    extern void regis_timeout_expired();

    milliseconds = ticks * 50 / 3;
    rs->timer = XtAppAddTimeOut( XtWidgetToApplicationContext( (Widget)w ),
	milliseconds, regis_timeout_expired, w );
    s_stop_output( w, STOP_OUTPUT_OTHER );
    rs->paused = TRUE;
    if ( rs->sc_source == 0 )
	{
	rs->paused_data = rs->sc_pointer;
	rs->sc_pointer = "";
	}
    else
	{
	rs->paused_data = rs->sc_stack[0];
	rs->sc_stack[0] = "";
	}
    rs->bufpos = NULL;
    rs->bufpos = (char *) getl_regis();
}

G140_PRINT_SCREEN( x, y, w, h, hx, hy )
    int x, y, w, h, hx, hy;
{
    struct regis_cntx *rs = RSTRUCT;

    print_graphics_screen( rs->widget,
	MAP_X_ABSOLUTE( x ) - X_MARGIN, MAP_Y_ABSOLUTE( y ) - Y_MARGIN,
	MAP_X_RELATIVE( w ), MAP_Y_RELATIVE( h ),
	hx, hy, TRUE );
}

rgid_process( buffer, length )
    int *buffer;
    int length;
{
    int i;
    struct regis_cntx *rs;

    rs = RSTRUCT;
    if ( rs->sc_current_opcode == G58_OP_DRAW_LINES
      || rs->sc_current_opcode == G64_OP_BEGIN_FILLED_FIGURE )
	{  /* drawing lines, so add new points */
	for ( i = 0; i < length; i += 2 )
	    {  /* for each x,y pair, append to point_list */
	    if ( rs->numpoints >= REG_MAX_POINTS )
		if ( rs->sc_current_opcode == G64_OP_BEGIN_FILLED_FIGURE )
		    break;	/* ignore extra points in filled figures */
		else
		    {  /* draw the vectors we have, then start a new series */
		    G2_END_LIST();
		    G59_DRAW_LINES_END_LIST();
		    }
	    rs->point_list[rs->numpoints].x =
	      MAP_X_ABSOLUTE( buffer[i] );
	    rs->point_list[rs->numpoints++].y =
	      MAP_Y_ABSOLUTE( buffer[i+1] );
	    }
	rs->x_soft_pos = buffer[length-2];
	rs->y_soft_pos = buffer[length-1];
	}
    else if ( rs->sc_current_opcode == G94_OP_DRAW_CHARACTERS )
	{
	for ( i = 0; i < length; i++ )
	    {
	    draw_character( buffer[i] );
	    }
	}
}

gid_report( buf_addr, buf_len )
    int *buf_addr;
    int buf_len;
{
    return 0;
}

gr_flush()
{
}

SFreqr_request_report( buffer, ldblock )
    int *buffer;
    struct LDblock *ldblock;
{
}

draw_character( ch )
    unsigned char ch;
{
    struct regis_cntx *rs = RSTRUCT;
    DECtermWidget w = rs->widget;
    int picture_width = w->common.display_width,
	picture_height = w->common.display_height;
    XImage image;
    int glyph_size, cell_width, cell_height, function;
    int storage_width, storage_height;
    int display_width, display_height;
    int unit_width, unit_height;
    int x_hard_pos, y_hard_pos;
    int new_x_soft_pos, new_y_soft_pos;
    int new_x_hard_pos, new_y_hard_pos;
    int x_offset, y_offset, x_origin, y_origin;
    Pixel foreground_color, background_color;
    char *glyph, *glyph2;
    Boolean allocated_glyph, fast_path, drew_background;
    int sav_x_soft_escapement, sav_y_soft_escapement;

    check_backing_store( w );

    if ( rs->kanji_regis )
    {
	if ( !rs->char_stack_empty )
	    {
	    int temp;
/* this swapping is needed to pass UDC */
	    temp = ch;
	    ch = rs->char_stack;
	    rs->char_stack = temp;

	    rs->char_width *= 2;
	    rs->gid_x_hard_unit_size *= 2;
	    rs->gid_x_hard_display_size *= 2;
	    sav_x_soft_escapement = rs->gid_x_soft_escapement;
	    sav_y_soft_escapement = rs->gid_y_soft_escapement;
	    if ( rs->gid_direction % 180 < 45 )
		rs->gid_x_soft_escapement *= 2;
	    else if ( rs->gid_direction % 180 < 90 )
		{
		rs->gid_x_soft_escapement = (int)( rs->gid_x_soft_escapement * 1.4142 );
		rs->gid_y_soft_escapement = (int)( rs->gid_y_soft_escapement * 1.4142 );
		}
	    else if ( rs->gid_direction % 180 < 135 )
		rs->gid_y_soft_escapement *= 2;
	    else
		{
		rs->gid_x_soft_escapement = (int)( rs->gid_x_soft_escapement * 1.4142 );
		rs->gid_y_soft_escapement = (int)( rs->gid_y_soft_escapement * 1.4142 );
		}
	    }
	else if ( (ch < 128 && rs->g_char_set[0] == REG_DEC_KANJI) ||
		(ch >= 128 && rs->g_char_set[1] == REG_DEC_KANJI) )
	    {                                     
	    rs->char_stack = ch;
	    rs->char_stack_empty = False;
	    return;
	    }
    }
    else
    {

/*
 * For 8 bit characters in alphabet 0, re-map the character depending on
 * the user preference set.
 */
    if ( ch >= 0xa0 && rs->gid_alphabet == 0 )
	{
	if ( w->common.userPreferenceSet == DECwDEC_Supplemental )
	    {
	    if ( w->common.v1_encodings )
		{
		if ( dec_supplemental[ ch - 32 - 128 ] == 0 )
		    ch = 32;	/* really should be error character */
		}
	    else
		{  /* find equivalent in ISO Latin 1 */
		ch = dec_supplemental[ ch - 32 - 128 ];
		if ( ch == 0 )
		    ch = 32;	/* really should be error character */
		}
	    }
	else
	    {
	    if ( w->common.v1_encodings )
		{  /* convert to DEC Multinational */
		ch = iso_latin_1[ ch - 32 - 128 ];
		}
	    }
	}

    }
/*
 * Calculate the current position in the physical (hard) coordinate space
 * (pixels).  We recalculate before drawing each character, so that each
 * character is drawn as close to its correct position as possible, but the
 * characters may not be equally spaced.
 */
    x_hard_pos = MAP_X_ABSOLUTE( rs->x_soft_pos );
    y_hard_pos = MAP_Y_ABSOLUTE( rs->y_soft_pos );

/*
 * Calculate the soft and hard position that will be set after this character
 * is drawn, based on the escapement.
 */
    new_x_soft_pos = rs->x_soft_pos + rs->gid_x_soft_escapement;
    new_x_hard_pos = MAP_X_ABSOLUTE( new_x_soft_pos );

    new_y_soft_pos = rs->y_soft_pos + rs->gid_y_soft_escapement;
    new_y_hard_pos = MAP_Y_ABSOLUTE( new_y_soft_pos );

/*
 * Calculate the hard display to use in drawing this character.  If the
 * display size is the same as the escapement in x and/or y we calculate the
 * size such that there is no gap between adjacent characters; otherwise
 * we use the pre-computed hard display size.
 */

    if ( rs->gid_x_hard_display_size ==
		MAP_X_RELATIVE(rs->gid_x_soft_escapement) )
	display_width = new_x_hard_pos - x_hard_pos;
    else
	display_width = rs->gid_x_hard_display_size;

    if ( rs->gid_y_hard_display_size ==
		MAP_Y_RELATIVE(rs->gid_y_soft_escapement) )
	display_height = new_y_hard_pos - y_hard_pos;
    else
	display_height = rs->gid_y_hard_display_size;

/*
 * Find the unit size, and scale the character to the right size
 */

    cell_width = rs->gid_x_hard_unit_size;
    cell_height = rs->gid_y_hard_unit_size;

    if ( rs->gid_alphabet == 0 )
	{
	storage_width = rs->char_width;
	storage_height = rs->char_height;
	}
    else
	{
	if ( rs->kanji_regis )
	    {
	    storage_width = rs->al_width[rs->gid_alphabet];
	    storage_height = rs->al_height[rs->gid_alphabet];
	    }
	else
	    {
	    storage_width = rs->max2_alph_cell_width;
	    storage_height = rs->max1_alph_cell_height;
	    }
	}

/* fast path means draw characters as text instead of as bitmaps */

    fast_path = ( rstruct->gid_alphabet == 0
      && cell_width == storage_width
      && cell_height == storage_height
      && display_width == storage_width
      && display_height == storage_height
      && rs->gid_direction == 0 && rs->gid_oblique == 0
#if 1	/* as long as XDrawString is broken */
      && ( w->common.hardware_planes == 1 
	|| rs->writing_mode != G42_MODE_OVERLAY
      && rs->writing_mode != G40_MODE_COMPLEMENT )
#endif
      && rs->writing_mode != G43_MODE_OVERLAY_NEGATE
      && rs->writing_mode != G41_MODE_COMPLEMENT_NEGATE );

    if ( fast_path )
	glyph = (char *) -1;	/* so we can say "if ( glyph != NULL )" instead of
			   "if ( fast_path || glyph != NULL )" */

    if ( ( rs->writing_mode & ~1 ) == G44_MODE_REPLACE &&
      ( rs->gid_oblique != 0 || rs->gid_direction % 90 != 0
      || display_width > cell_width || display_height > cell_height )
      || ( rs->writing_mode & ~1 ) == G46_MODE_ERASE )
	{
	draw_character_background( x_hard_pos, y_hard_pos,
		display_width, display_height );
	drew_background = TRUE;
	}
    else
	drew_background = FALSE;

    if ( ( rs->writing_mode & ~1 ) == G46_MODE_ERASE )
	{
	rs->x_soft_pos = new_x_soft_pos;
	rs->y_soft_pos = new_y_soft_pos;
	return;
	}

/* create a bitmap for character unless using the fast path */

    if ( ! fast_path )
	{
	if ( rs->font_glyphs[rs->gid_alphabet][ ch ] == NULL ||
		( rs->kanji_regis && !rs->char_stack_empty ))
	    {  /* read a new font glyph from the server */
	    get_glyph( rs->gid_alphabet, ch );
	    }

/* scale the character if necessary */

	unit_width = cell_width;
	unit_height = cell_height;
	if ( rs->writing_mode != G42_MODE_OVERLAY
	  && rs->writing_mode != G40_MODE_COMPLEMENT )
	    {
	    if ( cell_width < display_width )
		cell_width = display_width;
	    if ( cell_height < display_height )
		cell_height = display_height;
	    }
/*
 * We impose a restriction here that the character be no larger than twice
 * the entire picture size.  This is to work around a problem where we were
 * crashing due to XPutImage filling up the regis stack on the following
 * command:
 *
 *	S(A[0,0][1,1])T(U[32,80])"A"
 *
 * At first, I thought we could merely restrict the glyph size to the picture
 * size, and let scale_character scale a portion of the character to any
 * size it likes.  But that is wrong, because suppose user scales the character
 * to huge but starts drawing it way up and to the left off the screen,
 * expecting the lower right portion of the character to appear on the
 * screen.
 *
 * By restricting the character to twice the picture size, it's still not
 * correct, but it's better than a crash.  And it means that some amount of
 * repositioning the letter will work o.k.
 *
 * The correct solution is to only allocate glyph to be the portion of the
 * character that will appear on the screen.  But then the other routines,
 * such as oblique_character and rotate_character would have to be
 * rewritten.  For example, rotate_character probably rotates the character
 * around the middle of its given glyph, and would have to be taught to have
 * arbitrary center of rotations, even ones outside the given glyph itself.
 *
 * Note:  I've only fixed it here, and not in end_filled_figure, where I
 * bet someone could concoct an example that crashes too !
 */
	cell_width = min( cell_width, 2*picture_width );
	cell_height = min( cell_height, 2*picture_height );
	unit_width = min( unit_width, 2*picture_width );
	unit_height = min( unit_height, 2*picture_height );

	glyph_size = ( cell_width + 7 ) / 8 * cell_height;

	if ( cell_width == 0 || cell_height == 0 )
	    {
	    glyph = NULL;
	    allocated_glyph = False;
	    }
	else if ( cell_width == storage_width && cell_height == storage_height
	  && cell_width == unit_width && cell_height == unit_height )
	    {
	    glyph = rs->font_glyphs[rs->gid_alphabet][ch];
	    allocated_glyph = False;
	    }
	else
	    {
	    glyph = XtCalloc( glyph_size, 1 );
	    allocated_glyph = True;
	    scale_character( rs->font_glyphs[rs->gid_alphabet][ch],
	      storage_width, storage_height, glyph, unit_width, unit_height,
	      cell_width );
	    }

/* negate the character (i.e. invert each pixel) if necessary */

	if ( rs->writing_mode & 1 && glyph != NULL )
	    {
	    if ( ! allocated_glyph )
		glyph2 = XtMalloc( glyph_size );
	    else
		glyph2 = glyph;
	    negate_character( glyph, cell_width, cell_height, glyph2 );
	    if ( allocated_glyph && glyph != glyph2 )
		XtFree( glyph );
	    glyph = glyph2;
	    allocated_glyph = True;
	    }

/* clip the character to the display size if necessary */

	if ( display_width < cell_width || display_height < cell_height )
	    clip_character( glyph, cell_width, cell_height,
	      min( display_width, cell_width ),
	      min( display_height, cell_height ) );

/* oblique the character if necessary */

	if ( rs->gid_oblique != 0 && glyph != NULL )
	    {
	    oblique_character( glyph, cell_width, cell_height, rs->gid_oblique,
	      &glyph2, &cell_width, &cell_height, &x_offset, &y_offset );
	    if ( allocated_glyph && glyph != glyph2 )
		XtFree( glyph );
	    glyph = glyph2;
	    allocated_glyph = TRUE;
	    }
	else
	    {
	    x_offset = 0;
	    y_offset = 0;
	    }

/* rotate the character if necessary */

	if ( rs->gid_direction != 0 && glyph != NULL )
	    {
	    rotate_character( glyph, cell_width, cell_height,
	      x_offset, y_offset, rs->gid_direction, &glyph2,
	      &cell_width, &cell_height, &x_offset, &y_offset );
	    if ( glyph != glyph2 )
		{
		if ( allocated_glyph )
		    XtFree( glyph );
	        glyph = glyph2;
	        allocated_glyph = TRUE;
		}
	    }
	}  /* end of if ( !fast_path ) */

/* draw the character */

    if ( glyph != NULL )
	{
	if ( fast_path )
	    {
	    if ( rs->writing_mode & 1 )
		{  /* replace negate means exchange foreground/background */
		foreground_color = fetch_color( rs->background_index );
		background_color = fetch_color( rs->foreground_index );
		function = GXcopy;
		}
	    else
		{
		foreground_color = fetch_color( rs->foreground_index );
		background_color = fetch_color( rs->background_index );
		if ( rs->writing_mode == G40_MODE_COMPLEMENT )
		    function = GXinvert;
		else
		    function = GXcopy;
		}
	    }
	else if ( ( rs->writing_mode & ~1 ) == G44_MODE_REPLACE
	  && ! drew_background )
	    {
	    foreground_color = fetch_color( rs->foreground_index );
	    background_color = fetch_color( rs->background_index );
	    function = GXcopy;
	    }
	else if ( w->common.hardware_planes > 1 )
	    {
	    if ( ( rs->writing_mode & ~1 ) == G42_MODE_OVERLAY
	      || drew_background )
		{
		foreground_color = fetch_color( rs->foreground_index );
		background_color = 0;
		function = GXor;
		}
	    else	/* complement */
		{
		foreground_color = -1;
		background_color = 0;
		function = GXxor;
		}
	    }
	else
	    {
	    foreground_color = 1;
	    background_color = 0;
	    if ( ( rs->writing_mode & ~1 ) == G40_MODE_COMPLEMENT
	      && ! drew_background )
		function = GXxor;
	    else if ( fetch_color( rs->foreground_index ) == 1 )
		function = GXor;
	    else
		function = GXandInverted;
	    }
	if ( rs->text_gc_background_color != background_color
#if 0	/* always set background until XDrawString is fixed */
	  && ! ( fast_path && ( rs->writing_mode & ~1 ) != G44_MODE_REPLACE ) )
#else
	  )
#endif
	    {
	    XSetBackground( rs->display, rs->text_gc, background_color );
	    if ( w->common.backing_store_active )
		XSetBackground( rs->display, rs->bs_text_gc, background_color );
	    rs->text_gc_background_color = background_color;
	    }
	}
    else
	{
	foreground_color = fetch_color( rs->foreground_index );
	function = rs->function;
	}

    if ( rs->text_gc_foreground_color != foreground_color && glyph != NULL )
	{
	XSetForeground( rs->display, rs->text_gc, foreground_color );
	if ( w->common.backing_store_active )
	    XSetForeground( rs->display, rs->bs_text_gc, foreground_color );
	rs->text_gc_foreground_color = foreground_color;
	}
    if ( rs->text_gc_function != function && glyph != NULL )
	{
	XSetFunction( rs->display, rs->text_gc, function );
	if ( w->common.backing_store_active )
	    XSetFunction( rs->display, rs->bs_text_gc, function );
	rs->text_gc_function = function;
	}
    if ( ( w->common.graphics_mode == ALLOCATED_PLANES
        || w->common.graphics_mode == ALLOCATED_COLORMAP )
      && rs->text_gc_plane_mask != rs->plane_mask && glyph != NULL )
	{
	XSetPlaneMask( rs->display, rs->text_gc, rs->plane_mask );
	if ( w->common.backing_store_active )
	    XSetPlaneMask( rs->display, rs->bs_text_gc, rs->plane_mask );
	rs->text_gc_plane_mask = rs->plane_mask;
	}

    if ( fast_path )
	{
#if 1	/* XDrawString doesn't seem to work */
	if ( rs->writing_mode == G42_MODE_OVERLAY
	  || rs->writing_mode == G40_MODE_COMPLEMENT )
	    {  /* draw only the foreground */
	  if ( rs->kanji_regis )
	    draw_common( rs->display, rs->window, rs->text_gc, x_hard_pos,
	      y_hard_pos + rs->char_ascent, &ch, 0 );
	  else
	    XDrawString( rs->display, rs->window, rs->text_gc, x_hard_pos,
	      y_hard_pos + rs->char_ascent, (char *)&ch, 1 );
	    if ( w->common.backing_store_active )
	      if ( rs->kanji_regis )
		draw_common( rs->display, rs->bs_pixmap, rs->bs_text_gc,
		  x_hard_pos - rs->x_offset,
		  y_hard_pos + rs->char_ascent - rs->y_offset, &ch, 0 );
	      else
		XDrawString( rs->display, rs->bs_pixmap, rs->bs_text_gc,
		  x_hard_pos - rs->x_offset,
		  y_hard_pos + rs->char_ascent - rs->y_offset, (char *)&ch, 1 );
	    }
	else
#endif
	    {  /* draw both the foreground and background */
	  if ( rs->kanji_regis )
	    draw_common( rs->display, rs->window, rs->text_gc,
	      x_hard_pos, y_hard_pos + rs->char_ascent, &ch, 1 );
	  else
	    XDrawImageString( rs->display, rs->window, rs->text_gc,
	      x_hard_pos, y_hard_pos + rs->char_ascent, (char *)&ch, 1 );
	    if ( w->common.backing_store_active )
	      if ( rs->kanji_regis )
		draw_common( rs->display, rs->bs_pixmap, rs->bs_text_gc,
		  x_hard_pos - rs->x_offset,
		  y_hard_pos + rs->char_ascent - rs->y_offset, &ch, 1 );
	      else
		XDrawImageString( rs->display, rs->bs_pixmap, rs->bs_text_gc,
		  x_hard_pos - rs->x_offset,
		  y_hard_pos + rs->char_ascent - rs->y_offset, (char *)&ch, 1 );
	    }
	}
    else if ( glyph != NULL )
    {
    image.width = cell_width;
    image.height = cell_height;
    image.xoffset = 0;
    image.format = XYBitmap;
    image.data = glyph;
    image.byte_order = LSBFirst;
    image.bitmap_unit = 8;
    image.bitmap_bit_order = LSBFirst;
    image.bitmap_pad = 8;
    image.depth = 1;
    image.bytes_per_line = ( cell_width + 7 ) / 8;
    image.bits_per_pixel = 1;
    image.obdata = NULL;
	if ( w->common.hardware_planes > 1 && ( ( rs->writing_mode & ~1 ) ==
	  G42_MODE_OVERLAY ) || drew_background )
	    XPutImage( rs->display, rs->window, rs->text_clear_gc, &image,
	      0, 0, x_hard_pos - x_offset, y_hard_pos - y_offset,
	      cell_width, cell_height );
	XPutImage( rs->display, rs->window, rs->text_gc, &image, 0, 0,
	  x_hard_pos - x_offset, y_hard_pos - y_offset,
	  cell_width, cell_height );
	if ( w->common.backing_store_active )
	    {
	    if ( w->common.hardware_planes > 1 && ( rs->writing_mode & ~1 ) ==
	      G42_MODE_OVERLAY )
		XPutImage( rs->display, rs->bs_pixmap, rs->bs_text_clear_gc,
		  &image, 0, 0, x_hard_pos - x_offset - rs->x_offset,
		  y_hard_pos - y_offset - rs->y_offset, cell_width,
		  cell_height );
	    XPutImage( rs->display, rs->bs_pixmap, rs->bs_text_gc, &image, 0, 0,
	      x_hard_pos - x_offset - rs->x_offset,
	      y_hard_pos - y_offset - rs->y_offset,
	      cell_width, cell_height );
	    }

    if ( allocated_glyph )
	XtFree( glyph );
    }

    rs->x_soft_pos = new_x_soft_pos;
    rs->y_soft_pos = new_y_soft_pos;

    if ( rs->kanji_regis && !rs->char_stack_empty )
    {
	rs->char_width /= 2;
	rs->gid_x_hard_unit_size /= 2;
	rs->gid_x_hard_display_size /= 2;
	rs->gid_x_soft_escapement = sav_x_soft_escapement;
	rs->gid_y_soft_escapement = sav_y_soft_escapement;
	rs->char_stack_empty = True;
    }
}

/* scale_character - change the size of a character bitmap */

scale_character( src, src_width, src_height, dest, dest_width, dest_height,
		 dest_buffer_width )
    char *src;		/* source bitmap */
    int src_width,	/* source width in pixels */
	src_height;	/* source height in pixels */
    char *dest;		/* destination bitmap */
    int dest_width,	/* destination width in pixels */
	dest_height,	/* destination height in pixels */
	dest_buffer_width;
			/* width of destination buffer in pixels */
{
    char *src_row,	/* start of current scan line in source */
	 *src_byte;	/* pointer to byte in source */
    int src_mask,	/* bit mask within source byte */
	src_x,		/* current x pixel in source */
	src_size,	/* number of bytes in source */
	src_wbyte;	/* source width in bytes */
    char *dest_row,	/* start of current scan line in dest */
	 *dest_byte;	/* pointer to byte in dest */
    int dest_mask,	/* bit mask within destination byte */
	dest_x,		/* current x pixel in destination */
	dest_size,	/* number of bytes in destination */
	dest_wbyte,	/* destination width */
	height_error,	/* error term for height */
	width_error;	/* error term for width */

/*

There are seven cases to consider:
	1. Source and destination are the same width and height
	2. Source and destination are the same width but the source
	   is shorter.
	3. Source and destination are the same width but the source
	   is taller.
	4. Source is narrower than destination and source is shorter
	   than or the same height as the destination.
	5. Source is narrower than destination but source is taller
	   than destination.
	6. Source is wider than destination but source is shorter than
	   or the same height as the destination.
	7. Source is wider than destination and source is taller than
	   destination.

(There is little advantage in optimizing cases where the widths are
different but the heights are the same.)

Bresenham's algorithm extrapolates bits if the source is smaller and
OR's together bits if the source is bigger than the destination.  The
algorithm is applied independently in the height and width.

For a source height of N, for example, the pixels are numbered 0 to N-1,
which means the Bresenham delta value is N-1 rather than N.  This is
reflected in the initial error terms and error increments in the following
code.

*/

/*
 * Calculate the number of bytes in the source and destination and their
 *  widths in bytes
 */
    src_wbyte = ( src_width + 7 ) / 8;
    src_size = src_height * src_wbyte;
    dest_wbyte = ( dest_buffer_width + 7 ) / 8;
    dest_size = dest_height * dest_wbyte;
/*
 * Zero out the destination, so we only have to write 1 bits.
 */
    memset( dest, 0, dest_size );
/*
 * Split up into the seven cases
 */
    if ( src_width == dest_width && src_height == dest_height )
	 memcpy( dest, src, dest_size );
				/* same size so just copy */
    else if ( src_width == dest_width && src_height < dest_height )
	{			/* same width, source is shorter */
	height_error = 2 * src_height - dest_height - 1;
	src_row = src;
	for ( dest_row = dest; dest_row < dest + dest_size;
	  dest_row += dest_wbyte )
	    {
	    memcpy( dest_row, src_row, dest_wbyte );
				/* copy a scan line */
	    if ( height_error > 0 )
		{		/* error big enough to increment source */
		src_row += src_wbyte;
		height_error += 2 * ( src_height - dest_height );
		}
	    else		/* error still small so don't incr source */
		height_error += 2 * ( src_height - 1 );
	    }
	}
    else if ( src_width == dest_width && src_height > dest_height )
	{			/* same width but source taller */
	height_error = 2 * dest_height - src_height - 1;
	dest_row = dest;
	for ( src_row = src; src_row < src + src_size; src_row += src_wbyte )
	    {
	    memcpy( dest_row, src_row, dest_wbyte );
	    if ( height_error > 0 )
		{		/* error term too big so bump dest */
		dest_row += dest_wbyte;
		height_error += 2 * ( dest_height - src_height );
		}
	    else
		height_error += 2 * dest_height - 2;
	    }
	}
    else if ( src_width < dest_width && src_height <= dest_height )
	{			/* source narrower and shorter */
	height_error = 2 * src_height - dest_height - 1;
	src_row = src;
	for ( dest_row = dest; dest_row < dest + dest_size;
	  dest_row += dest_wbyte )
	    {
	    width_error = 2 * src_width - dest_width - 1;
	    src_byte = src_row;
	    src_mask = 1;
	    for ( dest_x = 0, dest_byte = dest_row; dest_x < dest_width;
	      dest_byte++ )
		{
		for ( dest_mask = 1; dest_mask <= 0x80 && dest_x < dest_width;
		  dest_mask <<= 1, dest_x++ )
		    {
		    if ( *src_byte & src_mask )
			*dest_byte |= dest_mask;
		    if ( width_error > 0 )
			{
			if ( src_mask == 0x80 )
			    {
			    src_mask = 1;
			    src_byte++;
			    }
			else
			    src_mask <<= 1;
			width_error += 2 * ( src_width - dest_width );
			}
		    else
			width_error += 2 * src_width - 2;
		    }
		}
	    if ( height_error > 0 )
		{
		src_row += src_wbyte;
		height_error += 2 * ( src_height - dest_height );
		}
	    else
		height_error += 2 * src_height - 2;
	    }
	}
    else if ( src_width < dest_width && src_height > dest_height )
	{	/* source narrower and taller than destination */
	height_error = 2 * dest_height - src_height - 1;
	dest_row = dest;
	for ( src_row = src; src_row < src + src_size; src_row += src_wbyte )
	    {
	    width_error = 2 * src_width - dest_width - 1;
	    src_byte = src_row;
	    src_mask = 1;
	    for ( dest_x = 0, dest_byte = dest_row; dest_x < dest_width;
	      dest_byte++ )
		{
		for ( dest_mask = 1; dest_mask <= 0x80 && dest_x < dest_width;
		  dest_mask <<= 1, dest_x++ )
		    {
		    if ( *src_byte & src_mask )
			*dest_byte |= dest_mask;
		    if ( width_error > 0 )
			{
			if ( src_mask == 0x80 )
			    {
			    src_mask = 1;
			    src_byte++;
			    }
			else
			    src_mask <<= 1;
			width_error += 2 * ( src_width - dest_width );
			}
		    else
			width_error += 2 * src_width - 2;
		    }
		}
	    if ( height_error > 0 )
		{
		dest_row += dest_wbyte;
		height_error += 2 * ( dest_height - src_height );
		}
	    else
		height_error += 2 * dest_height - 2;
	    }
	}
    else if ( src_width > dest_width && src_height <= dest_height )
	{			/* source is wider and shorter than dest */
	height_error = 2 * src_height - dest_height - 1;
	src_row = src;
	for ( dest_row = dest; dest_row < dest + dest_size;
	  dest_row += dest_wbyte )
	    {
	    width_error = 2 * dest_width - src_width - 1;
	    dest_byte = dest_row;
	    dest_mask = 1;
	    for ( src_x = 0, src_byte = src_row; src_x < src_width; src_byte++ )
		{
		for ( src_mask = 1; src_mask <= 0x80 && src_x < src_width;
		  src_mask <<= 1, src_x++ )
		    {
		    if ( *src_byte & src_mask )
			*dest_byte |= dest_mask;
		    if ( width_error > 0 )
			{
			if ( dest_mask == 0x80 )
			    {
			    dest_mask = 1;
			    dest_byte++;
			    }
			else
			    dest_mask <<= 1;
			width_error += 2 * ( dest_width - src_width );
			}
		    else
			width_error += 2 * dest_width - 2;
		    }
		}
	    if ( height_error > 0 )
		{
		src_row += src_wbyte;
		height_error += 2 * ( src_height - dest_height);
		}
	    else
		height_error += 2 * src_height - 2;
	    }
	}
    else
	{		/* source wider and taller than destination */
	height_error = 2 * dest_height - src_height - 1;
	dest_row = dest;
	for ( src_row = src; src_row < src + src_size; src_row += src_wbyte )
	    {
	    width_error = 2 * dest_width - src_width - 1;
	    dest_byte = dest_row;
	    dest_mask = 1;
	    for ( src_x = 0, src_byte = src_row; src_x < src_width;
	      src_byte++ )
		{
		for ( src_mask = 1; src_mask <= 0x80 && src_x < src_width;
		  src_mask <<= 1, src_x++ )
		    {
		    if ( *src_byte & src_mask )
			*dest_byte |= dest_mask;
		    if ( width_error > 0 )
			{
			if ( dest_mask == 0x80 )
			    {
			    dest_mask = 1;
			    dest_byte++;
			    }
			else
			    dest_mask <<= 1;
			width_error += 2 * ( dest_width - src_width );
			}
		    else
			width_error += 2 * dest_width - 2;
		    }
		}
	    if ( height_error > 0 )
		{
		dest_row += dest_wbyte;
		height_error += 2 * ( dest_height - src_height );
		}
	    else
		height_error += 2 * dest_height - 2;
	    }
	}
}

/* reverse_character - mirror image a character along the vertical axis */

reverse_character( src, width, height, dest )
    char *src;		/* address of source bitmap */
    int width,		/* width of source & destination bitmaps in pixels */
	height;		/* height of source & destination bitmaps in pixels */
    char *dest;		/* address of destination bitmap */
{
    int size,		/* size of source & destination bitmaps in bytes */
	wbyte;		/* size of a scan line in bytes */
    char *src_row,	/* start of current row in source */
	*src_byte,	/* address of current byte in source */
	*dest_row,	/* start of current row in destination */
	*dest_byte;	/* address of current byte in destination */
    int src_mask,	/* mask with current bit in src_byte */
	src_x,		/* current x pixel in source */
	dest_mask;	/* mask with current bit in dest_byte */

    wbyte = ( width + 7 ) / 8;
			/* calculate width in byte */
    size = height * wbyte;
			/* calculate the size of each bitmap */

    memset( dest, 0, size );
			/* zero out the destination so we only have to write
			   the 1 bits */
    for ( src_row = src, dest_row = dest; src_row < src + size;
      src_row += wbyte, dest_row += wbyte )
	{		/* for each row in the source and destination */
	dest_byte = dest_row + wbyte - 1;
	dest_mask = ( 1 << ( ( width - 1 ) % 8 ) );
			/* point to the last bit in the destination */
	for ( src_x = 0, src_byte = src_row; src_x < width; src_byte++ )
	    {		/* for each source byte in the row */
	    for ( src_mask = 1; src_mask <= 0x80 && src_x < width;
	      src_mask <<= 1, src_x++ )
		{	/* for each bit within each source byte */
		if ( *src_byte & src_mask )
		    *dest_byte |= dest_mask;
			/* copy a pixel from source to destination */
		if ( dest_mask == 1 )
		    {	/* reached the end of a destination byte */
		    dest_mask = 0x80;
		    dest_byte--;
		    }
		else
		    dest_mask >>= 1;
			/* move one pixel left in the destination */
		}
	    }
	}
}

/* clip_character - clip a character to its display size */

clip_character( src, width, height, clip_width, clip_height )
    char *src;		/* address of source & destination bitmaps */
    int width,		/* width of source & destination bitmaps in pixels */
	height;		/* height of source & destination bitmaps in pixels */
    int clip_width,	/* width to clip to (display size x) */
	clip_height;	/* height to clip to (display size y) */
{
    int size,		/* size of bitmap in bytes */
	wbyte;		/* size of a scan line in bytes */
    char *row,		/* start of current row */
	*byte;		/* address of current byte */
    int mask;		/* mask of bits to clear in byte */

    wbyte = ( width + 7 ) / 8;
			/* calculate width in bytes */
    size = height * wbyte;
			/* calculate bitmap size in bytes */

    for ( row = src; row < src + size; row += wbyte )
	{		/* for each row */
	if ( row < src + clip_height * wbyte )
	    {  /* row not completely clipped, check for width clipping */
	    byte = row + clip_width / 8;
	    if ( clip_width % 8 != 0 )
		{ 	/* zero out the remainder of the first clipped byte */
		mask = ( 1 << ( clip_width % 8 ) ) - 1;
		*byte++ &= mask;
		}
	    }
	else
	    byte = row;		/* clip the entire row */
	while ( byte < row + wbyte )
	    {  /* zero out the rest of the row, byte by byte */
	    *byte++ = 0;
	    }
	}
}

/* rotate_character - rotate a character bitmap

Note: this algorithm works with multiples of 45 degrees and makes diagonal
characters bigger than vertical or horizotal characters.  This is compatible
with ReGIS devices such as the VT240 and VT330.

Since it requires a certain amount of calculation to determine the size of
the destination bitmap, the routine allocates the destination bitmap.  The
caller is expected to free the bitmap by calling XtFree().

*/

rotate_character( src, src_width, src_height, src_x_offset, src_y_offset,
  angle, p_dest, p_dest_width, p_dest_height, p_dest_x_offset,
  p_dest_y_offset )
    char *src;		/* address of source bitmap */
    int src_width,	/* width of source bitmap in pixels */
	src_height,	/* height of source bitmap in pixels */
	src_x_offset,	/* x offset of character origin in source bitmap */
	src_y_offset,	/* y offset of character origin in dest bitmap */
	angle;		/* number of degrees to rotate by (<0 => clockwise) */
    char **p_dest;	/* OUTPUT: address of destination bitmap */
    int *p_dest_width,	/* OUTPUT: width of destination bitmap in pixels */
	*p_dest_height,	/* OUTPUT: height of destination bitmap in pixels */
	*p_dest_x_offset,	/* OUTPUT: x offset of origin in dest */
	*p_dest_y_offset;	/* OUTPUT: y offset of origin in dest */
{
    char *dest,		/* address of destination bitmap */
	*src_row,	/* address of start of source row */
	*src_byte,	/* address of current byte in source row (inner loop) */
	*dest_row_byte,	/* address of start of destination row */
	*dest_byte,	/* address of current byte in dest row (inner loop) */
	*extra_byte;	/* addres of byte to fill with extra pixel */

    int dest_width,	/* width of destination bitmap in pixels */
	dest_height,	/* height of destination bitmap in pixels */
	src_wbyte,	/* width of a source row in bytes */
	dest_wbyte,	/* width of a destination row in bytes */
	dest_size,	/* size of destination bitmap in bytes */
	big,		/* width & height of diagonal character */
	src_x,		/* source x position for inner loop */
	src_y,		/* source y position for inner/outer loop */
	src_mask,	/* mask for current bit in source (inner loop) */
	dest_row_x,	/* destination x position for outer loop */
	dest_row_y,	/* destination y position for outer loop */
	dest_row_mask,	/* mask for current bit in destination (outer loop) */
	dest_x,		/* destination x position for inner loop */
	dest_y,		/* destination y position for inner loop */
	dest_mask,	/* mask for current bit in destination (inner loop) */
	width_dx,	/* x increment to advance destination along width */
	width_dy,	/* y increment to advance destination along width */
	height_dx,	/* x increment to advance destination along height */
	height_dy,	/* y increment to advance destination along height */
	edge_x,		/* destination x at edge not to be filled after */
	edge_y,		/* destination y at edge not to be filled after */
	extra_mask;	/* mask for bit to fill in with extra pixel */

    big = src_width + src_height;

    angle %= 360;			/* convert to -359 .. 359 */
    if ( angle < 0 )
	angle += 360;			/* convert to 0 .. 359 */

    switch( angle / 45 )
	{
	case 0:			/* 0 degree rotation */
	    *p_dest = src;
	    *p_dest_width = src_width;
	    *p_dest_height = src_height;
	    *p_dest_x_offset = src_x_offset;
	    *p_dest_y_offset = src_y_offset;
	    return;
	case 1:			/* 45 degree rotation */
	    dest_width = dest_height = big;
	    dest_row_x = 0;
	    dest_row_y = src_width - 1;
	    width_dx = 1;
	    width_dy = -1;
	    height_dx = 1;
	    height_dy = 1;
	    edge_x = src_width - 1;
	    edge_y = src_height - 1;
	    break;
	case 2:			/* 90 degree rotation */
	    dest_width = src_height;
	    dest_height = src_width;
	    dest_row_x = 0;
	    dest_row_y = src_width - 1;
	    width_dx = 0;
	    width_dy = -1;
	    height_dx = 1;
	    height_dy = 0;
	    break;
	case 3:			/* 135 degree rotation */
	    dest_width = dest_height = big;
	    dest_row_x = src_width - 1;
	    dest_row_y = big - 1;
	    width_dx = -1;
	    width_dy = -1;
	    height_dx = 1;
	    height_dy = -1;
	    edge_x = src_width - 1;
	    edge_y = 0;
	    break;
	case 4:			/* 180 degree rotation */
	    dest_width = src_width;
	    dest_height = src_height;
	    dest_row_x = src_width - 1;
	    dest_row_y = src_height - 1;
	    width_dx = -1;
	    width_dy = 0;
	    height_dx = 0;
	    height_dy = -1;
	    break;
	case 5:			/* 225 degree rotation */
	    dest_width = dest_height = big;
	    dest_row_x = big - 1;
	    dest_row_y = src_height - 1;
	    width_dx = -1;
	    width_dy = 1;
	    height_dx = -1;
	    height_dy = -1;
	    edge_x = src_width - 1;
	    edge_y = src_height - 1;
	    break;
	case 6:			/* 270 degree rotation */
	    dest_width = src_height;
	    dest_height = src_width;
	    dest_row_x = src_height - 1;
	    dest_row_y = 0;
	    width_dx = 0;
	    width_dy = 1;
	    height_dx = -1;
	    height_dy = 0;
	    break;
	case 7:			/* 315 degree rotation */
	    dest_width = dest_height = big;
	    dest_row_x = src_height - 1;
	    dest_row_y = 0;
	    width_dx = 1;
	    width_dy = 1;
	    height_dx = -1;
	    height_dy = 1;
	    edge_x = src_width - 1;
	    edge_y = 0;
	    break;
	}
    src_wbyte = ( src_width + 7 ) / 8;
    dest_wbyte = ( dest_width + 7 ) / 8;
    dest_size = dest_wbyte * dest_height;
    dest = XtCalloc( dest_size, 1 );

    *p_dest = dest;
    *p_dest_width = dest_width;
    *p_dest_height = dest_height;
    *p_dest_x_offset = dest_row_x + src_x_offset * width_dx + src_y_offset *
      height_dx;
    *p_dest_y_offset = dest_row_y + src_x_offset * width_dy + src_y_offset *
      height_dy;

    dest_row_byte = dest + dest_row_y * dest_wbyte + dest_row_x / 8;
			/* calculate address of character origin */
    dest_row_mask = ( 1 << ( dest_row_x % 8 ) );
			/* calculate bit mask for character origin */

    for ( src_y = 0, src_row = src; src_y < src_height; src_y++,
      src_row += src_wbyte )
	{		/* for each row in the source */
	dest_x = dest_row_x;
	dest_y = dest_row_y;
	dest_byte = dest_row_byte;
	dest_mask = dest_row_mask;
			/* point to start of row in destination */

	for ( src_x = 0, src_byte = src_row; src_x < src_width; src_byte++ )
	    {		/* for each byte in source row */
	    for ( src_mask = 1; src_mask <= 0x80 && src_x < src_width;
	      src_mask <<= 1, src_x++ )
		{	/* for each bit in source byte */
		if ( *src_byte & src_mask )
		    {
		    *dest_byte |= dest_mask;
			/* copy a bit from source to destination */
		    if ( dest_width == big && src_x != edge_x
		      && src_y != edge_y )
			{  /* expanded character, fill in extra pixel */
			extra_byte = dest_byte;
			extra_mask = SHIFT_LEFT (dest_mask, width_dx);
			if ( extra_mask > 0x80 )
			    {	/* shifted into next byte to the right */
			    extra_mask = 1;
			    extra_byte++;
			    }
			else if ( extra_mask == 0 )
			    {	/* shifted into next byte to the left */
			    extra_mask = 0x80;
			    extra_byte--;
			    }
			*extra_byte |= extra_mask;
			}
		    }
		/* advance along width */
		dest_x += width_dx;
		dest_y += width_dy;
		SHIFT_LEFT_IN_PLACE (dest_mask, width_dx);
		if ( dest_mask > 0x80 )
		    {	/* advanced into next byte to the right */
		    dest_mask = 1;
		    dest_byte++;
		    }
		else if ( dest_mask == 0 )
		    {	/* advanced into next byte to the left */
		    dest_mask = 0x80;
		    dest_byte--;
		    }
		dest_byte += width_dy * dest_wbyte;
		}
	    }
	/* advance along height */
	dest_row_x += height_dx;
	dest_row_y += height_dy;
	SHIFT_LEFT_IN_PLACE (dest_row_mask, height_dx);
	if ( dest_row_mask > 0x80 )
	    {		/* advanced into next byte to the right */
	    dest_row_mask = 1;
	    dest_row_byte++;
	    }
	else if ( dest_row_mask == 0 )
	    {		/* advanced into next byte to the left */
	    dest_row_mask = 0x80;
	    dest_row_byte--;
	    }
	dest_row_byte += height_dy * dest_wbyte;
	}
}

/* oblique_character - italicize a character bitmap

Note: since the size of the destination bitmap is a function of the size of
of the source bitmap and the angle of obliqueness, this routine allocates the
destination bitmap and returns its address and size.  The caller is expected
to free the bitmap by calling XtFree.

This routine only supports four obliqueness angles (besides zero), with
a 2:1 and 1:1 ratio between vertical increment and horizontal increment.
This correponds to angles of +45, +26.6, -26.6 and -45 degrees.  The cut-off
points are compatible with ReGIS on the VT240 and VT340 terminals.

For simplicity, the character origin is assumed to be at the upper left
corner of the bitmap.  If a character is both rotated and obliqued, it should
be obliqued first, then rotated.

*/

oblique_character( src, src_width, src_height, angle, p_dest, p_dest_width,
  p_dest_height, p_dest_x_offset, p_dest_y_offset )
    char *src;			/* address of source bitmap */
    int src_width,		/* width of source bitmap in pixels */
	src_height,		/* height of source bitmap in pixels */
	angle;			/* oblique angle in degrees (<0 => clockwise) */
    char **p_dest;		/* OUTPUT: address of destination bitmap */
    int *p_dest_width,		/* OUTPUT: width of destination in pixels */
	*p_dest_height,		/* OUTPUT: height of destination in pixels */
	*p_dest_x_offset,	/* OUTPUT: x offset of origin in destination */
	*p_dest_y_offset;	/* OUTPUT: y offset of origin in destination */
{
    char *dest,			/* address of destination bitmap */
	*src_row,		/* address of current row in source */
	*src_byte,		/* address of current byte in source */
	*dest_row,		/* address of current row in destination */
	*dest_byte;		/* address of current byte in destination */
    int dest_width,		/* width of destination bitmap in pixels */
	src_wbyte,		/* width of a source row in bytes */
	src_x,			/* current pixel offset in source row */
	src_y,			/* current row number in source */
	src_mask,		/* mask of current bit within source byte */
	dest_wbyte,		/* width of a destination row in bytes */
	dest_size,		/* size of destination bitmap in bytes */
	dest_row_x,		/* x offset of start of current row in dest */
	dest_row_mask,		/* mask of bit at start of current row */
	dest_mask,		/* mask of current bit in destination byte */
	dx,			/* x increment to next row (+1 or -1) */
	slope,			/* twice the ratio xinc:yinc (1 or 2) */
	even_odd,		/* sum of slopes over rows seen so far */
	h2;			/* half the source height, rounded up */

    h2 = ( src_height + 1 ) / 2;
    if ( angle > 30 )
	{			/* +45 degree obliqueness */
	slope = 2;		/* 1:1 slope */
	dx = 1;			/* to the left */
	dest_row_x = 0;
	dest_width = src_width + src_height;
	}
    else if ( angle > 0 )
	{			/* +26.6 degree obliqueness */
	slope = 1;		/* 2:1 slope */
	dx = 1;			/* to the left */
	dest_row_x = 0;
	dest_width = src_width + h2;
	}
    else if ( angle < -30 )
	{			/* -45 degree obliqueness */
	slope = 2;		/* 1:1 slope */
	dx = -1;		/* to the right */
	dest_row_x = src_height;
	dest_width = src_width + src_height;
	}
    else if ( angle < 0 )
	{			/* -26.6 degree obliqueness */
	slope = 1;		/* 2:1 slope */
	dx = -1;		/* to the right */
	dest_row_x = h2;
	dest_width = src_width + h2;
	}
    else
	{			/* 0 degree obliqueness /
	*p_dest = src;		/* just fill in the blanks and return */
	*p_dest_width = src_width;
	*p_dest_height = src_height;
	*p_dest_x_offset = 0;
	*p_dest_y_offset = 0;
	return;
	}
    src_wbyte = ( src_width + 7 ) / 8;
    dest_wbyte = ( dest_width + 7 ) / 8;
    dest_size = dest_wbyte * src_height;
    dest = XtCalloc( dest_size, 1 );

    *p_dest = dest;
    *p_dest_width = dest_width;
    *p_dest_height = src_height;
    *p_dest_x_offset = dest_row_x;
    *p_dest_y_offset = 0;

    even_odd = 1;		/* don't increment initially unless 1:1 */
    dest_row = dest + dest_row_x / 8;
    dest_row_mask = ( 1 << ( dest_row_x % 8 ) );

    for ( src_y = 0, src_row = src; src_y < src_height; src_y++,
      src_row += src_wbyte )
	{			/* for each row */
	dest_byte = dest_row;
	dest_mask = dest_row_mask;
				/* point to start of destination row */
	for ( src_x = 0, src_byte = src_row; src_x < src_width; src_byte++ )
	    {			/* for each byte in source row */
	    for ( src_mask = 1; src_mask <= 0x80 && src_x < src_width;
	      src_mask <<= 1, src_x++ )
		{		/* for each bit in source byte */

		/* copy a pixel from the source to the destination */
		if ( *src_byte & src_mask )
		    *dest_byte |= dest_mask;

		/* advance to next pixel in current row */
		if ( dest_mask == 0x80 )
		    {
		    dest_mask = 1;
		    dest_byte++;
		    }
		else
		    dest_mask <<= 1;
		}
	    }
	even_odd += slope;
	if ( even_odd & 1 )
	    {  /* time to start next row at a different place */
	    SHIFT_LEFT_IN_PLACE (dest_row_mask, dx);
	    if ( dest_row_mask > 0x80 )
		{	/* advanced to next byte to the right */
		dest_row_mask = 1;
		dest_row++;
		}
	    else if ( dest_row_mask == 0 )
		{	/* advanced to next byte to the left */
		dest_row_mask = 0x80;
		dest_row--;
		}
	  }
	dest_row += dest_wbyte;	/* advance to next row in destination */
	}
}

/* draw_character_background - draw the background of a character

Implicit inputs (in context block):
	gid_direction			rotation angle in degrees
	gid_oblique			italic angle in degrees
	writing_mode			current GIDIS writing mode
	foreground_color		X foreground pixel value
	background_color		X background pixel value
	display				X display context
	text_background_gc		graphics context for text backgrounds
	plane_mask			current correct plane mask
	window				X window context

Implicit input/outputs (in context block):
	text_background_gc_foreground	last foreground color set in GC
	text_background_gc_plame_mask	last plane mask set in GC

Note: This routine must emulate some of the calculations in rotate_character()
and oblique_character() to determine the four endpoints of the rectangle
or parallelogram to fill with the character background color.  It's primary
effect is to draw the character background in the window; it may also change
graphics state in the character background GC.

*/

draw_character_background( x, y, width, height )
    int x, y;			/* origin of character in pixel space */
    int width,			/* width of background in pixels */
	height;			/* height of background in pixels */
{
    struct regis_cntx *rs = RSTRUCT;
				/* ReGIS context block */
    DECtermWidget w = rs->widget;

    XPoint points[4];		/* endpoints of rectangle/parallelogram */
    int	width_dx,		/* x offset of cell along character width */
	width_dy,		/* y offset of cell along character width */
	height_dx,		/* x offset of cell along character height */
	height_dy,		/* y offset of cell along character height */
	ital_dx,		/* x offset to italicize cell */
	ital_dy,		/* y offset to italicize cell */
	rotation,		/* normalized rotation angle */
	foreground;		/* color in which to draw background */

/* account for cell rotation */

    rotation = rs->gid_direction % 360;
    if ( rotation < 0 )
	rotation += 360;
    switch( rotation / 45 )
	{
	case 0:		/* 0 degrees */
	    width_dx = width;
	    width_dy = 0;
	    height_dx = 0;
	    height_dy = height;
	    break;
	case 1:		/* 45 degrees */
	    width_dx = width;
	    width_dy = -width;
	    height_dx = height;
	    height_dy = height;
	    break;
	case 2:		/* 90 degrees */
	    width_dx = 0;
	    width_dy = -width;
	    height_dx = height;
	    height_dy = 0;
	    break;
	case 3:		/* 135 degrees */
	    width_dx = -width;
	    width_dy = -width;
	    height_dx = height;
	    height_dy = -height;
	    break;
	case 4:		/* 180 degrees */
	    width_dx = -width;
	    width_dy = 0;
	    height_dx = 0;
	    height_dy = -height;
	    break;
	case 5:		/* 225 degrees */
	    width_dx = -width;
	    width_dy = width;
	    height_dx = -height;
	    height_dy = -height;
	    break;
	case 6:		/* 270 degrees */
	    width_dx = 0;
	    width_dy = width;
	    height_dx = -width;
	    height_dy = 0;
	    break;
	case 7:		/* 315 degrees */
	    width_dx = width;
	    width_dy = width;
	    height_dx = -height;
	    height_dy = height;
	    break;
	}

/* account for cell obliquing */

    if ( rs->gid_oblique > 30 )
	{
	ital_dx = width_dx;
	ital_dy = width_dy;
	}
    else if ( rs->gid_oblique > 0 )
	{
	ital_dx = ( width_dx + ( ( width_dx < 0 ) ? -1 : 1 ) ) / 2;
	ital_dy = ( width_dy + ( ( width_dy < 0 ) ? -1 : 1 ) ) / 2;
	}
    else if ( rs->gid_oblique < -30 )
	{
	ital_dx = - width_dx;
	ital_dy = - width_dy;
	}
    else if ( rs->gid_oblique < 0 )
	{
	ital_dx = - ( width_dx + ( ( width_dx < 0 ) ? -1 : 1 ) ) / 2;
	ital_dy = - ( width_dy + ( ( width_dy < 0 ) ? -1 : 1 ) ) / 2;
	}
    else
	{
	ital_dx = 0;
	ital_dy = 0;
	}

/* compute the endpoints of the rectangle or parallelogram */

    points[0].x = x;
    points[0].y = y;
    points[1].x = width_dx - 1;
    points[1].y = width_dy - 1;
    points[2].x = height_dx + ital_dx - 1;
    points[2].y = height_dy + ital_dy - 1;
    points[3].x = - ( width_dx - 1 );
    points[3].y = - ( width_dy - 1 );

/* set the color for the character background */

    if ( rs->writing_mode == G47_MODE_ERASE_NEGATE )
	foreground = fetch_color( rs->foreground_index );
    else
	foreground = fetch_color( rs->background_index );
    if ( foreground != rs->text_background_gc_foreground )
	{	/* need to change the character background color */
	XSetForeground( rs->display, rs->text_background_gc, foreground );
	if ( w->common.backing_store_active )
	    XSetForeground( rs->display, rs->bs_text_background_gc,
	      foreground );
	rs->text_background_gc_foreground = foreground;
	}

/* set the plane mask */

    if ( ( w->common.graphics_mode == ALLOCATED_PLANES
	|| w->common.graphics_mode == ALLOCATED_COLORMAP )
      && rs->text_background_gc_plane_mask != rs->plane_mask )
	{	/* need to change the plane mask */
	XSetPlaneMask( rs->display, rs->text_background_gc, rs->plane_mask );
	if ( w->common.backing_store_active )
	    XSetPlaneMask( rs->display, rs->bs_text_background_gc,
	      rs->plane_mask );
	rs->text_background_gc_plane_mask = rs->plane_mask;
	}

/* draw the character background */

    XFillPolygon( rs->display, rs->window, rs->text_background_gc,
      points, 4, Convex, CoordModePrevious );
    if ( w->common.backing_store_active )
	{
	points[0].x -= rs->x_offset;
	points[0].y -= rs->y_offset;
	XFillPolygon( rs->display, rs->bs_pixmap, rs->bs_text_background_gc,
	  points, 4, Convex, CoordModePrevious );
	}
}

/* negate_character - reverse the colors in a bitmap character */

negate_character( src, cell_width, cell_height, dest )
    char *src;		/* address of source bitmap */
    int cell_width,	/* width of source/destination in pixels */
	cell_height;	/* height of source/destination in pixels */
    char *dest;		/* address of destination bitmap */
{
    int cell_size,	/* size of source/destination in bytes */
	i;		/* loop index for source/destination bytes */

    cell_size = ( cell_width + 7 ) / 8 * cell_height;
    for ( i = 0; i < cell_size; i++ )
	dest[i] = ~src[i];
			/* negate each pixel, a byte at a time */
}

/* regis_create_display - allocate resources associated with a ReGIS window

Implicit inputs (in ReGIS context block):
	display		X display context
	cmap		X color map context for window
	base_color	color returned from XAllocColorCells
	allocated_plane_mask
			mask of planes returned from XAllocColorCells
	gc		graphics context for drawing vectors
	text_gc		graphics context for drawing characters
	text_background_gc
			graphics context for drawing character backgrounds
	shade_gc	graphics context for filling polygons
	load_font_gc	graphics context for reading alphabet 0 from server
	tpx_pixmap	text pixmap (characters drawn as stippled patterns)
	tpx_gc		graphics context for writing to the text pixmap
	spx_pixmap	shading pixmap (stipple pattern for polyon fills)
	spx_gc		graphics context for writing to the shading pixmap
	font_glyphs	stored bitmaps for all text alphabets

*/

regis_create_display( w )
    DECtermWidget w;
{
    struct regis_cntx *rs = ( struct regis_cntx * ) w->regis;
    int alph, ch;
    XGCValues values;

    rs->gc = XCreateGC( rs->display, rs->window, 0, 0 );
    values.font = w->output.font_list[ FONT_NORMAL ]->fid;
    rs->text_gc = XCreateGC( rs->display, rs->window, GCFont, &values );
    rs->text_background_gc = XCreateGC( rs->display, rs->window, 0, 0 );
    rs->shade_gc = XCreateGC( rs->display, rs->window, 0, 0 );

    values.foreground = 0;
    values.background = -1;
    values.function = GXand;
    rs->text_clear_gc = XCreateGC( rs->display, rs->window, GCForeground |
      GCBackground | GCFunction, &values );

    values.foreground = 0;
    values.plane_mask = -1;
    rs->screen_erase_gc = XCreateGC( rs->display, rs->window,
      GCForeground | GCPlaneMask, &values );
    rs->screen_erase_gc_foreground = 0;

    rs->load_font_pixmap = NULL;
    rs->tpx_pixmap = NULL;
    rs->spx_pixmap = NULL;

/*
 * This code is from G3_INITIALIZE and is duplicated to insure that
 * the font_glyphs array is initialized.
 */

    for ( alph = 0; alph < TOTAL3_NUMBER_OF_ALPHABETS; alph++ )
	for ( ch = 0; ch < 256; ch++ )
	    if ( alph == 0 )
#ifdef CANNED_TEXT_FONT
		rs->font_glyphs[ alph ][ ch ] = alph0[ ch ];
#else
		rs->font_glyphs[ alph ][ ch ] = NULL;
#endif
	    else if ( rs->kanji_regis )
		rs->font_glyphs[ alph ][ ch ] = solid_wide;
	    else
		rs->font_glyphs[ alph ][ ch ] = solid;

    w->common.graphics_visible = FALSE;
}

/* regis_destroy_display - free the resources associated with a ReGIS window

Implicit inputs (in ReGIS context block):
	display		X display context
	cmap		X color map context for window
	base_color	color returned from XAllocColorCells
	allocated_plane_mask
			mask of planes returned from XAllocColorCells
	gc		graphics context for drawing vectors
	text_gc		graphics context for drawing characters
	text_background_gc
			graphics context for drawing character backgrounds
	shade_gc	graphics context for filling polygons
	load_font_gc	graphics context for reading alphabet 0 from server
	tpx_pixmap	text pixmap (characters drawn as stippled patterns)
	tpx_gc		graphics context for writing to the text pixmap
	spx_pixmap	shading pixmap (stipple pattern for polyon fills)
	spx_gc		graphics context for writing to the shading pixmap
	font_glyphs	stored bitmaps for all text alphabets

*/

regis_destroy_display( w )
    DECtermWidget w;
{
    int alph,		/* loop index for alphabet numbers */
	ch;		/* loop index for character codes */
    char *glyph;	/* address of a character bitmap */
    struct regis_cntx *rs;
			/* ReGIS context block */

    rs = ( struct regis_cntx * ) w->regis;

    XFreeGC( rs->display, rs->gc );
    XFreeGC( rs->display, rs->text_gc );
    XFreeGC( rs->display, rs->text_clear_gc );
    XFreeGC( rs->display, rs->text_background_gc );
    XFreeGC( rs->display, rs->shade_gc );
    XFreeGC( rs->display, rs->screen_erase_gc );

    if ( rs->load_font_pixmap != NULL )
	{
	XFreeGC( rs->display, rs->load_font_gc );
	XFreePixmap( rs->display, rs->load_font_pixmap );
	}
    if ( rs->tpx_pixmap != NULL )
	{	/* text pixmap was created so destroy it */
	XFreeGC( rs->display, rs->tpx_gc );
	XFreePixmap( rs->display, rs->tpx_pixmap );
	}
    if ( rs->spx_pixmap != NULL )
	{	/* shading pixmap was created so destroy it */
	XFreeGC( rs->display, rs->spx_gc );
	XFreePixmap( rs->display, rs->spx_pixmap );
	}
    destroy_backing_store( w );
    w->common.graphics_visible = FALSE;

#ifdef CANNED_TEXT_FONT

    for ( alph = 1; alph < TOTAL3_NUMBER_OF_ALPHABETS; alph++ )

#else

    for ( alph = 0; alph < TOTAL3_NUMBER_OF_ALPHABETS; alph++ )

#endif

	for ( ch = 0; ch < 256; ch++ )
	    {	/* free all character glyphs that were allocated */
	    glyph = rs->font_glyphs[ alph ][ ch ];
	    if ( glyph != NULL && glyph != solid && glyph != solid_wide )
		{
		XtFree( glyph );
		rs->font_glyphs[ alph ][ ch ] = NULL;
		}
	    }
}

/* glyph_is_halftone - test whether a shading pattern is a checkerboard

Note: width must be 8 (so the glyph is an array of char).

*/

Boolean glyph_is_halftone( glyph, width, height )
    unsigned char *glyph;	/* shading pattern bitmap */
    int width,			/* bitmap width in pixels */
	height;			/* bitmap height in pixels */
{
    int even_odd,	/* 1 if 0xaa should be on odd-numbered lines */
	i;		/* loop index for lines in the bitmap */

    if ( glyph[0] == 0xaa )
	even_odd = 0;
    else
	even_odd = 1;
    for ( i = 0; i < height; i++ )
	if ( ( i & 1 ) == even_odd && glyph[i] != 0xaa
	  || ( i & 1 ) != even_odd && glyph[i] != 0x55 )
	    return False;
    return True;
}

/* request_current_position - return the current position in soft coordinates */

request_current_position( px, py )
    int *px,		/* OUTPUT: soft position, x */
	*py;		/* OUTPUT: soft position, y */
{
    struct regis_cntx *rs = RSTRUCT;

    *px = rs->x_soft_pos;
    *py = rs->y_soft_pos;
}

/* request_current_pointer - return the current pointer position */

request_current_pointer( px, py )
    int *px,		/* OUTPUT: soft pointer position, x */
	*py;		/* OUTPUT: soft pointer position, y */
{
    Window root,	/* root window ID */
	child;		/* window ID of child containing pointer */
    int root_x,		/* pointer x coordinate relative to root window */
	root_y,		/* pointer y coordinate relative to root window */
	win_x,		/* pointer x coordinate relative to root window */
	win_y;		/* pointer y coordinate relative to root window */
    unsigned int mask;	/* current state of buttons and modifier keys */
    struct regis_cntx *rs = RSTRUCT;

    XQueryPointer( rs->display, rs->window, &root, &child, &root_x, &root_y,
      &win_x, &win_y, &mask );
    *px = UNMAP_X_ABSOLUTE( win_x );
    *py = UNMAP_Y_ABSOLUTE( win_y );
}

/* regis_button_handler - called when a mouse button is activated */

static Boolean regis_button_handler( w, button, buttonstate, x, y, down )
    DECtermWidget w;		/* widget context block */
    unsigned int button,	/* button number */
	buttonstate,		/* mask of button and modifiers before event */
	x,			/* x pixel address of mouse */
	y;			/* y pixel address of mouse */
    Boolean down;		/* True for button down, False for button up */
{
    struct regis_cntx *rs;	/* ReGIS context block */
    int offset, loc_length, rx, ry;
    char *loc_data;
    Bool skip = FALSE, ansi_return = FALSE, eat_event = FALSE;

    if ( w->regis == NULL )
	{  /* ReGIS context block has not been created yet */
	skip = TRUE;
	}
    else
	rs = RSTRUCT = ( struct regis_cntx * ) w->regis;

    if ( skip || ! rs->initialized
      || rs->input_mode == 0 && rs->input_pending == False )
	{  /* not initialized, or one-shot input mode and not expecting a
	      mouse event */
	skip = TRUE;	/* so let the event be handled the default way */
	}

    if ( ! skip )
	{
	eat_event = TRUE;
	offset = down ? button : button + 5;
	loc_length = s_return_lkd( w, offset, &loc_data );
	if ( loc_length== 0 )
	    skip = TRUE;	/* no button definition */
	else
	    {
	    rx = UNMAP_X_ABSOLUTE( x );
	    ry = UNMAP_Y_ABSOLUTE( y );
	    if ( rx < 0 || rx >= rs->output_ids_width
	      || ry < 0 || ry >= rs->output_ids_height )
		{
		rx = ( -1 );
		ry = ( -1 );
		}
	    /* send the report */
	    if ( rs->input_pending )
		i_enable_input( w, NULL );
			/* else we'll get back our own report! */
	    i_send_unsolicited_report( w, loc_data, loc_length );
	    if ( rx < 0 || ry < 0 )
		i_send_unsolicited_report( w, "[]\r", 3 );
			/* error indicator */
	    else
		rprt2_position( rx, ry, FALSE );
			/* unsolicited position report */
	    if ( rs->input_pending )
		{	/* wake up if in one-shot mode */
		rs->input_pending = FALSE;
		rs->paused = FALSE;
		G122_DISABLE_INPUT();
			/* disable regis_button_handler */
		s_start_output( w, STOP_OUTPUT_OTHER );
		}
	    }
	}

    if ( rs->text_button_handler != NULL )
	ansi_return = (*rs->text_button_handler)(w,button,buttonstate,x,y,down);
		/* generate ANSI report too, if enabled */

    if ( eat_event || ansi_return )
	return TRUE;
    else
	return FALSE;
}

/* regis_input_handler - called when we receive input in one-shot mode */

static Boolean regis_input_handler( w, data, length )
    DECtermWidget w;
    unsigned char *data;
    int length;
{
    struct regis_cntx *rs;	/* ReGIS context block */
    int x, y, rx, ry, dummy;
    Window wdummy;

    if ( w->regis == NULL )
	{  /* ReGIS context block has not been created yet */
	return False;	/* shouldn't happen; handle the character normally */
	}
    else
	rs = RSTRUCT = ( struct regis_cntx * ) w->regis;

/* this is a hack, but: make sure is isn't an arrow key report */

    if ( length == 2 && data[0] == 155	/* CSI */
		&& ( 'A' <= data[1] && data[1] <= 'D' ) )
	{  /* it's a cursor key report */
	return True;		/* ignore it */
 	}

    i_enable_input( w, NULL );	/* disable ourselves as the input handler */
    i_send_unsolicited_report( w, data, length );
				/* send the keystroke or sequence */

/* report the current position in ReGIS coordinates */

    if (!XQueryPointer( XtDisplay(w), XtWindow(w), &wdummy, &wdummy,
		&dummy, &dummy, &x, &y, (unsigned int *)&dummy )
	     || x < X_MARGIN || x >= X_MARGIN + w->common.display_width
	     || y < Y_MARGIN || y >= Y_MARGIN + w->common.display_height )
	{	/* locator is off screen or outside display area */
	rx = ( -1 );	/* special error indicator */
	ry = ( -1 );
	}
    else
	{	/* in display area, so convert to coordinates */
	rx = UNMAP_X_ABSOLUTE( x );
	ry = UNMAP_Y_ABSOLUTE( y );
	if ( rx < 0 || rx >= rs->output_ids_width
	  || ry < 0 || ry >= rs->output_ids_height )
	    {	/* locator is outside addressable area */
	    rx = ( -1 );
	    ry = ( -1 );
	    }
	}
    if ( rx < 0 || ry < 0 )
	i_send_unsolicited_report( w, "[]\r", 3 );
		/* indicates bad position */
    else
	rprt2_position( rx, ry, FALSE );
		/* send unsolicited position report */

/* now wake up from hibernation */

    rs->paused = FALSE;
    rs->input_pending = FALSE;
    G122_DISABLE_INPUT();
		/* disable regis_button_handler */
    s_start_output( w, STOP_OUTPUT_OTHER );
    return True;			/* don't handle this input twice */
}

/* find_rgb_color - read an RGB color from the X color map */

find_rgb_color( w, pixel, red, green, blue, mono )
    DECtermData *w;		/* DECterm widget context */
    Pixel pixel;		/* DECterm pixel value */
    unsigned long *red, *green, *blue;
				/* OUTPUT: RGB color read from color map */
    unsigned long *mono;	/* OUTPUT: supply mono value as well */
{
    XColor color;		/* structure for returning color info */

    color.pixel = pixel;
    XQueryColor( XtDisplay(w), w->core.colormap, &color );
    *red = color.red;
    *green = color.green;
    *blue = color.blue;
    *mono = ( *green * 59 + *red * 30 + *blue * 11 ) / 100;
}

/* allocate_color_map - allocate a logical color map */

void allocate_color_map( w )
    DECtermWidget w;		/* DECterm widget context */
{
    int plane, color, num_colors;
    Pixel pixel;
    Colormap cmap;
    Window shell;
    int i;

    if ( w->common.color_map_allocated )
	return;

    w->common.color_map_allocated = TRUE;
    num_colors = ( 1 << w->common.bitPlanes );

/*
 * If we're dealing with read-write color cells (pseudo color, either
 * allocating planes out of the default colormap or else allocating our
 * own private colormap) we need to allocate the color cells, set up our
 * color map array to point to the pixel values, and then redraw the display
 * to reflect the changed pixel values for the text foreground and
 * background colors.
 */
    if ( w->common.graphics_mode == ALLOCATED_PLANES
      || w->common.graphics_mode == ALLOCATED_COLORMAP )
	{  /* read-write color cells */
	if ( XAllocColorCells( XtDisplay(w), w->core.colormap, False,
	      w->common.plane_masks, w->common.bitPlanes,
	      &w->common.base_color, 1 ) )
	    w->common.graphics_mode = ALLOCATED_PLANES;
	else
	    {  /* private colormap */
	    o_free_ansi_colors( w );	/* ANSI colors no longer valid */
	    w->common.graphics_mode = ALLOCATED_COLORMAP;
	    shell = XtWindow( w->core.parent->core.parent );
	    w->core.colormap = XCreateColormap( XtDisplay(w), shell,
	      w->common.visual, AllocAll );
	    XSetWindowColormap( XtDisplay(w), shell, w->core.colormap );
	    w->common.base_color = 0;
	    for ( plane = 0; plane < w->common.bitPlanes; plane++ )
		w->common.plane_masks[ plane ] = ( 1 << plane );
	    }
	w->common.allocated_plane_mask = 0;
	for ( plane = 0; plane < w->common.bitPlanes; plane++ )
	    {  /* OR in the bits for the planes we were given */
	    w->common.allocated_plane_mask |=
		    w->common.plane_masks[ plane ];
	    }
	o_set_cursor_plane_mask( w );
	for ( color = 0; color < num_colors; color++ )
	    {  /* figure out the pixel value for each color map entry */
	    pixel = w->common.base_color;
	    for ( plane = 0; plane < w->common.bitPlanes; plane++ )
		if ( color & ( 1 << plane ) )
		    pixel |= w->common.plane_masks[ plane ];
	    w->common.color_map[ color ].pixel = pixel;
	    }
	XStoreColors( XtDisplay(w), w->core.colormap, w->common.color_map,
		num_colors );
	set_background_color( w );
	set_foreground_color( w );
	o_repaint_display( w );
	}
/*
 * If we're using read-only color map entries (e.g. true color or single
 * plane monochrome), all we have to do is initialize a couple of things
 * like the cursor plane mask.
 */
    else
	{
	if ( w->common.graphics_mode == ALLOCATED_COLORS )
	    {  /* read-only colormap access */
	    for ( i = 0; i < num_colors; i++ )
		{
		w->common.pixel_allocated[i] = FALSE;
		w->common.pixel_valid[i] = FALSE;
	    	}
	    }
	o_set_cursor_plane_mask( w );
	}
}

/* destroy_color_map - deallocate a logical color map */

void destroy_color_map( w )
    DECtermWidget w;
{
    Window shell;
    Colormap default_cmap;

    o_free_ansi_colors( w );

    if ( ! w->common.color_map_allocated )
	return;

    if ( ! ONE_PLANE(w) )
	{
	if ( w->common.graphics_mode == ALLOCATED_COLORS )
	    regis_free_colors( w );
	else if ( w->common.graphics_mode == ALLOCATED_COLORMAP )
	    {
	    shell = XtWindow( w->core.parent->core.parent );
	    default_cmap = w->common.default_colormap;
	    XSetWindowColormap( XtDisplay(w), shell, default_cmap );
	    XFreeColormap( XtDisplay(w), w->core.colormap );
	    w->core.colormap = default_cmap;
	    }
	else
	    XFreeColors( XtDisplay(w), w->core.colormap, &w->common.base_color,
	      1, w->common.allocated_plane_mask );
	}

    w->common.color_map_allocated = FALSE;
}

/* regis_update_rectangle - update a rectangle from the backing store */

regis_update_rectangle( w, x, y, width, height, pixmap )
    DECtermWidget w;		/* DECterm widget context */
    int x, y, width, height;	/* rectangle to update */
    Pixmap pixmap;
{
    int adjusted_x, adjusted_y, adjusted_width, adjusted_height,
      adjustments_made;
    struct regis_cntx *rs;	/* ReGIS context */
    Drawable drawable;

    rs = ( struct regis_cntx * ) w->regis;

    drawable = (Drawable) ((pixmap == NULL) ? rs->window : pixmap);

    if ( rs->defer_count > 0 )
	regis_execute_deferred( w );

/* make sure the area to be updated is within the display area */

    if ( x < X_MARGIN )
	{
	width -= X_MARGIN - x;
	x = X_MARGIN;
	}
    if ( x + width > X_MARGIN + w->common.display_width )
	width = X_MARGIN + w->common.display_width - x;
    if ( y < Y_MARGIN )
	{
	height -= Y_MARGIN - y;
	y = Y_MARGIN;
	}
    if ( y + height > Y_MARGIN + w->common.display_height )
	height = Y_MARGIN + w->common.display_height - y;

/* now make sure there is an underlying backing store */

    adjusted_x = x;
    adjusted_y = y;
    adjusted_width = width;
    adjusted_height = height;
    adjustments_made = FALSE;

    if ( adjusted_x - rs->x_offset < 0 )
	{
	adjusted_width -= rs->x_offset - adjusted_x;
	adjusted_x = rs->x_offset;
	adjustments_made = TRUE;
	}
    if ( adjusted_x - rs->x_offset + adjusted_width > w->common.logical_width )
	{
	adjusted_width = w->common.logical_width - adjusted_x + rs->x_offset;
	adjustments_made = TRUE;
	}
    if ( adjusted_y - rs->y_offset < 0 )
	{
	adjusted_height -= rs->y_offset - adjusted_y;
	adjusted_y = rs->y_offset;
	adjustments_made = TRUE;
	}
    if ( adjusted_y - rs->y_offset + adjusted_height >
      w->common.logical_height )
	{
	adjusted_height = w->common.logical_height - adjusted_y + rs->y_offset;
	adjustments_made = TRUE;
	}
    /* can't apply XClearArea to a pixmap.  Have to use XFillRectangle()
     */

    if ( adjustments_made && width > 0 && height > 0 ) {
	if ( pixmap != NULL)  /* using reverse_gc is most suitable */
		XFillRectangle( rs->display, drawable, w->output.reverse_gc,
				x, y, width, height );
	else
	    XClearArea( rs->display, rs->window, x, y, width, height, FALSE );
    }

    if ( adjusted_width > 0 && adjusted_height > 0 )
	XCopyArea( rs->display, rs->bs_pixmap, drawable, w->sixel.bs_gc,
	  adjusted_x - rs->x_offset, adjusted_y - rs->y_offset,
	  adjusted_width, adjusted_height, adjusted_x, adjusted_y );
}

/* regis_erase_rectangle - erase a rectangle of the backing store */

regis_erase_rectangle( w, pixel, x, y, width, height )
    DECtermWidget w;		/* DECterm widget context */
    Pixel pixel;		/* color to erase to */
    int x, y, width, height;	/* rectangle to erase */
{
    int i;
    struct regis_cntx *rs;	/* ReGIS context */

    rs = ( struct regis_cntx * ) w->regis;
    if ( rs == NULL )
	return;
    if ( w->common.graphics_visible )
	{
	if ( x <= 0 && y <= 0 && width >= w->common.logical_width
	  && height >= w->common.logical_height )
	    {
	    destroy_backing_store( w );
	    w->common.graphics_visible = FALSE;
	    }
	else if ( w->common.backing_store_active )
	    {	/* add the erase operation to the defer queue */
	    /* first prune the queue by removing any preceding operations
	       that operate entirely within the affected rectangle */
	    for ( i = rs->defer_count - 1; i >= 0; i-- )
		{  /* for entry starting with the most recent one */
		if ( rs->defer_queue[i].p2 < x
		  || rs->defer_queue[i].p3 < y
		  || rs->defer_queue[i].p4 + rs->defer_queue[i].p2 > x + width
		  || rs->defer_queue[i].p5 + rs->defer_queue[i].p3 > y +
		  height )
		    break;
		}
	    rs->defer_count = i + 1;
	    regis_defer_operation( w, DEFER_ERASE, pixel, x, y, width, height );
	    }
	}
}

/* regis_resize_window - adjust the visible display area */

regis_resize_window( w )
    DECtermWidget w;
{
    struct regis_cntx *rs = (struct regis_cntx *) w->regis;

    if ( ! XtIsRealized(w) || rs == NULL || ! rs->initialized )
	return;
    RSTRUCT = rs;
    G29_SET_OUTPUT_VIEWPORT( X_MARGIN, Y_MARGIN,
	MAP_X_RELATIVE( rs->output_ids_width ),
	MAP_Y_RELATIVE( rs->output_ids_height ) );
}

/* regis_resize_terminal - resize the underlying logical display */

regis_resize_terminal( w )
    DECtermWidget w;
{
    Drawable new_pixmap;
    GC new_gc;
    struct regis_cntx *rs;
    int width, height;

    rs = ( struct regis_cntx * ) w->regis;

    if ( rs == NULL || ! w->common.backing_store_active )
	return;

    if ( rs->defer_count > 0 )
	regis_execute_deferred( w );
    new_pixmap = create_pixmap( rs->display, rs->window,
      w->common.logical_width, w->common.logical_height,
      w->common.hardware_planes );

    if ( new_pixmap == NULL )
	{
	destroy_backing_store( w );
	WVT$CLEAR_REGIS( w );
	return;
	}
	
    new_gc = XCreateGC( rs->display, new_pixmap, 0, 0 );

    width = ( w->common.logical_width < rs->bs_width ) ?
      w->common.logical_width : rs->bs_width;
    height = ( w->common.logical_height < rs->bs_height ) ?
      w->common.logical_height : rs->bs_height;

    if ( w->common.logical_width > rs->bs_width
      || w->common.logical_height > rs->bs_height )
	{  /* new backing store is bigger so fill the excess with background */
	XSetForeground( rs->display, new_gc, w->core.background_pixel );
	XFillRectangle( rs->display, new_pixmap, new_gc, 0, 0,
	  w->common.logical_width, w->common.logical_height );
	}

    XCopyArea( rs->display, rs->bs_pixmap, new_pixmap, rs->gc,
      0, 0, width, height, 0, 0 );

    regis_destroy_display( w );
    regis_create_display( w );
    create_backing_store( w, new_pixmap, new_gc );
    WVT$CLEAR_REGIS( w );
}

/* regis_scroll_up - (defer) copy an area of the backing store up */

regis_scroll_up( w, dy, x, y, width, height )
    DECtermWidget w;		/* DECterm context block */
    int dy;			/* number of pixels to scroll up by */
    int x, y, width, height;	/* rectangle to be scrolled */
{
    int i;
    struct regis_cntx *rs = ( struct regis_cntx * ) w->regis;

/* prune the defer queue by combining adjacent operations into a single
   request */

    for ( i = rs->defer_count - 1; i >= 0; i-- )
	{  /* for each queue element starting with the last one */
	if ( rs->defer_queue[i].op != DEFER_SCROLL_VERTICAL
	  || rs->defer_queue[i].p1 > 0
	  || rs->defer_queue[i].p2 != x
	  || rs->defer_queue[i].p3 != y
	  || rs->defer_queue[i].p4 != width
	  || rs->defer_queue[i].p5 != height )
	    break;
	dy -= rs->defer_queue[i].p1;
	}
    rs->defer_count = i + 1;
    if ( dy >= height )
	regis_erase_rectangle( w, w->core.background_pixel, x, y, width,
	  height );
    else
	regis_defer_operation( w, DEFER_SCROLL_VERTICAL, -dy, x, y, width,
	  height );
}

/* regis_scroll_down - (defer) copy an area of the backing store down */

regis_scroll_down( w, dy, x, y, width, height )
    DECtermWidget w;		/* DECterm context block */
    int dy;			/* number of pixels to scroll down by */
    int x, y, width, height;	/* rectangle to be scrolled */
{
    int i;
    struct regis_cntx *rs = ( struct regis_cntx * ) w->regis;

/* prune the defer queue by combining adjacent operations into a single
   request */

    for ( i = rs->defer_count - 1; i >= 0; i-- )
	{  /* for each queue element starting with the last one */
	if ( rs->defer_queue[i].op != DEFER_SCROLL_VERTICAL
	  || rs->defer_queue[i].p1 < 0
	  || rs->defer_queue[i].p2 != x
	  || rs->defer_queue[i].p3 != y
	  || rs->defer_queue[i].p4 != width
	  || rs->defer_queue[i].p5 != height )
	    break;
	dy += rs->defer_queue[i].p1;
	}
    rs->defer_count = i + 1;
    if ( dy >= height )
	regis_erase_rectangle( w, w->core.background_pixel, x, y, width,
	  height );
    else
	regis_defer_operation( w, DEFER_SCROLL_VERTICAL, dy, x, y, width,
	  height );
}

/* regis_scroll_left - (defer) copy an area of the backing store left */

regis_scroll_left( w, dx, x, y, width, height )
    DECtermWidget w;		/* DECterm context block */
    int dx;			/* number of pixels to scroll left by */
    int x, y, width, height;	/* rectangle to be scrolled */
{
    regis_defer_operation( w, DEFER_SCROLL_HORIZONTAL, -dx,
      x, y, width, height );
}

/* regis_scroll_right - (defer) copy an area of the backing store right */

regis_scroll_right( w, dx, x, y, width, height )
    DECtermWidget w;		/* DECterm context block */
    int dx;			/* number of pixels to scroll right by */
    int x, y, width, height;	/* rectangle to be scrolled */
{
    regis_defer_operation( w, DEFER_SCROLL_HORIZONTAL, dx,
      x, y, width, height );
}

/* regis_defer_operation - queue up an operation for later */

regis_defer_operation( w, op, p1, p2, p3, p4, p5 )
    DECtermWidget w;		/* DECterm context block */
    int op;			/* operation being deferred */
    int p1, p2, p3, p4, p5;	/* arguments for operation */
{
    struct regis_cntx *rs;

    rs = ( struct regis_cntx * ) w->regis;

    if ( rs == NULL || ! w->common.backing_store_active )
	return;
    if ( rs->defer_count >= MAX_DEFERRED_OPERATIONS )
	regis_execute_deferred( w );
    rs->defer_queue[ rs->defer_count ].op = op;
    rs->defer_queue[ rs->defer_count ].p1 = p1;
    rs->defer_queue[ rs->defer_count ].p2 = p2;
    rs->defer_queue[ rs->defer_count ].p3 = p3;
    rs->defer_queue[ rs->defer_count ].p4 = p4;
    rs->defer_queue[ rs->defer_count ].p5 = p5;
    ++rs->defer_count;
}

/* regis_execute_deferred - excute operations from defer_queue */

regis_execute_deferred( w )
    DECtermWidget w;		/* DECterm widget context */
{
    struct regis_cntx *rs;
    int i, dx, dy, x, y, width, height;
    Pixel pixel;

    rs = ( struct regis_cntx * ) w->regis;

    if ( ! w->common.backing_store_active )
	{
	rs->defer_count = 0;
	return;
	}

    for ( i = 0; i < rs->defer_count; i++ )
	switch( rs->defer_queue[i].op )
	    {
	    case DEFER_SCROLL_VERTICAL:
		dy = rs->defer_queue[i].p1;
		x = rs->defer_queue[i].p2;
		y = rs->defer_queue[i].p3;
		width = rs->defer_queue[i].p4;
		height = rs->defer_queue[i].p5;
		if ( dy > 0 )
		    {
		    XCopyArea( rs->display, rs->bs_pixmap, rs->bs_pixmap,
		      w->sixel.bs_gc, x, y, width, height - dy, x, y + dy );
		    XFillRectangle( rs->display, rs->bs_pixmap, w->sixel.bs_gc,
		      x, y, width, dy );
		    }
		else
		    {
		    XCopyArea( rs->display, rs->bs_pixmap, rs->bs_pixmap,
		      w->sixel.bs_gc, x, y - dy, width, height + dy, x, y );
		    XFillRectangle( rs->display, rs->bs_pixmap, w->sixel.bs_gc,
		      x, y + height + dy, width, -dy );
		    }
		break;
	    case DEFER_SCROLL_HORIZONTAL:
		dx = rs->defer_queue[i].p1;
		x = rs->defer_queue[i].p2;
		y = rs->defer_queue[i].p3;
		width = rs->defer_queue[i].p4;
		height = rs->defer_queue[i].p5;
		if ( dx > 0 )
		    XCopyArea( rs->display, rs->bs_pixmap, rs->bs_pixmap,
		      w->sixel.bs_gc, x, y, width - dx, y, x + dx, y );
		else
		    XCopyArea( rs->display, rs->bs_pixmap, rs->bs_pixmap,
		      w->sixel.bs_gc, x - dx, y, width + dx, height, x, y );
		break;
	    case DEFER_ERASE:
		pixel = rs->defer_queue[i].p1;
		x = rs->defer_queue[i].p2;
		y = rs->defer_queue[i].p3;
		width = rs->defer_queue[i].p4;
		height = rs->defer_queue[i].p5;
		XFillRectangle( rs->display, rs->bs_pixmap, w->sixel.bs_gc,
		  x, y, width, height );
		break;
	    }
    rs->defer_count = 0;
}

/* adjust_point_list - convert point list coordinates to logical space */

adjust_point_list()
{
    int i;
    struct regis_cntx *rs = RSTRUCT;

    for ( i = 0; i < rs->numpoints; i++ )
	{
	rs->point_list[i].x -= rs->x_offset;
	rs->point_list[i].y -= rs->y_offset;
	}
}

/* regis_adjust_origin - adjust the x and y offsets */

regis_adjust_origin( w )
    DECtermWidget w;
{
    struct regis_cntx *rs = ( struct regis_cntx * ) w->regis;

    rs->x_offset = X_MARGIN - w->common.origin_x;
    rs->y_offset = Y_MARGIN - w->common.origin_y;
}

/* check_backing_store - create the backing store if it doesn't exist */

check_backing_store( w )
    DECtermWidget w;
{
    w->common.graphics_visible = TRUE;

    if ( w->common.backing_store_active || ! w->common.backingStoreEnable )
	return;

    create_backing_store( w, NULL, NULL );
}

/* create_backing_store - create the graphics backing store and GCs */

create_backing_store( w, new_pixmap, new_gc )
    DECtermWidget w;
    Drawable new_pixmap;
    GC new_gc;
{
    XGCValues xgcv;
    Pixel pixel;
    struct regis_cntx *rs = RSTRUCT = ( struct regis_cntx * ) w->regis;

    if ( !rs->initialized )
	G3_INITIALIZE();

    if ( new_pixmap == NULL )
	{
	rs->bs_pixmap = create_pixmap( rs->display, rs->window,
	  w->common.logical_width, w->common.logical_height,
	  w->common.hardware_planes );
/*
 * Check for failure.  We really should print an error message here, but we
 * dont want to print hundreds of error messages, one for each attempt to
 * create the backing store pixmap.  For now, just return.
 */
	if ( rs->bs_pixmap == NULL )
	    return;
	}
    else
	rs->bs_pixmap = new_pixmap;

    w->common.backing_store_active = TRUE;

    rs->bs_width = w->common.logical_width;
    rs->bs_height = w->common.logical_height;

    xgcv.graphics_exposures = False;

    w->sixel.bs_gc = XCreateGC( rs->display, rs->bs_pixmap,
	GCGraphicsExposures, &xgcv );

    if ( new_gc == NULL )
	{
	rs->bs_gc = XCreateGC( rs->display, rs->bs_pixmap, 0, 0 );
	if ( rs->bs_gc == NULL )
	    {
	    destroy_backing_store( w );
	    return;
	    }
	}
    else
	rs->bs_gc = new_gc;
    xgcv.foreground = rs->gc_foreground_color;
    xgcv.background = rs->gc_background_color;
    xgcv.function = rs->gc_function;
    xgcv.line_style = rs->gc_line_style;
    xgcv.plane_mask = rs->gc_plane_mask;
    rs->gc_pattern = -1;
    rs->gc_multiplier = 0;
    XChangeGC( rs->display, rs->bs_gc, GCForeground | GCBackground |
      GCFunction | GCLineStyle | GCPlaneMask, &xgcv );

    xgcv.foreground = rs->text_gc_foreground_color;
    xgcv.background = rs->text_gc_background_color;
    xgcv.function = rs->text_gc_function;
    xgcv.plane_mask = rs->text_gc_plane_mask;
    xgcv.font = w->output.font_list[ FONT_NORMAL ]->fid;
    rs->bs_text_gc = XCreateGC( rs->display, rs->bs_pixmap, GCForeground |
      GCBackground | GCFunction | GCPlaneMask | GCFont, &xgcv );
    if ( rs->bs_text_gc == NULL )
	{
	destroy_backing_store( w );
	return;
	}

    xgcv.foreground = 0;
    xgcv.background = -1;
    xgcv.function = GXand;
    rs->bs_text_clear_gc = XCreateGC( rs->display, rs->bs_pixmap,
      GCForeground | GCBackground | GCFunction, &xgcv );
    if ( rs->bs_text_clear_gc == NULL )
	{
	destroy_backing_store( w );
	return;
	}

    xgcv.foreground = rs->text_background_gc_foreground;
    xgcv.function = rs->text_background_gc_function;
    xgcv.plane_mask = rs->text_background_gc_plane_mask;
    rs->bs_text_background_gc = XCreateGC( rs->display, rs->bs_pixmap,
      GCForeground | GCBackground | GCPlaneMask, &xgcv );
    if ( rs->bs_text_background_gc == NULL )
	{
	destroy_backing_store( w );
	return;
	}

    xgcv.foreground = rs->screen_erase_gc_foreground;
    xgcv.plane_mask = -1;
    rs->bs_screen_erase_gc = XCreateGC( rs->display, rs->bs_pixmap,
      GCForeground | GCPlaneMask, &xgcv );
    if ( rs->bs_screen_erase_gc == NULL )
	{
	destroy_backing_store( w );
	return;
	}

    xgcv.foreground = rs->shade_gc_foreground_color;
    xgcv.background = rs->shade_gc_background_color;
    xgcv.function = rs->shade_gc_function;
    xgcv.fill_style = rs->shade_gc_fill_style;
    xgcv.plane_mask = rs->shade_gc_plane_mask;
    rs->bs_shade_gc = XCreateGC( rs->display, rs->bs_pixmap,
      GCForeground | GCBackground | GCFunction | GCFillStyle | GCPlaneMask,
      &xgcv );
    if ( rs->bs_shade_gc == NULL )
	{
	destroy_backing_store( w );
	return;
	}

    set_bs_clipping_rectangles();

    rs->defer_count = 0;

    XSetForeground( rs->display, w->sixel.bs_gc, w->core.background_pixel );
    XFillRectangle( rs->display, rs->bs_pixmap, w->sixel.bs_gc,
      0, 0, w->common.logical_width, w->common.logical_height );
}

/* destroy_backing_store - release the resources used by the backing store */

destroy_backing_store( w )
    DECtermWidget w;
{
    struct regis_cntx *rs = ( struct regis_cntx * ) w->regis;

    if ( w->common.backing_store_active )
	{
	if ( rs->bs_pixmap != NULL )
	    {
	    XFreePixmap( rs->display, rs->bs_pixmap );
	    rs->bs_pixmap = NULL;
	    }
	if ( w->sixel.bs_gc != NULL )
	    {
	    XFreeGC( rs->display, w->sixel.bs_gc );
	    w->sixel.bs_gc = NULL;
	    }
	if ( rs->bs_gc != NULL )
	    {
	    XFreeGC( rs->display, rs->bs_gc );
	    rs->bs_gc = NULL;
	    }
	if ( rs->bs_text_gc != NULL )
	    {
	    XFreeGC( rs->display, rs->bs_text_gc );
	    rs->bs_text_gc = NULL;
	    }
	if ( rs->bs_text_clear_gc != NULL )
	    {
	    XFreeGC( rs->display, rs->bs_text_clear_gc );
	    rs->bs_text_clear_gc = NULL;
	    }
	if ( rs->bs_text_background_gc != NULL )
	    {
	    XFreeGC( rs->display, rs->bs_text_background_gc );
	    rs->bs_text_background_gc = NULL;
	    }
	if ( rs->bs_screen_erase_gc != NULL )
	    {
	    XFreeGC( rs->display, rs->bs_screen_erase_gc );
	    rs->bs_screen_erase_gc = NULL;
	    }
	if ( rs->bs_shade_gc != NULL )
	    {
	    XFreeGC( rs->display, rs->bs_shade_gc );
	    rs->bs_shade_gc = NULL;
	    }
	w->common.backing_store_active = FALSE;
	}
}

/* get_glyph - read a glyph from the server */

get_glyph( alph, ch )
    int alph; unsigned char ch;
{
    struct regis_cntx *rs = RSTRUCT;
    DECtermWidget w = rs->widget;
    int x, y, image_bit, glyph_size;
    char *image_byte;
    XImage *g_image;

    glyph_size = ( rs->char_width + 7 ) / 8 * rs->char_height;
    if ( rs->kanji_regis )
    {
    if ( rs->font_glyphs[alph][ ch ] != NULL )
    	XtFree( rs->font_glyphs[alph][ ch ] );
    rs->font_glyphs[alph][ ch ] = ( char * ) XtCalloc( glyph_size, 1 );

    if ( rs->load_font_pixmap == NULL )
	{
	rs->load_font_pixmap = XCreatePixmap( rs->display, rs->window,
		rs->char_width, rs->char_height, 1 );
	rs->load_font_gc = XCreateGC( rs->display, rs->load_font_pixmap, 0, 0 );
	XSetForeground( rs->display, rs->load_font_gc, 1 );
	XSetBackground( rs->display, rs->load_font_gc, 0 );
	} else {
	XFreePixmap( rs->display, rs->load_font_pixmap );
	rs->load_font_pixmap = XCreatePixmap( rs->display, rs->window,
		rs->char_width, rs->char_height, 1 );
	}

    draw_common( rs->display, rs->load_font_pixmap, rs->load_font_gc,
	0, rs->char_ascent, &ch, 1 );
    }
    else
    {
    rs->font_glyphs[alph][ ch ] = ( char * ) XtCalloc( glyph_size, 1 );
    if ( rs->load_font_pixmap == NULL )
	{
	rs->load_font_pixmap = XCreatePixmap( rs->display, rs->window,
	  rs->char_width, rs->char_height, 1 );
	rs->load_font_gc = XCreateGC( rs->display, rs->load_font_pixmap, 0, 0 );
	XSetFont( rs->display, rs->load_font_gc,
	  w->output.font_list[ FONT_NORMAL ]->fid );
	XSetForeground( rs->display, rs->load_font_gc, 1 );
	XSetBackground( rs->display, rs->load_font_gc, 0 );
	}
    XDrawImageString( rs->display, rs->load_font_pixmap, rs->load_font_gc,
      0, rs->char_ascent, (char *)&ch, 1 );
    }
    g_image = XGetImage( rs->display, rs->load_font_pixmap, 0, 0,
      rs->char_width, rs->char_height, 1, XYPixmap );
    if ( g_image == NULL )
	return;	/* eventually might want an error message */
    image_byte = rs->font_glyphs[alph][ch];
    image_bit = 1;
    for ( y = 0; y < rs->char_height; y++ )
	{
	for ( x = 0; x < rs->char_width; x++ )
	    {
	    if ( XGetPixel( g_image, x, y ) == 1 )
		*image_byte |= image_bit;
	    if ( image_bit == 128 )
		{
		image_byte ++;
		image_bit = 1;
		}
	    else
		image_bit <<= 1;
	    }
	if ( image_bit != 1 )
	    {
	    image_byte++;
	    image_bit = 1;
	    }
	}
/*    XDestroyImage( g_image );*/
}

/* set_screen_background - set the screen background color

The screen background is a mono cutoff value: anything with that mono value or
less is drawn black and anything with that mono value or more is drawn white.

*/

set_screen_background( background_pixel )
    Pixel background_pixel;
{
    int mono;
    int color, num_colors;
    Pixel pixel;
    struct regis_cntx *rs = RSTRUCT;
    DECtermWidget w = rs->widget;

    if ( background_pixel == w->common.black_pixel )
	mono = 0;
    else
	mono = 0xffff;

    rs->screen_background_mono = mono;

    num_colors = ( 1 << w->common.bitPlanes );

    for ( color = 0; color < num_colors; color++ )
	{
	if ( mono < 0x8000 )
	    if ( w->common.color_map_mono[ color ] >= 0x4000 )
		pixel = w->common.white_pixel;
	    else
		pixel = w->common.black_pixel;
	else
	    if ( w->common.color_map_mono[ color ] >= 0xc000 )
		pixel = w->common.white_pixel;
	    else
		pixel = w->common.black_pixel;
	w->common.color_map[ color ].pixel = pixel;
	}
}

/* regis_timeout_expired -- called when the timer has gone off for S(T) */

void regis_timeout_expired( w )
    DECtermWidget w;
{
    struct regis_cntx *rs = ( struct regis_cntx * ) w->regis;

    rs->paused = FALSE;
    rs->timer = NULL;
    s_start_output( w, STOP_OUTPUT_OTHER );
}

/* report_cell_standard - report the standard character size */

void report_cell_standard( unit_xp, unit_yp, display_xp, display_yp )
    int *unit_xp, *unit_yp, *display_xp, *display_yp;
{
    struct regis_cntx *rs = RSTRUCT;
    DECtermWidget w = rs->widget;
    float x_scale, y_scale;

/*
 * First calculate the scale factor to address the window as [0,0][799,479].
 * This parallels the code in G28_SET_OUTPUT_IDS.
 */

    x_scale = w->common.logical_width;
    x_scale /= 800;
    y_scale = w->common.logical_height;
    y_scale /= 480;
    if ( x_scale > y_scale )
	x_scale = y_scale;
    else
	y_scale = x_scale;
/*
 * Now scale a standard [8,20] character using the calculated scale factor.
 * Round to the nearest integer pixel size.
 */
    *unit_xp = UNMAP_X_RELATIVE( (int) ( x_scale * 8 + 0.5) );
    *unit_yp = UNMAP_Y_RELATIVE( (int) ( y_scale * 20 + 0.5 ) );
    *display_xp = UNMAP_X_RELATIVE( (int) ( x_scale * 9 + 0.5 ) );
    *display_yp = UNMAP_Y_RELATIVE( (int) ( y_scale * 20 + 0.5 ) );
}

/* regis_change_font - change the standard font size */

void regis_change_font( w )
    DECtermWidget w;
{
    int ch, alph;
    char *glyph;
    struct regis_cntx *rs = ( struct regis_cntx * ) w->regis;

    if ( rs == NULL )
	return;
    rs->char_width = w->output.char_width;
    if ( rs->kanji_regis )
    rs->char_height = w->output.real_char_height;
    else
    rs->char_height = w->output.char_height;

    if ( ! rs->initialized )
	return;

    if ( rs->kanji_regis )
	G3_INITIALIZE();

#ifdef CANNED_TEXT_FONT

    for ( alph = 1; alph < TOTAL3_NUMBER_OF_ALPHABETS; alph++ )

#else

    for ( alph = 0; alph < TOTAL3_NUMBER_OF_ALPHABETS; alph++ )

#endif

	for ( ch = 0; ch < 256; ch++ )
	    {	/* free all character glyphs that were allocated */
	    glyph = rs->font_glyphs[ alph ][ ch ];
	    if ( glyph != NULL && glyph != solid && glyph != solid_wide )
		{
		XtFree( glyph );
		rs->font_glyphs[ alph ][ ch ] = NULL;
		}
	    }

    if ( rs->load_font_pixmap != NULL )
	{	/* pixmap size is changing, so free it */
	XFreeGC( rs->display, rs->load_font_gc );
	XFreePixmap( rs->display, rs->load_font_pixmap );
	rs->load_font_pixmap = NULL;
	}

    RSTRUCT = rs;
    ucs_update_cell_standard();	/* update character size info */
}


/* set_hls_color - set a color map entry given its HLS color */

set_hls_color( index, hue, lightness, saturation )
    int index;			/* ReGIS color index */
    int hue, lightness, saturation;
				/* HLS color to set */
{
    int red, green, blue, mono;	/* RGB equivalent color */

    hlsrgb( hue, lightness, saturation, &red, &green, &blue, &mono );
    G55_SET_COLOR_MAP_ENTRY( index, red, green, blue, mono );
}

/* set_color - set a "pure" color using its code value */

set_color( index, color_code )
    int index, color_code;
{
    struct regis_cntx *rs = RSTRUCT;
    DECtermWidget w = rs->widget;
    XColor *color = &w->common.pure_color[color_code];
    unsigned long red = color->red, green = color->green, blue = color->blue,
	mono;

    mono = ( green * 59 + red * 30 + blue * 11 ) / 100;
    G55_SET_COLOR_MAP_ENTRY( index, red, green, blue, mono );
}

/* set_gray - set a color map entry to a shade of gray */

set_gray( index, percent_lightness )
    int index, percent_lightness;
{
    int lightness;

    lightness = percent_lightness * 65536 / 100;
    if ( lightness > 65535 )
	lightness = 65535;
    G55_SET_COLOR_MAP_ENTRY( index, lightness, lightness, lightness,
	lightness );
}

/* find_default_background - look up the background color and store its value */

find_default_background( w )
    DECtermWidget w;
{
    find_rgb_color( w, w->core.background_pixel,
      &w->common.default_background_red,
      &w->common.default_background_green,
      &w->common.default_background_blue,
      &w->common.default_background_mono );
}

/* find_default_foreground - look up the foreground color and store its value */

find_default_foreground( w )
    DECtermWidget w;
{
    find_rgb_color( w, w->manager.foreground,
      &w->common.default_foreground_red,
      &w->common.default_foreground_green,
      &w->common.default_foreground_blue,
      &w->common.default_foreground_mono );
}

/* set_default_background - set ReGIS color 0 to initial background color */

set_default_background( w )
    DECtermWidget w;
{
    regis_set_widget( w );
    G55_SET_COLOR_MAP_ENTRY( w->common.text_background_index,
	w->common.default_background_red,
	w->common.default_background_green,
	w->common.default_background_blue,
	w->common.default_background_mono );
    set_background_color( w );
}

/* set_background_color - set ReGIS color 0 to the background color */

set_background_color( w )
    DECtermWidget w;
{
    struct regis_cntx *rs = (struct regis_cntx *) w->regis;

    RSTRUCT = rs;

    if ( w->common.color_map_allocated ) {
	if ( ! ONE_PLANE(w) )
	    o_set_background_color( w,
		fetch_color( w->common.text_background_index ) );
	else {
	    o_set_background_color( w, w->core.background_pixel );
	    set_screen_background( w->common.color_map_mono[
	      w->common.text_background_index]);
	}
    } else
	o_set_background_color( w, w->core.background_pixel );
}

/* set_default_foreground - set ReGIS color 7 to initial foreground */

set_default_foreground( w )
    DECtermWidget w;
{
    regis_set_widget( w );

    G55_SET_COLOR_MAP_ENTRY( w->common.text_foreground_index,
	w->common.default_foreground_red,
	w->common.default_foreground_green,
	w->common.default_foreground_blue,
	w->common.default_foreground_mono );
    set_foreground_color( w );
}

/* set_foreground_color - set ReGIS color 7 to the foreground color */

set_foreground_color( w )
    DECtermWidget w;
{
    struct regis_cntx *rs = ( struct regis_cntx * ) w->regis;

    RSTRUCT = rs;

    if ( w->common.color_map_allocated ) {
	if ( ! ONE_PLANE(w) )
	    o_set_foreground_color( w,
		fetch_color( w->common.text_foreground_index ) );
	else
	    o_set_foreground_color( w, w->manager.foreground );
    } else
	o_set_foreground_color( w, w->manager.foreground );
}

/*
 * WVT$RESET_COLORMAP
 *
 * This routine sets the colormap to its default colors.  This may have the
 * side effects of allowing DECterm to deallocate its colormap, which
 * it can do if there are no graphics visible.
 */

WVT$RESET_COLORMAP( w )
    DECtermWidget w;
{
    struct regis_cntx *rs = ( struct regis_cntx * ) w->regis;
    Boolean color_monitor;
    int num_colors, color;

    if ( w->common.color_map_allocated && ! w->common.graphics_visible )
	{  /* revert to text-only mode with default color map */
	o_set_foreground_color( w, w->common.original_foreground_color );
	o_set_background_color( w, w->common.original_background_color );
	o_set_cursor_plane_mask( w );
        w->common.color_map[ w->common.text_foreground_index ].pixel =
                w->manager.foreground;
        w->common.color_map[ w->common.text_background_index ].pixel =
                w->core.background_pixel;
	o_repaint_display( w );
	destroy_color_map( w );
		/* revert to text-only mode with default color map */
	}

    regis_set_widget( w );

    num_colors = ( 1 << w->common.bitPlanes );
    color_monitor = ( w->common.visual->class != GrayScale
      && w->common.visual->class != StaticGray
      && ! ONE_PLANE(w) );

    set_default_background( w );
    set_default_foreground( w );

    if ( color_monitor )
	{
	for ( color = 0; color < num_colors; color++ )
	    {
	    switch ( color % 16 )
		{
		case 0:
		    if ( color > 0 )
			set_color( color, BLACK );
		    break;
		case 1:
		    if ( num_colors > 2 )
			set_color( color, BLUE );
		    break;
		case 2:
                    if ( num_colors > 8 )
			set_color( color, RED );
		    break;
		case 3:
		    set_color( color, GREEN );
		    break;
		case 4:
		    set_color( color, MAGENTA );
		    break;
		case 5:
		    if ( rs->kanji_regis )
		    set_color( color, CYAN );
		    else
		    set_color( color, BLACK );
		    break;
		case 6:
		    if ( rs->kanji_regis )
		    set_color( color, YELLOW );
		    else
		    set_color( color, WHITE );
		    break;
		case 7:
                    if ( num_colors < 16 )
			if ( rs->kanji_regis )
			set_gray( color, 80 );
			else
			set_gray( color, 53 );
		    break;
		case 8:
		    if ( rs->kanji_regis )
		    set_gray( color, 53 );
		    else
		    set_gray( color, 26 );
		    break;
		case 9:
		    set_color( color, BLUE );
		    break;
		case 10:
		    set_color( color, RED );
		    break;
		case 11:
		    set_color( color, GREEN );
		    break;
		case 12:
		    set_color( color, MAGENTA );
		    break;
		case 13:
		    set_color( color, CYAN );
		    break;
		case 14:
		    set_color( color, YELLOW );
		    break;
		case 15:
		    if ( rs->kanji_regis )
		    set_gray( color, 100 );
		    else
		    set_gray( color, 80 );
		    break;
		}
	    }
	}
    else
	{
	for ( color = 0; color < num_colors; color++ )
	    {
	    switch( color % 8 )
		{
		case 0:
		    if ( color > 0 )
			set_gray( color, 0 );
		    break;
		case 1:
		    if ( num_colors > 2 )
			set_gray( color, 33 );
		    break;
		case 2:
                    if ( num_colors > 8 )
			set_gray( color, 66 );
		    break;
		case 3:
		    set_gray( color, 100 );
		    break;
		case 4:
		    set_gray( color, 25 );
		    break;
		case 5:
		    set_gray( color, 50 );
		    break;
		case 6:
		    set_gray( color, 75 );
		    break;
		case 7:
                    if ( num_colors < 16 || color > 7 )
			set_gray( color, 85 );
		    break;
		}
	    }
	}

}

/* fetch_color - update the value of a color and possibly allocate it.
		ReGIS version, also applies plane mask if appropriate */

Pixel fetch_color( index )
    int index;
{
    struct regis_cntx *rs = RSTRUCT;
    DECtermWidget w = rs->widget;

    if ( ONE_PLANE(w)
      || w->common.graphics_mode == ALLOCATED_COLORS )
	index &= rs->plane_mask;

    return regis_fetch_color( w, index );
}

/* regis_fetch_color - update the value of a color and possibly allocate it.
		Common (exported) version */

Pixel regis_fetch_color( w, index )
    DECtermWidget w;
    int index;
{
    if ( w->common.graphics_mode != ALLOCATED_COLORS
      || w->common.pixel_valid[ index ] )
	return w->common.color_map[ index ].pixel;
    XAllocColor( XtDisplay(w), w->core.colormap, &w->common.color_map[ index ] );
    w->common.pixel_valid[ index ] = TRUE;
    w->common.pixel_allocated[ index ] = TRUE;
    return w->common.color_map[ index ].pixel;
}

regis_bump_color( w, index )
    DECtermWidget w;
    int index;
{
    Pixel pixel;
    struct color_list_element *fle;

    pixel = w->common.color_map[ index ].pixel;

    fle = w->common.free_color_list;

    if ( fle == NULL || fle->n == COLORS_PER_LIST_ELEMENT )
	{
	fle = ALLOC_ARRAY( struct color_list_element, 1 );
	fle->n = 0;
	if ( w->common.free_color_list != NULL )
	    fle->next = w->common.free_color_list;
	else
	    fle->next = NULL;
	w->common.free_color_list = fle;
	}
    fle->pixels[fle->n++] = pixel;
}

regis_free_colors( w )
    DECtermWidget w;
{
    struct color_list_element *fle, *tmp;
    int i;

    for ( fle = w->common.free_color_list; fle != NULL; )
	{
	XFreeColors( XtDisplay(w), w->core.colormap, fle->pixels, fle->n, 0 );
	tmp = fle->next;
	XtFree( (char *)fle );
	fle = tmp;
	}
    w->common.free_color_list = NULL;
}

/*
 * create_pixmap
 *
 * This routine creates a pixmap, returning NULL if the pixmap couldn't be
 * created.
 *
 * Note: calling this routine will cancel any error handler that was
 * previously in effect.
 */

static Pixmap create_pixmap( display, window, width, height, depth )
    Display *display;
    Window window;
    int width, height, depth;
{
    Pixmap pixmap;
    int (*saved_error_handler)();
/*
 * First, process any outstanding errors and input events.  Don't discard
 * events.
 */
    XSync( display, False );
/*
 * Set up the handler to catch errors.
 */
    cp_error = False;
    saved_error_handler = XSetErrorHandler( (XErrorHandler)cp_error_handler );
/*
 * Create the pixmap.
 */
    pixmap = XCreatePixmap( display, window, width, height, depth );

/*
 * Synchronize, reset the error handler, test for an error and return.
 */
    XSync( display, False );
    XSetErrorHandler( saved_error_handler );
    if ( cp_error )
	pixmap = NULL;
    return pixmap;
}

static void cp_error_handler( display, event )
    Display *display;
    XErrorEvent *event;
{
    cp_error = True;
}

draw_common( dpy, drw, gc, x, y, ch, mode )
Display *dpy;
Drawable drw;
GC gc;
int x, y;
unsigned char *ch;
int mode;
{
    struct regis_cntx *rs = RSTRUCT;

    int char_set;
    unsigned char char_code;
    XChar2b char2b;

    char_code = *ch;
    if ( !rs->char_stack_empty )
    {
	if (( char_set = REG_DEC_KANJI ) == rs->g_char_set[0] )
	    XSetFont( dpy, gc, rs->g_text_font[0]->fid );
	else
	    XSetFont( dpy, gc, rs->g_text_font[1]->fid );
    }
    else if ( char_code < 128 )
    {
	char_set = rs->g_char_set[0];
	XSetFont( dpy, gc, rs->g_text_font[0]->fid );
    }
    else
    {
	char_set = rs->g_char_set[1];
	XSetFont( dpy, gc, rs->g_text_font[1]->fid );
    }
    switch ( char_set )
    {
	case REG_ASCII:
	    char_code &= 127;
	    break;
	case REG_LINE_DRAWING:
	    char_code &= 127;
	    if ( char_code >= 96 && char_code <= 126 )
		char_code -= 95;
	    break;
	case REG_SUPPLEMENTAL:          
	    char_code |= 128;                    
	    break;
  	case REG_JIS_ROMAN:
	    char_code &= 127;
	    break;
	case REG_JIS_KATAKANA:
	    char_code |= 128;
	    break;                                     
	case REG_DEC_KANJI:
	    break;
    }
    if ( char_set == REG_DEC_KANJI )
    {
/* instead of XClearArea or something ... */
	char2b.byte1 = 0xa1;
	char2b.byte2 = 0xa1;
	XDrawImageString16( dpy, drw, gc, x, y, &char2b, 1 );

/* remember you've swapped byte order ... */
	char2b.byte1 = char_code | 128;
	char2b.byte2 = rs->char_stack;

 	if ( mode = 0 )
	    XDrawString16( dpy, drw, gc, x, y, &char2b, 1 );
	else
	    XDrawImageString16( dpy, drw, gc, x, y, &char2b, 1 );
    }
    else
    {
	if ( mode = 0 )
	    XDrawString( dpy, drw, gc, x, y, (char *)&char_code, 1 );
	else
	    XDrawImageString( dpy, drw, gc, x, y, (char *)&char_code, 1 );
    }
}
