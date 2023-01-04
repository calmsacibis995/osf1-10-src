/* #module WV_TERMMGR "X3.0-5" */
/*
 *  Title:	WV_TERMMGR
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
 *  This module contains the routine to perform terminal management functions.
 *
 *  Routines included in this module:
 *
 *	xris      - reset all state to initial values
 *	xda       - dispatch device attribute requests
 *	xscl      - select conformance level
 *	xstr      - soft terminal reset
 *
 *  Author:	Frederick G. Kleinsorge
 *		Low-End Workstation Graphics Engineering
 *
 * Revision History:
 *
 * Alfred von Campe     30-Sep-1993     BL-E
 *      - Add multi-page support.
 *
 *  Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 * Alfred von Campe     25-Mar-1993     V1.2/BL2
 *      - Add F1-F5 key support
 *
 * Aston Chan		12-Mar-1993	V1.2/BL2
 *	- Add Turkish/Greek support.
 *
 * Aston Chan		26-Jan-1993	V1.2/Bl1
 *	- CLD MUH02337.  Send "\233>28;" in secda instead of "\233>55;". 
 *	  That's a bug in I18n code.
 *
 * Eric Osman		11-June-1992	Sun
 *	- Casting to make compiler happy
 *
 * Eric Osman		23-Apr-1992	Alpha
 *	- increase size of report buffer so the strcat's don't go off the
 *	  end which crashes DECterm on Alpha systems
 *
 * Aston Chan		17-Dec-1991	V3.1
 *	- I18n code merge
 *
 * Bob Messenger	13-Sep-1990	X3.0-7
 *	- In the primary DA response, don't say we support color text unless
 *	  we're on a color monitor.
 *
 * Bob Messenger        17-Jul-1990     X3.0-5
 *      Merge in Toshi Tanimoto's changes to support Asian terminals -
 *	- initial character sets mapping
 *	- primary device attributes response
 *
 * Bob Messenger	21-Jun-1990		X3.0-5
 *	- Add printer port support.
 *		- set pf1_m_prt_enabled in wvt$w_print_flags
 *		- include parameter 2 in the primary DA response
 *
 * Bob Messenger	13-Apr-1990		X3.0-2
 *	- Enable answerback in xris.
 *
 * Bob Messenger	12-Jul-1989		X2.0-16
 *	- Fix VT340 ID (should be same as VT330 ID).
 *
 * Bob Messenger	14-Jun-1989		X2.0-14
 *	- Don't clobber wvt$b_nrc_set during reset by setting it to
 *	  wvt$b_national_keyboard (which is always 0).
 *
 * Bob Messenger	26-May-1989		X2.0-12
 *	- Back out the answerback change, since it wasn't approved in
 *	  DECWINDOWS_V2_FT1.
 *
 * Bob Messenger	14-May-1989		X2.0-10
 *	- Enable answerback.
 *
 * Bob Messenger	 4-Apr-1989		X2.0-5
 *	- Enable ANSI color text on color systems.
 *
 * Bob Messenger	 2-Apr-1989		X2.0-5
 *	- Moved wvt$l_column_width to specific area
 *
 * Bob Messenger	22-Mar-1989		X2.0-3
 *	- Support VT330 ID
 *
 * Bob Messenger	24-Jan-1989		X1.1-1
 *	- RIS should initialize locator key definitions
 *
 * Bob Messenger	16-Jan-1989		X1.1-1
 *	- moved many ld fields to common area
 *
 * Bob Messenger	11-Jan-1988		X1.1-1
 *	- move wvt$b_cursts to common area
 *
 * Bob Messenger	10-Dec-1988		X1.1-1
 *	- RIS should turn off new line mode
 *
 * Tom Porcher		20-Oct-1988		X0.5-4
 *	- Fix DECSTR to set current renditions to "erasable".
 *
 * Tom Porcher		16-Aug-1988		X0.4-43
 *	- DECSTR no longer resets Auto Wrap.  Let's see who complains.
 *	  (This was done because Ultrix doesn't handle auto wrap, so people
 *	  rely on the terminal doing it.  The spec and eventually the VSRM
 *	  should be changed accordingly.)
 *
 * Mike Leibow		16-Aug-1988
 *	- Added WVT$UNLOCK_KEYBOARD to xstr().
 *
 * Mike Leibow          07-Jun-1988
 *      - Prepared code for status line.  Some ld fields are now accessed
 *        by _cld instead of _ld.
 *
 *              Peter Sichel            3-May-1988
 *      - Fix DECterm DA to correspond with what we actually implemented
 *
 *		Tom Porcher		20-Jan-1988
 *	- Changed all WVT$ routines to uppercase.
 *
 *		Tom Porcher		29-Dec-1987
 *	- Added callbacks for changing keypad mode.
 *
 * FGK0027	Frederick G. Kleinsorge	29-Jul-1987
 *  
 * o Reset the FF/LF counter in RIS and DECSTR
 *
 * FGK0026	Frederick G. Kleinsorge	23-Jul-1987
 *  
 * o Add scroll deferral computation
 *
 * FGK0025	Frederick G. Kleinsorge	21-Jul-1987
 *  
 * o Add Rectangular area editing DA parameter #28
 *
 * FGK0024	Frederick G. Kleinsorge	21-Apr-1987
 *  
 * o Mass edit symbols
 *
 * FGK0023	Frederick G. Kleinsorge	16-Apr-1987
 *  
 * o Change the data type of ld
 *
 * FGK0022	Frederick G. Kleinsorge	08-Apr-1987
 *  
 * o Add feature check for ISO latin-1 because of compose table bug in
 *   the driver (UIS).
 *
 * FGK0021	Frederick G. Kleinsorge	17-Mar-1987
 *  
 * o Fix the check for the secondary ID.  It should reply as
 *   a Native VWS terminal unless VT200 is set in the alternate ID
 *   the code *was* not sending anything if the alternate ID was not
 *   0 or VT200.
 *
 * FGK0020	Frederick G. Kleinsorge	13-Mar-1987
 *  
 * o Change the ANSI color extension check in the DA
 *   to use the vt2_m_ansi_color flag instead of counting
 *   the planes (flag is set at terminal creation).
 *
 * FGK0019	Frederick G. Kleinsorge	06-Mar-1987
 *  
 * o Allow native ID to be overridden by VT200_ID
 *
 * FGK0018	Frederick G. Kleinsorge	05-Mar-1987
 *  
 * o V3.2
 *
 * FGK0017	Frederick G. Kleinsorge	13-Feb-1987
 *  
 * o Change secondary DA to use the VWS Emulator ID
 *
 * FGK0016	Frederick G. Kleinsorge	04-Feb-1987
 *  
 * o Set the compose tables in RIS and DECSTR
 *   based on user_preference
 *
 * FGK0015	Frederick G. Kleinsorge	03-Feb-1987
 *  
 * o Reset pointer pattern in RIS if in LRP
 *
 * FGK0014	Frederick G. Kleinsorge	12-Jan-1987
 *  
 * o Move pars_init routine to wv_parser.c
 *
 * FGK0013	Frederick G. Kleinsorge	01-Oct-1986
 *  
 * o Add alternate terminal IDs for VT100 mode.
 *
 * FGK0012	Frederick G. Kleinsorge	29-Sep-1986
 *  
 * o Add secondary DA for VT240 when ReGIS is loaded.
 *
 * FGK0011	Frederick G. Kleinsorge	9-Sep-1986
 *  
 * o Get rid of PANDA stuff
 *
 * FGK0010	Frederick G. Kleinsorge	14-Aug-1986
 *  
 * o Indicate ISO 8-bit support (keyboard not done yet)
 *
 * FGK0009	Frederick G. Kleinsorge	22-Jul-1986
 *  
 * o Update version to X04-017
 *
 * FGK0008	Frederick G. Kleinsorge	17-Jul-1986
 *
 * o Add ReGIS DA parameter (;3) if bit is set in wvt$l_vt200_flags_2
 *
 * FGK0007	Frederick G. Kleinsorge	14-Jul-1986
 *
 * o Remove the reset of the color map from xris() -- do the
 *   reset from above this routine - avoids double-setting on
 *   creation.
 *
 * FGK0006	Frederick G. Kleinsorge	14-Jul-1986
 *
 * o Add DA parameter ";22" for ANSI Color Text extension
 *
 * FGK0005	Frederick G. Kleinsorge	26-Jun-1986
 *
 * o Move color map init above the erase_screen -- it makes a
 *   difference on a QVSS (NONRETRO device-type),
 *
 * FGK0004	Frederick G. Kleinsorge	24-Jun-1986
 *
 * o Add color map initialization, reset vt1_m_screen_mode on RIS
 *
 * FGK0003	Frederick G. Kleinsorge	10-Jun-1986
 *
 * o Added initialization of RIGHT and LEFT margins
 *
 * o Added LEVEL3 checks
 *
 * FGK0002	Frederick G. Kleinsorge	06-Jun-1986
 *
 * o Added initialization of locator button reports
 *
 * o Removed DRCS from DA string (not implemented yet)
 *
 * FGK0001	Frederick G. Kleinsorge	31-JAN-1986
 *
 * o Reset line renditions in RIS code.
 *
 */

