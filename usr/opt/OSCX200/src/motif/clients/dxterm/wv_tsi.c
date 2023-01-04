/* #module wv_tsi "X3.0-6" */
/*
 *  Title:	wv_tsi.c - terminal state interrogation routines
 *
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © Digital Equipment Corporation, 1988, 1993 All Rights       |
 *  | Reserved.  Unpublished rights reserved under the copyright laws of     |
 *  | the United States.                                                     |
 *  |                                                                        |
 *  | The software contained on this media is proprietary to and embodies    |
 *  | the confidential technology of Digital Equipment Corporation.          |
 *  | Possession, use, duplication or dissemination of the software and      |
 *  | media is authorized only pursuant to a valid written license from      |
 *  | Digital Equipment Corporation.                                         |
 *  |                                                                        |
 *  | RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the      |
 *  | U.S. Government is subject to restrictions as set forth in             |
 *  | Subparagraph (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19,    |
 *  | as applicable.                                                         |
 *  |                                                                        |
 *  | The information in this software is subject to change  without  notice |
 *  | and  should  not  be  construed  as  a commitment by Digital Equipment |
 *  | Corporation.                                                           |
 *  |                                                                        |
 *  | DIGITAL assumes no responsibility for the use or  reliability  of  its |
 *  | software on equipment which is not supplied by DIGITAL.                |
 *  +------------------------------------------------------------------------+
 *  
 *  
 *  Module Abstract:
 *
 *	<short description of module contents>
 *
 *  Procedures contained in this module:
 *
 *	<list of procedure names and abstracts>
 *
 *  Author:	Tom Porcher   29-Jun-1988
 *
 *  Modification history:
 *
 * Alfred von Campe     30-Sep-1993     BL-E
 *      - Add multi-page support.
 *
 * Eric Osman		30-Jul-1993	BL-D
 *      - Merge vxt and vms decterm sources.
 *
 * Aston Chan		12-Mar-1993	V1.2/BL1
 *	- Add Turkish/Greek support.
 *	- DECCIR should response active line and active column instead of
 *	  the saved ones.
 *
 * Alfred von Campe     04-Mar-1992     V3.1
 *      - Fix DECRQM sequence for DECNKM mode, which was returning
 *        RQM_UNKNOWN_MODE instead of RQM_SET_MODE or RQM_RESET_MODE.
 *
 * Aston Chan		17-Dec-1991	V3.1
 *	- I18n code merge
 *
 * Aston Chan		04-Nov-1991	V3.1
 *	- Add Bob's fix in rgbhls() to avoid being divided by zero.
 *
 * Aston Chan		1-Sep-1991	Alpha
 *	- Add <> and .h to #include's.  Complained by DECC compiler Release 10.
 *
 * Bob Messenger	27-Aug-1990	X3.0-6
 *	Fix rgbhls() for red to yellow range.
 *
 *  Michele Lien    24-Aug-1990 VXT X0.0
 *  - Modify this module to work on VXT platform. Change #ifdef VMS to
 *    #ifdef VMS_DECTERM so that VXT specific code can be compiled under
 *    VMS or ULTRIX development environment.
 *
 * Bob Messenger        17-Jul-1990     X3.0-5
 *      Merge in Toshi Tanimoto's changes to support Asian terminals -
 *	- TSI specific to Kanji terminal (Tomcat)
 *
 * Bob Messenger	13-Jul-1990	X3.0-5
 *	- Add DECRPSS reports for DECELR, DECEFR an DECSLE.
 *	- Fix report for conformance level 1 (again).
 *
 * Mark Woodbury	25-May-1990	X3.0-3M
 *	- Motif update
 *
 * Bob Messenger	13-Apr-1990	X3.0-2
 *	- Use 8 bit control characters in reports.
 *
 * Bob Messenger	 9-Apr-1990	X3.0-2
 *	- Merge UWS and VMS changes.
 *
 * Bob Messenger	12-Mar-1990	V2.1 (VMS V5.4)
 *	- Color table reports should start with DCS, not CSI.
 *	- Fix report for conformance level 1.
 *
 * Bob Messenger	 2-Mar-1990	V2.1 (UWS V4.0)
 *	- Don't rely on the value returned by sprintf (fixes crash on Ultrix
 *	  when generating color table reports).
 *
 * Bob Messenger	21-Apr-1989	X2.0-7
 *	- Avoid compilation warnings on Ultrix.
 *
 * Bob Messenger	11-Apr-1989	X2.0-6
 *	- Implement color table reports.
 *
 * Bob Messenger	 2-Apr-1989	X2.0-5
 *	- Moved wvt$l_column_width to specific area.
 *
 * Bob Messenger	30-Jan-1989	X1.1-1
 *	- Make DECRPSS conform to spec: 0 means invalid, 1 means valid.
 *
 * Bob Messenger	16-Jan-1989	X1.1-1
 *	- Moved many ld fields into common area.
 *
 * Bob Messenger	11-Jan-1988	X1.1-1
 *	- Moved wvt$b_cursts to common area
 *
 */


#include "wv_hdr.h"
#include <ctype.h>

/*
 *	Terminal State Interrogation
 *	----------------------------
 *	
 *	Sequences:
 *	
 *	DECRQM	    CSI (?) Ps $ p
 *		response:	DECRPM	    CSI (?) Ps ; Ps $ y
 *	
 *	DECNKM	    SM/RM 66	(set=application)
 *	
 *	DECRQSS	    DCS $ q ... ST
 *		response:	DECRPSS	    DCS Ps $ r ... ST
 *	
 *	DECRQPSR    CSI ps $ w
 *		response:	DECPSR	    DCS Ps $ u ... ST
 *	
 *	DECRSPS	    DCS Ps $ t ... ST
 *	
 *	DECRQTSR    CSI Ps $ u
 *		response:	DECTSR	    DCS Ps $ s ... ST
 *	
 *	DECRSTS	    DCS Ps $ p
 *	
 */                                      
#define RQM_UNKNOWN_MODE 0
#define RQM_SET_MODE     1
#define RQM_RESET_MODE   2
#define RQM_SET_PERM     3
#define RQM_RESET_PERM   4               
void
xdecrqm( ld )
wvtp ld;
{
register int n, y, response;

 if (_cld wvt$b_parmcnt != 1) return;
 response = RQM_UNKNOWN_MODE;
 n = 1;  /* used to be a loop */
  switch (_cld wvt$b_privparm)
    {

    case 0:	/*
		 *  ANSI standard terminal modes.
		 *
		 */

      switch (_cld wvt$l_parms[n-1])
        {
	case 1: /* GATM */
		response = RQM_RESET_PERM;
		break;		
        case 2:	/* KAM */
		 if (_cld wvt$l_vt200_common_flags & vtc1_m_kbd_action_mode)
			response = RQM_SET_MODE;
		 else
			response = RQM_RESET_MODE;
		 break;

	case 3: /* CRM */
		response = RQM_RESET_MODE;
		break;
        case 4:	/*  IRM */
		 if (_cld wvt$l_vt200_common_flags & vtc1_m_insert_mode)
			response = RQM_SET_MODE;
		 else
			response = RQM_RESET_MODE;
		 break;
        case 5: /* SRTM */
		 response = RQM_RESET_PERM;
		 break;

	case 6: /* ERM */
		 response = RQM_UNKNOWN_MODE;
		break;

	case 7: /* VEM */
		 response = RQM_RESET_PERM;
		 break;
	case 10: /* HEM */
		response = RQM_RESET_PERM;
		break;
	case 11: /* PUM */
		response = RQM_RESET_PERM;
		break;
	case 12: /* SRM */
		if (_cld wvt$l_vt200_common_flags & vtc1_m_echo_mode)
			response = RQM_SET_MODE;
		else
			response = RQM_RESET_MODE;
		 break;

	case 13: /* FEAM */
		response = RQM_RESET_PERM;
		break;
	case 14: /* FETM */
		response = RQM_RESET_PERM;
		break;
	case 15: /* MATM */
		response = RQM_RESET_PERM;
		break;
	case 16: /* TTM */
		response = RQM_RESET_PERM;
		break;
	case 17: /* SATM */
		response = RQM_RESET_PERM;
		break;
	case 18: /* TSM */
		response = RQM_RESET_PERM;
		break;
	case 19: /* EBM */
		response = RQM_RESET_PERM;
		break;
        case 20:/* LNM */
		if (_cld wvt$l_vt200_flags & vt1_m_new_line_mode)
			response = RQM_SET_MODE;
		else
			response = RQM_RESET_MODE;
		break;

        default:
		response = RQM_UNKNOWN_MODE;
		break;
        }
      break;

    case 1:   	/*
		 *  DEC PRIVATE MODES (CSI ? pn h)
		 *
		 */

      switch (_cld wvt$l_parms[n-1])
        {

        case 1:	/* DECCKM */
		if (_cld wvt$l_vt200_common_flags & vtc1_m_cursor_key_mode)
			response = RQM_SET_MODE;
		else
			response = RQM_RESET_MODE;
		 break;

	case 2:	/* DECANM */
		 response = RQM_SET_MODE;
		 break;

        case 3:	/* DECCOLM */
		 if (_ld wvt$l_column_width > 80)
			response = RQM_SET_MODE;
		 else
			response = RQM_RESET_MODE; 
                 break;

        case 4:	/* DECSCLM */
                 if (_cld wvt$l_vt200_flags & vt1_m_scroll_mode)
			response = RQM_SET_MODE;
		 else
			response = RQM_RESET_MODE;
		 break;

        case 5:	/* DECSCNM */
                 if (_cld wvt$l_vt200_common_flags & vtc1_m_screen_mode)
			response = RQM_SET_MODE;
		 else
			response = RQM_RESET_MODE;
		 break;

        case 6:	/* DECOM */
		 if (_ld wvt$l_vt200_specific_flags & vts1_m_origin_mode)
		       	response = RQM_SET_MODE;
		 else 
			response = RQM_RESET_MODE;
		 break;

	case 7:	/* DECAWM */
		 if (_ld wvt$l_vt200_specific_flags & vts1_m_auto_wrap_mode)
		 	response = RQM_SET_MODE;
		 else
			response = RQM_RESET_MODE;
		 break;

        case 8:	/* DECARM */
                 if (_cld wvt$l_kb_attributes & UIS$M_KB_AUTORPT)
			response = RQM_SET_MODE;
		 else
			response = RQM_RESET_MODE;
		 break;

        case 18:/* DECPFF */
		if (_cld wvt$w_print_flags & pf1_m_prt_ff_mode)
			response = RQM_SET_MODE;
		else
			response = RQM_RESET_MODE;
		 break;

        case 19:/* DECPEX */
		if (_cld wvt$w_print_flags & pf1_m_prt_extent_mode)
			response = RQM_SET_MODE;
		else
			response = RQM_RESET_MODE;
		break;

        case 25:/* DECTCEM */
		if (_cld wvt$b_cursts & cs1_m_cs_enabled)
			response = RQM_SET_MODE;
		else
			response = RQM_RESET_MODE;
                 break;

	case 42:/* DECNRCM */
		if (_cld wvt$l_vt200_flags & vt1_m_nrc_mode)
			response = RQM_SET_MODE;
		else
			response = RQM_RESET_MODE;
		 break;

	case 59:/* DECKKDM */
		if (_cld wvt$l_ext_flags & vte1_m_tomcat) {
		    if (_cld wvt$l_ext_specific_flags & vte2_m_kanji_mode)
			response = RQM_SET_MODE;
		    else
			response = RQM_RESET_MODE;
		}
		 break;

	case 66:/* DECNKM */	/* CSI ? 6 6 h */
		if (_cld wvt$l_vt200_common_flags & vtc1_m_keypad_mode)
			response = RQM_SET_MODE;
		else
			response = RQM_RESET_MODE;
		 break;

	case 67:/* DECBKM */
		if (_cld wvt$l_vt200_flags_2 & vt2_m_backarrow_mode)
                	response = RQM_SET_MODE;
		else	
			response = RQM_RESET_MODE;
		 break;

	case 80:/* DECSDM */
		if (_cld wvt$l_ext_flags & vte1_m_asian_common) {
		    if (_cld wvt$l_ext_specific_flags & vte2_m_sixel_scroll_mode)
			response = RQM_SET_MODE;
		    else
			response = RQM_RESET_MODE;
		}
		 break;

	case 92:/* DECLCSM */	/* CSI ? 9 2 h */
		if ( _cld wvt$l_ext_flags & vte1_m_fishcat ) {
		    if (_cld wvt$l_ext_specific_flags & vte2_m_leading_code_mode)
			response = RQM_SET_MODE;
		    else
			response = RQM_RESET_MODE;
		}
		 break;

        default: response = RQM_UNKNOWN_MODE; break;

        }

      break;

    default: response = RQM_UNKNOWN_MODE; break;
    }
	/* 7bit controlls mode	*/
    if ( !(_cld wvt$l_vt200_common_flags & vtc1_m_c1_transmission_mode) )
        WVT$TERMINAL_TRANSMIT(ld, "\033[");
    else
    WVT$TERMINAL_TRANSMIT(ld, "\233");
    if (_cld wvt$b_privparm) WVT$TERMINAL_TRANSMIT(ld, "?");
    xmit_value(ld, _cld wvt$l_parms[0]);
    WVT$TERMINAL_TRANSMIT(ld, ";");
    xmit_value(ld, response);
    WVT$TERMINAL_TRANSMIT(ld, "$y");
}
void
xdecrqpsr( ld )
wvtp ld;
{
if (_cld wvt$b_parmcnt != 1) return;
if (_cld wvt$l_parms[0] == 1) xdeccir(ld);
if (_cld wvt$l_parms[0] == 2) xdectabsr(ld);
};

