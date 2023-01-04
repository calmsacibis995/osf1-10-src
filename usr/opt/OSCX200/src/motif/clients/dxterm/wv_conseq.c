/* #module WV_CONSEQ "X3.0-5"  */
/*
 *  Title:	WV_CONSEQ
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
 *  This module contains routines to set display state, and select
 *  character rendition and attributes.
 *
 *  Routines contained in this module:
 *
 *	xcntseq   - dispatch control sequence functions
 *	seqparm   - Make local copies of parsed sequence parameters
 *	xsm       - dispatch set mode functions
 *	xrm       - dispatch reset mode functions
 *	xsgr      - select wvt$b_graphic rendition
 *	xsca      - select character attributes
 *	xdecstbm  - set top and bottom margins
 *	xtbc      - clear horizontal tab stops
 *	xdecefr   - enable filter rectangles
 *	xdecsle   - select locator events
 *	xdecelr   - enable locator reports
 *	xdecslcs  - select locator cursor style
 *
 *  Author:	Frederick G. Kleinsorge
 *		Low-End Workstation Graphics Engineering
 *
 *		Adapted from code for the PRO Series Terminal
 *		Emulator.
 *
 * Revision History:
 *
 * Alfred von Campe     15-Oct-1993     BL-E
 *      - Add typecasts to satisfy OSF/1 compiler.
 *
 * Alfred von Campe     30-Sep-1993     BL-E
 *      - Add multi-page support.
 *
 *  Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 * Alfred von Campe     25-Mar-1993     V1.2/BL2
 *      - Add F1-F5 key support.
 *
 * Aston Chan		05-Feb-1993	V1.2/BL2
 *	- Fix problem with German keyboard.  Garbage displayed after this :
 *	  <esc>[?;2h.  Problem is in xsm() (and xrm()) the gr is set to 1
 *	  by I18n code.  It should ONLY be set to 1 in vte1_m_tomcat mode.
 *
 * Aston Chan		17-Dec-1991	V3.1
 *	- I18n code merge
 *
 * Alfred von Campe     20-May-1990     V3.0
 *      - Don't change to condensed font if German Standard font is in use.
 *    
 * Bob Messenger	20-Jul-1990	X3.0-5
 *	- Support the Enable Session command to give the current window the
 *	  input focus.
 *
 * Bob Messenger        17-Jul-1990     X3.0-5
 *      Merge in Toshi Tanimoto's changes to support Asian terminals -
 *	- character set definitions for DECANM
 *	- DECSDM sixel scroll mode
 *	- DECKKDM Kanji/Katakana terminal mode (Japan)
 *	- CRM control representation mode (Japan)
 *
 * Bob Messenger	 7-Jul-1990	X3.0-5
 * 	- Update the printExtent resource when setting or resetting printer
 *	  extent mode.
 *
 * Bob Messenger	23-Mar-1990	V2.1
 *	- Only require conformance level 2, not 3, for terminal state reports.
 *
 * Bob Messenger	12-Mar-1990	V2.1
 *	- Fix SGR for default foreground color (missing break).
 *
 * Bob Messenger	 7-Jun-1989	X2.0-13
 *	- Disable DECARM, this time for good.
 *
 * Bob Messenger	28-May-1989	X2.0-13
 *	- wvt$b_tab_stops goes from 1 to MAX_COLUMN, not 0 to MAX_COLUMN - 1
 *	- ignore ANM (VT52 mode) if in status line
 *
 * Bob Messenger	16-May-1989	X2.0-12
 *	_ Back out the DECARM change, in the hope that we can fix the
 *	  auto repeat frenzy bug in time for IFT.
 *
 * Bob Messenger	 9-May-1989	X2.0-10
 *	- Change DECRCS to DECRQDCC and DECRPDCC (Request/Report Device
 *	  Color Capabilities) and use approved bindings.
 *	- Disable the DECARM sequence, which controls auto-repeat, as a
 *	  temporary fix for the WPSPLUS/ALL-IN-1 auto-repeat problem.
 *
 * Bob Messenger	11-Apr-1989	X2.0-6
 *	- Implement DECCTR (color table report).
 *	- Implement "DECRCS" (tentative name) request/report colors status.
 *
 * Bob Messenger	 4-Apr-1989	X2.0-5
 *	- Implement ANSI color text (slightly differently from VWS).
 *
 * Bob Messenger	 2-Apr-1989	X2.0-5
 *	- Move wvt$l_column_width to specific area.
 *
 * Bob Messenger	20-Jan-1989	X1.1-1
 *	- Return DECLRP with 0 event code if locator is disabled in DECRQLP.
 *
 * Bob Messenger	16-Jan-1989	X1.1-1
 *	- moved many ld fields to common area
 *
 * Bob Messenger	11-Jan-1989	X1.1-1
 *	- move wvt$b_cursts to common area
 *
 * Bob Messenger	06-Jan-1989	X1.1-1
 *	- Fix TBC with 3 parameter so it doesn't clobber the conformance
 *	  level (DECWINDOWS-IFT QAR #1395).
 *
 * Tom Porcher		20-Oct-1988	X0.5-4
 *	- Fix DECSCA with 0 parameter to turn "not erasable" attribute off.
 *	  Some dingbat defined the attribute in the renditions as "eraseable"
 *	  so this really means "set the bit".
 *
 * Bob Messenger	31-Aug-1988	X0.5-0
 *	- use non-conforming resize for DECSLPP and DECSCPP (i.e. don't
 *	  clear the screen).
 *
 * Mike Leibow		16-Aug-1988	X0.4-43
 *	- changing DECCOLM while in status display screwed things up bad.
 *	- added DECNKM to reset mode and set mode functions.
 *	- fixed KAM.  Didn't do anything.
 *
 * Mike Leibow          07-Jun-1988
 *      - Prepared code for status line.  Some ld fields are now accessed
 *        by _cld instead of -ld.
 *
 *    
 *		Tom Porcher		29-Dec-1987
 *	- Added callbacks for changing cursor key mode.
 *
 *    FGK0017	Frederick G. Kleinsorge	29-Jul-1987
 *  
 *    o Clear FF counter in DECSTBM.
 *    
 *    FGK0016	Frederick G. Kleinsorge	22-Jul-1987
 *  
 *    o Compute up scroll defer max
 *    
 *    FGK0015	Frederick G. Kleinsorge	11-May-1987
 *  
 *    o Finish DECCEAM mode SRM
 *    
 *    FGK0014	Frederick G. Kleinsorge	28-Apr-1987
 *  
 *    o Insert experimental select mask sequence and SRM for stream/rect
 *    
 *    FGK0013	Frederick G. Kleinsorge	17-Apr-1987
 *  
 *    o Mass edit symbol names to use standard VMS conventions
 *    
 *    FGK0012	Frederick G. Kleinsorge	16-Apr-1987
 *  
 *    o Change data type of ld, use macro "wvtp"
 *    
 *    FGK0011	Frederick G. Kleinsorge	13-Mar-1987
 *  
 *    o Only allow ANSI color changes if vt2_m_ansi_color is available
 *    
 *    FGK0010	Frederick G. Kleinsorge	10-Mar-1987
 *  
 *    o Tested wrong flag word for DECSLCS for lrp mode, use  flags_2
 *    
 *    FGK0009	Frederick G. Kleinsorge	05-Mar-1987
 *  
 *    o V3.2
 *    
 *    FGK0008	Frederick G. Kleinsorge	27-Feb-1987
 *  
 *    o Eliminate discrete display flush
 *    
 *    o Add transparent mode cut/paste enable/disable
 *
 *    FGK0007	Frederick G. Kleinsorge	20-Jan-1987
 *  
 *    o Allow DECTCEM (text cursor enable) in Level 1 mode, contrary
 *      to the SRM because the VT200 (and Panda) allow it.
 *    
 *    FGK0006	Frederick G. Kleinsorge	04-Dec-1986
 *  
 *    o LRP pointer pattern to distinguish cut/paste mode from LRP mode
 *
 *    o Add experimental cut/paste codes in the <CSI> . F block
 *
 *    o Reformat processing of controls with 2 intermediates
 *    
 *    o Insert VS-II experimental controls <CSI> . F
 *    
 *    FGK0005	Frederick G. Kleinsorge	09-Sep-1986
 *  
 *    o Conditionalize out PANDA functionality
 *
 *    FGK0004	Frederick G. Kleinsorge	22-Jul-1986
 *  
 *    o Update version to X04-017
 *    
 *    FGK0003	Frederick G. Kleinsorge	14-Jul-1986
 *    
 *    o Added DECEFR Filter Rectangle control sequence for locator mode.
 *    
 *    FGK0002	Frederick G. Kleinsorge	10-Jun-1986
 *    
 *    o Add csa_M_INVISIBLE attribute.
 *
 *    o Change level 2 checks to >= LEVEL2 to allow LEVEL3 to work.
 *
 *    o Add DECCAEM - Change Attribute Extent mode (set/reset)
 *
 *    o Add DECVSSM - Vertical Split Screen Scrolling mode (set/reset)
 *
 *    o Add LEFT/RIGHT margins.
 *
 *    FGK0001	Frederick G. Kleinsorge	09-Jun-1986
 *    
 *    o Added parse for DECCAD, DECCAL, DECRAD, DECRAL, DECERA, DECSRA & DECFRA
 *    
 *    o Modified seqparm to clear the all _ld wvt$l_parms[] array elements
 *    
 */

#include "wv_hdr.h"
/* #include "shrlib$:uisusrdef.h" */


/************************************************************/
xcntseq(ld)		/* Control Sequence dispatch table  */
/************************************************************/

wvtp ld;

