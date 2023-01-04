/*
 *  Title:	mx.h
 *
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
 *  
 *  Module Abstract:
 *
 *	include file for DECterm Application modules
 *
 *  Procedures contained in this module:
 *
 *	<list of procedure names and abstracts>
 *
 *  Author:	<original author>
 *
 *  Modification history:
 *
 *  Alfred von Campe    08-Nov-1993     BL-E
 *      - Add F11 key feature from dxterm.
 *
 *  Eric Osman		18-Oct-1993	BL-E
 *	- Add icon name.
 *
 *  Grace Chung         15-Sep-1993	BL-E
 *      - Add 7-bit/8-bit printer support.
 *
 *  Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 * Alfred von Campe     08-Jun-1993     DECterm/BL-C
 *      - Add icon_size to streams structure to keep track of icon size.
 *      - Add flag to pty structure to signal when it's safe to write to pty.
 *
 * Aston Chan		12-Mar-1993	V1.2/BL2
 *	- Add Turkish/Greek support.
 *
 * Alfred von Campe	04-Feb-1993     Ag/BL12 (MUP)
 *      - Add MAX_PRINT_FILE_NAME_LENGTH constant and change the file_name
 *        element of printer structure to an array instead of a pointer.
 *
 *  DAM			10-Nov-1992	VXT V1.2?
 *	- removed customize_lock from tm_param_list. Obsolete.
 *
 *  Eric Osman		 5-Oct-1992	VXT V1.2
 *	- Add setup.concealed_ans
 *
 * Aston Chan		20-Aug-1992	post V1.1
 *	- ToLung's fix of making 2-byte title name in DECW$TERMINAL_DEFAULT.DAT
 *	  work correctly.
 *
 *  Eric Osman		20-Aug-1992	VXT V1.2
 *	- Add print_graphics_delay_id for vxt
 *
 * Eric Osman		18-Dec-1991	V3.1
 *	- Add pty.ptd_data to hold ptd buffer (which we'll keep on free list
 *	  when DECterm is delete)
 *	- Remove "PAGE" and "PAGE_SIZE" since on various platforms, page size
 *	  is different.
 *
 * Aston Chan		17-Dec-1991	V3.1
 *	- I18n code merge
 *
 * Aston Chan		13-Nov-1991	V3.1
 *	- Add HYPERHELP flag because HyperHelp is not ready for OSF/TIN yet.
 *	- Add help_context and wait_cursor to stream structure.
 *
 * Eric Osman		11-Nov-1991	V3.1
 *	- Add symbol NON_STREAM to be -1 for global warning windows.
 *
 * Alfred von Campe	06-Oct-1991     Hercules/1 T0.7
 *      - Changed private #include file names to all lower case.
 *      - Fixed cut & paste problem in vi (from Randall Brown).
 *	- Added F11 key feature from ULTRIX.
 *
 * Aston Chan		1-Sep-1991	Alpha
 *	- Add <> and .h to #include's.  Complained by DECC compiler Release 10.
 *
 *  Alfred von Campe    15-May-1991     V3.0
 *	- Add on_help_parent to setup struct to support the "On Help" menu item.
 *
 *  Randall Brown	 5-Apr-1991	V4.2
 *	- Added changes to support new functionality added to kernel to
 *	  correct problem with cut and paste in vi
 *
 *  Michele Lien    11-Dec-1990	VXT X0.0 BL2
 *    -VXT can only direct printer output to a dedicated printer port, "printer
 *    destination" option in printer customize menu for DECterm V3.0 has been 
 *    taken out.  All the resources associated with printer destination and 
 *    port name can only be conditionally compiled for non-VXT systems.  
 *    Printer destination and printer port name has been hard coded to the 
 *    local printer port.
 *
 *  Bob Messenger	15-Sep-1990	X3.0-7
 *	- Add support for GS fonts.
 *
 *  Michele Lien    24-Aug-1990 VXT X0.0
 *  - Modify this module to work on VXT platform. Change "ifdef VMS" to
 *    "ifdef VMS_DECTERM" so that VXT specific code can be compiled under
 *    VMS or ULTRIX development environment.
 *
 *  Bob Messenger       17-Jul-1990     X3.0-5
 *      Merge in Toshi Tanimoto's changes to support Asian terminals -
 *	- widgets specific to Asian terminals
 *
 * Bob Messenger	30-Jun-19090	X3.0-5
 *	- Add definitions for printer port support: "printer_active",
 *	 "printer_destination" and "printer_status" in STREAM.
 *
 * Mark Woodbury	25-May-1990 X3.0-3M
 *	- Motif changes.  C$include is used to find include file now.  They may
 *    chage to decw$include again later.
 *
 * Bob Messenger	 9-Apr-1990	X3.0-2
 *	- Merge UWS and VMS changes.
 *
 * Mark Granoff		29-Jan-1990	X3.0-1 (VMS DW V3 BL1)
 *	- Added some #define's for new pty I/O buffers
 *	- Commented out MAX_PTY_WRITE and MAX_PTY_READ #define's as they
 *	  will be obsolete. 
 *
 * Bob Messenger	27-Feb-1990	V2.1 (UWS V4.0)
 *	- Support secure keyboard.
 *
 * Bob Messenger	 8-Aug-1989	X2.0-18
 *	- Add setup.dialect_select.
 *
 * Bob Messenger	31-May-1989	X2.0-13
 *	- Remove the definition for the TEST macro.
 *
 * Bob Messenger	 8-Apr-1989	X2.0-6
 *	- Add cs_help_parent to support context sensitive help.
 *
 * Bob Messenger	18-Mar-1989	X2.0-3
 *	- Add definitions for V2 features: terminal driver resize, title,
 *	  icon name, batch scrolling, transcript size, VT330 ID, Customize
 *	  graphics.  Remove shift lock/caps lock.
 *
 * Tom Porcher		16-Aug-1988	X0.4-43
 *	- remove keyclick.
 *	- add control-Q/S = Hold.
 *
 * Tom Porcher		11-Aug-1988	X0.4-43
 *	- change window_name to a String (char *).
 *
 * Mike Leibow           29-Jul-1988
 *      - added window_name to STREAMS so that copyright
 *        message could be added.
 *
 * Tom Porcher		 7-Jun-1988	X0.4-32
 *	- Added app_context to STREAM record.
 *
 * Tom Porcher		 7-Jun-1988	X0.4-31
 *	- Added DECTERM_APPL_NAME and DECTERM_APPL_CLASS macros.
 *
 * Tom Porcher		18-May-1988	X0.4-28
 *	- put BufferMax back.
 *
 * Tom Porcher		18-May-1988	X0.4-27
 *	- added stm->to_pty_hold_b.
 *
 * Tom Porcher		 5-Apr-1988	X0.4-12
 *	- Correct "XAtom.h" to "Xatom.h".
 *	- define globaldef/globalref for Ultrix.
 *
 * Tom Porcher		 5-Apr-1988	X0.4-7
 *	- Added "display" item to STREAM structure.
 *
 * Tom Porcher		10-Mar-1988	X0.4-2
 *	- Added "file" record to STREAM structure.
 *
 * Tom Porcher		25-Jan-1988	X0.3-2
 *	- changed STM.chan to be an "int" rather than LONG.
 *	  That's the way it's used!!
 *
 * If compiled with "LOGGING" defined as "1", some extra code is
 * executed that OPENS the log file but the program doesn't write
 * anything into it.
 *
 * In order to enable actual writing into the log file, the global
 * flag LOGGING_FLAG must be set to 1.  You can do this in the
 * debugger with:
 *
 *	$ run/debug this-program
 *	DBG> deposit LOGGING_FLAG = 1
 *	DBG> go
 */
