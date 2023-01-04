/* #module dt_sixel "X0.4" */
/*
 *  Title:	DECterm Sixel Module
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
 *	Parses and outputs sixel data as part of the DECterm widget.
 *
 *  Procedures contained in this module:
 *
 *	sixel_create		Initialize a sixel context
 *	sixel_destroy		Destroy a sixel context
 *	WVT$SET_OVERLAY_SIXEL	Set overlay mode (GXor)
 *	WVT$SET_REPLACE_SIXEL	Set replace mode (GXcopy)
 *	WVT$SIXEL_FLUSH		Init sixel buffer state
 *	xsixel			Parse a block of sixel data
 *
 *  Author:	Robert Messenger
 *
 *  Modification history:
 *
 * Alfred von Campe     18-Dec-1993     BL-E
 *	- Change all occurrances of common.foreground to manager.foreground.
 *
 *  Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 *  Eric Osman		11-May-1993	VXT V2.0
 *	- Allow #nn (hex) for specifying non-printable chars in answerback
 *	  message
 *
 * Eric Osman		04-Feb-1993
 *	- Handle overlay mode properly.  multicolor horizontal line wasn't
 *	  working.  Fix by doing a write on every color change in overlay mode.
 *
 * Aston Chan		19-Nov-1992	Post V3.1/SSB
 *	- Fixed an AccVio when first type mickey.six, "reset terminal" and
 *	  then type eric.vt240.  Problem is sixel->data not cleared.
 *
 * Dave Doucette	04-Apr-1992	V3.1/SSB
 *	- Added to-lung's fix: call XSetForeground in Sixel overlay mode.
 *
 * Aston Chan		03-Mar-1992	V3.1/BL6
 *	- Make sure x, y, width and height in write_sixel_data() are valid
 *	  before passing to XCreatePixmap().
 *
 * Aston Chan		17-Dec-1991	V3.1
 *	- I18n code merge
 *
 * Aston Chan		28-Oct-1991	V3.1
 *	- Not all fields of the sixel structure got initialized.  It causes
 *	  some undefined initial states being used.  Add initialization in
 *	  sixel_initialize().
 *	- Implicitly freeing the data field by XDestroyImage() will be
 *	  complained by Fake_vm.
 *
 * Bob Messenger	23-May-1991	X3.0
 *	- Support overlay mode.
 *
 * Bob Messenger	17-Jul-1990	X3.0-5
 *	Merge in Toshi Tanimoto's changes to support Asian terminals -
 *	- sixel scroll on/off
 *
 * Bob Messenger	11-Apr-1989	X2.0-6
 *	- Call allocate_color_map instead of init_color_map.
 *
 * Bob Messenger	 7-Apr-1989	X2.0-6
 *	- Fix bug where sixel.color was set to sixel.number_status instead
 *	  of sixel.number
 *
 * Bob Messenger	 7-Apr-1989	X2.0-6
 *	- support variable number of bit planes
 *
 * Bob Messenger	 4-Apr-1989	X2.0-5
 *	- support backing store enable
 *
 * Bob Messenger	31-Mar-1989	X2.0-5
 *	- use straight modulo function into colormap, support shared colormap
 *	  entries
 *
 * Bob Messenger	18-Jan-1989	X1.1-1
 *	- prevent crash with bad aspect ratio
 *
 * Bob Messenger	17-Jan-1989	X1.1-1
 *	- moved many ld fields into common area
 *
 * Bob Messenger	13-Jan-1989	X1.1-1
 *	- scroll within scrolling region, not to top of logical display
 *	- check for invalid parameters and overflow during parsing
 *
 * Bob Messenger	07-Dec-1988	X1.1-1
 *	- don't overwrite border
 *	- use correct colors on single plane system
 *
 * Bob Messenger	30-Nov-1988	X1.1-1
 *	- treat repeat counts of 0 as 1
 *
 * Bob Messenger	28-Nov-1988	V1.0-1
 *	- prevent crash with large color numbers (QAR 471)
 *
 * Tom Porcher		12-Aug-1988	X0.4-44
 *	- changed fg/bg references to common/core.
 *
 * Tom Porcher		21-Apr-1988	X0.4-10
 *	- Split sixel_create() into sixel_initialize() and sixel_realize().
 *
 *  R. Messenger	2-Mar-1988
 *	Module created.
 *
 */

#include "wv_hdr.h"
#undef character
#include "regstruct.h"

#ifndef min
#define min(x,y) (((x)<(y))?(x):(y))
#endif

Pixel regis_fetch_color();


/* sixel_initialize - create a sixel context */

