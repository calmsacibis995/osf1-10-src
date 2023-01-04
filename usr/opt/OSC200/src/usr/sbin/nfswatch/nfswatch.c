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
static char *rcsid = "@(#)$RCSfile: nfswatch.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/12/21 23:35:43 $";
#endif
/* Based on:
 * /home/harbor/davy/system/nfswatch/RCS/nfswatch.c,v 4.0 1993/03/01 19:59:00 davy Exp $";
 */

#include "os.h"

/*
 * nfswatch - NFS server packet monitoring program.
 *
 * David A. Curry				Jeffrey C. Mogul
 * Purdue University				Digital Equipment Corporation
 * Engineering Computer Network			Western Research Laboratory
 * 1285 Electrical Engineering Building		250 University Avenue
 * West Lafayette, IN 47907-1285		Palo Alto, CA 94301
 * davy@ecn.purdue.edu				mogul@decwrl.dec.com
 *
 * log: nfswatch.c,v
 * Revision 4.0  1993/03/01  19:59:00  davy
 * NFSWATCH Version 4.0.
 *
 * Revision 3.13  1993/02/24  17:44:45  davy
 * Added -auth mode, changes to -proc mode, -map option, -server option.
 *
 * Revision 3.12  1993/01/20  14:52:30  davy
 * Added -T maxtime option.
 *
 * Revision 3.11  1993/01/16  19:08:59  davy
 * Corrected Jeff's address.
 *
 * Revision 3.10  1993/01/15  22:09:20  davy
 * Fixed for Sun FDDI using the NIT.
 *
 * Revision 3.9  1993/01/15  19:33:39  davy
 * Miscellaneous cleanups.
 *
 * Revision 3.8  1993/01/15  15:43:36  davy
 * Assorted changes for porting to Solaris 2.x/SVR4.
 *
 * Revision 3.7  1993/01/14  15:51:16  davy
 * Added FDDI code and device type calculation to NIT and SNOOP.  The FDDI
 * stuff almost definitely won't work without modification on the SNOOP
 * side; it still needs to be tested on the NIT side.
 *
 * Revision 3.6  1993/01/13  21:24:19  davy
 * Assorted changes for porting to IRIX.
 *
 * Revision 3.5  1993/01/13  20:18:17  davy
 * Put in OS-specific define scheme, and merged in Tim Hudson's code for
 * SGI systems (as yet untested).
 *
 * Revision 3.4  1993/01/13  18:59:30  davy
 * Changed sigvec calls to signal calls, for portability to other os versions.
 *
 * Revision 3.3  1993/01/13  15:12:05  davy
 * Added background mode.
 *
 * Revision 3.2  1992/07/24  18:47:57  mogul
 * Added FDDI support
 *
 * Revision 3.1  1991/01/23  16:56:19  mogul
 * Black magic
 *
 * Revision 3.1  1991/01/23  16:56:19  mogul
 * Black magic
 *
 * Revision 3.0  91/01/23  08:23:11  davy
 * NFSWATCH Version 3.0.
 * 
 * Revision 1.5  91/01/04  16:05:15  davy
 * Updated version number.
 * 
 * Revision 1.4  91/01/04  15:54:29  davy
 * New features from Jeff Mogul.
 * 
 * Revision 1.3  90/12/04  08:06:41  davy
 * Changed version number to 2.1.
 * 
 * Revision 1.2  90/08/17  15:47:29  davy
 * NFSWATCH Version 2.0.
 * 
 * Revision 1.1  88/11/29  11:20:40  davy
 * NFSWATCH Release 1.0
 * 
 */
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <net/if.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>

#ifdef USE_DLPI
#include <sys/stream.h>
#include <sys/stropts.h>

#if defined(SVR4) && !defined(SUNOS5)
char	*devices[] = {
	"emd0", "emd1", "emd2", "emd3", "emd4",
	0
};
#endif

#ifdef SUNOS5
#include <sys/bufmod.h>

char	*devices[] = {
	"le0", "le1", "le2", "le3", "le4",
	"ie0", "ie1", "ie2", "ie3", "ie4",
	"fddi0", "fddi1", "fddi2", "fddi3", "fddi4",
	0
};
#endif
#endif /* USE_DLPI */

#ifdef USE_NIT
#include <net/nit_if.h>
#include <net/nit_buf.h>

