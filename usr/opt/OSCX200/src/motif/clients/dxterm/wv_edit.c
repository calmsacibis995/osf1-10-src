/* #module WV_EDIT "X03-317" */
/*
 *  Title:	WV_EDIT
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © 1985, 1993						     |
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
 *  This module contains routines to handle display editing functions.
 *  
 *
 *  Routines contained in this module:
 *
 *	xech   - erase character
 *	xdch   - delete character
 *	xich   - insert character
 *	xsel   - selective erase in line
 *	xel    - erase in line
 *	xdl    - delete line
 *	xil    - insert line
 *	xsed   - selective erase in display
 *	xed    - erase in display
 *
 ****************************************************************
 *
 *		 (Conditionalized)
 *
 *	xcad   - change in display			(Level 3)
 *	xcal   - change in line				(Level 3)
 *	xrad   - Reverse in display			(Level 3)
 *	xral   - Reverse in line			(Level 3)
 *	xera   - erase rectangular area			(Level 3)
 *	xsera  - selective erase rectangular area	(Level 3)
 *	xfra   - fill rectangular area			(Level 3)
 *
 ****************************************************************
 *
 *	selera_segment	- selective erase segment (common routine)
 *	change_segment	- change segment (common routine)
 *
 *  Author:	Frederick G. Kleinsorge
 *		Low-End Workstation Graphics Engineering
 *
 *		Adapted from code for the PRO Series Terminal
 *		Emulator.
 *
 *  Revision History:
 *
 *  Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 * Aston Chan		17-Dec-1991	V3.1
 *	- I18n code merge
 *
 * Bob Messenger	 2-Apr-1989	X2.0-5
 *	- move wvt$l_column_width to specific area
 *
 * Bob Messenger	16-Jan-1989	X1.1-1
 *	- moved many ld fields into common area
 *
 * Mike Leibow		16-Aug-1988	X0.4-44
 *	- fixed invalid parameters bug in xel().
 *
 * Mike Leibow          07-Jun-1988
 *      - Prepared code for status line.  Some ld fields are now accessed
 *        by _cld instead of _ld.
 *
 * Tom Porcher		16-Apr-1988	X0.4-10
 *	- Added missing fourth parameter to call to WVT$UP_SCROLL().
 *
 *		Tom Porcher	20-Jan-1988
 *
 *  o  Made all WVT$ERASE_DISPLAY()s the same upper case.
 *
 *  FGK0017	Frederick G. Kleinsorge	11-May-1987
 *  
 *  o Finish DECCAEM mode coding.
 *
 *  FGK0016	Frederick G. Kleinsorge	08-May-1987
 *  
 *  o Remove extranious clear of masks in build_masks routine
 *
 *  FGK0015	Frederick G. Kleinsorge	17-Apr-1987
 *  
 *  o Mass edit VMS compliant symbol names
 *
 *  FGK0014	Frederick G. Kleinsorge	16-Apr-1987
 *  
 *  o Change ld data type
 *
 *  FGK0013	Frederick G. Kleinsorge	25-Mar-1987
 *  
 *  o	Fix bad use of varable "n" in erase in display routine.
 *
 *  FGK0012	Frederick G. Kleinsorge	05-Mar-1987
 *  
 *  o	V3.2
 *
 *  FGK0011	Frederick G. Kleinsorge	27-Feb-1987
 *  
 *  o	Remove discrete display flushes
 *
 *  FGK0010	Frederick G. Kleinsorge	10-Feb-1987
 *  
 *  o	Backed out the erase in xdl and xil to make it scroll again.
 *
 *  FGK0009	Frederick G. Kleinsorge	20-Jan-1987
 *  
 *  o	Changed references to line_width(_ld wvt$l_actv_line) to ld wvt$l_actv_width
 *
 *  o	Changed xdl and xil to use a WVT$ERASE_DISPLAY if the active
 *	line + the delete/insert count is > to the bottom margin.
 *	This should be much faster when on a GPX, but is probably in
 *	violation of the SRM.
 *
 *  o	Change xich and xdch to do a WVT$ERASE_DISPLAY if the count +
 *	the position is > = to the end of line.  Avoid doing a meaningless
 *	data shuffle.
 *
 *  FGK0008	Frederick G. Kleinsorge	9-Sep-1986
 *  
 *  o	Removed the Level-3 editing functions due to decommit by PANDA,
 *	the code is conditionalized out.
 *
 *
 *  FGK0007	Frederick G. Kleinsorge	14-Aug-1986
 *  
 *  o	Change code to use NULL as the empty character.  Also need to change
 *	NULL characters into SPACE characters in some cases (like DECCAD).
 *
 *  FGK0006	Frederick G. Kleinsorge	22-Jul-1986
 *  
 *  o	Update version to X04-017, fix DELETE CHARACTER.  The clearing of the
 *	end of line was done RIGHT_EDGE-(n+1) instead of (RIGHT_EDGE-n)+1
 *
 *  FGK0005	Frederick G. Kleinsorge	30-Jun-1986
 *
 *  o	Change rectangular erase to look for DBLW lines.
 *
 *  FGK0004	Frederick G. Kleinsorge	27-Jun-1986
 *
 *  o	Shrink code.  Alter ERASE code to use real erase instead of
 *	spaces display for partial lines.
 *
 *  o	Get rid of DECCAEM -- cannot be implemented as the code exists
 *	now.
 *
 *  FGK0003	Frederick G. Kleinsorge	10-Jun-1986
 *  
 *  o	Add the csa_M_INVISIBLE attribute.
 *
 *  o	Add DECCAEM mode.
 *
 *  o	Add LEFT/RIGHT margins (to insert/delete line/character).
 *
 *  FGK0002	Frederick G. Kleinsorge	09-Jun-1986
 *  
 *  o	Make common erase/change/selective erase Segment routines.
 *
 *  o	Be more intellegent about start/end positions for re-display of
 *	line segments for erase functions (don't just do it to EOL).
 *
 *  FGK0001	Frederick G. Kleinsorge	06-Jun-1986
 *  
 *  o	Add the Change, Reverse, Fill and Erase Area controls.
 *
 */

#include "wv_hdr.h"


/****************************/
xech(ld) /* Erase Character */
/****************************/

wvtp ld;

{

register int n;

 edit_setup(ld);

 if (_cld wvt$l_parms[0] == 0) n = 1;
 else n = _cld wvt$l_parms[0];

 if (_cld wvt$l_actv_width < _ld wvt$l_actv_column + n)
    n = (_cld wvt$l_actv_width - _ld wvt$l_actv_column) + 1;

 WVT$ERASE_DISPLAY(
		ld,
		_ld wvt$l_actv_line,
		_ld wvt$l_actv_column,
		_ld wvt$l_actv_line,
		_ld wvt$l_actv_column+(n-1));

}

/*****************************/                   
xdch(ld) /* Delete Character */
/*****************************/

wvtp ld;

