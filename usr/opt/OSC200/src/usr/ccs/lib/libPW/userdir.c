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
static char	*sccsid = "@(#)$RCSfile: userdir.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 02:43:31 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 

/*
 * COMPONENT_NAME: (LIBPW) Programmers Workbench Library
 *
 * FUNCTIONS: userdir
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 */

#include <stdio.h>
#include <pwd.h>

/*
 * FUNCTION: Gets user's login directory.
 *
 * RETURN VALUE DESCRIPTIONS:
 *	Returns pointer to login directory on success,
 *	0 on failure.
 */
/*
	Gets user's login directory.
	The argument must be an integer.
	Note the assumption about position of directory field in
	password file (no group id in password file).
	Returns pointer to login directory on success,
	0 on failure.
        Remembers user ID and login directory for subsequent calls.
 */

#ifndef	NLS
#define NLstrncpy strncpy
#endif


char *
userdir(uid_t uid)
{
#define	ODIRMAX	33
	static uid_t ouid = -1;
	static char odir[ODIRMAX];
	struct passwd *pwd;

	if (ouid != uid || ouid == -1) {
		if ((pwd = getpwuid(uid)) == NULL)
		    return(NULL);
		NLstrncpy(odir, pwd->pw_dir, ODIRMAX);
		odir[ODIRMAX - 1] = '\0';
	}
	return(odir);
}
