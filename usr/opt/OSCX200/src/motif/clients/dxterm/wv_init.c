/* #module WV_INIT "V2.1" */
/*
 *  Title:	WV_INIT
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © 1985, 1993                                                 |
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
 *  This module contains the parser init and resize init code.
 *
 *  The routines contained in this module are:
 *
 *	xinit    - Init the parser
 *	xresize  - Re-init stuff following a resize
 *
 *  Author:	Frederick G. Kleinsorge
 *		Low-End Workstation Graphics Engineering
 *
 *  Modification history:
 *
 *  Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 * Alfred von Campe     14-Oct-1992     Ag/BL10
 *      - Added typecasts to satisfy Alpha compiler.
 *
 * Eric Osman		17-Aug-1992	VXT V1.2
 *	- Fix typo where width was being normalized to max height instead of max
 *	  width.
 *
 * Aston Chan		27-Jul-1992	V1.1/post ssb
 *	- in xresize(), _ld wvt$l_actv_column was set to
 *	  wvt$l_page_length by mistake.  It may be the cause of
 *	  some of Tim Fennell's problem.
 *
 * Aston Chan		17-Dec-1991	V3.1
 *	- I18n code merge
 *
 * Aston Chan		1-Sep-1991	Alpha
 *	- Change type of t_width and t_page in xresize() to long int. 
 *	  Complained by DECC.
 *
 * Alfred von Campe     20-May-1991     V3.0
 *      - Change XtCalloc to DECwTermXtCalloc, and XtMalloc to DECwTermXtMalloc
 *        to avoid crashing when increasing number of lines off top or otherwise
 *        resizing the terminal.
 *
 * Alfred von Campe     04-Feb-1991     T3.0
 *      - Change calloc to XtCalloc, malloc to XtMalloc, and free to XtFree.
 *
 * Bob Messenger	12-Mar-1990	V2.1
 *	- Don't crash if we can't allocate memory when resizing.
 *
 * Bob Messenger	18-Aug-1989	X2.0-18
 *	- Fixed memory allocation bug: set wvt$w_trans_widths and
 *	  wvt$b_trans_rendits to the address of the arrays for the main
 *	  array instead of the arrays for the transcript.
 *
 * Bob Messenger	31-May-1989	X2.0-13
 *	- Replace bcopy with memcpy.
 *
 * Bob Messenger	26-Apr-1989	X2.0-8
 *	- Fix bottom and right margins when resizing.
 *	- Call s_clear_display instead of DECwTermClearDisplay so erased
 *	  lines will be saved in the transcript.
 *
 * Bob Messenger	21-Apr-1989	X2.0-7
 *	- Avoid compilation warnings on Ultrix.
 *
 * Bob Messenger	 2-Apr-1989	X2.0-5
 *	- xinit and xresize resize the transcript too, and all arrays are
 *	  allocated based on actual page size, not maximum size.
 *
 * Bob Messenger	16-Jan-1989	X1.1-1
 *	- Moved many ld fields into common area.
 *
 * Mike Leibow          07-Jun-1988
 *      - Prepared code for status line.  Some ld fields are now accessed
 *        by _cld instead of -ld.
 *
 *  FGK0004	Frederick G. Kleinsorge	22-Jul-1987
 *  
 *  o Compute up scroll defer max.
 *
 *  FGK0003	Frederick G. Kleinsorge	16-Apr-1987
 *  
 *  o Change data type of ld
 *
 *  FGK0002	Frederick G. Kleinsorge	05-Mar-1987
 *  
 *  o V3.2
 *
 *  FGK0001	Frederick G. Kleinsorge	22-Jul-1986
 *  
 *  o Update version to X04-017
 *
 *
 */

#include "wv_hdr.h"


/**************************************************************/
xinit(ld,t_width,t_page,t_transcript)   /* Init the structure */
/**************************************************************/

wvtp ld;
int t_width;		/* number of columns */
int t_page;		/* number of lines in main display */
int t_transcript;	/* number of lines in transcript */

