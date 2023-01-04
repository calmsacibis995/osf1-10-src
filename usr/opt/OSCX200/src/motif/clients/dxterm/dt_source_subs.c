/*
 *  Title:	DT_SOURCE_SUBS
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © 1985, 1993                                                 |
 *  | By Digital Equipment Corporation, Maynard, Mass.                       |
 *  | All rights reserved.                                                   |
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
 *	This module contains the subroutines necessary to drive the other
 *	modules.  This module shares and modifies display list information
 *	created by the WV_*.c modules.  This module also contains the routines
 *	that are exported to other modules.  (Dwt*, and s_*).
 *
 *  Environment:  VAX/VMS    VAX-11 C
 *
 *  Author:  Michael S. Leibow		08-09-1987
 *
 *  Modified by:
 *
 * Alfred von Campe     13-Nov-1993     BL-E
 *      - Buffer data if DECwTermPutData() is called after the output has been
 *        stopped.  This fixes the problem where Fileview hangs when the user
 *        hits F1 in a DCL Command window (we were stuck in a loop in put_data).
 *
 * Alfred von Campe     30-Sep-1993     BL-E
 *      - Add multi-page support.
 *
 * Grace Chung          15-Sep-1993     BL-E
 *      - Add 7-bit/8-bit printer support.
 *
 * Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 * Eric Osman		11-May-1993	VXT V2.0
 *	- Allow #nn (hex) for specifying non-printable chars in answerback
 *	  message
 *
 * Alfred von Campe     25-Mar-1993     V1.2/BL2
 *      - OSF/1 code merge.
 *
 * Aston Chan		18-Mar-1993	V1.2/BL2
 *	- Use #ifdef XlibSpecificationRelease  instead of #ifdef R5_XLIB
 *
 * Aston Chan		12-Mar-1993	V1.2/BL2
 *	- Add Turkish/Greek support.
 *	- ApplicationShellWidget is no longer available in V1.2.  Type Widget
 *	  is good enough for our purpose.
 *
 * Alfred von Campe     20-Jan-1993	Ag/BL12 (SSB)
 *	- Add XKME support for Hebrew I18N.
 *      - Hardcode the format to 32 in convert_selection().
 *
 * Aston Chan		13-Jan-1993	Post V1.1
 *	- Fix SPR ICA-44429 that <x] key if set to backspace is reset by EDT
 *	  or TPU.  Problem is a bug with I18n code in WVT$SET_MISC_MODES().
 *	  Confirmed with I18n people that it is a bug.
 *
 * Eric Osman		13-Oct-1992	VXT V1.2
 *	- update copy and paste logic so it doesn't hang  (use motif 1.1
 *	  version of code)
 *
 * Alfred von Campe     08-Oct-1992     Ag/BL10
 *	- Added typecasts to satisfy Alpha compiler.
 *
 * Steve Klein		 4-Sep-1992     VXT V1.2
 *	- Add call to x_set_value_keyboardDialect in WVT$GET_TERMINAL_SETUP to
 *	  make initial settings take effect.
 *
 * Eric Osman		11-June-1992	Sun
 *	- Remove "cda" include file that isn't needed and isn't defined
 *	  on Sun anyway.
 *
 * Eric Osman		 5-June-1992	VXT V1.1-A
 *	- In s_initialize, clear work_proc_registered *before* calling WVT$INIT
 *	  because WVT$INIT calls flush_source which sets work_proc_registered!
 *
 * Alfred von Campe     26-May-1992     Post V3.1
 *	- Remove unnecessary XtFree() call.
 *
 * Alfred von Campe     02-Apr-1992     Ag/BL6.2.1
 *	- Change #includes to satisfy OSF/1 compiler (partly from To-lung).
 *
 * Aston Chan		19-Feb-1992	V3.1/BL5
 *	- OSC sequence for changing Title is not working because of the I18n
 *	  code merge.  We have to have the callback in order to update
 *	  the stm->window_name, which will be used as final title.
 *
 * Eric Osman		22-Jan-1992	V3.1
 *	- In s_initialize, clear work_proc_registered *before* calling WVT$INIT
 *	  because WVT$INIT calls flush_source which sets work_proc_registered!
 *
 * Dave Doucette	14-Jan-1992	V3.1
 *	- Included Shai Ophir's DECterm/DECwrite paste bug fix.
 *
 * Eric Osman		20-Dec-1991	V3.1
 *	- Free up printer port and filenames upon exit
 *
 * Aston Chan		19-Dec-1991	V3.1
 *	- Changing the I18n entry points to _DECwTerm* prefix
 *
 * Aston Chan		17-Dec-1991	V3.1
 *	- I18n code merge
 *
 * Alfred von Campe     14-Oct-1991     V3.1
 *      - Merged Michele Lien's fix to allocate two more resources with
 *        XtNewString() so that we can XtFree them safely later.
 *
 * Michele Lien		12-Sep-1991	VXT V1.0
 *	- Fixed the bug of embedding the regis time delay command in
 *	  macrographs causes VXT to crash.  The problem is in push_data().
 *	  
 * Alfred von Campe     20-May-1991     V3.0
 *      - Add DECwTermXtMalloc and DECwTermXtCalloc routines.
 *
 * Eric osman		21-Mar-1991	V3.0
 *	- For string resources in dt_resources.h, use XtnewString here
 *	  so that XtFree later doesn't erroneously try to free storage
 *	  that was allocated by the C compiler.
 *
 * Alfred von Campe     26-Mar-1991     T3.0
 *      - Fix problem with quick-copying into DECwrite (crashed on ULTRIX).
 *
 * Alfred von Campe     04-Feb-1991     T3.0
 *      - Change free to XtFree.
 *
 * Eric Osman		14-Jan-1991
 *	- Added some calls to "free" in s_destroy to plug some minor
 *	  memory leaks.
 *
 * Bob Messenger	18-Sep-1990	X3.0-7
 *	- DECwTermReset should restore the default tab stops.
 *
 * Bob Messenger	13-Sep-1990	X3.0-7
 *	- Check for transcriptSize being negative.
 *
 * Bob Messenger	26-Aug-1990	X3.0-6
 *	- Free malloc()'ed data with free(), not XtFree(),
 *
 *  Michele Lien    24-Aug-1990 VXT X0.0
 *  - Modify this module to work on VXT platform. Change #ifdef VMS to
 *    #ifdef VMS_DECTERM so that VXT specific code can be compiled under
 *    VMS or ULTRIX development environment.
 *
 * Bob Messenger	20-Jul-1990	X3.0-5
 *	- Add WVT$ENABLE_SESSION routine to allow an escape sequence to
 *	  give the input focus to the current window.
 *
 * Bob Messenger	17-Jul-1990	X3.0-5
 *	Merge in Toshi Tanimoto's changes to support Asian terminals -
 *	- allocate/free extended renditions as needed
 *	- set value functions specific to Kanji terminal (Tomcat)
 *
 * Bob Messenger	23-Jun-1990	X3.0-5
 *	- Add printer port support.
 *
 * Bob Messenger	13-Apr-1990	X3.0-2
 *	- Fix scroll_selection to deal with scrolling regions correctly.
 *	- Changing the userPreferenceSet and terminalMode resources should not
 *	  cause a soft terminal reset.  The implied reset should only be done
 *	  if the user preference set or terminal mode is changed using an
 *	  escape sequence.
 *
 * Bob Messenger	 9-Apr-1990	X3.0-2
 *	- Merge UWS and VMS changes.
 *
 * Bob Messenger	19-Mar-1990	V2.1 (VMS V5.4)
 *	- Avoid memory leak in wvt$up_scroll when doing deferred scrolling
 *	  (change made on Ultrix 7-Nov-1989).
 *
 * Bob Messenger	12-Mar-1990	V2.1 (VMS V5.4)
 *	- Don't crash if we can't allocate enough memory when resizing.
 *	- Free the transcript if we aren't saving lines off the top.
 *
 * Bob Messenger	 7-Nov-1989	V2.0 (UWS V2.2)
 *	- Avoid memory leak in wvt$up_scroll when doing deferred scrolling.
 *
 * Bob Messenger	 5-Oct-1989	V2.0 (UWS V2.2)
 *	- Correction to 8-Aug-1989 change: always set/reset 8 bit mode when
 *	  changing the terminal mode, even if the widget hasn't been realized.
 *
 * Bob Messenger	 8-Aug-1989	X2.0-18
 *	- Don't do a soft terminal reset when the terminal mode or user
 *	  preference set has been changed unless the widget has been realized
 *	  (otherwise some of the user's saved settings will be ignored).
 *
 * Bob Messenger	24-Jul-1989	X2.0-16
 *	- Add a new exported routine: DECwTermFlushOutput.
 *
 * Bob Messenger	17-Jul-1989	X2.0-16
 *	- WVT$ERASE_DISPLAY shouldn't reset line renditions becuase it's
 *	  called for EL etc.  Instead, reset line renditions every place
 *	  it's needed.
 *
 * Bob Messenger	13-Jun-1989	X2.0-16
 *	- Use COPY macro for copying memory: memcpy on VMS, bcopy on Ultrix.
 *	  This is because memcpy on Ultrix doesn't always work if the
 *	  source and destination overlap.
 *
 * Bob Messenger	28-May-1989	X2.0-13
 *	- Erase the status line and put the cursor in the main display
 *	  when the status line is disabled.
 *	- When setting terminal mode, don't do a soft reset unless the
 *	  conformance level (or VT52 mode) is changing.
 *	- Replace bcopy and bzero with memcpy and memset.
 *
 * Bob Messenger	14-May-1989	X2.0-10
 *	- Eliminate printfs (they were in the left scroll and right scroll
 *	  routines, which were never called anyway).
 *
 * Bob Messenger	28-Apr-1989	X2.0-8
 *	- Fix bug with batch scrolling and erasing the top line, and add a
 *	  s_clear_display that clears the screen but also scrolls erased
 *	  lines into the transcript.
 *
 * Bob Messenger	21-Apr-1989	X2.0-7
 *	- Changed "title" and "iconName" to XtNtitle and XtNiconName.
 *	- Fixed bug where scrolling too many lines at once puts the
 *	  emulator into a loop.
 *
 * Bob Messenger	11-Apr-1989	X2.0-6
 *	- Implemented saveErasedLines.
 *
 * Bob Messenger	 7-Apr-1989	X2.0-6
 *	- Convert to new widget bindings (DECwTerm instead of DwtDECterm).
 *	  Also keep old bindings for backward compatibility.
 *	- Removed support for tab stops and display controls (since they
 *	  won't be implemented in V2).
 *
 * Bob Messenger	 1-Apr-1989	X2.0-5
 *	- Support transcriptSize
 *
 * Bob Messenger	31-Mar-1989	X2.0-5
 *	- Added support for OSC sequences to set title and icon name
 *	- Changed s_set_value_title and s_set_value_iconName to
 *	  s_set_value_defaultTitle and s_set_value_defaultIconName
 *
 * Bob Messenger	15-Mar-1989	X2.0-3
 *	- Added stub for s_set_value_transcriptSize.
 *	- Added VT330 id
 *	- Make global selection recognize TARGETS, and return FALSE if
 *	  the requested target was not supported.
 *
 * Bob Messenger	04-Mar-1989	X2.0-2
 *	- Removed support for Dutch NRC
 *
 * Bob Messenger	15-Feb-1989	X1.1-1
 *	- Fix Ultrix compilation errors.
 *
 * Bob Messenger	20-Jan-1989	X1.1-1
 *	- Use XQueryPointer to find the pointer position when a filter
 *	  rectangle is enabled, instead of relying on the last position set
 *	  from motion events.
 *
 * Bob Messenger	18-Jan-1989	X1.1-1
 *	- Set line rendition to single width in WVT$ERASE_DISPLAY (which
 *	  is called by WVT$DOWN_SCROLL, for example).
 *
 * Bob Messenger	16-Jan-1989	X1.1-1
 *	- Moved many ld fields to common area
 *
 * Bob Messenger	11-Jan-1989	X1.1-1
 *	- Make sure setting textCursorEnable sets wvt$b_cursts, and vice
 *	  versa, and move wvt$b_cursts to common area
 *
 * Bob Messenger	12-Dec-1988	X1.1-1
 *	- Make sure that the set-up resources correspond to the main display
 *	  state, not the status display state
 *
 * Tom Porcher		20-Oct-1988	X0.5-4
 *	- Force a soft terminal reset when user preference set is changed.
 *	- Use correct value for UDK lock condition.
 *
 * Tom Porcher		18-Oct-1988	X0.5-4
 *	- Removed conditional on call of o_set_marker() (defer != 1) in
 *	  WVT$UP_SCROLL().  This seems to be where the marker has gone!
 *	- Only allow convert_selection() to pass ASCII, DEC MCS, and
 *	  ISO Latin 1 characters (i.e., not line-drawing!).
 *	- Made WVT$ENABLE/DISABLE_STATUS_DISPLAY not call output to
 *	  refresh if the state is not changing.
 *
 * Bob Messenger	 8-Oct-1988	X0.5-3
 *	- allow for deferred batch scrolling.  Added s_execute_deferred,
 *	  which can be called from DT_output.c.
 *
 * Tom Porcher		 9-Sep-1988	X0.5-2
 *	- fix resize so that if it's bigger than MAX_LINES/COLUMNS then it
 *	  always updates.
 *
 * Tom Porcher		 9-Sep-1988	X0.5-1
 *	- convert_selection now replaces zeros in the display list with ' '.
 *
 * Tom Porcher		 7-Sep-1988	X0.5-0
 *	- s_set_selection() now only disowns the selection if it has it.
 *	- lose_selection() now assumes that the selection has been disowned.
 *	- removed DwtDECtermWorkProc() (temp for VUE no longer used).
 *      - Change DwtDECtermSelectAll() and DwtDECtermCopySelection() to use
 *        selection and time.
 *
 * Eric Osman		2-Sep-1988	BL9.2
 *	- Separate codes and renditions into independently allocated buffers.
 *
 * Mike Leibow		16-Aug-1988	X0.4-44
 *	- added WVT$LOCK_KEYBOARD and WVT$UNLOCK_KEYBOARD
 *
 * Tom Porcher		12-Aug-1988	X0.4-44
 *	- changing screenMode now sets reverseVideo to TRUE.
 *	- Added s_set_value_tabStops() and XtFree() of tabStops in s_destroy().
 *
 * Tom Porcher		15-Jul-1988	X0.4-37
 *	- Made DwtDECtermSelectAll() not crash by adding "type" parameter to
 *	  s_set_selection() call.
 *	- fixed refresh when changing selection types:  s_set_selection()
 *	  was using input.selection_type.
 *
 * Tom Porcher		10-Jul-1988	X0.4-36
 *	- made WVT$SET_KB_ATTRIBUTES() change autoRepeatEnable resource.
 *	- made changing keyclick or autorepeat call
 *	  i_change_keyboard_control().
 *
 * Tom Porcher		 8-Jul-1988	X0.4-36
 *	- changed calloc() to XtCalloc().
 *
 * Tom Porcher		21-Jun-1988	X0.4-32
 *	- Changed XtAddWorkProc() to XtAppAddWorkProc() for new intrinsics.
 *	- corrected s_work_proc to return TRUE to remove itself.
 *
 * Tom Porcher		24-May-1988	X0.4-30
 *	- fix forward declaration of s_work_proc for Ultrix.
 *
 * Bob Messenger	24-May-1988	X0.4-28
 *	- Add work procedure to redraw cursor, to improve performance.
 *
 * Eric Osman		17-May-1988	X0.4-27
 *	- Make blink timer only run when it needs to
 *
 * Tom Porcher		 9-May-1988	X0.4-26
 *	- Removed CurrentTime kludge.
 *	- added output flow control.
 *
 * Tom Porcher		21-Apr-1988	X0.4-10
 *	- Add s_initialize() to replace s_create() so that all data structures
 *	  will be initialized for a set_values before being realized.  This will
 *	  happen if either a set-up file or a customization string is specified.
 *	- Change name of s_create() to s_realize().
 *
 * Tom Porcher		 5-Apr-1988	X0.4-7
 *	- Change to new Xtoolkit Selection mechanism.
 *	- Attempt to fix blinking selection as it scrolls.
 *	- Added kludge to allow CurrentTime to work for selections;
 *	  this should be removed at next toolkit baselevel.
 *
 * Eric Osman		30-Mar-1988	X0.4-3
 *	- Make cursor blinking be the default
 *
 * Tom Porcher		16-Jan-1988	X0.3-2
 *	- Changed FMT8BIT to XA_STRING.  Requesting a selection type of 1
 *	  (FMT8BIT from the Text Widget) will return an item of type 1, so
 *	  the Text Widget can fetch stuff from us.
 *	- Added DwtDECtermSelectAll().
 *	- Added DwtDECtermGetSelection().
 *
 * Tom Porcher		16-Jan-1988	X0.3-2
 *	- Corrected computation of end of line on last line of selection.
 *
 * Tom Porcher		31-Dec-1987	X0.3
 *	- Made changes for BL6.2 of DECwindows.
 *
 * Tom Porcher		23-Dec-1987	X0.3
 *	- All routines called with a widget now convert the argument to
 *	  a "wvtp" if necessary.
 *	- Added code to support selection.
 *	- s_read_data() now returns a zero-width line outside of display region.
 *	- Added WVT$SET_KEY_MODE when keypad mode or cursor key mode is
 *	  changed.
 *
 */

#ifndef XK_LATIN1  /* these must be defined before keysymdef is included*/
#define		XK_LATIN1	1
#endif
#ifndef XK_HEBREW
#define		XK_HEBREW	1
#endif

#include "wv_hdr.h"

#if defined (VMS_DECTERM) || defined (VXT_DECTERM)
#include "DECspecific.h"
#include <keysymdef.h>
#include <x.h>
#include <xproto.h>
#include <xlib.h>
#include "xsmeproto_include.h"
#else
#include <DXm/DECspecific.h>
#include <X11/keysymdef.h>
#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/Xlib.h>
#include <X11/extensions/xkmeproto_include.h>
#endif

#ifdef VMS_DECTERM
extern XSMEDoKBModeSwitch();
#define DoKBModeSwitch XSMEDoKBModeSwitch
#endif

#ifdef VXT_DECTERM
extern XKMEDoKBModeSwitch();
#define DoKBModeSwitch XKMEDoKBModeSwitch
#endif

#if !defined(VMS_DECTERM) && !defined(VXT_DECTERM)
extern XKMEDoKBModeSwitch();
#define DoKBModeSwitch XKMEDoKBModeSwitch
#endif

enum kb_request{ PRIMARY_REQUEST =1, SECONDARY_REQUEST , 
                 DISPLAY_PRIMARY, DISPLAY_SECONDARY, TOGGLE_REQUEST = 255 };
static int myerror_status = KBModeSwitchSuccess;

#define moof(x)	((_cld wvt$l_vt200_common_flags & vtc1_m_actv_status_display) ? \
	    	(STATUS_LINE) : (x))      

#define MAX_DISPLAY_CHARS 85

#ifdef VXT_VMS_ENV
#define VMS_DECTERM True    /* turn on VMS environment */
#endif

#ifdef VMS_DECTERM
#define COPY( d, s, n ) memcpy( d, s, n )
#else
#define COPY( d, s, n ) bcopy( s, d, n )
#endif

#ifdef VXT_VMS_ENV
#undef VMS_DECTERM     /* turn off VMS environment */
#endif

extern void i_reset_aim();
extern DECtermPosition o_convert_XY_to_position();
static void lose_selection();
static void flush_source();
static void get_locator_position(), convert_locator_coordinates(),
    convert_button_state(), set_character_sets();
static Boolean s_button_handler(), s_work_proc();

void s_set_selection();

