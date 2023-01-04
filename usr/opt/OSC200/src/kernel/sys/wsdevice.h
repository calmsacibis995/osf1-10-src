/*
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 * HISTORY
 */
/*	
 *	@(#)$RCSfile: wsdevice.h,v $ $Revision: 4.2.11.6 $ (DEC) $Date: 1993/11/17 17:28:33 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/************************************************************************
 *									*
 *			Copyright (c)  1989 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/
#ifndef _WSDEVICE_H_
#define _WSDEVICE_H_
/*
 * This file defines the interface that all workstation devices must use
 * to register themselves with the ws pseudo-device.  This should allow
 * new devices to be added without having to modify other drivers,
 * with large code sharing among screens or devices (cursor chips, LUTs)
 * of like types.  New frame buffer devices should be able to run without
 * modification of the X server to support them.
 *
 * The first section of this file documents the functions the driver
 * uses to identify itself to the ws driver.
 * I wish I had function prototypes available....  Sigh...
 */
/*
 * Each function will always be called with a handle (closure), to reduce
 * code required.  While a single closure would work, it would
 * reduce code sharing, as too many details of a single driver would
 * be visible in various device specific routines, like position cursor,
 * which really only needs the coordinates and the address of the cursor
 * chip.
 */

typedef struct {
	caddr_t  (*init_closure)();
	int  (*init_color_map)();
	int  (*load_color_map_entry)();
        void (*clean_color_map)();
	int  (*video_on)();	/* not clear where these should go */
	int  (*video_off)();	/* but existing hardware its in cursor chips */
	caddr_t cmc;
} ws_color_map_functions;

typedef struct {
	caddr_t  (*init_closure)();
	int  (*load_cursor)();
	int  (*recolor_cursor)();
	int  (*set_cursor_position)();
	int  (*cursor_on_off)();
	caddr_t cc;
} ws_cursor_functions;

typedef struct {
	caddr_t  (*init_closure)();
	int  (*init_screen)();
	int  (*clear_screen)();
        int  (*scroll_screen)();
	int  (*blitc)();		/* output one character */
	int  (*map_unmap_screen)();
	int  (*ioctl)();
	void (*close)();
	caddr_t sc;
} ws_screen_functions;

/* structure containing entry points for terminal emulation support */
/* NOTE: all entries are necessary for emulation to be registered */
typedef struct {
	int  (*charput)();		/* put char */
	int  (*charclear)();		/* clear chars */
	int  (*charmvup)();		/* move chars up */
	int  (*charmvdown)();		/* move chars down */
	int  (*charattr)();		/* set char attribute */
} ws_emulation_functions;

/* defines for "charattr" function */
#define ATTR_NORMAL	0
#define ATTR_REVERSE	1

extern int ws_define_screen();

/* 
 * all devices must at least have ws_device entries, plus whatever private
 * information they need.  Note that this makes ordering critical.
 */
typedef struct {
	int  hardware_type;
	int axis_count;			/* only used by extension devices */
	caddr_t dc;
	char *name;
	caddr_t (*init_closure)();
	int  (*ioctl)();
	void (*init_device)();
	void (*reset_device)();
	void (*enable_device)();
	void (*disable_device)();
	void (*set_device_mode)();
	void (*get_device_info)();
} ws_device;	

typedef struct {
	int  hardware_type;
	int axis_count;			/* not used */
	caddr_t kc;
	char *name;
	caddr_t (*init_closure)();
	int  (*ioctl)();
	void (*init_keyboard)();
	void (*reset_keyboard)();
	void (*enable_keyboard)();
	void (*disable_keyboard)();
	int  (*set_keyboard_control)();
	void (*get_keyboard_info)();
	void (*ring_bell)();
	void (*process_keyboard_event)();
	void (*process_keyboard_char)();
	ws_keyboard_control control;
	ws_keyboard_definition *definition;
	ws_keycode_modifiers *modifiers;
	unsigned int *keysyms;
	unsigned char *keycodes;
} ws_keyboard;

