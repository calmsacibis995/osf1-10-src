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
/************************************************************************
 *									*
 *			Copyright (c) 1990 by				*
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

/************************************************************************
 * Modification History
 * 3-July-91 -- Jim Gettys
 *	       Added HX (sfb) support for bt459
 *
 * 13-Aug-90 -- Sam Hsu
 * 
 *	       Created from fb_data.c, px_data.c, ws_device.c.
 *
 ************************************************************************/
#define _WS_DATA_C_

#include <sys/param.h>
#include <sys/errno.h>
#include <sys/buf.h>
#include <kern/queue.h>
#include <sys/workstation.h>
#include <sys/inputdevice.h>
#include <sys/wsdevice.h>
#include <io/common/devio.h>
#include <io/common/devdriver.h>
#include <io/dec/ws/vsxxx.h>	/* FIXME - move common defs elsewhere ? */
#include <io/dec/tc/slu.h>	/* FIXME - move slu.h to io/dec/ws ? */

#ifdef BINARY

extern ws_devices 	devices[];
extern ws_screens 	screens[];
extern ws_keyboard 	keyboard;
extern ws_pointer	mouse;
extern ws_hardware_type mouse_closure;
extern ws_cursor_data	default_cursor;
extern ws_monitor	monitor_type[];
extern int		nmon_types;
extern unsigned int	cdata[];

extern struct slu	slu;  
extern ws_pointer_report current_rep;
extern u_short		pointer_id;
extern char		*special_keys[];
extern int		special_keys_size;

/* These entry points are used by the keyboard/mouse interface layer */
extern int 	(*v_consputc)();
extern int 	(*v_consgetc)();
extern int 	(*vs_gdopen)();
extern int 	(*vs_gdclose)();
extern int 	(*vs_gdread)();
extern int 	(*vs_gdwrite)();
extern int 	(*vs_gdselect)();
extern int 	(*vs_gdkint)();		/* keyboard interrupt */
extern int 	(*vs_gdpint)();		/* pointer interrupt */
extern int	(*vs_gdmmap)();
extern int 	(*vs_gdioctl)();
extern int 	(*vs_gdstop)();

#else /*BINARY*/

/* The following data declarations are *always* needed */

/*
 * On a workstation, these variables identify
 * the type of graphics display device and
 * which units are present. This info is filled
 * in by the graphics device drivers.
 */
int   ws_display_type = 0;    /* Major device number of display device */
int   ws_display_units = 0;   /* Bit field of units (bit0 = unit 0, etc) */
int   ws_num_controllers = 0;    /* number of graphics options */
int   ws_graphics_config_error = 0; /* flag for gfx config problems */
char   *(*ws_graphics_name_proc)(); /* routine to return graphics option name */
char   *(*ws_keyboard_name_proc)(); /* routine to return keyboard name */
char   *(*ws_pointer_name_proc)(); /* routine to return pointer name */
int	(*ws_graphics_get_width_proc)();
int	(*ws_graphics_get_height_proc)();


/* These entry points are used by the keyboard/mouse interface layer */
int 	(*v_consputc)()  = 0;
int 	(*v_consgetc)()  = 0;
int 	(*vs_gdopen)()   = 0;
int 	(*vs_gdclose)()  = 0;
int 	(*vs_gdread)()   = 0;
int 	(*vs_gdwrite)()  = 0;
int 	(*vs_gdselect)() = 0;
int 	(*vs_gdkint)()   = 0;	/* keyboard interrupt */
int 	(*vs_gdpint)()   = 0;	/* pointer interrupt */
int	(*vs_gdmmap)()   = 0;
int 	(*vs_gdioctl)()  = 0;
int 	(*vs_gdstop)()   = 0;

/* global keyboard and mouse data */
/* the *real* keyboard and mouse stuff is copied into here */
/* during console initialization early in boot-up, */
/* by the hardware-specific console init code; */
/* for example, the lk201 keyboard and vsxxx mouse particulars are */
/* copied during scc_cons_init processing on FLAMINGO */

ws_keyboard keyboard;
ws_hardware_type mouse_closure;
ws_pointer mouse;
struct  slu slu;  
ws_pointer_report current_rep;
u_short pointer_id;

/*
 * for LK201/VSXXX data structures
 *
 * these are referenced in io/dec/tc/scc.c, but won't be declared
 *  unless there is an fb, px, or pv defined.
 */
#include "fb.h"
#include "px.h"
#include "pv.h"

#if NFB == 0 && NPX == 0 && NPV == 0
ws_keyboard		lk201_keyboard;
ws_hardware_type	vsxxx_mouse_closure;
ws_pointer		vsxxx_mouse;
#endif /* NFB == 0 && NPX == 0 && NPV == 0 */

/*
 * for PCXAL/PCXAS data structures
 *
 * these are referenced in io/dec/eisa/gpc.c, but won't be declared
 *  unless there is an vga defined.
 */
#include "vga.h"

#if NVGA == 0
ws_keyboard		pcxal_keyboard;
ws_hardware_type	pcxas_mouse_closure;
ws_pointer		pcxas_mouse;
#endif /* NVGA == 0 */

