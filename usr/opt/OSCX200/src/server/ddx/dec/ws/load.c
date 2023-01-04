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
static char *rcsid = "@(#)$RCSfile: load.c,v $ $Revision: 1.1.2.5 $ (DEC) $Date: 93/02/02 12:03:34 $";
#endif

#include <dlfcn.h>
#include <stdio.h>
#include <sys/types.h>
#include "X.h"
#include "scrnintstr.h"
#include "servermd.h"
#include <X11/Xserver/loadable_server.h>

int LoadAndInitDeviceModule(moduleID, argc, argv)
    char 	* moduleID;
    int 	argc;
    char 	** argv;
{
    int 	j;
    char 	* DeviceName;
    void	(*InitProc)();

    j = LS_GetLibraryReqByDeviceName(
	LS_DeviceLibraries, LS_NumDeviceLibraries, moduleID);
    if ( j >=  0 ) {
	if ( LS_LoadLibraryReqs(LS_DeviceLibraries, j, 1, True) == 0 ) {
	    ErrorF ( "Cannot load device library for device %s\n",
		moduleID);
	    return(-1);
	}
	InitProc = LS_GetInitProc(LS_DeviceLibraries, j);
	if ( InitProc == NULL ) {
	    ErrorF ("Cannot find initialization procedure.\n");
	    ErrorF ("                 device type: %s\n",moduleID);
	    ErrorF ("                library name: %s\n",
		LS_GetLibName(LS_DeviceLibraries, j));
	    ErrorF ("           library file name: %s\n",
		LS_GetLibFileName(LS_DeviceLibraries, j));
	    ErrorF ("initalization procedure name: %s\n",
		LS_GetInitProcName(LS_DeviceLibraries, j));
	    return(-1);
	}
	LS_MarkLibraryInited(LS_DeviceLibraries, j);

	j = AddScreen(InitProc, argc, argv);
    }
    else {
	ErrorF ("Workstation device failure.\n");
	ErrorF ("Cannot find loadable library to handle device %s.\n",
		moduleID);
	return(-1);
    }

    return(j);
}

/* 
 In order to do this, we need to somehow walk the list of devices
 so we know how many times to close each library.
 Or, we need a way of saying unconditionally close this library,
 regardless of the reference count. That's dangerous...
 In any case, the memory savings are nil since we will reopen the 
 library on starting up again.
 Replacement of the library is potentially dangerous since we make
 assumptions that the underlying system can support it in some way.
 I do not recommend doing this...
 jpl

void CloseDownDDX(screen)
{
    LS_UnLoadLibraryReqs(LS_DeviceLibraries, 0, LS_NumDeviceLibraries);
}
*/