{

int temp, temp2;

 /* dispatch control sequences */

 switch (_cld wvt$b_intercnt)

  {

  /* Control sequences with no intermediates */

  case 0:

    switch (_cld wvt$b_finalchar)
      {

      case '@': if (_cld wvt$b_conformance_level >= LEVEL2)
                xich(ld);     break;			/* insert character   */
      case 'A': xcuu(ld);     break;			/* cursor UP          */
      case 'B': xcud(ld);     break;			/* cursor DOWN        */
      case 'C': xcuf(ld);     break;			/* cursor FORWARD     */
      case 'D': xcub(ld);     break;			/* cursor BACKWARD    */
      case 'H': xcup(ld);     break;			/* cursor POSITION    */
      case 'J': xed(ld);      break;			/* erase  in display  */
      case 'K': xel(ld);      break;			/* erase  in line     */
      case 'L': xil(ld);      break;			/* insert line        */
      case 'M': xdl(ld);      break;			/* delete line        */
      case 'P': xdch(ld);     break;			/* delete character   */
      case 'U': xnp(ld);      break;                    /* NP - next page     */
      case 'V': xpp(ld);      break;                    /* PP - previous page */
      case 'X': if (_cld wvt$b_conformance_level >= LEVEL2)
                xech(ld);     break;			/* erase  character   */
      case 'c': xda(ld);      break;			/* device attributes  */
      case 'f': xcup(ld);     break;			/* cursor position    */
      case 'g': xtbc(ld);     break;			/* tab clear          */
      case 'h': xsm(ld);      break;			/* set mode           */
      case 'i': xmc(ld);      break;			/* media copy         */
      case 'l': xrm(ld);      break;			/* reset mode         */
      case 'm': xsgr(ld);     break;			/* sel graphic rend   */
      case 'n': xdsr(ld);     break;			/* Device Status Rep  */
      case 'p':	if ((_cld wvt$b_parmcnt == 0) || (_cld wvt$l_parms[0] == 0))
		     _ld wvt$b_single_shift = SSS_3_TCS; /* DECSSS TCS shift  */
		else _ld wvt$b_single_shift = ERROR_SHIFT; /* Next chr is error  */
			      break;			
      case 'r': xdecstbm(ld); break;			/* top&bottom margins */
#if EXTENDED_PANDA
      case 's': xdecslrm(ld); break;			/* left&right margins */
#endif
      case 't':						/* DECSLPP set lines  */

		if (_cld wvt$l_parms[0] == 0) _cld wvt$l_parms[0] = 24;
		if (_cld wvt$l_vt200_common_flags & vtc1_m_actv_status_display)
			break;
		WVT$SET_TERMINAL_SIZE(	ld,
					_ld wvt$l_column_width,
					_cld wvt$l_parms[0], 0);
			      break;

      default:                break;

      }

    break;



  /* Control sequences with one intermediate */

  case 1:

    switch (_cld wvt$b_inters[0])
       {
	case 32:

		switch (_cld wvt$b_finalchar)
		       {
		       case 'P': xppa(ld); break;       /* PPA - Page Position Absolute */
		       case 'Q': xppr(ld); break;       /* PPR - Page Position Relative */
		       case 'R': xppb(ld); break;       /* PPB - Page Position Backward */
		       default:  break;
		       }
		break;

	case '!':

		switch (_cld wvt$b_finalchar)
		       {
						/* Soft terminal reset        */
						/* CSI !p	- DECSTR      */
			case 'p': xstr(ld); break;

			default:  break;
		       }
		break;

	case 34:

		switch (_cld wvt$b_finalchar)
		       {
						/* Set conformance level      */
						/* CSI "p	- DECSCL      */
			case 'p': xscl(ld); break;

						/* Select Char attributes     */
						/* CSI "q	- DECSCA      */
			case 'q': if (_cld wvt$b_conformance_level >= LEVEL2)
				  xsca(ld); break;

			default:  break;
		       }
		break;

	case '#':

		break;

	case '$':

		switch (_cld wvt$b_finalchar)
		       {
						/* Request mode               */
						/* CSI ps $ p   - DECRQM      */
						/* CSI ? ps $ p - DECRQM      */
			case 'p': if (_cld wvt$b_conformance_level >= LEVEL2)
				  xdecrqm(ld); break;
						/* presentation state report */
						/* CSI ps $ w   - DECPSR      */
			case 'w': if (_cld wvt$b_conformance_level >= LEVEL2)
				  xdecrqpsr(ld); break;
			case 'u': if (_cld wvt$b_conformance_level >= LEVEL2)
				  xdecrqtsr(ld); break;
#if EXTENDED_PANDA
						/* Change attributes in Line  */
						/* CSI $q	- DECCAL      */
			case 'q': if (_cld wvt$b_conformance_level >= LEVEL2)
				  xcal(ld); break;

						/* Change attrib in display   */
						/* CSI $r	- DECCAD      */
			case 'r': if (_cld wvt$b_conformance_level >= LEVEL2)
				  xcad(ld); break;

						/* Reverse attrib in line     */
						/* CSI $s	- DECRAL      */
			case 's': if (_cld wvt$b_conformance_level >= LEVEL2)
				  xral(ld); break;

						/* Reverse attrib in display  */
						/* CSI $t 	- DECRAD      */
			case 't': if (_cld wvt$b_conformance_level >= LEVEL2)
				  xrad(ld); break;

						/* Fill rectangular area      */
						/* CSI $x 	- DECFRA      */
			case 'x': if (_cld wvt$b_conformance_level >= LEVEL2)
				  xfra(ld); break;

						/* Erase Rectangular Area     */
						/* CSI $z	- DECERA      */
			case 'z': if (_cld wvt$b_conformance_level >= LEVEL2)
				  xera(ld); break;

						/* Sel Erase Rectangular Area */
						/* CSI ${	- DECSERA     */
			case '{': if (_cld wvt$b_conformance_level >= LEVEL2)
				  xsera(ld); break;
#endif
						/* Copy Rectangular area      */
						/* CSI $v 	- DECCRA      */
			case 'v': if (_cld wvt$b_conformance_level >= LEVEL2)
				  xcra(ld); break;

						/* Set Columns per page       */
						/* CSI Pn $|	- DECSCPP     */
			case '|': if (_cld wvt$l_parms[0] == 0)
					_cld wvt$l_parms[0] = 80;
				    WVT$SET_TERMINAL_SIZE(
							ld,
							_cld wvt$l_parms[0],
							_ld wvt$l_page_length,
							0);
				  break;
						/* select active status display */
						/* CSI Ps $ }   - DECSASD     */
			case '}':
				xdecsasd(ld);
				break;
						/* select status display t*/
						/* CSI Ps $ ~   - DECSSDT     */
			case '~':
				xdecssdt(ld);
				break;

			default:  break;

		       }
		break;

	case '%':

		break;

	case '&':

		switch (_cld wvt$b_finalchar)
		       {
						/* Req. user pref set setting */
						/* CSI &u 	- DECRQUPSS   */
			case 'u':  if (_cld wvt$b_conformance_level >= LEVEL2)
		/* C1 control code depends on 7/8 bit node	*/
	if (_cld wvt$l_vt200_common_flags & vtc1_m_c1_transmission_mode)
			{
				   if (_cld wvt$b_user_preference_set == ISO_LATIN_1)
				       WVT$TERMINAL_TRANSMIT(ld,"\2201!uA\234");
				   else
				       WVT$TERMINAL_TRANSMIT(ld,"\2200!u%5\234");
			}
		else	{
				   if (_cld wvt$b_user_preference_set == ISO_LATIN_1) 
				       WVT$TERMINAL_TRANSMIT(ld,"\033P1!uA\033\\");
				   else
				       WVT$TERMINAL_TRANSMIT(ld,"\033P0!u%5\033\\");
			}
				   break;

						/* CSI &x - Enable Session */
			case 'x':  WVT$ENABLE_SESSION(ld);
				   break;

			default:  break;

		       }
		break;

	case 39:

		switch (_cld wvt$b_finalchar)
		       {
						/* Enable Filter rectangle    */
						/* CSI 'w 	- DECEFR      */
			case 'w': if (_cld wvt$b_conformance_level >= LEVEL2)
				  xdecefr(ld); break;

						/* Enable Locator Reports     */
						/* CSI 'z	- DECELR      */
			case 'z': if (_cld wvt$b_conformance_level >= LEVEL2)
				  xdecelr(ld); break;

						/* Select Locator Events      */
						/* CSI '{	- DECSLE      */
			case '{': if (_cld wvt$b_conformance_level >= LEVEL2)
				  xdecsle(ld); break;

						/* Request Location (poll)    */
						/* CSI '|	- DECRQLP     */
			case '|': if ((_cld wvt$b_conformance_level >= LEVEL2)) {
				    if (_cld wvt$l_vt200_common_flags &
				      vtc1_m_locator_report_mode)
					WVT$REQUEST_LOCATION(ld);
				    else
					WVT$LOCATOR_ERROR_REPORT(ld);
				  }
				  break;
#if EXTENDED_PANDA
						/* Insert Column              */
						/* CSI '}	- DECIC       */
			case '}': if (_cld wvt$b_conformance_level >= LEVEL2)
				  xicol(ld); break;

						/* Delete Column              */
						/* CSI '~	- DECDC       */
			case '~': if (_cld wvt$b_conformance_level >= LEVEL2)
				  xdcol(ld); break;
#endif
			default:  break;

		       }
		break;

	case '(':

		break;

	case ')':

		switch (_cld wvt$b_finalchar)
		       {

						/* Select trans Paste format  */
						/* CSI )s 	- DECSTPF     */
			case 's': xdecstpf(ld); break;

						/* Transmit Paste Buffer      */
						/* CSI )t 	- DECTTPB     */
			case 't': xdecttpb(ld); break;

						/* Select Loc Cursor Style    */
						/* CSI )u 	- DECSLCS     */
			case 'u': xdecslcs(ld); break;

						/* Request device color capab.*/
						/* CSI ) }      - DECRQDCC    */
			case '}': xdecrcs(ld); break;
			default:  break;

		       }

		break;

	case '*':

		switch (_cld wvt$b_finalchar)
		       {
			case '}': xdeclfkc(ld); break;
		       }

		break;


	case '+':
	case ',':
	case '-':

		break;

	case '.':
#ifdef VSII_EXPERIMENTAL
		/* VS-II Experimental CSI Block */

		switch (_cld wvt$b_finalchar)
		       {

			case 'p':

				/*
				 *  Set stream select mask
				 */
				temp = _ld wvt$w_actv_rendition;

				_ld wvt$w_actv_rendition =
					_ld wvt$l_select_mask;

				temp2 = xsgr(ld);

				_ld wvt$l_select_mask =
					(_ld wvt$w_actv_rendition &
					  ( csa_M_REVERSE|
					    csa_M_BOLD|
					    csa_M_UNDERLINE|
					    MASK_TEXT|
					    MASK_TEXT_BCK ));

				if (temp2 & 1)
					_ld wvt$l_select_mask |= hlm_M_HIGH_FORE;

				if (temp2 & 2)
					_ld wvt$l_select_mask |= hlm_M_HIGH_BACK;

				if (!_ld wvt$l_select_mask)
					_ld wvt$l_select_mask = csa_M_REVERSE;

				_ld wvt$l_select_mask <<= 16; /* high word */

				_ld wvt$w_actv_rendition = temp;
				break;

			case 'q':
			case 'r':
			case 's':
			case 't':
			case 'u':
			case 'v':
			case 'w':
			case 'x':
			case 'y':
			case 'z':
			case '{':
			case '|':
			case '}':
			case '~':
			default:  break;

		       }
#endif
		break;
	case '/':

		break;

	default: break;
       }

    break;

  default: break;

  }

return(0);
}