/* #define LOGGING 1 */
/* #define FULL_LOGGING 1 */
#define THREAD 1
/*
 * #define LOGGING 1	for basic logging info;
 * #define LOGGING 2	for basic info plus some buffer dumps;
 * #define FULL_LOGGING	for muchu logging info (overlaps LOGGING 2, so forces
 *				LOGGING 1).
 * #define THREAD 1	to thread all buffers together and compile in
 *			dumpallbufs().
 *
 * 13-Mar-86 Jerry Leichter
 * 	Only define LOGGING if FULL_LOGGING and no previous def for LOGGING!
 *	Added THREAD support.
 * 14-Apr-86 Eric Osman
 *	If logging undefined by developer, leave it undefined.
 *
 */
/*
 * maybe_fprintf was a conditional fprintf routine
 * that allowed selectable logging at run time.
 * maybe_fprintf was written in Bliss, so is not portable.
 */
#define maybe_fprintf fprintf

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#if defined (VMS_DECTERM) || defined(VXT_DECTERM)
#include "Xm.h"
#include "DECspecific.h"
#include "MrmAppl.h"
#include "Xatom.h"
#include "Vendor.h"
#else
#include <Xm/Xm.h>
#include <DXm/DECspecific.h>
#include <Mrm/MrmAppl.h>
#include <X11/Xatom.h>
#include <X11/Vendor.h>
#endif

