/* #module WV_ESCSEQ "X03-307" */
/*  Title:	WV_ESCSEQ
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
 *  This module contains routines to designate character sets,
 *  select line renditions, and save and restore cursor
 *  (all functions invoked by escape sequences).
 *  
 *  Routines contained in this module:
 *
 *	xescseq    - dispatch escape sequence functions
 *	xdesignate - designate character set
 *	xdesignate2 - designate character set (extended for <esc>I%x)
 *	xdecdwl    - double width line
 *	xxdhlt     - double hwight line top
 *	xxdhlb     - double height line bottom
 *	xdecsc     - save cursor, etc.
 *	xdecrc     - restore cursor, etc.
 *
 *  Author:	Frederick G. Kleinsorge
 *		Low-End Workstation wvt$b_graphics Engineering
 *
 *		Adapted from code for the PRO Series Terminal
 *
 *
 * MODIFICATION HISTORY:
 *
 *  Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 * Aston Chan		12-Mar-1993	V1.2/BL2
 *	- Escape sequence   <esc>)B should not reset _ld wvt$b_nrc_set.
 *	  This solves the problem of NRC mode being reset by EDT.
 *	- Add Turkish/Greek support.
 *
 * Alfred von Campe     13-Mar-1992     V3.1/BL6
 *	- Add support for ESC $ {@,A,B,C} sequence (from To-lung).
 *
 * Aston Chan		17-Dec-1991	V3.1
 *	- I18n code merge
 *
 *  Bob Messenger       17-Jul-1990     X3.0-5
 *      Merge in Toshi Tanimoto's changes to support Asian terminals -
 *	- designation to Asian character sets
 *
 * Bob Messenger	29-May-1989	X2.0-13
 *	- Fix Restore Cursor (it was setting bits in the wrong flags word).
 *
 * Bob Messenger	12-Apr-1989	X2.0-6
 *	- Added support for quasi-"DRCS" font.
 *
 * Bob Messenger	 2-Apr-1989	X2.0-5
 *	- Moved wvt$l_column_width to specific area
 *
 * Bob Messenger	19-Mar-1989	X2.0-3
 *	- Added APL support
 *
 * Bob Messenger	04-Mar-1989	X2.0-2
 *	- Removed support for Dutch NRC
 *
 * Bob Messenger	16-Jan-1989	X1.1-1
 *	- Moved many ld fields into common area.
 *
 * Mike Leibow          07-Jun-1988
 *      - Prepared code for status line.  Some ld fields are now accessed
 *        by _cld instead of _ld.
 *
 *		Tom Porcher		20-Jan-1988
 *	- made WVT$ERASE_DISPLAY and WVT$SCREEN_ALIGNMENT all uppercase.
 *
 *		Tom Porcher		29-Dec-1987
 *	- Added callbacks for changing keypad mode.
 *
 *  FGK0008	Frederick G. Kleinsorge	16-Apr-1987
 *  
 *  o change ld data type
 *
 *  FGK0007	Frederick G. Kleinsorge	08-Apr-1987
 *  
 *  o Add feature check for ISO Latin-1 to allow bypass because of driver
 *    bug.
 *
 *  FGK0006	Frederick G. Kleinsorge	05-Mar-1987
 *  
 *  o V3.2
 *
 *  FGK0005	Frederick G. Kleinsorge 27-Feb-1987
 *  
 *  o Remove discrete display flushes
 *
 *  FGK0004	Frederick G. Kleinsorge 03-Dec-1986
 *  
 *  o Check for TCS or APL font presence, ignore designate
 *    if the font isn't present.
 *
 *  FGK0003	Frederick G. Kleinsorge 09-Sep-1986
 *  
 *  o Conditionalize PANDA features
 *
 *  FGK0002	Frederick G. Kleinsorge	14-Aug-1986
 *  
 *  o Add designations for ISO Latin-1, change for user preference
 *    instead of supplemental, add ANSI subset announcer.
 *
 *  FGK0001	Frederick G. Kleinsorge	22-Jul-1986
 *  
 *  o Update version to X04-017
 */


#include "wv_hdr.h"

/* globalvalue UCB$W_VS_STATE, UCB$M_VS_TCS_PRESENT, UCB$M_VS_APL_PRESENT;
*/


/*************************************************************/
xescseq(ld, state, start, final) /* execute escape sequences */
/*************************************************************/

wvtp ld;
struct parse_stack *state;
int start, final;

