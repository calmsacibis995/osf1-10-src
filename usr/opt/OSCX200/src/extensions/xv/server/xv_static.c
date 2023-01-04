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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: xv_static.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 92/09/25 19:44:15 $";
#endif

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

#include <stdio.h>

#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "os.h"
#include "scrnintstr.h"

/* There needs to be some generic routines in the workstaion device code
 * so that this code doesn't have to know about explicit data types here.
 */
#include <sys/workstation.h>
#include <sys/inputdevice.h>
#include <X11/Xserver/ws.h>

extern int wsScreenPrivateIndex;

void 
XvQueryAndLoadScreen(pScreen)
    ScreenPtr pScreen;
{
    wsScreenPrivate * wsp = (wsScreenPrivate *)
	pScreen->devPrivates[wsScreenPrivateIndex].ptr;
    ws_screen_descriptor * screenDesc = wsp->screenDesc;

    /* 
     * We only know about tx's right now for static
     */
    if ( strncmp("PMAG-RO", screenDesc->moduleID, strlen("PMAG-RO")) == 0 ||
         strncmp("PMAG-JA", screenDesc->moduleID, strlen("PMAG-JA")) == 0 ) 
	 XvropScreenInit(pScreen);
    
    return;
}


void 
XvUnLoadScreens()
{
    return;
}