void sixel_initialize(w)
    DECtermData *w;		/* widget context block */
{
    w->sixel.data = NULL;	/* no output block yet */
    w->sixel.mode = REPLACE_MODE;
    w->sixel.bs_gc = NULL;
    w->sixel.pixmap = NULL;
    w->sixel.pixmap_width = 0;
    w->sixel.pixmap_height = 0;
    w->sixel.state =0;
    w->sixel.x_origin =0;
    w->sixel.y_origin =0;
    w->sixel.width =0;
    w->sixel.height =0;
    w->sixel.wbyte =0;
    w->sixel.size =0;
    w->sixel.byte =0;
    w->sixel.mask =0;
    w->sixel.repeat =0;
    w->sixel.pixel_count = 0;
    w->sixel.multiplier =0;
    w->sixel.aspect_numerator =0;
    w->sixel.aspect_denominator =0;
    w->sixel.horizontal_extent =0;
    w->sixel.vertical_extent=0;
    w->sixel.row=0;
    w->sixel.color =0;
    w->sixel.rgb_flag =0;
    w->sixel.red =0;
    w->sixel.green =0;
    w->sixel.blue =0;
    w->sixel.hue =0;
    w->sixel.lightness =0;
    w->sixel.saturation=0;
    w->sixel.number_value =0;
    w->sixel.number_status =0;
    w->sixel.number_next_state=0;
    w->sixel.gc=NULL;
    w->sixel.bs_pixmap=NULL;
    w->sixel.pixmap_gc=NULL;
    w->sixel.bs_pixmap_gc=NULL;
    w->sixel.bs_pixmap_width =0;
    w->sixel.bs_pixmap_height =0;
    w->sixel.one_plane_image=False;

}

void sixel_realize(w)
    DECtermData *w;		/* widget context block */
{
    w->sixel.one_plane_image = (( w->sixel.mode == OVERLAY_MODE )
      || ONE_PLANE(w));
    w->sixel.gc = XCreateGC( XtDisplay(w), XtWindow(w), 0, NULL );
    XSetForeground( XtDisplay(w), w->sixel.gc, w->manager.foreground );
    XSetBackground( XtDisplay(w), w->sixel.gc, w->core.background_pixel );
}

/* sixel_destroy - destroy a sixel context */

void sixel_destroy(w)
    DECtermData *w;		/* widget context block */
{
    if ( w->sixel.data != NULL )
	{
	XtFree( w->sixel.data );
	w->sixel.data = NULL;
	}

    if ( w->sixel.gc != NULL )
	{
        XFreeGC( XtDisplay(w), w->sixel.gc );
	w->sixel.gc = NULL;
	}

    if ( w->sixel.pixmap != NULL )
	{
	XFreePixmap( XtDisplay(w), w->sixel.pixmap );
	w->sixel.pixmap = NULL;
	}

    if ( w->sixel.pixmap_gc != NULL )
	{
	XFreeGC( XtDisplay(w), w->sixel.pixmap_gc );
	w->sixel.pixmap_gc = NULL;
	}

    if ( w->sixel.bs_pixmap != NULL ) {
	XFreePixmap( XtDisplay( w ), w->sixel.bs_pixmap );
	w->sixel.bs_pixmap = NULL;
    }
    if ( w->sixel.bs_pixmap_gc != NULL ) {
	XFreeGC( XtDisplay( w ), w->sixel.bs_pixmap_gc );
	w->sixel.bs_pixmap_gc = NULL;
    }
}

/* WVT$SET_OVERLAY_SIXEL - set overlay mode for writing sixel data */

WVT$SET_OVERLAY_SIXEL(ld)
wvtp	ld;			/* widget context block */
{
    if ( ld->sixel.mode == REPLACE_MODE )
	XSetFillStyle( XtDisplay(ld), ld->sixel.gc, FillStippled );
    ld->sixel.mode = OVERLAY_MODE;
    ld->sixel.one_plane_image = True;
}

/* WVT$SET_REPLACE_SIXEL - set replace mode for writing sixel data */

WVT$SET_REPLACE_SIXEL(ld)
wvtp ld;			/* widget context block */
{
    if ( ld->sixel.mode == OVERLAY_MODE )
	XSetFillStyle( XtDisplay(ld), ld->sixel.gc, FillSolid );
    ld->sixel.mode = REPLACE_MODE;
    if ( ! ONE_PLANE(ld) )
	ld->sixel.one_plane_image = False;
}

/*
 * Routine to get to left margin.
 */
goto_sixel_left_margin(ld) wvtp ld;
{
    if (( _cld wvt$l_ext_flags & vte1_m_asian_common ) &&
	( _cld wvt$l_ext_specific_flags & vte2_m_sixel_scroll_mode )) {
	ld->sixel.x_origin = 0;
    } else {
    ld->sixel.x_origin = ( _ld wvt$l_actv_column - 1 ) * ld->output.char_width;
    }
    ld->sixel.pixel_count = 0;
}

/* WVT$SIXEL - init sixel buffer state */