{

register int *temp, y;

/* get intermediates */
start += 1;

for (_cld wvt$b_intercnt=0; _cld wvt$b_intercnt<MAXINTERS; _cld wvt$b_intercnt++)
 {

  if (start < final)
       {
	_cld wvt$b_inters[_cld wvt$b_intercnt] = state->data[start];
	start += 1;
       }
  else break;
  }

/* get sequence final */
_cld wvt$b_finalchar = state->data[final];

/* dispatch escape sequences */


switch (_cld wvt$b_intercnt) {

/* No intermediates */

  case 0:
    switch (_cld wvt$b_finalchar) {

#if EXTENDED_PANDA
      case '6':	xdecbi(ld);
		break;
#endif

      case '7':	xdecsc(ld);
		break;

      case '8':	xdecrc(ld);
		break;

#if EXTENDED_PANDA
      case '9':	xdecfi(ld);
		break;
#endif

      case '=':	_cld wvt$l_vt200_common_flags |= vtc1_m_keypad_mode;
		WVT$SET_MISC_MODES( ld );
		break;

      case '>':	_cld wvt$l_vt200_common_flags &= ~vtc1_m_keypad_mode;
		WVT$SET_MISC_MODES( ld );
		break;

      case 'c': WVT$RESET_COLORMAP(ld);	/* Reset the colormap */
		xris(ld);		/* reset the terminal */
		break;

      case 'n': _ld wvt$b_gl = 2;
		break;

      case 'o': _ld wvt$b_gl = 3;
		break;

      case '|': _ld wvt$b_gr = 3;
		break;

      case '}': _ld wvt$b_gr = 2;
		break;

      case '~': _ld wvt$b_gr = 1;
		break;

      default:	break;

      }
    break;

/* 1 intermediate */

  case 1:

    switch (_cld wvt$b_inters[0]) {

      case '#':	switch (_cld wvt$b_finalchar)		/* ESC # final */
	{
          case '3':
#if EXTENDED_PANDA
			if (_cld wvt$l_vt200_flags_2 & vt2_m_vss_scroll_mode) break;
#endif
			if (line_rendition(_ld wvt$l_actv_line) == csa_M_DOUBLE_HIGH) break;
			xdhlt(ld);
			break;

          case '4':
#if EXTENDED_PANDA
			if (_cld wvt$l_vt200_flags_2 & vt2_m_vss_scroll_mode) break;
#endif
			if (line_rendition(_ld wvt$l_actv_line) ==
			(csa_M_DOUBLE_HIGH | csa_M_DOUBLE_BOTTOM)) break;
			xdhlb(ld);
			break;

          case '5':
#if EXTENDED_PANDA
			if (_cld wvt$l_vt200_flags_2 & vt2_m_vss_scroll_mode) break;
#endif
			if (line_rendition(_ld wvt$l_actv_line) == SINGLE_WIDTH) break;
			line_rendition(_ld wvt$l_actv_line) = SINGLE_WIDTH;
			_cld wvt$l_actv_width = line_width(_ld wvt$l_actv_line)
                                              = _ld wvt$l_column_width;
			_ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;
			display_segment(ld, _ld wvt$l_actv_line,
					 1, _ld wvt$l_column_width);
			break;

          case '6':
#if EXTENDED_PANDA
			if (_cld wvt$l_vt200_flags_2 & vt2_m_vss_scroll_mode) break;
#endif
			if (line_rendition(_ld wvt$l_actv_line) == csa_M_DOUBLE_WIDTH) break;
			xdwl(ld);
			break;

	  case '8':
	/* DECALN should cause initialization of active display.	*/
	if (_cld wvt$l_vt200_common_flags & vtc1_m_actv_status_display)
                                WVT$MAIN_DISPLAY(ld);
			_ld wvt$l_actv_line = _ld wvt$l_actv_column =
			_ld wvt$l_disp_pos  = _ld wvt$l_top_margin  = 1;
			_ld wvt$w_actv_rendition = csa_M_SELECTIVE_ERASE;
			_cld wvt$l_actv_width = _ld wvt$l_column_width;
			_ld wvt$l_bottom_margin = _ld wvt$l_page_length;
			_ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;
			_ld wvt$l_vt200_specific_flags &= ~vts1_m_origin_mode;
			for (y = 1; y <= _ld wvt$l_page_length; y++)
			     {
			      line_width(y) = _ld wvt$l_column_width;
			      line_rendition(y) = SINGLE_WIDTH;
			      }
			WVT$ERASE_DISPLAY(
					  ld,
					   1,
					   1,
					  _ld wvt$l_page_length,
					  _ld wvt$l_column_width) ; /* clear */

			WVT$SCREEN_ALIGNMENT(ld);   /* Do screen alignment */
			break;

          default:	break;
        }
        break;

      case '(':	xdesignate(ld, 0);
		break;			/* ESC ( final */

      case ')':	xdesignate(ld, 1);
		break;			/* ESC ) final */

      case '*':	xdesignate(ld, 2);
		break;			/* ESC * final */

      case '+':	xdesignate(ld, 3);
		break;			/* ESC + final */

      case '$':	xdesignate(ld, 0);
		break;			/* ESC $ final, final = {@,A,B,C} */

      /* ISO single byte registered 96-character sets */

      case '-':	xdesignate(ld, 1);
		break;			/* ESC - final */

      case '.':	xdesignate(ld, 2);
		break;			/* ESC . final */

      case '/':	xdesignate(ld, 3);
		break;			/* ESC / final */

      case 32:				/* ESC SP final */

        if (_cld wvt$b_conformance_level >= LEVEL2)
         {
          switch (_cld wvt$b_finalchar)
	   {
            case 'F':

              if (_cld wvt$b_conformance_level >= LEVEL2)
                _cld wvt$l_vt200_common_flags &= ~vtc1_m_c1_transmission_mode;
              break;

            case 'G':

              if (_cld wvt$b_conformance_level >= LEVEL2)
                _cld wvt$l_vt200_common_flags |= vtc1_m_c1_transmission_mode;
              break;

	    /* Announce Subset of code extension for dpANS X3.134.1-198x */

            case 'L':
            case 'M':

	      if (_cld wvt$l_vt200_flags_2 & vt2_m_enable_ISO_latin)
                {
		  if (_cld wvt$b_conformance_level >= LEVEL2)
		    {
		     _ld wvt$b_g_sets[0] = ASCII;
		     _ld wvt$b_g_sets[1] = ISO_LATIN_1;
		     _ld wvt$b_ext_g_sets[0] = STANDARD_SET;
		     _ld wvt$b_ext_g_sets[1] = STANDARD_SET;
		     _ld wvt$b_gl = 0;
		     _ld wvt$b_gr = 1;
		    }
		 }

              break;

            case 'N':

              if (_cld wvt$b_conformance_level >= LEVEL2)
		{
		 _ld wvt$b_g_sets[0] = ASCII;
		 _ld wvt$b_ext_g_sets[0] = STANDARD_SET;
		 _ld wvt$b_gl = 0;
		}

              break;

            default: break;
           }
	   WVT$SET_TERMINAL_MODE(ld);
         }
        break;

      default: break;
      }
    break;
/* 2 and 3 intermediates */

  case 2:

  case 3:


    switch (_cld wvt$b_inters[0]) {

      case 40: xdesignate(ld, 0); break;	/* <ESC>(<final>  => G0 */
      case 41: xdesignate(ld, 1); break;	/* <ESC>)<final>  => G1 */
      case 42: xdesignate(ld, 2); break;	/* <ESC>*<final>  => G2 */
      case 43: xdesignate(ld, 3); break;	/* <ESC>+<final>  => G3 */

      case 36: switch (_cld wvt$b_inters[1]) {
		    case 41: xdesignate(ld, 1); break;
						/* <ESC>$)<final> => G1 */
		    case 42: xdesignate(ld, 2); break;
						/* <ESC>$*<final> => G2 */
		    case 43: xdesignate(ld, 3); break;
						/* <ESC>$+<final> => G3 */
		    default: break;
	       }
	       break;

      default: break;
      }

    break;

  default: break;
  }   /* end switch wvt$b_intercnt */
/*
	Ignore all unrecognized escape sequences
*/
return(0);
}