{
register int x, y, n, right_edge;

 if (!(edit_setup(ld))) return;

 y = _ld wvt$l_actv_line;

 if (_cld wvt$l_parms[0] == 0) n = 1;
 else n = _cld wvt$l_parms[0];

 if (_ld wvt$l_right_margin > _cld wvt$l_actv_width)
	right_edge = _cld wvt$l_actv_width;
 else right_edge = _ld wvt$l_right_margin;

 if (right_edge < _ld wvt$l_actv_column + n)
   {

  /*
   *	Delete character when the count + current position is > than the
   *	right margin...
   *
   *	example:
   *
   *	n			= 5
   *	wvt$l_actv_column	= 76
   *	right_edge		= 80
   *
   *	C = "cleared"
   *
   *		       +-- wvt$l_actv_column (76)
   *		       !
   *		       !       +- right_edge (80)
   *		       v       v
   *	+-------------+-+-+-+-+-+
   *	!             !C!C!C!C!C!
   *	+-------------+-+-+-+-+-+
   *		     ^           ^
   *		     !           +- wvt$l_actv_column+n (76+5 = 81)
   *		     !
   *		     +- right_edge-n (80-5 = 75)
   *
   */

   WVT$ERASE_DISPLAY(
	ld,
	_ld wvt$l_actv_line,
	_ld wvt$l_actv_column,
	_ld wvt$l_actv_line,
	_cld wvt$l_actv_width);

  }
 else
  {

 /*
  *	Delete character when the count + current position is =< the
  *	right margin...
  *
  *	example:
  *
  *	n			= 2
  *	wvt$l_actv_column	= 76
  *	right_edge		= 80
  *
  *
  *	O = "old data moved"
  *	C = "cleared"
  *
  *		       +-- wvt$l_actv_column (76)
  *		       !
  *		       !       +- right_edge (80)
  *		       v       v
  *	+-------------+-+-+-+-+-+
  *	!             !O!O!O!C!C!
  *	+-------------+-+-+-+-+-+
  *		           ^
  *		           !
  *		           +- right_edge-n (80-2 = 78)
  *
  */

   for (x = _ld wvt$l_actv_column; x <= right_edge-n; x++)
    {
     character(y,x) = character(y,x+n);
     rendition(y,x) = rendition(y,x+n);
     ext_rendition( y, x ) = ext_rendition( y, x + n );
    }

   for (x = (right_edge-n)+1; x <= right_edge; x++)
    {
     rendition(y,x) = NULL;	/* The cell is unwritten */
     character(y,x) = NULL;	/*                       */
     ext_rendition( y, x ) = NULL;
    }
   _ld wvt$b_disp_eol = TRUE;                   
    if (( _cld wvt$l_ext_flags & vte1_m_hebrew ) &&
	( _cld wvt$l_ext_specific_flags & vte2_m_rtl ))
	display_segment( ld, y, _ld wvt$l_right_margin, 0 );
   display_segment(ld, y, _ld wvt$l_actv_column, 0);

  }

}

/*****************************/
xich(ld) /* Insert Character */
/*****************************/

wvtp ld;

{

register int n, x, y, right_edge;

 if (!(edit_setup(ld))) return;

 y = _ld wvt$l_actv_line;

 if (_cld wvt$l_parms[0] == 0) n = 1;
 else n = _cld wvt$l_parms[0];

 if (_ld wvt$l_right_margin > _cld wvt$l_actv_width)
	right_edge = _cld wvt$l_actv_width;
 else right_edge = _ld wvt$l_right_margin;

 if (right_edge < _ld wvt$l_actv_column + n)
  {

 /*
  *	Insert character when the count + current position is > than the
  *	right margin...
  *
  *	example:
  *
  *	n			= 5
  *	wvt$l_actv_column	= 76
  *	right_edge		= 80
  *
  *	C = "cleared"
  *
  *		       +-- wvt$l_actv_column (76)
  *		       !
  *		       !       +- right_edge (80)
  *		       v       v
  *	+-------------+-+-+-+-+-+
  *	!             !C!C!C!C!C!
  *	+-------------+-+-+-+-+-+
  *		     ^           ^
  *		     !           +- wvt$l_actv_column+n (76+5 = 81)
  *		     !
  *		     +- right_edge-n (80-5 = 75)
  *
  */

   WVT$ERASE_DISPLAY(
	ld,
	_ld wvt$l_actv_line,
	_ld wvt$l_actv_column,
	_ld wvt$l_actv_line,
	_cld wvt$l_actv_width);
  }
 else
  {

 /*
  *	Insert character when the count + current position is =< the
  *	right margin...
  *
  *	example:
  *
  *	n			= 2
  *	wvt$l_actv_column	= 76
  *	right_edge		= 80
  *
  *
  *	O = "old data moved"
  *	C = "cleared"
  *
  *		       +-- wvt$l_actv_column (76)
  *		       !
  *		       !       +- right_edge (80)
  *		       v       v
  *	+-------------+-+-+-+-+-+
  *	!             !C!C!O!O!O!
  *	+-------------+-+-+-+-+-+
  *		           ^
  *                         !
  *		           +--- wvt$l_actv_column+n (76+2 = 78)
  *
  */


  for (x = right_edge; x >= _ld wvt$l_actv_column+n; x--)
    {
       character(y,x) = character(y,x-n);
       rendition(y,x) = rendition(y,x-n);
       ext_rendition( y, x ) = ext_rendition( y, x - n );
    }

  for (x = _ld wvt$l_actv_column; x < _ld wvt$l_actv_column+n; x++)
   {
    rendition(y,x) = NULL;
    character(y,x) = NULL;
    ext_rendition( y, x ) = NULL;
   }

  _ld wvt$b_disp_eol = TRUE;
    if (( _cld wvt$l_ext_flags & vte1_m_hebrew ) &&
	( _cld wvt$l_ext_specific_flags & vte2_m_rtl ))
	display_segment( ld, y, _ld wvt$l_right_margin, 0 );
  display_segment(ld, y, _ld wvt$l_actv_column, 0);

 }

}


/************************************/
xsel(ld) /* Selective Erase in Line */
/************************************/

wvtp ld;

{

register int n;

 edit_setup(ld);

 for (n=1; n<=_cld wvt$b_parmcnt; n++)
   {
    switch (_cld wvt$l_parms[n-1])
      {
        case 0:	selera_segment(ld, _ld wvt$l_actv_column,
				   _cld wvt$l_actv_width,
				   _ld wvt$l_actv_line);
		break;

        case 1:	selera_segment(ld, 1, _ld wvt$l_actv_column,
				      _ld wvt$l_actv_line);
		break;

        case 2:	selera_segment(ld, 1, _cld wvt$l_actv_width,
				      _ld wvt$l_actv_line);
		break;

        default: break;
       }
    }
}

/*************************/
xel(ld) /* Erase in Line */
/*************************/

wvtp ld;