xdectabsr(ld)
wvtp ld;
{
int i, not_first_tab_stop = 0;

	/* 7bit controlls mode	*/
    if ( !(_cld wvt$l_vt200_common_flags & vtc1_m_c1_transmission_mode) )
	WVT$TERMINAL_TRANSMIT(ld, "\033P2$u");
    else
WVT$TERMINAL_TRANSMIT(ld, "\2202$u");
i = 0;
		/* Tab Stop Report should begin 2nd column,		*/
		/*		because Tab don't set 1st column.	*/
for (i=2; i <= _ld wvt$l_column_width; i++) {
   if (_cld wvt$b_tab_stops[i]) {
	if (not_first_tab_stop) WVT$TERMINAL_TRANSMIT(ld, "/");
	xmit_value(ld, i);
	not_first_tab_stop = 1;
    }
}
	/* 7bit controlls mode	*/
    if ( !(_cld wvt$l_vt200_common_flags & vtc1_m_c1_transmission_mode) )
	WVT$TERMINAL_TRANSMIT(ld, "\033\\");
    else
WVT$TERMINAL_TRANSMIT(ld, "\234");
}

xdeccir( ld )
wvtp ld;
{
/*	EIC_JPN							*/
/*	The current active settings are sent as DECCIR		*/
/*	But original DECterm sends saved settings as DECCIR,	*/
/*	and sends illegal value as selective erase flag.	*/
unsigned char ch, c[3];

c[1] = ';';
c[2] = '\000';

	/* 7bit controlls mode	*/
if ( !( _cld wvt$l_vt200_common_flags & vtc1_m_c1_transmission_mode ))
    WVT$TERMINAL_TRANSMIT(ld, "\033P1$u");  /* DCS 1 $ u */
else
WVT$TERMINAL_TRANSMIT(ld, "\2201$u");  /* DCS 1 $ u */
xmit_value(ld, _ld wvt$l_actv_line);    /* line ; */
WVT$TERMINAL_TRANSMIT(ld, ";");
xmit_value(ld, _ld wvt$l_actv_column);  /* column ; */
WVT$TERMINAL_TRANSMIT(ld, ";");
xmit_value(ld, _cld current_page + 1);  /* page ; */
WVT$TERMINAL_TRANSMIT(ld, ";");
ch = 0x40;
if (_ld wvt$w_actv_rendition & csa_M_REVERSE) ch |= 8;
if (_ld wvt$w_actv_rendition & csa_M_BLINK) ch |= 4;
if (_ld wvt$w_actv_rendition & csa_M_UNDERLINE) ch |= 2;
if (_ld wvt$w_actv_rendition & csa_M_BOLD) ch |= 1;
c[0] = ch;
WVT$TERMINAL_TRANSMIT(ld, c);
ch = 0x40;
if (_ld wvt$w_actv_rendition & csa_M_SELECTIVE_ERASE) ch |= 1;
c[0] = ch;
WVT$TERMINAL_TRANSMIT(ld, c);
ch = 0x40;
if (_ld wvt$l_vt200_specific_flags & vts1_m_auto_wrap_mode) ch |= 8;
if (_ld wvt$l_vt200_specific_flags & vts1_m_origin_mode) ch |= 1;
if (_ld wvt$b_single_shift == SS_3) ch |= 4;
if (_ld wvt$b_single_shift == SS_2) ch |= 2;
c[0] = ch;
WVT$TERMINAL_TRANSMIT(ld, c);
xmit_value(ld, _ld wvt$b_gl);
WVT$TERMINAL_TRANSMIT(ld, ";");
xmit_value(ld, _ld wvt$b_gr);
WVT$TERMINAL_TRANSMIT(ld, ";");
ch = 0x40;
if (_ld wvt$b_g_sets[0] == ISO_LATIN_1 && !(_ld wvt$b_ups & 1))  ch |= 1;
if (_ld wvt$b_g_sets[1] == ISO_LATIN_1 && !(_ld wvt$b_ups & 2)) ch |= 2;
if (_ld wvt$b_g_sets[2] == ISO_LATIN_1 && !(_ld wvt$b_ups & 4)) ch |= 4;
if (_ld wvt$b_g_sets[3] == ISO_LATIN_1 && !(_ld wvt$b_ups & 8)) ch |= 8;
c[0] = ch;
WVT$TERMINAL_TRANSMIT(ld, c);
tsi_transmit_gset(ld, 0);
tsi_transmit_gset(ld, 1);
tsi_transmit_gset(ld, 2);
tsi_transmit_gset(ld, 3);
	/* 7bit controlls mode	*/
if ( !(_cld wvt$l_vt200_common_flags & vtc1_m_c1_transmission_mode) )
	WVT$TERMINAL_TRANSMIT(ld, "\033\\");
else
WVT$TERMINAL_TRANSMIT(ld, "\234");
}

tsi_transmit_gset(ld, n)
wvtp ld;
int n;
{
	switch ( _ld wvt$b_ext_g_sets[n] ) {
	case ONE_BYTE_SET:
	    if ( _ld wvt$b_g_sets[n] == JIS_ROMAN) {
		WVT$TERMINAL_TRANSMIT(ld, "J");
		return;
	    }
	    if ( _ld wvt$b_g_sets[n] == JIS_KATAKANA) {
		WVT$TERMINAL_TRANSMIT(ld, "I");
		return;
	    }
	    if ( _ld wvt$b_g_sets[n] == KS_ROMAN) {
		WVT$TERMINAL_TRANSMIT(ld, "%?");
		return;
	    }
	    if (_ld wvt$b_g_sets[n] == HEB_SUPPLEMENTAL) {
		WVT$TERMINAL_TRANSMIT(ld, "\"4");
		return;                      
	    }
	    if (_ld wvt$b_g_sets[n] == ISO_LATIN_8) {
		WVT$TERMINAL_TRANSMIT(ld, "H");
		return;
	    }

	    break;
	case TWO_BYTE_SET:
	    if ( _ld wvt$b_g_sets[n] == DEC_KANJI ) {
		WVT$TERMINAL_TRANSMIT( ld, "$3" );
		return;
	    }
	    if ( _ld wvt$b_g_sets[n] == DEC_HANZI ) {
		WVT$TERMINAL_TRANSMIT( ld, "$2" );
		return;
	    }
	    if ( _ld wvt$b_g_sets[n] == DEC_HANGUL ) {
		WVT$TERMINAL_TRANSMIT( ld, "$4" );
		return;
	    }
	    break;
	case FOUR_BYTE_SET:
	    if ( _ld wvt$b_g_sets[n] == DEC_HANYU ) {
		WVT$TERMINAL_TRANSMIT( ld, "$5" );
		return;
	    }
	    break;
	case STANDARD_SET:
	if (_ld wvt$b_g_sets[n] == ASCII) {
		switch (_ld wvt$b_nrc_set) {
			case 0:
				WVT$TERMINAL_TRANSMIT(ld, "B");
				break;
			case 2:
				WVT$TERMINAL_TRANSMIT(ld, "A");
				break;
			case 4:
				WVT$TERMINAL_TRANSMIT(ld, "Q");
				break;
			case 5:  
				WVT$TERMINAL_TRANSMIT(ld, "6");
				break;
			case 6:
				WVT$TERMINAL_TRANSMIT(ld, "5");
				break;
			case 7:
				WVT$TERMINAL_TRANSMIT(ld, "K");
				break;
			case 8:
				WVT$TERMINAL_TRANSMIT(ld, "4");
				break;
		 	case 9:
				WVT$TERMINAL_TRANSMIT(ld, "Y");
				break;
			case 10:
				WVT$TERMINAL_TRANSMIT(ld, "=");
				break;
			case 12:
				WVT$TERMINAL_TRANSMIT(ld, "7");
				break;
			case 14:
				WVT$TERMINAL_TRANSMIT(ld, "R");
				break;
			case 15:
				WVT$TERMINAL_TRANSMIT(ld, "Z");
				break;
			case 16:
				WVT$TERMINAL_TRANSMIT(ld, "%6");
				break;
			case 17:
				WVT$TERMINAL_TRANSMIT(ld, "%=");
				break;
			case 18:		/* Turkish NRCS */
				WVT$TERMINAL_TRANSMIT(ld, "%2");
				break;
			case 19:		/* Greek NRCS */
				WVT$TERMINAL_TRANSMIT(ld, "\">");
				break;
			default :
				WVT$TERMINAL_TRANSMIT(ld, "B");
				break;
		}
		return;
	}

	switch (_ld wvt$b_g_sets[n])
	    {
	    case LINE_DRAWING :
		WVT$TERMINAL_TRANSMIT(ld, "0");
		return;

	    case HEB_SUPPLEMENTAL :
		WVT$TERMINAL_TRANSMIT(ld, "\"4");
		return;

	    case ISO_LATIN_8 :
		WVT$TERMINAL_TRANSMIT(ld, "H");
		return;

	    case GREEK_SUPPLEMENTAL :
		WVT$TERMINAL_TRANSMIT(ld, "\"?");
		return;                      
	    case ISO_LATIN_7 :
		WVT$TERMINAL_TRANSMIT(ld, "F");
		return;                      
	    case TURKISH_SUPPLEMENTAL :
		WVT$TERMINAL_TRANSMIT(ld, "%0");
		return;                      
	    case ISO_LATIN_5 :
		WVT$TERMINAL_TRANSMIT(ld, "M");
		return;                      
	    case SUPPLEMENTAL :
		WVT$TERMINAL_TRANSMIT(ld, "%5");
		return;                      
	    case ISO_LATIN_1 :
		WVT$TERMINAL_TRANSMIT(ld, "A");
		return;
	    }

	if ((( _cld wvt$l_ext_flags & vte1_m_asian_common ) &&
	        _ld wvt$b_ups & (1<<n) ) ||
	    ( !( _cld wvt$l_ext_flags & vte1_m_asian_common ) &&
		  _ld wvt$b_g_sets[n] == _cld wvt$b_user_preference_set )) {
		WVT$TERMINAL_TRANSMIT(ld, "<");
		return;
	}
	if (_ld wvt$b_g_sets[n] == TECHNICAL) {
		WVT$TERMINAL_TRANSMIT(ld, ">");
		return;
	}
	break;
	default:

	/* nothing above.  Somehow an unknown character set got stuck in
	  this gset.  Gotta send something.  How about ASCII for the
	  default? */
	WVT$TERMINAL_TRANSMIT(ld, "B");
	break;
	}
}