/***************************************************/
xdesignate(ld, gset) /* Designate for ESC Ig FINAL */
/***************************************************/

wvtp ld;
unsigned short gset;

{

unsigned short *temp;
int was_nset;
was_nset = TRUE;		/* Assume it is a NRC */
		/* clear user preferred supplemental flag	*/
    _ld wvt$b_ups &= ~( 1 << gset );

switch (_cld wvt$b_intercnt)
 {
  case 1:

   switch (_cld wvt$b_inters[0])
    {

     case '(':				/* Ig = ( */
     case ')': 				/* Ig = ) */
     case '*': 				/* Ig = * */
     case '+': 				/* Ig = + */

     if ( _cld wvt$b_inters[0] == '(' )
	if (_cld wvt$b_conformance_level==LEVEL1 && _cld wvt$b_finalchar=='1') {
	    _ld wvt$b_g_sets[0] = ASCII;
	    _ld wvt$b_ext_g_sets[0] = STANDARD_SET;
	    break;
	}


    if ( _cld wvt$b_inters[0] == ')' )
	if (_cld wvt$b_conformance_level == LEVEL1) {
	    if (_cld wvt$b_finalchar == '1') {
		ld->common.rightToLeft = TRUE;
		_cld wvt$l_ext_specific_flags |= vte2_m_rtl; 
		_cld wvt$l_ext_specific_flags |= vte2_m_kb_soft_switch;
		if ( _cld wvt$l_ext_flags & vte1_m_hebrew )
		DwtDECtermToggleKeyboard(ld, TRUE);
		break;
	    }
	    else if (_cld wvt$b_finalchar == 'B') {
		ld->common.rightToLeft = FALSE;
		_cld wvt$l_ext_specific_flags &= ~vte2_m_rtl; 
		_ld wvt$b_g_sets[1] = ASCII;
		_ld wvt$b_ext_g_sets[1] = STANDARD_SET;
		break;
	    }
	}


      switch (_cld wvt$b_finalchar)
       {

        case 'B':	_ld wvt$b_g_sets[gset] = ASCII;
			_ld wvt$b_ext_g_sets[gset] = STANDARD_SET;
			break;

        case '0':	_ld wvt$b_g_sets[gset] = LINE_DRAWING;
			_ld wvt$b_ext_g_sets[gset] = STANDARD_SET;
			was_nset = FALSE;
			break;

	case '<':	if ( _cld wvt$b_conformance_level > LEVEL1) {
			_ld wvt$b_g_sets[gset] = _cld wvt$b_user_preference_set;
			if ( _cld wvt$b_user_preference_set == ISO_LATIN_8 ||
			     _cld wvt$b_user_preference_set == HEB_SUPPLEMENTAL )
			    _ld wvt$b_ext_g_sets[gset] = ONE_BYTE_SET;
			else
			_ld wvt$b_ext_g_sets[gset] = STANDARD_SET;
		/* Set user preferred supplemental flag.	*/
			_ld wvt$b_ups |= ( 1<<gset );
			}
			was_nset = FALSE;
			break;
/*MOOF*/
        case '>': /*	temp = _cld wvt$a_ucbadr + UCB$W_VS_STATE;
			if (*temp & UCB$M_VS_TCS_PRESENT) */
			if ( _cld wvt$b_conformance_level > LEVEL1) {
				_ld wvt$b_g_sets[gset] = TECHNICAL;
				_ld wvt$b_ext_g_sets[gset] = STANDARD_SET;
			}
			was_nset = FALSE;
			break;

	/* NRC sets */
  
        case 'A': _ld wvt$b_nrc_set = 2;	break; /* UK */
	case '9':
        case 'Q': _ld wvt$b_nrc_set = 4;	break; /* CANADIAN */
        case 'E':
        case '`':
        case '6': _ld wvt$b_nrc_set = 5;	break; /* NORWEGIAN/DANISH */
        case 'C':
        case '5': _ld wvt$b_nrc_set = 6;	break; /* FINNISH */
        case 'K': _ld wvt$b_nrc_set = 7;	break; /* GERMAN */
        case 'Y': _ld wvt$b_nrc_set = 9;	break; /* ITALIAN */
        case '=': _ld wvt$b_nrc_set = 10;	break; /* SWISS */
        case 'H':
        case '7': _ld wvt$b_nrc_set = 12;	break; /* SWEDISH */
        case 'R': _ld wvt$b_nrc_set = 14;	break; /* FRENCH */
        case 'Z': _ld wvt$b_nrc_set = 15;	break; /* SPANISH */

        case 'J': _ld wvt$b_g_sets[gset] = JIS_ROMAN;
		  _ld wvt$b_ext_g_sets[gset] = ONE_BYTE_SET;
		  was_nset = FALSE;
		  break;
	case 'I': _ld wvt$b_g_sets[gset] = JIS_KATAKANA;
		  _ld wvt$b_ext_g_sets[gset] = ONE_BYTE_SET;
		  was_nset = FALSE;
		  break;

        default:  was_nset = FALSE;
		  break;

       }
      break;

     case '$':	was_nset = FALSE;
		switch ( _cld wvt$b_finalchar ) {
		case '@':
		case 'B':	_ld wvt$b_g_sets[gset] = DEC_KANJI;
				_ld wvt$b_ext_g_sets[gset] = TWO_BYTE_SET;
				break;
		case 'A':	_ld wvt$b_g_sets[gset] = DEC_HANZI;
				_ld wvt$b_ext_g_sets[gset] = TWO_BYTE_SET;
				break;
		case 'C':	_ld wvt$b_g_sets[gset] = DEC_HANGUL;
				_ld wvt$b_ext_g_sets[gset] = TWO_BYTE_SET;
				break;
		default:	break;
		}
		break;
	
     case '-': 				/* Ig = - */
     case '.': 				/* Ig = . */
     case '/': 				/* Ig = / */

	if (_cld wvt$b_conformance_level == LEVEL1) {
	    was_nset = FALSE;
	    break;
	}

      switch (_cld wvt$b_finalchar)
       {
	/* case for default 96 character set.  'A', 'H', 'F', 'M' stand
	 * for  ISO_Latin1, ISO_Hebrew, ISO_Greek and ISO_Turkish respectively
	 */

        case 'A':	if (_cld wvt$l_vt200_flags_2 & vt2_m_enable_ISO_latin)
			  {
			   _ld wvt$b_g_sets[gset] = ISO_LATIN_1;
			   _ld wvt$b_ext_g_sets[gset] = STANDARD_SET;
			  }
			was_nset = FALSE;
			break;

        case 'H':	if (_cld wvt$l_vt200_flags_2 & vt2_m_enable_ISO_latin)
                          {
                           _ld wvt$b_g_sets[gset] = ISO_LATIN_8;
			   _ld wvt$b_ext_g_sets[gset] = ONE_BYTE_SET;
                          }
                        was_nset = FALSE;
                        break;

        case 'F':	if (_cld wvt$l_vt200_flags_2 & vt2_m_enable_ISO_latin)
                          {                                  /* ISO-GREEK */
                           _ld wvt$b_g_sets[gset] = ISO_LATIN_7;
			   _ld wvt$b_ext_g_sets[gset] = STANDARD_SET;
                          }
                        was_nset = FALSE;
                        break;

        case 'M':	if (_cld wvt$l_vt200_flags_2 & vt2_m_enable_ISO_latin)
                          {                                 /* ISO-TURKISH */
                           _ld wvt$b_g_sets[gset] = ISO_LATIN_5;
			   _ld wvt$b_ext_g_sets[gset] = STANDARD_SET;
                          }
                        was_nset = FALSE;
                        break;

        default:	was_nset = FALSE;
			break;
       }
      break;

     default: was_nset = FALSE;
	      break;
   }

    /* Set ASCII set if a nrc set was designated */

    if (was_nset) {
	_ld wvt$b_g_sets[gset] = ASCII;
	_ld wvt$b_ext_g_sets[gset] = STANDARD_SET;
    }
    break;

  case 2:

    switch (_cld wvt$b_inters[1])
	{

	 case 37:	/* case for Dscs  in the form  "% x" */

	    switch (_cld wvt$b_inters[0])
	     {
	      case '(':					/* Ig = ( */
	      case ')': 				/* Ig = ) */
	      case '*': 				/* Ig = * */
	      case '+': 				/* Ig = + */
		       if (_cld wvt$b_conformance_level > LEVEL1)
		       xdesignate2(ld, gset);		/* ESC Ig % final */
		       break;
	      default: break;
	     }
            break;

	 case 34:	/* case for Dscs in the form "\" x" */

	    switch (_cld wvt$b_inters[0])
	     {
	      case '(':					/* Ig = ( */
	      case ')': 				/* Ig = ) */
	      case '*': 				/* Ig = * */
	      case '+': 				/* Ig = + */
		       if (_cld wvt$b_conformance_level > LEVEL1)
		       xdesignate3(ld, gset);		/* ESC Ig " Final */
		       break;
	      default: break;
	     }
            break;

	 case 38:	/* case for Dscs in the form "& x" */

	    switch (_cld wvt$b_inters[0])
	     {
	      case '(':					/* Ig = ( */
	      case ')': 				/* Ig = ) */
	      case '*': 				/* Ig = * */
	      case '+': 				/* Ig = + */
		       if (_cld wvt$b_conformance_level > LEVEL1)
		       xdesignate4(ld, gset);		/* ESC Ig & Final */
		       break;
	      default: break;
	     }
            break;

	case 32:	/* case for Dscs in the form " x" */

	    switch (_cld wvt$b_inters[0])
	     {
	      case '(':					/* Ig = ( */
	      case ')': 				/* Ig = ) */
	      case '*': 				/* Ig = * */
	      case '+': 				/* Ig = + */
		       if (_cld wvt$b_conformance_level > LEVEL1)
		       xdesignate5(ld, gset);
		       break;
	      default: break;
	     }
            break;

	case 41:					/* ESC $ ) Final */
	case 42:					/* ESC $ * Final */
	case 43:					/* ESC $ + Final */
	    if (_cld wvt$b_inters[0] == '$')
		xdesignate6(ld, gset);
            break;

	}

    break;

  case 3:

#if DRCS_EXTENSION
    if (_cld wvt$b_dscs_wvt$b_intercnt == 2 && _cld wvt$b_inters[1] == _cld wvt$b_dscs[0]
	&& _cld wvt$b_inters[2] == _cld wvt$b_dscs[1] && _cld wvt$b_finalchar == _cld wvt$b_dscs[2])
	{
	 xdrcsfont(ld, gset);
	 break;
	}
#endif

  default: break;
  }
}

