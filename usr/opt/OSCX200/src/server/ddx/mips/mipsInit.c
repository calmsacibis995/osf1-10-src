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
 * $XConsortium: mipsInit.c,v 1.5 91/07/18 22:58:20 keith Exp $
 *
 * Copyright 1991 MIPS Computer Systems, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of MIPS not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  MIPS makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * MIPS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL MIPS
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#ident	"$Header: /usr/sde/x11/rcs/x11/src/./server/ddx/mips/mipsInit.c,v 1.2 91/12/15 12:42:16 devrcs Exp $"

#include <sys/types.h>
#include <sysv/sys/cpu_board.h>
#include <sysv/sys/lock.h>
#ifndef NOSIGNALS
#include <sysv/sys/signal.h>
#endif NOSIGNALS
#undef FID

#include "X.h"
#include "Xproto.h"
#include "keysym.h"
#include "input.h"
#include "cursor.h"
#include "colormapst.h"
#include "scrnintstr.h"
#include "servermd.h"
#include "mipointer.h"

#include "mips.h"
#include "mipsFb.h"
#include "mipsIo.h"
#include "mipsKbd.h"
#include "mipsMouse.h"

int mipsSysType;

extern int	mipsMouseProc(), mipsKeybdProc();

extern int      defaultColorVisualClass;
extern int monitorResolution;
int		DDXnoreset = 0;
int		DDXxled1 = 0;
int		DDXxled2 = 0;
int		DDXxled3 = 0;
static int mipsScreenTypeWanted;
int mipsMonitorSize;
static int mipsNoFBHW;
static int mipsNoFBCache;
static char *blackPixelName = "black";
static char *whitePixelName = "white";

#if NETWORK_KEYBD
extern char	*netKeybdAddr;
#endif /* NETWORK_KEYBD */
#if EMULATE_COLOR || EMULATE_MONO
int		DDXemulate = 0;
#endif /* EMULATE_COLOR || EMULATE_MONO */
#if PIXIE
int		pixie = 0;
#endif /* PIXIE */

/* Globals for io */

DevicePtr	pPointer;
DevicePtr	pKeyboard;
volatile extern int	mipsIOReady;

MipsScreenRec mipsScreen[MAXSCREENS];

extern Bool mipsMap2030();
extern Bool mipsMap3230c();
extern Bool mipsMap3230m();

static Bool (*mipsMapProc[][3])() = {
	0,
	mipsMap2030,
	0,		/* 2030 mono */
	0,
	mipsMap3230c,
	mipsMap3230m,
};

/*
 * We don't have real frame buffer devices, so as a hack
 * we always use unit 0 for the color screen and unit 1 for
 * the mono screen
 */
#define COLOR_UNIT	0
#define MONO_UNIT	1

/* supported pixmap formats */
static PixmapFormatRec formats[] = {
    {8, 8, BITMAP_SCANLINE_PAD},
    {1, 1, BITMAP_SCANLINE_PAD},
};
#define NUMFORMATS	sizeof(formats)/sizeof(formats[0])

extern void mipsInstallColormap();
extern void mipsUninstallColormap();
extern int mipsListInstalledColormaps();
extern void mipsStoreColors();
Bool mipsScreenInit();
static Bool mipsSaveScreen();
static Bool mipsCloseScreen();

/* Open output to the screen framebuffer */
InitOutput(screenInfo, argc, argv)
	ScreenInfo *screenInfo;
	int argc;
	char **argv;
{
	int i;

	systemType();

	screenInfo->imageByteOrder = IMAGE_BYTE_ORDER;
	screenInfo->bitmapScanlineUnit = BITMAP_SCANLINE_UNIT;
	screenInfo->bitmapScanlinePad = BITMAP_SCANLINE_PAD;
	screenInfo->bitmapBitOrder = BITMAP_BIT_ORDER;

	screenInfo->numPixmapFormats = 1;
	screenInfo->formats[0] = formats[0];

	if (mipsScreenTypeWanted)
		MipsScreenNumToPriv(0)->type = mipsScreenTypeWanted;
	else {
		MipsScreenNumToPriv(COLOR_UNIT)->type = MIPS_SCRTYPE_COLOR;
		MipsScreenNumToPriv(MONO_UNIT)->type = MIPS_SCRTYPE_MONO;
	}

	for (i = 0; i < MAXSCREENS; i++) {
		MipsScreenPtr pm = MipsScreenNumToPriv(i);
		Bool(*fun)();

		if (pm->type == MIPS_SCRTYPE_DISABLED)
			continue;

		pm->unit = i;

		if (!(fun = mipsMapProc[mipsSysType][pm->type]) ||
			!(*fun)(pm))
			continue;

		if (monitorResolution)
			pm->dpi = monitorResolution;

		if (mipsNoFBCache)
			pm->fbnorm = pm->fbnocache;

		if (mipsNoFBHW)
			pm->cap = 0;

		if (pm->bitsPerPixel > 1) {
			screenInfo->numPixmapFormats = 2;
			screenInfo->formats[1] = formats[1];
		}

		(void) AddScreen(mipsScreenInit, argc, argv);
	}
}

