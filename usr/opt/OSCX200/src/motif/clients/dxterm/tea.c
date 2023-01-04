/* #module TEA.c "X3.0-4" */
/*
 *  Title:	TEA.c
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © Digital Equipment Corporation, 1988, 1993 All Rights       |
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
 * Terminal emulator application.  Responsible for creating associations of
 * DECterm widget, pseudo-terminal (PTY) , and user application process.
 *
 * The user application sends data to PTY, which we receive and ship
 * to the DECterm widget.  When user types, we receive the input from
 * the widget and ship it to the PTY for the user application to see
 * as terminal input.
 *
 * Author: Eric Osman, Sep. 30, 1987
 *
 * This is a modification of original PTY support code by Jerry Leichter.
 *
 *  Modification history:
 *
 * Alfred von Campe     01-Dec-1993     BL-E
 *      - Pass the transport to _DECwTermGetHostName().
 *
 * Alfred von Campe     25-Oct-1993     BL-E
 *      - Change turkish language designator to tr_TR.
 *      - Merge changes from DECWIN CMS library:
 *        o Add workaround for SET DISPLAY bug.
 *        o Fix startup problems with non-DECnet transports.
 *
 * Alfred von Campe     15-Oct-1993     BL-E
 *      - Don't remove -display from argv on non-VMS systems.
 *
 * Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 * Alfred von Campe     26-Jul-1993     BL-D
 *      - Change MrmOpenHierarchy to MrmOpenHierarchyPerDisplay for Motif V1.2.
 *
 * Alfred von Campe     08-Jun-1993     DECterm/BL-C
 *	- Pass new flag word from mailbox message to create_session() routine.
 *
 * Alfred von Campe     25-May-1993     V1.2
 *	- Remove call to vms_initialize() since it's no longer needed.
 *
 * Aston Chan		23-Apr-1993	V1.2/BL2
 *	Startup/Fullname support.  V1.2/BL1 startup problem.
 *	- Don't use CEF's, use the selection mechanism instead for startup.
 *	- Don't use sys$node, call _DECwTermGetHostName() instead for fullname
 *	  support.
 *	- Don't duplicate the parse displayname here.  Call
 *	  _DECwTermScanDisplay() instead.
 *	- Replace XtToolkitInitialize(), XtCreateApplicationContext() and
 *	  XtOpenDisplay() by XtAppInitialize() to fix problem with V1.2/BL1
 *	  where MrmOpenHierarchy() always failed.
 *	- Change TEA_display to be globaldef so that we can get rid of the
 *	  open_display() routine totally.
 *
 * Alfred von Campe     25-Mar-1993     V1.2/BL2
 *      - OSF/1 code merge.
 *
 * Aston Chan		12-Mar-1993	V1.2/BL2
 *	- Add Turkish/Greek support.
 *
 * Aston Chan		22-Jan-1993	V1.2/BL1 
 *	- Replace unneccessary use of SYS$ by LIB$ calls to avoid misuse of
 *	  Event Flags.
 *	- Change _DECWTERMSTRINGTOUPPER to *bumpy* case.
 *
 * Alfred von Campe     20-Jan-1993	Ag/BL12 (SSB)
 *      - Only compare the first 5 characters of LANG, since the AOSF/1
 *        session manager also adds the encoding (i.e., iw_IL.88598).
 *
 * DAM			10-Nov-1992	VXT V1.2?
 *	- All references to locked customizations have been removed. Obsolete.
 *
 * Alfred von Campe     14-Oct-1992     Ag/BL10
 *      - Added typecasts to satisfy Alpha compiler.
 *
 * Eric Osman		11-Sep-1992	VXT V1.2
 *	- Use streams_active instead of count_decterms
 *	- Don't create decterm if doing so would get us out of S4.
 *
 * Eric Osman		 2-Sep-1992	VXT V1.2
 *	- do more accurate determination of whether there's enough mem for
 *	  a new decterm
 *
 * Tavit Ohanian	29-Aug-1992
 *	- rework synchronization with VXT Terminal Manager.
 *
 * Aston Chan		20-Aug-1992	Post V1.1
 *	- ToLung's fix to make DECterm widget recognize locale name with
 *	  codeset.  E.g.  zh_TW.dechanyu or ja_JP.sjis etc.
 *
 * Eric Osman		11-June-1992	Sun
 *	- Some casting to make compilers happy
 *
 * Aston Chan		05-May-1992	Post V3.1/SSB
 *	- Change version announcement message to show version and compile
 *	  date/time.
 *	- Add Shai's fix of reading incorrect uid file in localized
 *	  Ultrix DECterm.  Fix is not to extend argv for xnllanguage if
 *	  language index is not set.
 *
 * Alfred von Campe     02-Apr-1992     Ag/BL6.2.1
 *      - Add support for I18N under OSF/1.
 *      - Check for -geoemtry and -size before calling XOpenDisplay().
 *
 * Aston Chan		17-Mar-1992	V3.1/BL6
 *	- Redefine DECW$DISPLAY to make cold-start of HyperHelp work.
 *
 * Dave Doucette	13-Feb-1992	V3.1/1.1
 *	- Changed logical name from DECTERM_DUPLICATE_OK to 
 *	  DECW$DECTERM_DUPLICATE_OK
 *
 * Aston Chan		22-Jan-1992	V3.1
 *	- Fix a bug when running the controller (on OSF or VXT which doesn't
 *	  have a default language) by specifying -number option without
 *	  specifying -xnllanguage.  In this case, the language is default to
 *	  be "en_US" which is a wrong assumption.
 *	- Bug fix in #ifdef for Alpha because vms_initialize() is #ifdef out.
 *	- Bug fix for create/term/display=*::0.0 not working after LOGINOUT
 *	  fix.
 *
 * Aston Chan		17-Jan-1992	V3.1
 *	- With LOGINOUT fix, we can't pass the language index by using
 *	  SYS$INPUT (part of I18n fix).  Instead, the language string is
 *	  passed by using -xnlLanguage as parameter.
 *	- "-logout" flag is also added so that DECterm controller will stop
 *	  it's own process when the very last DECterm is logged out.
 *	- Don't open display in I18n code for getting the lang_index.  Just
 *	  use the TEA_display.  In general, share the display if possible 
 *	  because opendisplay is very expensive.
 *	- Add ToLung's fix for multilingual DECterm.
 *
 * Eric Osman		20-Dec-1991	V3.1
 *	- More i18n, don't do realloc on argv.
 *
 * Aston Chan		19-Dec-1991	V3.1
 *	- Add _DECwTerm prefix for I18N entry points.
 *
 * Eric Osman		18-Dec-1991	V3.1
 *	- I18n bug, if lang_index is large due to get_lang_index routine
 *	  giving large index because no language set on server because no
 *	  session manager run there, default to a reasonable value.
 *
 * Aston Chan		17-Dec-1991	V3.1
 *	- I18n code merge
 *
 * Aston Chan		11-Dec-1991	V3.1
 *	- Merge in the I18n codes.
 *	- Remove #include <keysym.h> and extern int CvtKeycodeToModifier();
 *	  which are obsolete in the I18n versions.
 *
 * Aston Chan		27-Nov-1991	V3.1
 *	- Add #include <stdlib.h> to workaround the decc$exit problem.
 *	- Remove DUMMY_ARGS_FOR_DECC because argc and argv are now supported
 *	  by DECC
 *
 * Eric Osman	       11-Nov-1991     V3.1
 *	- Print diagnostic information about event flag cluster, to aid in
 *	  analyzing situations where decterms don't come up
 *	- Snapshot quotas at startup.
 *	- If DECTERM_CHECK_MEM is defined, turn on fake_vm.
 *
 * Alfred von Campe    05-Nov-1991     Hercules/1 BL5
 *	- Add a call to Exit() in process_exit() on non-VMS platforms.
 *      - Parse -geometry a little differently, so that dxterm will take
 *	  rowxcol instead of widthxheight.
 *
 * Alfred von Campe    6-Oct-1991     Hercules/1 T0.7
 *      - Moved some stuff in and out of #ifdef VMS sections.  This was
 *        really hard to figure out because Bill Matthews made some changes
 *        and didn't document them in here.
 *      - Removed superfluous "&" characters.
 *
 * Aston Chan		12-Sep-1991	Alpha
 *	- Change the optional argument for descptr() to compulsory.  If
 *	  strlen of first parameter is desired, just pass zero for the 2nd
 *	  parameter.
 *	- AccVio in LIB$GETJPI().  Should not use constant string when 
 *	  defining username, which is the return descriptor from lib$getjpi.
 * Aston Chan		7-Sep-1991	Alpha
 *	- Dummy argv and argc added because of the restriction of argv,argc
 *	  not supported in CRTL yet.  It is conditionalized by
 *	  DUMMY_ARGS_FOR_DECC
 * Aston Chan		1-Sep-1991	Alpha
 *	- Add <> and .h to #include's.  Complained by DECC compiler Release 10.
 *	- #pragma builtins  not supported by Alpha.  It is VAX/VMS specific.
 *
 * Eric Osman		13-Jun-1991
 *	- Hash language into event cluster name so separate controller is
 *	  used for each language.
 *
 * Eric Osman		3-Jun-1991	V3.0
 *	- Instead of checking for mode==interactive, check for
 *	  getenv("DECTERM_DUPLICATE_OK").  This allows interactive debugging
 *	  to be diagnosed properly.
 *	- Put name of cluster in log file.
 *	- Look up decw$display in process name table before job table.
 *	- Replace old get_display_name with get_display_info that correctly
 *	  handles case of sys$output pointing to a wsa or a node::n.n.  This
 *	  fixes problem of decterms not coming up on multi-head systems.
 *
 * Eric Osman		15-May-1991
 *	- Call setup_exit_handler so that log file is only written on
 *	  crashes, not on every error.
 *
 * Eric Osman		 7-Mar-1991
 *	- Enable catching of access violations, so log file can be written.
 *
 * Bob Messenger	 3-Oct-1990	X3.0-7
 *	- Don't set EF_CTRLR_STARTING here, since it is set by DECW$TERM_PORT.
 *
 * Bob Messenger	26-Aug-1990	X3.0-6
 *	- Add hooks for debugging: debug_create_decterm() and debug_show_vm().
 *
 *  Michele Lien    24-Aug-1990 VXT X0.0
 *  - Modify this module to work on VXT platform. Change #ifdef VMS to
 *    #ifdef VMS_DECTERM so that VXT specific code can be compiled under
 *    VMS or ULTRIX development environment.
 *
 * Bob Messenger	32-Jul-1990	X3.0-6
 *	- Fix status returned to the calling process by process_exit.
 *	- Work around the "zombie controller" bug, where the common event flag
 *	  cluster never gets deleted, apparently because the process that owns
 *	  it never exits.  The workaround is to run decw$terminal.exe
 *	  interactively; this bypasses the check for an existing controller.
 *	- Change message from "PTY exiting" to "DECterm exiting".
 *
 * Mark Granoff		13-Jun-1990	X3.0-4
 *	- Declare hash() as static.
 *
 * Mark Granoff		12-Jun-1990	X3.0-4
 *	- Merged the previous two mods (separate generations) into this
 *	  single generation.
 *
 * Mark Granoff		 1-Jun-1990	X3.0-3
 *	- Fixed CEF naming problem.
 *
 * Mark Woodbury	25-May-1990	X3.0-3M
 *	- Motif update
 *
 * Mark Granoff		 15-May-1990	X3.0-3
 *	- Added code for Common Event Flag-based synchronization.
 *	- Added some routines (gleaned from the loginout code) for returning
 *	  WSAn device information (e.g. nodename, server & screen numbers), and
 *	  a routine to return the nodename of the system.
 *	- Added a condition to the main event processing loop to check the
 *	  non_ast_queue list head; if it does not equal itself then the
 *	  controller should continue to operate.  This (hopefully) eleviates
 *	  the reported mailbox race condition for a dieing controller.
 *
 * Mark Granoff		 5-Mar-1990	X3.0-1
 *	- Added code for controller to name itself, to avoid DUPLNAM
 *	  errors.
 *
 * Bob Messenger	16-Aug-1989	X2.0-19
 *	- Don't check for duplicate controller.
 *
 * Bob Messenger	20-Jul-1989	X2.0-16
 *	- Define LANGUAGE_SWITCHING_TOOLKIT.
 *
 * Bob Messenger	11-Jul-1989	X2.0-16
 *	- Before calling vms_initialize, see if decw$display is undefined
 *	  and sys$output is in node::x.y format.  If so, set decw$display
 *	  to sys$output and set sys$output to sys$error.
 *
 * Bob Messenger	 7-Jun-1989	X2.0-13
 *	- Make the call to u_prime_mailbox conditional on VMS.
 *
 * Bob Messenger	29-May-1989	X2.0-13
 *	_ Changes needed to prevent multiple controllers from being
 *	  created, especially at system startup time.  Announce existence
 *	  right after we're started instead of at the start of the main
 *	  loop, and if we terminate before the main loop reply to any requests
 *	  that are in the mailbox.
 *	- Add a log_message routine to replace the fprintf's on stderr.  This
 *	  allows mesages to be flushed so they're not lost if the controller
 *	  crashes.
 *	- Don't use the TEST macro, which does a hard exit.
 *
 * Bob Messenger	21-May-1989	X2.0-12
 *	- Make the language switching change conditional on
 *	  LANGUAGE_SWITCHING_TOOLKIT, which will be undefined until the
 *	  toolkit lets us specify the uid file name as just DECW$TERMINAL.
 *
 * Bob Messenger	19-May-1989	X2.0-11
 *	- Language switching support: don't include directory or file type
 *	  in uid file name.
 *	- Workaround for problem with spawn.
 *
 * Bob Messenger	16-May-1989	X2.0-10
 *	- Fix argc argument to open_display.
 *
 * Bob Messenger	 8-May-1989	X2.0-10
 *	- Get UID file from DECW$SYSTEM_DEFAULTS: instead of SYS$LIBRARY:.
 *	- Initialize parsing flags correctly (don't depend on them being
 *	  initialized to zero).
 *	- Change all printf calls to fprintf on stderr.
 *	- Call XtOpenDisplay before MrmOpenHierarchy, to support language
 *	  switching.
 *
 * Bob Messenger	30-Apr-1989	X2.0-9
 *	- Convert to new creation request format.
 *
 * Bob Messenger	10-Apr-1989	X2.0-6
 *	- Use status codes from decw$terminalmsg
 *	- Change create() to create_session().
 *	- Call u_announce_existence after printing startup message (may
 *	  help timing problems during initialization).
 *	- Exit with status code on VMS.
 *	- All exit calls go through process_exit.
 *
 * Bob Messenger	 7-Apr-1989	X2.0-6
 *	- Convert to new widget bindings (DECwTerm instead of DwtDECterm).
 *
 * Tom Porcher		20-Oct-1988	X0.5-4
 *	- Exit with correct (zero) status.
 *	- Strip directory name from argv[0] on Ultrix.
 *
 * Karen Brouillette	19-Oct-1988	X0.5-3
 *	- add call to vms_initialize.   This is needed now that the
 *	  session manager creates this process without running loginout.
 *	  vms_initialize will define the logical sys$login to be the
 *	  users directory as specified in the UAF record.  The logical
 *	  name decw$user_defaults is defined to sys$login by default
 *
 * Tom Porcher		 5-Aug-1988	X0.4-43
 *	- removed startup message about "DBG> ...".
 *
 * Tom Porcher		 5-Aug-1988	X0.4-42
 *	- fixed -L to take last arg.
 *	- fixed -xrm to allow multiple -xrm options.
 *
 * Tom Porcher		 9-Jul-1988	X0.4-35
 *	- re-arranged main loop so that it checks run_flag before blocking.
 *	- change -Xrm to -xrm.
 *	- parse -S as "-Sccnnn" instead of "-S ccnnn".
 *
 * Tom Porcher		 6-Jul-1988	X0.4-35
 *	- add open_display() to open display only once.
 *
 * Tom Porcher		 6-Jul-1988	X0.4-35
 *	- Remove DwtDECApplication() until toolkit fixed.
 *
 * Tom Porcher		30-Jun-1988	X0.4-34
 *	- add termporary work-around for DECterm ACCVIO on "Quit":  close
 *	  displays from main loop.
 *	- fix Ultrix command line parsing.
 *
 * Tom Porcher		21-Jun-1988	X0.4-32
 *	- remove XtToolkitInitialize() and XtOpenDisplay.  These are now done
 *	  on a per-DECterm basis in ptysub.c.
 *	  Note that this means that argv and argc are no longer passed to
 *	  the Xtoolkit.
 *	- Replaced XtMainLoop() with code that processes X events first.
 *
 * Tom Porcher		 8-Jun-1988	X0.4-31
 *	- merged in code from TEA_ultrix.c.
 *
 * Tom Porcher		 7-Jun-1988	X0.4-31
 *	- used DECTERM_APPL_xxx macros.
 *
 * Eric Osman		24-May-1988	X0.4-28
 *	- Use XtAddInput and XtMainLoop.
 *
 * Tom Porcher		10-May-1988	X0.4-26
 *	- Added call to pt_is_full() in inputCallback.
 *
 * Eric Osman		 5-May-1988	X0.4-13
 *	- Support for new DwtDECtermPort
 *
 * Peter Sichel         12-Apr-1988     X0.4-7
 *      - changed to get UID file from
 *         SYS$LIBRARY:decw$terminal.uid      (VMS)
 *         /usr/lib/DECterm                  (Ultrix)
 *      added message with "printf" to explain this if file is not found
 *
 * Peter Sichel         31-Mar-1988     X0.4-3
 *      - Added DRM initialization for using UIL created setup widgets
 *        UID file is assumed to be in SYS$LIBRARY:decterm.uid
 *        this needs to be upgraded for Ultrix
 *
 * Tom Porcher		 5-Apr-1988	X0.4-7
 *	- Changed XtInitialize to XtToolkitInitialize/XtOpenDisplay.
 *	- Added display_name parameter to create().
 *	- reduced the number of calls to create() to two, and removed
 *	  general_create_new_emulator(), which was a synonym.
 *
 * Eric Osman		30-Mar-1988	X0.4-3
 *	- Add fdf routine for calling in debugger to set
 *	  free_decterm_flag.  fdf will be available as a symbol even if
 *	  non-debugging version is run with /DEBUG
 *
 * Tom Porcher		21-Mar-1988	X0.4-3
 *	- Added name parameter to create().
 *
 * Tom Porcher		10-Mar-1988	X0.4-2
 *	- Added setup_file and customization parameters to create().
 *
 * Tom Porcher		15-Jan-1987	X0.3-2
 *	- Changed initial name and class to "decterm" and "DECterm"
 *	  so that resources work.
 *
 * Eric Osman		6-Jan-1987	X0.3
 *	Put in global free_decterm_flag, which defaults to 0, but if set
 *	to 1 (in debugger, for instance), means to create a free DECterm.
 * Tom Porcher		3-Jan-1987	X0.3
 *	Made input callback conform to DECwindows callback format.
 *
 * Eric Osman		1-Dec-1987	X0.3
 *	Add support for DECW$CREATE_DECTERM
 *
 * Tom Porcher         30-Nov-1987	X0.3
 *	changed "Xutil" to "Xutil.h"
 *
 * Tom Porcher         12-Nov-1987	X0.3
 *	Removed "focus_handler()".
 *
 * Tom Porcher          5-Nov-1987
 *      Added changes to support *new* XLIB (BL5) way of using event flags.
 */

