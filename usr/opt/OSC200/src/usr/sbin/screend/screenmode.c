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
static char *rcsid = "@(#)$RCSfile: screenmode.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/15 18:25:20 $";
#endif

/* screenmode.c
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

struct	screen_data sdat;

main(argc, argv)
int argc;
char **argv;
{
	int s;
	int mode;

	if (argc > 2) {
		fprintf(stderr, "usage: screenmode [on | off]\n");
		exit(1);
	}
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("screenmode: socket");
		exit(1);
	}
	if (argc == 2) {
		if (strcmp(argv[1], "on") == 0)
			mode = SCREENMODE_ON;
		else if (strcmp(argv[1], "off") == 0)
			mode = SCREENMODE_OFF;
		else {
			fprintf(stderr, "screenmode: `%s' is not legal\n",
				argv[1]);
			exit(1);
		}
	}
	else {
		mode = SCREENMODE_NOCHANGE;
	}
	if (ioctl(s, SIOCSCREENON, (caddr_t)&mode) < 0) {
		perror("ioctl (SIOCSCREENON)");
		exit(1);
	}
	if (argc < 2) {
	    printf("screening is %s\n",
			(mode == SCREENMODE_ON) ? "on" : "off");
	}
}