{

int n, start, end;

 edit_setup(ld);

 switch (_cld wvt$b_privparm)
  {
    case 0:

	if (_cld wvt$l_parms[0] >= 3) break;

	for (n=1; n<=_cld wvt$b_parmcnt; n++)
           {

	    start = _ld wvt$l_actv_column;
	    end   = _cld wvt$l_actv_width;

 	    switch (_cld wvt$l_parms[n-1])
               {
	        case 1:
			start = 1;
			end   = _ld wvt$l_actv_column;
			break;

	        case 2:
			start = 1;
			break;

	        default:
			break;
	        }

              WVT$ERASE_DISPLAY(
				ld,
				_ld wvt$l_actv_line,
				start,
				_ld wvt$l_actv_line,
				end);

	    }
	break;

    case 1:		/* Do a Selective erase */
		/* DECSEL should be ignored in VT100 MODE	*/
        if( _cld wvt$b_conformance_level > LEVEL1 )
	xsel(ld);
	break;

    default:
	break;
    }
}

/***********************/
xdl(ld) /* Delete Line */
/***********************/

wvtp ld;

{

int n;

 edit_setup(ld);

 if ((_ld wvt$l_actv_line  <= _ld wvt$l_bottom_margin) &&
     (_ld wvt$l_top_margin <= _ld wvt$l_actv_line))
   {

    if (_cld wvt$l_parms[0] == 0) n = 1;
    else n = _cld wvt$l_parms[0];

 /*
  * 
  * This code fragment violates the VSRM by doing an erase if the count
  * is > than the region to be scrolled.  The VSRM indicates that scrolling
  * should be done.  This is much faster.
  * 
  * I have commented it out for the present.  To use it, comment out the
  * "if (...)" that minimizes n.
  * 
  *    if ((_ld wvt$l_actv_line+n) > _ld wvt$l_bottom_margin)
  *	 WVT$ERASE_DISPLAY(
  *		ld,
  *		_ld wvt$l_actv_line,
  *		1,
  *		_ld wvt$l_bottom_margin,
  *		_ld wvt$l_column_width);
  *    else
  * 
  * 
  */

    if ((_ld wvt$l_actv_line+n) > _ld wvt$l_bottom_margin)
	n = (_ld wvt$l_bottom_margin - _ld wvt$l_actv_line) + 1;

    WVT$UP_SCROLL(ld, _ld wvt$l_actv_line, n, 0);

    _ld wvt$l_actv_column = _ld wvt$l_left_margin;

   }
}


/***********************/
xil(ld) /* Insert Line */
/***********************/

wvtp ld;

{

int n;

 edit_setup(ld);

 if ((_ld wvt$l_actv_line <= _ld wvt$l_bottom_margin) && (_ld wvt$l_top_margin <= _ld wvt$l_actv_line))
   {
    if (_cld wvt$l_parms[0] == 0) n = 1;
    else n = _cld wvt$l_parms[0];

 /*
  *    if ((_ld wvt$l_actv_line+n) > _ld wvt$l_bottom_margin)
  *	 WVT$ERASE_DISPLAY(
  *		ld,
  *		_ld wvt$l_actv_line,
  *		1,
  *		_ld wvt$l_bottom_margin,
  *		_ld wvt$l_column_width);
  *    else
  *      WVT$DOWN_SCROLL(ld, _ld wvt$l_actv_line, n);
  *
  */

    if ((_ld wvt$l_actv_line+n) > _ld wvt$l_bottom_margin)
	n = (_ld wvt$l_bottom_margin - _ld wvt$l_actv_line) + 1;

    WVT$DOWN_SCROLL(ld, _ld wvt$l_actv_line, n);

    _ld wvt$l_actv_column = _ld wvt$l_left_margin;
   }
}

#if EXTENDED_PANDA
/***************************/
xdcol(ld) /* Delete Column */
/***************************/

wvtp ld;

{

int n;

 if (!(_cld wvt$l_vt200_flags_2 & vt2_m_vss_scroll_mode))
	return;		/* If not DECVSSM exit */

 if (!(edit_setup(ld))) return;

 if ((_ld wvt$l_actv_line <= _ld wvt$l_bottom_margin) &&
     (_ld wvt$l_top_margin <= _ld wvt$l_actv_line))
   {

    if (_cld wvt$l_parms[0] == 0) n = 1;
    else n = _cld wvt$l_parms[0];

    if (_ld wvt$l_right_margin < (_ld wvt$l_actv_column+n))
        n = (_ld wvt$l_right_margin-_ld wvt$l_actv_column)+1;

    WVT$LEFT_SCROLL(ld, _ld wvt$l_actv_column, n);

   }
}
#endif

#if EXTENDED_PANDA
/***************************/
xicol(ld) /* Insert Column */
/***************************/

wvtp ld;

{

int n, temp;

 if (!(_cld wvt$l_vt200_flags_2 & vt2_m_vss_scroll_mode))
	return; /* If not DECVSSM exit */

 if (!(edit_setup(ld))) return;

 if ((_ld wvt$l_actv_line <= _ld wvt$l_bottom_margin) &&
     (_ld wvt$l_top_margin <= _ld wvt$l_actv_line))
   {
    if (_cld wvt$l_parms[0] == 0) n = 1;
    else n = _cld wvt$l_parms[0];

    temp = _ld wvt$l_left_margin;
    _ld wvt$l_left_margin = _ld wvt$l_actv_column;

    if (_ld wvt$l_right_margin < (_ld wvt$l_actv_column + n))
	n = (_ld wvt$l_right_margin-_ld wvt$l_actv_column)+1;

    WVT$RIGHT_SCROLL(ld, _ld wvt$l_right_margin, n);

    _ld wvt$l_left_margin = temp;

   }
}
#endif

/***************************************/
xsed(ld) /* Selective Erase in Display */
/***************************************/

wvtp ld;

{
register int y, x, n, d;

 edit_setup(ld);

 for (n=1; n<=_cld wvt$b_parmcnt; n++)
   {
    switch (_cld wvt$l_parms[n-1])
       {
	case 0: /* erase to end of display */
		selera_segment(	ld,
				_ld wvt$l_actv_column,
				_cld wvt$l_actv_width,
				_ld wvt$l_actv_line);

		for (y = _ld wvt$l_actv_line+1; y <= _ld wvt$l_page_length; y++)
		    selera_segment(ld, 1, line_width(y), y);

		break;

	case 1: /* erase to beginning of display */

		for (y = 1; y <= _ld wvt$l_actv_line-1; y++)
		    selera_segment(ld, 1, line_width(y), y);

		selera_segment(	ld,
				1,
				_ld wvt$l_actv_column,
				_ld wvt$l_actv_line);

		break;

	case 2: /* erase entire display */

		for (y=1; y<=_ld wvt$l_page_length; y++)
		    selera_segment(ld, 1, line_width(y), y);

		break;

	default:

	    break;
	}
    }
}

/****************************/
xed(ld) /* Erase in Display */
/****************************/

wvtp ld;

