/* #module DT_source_stubs.c "X0.0" */
/*
 *  Title:	DT_source_stubs.c
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © 1988, 1993                                                 |
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
 *	<short description of module contents>
 *
 *  Procedures contained in this module:
 *
 *	<list of procedure names and abstracts>
 *
 *  Author:	<original author>
 *
 *  Modification history:
 *
 *  Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 * Aston Chan		30-Dec-1991	V3.1
 *	- Change #ifndef I18N_MULTIBYTE && I18N_HEB to
 *	  #if !defined(I18N_MULTIBYTE) && !defined(I18N_HEB)
 *	- Typo in _DECwTermRedisplay().
 *
 * Aston Chan		19-Dec-1991	V3.1
 *	- Add stubs for routines exist only when I18n_* flags are on.
 *	  To avoid screwing up the symbol_vector.
 *	- Change I18n entry names to _DECwTerm prefix.
 *
 * Bob Mesenger		17-Jul-1990	X3.0-5
 *	Merge in Toshi Tanimoto's changes to support Asian terminals -
 *	- functions used in control representation mode
 *
 *  Bob Messenger	01-Jul-1990	X2.0-5
 *	- Remove stub routine for WVT$PRINT_LINE.
 */

#include "wv_hdr.h"

WVT$SMOOTH_SCROLL(ld)
wvtp ld;
{ /* hey bob, do smooth scrolling */
}

WVT$JUMP_SCROLL(ld)
wvtp ld;
{
}

WVT$RECALL_FONT(ld)
wvtp ld;
{
/* I believe this one returns the font used before "don't interpret controls"
   is selected.
*/
    if ( _cld wvt$l_ext_flags & vte1_m_asian_common ) {
	_ld wvt$b_gl = _ld wvt$b_sav_gl_crm;
	_ld wvt$b_gr = _ld wvt$b_sav_gr_crm;
	_ld wvt$b_g_sets[0] = _ld wvt$b_save_g_sets_crm[0];
	_ld wvt$b_g_sets[1] = _ld wvt$b_save_g_sets_crm[1];
	_ld wvt$b_ext_g_sets[0] = _ld wvt$b_save_ext_g_sets_crm[0];
	_ld wvt$b_ext_g_sets[1] = _ld wvt$b_save_ext_g_sets_crm[1];
	_ld wvt$b_ups = _ld wvt$b_save_ups_crm;
    }
}

WVT$CONTROL_FONT(ld)
wvtp ld;
{
    if ( _cld wvt$l_ext_flags & vte1_m_asian_common ) {
	_ld wvt$b_sav_gl_crm = _ld wvt$b_gl;
	_ld wvt$b_sav_gr_crm = _ld wvt$b_gr;
	_ld wvt$b_gl = 0;
	_ld wvt$b_gr = 1;
	_ld wvt$b_save_g_sets_crm[0] = _ld wvt$b_g_sets[0];
	_ld wvt$b_save_g_sets_crm[1] = _ld wvt$b_g_sets[1];
	_ld wvt$b_g_sets[0] = CRM_FONT_L;
	_ld wvt$b_g_sets[1] = CRM_FONT_R;
	_ld wvt$b_save_ext_g_sets_crm[0] = _ld wvt$b_ext_g_sets[0];
	_ld wvt$b_save_ext_g_sets_crm[1] = _ld wvt$b_ext_g_sets[1];
	_ld wvt$b_save_ups_crm = _ld wvt$b_ups;
	_ld wvt$b_ext_g_sets[0] = ONE_BYTE_SET;
	_ld wvt$b_ext_g_sets[1] = ONE_BYTE_SET;
    }
}

WVT$RESET_KB_COMPOSE(ld)
wvtp ld;
{

}

WVT$PASTE_TEXT()
{
}

WVT$SELECT_FONT()
{
}

WVT$SET_ISO_KB_COMPOSE()
{
}

CHANGE_LRP_POINTER_PATTERN()
{
}

RESET_LRP_POINTER_PATTERN()
{
}
