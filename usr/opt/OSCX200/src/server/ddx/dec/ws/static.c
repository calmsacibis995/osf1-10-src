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
static char *rcsid = "@(#)$RCSfile: static.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 93/02/02 12:05:41 $";
#endif

#include <stdio.h>
#include <sys/types.h>
#include "X.h"
#include "scrnintstr.h"
#include "servermd.h"
#include <sys/workstation.h>
#include <sys/inputdevice.h>
#include "ws.h"

extern wsAcceleratorTypes types[];
extern int num_accelerator_types;
extern Bool fbInitProc();

int LoadAndInitDeviceModule(moduleID, argc, argv)
	char 	* moduleID;
	int 	argc;
	char 	** argv;
{
	int j, DECaccelerator = FALSE;

	for (j = 0; j < num_accelerator_types; j++) {
	    if (strcmp (moduleID, types[j].moduleID) == 0) {
		DECaccelerator = TRUE;
		break;
	    }
	}
    	
        if(DECaccelerator)
	    j = AddScreen(types[j].createProc, argc, argv);
	else
	    j = AddScreen(fbInitProc, argc, argv);
	
	return(j);
}