{

register int y, x, n;

 edit_setup(ld);

 switch (_cld wvt$b_privparm)
   {
    case 0:

	for (n=1; n<=_cld wvt$b_parmcnt; n++)
           {
	    switch (_cld wvt$l_parms[n-1])
               {

		case 0: /* erase to end of display */

		    x = _ld wvt$l_actv_line + 1;

		    if (_ld wvt$l_actv_column == 1) x -= 1;
                    else
                       	WVT$ERASE_DISPLAY(
					ld,
					_ld wvt$l_actv_line,
					_ld wvt$l_actv_column,
					_ld wvt$l_actv_line,
					_cld wvt$l_actv_width);

		    for (y=x; y<=_ld wvt$l_page_length; y++)
                       {
			line_rendition(y) = SINGLE_WIDTH;
			line_width(y) = _ld wvt$l_column_width;
                       }

                    WVT$ERASE_DISPLAY(
					ld,
					x,
					1,
					_ld wvt$l_page_length,
					_ld wvt$l_column_width);


		    break;

		case 1: /* erase to beginning of display */

		    for (y=1; y<=_ld wvt$l_actv_line-1; y++)
                       {
			line_rendition(y) = SINGLE_WIDTH;
			line_width(y) = _ld wvt$l_column_width;
                       }

                    if (_ld wvt$l_actv_line != 1)

			WVT$ERASE_DISPLAY(
					ld,
					1,
					1,
					_ld wvt$l_actv_line-1,
					_ld wvt$l_column_width);

		    if (_ld wvt$l_actv_column == _cld wvt$l_actv_width)
                       {
			line_rendition(_ld wvt$l_actv_line) = SINGLE_WIDTH;
			_cld wvt$l_actv_width = _ld wvt$l_column_width;

                        WVT$ERASE_DISPLAY(
					ld,
					_ld wvt$l_actv_line,
					1,
					_ld wvt$l_actv_line,
					_ld wvt$l_column_width);

                       }
                    else 
                       	WVT$ERASE_DISPLAY(
					ld,
					_ld wvt$l_actv_line,
					1,
					_ld wvt$l_actv_line,
					_ld wvt$l_actv_column);
		    break;

		case 2: /* erase entire display */

		    for (y=1; y<=_ld wvt$l_page_length; y++)
                       {
			line_rendition(y) = SINGLE_WIDTH;
			line_width(y) = _ld wvt$l_column_width;
                       }

                     WVT$ERASE_DISPLAY(
					ld,
					1,
					1,
					_ld wvt$l_page_length,
					_ld wvt$l_column_width);
		    break;

		default:
		    break;
		}
	    }
	break;

  case 1:
		/* DECSED should be ignored in VT100 MODE	*/
        if( _cld wvt$b_conformance_level > LEVEL1 )
	xsed(ld);
	break;

  default:
	break;
    }
}

#if EXTENDED_PANDA
/*****************************************/
xcad(ld) /* Change attributes in display */
/*****************************************/

wvtp ld;

{

register int n, x, y;
short clear, set;

/*
 *  
 *  Change attributes in display (level 3 function)
 *  
 *  CSI Ps1; Ps2...Psn $ r
 *  
 *  Alters attributes (rendition) of characters in display.
 *  
 *  0  = From active position to end of display
 *  1  = From active position to start of display
 *  2  = Entire display
 *  ?3 = Rectangular area of display
 *
 */

 edit_setup(ld);

  switch (_cld wvt$b_privparm)
    {

    case 0:	/*
		 *  Parameter Ps1 of 0, 1 or 2
		 *
		 *  0 = From active position to end of display
		 *  1 = From active position to start of display
		 *  2 = Entire display
		 *
		 */

	if (_cld wvt$b_parmcnt < 2) _cld wvt$b_parmcnt = 2;

	clear = 0;
	set   = 0;

	for (n=1; n < _cld wvt$b_parmcnt; n++) build_masks(ld, n, &clear, &set);

	switch (_cld wvt$l_parms[0])
	{

	case 0: /* change to end of display */

		n = _ld wvt$l_actv_line + 1;

		if (_ld wvt$l_actv_column == 1) n -= 1;
		else
			change_segment(	ld,
					_ld wvt$l_actv_column,
					_cld wvt$l_actv_width,
					_ld wvt$l_actv_line,
					clear,
					set);

		for (y = n; y <= _ld wvt$l_page_length; y++)
			change_segment(	ld,
					1,
					line_width(y),
					y,
					clear,
					set);
		break;


	case 1: /* change to beginning of display */

		for (y = 1; y <= _ld wvt$l_actv_line-1; y++)
			change_segment(	ld,
					1,
					line_width(y),
					y,
					clear,
					set);

			change_segment(	ld,
					1,
					_ld wvt$l_actv_column,
					_ld wvt$l_actv_line,
					clear,
					set);

		break;

	case 2: /* change entire display */

		for (y = 1; y <= _ld wvt$l_page_length; y++)
			change_segment(	ld,
					1,
					line_width(y),
					y,
					clear,
					set);

		break;

	default:
		break;
	}
	break;

    case 1:   	/*
		 *  DEC PRIVATE MODES (CSI ?3 ... $r)
		 *
		 *
		 *  Change the attributes of the characters in
		 *  a rectangular area of the display.
		 *
		 *   *** Need to put back DECCAEM into the code ***
		 *
		 *  If DECCAEM is set, the entire screen between the
		 *  top and bottom positions is changed.  If reset,
		 *  the area is bounded on the left and right.
		 *
		 */

	switch (_cld wvt$l_parms[0])
	{
	
	/* Only ?3 is defined */

	case 3:	clear = 0;
		set   = 0;
	
		if (_cld wvt$b_parmcnt < 6) return (0);

		if (_cld wvt$l_parms[1] < 1) _cld wvt$l_parms[1] = 1;
		if (_cld wvt$l_parms[2] < 1) _cld wvt$l_parms[2] = 1;

		if ((_cld wvt$l_parms[3] < 1) ||
		    (_cld wvt$l_parms[3] > _ld wvt$l_page_length))
			_cld wvt$l_parms[3] = _ld wvt$l_page_length;

		if ((_cld wvt$l_parms[4] < 1) ||
		    (_cld wvt$l_parms[4] > _ld wvt$l_column_width))
			_cld wvt$l_parms[4] = _ld wvt$l_column_width;

		if (_cld wvt$l_parms[2] > _cld wvt$l_parms[4]) return (0);
		if (_cld wvt$l_parms[1] > _cld wvt$l_parms[3]) return (0);

		for (n=5; n < _cld wvt$b_parmcnt; n++)
			build_masks(ld, n, &clear, &set);


		if (( _cld wvt$l_parms[1] !=  _cld wvt$l_parms[3] ) &&
		   ( _cld wvt$l_vt200_flags_3 & vt3_m_ch_attr_extent_mode ))
		      {
		       change_segment(ld,
					_cld wvt$l_parms[2],
					_ld wvt$l_column_width,
					_cld wvt$l_parms[1],
					clear,
					set);

		       for (y = _cld wvt$l_parms[1]+1; y < _cld wvt$l_parms[3]; y++)
				change_segment(	ld,
						1,
						_ld wvt$l_column_width,
						y,
						clear,
						set);

		       change_segment(ld,
					1,
					_cld wvt$l_parms[4],
					_cld wvt$l_parms[3],
					clear,
					set);


		    }
		else
		    {
		     for (y = _cld wvt$l_parms[1]; y <= _cld wvt$l_parms[3]; y++)
				change_segment(	ld,
						_cld wvt$l_parms[2],
						_cld wvt$l_parms[4],
						y,
						clear,
						set);

		    }

		break;


	default:	/* Only ?3 defined */
		break;
	}
	break;
	
    default:
	break;
    }
}
#endif

