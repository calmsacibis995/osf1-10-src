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
Copyright 1991 by Digital Equipment Corporation, Maynard, Massachusetts,
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

******************************************************************/
/* $XConsortium: ws.h,v 1.3 92/04/06 18:19:32 keith Exp $ */

#define NOMAPYET        (ColormapPtr) 1

#define  ARG_DPIX	(1 << 0)
#define  ARG_DPIY	(1 << 1)
#define  ARG_DPI	(1 << 2)
#define  ARG_BLACKVALUE	(1 << 3)
#define  ARG_WHITEVALUE	(1 << 4)
#define	 ARG_CLASS	(1 << 5)
#define	 ARG_EDGE_L	(1 << 6)
#define	 ARG_EDGE_R	(1 << 7)
#define	 ARG_EDGE_T	(1 << 8)
#define	 ARG_EDGE_B	(1 << 9)
#define  ARG_MONITOR	(1 << 10)
#define	 ARG_DEPTH	(1 << 11)
#define  ARG_TXXORFIX	(1 << 12)	/* for TX */
#define  ARG_TXBANKSW	(1 << 13)	/* for TX */
#define  ARG_TXRVISUAL	(1 << 14)	/* for TX */

typedef struct  {
	int flags;
	int dpix;
	int dpiy;
	int dpi;
	int class;
	char  *blackValue;
	char  *whiteValue;
	int edge_left;
	int edge_right;
	int edge_top;
	int edge_bottom;
	ws_monitor monitor;
	int depth;
	int txXorFix;		/* for TX */
	int txBankSwitch;	/* for TX */
	int txRootDepth;	/* for TX */
	int txRootClass;	/* for TX */
} ScreenArgsRec;

typedef struct {
	unsigned int		currentmask; 	/* saved plane mask */
	BoxPtr			cursorConstraint;
	ws_screen_descriptor	*screenDesc;
	ColormapPtr		pInstalledMap;
	ScreenArgsRec 		*args;
	Bool			(*CloseScreen)();
	void			(*CursorControl)();
} wsScreenPrivate;

typedef struct {
	char *moduleID;	/* graphic module ID */
	Bool (*createProc)();	/* create procedure for this hardware type */
} wsAcceleratorTypes;

extern void wsStoreColors();
extern void wsInstallColormap();
extern void wsUninstallColormap();
extern int  wsListInstalledColormaps();
extern int  wsScreenPrivateIndex;
extern Bool wsSaveScreen();
extern void wsMakeScreenOnly();
extern void wsInputOutputFinish();
extern int  wsRemapPhysToLogScreens;
extern void wsEnableScreen();
extern void wsDisableScreen();
extern int  wsPhysScreenNum();
extern void wsRegisterAbortProc();
extern void wsRegisterGiveUpProc();
extern wsScreenPrivate * wsAllocScreenInfo();
extern int  wsDisabledScreens[];
extern int  wsOnlyScreen;
extern int  wsPhysToLogScreens[];
extern int dpix, dpiy, dpi;

extern ScreenArgsRec screenArgs[];

extern ScreenPtr wsScreens[];
extern int class;
extern int forceDepth;
extern int fdPM;   /* this is the file descriptor for screen so
		    can do IOCTL to colormap */
extern int ws_cpu;

#define WSP_PTR(pScr) \
    ((wsScreenPrivate*)(pScr)->devPrivates[wsScreenPrivateIndex].ptr)
#define WS_SCREEN(pScr) (WSP_PTR(pScr)->screenDesc->screen)
#define wspCursorControl(psn, control) \
    if (psn >= 0) (*WSP_PTR(wsScreens[psn])->CursorControl)(psn, control)