/*
 * for WS pseudo-device data structures
 */
#include "ws.h"

#if NWS > 0

unsigned int cdata[16] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
};

ws_cursor_data default_cursor = {
    DEFAULT_CURSOR ,
};

ws_devices devices[NUMINPUTDEVICES];
ws_screens screens[NUMSCREENS];

/*
 * This is so an X server can deal with different phosphor types.
 */
ws_monitor monitor_type[] = {
    MONITOR_VRUNK,
    MONITOR_VR299,
    MONITOR_VR290,
    MONITOR_VR260,
    MONITOR_VR262,
    MONITOR_VR150,
    MONITOR_VRE01,
    MONITOR_VR160,
    MONITOR_VR297,
    MONITOR_VRT19,
};
int nmon_types = sizeof(monitor_type) / sizeof(ws_monitor);

/*
 * Generic keyboard special purpose keys. Translations from the
 * specific keyboard tables (currently just PCXAL and LK201) 
 * includes codes for the function keys and other goodies. They are
 * currently flagged by the presence of the 8th bit being set.  The 
 * following table is accessed by removing that bit and using the 
 * result as the index to the following table. Note that table begins
 * a null entry.
 *
 * NOTE: These may be equivalent to VT 220 keysyms for the special
 *	 keys.
 */
char	*special_keys[]={
	0,						/* 0'th */
	"\33[11~",			/* f1 */	/* 1'th */
	"\33[12~",			/* f2 */	/* 2'th */
	"\33[13~",			/* f3 */	/* 3'th */
	"\33[14~",			/* f4 */	/* 4'th */
	"\33[15~",			/* f5 */	/* 5'th */
	"\33[17~",			/* f6 */	/* 6'th */
	"\33[18~",			/* f7 */	/* 7'th */
	"\33[19~",			/* f8 */	/* 8'th */
	"\33[20~",			/* f9 */	/* 9'th */
	"\33[21~",			/* f10 */	/* a'th */
	"\33[26~",			/* f14 */	/* b'th */
	"\33[28~",			/* f15 */	/* c'th */
	"\33[29~",			/* f16 */	/* d'th */
	"\33[31~",			/* f17 */	/* e'th */
	"\33[32~",			/* f18 */	/* f'th */
	"\33[33~",			/* f19 */	/* 10'th */
	"\33[34~",			/* f20 */	/* 11'th */
	"\33[1~",			/* find */	/* 12'th */
	"\33[2~",			/* insert */	/* 13'th */
	"\33[3~",			/* remove */	/* 14'th */
	"\33[4~",			/* select */	/* 15'th */
	"\33[5~",			/* prev */	/* 16'th */
	"\33[6~",			/* next */	/* 17'th */
	"\33OP",			/* pf1 */	/* 18'th */
	"\33OQ",			/* pf2 */	/* 19'th */
	"\33OR",			/* pf3 */	/* 1a'th */
	"\33OS",			/* pf4 */	/* 1b'th */
	"\33[D",			/* left */	/* 1c'th */
	"\33[C",			/* right */	/* 1d'th */
	"\33[B",			/* down */	/* 1e'th */
	"\33[A",			/* up */	/* 1f'th */
	"\33Op",			/* key pad 0 */	/* 20'th */
	"\33On",			/* key pad . */	/* 21'th */
	"\33OM",			/* key pad enter *//* 22'th */
	"\33Oq",			/* key pad 1 */	/* 23'th */
	"\33Or",			/* key pad 2 */	/* 24'th */
	"\33Os",			/* key pad 3 */	/* 25'th */
	"\33Ot",			/* key pad 4 */	/* 26'th */
	"\33Ou",			/* key pad 5 */	/* 27'th */
	"\33Ov",			/* key pad 6 */	/* 28'th */
	"\33O/*",			/* key pad , */	/* 29'th */
	"\33Ow",			/* key pad 7 */	/* 2a'th */
	"\33Ox",			/* key pad 8 */	/* 2b'th */
	"\33Oy",			/* key pad 9 */	/* 2c'th */
	"\33Om",			/* key pad - */	/* 2d'th */
	"",				/* unused    */ /* 2e'th */
	"",				/* unused    */ /* 2f'th */
	"\33[23~",			/* f11	     */ /* 30'th */
	"\33[24~",			/* f12	     */ /* 31'th */
	"\33[H",			/* home	     */ /* 32'th */
	"",				/* middle    */ /* 33'th */
	"",				/* prtsc     */ /* 34'th */
	"",				/* pause     */ /* 35'th */

#ifdef FIXME
/* NOTE: no idea if this stuff actually gets referenced from lk201 code */
	/*
	 * The following strings are to allow a numeric keypad
	 * mode and still use the same translation tables
	 */
	"0",
	".",
	"\r",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	",",
	"7",
	"8",
	"9",
	"-"
#endif /* FIXME */
};

int	special_keys_size = sizeof(special_keys) / sizeof(char *);

#endif /* NWS > 0 */

#endif /* BINARY */