char	*devices[] = {
	"le0", "le1", "le2", "le3", "le4",
	"ie0", "ie1", "ie2", "ie3", "ie4",
	"fddi0", "fddi1", "fddi2", "fddi3", "fddi4",
	0
};

#endif /* USE_NIT */

#ifdef USE_PFILT
#include <net/pfilt.h>

char	*devices[] = {
	"pf0", "pf1", "pf2", "pf3", "pf4", "pf5",
	"pf6", "pf7", "pf8", "pf9",
	0
};
#endif /* USE_PFILT */

#ifdef USE_SNOOP
#include <net/soioctl.h>
#include <net/raw.h>
#include <netinet/if_ether.h>

#define ETHERHDRPAD		RAW_HDRPAD(sizeof(struct ether_header))

char	*devices[] = {
	"et0", "et1", "et2", "et3", "et4",
	"ec0", "ec1", "ec2", "ec3", "ec4",
	"fxp0", "fxp1", "fxp2", "fxp3", "fxp4",
	"enp0", "enp1", "enp2", "enp3", "enp4",
	"ipg0", "ipg1", "ipg2", "ipg3", "ipg4",
	0
};

struct etherpacket {
	struct snoopheader	snoop;
	char			pad[ETHERHDRPAD];
	struct ether_header	ether;
	char			data[ETHERMTU];
};
#endif /* USE_SNOOP */

#include "nfswatch.h"

char		*pname;				/* program name		*/

FILE		*logfp;				/* log file pointer	*/

Counter		pkt_total = 0;			/* total packets seen	*/
Counter		pkt_drops = 0;			/* total packets dropped*/
Counter		int_pkt_total = 0;		/* packets this interval*/
Counter		int_pkt_drops = 0;		/* dropped this interval*/
Counter		dst_pkt_total = 0;		/* total pkts to host	*/
Counter		int_dst_pkt_total = 0;		/* pkts to host this int*/

int		if_fd[MAXINTERFACES];		/* LAN device file desc	*/
int		if_dlt[MAXINTERFACES];		/* LAN data link type	*/

int		bgflag = 0;			/* "-bg" specified	*/
int		srcflag = 0;			/* "-src" specified	*/
int		dstflag = 0;			/* "-dst" specified	*/
int		allflag = 0;			/* "-all" specified	*/
int		allintf = 0;			/* "-allif" specified	*/
int		logging = 0;			/* 1 when logging on	*/
int		learnfs = 0;			/* learn other servers	*/
int		do_update = 0;			/* time to update screen*/
int		showwhich = 0;			/* show filesys or files*/
int		serverflag = 0;			/* "-server" specified	*/
int		cycletime = CYCLETIME;		/* update cycle time	*/
int		totaltime = 0;			/* total run time	*/
int		truncation = 200;		/* pkt trunc len - magic*/
int		sortbyusage = 0;		/* sort by usage counts	*/
int		nnfscounters = 0;		/* # of NFS counters	*/
int		nfilecounters = 0;		/* # of file counters	*/
int		nauthcounters = 0;		/* # of auth counters	*/
int		nclientcounters = 0;		/* # of client counters */
int		screen_inited = 0;		/* 1 when in curses	*/

struct timeval	starttime;			/* time we started	*/

int		ninterfaces;			/* number of interfaces	*/

ipaddrt		thisdst = 0;			/* cached IP dst of pkt	*/
ipaddrt		srcaddrs[MAXHOSTADDR];		/* src host net addrs	*/
ipaddrt		dstaddrs[MAXHOSTADDR];		/* dst host net addrs	*/
ipaddrt		serveraddrs[MAXHOSTADDR];	/* server host net addrs*/

char		myhost[MAXHOSTNAMELEN];		/* local host name	*/
char		srchost[MAXHOSTNAMELEN];	/* source host name	*/
char		dsthost[MAXHOSTNAMELEN];	/* destination host name*/
char		serverhost[MAXHOSTNAMELEN];	/* server host name	*/

char		*prompt = PROMPT;		/* prompt string	*/
char		*filelist = NULL;		/* list of files	*/
char		*logfile = LOGFILE;		/* log file name	*/
char		*mapfile = NULL;		/* map file name	*/
char		*snapshotfile = SNAPSHOTFILE;	/* snapshot file name	*/