#if EXTENDED_PANDA
/**************************************/
xcal(ld) /* Change attributes in line */
/**************************************/

wvtp ld;

{

register int n, x, y;
short clear, set;

/*
 *  
 *  Change attributes in line
 *  
 *  CSI Ps1; Ps2...Psn $ r
 *  
 *  Alters attributes (rendition) of characters in line.
 *  
 *  0  = From active position to end of line
 *  1  = From active position to start of line
 *  2  = Entire line
 *  ?3 = Segment of display
 *
 */

 edit_setup(ld);

  switch (_cld wvt$b_privparm)
    {

    case 0:	/*
		 *  Parameter Ps1 of 0, 1 or 2
		 *
		 *  0 = From active position to end
		 *  1 = From active position to start
		 *  2 = Entire line
		 *
		 */

	clear = 0;
	set   = 0;

	if (_cld wvt$b_parmcnt < 2) _cld wvt$b_parmcnt = 2;

	for (n=1; n < _cld wvt$b_parmcnt; n++) build_masks(ld, n, &clear, &set);

	switch (_cld wvt$l_parms[0])
	{

	   case 0: /* change to end of line */

			change_segment(	ld,
					_ld wvt$l_actv_column,
					_cld wvt$l_actv_width,
					_ld wvt$l_actv_line,
					clear,
					set);

			break;


	   case 1: /* change to beginning of line */

			change_segment(	ld,
					1,
					_ld wvt$l_actv_column,
					_ld wvt$l_actv_line,
					clear,
					set);

			break;

	   case 2: /* change entire line */

			change_segment(	ld,
					1,
					_cld wvt$l_actv_width,
					_ld wvt$l_actv_line,
					clear,
					set);

			break;

	   default:
		break;
	}

	break;

    case 1:   	/*
		 *  DEC PRIVATE MODES (CSI ?3 ... $q)
		 *
		 *
		 *  Change the attributes of the characters in
		 *  an arbitrary segment of the line.
		 *
		 *
		 */

	switch (_cld wvt$l_parms[0])
	{
	
	/* Only ?3 is defined */

	  case 3:

		clear = 0;
		set   = 0;
	
		if (_cld wvt$b_parmcnt < 4) _cld wvt$b_parmcnt = 4;

		if (_cld wvt$l_parms[1] < 1)
			_cld wvt$l_parms[1] = 1;
		else if (_cld wvt$l_parms[1] > _ld wvt$l_column_width)
			_cld wvt$l_parms[1] = _ld wvt$l_column_width;

		if (_cld wvt$l_parms[2] < 1)
			_cld wvt$l_parms[2] = _ld wvt$l_column_width;
		else if (_cld wvt$l_parms[2] > _ld wvt$l_column_width)
			_cld wvt$l_parms[2] = _ld wvt$l_column_width;

		if (_cld wvt$l_parms[1] > _cld wvt$l_parms[2]) return (0);

		for (n=3; n < _cld wvt$b_parmcnt; n++)
			build_masks(ld, n, &clear, &set);

		change_segment(	ld,
				_cld wvt$l_parms[1],
				_cld wvt$l_parms[2],
				_ld wvt$l_actv_line,
				clear,
				set);

		break;

	   default:	/* Only ?3 defined */
		break;
	}
	break;
	
    default:
	break;
    }
}
#endif

#if EXTENDED_PANDA
/******************************************/
xrad(ld) /* Reverse attributes in display */
/******************************************/

wvtp ld;

{

register int n, x, y;
short set;

/*
 *  
 *  Reverse attributes in display (level 3 function)
 *  
 *  CSI Ps1; Ps2...Psn $ r
 *  
 *  Reverses attributes (rendition) of characters in display.
 *
 * Psl =
 *  
 *  0  = From active position to end of display
 *  1  = From active position to start of display
 *  2  = Entire display
 *  ?3 = Rectangular area of display
 *
 * Ps2 ... Psn =
 *
 *  A valid SGR parameter of type 0, 1, 4, 5, 7 or 8
 *
 *  Ps2 *must* be specified.  There is no default, the sequence is ignored
 *  if the parameter count is < 2.
 *
 */

 edit_setup(ld);

  switch (_cld wvt$b_privparm)
    {

    case 0:	/*
		 *  Parameter Ps1 of 0, 1 or 2
		 *
		 *  0 = From active position to end of display
		 *  1 = From active position to start of display
		 *  2 = Entire display
		 *
		 */

	set   = 0;

	if (_cld wvt$b_parmcnt < 2) return (0);

	for (n=1; n < _cld wvt$b_parmcnt; n++)
	  {
	  switch (_cld wvt$l_parms[n])
	    {
	    case 0:	set   |=	csa_M_BOLD |
					csa_M_UNDERLINE |
					csa_M_BLINK |
					csa_M_REVERSE |
					csa_M_INVISIBLE;
			break;

	    case 1:	set   |=	csa_M_BOLD;
			break;

	    case 4:	set   |=	csa_M_UNDERLINE;
			break;

	    case 5:  	set   |=	csa_M_BLINK;
			break;

	    case 7:	set   |=	csa_M_REVERSE;
			break;

	    case 8:	set   |=	csa_M_INVISIBLE;
			break;

	    default: break;
	    }
	  }

	if (!set) return (0);

	switch (_cld wvt$l_parms[0])
	{

	case 0: /* Reverse to end of display */

		n = _ld wvt$l_actv_line + 1;

		if (_ld wvt$l_actv_column == 1) n -= 1;
		else
		  {
		   for (x=_ld wvt$l_actv_column; x <= _cld wvt$l_actv_width; x++)
			{
			 rendition(_ld wvt$l_actv_line,x) ^= set;
			 if (!character(_ld wvt$l_actv_line,x))
				character(_ld wvt$l_actv_line,x) = SPACE;
			}
		   display_segment(ld,
			_ld wvt$l_actv_line,
			_ld wvt$l_actv_column,
			_cld wvt$l_actv_width - _ld wvt$l_actv_column + 1);
		  }

		for (y = n; y <= _ld wvt$l_page_length; y++)
                  {
		   for (x = 1; x <= line_width(y); x++)
			{
			 rendition(y,x) ^= set;
			 if (!character(y,x))
				character(y,x) = SPACE;
			}
		   display_segment(ld, y, 1, line_width(y));
		  }

		break;

	case 1: /* rEVERSE to beginning of display */

		for (y=1; y<=_ld wvt$l_actv_line-1; y++)
                       {
			for (x=1; x<=line_width(y); x++)
				{
				 rendition(y,x) ^= set;
				 if (!character(y,x))
					character(y,x) = SPACE;
				}
			display_segment(ld, y, 1, line_width(y));
                       }

		for (x=1; x<=_ld wvt$l_actv_column; x++)
			{
			 rendition(_ld wvt$l_actv_line,x) ^= set;
			 if (!character(_ld wvt$l_actv_line,x))
				character(_ld wvt$l_actv_line,x) = SPACE;
			}
		display_segment(ld,
				_ld wvt$l_actv_line,
				1,
				_ld wvt$l_actv_column);

		    break;

	case 2: /* Reverse entire display */

		for (y = 1; y <= _ld wvt$l_page_length; y++)
		       {
			for (x = 1; x <= line_width(y); x++)
				{
				 rendition(y,x) ^= set;
				 if (!character(y,x))
					character(y,x) = SPACE;
				}
			display_segment(ld, y, 1, line_width(y));
		       }
		break;

	default:
		break;
	}
	break;

    case 1:   	/*
		 *  DEC PRIVATE MODES (CSI ?3 ... $r)
		 *
		 *
		 *  Reverse the attributes of the characters in
		 *  a rectangular area of the display.
		 *
		 *
		 */

	switch (_cld wvt$l_parms[0])
	{
	
	/* Only ?3 is defined */

	case 3:	set   = 0;
	
		if (_cld wvt$b_parmcnt < 6) return (0); /* MUST be at least 6 */

		if (_cld wvt$l_parms[1] < 1) _cld wvt$l_parms[1] = 1;		/* Top    */
		if (_cld wvt$l_parms[2] < 1) _cld wvt$l_parms[2] = 1;		/* Left   */

		if (_cld wvt$l_parms[3] < 1)
			_cld wvt$l_parms[3] = _ld wvt$l_page_length;
		else if (_cld wvt$l_parms[3] > _ld wvt$l_page_length)
			_cld wvt$l_parms[3] = _ld wvt$l_page_length;

		if (_cld wvt$l_parms[4] < 1)
			_cld wvt$l_parms[4] = _ld wvt$l_column_width;
		else if (_cld wvt$l_parms[4] > _ld wvt$l_column_width)
			_cld wvt$l_parms[4] = _ld wvt$l_column_width;

		if (_cld wvt$l_parms[2] > _cld wvt$l_parms[4]) return (0);
		if (_cld wvt$l_parms[1] > _cld wvt$l_parms[3]) return (0);

		for (n=5; n < _cld wvt$b_parmcnt; n++)
		  {
		  switch (_cld wvt$l_parms[n])
		    {
		    case 0:	set   |=	csa_M_BOLD |
						csa_M_UNDERLINE |
						csa_M_BLINK |
						csa_M_REVERSE |
						csa_M_INVISIBLE;
				break;

		    case 1:	set   |=	csa_M_BOLD;
				break;

		    case 4:	set   |=	csa_M_UNDERLINE;
				break;

		    case 5:	set   |=	csa_M_BLINK;
				break;

		    case 7:	set   |=	csa_M_REVERSE;
				break;

		    case 8:	set   |=	csa_M_INVISIBLE;
				break;

		    default: break;
		    }
		  }
	
		if (!set) return (0);

		for (y = _cld wvt$l_parms[1]; y <= _cld wvt$l_parms[3]; y++)
		       {
			for (x = _cld wvt$l_parms[2]; x <= _cld wvt$l_parms[4]; x++)
				{
				 rendition(y,x) ^= set;
				 if (!character(y,x))
					character(y,x) = SPACE;
				}
			display_segment(ld,
					y,
					_cld wvt$l_parms[2],
					_cld wvt$l_parms[4] - _cld wvt$l_parms[2] + 1);
		       }

		break;


	default:	/* Only ?3 defined */
		break;
	}
	break;
	
    default:
	break;
    }
}
#endif

