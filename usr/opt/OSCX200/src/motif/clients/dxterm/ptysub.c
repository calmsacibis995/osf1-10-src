/* #module ptysub.c "T3.0-EFT2" */
/*
 *  Title:	ptysub.c
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © 1988, 1993 Digital Equipment Corporation.  All Rights      |
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
 *  Module Abstract:
 *
 *	Routines that manage streams and talk to the PTY's.
 *
 *  Procedures contained in this module:
 *
 *	<list of procedure names and abstracts>
 *
 *  Author:	<original author>
 *
 * Modified by:
 *
 * Alfred von Campe     15-Dec-1993     BL-E
 *	- Change killpg() to kill() due to different getpgrp() semantics on
 *        ULTRIX and DEC OSF/1.
 *
 * Alfred von Campe     07-Dec-1993     BL-E
 *	- Remove all focus problem workarounds now that DECterm subclasses
 *	  off of XmManager.
 *
 * Alfred von Campe     20-Nov-1993     BL-E
 *	- Make sure child processes are notified when dxterm goes away.
 *
 * Alfred von Campe     08-Nov-1993     BL-E
 *	- Add Bob Messenger's fix for V1.2 focus problems.
 *
 * Alfred von Campe     25-Oct-1993     BL-E
 *	- Change turkish language designator to tr_TR.
 *
 * Alfred von Campe     19-Oct-1993     BL-E
 *	- Set the input focus to the DECterm widget on WM_TAKE_FOCUS protocol
 *        message.  This fixes the focus problems on Motif V1.2.
 *
 * Eric Osman		18-Oct-1993	BL-E
 *      - Fix -xrm "*.iconName:foo" by only defaulting icon name if user didn't
 *	  give a custom one.
 *
 * Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 * Alfred von Campe     08-Jun-1993     DECterm/BL-C
 *	- Don't set the icon pixmap unless the icon size has changed.
 *      - Add flag word to create_session() and p_new_terminal().
 *
 * Eric Osman		 4-May-1993	VXT V2.0
 *
 *	- Make copy of transport name to fix BREAK key problem.
 *
 * Aston Chan		23-Apr-1993	V1.2/BL2
 *	Startup/Fullname support.  Hebrew DECterm problem.
 *	- Define TEA_display to globalref to get rid of the open_display()
 *	  routine.
 *	- Fix Hebrew problem of the title "DECterm nn" where "nn" is not
 *	  reversed as right to left string.  DECW_SSB QAR 846.
 *
 * Aston Chan		12-Mar-1993	V1.2/BL1
 *	- Add Turkish/Greek Support.
 *
 * DAM			10-Nov-1992	VXT V1.2?
 *	- DECterm tests the options (graying out customizations) itself.
 *
 * Alfred von Campe     08-Oct-1992     Ag/BL10
 *      - Added typecasts to satisfy Alpha compiler.
 *
 * Eric Osman		 5-Oct-1992	VXT V1.2
 *	- Free concealed_ans cell.
 *
 * Aston Chan		11-Sep-1992	Alpha SSB
 *	- Fix show stopper of Alpha SSB.  We can't use /SYSEXE option anymore
 *	  on latest Alpha release which means TTY$GL_DEFCHAR and
 *	  TTY$GL_DEFCHAR2 won't get resolved.  Fix is to use SYS$GETDVIW()
 *	  to get the default characteristics.  Call p_get_defchar() in
 *	  pty_init().
 *
 * Eric Osman		11-Sep-1992	VXT V1.2
 *	- On virtual vxt, leave controller up even after last decterm has
 *	  gone away.
 *	- Remove count_decterms routine, since streams_active is suffish.
 *
 * Eric Osman		3-Sep-1992	VXT V1.2
 *	- Add count_decterms routine
 *
 * Aston Chan		20-Aug-1992	post V1.1
 *	- ToLung's fix of making 2-byte title name in DECW$TERMINAL_DEFAULT.DAT
 *	  work correctly.
 *	- ToLung's fix to make terminal type switchable on DECterm widget
 *	  based on xnllanguage in Japanese TPU.
 *
 * Eric Osman		20-Aug-1992	VXT V1.2
 *	- Initialize printer.graphics_delay to NULL for vxt only
 *
 * Eric Osman		11-June-1992	Sun
 *	- Some casting changes for Sun compiler
 *
 * Alfred von Campe     26-May-1992     Post V3.1
 *      - Fix the way the correct icon size is chosen.
 *
 * Alfred von Campe     02-Apr-1992     Ag/BL6.2.1
 *      - Change XrmFreeDatabase() to the supported XrmDestroyDatabase().
 *
 * Alfred von Campe     24-Feb-1992     V3.1
 *      - Added setIconPixmap routine to handle Reparent events.
 *      - Use XmNiconPixmap instead of XtNiconPixmap.
 *      - Removed SetWMHints() on VMS, since it introduced a bug where the
 *        window would be resized incorrectly when doing a SET/TERM/WIDTH.
 *
 * Eric Osman		19-Dec-1991	V3.1
 *	- release memory after using MrmFetchLiteral
 *	- Use XmNwidth instead of XtNwidth in copyright code
 *
 * Aston Chan		18-Dec-1991	V3.1
 *	- Inside do_copyright_notice(), title_cs has to be freed after use.
 *      - Also #ifdef 0 removed.
 *
 * Aston Chan		17-Dec-1991	V3.1
 *	- I18n code merge
 *
 * Alfred von Campe     02-Dec-1991     V3.1
 *      - Added SetWMHints() routine.
 *
 * Aston Chan		18-Nov-1991	V3.1
 *	- DXmHelpSystemOpen() is only called when user invoke help (in
 *	  MENU_STUBS.C
 *
 * Aston Chan		13-Nov-1991	V3.1
 *	- Add HyperHelp support. Adding help_context to stream.
 *
 * Eric Osman		 7-Oct-1991	V3.1
 *	- If critical quotas are low when a new decterm is requested, put up a
 *	  warning box instead of losing all the rest of the decterms.
 *
 * Alfred von Campe      6-Oct-1991     Hercules/1 T0.7
 *      - Changed #include "protocols.h" to "Protocols.h".
 *      - Added #extern definition for XmCreateMainWindow().
 *
 * Aston Chan		 1-Sep-1991	Alpha
 *	- Add globaldef for streams which will be referenced by other modules.
 *	  It is required by DECC.
 *
 * Alfred von Campe     19-Jun-1991     V3.0
 *      - Clean up icon support code and make the small icon the default.
 *      - Avoid setting the terminal size to 0 rows (fixes SET TERM/PAGE=0 QAR).
 *
 * Alfred von Campe     20-May-1991     V3.0
 *      - Add support for all three icon sizes.
 *      - Start first DECterm at 30,50 instead of 0,0 (it looks nicer this way).
 *
 * Bob Messenger	15-Apr-1991	T3.0-EFT2
 *	- Move wm_protocol event handling from dt_input.c to ptysub.c, to avoid
 *	  link errors.
 *
 * Jim Bay   		13-Apr-1991	T3.0-EFT2
 *	- Incorporated new bitmap for now icon style (courtesy Bob Messenger)
 *
 * Eric Osman		18-Jan-1991
 *	- More leaks.  Free up strings fetch by MrmFetchLiteral.
 *
 * Eric Osman		14-Jan-1991
 *	- Plug small memory leak.  Free up copyright notice when drying stream.
 *
 *  Michele Lien    24-Aug-1990 VXT X0.0
 *  - Modify this module to work on VXT platform. Change #ifdef VMS to
 *    #ifdef VMS_DECTERM so that VXT specific code can be compiled under
 *    VMS or ULTRIX development environment.
 *
 * Bob Messenger	25-Jul-1990	X3.0-5
 *	- Pass the rdb to PutOneResource by reference instead of by value.
 *
 * Bob Messenger	19-Jul-1990	X3.0-5
 *	- Write the default values of resources whose default values are
 *	  computed at run-time into the default resource database after
 *	  the DECterm widget has been realized. (This is to support 100 dpi
 *	  18 point fonts; DECterm decides whether to use an 18 or 14 point
 *	  big font on 100 dpi systems depending on whether it's a 15 or 19
 *	  inch monitor.)
 *
 * Bob Messenger	30-Jun-1990	X3.0-5
 *	- Add printer port support.
 *		- Initialize callbacks.
 *
 * Mark Woodbury	25_may-1990 X3.0-3M
 *	- Motif update
 *
 * Bob Messenger	 2-May-1990	X3.0-2
 *	- Change DECW$K_MSG_EXIT to DECW$K_MSG_EXIT_NO_FONT, so applications
 *	  using the DECterm widget won't have to link with the DECterm
 *	  message file.
 *
 * Bob Messenger	24-Apr-1990	X3.0-2
 *	- Support DWT$K_MSG_EXIT.
 *
 * Bob Messenger	 8-Apr-1990	X3.0-2
 *	- Merge UWS and VMS changes.
 *
 * Mark Granoff		30-Mar-1990	X3.0-1 (VMS DW V3 BL1)
 *	- Merged the last two mods (19-Mar and 20-Feb) into mainline.
 *
 * Bob Messenger	19-Mar-1990	V2.1 (VMS V5.4)
 *	- Call XFreeFontInfo instead of XFreeFont on title font (fixes problem
 *	  where a percentage of DECterm windows aren't started from the
 *	  session manager on Ultrix).  (Change made on Ultrix 18-Sep-1990).
 *
 * Mark Granoff		20-Feb-1990	X3.0-1 (never released)
 *	- Increase MAX_PTY_INPUT from 8 to 256.
 *
 * Bob Messenger	08-Oct-1989	V2.0 (UWS V2.2)
 *	- Don't write informational error messages on non-VMS systems.
 *
 * Bob Messenger	18-Sep-1989	V2.0 (UWS V2.2)
 *	- Call XFreeFontInfo instead of XFreeFont on title font (fixes problem
 *	  where a percentage of DECterm windows aren't started from the
 *	  session manager).
 *
 * Bob Messenger	17-Aug-1989	X2.0-19
 *	- Don't XQueryFont on title font if the titleFont resource comes
 *	  back as NULL.
 *	- Use new icons.
 *
 * Bob Messenger	15-Aug-1989	X2.0-19
 *	- Fix up the code that decides which copyright notice to use, and
 *	  don't print an error message if the title font can't be queried.
 *	- If the title is changed by an OSC, also change the internal
 *	  copy of the default title, to be restored after the first input.
 *	- Make sure window is on screen when created.
 *
 * Bob Messenger	18-Jul-1989	X2.0-13
 *	- Make default title be just "DECterm" on Ultrix.
 *
 * Bob Messenger	30-May-1989	X2.0-13
 *	- Change fprintf's to log_message, so the messages get flushed.
 *	- p_new_terminal returns a VMS condition code on VMS or TRUE for
 *	  success and FALSE for failure on Ultrix, so test the bottom bit
 *	  of the return value.
 *	- Use hex for displaying VMS condition codes.
 *	- Use memset instead of bzero.
 *
 * Bob Messenger	17-May-1989	X2.0-11
 *	- Avoid unnecessary printfs unless verbose_flag is set.
 *
 * Bob Messenger	13-May-1989	X2.0-10
 *	- Read the application title and copyright notices from the UID
 *	  file, so they are translatable.
 *	- Add callback routine for errorCallback, so error messages from
 *	  the widget can be written to log file instead of written to the
 *	  WSAn: device on VMS (or to the caller's terminal for other
 *	  applications that use the widget).
 *	- Convert all printf calls to fprintf on stderr.
 *
 * Bob Messenger	30-Apr-1989	X2.0-9
 *	- Add terminal characteristics parameter to create_session.
 *
 * Bob Messenger	28-Apr-1989	X2.0-8
 *	- Fix the icon pixmap in windows on the second screen of a multi-headed
 *	  system.
 *
 * Bob Messenger	21-Apr-1989	X2.0-7
 *	- Use XtNtitle and XtNiconName instead of XmNtitle and
 *	  XmNiconName.
 *
 * Bob Messenger	19-Apr-1989	X2.0-6
 *	- Return status correctly from create_parent.
 * 	- Exit with status code on VMS.
 *	- Call process_exit instead of exit.
 *
 * Bob Messenger	10-Apr-1989	X2.0-6
 *	- Use error codes from decw$terminalmsg
 *	- Change create() to create_session().
 *	- Exit with DECW$_CANT_OPEN_DISPLAY if XOpenDisplay fails.  If the
 *	  emulator is changed later to support multiple displays per
 *	  controller this should simply return the status in the reply
 *	  message.
 *
 * Bob Messenger	 9-Apr-1989	X2.0-6
 *	- Add helpCallback when creating DECterm widget.
 *
 * Bob Messenger	 7-Apr-1989	X2.0-6
 *	- Use new widget bindings (DECwTerm instead of DwtDECterm).
 *	- Move dwt_get_optimal_size here from dwt_undefined.c, but
 *	  #if 0 since it's no longer called (preserve it in case we
 *	  need it later).
 *
 * Bob Messenger	 3-Apr-1989	X2.0-5
 *	- Exit when last stream is destroyed, even on VMS.
 *
 * Bob Messenger	 1-Apr-1989	X2.0-5
 *	- Add callback for shellValuesCallback, to support setting the
 *	  window title and icon name with OSC sequences.
 *	- Support terminalDriverResize
 *
 * Eric Osman		 7-Nov-1988	v1.0
 *	- Put up caution box if problem during DECterm creation
 *
 * Tom Porcher		14-Sep-1988	X0.5-2
 *	- Make default title/icon name be "DECterm", "DECterm 2", ...
 *
 * Tom Porcher		14-Sep-1988	X0.5-2
 *	- Make copyright notice disappear when pull-down menus pulled-down.
 *	- Make copyright notice adjust to size of window.
 *
 * Tom Porcher		11-Aug-1988	X0.4-44
 *	- Make copyright notice disappear when menubar touched.
 *
 * Mike Leibow          29-Jul-1988	X0.4-41
 *      - Put copyright notice in title bar.
 *
 * Eric Osman		25-Jul-1988	X0.4-38
 *	- Respond to system-level geometry changes.
 *
 * Tom Porcher		20-Jul-1988	X0.4-38
 *	- Added -geometry to shell for Ultrix.
 *
 * Tom Porcher		18-Jul-1988	X0.4-37
 *	- Moved VMS-specific PTY code pc_xon and pc_xoff to pty_vms.c.
 *
 * Eric Osman		14-Jul-1988	X0.4-35
 *	- When deleting stream on VMS, do it by setting kill-flag, then
 *	  putting entry on queue that says to really delete the stream.
 *
 * Tom Porcher		 8-Jul-1988	X0.4-35
 *	- made Ultrix version exit only when no streams are active by
 *	  setting run_flag to 0.
 *	- Corrected XrmFree; should be XrmFreeDatabase.
 *	- added call to file_destroy() to deallocate stuff allocated by
 *	  TEA_file.
 *
 * Tom Porcher		 8-Jul-1988	X0.4-35
 *	- Call open_display() rather than XtOpenDisplay.
 *	- Make create() return FALSE when display cannot be opened.
 *
 * Tom Porcher		 5-Jul-1988	X0.4-34
 *	- Pass terminal size to p_new_terminal().
 *
 * Tom Porcher          30-Jun-1988     X0.4-34
 *      - remove work-around for BL8.4/5 intrinsics XtGetValues() bug.
 *	- correct all XtGetValues of Positions and Dimensions.
 *
 * Tom Porcher		30-Jun-1988	X0.4-34
 *	- add termporary work-around for DECterm ACCVIO on "Quit":  close
 *	  displays from main loop.
 *
 * Tom Porcher		24-Jun-1988	X0.4-33
 *	- temporary work-around for BL8.4/5 intrinsics XtGetValues() bug:
 *	  Boolean => int
 *
 * Tom Porcher		21-Jun-1988	X0.4-32
 *	- create() now creates a new Xt application context for each stream.
 *	- removed open_display() and close_display().
 *	- fixed incorrect order of name, display_name in create() and
 *	  create_parent().
 *
 * Tom Porcher		 7-Jun-1988	X0.4-31
 *	- used DECTERM_APPL_xxx macros.
 *
 * Tom Porcher		 1-Jun-1988	X0.4-30
 *	- remove version number from title.
 *
 * Tom Porcher		30-May-1988	X0.4-30
 *	- use new PTY calls (see pty_vms.c).
 *
 * Tom Porcher		18-May-1988	X0.4-27
 *	- Allowed p_send_data to return a count of bytes actually sent.
 *
 * Tom Porcher		11-May-1988	X0.4-26
 *	- Added flow control.
 *
 * Tom Porcher		 5-Apr-1988	X0.4-14
 *	- Changed XtFree() of XrmDatabase to XrmFree (defined as Xfree).
 *	- Changed malloc() and free() to XtMalloc() and XtFree().
 *
 * Tom Porcher		 5-Apr-1988	X0.4-7
 *	- Corrected name of XtNallowShellResize for new toolkit.
 *	- Added "display" parameter to XtCreateApplicationShell,
 *	  and display_name as parameter to create().
 *	- Added open_display() and close_display().
 *
 * Tom Porcher		21-Mar-1988	X0.4-3
 *	- Added name, setup_file, customization parameters to create().
 *
 * Tom Porcher		13-Feb-1988	X0.3-4
 *	- Made Ultrix version exit when stream destroyed.
 *	- Added call to p_set_terminal_size on resize callback.
 *
 * Tom Porcher		20-Jan-1988	X0.3-3
 *	- Ultrix changes:
 *	    - defined extern Widget routines
 *	    - replaced goto with if block
 *
 * Tom Porcher		17-Jan-1988	X0.3-2
 *	- Changed case on "flavor" in create() to not call u_new_process
 *	  for _IDLE.  This was added inadvertently with the Ultrix mods.
 *
 * Tom Porcher		16-Jan-1988	X0.3-2
 *	- changed name of shell to "decterm" and name of work area to "terminal"
 *	  to make Xdefaults work.
 *	  Title is now set seperately.
 *
 * Peter Sichel		15-Jan-1988	X0.3
 *	- Integrated Ultrix changes.
 *
 * Tom Porcher		 8-Jan-1988	X0.3
 *	- Added call to dwt_get_optimal_size() on menubar.
 *
 * Eric Osman		 8-Jan-1988	X0.3
 *	- Call u_disconnected_p instead of p_disconnected_p (name change)
 *	- Remove XtPopUp call
 * Tom Porcher		 5-Jan-1988	X0.3
 *	- Changed computation for menubar height to include its borderwidth.
 *	- Removed code to default screenMode.
 *
 * Tom Porcher		31-Dec-1987
 *	- Changed call to XtGetValues; arg is now pointer.
 *	- made callback lists dynamic.
 *	- removed stm->display; no longer referenced.
 *	- changed XtCreatePopupShell to XtCreateApplicationShell
 *	- resizeCallback now checks autoResize setting.
 *
 * Eric Osman		18-Nov-1987
 *	Put back the xon/xoff stuff to attempt to increase throughput.
 * Eric Osman		17-Nov-1987
 *	Remove vms-dependent stuff.
 * Tom Porcher		5-Nov-1987
 *	Added changes to support *new* XLIB (BL5) way of using event flags.
 * Eric Osman		4-Nov-87
 *	Add mechanism for measuring PTY throughput by only sending one
 *	out of every 1000 datums to screen (which will be revealing if
 *	the program producing the data is announcing its performance)
 * Michael Leibow	Holloween-87
 *	Changed initialization code to make terminal, menu-bar, and
 *	main_window widget instead of just terminal widget.
 * Eric Osman		30-Sep-87
 *	Uses PY device for workstation.
 * Tom Porcher		29-Aug-85
 *	Now uses "PC" devices.
 *
 * Tom Porcher		19-Aug-85
 *	Changed pr_data(b->data,isn) to pr_data(b,isn) [2 places].
 *
 * Jerry Leichter	13-Mar-86
 *	Re-enable calls to pt_data() in pr_xoff() and pr_xon(); they had been
 *	no-op'ed out for no apparent reason.
 *
 * Jerry Leichter/Eric Osman	17-Mar-86
 *	Use a separate input buffer per stream for each PTY, break into small
 *	bufferloads in send1().
 *
 * Eric Osman		21-Mar-86
 *	In SEND1, at pr_data call for NULL chain case, lower stm->bytes so
 *	that XON will happen.
 *
 *	Move second to last closing brace in SEND1 to earlier so XON code
 *	gets executed more often.
 */