#include "wv_hdr.h"
#define _state _cld com_state.

#define SET_SUPPLEMENTAL(a)					\
	((a) & vte1_m_greek	? GREEK_SUPPLEMENTAL	:	\
	((a) & vte1_m_turkish	? TURKISH_SUPPLEMENTAL	:	\
	((a) & vte1_m_hebrew	? HEB_SUPPLEMENTAL	:	\
				  SUPPLEMENTAL)))

#define IsDECSupplemental(s) ((s) == SUPPLEMENTAL ||		\
			      (s) == GREEK_SUPPLEMENTAL ||	\
			      (s) == TURKISH_SUPPLEMENTAL ||	\
			      (s) == HEB_SUPPLEMENTAL)

/* globalvalue UCB$W_VS_STATE, UCB$M_VS_TCS_PRESENT,
            UCB$L_VS_PLANE_COUNT, UCB$M_VS_APL_PRESENT;
*/

/***********************************/
xris(ld) /* Reset to Initial State */
/***********************************/

wvtp ld;

/*
 *  RESET to initial state:
 *
 *	o Clears UDKs
 *	o Clears screen
 *	o Sets cursor to upper left (1,1)
 *	o Sets SGR to normal
 *	o Sets selective erase to "not erasable"
 *	o Sets all characters to spaces
 *	o Recalls all setup default settings
 *	o Sets last parser state to R_GRAPHIC
 *	o Resets designated fonts
 *	o Resets LK201 attributes
 *	o Resets line rendition attributes to SINGLE WIDTH
 *	o Resets UIS scrolling setting to setup
 *	o Resets the Top and Bottom margins
 *	o Resets the Right and Left margins
 *	o Resets DECVSSM
 *      o Clears saved pages
 *
 */

{

register int x,y;
int i, temp;

_cld wvt$b_conforming_resize = 1;
_cld wvt$b_conformance_level = LEVEL3;
_cld wvt$l_vt200_common_flags = vtc1_m_enable_locator | vtc1_m_auto_answerback;

_cld wvt$l_flags = vt4_m_bell;

_cld wvt$w_print_flags = pf1_m_prt_enabled | pf1_m_prt_transmission_mode;
if ( ld->common.printMode == DECwAutoPrintMode )
	_cld wvt$w_print_flags |= pf1_m_auto_print_mode;
else if ( ld->common.printMode == DECwPrintControllerMode )
	_cld wvt$w_print_flags |= pf1_m_prt_controller_mode;
if ( ld->common.printFormFeedMode )
	_cld wvt$w_print_flags |= pf1_m_prt_ff_mode;
if ( ld->common.printExtent != DECwScrollRegionOnly )
	_cld wvt$w_print_flags |= pf1_m_prt_extent_mode;

if (_cld wvt$l_vt200_flags & vt1_m_scroll_mode) WVT$SMOOTH_SCROLL(ld);
	else WVT$JUMP_SCROLL(ld);

if (_cld wvt$l_vt200_flags & vt1_m_display_controls_mode) WVT$RECALL_FONT(ld);

_cld wvt$l_vt200_flags   &= ~(	vt1_m_vt52_cursor_seq|
				vt1_m_display_controls_mode|
				vt1_m_new_line_mode);

_cld wvt$l_vt200_flags |= vt1_m_ansi_mode | vt1_m_cursor_blink_mode;

_cld wvt$l_vt200_flags_2 &= ~(	vt2_m_vss_scroll_mode );
_cld wvt$l_vt200_flags_2 |= vt2_m_delete_disabled |
				vt2_m_resize_disabled |
				vt2_m_shrink_disabled |
				vt2_m_addopt_disabled |
				vt2_m_enable_ISO_latin;

#if DEVICE_TYPE == DECTERM_DEVICE

if ( ld->common.visual->class == StaticColor
		|| ld->common.visual->class == PseudoColor
		|| ld->common.visual->class == TrueColor
		|| ld->common.visual->class == DirectColor )
	_cld wvt$l_vt200_flags_2 |= vt2_m_ansi_color;
else
	_cld wvt$l_vt200_flags_2 &= ~vt2_m_ansi_color;

#endif

_cld wvt$b_last_event = R_GRAPHIC;

WVT$MAIN_DISPLAY(ld);
WVT$GET_TERMINAL_SETUP(ld);		   /* Get setup (NVR) information */
xdisplay_ris(ld);
WVT$STATUS_DISPLAY(ld);
xdisplay_ris(ld);
WVT$MAIN_DISPLAY(ld);

WVT$SET_KB_ATTRIBUTES(ld);		   /* Set LK201 attributes */

if (_cld wvt$l_vt200_common_flags & vtc1_m_locator_report_mode)
	RESET_LRP_POINTER_PATTERN(ld);

for	(y = 0; y<MAX_UDK_VALUE; y++)	   /* Clear UDK's */
	 _cld wvt$w_udk_length[y] = 0;
    
_cld wvt$w_udk_space_used= 0;

_cld wvt$l_vt200_common_flags &= ~(vtc1_m_insert_mode |
				   vtc1_m_kbd_action_mode |
				vtc1_m_udk_lock_control |
				vtc1_m_locator_report_mode|
				vtc1_m_locator_one_shot);

if (!_cld wvt$b_user_preference_set)
    _cld wvt$b_user_preference_set = SET_SUPPLEMENTAL(_cld wvt$l_ext_flags);

if (_cld wvt$l_vt200_flags_2 & vt2_m_enable_ISO_latin)
 {
  if (IsDECSupplemental(_cld wvt$b_user_preference_set))
      WVT$RESET_KB_COMPOSE(ld);
  else
      WVT$SET_ISO_KB_COMPOSE(ld);
 }

_cld wvt$f1_key_mode = DECwFactoryDefault;
_cld wvt$f2_key_mode = DECwFactoryDefault;
_cld wvt$f3_key_mode = DECwFactoryDefault;
_cld wvt$f4_key_mode = DECwFactoryDefault;
_cld wvt$f5_key_mode = DECwFactoryDefault;

/* Release any ReGIS resources */
if (_mld wvt$l_vt200_specific_flags & vts1_m_regis_available) WVT$CLEAR_REGIS(ld);

/* clear in_dcs after clearing ReGIS, so ReGIS will know when it's being
 * aborted
 */

_cld wvt$b_in_dcs	= 0;

for (i = 0; i < MAX_NUMBER_OF_PAGES; i++)
    if (_cld page[i].allocated)
        free_page(ld, i);

/* initialize locator key definitions */
WVT$CLEAR_LKD( ld );

}