/******************************************************/
xdesignate2(ld, gset) /* Designate for ESC Ig % FINAL */
/******************************************************/

wvtp ld;
unsigned short gset;

{

int was_nset;

was_nset = TRUE;  /* Assume it is a NRC */

switch (_cld wvt$b_finalchar)
  {
							/* DEC Turkish */
   case '0':	_ld wvt$b_g_sets[gset] = TURKISH_SUPPLEMENTAL; 
		_ld wvt$b_ext_g_sets[gset] = STANDARD_SET;
		was_nset = FALSE;
		break;
   case '5':	_ld wvt$b_g_sets[gset] = SUPPLEMENTAL;
		_ld wvt$b_ext_g_sets[gset] = STANDARD_SET;
		was_nset = FALSE;
		break;
   case '6':	_ld wvt$b_nrc_set = 16;
		break;			/* PORTUGUESE */
   case '=':	_ld wvt$b_nrc_set = 17;
		break;			/* HEBREW */

   case '2':	_ld wvt$b_nrc_set = 18;
		break;			/* Turkish NRCS */

   case '?':	_ld wvt$b_g_sets[gset] = KS_ROMAN;
		_ld wvt$b_ext_g_sets[gset] = ONE_BYTE_SET;
		was_nset = FALSE;
		break;

   default:	was_nset = FALSE;
		break;
  }

/* Set ASCII set if a nrc set was designated */
if (was_nset) {
     _ld wvt$b_g_sets[gset] = ASCII;
     _ld wvt$b_ext_g_sets[gset] = STANDARD_SET;
}
}