/************************************************************/
seqparm(ld, state, start, final) /* get sequence parameters */
/************************************************************/

wvtp ld;
struct parse_stack *state;
int start, final;

/*
 *
 * seqparm - get sequence parameters
 *
 *	This routine massages the information returned on a parse stack
 *	and stores it in local variables within the current display context
 *
 * outputs:
 *	wvt$b_privparm	The private parameter char value - 64 or zero if none
 *	wvt$l_parms[]		The individual numeric parameters or zero if omitted
 *	wvt$b_parmcnt		The number of numeric parameters including omitted
 *	wvt$b_inters[]	The individual intermediates at end of seq
 *	wvt$b_intercnt	The number of intermediates
 *	wvt$b_finalchar	The final char value
 *
 */

{

 /*
  *	get private parameter if any
  */

 _cld wvt$b_privparm = 0;
 start += 1;

 if (state->value[start] == CS_PRIVATE)
   {
    _cld wvt$b_privparm = 64 - state->data[start];
    start += 1;
   }

 /*
  *	collect numeric parameters
  */

 for (_cld wvt$b_parmcnt = 0; _cld wvt$b_parmcnt < MAXPARMS;_cld wvt$b_parmcnt++)
    _cld wvt$l_parms[_cld wvt$b_parmcnt] = 0;

 for (_cld wvt$b_parmcnt=0; _cld wvt$b_parmcnt < MAXPARMS; _cld wvt$b_parmcnt++)
    {
     if ((state->value[start] == CS_PARAM) ||
	 (state->value[start] == CS_OMIT))
	{
         _cld wvt$l_parms[_cld wvt$b_parmcnt] = state->data[start];
         start += 1;
        }
     else break;
    }

 /*
  *	get intermediates
  */

 for (_cld wvt$b_intercnt=0; _cld wvt$b_intercnt<MAXINTERS; _cld wvt$b_intercnt++)
    {
     if (state->value[start] == CS_INTER)
       {
        _cld wvt$b_inters[_cld wvt$b_intercnt] = state->data[start];
        start += 1;
       }
     else break;
    }

 /*
  *	get sequence final
  */

 _cld wvt$b_finalchar = state->data[final];

}

/***********************************/
xsm(ld) /* Set Mode dispatch table */
/***********************************/

wvtp ld;

{

register int n, y;

 if (_cld wvt$b_parmcnt == 0) _cld wvt$b_parmcnt = 1;
 for (n=1; n<=_cld wvt$b_parmcnt; n++) /* check all parameters */
  {
  switch (_cld wvt$b_privparm)
    {

    case 0:	/*
		 *  ANSI standard terminal modes.
		 *
		 */

      switch (_cld wvt$l_parms[n-1])
        {
        case 2:	/*
		 *  KAM -
		 *
		 *
		 */
		 if (!(_cld wvt$l_vt200_common_flags & vtc1_m_feature_lock)) {
			_cld wvt$l_vt200_common_flags |= vtc1_m_kbd_action_mode;
			WVT$LOCK_KEYBOARD(ld);
		 }
		 break;

        case 3:	/*
		 *  CRM -
		 *
		 *  Control Representation Mode
		 *
		 */
		 _cld wvt$l_vt200_flags |= vt1_m_display_controls_mode;
		 if ( _cld wvt$l_ext_flags & vte1_m_asian_common )
			WVT$CONTROL_FONT(ld);
		 break;
        case 4:	/*
		 *  IRM -
		 *
		 *  Enter INSERT mode
		 *
		 */

		 _cld wvt$l_vt200_common_flags |= vtc1_m_insert_mode;
                 _ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;
		 break;

        case 12:/*
		 *  SRM -
		 *
		 *  Echo Mode (send/receive mode)
		 *
		 */

		 _cld wvt$l_vt200_common_flags &= ~vtc1_m_echo_mode;
		{
		DECtermWidget w = ld_to_w(ld);

		w->common.localEcho = False;
		}
		 break;

        case 20:/*
		 *  LNM -
		 *
		 *  New Line Mode
		 *  
		 *
		 */

		 _cld wvt$l_vt200_flags |= vt1_m_new_line_mode;
		 break;

        default: break;
        }
      break;

    case 1:   	/*
		 *  DEC PRIVATE MODES (CSI ? pn h)
		 *
		 */

      switch (_cld wvt$l_parms[n-1])
        {

        case 1:	/*
		 *  DECCKM - Cursor Key mode
		 *
		 *
		 */

		 _cld wvt$l_vt200_common_flags |= vtc1_m_cursor_key_mode;
		 break;

	case 2:	/*
		 *  DECANM - ANSI mode
		 *
		 *
		 */

		 if (_cld wvt$l_vt200_common_flags & vtc1_m_actv_status_display)
		     return;	/* ignore if in status line */
		 _cld wvt$l_vt200_flags |= vt1_m_ansi_mode;
		 _ld wvt$b_gl = 0;

		 if ( _cld wvt$l_ext_flags & vte1_m_tomcat ) {

		    _ld wvt$b_gr = 1;

		    if (_cld wvt$l_ext_specific_flags & vte2_m_jisroman_mode) {
			_ld wvt$b_g_sets[0] =     JIS_ROMAN;
			_ld wvt$b_ext_g_sets[0] = ONE_BYTE_SET;
		    } else {
			_ld wvt$b_g_sets[0] =     ASCII;
			_ld wvt$b_ext_g_sets[0] = STANDARD_SET;
		    }
		    _ld wvt$b_g_sets[1] =     JIS_KATAKANA;
		    _ld wvt$b_ext_g_sets[1] = ONE_BYTE_SET;
		 } else {
		    if ( _cld wvt$l_ext_flags & vte1_m_dickcat &&
			 _cld wvt$l_ext_specific_flags & vte2_m_ksroman_mode ) {
			_ld wvt$b_g_sets[0] = KS_ROMAN;
			_ld wvt$b_ext_g_sets[0] = ONE_BYTE_SET;
		    } else {
			_ld wvt$b_g_sets[0] = ASCII;
			_ld wvt$b_ext_g_sets[0] = STANDARD_SET;
		    }
		    _ld wvt$b_g_sets[1] = LINE_DRAWING;
		    _ld wvt$b_ext_g_sets[1] = STANDARD_SET;
		 }
		 break;

        case 3:	/*
		 *  DECCOLM -
		 *
		 *  If conforming, then always change to 132 column mode.
		 *
		 *  If NOT conforming, change to 132 column mode ONLY if
		 *  the current width is less than 81.  Otherwise
		 *  do a change width using the current width (clear the
		 *  display and reset a couple parameters).
		 *
		 *  This hack is provided to try to get around the fact
		 *  DCL does both a QIO setmode AND a DECCOLM sequence.
		 *
		 *  When DCL is corrected, this should set the page length
		 *  to 24 and the width to 132 and use a 1 for the resize
		 *  flag (clear the display)!
		 */
		 {
		 int in_status_display;

		 in_status_display = (_cld wvt$l_vt200_common_flags &
			vtc1_m_actv_status_display) != 0;
		 
		 if (in_status_display) WVT$MAIN_DISPLAY(ld);

		 if (!(_cld wvt$l_flags & vt4_m_conforming_wid))
		    {
			if (_ld wvt$l_column_width < 81) 
			  WVT$SET_TERMINAL_SIZE(ld, 132,
						_ld wvt$l_page_length,
						_cld wvt$b_conforming_resize);
			else
			  WVT$SET_TERMINAL_SIZE(ld, _ld wvt$l_column_width,
						_ld wvt$l_page_length,
						_cld wvt$b_conforming_resize);
		    }
                 else WVT$SET_TERMINAL_SIZE(ld, 132,
						_ld wvt$l_page_length,
						_cld wvt$b_conforming_resize);

		 if (in_status_display) WVT$STATUS_DISPLAY(ld);
                 _ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;

/* note: the following code is specific to DECterm */

		if ( ld->common.adjustFontSizes && ! ld->common.condensedFont &&
		    (ld->common.fontSetSelection != DECwGSFont))
		    {
		    ld->common.condensedFont = TRUE;
		    o_set_value_condensedFont( ld, ld );
		    }
		 }

                 break;

        case 4:	/*
		 *  DECSCLM - Scroll mode
		 *
		 *
		 */

		  if (!(_cld wvt$l_vt200_common_flags & vtc1_m_feature_lock))
                  {
                   _cld wvt$l_vt200_flags |= vt1_m_scroll_mode;
                   WVT$SMOOTH_SCROLL(ld);
                  }
		 break;

        case 5:	/*
		 *  DECSCNM - Screen mode
		 *
		 *
		 */

		 if (!(_cld wvt$l_vt200_common_flags & vtc1_m_feature_lock)) 
                  {
		 if ( _cld wvt$l_ext_flags & vte1_m_asian_common ) {
		   if ((_cld wvt$l_vt200_common_flags & vtc1_m_screen_mode))
			WVT$REVERSE_VIDEO(ld);
                   _cld wvt$l_vt200_common_flags &= ~vtc1_m_screen_mode;
		 } else {
                   if (!(_cld wvt$l_vt200_common_flags & vtc1_m_screen_mode))
			WVT$REVERSE_VIDEO(ld);
                   _cld wvt$l_vt200_common_flags |= vtc1_m_screen_mode;
		 }
                  }
		 break;

        case 6:	/*
		 *  DECOM - Origin Mode
		 *
		 *
		 */

		 _ld wvt$l_vt200_specific_flags |= vts1_m_origin_mode;
                 _ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;
                 _ld wvt$l_actv_line = _ld wvt$l_top_margin;
		 _ld wvt$l_actv_column = _ld wvt$l_left_margin;
                 break;

	case 7:	/*
		 *  DECAWM - Auto-wrap mode
		 *
		 *
		 */

		 _ld wvt$l_vt200_specific_flags |= vts1_m_auto_wrap_mode;
		 break;

        case 8:	/*
		 *  DECARM - Auto-repeat mode
		 *
		 *
		 */

#if 0	/* disable this feature */
		  if (!(_cld wvt$l_vt200_common_flags & vtc1_m_feature_lock))
                  {
                   _cld wvt$l_kb_attributes |= UIS$M_KB_AUTORPT;
                   WVT$SET_KB_ATTRIBUTES(ld);
                  }
#endif
		 break;

	case 9:	/*
		 *  INTERLACE - NOT SUPPORTED - NOT MEANINGFUL
		 *
		 *
		 */

		 break;

        case 18:/*
		 *  DECPFF - Print Form-Feed
		 *
		 *
		 */

		 if (_cld wvt$w_print_flags & pf1_m_prt_enabled) 
			_cld wvt$w_print_flags |= pf1_m_prt_ff_mode;
		 break;

        case 19:/*
		 *  DECPEX - Print Extent
		 *
		 *
		 */

		 if (_cld wvt$w_print_flags & pf1_m_prt_enabled) 
			{
			_cld wvt$w_print_flags |= pf1_m_prt_extent_mode;
			ld->common.printExtent = DECwFullPage;
			}
		 break;

        case 25:/*
		 *  DECTCEM - Cursor Enable
		 *
		 *
		 */

		 /*
		  * The VSRM specifies this as a Level 2 function, but
		  * existing terminals seem to honor it in VT100 mode
		  * as well.  No-op the conformance level check.
		  *
		  * if (_cld wvt$b_conformance_level >= LEVEL2)
		  *
		  */

                 _cld wvt$b_cursts |= cs1_m_cs_enabled;
		 WVT$ENABLE_CURSOR(ld);
                 break;

	case 34:/*
		 *  DECLRM - Right to Left/Left to Right 
		 *
		 *
		 */

		 _cld wvt$l_ext_specific_flags |= vte2_m_rtl;
		 break;

	case 35:/*
		 *  DECHEBM - English/Hebrew keyboard layout
		 *
		 *
		 */
		 _cld wvt$l_ext_specific_flags |= vte2_m_kb_soft_switch;
		 if ( _cld wvt$l_ext_flags & vte1_m_hebrew )
		 DwtDECtermToggleKeyboard(ld, TRUE);
		 break;

	case 36:/*
		 *  DECHEM - NRC/Multinational mode 
		 *
		 *
		 */

		 _cld wvt$l_vt200_flags |= vt1_m_nrc_mode;
		 break;

	case 42:/*
		 *  DECNRCM - NRC/Multinational mode 
		 *
		 *
		 */

		 _cld wvt$l_vt200_flags |= vt1_m_nrc_mode;
		 break;

	case 59:/*
		 *  DECKKDM - Kanji/katakana display mode 
		 *
		 *
		 */
		 _cld wvt$l_ext_specific_flags |= vte2_m_kanji_mode;
/*		 if (_cld wvt$l_ext_flags & vte1_m_tomcat)	*/
/* 		 allow designation in all terminal type		*/
		 {
			_ld wvt$b_gl = 0;
			_ld wvt$b_gr = 3;
			if (_cld wvt$l_ext_specific_flags & vte2_m_jisroman_mode)
			{
			    _ld wvt$b_g_sets[0] =     JIS_ROMAN;
			    _ld wvt$b_ext_g_sets[0] = ONE_BYTE_SET;
			} else {
			    _ld wvt$b_g_sets[0] =     ASCII;
			    _ld wvt$b_ext_g_sets[0] = STANDARD_SET;
			}
			_ld wvt$b_g_sets[1] =     LINE_DRAWING;
			_ld wvt$b_ext_g_sets[1] = STANDARD_SET;
			_ld wvt$b_g_sets[2] =     JIS_KATAKANA;
			_ld wvt$b_ext_g_sets[2] = ONE_BYTE_SET;
			_ld wvt$b_g_sets[3] =     DEC_KANJI;
			_ld wvt$b_ext_g_sets[3] = TWO_BYTE_SET;
		 }
		 break;

#if EXTENDED_PANDA
	case 63:/*
		 *  DECCAEM - Change Attribute Extent Mode
		 *
		 */

		 _cld wvt$l_vt200_flags_3 |= vt3_m_ch_attr_extent_mode;
		 break;
#endif

	case 66:/*
		 *
		 * DECNKM - numeric keypad mode
		 */

		_cld wvt$l_vt200_common_flags |= vtc1_m_keypad_mode;
		WVT$SET_MISC_MODES(ld);
                break;

	case 67:/*
		 *  DECBKM - Backarrow mode
		 *
		 *  Reset state means DEL transmitted when <X] key is
		 *  pressed.
		 *
		 */

		 _cld wvt$l_vt200_flags_2 |= vt2_m_backarrow_mode;
		 break;

#if EXTENDED_PANDA
	case 69:/*
		 *  DECVSSM - Vertical Split Screen Mode
		 *
		 *  Vertical Split Screen Mode:  Set the flag
		 *  reset all lines to SINGLE WIDTH (refresh any
		 *  line that changes).
		 *
		 */

		 _cld wvt$l_vt200_flags_2 |= vt2_m_vss_scroll_mode;

		 for (y = 1; y <= _ld wvt$l_page_length; y++)
		   {
			if (line_rendition(y) == SINGLE_WIDTH) continue;

			line_rendition(y) = SINGLE_WIDTH;
			line_width(y) = _ld wvt$l_column_width;
			/* re-display line */
			display_segment(ld, y, 1, _ld wvt$l_column_width);

		   }

		 break;
#endif

	case 76:/*
		 *  DECTPM - Enable Transparent Paste Operations
		 *
		 */

		 _cld wvt$l_vt200_flags |= vt1_m_enable_paste;
		 break;

	case 77:/*
		 *  DECTCM - Enable Transparent Cut Operations
		 *
		 */

		 _cld wvt$l_vt200_flags_2 |= vt2_m_enable_cut;
		 break;

	case 80:/*                       
		 *  DECSDM - Sixel scroll
		 *
		 */
		 _cld wvt$l_ext_specific_flags |= vte2_m_sixel_scroll_mode;
		 break;

	case 88:/*
		 *  **** Experimental set mode for stream select
		 *
		 */

		 _cld wvt$l_flags |= vt4_m_edit_select;
		 break;

	case 89:/*
		 *  **** Experimental set mode for trimming trailing unwritten
		 *  characters
		 */

		 _cld wvt$l_flags |= vt4_m_select_trim;
		 break;

	case 92:/*
		 *  DECLCSM - Leading Code Supressing Mode
		 *
		 */
		 _cld wvt$l_ext_specific_flags |= vte2_m_leading_code_mode;
		 break;

        default: break;

        }

      break;

    default: break;
    }
  }

  WVT$SET_MISC_MODES( ld );
}