void
xdecrqtsr( ld )
wvtp ld;
{
if (_cld wvt$b_parmcnt < 1) return;
if (_cld wvt$l_parms[0] == 1 && _cld wvt$b_parmcnt == 1) xdectsr(ld);
else if (_cld wvt$l_parms[0] == 2 && _cld wvt$b_parmcnt <= 2) xdecctr(ld);
};


xdectsr(ld)
wvtp ld;
{
	struct tsr_buffer_struct buff;
        int i;

	/* 8bit controlls mode	*/
    if (!(_cld wvt$l_vt200_common_flags & vtc1_m_c1_transmission_mode))
	tsr_buff_init7(&buff);
    else
	tsr_buff_init(&buff);
	tsr_put_byte(&buff, (char)_ld wvt$l_page_length);
	tsr_put_byte(&buff, (char)_ld wvt$l_column_width);
      	tsr_put_byte(&buff, (char)_ld wvt$l_actv_line);
	tsr_put_byte(&buff, (char)_ld wvt$l_actv_column);
/* Active rendition and conformance level also should be saved.	*/
	tsr_put_word(&buff, _ld wvt$w_actv_rendition);
	tsr_put_word(&buff, _ld wvt$w_actv_ext_rendition);
	tsr_put_byte(&buff, _cld wvt$b_conformance_level);
    tsr_put_byte(&buff, _ld wvt$b_g_sets[0]);
    tsr_put_byte(&buff, _ld wvt$b_g_sets[1]);
    tsr_put_byte(&buff, _ld wvt$b_g_sets[2]);
    tsr_put_byte(&buff, _ld wvt$b_g_sets[3]);
	/* User Preferred Supplemental flag also should be saved.	*/
    tsr_put_byte(&buff, _ld wvt$b_ups);
    tsr_put_byte(&buff, _ld wvt$b_gl);
    tsr_put_byte(&buff, _ld wvt$b_gr);
	tsr_put_byte(&buff, _ld wvt$b_single_shift);
	tsr_put_byte(&buff, (unsigned char)_ld wvt$l_save_line);
	tsr_put_byte(&buff, (unsigned char)_ld wvt$l_save_column);
	tsr_put_word(&buff, _ld wvt$w_save_rendition);
	tsr_put_long(&buff, _ld wvt$l_save_vt200_flags);
      	tsr_put_byte(&buff, _ld wvt$b_save_g_sets[0]);
	tsr_put_byte(&buff, _ld wvt$b_save_g_sets[1]);
	tsr_put_byte(&buff, _ld wvt$b_save_g_sets[2]);
	tsr_put_byte(&buff, _ld wvt$b_save_g_sets[3]);
	tsr_put_byte(&buff, _ld wvt$b_sav_gl);
	tsr_put_byte(&buff, _ld wvt$b_sav_gr);
	for (i=1; i <= MAX_COLUMN; i++)
	   tsr_put_bit(&buff, _cld wvt$b_tab_stops[i]);
      	tsr_put_pad(&buff);
	tsr_put_byte(&buff, (unsigned char)_ld wvt$l_top_margin);
	tsr_put_byte(&buff, (unsigned char)_ld wvt$l_bottom_margin);
	tsr_put_bit(&buff, _cld wvt$l_vt200_flags & vt1_m_nrc_mode);
	tsr_put_bit(&buff, ld->common.statusDisplayEnable);
	tsr_put_bit(&buff, _cld wvt$l_vt200_common_flags &
		vtc1_m_actv_status_display);
	tsr_put_bit(&buff, _cld wvt$l_vt200_common_flags & vtc1_m_insert_mode);
	tsr_put_bit(&buff, _ld wvt$l_vt200_specific_flags & vts1_m_origin_mode);
	tsr_put_bit(&buff, _ld wvt$l_vt200_specific_flags & vts1_m_last_column);
	tsr_put_bit(&buff, _cld wvt$l_vt200_common_flags & vtc1_m_keypad_mode);
	tsr_put_bit(&buff, _cld wvt$l_vt200_common_flags & vtc1_m_cursor_key_mode);
	tsr_put_bit(&buff, _cld wvt$b_cursts & cs1_m_cs_enabled);
	tsr_put_bit(&buff, _cld wvt$l_vt200_flags & vt1_m_new_line_mode);
	tsr_put_bit(&buff, _ld wvt$l_vt200_specific_flags & vts1_m_auto_wrap_mode);
	tsr_put_bit(&buff, _cld wvt$l_vt200_common_flags & vtc1_m_screen_mode);
	tsr_put_bit(&buff, _cld wvt$l_vt200_common_flags & vtc1_m_auto_answerback);
		/* Transmission mode flag also should be saved.	*/
	tsr_put_bit(&buff, _cld wvt$l_vt200_common_flags & vtc1_m_c1_transmission_mode);
	tsr_put_bit(&buff, _cld wvt$l_flags & vt4_m_bell);
	tsr_put_bit(&buff, _cld wvt$l_flags & vt4_m_margin_bell);
	tsr_put_bit(&buff, _cld wvt$l_flags & vt4_m_underline_cursor);
	tsr_put_bit(&buff, _cld wvt$l_vt200_flags_2 & vt2_m_backarrow_mode);
	tsr_put_pad(&buff);
	tsr_put_byte(&buff, _ld wvt$b_ext_g_sets[0]);
	tsr_put_byte(&buff, _ld wvt$b_ext_g_sets[1]);
	tsr_put_byte(&buff, _ld wvt$b_ext_g_sets[2]);
	tsr_put_byte(&buff, _ld wvt$b_ext_g_sets[3]);
	tsr_put_word(&buff, _ld wvt$w_save_ext_rendition);
      	tsr_put_byte(&buff, _ld wvt$b_save_ext_g_sets[0]);
	tsr_put_byte(&buff, _ld wvt$b_save_ext_g_sets[1]);
	tsr_put_byte(&buff, _ld wvt$b_save_ext_g_sets[2]);
	tsr_put_byte(&buff, _ld wvt$b_save_ext_g_sets[3]);

	if ( _cld wvt$l_ext_flags & vte1_m_asian_common )
	    {
	    tsr_put_byte(&buff, _cld wvt$b_char_stack[0]);
	    tsr_put_byte(&buff, _cld wvt$b_char_stack[1]);
	    tsr_put_byte(&buff, _cld wvt$b_char_stack[2]);
	    tsr_put_byte(&buff, _cld wvt$b_char_stack_top);
	    tsr_put_bit(&buff, _cld wvt$l_ext_specific_flags & vte2_m_sixel_scroll_mode);
	    }
	if ( _cld wvt$l_ext_flags & vte1_m_tomcat )
	    {
	    tsr_put_bit(&buff, _cld wvt$l_ext_specific_flags & vte2_m_jisroman_mode);
	    tsr_put_bit(&buff, _cld wvt$l_ext_specific_flags & vte2_m_kanji_mode);
	    }
	if ( _cld wvt$l_ext_flags & vte1_m_dickcat )
	    {
	    tsr_put_bit(&buff, _cld wvt$l_ext_specific_flags & vte2_m_intermediate_char);
	    tsr_put_bit(&buff, _cld wvt$l_ext_specific_flags & vte2_m_ksroman_mode);
	    }
	if ( _cld wvt$l_ext_flags & vte1_m_fishcat )
	    {
	    tsr_put_bit(&buff, _cld wvt$l_ext_specific_flags & vte2_m_leading_code_mode);
	    }
	if ( _cld wvt$l_ext_flags & vte1_m_hebrew )
	    {
	    tsr_put_bit(&buff, _cld wvt$l_ext_specific_flags & vte2_m_rtl);
	    }
	tsr_put_pad(&buff);

	/* 8bit controlls mode	*/
    if (_cld wvt$l_vt200_common_flags & vtc1_m_c1_transmission_mode)
	tsr_put_checksum8(&buff);
    else
	tsr_put_checksum(&buff);
	tsr_transmit_buffer(ld, &buff);
}
                   
	/* 7bit controlls mode	*/
tsr_buff_init7(buff)
struct tsr_buffer_struct *buff;
{
	int i;
	buff->cp = &buff->buffer[5];
	buff->index = 5;
	buff->bit_index = 0;
	buff->buffer[0] = '\033';
	buff->buffer[1] = 'P';
	buff->buffer[2] = '1';
	buff->buffer[3] = '$';
	buff->buffer[4] = 's';
	for (i=5; i != TSR_BUF_SIZE; i++) buff->buffer[i] = (char)0x40;
}

tsr_buff_init(buff)
struct tsr_buffer_struct *buff;
{              
	int i;

	buff->cp = &buff->buffer[4];
	buff->index = 4;
	buff->bit_index = 0;
	buff->buffer[0] = '\220';
	buff->buffer[1] = '1';
	buff->buffer[2] = '$';
	buff->buffer[3] = 's';
	for (i=4; i < TSR_BUF_SIZE; i++) buff->buffer[i] = (char)0x40;
}

Boolean tsr_get_checksum(buff)
struct tsr_buffer_struct *buff;
{
	int i;
	unsigned char checksum, checksum1;

	checksum = checksum1 = 0;
	for (i=0; i < buff->index - 2; i++) checksum += buff->buffer[i];
	checksum1 = (buff->buffer[i] & 15);
	buff->buffer[i] = 0x40;
	checksum1 |= (buff->buffer[++i] & 15) << 4;
	buff->buffer[i] = 0x40;
	return(checksum - checksum1);
}

	/* 8bit controlls mode	*/
tsr_put_checksum8(buff)
struct tsr_buffer_struct *buff;
{
	int i;
	unsigned char checksum;
	checksum = 0;
	for (i=4; i < buff->index; i++) checksum += buff->buffer[i];
	tsr_put_byte(buff, checksum);
}

tsr_put_checksum(buff)
struct tsr_buffer_struct *buff;
{
	int i;
	unsigned char checksum;

