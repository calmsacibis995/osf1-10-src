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
static char *rcsid = "@(#)$RCSfile: pfconfig.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/10 15:57:53 $";
#endif

/*
 * Based on:
 * static char *sccsid = "@(#)pfconfig.c  4.3	(ULTRIX)	2/21/91";
 */
/*
 * pfconfig.c
 *
 * Configure system-wide packet filter settings
 *
 * Usage:
 *	pfconfig [-/+p[romisc]] [-b[acklog] nnn] [-/+c[opyall]] [-a[ll]]
 *			[devicename ...]
 *
 * HISTORY:
 *	22 May 1990	Jeffrey Mogul	DECWRL
 *		Added -/+c[opyall]
 *
 *	24 July 1989	Jeffrey Mogul	DECWRL
 *		- Created.
 */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/pfilt.h>
#include <stdio.h>

int promisc = -1;
int copyall = -1;
int backlog = 0;
int printit = 1;
int doall = 0;
char *default_ifname = "pf0";

main(argc, argv)
int argc;
char **argv;
{
	while (argc > 1) {
	    if (argv[1][0] == '-') {
		switch (argv[1][1]) {
		    case 'p':
			promisc = 0;
			printit = 0;
			break;

		    case 'c':
			copyall = 0;
			printit = 0;
			break;

		    case 'a':
			doall++;
			break;

		    case 'b':
			if (argc < 3) {
			    fprintf(stderr,
			    	"%s: %s must be followed by a number\n",
					argv[0], argv[1]);
			    exit(1);
			}
			backlog = atoi(argv[2]);
			argc--;
			argv++;
			printit = 0;
			break;

		    default:
			fprintf(stderr, "%s: %s not a valid option\n",
				argv[0], argv[1]);
			exit(1);
		}
	    }
	    else if (argv[1][0] == '+') {
		switch (argv[1][1]) {
		    case 'c':
			copyall = 1;
			printit = 0;
			break;

		    case 'p':
			promisc = 1;
			printit = 0;
			break;

		    default:
			fprintf(stderr, "%s: %s not a valid option\n",
				argv[0], argv[1]);
			exit(1);
		}
	    }
	    else
		DoInterface(argv[1]);
	    argc--;
	    argv++;
	}

	if (doall)
	    DoAll();
}

DoInterface(ifname)
char *ifname;
{
	int fid;
	struct ifreq ifr;

	fid = GetPFfid(ifname);	
	if (fid < 0 ) {
	    perror(ifname);
	    exit(1);
	}
	strcpy(ifr.ifr_name, ifname);
	if (ioctl(fid, EIOCSETIF, &ifr) < 0) {
	    perror(ifname);
	    exit(1);
	}

	DoFid(fid);

	close(fid);
}
	
DoAll()
{
	int fid;
	struct ifreq ifr;
	int unit;
	char tryname[64];


	for (unit = 0; unit <= 9; unit++) {
	    sprintf(tryname, "%s%d", ENGENPREFIX, unit);
	    fid = GetPFfid(tryname);
	    if (fid < 0)
		continue;
	    sprintf(ifr.ifr_name, "%s%d", ENGENPREFIX, unit);
	    if (ioctl(fid, EIOCSETIF, &ifr) < 0) {
		continue;
	    }
	    DoFid(fid);
	}

	close(fid);
}

DoFid(fid)
int fid;
{
	if (promisc >= 0) {
	    if (ioctl(fid, EIOCALLOWPROMISC, &promisc) < 0) {
		perror("EIOCALLOWPROMISC");
		exit(1);
	    }
	}

	if (copyall >= 0) {
	    if (ioctl(fid, EIOCALLOWCOPYALL, &copyall) < 0) {
		perror("EIOCALLOWCOPYALL");
		exit(1);
	    }
	}

	if (backlog > 0) {
	    if (ioctl(fid, EIOCMAXBACKLOG, &backlog) < 0) {
		perror("EIOCMAXBACKLOG");
		exit(1);
	    }
	}

	if (printit) {
	    struct ifreq ifr;
	    int cur_copyall = -1;
	    int cur_promisc = -1;
	    int cur_backlog = -1;

	    if (ioctl(fid, EIOCIFNAME, &ifr) < 0) {
		perror("EIOCIFNAME");
		exit(1);
	    }
	    if (ioctl(fid, EIOCALLOWPROMISC, &cur_promisc) < 0) {
		perror("EIOCALLOWPROMISC");
		exit(1);
	    }
	    if (ioctl(fid, EIOCALLOWCOPYALL, &cur_copyall) < 0) {
		perror("EIOCALLOWCOPYALL");
		exit(1);
	    }
	    if (ioctl(fid, EIOCMAXBACKLOG, &cur_backlog) < 0) {
		perror("EIOCMAXBACKLOG");
		exit(1);
	    }
	    printf("%s: maximum backlog is %d;", ifr.ifr_name, cur_backlog);
	    if (cur_promisc)
		printf(" auto-promiscuous mode is enabled\n");
	    else
		printf(" auto-promiscuous mode is disabled\n");
	    if (cur_copyall)
		printf("\tauto-copyall mode is enabled\n");
	    else
		printf("\tauto-copyall mode is disabled\n");
	}
}

