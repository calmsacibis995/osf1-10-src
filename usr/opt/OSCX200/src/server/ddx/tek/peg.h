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
/* $Header: /usr/sde/x11/rcs/x11/src/./server/ddx/tek/peg.h,v 1.2 91/12/15 12:42:16 devrcs Exp $ */

#ifndef PEG_H
#define PEG_H

#include <sys/types.h>

#ifdef	UTEK
#include <box/display.h>
#include <machine/hdwr_config.h>
#include "svc.h"
#endif	/* UTEK */

#ifdef	UTEKV
#include "redwing/keyboard.h"
#include "redwing/display.h"
#endif	/* UTEK */

#include "colormapst.h"
#include <assert.h>
#include <stdio.h>

#ifdef NOTDEF
#include "color.h"
#include "pfb.h"
#endif
#include "debug.h"

#define MOTION_BUFFER_OFF

/*
 * Pathname for the 8 bells used in pegInitBells().
 */
#define	BELLNAME	"/usr/lib/X11/bells/bell%d.dev"

/*
 * Pathnames for configuration variables.
 */
#define XSYSCONTROLS	"/usr/lib/X/Xcontrols"
#define XUSRCONTROLS	"/.Xcontrols"

/*
 * Constants used for configuring the memory for X11.
 */
#define	ONE_MB		0x100000
#define	FOUR_MB		0x400000
#define	STACK_SIZE	(2*ONE_MB)


/*
 * Minimum value for a keycode according to the spec (1-7 are reserved
 * for other things).
 */
#define MINKEYCODE	8

/*
 * A buffer size for internal buffers.
 */
#define	BUFFERSIZE	1024

#ifdef	UTEK
/*
 * short defines for accessing fields within the UTEK soft_config struct
 */
#define CPU_BOARD(softp)	(softp)->sc_hdwr.cpu_u.hc_cpu.cpu_board
#define DISPLAY_TYPE(softp)	(softp)->sc_hdwr.screen_u.hc_screen.dtype
#define SCREEN_X(softp)		(softp)->sc_hdwr.screen_u.hc_screen.screen_x
#define SCREEN_Y(softp)		(softp)->sc_hdwr.screen_u.hc_screen.screen_y
#define MM_SCREEN_X(softp)	(softp)->sc_hdwr.screen_u.hc_screen.mm_screen_x
#define MM_SCREEN_Y(softp)	(softp)->sc_hdwr.screen_u.hc_screen.mm_screen_y
#define BITMAP_X(softp)		(softp)->sc_hdwr.screen_u.hc_screen.bitmap_x
#define BITMAP_Y(softp)		(softp)->sc_hdwr.screen_u.hc_screen.bitmap_y
#define N_PLANE(softp)		(softp)->sc_hdwr.screen_u.hc_screen.fb_nplane
#define KEYIDLANG(softp) \
	((softp)->sc_info.sc_regs.keyboardlangidstyle & 0xff00)
#define KEYIDSTYLE(softp) \
	((softp)->sc_info.sc_regs.keyboardlangidstyle & 0xff)

#endif	/* UTEK */

#ifdef	UTEKV
/*
 * short defines for accessing fields within the Redwing - Xdriver shared memory
 */
#define DISPLAY_TYPE(dsp)	(dsp)->ds_displayModel
#define SCREEN_X(dsp)		(dsp)->ds_fbScreen_x
#define SCREEN_Y(dsp)		(dsp)->ds_fbScreen_y
#define BITMAP_X(dsp)		(dsp)->ds_fbBitmap_x
#define BITMAP_Y(dsp)		(dsp)->ds_fbBitmap_y
#define N_PLANE(dsp)		(dsp)->ds_fbDepth
#define KEYIDLANG(dsp)		((dsp)->keyboardlangidstyle & 0xff00)
#define KEYIDSTYLE(dsp)		((dsp)->keyboardlangidstyle & 0xff)
#endif	/* UTEKV */

#define	N_BELLS		8	/* XXX - this should accomodate X11 better */
#define	N_TIMERS	4
#define	KEYREPEAT	0
#define	JOYNSREPEAT	1	/* north-south repeat */
#define	JOYEWREPEAT	2	/* east-west repeat */
#define	PANREPEAT	3

#define	True		1
#define	False		0