	checksum = 0;
	for (i=5; i < buff->index; i++) checksum += buff->buffer[i];
	tsr_put_byte(buff, checksum);
}

tsr_put_long(buff, lng)
struct tsr_buffer_struct *buff;
unsigned long lng;
{
	if (buff->bit_index) tsr_put_pad(buff);
	if (buff->index > TSR_BUF_SIZE-8) return;
	*(buff->cp++) |= lng & 15;
	lng >>= 4;
	*(buff->cp++) |= lng & 15;
	lng >>=4;
	*(buff->cp++) |= lng & 15;
	lng >>=4;
	*(buff->cp++) |= lng & 15;
	lng >>=4;
	*(buff->cp++) |= lng & 15;
	lng >>=4;
	*(buff->cp++) |= lng & 15;
	lng >>=4;
	*(buff->cp++) |= lng & 15;
	lng >>=4;
	*(buff->cp++) |= lng & 15;
	buff->index += 8;
	return;
}

unsigned long
tsr_get_long(buff)
struct tsr_buffer_struct *buff;
{
	unsigned long ret;

	if (buff->bit_index) tsr_put_pad(buff);
	if (buff->index > TSR_BUF_SIZE-8) return(0);
	ret = *(buff->cp++) & 15;
	ret |= (*(buff->cp++) & 15) << 4;
	ret |= (*(buff->cp++) & 15) << 8;
	ret |= (*(buff->cp++) & 15) << 12;
	ret |= (*(buff->cp++) & 15) << 16;
	ret |= (*(buff->cp++) & 15) << 20;
	ret |= (*(buff->cp++) & 15) << 24;
	ret |= (*(buff->cp++) & 15) << 28;
	buff->index += 8;
	return(ret);
}

tsr_put_word(buff, wrd)
struct tsr_buffer_struct *buff;
unsigned short int wrd;
{
	if (buff->bit_index) tsr_put_pad(buff);
	if (buff->index > TSR_BUF_SIZE-4) return;
	*(buff->cp++) |= wrd & 15;
	wrd >>= 4;
	*(buff->cp++) |= wrd & 15;
	wrd >>= 4;
	*(buff->cp++) |= wrd & 15;
	wrd >>= 4;
	*(buff->cp++) |= wrd & 15;
	buff->index += 4;
	return;
}

unsigned short int
tsr_get_word(buff)
struct tsr_buffer_struct *buff;
{
	unsigned short int ret;

	if (buff->bit_index) tsr_put_pad(buff);
	if (buff->index > TSR_BUF_SIZE-4) return(0);
	ret = *(buff->cp++) & 15;
	ret |= (*(buff->cp++) & 15) << 4;
	ret |= (*(buff->cp++) & 15) << 8;
	ret |= (*(buff->cp++) & 15) << 12;
	buff->index += 4;
	return(ret);
}

tsr_put_byte(buff, byte)
struct tsr_buffer_struct *buff;
unsigned char byte;
{
	if (buff->bit_index) tsr_put_pad(buff);
	if (buff->index > TSR_BUF_SIZE-2) return;
	*(buff->cp++) |= byte & 15;
	byte >>= 4;
	*(buff->cp++) |= byte & 15;
	buff->index += 2;
	return;
}

unsigned char
tsr_get_byte(buff)
struct tsr_buffer_struct *buff;
{
	unsigned char ret;

	if (buff->bit_index) tsr_put_pad(buff);
	if (buff->index > TSR_BUF_SIZE-2) return(0);
	ret = *(buff->cp++) & 15;
	ret |= (*(buff->cp++) & 15) << 4;
	buff->index += 2;
	return(ret);
}

tsr_put_bit(buff, n)
struct tsr_buffer_struct *buff;
unsigned long n;
{
	if (buff->index > TSR_BUF_SIZE) return;
	if (n)
		*(buff->cp) |= (1 << buff->bit_index);
	buff->bit_index++;
	if (buff->bit_index >= 4) {
		buff->bit_index = 0;
    		buff->index++;
		buff->cp++;
	}
}

tsr_get_bit(buff)
struct tsr_buffer_struct *buff;
{
	int ret;

	if (buff->index > TSR_BUF_SIZE) return(0);
	ret = *(buff->cp) & (1 << buff->bit_index);
	buff->bit_index++;
	if (buff->bit_index >= 4) {
		buff->bit_index = 0;
    		buff->index++;
		buff->cp++;
	}
	return(ret);
}

tsr_get_pad(buff)
struct tsr_buffer_struct *buff;
{
	tsr_put_pad(buff);
}

tsr_put_pad(buff)
struct tsr_buffer_struct *buff;
{
	if (buff->index > TSR_BUF_SIZE) return;
	if (buff->bit_index) {           
		buff->bit_index = 0;
		buff->index++;
		buff->cp++;
	}   
}

tsr_transmit_buffer(ld, buff)
wvtp ld;
struct tsr_buffer_struct *buff;
{
	int i;

	/* 7bit controlls mode	*/
  if ( !(_cld wvt$l_vt200_common_flags & vtc1_m_c1_transmission_mode)) {
	buff->buffer[buff->index++] = '\033';
	buff->buffer[buff->index++] = '\\';
	}
    else
	buff->buffer[buff->index++] = '\234';
	buff->buffer[buff->index++] = '\000';
	WVT$TERMINAL_TRANSMIT(ld, buff->buffer);
}

/************************************************/
xdecctr(ld)		/* color table report   */
/************************************************/

wvtp ld;

{
    DECtermWidget w = ld_to_w( ld );
    int num_colors = 1 << w->common.bitPlanes;
    int i, coordinate, cx, cy, cz;
    char *buf;
    char *bptr = buf;

    if ( _cld wvt$b_parmcnt == 1 || _cld wvt$l_parms[1] == 0 )
	coordinate = 1;		/* HLS is the default */
    else
	coordinate = _cld wvt$l_parms[1];
    if ( coordinate > 2 )
	return;			/* illegal universal coordinate system */

    buf = XtMalloc( num_colors * 18 + 6 );
    bptr = buf;
	/* 7bit controlls mode	*/
  if ( !(_cld wvt$l_vt200_common_flags & vtc1_m_c1_transmission_mode) )
    sprintf( bptr, "\033P2$s" );
  else
    sprintf( bptr, "\2202$s" );
    bptr += strlen( bptr );
    for ( i = 0; i < num_colors; i++ )
	{
	if ( coordinate == 1 )
	    {  /* HLS */
	    rgbhls( w->common.color_map[i].red, w->common.color_map[i].green,
		    w->common.color_map[i].blue, &cx, &cy, &cz );
	    }
	else
	    {  /* RGB */
	    cx = TRIM_RGB( w->common.color_map[i].red);
	    cy = TRIM_RGB( w->common.color_map[i].green);
	    cz = TRIM_RGB( w->common.color_map[i].blue);
	    }
	sprintf( bptr, "%d;%d;%d;%d;%d/", i, coordinate, cx, cy, cz );
	bptr += strlen( bptr );
	}
	/* 7bit controlls mode	*/
  if ( !(_cld wvt$l_vt200_common_flags & vtc1_m_c1_transmission_mode) )
    sprintf( bptr, "\033\\" );
  else
    sprintf( bptr, "\234" );
    WVT$TERMINAL_TRANSMIT( ld, buf );
    XtFree( buf );
}

/*
 * rgbhls
 *
 * Convert an X-windows style (0 to 65535) RGB color to HLS.  This parallels
 * the routine hlsrgb in reg_color.c, but I've put rgbhls in this module
 * to separate ANSI from ReGIS.
 *
 * The key equations from hlsrgb are:
 *
 *	if ( lightness <= 50 )
 *	    prime = lightness * ( 100 + saturation ) / 100;
 *	else
 *	    prime = lightness + saturation - ( lightness * saturation / 100 );
 *
 *	comp = 2 * lightness - prime;
 *
 *	ramp = ( prime - comp ) * ( hue % 60 ) / 60;
 */

rgbhls( red, green, blue, hue, lightness, saturation )
    int red,		/* input: 0 <= red <= MAX_RGB */
	green,		/* input: 0 <= green <= MAX_RGB */
	blue;		/* input: 0 <= blue <= MAX_RGB */
    int *hue,		/* output: 0 <= hue < 360; blue at 0 */
	*lightness,	/* output: 0 <= lightness <= 100 */
	*saturation;	/* output: 0 <= saturation <= 100 */
{
    int prime;		/* value of primary color */
    int comp;		/* value of complementary color */
    int ramp;		/* value partially between comp and prime */
    int sixth;		/* hue / 60, i.e. which of six hue ranges we're in */

#define MAX_RGB 65535

/*
 * convert RGB to 0..100
 */

    red = TRIM_RGB( red );
    green = TRIM_RGB( green );
    blue = TRIM_RGB( blue );

/*
 * first decide which sixth of the color wheel we're in, based on which
 * is the biggest between red, green and blue
 */

    if ( blue > green )
	if ( blue > red )
	    if ( red > green )
		sixth = 0;	/* blue to magenta */
	    else
		sixth = 5;	/* cyan to blue */
	else
	    sixth = 1;		/* magenta to red */
    else
	if ( green > red )
	    if ( red > blue )
		sixth = 3;	/* yellow to green */
	    else
		sixth = 4;	/* green to cyan */
	else
	    sixth = 2;		/* red to yellow */

/*
 * now figure out the values of prime, comp and ramp (reverse the logic
 * in hlsrgb)
 */

    switch ( sixth )
	{
	case 0:			/* hue 0 to 59 = blue to magenta */
	    prime = blue;
	    comp = green;
	    ramp = red - green;
	    break;
	case 1:			/* hue 60 to 119 = magenta to red */
	    prime = red;
	    comp = green;
	    ramp = red - blue;
	    break;
	case 2:			/* hue 120 to 179 = red to yellow */
	    prime = red;
	    comp = blue;
	    ramp = green - blue;
	    break;
	case 3:			/* hue 180 to 239 = yellow to green */
	    prime = green;
	    comp = blue;
	    ramp = green - red;
	    break;
	case 4:			/* hue 240 to 299 = green to cyan */
	    prime = green;
	    comp = red;
	    ramp = blue - red;
	    break;
	case 5:			/* hue 300 to 359 = cyan to blue */
	    prime = blue;
	    comp = red;
	    ramp = blue - green;
	    break;
	}
/*
 * Calculate the hue.  Since ramp = ( prime - comp ) * ( hue % 60 ) / 60,
 * we know that ( hue % 60 ) = ( ramp * 60 ) / ( prime - comp).  First
 * check for  division by 0, i.e. prime == comp, which means that the
 * saturation is zero.  If that happens then lightness == prime == comp.
 */
    if ( prime == comp )
	{
	*hue = 0;	/* nominal value - it's undefined */
	*lightness = prime;
	*saturation = 0;
	return;
	}
    *hue = sixth * 60 + ( ramp * 60 ) / ( prime - comp );
/*
 * Calculate the lightness.  Since comp = 2 * lightness - prime, we know
 * that lightness = ( prime + comp ) / 2.
 */
    *lightness = ( prime + comp ) / 2;
/*
 * Finally, solve for the saturation (see equations above).  We know that
 * lightness > 0, since otherwise prime == comp == 0.  We also know that
 * lightness < 100, since otherwise prime == comp == 100.
 */
    if ( *lightness == 0 || *lightness == 100 )
    {
	/* avoid divided by zero */
	*saturation = 0;
	*hue = 0;
    }
    else
    {
	if ( *lightness <= 50 )
	    *saturation = ( prime * 100 ) / *lightness - 100;
    	else
	    *saturation = ( ( prime - *lightness ) * 100 ) / ( 100 - *lightness );
    }

    if ( *saturation > 100 )
	*saturation = 100;	/* appease the gods of round-off error */
}

