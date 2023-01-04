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
 * $XConsortium: aixCursor.c,v 1.3 91/07/16 12:58:05 jap Exp $
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

#include <stdio.h>
#define NEED_EVENTS
#include "X.h"
#include "Xproto.h"
#include "input.h"
#include "scrnintstr.h"
#include "cursorstr.h"
#include "miscstruct.h"

#include "aixCursor.h"

#include "ibmScreen.h"
#include "ibmIO.h"
#include "ibmTrace.h"

#include "OSio.h"

/***============================================================***/

int
AIXSetCursorPosition( pScr, x, y, generateEvent )
    register ScreenPtr  pScr;
    register int        x,y;
    Bool        generateEvent;
{
xEvent          motion;
DevicePtr       mouse;

    TRACE(("AIXSetCursorPosition( pScr= 0x%x, x= %d, y= %d )\n",pScr,x,y));

    if (pScr->myNum!=ibmCurrentScreen) {

	/* if this function is defined */
	if( ibmHideCursor(ibmCurrentScreen) )
	        (*ibmHideCursor(ibmCurrentScreen))(ibmScreen(ibmCurrentScreen));
	/* if this function is defined */

	ibmCurrentScreen= pScr->myNum;
    }

    if (generateEvent) {
	ProcessInputEvents();
	motion.u.keyButtonPointer.rootX=        x;
	motion.u.keyButtonPointer.rootY=        y;
	motion.u.keyButtonPointer.time= lastEventTime;
	motion.u.u.type=                        MotionNotify;
	mouse= LookupPointerDevice();
	(*mouse->processInputProc)(&motion,mouse,1);
    }
    AIXCurrentX= x;
    AIXCurrentY= y;
    (*(ibmCursorShow(pScr->myNum)))(x,y);
    return TRUE;
}

/***============================================================***/

void
AIXPointerNonInterestBox( pScr, pBox )
ScreenPtr       pScr;
BoxPtr          pBox;
{

    TRACE(("AIXPointerNonInterestBox( pScr= 0x%x, pBox= 0x%x )\n"));

    return ;
}

/***============================================================***/

void
AIXConstrainCursor( pScr, pBox )
register ScreenPtr      pScr;
register BoxPtr         pBox;
{

    TRACE(("AIXConstrainCursor( pScr= 0x%x, pBox= 0x%x )\n"));

    return ;
}

/***============================================================***/

void
AIXCursorLimits( pScr, pCurs, pHotBox, pTopLeftBox )
ScreenPtr       pScr;
CursorPtr       pCurs;
BoxPtr          pHotBox;
BoxPtr          pTopLeftBox;
{
    TRACE(("AIXCursorLimits( pScr= 0x%x, pCurs= 0x%x, pHotBox= 0x%x, pTopLeftBox= 0x%x)\n", pScr, pCurs, pHotBox, pTopLeftBox));

    pTopLeftBox->x1= max( pHotBox->x1, 0 );
    pTopLeftBox->y1= max( pHotBox->y1, 0 );
    pTopLeftBox->x2= min( pHotBox->x2, pScr->width );
    pTopLeftBox->y2= min( pHotBox->y2, pScr->height );

    return ;
}