typedef struct {
	int  hardware_type;
	int axis_count;			/* not used */
	caddr_t pc;
	char *name;
	caddr_t (*init_closure)();
	int  (*ioctl)();
	int  (*init_pointer)();
	void (*reset_pointer)();
	void (*enable_pointer)();
	void (*disable_pointer)();
	void (*set_pointer_mode)();
	void (*get_pointer_info)();
	void (*get_position_report)();
	int  (*process_tablet_event)();
	int  (*process_mouse_event)();
	void (*set_tablet_overhang)();
	int mswitches;			/* current pointer switches     */
	ws_pointer_control pr;		/* pointer rates		*/
	ws_cursor_position position;	/* current pointer position	*/
        ws_cursor_data cursor;          /* for cursor tracking          */
	ws_pointer_box suppress;	/* suppress motion when inside  */
					/* this box.			*/
	ws_pointer_box constrain;	/* prevent cursor from leaving	*/
	int tablet_scale_x;		/* scale factors for tablet pointer */
	int tablet_scale_y;
	unsigned int tablet_overhang;
        short tablet_max_x;
        short tablet_max_y;
        char tablet_x_axis;             /* 0=left, 1=right */
        char tablet_y_axis;             /* 0=bottom, 1=top */
        char tablet_new_screen;         /* 1=just entered new screen */
} ws_pointer;

/* Pointer report structure definition */
typedef struct {
        char state;                     /* buttons and sign bits        */
        short dx;                       /* delta X since last change    */
        short dy;                       /* delta Y since last change    */
        char bytcnt;                    /* mouse report byte count      */
} ws_pointer_report;
	
/*
 * "state" field definitions for ws_pointer_report
 *	RIGHT_BUTTON	- set if right MB is set
 *	MIDDLE_BUTTON	- set if middle MB is set
 *	LEFT_BUTTON	- set if left MB is set
 *	Y_SIGN		- set if dy is NEGATIVE
 *	X_SIGN		- set if dx is NEGATIVE
 */
#define WSPR_RIGHT_BUTTON	0x01
#define WSPR_MIDDLE_BUTTON	0x02
#define WSPR_LEFT_BUTTON	0x04
#define WSPR_Y_SIGN		0x10
#define WSPR_X_SIGN		0x20

typedef struct {
    int (*(kb_getc))();
    int (*(kb_putc))();
    short timeout;		/* autorepeat timeout */
    short interval;		/* autorepeat interval */
    ws_keyboard *kp;		/* keyboard associated with this device */
    unsigned int status;	/* flags */
    unsigned int keys[8];	/* current state of keys */
    ws_pointer *p;
    ws_event_queue *queue;
    short last;
} ws_keyboard_state;

/* "status" field flag bit definitions */
#define KB_STATUS_SHIFT		(1<<0)
#define KB_STATUS_CNTRL		(1<<1)
#define KB_STATUS_CAPSLOCK	(1<<2)
#define KB_STATUS_HOLD		(1<<3)
#define KB_STATUS_NUMLOCK	(1<<4)
#define KB_STATUS_INRESET	(1<<5)
#define KB_STATUS_UPDNMODE	(1<<6)
#define KB_STATUS_REPEATING	(1<<7)
#define KB_STATUS_PREFIX	(1<<8)
#define KB_STATUS_ALT		(1<<9)
#define KB_STATUS_WAIT		(1<<10)
#define KB_STATUS_COMPOSE	(1<<11)