void
xdecrsps_cir( ld, code )
wvtp ld;
unsigned char code;
{
int event, start, final;
/* These variables are used to implement designation in DECRSPS.	*/
unsigned char final_p;
int g_set;

	/* Parse the character stream and ignore until ST */

	parse_ansi(ld, code, &event, &start, &final);
	_cld wvt$b_last_event = event;

	if (event == R_GRAPHIC)
	{
	   if (_cld wvt$dcs_count >= 0 && _cld wvt$dcs_count <= 2)
		{
		  	if (code >= '0' && code <= '9') {
				_cld wvt$l_parms[0] *= 10;
				_cld wvt$l_parms[0] += code - '0';
			} else if (code == ';') {
				switch (_cld wvt$dcs_count) {
					case 0:
						_ld wvt$l_actv_line = _cld wvt$l_parms[0];
						break;
					case 1:
						_ld wvt$l_actv_column = _cld wvt$l_parms[0];
						break;
					case 2:
						break;
				}
			    _cld wvt$dcs_count++;
				_cld wvt$l_parms[0] = 0;
			}
			return;
		}       
		if (_cld wvt$dcs_count >= 3 && _cld wvt$dcs_count <= 9)
		{                   
			if (code != ';') {
				_cld wvt$dcs_final[_cld wvt$dcs_final_index++] = code;
		    } else {
				switch (_cld wvt$dcs_count) {
					case 3:
					   	if (_cld wvt$dcs_final[0] & 8)
							_ld wvt$w_actv_rendition |= csa_M_REVERSE;
						else
							_ld wvt$w_actv_rendition &= ~csa_M_REVERSE;
						if (_cld wvt$dcs_final[0] & 4)
							_ld wvt$w_actv_rendition |= csa_M_BLINK;
						else
							_ld wvt$w_actv_rendition &= ~csa_M_BLINK;
						if (_cld wvt$dcs_final[0] & 2)
							_ld wvt$w_actv_rendition |= csa_M_UNDERLINE;
						else
							_ld wvt$w_actv_rendition &= ~csa_M_UNDERLINE;
						if (_cld wvt$dcs_final[0] & 1)
							_ld wvt$w_actv_rendition |= csa_M_BOLD;
						else
							_ld wvt$w_actv_rendition &= ~csa_M_BOLD;
						break;
					case 4:
	/* csa_M_SELECTIVE_ERASE = off ... protect	*/
	/* csa_M_SELECTIVE_ERASE = on  ... erase	*/
						if (_cld wvt$dcs_final[0] & 1)
							_ld wvt$w_actv_rendition &= ~csa_M_SELECTIVE_ERASE; 
						else		/*002+*/
							_ld wvt$w_actv_rendition |= csa_M_SELECTIVE_ERASE;
						break;
					case 5:
/* If this bit is set, we must autowrap before next character.	*/
						if (_cld wvt$dcs_final[0] & 8){
							_ld wvt$l_vt200_specific_flags |= vts1_m_auto_wrap_mode;
							_ld wvt$l_vt200_specific_flags |= vts1_m_last_column;
                                                }
						else {
							_ld wvt$l_vt200_specific_flags &= ~vts1_m_auto_wrap_mode;
							_ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;
                                                }
						if (_cld wvt$dcs_final[0] & 1)
							_ld wvt$l_vt200_specific_flags |= vts1_m_origin_mode;
						else
							_ld wvt$l_vt200_specific_flags &= ~vts1_m_origin_mode;
						if (_cld wvt$dcs_final[0] & 4)
							_ld wvt$b_single_shift = SS_3;
						if (_cld wvt$dcs_final[0] & 2)
							_ld wvt$b_single_shift = SS_2;
						break;
					case 6:
			   			_ld wvt$b_gl = _cld wvt$dcs_final[0] - '0';
						break;
					case 7:
						_ld wvt$b_gr = _cld wvt$dcs_final[0] - '0';
						break;
					case 8:
/* For designation, we copy this parameter to char_set_size.	*/
/* wvt$l_parms is used temporarily to save character set size.	*/
				_cld wvt$l_parms[0] = _cld wvt$dcs_final[0];
						break;
				}
				_cld wvt$dcs_count++;
				_cld wvt$dcs_final_index = 0;
			}
		}
	   return;
	}

	if ((event == R_CONTROL) &&
	    _cld wvt$r_com_state.data[final] == C1_ST)
	{
		if (_cld wvt$dcs_count == 9) /* sequence ended properly */
		{
			/* fake four calls to xscs */
/********************************************************/
/* Sdesig: This has been not inplemented.		*/
/*							*/
/*                - Note -				*/
/* The conformance level should be checked before	*/
/* starting DECRSPS sequence.				*/
			final_p = 0;
			for( g_set = 0; g_set < 4; g_set++ )
			{
/********************************************************/
/* Clear User Perferred Supplemental flag.		*/
			    _ld wvt$b_ups &= ~( 1 << g_set );
                            if( final_p >= _cld wvt$dcs_final_index )
                            {
                               break;
                            }
                            switch( _cld wvt$dcs_final[final_p] )
                            {
                                case 'B': if(!( _cld wvt$l_parms[0] & ( 0x01 << g_set )))
						_ld wvt$b_ext_g_sets[g_set] = STANDARD_SET ,
						_ld wvt$b_g_sets[g_set] = ASCII ;
                                          break ;
                                case '%': if(!( _cld wvt$l_parms[0] & ( 0x01 << g_set ))){
                                              final_p++ ;
                                              if ( final_p < _cld wvt$dcs_final_index ) {
						if ( _cld wvt$dcs_final[final_p] == '5' )
						_ld wvt$b_ext_g_sets[g_set] = STANDARD_SET,
						_ld wvt$b_g_sets[g_set] = SUPPLEMENTAL;
						else if ( _cld wvt$dcs_final[final_p] == '?' )
						_ld wvt$b_ext_g_sets[g_set] = ONE_BYTE_SET,
						_ld wvt$b_g_sets[g_set] = KS_ROMAN;
						else if ( _cld wvt$dcs_final[final_p] == '0' )
						_ld wvt$b_ext_g_sets[g_set] = STANDARD_SET,
						_ld wvt$b_g_sets[g_set] = TURKISH_SUPPLEMENTAL;
					      }
                                          }
                                          break ;
                                case '<': if(!( _cld wvt$l_parms[0] & ( 0x01 << g_set )))
						_ld wvt$b_ext_g_sets[g_set] = STANDARD_SET ,
						_ld wvt$b_g_sets[g_set] = _cld wvt$b_user_preference_set;
	/* Set User Preferred Supplemental flag		*/
						_ld wvt$b_ups |= ( 1<<g_set);

                                          break;
                                case '0': if(!( _cld wvt$l_parms[0] & ( 0x01 << g_set )))
						_ld wvt$b_ext_g_sets[g_set] = STANDARD_SET ,
						_ld wvt$b_g_sets[g_set] = LINE_DRAWING;
                                          break;
                                case '>': if(!( _cld wvt$l_parms[0] & ( 0x01 << g_set )))
						_ld wvt$b_ext_g_sets[g_set] = STANDARD_SET ,
						_ld wvt$b_g_sets[g_set] = TECHNICAL;
                                          break;
                                case 'H': if(!( _cld wvt$l_parms[0] & ( 0x01 << g_set )))
						_ld wvt$b_ext_g_sets[g_set] = ONE_BYTE_SET ,
						_ld wvt$b_g_sets[g_set] = ISO_LATIN_8;
                                          break;
                                case '"': if(!( _cld wvt$l_parms[0] & ( 0x01 << g_set ))){
                                              final_p++ ;
                                              if ( final_p < _cld wvt$dcs_final_index &&
						_cld wvt$dcs_final[final_p] == '4' )
						_ld wvt$b_ext_g_sets[g_set] = ONE_BYTE_SET,
						_ld wvt$b_g_sets[g_set] = HEB_SUPPLEMENTAL;
                                              else if ( final_p < _cld wvt$dcs_final_index &&
						_cld wvt$dcs_final[final_p] == '?' )
						_ld wvt$b_ext_g_sets[g_set] = STANDARD_SET,
						_ld wvt$b_g_sets[g_set] = GREEK_SUPPLEMENTAL;
                                          }
                                          break ;
                                case 'F': if(!( _cld wvt$l_parms[0] & ( 0x01 << g_set )))
						_ld wvt$b_ext_g_sets[g_set] = STANDARD_SET ,
						_ld wvt$b_g_sets[g_set] = ISO_LATIN_7;
                                          break;

                                case 'M': if(!( _cld wvt$l_parms[0] & ( 0x01 << g_set )))
						_ld wvt$b_ext_g_sets[g_set] = STANDARD_SET ,
						_ld wvt$b_g_sets[g_set] = ISO_LATIN_5;
                                          break;

                                case 'J': if(!( _cld wvt$l_parms[0] & ( 0x01 << g_set )))
						_ld wvt$b_ext_g_sets[g_set] = ONE_BYTE_SET ,
						_ld wvt$b_g_sets[g_set] = JIS_ROMAN;
                                          break;
                                case 'I': if(!( _cld wvt$l_parms[0] & ( 0x01 << g_set )))
						_ld wvt$b_ext_g_sets[g_set] = ONE_BYTE_SET ,
						_ld wvt$b_g_sets[g_set] = JIS_KATAKANA;
                                          break;
                                case 'A': if( _cld wvt$l_parms[0] & ( 0x01 << g_set ))
						_ld wvt$b_ext_g_sets[g_set] = STANDARD_SET ,
						_ld wvt$b_g_sets[g_set] = ISO_LATIN_1;
                                          break;
                                case '$': if(!( _cld wvt$l_parms[0] & ( 0x01 << g_set ))){
                                              final_p++;
                                              if ( final_p < _cld wvt$dcs_final_index )
						switch ( _cld wvt$dcs_final[final_p] ) {
						case '1':
						case '3':
						_ld wvt$b_ext_g_sets[g_set] = TWO_BYTE_SET;
						_ld wvt$b_g_sets[g_set] = DEC_KANJI; break;
						case '2':
						case 'A':
						_ld wvt$b_ext_g_sets[g_set] = TWO_BYTE_SET;
						_ld wvt$b_g_sets[g_set] = DEC_HANZI; break;
						case '4':
						case 'C':
						_ld wvt$b_ext_g_sets[g_set] = TWO_BYTE_SET;
						_ld wvt$b_g_sets[g_set] = DEC_HANGUL; break;
						case '5':
						_ld wvt$b_ext_g_sets[g_set] = FOUR_BYTE_SET;
						_ld wvt$b_g_sets[g_set] = DEC_HANYU; break;
						default:	break;
						}
                                          }
                                          break;
                                default:  break;
                            }
                            final_p++;
                        }
		}
		_cld wvt$b_in_dcs = FALSE;
		return;
	}
}

