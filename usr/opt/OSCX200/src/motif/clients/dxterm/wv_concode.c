/* #module WV_CONCODE "X03-310" */
/*
 *  Title:	WV_CONCODE
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
 *
 *	Handle control code functions.
 *  
 *	Routines contained in this module:
 *
 *		xcontrol     - dispatch table for control codes
 *		xht          - horizontal tabulation
 *		xlf          - line feed
 *		xind         - index
 *		xri          - Reverse index
 *		xnel         - new line
 *
 *  Author:	Frederick G. Kleinsorge
 *		Low-End Workstation graphics Engineering
 *
 *		Adapted from code for the PRO Series Terminal
 *		Emulator.
 *
 *  Revision History:
 *
 *  Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 *  Eric Osman		11-May-1993	VXT V2.0
 *	- Allow #nn (hex) for specifying non-printable chars in answerback
 *	  message
 *
 * Aston Chan		17-Dec-1991	V3.1
 *	- I18n code merge
 *  
 * Bob Messenger	26-Aug-1990	X3.0-6
 *	- Fix test for being in auto-print mode.  (Work around compiler bug.)
 *
 * Bob Messenger	 6-Jul-1990	X3.0-5
 *	- Implemented auto-print mode.
 *
 * Bob Messenger	16-Jan-1989	X1.1-1
 *	- Moved many ld fields to common area.
 *
 * Mike Leibow		16-Aug-1988	X0.4-44
 * 	- no longer send a DA report in level 2 or level 3 mode in response
 *	  to DECID.
 *
 * Mike Leibow		07-Jun-1988
 *      - Prepared code for status line.  Some ld fields are now accessed
 *        by _cld instead of -ld.
 *
 * Tom Porcher		16-Apr-1988	X0.4-10
 *	- Added missing fourth parameter to one call to WVT$UP_SCROLL.
 *
 *  RDM0011	Robert D. Messenger 17-Dec-1987
 *
 *  o Add code to ignore APC sequences.
 *
 *  FGK0010	Frederick G. Kleinsorge	29-Jul-1987
 *  
 *  o Add runaway LF check to XLF code.
 *  
 *  FGK0009	Frederick G. Kleinsorge	22-Jul-1987
 *  
 *  o Add deffered up scroll logic to xlf()
 *  
 *  FGK0008	Frederick G. Kleinsorge	17-Apr-1987
 *  
 *  o Mass edit of names to confrom to VMS standards
 *  
 *  FGK0007	Frederick G. Kleinsorge	16-Apr-1987
 *  
 *  o Change ld data type to use macro "wvtp"
 *  
 *  FGK0006	Frederick G. Kleinsorge	05-Mar-1987
 *  
 *  o V3.2
 *  
 *  FGK0005	Frederick G. Kleinsorge	27-Feb-1987
 *  
 *  o Remove the discrete display flush code
 *
 *  FGK0004	Frederick G. Kleinsorge	09-Sep-1986
 *  
 *  o Conditionalize out the PANDA functionality
 *
 *  FGK0003	Frederick G. Kleinsorge	14-Aug-1986
 *  
 *  o Change XHT() to convert NULLs to SPACEs if needed.
 *
 *  FGK0002	Frederick G. Kleinsorge	22-Jul-1986
 *  
 *  o Update version to X04-017
 *
 *  FGK0001	Frederick G. Kleinsorge	10-Jun-1986
 *  
 *  o Added LEFT and RIGHT margins.
 *
 */

#include "wv_hdr.h"


/*************************************************/
xcontrol(ld,code) /* Control Code dispatch table */
/*************************************************/

wvtp ld;
unsigned int code;