#include "mx.h"
#include <X11/Xutil.h>

#ifndef VMS_DECTERM
#ifdef VXT_DECTERM
#include <process.h>
#include <semaphore.h>
#include "file.h"
#include <stdlib.h>
#include <vxtio.h>
#include "vxtdecterm.h"
#include <lomem.h>
#include "msgboxconstants.h"

#else
#include <stdlib.h>
#include <sys/file.h>
#include <signal.h>
#include <strings.h>

extern void SYS_reapchild();

#endif VXT_DECTERM
#else
#include <ssdef>
#include <iodef>
#include <descrip>
#include <lnmdef>
#include <dvidef>
#include <dcdef>
#include <jpidef>
#include <psldef>
#include <syidef>

#ifndef DT$_DECW_PSEUDO
#define DT$_DECW_PSEUDO 7	/* from wsdrvdef.h */
#endif VMS_DECTERM

#define IO$M_WS_DISPLAY 64
#define DECW$C_WS_DSP_NODE 1
#define DECW$C_WS_DSP_TRANSPORT 2
#define DECW$C_WS_DSP_SERVER 3
#define DECW$C_WS_DSP_SCREEN 4
#define CONTROLLER_IMAGE_IDENT	"DECW"	/* Use first four chars of controller image name */

#endif

#define MAX_USERNAME_LEN   16
#define MAX_NODE_NAME_LEN  1024
#define MAX_SERVER_LEN     20
#define MAX_SCREEN_LEN     20
#define MAX_TRANSPORT_LEN  100
#define MAX_LANG_LEN	   20

/* error messages from decw$terminalmsg.msg */

#ifdef VMS_DECTERM

globalvalue DECW$_BAD_REQUEST, DECW$_CANT_OPEN_DRM, DECW$_CANT_OPEN_DISPLAY,
	    DECW$_CANT_CREATE_ATOM;

#define SUCCESSFUL_PROCESS_EXIT_STATUS SS$_NORMAL

#else

#define DECW$_BAD_REQUEST	0
#define DECW$_CANT_OPEN_DRM	0
#define DECW$_CANT_OPEN_DISPLAY	0
#define DECW$_CANT_CREATE_ATOM  0

#define SUCCESSFUL_PROCESS_EXIT_STATUS 0

#endif