#if EXTENDED_PANDA
/***************************************/
xral(ld) /* Reverse attributes in line */
/***************************************/

wvtp ld;

{

register int n, x, y;
short set;

/*
 *  
 *  csa_M_REVERSE attributes in line
 *  
 *  CSI Ps1; Ps2...Psn $ r
 *  
 *  csa_M_REVERSEs attributes (rendition) of characters in line.
 *  
 *  0  = From active position to end of line
 *  1  = From active position to start of line
 *  2  = Entire line
 *  ?3 = Segment of display
 *
 */

edit_setup(ld);

  switch (_cld wvt$b_privparm)
    {

    case 0:	/*
		 *  Parameter Ps1 of 0, 1 or 2
		 *
		 *  0 = From active position to end
		 *  1 = From active position to start
		 *  2 = Entire line
		 *
		 */

	set   = 0;

	if (_cld wvt$b_parmcnt < 2) return (0);

	for (n=1; n < _cld wvt$b_parmcnt; n++)
	  {
	  switch (_cld wvt$l_parms[n])
	    {
	    case 0: 	set |=	csa_M_BOLD|
				csa_M_UNDERLINE|
				csa_M_BLINK|
				csa_M_REVERSE|
				csa_M_INVISIBLE;	break;
	    case 1:	set |=	csa_M_BOLD;		break;
	    case 4:	set |=	csa_M_UNDERLINE;	break;
	    case 5:	set |=	csa_M_BLINK;		break;
	    case 7:	set |=	csa_M_REVERSE;		break;
	    case 8:	set |=	csa_M_INVISIBLE;	break;
	    default: break;
	    }
	  }

	if (!set) return (0);

	switch (_cld wvt$l_parms[0])
	{

	   case 0: /* Reverse to end of line */

		for (x = _ld wvt$l_actv_column; x <= _cld wvt$l_actv_width; x++)
			{
			 rendition(_ld wvt$l_actv_line,x) ^= set;
			 if (!character(_ld wvt$l_actv_line,x))
				character(_ld wvt$l_actv_line,x) = SPACE;
			}
		display_segment(ld, _ld wvt$l_actv_line, _ld wvt$l_actv_column,
			_cld wvt$l_actv_width - _ld wvt$l_actv_column + 1);

		break;


	   case 1: /* Reverse to beginning of line */

		for (x=1; x<=_ld wvt$l_actv_column; x++) 
			{
			 rendition(_ld wvt$l_actv_line,x) ^=  set;
			 if (!character(_ld wvt$l_actv_line,x))
				character(_ld wvt$l_actv_line,x) = SPACE;
			}
		display_segment(ld, _ld wvt$l_actv_line,
				1, _ld wvt$l_actv_column);

		break;

	   case 2: /* Reverse entire line */

		for (x = 1; x <= _cld wvt$l_actv_width; x++)
			{
			 rendition(_ld wvt$l_actv_line,x) ^=  set;
			 if (!character(_ld wvt$l_actv_line,x))
				character(_ld wvt$l_actv_line,x) = SPACE;
			}
		display_segment(ld, _ld wvt$l_actv_line,
				1, _cld wvt$l_actv_width);

		break;

	   default:
		break;
	}
	break;

    case 1:   	/*
		 *  DEC PRIVATE MODES (CSI ?3 ... $q)
		 *
		 *
		 *  Change the attributes of the characters in
		 *  an arbitrary segment of the line.
		 *
		 *
		 */

	switch (_cld wvt$l_parms[0])
	{
	
	/* Only ?3 is defined */

	  case 3:

		set   = 0;
	
		if (_cld wvt$b_parmcnt < 4) return (0);

		if (_cld wvt$l_parms[1] < 1)
			_cld wvt$l_parms[1] = 1;
		else if (_cld wvt$l_parms[1] > _ld wvt$l_column_width)
			_cld wvt$l_parms[1] = _ld wvt$l_column_width;

		if (_cld wvt$l_parms[2] < 1)
			_cld wvt$l_parms[2] = _ld wvt$l_column_width;
		else if (_cld wvt$l_parms[2] > _ld wvt$l_column_width)
			_cld wvt$l_parms[2] = _ld wvt$l_column_width;

		if (_cld wvt$l_parms[1] > _cld wvt$l_parms[2]) return (0);

		for (n=3; n < _cld wvt$b_parmcnt; n++)
		  {
		  switch (_cld wvt$l_parms[n])
		    {
		    case 0:	set  |=	csa_M_BOLD|
					csa_M_UNDERLINE|
					csa_M_BLINK|
					csa_M_REVERSE|
					csa_M_INVISIBLE;	break;
		    case 1:	set  |=	csa_M_BOLD;		break;
		    case 4:	set  |=	csa_M_UNDERLINE;	break;
		    case 5:	set  |=	csa_M_BLINK;		break;
		    case 7:	set  |=	csa_M_REVERSE;		break;
		    case 8:	set  |=	csa_M_INVISIBLE;	break;

		    default: break;
		    }
		  }
	
		if (!set) return (0);

		for (x = _cld wvt$l_parms[1]; x <= _cld wvt$l_parms[2]; x++)
			{
			 if (!character(_ld wvt$l_actv_line,x))
				character(_ld wvt$l_actv_line,x) = SPACE;
			 rendition(_ld wvt$l_actv_line,x) ^= set;
			}
		display_segment(ld, _ld wvt$l_actv_line,
				_cld wvt$l_parms[1],
				_cld wvt$l_parms[2] - _cld wvt$l_parms[1] +1);

		break;

	   default:	/* Only ?3 defined */
		break;
	}
	break;
	
    default:
	break;
    }
}
#endif