/*ARGSUSED*/
Bool
mipsScreenInit(index, pScreen, argc, argv)
	int index;
	ScreenPtr pScreen;
	int argc;
	char *argv[];
{
	MipsScreenPtr pm = MipsScreenNumToPriv(index);
	int fbtype = pm->type;
	Bool (*fun)();
	extern Bool cfbScreenInit();
	extern Bool mfbScreenInit();
	extern Bool mipsCreateDefColormap();
	
	fun = fbtype == MIPS_SCRTYPE_COLOR ?
		cfbScreenInit : mfbScreenInit;
	if (!(*fun)(pScreen, (pointer) pm->fbnorm,
		pm->scr_width, pm->scr_height,
		pm->dpi, pm->dpi,
		pm->fb_width))
		return FALSE;

	if (fbtype == MIPS_SCRTYPE_COLOR) {
#ifndef STATIC_COLOR
		pScreen->InstallColormap = mipsInstallColormap;
		pScreen->UninstallColormap = mipsUninstallColormap;
		pScreen->ListInstalledColormaps = mipsListInstalledColormaps;
		pScreen->StoreColors = mipsStoreColors;
#endif /* STATIC_COLOR */
		if (pm->depth == 4)
			mipsFixScreen4(pScreen);
	}
	else {
		pScreen->whitePixel =   1;
		pScreen->blackPixel =   0;
	}

	pm->CloseScreen = pScreen->CloseScreen;
	pScreen->CloseScreen = mipsCloseScreen;
	pScreen->SaveScreen = mipsSaveScreen;
	(void) mipsSaveScreen(pScreen, SCREEN_SAVER_FORCER);

#ifdef X11R4
	{
		extern miPointerCursorFuncRec mipsPointerCursorFuncs;

		if (!miDCInitialize(pScreen, &mipsPointerCursorFuncs))
			return FALSE;
	}
#else /* X11R4 */
	if (!mipsCursorInit(pm, pScreen))
		return FALSE;
#endif /* X11R4 */

	return mipsCreateDefColormap(index, pScreen,
		whitePixelName, blackPixelName);
}

static
systemType()
{
    switch (cpubd()) {
	case BRDTYPE_R3030:
	    mipsSysType = RS3230;
	    break;

	case BRDTYPE_I2000:
	case BRDTYPE_I2000S:
	default:
	    mipsSysType = RS2030;
	    break;
    }
}

static Bool
mipsSaveScreen(pScreen, on)
	ScreenPtr pScreen;
	Bool on;
{
	MipsScreenPtr pm = MipsScreenToPriv(pScreen);

	if (on != SCREEN_SAVER_ON)
		lastEventTime = GetTimeInMillis();

	if (pm->Blank)
		(*pm->Blank)(pm, on);

	return TRUE;
}

static Bool
mipsCloseScreen(i, pScreen)
    int		i;
    ScreenPtr	pScreen;
{
    MipsScreenPtr   pm = MipsScreenToPriv(pScreen);
    Bool	    ret;

#ifdef SYSV
    (void) sigseg (SIGPOLL, SIG_IGN);
#else
    (void) signal (SIGPOLL, SIG_IGN);
#endif
    pScreen->CloseScreen = pm->CloseScreen;
    ret = (*pScreen->CloseScreen) (i, pScreen);
    (void) (*pScreen->SaveScreen) (pScreen, SCREEN_SAVER_OFF);
    return ret;
}

/* Open input from the mouse and keyboard */

/* ARGSUSED */
void
InitInput(argc, argv)
int argc;
char *argv[];
{
    extern int	sigIOfunc();
    static int	zero = 0;

    initKeybd();

    pPointer = AddInputDevice(mipsMouseProc, TRUE);
    pKeyboard = AddInputDevice(mipsKeybdProc, TRUE);

    RegisterPointerDevice(pPointer);
    RegisterKeyboardDevice(pKeyboard);
    miRegisterPointerDevice(screenInfo.screens[0], pPointer);

#ifdef X11R4
    SetInputCheck(&zero, &mipsIOReady);
#else /* X11R4 */
    if (!mieqInit(pKeyboard, pPointer))
	return;
#endif /* X11R4 */

#ifdef SYSV
    (void) sigset(SIGPOLL, sigIOfunc);
#else /* SYSV */
    (void) signal(SIGPOLL, sigIOfunc);
#endif /* SYSV */
}