/******************************************************/
xdesignate3(ld, gset) /* Designate for ESC Ig " FINAL */
/******************************************************/

wvtp ld;
unsigned short gset;

{

unsigned short *temp;

switch (_cld wvt$b_finalchar)
  {
    case '0':	if ( _cld wvt$b_inters[0] == '+' ) {
		    if (_cld wvt$l_ext_flags & vte1_m_tomcat) {
			_ld wvt$b_g_sets[gset] = DEC_KANJI;
			_ld wvt$b_ext_g_sets[gset] = TWO_BYTE_SET;
		    } else if (_cld wvt$l_ext_flags & vte1_m_bobcat) {
			_ld wvt$b_g_sets[gset] = DEC_HANZI;
			_ld wvt$b_ext_g_sets[gset] = TWO_BYTE_SET;
		    } else if (_cld wvt$l_ext_flags & vte1_m_dickcat) {
			_ld wvt$b_g_sets[gset] = DEC_HANGUL;
			_ld wvt$b_ext_g_sets[gset] = TWO_BYTE_SET;
		    } else if (_cld wvt$l_ext_flags & vte1_m_fishcat) {
			_ld wvt$b_g_sets[gset] = DEC_HANYU;
			_ld wvt$b_ext_g_sets[gset] = FOUR_BYTE_SET;
		    }
		}
		break;
   case '4':	_ld wvt$b_g_sets[gset] = HEB_SUPPLEMENTAL;
		_ld wvt$b_ext_g_sets[gset] = ONE_BYTE_SET;
		break;

   case '?':	_ld wvt$b_g_sets[gset] = GREEK_SUPPLEMENTAL;    /* DEC Greek */
		_ld wvt$b_ext_g_sets[gset] = STANDARD_SET;
		break;

   case '>':	_ld wvt$b_nrc_set = 19;
		break;			/* Greek NRCS */

   /* APL Overstrike set */
/*MOOF*/
   case '8': /*	temp = _cld wvt$a_ucbadr + UCB$W_VS_STATE;
		if (*temp & UCB$M_VS_APL_PRESENT) */
            if (_cld wvt$b_conformance_level > LEVEL1) {
			_ld wvt$b_g_sets[gset] = APL;
			_ld wvt$b_ext_g_sets[gset] = STANDARD_SET;
	    }
		break;
   default:  break;
  }

}