WVT$SIXEL_FLUSH(ld)
wvtp ld;			/* widget context block */
{
    int i;

    o_disable_cursor( ld, CURSOR_DISABLED_DCS );
    allocate_color_map( ld );

  /* set effective foreground and background for each sixel sequence */
    XSetForeground( XtDisplay(ld), ld->sixel.gc, ld->manager.foreground );
    XSetBackground( XtDisplay(ld), ld->sixel.gc, ld->core.background_pixel );
    if (( _cld wvt$l_ext_flags & vte1_m_asian_common ) &&
	( _cld wvt$l_ext_specific_flags & vte2_m_sixel_scroll_mode )) {
	WVT$ERASE_DISPLAY( ld, 1, 1,
	    _ld wvt$l_page_length, _ld wvt$l_column_width );
	ld->sixel.x_origin = 0;
	ld->sixel.y_origin = 0;
    } else {
    ld->sixel.x_origin = ( _ld wvt$l_actv_column - 1 ) * ld->output.char_width;
    ld->sixel.y_origin = ( _ld wvt$l_actv_line - 1 ) * ld->output.char_height;
    ld->sixel.width = ld->common.logical_width - ld->sixel.x_origin;
    }
    ld->sixel.wbyte = ld->sixel.one_plane_image ? ( ld->sixel.width + 7 ) / 8
		: ld->sixel.width;
    ld->sixel.byte = 0;
    ld->sixel.mask = 1;
    ld->sixel.state = PROCESS_SIXEL;
    ld->sixel.repeat = 1;
    ld->sixel.pixel_count = 0;
    ld->sixel.color = -1;	/* means no color was specified */
    if (ld->sixel.data) {
	XtFree(ld->sixel.data);
	ld->sixel.data = NULL;
    }
}

/* xsixel - parse a block of sixel data */

