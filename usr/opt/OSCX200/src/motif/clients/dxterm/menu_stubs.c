/* #module menu_stubs "X3.0-7" */
/*
 *  Title:	menu_stubs.c
 *
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © Digital Equipment Corporation, 1988, 1993  All Rights      |
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
 *	Callbacks for DECterm user interface including
 *      all pull-down menus and setup dialogs
 *
 *  Modification history:
 *
 * Alfred von Campe     08-Nov-1993     BL-E
 *     - Add F11 key feature from dxterm.
 *
 * Alfred von Campe     02-Nov-1993     BL-E
 *	- Use 10 and 14 point fonts on 100 DPI monitors.
 *
 * Alfred von Campe     25-Oct-1993     BL-E
 *      - Remove CDA dependancy.
 *
 * Grace Chung          15-Sep-1993     BL-E
 *       - Add 7-bit/8-bit printer support.
 *
 * Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 * Alfred von Campe     25-Mar-1993     V1.2/BL2
 *      - OSF/1 code merge.
 *
 * Aston Chan		12-Mar-1993	V1.2/BL2
 *	- Add Turkish/Greek support.
 *
 * Alfred von Campe     04-Feb-1993     Ag/Bl12 (MUP)
 *	- Tell DXmPrint widget to always delete file after printing.
 *
 * Aston Chan		22-Jan-1993	V1.2/BL1
 *	- Fix FileSelection dialog boxes' context sensitive help.
 *
 * Alfred von Campe     20-Jan-1993     Ag/Bl12 (SSB)
 *	- Include menubar height when computing new position.
 *
 * DAM                  10-Nov-1992     VXT V1.2?
 *      - Removing resolve_lock_state_cb().  Obsolete.
 *
 * Alfred von Campe     08-Oct-1992     Ag/BL10
 *	- Added typecasts to satisfy Alpha compiler.
 *
 * Eric Osman		 5-Oct-1992	VXT V1.2
 *	- If user types new answerback message, then clicks conceal, use
 *	  concealed_ans cell to remember the message to use.
 *
 * Aston Chan		20-Aug-1992	Post V1.1
 *	- Add Shai's fix.  Add pre-declared functions stubs in order to make it
 *	  compile cleanly on U*x.
 *
 * Eric Osman           11-June-1992     Sun
 *      - On non-Vms, don't include CDA file.
 *
 * Aston Chan		28-May-1992	V3.1 (post ssb)
 *	- Fix the HyperHelp context for "OnWindow", "OnVersion" and "OnHelp"
 *	  topics.
 *	- Don't open HyperHelp context for version dialog box.
 *
 * Aston Chan		11-May-1992	V3.1 (post ssb)
 *	- Change version dialog box to show version and compile date/time.
 *
 * Aston Chan		19-Dec-1991	V3.1
 *	- Change the I18n public entry points to _DECwTerm* prefix
 *	- Merge in new fix for I18n versions
 *
 * Aston Chan		17-Dec-1991	V3.1
 *	- I18n code merge
 *
 * Alfred von Campe     02-Dec-1991     V3.1
 *      - Add calls to SetWMHints().
 *
 * Aston Chan		18-Nov-1991	V3.1
 *	- DXmHelpSystemOpen() is now called only when help is activated.
 *	  This fix garanteed that HyperHelp code path is *only* activated
 *	  when user click help or press help key.
 *
 * Aston Chan		14-Nov-1991	V3.1
 *	- HyperHelp support by adding help_system_proc, help_activate_proc,
 *	  help_error and k_help_on* constants.  Adding calls to
 *	  DXmHelpSystemDisplay() for bring up the HyperHelp topic window.
 *	- Use XmStringFree() to free XmString instead of XtFree().
 *
 * Alfred von Campe     13-Nov-1991     V3.1
 *      - Check new autoAdjustPosition resource before deciding to move window.
 *
 * SPM			04-Nov-1991	V3.1
 *	- Add VXT specfic 2-level lock routine resolve_lock_state_cb().
 *	  This was a MrmNcreateCallback added to decterm.UIL.
 *
 * Alfred von Campe    06-Oct-1991     Hercules/1 T0.7
 *      - Changed private #include file names to all lower case.
 *	- Added #extern definition for XmTextGetString().
 *	- Added F11 key feature from ULTRIX.
 *      - Corrected compilation problems for secure keyboard feature.
 *      - Corrected spelling of s_MRMHierarchy and XmToggleButtonCallbackStruct.
 *      - Don't use set_something(); call XSetArg and XSetValues directly.
 *      - Don't override help library name; use DECTERM_APPL_CLASS.
 *
 * Aston Chan		27-Sep-1991	V3.1
 *	- When 7-bit NRCS dialect is selected, don't change the resource until 
 *	  user pressed OK or Apply so that Cancel can undo the selection. 
 *	  QAR 0021 of MACAW QAR DB.
 *	  More on this QAR...  dialect_select_cb() is obsolete.
 *	  Also dialect_cancel_cb() can just unmange the dialogue box.
 *
 * Aston Chan		1-Sep-1991	Alpha
 *	- Add globalref definition for streams.  Required by DECC.
 *
 * Alfred von Campe     24-May-1991     V3.0
 *	- Change help frame name from DECterm to Overview.
 *
 * Alfred von Campe     15-May-1991     V3.0
 *	- Add support for the "On Context" and "On Help" menu items.
 *      - Ignore condensed font requests if the German Standard font is used.
 *
 *  Michele Lien    11-Dec-1990	VXT X0.0 BL2
 *  - VXT can only direct printer output to a dedicated printer port, "printer
 *    destination" option in printer customize menu for DECterm V3.0 has been 
 *    taken out.  All the resources associated with printer destination and 
 *    port name can only be conditionally compiled for non-VXT systems.  
 *    Printer destination and printer port name has been hard coded to the 
 *    local printer port.
 *
 * Bob Messenger	15-Sep-1990	X3.0-7
 *	- Add support for GS fonts.
 *
 * Bob Messenger	13-Sep-1990	X3.0-7
 *	- Check for the batch scroll count, transcript size or number of
 *	  bit planes being negative.
 *
 * Bob Messenger	27-Aug-1990	X3.0-6
 *	- Fix Print Color Printing button (read state from Color Printing
 *	  buton, not from Monochrome Printing).
 *
 *  Michele Lien    24-Aug-1990 VXT X0.0
 *  - Modify this module to work on VXT platform. Change #ifdef VMS to
 *    #ifdef VMS_DECTERM so that VXT specific code can be compiled under
 *    VMS or ULTRIX development environment.
 *
 * Bob Messenger        17-Jul-1990     X3.0-5
 *      Merge in Toshi Tanimoto's changes to support Asian terminals -
 *	- callbacks
 *
 * Bob Messenger	 4-Jul-1990	X3.0-5
 *	- Add printer port support.
 *
 * Mark Woodbury	25-may-1990 X3.0-3M
 *	Motif Changes:
 *    1. Mrm does not like the terminal widget to be the parent of the
 *       popup dialog boxes make the shell widget the parent.
 *
 * Bob Messenger	 9-Apr-1990	X3.0-2
 *	- Merge changes from UWS V4.0 and VMS V5.4.
 *
 * Bob Messenger	12-Mar-1990	V2.1 (VMS V5.4)
 *	- Initialize Host Status Display button correctly.
 *
 * Bob Messenger	27-Feb-1990	V2.1 (UWS V4.0)
 *	- Added secure keyboard feature for UWS V4.0.
 *	- Incorporate Mark Granoff's 7-Feb-1990 change to X3.0-1: "Add XtFree
 *	  calls for DwtSTextGetString calls - they should be there to free
 *	  memory held by strings allocated by DwtSTextString".
 *
 * Mark Granoff		 7-Feb-1990	X3.0-1
 *	- Add XtFree calls for XmTextGetString calls - they should be there to
 *	  free memory help by strings allocated by DwtSTextString.
 *
 * Bob Messenger	23-Aug-1989	X2.0-20 (UWS V2.2)
 *	- Fixed Ultrix compilation error (needed cast for keyboardDialect).
 *	- Changed Ultrix help library name from DXterm to dxterm.
 *
 * Bob Messenger	 9-Aug-1989	X2.0-19
 *	- When resizing window, reposition it so that it's completely visible.
 *
 * Bob Messenger	 8-Aug-1989	X2.0-18
 *	- Set the initial 7-Bit NRCS Selection list box selection according
 *	  to the resource value.
 *
 * Bob Messenger	20-Jul-1989	X2.0-16
 *	- Define LANGUAGE_SWITCHING_TOOLKIT
 *
 * Bob Messenger	30-May-1989	X2.0-13
 *	- Convert fprintf's to calls to log_message, so messages get flushed.
 *
 * Bob Messenger	21-May-1989	X2.0-12
 *	- Make language switching change conditional on
 *	  LANGUAGE_SWITCHING_TOOLKIT, which will be undefined until
 *	  the toolkit lets us specify the help library name as just
 *	  DECW$TERMINAL.
 *
 * Bob Messenger	19-May-1989	X2.0-11
 *	- Language switching: don't include directory in help library name.
 *
 * Bob Messenger	14-May-1989	X2.0-10
 *	- Change printf calls to fprintf on stderr.
 *
 * Bob Messenger	 3-May-1989	X2.0-9
 *	- Make context sensitive help work by working around an apparent bug
 *	  in the help widget: have to reload widget each time to get new topic.
 *
 * Bob Messenger	22-Apr-1989	X2.0-7
 *	- Change XmNtitle and XmNiconName to XtNtitle and XtNiconName.
 *	- Fix status display enable button.
 *
 * Bob Messenger	 8-Apr-1989	X2.0-6
 *	- Support context sensitive help.
 *	- Use (hopefully) better names for keyboard mapping resource values.
 *
 * Bob Messenger	 7-Apr-1989	X2.0-6
 *	- Use new widget bindings (DECwTerm instead of DwtDECterm).
 *	- Move find_top_level_widget here from dwt_undefined.c.
 *
 * Bob Messenger	 1-Apr-1989	X2.0-5
 *	- Fix support for title and icon name.
 *
 * Bob Messenger	18-Mar-1989	X2.0-3
 *	- Convert to V2.0 user interface: add terminal driver resize, window
 *	  title, icon name, batch scrolling, transcript size, VT330 ID,
 *	  and Customize Graphics and take out Shift Lock/Caps Lock.
 *	- Put callbacks back in alphabetical order (might speed things up)
 *	- Fix problem where Overview and About libraries couldn't be accessed
 *	  (call MrmFetchWidgetOverride instead of MrmFetchWidget).
 *
 * Tom Porcher		20-Oct-1988	X0.5-4
 *	- Add seperate callback for Help/Overview.
 *	- Make Shift-Help/About call up "Show Version".
 *	- Make routines more tolerant of UIL file mismatch:  never exit.
 *
 * Eric Osman		13-Sep-1988	FT2.1
 *	- Fix delete/backspace radio button to take on correct value
 *	(mispellt resource name in XtSetArg)
 *
 * Tom Porcher		14-Sep-1988	X0.5-2
 *	- Add declaration on copyright_callback() (in ptysub.c).
 *
 * Mike Leibow		16-Aug-1988	X0.4-44
 *	- font sizes change according to adjustFontsizes button and
 *	  number of columns.  Used to remain constant.
 *
 * Tom Porcher          16-Aug-1988     X0.4-43
 *	- remove keyclick.
 *	- add control-Q/S = Hold.
 *
 * Tom Porcher          12-Aug-1988     X0.4-43
 *	- Set reverseVideo resource to TRUE if screenMode changes.
 *	- couplingHorizontal was being set from scrollHorizontal.  I fixed it.
 *
 * Tom Porcher          10-Aug-1988     X0.4-35
 *	- Set state of "record lines off top" button from current value.
 *
 * Tom Porcher          30-Jun-1988     X0.4-34
 *      - remove work-around for BL8.4/5 intrinsics XtGetValues() bug.
 *	- correct all XtGetValues of Positions and Dimensions.
 *
 * Tom Porcher          24-Jun-1988     X0.4-33
 *      - temporary work-around for BL8.4/5 intrinsics XtGetValues() bug:
 *        Boolean => int
 *
 * Tom Porcher		16-Jun-1988	X0.4-32
 *	- changed "external" back to "globalref" since "external" conflicts
 *	  with a toolkit symbol of the same name.
 *
 * Tom Porcher		21-Jun-1988	X0.4-32
 *	- Changed DwtRegisterDRMCallbacks to DwtRegisterDRMNames.
 *
 * Peter Sichel         30-May-1988    X0.4-30
 *      - Added help menu with "Show Version" and "About DECterm"
 *
 * Tom Porcher		11-May-1988	X0.4-26
 *	- Added "Clear Comm".
 *
 * Tom Porcher		 4-May-1988	X0.4-16
 *	- Fixed ACCVIO in window_set_resources:  arglist was getting
 *	  a bogus entry.
 *
 * Tom Porcher		25-Apr-1988	X0.4-12
 *	- Fixed Ultrix compilation problems.
 *
 *  Peter Sichel        22-Apr-1988     X0.4-11
 *      - Moved save_lines_off_top from Commands menu
 *        to window setup.  Commented out un-implemented widgets
 *        Added text input for rows and columns
 *
 *  Peter Sichel        10-Apr-1988     X0.4-8
 *      - Extensively modified to use UIL setup widgets
 *
 *  Tom Porcher		13-Feb-1988	X0.3-4
 *	- Removed "Create Emulator" button
 *
 *  Tom Porcher		16-Jan-1988	X0.3-2
 *	- Made columns/rows scales drag change the value displayed.
 */

#include "mx.h"
#include "dectermp.h"
#include "dlfd.h"	/* pick up symbol MAX_FONT_NAME_SIZE */
#if defined(VMS_DECTERM) || defined(VXT_DECTERM)
#include "DXmHelpB.h"
#include "DXmPrint.h"
#include "DXmCSText.h"
#else
#include <DXm/DXmHelpB.h>
#include <DXm/DXmPrint.h>
#include <DXm/DXmCSText.h>
#endif

#ifdef VXT_DECTERM
#include <stdlib.h>
#include "str_func.h"
#include "vxtconfig.h"
#endif

#ifndef XK_HEBREW
#define XK_HEBREW 1
#endif

#define LANGUAGE_SWITCHING_TOOLKIT 1

/* external function declarations */
extern Widget XmCreateToggleButton();
extern Widget find_top_level_widget();

#ifdef VXT_DECTERM
extern int get_printer_config();
#endif

/*
 * Note: The XmFileSelectionBoxGetChild routine isn't yet ported to vxt.
 * However, I peeked at what it does, and for the parameter we're using
 * in this module, it merely calls XmSelectionBoxGetChild, which is ported
 * to vxt, so we cheat for now by defining a macro that redefines this
 * routine.  This is obviously temporary until the routine is ported to
 * vxt.  /Eric Osman 11-Jun-1993
 */
#ifdef VXT_DECTERM
#define XmFileSelectionBoxGetChild(a,b) (XmSelectionBoxGetChild((a),(b)))
extern Widget XmSelectionBoxGetChild();
#else
extern Widget XmFileSelectionBoxGetChild();
#endif

extern char *XmTextGetString();
extern void _DECwTermSetTitle();
extern void _DECwTermSetIconName();
extern void SetWMHints();

#define WINDOW_PARENT stm->setup.window_parent
#define DISPLAY_PARENT stm->setup.display_parent
#define GENERAL_PARENT stm->setup.general_parent
#define KEYBOARD_PARENT stm->setup.keyboard_parent
#define DIALECT_PARENT stm->setup.dialect_parent
#define GRAPHICS_PARENT stm->setup.graphics_parent
#define PRINTER_PARENT stm->setup.printer_parent
#define QUEUED_PRINTER_OPTIONS_PARENT stm->setup.queued_printer_options_parent
#define TERMINALTYPE_PARENT stm->setup.terminaltype_parent

#define SHOW_VERSION_PARENT  stm->setup.show_version_parent
#define HELP_ABOUT_PARENT stm->setup.help_about_parent
#define HELP_OVERVIEW_PARENT stm->setup.help_overview_parent
#define CS_HELP_PARENT stm->setup.cs_help_parent /* context sensitive help */
#define ON_HELP_PARENT stm->setup.on_help_parent

#define BIG_FONT 0
#define LITTLE_FONT 1
#define GS_FONT 2

/* external function declarations for DRM symbols */
extern void file_new_cb();
extern void file_open_cb();
extern void file_open_fs_cb();
extern void file_save_cb();
extern void file_saveas_cb();
extern void file_saveas_fs_cb();
extern void file_revert_cb();
extern void file_exit_cb();

extern void edit_copy_cb();
extern void edit_paste_cb();
extern void edit_selectall_cb();

extern void print_page_cb();
extern void print_selected_cb();
extern void print_all_cb();
extern void print_graphics_cb();
extern void finish_printing_cb();
extern void cancel_printing_cb();

extern void copyright_callback();

extern void warn_window_cb();

/*
 * Global data
 */

/* DRM */
globalref MrmHierarchy s_MRMHierarchy;    /* DRM database id */
globalref MrmType *dummy_class;           /* and class var */

globalref Boolean multi_uid;
globalref char decterm_version[];
globalref char build_date[];
globalref char build_time[];

extern STREAM *streams[];

#ifdef HYPERHELP
static void help_system_proc();
static void help_activate_proc();
void help_error();
#define k_help_on_context 1    /* note that these k_help_on_* must match */
#define k_help_on_window  2    /* those in DECTERM_DEF.UIL file!!	 */
#define k_help_on_help    3
#define k_help_on_version 4
#endif /* HYPERHELP */

/*
 * Forward references
 */

Boolean create_queued_options_box();

/*
 * Convert widget id to stream pointer
 */
STREAM *convert_widget_to_stream(w)
Widget w;
{
    ISN isn;  
    STREAM *stm;
    Widget parent;

    /* get widget id of top level application shell */
    parent = find_top_level_widget(w);
    /* search stream database for corresponding stream */
    for (isn = 0; isn <= MaxISN; isn++)
        {
        if (streams[isn] != NULL)
            {
            stm = streams[isn];
            if (stm->parent == parent) return(stm);
            }
        }

    /* didn't find it */

log_error(
"Internal consistency error, couldn't find thread for widget callback\n");
    return(NULL);
}

/*
 *
 * ROUTINE:  ToggleKeyboard
 *
 * ABSTRACT:
 *
 *  Toggle keyboard for CSText widget
 *
 */
XtActionProc ToggleKeyboard(w,event,s,card)
Widget   w;
XEvent   event;
String   *s;
Cardinal *card;
{
    STREAM *stm = convert_widget_to_stream( w );
    if ( stm->terminalType == DECwHebrew )
_DECwTermToggleKeyboard( XtDisplay(w) );
}

/*
 *
 * ROUTINE:  InverseString
 *
 * ABSTRACT: 
 *
 *  Inverses NULL terminated string
 *
 * PARAMETERS:
 *
 *   str - pointer to NULL terminated character string 
 *
 * RETURN VALUE:
 *
 *  Inversed string
 */
void InverseString(str)
char  *str;
{
int  i,len;
char c;

  len = strlen(str);
  for( i = len/2-1; i>=0; i--) {
    c = str[i]; 
    str[i] = str[len-i-1];
    str[len-i-1] = c;
  }
}

/*
 * menubar callbacks
 */

#ifdef SECURE_KEYBOARD
void commands_map_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{     
    STREAM *stm;
    Arg arglist[1];
    Boolean secure_keyboard;

    copyright_callback( w, closure, call_data);	/* because UIL can't */

    stm = convert_widget_to_stream(w);

    if (stm->setup.commands_secure != NULL ) {
	XtSetArg( arglist[0], "secureKeyboard", &secure_keyboard);
	XtGetValues( stm->terminal, arglist, 1 );
	XmToggleButtonSetState(stm->setup.commands_secure,
				secure_keyboard);
    }
}

void commands_secure_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{     
    STREAM *stm;
    Arg arglist[1];
    Boolean secure_keyboard;

    stm = convert_widget_to_stream(w);

    /* if widget was just created, log its widget id */
    if (stm->setup.commands_secure == NULL) {
	stm->setup.commands_secure = w;

	XtSetArg( arglist[0], XmNvisibleWhenOff, TRUE);
	XtSetValues( w, arglist, 1 );

	XtSetArg( arglist[0], "secureKeyboard", &secure_keyboard);
	XtGetValues( stm->terminal, arglist, 1 );
  	XmToggleButtonSetState(w, secure_keyboard);


    }

    if (call_data->reason == XmCR_VALUE_CHANGED) {
        XtSetArg( arglist[0], "secureKeyboard", call_data->set );
        XtSetValues( stm->terminal, arglist, 1 );
    }
}
#endif

void commands_clearlinesofftop_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
    STREAM *stm;
    stm = convert_widget_to_stream(w);

    DECwTermClearTranscript( stm->terminal );
}

void commands_resizewindow_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
    STREAM	*stm;
    int		n;
    int		theight, twidth, width_inc, height_inc;
    Dimension	screen_width, screen_height;
    Dimension	mheight, mbwidth, window_width, window_height, border_width;
    Position    x, y;
    Arg		arglist[5];
    Screen	*screen;
    Boolean	adjust_position, need_to_move;

    stm = convert_widget_to_stream(w);

    n = 0;
    XtSetArg( arglist[n], DECwNdisplayWidth, &twidth); n++;
    XtSetArg( arglist[n], DECwNdisplayHeight, &theight); n++;
    XtSetArg( arglist[n], DECwNdisplayWidthInc, &width_inc); n++;
    XtSetArg( arglist[n], DECwNdisplayHeightInc, &height_inc); n++;
    XtSetArg( arglist[n], DECwNautoAdjustPosition, &adjust_position); n++;
    XtGetValues( stm->terminal, arglist, n);

    n = 0;
    XtSetArg( arglist[n], XmNheight, &mheight); n++;
    XtSetArg( arglist[n], XmNborderWidth, &mbwidth); n++;
    XtGetValues( stm->menubar, arglist, n);

    XtSetArg( arglist[0], XmNwidth, twidth );
    XtSetArg( arglist[1], XmNheight, theight + mheight + mbwidth + mbwidth );
    XtSetValues( stm->widget, arglist, 2);

/* make sure the window is visible (if possible) */

    if (adjust_position)
    {
        n = 0;
        XtSetArg( arglist[n], XmNx, &x); n++;
        XtSetArg( arglist[n], XmNy, &y); n++;
        XtSetArg( arglist[n], XmNwidth, &window_width); n++;
        XtSetArg( arglist[n], XmNheight, &window_height); n++;
        XtSetArg( arglist[n], XmNborderWidth, &border_width); n++;
        XtGetValues( stm->parent, arglist, n);

    /* adjust for the border width.  I've added an extra 2 pixels as a fudge
       factor; for some reason the shell widget doesn't report its true width
       and height */

        window_width += border_width * 2 + 2;
        window_height += border_width * 2 + 2;

        screen = XtScreen( stm->parent );
        screen_width = WidthOfScreen( screen );
        screen_height = HeightOfScreen( screen );

        n = 0;
        need_to_move = False;

    /* see if the window will fit on the screen */

        if ( x + window_width >= screen_width )
            {
            x = screen_width - window_width;
            need_to_move = True;
            }
        if ( x < 0 )
            {
            x = 0;
            need_to_move = True;
            }
        if ( y + window_height >= screen_height )
            {
            y = screen_height - window_height;
            need_to_move = True;
            }
        if ( y < (int)mheight )
            {
            y = mheight;
            need_to_move = True;
            }
        if ( need_to_move )
            {
            XtSetArg( arglist[n], XmNx, x ); n++;
            XtSetArg( arglist[n], XmNy, y ); n++;
            XtSetValues( stm->parent, arglist, n );
            }
    }
}

void commands_resetterminal_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
    STREAM *stm;
    stm = convert_widget_to_stream(w);

    DECwTermReset(stm->terminal);
}

void commands_cleardisplay_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
    STREAM *stm;
    stm = convert_widget_to_stream(w);
    
    DECwTermClearDisplay(stm->terminal);
}                            


void commands_clearcomm_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
    STREAM *stm;
    stm = convert_widget_to_stream(w);
    
    DECwTermClearComm(stm->terminal);
    pty_clear_comm( stm );    
}                            

void commands_convertdisplay_cre_cb(w, closure, call_data)
Widget w;
caddr_t closure;

XmAnyCallbackStruct *call_data;
{
    STREAM *stm;
    stm = convert_widget_to_stream(w);

    stm->setup.convert_display = w;
}

void commands_convertdisplay_act_cb(w, closure, call_data)
Widget w;
caddr_t closure;

XmAnyCallbackStruct *call_data;
{
    STREAM *stm;
    stm = convert_widget_to_stream(w);

    _DECwTermRedisplay(stm->terminal);
}

void commands_map2_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
    STREAM *stm;
    stm = convert_widget_to_stream(w);

    if ( stm->setup.copy_dir ) {
	if ( stm->terminalType == DECwHebrew )
	    XtSetSensitive( stm->setup.copy_dir, TRUE );
	else
	    XtSetSensitive( stm->setup.copy_dir, FALSE );
    }
    if (( stm->terminalType == DECwHebrew ) &&
	_DECwTerm7bitMode( stm->terminal ))
	XtSetSensitive(stm->setup.convert_display, TRUE);
    else
	XtSetSensitive(stm->setup.convert_display, FALSE);
    copyright_callback( w, closure, call_data );
}

/* callbacks for selection direction */
void copy_dir_cre_cb( w, closure, call_data )
Widget w;
caddr_t closure;
XmAnyCallbackStruct *call_data;
{
    STREAM *stm = convert_widget_to_stream( w );
    stm->setup.copy_dir = w;
}

void copy_dir_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
    STREAM *stm;
    stm = convert_widget_to_stream(w);

    if ( _DECwTermCopyDirection( stm->terminal )) {
	XtSetSensitive( stm->setup.copy_dir_rtol, FALSE );
	XtSetSensitive( stm->setup.copy_dir_ltor, TRUE );
    } else {
	XtSetSensitive( stm->setup.copy_dir_rtol, TRUE );
	XtSetSensitive( stm->setup.copy_dir_ltor, FALSE );
    }
}

void copy_dir_rtol_cre_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmAnyCallbackStruct *call_data;
{
    STREAM		*stm;
    stm = convert_widget_to_stream(w);

    stm->setup.copy_dir_rtol = w;
}

void copy_dir_ltor_cre_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmAnyCallbackStruct *call_data;
{
    STREAM *stm;
    stm = convert_widget_to_stream(w);

    stm->setup.copy_dir_ltor = w;
}

void copy_dir_rtol_act_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmAnyCallbackStruct *call_data;
{
    Arg arglist[1];
    STREAM		*stm;
    stm = convert_widget_to_stream(w);

    _DECwTermSetCopyDirection( stm->terminal, TRUE );
    XtSetArg( arglist[0], DECwNselectionRtoL, TRUE );
    XtSetValues( stm->terminal, arglist, 1 );
}

void copy_dir_ltor_act_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmAnyCallbackStruct *call_data;
{
    Arg arglist[1];
    STREAM *stm;
    stm = convert_widget_to_stream(w);

    _DECwTermSetCopyDirection( stm->terminal, FALSE );
    XtSetArg( arglist[0], DECwNselectionRtoL, FALSE );
    XtSetValues( stm->terminal, arglist, 1 );
}