#define KB_IS_SHIFT(lp)		((lp)->status & KB_STATUS_SHIFT)
#define KB_IS_CNTRL(lp)		((lp)->status & KB_STATUS_CNTRL)
#define KB_IS_CAPSLOCK(lp)	((lp)->status & KB_STATUS_CAPSLOCK)
#define KB_IS_HOLD(lp)		((lp)->status & KB_STATUS_HOLD)
#define KB_IS_NUMLOCK(lp)	((lp)->status & KB_STATUS_NUMLOCK)
#define KB_IS_INRESET(lp) 	((lp)->status & KB_STATUS_INRESET)
#define KB_IS_UPDNMODE(lp)	((lp)->status & KB_STATUS_UPDNMODE)
#define KB_IS_REPEATING(lp)	((lp)->status & KB_STATUS_REPEATING)
#define KB_IS_PREFIX(lp)	((lp)->status & KB_STATUS_PREFIX)
#define KB_IS_ALT(lp)		((lp)->status & KB_STATUS_ALT)
#define KB_IS_WAIT(lp)		((lp)->status & KB_STATUS_WAIT)
#define KB_IS_COMPOSE(lp)	((lp)->status & KB_STATUS_COMPOSE)

#define KB_SET_SHIFT(lp)	((lp)->status |= KB_STATUS_SHIFT)
#define KB_SET_CNTRL(lp)	((lp)->status |= KB_STATUS_CNTRL)
#define KB_SET_CAPSLOCK(lp)	((lp)->status |= KB_STATUS_CAPSLOCK)
#define KB_SET_HOLD(lp)		((lp)->status |= KB_STATUS_HOLD)
#define KB_SET_NUMLOCK(lp)	((lp)->status |= KB_STATUS_NUMLOCK)
#define KB_SET_INRESET(lp) 	((lp)->status |= KB_STATUS_INRESET)
#define KB_SET_UPDNMODE(lp)	((lp)->status |= KB_STATUS_UPDNMODE)
#define KB_SET_REPEATING(lp)	((lp)->status |= KB_STATUS_REPEATING)
#define KB_SET_PREFIX(lp)	((lp)->status |= KB_STATUS_PREFIX)
#define KB_SET_ALT(lp)		((lp)->status |= KB_STATUS_ALT)
#define KB_SET_WAIT(lp)		((lp)->status |= KB_STATUS_WAIT)
#define KB_SET_COMPOSE(lp)	((lp)->status |= KB_STATUS_COMPOSE)

#define KB_CLR_SHIFT(lp)	((lp)->status &= ~KB_STATUS_SHIFT)
#define KB_CLR_CNTRL(lp)	((lp)->status &= ~KB_STATUS_CNTRL)
#define KB_CLR_CAPSLOCK(lp)	((lp)->status &= ~KB_STATUS_CAPSLOCK)
#define KB_CLR_HOLD(lp)		((lp)->status &= ~KB_STATUS_HOLD)
#define KB_CLR_NUMLOCK(lp)	((lp)->status &= ~KB_STATUS_NUMLOCK)
#define KB_CLR_INRESET(lp) 	((lp)->status &= ~KB_STATUS_INRESET)
#define KB_CLR_UPDNMODE(lp)	((lp)->status &= ~KB_STATUS_UPDNMODE)
#define KB_CLR_REPEATING(lp)	((lp)->status &= ~KB_STATUS_REPEATING)
#define KB_CLR_PREFIX(lp)	((lp)->status &= ~KB_STATUS_PREFIX)
#define KB_CLR_ALT(lp)		((lp)->status &= ~KB_STATUS_ALT)
#define KB_CLR_WAIT(lp)		((lp)->status &= ~KB_STATUS_WAIT)
#define KB_CLR_COMPOSE(lp)	((lp)->status &= ~KB_STATUS_COMPOSE)

