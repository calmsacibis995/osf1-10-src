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
 * $XConsortium: ibmMouse.c,v 1.2 91/07/16 13:08:44 jap Exp $
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

#include "X.h"
#include "Xproto.h"
#include "miscstruct.h"
#include "scrnintstr.h"
#include "input.h"
#include "cursorstr.h"

#include "ibmIO.h"
#include "ibmMouse.h"
#include "ibmScreen.h"

#include "OSio.h"
#include "ibmTrace.h"

/***================================================================***/

DevicePtr	ibmPtr;
int		ibmDoPointerAccel=		0;
int		ibmPointerThreshold= 		0;
int		ibmPointerAccelNumerator= 	0;
int		ibmPointerAccelDenominator= 	0;

/***================================================================***/

void
ibmChangePointerControl(pDevice,pCtrl)
DevicePtr	pDevice;
PtrCtrl		*pCtrl;
{
int	scale;

    TRACE(("ibmChangePointerControl( pDevice= 0x%x, pCtrl= 0x%x )\n",
						pDevice,pCtrl));
    if ((pCtrl->num>0)&&(pCtrl->den>0)) {
	ibmPointerAccelNumerator=		pCtrl->num;
	ibmPointerAccelDenominator=		pCtrl->den;
    }
    else {
	if (pCtrl->num==-1)		ibmPointerAccelNumerator= 1;
	if (pCtrl->den==-1)		ibmPointerAccelDenominator= 1;
    }
    if (pCtrl->threshold>=0)
	ibmPointerThreshold=		pCtrl->threshold;
    else if (pCtrl->threshold==-1)
	ibmPointerThreshold=		1;
    ibmDoPointerAccel= (ibmPointerAccelNumerator!=ibmPointerAccelDenominator);
    return;
}
