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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: logname.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/08/24 20:04:24 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDSAUTH) security: authentication functions
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 *
 * logname.c   1.8  com/cmd/stat,3.1,9021 12/20/89 10:04:08";
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>

int
main(int argc, const char *argv[])
{
	char *name;

	if ((name = getlogin()) == NULL) {
		perror (argv[0]);
		exit(1);
	}

	puts(name);

	exit(0);
}