void s_initialize(w)
    DECtermWidget w;
{
    int i, temp;
    wvtp ld = w_to_ld( w );
/*
 * The following newstring calls prevent the set value action routines
 * from lousing up memory, which would happen if they attempted to xtfree
 * memory that was compiled into the dt_resources.h as a default string.
 * CAUTION:  To prevent memory leaks, if you add more here, change s_destroy
 * too.
 */
    if ( w->common.answerbackMessage != NULL )
	w->common.answerbackMessage = XtNewString(w->common.answerbackMessage);

    if ( w->common.printerFileName != NULL )
	w->common.printerFileName = XtNewString(w->common.printerFileName);

    if ( w->common.printerPortName != NULL )
	w->common.printerPortName = XtNewString(w->common.printerPortName);

    if ( w->common.defaultTitle != NULL )
	w->common.defaultTitle = XtNewString(w->common.defaultTitle);

    if ( w->common.defaultIconName != NULL )
	w->common.defaultIconName = XtNewString(w->common.defaultIconName);

    w->source.work_proc_registered = FALSE;

/*
 * It's important that work_proc_registered be inited before calling
 * WVT$INIT, since that calls flush_source which *sets* work_proc_registered.
 */
    WVT$INIT(ld);
    i_enable_button( w, s_button_handler );
    o_set_cursor_position(w, moof(1), 0);
    _cld targets_atom = XInternAtom( XtDisplay(w), "TARGETS", FALSE );
    Source_select_begin(w, 0) = 0;
    Source_select_begin(w, 1) = 0;
    Source_select_end(w, 0) = 0;
    Source_select_end(w, 1) = 0;
    w->source.stop_output = 0;
    w->source.put_data = NULL;
    w->source.put_count = 0;

    _ld wvt$l_defer_limit = ld->common.batchScrollCount;
    if ( ld->common.batchScrollCount <= 1 )
	{
	_cld wvt$l_vt200_flags_3 &= ~vt3_m_defer_up_scroll;
	_ld wvt$l_defer_max = 0;
	}
    else
	{
	_cld wvt$l_vt200_flags_3 |= vt3_m_defer_up_scroll;
	temp = _ld wvt$l_bottom_margin - _ld wvt$l_top_margin;
	if ( ld->common.batchScrollCount > temp )
	    _ld wvt$l_defer_max = temp;
	else
	    _ld wvt$l_defer_max = ld->common.batchScrollCount;
	}

    for(i = 0; i < MAX_NUMBER_OF_PAGES; i++)
    {
        _cld page[i].allocated = FALSE;
        _cld page[i].code_cells = NULL;
        _cld page[i].rend_cells = NULL;
        _cld page[i].ext_rend_cells = NULL;
        _cld page[i].widths = NULL;
        _cld page[i].rendits = NULL;
    }
    _cld current_page = 0;
}

void s_realize(w)
    DECtermWidget w;
{}

void s_destroy(w)
    DECtermWidget w;
{
    int i;
    wvtp ld = w_to_ld( w );

    if ( Source_has_selection(w,0))
	XtDisownSelection((Widget)w, XA_PRIMARY, CurrentTime);
    if ( Source_has_selection(w,1))
	XtDisownSelection((Widget)w, XA_SECONDARY, CurrentTime);
    WVT$STATUS_DISPLAY(ld);
    XtFree((char *)_ld wvt$a_code_cells);
    XtFree((char *)_ld wvt$a_rend_cells);
    XtFree((char *)_ld wvt$a_trans_code_base);
    XtFree((char *)_ld wvt$a_trans_rend_base);
    XtFree((char *)_ld wvt$w_trans_widths);
    XtFree((char *)_ld wvt$b_trans_rendits);
    XtFree((char *)_ld wvt$a_ext_rend_cells);
    XtFree((char *)_ld wvt$a_trans_ext_rend_base);
    WVT$MAIN_DISPLAY(ld);
    XtFree((char *)_ld wvt$a_code_cells);
    XtFree((char *)_ld wvt$a_rend_cells);
    XtFree((char *)_ld wvt$a_trans_code_base);
    XtFree((char *)_ld wvt$a_trans_rend_base);
    XtFree((char *)_ld wvt$w_trans_widths);
    XtFree((char *)_ld wvt$b_trans_rendits);
    XtFree((char *)_ld wvt$a_ext_rend_cells);
    XtFree((char *)_ld wvt$a_trans_ext_rend_base);
    if ( w->source.work_proc_registered )
	XtRemoveWorkProc( w->source.work_proc_id );
    if ( w->common.answerbackMessage != NULL )
	XtFree( w->common.answerbackMessage );
    if ( w->common.printerFileName != NULL )
	XtFree( w->common.printerFileName );
    if ( w->common.printerPortName != NULL )
	XtFree( w->common.printerPortName );
    if ( w->common.defaultTitle != NULL )
	XtFree( w->common.defaultTitle );
    if ( w->common.defaultIconName != NULL )
	XtFree( w->common.defaultIconName );

    for(i = 0; i < MAX_NUMBER_OF_PAGES; i++)
        if(_cld page[i].allocated)
            free_page(ld, i);
}

/*
 * put_data( w, data, count )
 *	Send ANSI data to display
 *
 * It would be nice if we could check for events in this loop.
 * The only way to make that happen would be to make this an event handler.
 */
static void
put_data( w, data, count )
    DECtermWidget w;
    char *data;
    int count;
{
    wvtp ld = w_to_ld( w );
    char *s;
    int rem, chars_used;

/* Send chunks of MAX_DISPLAY_CHARS to ansi_display(), checking if
 * we have been suspended each time.
 */

    for ( s = data, rem = count;
	  rem > 0 && !(w->source.stop_output & ~STOP_OUTPUT_TEMP) ; )
    {
        chars_used = ansi_display( ld, s,
		      rem < MAX_DISPLAY_CHARS ? rem : MAX_DISPLAY_CHARS );
	if (chars_used > count)
	    chars_used = count;
	if (chars_used < 0 )
	    chars_used = 0;
	s += chars_used;
	rem -= chars_used;
    }

/* If we keep being called even if the output has been suspended, just keep
 * buffering up the data.
 */

    if ( rem > 0 ) {
	w->source.put_data = (char *) XtRealloc( w->source.put_data,
	                                         w->source.put_count + rem );
	COPY( w->source.put_data + w->source.put_count, s, rem );
	w->source.put_count += rem;
    }
}


/*
 * put_buffered_data( w )
 *	Send the data that has been buffered when suspended.
 */
static void
put_buffered_data( w )
    DECtermWidget w;
{
    char *data;
    int count;

    if ( (data = w->source.put_data) != NULL ) {
	w->source.put_data = NULL;
	count = w->source.put_count;
	w->source.put_count = 0;
        put_data( w, data, count );
	XtFree( data );
    }
}

void DECwTermReset(w)
    DECtermWidget w;
{
    wvtp ld = w_to_ld( w );
    int in_status_now;

	/* Do a soft terminal reset on both status and main displays */
	xstr(ld);		/* reset to initial state */
	if (_ld wvt$l_vt200_specific_flags & vts1_m_regis_available)
		WVT$CLEAR_REGIS(ld);
	_cld wvt$b_last_event = R_GRAPHIC;
	_cld wvt$b_in_dcs        = 0;
	pars_init(ld);

	if ( _cld wvt$l_ext_flags & vte1_m_asian_common ) {
	    _cld wvt$l_vt200_common_flags |= vtc1_m_screen_mode;
	    w->common.screenMode = TRUE;
	    w->common.reverseVideo = FALSE;
	}

	/* reset the color map */
	WVT$RESET_COLORMAP( ld );

	/* reset tab stops */
	set_tab_stops(ld, 8);

	/* reset input method */
	i_reset_aim( w );
}

void DECwTermClearDisplay(w)
    DECtermWidget w;
{
    Boolean saveErasedLines;

/* don't save lines that were explicitly erased with DECwTermClearDisplay */

    saveErasedLines = w->common.saveErasedLines;
    w->common.saveErasedLines = False;
    s_clear_display( w );
    w->common.saveErasedLines = saveErasedLines;
}

void DECwTermClearTranscript(w)
    DECtermWidget w;
{
    wvtp ld = w_to_ld( w );
    _mld wvt$l_transcript_top = 1;
    o_set_top_line(w, 1);
}

void DECwTermPutData(w, data, count)
    DECtermWidget w;
    char *data;
    unsigned count;
{
    o_disable_cursor(w, CURSOR_DISABLED_SOURCE);
    put_data( w, data, count );
    flush_source( w );
}

void DECwTermConvertXYToANSI( w, x, y, colp, rowp )
    DECtermWidget w;
    int	x,y;
    int *colp,*rowp;
{
    wvtp ld = w_to_ld( w );
    DECtermPosition pos;

    pos = o_convert_XY_to_position( w, x, moof(y));
    *colp = DECtermPosition_column( pos ) + 1;
    *rowp = DECtermPosition_row( pos );
    if (*rowp == STATUS_LINE) *rowp = 1;
}

void DECwTermConvertANSIToXY( w, col, row, xp, yp )
    DECtermWidget w;
    int col,row;
    int	*xp,*yp;
{
    wvtp ld = w_to_ld( w );

    o_convert_position_to_XY( w,
			      DECtermPosition_position( col-1, moof(row) ),
			      xp, yp );
}

/*
 * s_stop_output( w, reason-mask )
 *	Stop sending output for specified reason.
 */
void
s_stop_output( w, reason )
    DECtermWidget w;
    unsigned int reason;
{
    int call_data;
    unsigned int old_state;

    old_state = w->source.stop_output;
    w->source.stop_output |= reason;

    if (!old_state) {
	call_data = DECwCRStopOutput;
        XtCallCallbacks( (Widget)w, DECwNstopOutputCallback, &call_data );
    }
}
                   

/*
 * s_start_output( w, reason-mask )
 *	Resume sending output for specified reason.
 */
void
s_start_output( w, reason )
    DECtermWidget w;
    unsigned int reason;
{
    int call_data;

    if (!w->source.stop_output)
	return;

    w->source.stop_output |= STOP_OUTPUT_TEMP;

    if ( (w->source.stop_output &= ~reason) == STOP_OUTPUT_TEMP) {
	put_buffered_data( w );
	flush_source( w );
    }
     
    w->source.stop_output &= ~STOP_OUTPUT_TEMP;

    if (!w->source.stop_output) {
	call_data = DECwCRStartOutput;
        XtCallCallbacks( (Widget)w, DECwNstartOutputCallback, &call_data );
    }
}

s_line_structure *s_read_data(w, line)
    DECtermWidget w;
    int line;
{
    wvtp ld = w_to_ld( w );
    int in_status_line;

    in_status_line = _cld wvt$l_vt200_common_flags & vtc1_m_actv_status_display;
    if ( line >= STATUS_LINE ) {
	line = 1;
	WVT$STATUS_DISPLAY(ld);
    } else WVT$MAIN_DISPLAY(ld);

    if ( line >= _ld wvt$l_transcript_top && line <= w->common.rows )
	{
	int begin, end;

	/* Make the code_base and rend_base stuff 0 based instead of 1 based */
	_cld line_structure[0].a_code_base = &_ld wvt$a_code_base[line][1];
	_cld line_structure[0].a_rend_base = &_ld wvt$a_rend_base[line][1];
	_cld line_structure[0].a_ext_rend_base = &_ld wvt$a_ext_rend_base[line][1];
	_cld line_structure[0].w_widths = _ld wvt$w_widths[line];
	_cld line_structure[0].b_rendits = _ld wvt$b_rendits[line];

	if (Source_has_selection(w, 0) &&
	     (begin = DECtermPosition_row( Source_select_begin(w, 0) ))
		 <= line &&
	     (end = DECtermPosition_row( Source_select_end(w, 0) ))
		 >= line)
	    {
	    if ( begin == line )
		_cld line_structure[0].highlight_begin =
		    DECtermPosition_column( Source_select_begin(w, 0) );
	    else
		_cld line_structure[0].highlight_begin = 0;

	    if ( end == line )
		_cld line_structure[0].highlight_end =
		    DECtermPosition_column( Source_select_end(w, 0) );
	    else
		_cld line_structure[0].highlight_end =
		    _cld line_structure[0].w_widths;
	    if ( _cld wvt$l_ext_flags & vte1_m_hebrew ) {
	    if ( _cld wvt$l_ext_specific_flags & vte2_m_copy_dir ) {
		if ( begin == line && end != line ) {
		    _cld line_structure[0].highlight_end =
			_cld line_structure[0].highlight_begin;
		    _cld line_structure[0].highlight_begin = 0;
		} else if ( begin != line && end == line ) {
		    _cld line_structure[0].highlight_begin =
			_cld line_structure[0].highlight_end;
		    _cld line_structure[0].highlight_end =
			_cld line_structure[0].w_widths;
		}
	    }
	    }
	    }
	else if (Source_has_selection(w, 1) &&
	     (begin = DECtermPosition_row( Source_select_begin(w, 1) ))
		 <= line &&
	     (end = DECtermPosition_row( Source_select_end(w, 1) ))
		 >= line)
	    {
	    if ( begin == line )
		_cld line_structure[0].highlight_begin =
		    DECtermPosition_column( Source_select_begin(w, 1) );
	    else
		_cld line_structure[0].highlight_begin = 0;

	    if ( end == line )
		_cld line_structure[0].highlight_end =
		    DECtermPosition_column( Source_select_end(w, 1) );
	    else
		_cld line_structure[0].highlight_end =
		    _cld line_structure[0].w_widths;
	    if ( _cld wvt$l_ext_flags & vte1_m_hebrew ) {
	    if ( _cld wvt$l_ext_specific_flags & vte2_m_copy_dir ) {
		if ( begin == line && end != line ) {
		    _cld line_structure[1].highlight_end =
			_cld line_structure[1].highlight_begin;
		    _cld line_structure[1].highlight_begin = 0;
		} else if ( begin != line && end == line ) {
		    _cld line_structure[1].highlight_begin =
			_cld line_structure[1].highlight_end;
		    _cld line_structure[1].highlight_end =
			_cld line_structure[1].w_widths;
		}
	    }
	    }
	    }
	else
	    {
	    _cld line_structure[0].highlight_begin = 0;
	    _cld line_structure[0].highlight_end = 0;
	    }
	}
    else
	{
	_cld line_structure[0].w_widths = 0;
	_cld line_structure[0].b_rendits = 0;
	_cld line_structure[0].highlight_begin = 0;
	_cld line_structure[0].highlight_end = 0;
	}
    if (in_status_line) WVT$STATUS_DISPLAY(ld);
      else WVT$MAIN_DISPLAY(ld);
    return (&_cld line_structure[0]);
}

WVT$ERASE_DISPLAY(ld, topy, leftx, boty, rightx)
wvtp		ld;			/* wvt structure pointer */
int		topy;			/* top line to erase */
int		boty;			/* bottom line to erase */
int		leftx;
int		rightx;
{
unsigned char	*charptr;		/* pointer to character array */
REND		*rendptr;		/* pointer to rendition array */
EXT_REND	*ext_rendptr;		/* pointer to ext_rendition array */
int		i,j;			/* scratch */
int		top_margin, bottom_margin;
DECtermWidget   w = ld_to_w( ld );

    s_set_selection( w, 0, 0, CurrentTime, 0 );
    s_set_selection( w, 0, 0, CurrentTime, 1 );

    /*
    * If the top line is line 1, we're erasing the full width of the screen,
    * and the saveErasedLines resource is enabled, convert the erase to a
    * scroll into the transcript.
    */
    if ( topy == 1 && leftx == 1 && rightx == _ld wvt$l_column_width
      && w->common.saveErasedLines && ! (_cld wvt$l_vt200_common_flags &
      vtc1_m_actv_status_display ) )
	{
	if( _ld wvt$l_defer_count > 0 )
	    s_execute_deferred( ld );
	top_margin = _ld wvt$l_top_margin;
	bottom_margin = _ld wvt$l_bottom_margin;
	_ld wvt$l_top_margin = 1;
	_ld wvt$l_bottom_margin = boty;
	    WVT$UP_SCROLL(ld, 1, boty, 0);
			/* changing margins so we can't defer this */
	_ld wvt$l_top_margin = top_margin;
	_ld wvt$l_bottom_margin = bottom_margin;
	return;
	}

    /*
    * Blank the character array, Zero the rendition array
    */
    if (_ld wvt$a_code_cells != NULL)	/* if arrays are allocated... */
	for (i=topy;  i<=boty;  i++)	/* for each line cleared... */
	    {
	    charptr = &_ld wvt$a_code_base[i][leftx];
					/* get character array base */
	    rendptr = &_ld wvt$a_rend_base[i][leftx];
					/* get rendition array base */

	    ext_rendptr = &_ld wvt$a_ext_rend_base[i][leftx];
					/* get ext_rendition array base */

	    for (j=0;  j<=rightx-leftx;  j++)
		{
		*charptr++ = NULL;	/* blank the characters */
		*rendptr++ = NULL;	/* set rendition to plain */
		*ext_rendptr++ = NULL;	/* set ext_rendition to plain */
		}
	    };
    /*
     * Call output routines to clear the correct part of the window
     */
    if (leftx == 1 && rightx == _ld wvt$l_column_width) {
	o_erase_lines(w, moof(topy), moof(boty-topy+1));
    } else {
	for (i=topy; i<=boty; i++) {
		o_display_segment(w, moof(i), leftx-1, rightx-leftx+1);
	}
    }
}

WVT$ERASE_DISPLAY_LIST(ld)
wvtp		ld;
{
unsigned char	*charptr;		/* pointer to character array */
REND		*rendptr;		/* pointer to rendition array */
EXT_REND	*ext_rendptr;		/* pointer to ext_rendition array */
int		i,j,y;			/* scratch */
DECtermWidget   w = ld_to_w( ld );

    s_set_selection( w, 0, 0, CurrentTime, 0 );
    s_set_selection( w, 0, 0, CurrentTime, 1 );

    /*
    * Blank the character array, Zero the rendition array
    */
    if (_ld wvt$a_code_cells != NULL)	/* if arrays are allocated... */
	for (i=1;  i<=_ld wvt$l_page_length;  i++)  /* for each line cleared... */
	    {
	    charptr = &_ld wvt$a_code_base[i][1];/* get character array base */
	    rendptr = &_ld wvt$a_rend_base[i][1];/* get rendition array base */
	    ext_rendptr = &_ld wvt$a_ext_rend_base[i][1];
					/* get ext_rendition array base */
	    for (j=0;  j<=_ld wvt$l_column_width-1;  j++)
		{
		*charptr++ = NULL;		/* blank the characters */
		*rendptr++ = NULL;	/* set rendition to plain */
		*ext_rendptr++ = NULL;	/* set ext_rendition to plain */
		}
	    _ld wvt$b_rendits[i] = SINGLE_WIDTH;
	    };
    /*
     * Reset the line renditions to single width
     */
	for (y=1; y<=_ld wvt$l_page_length; y++)
            {
	    line_rendition(y) = SINGLE_WIDTH;
	    line_width(y) = _ld wvt$l_column_width;
            }
}

WVT$SCREEN_ALIGNMENT(ld)
wvtp		ld;			/* wvt structure pointer */
{
    s_fill_display(ld, 'E');
};

s_fill_display(ld, c)
wvtp		ld;			/* wvt structure pointer */
char c;
{
unsigned char	*charptr;		/* pointer to character array */
REND		*rendptr;		/* pointer to rendition array */
EXT_REND	*ext_rendptr;		/* pointer to ext_rendition array */
int		i,j;			/* scratch */
int		topy = 1;
int		boty = _ld wvt$l_page_length;
int		leftx = 1;
int		rightx = _ld wvt$l_column_width;
DECtermWidget w = ld_to_w( ld );

    /*
     * Reset the line renditions to single width
     */
    for (i=1; i<=_ld wvt$l_page_length; i++)
            {
	    line_rendition(i) = SINGLE_WIDTH;
	    line_width(i) = _ld wvt$l_column_width;
            }
    if (_ld wvt$a_code_cells != NULL)	/* if arrays are allocated... */
	for (i=topy;  i<=boty;  i++)	/* for each line cleared... */
	    {
	    charptr = &_ld wvt$a_code_base[i][leftx];/* get character array base */
	    rendptr = &_ld wvt$a_rend_base[i][leftx];/* get rendition array base */
	    ext_rendptr = &_ld wvt$a_ext_rend_base[i][leftx];/* get ext_rendition array base */
	    for (j=0;  j<=rightx-leftx;  j++)
		{
		*charptr++ = c;		/* blank the characters */
		*rendptr++ = csa_M_SELECTIVE_ERASE;
		*ext_rendptr++ = csa_M_SELECTIVE_ERASE;
		}
	    };
    /*
     * Call output routines to put E's in the correct part of the window
     */
	for (i=topy; i<=boty; i++) {
		o_display_segment(w, moof(i), leftx-1, rightx-leftx+1);
	}
};

WVT$ERASE_SCREEN(ld, topy, leftx, boty, rightx)
wvtp		ld;			/* wvt structure pointer */
int		topy;			/* top line to erase */
int		boty;			/* bottom line to erase */
int		leftx;
int		rightx;
{
int		i;
DECtermWidget w = ld_to_w( ld );

    s_set_selection( w, 0, 0, CurrentTime, 0 );
    s_set_selection( w, 0, 0, CurrentTime, 1 );

    /*
     * Call output routines to clear the correct part of the window
     */
    if (leftx == 1 && rightx == _ld wvt$l_column_width) {
	o_erase_lines(w, moof(topy), moof(boty-topy+1));
    } else {
	for (i=topy; i<=boty; i++) {
		o_display_segment(w, moof(i), leftx-1, rightx-leftx+1);
	}
    }
}

