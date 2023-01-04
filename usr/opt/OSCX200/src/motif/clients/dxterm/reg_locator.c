/* #module REGIS_LOCATOR "X03-300" */
/*
 *  Title:	REGIS_LOCATOR
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © 1985, 1993                                                 |
 *  | By Digital Equipment Corporation, Maynard, Mass.                       |
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
 *  Maintain Locator Key Definitions.  This code is adapted from the
 *  UDK code to do the same thing for the locator buttons.  It needed
 *  a number of changes, but is essentially the same logic.
 *
 *  Routines in this module:
 *
 *	xlkd		-  Control Module for locator key definition
 *	lkddef		-  Define/delete definition
 *
 *  Author:	Frederick G. Kleinsorge
 *		Low-End Workstation Graphics Engineering
 *
 *  Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 *  Bob Messenger	24-Jan-1989	X1.1-1
 *	- Modified for inclusion in DECterm, and support loading button 0
 *	  as the null button report.
 *
 *  FGK0002	Frederick G. Kleinsorge	09-Jan-1987
 *  
 * 	o	Create WR_LOCATOR from WV_LOCATOR and move this module into
 *		the ReGIS driver.
 *
 *  FGK0001	Frederick G. Kleinsorge	22-Jul-1986
 *  
 * 	o	Create, Update version to X04-017
 *
 *
 */

#include "wv_hdr.h"


/************************************************************/
xlkd(ld, code)       /* DECLKD Load Locator Key Definitions */
/************************************************************/

wvtp ld;
int code;
{
int event,start,final;
int offset;

if ((_cld wvt$b_last_event != R_CONTINUE) && ((0140 & code) != 0))
	_cld wvt$b_last_event = (event = R_GRAPHIC);

else

	{
	 parse_ansi(ld, code, &event, &start, &final);
	 _cld wvt$b_last_event = event;

	  switch (event)

	   {

	    case R_GRAPHIC:

	    case R_CONTROL:	code = _cld wvt$r_com_state.data[final];
				break;

	    case R_CONTINUE:

	    default:		return;
				break;
	   } 
	 }

switch (_cld wvt$w_loc_state)
 {

    case U_START:

	_cld wvt$w_loc_code = (-1);			/* Key value */
	_cld wvt$l_vt200_common_flags |= vtc1_m_loc_code_pair;
							/* Need hex pairs */
	_cld wvt$w_loc_code_value = 0;			/* Intermediate build */
	_cld wvt$w_loc_length[10] =
	_cld wvt$w_loc_length[11] = 0;			/* Used for scratch */
	_cld wvt$w_loc_half = 10;			/* Offset for build */

   case U_CODE:

	switch (event)
  	{

	case R_GRAPHIC:	_cld wvt$w_loc_state = U_CODE;

			if ((code >= 48) && (code <= 57))
			   {
			    if ( _cld wvt$w_loc_code < 0 )
				_cld wvt$w_loc_code = 0;
			    _cld wvt$w_loc_code =
			      (_cld wvt$w_loc_code*10) + (code - 48);
			   }
			else

		           {
			    switch (code)
		             {

			      case 47:	_cld wvt$w_loc_state = U_KEY_DEF;
					break;

			      case 59:	_cld wvt$w_loc_state = U_START;
					break;
		             }
		           }

			break;

	case R_CONTROL:	if ((code >= 8) && (code <= 13)) break;
			if ((code == C1_ST) &&
			    (_cld wvt$w_loc_code >= 0) &&
			    (_cld wvt$l_vt200_flags & vtc1_m_loc_code_pair)) lkddef(ld);
			_cld wvt$b_in_dcs = FALSE;

	default:	break;
	}

    break;

    case U_KEY_DEF:

	switch (event)
	{

	case R_GRAPHIC:	if (code == 59)
				{
				 if ((_cld wvt$w_loc_code >= 0)      &&
				     (_cld wvt$w_loc_half == 11) &&
				     (_cld wvt$l_vt200_common_flags
				       & vtc1_m_loc_code_pair)) lkddef(ld);
				 _cld wvt$w_loc_state = U_START;
				 break;
				}

			if (code == 47)		/* If we get a "/" then do the UP */
				{
				 if ((_cld wvt$w_loc_code >= 0)      &&
				     (_cld wvt$w_loc_half == 10) &&
				     (_cld wvt$l_vt200_common_flags
				       & vtc1_m_loc_code_pair)) lkddef(ld);
				 _cld wvt$l_vt200_common_flags |= vtc1_m_loc_code_pair;
				 _cld wvt$w_loc_half = 11;
				 _cld wvt$w_loc_code_value = 0;
				 break;
				}

			if (_cld wvt$l_vt200_common_flags & vtc1_m_loc_code_pair)
				{
				 _cld wvt$l_vt200_common_flags &= ~vtc1_m_loc_code_pair;

				 if ((code >= 48) && (code <= 57))
				   _cld wvt$w_loc_code_value = (code - 48);
				   else
				      if ((code >= 65) && (code <= 70))
				         _cld wvt$w_loc_code_value = (code - 55);
				      else
				         if ((code >= 97) && (code <= 102))
				            _cld wvt$w_loc_code_value = (code - 87);
				         else
					    break;
				}

			else

				{

				 _cld wvt$l_vt200_common_flags |=  vtc1_m_loc_code_pair;

				  if ((code >= 48) && (code <= 57))
				    _cld wvt$w_loc_code_value =
				      _cld wvt$w_loc_code_value*16 + (code - 48);
				    else
				       if ((code >= 65) && (code <= 70))
				          _cld wvt$w_loc_code_value =
					    _cld wvt$w_loc_code_value*16 + (code - 55);
				       else
				          if ((code >= 97) && (code <= 102))
				             _cld wvt$w_loc_code_value =
					       _cld wvt$w_loc_code_value*16 + (code - 87);
				          else
			        	     break;
				}

			if (_cld wvt$l_vt200_common_flags & vtc1_m_loc_code_pair)
		           {
			    offset = (_cld wvt$w_loc_half * 6) + _cld wvt$w_loc_length[_cld wvt$w_loc_half];

			    if (_cld wvt$w_loc_length[_cld wvt$w_loc_half] < 6)
		             {
				 _cld wvt$w_loc_length[_cld wvt$w_loc_half] += 1;
			         _cld wvt$b_loc_area[offset] = _cld wvt$w_loc_code_value;
			         _cld wvt$w_loc_code_value = 0;
			     }
			   }
			break;

	case R_CONTROL:	if ((code >= 8) && (code <= 13)) break;
			if ((code == 27) ||
			    (code == 26) ||
			    (code == 24) ||
			    (code > 127))
			    {
				if ((_cld wvt$w_loc_code >= 0) &&
					(_cld wvt$l_vt200_common_flags &
					  vtc1_m_loc_code_pair))
				    lkddef(ld);
				_cld wvt$b_in_dcs = FALSE;
			    }
	default:	break;

	}
 }
}

