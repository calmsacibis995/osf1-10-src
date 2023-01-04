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
/* $XConsortium: CloseHook.h,v 1.5 91/07/22 23:45:30 converse Exp $
 *
 * Copyright 1988 by the Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific, 
 * written prior permission. M.I.T. makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * The X Window System is a Trademark of MIT.
 *
 * The interfaces described by this header file are for miscellaneous utilities
 * and are not part of the Xlib standard.
 */

#ifndef _XMU_CLOSEHOOK_H_
#define _XMU_CLOSEHOOK_H_

#include <X11/Xfuncproto.h>

typedef caddr_t CloseHook;

typedef int (*XmuCloseHookProc)(
#if NeedFunctionPrototypes
    Display*	/* dpy */,
    caddr_t	/* data */
#endif 
);

_XFUNCPROTOBEGIN

extern CloseHook XmuAddCloseDisplayHook(
#if NeedFunctionPrototypes
    Display *	/* dpy */,
    XmuCloseHookProc	/* proc */,
    caddr_t	/* arg */
#endif
);

extern Bool XmuLookupCloseDisplayHook(
#if NeedFunctionPrototypes
    Display *	/* dpy */,
    CloseHook	/* handle */,
    XmuCloseHookProc	/* proc */,
    caddr_t	/* arg  */
#endif
);

extern Bool XmuRemoveCloseDisplayHook(
#if NeedFunctionPrototypes
    Display *	/* dpy */,
    CloseHook	/* handle */,
    XmuCloseHookProc	/* proc */,
    caddr_t	/* arg */
#endif
);

_XFUNCPROTOEND

#endif /* _XMU_CLOSEHOOK_H_ */