#include "decterm.h"

#ifndef VAXC
#define globaldef
#define globalref extern
#endif

#ifdef VMS_DECTERM
#define DECTERM_APPL_NAME "decw$terminal"
#define DECTERM_APPL_CLASS "DECW$TERMINAL"
#define DECTERM_APPL_TITLE "DECterm"
#else
#ifdef VXT_DECTERM
#define DECTERM_APPL_NAME "decterm"
#define DECTERM_APPL_CLASS "DECterm"
#define DECTERM_APPL_TITLE "DECterm"
#else VXT_DECTERM
#define DECTERM_APPL_NAME "dxterm"
#define DECTERM_APPL_CLASS "DXterm"
#define DECTERM_APPL_TITLE "DECterm"
#endif VXT_DECTERM
#endif VMS_DECTERM

#define DECTERM_APPL_SUFFIX "multi"

#ifdef VXT_DECTERM

/* defining DECterm uid file to be used over LAD/LAST */

#define VXT_DECTERM_UID_NAME "DECTERM"
#define VXT_DECTERM_UID_TEXT "DECTERM_TEXT"

#endif VXT_DECTERM

#ifdef VMS_DECTERM
#include "DECtermPort_internal.h"
#else
#define DWT$K_ERROR_BADPARAM 2
#define DWT$K_ERROR_MAX_EMULATORS 3
#define DWT$K_MAX_TERM_NAME_LEN 50
#define DWT$K_CREATE_DECTERM_IDLE		1
#define DWT$K_CREATE_DECTERM_LOGGED_IN		2
#define DWT$K_CREATE_DECTERM_PROMPT		3
#endif VMS_DECTERM

/*HyperHelp - HyperHelp flag only available for VMS at this moment.
 */
#ifdef VMS_DECTERM
#define HYPERHELP 1
#endif /* VMS */

#define MaxISN 30			/* ISN's go from 0 to MaxISN	*/
#define NON_STREAM -1			/* value guaranteed to *not* be a stm */
/* New PTY definitions */
#define CHAR_BUF_SIZE	504		/* Space in I/O buffer for data */
#define IO_BUFFERS	2		/* Number of I/O buffers */

#define PTD$C_SEND_XON		0 /* Pseudo Terminal Driver event */
#define PTD$C_SEND_BELL		1 /* types. When these are in     */
#define PTD$C_SEND_XOFF 	2 /* SYS$LIBRARY:VAXCDEF.TLB they */
#define PTD$C_STOP_OUTPUT	3 /* should be removed from here. */
#define PTD$C_RESUME_OUTPUT	4
#define PTD$C_CHAR_CHANGED 	5
#define PTD$C_ABORT_OUTPUT 	6
#define PTD$C_START_READ 	7
#define PTD$C_MIDDLE_READ 	8
#define PTD$C_END_READ 		9
#define PTD$C_ENABLE_READ 	10
#define PTD$C_DISABLE_READ 	11
#define PTD$C_MAX_EVENTS 	12

#define MAX_PTY_WRITE	100		/* Size of PTY write buffer	*/
#define MAX_PTY_READ	200		/* Size of PTY read buffer	*/

/*
 * Fake ISN's for marking buffers not associated with any stream
 */
#define FREE_ISN	1000		/* On the free list		*/
#define TTY_ISN		1001		/* Being used for TTY I/O	*/
#define MOVING_ISN	1002		/* Passed off the an _data fn	*/
#define ALLOC_ISN	1003		/* Allocated by allocate()	*/