NFSCounter	nfs_counters[MAXEXPORT];	/* NFS request counters	*/
FileCounter	fil_counters[MAXEXPORT];	/* file request counters*/
PacketCounter	pkt_counters[PKT_NCOUNTERS];	/* packet counters	*/
ProcCounter	prc_counters[MAXNFSPROC+2];	/* procedure counters	*/
				/* extra space simplifies sort_prc_counters */
int		prc_countmap[MAXNFSPROC];	/* allows sorting	*/
ClientCounter	clnt_counters[MAXCLIENTS];	/* per-client counters	*/
AuthCounter	auth_counters[MAXAUTHS];	/* per-auth counters	*/

extern void finish();

#ifdef ultrix
void
fpe_warn()
{
	fprintf(stderr, "nfswatch: mystery bug encountered.\n");
	finish(-1);
}
#endif

main(argc, argv)
int argc;
char **argv;
{
	register int i;
	char *device = NULL;
	extern void nfswatch();

	pname = *argv;

	/*
	 * Get our host name.  The default destination
	 * host is the one we're running on.
	 */
	if (gethostname(myhost, sizeof(myhost)) < 0) {
		error("gethostname");
		finish(-1);
	}

	(void) strcpy(dsthost, myhost);

	/*
	 * Process arguments.
	 */
	while (--argc) {
		if (**++argv != '-')
			usage();

		/*
		 * Set destination host.
		 */
		if (!strcmp(*argv, "-dst")) {
			if (--argc <= 0)
				usage();

			(void) strcpy(dsthost, *++argv);
			dstflag++;
			continue;
		}

		/*
		 * Set source host.
		 */
		if (!strcmp(*argv, "-src")) {
			if (--argc <= 0)
				usage();

			(void) strcpy(srchost, *++argv);
			srcflag++;
			continue;
		}

		/*
		 * Set server host.
		 */
		if (!strcmp(*argv, "-server")) {
			if (--argc <= 0)
				usage();

			(void) strcpy(serverhost, *++argv);
			serverflag++;
			continue;
		}

		/*
		 * Device to use.
		 */
		if (!strcmp(*argv, "-dev")) {
			if (--argc <= 0)
				usage();

			device = *++argv;
			continue;
		}

		/*
		 * Log file name.
		 */
		if (!strcmp(*argv, "-lf")) {
			if (--argc <= 0)
				usage();

			logfile = *++argv;
			continue;
		}

		/*
		 * Snapshot file name.
		 */
		if (!strcmp(*argv, "-sf")) {
			if (--argc <= 0)
				usage();

			snapshotfile = *++argv;
			continue;
		}

		/*
		 * List of files.
		 */
		if (!strcmp(*argv, "-f")) {
			if (--argc <= 0)
				usage();

			if (showwhich == 0)
				showwhich = SHOWINDVFILES;

			filelist = *++argv;
			continue;
		}

		/*
		 * Set map file.
		 */
		if (!strcmp(*argv, "-map")) {
			if (--argc <= 0)
				usage();

			mapfile = *++argv;
			continue;
		}
		
		/*
		 * Set total run time.
		 */
		if (!strcmp(*argv, "-T")) {
			if (--argc <= 0)
				usage();
			
			totaltime = atoi(*++argv);
			continue;
		}

		/*
		 * Change cycle time.
		 */
		if (!strcmp(*argv, "-t")) {
			if (--argc <= 0)
				usage();

			cycletime = atoi(*++argv);
			continue;
		}

		/*
		 * Show RPC authentication.
		 */
		if (!strcmp(*argv, "-auth")) {
			showwhich = SHOWAUTH;
			continue;
		}

		/*
		 * Show file systems.
		 */
		if (!strcmp(*argv, "-fs")) {
			showwhich = SHOWFILESYSTEM;
			continue;
		}

		/*
		 * Show individual files.
		 */
		if (!strcmp(*argv, "-if")) {
			showwhich = SHOWINDVFILES;
			continue;
		}

		/*
		 * Show NFS procedures
		 */
		if (!strcmp(*argv, "-procs")) {
			showwhich = SHOWNFSPROCS;
			continue;
		}

		/*
		 * Show NFS clients
		 */
		if (!strcmp(*argv, "-clients")) {
			showwhich = SHOWCLIENTS;
			continue;
		}

		/*
		 * Turn on logging.
		 */
		if (!strcmp(*argv, "-l")) {
			logging++;
			continue;
		}

		/*
		 * Run in background mode.
		 */
		if (!strcmp(*argv, "-bg")) {
			logging++;
			bgflag++;
			continue;
		}

		/*
		 * Watch all traffic.
		 */
		if (!strcmp(*argv, "-all")) {
			allflag++;
			continue;
		}

		/*
		 * Use all interfaces.
		 */
		if (!strcmp(*argv, "-allif")) {
			allintf++;
			continue;
		}

		/*
		 * Sort file systems by usage, not name.
		 */
		if (!strcmp(*argv, "-usage")) {
			sortbyusage++;
			continue;
		}

		usage();
	}

	/*
	 * Check what we're showing.
	 */
	switch (showwhich) {
	case 0:			/* default */
		showwhich = SHOWFILESYSTEM;
		break;
	case SHOWINDVFILES:
		if (filelist == NULL) {
			(void) fprintf(stderr, "%s: must specify file list with -fi.\n", pname);
			finish(-1);
		}

		break;
	}

	/*
	 * Trap signals so we can clean up.
	 */
	(void) signal(SIGINT, finish);
	(void) signal(SIGQUIT, finish);
	(void) signal(SIGTERM, finish);

#ifdef sgi
	/*
	 * Kludge to prevent coredumps when the optimizer's on?
	 */
	(void) signal(SIGFPE, SIG_IGN);
#endif

#ifdef ultrix
	(void) signal(SIGFPE, fpe_warn);
#endif

#ifdef USE_DLPI
	/*
	 * Set up the data link interface provider  right away,
	 * since we probably need super-user permission.
	 */
	if (allintf) {
	    ninterfaces = 0;
	    for (i=0; devices[i] != NULL; i++) {
		if_fd[ninterfaces] = setup_dlpi_dev(&devices[i]);

		if (if_fd[ninterfaces] >= 0) {
		    if_dlt[ninterfaces] = dlpi_devtype(if_fd[ninterfaces]);
		    ninterfaces++;
		}
	    }
	}
	else {
	    if_fd[0] = setup_dlpi_dev(&device);

	    if (if_fd[0] < 0) {
		error(device);
		finish(-1);
	    }

	    if_dlt[0] = dlpi_devtype(if_fd[0]);
	    ninterfaces = 1;
	}
#endif /* USE_DLPI */

#ifdef USE_NIT
	/*
	 * Set up the network interface tap right away,
	 * since we probably need super-user permission.
	 */
	if (allintf) {
	    ninterfaces = 0;
	    for (i=0; devices[i] != NULL; i++) {
		if_fd[ninterfaces] = setup_nit_dev(&devices[i]);

		if (if_fd[ninterfaces] >= 0) {
		    if_dlt[ninterfaces] = nit_devtype(devices[i]);
		    ninterfaces++;
		}
	    }
	}
	else {
	    if_fd[0] = setup_nit_dev(&device);

	    if (if_fd[0] < 0) {
		error(device);
		finish(-1);
	    }

	    if_dlt[0] = nit_devtype(device);
	    ninterfaces = 1;
	}
#endif /* USE_NIT */

#ifdef USE_PFILT
	/*
	 * Set up the packet filter interface now,
	 * although we don't need super-user permission.
	 */
	if (allintf) {
	    ninterfaces = 0;
	    for (i=0; devices[i] != NULL; i++) {
		if_fd[ninterfaces] = setup_pfilt_dev(&devices[i]);

		if (if_fd[ninterfaces] >= 0) {
		    if_dlt[ninterfaces] = pfilt_devtype(if_fd[ninterfaces]);
		    ninterfaces++;
		}
	    }
	}
	else {
	    if_fd[0] = setup_pfilt_dev(&device);

	    if (if_fd[0] < 0) {
		error(device);
		finish(-1);
	    }

	    if_dlt[0] = pfilt_devtype(if_fd[0]);
	    ninterfaces = 1;
	}
#endif /* USE_PFILT */

#ifdef USE_SNOOP
	/*
	 * Set up the snoop interface right away,
	 * since we probably need super-user permission.
	 */
	if (allintf) {
	    ninterfaces = 0;
	    for (i=0; devices[i] != NULL; i++) {
		if_fd[ninterfaces] = setup_snoop_dev(&devices[i]);

		if (if_fd[ninterfaces] >= 0) {
		    if_dlt[ninterfaces] = snoop_devtype(devices[i]);
		    ninterfaces++;
		}
	    }
	}
	else {
	    if_fd[0] = setup_snoop_dev(&device);

	    if (if_fd[0] < 0) {
		error(device);
		finish(-1);
	    }

	    if_dlt[0] = snoop_devtype(device);
	    ninterfaces = 1;
	}
#endif /* USE_SNOOP */

	if (ninterfaces < 1) {
		fprintf(stderr, "%s: no valid interfaces.\n", pname);
		finish(-1);
	}

	/*
	 * Now lose super-user permission, since we
	 * don't need it for anything else.
	 */
#ifdef SVR4
	(void) setuid(getuid());
	(void) seteuid(getuid());
#else
	(void) setreuid(getuid(), getuid());
#endif

	/*
	 * Look up the network addresses of the source and
	 * destination hosts.
	 */
	get_net_addrs();

	/*
	 * Tell the user what's going on.
	 */
	(void) printf("NFSWATCH Version %s\n", VERSION);

	if (serverflag) {
		(void) printf("Watch packets to/from %s on ", serverhost);
	}
	else {
		(void) printf("Watch packets from %s to %s on ",
			      (srcflag ? srchost : "all hosts"), dsthost);
	}

	if (allintf)
		(void) printf("all interfaces;\n");
	else {
		(void) printf("%s ", dlt_name(if_dlt[0]));
		(void) printf("interface %s;\n", device);
	}

	(void) printf("log to \"%s\" (logging %s);\n", logfile,
		(logging ? "on" : "off"));

	if (bgflag) {
		(void) printf("cycle time %d seconds;\n", cycletime);
		(void) printf("running as a daemon in the background...");
	}
	else {
		(void) printf("snapshots to \"%s\";\n", snapshotfile);
		(void) printf("cycle time %d seconds...", cycletime);
	}

	(void) fflush(stdout);

	/*
	 * No more informational output, so fork and exit if in
	 * background mode.
	 */
	if (bgflag) {
		int pid;

		if ((pid = fork()) < 0) {
			error("fork");
			finish(-1);
		}

		if (pid != 0) {
			printf("pid = %d\n", pid);
			exit(0);
		}
	}

	/*
	 * Set up a pseudo RPC server.
	 */
	setup_rpcxdr();

	/*
	 * Set up the screen.
	 */
	if (!bgflag)
		setup_screen(device);

	/*
	 * Set up the packet counters.  This must be done after
	 * setup_screen because they use the LINES variable.
	 */
	setup_pkt_counters();
	setup_nfs_counters();
	setup_proc_counters();

	if (filelist)
		setup_fil_counters();

	if (mapfile)
		setup_map_file();

	/*
	 * Now label the screen.
	 */
	if (!bgflag)
		label_screen();

	/*
	 * Open log file if logging is on.
	 */
	if (logging) {
		if ((logfp = fopen(logfile, "a")) == NULL) {
			error(logfile);
			finish(-1);
		}

		(void) fprintf(logfp, "#\n# startlog\n#\n");
		(void) fprintf(logfp, "# NFSwatch log file\n");
		(void) fprintf(logfp, "#    Packets from: %s\n",
			(srcflag ? srchost : "all hosts"));
		(void) fprintf(logfp, "#    Packets to:   %s\n#\n",
			dsthost);
	}

	/*
	 * Go watch packets.  Never returns.
	 */
	nfswatch();
}

