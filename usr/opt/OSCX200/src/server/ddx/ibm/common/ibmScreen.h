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
 * $XConsortium: ibmScreen.h,v 1.3 91/09/09 13:23:39 rws Exp $
 *
 * Copyright IBM Corporation 1987,1988,1989,1990,1991
 *
 * All Rights Reserved
 *
 * License to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of IBM not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS, AND 
 * NONINFRINGEMENT OF THIRD PARTY RIGHTS, IN NO EVENT SHALL
 * IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
*/

#ifndef IBMSCREEN_H
#define IBMSCREEN_H

	/*
	 * IBM specific, per-screen information
	 */

typedef struct IBMPERSCRINFO {
	BoxRec		 ibm_ScreenBounds;

	int		 ibm_NumFormats;
	PixmapFormatRec	*ibm_ScreenFormats;

	Bool		(*ibm_InitFunc)();
	int		(*ibm_ProbeFunc)(); /* returns file descriptor */
	void		(*ibm_HideCursor)();

	char		 *ibm_ScreenFlag;
	char		 *ibm_ScreenDevice;
	char		 *ibm_ScreenPointer;
	void		(*ibm_SaveFunc)();
	void		(*ibm_RestoreFunc)();

	ScreenPtr	  ibm_Screen;

	int		  ibm_ScreenFD;

	int		  ibm_Wanted;

	short		  ibm_CursorHotX;
	short		  ibm_CursorHotY;
	void		(*ibm_CursorShow)();
	CursorPtr	  ibm_CurrentCursor;
	int		  ibm_ScreenState ;
	unsigned	  ibm_DeviceID;
} ibmPerScreenInfo;

#define SCREEN_UNINITIALIZED	0
#define	SCREEN_INACTIVE		1
#define	SCREEN_ACTIVE		2

#define	ibmScreenBounds(n)	(&ibmScreens[(n)]->ibm_ScreenBounds)
#define	ibmScreenMinX(n)	(ibmScreenBounds(n)->x1)
#define	ibmScreenMinY(n)	(ibmScreenBounds(n)->y1)
#define	ibmScreenMaxX(n)	(ibmScreenBounds(n)->x2)
#define	ibmScreenMaxY(n)	(ibmScreenBounds(n)->y2)
#define	ibmNumFormats(n)	(ibmScreens[(n)]->ibm_NumFormats)
#define	ibmScreenFormats(n)	(ibmScreens[(n)]->ibm_ScreenFormats)
#define	ibmScreenInit(n)	(ibmScreens[(n)]->ibm_InitFunc)
#define	ibmHideCursor(n)	(ibmScreens[(n)]->ibm_HideCursor)
#define	ibmScreenFlag(n)	(ibmScreens[(n)]->ibm_ScreenFlag)
#define	ibmScreenDevice(n)	(ibmScreens[(n)]->ibm_ScreenDevice)
#define	ibmScreenPointer(n)	(ibmScreens[(n)]->ibm_ScreenPointer)
#define	ibmScreenFD(n)		(ibmScreens[(n)]->ibm_ScreenFD)
#define	ibmCursorHotX(n)	(ibmScreens[(n)]->ibm_CursorHotX)
#define	ibmCursorHotY(n)	(ibmScreens[(n)]->ibm_CursorHotY)
#define	ibmCursorShow(n)	(ibmScreens[(n)]->ibm_CursorShow)
#define	ibmCurrentCursor(n)	(ibmScreens[(n)]->ibm_CurrentCursor)
#define	ibmScreen(n)		(ibmScreens[(n)]->ibm_Screen)
#define	ibmScreenState(n)	(ibmScreens[(n)]->ibm_ScreenState)
#define	ibmSetScreenState(n,s)	(ibmScreens[(n)]->ibm_ScreenState=(s))
#define	ibmDeviceID(n)		(ibmScreens[(n)]->ibm_DeviceID)

/* Macro Calls For Dynamically changing the screen set */
#define	ibmSaveScreenInfo(n)	(* ibmScreens[(n)]->ibm_SaveFunc)(n)
#define	ibmRestoreScreenInfo(n)	(* ibmScreens[(n)]->ibm_RestoreFunc)(n)

extern	int		 ibmSaveScreen();
extern	ibmPerScreenInfo *ibmPossibleScreens[];
extern	ibmPerScreenInfo *ibmScreens[MAXSCREENS];
extern	int		 ibmNumScreens;
extern	int		 ibmXWrapScreen;
extern	int		 ibmYWrapScreen;
extern	int		 ibmCurrentScreen;
extern	int		 ibmUseHardware;
extern  char		 *ibmWhitePixelText;
extern  char		 *ibmBlackPixelText;
extern  Bool		 ibmDontZap;

#endif /* IBMSCREEN_H */