#if EXTENDED_PANDA
/***********************************/
xera(ld) /* Erase rectangular area */
/***********************************/

wvtp ld;

{

register int x, y;

/*
 *  
 *  Erase rectangular area (level 3 function)
 *  
 *  CSI Pt; Pl; Pb; Pr $ z
 *  
 *  Erases a rectangular area of the screen.
 *  this function must break the erase down if
 *  there are double wide or double high lines
 *  in the area.  This is beacuse the erase
 *  subfunction tests the line type of the first
 *  line only for basing it's size to erase.
 *
 */

 edit_setup(ld);

 if (_cld wvt$l_parms[0] < 1) _cld wvt$l_parms[0] = 1;
 if (_cld wvt$l_parms[1] < 1) _cld wvt$l_parms[1] = 1;

 if (_cld wvt$l_parms[2] < 1)
	_cld wvt$l_parms[2] = _ld wvt$l_page_length;
 else if (_cld wvt$l_parms[2] > _ld wvt$l_page_length)
	_cld wvt$l_parms[2] = _ld wvt$l_page_length;

 if (_cld wvt$l_parms[3] < 1)
	_cld wvt$l_parms[3] = _ld wvt$l_column_width;
 else if (_cld wvt$l_parms[3] > _ld wvt$l_column_width)
	_cld wvt$l_parms[3] = _ld wvt$l_column_width;

 if (_cld wvt$l_parms[0] > _cld wvt$l_parms[2]) return (0);
 if (_cld wvt$l_parms[1] > _cld wvt$l_parms[3]) return (0);

 x = _cld wvt$l_parms[0];

 for (y = _cld wvt$l_parms[0]; y <= _cld wvt$l_parms[2]; y++)
	if (line_rendition(y) != SINGLE_WIDTH)
		{
		  if (x != y)
		  WVT$ERASE_DISPLAY(
					ld,
					x,
					_cld wvt$l_parms[1],
					y-1,
					_cld wvt$l_parms[3]);
		  WVT$ERASE_DISPLAY(
					ld,
					y,
					_cld wvt$l_parms[1],
					y,
					_cld wvt$l_parms[3]);
		  x = y+1;
		 };

		  if (x <= y)
		  WVT$ERASE_DISPLAY(
					ld,
					x,
					_cld wvt$l_parms[1],
					y,
					_cld wvt$l_parms[3]);
}
#endif

#if EXTENDED_PANDA
/**********************************************/
xsera(ld) /* Selective erase rectangular area */
/**********************************************/

wvtp ld;

{

register int x, y;

/*
 *  
 *  Selective Erase Rectangular Area (level 3 function)
 *  
 *  CSI Pt; Pl; Pb; Pr $ z
 *  
 */

 edit_setup(ld);

 if (_cld wvt$l_parms[0] < 1) _cld wvt$l_parms[0] = 1;
 if (_cld wvt$l_parms[1] < 1) _cld wvt$l_parms[1] = 1;

 if (_cld wvt$l_parms[2] < 1)
	_cld wvt$l_parms[2] = _ld wvt$l_page_length;
 else if (_cld wvt$l_parms[2] > _ld wvt$l_page_length)
	_cld wvt$l_parms[2] = _ld wvt$l_page_length;

 if (_cld wvt$l_parms[3] < 1)
	_cld wvt$l_parms[3] = _ld wvt$l_column_width;
 else if (_cld wvt$l_parms[3] > _ld wvt$l_column_width)
	_cld wvt$l_parms[3] = _ld wvt$l_column_width;

 if (_cld wvt$l_parms[0] > _cld wvt$l_parms[2]) return (0);
 if (_cld wvt$l_parms[1] > _cld wvt$l_parms[3]) return (0);

 for (y = _cld wvt$l_parms[0]; y <= _cld wvt$l_parms[2]; y++)
	selera_segment(	ld,
			_cld wvt$l_parms[1],
			_cld wvt$l_parms[3],
			y);
}
#endif

#if EXTENDED_PANDA
/**********************************/
xfra(ld) /* Fill rectangular area */
/**********************************/

wvtp ld;

{

register int n, x, y;
unsigned short temp;

/*
 *  
 *  Fill rectangular area (level 3 function)
 *  
 *  CSI Pch; Pt; Pl; Pb; Pr $ z
 *  
 */


 if (_cld wvt$b_parmcnt < 1) _cld wvt$l_parms[0] = 32;

 edit_setup(ld);

 if (_cld wvt$l_parms[0] == 0) _cld wvt$l_parms[0] = 32;

 if (((_cld wvt$l_parms[0] < 32) || (_cld wvt$l_parms[0] > 255)) ||
    ((_cld wvt$l_parms[0] > 126) && (_cld wvt$l_parms[0] < 160)))  return;

 if (_cld wvt$l_parms[1] < 1) _cld wvt$l_parms[1] = 1;
 if (_cld wvt$l_parms[2] < 1) _cld wvt$l_parms[2] = 1;

 if (_cld wvt$l_parms[1] < 1)
	_cld wvt$l_parms[3] = _ld wvt$l_page_length;
 else if (_cld wvt$l_parms[3] > _ld wvt$l_page_length)
	_cld wvt$l_parms[3] = _ld wvt$l_page_length;

 if (_cld wvt$l_parms[4] < 1)
	_cld wvt$l_parms[4] = _ld wvt$l_column_width;
 else if (_cld wvt$l_parms[4] > _ld wvt$l_column_width)
	_cld wvt$l_parms[4] = _ld wvt$l_column_width;

 if (_cld wvt$l_parms[1] > _cld wvt$l_parms[3]) return (0);
 if (_cld wvt$l_parms[2] > _cld wvt$l_parms[4]) return (0);

 temp = _ld wvt$w_actv_rendition;

 if (_cld wvt$l_parms[0] > 127) temp |= _ld wvt$b_g_sets[_ld wvt$b_gr];
 else  temp |= _ld wvt$b_g_sets[_ld wvt$b_gl];

 for (y = _cld wvt$l_parms[1]; y <= _cld wvt$l_parms[3]; y++)
	{
	 for (x = _cld wvt$l_parms[2]; x <= _cld wvt$l_parms[4]; x++)
		{
		 rendition(y,x) = temp;
		 character(y,x) = _cld wvt$l_parms[0];
		 ext_rendition( y, x ) = _ld wvt$w_actv_ext_rendition;
		}
	 display_segment(ld, y, _cld wvt$l_parms[2],
				_cld wvt$l_parms[4] - _cld wvt$l_parms[2] + 1);
	}

}
#endif