#if 0
void popup_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
    STREAM *stm;
    stm = convert_widget_to_stream(w);

    if ( _DECwTermCopyDirection(stm->terminal )) {
	XtSetSensitive( stm->setup.popup_rtol, FALSE );
	XtSetSensitive( stm->setup.popup_ltor, TRUE );
    } else {
	XtSetSensitive( stm->setup.popup_rtol, TRUE );
	XtSetSensitive( stm->setup.popup_ltor, FALSE );
    }
}

void popup_rtol_cre_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmAnyCallbackStruct *call_data;
{
    STREAM *stm;
    stm = convert_widget_to_stream(w);

    stm->setup.popup_rtol = w;
}

void popup_ltor_cre_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmAnyCallbackStruct *call_data;
{
    STREAM *stm;
    stm = convert_widget_to_stream(w);

    stm->setup.popup_ltor = w;
}

void popup_rtol_act_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmAnyCallbackStruct *call_data;
{
    Arg arglist[1];
    STREAM *stm;
    stm = convert_widget_to_stream(w);

    _DECwTermSetCopyDirection( stm->terminal, TRUE );
    XtSetArg( arglist[0], DECwNselectionRtoL, TRUE );
    XtSetValues( stm->terminal, arglist, 1 );
}

void popup_ltor_act_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmAnyCallbackStruct *call_data;
{
    Arg arglist[1];
    STREAM *stm;
    stm = convert_widget_to_stream(w);

    _DECwTermSetCopyDirection( stm->terminal, FALSE );
    XtSetArg( arglist[0], DECwNselectionRtoL, FALSE );
    XtSetValues( stm->terminal, arglist, 1 );
}

#endif

void help_show_version_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;          
{
    STREAM *stm;
    stm = convert_widget_to_stream(w);
	
    /* check if we need to fetch the widgets from disk */
    if (SHOW_VERSION_PARENT == 0)
        {

        /* fetch dialog box from DRM */
        if (MrmFetchWidget
               (
               s_MRMHierarchy,
               "help_show_version_db",
               stm->parent,
               & SHOW_VERSION_PARENT,
               & dummy_class       
               )
           != MrmSUCCESS)
            {
            log_error( "Unable to fetch widget definitions from DRM\n");
	    return;
            }
        }

    /* display it */
    XtUnmanageChild(SHOW_VERSION_PARENT);
    XtManageChild(SHOW_VERSION_PARENT);
}                            

void show_version_label_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;          
{
    XmString value_cs;
    Arg al[2];
    char date_time[100];

    sprintf(date_time, "%s, %s %s", decterm_version, build_date, build_time);

    /* label widget was just created, initialize it */

    value_cs = XmStringCreate(date_time, XmSTRING_DEFAULT_CHARSET);
    XtSetArg (al[0], XmNlabelString, value_cs);
    XtSetValues (w, al, 1);
    XmStringFree( value_cs );
}

void show_version_acknowledge_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;          
{
    STREAM *stm;
    stm = convert_widget_to_stream(w);

    /* take down show version dialog box */
    XtUnmanageChild(SHOW_VERSION_PARENT);
}


void help_about_cb(w, closure, call_data)
Widget w;                         
caddr_t closure;
XmAnyCallbackStruct *call_data;
{
    STREAM *stm;
    stm = convert_widget_to_stream(w);

    /* Call up show version if Shift down */
    if (call_data->event->xbutton.state & ShiftMask) {
	help_show_version_cb( w, closure, call_data );
	return;
    }

    /* check if we need to fetch the widgets from disk */
    if (HELP_ABOUT_PARENT == 0)
        {
	Arg		arglist[1];
        XmString	library;

	/* over-ride the library name */
	

	library = XmStringCreate( 

#ifdef LANGUAGE_SWITCHING_TOOLKIT
				DECTERM_APPL_CLASS
#else
#ifdef VMS_DECTERM
 		                 "SYS$HELP:DECW$TERMINAL"
#else
 		                 "/usr/lib/X11/help/dxterm"
#endif VMS_DECTERM
#endif
                               , XmSTRING_DEFAULT_CHARSET);
	XtSetArg(arglist[0],DXmNlibrarySpec,library);

        /* fetch dialog box from DRM */
        if (MrmFetchWidgetOverride
               (
               s_MRMHierarchy,
               "help_about_hb",
               stm->parent,
	       NULL,
	       arglist,
	       1,
               & HELP_ABOUT_PARENT,
               & dummy_class       
               )
           != MrmSUCCESS)
            {
            log_error( "Unable to fetch widget definitions from DRM\n");
	    return;
            }
	XmStringFree( library );
        }
    XtUnmanageChild(HELP_ABOUT_PARENT);
    XtManageChild(HELP_ABOUT_PARENT);
}                            

void help_overview_cb(w, closure, call_data)
Widget w;                         
caddr_t closure;
caddr_t call_data;
{
    STREAM *stm;
    stm = convert_widget_to_stream(w);
	
    /* check if we need to fetch the widgets from disk */
    if (HELP_OVERVIEW_PARENT == 0)
        {
	Arg		arglist[1];
        XmString	library;

	/* override the library name */

	library = XmStringCreate(
#ifdef LANGUAGE_SWITCHING_TOOLKIT
 		                 DECTERM_APPL_CLASS
#else
#ifdef VMS_DECTERM
		                 "SYS$HELP:DECW$TERMINAL"
#else
 		                 "/usr/lib/X11/help/dxterm"
#endif VMS_DECTERM
#endif
                               , XmSTRING_DEFAULT_CHARSET); 
	XtSetArg(arglist[0],DXmNlibrarySpec,library);

        /* fetch dialog box from DRM */
        if (MrmFetchWidgetOverride
               (
               s_MRMHierarchy,
               "help_overview_hb",
               stm->parent,
	       NULL,
	       arglist,
	       1,
               & HELP_OVERVIEW_PARENT,
               & dummy_class       
               )
           != MrmSUCCESS)
            {
            log_error( "Unable to fetch widget definitions from DRM\n");
	    return;
            }
	XmStringFree( library );
        }
    XtUnmanageChild(HELP_OVERVIEW_PARENT);
    XtManageChild(HELP_OVERVIEW_PARENT);
}                            

void cs_help_cb( w, object_help_key, callbackdata )
    Widget w;
    XmString *object_help_key;
    XmAnyCallbackStruct *callbackdata;
{
    STREAM *stm;
    Arg		arglist[2];

    stm = convert_widget_to_stream(w);

    /* check if we need to fetch the widgets from disk */
#if 0
    if (CS_HELP_PARENT == 0)
#else
    if ( TRUE )
#endif
	{
        XmString	library;

	/* override the library name */

	library = XmStringCreate(
#ifdef LANGUAGE_SWITCHING_TOOLKIT
				DECTERM_APPL_CLASS
#else
#ifdef VMS_DECTERM
                                 "SYS$HELP:DECW$TERMINAL"
#else
                                 "/usr/lib/X11/help/dxterm"
#endif VMS_DECTERM
#endif
                               , XmSTRING_DEFAULT_CHARSET);
	XtSetArg(arglist[0],DXmNlibrarySpec,library);
	XtSetArg(arglist[1],DXmNfirstTopic, object_help_key);

	/* fetch dialog box from DRM */
	if (MrmFetchWidgetOverride
		(
		s_MRMHierarchy,
		"cs_help_hb",
		stm->parent,
		NULL,
		arglist,
		2,
		& CS_HELP_PARENT,
		& dummy_class
		)
	    != MrmSUCCESS)
	     {
	     log_error( "Unable to fetch widget definitions from DRM\n");
	     return;
	     }
	XmStringFree( library );
	}
    else
	{  /* we already have the widget, so simply make that topic appear */
	XtSetArg( arglist[0], DXmNfirstTopic, object_help_key );
	XtSetValues( CS_HELP_PARENT, arglist, 1 );
	}
    XtUnmanageChild(CS_HELP_PARENT);
    XtManageChild(CS_HELP_PARENT);
}

void on_help_cb(w, closure, call_data)
Widget w;                         
caddr_t closure;
caddr_t call_data;
{
    STREAM *stm;
    stm = convert_widget_to_stream(w);
	
    /* check if we need to fetch the widgets from disk */
    if (ON_HELP_PARENT == 0)
        {
	Arg		arglist[1];
        XmString	library;

	/* override the library name */

	library = XmStringCreate(
#ifdef LANGUAGE_SWITCHING_TOOLKIT
 		                 DECTERM_APPL_CLASS
#else
#ifdef VMS_DECTERM
		                 "SYS$HELP:DECW$TERMINAL"
#else
 		                 "/usr/lib/X11/help/dxterm"
#endif VMS_DECTERM
#endif
                               , XmSTRING_DEFAULT_CHARSET); 
	XtSetArg(arglist[0],DXmNlibrarySpec,library);

        /* fetch dialog box from DRM */
        if (MrmFetchWidgetOverride
               (
               s_MRMHierarchy,
               "on_help_hb",
               stm->parent,
	       NULL,
	       arglist,
	       1,
               & ON_HELP_PARENT,
               & dummy_class       
               )
           != MrmSUCCESS)
            {
            log_error( "Unable to fetch widget definitions from DRM\n");
	    return;
            }
	XmStringFree( library );
        }
    XtUnmanageChild(ON_HELP_PARENT);
    XtManageChild(ON_HELP_PARENT);
}                            

void on_context_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;          
{
    Widget topWidget = find_top_level_widget(w);
    DXmHelpOnContext(topWidget, FALSE);
}   

void decwterm_help_cb(w, stm, call_data)
    Widget w;
    STREAM *stm;
    caddr_t call_data;
{

#ifdef HYPERHELP

	if ( stm->help_context == NULL )
	{
	    /* Open a HyperHelp context for this stream.
	     * Open it *only* after user invoke help from DECterm.
	     */
	    DXmHelpSystemOpen( &stm->help_context,
			       stm->parent,
			       DECTERM_APPL_CLASS,
			       help_error,
			       "Help System Error" );
	}

	DXmHelpSystemDisplay( stm->help_context, DECTERM_APPL_CLASS, "topic",
			      "decterm", help_error, "Help System Error" );
#else

/* it's too complicated trying to get the help key from the UIL file, so
   for now we'll just hard code it as "DECterm" */

    XmString key; 
#ifdef ORIGINAL_CODE
 * key = DwtLatin1String( "DECterm" )
#endif

    key = XmStringCreate("Overview", XmSTRING_DEFAULT_CHARSET);
    cs_help_cb( w, key, call_data );
    XmStringFree( key );

#endif /* HYPERHELP */

}    
    
/****************************
 *  window setup callbacks  *
 ****************************/
                               
void setup_window_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
	STREAM *stm;
	int n;
	DECwFontSetSelection fontSetSelection;
	Boolean autoResizeTerminal, autoResizeWindow, terminalDriverResize,
	  condensedFont, adjustFontSizes;
	char *title, *iconName;
	Boolean bigfontselection;
	char *big_font_name, *big_font_other_name=NULL;
	Boolean littlefontselection;
	char *little_font_name, *little_font_other_name=NULL;
	Boolean gsfontselection;
	char *gs_font_name, *gs_font_other_name=NULL;
        char big_font_default_name[MAX_FONT_NAME_SIZE],
	     little_font_default_name[MAX_FONT_NAME_SIZE],
	     gs_font_default_name[MAX_FONT_NAME_SIZE];
	Arg arglist[30];
        int rows_num, columns_num;
        char rows_text[4], columns_text[4];
	Atom titleEncoding, iconNameEncoding;
	Atom XA_COMPOUND_TEXT =
	    XInternAtom( XtDisplay( w ), "COMPOUND_TEXT", FALSE );
	XmString title_cs = NULL;
	XmString iconName_cs = NULL;
	long count, status;
	XmString label_string;

        stm = convert_widget_to_stream(w);

        /* get and save current setup state from DECterm widget */
    	n = 0;
	XtSetArg( arglist[n], DECwNfontSetSelection, &fontSetSelection); n++;
	XtSetArg( arglist[n], DECwNbigFontSetSelection, &bigfontselection); n++;
	XtSetArg( arglist[n], DECwNbigFontSetName, &big_font_name); n++;
	XtSetArg( arglist[n], DECwNbigFontOtherName, &big_font_other_name); n++;
	XtSetArg( arglist[n], DECwNlittleFontSetSelection, &littlefontselection); n++;
	XtSetArg( arglist[n], DECwNlittleFontSetName, &little_font_name); n++;
	XtSetArg( arglist[n], DECwNlittleFontOtherName, &little_font_other_name); n++;
	XtSetArg( arglist[n], DECwNgsFontSetSelection, &gsfontselection); n++;
	XtSetArg( arglist[n], DECwNgsFontSetName, &gs_font_name); n++;
	XtSetArg( arglist[n], DECwNgsFontOtherName, &gs_font_other_name); n++;
    	XtSetArg( arglist[n], DECwNautoResizeTerminal, &autoResizeTerminal);
	  n++;
    	XtSetArg( arglist[n], DECwNautoResizeWindow, &autoResizeWindow); n++;
	XtSetArg( arglist[n], DECwNterminalDriverResize, &terminalDriverResize);
	  n++;
    	XtSetArg( arglist[n], DECwNcondensedFont, &condensedFont); n++;
	XtSetArg( arglist[n], DECwNadjustFontSizes, &adjustFontSizes); n++;
        XtSetArg( arglist[n], DECwNrows, &rows_num ); n++;
        XtSetArg( arglist[n], DECwNcolumns, &columns_num ); n++;
	XtGetValues( stm->terminal, arglist, n);
	
	n = 0;
	XtSetArg( arglist[n], XtNtitle, &title ); n++;
	XtSetArg( arglist[n], XtNiconName, &iconName ); n++;
	XtSetArg( arglist[n], XtNtitleEncoding, &titleEncoding ); n++;
	XtSetArg( arglist[n], XtNiconNameEncoding, &iconNameEncoding ); n++;
	XtGetValues( stm->parent, arglist, n );

        /* check if we need to fetch the widgets from disk */
	if (WINDOW_PARENT == 0)
            {

            /* fetch dialog box from DRM */
            if (MrmFetchWidget
                   (
                   s_MRMHierarchy,
                   "setup_window_db",
                   stm->parent,
                   & WINDOW_PARENT,
                   & dummy_class       
                   )
               != MrmSUCCESS)
                {
                log_error(
			"Unable to fetch widget definitions from DRM\n");
	        return;
                }
            }

	/* initialize widgets to current state */
	XmToggleButtonSetState(stm->setup.window_big_font_set,
		fontSetSelection == DECwBigFont);
	XmToggleButtonSetState(stm->setup.window_little_font_set,
		fontSetSelection == DECwLittleFont);
	if ( multi_uid || stm->terminalType == DECwKanji ) {
	XmToggleButtonSetState(stm->setup.window_fine_font_set,
		fontSetSelection == DECwFineFont);
	}
	if ( multi_uid || !IsAsianTerminalType(stm->terminalType) )
	XmToggleButtonSetState(stm->setup.window_gs_font_set,
		fontSetSelection == DECwGSFont);
	if ( !IsAsianTerminalType(stm->terminalType) ) {

	XmToggleButtonSetState(stm->setup.window_normal_font,
		( ! condensedFont ) && ( ! adjustFontSizes ) );
	XmToggleButtonSetState(stm->setup.window_condensed_font,
		condensedFont && ( ! adjustFontSizes ) );
	XmToggleButtonSetState(stm->setup.window_variable_font,
		adjustFontSizes );
	}

       /* Find big font default name - taking in consideration of screen dpi */

        strcpy( big_font_default_name, DEFAULT_BIG_FONT_SET_NAME );

       /* Use a 14 point font instead of an 18 point font on 100 DPI monitors */
        if ( GetDisplayDPI( XtDisplay(stm->terminal) ) > 90 )
        {
             DLFDSetFontNameField( DLFD_POINT_SIZE, big_font_default_name, 
    				3, "140" );
        }
	if ( bigfontselection && 
		strcmp(big_font_name, big_font_default_name) )
	{
	     /* use user specified big font */

	    XmToggleButtonSetState(stm->setup.window_big_font_default, 0);
	    XmToggleButtonSetState(stm->setup.window_big_font_other, 1);
	}
	else
	{
	     /* use default big font */

	    XmToggleButtonSetState(stm->setup.window_big_font_default, 1);
	    XmToggleButtonSetState(stm->setup.window_big_font_other, 0);
	}
	if ( big_font_other_name == NULL || big_font_other_name[0] == '\0' )
	{
	    XmTextSetString(stm->setup.window_big_font_other_name, 
				&big_font_default_name[0]);
	}
	else
	{
	    XmTextSetString(stm->setup.window_big_font_other_name,
			big_font_other_name);
	}


     /* Find little font default name - taking in consideration of screen dpi */

        strcpy( little_font_default_name, DEFAULT_LITTLE_FONT_SET_NAME );

        /* Use a 10 point font instead of a 14 point font on 100 DPI monitors */
        if ( GetDisplayDPI( XtDisplay(stm->terminal) ) > 90 )
        {
            DLFDSetFontNameField( DLFD_POINT_SIZE, little_font_default_name, 
    				3, "100" );
        }
	if ( littlefontselection && 
		strcmp(little_font_name, little_font_default_name) )
	{
	     /* use user specified little font */

	    XmToggleButtonSetState(stm->setup.window_little_font_default, 0);
	    XmToggleButtonSetState(stm->setup.window_little_font_other, 1);
	}
	else
	{
	     /* use default little font */

	    XmToggleButtonSetState(stm->setup.window_little_font_default, 1);
	    XmToggleButtonSetState(stm->setup.window_little_font_other, 0);
	}
	if ( little_font_other_name == NULL ||
	     little_font_other_name[0] == '\0')
	{
	    XmTextSetString(stm->setup.window_little_font_other_name, 
				&little_font_default_name[0]);
	}
	else
	{
	    XmTextSetString(stm->setup.window_little_font_other_name,
			little_font_other_name);
	}

        strcpy( gs_font_default_name, DEFAULT_GS_FONT_SET_NAME );
	if ( gsfontselection && 
		strcmp(gs_font_name, gs_font_default_name) )
	{
	     /* use user specified gs font */

	    XmToggleButtonSetState(stm->setup.window_gs_font_default, 0);
	    XmToggleButtonSetState(stm->setup.window_gs_font_other, 1);
	}
	else
	{
	     /* use default gs font */

	    XmToggleButtonSetState(stm->setup.window_gs_font_default, 1);
	    XmToggleButtonSetState(stm->setup.window_gs_font_other, 0);
	}
	if ( gs_font_other_name == NULL ||
		gs_font_other_name[0] == '\0' )
	{
	    XmTextSetString(stm->setup.window_gs_font_other_name, 
		&gs_font_default_name[0]);
	}
	else
	{
	    XmTextSetString(stm->setup.window_gs_font_other_name,
			gs_font_other_name);
	}

	XmToggleButtonSetState(stm->setup.window_auto_resize_terminal,
 		autoResizeTerminal );
	XmToggleButtonSetState(stm->setup.window_auto_resize_window,
		autoResizeWindow );

#if !defined (VXT_DECTERM)
	XmToggleButtonSetState(stm->setup.window_terminal_driver_resize,
		terminalDriverResize );
#endif

        sprintf(rows_text, "%3d", rows_num);
        XmTextSetString(stm->setup.window_rows_text, rows_text);
        sprintf(columns_text, "%3d", columns_num);
        XmTextSetString(stm->setup.window_columns_text, columns_text);

	title_cs = ( titleEncoding == XA_COMPOUND_TEXT ) ?
		   ( XmString )XmCvtCTToXmString( title ) :
		   ( XmString )DXmCvtFCtoCS( title, &count, &status );
	iconName_cs = ( iconNameEncoding == XA_COMPOUND_TEXT ) ?
		      ( XmString )XmCvtCTToXmString( iconName ) :
		      ( XmString )DXmCvtFCtoCS( iconName, &count, &status );

	if ( multi_uid || IsAsianOrHebrewTermType(stm->terminalType) ) {
	    DXmCSTextSetString( stm->setup.window_title, title_cs );
	    DXmCSTextSetString( stm->setup.window_icon_name, iconName_cs );
	} else {
	    title = DXmCvtCStoFC( title_cs, &count, &status );
	    iconName = DXmCvtCStoFC( iconName_cs, &count, &status );
	    XmTextSetString( stm->setup.window_title, title );
	    XmTextSetString( stm->setup.window_icon_name, iconName );
	    XtFree( title );
	    XtFree( iconName );
	}
	XmStringFree( title_cs );
	XmStringFree( iconName_cs );
	if ( multi_uid ) {
	    if ( !IsAsianTerminalType(stm->terminalType) ) {
		XtSetSensitive( stm->setup.window_gs_font_set, True );
		XtSetSensitive( stm->setup.window_normal_font, True );
		XtSetSensitive( stm->setup.window_condensed_font, True );
		XtSetSensitive( stm->setup.window_variable_font, True );
		label_string = XmStringCreateSimple( "  48" );
		XtSetArg( arglist[0], XmNlabelString, label_string );
		XtSetValues( stm->setup.window_rows_48_button, arglist, 1 );
		XmStringFree( label_string );
		label_string = XmStringCreateSimple( "  72" );
		XtSetArg( arglist[0], XmNlabelString, label_string );
		XtSetValues( stm->setup.window_rows_72_button, arglist, 1 );
		XmStringFree( label_string );
		label_string = XmStringCreateSimple( " 132" );
		XtSetArg( arglist[0], XmNlabelString, label_string );
		XtSetValues( stm->setup.window_columns_132_button, arglist, 1 );
		XmStringFree( label_string );
	    } else {
		XtSetSensitive( stm->setup.window_gs_font_set, False );
		XtSetSensitive( stm->setup.window_normal_font, False );
		XtSetSensitive( stm->setup.window_condensed_font, False );
		XtSetSensitive( stm->setup.window_variable_font, False );
		label_string = XmStringCreateSimple( "  32" );
		XtSetArg( arglist[0], XmNlabelString, label_string );
		XtSetValues( stm->setup.window_rows_48_button, arglist, 1 );
		XmStringFree( label_string );
		label_string = XmStringCreateSimple( "  40" );
		XtSetArg( arglist[0], XmNlabelString, label_string );
		XtSetValues( stm->setup.window_rows_72_button, arglist, 1 );
		XmStringFree( label_string );
		label_string = XmStringCreateSimple( " 126" );
		XtSetArg( arglist[0], XmNlabelString, label_string );
		XtSetValues( stm->setup.window_columns_132_button, arglist, 1 );
		XmStringFree( label_string );
	    }
	    if ( stm->terminalType == DECwKanji )
		XtSetSensitive( stm->setup.window_fine_font_set, True );
	    else
		XtSetSensitive( stm->setup.window_fine_font_set, False );
	}

        /* put up window dialog box and return */
	XtUnmanageChild(WINDOW_PARENT);
	XtManageChild(WINDOW_PARENT);
	return;
}          
  
void window_ok_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{     
	STREAM *stm;
        stm = convert_widget_to_stream(w);

	window_set_resources( stm );
#ifdef VXT_DECTERM
	if ( VxtSwPhysicalMemory() || !(VxtSwPagefileEnabled()) )
	{
	    XtDestroyWidget(WINDOW_PARENT);
	    WINDOW_PARENT = NULL;
	}
	else 
#endif VXT_DECTERM
	XtUnmanageChild(WINDOW_PARENT);
}

void window_apply_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{     
	STREAM *stm;
        stm = convert_widget_to_stream(w);

	window_set_resources( stm );
}

void window_cancel_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
	STREAM *stm;
        stm = convert_widget_to_stream(w);

#ifdef VXT_DECTERM
	if ( VxtSwPhysicalMemory() || !(VxtSwPagefileEnabled()) )
	{
	    XtDestroyWidget(WINDOW_PARENT);
	    WINDOW_PARENT = NULL;
	}
	else 
#endif VXT_DECTERM
	XtUnmanageChild( WINDOW_PARENT );
}