xdisplay_ris(ld)
wvtp ld;
{      
	register int x, y;
	int temp;

WVT$ERASE_DISPLAY(	ld,
			1,
			1,
			_ld wvt$l_page_length,
			_ld wvt$l_column_width); /* Clear the screen */

for	(y = 1; y <= _ld wvt$l_page_length; y++) /* Clear the line renditions */
	{
	 line_width(y) = _ld wvt$l_column_width;
	 line_rendition(y) = SINGLE_WIDTH;
	}

_ld wvt$l_vt200_specific_flags &= ~( vts1_m_origin_mode|
				     vts1_m_last_column);

if (_cld wvt$l_vt200_common_flags & vtc1_m_actv_status_display )
    _ld wvt$l_vt200_specific_flags &= ~vts1_m_regis_available;
else
    _ld wvt$l_vt200_specific_flags |= vts1_m_regis_available;

/* Clear User Preferred Suplemental flag, TN400, EIC_JPN	*/
_ld wvt$b_save_ups = _ld wvt$b_ups = 0;
if ( _cld wvt$l_ext_flags & vte1_m_tomcat ) {

_ld wvt$b_gl = _ld wvt$b_sav_gl = 0;
_ld wvt$b_gr = _ld wvt$b_sav_gr = 2;

if ( _cld wvt$l_ext_specific_flags & vte2_m_jisroman_mode ) {
  _ld wvt$b_save_g_sets[0]  = _ld wvt$b_g_sets[0] = JIS_ROMAN;
  _ld wvt$b_save_ext_g_sets[0]  = _ld wvt$b_ext_g_sets[0] = ONE_BYTE_SET;
} else {
  _ld wvt$b_save_g_sets[0]  = _ld wvt$b_g_sets[0] = ASCII;
  _ld wvt$b_save_ext_g_sets[0]  = _ld wvt$b_ext_g_sets[0] = STANDARD_SET;
}
_ld wvt$b_save_g_sets[1]  = _ld wvt$b_g_sets[1] = JIS_KATAKANA;
_ld wvt$b_save_ext_g_sets[1]  = _ld wvt$b_ext_g_sets[1] = ONE_BYTE_SET;
_ld wvt$b_save_g_sets[2]  = _ld wvt$b_g_sets[2] = JIS_KATAKANA;
_ld wvt$b_save_ext_g_sets[2]  = _ld wvt$b_ext_g_sets[2] = ONE_BYTE_SET;
_ld wvt$b_save_g_sets[3]  = _ld wvt$b_g_sets[3] = LINE_DRAWING;
_ld wvt$b_save_ext_g_sets[3]  = _ld wvt$b_ext_g_sets[3] = STANDARD_SET;

if ( _cld wvt$l_ext_specific_flags & vte2_m_kanji_mode ) {
  _ld wvt$b_gr = _ld wvt$b_sav_gr = 3;
  _ld wvt$b_save_g_sets[1]  = _ld wvt$b_g_sets[1] = LINE_DRAWING;
  _ld wvt$b_save_ext_g_sets[1]  = _ld wvt$b_ext_g_sets[1] = STANDARD_SET;
  _ld wvt$b_save_g_sets[3]  = _ld wvt$b_g_sets[3] = DEC_KANJI;
  _ld wvt$b_save_ext_g_sets[3]  = _ld wvt$b_ext_g_sets[3] = TWO_BYTE_SET;
}

if (!_cld wvt$b_user_preference_set)
     _cld wvt$b_user_preference_set = SET_SUPPLEMENTAL(_cld wvt$l_ext_flags);

_cld wvt$b_char_stack_top = 0;
_cld wvt$l_ext_specific_flags &= ~vte2_m_sixel_scroll_mode;

} else if ( _cld wvt$l_ext_flags & vte1_m_asian_common ) {
    _ld wvt$b_gl = _ld wvt$b_sav_gl = 0;
    _ld wvt$b_gr = _ld wvt$b_sav_gr = 3;
    _ld wvt$b_save_g_sets[0] = _ld wvt$b_g_sets[0] = ASCII;
    _ld wvt$b_save_g_sets[1] = _ld wvt$b_g_sets[1] = LINE_DRAWING;
    _ld wvt$b_save_ext_g_sets[0] = _ld wvt$b_ext_g_sets[0] =
    _ld wvt$b_save_ext_g_sets[1] = _ld wvt$b_ext_g_sets[1] = STANDARD_SET;
    if ( _cld wvt$l_ext_flags & vte1_m_bobcat ) {
	_ld wvt$b_save_g_sets[2] = _ld wvt$b_g_sets[2] = ASCII;
	_ld wvt$b_save_g_sets[3] = _ld wvt$b_g_sets[3] = DEC_HANZI;
	_ld wvt$b_save_ext_g_sets[2] = _ld wvt$b_ext_g_sets[2] = STANDARD_SET;
	_ld wvt$b_save_ext_g_sets[3] = _ld wvt$b_ext_g_sets[3] = TWO_BYTE_SET;
    } else if ( _cld wvt$l_ext_flags & vte1_m_dickcat ) {
	if ( _cld wvt$l_ext_specific_flags & vte2_m_ksroman_mode ) {
	    _ld wvt$b_save_g_sets[0] = _ld wvt$b_g_sets[0] = KS_ROMAN;
	    _ld wvt$b_save_ext_g_sets[0] = _ld wvt$b_ext_g_sets[0] = ONE_BYTE_SET;
	}
	_ld wvt$b_save_g_sets[2] = _ld wvt$b_g_sets[2] = DEC_HANGUL;
	_ld wvt$b_save_g_sets[3] = _ld wvt$b_g_sets[3] = DEC_HANGUL;
	_ld wvt$b_save_ext_g_sets[2] = _ld wvt$b_ext_g_sets[2] = TWO_BYTE_SET;
	_ld wvt$b_save_ext_g_sets[3] = _ld wvt$b_ext_g_sets[3] = TWO_BYTE_SET;
    } else if ( _cld wvt$l_ext_flags & vte1_m_fishcat ) {
	_ld wvt$b_save_g_sets[2] = _ld wvt$b_g_sets[2] = ASCII;
	_ld wvt$b_save_g_sets[3] = _ld wvt$b_g_sets[3] = DEC_HANYU;
	_ld wvt$b_save_ext_g_sets[2] = _ld wvt$b_ext_g_sets[2] = STANDARD_SET;
	_ld wvt$b_save_ext_g_sets[3] = _ld wvt$b_ext_g_sets[3] = FOUR_BYTE_SET;
    }
    if (!_cld wvt$b_user_preference_set)
	_cld wvt$b_user_preference_set = SET_SUPPLEMENTAL(_cld wvt$l_ext_flags);

    _cld wvt$b_char_stack_top = 0;
    _cld wvt$l_ext_specific_flags &= ~vte2_m_sixel_scroll_mode;
	
} else {

_ld wvt$b_save_ext_g_sets[0]  = _ld wvt$b_ext_g_sets[0] = 
_ld wvt$b_save_ext_g_sets[1]  = _ld wvt$b_ext_g_sets[1] = 
_ld wvt$b_save_ext_g_sets[2]  = _ld wvt$b_ext_g_sets[2] = 
_ld wvt$b_save_ext_g_sets[3]  = _ld wvt$b_ext_g_sets[3] = STANDARD_SET;


_ld wvt$b_gl = _ld wvt$b_sav_gl = 0;                     
_ld wvt$b_gr = _ld wvt$b_sav_gr = 2;

_ld wvt$b_save_g_sets[0]  = _ld wvt$b_g_sets[0] =
_ld wvt$b_save_g_sets[1]  = _ld wvt$b_g_sets[1] = ASCII;

_ld wvt$b_gl = _ld wvt$b_sav_gl = 0;
_ld wvt$b_gr = _ld wvt$b_sav_gr = 2;

_ld wvt$b_save_g_sets[0]  = _ld wvt$b_g_sets[0] =
_ld wvt$b_save_g_sets[1]  = _ld wvt$b_g_sets[1] = ASCII;

if (!_cld wvt$b_user_preference_set)
     _cld wvt$b_user_preference_set = SET_SUPPLEMENTAL(_cld wvt$l_ext_flags);
 
if (_cld wvt$b_conformance_level >= LEVEL2)
 {
 _ld wvt$b_save_g_sets[2]   = _ld wvt$b_save_g_sets[3] =
 _ld wvt$b_g_sets[2]        = _ld wvt$b_g_sets[3]      = 
 _cld wvt$b_user_preference_set;
    if ( _cld wvt$b_user_preference_set == ISO_LATIN_8 ||
	 _cld wvt$b_user_preference_set == HEB_SUPPLEMENTAL )
	_ld wvt$b_save_ext_g_sets[2] = _ld wvt$b_ext_g_sets[2] =
	_ld wvt$b_save_ext_g_sets[3] = _ld wvt$b_ext_g_sets[3] = ONE_BYTE_SET;
 }
else
 {
  _ld wvt$b_save_g_sets[2]  = _ld wvt$b_g_sets[2] =
  _ld wvt$b_save_g_sets[3]  = _ld wvt$b_g_sets[3] =
  ASCII;
 }
}
 
_ld wvt$b_disp_eol	= 0;
_ld wvt$b_single_shift	= 0;
_ld wvt$a_cur_cod_ptr   = 0;
_ld wvt$w_save_rendition= 0;
_ld wvt$w_save_ext_rendition= 0;

_ld wvt$w_actv_rendition = csa_M_SELECTIVE_ERASE;
_ld wvt$w_actv_ext_rendition = 0;

_ld wvt$l_actv_line	= _ld wvt$l_actv_column = _ld wvt$l_disp_pos
 			= _ld wvt$l_top_margin  = _ld wvt$l_save_line
			= _ld wvt$l_save_column = _ld wvt$l_left_margin
			= 1;

_ld wvt$l_bottom_margin = _ld wvt$l_page_length;
_ld wvt$l_right_margin  = _ld wvt$l_column_width;

temp = _ld wvt$l_bottom_margin - _ld wvt$l_top_margin + 1;
if ( _ld wvt$l_defer_limit > temp )
    _ld wvt$l_defer_max = temp;
else
    _ld wvt$l_defer_max = _ld wvt$l_defer_limit;
}
             