#define	JoyKeyToMask(key)	(1<<((key)-KBJoyRight))
#define	JOYRIGHT_BIT	JoyKeyToMask(KBJoyRight)	/* 0x1 */
#define	JOYUP_BIT	JoyKeyToMask(KBJoyUp)		/* 0x2 */
#define	JOYLEFT_BIT	JoyKeyToMask(KBJoyLeft)		/* 0x4 */
#define	JOYDOWN_BIT	JoyKeyToMask(KBJoyDown)		/* 0x8 */
#define	JOYEWMASK	(JOYRIGHT_BIT|JOYLEFT_BIT)
#define	JOYNSMASK	(JOYUP_BIT|JOYDOWN_BIT)
#define	JoyEWDepressed	(pegInfo.kv.joyState & JOYEWMASK)
#define	JoyNSDepressed	(pegInfo.kv.joyState & JOYNSMASK)
#define	JoyDepressed	(pegInfo.kv.joyState & (JOYEWMASK | JOYNSMASK))

#ifdef XTESTEXT1
/*
 * The Input synthesis extension uses these to disallow AutoRepeating while
 * ProcessInputEvents is processing synthetic keypress events
 */
extern int AllowAutoRepeat;
#define	DISALLOW_AUTOREPEAT	(AllowAutoRepeat = 0)
#define	ALLOW_AUTOREPEAT	(AllowAutoRepeat = 1)
#define	AUTOREPEAT_ALLOWED	(AllowAutoRepeat)
#else
#define AUTOREPEAT_ALLOWED	(TRUE)
#endif

/*
 * KeyDepressed() determines if a mapped keycode is depressed;
 * rawKeyDepressed() determines if a keycode directly out of box/keyboard.h
 * is depressed.
 */
#define	_checkmask(array, code)	((array)[ (code) >> 3 ] & (1 << ((code) & 7)))
#define	_setmask(array, code)	((array)[ (code) >> 3 ] |= (1 << ((code) & 7)))

#define	RawKeyDepressed(k)						\
	    _checkmask(((DeviceIntPtr)pegInfo.pKeyboard)->key->down, k+MINKEYCODE)
#define	KeyDepressed(k)						\
	    _checkmask(((DeviceIntPtr)pegInfo.pKeyboard)->key->down, k)
#define	KeyRepeatable(k)						\
	    _checkmask(							\
		((DeviceIntPtr)pegInfo.pKeyboard)->kbdfeed->ctrl.autoRepeats, \
		k)
#define	MakeRawKeyRepeatable(k)					\
	    _setmask(							\
		((DeviceIntPtr)pegInfo.pKeyboard)->kbdfeed->ctrl.autoRepeats, \
		     k+MINKEYCODE)
#define	KeyIsModifier(k)						\
	    ((DeviceIntPtr)pegInfo.pKeyboard)->key->modifierMap[k]
#define	AutoRepeatOn()							\
	    ((((DeviceIntPtr)pegInfo.pKeyboard)->kbdfeed->ctrl.autoRepeat != 0) \
	    && AUTOREPEAT_ALLOWED)

/*
 * At most, there are three things we want timers for: panning, joy disk
 * key repeats, and normal key repeats.  So, we have a list of three timers.
 * For setting the timer, we simply look for the next timer to happen and
 * put that in the display state.  When we get the timeout event, we simply
 * service any timer that appears to have expired.
 */ 
typedef struct _timer {
	u_long	when;
	short	key;
	short	delay;
} Timer;

typedef struct _bells {
	int	fd;
        char    *str[ N_BELLS ];
        int     len[ N_BELLS ];
} Bells;

typedef struct _keyvars {
	Timer		timer[ N_TIMERS ];
	Bool		panEnabled;		/* true if panning is on */
	Bool		panStop;		/* true if panning continues */
						/* after release of the key */
	Bool		readyToUnlock;		/* used to implement toggle */
	long		panInertia;		/* behaviour after panning */
						/* key is released */
	long		nPanDelays;
	long		nPanDeltaX;		/* used only during init */
	long		nPanDeltaY;		/* used only during init */
	long		*panDelays;
	long		*panDeltaX;
	long		*panDeltaY;
	long		nKeyDelays;
	long		*keyDelays;
	long		joyState;		/* state of the joy disk */
	short		keyStack[ MAP_LENGTH ];	/* stack of repeating keys */
	unsigned short	keyTOS;			/* Top of stack for keyStack */
} KeyVars;

/* this is a guess at the register layout.  No documentation could
 * be found...
 */

typedef struct _cdp_regs {
    unsigned long   write0Reg,
		    write1Reg,
		    planeEnableReg,
		    filterReg,
		    maskRegSet;
    unsigned long   pad[5]; /* defined length is 10 */
} COLOR_CNTL;

/* hack... */
typedef long	MotionQueue;