window_set_resources( stm )
    STREAM *stm;
{
    Arg arglist[30];
    int n;
    int big_font_set_button, condensedFont_button,
      little_font_set_button, gs_font_set_button,
      adjustFontSizes_button, autoResizeTerminal_button,
      autoResizeWindow_button, terminalDriverResize_button;
    int fine_font_set_button;
    int rows_num, rows_num_old, columns_num, columns_num_old;
    char *rows_text, *columns_text, *title_text, *icon_text;
    Boolean rows_or_columns_changed = FALSE;
    XmString title_text_cs, icon_text_cs, title_text_var, icon_text_var;
    XmStringCharSet title_charset, icon_charset;
    long count, status;

    Boolean big_font_default_button, big_font_other_button;
    Boolean little_font_default_button, little_font_other_button;
    Boolean gs_font_default_button, gs_font_other_button;

    char *big_font_other_name;
    char *little_font_other_name;
    char *gs_font_other_name;
    char big_font_default_name[MAX_FONT_NAME_SIZE],
	little_font_default_name[MAX_FONT_NAME_SIZE],
	gs_font_default_name[MAX_FONT_NAME_SIZE];

    char string[256];

    SetWMHints(stm, FALSE);     /* Turn off hints so widget can resize freely */

    /* get number of rows and columns from simple text widget */
    n = 0;
    XtSetArg( arglist[n], DECwNrows, &rows_num_old ); n++;
    XtSetArg( arglist[n], DECwNcolumns, &columns_num_old ); n++;
    XtGetValues( stm->terminal, arglist, n);
    rows_num = rows_num_old;
    rows_text = XmTextGetString(stm->setup.window_rows_text);
    sscanf(rows_text, "%3d", &rows_num);
    XtFree(rows_text);
    if (rows_num < 1) rows_num = rows_num_old;
    if (rows_num > 255) rows_num = 255;
    if (rows_num != rows_num_old) rows_or_columns_changed = TRUE;

    columns_num = columns_num_old;
    columns_text = XmTextGetString(stm->setup.window_columns_text);
    sscanf(columns_text, "%3d", &columns_num);
    XtFree(columns_text);
    if (columns_num < 1) columns_num = columns_num_old;
    if (columns_num > 255) columns_num = 255;
    if (columns_num != columns_num_old) rows_or_columns_changed = TRUE;

    /* read button states */
    big_font_set_button =               
      XmToggleButtonGetState(stm->setup.window_big_font_set);
    little_font_set_button =               
      XmToggleButtonGetState(stm->setup.window_little_font_set);
    if ( multi_uid || stm->terminalType == DECwKanji )
    fine_font_set_button =
      XmToggleButtonGetState(stm->setup.window_fine_font_set);

    if ( multi_uid || !IsAsianTerminalType(stm->terminalType) ) {
    gs_font_set_button =
      XmToggleButtonGetState(stm->setup.window_gs_font_set);
    condensedFont_button =
      XmToggleButtonGetState(stm->setup.window_condensed_font);
    adjustFontSizes_button =
      XmToggleButtonGetState(stm->setup.window_variable_font);
    }

    big_font_default_button =               
      XmToggleButtonGetState(stm->setup.window_big_font_default);
    big_font_other_button =               
      XmToggleButtonGetState(stm->setup.window_big_font_other);
    big_font_other_name =               
      XmTextGetString(stm->setup.window_big_font_other_name);
    little_font_default_button =               
      XmToggleButtonGetState(stm->setup.window_little_font_default);
    little_font_other_button =               
      XmToggleButtonGetState(stm->setup.window_little_font_other);
    little_font_other_name =               
      XmTextGetString(stm->setup.window_little_font_other_name);
    gs_font_default_button =               
      XmToggleButtonGetState(stm->setup.window_gs_font_default);
    gs_font_other_button =               
      XmToggleButtonGetState(stm->setup.window_gs_font_other);
    gs_font_other_name =               
      XmTextGetString(stm->setup.window_gs_font_other_name);

    /* remove leading and trailing tabs and spaces in the font names */

#define STR$_COLLAPSE	2	/* Discard all spaces and tabs */
#define STR$_LEADING	8	/* Discard leading spaces and tabs */
#define STR$_COMPRESS	16	/* Reduce spaces and tabs to a single space */
#define STR$_UPPER	32	/* Convert lowercase to uppercase */
#define	STR$_TRAILING	128	/* Discard trailing spaces and tabs */
#define STR$_TRIM	(STR$_LEADING|STR$_COMPRESS|STR$_TRAILING)

    {
    static int StrEdit();
    StrEdit( &string[0], big_font_other_name, STR$_LEADING | STR$_TRAILING );
    XmStringFree( big_font_other_name );
    big_font_other_name = XtNewString( string );

    StrEdit( &string[0], little_font_other_name, STR$_LEADING | STR$_TRAILING );
    XmStringFree( little_font_other_name );
    little_font_other_name = XtNewString( string );

    StrEdit( &string[0], gs_font_other_name, STR$_LEADING | STR$_TRAILING );
    XmStringFree( gs_font_other_name );
    gs_font_other_name = XtNewString( string );
    }

    autoResizeTerminal_button =
      XmToggleButtonGetState(stm->setup.window_auto_resize_terminal);
    autoResizeWindow_button =
      XmToggleButtonGetState(stm->setup.window_auto_resize_window);

#if !defined (VXT_DECTERM)
    terminalDriverResize_button =
      XmToggleButtonGetState(stm->setup.window_terminal_driver_resize);
#endif

    /* read the titles */
    if ( multi_uid || IsAsianOrHebrewTermType(stm->terminalType) ) {
    title_text_cs = DXmCSTextGetString( stm->setup.window_title );
    if ( !title_text_cs )
	title_text_cs = DXmCvtFCtoCS( "", &count, &status );
    icon_text_cs = DXmCSTextGetString( stm->setup.window_icon_name );
    if ( !icon_text_cs )
	icon_text_cs = DXmCvtFCtoCS( "", &count, &status );
    } else {
    title_text = XmTextGetString(stm->setup.window_title);
    icon_text = XmTextGetString(stm->setup.window_icon_name);
    }

    /* set the resources (whether changed or not) */
    n = 0;
    if (rows_or_columns_changed)
        {
        XtSetArg( arglist[n], DECwNrows, rows_num ); n++;
        XtSetArg( arglist[n], DECwNcolumns, columns_num ); n++;
        }
    if ( big_font_set_button )
        XtSetArg( arglist[n], DECwNfontSetSelection, DECwBigFont);
    else if ( little_font_set_button )
        XtSetArg( arglist[n], DECwNfontSetSelection, DECwLittleFont);
    else if (( multi_uid || !IsAsianTerminalType(stm->terminalType) )
		&& gs_font_set_button )
	XtSetArg( arglist[n], DECwNfontSetSelection, DECwGSFont);
    else if ( multi_uid || stm->terminalType == DECwKanji )
        XtSetArg( arglist[n], DECwNfontSetSelection, DECwFineFont);
    n++;
    if ( multi_uid || !IsAsianTerminalType(stm->terminalType) ) {
    if ( ! adjustFontSizes_button )
	{
        if ( gs_font_set_button )  /* ignore condensed request if GS font */
            {
	        XtSetArg( arglist[n], DECwNcondensedFont, False ); n++;
            }
	else
            {
	        XtSetArg( arglist[n], DECwNcondensedFont, condensedFont_button ); n++;
            }
	XtSetArg( arglist[n], DECwNadjustFontSizes, False ); n++;
	}
    else
	{
        XtSetArg( arglist[n], DECwNadjustFontSizes, True ); n++;
	if ((columns_num > 80) && !gs_font_set_button)
	    {
		XtSetArg( arglist[n], DECwNcondensedFont, True ); n++;
	    }
	else
	    {
		XtSetArg( arglist[n], DECwNcondensedFont, False ); n++;
	    }
	}
    }

    /* init local default font variables */

    big_font_default_name[0] = '\0';
    little_font_default_name[0] = '\0';
    gs_font_default_name[0] = '\0';

    /* Find big font default name - taking in consideration of screen dpi */

    strcpy( big_font_default_name, DEFAULT_BIG_FONT_SET_NAME );

    /* Use a 14 point font instead of an 18 point font on 100 DPI monitors */
    if ( GetDisplayDPI( XtDisplay(stm->terminal) ) > 90 )
    {
        DLFDSetFontNameField( DLFD_POINT_SIZE, big_font_default_name, 
    				3, "140" );
    }

    if ( big_font_default_button )
    {

        XtSetArg( arglist[n], DECwNbigFontSetName, &big_font_default_name[0]);
	n++;
        XtSetArg( arglist[n], DECwNbigFontSetSelection, False);
	n++;
    }
    else
    {	
	if ( big_font_other_name == NULL || big_font_other_name[0] == '\0' ||
		!strcmp(big_font_other_name, big_font_default_name) )
	{
            XtSetArg( arglist[n], DECwNbigFontSetName, 
    			&big_font_default_name[0]);    n++;
            XtSetArg( arglist[n], DECwNbigFontSetSelection, False);    n++;
	}
	else
	{
            XtSetArg( arglist[n], DECwNbigFontSetName, big_font_other_name); 
	    n++;
            XtSetArg( arglist[n], DECwNbigFontSetSelection, True);    n++;
	}
    }
    if ( big_font_other_name == NULL || big_font_other_name[0] == '\0' )
    {
        XtSetArg( arglist[n], DECwNbigFontOtherName, 
			"");
	n++;
    }
    else
    {
        XtSetArg( arglist[n], DECwNbigFontOtherName, big_font_other_name);
	n++;
    }

    /* Find little font default name - taking in consideration of screen dpi */

    strcpy( little_font_default_name, DEFAULT_LITTLE_FONT_SET_NAME );

    /* Use a 10 point font instead of a 14 point font on 100 DPI monitors */
    if ( GetDisplayDPI( XtDisplay(stm->terminal) ) > 90 )
    {
        DLFDSetFontNameField( DLFD_POINT_SIZE, little_font_default_name, 
    				3, "100" );
    }
    if ( little_font_default_button )
    {
        XtSetArg( arglist[n], DECwNlittleFontSetName, &little_font_default_name[0]);
	n++;
        XtSetArg( arglist[n], DECwNlittleFontSetSelection, False);
	n++;
    }
    else
    {	
	if ( little_font_other_name == NULL ||
		little_font_other_name[0] == '\0' ||
		!strcmp(little_font_other_name, little_font_default_name) )
	{
            XtSetArg( arglist[n], DECwNlittleFontSetName, 
    			&little_font_default_name[0]);    n++;
            XtSetArg( arglist[n], DECwNlittleFontSetSelection, False);    n++;
	}
	else
	{
            XtSetArg( arglist[n], DECwNlittleFontSetName, little_font_other_name); 
	    n++;
            XtSetArg( arglist[n], DECwNlittleFontSetSelection, True);    n++;
	}
    }

    if ( little_font_other_name == NULL || little_font_other_name[0] == '\0' )
    {
        XtSetArg( arglist[n], DECwNlittleFontOtherName, 
			"");
	n++;
    }
    else
    {
        XtSetArg( arglist[n], DECwNlittleFontOtherName, little_font_other_name);
	n++;
    }

    strcpy( gs_font_default_name, DEFAULT_GS_FONT_SET_NAME );
    if ( gs_font_default_button )
    {
        XtSetArg( arglist[n], DECwNgsFontSetName, &gs_font_default_name[0]);
	n++;
        XtSetArg( arglist[n], DECwNgsFontSetSelection, False);
	n++;
    }
    else
    {	
	if ( gs_font_other_name == NULL || gs_font_other_name[0] == '\0' ||
		!strcmp(gs_font_other_name, gs_font_default_name) )
	{
            XtSetArg( arglist[n], DECwNgsFontSetName, 
    			&gs_font_default_name[0]);    n++;
            XtSetArg( arglist[n], DECwNgsFontSetSelection, False);    n++;
	}
	else
	{
            XtSetArg( arglist[n], DECwNgsFontSetName, gs_font_other_name); 
	    n++;
            XtSetArg( arglist[n], DECwNgsFontSetSelection, True);    n++;
	}
    }

    if ( gs_font_other_name == NULL || gs_font_other_name[0] == '\0' )
    {
        XtSetArg( arglist[n], DECwNgsFontOtherName, 
			"");
	n++;
    }
    else
    {
        XtSetArg( arglist[n], DECwNgsFontOtherName, gs_font_other_name);
	n++;
    }

    XtSetArg( arglist[n], DECwNautoResizeTerminal,
		autoResizeTerminal_button );  n++;
    XtSetArg( arglist[n], DECwNautoResizeWindow,
		autoResizeWindow_button ); n++;
    XtSetArg( arglist[n], DECwNterminalDriverResize,
		terminalDriverResize_button );  n++;
    XtSetValues( stm->terminal, arglist, n );

    if ( big_font_other_name != NULL )
    	XtFree(big_font_other_name);
    if (little_font_other_name != NULL)
        XtFree(little_font_other_name);
    if ( gs_font_other_name != NULL )
    	XtFree(gs_font_other_name);

    n = 0;

    if ( multi_uid || IsAsianOrHebrewTermType(stm->terminalType) ) {
    _DECwTermSetTitle( stm->parent, stm->terminal, title_text_cs );
    _DECwTermSetIconName( stm->parent, stm->terminal, icon_text_cs );
    XmStringFree( title_text_cs );
    XmStringFree( icon_text_cs );
    } else {
    XtSetArg( arglist[n], XtNtitle, title_text ); n++;
    XtSetArg( arglist[n], XtNiconName, icon_text ); n++;
    XtSetValues( stm->parent, arglist, n );
    XtFree( title_text );
    XtFree( icon_text );
    }

    SetWMHints(stm, TRUE);      /* Turn on hints after widget has resized */
}

void big_font_set_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;
        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */

	stm->setup.window_big_font_set = w;
}       

void little_font_set_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;
        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id 
        if (call_data->reason == DwtCRCreate)*/

	stm->setup.window_little_font_set = w;
}

void gs_font_set_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;
        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */

	stm->setup.window_gs_font_set = w;
}       

void big_font_default_create_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;
        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */

	stm->setup.window_big_font_default = w;
}       

void big_font_other_create_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;
        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */

	stm->setup.window_big_font_other = w;
}       

void big_font_other_arm_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;
        stm = convert_widget_to_stream(w);
	XmProcessTraversal(stm->setup.window_big_font_other_name, XmTRAVERSE_CURRENT);
}       

void big_font_name_create_cb(w, closure, call_data)
Widget w;
caddr_t closure;
unsigned int *call_data;
{
	STREAM *stm;
        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */

	stm->setup.window_big_font_other_name = w;
}       

void big_font_name_focus_cb(w, closure, call_data)
Widget w;
caddr_t closure;
unsigned int *call_data;
{
	STREAM *stm;
        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */

	XmToggleButtonSetState(stm->setup.window_big_font_other, 1);
	XmToggleButtonSetState(stm->setup.window_big_font_default, 0);
}       

void little_font_default_create_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;
        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */

	stm->setup.window_little_font_default = w;
}       

void little_font_other_create_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;
        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */

	stm->setup.window_little_font_other = w;
}       

void little_font_other_arm_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;
        stm = convert_widget_to_stream(w);
	XmProcessTraversal(stm->setup.window_little_font_other_name, XmTRAVERSE_CURRENT);
}       

void little_font_name_create_cb(w, closure, call_data)
Widget w;
caddr_t closure;
unsigned int *call_data;
{
	STREAM *stm;
        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */

	stm->setup.window_little_font_other_name = w;
}       

void little_font_name_focus_cb(w, closure, call_data)
Widget w;
caddr_t closure;
unsigned int *call_data;
{
	STREAM *stm;
        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */

	XmToggleButtonSetState(stm->setup.window_little_font_other, 1);
	XmToggleButtonSetState(stm->setup.window_little_font_default, 0);
}       

void gs_font_default_create_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;
        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */

	stm->setup.window_gs_font_default = w;
}       

void gs_font_other_create_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;
        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */

	stm->setup.window_gs_font_other = w;
}       

void gs_font_other_arm_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;
        stm = convert_widget_to_stream(w);
	XmProcessTraversal(stm->setup.window_gs_font_other_name, XmTRAVERSE_CURRENT);
}       

void gs_font_name_create_cb(w, closure, call_data)
Widget w;
caddr_t closure;
unsigned int *call_data;
{
	STREAM *stm;
        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */

	stm->setup.window_gs_font_other_name = w;
}       

void gs_font_name_focus_cb(w, closure, call_data)
Widget w;
caddr_t closure;
unsigned int *call_data;
{
	STREAM *stm;
        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */

	XmToggleButtonSetState(stm->setup.window_gs_font_other, 1);
	XmToggleButtonSetState(stm->setup.window_gs_font_default, 0);
}       

void fine_font_set_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;
        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id */
            stm->setup.window_fine_font_set = w;
}

void normal_font_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;
        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id 
        if (call_data->reason == DwtCRCreate)*/

	stm->setup.window_normal_font = w;
}

void condensed_font_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;
        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id 
        if (call_data->reason == DwtCRCreate)*/

	stm->setup.window_condensed_font = w;
}

void variable_font_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;
        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id 
        if (call_data->reason == XmCRCreate)*/

	stm->setup.window_variable_font = w;
}

void rows_text_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
    STREAM *stm;

    stm = convert_widget_to_stream(w);

    /* widget was just created, log its widget id */
    stm->setup.window_rows_text = w;
}

void rows_24_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
    STREAM *stm;
    char rows_text[4];
    stm = convert_widget_to_stream(w);

    sprintf(rows_text, "%3d", 24);
    XmTextSetString(stm->setup.window_rows_text, rows_text);

}

void rows_48_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
    STREAM *stm;
    char rows_text[4];
    stm = convert_widget_to_stream(w);

    if ( IsAsianTerminalType(stm->terminalType) )
    sprintf(rows_text, "%3d", 32);
    else
    sprintf(rows_text, "%3d", 48);
    XmTextSetString(stm->setup.window_rows_text, rows_text);
}

void rows_72_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
    STREAM *stm;
    char rows_text[4];
    stm = convert_widget_to_stream(w);

    if ( IsAsianTerminalType(stm->terminalType) )
    sprintf(rows_text, "%3d", 40);
    else
    sprintf(rows_text, "%3d", 72);
    XmTextSetString(stm->setup.window_rows_text, rows_text);
}

void columns_text_cb(w, closure, call_data)
Widget w;                      
caddr_t closure;
caddr_t call_data;
{
    STREAM *stm;

    stm = convert_widget_to_stream(w);

    /* widget was just created, log its widget id */
    stm->setup.window_columns_text = w;
}


void columns_80_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
    STREAM *stm;
    char columns_text[4];

    stm = convert_widget_to_stream(w);

    sprintf(columns_text, "%3d", 80);
    XmTextSetString(stm->setup.window_columns_text, columns_text);
}

void columns_132_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
    STREAM *stm;
    char columns_text[4];

    stm = convert_widget_to_stream(w);

    if ( IsAsianTerminalType(stm->terminalType) )
    sprintf(columns_text, "%3d", 126);
    else
    sprintf(columns_text, "%3d", 132);
    XmTextSetString(stm->setup.window_columns_text, columns_text);
}

void rows_48_create_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
    STREAM *stm;
    stm = convert_widget_to_stream(w);

    stm->setup.window_rows_48_button = w;
}

void rows_72_create_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
    STREAM *stm;
    stm = convert_widget_to_stream(w);

    stm->setup.window_rows_72_button = w;
}

void columns_132_create_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
    STREAM *stm;
    stm = convert_widget_to_stream(w);

    stm->setup.window_columns_132_button = w;
}

void auto_resize_terminal_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
    STREAM *stm;
    stm = convert_widget_to_stream(w);

    /* if widget was just created, log its widget id 
    if (call_data->reason == DwtCRCreate)*/

    stm->setup.window_auto_resize_terminal = w;
}

void auto_resize_window_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
    STREAM *stm;
    stm = convert_widget_to_stream(w);

    /* if widget was just created, log its widget id 
    if (call_data->reason == DwtCRCreate)*/

    stm->setup.window_auto_resize_window = w;
}

void terminal_driver_resize_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
    STREAM *stm;
    stm = convert_widget_to_stream(w);

    /* if widget was just created, log its widget id
    if (call_data->reason == DwtCRCreate)*/

    stm->setup.window_terminal_driver_resize = w;
}

void window_title_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
    STREAM *stm;

    stm = convert_widget_to_stream(w);

    /* widget was just created, log its widget id */
    stm->setup.window_title = w;
}

void icon_name_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
    STREAM *stm;

    stm = convert_widget_to_stream(w);

    /* widget was just created, log its widget id */
    stm->setup.window_icon_name = w;
}


void upss_radio_create_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
    STREAM *stm;

    stm = convert_widget_to_stream(w);

    /* fetch upss_dec_tb and upss_iso_tb unconditionally*/
    if (MrmFetchWidget
	(
	s_MRMHierarchy,
	"upss_dec_tb",
	w,	/* the radio box is the parent */
	& stm->setup.general_upss_dec,
	& dummy_class
	)
    != MrmSUCCESS)
    {
	log_message( "Unable to fetch widget definitions from DRM\n");
	return;
    }

    if (MrmFetchWidget
	(
	s_MRMHierarchy,
	"upss_iso_tb",
	w,	/* the radio box is the parent */
	& stm->setup.general_upss_iso,
	& dummy_class
	)
    != MrmSUCCESS)
    {
	log_message( "Unable to fetch widget definitions from DRM\n");
	return;
    }

    XtManageChild(stm->setup.general_upss_dec);
    XtManageChild(stm->setup.general_upss_iso);

    /* fetch Turkish's only if it is Turkish DECterm */
    if (stm->terminalType == DECwTurkish)
	{
	/* fetch upss_dec_turkish_tb */
        if (MrmFetchWidget
               (
               s_MRMHierarchy,
               "upss_dec_turkish_tb",
               w,	/* the radio box is the parent */
               & stm->setup.general_upss_dec_turkish,
               & dummy_class       
               )
           != MrmSUCCESS)
            {
            log_message( "Unable to fetch widget definitions from DRM\n");
	    return;
            }

	/* fetch upss_iso_turkish_tb */
        if (MrmFetchWidget
               (
               s_MRMHierarchy,
               "upss_iso_turkish_tb",
               w,	/* the radio box is the parent */
               & stm->setup.general_upss_iso_turkish,
               & dummy_class       
               )
           != MrmSUCCESS)
            {
            log_message( "Unable to fetch widget definitions from DRM\n");
	    return;
            }

	XtManageChild(stm->setup.general_upss_dec_turkish);
	XtManageChild(stm->setup.general_upss_iso_turkish);
	}
    else if (stm->terminalType == DECwGreek)
	{
	/* fetch upss_dec_greek_tb only if it is Greek DECterm*/
        if (MrmFetchWidget
               (
               s_MRMHierarchy,
               "upss_dec_greek_tb",
               w,	/* the radio box is the parent */
               & stm->setup.general_upss_dec_greek,
               & dummy_class       
               )
           != MrmSUCCESS)
            {
            log_message( "Unable to fetch widget definitions from DRM\n");
	    return;
            }

/* DEC Greek is not supported, according to Kimon.  I still leave the stub
 * here anyway because the escape sequence can still change it.  It's just
 * that stm->setup.general_upss_dec_greek is not managed.
 */
#if 0
	XtManageChild(stm->setup.general_upss_dec_greek);
#endif

	/* fetch upss_iso_greek_tb */
        if (MrmFetchWidget
               (
               s_MRMHierarchy,
               "upss_iso_greek_tb",
               w,	/* the radio box is the parent */
               & stm->setup.general_upss_iso_greek,
               & dummy_class       
               )
           != MrmSUCCESS)
            {
            log_message( "Unable to fetch widget definitions from DRM\n");
	    return;
            }

	XtManageChild(stm->setup.general_upss_iso_greek);
	}
}


void file_open_create_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
    Widget ok_wid, filter_wid, cancel_wid, help_wid;

    ok_wid	= XmFileSelectionBoxGetChild(w, XmDIALOG_OK_BUTTON);
    filter_wid	= XmFileSelectionBoxGetChild(w, XmDIALOG_APPLY_BUTTON);
    cancel_wid	= XmFileSelectionBoxGetChild(w, XmDIALOG_CANCEL_BUTTON);
    help_wid	= XmFileSelectionBoxGetChild(w, XmDIALOG_HELP_BUTTON);

    XtRemoveAllCallbacks(ok_wid,     XmNhelpCallback);
    XtRemoveAllCallbacks(filter_wid, XmNhelpCallback);
    XtRemoveAllCallbacks(cancel_wid, XmNhelpCallback);
    XtRemoveAllCallbacks(help_wid,   XmNhelpCallback);

#ifdef HYPERHELP
    XtAddCallback(ok_wid,     XmNhelpCallback, help_system_proc,
						"settings_from_ok");
    XtAddCallback(filter_wid, XmNhelpCallback, help_system_proc,
						"settings_from_filter");
    XtAddCallback(cancel_wid, XmNhelpCallback, help_system_proc,
						"settings_from_cancel");
    XtAddCallback(help_wid,   XmNhelpCallback, help_system_proc,
						"settings_from_help");
#endif
}

void file_saveas_create_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
    Widget ok_wid, filter_wid, cancel_wid, help_wid;

    ok_wid	= XmFileSelectionBoxGetChild(w, XmDIALOG_OK_BUTTON);
    filter_wid	= XmFileSelectionBoxGetChild(w, XmDIALOG_APPLY_BUTTON);
    cancel_wid	= XmFileSelectionBoxGetChild(w, XmDIALOG_CANCEL_BUTTON);
    help_wid	= XmFileSelectionBoxGetChild(w, XmDIALOG_HELP_BUTTON);

    XtRemoveAllCallbacks(ok_wid,     XmNhelpCallback);
    XtRemoveAllCallbacks(filter_wid, XmNhelpCallback);
    XtRemoveAllCallbacks(cancel_wid, XmNhelpCallback);
    XtRemoveAllCallbacks(help_wid,   XmNhelpCallback);

#ifdef HYPERHELP
    XtAddCallback(ok_wid,     XmNhelpCallback, help_system_proc,
							"save_as_ok");
    XtAddCallback(filter_wid, XmNhelpCallback, help_system_proc,
							"save_as_filter");
    XtAddCallback(cancel_wid, XmNhelpCallback, help_system_proc,
							"save_as_cancel");
    XtAddCallback(help_wid,   XmNhelpCallback, help_system_proc,
							"save_as_help");
#endif
}



/*****************************
 *  display setup callbacks  *
 *****************************/

void setup_display_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
	STREAM *stm;
	int n, batchScrollCount, transcriptSize;
	Boolean screenMode, autoWrapEnable, cursorBlinkEnable,
	  displayControls, textCursorEnable, statusDisplayEnable,
	  scrollHorizontal, scrollVertical, couplingHorizontal,
	  controlRepresentationMode, leadingCodeEnable,
	  couplingVertical, saveLinesOffTop;
  	DECwCursorStyle cursorStyle;
	Arg arglist[20];
	char batch_scroll_text[4], transcript_size_text[5];

        stm = convert_widget_to_stream(w);


        /* get and save current settings from terminal widget */
	n = 0;
	XtSetArg( arglist[n], DECwNsaveLinesOffTop, &saveLinesOffTop ); n++;
	XtSetArg( arglist[n], DECwNscrollHorizontal, &scrollHorizontal ); n++;
	XtSetArg( arglist[n], DECwNscrollVertical, &scrollVertical ); n++;
  	XtSetArg( arglist[n], DECwNcouplingHorizontal, &couplingHorizontal);
	  n++;
	XtSetArg( arglist[n], DECwNcouplingVertical, &couplingVertical); n++;
	XtSetArg( arglist[n], DECwNscreenMode, &screenMode); n++;
	XtSetArg( arglist[n], DECwNautoWrapEnable, &autoWrapEnable); n++;
	XtSetArg( arglist[n], DECwNcursorBlinkEnable, &cursorBlinkEnable); n++;
	XtSetArg( arglist[n], DECwNcursorStyle, &cursorStyle); n++;
	XtSetArg( arglist[n], DECwNtextCursorEnable, &textCursorEnable); n++;
  	XtSetArg( arglist[n], DECwNstatusDisplayEnable, &statusDisplayEnable);
	  n++;
	XtSetArg( arglist[n], DECwNbatchScrollCount, &batchScrollCount ); n++;
	XtSetArg( arglist[n], DECwNtranscriptSize, &transcriptSize ); n++;

	if ( multi_uid || stm->terminalType == DECwHanyu ) {
	XtSetArg( arglist[n], DECwNleadingCodeEnable, &leadingCodeEnable ); n++;
	}
	if ( multi_uid || IsAsianTerminalType(stm->terminalType) ) {
	XtSetArg( arglist[n], DECwNcontrolRepresentationMode,
		&controlRepresentationMode); n++;
	}

  	XtGetValues( stm->terminal, arglist, n);


        /* check if we need to fetch the widgets from disk */
     	if (DISPLAY_PARENT == 0)
            {

            /* fetch dialog box from DRM */
            if (MrmFetchWidget
                   (
                   s_MRMHierarchy,
                   "setup_display_db",
                   stm->parent,
                   & DISPLAY_PARENT,
                   & dummy_class
                   )
               != MrmSUCCESS)
                {
                log_error(
			"Unable to fetch widget definitions from DRM\n");
	        return;
                }
            }

	/* initialize widgets to current state */
    	XmToggleButtonSetState(stm->setup.display_save_lines_off_top,
		saveLinesOffTop);
    	XmToggleButtonSetState(stm->setup.display_scroll_horizontal,
		scrollHorizontal);
	XmToggleButtonSetState(stm->setup.display_scroll_vertical,
		scrollVertical);
    	XmToggleButtonSetState(stm->setup.display_coupling_horizontal,
		couplingHorizontal);
	XmToggleButtonSetState(stm->setup.display_coupling_vertical,
		couplingVertical);
    	XmToggleButtonSetState(stm->setup.display_screen_mode_set,
		screenMode);
	XmToggleButtonSetState(stm->setup.display_screen_mode_reset,
		! screenMode);
	XmToggleButtonSetState(stm->setup.display_auto_wrap_enable,
		autoWrapEnable);
	XmToggleButtonSetState(stm->setup.display_cursor_blink_enable,
		cursorBlinkEnable);
  	XmToggleButtonSetState(stm->setup.display_block_cursor,
		(cursorStyle == DECwCursorBlock) );
	XmToggleButtonSetState(stm->setup.display_underline_cursor,
		(cursorStyle == DECwCursorUnderline) );
	XmToggleButtonSetState(stm->setup.display_text_cursor_enable,
		textCursorEnable );
	XmToggleButtonSetState(stm->setup.display_no_status_display,
		! statusDisplayEnable );
	if ( multi_uid || stm->terminalType == DECwKanji )
	XmToggleButtonSetState(stm->setup.display_display_controls,
		controlRepresentationMode );
	if ( multi_uid || stm->terminalType == DECwHanyu )
	XmToggleButtonSetState(stm->setup.display_display_leading_code,
		leadingCodeEnable );
	XmToggleButtonSetState(stm->setup.display_status_display,
		statusDisplayEnable );
	sprintf(batch_scroll_text, "%3d", batchScrollCount);
	XmTextSetString(stm->setup.display_batch_scroll,
		batch_scroll_text );
	sprintf(transcript_size_text, "%4d", transcriptSize );
	XmTextSetString(stm->setup.display_transcript_size,
		transcript_size_text );	

	if ( multi_uid ) {
	    if ( IsAsianTerminalType(stm->terminalType) )
		XtSetSensitive( stm->setup.display_display_controls, True );
	    else
		XtSetSensitive( stm->setup.display_display_controls, False );
	    if ( stm->terminalType == DECwHanyu )
		XtSetSensitive( stm->setup.display_display_leading_code, True );
	    else
		XtSetSensitive( stm->setup.display_display_leading_code, False );
	}

        /* put up display dialog box and return */
	XtUnmanageChild(DISPLAY_PARENT);
	XtManageChild(DISPLAY_PARENT);
	return;
}
            