/************************************/
xda(ld) /* Device Attribute Reports */
/************************************/

wvtp ld;

{

 unsigned short *temp;
 char secda[30];
#if DEVICE_TYPE == DECTERM_DEVICE				/*RDM*/
 char *dot;							/*RDM*/
 int version_len;						/*RDM*/
 globalref char decterm_version[];	     			/*RDM*/
#endif								/*RDM*/

 if (_cld wvt$l_parms[0] == 0)
   {
    switch (_cld wvt$b_privparm)
	{
	 case 0:				/* Send Primary DA */

		xda1rpt(ld);
		break;

	 case 2:				/* Secondary DA string */

		/* There is no secondary DA string on a level 1 device */
 		if (_cld wvt$b_conformance_level < LEVEL2) break;

#if DEVICE_TYPE == VWS_DEVICE				/*RDM*/

		/* In level 2 we will return either the Native ID or if
		   the alternate ID is set to VT200 we will return the
		   VT200 device ID for backwards compatability with the
		   VT220/240.  Level 3 ID's are not currently in the code */

		if (_cld wvt$b_alt_term_id != VT200_ID)
		    {
			/* Native ID */
				     strcpy(secda,"\233>29;32;");
		    /*                               ^  ^  ^
		     *                               |  |  |
		     *                               |  |  |
		     *                               |  |  |
		     *                               |  |  |  
		     *                               |  |  +-- Software Revision.  Use the VWS release number
		     *                               |  +-- The VWS Terminal Emulator is type "29"
		     *                               +-- CSI (will be translated to 7-bit if needed)
		     */

		        /* Number of planes */
		    
/*MOOF*/	   /*     temp = _cld wvt$a_ucbadr + UCB$L_VS_PLANE_COUNT; */
		   /*     xmit_value(ld, *temp); */
		    
				   strcat(secda, ";0c");
		    /*                             ^^
		     *                             ||
		     *                             |+-- Final character is "c"
		     *                             +- No more options.
		     */
		    }
		else
		    {
		
		    /*
		     *  This is the OLD secondary DA report for the VT220/240
		     *  If ReGIS is available we will respond as a VT240 if
		     *  not we will respond as a VT220.
		     */

			if (_ld wvt$l_vt200_specific_flags & vts1_m_regis_available)
				strcpy(secda, "\233>2;21;0c");
			else strcpy(secda, "\233>1;10;0c");
		     }
#endif

#if DEVICE_TYPE == DECTERM_DEVICE					/*RDM*/

		/* C1 control can be sent in the only 8bit mode	*/
		if (_cld wvt$l_vt200_common_flags & vtc1_m_c1_transmission_mode)
		    strcpy(secda, "\233>28;" );
		else
		    strcpy(secda, "\033[>28;" );

		version_len = strlen( decterm_version );
		dot = (char *) strchr( decterm_version, '.' );
		if ( dot == 0 )
		    dot = decterm_version + version_len;
		if ( decterm_version[1] != '0' )
			strncat(secda, decterm_version+1,
  				dot - decterm_version - 1 );
		if ( dot < decterm_version + version_len - 1 )
			strncat(secda, dot + 1, 1 );
		else
			strcat(secda, "0" );
		if ( _cld wvt$l_ext_flags & vte1_m_tomcat ) {
		if ( _cld wvt$l_ext_specific_flags & vte2_m_kanji_78 )
		        strcat(secda, ";1c" );
                else
		        strcat(secda, ";2c" );
		} else if ( _cld wvt$l_ext_flags & vte1_m_bobcat )
		        strcat(secda, ";4c" );
		else if ( _cld wvt$l_ext_flags & vte1_m_dickcat )
		        strcat(secda, ";6c" );
		else if ( _cld wvt$l_ext_flags & vte1_m_fishcat )
		        strcat(secda, ";5c" );
		else if ( _cld wvt$l_ext_flags & vte1_m_greek )
			strcat(secda, ";7c" );
		else if ( _cld wvt$l_ext_flags & vte1_m_turkish )
			strcat(secda, ";8c" );
		else
		    strcat(secda, ";0c" );
#endif									/*RDM*/

		WVT$TERMINAL_TRANSMIT(ld, secda);
		break;
	}
   }                                         
}


