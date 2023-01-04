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
static char *rcsid = "@(#)$RCSfile: screenstat.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/15 18:27:10 $";
#endif

/* screenstat.c
 *
 * 19 December 1988	Jeffrey Mogul/DECWRL
 *	Created (mostly from ifconfig.c)
 *	Copyright 1989, 1990 Digital Equipment Corporation
 */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <net/if.h>
#include <netinet/in.h>
#include <net/gw_screen.h>

#include <stdio.h>

struct	screen_stats sstats;

main(argc, argv)
int argc;
char **argv;
{
	int s;

	if (argc != 1) {
		fprintf(stderr, "usage: screenstat\n");
		exit(1);
	}
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("screenstat: socket");
		exit(1);
	}
	if (ioctl(s, SIOCSCREENSTATS, (caddr_t)&sstats) < 0) {
		perror("ioctl (SIOCSCREENSTATS)");
		exit(1);
	}
	PrintScreenStat(&sstats);
}

PrintScreenStat(sstp)
struct screen_stats *sstp;
{
	printf("total packets screened:\t%d\n", sstp->ss_packets);
	printf("total accepted:\t%d\n", sstp->ss_accept);
	printf("total rejected:\t%d\n", sstp->ss_reject);
	printf("packets dropped:\n");
	printf("\tbecause buffer was full:\t%d\n", sstp->ss_nobuffer);
	printf("\tbecause user was out of sync:\t%d\n",
			sstp->ss_badsync);
	printf("\tbecause too old:\t%d\n", sstp->ss_stale);
	printf("total dropped:\t%d\n",
		sstp->ss_nobuffer + sstp->ss_badsync + sstp->ss_stale);
}
