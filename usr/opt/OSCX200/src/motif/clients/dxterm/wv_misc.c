/*
 *  Title:	WV_MISC
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
 *  This module contains miscellaneous routines.
 *  
 *  Routines contained in this module:
 *
 *	xdsr		- dispatch device status requests
 *	xmc		- media copy dispatcher
 *	xdcsseq		- dispatch DCS introducer functions
 *	xdecatff	- assign font family
 *	xmit_value	- convert binary to decimal and transmit to keyboard
 *	cpystr		- copy a string, return address of the terminator
 *			  in the result string.
 *	print_line	- write a string to the printer
 *
 *  Author:	Frederick G. Kleinsorge
 *		Low-End Workstation Graphics Engineering
 *
 * Modification history:
 *
 * Alfred von Campe     30-Sep-1993     BL-E
 *      - Use replace mode as a default for sixel drawing.
 *
 * Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 * Alfred von Campe     14-Oct-1992     Ag/BL10
 *      - Added typecasts to satisfy Alpha compiler.
 *
 * Eric Osman		19-Aug-1992	VXT V1.2
 *	- Add raster_change_ok bit for vms sixel bug fixes.
 *
 * Dave Doucette	27-Apr-1992	V3.1/Motif 1.1
 *	- Added other Asian terminal types to be included in the
 *	  Sixel Aspect ratio fix.
 *
 * Dave Doucette	22-Apr-1992	V3.1/Motif 1.1
 *	- Fixed Sixel Aspect ratio to test for DECwHanzi terminal type.
 *
 * Aston Chan		17-Dec-1991	V3.1
 *	- I18n code merge
 *
 * Aston Chan		18-Oct-1991	V3.1
 *	- Put "case 2:" in xdcsseq() routine before "default".
 *
 * Alfred von Campe     06-Oct-1991     Hercules/1 T0.7
 *      - Make cpystr() return an unsigned char instead of just char.
 *
 * Bob Messenger	 9-Sep-1990	X3.0-7
 *	- Parse Graphics to Host and Graphics to Printer sequences.
 *
 * Bob Messenger        17-Jul-1990     X3.0-5
 *      Merge in Toshi Tanimoto's changes to support Asian terminals -
 *	- ReGIS screen mode
 *
 * Bob Messenger	13-Jun-1990	X3.0-5
 *	- Fix the DSR for "no locator device", using status code 53 instead
 *	  of 59 (even though this should never happen because
 *	  vtc1_m_enable_locator is always set in wvt$l_vt200_common_flags).
 *
 * Bob Messenger	21-Jun-1990	X3.0-5
 *	- Add printer port support.
 *
 * Bob Messenger	 9-May-1989	X2.0-10
 *	- Always return Ps2 = 0 (keyboard language unknown) for keyboard
 *	  language DSR.
 *
 * Bob Messenger	11-Apr-1989	X2.0-6
 *	- Support parameter 2 (color table report) for DECRSTS.
 *
 * Bob Messenger	 7-Apr-1989	X2.0-6
 *	- Use new widget bindings (DECw prefix).
 *
 * Bob Messenger	 2-Apr-1989	X2.0-5
 *	- Moved wvt$l_column_width to specific area.
 *
 * Bob Messenger	24-Jan-1989	X1.1-1
 *	- Support DECLKD (locator key definition)
 *
 * Bob Messenger	16-Jan-1989	X1.1-1
 *	- Move many ld fields to common area
 *
 * Bob Messenger	12-Jan-1989	X1.1-1
 *	- Return correct keyboard dialect for keyboard DSR
 *
 * Tom Porcher		20-Oct-1988	X0.5-4
 *	- Corrected UDK lock report to return condition, not last parameter.
 *
 * Mike Leibow          07-Jun-1988
 *      - Prepared code for status line.  Some ld fields are now accessed
 *        by _cld instead of _ld.
 *
 *		Tom Porcher		20-Jan-1988
 *	- change WVT$ routines to all uppercase.
 *
 *	FGK0012	 Frederick G. Kleinsorge	20-Apr-1987
 *  
 * 	o Mass symbol edit
 *
 *	FGK0011  Frederick G. Kleinsorge	16-Apr-1987
 *  
 * 	o change the data type of ld
 *
 *	FGK0010	 Frederick G. Kleinsorge	08-Apr-1987
 *  
 * 	o Check for ISO latin-1 enable flag for <DCS>1!u ...
 *
 *	FGK0009  Frederick G. Kleinsorge	06-Mar-1987
 *  
 * 	o Add case for DCS !x sequence (copy text to paste buffer)
 *
 *	FGK0008	 Frederick G. Kleinsorge	05-Mar-1987
 *  
 * 	o V3.2
 *
 *	FGK0007	 Frederick G. Kleinsorge	09-Jan-1987
 *  
 * 	o Ignore DECLKD if ReGIS is not available.
 *
 *	FGK0006	 Frederick G. Kleinsorge	26-Nov-1986
 *  
 * 	o Reports for terminal width/height had numeric values
 *	  reversed (column returned lines...) 
 *
 *	FGK0005	 Frederick G. Kleinsorge	13-Aug-1986
 *  
 * 	o Fix background select for SIXELs (error in SRM)
 *
 *	FGK0004	 Frederick G. Kleinsorge	22-Jul-1986
 *  
 * 	o Update version to X04-017
 *
 *	FGK0003  Frederick G. Kleinsorge 17-Jul-1986
 *
 *	o Do the ReGIS initialization code.
 *
 *	FGK0002	 Frederick G. Kleinsorge 10-Jul-1986
 *
 *	o Change the SIXEL init code to use OVERLAY mode
 *	  for background select of 1 (or default) -- it
 *	  still will clear the screen.  A value of 3 will
 *	  NOT clear the screen, and use REPLACE mode.  This
 *	  will make it more compliant with the VT240.
 *
 *	FGK0001  Frederick G. Kleinsorge 26-Jun-1986
 *
 *	o Change xmit_value to not send a leading 0 for
 *	  numbers < 10.  (routine is used by xda to send
 *	  conformance_level based DA string).
 *
 */