/* screen down */
WVT$DOWN_SCROLL(ld, topl, lcnt)
wvtp		ld;			/* wvt structure pointer */
int		topl;			/* top line of region */
int		lcnt;			/* number of lines to scroll down */
{
int		i;			/* scratch */
int		j;			/* scratch */
unsigned char	**codesave,		/* code vector temporary */
		*codesave_fast[100];
REND		**rendsave,		/* rend vector temporary */
		*rendsave_fast[100];
EXT_REND	**ext_rendsave,		/* ext_rend vector temporary */
		*ext_rendsave_fast[100];
int		botl = _ld wvt$l_bottom_margin; /* region bottom line number */
int		regs = botl-(topl-1); /* scroll region size */
DECtermWidget	w = ld_to_w( ld );


	/* check to see if scroll amount is bigger then region */
	if (botl - topl < lcnt) {
		WVT$ERASE_DISPLAY(ld, topl, 1, botl, _ld wvt$l_column_width);
		for ( i = topl; i <= botl; i++ ) {
		    line_width(i) = _ld wvt$l_column_width;
		    line_rendition(i) = SINGLE_WIDTH;
		}
		return;
	}
	/*
	* Now adjust the character/rendition arrays
	*/

	if ( lcnt <= 100 )
	    {
	    codesave = codesave_fast;
	    rendsave = rendsave_fast;
	    ext_rendsave = ext_rendsave_fast;
	    }
	else
	    {
	    codesave = (unsigned char **) XtMalloc(lcnt * sizeof ( *codesave ));
	    rendsave = (REND **) XtMalloc( lcnt * sizeof ( *rendsave ) );
	    ext_rendsave = (EXT_REND **) XtMalloc( lcnt * sizeof ( *ext_rendsave ) );
	    }

	   /*
	    * roll the wvt$a_code_base vector down
	    */
	    COPY(&codesave[0],	/* save bottom lines*/
		  &_ld wvt$a_code_base[botl-lcnt+1],
		  lcnt*sizeof(unsigned char *));

	    COPY(&_ld wvt$a_code_base[topl+lcnt],
    		  &_ld wvt$a_code_base[topl],		/* move lines down*/
		  (regs-lcnt) * sizeof(unsigned char *));

	    for ( i = 0; i < lcnt; i++ )		/* blank characters*/
		for (j=1; j <= _ld wvt$l_column_width; j++)
			codesave[i][j] = NULL;

	    COPY(&_ld wvt$a_code_base[topl],
		  &codesave[0],			/* reuse at top */
		  lcnt*sizeof(unsigned char *));

	    /*
	    * roll the rendition base vector
	    */
	    COPY(&rendsave[0],
		  &_ld wvt$a_rend_base[botl-lcnt+1],	/* save bottom lines*/
		  lcnt*sizeof(REND *));

	    COPY(&_ld wvt$a_rend_base[topl+lcnt],
		  &_ld wvt$a_rend_base[topl],		/* move lines down*/
		  (regs-lcnt)*sizeof(REND *));

	    for ( i = 0; i < lcnt; i++ )		/* blank renditions */
		for (j=1; j <= _ld wvt$l_column_width; j++)
			rendsave[i][j] = NULL;

	    COPY(&_ld wvt$a_rend_base[topl],
		  &rendsave[0],				/* reuse at top */
      		  lcnt*sizeof(REND *));

	    /*
	    * roll the ext_rendition base vector
	    */
	    COPY(&ext_rendsave[0],
		  &_ld wvt$a_ext_rend_base[botl-lcnt+1], /* save bottom lines*/
		  lcnt*sizeof(EXT_REND *));

	    COPY(&_ld wvt$a_ext_rend_base[topl+lcnt],
		  &_ld wvt$a_ext_rend_base[topl],	/* move lines down*/
		  (regs-lcnt)*sizeof(EXT_REND *));

	    for ( i = 0; i < lcnt; i++ )		/* blank renditions */
		for (j=1; j <= _ld wvt$l_column_width; j++)
			ext_rendsave[i][j] = NULL;

	    COPY(&_ld wvt$a_ext_rend_base[topl],
		  &ext_rendsave[0],			/* reuse at top */
		  lcnt*sizeof(EXT_REND *));

	    COPY(&_ld wvt$b_rendits[topl+lcnt],
		  &_ld wvt$b_rendits[topl],
		  lcnt * sizeof(unsigned char));

	    memset(&_ld wvt$b_rendits[topl],0,lcnt*sizeof(unsigned char));
		/* fill with null line rendition */

	    /*
	    * shift the column width vector filling with screen width
	    */
	    COPY(&_ld wvt$w_widths[topl+lcnt],
		    &_ld wvt$w_widths[topl],
		    (regs-lcnt)*sizeof(short) );
	    for ( i = 0; i < lcnt; i++ )
		_ld wvt$w_widths[topl+i] = _ld wvt$l_column_width;

	if ( codesave != codesave_fast )
	    {
	    XtFree( (char *)codesave );
	    XtFree( (char *)rendsave );
	    XtFree( (char *)ext_rendsave );
	    }

	/* Call output routines to adjust window */
	o_scroll_lines(w, -lcnt, moof(topl), regs);

	/* Scroll the selection, if necessary */
	scroll_selection( w, -lcnt, moof(topl), regs);

};

WVT$UP_SCROLL(ld, topl, lcnt, defer)
wvtp		ld;			/* wvt structure pointer */
int		topl;			/* top line of region */
int		lcnt;			/* number of lines to scroll up */
int		defer;			/* 1=internal, 2=window, 0=both */
{
int		i;			/* scratch */
int		j;			/* scratch */
unsigned char	**codesave,		/* code vector temporary */
		*codesave_fast[100];
REND		**rendsave,		/* rend vector temporary */
		*rendsave_fast[100];
EXT_REND	**ext_rendsave,		/* ext_rend vector temporary */
		*ext_rendsave_fast[100];
int		botl = _ld wvt$l_bottom_margin; /* region bottom line number */
int		regs;			/* scroll region size */
Boolean		saveErasedLines;	/* saved value of resource */

DECtermWidget w = ld_to_w( ld );

	/*
	* If we're scrolling above the top of the logical display, add the
	* new lines to the transcript.
	*/

	if ( !(_cld wvt$l_vt200_common_flags & vtc1_m_actv_status_display) &&
		topl == 1 && w->common.saveLinesOffTop
		&& _ld wvt$l_transcript_size > 0 ) {
	    if ( _ld wvt$l_transcript_top == 1)
		o_set_marker(w, 0, moof(1));
	    if ( defer == 2 )
		topl = _ld wvt$l_transcript_top;
	    else
		topl = _ld wvt$l_transcript_top - lcnt;
	    if ( topl < 1 - _ld wvt$l_transcript_size )
		topl = 1 - _ld wvt$l_transcript_size;
	    _ld wvt$l_transcript_top = topl;
	}		

	regs = botl-(topl-1); /* scroll region size */

	if (botl - topl < lcnt) {
	    /* temporarily disable saveErasedLines so we don't loop! */
	    saveErasedLines = w->common.saveErasedLines;
	    w->common.saveErasedLines = FALSE;
	    WVT$ERASE_DISPLAY(ld, topl, 1, botl,
	      _ld wvt$l_column_width);
	    for ( i = topl; i <= botl; i++ ) {
		line_width(i) = _ld wvt$l_column_width;
		line_rendition(i) = SINGLE_WIDTH;
	    }
	    w->common.saveErasedLines = saveErasedLines;
	    return;
	}

	/*
	* Now adjust the character/rendition arrays
	*/

	if ( lcnt <= 100 )
	    {
	    codesave = codesave_fast;
	    rendsave = rendsave_fast;
	    ext_rendsave = ext_rendsave_fast;
	    }
	else
	    {
	    codesave = (unsigned char **) XtMalloc(lcnt * sizeof( *codesave ));
	    rendsave = (REND **) XtMalloc( lcnt * sizeof( *rendsave ) );
	    ext_rendsave = (EXT_REND **) XtMalloc( lcnt * sizeof ( *ext_rendsave ) );
	    }

	    /*
	    * roll the wvt$a_code_base vector up
	    */
	if (defer != 2) {
	    COPY(&codesave[0],
		  &_ld wvt$a_code_base[topl],		/* save top lines */
		  lcnt*sizeof(unsigned char *));

	    COPY(&_ld wvt$a_code_base[topl],
		  &_ld wvt$a_code_base[topl+lcnt],	/* move lines up*/
		  (regs-lcnt)*sizeof(unsigned char *));

	    for ( i = 0; i < lcnt; i++ )		/* blank characters*/
		for (j=1;  j <= _ld wvt$l_column_width;  j++)
		    (codesave[i])[j]=NULL;

	    COPY(&_ld wvt$a_code_base[botl-lcnt+1],
		  &codesave[0],				/* reuse at bottom */
		  lcnt*sizeof(unsigned char *));

	    /*
	    * roll the rendition base vector
	    */
	    COPY(&rendsave[0],
		  &_ld wvt$a_rend_base[topl],		/* save top lines*/
		  lcnt*sizeof(REND *));

	    COPY(&_ld wvt$a_rend_base[topl],
		   &_ld wvt$a_rend_base[topl+lcnt],	/* move lines up*/
		  (regs-lcnt)*sizeof(REND *));

	    for ( i = 0; i < lcnt; i++ )		/* plain renditions */
		memset(rendsave[i]+1,0,_ld wvt$l_column_width * sizeof(REND));

	    COPY(&_ld wvt$a_rend_base[botl-lcnt+1],
		   &rendsave[0],			/* reuse at bottom */
		  lcnt*sizeof(REND *));

	    /*
	    * roll the ext_rendition base vector
	    */
	    COPY(&ext_rendsave[0],
		  &_ld wvt$a_ext_rend_base[topl],	/* save top lines*/
		  lcnt*sizeof(EXT_REND *));

	    COPY(&_ld wvt$a_ext_rend_base[topl],
		   &_ld wvt$a_ext_rend_base[topl+lcnt],	/* move lines up*/
		  (regs-lcnt)*sizeof(EXT_REND *));

	    for ( i = 0; i < lcnt; i++ )		/* plain renditions */
		memset(ext_rendsave[i]+1,0,_ld wvt$l_column_width * sizeof(EXT_REND));

	    COPY(&_ld wvt$a_ext_rend_base[botl-lcnt+1],
		   &ext_rendsave[0],			/* reuse at bottom */
		  lcnt*sizeof(EXT_REND *));

	    COPY(&_ld wvt$b_rendits[topl],
		   &_ld wvt$b_rendits[topl+lcnt],
		  (regs-lcnt) * sizeof(unsigned char));

	    memset(&_ld wvt$b_rendits[botl-lcnt+1],0,lcnt*sizeof(unsigned char));
		/* fill with null line rendition */

	    /*
	    * shift the column width vector filling with screen width
	    */
	    COPY(&_ld wvt$w_widths[topl],
		    &_ld wvt$w_widths[topl+lcnt],
		    (regs - lcnt)*sizeof(short) );

	    for ( i = 0; i < lcnt; i++ )
		_ld wvt$w_widths[botl-i] = _ld wvt$l_column_width;

	    if ( topl < 1 ) {
		o_set_top_line( w, moof(topl) );
	    }

} /* defer for internal display */

if (defer != 1) {    /* Call output routines to adjust window */
	o_scroll_lines(w, lcnt, moof(topl), regs);

	/* Scroll the selection, if necessary */
	scroll_selection( w, lcnt, moof(topl), regs);
}	

	    if ( codesave != codesave_fast )
		{
		XtFree( (char *)codesave );
		XtFree( (char *)rendsave );
		XtFree( (char *)ext_rendsave );
		}

};


WVT$LEFT_SCROLL(ld, leftc, ccnt)
wvtp		ld;			/* wvt structure pointer */
int		leftc;			/* left column */
int		ccnt;			/* number of columns to scroll left */
{
int		i;			/* scratch */
int		j;			/* scratch */
int		rightc = _ld wvt$l_right_margin; /* right column */
int		topl;
int		botl;
DECtermWidget w = ld_to_w( ld );

	topl = _ld wvt$l_top_margin;
	botl = _ld wvt$l_bottom_margin;

	/*
	* Now adjust the character/rendition arrays
	*/

	/*
	* roll the code_base vector up
	*/
	for (i = topl; i <= botl; i++ ) {
		for (j = leftc; j <= rightc; j++) {
			if (j+ccnt > rightc)
				_ld wvt$a_code_base[i][j] = NULL;
			else
			_ld wvt$a_code_base[i][j] = _ld wvt$a_code_base[i][j+ccnt];
		}
	}
	/*
	* roll the rendition base vector
	*/
	for (i = topl; i <= botl; i++ ) {
		for (j = leftc; j <= rightc; j++) {
			if (j+ccnt > rightc)
				_ld wvt$a_rend_base[i][j] = NULL;
			else
			_ld wvt$a_rend_base[i][j] = _ld wvt$a_rend_base[i][j+ccnt];
		}
	}
	/*
	* roll the ext_rendition base vector
	*/
	for (i = topl; i <= botl; i++ ) {
		for (j = leftc; j <= rightc; j++) {
			if (j+ccnt > rightc)
			    _ld wvt$a_ext_rend_base[i][j] = NULL;
			else
			    _ld wvt$a_ext_rend_base[i][j] = _ld wvt$a_ext_rend_base[i][j+ccnt]; 
		}
	}

	/* call output routines to update window */
	for (i=topl; i<=botl; i++) {
		o_display_segment(w, moof(i), 0, _ld wvt$w_widths[i]);
	}
};

WVT$RIGHT_SCROLL(ld, rightc, ccnt)
wvtp		ld;			/* wvt structure pointer */
int		rightc;			/* right column */
int		ccnt;			/* number of columns to scroll left */
{
int		i;			/* scratch */
int		j;			/* scratch */
int		leftc = _ld wvt$l_left_margin; /* left column */
int		topl;
int		botl;
DECtermWidget w = ld_to_w( ld );

	topl = _ld wvt$l_top_margin;
	botl = _ld wvt$l_bottom_margin;

	/*
	* Now adjust the character/rendition arrays
	*/

	/*
	* roll the code_base vector up
	*/
	for (i = topl; i <= botl; i++ ) {
		for (j = rightc; j >= leftc; j--) {
			if (j-ccnt < leftc)
				_ld wvt$a_code_base[i][j] = NULL;
			else
			_ld wvt$a_code_base[i][j] = _ld wvt$a_code_base[i][j-ccnt];
		}
	}
	/*
	* roll the rendition base vector
	*/
	for (i = topl; i <= botl; i++ ) {
		for (j = rightc; j >= leftc; j--) {
			if (j-ccnt < leftc)
				_ld wvt$a_rend_base[i][j] = NULL;
			else
			_ld wvt$a_rend_base[i][j] = _ld wvt$a_rend_base[i][j-ccnt];
		}
	}
	/*
	* roll the ext_rendition base vector
	*/
	for (i = topl; i <= botl; i++ ) {
		for (j = rightc; j >= leftc; j--) {
			if (j-ccnt < leftc)
			    _ld wvt$a_ext_rend_base[i][j] = NULL;
			else
			    _ld wvt$a_ext_rend_base[i][j] = _ld wvt$a_ext_rend_base[i][j-ccnt];
		}
	}

	/* call output routines to update window */
	for (i=topl; i<=botl; i++) {
		o_display_segment(w, moof(i), 0, _ld wvt$w_widths[i]);
	}
};

WVT$BELL(ld)
wvtp	ld;
{
DECtermWidget w = ld_to_w( ld );

	if ((vt4_m_bell|vt4_m_margin_bell) & _cld wvt$l_flags) {
		XBell(XtDisplay(w), 0);
	}
}

WVT$SET_KB_ATTRIBUTES(ld)
    wvtp ld;
{
    XKeyboardControl values;
    DECtermWidget w = ld_to_w( ld );

    w->common.autoRepeatEnable = ( _cld wvt$l_kb_attributes & UIS$M_KB_AUTORPT )
	? TRUE : FALSE;

    i_change_keyboard_control( w );
}

display_segment(ld, line, pos, numchars)
wvtp	ld;
int	line;
int	pos;
int	numchars;
{
DECtermWidget w = ld_to_w( ld );

        if ((_ld wvt$w_actv_rendition & csa_M_NODEFAULT_TEXT) ||
            (_ld wvt$w_actv_rendition & csa_M_NODEFAULT_TEXT_BCK)) {
                _ld wvt$b_disp_eol = FALSE;
                o_display_segment(w, moof(line), 0, _ld wvt$w_widths[line]);
        } else
	if (_ld wvt$b_disp_eol == TRUE) {
		_ld wvt$b_disp_eol = FALSE;
	    if (( _cld wvt$l_ext_flags & vte1_m_hebrew ) &&
		( _cld wvt$l_ext_specific_flags & vte2_m_rtl ))
		o_display_segment( w, moof(line), 0, pos );
	    else
		o_display_segment(w, moof(line), pos-1, _ld wvt$w_widths[line]-pos+1);
	} else {
	    if (( _cld wvt$l_ext_flags & vte1_m_hebrew ) &&
		( _cld wvt$l_ext_specific_flags & vte2_m_rtl ))
		o_display_segment( w, moof(line), pos+numchars, -numchars );
	    else
		o_display_segment( w, moof(line), pos-1, numchars );
	}
}

WVT$REFRESH_DISPLAY(ld, top, bot)
wvtp	ld;
int top, bot;
{
DECtermWidget w = ld_to_w( ld );
	register int i;

	for (i=top; i != bot+1; i++) {
		o_display_segment(w, moof(i), 0, _ld wvt$w_widths[i]);
	}
}

WVT$GET_TERMINAL_SETUP(ld)
wvtp	ld;
{
DECtermWidget w = ld_to_w( ld );

set_tab_stops(ld, 8);
s_set_value_misc( w, w );
s_set_value_cursor_blink( w, w);
s_set_value_terminalMode( w, w );
s_set_value_responseDA( w, w );
s_set_value_keyboardDialect( w, w );
s_set_value_userPreferenceSet( w, w );
s_set_value_screenMode( w, w );
s_set_value_statusDisplayEnable(w, w);
s_set_value_autoRepeatEnable( w, w );
s_set_value_jisRomanAsciiMode( w, w );
s_set_value_kanjiKatakanaMode( w, w );
s_set_value_kanji_78_83( w, w );
s_set_value_crm( w, w );
s_set_value_ksRomanAsciiMode( w, w );
}

set_tab_stops(ld, width)
wvtp	ld;
int	width;
{
	register int i;

	for (i=0; i <= MAX_COLUMN; i++) _cld wvt$b_tab_stops[i] = 0;
	if (width <= 0) return;
	for (i=width+1; i <= MAX_COLUMN; i+=width) _cld wvt$b_tab_stops[i] = 1;
}

static void
flush_source(w)
    DECtermWidget w;
{
    wvtp ld = w_to_ld( w );

    if ( ! _cld work_proc_registered )
	{
	_cld work_proc_id =
	  XtAppAddWorkProc(XtWidgetToApplicationContext((Widget)w),
			   s_work_proc, ld );
	_cld work_proc_registered = TRUE;
	}
}

static Boolean
s_work_proc( ld )
    wvtp ld;
{
    DECtermWidget w = ld_to_w( ld );

    /* Flush any undisplayed updates */

    if (_ld wvt$l_defer_count)	/* if we have been defering up scrolls */
	{
	 s_execute_deferred( ld );
	}
    else
	{
	 if ((_ld wvt$l_actv_column - _ld wvt$l_disp_pos) || _ld wvt$b_disp_eol)
		display_segment(ld, _ld wvt$l_actv_line, _ld wvt$l_disp_pos,
				    _ld wvt$l_actv_column - _ld wvt$l_disp_pos);
	}

    _ld wvt$l_disp_pos = _ld wvt$l_actv_column;
    if (( _cld wvt$l_ext_flags & vte1_m_dickcat ) && XtIsRealized( w )) {
        o_set_cursor_position(w, moof(_ld wvt$l_actv_line),
			  _ld wvt$l_actv_column-1);
	w->source.wvt$l_ext_specific_flags |= vte2_m_intermediate_char;
	if ( w->input.string16[0].byte1 || w->input.string16[0].byte2 ) {
	    o_draw_intermediate_char( w, w->input.string16 );
	    w->input.string16[0].byte1 = 0;
	    w->input.string16[0].byte2 = 0;
	}
    } else
    o_set_cursor_position(w, moof(_ld wvt$l_actv_line), _ld wvt$l_actv_column-1);
    o_cursor_couple( w );
    o_enable_cursor(w, CURSOR_DISABLED_SOURCE);
    _cld work_proc_registered = FALSE;
    return TRUE;
}

/*
 * DECwTermFlushOutput
 *
 * Calling this routine ensures that the data displayed in the window
 * reflects the current contents of the display list.  Currently, this
 * is done by calling the work procedure.
 */