/*************************************/
xrm(ld) /* Reset Mode dispatch table */
/*************************************/

wvtp ld;

{

int n;

 if (_cld wvt$b_parmcnt == 0) _cld wvt$b_parmcnt = 1;
 for (n=1; n<=_cld wvt$b_parmcnt; n++) /* check all parameters */
  {
  switch (_cld wvt$b_privparm)
    {
    case 0:	/*
		 *  ANSI standard terminal modes.
		 *
		 */

      switch (_cld wvt$l_parms[n-1])
        {

        case 2:	/*
		 *  KAM -
		 *
		 *
		 */
		 if (!(_cld wvt$l_vt200_common_flags & vtc1_m_feature_lock)) {
			_cld wvt$l_vt200_common_flags &= ~vtc1_m_kbd_action_mode;
			WVT$UNLOCK_KEYBOARD(ld);
		 }
		 break;

        case 3:	/*
		 *  CRM -
		 *
		 *  Control Representation Mode
		 *
		 */
		 _cld wvt$l_vt200_flags &= ~vt1_m_display_controls_mode;
		 if ( _cld wvt$l_ext_flags & vte1_m_asian_common )
			WVT$RECALL_FONT(ld);
		 break;

        case 4:	/*
		 *  IRM -
		 *
		 *  Enter REPLACE Mode (INSERT MODE = FALSE)
		 *
		 */

		 _cld wvt$l_vt200_common_flags &= ~vtc1_m_insert_mode;
		 _ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;
		 break;

	case 12:/*
		 *  SRM -
		 *
		 *  Echo Mode (send/receive mode) Not Supported
		 *
		 */

		 _cld wvt$l_vt200_common_flags |= vtc1_m_echo_mode;
		{
		DECtermWidget w = ld_to_w(ld);

		w->common.localEcho = True;
		}
		 
		 break;

        case 20:/*
		 *  LNM -
		 *
		 *  New Line Mode
		 *  
		 *
		 */

		 _cld wvt$l_vt200_flags &= ~vt1_m_new_line_mode;
		 break;

        default: break;
        }
      break;

    case 1:   	/*
		 *  DEC PRIVATE MODES (CSI ? pn l)
		 *
		 */

      switch (_cld wvt$l_parms[n-1])
        {

        case 1:	/*
		 *  DECCKM -
		 *
		 *
		 */

 		 _cld wvt$l_vt200_common_flags &= ~vtc1_m_cursor_key_mode;
		 break;

	case 2:	/*
		 *  DECANM -
		 *
		 *
		 */

		 if (_cld wvt$l_vt200_common_flags & vtc1_m_actv_status_display)
		     return;	/* ignore if in status line */
		 _cld wvt$l_vt200_flags &= ~(vt1_m_ansi_mode |
					    vt1_m_vt52_cursor_seq);
		 _ld wvt$b_gl = 0;

		 if ( _cld wvt$l_ext_flags & vte1_m_tomcat ) {

		    _ld wvt$b_gr = 1;

		    if (_cld wvt$l_ext_specific_flags & vte2_m_jisroman_mode) {
			_ld wvt$b_g_sets[0] =     JIS_ROMAN;
			_ld wvt$b_ext_g_sets[0] = ONE_BYTE_SET;
		    } else {
			_ld wvt$b_g_sets[0] =     ASCII;
			_ld wvt$b_ext_g_sets[0] = STANDARD_SET;
		    }
		    _ld wvt$b_g_sets[1] =     JIS_KATAKANA;
		    _ld wvt$b_ext_g_sets[1] = ONE_BYTE_SET;
		 } else {
		    if ( _cld wvt$l_ext_flags & vte1_m_dickcat &&
			 _cld wvt$l_ext_specific_flags & vte2_m_ksroman_mode ) {
			_ld wvt$b_g_sets[0] = KS_ROMAN;
			_ld wvt$b_ext_g_sets[0] = ONE_BYTE_SET;
		    } else {
			_ld wvt$b_g_sets[0] = ASCII;
			_ld wvt$b_ext_g_sets[0] = STANDARD_SET;
		    }
		    _ld wvt$b_g_sets[1] = LINE_DRAWING;
		    _ld wvt$b_ext_g_sets[1] = STANDARD_SET;
		 }
		 WVT$SET_TERMINAL_MODE(ld);
		 break;

        case 3:	/*
		 *  DECCOLM -
		 *
		 *  If conforming, then always change to 80 column mode.
		 *
		 *  If NOT conforming, change to 80 column mode ONLY if
		 *  the current width is greater than 80.  Otherwise
		 *  do a change width using the current width (clear the
		 *  display and reset a couple parameters).
		 *
		 *  This hack is provided to try to get around the fact
		 *  DCL does both a QIO setmode AND a DECCOLM sequence.
		 *
		 *  When DCL is corrected, this should set the page length
		 *  to 24 and the width to 80 and use a 1 for the resize
		 *  flag (clear the display)!
		 */
		 {
		 int in_status_display;

		 in_status_display = (_cld wvt$l_vt200_common_flags &
			vtc1_m_actv_status_display) != 0;
		 
		 if (in_status_display) WVT$MAIN_DISPLAY(ld);

		 if (!(_cld wvt$l_flags & vt4_m_conforming_wid))
		    {
			if (_ld wvt$l_column_width > 80) 
			   WVT$SET_TERMINAL_SIZE(ld, 80,
						_ld wvt$l_page_length,
						_cld wvt$b_conforming_resize);
			else
			   WVT$SET_TERMINAL_SIZE(ld, _ld wvt$l_column_width,
						_ld wvt$l_page_length,
						_cld wvt$b_conforming_resize);
		    }
                 else WVT$SET_TERMINAL_SIZE(ld, 80,
						_ld wvt$l_page_length,
						_cld wvt$b_conforming_resize);

		 if (in_status_display) WVT$STATUS_DISPLAY(ld);

                 _ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;

/* note: the following code is specific to DECterm */

		if ( ld->common.adjustFontSizes && ld->common.condensedFont )
		    {
		    ld->common.condensedFont = FALSE;
		    o_set_value_condensedFont( ld, ld );
		    }
		 }
                 break;

        case 4:	/*
		 *  DECSCLM -
		 *
		 *
		 */

		 if (!(_cld wvt$l_vt200_common_flags & vtc1_m_feature_lock))
                  {
                   _cld wvt$l_vt200_flags &= ~vt1_m_scroll_mode;
                   WVT$JUMP_SCROLL(ld);
                  }
		 break;

        case 5:	/*
		 *  DECSCNM -
		 *
		 *
		 */

		 if (!(_cld wvt$l_vt200_common_flags & vtc1_m_feature_lock))
                  {
		 if ( _cld wvt$l_ext_flags & vte1_m_asian_common ) {
		   if (!(_cld wvt$l_vt200_common_flags & vtc1_m_screen_mode))
			WVT$REVERSE_VIDEO(ld);
                   _cld wvt$l_vt200_common_flags |= vtc1_m_screen_mode;
		 } else {
		   if ((_cld wvt$l_vt200_common_flags & vtc1_m_screen_mode))
			WVT$REVERSE_VIDEO(ld);
                   _cld wvt$l_vt200_common_flags &= ~vtc1_m_screen_mode;
		}
                  }
		 break;

        case 6:	/*
		 *  DECOM -
		 *
		 *
		 */

		 _ld wvt$l_vt200_specific_flags &= ~vts1_m_origin_mode;
		 _ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;
                 _ld wvt$l_actv_line = _ld wvt$l_actv_column = 1;
                 break;

	case 7:	/*
		 *  DECAWM -
		 *
		 *
		 */

		 _ld wvt$l_vt200_specific_flags &= ~vts1_m_auto_wrap_mode;
		 break;

        case 8:	/*
		 *  DECARM -
		 *
		 *
		 */

#if 0	/* disable this feature */
		 if (!(_cld wvt$l_vt200_common_flags & vtc1_m_feature_lock))
                  {
                   _cld wvt$l_kb_attributes &= ~UIS$M_KB_AUTORPT;
                   WVT$SET_KB_ATTRIBUTES(ld);
                   _ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;
                  }
#endif
		 break;

	case 9:	/*
		 *  INTERLACE - NOT SUPPORTED - NOT MEANINGFUL
		 *
		 *
		 */

		 break;

        case 18:/*
		 *  DECPFF -
		 *
		 *
		 */

		 if (_cld wvt$w_print_flags & pf1_m_prt_enabled)
			_cld wvt$w_print_flags &= ~pf1_m_prt_ff_mode;
		 break;

        case 19:/*
		 *  DECPEX -
		 *
		 *
		 */

		 if (_cld wvt$w_print_flags & pf1_m_prt_enabled)
			{
			_cld wvt$w_print_flags &= ~pf1_m_prt_extent_mode;
			ld->common.printExtent = DECwScrollRegionOnly;
			}
		 break;

        case 25:/*
		 *  DECTCEM -
		 *
		 *
		 */

		 /*
		  * The VSRM specifies this as a Level 2 function, but
		  * existing terminals seem to honor it in VT100 mode
		  * as well.  No-op the conformance level check.
		  *
		  * if (_cld wvt$b_conformance_level >= LEVEL2)
		  *
		  */

                 _cld wvt$b_cursts &= ~cs1_m_cs_enabled;
		 WVT$DISABLE_CURSOR(ld);
                 break;

	case 34:/*
		 *  DECLRM - Right to Left/Left to Right 
		 *
		 *
		 */

		 _cld wvt$l_ext_specific_flags &= ~vte2_m_rtl;
		 break;

	case 35:/*
		 *  DECHEBM - English/Hebrew keyboard layout
		 *
		 *
		 */
		 _cld wvt$l_ext_specific_flags |= vte2_m_kb_soft_switch;
		 if ( _cld wvt$l_ext_flags & vte1_m_hebrew )
		 DwtDECtermToggleKeyboard(ld, FALSE);
		 break;

	case 36:/*
		 *  DECHEM - NRC/Multinational mode 
		 *
		 *
		 */

		 _cld wvt$l_vt200_flags |= vt1_m_nrc_mode;
		 break;

	case 42:/*
		 *  DECNRCM -
		 *
		 *
		 */

		 _cld wvt$l_vt200_flags &= ~vt1_m_nrc_mode;
		 break;

	case 59:/*
		 *  DECKKDM - Kanji/katakana display mode 
		 *
		 *
		 */
		 _cld wvt$l_ext_specific_flags &= ~vte2_m_kanji_mode;
/*		 if (_cld wvt$l_ext_flags & vte1_m_tomcat)	*/
/* 		 allow designation in all terminal type		*/
		 {
			_ld wvt$b_gl = 0;
			_ld wvt$b_gr = 2;
			if (_cld wvt$l_ext_specific_flags & vte2_m_jisroman_mode)
			{
			    _ld wvt$b_g_sets[0] =     JIS_ROMAN;
			    _ld wvt$b_ext_g_sets[0] = ONE_BYTE_SET;
			} else {
			    _ld wvt$b_g_sets[0] =     ASCII;
			    _ld wvt$b_ext_g_sets[0] = STANDARD_SET;
			}
			_ld wvt$b_g_sets[1] =     JIS_KATAKANA;
			_ld wvt$b_ext_g_sets[1] = ONE_BYTE_SET;
			_ld wvt$b_g_sets[2] =     JIS_KATAKANA;
			_ld wvt$b_ext_g_sets[2] = ONE_BYTE_SET;
			_ld wvt$b_g_sets[3] =     LINE_DRAWING;
			_ld wvt$b_ext_g_sets[3] = STANDARD_SET;
		 }
		 break;

#if EXTENDED_PANDA
	case 63:/*
		 *  DECCAEM - Change Attribute Extent Mode
		 *
		 *
		 */

		 _cld wvt$l_vt200_flags_3 &= ~vt3_m_ch_attr_extent_mode;
		 break;
#endif

	case 66:/*
		 *
		 * DECNKM - numeric keypad mode
		 */

		_cld wvt$l_vt200_common_flags &= ~vtc1_m_keypad_mode;
		WVT$SET_MISC_MODES(ld);
                break;

	case 67:/*
		 *  DECBKM - Backarrow mode
		 *
		 *  Reset state means DEL transmitted when <X] key is
		 *  pressed.
		 *
		 */

		 _cld wvt$l_vt200_flags_2 &= ~vt2_m_backarrow_mode;
		 break;

#if EXTENDED_PANDA
	case 69:/*
		 *  DECVSSM - Vertical Split Screen Mode
		 *
		 *  Reset state flags vertical scrolling OFF and
		 *  sets the margins to the extremes of the page
		 *
		 */

		 _cld wvt$l_vt200_flags_2 &= ~vt2_m_vss_scroll_mode;
		 _ld wvt$l_left_margin = 1;
		 _ld wvt$l_right_margin = _ld wvt$l_column_width;
		 break;
#endif

	case 76:/*
		 *  DECTPM - Disable Transparent Paste Operations
		 *
		 */

		 _cld wvt$l_vt200_flags &= ~vt1_m_enable_paste;
		 break;

	case 77:/*
		 *  DECTCM - Disable Transparent Cut Operations
		 *
		 */

		 _cld wvt$l_vt200_flags_2 &= ~vt2_m_enable_cut;
		 break;

	case 80:/*                       
		 *  DECSDM - Sixel scroll
		 *
		 */
		 if (_cld wvt$l_ext_flags & vte1_m_asian_common)
			_cld wvt$l_ext_specific_flags &= ~vte2_m_sixel_scroll_mode;
		 break;

	case 88:/*
		 *  **** Experimental reset mode for stream select
		 *
		 */

		 _cld wvt$l_flags &= ~vt4_m_edit_select;
		 break;

	case 89:/*
		 *  **** Experimental reset mode for select trim of trailing
		 *  unwritten characters
		 */

		 _cld wvt$l_flags &= ~vt4_m_select_trim;
		 break;

	case 92:/*
		 *  DECLCSM - Leading Code Supressing Mode
		 *
		 */
		 _cld wvt$l_ext_specific_flags &= ~vte2_m_leading_code_mode;
		 break;

        default: break;

        }

      break;

    default: break;
    }
  }

  WVT$SET_MISC_MODES( ld );
}