void
xdecrsps_tabsr( ld, code )
wvtp ld;
unsigned char code;
{
int event, start, final;

	/* Parse the character stream and ignore until ST */

	parse_ansi(ld, code, &event, &start, &final);
	_cld wvt$b_last_event = event;

	if (event == R_GRAPHIC)
	{
		  	if (code >= '0' && code <= '9') {
				_cld wvt$l_parms[0] *= 10;
				_cld wvt$l_parms[0] += code - '0';
				_cld wvt$dcs_count++;
			} else if (code == '/') {
				_cld wvt$b_tab_stops[_cld wvt$l_parms[0]] = 1;
				_cld wvt$l_parms[0] = 0;
				_cld wvt$dcs_count = 0;
			}
		return; 
	}

	if ((event == R_CONTROL) &&
	    _cld wvt$r_com_state.data[final] == C1_ST)
	{
		if (_cld wvt$dcs_count != 0)  /* there is one more tab to set */
			_cld wvt$b_tab_stops[_cld wvt$l_parms[0]] = 1;
		_cld wvt$b_in_dcs = FALSE;
    		return;
	}
}

void
xdecrsts( ld, code )
wvtp ld;
unsigned char code;
{
int event, start, final, i, rows, columns, actv_status_display;

	/* Parse the character stream and ignore until ST */

	parse_ansi(ld, code, &event, &start, &final);
	_cld wvt$b_last_event = event;

	if (event == R_GRAPHIC)
	 {
	   *(_cld wvt$dcs_tsr_buff.cp)++ = code;
	   _cld wvt$dcs_tsr_buff.index++;
	   return;
	 }
	if ((event == R_CONTROL) &&
	    _cld wvt$r_com_state.data[final] == C1_ST)
	{
	   if (tsr_get_checksum(&_cld wvt$dcs_tsr_buff) != 0) {
		   _cld wvt$b_in_dcs = FALSE;
		   return;
	   }
	   _cld wvt$dcs_tsr_buff.end_index = _cld wvt$dcs_tsr_buff.index;
	   _cld wvt$dcs_tsr_buff.end_bit_index = _cld wvt$dcs_tsr_buff.bit_index;
	   _cld wvt$dcs_tsr_buff.index = 0;
	   _cld wvt$dcs_tsr_buff.bit_index = 0;
	   _cld wvt$dcs_tsr_buff.cp = _cld wvt$dcs_tsr_buff.buffer;
	   rows = tsr_get_byte(&_cld wvt$dcs_tsr_buff);
	   columns = tsr_get_byte(&_cld wvt$dcs_tsr_buff);
	   if (rows && columns) WVT$SET_TERMINAL_SIZE(ld, columns, rows, 0);
	   _ld wvt$l_actv_line = tsr_get_byte(&_cld wvt$dcs_tsr_buff);
	   _ld wvt$l_actv_column = tsr_get_byte(&_cld wvt$dcs_tsr_buff);
/*Active rendition and conformance level also should be restored.	*/
	   _ld wvt$w_actv_rendition = tsr_get_word(&_cld wvt$dcs_tsr_buff);
	   _ld wvt$w_actv_ext_rendition = tsr_get_word(&_cld wvt$dcs_tsr_buff);
	   _cld wvt$b_conformance_level = tsr_get_byte(&_cld wvt$dcs_tsr_buff);
	   _ld wvt$b_g_sets[0] = tsr_get_byte(&_cld wvt$dcs_tsr_buff);
	   _ld wvt$b_g_sets[1] = tsr_get_byte(&_cld wvt$dcs_tsr_buff);
	   _ld wvt$b_g_sets[2] = tsr_get_byte(&_cld wvt$dcs_tsr_buff);
	   _ld wvt$b_g_sets[3] = tsr_get_byte(&_cld wvt$dcs_tsr_buff);
/* User Preferred Supplemental flag also should be restored.	*/
	   _ld wvt$b_ups = tsr_get_byte(&_cld wvt$dcs_tsr_buff);
	   _ld wvt$b_gl = tsr_get_byte(&_cld wvt$dcs_tsr_buff);
	   _ld wvt$b_gr = tsr_get_byte(&_cld wvt$dcs_tsr_buff);
	   _ld wvt$b_single_shift = tsr_get_byte(&_cld wvt$dcs_tsr_buff);
	   _ld wvt$l_save_line = tsr_get_byte(&_cld wvt$dcs_tsr_buff);
	   _ld wvt$l_save_column = tsr_get_byte (&_cld wvt$dcs_tsr_buff);
	   _ld wvt$w_save_rendition = tsr_get_word(&_cld wvt$dcs_tsr_buff);
	   _ld wvt$l_save_vt200_flags = tsr_get_long(&_cld wvt$dcs_tsr_buff);
	   _ld wvt$b_save_g_sets[0] = tsr_get_byte(&_cld wvt$dcs_tsr_buff);
	   _ld wvt$b_save_g_sets[1] = tsr_get_byte(&_cld wvt$dcs_tsr_buff);
	   _ld wvt$b_save_g_sets[2] = tsr_get_byte(&_cld wvt$dcs_tsr_buff);
	   _ld wvt$b_save_g_sets[3] = tsr_get_byte(&_cld wvt$dcs_tsr_buff);
	   _ld wvt$b_sav_gl = tsr_get_byte(&_cld wvt$dcs_tsr_buff);
	   _ld wvt$b_sav_gr = tsr_get_byte(&_cld wvt$dcs_tsr_buff);
	   for (i=1; i <= MAX_COLUMN; i++)
		_cld wvt$b_tab_stops[i] = tsr_get_bit(&_cld wvt$dcs_tsr_buff);
	   tsr_get_pad(&_cld wvt$dcs_tsr_buff);
	   _ld wvt$l_top_margin = tsr_get_byte(&_cld wvt$dcs_tsr_buff);
	   _ld wvt$l_bottom_margin = tsr_get_byte(&_cld wvt$dcs_tsr_buff);
	   if (tsr_get_bit(&_cld wvt$dcs_tsr_buff))
		_cld wvt$l_vt200_flags |= vt1_m_nrc_mode;
	   else
		_cld wvt$l_vt200_flags &= ~vt1_m_nrc_mode;
	   if (tsr_get_bit(&_cld wvt$dcs_tsr_buff))
		WVT$ENABLE_STATUS_DISPLAY(ld);
	   else
		WVT$DISABLE_STATUS_DISPLAY(ld);
	   actv_status_display = tsr_get_bit(&_cld wvt$dcs_tsr_buff);
	   if (tsr_get_bit(&_cld wvt$dcs_tsr_buff))
		_cld wvt$l_vt200_common_flags |= vtc1_m_insert_mode;
	   else
		_cld wvt$l_vt200_common_flags &= ~vtc1_m_insert_mode;
	   if (tsr_get_bit(&_cld wvt$dcs_tsr_buff))
		_ld wvt$l_vt200_specific_flags |= vts1_m_origin_mode;
	   else
		_ld wvt$l_vt200_specific_flags &= ~vts1_m_origin_mode;
	   if (tsr_get_bit(&_cld wvt$dcs_tsr_buff))
		_ld wvt$l_vt200_specific_flags |= vts1_m_last_column;
	   else
		_ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;
	   if (tsr_get_bit(&_cld wvt$dcs_tsr_buff))
		_cld wvt$l_vt200_common_flags |= vtc1_m_keypad_mode;
	   else
		_cld wvt$l_vt200_common_flags &= ~vtc1_m_keypad_mode;
           if (tsr_get_bit(&_cld wvt$dcs_tsr_buff))
		_cld wvt$l_vt200_common_flags |= vtc1_m_cursor_key_mode;
	   else
		_cld wvt$l_vt200_common_flags &= ~vtc1_m_cursor_key_mode;
	   if (tsr_get_bit(&_cld wvt$dcs_tsr_buff)) {
		_cld wvt$b_cursts |= cs1_m_cs_enabled;
		WVT$ENABLE_CURSOR(ld);
	   } else {
		_cld wvt$b_cursts &= ~cs1_m_cs_enabled;
		WVT$DISABLE_CURSOR(ld);
	   }
           if (tsr_get_bit(&_cld wvt$dcs_tsr_buff))
		_cld wvt$l_vt200_flags |= vt1_m_new_line_mode;
	   else
		_cld wvt$l_vt200_flags &= ~vt1_m_new_line_mode;
	   if (tsr_get_bit(&_cld wvt$dcs_tsr_buff))
		_ld wvt$l_vt200_specific_flags |= vts1_m_auto_wrap_mode;
	   else
		_ld wvt$l_vt200_specific_flags &= ~vts1_m_auto_wrap_mode;
	   if (tsr_get_bit(&_cld wvt$dcs_tsr_buff)) {
		if ((_cld wvt$l_vt200_common_flags & vtc1_m_screen_mode) == 0) {
			WVT$REVERSE_VIDEO(ld);
		}
	   } else {
		if ((_cld wvt$l_vt200_common_flags & vtc1_m_screen_mode) != 0) {
			WVT$REVERSE_VIDEO(ld);
		}
	   }
	   if (tsr_get_bit(&_cld wvt$dcs_tsr_buff))
		_cld wvt$l_vt200_common_flags |= vtc1_m_auto_answerback;
	   else
		_cld wvt$l_vt200_common_flags &= ~vtc1_m_auto_answerback;
/* Transmission mode flag also should be restored.	*/
     if (tsr_get_bit(&_cld wvt$dcs_tsr_buff))
	_cld wvt$l_vt200_common_flags |= vtc1_m_c1_transmission_mode;
     else
	_cld wvt$l_vt200_common_flags &= ~vtc1_m_c1_transmission_mode;
	   if (tsr_get_bit(&_cld wvt$dcs_tsr_buff))                   
		_cld wvt$l_flags |= vt4_m_bell;
	   else
		_cld wvt$l_flags &= ~vt4_m_bell;
	   if (tsr_get_bit(&_cld wvt$dcs_tsr_buff))
		_cld wvt$l_flags |= vt4_m_margin_bell;
	   else
		_cld wvt$l_flags &= ~vt4_m_margin_bell;
	   if (tsr_get_bit(&_cld wvt$dcs_tsr_buff))
		_cld wvt$l_flags |= vt4_m_underline_cursor;
	   else
		_cld wvt$l_flags &= ~vt4_m_underline_cursor;
	   if (tsr_get_bit(&_cld wvt$dcs_tsr_buff))
		_cld wvt$l_vt200_flags_2 |= vt2_m_backarrow_mode;
	   else
		_cld wvt$l_vt200_flags_2 &= ~vt2_m_backarrow_mode;
/* Should be use tsr_get_pad() after used tsr_get_bit().	*/
	   tsr_get_pad(&_cld wvt$dcs_tsr_buff);

	   _ld wvt$b_ext_g_sets[0] = tsr_get_byte(&_cld wvt$dcs_tsr_buff);
	   _ld wvt$b_ext_g_sets[1] = tsr_get_byte(&_cld wvt$dcs_tsr_buff);
	   _ld wvt$b_ext_g_sets[2] = tsr_get_byte(&_cld wvt$dcs_tsr_buff);
	   _ld wvt$b_ext_g_sets[3] = tsr_get_byte(&_cld wvt$dcs_tsr_buff);
	   _ld wvt$w_save_ext_rendition = tsr_get_word(&_cld wvt$dcs_tsr_buff);
	   _ld wvt$b_save_ext_g_sets[0] = tsr_get_byte(&_cld wvt$dcs_tsr_buff);
	   _ld wvt$b_save_ext_g_sets[1] = tsr_get_byte(&_cld wvt$dcs_tsr_buff);
	   _ld wvt$b_save_ext_g_sets[2] = tsr_get_byte(&_cld wvt$dcs_tsr_buff);
	   _ld wvt$b_save_ext_g_sets[3] = tsr_get_byte(&_cld wvt$dcs_tsr_buff);

	if ( _cld wvt$l_ext_flags & vte1_m_asian_common )
	    {
	    _cld wvt$b_char_stack[0] = tsr_get_byte(&_cld wvt$dcs_tsr_buff);
	    _cld wvt$b_char_stack[1] = tsr_get_byte(&_cld wvt$dcs_tsr_buff);
	    _cld wvt$b_char_stack[2] = tsr_get_byte(&_cld wvt$dcs_tsr_buff);
	    _cld wvt$b_char_stack_top = tsr_get_byte(&_cld wvt$dcs_tsr_buff);
	    if (tsr_get_bit(&_cld wvt$dcs_tsr_buff))
		_cld wvt$l_ext_specific_flags |= vte2_m_sixel_scroll_mode;
	    else
		_cld wvt$l_ext_specific_flags &= ~vte2_m_sixel_scroll_mode;
	    }
	if ( _cld wvt$l_ext_flags & vte1_m_tomcat )
	    {
	    if (tsr_get_bit(&_cld wvt$dcs_tsr_buff))
		_cld wvt$l_ext_specific_flags |= vte2_m_jisroman_mode;
	    else
		_cld wvt$l_ext_specific_flags &= ~vte2_m_jisroman_mode;
	    if (tsr_get_bit(&_cld wvt$dcs_tsr_buff))
		_cld wvt$l_ext_specific_flags |= vte2_m_kanji_mode;
	    else
		_cld wvt$l_ext_specific_flags &= ~vte2_m_kanji_mode;
	    }
	if ( _cld wvt$l_ext_flags & vte1_m_dickcat )
	    {
	    if (tsr_get_bit(&_cld wvt$dcs_tsr_buff))
		_cld wvt$l_ext_specific_flags |= vte2_m_intermediate_char;
	    else
		_cld wvt$l_ext_specific_flags &= ~vte2_m_intermediate_char;
	    if (tsr_get_bit(&_cld wvt$dcs_tsr_buff))
		_cld wvt$l_ext_specific_flags |= vte2_m_ksroman_mode;
	    else
		_cld wvt$l_ext_specific_flags &= ~vte2_m_ksroman_mode;
	    }
	if ( _cld wvt$l_ext_flags & vte1_m_fishcat )
	    {
	    if (tsr_get_bit(&_cld wvt$dcs_tsr_buff))
		_cld wvt$l_ext_specific_flags |= vte2_m_leading_code_mode;
	    else
		_cld wvt$l_ext_specific_flags &= ~vte2_m_leading_code_mode;
	    }
	if ( _cld wvt$l_ext_flags & vte1_m_hebrew )
	    {
	    if (tsr_get_bit(&_cld wvt$dcs_tsr_buff))
		_cld wvt$l_ext_specific_flags |= vte2_m_rtl;
	    else
		_cld wvt$l_ext_specific_flags &= ~vte2_m_rtl;
	    }
	   _cld wvt$b_in_dcs = FALSE;
	   WVT$SET_MISC_MODES(ld);
	   WVT$SET_KB_ATTRIBUTES(ld);
	   if (actv_status_display)
		WVT$STATUS_DISPLAY(ld);
	   else
		WVT$MAIN_DISPLAY(ld);
	}
}