void DECwTermFlushOutput( w )
    DECtermWidget w;
{
    s_work_proc( w );
}

/*
 * The following routine transmits the answerback message, translating
 *
 *	#nn
 *
 * to the character of the specified 2-DIGIT hex code.  For example,
 *
 *	#0D
 *
 * means carriage return.
 *
 * Also, if two # chars are seen, like this:
 *
 *	##
 *
 * then a single # is transmitted.
 *
 * If anything other than a valid 2-digit hex code is detected after a #,
 * then the # is treated as a normal text character.
 */
transmit_answerback(ld, text)
wvtp	ld;
char *text;
{
char *ptr = text;			/* running pointer to text */
char *reg;				/* pointer to contiguous regular text */
int n_reg;				/* number of regular text chars */
char *beyond = &text[strlen (text)];	/* pointer to just beyond text */
DECtermWidget w = ld_to_w( ld );	/* widget to write to */
char q = '#';				/* quoting character */
char hex_str[3];			/* room for 2 hex digits and null */
int hex;				/* int char for sscanf output */
int status;

hex_str[2] = 0;				/* put null at end of hex string */
n_reg = 0;
reg = text;
while (*ptr)
    if (*ptr != q)
	n_reg++, ptr++;
    else if (beyond - ptr > 1 && *(ptr+1) == q)
	{				/* we see ## */
	if (n_reg) i_report_data (w, reg, n_reg);	/* send the regs */
	n_reg = 0;			/* no more regs yet */
	i_report_data (w, &q, 1);	/* send the # */
	reg = (ptr += 2);		/* advance beyond the ## */
	}
    else if (beyond - ptr > 2)
	{				/* we see # and at least 2 more chars */
	if (n_reg) i_report_data (w, reg, n_reg);	/* send the regs */
	n_reg = 0;			/* no more regs yet */
	strncpy (hex_str, ptr+1, 2);	/* isolate the hex digits */
	status = sscanf (hex_str, "%x", &hex); /* calculate the character */
	if (status != 1)		/* legal hex number ? */
	    {				/* no */
	    i_report_data (w, &q, 1);	/* bad number, send the # */
	    i_report_data (w, hex_str, 2); /* send the bad characters */
	    reg = (ptr += 3);		/* advance beyond the #nn */
	    }
	else				/* yes, legal number */
	    {
	    i_report_data (w, &hex, 1); /* send the special character */
	    reg = (ptr += 3);		/* advance beyond the #nn */
	    }
	}
    else
	{	    	    		/* we see # but not 2 chars after */
	n_reg += beyond - ptr;		/* accumulate rest into regs */
	i_report_data (w, reg, n_reg);	/* send the regs */
	n_reg = 0;			/* no more regs anymore */
	break;				/* we've sent everything */
	}

if (n_reg) i_report_data (w, reg, n_reg); /* send rest of regs */
}

WVT$TERMINAL_TRANSMIT(ld, text)
wvtp	ld;
char *text;
{
DECtermWidget w = ld_to_w( ld );

	i_report_data(w, text, strlen(text));
}

WVT$CURSOR_POSITION_REPORT(ld, y, x)
wvtp	ld;
short y;
short x;
{
	char	report[50];

	/* 7bit/8bit mode	*/
        if ( !(_cld wvt$b_conformance_level > LEVEL1 &&
            (_cld wvt$l_vt200_common_flags & vtc1_m_c1_transmission_mode)))
          sprintf(report, "\033[%d;%dR", y, x);
	else
	sprintf(report, "\233%d;%d;%dR", y, x, _cld current_page + 1);
	WVT$TERMINAL_TRANSMIT(ld, report);
}

WVT$REVERSE_VIDEO(ld)
wvtp	ld;
{
DECtermWidget w = ld_to_w( ld );

	w->common.screenMode =
	    (_cld wvt$l_vt200_common_flags ^= vtc1_m_screen_mode) & vtc1_m_screen_mode
		? TRUE : FALSE;
	w->common.reverseVideo = TRUE;
	o_set_value_reverseVideo( w, w );
}

WVT$SET_TERMINAL_SIZE(ld, columns, rows, conforming)
wvtp	ld;
int columns, rows;
unsigned char conforming;
{
	DECtermWidget w = ld_to_w( ld );
	int old_rows = _mld wvt$l_page_length,
	    old_columns = _mld wvt$l_column_width,
	    in_status;

	in_status = ( _cld wvt$l_vt200_common_flags
		& vtc1_m_actv_status_display );

	if (columns > MAX_COLUMN-1) columns = MAX_COLUMN-1;
	if (rows > MAX_LINE-1) rows = MAX_LINE-1;

	/* update resource values if not already updated */
	w->common.rows = rows;
	w->common.columns = columns;

	WVT$MAIN_DISPLAY(ld);
	if ( rows == _ld wvt$l_page_length &&
	     columns == _ld wvt$l_column_width) {
		/* no need to reallocate display list, size not changing */
		if ( conforming == 1 ) {
			s_clear_display(w);
		     _ld wvt$l_top_margin = 1;
		     _ld wvt$l_bottom_margin = rows;
		     _ld wvt$l_vt200_specific_flags &= ~(vts1_m_last_column);
			/* conforming resize clears display */
		     }

		if ( in_status )
		    WVT$STATUS_DISPLAY( ld );

		return;
		}

	if ( !xresize( ld, columns, rows, _ld wvt$l_transcript_size,
		conforming ) )
	    {
	    w->common.rows = old_rows;
	    w->common.columns = old_columns;
	    if ( ! in_status )
		WVT$MAIN_DISPLAY( ld );
	    return;
	    }

	WVT$STATUS_DISPLAY( ld );
	xresize( ld, columns, 1, 0, conforming );

/*
 * Inform the output module that the size has changed
 */

	if ( rows != old_rows )
	    o_set_bottom_line( w, rows );

	if ( columns != old_columns )
	    o_set_display_width( w, columns );

    /* reset the scrolling region */

     _mld wvt$l_top_margin = 1;
     _mld wvt$l_bottom_margin = rows;
     _mld wvt$l_vt200_specific_flags &= ~(vts1_m_last_column);

	if (conforming == 1) {
	    _mld wvt$l_actv_line = 1;
	    _mld wvt$l_actv_column = 1;
	    o_set_cursor_position( w, moof(1), 0);
	}
          
	if ( ! in_status )
	    WVT$MAIN_DISPLAY( ld );
}
                              
WVT$SET_MISC_MODES( ld )
    wvtp ld;
{
    DECtermWidget w = ld_to_w( ld );

#define C_SET_RESOURCE( res, loc, flag )                \
	w->common.res = (_cld loc & flag) ? TRUE : FALSE;        
#define C_SET_RESOURCE_N( res, loc, flag )                \
	w->common.res = (_cld loc & flag) ? FALSE : TRUE;        
#define SET_RESOURCE( res, loc, flag )			\
	w->common.res = (_mld loc & flag) ? TRUE : FALSE;
#define SET_RESOURCE_N( res, loc, flag )		\
	w->common.res = (_mld loc & flag) ? FALSE : TRUE;

	C_SET_RESOURCE( lockUDK,
	    wvt$l_vt200_common_flags, vtc1_m_lock_set )
	C_SET_RESOURCE( lockUserFeatures,
	    wvt$l_vt200_common_flags, vtc1_m_feature_lock )
	C_SET_RESOURCE( marginBellEnable,
	    wvt$l_flags, vt4_m_margin_bell )
	C_SET_RESOURCE( warningBellEnable,
	    wvt$l_flags, vt4_m_bell )
	C_SET_RESOURCE( newLineMode,
	    wvt$l_vt200_flags, vt1_m_new_line_mode )
	SET_RESOURCE( autoWrapEnable,
	    wvt$l_vt200_specific_flags, vts1_m_auto_wrap_mode )
	C_SET_RESOURCE( applicationKeypadMode,
	    wvt$l_vt200_common_flags, vtc1_m_keypad_mode )
	C_SET_RESOURCE( applicationCursorKeyMode,
	    wvt$l_vt200_common_flags, vtc1_m_cursor_key_mode )
	C_SET_RESOURCE_N( eightBitCharacters,
	    wvt$l_vt200_flags, vt1_m_nrc_mode )
	C_SET_RESOURCE( printFormFeedMode,
	    wvt$w_print_flags, pf1_m_prt_ff_mode )
	C_SET_RESOURCE_N( leadingCodeEnable,
	    wvt$l_ext_specific_flags, vte2_m_leading_code_mode )
	C_SET_RESOURCE( rightToLeft,
	    wvt$l_ext_specific_flags, vte2_m_rtl )
	C_SET_RESOURCE( selectionRtoL,
	    wvt$l_ext_specific_flags, vte2_m_copy_dir )
}

s_set_value_columns_rows( oldw, neww )
    DECtermWidget oldw, neww;
{
    wvtp ld = w_to_ld( neww );

	WVT$SET_TERMINAL_SIZE(ld, neww->common.columns, neww->common.rows, 0);
	o_set_cursor_position(neww, moof(_ld wvt$l_actv_line),
	  _ld wvt$l_actv_column-1);
}

s_set_value_screenMode( oldw, neww )
    DECtermWidget oldw, neww;
{
    wvtp ld = w_to_ld( neww );

    if (neww->common.screenMode)
	_cld wvt$l_vt200_common_flags |= vtc1_m_screen_mode;
    else
	_cld wvt$l_vt200_common_flags &= ~vtc1_m_screen_mode;
}

s_set_value_statusDisplayEnable( oldw, neww )
    DECtermWidget oldw, neww;
{
    wvtp ld = w_to_ld( neww );

    WVT$MAIN_DISPLAY(ld);
    o_set_value_statusDisplayEnable( oldw, neww );
    if ( ! neww->common.statusDisplayEnable )
	{
	WVT$STATUS_DISPLAY(ld);
	WVT$ERASE_DISPLAY( ld, 1, 1, 1, _ld wvt$l_column_width );
	WVT$MAIN_DISPLAY(ld);
	}
    flush_source( neww );
}

s_set_value_saveLinesOffTop( oldw, neww )
    DECtermWidget oldw, neww;
{
    wvtp ld = w_to_ld( neww );
    int in_status = ( _cld wvt$l_vt200_common_flags
		& vtc1_m_actv_status_display );

    WVT$MAIN_DISPLAY( ld );
    xresize( ld, _ld wvt$l_column_width, _ld wvt$l_page_length,
		neww->common.saveLinesOffTop ? neww->common.transcriptSize : 0,
		0 );
    o_set_top_line( ld, _ld wvt$l_transcript_top );
    if ( in_status )
	WVT$STATUS_DISPLAY( ld );
}
	    	
s_set_value_userPreferenceSet( oldw, neww )      
    DECtermWidget oldw, neww;
{
    wvtp ld = w_to_ld( neww );


    switch (neww->common.userPreferenceSet)
	{
	case DECwISO_Latin1_Supplemental :
	    _cld wvt$b_user_preference_set = ISO_LATIN_1;
	    break;

	case DECwDEC_Hebrew_Supplemental :
	    _cld wvt$b_user_preference_set = HEB_SUPPLEMENTAL;
	    break;

	case DECwISO_Latin8_Supplemental :
	    _cld wvt$b_user_preference_set = ISO_LATIN_8;
	    break;

	case DECwDEC_Turkish_Supplemental :
	    _cld wvt$b_user_preference_set = TURKISH_SUPPLEMENTAL;
	    break;

	case DECwISO_Latin5_Supplemental :
	    _cld wvt$b_user_preference_set = ISO_LATIN_5;
	    break;

	case DECwDEC_Greek_Supplemental :
	    _cld wvt$b_user_preference_set = GREEK_SUPPLEMENTAL;
	    break;

	case DECwISO_Latin7_Supplemental :
	    _cld wvt$b_user_preference_set = ISO_LATIN_7;
	    break;

	default :	/* SUPPLEMENTAL will be a default */
	    _cld wvt$b_user_preference_set = SUPPLEMENTAL;
	    break;
	}

    set_character_sets(ld);
}

s_set_value_responseDA( oldw, neww )
    DECtermWidget oldw, neww;
{
    wvtp ld = w_to_ld( neww );

	switch ( neww->common.responseDA )
	    {
	    case DECwVT100_ID:
	    	_cld wvt$b_alt_term_id = VT100_ID;
	    	break;
	    case DECwVT101_ID:
	    	_cld wvt$b_alt_term_id = VT101_ID;
	    	break;
	    case DECwVT102_ID:
	    	_cld wvt$b_alt_term_id = VT102_ID;
	    	break;
	    case DECwVT125_ID:
	    	_cld wvt$b_alt_term_id = VT125_ID;
	    	break;
	    case DECwVT220_ID:
	    	_cld wvt$b_alt_term_id = VT220_ID;
	    	break;
	    case DECwVT240_ID:
	    	_cld wvt$b_alt_term_id = VT240_ID;
	    	break;
	    case DECwVT320_ID:
	    	_cld wvt$b_alt_term_id = VT320_ID;
	    	break;
	    case DECwVT330_ID:
	    	_cld wvt$b_alt_term_id = VT330_ID;
	    	break;
	    case DECwVT340_ID:
	    	_cld wvt$b_alt_term_id = VT340_ID;
	    case DECwVT80_ID:
	    	_cld wvt$b_alt_term_id = VT80_ID;
	    	break;
	    case DECwVT100J_ID:
	    	_cld wvt$b_alt_term_id = VT100J_ID;
	    	break;
	    case DECwVT102J_ID:
	    	_cld wvt$b_alt_term_id = VT102J_ID;
	    	break;
	    case DECwVT220J_ID:
	    	_cld wvt$b_alt_term_id = VT220J_ID;
	    	break;
	    case DECwVT282_ID:
	    	_cld wvt$b_alt_term_id = VT282_ID;
	    	break;
	    case DECwVT284_ID:
	    	_cld wvt$b_alt_term_id = VT284_ID;
	    	break;
	    case DECwVT286_ID:
	    	_cld wvt$b_alt_term_id = VT286_ID;
	    	break;
	    case DECwVT382_ID:
	    	_cld wvt$b_alt_term_id = VT382_ID;
	    	break;
	    case DECwVT382CB_ID:
	    	_cld wvt$b_alt_term_id = VT382CB_ID;
	    	break;
	    case DECwVT382K_ID:
	    	_cld wvt$b_alt_term_id = VT382K_ID;
	    	break;
	    case DECwVT382D_ID:
	    	_cld wvt$b_alt_term_id = VT382D_ID;
	    	break;
	    case DECwDECtermID:
	    default:
	    	_cld wvt$b_alt_term_id = DECTERM_ID;
	    	break;
	    }
}

WVT$SET_TERMINAL_MODE( ld )
wvtp ld;
{
    DECtermWidget   w = ld_to_w( ld );

    if (_cld wvt$l_vt200_flags & vt1_m_ansi_mode) {
	switch (_cld wvt$b_conformance_level) {
		case LEVEL1:
			w->common.terminalMode = DECwVT100_Mode;
			break;
		case LEVEL2: /* The architecture folks decided not to have a
				vt200 mode in set-up on the vt300 series
				terminals.  So, if the terminal is in level 2
				mode, lie to the user and tell him that it is
				in level 3 mode */
		case LEVEL3:
			if (_cld wvt$l_vt200_common_flags & vtc1_m_c1_transmission_mode)
				w->common.terminalMode = DECwVT300_8_BitMode;
			else
				w->common.terminalMode = DECwVT300_7_BitMode;
			break;
	}
  } else w->common.terminalMode = DECwVT52_Mode;
}

s_set_value_terminalMode( oldw, neww )
    DECtermWidget oldw, neww;
{
    wvtp ld = w_to_ld( neww );

    Boolean need_str = TRUE;

	switch ( neww->common.terminalMode ) {
		case DECwVT52_Mode :
			_cld wvt$l_vt200_flags &= ~(vt1_m_ansi_mode|vt1_m_vt52_cursor_seq);
			_cld wvt$l_vt200_common_flags &= ~vtc1_m_c1_transmission_mode;
			_cld wvt$b_conformance_level = LEVEL1;
			break;
		case DECwVT100_Mode :
			_cld wvt$l_vt200_flags |= vt1_m_ansi_mode;
			_cld wvt$l_vt200_common_flags &= ~vtc1_m_c1_transmission_mode;
			_cld wvt$b_conformance_level = LEVEL1;
			break;
		case DECwVT300_7_BitMode :
			if ( _cld wvt$b_conformance_level == LEVEL3 )
			    need_str = FALSE;
			_cld wvt$l_vt200_flags |= vt1_m_ansi_mode;
			_cld wvt$l_vt200_common_flags &= ~vtc1_m_c1_transmission_mode;
			_cld wvt$b_conformance_level = LEVEL3;
			break;
		case DECwVT300_8_BitMode :
			if ( _cld wvt$b_conformance_level == LEVEL3 )
			    need_str = FALSE;
			_cld wvt$l_vt200_flags |= vt1_m_ansi_mode;
			_cld wvt$l_vt200_common_flags |= vtc1_m_c1_transmission_mode;
			_cld wvt$b_conformance_level = LEVEL3;
			break;
	}
	if ( need_str ) {
	    set_character_sets(ld);
	}
}