{
int y, len, array_size, s_code_cell, s_rend_cell, s_code_base, s_rend_base,
	s_widths, s_rendits;
unsigned char *codes;
REND *rends;

int s_ext_rend_cell, s_ext_rend_base;
EXT_REND *ext_rends;

_ld wvt$l_page_length = t_page;
_ld wvt$l_column_width =  t_width;
_ld wvt$l_transcript_size = t_transcript;
_ld wvt$l_transcript_top = 1;	/* transcript is empty */
_ld wvt$l_defer_count = 0;

s_code_cell = sizeof( unsigned char );
s_rend_cell = sizeof( REND );
s_code_base = sizeof( unsigned char * );
s_rend_base = sizeof( REND * );
s_widths = sizeof( unsigned short int );
s_rendits = sizeof( unsigned char );

s_ext_rend_cell = sizeof( EXT_REND );
s_ext_rend_base = sizeof( EXT_REND * );

len = t_page + t_transcript;
array_size = len * t_width;

/*
 * Allocate space for all arrays
 */

_ld wvt$a_code_cells = (unsigned char *) XtCalloc( array_size, s_code_cell );
_ld wvt$a_rend_cells = (REND *) XtCalloc( array_size, s_rend_cell );
_ld wvt$a_trans_code_base = (unsigned char **) XtMalloc( len * s_code_base );
_ld wvt$a_trans_rend_base = (REND **) XtMalloc( len * s_rend_base );
_ld wvt$w_trans_widths = (unsigned short int *) XtMalloc( len * s_widths );
_ld wvt$b_trans_rendits = (unsigned char *) XtMalloc( len * s_rendits );
_ld wvt$a_ext_rend_cells = (EXT_REND *) XtCalloc( array_size, s_ext_rend_cell );
_ld wvt$a_trans_ext_rend_base = (EXT_REND **) XtMalloc( len * s_ext_rend_base );

/*
 * NOTE:
 *
 *	The cell and rendition arrays used in this parser are all
 *	referenced as one-based arrays (even though C is zero based).
 *	To save a little space, when the arrays are allocated by
 *	the driver, no room is reserved for the zero element.  Thus
 *	the following pointer-to-array point to the byte preceding the actual
 *	cell/rendition arrays.  Be careful about this (never reference
 *	element zero).
 *
 *	Within each array, the transcript comes before the main display.
 *	The transcript line arrays start at the top of the transcript, not
 *	at one line above the top of the transcript, since within the
 *	parser lines are always referenced relative to the start of the
 *	main display.
 *	
 */

codes = _ld wvt$a_code_cells - 1;
rends = _ld wvt$a_rend_cells - 1;
ext_rends = _ld wvt$a_ext_rend_cells - 1;

for (y = 0; y < len; y++) {

    _ld wvt$a_trans_code_base[y] = codes; codes += t_width;
    _ld wvt$a_trans_rend_base[y] = rends; rends += t_width;
    _ld wvt$a_trans_ext_rend_base[y] = ext_rends; ext_rends += t_width;
    _ld wvt$w_trans_widths[y] = t_width;
    _ld wvt$b_trans_rendits[y] = 0;

    }

_ld wvt$a_code_base = &_ld wvt$a_trans_code_base[ t_transcript - 1 ];
_ld wvt$a_rend_base = &_ld wvt$a_trans_rend_base[ t_transcript - 1 ];
_ld wvt$a_ext_rend_base = &_ld wvt$a_trans_ext_rend_base[ t_transcript - 1 ];
_ld wvt$w_widths = &_ld wvt$w_trans_widths[ t_transcript - 1 ];
_ld wvt$b_rendits = &_ld wvt$b_trans_rendits[ t_transcript - 1 ];

_ld wvt$l_page_length = t_page;
_ld wvt$l_column_width = t_width;
_ld wvt$l_transcript_size = t_transcript;

pars_init(ld);	/* Init Parser State */
}

/**************************************************************************/
Boolean xresize(ld,t_width,t_page,t_transcript,resize_flag)
/**************************************************************************/
                                
wvtp ld;                        
/*ALPHA t_width and t_page should be long int */
long int t_width, t_page;
int t_transcript, resize_flag;