/****************************************************************/
selera_segment(ld, begin, end, line) /* Selective Erase Segment */
/****************************************************************/

wvtp ld;
int begin, end, line;

{

register int x, first, last;

 first=last=0;
 for (x = begin; x <= end; x++)
	if (rendition(line,x) & csa_M_SELECTIVE_ERASE)
		{
		 character(line,x) = SPACE;
/* Reset the Kanji flags.  This will fix the bug that the Kanji erased	*/
/*			by DECSED changes to "square".			*/
/*	    rendition (line, x) &= ~(DEC_KANJI);			*/
/*	    ext_rendition (line, x) &= ~(FIRST_BYTE | TWO_BYTE_SET);	*/
		 rendition( line, x ) &= ~csa_M_CHAR_SET;
		 ext_rendition( line, x ) &= ~csa_M_EXT_CHAR_SET;
		 if (!first) first = x;
		 last = x;
		}
 if (first)
	display_segment(ld, line, first, last-first+1);
}


#if EXTENDED_PANDA
/*******************************************************************/
change_segment(ld, begin, end, line, clear, set) /* Change Segment */
/*******************************************************************/

wvtp ld;
int begin, end, line;
unsigned short clear, set;

/*
 *  Change the attributes is the segment according to the contents of the
 *  clear and set masks.
 *
 *  Check for DECCAEM... if the position is unwritten (character = 0)
 *  and DECCAEM is set, then do not change.
 *
 *  Also, I have assumed that if *not* in DECCAEM and the position is
 *  unwritten we must implicitly convert the position to a SPACE.
 *
 */

{

register int x, first, last;
register unsigned short temp;

 first=last=0;
 for (x = begin; x <= end; x++)
	{
	 if ( (_cld wvt$l_vt200_flags_3 & vt3_m_ch_attr_extent_mode) &&
	    (!character(line,x)) ) continue; /* DECCAEM check */
	 temp = rendition(line ,x);
	 rendition(line ,x) &= ~clear;
	 rendition(line ,x) |=  set;
	 if (!character(line,x)) character(line,x) = SPACE;
	 if (temp != rendition(line ,x))
		{
		 if (!first) first = x;
		 last = x;
		}
	}
 if (first)
	display_segment(ld, line, first, last-first+1);
}
#endif

#if EXTENDED_PANDA
/********************************************************/
build_masks(ld, n, clear, set) /* build set/clear masks */
/********************************************************/

wvtp ld;
register int n;
unsigned short *clear, *set;

{
 switch (_cld wvt$l_parms[n])
   {
    case 0:  *clear |=
		csa_M_BOLD|
		csa_M_UNDERLINE|
		csa_M_BLINK|
		csa_M_REVERSE|
		csa_M_INVISIBLE|
		MASK_TEXT|
		MASK_TEXT_BCK;

	     *set    = 0;			break;

    case 1:  *set   |= csa_M_BOLD;		break;

    case 4:  *set   |= csa_M_UNDERLINE;		break;

    case 5:  *set   |= csa_M_BLINK;		break;

    case 7:  *set   |= csa_M_REVERSE;		break;

    case 8:  *set   |= csa_M_INVISIBLE;		break;

    case 22: *clear |= csa_M_BOLD;
	     *set   &= ~csa_M_BOLD;		break;

    case 24: *clear |= csa_M_UNDERLINE;
	     *set   &= ~csa_M_UNDERLINE;	break;

    case 25: *clear |= csa_M_BLINK;
	     *set   &= ~csa_M_BLINK;		break;

    case 27: *clear |= csa_M_REVERSE;
	     *set   &= ~csa_M_REVERSE;		break;

    case 28: *clear |= csa_M_INVISIBLE;
	     *set   &= ~csa_M_INVISIBLE;	break;

	    /* ANSI color text - foreground */
	
    case 30: *set |= BLACK_TEXT;		break;
    case 31: *set |= RED_TEXT;			break;
    case 32: *set |= GREEN_TEXT;		break;
    case 33: *set |= YELLOW_TEXT;		break;
    case 34: *set |= BLUE_TEXT;			break;
    case 35: *set |= MAGENTA_TEXT;		break;
    case 36: *set |= CYAN_TEXT;			break;
    case 37: *set |= WHITE_TEXT;		break;

    case 39: *set |= WHITE_TEXT;		break;
	
	    /* ANSI color text - background */
	
    case 40: *set |= BLACK_TEXT_BCK;		break;
    case 41: *set |= RED_TEXT_BCK;		break;
    case 42: *set |= GREEN_TEXT_BCK;		break;
    case 43: *set |= YELLOW_TEXT_BCK;		break;
    case 44: *set |= BLUE_TEXT_BCK;		break;
    case 45: *set |= MAGENTA_TEXT_BCK;		break;
    case 46: *set |= CYAN_TEXT_BCK;		break;
    case 47: *set |= WHITE_TEXT_BCK;		break;

    case 49: *set = BLACK_TEXT_BCK;		break;

    default: break;
   }

 if (_cld wvt$l_vt200_flags_2 & vt2_m_ansi_color)
   {
    if ((_cld wvt$l_parms[n] >= 30) && (_cld wvt$l_parms[n] <= 39))
	*clear |= MASK_TEXT; 
    else if ((_cld wvt$l_parms[n] >= 40) && (_cld wvt$l_parms[n] <= 49))
	*clear |= MASK_TEXT_BCK; 
   }
 else
   {
	*clear |=  (MASK_TEXT | MASK_TEXT_BCK);
	*set   &= ~(MASK_TEXT | MASK_TEXT_BCK);
   }
}
#endif

/****************************************/
edit_setup(ld) /* common setup for edit */
/****************************************/

wvtp ld;

{

 _ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;

 if (_cld wvt$b_parmcnt == 0) _cld wvt$b_parmcnt = 1;

 if ((_ld wvt$l_actv_column < _ld wvt$l_left_margin) ||
    (_ld wvt$l_actv_column > _ld wvt$l_right_margin)) return (0);
 else return (1);

}