{

switch(code)

  {



  case C0_ENQ:				/* ENQ, Send answerback */

    if (_cld wvt$l_vt200_common_flags & vtc1_m_auto_answerback)
	transmit_answerback (ld,_cld wvt$b_answerback);

    break;



  case C0_BEL:				/* Ring bell */

    if (_cld wvt$l_flags & vt4_m_bell) WVT$BELL(ld);
    break;



  case C0_BS:				/* Backspace */

    _ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;

    if (( _cld wvt$l_ext_flags & vte1_m_hebrew ) &&
	( _cld wvt$l_ext_specific_flags & vte2_m_rtl )) {
	if (_ld wvt$l_actv_column < line_width(_ld wvt$l_actv_line))
	    _ld wvt$l_actv_column +=1;
    }
    else
    if ((_ld wvt$l_left_margin != _ld wvt$l_actv_column) &&
	(_ld wvt$l_actv_column > 1)) _ld wvt$l_actv_column -=1;

    break;



  case C0_HT:				/* Tab */

    xht(ld);

    break;



  case C0_LF:				/* line feed,    */
  case C0_VT:				/* vertical tab, */
  case C0_FF:				/* form feed     */

    /*
     * Auto print the line we are leaving if auto print mode is enabled and
     * we are not in the status line.
     *
     * Work around a compiler bug (VAX C V3.1-051 on VMS): it
     * executed the print_lines statement when auto print mode was
     * not enabled, rather than when it was enabled.
     */

    {
    int auto_print_enabled, status_display_active;

    auto_print_enabled = ( _cld wvt$w_print_flags &
      pf1_m_auto_print_mode ) != 0;
    status_display_active = ( _cld wvt$l_vt200_common_flags
      & vtc1_m_actv_status_display ) != 0;

    if ( auto_print_enabled && ! status_display_active )
	{
	print_lines( ld, 1, _mld wvt$l_actv_line,
	  line_width(_mld wvt$l_actv_line), _mld wvt$l_actv_line, code );
	}
    }

    xlf(ld);

    break;



  case C0_CR:				/* Carrage Return */

    if (( _cld wvt$l_ext_flags & vte1_m_hebrew ) &&
	( _cld wvt$l_ext_specific_flags & vte2_m_rtl ))
	    _ld wvt$l_actv_column = line_width(_ld wvt$l_actv_line);
    else
    if (_ld wvt$l_actv_column < _ld wvt$l_left_margin)
		_ld wvt$l_actv_column = 1;
    else	_ld wvt$l_actv_column = _ld wvt$l_left_margin;

    _ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;

    break;



  case C0_SO:				/* Shift-Out */

    _ld wvt$b_gl = 1;

    break;



  case C0_SI:				/* Shift-In */

    _ld wvt$b_gl = 0;

    break;



  case C0_DC1:				/* XON */

    _cld wvt$l_vt200_common_flags &= ~vtc1_m_kbd_action_mode;	/* Use KAM for this */

    break;



  case C0_DC3:				/* XOFF */

    _cld wvt$l_vt200_common_flags |= vtc1_m_kbd_action_mode;	/* Use KAM for this */

    break;



  case C0_SUB:				/* Sub */

/*
 * Note:
 *
 *	The VWS ASCII fonts have the VT100 line
 *	drawing set loaded into positions 0-39
 *	of the font files.  To image the C0_SUB
 *	character (reverse question mark) I must
 *	use one of the unused character positions
 *	-- in this case 128.  Routines that extract
 *	data from the cell display for re-transmission
 *	(e.g. PRINT) should translate the 128 into a
 *	C0_SUB character.
 *
 *	Force the display of the data here.
 *
 */

    xinsertchar(ld, 128);

    display_segment(ld, _ld wvt$l_actv_line, _ld wvt$l_disp_pos,
			_ld wvt$l_actv_column - _ld wvt$l_disp_pos);


    _ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;

    break;



  case C1_IND:				/* Index */

    xind(ld);

    break;



  case C1_NEL:				/* New Line */

    xnel(ld);

    break;



  case C1_HTS:				/* Set Horizontal Tab Stop */

    if (!(_cld wvt$l_vt200_common_flags & vtc1_m_feature_lock)) 
	_cld wvt$b_tab_stops[_ld wvt$l_actv_column] = TRUE;

    break;



  case C1_RI:				/* Reverse Index */

    xri(ld);
    break;



  case C1_SS2:				/* Single Shift 2 */

    _ld wvt$b_single_shift = SS_2;

    break;


  case C1_SS3:				/* Single Shift 3 */

    _ld wvt$b_single_shift = SS_3; 

    break;


  case C1_ST:				/* String Terminator */

    _cld wvt$b_in_dcs = FALSE;

    break;


  case 154:				/* DECID Get Terminal ID */

    if (_cld wvt$b_conformance_level <= LEVEL1) xda1rpt(ld);

    break;



  case C1_OSC:				/* Process OSC via DCS path */

    _cld wvt$b_in_dcs = C1_OSC;
    _cld wvt$b_osc_type  = 0;
    _cld wvt$b_osc_state = 0;

    break;



  case C1_SOS:				       /* Start of string	*/
  case C1_PM:				/* Process Priv Msg ignore it */
  case C1_APC:				/* Process APC ignore it */	/*RDM*/

    _cld wvt$b_in_dcs = IGNORE_DCS;

    break;



  default:

    break;
  }

}


/*****************************************/
xht(ld) /* execute Horizontal Tabulation */
/*****************************************/

wvtp ld;

/*
 *  
 *  Move the position forward until the next tab stop. Or, if
 *  to the left of the right margin -- until the right margin
 *  or edge of page.  Or if past the right margin -- until the
 *  edge of the page.
 *  
 *  The LEFT and RIGHT margins are only in effect (that is, they
 *  can be different than the 1 x Page_Width) if in vertical split
 *  screen mode -- in which case there can be NO double width lines.
 *  
 *  Convert NULL characters into SPACE if needed (e.g. flag as written).
 */

