/* #module DT_input "T3.0-EFT2" */
/*
 *  Title:	DT_input
 *
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
 *  
 *  Module Abstract:
 *
 *  Procedures contained in this module:
 *
 *  Author:
 *
 *  Modification history:
 *
 *  Alfred von Campe    18-Nov-1993     BL-E
 *      - Fix Hebrew problem where you can't switch back to English.
 *
 *  Alfred von Campe    08-Nov-1993     BL-E
 *      - Add F11 key feature from dxterm.
 *
 *  Alfred von Campe    19-Oct-1993     BL-E
 *	- Add support for PC keyboards and Jensen.
 *
 *  Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 *  Eric Osman		11-May-1993	VXT V2.0
 *	- Allow #nn (hex) for specifying non-printable chars in answerback
 *	  message
 *
 *  Alfred von Campe    03-May-1993     V1.2/BL2
 *      - Only remap keys if they don't already have the desired keysym.
 *
 *  Alfred von Campe    25-Mar-1993     V1.2/BL2
 *      - OSF/1 code merge.
 *      - Add F1-F5 key support.
 *
 *  Aston Chan		12-Mar-1993	V1.2/BL2
 *	- Add Turkish/Greek support.
 *	- Add <ctype.h> and delete <DwtAppl.h> as Dwtlibshr.exe is not
 *	  available in V1.2 anymore.
 *	- Fix SPR ICA-45693 where occluded DECterm pasted the previously
 *	  selected string unexpectedly.
 *
 *  Alfred von Campe    02-Dec-1992     Ag/BL11
 *      - Check for keysym XK_Help instead of HelpMask in i_event_handler.
 *
 *  Eric Osman		13-Oct-1992	VXT V1.2
 *	- Use the Motif 1.1 ssb version of copy and paste
 *
 *  Eric Osman		 3-Sep-1992	VXT V1.2
 *	- Do the local echo *before enqueueing the data, so that the buffer
 *	  hasn't been deallocated.
 *
 *  Eric Osman		21-Aug-1992	VXT V1.2
 *	- If printer controller mode, don't do anything with alt-a.  Only
 *	  do something if not in printer controller mode.
 *	- Also, don't honor f2 and shift-f2 requests during printer controller
 *	  mode.
 *
 *  Aston Chan		20-Aug-1992	Post V1.1
 *	- ToLung's fix to make DECterm widget recognize locale name with
 *	  codeset.  E.g.  zh_TW.dechanyu or ja_JP.sjis etc.
 *	- ToLung's fix to support 'Alt' Key on LK401 during hex input & compose
 *	  input in VT382 emulation.
 *
 *  Eric Osman          11-June-1992     Sun
 *      - Add DXK_Remove, which seems not to be defined on Sun
 *
 *  Alfred von Campe    26-May-1992     Post V3.1
 *      - Make "Comma key sends ,, or ,<" option really work on LK401 keyboards.
 *
 *  Alfred von Campe    02-Apr-1992     Ag/BL6.2.1
 *      - Stub out AIM routines and change AIM routine initialization to
 *        satisfy OSF/1 compiler.
 *
 *  Alfred von Campe    27-Mar-1992     V3.1/Bl6
 *      - Make the "Comma key sends ,, or ,<" option work on LK401 keyboards.
 *
 *  Aston Chan		19-Dec-1991	V3.1
 *	- Changing the I18n public entry points to DECwTerm* prefix
 *	- Adding stubs for routine _DECwTermMappingHandler() for non I18n build.
 *	- Add back the Quick copy fix that I left out by mistake when doing 
 *	  I18n merge.
 *	  
 *  Aston Chan		17-Dec-1991	V3.1
 *	- I18n code merge
 *
 *  Aston Chan		20-Nov-1991	V3.1
 *	- Fix Quick Copy by adding client message event to the shell of DECterm
 *	  widget.
 *
 *  Alfred von Campe    06-Oct-1991     Hercules/1 T0.7
 *	- Added F11 key feature from ULTRIX.
 *      - Changed private #include file names to all lower case.
 *      - Removed noshare from a couple of static declarations.
 *      - Changed DectermUpdateCallback() to DecTermUpdateCallback().
 *      - Changed Widget parameter type to DECtermWidget i_do_StuffSecondary().
 *
 * Bob Messenger	15-Apr-1991	T3.0-EFT2
 *	- Fix link errors:
 *		Move wm_protocol event handling to ptysub.c.
 *		Call widget_message instead of log_message.
 *
 *  Jim Bay		13-Apr-1991	VMS T3.0-EFT2
 *  Jim Bay		21-Mar-1991	UWS 4.2 (BL6)
 *	- Fixed translation for AustrianGerman 7-bit NRCS
 *	  Corrected E00 (case 197 -> 196) and C11 (case 126)
 *
 * Eric Osman		21-Mar-1991	V3.0
 *	- Allow i_event_handler to be called from scrollbar when text
 *	  keys are typed while mouse is over scrollbar.
 *
 * Eric Osman		4-Mar-1991
 *	- Expand the client message mechanism so we can capture delete
 *	  requests, and delete just one DECterm instead of entire
 *	  controller.
 *
 * Bob Messenger	 3-Oct-1990	X3.0-7
 *	- Back out the 6-Sep-1990 change "If i_focus_handler is called
 *	  with a FocusIn event...", since this introduced the "duelling
 *	  DECterm" bug, where two DECterm windows alternately grab the
 *	  input focus.
 *
 * Bob Messenger	11-Sep-1990	X3.0-7
 *	- Send XA_PRIMARY instead of XA_SECONDARY when killing selection.
 *
 * Bob Messenger	 6-Sep-1990	X3.0-7
 *	- Add i_send_unsolicited_report, for unsolicited button reports.
 *	- Don't force event type to be ClientMessage in i_do_stuff_selection.
 *	- If i_focus_handler is called with a FocusIn event, make sure that
 *	  we really have the input focus (fixes/works around a problem with
 *	  secondary selection).
 *	- Allow only MB2 for secondary selection.
 *
 * Bob Messenger	27-Aug-1990	X3.0-6
 *	- Use MB2 for selection, as a synonym for MB3.
 *
 *  Michele Lien    24-Aug-1990 VXT X0.0
 *  - Modify this module to work on VXT platform. Change #ifdef VMS to
 *    #ifdef VMS_DECTERM so that VXT specific code can be compiled under
 *    VMS or ULTRIX development environment.
 *
 * Bob Messenger	20-Jul-1990	X3.0-5
 *	- Shift/F2 is Graphics Print Screen.
 *
 * Bob Messenger	17-Jul-1990	X3.0-5
 *	Merge in Toshi Tanimoto's changes to support Asian terminals -
 *	- Kana keysym handling
 *
 * Bob Messenger	13-Jul-1990	X3.0-5
 *	- Check for Alt/key synonyms for function keys, in order to support
 *	  the UNIX keyboard.
 *
 * Bob Messenger	23-Jun-1990	X3.0-5
 *	- Add printer port support.
 *		- F2 does a print page (text print screen).
 *		- Make lookup_nrc_code global, and have it return a status
 *		  code to indicate whether the input code could be translated
 *		  to a code in the nrc set.
 *	- Add secure keyboard support.
 *
 * Mark Woodbury	25-May-1990 X3.0-3M
 *  - Motif update
 *
 * Bob Messenger	08-Aug-1989	X2.0-18
 *	- Back out the 29-May-1989 "Always send keyboard data even if input
 *	  has been suspended" change.
 *
 * Bob Messenger	16-Jun-1989	X2.0-15
 *	- Fix the previous fix: return if shifted or controlled editing keys
 *	  or controlled function keys.
 *
 * Bob Messenger	14-Jun-1989	X2.0-14
 *	- Fix test for shifted and controlled function keys; currently it
 *	  tests for anything shifted or controlled *except* a function key,
 *	  which breaks things like NRC character conversion.
 *
 * Bob Messenger	29-May-1989	X2.0-13
 *	- Don't transmit anything for shifted and controlled editing keys.
 *	- Replace bcopy and bzero with memcpy and memset.
 *	- Always send keyboard data even if input has been suspended.
 *
 * Bob Messenger	08-Apr-1989	X2.0-6
 *	- Allow for context sensitive help by only sending the sequence for
 *	  the Help key on a key up when there has been no button pressed
 *	  since the key down.
 *	- Call help callback when button 1 pressed while help key is down.
 *	- Use (hopefully) better names for keyboard mapping resources.
 *
 * Bob Messenger	07-Apr-1989	X2.0-6
 *	- Convert to new widget bindings (DECwTerm instead of DwtDECterm).
 *	  Keep old routine names for backward compatibility.
 *
 * Bob Messenger	03-Mar-1989	X2.0-2
 *	- Added missing characters in lookup_nrc_code, removed Dutch NRC
 *
 * Bob Messenger	29-Jan-1989	X1.1-1
 *	- Added enqueue_input_data to support ReGIS one-shot input mode.
 *
 * Bob Messenger	09-Dec-1988	X1.1-1
 *	- Support new line mode
 *
 * Bob Messenger	17-Nov-1988	X0.5-10
 *	- Check for lockUserFeatures properly when changing auto repeat.
 *
 * Tom Porcher		20-Oct-1988	X0.5-4
 *	- Fix DEC MCS compose and NRC translation by using values from
 *	  resources and not source private structures.
 *	- Fix lookup_nrc_code() so it works.
 *	- Move check for MCS compose sequences to *before* XLookupString().
 *
 * Eric Osman		17-Oct-1988	BL11.2
 *	- Fix ctrl/<x] by making sure it's key is NOT modified with
 *	  control before doing translation for backspace vs. delete
 *	- Make wait light come on when KAM sent or typein buffer is full
 *
 * Eric Osman		 7-Sep-1988	BL10.1
 *	- Do appropriate translations of comma,angle,tilde,delete keys.
 *
 * Tom Porcher		 9-Sep-1988	X0.5-1
 *	- Only allow word/line select w/MB1.
 *	- Select current word/line when selecting backwards.
 *	- Don't do anything on MB2.
 *	- Words now end at zeros in the display list, too.
 *
 * Tom Porcher		 7-Sep-1988	X0.5-0
 *	- use button time rather than CurrentTime to cancel a selection
 *	  when button is pressed for the first time.
 *
 * Tom Porcher		16-Aug-1988	X0.4-43
 *	- Add controlQSHold control (Ctrl-Q/S as Unhold/Hold).
 *	- Remove control of keyclick.
 *	- Add test of userFeaturesLock to auto-repeat setting.
 *
 * Mike Leibow		16-Aug-1988	X0.4-44
 *	- if the control key is depressed at the same time as a F6 - F20 key,
 *	  the keystrokes are ignored.
 *	- added i_lock_keyboard and i_unlock_keyboard.
 *
 * Tom Porcher		10-Aug-1988	X0.4-43
 *	- Attempt to fix un-focus-able DECterm problem:  i_accept_focus()
 *	  now ALWAYS does an XSetInputFocus().
 *
 * Tom Porcher		28-Jul-1988	X0.4-41
 *	- Fix bug where keys were put on keyboard_queue AND freed.
 *
 * Tom Porcher		15-Jul-1988	X0.4-37
 *	- Make i_destroy() not call i_clear_comm() but just empty all queues.
 *	  This was causing too many side effects, like causing source to
 *	  register it's work procedure!
 *	- Make 1+3n clicks clear selection.
 *
 * Eric Osman		14-Jul-1988	X0.4-36
 *	- Clear wait-light condition when sending more input.
 *
 * Tom Porcher		10-Jul-1988	X0.4-36
 *	- change keyboard controls on each focus event.
 *
 * Rich Hyde		29-Jun-1988	X0.4-33
 *	- Add word/line select and Quick Copy.
 *
 * Eric Osman		6-Jun-1988	X0.4-30
 *	- Do input NRC conversion, and fix compose which VT52 fix had
 *	  broken
 *
 * Eric Osman		18-May-1988	X0.4-28
 *	- Tell output module that focus has come in or out (for blink control).
 *
 * Bob Messenger	17-May-1988	X0.4-28
 *	- Require a threshold of half a text character before a region is
 *	  selected.
 *
 * Eric Osman		11-May-1988	X0.4-26
 *	- Added compose key support
 *
 * Tom Porcher		 9-May-1988	X0.4-26
 *	- Added input flow control.
 *	- Removed special code for keycodes XK_F11-F13 and Remove.
 *	- Removed kludge for CurrentTime.
 *
 * Tom Porcher		21-Apr-1988	X0.4-10
 *	- Replaced i_create() with i_initialize() called at widget creation.
 *	- Renamed i_create() (now empty) to i_realize().
 *	- Test to see if widget is realized before accepting focus.
 *
 * Tom Porcher		 5-Apr-1988	X0.4-7
 *	- Added new time parameter to i_accept_focus; also returns Boolean.
 *	- Added time parameter to s_set_selection().
 *	- Changed Toolkit selection routines to new call formats.
 *	- Added kludge to make CurrentTime work in DT_source_subs.c.
 *
 * Tom Porcher		30-Mar-1988	X0.4-5
 *	- Changed FMT8BIT to XA_STRING.
 *	- Added DwtDECtermStuff().
 *	- Removed setting of "has_focus" from accept_focus().
 *
 * Tom Porcher		17-Jan-1988	X0.3-2
 *	- Reset "selection_in_progress" on initialization.
 *
 *  Eric Osman		8-Jan-1988	X0.3
 *	- Clear motion handler to avoid false callbacks.
 *
 *  Tom Porcher		31-Dec-1987	X0.3
 *	- Changed callback mechanism to use XtCallCallbacks and standard format.
 *	- Added i_destroy() to deallocate all dynamic memory (event handlers).
 *
 *  Tom Porcher		23-Dec-1987	X0.3
 *	- Major revision to support Selection and Stuff.
 *
 *  Tom Porcher		30-Nov-1987	X0.3
 *	Changed "Keysym.h" to "keysym.h".
 *
 *  <modifier's name>	<date>		<ident of revised code>
 *	<description of change and purpose of change>
 *
 */

/*
 * The "Remove" key seems to not be defined on Sun.
 */
#ifndef DXK_Remove
#define DXK_Remove 0x1000FF00
#endif

#define UPSS	w->source.wvt$b_user_preference_set
#ifndef XK_HEBREW
#define XK_HEBREW 1
#endif

#include <ctype.h>
#include "dectermp.h"
#include "wv_hdr.h"
#include "DXmAIM.h"

#include <X11/keysym.h>

#ifdef VMS_DECTERM	/* for dynamic image activation, 911030, TN350 */
#include "descrip.h"
#include "ssdef.h"
#include "libdef.h"
#define DXmCreateAIM( w, s, al, ac )	(*create_aim_ptr)( w, s, al, ac )
#define DXmAIMGetRIMPList( w, s, ac )	(*get_rimplist_ptr)( w, s, ac )
#define DXmAIMIsRIMPAvailable( w, s )	(*is_rimp_available_ptr)( w, s )
#endif

#if defined(VMS_DECTERM) || defined(VXT_DECTERM)
#include "DECspecific.h"
#else
#include <DXm/DECspecific.h>
#endif

#define HelpMask Mod5Mask	/* Mod5 is the help key */

extern void s_set_selection();
extern void s_stop_output();

static void i_do_stuff();
static void KEY_input();
static void i_focus_handler();
static void i_event_handler();
       void _DECwTermMappingHandler();
static void transmit_input_data();
static void i_do_StuffSecondary();
static void send_more_input();
Boolean enqueue_input_data();
static int hex_convert();
static void i_create_aim();	/* 910509, TN005a */
static void draw_intermediate_char();
static void get_current_cursor_char();
static void pre_edit_draw();
static void pre_edit_start();
static void query_xy_position();
static void rimp_destroy();
static void sec_pre_edit_start();
static void set_cursor_position();
extern void o_draw_intermediate_char();
extern s_line_structure *s_read_data();
#if defined(VMS_DECTERM)
	/* for dynamic image activation, 911030, TN350 */