/* DDX - specific abort routine.  Called by AbortServer(). */

void
AbortDDX()
{
	MipsScreenPtr pm;

	for (pm = MipsScreenNumToPriv(0);
		pm < MipsScreenNumToPriv(MAXSCREENS); pm++)
		if (pm->Close)
			(*pm->Close)(pm);
}

/* Called by GiveUp(). */

void
ddxGiveUp()
{
	AbortDDX();
}

#define	BAD_ARG	(-66)	/* convenient invalid argument */

/* Process command line arguments specific to mips Xserver */
int
ddxProcessArgument(argc, argv, i)
	int argc;
	char *argv[];
	int i;
{
	int v;
	static void BadUse();

	argv += i;
	argc -= i;

	if (matcharg(argv[0], "-bp") == 0) {
		if (argc < 2)
			BadUse();
		blackPixelName = argv[1];
		return 2;
	}
	if (matcharg(argv[0], "-wp") == 0) {
		if (argc < 2)
			BadUse();
		whitePixelName = argv[1];
		return 2;
	}
#ifdef X11R4
	if (matcharg(argv[0], "-mb") == 0) {
		extern int motionBufferSize;

		if (argc < 2)
			BadUse();
		v = atoi(argv[1]);
		if (v < 0)
			v = 0;
		if (v > 1024)
			v = 1024;
		motionBufferSize = v;
		return 2;
	}
#endif /* X11R4 */
	if (matcharg(argv[0], "-pl*ock") == 0) {
		(void) plock(PROCLOCK);
		return 1;
	}
	if (matcharg(argv[0], "-tl*ock") == 0) {
		(void) plock(TXTLOCK);
		return 1;
	}
	if (matcharg(argv[0], "-dl*ock") == 0) {
		(void) plock(DATLOCK);
		return 1;
	}
	if (matcharg(argv[0], "-ni*ce") == 0) {
		if (argc < 2)
			BadUse();
		(void) nice(atoi(argv[1]));
		return 2;
	}
	if (matcharg(argv[0], "-nor*eset") == 0) {
		DDXnoreset = 1;
		return 1;
	}
	if (matcharg(argv[0], "-xled1") == 0) {
		DDXxled1 = 1;
		return 1;
	}
	if (matcharg(argv[0], "-xled2") == 0) {
		DDXxled2 = 1;
		return 1;
	}
	if (matcharg(argv[0], "-xled3") == 0) {
		DDXxled3 = 1;
		return 1;
	}
	if (matcharg(argv[0], "-vi*sual") == 0) {
		if (argc < 2)
			BadUse();

		if ((v = matchargs(argv[1], BAD_ARG,
			"d*efault", -1,
			"staticg*ray", StaticGray,
			"g*rayscale", GrayScale,
			"staticc*olor", StaticColor,
			"p*seudocolor", PseudoColor,
			(char *) 0)) == BAD_ARG)
			BadUse();

		defaultColorVisualClass = v;
		return 2;
	}
	if (matcharg(argv[0], "-en*able") == 0) {
		if (argc < 2)
			BadUse();
		if ((v = matchargs(argv[1], BAD_ARG,
			"c*olor", MIPS_SCRTYPE_COLOR,
			"m*onochrome", MIPS_SCRTYPE_MONO,
			(char *) 0)) == BAD_ARG)
			BadUse();
		mipsScreenTypeWanted = v;
		return 2;
	}
	if (matcharg(argv[0], "-mo*nochrome") == 0) {
		mipsScreenTypeWanted = MIPS_SCRTYPE_MONO;
		return 1;
	}
	if (matcharg(argv[0], "-1*9inch") == 0) {
		mipsMonitorSize = 1;
		return 1;
	}
	if (matcharg(argv[0], "-noca*che") == 0) {
		mipsNoFBCache = 1;
		return 1;
	}
	if (matcharg(argv[0], "-nohw") == 0) {
		mipsNoFBHW = 1;
		return 1;
	}
	if (matcharg(argv[0], "-k*eyboard") == 0) {
		if (argc < 2)
			BadUse();
		if ((v = matchargs(argv[1], BAD_ARG,
			"de*fault", DEFAULT_KEYBOARD,
#ifdef AT_KEYBOARD
			"at", AT_KEYBOARD,
#endif				/* AT_KEYBOARD */
#ifdef XT_KEYBOARD
			"xt", XT_KEYBOARD,
#endif				/* XT_KEYBOARD */
#ifdef UNIX1_KEYBOARD
			"unix", UNIX1_KEYBOARD,
#endif				/* UNIX1_KEYBOARD */
			(char *) 0)) == BAD_ARG)
			BadUse();
		keybdPriv.type = v;

		if (argc < 3 || argv[2][0] == '-')
			return 2;
			
		keybdPriv.unit = atoi(argv[2]);
		return 3;
	}
	if (matcharg(argv[0], "-po*inter") == 0) {
		if (argc < 2)
			BadUse();
		if ((v = matchargs(argv[1], BAD_ARG,
			"de*fault", MIPS_MOUSE_DEFAULT,
			"m*", MIPS_MOUSE_MOUSEMAN,
			(char *) 0)) == BAD_ARG)
			BadUse();
		mousePriv.type = v;

		if (argc < 3 || argv[2][0] == '-')
			return 2;
			
		mousePriv.unit = atoi(argv[2]);

		if (argc < 4 || argv[3][0] == '-')
			return 3;

		mousePriv.baud = atoi(argv[3]);

		if (argc < 5 || argv[4][0] == '-')
			return 4;

		mousePriv.rate = atoi(argv[5]);

		return 5;
	}
#if NETWORK_KEYBD
	if (matcharg(argv[0], "-ne*tkeybd") == 0) {
		if (argc < 2)
			BadUse();
		netKeybdAddr = argv[1];
		return 2;
	}
#endif				/* NETWORK_KEYBD */
#if EMULATE_COLOR || EMULATE_MONO
	if (matcharg(argv[0], "-em*ulate") == 0) {
		DDXemulate = 1;
		return 1;
	}
#endif				/* EMULATE_COLOR || EMULATE_MONO */
#if PIXIE
	if (matcharg(argv[0], "-pi*xie") == 0) {
		pixie = 1;
		return 1;
	}
#endif				/* PIXIE */
	return 0;
}