#include "wv_hdr.h"

#ifdef VXT_DECTERM
#include "vxtprinter.h"
extern void VxtPrinterGetStatus();
#endif VXT_DECTERM

/*******************************/
unsigned char *cpystr(s1,s2) /* Copy a string */
/*******************************/

unsigned char *s1, *s2;


/*
    copy's from the second string to the first.  No boundry checks done,
    input string needs to be null terminated.  Pointer to end of output
    string is returned (will be pointing to a null byte).

 */

{

register unsigned char *a, *b;

a = s1;
b = s2;


	while(*a++ = *b++);
	return --a;

}

/**********************************/
xdsr(ld) /* Device Status Reports */
/**********************************/

wvtp ld;

{

char tmpstr[20];

	switch (_cld wvt$b_privparm)

	  {
	
	   case 0: /* not a private  */

	    switch (_cld wvt$l_parms[0])
	      {
	
	      case 5:		/* Terminal Status = _ALWAYS_ ready! */
	
		/* C1 control can be sent in the only 8bit mode	*/
		if ( !( _cld wvt$l_vt200_common_flags & vtc1_m_c1_transmission_mode ))
		    WVT$TERMINAL_TRANSMIT(ld,"\033[0n");
		else
		    WVT$TERMINAL_TRANSMIT(ld,"\2330n");
	
	        break;
	
	      case 6: xcpr(ld); break;	/* Cursor position report */
	
	      }

	    break;

	  case 1: /* private parameter '?' */

	    switch (_cld wvt$l_parms[0])

	      {

	      case 15:		/* Printer status */

#ifdef VXT_DECTERM
		{
	            {
	            VxtPrinterStatus prt_status;
	            unsigned int mode;
		    int status_report;
		    char buffer[6];

	            VxtPrinterGetStatus (&prt_status, &mode);
	            switch( prt_status) {
	            case no_printer:
			status_report = DECwNoPrinter;
		    break;
	            case not_available_not_ready:
	            case not_available_ready:
		    case available_not_ready:
			status_report = DECwPrinterNotReady;
		    break;
		    case available_ready:
			status_report =  DECwPrinterReady;
		    break;
		    default:
			status_report = DECwNoPrinter;
	            } /* switch() */

		    if ( !( _cld wvt$l_vt200_common_flags &
			vtc1_m_c1_transmission_mode ))
		    sprintf( buffer, "\033[?%dn", status_report );
		    else
		    sprintf( buffer, "\233?%dn", status_report );
		    WVT$TERMINAL_TRANSMIT( ld, buffer );
	            }
		}
#else VXT_DECTERM
		{
		int call_data = DECwCRPrinterStatus;
		DECtermWidget w = ld_to_w(ld);
		char buffer[6];

		/*
		 * Ask the application to update the value of printerStatus.
		 */
		XtCallCallbacks((Widget)w, DECwNprinterStatusCallback,
				&call_data );
		/*
		 * Make sure the printer status is valid.
		 */
		if ( w->common.printerStatus != DECwPrinterReady
		  && w->common.printerStatus != DECwPrinterNotReady
		  && w->common.printerStatus != DECwNoPrinter )
		    w->common.printerStatus = DECwNoPrinter;
		/*
		 * Format the response and send it to the application.
		 */
		/* C1 control can be sent only in 8bit mode	*/
		if ( !( _cld wvt$l_vt200_common_flags & vtc1_m_c1_transmission_mode ))
		sprintf( buffer, "\033[?%dn", w->common.printerStatus );
		else
		sprintf( buffer, "\233?%dn", w->common.printerStatus );
		WVT$TERMINAL_TRANSMIT( ld, buffer );
		}
#endif VXT_DECTERM
		break;

	      case 25:		/* UDK lock condition */

		/* C1 control can be sent in the only 8bit mode	*/
		if ( !( _cld wvt$l_vt200_common_flags & vtc1_m_c1_transmission_mode )) {
	        if (_cld wvt$l_vt200_common_flags & vtc1_m_lock_set)
		       WVT$TERMINAL_TRANSMIT(ld,"\033[?21n");
	        else   WVT$TERMINAL_TRANSMIT(ld,"\033[?20n");
		} else {
	        if (_cld wvt$l_vt200_common_flags & vtc1_m_lock_set)
		       WVT$TERMINAL_TRANSMIT(ld,"\233?21n");
	        else   WVT$TERMINAL_TRANSMIT(ld,"\233?20n");
		}
		break;

	      case 26:		/* KB language */

		if ( _cld wvt$l_ext_flags & vte1_m_tomcat ) {
		if (_cld wvt$l_vt200_common_flags & vtc1_m_c1_transmission_mode)
			WVT$TERMINAL_TRANSMIT(ld, "\233?27;17n");
		else	WVT$TERMINAL_TRANSMIT(ld, "\033[?27;17n");
		} else if ( _cld wvt$l_ext_flags & vte1_m_bobcat ) {
		if (_cld wvt$l_vt200_common_flags & vtc1_m_c1_transmission_mode)
			WVT$TERMINAL_TRANSMIT(ld, "\233?27;24n");
		else	WVT$TERMINAL_TRANSMIT(ld, "\033[?27;24n");
		} else if ( _cld wvt$l_ext_flags & vte1_m_dickcat ) {
		if (_cld wvt$l_vt200_common_flags & vtc1_m_c1_transmission_mode)
			WVT$TERMINAL_TRANSMIT(ld, "\233?27;25n");
		else	WVT$TERMINAL_TRANSMIT(ld, "\033[?27;25n");
		} else if ( _cld wvt$l_ext_flags & vte1_m_fishcat ) {
		if (_cld wvt$l_vt200_common_flags & vtc1_m_c1_transmission_mode)
			WVT$TERMINAL_TRANSMIT(ld, "\233?27;27n");
		else	WVT$TERMINAL_TRANSMIT(ld, "\033[?27;27n");
		}
		else
		/* C1 control can be sent in the only 8bit mode	*/
		if ( !( _cld wvt$l_vt200_common_flags & vtc1_m_c1_transmission_mode ))
		WVT$TERMINAL_TRANSMIT(ld, "\033[?27;0n");
		else
		WVT$TERMINAL_TRANSMIT(ld, "\233?27;0n");
				/* keyboard language is unknown */

		break;

	      case 55:		/* Locator Status */

		/* C1 control can be sent only in 8bit mode	*/
		if ( !( _cld wvt$l_vt200_common_flags & vtc1_m_c1_transmission_mode ))
		strcpy(tmpstr,  "\033[?5");
		else
		strcpy(tmpstr,  "\233?5");

		if (!(_cld wvt$l_vt200_common_flags & vtc1_m_enable_locator))
		   strcat(tmpstr, "3n");
		      else
			if (GET_LOCATOR_STATUS(ld))
			    strcat(tmpstr, "0n");
				else strcat(tmpstr, "8n");
		WVT$TERMINAL_TRANSMIT (ld, tmpstr);

		break;

	      case 56:		/* Locator Type */

		/* C1 control can be sent only in 8bit mode	*/
		if ( !( _cld wvt$l_vt200_common_flags & vtc1_m_c1_transmission_mode )) {
		if (_cld wvt$l_vt200_common_flags & vtc1_m_tablet_connected)
			WVT$TERMINAL_TRANSMIT (ld,"\033[?57;2n");
		else
			WVT$TERMINAL_TRANSMIT (ld,"\033[?57;1n");
		} else {
		if (_cld wvt$l_vt200_common_flags & vtc1_m_tablet_connected)
			WVT$TERMINAL_TRANSMIT (ld,"\233?57;2n");
		else
			WVT$TERMINAL_TRANSMIT (ld,"\233?57;1n");

		}
		break;

	      default: break;

      }

    break;

  default: break;

  }

}