#define XOFF	'\023'
#define XON	'\021'

#define OK(x)		((x)&1)
#define FAILED(x)	((~(x))&1)

/*
 * Used in calling things that want a count and an address.
 */
#define CSTRING(s) (strlen(s)-1),s

/*
 * Set of a critical section (one in which AST deliver is blocked)
 */
#if VMS_DECTERM
#define BEGIN_CS {int CS__; CS__ = SYS$SETAST(0);
#else
#define BEGIN_CS
#endif

#if VMS_DECTERM
#ifndef SS$_WASSET

#include <ssdef>
#endif
#define END_CS if (CS__ == SS$_WASSET) SYS$SETAST(1);}
#else
#define END_CS
#endif

typedef unsigned char	BYTE;
typedef unsigned short	WORD;
typedef unsigned int	LONG;
typedef char *		ADDR;

typedef struct non_ast_block
{	LONG flink;
	LONG blink;
	LONG routine;
	LONG param;
} NON_AST_BLOCK;

typedef struct iosb
{	WORD	status;
	WORD	count;
	LONG	device_data;
} IOSB;

/*
 * TTY characteristics buffer
 */
typedef struct tt_charbuf
{	BYTE	class;
	BYTE	type;
	WORD	width;
	unsigned basic : 24;
	BYTE	length;
	LONG	extended;
} TT_CHARBUF;

typedef short	ISN;			/* Internal stream number	*/
					/* limited to a short because	*/
					/* ast parameter for pty xon/off*/
					/* notification is only 16-bits	*/

#ifdef VMS_DECTERM
typedef struct buffer			/* Data transfer buffer		*/
{	LONG		count;		/* Bytes in use in data[]	*/
	struct buffer 	*link;		/* Next buffer in chain		*/
	IOSB		iosb;		/* Private IOSB			*/
	ISN		isn;		/* Stream buffer is part of	*/
	short		chan;		/* channel (for non PTY's)	*/
	int		unit;		/* unit number */
	int		(*handler)();	/* caller's handler		*/
	int		tag;		/* tag to pass to caller	*/
	int		efn;		/* event flag			*/
	BYTE		data[BufferMax];
					/* Actual data			*/
#ifdef THREAD
	struct buffer	*thread;	/* Buffer thread for dumping	*/
#endif
} BUFFER;
#endif VMS_DECTERM

/* New I/O Buffer for new PTY */
typedef struct {
    short int	status;			/* IOSB used for terminal	*/
    short int	byte_cnt;		/* requests.			*/
    short int	io_status;		/* Status longword used by	*/
    short int	io_byte_cnt;		/* pseudo terminal ctrl req's.  */
    char	data[CHAR_BUF_SIZE];	/* Data buffer			*/
} io_buff;

typedef struct ptd_data
	{
	struct ptd_data *flink;		/* pointer to next block */
	caddr_t addrs[2];		/* two longwords returned by $expreg */
	} ptd_data;

struct pty {
	int		chan;
	XtInputId	read_id;
#if defined(VMS_DECTERM) || defined(VXT_DECTERM)
	io_buff		*read_buffer;
	io_buff		*write_buffer;
#else
        char            read_buffer[MAX_PTY_READ];
        char            write_buffer[4096];
#endif  
	ptd_data	*ptd_data;     /* holds ptd buffer block */
	Boolean		active;
	Boolean         ok_to_write;
	int		read_efn;
	Boolean		write_full;
	int		tslot;
	XtInputId	write_id;
	char		*write_bufstr;
	char		*write_bufptr;
	char		*write_bufend;
	char 		*io_transport;	/* connection file class, such as
					   VxtFileClassSerial, etc */
	char 		*io_path;   	/* connection path, this is used for
					   passing the param to VxtFileOpen */
};

