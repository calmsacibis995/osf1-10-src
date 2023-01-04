/* #module WV_STATLINE */
/*
 *  Title:	WV_STATLINE
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
 *
 *	Handle status line functions
 *  
 *	Routines contained in this module:
 *
 *		xdecsasd	- select active status display
 *		xdecssdt	- select status display type
 *
 *  Author:	Mike Leibow
 *
 *  Revision History:
 *
 *  Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 * Bob Messenger	17-Jan-1989	X1.1-1
 *	- moved many ld fields to common area
 *
 * Tom Porcher		18-Oct-1988	X0.5-4
 *	- allow no parameters in DECSSDT to turn status line off.
 *  
 */

#include "wv_hdr.h"

xdecsasd(ld)
wvtp ld;
{
	if (_cld wvt$b_conformance_level < LEVEL2 
	    || !ld->common.statusDisplayEnable )
		return;
	if (_cld wvt$b_parmcnt > 1) return;
	switch (_cld wvt$l_parms[0]) {
		case 0:
			WVT$MAIN_DISPLAY(ld);
			break;
		case 1:
			WVT$STATUS_DISPLAY(ld);
			break;
	}
}

xdecssdt(ld)
wvtp ld;
{
	if (_cld wvt$b_conformance_level < LEVEL2) return;
	if (_cld wvt$b_parmcnt > 1) return;
	switch (_cld wvt$l_parms[0]) {
		case 0:
		case 1:
			WVT$DISABLE_STATUS_DISPLAY(ld);
			break;
		case 2:
			WVT$ENABLE_STATUS_DISPLAY(ld);
			break;
	}
}