xsixel( ld, pointer, length )
    wvtp ld;			/* widget context block */
    char **pointer;		/* INPUT/OUTPUT: address of next character */
    int length;			/* number of characters in input buffer */
{
    char *iptr,			/* address of current input character */
	*ilast,			/* one past last input character in buffer */
	ch;			/* current input character */
    int i;			/* loop index */

    for ( iptr = *pointer, ilast = iptr + length; iptr < ilast; iptr++ )
	{	/* for each input character */
	ch = *iptr;
	if ( ch == '\32' ) ch = '\77';
	if ( ch == '\33' || ch == '\30' || '\200' <= ch && ch <= '\237' )
	    {	/* exit from DCS for escape, CAN, SUB or C1 control */
	    if ( ld->sixel.data != NULL )
		write_sixel_data(ld);
	    *pointer = iptr;
	    _cld wvt$b_in_dcs = False;
	    if ( _cld wvt$l_ext_flags & vte1_m_asian_common )
		{
		ld->common.cursor_plane_mask = ld->manager.foreground ^
		  ld->core.background_pixel;
		o_set_cursor_plane_mask( ld );
		}
	    o_enable_cursor( ld, CURSOR_DISABLED_DCS );
	    return;
	    }
	if ( ch & 0140 )
	    {	/* graphic character */
	    ch &= 127;		/* strip off the 8th bit */
	    switch ( ld->sixel.state )
		{
		case PROCESS_SIXEL:
		    switch( ch )
			{
			case '!':	/* repeat introducer */
			    PARSE_NUMBER( PROCESS_REPEAT )
			    break;
			case '"':	/* raster introducer */
			    PARSE_NUMBER( PROCESS_RASTER )
			    break;
			case '#':	/* color introducer */
			    if ( ! ONE_PLANE(ld) )
				PARSE_NUMBER( PROCESS_COLOR )
/*
 * Since overlay mode is implemented by writing one color at a time, we need
 * to write the buffer any time the color changes.
 */
			    if ( ld->sixel.mode == OVERLAY_MODE )
				{
				write_sixel_data( ld );
				alloc_sixel_buffer( ld );
				}
			    break;
			case '$':	/* graphic CR */
			    if ( ld->sixel.mode == OVERLAY_MODE )
				{
				write_sixel_data( ld );
				alloc_sixel_buffer( ld );
				}
			    goto_sixel_left_margin( ld );
			    ld->sixel.byte = 0;
			    ld->sixel.mask = 1;
			    break;
			case '+':	/* graphics top of form */
			    write_sixel_data( ld );
			    ld->sixel.x_origin = 0;
			    ld->sixel.y_origin = 0;
			    ld->sixel.byte = 0;
			    ld->sixel.mask = 1;
			    break;
			case '-':	/* graphic new line */
			    write_sixel_data( ld );
			    alloc_sixel_buffer( ld );
			    goto_sixel_left_margin( ld );
			    ld->sixel.y_origin += ld->sixel.height;
			    break;
			case 127:	/* out of range */
			    break;
			default:
			    if ( ch >= '?' )
				{	/* found a sixel */
				add_sixel( ld, (int) ( ch - '?' ) );
				}
			    break;
			}
		    break;

		case PROCESS_NUMBER:
		    if ( '0' <= ch && ch <= '9' )
			{  /* add a digit */
			if ( ld->sixel.number_value >= 32767 )
			    {	/* overflow! */
			    ld->sixel.number_status = BIG_NUMBER;
			    }
			else
			    {
			    ld->sixel.number_value *= 10;
			    ld->sixel.number_value += ch - '0';
			    ld->sixel.number_status = GOOD_NUMBER;
			    }
			}
		    else
			{	/* found a non-digit */
			--iptr;	/* push back the character */
			if ( ld->sixel.number_next_state == PROCESS_REPEAT
			  && ( ch < '\77' || '\176' < ch ) ) {
			    ld->sixel.state = PROCESS_SIXEL;
			    break;
			    }
			ld->sixel.state = ld->sixel.number_next_state;
			if ( ld->sixel.number_status == GOOD_NUMBER
			  && ld->sixel.number_value > 32767 )
			    {
			    ld->sixel.number_status = BIG_NUMBER;
			    }
			else if ( ld->sixel.number_status == NO_NUMBER )
			    ld->sixel.number_value = 0;
				/* as a service to our customers */
			if ( ld->sixel.number_status == BIG_NUMBER )
			    ld->sixel.number_value = 32767;
			}
		    break;

		case PROCESS_REPEAT:
		    --iptr;		/* push back the character */
		    if ( ld->sixel.number_status == NO_NUMBER
		      || ld->sixel.number_value == 0 )
			ld->sixel.repeat = 1;
		    else
			ld->sixel.repeat = ld->sixel.number_value;
		    ld->sixel.state = PROCESS_SIXEL;
		    break;

		case PROCESS_RASTER:
		    if ( ld->sixel.number_status == NO_NUMBER || ch != ';' )
			{	/* bad command, so ignore it */
			--iptr;	/* push back character */
			ld->sixel.state = PROCESS_SIXEL;
			}
		    else
			{
			if ( ld->sixel.number_status == BIG_NUMBER
			  || ld->sixel.number_value > 32767 )
			    ld->sixel.aspect_numerator = 32767;
			else
			    ld->sixel.aspect_numerator = ld->sixel.number_value;
			PARSE_NUMBER( PROCESS_RASTER_2 )
			}
		    break;

		case PROCESS_RASTER_2:
		    if ( ld->sixel.number_status == NO_NUMBER )
			{	/* bad command, so ignore it */
			--iptr;	/* push back character */
			ld->sixel.state = PROCESS_SIXEL;
			}
		    else
			{
			if ( ld->sixel.number_status == BIG_NUMBER
			  || ld->sixel.number_value > 32767 )
			    ld->sixel.aspect_denominator = 32767;
			else
			    ld->sixel.aspect_denominator =
			      ld->sixel.number_value;
			if ( ld->sixel.raster_change_ok ) {
			if ( ld->sixel.aspect_numerator != 0
			  && ld->sixel.aspect_denominator != 0 )
			    {  /* set the aspect ratio */
			    _cld wvt$l_sixel_lines =
			      (((( ld->sixel.aspect_numerator * 10 ) /
			      ld->sixel.aspect_denominator ) + 5 ) / 10) * 6;
			    if ( _cld wvt$l_sixel_lines < 6 )
				_cld wvt$l_sixel_lines = 6;
			    }
			}
			if ( ch == ';' )
			    {
			    PARSE_NUMBER( PROCESS_RASTER_3 )
			    }
			else
			    {
			    ld->sixel.state = PROCESS_SIXEL;
			    --iptr;
			    }
			}
		    break;

		case PROCESS_RASTER_3:
		    if ( ld->sixel.number_status == NO_NUMBER )
			{	/* bad command, so ignore it */
			--iptr;
			ld->sixel.state = PROCESS_SIXEL;
			}
		    else
			{
			if( ld->sixel.raster_change_ok )
			ld->sixel.horizontal_extent = ld->sixel.number_value;
			if ( ch == ';' )
			    PARSE_NUMBER( PROCESS_RASTER_3 )
			else
			    {
			    ld->sixel.state = PROCESS_SIXEL;
			    --iptr;
			    }
			}
		    break;

		case PROCESS_RASTER_4:
		    --iptr;	/* push back character */
		    if ( ld->sixel.number_status == NO_NUMBER )
			{	/* bad command, so ignore it */
			ld->sixel.state = PROCESS_SIXEL;
			}
		    else
			{
			if( ld->sixel.raster_change_ok )
			ld->sixel.vertical_extent = ld->sixel.number_value;
			ld->sixel.state = PROCESS_SIXEL;
			}
		    break;

		case PROCESS_COLOR:
		    if ( ld->sixel.number_status != GOOD_NUMBER
		      || ld->sixel.number_value > 255 )
			{	/* bad command, so ignore it */
			--iptr;
			ld->sixel.state = PROCESS_SIXEL;
			break;
			}
		    ld->sixel.color = ld->sixel.number_value;
		    if ( ch == ';' )
			{  /* HLS or RGB color specifier */
			PARSE_NUMBER( PROCESS_RGBFLAG )
			}
		    else
			{
			ld->sixel.state = PROCESS_SIXEL;
			--iptr;
			}
		    break;

		case PROCESS_RGBFLAG:
		    if ( ld->sixel.number_status != GOOD_NUMBER
		      || ld->sixel.number_value < 1
		      || ld->sixel.number_value > 2
		      || ch != ';' )
			{	/* bad command so ignore it */
			--iptr;
			ld->sixel.state = PROCESS_SIXEL;
			}
		    else if ( ld->sixel.number_value == 2 )
			{
			ld->sixel.rgb_flag = True;
			ld->sixel.red = 0;
			ld->sixel.green = 0;
			ld->sixel.blue = 0;
			PARSE_NUMBER( PROCESS_RED )
			}
		    else
			{
			ld->sixel.rgb_flag = False;
			ld->sixel.hue = 0;
			ld->sixel.lightness = 0;
			ld->sixel.saturation = 0;
			PARSE_NUMBER( PROCESS_HUE )
			}
		    break;

		case PROCESS_RED:
		    ld->sixel.red = min( ld->sixel.number_value, 100 );
		    if ( ch == ';' )
			PARSE_NUMBER( PROCESS_GREEN )
		    else
			{
			set_sixel_color( ld );
			ld->sixel.state = PROCESS_SIXEL;
			--iptr;
			}
		    break;

		case PROCESS_GREEN:
		    ld->sixel.green = min( ld->sixel.number_value, 100 );
		    if ( ch == ';' )
			PARSE_NUMBER( PROCESS_BLUE )
		    else
			{
			set_sixel_color( ld );
			ld->sixel.state = PROCESS_SIXEL;
			--iptr;
			}
		    break;


		case PROCESS_BLUE:
		    ld->sixel.blue = min( ld->sixel.number_value, 100 );
		    set_sixel_color( ld );
		    ld->sixel.state = PROCESS_SIXEL;
		    --iptr;
		    break;

		case PROCESS_HUE:
		    ld->sixel.hue = ld->sixel.number_value % 360;
		    if ( ch == ';' )
			PARSE_NUMBER( PROCESS_LIGHTNESS )
		    else
			{
			set_sixel_color( ld );
			ld->sixel.state = PROCESS_SIXEL;
			--iptr;
			}
		    break;

		case PROCESS_LIGHTNESS:
		    ld->sixel.lightness = min( ld->sixel.number_value, 100 );
		    if ( ch == ';' )
			PARSE_NUMBER( PROCESS_SATURATION )
		    else
			{
			set_sixel_color( ld );
			ld->sixel.state = PROCESS_SIXEL;
			--iptr;
			}
		    break;

		case PROCESS_SATURATION:
		    ld->sixel.saturation = min( ld->sixel.number_value, 100 );
		    set_sixel_color( ld );
		    ld->sixel.state = PROCESS_SIXEL;
		    --iptr;
		    break;

		}
	    }
	else
	    switch ( ch )
		{
		case C0_ENQ:	/* ^E */
		    if ( _cld wvt$l_vt200_common_flags & vtc1_m_auto_answerback )
			transmit_answerback ( ld, &_cld wvt$b_answerback[0] );
		    break;

		case C0_BEL:	/* ^G */
		    WVT$BELL( ld );
		    break;

		default:
		    break;	/* ignore anything else */
		}
           if(( ld->sixel.state < PROCESS_RASTER
                      || PROCESS_RASTER_4 < ld->sixel.state )
                      && !( ld->sixel.state == PROCESS_NUMBER
                      && ld->sixel.number_next_state >= PROCESS_RASTER
                      && PROCESS_RASTER_4 >= ld->sixel.number_next_state ))
               ld->sixel.raster_change_ok = 0;
	}
    *pointer = iptr;		/* skip past the input string */
}

