/* #module WV_VT52 "X3.0-5" */
/*
 *  Title:	WV_VT52
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
 *  Routines to handle VT52 emulation
 *  
 *
 *  Routines contained in this module:
 *
 *	x52escseq - dispatch table for VT52 escape sequences
 *	x52cup	  - VT52 cursor address sequence
 *
 *  Author:	Frederick G. Kleinsorge
 *		Low-End Workstation Graphics Engineering
 *
 *		Adapted from code for the PRO Series Terminal
 *		Emulator.
 *
 * Revisions:
 *
 *  Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 * Aston Chan		17-Dec-1991	V3.1
 *	- I18n code merge
 *
 * Bob Messenger	 7-Jul-1990	X3.0-5
 *	- Add printer port support.
 *
 * Bob Messegner	23-Apr-1990	X3.0-2
 *	- If a cursor position is set to beyond the page boundary, leave that
 *	  coordinate unchanged.
 *
 * Bob Messenger	22-Mar-1990	V2.1
 *	- Back out X2.0-13 change altogether: don't limit to 24 rows,
 *	  80 columns.
 *
 * Bob Messenger	12-Mar-1990	V2.1
 *	- Fix bug introduced in X2.0-13: limit to 24 rows, 80 columns,
 *	  not 24 columns, 80 rows!
 *
 * Bob Messenger	28-May-1989	X2.0-13
 *	- Limit cursor positioning to 24 x 80.
 *
 * Bob Messenger	16-Jan-1989	X1.1-1
 *	- Moved many ld fields into common area.
 *
 *  FGK0004	Frederick G. Kleinsorge	21-Apr-1987
 *  
 *  o Mass edit of symbols
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
 */

#include "wv_hdr.h"


/******************************************************/
x52escseq(ld, code) /* VT52 escape seq dispatch table */
/******************************************************/

wvtp ld;
int code;

{
_cld wvt$l_parms[0] = 0;
_cld wvt$l_parms[1] = 0;
_cld wvt$b_parmcnt = 0;
_cld wvt$b_privparm = 0;

switch (code) {

  case 60:

                                       /* Enter ANSI mode (exit VT52 mode) */
    _cld wvt$l_vt200_flags |= vt1_m_ansi_mode; 
    _cld wvt$b_conformance_level = LEVEL1; 
    WVT$SET_TERMINAL_MODE(ld);
    xstr(ld);				      /* Do a soft terminal reset */

     break;

  case 61:			/* Enter alternate keypad mode */

    _cld wvt$l_vt200_common_flags |= vtc1_m_keypad_mode;
    WVT$SET_MISC_MODES( ld );
    break;

  case 62:			/* Exit Keypad mode */

    _cld wvt$l_vt200_common_flags &= ~vtc1_m_keypad_mode;
    WVT$SET_MISC_MODES( ld );
    break;

  case 129:			/* Cursor UP */

    xcuu(ld);
    break;

  case 130:			/* Cursor DOWN */

    xcud(ld);
    break;

  case 131:			/* Cursor FORWARD (right) */

    xcuf(ld);
    break;

  case 132:			/* Cursor BACK (left) */

    xcub(ld);
    break;

  case 134:			/* Enter graphics mode */

	/* LINE DRAWING set is in G3	*/
    if ( _cld wvt$l_ext_flags & vte1_m_tomcat )
	_ld wvt$b_gl = 3;	/* Use LINE DRAWING set */
    else
    _ld wvt$b_gl = 1;		/* Use LINE DRAWING set */
    break;

  case 135:			/* Exit graphics mode (ASCII) */

    _ld wvt$b_gl = 0;
    break;

  case 136:			/* Position cursor HOME */

    xcup(ld);
    break;

  case 137:			/* Reverse linefeed */

    xri(ld);
    break;

  case 138:			/* Erase display */

    xed(ld);
    break;

  case 139:			/* Erase Line */

    xel(ld);
    break;

  case 150:			/* Print Cursor Line */
    print_lines( ld, 1, _mld wvt$l_actv_line, line_width(_mld wvt$l_actv_line),
      _mld wvt$l_actv_line, '\n' );
    break;

  case 151:			/* Enter printer controller mode */
    WVT$ENTER_PRINT_CONTROLLER_MODE( ld );
    break;

  case 152:			/* Exit printer controller mode */
    WVT$EXIT_PRINT_CONTROLLER_MODE( ld );
    break;

  case 153:			/* Direct Cursor Address (defer) */

    _cld wvt$l_vt200_flags |= vt1_m_vt52_cursor_seq;
    _cld wvt$b_last_event = R_CONTINUE;
    break;

  case 154:			/* Identify */

    WVT$TERMINAL_TRANSMIT(ld,"\33/Z");
    break;

  case 157:			/* Print screen */
    DECwTermPrintTextScreen( ld );
    break;

  case 158:			/* Enter auto print mode */
    WVT$ENTER_AUTO_PRINT_MODE( ld );
    break;

  case 159:			/* Exit auto print mode */
    WVT$EXIT_AUTO_PRINT_MODE( ld );
    break;

  }

}

/*************************************/
x52cup(ld) /* VT52 cursor addressing */
/*************************************/

wvtp ld;

{

if ( _cld wvt$l_parms[0] > _ld wvt$l_page_length )
     _cld wvt$l_parms[0] = _ld wvt$l_actv_line;
if ( _cld wvt$l_parms[1] > _ld wvt$l_column_width )
     _cld wvt$l_parms[1] = _ld wvt$l_actv_column;

xcup(ld);	/* Execute ANSI cursor position */

_cld wvt$l_vt200_flags &= ~vt1_m_vt52_cursor_seq;

}