/*********************************/
lkddef(ld) /* Load a Locator Key */
/*********************************/

wvtp ld;

{

/*
 * The locator definitions are laid out:
 * 
 * 	_cld wvt$b_loc_area[0]
 * 
 * 	/ DKEY0 / DKEY1 / DKEY2 / DKEY3 / DKEY4 /
 * 	/ UKEY0 / UKEY1 / UKEY2 / UKEY3 / UKEY4 /
 * 	/ WKEY1 / WKEY1 /
 * 
 * Each entry is 6 bytes long.  UKEYn is the
 * UP key data, DKEYn is the DOWN definition.
 * WKEYn is the buffer scratch for input of
 * new definitions. (DKEY0 is used for null
 * button reports, and UKEY0 is unused.)
 * 
 * Each entry has a length field that indicates
 * just how long it is.
 * 
 * 
 * If the button code is < 0 or > 4 we will ignore
 * it.  Otherwise we'll multiply it by 2 if it's the
 * UP transition -- which is indicated by _cld wvt$w_loc_half
 * being an "11".
 * 
 * We'll just copy the scratch length into the real
 * length and copy the data down.
 * 
 */


register int i,button_code;
register unsigned char *work_pointer_new, *work_pointer_old;

button_code = _cld wvt$w_loc_code;

if ((button_code > 4) || (button_code < 0)) return;

if (_cld wvt$w_loc_half == 11) button_code += 5;

_cld wvt$w_loc_length[button_code] = _cld wvt$w_loc_length[_cld wvt$w_loc_half];

work_pointer_new = &_cld wvt$b_loc_area[6 * button_code];
work_pointer_old = &_cld wvt$b_loc_area[6 * _cld wvt$w_loc_half];

for (i = _cld wvt$w_loc_length[button_code]; i > 0 ; i -= 1)
	*work_pointer_new++ = *work_pointer_old++;

}