/*************************************/
xsgr(ld) /* Select Graphic Rendition */
/*************************************/

wvtp ld;

{

int n, temp;

temp = 0;

if (_cld wvt$b_parmcnt == 0) _cld wvt$b_parmcnt = 1;
if (_cld wvt$b_privparm == 0)
for (n=1; n<=_cld wvt$b_parmcnt; n++)
  {
  if (_cld wvt$l_parms[n-1] < 30)
  {
   switch (_cld wvt$l_parms[n-1])
    {
    case 0: _ld wvt$w_actv_rendition &= csa_M_SELECTIVE_ERASE;
	    temp = 0; break;

    case 1: _ld wvt$w_actv_rendition |= csa_M_BOLD;		break;
    case 4: _ld wvt$w_actv_rendition |= csa_M_UNDERLINE;	break;
    case 5: _ld wvt$w_actv_rendition |= csa_M_BLINK;		break;
    case 7: _ld wvt$w_actv_rendition |= csa_M_REVERSE;		break;
#if 0	/* we use this bit for something else */
    case 8: _ld wvt$w_actv_rendition |= csa_M_INVISIBLE;	break;
#endif

    /* We don't have to let these through in LEVEL 1 mode, but we will */

    case 22:_ld wvt$w_actv_rendition &= ~csa_M_BOLD;		break;
    case 24:_ld wvt$w_actv_rendition &= ~csa_M_UNDERLINE;	break;
    case 25:_ld wvt$w_actv_rendition &= ~csa_M_BLINK;		break;
    case 27:_ld wvt$w_actv_rendition &= ~csa_M_REVERSE;		break;
#if 0	/* we use this bit for something else */
    case 28:_ld wvt$w_actv_rendition &= ~csa_M_INVISIBLE;	break;
#endif

    default: break;
    }
  }
  else
  {
   if (_cld wvt$l_vt200_flags_2 & vt2_m_ansi_color)
   {

   if (_cld wvt$l_parms[n-1] < 40) temp |= 1;
	else if (_cld wvt$l_parms[n-1] < 50) temp |= 2;

   switch (_cld wvt$l_parms[n-1])
    {
    /* ANSI color text - foreground */

    case 30:	_ld wvt$w_actv_rendition =
			(_ld wvt$w_actv_rendition & ~MASK_TEXT) | BLACK_TEXT
							| csa_M_NODEFAULT_TEXT;
		break;

    case 31:	_ld wvt$w_actv_rendition =
			(_ld wvt$w_actv_rendition & ~MASK_TEXT) | RED_TEXT
							| csa_M_NODEFAULT_TEXT;
		break;

    case 32:	_ld wvt$w_actv_rendition =
			(_ld wvt$w_actv_rendition & ~MASK_TEXT) | GREEN_TEXT
							| csa_M_NODEFAULT_TEXT;
		break;

    case 33:	_ld wvt$w_actv_rendition =
			(_ld wvt$w_actv_rendition & ~MASK_TEXT) | YELLOW_TEXT
							| csa_M_NODEFAULT_TEXT;
		break;

    case 34:	_ld wvt$w_actv_rendition =
			(_ld wvt$w_actv_rendition & ~MASK_TEXT) | BLUE_TEXT
							| csa_M_NODEFAULT_TEXT;
		break;

    case 35:	_ld wvt$w_actv_rendition =
			(_ld wvt$w_actv_rendition & ~MASK_TEXT) | MAGENTA_TEXT
							| csa_M_NODEFAULT_TEXT;
		break;

    case 36:	_ld wvt$w_actv_rendition =
			(_ld wvt$w_actv_rendition & ~MASK_TEXT) | CYAN_TEXT
							| csa_M_NODEFAULT_TEXT;
		break;

    case 37:	_ld wvt$w_actv_rendition =
			(_ld wvt$w_actv_rendition & ~MASK_TEXT) | WHITE_TEXT
							| csa_M_NODEFAULT_TEXT;
		break;

    case 39:	_ld wvt$w_actv_rendition &= ~csa_M_NODEFAULT_TEXT;
		break;

    /* ANSI color text - background */

    case 40: 	_ld wvt$w_actv_rendition =
			(_ld wvt$w_actv_rendition & ~MASK_TEXT_BCK) |
						     BLACK_TEXT_BCK |
						     csa_M_NODEFAULT_TEXT_BCK;
		break;

    case 41:	_ld wvt$w_actv_rendition =
			(_ld wvt$w_actv_rendition & ~MASK_TEXT_BCK) |
						     RED_TEXT_BCK   |
						     csa_M_NODEFAULT_TEXT_BCK;
		break;

    case 42:	_ld wvt$w_actv_rendition =
			(_ld wvt$w_actv_rendition & ~MASK_TEXT_BCK) |
						     GREEN_TEXT_BCK |
						     csa_M_NODEFAULT_TEXT_BCK;
		break;

    case 43:	_ld wvt$w_actv_rendition =
			(_ld wvt$w_actv_rendition & ~MASK_TEXT_BCK) |
						     YELLOW_TEXT_BCK|
						     csa_M_NODEFAULT_TEXT_BCK;
		break;

    case 44:	_ld wvt$w_actv_rendition =
			(_ld wvt$w_actv_rendition & ~MASK_TEXT_BCK) |
						     BLUE_TEXT_BCK  |
						     csa_M_NODEFAULT_TEXT_BCK;
		break;

    case 45:	_ld wvt$w_actv_rendition =
			(_ld wvt$w_actv_rendition & ~MASK_TEXT_BCK) |
						     MAGENTA_TEXT_BCK |
						     csa_M_NODEFAULT_TEXT_BCK;
		break;

    case 46:	_ld wvt$w_actv_rendition =
			(_ld wvt$w_actv_rendition & ~MASK_TEXT_BCK) |
						     CYAN_TEXT_BCK  |
						     csa_M_NODEFAULT_TEXT_BCK;
		break;

    case 47:	_ld wvt$w_actv_rendition =
			(_ld wvt$w_actv_rendition & ~MASK_TEXT_BCK) |
						     WHITE_TEXT_BCK |
						     csa_M_NODEFAULT_TEXT_BCK;
		break;

    case 49:	_ld wvt$w_actv_rendition &= ~csa_M_NODEFAULT_TEXT_BCK;
		break;

    default: break;
    }
   }
  }
 }
 return (temp);
}