static DXmAIMWidget (*create_aim_ptr)();
static void (*get_rimplist_ptr)();
static Boolean (*is_rimp_available_ptr)();
#endif
queue_element *queue_alloc();
static void i_shell_client_handler();
static Widget shell_widget();

#ifdef SECURE_KEYBOARD
void i_set_value_secureKeyboard();
#endif

#ifdef MEASURE_KEYBOARD_PERFORMANCE

#include timeb
extern int key_enabled, key_max, key_min, key_total, key_num;
extern struct timeb key_time;
extern char key_char;
#endif

#define MAX_KEYCODE_LENGTH 20
#define MAX_KEYBOARD_QUEUE 10
#define MAX_REPORT_QUEUE 4
#define MAX_PASTE_QUEUE 2
#define WAIT_LED 1
#define HOLD_LED 4
#define HEX_CODE_LENGTH 1

/*
 * Table of atoms to check for in client messages, and what handler
 * to call to handle that particular atom.
 */

typedef struct {
    char do_not_create;	/* 0 if creating atom is o.k. */
    Atom atom;		/* Slot number of atom (see define_client_atoms) */
    char *name;		/* Name of the atom for lookup up slot number */
    void (*handler)();	/* Routine that handles this atom */
    } client_atom;

static char client_atoms_inited = 0;

#define n_client_atoms 2
static client_atom client_atoms[n_client_atoms] =
{
#define StuffSelectionAtom client_atoms[0].atom
			0,	0,"STUFF_SELECTION",	i_do_StuffSecondary,
#define KillSelectionAtom client_atoms[1].atom
			0,	0,"KILL_SELECTION",	0
};

/*
 * We come here to define the atom values once at startup so we don't
 * have to keep redefining them during operation.
 */
define_client_atoms(w)
    DECtermWidget w;
{
Display *dpy = XtDisplay(w);
int i;

/*
 * We come here for each DECterm, but the slot values for atoms are
 * constant for a given display.  Hence we use the flag client_atoms_inited to
 * make sure we don't run again and again.
 *
 * We use the flag instead of merely calling define_client_atoms once
 * during controller startup, since the caller might be using a DECterm
 * widget and hence never go through our controller startup.
 */
if (client_atoms_inited) return;
else client_atoms_inited = True;

for (i=0; i<n_client_atoms; i++)
    if ((client_atoms[i].atom = XmInternAtom (dpy, client_atoms[i].name,
					    client_atoms[i].do_not_create))
	== None)
    widget_message (
"DECterm controller failed to get XmInternAtom on the name \"%s\"\n",
	client_atoms[i].name);
}

/*
 * We come here when a client message arrives, such as a delete request
 * or cut-paste request from window manager.
 *
 * Figure out here if we understand the request, dispatching on those
 * we do, and ignoring the rest.
 */

client_event_handler(w, tag, event)
Widget w;
caddr_t tag;
XEvent *event;
{
XClientMessageEvent *cm = (XClientMessageEvent *)event;

int i;

if (event->type != ClientMessage)
	return;

for (i=0; i<n_client_atoms; i++)
  if (cm->message_type == client_atoms[i].atom && client_atoms[i].handler != 0)

	{
	(*client_atoms[i].handler)(w, tag, event);
	break;
	}
}

/* Routine to handle client message for the shell widget.  The original
 * problem was that when title bar is clicked or when DECterm widget is
 * deiconified, the shell widget is the one who got the focus.  Therefore
 * the quick copy event (actually a client message event) is sent to it's
 * shell where this client message event was originally ignored.  By adding
 * this event handler, the quick copy event is handled properly by calling
 * the i_do_StuffSecondary() explicitly.
 *
 * Since we can't guarantee if the shell widget has another client message
 * event handler or not, we can't set the *propagate parameter (the 4th
 * parameter not shown here which is new to X11R4) to False so that we won't
 * block this event.
 */
static void i_shell_client_handler(wid, w, event)
    Widget wid;
    DECtermWidget w;
    XEvent *event;
{
    XClientMessageEvent *cm = (XClientMessageEvent *) event;

    if ( event->type != ClientMessage ||
	 cm->message_type != StuffSelectionAtom )
	return;

    i_do_StuffSecondary(w, NULL, event);
}


/*
 * queue handling routines
 */

static void
queue_init( qh )
    queue_header *qh;
{
    qh->head = NULL;
    qh->tail = (queue_element *) &qh->head;
    qh->count = 0;
}

queue_element *queue_alloc( size )
int size;
{
    queue_element *qe;

    qe = (queue_element *) XtMalloc( size+sizeof(queue_element));
    qe->length = size;
    qe->ptr = qe->data;
    return (qe);
}

#define queue_free( qe ) XtFree( (char *) qe )

static int
queue_insert( qh, qe )
    queue_header *qh;
    queue_element *qe;
{
    qe->link = NULL;
    qh->tail->link = qe;
    qh->tail = qe;
    return (qh->count = qh->count + 1);
}

static int
queue_stuff( qh, qe )		/* stuff at head of queue */
    queue_header *qh;
    queue_element *qe;
{
    if ((qe->link = qh->head) == NULL) {
	qh->tail = qe;
    }
    qh->head = qe;
    return (qh->count = qh->count + 1);
}

static queue_element
*queue_remove( qh )
    queue_header *qh;
{
    queue_element *qe;

    if ((qe = qh->head) != NULL) {
	if ((qh->head = qe->link) == NULL) {
	    qh->tail = (queue_element *) &qh->head;
	}
        qh->count = qh->count - 1;
    }

    return (qe);
}

static void
queue_empty( qh )
    queue_header *qh;
{
    queue_element *qe;

    while ( (qe = queue_remove( qh )) != NULL )
	queue_free( qe );
}

/*
 * i_change_keyboard_control
 *
 * This routine should be called whenever
 *	w->common.autoRepeatEnable
 *	w->input.wait
 *	w->input.hold
 *	w->input.locked
 * changes.  It will set all states to their new value if
 * we have focus; otherwise, it will do nothing.
 */
void
i_change_keyboard_control( w )
    DECtermWidget w;
{
    XKeyboardControl values;

    if (w->input.has_focus) {

/*
 * Do autorepeat mode
 */
	values.auto_repeat_mode = w->common.autoRepeatEnable
		? AutoRepeatModeOn : AutoRepeatModeOff;
	if (!w->common.lockUserFeatures)	
	XChangeKeyboardControl( XtDisplay(w),
				KBAutoRepeatMode,
				&values );
/*
 * Do the wait light.
 */
	values.led_mode = w->input.wait || w->input.locked;
	values.led = WAIT_LED;			/* do one LED at a time */
	XChangeKeyboardControl( XtDisplay(w),
				KBLed | KBLedMode,
				&values );
/*
 * Do the hold light.
 */
	values.led_mode = w->input.hold;
	values.led = HOLD_LED;
	XChangeKeyboardControl( XtDisplay(w), KBLed | KBLedMode,
				&values );
    }
}

static
void add_event_handlers( w )
    DECtermWidget w;
{
    XtAddEventHandler((Widget)w, NoEventMask, TRUE ,_DECwTermMappingHandler,0L);
    XtAddEventHandler((Widget)w, KeyPressMask | KeyReleaseMask, FALSE,
		      i_event_handler, (XtPointer)KEYBOARD_HANDLER);
    XtAddEventHandler((Widget)w, ButtonPressMask | ButtonReleaseMask, FALSE,
		      i_event_handler, (XtPointer)BUTTON_HANDLER);
    XtAddEventHandler( (Widget)w,
		       ButtonMotionMask|Button1MotionMask|
		       Button2MotionMask|Button3MotionMask|Button4MotionMask|
		       Button5MotionMask,
		       TRUE, i_event_handler,
		       (XtPointer)MOVEMENT_HANDLER );
    XtAddEventHandler((Widget)w, FocusChangeMask, FALSE, i_focus_handler, 0);
    XtAddEventHandler((Widget)w, 0, TRUE ,
		      (XtEventHandler)client_event_handler, 0);
    XtAddEventHandler(shell_widget(w), 0, TRUE,
		      (XtEventHandler)i_shell_client_handler, w);
}

static
void remove_event_handlers( w )
    DECtermWidget w;
{
    XtRemoveEventHandler((Widget)w, NoEventMask, TRUE ,
			 _DECwTermMappingHandler, 0);
    XtRemoveEventHandler((Widget)w, KeyPressMask | KeyReleaseMask,
			 FALSE, i_event_handler, (XtPointer)KEYBOARD_HANDLER);
    XtRemoveEventHandler((Widget)w, ButtonPressMask | ButtonReleaseMask, FALSE,
			 (XtEventHandler)i_event_handler,
			 (XtPointer)BUTTON_HANDLER);
    XtRemoveEventHandler( (Widget)w,
		       PointerMotionMask|ButtonMotionMask|Button1MotionMask|
		       Button2MotionMask|Button3MotionMask|Button4MotionMask|
		       Button5MotionMask,
		       TRUE, i_event_handler,
		       (XtPointer)MOVEMENT_HANDLER );
    XtRemoveEventHandler((Widget)w, FocusChangeMask, FALSE, i_focus_handler, 0);
    XtRemoveEventHandler((Widget)w, 0, TRUE,
			 (XtEventHandler)client_event_handler, 0);
    XtRemoveEventHandler(shell_widget(w), 0, TRUE,
			 (XtEventHandler)i_shell_client_handler, w);
}


static
void empty_all_queues( w )
    DECtermWidget w;
{
    if (w->input.current_qe != NULL) {
	queue_free( w->input.current_qe );
        w->input.current_qe = NULL;
    }
    queue_empty( &w->input.keyboard_queue );
    queue_empty( &w->input.report_queue );
    queue_empty( &w->input.paste_queue );
}

static int hex_convert( value )
    char value;
{
    switch (value) {
	case '0' : return (0);
	case '1' : return (1);
	case '2' : return (2);
	case '3' : return (3);
	case '4' : return (4);
	case '5' : return (5);
	case '6' : return (6);
	case '7' : return (7);
	case '8' : return (8);
	case '9' : return (9);
	case 'a' :
	case 'A' : return (10);
	case 'b' :
	case 'B' : return (11);
	case 'c' :
	case 'C' : return (12);
	case 'd' :
	case 'D' : return (13);
	case 'e' :
	case 'E' : return (14);
	case 'f' :
	case 'F' : return (15);
    }
}

#if defined(VMS_DECTERM)

		/* for dynamic image activation, 911030, TN350 */
static unsigned int exception_handler( sigargs, mchargs )
    unsigned long sigargs[];
    unsigned long mchargs[5];
{
    return( SS$_CONTINUE );
}

static DXmAIMWidget _DXmCreateAIM( w, name, al, ac )
    Widget w;
    char *name;
    Arg *al;
    int ac;
{
    return( NULL );
}

static void _DXmAIMGetRIMPList( w, name_list, count )
    DXmAIMWidget w;
    char ***name_list;
    int *count;
{
    ***name_list = NULL;
    *count = 0;
}

static Boolean _DXmAIMIsRIMPAvailable( w, name )
    DXmAIMWidget w;
    char *name;
{
    return( True );
}

static void i_activate_aimshr()
{
    $DESCRIPTOR( file_name,			"DECW$DXMAIMSHR" );
    $DESCRIPTOR( create_aim_symbol,		"DXmCreateAIM" );
    $DESCRIPTOR( get_rimplist_symbol,		"DXmAIMGetRIMPList" );
    $DESCRIPTOR( is_rimp_available_symbol,	"DXmAIMIsRIMPAvailable" );

    VAXC$ESTABLISH( exception_handler );
    if ( SS$_NORMAL != LIB$FIND_IMAGE_SYMBOL(	&file_name,
						&create_aim_symbol,
						&create_aim_ptr,
						NULL ))
	create_aim_ptr = _DXmCreateAIM;
    if ( SS$_NORMAL != LIB$FIND_IMAGE_SYMBOL(	&file_name,
						&get_rimplist_symbol,
						&get_rimplist_ptr,
						NULL ))
	get_rimplist_ptr = _DXmAIMGetRIMPList;
    if ( SS$_NORMAL != LIB$FIND_IMAGE_SYMBOL(	&file_name,
						&is_rimp_available_symbol,
						&is_rimp_available_ptr,
						NULL ))
	is_rimp_available_ptr = _DXmAIMIsRIMPAvailable;
    LIB$REVERT();
}
#else
static DXmAIMWidget DXmCreateAIM( w, name, al, ac )
    Widget w;
    char *name;
    Arg *al;
    int ac;
{
    return( NULL );
}

static void DXmAIMGetRIMPList( w, name_list, count )
    DXmAIMWidget w;
    char ***name_list;
    int *count;
{
    ***name_list = NULL;
    *count = 0;
}

static Boolean DXmAIMIsRIMPAvailable( w, name )
    DXmAIMWidget w;
    char *name;
{
    return( True );
}
#endif /* VMS_DECTERM */

/* 910830, TN200 */
static char *lang_to_type( lang )
    char *lang;
{
    static char *type[5] = { "", "Kanji", "Hanzi", "Hangul", "Hanyu" };
    if ( !strncmp( lang, "ja_JP", 5 ))
	return( type[1] );
    else if ( !strncmp( lang, "zh_CN", 5 ))
	return( type[2] );
    else if ( !strncmp( lang, "ko_KR", 5 ))
	return( type[3] );
    else if ( !strncmp( lang, "zh_TW", 5 ))
	return( type[4] );
    else
	return( type[0] );
}

/* 910627, TN005a */
static Boolean i_rimp_exist( w, lang )
    DXmAIMWidget w;
    char *lang;
{
    Boolean exist;
    int count, i;
    char **rimp_list;

    if ( w && lang) {
	exist = False;
	DXmAIMGetRIMPList( w, &rimp_list, &count );
	for ( i = 0; i < count; i++ ) {
	    if ( !strncmp( lang, rimp_list[i], 5 ))
		exist = DXmAIMIsRIMPAvailable( w, rimp_list[i] );
	    XtFree( rimp_list[i] );
	}	   
    } else if ( !lang )
	exist = True;
    else
	exist = False;	/* returns False in case aim widget does not exist */
    return( exist );
}

/* 910507, TN005a */
static void i_create_aim( w )
    DECtermWidget w;
{
    Arg arglist[20];
    int ac;
    char *lang;
    XmFontList fontlist = DXmFontListCreateDefault( w, NULL );
    XtCallbackRec draw_intermediate_char_cb[2],
		  get_current_cursor_char_cb[2],
		  pre_edit_draw_cb[2],
		  pre_edit_start_cb[2],
		  query_xy_position_cb[2],
		  rimp_destroy_cb[2],
		  sec_pre_edit_start_cb[2],
		  set_cursor_position_cb[2];

#define INIT_AIM_CALLBACK( m, p, t ) \
	m[0].callback = p; \
	m[0].closure  = (caddr_t)t; \
	m[1].callback = NULL

    INIT_AIM_CALLBACK(draw_intermediate_char_cb, draw_intermediate_char, w);
    INIT_AIM_CALLBACK(get_current_cursor_char_cb, get_current_cursor_char, w);
    INIT_AIM_CALLBACK(pre_edit_draw_cb, pre_edit_draw, w);
    INIT_AIM_CALLBACK(pre_edit_start_cb, pre_edit_start, w);
    INIT_AIM_CALLBACK(query_xy_position_cb, query_xy_position, w);
    INIT_AIM_CALLBACK(rimp_destroy_cb, rimp_destroy, w);
    INIT_AIM_CALLBACK(sec_pre_edit_start_cb, sec_pre_edit_start, w);
    INIT_AIM_CALLBACK(set_cursor_position_cb, set_cursor_position, w);

#undef INIT_AIM_CALLBACK

    ac = 0;
    lang = XtMalloc( 6 );
    if ( w->source.wvt$l_ext_flags & vte1_m_bobcat ) {
	strcpy( lang, "zh_CN" );
    } else if ( w->source.wvt$l_ext_flags & vte1_m_dickcat ) {
	strcpy( lang, "ko_KR" ); 
    } else if ( w->source.wvt$l_ext_flags & vte1_m_fishcat ) {
	strcpy( lang, "zh_TW" );
    } else {
	XtFree( lang );
	lang = NULL;
    }
    XtSetArg( arglist[ac], DXmNaimLanguage, lang ); ac++;
    XtSetArg( arglist[ac], DXmNaimUserDefinedIM, NULL ); ac++;
    XtSetArg( arglist[ac], DXmNaimDrawIntermediateChar,
	draw_intermediate_char_cb ); ac++;
    XtSetArg( arglist[ac], DXmNaimGetCurrentCursorChar,
	get_current_cursor_char_cb ); ac++;
    XtSetArg( arglist[ac], DXmNaimPreEditDraw, pre_edit_draw_cb ); ac++;
    XtSetArg( arglist[ac], DXmNaimPreEditStart, pre_edit_start_cb ); ac++;
    XtSetArg( arglist[ac], DXmNaimQueryXYPosition, query_xy_position_cb );
	ac++;
    XtSetArg( arglist[ac], DXmNaimRimpDestroy, rimp_destroy_cb ); ac++;
    XtSetArg( arglist[ac], DXmNaimSecPreEditStart, sec_pre_edit_start_cb );
	ac++;
    XtSetArg( arglist[ac], DXmNaimSetCursorPosition, set_cursor_position_cb );
	ac++;
    XtSetArg( arglist[ac], DXmNaimTextWidgetFontList, fontlist ); ac++;
    w->input.aim = DXmCreateAIM( w, "AIM", arglist, ac );
    if ( w->input.aim )
	XtRealizeWidget( (Widget)w->input.aim );
    if ( !i_rimp_exist( w->input.aim, lang ))
	widget_error( w, DECW$K_MSG_NO_INPUT_METHOD, 0, lang_to_type( lang ));
    if ( lang )
	XtFree( lang );
}