/******************************************************/
xdesignate4(ld, gset) /* Designate for ESC Ig & FINAL */
/******************************************************/

wvtp ld;
unsigned short gset;

{

unsigned short *temp;

switch (_cld wvt$b_finalchar)
  {

   /* APL Supplemental set */

   case '0': 	_ld wvt$b_g_sets[gset] = APL;
		_ld wvt$b_ext_g_sets[gset] = STANDARD_SET;
		break;
   default:  break;
  }

}

/******************************************************/
xdesignate5(ld, gset) /* Designate for ESC Ig SP FINAL*/
/******************************************************/

wvtp ld;
unsigned short gset;

{

unsigned short *temp;

switch (_cld wvt$b_finalchar)
  {

   /* limited DRCS set */

   case '@': 	_ld wvt$b_g_sets[gset] = DRCS_FONT;
		_ld wvt$b_ext_g_sets[gset] = STANDARD_SET;
		break;
   default:  break;
  }

}

#if DRCS_EXTENSION
/************************************************************/
xdrcsfont (ld, gset)	/* Designate DRCS font		    */
/************************************************************/

wvtp ld;
unsigned short gset;

{
_ld wvt$b_g_sets[gset] = DRCS_FONT;
_ld wvt$b_ext_g_sets[gset] = STANDARD_SET;
}
#endif

/******************************************************/
xdesignate6(ld, gset) /* Designate for ESC $ + FINAL */
/******************************************************/