/*********************************************/
xmc(ld) /* Media Copy - Character Cell Print */
/*********************************************/

wvtp ld;

{

long status, line;

if (!(_cld wvt$w_print_flags & pf1_m_prt_enabled)) return;

	switch (_cld wvt$b_privparm)

	  {

	   case 0:  /* not a private */

	    switch (_cld wvt$l_parms[0])

	      {
	      case 0:

			if ( _cld wvt$l_vt200_common_flags &
					vtc1_m_actv_status_display ) {
			   print_lines( ld, 1, _mld wvt$l_actv_line,
				line_width(_mld wvt$l_actv_line),
				_mld wvt$l_actv_line,'\n' );
			   break;
			}

			DECwTermPrintTextScreen(ld);
			break;		/* print screen */

	      case 4:	WVT$EXIT_PRINT_CONTROLLER_MODE(ld);
			break;

	      case 5:	WVT$ENTER_PRINT_CONTROLLER_MODE(ld);
			break;

	      default:	break;
	      }

	    break;

	   case 1:  /* private paramater '?' */

	    switch (_cld wvt$l_parms[0])

	      {
	      case 0: _cld wvt$w_print_flags &= ~pf1_m_graphics_to_host; break;
						/* graphics to printer */
	      case 1: print_lines( ld, 1, _mld wvt$l_actv_line,
			line_width(_mld wvt$l_actv_line), _mld wvt$l_actv_line,
			'\n' );	break;		/* print line */
	      case 2: _cld wvt$w_print_flags |= pf1_m_graphics_to_host; break;
						/* graphics to host */
	      case 4: WVT$EXIT_AUTO_PRINT_MODE(ld);   break;
	      case 5: WVT$ENTER_AUTO_PRINT_MODE(ld);  break;
	      case 8:  /* reset printer to host mode */
	          {
		  DECtermWidget w = ld_to_w(ld);
		  int call_data = 0;
		  w->common.printerToHostEnabled = False;
		  XtCallCallbacks( (Widget)w, DECwNstopPrinterToHostCallback,
		    &call_data);
		  }
		  break;
	      case 9:  /* set printer to host mode */
	          {
		  DECtermWidget w = ld_to_w(ld);
		  int call_data = 0;
		  w->common.printerToHostEnabled = True;
		  XtCallCallbacks( (Widget)w, DECwNstartPrinterToHostCallback,
		    &call_data);
		  }
		  break;

	      default: break;
	      }

	    break;

	   default: break;

	  }
}