void i_reset_aim( w )
    DECtermWidget w;
{
    char *lang;
    Arg arglist[1];

    lang = XtMalloc( 6 );
    if ( w->source.wvt$l_ext_flags & vte1_m_bobcat )
	strcpy( lang, "zh_CN" );
    else if ( w->source.wvt$l_ext_flags & vte1_m_dickcat )
	strcpy( lang, "ko_KR" );
    else if ( w->source.wvt$l_ext_flags & vte1_m_fishcat )
	strcpy( lang, "zh_TW" );
    else {
	XtFree( lang );
	lang = NULL;
    }
    if ( w->input.aim ) {
	XtSetArg( arglist[0], DXmNaimLanguage, lang );
	XtSetValues( (Widget)w->input.aim, arglist, 1 );
    }
    if ( !i_rimp_exist( w->input.aim, lang ))
	widget_error( w, DECW$K_MSG_NO_INPUT_METHOD, 0, lang_to_type( lang ));
    if ( lang )
	XtFree( lang );
}

static void draw_intermediate_char(w, tag, cb_data)
    Widget w;
    int *tag;
    XmAnyCallbackStruct *cb_data;
{
    DXmAIMWidget aim = (DXmAIMWidget) w;
    DECtermWidget widget = (DECtermWidget) tag;
    DXmAIMCallbackStruct *data = (DXmAIMCallbackStruct *) cb_data;
    XChar2b string16[1];
#if 1
    XmStringContext context;
    XmStringCharSet charset;
    XmStringDirection dir;
    Boolean sep;
    char *buf = NULL;
/*    long count, status;	*/
#endif

    if ( !( widget->source.wvt$l_ext_flags & vte1_m_dickcat )) {
	XmStringFree( data->new_string );
	return;
    } else if ( !data->new_string ) {
	string16[0].byte1 = string16[0].byte2 = 0;
	o_draw_intermediate_char( widget, string16 );
    } else {
#if 1
/*	buf = DXmCvtCStoFC( data->new_string, &count, &status );	*/
	if ( XmStringInitContext( &context, data->new_string )) {
	    if ( !XmStringGetNextSegment( context, &buf, &charset, &dir, &sep ))
		buf = NULL;
	    XmStringFreeContext( context );
	}
	if ( buf ) {
#endif
	if ( widget->source.wvt$l_ext_specific_flags & vte2_m_intermediate_char ) {
#if 1
	    string16[0].byte1 = *buf;
	    string16[0].byte2 = *(buf + 1);
#else
	    string16[0].byte1 = data->new_string[28];
	    string16[0].byte2 = data->new_string[29];
#endif
	    o_draw_intermediate_char( widget, string16 );
	} else {
#if 1
	    widget->input.string16[0].byte1 = *buf;
	    widget->input.string16[0].byte2 = *(buf + 1);
#else
	    widget->input.string16[0].byte1 = data->new_string[28];
	    widget->input.string16[0].byte2 = data->new_string[29];
#endif
	}
#if 1
	XtFree( buf );
	}
#endif
    XmStringFree( data->new_string );
    }
}
    
static void get_current_cursor_char(w, tag, cb_data)
    Widget w;
    int *tag;
    XmAnyCallbackStruct *cb_data;
{
    DXmAIMWidget aim = (DXmAIMWidget) w;
    DECtermWidget widget = (DECtermWidget) tag;
    DXmAIMCallbackStruct *data = (DXmAIMCallbackStruct *) cb_data;
#if 1
    static unsigned char value[3];
    XmStringCharSet charset = "KSC5601.1987-1";
/*    int count, status;	*/
#else
    static unsigned char cs_ko_kr[30] =
		      { 0xdf, 0xff, 0x79, 0x1e, 0x03, 0x01, 0x00, 0x00,
			0x01, 0x0e, 0x00, 0x4b, 0x53, 0x43, 0x35, 0x36,
			0x30, 0x31, 0x2e, 0x31, 0x39, 0x38, 0x37, 0x2d,
			0x31, 0x02, 0x02, 0x00, 0x00, 0x00 };
    unsigned char *cs;
#endif
    s_line_structure *ls = s_read_data( widget, widget->output.cursor_row );
    int column = widget->output.cursor_column;

    if ( !( widget->source.wvt$l_ext_flags & vte1_m_dickcat ))
	return;
    if ( column == ( widget->output.left_visible_column +
		     widget->output.visible_columns ))
	column--;
#if 1
    value[0] = ls->a_code_base[column++];
    value[1] = ls->a_code_base[column];
    value[2] = '\0';
/*    data->new_string = DXmCvtFCtoCS( value, &count, &status );	*/
    data->new_string = XmStringCreate( value, charset );
#else
    cs = (unsigned char *) XtMalloc( 30 );
    memcpy( cs, cs_ko_kr, 30 );
    cs[28] = ls->a_code_base[column++];
    cs[29] = ls->a_code_base[column];
    data->new_string = (XmString) cs;
#endif
}

static void pre_edit_draw(w, tag, cb_data)
    Widget w;
    int *tag;
    XmAnyCallbackStruct *cb_data;
{
    DXmAIMWidget aim = (DXmAIMWidget) w;
    DECtermWidget widget = (DECtermWidget) tag;
    DXmAIMCallbackStruct *data = (DXmAIMCallbackStruct *) cb_data;
    DECwTermInputCallbackStruct call_data;
#if 1
    XmStringContext context;
    XmStringCharSet charset;
    XmStringDirection dir;
    Boolean sep;
    char *buf = NULL, *leading_code = "\302\313";
    int count;
/*    int status;	*/
#else
    int count;
    char buf[4];
#endif

    if ( !data->new_string )
	return;
#if 1
    if ( widget->source.wvt$l_ext_flags & vte1_m_dickcat )
	widget->source.wvt$l_ext_specific_flags &= ~vte2_m_intermediate_char;
/*    buf = DXmCvtCStoFC( data->new_string, &count, &status );	*/
/*    if ( status == DXmCvtStatusOK ) {				*/
    if ( XmStringInitContext( &context, data->new_string )) {
	if ( !XmStringGetNextSegment( context, &buf, &charset, &dir, &sep ))
	    buf = NULL;
	XmStringFreeContext( context );
    }
    if ( buf ) {
	if ( !strcmp( charset, "DEC.DTSCS.1990-2" )) {
	    char *tmp = buf;
	    buf = XtMalloc( 5 );
	    strcpy( buf, leading_code );
	    strcat( buf, tmp );
	    XtFree( tmp );
	}
	count = strlen( buf );
#else
    if ( widget->source.wvt$l_ext_flags & vte1_m_bobcat ) {
	count = (int)data->new_string[25];
	buf[0] = (char)( data->new_string[27] |= 128 );
	buf[1] = (char)( data->new_string[28] |= 128 );
    } else if ( widget->source.wvt$l_ext_flags & vte1_m_dickcat ) {
	widget->source.wvt$l_ext_specific_flags &= ~vte2_m_intermediate_char;
	count = (int)data->new_string[26];
	buf[0] = (char)( data->new_string[28] |= 128 );
	buf[1] = (char)( data->new_string[29] |= 128 );
    } else if ( widget->source.wvt$l_ext_flags & vte1_m_fishcat ) {
	if ( (char)data->new_string[03] == 0x20 ) {
	    count = (int)data->new_string[28] + 2;
	    buf[0] = 0xc2;
	    buf[1] = 0xcb;
	    buf[2] = (char)data->new_string[30];
	    buf[3] = (char)data->new_string[31];
	} else if ( (char)data->new_string[03] == 0x23 ) {
	    count = (int)data->new_string[31];
	    buf[0] = (char)data->new_string[33];
	    buf[1] = (char)data->new_string[34];
	} else
	    return;
    } else
	return;
#endif
	call_data.reason = DECwCRInput;
	call_data.data = buf;
	call_data.count = count;
	XtCallCallbacks( (Widget)widget, DECwNinputCallback, &call_data );
#if 1
	XtFree( buf );
    }
#endif
    XmStringFree( data->new_string );
}

static void pre_edit_start(w, tag, cb_data)
    Widget w;
    int *tag;
    XmAnyCallbackStruct *cb_data;
{
}
    
static void query_xy_position(w, tag, cb_data)
    Widget w;
    int *tag;
    XmAnyCallbackStruct *cb_data;
{
}

static void rimp_destroy(w, tag, cb_data)
    Widget w;
    int *tag;
    XmAnyCallbackStruct *cb_data;
{
    DXmAIMWidget aim = (DXmAIMWidget) w;
    DECtermWidget widget = (DECtermWidget) tag;
    DXmAIMCallbackStruct *data = (DXmAIMCallbackStruct *) cb_data;
    Arg arglist[1];
    char *lang;

    XtSetArg( arglist[0], DXmNaimLanguage, &lang );
    XtGetValues( (Widget)aim, arglist, 1 );
    widget_error( widget, DECW$K_MSG_NO_INPUT_METHOD, 0, lang_to_type( lang ));
}

static void sec_pre_edit_start(w, tag, cb_data)
    Widget w;
    int *tag;
    XmAnyCallbackStruct *cb_data;
{
}
    
static void set_cursor_position(w, tag, cb_data)
    Widget w;
    int *tag;
    XmAnyCallbackStruct *cb_data;
{
}

void i_initialize( w )
    DECtermWidget w;
{
    w->input.motion_handler = NULL;
    w->input.button_handler = NULL;
    w->input.input_handler = NULL;
    w->input.has_focus = FALSE;
    w->input.selection_in_progress = FALSE;
    memset (&w->input.compose_status, 0, sizeof (w->input.compose_status));
    w->input.hold = FALSE;
    w->input.wait = FALSE;
    w->input.locked = FALSE;
    w->input.suspended = FALSE;
    w->input.current_qe = NULL;
    w->input.key_pressed = FALSE;
#ifdef SECURE_KEYBOARD
    w->input.keyboard_grabbed = FALSE;
#endif
    w->input.hex_count = 0;
    w->input.hex_enable = FALSE;
    w->input.aim = NULL;	/* 910611, TN005a */
    w->input.string16[0].byte1 = 0;
    w->input.string16[0].byte2 = 0;
    queue_init( &w->input.keyboard_queue );
    queue_init( &w->input.report_queue );
    queue_init( &w->input.paste_queue );

    if ( w->common.inputCallback != NULL )
	add_event_handlers( w );
    w->common.whiteSpaceCharacters =
	XtNewString( ( w->common.whiteSpaceCharacters != NULL ) ?
			   w->common.whiteSpaceCharacters : "" );
    Selection_direction(w) = FALSE;
    define_client_atoms (w);
}

void i_realize( w )
    DECtermWidget w;
{
#if defined (VMS_DECTERM)
    i_activate_aimshr();
#endif
    if XtIsRealized( w )
	i_create_aim( w );

#ifdef SECURE_KEYBOARD
    i_set_value_secureKeyboard( w, w );
#endif
}

void i_clear_comm( w )
    DECtermWidget w;
{
    empty_all_queues( w );

    w->input.suspended = FALSE;
    w->input.hold = FALSE;
    w->input.wait = FALSE;
    w->input.locked = FALSE;
    w->input.key_pressed = FALSE;
    i_change_keyboard_control( w );
    s_start_output( w, STOP_OUTPUT_REPORT | STOP_OUTPUT_HOLD );
}

void i_destroy( w )
    DECtermWidget w;
{
    remove_event_handlers( w );

    empty_all_queues( w );

    XtFree( w->common.whiteSpaceCharacters );

    if ( w->input.aim )
	XtDestroyWidget( (Widget)w->input.aim );

}

void i_set_value_inputCallback( oldw, neww )
    DECtermWidget oldw, neww;
{

    
    DecTermUpdateCallback ( oldw, &(oldw->common.inputCallback),
                       neww, &(neww->common.inputCallback),
		       DECwNinputCallback );

    if ( neww->common.inputCallback != NULL ) {
	add_event_handlers( neww );
    } else {
	remove_event_handlers( neww );
	i_clear_comm( neww );
    }
}

void i_set_value_whiteSpace( oldw, neww )
    DECtermWidget oldw, neww;
{
    XtFree( oldw->common.whiteSpaceCharacters );

    neww->common.whiteSpaceCharacters =
	XtNewString( ( neww->common.whiteSpaceCharacters != NULL ) ?
			   neww->common.whiteSpaceCharacters : "" );
}

#ifdef SECURE_KEYBOARD
void i_set_value_secureKeyboard( oldw, neww )
    DECtermWidget oldw, neww;
{
    if (!XtIsRealized(neww)) return;

    if ( neww->common.secureKeyboard != neww->input.keyboard_grabbed ) {
	if ( neww->common.secureKeyboard ) {
	    /* attempt to grab keyboard */
	    if (XGrabKeyboard( XtDisplay(neww), XtWindow(neww),
                          True, GrabModeAsync, GrabModeAsync, CurrentTime )
              == GrabSuccess) {
		neww->input.keyboard_grabbed = TRUE;
		neww->common.reverseVideo = TRUE;
		o_set_value_reverseVideo( oldw, neww );
	    } else {
		neww->common.secureKeyboard = FALSE;
		XBell( XtDisplay( neww ), 100 );
	    }	
	} else {
	    /* ungrab keyboard */
	    XUngrabKeyboard( XtDisplay( neww ), CurrentTime );
	    neww->input.keyboard_grabbed = FALSE;
	    neww->common.reverseVideo = TRUE;
	    o_set_value_reverseVideo( oldw, neww );
	}
    }
}
#endif