static void set_character_sets(ld)
wvtp ld;
{
    Boolean in_status_line;

    in_status_line = _cld wvt$l_vt200_common_flags & vtc1_m_actv_status_display;

/* this code is taken from xdisplaystr.  We can't do a real xstr because it
   will reset user-settable features */

	WVT$STATUS_DISPLAY(ld);
	if ( _cld wvt$l_ext_flags & vte1_m_tomcat )
	{
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

	if ( _cld wvt$l_ext_specific_flags & vte2_m_kanji_mode &&
             _cld wvt$l_vt200_flags & vt1_m_ansi_mode ) {
	  _ld wvt$b_gr = _ld wvt$b_sav_gr = 3;
	  _ld wvt$b_save_g_sets[1]  = _ld wvt$b_g_sets[1] = LINE_DRAWING;
	  _ld wvt$b_save_ext_g_sets[1]  = _ld wvt$b_ext_g_sets[1] = STANDARD_SET;
	  _ld wvt$b_save_g_sets[3]  = _ld wvt$b_g_sets[3] = DEC_KANJI;
	  _ld wvt$b_save_ext_g_sets[3]  = _ld wvt$b_ext_g_sets[3] = TWO_BYTE_SET;
	}
	} else if ( _cld wvt$l_ext_flags & vte1_m_asian_common ) {
	/* for bobcat, dickcat & fishcat only */
	  _ld wvt$b_gl = _ld wvt$b_sav_gl = 0;
	  _ld wvt$b_gr = _ld wvt$b_sav_gr = 3;
	  _ld wvt$b_save_g_sets[0] = _ld wvt$b_g_sets[0] = ASCII;
	  _ld wvt$b_save_g_sets[1] = _ld wvt$b_g_sets[1] = LINE_DRAWING;
	  _ld wvt$b_save_ext_g_sets[0] = _ld wvt$b_ext_g_sets[0] =
	  _ld wvt$b_save_ext_g_sets[1] = _ld wvt$b_ext_g_sets[1] =
							   STANDARD_SET;
	  if ( _cld wvt$l_ext_flags & vte1_m_bobcat ) {
	    _ld wvt$b_save_g_sets[2] = _ld wvt$b_g_sets[2] = ASCII;
	    _ld wvt$b_save_g_sets[3] = _ld wvt$b_g_sets[3] = DEC_HANZI;
	    _ld wvt$b_save_ext_g_sets[2] = _ld wvt$b_ext_g_sets[2] =
							     STANDARD_SET;
	    _ld wvt$b_save_ext_g_sets[3] = _ld wvt$b_ext_g_sets[3] =
							     TWO_BYTE_SET;
	  } else if ( _cld wvt$l_ext_flags & vte1_m_dickcat ) {
	    if ( _cld wvt$l_ext_specific_flags & vte2_m_ksroman_mode ) {
		_ld wvt$b_save_g_sets[0] = _ld wvt$b_g_sets[0] = KS_ROMAN;
		_ld wvt$b_save_ext_g_sets[0] =
		_ld wvt$b_ext_g_sets[0] = ONE_BYTE_SET;
	    }
	    _ld wvt$b_save_g_sets[2] = _ld wvt$b_g_sets[2] = DEC_HANGUL;
	    _ld wvt$b_save_g_sets[3] = _ld wvt$b_g_sets[3] = DEC_HANGUL;
	    _ld wvt$b_save_ext_g_sets[2] = _ld wvt$b_ext_g_sets[2] =
	    _ld wvt$b_save_ext_g_sets[3] = _ld wvt$b_ext_g_sets[3] =
							     TWO_BYTE_SET;
	  } else if ( _cld wvt$l_ext_flags & vte1_m_fishcat ) {
	    _ld wvt$b_save_g_sets[2] = _ld wvt$b_g_sets[2] = ASCII;
	    _ld wvt$b_save_g_sets[3] = _ld wvt$b_g_sets[3] = DEC_HANGUL;
	    _ld wvt$b_save_ext_g_sets[2] = _ld wvt$b_ext_g_sets[2] =
							     STANDARD_SET;
	    _ld wvt$b_save_ext_g_sets[3] = _ld wvt$b_ext_g_sets[3] =
							     FOUR_BYTE_SET;

	  }		
	} else {
	_ld wvt$b_save_ext_g_sets[0]  = _ld wvt$b_ext_g_sets[0] =
	_ld wvt$b_save_ext_g_sets[1]  = _ld wvt$b_ext_g_sets[1] =
	_ld wvt$b_save_ext_g_sets[2]  = _ld wvt$b_ext_g_sets[2] =
	_ld wvt$b_save_ext_g_sets[3]  = _ld wvt$b_ext_g_sets[3] = STANDARD_SET;
	_ld wvt$b_sav_gl = _ld wvt$b_gl = 0;
	_ld wvt$b_sav_gr = _ld wvt$b_gr = 2;

	_ld wvt$b_save_g_sets[0]  = _ld wvt$b_g_sets[0] =
	_ld wvt$b_save_g_sets[1]  = _ld wvt$b_g_sets[1] = ASCII;

	if (_cld wvt$b_conformance_level >= LEVEL2) {
	      _ld wvt$b_save_g_sets[2]  = _ld wvt$b_save_g_sets[3] =
	      _ld wvt$b_g_sets[2]       = _ld wvt$b_g_sets[3]      =
		      _cld wvt$b_user_preference_set;
	    if (( _cld wvt$b_user_preference_set == ISO_LATIN_8 ) ||
		( _cld wvt$b_user_preference_set == HEB_SUPPLEMENTAL )) {
		_ld wvt$b_save_ext_g_sets[2] = _ld wvt$b_ext_g_sets[2] =
		_ld wvt$b_save_ext_g_sets[3] = _ld wvt$b_ext_g_sets[3] =
		ONE_BYTE_SET;
	    }
	} else {
	      _ld wvt$b_save_g_sets[2]  = _ld wvt$b_g_sets[2] =
	      _ld wvt$b_save_g_sets[3]  = _ld wvt$b_g_sets[3] = ASCII;
	}
	}

	WVT$MAIN_DISPLAY(ld);
	if ( _cld wvt$l_ext_flags & vte1_m_tomcat )
	{
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

	if ( _cld wvt$l_ext_specific_flags & vte2_m_kanji_mode &&
             _cld wvt$l_vt200_flags & vt1_m_ansi_mode ) {
	  _ld wvt$b_gr = _ld wvt$b_sav_gr = 3;
	  _ld wvt$b_save_g_sets[1]  = _ld wvt$b_g_sets[1] = LINE_DRAWING;
	  _ld wvt$b_save_ext_g_sets[1]  = _ld wvt$b_ext_g_sets[1] = STANDARD_SET;
	  _ld wvt$b_save_g_sets[3]  = _ld wvt$b_g_sets[3] = DEC_KANJI;
	  _ld wvt$b_save_ext_g_sets[3]  = _ld wvt$b_ext_g_sets[3] = TWO_BYTE_SET;
	}
	} else if ( _cld wvt$l_ext_flags & vte1_m_asian_common ) {
	/* for bobcat, dickcat & fishcat only */
	  _ld wvt$b_gl = _ld wvt$b_sav_gl = 0;
	  _ld wvt$b_gr = _ld wvt$b_sav_gr = 3;
	  _ld wvt$b_save_g_sets[0] = _ld wvt$b_g_sets[0] = ASCII;
	  _ld wvt$b_save_g_sets[1] = _ld wvt$b_g_sets[1] = LINE_DRAWING;
	  _ld wvt$b_save_ext_g_sets[0] = _ld wvt$b_ext_g_sets[0] =
	  _ld wvt$b_save_ext_g_sets[1] = _ld wvt$b_ext_g_sets[1] =
							   STANDARD_SET;
	  if ( _cld wvt$l_ext_flags & vte1_m_bobcat ) {
	    _ld wvt$b_save_g_sets[2] = _ld wvt$b_g_sets[2] = ASCII;
	    _ld wvt$b_save_g_sets[3] = _ld wvt$b_g_sets[3] = DEC_HANZI;
	    _ld wvt$b_save_ext_g_sets[2] = _ld wvt$b_ext_g_sets[2] =
							     STANDARD_SET;
	    _ld wvt$b_save_ext_g_sets[3] = _ld wvt$b_ext_g_sets[3] =
							     TWO_BYTE_SET;
	  } else if ( _cld wvt$l_ext_flags & vte1_m_dickcat ) {
	    if ( _cld wvt$l_ext_specific_flags & vte2_m_ksroman_mode ) {
		_ld wvt$b_save_g_sets[0] = _ld wvt$b_g_sets[0] = KS_ROMAN;
		_ld wvt$b_save_ext_g_sets[0] =
		_ld wvt$b_ext_g_sets[0] = ONE_BYTE_SET;
	    }
	    _ld wvt$b_save_g_sets[2] = _ld wvt$b_g_sets[2] = DEC_HANGUL;
	    _ld wvt$b_save_g_sets[3] = _ld wvt$b_g_sets[3] = DEC_HANGUL;
	    _ld wvt$b_save_ext_g_sets[2] = _ld wvt$b_ext_g_sets[2] =
	    _ld wvt$b_save_ext_g_sets[3] = _ld wvt$b_ext_g_sets[3] =
							     TWO_BYTE_SET;
	  } else if ( _cld wvt$l_ext_flags & vte1_m_fishcat ) {
	    _ld wvt$b_save_g_sets[2] = _ld wvt$b_g_sets[2] = ASCII;
	    _ld wvt$b_save_g_sets[3] = _ld wvt$b_g_sets[3] = DEC_HANGUL;
	    _ld wvt$b_save_ext_g_sets[2] = _ld wvt$b_ext_g_sets[2] =
							     STANDARD_SET;
	    _ld wvt$b_save_ext_g_sets[3] = _ld wvt$b_ext_g_sets[3] =
							     FOUR_BYTE_SET;

	  }		
	} else {
	_ld wvt$b_save_ext_g_sets[0]  = _ld wvt$b_ext_g_sets[0] =
	_ld wvt$b_save_ext_g_sets[1]  = _ld wvt$b_ext_g_sets[1] =
	_ld wvt$b_save_ext_g_sets[2]  = _ld wvt$b_ext_g_sets[2] =
	_ld wvt$b_save_ext_g_sets[3]  = _ld wvt$b_ext_g_sets[3] = STANDARD_SET;
	_ld wvt$b_sav_gl = _ld wvt$b_gl = 0;
	_ld wvt$b_sav_gr = _ld wvt$b_gr = 2;

	_ld wvt$b_save_g_sets[0]  = _ld wvt$b_g_sets[0] =
	_ld wvt$b_save_g_sets[1]  = _ld wvt$b_g_sets[1] = ASCII;

	if (_cld wvt$b_conformance_level >= LEVEL2) {
	      _ld wvt$b_save_g_sets[2]  = _ld wvt$b_save_g_sets[3] =
	      _ld wvt$b_g_sets[2]       = _ld wvt$b_g_sets[3]      =
		      _cld wvt$b_user_preference_set;
	    if (( _cld wvt$b_user_preference_set == ISO_LATIN_8 ) ||
		( _cld wvt$b_user_preference_set == HEB_SUPPLEMENTAL )) {
		_ld wvt$b_save_ext_g_sets[2] = _ld wvt$b_ext_g_sets[2] =
		_ld wvt$b_save_ext_g_sets[3] = _ld wvt$b_ext_g_sets[3] =
		ONE_BYTE_SET;
	    }
	} else {
	      _ld wvt$b_save_g_sets[2]  = _ld wvt$b_g_sets[2] =
	      _ld wvt$b_save_g_sets[3]  = _ld wvt$b_g_sets[3] = ASCII;
	}
	}

    if (in_status_line) WVT$STATUS_DISPLAY(ld);
      else WVT$MAIN_DISPLAY(ld);
}

s_set_value_keyboardDialect( oldw, neww )
    DECtermWidget oldw, neww;
{
    wvtp ld = w_to_ld( neww );

    switch( neww->common.keyboardDialect )
	{
	case DECwNorthAmericanDialect:
	case DECwDutchDialect:
	    _mld wvt$b_nrc_set = 0;	/* ASCII */
	    break;
	case DECwFlemishDialect:
	case DECwBelgianFrenchDialect:
	    _mld wvt$b_nrc_set = 14;	/* French */
	    break;
	case DECwCanadianFrenchDialect:
	    _mld wvt$b_nrc_set = 4;	/* Canadian */
	    break;
	case DECwBritishDialect:
	    _mld wvt$b_nrc_set = 2;	/* United Kingdom */
	    break;
	case DECwDanishDialect:
	case DECwNorwegianDialect:
	    _mld wvt$b_nrc_set = 5;	/* Norwegian/Danish */
	    break;
	case DECwFinnishDialect:
	    _mld wvt$b_nrc_set = 6;	/* Finnish */
	    break;
	case DECwAustrianGermanDialect:
	    _mld wvt$b_nrc_set = 7;	/* German */
	    break;
	case DECwItalianDialect:
	    _mld wvt$b_nrc_set = 9;	/* Italian */
	    break;
	case DECwSwissFrenchDialect:
	case DECwSwissGermanDialect:
	    _mld wvt$b_nrc_set = 10;	/* Swiss */
	    break;
	case DECwSwedishDialect:
	    _mld wvt$b_nrc_set = 12;	/* Swedish */
	    break;
	case DECwSpanishDialect:
	    _mld wvt$b_nrc_set = 15;	/* Spanish */
	    break;
	case DECwPortugueseDialect:
	    _mld wvt$b_nrc_set = 16;	/* Portuguese */
	    break;
	case DECwHebrewDialect:
	    _mld wvt$b_nrc_set = 17;	/* Hebrew */
	    break;

/* Turkish' and Greek's NRCS support is not available to users yet. */

	case DECwTurkishDialect:
	    _mld wvt$b_nrc_set = 18;	/* Turkish */
	    break;

	case DECwGreekDialect:
	    _mld wvt$b_nrc_set = 19;	/* Greek */
	    break;
	}
}

s_set_value_eightBitCharacters( oldw, neww )
    DECtermWidget oldw, neww;
{
    wvtp ld = w_to_ld( neww );

    if ( neww->common.eightBitCharacters )
	{
	_cld wvt$l_vt200_flags &= ~vt1_m_nrc_mode;
	/* Clear User Preferred Supplemental flag */
        _mld wvt$b_ups = 0;
	if ( !( _cld wvt$l_ext_flags & vte1_m_hebrew )) {
	    _mld wvt$b_g_sets[ 0 ] = _mld wvt$b_g_sets[ 1 ] = ASCII;
	    _mld wvt$b_g_sets[ 2 ] = _mld wvt$b_g_sets[ 3 ] = SUPPLEMENTAL;
	    _mld wvt$b_ext_g_sets[ 0 ] = _mld wvt$b_ext_g_sets[ 1 ] =
	    _mld wvt$b_ext_g_sets[ 2 ] = _mld wvt$b_ext_g_sets[ 3 ] =
		STANDARD_SET;
	}
	}
    else
	{
	_cld wvt$l_vt200_flags |= vt1_m_nrc_mode;
	/* Clear User Preferred Supplemental flag */
        _mld wvt$b_ups &= 0x0c;
	if ( !( _cld wvt$l_ext_flags & vte1_m_hebrew )) {
	    _mld wvt$b_g_sets[ 0 ] = _mld wvt$b_g_sets[ 1 ] = ASCII;
	    _mld wvt$b_ext_g_sets[ 0 ] = _mld wvt$b_ext_g_sets[ 1 ] =
		STANDARD_SET;
	}
	}
}

s_set_value_jisRomanAsciiMode( oldw, neww )
    DECtermWidget oldw, neww;
{      
    wvtp ld = w_to_ld( neww );

    if ( neww->common.jisRomanAsciiMode == DECwJisRomanMode )
	_cld wvt$l_ext_specific_flags |= vte2_m_jisroman_mode;
    else
	_cld wvt$l_ext_specific_flags &= ~vte2_m_jisroman_mode;
	/* Clear User Preferred Supplemental flag */
        _mld wvt$b_ups &= 0x0e;

    set_character_sets(ld);
}

s_set_value_kanjiKatakanaMode( oldw, neww )
    DECtermWidget oldw, neww;
{
    int temp;
                                  
    wvtp ld = w_to_ld( neww );

    if ( neww->common.kanjiKatakanaMode == DECwKanjiMode)
	_cld wvt$l_ext_specific_flags |= vte2_m_kanji_mode;
    else
	_cld wvt$l_ext_specific_flags &= ~vte2_m_kanji_mode;
	/* Clear User Preferred Supplemental flag */
        _mld wvt$b_ups &= 0;

     set_character_sets(ld);
}

s_set_value_kanji_78_83( oldw, neww )	/* Set/Reset '78-Kanji flag. */
    DECtermWidget oldw, neww;
{
    int temp;
    wvtp ld = w_to_ld( neww );
    _cld wvt$l_ext_specific_flags &= ~vte2_m_kanji_78;
    if ( neww->common.kanji_78_83 == DECwKanji_78) {
        _cld wvt$l_ext_specific_flags |= vte2_m_kanji_78;
    }
    set_character_sets(ld);
}

s_set_value_crm( oldw, neww )
    DECtermWidget oldw, neww;
{
    wvtp ld = w_to_ld( neww );

    if ( neww->common.controlRepresentationMode )
	{
	_cld wvt$l_vt200_flags |= vt1_m_display_controls_mode;
        WVT$CONTROL_FONT( ld );
	}
    else 
	{
	_cld wvt$l_vt200_flags &= ~vt1_m_display_controls_mode;
        WVT$RECALL_FONT( ld );
        }
}

s_set_value_ksRomanAsciiMode( oldw, neww )
    DECtermWidget oldw, neww;
{      
    wvtp ld = w_to_ld( neww );

    if ( neww->common.ksRomanAsciiMode == DECwKsRomanMode )
	_cld wvt$l_ext_specific_flags |= vte2_m_ksroman_mode;
    else
	_cld wvt$l_ext_specific_flags &= ~vte2_m_ksroman_mode;
	/* Clear User Preferred Supplemental flag */
    _mld wvt$b_ups &= 0;
    set_character_sets(ld);
}

void s_set_wvt$l_ext_flags( w )
    DECtermWidget w;
{
    wvtp ld = w_to_ld( w );
    _cld wvt$l_ext_flags = 0;
    switch ( ld->common.terminalType ) {
	case DECwKanji:	_cld wvt$l_ext_flags |= vte1_m_tomcat;
			break;
	case DECwHanzi:	_cld wvt$l_ext_flags |= vte1_m_bobcat;
			break;
	case DECwHangul:_cld wvt$l_ext_flags |= vte1_m_dickcat;
			break;
	case DECwHanyu:	_cld wvt$l_ext_flags |= vte1_m_fishcat;
			break;
	case DECwHebrew:_cld wvt$l_ext_flags |= vte1_m_hebrew;
			break;
	case DECwTurkish:_cld wvt$l_ext_flags |= vte1_m_turkish;
			break;
	case DECwGreek: _cld wvt$l_ext_flags |= vte1_m_greek;
			break;
	case DECwStandard:
	case DECwMulti:
	default:	break;
    }
}

s_set_value_terminalType( oldw, neww )
    DECtermWidget oldw, neww;
{
    char *bigFontSetName = NULL, *littleFontSetName = NULL;
    Boolean font_changed = False,
	    dialect_changed = False,
	    upss_changed = False;
    wvtp ld = w_to_ld( neww );

    if ( !( _cld wvt$l_ext_specific_flags & vte2_m_multi_mode ))
	return;
    s_set_wvt$l_ext_flags( neww );
    if ( _cld wvt$l_ext_flags & vte1_m_hebrew ) {
	_cld wvt$l_ext_specific_flags &= ~vte2_m_not_first_reset;
	if ( ld->common.keyboardDialect != DECwHebrewDialect ) {
	    dialect_changed = True;
	    ld->common.keyboardDialect = DECwHebrewDialect;
	}
	if ( ld->common.userPreferenceSet != DECwDEC_Hebrew_Supplemental &&
	     ld->common.userPreferenceSet != DECwISO_Latin8_Supplemental ) {
	    upss_changed = True;
	    ld->common.userPreferenceSet = DECwISO_Latin8_Supplemental;
	}
    } else {
	if ( ld->common.keyboardDialect == DECwHebrewDialect ) {
	    dialect_changed = True;
	    ld->common.keyboardDialect = DECwNorthAmericanDialect;
	}
	if ( ld->common.userPreferenceSet == DECwDEC_Hebrew_Supplemental ||
	     ld->common.userPreferenceSet == DECwISO_Latin8_Supplemental ) {
	    upss_changed = True;
	    ld->common.userPreferenceSet = DECwDEC_Supplemental;
	}
    }
    if ( dialect_changed )
	s_set_value_keyboardDialect( oldw, neww );
    if ( upss_changed )
	s_set_value_userPreferenceSet( oldw, neww );

    /* adjust font for standard & asian DECterm, 911029 */
    if ( _cld wvt$l_ext_flags & vte1_m_asian_common ) {
	if ( strncmp( ld->common.bigFontSetName, "-ADECW", 6 )) {
	    bigFontSetName	= DEFAULT_ASIAN_BIG_FONT_SET_NAME;
	    littleFontSetName	= DEFAULT_ASIAN_LITTLE_FONT_SET_NAME;
	    font_changed	= True;
	}
	if ( ld->common.fontSetSelection == DECwGSFont ) {
	    ld->common.fontSetSelection	= DECwLittleFont;
	    font_changed	= True;
	}
    } else {
	if ( !strncmp( ld->common.bigFontSetName, "-ADECW", 6 )) {
	    bigFontSetName	= DEFAULT_BIG_FONT_SET_NAME;
	    littleFontSetName	= DEFAULT_LITTLE_FONT_SET_NAME;
	    font_changed	= True;
	}
	if ( ld->common.fontSetSelection == DECwFineFont ) {
	    ld->common.fontSetSelection	= DECwLittleFont;
	    font_changed	= True;
	}
    }
    if ( bigFontSetName ) {
	if ( ld->output.status & BIG_FONT_SET_NAME_ALLOCATED )
	    XtFree( ld->common.bigFontSetName );
	ld->common.bigFontSetName = XtMalloc( strlen(bigFontSetName)+1 );
	strcpy( ld->common.bigFontSetName, bigFontSetName );
	ld->output.status |= BIG_FONT_SET_NAME_ALLOCATED;
    }
    if ( littleFontSetName ) {
	if ( ld->output.status & LITTLE_FONT_SET_NAME_ALLOCATED )
	    XtFree( ld->common.littleFontSetName );
	ld->common.littleFontSetName = XtMalloc( strlen(littleFontSetName)+1 );
	strcpy( ld->common.littleFontSetName, littleFontSetName );
	ld->output.status |= LITTLE_FONT_SET_NAME_ALLOCATED;
    }
    if ( font_changed )
	o_set_value_fontSetSelection( oldw, neww );

    /* reset terminal */
    if ( XtIsRealized( neww ))
	DECwTermReset( neww );
}

s_set_value_rToL( oldw, neww )
    DECtermWidget oldw, neww;
{
    wvtp ld = w_to_ld( neww );

    if (neww->common.rightToLeft)
	_cld wvt$l_ext_specific_flags |= vte2_m_rtl;
    else
	_cld wvt$l_ext_specific_flags &= ~vte2_m_rtl;
}

s_set_value_copyDir( oldw, neww )
    DECtermWidget oldw, neww;
{
    wvtp ld = w_to_ld( neww );

    if (neww->common.selectionRtoL)
	_cld wvt$l_ext_specific_flags |= vte2_m_copy_dir;
    else
	_cld wvt$l_ext_specific_flags &= ~vte2_m_copy_dir;
}

s_set_value_keyclickEnable( oldw, neww )
    DECtermWidget oldw, neww;
{
    i_change_keyboard_control( neww );
}

s_set_value_autoRepeatEnable( oldw, neww )
    DECtermWidget oldw, neww;
{
    wvtp ld = w_to_ld( neww );

    if ( neww->common.autoRepeatEnable )
	_cld wvt$l_kb_attributes |= UIS$M_KB_AUTORPT;
    else
	_cld wvt$l_kb_attributes &= ~UIS$M_KB_AUTORPT;

    i_change_keyboard_control( neww );
}

