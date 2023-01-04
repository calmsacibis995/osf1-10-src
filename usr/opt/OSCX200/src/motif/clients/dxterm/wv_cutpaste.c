/* #module WV_CUTPASTE "X03-305" */
/*
 *  Title:	WV_CUTPASTE
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © 1986, 1993						     |
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
 *  Implement Cut and Paste control strings
 *
 *  (Enable/Disable are in CONSEQ as set/reset mode controls)
 *
 *  Routines implemented in this module:
 *
 *  xdettpb	- Paste Text Buffer
 *  xdectpb	- Copy  Text Buffer
 *  xdecstpf	- Select Transparent Paste Format
 *
 *  Author:	Frederick G. Kleinsorge
 *		Low-End Workstation Graphics Engineering
 *
 *  Revision history:
 *
 *  Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 *  Bob Messenger	16-Jan-1989	X1.1-1
 *	- moved many ld fields into common area
 *
 *  Michael Leibow  03-JUN-1988
 *    put all cut/paste operations between IFDEF, ENDIF
 *    preprocessor statements because decterm doesn't need it.
 *
 *  FGK0005	Frederick G. Kleinsorge	16-Apr-1987
 *
 *  o Mass edit of symbols to conform to VMS standards
 *
 *  FGK0005	Frederick G. Kleinsorge	16-Apr-1987
 *
 *  o Change ld data type
 *
 *  FGK0004	Frederick G. Kleinsorge	15-Mar-1987
 *
 *  o Add additional param to paste for hex-pair encoding
 *
 *  FGK0003	Frederick G. Kleinsorge	06-Mar-1987
 *
 *  o Change to use the DEC STD 138 names and show the 'real'
 *    sequences in the documentation.
 *
 *  o Remove uneeded reference to UISUSRDEF.H
 *
 *  FGK0002	Frederick G. Kleinsorge	05-Mar-1987
 *
 *  o V3.2
 *
 *  FGK0001	Frederick G. Kleinsorge	04-Dec-1986
 *
 *  o Create.
 *
 */

#include "wv_hdr.h"

/**********************************/
xdecttpb(ld) /* Paste Text Buffer */
/**********************************/

wvtp ld;

/*
 *	Paste Text
 *
 *	<CSI> Ps; Pf )t
 *
 *	Ps	- Format
 *
 *			0   = Graphic data only (default)
 *			1   = Unfiltered
 *
 *	Pf	- DCS envelope
 *
 *			0   = Normal ASCII data
 *			1   = DCS sequence, data encoded as hex pairs
 *
 */

{
#ifdef CUTPASTE
  WVT$PASTE_TEXT(ld, _cld wvt$l_parms[0], _cld wvt$l_parms[1]);
#endif
} 

/********************************************/
xdecctpb(ld, code) /* Cut  (to) Text Buffer */
/********************************************/

wvtp ld;
int code;

/*
 *	Cut/Copy Text to Paste buffer
 *
 *	<DCS>!x D...D <ST>
 *
 *	*pbuf_lock	- Pointer to buffer lock,
 *			    bit 0 / clear = free
 *			              set = locked
 *	*pbuf_max	- Pointer to maximum size of buffer
 *	*pbuf_count	- Pointer to current count in buffer
 *	*pbuf_buffer	- Pointer to start of buffer
 *
 */

{
#ifdef CUTPASTE
int event,start,final;
unsigned char *offset;

 parse_ansi(ld, code, &event, &start, &final);
 _cld wvt$b_last_event = event;

 switch (event)

   {

    case R_GRAPHIC:	break;

    case R_CONTROL:	if (_cld wvt$r_com_state.data[final]==C1_ST)
			   {
			    *_cld wvt$a_pbuf_lock &= ~1;
			    _cld wvt$b_in_dcs = FALSE;
			   }
			  else if ((_cld wvt$r_com_state.data[final]  >= C0_HT) &&
				    (_cld wvt$r_com_state.data[final] <= C0_CR))
				return;

    default:		*_cld wvt$a_pbuf_lock &= ~1;
			_cld wvt$b_in_dcs = FALSE;
			return;
   } 

 switch (_cld wvt$w_udk_state)

   {

    case U_START:

	if (*_cld wvt$a_pbuf_lock) /* If the buffer is locked then ignore */
		{
		 _cld wvt$b_in_dcs = IGNORE_DCS;
		 return;
		}

	*_cld wvt$a_pbuf_lock |= 1;			/* Lock user buffer */
	*_cld wvt$a_pbuf_count = 0;			/* Set count to zero */
	_cld wvt$l_vt200_flags |= vt1_m_code_pair;	/* Init code-pair */
	_cld wvt$w_code_value = 0;			/* Init code value */
	_cld wvt$w_udk_state = U_KEY_DEF;		/* Set state */

    case U_KEY_DEF:

	/* Convert hex code to binary or exit if invalid wvt$b_graphic */

	if ((code >= 48) && (code <= 57))
	    _cld wvt$w_code_value = _cld wvt$w_code_value*16 + (code - 48);
	  else
	    if ((code >= 65) && (code <= 70))
	      _cld wvt$w_code_value = _cld wvt$w_code_value*16 + (code - 55);
	    else
	      if ((code >= 97) && (code <= 102))
	        _cld wvt$w_code_value = _cld wvt$w_code_value*16 + (code - 87);
	      else
		if (code == 32) break;
		  else
	               {
			*_cld wvt$a_pbuf_lock &= ~1;
			_cld wvt$b_in_dcs = IGNORE_DCS;
			break;
		       }

	/* Toggle the code-pair state */

	if (_cld wvt$l_vt200_flags & vt1_m_code_pair)
		_cld wvt$l_vt200_flags &= ~vt1_m_code_pair;
	  else  _cld wvt$l_vt200_flags |= vt1_m_code_pair;

	/* If we have a full pair then insert the character into the buffer */

	if (_cld wvt$l_vt200_flags & vt1_m_code_pair)
           {
	    if ((*_cld wvt$a_pbuf_count < *_cld wvt$a_pbuf_max) &&
		 (_cld wvt$a_pbuf_buffer != 0))
		{
		 offset = *_cld wvt$a_pbuf_count + _cld wvt$a_pbuf_buffer;
		 *offset = _cld wvt$w_code_value;
		 *_cld wvt$a_pbuf_count += 1;
		 _cld wvt$w_code_value = 0;
		}
	    else	/* Over max buffer size */
		{
		 *_cld wvt$a_pbuf_lock &= ~1;	/* Release buffer */
		 _cld wvt$b_in_dcs = IGNORE_DCS;	/* Exit */
		}
	   }
   }
#endif
} 

/************************************************/
xdecstpf(ld) /* Select Transparent Paste Format */
/************************************************/

wvtp ld;

/*
 *	Select Transparent Paste Format
 *
 *	<CSI> Ps )s
 *
 *	Ps	- Format
 *
 *			0   = wvt$b_graphic data only (default)
 *			1   = Unfiltered
 *
 */

{ 
#ifdef CUTPASTE
  _cld wvt$b_paste_output_format = _cld wvt$l_parms[0];
#endif
} 