static void i_focus_handler( w, tag, event )
    DECtermWidget w;
    caddr_t tag;
    XEvent *event;
{
    if ( event->type == FocusIn )
	{
	if ( _DECwTermIsKBHebrew( w ))
	    set_kb_bit( w, TRUE );
	else
	    set_kb_bit( w, FALSE );
/* If there is a pending request for keyboard mapping, handle it.*/
	set_kb_pending( w );
/* if resource is TRUE redisplay here else when Ctrl/Heb detected */
/* but do this only if we really have the focus.*/
	if ( event->xfocus.detail != NotifyPointer )
	    if ( w->common.redisplay7bit )
		DECwTermRedisplay7bit( w );
/*
 * Backed out the following change, since it introduced the "duelling DECterm"
 * bug.
 */
#if 0
	/*
	 * Work around an apparent problem with the Motif window manager: if
	 * the user clicks on the title bar, the DECterm widget gets a
	 * FocusIn event, but XGetInputFocus shows the shell's window as having
	 * the input focus.  This means that quick copy, for example, won't
	 * work, because the ClientMessage will go to the wrong window.
	 */
	if ( ! w->input.has_focus )
	    {
	    Window focus_window;
	    int revert;

	    XGetInputFocus( XtDisplay(w), &focus_window, &revert );
	    if ( focus_window != XtWindow(w) )
		XSetInputFocus( XtDisplay(w), XtWindow(w), RevertToNone,
				CurrentTime );
	    }
#endif
	w->input.has_focus = TRUE;
	i_change_keyboard_control( w );
	o_focus_in (w, tag, event);
	}
    else
        if ( event->type == FocusOut )
	    {
#ifdef SECURE_KEYBOARD
	    if (event->xfocus.mode == NotifyUngrab && w->input.keyboard_grabbed)
		{
		w->input.keyboard_grabbed = FALSE;
		w->common.secureKeyboard = FALSE;
		w->common.reverseVideo = TRUE;
		o_set_value_reverseVideo( w, w );
		XBell( XtDisplay( w ), 100 );
		}
#endif
	    w->input.has_focus = FALSE;
	    o_focus_out (w, tag, event);
	    }
}

Boolean
i_accept_focus( w, timep )
    DECtermWidget w;
    Time *timep;
{
    if ( (w->common.inputCallback != NULL) &&
/*	 (! w->input.has_focus) && */
	 (XtIsRealized(w)) )
    {
	XSetInputFocus( XtDisplay(w), XtWindow(w),
		        RevertToNone, *timep );
	return (TRUE);
    } else {
	return (FALSE);
    }
}

/*
 * i_check_cursor_blink( w )
 *
 * check to see if cursor should be blinking
 *
 * Returns TRUE if cursor should be blinking, i.e.,
 * we have focus or we are an output-only widget.
 */
Boolean
i_check_cursor_blink( w )
    DECtermWidget w;
{
    return ( w->input.has_focus || w->common.inputCallback == NULL );
}

/*
 * enqueue_input_data( w, queue, queue_size, qe )
 *
 * This routine is called by send_key, i_report_data and stuff_proc to put a
 * queue element into one of the input qeuues.  It checks to see if
 * a routine within the DECterm widget has enabled input handling; if so it
 * passes the data to that routine instead of enqueueing it.  This is
 * designed to handle ReGIS one-shot input mode: the regis input handler will
 * first disable input handling (this is important!) and then call
 * i_report_data to return the keystroke and the coordinate in a single
 * atomic report.  i_report_data will then call enqueue_input_data to put
 * the report into the report buffer.
 *
 * This routine returns False if the input request would overflow the queue;
 * otherwise it returns True.
 */
static Boolean enqueue_input_data( w, queue, queue_size, qe )
    DECtermWidget w;
    queue_header *queue;
    int queue_size;
    queue_element *qe;
{
    if ( w->input.input_handler != NULL )
	if ( (*w->input.input_handler)(w, qe->ptr, qe->length ) )
	    {	/* handler accepted event so throw away queue element */
	    queue_free( qe );
	    return True;
	    }
    if ( queue_insert( queue, qe ) >= queue_size )
	return False;	/* overflow! */
    else
	{	/* we have some more input for the application */
	send_more_input( w );
	return True;
	}
}

/*
 * send_more_input( w )
 *	Send more input from one of the input queues
 *
 * This routine is called whenever data has been inserted in any of the
 * input queues.  If there is data pending to be sent and input
 * has not been suspended by the application, we will send it with
 * the inputCallback.
 *
 * The rules are:
 *   - We will never send more than one queue element in a single callback.
 *   - We will never send more than maxInput characters in a single callback.
 *   - We will never break up a queue element from the keyboard or report queue.
 *   - We always process keyboard data first, then report data, then paste data.
 */
static void
send_more_input( w )
    DECtermWidget	w;
{
    DECwTermInputCallbackStruct call_data;
    char *d;
    queue_element *qe;
    Boolean from_paste_queue;
    Boolean allocated_str;
    int max_count;
#define LOCAL_STR_SIZE 40
    char local_str[LOCAL_STR_SIZE];

    if ( w->common.inputCallback == NULL )
	return;

    while ( !w->input.suspended ) {

        from_paste_queue = FALSE;
        if ( (qe = w->input.current_qe) != NULL ) {
	    w->input.current_qe = NULL;
        } else if ( (qe = queue_remove( &w->input.keyboard_queue )) != NULL ) {
	    if (w->input.wait)
		{
		w->input.wait = FALSE;
		i_change_keyboard_control( w );
		}
	} else if ( (qe = queue_remove( &w->input.report_queue )) != NULL ) {
	    if ( w->input.report_queue.count == 0 )
		s_start_output( w, STOP_OUTPUT_REPORT );
	} else if ( (qe = queue_remove( &w->input.paste_queue )) != NULL ) {
	    from_paste_queue = TRUE;
	} else
	    break;

        allocated_str = False;
	max_count = w->common.maxInput > 2 ? w->common.maxInput : 2;

        if ( w->common.terminalMode == DECwVT300_8_BitMode ) {
	    call_data.data = qe->ptr;
	    call_data.count = qe->length < max_count ? qe->length : max_count;
	    qe->ptr += call_data.count;
	    qe->length -= call_data.count;
	} else {

/* expand C1 controls into ESC Fe form */

	    max_count = (qe->length*2) < max_count ? (qe->length*2) : max_count;
	    if ( max_count <= LOCAL_STR_SIZE ) {
	        call_data.data = local_str;
	    } else {
		call_data.data = XtMalloc( max_count );
		allocated_str = True;
	    }
	    for ( d = call_data.data;
		  qe->length > 0 && d < call_data.data + max_count;
		  qe->ptr++, qe->length-- )
	    {
	        if ( '\200' <= *qe->ptr && *qe->ptr < '\240' ) {
		    *d++ = '\33';
		    *d++ = *qe->ptr - '\100';
		} else {
		    *d++ = *qe->ptr;
		}
	    }
	    call_data.count = d - call_data.data;
	}

        call_data.reason = DECwCRInput;
        XtCallCallbacks( (Widget)w, DECwNinputCallback, &call_data );

        if ( allocated_str )
	    XtFree( call_data.data );

	if (qe->length == 0) {
	    queue_free( qe );
	} else {
	    if (from_paste_queue) {
		queue_stuff( &w->input.paste_queue, qe );
	    } else {
		w->input.current_qe = qe;
	    }
	}

    } /* while not suspended */
}

/*
 * send_key( w, event, keysym )
 *	A code-generating key has been pressed
 */
static void
send_key( w, event, keysym )
    DECtermWidget	w;
    XKeyPressedEvent	*event;
    KeySym		keysym;
{
    queue_element *qe;
    int len;

    qe = queue_alloc( MAX_KEYCODE_LENGTH );
    
    KEY_input( w, event, keysym, qe->data, &w->input.compose_status, &len );


#ifdef MEASURE_KEYBOARD_PERFORMANCE
  if (len>0) {
    if ( qe->data[0] == '+' )
	{
	key_enabled = True;
	key_max = 0;
	key_min = 0x7fffffff;
	key_total = 0;
	key_num = 0;
	}
    else if ( qe->data[0] == '-' )
	{
	key_enabled = False;
	}
    if ( key_enabled )
	{
	if ( qe->data[0] == '=' )
	    {
	    char buf[80];
	    key_enabled = False;
	    sprintf( buf, "\33[H\33[J\t\tKeyboard Performance\r\n\n" );
	    DECwTermPutData( w, buf, strlen( buf ) );
	    sprintf( buf, "Average time: %f milliseconds\r\n", ( key_num > 0 )
	      ? ( (float) key_total / (float) key_num ) : 0.0 );
	    DECwTermPutData( w, buf, strlen( buf ) );
	    sprintf( buf, "Maximum time: %d milliseconds\r\n", key_max );
	    DECwTermPutData( w, buf, strlen( buf ) );
	    sprintf( buf, "Minimum time: %d milliseconds\r\n", key_min );
	    DECwTermPutData( w, buf, strlen( buf ) );
	    return;
	    }
	ftime( &key_time );
	key_char = qe->data[0];
	}
  }
#endif

    {
    wvtp ld = w_to_ld( w );

    if ( (_cld wvt$l_vt200_common_flags & vtc1_m_echo_mode) &&
	 !(_cld wvt$w_print_flags & pf1_m_prt_controller_mode) )
	DECwTermPutData( w, qe->data, len );
    }

    if ( (qe->length = len) > 0 ) {
	if (w->input.wait || w->input.locked) {
	    XBell( XtDisplay(w), 0 );
	    queue_free( qe );
	} else {
            w->input.key_pressed = TRUE;
	    if ( ! enqueue_input_data( w, &w->input.keyboard_queue,
			MAX_KEYBOARD_QUEUE, qe ) ) {
	        w->input.wait = TRUE;
	        i_change_keyboard_control( w );
	    }
	}
    } else {
	queue_free( qe );
    }
}


/*
 * hold_key_pressed( w, event )
 *	The Hold Screen key has been pressed
 *	  or Ctrl-S or Ctrl-Q have been pressed.
 */
static void
hold_key_pressed( w, event, keysym )
    DECtermWidget	w;
    XKeyPressedEvent	*event;
    KeySym		keysym;
{
    if ( keysym == XK_Q || keysym == XK_q )
	w->input.hold = FALSE;
    else if ( keysym == XK_S || keysym == XK_s )
	w->input.hold = TRUE;
    else
	w->input.hold = !w->input.hold;

    if ( w->input.hold )
	s_stop_output( w, STOP_OUTPUT_HOLD );
    else
	s_start_output( w, STOP_OUTPUT_HOLD );

    i_change_keyboard_control( w );
}

/*
 * i_start/do/end_selection -- handle button events for selection
 */
#define	BOL(w,p)  DECtermPosition_position( 0, DECtermPosition_row(p))
#define	EOL(w,p)  DECtermPosition_position( (w)->common.columns+1, \
					    DECtermPosition_row(p))
static DECtermPosition BeginningOfWord(w, pos)
DECtermWidget w;
DECtermPosition pos;
{
	register int offset = DECtermPosition_column(pos);
	register char *line;
	register char *match;
	register s_line_structure *ls;
	
	ls = s_read_data( w, DECtermPosition_row(pos));
	line = (char *)ls->a_code_base;
	for(;offset >= 0; offset --) {
		for(match = w->common.whiteSpaceCharacters;
		     *match;  match++) {
			if(line[offset] == 0 || line[offset] == *match) {
				if(offset <  DECtermPosition_column(pos))
				  offset ++;
				return(DECtermPosition_position(offset,
		       			DECtermPosition_row(pos))); 
			}
		}
	}
	return(DECtermPosition_position(0,DECtermPosition_row(pos)));
	
}

static DECtermPosition EndOfWord(w,pos)
DECtermWidget w;
DECtermPosition pos;
{
	register int offset = DECtermPosition_column(pos);
	register char *line;
	register char *match;
	register s_line_structure *ls;
	register int row = DECtermPosition_row(pos) ;

	ls = s_read_data( w, row);
	line = (char *)ls->a_code_base;
#ifdef notdef
	if(match = strpbrk(&line[offset],w->common.whiteSpaceCharacters))
	  offset = match - line;
	else offset = w->common.columns;
	return(DECtermPosition_position(offset,
		       			DECtermPosition_row(pos)));
#else
	for(;offset < ls->w_widths; offset ++) {
		if(line[offset] == 0) {
			return(DECtermPosition_position(offset,
	       			DECtermPosition_row(pos)));
		}
		for(match =  w->common.whiteSpaceCharacters;
		     *match;  match++) {
			if(line[offset] == *match) {
				return(DECtermPosition_position(offset,
		       			DECtermPosition_row(pos))); 
			}
		}
	}
#endif
	return(DECtermPosition_position(w->common.columns,
					DECtermPosition_row(pos)));
}
static void i_extend_selection( w, event )
    DECtermWidget w;
    XEvent *event;
{
    DECtermPosition pos, temp, end;

    Selection_in_Progress(w) = Last_Selection(w);
    pos = o_convert_XY_to_position( w, event->xbutton.x +
		      w->common.displayWidthInc / 2, event->xbutton.y );
    w->input.selection_extend =  TRUE;
    if(pos < Selection_begin(w)) {
	    Extend_save_end(w) = Selection_begin(w);
	    Selection_direction(w) = FALSE;
	    switch(Selection_in_Progress(w)) {
		  case 1:
		    Selection_begin(w) = pos;
		    break;
		  case 2:
		    Selection_begin(w) = BeginningOfWord(w, pos);
		    break;
		  case 3:
		    Selection_begin(w) = BOL(w, pos);
	    }
    } else if(pos > Selection_end(w)) {
	    Extend_save_end(w) = Selection_end(w);
	    Selection_direction(w) = TRUE;
	    Selection_begin(w);
	    switch(Selection_in_Progress(w)) {
		  case 1:
		    Selection_end(w) = pos;
		    break;
		  case 2:
		    Selection_end(w) = EndOfWord(w, pos);
		    break;
		  case 3:
		    Selection_end(w) = EOL(w, pos);
	    }
    } else {
	    if( ( pos -Selection_begin(w) ) > 
	       ( ( Selection_end(w) - Selection_begin(w) )/2)  ) {
		    Extend_save_end(w) = Selection_end(w);
		    Selection_direction(w) = TRUE;
		    switch(Selection_in_Progress(w)) {
			  case 1:
			    Selection_end(w) = pos;
			    break;
			  case 2:
			    Selection_end(w) = EndOfWord(w, pos);
			    break;
			  case 3:
			    Selection_end(w) = EOL(w, pos);
		    }
	    } else { 
		    Extend_save_end(w) = Selection_begin(w);
		    Selection_direction(w) = FALSE;
		    switch(Selection_in_Progress(w)) {
			  case 1:
			    Selection_begin(w) = pos;
			    break;
			  case 2:
			    Selection_begin(w) = BeginningOfWord(w, pos);
			    break;
			  case 3:
			    Selection_begin(w) = BOL(w, pos);
		    }
	    }
    }
    s_set_selection(w, Selection_begin(w),Selection_end(w),
		    event->xbutton.time, Selection_type(w));
}

static void i_do_selection( w, event )
    DECtermWidget w;
    XEvent *event;
{
    DECtermPosition fixed, end;
    int thresholdx, thresholdy;
    Boolean dir;
    if ( Selection_in_Progress( w ))
    {
	end = o_convert_XY_to_position( w, event->xbutton.x +
		      w->common.displayWidthInc / 2, event->xbutton.y );

	if(w->input.selection_extend) {
		if ( (Selection_direction(w) == TRUE) 
		    && (end < Selection_begin(w)) ) {
			Selection_end(w) = Extend_save_end(w);
			Extend_save_end(w) = Selection_begin(w);
			dir = Selection_direction(w) !=Selection_direction(w);
		} else  if( (Selection_direction(w) == FALSE) 
			   && (end > Selection_end(w))){
			Selection_begin(w) = Extend_save_end(w);
			Extend_save_end(w) =  Selection_begin(w);
			dir = Selection_direction(w) !=Selection_direction(w);
		} else dir = Selection_direction(w);
		fixed = dir ? Selection_begin(w) : Selection_end(w);
	} else { 
		if (end < Selection_begin(w)) 
		  dir = FALSE;
		else
		  dir = TRUE;
		fixed = Selection_begin(w);
	}
		
	switch(Selection_in_Progress(w)) {
	      case 1:
		if(!Selection_confirmed(w)) {
			thresholdx = w->common.selectThreshold;
			thresholdy = w->common.selectThreshold;
			if ( event->xbutton.x + thresholdx < Selection_x(w)
			    || event->xbutton.x - thresholdx > Selection_x(w)
			    || event->xbutton.y + thresholdy < Selection_y(w)
			    || event->xbutton.y - thresholdy > Selection_y(w)){
				Selection_confirmed(w) = TRUE;
			}  else
				return;
		}
		break;
	      case 2:
		if(dir) {
		  end = EndOfWord(w,end);
		} else {
		  end = BeginningOfWord(w,end);
		  fixed = EndOfWord(w,fixed);
		}
		break;
	      case 3:
		if(dir) {
		  end = EOL(w, end);
		} else {
		  end = BOL(w,end);
		  fixed = EOL(w,fixed);
		}
		break;
	}

	if(dir) {
		s_set_selection( w, fixed, end, event->xbutton.time,
				Selection_type(w));
		Selection_end(w) = end;
	} else{
		s_set_selection( w, end, fixed, event->xbutton.time,
				Selection_type(w));
		if(w->input.selection_extend)
		  Selection_begin(w) = end;
	}
	if(!(w->input.selection_extend))
	  Selection_end(w) = end;
     }
}