#define KB_TGL_SHIFT(lp)	((lp)->status ^= KB_STATUS_SHIFT)
#define KB_TGL_CNTRL(lp)	((lp)->status ^= KB_STATUS_CNTRL)
#define KB_TGL_CAPSLOCK(lp)	((lp)->status ^= KB_STATUS_CAPSLOCK)
#define KB_TGL_HOLD(lp)		((lp)->status ^= KB_STATUS_HOLD)
#define KB_TGL_NUMLOCK(lp)	((lp)->status ^= KB_STATUS_NUMLOCK)
#define KB_TGL_INRESET(lp) 	((lp)->status ^= KB_STATUS_INRESET)
#define KB_TGL_UPDNMODE(lp)	((lp)->status ^= KB_STATUS_UPDNMODE)
#define KB_TGL_REPEATING(lp)	((lp)->status ^= KB_STATUS_REPEATING)
#define KB_TGL_PREFIX(lp)	((lp)->status ^= KB_STATUS_PREFIX)
#define KB_TGL_ALT(lp)		((lp)->status ^= KB_STATUS_ALT)
#define KB_TGL_WAIT(lp)		((lp)->status ^= KB_STATUS_WAIT)
#define KB_TGL_COMPOSE(lp)	((lp)->status ^= KB_STATUS_COMPOSE)


typedef struct {
        int device_type;
        union {
          ws_device *dp;
          ws_keyboard *kp;
          ws_pointer *pp;
        } p;
} ws_devices;

/*
 * flags to load_{cursor,colormap} specifying whether to use
 * vblank synchronization.
 */
#define VSYNC   1
#define NOSYNC  0

/* consDev variable and its possible values */
extern int consDev;
#define CONS_DEV      0x01
#define GRAPHIC_DEV   0x02

/* number of input device possible */
#define NUMINPUTDEVICES 4

/* define for default cursor and its data */
#define DEFAULT_CURSOR	0,16,16,0,0,cdata,cdata
extern unsigned int	cdata[];

/* a screen has a screen, a color map, and a cursor */
typedef struct {
        ws_screen_descriptor *sp;
        ws_visual_descriptor *vp;
        ws_depth_descriptor *dp;
        ws_screen_functions *f;
        ws_color_map_functions *cmf;
        ws_cursor_functions *cf;
        ws_screen_box adj_screens;
} ws_screens;

#define TOP_EDGE	0x1
#define BOTTOM_EDGE	0x2
#define LEFT_EDGE	0x4
#define RIGHT_EDGE	0x8

#define TOY ((time.tv_sec * 1000) + (time.tv_usec / 1000))

#ifdef __alpha
#define NUMSCREENS 7
#else /* __alpha */
#define NUMSCREENS 3
#endif /* __alpha */
                                                                                
#ifdef KERNEL
/* FIXME FIXME - this should be private to ws_device. */
/* FIXME FIXME - need to change pcxas and vsxxx support to remove from here. */
typedef struct {
	ws_descriptor ws;
	struct queue_entry rsel;	/* process waiting for select */
	ws_event_queue *queue;		/* where to find shared queue */
	ws_event *events;		/* where the driver puts the events*/
	ws_event_queue *user_queue_address;
	ws_motion_buffer *mb;
	ws_motion_history *motion;
	short open_flag;
	short dev_in_use;
	short mouse_on;
	short keybd_reset;
	short max_event_size;
	short max_axis_count;
	ws_pointer_report last_rep;
	char new_switch, old_switch;
	struct proc *p_server_proc;
	int server_proc_ref_count;
	unsigned int server_proc_screens;
	ws_emulation_functions *emul_funcs;
	int mouse_screen;		/* which screen the HW mouse is on */
	unsigned int cbuf[128], mbuf[128];
	int (*(server_vm_callback[NUMSCREENS]))();
	caddr_t server_proc_data[NUMSCREENS];
} ws_info;
#else /* KERNEL */
/* FIXME FIXME - get XServer to change its use of this... */
#define VSXXX 0
#endif /* KERNEL */

/* ws provided interface interface for mapping to user space */
caddr_t ws_map_region(/* addr, nbytes, how */);
/*	caddr_t addr;
	int nbytes;
	int how;
*/
#endif /*wsdevice.h*/
