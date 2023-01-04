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
static char	*sccsid = "@(#)$RCSfile: rdate.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/04/17 15:14:39 $";
#endif lint

/*
 */
/*-----------------------------------------------------------------------
 *	Modification History
 *
 *      4/16/91 -- Mary Walker
 *              Moved code to OSF - internationalized the messages
 *
 *      9/06/90 -- terry
 *              Modified code to use correct network address for 
 *              finding the time on a specific network.
 *              Also added exit(1) after "unknown network" message to
 *              exit gracefully.
 *
 *	4/15/85 -- jrs
 *		Use INADDR_BROADCAST as default when none specified
 *		to get around subnet addr problems.
 *
 *	4/5/85 -- jrs
 *		Created to allow machines to set time from network.
 *		Based on a concept by Marshall Rose of UC Irvine
 *		and the internet specifications for time server.
 *
 *-----------------------------------------------------------------------
 */

/*
 *	The syntax for this client is:
 *	rdate [-sv] [network]
 *
 *	where: 
 *		-s	Set time from network median
 *		-v	Print time for each responding network host
 *		network	The network broadcast addr to poll for time
 *
 *	If no switches are set, rdate will just report the network median time
 *	If no network is specified, rdate will use the host's primary network.
 *
 *	It is intended that rdate will normally be used in the /etc/rc file
 *	with the -s switch to set the system date.  This is especially useful
 *	on machines such as MicroVax I's that have no t.o.y. clock.
 */

#include <netdb.h>
#include <stdio.h>
#include <utmp.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <nl_types.h>
#include <locale.h>
#include "rdate_msg.h"

#define MSGSTR(n,s)catgets(catd,MS_RDATE,n,s)
nl_catd catd;

#define	WTMP	"/usr/adm/wtmp"
#define	RESMAX	100

struct	utmp wtmp[2] = { { "|", "", "", 0 }, { "{", "", "", 0 } };

struct	servent *getservbyname();
struct	netent *getnetbyname();
struct	hostent *gethostbyname();
struct	in_addr inet_makeaddr();
int	tcomp();

