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
static char *rcsid = "@(#)$RCSfile: screentest.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/15 18:35:36 $";
#endif

/*
 * screentest.c	
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
	struct screen_data scdata;
	int cointoss = 0;

	if (argc != 1) {
		fprintf(stderr, "usage: screentest\n");
		exit(1);
	}
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("screentest: socket");
		exit(1);
	}

	scdata.sd_xid = 0;

	while (1) {
	    scdata.sd_family = AF_UNSPEC;
	    if (ioctl(s, SIOCSCREEN, (caddr_t)&scdata) < 0) {
		perror("ioctl (SIOCSCREEN)");
		exit(1);
	    }
	    cointoss++;
	    if (cointoss & 1)
		scdata.sd_action = SCREEN_ACCEPT;
	    else if (cointoss & 2)
		scdata.sd_action = SCREEN_NOTIFY;
	    PrintScreenData(&scdata);
	    printf("\n");
	}
}

PrintScreenData(sdp)
register struct screen_data *sdp;
{
	printf("af %d count %d dlen %d xid %x action %x",
		sdp->sd_family,
		sdp->sd_count, sdp->sd_dlen, sdp->sd_xid, sdp->sd_action);
	if (sdp->sd_action & SCREEN_ACCEPT)
		printf(" ACCEPT");
	else
		printf(" REJECT");
	if (sdp->sd_action & SCREEN_NOTIFY)
		printf(" NOTIFY");
	printf("\n");

	PrintIPHeader(&(sdp->sd_arrival), sdp->sd_data, sdp->sd_dlen);
}

yyerror(s)
char *s;
{
	fflush(stdout);
	fprintf(stderr, "%s\n", s);
}

/* need this here so we can link without parser code */
yywarn(s)
char *s;
{
	fflush(stdout);
	fprintf(stderr, "Warning: %s\n", s);
}