/************************************************/
xda1rpt(ld) /* Primary Device Attributes Report */
/************************************************/

wvtp ld;

/*
 *
 *   Primary DA strings
 *
 */

{

 unsigned short *temp;
 char da[100];			/*** CAUTION:  watch size with strcat's ! ***/

   if (_cld wvt$b_alt_term_id)
	{
	switch (_cld wvt$l_ext_flags)
	{
	case vte1_m_tomcat:
	 if (_cld wvt$b_alt_term_id == VT80_ID)
		WVT$TERMINAL_TRANSMIT(ld, "\033[?18;2c");
	  else if (_cld wvt$b_alt_term_id == VT100_ID)
		WVT$TERMINAL_TRANSMIT(ld, "\033[?1;2c");
	  else if (_cld wvt$b_alt_term_id == VT100J_ID)
		WVT$TERMINAL_TRANSMIT(ld, "\033[?5;2c");
	  else if (_cld wvt$b_alt_term_id == VT101_ID)
		WVT$TERMINAL_TRANSMIT(ld, "\033[?1;0c");
	  else if (_cld wvt$b_alt_term_id == VT102_ID)
		WVT$TERMINAL_TRANSMIT(ld, "\033[?6c");
	  else if (_cld wvt$b_alt_term_id == VT102J_ID)
		WVT$TERMINAL_TRANSMIT(ld, "\033[?15c");
          else if (_cld wvt$b_alt_term_id == VT220J_ID)
		WVT$TERMINAL_TRANSMIT(ld,"\233?62;1;2;5;6;8c");
          else if (_cld wvt$b_alt_term_id == VT282_ID)
		WVT$TERMINAL_TRANSMIT(ld,"\233?62;1;2;4;5;6;8;10;11c");
          else if (_cld wvt$b_alt_term_id == VT284_ID)
		WVT$TERMINAL_TRANSMIT(ld,"\233?62;1;2;3;4;5;6;8;10;11c");
          else if (_cld wvt$b_alt_term_id == VT286_ID)
		WVT$TERMINAL_TRANSMIT(ld,"\233?62;1;2;3;4;5;6;8;10;11c");
          else if (_cld wvt$b_alt_term_id == VT320_ID)
		WVT$TERMINAL_TRANSMIT(ld,"\233?63;1;2;6;8c");
          else if (_cld wvt$b_alt_term_id == VT382_ID)
		WVT$TERMINAL_TRANSMIT(ld,"\233?63;1;2;4;5;6;8;10;15c");
	  else WVT$TERMINAL_TRANSMIT(ld, "\033[?1;2c");
	  break;

	case vte1_m_bobcat:
	 if (_cld wvt$b_alt_term_id == VT100_ID)
		WVT$TERMINAL_TRANSMIT(ld, "\033[?1;2c");
	  else if (_cld wvt$b_alt_term_id == VT101_ID)
		WVT$TERMINAL_TRANSMIT(ld, "\033[?1;0c");
	  else if (_cld wvt$b_alt_term_id == VT102_ID)
		WVT$TERMINAL_TRANSMIT(ld, "\033[?6c");
          else if (_cld wvt$b_alt_term_id == VT220_ID)
		WVT$TERMINAL_TRANSMIT(ld,"\233?62;1;2;6;8;9c");
          else if (_cld wvt$b_alt_term_id == VT320_ID)
		WVT$TERMINAL_TRANSMIT(ld,"\233?63;1;2;6;8c");
          else if (_cld wvt$b_alt_term_id == VT382CB_ID)
		WVT$TERMINAL_TRANSMIT(ld,"\233?63;1;2;4;6;8;15;30c");
	  else WVT$TERMINAL_TRANSMIT(ld, "\033[?1;2c");
	  break;

	case vte1_m_dickcat:
	 if (_cld wvt$b_alt_term_id == VT100_ID)
		WVT$TERMINAL_TRANSMIT(ld, "\033[?1;2c");
	  else if (_cld wvt$b_alt_term_id == VT101_ID)
		WVT$TERMINAL_TRANSMIT(ld, "\033[?1;0c");
	  else if (_cld wvt$b_alt_term_id == VT102_ID)
		WVT$TERMINAL_TRANSMIT(ld, "\033[?6c");
          else if (_cld wvt$b_alt_term_id == VT220_ID)
		WVT$TERMINAL_TRANSMIT(ld,"\233?62;1;2;6;8;9c");
          else if (_cld wvt$b_alt_term_id == VT320_ID)
		WVT$TERMINAL_TRANSMIT(ld,"\233?63;1;2;6;8c");
          else if (_cld wvt$b_alt_term_id == VT382K_ID)
		WVT$TERMINAL_TRANSMIT(ld,"\233?63;1;2;4;6;8;15;33c");
	  else WVT$TERMINAL_TRANSMIT(ld, "\033[?1;2c");
	  break;

	case vte1_m_fishcat:
	 if (_cld wvt$b_alt_term_id == VT100_ID)
		WVT$TERMINAL_TRANSMIT(ld, "\033[?1;2c");
	  else if (_cld wvt$b_alt_term_id == VT101_ID)
		WVT$TERMINAL_TRANSMIT(ld, "\033[?1;0c");
	  else if (_cld wvt$b_alt_term_id == VT102_ID)
		WVT$TERMINAL_TRANSMIT(ld, "\033[?6c");
          else if (_cld wvt$b_alt_term_id == VT220_ID)
		WVT$TERMINAL_TRANSMIT(ld,"\233?62;1;2;6;8;9c");
          else if (_cld wvt$b_alt_term_id == VT320_ID)
		WVT$TERMINAL_TRANSMIT(ld,"\233?63;1;2;6;8c");
          else if (_cld wvt$b_alt_term_id == VT382D_ID)
		WVT$TERMINAL_TRANSMIT(ld,"\233?63;1;2;4;6;8;15;40c");
	  else WVT$TERMINAL_TRANSMIT(ld, "\033[?1;2c");
	  break;

	default:
	 if (_cld wvt$b_alt_term_id == VT100_ID)
		WVT$TERMINAL_TRANSMIT(ld, "\033[?1;2c");
	  else if (_cld wvt$b_alt_term_id == VT101_ID)
		WVT$TERMINAL_TRANSMIT(ld, "\033[?1;0c");
	  else if (_cld wvt$b_alt_term_id == VT102_ID)
		WVT$TERMINAL_TRANSMIT(ld, "\033[?6c");
	  else if (_cld wvt$b_alt_term_id == VT125_ID)
		WVT$TERMINAL_TRANSMIT(ld, "\033[?12;7;1;10;102c");
	  else if (_cld wvt$b_alt_term_id == VT220_ID)
		WVT$TERMINAL_TRANSMIT(ld, "\233?62;1;2;6;8;9c");
	  else if (_cld wvt$b_alt_term_id == VT240_ID)
		WVT$TERMINAL_TRANSMIT(ld, "\233?62;1;2;3;4;6;8;9c");
	  else if (_cld wvt$b_alt_term_id == VT320_ID)
		WVT$TERMINAL_TRANSMIT(ld, "\233?63;1;2;6;8;9c");
	  else if (_cld wvt$b_alt_term_id == VT330_ID
	    ||     _cld wvt$b_alt_term_id == VT340_ID )
		WVT$TERMINAL_TRANSMIT(ld,"\233?63;1;2;3;4;6;8;9;13;15;16;18;19c");
	  else WVT$TERMINAL_TRANSMIT(ld, "\033[?1;2c");

	  break;
	}
	}
 else
	{

	 strcpy (da, "\233");

/*
 * Primary device attributes.  As they are
 * added to the emulator, we'll add them here.
 *
 * 62	= LEVEL 2	(VT200)
 * 63	= LEVEL 3	(VT300/PANDA) ***** CONDITIONALLY REMOVED *****
 *
 *  1	= 132 columns		YES
 *  2	= Printer Port		 NO
 *  3	= ReGIS			YES
 *  4	= Sixel			YES
 *  5	= Katakana		 NO
 *  6	= Selective Erase	YES
 *  7	= DRCS			 NO
 *  8	= UDK			YES
 *  9	= NRC			 NO
 * 10	=
 * 11	= 25th status line	(assumed in 63)
 * 12	= 
 * 13	= Edit mode		 NO
 * 14	= 8-Bit (ISO Latin nr1)	(assumed in 63)
 * 15	= DEC TCS Set		YES
 * 16	= Locator Device Port	YES
 * 17	= Terminal state int	(assumed in 63)
 * 18	= Windowing (?)		 NO
 * 19	= Multiple sessions	 NO
 * 20	= APL Overstrike Set	 NO
 * 21	= Horizontal scroll	 NO
 * 22	= ANSI Color Text	 NO
 * 23	=
 * 24	=
 * 25	=                     
 * 26	=
 * 27	=
 * 28	= Rectangular Area	 NO	(#28 is prelim)
 * 29   = Text Locator          YES
 *
 * 10	= Two-byte Kanji
 * 30	= Two-byte Hanzi
 * 33	= Two-byte Korean
 * 40	= Two-byte Hanyu
 */

#if CONFORMANCE_LEVEL == LEVEL3			/*RDM*/
	 strcat(da, "?63;1;2");			/*RDM*/
#else						/*RDM*/
	 strcat(da, "?62;1");
#endif						/*RDM*/

#if DEVICE_TYPE == DECTERM_DEVICE               /*PAS*/
        /*
         * define DECterm Primary DA
         */
	if (( _cld wvt$l_ext_flags & vte1_m_asian_common ) &&
	    ld->common.regisScreenMode )
	    strcat(da, ";3");
	switch (_cld wvt$l_ext_flags)
	{
	case vte1_m_tomcat:
	  strcat(da, ";4;5;6;8;10;15;16;29");
	  break;

	case vte1_m_bobcat:
	  strcat(da, "4;6;8;15;30");
	  break;

	case vte1_m_dickcat:
	  strcat(da, "4;6;8;15;33");
	  break;

	case vte1_m_fishcat:
	  strcat(da, "4;6;8;15;40");
	  break;

	default:
        strcat(da, ";2;3;4;6;8;9;15;16");

	if ( _cld wvt$l_vt200_flags_2 & vt2_m_ansi_color )
	    {  /* only support color text on a color monitor */
	    strcat(da, ";22" );
	    }
	strcat(da, ";29");

	 break;
	}
#endif


#if DEVICE_TYPE == VWS_DEVICE			/*PAS*/
        /*
         *  VWS (UIS) Primary DA
         *  -- save code fragment for reference
         */
	 temp = _cld wvt$a_ucbadr + UCB$W_VS_STATE;

	 if (_cld wvt$w_print_flags & pf1_m_prt_enabled)
		strcat(da, ";2");		/* Print */

	 if (_ld wvt$l_vt200_specific_flags & vts1_m_regis_available)
		strcat(da, ";3");		/* ReGIS */

	 strcat(da, ";4;6;8;9");

	 if (_cld wvt$l_vt200_flags_2 & vt2_m_enable_ISO_latin)
		strcat(da, ";14");	/* ISO */

	 if (*temp & UCB$M_VS_TCS_PRESENT)
		strcat(da, ";15");	/* TCS */	/*RDM*/

	 if (_cld wvt$l_vt200_flags & vt1_m_enable_locator)
		strcat(da, ";16");	/* Locator */

	 if (*temp & UCB$M_VS_APL_PRESENT)
	    	WVT$TERMINAL_TRANSMIT(ld,";20");	/* APL set */
#endif						/*RDM*/
#if EXTENDED_PANDA
		WVT$TERMINAL_TRANSMIT(ld,";21");	/* Horiz. Scroll */
#endif
#if DEVICE_TYPE == VWS_DEVICE			/*RDM*/
	 /* vt2_m_ansi_color is set if there are at least 8 colors */
	 if (_cld wvt$l_vt200_flags_2 & vt2_m_ansi_color)
		strcat(da, ";22");	/* ANSI color */
#endif						/*RDM*/
#if EXTENDED_PANDA
		WVT$TERMINAL_TRANSMIT(ld,";28");	/* Rect. Area Edit */
#endif

        /*
         * transmit the resulting DA
         */
	 strcat(da, "c");
	 WVT$TERMINAL_TRANSMIT(ld, da);
	}
}

