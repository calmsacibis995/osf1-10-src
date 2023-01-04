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
/* $XConsortium: evtomask.c,v 1.8 91/02/20 18:49:00 rws Exp $ */
/* Copyright    Massachusetts Institute of Technology    1987	*/

/*
Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in advertising or
publicity pertaining to distribution of the software without specific,
written prior permission.  M.I.T. makes no representations about the
suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.
*/

#include <X11/X.h>

#if __STDC__
#define Const const
#else
#define Const /**/
#endif

/*
 * This array can be used given an event type to determine the mask bits
 * that could have generated it.
 */
long Const _Xevent_to_mask [LASTEvent] = {
	0,						/* no event 0 */
	0,						/* no event 1 */
	KeyPressMask,					/* KeyPress */
	KeyReleaseMask,					/* KeyRelease */
	ButtonPressMask,				/* ButtonPress */
	ButtonReleaseMask,				/* ButtonRelease */
	PointerMotionMask|PointerMotionHintMask|Button1MotionMask|
		Button2MotionMask|Button3MotionMask|Button4MotionMask|
		Button5MotionMask|ButtonMotionMask,	/* MotionNotify */
	EnterWindowMask,				/* EnterNotify */
	LeaveWindowMask,				/* LeaveNotify */
	FocusChangeMask,				/* FocusIn */
	FocusChangeMask,				/* FocusOut */
	KeymapStateMask,				/* KeymapNotify */
	ExposureMask,					/* Expose */
	ExposureMask,					/* GraphicsExpose */
	ExposureMask,					/* NoExpose */
	VisibilityChangeMask,				/* VisibilityNotify */
	SubstructureNotifyMask,				/* CreateNotify */
	StructureNotifyMask|SubstructureNotifyMask,	/* DestroyNotify */
	StructureNotifyMask|SubstructureNotifyMask,	/* UnmapNotify */
	StructureNotifyMask|SubstructureNotifyMask,	/* MapNotify */
	SubstructureRedirectMask,			/* MapRequest */
	SubstructureNotifyMask|StructureNotifyMask,	/* ReparentNotify */
	StructureNotifyMask|SubstructureNotifyMask,	/* ConfigureNotify */
	SubstructureRedirectMask,			/* ConfigureRequest */
	SubstructureNotifyMask|StructureNotifyMask,	/* GravityNotify */
	ResizeRedirectMask,				/* ResizeRequest */
	SubstructureNotifyMask|StructureNotifyMask,	/* CirculateNotify */
	SubstructureRedirectMask,			/* CirculateRequest */
	PropertyChangeMask,				/* PropertyNotify */
	0,						/* SelectionClear */
	0,						/* SelectionRequest */
	0,						/* SelectionNotify */
	ColormapChangeMask,				/* ColormapNotify */
	0,						/* ClientMessage */
	0,						/* MappingNotify */
};