void
xdecrsts_ctr(ld, code)
wvtp ld;
unsigned char code;
{
    int event, start, final, drop, value;

/* Parse the character steam to look for ST */

    parse_ansi(ld, code, &event, &start, &final );
    _cld wvt$b_last_event = event;

    switch ( event )
	{
	case R_GRAPHIC:
	case R_CONTROL:
	    code = _cld wvt$r_com_state.data[final];
	    break;
	case R_CONTINUE:
	default:
	    return;
	}

    if ( event == R_CONTROL )
	{
	if ( ! ( 8 <= code && code <= 13 ) )
	    _cld wvt$b_in_dcs = FALSE;
	return;
	}
	
    if ( code == ' ' || '\010' <= code && code <= '\015' )
	return;	/* ignore whitespace */

    drop = FALSE;
    while ( ! drop )
	{
	drop = TRUE;
	switch ( _cld wvt$b_ctr_state )
	    {
	    case CTR_STATE_INIT:
		if ( ! isdigit( code ) )
		    _cld wvt$b_ctr_state = CTR_STATE_IGNORE;
		else
		    {	/* parse color number */
		    CTR_PARSE_NUMBER( CTR_STATE_COLOR );
		    drop = FALSE;	/* re-parse digit */
		    }
		break;
	    case CTR_STATE_COLOR:
		if ( code != ';' )
		    _cld wvt$b_ctr_state = CTR_STATE_IGNORE;
		else
		    {
		    _cld wvt$w_ctr_color = _cld wvt$w_ctr_number;
		    CTR_PARSE_NUMBER( CTR_STATE_COORDINATE );
		    }
		break;
	    case CTR_STATE_COORDINATE:
		if ( code != ';' )
		    _cld wvt$b_ctr_state = CTR_STATE_IGNORE;
		else
		    {
		    _cld wvt$w_ctr_coordinate = _cld wvt$w_ctr_number;
		    CTR_PARSE_NUMBER( CTR_STATE_CX );
		    }
		break;
	    case CTR_STATE_CX:	/* hue or red */
		if ( code != ';' )
		    _cld wvt$b_ctr_state = CTR_STATE_IGNORE;
		else
		    {
		    _cld wvt$w_ctr_cx = _cld wvt$w_ctr_number;
		    CTR_PARSE_NUMBER( CTR_STATE_CY );
		    }
		break;
	    case CTR_STATE_CY:	/* lightness or green */
		if ( code != ';' )
		    _cld wvt$b_ctr_state = CTR_STATE_IGNORE;
		else
		    {
		    _cld wvt$w_ctr_cy = _cld wvt$w_ctr_number;
		    CTR_PARSE_NUMBER( CTR_STATE_CZ );
		    }
		break;
	    case CTR_STATE_CZ:	/* saturation or blue */
		if ( code != '/' )
		    _cld wvt$b_ctr_state = CTR_STATE_IGNORE;
		else
		    {
		    _cld wvt$w_ctr_cz = _cld wvt$w_ctr_number;
		    ctr_set_color( ld );
		    CTR_PARSE_NUMBER( CTR_STATE_COLOR );
		    }
		break;
	    case CTR_STATE_NUMBER:
		if ( ! isdigit( code ) )
		    _cld wvt$b_ctr_state = CTR_STATE_IGNORE;
		_cld wvt$w_ctr_number = 0;
		_cld wvt$b_ctr_state = CTR_STATE_NUMBER_1;
		drop = FALSE;
		break;
	    case CTR_STATE_NUMBER_1:
		if ( ! isdigit( code ) )
		    {
		    _cld wvt$b_ctr_state = _cld wvt$b_ctr_return_state;
		    drop = FALSE;
		    }
		else
		    {
		    value = _cld wvt$w_ctr_number * 10;
		    value += code - '0';
		    if ( value >= 65536 )
			_cld wvt$b_ctr_state = CTR_STATE_IGNORE;
		    else
			_cld wvt$w_ctr_number = value;
		    }
		break;
	    case CTR_STATE_IGNORE:
	    default:
		break;
	    }
	}
}

ctr_set_color( ld )
    wvtp ld;
{
    DECtermWidget w = ld_to_w( ld );
    int num_colors = ( 1 << w->common.bitPlanes );
    int red, green, blue, mono;

    if ( _cld wvt$w_ctr_color >= num_colors )
	return;	/* invalid color */

    regis_set_widget( ld );
    if ( ! w->common.color_map_allocated )
	allocate_color_map( w );

    if ( _cld wvt$w_ctr_coordinate == 1 )
	{	/* HLS */
	set_hls_color( _cld wvt$w_ctr_color, _cld wvt$w_ctr_cx,
		_cld wvt$w_ctr_cy, _cld wvt$w_ctr_cz );
	}
    else if ( _cld wvt$w_ctr_coordinate == 2 )
	{	/* RGB */
	red = normalize_rgb( _cld wvt$w_ctr_cx );
	green = normalize_rgb( _cld wvt$w_ctr_cy );
	blue = normalize_rgb( _cld wvt$w_ctr_cz );
	mono = ( green * 59 + red * 30 + blue * 11 ) / 100;
	G55_SET_COLOR_MAP_ENTRY( _cld wvt$w_ctr_color, red, green, blue, mono );
	}
}

int normalize_rgb( value )
    int value;
{
    int norm_value;

    if ( value > 100 )
	value = 100;
    norm_value = ( value * 65536 ) / 100 - 1;
    if ( norm_value < 0 )
	norm_value = 0;
    return norm_value;
}

/***********************************************/
xdecrqss(ld, code) /* Report on various states */
/***********************************************/

wvtp ld;
unsigned char code;