main(argc, argv)
int	argc;
char	**argv;
{
	int	set = 0;
	int	verbose = 0;
	int	on = 1;
	char	*net = NULL;
	struct	servent	*tserv;
	struct	netent	*tnet;
	struct	hostent	*thost;
	struct	sockaddr_in netaddr;
	struct	timeval baset, nowt, timeout;
	struct	timezone basez, nowz;
	char	hostnam[32], resbuf[16], *swtp;
	int	argp, wtmpfd, tsock, readsel, writesel, selmax, selvalue;
	int	rescount, median, addrsiz;
	unsigned int reslist[RESMAX], resvalue;

	for (argp = 1; argp < argc; argp++) {
		if (*argv[argp] == '-') {
			for (swtp = &argv[argp][1]; *swtp != '\0'; swtp++) {
				switch (*swtp) {

				case 's':	/* set time */
					set = 1;
					break;

				case 'v':	/* verbose report */
					verbose = 1;
					break;

				default:
					fprintf(stderr, 
						MSGSTR(RDATE_UNKWN_SW,
						"rdate: Unknown switch - %c\n"),
						 *swtp);
					exit(1);
				}
			}
		} else {
			net = argv[argp];
		}
	}

	/* research network and service information */

	if ((tserv = getservbyname("time", "udp")) == NULL) {
		fprintf(stderr, MSGSTR(RDATE_TIME_UNKWN, 
				       "rdate: Time service unknown\n"));
		exit(1);
	}
	netaddr.sin_family = AF_INET;
	netaddr.sin_port = tserv->s_port;
	if (net == NULL) {
		netaddr.sin_addr.s_addr = INADDR_BROADCAST;
	} else {
		if ((tnet = getnetbyname(net)) == NULL) {
			fprintf(stderr, MSGSTR(RDATE_NET_UNKWN, 
				       "rdate: Unknown network - %s\n"),
				        net);
			exit(1);
		}
		netaddr.sin_addr = inet_makeaddr(tnet->n_net, INADDR_ANY);
	}

	/* set up the socket and define the base time */

	if ((tsock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		fprintf(stderr, MSGSTR(RDATE_SOCKCREATE_FAIL,
				       "rdate: socket create failure\n"));
		exit(1);
	}
	if (setsockopt(tsock, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) < 0) {
		fprintf(stderr, MSGSTR(RDATE_BROAD_FAIL, 
				       "rdate: set broadcast failure\n"));
		exit(1);
	}
	(void) gettimeofday(&baset, &basez);

	/* set up for select, then yell for someone to tell us the time */

	timeout.tv_sec = 2;
	timeout.tv_usec = 0;
	readsel = 1 << tsock;
	writesel = 0;
	selmax = tsock + 1;
	rescount = 0;

	if (sendto(tsock, resbuf, sizeof(resbuf), 0, &netaddr,
				sizeof(netaddr)) < 0) {
		fprintf(stderr, MSGSTR(RDATE_SOCKSEND_FAIL, 
				       "rdate: socket send failure\n"));
		exit(1);
	}

	/* loop for incoming packets.  We will break out on error
	   or timeout period expiration */

	while ((selvalue = select(selmax, &readsel, &writesel, &writesel,
				&timeout)) > 0) {

		/* reset for next select */

		timeout.tv_sec = 2;
		timeout.tv_usec = 0;
		readsel = 1 << tsock;
		writesel = 0;
		selmax = tsock + 1;
		
		/* try to pick up packet */

		addrsiz = sizeof(netaddr);
		if (recvfrom(tsock, resbuf, sizeof(resbuf), 0, &netaddr,
				&addrsiz) != sizeof(resvalue)) {
			continue;
		}
		
		/* this little piece of code is to insure that all
		   incoming times are stamped from same base time */

		(void) gettimeofday(&nowt, &nowz);
		resvalue = ntohl(*(unsigned int *)resbuf) - 2208988800;
		reslist[rescount++] = resvalue - (nowt.tv_sec - baset.tv_sec);

		/* if we are verbose, explain what we just got */

		if (verbose != 0) {
			thost = gethostbyaddr((char *)&netaddr.sin_addr,
					sizeof(netaddr.sin_addr), AF_INET);
			if (thost == NULL)
			    printf(MSGSTR(RDATE_VERBOSE_UNKWN_MSG, 
				  "*Unknown*: %s"),
				   ctime((time_t *)&resvalue));
			else
			    printf(MSGSTR(RDATE_VERBOSE_MSG, 
				  "%s: %s"), thost->h_name, 
				   ctime((time_t *)&resvalue));
		}

		/* if list is full, we are done */

		if (rescount >= RESMAX) {
			selvalue = 0;
			break;
		}
	}

	/* make sure we did not end abnormally */

	if (selvalue != 0) {
		fprintf(stderr, MSGSTR(RDATE_SELECT_FAIL, 
				       "rdate: select failure\n"));
		exit(1);
	}

	/* cheap exit if time list is empty */

	if (rescount == 0) {
		printf(MSGSTR(RDATE_FAIL, "Network time indeterminate\n"));
		exit(0);
	}

	/* sort the time list and pick median */

	qsort(reslist, rescount, sizeof(resvalue), tcomp);
	median = (rescount - 1) / 2;

	/* adjust selected value from base time to present */

	(void) gettimeofday(&nowt, &nowz);
	resvalue = reslist[median] + (nowt.tv_sec - baset.tv_sec);

	/* if setting, do it, otherwise just print conclusions */

	if (set == 0) {
		fprintf(stderr, MSGSTR(RDATE_TIME, "Network time is %s"), 
			ctime((time_t *)&resvalue));
	} else {
		wtmp[0].ut_time = nowt.tv_sec;
		wtmp[1].ut_time = resvalue;
		nowt.tv_sec = resvalue;
		if (settimeofday(&nowt, &nowz) != 0) {
			fprintf(stderr, MSGSTR(RDATE_SET_FAIL, 
					       "rdate: Time set failed\n"));
			exit(1);
		}
		if ((wtmpfd = open(WTMP, O_WRONLY|O_APPEND)) >= 0) {
			(void) write(wtmpfd, wtmp, sizeof(wtmp));
			(void) close(wtmpfd);
		}
		printf(MSGSTR(RDATE_SET_SUCCESS, "Time set to %s"), 
		       ctime((time_t *)&resvalue));
	}
}

/*
 *	This function aids the sort in the main routine.
 *	It compares two unsigned ints and returns accordingly.
 */

tcomp(first, second)
unsigned int *first, *second;
{
	if (*first < *second) {
		return(-1);
	} else if (*first > *second) {
		return(1);
	} else {
		return(0);
	}
}