static void i_start_selection( w, event, type )
    DECtermWidget w;
    XEvent *event;
    int type;
{
    DECtermPosition pos, temp;

    if (w->common.doubleClickDelay >= (event->xbutton.time - Selection_time(w))
	&& type == 0 ) {
	    Selection_in_Progress(w) = Last_Selection(w) + 1;
	    if(Selection_in_Progress(w) >= 4)
	      Selection_in_Progress(w) = 1;
    } else {
	    Selection_in_Progress(w) = 1;
    }
    pos =  o_convert_XY_to_position( w, event->xbutton.x +
		      w->common.displayWidthInc / 2, event->xbutton.y );
    Selection_type(w) = type;
    Selection_confirmed(w) = FALSE;
    Selection_x(w) = event->xbutton.x;
    Selection_y(w) = event->xbutton.y;
    switch(Selection_in_Progress(w)) {
	  case 1:
            s_set_selection(w, 0, 0, event->xbutton.time, type);
            if (type == 0) s_set_selection(w, 0, 0, event->xbutton.time, 1);
	    Selection_begin(w) = pos;
	    return;
	  case 2:
	    Selection_confirmed(w) = TRUE;
	    Selection_begin(w) = BeginningOfWord(w,Selection_begin(w));
	    Selection_end(w) =  EndOfWord(w,pos);
	    break;
	  case 3:
	    Selection_confirmed(w) = TRUE;
	    Selection_begin(w) = BOL(w, Selection_begin(w));
	    Selection_end(w) = EOL(w, pos);
	    break;
    }
    if(Selection_begin(w) > Selection_end(w)) {
	    Selection_direction(w) = !Selection_direction(w);
	    temp = Selection_end(w);
	    Selection_end(w) = Selection_begin(w);
	    Selection_begin(w) = temp;
    }
    s_set_selection(w, Selection_begin(w),Selection_end(w),
		    event->xbutton.time, Selection_type(w));
}
static void i_do_quick_copy(w, event)
    DECtermWidget w;
    XEvent *event;
{
    XClientMessageEvent cm;
    int revert;

    cm.type = ClientMessage;
    cm.display = XtDisplay(w);
    cm.message_type = StuffSelectionAtom;
    XGetInputFocus(XtDisplay(w), &cm.window, &revert);
    cm.format = 32;
    cm.data.l[0] = XA_SECONDARY;
    cm.data.l[1] = event->xbutton.time;
    XSendEvent(XtDisplay(w), cm.window, TRUE, NoEventMask, (XEvent *)&cm);
}

static void i_send_kill(w, event)
    DECtermWidget w;
    XEvent *event;
{
XClientMessageEvent cm;
    int revert;

    cm.type = ClientMessage;
    cm.display = XtDisplay(w);
    cm.message_type = KillSelectionAtom;
    cm.window = XGetSelectionOwner(XtDisplay(w), XA_PRIMARY);
    cm.format = 32;
    cm.data.l[0] = XA_PRIMARY;
    cm.data.l[1] = event->xbutton.time;
    XSendEvent(XtDisplay(w), cm.window, TRUE, NoEventMask, (XEvent *)&cm);
}

static void i_end_selection( w, event )
    DECtermWidget w;
    XEvent *event;
{
    Selection_time(w) = event->xbutton.time;

    i_do_selection( w, event );
    if(Selection_type(w) == 1) {
	if (event->xbutton.button == Button2)
		/* fix SPR ICA-45693, paste only when button 2 is released */
	    if(Selection_confirmed(w) == TRUE) {
		    i_do_quick_copy(w, event);
	    } else {
		    i_do_stuff(w, event);
		    if(event->xbutton.state & ControlMask) {
			    i_send_kill(w, event);
		    }
	    }
    }
    
    Last_Selection(w) = Selection_in_Progress(w);
    Selection_in_Progress(w) = FALSE;
    w->input.selection_extend = FALSE;
}

static Boolean check_format( target, atom_list, length )
    Atom target;
    Atom *atom_list;
    unsigned long *length;
{
    int i;
    for ( i = 0; i < *length; i++ )
	if ( atom_list[i] == target ) return TRUE;
    return FALSE;
}

/*
 * stuff_proc( ... )
 *	Stuff data into DECterm's input stream
 *
 * This routine is called as a callback from XtGetSelectionValue()
 * or from DECwTermStuff().
 */
static void stuff_proc( w, closure, selectionp, typep, value, lengthp, formatp )
    DECtermWidget w;
    Opaque closure;
    Atom *selectionp;
    Atom *typep;
    char *value;
    int *lengthp;
    int *formatp;
{
    char *cp;
    int i;
    queue_element *qe;
    Atom current = (Atom)closure;
    Atom next;
    Atom XA_TARGETS = XInternAtom( XtDisplay( w ), "TARGETS", FALSE);
    Atom XA_DDIF = XInternAtom( XtDisplay( w ), "DDIF", FALSE);
    Atom XA_COMPOUND_TEXT = XInternAtom( XtDisplay( w ), "COMPOUND_TEXT", FALSE);
    XmString cs;
    long count = 0L, status = 0L;

    if ( *typep == XA_ATOM ) {
	if ( check_format( XA_DDIF, value, lengthp ))
	    next = XA_DDIF;
	else if ( check_format( XA_COMPOUND_TEXT, value, lengthp ))
	    next = XA_COMPOUND_TEXT;
	else if ( check_format( XA_STRING, value, lengthp ))
	    next = XA_STRING;
	else
	    return;
	XtGetSelectionValue((Widget)w, *selectionp, next,
			    (XtSelectionCallbackProc)stuff_proc, (Opaque)next,
			    ( *selectionp == XA_PRIMARY ) ?
			       w->input.primary_selection_time :
			       w->input.secondary_selection_time );
	return;
    }
    if ( *typep != current ) {
	if ( current == XA_TARGETS )
	    next = XA_DDIF;
	else if ( current == XA_DDIF )
	    next = XA_COMPOUND_TEXT;
	else if ( current == XA_COMPOUND_TEXT )
	    next = XA_STRING;
	else
	    return;
	XtGetSelectionValue( (Widget)w, *selectionp, next,
			    (XtSelectionCallbackProc)stuff_proc,
			    (Opaque)next,
			    ( *selectionp == XA_PRIMARY ) ?
			      w->input.primary_selection_time :
			      w->input.secondary_selection_time );
	return;
    }

    if ( (cp = value) == NULL || *lengthp == 0 )
	return;
    if ( w->input.paste_queue.count >= MAX_PASTE_QUEUE ) {
	XBell( XtDisplay(w), 0 );
	return;
    }

    if ( *typep == XA_DDIF )
	cs = DXmCvtDDIFtoCS( value, &count, &status );
    else if ( *typep == XA_COMPOUND_TEXT )
	cs = XmCvtCTToXmString( value );
    else if ( *typep == XA_STRING )
	cs = DXmCvtFCtoCS( value, &count, &status );    
    else
	return;
    if ( *typep == XA_STRING )
	cp = value = DXmCvtCStoOS( cs, lengthp, &status );
    else
	cp = value = DXmCvtCStoFC( cs, lengthp, &status );
    XmStringFree( cs );
    qe = queue_alloc( *lengthp );
    memcpy( qe->data, cp, *lengthp );
 
/* replace all \n with \r */
    if (! w->common.newLineMode) {
	for ( i=0; i<*lengthp; cp++, i++ ) {
	    for ( ; i<*lengthp && *cp != '\n'; cp++, i++ );
	    if ( i<*lengthp ) {
		qe->data[i] = '\r';
	    }
	}
    }

/* Added to ensure that PASTEd text is echoed when local echo is enabled */
    {
    wvtp ld = w_to_ld( w );

    if ( (_cld wvt$l_vt200_common_flags & vtc1_m_echo_mode) &&
	 !(_cld wvt$w_print_flags & pf1_m_prt_controller_mode) )
	DECwTermPutData( w, qe->data, *lengthp );
    }

    XtFree( value );
    enqueue_input_data( w, &w->input.paste_queue, MAX_PASTE_QUEUE, qe );
}

static void i_do_stuff( w, event )
    DECtermWidget w;
    XEvent *event;
{
    Atom XA_DDIF = XInternAtom( XtDisplay( w ), "DDIF", FALSE );
    w->input.primary_selection_time = event->xbutton.time;
    XtGetSelectionValue((Widget)w, XA_PRIMARY, XA_DDIF,
			(XtSelectionCallbackProc)stuff_proc,
			(Opaque)XA_DDIF, w->input.primary_selection_time );
}

static void i_do_StuffSecondary(w, tag, event)
DECtermWidget w;
caddr_t tag;
XEvent *event;
{
    Atom XA_DDIF = XInternAtom( XtDisplay( w ), "DDIF", FALSE );
XClientMessageEvent *cm = (XClientMessageEvent *)event;

#ifdef SECURE_KEYBOARD
    if (w->common.allowQuickCopy)
#endif
    {
	w->input.secondary_selection_time = cm->data.l[1];
	XtGetSelectionValue((Widget)w, XA_SECONDARY, XA_DDIF,
			    (XtSelectionCallbackProc)stuff_proc,
			    (Opaque)XA_DDIF, w->input.secondary_selection_time);
    }
}


            
static void i_event_handler(w, tag, event)
    DECtermWidget w;
    caddr_t tag;
    XEvent *event;
{
    Boolean used_button;
    wvtp ld = w_to_ld( w );

#define buttonDOWN 1
#define buttonUP   0

#ifdef SECURE_KEYBOARD
    if ( event->xany.send_event && !w->common.allowSendEvents ) return;
#endif

    if ( event->type == KeyPress ) {
	KeySym keysym;

	keysym = XLookupKeysym( (XKeyEvent *)event, 0 );

	/*
	 * Hold Screen if: 1. The F1 key is enabled, it is pressed and the
	 *                    Alt key is not down, or
	 *                 2. the Pause key is pressed, or
	 *                 3. the Scroll Lock key is pressed, or
	 *                 4. "Ctrl-Q,Ctrl-S" is enabled, the control key is
	 *                    pressed, and either one of the "Q", "q", "S", or
	 *                    "s" keys is pressed.
	 */
	
        if (((_cld wvt$f1_key_mode == DECwFactoryDefault ||
              _cld wvt$f1_key_mode == DECwLocalFunction ) &&
              keysym == XK_F1 && ! ( event->xkey.state & Mod1Mask )) ||
	     keysym == XK_Pause ||
	     keysym == XK_Scroll_Lock ||
	     ( w->common.controlQSHold &&
	       (event->xkey.state & ControlMask) &&
	       ( keysym == XK_Q || keysym == XK_q ||	/* Ctrl-Q - Unhold */
		 keysym == XK_S || keysym == XK_s ) ) )	/* Ctrl-S - Hold */
	{
	    hold_key_pressed( w, event, keysym );
	}
	/*
	 * Print Screen if: 1. The F2 key is enabled, it is pressed and the
	 *                     Alt key is not down, or
	 *                  2. The Print key is pressed.
	 */
	
	else if (((_cld wvt$f2_key_mode == DECwFactoryDefault ||
	           _cld wvt$f2_key_mode == DECwLocalFunction) &&
	           keysym == XK_F2 && ! ( event->xkey.state & Mod1Mask )) ||
		 keysym == XK_Print ) {
	    if ( event->xkey.state & ShiftMask )
		DECwTermPrintGraphicsScreen( w );
	    else
		DECwTermPrintTextScreen( w );
	}
	else if ( ( keysym == XK_A ||  keysym == XK_a ) && 
		( event->xkey.state & Mod1Mask ) ){
	/*
	 *  <Alt> A toggles the auto print mode, but is ignored in
	 *  printer controller mode.
	 */
	    {
            wvtp ld = w_to_ld( w );
	    if (!(_cld wvt$w_print_flags & pf1_m_prt_controller_mode))
	    if ( _cld wvt$w_print_flags & pf1_m_auto_print_mode )
		{
		/* auto print already enabled, disable it */
	        WVT$EXIT_AUTO_PRINT_MODE(ld);
		}
	    else
        	{
		/* auto print disabled, enable it */
	        WVT$ENTER_AUTO_PRINT_MODE(ld);
		}
	    }
        } else if ( keysym == XK_F5
		    && (_cld wvt$f5_key_mode == DECwFactoryDefault ||
	                _cld wvt$f5_key_mode == DECwLocalFunction)
		    && !( event->xkey.state & Mod1Mask ) ) { 
	    {
            wvtp ld = w_to_ld( w );

	    if ( event->xkey.state & ControlMask )
	    {
		/* Control F5 is send an answerback message, unless the Alt 
		   key is down */

		if (_cld wvt$l_vt200_common_flags & vtc1_m_auto_answerback)
		transmit_answerback (ld,_cld wvt$b_answerback);
	    }
	    else if ( event->xkey.state & ShiftMask )
	    {
#if defined (VXT_DECTERM)
/*
 * For now, on vxt, shift-F5 blows away the decterm.  However, perhaps this
 * is wrong and shift-F5 should send a long break on serial decterm.  Dave
 * Faulkner doesn't think vxt yet has a routine to send long break.  Eric
 * Osman 3-Aug-1993
 */
	    int call_data = 0;
		/* Shift F5 is disconnect DECterm session, unless the Alt key
		   is down */

	    XtCallCallbacks ((Widget)w, DECwNexitCallback, &call_data);
#endif
	    } 
	    else
		{	/* F5 is the BREAK key */
		int call_data = 0;
		XtCallCallbacks ((Widget)w, DECwNsendBreakCallback, &call_data);
		}
	    }
	} else if ( keysym == XK_Help ) {
	    w->input.button_since_help_pressed = False;
	} else if ( w->input.hex_enable ) {
	    KeySym hex_keysym;
	    char buf[MAX_KEYCODE_LENGTH];
	    int len;
	    KEY_input( w, event, hex_keysym, buf, &w->input.compose_status, &len );
	    if ( isdigit( buf[0] ) || (( buf[0] >= 'A' ) && ( buf[0] <= 'F' )) ||
				      (( buf[0] >= 'a' ) && ( buf[0] <= 'f' ))) {
		if ( !w->input.hex_count ) {
		    w->input.hex_buf = buf[0];
		    w->input.hex_count = 1;
		} else {
		    DECwTermInputCallbackStruct call_data;
		    unsigned short hex_value =
			hex_convert( w->input.hex_buf ) * 16 +
			hex_convert( buf[0] );
		    call_data.reason = DECwCRInput;
		    call_data.data = (char *)(&hex_value);
		    call_data.count = 1;
		    XtCallCallbacks((Widget)w, DECwNinputCallback, &call_data);
		    w->input.hex_enable = FALSE;
		}
	    } else {
		w->input.hex_enable = FALSE;
		XBell( XtDisplay(w), 0 );
	    }
	} else if ((( event->xkey.state & ControlMask ) &&
		    ( keysym == XK_KP_F1 ) &&
		    ( w->source.wvt$l_ext_flags & vte1_m_dickcat )) ||
		   (( event->xkey.state & ControlMask ) &&
			/* support LK401, control compose for hex input */
		    ((( event->xkey.state & Mod1Mask ) &&
		      ( keysym == XK_space )) || 
		      ( keysym == XK_Multi_key )) &&
		    ( w->source.wvt$l_ext_flags & vte1_m_chinese_common ))) {
	    w->input.hex_count = 0;
	    w->input.hex_buf = NULL;
	    w->input.hex_enable = TRUE;

	} else if (( w->source.wvt$l_ext_flags & vte1_m_hebrew ) &&
		keysym == XK_Alt_L && ( event->xkey.state & ControlMask )) {
	    DECwTermRedisplay7bit( w );
	} else if (( w->source.wvt$l_ext_flags & vte1_m_hebrew ) &&
		keysym == XK_Mode_switch ) {
	    wvtp ld = w_to_ld( w );
	   /* First update the keyboard state bit. */
	    if (_DECwTermIsKBHebrew( w ))
	        set_kb_bit( w, TRUE );
	    else
	        set_kb_bit( w, FALSE );
	   /* Redisplay only if not from a soft switch. */
	    if ( _cld wvt$l_ext_specific_flags & vte2_m_kb_soft_switch )
		_cld wvt$l_ext_specific_flags &= ~vte2_m_kb_soft_switch;
  	    else
		DECwTermRedisplay7bit( w );
        } else {
	    send_key( w, event, keysym );
        }
    } else

    if ( event->type == KeyRelease ) {
	KeySym keysym = XLookupKeysym( (XKeyEvent *)event, 0 );
	if ( keysym == XK_Help ) {

	    if ( ! w->input.button_since_help_pressed ) {
		send_key( w, event, keysym );
	    }
	}
    } else

    if (event->type == MotionNotify && ! (event->xkey.state & HelpMask)) {
	if (w->input.selection_in_progress) {
	    i_do_selection( w, event );
	} else {
	    if (w->input.motion_handler != NULL) {
	        (*w->input.motion_handler)( w, event->xmotion.x,
                                            event->xmotion.y );
	    }
	}
    } else

    if (event->type == ButtonPress) {
	used_button = FALSE;
        w->input.button_since_help_pressed = True;
	if ( event->xbutton.state & HelpMask )
	    used_button = TRUE;	/* ignore button if Help key is down */
	if( ! used_button && event->xbutton.button == 1)
	  i_accept_focus( w, &event->xbutton.time );
	if ( ! used_button && w->input.button_handler != NULL)
	    if ( (*w->input.button_handler)( w, event->xbutton.button,
					       event->xbutton.state,
					       event->xbutton.x,
					       event->xbutton.y,
					       buttonDOWN ) )
	        used_button = TRUE;
	if ( ! used_button ) {
	    if ( event->xbutton.button == 1 ) 
	        if(event->xbutton.state & ShiftMask) {
		    if(Source_has_selection(w, 0)) {
			      i_extend_selection(w, event);
		      }
		} else {
	            i_start_selection( w, event, 0 );
		}
	    if ( event->xbutton.button == 2 )
	      i_start_selection( w, event, 1);
	}
    } else

    if (event->type == ButtonRelease) {
        w->input.button_since_help_pressed = True;
	if ( event->xbutton.state & HelpMask ) { /* context sensitive help */
	    if (event->xbutton.button == 1 ) {
		XmAnyCallbackStruct call_data;

		call_data.reason = XmCR_HELP;
		call_data.event = event;
		XtCallCallbacks( (Widget)w, DECwNhelpCallback, &call_data );
	    }
	    used_button = TRUE;
	} else
	    used_button = FALSE;
	if ( ! used_button && w->input.button_handler != NULL)
	    if ( (*w->input.button_handler)( w, event->xbutton.button,
					       event->xbutton.state,
					       event->xbutton.x,
					       event->xbutton.y,
					       buttonUP ) )
		used_button = TRUE;
	if ( ! used_button ) {
	    if ( event->xbutton.button == 1 || event->xbutton.button == 2 ) 
	        i_end_selection( w, event );
	}
    } 

    if (( w->source.wvt$l_ext_flags & vte1_m_hebrew ) &&
	     ( event->type == KeyRelease )) {
	if ( XLookupKeysym( (XKeyEvent *)event, 0 ) == XK_Mode_switch ) {
	    wvtp ld = w_to_ld( w );
	   /* First update the keyboard state bit. */
	    if ( _DECwTermIsKBHebrew( w ))
	        set_kb_bit( w, TRUE);
	    else
	        set_kb_bit( w, FALSE);
	   /* Redisplay only if not from a soft switch. */
	    if ( _cld wvt$l_ext_specific_flags & vte2_m_kb_soft_switch )
		_cld wvt$l_ext_specific_flags &= ~vte2_m_kb_soft_switch;
  	    else
		DECwTermRedisplay7bit( w );
        }
    }
}