#include "mx.h"
#if defined(VMS) || defined(VXT_DECTERM)
#include "DXmCSText.h"
#include "MrmAppl.h"
#include "Intrinsic.h"
#include "Protocols.h"
#else
#include <errno.h>
#include <signal.h>
#include <DXm/DXmCSText.h>
#include <Mrm/MrmAppl.h>
#include <X11/Intrinsic.h>
#include <X11/Protocols.h>
#endif

#ifdef VXT_DECTERM
#include "stdarg.h"
#include <Vxtio.h>
#include <VxtPrinter.h>
#include <VxtConfig.h>
#include <VxtResources.h>
#include "msgboxconstants.h"
#include "msgboxpacket.h"
#endif

#if defined (VXT_DECTERM)
#define DECTERM_RES_lockOptions VXTRES_decterm_lockOptions
#elif defined (VMS_DECTERM)
#define DECTERM_RES_lockOptions "DECW$TERMINAL.lockOptions"
#else
#define DECTERM_RES_lockOptions "DXterm.lockOptions"
#endif

#ifdef HYPERHELP
extern void help_error();
#endif /* HYPERHELP */

/* messages from decw$terminalmsg */

#if defined(VMS_DECTERM)

globalvalue DECW$_MAX_EMULATORS, DECW$_BAD_ISN,
	    DECW$_CANT_CREATE_DRM_CONTEXT, DECW$_CANT_FETCH_DRM_VALUE,
	    DECW$_BAD_DRM_VALUE, DECW$_CANT_OPEN_FONT, DECW$_INSUFF_QUOTA;

#else

#define DECW$_MAX_EMULATORS 0
#define DECW$_BAD_ISN 0
#define DECW$_CANT_CREATE_DRM_CONTEXT 0
#define DECW$_CANT_FETCH_DRM_VALUE 0
#define DECW$_BAD_DRM_VALUE 0
#define DECW$_CANT_OPEN_FONT 0
#define DECW$_INSUFF_QUOTA 0
#endif

globalref verbose_flag;		/* TRUE means produce more verbose output
				   on stderr (usually goes to log file) */

#define MAX_PTY_INPUT 256

/* latest V3 icons */