Atom controller_atom = None;
Window controller_window = NULL;

/*
 * external functions
 */
extern void tea_register_callbacks();

/*
 * predefine
 */
static Bool check_duplicate();

static int tea_io_error_handler();
int x_error_handler();

/*
 * Global Data
 */

globaldef char run_flag=0;
globaldef char	attach_flag=0,

#ifdef VXT_DECTERM
		login_flag = 0,
#else
		login_flag=1,
#endif
#ifdef VMS_DECTERM
		verbose_flag=1,
#else
		verbose_flag=0,
#endif
		mbx_flag=0,
		measure_pty_flag=0,
		measure_scroll_flag=0;
#ifdef VMS_DECTERM
globalref int non_ast_queue[];
#endif
int logout_flag=0;	/* flag to see if we need to delprc() ourself
			 * when the controller is end.
			 */

#ifdef LOGGING
/* globalref char logging_flag = 0; */
globalref char logging_flag;
#endif

#ifdef VMS_DECTERM
globaldef int tea_efn = 0;
extern short mailbox_channel;
extern IOSB mailbox_iosb;
extern create_decterm_request_union mailbox_buffer;
extern int mailbox_ef;
#endif

static char ext_argv_1[13] = "-xnllanguage";
static char ext_argv_2[100] = "";
static int ext_lang_index = 0;	/* this lang_index will be passed to ptysub */

/* following is from dectermport.c */
#ifdef VMS_DECTERM
#define env_variable    "XNL$LANG"
#else
#define env_variable    "LANG"
#endif VMS_DECTERM
#define n_langs 30
/* Every entry in this table should be 5 characters long, since we later do
 * a strncmp(lang, lang_table[i], 5)
 */
static char *lang_table[n_langs] =
{
"en_US",	/* 1  American English */
"en_AU",	/* 2  Australian  */
"de_AT",	/* 3  Austrian */
"fr_BE",	/* 4  French (Belgium) */
"nl_BE",	/* 5  Dutch (Belgium) */
"en_GB",	/* 6  British  English */
"fr_CA",	/* 7  Canadian French  */
"zh_CN",	/* 8  Chinese-China */
"zh_TW",	/* 9  Chinese-Taiwan */
"da_DK",	/* 10  Danish */
"nl_NL",	/* 11  Dutch */
"fj_FJ",	/* 12  Fiji */
"fi_FI",	/* 13  Finnish */
"fr_FR",	/* 14  French */
"de_DE",	/* 15  German */
"iw_IL",	/* 16  Hebrew */
"it_IT",	/* 17  Italian */
"ja_JP",	/* 18  Japanese */
"ko_KR",	/* 19  Korean */
"en_NZ",	/* 20  New Zealand */
"no_NO",	/* 21  Norwegian */
"en_PG",	/* 22  Papua New Guinea */
"pt_PT",	/* 23  Portuguese */
"es_ES",	/* 24  Spanish */
"sv_SE",	/* 25  Swedish */
"fr_CH",	/* 26  Swiss French */
"de_CH",	/* 27  Swiss German */
"it_CH",	/* 28  Swiss Italian */
"gr_GR",	/* 29  Greek */
"tr_TR",	/* 30  Turkish */
};

globaldef XtAppContext TEA_app_context;

#ifndef VMS_DECTERM
globaldef char console_flag = 0;
globaldef char login_shell_flag = 0;
globaldef char *getty_dev = 0;
globaldef char *slave_pty = 0;
globaldef char **cmd_to_exec = 0;

globaldef int dummy_fd_1 = 0, dummy_fd_2 = 0;
#endif

globaldef char *geometry_arg = 0;

/* DRM */
globaldef MrmHierarchy s_MRMHierarchy;    /* DRM database id */
globaldef MrmType *dummy_class;           /* and class var */

globaldef Boolean multi_uid;

/*
 * specify where to find UID file for VMS and Ultrix
 */

static char *db_filename_vec[2];         /* DRM hierarchy file list */

int max_decterms_seen = 0;		/* high water mark */
int mem_estimate = 0;			/* estimated bytes needed per decterm */
globaldef char def_sys_db_is_set=False;	/* 1 = defaults read from DECterm
					       widgets are put into xrm system
					       default. This shoud only be
					       done once */

#if defined (VMS_DECTERM) || defined (VXT_DECTERM)
globaldef int free_decterm_flag=0;
#else
globaldef int free_decterm_flag=1;
#endif

#ifdef VXT_DECTERM
/* must be first code piece */
vxtdecterm_main(argc, argv)
int argc;
char *argv[];
{
    void term_main();
 
    term_main();
}
#endif

fdf(){free_decterm_flag=1;}

int max_mem_decterms;   /* if non-0, number of DECterms there's memory for. */
globaldef Display *TEA_display = NULL;
static Display *closed_displays[] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL};

Widget dummy_shell = NULL;

void
close_display( display )
    Display *display;
{
    int i;

/* THIS IS A NO-OP because of open_display()
    for ( i=0; i<XtNumber(closed_displays); i++) {
	if (closed_displays[i] == NULL) {
	    closed_displays[i] = display;
	    break;
	}
    }
*/
}

static void
check_displays()
{
    int i;

    for ( i=0; i<XtNumber(closed_displays); i++) {
	if (closed_displays[i] != NULL) {
	    XtCloseDisplay( closed_displays[i] );
	    closed_displays[i] = NULL;
	}
    }
}


#ifdef VMS_DECTERM
/*
 * Define a logical name in LNM$PROCESS.
 */

static void deflog( name, value )
    char *name, *value;
{
    $DESCRIPTOR( name_desc, "" );
    $DESCRIPTOR( lnm_process, "LNM$PROCESS" );
    struct
	{
	short length, code;
	char *buffer;
	short *return_address;
	int end_list;
	} item_list;
    int status;

    name_desc.dsc$a_pointer = name;
    name_desc.dsc$w_length = strlen( name );
    item_list.length = strlen( value );
    item_list.code = LNM$_STRING;
    item_list.buffer = value;
    item_list.end_list = 0;
    status = sys$crelnm( 0, &lnm_process, &name_desc, 0, &item_list );
    if (!(status&1))
	{
	log_message ("In deflog, sys$crelnm failed to define \"%s\" as \"%s\", status = %d\n", name, value, status);
	process_exit (status);
	}
    else log_message ("In deflog, sys$crelnm defined \"%s\" as \"%s\"\n", name, value);
}

/*
 * get_screen_info takes a string describing a screen, and parses out
 * the individual parts.
 *
 * We assume the screen descriptor string is of one of these forms:
 *
 *	node::server.screen	(In all of these, single colon is o.k. too)
 *
 *	node::server		(We assume screen 0)
 *
 * The node part itself might be quite wierd, even containing colons and
 * dots, for instance we might have this tender morsel:
 *
 *	123.456::777.2::0.1
 *
 * We look for colons from the right, so the above example we'd consider to be
 * node "123.456::777.2", server "0", and screen "1".
 */
static void get_screen_info (display_name, node_name, server_num, screen_num,
			     transport)
    char *display_name, *node_name, *server_num, *screen_num, *transport;
{
    int i;
    int display_len;
    int status;
    int server, screen;
    char *cur_node;

    display_len = strlen (display_name);

    /* This will parse the display_name, and return the server and screen
     * number in one shot.
     */
    status = _DECwTermScanDisplay(display_name, node_name, &server, &screen);

    if (FAILED(status))
	process_exit(status);

    /* We should call _DECwTermGetHostName() to expand the node name
     */
    if (FAILED(status = _DECwTermGetHostName(&cur_node, node_name, NULL)))
    {
	log_message("Error return from _DECwTermGetHostName.  Status = %x\n",
		    status);
	process_exit(status);
    }

    strncpy (node_name, cur_node, MAX_NODE_NAME_LEN);

    XtFree(cur_node);

    /*
     * Default transport to be DECNET
     */

    strcpy( transport, "DECNET" );

    /*
     * Convert server and screen number to string
     */
    sprintf(server_num, "%d", server);
    sprintf(screen_num, "%d", screen);

    log_message (
	"Parsed \"%s\" into node=%s, server=%s, screen=%s, transport=%s\n",
	display_name, node_name, server_num, screen_num, transport);
}

/*
 * get_screen_info_from_wsa takes the name of a wsa device and gets info about
 * what specific screen on the network the device actually points to.
 */
static void get_screen_info_from_wsa (wsa_name, node_name, server_num,
    screen_num, transport)
    char *wsa_name, *node_name, *server_num, *screen_num, *transport;
{
int chan;
int status;
int ret_len;
int wsa_desc[2] = {strlen (wsa_name), wsa_name};

status = sys$assign (wsa_desc, &chan, 0, 0);
if (!(status&1))
    {
    log_message ("sys$assign(\"%s\") returned bad status = %d\n", wsa_name,
	status);
    process_exit (status);
    }

get_pseudo_param(chan, DECW$C_WS_DSP_NODE, 
		    node_name, MAX_NODE_NAME_LEN, &ret_len);
get_pseudo_param(chan, DECW$C_WS_DSP_SERVER,
                     server_num, MAX_SERVER_LEN, &ret_len);
get_pseudo_param(chan, DECW$C_WS_DSP_SCREEN,
                     screen_num, MAX_SCREEN_LEN, &ret_len);
get_pseudo_param(chan, DECW$C_WS_DSP_TRANSPORT,
		     transport, MAX_TRANSPORT_LEN, &ret_len);

/* We should call _DECwTermGetHostName() to expand the node_name only if 
 * the transport is DECNET
 */
{
    char *cur_node = NULL;
    int node_len;

    /* don't forget to free cur_node after use
     */
    if (strncmp(transport, "DECNET", MAX_TRANSPORT_LEN) == 0)
    {
        if (FAILED(status = _DECwTermGetHostName(&cur_node, node_name, NULL)))
        {
	    log_message("_DECwTermGetHostName error.  Status = %x\n", status);
	    process_exit(status);
        }

        strncpy (node_name, cur_node, MAX_NODE_NAME_LEN);
    }

    node_len = strlen (node_name);

    if (cur_node) XtFree(cur_node);

    if (node_len >= 2 && strcmp ("::", node_name + node_len - 2) == 0)
	    node_name[node_len - 2] = 0;
}

status = sys$dassgn (chan);
if (!(status&1))
    {
    log_message ("$dassgn(%d) returned bad status = %d\n", chan, status);
    }
}

/*
 * This routine first makes sure decw$display points to something.  If not,
 * it points it to whatever passed_in_dpy says.  Note that the SYS$OUTPUT and
 * SYS$ERROR are no longer pointing to WSAn info after the LOGINOUT fix.
 * The WSAn info is passed from DECTERMPORT.C by -display option.
 *
 * Then, the routine sees if decw$display now points to a wsa device or
 * a string.  If a device, we ask the system for the node name , screen number,
 * and server number constituents.
 *
 * if a string, we treat it as something like "node::server.screen" and
 * parse out the individual parts.
 */