s_set_value_cursor_blink( oldw, neww )
    DECtermWidget oldw, neww;
{
    wvtp ld = w_to_ld( neww );

    if ( neww->common.cursorBlinkEnable )
	{
	_cld wvt$l_vt200_flags |= vt1_m_cursor_blink_mode;
	start_blink_timer (neww, 0);
	}
    else
	_cld wvt$l_vt200_flags &= ~vt1_m_cursor_blink_mode;
}

s_set_value_batchScrollCount( oldw, neww )
    DECtermWidget oldw, neww;
{
    int temp;
    wvtp ld = w_to_ld( neww );

    _mld wvt$l_defer_limit = ld->common.batchScrollCount;
    if ( ld->common.batchScrollCount <= 1 )
	{
	_cld wvt$l_vt200_flags_3 &= ~vt3_m_defer_up_scroll;
	_mld wvt$l_defer_max = 0;
	}
    else
	{
	_cld wvt$l_vt200_flags_3 |= vt3_m_defer_up_scroll;
	temp = _mld wvt$l_bottom_margin - _mld wvt$l_top_margin;
	if ( ld->common.batchScrollCount > temp )
	    _mld wvt$l_defer_max = temp;
	else
	    _mld wvt$l_defer_max = ld->common.batchScrollCount;
	}
}

s_enable_batch_scrolling( w )
    DECtermWidget w;
{
    s_set_value_batchScrollCount( w, w );
}

s_disable_batch_scrolling( w )
    DECtermWidget w;
{
    wvtp ld = w_to_ld( w );

    _cld wvt$l_vt200_flags_3 &= ~vt3_m_defer_up_scroll;
    _mld wvt$l_defer_max = 0;
    _mld wvt$l_defer_limit = 0;
}

s_set_value_localEcho( oldw, neww )
    DECtermWidget oldw, neww;
{
    char *aptr;
    int i;
    wvtp ld = w_to_ld( neww );

    if ( neww->common.localEcho )
	 _cld wvt$l_vt200_common_flags |= vtc1_m_echo_mode;
    else      
	 _cld wvt$l_vt200_common_flags &= ~vtc1_m_echo_mode;
}

s_set_value_concealAnswerback( oldw, neww )
    DECtermWidget oldw, neww;
{
    char *aptr;
    int i;
    wvtp ld = w_to_ld( neww );

/*    if ( neww->common.concealAnswerback )
    {
	neww->common.answerbackMessage = XtNewString("<concealed>");
	s_set_value_answerbackMessage( oldw, neww );          
    }
*/
}

s_set_value_answerbackMessage( oldw, neww )
    DECtermWidget oldw, neww;
{
    char *aptr;
    int i;
    wvtp ld = w_to_ld( neww );

    if ( oldw->common.answerbackMessage != NULL )
	XtFree( oldw->common.answerbackMessage );

    if ( neww->common.answerbackMessage != NULL )
	neww->common.answerbackMessage =
	  XtNewString( neww->common.answerbackMessage );

    for ( i = 0, aptr = neww->common.answerbackMessage; i < MAX_ANSWERBACK &&
      *aptr != NULL; i++ )
	_cld wvt$b_answerback[i] = *aptr++;
    _cld wvt$b_answerback[i] = '\0';
}

s_set_value_misc( oldw, neww )
    DECtermWidget oldw, neww;
{                 
    wvtp ld = w_to_ld( neww );

#define C_SET_FLAG(res, loc, flag )		\
	if (neww->common.res) _cld loc |= flag;	\
	else _cld loc &= ~flag;
#define C_SET_FLAG_N( res, loc, flag)		\
	if (!neww->common.res) _cld loc |= flag;	\
	else _cld loc &= ~flag;
#define SET_FLAG( res, loc, flag )		\
	if (neww->common.res) _mld loc |= flag;	\
	else _mld loc &= ~flag;
#define SET_FLAG_N( res, loc, flag )		\
	if (!neww->common.res) _mld loc |= flag;	\
	else _mld loc &= ~flag;

	C_SET_FLAG( lockUDK,
	    wvt$l_vt200_common_flags, vtc1_m_lock_set )
	C_SET_FLAG( lockUserFeatures,
	    wvt$l_vt200_common_flags, vtc1_m_feature_lock )
	C_SET_FLAG( marginBellEnable,
	    wvt$l_flags, vt4_m_margin_bell )
	C_SET_FLAG( warningBellEnable,
	    wvt$l_flags, vt4_m_bell )
	C_SET_FLAG( newLineMode,
	    wvt$l_vt200_flags, vt1_m_new_line_mode )
	SET_FLAG( autoWrapEnable,
	    wvt$l_vt200_specific_flags, vts1_m_auto_wrap_mode )
	C_SET_FLAG( applicationKeypadMode,
	    wvt$l_vt200_common_flags, vtc1_m_keypad_mode )
	C_SET_FLAG( applicationCursorKeyMode,
	    wvt$l_vt200_common_flags, vtc1_m_cursor_key_mode )
	C_SET_FLAG_N( eightBitCharacters,
	    wvt$l_vt200_flags, vt1_m_nrc_mode )
	C_SET_FLAG( printFormFeedMode,
	    wvt$w_print_flags, pf1_m_prt_ff_mode )
	C_SET_FLAG( print8BitMode,
	    wvt$w_print_flags, pf1_m_prt_transmission_mode )
	C_SET_FLAG_N( leadingCodeEnable,
	    wvt$l_ext_specific_flags, vte2_m_leading_code_mode )
}

void s_motion_handler(w, x, y)
    int x, y;
    DECtermWidget w;                                  
{
    wvtp ld = w_to_ld( w );
    Boolean ok = TRUE;

    if (_cld wvt$l_vt200_common_flags & vtc1_m_enable_locator &&
	_cld wvt$l_vt200_common_flags & vtc1_m_locator_report_mode) {
	convert_locator_coordinates(ld, x, y);
	x = _cld wvt$l_locator_x;
        y = _cld wvt$l_locator_y;
	if (_cld wvt$l_filter_ux == 0) {
		WVT$ENABLE_MOVEMENT_REPORT(ld, 0, 0, 0, 0, 0);  /* disable rec */
		s_do_locator_report(w, 10, _cld wvt$l_button_data, y, x, 1); /* page always 1 */
		return;
	}
	if (ok) ok = x > _cld wvt$l_filter_ux || x < _cld wvt$l_filter_lx ||
		     y > _cld wvt$l_filter_uy || y < _cld wvt$l_filter_ly;
	if (ok) {
		WVT$ENABLE_MOVEMENT_REPORT(ld, 0, 0, 0, 0, 0);  /* disable rec */
		s_do_locator_report(w, 10, _cld wvt$l_button_data, y, x, 1); /* page always 1 */
		/*		        ^    ^	  		*/
		/*		        |    +--Button Mask	*/
		/*		        +----Reason for report	*/
	}
    }
}

static void convert_locator_coordinates(ld, x, y)
    wvtp	ld;
    int	x, y;
{
    DECtermWidget   w = ld_to_w( ld );
    DECtermPosition pos;
    int line_spacing;

    if ( x >= 0 && y >= 0 ) {	/* make sure it's in the window */
	if (_cld wvt$l_vt200_common_flags & vtc1_m_locator_cell_position) {
		pos = o_convert_XY_to_position(w, x, moof(y) );
		x = DECtermPosition_column( pos )+1;
		y = DECtermPosition_row( pos );
			/* cells in output columns are from 0 .. infinity */
			/* must make them one based */
			/* cells in output rows are from -inf to +inf */
			/* top of logical display is at initial_line */
		if ( _mld wvt$l_vt200_specific_flags & vts1_m_origin_mode ) {
			/* origin mode, so relative to scrolling region */
		    if ( y < _mld wvt$l_top_margin
		      || y > _mld wvt$l_bottom_margin
		      || x < 1 || x > _ld wvt$l_column_width ) {
			x = (-1);
			y = (-1);
		    } else {
			y -= _mld wvt$l_top_margin - 1;
		    }
		} else {
			/* no need to adjust rows, since initial line is 1 */
		    if ( y < 1 || y > _mld wvt$l_page_length
		      || x < 1 || x > _ld wvt$l_column_width ) {
			x = (-1);	/* outside main display */
			y = (-1);
		    }
		}
	} else {
		o_convert_XY_to_pixels(w, x, moof(y), &x, &y, &line_spacing);
		if ( x < 0 || x >= w->common.logical_width
		  || y < 0 || y >= w->common.logical_height) {
			x = (-1);	/* outside main display */
			y = (-1);
		}
	}
    } else {
	x = y = (-1);
    }
    _cld wvt$l_locator_x = x;
    _cld wvt$l_locator_y = y;
}

static int reverse_buttonstate[8] = {
	0, 4, 2, 6,
	1, 5, 3, 7
};

static Boolean s_button_handler(w, button, buttonstate, x, y, down)
    DECtermWidget w;
    int button, buttonstate, x, y, down;
{
    wvtp ld = w_to_ld( w );
    DECtermPosition pos;

  if (_cld wvt$l_vt200_common_flags & vtc1_m_locator_report_mode)
  {
    buttonstate >>= 8;
    if (down) {
		buttonstate |= (1 << (button-1));
    } else {
		buttonstate &= ~(1 << (button-1));
    }
    buttonstate &= 7;	 /* only three locator buttons possible so mask any
			    garbage off */ 
    /* DECwindows buttonstate is reverse of ANSI buttonstate */
    buttonstate = reverse_buttonstate[buttonstate];

    _cld wvt$l_button_data = buttonstate;

    if (_cld wvt$l_vt200_common_flags & vtc1_m_enable_locator) {
	convert_locator_coordinates(ld, x, y);
	if ( x < 0 )
		return;		/* outside main display */
	x = _cld wvt$l_locator_x;
	y = _cld wvt$l_locator_y;

	if (_cld wvt$l_vt200_common_flags & (vtc1_m_locator_down_reports |
				       vtc1_m_locator_up_reports)) {
		if (down && _cld wvt$l_vt200_common_flags & vtc1_m_locator_down_reports) {
			s_do_locator_report(w, button*2+(1-down), buttonstate, y, x, 1);
		}
		if (!down && _cld wvt$l_vt200_common_flags & vtc1_m_locator_up_reports) {
			s_do_locator_report(w, button*2+(1-down), buttonstate, y, x, 1);
		}
	}

    }
    return( TRUE );

  } else {
    return( FALSE );
  }
    
}


WVT$ENABLE_MOVEMENT_REPORT(ld, enable, top, left, bottom, right)
wvtp	ld;
int enable, top, left, bottom, right;
{
DECtermWidget w = ld_to_w( ld );
Window wdummy;
int x, y, dummy;

	if (enable) {
		if (! (_cld wvt$l_vt200_common_flags & vtc1_m_locator_filter)) {
			/* if not already on */
		    i_enable_motion( w, s_motion_handler );
		}
		_cld wvt$l_vt200_common_flags |= vtc1_m_locator_filter;
		_cld wvt$l_filter_ux = right;
		_cld wvt$l_filter_lx = left;
		_cld wvt$l_filter_uy = bottom;
		_cld wvt$l_filter_ly = top;
		if (top == 0 && bottom == 0 && left == 0 && right == 0) return;
		if (!XQueryPointer(XtDisplay(w), XtWindow(w),
				   &wdummy, &wdummy,
				   &dummy, &dummy, &x, &y,
				   (unsigned int *)&dummy ))
		    x = y = (-1);
			/* find the current locator position */
		s_motion_handler( ld, x, y );
			/* pretend we moved to where we are */
	} else {
		i_enable_motion( w, NULL );
		_cld wvt$l_vt200_common_flags &= ~vtc1_m_locator_filter;
	}
}

void s_doing_report(w)
    DECtermWidget w;
{
    int check;
    wvtp ld = w_to_ld( w );

	if ((check = (_cld wvt$l_vt200_common_flags & vtc1_m_locator_one_shot)) ||
	    _cld wvt$l_vt200_common_flags & vtc1_m_locator_filter) {
		if (check) WVT$ENABLE_MOVEMENT_REPORT(ld, 0, 0, 0, 0, 0);
		_cld wvt$l_vt200_common_flags &= ~(vtc1_m_locator_one_shot |
					     vtc1_m_locator_report_mode | 
					     vtc1_m_locator_filter |
					     vtc1_m_locator_down_reports |
					     vtc1_m_locator_up_reports);
	}
}


WVT$REQUEST_LOCATION(ld) 
wvtp	ld;
{
DECtermWidget w = ld_to_w( ld );

	if ( !(_cld wvt$l_vt200_common_flags & vtc1_m_locator_filter ))
		{  /* position unknown so ask for it */
		get_locator_position(ld);
		}
	s_do_locator_report(w, 1, _cld wvt$l_button_data,
		_cld wvt$l_locator_y, _cld wvt$l_locator_x, 1); /* page always 1 */
}

WVT$LOCATOR_ERROR_REPORT(ld)
wvtp	ld;
{
DECtermWidget w = ld_to_w( ld );

	s_do_locator_report(w, 0, 0, 0, 0, 1);
}

static void get_locator_position(ld)
wvtp	ld;
{
DECtermWidget w = ld_to_w( ld );
Window wdummy;
int x, y, dummy;
unsigned int mask;

	if (!XQueryPointer(XtDisplay(w), XtWindow(w),
			   &wdummy,&wdummy,
			   &dummy, &dummy, &x, &y, &mask )
	  || x < X_MARGIN || x >= X_MARGIN + w->common.display_width
	  || y < Y_MARGIN || y >= Y_MARGIN + w->common.display_height )
		x = y = (-1);	/* off screen or outside display area */
	convert_locator_coordinates(ld, x, y);
	convert_button_state(ld, mask);
}

static void convert_button_state(ld, buttonstate)
    wvtp ld;
    int buttonstate;
{
    int mask = 0;

    if ( buttonstate & Button1Mask )
	mask |= 4;
    if ( buttonstate & Button2Mask )
	mask |= 2;
    if ( buttonstate & Button3Mask )
	mask |= 1;
    _cld wvt$l_button_data = mask;
}

Boolean GET_LOCATOR_STATUS(ld)
wvtp	ld;
{
	return(1);
}

s_do_locator_report(w, event, button, row, column, page)
DECtermWidget w;
int event, button, row, column, page;
{
	wvtp ld = w_to_ld( w );
	char report[100];
	Boolean disabled = ( _cld wvt$l_vt200_common_flags &
	  vtc1_m_locator_report_mode );

	s_doing_report(w);
	if ( row < 0 || event == 0 || disabled )
		sprintf(report, "\2330&w");  /* locator unavailable */
	else
		sprintf(report, "\233%d;%d;%d;%d&w",
			event, button, row, column);
		/* omit the page parameter because it is always 1 */
	i_report_data(w, report, strlen(report));
}


/*
 * selection routines
 */

/*
 * scroll_selection -- called when display list is scrolled
 *
 * Note: this routine should be called after o_scroll_lines, since
 * s_set_selection calls o_display_segment.
 */

static scroll_selection( w, count, top_line, size )
    DECtermWidget w;
    int count, top_line, size;
{
    DECtermPosition begin,end;
    int begin_row, begin_column, end_row, end_column, t;

    for ( t = 0; t < 2; t++ ) {
		/* for each selection type: 0 = primary, 1 = secondary */

	if ( Source_has_selection(w, t) ) {

	    begin = Source_select_begin(w, t);
	    begin_row = DECtermPosition_row( begin );
	    begin_column = DECtermPosition_column( begin );

	    end = Source_select_end(w, t);
	    end_row = DECtermPosition_row( end );
	    end_column = DECtermPosition_column( end );

	    if ( top_line <= begin_row && begin_row < top_line + size ) {

		/* selection starts within scrolling region */

		begin_row -= count;
		if ( begin_row < top_line ) {
		    /* selection scrolled off top of scroll region, so
		     * set start of selection to top of scroll region.
		     */
		    begin_row = top_line;
		    begin_column = 0;
		}
		else if ( begin_row >= top_line + size ) {
		    /* selection scrolled off bottom of scroll region, so
		     * set start of selection to bottom of scroll region.
		     */
		    begin_row = top_line + size;
		    begin_column = 0;
		}
	    }

	    if ( top_line <= end_row && end_row < top_line + size ) {

		/* selection ends within scrolling region */

		end_row -= count;
		if ( end_row < top_line ) {
		    /* selection scrolled off top of scroll region, so
		     * set end of selection to top of scroll region.
		     */
		    end_row = top_line;
		    end_column = 0;
		}
		else if ( end_row >= top_line + size ) {
		    /* selection scrolled off bottom of scroll region, so
		     * set end of selection to bottom of scroll region.
		     */
		    end_row = top_line + size;
		    end_column = 0;
		}
	    }

	    begin = DECtermPosition_position( begin_column, begin_row );
	    end = DECtermPosition_position( end_column, end_row );

	    /* update the selection if it changed */

	    if ( Source_select_begin(w, t) != begin
	         || Source_select_end(w, t) != end )
		s_set_selection( w, begin, end, CurrentTime, t );
	}
    }
}

/*
 * Routines called by Toolkit to manipulate the selection when we have it:
 *
 *	convert_selection -- get selection and return it to requestor
 *	lose_selection -- we no longer have the selection
 */