void display_ok_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
	STREAM *stm;
        stm = convert_widget_to_stream(w);

	display_set_resources( stm );
#ifdef VXT_DECTERM
	if ( VxtSwPhysicalMemory() || !(VxtSwPagefileEnabled()) )
	{
	    XtDestroyWidget(DISPLAY_PARENT);
	    DISPLAY_PARENT = NULL;
	}
	else 
#endif VXT_DECTERM
	XtUnmanageChild(DISPLAY_PARENT);
}

void display_apply_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
	STREAM *stm;
        stm = convert_widget_to_stream(w);

	display_set_resources( stm );
}

void display_cancel_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{                              
	STREAM *stm;
        stm = convert_widget_to_stream(w);

#ifdef VXT_DECTERM
	if ( VxtSwPhysicalMemory() || !(VxtSwPagefileEnabled()) )
	{
	    XtDestroyWidget(DISPLAY_PARENT);
	    DISPLAY_PARENT = NULL;
	}
	else 
#endif VXT_DECTERM
	XtUnmanageChild(DISPLAY_PARENT);
}

display_set_resources( stm )
    STREAM *stm;
{
    Arg arglist[20];
    int n;
    int screenMode_button, autoWrapEnable_button, cursorBlinkEnable_button,
      block_cursor_button, displayControls_button, textCursorEnable_button,
      statusDisplayEnable_button, scrollHorizontal_button,
      scrollVertical_button, couplingHorizontal_button,
      controlRepresentationMode_button, leadingCodeEnable_button,
      couplingVertical_button, saveLinesOffTop_button,
      batch_scroll_count, transcript_size;
    Boolean screenMode;
    char *batch_scroll_text, *transcript_size_text;

    SetWMHints(stm, FALSE);     /* Turn off hints so widget can resize freely */

    /* read button states */
    scrollHorizontal_button =
      XmToggleButtonGetState(stm->setup.display_scroll_horizontal);
    scrollVertical_button =
      XmToggleButtonGetState(stm->setup.display_scroll_vertical);
    saveLinesOffTop_button =
      XmToggleButtonGetState(stm->setup.display_save_lines_off_top);
    couplingHorizontal_button =
      XmToggleButtonGetState(stm->setup.display_coupling_horizontal);
    couplingVertical_button =
      XmToggleButtonGetState(stm->setup.display_coupling_vertical);
    screenMode_button =
      XmToggleButtonGetState(stm->setup.display_screen_mode_set);
    autoWrapEnable_button =
      XmToggleButtonGetState(stm->setup.display_auto_wrap_enable);
    cursorBlinkEnable_button =
      XmToggleButtonGetState(stm->setup.display_cursor_blink_enable);
    block_cursor_button =
      XmToggleButtonGetState(stm->setup.display_block_cursor);
    textCursorEnable_button =
      XmToggleButtonGetState(stm->setup.display_text_cursor_enable);
    statusDisplayEnable_button =
      XmToggleButtonGetState(stm->setup.display_status_display);
    batch_scroll_text =
      XmTextGetString(stm->setup.display_batch_scroll);
    transcript_size_text =
      XmTextGetString(stm->setup.display_transcript_size);
    if ( multi_uid || stm->terminalType == DECwKanji )	
    controlRepresentationMode_button =
      XmToggleButtonGetState(stm->setup.display_display_controls);
    if ( multi_uid || stm->terminalType == DECwHanyu )	
    leadingCodeEnable_button =
      XmToggleButtonGetState(stm->setup.display_display_leading_code);

    batch_scroll_count = atoi( batch_scroll_text );
    if ( batch_scroll_count < 0 )
	batch_scroll_count = 0;
    transcript_size = atoi( transcript_size_text );
    if ( transcript_size < 0 )
	transcript_size = 0;

    /* If screenMode has changed, set reverseVideo to TRUE */
    XtSetArg( arglist[0], DECwNscreenMode, &screenMode );
    XtGetValues( stm->terminal, arglist, 1 );

    /* set the resources (even if not changed ) */
    n = 0;
    if (screenMode != screenMode_button) {
        XtSetArg( arglist[n], DECwNreverseVideo, TRUE ); n++;
    }
    XtSetArg( arglist[n], DECwNscreenMode, screenMode_button ); n++;
    XtSetArg( arglist[n], DECwNautoWrapEnable, autoWrapEnable_button ); n++;
    XtSetArg( arglist[n], DECwNcursorBlinkEnable, cursorBlinkEnable_button );
      n++;
    XtSetArg( arglist[n], DECwNcursorStyle, block_cursor_button ?
      DECwCursorBlock : DECwCursorUnderline ); n++;
    XtSetArg( arglist[n], DECwNtextCursorEnable, textCursorEnable_button ); n++;
    XtSetArg( arglist[n], DECwNstatusDisplayEnable,
	statusDisplayEnable_button );  n++;
    XtSetArg( arglist[n], DECwNsaveLinesOffTop, saveLinesOffTop_button ); n++;
    XtSetArg( arglist[n], DECwNscrollHorizontal, scrollHorizontal_button ); n++;
    XtSetArg( arglist[n], DECwNscrollVertical, scrollVertical_button ); n++;
    XtSetArg( arglist[n], DECwNcouplingHorizontal,
	couplingHorizontal_button ); n++;
    XtSetArg( arglist[n], DECwNcouplingVertical, couplingVertical_button ); n++;
    XtSetArg( arglist[n], DECwNbatchScrollCount, batch_scroll_count ); n++;
    XtSetArg( arglist[n], DECwNtranscriptSize, transcript_size ); n++;
    if ( multi_uid || stm->terminalType == DECwKanji ) {
    XtSetArg( arglist[n], DECwNcontrolRepresentationMode,
	controlRepresentationMode_button ); n++;
    }
    if ( multi_uid || stm->terminalType == DECwHanyu ) {
    XtSetArg( arglist[n], DECwNleadingCodeEnable, leadingCodeEnable_button );
	n++;
    }

    XtSetValues( stm->terminal, arglist, n );
    XtFree(batch_scroll_text);
    XtFree(transcript_size_text);

    SetWMHints(stm, TRUE);      /* Turn on hints after widget has resized */
}

void save_lines_off_top_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{     
    STREAM *stm;
    stm = convert_widget_to_stream(w);

    /* if widget was just created, log its widget id
    if (call_data->reason == DwtCRCreate)*/

    stm->setup.display_save_lines_off_top = w;
}


void scroll_horizontal_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
    	STREAM *stm;
        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id 
        if (call_data->reason == DwtCRCreate)*/

	stm->setup.display_scroll_horizontal = w;
}
  
void scroll_vertical_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;
        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id 
        if (call_data->reason == DwtCRCreate)*/

	stm->setup.display_scroll_vertical = w;
}
    
void coupling_horizontal_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;
        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id 
        if (call_data->reason == DwtCRCreate)*/

	stm->setup.display_coupling_horizontal = w;
}

void coupling_vertical_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;
        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id 
        if (call_data->reason == DwtCRCreate)*/

	stm->setup.display_coupling_vertical = w;
}


void auto_wrap_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
    STREAM *stm;
    stm = convert_widget_to_stream(w);

    /* if widget was just created, log its widget id 
    if (call_data->reason == DwtCRCreate)*/

    stm->setup.display_auto_wrap_enable = w;
}

void display_cursor_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
    STREAM *stm;
    stm = convert_widget_to_stream(w);

    /* if widget was just created, log its widget id 
    if (call_data->reason == DwtCRCreate)*/

    stm->setup.display_text_cursor_enable = w;
}

void light_text_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
    STREAM *stm;
    stm = convert_widget_to_stream(w);

    /* if widget was just created, log its widget id 
    if (call_data->reason == DwtCRCreate)*/

    stm->setup.display_screen_mode_reset = w;
}
    
void dark_text_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
    STREAM *stm;
    stm = convert_widget_to_stream(w);

    /* if widget was just created, log its widget id 
    if (call_data->reason == DwtCRCreate)*/

    stm->setup.display_screen_mode_set = w;
}

void cursor_blink_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
    STREAM *stm;
    stm = convert_widget_to_stream(w);

    /* if widget was just created, log its widget id 
    if (call_data->reason == DwtCRCreate) */

    stm->setup.display_cursor_blink_enable = w;
}

void block_cursor_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
    STREAM *stm;
    stm = convert_widget_to_stream(w);

    /* if widget was just created, log its widget id 
    if (call_data->reason == DwtCRCreate) */

    stm->setup.display_block_cursor = w;
}

void underline_cursor_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
    STREAM *stm;
    stm = convert_widget_to_stream(w);

    /* if widget was just created, log its widget id 
    if (call_data->reason == DwtCRCreate) */

    stm->setup.display_underline_cursor = w;
}

void interpret_controls_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
	this routine is not referenced in any DECterm code
*/
{
    STREAM *stm;
    stm = convert_widget_to_stream(w);

    /* if widget was just created, log its widget id
    if (call_data->reason == DwtCRCreate)*/

    stm->setup.display_interpret_controls = w;
}                       

void display_controls_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
	This routine is not referenced in any decterm code
*/
{
    STREAM *stm;
    stm = convert_widget_to_stream(w);

    /* if widget was just created, log its widget id
    if (call_data->reason == DwtCRCreate)*/

    stm->setup.display_display_controls = w;
}

void no_status_display_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
    STREAM *stm;
    stm = convert_widget_to_stream(w);

    /* if widget was just created, log its widget id
    if (call_data->reason == DwtCRCreate)*/

    stm->setup.display_no_status_display = w;
}
                                                    
void host_status_display_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
    STREAM *stm;
    stm = convert_widget_to_stream(w);

    /* if widget was just created, log its widget id
    if (call_data->reason == DwtCRCreate)*/

    stm->setup.display_status_display = w;
}

void batch_scroll_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
    STREAM *stm;

    stm = convert_widget_to_stream(w);

    /* widget was just created, log its widget id */
    stm->setup.display_batch_scroll = w;
}

void transcript_size_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
    STREAM *stm;

    stm = convert_widget_to_stream(w);

    /* widget was just created, log its widget id */
    stm->setup.display_transcript_size = w;
}

void display_leading_code_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
    STREAM *stm;
    stm = convert_widget_to_stream(w);
    /* if widget was just created, log its widget id
    if (call_data->reason == DwtCRCreate) */
	stm->setup.display_display_leading_code = w;
}

void suppress_leading_code_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
#if 0
    STREAM *stm;
    stm = convert_widget_to_stream(w);
    /* if widget was just created, log its widget id
    if (call_data->reason == DwtCRCreate) */
	stm->setup.display_suppress_leading_code = w;
#endif
}

/*****************************
 *  general setup callbacks  *
 *****************************/
                               
void setup_general_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
	STREAM *stm;
	int n;
	Arg arglist[20];
	Boolean newLineMode, lockUDK, lockUserFeatures,
	  applicationCursorKeyMode, applicationKeypadMode, eightBitCharacters;
	Boolean local_echo_mode;
	Boolean conceal_answerback_mode;
	char *answerback_message, *conceal_message;
    	caddr_t value_id;
    	MrmCode value_type;
	int status;
	DECwResponseDA responseDA;
	DECwUserPreferenceSet userPreferenceSet;
	DECwTerminalMode terminalMode;
	DECwJisRomanAsciiMode jisRomanAsciiMode;
	DECwKanjiKatakanaMode kanjiKatakanaMode;
	DECwKanjiKatakanaMode kanji_78_83;
	DECwKsRomanAsciiMode ksRomanAsciiMode;
	Boolean rightToLeft;
        stm = convert_widget_to_stream(w);


        /* get and save current setup state from widget */
	n = 0;
	XtSetArg( arglist[n], DECwNnewLineMode, &newLineMode); n++;
	XtSetArg( arglist[n], DECwNlockUDK, &lockUDK); n++;
	XtSetArg( arglist[n], DECwNlockUserFeatures, &lockUserFeatures); n++;
	XtSetArg( arglist[n], DECwNlocalEcho, &local_echo_mode); n++;
	XtSetArg( arglist[n], DECwNconcealAnswerback, 
			&conceal_answerback_mode); n++;
	XtSetArg( arglist[n], DECwNanswerbackMessage, 
			&answerback_message); n++;
	XtSetArg( arglist[n], DECwNapplicationCursorKeyMode,
		   &applicationCursorKeyMode); n++;
	XtSetArg( arglist[n], DECwNresponseDA, &responseDA); n++;
	XtSetArg( arglist[n], DECwNapplicationKeypadMode,
		   &applicationKeypadMode); n++;
	XtSetArg( arglist[n], DECwNuserPreferenceSet, &userPreferenceSet); n++;
	if ( multi_uid || stm->terminalType == DECwKanji ) {
	XtSetArg( arglist[n], DECwNjisRomanAsciiMode, &jisRomanAsciiMode); n++;
	XtSetArg( arglist[n], DECwNkanjiKatakanaMode, &kanjiKatakanaMode); n++;
	XtSetArg( arglist[n], DECwNkanji_78_83, &kanji_78_83); n++;
	}
	if ( multi_uid || stm->terminalType == DECwHangul ) {
	XtSetArg( arglist[n], DECwNksRomanAsciiMode, &ksRomanAsciiMode); n++;
	}
	if ( multi_uid || !IsAsianTerminalType(stm->terminalType) )
	{
	XtSetArg( arglist[n], DECwNeightBitCharacters,
		   &eightBitCharacters); n++;
	}
	if ( multi_uid || stm->terminalType == DECwHebrew ) {
	XtSetArg( arglist[n], DECwNrightToLeft, &rightToLeft ); n++;
	}
	XtSetArg( arglist[n], DECwNterminalMode, &terminalMode); n++;
	XtGetValues( stm->terminal, arglist, n);


        /* check if we need to fetch the widgets from disk */
	if (GENERAL_PARENT == 0)
            {

            /* fetch dialog box from DRM */
            if (MrmFetchWidget
                   (
                   s_MRMHierarchy,
                   "setup_general_db",
                   stm->parent, 
                   & GENERAL_PARENT,
                   & dummy_class
                   )
               != MrmSUCCESS)
                {
                log_error(
			"Unable to fetch widget definitions from DRM\n");
	        return;
                }
            }

	/* initialize widgets to current state */
  	XmToggleButtonSetState(stm->setup.general_newline,
 		newLineMode);
 	XmToggleButtonSetState(stm->setup.general_udk_locked,
 		lockUDK);
 	XmToggleButtonSetState(stm->setup.general_features_locked,
 		lockUserFeatures);
 	XmToggleButtonSetState(stm->setup.general_local_echo,
 		local_echo_mode);
 	XmToggleButtonSetState(stm->setup.general_conceal_answerback,
 				conceal_answerback_mode);
	if ( conceal_answerback_mode )
	{
	    /*
	     * Read the word "<concealed>" from the UID file.
 	     */
	    if ((status = MrmFetchLiteral(
		    s_MRMHierarchy,
		    "concealed_message_string",
		    stm->display,
		    &value_id,
		    &value_type)) != MrmSUCCESS)
	    {
		log_error(
		    "Unable to fetch application title from MRM\n" );
		if ( status == MrmNOT_FOUND )
		    log_error( "concealed_message_string literal not found\n");
	    }
	{
	long len, status;
	conceal_message = (char *) DXmCvtCStoFC (value_id, &len, &status);
    	XmTextSetString(stm->setup.general_answerback_message, conceal_message);
	XtFree (conceal_message);
	}
	if (stm->concealed_ans)
	    XtFree (stm->concealed_ans);
	stm->concealed_ans = XtNewString (answerback_message);
	}
	else
	{
	    XmTextSetString(stm->setup.general_answerback_message,
			answerback_message);
	}
 	XmToggleButtonSetState(stm->setup.general_normal_cursor_keys,
 		! applicationCursorKeyMode);
 	XmToggleButtonSetState(stm->setup.general_appl_cursor_keys,
 		applicationCursorKeyMode);
 	XmToggleButtonSetState(stm->setup.general_decterm_id,
 		responseDA == DECwDECtermID);
	if ( multi_uid || stm->terminalType == DECwKanji ) {
 	XmToggleButtonSetState(stm->setup.general_vt382_id,
 		responseDA == DECwVT382_ID);
  	XmToggleButtonSetState(stm->setup.general_vt320_id,
 		responseDA == DECwVT320_ID);
 	XmToggleButtonSetState(stm->setup.general_vt286_id,
 		responseDA == DECwVT286_ID);
 	XmToggleButtonSetState(stm->setup.general_vt284_id,
 		responseDA == DECwVT284_ID);
 	XmToggleButtonSetState(stm->setup.general_vt282_id,
 		responseDA == DECwVT282_ID);
 	XmToggleButtonSetState(stm->setup.general_vt220j_id,
 		responseDA == DECwVT220J_ID);
 	XmToggleButtonSetState(stm->setup.general_vt102j_id,
 		responseDA == DECwVT102J_ID);
 	XmToggleButtonSetState(stm->setup.general_vt102_id,
 		responseDA == DECwVT102_ID);
 	XmToggleButtonSetState(stm->setup.general_vt101_id,
 		responseDA == DECwVT101_ID);
 	XmToggleButtonSetState(stm->setup.general_vt100j_id,
 		responseDA == DECwVT100J_ID);
 	XmToggleButtonSetState(stm->setup.general_vt100_id,
 		responseDA == DECwVT100_ID);
 	XmToggleButtonSetState(stm->setup.general_vt80_id,
 		responseDA == DECwVT80_ID);
	}
	if ( multi_uid || stm->terminalType == DECwHanzi ) {
 	XmToggleButtonSetState(stm->setup.general_vt382cb_id,
 		responseDA == DECwVT382CB_ID);
  	XmToggleButtonSetState(stm->setup.general_vt320_id,
 		responseDA == DECwVT320_ID);
 	XmToggleButtonSetState(stm->setup.general_vt220_id,
 		responseDA == DECwVT220_ID);
 	XmToggleButtonSetState(stm->setup.general_vt102_id,
 		responseDA == DECwVT102_ID);
 	XmToggleButtonSetState(stm->setup.general_vt101_id,
 		responseDA == DECwVT101_ID);
 	XmToggleButtonSetState(stm->setup.general_vt100_id,
 		responseDA == DECwVT100_ID);
	}
	if ( multi_uid || stm->terminalType == DECwHangul ) {
 	XmToggleButtonSetState(stm->setup.general_vt382k_id,
 		responseDA == DECwVT382K_ID);
  	XmToggleButtonSetState(stm->setup.general_vt320_id,
 		responseDA == DECwVT320_ID);
 	XmToggleButtonSetState(stm->setup.general_vt220_id,
 		responseDA == DECwVT220_ID);
 	XmToggleButtonSetState(stm->setup.general_vt102_id,
 		responseDA == DECwVT102_ID);
 	XmToggleButtonSetState(stm->setup.general_vt101_id,
 		responseDA == DECwVT101_ID);
 	XmToggleButtonSetState(stm->setup.general_vt100_id,
 		responseDA == DECwVT100_ID);
	}
	if ( multi_uid || stm->terminalType == DECwHanyu ) {
 	XmToggleButtonSetState(stm->setup.general_vt382d_id,
 		responseDA == DECwVT382D_ID);
  	XmToggleButtonSetState(stm->setup.general_vt320_id,
 		responseDA == DECwVT320_ID);
 	XmToggleButtonSetState(stm->setup.general_vt220_id,
 		responseDA == DECwVT220_ID);
 	XmToggleButtonSetState(stm->setup.general_vt102_id,
 		responseDA == DECwVT102_ID);
 	XmToggleButtonSetState(stm->setup.general_vt101_id,
 		responseDA == DECwVT101_ID);
 	XmToggleButtonSetState(stm->setup.general_vt100_id,
 		responseDA == DECwVT100_ID);
	}

	if ( multi_uid || !IsAsianTerminalType(stm->terminalType) ) {
 	XmToggleButtonSetState(stm->setup.general_vt340_id,
 		responseDA == DECwVT340_ID);
 	XmToggleButtonSetState(stm->setup.general_vt330_id,
 		responseDA == DECwVT330_ID);
  	XmToggleButtonSetState(stm->setup.general_vt320_id,
 		responseDA == DECwVT320_ID);
 	XmToggleButtonSetState(stm->setup.general_vt240_id,
 		responseDA == DECwVT240_ID);
 	XmToggleButtonSetState(stm->setup.general_vt220_id,
 		responseDA == DECwVT220_ID);
 	XmToggleButtonSetState(stm->setup.general_vt125_id,
 		responseDA == DECwVT125_ID);
 	XmToggleButtonSetState(stm->setup.general_vt102_id,
 		responseDA == DECwVT102_ID);
 	XmToggleButtonSetState(stm->setup.general_vt101_id,
 		responseDA == DECwVT101_ID);
 	XmToggleButtonSetState(stm->setup.general_vt100_id,
 		responseDA == DECwVT100_ID);
 	XmToggleButtonSetState(stm->setup.general_vt100_id,
 		responseDA == DECwVT100_ID);
	}
 	XmToggleButtonSetState(stm->setup.general_numeric_keypad,
 		! applicationKeypadMode);
 	XmToggleButtonSetState(stm->setup.general_appl_keypad,
 		applicationKeypadMode);
 	XmToggleButtonSetState(stm->setup.general_upss_dec,
 		userPreferenceSet == DECwDEC_Supplemental);
 	XmToggleButtonSetState(stm->setup.general_upss_iso,
 		userPreferenceSet == DECwISO_Latin1_Supplemental);

	if (stm->terminalType == DECwTurkish) {
	    if (stm->setup.general_upss_dec_turkish)  /* only if uid exists */
		XmToggleButtonSetState(stm->setup.general_upss_dec_turkish,
			userPreferenceSet == DECwDEC_Turkish_Supplemental);
	    if (stm->setup.general_upss_iso_turkish)  /* only if uid exists */
		XmToggleButtonSetState(stm->setup.general_upss_iso_turkish,
			userPreferenceSet == DECwISO_Latin5_Supplemental);
	}

	if (stm->terminalType == DECwGreek) {
	    if (stm->setup.general_upss_dec_greek)  /* only if uid exists */
		XmToggleButtonSetState(stm->setup.general_upss_dec_greek,
			userPreferenceSet == DECwDEC_Greek_Supplemental);
	    if (stm->setup.general_upss_iso_greek)  /* only if uid exists */
		XmToggleButtonSetState(stm->setup.general_upss_iso_greek,
			userPreferenceSet == DECwISO_Latin7_Supplemental);
	}

	if ( multi_uid || stm->terminalType == DECwHebrew ) {
	XmToggleButtonSetState(stm->setup.general_upss_dec_heb,
		userPreferenceSet == DECwDEC_Hebrew_Supplemental);
	XmToggleButtonSetState(stm->setup.general_upss_iso_heb,
		userPreferenceSet == DECwISO_Latin8_Supplemental);
	XmToggleButtonSetState(stm->setup.general_ltor, !rightToLeft);
	XmToggleButtonSetState(stm->setup.general_rtol, rightToLeft);
	}
	if ( multi_uid || stm->terminalType == DECwKanji ) {
 	XmToggleButtonSetState(stm->setup.general_jisroman_mode,
 		jisRomanAsciiMode == DECwJisRomanMode);
 	XmToggleButtonSetState(stm->setup.general_ascii_mode,
 		jisRomanAsciiMode == DECwAsciiMode);
 	XmToggleButtonSetState(stm->setup.general_kanji_mode,
 		kanjiKatakanaMode == DECwKanjiMode);
     	XmToggleButtonSetState(stm->setup.general_katakana_mode,
 		kanjiKatakanaMode == DECwKatakanaMode);
     	XmToggleButtonSetState(stm->setup.general_kanji_78,
 		kanji_78_83 == DECwKanji_78);
     	XmToggleButtonSetState(stm->setup.general_kanji_83,
 		kanji_78_83 == DECwKanji_83);
	}
	if ( multi_uid || stm->terminalType == DECwHangul ) {
 	XmToggleButtonSetState(stm->setup.general_ksroman_mode,
 		ksRomanAsciiMode == DECwKsRomanMode);
 	XmToggleButtonSetState(stm->setup.general_ksascii_mode,
 		ksRomanAsciiMode == DECwKsAsciiMode);
	}
	if ( multi_uid || !IsAsianTerminalType(stm->terminalType) )
	{
 	XmToggleButtonSetState(stm->setup.general_eight_bit,
 		eightBitCharacters );
 	XmToggleButtonSetState(stm->setup.general_seven_bit,
 		! eightBitCharacters );
	}
 	XmToggleButtonSetState(stm->setup.general_vt300_8bitc_mode,
 		terminalMode == DECwVT300_8_BitMode);
 	XmToggleButtonSetState(stm->setup.general_vt300_7bitc_mode,
 		terminalMode == DECwVT300_7_BitMode);
 	XmToggleButtonSetState(stm->setup.general_vt100_mode,
 		terminalMode == DECwVT100_Mode);
 	XmToggleButtonSetState(stm->setup.general_vt52_mode,
 		terminalMode == DECwVT52_Mode);

	if ( multi_uid ) {
	    if ( !IsAsianTerminalType(stm->terminalType) ) {
		XtSetSensitive( stm->setup.general_vt382_id, False );
		XtSetSensitive( stm->setup.general_vt340_id, True );
		XtSetSensitive( stm->setup.general_vt330_id, True );
		XtSetSensitive( stm->setup.general_vt240_id, True );
		XtSetSensitive( stm->setup.general_vt220_id, True );
		XtSetSensitive( stm->setup.general_vt125_id, True );
	    } else {
		XtSetSensitive( stm->setup.general_vt382_id, True );
		XtSetSensitive( stm->setup.general_vt340_id, False );
		XtSetSensitive( stm->setup.general_vt330_id, False );
		XtSetSensitive( stm->setup.general_vt240_id, False );
		XtSetSensitive( stm->setup.general_vt220_id, False );
		XtSetSensitive( stm->setup.general_vt125_id, False );
	    }
	    if ( stm->terminalType == DECwKanji ) {
		XtSetSensitive( stm->setup.general_vt286_id, True );
		XtSetSensitive( stm->setup.general_vt284_id, True );
		XtSetSensitive( stm->setup.general_vt282_id, True );
		XtSetSensitive( stm->setup.general_vt220j_id, True );
		XtSetSensitive( stm->setup.general_vt102j_id, True );
		XtSetSensitive( stm->setup.general_vt100j_id, True );
		XtSetSensitive( stm->setup.general_vt80_id, True );
		XtSetSensitive( stm->setup.general_jisroman_mode, True );
		XtSetSensitive( stm->setup.general_ascii_mode, True );
		XtSetSensitive( stm->setup.general_kanji_mode, True );
		XtSetSensitive( stm->setup.general_katakana_mode, True );
		XtSetSensitive( stm->setup.general_kanji_78, True );
		XtSetSensitive( stm->setup.general_kanji_83, True );
	    } else {
		XtSetSensitive( stm->setup.general_vt286_id, False );
		XtSetSensitive( stm->setup.general_vt284_id, False );
		XtSetSensitive( stm->setup.general_vt282_id, False );
		XtSetSensitive( stm->setup.general_vt220j_id, False );
		XtSetSensitive( stm->setup.general_vt102j_id, False );
		XtSetSensitive( stm->setup.general_vt100j_id, False );
		XtSetSensitive( stm->setup.general_vt80_id, False );
		XtSetSensitive( stm->setup.general_jisroman_mode, False );
		XtSetSensitive( stm->setup.general_ascii_mode, False );
		XtSetSensitive( stm->setup.general_kanji_mode, False );
		XtSetSensitive( stm->setup.general_katakana_mode, False );
		XtSetSensitive( stm->setup.general_kanji_78, False );
		XtSetSensitive( stm->setup.general_kanji_83, False );
	    }
	    if ( stm->terminalType == DECwHangul ) {
		XtSetSensitive( stm->setup.general_ksroman_mode, True );
		XtSetSensitive( stm->setup.general_ksascii_mode, True );
	    } else {
		XtSetSensitive( stm->setup.general_ksroman_mode, False );
		XtSetSensitive( stm->setup.general_ksascii_mode, False );
	    }

	    if ( stm->terminalType == DECwHebrew ) {
		XtSetSensitive( stm->setup.general_upss_dec_heb, True );
		XtSetSensitive( stm->setup.general_upss_iso_heb, True );
		XtSetSensitive( stm->setup.general_ltor, True );
		XtSetSensitive( stm->setup.general_rtol, True );
	    } else {
		XtSetSensitive( stm->setup.general_upss_dec_heb, False );
		XtSetSensitive( stm->setup.general_upss_iso_heb, False );
		XtSetSensitive( stm->setup.general_ltor, False );
		XtSetSensitive( stm->setup.general_rtol, False );
	    }
	}

        /* put up general dialog box and return */
	XtUnmanageChild(GENERAL_PARENT);
	XtManageChild(GENERAL_PARENT);
	return;
}
 
