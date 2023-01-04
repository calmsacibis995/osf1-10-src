/* #module wv_hdr.h "X0.0" */
/*
 *  Title:	wv_hdr.h
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © 1993                                                       |
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
 *  Author:	Daniel McKracken
 *
 *  Modification history:
 *
 *  Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 *  Alfred von Campe    14-Oct-1992     Ag/BL10
 *      - Added a !defined(__alpha).
 *
 *  Aston Chan		17-Dec-1991	V3.1
 *	- I18n code merge
 *
 *  Alfred von Campe    06-Oct-1991     Hercules/1 T0.7
 *      - Changed private #include file names to all lower case.
 *
 *  Aston Chan		1-Sep-1991	Alpha
 *	- Don't redefine NULL.  Complained by DECC
 *
 *  Bob Messenger       17-Jul-1990     X3.0-5
 *      Merge in Toshi Tanimoto's changes to support Asian terminals -
 *	- definition of macro ext_rendition()
 *
 *  Bob Messenger	21-Jun-1990	X3.0-5
 *	- Support printer port extension.
 *		- Define PRINTER_EXTENSION as TRUE.
 *
 *  Bob Messenger	12-Dec-1988	X1.1-1
 *	- add _mld and _sld definitions so we won't use status display's
 *	  state by mistake
 */

#include "dectermp.h"

/* define true and false */
/* #define TRUE	1  previously defined in "intrinsics.h" */
#define FALSE	0

#if !defined(ALPHA) && !defined(__alpha)
#define	NULL	0
#endif /* ALPHA */

#define	EOF	(-1)

#define SPACE  32
#define LC1			0xc2
#define LC2			0xcb

/* define target environment */

#define PRO			FALSE
#define RAINBOW			FALSE
#define PROVUE			TRUE

/* define compiler specific non-standard usages */

#define WHITESMITHS_C		FALSE
#define DECUS_C			FALSE

#define LEVEL_1			TRUE
#define LEVEL_2			TRUE
#define LEVEL_3			TRUE

/* define these to add extensions to the architecture */
#define COLUMN_EXTENSION	TRUE	/* Always */
#define DRCS_EXTENSION		FALSE	/* */
#define UDK_EXTENSION		TRUE	/* Always */
#define REGIS_EXTENSION		FALSE	/* */
#define SIXEL_EXTENSION		FALSE	/* */
#define SEL_ERASE_EXTENSION	TRUE	/* Always */
#define PRINTER_EXTENSION	TRUE	/* */
#define HORIZONTAL_SCROLL	FALSE

#define EXTENDED_PANDA FALSE		/* Extended functions that were
					   removed from the PANDA and that
					   caused it's removal from the
					   emulator */


/*
 *
 *  _cld   is a POINTER to the common fields of the main and status display
 *  _ld    is a POINTER to the specific structure of the main or status display
 *  _mld   is a POINTER to the specific structure of the main display
 *  _sld   is a POINTER to the specific structure of the status display
 *
 *  wvtp   is a macro to define the formal parameter for
 *         calls as a pointer to a structure
 */

#define _cld ld->source.
#define _ld ld->source.SpecificSourceData->
#define _mld ld->source.MainSourceData.
#define _sld ld->source.StatusSourceData.
#define wvtp register _wvtp
#define _wvtp DECtermWidget

#define ld_to_w(ld) (DECtermWidget) ld
#define w_to_ld(w) (_wvtp) w

/*
 *	Macros to access the display data structure.  If changes are made to
 *	the display structure, these must be changed as well.
 */

#define line_width(y) _ld wvt$w_widths[y]
#define line_rendition(y) _ld wvt$b_rendits[y]
#define character(y,x) (_ld wvt$a_code_base[y])[x]
#define rendition(y,x) (_ld wvt$a_rend_base[y])[x]
#define ext_rendition(y,x) (_ld wvt$a_ext_rend_base[y])[x]
