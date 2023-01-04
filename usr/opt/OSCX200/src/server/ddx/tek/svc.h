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
/***********************************************************
Copyright 1987 by Tektronix, Beaverton, Oregon,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the names of Tektronix or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

TEKTRONIX DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/*
 * This file contains...
 *    .	Function codes for Display Supervisor Vector Calls.
 *
 *    .	getMachineType definitions.
 *
 *    .	4316/7 color SVC structures definitions for the cursor forms passed
 *	from the user.
 *
 *    .	4316/7 color SVC structures definitions for the color maps passed from
 *	the user.
 */
#ifndef LINT
#ifdef RCS_ID
static char *rcsid=  "$Header: /usr/sde/x11/rcs/x11/src/./server/ddx/tek/svc.h,v 1.2 91/12/15 12:42:16 devrcs Exp $";
#endif /* RCS_ID */
#endif /* LINT */
#ifndef SVC_H
#define SVC_H

#ifdef ASSEMBLER
/*
 * The magic number for Display System Calls
 * ( assembly langauge instruction "Trap #13" )
 */
#define	DisplayTrapNumber	13	

/*
 * Function codes for Display Supervisor Vector Calls.
 *
 * 00 - 35 are for Standard display system support.  36 - 39 are reserved.
 */
#define	cursorOn		0
#define	cursorOff		1
#define	cursorLink		2
#define	cursorUnlink		3
#define	cursorPanOn		4
#define	cursorPanOff		5
#define	displayOn		6
#define	displayOff		7
#define	joyPanOn		8
#define	joyPanOff		9	
#define	timeoutOn		10
#define	timeoutOff		11
#define	blackOnWhite		12
#define	whiteOnBlack		13
#define	terminalOn		14
#define	terminalOff		15
#define	getMousePoint		16
#define	setMousePoint		17
#define	getCursorPoint		18
#define	setCursorPoint		19
#define	getButtons		20
#define	setSource		21
#define	setDest			22
#define	updateComplete		23
#define	getCursorForm		24
#define	setCursorForm		25
#define	getViewport		26
#define	setViewport		27
#define	getDisplayState		28
#define	setKeyboardCode	 	29
#define	getMouseBounds	 	30
#define	setMouseBounds	 	31
#define	XYtoRC		 	32
#define	RCtoXY		 	33
#define	setCursorOffset		34
#define	getCursorOffset		35
#define	setCursorSpeed  	36

/*
 * 40 - 49 are for Standard display system "events" support
 */
#define	eventsEnable		40
#define	eventsDisable		41
#define	eventSignalOn		42
#define	eventMouseInterval 	43
#define	getEventCount		44
#define	getNewEventCount 	45 
#define	getNextEvent		46
#define	getMillisecondTime 	47
#define	setAlarmTime		48
#define	clearAlarm		49

/*
 * 50 - 55 are for 4316/4317 color/gray operations.
 */
#define	setCursorSourceAndMask  50
#define	getCursorSourceAndMask  51
#define	setCrosshairCursor 	52
#define	getCrosshairCursor 	53
#define	setCursorMode		54
#define	getCursorMode		55

/*
 * 59 is reserved.
 *
 * putAnEventCode   	59	put a RAW keycode into the event queue.(debug)
 */

/*
 * 63 - 69 are for 4316/7 color/gray operations.
 */
#define	setCursorColor			63
#define	getCursorColor			64
#define	setColorMap			65
#define	getColorMap			66
#define	setGrayMap			67
#define	getGrayMap			68
#define	getColorEntry			69
#define	getGrayEntry			70
#define	lockDisplay			71
#define	unlockDisplay			72
#define	lockColorMap			73
#define	unlockColorMap			74

/*
 * 75 - 77 are get/set machine type
 */
#define	getMachineType			75
#define	setMachineType			76
#define	getRealMachineType		77

/*
 * and the tail end...
 */
#define	lastDisplaySVCcode		90

#else /* ASSEMBLER */

/*
 * _svcCursorXY also declared in /usr/include/box/display.h
 */
#ifndef DISPLAY_H
typedef struct _svcCursorXY {
	short x, y;
} SvcCursorXY, *SvcCursorXYPtr;
#endif  /* DISPLAY_H */