struct setup {
	Widget window_parent;
	Widget window_rows_text;
	Widget window_columns_text;
	Widget window_rows_48_button;
	Widget window_rows_72_button;
	Widget window_columns_132_button;
	Widget window_auto_resize_terminal;
	Widget window_auto_resize_window;
	Widget window_terminal_driver_resize;
	Widget window_big_font_set;
	Widget window_little_font_set;
	Widget window_gs_font_set;
	Widget window_big_font_default;
	Widget window_big_font_other;
	Widget window_big_font_other_name;
	Widget window_little_font_default;
	Widget window_little_font_other;
	Widget window_little_font_other_name;
	Widget window_gs_font_default;
	Widget window_gs_font_other;
	Widget window_gs_font_other_name;
	Widget window_normal_font;
	Widget window_condensed_font;
	Widget window_variable_font;
	Widget window_title;
	Widget window_icon_name;
	Widget window_fine_font_set;
	Widget display_save_lines_off_top;
       	Widget display_scroll_horizontal;
	Widget display_scroll_vertical;
	Widget display_coupling_horizontal;
	Widget display_coupling_vertical;
	Widget display_auto_wrap_enable;
	Widget display_text_cursor_enable;
	Widget display_cursor_blink_enable;
	Widget display_screen_mode_set;
	Widget display_screen_mode_reset;
	Widget display_block_cursor;
	Widget display_underline_cursor;
	Widget display_interpret_controls;
	Widget display_display_controls;
	Widget display_no_status_display;
	Widget display_status_display;
	Widget display_batch_scroll;
	Widget display_transcript_size;
	Widget display_parent;
	Widget display_display_leading_code;
	Widget general_newline;
	Widget general_udk_locked;
	Widget general_features_locked;
	Widget general_normal_cursor_keys;
	Widget general_appl_cursor_keys;
	Widget general_decterm_id;
	Widget general_vt340_id;
	Widget general_vt330_id;
	Widget general_vt320_id;
	Widget general_vt240_id;
	Widget general_vt220_id;
	Widget general_vt125_id;
	Widget general_vt102_id;
	Widget general_vt101_id;
	Widget general_vt100_id;
	Widget general_numeric_keypad;
	Widget general_appl_keypad;
	Widget general_upss_dec;
	Widget general_upss_iso;
	Widget general_upss_dec_heb;
	Widget general_upss_iso_heb;
	Widget general_upss_dec_turkish;
	Widget general_upss_iso_turkish;

	Widget general_upss_dec_greek;
	Widget general_upss_iso_greek;

	Widget general_eight_bit;
	Widget general_seven_bit;
	Widget general_vt300_8bitc_mode;
	Widget general_vt300_7bitc_mode;
	Widget general_vt100_mode;
	Widget general_vt52_mode;
	Widget general_local_echo;
	Widget general_answerback_message;
	Widget general_conceal_answerback;
	Widget general_parent;
	Widget general_vt382d_id;
	Widget general_vt382k_id;
	Widget general_vt382cb_id;
	Widget general_vt382_id;
	Widget general_vt286_id;
	Widget general_vt284_id;
	Widget general_vt282_id;
	Widget general_vt220j_id;
	Widget general_vt102j_id;
	Widget general_vt100j_id;
	Widget general_vt80_id;
	Widget general_jisroman_mode;
	Widget general_ascii_mode;
	Widget general_kanji_mode;
	Widget general_katakana_mode;
	Widget general_regis_screen_mode;
	Widget general_kanji_78;
	Widget general_kanji_83;
	Widget general_ksroman_mode;
	Widget general_ksascii_mode;
	Widget general_ltor;
	Widget general_rtol;
	Widget keyboard_parent;
	Widget keyboard_warning_bell_enable;
	Widget keyboard_margin_bell_enable;
	Widget keyboard_control_QS_hold;
	Widget keyboard_auto_repeat_enable;
	Widget keyboard_backarrow_delete;
	Widget keyboard_backarrow_backspace;
	Widget keyboard_e00_open_quote_tilde;
	Widget keyboard_e00_escape;
	Widget keyboard_b00_angle_brackets;
	Widget keyboard_b00_open_quote_tilde;
	Widget keyboard_b08_comma_comma;
	Widget keyboard_b08_comma_leftangle;
#if !defined(VXT_DECTERM)
	Widget keyboard_g11_escape;
	Widget keyboard_g11_f11;
#endif
	Widget dialect_parent;
	Widget dialect_select;
	Widget graphics_share_colormap_entries;
	Widget graphics_enable_backing_store;
	Widget graphics_macrograph_report;
	Widget graphics_bit_planes_text;
	Widget graphics_parent;
	Widget terminaltype_standard;
	Widget terminaltype_kanji;
	Widget terminaltype_hanzi;
	Widget terminaltype_hangul;
	Widget terminaltype_hanyu;
	Widget terminaltype_hebrew;
	Widget terminaltype_parent;
	Widget printer_queued_printer;
	Widget printer_port;
	Widget printer_file;
	Widget printer_none;
	Widget printer_port_name;
	Widget printer_file_name;
	Widget printer_normal_print;
	Widget printer_auto_print;
	Widget printer_controller;
	Widget printer_full_page;
	Widget printer_full_page_transcript;
	Widget printer_scroll_region;
	Widget printer_selection;
	Widget printer_national;
	Widget printer_national_line_drawing;
	Widget printer_all_characters;
	Widget printer_form_feed;
	Widget printer_to_host;
	Widget printer_graphics;
	Widget printer_background;
	Widget printer_level_1;
	Widget printer_level_2;
	Widget printer_la210;
	Widget printer_compressed;
	Widget printer_expanded;
	Widget printer_rotated;
	Widget printer_monochrome;
	Widget printer_color;
	Widget printer_7_bit;
	Widget printer_8_bit;
	Widget printer_hls;
	Widget printer_rgb;
	Widget printer_parent;