#define SMALL_ICON_SIZE 32
static char small_icon_bits[] = {
   0xff, 0xff, 0xff, 0xff, 0xab, 0xaa, 0xaa, 0x2a, 0x55, 0x55, 0x55, 0x55,
   0xab, 0xaa, 0xaa, 0x2a, 0xd5, 0xff, 0xff, 0x55, 0xab, 0xaa, 0xaa, 0x2a,
   0x75, 0x00, 0x00, 0x51, 0xab, 0xaa, 0xaa, 0x2b, 0x75, 0x54, 0x55, 0x51,
   0xab, 0xaa, 0xaa, 0x2b, 0x75, 0x54, 0x55, 0x51, 0xab, 0xaa, 0xaa, 0x2b,
   0x75, 0x54, 0x55, 0x51, 0xab, 0xaa, 0xaa, 0x2b, 0x75, 0x54, 0x55, 0x51,
   0xab, 0xaa, 0xaa, 0x2b, 0x75, 0x54, 0x55, 0x51, 0xab, 0xfe, 0xff, 0x2b,
   0x15, 0x55, 0x55, 0x51, 0x2b, 0x00, 0x00, 0x28, 0x55, 0x55, 0x55, 0x55,
   0xab, 0xff, 0xff, 0x2a, 0xd5, 0x55, 0x55, 0x54, 0xeb, 0xee, 0xee, 0x28,
   0x75, 0x11, 0x11, 0x51, 0xbb, 0xbb, 0xbb, 0x23, 0x5d, 0x44, 0x44, 0x44,
   0xa3, 0xaa, 0xaa, 0x0a, 0x05, 0x00, 0x00, 0x40, 0x0b, 0x00, 0x00, 0x20,
   0x55, 0x55, 0x55, 0x55, 0x01, 0x00, 0x00, 0x00};


#define MEDIUM_ICON_SIZE 50
static char medium_icon_bits[] = {
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03, 0x55, 0x55, 0x55, 0x55, 0x55,
   0x55, 0x01, 0xab, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x00, 0x55, 0x55, 0x55,
   0x55, 0x55, 0x55, 0x01, 0xab, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x00, 0x55,
   0x55, 0x55, 0x55, 0x55, 0x55, 0x01, 0xab, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
   0x00, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x01, 0xab, 0xfe, 0xff, 0xff,
   0xbf, 0xaa, 0x00, 0x55, 0x57, 0x55, 0x55, 0x55, 0x55, 0x01, 0xab, 0xaa,
   0xaa, 0xaa, 0x2a, 0xaa, 0x00, 0x55, 0x17, 0x00, 0x00, 0x50, 0x55, 0x01,
   0xab, 0xaa, 0xaa, 0xaa, 0x3a, 0xaa, 0x00, 0x55, 0x47, 0x55, 0x55, 0x55,
   0x55, 0x01, 0xab, 0xaa, 0xaa, 0xaa, 0x3a, 0xaa, 0x00, 0x55, 0x47, 0x55,
   0x55, 0x55, 0x55, 0x01, 0xab, 0xaa, 0xaa, 0xaa, 0x3a, 0xaa, 0x00, 0x55,
   0x47, 0x55, 0x55, 0x55, 0x55, 0x01, 0xab, 0xaa, 0xaa, 0xaa, 0x3a, 0xaa,
   0x00, 0x55, 0x47, 0x55, 0x55, 0x55, 0x55, 0x01, 0xab, 0xaa, 0xaa, 0xaa,
   0x3a, 0xaa, 0x00, 0x55, 0x47, 0x55, 0x55, 0x55, 0x55, 0x01, 0xab, 0xaa,
   0xaa, 0xaa, 0x3a, 0xaa, 0x00, 0x55, 0x47, 0x55, 0x55, 0x55, 0x55, 0x01,
   0xab, 0xaa, 0xaa, 0xaa, 0x3a, 0xaa, 0x00, 0x55, 0x47, 0x55, 0x55, 0x55,
   0x55, 0x01, 0xab, 0xaa, 0xaa, 0xaa, 0x3a, 0xaa, 0x00, 0x55, 0x47, 0x55,
   0x55, 0x55, 0x55, 0x01, 0xab, 0xaa, 0xaa, 0xaa, 0x3a, 0xaa, 0x00, 0x55,
   0x47, 0x55, 0x55, 0x55, 0x55, 0x01, 0xab, 0xea, 0xff, 0xff, 0x3f, 0xaa,
   0x00, 0x55, 0x51, 0x55, 0x55, 0x15, 0x55, 0x01, 0xab, 0x02, 0x00, 0x00,
   0x80, 0xaa, 0x00, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x01, 0xab, 0xfa,
   0xff, 0xff, 0xbf, 0xaa, 0x00, 0x55, 0x5d, 0x55, 0x55, 0x15, 0x55, 0x01,
   0xab, 0xae, 0xaa, 0xaa, 0x2a, 0xaa, 0x00, 0x55, 0xd7, 0xdd, 0xdd, 0x5d,
   0x54, 0x01, 0xab, 0x2b, 0x22, 0x22, 0xa2, 0xa8, 0x00, 0xd5, 0xdd, 0xdd,
   0xdd, 0xdd, 0x51, 0x01, 0xeb, 0x22, 0x22, 0x22, 0x22, 0xa2, 0x00, 0x55,
   0x55, 0x55, 0x55, 0x55, 0x45, 0x01, 0x0b, 0x00, 0x00, 0x00, 0x00, 0xa0,
   0x00, 0x15, 0x00, 0x00, 0x00, 0x00, 0x50, 0x01, 0xab, 0xaa, 0xaa, 0xaa,
   0xaa, 0xaa, 0x00, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x01, 0xab, 0xaa,
   0xaa, 0xaa, 0xaa, 0xaa, 0x00, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x01,
   0xab, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00};

