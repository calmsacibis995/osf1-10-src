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
static char *rcsid = "@(#)$RCSfile: miinitext_load.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 92/11/24 10:35:33 $";
#endif
#include <X11/Xserver/loadable_server.h>
#include <stdio.h>
#include "misc.h"

extern Bool noTestExtensions;

/*ARGSUSED*/
void
InitExtensions(argc, argv)
    int		argc;
    char	*argv[];
{
    register 		int i;
    void 		(*InitProc)();

    for ( i = 0 ; i < LS_NumExtensionsLibraries; i++ ) {
	if ( noTestExtensions && (
	     strcmp(LS_GetLibName(LS_ExtensionsLibraries, i), 
		"xtest1") == 0 ||
	     strcmp(LS_GetLibName(LS_ExtensionsLibraries, i), 
		"xtest") == 0 ||
	     strcmp(LS_GetLibName(LS_ExtensionsLibraries, i), 
		"_dec_xtrapext") == 0 ) )
	    continue;
        if ( LS_LoadLibraryReqs(LS_ExtensionsLibraries, 
		i, 1, True) == 0 ) {
	    fprintf(stderr, "Cannot load all extension libraries.\n");
	    fprintf(stderr, "Library %s will not be an available extension.\n",
	        LS_GetLibName(LS_ExtensionsLibraries, i));
	    continue;
	}
	InitProc = LS_GetInitProc(LS_ExtensionsLibraries, i);
	if ( InitProc != NULL )  {
	    InitProc(/*argc, argv*/);
	    LS_MarkLibraryInited(LS_ExtensionsLibraries, i);
	}
	else {
	    fprintf(stderr, "Cannot initialize all extension libraries.\n");
	    fprintf(stderr, "Library %s will not be an available extension.\n",
	        LS_GetLibName(LS_ExtensionsLibraries, i));
	    fprintf(stderr, "Cannot find initialization routine %s\n",
		LS_GetInitProcName(LS_ExtensionsLibraries, i));
	}
    }
}
