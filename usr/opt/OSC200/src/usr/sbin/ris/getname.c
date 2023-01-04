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
static char *rcsid = "@(#)$RCSfile: getname.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/15 19:01:02 $";
#endif
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <netinet/in.h>

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <netdb.h>

char line[MAXHOSTNAMELEN + 1];
char domain[MAXHOSTNAMELEN + 1];
int lflag=0;
char *index();

/*
 * This program translates an IP address into a hostname suitable
 * for inclusion in the "risdb" database.  The name returned
 * is user-friendly, ie. a short name if within the local domain, or
 * a fully qualified domain name if outside of the local domain.
 *
 * If the -l switch is specified, the program translates a hostname
 * into a fully qualified domian name, suitable for inclusion in the
 * .rhosts file on the server.
 */
main(argc, argv)
int argc;
char *argv[];
{
	argc--, argv++;
	for ( ; argc >  0 && argv[0][0] == '-'; argc--, argv++) {
		for (argv[0]++; *argv[0]; argv[0]++)
			switch (*argv[0]) {
			/* We may want to add more cases later */
			case 'l':
				lflag=1;
				break;
			}
	}

	/* Check for bad usage */
	if ((argc < 1) || (argc > 1))
		exit(-1);

	if (lflag == 1) {
		if ((fullname(*argv)) < 0)
			exit(-1);
	}
	else {
		if ((addrtoname(*argv)) < 0)
			exit(-1);
	}
	printf("%s\n", line);
	exit(0);
}

/*
 * Parse the IP address into a hostname suitable for use in "risdb".
 * Input is an IP address, we return 0 on success (with "line" containing
 * the name value) or return -1 on error.
 */
addrtoname(addr)
char *addr;
{
	struct hostent *hp;
	struct in_addr in;
	char *cp;

	if (gethostname(domain, MAXHOSTNAMELEN) == 0
	&& (cp = index(domain, '.')))
		(void) strcpy(domain, cp + 1);
	else
		domain[0] = 0;

	/* addr is an IP address in dotted decimal notation */
	in.s_addr = inet_addr(addr);
	hp = gethostbyaddr(&in, sizeof (struct in_addr), AF_INET);
	if (hp) {
		/* return a short name iff the host is in our domian */
		if ((cp = index(hp->h_name, '.'))
		&& !strcmp(cp + 1, domain))
			*cp = 0;
		strcpy(line, hp->h_name);
		return(0);
	}
	else {
		/* name lookup failed */
		return(-1);
	}
}


/*
 * Parse a hostname into an official hostname (the fully qualified
 * domain name).  Input is a hostname, we return 0 on success
 * (with "line" containing the name value) or return -1 on error.
 */
fullname(name)
char *name;
{
	struct hostent *hp;

	hp = gethostbyname(name);
	if (hp) {
		strcpy(line, hp->h_name);
		return(0);
	}
	else {
		/* name lookup failed */
		return(-1);
	}
}