/********************************************************************/
xdcsseq(ld) /* Device Control String - Find out which kind and init */
/********************************************************************/

wvtp ld;

{

int n;
short x, y;
register int *r1,r2;


	_cld wvt$dcs_final_index = 0;
/* If CAN or SUB detected in DCS, then ignore DCS sequence.	*/
	if ( _cld wvt$b_can_sub_detected == 1 ) {
	   _cld wvt$b_can_sub_detected = 0;
	   _cld wvt$b_in_dcs = IGNORE_DCS;
	   return;
	}
	switch (_cld wvt$b_finalchar)
	  {

  case 'u':	   /* final = "u", Assign user-prefered supplemental set */

	/* Look for DCS Ps !u */

	if ((_cld wvt$l_vt200_flags_2 & vt2_m_enable_ISO_latin) &&
	   ((_cld wvt$b_intercnt == 1) &&
	    (_cld wvt$b_inters[0] == 33)))
		_cld wvt$b_in_dcs = _cld wvt$b_finalchar;
	   else
		_cld wvt$b_in_dcs = IGNORE_DCS;	

	break;


  case 'w':	   /* final = "w", Locator button load (ReGIS only) */


	/* Look for DCS Pc $w ... */
    if (!(_ld wvt$l_vt200_specific_flags & vts1_m_regis_available))
	{
	 _cld wvt$b_in_dcs = IGNORE_DCS;
	 return;
	}

	if ((_cld wvt$b_intercnt == 1) && (_cld wvt$b_inters[0] == 36))
	  {
	   switch (_cld wvt$l_parms[0])
	       {
		case 0:

			/* clear definitions */
			for (n = 0; n < 10; n ++)
				_cld wvt$w_loc_length[n] = 0;

		case 1:
		default:

			_cld wvt$b_in_dcs = _cld wvt$b_finalchar;
			_cld wvt$w_loc_state = U_START;
			break;
	       }
	  }
	else
		_cld wvt$b_in_dcs = IGNORE_DCS;	

	break;


  case 'p': 	   /* wvt$b_finalchar = "p", ReGIS */

    if ( (_cld wvt$b_intercnt == 1) && (_cld wvt$b_inters[0] == '$') ) {
	if (_cld wvt$b_parmcnt == 1) {
		if (_cld wvt$l_parms[0] == 1) {
			_cld wvt$b_in_dcs = DECRSTS;
			_cld wvt$dcs_tsr_buff.cp = _cld wvt$dcs_tsr_buff.buffer;
			_cld wvt$dcs_tsr_buff.index = 0;
			_cld wvt$dcs_tsr_buff.bit_index = 0;
		} else if (_cld wvt$l_parms[0] == 2 ) {
			_cld wvt$b_in_dcs = DECRSTS_CTR;
				/* color table report */
			_cld wvt$b_ctr_state = CTR_STATE_INIT;
		} else _cld wvt$b_in_dcs = IGNORE_DCS;
	} else _cld wvt$b_in_dcs = IGNORE_DCS;
	return;
    }
    if ( (_cld wvt$b_parmcnt > 1) ||
	!(_ld wvt$l_vt200_specific_flags & vts1_m_regis_available))
	/* If there were more than 1 parameter *or* if ReGIS is not available */
	{
	 _cld wvt$b_in_dcs = IGNORE_DCS;
	 return;
	}

    if (( _cld wvt$l_ext_flags & vte1_m_asian_common ) &&
	!ld->common.regisScreenMode ) {
	 _cld wvt$b_in_dcs = IGNORE_DCS;
	 return;
    }

    switch (_cld wvt$l_parms[0])
	{
	 case 0:
	 case 1:
	 case 2:
	 case 3: _cld wvt$b_regis_mode = _cld wvt$l_parms[0];
		 break;
	default: break;
	}

      _cld wvt$b_in_dcs = _cld wvt$b_finalchar;

      WVT$START_REGIS(ld);	/* Initialize ReGIS parser */

    break;



  case '|':	   /* wvt$b_finalchar = "|", User Defined Key Definition */

    if (_cld wvt$b_parmcnt > 2)
	{
	 _cld wvt$b_in_dcs = IGNORE_DCS;
	 break;
	}

    switch (_cld wvt$l_parms[0])
	{
	 case 0:
		_cld wvt$l_vt200_common_flags |=  vtc1_m_udk_erase_control;
		break;
	 case 1:
		_cld wvt$l_vt200_common_flags &= ~vtc1_m_udk_erase_control;
		break;
	 default:
		break;
	}

    switch (_cld wvt$l_parms[1])
	{
	 case 1:
		_cld wvt$l_vt200_common_flags &= ~vtc1_m_udk_lock_control;
		break;
	 default:
		_cld wvt$l_vt200_common_flags |= vtc1_m_udk_lock_control;
		break;
	}

    if (!(_cld wvt$l_vt200_common_flags & vtc1_m_lock_set))
        {
	 _cld wvt$b_in_dcs = _cld wvt$b_finalchar;
	 _cld wvt$w_udk_state = U_START;

	 if (_cld wvt$l_vt200_common_flags & vtc1_m_udk_erase_control)
		{
		 for (n = 0; n<MAX_UDK_VALUE; n++)
			_cld wvt$w_udk_length[n] = 0;
		 _cld wvt$w_udk_space_used = 0;
		}

	}
    else _cld wvt$b_in_dcs = IGNORE_DCS;

    break;


  case 'q':	   /* wvt$b_finalchar = "q" */

	  /*  The final was a "q", it probably is a SIXEL command,
	      but it could be a DECRQSS (report query) command --
	      check the wvt$b_intercnt
	   */


	   if (_cld wvt$b_intercnt == 0)
	    {
	    /* SIXELs */

	     _cld wvt$b_in_dcs = _cld wvt$b_finalchar;
             ld->sixel.raster_change_ok = 1;

	     switch (_cld wvt$l_parms[1])

	      {

		case 1:		/* Background select = Use OVERLAY mode */

			WVT$SET_OVERLAY_SIXEL(ld);
			break;

		case 2:		/* Use REPLACE mode */

			WVT$SET_REPLACE_SIXEL(ld);
			break;

		default:	/* Background select = DEFAULT (clear first) */

			WVT$SET_REPLACE_SIXEL(ld);

			WVT$ERASE_DISPLAY(	ld,
						_ld wvt$l_actv_line,
						_ld wvt$l_actv_column,
						_ld wvt$l_page_length,
						_ld wvt$l_column_width);

			for ( y = _ld wvt$l_actv_line;
				y <= _ld wvt$l_page_length; y++ )
			    {
			    line_rendition(y) = SINGLE_WIDTH;
			    line_width(y) = _ld wvt$l_column_width;
			    }

			break;


	       }

	     switch (_cld wvt$l_parms[0])
		/* Set the Aspect ratio */
		{
		 case 0:
		 default:
		 	 if (( ld->common.terminalType == DECwKanji  ) ||
			     ( ld->common.terminalType == DECwHanzi  ) ||
			     ( ld->common.terminalType == DECwHanyu  ) ||
			     ( ld->common.terminalType == DECwHangul )   )
			{
			/* The default and 0 aspect ratios are different
			   for the Asian VTs.
			*/
			_cld wvt$l_sixel_lines = 6;   break;	/* 1:1 */
			}
		 case 1:
		 case 5:
		 case 6:

			 _cld wvt$l_sixel_lines = 2*6; break;	/* 2:1 */

		 case 2: _cld wvt$l_sixel_lines = 5*6; break;	/* 5:1 */

		 case 3:
		 case 4: _cld wvt$l_sixel_lines = 3*6; break;	/* 3:1 */

		 case 7:
		 case 8:
		 case 9: _cld wvt$l_sixel_lines = 6;   break;	/* 1:1 */

		}

/* For compatibility with terminals we should erase the text display list,
   but this causes problems on DECterm because the screen gets redrawn
   when the color map is allocated */
/*	     WVT$ERASE_DISPLAY_LIST(ld);	*/

	     WVT$SIXEL_FLUSH(ld);	/* Init sixel buffer state */

	     break; /* **** END OF SIXEL INIT **** */

	    }

	   /* See if it was DECRQSS = <dcs>$q D...D<st> */

	   else if ((_cld wvt$b_intercnt == 1) &&
		    (_cld wvt$b_inters[0] == DECRQSS))

	    {
	     /* Request state setting */
	     _cld wvt$b_in_dcs = DECRQSS;
	     break;
	    }


	   /* Wasn't anything we recognized -- set IGNORE_DCS state */

	   _cld wvt$b_in_dcs = IGNORE_DCS;

	   break;

  case 't':	   /* wvt$b_finalchar = "t", restore presentation state */
	/* Look for <DCS> ps $ t */
	if ((_cld wvt$b_intercnt == 1) && (_cld wvt$b_inters[0] == '$'))
		{
		if (_cld wvt$l_parms[0] == 1) {
			_cld wvt$dcs_final_index = 0;
			_cld wvt$dcs_count = 0;   /* count parameters */
			_cld wvt$l_parms[0] = 0;  /* used to build numbers */
			_cld wvt$b_in_dcs = DECRSPS_CIR;
			break;
		}
		if (_cld wvt$l_parms[0] == 2) {
			for (n = 1; n <= MAX_COLUMN; n++)
				_cld wvt$b_tab_stops[n] = 0;
			_cld wvt$dcs_final_index = 0;
			_cld wvt$dcs_count = 0;
			_cld wvt$l_parms[0] = 0;  /* used to build numbers */
			_cld wvt$b_in_dcs = DECRSPS_TABSR;
			break;
		}
		}
	_cld wvt$b_in_dcs = IGNORE_DCS;
	break;                                
	
  case 'x':	   /* wvt$b_finalchar = "x", Copy data to Paste Buffer */

	/* Look for <DCS> !x */

	if ((_cld wvt$b_intercnt == 1) && (_cld wvt$b_inters[0] == 33))
		{
		 _cld wvt$b_in_dcs = _cld wvt$b_finalchar;	/* DECCTPB */
		 _cld wvt$w_udk_state = U_START; /* Use the UDK variables */
		 pars_init(ld); /* Init the parser */
		}
	else	_cld wvt$b_in_dcs = IGNORE_DCS;	

	break;


#if DRCS_EXTENSION
  case '{':		/* wvt$b_finalchar = "{", Soft character set load */ 
    if (_cld wvt$b_conformance_level == LEVEL1)
	{
	 _cld wvt$b_in_dcs = IGNORE_DCS; /* Ignore sequence in VT100 mode */
	 break;
	}

    for (x = _cld wvt$b_parmcnt; x < 6; x++)
	_cld wvt$l_parms[x] = 0;	/* Zero excess parameters */

/*
 *  Note to myself... the following switch looks bogus to me.  As a matter of
 *  fact a lot of this code looks suspect.  If/when DRCS is ever implemented
 *  I'll need to check this.
 *
 */

    for (x = 0; x < 6; x++)
      switch (x)
	{
        case 0:
		if (_cld wvt$l_parms[0] > 1)
			_cld wvt$b_in_dcs = IGNORE_DCS;
		break;
	case 1:
		if ((_cld wvt$l_parms[1] == 0) ||
		    (_cld wvt$l_parms[1] > 94))
			_cld wvt$b_in_dcs = IGNORE_DCS;
		break;
	case 2:
		if (_cld wvt$l_parms[2] > 2)
			_cld wvt$b_in_dcs = IGNORE_DCS;
		break;
	case 3:
		if ((_cld wvt$l_parms[3] > 0) &&
		    (_cld wvt$l_parms[3] < 4) ||
		    (_cld wvt$l_parms[3] > 4))
			_cld wvt$b_in_dcs = IGNORE_DCS;
		break;
	case 4:
	case 5:
		if (_cld wvt$l_parms[x] > 2)
			_cld wvt$b_in_dcs = IGNORE_DCS;
		break; 
	}

    if (_cld wvt$b_in_dcs == IGNORE_DCS)
	break;
    if (_cld wvt$l_parms[2] == 2)
	_cld wvt$l_parms[2] = 0;	/* Don't erase all fonts */
    if (_cld wvt$l_parms[3] == 4)
	_cld wvt$l_parms[3] = 0;	/* Use device default */
    if (_cld wvt$l_parms[4] == 1)
	_cld wvt$l_parms[4] = 0;	/* Use device default */
    if (_cld wvt$l_parms[5] == 1)
	_cld wvt$l_parms[5] = 0;	/* Use device default */

    for (x = 3; x < 6; x++)
      switch (x)
	{
	 case 3:
		if (_cld wvt$l_parms[3] != _cld Drcs_Pcms)
			_cld wvt$l_parms[2] = 0;
		break;
	case 4:
		if (_cld wvt$l_parms[4] != _cld Drcs_Pw)
			_cld wvt$l_parms[2] = 0;
		break;
	case 5:
		if (_cld wvt$l_parms[5] != _cld wvt$b_drcs_pt)
			_cld wvt$l_parms[2] = 0;
		break;
	}

    if (_cld wvt$l_parms[2] == 0)
	{
	 _cld wvt$b_dscs_intercnt = 0;
	 for (x = 0; x < 3; x++)
		_cld wvt$b_dscs[x] = 0;
	}

    _cld wvt$b_in_dcs = _cld wvt$b_finalchar;
    _cld wvt$b_drcs_state = 1;
    _cld Drcs_Pcms = _cld wvt$l_parms[3];
    _cld Drcs_Pw = _cld wvt$l_parms[4];
    _cld wvt$b_drcs_pt = _cld wvt$l_parms[5];
    _cld wvt$b_intercnt = 0;
    _cld wvt$b_inters[0] = 0;
    _cld wvt$b_inters[1] = 0;
    break;
#endif


  case '}':		/* wvt$b_finalchar = "}", Assign Type Font Family */

	/*  DECATFF <DCS>p1;p2}xxxxxxx<ST> */

	if ((_cld wvt$l_parms[0] ==  2) &&
	   ((_cld wvt$l_parms[1] ==  0) ||
	    (_cld wvt$l_parms[1] == 10)))

	       {
		_cld wvt$b_in_dcs = DECATFF;
		_cld wvt$l_work_string.offset = 0;
		_cld wvt$l_work_string.address = _cld wvt$b_work_buffer;
	       }

	else _cld wvt$b_in_dcs = IGNORE_DCS;
  break;

  default:

	    _cld wvt$b_in_dcs = IGNORE_DCS;
	    break;

	  }
}