/*
 * The get/set Machine type calls support the 440x (UniFLEX) and 440x+ (UTek)
 * systems. The "Real" machine type cannot be changed. The get/set machine type
 * may be manipulated to target software characteristics to multiple platforms.
 */
typedef struct _svcMachineType {
	long	model:16;		/* can be 0x4404, 0x4405, 0x4406, */
					/* 0x4316, 0x4317
	long	plus:1;			/* True if this is a UTek + series */
	long	cdp:1;			/* True if CDP hardware is present */
	long	color:1;		/* True if color, b&w otherwise */
} SvcMachineType, *SvcMachineTypePtr;
extern SvcMachineType GetSvcMachineType();

typedef struct _svcCursorSpeed {
    	short threshold;
    	short multiplier;
    	short divisor;
} SvcCursorSpeed, *SvcCursorSpeedPtr;

/*
 * Display state record returned by getDisplayState system call
 */
typedef struct _svcDisplayState {
	long	reservedA:15;
	long	eventsOn:1;	/* true if event mechanism is on */
	long	reservedB:4;
	long	discPanOn:1;	/* true if joydisk causes viewport panning */
	long	cursorPanOn:1;	/* true if cursor can cause viewport panning */
	long	mouseLinked:1;	/* true if mouse is linked to cursor */
	long	cursorOn:1;	/* true if graphics cursor is enabled */
	long	reservedC:3;
	long	lockLedOn:1;	/* true if caps lock LED is illuminated */
	long	emulatorOn:1;	/* true if terminal emulator output enabled */
	long	blackOnWhite:1;	/* true if black on white, w on b otherwise */
	long	screenSaver:1;	/* true if screen save is active */
	long	displayOn:1;	/* true if on */
	long	viewport;	/* upper left corner point of viewport */
	long	mouseBound_ul;	/* upper left corner point of mouse bounds */
	long	mouseBound_lr;	/* lower right corner point of mouse bounds */
	short	cursor[ 16 ];	/* cursor image */
	char	keyboardCode;	/* current keyboard encoding 0=events 1=ansi */
	char	reserved1;	/* reserved for future use
	short	lineIncrement;	/* number of bytes from between lines */
	short	width;		/* width of virtual display bitmap */
	short	height;		/* height of virtual display bitmap */
	short	viewPortWidth;	/* width of viewport */
	short	viewPortHeight;	/* height of viewport */
	SvcCursorXY cursorOffset; /* X and Y graphic cursor offset */
	long	reservedD[2];	/* reserved for future use */
} SvcDisplayState, *SvcDisplayStatePtr;
extern SvcDisplayState *GetDisplayState();
	
/*
 * structure for setting RGB cursor values.
 */
typedef struct _svcCursorColor {
	unsigned short	redFore;	/* RED value for cursor foreground */
	unsigned short	greenFore;	/* GREEN value "" "" */
	unsigned short	blueFore;	/* BLUE value  "" "" */
	unsigned short	redBack;	/* RED value for cursor background */
	unsigned short	greenBack;	/* GREEN value "" "" */
	unsigned short	blueBack;	/* BLUE value  "" "" */
} SvcCursorColor, *SvcCursorColorPtr;
extern int SetCursorColor(),
	   GetCursorColor();

/*
 * Cursor size required by SVC
 * Number of longs needed to store cursor data required by SVC (64x64/32)
 */
#define SvcCursorSize 64
#define SvcCursorDataSize 128

/*
 * structure used by the svc for set/get CursorSourceAndMask display
 * system calls expect/return this structure.
 */
typedef struct _svcCursorForm {
    	short xoff, yoff;
	unsigned long *src;
	unsigned long *mask;
} SvcCursorForm, *SvcCursorFormPtr;
extern int SetCursorSourceAndMask(),
	   GetCursorSourceAndMask();

/*
 * color map structure
 */
typedef struct _svcColorDef {
	unsigned short	pixel;
	unsigned short	red;
	unsigned short	green;
	unsigned short	blue;
} SvcColorDef, *SvcColorDefPtr;

#endif /* ASSEMBLER */
#endif /* SVC_H */
