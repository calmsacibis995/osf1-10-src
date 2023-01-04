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
static char *rcsid = "@(#)$RCSfile: xv_load.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 92/10/23 15:55:15 $";
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

#include <X11/Xserver/loadable_server.h>

/* There needs to be some generic routines in the workstaion device code
 * so that this code doesn't have to know about explicit data types here.
 * Work to be done...
 */
#include <sys/workstation.h>
#include <sys/inputdevice.h>
#include <X11/Xserver/ws.h>

#define XV_EXTENSION_NAME "xv"

extern int wsScreenPrivateIndex;

int 	XvInitedLibraries[MAXSCREENS];
int 	XvNumInitedLibraries = 0;

void 
XvQueryAndLoadScreen(pScreen)
    ScreenPtr pScreen;
{
    wsScreenPrivate * wsp = (wsScreenPrivate *)
	pScreen->devPrivates[wsScreenPrivateIndex].ptr;
    ws_screen_descriptor * screenDesc = wsp->screenDesc;
    register int 	i;
    register char 	* ptr;
    void		* liblist;
    int			count;
    void 		(*InitProc)();

    /* Find this extension */
    i = LS_GetLibraryReqByLibName(
	LS_ExtensionsLibraries, LS_NumExtensionsLibraries,
	XV_EXTENSION_NAME);
    if ( i < 0 ) {
	fprintf(stderr, "Xv: cannot locate xv library in extensions list.\n");
	fprintf(stderr, "Xv: cannot load device dependent components.\n");
	return;
    }
    
    /* get the list of device specific components */
    if ( LS_GetSubLibList(LS_ExtensionsLibraries, i, &liblist, &count) 
	 == False ) {
	/* Debug 
	fprintf(stderr, "Xv: cannot find any device dependent components.\n");
	fprintf(stderr, "Xv: cannot load device dependent components.\n");
	*/
	return;
    }

    /* find a matching component, matching the device name from the 
     * driver to the device name in the component list 
     */
    i = LS_GetLibraryReqByDeviceName( liblist, count, screenDesc->moduleID);

    if ( i < 0 )
	return;

    /* load the library */
    if ( LS_LoadLibraryReqs(liblist, i, 1, True) == 0 ) {
	fprintf(stderr, "Xv: cannot load device dependend component for %s\n",
	    screenDesc->moduleID);
	return;
    }

    /* initialize the sub-extension */
    InitProc = LS_GetInitProc(liblist, i);

    if ( InitProc != NULL )  {
	InitProc(pScreen);
	LS_MarkLibraryInited(LS_ExtensionsLibraries, i);
    }
    else {
    	LS_UnLoadLibraryReqs(liblist, i, 1);
	fprintf(stderr, "Xv: cannot find initialization routine %s\n",
	    LS_GetInitProcName(LS_ExtensionsLibraries, i));
	fprintf(stderr, "    for device dependent component %s\n",
	    screenDesc->moduleID);
	return;
    }

    XvInitedLibraries[XvNumInitedLibraries++] = i;

    return;
}


void 
XvUnLoadScreens()
{
    register char 	* ptr;
    void		* liblist;
    int			count;
    register int	i;

    /* Find this extension */
    i = LS_GetLibraryReqByLibName(
	LS_ExtensionsLibraries, LS_NumExtensionsLibraries,
	XV_EXTENSION_NAME);
    if ( i < 0 ) 
	return;
    
    if ( LS_GetSubLibList(LS_ExtensionsLibraries, i, &liblist, &count) 
	 == False ) 
	return;

    /* unload them all. 
     * We need to remember what we've loaded since we have to unload 
     * the libraries the same number of times that we loaded them
     * to clear the reference counts.
     */
    for ( i = 0 ; i < XvNumInitedLibraries; i++ )
        LS_MarkForUnloadLibraryReqs(liblist, XvInitedLibraries[i], 1);

    return;
}