/***************************************/
xsca(ld) /* Select Character Attribute */
/***************************************/

wvtp ld;

{

int n;

if (_cld wvt$b_parmcnt == 0) _cld wvt$b_parmcnt = 1;
for (n=1; n<=_cld wvt$b_parmcnt; n++)
  {
  switch (_cld wvt$l_parms[n-1])
    {
    case 1: _ld wvt$w_actv_rendition &= ~csa_M_SELECTIVE_ERASE; break;
    case 0:
    case 2: _ld wvt$w_actv_rendition |=  csa_M_SELECTIVE_ERASE; break;
    default: break;
    }
  }
}

/*******************************************/
xdecstbm(ld) /* Set Top and Bottom Margins */
/*******************************************/

wvtp ld;

{
 int temp;

 _ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;

 if (_cld wvt$l_parms[0] == 0) _cld wvt$l_parms[0] = 1;
 if (_cld wvt$l_parms[1] == 0) _cld wvt$l_parms[1] = _ld wvt$l_page_length;
 if (_cld wvt$l_parms[0] < _cld wvt$l_parms[1] &&
     _cld wvt$l_parms[1] <= _ld wvt$l_page_length)
    {
     _ld wvt$l_top_margin = _cld wvt$l_parms[0];
     _ld wvt$l_bottom_margin = _cld wvt$l_parms[1];
     _ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;
     if (!(_ld wvt$l_vt200_specific_flags & vts1_m_origin_mode))
	_ld wvt$l_actv_column = 1;
        else _ld wvt$l_actv_column = _ld wvt$l_left_margin;

     if (!(_ld wvt$l_vt200_specific_flags & vts1_m_origin_mode)) _ld wvt$l_actv_line = 1;
         else _ld wvt$l_actv_line = _ld wvt$l_top_margin;

     temp = _ld wvt$l_bottom_margin - _ld wvt$l_top_margin + 1;
     if ( _ld wvt$l_defer_limit > temp )
	 _ld wvt$l_defer_max = temp;
     else
	 _ld wvt$l_defer_max = _ld wvt$l_defer_limit;

    }
}

#if EXTENDED_PANDA
/*******************************************/
xdecslrm(ld) /* Set Left and Right Margins */
/*******************************************/

wvtp ld;

{

/*
 *   Only functions if Vertical Split Screen Scrolling Mode is TRUE
 *
 */

 _ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;

 if (_cld wvt$l_parms[0] == 0) _cld wvt$l_parms[0] = 1;
 if (_cld wvt$l_parms[1] == 0) _cld wvt$l_parms[1] = _ld wvt$l_column_width;

 if (_cld wvt$l_vt200_flags_2 & vt2_m_vss_scroll_mode)
  {
   if (_cld wvt$l_parms[0] < _cld wvt$l_parms[1] &&
       _cld wvt$l_parms[1] <= _ld wvt$l_column_width)
     {
      _ld wvt$l_left_margin = _cld wvt$l_parms[0];
      _ld wvt$l_right_margin = _cld wvt$l_parms[1];
      _ld wvt$l_vt200_specific_flags &= ~vts1_m_origin_mode;
      _ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;

      if (!(_ld wvt$l_vt200_specific_flags & vts1_m_origin_mode))
	_ld wvt$l_actv_column = 1;
      else _ld wvt$l_actv_column = _ld wvt$l_left_margin;
 
     if (!(_ld wvt$l_vt200_specific_flags & vts1_m_origin_mode))
	_ld wvt$l_actv_line = 1;
     else _ld wvt$l_actv_line = _ld wvt$l_top_margin;
     }
  }
}
#endif

/*****************************/
xtbc(ld) /* Tabulation Clear */
/*****************************/

wvtp ld;

{

int x, n;

 if (!(_cld wvt$l_vt200_common_flags & vtc1_m_feature_lock))
  {
   if (_cld wvt$b_parmcnt == 0) _cld wvt$b_parmcnt = 1;
   for (n=1; n<=_cld wvt$b_parmcnt; n++)
    {
     switch (_cld wvt$l_parms[n-1])
      {
       case 0:	_cld wvt$b_tab_stops[_ld wvt$l_actv_column] = FALSE;
		break;

       case 3:	for (x=1; x<=MAX_COLUMN; x++)
			_cld wvt$b_tab_stops[x] = FALSE;
		break;

       default: break;
      }
    }
  }
}

/**************************************/
xdecelr(ld) /* Enable Locator Reports */
/**************************************/

wvtp ld;