{

int y, len, array_size, s_code_cell, s_rend_cell, s_code_base, s_rend_base,
	s_widths, s_rendits, top, bottom, width, temp;
unsigned char *new_code_cells, **new_trans_code_base, **new_code_base,
	*new_trans_rendits, *new_rendits, *codes;
REND *new_rend_cells, **new_trans_rend_base, **new_rend_base, *rends;
unsigned short int *new_trans_widths, *new_widths;
int s_ext_rend_cell, s_ext_rend_base;
EXT_REND *new_ext_rend_cells, **new_trans_ext_rend_base, **new_ext_rend_base,
	*ext_rends;

s_code_cell = sizeof( unsigned char );
s_rend_cell = sizeof( REND );
s_code_base = sizeof( unsigned char * );
s_rend_base = sizeof( REND * );
s_widths = sizeof( unsigned short int );
s_rendits = sizeof( unsigned char );
s_ext_rend_cell = sizeof( EXT_REND );
s_ext_rend_base = sizeof( EXT_REND * );

len = t_page + t_transcript;
array_size = len * t_width;

/*
 * Allocate new arrays
 */

new_trans_code_base = (unsigned char **) DECwTermXtMalloc( len * s_code_base );
new_trans_rend_base = (REND **) DECwTermXtMalloc( len * s_rend_base );
new_code_cells = (unsigned char *) DECwTermXtCalloc( array_size, s_code_cell );
new_rend_cells = (REND *) DECwTermXtCalloc( array_size, s_rend_cell );
new_trans_widths = (unsigned short int *) DECwTermXtMalloc( len * s_widths );
new_trans_rendits = (unsigned char *) DECwTermXtMalloc( len * s_rendits );
new_ext_rend_cells = (EXT_REND *) DECwTermXtCalloc( array_size, s_ext_rend_cell );
new_trans_ext_rend_base = (EXT_REND **) DECwTermXtMalloc( len * s_ext_rend_base );

/*
 * See if there is enough space
 */

if ( new_code_cells == NULL
	|| new_rend_cells == NULL
	|| new_trans_code_base == NULL
	|| new_trans_rend_base == NULL
	|| new_ext_rend_cells == NULL
	|| new_trans_ext_rend_base == NULL
	|| new_trans_widths == NULL
	|| new_trans_rendits == NULL )
    {
    if ( new_code_cells != NULL )
	XtFree( (char *)new_code_cells );
    if ( new_rend_cells != NULL )
	XtFree( (char *)new_rend_cells );
    if ( new_trans_code_base != NULL )
	XtFree( (char *)new_trans_code_base );
    if ( new_trans_rend_base != NULL )
	XtFree( (char *)new_trans_rend_base );
    if ( new_ext_rend_cells != NULL )
	XtFree( (char *)new_ext_rend_cells );
    if ( new_trans_ext_rend_base != NULL )
	XtFree( (char *)new_trans_ext_rend_base );
    if ( new_trans_widths != NULL )
	XtFree( (char *)new_trans_widths );
    if ( new_trans_rendits != NULL )
	XtFree( (char *)new_trans_rendits );
    return FALSE;
    }

/*
 * Initialize line arrays, including pointers into the cell arrays.
 */

codes = new_code_cells - 1;
rends = new_rend_cells - 1;
ext_rends = new_ext_rend_cells - 1;

for ( y = 0; y < len; y++ ) {

    new_trans_code_base[y] = codes; codes += t_width;
    new_trans_rend_base[y] = rends; rends += t_width;
    new_trans_ext_rend_base[y] = ext_rends; ext_rends += t_width;
    new_trans_widths[y] = t_width;
    new_trans_rendits[y] = 0;

    }

/*
 * The main display is offset by 1, so line 1 is the top line in the
 * main display and line 0 is the bottom line in the transcript.
 */
 
new_code_base = &new_trans_code_base[ t_transcript - 1 ];
new_rend_base = &new_trans_rend_base[ t_transcript - 1 ];
new_ext_rend_base = &new_trans_ext_rend_base[ t_transcript - 1 ];
new_widths = &new_trans_widths[ t_transcript - 1 ];
new_rendits = &new_trans_rendits[ t_transcript - 1 ];

/*
 * Figure out the first and last lines that need to be preserved in the
 * new display list, based on whether or not this is a conforming resize.
 * The transcript (the part that's been written to, that is), is always
 * preserved.
 */

if ( _ld wvt$l_transcript_top < 1 - t_transcript )
	_ld wvt$l_transcript_top = 1 - t_transcript;

top = _ld wvt$l_transcript_top;

if ( resize_flag == 1 )
    bottom = 0;		/* bottom line in transcript */
else
    bottom = ( t_page < _ld wvt$l_page_length ) ?
		t_page : _ld wvt$l_page_length;

width = ( t_width < _ld wvt$l_column_width ) ?
		t_width : _ld wvt$l_column_width;

for ( y = top; y <= bottom; y++ )
    {
    memcpy( new_code_base[y]+1, _ld wvt$a_code_base[y]+1, width * s_code_cell );
    memcpy( new_rend_base[y]+1, _ld wvt$a_rend_base[y]+1, width * s_rend_cell );
    memcpy( new_ext_rend_base[y]+1, _ld wvt$a_ext_rend_base[y]+1,
	width * s_ext_rend_cell );
    if ( _ld wvt$w_widths[y] < _ld wvt$l_column_width )
	new_widths[y] = ( t_width + 1 ) / 2;
    else
	new_widths[y] = t_width;
    new_rendits[y] = _ld wvt$b_rendits[y];
    }

/*
 * Free the old arrays and point to the new ones
 */

XtFree( (char *)_ld wvt$a_code_cells );
XtFree( (char *)_ld wvt$a_rend_cells );
XtFree( (char *)_ld wvt$a_trans_code_base );
XtFree( (char *)_ld wvt$a_trans_rend_base );
XtFree( (char *)_ld wvt$w_trans_widths );
XtFree( (char *)_ld wvt$b_trans_rendits );
XtFree( (char *)_ld wvt$a_ext_rend_cells );
XtFree( (char *)_ld wvt$a_trans_ext_rend_base );
   
_ld wvt$a_code_cells = new_code_cells;
_ld wvt$a_rend_cells = new_rend_cells;
_ld wvt$a_trans_code_base = new_trans_code_base;
_ld wvt$a_trans_rend_base = new_trans_rend_base;
_ld wvt$a_ext_rend_cells = new_ext_rend_cells;
_ld wvt$a_trans_ext_rend_base = new_trans_ext_rend_base;
_ld wvt$w_trans_widths = new_trans_widths;
_ld wvt$b_trans_rendits = new_trans_rendits;

_ld wvt$a_code_base = new_code_base;
_ld wvt$a_rend_base = new_rend_base;
_ld wvt$a_ext_rend_base = new_ext_rend_base;
_ld wvt$w_widths = new_widths;
_ld wvt$b_rendits = new_rendits;

_ld wvt$l_right_margin = t_width;
if ( _ld wvt$l_bottom_margin == _ld wvt$l_page_length
  || _ld wvt$l_bottom_margin > t_page )
    _ld wvt$l_bottom_margin = t_page;

_ld wvt$l_page_length = t_page;
_ld wvt$l_column_width = t_width;
_ld wvt$l_transcript_size = t_transcript;

if ( _ld wvt$l_actv_line > _ld wvt$l_page_length )
    _ld wvt$l_actv_line = _ld wvt$l_page_length;

if ( _ld wvt$l_actv_column > _ld wvt$l_column_width )
    _ld wvt$l_actv_column = _ld wvt$l_column_width;

_ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;

_ld wvt$a_cur_cod_ptr = 0;

if (!resize_flag) _ld wvt$l_save_column = _ld wvt$l_save_line = 1;

temp = _ld wvt$l_bottom_margin - _ld wvt$l_top_margin - 1;
if ( _ld wvt$l_defer_limit > temp )
    _ld wvt$l_defer_max = temp;
else
    _ld wvt$l_defer_max = _ld wvt$l_defer_limit;

return TRUE;
}
