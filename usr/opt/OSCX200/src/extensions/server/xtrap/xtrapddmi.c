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
**************************************************************************
**                   DIGITAL EQUIPMENT CORPORATION                      **
**                         CONFIDENTIAL                                 **
**    NOT FOR MODIFICATION OR REDISTRIBUTION IN ANY MANNER WHATSOEVER   **
**************************************************************************
*/
/*****************************************************************************
Copyright 1987, 1988, 1989, 1990, 1991 by Digital Equipment Corp., Maynard, MA

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

*****************************************************************************/
/*
 *  ABSTRACT:
 *
 *      This module is the platform-specific but conditionally independent
 *      code for the XTrap extension (usually I/O or platform setup).
 *      This is shared code and is subject to change only by team approval.
 *
 *  CONTRIBUTORS:
 *
 *      Dick Annicchiarico
 *      Robert Chesler
 *      Gene Durso
 *      Marc Evans
 *      Alan Jamison
 *      Mark Henry
 *      Ken Miller
 *
 */
#ifndef lint
static char RCSID[] = "$Header: /alphabits/u3/x11/ode/rcs/x11/src/extensions/server/xtrap/xtrapddmi.c,v 1.1.2.3 92/04/27 14:04:57 Leela_Obilichetti Exp $";
#endif

#include <errno.h>
#include "Xos.h"
#ifdef PC
# include "fcntl.h"
# include "io.h"
# define O_NDELAY 0L
#endif

#define NEED_REPLIES
#define NEED_EVENTS
#include "X.h"        /* From library include environment */
#include "input.h"    /* From server include env. (must be before Xlib.h!) */
#ifdef PC
# include "scrninst.h"
#else
# include "scrnintstr.h"
#endif

#include "xtrapdi.h"
#include "xtrapddmi.h"
#include "xtrapproto.h"

extern int XETrapErrorBase;
extern xXTrapGetAvailReply XETrap_avail;
extern DevicePtr XETrapKbdDev;
extern DevicePtr XETrapPtrDev;

/*
 *  DESCRIPTION:
 *
 *      This function performs the platform specific setup for server
 *      extension implementations.
 */
void XETrapPlatformSetup()
{
}


#if !defined _XINPUT
/*
 *  DESCRIPTION:
 *
 *  This routine processes the simulation of some input event.
 *
 */
#ifdef FUNCTION_PROTOS
int XETrapSimulateXEvent(register xXTrapInputReq *request,
    register ClientPtr client)
#else
int XETrapSimulateXEvent(request, client)
    register xXTrapInputReq *request;
    register ClientPtr client;
#endif
{
    ScreenPtr pScr;
    int status = Success;
    xEvent xev;
    register int x = request->input.x;
    register int y = request->input.y;
    DevicePtr keydev = LookupKeyboardDevice();
    DevicePtr ptrdev = LookupPointerDevice();

    if (request->input.screen < screenInfo.numScreens)
    {
#if (!defined X11R3 || defined MITR5)
        pScr = screenInfo.screens[request->input.screen];
#else
        pScr = &screenInfo.screen[request->input.screen];
#endif
    }
    else
    {   /* Trying to play bogus events to this WS! */
#ifdef VERBOSE
        ErrorF("%s:  Trying to send events to screen %d!\n", XTrapExtName,
            request->input.screen);
#endif
        status = XETrapErrorBase + BadScreen;
    }
    /* Fill in the event structure with the information
     * Note:  root, event, child, eventX, eventY, state, and sameScreen
     *        are all updated by FixUpEventFromWindow() when the events
     *        are delivered via DeliverDeviceEvents() or whatever.  XTrap
     *        needs to only concern itself with type, detail, time, rootX, 
     *        and rootY.
     */
    if (status == Success)
    {
        xev.u.u.type   = request->input.type;
        xev.u.u.detail = request->input.detail;
        xev.u.keyButtonPointer.time   = GetTimeInMillis();
        xev.u.keyButtonPointer.rootX = x;
        xev.u.keyButtonPointer.rootY = y;

        if (request->input.type == MotionNotify)
        {   /* Set new cursor position on screen */
            XETrap_avail.data.cur_x = x;
            XETrap_avail.data.cur_y = y;
            if (!(*pScr->SetCursorPosition)(pScr, x, y, xFalse))
            {
                status = BadImplementation;
            }
        }
    }
    if (status == Success)
    {
        switch(request->input.type)
        {   /* Now process the event appropriately */
            case KeyPress:
            case KeyRelease:
#if !defined X11R3 || defined VMSDW_V3
                (*XETrapKbdDev->realInputProc)(&xev,keydev, 1L);
#else
                (*XETrapKbdDev->processInputProc)(&xev,keydev);
#endif
                break;
            case MotionNotify:
            case ButtonPress:
            case ButtonRelease:
#if !defined X11R3 || defined VMSDW_V3
                (*XETrapPtrDev->realInputProc)(&xev,ptrdev, 1L);
#else
                (*XETrapPtrDev->processInputProc)(&xev,ptrdev);
#endif
                break;
            default:
                status = BadValue;
                break;
        }
    }
    return(status);
}
#endif /* _XINPUT */

#if defined vms && !defined LINKED_IN
/* Used by swapping code not visible from VMS (from main.c) */
#ifndef BLADE
void
NotImplemented()
{
    FatalError("Not implemented");
}
#endif

int
#ifdef __STDC__
ProcBadRequest( ClientPtr client)
#else
ProcBadRequest(client)
    ClientPtr client;
#endif
{
    return (BadRequest);
}

#endif /* vms && ! LINKED_IN */