void general_ok_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
	STREAM *stm;
        stm = convert_widget_to_stream(w);

	general_set_resources( stm );
#ifdef VXT_DECTERM
	if ( VxtSwPhysicalMemory() || !(VxtSwPagefileEnabled()) )
	{
	    XtDestroyWidget(GENERAL_PARENT);
	    GENERAL_PARENT = NULL;
	}
	else 
#endif VXT_DECTERM
	XtUnmanageChild(GENERAL_PARENT);
}

void general_apply_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
	STREAM *stm;
        stm = convert_widget_to_stream(w);

	general_set_resources( stm );
}

void general_cancel_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
	STREAM *stm;
        stm = convert_widget_to_stream(w);

#ifdef VXT_DECTERM
	if ( VxtSwPhysicalMemory() || !(VxtSwPagefileEnabled()) )
	{
	    XtDestroyWidget(GENERAL_PARENT);
	    GENERAL_PARENT = NULL;
	}
	else 
#endif VXT_DECTERM
	XtUnmanageChild( GENERAL_PARENT );
}

                              
general_set_resources( stm )
    STREAM *stm;
{
    Arg arglist[30];
    int n;
    int newline_button, udk_locked_button, features_locked_button,
      appl_cursor_keys_button, decterm_id_button, vt340_id_button,
      vt330_id_button,
      vt382cb_id_button, vt382k_id_button, vt382d_id_button,
      vt382_id_button, vt286_id_button, vt284_id_button, vt282_id_button, 
      vt220j_id_button, vt102j_id_button, vt100j_id_button, vt80_id_button,
      jisroman_mode_button, ascii_mode_button, kanji_mode_button,
      katakana_mode_button, kanji_78_button, kanji_83_button,
      ksroman_mode_button, ksascii_mode_button,
      vt320_id_button, vt240_id_button, vt220_id_button, vt125_id_button,
      vt102_id_button, vt101_id_button, vt100_id_button, appl_keypad_button,
      upss_dec_button, eight_bit_button, vt300_8bitc_mode_button,
      upss_dec_heb_button, upss_iso_heb_button, 
      ltor_button, rtol_button,
      upss_dec_turkish_button, upss_iso_turkish_button,
      upss_dec_greek_button, upss_iso_greek_button,
      vt300_7bitc_mode_button, vt100_mode_button, vt52_mode_button;
    Boolean upss_set = False;
      int local_echo_button;
      int conceal_answerback_button;
      char *answerback_message;

    /* read button states */
    newline_button = XmToggleButtonGetState(stm->setup.general_newline);
    udk_locked_button = XmToggleButtonGetState(stm->setup.general_udk_locked);
    features_locked_button =
      XmToggleButtonGetState(stm->setup.general_features_locked);
    local_echo_button =
      XmToggleButtonGetState(stm->setup.general_local_echo);
    conceal_answerback_button =
      XmToggleButtonGetState(stm->setup.general_conceal_answerback);
    if ( conceal_answerback_button )
	{
	answerback_message = XtNewString (stm->concealed_ans);
	}
    else
	{
        answerback_message = XmTextGetString(stm->setup.general_answerback_message);
	}
    appl_cursor_keys_button =
      XmToggleButtonGetState(stm->setup.general_appl_cursor_keys);
    if ( multi_uid || stm->terminalType == DECwKanji ) {
    decterm_id_button = XmToggleButtonGetState(stm->setup.general_decterm_id);
    vt382_id_button = XmToggleButtonGetState(stm->setup.general_vt382_id);
    vt320_id_button = XmToggleButtonGetState(stm->setup.general_vt320_id);
    vt286_id_button = XmToggleButtonGetState(stm->setup.general_vt286_id);
    vt284_id_button = XmToggleButtonGetState(stm->setup.general_vt284_id);
    vt282_id_button = XmToggleButtonGetState(stm->setup.general_vt282_id);
    vt220j_id_button = XmToggleButtonGetState(stm->setup.general_vt220j_id);
    vt102j_id_button = XmToggleButtonGetState(stm->setup.general_vt102j_id);
    vt102_id_button = XmToggleButtonGetState(stm->setup.general_vt102_id);
    vt101_id_button = XmToggleButtonGetState(stm->setup.general_vt101_id);
    vt100j_id_button = XmToggleButtonGetState(stm->setup.general_vt100j_id);
    vt100_id_button = XmToggleButtonGetState(stm->setup.general_vt100_id);
    vt80_id_button = XmToggleButtonGetState(stm->setup.general_vt80_id);
    }
    if ( multi_uid || stm->terminalType == DECwHanzi ) {
    decterm_id_button = XmToggleButtonGetState(stm->setup.general_decterm_id);
    vt382cb_id_button = XmToggleButtonGetState(stm->setup.general_vt382cb_id);
    vt320_id_button = XmToggleButtonGetState(stm->setup.general_vt320_id);
    vt220_id_button = XmToggleButtonGetState(stm->setup.general_vt220_id);
    vt102_id_button = XmToggleButtonGetState(stm->setup.general_vt102_id);
    vt101_id_button = XmToggleButtonGetState(stm->setup.general_vt101_id);
    vt100_id_button = XmToggleButtonGetState(stm->setup.general_vt100_id);
    }
    if ( multi_uid || stm->terminalType == DECwHangul ) {
    decterm_id_button = XmToggleButtonGetState(stm->setup.general_decterm_id);
    vt382k_id_button = XmToggleButtonGetState(stm->setup.general_vt382k_id);
    vt320_id_button = XmToggleButtonGetState(stm->setup.general_vt320_id);
    vt220_id_button = XmToggleButtonGetState(stm->setup.general_vt220_id);
    vt102_id_button = XmToggleButtonGetState(stm->setup.general_vt102_id);
    vt101_id_button = XmToggleButtonGetState(stm->setup.general_vt101_id);
    vt100_id_button = XmToggleButtonGetState(stm->setup.general_vt100_id);
    }
    if ( multi_uid || stm->terminalType == DECwHanyu ) {
    decterm_id_button = XmToggleButtonGetState(stm->setup.general_decterm_id);
    vt382d_id_button = XmToggleButtonGetState(stm->setup.general_vt382d_id);
    vt320_id_button = XmToggleButtonGetState(stm->setup.general_vt320_id);
    vt220_id_button = XmToggleButtonGetState(stm->setup.general_vt220_id);
    vt102_id_button = XmToggleButtonGetState(stm->setup.general_vt102_id);
    vt101_id_button = XmToggleButtonGetState(stm->setup.general_vt101_id);
    vt100_id_button = XmToggleButtonGetState(stm->setup.general_vt100_id);
    }
    if ( multi_uid || !IsAsianTerminalType(stm->terminalType) ) {
    decterm_id_button = XmToggleButtonGetState(stm->setup.general_decterm_id);
    vt340_id_button = XmToggleButtonGetState(stm->setup.general_vt340_id);
    vt330_id_button = XmToggleButtonGetState(stm->setup.general_vt330_id);
    vt320_id_button = XmToggleButtonGetState(stm->setup.general_vt320_id);
    vt240_id_button = XmToggleButtonGetState(stm->setup.general_vt240_id);
    vt220_id_button = XmToggleButtonGetState(stm->setup.general_vt220_id);
    vt125_id_button = XmToggleButtonGetState(stm->setup.general_vt125_id);
    vt102_id_button = XmToggleButtonGetState(stm->setup.general_vt102_id);
    vt101_id_button = XmToggleButtonGetState(stm->setup.general_vt101_id);
    vt100_id_button = XmToggleButtonGetState(stm->setup.general_vt100_id);
    }
    appl_keypad_button =
      XmToggleButtonGetState(stm->setup.general_appl_keypad);
    upss_dec_button = XmToggleButtonGetState(stm->setup.general_upss_dec);
    if ( multi_uid || stm->terminalType == DECwKanji ) {
    jisroman_mode_button = XmToggleButtonGetState(stm->setup.general_jisroman_mode);
    ascii_mode_button = XmToggleButtonGetState(stm->setup.general_ascii_mode);
    kanji_mode_button = XmToggleButtonGetState(stm->setup.general_kanji_mode);
    katakana_mode_button = XmToggleButtonGetState(stm->setup.general_katakana_mode);
    kanji_78_button = XmToggleButtonGetState(stm->setup.general_kanji_78);
    kanji_83_button = XmToggleButtonGetState(stm->setup.general_kanji_83);
    }
    if ( multi_uid || stm->terminalType == DECwHangul ) {
    ksroman_mode_button = XmToggleButtonGetState(stm->setup.general_ksroman_mode);
    ksascii_mode_button = XmToggleButtonGetState(stm->setup.general_ksascii_mode);
    }
    if ( multi_uid || !IsAsianTerminalType(stm->terminalType) )
    {
    eight_bit_button = XmToggleButtonGetState(stm->setup.general_eight_bit);
    }
    if ( multi_uid || stm->terminalType == DECwHebrew ) {
        upss_dec_heb_button = XmToggleButtonGetState(
					stm->setup.general_upss_dec_heb);
        upss_iso_heb_button = XmToggleButtonGetState(
					stm->setup.general_upss_iso_heb);
        ltor_button = XmToggleButtonGetState(stm->setup.general_ltor);
        rtol_button = XmToggleButtonGetState(stm->setup.general_rtol);
    }

    if ( stm->terminalType == DECwTurkish ) {
        upss_dec_turkish_button = XmToggleButtonGetState(
					stm->setup.general_upss_dec_turkish);
        upss_iso_turkish_button = XmToggleButtonGetState(
					stm->setup.general_upss_iso_turkish);
    }

    if ( stm->terminalType == DECwGreek ) {
        upss_dec_greek_button = XmToggleButtonGetState(
					stm->setup.general_upss_dec_greek);
        upss_iso_greek_button = XmToggleButtonGetState(
					stm->setup.general_upss_iso_greek);
    }

    vt300_8bitc_mode_button =
      XmToggleButtonGetState(stm->setup.general_vt300_8bitc_mode );
    vt300_7bitc_mode_button =
      XmToggleButtonGetState(stm->setup.general_vt300_7bitc_mode );
    vt100_mode_button = XmToggleButtonGetState(stm->setup.general_vt100_mode);
    vt52_mode_button = XmToggleButtonGetState(stm->setup.general_vt52_mode);

    /* set the resources (even if not changed) */
    n = 0;
    XtSetArg( arglist[n], DECwNnewLineMode, newline_button ); n++;
    XtSetArg( arglist[n], DECwNlockUDK, udk_locked_button ); n++;
    XtSetArg( arglist[n], DECwNlockUserFeatures, features_locked_button ); n++;
    XtSetArg( arglist[n], DECwNlocalEcho, local_echo_button); n++;
    XtSetArg( arglist[n], DECwNconcealAnswerback, 
			conceal_answerback_button); n++;
    XtSetArg( arglist[n], DECwNanswerbackMessage, answerback_message); n++;
    XtSetArg( arglist[n], DECwNapplicationCursorKeyMode,
	appl_cursor_keys_button );  n++;
    if ( multi_uid || stm->terminalType == DECwKanji ) {
    if ( decterm_id_button )
	{ XtSetArg( arglist[n], DECwNresponseDA, DECwDECtermID ); n++; }
    else if ( vt382_id_button )
	{ XtSetArg( arglist[n], DECwNresponseDA, DECwVT382_ID ); n++; }
    else if ( vt286_id_button )
	{ XtSetArg( arglist[n], DECwNresponseDA, DECwVT286_ID ); n++; }
    else if ( vt284_id_button )
	{ XtSetArg( arglist[n], DECwNresponseDA, DECwVT284_ID ); n++; }
    else if ( vt282_id_button )
	{ XtSetArg( arglist[n], DECwNresponseDA, DECwVT282_ID ); n++; }
    else if ( vt220j_id_button )
	{ XtSetArg( arglist[n], DECwNresponseDA, DECwVT220J_ID ); n++; }
    else if ( vt102j_id_button )
	{ XtSetArg( arglist[n], DECwNresponseDA, DECwVT102J_ID ); n++; }
    else if ( vt102_id_button )
	{ XtSetArg( arglist[n], DECwNresponseDA, DECwVT102_ID ); n++; }
    else if ( vt101_id_button )
	{ XtSetArg( arglist[n], DECwNresponseDA, DECwVT101_ID ); n++; }
    else if ( vt100j_id_button )
	{ XtSetArg( arglist[n], DECwNresponseDA, DECwVT100J_ID ); n++; }
    else if ( vt100_id_button )
	{ XtSetArg( arglist[n], DECwNresponseDA, DECwVT100_ID ); n++; }
    else if ( vt80_id_button )
	{ XtSetArg( arglist[n], DECwNresponseDA, DECwVT80_ID ); n++; }
    }
    if ( multi_uid || stm->terminalType == DECwHanzi ) {
    if ( decterm_id_button )
	{ XtSetArg( arglist[n], DECwNresponseDA, DECwDECtermID ); n++; }
    else if ( vt382cb_id_button )
	{ XtSetArg( arglist[n], DECwNresponseDA, DECwVT382CB_ID ); n++; }
    else if ( vt220_id_button )
	{ XtSetArg( arglist[n], DECwNresponseDA, DECwVT220_ID ); n++; }
    else if ( vt102_id_button )
	{ XtSetArg( arglist[n], DECwNresponseDA, DECwVT102_ID ); n++; }
    else if ( vt101_id_button )
	{ XtSetArg( arglist[n], DECwNresponseDA, DECwVT101_ID ); n++; }
    else if ( vt100_id_button )
	{ XtSetArg( arglist[n], DECwNresponseDA, DECwVT100_ID ); n++; }
    }
    if ( multi_uid || stm->terminalType == DECwHangul ) {
    if ( decterm_id_button )
	{ XtSetArg( arglist[n], DECwNresponseDA, DECwDECtermID ); n++; }
    else if ( vt382k_id_button )
	{ XtSetArg( arglist[n], DECwNresponseDA, DECwVT382K_ID ); n++; }
    else if ( vt220_id_button )
	{ XtSetArg( arglist[n], DECwNresponseDA, DECwVT220_ID ); n++; }
    else if ( vt102_id_button )
	{ XtSetArg( arglist[n], DECwNresponseDA, DECwVT102_ID ); n++; }
    else if ( vt101_id_button )
	{ XtSetArg( arglist[n], DECwNresponseDA, DECwVT101_ID ); n++; }
    else if ( vt100_id_button )
	{ XtSetArg( arglist[n], DECwNresponseDA, DECwVT100_ID ); n++; }
    }
    if ( multi_uid || stm->terminalType == DECwHanyu ) {
    if ( decterm_id_button )
	{ XtSetArg( arglist[n], DECwNresponseDA, DECwDECtermID ); n++; }
    else if ( vt382d_id_button )
	{ XtSetArg( arglist[n], DECwNresponseDA, DECwVT382D_ID ); n++; }
    else if ( vt220_id_button )
	{ XtSetArg( arglist[n], DECwNresponseDA, DECwVT220_ID ); n++; }
    else if ( vt102_id_button )
	{ XtSetArg( arglist[n], DECwNresponseDA, DECwVT102_ID ); n++; }
    else if ( vt101_id_button )
	{ XtSetArg( arglist[n], DECwNresponseDA, DECwVT101_ID ); n++; }
    else if ( vt100_id_button )
	{ XtSetArg( arglist[n], DECwNresponseDA, DECwVT100_ID ); n++; }
    }
    if ( multi_uid || !IsAsianTerminalType(stm->terminalType) ) {
    if ( decterm_id_button )
	{ XtSetArg( arglist[n], DECwNresponseDA, DECwDECtermID ); n++; }
    else if ( vt340_id_button )
	{ XtSetArg( arglist[n], DECwNresponseDA, DECwVT340_ID ); n++; }
    else if ( vt330_id_button )
	{ XtSetArg( arglist[n], DECwNresponseDA, DECwVT330_ID ); n++; }
    else if ( vt320_id_button )
	{ XtSetArg( arglist[n], DECwNresponseDA, DECwVT320_ID ); n++; }
    else if ( vt240_id_button )
	{ XtSetArg( arglist[n], DECwNresponseDA, DECwVT240_ID ); n++; }
    else if ( vt220_id_button )
	{ XtSetArg( arglist[n], DECwNresponseDA, DECwVT220_ID ); n++; }
    else if ( vt125_id_button )
	{ XtSetArg( arglist[n], DECwNresponseDA, DECwVT125_ID ); n++; }
    else if ( vt102_id_button )
	{ XtSetArg( arglist[n], DECwNresponseDA, DECwVT102_ID ); n++; }
    else if ( vt101_id_button )
	{ XtSetArg( arglist[n], DECwNresponseDA, DECwVT101_ID ); n++; }
    else if ( vt100_id_button )
	{ XtSetArg( arglist[n], DECwNresponseDA, DECwVT100_ID ); n++; }
    }
    XtSetArg( arglist[n], DECwNapplicationKeypadMode, appl_keypad_button ); n++;

    switch (stm->terminalType)
    {
	case DECwTurkish :
	    if ( upss_dec_turkish_button )
	    {
		XtSetArg( arglist[n], DECwNuserPreferenceSet, 
			  DECwDEC_Turkish_Supplemental); n++;
	    }
	    else if (upss_iso_turkish_button)
	    {
		XtSetArg( arglist[n], DECwNuserPreferenceSet, 
			  DECwISO_Latin5_Supplemental); n++;
	    }
	    upss_set = (upss_dec_turkish_button || upss_iso_turkish_button);
	    break;

	case DECwGreek :
	    if ( upss_dec_greek_button )
	    {
		XtSetArg( arglist[n], DECwNuserPreferenceSet, 
			  DECwDEC_Greek_Supplemental); n++;
	    }
	    else if (upss_iso_greek_button)
	    {
		XtSetArg( arglist[n], DECwNuserPreferenceSet, 
			  DECwISO_Latin7_Supplemental); n++;
	    }
	    upss_set = (upss_dec_greek_button || upss_iso_greek_button);
	    break;

	case DECwHebrew :
	    if ( upss_dec_heb_button )
	    {
		XtSetArg( arglist[n], DECwNuserPreferenceSet, 
			  DECwDEC_Hebrew_Supplemental); n++;
	    }
	    else if ( upss_iso_heb_button )
	    {
		XtSetArg( arglist[n], DECwNuserPreferenceSet, 
			  DECwISO_Latin8_Supplemental ); n++;
	    }
	    upss_set = (upss_dec_heb_button || upss_iso_heb_button);
	    break;

	default : break;
    }

    /* if upss not set, it must be either dec supplemental or iso latin1 */
    if (!upss_set)
    {
	XtSetArg( arglist[n], DECwNuserPreferenceSet, upss_dec_button ?
		DECwDEC_Supplemental : DECwISO_Latin1_Supplemental );
	n++;
    }


    if ( multi_uid || stm->terminalType == DECwKanji ) {
    if ( jisroman_mode_button )
	{ XtSetArg( arglist[n], DECwNjisRomanAsciiMode, DECwJisRomanMode ); n++; }
    else if ( ascii_mode_button )
	{ XtSetArg( arglist[n], DECwNjisRomanAsciiMode, DECwAsciiMode ); n++; }
    if ( kanji_mode_button )
	{ XtSetArg( arglist[n], DECwNkanjiKatakanaMode, DECwKanjiMode ); n++; }
    else if ( katakana_mode_button )
	{ XtSetArg( arglist[n], DECwNkanjiKatakanaMode, DECwKatakanaMode ); n++; };
    if ( kanji_78_button )
	{ XtSetArg( arglist[n], DECwNkanji_78_83, DECwKanji_78 ); n++; }
    else if ( kanji_83_button )
	{ XtSetArg( arglist[n], DECwNkanji_78_83, DECwKanji_83 ); n++; };
    }
    if ( multi_uid || stm->terminalType == DECwHangul ) {
    if ( ksroman_mode_button )
	{ XtSetArg( arglist[n], DECwNksRomanAsciiMode, DECwKsRomanMode ); n++; }
    else if ( ksascii_mode_button )
	{ XtSetArg( arglist[n], DECwNksRomanAsciiMode, DECwKsAsciiMode ); n++; }
    }
    if ( multi_uid || !IsAsianTerminalType(stm->terminalType) )
    {
    XtSetArg( arglist[n], DECwNeightBitCharacters, eight_bit_button ); n++;
    }
    if (( ltor_button ) && ( stm->terminalType == DECwHebrew ) ) {
	XtSetArg( arglist[n], DECwNrightToLeft, FALSE );
	n++;
    }
    else if (( rtol_button ) && ( stm->terminalType == DECwHebrew ) ) {
	XtSetArg( arglist[n], DECwNrightToLeft, TRUE );
	n++;
    }

    if ( vt300_8bitc_mode_button )
	{ XtSetArg( arglist[n], DECwNterminalMode, DECwVT300_8_BitMode ); n++; }
    else if ( vt300_7bitc_mode_button )
	{ XtSetArg( arglist[n], DECwNterminalMode, DECwVT300_7_BitMode ); n++; }
    else if ( vt100_mode_button )
	{ XtSetArg( arglist[n], DECwNterminalMode, DECwVT100_Mode ); n++; }
    else if ( vt52_mode_button )
	{ XtSetArg( arglist[n], DECwNterminalMode, DECwVT52_Mode ); n++; }
    XtSetValues( stm->terminal, arglist, n );
}

void newline_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id 
        if (call_data->reason == DwtCRCreate) */

	stm->setup.general_newline = w;
}

void udk_locked_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */

	stm->setup.general_udk_locked = w;
}

void features_locked_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */

	stm->setup.general_features_locked = w;
}

void local_echo_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */

	stm->setup.general_local_echo = w;
}

void answerback_message_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */

	stm->setup.general_answerback_message = w;
}

void change_conceal_answerback_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
	STREAM *stm;
	int conceal_answerback_button;

        stm = convert_widget_to_stream(w);

	/* if conceal answerback button is already on, then need to write
	   an empty string to answerback message text field, and turn off
	   the conceal answerback button */

        conceal_answerback_button =
          XmToggleButtonGetState(stm->setup.general_conceal_answerback);
	if (conceal_answerback_button)
	{
	    XmTextSetString(stm->setup.general_answerback_message,"");
	    conceal_answerback_button = 0;
 	    XmToggleButtonSetState(stm->setup.general_conceal_answerback,
 				conceal_answerback_button, False);
	}
}

void conceal_answerback_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */

	stm->setup.general_conceal_answerback = w;

}

void toggle_conceal_answerback_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;
	int conceal_answerback_button;
	int status;
    	caddr_t value_id;
    	MrmCode value_type;
	char *message;

        stm = convert_widget_to_stream(w);

	/* once the conceal_answerback toggle button is on, it cannot be turned
	   off by clicking it again.  The only way to turn it off is to click
	   on the answerback message text field */

        conceal_answerback_button =
          XmToggleButtonGetState(stm->setup.general_conceal_answerback);
	if (conceal_answerback_button == 0)
	{
	    conceal_answerback_button = 1;
 	    XmToggleButtonSetState(stm->setup.general_conceal_answerback,
 				conceal_answerback_button, False);
	}
	else
	{
	    /*
	     * Read the word "<concealed>" from the UID file.
 	     */
	    if ((status = MrmFetchLiteral(
		    s_MRMHierarchy,
		    "concealed_message_string",
		    stm->display,
		    &value_id,
		    &value_type)) != MrmSUCCESS)
	    {
		log_error(
		    "Unable to fetch application title from MRM\n" );
		if ( status == MrmNOT_FOUND )
		    log_error( "concealed_message_string literal not found\n");
	    }
	    {
	    long len, status;
	    message = (char *) DXmCvtCStoFC (value_id, &len, &status);
/*
 * We're about to set the widget string to the word "concealed".  However,
 * we first store whatever was in that string into concealed_ans so we remember
 * what to really use.  Of course, we first free any previous usage of
 * concealed_ans.
 */
	    if (! stm->concealed_ans)
		XtFree (stm->concealed_ans);
	    stm->concealed_ans = XmTextGetString (
		stm->setup.general_answerback_message);
    	    XmTextSetString(stm->setup.general_answerback_message, message);
	    XtFree (message);
	    }
	}
}