{

_ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;

 do
  {

  if ((_ld wvt$l_actv_column == _ld wvt$l_right_margin) ||
      (_ld wvt$l_actv_column >= line_width(_ld wvt$l_actv_line)))
	{
	 _ld wvt$b_disp_eol = TRUE; /* Force a display to EOL */
	 break;
	}
  else
	{
	 if (!character(_ld wvt$l_actv_line, _ld wvt$l_actv_column))
		character(_ld wvt$l_actv_line, _ld wvt$l_actv_column) = SPACE;
	 _ld wvt$l_actv_column++;
	}

  } while (!(_cld wvt$b_tab_stops[_ld wvt$l_actv_column]));
}


/*****************************/
xlf(ld) /* execute Line Feed */
/*****************************/

wvtp ld;

{

 _ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;

 if (_cld wvt$l_vt200_flags & vt1_m_new_line_mode)
	if (( _cld wvt$l_ext_flags & vte1_m_hebrew ) &&
	    ( _cld wvt$l_ext_specific_flags & vte2_m_rtl )) 
	    _ld wvt$l_actv_column = _ld wvt$l_right_margin;
	else
	_ld wvt$l_actv_column = _ld wvt$l_left_margin;

 if (_ld wvt$l_actv_line == _ld wvt$l_bottom_margin)
	{
	 if ( (_cld wvt$l_vt200_flags_3 & vt3_m_defer_up_scroll)
		&& (_ld wvt$l_defer_max))
	   {
	    _ld wvt$l_defer_count += 1;
	    WVT$UP_SCROLL(ld, _ld wvt$l_top_margin, 1, 1);

	    if (_ld wvt$l_defer_count >= _ld wvt$l_defer_max)
		{
		s_execute_deferred( ld );
		}
	   }
	 else
	   {
	    WVT$UP_SCROLL(ld, _ld wvt$l_top_margin, 1, 0);
	   }
	}
 else if (_ld wvt$l_actv_line < _ld wvt$l_page_length)
	_ld wvt$l_actv_line++;

/*
 * Get new line width.  The new line may have a different
 * length - adjust column if needed
 */

 _cld wvt$l_actv_width  = line_width(_ld wvt$l_actv_line);

 if (_ld wvt$l_right_margin < _cld wvt$l_actv_width)
	_cld wvt$l_actv_width  = _ld wvt$l_right_margin;

 if (_cld wvt$l_actv_width < _ld wvt$l_actv_column)
	_ld wvt$l_actv_column = _cld wvt$l_actv_width;

 WVT$KEY_PRESSED(ld);	/* clear key pressed flag */
}


/**************************/
xind(ld) /* execute Index */
/**************************/

wvtp ld;

{

 _ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;

 if (_ld wvt$l_actv_line == _ld wvt$l_bottom_margin)
	WVT$UP_SCROLL(ld, _ld wvt$l_top_margin, 1, 0);
 else if (_ld wvt$l_actv_line < _ld wvt$l_page_length)
	_ld wvt$l_actv_line++;

 /* new line may have a different length - adjust column */

 if (line_width(_ld wvt$l_actv_line) < _ld wvt$l_actv_column)
	_ld wvt$l_actv_column = line_width(_ld wvt$l_actv_line);

 WVT$KEY_PRESSED(ld);	/* clear key pressed flag */

}

#if EXTENDED_PANDA
/************************************/
xdecfi(ld) /* execute Forward Index */
/************************************/

wvtp ld;

{

 _ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;

 if (_ld wvt$l_actv_column == _ld wvt$l_right_margin)
	WVT$LEFT_SCROLL(ld, _ld wvt$l_left_margin, 1);
 else if (_ld wvt$l_actv_column < line_width(_ld wvt$l_actv_line))
	_ld wvt$l_actv_column++;

}
#endif

/*********************************/
xri(ld) /* execute Reverse Index */
/*********************************/

wvtp ld;

{

 _ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;

 if (_ld wvt$l_actv_line == _ld wvt$l_top_margin)
	WVT$DOWN_SCROLL(ld, _ld wvt$l_top_margin, 1);
 else if (_ld wvt$l_actv_line > 1)
	_ld wvt$l_actv_line--;

 /* new line may have a different length - adjust column */

 if (line_width(_ld wvt$l_actv_line) < _ld wvt$l_actv_column)
 	_ld wvt$l_actv_column = line_width(_ld wvt$l_actv_line);

 WVT$KEY_PRESSED(ld);	/* clear key pressed flag */

}


#if EXTENDED_PANDA
/*********************************/
xdecbi(ld) /* execute Back Index */
/*********************************/

wvtp ld;

{

 _ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;

 if (_ld wvt$l_actv_column == _ld wvt$l_left_margin)
	WVT$RIGHT_SCROLL(ld, _ld wvt$l_right_margin, 1);
 else if (_ld wvt$l_actv_column > 1)
	_ld wvt$l_actv_column--;

}
#endif

/*****************************/
xnel(ld) /* execute New Line */
/*****************************/

wvtp ld;

{

 _ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;

 _ld wvt$l_actv_column = _ld wvt$l_left_margin; /* <cr> */
 xlf(ld); /* Execute a Line feed */

}
