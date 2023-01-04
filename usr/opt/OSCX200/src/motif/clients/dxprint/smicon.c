/*
*****************************************************************************
**                                                                          *
**                         COPYRIGHT (c) 1987, 1988, 1989 BY		    *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**                         ALL RIGHTS RESERVED                              *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
*****************************************************************************
**++
**  FACILITY:
**
**      Session Manager
**
**  AUTHOR:
**
**      Jake VanNoy 
**
**  ABSTRACT:
**
**      This module contains the icon bitmap.
**
**  ENVIRONMENT:
**
**      User mode, executable image.
**
**
**  MODIFICATION HISTORY:
**
**	02-MAY-1989	Karen Brouillette
**	    Changes for Ultrix.
**      28-APR-1989     Karen Brouillette
**          New format for modification history.
**	V2-001			Karen Brouillette	01-FEB-1989
**		Update copyright.  Add module headers.
**	V1-002			Karen Brouillette	16-JUN-1988
**		Use XCreatePixmapfromBitmapData instead of setting
**		all of the image fields ourselves.  In particular
**		the pad field here was set to 16 instead of 8 as
**		it should have been, which caused the 17x17 icon
**		to be garbage, and the 16x16 icon to be incorrect
**		centered in iconify box.  Now we can use 17x17 size
**		instead.
**      V1-001  JLV0001         Jake VanNoy		18-Apr-1988
**              Copied from Notepad.
**
**--
*/

#include "iprdw.h"
#include "smdata.h"
#include "sm_icon.h"
#include "sm_iconify.h"
#if defined(VMS) && !defined(__DECC)
#pragma nostandard
#endif
#include <X11/Vendor.h>
#if defined(VMS) && !defined(__DECC)
#pragma standard
#endif
#include "prdw_entry.h"

int IconInit
#if _PRDW_PROTO_
(Widget w)
#else
(w)
Widget w;
#endif
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      Create each of the icon pixmaps out of the static bit data in
**	the include files.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**	sm_icon_bits - Bits which make up the key icon for the icon box
**	sm_iconify_bits - Bits which are the key icon for the title bar
**	sm_icon_reverse_bits - Reverse video bits for icon box
**	sm_iconify_reverse_bits - Reverse video bits for title bar
**
**  IMPLICIT OUTPUTS:
**
**	smdata.icon - The pixmap for icon box
**	smdata.iconify - The pixmap for the title bar
**	smdata.reverse_icon - The pixmap for the reverse video icon for box
**	smdata.reverse_iconify - The pixmap for the reverse video icon for title
**
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/
{
    Display *display = XtDisplay(w);
    Screen *screen = XtScreen(w);
    
    smdata.icon = XCreatePixmapFromBitmapData
    (
	display,
	RootWindowOfScreen (screen),
	sm_icon_bits,
	(Dimension) sm_icon_width,
	(Dimension) sm_icon_height,
	(unsigned long) BlackPixelOfScreen (screen), 
	(unsigned long) WhitePixelOfScreen (screen),
	(unsigned int) 1
    );

    smdata.iconify = XCreatePixmapFromBitmapData
    (
	display,
	RootWindowOfScreen (screen),
	sm_iconify_bits,
	(Dimension) sm_iconify_width,
	(Dimension) sm_iconify_height,
	(unsigned long) BlackPixelOfScreen (screen), 
	(unsigned long) WhitePixelOfScreen (screen),
	(unsigned int) 1
    );
}
