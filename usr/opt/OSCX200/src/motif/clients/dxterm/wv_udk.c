/* #module WV_UDK "X03-304" */
/*
 *  Title:	WV_UDK
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
 *  Maintain User Definable Keys
 *
 *  Routines in this module:
 *
 *	xudk	-  Control Module for key definition
 *	udkdef	-  Define/delete definition
 *
 *  Author:	Frederick G. Kleinsorge
 *		Low-End Workstation wvt$b_graphics Engineering
 *
 *		Adapted from code for the PRO Series Terminal
 *		Emulator.
 *
 * Revisions:
 *
 *  Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 *  Alfred von Campe    25-Mar-1993     V1.2/BL2
 *      - Add F1-F5 key support.
 *
 * Bob Messenger	15-Jul-1989	X2.0-16
 *	- don't exit DCS on error conditions
 *
 * Bob Messenger	17-Jan-1989	X1.1-1
 *	- moved many ld fields to common area
 *
 * Tom Porcher		20-Oct-1988	X0.5-4
 *	- Call out WVT$SET_MISC_MODES() when UDK lock is set.
 *
 *  FGK0004	Frederick G. Kleinsorge	22-Apr-1987
 *  
 *  o Mass edit symbol name changes
 *
 *  FGK0003	Frederick G. Kleinsorge	16-Apr-1987
 *  
 *  o Change data type of ld
 *
 *  FGK0002	Frederick G. Kleinsorge	05-Mar-1987
 *  
 *  o V3.2
 *
 *  FGK0001	Frederick G. Kleinsorge	22-Jul-1986
 *  
 *  o Update version to X04-017
 *
 */

#include "wv_hdr.h"


/***************************************/
xudk(ld, code) /* Process UDK sequence */
/***************************************/

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

switch (_cld wvt$w_udk_state)
 {

    case U_START:

      _cld wvt$w_udk_code = 0;
      _cld wvt$l_vt200_common_flags |= vtc1_m_code_pair;
      _cld wvt$w_udk_size = 0;
      _cld wvt$w_code_value = 0;

   case U_CODE:

      switch (event)
       {
	case R_GRAPHIC:

	  _cld wvt$w_udk_state = U_CODE;
          if ((code >= 48) && (code <= 57))
	    _cld wvt$w_udk_code = _cld wvt$w_udk_code*10 + (code - 48);
          else
           {
	    switch (code)
             {
	      case 47: _cld wvt$w_udk_state = U_KEY_DEF; break;

	      case 59: if (_cld wvt$l_vt200_common_flags & vtc1_m_code_pair)
				udkdef(ld);
                       _cld wvt$w_udk_state = U_START; 
                       break;
              }
           }
	  break;

	case R_CONTROL:

	  if (code == C1_ST)
           {
            if (_cld wvt$l_vt200_common_flags & vtc1_m_code_pair) udkdef(ld);
	    _cld wvt$b_in_dcs = FALSE;
	    if (_cld wvt$l_vt200_common_flags & vtc1_m_udk_lock_control)
		{
		_cld wvt$l_vt200_common_flags |= vtc1_m_lock_set;
		WVT$SET_MISC_MODES(ld);
		}
	   }

	default: break;
       }
    break;

    case U_KEY_DEF:

      switch (event)
       {

	case R_GRAPHIC:

	  if (code == 59)
           {
            if (_cld wvt$l_vt200_common_flags & vtc1_m_code_pair) udkdef(ld);
            _cld wvt$w_udk_state = U_START;
            break;
           }

	  if ((code >= 48) && (code <= 57))
	    _cld wvt$w_code_value = _cld wvt$w_code_value*16 + (code - 48);
	  else
	    if ((code >= 65) && (code <= 70))
	      _cld wvt$w_code_value = _cld wvt$w_code_value*16 + (code - 55);
	    else
	      if ((code >= 97) && (code <= 102))
	        _cld wvt$w_code_value = _cld wvt$w_code_value*16 + (code - 87);
	      else
		break;

	  if (_cld wvt$l_vt200_common_flags & vtc1_m_code_pair)
		_cld wvt$l_vt200_common_flags &= ~vtc1_m_code_pair;
	  else  _cld wvt$l_vt200_common_flags |= vtc1_m_code_pair;

	  if (_cld wvt$l_vt200_common_flags & vtc1_m_code_pair)
           {
	    offset = _cld wvt$w_udk_size + _cld wvt$w_udk_space_used;
	    if (offset <= UDK_AREA_SIZE)
             {
	      _cld wvt$b_udk_area[offset] = _cld wvt$w_code_value;
	      _cld wvt$w_udk_size += 1;
	     }
	    _cld wvt$w_code_value = 0;
	    }
	  break;

	case R_CONTROL:

	  if (code == C1_ST)
           {
	    if (_cld wvt$l_vt200_common_flags & vtc1_m_code_pair) udkdef(ld); 
	    _cld wvt$b_in_dcs = FALSE;
	    if (_cld wvt$l_vt200_common_flags & vtc1_m_udk_lock_control)
		{
		_cld wvt$l_vt200_common_flags |= vtc1_m_lock_set;
		WVT$SET_MISC_MODES(ld);
		}
	   }

	default: break;

	}
    }
}

/***************************/
udkdef(ld) /* Define a UDK */
/***************************/

wvtp ld;

{

register int i,function_key;
register unsigned char *work_pointer_new, *work_pointer_old;

function_key = _cld wvt$w_udk_code;

if (function_key >= 11 && function_key <= 15)           /* Map 11-15 to 0-4 */
  function_key -= 11;
else
  if (function_key >= 17 && function_key <= 21)         /* Map 17-21 to 5-9 */
    function_key -= 12;
  else
    if (function_key >= 23 && function_key <= 26)       /* Map 23-26 to 10-13 */
      function_key -= 13;
    else
      if (function_key >= 28 && function_key <= 29)     /* Map 28-29 to 14-15 */
        function_key -= 14;
      else 
         if (function_key >= 31 && function_key <= 34)  /* Map 31-34 to 16-19 */
            function_key -= 15;
         else
	    return;

if (_cld wvt$w_udk_length[function_key])
 {
  i = (_cld wvt$w_udk_space_used + _cld wvt$w_udk_size) -
      (_cld wvt$w_udk_data[function_key] + _cld wvt$w_udk_length[function_key]);

  if (i)
   {
    for ( work_pointer_new = -(_cld wvt$w_udk_length[function_key]) + 
        ( work_pointer_old = 
          &_cld wvt$b_udk_area[_cld wvt$w_udk_data[function_key] +
                              _cld wvt$w_udk_length[function_key]]);
          0 < i; i -= 1 )
        *work_pointer_new++ = *work_pointer_old++;

    for ( i = 0; i < MAX_UDK_VALUE; i +=1 )
        if (_cld wvt$w_udk_data[i] > _cld wvt$w_udk_data[function_key])
            _cld wvt$w_udk_data[i] -= _cld wvt$w_udk_length[function_key];

    _cld wvt$w_udk_space_used -= _cld wvt$w_udk_length[function_key];
   }

 }

  _cld wvt$w_udk_data[function_key] = _cld wvt$w_udk_space_used;
  _cld wvt$w_udk_length[function_key] = _cld wvt$w_udk_size;
  _cld wvt$w_udk_space_used += _cld wvt$w_udk_size;
}
