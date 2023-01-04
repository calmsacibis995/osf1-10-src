/* #module DT_input.h "X0.0" */
/*
 *  Title:	DT_input.h
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © 1988,1993                                                  |
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
 *  Eric Osman	    13-Oct-1992	VXT V1.2
 *  - Add selection_time cells for copy paste hang bug fi
 *
 *  Aston Chan		17-Dec-1991	V3.1
 *	- I18n code merge
 *
 *  Alfred von Campe    06-Oct-1991     Hercules/1 T0.7
 *	- Added secure keyboard feature from ULTRIX.
 *
 *  Michele Lien    24-Aug-1990 VXT X0.0
 *  - Modify this module to work on VXT platform. Change #ifdef VMS to
 *    #ifdef VMS_DECTERM so that VXT specific code can be compiled under
 *    VMS or ULTRIX development environment.
 *
 * Bob Messenger	27-Feb-1990	V2.1
 *	- Added keyboard_grabbed field to support secure keyboard in UWS V4.0.
 *
 * Bob Messenger	 8-Apr-1989	X2.0-6
 *	- Added button_since_help_pressed to support context sensitive help.
 *
 * Bob Messenger	29-Jan-1989	X1.1-1
 *	- Added input_handler field to support ReGIS one-shot input mode.
 *
 * Mike Leibow		16-Aug-1988	X0.4-44
 *	- Added locked field to InputData structure.
 *
 * Tom Porcher		26-May-1988	X0.4-28
 *	- fixed null array for Ultrix pcc.
 *
 * Tom Porcher		 9-May-1988	X0.4-26
 *	- Added structures for input flow control.
 *
 */

/* input module definitions */

#ifndef _DT_INPUT_H
#define _DT_INPUT_H
             
#include "DXmAIM.h"
#define KEYBOARD_HANDLER	1
#define BUTTON_HANDLER		2
#define MOVEMENT_HANDLER	3
#define KEYBOARD_RELEASE_HANDLER 4

#ifndef NULL_ARRAY
#if defined(VMS_DECTERM) || defined(VXT_DECTERM)
#define NULL_ARRAY
#else
#define NULL_ARRAY 1
#endif
#endif

typedef struct _queue_element {		/* queue_element */
    struct _queue_element *link;
    char *ptr;
    int length;
    char data[NULL_ARRAY];
} queue_element;

typedef struct {			/* queue_header */
    queue_element *head;
    queue_element *tail;
    int count;
} queue_header;

typedef struct {			/* InputData */
	Boolean has_focus;
	Boolean (*button_handler)();
	Boolean (*input_handler)();
	void (*motion_handler)();
	int selection_in_progress;
	Time selection_time;
	Time selection_double_click_time;
	char *selection_word_breaks;
	Boolean selection_confirmed;
	Boolean selection_direction;
	Boolean selection_extend;
	unsigned char selection_type;
	unsigned char last_selection_in_progress;
	DECtermPosition oldend;
	struct  {
		DECtermPosition selection_begin;
		DECtermPosition selection_end;
		int selection_start_x, selection_start_y;
	}select[2];
	XComposeStatus compose_status;
	Boolean	hold;
	Boolean	wait;
	Boolean locked;	/* Keyboard action mode */
	Boolean	suspended;
	queue_header keyboard_queue;
	queue_header report_queue;
	queue_header paste_queue;
	queue_element *current_qe;
	Boolean key_pressed;
	Boolean button_since_help_pressed;
#ifdef SECURE_KEYBOARD
	Boolean keyboard_grabbed;
#endif SECURE_KEYBOARD
	Boolean hex_enable;
	char    hex_buf;
	int     hex_count;
	DXmAIMWidget aim;
	XChar2b string16[1];
	Time primary_selection_time;
	Time secondary_selection_time;
} InputData;
#define Selection_type(w)	(w->input.selection_type)
#define Selection_in_Progress(w) \
  (w->input.selection_in_progress)
#define Selection_double_time(w) \
  (w->input.selection_double_click_time)
#define Selection_begin(w) \
  (w->input.select[Selection_type(w)].selection_begin)
#define Selection_end(w) \
  (w->input.select[Selection_type(w)].selection_end)
#define Selection_x(w) \
  (w->input.select[Selection_type(w)].selection_start_x)
#define Selection_y(w) \
  (w->input.select[Selection_type(w)].selection_start_y)
#define Selection_confirmed(w) \
  (w->input.selection_confirmed)
#define Selection_direction(w) \
  (w->input.selection_direction)
#define Selection_time(w)	(w->input.selection_time)

#define Last_Selection(w)	(w->input.last_selection_in_progress)
#define Extend_save_end(w) 	(w->input.oldend)
#endif
