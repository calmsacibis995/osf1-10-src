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
/************************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

********************************************************/

/* $XConsortium: globals.c,v 1.51 92/03/13 15:40:57 rws Exp $ */

#include "X.h"
#include "Xmd.h"
#include "misc.h"
#include "windowstr.h"
#include "scrnintstr.h"
#include "input.h"
#include "dixfont.h"
#include "site.h"
#include "dixstruct.h"
#include "os.h"

ScreenInfo screenInfo;
KeybdCtrl defaultKeyboardControl = {
	DEFAULT_KEYBOARD_CLICK,
	DEFAULT_BELL,
	DEFAULT_BELL_PITCH,
	DEFAULT_BELL_DURATION,
	DEFAULT_AUTOREPEAT,
	DEFAULT_AUTOREPEATS,
	DEFAULT_LEDS,
	0};

PtrCtrl defaultPointerControl = {
	DEFAULT_PTR_NUMERATOR,
	DEFAULT_PTR_DENOMINATOR,
	DEFAULT_PTR_THRESHOLD,
	0};

ClientPtr *clients;
ClientPtr  serverClient;
int  currentMaxClients;   /* current size of clients array */

WindowPtr *WindowTable;

unsigned long globalSerialNumber = 0;
unsigned long serverGeneration = 0;

/* these next four are initialized in main.c */
#if LONG_BIT == 64
int  ScreenSaverTime;
int  ScreenSaverInterval;
int  ScreenSaverBlanking;
int  ScreenSaverAllowExposures;
int  defaultScreenSaverTime = DEFAULT_SCREEN_SAVER_TIME;
int  defaultScreenSaverInterval = DEFAULT_SCREEN_SAVER_INTERVAL;
#else /* LONG_BIT == 32 */
long ScreenSaverTime;
long ScreenSaverInterval;
int  ScreenSaverBlanking;
int  ScreenSaverAllowExposures;
long defaultScreenSaverTime = DEFAULT_SCREEN_SAVER_TIME;
long defaultScreenSaverInterval = DEFAULT_SCREEN_SAVER_INTERVAL;
#endif /* LONG_BIT */

int  defaultScreenSaverBlanking = DEFAULT_SCREEN_SAVER_BLANKING;
int  defaultScreenSaverAllowExposures = DEFAULT_SCREEN_SAVER_EXPOSURES;
#ifndef NOLOGOHACK
int  logoScreenSaver = DEFAULT_LOGO_SCREEN_SAVER;
#endif

char *defaultFontPath = COMPILEDDEFAULTFONTPATH;
char *defaultTextFont = COMPILEDDEFAULTFONT;
char *defaultCursorFont = COMPILEDCURSORFONT;
char *rgbPath = RGB_DB;
char *defaultDisplayClass = COMPILEDDISPLAYCLASS;
FontPtr defaultFont;   /* not declared in dix.h to avoid including font.h in
			every compilation of dix code */
CursorPtr rootCursor;
ClientPtr requestingClient;	/* XXX this should be obsolete now, remove? */

TimeStamp currentTime;
TimeStamp lastDeviceEventTime;

Bool permitOldBugs = FALSE; /* turn off some error checking, to permit certain
			     * old broken clients (like R2/R3 xterms) to work
			     */

int defaultColorVisualClass = -1;
int monitorResolution = 0;

char *display;

#if LONG_BIT == 64
int TimeOutValue = DEFAULT_TIMEOUT * MILLI_PER_SECOND;
#else /* LONG_BIT == 32 */
long TimeOutValue = DEFAULT_TIMEOUT * MILLI_PER_SECOND;
#endif /* LONG_BIT */
int	argcGlobal;
char	**argvGlobal;