/* alloc_sixel_buffer - allocate a buffer for the sixel data */

alloc_sixel_buffer( ld )
    wvtp ld;
{
    int size, i;

    ld->sixel.height = _cld wvt$l_sixel_lines;
    size = ld->sixel.wbyte * _cld wvt$l_sixel_lines;

    if ( ld->sixel.data != NULL )
	{
	if ( size == ld->sixel.size )
	    return;	/* no need to allocate a new buffer */
	XtFree( ld->sixel.data );
	}
    ld->sixel.size = size;

    if ( ld->sixel.one_plane_image )
	ld->sixel.data = XtCalloc( ld->sixel.size, 1 );
    else
	{
	ld->sixel.data = XtMalloc( ld->sixel.size );
	for ( i = 0; i < ld->sixel.size; i++ )
	    ld->sixel.data[i] = ld->core.background_pixel;
	}

    ld->sixel.row = ld->sixel.data;
    ld->sixel.byte = 0;
    ld->sixel.mask = 1;
}

/* add_sixel - write a sixel into the memory bitmap */

add_sixel( ld, sixel )
    wvtp ld;		/* widget context block */
    int sixel;		/* six bits of data for successive scan lines */
{
    int row,		/* row number, 0 to 5 */
	mask,		/* mask of current bit in bitmap byte */
	final_mask,	/* mask for next sixel */
	repeat_left,	/* loop index for repeat count */
	lcnt,		/* loop index for aspect ratio */
	i;		/* loop index for initializing bitmap */
    char *row_ptr,	/* start of current row in bitmap */
	*bptr,		/* address of current byte in bitmap */
	*final_bptr;	/* address of byte in bitmap for next sixel */
    Pixel color;

    if ( ld->sixel.data == NULL )
	{  /* we can't allocate the bitmap until we know the aspect ratio */
	ld->sixel.height = _cld wvt$l_sixel_lines;
	ld->sixel.size = ld->sixel.wbyte * ( ( ( ld->sixel.height +
	  _cld wvt$l_sixel_lines - 1 ) / _cld wvt$l_sixel_lines ) *
	  _cld wvt$l_sixel_lines );
	if ( ld->sixel.one_plane_image )
	    ld->sixel.data = XtCalloc( ld->sixel.size, 1 );
	else
	    {
	    ld->sixel.data = XtMalloc( ld->sixel.size );
	    for ( i = 0; i < ld->sixel.size; i++ )
		ld->sixel.data[i] = ld->core.background_pixel;
	    }

	ld->sixel.row = ld->sixel.data;
	}

    if ( ld->sixel.row >= ld->sixel.data + ld->sixel.size )
	return;		/* past the end of the bitmap */

    if ( ld->sixel.one_plane_image )
	{  /* single plane = one bit per pixel */
	for ( row = 0, row_ptr = ld->sixel.row; row < 6; row++ )
	    {  /* for each of six scan lines */
	    for ( lcnt = 0; lcnt < _cld wvt$l_sixel_lines / 6; lcnt++,
	      row_ptr += ld->sixel.wbyte )
		{  /* for each repitition due to the aspect ratio */
		bptr = &row_ptr[ ld->sixel.byte ];
		mask = ld->sixel.mask;
		for ( repeat_left = ld->sixel.repeat; repeat_left > 0
		  && bptr < row_ptr + ld->sixel.wbyte && mask != 1;
		  repeat_left-- )
		    {  /* fill up the current byte */
		    if ( sixel & 1 )
			*bptr |= mask;
		    mask <<= 1;
		    if ( mask > 0x80 )
			{  /* reached the end of the byte */
			bptr++;
			mask = 1;
			}
		    }
		for ( ; repeat_left >= 8 && bptr < row_ptr + ld->sixel.wbyte;
		  repeat_left -= 8 )
		    {  /* fill as many complete bytes as possible */
		    if ( sixel & 1 )
			*bptr = 0xff;
		    *bptr++;
		    }
		for ( ; repeat_left > 0 && bptr < row_ptr + ld->sixel.wbyte;
		  repeat_left-- )
		    {  /* fill up the last byte */
		    if ( sixel & 1 )
			*bptr |= mask;
		    mask <<= 1;
		    }
		if ( row == 0 && lcnt == 0 )
		    {
		    final_bptr = bptr;
		    final_mask = mask;
		    }
		}
	    sixel >>= 1;
	    }
	ld->sixel.byte = final_bptr - ld->sixel.row;
	ld->sixel.mask = final_mask;
	}
    else
	{  /* multi-plane = 8 bits per pixel */

   /* This code chooses which colors to use for sixel drawing depending
   ** on the number of planes.  The rules are:
   **		color 3 for emulating 2 planes
   **		color 7 for 4 planes
   */
	if ( ld->sixel.color < 0 ) /* No Sixel color definition */
	    color = regis_fetch_color( ld, ld->common.text_foreground_index );
	else
	    color = regis_fetch_color( ld, ld->sixel.color %
		( 1 << ld->common.bitPlanes ) );
	for ( row = 0, row_ptr = ld->sixel.row; row < 6; row++ )
	    {  /* for each of six scan lines */
	    if ( sixel & 1 )
		{  /* don't bother unless we're drawing something */
		for ( lcnt = 0; lcnt < _cld wvt$l_sixel_lines / 6; lcnt++,
		  row_ptr += ld->sixel.wbyte )
		    {  /* for each repetition due to the aspect ratio */
		    bptr = &row_ptr[ ld->sixel.byte ];
		    for ( repeat_left = ld->sixel.repeat; repeat_left > 0
		      && bptr < row_ptr + ld->sixel.wbyte; repeat_left-- )
			*bptr++ = color;
		    }
		}
	    else 
		{  /* skip this row */
		row_ptr += ld->sixel.wbyte * _cld wvt$l_sixel_lines / 6;
		}
	    sixel >>= 1;
	    }
	ld->sixel.byte += ld->sixel.repeat;
	}
/*
 * .pixel_count stores how many pixels wide we've accumulated.  This is used by
 * write_sixel_data so it can know how far to the right the next write should
 * start.  It is only important in overlay mode when the color changes in the
 * middle of a line, in which case we do more than one write on that line.
 *
 */
    ld->sixel.pixel_count += ld->sixel.repeat;
    ld->sixel.repeat = 1;
}