{

/*
 *  If the locator is enabled (local setup) then setup the reporting mode:
 *  
 *  Ps = 0	No Reports (Default)
 *  Ps = 1	Enable
 *  Ps = 1	Enable for one-shot
 *  
 *  Pu = 0	Character cells (default)
 *  Pu = 1	Pixels
 *  Pu = 2	Character cells
 *  Pu = 3	ReGIS (Not Supported Yet)
 *  
 */

 if (_cld wvt$l_vt200_common_flags & vtc1_m_enable_locator)
       {
	if (_cld wvt$l_parms[0])
	       {
		_cld wvt$l_vt200_common_flags &= ~vtc1_m_locator_cell_position;

		if (_cld wvt$l_parms[0] == 2)
			_cld wvt$l_vt200_common_flags |= vtc1_m_locator_one_shot;

		if (_cld wvt$l_parms[1] != 1)
			_cld wvt$l_vt200_common_flags |= vtc1_m_locator_cell_position;

		CHANGE_LRP_POINTER_PATTERN(ld);

		_cld wvt$l_vt200_common_flags |= vtc1_m_locator_report_mode;
	       }
	else
	       {
		if (_cld wvt$l_vt200_common_flags & vtc1_m_locator_report_mode)
			RESET_LRP_POINTER_PATTERN(ld);

		_cld wvt$l_vt200_common_flags &= ~vtc1_m_locator_report_mode;
	       }

	WVT$ENABLE_MOVEMENT_REPORT(ld, 0, 0, 0, 0, 0); /* Disable any filter */

       }

}

/**************************************/
xdecsle(ld) /* Select Locator Events  */
/**************************************/

wvtp ld;

{

register int n;

 if (_cld wvt$l_vt200_common_flags & vtc1_m_enable_locator)
       {

	if (!_cld wvt$b_parmcnt) _cld wvt$b_parmcnt = 1;

	for (n=1; n<=_cld wvt$b_parmcnt; n++)

 	 {
	  switch (_cld wvt$l_parms[n-1])
	   {

	    case 0: _cld wvt$l_vt200_common_flags &=
			~(vtc1_m_locator_down_reports |
			  vtc1_m_locator_up_reports);
		    break;

	    case 1: _cld wvt$l_vt200_common_flags |=
			vtc1_m_locator_down_reports;
		    break;

	    case 2: _cld wvt$l_vt200_common_flags &=
			~vtc1_m_locator_down_reports;
		    break;

	    case 3: _cld wvt$l_vt200_common_flags |=
			vtc1_m_locator_up_reports;
		    break;

	    case 4: _cld wvt$l_vt200_common_flags &=
			~vtc1_m_locator_up_reports;
		    break;

	    default:
		    break;
	   }
	 }
       }
}


/***************************************/
xdecefr(ld) /* Enable Filter Rectangle */
/***************************************/

wvtp ld;

{

 if (_cld wvt$l_vt200_common_flags & vtc1_m_enable_locator)
	    WVT$ENABLE_MOVEMENT_REPORT(	ld,
					1,
					_cld wvt$l_parms[0],
					_cld wvt$l_parms[1],
					_cld wvt$l_parms[2],
					_cld wvt$l_parms[3]  );

}

/***************************************/
xdecslcs(ld) /* Select Locator Cursor  */
/***************************************/

wvtp ld;

{

unsigned char old_type;

 old_type = _cld wvt$b_loc_cursor_type;
 _cld wvt$b_loc_cursor_type = _cld wvt$l_parms[0];

 if ((_cld wvt$l_vt200_common_flags & vtc1_m_enable_locator) &&
     (_cld wvt$l_vt200_common_flags & vtc1_m_locator_report_mode) &&
     (_cld wvt$b_loc_cursor_type != old_type))
		 CHANGE_LRP_POINTER_PATTERN(ld);

}

/***************************************/
xdecrcs(ld) /* Request colors status   */
/***************************************/

wvtp ld;

{
    DECtermWidget w = ld_to_w( ld );
    char buf[12];
    int reply;

    switch ( _cld wvt$l_parms[0] )	/* request type */
	{
	case 0:
	case 1:				/* number of emulated planes */
	    reply = w->common.bitPlanes;
	    break;
	case 2:				/* number of hardware planes */
	    reply = w->common.hardware_planes;
	    break;
	case 3:				/* visual class */
	    switch ( w->common.visual->class )
		{
		case StaticGray:
		    reply = 1;
		    break;
		case GrayScale:
		    reply = 2;
		    break;
		case StaticColor:
		    reply = 3;
		    break;
		case PseudoColor:
		    reply = 4;
		    break;
		case TrueColor:
		    reply = 5;
		    break;
		case DirectColor:
		    reply = 6;
		    break;
		}
	    break;
	default:
	    return;			/* unrecognized request */
	}

    sprintf( buf, _cld wvt$l_vt200_common_flags & vtc1_m_c1_transmission_mode ?
	"\233%d;%d)~" : "\033[%d;%d)~",
	(_cld wvt$l_parms[0] == 0) ? 1 : _cld wvt$l_parms[0], reply );
    WVT$TERMINAL_TRANSMIT( ld, buf );
}

/*******************************************/
xdeclfkc(ld) /* Local Function Key Control */
/*******************************************/

wvtp ld;

{
    int i = 0, fkey, function;
    int count = _cld wvt$b_parmcnt;
    DECwFunctionKeyMode mode;
    Boolean done = FALSE;

    if (count & 1L)
        count += 1;     /* If count is odd, make it next higer even number, so
                         * that we always process parameter pairs.  The parser
                         * will initialize unspecified parameters to zero.
                         */
    do
    {
        fkey  = _cld wvt$l_parms[i++];
        function = _cld wvt$l_parms[i++];

        switch (function)
        {
            case 0: mode = DECwFactoryDefault;  break;
            case 1: mode = DECwLocalFunction;   break;
            case 2: mode = DECwSendKeySequence; break;
            case 3: mode = DECwDisableKey;      break;
           default: mode = DECwFactoryDefault;  break;
        }

        if (i <= count)
        {
            switch (fkey)
            {
                case 0: _cld wvt$f1_key_mode = mode;
                        _cld wvt$f2_key_mode = mode;
                        _cld wvt$f3_key_mode = mode;
                        _cld wvt$f4_key_mode = mode;
                        _cld wvt$f5_key_mode = mode; /* F5 is a special case.
                                                        On a VT420 F5 can only
                                                        be changed through the
                                                        user interface for
                                                        security reasons. */
                        break;

                case 1: _cld wvt$f1_key_mode = mode;
                        break;

                case 2: _cld wvt$f2_key_mode = mode;
                        break;

                case 3: _cld wvt$f3_key_mode = mode;
                        break;

                case 4: _cld wvt$f4_key_mode = mode;
                        break;

               default: break;
            }
        }
        else
        {
            done = TRUE;
        }
    }
    while (!done);
}

/*
 * The routines below are used to implement the multi-page feature of our
 * video terminals (VT3XX, VT420).  Specifically, they are used to implement
 * the following escape sequences: DECCRA, NP, PP, PPA, PPB, PPR.
 */
 

/*
 * set_current_page() saves the active (displayed) page to the array that holds
 * the off-screen pages, and then copies the page new_page to the active page.
 * The cursor position is changed to (1,1) if reset_cursor is TRUE.
 */

void set_current_page(ld, new_page, reset_cursor)
wvtp ld;
int new_page;
Boolean reset_cursor;
{
    DECtermWidget w = ld_to_w(ld);

/* Ignore this sequence if we are in the status line */

    if ((_cld wvt$l_vt200_common_flags & vtc1_m_actv_status_display) != 0)
        return;

/*
 * First do some out-of-bounds checking.  Remember that pages are numbered
 * 1 through MAX_NUMBER_OF_PAGES, but they are saved in a C array, so that
 * we have to access them as 0 through MAX_NUMBER_OF_PAGES-1.
 */

    if (new_page < 0)
    {
	new_page = 0;
    }
    else if (new_page >= MAX_NUMBER_OF_PAGES)
    {
	new_page = MAX_NUMBER_OF_PAGES - 1;
    }

/* Don't do anything if the requested page is already active */

    if (new_page == _cld current_page)
        return;

/*
 * Try to allocate memory to save current page.  If this fails, just ignore
 * this request altogether and return.
 */

    if(!alloc_page(ld, _cld current_page))
        return;

/* Now save the current page and restore the new page if it's available */

    save_page(ld, _cld current_page);
    restore_page(ld, new_page);

/* Disown the selection */

    s_set_selection( w, 0, 0, CurrentTime, 0 );
    s_set_selection( w, 0, 0, CurrentTime, 1 );

/* Reset the cursor if needed (only in case of NP or PP) */

    if(reset_cursor)
    {
        _ld wvt$l_actv_line = 1;
        _ld wvt$l_actv_column = 1;
    }

/* Update the fast pointers */

    _ld wvt$a_cur_cod_ptr = _ld wvt$a_code_base[_ld wvt$l_actv_line] +
                                _ld wvt$l_actv_column;
    _ld wvt$a_cur_rnd_ptr = _ld wvt$a_rend_base[_ld wvt$l_actv_line] +
                                _ld wvt$l_actv_column;
    _ld wvt$a_cur_ext_rnd_ptr = _ld wvt$a_ext_rend_base[_ld wvt$l_actv_line] +
                                    _ld wvt$l_actv_column;

/* Finally, set the current page and repaint the display */

    _cld current_page = new_page;
    WVT$REFRESH_DISPLAY(ld, 1, _ld wvt$l_page_length);
}


/*
 * alloc_page() allocates element page_num of the array that holds the
 * off-screen pages.  It returns TRUE if it succesfully allocates it and
 * FALSE if not.  This routine must be called before save_page().
 */