void normal_cursor_keys_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */

	stm->setup.general_normal_cursor_keys = w;
}

void appl_cursor_keys_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */

	stm->setup.general_appl_cursor_keys = w;
}

void decterm_id_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id 
        if (call_data->reason == DwtCRCreate) */

	stm->setup.general_decterm_id = w;
}

void vt340_id_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id 
        if (call_data->reason == DwtCRCreate) */

	stm->setup.general_vt340_id = w;
}

void vt330_id_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */

	stm->setup.general_vt330_id = w;
}

void vt320_id_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
     	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */

	stm->setup.general_vt320_id = w;
}

void vt240_id_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */
	
	stm->setup.general_vt240_id = w;
}

void vt220_id_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */

	stm->setup.general_vt220_id = w;
}

void vt125_id_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */

	stm->setup.general_vt125_id = w;
}

void vt102_id_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */

	stm->setup.general_vt102_id = w;
}

void vt101_id_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */

	stm->setup.general_vt101_id = w;
}

void vt100_id_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */

	stm->setup.general_vt100_id = w;
}

void vt382_id_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */
	{
            stm->setup.general_vt382_id = w;
            stm->setup.general_vt382cb_id = w;
            stm->setup.general_vt382k_id = w;
            stm->setup.general_vt382d_id = w;
	}
}

void vt382cb_id_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */
            stm->setup.general_vt382cb_id = w;
}

void vt382k_id_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */
            stm->setup.general_vt382k_id = w;
}

void vt382d_id_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */
            stm->setup.general_vt382d_id = w;
}

void vt286_id_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */
            stm->setup.general_vt286_id = w;
}

void vt284_id_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */
            stm->setup.general_vt284_id = w;
}
       
void vt282_id_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */
            stm->setup.general_vt282_id = w;
}

void vt220j_id_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */
            stm->setup.general_vt220j_id = w;
}

void vt102j_id_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */
            stm->setup.general_vt102j_id = w;
}

void vt100j_id_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */
            stm->setup.general_vt100j_id = w;
}

void vt80_id_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */
            stm->setup.general_vt80_id = w;
}

void jisroman_mode_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */
            stm->setup.general_jisroman_mode = w;
}

void ascii_mode_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */
            stm->setup.general_ascii_mode = w;
}

void kanji_mode_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */
            stm->setup.general_kanji_mode = w;
}

void katakana_mode_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */
            stm->setup.general_katakana_mode = w;
}

void kanji_78_cb(w, closure, call_data)	/* 910903, TN301 */
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */
            stm->setup.general_kanji_78 = w;
}

void kanji_83_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */
            stm->setup.general_kanji_83 = w;
}

void ksroman_mode_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);
	stm->setup.general_ksroman_mode = w;
}

void ksascii_mode_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);
	stm->setup.general_ksascii_mode = w;
}

void regis_screen_mode_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */
            stm->setup.general_regis_screen_mode = w;
}

void numeric_keypad_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */

	stm->setup.general_numeric_keypad = w;
}

void appl_keypad_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */

	stm->setup.general_appl_keypad = w;
}

void upss_dec_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id 
        if (call_data->reason == DwtCRCreate) */

	stm->setup.general_upss_dec = w;
}

void upss_iso_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */

	stm->setup.general_upss_iso = w;
}

void upss_dec_turkish_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);
	stm->setup.general_upss_dec_turkish = w;
}

void upss_iso_turkish_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);
	stm->setup.general_upss_iso_turkish = w;
}

void upss_dec_greek_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);
	stm->setup.general_upss_dec_greek = w;
}

void upss_iso_greek_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);
	stm->setup.general_upss_iso_greek = w;
}

void upss_dec_heb_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);
	stm->setup.general_upss_dec_heb = w;
}
void upss_iso_heb_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);
	stm->setup.general_upss_iso_heb = w;
}

void eight_bit_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */

	stm->setup.general_eight_bit = w;
}

void seven_bit_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */

	stm->setup.general_seven_bit = w;
}

void vt300_8bitc_mode_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */

	stm->setup.general_vt300_8bitc_mode = w;
}

void vt300_7bitc_mode_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id 
        if (call_data->reason == DwtCRCreate) */

	stm->setup.general_vt300_7bitc_mode = w;
}

void vt100_mode_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id 
        if (call_data->reason == DwtCRCreate) */

	stm->setup.general_vt100_mode = w;
}

void vt52_mode_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id 
        if (call_data->reason == DwtCRCreate) */

	stm->setup.general_vt52_mode = w;
}

void cursor_ltor_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);
	stm->setup.general_ltor = w;
}

void cursor_rtol_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);
	stm->setup.general_rtol = w;
}

/******************************
 *  keyboard setup callbacks  *
 ******************************/

void setup_keyboard_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
   	STREAM *stm;
	int n;
	Arg arglist[20];
	Boolean warningBellEnable, marginBellEnable, controlQSHold,
	  autoRepeatEnable;
#ifdef SECURE_KEYBOARD
	Boolean allow_quickcopy;
#endif
	DECwBackarrowKey backarrowKey;
	DECwOpenQuoteTildeKey openQuoteTildeKey;
	DECwAngleBracketsKey angleBracketsKey;
	DECwPeriodCommaKeys periodCommaKeys;
#if !defined (VXT_DECTERM)
	DECwF11EscapeKey f11EscapeKey;
#endif

        stm = convert_widget_to_stream(w);

        /* get and save current setup state from widget */
	n = 0;
	XtSetArg( arglist[n], DECwNwarningBellEnable,
		   &warningBellEnable); n++;
	XtSetArg( arglist[n], DECwNmarginBellEnable,
		   &marginBellEnable); n++;
	XtSetArg( arglist[n], DECwNcontrolQSHold,
		   &controlQSHold); n++;
#ifdef SECURE_KEYBOARD
	XtSetArg( arglist[n], "allowQuickCopy",
		   &allow_quickcopy); n++;
#endif
	XtSetArg( arglist[n], DECwNautoRepeatEnable,
		   &autoRepeatEnable); n++;
	XtSetArg( arglist[n], DECwNbackarrowKey,
		   &backarrowKey); n++;
	XtSetArg( arglist[n], DECwNopenQuoteTildeKey,
		   &openQuoteTildeKey); n++;
	XtSetArg( arglist[n], DECwNangleBracketsKey,
		   &angleBracketsKey); n++;
	XtSetArg( arglist[n], DECwNperiodCommaKeys,
		   &periodCommaKeys); n++;
#if !defined(VXT_DECTERM)
	XtSetArg( arglist[n], DECwNf11EscapeKey,
		   &f11EscapeKey); n++;
#endif
	XtGetValues( stm->terminal, arglist, n);


        /* check if we need to fetch the widgets from disk */
	if (KEYBOARD_PARENT == 0)
            {

            /* fetch dialog box from DRM */
            if (MrmFetchWidget
                   (
                   s_MRMHierarchy,
                   "setup_keyboard_db",
                   stm->parent,
                   & KEYBOARD_PARENT,
                   & dummy_class
                   )
               != MrmSUCCESS)
                {
                log_error(
			"Unable to fetch widget definitions from DRM\n");
	        return;
                }
            }

	/* initialize widgets to current state */
  	XmToggleButtonSetState(stm->setup.keyboard_warning_bell_enable,
 		warningBellEnable);
  	XmToggleButtonSetState(stm->setup.keyboard_margin_bell_enable,
 		marginBellEnable);
  	XmToggleButtonSetState(stm->setup.keyboard_control_QS_hold,
 		controlQSHold);
#ifdef SECURE_KEYBOARD
  	XmToggleButtonSetState(stm->setup.keyboard_allow_quickcopy,
 		allow_quickcopy);
#endif
  	XmToggleButtonSetState(stm->setup.keyboard_auto_repeat_enable,
 		autoRepeatEnable);
  	XmToggleButtonSetState(stm->setup.keyboard_backarrow_backspace,
 		(backarrowKey == DECwBackarrowBackspace) );
  	XmToggleButtonSetState(stm->setup.keyboard_backarrow_delete,
 		(backarrowKey == DECwBackarrowDelete) );
  	XmToggleButtonSetState(stm->setup.keyboard_e00_escape,
 		(openQuoteTildeKey == DECwTildeEscape) );
  	XmToggleButtonSetState(stm->setup.keyboard_e00_open_quote_tilde,
 		(openQuoteTildeKey == DECwTildeOpenQuoteTilde) );
  	XmToggleButtonSetState(stm->setup.keyboard_b00_open_quote_tilde,
 		(angleBracketsKey == DECwAngleOpenQuoteTilde) );
  	XmToggleButtonSetState(stm->setup.keyboard_b00_angle_brackets,
 		(angleBracketsKey == DECwAngleAngleBrackets) );
  	XmToggleButtonSetState(stm->setup.keyboard_b08_comma_leftangle,
 		(periodCommaKeys == DECwCommaAngleBrackets) );
  	XmToggleButtonSetState(stm->setup.keyboard_b08_comma_comma,
 		(periodCommaKeys == DECwCommaPeriodComma) );
#if !defined(VXT_DECTERM)
	XmToggleButtonSetState(stm->setup.keyboard_g11_escape,
		(f11EscapeKey == DECwF11Escape) );
	XmToggleButtonSetState(stm->setup.keyboard_g11_f11,
                (f11EscapeKey == DECwF11F11) );
#endif

                               
        /* put up keyboard dialog box and return */
	XtUnmanageChild(KEYBOARD_PARENT);
	XtManageChild(KEYBOARD_PARENT);
	return;
}
 
void keyboard_ok_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
	STREAM *stm;
        stm = convert_widget_to_stream(w);

	keyboard_set_resources( stm );
#ifdef VXT_DECTERM
	if ( VxtSwPhysicalMemory() || !(VxtSwPagefileEnabled()) )
	{
	    XtDestroyWidget(KEYBOARD_PARENT);
	    KEYBOARD_PARENT = NULL;
	}
	else 
#endif VXT_DECTERM
	XtUnmanageChild(KEYBOARD_PARENT);
}
 
void keyboard_apply_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
	STREAM *stm;
        stm = convert_widget_to_stream(w);

	keyboard_set_resources( stm );
}
 
void keyboard_cancel_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{         
   	STREAM *stm;
        stm = convert_widget_to_stream(w);

#ifdef VXT_DECTERM
	if ( VxtSwPhysicalMemory() || !(VxtSwPagefileEnabled()) )
	{
	    XtDestroyWidget(KEYBOARD_PARENT);
	    KEYBOARD_PARENT = NULL;
	}
	else 
#endif VXT_DECTERM
	XtUnmanageChild(KEYBOARD_PARENT);
}

keyboard_set_resources( stm )
    STREAM *stm;
{
    Arg arglist[20];
    int n;
    int warning_bell_enable_button, margin_bell_enable_button,
      control_QS_hold_button, auto_repeat_enable_button,
#ifdef SECURE_KEYBOARD
      allow_quickcopy_button,
#endif
      backarrow_backspace_button,
      e00_escape_button, b00_open_quote_tilde_button,
      b08_comma_leftangle_button, g11_f11_escape_button;

    /* read button states */
    warning_bell_enable_button =
      XmToggleButtonGetState( stm->setup.keyboard_warning_bell_enable );
    margin_bell_enable_button =
      XmToggleButtonGetState( stm->setup.keyboard_margin_bell_enable );
    control_QS_hold_button =
      XmToggleButtonGetState( stm->setup.keyboard_control_QS_hold );
#ifdef SECURE_KEYBOARD
    allow_quickcopy_button =
      XmToggleButtonGetState( stm->setup.keyboard_allow_quickcopy );
#endif
    auto_repeat_enable_button =
      XmToggleButtonGetState( stm->setup.keyboard_auto_repeat_enable );
    backarrow_backspace_button =
      XmToggleButtonGetState( stm->setup.keyboard_backarrow_backspace );
    e00_escape_button =
      XmToggleButtonGetState( stm->setup.keyboard_e00_escape );
    b00_open_quote_tilde_button =
      XmToggleButtonGetState( stm->setup.keyboard_b00_open_quote_tilde );
    b08_comma_leftangle_button =
      XmToggleButtonGetState( stm->setup.keyboard_b08_comma_leftangle );
#if !defined (VXT_DECTERM)
    g11_f11_escape_button =
      XmToggleButtonGetState( stm->setup.keyboard_g11_escape);
#endif

    /* set the resources (whether changed or not) */
    n = 0;
    XtSetArg( arglist[n], DECwNwarningBellEnable, warning_bell_enable_button );
	n++;
    XtSetArg( arglist[n], DECwNmarginBellEnable, margin_bell_enable_button );
	n++;
    XtSetArg( arglist[n], DECwNcontrolQSHold, control_QS_hold_button );
	n++;
#ifdef SECURE_KEYBOARD
    XtSetArg( arglist[n], DECwNallowQuickCopy, allow_quickcopy_button );
	n++;
#endif
    XtSetArg( arglist[n], DECwNautoRepeatEnable, auto_repeat_enable_button );
	n++;
    XtSetArg( arglist[n], DECwNbackarrowKey, backarrow_backspace_button ?
      DECwBackarrowBackspace : DECwBackarrowDelete );
	n++;
    XtSetArg( arglist[n], DECwNopenQuoteTildeKey, e00_escape_button ?
      DECwTildeEscape : DECwTildeOpenQuoteTilde );
	n++;
    XtSetArg( arglist[n], DECwNangleBracketsKey, b00_open_quote_tilde_button ?
      DECwAngleOpenQuoteTilde : DECwAngleAngleBrackets );
	n++;
    XtSetArg( arglist[n], DECwNperiodCommaKeys, b08_comma_leftangle_button ?
      DECwCommaAngleBrackets : DECwCommaPeriodComma );
	n++;
#if !defined(VXT_DECTERM)
    XtSetArg( arglist[n], DECwNf11EscapeKey, g11_f11_escape_button ?
      DECwF11Escape : DECwF11F11 );
        n++;
#endif
    XtSetValues( stm->terminal, arglist, n );
}

void warning_bell_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id 
        if (call_data->reason == DwtCRCreate) */

	stm->setup.keyboard_warning_bell_enable = w;
}

void margin_bell_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id 
        if (call_data->reason == DwtCRCreate) */

	stm->setup.keyboard_margin_bell_enable = w;
}

void control_QS_hold_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id 
        if (call_data->reason == DwtCRCreate) */

	stm->setup.keyboard_control_QS_hold = w;
}

#ifdef SECURE_KEYBOARD
void allow_quickcopy_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */
            stm->setup.keyboard_allow_quickcopy = w;
}
#endif

void auto_repeat_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id 
        if (call_data->reason == DwtCRCreate) */

	stm->setup.keyboard_auto_repeat_enable = w;
}

void backarrow_BS_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id 
        if (call_data->reason == DwtCRCreate) */

	stm->setup.keyboard_backarrow_backspace = w;
}

void backarrow_DEL_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id 
        if (call_data->reason == DwtCRCreate) */

	stm->setup.keyboard_backarrow_delete = w;
}

void comma_comma_cb(w, closure, call_data)
Widget w;
caddr_t closure;          
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id 
        if (call_data->reason == DwtCRCreate) */

	stm->setup.keyboard_b08_comma_comma = w;
}

void comma_angle_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id 
        if (call_data->reason == DwtCRCreate) */

	stm->setup.keyboard_b08_comma_leftangle = w;
}

#if !defined(VXT_DECTERM)

void f11_f11_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
        STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */
            stm->setup.keyboard_g11_f11 = w;
}

void f11_escape_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
        STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id
        if (call_data->reason == DwtCRCreate) */
            stm->setup.keyboard_g11_escape = w;
}
#endif

void tilde_tilde_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id 
        if (call_data->reason == DwtCRCreate) */

	stm->setup.keyboard_e00_open_quote_tilde = w;
}

void tilde_escape_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id 
        if (call_data->reason == DwtCRCreate) */

	stm->setup.keyboard_e00_escape = w;
}

void angle_angle_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id 
        if (call_data->reason == DwtCRCreate) */

	stm->setup.keyboard_b00_angle_brackets = w;
}

void angle_tilde_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
	There is no Motif equivalent to the DwtCRCreate call back reason.
	Looking at the UIL code this call back is only referenced as the
	MrmNcreateCallback so a check for the create reason was taken out and
 	the widget id is just logged.
*/
{
	STREAM *stm;

        stm = convert_widget_to_stream(w);

        /* if widget was just created, log its widget id 
        if (call_data->reason == DwtCRCreate) */

	stm->setup.keyboard_b00_open_quote_tilde = w;
}



/*****************************
 *  dialect setup callbacks  *
 *****************************/

void setup_keyboarddialect_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
	STREAM *stm;
        int n;
        Arg arglist[2];
	DECwKeyboardDialect keyboardDialect;

        stm = convert_widget_to_stream(w);


        /* get and save current setup state from widget */
	n = 0;
	XtSetArg( arglist[n], DECwNkeyboardDialect, &keyboardDialect); n++;
	XtGetValues( stm->terminal, arglist, n);

        /* check if we need to fetch the widgets from disk */
	if (DIALECT_PARENT == 0)
            {

            /* fetch dialog box from DRM */
            if (MrmFetchWidget
                   (
                   s_MRMHierarchy,
                   "setup_keyboard_dialect_db",
                   stm->parent,
                   & DIALECT_PARENT,
                   & dummy_class
                   )
               != MrmSUCCESS)
                {
                log_error(
			"Unable to fetch widget definitions from DRM\n");
	        return;
                }
            }

	/* initialize list box to current state */
	XmListSelectPos( stm->setup.dialect_select, (int)keyboardDialect + 1,
	    False );
		/* note: this assumes that the dialects appear in the list
		   box in the same order than they are defined in decterm.h */

        /* put up keyboard dialect dialog box and return */
	XtUnmanageChild(DIALECT_PARENT);
	XtManageChild(DIALECT_PARENT);
	return;
}
 
void dialect_ok_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
	STREAM *stm;
        stm = convert_widget_to_stream(w);
  
	dialect_set_resources( stm );
#ifdef VXT_DECTERM
	if ( VxtSwPhysicalMemory() || !(VxtSwPagefileEnabled()) )
	{
	    XtDestroyWidget(DIALECT_PARENT);
	    DIALECT_PARENT = NULL;
	}
	else 
#endif VXT_DECTERM
	XtUnmanageChild(DIALECT_PARENT);
}

void dialect_apply_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
	STREAM *stm;
        stm = convert_widget_to_stream(w);

	dialect_set_resources( stm );
}

void dialect_cancel_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{         
	STREAM *stm;

	/* this is needed because DIALECT_PARENT is a macro of
	 * stm->setup.dialect_parent and we need to have an updated stm.
	 */
        stm = convert_widget_to_stream(w);

#ifdef VXT_DECTERM
	if ( VxtSwPhysicalMemory() || !(VxtSwPagefileEnabled()) )
	{
	    XtDestroyWidget(DIALECT_PARENT);
	    DIALECT_PARENT = NULL;
	}
	else 
#endif VXT_DECTERM
	XtUnmanageChild(DIALECT_PARENT);
}

dialect_set_resources( stm )
    STREAM *stm;
{
    Arg arglist[2];
    int n;
    int *selected_list;
    int position_count;

/* 
note: this assumes that the dialects are in the same order
      in the list box as in the definition of the DECwKeyboardDialect enum 
*/

	/* position_count should always be 1 and selected_list should be
	 * the position of the selected item
	 */
	if (XmListGetSelectedPos( stm->setup.dialect_select,
				  &selected_list, &position_count) )
	{
	    XtSetArg( arglist[0], DECwNkeyboardDialect, selected_list[0] - 1);
	    XtSetValues( stm->terminal, arglist, 1 );
	    XtFree( (char *)selected_list );
	}
}
/*
There is no Motif equivalent to the DwtCRCreate call back reason.
The dialect_select_cb is now only used for the single select callback.
dialect_create_cb is new for the creation callback.
*/

void dialect_create_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmListCallbackStruct *call_data;
{
	STREAM *stm;
	Arg arglist[1];

	stm = convert_widget_to_stream(w);

	stm->setup.dialect_select = w;
}

/**************************
 *  tabs setup callbacks  *
 **************************/

void setup_tabs_cb(){};




/******************************
 *  graphics setup callbacks  *
 ******************************/

void setup_graphics_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
	STREAM *stm;
	int n, shareColormapEntries, backingStoreEnable,
	  macrographReportEnable, bitPlanes;
	int regisScreenMode;
	Arg arglist[15];
	char bit_planes_text[3];

        stm = convert_widget_to_stream(w);


        /* get and save current settings from terminal widget */
	n = 0;
	XtSetArg( arglist[n], DECwNshareColormapEntries,
		&shareColormapEntries );  n++;
	XtSetArg( arglist[n], DECwNbackingStoreEnable, &backingStoreEnable );
		n++;
	XtSetArg( arglist[n], DECwNmacrographReportEnable,
		&macrographReportEnable ); n++;
	if ( multi_uid || IsAsianTerminalType(stm->terminalType) ) {
	XtSetArg( arglist[n], DECwNregisScreenMode, &regisScreenMode ); n++;
	}
	XtSetArg( arglist[n], DECwNbitPlanes, &bitPlanes ); n++;

  	XtGetValues( stm->terminal, arglist, n);


        /* check if we need to fetch the widgets from disk */
     	if (GRAPHICS_PARENT == 0)
            {

            /* fetch dialog box from DRM */
            if (MrmFetchWidget
                   (
                   s_MRMHierarchy,
                   "setup_graphics_db",
                   stm->parent,
                   & GRAPHICS_PARENT,
                   & dummy_class
                   )
               != MrmSUCCESS)
                {
                log_error(
			"Unable to fetch widget definitions from DRM\n");
	        return;
                }
            }

	/* initialize widgets to current state */
	XmToggleButtonSetState(stm->setup.graphics_share_colormap_entries,
		shareColormapEntries);
	XmToggleButtonSetState(stm->setup.graphics_enable_backing_store,
		backingStoreEnable);
	XmToggleButtonSetState(stm->setup.graphics_macrograph_report,
		macrographReportEnable);
	if ( multi_uid || stm->terminalType == DECwKanji )
	XmToggleButtonSetState(stm->setup.general_regis_screen_mode,
		regisScreenMode);
	sprintf(bit_planes_text, "%2d", bitPlanes);
	XmTextSetString(stm->setup.graphics_bit_planes_text,
		bit_planes_text );

	if ( multi_uid ) {
	    if ( IsAsianTerminalType(stm->terminalType) )
		XtSetSensitive( stm->setup.general_regis_screen_mode, True );
	    else
		XtSetSensitive( stm->setup.general_regis_screen_mode, False );
	}

        /* put up graphics dialog box and return */
	XtUnmanageChild(GRAPHICS_PARENT);
	XtManageChild(GRAPHICS_PARENT);
	return;
}
            
void graphics_ok_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
	STREAM *stm;
        stm = convert_widget_to_stream(w);

	graphics_set_resources( stm );
#ifdef VXT_DECTERM
	if ( VxtSwPhysicalMemory() || !(VxtSwPagefileEnabled()) )
	{
	    XtDestroyWidget(GRAPHICS_PARENT);
	    GRAPHICS_PARENT = NULL;
	}
	else 
#endif VXT_DECTERM
	XtUnmanageChild(GRAPHICS_PARENT);
}

void graphics_apply_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
	STREAM *stm;
        stm = convert_widget_to_stream(w);

	graphics_set_resources( stm );
}

void graphics_cancel_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{                              
	STREAM *stm;
        stm = convert_widget_to_stream(w);

#ifdef VXT_DECTERM
	if ( VxtSwPhysicalMemory() || !(VxtSwPagefileEnabled()) )
	{
	    XtDestroyWidget(GRAPHICS_PARENT);
	    GRAPHICS_PARENT = NULL;
	}
	else 
#endif VXT_DECTERM
	XtUnmanageChild( GRAPHICS_PARENT );
}

graphics_set_resources( stm )
    STREAM *stm;
{
    Arg arglist[20];
    int n;
    int shareColormapEntries_button, backingStoreEnable_button,
      macrographReportEnable_button, bit_planes;
    int regis_screen_mode_button;
    char *bit_planes_text;

    /* read button states */
    shareColormapEntries_button =
      XmToggleButtonGetState(stm->setup.graphics_share_colormap_entries);
    backingStoreEnable_button =
      XmToggleButtonGetState(stm->setup.graphics_enable_backing_store);
    macrographReportEnable_button =
      XmToggleButtonGetState(stm->setup.graphics_macrograph_report);
    if ( multi_uid || stm->terminalType == DECwKanji )
    regis_screen_mode_button =
      XmToggleButtonGetState(stm->setup.general_regis_screen_mode);
    bit_planes_text =
      XmTextGetString(stm->setup.graphics_bit_planes_text);

    bit_planes = atoi( bit_planes_text );
    if ( bit_planes < 0 )
	bit_planes = 0;

    /* set the resources (even if not changed ) */
    n = 0;
    XtSetArg( arglist[n], DECwNshareColormapEntries,
	    shareColormapEntries_button );  n++;
    XtSetArg( arglist[n], DECwNbackingStoreEnable, backingStoreEnable_button );
	    n++;
    XtSetArg( arglist[n], DECwNmacrographReportEnable,
	    macrographReportEnable_button ); n++;
    if ( multi_uid || stm->terminalType == DECwKanji ) {
    XtSetArg( arglist[n], DECwNregisScreenMode, regis_screen_mode_button ); n++;
    }
    XtSetArg( arglist[n], DECwNbitPlanes, bit_planes ); n++;

    XtSetValues( stm->terminal, arglist, n );
    XtFree(bit_planes_text);
}

void share_colormap_entries_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
    There is no Motif equivalent to the DwtCRCreate call back reason.
    Looking at the UIL code this call back is only referenced as the
    MrmNcreateCallback so a check for the create reason was taken out and
    the widget id is just logged.
*/
{     
    STREAM *stm;
    stm = convert_widget_to_stream(w);

    /* if widget was just created, log its widget id 
    if (call_data->reason == DwtCRCreate) */

    stm->setup.graphics_share_colormap_entries = w;
}

void enable_backing_store_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
    There is no Motif equivalent to the DwtCRCreate call back reason.
    Looking at the UIL code this call back is only referenced as the
    MrmNcreateCallback so a check for the create reason was taken out and
    the widget id is just logged.
*/
{     
    STREAM *stm;
    stm = convert_widget_to_stream(w);

    /* if widget was just created, log its widget id 
    if (call_data->reason == DwtCRCreate) */

    stm->setup.graphics_enable_backing_store = w;
}