/*
 * This InputFromWidgets routine was added to solve
 * the problem that when user put mouse on scrollbar, text input characters no
 * longer went to the DECterm.  Compare with DECwindows mail, in which if
 * you are typing a message to someone and you move the mouse onto the
 * scrollbar, you can keep typing your message.  This solution was
 * commissioned generously by Micky Balladelli.
 */

void InputFromWidgets( w, e, params, num_params )
Widget  w;
XKeyEvent *e;
char **params;
int num_params;
{
    i_event_handler( XtParent( w ), NULL, e );
}

void _DECwTermMappingHandler(w, tag, event)
    DECtermWidget w;
    caddr_t tag;
    XEvent *event;
{
    if ( w->source.wvt$l_ext_flags & vte1_m_hebrew )
    if (event->type == MappingNotify) {
	if ( event->xmapping.request == MappingKeyboard &&
					event->xmapping.count > 1 ) {
	    if ( _DECwTermIsKBHebrew( w ))
		set_kb_bit( w, TRUE );
	    else
		set_kb_bit( w, FALSE );
	}
    } 
}

void i_enable_motion( w, proc )
    DECtermWidget w;
    void (*proc)();
{
    w->input.motion_handler = proc;
    if ( proc == NULL )
	{
	XtRemoveEventHandler( (Widget)w, PointerMotionMask, TRUE,
			     (XtEventHandler)i_event_handler,
			     (XtPointer)MOVEMENT_HANDLER );
    XtAddEventHandler( (Widget)w,
		       ButtonMotionMask|Button1MotionMask|
		       Button2MotionMask|Button3MotionMask|Button4MotionMask|
		       Button5MotionMask,
		       TRUE, (XtEventHandler)i_event_handler,
		       (XtPointer)MOVEMENT_HANDLER );
	}
    else
	XtAddEventHandler((Widget)w, PointerMotionMask, TRUE,
			  (XtEventHandler)i_event_handler,
			  (XtPointer)MOVEMENT_HANDLER );
}


void i_enable_button( w, proc )
    DECtermWidget w;
    Boolean (*proc)();
{
    w->input.button_handler = proc;
}

void i_enable_input( w, proc )
    DECtermWidget w;
    Boolean (*proc)();
{
    w->input.input_handler = proc;
}

/*
 * i_report_data
 *	This routine sends a solicited report (i.e. a report sent in response
 *	to a request from the application) to the application.
 */
void i_report_data( w, text, len )
    DECtermWidget w;
    char *text;
    int len;
{
    queue_element *qe;

    qe = queue_alloc( len );
    memcpy( qe->data, text, len );
    if (!enqueue_input_data( w, &w->input.report_queue, MAX_REPORT_QUEUE, qe))
	s_stop_output( w, STOP_OUTPUT_REPORT );
}

/*
 * i_send_unsolicited_report
 *	This routine is called to send an unsolicited report (i.e. a report
 *	sent in response to a button press) to the application.  It differs
 *	from i_report_data in two ways: it inserts the report into the
 *	keyboard input queue instead of the report queue, and if the report
 *	can't be sent right away it locks the keyboard instead of stopping
 *	output.  This is important to prevent a deadlock where the application
 *	is blocked trying to send output and DECterm has stopped output
 *	and is waiting for an xon from the pty driver.
 */

void i_send_unsolicited_report( w, text, len )
    DECtermWidget w;
    char *text;
    int len;
{
    queue_element *qe;

    qe = queue_alloc( len );
    memcpy( qe->data, text, len );
    if ( (qe->length = len) > 0 ) {
	if (w->input.wait || w->input.locked) {
	    XBell( XtDisplay(w), 0 );
	    queue_free( qe );
	} else {
            w->input.key_pressed = TRUE;
	    if ( ! enqueue_input_data( w, &w->input.keyboard_queue,
			MAX_KEYBOARD_QUEUE, qe ) ) {
	        w->input.wait = TRUE;
	        i_change_keyboard_control( w );
	    }
	}
    } else {
	queue_free( qe );
    }
}

/*
 * DECwTermStuff()
 *	Stuff a selection item into DECterm's input stream.
 */
void
DECwTermStuff( w, type, value, length )
    DECtermWidget w;
    Atom	type;
    char	*value;
    int		length;
{
    Atom selection = XA_PRIMARY;
    int format = 8;

    stuff_proc( w, (Opaque)type, &selection, &type, value, &length, &format );
}

/*
 * DECwTermStopInput( w )
 *	Stop calling the inputCallback
 */
void
DECwTermStopInput( w )
    DECtermWidget w;
{
    w->input.suspended = TRUE;
}


/*
 * DECwTermStartInput( w )
 *	Resume calling the inputCallback
 */
void
DECwTermStartInput( w )
    DECtermWidget w;
{
    w->input.suspended = FALSE;

    send_more_input( w );
}

/* lookup_nrc_code takes a string and converts it in place according
   to what NRC (National Replacement Character set) is in effect.
   This routine assumes the caller has already decided that NRC mode
   is in effect and that the string contains exactly one byte.
   The source is assumed to be ISO Latin 1.

   The return value is True if the input code could be translated into a
   code in the NRC set.
 */

int lookup_nrc_code( w, string )
    DECtermWidget		w;
    unsigned char		*string;
{
	unsigned char code = 0;

	switch (w->common.keyboardDialect)
	{
	case DECwBritishDialect:	/* U.K. */
	    switch (string[0])
	    {
case 163: code = '#'; break; /* British pound sign */
default: break;
	    }
	    break;

	case DECwFlemishDialect:
	case DECwBelgianFrenchDialect:	/* French */
	    switch (string[0])
	    {
case 163: code = '#'; break; /* British pound sign */
case 167: code = ']'; break; /* section */
case 168: code = '~'; break; /* dieresis */
case 176: code = '['; break; /* degrees */
case 224: code = '@'; break; /* a-grave */
case 231: code = '\\'; break; /* c-cedilla */
case 232: code = '}'; break; /* e-grave */
case 233: code = '{'; break; /* e-acute */
case 249: code = '|'; break; /* u-grave */
case 251: code = '~'; break; /* u-circumflex */
default: break;
	    }
	    break;

	case DECwCanadianFrenchDialect: /* French Canadian */
	    switch (string[0])
	    {
case 224: code = '@'; break; /* a-grave */
case 226: code = '['; break; /* a-circumflex */
case 231: code = '\\'; break; /* c-cedilla */
case 232: code = '}'; break; /* e-grave */
case 233: code = '{'; break; /* e-acute */
case 234: code = ']'; break; /* e-circumflex */
case 238: code = '^'; break; /* i-circumflex */
case 244: code = '`'; break; /* o-circumflex */
case 249: code = '|'; break; /* u-grave */
case 251: code = '~'; break; /* u-circumflex */
default: break;
	    }
	    break;

	case DECwDanishDialect:
	case DECwNorwegianDialect: /* Norwegian/Danish */
	    switch (string[0])
	    {
case 197: code = ']'; break; /* A-circle */
case 198: code = '['; break; /* AE */
case 216: code = '\\'; break; /* O-slash */
case 229: code = '}'; break; /* a-circle */
case 230: code = '{'; break; /* ae */
case 248: code = '|'; break; /* o-slash */
default: break;
	    }
	    break;

	case DECwFinnishDialect:	/* Finnish */
	    switch (string[0])
	    {
case 196: code = '['; break; /* A-dieresis */
case 197: code = ']'; break; /* A-circle */
case 214: code = '\\'; break; /* O-umlaut */
case 220: code = '^'; break; /* U-umlaut */
case 228: code = '{'; break; /* a-umlaut */
case 229: code = '}'; break; /* a-circle */
case 233: code = '`'; break; /* e-acute */
case 246: code = '|'; break; /* o-umlaut */
case 252: code = '~'; break; /* u-umlaut */
default: break;
	    }
	    break;

	case DECwAustrianGermanDialect: /* German */
	    switch (string[0])
	    {
case 126: code = 0; break; /* left apostrophe */
case 167: code = '@'; break; /* section */
case 196: code = '['; break; /* A-umlaut */
case 214: code = '\\'; break; /* O-umlaut */
case 220: code = ']'; break; /* U-umlaut */
case 223: code = '~'; break; /* sharp s */
case 228: code = '{'; break; /* a-umlaut */
case 246: code = '|'; break; /* o-umlaut */
case 252: code = '}'; break; /* u-umlaut */
default: break;
	    }
	    break;

	case DECwItalianDialect: /* Italian */
	    switch (string[0])
	    {
case 163: code = '#'; break; /* British pound sign */
case 167: code = '@'; break; /* section */
case 176: code = '['; break; /* degree sign */
case 224: code = '{'; break; /* a-grave */
case 231: code = '\\'; break; /* c-cedilla */
case 232: code = '}'; break; /* e-grave */
case 233: code = ']'; break; /* e-acute */
case 236: code = '~'; break; /* i-grave */
case 242: code = '|'; break; /* o-grave */
case 249: code = '`'; break; /* u-grave */
default: break;
	    }
	    break;

	case DECwSwissFrenchDialect:
	case DECwSwissGermanDialect: /* Swiss */ 
	    switch (string[0])
	    {
case 224: code = '@'; break; /* a-grave */
case 228: code = '{'; break; /* a-umlaut  */
case 231: code = '\\'; break; /* c-cedilla */
case 232: code = '_'; break; /* e-grave  */
case 233: code = '['; break; /* e-acute  */
case 234: code = ']'; break; /* e-circumflex  */
case 238: code = '^'; break; /* i-circumflex  */
case 244: code = '`'; break; /* o-circumflex  */
case 246: code = '|'; break; /* o-umlaut  */
case 249: code = '#'; break; /* u-grave  */
case 251: code = '~'; break; /* u-circumflex  */
case 252: code = '}'; break; /* u-umlaut  */
default: break;
	    }
	    break;

	case DECwSwedishDialect: /* Swedish */
	    switch (string[0])
	    {
case 196: code = '['; break; /* A-dieresis */
case 197: code = ']'; break; /* A-circle */
case 201: code = '@'; break; /* E-acute */
case 214: code = '\\'; break; /* O-umlaut */
case 220: code = '^'; break; /* U-umlaut */
case 228: code = '{'; break; /* a-umlaut */
case 229: code = '}'; break; /* a-circle */
case 233: code = '`'; break; /* e-acute */
case 246: code = '|'; break; /* o-umlaut */
case 252: code = '~'; break; /* u-umlaut */
default: break;
	    }
	    break;

	case DECwSpanishDialect: /* Spanish */
	    switch (string[0])
	    {
case 161: code = '['; break; /* inverted ! */
case 163: code = '#'; break; /* British pound sign */
case 167: code = '@'; break; /* section */
case 176: code = '|'; break; /* degrees */
case 191: code = ']'; break; /* inverted ? */
case 209: code = '\\'; break; /* N-tilde */
case 231: code = '~'; break; /* c-cedilla */
case 241: code = '}'; break; /* n-tilde */
case 96: code = '{'; break; /* grave accent */
default: break;
	    }
	    break;

	case DECwPortugueseDialect: /* Portuguese */
	    switch (string[0])
	    {
case 195: code = '['; break; /* A-tilde */
case 199: code = '\\'; break; /* C-cedilla */
case 213: code = ']'; break; /* O-tilde */
case 227: code = '{'; break; /* a-tilde */
case 231: code = '|'; break; /* c-cedilla */
case 245: code = '}'; break; /* o-tilde */
default: break;
	    }
	    break;

	default: break;
	}

	if ( code == 0 )
	    return False;

	string[0] = code;
	return True;
}