	Widget printer_main_display_24;
	Widget printer_status_display_25;
	Widget print_widget_id;
	Widget queued_printer_options_parent;

	Widget show_version_parent;
	Widget help_about_parent;
	Widget help_overview_parent;
	Widget cs_help_parent;
	Widget on_help_parent;
	Widget convert_display;
	Widget copy_dir;
	Widget copy_dir_ltor;
	Widget copy_dir_rtol;

#ifdef SECURE_KEYBOARD
	Widget commands_secure;
	Widget keyboard_allow_quickcopy;
#endif

	Widget options_widget_id;
	Widget save_options_id;
};

struct file {
	XrmDatabase	default_rdb;
	XrmDatabase	current_rdb;
	char		*current_filename;
	Widget open_fs;
	Widget saveas_fs;
    
};

#define MAX_DEPTH_OF_LINK_LIST	       25    /* determines the threshold in 
						which to stop reading more data
						to be printed */

struct prt_buf_struct {
	struct prt_buf_struct *next;  /* next line in the link list 
						to be printed */
	char *data_buf;			     /* one line of data to be printed*/
	char *start_ptr;		     /* points to the beginning of the
						data to be printed */
	int count;			     /* number to bytes left in the
						buffer to be printed */
};

#define READ_ALLOWED 1	/* 1 = printer port is serial
			   0 = printer port is parallel (write only port). */

/* The constant below defines the max length allowed for a file name when
 * we are printing something.  It has to be the maximum of (see routine
 * start_printing in tea_printer.c):
 *
 *  the length of the string "SYS$SCRATCH:DECW$TERMINAL_PRINT.TMP"
 *  the constant L_tmpnam in stdio.h on non-VMS systems
 *  the width of the port_name XmText field (hardcoded to 40 in UIL)
 *  the width of the file_name XmText field (hardcoded to 100 in UIL)
 */

#define MAX_PRINT_FILE_NAME_LENGTH 100

struct printer {
	Boolean active;
	DECwPrintingDestination destination;
	int status;
	XtIntervalId graphics_delay_id;	/* wait for menu to disappear */
	int file;			/* channel for printing */
	int in_file;			/* channel for printer-to-host mode */
	char file_name[MAX_PRINT_FILE_NAME_LENGTH + 1];
	XtInputId  rd_id;   /* input id for non-blocking reading mode */
	XtInputId  wr_id;   /* input id for non-blocking writing mode */
	int rd_efn;			/* event flag for vms printer-to-host*/
	int wr_efn;			/* event flag for vms printing */
	struct prt_buf_struct *prt_buf; /* printer buffer link list */
	int depth_of_link_list; /* depth of the printer buffer link list, this 
				   is to be used to check the threshold in
				   which to stop reading more data to be
				   printed */
	Boolean stop_reading_input; /* used in non-blocking printing to stop
					reading input */
};