static Boolean convert_selection( w, selectionp, targetp,
				  typep, value, lengthp, formatp )
    DECtermWidget w;
    Atom *selectionp;
    Atom *targetp;
    Atom *typep;
    caddr_t *value;
    long *lengthp;
    int *formatp;
{
    char *ptr;
    int line, column;
    int type;
    Atom *targets_list;
    wvtp ld = w_to_ld( w );
    Atom XA_TARGETS = XInternAtom( XtDisplay( w ), "TARGETS", FALSE);
    Atom XA_DDIF = XInternAtom( XtDisplay( w ), "DDIF", FALSE);
    Atom XA_COMPOUND_TEXT = XInternAtom( XtDisplay( w ), "COMPOUND_TEXT", FALSE);
    XmString cs;
    long count, status, len;	/* 911021, TN018 */
    Boolean rToL = (( _cld wvt$l_ext_flags & vte1_m_hebrew ) &&
		    ( _cld wvt$l_ext_specific_flags & vte2_m_copy_dir )) ?
		    True : False;
    int inc = rToL ? -1 : 1;

    if(*selectionp == XA_PRIMARY ) {
      type = 0;
    }else {
	    if(*selectionp == XA_SECONDARY )
	      type = 1;
	    else
	      return(FALSE);
    }
    if(! Source_has_selection(w, type))
      return (FALSE);

    if ( *targetp == XA_TARGETS ) {
	targets_list = (Atom *)XtMalloc( 3 * sizeof( Atom ));
	targets_list[0] = XA_DDIF;
	targets_list[1] = XA_COMPOUND_TEXT;
	targets_list[2] = XA_STRING;
	*typep = XA_ATOM;
	*value = (caddr_t)targets_list;
	*lengthp = 3;
	*formatp = 32;	/* This value needs to be 32 on all platforms */
	return( TRUE );
    }
    if ( *targetp == XA_DDIF )
	*typep = XA_DDIF;
    else if ( *targetp == XA_COMPOUND_TEXT )
	*typep = XA_COMPOUND_TEXT;
    else if ( *targetp == XA_STRING )
	*typep = XA_STRING;
    else if ( *targetp == XA_PRIMARY )
	*typep = XA_PRIMARY;
    else
	return( FALSE );

/*
 * XA_STRING and XA_PRIMARY are both treated as strings
 */

    *formatp = 8;

    len = (unsigned) (( w->common.columns+1 ) *
	( DECtermPosition_row(Source_select_end(w, type)) -
	  DECtermPosition_row(Source_select_begin(w, type)) + 1 ));
    ptr = *value = XtMalloc( len );

    for ( line = DECtermPosition_row(Source_select_begin(w, type)),
	  column = DECtermPosition_column(Source_select_begin(w, type));
	  line <= DECtermPosition_row(Source_select_end(w, type));
          line++,
	  column = -1)
    {
	s_line_structure *ls;
	int end_of_line;

	if ( rToL ) {
	ls = s_read_data( w, line );
	if ( column < 0 )
	    *ptr++ = '\n';
	if ( DECtermPosition_row(Source_select_begin(w,type)) ==
	     DECtermPosition_row(Source_select_end(w,type)) ) {
	    column = DECtermPosition_column(Source_select_end(w,type)) - 1;
	    end_of_line = DECtermPosition_column(Source_select_begin(w,type));
	} else {
	    if ( line == DECtermPosition_row(Source_select_begin(w,type)) )
		column = DECtermPosition_column(Source_select_begin(w,type)) - 1;
	    else
		column = ls->w_widths - 1;
	    if ( line == DECtermPosition_row(Source_select_end(w,type)) )
		end_of_line = DECtermPosition_column(Source_select_end(w,type));
	    else
	    for ( end_of_line = 0; ls->a_code_base[end_of_line] == ' ' ||
				   ls->a_code_base[end_of_line] == 0;
		  end_of_line++ );
	}
	} else {
	if (column < 0)
	    {
	    column = 0;
	    *ptr++ = '\n';	/* throw in a newline every line */
	    }

	ls = s_read_data( w, line );

	/* find last non-blank char */
	for ( end_of_line = ls->w_widths;
	      ls->a_code_base[end_of_line-1] == ' ' ||
	          ls->a_code_base[end_of_line-1] == 0;
	      end_of_line-- );
	if ( line == DECtermPosition_row(Source_select_end(w, type)) &&
	       end_of_line > DECtermPosition_column(Source_select_end(w, type)))
	    end_of_line = DECtermPosition_column(Source_select_end(w, type));
	}

/* Copy only ASCII, DEC MCS, or ISO LATIN 1 characters */

	/* copy characters on line to destination string */
	for ( ; ( rToL && column >= end_of_line ) ||
		( !rToL && column < end_of_line ); column += inc )
	    switch ( ls->a_ext_rend_base[column] & csa_M_EXT_CHAR_SET ) {
		case STANDARD_SET:
		    switch ( ls->a_rend_base[column] & csa_M_CHAR_SET ) {
			case ASCII:
			case SUPPLEMENTAL:
			case TURKISH_SUPPLEMENTAL:
			case GREEK_SUPPLEMENTAL:
			case ISO_LATIN_5:
			case ISO_LATIN_7:
			case ISO_LATIN_1:
			    *ptr++ = ls->a_code_base[column] == 0 ?
				' ' : ls->a_code_base[column];
			    break;
		    }
		    break;
		case ONE_BYTE_SET:
		    switch ( ls->a_rend_base[column] & csa_M_CHAR_SET ) {
			case JIS_ROMAN:
			case JIS_KATAKANA:
			case CRM_FONT_L:
			case CRM_FONT_R:
			case KS_ROMAN:
			case ISO_LATIN_8:
			case HEB_SUPPLEMENTAL:

			    *ptr++ = ls->a_code_base[column] == 0 ?
				' ' : ls->a_code_base[column];
			    break;
		    }
		    break;
		case TWO_BYTE_SET:
		    switch ( ls->a_rend_base[column] & csa_M_CHAR_SET ) {
			case DEC_KANJI:
			case DEC_HANZI:
			case DEC_HANGUL:
			    *ptr++ = ls->a_code_base[column] == 0 ?
				' ' : ls->a_code_base[column];
			    break;
		    }
		    break;
		case FOUR_BYTE_SET:
		    switch ( ls->a_rend_base[column] & csa_M_CHAR_SET ) {
			case DEC_HANYU:
			    *ptr++ = ls->a_code_base[column] == 0 ?
				' ' : ls->a_code_base[column];
			    break;
			case DEC_HANYU_4:
			    if ( ls->a_ext_rend_base[column] ==
				THIRD_OF_FOUR ) {
/* fix allocation problem, 911028 */
				int offset = ptr - *value;
				*value = XtRealloc( *value, len+=2 );
				ptr = (char *) ( *value + offset );
/* fix end ...................... */
				*ptr++ = LC1;
				*ptr++ = LC2;
			    }
			    *ptr++ = ls->a_code_base[column] == 0 ?
				' ' : ls->a_code_base[column];
			    break;
		    }
		    break;
		default:
		    break;
	    }
    }
    *lengthp = ptr - *value;
    *ptr++ = 0;
    cs = DXmCvtFCtoCS( *value, &count, &status );
    XtFree( *value );
    if ( *typep == XA_DDIF ) {
	*value = DXmCvtCStoDDIF( cs, lengthp, &status );
    } else if ( *typep == XA_COMPOUND_TEXT ) {
	*value = XmCvtXmStringToCT( cs );
	*lengthp = strlen( *value );
    } else {	/* assume ( *typep == XA_STRING ) */
	*value = DXmCvtCStoOS( cs, lengthp, &status );
    }
    XmStringFree( cs );

    if (type == 1)
      s_set_selection( w, 0, 0, CurrentTime, 1 );
    return TRUE;
}

static void lose_selection( w, selectionp )
    DECtermWidget w;
    Atom *selectionp;
{
    int type;
    
    if (*selectionp == XA_PRIMARY) type = 0;
    else if (*selectionp == XA_SECONDARY) type = 1;
    else return;

    Source_has_selection(w, type) = FALSE;
    s_set_selection( w, 0, 0, CurrentTime, type );
}


/*
 * Routine called by  input  when selection is changed by user
 *
 *	s_set_selection -- set new selection or cancel existing selection
 *                         (if begin >= end, then cancel)
 */

void s_set_selection( w, begin, end, time, type )
    DECtermWidget w;
    DECtermPosition begin;
    DECtermPosition end;
    Time time;
{
    wvtp ld = w_to_ld(w);

    DECtermPosition
        old_begin = Source_select_begin(w,type),
        old_end =   Source_select_end(w,type),
	top =       DECtermPosition_position( 0,
			_mld wvt$l_transcript_top ),
	bottom =    DECtermPosition_position( 0, w->common.rows + 1 );

    if ( begin < top )
	begin = top;
    if ( end > bottom )
	end = bottom;

/*
 * Inform the toolkit that we think we have the selection
 */
    if ( begin < end ) {
        if (XtOwnSelection((Widget) w,type == 0 ?XA_PRIMARY
			   : XA_SECONDARY, time,
			   (XtConvertSelectionProc)convert_selection,
			   (XtLoseSelectionProc)lose_selection, NULL ) ) {
	    Source_has_selection(w, type) = TRUE;
	} else {
	    begin = end = 0;
	    Source_has_selection(w, type) = FALSE;
	}
    } else {
	begin = end = 0;
        if ( Source_has_selection(w,type) ) {
	    Source_has_selection(w, type) = FALSE;
	    XtDisownSelection((Widget)w ,
			      type == 0 ?XA_PRIMARY : XA_SECONDARY, time );
	}
    }
    {
	int begin_column = DECtermPosition_column( begin );
	s_line_structure *begin_ls = s_read_data( w,
			DECtermPosition_row( begin ));

	if ( 0 < begin_column ) {
	    if (( begin_ls->a_ext_rend_base[begin_column] &
		csa_M_BYTE_OF_CHAR ) == LAST_OF_TWO )
		begin--;
	    else if (( begin_ls->a_ext_rend_base[begin_column] &
		csa_M_EXT_CHAR_SET ) == FOUR_BYTE_SET ) {
		switch ( begin_ls->a_ext_rend_base[begin_column] ) {
		    case SECOND_OF_FOUR_LC: 
		    case SECOND_OF_FOUR:
		    case FOURTH_OF_FOUR:
			begin--;
			break;
		    case THIRD_OF_FOUR_LC:
			begin-=2;
			break;
		    case FOURTH_OF_FOUR_LC:
			begin-=3;
			break;
		}
	    }
	}
    }
    {
	int end_column = DECtermPosition_column( end ) - 1;
	s_line_structure *end_ls = s_read_data( w,
			DECtermPosition_row( end ));

	if (( 0 <= end_column ) && ( end_column < MAX_COLUMN )) {
	    if (( end_ls->a_ext_rend_base[end_column] &
		csa_M_BYTE_OF_CHAR ) == FIRST_OF_TWO )	
		end++;
	    else if (( end_ls->a_ext_rend_base[end_column] &
		csa_M_EXT_CHAR_SET ) == FOUR_BYTE_SET ) {
		switch ( end_ls->a_ext_rend_base[end_column] ) {
		    case THIRD_OF_FOUR_LC: 
		    case THIRD_OF_FOUR:
		    case FIRST_OF_FOUR:
			end++;
			break;
		    case SECOND_OF_FOUR_LC:
			end+=2;
			break;
		    case FIRST_OF_FOUR_LC:
			end+=3;
			break;
		}
	    }
	}
    }


    Source_select_begin(w, type) = begin;
    Source_select_end(w, type) = end;

/*
 * Compute range(s) that need to be refreshed.
 *
 *    Case 1:  The old and new regions are disjoint.
 *             Refresh them individually.
 *
 *    Case 2:  The old an new regions overlap or are adjacent.
 *             Refresh the changed begin range and the changed end range.
 */

    if (( _cld wvt$l_ext_flags & vte1_m_hebrew ) &&
	( _cld wvt$l_ext_specific_flags & vte2_m_copy_dir )) {
	if ( end >= old_begin && old_end >= begin ) {
	    if ((begin & 0xffffff00) < (old_begin & 0xffffff00)) {
		o_display_region( w, begin & 0xffffff00, begin);
		o_display_region( w, old_begin, old_begin | 0xff);
	    }
	    else if ((begin & 0xffffff00) > (old_begin & 0xffffff00)) {
		o_display_region( w, old_begin & 0xffffff00, old_begin);
		o_display_region( w, begin, begin | 0xff);
	    }
	    if ((old_end & 0xffffff00) < (end & 0xffffff00)) {
		o_display_region( w, end , end | 0xff);
		o_display_region( w, old_end & 0xffffff00 , old_end);
	    }
	    else if ((old_end & 0xffffff00) > (end & 0xffffff00)) {
		o_display_region( w, old_end , old_end | 0xff);
		o_display_region( w, end & 0xffffff00 , end);
	    }
	}
	else if (begin == 0 && end == 0) {
		o_display_region( w, old_begin & 0xffffff00, old_begin);
		o_display_region( w, old_end, old_end | 0xff);
	}
	else if (old_begin == 0 && old_end == 0) {
		o_display_region( w, begin & 0xffffff00, begin);
		o_display_region( w, end, end | 0xff);
	}
    }
    if ( end < old_begin || old_end < begin )
    {
	o_display_region( w, old_begin, old_end );
	o_display_region( w, begin, end );
    }
    else
    {
	o_display_region( w, old_begin, begin );
	o_display_region( w, old_end, end );
    }

}


/*
 * DECwTermSelectAll()
 *	Select all text in the logical display as the primary selection.
 */
void DECwTermSelectAll( w, selection, time )
    DECtermWidget w;
    Atom selection;
    Time time;
{
    int type;

    if (selection == XA_PRIMARY ) type = 0;
    else if (selection == XA_SECONDARY ) type = 1;
    else return;

    s_set_selection( w,
		     DECtermPosition_position( 0, 1 ),
		     DECtermPosition_position( 0, w->common.rows+1 ),
		     time, type );
}


/*
 * DECwTermCopySelection
 *	Return the current selection and un-select it.
 */
void DECwTermCopySelection( w, selection, target, callback, closure, time )
    DECtermWidget w;
    Atom	selection;
    Atom 	target;
    void	(*callback)();
    Opaque	closure;
    Time	time;
{
    char	*value;
    long	length;
    Atom	resulttype;
    int		format;
    int		type;

    if (selection == XA_PRIMARY ) type = 0;
    else if (selection == XA_SECONDARY ) type = 1;
    else return;

    if (!convert_selection( w, &selection, &target,
			    &resulttype, &value, &length, &format))
	value = NULL;
    (*callback)( w, closure, &selection, &resulttype, value, &length, &format, time ); 
    s_set_selection( w, 0, 0, time, type );
}

void _DECwTermSetTitle( shell, w, cs_title )	/* modified from DWI18n_SetTitle */
    Widget shell;
    DECtermWidget w;
    XmString cs_title;
{
    Arg arglist[2];
    XmStringCharSet charset;
    long count, status;
    char *title, *fc_title;

    if ( !cs_title )
	return;
    if ( DXmCSContainsStringCharSet( cs_title ) ||
	( !( w->source.wvt$l_ext_flags & vte1_m_asian_common ) &&
	  !XmIsMotifWMRunning( shell ))) {
	charset = "STRING";
	title = DXmCvtCStoFC( cs_title, &count, &status );
    } else {
	charset = "COMPOUND_TEXT";
	title = XmCvtXmStringToCT( cs_title );
    }
    XtSetArg( arglist[0], XtNtitle, title );
    XtSetArg( arglist[1], XtNtitleEncoding,
	XInternAtom( XtDisplay( shell ), charset, FALSE ));
    XtSetValues( (Widget)shell, arglist, 2 );
    XtFree( (char *)title );
}

void _DECwTermSetIconName( shell, w, cs_icon )	/* modified from DWI18n_SetIconName */
    Widget shell;
    DECtermWidget w;
    XmString cs_icon;
{
    Arg arglist[2];
    XmStringCharSet charset;
    long count, status;
    char *icon, *fc_icon;

    if ( !cs_icon )
	return;
    if ( DXmCSContainsStringCharSet( cs_icon ) ||
	( !( w->source.wvt$l_ext_flags & vte1_m_asian_common ) &&
	  !XmIsMotifWMRunning( shell ))) {
	charset = "STRING";
	icon = DXmCvtCStoFC( cs_icon, &count, &status );
    } else {
	charset = "COMPOUND_TEXT";
	icon = XmCvtXmStringToCT( cs_icon );
    }
    XtSetArg( arglist[0], XtNiconName, icon );
    XtSetArg( arglist[1], XtNiconNameEncoding,
	XInternAtom( XtDisplay( shell ), charset, FALSE ));
    XtSetValues( (Widget)shell, arglist, 2 );
    XtFree( (char *)icon );
}

/*
 * An error Handler for DT_toggle_keyboard (following).
 */
static int MyError(dpy, event)   
    Display *dpy;
    XErrorEvent *event;
{
    if ( event->error_code == BadRequest )
	myerror_status = KBModeSwitchInvalidCmd;
    return KBModeSwitchInvalidCmd;
}

/*
 * A keyboard toggle routine for both the new (V3) server and the old server.
 * This routine actually replaces the KM routine toggle_keyboard.
 */
void DT_toggle_keyboard( w )
    DECtermWidget w;
{
    Display *dpy = XtDisplay( w );
    wvtp ld = w_to_ld(w);
    int status;
    int	(*old_handler)();
   /*
    * If XSME not available, call KM directly.
    */
    if ( w->common.disableXSME ) {
        set_kb_state( dpy, TOGGLE_REQUEST );
	return;
    }   
   /* 
    * Trap errors if no ext exists 
    */
    old_handler = XSetErrorHandler( MyError );
    XSynchronize( dpy, True );

#ifndef VXT_DECTERM
   /* 
    * Set the keyboard according to the KM status known to DECterm. 
    */
    if ( _cld wvt$l_ext_specific_flags & vte2_m_kb_map ) 
	status = DoKBModeSwitch( dpy, UnlockModeSwitch );
    else
	status = DoKBModeSwitch( dpy, LockDownModeSwitch );
#else
	status = KBModeSwitchSuccess;
#endif

   /* 
    * Restore previous error handler 
    */
    XSetErrorHandler( old_handler );
    XSynchronize( dpy, False );
   /* 
    * If toggle failed, try to do it thru KM.
    */
    if (( myerror_status == KBModeSwitchInvalidCmd ) ||
	(( status != KBModeSwitchSuccess ) && ( status != KBModeSwitchNoop )))
        set_kb_state( dpy, TOGGLE_REQUEST );
   /*
    * Else just tell Km about the toggle. (Don't do it if Noop returned).
    */
    else
        if ( status != KBModeSwitchNoop ) {
	   /* 
            * The DECterm flags still show the old state of the keyboard.
            * Thus if vte2_m_km_map bit is set, it means Unlock has been done.
	    */
	    if ( _cld wvt$l_ext_specific_flags & vte2_m_kb_map ) 
	        set_kb_state( dpy, DISPLAY_PRIMARY );
	    else
	        set_kb_state( dpy, DISPLAY_SECONDARY );
	}
    return;
}

void set_kb_bit( w, state )
    DECtermWidget w;
    Boolean state;
{
    wvtp ld = w_to_ld( w );

    if ( state )
	_cld wvt$l_ext_specific_flags |= vte2_m_kb_map;
    else
	_cld wvt$l_ext_specific_flags &= ~vte2_m_kb_map;
}

void set_kb_pending( w )
    DECtermWidget w;
{
    wvtp ld = w_to_ld( w );

    if ( _cld wvt$l_ext_specific_flags & vte2_m_kb_map_pend_rqst ) {
	if (( _cld wvt$l_ext_specific_flags & vte2_m_kb_map_pend_toheb ) &&
	    ( !( _cld wvt$l_ext_specific_flags & vte2_m_kb_map ))) {
		DT_toggle_keyboard( w );
		_cld wvt$l_ext_specific_flags |= vte2_m_kb_map;
	} 
	if (( !( _cld wvt$l_ext_specific_flags & vte2_m_kb_map_pend_toheb )) &&
	    ( _cld wvt$l_ext_specific_flags & vte2_m_kb_map )) {
		DT_toggle_keyboard( w );
		_cld wvt$l_ext_specific_flags &= ~vte2_m_kb_map;
	}
	_cld wvt$l_ext_specific_flags &= ~vte2_m_kb_map_pend_rqst;
	_cld wvt$l_ext_specific_flags &= ~vte2_m_kb_map_pend_toheb;
    }
}

static void s_redisplay( w, heb_to_iso, iso_to_heb )
    DECtermWidget w;
    Boolean heb_to_iso, iso_to_heb;
{
    wvtp ld = w_to_ld( w );
    unsigned int i, j,
	topy = 1,			/* top line to erase */
	boty = _ld wvt$l_page_length,	/* bottom line to erase */
	leftx = 1,
	rightx = _ld wvt$l_column_width;
    unsigned char *charptr;		/* pointer to character array */
    REND *rendptr;			/* pointer to rendition array */
    EXT_REND *ext_rendptr;		/* pointer to ext_rendition array */

    if ( _ld wvt$a_code_cells ) {		/* if arrays are allocated... */
	for ( i=topy; i<=boty; i++ ) {		/* for each line ... */
	    charptr = &_ld wvt$a_code_base[i][leftx];/* character array base */
	    rendptr = &_ld wvt$a_rend_base[i][leftx];/* rendition array base */
	    ext_rendptr = &_ld wvt$a_ext_rend_base[i][leftx];
	    for ( j=0; j<=rightx-leftx; j++ ) {
		if ( heb_to_iso &&
		    *charptr >= 0xe0 && *charptr <= 0xfa ) { /* hebrew */
		    if ((( *ext_rendptr & csa_M_EXT_CHAR_SET ) == ONE_BYTE_SET )
		    && (( *rendptr & csa_M_CHAR_SET ) == ISO_LATIN_8 ||
			( *rendptr & csa_M_CHAR_SET ) == HEB_SUPPLEMENTAL )) {
			*ext_rendptr &= ~csa_M_EXT_CHAR_SET;
			*charptr &= 0x7f;
			*rendptr &= ~csa_M_CHAR_SET;
		    }
		} else if ( iso_to_heb &&
		    *charptr >= 0x60 && *charptr <= 0x7a ) { /* ASCII */
		    if ((( *ext_rendptr & csa_M_EXT_CHAR_SET ) == STANDARD_SET ) &&
			( *rendptr & csa_M_CHAR_SET ) == ASCII ) {
			*ext_rendptr |= ONE_BYTE_SET;
			*charptr |= 0x80;
			*rendptr |= ISO_LATIN_8;
		    }
		}
		charptr++;
		rendptr++;
		ext_rendptr++;
	    }
	}
    /*
     * Make sure output knows the correct width and height
     */
	for ( i=topy; i<=boty; i++ ) 
	    o_display_segment( w, moof(i), leftx-1, rightx-leftx+1 );
    }
}

/*
 * A routine to check if the keyboard is on a Hebrew state, for both the
 * new (V3) server and the old server.
 */
Boolean _DECwTermIsKBHebrew( w )
    DECtermWidget w;
{
    Display *dpy = XtDisplay( w );
    Window wdummy;
    int dummy;
    unsigned int mask;

   /*
    * First try the old-fashioned (V2) check - assuming there is a 
    * two-columns keyboard map.
    */
    if ( XKeycodeToKeysym( dpy, XKeysymToKeycode( dpy, XK_A ), 0 ) ==
	XK_hebrew_shin )
	return TRUE;
   /*
    * Then try to query the modifiers map, assuming there is an extension
    * to the server. (V3 server).
    */
    XQueryPointer(dpy, XtWindow( w ), &wdummy, &wdummy, &dummy, &dummy,
		  &dummy, &dummy, (unsigned int *)&mask );

#if (XlibSpecificationRelease >= 5)	/* R5 or later */
    if ( mask & ModeSwitchOfDisplay(dpy) )
#else
    if ( mask & dpy->mode_switch )
#endif
	return TRUE;
   /*
    * At this point, keyboard is not in a Hebrew state.
    */
    return FALSE;
}

void _DECwTermRedisplay( w )
    DECtermWidget w;
{
    s_redisplay( w, TRUE, TRUE );
}

