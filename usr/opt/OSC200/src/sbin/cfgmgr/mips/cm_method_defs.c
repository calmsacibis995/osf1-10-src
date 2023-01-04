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
static char *rcsid = "@(#)$RCSfile: cm_method_defs.c,v $ $Revision: 1.1.8.2 $ (DEC) $Date: 1993/03/15 20:46:47 $";
#endif
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */

/*
 * OSF/1 Release 1.0
 *
 */


#include <stdio.h>
#include <sys/types.h>

#include "../cm_method.h"


#ifdef STREAMS_METHOD
extern STREAMS_method();
#endif
#ifdef FILSYS_METHOD
extern FILSYS_method();
#endif
#ifdef XTI_METHOD
extern XTI_method();
#endif
#ifdef INET_METHOD
extern INET_method();
#endif
#ifdef XNS_METHOD
extern XNS_method();
#endif
#ifdef BIN_COMPAT_METHOD
extern BIN_COMPAT_method();
#endif
#ifdef DEVICE_METHOD
extern DEVICE_method();
#endif


extern cmgr_cmfunc_t   cmgr_cmfunc[];
cmgr_cmfunc_t   cmgr_cmfunc[] = {

#ifdef STREAMS_METHOD
		{"streams",	&STREAMS_method},
#endif

#ifdef FILSYS_METHOD
		{"filsys",	&FILSYS_method},
#endif

#ifdef XTI_METHOD
        	{"xti",		&XTI_method},
#endif

#ifdef INET_METHOD
        	{"inet",	&INET_method},
#endif

#ifdef XNS_METHOD
        	{"xns",		&XNS_method},
#endif

#ifdef BIN_COMPAT_METHOD
        	{"compat",		&BIN_COMPAT_method},
#endif

#ifdef DEVICE_METHOD
                {"device",      &DEVICE_method},
#endif

	        {NULL,		NULL}
};