typedef struct _init {
    	ScreenPtr	pScr;		/* ...if not otherwise available */
	caddr_t		pixelFb;	/* packed-pixel frame buffer address */
	caddr_t		oneColorFb;	/* one-color frame buffer address */
	caddr_t		twoColorFb;	/* two-color frame buffer address */
	caddr_t		videoCtl;	/* address of video control register */
	COLOR_CNTL	*cdpCtl;	/* address of cdp control register */
	SvcColorDefPtr	colorDefDefault;
	DevicePtr	pKeyboard;
	DevicePtr	pPointer;
	Bool		screenIsSaved;	/* true if screen saver is active */
	Bool		softwareCursor;	/* true if cursor is not in hardware */
	int		width;		/* width of frame buffer in pixels */
	int		height;		/* height of frame buffer in pixels */
	int		depth;		/* screen depth (an array in future?) */
	int		entries;	/* # of colormap entries (2**depth) */
	int		scrWidth;	/* width of visible screen */
	int		scrHeight;	/* height of visible screen */
	int		mmScreenX;	/* Screen width in mm */
	int		mmScreenY;	/* Screen height in mm */
	int		consolePid;	/* process id to send console output */
	int		eventFd;	/* file desc. for select on events */
	int		lastEventTime;
	int		qLimit;		/* maximum queable events */
	int		nSupportedDepths;/* count of supportedDepths */
	int		nVisuals;	/* count of visuals */
	VisualPtr	visuals;	/* supported visuals */
	DepthPtr	supportedDepths;/* supported depths */
	EventQueue	*queue;		/* pointer to shared mem queue */
	MotionQueue     *motionQueue;   /* pointer to motion history queue */
	BoxRec		constraintBox;	/* box where cursor is constrained */
#ifdef	UTEK
	struct soft_config *softp;	/* pointer to software config block */
#endif	/* UTEK */
	XDisplayState	*dsp;		/* shared memory w/ X kernel driver */
	KeyVars		kv;
	Bells		bells;
        int             fAvailableCDP;	/* True iff CDP can be used */
#ifdef	UTEK
	unsigned short	*videostate;	/* pointer to kernel - videostate
					 * shadow var. in soft_config (UTek).
					 */
#endif	/* UTEK */
#ifdef	UTEKV
	unsigned char	*videostate;	/* pointer to kernel - videostate
					 * shadow var. in xdisplaystate (UTEKV).
					 */
#endif	/* UTEKV */

} InitInfo;

extern InitInfo	pegInfo;

/*
 * Test Consortium Input Extension variables
 */
#ifdef XTESTEXT1
extern SvcCursorXY LastMousePosition;	/* Previous Cursor position */
/*
 * defined in xtestext1di.c
 */
extern KeyCode xtest_command_key;
extern int      on_steal_input;
extern Bool     XTestStealKeyData();
#endif /* XTESTEXT1 */

/*
 * Library declarations.
 */
extern char	*index();
extern char	*strpbrk();
extern char	*strtok();
extern char	*getenv();

/*
 * global pegasus routines.
 */
extern Bool peg1ScreenInit();
extern Bool pegSaveScreen();
extern Bool pegRealizeCursor();
extern Bool pegUnrealizeCursor();
extern void pegRecolorCursor();
extern Bool pegDisplayCursor();
extern Bool peg1DisplayCursor();
extern Bool pegSetCursorPosition();
extern void pegCursorLimits();
extern void pegPointerNonInterestBox();
extern void pegConstrainCursor();
extern void pegQueryBestSize();
extern void pegInitKeys();
extern void InitKeybdState();
extern int pegMouseProc();
extern int pegKeybdProc();

/*
 * Routines for software cursor protection.
 */
extern Bool pegSoftCreateWindow();
extern Bool pegSoftCreateGC();
extern Bool pegSoftChangeWindowAttr();
extern void pegSoftGetImage();
extern unsigned int *pegSoftGetSpans();

/*
 * pfb routines needed here.
 */
extern Bool pfbScreenInit();
extern Bool pfbCloseScreen();
extern void pfb1ResolveColor();
extern void pfb1CreateColormap();
extern void pfbInstallColormap();
extern void pfbUninstallColormap();
extern void pfbListInstalledColormaps();
extern void pfbStoreColors();

/*
 * gfb routines needed here.
 */
extern void gfbXTLKBMappings();
extern int gfbInitMotionQueue();
extern int gfbGetMotionEvents();
extern int gfbSaveMotionEvent();
extern int gfbSendMotionEvent();


/*
 * external mi routines needed here... not many.
 */
void miRecolorCursor();

#endif /* PEG_H */