_DECwTerm7bitMode( w )
    DECtermWidget w;
{
    wvtp ld = w_to_ld( w );

    return(( _cld wvt$b_conformance_level == LEVEL1
		|| ( _cld wvt$l_vt200_flags & vt1_m_nrc_mode ))
	    && w->common.keyboardDialect == DECwHebrewDialect );
}

void DECwTermRedisplay7bit( w )
    DECtermWidget w;
{
    wvtp ld = w_to_ld( w );

    if ( _cld wvt$b_conformance_level != LEVEL1 &&
	!( _cld wvt$l_vt200_flags & vt1_m_nrc_mode ))
	return;
    if ( w->common.keyboardDialect != DECwHebrewDialect )
	return;
    s_redisplay( w,
	_cld wvt$l_ext_specific_flags & vte2_m_kb_map ? FALSE : TRUE,
	_cld wvt$l_ext_specific_flags & vte2_m_kb_map ? TRUE : FALSE );
}

DwtDECtermToggleKeyboard( w, set )
    DECtermWidget w;
    Boolean set;
{
    wvtp ld = w_to_ld( w );

/* Do all this only if we have the input focus*/
    if ( w->input.has_focus ) {
	if ( _DECwTermIsKBHebrew( w )) 
	    _cld wvt$l_ext_specific_flags |= vte2_m_kb_map;
	else
	    _cld wvt$l_ext_specific_flags &= ~vte2_m_kb_map;
	if ( set ) {
	    if ( !( _cld wvt$l_ext_specific_flags & vte2_m_kb_map )) {
		DT_toggle_keyboard( w );
		_cld wvt$l_ext_specific_flags |= vte2_m_kb_map;
	    }
	} else {
	    if ( _cld wvt$l_ext_specific_flags & vte2_m_kb_map ) {
		DT_toggle_keyboard( w );
		_cld wvt$l_ext_specific_flags &= ~vte2_m_kb_map;
	    }
	}
    } else {	/* else, just collect the requests */
	_cld wvt$l_ext_specific_flags |= vte2_m_kb_map_pend_rqst;
	if ( set )
	    _cld wvt$l_ext_specific_flags |= vte2_m_kb_map_pend_toheb;
	else
	    _cld wvt$l_ext_specific_flags &= ~vte2_m_kb_map_pend_toheb;
    }
}

WVT$MAIN_DISPLAY(ld)
wvtp ld;
{
	_cld SpecificSourceData = & _cld MainSourceData;
	_cld wvt$l_vt200_common_flags &= ~vtc1_m_actv_status_display;
	/* the s_work_proc() function will handle the cursor redrawing */
}

WVT$STATUS_DISPLAY(ld)
wvtp ld;
{
	_cld SpecificSourceData = & _cld StatusSourceData;
	_cld wvt$l_vt200_common_flags |= vtc1_m_actv_status_display;
	/* the s_work_proc() function will handle the cursor redrawing */
}

WVT$ENABLE_STATUS_DISPLAY(ld)
wvtp ld;
{
DECtermWidget w = ld_to_w( ld );

    if (!w->common.statusDisplayEnable) {
	w->common.statusDisplayEnable = TRUE;
	o_set_value_statusDisplayEnable( w, w );
    }
}

WVT$DISABLE_STATUS_DISPLAY(ld)
wvtp ld;
{
DECtermWidget w = ld_to_w( ld );

    if (w->common.statusDisplayEnable) {
	w->common.statusDisplayEnable = FALSE;
	WVT$MAIN_DISPLAY(ld);
	o_set_value_statusDisplayEnable( w, w );
	WVT$STATUS_DISPLAY(ld);
	WVT$ERASE_DISPLAY( ld, 1, 1, 1, _ld wvt$l_column_width );
    }
    WVT$MAIN_DISPLAY(ld);
}

WVT$KEY_PRESSED(ld)
wvtp ld;
{
	i_key_pressed(ld);
}

s_clear_comm(ld)
wvtp ld;
{
	_cld wvt$l_vt200_common_flags &= ~vtc1_m_kbd_action_mode;
	if (_ld wvt$l_vt200_specific_flags & vts1_m_regis_available)
		WVT$CLEAR_REGIS(ld);
	_cld wvt$b_last_event = R_GRAPHIC;
	_cld wvt$b_in_dcs        = 0;
	pars_init(ld);
}

xosc_seq(ld, code)
wvtp ld;
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

    switch( _cld wvt$b_osc_state )
	{
	case OSC_STATE_INIT:
	    if ( code == '2' )
		_cld wvt$b_osc_state = OSC_STATE_PARSING_TYPE;
	    else if ( code == C0_ESC || code == C1_ST
		   || code == C0_SUB || code == C0_CAN )
		_cld wvt$b_in_dcs = FALSE;
	    break;

	case OSC_STATE_PARSING_TYPE:
	    if ( '0' <= code && code <= '9' || 'A' <= code && code <= 'Z' )
		{
		_cld wvt$b_osc_type = code;
		_cld wvt$l_osc_length = 0;
		_cld wvt$b_osc_state = OSC_STATE_NEED_SEMICOLON;
		}
	    else if ( code == C0_ESC || code == C1_ST
		   || code == C0_SUB || code == C0_CAN )
		_cld wvt$b_in_dcs = FALSE;
	    else
		_cld wvt$b_osc_state = OSC_STATE_BAD_COMMAND;
	    break;

	case OSC_STATE_NEED_SEMICOLON:
	    if ( code == ';' )
		_cld wvt$b_osc_state = OSC_STATE_PARSING_STRING;
	    else if ( code == C0_ESC || code == C1_ST
		   || code == C0_SUB || code == C0_CAN )
		_cld wvt$b_in_dcs = FALSE;
	    else
		_cld wvt$b_osc_state = OSC_STATE_BAD_COMMAND;
	    break;

	case OSC_STATE_PARSING_STRING:
	    if ( code == C0_ESC || code == C1_ST )
		{
		_cld wvt$b_osc_string[ _cld wvt$l_osc_length ] = '\0';
		execute_osc_command( ld );
		_cld wvt$b_in_dcs = FALSE;
		}
	    else if ( code == C0_SUB || code == C0_CAN )
		_cld wvt$b_in_dcs = FALSE;
	    else if ( _cld wvt$l_osc_length < OSC_MAX_STRING )
		_cld wvt$b_osc_string[ _cld wvt$l_osc_length++ ] = code;
	    break;

	case OSC_STATE_BAD_COMMAND:
	default:
	    if ( code == C0_ESC || code == C1_ST
	      || code == C0_SUB || code == C0_CAN )
		_cld wvt$b_in_dcs = FALSE;
	    break;
	}
}

execute_osc_command( ld )
    wvtp ld;
{
    DECtermWidget w = ld_to_w( ld );
    char *resource, *value;
    Arg arglist[1];
    DECwTermArgCallbackStruct call_data;
    XmString cs;
    long count, status;
    Widget parent = (Widget)w;

    switch ( _cld wvt$b_osc_type )
	{
	case '1':	/* Set_Terminal_Banner - Sets Window Title */
	    resource = XtNtitle;
	    if ( _cld wvt$l_osc_length == 0 )
		value = w->common.defaultTitle;
	    else
		value = (char *) _cld wvt$b_osc_string;
	    break;
	case 'L':	/* Set_Icon_Name - Sets Icon Name */
	    resource = XtNiconName;
	    if ( _cld wvt$l_osc_length == 0 )
		value = w->common.defaultIconName;
	    else
		value = (char *) _cld wvt$b_osc_string;
	    break;
	default:
	    return;	/* bad command type */
	}

    cs = DXmCvtFCtoCS( value, &count, &status );
    while ( XtParent( parent ))
	parent = XtParent( parent );
    if ( _cld wvt$b_osc_type == '1' )
	_DECwTermSetTitle( parent, w, cs );
    else if ( _cld wvt$b_osc_type == 'L' )
	_DECwTermSetIconName( parent, w, cs );
    XmStringFree( cs );

/* OSC bug fix.  We have to have the following callback to update the
 * stm->window_name or otherwise, user specified title will be ignored
 */
    XtSetArg( arglist[0], resource, value );

    call_data.reason = DECwCRShellValues;
    call_data.arglist = arglist;
    call_data.num_args = 1;

    XtCallCallbacks( (Widget)w, DECwNshellValuesCallback, &call_data );
}

s_set_value_textCursorEnable(oldw, neww)
    DECtermWidget oldw, neww;
{
    wvtp ld = w_to_ld( neww );

	if (neww->common.textCursorEnable == TRUE) {
		_cld wvt$b_cursts |= cs1_m_cs_enabled;
	} else {
		_cld wvt$b_cursts &= ~cs1_m_cs_enabled;
	}
	o_set_value_textCursorEnable(oldw, neww);
}

s_set_value_transcriptSize(oldw, neww)
    DECtermWidget oldw, neww;
{
    wvtp ld = w_to_ld( neww );
    int in_status = ( _cld wvt$l_vt200_common_flags
		& vtc1_m_actv_status_display );

    if ( neww->common.transcriptSize < 0 )
	{
	neww->common.transcriptSize = oldw->common.transcriptSize;
	return;
	}

    WVT$MAIN_DISPLAY( ld );
    xresize( ld, _ld wvt$l_column_width, _ld wvt$l_page_length,
		neww->common.transcriptSize, 0 );
    neww->common.transcriptSize = _ld wvt$l_transcript_size;
			/* in case xresize failed */
    neww->common.transcriptSize = _ld wvt$l_transcript_size;
			/* in case xresize failed */
    o_set_top_line( ld, _ld wvt$l_transcript_top );
    if ( in_status )
	WVT$STATUS_DISPLAY( ld );
}

s_set_value_defaultTitle( oldw, w )
    DECtermWidget oldw, w;
{
    char *s;

    if ( oldw->common.defaultTitle != NULL )
	XtFree( oldw->common.defaultTitle );
    s = XtMalloc( strlen( w->common.defaultTitle ) + 1 );
    strcpy( s, w->common.defaultTitle );
    w->common.defaultTitle = s;
}

s_set_value_defaultIconName( oldw, w )
    DECtermWidget oldw, w;
{
    char *s;

    if ( oldw->common.defaultIconName != NULL )
	XtFree( oldw->common.defaultIconName );
    s = XtMalloc( strlen( w->common.defaultIconName ) + 1 );
    strcpy( s, w->common.defaultIconName );
    w->common.defaultIconName = s;
}

WVT$DISABLE_CURSOR(ld)
wvtp ld;
{
DECtermWidget w = ld_to_w( ld );

	w->common.textCursorEnable = FALSE;
	o_set_value_textCursorEnable(w, w);
}

WVT$ENABLE_CURSOR(ld)
wvtp ld;
{
DECtermWidget w = ld_to_w( ld );

	w->common.textCursorEnable = TRUE;
	o_set_value_textCursorEnable(w, w);
}

WVT$LOCK_KEYBOARD(ld)
wvtp ld;
{
	i_lock_keyboard(ld);
}

WVT$UNLOCK_KEYBOARD(ld)
wvtp ld;
{
	i_unlock_keyboard(ld);
}

s_execute_deferred( ld )
wvtp ld;
{
int temp;

    if (_mld wvt$l_defer_count)	/* if we have been defering up scrolls */
	{
	 if (_mld wvt$l_defer_count >=
		(_mld wvt$l_bottom_margin - _mld wvt$l_top_margin)+1)
		{
		 WVT$ERASE_SCREEN(ld,	_mld wvt$l_top_margin,
					_mld wvt$l_left_margin,
					_mld wvt$l_bottom_margin,
					_mld wvt$l_right_margin);
		}
	 else
		{
		 WVT$UP_SCROLL(ld,	_mld wvt$l_top_margin,
					_mld wvt$l_defer_count, 2);
		}

	 temp = (_mld wvt$l_bottom_margin - _mld wvt$l_defer_count) + 1;
	 if (temp < _mld wvt$l_top_margin) temp = _mld wvt$l_top_margin;

	 _mld wvt$l_defer_count = 0;

	 WVT$REFRESH_DISPLAY(ld, temp, _mld wvt$l_bottom_margin);
	}
}

WVT$CLEAR_LKD( ld )	/* clear locator key definitions */
wvtp	ld;
{

_cld wvt$w_loc_length[0] = 6;	/* null button */
COPY( &_cld wvt$b_loc_area[0*6], "\33[240~", 6 );

_cld wvt$w_loc_length[1] = 6;	/* button 1 down */
COPY( &_cld wvt$b_loc_area[1*6], "\33[241~", 6 );

_cld wvt$w_loc_length[2] = 6;	/* button 2 down */
COPY( &_cld wvt$b_loc_area[2*6], "\33[243~", 6 );

_cld wvt$w_loc_length[3] = 6;	/* button 3 down */
COPY( &_cld wvt$b_loc_area[3*6], "\33[245~", 6 );

_cld wvt$w_loc_length[4] = 6;	/* button 4 down */
COPY( &_cld wvt$b_loc_area[4*6], "\33[247~", 6 );

_cld wvt$w_loc_length[5] = 0;	/* unused */
_cld wvt$w_loc_length[6] = 0;	/* button 1 up */
_cld wvt$w_loc_length[7] = 0;	/* button 2 up */
_cld wvt$w_loc_length[8] = 0;	/* button 3 up */
_cld wvt$w_loc_length[9] = 0;	/* button 4 up */

}

int s_return_lkd( w, offset, pdata )
    DECtermWidget w;
    int offset;
    unsigned char **pdata;
{
    wvtp ld = w_to_ld( w );

    *pdata = &_cld wvt$b_loc_area[ offset * 6 ];
    return _cld wvt$w_loc_length[ offset ];
}

/*
 * s_clear_display - like DECwTermClearDisplay except it saves erased lines
 *		     in the transcript (if the resource is set)
 */

s_clear_display( w )
    DECtermWidget w;
{
    int y;
    wvtp ld = w_to_ld( w );
    int in_status_now;
	in_status_now = _cld wvt$l_vt200_common_flags & vtc1_m_actv_status_display;

	WVT$STATUS_DISPLAY(ld);
	for (y=1; y<=_ld wvt$l_page_length; y++)
            {
	    line_rendition(y) = SINGLE_WIDTH;
	    line_width(y) = _ld wvt$l_column_width;
            }
	WVT$ERASE_DISPLAY(ld, 1, 1,
		_ld wvt$l_page_length,
		_ld wvt$l_column_width);	/* Clear Window */
	_ld wvt$l_actv_line = 1;
	_ld wvt$l_actv_column = 1; _ld wvt$l_disp_pos = 1;
        _ld wvt$a_cur_cod_ptr = _ld wvt$a_code_base[_ld wvt$l_actv_line] +
		_ld wvt$l_actv_column;
        _ld wvt$a_cur_rnd_ptr = _ld wvt$a_rend_base[_ld wvt$l_actv_line] +
		_ld wvt$l_actv_column;
	_ld wvt$a_cur_ext_rnd_ptr = _ld wvt$a_ext_rend_base[_ld wvt$l_actv_line] +
		_ld wvt$l_actv_column;
	WVT$MAIN_DISPLAY(ld);
	for (y=1; y<=_ld wvt$l_page_length; y++)
            {
	    line_rendition(y) = SINGLE_WIDTH;
	    line_width(y) = _ld wvt$l_column_width;
            }
	WVT$ERASE_DISPLAY(ld, 1, 1,
		_ld wvt$l_page_length,
		_ld wvt$l_column_width);	/* Clear Window */
	_ld wvt$l_actv_line = 1;
	_ld wvt$l_actv_column = 1; _ld wvt$l_disp_pos = 1;
        _ld wvt$a_cur_cod_ptr = _ld wvt$a_code_base[_ld wvt$l_actv_line] +
		_ld wvt$l_actv_column;
        _ld wvt$a_cur_rnd_ptr = _ld wvt$a_rend_base[_ld wvt$l_actv_line] +
		_ld wvt$l_actv_column;
	_ld wvt$a_cur_ext_rnd_ptr = _ld wvt$a_ext_rend_base[_ld wvt$l_actv_line] +
		_ld wvt$l_actv_column;
	o_set_cursor_position(w, moof(1), 0);
	if (in_status_now) WVT$STATUS_DISPLAY(ld);
}

/* WVT$ENABLE_SESSION - give the input focus to the current window */

WVT$ENABLE_SESSION(ld)
    wvtp ld;
{
    DECtermWidget w = ld_to_w(ld);

    XSetInputFocus( XtDisplay(w), XtWindow(w), RevertToNone, CurrentTime );
}

/* old widget bindings for backward compatibility */

void DwtDECtermReset(w)
    DECtermWidget w;
{
    DECwTermReset(w);
}

void DwtDECtermClearDisplay(w)
    DECtermWidget w;
{
    DECwTermClearDisplay(w);
}

void DwtDECtermClearTranscript(w)
    DECtermWidget w;
{
    DECwTermClearTranscript(w);
}

void DwtDECtermPutData(w, data, count)
    DECtermWidget w;
    char *data;
    unsigned count;
{
    DECwTermPutData(w, data, count);
}

void DwtDECtermConcertXYToANSI( w, x, y, colp, rowp )
    DECtermWidget w;
    int	x,y;
    int *colp,*rowp;
{
    DECwTermConvertXYToANSI( w, x, y, colp, rowp );
}

void DwtDECtermConvertANSIToXY( w, col, row, xp, yp )
    DECtermWidget w;
    int col,row;
    int	*xp,*yp;
{
    DECwTermConvertANSIToXY( w, col, row, xp, yp );
}

void DwtDECtermSelectAll( w, selection, time )
    DECtermWidget w;
    Atom selection;
    Time time;
{
    DECwTermSelectAll( w, selection, time );
}

void DwtDECtermCopySelection( w, selection, target, callback, closure, time )
    DECtermWidget w;
    Atom	selection;
    Atom 	target;
    void	(*callback)();
    Opaque	closure;
    Time	time;
{
    DECwTermCopySelection( w, selection, target, callback, closure, time );
}


/*   set copy direction bit:
 * 	TRUE is right to left
 *	FALSE is left to right
 */
_DECwTermSetCopyDirection( w, direction )
    DECtermWidget w;
    Boolean direction;
{
    wvtp ld = w_to_ld( w );

    if ( direction )
	_cld wvt$l_ext_specific_flags |= vte2_m_copy_dir;
    else
	_cld wvt$l_ext_specific_flags &= ~vte2_m_copy_dir;
}

_DECwTermCopyDirection( w )
    DECtermWidget w;
{
    wvtp ld = w_to_ld( w );

    s_set_selection( w, 0, 0, CurrentTime, 0);
    s_set_selection( w, 0, 0, CurrentTime, 1);
    return( _cld wvt$l_ext_specific_flags & vte2_m_copy_dir );
}

/*
 * DECwindows applications are supposed to use XtMalloc (not malloc) to ask
 * for memory.  Unfortunately, if XtMalloc fails it doesn't return NULL like
 * malloc does; instead, it invokes a (standard) error handler.  To remedy
 * this problem, the widget should call DECwTermXtMalloc instead, which will
 * return NULL if the requested memory can not be allocated.  Use XtFree to
 * free memory allocated with DECwTermXtMalloc.  The same applies to
 * DECwTermXtCalloc.
 *
 * These routines also exist in UTIL.C (where they are called DECtermMumble
 * instead of DECwTermMumble), but are replicated here, so that the build
 * doesn't fail when we link shareable.
 */

static Boolean DECwTermXtMallocFailed;
static Boolean DECwTermXtCallocFailed;
static XtAppContext app_context = NULL;

XtAppContext
get_app_context()
{
    if (app_context == NULL)
        app_context = XtCreateApplicationContext();

    return(app_context);
}


static void
DECwTermXtMallocErrorHandler()
{
    DECwTermXtMallocFailed = TRUE;
}


char *
DECwTermXtMalloc(size)
int size;
{
char *buffer;

XtErrorMsgHandler oldHandler = XtAppSetErrorMsgHandler(get_app_context(),
			       (XtErrorMsgHandler)DECwTermXtMallocErrorHandler);

    DECwTermXtMallocFailed = FALSE;

    buffer = XtMalloc(size);

    XtAppSetErrorMsgHandler(app_context, oldHandler);

    if(DECwTermXtMallocFailed)
        return NULL;
    else
        return buffer;
}


static void
DECwTermXtCallocErrorHandler()
{
    DECwTermXtCallocFailed = TRUE;
}


char *
DECwTermXtCalloc(num, size)
int num, size;
{
char *buffer;

XtErrorMsgHandler oldHandler = XtAppSetErrorMsgHandler(get_app_context(),
			       (XtErrorMsgHandler)DECwTermXtCallocErrorHandler);

    DECwTermXtCallocFailed = FALSE;

    buffer = XtCalloc(num, size);

    XtAppSetErrorMsgHandler(app_context, oldHandler);

    if(DECwTermXtCallocFailed)
        return NULL;
    else
        return buffer;
}