/* write_sixel_data - write the sixels to the screen */

write_sixel_data( ld )
    wvtp ld;			/* widget context block */
{
    XImage *image;
    struct regis_cntx *rs;
    int x, y, width, height, x_origin, y_origin;
    int format;
    Pixel color;
    XGCValues xgcv;

    if ( _cld wvt$l_ext_flags & vte1_m_asian_common &&
	 _cld wvt$l_ext_specific_flags & vte2_m_sixel_scroll_mode )
	{
	if ( ld->sixel.y_origin + ld->sixel.height >=
		ld->common.logical_height )
	    ld->sixel.height = 0;
	}
    else
	while ( ld->sixel.y_origin + ld->sixel.height >
	_ld wvt$l_bottom_margin * ld->output.char_height - ld->common.origin_y )
	{
	WVT$UP_SCROLL( ld, _ld wvt$l_top_margin, 1, FALSE );
	ld->sixel.y_origin -= ld->output.char_height;
	}

    if ( ld->sixel.data == 0 )
	return;

    if ( !ld->sixel.width || !ld->sixel.height )
	return;

    if ( ld->sixel.mode == OVERLAY_MODE
      && ld->sixel.byte == 0 && ld->sixel.mask == 1 )
	{
	/*
	 * overlay mode: no bits were set so return
	 */
	return;
	}

	{
	/* Move cursor to the bottom of the sixel data */
	int line;
     	line = ( ld->common.origin_y + ld->sixel.y_origin + ld->sixel.height )
		/ ld->output.char_height + _ld wvt$l_top_margin;
	_ld wvt$l_actv_line = ( line <= _ld wvt$l_bottom_margin )
		? line : _ld wvt$l_bottom_margin;
	_ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;
	}
    
    if ( ld->sixel.one_plane_image )
	format = XYBitmap;
    else
	format = ZPixmap;
    image = XCreateImage( XtDisplay(ld), ld->common.visual,
      ld->sixel.one_plane_image ? 1 : ld->common.hardware_planes,
      format, 0, ld->sixel.data,
      ld->sixel.width, ld->sixel.height, 8, ld->sixel.wbyte );
    image->bitmap_bit_order = LSBFirst;
    image->byte_order = LSBFirst;
    if ( ONE_PLANE(ld) )
	{
	XSetForeground( XtDisplay(ld), ld->sixel.gc, ld->manager.foreground );
	XSetBackground( XtDisplay(ld), ld->sixel.gc, ld->core.background_pixel );
	}
    x = ld->sixel.x_origin - ld->common.origin_x + X_MARGIN;
    y = ld->sixel.y_origin - ld->common.origin_y + Y_MARGIN;
    width = ld->sixel.width;
    height = ld->sixel.height;

    /* if x < 0, ignore ld->common.origin_x
     */
    if ( x < 0 )
	x = ld->sixel.x_origin + X_MARGIN;

    if ( x < X_MARGIN )
	{
	width -= X_MARGIN - x;
	x = X_MARGIN;
	}
    if ( x + width > X_MARGIN + ld->common.display_width )
	width = X_MARGIN + ld->common.display_width - x;

    /* Make sure width is positive.  If it is negative, XCreatePixmap() will
     * crash.  If it is zero, then x_origin = x % width will crash.
     */
    if ( width <= 0 )
	width = X_MARGIN + ld->common.display_width;

    /* if y < 0, ignore ld->common.origin_y
     */
    if ( y < 0 )
	y = ld->sixel.y_origin + Y_MARGIN;

    if ( y < Y_MARGIN )
	{
	height -= Y_MARGIN - y;
	y = Y_MARGIN;
	}
    if ( y + height > Y_MARGIN + ld->common.display_height )
	height = Y_MARGIN + ld->common.display_height - y;

    /* Make sure height is positive.  If it is negative, XCreatePixmap() will
     * crash.  If it is zero, then y_origin = y % height will crash.
     */
    if ( height <= 0 )
	height = Y_MARGIN + ld->common.display_height;

    if ( ld->sixel.mode == OVERLAY_MODE )
	{
	/*
	 * overlay, so draw image as a stipple pattern
	 */
	if ( ld->sixel.pixmap != NULL && ( ld->sixel.pixmap_width != width
	  || ld->sixel.pixmap_height != height ) )
	    {
	    /*
	     * pixmap exists but wrong size, so free it
	     */
	    XFreePixmap( XtDisplay(ld), ld->sixel.pixmap );
	    ld->sixel.pixmap = NULL;

	    if ( ld->sixel.pixmap_gc != NULL )
		{
	        XFreeGC( XtDisplay(ld), ld->sixel.pixmap_gc );
		ld->sixel.pixmap_gc = NULL;
		}
	    }
	if ( ld->sixel.pixmap == NULL )
	    {
	    /*
	     * need to create the pixmap, size of one sixel line
	     */
	    ld->sixel.pixmap = XCreatePixmap( XtDisplay(ld), XtWindow(ld),
	      width, height, 1 );
	    ld->sixel.pixmap_width = width;
	    ld->sixel.pixmap_height = height;
	    xgcv.background = 0;
	    xgcv.foreground = 1;
	    ld->sixel.pixmap_gc = XCreateGC( XtDisplay(ld), ld->sixel.pixmap,
		GCForeground | GCBackground, &xgcv );
	    }
	/*
	 * Write the (single plane) pattern to the pixmap.
	 */
	XPutImage( XtDisplay(ld), ld->sixel.pixmap, ld->sixel.pixmap_gc, image,
	  0, 0, 0, 0, width, height );
	/*
	 * Figure out which color to use for the foreground (1's in the
	 * bitmap; nothing will be written for the 0's).  This could be
	 * optimized: we shouldn't have to XSetForeground unless the color
	 * changes.
	 */


	if ( ONE_PLANE(ld) ) {
	    if ( ld->common.original_background_color >
		 ld->common.original_foreground_color ) /* light back ground */
		color = ld->common.black_pixel;
	    else
		color = ld->common.white_pixel;
	} else if ( ld->sixel.color < 0 ) { /* No Sixel color definition */
	    if ( ld->common.original_background_color >
		 ld->common.original_foreground_color ) /* light back ground */
		color = regis_fetch_color ( ld,
			ld->common.text_foreground_index ?
			ld->common.text_background_index :
			ld->common.text_foreground_index );
	    else
	    	color = regis_fetch_color( ld,
			ld->common.text_foreground_index ?
   			ld->common.text_foreground_index : 
   			ld->common.text_background_index );
	} else {
	    color = regis_fetch_color( ld, ld->sixel.color %
		( 1 << ld->common.bitPlanes ) );
	}

	XSetForeground( XtDisplay(ld), ld->sixel.gc, color );

	/*
	 * Now draw the image in the window by using it as a stipple pattern.
	 * Choose the origin so that the starting point of the pattern is
	 * the upper left corner of the rectangle we draw.
	 */
	x_origin = x % width;
	y_origin = y % height;
	XSetTSOrigin( XtDisplay(ld), ld->sixel.gc, x_origin, y_origin );
	/*
	 * Just writing the pixmap isn't enough on some servers - also need
	 * to set the stipple pattern again.
	 */
	XSetStipple( XtDisplay(ld), ld->sixel.gc, ld->sixel.pixmap );
	XFillRectangle( XtDisplay(ld), XtWindow(ld), ld->sixel.gc,
	  x, y, width, height );
	}
    else
	{
	/*
	 * replace mode, so draw directly into window
	 */
	XPutImage( XtDisplay(ld), XtWindow(ld), ld->sixel.gc, image,
	  0, 0, x, y, width, height );
	}

    rs = ( struct regis_cntx * ) ld->regis;
    check_backing_store( ld );
    if ( ld->common.backing_store_active )
	{
	regis_execute_deferred( ld );
	if ( !ld->sixel.bs_pixmap_gc ) {
	    xgcv.foreground = color;
	    xgcv.background = ld->core.background_pixel;
	    xgcv.graphics_exposures = False;
	    xgcv.fill_style = FillStippled;
	    ld->sixel.bs_pixmap_gc = XCreateGC( rs->display, rs->bs_pixmap,
		GCForeground | GCBackground | GCGraphicsExposures |
		GCFillStyle, &xgcv );
	}
	if ( ld->sixel.mode == OVERLAY_MODE ) {
	    XSetForeground( rs->display, ld->sixel.bs_pixmap_gc, color );
	    XSetStipple( rs->display, ld->sixel.bs_pixmap_gc,
		ld->sixel.pixmap );
	    XSetTSOrigin( rs->display, ld->sixel.bs_pixmap_gc,
		ld->sixel.x_origin % ld->sixel.width,
		ld->sixel.y_origin % ld->sixel.height );
	    XFillRectangle( rs->display, rs->bs_pixmap, ld->sixel.bs_pixmap_gc,
		ld->sixel.x_origin, ld->sixel.y_origin, ld->sixel.width,
		ld->sixel.height );
	} else {
	    XPutImage( rs->display, rs->bs_pixmap, ld->sixel.bs_pixmap_gc, image,
		0, 0, ld->sixel.x_origin, ld->sixel.y_origin,
		ld->sixel.width, ld->sixel.height );
	}
	}

    XDestroyImage( image );
    ld->sixel.data = 0;
/*
 * Update sixel origin, so next write happens in correct location.  This
 * is crucial for overlay mode, in which the next write might happen to the
 * right of previous.  In replace mode, we only do a single write for the
 * entire horizontal strip.
 */
ld->sixel.x_origin += ld->sixel.pixel_count;
ld->sixel.pixel_count = 0;
}

/* set_sixel_color - set a color in the sixel color map */

set_sixel_color( ld )
    wvtp ld;		/* widget context block */
{
    int red, green, blue, mono;

    regis_set_widget( ld );	/* point to ReGIS context block */
    if ( ld->sixel.rgb_flag )
	{
	red = ld->sixel.red * 65536 / 100;
	if ( red > 65535 )
	    red = 65535;
	green = ld->sixel.green * 65536 / 100;
	if ( green > 65535 )
	    green = 65535;
	blue = ld->sixel.blue * 65536 / 100;
	if ( blue > 65535 )
	    blue = 65535;
	mono = ( green * 59 + red * 30 + blue * 11 ) / 100;
	}
    else
	hlsrgb( ld->sixel.hue, ld->sixel.lightness, ld->sixel.saturation,
	  &red, &green, &blue, &mono );
    G55_SET_COLOR_MAP_ENTRY(
		ld->sixel.color % ( 1 << ld->common.bitPlanes ),
		red, green, blue, mono );
}
