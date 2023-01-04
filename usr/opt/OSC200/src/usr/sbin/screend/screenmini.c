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
static char *rcsid = "@(#)$RCSfile: screenmini.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/15 18:22:57 $";
#endif

/*
 * screenmini.c	
 *
 * 19 December 1988	Jeffrey Mogul/DECWRL
 *	Created (mostly from ifconfig.c)
 *	Copyright 1988, 1990 Digital Equipment Corporation
 */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <net/if.h>
#include <netinet/in.h>
#include <net/gw_screen.h>

#include <stdio.h>

main(argc, argv)
int argc;
char **argv;
{
	int s;
	int mode;
	struct screen_data scdata;
	int waste;

	if (argc != 2) {
		fprintf(stderr, "usage: screenmini waste\n");
		exit(1);
	}
	waste = atoi(argv[1]);
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("screentest: socket");
		exit(1);
	}

	scdata.sd_xid = 0;

	while (waste-- > 0) {
	    if (fork()==0) {
		scdata.sd_family = AF_UNSPEC;
		if (ioctl(s, SIOCSCREEN, (caddr_t)&scdata) < 0) {
		    perror("ioctl (SIOCSCREEN)");
		    exit(1);
		}
		exit(0);
	    }
	    wait(0);
	}
	printf("go\n");

	while (1) {
	    scdata.sd_family = AF_UNSPEC;
	    if (ioctl(s, SIOCSCREEN, (caddr_t)&scdata) < 0) {
		perror("ioctl (SIOCSCREEN)");
		exit(1);
	    }
	    scdata.sd_action = SCREEN_ACCEPT;
	}
}