{
int event, start, final, ps, pu;
char buffer[60], *bptr;

	/* Parse the character stream and ignore until ST */

	parse_ansi(ld, code, &event, &start, &final);
	_cld wvt$b_last_event = event;

	if (event == R_GRAPHIC)

	 {
	   _cld wvt$dcs_final[_cld wvt$dcs_final_index++] = code;
	   return;
	 }

	if ((event == R_CONTROL) &&
	    _cld wvt$r_com_state.data[final] == C1_ST)
	 {
	   if (_cld wvt$dcs_final_index == 0) {
		unknown_decrqss(ld);
		return;
	   } 
	   switch (_cld wvt$dcs_final[0])
	     {
	      case '$':
		if (_cld wvt$dcs_final_index == 2) {
		  switch (_cld wvt$dcs_final[1]) {

		    case '}' :
		    if (_cld wvt$l_vt200_common_flags & vtc1_m_c1_transmission_mode){
		      WVT$TERMINAL_TRANSMIT(ld, "\2201$r");
		      if (_cld wvt$l_vt200_common_flags & vtc1_m_actv_status_display)
			WVT$TERMINAL_TRANSMIT(ld, "1$}\234");
		      else
			WVT$TERMINAL_TRANSMIT(ld, "0$}\234");
		    } else {
		      WVT$TERMINAL_TRANSMIT(ld, "\033P1$r");
		      if (_cld wvt$l_vt200_common_flags & vtc1_m_actv_status_display)	
			WVT$TERMINAL_TRANSMIT(ld, "1$}\033\\");
		      else
			WVT$TERMINAL_TRANSMIT(ld, "0$}\033\\");
		    }
		      _cld wvt$b_in_dcs = FALSE;
		      return;

		    case '~' :
	/* 8bit controlls mode	*/
		    if (_cld wvt$l_vt200_common_flags & vtc1_m_c1_transmission_mode){
		      WVT$TERMINAL_TRANSMIT(ld, "\2201$r");
		      if (ld->common.statusDisplayEnable)
		  	WVT$TERMINAL_TRANSMIT(ld, "2$~\234");
		      else
			WVT$TERMINAL_TRANSMIT(ld, "0$~\234");
		    } else {
		      WVT$TERMINAL_TRANSMIT(ld, "\033P1$r");
		      if (ld->common.statusDisplayEnable)
		  	WVT$TERMINAL_TRANSMIT(ld, "2$~\033\\");
		      else
			WVT$TERMINAL_TRANSMIT(ld, "0$~\033\\");
		    }
		      _cld wvt$b_in_dcs = FALSE;
		      return;
		  }
		}
		unknown_decrqss(ld);
		break;

	      case '"':
		if (_cld wvt$dcs_final_index == 2) {
		  switch (_cld wvt$dcs_final[1]) {

		    case 'q' :                    
/****************************************************************/
/*   if terminal sends DECSCA, host sends DECRQSS to terminal.	*/
/*   And response of DECRQSS is:				*/
/*           DECSCA(0 or 2)  ->    DECRQSS(0)			*/
/*           DECSCA(1)       ->    DECRQSS(1)			*/
	/* 8bit controlls mode	*/
		    if (_cld wvt$l_vt200_common_flags & vtc1_m_c1_transmission_mode){
		      WVT$TERMINAL_TRANSMIT(ld, "\2201$r");
		      if (_ld wvt$w_actv_rendition & csa_M_SELECTIVE_ERASE)
			WVT$TERMINAL_TRANSMIT(ld, "0\"q\234");
		      else
			WVT$TERMINAL_TRANSMIT(ld, "1\"q\234");
		    } else {
		      WVT$TERMINAL_TRANSMIT(ld, "\033P1$r");
		      if (_ld wvt$w_actv_rendition & csa_M_SELECTIVE_ERASE)
			WVT$TERMINAL_TRANSMIT(ld, "0\"q\033\\");
		      else
			WVT$TERMINAL_TRANSMIT(ld, "1\"q\033\\");
		    }
		      _cld wvt$b_in_dcs = FALSE;
		      return;

		    case 'p' :
	/* 7bit controlls mode	*/
	if (!(_cld wvt$l_vt200_common_flags & vtc1_m_c1_transmission_mode))
		      WVT$TERMINAL_TRANSMIT(ld, "\033P1$r");
		      else
		      WVT$TERMINAL_TRANSMIT(ld, "\2201$r");
		      switch (_cld wvt$b_conformance_level) {
			case LEVEL1 :
			  WVT$TERMINAL_TRANSMIT(ld, "61");
			  break;
			case LEVEL2 :
			  WVT$TERMINAL_TRANSMIT(ld, "62");
			  break;
			case LEVEL3 :
			  WVT$TERMINAL_TRANSMIT(ld, "63");
			  break;
			}
		      if (_cld wvt$b_conformance_level > LEVEL1) {
			if (_cld wvt$l_vt200_common_flags &
				vtc1_m_c1_transmission_mode) {
			  WVT$TERMINAL_TRANSMIT(ld, ";0");
			} else {
		     	  WVT$TERMINAL_TRANSMIT(ld, ";1");
			}
		      }
	/* 7bit controlls mode	*/
	if (!(_cld wvt$l_vt200_common_flags & vtc1_m_c1_transmission_mode))
		      WVT$TERMINAL_TRANSMIT(ld, "\"p\033\\");
		      else
		      WVT$TERMINAL_TRANSMIT(ld, "\"p\234");
		      _cld wvt$b_in_dcs = FALSE;
		      return;
		  }
		}
		unknown_decrqss(ld);
		break;

		case 'r' :
	/* 7bit controlls mode	*/
	if ( !( _cld wvt$l_vt200_common_flags & vtc1_m_c1_transmission_mode ))
		if (_cld wvt$dcs_final_index == 1) {
		  WVT$TERMINAL_TRANSMIT(ld, "\033P1$r");
		  xmit_value(ld, _ld wvt$l_top_margin);
		  WVT$TERMINAL_TRANSMIT(ld, ";");
		  xmit_value(ld, _ld wvt$l_bottom_margin);
		  WVT$TERMINAL_TRANSMIT(ld, "r\033\\");
		  _cld wvt$b_in_dcs = FALSE;
		  return;
		} else
		if (_cld wvt$dcs_final_index == 1) {
		  WVT$TERMINAL_TRANSMIT(ld, "\2201$r");
		  xmit_value(ld, _ld wvt$l_top_margin);
		  WVT$TERMINAL_TRANSMIT(ld, ";");
		  xmit_value(ld, _ld wvt$l_bottom_margin);
		  WVT$TERMINAL_TRANSMIT(ld, "r\234");
		  _cld wvt$b_in_dcs = FALSE;
		  return;
		}
		unknown_decrqss(ld);
		return;
		
		case 'm' :
	/* 7bit controlls mode	*/
	if ( !( _cld wvt$l_vt200_common_flags & vtc1_m_c1_transmission_mode ))
		if (_cld wvt$dcs_final_index == 1) {
	          WVT$TERMINAL_TRANSMIT(ld,"\033P1$r0");
		  if (_ld wvt$w_actv_rendition & csa_M_BOLD)
			WVT$TERMINAL_TRANSMIT(ld, ";1");
		  if (_ld wvt$w_actv_rendition & csa_M_UNDERLINE)
			WVT$TERMINAL_TRANSMIT(ld, ";4");
		  if (_ld wvt$w_actv_rendition & csa_M_BLINK)
			WVT$TERMINAL_TRANSMIT(ld, ";5");
		  if (_ld wvt$w_actv_rendition & csa_M_REVERSE)
			WVT$TERMINAL_TRANSMIT(ld, ";7");
                  WVT$TERMINAL_TRANSMIT(ld, "m\033\\");
		  _cld wvt$b_in_dcs = FALSE;
		  return;
		} else
		if (_cld wvt$dcs_final_index == 1) {
	          WVT$TERMINAL_TRANSMIT(ld,"\2201$r0");
		  if (_ld wvt$w_actv_rendition & csa_M_BOLD)
			WVT$TERMINAL_TRANSMIT(ld, ";1");
		  if (_ld wvt$w_actv_rendition & csa_M_UNDERLINE)
			WVT$TERMINAL_TRANSMIT(ld, ";4");
		  if (_ld wvt$w_actv_rendition & csa_M_BLINK)
			WVT$TERMINAL_TRANSMIT(ld, ";5");
		  if (_ld wvt$w_actv_rendition & csa_M_REVERSE)
			WVT$TERMINAL_TRANSMIT(ld, ";7");
                  WVT$TERMINAL_TRANSMIT(ld, "m\234");
		  _cld wvt$b_in_dcs = FALSE;
		  return;
		}
		unknown_decrqss(ld);
		return;

		case '\'' :
		if (_cld wvt$dcs_final_index == 2) {
		  switch (_cld wvt$dcs_final[1]) {

		    case 'z' :		/* DECELR - Enable Locator Reports */
	/* 7bit controlls mode	*/
	if ( !( _cld wvt$l_vt200_common_flags & vtc1_m_c1_transmission_mode ))
		      WVT$TERMINAL_TRANSMIT(ld, "\033P1$r");
		else
		      WVT$TERMINAL_TRANSMIT(ld, "\2201$r");
		      if (_cld wvt$l_vt200_common_flags &
				vtc1_m_locator_report_mode)
			{
			/*
			 * Locator reports are enabled. Check for one shot.
			 */
			if ( _cld wvt$l_vt200_common_flags &
				vtc1_m_locator_one_shot )
			    ps = 2;
			else
			    ps = 1;
			}
		      else
			{
			/*
			 * Locator reports are disabled.
			 */
			ps = 0;
			}
		      /*
		       * Check for character cell or physical pixels.
		       */
		      if ( _cld wvt$l_vt200_common_flags &
				vtc1_m_locator_cell_position )
			pu = 2;
		      else
			pu = 1;
		      /*
		       * Format and send the report.
		       */
	/* 7bit controlls mode	*/
	if ( !( _cld wvt$l_vt200_common_flags & vtc1_m_c1_transmission_mode ))
		      sprintf( buffer, "%d;%d'z\033\\", ps, pu );
		else
		      sprintf( buffer, "%d;%d'z\234", ps, pu );
		      WVT$TERMINAL_TRANSMIT( ld, buffer );
		      _cld wvt$b_in_dcs = FALSE;
		      return;

		    case 'w' :		/* DECEFR - Enable filter rectangles */
	/* 7bit controlls mode	*/
	if ( !( _cld wvt$l_vt200_common_flags & vtc1_m_c1_transmission_mode ))
		      sprintf( buffer, "\033P1$r%d;%d;%d;%d'w\033\\",
			_cld wvt$l_filter_ly, _cld wvt$l_filter_lx,
			_cld wvt$l_filter_uy, _cld wvt$l_filter_ux );
	else
		      sprintf( buffer, "\2201$r%d;%d;%d;%d'w\234",
			_cld wvt$l_filter_ly, _cld wvt$l_filter_lx,
			_cld wvt$l_filter_uy, _cld wvt$l_filter_ux );
		      WVT$TERMINAL_TRANSMIT( ld, buffer );
		      _cld wvt$b_in_dcs = FALSE;
		      return;

		    case '{' :		/* DECSLE - Select Locator Events */
		      bptr = buffer;
	/* 7bit controlls mode	*/
	if ( !( _cld wvt$l_vt200_common_flags & vtc1_m_c1_transmission_mode ))
		      *bptr++ = '\033P';
		else
		      *bptr++ = '\220';
		      *bptr++ = '1';
		      *bptr++ = 'r';
		      if ( _cld wvt$l_vt200_common_flags &
				vtc1_m_locator_down_reports )
			*bptr++ = '1';	/* 1 = report button down events */
		      else
			*bptr++ = '2';	/* 2 = don't report button down events */
		      *bptr++ = ';';
		      if ( _cld wvt$l_vt200_common_flags &
				vtc1_m_locator_up_reports )
			*bptr++ = '3';	/* 3 = report button up events */
		      else
			*bptr++ = '4';	/* 4 = don't report button up events */
		      *bptr++ = '\'';
		      *bptr++ = '{';
	/* 7bit controlls mode	*/
	if ( !( _cld wvt$l_vt200_common_flags & vtc1_m_c1_transmission_mode ))
		      *bptr++ = '\033\\';
		else
		      *bptr++ = '\234';
		      *bptr++ = '\0';
		      WVT$TERMINAL_TRANSMIT(ld, buffer);
		      _cld wvt$b_in_dcs = FALSE;
		      return;
		  }
		}
		unknown_decrqss(ld);
		break;

		default :
			unknown_decrqss(ld);
			break;
		}
	}

}

unknown_decrqss(ld)
wvtp ld;
{
	        /* Send <dcs>0$r<st> -- Unknown Request Type */

	/* 7bit controlls mode	*/
   if ( !( _cld wvt$l_vt200_common_flags & vtc1_m_c1_transmission_mode )) {
	        WVT$TERMINAL_TRANSMIT(ld,"\033P");
	        WVT$TERMINAL_TRANSMIT(ld,"0$r\033\\");
		} else {
	        WVT$TERMINAL_TRANSMIT(ld,"\220");
	        WVT$TERMINAL_TRANSMIT(ld,"0$r\234");
		}
	        _cld wvt$b_in_dcs = FALSE;
}