GetPFfid(ifname)
char *ifname;
{
	int fid;

	if ((fid = pfopen(ifname, 0)) < 0) {
	    return(-1);
	}
	return(fid);
}


/* XXX: pfopen() routine belongs in libc.a (after the maintenance release) */
/* Based on:
 * static char	*sccsid = "@(#)pfopen.c	5.1	(ULTRIX)	3/29/91";
 */

/*
 * Modification History:
 *
 *  2-Aug-89	Jeffrey Mogul/DECWRL
 *	Slight restructuring.
 *
 * 13-Jun-89	jsd
 *	Created this module for use with the Ethernet Packet Filter
 *
 */

#include <sys/socket.h>
#include <sys/time.h>	/* for timeval struct in pfilt.h */
#include <sys/file.h>
#include <sys/errno.h>
/*** already have these in pfconfig.c (above):
#include <net/if.h>
#include <net/pfilt.h>
***/
#include <stdio.h>

#define	PFPREFIX	"/dev/pf/pfilt"		/* prefix for device names */

/* define this for reverse compatibility */
/* #define	PFCOMPATNAME	"/dev/eneta0"	   old style name */

#define	PFMAXMINORS	256			/* 8-bit minor device field */

extern int errno;

/*
 * pfopen(ifname, flags): to support access to the Ethernet Packet Filter.
 * (using kernel option PACKETFILTER, pseudo-device packetfilter)
 *
 * ifname is a ptr to the Ethernet device name ("ln0", "ne0", "pf0", etc.)
 *	or NULL for default
 * flags are passed to the open() system call.
 *
 * return value:
 *	special device file descriptor on success
 *	-1 on failure with errno set to indicate the error
 *
 */
pfopen(ifname, flags)
char *ifname;			/* "ln0", "xna1", "pf0", etc. or NULL */
int flags;
{
	int i;			/* loop counter */
	int fd;			/* file descriptor */
	char tryname[128];	/* device name: "/dev/pf/pfiltnn" */
	static int setif();

	if (ifname && (ifname[0] == 0))
	    ifname = NULL;	/* change empty string to NULL string */

#ifdef	PFCOMPATNAME
	/* backwards compatible with Stanford-style packet filter */
	if ((fd = open(PFCOMPATNAME, flags, 0)) >= 0) {
	    return(setif(fd, ifname));
	}
#endif	/* PFCOMPATNAME */

	/* find next available device under the /dev/pf directory */
	for (i = 0; i < PFMAXMINORS; i++) {
		sprintf(tryname, "%s%d", PFPREFIX, i);
		fd = open(tryname, flags, 0);
		if (fd < 0) {
			switch (errno) {
			case EBUSY:	/* device in use */
				continue;	/* try the next entry */
			case ENOENT:	/* ran out of filenames */
			case ENXIO:	/* no more configured in kernel */
			default:	/* something else went wrong */
				return(-1);
			}
		}
		/* open succeeded, set the interface name */
		return(setif(fd, ifname));
	}
	return(-1);	/* didn't find an openable device */
}

static int setif(fd, ifname)
int fd;
char *ifname;
{
	if (ifname == NULL)	/* use default */
	    return(fd);

	if (ioctl(fd, EIOCSETIF, ifname) < 0) {
		close(fd);
		return(-1);
	}
	/* return the file descriptor */
	return(fd);
}