/* callbacks */
/* not portable to pcc */
#define SET_CALLBACK( m, p, t)	   				\
	XtCallbackRec m[2] = { {p,t}, {NULL, NULL} }

/* declare */
#define DECLARE_CALLBACK( m, p, t)					\
	XtCallbackRec m[2]

/* initialize explicitly */
/* note "t" is type cast as (int) to work around an inconsistency in
   DECtoolkit.  XtCallbackRec.tag should be caddr_t.  This is a bug 

   The DwtCallbackRec data structure is not available for Motif.  They do not
   have a replacement, they revert back to the Intrinsic XtCallbackRec.
*/
#define INIT_CALLBACK( m, p, t)	                    \
	m[0].callback = (XtCallbackProc)p;          \
	m[0].closure = (XtPointer)t;                \
	m[1].callback = (XtCallbackProc)NULL



typedef struct stream			/* A data stream		*/
{
	ISN isn;			/* Stream number		*/
	int pid;			/* Subprocess			*/
	int icon_size;                  /* Current icon size            */
	XtAppContext app_context;	/* Xt application context	*/
	Display *display;		/* display			*/
	Widget menubar;			/* menu bar widget		*/
	Widget terminal;		/* decterm widget		*/
	Widget widget;			/* Main Window Widget		*/
	Widget parent;			/* Parent widget		*/
	WORD	flags;			/* STM_ flag bits		*/
	String	window_name;		/* title bar name	        */
	String	icon_name;		/* name that appears on icon	*/
	Atom	window_name_encoding;	/* title bar name encoding	*/
	char	terminal_name[DWT$K_MAX_TERM_NAME_LEN];
					/* Name of terminal		*/
	LONG	job_final_status;	/* final status of user process	*/
	struct pty pty;
	struct setup setup;
	struct file file;
	struct printer printer;
	Opaque help_context;		/* for HyperHelp		*/
	DECwTerminalType terminalType;
	short customize_lock;
			    /* 0 = no locking */
			    /* 1 = customize lock, customize menu unchangeable*/
			    /* 2 = customize lock, customize menu unsaveable */
	/* need to store default title bar name and icon name because system
	   defaults do not contain default title bar name and icon name.
	   Need to keep track of them separately in order to be able restore
	   system options to the correct title name and icon name. */

	String default_db_title;	/* contains system default database
					   title name, "VXT DECterm". */
	String default_title;	/* contains DECterm default title for each
				   window.  It is the node name + transport +
				"VXT DECterm" + the decterm window number (for
				 example: "raynal Telnet VXT DECterm 1") */
	String concealed_ans;		/* hidden answerback message */
} STREAM;
#define STM_KILL_PENDING 1		/* Clearing of IN_USE pending	*/

/*
 * Routines provided by the user interface module
 *
 * pr_data(buf,isn)	Transmits data from PTY[isn] to user interface module
 * tr_data(buf)		Transmits data from TTY line to user interface module
 * time_out()		Called every 10 seconds; handles lost reports
 * pt_xoff(isn)		Prevents user interface from calling pt_data( ,isn)
 * pt_xon(isn)		Un-do pt_xoff(isn)
 */
extern void pr_data(), tr_data(), time_out(), pt_xoff(), pt_xon();

/*
 * Routines defined here
 *
 * pt_data(buf,isn)	Transmit data to PTY[isn]
 * tt_data(buf)		Transmit data to TTY line
 * pr_xoff(isn)		Prevent PTY[isn] from calling pr_data( ,isn)
 * pr_xon(isn)		Un-do pr_xoff(isn)
 * create(isn)		Open a new PTY[isn]
 * destroy(isn)		Close PTY[isn]
 * buf = allocate()	Allocate a new buffer
 * discard(buf)		De-allocate a buffer
 */

extern void destroy();
extern STREAM *GetSTM();

#ifdef VMS_DECTERM
extern void discard();
extern struct dsc$descriptor_s	*descr();
extern void DiscAST();
#endif

#ifdef LOGGING
extern FILE *logfile;
#endif