void macrograph_report_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
    There is no Motif equivalent to the DwtCRCreate call back reason.
    Looking at the UIL code this call back is only referenced as the
    MrmNcreateCallback so a check for the create reason was taken out and
    the widget id is just logged.
*/
{     
    STREAM *stm;
    stm = convert_widget_to_stream(w);

    /* if widget was just created, log its widget id 
    if (call_data->reason == DwtCRCreate) */

    stm->setup.graphics_macrograph_report = w;
}

void bit_planes_text_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
/*
    There is no Motif equivalent to the DwtCRCreate call back reason.
    Looking at the UIL code this call back is only referenced as the
    MrmNcreateCallback so a check for the create reason was taken out and
    the widget id is just logged.
*/
{     
    STREAM *stm;
    stm = convert_widget_to_stream(w);

    /* if widget was just created, log its widget id 
    if (call_data->reason == DwtCRCreate) */

    stm->setup.graphics_bit_planes_text = w;
}


/******************************
 *  terminaltype setup callbacks  *
 ******************************/

void setup_terminaltype_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
	STREAM *stm;
	DECwTerminalType terminalType;
	Arg arglist[1];

        stm = convert_widget_to_stream(w);
	XtSetArg( arglist[0], DECwNterminalType, &terminalType );
  	XtGetValues( stm->terminal, arglist, 1 );

        /* check if we need to fetch the widgets from disk */
     	if ( TERMINALTYPE_PARENT == 0)
            {
            /* fetch dialog box from DRM */
            if (MrmFetchWidget
                   (
                   s_MRMHierarchy,
                   "setup_terminaltype_db",
                   stm->parent,
                   & TERMINALTYPE_PARENT,
                   & dummy_class
                   )
               != MrmSUCCESS)
                {
                log_message(
			"Unable to fetch widget definitions from DRM\n");
	        return;
                }
            }

	/* initialize widgets to current state */
	XmToggleButtonSetState( stm->setup.terminaltype_standard,
		terminalType == DECwStandard );
	XmToggleButtonSetState( stm->setup.terminaltype_kanji,
		terminalType == DECwKanji );
	XmToggleButtonSetState( stm->setup.terminaltype_hanzi,
		terminalType == DECwHanzi );
	XmToggleButtonSetState( stm->setup.terminaltype_hangul,
		terminalType == DECwHangul );
	XmToggleButtonSetState( stm->setup.terminaltype_hanyu,
		terminalType == DECwHanyu );
	XmToggleButtonSetState( stm->setup.terminaltype_hebrew,
		terminalType == DECwHebrew );

        /* put up graphics dialog box and return */
	XtUnmanageChild(TERMINALTYPE_PARENT);
	XtManageChild(TERMINALTYPE_PARENT);
	return;
}
            
void terminaltype_ok_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
	STREAM *stm;
        stm = convert_widget_to_stream(w);

	terminaltype_set_resources( stm );
	XtUnmanageChild(TERMINALTYPE_PARENT);
}

void terminaltype_apply_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
	STREAM *stm;
        stm = convert_widget_to_stream(w);

	terminaltype_set_resources( stm );
}

void terminaltype_cancel_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{                              
	STREAM *stm;
        stm = convert_widget_to_stream(w);

	XtUnmanageChild( TERMINALTYPE_PARENT );
}

terminaltype_set_resources( stm )
    STREAM *stm;
{
    Arg arglist[1];
    DECwTerminalType terminalType;

    /* read button states */
    if ( XmToggleButtonGetState( stm->setup.terminaltype_standard ))
	terminalType = DECwStandard;
    else if ( XmToggleButtonGetState( stm->setup.terminaltype_kanji ))
	terminalType = DECwKanji;
    else if ( XmToggleButtonGetState( stm->setup.terminaltype_hanzi ))
	terminalType = DECwHanzi;
    else if ( XmToggleButtonGetState( stm->setup.terminaltype_hangul ))
	terminalType = DECwHangul;
    else if ( XmToggleButtonGetState( stm->setup.terminaltype_hanyu ))
	terminalType = DECwHanyu;
    else if ( XmToggleButtonGetState( stm->setup.terminaltype_hebrew ))
	terminalType = DECwHebrew;
    else
	terminalType = DECwStandard;

    /* set the resources (even if not changed ) */
    XtSetArg( arglist[0], DECwNterminalType, terminalType );
    XtSetValues( stm->terminal, arglist, 1 );
    stm->terminalType = terminalType;
}

void terminaltype_standard_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{     
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.terminaltype_standard = w;
}

void terminaltype_kanji_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{     
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.terminaltype_kanji = w;
}

void terminaltype_hanzi_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{     
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.terminaltype_hanzi = w;
}

void terminaltype_hangul_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{     
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.terminaltype_hangul = w;
}

void terminaltype_hanyu_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{     
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.terminaltype_hanyu = w;
}

void terminaltype_hebrew_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmToggleButtonCallbackStruct *call_data;
{     
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.terminaltype_hebrew = w;
}

/********************************
 *  Is printer supported *
 ********************************/

void is_printer_supported_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
#ifdef VXT_DECTERM
	Arg arglist[1];

	XtSetArg( arglist[0], XmNsensitive, False );

	/* Some hardware platforms do not have printer support, such as the
	   VT1300.  In those cases, need to gray out printer options. */

	if( VxtSystemType() != VXTDWTII)
	    XtSetValues( w, arglist, 1 );
#endif
}

/******************************
 *  printer setup callbacks   *
 ******************************/

void setup_printer_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
	STREAM *stm;
	int n;
	Arg arglist[20];
	DECwPrintingDestination destination;
	String port_name, file_name;
	DECwPrintMode print_mode;
	DECwPrintExtent print_extent;
	DECwPrintDataType print_data_type;
	Boolean form_feed_mode, to_host, graphics, background, color_mode,
		eight_bits, hls_syntax;
	DECwSixelLevel sixel_level;
	DECwDisplayMode display_mode;
	DECwPrintFormat print_format;

	stm = convert_widget_to_stream(w);

	/* get and save current settings from terminal widget */
	n = 0;

	XtSetArg( arglist[n], DECwNprintingDestination, &destination ); n++;
	XtSetArg( arglist[n], DECwNprinterPortName, &port_name ); n++;
	XtSetArg( arglist[n], DECwNprinterFileName, &file_name ); n++;
	XtSetArg( arglist[n], DECwNprintMode, &print_mode ); n++;
	XtSetArg( arglist[n], DECwNprintExtent, &print_extent ); n++;
	XtSetArg( arglist[n], DECwNprintDataType, &print_data_type ); n++;
	XtSetArg( arglist[n], DECwNprintFormFeedMode, &form_feed_mode ); n++;
	XtSetArg( arglist[n], DECwNprinterToHostEnabled, &to_host ); n++;
	XtSetArg( arglist[n], DECwNgraphicsPrintingEnabled, &graphics ); n++;
	XtSetArg( arglist[n], DECwNprintBackgroundMode, &background ); n++;
	XtSetArg( arglist[n], DECwNprintSixelLevel, &sixel_level ); n++;
	if ( multi_uid || stm->terminalType == DECwKanji ) {
	XtSetArg( arglist[n], DECwNprintDisplayMode, &display_mode ); n++;
	}
	XtSetArg( arglist[n], DECwNprintFormat, &print_format ); n++;
	XtSetArg( arglist[n], DECwNprintColorMode, &color_mode ); n++;
	XtSetArg( arglist[n], DECwNprint8BitMode, &eight_bits ); n++;
	XtSetArg( arglist[n], DECwNprintHLSColorSyntax, &hls_syntax ); n++;

	XtGetValues( stm->terminal, arglist, n );

	/* check if we need to fetch the widgets from disk */
     	if (PRINTER_PARENT == 0)
            {

            /* fetch dialog box from DRM */
            if (MrmFetchWidget
                   (
                   s_MRMHierarchy,
                   "setup_printer_db",
                   stm->parent,
                   & PRINTER_PARENT,
                   & dummy_class
                   )
               != MrmSUCCESS)
                {
                log_error(
			"Unable to fetch widget definitions from DRM\n");
	        return;
                }
            }

	/* initialize widgets to current state */

#if !defined (VXT_DECTERM)
	XmToggleButtonSetState(stm->setup.printer_queued_printer,
		(destination == DECwDestinationQueue));
	XmToggleButtonSetState(stm->setup.printer_port,
		(destination == DECwDestinationPort));
	XmToggleButtonSetState(stm->setup.printer_file,
		(destination == DECwDestinationFile));
	XmToggleButtonSetState(stm->setup.printer_none,
		(destination == DECwDestinationNone));
	XmTextSetString(stm->setup.printer_port_name, port_name );
	XmTextSetString(stm->setup.printer_file_name, file_name );
#endif

	XmToggleButtonSetState(stm->setup.printer_normal_print,
		(print_mode == DECwNormalPrintMode));
	XmToggleButtonSetState(stm->setup.printer_auto_print,
		(print_mode == DECwAutoPrintMode));
	XmToggleButtonSetState(stm->setup.printer_controller,
		(print_mode == DECwPrintControllerMode));

	XmToggleButtonSetState(stm->setup.printer_full_page,
		(print_extent == DECwFullPage));
	XmToggleButtonSetState(stm->setup.printer_full_page_transcript,
		(print_extent == DECwFullPagePlusTranscript));
	XmToggleButtonSetState(stm->setup.printer_scroll_region,
		(print_extent == DECwScrollRegionOnly));
	XmToggleButtonSetState(stm->setup.printer_selection,
		(print_extent == DECwSelectionOnly));

	XmToggleButtonSetState(stm->setup.printer_national,
		(print_data_type == DECwNationalOnly));
	XmToggleButtonSetState(stm->setup.printer_national_line_drawing,
		(print_data_type == DECwNationalPlusLineDrawing));
	XmToggleButtonSetState(stm->setup.printer_all_characters,
		(print_data_type == DECwPrintAllCharacters));

	XmToggleButtonSetState(stm->setup.printer_form_feed, form_feed_mode);
	XmToggleButtonSetState(stm->setup.printer_to_host, to_host);
	XmToggleButtonSetState(stm->setup.printer_graphics, graphics);

#ifdef VXT_DECTERM
    	if ( get_printer_config() == READ_ALLOWED )
#else
	if (1)
#endif
	{
            XtSetSensitive( stm->setup.printer_to_host, True);
	}
        else
	{
	    /* gray out printer to host button in "Printer Options" dialog box*/
            XtSetSensitive( stm->setup.printer_to_host, False);
	}

#if !defined (VXT_DECTERM)
	if ( multi_uid || !IsAsianTerminalType(stm->terminalType) )
	XmToggleButtonSetState(stm->setup.printer_la210,
		(sixel_level == DECwSixelLevel_LA210));
	if ( multi_uid ) {
	    if ( !IsAsianTerminalType(stm->terminalType) )
		XtSetSensitive( stm->setup.printer_la210, True );
	    else
		XtSetSensitive( stm->setup.printer_la210, False );
	}
	XmToggleButtonSetState(stm->setup.printer_background, background);
	XmToggleButtonSetState(stm->setup.printer_level_1,
		(sixel_level == DECwSixelLevel_1));
	XmToggleButtonSetState(stm->setup.printer_level_2,
		(sixel_level == DECwSixelLevel_2));
	XmToggleButtonSetState(stm->setup.printer_compressed,
		(print_format == DECwCompressedPrinting));
	XmToggleButtonSetState(stm->setup.printer_expanded,
		(print_format == DECwExpandedPrinting));
	XmToggleButtonSetState(stm->setup.printer_rotated,
		(print_format == DECwRotatedPrinting));

	XmToggleButtonSetState(stm->setup.printer_monochrome, !color_mode);
	XmToggleButtonSetState(stm->setup.printer_color, color_mode);

	XmToggleButtonSetState(stm->setup.printer_7_bit, !eight_bits);
	XmToggleButtonSetState(stm->setup.printer_8_bit, eight_bits);

	XmToggleButtonSetState(stm->setup.printer_hls, hls_syntax);
	XmToggleButtonSetState(stm->setup.printer_rgb, ! hls_syntax);
#endif

	if ( multi_uid || stm->terminalType == DECwKanji ) {
	XmToggleButtonSetState(stm->setup.printer_main_display_24,
		(display_mode == DECwMainDisplay24));
	XmToggleButtonSetState(stm->setup.printer_status_display_25,
		(display_mode == DECwStatusDisplay25));
	}
	if ( multi_uid ) {
	    if ( stm->terminalType == DECwKanji ) {
		XtSetSensitive( stm->setup.printer_main_display_24, True );
		XtSetSensitive( stm->setup.printer_status_display_25, True );
	    } else {
		XtSetSensitive( stm->setup.printer_main_display_24, False );
		XtSetSensitive( stm->setup.printer_status_display_25, False );
	    }
	}

	/* put up printer dialog box and return */
	XtUnmanageChild(PRINTER_PARENT);
	XtManageChild(PRINTER_PARENT);
	return;
}

void printer_ok_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
	STREAM *stm;
	stm = convert_widget_to_stream(w);

	printer_set_resources( stm );
#ifdef VXT_DECTERM
	if ( VxtSwPhysicalMemory() || !(VxtSwPagefileEnabled()) )
	{
	    XtDestroyWidget(PRINTER_PARENT);
	    PRINTER_PARENT = NULL;
	}
	else 
#endif VXT_DECTERM
	XtUnmanageChild(PRINTER_PARENT);
}

printer_apply_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
	STREAM *stm;
	stm = convert_widget_to_stream(w);

	printer_set_resources( stm );
}

void printer_cancel_cb(w, closure, call_data)
Widget w;
caddr_t closure;
caddr_t call_data;
{
	STREAM *stm;
	stm = convert_widget_to_stream(w);

#ifdef VXT_DECTERM
	if ( VxtSwPhysicalMemory() || !(VxtSwPagefileEnabled()) )
	{
	    XtDestroyWidget(PRINTER_PARENT);
	    PRINTER_PARENT = NULL;
	}
	else 
#endif VXT_DECTERM
	XtUnmanageChild( PRINTER_PARENT );
}

printer_set_resources( stm )
    STREAM *stm;
{
    int n;
    Arg arglist[20];
    int queued_printer_button, printer_port_button, printer_file_button,
	normal_print_button, auto_print_button, full_page_button,
	full_page_transcript_button, scroll_region_button, national_button,
	national_line_drawing_button, form_feed_button, printer_to_host_button,
	graphics_button, background_button, level_1_button, level_2_button,
	print_main_button,print_status_button,
	compressed_button, expanded_button, color_button, eight_bit_button,
        hls_button;
    DECwPrintingDestination destination;
    String port_name, file_name;
    DECwPrintMode print_mode;
    DECwPrintExtent print_extent;
    DECwPrintDataType print_data_type;
    DECwSixelLevel sixel_level;
    DECwDisplayMode display_mode;
    DECwPrintFormat print_format;

    /* read button states */

#if !defined (VXT_DECTERM)
    queued_printer_button =
      XmToggleButtonGetState(stm->setup.printer_queued_printer);
    printer_port_button =
      XmToggleButtonGetState(stm->setup.printer_port);
    printer_file_button =
      XmToggleButtonGetState(stm->setup.printer_file);
    port_name = XmTextGetString(stm->setup.printer_port_name);
    file_name = XmTextGetString(stm->setup.printer_file_name);
    background_button = XmToggleButtonGetState(stm->setup.printer_background);
    level_1_button =
      XmToggleButtonGetState(stm->setup.printer_level_1);
    level_2_button =
      XmToggleButtonGetState(stm->setup.printer_level_2);
    compressed_button =
      XmToggleButtonGetState(stm->setup.printer_compressed);
    expanded_button =
      XmToggleButtonGetState(stm->setup.printer_expanded);
    color_button =
      XmToggleButtonGetState(stm->setup.printer_color);
    eight_bit_button = 
      XmToggleButtonGetState(stm->setup.printer_8_bit);
    hls_button = 
      XmToggleButtonGetState(stm->setup.printer_hls);
#else
    queued_printer_button = 0;
    printer_port_button = 0;
    printer_file_button = 0;
    port_name = XtNewString ("unused_vxt_printer_port_name");
    file_name = XtNewString ("unused_vxt_printer_file_name");
    background_button = 0;
    level_1_button = 0;
    level_2_button = 0;
    compressed_button = 0;
    expanded_button = 0;
    color_button = 0;
    hls_button = 0;
#endif

    normal_print_button =
      XmToggleButtonGetState(stm->setup.printer_normal_print);
    auto_print_button =
      XmToggleButtonGetState(stm->setup.printer_auto_print);

    full_page_button =
      XmToggleButtonGetState(stm->setup.printer_full_page);
    full_page_transcript_button =
      XmToggleButtonGetState(stm->setup.printer_full_page_transcript);
    scroll_region_button =
      XmToggleButtonGetState(stm->setup.printer_scroll_region);

    national_button =
      XmToggleButtonGetState(stm->setup.printer_national);
    national_line_drawing_button =
      XmToggleButtonGetState(stm->setup.printer_national_line_drawing);

    form_feed_button =
      XmToggleButtonGetState(stm->setup.printer_form_feed);
    printer_to_host_button =
      XmToggleButtonGetState(stm->setup.printer_to_host);
    graphics_button =
      XmToggleButtonGetState(stm->setup.printer_graphics);

    if ( multi_uid || stm->terminalType == DECwKanji ) {
    print_main_button =
      XmToggleButtonGetState(stm->setup.printer_main_display_24);
    print_status_button =
      XmToggleButtonGetState(stm->setup.printer_status_display_25);
    }
    eight_bit_button =
      XmToggleButtonGetState(stm->setup.printer_8_bit);


    /* determine the values of resources controlled by radio boxes */
    if ( queued_printer_button )
	destination = DECwDestinationQueue;
    else if ( printer_port_button )
	destination = DECwDestinationPort;
    else if ( printer_file_button )
	destination = DECwDestinationFile;
    else
	destination = DECwDestinationNone;

    if ( normal_print_button )
	print_mode = DECwNormalPrintMode;
    else if ( auto_print_button )
	print_mode = DECwAutoPrintMode;
    else
	print_mode = DECwPrintControllerMode;

    if ( full_page_button )
	print_extent = DECwFullPage;
    else if ( full_page_transcript_button )
	print_extent = DECwFullPagePlusTranscript;
    else if ( scroll_region_button )
	print_extent = DECwScrollRegionOnly;
    else
	print_extent = DECwSelectionOnly;

    if ( national_button )
	print_data_type = DECwNationalOnly;
    else if ( national_line_drawing_button )
	print_data_type = DECwNationalPlusLineDrawing;
    else
	print_data_type = DECwPrintAllCharacters;

    if ( level_1_button )
	sixel_level = DECwSixelLevel_1;
    else if ( level_2_button )
	sixel_level = DECwSixelLevel_2;
    else
	if ( multi_uid || !IsAsianTerminalType(stm->terminalType) )
	sixel_level = DECwSixelLevel_LA210;

    if ( multi_uid || stm->terminalType == DECwKanji ) {
	if ( print_status_button )	
	   display_mode = DECwStatusDisplay25;
	else if ( print_main_button )
	   display_mode = DECwMainDisplay24;
    }

    if ( compressed_button )
	print_format = DECwCompressedPrinting;
    else if ( expanded_button )
	print_format = DECwExpandedPrinting;
    else
	print_format = DECwRotatedPrinting;

    /* set the resources (even if not changed) */
    n = 0;
    XtSetArg( arglist[n], DECwNprintingDestination, destination ); n++;
    XtSetArg( arglist[n], DECwNprinterPortName, port_name ); n++;
    XtSetArg( arglist[n], DECwNprinterFileName, file_name ); n++;
    XtSetArg( arglist[n], DECwNprintMode, print_mode ); n++;
    XtSetArg( arglist[n], DECwNprintExtent, print_extent ); n++;
    XtSetArg( arglist[n], DECwNprintDataType, print_data_type ); n++;
    XtSetArg( arglist[n], DECwNprintFormFeedMode, form_feed_button ); n++;
    XtSetArg( arglist[n], DECwNprinterToHostEnabled, printer_to_host_button ); n++;
    XtSetArg( arglist[n], DECwNgraphicsPrintingEnabled, graphics_button ); n++;
    XtSetArg( arglist[n], DECwNprintBackgroundMode, background_button ); n++;
    XtSetArg( arglist[n], DECwNprintSixelLevel, sixel_level ); n++;
    XtSetArg( arglist[n], DECwNprintFormat, print_format ); n++;
    XtSetArg( arglist[n], DECwNprintColorMode, color_button ); n++;
    XtSetArg( arglist[n], DECwNprint8BitMode, eight_bit_button ); n++;
    XtSetArg( arglist[n], DECwNprintHLSColorSyntax, hls_button ); n++;

    XtSetValues( stm->terminal, arglist, n );
    XtFree( port_name );
    XtFree( file_name );
}

void queued_printer_options_cb(w, closure, call_data)
    Widget w;
    caddr_t closure, call_data;
{
    STREAM *stm = convert_widget_to_stream(w);

    if ( QUEUED_PRINTER_OPTIONS_PARENT == 0 )
	if ( !create_queued_options_box( stm ))
	    return;		/* just return if we can't fetch the widget */
    XtUnmanageChild( QUEUED_PRINTER_OPTIONS_PARENT );
    XtManageChild( QUEUED_PRINTER_OPTIONS_PARENT );
}

void print_widget_create_cb( w, closure, call_data )
    Widget w;
    caddr_t closure, call_data;
{
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.print_widget_id = w;
}

void print_widget_ok_cb( w, closure, call_data )
    Widget w;
    caddr_t closure, call_data;
{
    STREAM *stm = convert_widget_to_stream(w);

    XtUnmanageChild( QUEUED_PRINTER_OPTIONS_PARENT );
    finish_printing( stm );
}

void print_widget_cancel_cb( w, closure, call_data )
    Widget w;
    caddr_t closure, call_data;
{
    STREAM *stm = convert_widget_to_stream(w);

    XtUnmanageChild( QUEUED_PRINTER_OPTIONS_PARENT );
}

Boolean create_queued_options_box( stm )
    STREAM *stm;
{
    Arg arglist[2];
	/* check if we need to fetch the widgets from disk */
     	if (QUEUED_PRINTER_OPTIONS_PARENT == 0)
            {

            /* fetch dialog box from DRM */
            if (MrmFetchWidget
                   (
                   s_MRMHierarchy,
                   "queued_printer_options_db",
                   stm->parent,
                   & QUEUED_PRINTER_OPTIONS_PARENT,
                   & dummy_class
                   )
               != MrmSUCCESS)
                {
                log_error(
			"Unable to fetch widget definitions from DRM\n");
	        return False;
                }
            }

    XtSetArg(arglist[0], DXmNdeleteFile, True );
    XtSetArg(arglist[1], DXmNsuppressOptionsMask, DXmSUPPRESS_DELETE_FILE );
    XtSetValues(QUEUED_PRINTER_OPTIONS_PARENT, arglist, 2);
    return True;
}

/*
 * These routines are called when each widget is created.
 */

void queued_printer_cb(w, closure, call_data)
    Widget w;
    caddr_t closure, call_data;
{
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.printer_queued_printer = w;
}

void printer_port_cb(w, closure, call_data)
    Widget w;
    caddr_t closure, call_data;
{
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.printer_port = w;
}
void printer_file_cb(w, closure, call_data)
    Widget w;
    caddr_t closure, call_data;
{
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.printer_file = w;
}

void printer_none_cb(w, closure, call_data)
    Widget w;
    caddr_t closure, call_data;
{
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.printer_none = w;
}

void printer_port_text_cb(w, closure, call_data)
    Widget w;
    caddr_t closure, call_data;
{
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.printer_port_name = w;
}
void printer_file_text_cb(w, closure, call_data)
    Widget w;
    caddr_t closure, call_data;
{
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.printer_file_name = w;
}

void normal_print_cb(w, closure, call_data)
    Widget w;
    caddr_t closure, call_data;
{
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.printer_normal_print = w;
}

void auto_print_cb(w, closure, call_data)
    Widget w;
    caddr_t closure, call_data;
{
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.printer_auto_print = w;
}

void printer_controller_cb(w, closure, call_data)
    Widget w;
    caddr_t closure, call_data;
{
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.printer_controller = w;
}

void print_full_page_cb(w, closure, call_data)
    Widget w;
    caddr_t closure, call_data;
{
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.printer_full_page = w;
}

void print_full_page_transcript_cb(w, closure, call_data)
    Widget w;
    caddr_t closure, call_data;
{
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.printer_full_page_transcript = w;
}

void print_scroll_region_cb(w, closure, call_data)
    Widget w;
    caddr_t closure, call_data;
{
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.printer_scroll_region = w;
}

void print_selection_cb(w, closure, call_data)
    Widget w;
    caddr_t closure, call_data;
{
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.printer_selection = w;
}

void print_national_cb(w, closure, call_data)
    Widget w;
    caddr_t closure, call_data;
{
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.printer_national = w;
}

void print_national_line_drawing_cb(w, closure, call_data)
    Widget w;
    caddr_t closure, call_data;
{
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.printer_national_line_drawing = w;
}

void print_all_characters_cb(w, closure, call_data)
    Widget w;
    caddr_t closure, call_data;
{
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.printer_all_characters = w;
}

void form_feed_terminator_cb(w, closure, call_data)
    Widget w;
    caddr_t closure, call_data;
{
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.printer_form_feed = w;
}

void printer_to_host_cb(w, closure, call_data)
    Widget w;
    caddr_t closure, call_data;
{
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.printer_to_host = w;
}

void graphics_printing_cb(w, closure, call_data)
    Widget w;
    caddr_t closure, call_data;
{
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.printer_graphics = w;
}

void background_printing_cb(w, closure, call_data)
    Widget w;
    caddr_t closure, call_data;
{
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.printer_background = w;
}

void level_1_sixel_cb(w, closure, call_data)
    Widget w;
    caddr_t closure, call_data;
{
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.printer_level_1 = w;
}

void level_2_sixel_cb(w, closure, call_data)
    Widget w;
    caddr_t closure, call_data;
{
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.printer_level_2 = w;
}

void la210_sixel_cb(w, closure, call_data)
    Widget w;
    caddr_t closure, call_data;
{
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.printer_la210 = w;
}

void print_main_display_24_cb(w, closure, call_data)
    Widget w;
    caddr_t closure, call_data;
{
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.printer_main_display_24 = w;
}

void print_status_display_25_cb(w, closure, call_data)
    Widget w;
    caddr_t closure, call_data;
{
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.printer_status_display_25 = w;
}

void compressed_printing_cb(w, closure, call_data)
    Widget w;
    caddr_t closure, call_data;
{
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.printer_compressed = w;
}

void expanded_printing_cb(w, closure, call_data)
    Widget w;
    caddr_t closure, call_data;
{
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.printer_expanded = w;
}

void rotated_printing_cb(w, closure, call_data)
    Widget w;
    caddr_t closure, call_data;
{
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.printer_rotated = w;
}

void monochrome_printing_cb(w, closure, call_data)
    Widget w;
    caddr_t closure, call_data;
{
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.printer_monochrome = w;
}