/*************************************/
xscl(ld) /* Select Conformance Level */
/*************************************/

wvtp ld;

{

 _cld wvt$l_vt200_common_flags &= ~vtc1_m_c1_transmission_mode; 

 switch (_cld wvt$l_parms[0])
	{
	 case 61: _cld wvt$b_conformance_level = LEVEL1; xstr(ld); break;
	 case 62: _cld wvt$b_conformance_level = LEVEL2; xstr(ld); break;
	 case 63: _cld wvt$b_conformance_level = LEVEL3; xstr(ld); break;
	 default: break;
	}
 if (_cld wvt$b_conformance_level >= LEVEL2)
	{
	 switch (_cld wvt$l_parms[1])
		{
		 case 0:
		 case 2: _cld wvt$l_vt200_common_flags |=  vtc1_m_c1_transmission_mode; break;
		 case 1: _cld wvt$l_vt200_common_flags &= ~vtc1_m_c1_transmission_mode; break;
		 default: break;
		}
	}
 WVT$SET_TERMINAL_MODE(ld);
}

/********************************/
xstr(ld) /* Soft Terminal Reset */
/********************************/

wvtp ld;


/*
 *
 * Problem: this should be ignored in level 1 mode,
 * but VT200s always execute it.  Emulate VT200 or
 * follow SRM?
 *
 */


