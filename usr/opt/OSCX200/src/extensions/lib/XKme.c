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
#ifdef MODE_SWITCH
/************************************************************************
Copyright 1990 by Digital Equipment Corporation, Maynard, Massachusetts.
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

************************************************************************/
/*
 *  FACILITY:
 *
 *	X Keyboard Management Extension
 *
 *	Instruct the server to lock-down or unlock the keyboard mode-switching
 *	function.
 *
 */
#include <stdio.h>
#define NEED_REPLIES

#ifdef VMS
#include "Xlibint.h"
#include "Xlib.h"
#else
#include <Xlibint.h>
#include <X11/Xlib.h>
#endif

#include "xkmeproto_include.h"

/*************************************************************************
 * function declarations
 *************************************************************************/

/*static Bool   XKMEInitialize();
static Status XKMEDoKBModeSwitch();*/ 

/*
 * Holds the request type code for this extension. The request type code
 * for this extension may vary depending on how many extensions are
 * installed already, so the initial value given below will be added to
 * the base request code that is aquired when this extension is
 * installed.
 */
static int XKmeReqCode = 0;


/*
 *  ROUTINE:
 *
 *      XKMEInitialize
 *
 *     Attempt to initialize this extension in the server. 
 *     True if extension connection initialized successfully, False if not.
 *
 */

Bool XKMEInitialize(dpy)
Display *dpy;
{
    XExtCodes *codes;

    /*
     * XInitExtension determines if the extension exists. Then, it
     * allocates storage for maintaining the information about the 
     * extension on the cnnection, chanins this onto the extension
     * list for the connection, and returns the information the stub
     * implementor will need to access the extension. 
     * If the extension does not exist, XInitExtension returns NULL.
     */
    codes = XInitExtension(dpy, KMEXTENSIONNAME);

    if (!codes) return (xFalse);

    XKmeReqCode = codes->major_opcode;

    return (xTrue);
}

/*
**
**  ROUTINE:
**
**      XKMEDoKBModeSwitch - perform software-emulated keyboard mode-switching
**			     For V3, only the manipulation of the lock-down
**			     type of keyboard mode-switching is supported on 
**			     the server side.
**
**
**
**  FUNCTION VALUE OR COMPLETION CODES:
**
**	Success		Success
**	non-zero	Failure
**
**
**  X ERRORS:
**
**	BadName
**
**/
Status XKMEDoKBModeSwitch (dpy,CommandOption) 
    Display *dpy;
    int CommandOption;
{
    xKMEDoKBModeSwitchReq *req;
    xKMEDoKBModeSwitchRep  rep;

    if(CommandOption != LockDownModeSwitch && CommandOption != UnlockModeSwitch)
       return(KBModeSwitchInvalidCmd);

    if (!XKmeReqCode){
	 XKMEInitialize (dpy);
    }

    LockDisplay (dpy);

    /* 
     * Get the next available X request packet in the buffer.
     * It sets the `length' field to the size (in 32-bit words) of the
     * request. It also sets the `reqType' field in the request to
     * X_KMEDoKBModeSwitch, which is not what is needed.
     *
     * GetReq is a macro defined in Xlibint.h
     */
    GetReq(KMEDoKBModeSwitch, req);
    req->reqType = XKmeReqCode;
    req->minor_opcode = X_KMEDoKBModeSwitch;
    req->mode = CommandOption;
    if (!_XReply (dpy, (xReply *) &rep, 0, xFalse)) {
	UnlockDisplay (dpy);
	return KBModeSwitchFailure;
    }
    UnlockDisplay (dpy);
    SyncHandle();
    return (rep.status);
}
#endif MODE_SWITCH