static void get_display_info(node_name, server_num, screen_num, transport,
			     passed_in_dpy)
    char *node_name, *server_num, *screen_num, *transport, *passed_in_dpy;
{
    char *s,*s1,*s2,*s3;
    int device_class, device_type, class_status, type_status;
    $DESCRIPTOR( ws_name, "" );

    /* if passed_in_dpy is not NULL, then it should be used instead of
     * DECW$DISPLAY.  -display always has the priority.  In our case of
     * $create/term, we always pass the display (either foo::x.y or wsa device 
     * name by -display option which means passed_in_dpy is always non-zero.
     */

    if ( *passed_in_dpy != 0 ) {
	s = passed_in_dpy;
	   /* make sure DECW$DISPLAY is defined in this process so that
	    * cold starting Bookreader from HyperHelp will work.
	    */
	deflog("DECW$DISPLAY", s);
    } else {
        s = getenv( "DECW$DISPLAY" );
    }

    /* s == NULL only if passed_in_dpy is NULL and DECW$DISPLAY is NULL
     * which is no display is set or specified.
     */
    if ( s == NULL ) {
	s = "";
    }

    ws_name.dsc$a_pointer = s;
    ws_name.dsc$w_length = strlen(s);

    class_status = lib$getdvi(&DVI$_DEVCLASS, 0, &ws_name, &device_class, 0, 0);

    if (FAILED (class_status))
	log_message("lib$getdvi failed in get_display_info, class_status = %x\n",
		    class_status);

    type_status = lib$getdvi( &DVI$_DEVTYPE, 0, &ws_name, &device_type, 0, 0);

    if (FAILED (type_status))
	log_message("lib$getdvi failed in get_display_info, type_status = %x\n",
		    type_status);

    if ( type_status == SS$_NORMAL && class_status == SS$_NORMAL &&
	 device_class == DC$_WORKSTATION && device_type == DT$_DECW_PSEUDO )
         get_screen_info_from_wsa (s, node_name, server_num, screen_num,
				   transport);
    else get_screen_info (s, node_name, server_num, screen_num, transport);
}

/*
 * When a message arrives from the outside world, handle it.
 * We queue the request to happen at non-ast level, lest we unknowingly
 * make some window call while another was interrupted.
 *
 * Input:
 *	tag		AST parameter
 *
 * Output:
 *			Queued message.
 */
void message_interrupt (tag)
{
	extern void respond_to_message ();

	non_ast (respond_to_message, tag);
}

/*
 * Come here at non-interrupt level to handle message, and to queue receipt
 * of new one.
 *
 * Input:
 *	dummy	AST parameter (ignored)
 *
 * Output:
 *			appropriate action taken
 */
void respond_to_message (dummy)
{
	int status;
	ISN isn;
	create_decterm_request *old_msg;
	new_create_decterm_request *new_msg;
	char *msg_buffer;
	void send_error ();
	
/*
 * Fill in the guard byte, to ensure that there is a terminator for the
 * null terminated strings in the message
 */
	mailbox_buffer.buffer[CreationBufferMax] = '\0';

/*
 * Dispatch according to message type.  NOTE:  For now, we only honor
 * the "IDLE" request.  In particular, we don't honor "LOGGED_IN", because
 * if we as the DECterm controller are logged in as SYSTEM, we would give
 * privileges to anyone (by letting them log in automatically as SYSTEM).
 */
	if ( ! FAILED( mailbox_iosb.status ) )
	switch (mailbox_buffer.type)
	{
	case DWT$K_CREATE_DECTERM_IDLE:
/*	case DWT$K_CREATE_DECTERM_LOGGED_IN:
	case DWT$K_CREATE_DECTERM_PROMPT:
*/
		old_msg = &mailbox_buffer.old;

		status = create_session(old_msg->type,
				 "",			/*  msg->display, someday */
			         DECTERM_APPL_NAME,	/*  msg->name, someday    */
			         old_msg->setup_file,
			         old_msg->customization,
				 0,
				 ext_lang_index,0,0,0,&isn);
		if (status == TRUE)
		    {
		    void send_terminal_name ();
		    send_terminal_name (old_msg->reply_adr, isn);
		    }
		else send_error (old_msg->reply_adr, status);
		break;

	case DWT$K_NEW_CREATE_DECTERM:

		new_msg = &mailbox_buffer.new;
		msg_buffer = mailbox_buffer.buffer;

/*
 * Do some checking in case we're sent a bogus message
 */

		if ( new_msg->length != mailbox_iosb.count
		  || new_msg->customization_offset < 0
		  || new_msg->customization_offset >= new_msg->length
		  || new_msg->setup_file_offset < 0
		  || new_msg->setup_file_offset >= new_msg->length
		  || new_msg->tt_chars_offset < 0
		  || new_msg->tt_chars_offset >= new_msg->length )
		    {
		    send_error(new_msg->reply_adr, DECW$_BAD_REQUEST );
		    break;
		    }
		status = create_session(DWT$K_CREATE_DECTERM_IDLE,
				"",			/* display */
				DECTERM_APPL_NAME,	/* name */
				new_msg->setup_file_offset ?
				    &msg_buffer[ new_msg->setup_file_offset ] :
				    0,
				new_msg->customization_offset ?
				    &msg_buffer[ new_msg->customization_offset ] :
				    0,
				new_msg->tt_chars_offset ?
				    &msg_buffer[ new_msg->tt_chars_offset ] :
				    0,
				ext_lang_index,0,0,new_msg->flags,&isn);

		if (status == TRUE)
		    {
		    void send_terminal_name ();
		    send_terminal_name (new_msg->reply_adr, isn);
		    }
		else send_error (new_msg->reply_adr, status);
		break;

	default:

		send_error (new_msg->reply_adr, DECW$_BAD_REQUEST);
		break;

	}
/*
 * Allow new message.
 */

	if FAILED (status = sys$qio(mailbox_ef, mailbox_channel, IO$_READVBLK,
		&mailbox_iosb, message_interrupt, 0, &mailbox_buffer,
		CreationBufferMax, 0, 0, 0, 0 ) )
	    log_message(
		    "Can't issue mailbox read, status is %x\n", status );
}

/*
 * Procedure to send name of created terminal back to caller.
 *
 * Input:
 *	reply_adr		unit number of mailbox to send message to
 *	isn			stream whose terminal is to be reported
 */
void send_terminal_name (reply_adr, isn)
{
	STREAM *stm = GetSTM (isn, "send_terminal_name");
	create_decterm_reply reply;

	reply.type = DWT$K_TERMINAL_NAME;
	strcpy (reply.terminal_name, stm->terminal_name);

	u_send_message (reply_adr, sizeof (create_decterm_reply), &reply);
}

/*
 * Procedure to send error back to caller.
 *
 * Input:
 *	reply_adr		unit number of mailbox to send message to
 *	error			error code to send back
 */
void send_error (reply_adr, error)
{
	create_decterm_reply reply;

	reply.status = error;
	reply.type = DWT$K_ERROR;

	u_send_message (reply_adr, sizeof (create_decterm_reply), &reply);
}
#endif VMS_DECTERM

/*
 * Entry point from terminal menu for creating new emulator.
 */
#ifndef VXT_DECTERM
int create_new_emulator ()
{
	return (create_session(DWT$K_CREATE_DECTERM_IDLE, "", DECTERM_APPL_NAME,
				     NULL, NULL, NULL, ext_lang_index,0,0,0,0));
}
#else

static int open_tm_socket();
static int close_tm_socket();
static room_for ();
static char lomem ();

/* terminal manager socket structure */

typedef struct tm_socket_struct {
int  sfd;
XtInputId  socket_id;
tm_param_list tm_p_list;
}tm_s_struct;

int create_new_emulator ( tm_socket )
tm_s_struct *tm_socket;
{
	tm_param_list *tm_p_l;
	int nb;
	char *transport, *path, *resource;
	int fd_count;

	tm_p_l = &tm_socket->tm_p_list;

	/* get the parameters from the terminal manager */
	nb = read(tm_socket->sfd, tm_p_l, sizeof( tm_param_list));

	if ( nb <= 0 )
	{
	    /* Some read error occurred.  close down all DECterm connections
	     * and kill DECterm process
             */

	    close_tm_socket(tm_socket);

	    if (nb < 0) {
	      run_flag = 0;
	    } else {
	      if (open_tm_socket() < 0)
		{
	    	  log_error(" cannot open socket to TM\n");
		}
	    }
	}
	else if (nb != sizeof( tm_param_list))
	{
	    log_error("error: did not read in the full list of TM parameters \n");
	}
	else {
	    int streams_active;
	    transport = tm_p_l->io_transport;
	    path = tm_p_l->io_path;
	    resource = tm_p_l->resource_list;

       /*
	* We avoid creating a DECterm window if memory low.  We want to
	* avoid a situation where user creates 5 decterms, is told there's
	* no room for another, then 4 are deleted, and user *still* can't
	* create another !  This happened when we merely trusted
	* Vxt*PhysMemState which didn't take lookaside lists into account
	* which is where memory goes when it has been malloced and then
	* freed.
	*
 	* To solve this problem, we distinguish between decterms created
	* that are a new high water mark, and those not. For new high water
	* mark, we assume we can ask mem mgr how much memory is left,
	* to decide whether there's room for another decterm.  Below
	* high water mark, we assume we've got enough memory on our
	* own lookaside lists for the decterm.
	*
	* For new high water mark case, we ask mem mgr how much memory is
	* available before and after this decterm so we have an estimate
	* of what a decterm costs.  To try to minimize false pessimism
	* due to possibility of simultaneous non-decterm allocations, we
	* use the minimum observed amount of memory required for new high
	* water mark decterms as our estimate.
	*
	* This scheme can fail in certain cases, for example if user
	* ups their number of lines-off-top before creating new decterms,
	* or if nasty simultaneous non-decterm allocations use up the
	* memory.
	*
	* NOTE:
	*
	*	The first decterm seems to require about 3 times more mem
	*	than subsequent ones, so we don't start recording how much mem
	*	is needed until we're creating the second decterm.
	*
	* So, here's the basic algorithm:
	*
	* if (new high water mark)
	*	if (per-decterm memory estimate known yet)
	*		if (allocating this estimate would make memory low)
	*		    don't create decterm
	*		else
	*		    if (creation succeeds)
	*			remember new estimate if lower
	*		    else
	*			keep old estimate
	*	else
	*		if (memory already low)
	*		    don't create decterm
	*		else
	*		    if (creation succeeds)
	*			store amount of mem used for estimate
	*		    else
	*			leave estimate unknown
	*
	* else
	*	create decterm
	*/
	    if (streams_active >= max_decterms_seen)
		if (mem_estimate)
		    if (! room_for (mem_estimate*1024))
			{   /* high water, no room */
			printf (
"DECTERM:  No creation because we wouldn't be in S4 if we allocated %d kb\n",
			    mem_estimate);
			vxt_msgbox_write( TM_MSG_WARNING, 1, k_mmg_enteringlowp,
			    0);
			}
		    else   /* high water, there's room */
			{
			int mem_used = VxtMemoryRemaining ();
			printf (
"DECTERM: VxtMemoryRemaining before new high-water decterm = %d kb\n",
			    mem_used);
			if (create_session(DWT$K_CREATE_DECTERM_IDLE, "",
			    DECTERM_APPL_NAME, NULL, NULL, NULL, ext_lang_index,
			    transport, path, 0, 0))
			    {
			    max_decterms_seen ++;
			    mem_used -= VxtMemoryRemaining ();
			    if (mem_used < mem_estimate)
				{
				mem_estimate = mem_used;
				printf (
"DECTERM: New estimate for mem needed per decterm = %d\n", mem_estimate);
				}
			    }
			else   /* high water, room, but failed */
			    printf (
"DECTERM: Can't create non-high-water decterm even with enough memory.\n");
			}
		else   /* high water, no estimate yet */
		    if (lomem ())
			{
			printf (
"DECTERM:  Refusing because we're not in S4\n",
			    mem_estimate);
			vxt_msgbox_write( TM_MSG_WARNING, 1, k_mmg_enteringlowp,
			    0);
			}
		    else   /* high water, no estimate yet, mem isn't low */
			{
			int mem_used = VxtMemoryRemaining ();
			printf (
"DECTERM: VxtMemoryRemaining before decterm = %d kb (no mem estimate yet)\n",
			    mem_used);
			if (create_session(DWT$K_CREATE_DECTERM_IDLE, "",
			    DECTERM_APPL_NAME, NULL, NULL, NULL, ext_lang_index,
			    transport, path, 0, 0))
			    {
			    if (++max_decterms_seen >= 2)
				{
				mem_estimate = mem_used - VxtMemoryRemaining ();
				printf (
"DECTERM: First estimate for mem needed per decterm = %d kb\n", mem_estimate);
				}
			    }
			else   /* no estimate, mem o.k., but create failed */
			    printf (
"DECTERM: Can't create high-water decterm even though there's enough mem.\n");
			}
	    else   /* not high water mark */
		if (!create_session(DWT$K_CREATE_DECTERM_IDLE, "",
		    DECTERM_APPL_NAME, NULL, NULL, NULL, ext_lang_index,
			transport, path, 0, 0))
		    printf (
"DECTERM: Can't create non-high-water decterm but we had %d of them before !\n",
			max_decterms_seen);
	    }
}