/*
 *  Soft Terminal Reset
 *
 *	o Sets text cursor ON
 *	o Sets REPLACE mode
 *	o Sets origin mode ABSOLUTE
 *	o DOES NOT Set auto-wrap FALSE
 *	o Sets KAM UNLOCKED
 *	o Sets Keypad mode NUMERIC
 *	o Sets Cursor Keys NORMAL
 *	o Sets Top Margin 1
 *	o Sets Bottom Margin wvt$l_page_length
 *	o Sets Right and Left margins to extreams
 *	o Sets NRC mode FALSE (MCS mode)
 *	o Sets character sets to power up
 *	o Sets SGR NORMAL
 *	o Sets Selective Erase to ERASABLE
 *	o Sets saved cursor position to 1,1
 *	o Resets the colormap
 * HEB  o Reset right to left
 */

{

char *temp;

WVT$MAIN_DISPLAY(ld);
if (!_cld wvt$b_user_preference_set)
     _cld wvt$b_user_preference_set = SET_SUPPLEMENTAL(_cld wvt$l_ext_flags);

if (_cld wvt$l_vt200_flags_2 & vt2_m_enable_ISO_latin)
 {
    if (IsDECSupplemental(_cld wvt$b_user_preference_set))
	WVT$RESET_KB_COMPOSE(ld);
    else
	WVT$SET_ISO_KB_COMPOSE(ld);
 }

_cld wvt$l_vt200_common_flags &= ~( vtc1_m_insert_mode |
			       	vtc1_m_keypad_mode |
				vtc1_m_cursor_key_mode|
				vtc1_m_kbd_action_mode |
				vtc1_m_locator_report_mode|
				vtc1_m_locator_one_shot);

/* Do this only if not first time reset */
if ( _cld wvt$l_ext_specific_flags & vte2_m_not_first_reset ) {
    _cld wvt$l_ext_specific_flags &= ~vte2_m_rtl;
    ld->common.rightToLeft = FALSE;
WVT$UNLOCK_KEYBOARD(ld);
xdisplaystr(ld);
WVT$STATUS_DISPLAY(ld);
xdisplaystr(ld);
WVT$MAIN_DISPLAY(ld);
} else {
    WVT$UNLOCK_KEYBOARD(ld);
    xdisplaystr(ld);
    WVT$STATUS_DISPLAY(ld);
    _cld wvt$l_ext_specific_flags &= ~vte2_m_not_first_reset;
    xdisplaystr(ld);
    WVT$MAIN_DISPLAY(ld);
}
}