/* print use message and quit */
static void
BadUse()
{
	extern void UseMsg();

	UseMsg();
	exit(1);
}

/* Print use message for command line arguments specific to mips Xserver */

static char *usetext[] = {
"-bp color		set default black-pixel color\n\
-wp color		set default white-pixel color\n\
-plock			process lock the server\n\
-tlock			text lock the server\n\
-dlock			data lock the server\n\
-nice incr		change the scheduling priority of the server\n\
-noreset		do not reset server on last client close\n\
-xled1			use LED for xled1 instead of Scroll Lock\n\
-xled2			use LED for xled2 instead of Caps Lock\n\
-xled3			use LED for xled3 instead of Num Lock\n\
-visual PseudoColor|StaticColor|GrayScale|StaticGray\n\
			use grayscale/color visual on the color screen\n\
-enable color|mono	enable the color and/or mono screens\n\
-19inch			set color screen dpi for 19\" monitor\n\
-keyboard Default|AT|XT|UNIX [unit#]\n\
			select type of keyboard to use\n\
-pointer Default|Mouseman [unit#] [baud] [sample_rate]\n\
			select type of pointer device to use",
#ifdef X11R4
"-mb size		size of motion buffer (0-1024, default 100)\n",
#endif /* X11R4 */
#if NETWORK_KEYBD
"-netkeybd net-addr	connect to keyboard at network address",
#endif /* NETWORK_KEYBD */
#if EMULATE_COLOR || EMULATE_MONO
"-emulate		emulate color or mono screens",
#endif /* EMULATE_COLOR || EMULATE_MONO */
#if PIXIE
"-pixie			enable use of pixie analysis",
#endif /* PIXIE */
	0
};

void
ddxUseMsg()
{
	char **p;

	for (p = usetext; *p; p++)
		ErrorF("%s\n", *p);
}

/*
 * match argument against list of keywords
 * "*" in keyword allows prefix match
 */

#include <varargs.h>
#include <ctype.h>

/* arg, bad val, key, ret val, key, ret val, 0 */
/*VARARGS*/
static int
matchargs(va_alist)
va_dcl
{
	va_list ap;
	char *arg, *key;
	int ret;

	va_start(ap);
	arg = va_arg(ap, char *);
	ret = va_arg(ap, int);

	while (key = va_arg(ap, char *)) {
		int val = va_arg(ap, int);
		char *ac = arg;
		int star = 0;

		for (;;) {
			char k = *key++, a = *ac++;

			if (isascii(a) && isupper(a))
				a = tolower(a);

			/* XXX once we've seen the star we have
			to remember abbrevs are OK */
			if (k == '*') {
				star = 1;
				if (a == 0) {
					ret = val;
					break;
				}
				ac--;
				continue;
			}
			if (a != k && !star)
				break;
			if (a == 0) {
				ret = val;
				break;
			}
		}
	}
	va_end(ap);
	return ret;
}

static int
matcharg(arg, key)
	char *arg, *key;
{
	return matchargs(arg, -1, key, 0, (char *) 0);
}