wvtp ld;
unsigned short gset;

{
switch (_cld wvt$b_finalchar)
  {

   case '1':
   case '@':
   case '3':
   case 'B':	_ld wvt$b_g_sets[gset] = DEC_KANJI;
		_ld wvt$b_ext_g_sets[gset] = TWO_BYTE_SET;
		break;
   case '2':
   case 'A':	_ld wvt$b_g_sets[gset] = DEC_HANZI;
		_ld wvt$b_ext_g_sets[gset] = TWO_BYTE_SET;
		break;
   case '4':
   case 'C':	_ld wvt$b_g_sets[gset] = DEC_HANGUL;
		_ld wvt$b_ext_g_sets[gset] = TWO_BYTE_SET;
		break;
   case '5':	_ld wvt$b_g_sets[gset] = DEC_HANYU;
		_ld wvt$b_ext_g_sets[gset] = FOUR_BYTE_SET;
		break;
   default:     break;
   }
}

/************************************************************/
xdecsc(ld)		/* Save Cursor                      */
/************************************************************/

wvtp ld;

{

_ld wvt$l_save_line = _ld wvt$l_actv_line;
_ld wvt$l_save_column = _ld wvt$l_actv_column;
_ld wvt$w_save_rendition = _ld wvt$w_actv_rendition;
_ld wvt$w_save_ext_rendition = _ld wvt$w_actv_ext_rendition;
_ld wvt$l_save_vt200_flags = _ld wvt$l_vt200_specific_flags;

_ld wvt$b_save_g_sets[0] = _ld wvt$b_g_sets[0];
_ld wvt$b_save_g_sets[1] = _ld wvt$b_g_sets[1];
_ld wvt$b_save_g_sets[2] = _ld wvt$b_g_sets[2];
_ld wvt$b_save_g_sets[3] = _ld wvt$b_g_sets[3];
_ld wvt$b_save_ext_g_sets[0] = _ld wvt$b_ext_g_sets[0];
_ld wvt$b_save_ext_g_sets[1] = _ld wvt$b_ext_g_sets[1];
_ld wvt$b_save_ext_g_sets[2] = _ld wvt$b_ext_g_sets[2];
_ld wvt$b_save_ext_g_sets[3] = _ld wvt$b_ext_g_sets[3];
_ld wvt$b_sav_gl = _ld wvt$b_gl;
_ld wvt$b_sav_gr = _ld wvt$b_gr;
}


/************************************************************/
xdecrc(ld)		/* Restore Cursor                   */
/************************************************************/

wvtp ld;

{

_ld wvt$l_vt200_specific_flags &= ~vts1_m_origin_mode;
_ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;

_ld wvt$l_actv_line = _ld wvt$l_save_line;

if (_ld wvt$l_save_vt200_flags & vts1_m_origin_mode)
  {
   _ld wvt$l_vt200_specific_flags |= vts1_m_origin_mode;
   if (_ld wvt$l_actv_line < _ld wvt$l_top_margin) _ld wvt$l_actv_line = _ld wvt$l_top_margin;
   if (_ld wvt$l_bottom_margin < _ld wvt$l_actv_line) _ld wvt$l_actv_line = _ld wvt$l_bottom_margin;
  }

_cld wvt$l_actv_width = line_width(_ld wvt$l_actv_line);

if (_cld wvt$l_actv_width < _ld wvt$l_save_column)
	_ld wvt$l_actv_column = _cld wvt$l_actv_width;
else	_ld wvt$l_actv_column = _ld wvt$l_save_column;

if (_ld wvt$l_save_vt200_flags & vts1_m_origin_mode)
  {
   if (_ld wvt$l_actv_column < _ld wvt$l_left_margin) _ld wvt$l_actv_column = _ld wvt$l_left_margin;
   if (_ld wvt$l_right_margin < _ld wvt$l_actv_column) _ld wvt$l_actv_column = _ld wvt$l_right_margin;
  }

_ld wvt$w_actv_rendition = _ld wvt$w_save_rendition;
_ld wvt$w_actv_ext_rendition = _ld wvt$w_save_ext_rendition;

if (_ld wvt$l_save_vt200_flags & vts1_m_last_column)
    _ld wvt$l_vt200_specific_flags |= vts1_m_last_column;

_ld wvt$b_g_sets[0] = _ld wvt$b_save_g_sets[0];
_ld wvt$b_g_sets[1] = _ld wvt$b_save_g_sets[1];
_ld wvt$b_g_sets[2] = _ld wvt$b_save_g_sets[2];
_ld wvt$b_g_sets[3] = _ld wvt$b_save_g_sets[3];
_ld wvt$b_ext_g_sets[0] = _ld wvt$b_save_ext_g_sets[0];
_ld wvt$b_ext_g_sets[1] = _ld wvt$b_save_ext_g_sets[1];
_ld wvt$b_ext_g_sets[2] = _ld wvt$b_save_ext_g_sets[2];
_ld wvt$b_ext_g_sets[3] = _ld wvt$b_save_ext_g_sets[3];
_ld wvt$b_gl = _ld wvt$b_sav_gl;
_ld wvt$b_gr = _ld wvt$b_sav_gr;
}