#define LARGE_ICON_SIZE 75
static char large_icon_bits[] = {
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x07, 0x55, 0x55,
   0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x01, 0xab, 0xaa, 0xaa, 0xaa,
   0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x02, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
   0x55, 0x55, 0x55, 0x01, 0xab, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
   0xaa, 0x02, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x01,
   0xab, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x02, 0x55, 0x55,
   0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x01, 0xab, 0xaa, 0xaa, 0xaa,
   0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x02, 0x55, 0x55, 0xff, 0xff, 0xff, 0xff,
   0xff, 0x55, 0x55, 0x01, 0xab, 0xaa, 0xab, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
   0xaa, 0x02, 0x55, 0xd5, 0x55, 0x55, 0x55, 0x55, 0x55, 0x51, 0x55, 0x01,
   0xab, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xa2, 0xaa, 0x02, 0x55, 0xd5,
   0x05, 0x00, 0x00, 0x00, 0x00, 0x55, 0x55, 0x01, 0xab, 0xaa, 0xa2, 0xaa,
   0xaa, 0xaa, 0xaa, 0xa2, 0xaa, 0x02, 0x55, 0xd5, 0x55, 0x55, 0x55, 0x55,
   0xd5, 0x55, 0x55, 0x01, 0xab, 0xaa, 0xa2, 0xaa, 0xaa, 0xaa, 0xaa, 0xa2,
   0xaa, 0x02, 0x55, 0xd5, 0x55, 0x55, 0x55, 0x55, 0xd5, 0x55, 0x55, 0x01,
   0xab, 0xaa, 0xa2, 0xaa, 0xaa, 0xaa, 0xaa, 0xa2, 0xaa, 0x02, 0x55, 0xd5,
   0x55, 0xff, 0x5f, 0x55, 0xd5, 0x55, 0x55, 0x01, 0xab, 0xaa, 0xa2, 0xaa,
   0xaa, 0xaa, 0xaa, 0xa2, 0xaa, 0x02, 0x55, 0xd5, 0x55, 0x55, 0x55, 0x55,
   0xd5, 0x55, 0x55, 0x01, 0xab, 0xaa, 0xa2, 0xaa, 0xfe, 0xff, 0xab, 0xa2,
   0xaa, 0x02, 0x55, 0xd5, 0x55, 0x55, 0x55, 0x55, 0xd5, 0x55, 0x55, 0x01,
   0xab, 0xaa, 0xa2, 0xaa, 0xaa, 0xaa, 0xaa, 0xa2, 0xaa, 0x02, 0x55, 0xd5,
   0x55, 0x55, 0x55, 0x55, 0xd5, 0x55, 0x55, 0x01, 0xab, 0xaa, 0xa2, 0xaa,
   0xaa, 0xaa, 0xaa, 0xa2, 0xaa, 0x02, 0x55, 0xd5, 0x55, 0xff, 0xd7, 0xff,
   0xd5, 0x55, 0x55, 0x01, 0xab, 0xaa, 0xa2, 0xaa, 0xaa, 0xaa, 0xaa, 0xa2,
   0xaa, 0x02, 0x55, 0xd5, 0x55, 0x55, 0x55, 0x55, 0xd5, 0x55, 0x55, 0x01,
   0xab, 0xaa, 0xa2, 0xaa, 0xaa, 0xaa, 0xaa, 0xa2, 0xaa, 0x02, 0x55, 0xd5,
   0x55, 0xff, 0xff, 0x55, 0xd5, 0x55, 0x55, 0x01, 0xab, 0xaa, 0xa2, 0xaa,
   0xaa, 0xaa, 0xaa, 0xa2, 0xaa, 0x02, 0x55, 0xd5, 0x55, 0x55, 0x55, 0x55,
   0xd5, 0x55, 0x55, 0x01, 0xab, 0xaa, 0xa2, 0xaa, 0xaa, 0xaa, 0xaa, 0xa2,
   0xaa, 0x02, 0x55, 0xd5, 0x55, 0xff, 0x5f, 0x55, 0xd5, 0x55, 0x55, 0x01,
   0xab, 0xaa, 0xa2, 0xaa, 0xaa, 0xaa, 0xaa, 0xa2, 0xaa, 0x02, 0x55, 0xd5,
   0x55, 0x55, 0x55, 0x55, 0xd5, 0x55, 0x55, 0x01, 0xab, 0xaa, 0xa2, 0xaa,
   0xaa, 0xaa, 0xaa, 0xa2, 0xaa, 0x02, 0x55, 0xd5, 0x55, 0x55, 0x55, 0x55,
   0xd5, 0x55, 0x55, 0x01, 0xab, 0xaa, 0xa2, 0xaa, 0xaa, 0xaa, 0xaa, 0xa2,
   0xaa, 0x02, 0x55, 0xd5, 0xf5, 0xff, 0xff, 0xff, 0xff, 0x55, 0x55, 0x01,
   0xab, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xa2, 0xaa, 0x02, 0x55, 0xd5,
   0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x01, 0xab, 0x2a, 0xaa, 0xaa,
   0xaa, 0xaa, 0xaa, 0xa2, 0xaa, 0x02, 0x55, 0x55, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x50, 0x55, 0x01, 0xab, 0xaa, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa8,
   0xaa, 0x02, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x01,
   0xab, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x02, 0x55, 0x55,
   0xfd, 0xff, 0xff, 0xff, 0xff, 0x55, 0x55, 0x01, 0xab, 0xaa, 0xae, 0xaa,
   0xaa, 0xaa, 0xaa, 0xa8, 0xaa, 0x02, 0x55, 0x55, 0x57, 0x55, 0x55, 0x55,
   0x55, 0x51, 0x55, 0x01, 0xab, 0xaa, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xa3,
   0xaa, 0x02, 0x55, 0xd5, 0x45, 0x44, 0x44, 0x44, 0x44, 0x44, 0x55, 0x01,
   0xab, 0xea, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x8a, 0xaa, 0x02, 0x55, 0x75,
   0x55, 0x55, 0x55, 0x55, 0x55, 0x15, 0x55, 0x01, 0xab, 0xba, 0xbb, 0xbb,
   0xbb, 0xbb, 0xbb, 0x3b, 0xaa, 0x02, 0x55, 0x5d, 0x44, 0x44, 0x44, 0x44,
   0x44, 0x44, 0x54, 0x01, 0xab, 0xae, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
   0xa8, 0x02, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x54, 0x01,
   0xab, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa8, 0x02, 0x55, 0x01,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x54, 0x01, 0xab, 0x02, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0xaa, 0x02, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
   0x55, 0x55, 0x55, 0x01, 0xab, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
   0xaa, 0x02, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x01,
   0xab, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x02, 0x55, 0x55,
   0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x01, 0xab, 0xaa, 0xaa, 0xaa,
   0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x02, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
   0x55, 0x55, 0x55, 0x01, 0xab, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
   0xaa, 0x02, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x01,
   0xab, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x02, 0x55, 0x55,
   0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x01, 0x01, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


/* declare external functions */
extern int p_new_terminal();
extern void p_set_terminal_size();
extern void p_close();
extern Boolean p_write();

extern void file_initialize();
extern void file_destroy();

extern STREAM *convert_widget_to_stream();
extern void commands_resizewindow_cb();
extern Widget SetupInit();
extern Widget DECwTermCreate();
extern Widget XmCreateMainWindow();

extern void decwterm_help_cb();

extern void start_printing_handler();
extern void stop_printing_handler();
static void send_break_handler();
static void destroy_callback();
extern void read_data_from_prt();
extern void stop_read_data_from_prt();
extern void print_line_handler();
extern void printer_status_handler();
extern void _DECwTermSetTitle();
extern void _DECwTermSetIconName();
       void close_stream();
       Boolean chars_from_widget();
static void do_copyright_notice();
static void undo_copyright_notice();

STREAM *streams[MaxISN+1];		/* Stream database		*/

       int streams_active = 0;
static int decterm_number = 0;
/*
   change the def_x,def_y position to 0,0 for motif.  Most people put their
   icons at the bottom of the screen.  Changed to 30,50 to be more visually
   appealing.
*/
globaldef int def_cols=0, def_rows=0, def_x=30, def_y=50;

globalref char run_flag;
globalref char attach_flag, login_flag, measure_pty_flag, measure_scroll_flag;

globalref char *geometry_arg;

globaldef int measure_pty_ctr;

globalref MrmHierarchy s_MRMHierarchy;    /* DRM database id */

globalref Boolean multi_uid;

static Atom wm_protocols_atom = 0;
static Atom wm_delete_window_atom = 0;

static void nonmaskable_event();


static void
get_terminal_size( stm, columnsp, rowsp, widthp, heightp )
    STREAM *stm;
    int *columnsp, *rowsp, *widthp, *heightp;
{
    Arg arglist[2];
    Dimension width, height;

    XtSetArg( arglist[0], DECwNcolumns, columnsp);
    XtSetArg( arglist[1], DECwNrows, rowsp);
    XtGetValues( stm->terminal, arglist, 2);
    XtSetArg( arglist[0], XmNwidth, &width);
    XtSetArg( arglist[1], XmNheight, &height);
    XtGetValues( stm->parent, arglist, 2);
    *widthp = width;
    *heightp = height;
}

   
/* The following routine was added to fix the problem where DECterm would resize
   itself slightly after being maximized if the Auto Resize Terminal feature was
   enabled.  The problem was that the window manager would resize DECterm such
   that it occupied the entire screen.  Since this was not on a whole character
   boundary, DECterm would compute an optimal size which was slightly less than
   the entire screen.  This prevented the user from using restore to return the
   DECterm to its original size.  So the solution was to set the window manager
   resize hints, such that it would never resize a DECterm to something other
   than on a character boundary.  But this introduced some problems.  One has
   to be careful to update the hints in the following cases:
   
        1. If the font size changes,
        2. If a scrollbar is enabled or disabled, or
        3. If the status line is enabled or disabled.

   But there's a catch!  The hints have to be set as soon as the font size is
   changed, but we don't know until after the font size has changed.  At this
   point the widget has already been resized, but the old hints were in effect,
   leaving the DECterm in an "illegal" state.  The solution is turn off the
   hints before the widget resizes itself, and turn them back on afterwards.
   The second parameter to this routine is an on-off switch.  So, to catch all
   three cases above, this routine is called after a DECterm has been created,
   from the routines that deal with the Window and Display dialog boxes (in
   menu_stubs.c) and in resize_handler and pty_size_changed (in pty_vms.c),
   since the font may change if the user does a SET TERM/WIDTH and the
   adjustFontSizes resource is set.  This last case was the hardest to catch,
   and there is still the opportunity for DECterm to get into an "illegal"
   state if the DECSCPP escape sequence is used.
 */
   
#define X_MARGIN 4              /* These three #defines are copied from   */
#define Y_MARGIN 4              /* dt_output.h, since that file can't be  */
#define SCROLL_BAR_WIDTH 19     /* #included from within the application. */

void
SetWMHints(stm, enable_hints)
STREAM  *stm;
Boolean enable_hints;
{
/* This function is being removed on VMS systems, because it introduced a bug
   when doing a SET/TERM/WIDTH and the adjustFontSizes resource is enabled.
   There were several symptons to this problem: the DECterm would lose a row
   and/or column (i.e., you would get a 79x23 DECterm when you were expecting
   80x24), or you would get a DECterm as wide as the screen (for example 164x24
   when you only asked for 132x24), or you would have the "incredible shrinking
   DECterm" problem if you used the XUI window manager instead of mwm.

   What happens is that we get an AST from the PTD driver informing us
   that the pty's size has changed, and we also get the DECCOLM escape
   sequence to inform us to change fonts.  There is a race condition I
   haven't been able to track down.  When running with -synchronous all is
   well, but if not, the resize problem appears, especially on slow servers.
*/
#ifndef VMS
    XSizeHints	sh;

    if (enable_hints)
    {
        Arg     arglist[5];
        Boolean	scrollHorizontal, scrollVertical, statusLine;
        int     height_inc, width_inc, n = 0;

        XtSetArg(arglist[n], DECwNscrollHorizontal, &scrollHorizontal); n++;
        XtSetArg(arglist[n], DECwNscrollVertical, &scrollVertical); n++;
        XtSetArg(arglist[n], DECwNstatusDisplayEnable, &statusLine); n++;
        XtSetArg(arglist[n], DECwNdisplayHeightInc, &height_inc); n++;
        XtSetArg(arglist[n], DECwNdisplayWidthInc, &width_inc); n++;
        XtGetValues(stm->terminal, arglist, n);

        sh.height_inc = height_inc;
        sh.width_inc = width_inc;
        sh.base_height = 2 * Y_MARGIN + stm->menubar->core.height +
                         2 * stm->menubar->core.border_width +
                         (scrollHorizontal ? SCROLL_BAR_WIDTH : 0) +
                         (statusLine ? height_inc + 3 : 0);
        sh.base_width = 2 * X_MARGIN + (scrollVertical ? SCROLL_BAR_WIDTH : 0);

    }
    else
    {
        sh.height_inc = 1;
        sh.width_inc = 1;
        sh.base_height = 0;
        sh.base_width = 0;
    }

    sh.flags = PResizeInc | PBaseSize;

    XSetWMNormalHints(stm->display, XtWindow(stm->parent), &sh);
#endif VMS
}


static void
resize_handler( w, stm, call_data )
    Widget w;
    STREAM *stm;
    int *call_data;        
{
    Arg arglist[2];
    Boolean auto_resize_window, terminal_driver_resize;
    int columns, rows, width, height;

    XtSetArg( arglist[0], DECwNautoResizeWindow, &auto_resize_window);
    XtSetArg( arglist[1], DECwNterminalDriverResize, &terminal_driver_resize);
    XtGetValues( stm->terminal, arglist, 2);

    SetWMHints(stm, FALSE);  /* Turn off hints so window can resize freely */

    if ( auto_resize_window )
	commands_resizewindow_cb( w, 0, 0 );

    get_terminal_size( stm, &columns, &rows, &width, &height );
    if ( terminal_driver_resize )
	p_set_terminal_size( stm, columns, rows, width, height );

    SetWMHints(stm, TRUE);   /* Turn on hints after resize */
}

static void
shell_values_handler( w, stm, call_data )
    Widget w;
    STREAM *stm;
    DECwTermArgCallbackStruct *call_data;
{
/*
 * This is a hack, but... if the title is changed, also change the default
 * title to be restored after the first input.
 */
    int i;

    for ( i = 0; i < call_data->num_args; i++ )
	if ( strcmp( call_data->arglist[i].name, "title" ) == 0 )
	    {
	    if ( stm->window_name != NULL )
		XtFree( stm->window_name );
	    stm->window_name = XtNewString( call_data->arglist[i].value );
	    }

    XtSetValues( stm->parent, call_data->arglist, call_data->num_args );
}

static void
widget_error_handler( w, stm, call_data )
    Widget w;
    STREAM *stm;
    DECwTermErrorCallbackStruct *call_data;        
{
    switch( call_data->code )
	{
#ifdef VMS_DECTERM
	case DECW$K_MSG_INFORMATIONAL:
	    fputs( call_data->text, stderr );
	    break;
	case DECW$K_MSG_SYSTEM_ERROR:
	    LIB$SIGNAL( call_data->status );
	    break;
#endif
	case DECW$K_MSG_CANT_FIND_FONT:
	    if ( ! XtIsRealized( stm->parent ) )
		log_message( "Can't find font %s\n", call_data->text );
	    else
		warn_window(stm, "find_font_warning", call_data->text );
	    break;
	case DECW$K_MSG_EXIT_NO_FONT:
	    process_exit( DECW$_CANT_OPEN_FONT );
	    break;
	case DECW$K_MSG_NO_INPUT_METHOD:
	    if ( ! XtIsRealized( stm->parent ) )
		log_message( "No %s input method\n", call_data->text );
	    else
		warn_window( stm, "input_method_warning", call_data->text );
	    break;
	}
}

void pty_init()
{    	int i;
	for (i = 0; i <= MaxISN; i++)
		streams[i] = NULL;
#ifdef VMS_DECTERM
	{
	extern void p_get_defchar();
	p_get_defchar();	/* to get TTY_DEFCHAR and TTY_DEFCHAR2 */
	}
#endif
}
/*
 * Finish up everything related to PTY's.  Basically, we just loop through
 * all the open streams and destroy them.
 */
void
pty_fin()
{	ISN i;

	for (i = 0; i <= MaxISN; i++)
		if (streams[i] != NULL )
			close_stream(streams[i]);
}

job_done (isn) {
	STREAM *stm = GetSTM (isn,"job_done");
	close_stream(stm);
	}

/*
 * o.k.  Here's where we tell the toolkit to please change the terminal
 * widget's size because the operating system has changed its idea
 * of the size.
 *
 */
void
pty_size_changed( stm, new_width, new_height )
STREAM *stm;
int     new_width, new_height;
{
    Arg arglist[3];
    Boolean terminal_driver_resize;
    int old_height, old_width;

    XtSetArg( arglist[0], DECwNterminalDriverResize, &terminal_driver_resize );
    XtSetArg( arglist[1], DECwNrows, &old_height );
    XtSetArg( arglist[2], DECwNcolumns, &old_width );
    XtGetValues( stm->terminal, arglist, 3 );

    if (new_height == 0) new_height = 1;

/* Check to see if the terminal size has actually changed before turning off
   window manager hints, since the hints are turned back on in resize_handler,
   and it won't get called unless there is a change in size.
 */
    if ((old_height != new_height || old_width != new_width) &&
        stm->parent != NULL)
    {
        SetWMHints(stm, FALSE); /* Turn off hints so widget can resize freely */

        if ( terminal_driver_resize )
        {
            XtSetArg( arglist[0], DECwNrows, new_height );
            XtSetArg( arglist[1], DECwNcolumns, new_width );

            XtSetValues( stm->terminal, arglist, 2 );
        }
    }
}

void
pty_clear_comm( stm )
    STREAM *stm;
{
    p_clear_write( stm );
    p_resume_read( stm );
}

static void
widget_full( w, stm, reasonp )
    Widget w;
    STREAM *stm;
    int *reasonp;
{
    p_stop_read( stm );
}

static void
widget_hungry( w, stm, reasonp )
    Widget w;
    STREAM *stm;
    int *reasonp;
{
    p_resume_read( stm );
}


/*
 * Routine to count how many DECterms are in use.
 */
decterms_in_use ()
{
ISN isn;
int tally = 0;
for (isn = 0; isn <= MaxISN; isn++) if (streams[isn]) tally++;
return tally;
}

/*
 * Start up a new stream.
 *
 * Input:
 *	flavor			type of startup to perform (log in?  etc.)
 *	setup_file		name of file for initial set-up
 *	customization		command-line customization string
 *	tt_chars		terminal characteristics on VMS, not used
 *				on Ultrix
 */

int create_session(flavor, display_name, name, setup_file, customization,
	tt_chars, lang_index, io_transport, io_path, flags, resultant_isn)
    int flavor;
    char *display_name;
    char *name;
    char *setup_file;
    char *customization;
    char *tt_chars;
    char *io_transport;
    char *io_path;
    ISN *resultant_isn;
{	
    STREAM *stm;
	ISN isn;
	char *type_id;
	XrmValue type_value;
	int n;
	Arg arglist[20];
	char *title, *iconName;
	XmString title_cs, iconName_cs;
	Atom titleEncoding, iconNameEncoding, XA_COMPOUND_TEXT;
	long count, status;

/*
 * Make sure quotas are sufficient for another decterm.  Snapshot current
 * quotas so on next decterm we can see if we have the anticipated amount
 * of space left.
 */
#ifdef VMS_DECTERM
	{
	char *low_quota = check_quotas ();
	if (low_quota)
	    {
	    warn_window (NON_STREAM, low_quota);
	    return DECW$_INSUFF_QUOTA;
	    }
	}
#endif VMS_DECTERM

#ifdef VXT_DECTERM
	sys_announce_new_decterm (io_transport, io_path);
#endif VXT_DECTERM
/*
 * Find unused stream.
 */
	stm = NULL;
	for (isn = 0; isn <= MaxISN; isn++)
	    if (streams[isn] == NULL )
		{
		stm = streams[isn] = (STREAM *) XtCalloc( sizeof ( *stm ), 1 );
		stm->isn = isn;
		break;
		}

	if (stm == NULL )
	    {
	    printf ("No more internal DECterm slots left\n");
#ifdef VXT_DECTERM
	   vxt_msgbox_write( TM_MSG_WARNING, 1, k_decterm_too_many_decterms, 0);
#endif
	    return (DECW$_MAX_EMULATORS);
	    }

	if (resultant_isn) *resultant_isn = isn;

	streams_active++;

	stm->printer.active = False;
	stm->printer.status = DECwNoPrinter;
	stm->printer.graphics_delay_id = NULL;

/* initialize io file class and path for establishing a connection.  We make
 * a copy of the string, since the copy we were given points into a structure
 * whose contents changes when next decterm is opened.
 */

    stm->pty.io_transport = io_transport ? XtNewString (io_transport) : 0;
    stm->pty.io_path = io_path ? XtNewString (io_path) : 0;

/*
 * Create parent.                     
 */
	{
	int status;
	extern int create_parent ();
	if (FAILED( status =
		create_parent (isn, display_name, name, lang_index))) {
	    close_stream( stm );
	    return(status);
	}
	}
/*
 * Create the DECterm widget.
 */
	{
	globalref int def_cols, def_rows;
	Boolean reverse_video;
	Dimension mwidth,mheight,mbwidth;
	int theight,twidth;
        XtCallbackRec input[2],  xoff[2], xon[2], resize[2], shell[2], help[2],
		error[2], start_printing[2], stop_printing[2], print_line[2],
		printer_status[2], start_printer_to_host[2],
		stop_printer_to_host[2], send_break[2], exit_decterm[2];

	INIT_CALLBACK( input, chars_from_widget, stm );
	INIT_CALLBACK( xoff, widget_full, stm );		
	INIT_CALLBACK( xon, widget_hungry, stm );
	INIT_CALLBACK( resize, resize_handler, stm );
	INIT_CALLBACK( shell, shell_values_handler, stm );
	INIT_CALLBACK( help, decwterm_help_cb, stm );
	INIT_CALLBACK( error, widget_error_handler, stm );
	INIT_CALLBACK( start_printing, start_printing_handler, stm );
	INIT_CALLBACK( stop_printing, stop_printing_handler, stm );
	INIT_CALLBACK( start_printer_to_host, read_data_from_prt, stm );
	INIT_CALLBACK( stop_printer_to_host, stop_read_data_from_prt, stm );
	INIT_CALLBACK( print_line, print_line_handler, stm );
	INIT_CALLBACK( printer_status, printer_status_handler, stm );
	INIT_CALLBACK( send_break, send_break_handler, stm);
	INIT_CALLBACK( exit_decterm, destroy_callback, stm);
#ifdef ORIGINAL_CODE
 *     stm->widget = DwtMainWindow(stm->parent, "main", 0, 0, 0, 0)
#endif

	{
	Arg	al[10];
	int	ac = 0;

	XtSetArg(al[ac], XmNx, 0); ac++;
	XtSetArg(al[ac], XmNy, 0); ac++;
	XtSetArg(al[ac], XmNwidth, 0); ac++;
	XtSetArg(al[ac], XmNheight, 0); ac++;
	stm->widget =  XmCreateMainWindow(stm->parent,"main", al, ac);
	}
    XtManageChild(stm->widget);

    n = 0;
    XtSetArg( arglist[n], XmNborderWidth, 0);			n++;
    XtSetValues( stm->widget, arglist, n);
    XtSetArg( arglist[n], DECwNinputCallback, input);		n++;
    XtSetArg( arglist[n], DECwNstopOutputCallback, xoff);	n++;
    XtSetArg( arglist[n], DECwNstartOutputCallback, xon);	n++;
    XtSetArg( arglist[n], DECwNresizeCallback, resize);		n++;
    XtSetArg( arglist[n], DECwNshellValuesCallback, shell);	n++;
    XtSetArg( arglist[n], DECwNhelpCallback, help);		n++;
    XtSetArg( arglist[n], DECwNerrorCallback, error);		n++;
    XtSetArg( arglist[n], DECwNstartPrintingCallback, start_printing);
								n++;
    XtSetArg( arglist[n], DECwNstopPrintingCallback, stop_printing);
								n++;
    XtSetArg( arglist[n], DECwNstartPrinterToHostCallback,
	start_printer_to_host);					n++;
    XtSetArg( arglist[n], DECwNstopPrinterToHostCallback, stop_printer_to_host);
								n++;
    XtSetArg( arglist[n], DECwNprintLineCallback, print_line);	n++;
    XtSetArg( arglist[n], DECwNprinterStatusCallback, printer_status);
								n++;
    XtSetArg( arglist[n], DECwNprinterStatus, stm->printer.status);
								n++;
    XtSetArg( arglist[n], DECwNsendBreakCallback, send_break);	n++;
    XtSetArg( arglist[n], DECwNexitCallback, exit_decterm);	n++;
    XtSetArg( arglist[n], DECwNmaxInput, MAX_PTY_INPUT);	n++;
    switch ( lang_index ) {
	case 99: XtSetArg( arglist[n], DECwNterminalType, DECwMulti );	n++;
		 break;
	case  7: XtSetArg( arglist[n], DECwNterminalType, DECwHanzi );	n++;
		 break;		/* zh_CN */
	case  8: XtSetArg( arglist[n], DECwNterminalType, DECwHanyu );	n++;
		 break;		/* zh_TW */
	case 15: XtSetArg( arglist[n], DECwNterminalType, DECwHebrew );	n++;
		 break;		/* iw_IL */
	case 17: XtSetArg( arglist[n], DECwNterminalType, DECwKanji );	n++;
		 break;		/* ja_JP */
	case 18: XtSetArg( arglist[n], DECwNterminalType, DECwHangul );	n++;
		 break;		/* ko_KR */
	case 28: XtSetArg( arglist[n], DECwNterminalType, DECwGreek );  n++;
		 break;		/* gr_GR */
	case 29: XtSetArg( arglist[n], DECwNterminalType, DECwTurkish );  n++;
		 break;		/* tr_TR */

	case  0:		/* en_US */
	default: XtSetArg( arglist[n], DECwNterminalType, DECwStandard2 ); n++;
		 break;
    }
    if (def_cols)
        {XtSetArg( arglist[n], DECwNcolumns, def_cols);		n++;}
    if (def_rows)
        {XtSetArg( arglist[n], DECwNrows, def_rows);		n++;}

    stm->terminal = DECwTermCreate(stm->widget, "terminal", arglist, n);
    XtManageChild(stm->terminal);

    stm->menubar = SetupInit(stm->widget, isn);
    XtManageChild(stm->menubar);

/*
 * Read in selected configuration file
 */
    file_initialize( stm, setup_file );

/*
 * Add in user-specified customization string
 */
    if (customization != NULL) {
	XrmDatabase rdb;

	rdb = XrmGetStringDatabase( customization );
	if (rdb != NULL) {
	    PutWidgetTreeDatabase( stm->parent, rdb );
	    XrmDestroyDatabase( rdb );
	}
    }

    /* Need to apply the locking customization for DECterm.  DAM
     */

    if (XrmGetResource(stm->file.default_rdb,
                        DECTERM_RES_lockOptions, DECTERM_RES_lockOptions,
                        &type_id, &type_value))
      {
      Arg arglist[1];
      Cardinal argcount;
      int real_value;

      real_value = atoi(type_value.addr);
      
      if (real_value == 2)
	{
        XtSetArg(arglist[0], XmNsensitive, FALSE);
        XtSetValues(stm->setup.options_widget_id, arglist, 1);
        }
      else if (real_value == 1)
	{
        XtSetArg(arglist[0], XmNsensitive, FALSE);
        XtSetValues(stm->setup.save_options_id, arglist, 1);
	}
      }

#ifdef VXT_DECTERM
    /* Printer characteristics on VXT platform are selected from the local
	terminal manager, therefore do not want separate selection on each 
	DECterm window.  Graphic print screen is done by calling a VXT
	specific routine, which reads printer characteristics directly,
        therefore characteristics associated with graphic printing need not 
	be set in DECterm. Therefore in DECterm level 1, level 2, or LA210 
	sixel graphic settings, and compressed, expanded or rotated graphics 
	printing are hard coded to some dummy values. */

    n = 0;
    XtSetArg( arglist[n], DECwNprintSixelLevel, DECwSixelLevel_1 ); n++;
    XtSetArg( arglist[n], DECwNprintFormat,  DECwCompressedPrinting ); n++;

    /* only RGB color syntax is supported in VXT */

    XtSetArg( arglist[n], DECwNprintHLSColorSyntax, False ); n++;

    XtSetValues( stm->terminal, arglist, n );
#endif VXT_DECTERM

    /*
     * Read back default title and icon name
     */

    XtSetArg( arglist[0], XtNtitle, &title );
    XtSetArg( arglist[1], XtNiconName, &iconName );
    XtSetArg( arglist[2], XtNtitleEncoding, &titleEncoding );
    XtSetArg( arglist[3], XtNiconNameEncoding, &iconNameEncoding );
    XtGetValues( stm->parent, arglist, 4 );
    XA_COMPOUND_TEXT =
	XInternAtom( XtDisplay( stm->parent ), "COMPOUND_TEXT", FALSE );
    if ( titleEncoding == XA_COMPOUND_TEXT ) {
	title_cs = XmCvtCTToXmString( title );
	title = DXmCvtCStoFC( title_cs, &count, &status );
	XmStringFree( title_cs );
    }
    if ( iconNameEncoding == XA_COMPOUND_TEXT ) {
	iconName_cs = XmCvtCTToXmString( iconName );
	iconName = DXmCvtCStoFC( iconName_cs, &count, &status );
	XmStringFree( iconName_cs );
    }

    /*
     * Write the values of resources whose default values are computed
     * at run-time into the default resource database.  This is so that if
     * the user saves current settings, the resources won't be written to the
     * file unless their value is different from the *computed* default.
     */

    PutOneResource( stm->terminal, DECwNbigFontSetName,
      &stm->file.default_rdb );
    PutOneResource( stm->terminal, DECwNlittleFontSetName,
      &stm->file.default_rdb );

    /*
     * Now that all initialization has been applied, correct
     * window size of DECterm widget.
     */

    XtSetArg( arglist[0], DECwNdisplayWidth, &twidth);
    XtSetArg( arglist[1], DECwNdisplayHeight, &theight);
    XtGetValues( stm->terminal, arglist, 2);
    XtSetArg( arglist[0], XmNwidth, twidth);
    XtSetArg( arglist[1], XmNheight, theight);
    XtSetArg( arglist[2], DECwNdefaultTitle, title );
    XtSetArg( arglist[3], DECwNdefaultIconName, iconName );
    XtSetValues( stm->terminal, arglist, 4);
    if ( titleEncoding == XA_COMPOUND_TEXT )
	XtFree( title );
    if ( iconNameEncoding == XA_COMPOUND_TEXT )
	XtFree( iconName );

    XmMainWindowSetAreas(stm->widget, stm->menubar, NULL,
						 NULL, NULL, stm->terminal);

    XtSetArg( arglist[0], DECwNterminalType, &stm->terminalType );
    XtGetValues( stm->terminal, arglist, 1 );

	}
/*
 * Make new DECterm appear on screen.
 */
	XtRealizeWidget(stm->parent);
/*
 * Put Copyright in title bar.
 * Must be called after all initialization is complete because
 * do_copyright_notice will do an XtGetValues to get the old name from the
 * title bar.
 * Must be done after XtRealize, lest width be unavailable with getvalues.
 */
	do_copyright_notice( stm );    /* copyright notice will remove itself */

	if ( multi_uid || IsAsianOrHebrewTermType(stm->terminalType)) {
	    XtSetArg( arglist[0], XtNiconName, &iconName );
	    XtSetArg( arglist[1], XtNiconNameEncoding, &iconNameEncoding );
	    XtGetValues( stm->parent, arglist, 2 );
	    iconName_cs = ( iconNameEncoding ==
	XInternAtom( XtDisplay( stm->parent ), "COMPOUND_TEXT", FALSE )) ?
		      ( XmString )XmCvtCTToXmString( iconName ) :
		      ( XmString )DXmCvtFCtoCS( iconName, &count, &status );
	    _DECwTermSetIconName( stm->parent, stm->terminal, iconName_cs );
	    XmStringFree( iconName_cs );
	}

/*
 * Make sure the window is on screen (this only works after the window has
 * been realized).
 */
	commands_resizewindow_cb( stm->parent, 0, 0 );
/*                               
 * Obtain pseudo-terminal (PTY)
 */
	{
	    int columns,rows,width,height, status;

	    get_terminal_size( stm, &columns, &rows, &width, &height );

	    status = p_new_terminal( stm, columns, rows, width, height,
			    stm->terminal_name, tt_chars , flags);
	    if ( ! ( status & 1 ) )
		{
		close_stream( stm );
		return status;
		}
	}
/*
 * Obtain user-process for new terminal if requested.
 */
	switch (flavor)
	{
#ifndef VXT_DECTERM
	case DWT$K_CREATE_DECTERM_LOGGED_IN:
	    u_new_process (isn, 1);
	    break;
	case DWT$K_CREATE_DECTERM_PROMPT:
	    u_new_process (isn, 0);
	    break;
#endif VXT_DECTERM
	case DWT$K_CREATE_DECTERM_IDLE:
	    break;
	default:
	    log_message(
		"Invalid flavor (%d) seen, should have been prechecked.\n",
		flavor);
	    close_stream( stm );
	    return FALSE;
	    break;
	}

        SetWMHints(stm, TRUE);  /* Turn on window manager hints */

	return TRUE;
}

/*
 * Relinquish stream resource.
 */

static void
destroy_stream (stm)
	STREAM *stm;
{
	streams[stm->isn] = NULL;
	XtFree( (char *)stm );
	if (--streams_active == 0)
	    run_flag = 0;	/* exit on last close */
}

/*
 * Close down a stream
 */

static void destroy_callback (w, stm, call_data)
{
close_stream (stm);
}

void
close_stream (stm)
    STREAM	*stm;
{

    Arg arglist[1];
    int to_host = 0;

/*
 * If stream already being killed, or already dead, ignore the request.
 */
	if (stm == NULL || stm->flags&STM_KILL_PENDING)
	return;

#ifdef LOGGING
	maybe_fprintf(logfile,"Destroying stream %d\n",stm->isn);
#endif

	/* close the printer */
	finish_printing(stm);
	if (stm->printer.graphics_delay_id)
	    {
	    XtRemoveTimeOut (stm->printer.graphics_delay_id);
	    stm->printer.graphics_delay_id = NULL;
	    }
	XtSetArg( arglist[0], DECwNprinterToHostEnabled, &to_host );
	XtGetValues( stm->terminal, arglist, 1 );

	if( to_host )
	    stop_read_data_from_prt(stm->terminal);

	stm->flags |= STM_KILL_PENDING;

#ifdef HYPERHELP

	if ( stm->help_context != NULL )
	{
	    DXmHelpSystemClose( stm->help_context,
				help_error,
				"Help System Error" );
	    stm->help_context = NULL;
	}
#endif /* HYPERHELP */

	p_close( stm );

	file_destroy( stm );

/*
 * Delete all the screen widgets associated with this stream.
 */
    if (stm->parent != NULL)
	{
	XtDestroyWidget (stm->parent);
	stm->parent = NULL;
	}
/*
 * Close the associated display and destroy the Xt application context.
 */
    if (stm->display != NULL)
/*	XtCloseDisplay( stm->display ); */
	close_display( stm->display );	/****** delay closing of display ******/
	/* We can't destroy app context because there is only one! */
	/* XtDestroyApplicationContext( stm->app_context ); */

/*
 * Free up memory used for copyright notice (why was it saved anyway?)
 */
    if ( stm->window_name != NULL )
	XtFree( stm->window_name );
    if ( stm->icon_name != NULL )
	XtFree( stm->icon_name );

    if ( stm->default_db_title != NULL )
	XtFree( stm->default_db_title );
    if ( stm->pty.io_transport != NULL )
	XtFree( stm->pty.io_transport );
    if ( stm->pty.io_path != NULL )
	XtFree( stm->pty.io_path );
    if ( stm->default_title != NULL )
	XtFree( stm->default_title );
    if ( stm->concealed_ans != NULL )
	XtFree( stm->concealed_ans );

#ifdef VMS_DECTERM
    /*
     * On vms, queue up actual destroy to happen after other events that
     * we want to allow to execute first in order that they release resources.
     */
	non_ast (destroy_stream, stm);
#else
    /*
     * terminate DECterm process if single-stream Ultrix DECterm
     */
     if (stm->pid > 1)
	 kill(-stm->pid, SIGHUP);
     destroy_stream (stm);
#endif VMS_DECTERM
}

/* routine to make a pixmap stolen from session manager */
static Pixmap MakePixmap (dpy, root, data, width, height)
Display *dpy;
Drawable root;
short *data;
Dimension width, height;
{
    Pixmap pid;

    pid = XCreatePixmapFromBitmapData (dpy, root, (char *)data,
	width, height, BlackPixel(dpy, DefaultScreen(dpy)),
	WhitePixel(dpy, DefaultScreen(dpy)), 1);
/*
	MWM expects a depth of 1.  Nancy may change this, but for now
        lets force 1.
	DefaultDepth(dpy, DefaultScreen(dpy)) );
*/
    return(pid);
}

/*
 * Make sure an ISN is valid and refers to an in_use stream.  Returns the
 * corresponding stream pointer or exits with error.
 */
STREAM *GetSTM(isn,who)
ISN isn;
char *who;
{	STREAM *stm;
	char msg[100];

	if (isn < 0 || isn > MaxISN)
	{
		log_message( "Illegal isn %d in call to %s\n",isn,who);
		process_exit( DECW$_BAD_ISN );
	}
	stm = streams[isn];
	return(stm);
}


/*
 * This routine returns the maximum icon size the window manager supports.
 */

getMaxIconSize(stm)
STREAM *stm;
{
    XIconSize   *icon_sizes;
    int         icon_count,
                icon_size = SMALL_ICON_SIZE; /* Default size */
    int         i, j = 0;

    if (XGetIconSizes (stm->display, XDefaultRootWindow(stm->display),
                       &icon_sizes, &icon_count))
    {
	for(i = 1; i < icon_count; i++)
        {
	    if ((icon_sizes[i].max_width >= icon_sizes[j].max_width) &&
	        (icon_sizes[i].max_height >= icon_sizes[j].max_height))
		j = i;
        }

	if ((icon_sizes[j].max_width <= 0) || (icon_sizes[j].max_height <= 0))
	{
	    XFree ((char *)icon_sizes);
	}
	else
	{
	    icon_size = icon_sizes[j].max_width;
	    XFree ((char *)icon_sizes);
	}

    }

    return(icon_size);
}


setIconPixmap(w, tag, event)
Widget w;
caddr_t tag;	/* ignored */
XEvent *event;
{
    Arg         arglist[1];
    Pixmap      new_icon_pixmap, old_icon_pixmap;
    int         max_icon_size;
    STREAM      *stm = convert_widget_to_stream(w);

    if(event->type != ReparentNotify) return;

    max_icon_size = getMaxIconSize(stm);

    if (stm->icon_size != max_icon_size)
    {
        stm->icon_size = max_icon_size;

        if(stm->icon_size < MEDIUM_ICON_SIZE)
            new_icon_pixmap = MakePixmap (stm->display,
                              XDefaultRootWindow(stm->display), small_icon_bits,
                              SMALL_ICON_SIZE, SMALL_ICON_SIZE );
        else if(stm->icon_size < LARGE_ICON_SIZE)
            new_icon_pixmap = MakePixmap (stm->display,
                              XDefaultRootWindow(stm->display), medium_icon_bits,
                              MEDIUM_ICON_SIZE, MEDIUM_ICON_SIZE );
        else
            new_icon_pixmap = MakePixmap (stm->display,
                              XDefaultRootWindow(stm->display), large_icon_bits,
                              LARGE_ICON_SIZE, LARGE_ICON_SIZE );

        XtSetArg(arglist[0], XmNiconPixmap, &old_icon_pixmap);
        XtGetValues(stm->parent, arglist, 1);

        XtSetArg(arglist[0], XmNiconPixmap, new_icon_pixmap);
        XtSetValues(stm->parent, arglist, 1);

        XFreePixmap(stm->display, old_icon_pixmap);
    }
}

/*
 * The following routine returns true if a custom icon name (either through
 * command line or resource file) has been specified for the given display
 * connection.
 *
 * We use this routine for determining whether we're allowed to override
 * with our own icon name (for example "decterm 1" "decterm 2" etc.), which
 * we only want to do if user *didn't* specify a custom one.
 */
static Boolean is_custom_icon_name_given (dpy) Display *dpy;
{
  XrmDatabase rdb;
  XrmValue rval;
  char *rtype;
  char *name_dot_res, *class_dot_res, *name, *class;
  Boolean result;
#define resource_template "%s.iconName"

  /* fetch our database, will contain resources merged from various sources */
  rdb = XtDatabase(dpy);

  /* name and class to construct the name of the resource we want */
  XtGetApplicationNameAndClass(dpy, &name, &class);

  /* make space to hold formatted resource name */
  name_dot_res = XtMalloc (sizeof(resource_template) + strlen (name));
  class_dot_res = XtMalloc (sizeof(resource_template) + strlen (class));

  /* construct fully qualified name and class of resource */
  sprintf(name_dot_res, resource_template, name);
  sprintf(class_dot_res, resource_template, class);

  /* try fetching it - if it is set then we'll return true */
  result = XrmGetResource(rdb, name_dot_res, class_dot_res, & rtype, &rval);

  /* relinquish space used */
  XtFree (name_dot_res);
  XtFree (class_dot_res);

  /* return true if custom icon name seen, false if not */
  return result;
}

/*
 * Procedure to create parent information for new stream.
 */

int create_parent (isn, display_name, name, lang_index)
	int lang_index;
	ISN isn;
	char *display_name;
	char *name;
{
	STREAM *stm = GetSTM (isn,"create_parent");
/*
 * Access the display.  This is only needed for waiting for its event flag.
 * Later, we'll rely on getting an ast.
 *
 * Also, initialize DEC toolkit.  In particular, ascertain the top-level widget
 * needed for creating all other widgets.
 */

    Arg		arglist[10];
    int		argcount = 0;
    char decterm_name[104];
    int		argc = 0;
    char	**argv;
    Pixmap	icon_pixmap;
    caddr_t	value_id;
    MrmCode  	value_type;
    int		status;

    globalref XtAppContext TEA_app_context;
    globalref Display *TEA_display;

    if (name == NULL || *name == '\0') {
	name = DECTERM_APPL_NAME;
    }

/*
 * Create a new application context for this stream
 * Open a new display (or an old one)
 */
    stm->app_context = TEA_app_context;	/* XtCreateApplicationContext(); */
    stm->display = TEA_display;

/*
 * Gather name string.
 * Call initilialization.
 * Remember various parameters for propogating to new DECterms.
 */

	if (measure_pty_flag)
	    sprintf (decterm_name,
	        "Measuring PTY speed.  Type \"RUN SCROLL\" (command won't echo)");
	else if (measure_scroll_flag)
	    sprintf (decterm_name, "Measuring windowing scroll speed WITHOUT PTY.");
	else 
	    {
/*
 * Set the default title.  We have to read the application title ("DECterm"
 * for the base product) from the UID file and then append the session
 * number to form the default title.
 * 
 * In Motif there is no resource context.  We call MrmFetchLiteral and are
 * returned a pointer to a NULL terminated string.    mtw 4-7-90
 *
 * Read the application title from the UID file.
 */

#ifdef VXT_DECTERM

	    if (strcmp( stm->pty.io_transport, VxtFileClassLATterminal) == 0 ) {
	    	status = MrmFetchLiteral(
		    s_MRMHierarchy,
		    "decterm_title_lat",
		    stm->display,
		    &value_id,
		    &value_type);
	    } else if (strcmp( stm->pty.io_transport, VxtFileClassTelnet) == 0 ) {
	    	status = MrmFetchLiteral(
		    s_MRMHierarchy,
		    "decterm_title_telnet",
		    stm->display,
		    &value_id,
		    &value_type);
	    } else if (strcmp( stm->pty.io_transport, VxtFileClassCterm) == 0 ) {
	    	status = MrmFetchLiteral(
		    s_MRMHierarchy,
		    "decterm_title_decnet",
		    stm->display,
		    &value_id,
		    &value_type);
	    } else if (strcmp( stm->pty.io_transport, VxtFileClassSerial) == 0 ) {
	    	status = MrmFetchLiteral(
		    s_MRMHierarchy,
		    "decterm_title_serial",
		    stm->display,
		    &value_id,
		    &value_type);
	    } else {
		/* Should never have taken this path */

	        status = MrmFetchLiteral(
		    s_MRMHierarchy,
		    "decterm_title",
		    stm->display,
		    &value_id,
		    &value_type);
	    }
	    if (status != MrmSUCCESS)
		{
		log_error(
		    "Unable to fetch application title from MRM\n" );
		if ( status == MrmNOT_FOUND )
		    log_error( "DECterm_title literal not found\n");
		return DECW$_CANT_FETCH_DRM_VALUE;
		}
#else VXT_DECTERM
	    if ((status = MrmFetchLiteral(
		    s_MRMHierarchy,
		    "decterm_title",
		    stm->display,
		    &value_id,
		    &value_type)) != MrmSUCCESS)
		{
		log_message(
		    "Unable to fetch application title from MRM\n" );
		if ( status == MrmNOT_FOUND )
		    log_message( "DECterm_title literal not found\n");
		return DECW$_CANT_FETCH_DRM_VALUE;
		}
#endif VXT_DECTERM
/*  I can't find the mrmcode types so far
	    if ( MrmRCType( drm_context ) != RGMrTypeChar8 )
		{
		XtFree (value_id);
		log_message(
			"DECterm title must be null-terminated ASCII\n" );
		return DECW$_BAD_DRM_VALUE;
		}
*/                                                     
/*
 * Form the title by appending the session number to the name we read.
 */
	    if ( lang_index == 15 )	/* iw_IL */
	    if ((status = MrmFetchLiteral(
		    s_MRMHierarchy,
                    "hebrew_title",
		    stm->display,
		    &value_id,
		    &value_type)) 
		== MrmSUCCESS);
#ifdef VMS_DECTERM
	    if (lang_index == 15)	/* iw_IL */
	    {
		int ltor_number, n, i;
		char ltor_string[20], rtol_string[20];

		ltor_number = ++decterm_number;

		sprintf(ltor_string, "%d", ltor_number);

		for (i = 0, n = strlen(ltor_string) - 1; n >= 0;  n--)
		    rtol_string[i++] = ltor_string[n];

		rtol_string[i] = 0;

		sprintf (decterm_name, "%s %s", value_id, rtol_string);
	    }
	    else
		sprintf (decterm_name, "%s %d", value_id, ++decterm_number );
#else
#ifdef VXT_DECTERM
{
	    char  *title_format, default_db_title[32];

	    /* For the VXT platform, default title and icon name retrieved
		from the uil file may not be iso-latin character for
		the different languages, so need to convert it to iso-latin. */

	    title_format = CSToLatin1(value_id);	    

	    /* "Save Options" saves the difference between what's in the
		DECterm database versus the default system database.  If a 
		user changes the window title and saves it, the DECterm database
		is different from the default system database, and therefore
		the difference is saved.  If a user brings up multiple windows
		using the default title, each window would have a different 
		default title.  When a user executes "Save Options", the
		DECterm database is different from the system database, but in
		this case, it is undesirable to save the difference.  So need
		to do some tricks to save, restore the correct default title.

		Here is the algorithm:
	
		stm->default_db_title = "VXT DECterm" 
		stm->default_title = the transport + "VXT DECterm" + a node
			name + the decterm window number (for example:
			"Lat VXT DECterm on GWEN 1")
		The default system database contains "VXT DECterm" 
		(stm->default_db_title).  Before a window comes up, it compares
		the title in the DECterm widget	with "VXT DECterm", if it 
		matches, it will put up the title in the format of 
		"Lat VXT DECterm on GWEN 1" (stm->default_title).  When a user
		applies "Save Options", the title in the DECterm widget is
		compared with "Lat VXT DECterm on GWEN 1", if it's the same,
		"VXT DECterm" is written to the system database.  This
		technique also applies to "Restore System Options" and 
		"Restore Options".
	    */

	    if ( strcmp( stm->pty.io_transport, VxtFileClassLATterminal ) == 0 ||
	         strcmp( stm->pty.io_transport, VxtFileClassTelnet ) == 0 ||
	         strcmp( stm->pty.io_transport, VxtFileClassCterm) == 0 ) {
	        sprintf (decterm_name, title_format, stm->pty.io_path,
			++decterm_number );
	    } else if (strcmp( stm->pty.io_transport, VxtFileClassSerial ) == 0 ) {
	        sprintf (decterm_name, title_format, ++decterm_number );
	    } else {
	        sprintf (decterm_name, "%s %d", title_format, ++decterm_number);
	    }

	    sprintf (default_db_title, "VXT DECterm");
            stm->default_db_title = XtNewString(default_db_title);
	    stm->default_title = XtNewString(decterm_name);
}
#else
	    cpystr( decterm_name, value_id );
#endif VXT_DECTERM
#endif VMS_DECTERM
	    XtFree (value_id);
/*
 * Now free up the resources we used.
	    MrmFreeResourceContext( drm_context );
 */
	    }

        stm->icon_size = getMaxIconSize(stm);

        if(stm->icon_size < MEDIUM_ICON_SIZE)
            icon_pixmap = MakePixmap (stm->display,
                          XDefaultRootWindow(stm->display), small_icon_bits,
                          SMALL_ICON_SIZE, SMALL_ICON_SIZE );
        else if(stm->icon_size < LARGE_ICON_SIZE)
            icon_pixmap = MakePixmap (stm->display,
                          XDefaultRootWindow(stm->display), medium_icon_bits,
                          MEDIUM_ICON_SIZE, MEDIUM_ICON_SIZE );
        else
            icon_pixmap = MakePixmap (stm->display,
                          XDefaultRootWindow(stm->display), large_icon_bits,
                          LARGE_ICON_SIZE, LARGE_ICON_SIZE );

/*
 * Set parameters on shell.  If user specified
 *
 *	-xrm "*.iconName:something"
 *
 * we let that prevail.  Otherwise, we set the icon name to default name,
 * for example "Decterm 2".
 */

        XtSetArg( arglist[argcount], "x", def_x);  argcount++;
        XtSetArg( arglist[argcount], "y", def_y);  argcount++;
        XtSetArg( arglist[argcount], XtNallowShellResize, TRUE);  argcount++;
	if (! is_custom_icon_name_given (stm->display))
	    {
	    XtSetArg( arglist[argcount], XtNiconName, decterm_name );
	    argcount++;
	    }
	XtSetArg( arglist[argcount], XmNiconPixmap, icon_pixmap ); argcount++;

	if (geometry_arg != NULL) {
	    XtSetArg( arglist[argcount], XtNgeometry, geometry_arg); argcount++;}

	stm->parent = XtAppCreateShell( name, DECTERM_APPL_CLASS,
	    applicationShellWidgetClass, stm->display, arglist, argcount );

/*
 * Make subsequent decterm appear at different location than previous.
 * For now, we let up to eight get created a bit down and to the right
 * of previous, then we restart them.
 */
	def_x += 30;
	def_y += 30;
	if (def_x > 100+8*30) def_x = 100, def_y = 100;

/*
 * Tell the window manager to send us delete_window messages, and set up
 * an event handler to catch nonmaskable events (including ClientMessage).
 */

	if (wm_delete_window_atom == 0)
	    wm_delete_window_atom = XInternAtom(stm->display,
		"WM_DELETE_WINDOW", False );
	XmAddWMProtocols(stm->parent, &wm_delete_window_atom, 1);
	XtAddEventHandler(stm->parent, NoEventMask, True, nonmaskable_event, 0);
	XtAddEventHandler(stm->parent, StructureNotifyMask, False,
	                  (XtEventHandler) setIconPixmap, 0);

	return TRUE;
}

/*
 * chars_from_widget sends data to the host, so the application there believes
 * the data was typed.  chars_from_widget returns 1 iff more data may be
 * sent.  If it returns 0, we assume widget_hungry will be called later.
 */
Boolean
chars_from_widget( w, stm, call_data )
    Widget w;
    STREAM *stm;
    DECwTermInputCallbackStruct *call_data;
{
Boolean result;
/*
 * When we receive characters from a widget, send them to the user application,
 * unless we're testing scrolling speed.
 */
    if (! measure_scroll_flag) {
	if ( !(result = p_write( stm, call_data->data, call_data->count ))) {
	    DECwTermStopInput( w );
	}
    }
    else result = 1;

return result;
}

/*
 * pc_resume_write()
 *
 * The PTY can now accept more data.
 */

void
pc_resume_write( stm )
    STREAM	*stm;
{
    extern resume_printer_to_host();
    DECwTermStartInput( stm->terminal );
    resume_printer_to_host(stm);
}

/*
 * pc_dead()
 *
 * The PTY has died.
 */

void
pc_dead( stm )
    STREAM	*stm;
{
    close_stream( stm );
}

/*
 * pc_read()
 *
 * Send accumulated user application output to widget.  For efficiency,
 * we coalesce all the output into a single string and do a single
 * call to the widget.
 *
 * Input:
 *	stm			which stream's data to send
 *
 * Output:
 *	data gets sent to widget
 *
 */
void
pc_read( stm, data, count )
    STREAM	*stm;
    char 	*data;
    int		count;
{
/*
 * Send all the data at once.
 */
	DECwTermPutData( stm->terminal, data, count );
}

/*
 * Copyright notice event handlers.  If a proper event occurs, remove the
 * event handlers, put the proper name in the title bar, and then
 * return.
 */

static void
copyright_event_handler(w, stm, event)
    Widget w;        
    STREAM *stm;
    XEvent *event;
{
    undo_copyright_notice( stm );
}

void
copyright_callback(w, tag, call_data)
    Widget w;        
    caddr_t tag;
    caddr_t call_data;
{
    STREAM *stm;

    stm = convert_widget_to_stream( w );
    undo_copyright_notice( stm );
}

static void
undo_copyright_notice( stm )
    STREAM *stm;
{
    Arg arglist[2];
    char *title;
    long count, status;
    XmString title_cs;
    Atom titleEncoding, XA_COMPOUND_TEXT =
	XInternAtom( XtDisplay( stm->parent ), "COMPOUND_TEXT", FALSE );

    if (stm->window_name != NULL) {

        XtRemoveEventHandler(stm->menubar, ButtonPressMask, FALSE,
			     (XtEventHandler) copyright_event_handler, stm);
        XtRemoveEventHandler(stm->terminal, ButtonPressMask, FALSE,
			     (XtEventHandler) copyright_event_handler, stm);
        XtRemoveEventHandler(stm->terminal, KeyPressMask, FALSE,
			     (XtEventHandler) copyright_event_handler, stm);

        XtSetArg( arglist[0], XtNtitle, stm->window_name);
        XtSetArg( arglist[1], XtNiconName, stm->icon_name);
        XtSetValues( stm->parent, arglist, 2 );

	if ( multi_uid || IsAsianOrHebrewTermType(stm->terminalType) ) {
	    title_cs = ( stm->window_name_encoding == XA_COMPOUND_TEXT ) ?
		   ( XmString )XmCvtCTToXmString( stm->window_name ) :
		   ( XmString )DXmCvtFCtoCS( stm->window_name, &count, &status );
	    _DECwTermSetTitle( stm->parent, stm->terminal, title_cs );
	    XmStringFree( title_cs );
	}

        XtFree( stm->window_name );
        stm->window_name = NULL;
        XtFree( stm->icon_name );
        stm->icon_name = NULL;
    }
}

/*
 * copyright notice is put into the windows title bar.  Event handlers
 * are set up to watch keyboard and mouse input.  As soon as input
 * occurs, the title bar resumes its real name and the event handlers
 * are removed.
 */

static void
do_copyright_notice( stm )
    STREAM	*stm;
{
    Arg arglist[4];
    String window_name, icon_name;
    caddr_t notice_text;
    MrmCode value_type;
    Font title_font;
    XFontStruct *font_struct;
    Dimension width, non_title_width;
    int i, notice_length, notice_width, status;
    char literal_name[20];
    int n_notices=8;	/* In future, this should be a literal in .uid file */
    
    XtSetArg( arglist[0], XtNtitle, &window_name);
    XtSetArg( arglist[1], XtNwidth, &width);
    XtSetArg( arglist[2], XtNtitleEncoding, &stm->window_name_encoding );
    XtSetArg( arglist[3], XtNiconName, &icon_name);
    XtGetValues( stm->parent, arglist, 4 );
    stm->window_name = XtNewString(window_name);
    stm->icon_name = XtNewString(icon_name);

/*
 * Put in the copyright notice.  The text of up to eight notices, in
 * descending order of size, are stored in the UID file.  We have to find
 * the length in pixels of each notice based on the font that they will be
 * drawn in, and choose the longest notice that will fit in the window.
 */

    if ( title_font == NULL )
	font_struct = NULL;
    else
	font_struct = XQueryFont( stm->display, title_font );

#if 0	/* ignore error until we figure out why this isn't working */
    if ( font_struct == NULL && verbose_flag )
	log_message( "Unable to query title font\n" );
#endif

/*
 * Get a resource context that can be used for reading values from the
 * UID file.  These will always be 8 bit null-terminated ASCII strings.
 */

    notice_text = NULL;

    for ( i = 1; i <= n_notices ; i++ )
	{
/*
 * For each copyright notice (some of which may be blank) read the notice
 * from the UID file and see if it will fit in the window.
 *
 * First, read the notice.
 */

	sprintf( literal_name, "copyright_notice_%d", i );

	if ((status = MrmFetchLiteral(
		    s_MRMHierarchy,
		    literal_name,
		    stm->display,
		    &notice_text,
		    &value_type)) != MrmSUCCESS)
	    {
		log_message(
		    "Unable to fetch copyright notice %d from DRM\n", i );
		if ( status == MrmNOT_FOUND ) {
		    log_message( "Copyright literal not found\n");
		    continue;
		}
	    }
/*
 * If the notice isn't empty, it becomes a candidate for being the notice
 * that is actually displayed.
 */

	notice_length = strlen( notice_text );
/*
 * Now find out how many pixels wide this copyright notice is and see if
 * the notice will fit.  If the XQueryFont failed, assume that each character
 * in the title is 8 pixels wide (this is because I couldn't get the
 * XQueryFont call to work in time for field test code freeze).
 */
	if ( notice_length > 0 )
	    {
	    if ( font_struct == NULL )
		notice_width = 8 * notice_length;
	    else
		notice_width = XTextWidth( font_struct, notice_text,
		    notice_length );
	    if ( notice_width <= width )
		break;
	    }
/*
 * If nothing fits, we'll use the last one on the list.
 */
	if (i != n_notices)
	    {
	    XtFree (notice_text);
	    notice_text = NULL;
	    }
	}
/*
 * At this point we've found the notice we're going to use (assuming the
 * UID file is set up correctly).  Set the title to the copyright notice.
 */
    if (notice_text)
	{
	Arg arglist[2];
	long count, status;
	XmString title_cs = DXmCvtFCtoCS( notice_text, &count, &status );
	XmStringCharSet charset;
	char *title;

	if ( title_cs )
	{
	    _DECwTermSetTitle( stm->parent, stm->terminal, title_cs );
	    XmStringFree( title_cs );  /* make sure to free it */
	}
	XtFree (notice_text);
	}
/*
 * Clean up the memory we used.

    MrmFreeResourceContext( drm_context );
 */
    if ( font_struct != NULL )
	XFreeFontInfo( NULL, font_struct, 1 );

    /* add event handlers */
    XtAddEventHandler(stm->menubar, ButtonPressMask, FALSE,
		      (XtEventHandler)copyright_event_handler, stm);
    XtAddEventHandler(stm->terminal, ButtonPressMask, FALSE,
		      (XtEventHandler)copyright_event_handler, stm);
    XtAddEventHandler(stm->terminal, KeyPressMask, FALSE,
		      (XtEventHandler)copyright_event_handler, stm);

    /* Note:  The .uil file directs the map callbacks on the pulldown menus
       to copyright_callback */
}

#if 0	/* no longer called, but preserve in case it's needed later */

void
dwt_get_optimal_size( w, width, height )
    Widget w;
    Dimension *width,*height;
{
    XtWidgetGeometry intended, reply;

    *width = 0;
    *height = 0;

    intended.request_mode = 0;  /* query both width and height */

    switch (XtQueryGeometry(w, &intended, &reply))
    {
	case XtGeometryAlmost:
	{
	    if (reply.request_mode & CWHeight)
		*height = reply.height;

	    if (reply.request_mode & CWWidth)
		*width = reply.width;
	    break;
	}

	case XtGeometryYes:
	{
	    *height = XtHeight(w);
	    *width = XtWidth(w);
	    break;
	}

	case XtGeometryNo:
	{
	    *height = XtHeight(w);
	    *width = XtWidth(w);
	    break;
	}
    }
}

#endif

/*
 * nonmaskable_event - event handler for nonmaskable events
 *
 * The only nonmaskable event we care about at this time is ClientMessage:
 * the window manager can send us a message asking us to close a window.
 */

static void nonmaskable_event(w, tag, event)
    Widget w;
    caddr_t tag;	/* ignored */
    XEvent *event;
{
    XClientMessageEvent *cm = (XClientMessageEvent *)event;
    Display *dpy = XtDisplay(w);

    if ( event->type == ClientMessage )
	{
	if (wm_protocols_atom == 0)
	    wm_protocols_atom = XInternAtom(dpy, "WM_PROTOCOLS", False);
	if (wm_delete_window_atom == 0)
	    wm_delete_window_atom = XInternAtom(dpy, "WM_DELETE_WINDOW", False);

	if ( cm->message_type == wm_protocols_atom
	  && cm->data.l[0] == wm_delete_window_atom )
	    {
	    file_exit_cb(w, tag, event);
	    }
	}
}


#ifdef VXT_DECTERM

extern int msgbox_open();
extern int msgbox_write();

/*
 * sys_announce_new_decterm sends a message announcing that a new decterm
 * is being created.
 */
sys_announce_new_decterm (io_transport, io_path)
{

if (strcmp(io_transport, VxtFileClassLATterminal) == 0)

    vxt_msgbox_write(TM_MSG_INFORMATIONAL,0,k_dt_start_decterm,2,"LAT",io_path);

else if (strcmp(io_transport, VxtFileClassTelnet ) == 0)

    vxt_msgbox_write(TM_MSG_INFORMATIONAL,0,k_dt_start_decterm,2,"Telnet",
	io_path);

else if (strcmp(io_transport, VxtFileClassCterm ) == 0)

    vxt_msgbox_write(TM_MSG_INFORMATIONAL,0,k_dt_start_decterm,2,"DECnet",
	io_path);

else if (strcmp(io_transport, VxtFileClassSerial ) == 0)

    vxt_msgbox_write(TM_MSG_INFORMATIONAL,0,k_dt_start_serial_decterm,0);

}
#endif VXT_DECTERM

/*
 * Come here when when BREAK key has been pressed.  We make sure this decterm
 * is running on a channel for which a BREAK key makes sense, such as
 * a vxt serial port.
 */
static void send_break_handler (w, stm, call_data)
    Widget w;
    STREAM *stm;
    int *call_data;
{
#ifdef VXT_DECTERM
if ( strcmp( stm->pty.io_transport, VxtFileClassSerial ) == 0 ) 
    {
    if ( send_break(NULL) <= 0 )
	log_error ("failed to send a break \n");
    }
#endif
}
