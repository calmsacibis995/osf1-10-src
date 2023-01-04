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
static char     *sccsid = "@(#)$RCSfile: domainname.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 01:24:46 $";
#endif
/*
 */

/* 
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 */
/*
 * domainname -- get (or set domainname)
 */
#include <stdio.h>

char domainname[256];
extern int errno;

main(argc,argv)
	char *argv[];
{
	int	myerrno;

	argc--;
	argv++;
	if (argc) {
		if (setdomainname(*argv,strlen(*argv)))
			perror("setdomainname");
		myerrno = errno;
	} else {
		getdomainname(domainname,sizeof(domainname));
		myerrno = errno;
		printf("%s\n",domainname);
	}
	exit(myerrno);
	/* NOTREACHED */
}