alloc_page(ld, page_num)
wvtp ld;
int page_num;
{
    int page_size = _ld wvt$l_column_width * _ld wvt$l_page_length;
    int t_page = _ld wvt$l_page_length;
    int s_code_cell = sizeof(unsigned char);
    int s_rend_cell = sizeof(REND);
    int s_ext_rend_cell = sizeof(EXT_REND);
    int s_widths = sizeof(unsigned short int);
    int s_rendits = sizeof(unsigned char);

/* If correct-sized page is already allocated just return, otherwise free it */

    if(_cld page[page_num].allocated == TRUE)
        if(_cld page[page_num].page_length == _ld wvt$l_page_length &&
           _cld page[page_num].column_width == _ld wvt$l_column_width)
        {
            return TRUE;
        }
        else
        {
            free_page(ld, page_num);
        }

/* We're still here, so now allocate the memory */

    _cld page[page_num].code_cells =
        (unsigned char *) DECwTermXtCalloc(page_size, s_code_cell);

    _cld page[page_num].rend_cells =
        (REND *) DECwTermXtCalloc(page_size, s_rend_cell);

    _cld page[page_num].ext_rend_cells =
        (EXT_REND *) DECwTermXtCalloc(page_size, s_ext_rend_cell);

    _cld page[page_num].widths =
        (unsigned short int *) DECwTermXtMalloc((t_page + 1) * s_widths);

    _cld page[page_num].rendits =
        (unsigned char *) DECwTermXtMalloc((t_page + 1) * s_widths);

/* Check to see if memory allocation failed and clean up if it did */

    if(_cld page[page_num].code_cells == NULL ||
       _cld page[page_num].rend_cells == NULL ||
       _cld page[page_num].ext_rend_cells == NULL ||
       _cld page[page_num].widths == NULL||
       _cld page[page_num].rendits == NULL)
    {
        free_page(ld, page_num);
        return FALSE;
    }
    else
    {
        _cld page[page_num].allocated = TRUE;
        _cld page[page_num].page_length = _ld wvt$l_page_length;
        _cld page[page_num].column_width = _ld wvt$l_column_width;
        return TRUE;
    }
}


/*
 * free_page() frees element page_num of the array that holds the
 * off-screen pages.
 */

free_page(ld, page_num)
wvtp ld;
int page_num;
{
    _cld page[page_num].allocated = FALSE;

    if(_cld page[page_num].code_cells != NULL)
    {
        XtFree((char *) _cld page[page_num].code_cells);
        _cld page[page_num].code_cells = NULL;
    }

    if(_cld page[page_num].rend_cells !=NULL)
    {
        XtFree((char *) _cld page[page_num].rend_cells);
        _cld page[page_num].rend_cells = NULL;
    }

    if(_cld page[page_num].ext_rend_cells !=NULL)
    {
        XtFree((char *) _cld page[page_num].ext_rend_cells);
        _cld page[page_num].ext_rend_cells = NULL;
    }

    if(_cld page[page_num].widths !=NULL)
    {
        XtFree((char *) _cld page[page_num].widths);
        _cld page[page_num].widths = NULL;
    }

    if(_cld page[page_num].rendits !=NULL)
    {
        XtFree((char *) _cld page[page_num].rendits);
        _cld page[page_num].rendits = NULL;
    }
}


/*
 * save_page() copies the active (currently displayed) page to element
 * page_num of the off-screen memory array.  Note that you must call
 * alloc_page() before calling this routine.
 */

save_page(ld, page_num)
wvtp ld;
int page_num;
{
    int n, offset;
    unsigned char *codeptr;
    REND *rendptr;
    EXT_REND *ext_rendptr;
    int s_code_cell = sizeof(unsigned char);
    int s_rend_cell = sizeof(REND);
    int s_ext_rend_cell = sizeof(EXT_REND);
    int t_page = _ld wvt$l_page_length;
    int t_width = _ld wvt$l_column_width;

    for (n = 1, offset = 0; n <= t_page; n++, offset += t_width)
    {
        codeptr = _cld page[page_num].code_cells + offset;
        rendptr = _cld page[page_num].rend_cells + offset;
        ext_rendptr = _cld page[page_num].ext_rend_cells + offset;

        memcpy(codeptr, &_ld wvt$a_code_base[n][1], t_width * s_code_cell);
        memcpy(rendptr, &_ld wvt$a_rend_base[n][1], t_width * s_rend_cell);
        memcpy(ext_rendptr, &_ld wvt$a_ext_rend_base[n][1], t_width * s_ext_rend_cell);

        _cld page[page_num].widths[n] = _ld wvt$w_widths[n];
        _cld page[page_num].rendits[n] = _ld wvt$b_rendits[n];
    }
}


/*
 * restore_page() copies data from saved page page_num to the active page.  If
 * the saved page is bigger than the current page, only the number of lines in
 * the current page are restored.  If the saved page is smaller then the current
 * page, then the saved page is restored and any remaining lines are cleared.
 */

#define min(x,y) (((x)<(y))?(x):(y))

restore_page(ld, page_num)
wvtp ld;
int page_num;
{
    int n, offset;
    unsigned char *codeptr;
    REND *rendptr;
    EXT_REND *ext_rendptr;
    int s_code_cell = sizeof(unsigned char);
    int s_rend_cell = sizeof(REND);
    int s_ext_rend_cell = sizeof(EXT_REND);
    int t_page = _ld wvt$l_page_length;
    int t_width = _ld wvt$l_column_width;
    int lines_to_restore = min(t_page, _cld page[page_num].page_length);

/* Restore page if it has been saved and the number of columns hasn't changed */

    if (_cld page[page_num].allocated &&
        _cld page[page_num].column_width == t_width)
    {
        for (n = 1, offset = 0; n <= lines_to_restore; n++, offset += t_width)
        {
            codeptr = _cld page[page_num].code_cells + offset;
            rendptr = _cld page[page_num].rend_cells + offset;
            ext_rendptr = _cld page[page_num].ext_rend_cells + offset;

            memcpy(&_ld wvt$a_code_base[n][1], codeptr, t_width * s_code_cell);
            memcpy(&_ld wvt$a_rend_base[n][1], rendptr, t_width * s_rend_cell);
            memcpy(&_ld wvt$a_ext_rend_base[n][1], ext_rendptr, t_width * s_ext_rend_cell);

            _ld wvt$w_widths[n] = _cld page[page_num].widths[n];
            _ld wvt$b_rendits[n] = _cld page[page_num].rendits[n];
        }

        if (t_page > _cld page[page_num].page_length)
        {
            /* Using ending values of n and offset from loop above */
            for ( ; n <= t_page; n++, offset += t_width)
            {
                memset(&(_ld wvt$a_code_base[n][1]), 0, t_width * s_code_cell);
                memset(&(_ld wvt$a_rend_base[n][1]), 0, t_width * s_rend_cell);
                memset(&(_ld wvt$a_ext_rend_base[n][1]), 0, t_width * s_ext_rend_cell);
                _ld wvt$w_widths[n] = t_width;
                _ld wvt$b_rendits[n] = 0;
            }
        }
    }
    else    /* Clear the display */
    {
        for (n = 1; n <= t_page; n++)
        {
            memset(&(_ld wvt$a_code_base[n][1]), 0, t_width * s_code_cell);
            memset(&(_ld wvt$a_rend_base[n][1]), 0, t_width * s_rend_cell);
            memset(&(_ld wvt$a_ext_rend_base[n][1]), 0, t_width * s_ext_rend_cell);
            _ld wvt$w_widths[n] = t_width;
            _ld wvt$b_rendits[n] = 0;
        }
    }
}


/**********************************/
xcra(ld) /* Copy rectangular area */
/**********************************/

wvtp ld;

{
/* This routine only implements the special case of the real DECCRA sequence
 * that is needed to satisfy the customer (GE).  Full support will have to be
 * added at a later date.  The special case that is handled by this routine is
 * when the entire page is copied (not an arbitrary rectangular area), and
 * either the source or the destination is the active page (i.e., you can not
 * copy from one off-screen page to another).
 */
    if (_cld wvt$b_parmcnt == 8 &&                          /* 8 parameters */
        _cld wvt$l_parms[0] == 1 &&                         /* From row 1 */
        _cld wvt$l_parms[1] == 1 &&                         /* From column 1 */
        _cld wvt$l_parms[2] == _ld wvt$l_page_length &&     /* Full length */
        _cld wvt$l_parms[3] == _ld wvt$l_column_width &&    /* Full width */
        _cld wvt$l_parms[5] == 1 &&                         /* To row 1 */
        _cld wvt$l_parms[6] == 1)                           /* To column 1 */
    {
        /* Make sure that valid page numbers were specified */
        if (_cld wvt$l_parms[4] == 0) _cld wvt$l_parms[4] = 1;
        if (_cld wvt$l_parms[4] > MAX_NUMBER_OF_PAGES)
            _cld wvt$l_parms[4] = MAX_NUMBER_OF_PAGES;

        if (_cld wvt$l_parms[7] == 0) _cld wvt$l_parms[7] = 1;
        if (_cld wvt$l_parms[7] > MAX_NUMBER_OF_PAGES)
            _cld wvt$l_parms[7] = MAX_NUMBER_OF_PAGES;

        /* Make sure we are copying either to or from the active page */
        if ((_cld wvt$l_parms[4] != _cld current_page + 1) &&
            (_cld wvt$l_parms[7] != _cld current_page + 1))
            return;

        /* Make sure the current page is allocated so we don't clear it */
        /* later in the call to restore_page */
        if (!alloc_page(ld, _cld current_page))
            return;

        /* Check to see if we are copying from or to the current page */
        if (_cld wvt$l_parms[4] == _cld current_page + 1)
        {
            int page_num = _cld wvt$l_parms[7] - 1;
            
            if (alloc_page(ld, page_num))
                save_page(ld, page_num);
        }   
        else
        {
            restore_page(ld, _cld wvt$l_parms[4] - 1);
            WVT$REFRESH_DISPLAY(ld, 1, _ld wvt$l_page_length);
        }
   }       
}

/*********************/
xnp(ld) /* Next page */
/*********************/

wvtp ld;

{
    if (_cld wvt$l_parms[0] == 0) _cld wvt$l_parms[0] = 1;
    set_current_page(ld, _cld current_page + _cld wvt$l_parms[0], TRUE);
}

/*************************/
xpp(ld) /* Previous page */
/*************************/

wvtp ld;

{
    if (_cld wvt$l_parms[0] == 0) _cld wvt$l_parms[0] = 1;
    set_current_page(ld, _cld current_page - _cld wvt$l_parms[0], TRUE);
}

/***********************************/
xppa(ld) /* Page Position Absolute */
/***********************************/

wvtp ld;

{
    if (_cld wvt$l_parms[0] == 0) _cld wvt$l_parms[0] = 1;
    set_current_page(ld, _cld wvt$l_parms[0] - 1, FALSE);
}

/***********************************/
xppr(ld) /* Page Position Relative */
/***********************************/

wvtp ld;

{
    if (_cld wvt$l_parms[0] == 0) _cld wvt$l_parms[0] = 1;
    set_current_page(ld, _cld current_page + _cld wvt$l_parms[0], FALSE);
}

/***********************************/
xppb(ld) /* Page Position Backward */
/***********************************/

wvtp ld;

{
    if (_cld wvt$l_parms[0] == 0) _cld wvt$l_parms[0] = 1;
    set_current_page(ld, _cld current_page - _cld wvt$l_parms[0], FALSE);
}