/*
 *  Routine to open a socket connection to the terminal manager for creating
 *  new DECterm windows
 */
static int open_tm_socket()
{
    tm_s_struct *tm_socket;
    int socket_fd;

    /* Need to make the socket name unique.
     */

    socket_fd = VxtFileOpen( VxtFileClassSocket, "DECtermControl", O_RDONLY, 0 );

#ifdef DEBUG_TM
    printf( " open_tm_socket(): socket_fd = %x\n", socket_fd );
#endif DEBUG_TM
    if ( socket_fd >= 0 ) {
	tm_socket = XtMalloc( sizeof( tm_s_struct) );
#ifdef DEBUG_TM
    printf( " tm_socket = %x, *tm_socket = %x\n", tm_socket, *tm_socket );
#endif DEBUG_TM
	bzero( tm_socket, sizeof( tm_s_struct) );
        tm_socket->sfd = socket_fd;
        tm_socket->socket_id = XtAppAddInput (TEA_app_context, socket_fd, 
			XtInputReadMask, create_new_emulator,
			tm_socket);
#ifdef DEBUG_TM
    printf( " tm_socket->socket_id = %x\n",  tm_socket->socket_id );
#endif DEBUG_TM
        }
    else {
        printf( "Failed to open socket to TM \n");
        return(-1);
        }

    return(1);
}

/*
 *  Routine to close a socket connection to the terminal manager 
 */
static int close_tm_socket( tm_socket )
    tm_s_struct *tm_socket;
{
#ifdef DEBUG_TM
    printf(" entering close_tm_socket\n");
    printf( " tm_socket %x, tm_socket->socket_id = %x\n",  tm_socket,
	tm_socket->socket_id );
#endif DEBUG_TM
    XtRemoveInput( tm_socket->socket_id );
    close( tm_socket->sfd );
    XtFree( tm_socket );

    return(1);
}

/*
 * lomem returns true(1) iff mem is already low.
 */
static char lomem ()
{
int memstate;
    memstate = VxtGetLowMemState();
return !(memstate&S4);
}

/*
 * room_for returns true iff there's room for the specified number of bytes.
 */
static room_for (nbytes)
{
return (VxtCheckNextMemState ( nbytes, 0) & S4) &&
    (VxtCheckNextMemState (nbytes, 1) & S4);
}

#endif VXT_DECTERM

/*
 * Here's a procedure that determines if a particular item was given
 * in the command line.
 *
 * Input:
 *	str		Pointer to string item in question
 *	argc		number of arguments to search
 *	argv		vector of arguments (only 1th through argcth examined)
 *	value		pointer to value (following string in question)
 *
 * Output:
 *	number of arg found or 0 if not found
 *	argv has arg removed
 */

static int
see_item_value( str, argcp, argv, valuep )
    char str[];
    int *argcp;
    char *argv[];
    char **valuep;
{
    int  n, i, found = 0, argc = *argcp;

    if (valuep != NULL) *valuep = NULL;

    for ( n=1; n<argc; n++ ) {
	if (!found) {
	    if ( 0 == strcmp( argv[n], str ) ) {
		i = found = n;
		if ( valuep != NULL ) {
		    n++;
		    if ( n < argc ) {
			*valuep = argv[n];
			(*argcp)--;
		    } else {
			return 0;
		    }
		}
		(*argcp)--;
	    }
	} else {
	    argv[i++] = argv[n];
	}
    }
    return found;
}

static
int see_item( str, argc, argv )
    char str[];
    int *argc;
    char *argv[];
{
    return see_item_value( str, argc, argv, NULL );
}

static int
see_item_prefix( str, argcp, argv, valuep )
    char str[];
    int *argcp;
    char *argv[];
    char **valuep;
{
    int  n, i, found = 0, argc = *argcp, len = strlen(str);

    *valuep = NULL;

    for ( n=1; n<argc; n++ ) {
	if (!found) {
	    if ( 0 == strncmp( argv[n], str, len ) ) {
		i = found = n;
		*valuep = argv[n] + len;
		*argcp--;
	    }
	} else {
	    argv[i++] = argv[n];
	}
    }
    return found;
}

/*
 * decw$term_get_lang_index equivalent routine in Ultrix
 */
#if !defined(VMS_DECTERM)
static char *get_lang (dpy_string, dpy_value)
char *dpy_string;
Display *dpy_value;
{
Display *dpy;
XrmDatabase db;
XrmValue value;
char *lang, *return_type, *fallback;
char *forced_language;

fallback = XtNewString ("language_unknown");
if (dpy_value)
    dpy = dpy_value;
else
    return fallback;

#if (XlibSpecificationRelease >= 5)	/* R5 or later */
    db = XrmGetDatabase (dpy);
#else
    db = XrmGetStringDatabase(dpy->xdefaults);
#endif

if ( !XrmGetResource(db, ".xnlLanguage", ".XnlLanguage", &return_type, &value)
     && (value.addr = getenv (env_variable)) == NULL )
        lang = fallback;
else
    {
    lang = XtNewString (value.addr);
    XtFree (fallback);
    }

#if !(XlibSpecificationRelease >= 5)	/* before R5 */
    XrmDestroyDatabase (db);	/* db has to be destroyed if created by
				 * XrmGetStringDatabase() but not
				 * XrmGetDatabase().
				 */
#endif

return lang;
}

int decw$term_get_lang_index( dpy_str, dpy_val )
    char *dpy_str;
    Display *dpy_val;
{
int i, sum = 0;
char *lang;

lang = get_lang (dpy_str, dpy_val);
for (i=0; i<n_langs; i++) if (0 == strncmp (lang, lang_table[i], 5))
    {
    XtFree (lang);
    return i;
    }
for (i=0; i<strlen (lang); i++) sum += lang[i];
XtFree (lang);
return sum;
}
#endif

Boolean font_exist( dpy, lang_index )
    Display *dpy;
    int lang_index;
{
    int count;
    char *charset = NULL, **font_name_list;

    if ( !dpy )
	return( True );
    switch ( lang_index ) {
	case 17: charset =
"-JDECW-Screen-*-*-*-*-*-*-*-*-*-*-JISX0208-Kanji11";
		 break;		/* ja_JP */
	case  7: charset =
"-ADECW-Screen-*-*-*-*-*-*-*-*-*-*-GB2312.1980-*";
		 break;		/* zh_CN */
	case 18: charset =
"-ADECW-Screen-*-*-*-*-*-*-*-*-*-*-KSC5601.1987-*";
		 break;		/* ko_KR */
	case  8: charset =
"-ADECW-Screen-*-*-*-*-*-*-*-*-*-*-DEC.CNS11643.1986-*";
		 break;		/* zh_TW */
	default: return( True );
    }
    font_name_list = XListFonts( dpy, charset, 1, &count );
    if ( count ) {
	XFreeFontNames( font_name_list );
	return( True );
    } else 
	return( False );
} 

#ifdef VMS_DECTERM
/*
	This routine returns a pointer to a descriptor of the supplied
	string. The descriptors are static allocated, and up to "Md1" may be
	used at once.  After that, the old ones are re-used. Be careful!

	The primary use of this routine is to allow passing of C strings into
	VMS system facilities and RTL functions.
*/
/*ALPHA NOTE!!! The second parameter is not optional anymore!  Pass a zero
                if strlen(s) is desired
*/
static struct dsc$descriptor_s *descptr( s, l)
char *s;
int l;
    {

#define Md1 10
    noshare static next_d = 0;
    noshare static struct dsc$descriptor_s dsclist[ Md1];

    if( next_d >= Md1)
	next_d = 0;

    if( l <= 0 )
	dsclist[ next_d].dsc$w_length = strlen(s);
    else
	dsclist[ next_d].dsc$w_length = l;

    dsclist[ next_d].dsc$b_dtype =  DSC$K_DTYPE_T;
    dsclist[ next_d].dsc$b_class =  DSC$K_CLASS_S;
    dsclist[ next_d].dsc$a_pointer = s;
    return( &dsclist[ next_d++]);
    }
#endif VMS_DECTERM

globalref def_cols, def_rows, def_x, def_y;
globalref char decterm_version[];
globalref char build_date[];
globalref char build_time[];

#ifdef LOGGING
FILE *logfile;
#endif