/* KEY_input taken from yterm's key.c */

static void KEY_input( w, event, keysym, string, compose_status, nbytes )
    DECtermWidget		w;
    char			*string;
    XComposeStatus		*compose_status;
    register XKeyPressedEvent	*event;
    KeySym			keysym;
    int				*nbytes;	/* length of returned string */
{
	char		*cp;
	char            temp[5];
	int             length;
        wvtp            ld = w_to_ld( w );
 
#define ComposeState(a) ((a)->chars_matched)
#define ComposeKeysym(a) ((a)->compose_ptr)
#define DXK_illegal 0x00FF

	cp = NULL;      
           
/*
 * Before looking up the key, change the key if the terminal is set up
 * for various translations having to do with layouts of comma, anglebracket,
 * tilde, delete and f11 keys.
 *
 *	NOTE:	In order to make compose sequences work correctly, the following
 *		code modifies the raw keycode received in the event (since
 *		that's where XLookupString will see it).
 *
 *		The proper solution for this problem is to rip out this code
 *		(and the setup screen entry for it), and move it to the
 *		session manager, and let it be workstation-wide.  eo9/19/88
 *
 *              To make the "Comma key sends ,, or ,<" option work on both
 *              LK201 and LK401 keyboards, we have to check to make sure that
 *              we only modify the keycode if the key has either ",," or ',<"
 *              printed on it, since most international keyboards do not.
 */
	if ( !( event->state & Mod1Mask ) ) /* don't do this if Alt is down */
	  switch (keysym) {

	    case XK_less :

		if (w->common.angleBracketsKey == DECwAngleOpenQuoteTilde)
		    event->keycode = XKeysymToKeycode(event->display, XK_grave);
		break;

	    case XK_greater :
		
		if (w->common.angleBracketsKey == DECwAngleOpenQuoteTilde)
		    event->keycode = XKeysymToKeycode(event->display,
		                                        XK_asciitilde);
		break;

	    case XK_comma :

		if (event->state & ShiftMask)
		{
                    length = XLookupString(event, temp, 5, NULL, NULL);

		    if ((temp[0] == '<') && (length == 1) &&
		        (w->common.periodCommaKeys == DECwCommaPeriodComma))
		    {
		        event->state &= !ShiftMask;
		        break;
		    }

		    if ((temp[0] == ',') && (length == 1) &&
		        (w->common.periodCommaKeys == DECwCommaAngleBrackets))
		    {
		        event->keycode = XKeysymToKeycode(event->display,
		                                          XK_less);
		        event->state &= !ShiftMask;
		        break;
		    }                               
		}
		break;

	    case XK_period :

		if (event->state & ShiftMask)
		{
                    length = XLookupString(event, temp, 5, NULL, NULL);

		    if ((temp[0] == '>') && (length == 1) &&
		        (w->common.periodCommaKeys == DECwCommaPeriodComma))
		    {
		        event->state &= !ShiftMask;
		        break;
		    }

		    if ((temp[0] == '.') && (length == 1) &&
		        (w->common.periodCommaKeys == DECwCommaAngleBrackets))
		    {
		        event->keycode = XKeysymToKeycode(event->display,
		                                          XK_greater);
		        break;
		    }                               
		}
		break;

#if !defined(VXT_DECTERM)
            case XK_F11 :
                if (w->common.f11EscapeKey == DECwF11Escape)
                {
                    event->keycode = XKeysymToKeycode(event->display,
                                                        XK_bracketleft);
                    event->state &= ~ShiftMask;
                    event->state |= ControlMask;
                }
                break;
#endif

	    case XK_quoteleft :
	    case XK_asciitilde :

		if (w->common.openQuoteTildeKey == DECwTildeEscape)
		{
		    event->keycode = XKeysymToKeycode(event->display,
		                                        XK_bracketleft);
		    event->state &= ~ShiftMask;
		    event->state |= ControlMask;
		}
		break;

	    case XK_Delete :

		if ((event->state & ControlMask) == 0)
		if (w->common.backarrowKey == DECwBackarrowBackspace)
		{
		    event->keycode = XKeysymToKeycode(event->display, XK_H);
		    event->state |= ControlMask;
		}
		break;

	    case XK_BackSpace :

		if (w->common.backarrowKey == DECwBackarrowDelete)
                    event->keycode = XKeysymToKeycode(event->display,XK_Delete);
		break;

	    }

	if ( w->common.userPreferenceSet == DECwDEC_Supplemental
	     && w->common.eightBitCharacters
	     && ComposeState(compose_status) == 2 )
	    MCSCompose(event, compose_status);

	if ( ComposeState(compose_status) == 2 
	    && ( w->source.wvt$l_ext_flags & vte1_m_hebrew )
	    && ( w->common.userPreferenceSet == DECwDEC_Hebrew_Supplemental ||
		 w->common.userPreferenceSet == DECwISO_Latin8_Supplemental )) {
	    *nbytes = XLookupString( event, string,
		MAX_KEYCODE_LENGTH, &keysym, compose_status );
	   /*
	    * Fixing the LK401 problem with Ctrl/Hebrew. 
	    * (Only in DECterm, do not apply this for XmText of CSText
	    */
	    if (( ComposeState(compose_status) == 1 ) &&
		( event->state & ControlMask )) 
	        ComposeState(compose_status) = 0;
           /* 
	    * Fixing the XLookupString for Hebrew (two-columns keyboard). 
	    */
	    if ( !*nbytes && 
	        ( keysym != NoSymbol ) && (( keysym >> 8 ) != 0xff )) {
		*nbytes = 1;
	        if ( event->state & ControlMask ) {
	            keysym = XKeycodeToKeysym( event->display,
			event->keycode, 1 );
		    string[0] = (char) toupper(( keysym & 0xff ) - 0x40 );
     	        } else
		    string[0] = ( keysym & 0xff );
	    }
	    if ( keysym > 0xbf ) {
		*nbytes = 0;
		keysym = 0;
		if ( !ComposeState(compose_status) )
		    XBell( XtDisplay( w ), 0 );
	    }
	}
	else 

	/* if it is dot "i", make sure the shift mask is on */
	if ( w->common.terminalType == DECwTurkish &&
	     (w->common.userPreferenceSet == DECwISO_Latin5_Supplemental ||
	      w->common.userPreferenceSet == DECwDEC_Turkish_Supplemental) &&
	     (event->state & LockMask) && (keysym == XK_i) )
		event->state |= ShiftMask;

	*nbytes = XLookupString(event, string,
		MAX_KEYCODE_LENGTH, &keysym, compose_status);

	/* If user chose Greek Latin, then q and Q will just beep */
	if ( w->common.terminalType == DECwGreek &&
	     (w->common.userPreferenceSet == DECwISO_Latin7_Supplemental ||
	      w->common.userPreferenceSet == DECwDEC_Greek_Supplemental) &&
	    *nbytes == 1 && (string[0] == 'Q' || string[0] == 'q') )
	    {
	    *nbytes = 0;
	    XBell( XtDisplay(w), 0 );
	    }

	if (( event->state & ShiftMask ) && 
	    ((( event->state & Mod1Mask ) && ( keysym == XK_space )) ||
	      ( keysym == XK_Multi_key )) &&
	    ( w->source.wvt$l_ext_flags & vte1_m_chinese_common )) {
	    ComposeState( compose_status ) = 1;
	    *nbytes = 0;
	}
	if ( w->source.wvt$l_ext_flags & vte1_m_hebrew ) {

       /*
	* Fixing the LK401 problem with Ctrl/Hebrew. 
	* (Only in DECterm, do not apply this for XmText of CSText
	*/
	if (( ComposeState(compose_status) == 1 ) &&
	    ( event->state & ControlMask ))
	    ComposeState(compose_status) = 0;
       /* 
	* Fixing the XLookupString for Hebrew (two-columns keyboard). 
	*/
	if ( !*nbytes && ( keysym != NoSymbol ) && (( keysym >> 8 ) != 0xff )) {
	    *nbytes = 1;
	    if ( event->state & ControlMask ) {
	        keysym = XKeycodeToKeysym( event->display, event->keycode, 1 );
		string[0] = (char) toupper(( keysym & 0xff ) - 0x40 );
    	    } else
		string[0] = ( keysym & 0xff );
	}
	}

	/*
	 * Re-map the keysym if this is an Alt/key synonym.
	 */
	if ( event->state & Mod1Mask )
	    {
	    /*
	     * Alt key is down.  Some keysyms should be translated regardless
	     * of whether the shift key is down; do these first.
	     */
	    switch ( keysym )
		{
		    case XK_F1:		keysym = XK_F11;	break;
		    case XK_F2:		keysym = XK_F12;	break;
		    case XK_F3:		keysym = XK_F13;	break;
		    case XK_F4:		keysym = XK_F14;	break;
		    case XK_F5:		keysym = XK_Help;	break;
		    case XK_F6:		keysym = XK_Menu;	break; /* Do */
		    case XK_F7:		keysym = XK_F17;	break;
		    case XK_F8:		keysym = XK_F18;	break;
		    case XK_F9:		keysym = XK_F19;	break;
		    case XK_F10:	keysym = XK_F20;	break;
		    case XK_Return:	keysym = XK_KP_Enter;	break;
		}
	    /*
	     * These keysyms are translate differently according to whether
	     * the shift key is down.
	     */
	    if ( event->state & ShiftMask )
		{
		switch ( keysym )
		    {
		    case XK_1:		keysym = XK_KP_1;	break;
		    case XK_2:		keysym = XK_KP_2;	break;
		    case XK_3:		keysym = XK_KP_3;	break;
		    case XK_4:		keysym = XK_KP_4;	break;
		    case XK_5:		keysym = XK_KP_5;	break;
		    case XK_6:		keysym = XK_KP_6;	break;
		    case XK_7:		keysym = XK_KP_7;	break;
		    case XK_8:		keysym = XK_KP_8;	break;
		    case XK_9:		keysym = XK_KP_9;	break;
		    case XK_0:		keysym = XK_KP_0;	break;
		    case XK_minus:	keysym = XK_KP_Subtract;  break;
		    case XK_comma:	keysym = XK_KP_Separator; break;
		    case XK_period:	keysym = XK_KP_Decimal;	break;
		    }
		}
	    else
		{
		switch ( keysym )
		    {
		    case XK_1:		keysym = XK_KP_F1;	break; /* PF1 */
		    case XK_2:		keysym = XK_KP_F2;	break;
		    case XK_3:		keysym = XK_KP_F3;	break;
		    case XK_4:		keysym = XK_KP_F4;	break;
		    case XK_5:		keysym = XK_Find;	break;
		    case XK_6:		keysym = XK_Insert;	break;
		    case XK_7:		keysym = DXK_Remove;	break;
		    case XK_8:		keysym = XK_Select;	break;
		    case XK_9:		keysym = XK_Prior;	break;
						/* Prev Screen */
		    case XK_0:		keysym = XK_Next;	break;
						/* Next Screen */
		    }
		}
	    }
	/*
	 * Finally, allow some other synonyms regardless of whether the Alt
	 * key is down.
	 */

	switch ( keysym )
	    {
	    case XK_F15:	keysym = XK_Help;	break;
	    case XK_F16:
	    case XK_Execute:	keysym = XK_Menu;	break;	/* Do */
	    }

	if ( w->source.wvt$l_ext_flags & vte1_m_tomcat )
	    /* if we got a kana keysym, strip its upper byte */
	    if ((0xff00 & keysym) == 0x0400) {
		string[0] = 0x00ff & keysym;
		*nbytes = 1;
	    }

/*
 * Test for inoperative key combinations: shifted and controlled editing keys
 * and controlled function keys.  Ideally these should not click, but
 * instead we ring the bell.
 */

	if ( event->state & (ShiftMask | ControlMask)
	    && ( keysym == XK_Select || keysym == XK_Prior
	      || keysym == XK_Next   || keysym == XK_Find
	      || keysym == XK_Insert || keysym == DXK_Remove )
	  || event->state & ControlMask
	    && ( XK_F6 <= keysym && keysym <= XK_F20
	      || keysym == XK_Help || keysym == XK_Menu ) )
		{
		XBell( XtDisplay(w), 0 );
		*nbytes = 0;
		return;
		}

	if ((w->common.terminalMode == DECwVT300_7_BitMode) ||
	    (w->common.terminalMode == DECwVT300_8_BitMode)) {
		switch (keysym) {
			case XK_Find : cp = "\2331~";
					break;
			case XK_Insert : cp = "\2332~";
					break;
			case DXK_Remove : cp = "\2333~";
					break;
			case XK_Select : cp = "\2334~";
					break;
			case XK_Prior : cp = "\2335~";
					break;
			case XK_Next : cp = "\2336~";
					break;
			case XK_F1 :
			        if (_cld wvt$f1_key_mode == DECwSendKeySequence)
				    if (event->state & ShiftMask)
					i_send_udk( w, 0);
				    else
					cp = "\23311~";
				break;
			case XK_F2 :
			        if (_cld wvt$f2_key_mode == DECwSendKeySequence)
				    if (event->state & ShiftMask)
					i_send_udk( w, 1);
				    else
					cp = "\23312~";
				break;
			case XK_F3 :
			        if (_cld wvt$f3_key_mode == DECwSendKeySequence)
				    if (event->state & ShiftMask)
					i_send_udk( w, 2);
				    else
					cp = "\23313~";
				break;
			case XK_F4 :
			        if (_cld wvt$f4_key_mode == DECwSendKeySequence)
				    if (event->state & ShiftMask)
					i_send_udk( w, 3);
				    else
					cp = "\23314~";
				break;
			case XK_F5 :
			        if (_cld wvt$f5_key_mode == DECwFactoryDefault ||
			            _cld wvt$f5_key_mode == DECwSendKeySequence)
				    if (event->state & ShiftMask)
					i_send_udk( w, 4);
				    else
					cp = "\23315~";
				break;
			case XK_F6 :
				if (event->state & ShiftMask)
					i_send_udk( w, 5);
				else
					cp = "\23317~";
				break;
			case XK_F7 :
				if (event->state & ShiftMask)
					i_send_udk( w, 6);
				else
					cp = "\23318~";
					break;
			case XK_F8 :
				if (event->state & ShiftMask)
					i_send_udk( w, 7);
				else
					cp = "\23319~";
					break;
			case XK_F9 :
				if (event->state & ShiftMask)
					i_send_udk( w, 8);
				else
					cp = "\23320~";
					break;
			case XK_F10 :
				if (event->state & ShiftMask)
					i_send_udk( w, 9);
				else
					cp = "\23321~";
					break;
			case XK_F11 :
				if (event->state & ShiftMask)
					i_send_udk( w, 10);
				else
					cp = "\23323~";
					break;
#if !defined(VXT_DECTERM)
			case XK_Escape :
                                if (w->common.f11EscapeKey == DECwF11F11)
				if (event->state & ShiftMask)
					i_send_udk( w, 10);
				else
					cp = "\23323~";
					break;
#endif					
			case XK_F12 :
				if (event->state & ShiftMask)
					i_send_udk( w, 11);
				else
					cp = "\23324~";
					break;
			case XK_F13 :
				if (event->state & ShiftMask)
					i_send_udk( w, 12);
				else
					cp = "\23325~";
					break;
			case XK_F14 :
				if (event->state & ShiftMask)
					i_send_udk( w, 13);
				else
					cp = "\23326~";
					break;
			case XK_Help :
				if (event->state & ShiftMask)
					i_send_udk( w, 14);
				else
					cp = "\23328~";
					break;
			case XK_Menu :
				if (event->state & ShiftMask)
					i_send_udk( w, 15);
				else
					cp = "\23329~";
					break;
			case XK_F17 :
				if (event->state & ShiftMask)
					i_send_udk( w, 16);
				else
					cp = "\23331~";		
					break;
			case XK_F18 :
				if (event->state & ShiftMask)
					i_send_udk( w, 17);
				else
					cp = "\23332~";
					break;
			case XK_F19 :
				if (event->state & ShiftMask)
					i_send_udk( w, 18);
				else
					cp = "\23333~";
					break;
			case XK_F20 : 
				if (event->state & ShiftMask)
					i_send_udk( w, 19);
				else
					cp = "\23334~";
					break;
		}
	}
	else {
		switch(keysym) {
			case XK_F11 : cp = "\033";
					break;
			case XK_F12 : cp = "\010";
					break;
			case XK_F13 : cp = "\012";
					break;
			case XK_Find :
			case XK_Insert :
			case DXK_Remove :
			case XK_Select :
			case XK_Prior :
			case XK_Next : cp = "";
				/* the editing keys don't transmit anything
				   in VT100 and VT52 mode */
					break;
		}
	}

	if ( cp == NULL ) {
		switch(keysym) {
			case XK_Left :
			    if (w->common.terminalMode == DECwVT52_Mode)
				cp = "\33D";
			    else
				if (w->common.applicationCursorKeyMode)
				    cp = "\217D";
				else
				    cp = "\233D";
			    break;
			case XK_Right :
			    if (w->common.terminalMode == DECwVT52_Mode)
				cp = "\33C";
			    else
				if (w->common.applicationCursorKeyMode)
				    cp = "\217C";
				else
				    cp = "\233C";
			    break;
			case XK_Up :
			    if (w->common.terminalMode == DECwVT52_Mode)
				cp = "\33A";
			    else
				if (w->common.applicationCursorKeyMode)
				    cp = "\217A";
				else
				    cp = "\233A";
			    break;
			case XK_Down :
			    if (w->common.terminalMode == DECwVT52_Mode)
				cp = "\33B";
			    else
				if (w->common.applicationCursorKeyMode)
				    cp = "\217B";
				else
				    cp = "\233B";
			    break;
			case XK_KP_Enter :
			    if (w->common.applicationKeypadMode)
				if (w->common.terminalMode == DECwVT52_Mode)
				    cp = "\33?M";
				else
				    cp = "\217M";
			    else if ( w->common.newLineMode )
				cp = "\r\n";
			    else
				cp = "\r";
			    break;
			case XK_KP_F1 :
			    if (w->common.terminalMode == DECwVT52_Mode)
				cp = "\33P";
			    else
				cp = "\217P";
			    break;
			case XK_KP_F2 :
			    if (w->common.terminalMode == DECwVT52_Mode)
				cp = "\33Q";
			    else
				cp = "\217Q";
			    break;
			case XK_KP_F3 :
			    if (w->common.terminalMode == DECwVT52_Mode)
				cp = "\33R";
			    else
				cp = "\217R";
			    break;
			case XK_KP_F4 :
			    if (w->common.terminalMode == DECwVT52_Mode)
				cp = "\33S";
			    else
				cp = "\217S";
			    break;
			case XK_KP_Separator :
			    if (w->common.applicationKeypadMode)
				if (w->common.terminalMode == DECwVT52_Mode)
				    cp = "\33?l";
				else
				    cp = "\217l";
			    else
				cp = ",";
			    break;
			case XK_KP_Subtract :
			    if (w->common.applicationKeypadMode)
				if (w->common.terminalMode == DECwVT52_Mode)
				    cp = "\33?m";
				else
				    cp = "\217m";
			    else
				cp = "-";
			    break;
			case XK_KP_Decimal :
			    if (w->common.applicationKeypadMode)
				if (w->common.terminalMode == DECwVT52_Mode)
				    cp = "\33?n";
				else
				    cp = "\217n";
			    else
				cp = ".";
			    break;
			case XK_KP_0 :
			    if (w->common.applicationKeypadMode)
				if (w->common.terminalMode == DECwVT52_Mode)
				    cp = "\33?p";
				else
				    cp = "\217p";
			    else
				cp = "0";
			    break;
			case XK_KP_1 :
			    if (w->common.applicationKeypadMode)
				if (w->common.terminalMode == DECwVT52_Mode)
				    cp = "\33?q";
				else
				    cp = "\217q";
			    else
				cp = "1";
			    break;
			case XK_KP_2 :
			    if (w->common.applicationKeypadMode)
				if (w->common.terminalMode == DECwVT52_Mode)
				    cp = "\33?r";
				else
				    cp = "\217r";
			    else
				cp = "2";
			    break;
			case XK_KP_3 :
			    if (w->common.applicationKeypadMode)
				if (w->common.terminalMode == DECwVT52_Mode)
				    cp = "\33?s";
				else
				    cp = "\217s";
			    else
				cp = "3";
			    break;
			case XK_KP_4 :
			    if (w->common.applicationKeypadMode)
				if (w->common.terminalMode == DECwVT52_Mode)
				    cp = "\33?t";
				else
				    cp = "\217t";
			    else
				cp = "4";
			    break;
			case XK_KP_5 :
			    if (w->common.applicationKeypadMode)
				if (w->common.terminalMode == DECwVT52_Mode)
				    cp = "\33?u";
				else
				    cp = "\217u";
			    else
				cp = "5";
			    break;
			case XK_KP_6 :
			    if (w->common.applicationKeypadMode)
				if (w->common.terminalMode == DECwVT52_Mode)
				    cp = "\33?v";
				else
				    cp = "\217v";
			    else
				cp = "6";
			    break;
			case XK_KP_7 :
			    if (w->common.applicationKeypadMode)
				if (w->common.terminalMode == DECwVT52_Mode)
				    cp = "\33?w";
				else
				    cp = "\217w";
			    else
				cp = "7";
			    break;
			case XK_KP_8 :
			    if (w->common.applicationKeypadMode)
				if (w->common.terminalMode == DECwVT52_Mode)
				    cp = "\33?x";
				else
				    cp = "\217x";
			    else
				cp = "8";
			    break;
			case XK_KP_9 :
			    if (w->common.applicationKeypadMode)
				if (w->common.terminalMode == DECwVT52_Mode)
				    cp = "\33?y";
				else
				    cp = "\217y";
			    else
				cp = "9";
			    break;
			case XK_Tab :
			    cp = "\011";
			    break;
			case XK_Return :
			    if ( w->common.newLineMode )
				cp = "\r\n";
			    else
				cp = "\r";
			    break;
			default :
			    {
			    if (!w->common.eightBitCharacters && *nbytes == 1)
			    {
				lookup_nrc_code (w, string);
				if ( w->source.wvt$l_ext_flags & vte1_m_hebrew )
				if ( w->common.keyboardDialect ==
					DECwHebrewDialect )
				    string[0] &= 0x7f;
			    }
			    }
		}
	}

	if (cp != NULL) {
	    strcpy( string, cp );
	    *nbytes = strlen( cp );
	}

}

