/* #module WV_CURSOR "X03-306" */
/*
 *  Title:	WV_CURSOR
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
 *  This module contains routines to control and report cursor position.
 *  
 *  Routines contained in this module:
 *
 *	xcuu    - cursor up
 *	xcud    - cursor down
 *	xcuf    - cursor forward
 *	xcub    - cursor backward
 *	xcup    - cursor position absolute
 *	xcpr    - cursor position report
 *
 *  Author:	Frederick G. Kleinsorge
 *		Low-End Workstation Graphics Engineering
 *
 *		Adapted from code for the PRO Series Terminal
 *		Emulator.
 *  
 *  
 *  Modification History:
 *  
 *  Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 * Bob Messenger	16-Jan-1989	X1.1-1
 *	- Moved many ld fields to common area
 *
 *  MSL0007	Michael Leibow 14-Jul-1988
 *  o xcuf did not work properly when active line's line width was smaller
 *    then right_margin.
 * 
 *  FGK0006	Frederick G. Kleinsorge	17-Apr-1987
 *  
 *  o Mass edit symbols to use VMS naming standards
 *  
 *  FGK0005	Frederick G. Kleinsorge	16-Apr-1987
 *  
 *  o Change the ld data type
 *  
 *  FGK0004	Frederick G. Kleinsorge	05-Mar-1987
 *  
 *  o V3.2
 *  
 *  FGK0003	Frederick G. Kleinsorge	27-Feb-1987
 *  
 *  o Remove discrete display flushes
 *  
 *  FGK0002	Frederick G. Kleinsorge	22-Jul-1986
 *  
 *  o Update version to X04-017
 *  
 *  FGK0001	Frederick G. Kleinsorge	10-Jun-1986
 *  
 *  o Add LEFT and RIGHT margins.
 *
 *  
 */

#include "wv_hdr.h"


/**********************/
xcuu(ld) /* Cursor Up */
/**********************/

wvtp ld;

{

int n;

 _ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;

 if (_cld wvt$l_parms[0] == 0) n = 1;
 else n = _cld wvt$l_parms[0];

 if (_ld wvt$l_top_margin <= _ld wvt$l_actv_line)
   {
    if (_ld wvt$l_top_margin <= _ld wvt$l_actv_line - n)
	_ld wvt$l_actv_line -= n;
    else _ld wvt$l_actv_line = _ld wvt$l_top_margin;
   }
 else
   {
    if (1 <= _ld wvt$l_actv_line - n) _ld wvt$l_actv_line -= n;
    else _ld wvt$l_actv_line = 1;
   }

 /* new line may have a different length - adjust column */

 if (line_width(_ld wvt$l_actv_line) < _ld wvt$l_actv_column)
	_ld wvt$l_actv_column = line_width(_ld wvt$l_actv_line);

 WVT$KEY_PRESSED(ld); /* clear key pressed flag */
}

/************************/
xcud(ld) /* Cursor Down */
/************************/

wvtp ld;

{

int n;

 _ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;

 if (_cld wvt$l_parms[0] == 0) n = 1;
 else n = _cld wvt$l_parms[0];

 if (_ld wvt$l_actv_line <= _ld wvt$l_bottom_margin)
   {
    if (_ld wvt$l_actv_line + n <= _ld wvt$l_bottom_margin)
	_ld wvt$l_actv_line += n;
    else _ld wvt$l_actv_line = _ld wvt$l_bottom_margin;
   }
 else
   {
    if (_ld wvt$l_actv_line + n <= _ld wvt$l_page_length)
	_ld wvt$l_actv_line += n;
    else _ld wvt$l_actv_line = _ld wvt$l_page_length;
   }

 /* new line may have a different length - adjust column */

 if (line_width(_ld wvt$l_actv_line) < _ld wvt$l_actv_column)
	_ld wvt$l_actv_column = line_width(_ld wvt$l_actv_line);

 WVT$KEY_PRESSED(ld); /* clear key pressed flag */

}

/***************************/
xcuf(ld) /* Cursor Forward */
/***************************/

wvtp ld;

{

int n;

 _ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;

 if (_cld wvt$l_parms[0] == 0) n = 1;
 else n = _cld wvt$l_parms[0];

 if (_ld wvt$l_actv_column <= _ld wvt$l_right_margin
     && line_width(_ld wvt$l_actv_line) >= _ld wvt$l_right_margin)
   {
    if (_ld wvt$l_actv_column + n < _ld wvt$l_right_margin)
	_ld wvt$l_actv_column += n;
    else _ld wvt$l_actv_column = _ld wvt$l_right_margin;
   }
 else
   {
    if (_ld wvt$l_actv_column + n < line_width(_ld wvt$l_actv_line))
	_ld wvt$l_actv_column += n;
    else _ld wvt$l_actv_column = line_width(_ld wvt$l_actv_line);
   }

}

/****************************/
xcub(ld) /* Cursor Backward */
/****************************/

wvtp ld;

{

int n;

 _ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;

 if (_cld wvt$l_parms[0] == 0) n = 1;
 else n = _cld wvt$l_parms[0];

 if (_ld wvt$l_left_margin <= _ld wvt$l_actv_column)
   {
    if (_ld wvt$l_left_margin < _ld wvt$l_actv_column - n)
	_ld wvt$l_actv_column -= n;
    else _ld wvt$l_actv_column = _ld wvt$l_left_margin;
   }
 else
   {
    if (1 < _ld wvt$l_actv_column - n) _ld wvt$l_actv_column -= n;
    else _ld wvt$l_actv_column = 1;
   }

}

/***************************************/
xcup(ld) /* Cursor Position (absolute) */
/***************************************/

wvtp ld;

{

int y, x;

 _ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;

 y = _cld wvt$l_parms[0];
 x = _cld wvt$l_parms[1];

 if (y == 0) y = 1;
 if (x == 0) x = 1;

 if (_ld wvt$l_vt200_specific_flags & vts1_m_origin_mode)
   {

    if ((_ld wvt$l_top_margin-1) + y <= _ld wvt$l_bottom_margin)
	_ld wvt$l_actv_line = (_ld wvt$l_top_margin-1) + y;
    else _ld wvt$l_actv_line = _ld wvt$l_bottom_margin;

    if ((_ld wvt$l_left_margin-1) + x <= _ld wvt$l_right_margin)
	_ld wvt$l_actv_column = (_ld wvt$l_left_margin-1) + x;
    else _ld wvt$l_actv_column = _ld wvt$l_right_margin;

   }
 else
   {

    if (y <= _ld wvt$l_page_length) _ld wvt$l_actv_line = y;
    else _ld wvt$l_actv_line = _ld wvt$l_page_length;

    if (x <= line_width(_ld wvt$l_actv_line)) _ld wvt$l_actv_column = x;
    else _ld wvt$l_actv_column = line_width(_ld wvt$l_actv_line);

   }

 WVT$KEY_PRESSED(ld); /* clear key pressed flag */

}


/***********************************/
xcpr(ld) /* Cursor Position Report */
/***********************************/

wvtp ld;

{

int y, x;

 if (_ld wvt$l_vt200_specific_flags & vts1_m_origin_mode)
   {
    y = (_ld wvt$l_actv_line - _ld wvt$l_top_margin) + 1;
    x = (_ld wvt$l_actv_column - _ld wvt$l_left_margin) + 1;
   }
 else
   {
    y = _ld wvt$l_actv_line;
    x = _ld wvt$l_actv_column;
   }

 WVT$CURSOR_POSITION_REPORT(ld,y,x);

}