#ifdef VXT_DECTERM
void term_main ()
{
    int argc = NULL;
    char *argv[] = NULL;
#else VXT_DECTERM
#ifdef COMBINE
term_main (argc, argv)
    int argc;
    char *argv[];
{
#else COMBINE
main (argc, argv)
    int argc;
    char *argv[];
{
#endif COMBINE
#endif VXT_DECTERM

	char  our_display[MAX_NODE_NAME_LEN+1];
	char  *display_arg = NULL;
	char  *setup_file = NULL;
	char  *customization = NULL;
	char  *temp_arg;
	int   flavor = DWT$K_CREATE_DECTERM_LOGGED_IN;
	int   n, status;
	int lang_index;
	long int    pid;	/* Process' PID */
	char spid[9],		/* Hex (char) representation of PID */
	     spid4[5];		/* Last four hex digits of PID */
	char cprocnam[16];	/* Controller process name */
	char node_name[1+MAX_NODE_NAME_LEN];
	char server_num[1+MAX_SERVER_LEN];
	char screen_num[1+MAX_SCREEN_LEN];
	char transport[1+MAX_TRANSPORT_LEN];
	char *temp;
#ifdef VXT_DECTERM
    	char *io_transport = VxtFileClassLATterminal;
   	char *io_path = "";
    	char *resource_list = 0;
#else
    	char *io_transport = 0;
   	char *io_path = 0;
    	char *resource_list = 0;
#endif VXT_DECTERM
	Boolean lang_index_set = False;
	String *ext_argv;

#ifdef VMS_DECTERM
	struct dsc$descriptor_s *contdescp;
	char user_name[MAX_USERNAME_LEN+1];
	$DESCRIPTOR(username, user_name);
#endif /* VMS */
	char *client_node;
	char *controller_atom_str;
	int  controller_atom_str_len;
	char lang_str[MAX_LANG_LEN + 1];
	Window selection_window;

#ifdef VMS_DECTERM
/*
 * If this is vms, store our error handler address in the first longword
 * of the vax frame.  This allows access violations to be caught by us
 * so we can write a log file showing what path of routine calls led to
 * the crash.
 */

/*ALPHA pragma builtins is VAX specific */

#if defined VMS_DECTERM && (!defined(ALPHA) || !defined(__alpha))
   {
#pragma builtins
	int *condition_handler = _READ_GPR(13);
   extern decterm_error_handler();
   *condition_handler = decterm_error_handler;
   setup_exit_handler();
   }
/*
 * If DECTERM_CHECK_MEMORY is defined, turn on memory-checking.  This must be
 * done *before* other initialization in order to properly take effect.
 *
 * Memory-checking makes everything run slower but allows us to catch
 * problems such as references to uninitialized data.  Such problems would
 * otherwise only be caught sporadically.
 *
 */
    if (getenv ("DECTERM_CHECK_MEMORY"))
	{
	globalref FAKE_VM_REAL_FREE_OFF, FAKE_VM_EFN_OFF;
	int value;
	log_message (
	"*** Running more slowly because doing memory-checking ***\n");
	FAKE_VM_EFN_OFF = 1;   /* Don't announce efn errors. */
	log_message ("FAKE_VM_EFN_OFF = %d\n", FAKE_VM_EFN_OFF);
/*
 * 1 means zap freed memory to funny pattern.
 * 3 means use separate pages for each allocated block and remove pages from
 *   address space.
 */
	FAKE_VM_REAL_FREE_OFF =
	    (value = getenv ("FAKE_VM_REAL_FREE_OFF")) ? atoi (value) : 3;
	log_message ("FAKE_VM_REAL_FREE_OFF = %d\n", FAKE_VM_REAL_FREE_OFF);
	fake_vm_intercept_xfer_vector ();
	}
#endif /* !ALPHA */

   log_message( "This is the Motif Version of the DECterm controller\n");

#endif VMS_DECTERM

    max_mem_decterms = 0;   /* maximum DECterms gets computed later */

/*
 * Do windows initialization.
 */

        MrmInitialize ();        /* init Mrm, before Xt */
        DXmInitialize ();        /* init DXm, before Xt */

/* ** don't do the XtToolkitInitiliaze and XtCreateApplicationContext here.
/*    Call XtAppInitialize() later which automatically call
/*    XtToolkitInitialize and XtOpendisplay.  Calling XtAppInitialize() will
/*    fix the error in MrmOpenHierarchy() in X11 R5 (V1.2/BL2).
/*
/*	XtToolkitInitialize();
/*
/* 
/* /*
/*  * We must create a single application context for DECterm, because
/*  * the toolkit only supports waiting for one context at a time.
/*  * This must be done before we open our display.
/*  */
/* 	TEA_app_context = XtCreateApplicationContext();
/* 
 **/

#ifdef VMS_DECTERM
    if (!see_item_value( "-d", &argc, argv, &display_arg ) )
	see_item_value( "-display", &argc, argv, &display_arg );

    if (display_arg == NULL)
    {
	our_display[0] = '\0';
	display_arg = our_display;
    }
#endif

    XSetIOErrorHandler(tea_io_error_handler);
    XSetErrorHandler(x_error_handler);

    if (see_item_value( "-geometry", &argc, argv, &temp_arg ))
    {
        int x=0, y=0;			/* just in case they change to */
        unsigned int cols=0, rows=0;	/*   Positions and Dimensions. */
        unsigned int flags;
        flags = XParseGeometry( temp_arg, &x, &y, &cols, &rows);
        if (flags & XValue) def_x = x;
        if (flags & YValue) def_y = y;
        if (flags & WidthValue) def_cols = cols;
        if (flags & HeightValue) def_rows = rows;
    }
    if (see_item_value( "-size", &argc, argv, &temp_arg ))
    {
	int x=0, y=0;			/* just in case they change to */
	unsigned int cols=0, rows=0;	/*   Positions and Dimensions. */
	unsigned int flags;
	flags = XParseGeometry( temp_arg, &x, &y, &cols, &rows);
	if (flags & WidthValue) def_cols = cols;
	if (flags & HeightValue) def_rows = rows;
    }

#ifdef VMS_DECTERM
/*
 * Get info about our display that we need to properly conjure the name
 * of the event flag cluster.
 */
    get_display_info(node_name, server_num, screen_num, transport, display_arg);
#endif VMS_DECTERM

    /* see if we need to logout when controller is destroyed
     */
    if (see_item( "-logout", &argc, argv )) logout_flag = 1;

/* get language index from command line */
    {
	char *lang;

	if ( see_item( "-multi", &argc, argv )) {
	    ext_lang_index = 99;
	    lang_index_set = True;
	}

	/* "-multi" has the priority
	 */
	if ( !lang_index_set &&
	     see_item_value( "-xnllanguage", &argc, argv, &lang )) {

	    if ( !strcmp( lang, "multi" )) {	/* If multi-DECterm, 920117 */
		ext_lang_index = 99;
		lang_index_set = True;
	    } else {
		for ( ext_lang_index = n_langs; ext_lang_index; )
		    if ( !strncmp( lang, lang_table[--ext_lang_index], 5 )) {
			lang_index_set = True;
			break;
		    }
	    }
	}
    }

/* include xnllanguage argument in argv for XtAppInitialize() */
    if ( lang_index_set ) {
        strncpy(
	    ext_argv_2,
	    ext_lang_index==99 ? lang_table[0] :
	    (ext_lang_index >= 0 && ext_lang_index<n_langs ?
	     lang_table[ext_lang_index] : "NO_LANG"), MAX_LANG_LEN);

        lang_index = ext_lang_index;
    } else {
	strcpy( ext_argv_2, "NO_LANG" );
    }

    (String *)ext_argv = (String *) XtMalloc( (argc+2) * sizeof(String *));
    memcpy( ext_argv, argv, argc * sizeof(String *) );

    if (lang_index_set)   /* extend only if language index was set */
    {
	ext_argv[argc++] = ext_argv_1;
	ext_argv[argc++] = ext_argv_2;
    }

/*
 * We have to open the display before opening the DRM hierarchy in order
 * to support language switching.  This is also a chance to see whether
 * we're pointing to a valid display, which is important on VMS since it
 * gives quick notification to the requestor.
 */
    dummy_shell = XtAppInitialize(&TEA_app_context,
				  DECTERM_APPL_CLASS,
				  0,
				  0,
				  &argc,
				  ext_argv,
				  NULL,
				  0,
				  0);

    /* XtAppInitialize failed.  Treat as Can't open display */
    if (dummy_shell == NULL)
    {
#ifdef VXT_DECTERM
        vxt_msgbox_write (TM_MSG_FATAL, 1, k_decterm_open_display_failed, 0);
#else
	log_message("XtAppInitialize() returned NULL\n");
#endif
	process_exit(DECW$_CANT_OPEN_DISPLAY);
    }

    /* restore argv */
    memcpy( argv, ext_argv, argc * sizeof(String *) );
    XtFree( (char *)ext_argv );

    TEA_display = XtDisplay(dummy_shell);

    if ( !lang_index_set ) {
        lang_index = decw$term_get_lang_index (0, TEA_display);
	ext_lang_index = lang_index;	/* set ext_lang_index here! */
	lang_index_set = True;
    }

#ifdef VMS_DECTERM
/*
 * Get index for language string.  We don't do this until *after*
 * get_display_info, since decw$display isn't properly set up yet.
 */
lang_index = decw$term_get_lang_index (0, TEA_display);
log_message ("DECterm controller language index = %d\n", lang_index);

    /* note, after this call, username.dsc$w_length will be set to the
     * resultant length.  If username descriptor is being used later, this
     * username.dsc$w_length has to be reset to the max length.
     */
    status = lib$getjpi(&JPI$_USERNAME, 0, 0, 0, &username,
			&username.dsc$w_length);

    if ( FAILED( status ) ) {
	log_message( "TEA could not get user name, status was %x\n",status );
	process_exit( status );
    }

    /* Chop off blanks
     */
    while (username.dsc$w_length >= 0 &&
	   username.dsc$a_pointer[username.dsc$w_length] == ' ')
	username.dsc$w_length--;

    username.dsc$a_pointer[username.dsc$w_length + 1] = '\0';

    /* get client node name (fullname) */
    status = _DECwTermGetHostName(&client_node, NULL, transport);

    if ( FAILED( status ) ) {
	log_message( "TEA could not get client node (fullname), status \
was %x\n",status );
	process_exit( status );
    }

    /* Create the selection atom name string with display name, transport name,
     * xnllanguage, server number, screen number, user name and xnllang
     */
    strcpy( lang_str,
	    ( lang_index == 99 ) ? "multi" :
	      (( lang_index >= 0 && lang_index < n_langs ) ?
		  lang_table[lang_index] : "NO_LANG" ));

    /* Contruct the string for selection atom.  The string will be in the
     * form :
     * _DEC_TERM_SEL_Client_Display_User_Transport_Server.Screen_XnlLang
     */
    controller_atom_str_len = 
		strlen("_DEC_TERM_SEL_") +
		strlen(client_node)	 + 1 +	/* client node name + "_" */
		strlen(node_name)	 + 1 +	/* node + "_" */
		strlen(username.dsc$a_pointer)	+ 1 + /* user + "_" */
		strlen(transport)	 + 1 +	/* transport + "_" */
		strlen(server_num)	 + 1 +	/* server + "." */
		strlen(screen_num)	 + 1 +	/* screen + "." */
		strlen(lang_str);		/* xnllanguage */


    controller_atom_str = (char *) XtMalloc(controller_atom_str_len + 1);

    log_message("controller_atom_str's len = %d\n", controller_atom_str_len);

    sprintf(controller_atom_str,
	    "_DEC_TERM_SEL_%s_%s_%s_%s_%s.%s_%s",
	    client_node, node_name, username.dsc$a_pointer,
	    transport, server_num, screen_num, lang_str
	    );

    XtFree(client_node);

    log_message("controller_atom_str = %s\n", controller_atom_str);

    controller_atom = XInternAtom(TEA_display, controller_atom_str, False);

    XtFree(controller_atom_str);

    if (controller_atom == None)
    {
	log_message("Failure status returned from XInternAtom\n");

	return DECW$_CANT_CREATE_ATOM;
    }

    log_message("Selection Atom = %d\n", controller_atom);

    /* create a controller window to hold the selection
     * The X Book didn't say it will return NULL when failed so no
     * validation of whether creation is okay is done here.
     */

    controller_window = XCreateSimpleWindow(TEA_display,
					    XDefaultRootWindow(TEA_display),
					    0, 0, 1, 1, 0, 0, 0);

    /* Own the selection.  Always create a new controller if requested.
     * We don't have to worry about the "zombie" controller as in old design
     * and therefore the DECTERM_DUPLICATE_OK flag is obsolete.
     */
    XSetSelectionOwner(TEA_display, controller_atom, controller_window,
		       CurrentTime);

/*	 
**  We must set our process name to contain part of our own PID (the last 4 hex digits).
*/

/* Get the current process' PID */
    lib$getjpi(&JPI$_PID, 0, 0, &pid, 0, 0);
/* Store its hex representation in a string */
    sprintf(spid,"%8lX",pid);	/* Pad to 8 characters */
    spid[8] = '\0';
    for (n=0; n<8; n++)		/* Fill spaces with 0's */
	if (spid[n] == ' ')
	    spid[n] = '0';

/* Grab the last 4 characters of that string, which are unique on this machine. */	 
    strncpy(spid4, &spid[4], 4);
    spid4[4] = '\0';
/* Build the new process name */
    sprintf(cprocnam, "DECW$TE_%s", spid4);
    contdescp = descptr( cprocnam, 0 );

/* Set our process name */
    status = sys$setprn(contdescp);
    if ( FAILED( status ) ) {
	log_message( "TEA could not set process name, status was %x\n",status );
	process_exit( status );
    }

#endif VMS_DECTERM

#ifndef VMS_DECTERM
	if (see_item( "-L", &argc, argv )) {
	    getty_dev = argv[--argc];
	}
	if ( n = see_item( "-e", &argc, argv ) ) {
	    argv[argc] = NULL;
	    cmd_to_exec = &argv[argc = n];
	}
	if (see_item( "-C", &argc, argv )) console_flag = 1;
	if (see_item( "-ls", &argc, argv )) login_shell_flag = 1;
	see_item_value( "-S", &argc, argv, &slave_pty );
	see_item_prefix( "-S", &argc, argv, &slave_pty );
#endif VMS_DECTERM

	if (see_item ("/attach", &argc, argv)) attach_flag = 1;
	if (see_item ("/mailbox", &argc, argv)) mbx_flag = 1;
	if (see_item ("/login", &argc, argv)) login_flag =1;
	if (see_item ("/nologin", &argc, argv)) login_flag =0;
	if (see_item ("/scroll_measure", &argc, argv))
						measure_scroll_flag =1;
	if (see_item ("/pty_measure", &argc, argv)) measure_pty_flag =1;
#ifdef LOGGING
	if (see_item ("/logging", &argc, argv)) logging_flag =1;
#endif

	see_item_value( "-setup", &argc, argv, &setup_file );
	while ( see_item_value( "-xrm", &argc, argv, &temp_arg )
		    ? TRUE :
		    see_item_value( "-customization", &argc, argv, &temp_arg )
	    ) {
	    if (customization == NULL)
		customization = XtNewString( temp_arg );
	    else
		customization = strcat(
				    strcat(
				        strcpy(
				            XtMalloc(
					        strlen(customization)
					            +strlen(temp_arg)+2),
				            customization),
					"\n"),
				    temp_arg);
	}
	if (see_item( "-v", &argc, argv )) verbose_flag = 1;
	if (see_item_value( "-number", &argc, argv, &temp_arg )) {
	    sscanf( temp_arg, "%d", &free_decterm_flag );
	}

#ifdef LOGGING
#ifdef VMS_DECTERM
	if (logging_flag)
	    {
	    logfile = fopen("pty.log","w");
	    log_message( "File PTY.LOG in use.\n");
	    setbuf (logfile, NULL);
/*	    time(&timeword);
	    maybe_fprintf (logfile,"Started at %s\n",ctime(&timeword));
*/
	    }
#else
#ifdef VXT_DECTERM
    logfile = NULL;
#else
	logfile = stderr;
#endif VXT_DECTERM
#endif VMS_DECTERM
#endif LOGGING

/*
 * Ultrix only:
 *
 * There is a problem in spawn(): it clobbers the next 2 I/O channels beyond
 * the one used by the display.  Ideally spawn() should be re-written, but
 * as a temporary workaround, open two files (to the null device) before
 * opening the display, and close them afterwards.
 */

#if !defined (VMS_DECTERM) && !defined(VXT_DECTERM)
	dummy_fd_1 = open( "/dev/null", O_RDONLY );
	dummy_fd_2 = open( "/dev/null", O_RDONLY );
#endif

        /* open the UID database */

#ifdef VXT_DECTERM
	db_filename_vec[0] = VXT_DECTERM_UID_NAME;
	db_filename_vec[1] = VXT_DECTERM_UID_TEXT;
#else
	if ( ext_lang_index == 99 ) {
	    db_filename_vec[0] = XtMalloc( strlen( DECTERM_APPL_CLASS ) +
		strlen( DECTERM_APPL_SUFFIX ) + 2 );
	    sprintf( db_filename_vec[0], "%s_%s",
		DECTERM_APPL_CLASS, DECTERM_APPL_SUFFIX );
	    multi_uid = True;
	} else {
	    db_filename_vec[0] = XtMalloc( strlen( DECTERM_APPL_CLASS ) + 1 );
	    sprintf( db_filename_vec[0], "%s", DECTERM_APPL_CLASS );
	    multi_uid = False;
	}
#endif

#if (((XmVERSION == 1) && (XmREVISION >=2)) || XmVERSION >=2)
        if (MrmOpenHierarchyPerDisplay
               (
               TEA_display,
#else
        if (MrmOpenHierarchy
               (
#endif
#ifdef VXT_DECTERM
	       2,
#else
               1,
#endif VXT_DECTERM
	       db_filename_vec,
               NULL,
               &s_MRMHierarchy       /* ptr to returned DRM id */
               )
            != MrmSUCCESS)
            {
            log_error( "Unable to open DRM User Interface Definition:\n");
            log_error( "\n %s\n", db_filename_vec[0] );
            process_exit( DECW$_CANT_OPEN_DRM );
            };

#ifndef VXT_DECTERM
	XtFree( db_filename_vec[0] );
	db_filename_vec[0] = NULL;
#endif

#ifdef DEBUG_VXT
printf(" opened Mrm hierarchy \n");
printf(" &s_dt_MRMHierarchy = %x\n", &s_dt_MRMHierarchy);
#endif
        /* bind UIL symbols to callback routines */
        tea_register_callbacks();

#ifdef VMS_DECTERM
/*
 * VMS-specific initialization:
 *
 * Get channel on which we can be awakened for non-window events, and tell
 * X what the channel is and who to call.  For now, we don't use an iosb
 * or a closure, so we pass 0 for those two parameters.
 */
	{
	extern void do_nonx_events ();
	int status;

	status = (u_open_wake_channel (&tea_efn));
	if ( FAILED( status ) )
	    {
	    log_message( "can't open wakeup channel, exiting\n" );
	    process_exit( status );
	    }
	XtAppAddInput (TEA_app_context, tea_efn, 0, do_nonx_events, 0);
	}
#else
#ifdef VXT_DECTERM
/*
 * VXT-specific initialization:
 */

    /* creating a socket to the terminal manager */

#ifdef DEBUG_VXT
        printf( "Waiting for T command.\n" );
#endif DEBUG_VXT

	if (open_tm_socket() < 0)
	{
	    printf("DECterm:  Cannot open socket to TM, exiting...\n");
	    exit(0);
	}
#else
/*
 * Ultrix-specific initialization:
 */

    /* 
     * Connect some signals
     */
    signal (SIGHUP, SIG_IGN);
#endif
#endif


#ifdef VMS_DECTERM
/*
 * Tell world how to get in touch with us.
 */
	status = u_announce_existence (display_arg, lang_index);
	if ( status != SS$_NORMAL )
	    {
	    process_exit( status );
	    }
#endif

/*
 * Set run-flag first, since all ast's depend on it.
 */
	run_flag = 1;
#ifdef LOGGING
	maybe_fprintf (logfile, "Entering pty_init\n");
#endif
	pty_init();

#ifdef LOGGING
	maybe_fprintf (logfile, "Returned from pty_init\n");
#endif

/*
 * Announce readiness.
 */
#if defined(VMS_DECTERM) || defined(VXT_DECTERM)
    if (verbose_flag) {
	log_message( "\n\
DECterm version %s compiled at %s %s now at your service...\n",
		 decterm_version, build_date, build_time);

#ifdef VMS_DECTERM
	log_message( "\n\
To create DECterms, use the Session Manager or use CREATE/TERM or call the\n\
DECwTermPort() routine from another process.\n");
#endif

	}
#endif
/*
 * Remember what quotas we're starting with.
 */
#ifdef VMS_DECTERM
	initialize_quotas ();
#endif
/*
 * Create one free set of pseudo-terminal, widget, and subprocess, if
 * user has set free_decterm_flag.
 */

#ifdef VXT_DECTERM
flavor = DWT$K_CREATE_DECTERM_IDLE;
#endif

#ifdef DEBUG_VXT
printf(" about to create session\n");
printf( " io_tranport = %s, io_path = %s, resource_list = %x, 
    io_tranport, io_path, resource_list);
printf(" flavor = %d\n",flavor);
#endif

	for (;free_decterm_flag>0;free_decterm_flag--) {
	    char *temp;
	    create_session( flavor,
		    display_arg,
#if defined(VMS_DECTERM) || defined(VXT_DECTERM)
		    DECTERM_APPL_NAME,
#else
		    (temp = rindex( argv[0], '/' )) != NULL ? temp+1 : argv[0],
#endif
		    setup_file,
		    customization,
		    NULL,
		    ext_lang_index,
            	    io_transport,
	    	    io_path,
            	    0,
		    0
		    );
	}

	if (customization != NULL) XtFree( customization );

/*
 * Issue the first read on mailbox.
 */
#ifdef VMS_DECTERM
	u_prime_mailbox();
#else
#ifndef VXT_DECTERM
	signal (SIGCHLD, SYS_reapchild);
#endif
#endif

/*
 * Wait until done.
 */
#ifndef XtIMAll
#define XtIMAll (XtIMXEvent|XtIMTimer|XtIMAlternateInput)
#endif

#ifdef DEBUG_VXT
printf("run_flag = %d\n", run_flag);
#endif

	while ((run_flag)
#ifdef VMS_DECTERM
		|| (non_ast_queue[0] != &non_ast_queue)
#endif
	){
/* MappingNotify should be handle separately. Therefore the whole logic should
/* be changed here. 
*/
	    Bool wait_for_XEvent = TRUE;
/* First handle XEvents. Enter into the While loop at least once so the
/* XtAppNextEvent takes place, and blocks for XEvents if there aren't any.
/* It also performs i/o to bring more events from the system. The loop ends
/* therefore only after an i/o, and after all current XEvents are processed.
*/
            while ( ( XtAppPending( TEA_app_context ) & XtIMXEvent ) ||
		    ( wait_for_XEvent ) ) {
	    	XEvent ev;
	    	caddr_t tag;
		Window focus_window;
		Widget focus_widget;
		int revert;
		wait_for_XEvent = FALSE;
	    	XtAppNextEvent( TEA_app_context, &ev );
	    	switch ( (&ev)->type ) {
		    case MappingNotify:
			if ( (&ev)->xmapping.count > 1 ) {
			    XGetInputFocus( (&ev)->xmapping.display, 
			        &focus_window, &revert );
			    focus_widget = XtWindowToWidget( 
	      		        (&ev)->xmapping.display, focus_window );
			    if ( focus_widget != NULL ) {
		    	       _DECwTermMappingHandler( focus_widget, tag, &ev );
			    }
			} 
			else
		            XtDispatchEvent( &ev );
		        break;

		    case SelectionRequest:
			{
			    XSelectionEvent event;
			    XSelectionRequestEvent tmp_ev;

			    /* If selection == controller_atom, it is generated
			     * by XConvertSelection() in DECtermport.c.  We
			     * should respond with SelectionNotify event.
			     */
			    if (ev.xselectionrequest.selection ==
				controller_atom)
			    {
				/* Prepare the event, with controller_atom
				 * as property field.
				 */
				event.type        = SelectionNotify;
				event.display     = TEA_display;
				event.send_event  = 1;
				event.requestor   =
						ev.xselectionrequest.requestor;
				event.selection   = controller_atom;
				event.target      = XA_STRING;
				event.property    = controller_atom;

				XSendEvent(TEA_display,
					   ev.xselectionrequest.requestor,
					   0, NoEventMask, (XEvent *)&event);

				/* send it now and flush the queue.
				 */
				XSync(TEA_display, False);

				/* see if there are more requests coming from
				 * same requestor.  If so, discard them.
				 */
				while (XCheckIfEvent(
					TEA_display,
					(XEvent *)&tmp_ev,
					check_duplicate,
					(char *)ev.xselectionrequest.requestor))
				{
				        /* discard those events.  do nothing */
				}
			    }
			    else
			    {
				/* SelectionRequest from other sources e.g.
				 * cut/paste.  We have to dispatch them now.
				 */
				XtDispatchEvent(&ev);
			    }

			    break;
			}

		    default:
		        XtDispatchEvent( &ev );
		        break;
	        }
	    }
/*Now process the other non-X events. First check for pending, do not block
/*for them!
*/
            if ( XtAppPending( TEA_app_context ) & ( XtIMTimer | XtIMAlternateInput ) )
                XtAppProcessEvent( TEA_app_context, ( XtIMTimer | XtIMAlternateInput ) );
        }

	def_sys_db_is_set = False;
	pty_fin();
	process_exit( SUCCESSFUL_PROCESS_EXIT_STATUS );
}

/*
 * A predicate procedure passed to XCheckIfEvent() in the main loop to
 * check if any duplicate SelectionRequests from same requestor.  If so
 * return True and the event will be discarded.
 */
static Bool check_duplicate(dpy, ev, arg)
    Display *dpy;
    XEvent  *ev;
    char    *arg;
{
    XSelectionRequestEvent *sel_req_ev;
    Window	win;

    sel_req_ev = (XSelectionRequestEvent *)ev;
    win	       = (Window) arg;

    if (sel_req_ev->requestor == win)
	return (True);

    return (False);
}

/* process_exit - clean up and then exit */

process_exit( status )
	int status;
{
	int status2;

#ifdef VMS_DECTERM

	/* destroy the dummy_shell if created */
	if (dummy_shell)
	    XtDestroyWidget(dummy_shell);

	/* In fact, this will be done automatically by the Server.  Just
	 * to make sure...
	 */
	if (controller_atom != None)
	{
	    if (controller_window != NULL)
	    {
		Window win;

		XGrabServer(TEA_display);

		win = XGetSelectionOwner(TEA_display, controller_atom);

		if (win == controller_window)
		    XSetSelectionOwner(TEA_display,
				       controller_atom,
				       None,
				       CurrentTime);

		XUngrabServer(TEA_display);
	    }
	}

	/* We have to notify the creator only if this controller is created
	 * by create/term process.  Otherwise, u_notify_creator will always
	 * return a NOSUCHDEV error.
	 */
	if (logout_flag)
	    if (FAILED(status2 = u_notify_creator( status )))
		log_message("u_notify_creator() returned status = %X\n",
								      status2);

	if (FAILED(status2 = u_destroy_mailbox( status )))
	    log_message("u_destroy_mailbox() returned status = %X\n", status2);
#endif

#ifdef LOGGING
	if (logging_flag) fclose(logfile);
#endif
	if (verbose_flag)
	    log_message( "DECterm exiting.\n\n");

#ifdef VMS_DECTERM
	if ( status != SS$_NORMAL )
	    {
	    short msglen;
	    char msgbuf[256];
	    $DESCRIPTOR( msgdesc, msgbuf );

	    sys$getmsg( status, &msglen, &msgdesc, 15, 0 );
	    log_message( "%.*s\n", msglen, msgbuf );
	    }
#endif
	/* destroy the controller_window */
	if (controller_window)
	    XDestroyWindow(TEA_display, controller_window);

	/* Close display */
	if (TEA_display)
	    XCloseDisplay(TEA_display);

	/* kill itself if logout is set (which is if this process was created
	 * by the loginout.exe
	 */
#ifdef VMS_DECTERM
	if ( logout_flag )
	    sys$delprc ( 0, 0 );
#endif

#if defined(VXT_DECTERM) || defined(VMS_DECTERM)
	exit( status );
#else
	Exit( status );   /* Exit is in process_ultrix.c */
#endif

}


static int tea_io_error_handler(display)
Display *display;
{
    printf(" fatal error: DECterm connection to server lost \n");
    process_exit( NULL );
}

int x_error_handler(display,myerr)
Display *display;
XErrorEvent *myerr;
{
    char msg[80];

    XGetErrorText(display,myerr->error_code, msg, 80);
    printf("X Error code %s\n", msg);
}


/*
 * log_error is similar to log_message, except that it is for conditions
 * that are errors as opposed to expected conditions.
 */
log_error ( format, p1,p2,p3,p4,p5,p6)
{
#ifdef VXT_DECTERM
#include <msglog.h>
VXTIntErr();
#endif
log_message (format, p1,p2,p3,p4,p5,p6);   /* fall into log_message for now */
}

/*
 * Routine to write and flush a formatted message to stderr.
 */

log_message( format, arg1, arg2, arg3, arg4, arg5, arg6 )
    char *format;
    char *arg1, *arg2, *arg3, *arg4, *arg5, *arg6;
{
#ifndef VXT_DECTERM
    fprintf( stderr, format, arg1, arg2, arg3, arg4, arg5, arg6 );
    fflush( stderr );
#else
    printf( format, arg1, arg2, arg3, arg4, arg5, arg6 );
#endif VXT_DECTERM
}

#ifdef VMS_DECTERM
#define init_str_desc(desc, string)             \
{   desc.dsc$w_length = strlen(string);         \
    desc.dsc$a_pointer = string;                \
    desc.dsc$b_class = DSC$K_CLASS_S;           \
    desc.dsc$b_dtype = DSC$K_DTYPE_T;           \
}

#define init_null_str_desc(desc, string)        \
{   desc.dsc$w_length = sizeof(string);         \
    desc.dsc$a_pointer = string;                \
    desc.dsc$b_class = DSC$K_CLASS_S;           \
    desc.dsc$b_dtype = DSC$K_DTYPE_T;           \
}

struct itm$item_list_3 {
    unsigned short      itm$w_bufsiz;
    unsigned short      itm$w_itmcod;
    char                *itm$l_bufadr;
    char                *itm$l_retlen;
};

static int get_pseudo_param (chan, item, item_buf, item_len, ret_len)
unsigned long chan, item, item_len;
unsigned long *ret_len;
unsigned char * item_buf;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Does a QIO to a WSA device to get a parameter out of
**	the device
**
**  FORMAL PARAMETERS:
**
**      chan - channel open to this device
**      item - The item we want info about
**      item_len - Lenght of the item_buf
**	ret_len - Amount of data returned from QIO
**	item_buf - The buffer to return data
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/
{
unsigned short iosb[4];
int i, status;
unsigned long len;

status = sys$qiow (_DECwTermGlobalEFN(), chan, IO$_SENSEMODE|IO$M_WS_DISPLAY,
		   &iosb, 0, 0,
		   item_buf,
	    	   item_len,
	    	   item, 0, 0, 0);

if (status != SS$_NORMAL)
    {
    log_message ("SENSEMODE(%d) on chan = %d failed with status = %d\n",
	item, chan, status);
    process_exit (status);
    }

if (!(iosb[0]&1))
    {
    log_message ("SENSEMODE(%d) on chan = %d failed with IOSB status = %d\n",
	item, chan, iosb[0]);
    process_exit (iosb[0]);
    }

len= iosb[2];

/* work around a SET DISPLAY bug where sometimes a control character can be
   appended to the buffer, e.g. a line feed after the screen number */

if (len > 0 && item_buf[len - 1] < ' ')
    len--;

item_buf[len] = 0;
*ret_len = len;
}

/*
 * Hooks for debugging.
 */

/*
 * debug_create_decterm - creates a single logged in DECterm window.
 */

debug_create_decterm()
{
    create_session( DWT$K_CREATE_DECTERM_LOGGED_IN, "", DECTERM_APPL_NAME,
      NULL, NULL, NULL, ext_lang_index, 0, 0, 0, 0);
}

/*
 * debug_show_vm - calls LIB$SHOW_VM to show virtual memory statistics.
 */

debug_show_vm( code )
    int code;
{
    LIB$SHOW_VM( &code );
}
#endif VMS_DECTERM