xdisplaystr(ld)
wvtp ld;
{

			/* Clear User Preferred Suplemental flag	*/
_ld wvt$b_save_ups = _ld wvt$b_ups = 0;
if ( _cld wvt$l_ext_flags & vte1_m_tomcat ) {

_ld wvt$b_gl = _ld wvt$b_sav_gl = 0;
_ld wvt$b_gr = _ld wvt$b_sav_gr = 2;

if ( _cld wvt$l_ext_specific_flags & vte2_m_jisroman_mode ) {
  _ld wvt$b_save_g_sets[0]  = _ld wvt$b_g_sets[0] = JIS_ROMAN;
  _ld wvt$b_save_ext_g_sets[0]  = _ld wvt$b_ext_g_sets[0] = ONE_BYTE_SET;
} else {
  _ld wvt$b_save_g_sets[0]  = _ld wvt$b_g_sets[0] = ASCII;
  _ld wvt$b_save_ext_g_sets[0]  = _ld wvt$b_ext_g_sets[0] = STANDARD_SET;
}
_ld wvt$b_save_g_sets[1]  = _ld wvt$b_g_sets[1] = JIS_KATAKANA;
_ld wvt$b_save_ext_g_sets[1]  = _ld wvt$b_ext_g_sets[1] = ONE_BYTE_SET;
_ld wvt$b_save_g_sets[2]  = _ld wvt$b_g_sets[2] = JIS_KATAKANA;
_ld wvt$b_save_ext_g_sets[2]  = _ld wvt$b_ext_g_sets[2] = ONE_BYTE_SET;
_ld wvt$b_save_g_sets[3]  = _ld wvt$b_g_sets[3] = LINE_DRAWING;
_ld wvt$b_save_ext_g_sets[3]  = _ld wvt$b_ext_g_sets[3] = STANDARD_SET;

if ( _cld wvt$l_ext_specific_flags & vte2_m_kanji_mode ) {
  _ld wvt$b_gr = _ld wvt$b_sav_gr = 3;
  _ld wvt$b_save_g_sets[1]  = _ld wvt$b_g_sets[1] = LINE_DRAWING;
  _ld wvt$b_save_ext_g_sets[1]  = _ld wvt$b_ext_g_sets[1] = STANDARD_SET;
  _ld wvt$b_save_g_sets[3]  = _ld wvt$b_g_sets[3] = DEC_KANJI;
  _ld wvt$b_save_ext_g_sets[3]  = _ld wvt$b_ext_g_sets[3] = TWO_BYTE_SET;
}

if (!_cld wvt$b_user_preference_set)
     _cld wvt$b_user_preference_set = SET_SUPPLEMENTAL(_cld wvt$l_ext_flags);

_cld wvt$b_char_stack_top = 0;
_cld wvt$l_ext_specific_flags &= ~vte2_m_sixel_scroll_mode;

} else if ( _cld wvt$l_ext_flags & vte1_m_asian_common ) {
    _ld wvt$b_gl = _ld wvt$b_sav_gl = 0;
    _ld wvt$b_gr = _ld wvt$b_sav_gr = 3;
    _ld wvt$b_save_g_sets[0] = _ld wvt$b_g_sets[0] = ASCII;
    _ld wvt$b_save_g_sets[1] = _ld wvt$b_g_sets[1] = LINE_DRAWING;
    _ld wvt$b_save_ext_g_sets[0] = _ld wvt$b_ext_g_sets[0] =
    _ld wvt$b_save_ext_g_sets[1] = _ld wvt$b_ext_g_sets[1] = STANDARD_SET;
    if ( _cld wvt$l_ext_flags & vte1_m_bobcat ) {
	_ld wvt$b_save_g_sets[2] = _ld wvt$b_g_sets[2] = ASCII;
	_ld wvt$b_save_g_sets[3] = _ld wvt$b_g_sets[3] = DEC_HANZI;
	_ld wvt$b_save_ext_g_sets[2] = _ld wvt$b_ext_g_sets[2] = STANDARD_SET;
	_ld wvt$b_save_ext_g_sets[3] = _ld wvt$b_ext_g_sets[3] = TWO_BYTE_SET;
    } else if ( _cld wvt$l_ext_flags & vte1_m_dickcat ) {
	if ( _cld wvt$l_ext_specific_flags & vte2_m_ksroman_mode ) {
	    _ld wvt$b_save_g_sets[0] = _ld wvt$b_g_sets[0] = KS_ROMAN;
	    _ld wvt$b_save_ext_g_sets[0] = _ld wvt$b_ext_g_sets[0] = ONE_BYTE_SET;
	}
	_ld wvt$b_save_g_sets[2] = _ld wvt$b_g_sets[2] = DEC_HANGUL;
	_ld wvt$b_save_g_sets[3] = _ld wvt$b_g_sets[3] = DEC_HANGUL;
	_ld wvt$b_save_ext_g_sets[2] = _ld wvt$b_ext_g_sets[2] = TWO_BYTE_SET;
	_ld wvt$b_save_ext_g_sets[3] = _ld wvt$b_ext_g_sets[3] = TWO_BYTE_SET;
    } else if ( _cld wvt$l_ext_flags & vte1_m_fishcat ) {
	_ld wvt$b_save_g_sets[2] = _ld wvt$b_g_sets[2] = ASCII;
	_ld wvt$b_save_g_sets[3] = _ld wvt$b_g_sets[3] = DEC_HANYU;
	_ld wvt$b_save_ext_g_sets[2] = _ld wvt$b_ext_g_sets[2] = STANDARD_SET;
	_ld wvt$b_save_ext_g_sets[3] = _ld wvt$b_ext_g_sets[3] = FOUR_BYTE_SET;
    }
    if (!_cld wvt$b_user_preference_set)
        _cld wvt$b_user_preference_set = SET_SUPPLEMENTAL(_cld wvt$l_ext_flags);

    _cld wvt$b_char_stack_top = 0;
    _cld wvt$l_ext_specific_flags &= ~vte2_m_sixel_scroll_mode;
	
} else {

_ld wvt$b_save_ext_g_sets[0]  = _ld wvt$b_ext_g_sets[0] = 
_ld wvt$b_save_ext_g_sets[1]  = _ld wvt$b_ext_g_sets[1] = 
_ld wvt$b_save_ext_g_sets[2]  = _ld wvt$b_ext_g_sets[2] = 
_ld wvt$b_save_ext_g_sets[3]  = _ld wvt$b_ext_g_sets[3] = STANDARD_SET;


_ld wvt$b_sav_gl = _ld wvt$b_gl = 0;
_ld wvt$b_sav_gr = _ld wvt$b_gr = 2;

_ld wvt$b_save_g_sets[0]  = _ld wvt$b_g_sets[0] =
_ld wvt$b_save_g_sets[1]  = _ld wvt$b_g_sets[1] = ASCII;

if (_cld wvt$b_conformance_level >= LEVEL2)
 {
  _ld wvt$b_save_g_sets[2]  = _ld wvt$b_save_g_sets[3] =
  _ld wvt$b_g_sets[2]       = _ld wvt$b_g_sets[3]      = _cld wvt$b_user_preference_set;
    if ( _cld wvt$b_user_preference_set == ISO_LATIN_8 ||
	 _cld wvt$b_user_preference_set == HEB_SUPPLEMENTAL )
	_ld wvt$b_save_ext_g_sets[2] = _ld wvt$b_ext_g_sets[2] =
	_ld wvt$b_save_ext_g_sets[3] = _ld wvt$b_ext_g_sets[3] = ONE_BYTE_SET;
 }
else
 {
  _ld wvt$b_save_g_sets[2]  = _ld wvt$b_g_sets[2] =
  _ld wvt$b_save_g_sets[3]  = _ld wvt$b_g_sets[3] = ASCII;
 }

}

	/* If in CRM char set should be CRM font	*/
if (( _cld wvt$l_ext_flags & vte1_m_asian_common ) &&
    ( _cld wvt$l_vt200_flags & vt1_m_display_controls_mode ))
        WVT$CONTROL_FONT (ld);

_ld wvt$l_save_line = _ld wvt$l_save_column = _ld wvt$l_top_margin = _ld wvt$l_left_margin = 1;
_ld wvt$w_save_rendition = _ld wvt$w_actv_rendition = csa_M_SELECTIVE_ERASE;

_ld wvt$l_vt200_specific_flags &= ~( vts1_m_origin_mode|
				     vts1_m_last_column );

/* If first time, set 'not_first_reset', reset nrc_mode if not Hebrew dialect */
if (!( _cld wvt$l_ext_specific_flags & vte2_m_not_first_reset )) {
    _cld wvt$l_ext_specific_flags |= vte2_m_not_first_reset;
    if ( ld->common.keyboardDialect != DECwHebrewDialect )
	_cld wvt$l_vt200_flags &= ~(	vt1_m_nrc_mode);
} else
_cld wvt$l_vt200_flags &= ~(	vt1_m_nrc_mode);

_cld wvt$l_vt200_flags_2 &= ~(	vt2_m_vss_scroll_mode );

_ld wvt$l_bottom_margin = _ld wvt$l_page_length;
_ld wvt$l_right_margin  = _ld wvt$l_column_width;      
_ld wvt$a_cur_cod_ptr     = 0;

_ld wvt$l_defer_max = 10;

/*
 *  Enable the text cursor.
 */

_cld wvt$b_cursts |= cs1_m_cs_enabled;
WVT$ENABLE_CURSOR( ld );

/*
 *  Init the ReGIS parser
 */
if (_ld wvt$l_vt200_specific_flags & vts1_m_regis_available) WVT$CLEAR_REGIS(ld);

WVT$SET_MISC_MODES( ld );   /* set modes */
}