void color_printing_cb(w, closure, call_data)
    Widget w;
    caddr_t closure, call_data;
{
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.printer_color = w;
}

void printer_7bit_cb(w, closure, call_data)
    Widget w;
    caddr_t closure, call_data;
{
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.printer_7_bit = w;
}

void printer_8bit_cb(w, closure, call_data)
    Widget w;
    caddr_t closure, call_data;
{
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.printer_8_bit = w;
}

void hls_syntax_cb(w, closure, call_data)
    Widget w;
    caddr_t closure, call_data;
{
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.printer_hls = w;
}

void rgb_syntax_cb(w, closure, call_data)
    Widget w;
    caddr_t closure, call_data;
{
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.printer_rgb = w;
}

void options_cb(w, closure, call_data)
    Widget w;
    caddr_t closure, call_data;
{
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.options_widget_id = w;
}

void save_options_cb(w, closure, call_data)
    Widget w;
    caddr_t closure, call_data;
{
    STREAM *stm = convert_widget_to_stream(w);

    stm->setup.save_options_id = w;
}

/*
 * Register all the symbols DRM needs to bind for us
 *
 * This routine is called once when the DECterm controller
 * starts up (from module tea.c).  It's more convenient to
 * put it here in the same module with the callback routines
 * it references.
 */      
void tea_register_callbacks()
{
    /*
     * Names and address of things DRM needs to bind.
     * Note: keep in lexicographical order so DRM
     * can do quicker lookups.
     */
    static
    MrmRegisterArg  namevec[] =       /* names known to UIL */
        {
#ifdef SECURE_KEYBOARD
	{"allow_quickcopy_cb",		(caddr_t) allow_quickcopy_cb},
#endif
	{"angle_angle_cb",		(caddr_t) angle_angle_cb},
	{"angle_tilde_cb",		(caddr_t) angle_tilde_cb},
	{"appl_cursor_keys_cb",		(caddr_t) appl_cursor_keys_cb},
	{"appl_keypad_cb",		(caddr_t) appl_keypad_cb},
	{"ascii_mode_cb",		(caddr_t) ascii_mode_cb},
	{"auto_print_cb",		(caddr_t) auto_print_cb},
	{"auto_repeat_cb",		(caddr_t) auto_repeat_cb},
	{"auto_resize_terminal_cb",	(caddr_t) auto_resize_terminal_cb},
	{"auto_resize_window_cb",	(caddr_t) auto_resize_window_cb},
	{"auto_wrap_cb",		(caddr_t) auto_wrap_cb},
	{"backarrow_BS_cb",		(caddr_t) backarrow_BS_cb},
	{"backarrow_DEL_cb",		(caddr_t) backarrow_DEL_cb},
	{"background_printing_cb",	(caddr_t) background_printing_cb},
	{"batch_scroll_cb",		(caddr_t) batch_scroll_cb},
	{"big_font_default_create_cb", 	(caddr_t) big_font_default_create_cb},
	{"big_font_name_create_cb", 	(caddr_t) big_font_name_create_cb},
	{"big_font_name_focus_cb", 	(caddr_t) big_font_name_focus_cb},
	{"big_font_other_create_cb", 	(caddr_t) big_font_other_create_cb},
	{"big_font_other_arm_cb", 	(caddr_t) big_font_other_arm_cb},
	{"little_font_default_create_cb", (caddr_t) little_font_default_create_cb},
	{"little_font_name_create_cb", 	(caddr_t) little_font_name_create_cb},
	{"little_font_name_focus_cb", 	(caddr_t) little_font_name_focus_cb},
	{"little_font_other_create_cb",	(caddr_t) little_font_other_create_cb},
	{"little_font_other_arm_cb", 	(caddr_t) little_font_other_arm_cb},
	{"gs_font_default_create_cb", 	(caddr_t) gs_font_default_create_cb},
	{"gs_font_name_create_cb", 	(caddr_t) gs_font_name_create_cb},
	{"gs_font_name_focus_cb", 	(caddr_t) gs_font_name_focus_cb},
	{"gs_font_other_create_cb", 	(caddr_t) gs_font_other_create_cb},
	{"gs_font_other_arm_cb", 	(caddr_t) gs_font_other_arm_cb},
	{"big_font_set_cb",		(caddr_t) big_font_set_cb},
	{"bit_planes_text_cb",		(caddr_t) bit_planes_text_cb},
	{"block_cursor_cb",		(caddr_t) block_cursor_cb},
	{"cancel_printing_cb",		(caddr_t) cancel_printing_cb},
	{"color_printing_cb",		(caddr_t) color_printing_cb},
	{"columns_132_cb",		(caddr_t) columns_132_cb},
	{"columns_132_create_cb",	(caddr_t) columns_132_create_cb},
	{"columns_80_cb",		(caddr_t) columns_80_cb},
	{"columns_text_cb",		(caddr_t) columns_text_cb},
	{"comma_angle_cb",		(caddr_t) comma_angle_cb},
	{"comma_comma_cb",		(caddr_t) comma_comma_cb},
	{"commands_clearcomm_cb",	(caddr_t) commands_clearcomm_cb},
	{"commands_cleardisplay_cb",	(caddr_t) commands_cleardisplay_cb},
	{"commands_clearlinesofftop_cb",(caddr_t) commands_clearlinesofftop_cb},
	{"commands_convertdisplay_act_cb", (caddr_t) commands_convertdisplay_act_cb},
	{"commands_convertdisplay_cre_cb", (caddr_t) commands_convertdisplay_cre_cb},
#ifdef SECURE_KEYBOARD
	{"commands_map_cb",		(caddr_t) commands_map_cb},
#endif
	{"commands_map2_cb",		(caddr_t) commands_map2_cb},
	{"commands_resetterminal_cb",	(caddr_t) commands_resetterminal_cb},
	{"commands_resizewindow_cb",	(caddr_t) commands_resizewindow_cb},
#ifdef SECURE_KEYBOARD
	{"commands_secure_cb",		(caddr_t) commands_secure_cb},
#endif
	{"compressed_printing_cb",	(caddr_t) compressed_printing_cb},
	{"condensed_font_cb",		(caddr_t) condensed_font_cb},
	{"control_QS_hold_cb",		(caddr_t) control_QS_hold_cb},
	{"copyright_callback",		(caddr_t) copyright_callback},
	{"copy_dir_cb",			(caddr_t) copy_dir_cb},
	{"copy_dir_cre_cb",		(caddr_t) copy_dir_cre_cb},
	{"copy_dir_ltor_cre_cb",	(caddr_t) copy_dir_ltor_cre_cb},
	{"copy_dir_ltor_act_cb",	(caddr_t) copy_dir_ltor_act_cb},
	{"copy_dir_rtol_cre_cb",	(caddr_t) copy_dir_rtol_cre_cb},
	{"copy_dir_rtol_act_cb",	(caddr_t) copy_dir_rtol_act_cb},
	{"coupling_horizontal_cb",	(caddr_t) coupling_horizontal_cb},
	{"coupling_vertical_cb",	(caddr_t) coupling_vertical_cb},
	{"cs_help_cb",			(caddr_t) cs_help_cb},
	{"cursor_blink_cb",		(caddr_t) cursor_blink_cb},
	{"cursor_ltor_cb",		(caddr_t) cursor_ltor_cb},
	{"cursor_rtol_cb",		(caddr_t) cursor_rtol_cb},
	{"dark_text_cb",		(caddr_t) dark_text_cb},
	{"decterm_id_cb",		(caddr_t) decterm_id_cb},
	{"dialect_apply_cb",		(caddr_t) dialect_apply_cb},
	{"dialect_cancel_cb",		(caddr_t) dialect_cancel_cb},
	{"dialect_ok_cb",		(caddr_t) dialect_ok_cb},
	{"dialect_create_cb",		(caddr_t) dialect_create_cb},
	{"display_apply_cb",		(caddr_t) display_apply_cb},
	{"display_cancel_cb",		(caddr_t) display_cancel_cb},
	{"display_controls_cb",		(caddr_t) display_controls_cb},
	{"display_cursor_cb",		(caddr_t) display_cursor_cb},
	{"display_leading_code_cb",	(caddr_t) display_leading_code_cb},
	{"display_ok_cb",		(caddr_t) display_ok_cb},
	{"edit_copy_cb",		(caddr_t) edit_copy_cb},
	{"edit_paste_cb",		(caddr_t) edit_paste_cb},
	{"edit_selectall_cb",		(caddr_t) edit_selectall_cb},
	{"eight_bit_cb",		(caddr_t) eight_bit_cb},
	{"enable_backing_store_cb",	(caddr_t) enable_backing_store_cb},
#if !defined(VXT_DECTERM)
	{"f11_escape_cb",               (caddr_t) f11_escape_cb},
	{"f11_f11_cb",                  (caddr_t) f11_f11_cb},
#endif
	{"expanded_printing_cb",	(caddr_t) expanded_printing_cb},
	{"features_locked_cb",		(caddr_t) features_locked_cb},
	{"local_echo_cb",		(caddr_t) local_echo_cb},
	{"conceal_answerback_cb",	(caddr_t) conceal_answerback_cb},
	{"answerback_message_cb",	(caddr_t) answerback_message_cb},
	{"change_conceal_answerback_cb",(caddr_t) change_conceal_answerback_cb},
	{"toggle_conceal_answerback_cb",(caddr_t) toggle_conceal_answerback_cb},
	{"file_exit_cb",		(caddr_t) file_exit_cb},
	{"file_new_cb",			(caddr_t) file_new_cb},
	{"file_open_cb",		(caddr_t) file_open_cb},
	{"file_open_fs_cb",		(caddr_t) file_open_fs_cb},
	{"file_revert_cb",		(caddr_t) file_revert_cb},
	{"file_save_cb",		(caddr_t) file_save_cb},
	{"file_saveas_cb",		(caddr_t) file_saveas_cb},
	{"file_saveas_fs_cb",		(caddr_t) file_saveas_fs_cb},
	{"fine_font_set_cb",		(caddr_t) fine_font_set_cb},
	{"finish_printing_cb",		(caddr_t) finish_printing_cb},
	{"form_feed_terminator_cb",	(caddr_t) form_feed_terminator_cb},
	{"general_apply_cb",		(caddr_t) general_apply_cb},
	{"general_cancel_cb",		(caddr_t) general_cancel_cb},
	{"general_ok_cb",		(caddr_t) general_ok_cb},
	{"graphics_apply_cb",		(caddr_t) graphics_apply_cb},
	{"graphics_cancel_cb",		(caddr_t) graphics_cancel_cb},
	{"graphics_ok_cb",		(caddr_t) graphics_ok_cb},
	{"graphics_printing_cb",	(caddr_t) graphics_printing_cb},
	{"gs_font_set_cb",		(caddr_t) gs_font_set_cb},
	{"help_about_cb",		(caddr_t) help_about_cb},
	{"help_overview_cb",		(caddr_t) help_overview_cb},
	{"help_show_version_cb",	(caddr_t) help_show_version_cb},
	{"hls_syntax_cb",		(caddr_t) hls_syntax_cb},
	{"host_status_display_cb",	(caddr_t) host_status_display_cb},
	{"icon_name_cb",		(caddr_t) icon_name_cb},
	{"interpret_controls_cb",	(caddr_t) interpret_controls_cb},
	{"jisroman_mode_cb",		(caddr_t) jisroman_mode_cb},
	{"kanji_mode_cb",		(caddr_t) kanji_mode_cb},
	{"katakana_mode_cb",		(caddr_t) katakana_mode_cb},
	{"kanji_78_cb",			(caddr_t) kanji_78_cb},
	{"kanji_83_cb",			(caddr_t) kanji_83_cb},
	{"keyboard_apply_cb",		(caddr_t) keyboard_apply_cb},
	{"keyboard_cancel_cb",		(caddr_t) keyboard_cancel_cb},
	{"keyboard_ok_cb",		(caddr_t) keyboard_ok_cb},
	{"ksascii_mode_cb",		(caddr_t) ksascii_mode_cb},
	{"ksroman_mode_cb",		(caddr_t) ksroman_mode_cb},
	{"la210_sixel_cb",		(caddr_t) la210_sixel_cb},
	{"level_1_sixel_cb",		(caddr_t) level_1_sixel_cb},
	{"level_2_sixel_cb",		(caddr_t) level_2_sixel_cb},
	{"light_text_cb",		(caddr_t) light_text_cb},
	{"little_font_set_cb",		(caddr_t) little_font_set_cb},
	{"macrograph_report_cb",	(caddr_t) macrograph_report_cb},
	{"margin_bell_cb",		(caddr_t) margin_bell_cb},
	{"monochrome_printing_cb",	(caddr_t) monochrome_printing_cb},
	{"newline_cb",			(caddr_t) newline_cb},
	{"no_status_display_cb",	(caddr_t) no_status_display_cb},
	{"normal_cursor_keys_cb",	(caddr_t) normal_cursor_keys_cb},
	{"normal_font_cb",		(caddr_t) normal_font_cb},
	{"normal_print_cb",		(caddr_t) normal_print_cb},
	{"numeric_keypad_cb",		(caddr_t) numeric_keypad_cb},
	{"on_context_cb",    		(caddr_t) on_context_cb},
	{"on_help_cb",    		(caddr_t) on_help_cb},
        {"options_cb",                  (caddr_t) options_cb},
	{"printer_7bit_cb",		(caddr_t) printer_7bit_cb},
	{"printer_8bit_cb",		(caddr_t) printer_8bit_cb},
	{"print_all_cb",		(caddr_t) print_all_cb},
	{"print_all_characters_cb",	(caddr_t) print_all_characters_cb},
	{"print_full_page_cb",		(caddr_t) print_full_page_cb},
	{"print_full_page_transcript_cb", (caddr_t)
					     print_full_page_transcript_cb},
	{"print_graphics_cb",		(caddr_t) print_graphics_cb},
	{"print_main_display_24_cb",	(caddr_t) print_main_display_24_cb},
	{"print_national_cb",		(caddr_t) print_national_cb},
	{"print_national_line_drawing_cb",  (caddr_t)
					      print_national_line_drawing_cb},
	{"print_page_cb",		(caddr_t) print_page_cb},
	{"print_scroll_region_cb",	(caddr_t) print_scroll_region_cb},
	{"print_selected_cb",		(caddr_t) print_selected_cb},
	{"print_selection_cb",		(caddr_t) print_selection_cb},
	{"print_status_display_25_cb",	(caddr_t) print_status_display_25_cb},
	{"print_widget_cancel_cb",	(caddr_t) print_widget_cancel_cb},
	{"print_widget_create_cb",      (caddr_t) print_widget_create_cb},
	{"print_widget_ok_cb",		(caddr_t) print_widget_ok_cb},
	{"printer_apply_cb",		(caddr_t) printer_apply_cb},
	{"printer_cancel_cb",		(caddr_t) printer_cancel_cb},
	{"printer_controller_cb",	(caddr_t) printer_controller_cb},
	{"printer_file_cb",		(caddr_t) printer_file_cb},
	{"printer_file_text_cb",	(caddr_t) printer_file_text_cb},
	{"printer_none_cb",		(caddr_t) printer_none_cb},
	{"printer_ok_cb",		(caddr_t) printer_ok_cb},
	{"printer_port_cb",		(caddr_t) printer_port_cb},
	{"printer_port_text_cb",	(caddr_t) printer_port_text_cb},
	{"printer_to_host_cb",		(caddr_t) printer_to_host_cb},
	{"queued_printer_cb",		(caddr_t) queued_printer_cb},
	{"queued_printer_options_cb",	(caddr_t) queued_printer_options_cb},
	{"regis_screen_mode_cb",	(caddr_t) regis_screen_mode_cb},
	{"rgb_syntax_cb",		(caddr_t) rgb_syntax_cb},
	{"rotated_printing_cb",		(caddr_t) rotated_printing_cb},
	{"rows_24_cb",			(caddr_t) rows_24_cb},
	{"rows_48_cb",			(caddr_t) rows_48_cb},
	{"rows_48_create_cb",		(caddr_t) rows_48_create_cb},
	{"rows_72_cb",			(caddr_t) rows_72_cb},
	{"rows_72_create_cb",		(caddr_t) rows_72_create_cb},
	{"rows_text_cb",		(caddr_t) rows_text_cb},
	{"save_lines_off_top_cb",	(caddr_t) save_lines_off_top_cb},
	{"scroll_horizontal_cb",	(caddr_t) scroll_horizontal_cb},
	{"scroll_vertical_cb",		(caddr_t) scroll_vertical_cb},
	{"is_printer_supported_cb",	(caddr_t) is_printer_supported_cb},
        {"save_options_cb",             (caddr_t) save_options_cb},
	{"setup_display_cb",		(caddr_t) setup_display_cb},
	{"setup_general_cb",		(caddr_t) setup_general_cb},
	{"setup_graphics_cb",		(caddr_t) setup_graphics_cb},
	{"setup_keyboard_cb",		(caddr_t) setup_keyboard_cb},
	{"setup_keyboarddialect_cb",	(caddr_t) setup_keyboarddialect_cb},
	{"setup_printer_cb",		(caddr_t) setup_printer_cb},
	{"setup_tabs_cb",		(caddr_t) setup_tabs_cb},
	{"setup_terminaltype_cb",	(caddr_t) setup_terminaltype_cb},
	{"setup_window_cb",		(caddr_t) setup_window_cb},
	{"seven_bit_cb",		(caddr_t) seven_bit_cb},
	{"share_colormap_entries_cb",	(caddr_t) share_colormap_entries_cb},
	{"show_version_acknowledge_cb",	(caddr_t) show_version_acknowledge_cb},
	{"show_version_label_cb",	(caddr_t) show_version_label_cb},
	{"terminaltype_apply_cb",	(caddr_t) terminaltype_apply_cb},
	{"terminaltype_cancel_cb",	(caddr_t) terminaltype_cancel_cb},
	{"terminaltype_hangul_cb",	(caddr_t) terminaltype_hangul_cb},
	{"terminaltype_hanyu_cb",	(caddr_t) terminaltype_hanyu_cb},
	{"terminaltype_hanzi_cb",	(caddr_t) terminaltype_hanzi_cb},
	{"terminaltype_hebrew_cb",	(caddr_t) terminaltype_hebrew_cb},
	{"terminaltype_kanji_cb",	(caddr_t) terminaltype_kanji_cb},
	{"terminaltype_ok_cb",		(caddr_t) terminaltype_ok_cb},
	{"terminaltype_standard_cb",	(caddr_t) terminaltype_standard_cb},
	{"terminal_driver_resize_cb",	(caddr_t) terminal_driver_resize_cb},
	{"tilde_escape_cb",		(caddr_t) tilde_escape_cb},
	{"tilde_tilde_cb",		(caddr_t) tilde_tilde_cb},
	{"transcript_size_cb",		(caddr_t) transcript_size_cb},
	{"udk_locked_cb",		(caddr_t) udk_locked_cb},
	{"underline_cursor_cb",		(caddr_t) underline_cursor_cb},
	{"upss_radio_create_cb",	(caddr_t) upss_radio_create_cb},
	{"upss_dec_cb",			(caddr_t) upss_dec_cb},
	{"upss_dec_turkish_cb",		(caddr_t) upss_dec_turkish_cb},
	{"upss_iso_turkish_cb",		(caddr_t) upss_iso_turkish_cb},
	{"upss_dec_greek_cb",		(caddr_t) upss_dec_greek_cb},
	{"upss_iso_greek_cb",		(caddr_t) upss_iso_greek_cb},
	{"upss_dec_heb_cb",		(caddr_t) upss_dec_heb_cb},
	{"upss_dec_heb_cb",		(caddr_t) upss_dec_heb_cb},
	{"upss_iso_cb",			(caddr_t) upss_iso_cb},
	{"upss_iso_heb_cb",		(caddr_t) upss_iso_heb_cb},
	{"variable_font_cb",		(caddr_t) variable_font_cb},
	{"vt100j_id_cb",		(caddr_t) vt100j_id_cb},
	{"vt100_id_cb",			(caddr_t) vt100_id_cb},
	{"vt100_mode_cb",		(caddr_t) vt100_mode_cb},
	{"vt101_id_cb",			(caddr_t) vt101_id_cb},
	{"vt102j_id_cb",		(caddr_t) vt102j_id_cb},
	{"vt102_id_cb",			(caddr_t) vt102_id_cb},
	{"vt125_id_cb",			(caddr_t) vt125_id_cb},
	{"vt220j_id_cb",		(caddr_t) vt220j_id_cb},
	{"vt220_id_cb",			(caddr_t) vt220_id_cb},
	{"vt240_id_cb",			(caddr_t) vt240_id_cb},
	{"vt282_id_cb",			(caddr_t) vt282_id_cb},
	{"vt284_id_cb",			(caddr_t) vt284_id_cb},
	{"vt286_id_cb",			(caddr_t) vt286_id_cb},
	{"vt300_7bitc_mode_cb",		(caddr_t) vt300_7bitc_mode_cb},
	{"vt300_8bitc_mode_cb",		(caddr_t) vt300_8bitc_mode_cb},
	{"vt320_id_cb",			(caddr_t) vt320_id_cb},
	{"vt330_id_cb",			(caddr_t) vt330_id_cb},
	{"vt340_id_cb",			(caddr_t) vt340_id_cb},
	{"vt382_id_cb",			(caddr_t) vt382_id_cb},
	{"vt382cb_id_cb",		(caddr_t) vt382cb_id_cb},
	{"vt382k_id_cb",		(caddr_t) vt382k_id_cb},
	{"vt382d_id_cb",		(caddr_t) vt382d_id_cb},
	{"vt52_mode_cb",		(caddr_t) vt52_mode_cb},
	{"vt80_id_cb",			(caddr_t) vt80_id_cb},
	{"warn_window_cb",		(caddr_t) warn_window_cb},
	{"warning_bell_cb",		(caddr_t) warning_bell_cb},
	{"window_apply_cb",		(caddr_t) window_apply_cb},
	{"window_cancel_cb",		(caddr_t) window_cancel_cb},
	{"window_ok_cb",		(caddr_t) window_ok_cb},
	{"window_title_cb",		(caddr_t) window_title_cb},
#ifdef HYPERHELP
	{"help_system_proc",		(caddr_t) help_system_proc},
	{"help_activate_proc",		(caddr_t) help_activate_proc},
#endif /* HYPERHELP */
	{"file_open_create_cb",		(caddr_t) file_open_create_cb},
	{"file_saveas_create_cb",	(caddr_t) file_saveas_create_cb},
        };

 
    /* Register the stuff DRM needs to bind for us */
    MrmRegisterNames
        (
        namevec,                          /* UIL Symbol list */
	XtNumber(namevec)
        );

}

Widget
find_top_level_widget( w )
    Widget w;
{
    Widget parent;

    parent = w;

    while ( XtParent(parent) != NULL )
	parent = XtParent(parent);
    return (parent);
}

#ifdef HYPERHELP
/*HyperHelp - Help system callback.  Create a help system session
 */
static void help_system_proc( widget, tag, reason )
Widget		    widget;
int		    *tag;
XmAnyCallbackStruct *reason;
{
    STREAM *stm;
    Widget w = find_top_level_widget(widget);

	stm = convert_widget_to_stream (w);

	if ( stm->help_context == NULL )
	{
	    /* Open a HyperHelp context for this stream.
	     * Open it *only* after user invoke help from DECterm.
	     */
	    DXmHelpSystemOpen( &stm->help_context,
			       stm->parent,
			       DECTERM_APPL_CLASS,
			       help_error,
			       "Help System Error" );
	}

	DXmHelpSystemDisplay( stm->help_context, DECTERM_APPL_CLASS, "topic",
			      tag, help_error, "Help System Error" );
}

static void help_activate_proc( widget, tag, reason )
Widget	             widget;
int		    *tag;
XmAnyCallbackStruct *reason;
{
    STREAM *stm;
    Widget w = find_top_level_widget(widget);
    int    help_index = *tag;

	stm = convert_widget_to_stream (w);

	if ( help_index == k_help_on_version &&
	     reason->event->xbutton.state & ShiftMask )
	{
	    help_show_version_cb( widget, tag, reason );
	    return;
	}

	if ( stm->help_context == NULL )
	{
	    /* Open a HyperHelp context for this stream.
	     * Open it *only* after user invoke help from DECterm.
	     */
	    DXmHelpSystemOpen( &stm->help_context,
			       stm->parent,
			       DECTERM_APPL_CLASS,
			       help_error,
			       "Help System Error" );
	}

	switch ( help_index )
	{
	    case k_help_on_context :
		DXmHelpOnContext(w, FALSE);

		break;

	    case k_help_on_window :
		/* "decterm" is the index for "OnWindow" help topic
		 */
		DXmHelpSystemDisplay( stm->help_context, DECTERM_APPL_CLASS,
				      "topic", "decterm", help_error,
				      "Help System Error" );
		break;

	    case k_help_on_help :
		/* "onhelp" is the index for "OnHelp" help topic
		 */
		DXmHelpSystemDisplay( stm->help_context, DECTERM_APPL_CLASS,
				      "topic", "onhelp", help_error,
				      "Help System Error" );
		break;

	    case k_help_on_version :
		/* "about" is the index for the "OnVersion" help topic
		 */
		DXmHelpSystemDisplay( stm->help_context, DECTERM_APPL_CLASS,
				      "topic", "about", help_error,
				      "Help System Error" );
		break;
	}

}

/*HyperHelp error handling routine
 */
void help_error( problem_string, status )
char	*problem_string;
int	status;
{
	log_message( "%s, status was %x\n", problem_string, status );
}

#endif /* HYPERHELP */


/*
 * The following is copied from str_func.c in vxt sources.  I really wish
 * these weren't used, since whenever non-standard C functions are called
 * from decterm, it's a headache porting it to other platforms on which
 * the non-standard function (StrEdit in this case) doesn't exist.
 *
 * For this case, we copy StrEdit here, and hope no place else in decterm
 * uses it. (If it does, we'll have to move StrEdit into its own module, and
 * maybe change its name so that other vxt uses don't clash in name space on
 * the link).
 */

static int isblank(ptr)
char *ptr;
    {
    for (; (*ptr == ' ') || (*ptr == '\t'); ptr++);
    if (*ptr != '\0') return (0);
    return (1);
    }

static int StrEdit(out,in,option)
char *out,*in;
int option;
    {
    register char c, *out_orig;
    int len;

    for (len = 0, out_orig = out; *in != '\0'; in++)
	{
    	c = *in;
	if ((c == ' ') || (c == '\t'))
	    {
	    if (option & STR$_COLLAPSE) continue;
	    if ((option & STR$_LEADING) && (out == out_orig)) continue;
	    if (option & STR$_COMPRESS)
		{	/* If previous char is space */
		if ((out != out_orig) && (*(out-1) == ' ')) continue;
		c = ' ';
		}
	    }
	else if (option & STR$_UPPER)
	    c = toupper (c);

	*out++ = c;
	len++;
	}

    *out = '\0';
    if (option & STR$_TRAILING)
	for (--out; len > 0;)
	    {
	    if ((*out  != ' ') && (*out != '\t')) break;
	    *out-- = '\0';
	    --len;
	    }

    return (len);
    }