/**********************************************/
xdecatff(ld, code) /* Assign Type Font Family */
/**********************************************/

wvtp ld;
unsigned int code;

{

unsigned char *temp;
int event, start, final;

  /* Parse the character stream and ignore until ST */

  parse_ansi(ld, code, &event, &start, &final);
  _cld wvt$b_last_event = event;

  if ((event == R_CONTROL) &&
	_cld wvt$r_com_state.data[final] == C1_ST)
	{

	 /* Pad with '0' characters if not NULL (i.e. set default) */

	 if (_cld wvt$l_work_string.offset > 0 && _cld wvt$l_work_string.offset < 7)
	    {
	     for (temp = _cld wvt$b_work_buffer + _cld wvt$l_work_string.offset;
	     _cld wvt$l_work_string.offset < 7; _cld wvt$l_work_string.offset += 1)
	     *temp++ = 48;
	    }

	/* Select new font */

	 WVT$SELECT_FONT(ld, _cld wvt$b_work_buffer, _cld wvt$l_work_string.offset);

	 _cld wvt$b_in_dcs = FALSE;
	 return;
	}

  if (event == R_GRAPHIC)
     {

	 /*  Get the family name.  The string may be 31 characters
	  *  long but we currently ignore all but the first 7
	  *  characters for now.
	  */

	 if (_cld wvt$l_work_string.offset <= 31) /* Limit */
	    {
	     temp = _cld wvt$b_work_buffer + _cld wvt$l_work_string.offset;
	     *temp = code;
	     _cld wvt$l_work_string.offset += 1;
	    }
     }

} 



/*******************************************************************/
xmit_value(ld, value) /* Convert value to Decimal and send to host */
/*******************************************************************/

wvtp ld;
int value;

{

unsigned short temp,temp2;

	/*
	    This routine does a quick & dirty binary to decimal conversion
	    and send the output to the host.  This is only written to handle
	    values up to 999.
	 */

   temp = value / 10;

   if (temp == 0)
	{
	 temp2 = (value % 10) + 48;
	 WVT$TERMINAL_TRANSMIT(ld,&temp2);
	 return;
	}

   if (temp >= 10) 
	{
	 temp2 = (temp / 10) + 48;
         WVT$TERMINAL_TRANSMIT(ld,&temp2);
	 temp %= 10;
	}

   temp2 = temp + 48;
   WVT$TERMINAL_TRANSMIT(ld,&temp2);
   temp2 = (value % 10) + 48;
   WVT$TERMINAL_TRANSMIT(ld,&temp2);

}