/************************************************************/
xdhlt(ld)		/* Double Height Top Line           */
/************************************************************/

wvtp ld;

{

register int x;

if (_cld wvt$l_vt200_common_flags & vtc1_m_actv_status_display) {
	xdwl(ld);
	return;
}
_ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;

line_rendition(_ld wvt$l_actv_line) = csa_M_DOUBLE_HIGH;

_cld wvt$l_actv_width = line_width(_ld wvt$l_actv_line) = _ld wvt$l_column_width / 2;

if (_cld wvt$l_actv_width < _ld wvt$l_actv_column)
    {
     _ld wvt$l_actv_column = _cld wvt$l_actv_width;
     _ld wvt$l_vt200_specific_flags |= vts1_m_last_column;
    }
 
for (x = _cld wvt$l_actv_width+1; x <= _ld wvt$l_column_width; x++)
   {
    character(_ld wvt$l_actv_line,x) = NULL;
    rendition(_ld wvt$l_actv_line,x) = NULL;
    ext_rendition(_ld wvt$l_actv_line,x) = NULL;
   }

_ld wvt$b_disp_eol = TRUE;
if (( _cld wvt$l_ext_flags & vte1_m_hebrew ) &&
    ( _cld wvt$l_ext_specific_flags & vte2_m_rtl ))
    display_segment( ld, _ld wvt$l_actv_line, _ld wvt$l_column_width, 0 );
else
display_segment(ld, _ld wvt$l_actv_line, 1, 0); /* re-display line */

}


/************************************************************/
xdhlb(ld)			/* Double Height Bottom Line*/
/************************************************************/

wvtp ld;

{

register int x;

if (_cld wvt$l_vt200_common_flags & vtc1_m_actv_status_display) {
	xdwl(ld);
	return;
}
_ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;

line_rendition(_ld wvt$l_actv_line) = (csa_M_DOUBLE_BOTTOM | csa_M_DOUBLE_HIGH);

_cld wvt$l_actv_width = line_width(_ld wvt$l_actv_line) = _ld wvt$l_column_width / 2;

if (_cld wvt$l_actv_width < _ld wvt$l_actv_column)
   {
    _ld wvt$l_actv_column = _cld wvt$l_actv_width;
    _ld wvt$l_vt200_specific_flags |= vts1_m_last_column;
   }
for (x = _cld wvt$l_actv_width+1; x <= _ld wvt$l_column_width; x++)
   {
    character(_ld wvt$l_actv_line,x) = NULL;
    rendition(_ld wvt$l_actv_line,x) = NULL;
    ext_rendition(_ld wvt$l_actv_line,x) = NULL;
   }

_ld wvt$b_disp_eol = TRUE;
if (( _cld wvt$l_ext_flags & vte1_m_hebrew ) &&
    ( _cld wvt$l_ext_specific_flags & vte2_m_rtl ))
    display_segment( ld, _ld wvt$l_actv_line, _ld wvt$l_column_width, 0 );
else
display_segment(ld, _ld wvt$l_actv_line, 1, 0); /* re-display line */

}


/************************************************************/
xdwl(ld)		/* Double Width Line                */
/************************************************************/

wvtp ld;

{

register int x;

_ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;

line_rendition(_ld wvt$l_actv_line) = csa_M_DOUBLE_WIDTH;

_cld wvt$l_actv_width = line_width(_ld wvt$l_actv_line) = _ld wvt$l_column_width / 2;

if (_cld wvt$l_actv_width < _ld wvt$l_actv_column)
   {
    _ld wvt$l_actv_column = _cld wvt$l_actv_width;
    _ld wvt$l_vt200_specific_flags |= vts1_m_last_column;
   }

for (x = _cld wvt$l_actv_width+1; x <= _ld wvt$l_column_width; x++)
   {
    character(_ld wvt$l_actv_line,x) = NULL;
    rendition(_ld wvt$l_actv_line,x) = NULL;
    ext_rendition(_ld wvt$l_actv_line,x) = NULL;
   }

_ld wvt$b_disp_eol = TRUE;
if (( _cld wvt$l_ext_flags & vte1_m_hebrew ) &&
    ( _cld wvt$l_ext_specific_flags & vte2_m_rtl ))
    display_segment( ld, _ld wvt$l_actv_line, _ld wvt$l_column_width, 0 );
else
display_segment(ld, _ld wvt$l_actv_line, 1, 0); /* re-display line */

}