/*
 * nfswatch - main packet reading loop.
 */
void
nfswatch()
{
	int i, cc;
	char *buf;
	char *malloc();
	fd_set readfds;
	struct timeval tv;
	extern void wakeup();
	struct itimerval itv;
	register char *bp, *cp, *bufstop;
#ifdef USE_DLPI
	int tdrops[MAXINTERFACES];
	struct strbuf strdata;
	struct sb_hdr *hdrp;
	int flags;
#endif
#ifdef USE_NIT
	int tdrops[MAXINTERFACES];
	struct nit_iftime *tstamp;
	struct nit_bufhdr *hdrp;
	struct nit_ifdrops *ndp;
#endif
#ifdef USE_PFILT
	struct enstamp stamp;
	int datalen;
#endif
#ifdef USE_SNOOP
	int tdrops[MAXINTERFACES];
	struct etherpacket ep;
	struct rawstats rs;
#endif

#ifdef USE_DLPI
	/*
	 * Allocate a buffer so it's properly aligned for
	 * casting to structure types.
	 */
	if ((buf = malloc(DLPI_CHUNKSIZE)) == NULL) {
		(void) fprintf(stderr, "%s: out of memory.\n", pname);
		finish(-1);
	}

	strdata.len = 0;
	strdata.buf = buf;
	strdata.maxlen = DLPI_CHUNKSIZE;
#endif

#ifdef USE_NIT
	/*
	 * Allocate a buffer so it's properly aligned for
	 * casting to structure types.
	 */
	if ((buf = malloc(NIT_CHUNKSIZE)) == NULL) {
		(void) fprintf(stderr, "%s: out of memory.\n", pname);
		finish(-1);
	}
#endif

#ifdef USE_PFILT
	/*
	 * Allocate a buffer so it's properly aligned for
	 * casting to structure types.
	 */
	if ((buf = malloc(PFILT_CHUNKSIZE)) == NULL) {
		(void) fprintf(stderr, "%s: out of memory.\n", pname);
		finish(-1);
	}
#endif

	/*
	 * Set up the alarm handler.
	 */
	(void) signal(SIGALRM, wakeup);

	/*
	 * Set up the alarm clock.
	 */
	(void) bzero((char *) &itv, sizeof(struct itimerval));

	itv.it_interval.tv_sec = cycletime;
	itv.it_interval.tv_usec = 0;

	itv.it_value = itv.it_interval;

	(void) setitimer(ITIMER_REAL, &itv, (struct itimerval *) 0);

	/*
	 * Set the start time.
 	 */
	(void) gettimeofday(&starttime, (struct timezone *) 0);

#ifdef USE_DLPI
	/*
	 * Flush the read queue of any packets that accumulated
	 * during setup time.
	 */
	for (i=0; i < ninterfaces; i++) {
		flush_dlpi(if_fd[i]);
		tdrops[i] = 0;
	}

	for (;;) {
		FD_ZERO(&readfds);

		for (i=0; i < ninterfaces; i++)
			FD_SET(if_fd[i], &readfds);

		/*
		 * See which nets have packets to read.
		 */
		cc = select(NFDBITS, &readfds, (fd_set *) 0, (fd_set *) 0, 0);

		if ((cc < 0) && (errno != EINTR)) {
			error("select");
			finish(-1);
		}
		if (cc == 0) {
			continue;
		}
		
		/*
		 * For each interface...
		 */
		for (i=0; i < ninterfaces; i++) {
			/*
			 * Nothing to read.
			 */
			if (!FD_ISSET(if_fd[i], &readfds))
				continue;

			/*
			 * Now read packets from the dlpi device.
			 */
			flags = 0;
			strdata.len = 0;
			if (getmsg(if_fd[i], NULL, &strdata, &flags) != 0)
				continue;

			bufstop = buf + strdata.len;
			bp = buf;

#ifdef SUNOS5
			/*
			 * Loop through the chunk, extracting packets.
			 */
			while (bp < bufstop) {
				cp = bp;

				/*
				 * Get the nit header.
				 */
				hdrp = (struct sb_hdr *) cp;
				cp += sizeof(struct sb_hdr);

				int_pkt_drops += hdrp->sbh_drops - tdrops[i];
				pkt_drops += hdrp->sbh_drops - tdrops[i];
				tdrops[i] = hdrp->sbh_drops;

				/*
				 * Filter the packet.
				 */
				if (if_dlt[i] == DLT_FDDI) {
					pkt_filter_fddi(cp, hdrp->sbh_msglen,
							&hdrp->sbh_timestamp);
				}
				else {
					pkt_filter_ether(cp, hdrp->sbh_msglen,
							 &hdrp->sbh_timestamp);
				}

				/*
				 * Skip over this packet.
				 */
				bp += hdrp->sbh_totlen;
			}
#else /* SUNOS5 */
			/*
			 * It's just a packet, no buffering.
			 */
			if (strdata.len) {
				/*
				 * Since we're not on SunOS5, that means we
				 * don't have DLIOCRAW, so we don't have the
				 * packet frame header.  So, we need to
				 * bypass that level of filtering.
				 */
				pkt_dispatch(bp, strdata.len, 0,
					     htons(DLPI_DEFAULTSAP), 0);
			}
#endif /* SUNOS5 */
		}
#endif /* USE_DLPI */

#ifdef USE_NIT
	/*
	 * Flush the read queue of any packets that accumulated
	 * during setup time.
	 */
	for (i=0; i < ninterfaces; i++) {
		flush_nit(if_fd[i]);
		tdrops[i] = 0;
	}

	for (;;) {
		FD_ZERO(&readfds);

		for (i=0; i < ninterfaces; i++)
			FD_SET(if_fd[i], &readfds);

		/*
		 * See which nets have packets to read.
		 */
		cc = select(NFDBITS, &readfds, (fd_set *) 0, (fd_set *) 0, 0);

		if ((cc < 0) && (errno != EINTR)) {
			error("select");
			finish(-1);
		}
		if (cc == 0) {
			continue;
		}
		
		/*
		 * For each interface...
		 */
		for (i=0; i < ninterfaces; i++) {
			/*
			 * Nothing to read.
			 */
			if (!FD_ISSET(if_fd[i], &readfds))
				continue;

			/*
			 * Now read packets from the nit device.
			 */
			if ((cc = read(if_fd[i], buf, NIT_CHUNKSIZE)) <= 0)
				continue;

			bufstop = buf + cc;
			bp = buf;

			/*
			 * Loop through the chunk, extracting packets.
			 */
			while (bp < bufstop) {
				cp = bp;

				/*
				 * Get the nit header.
				 */
				hdrp = (struct nit_bufhdr *) cp;
				cp += sizeof(struct nit_bufhdr);

				/*
				 * Get the time stamp.
				 */
				tstamp = (struct nit_iftime *) cp;
				cp += sizeof(struct nit_iftime);

				/*
				 * Get the number of dropped packets.
				 */
				ndp = (struct nit_ifdrops *) cp;
				cp += sizeof(struct nit_ifdrops);

				int_pkt_drops += ndp->nh_drops - tdrops[i];
				pkt_drops += ndp->nh_drops - tdrops[i];
				tdrops[i] = ndp->nh_drops;

				/*
				 * Filter the packet.
				 */
#ifdef notdef
				/*
				 * This is how it *should* be.  But the NIT
				 * device rips off the FDDI packet header
				 * and the 802.2 LLC header, and replaces
				 * them with an ethernet packet header.
				 */
				if (if_dlt[i] == DLT_FDDI) {
					pkt_filter_fddi(cp, hdrp->nhb_msglen,
							&tstamp->nh_timestamp);
				}
				else {
					pkt_filter_ether(cp, hdrp->nhb_msglen,
							 &tstamp->nh_timestamp);
				}
#else
				/*
				 * So... we just do this instead for both
				 * ethernet and FDDI.  Strange but true.
				 */
				pkt_filter_ether(cp, hdrp->nhb_msglen,
						 &tstamp->nh_timestamp);
#endif

				/*
				 * Skip over this packet.
				 */
				bp += hdrp->nhb_totlen;
			}
		}
#endif /* USE_NIT */

#ifdef USE_PFILT
	/*
	 * Flush the read queue of any packets that accumulated
	 * during setup time.
	 */
	for (i=0; i < ninterfaces; i++)
		flush_pfilt(if_fd[i]);

	for (;;) {
		FD_ZERO(&readfds);

		for (i=0; i < ninterfaces; i++)
			FD_SET(if_fd[i], &readfds);

		/*
		 * See which interfaces have any packets to read.
		 */
		cc = select(NFDBITS, &readfds, (fd_set *) 0, (fd_set *) 0, 0);

		if ((cc < 0) && (errno != EINTR)) {
			error("select");
			finish(-1);
		}
		if (cc == 0) {
			continue;
		}

		/*
		 * Now read packets from the packet filter device.
		 */
		for (i=0; i < ninterfaces; i++) {
			if (!FD_ISSET(if_fd[i], &readfds))
				continue;
			
			if ((cc = read(if_fd[i], buf, PFILT_CHUNKSIZE)) < 0) {
				lseek(if_fd[i], 0L, 0);

				/*
				 * Might have read MAXINT bytes.  Try again.
				 */
				if ((cc = read(if_fd[i], buf, PFILT_CHUNKSIZE)) < 0) {
					error("pfilt read");
					finish(-1);
				}
			}
		
			bp = buf;

			/*
			 * Loop through buffer, extracting packets.
			 */
			while (cc > 0) {
				/*
				 * Avoid alignment issues.
				 */
				(void) bcopy(bp, &stamp, sizeof(stamp));

				/*
				 * Treat entire buffer as garbage.
				 */
				if (stamp.ens_stamplen != sizeof(stamp))
					break;

				/*
				 * Get the number of dropped packets.
				 */
				int_pkt_drops += stamp.ens_dropped;
				pkt_drops += stamp.ens_dropped;

				/*
				 * Filter the packet.
				 */
				datalen = stamp.ens_count;

				if (datalen > truncation)
					datalen = truncation;

				if (if_dlt[i] == DLT_FDDI) {
				    /* Weird Ultrix padding */
				    pkt_filter_fddi(&(bp[sizeof(stamp) + 3]),
						    datalen,
						    &stamp.ens_tstamp);
				}
				else
				    pkt_filter_ether(&(bp[sizeof(stamp)]),
						     datalen,
						     &stamp.ens_tstamp);

				/*
				 * Skip over this packet.
				 */
				if (cc == (datalen + sizeof(stamp)))
					break;

				datalen = ENALIGN(datalen);
				datalen += sizeof(stamp);
				cc -= datalen;
				bp += datalen;
			}
		}

#endif /* USE_PFILT */

#ifdef USE_SNOOP
	/*
	 * Flush the read queue of any packets that accumulated
	 * during setup time.
	 */
	for (i=0; i < ninterfaces; i++) {
		flush_snoop(if_fd[i]);
		tdrops[i] = 0;
	}

	/*
	 * Now read packets from the snooper.
	 */
	for (;;) {
		int dropped;

		FD_ZERO(&readfds);

		for (i=0; i < ninterfaces; i++)
			FD_SET(if_fd[i], &readfds);

		/*
		 * See which nets have packets to read.
		 */
		cc = select(NFDBITS, &readfds, (fd_set *) 0, (fd_set *) 0, 0);

		if ((cc < 0) && (errno != EINTR)) {
			error("select");
			finish(-1);
		}
		if (cc == 0) {
			continue;
		}
		
		/*
		 * For each interface...
		 */
		for (i=0; i < ninterfaces; i++) {
			/*
			 * Nothing to read.
			 */
			if (!FD_ISSET(if_fd[i], &readfds))
				continue;

			/*
			 * Now read packets from the nit device.
			 */
			if ((cc = read(if_fd[i], &ep, sizeof(ep))) <= 0)
				continue;

			if (do_update) {
				/*
				 * Get the number of dropped packets.
				 */
				if (ioctl(if_fd[i], SIOCRAWSTATS, &rs) == 0) {
					dropped = rs.rs_snoop.ss_ifdrops +
						  rs.rs_snoop.ss_sbdrops;

					int_pkt_drops += dropped - tdrops[i];
					pkt_drops += dropped - tdrops[i];
					tdrops[i] = dropped;
				}
			}

			/*
			 * Filter the packet.
			 */
			if (if_dlt[i] == DLT_FDDI) {
				/*
				 * This probably won't work.
				 */
				pkt_filter_fddi(&ep.ether,
						ep.snoop.snoop_packetlen,
						&ep.snoop.snoop_timestamp);
			}
			else {
				pkt_filter_ether(&ep.ether,
						 ep.snoop.snoop_packetlen,
						 &ep.snoop.snoop_timestamp);
			}
		}
#endif /* USE_SNOOP */

		tv.tv_sec = 0;
		tv.tv_usec = 0;
		FD_ZERO(&readfds);
		FD_SET(0, &readfds);

		/*
		 * See if a command has been typed.
		 */
		if (!bgflag) {
			cc = select(NFDBITS, &readfds, (fd_set *) 0,
				    (fd_set *) 0, &tv);

			if ((cc > 0) && FD_ISSET(0, &readfds))
				command();
		}
	}
}