i_send_udk(w, key_num)
DECtermWidget w;
int key_num;
{
    if (w->source.wvt$w_udk_length[key_num])
	i_report_data(w,
	    &w->source.wvt$b_udk_area[w->source.wvt$w_udk_data[key_num]],
	    w->source.wvt$w_udk_length[key_num] );
    else
	    XBell( XtDisplay(w), 0 );
}

i_key_pressed(w)
DECtermWidget w;
{
	Boolean ret;

	ret = w->input.key_pressed;
	w->input.key_pressed = FALSE;
	return(ret);
}


MCSCompose(ev, cstatus)
    XKeyEvent *ev;
    XComposeStatus *cstatus;
{
    struct mcstab {
	KeySym ks1;
	KeySym ks2;
	KeySym new1;
	KeySym new2;
    };


    static struct mcstab mcs_conv_table[] = {

/*
 * map the MCS keysyms which are different from Latin1, to ensure correct
 * output from XLookupString
 */

	/* map oe to multiply, divide */

	XK_o,XK_e,	XK_minus,XK_colon,
	XK_O,XK_E,	XK_x,XK_x,

	/* map ydiaeresis to yacute */

	XK_y, XK_quotedbl, XK_y, XK_quoteright,
	XK_quotedbl, XK_y, XK_y, XK_quoteright,
	XK_y, XK_diaeresis, XK_y, XK_quoteright,
	XK_diaeresis, XK_y, XK_y, XK_quoteright,
	XK_Y, XK_quotedbl, XK_Y, XK_quoteright,
	XK_quotedbl, XK_Y, XK_Y, XK_quoteright,
	XK_Y, XK_diaeresis, XK_Y, XK_quoteright,
	XK_diaeresis, XK_Y, XK_Y, XK_quoteright,

	/* map currency to diaeresis */

	XK_o, XK_x, XK_quotedbl, XK_quotedbl,
	XK_O, XK_x, XK_quotedbl, XK_quotedbl,
	XK_0, XK_x, XK_quotedbl, XK_quotedbl,

	XK_x, XK_o, XK_quotedbl, XK_quotedbl,
	XK_x, XK_O, XK_quotedbl, XK_quotedbl,
	XK_x, XK_0, XK_quotedbl, XK_quotedbl,

	XK_o, XK_X, XK_quotedbl, XK_quotedbl,
	XK_O, XK_X, XK_quotedbl, XK_quotedbl,
	XK_0, XK_X, XK_quotedbl, XK_quotedbl,

	XK_X, XK_o, XK_quotedbl, XK_quotedbl,
	XK_X, XK_O, XK_quotedbl, XK_quotedbl,
	XK_X, XK_0, XK_quotedbl, XK_quotedbl,

/*
 * map the latin1 keysyms, which are not legal MCS keysyms
 * to cause a compose error.
 */

	/* map nobreakspace to illegal */

	XK_space, XK_space, DXK_illegal, DXK_illegal,

	/* map brokenbar to illegal */

	XK_bar, XK_bar, DXK_illegal, DXK_illegal,

	/* map diaeresis to illegal */

	XK_diaeresis, XK_space, DXK_illegal, DXK_illegal,
	XK_space, XK_diaeresis, DXK_illegal, DXK_illegal,
	XK_quotedbl, XK_quotedbl, DXK_illegal, DXK_illegal,

	/* map notsign to illegal */

	XK_minus, XK_comma, DXK_illegal, DXK_illegal,

	/* map hyphen to illegal */

	XK_minus, XK_minus, DXK_illegal, DXK_illegal,

	/* map registered to illegal */

	XK_r, XK_O, DXK_illegal, DXK_illegal,
	XK_r, XK_o, DXK_illegal, DXK_illegal,
	XK_r, XK_0, DXK_illegal, DXK_illegal,
	XK_R, XK_O, DXK_illegal, DXK_illegal,
	XK_R, XK_o, DXK_illegal, DXK_illegal,
	XK_R, XK_0, DXK_illegal, DXK_illegal,
	XK_O, XK_r, DXK_illegal, DXK_illegal,
	XK_o, XK_r, DXK_illegal, DXK_illegal,
	XK_0, XK_r, DXK_illegal, DXK_illegal,
	XK_O, XK_R, DXK_illegal, DXK_illegal,
	XK_o, XK_R, DXK_illegal, DXK_illegal,
	XK_0, XK_R, DXK_illegal, DXK_illegal,

	/* map macron to illegal */

	XK_asciicircum, XK_minus, DXK_illegal, DXK_illegal,
	XK_minus, XK_asciicircum, DXK_illegal, DXK_illegal,
	XK_asciicircum, XK_underscore, DXK_illegal, DXK_illegal,
	XK_underscore, XK_asciicircum, DXK_illegal, DXK_illegal,

	/* map acute to illegal */

	XK_acute, XK_space, DXK_illegal, DXK_illegal,
	XK_space, XK_acute, DXK_illegal, DXK_illegal,
	XK_quoteright, XK_quoteright, DXK_illegal, DXK_illegal,

	/* map cedilla to illegal */

	XK_comma, XK_comma, DXK_illegal, DXK_illegal,

	/* map threequarters to illegal */

	XK_3, XK_4, DXK_illegal, DXK_illegal,

	/* map eth to illegal */

	XK_D, XK_minus, DXK_illegal, DXK_illegal,
	XK_minus, XK_D, DXK_illegal, DXK_illegal,
	XK_d, XK_minus, DXK_illegal, DXK_illegal,
	XK_minus, XK_d, DXK_illegal, DXK_illegal,

	/* map multiply to illegal */

	XK_x, XK_x, DXK_illegal, DXK_illegal,
	XK_x, XK_X, DXK_illegal, DXK_illegal,
	XK_X, XK_x, DXK_illegal, DXK_illegal,
	XK_X, XK_X, DXK_illegal, DXK_illegal,

	/* map yacute to illegal */

	XK_y, XK_acute, DXK_illegal, DXK_illegal,
	XK_y, XK_quoteright, DXK_illegal, DXK_illegal,
	XK_acute, XK_y, DXK_illegal, DXK_illegal,
	XK_quoteright, XK_y, DXK_illegal, DXK_illegal,
	XK_Y, XK_acute, DXK_illegal, DXK_illegal,
	XK_Y, XK_quoteright, DXK_illegal, DXK_illegal,
	XK_acute, XK_Y, DXK_illegal, DXK_illegal,
	XK_quoteright, XK_Y, DXK_illegal, DXK_illegal,

	/* map thorn to illegal */

	XK_t, XK_h, DXK_illegal, DXK_illegal,
	XK_T, XK_H, DXK_illegal, DXK_illegal,

	/* map divide to illegal */

	XK_colon, XK_minus, DXK_illegal, DXK_illegal,
	XK_minus, XK_colon, DXK_illegal, DXK_illegal,

	NoSymbol, NoSymbol, NoSymbol, NoSymbol
    };

    struct mcstab *mcs_item;
    char buf[1];
    KeySym keysym;
    int i;

    /* what is the second key in the sequence (key just pressed) */

    XLookupString(ev, buf, 0, &keysym, NULL);

    /* if two-key compose key, convert to Latin1 Keysym */

    if ((keysym >> 8) == 0x1000FE)
	if ((ev->state & (ControlMask | Mod1Mask))==0)
	    if (strncmp(ServerVendor(ev->display), "DECWINDOWS", 10)==0)
		keysym &= 0x00FF;

    /* check to see if the compose sequence needs to be modified for MCS */

    i=0;
    while (mcs_conv_table[i].ks1 != NoSymbol) {
	mcs_item = &mcs_conv_table[i];
	if (mcs_item->ks1 == (KeySym)ComposeKeysym(cstatus) &&
						mcs_item->ks2 == keysym) {

	    /* modify to make Latin1 Compose produce correct result for MCS */

	    ComposeKeysym(cstatus) = (char *)mcs_item->new1;
	    keysym = mcs_item->new2;
	    ev->keycode = XKeysymToKeycode (ev->display, keysym);
	    if (XKeycodeToKeysym(ev->display, ev->keycode, 0) == keysym)
		ev->state &= ~(ShiftMask | LockMask);
	    else
		ev->state |= ShiftMask; 
	    break;
    	}
	i++;
    }
}

i_lock_keyboard( w )
DECtermWidget w;
{
	w->input.locked = True;
	i_change_keyboard_control( w );
}

i_unlock_keyboard( w )
DECtermWidget w;
{
	w->input.locked = False;
	i_change_keyboard_control( w );
}

/* backward compatibility entry points */

void
DwtDECtermStuff( w, type, value, length )
    DECtermWidget w;
    Atom	type;
    char	*value;
    int		length;
{
    DECwTermStuff( w, type, value, length );
}

void
DwtDECtermStopInput( w )
    DECtermWidget w;
{
    DECwTermStopInput( w );
}

void
DwtDECtermStartInput( w )
    DECtermWidget w;
{
    DECwTermStartInput( w );
}



static Widget shell_widget (w)
Widget w;
{
    Widget shell_id;

    for ( shell_id = w; !XtIsShell(shell_id); shell_id = XtParent(shell_id) )
	;

    return (shell_id);
}

#if defined(VMS_DECTERM)
			/* for dynamic image activation, 911030, TN350 */
#undef DXmCreateAIM
#undef DXmAIMGetRIMPList
#undef DXmAIMIsRIMPAvailable
#endif
