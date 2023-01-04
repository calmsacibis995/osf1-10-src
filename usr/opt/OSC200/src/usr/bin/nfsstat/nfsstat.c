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
static char	*sccsid = "@(#)$RCSfile: nfsstat.c,v $ $Revision: 4.2.4.4 $ (DEC) $Date: 1993/01/06 07:28:27 $";
#endif 
/* 
 * nfsstat: Network File System statistics
 *
 */
#define KERNELBASE 0x80000000
#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#ifdef sun
#include <sun/ndio.h>
#else sun
#ifdef vax
	/* vax doesn't do nd yet*/
#else vax
/*	put your machine dependent code here */
#endif vax
#endif sun
#include <nlist.h>

struct nlist nl[] = {
#define	X_RCSTAT	0
	{ "_rcstat" },
#define	X_CLSTAT	1
	{ "_clstat" },
#define	X_RSSTAT	2
	{ "_rsstat" },
#define	X_SVSTAT	3
	{ "_svstat" },
#ifdef sun
#define	X_NDSTAT	4
	{ "_ndstat" },
#else sun
#ifdef vax
        /* vax doesn't do nd yet*/
#else vax 
    /*    put your machine dependent code here  */
#endif vax 
#endif sun
	"",
};

#define coreadj(x)	((int)x - KERNELBASE)

int kflag = 0;			/* set if using core instead of kmem */
int kmem;			/* file descriptor for /dev/kmem */
char *vmunix = "/vmunix";	/* name for /vmunix */
char *core = "/dev/kmem";	/* name for /dev/kmem */

/*
 * Statistics data structures.  The next four structures (eight variables) describe
 * the statistics the kernel tracks for us.  Each structure is replicated.  The
 * l_xxx structures are used with -i, and contain the data from the previous
 * pass.
 */

/*
 * client side rpc statistics
 */
struct {
        int     rccalls;
        int     rcbadcalls;
        int     rcretrans;
        int     rcbadxids;
        int     rctimeouts;
        int     rcwaits;
        int     rcnewcreds;
	int 	rcbadverfs;
	int 	rctimers;
} rcstat, l_rcstat;

#define RFS_NPROC       18
#define NFS_MAXHIST 41

/*
 * client side nfs statistics
 */
struct {
        u_int     nclsleeps;              /* client handle waits */
        u_int     nclgets;                /* client handle gets */
        u_int     ncalls;                 /* client requests */
        u_int     nbadcalls;              /* rpc failures */
        u_int     reqs[32];               /* count of each request */
#ifdef notdef
	struct {
	  int time,sos;
	  int data[NFS_MAXHIST];
	} histo[32];
#endif
} clstat, l_clstat;

/*
 * Server side rpc statistics
 */
struct {
        int     rscalls;
        int     rsbadcalls;
        int     rsnullrecv;
        int     rsbadlen;
        int     rsxdrcall;
} rsstat, l_rsstat;

/*
 * server side nfs statistics
 */
struct {
        int     ncalls;         /* number of calls received */
        int     nbadcalls;      /* calls that failed */
        int     reqs[32];       /* count for each request */
} svstat, l_svstat;

#ifdef sun
struct ndstat ndstat;
#else sun
#ifdef vax
        /* vax doesn't do nd yet*/
#else vax 
/*        put your machine dependent code here */

#endif vax 
#endif sun


main(argc, argv)
	char *argv[];
{
	extern char *optarg;
	extern int optind;
	char option;
	int	cflag = 0;		/* client stats */
	int	dflag = 0;		/* network disk stats */
	int	interval = 0;		/* Seconds between displays */
	int	nflag = 0;		/* nfs stats */
	int	rflag = 0;		/* rpc stats */
	int	sflag = 0;		/* server stats */
	int	zflag = 0;		/* zero stats after printing */


	while ((option = getopt(argc, argv, "ci:nrsz")) != EOF)
		switch((char)option) {
			case 'c':
				cflag++;
				break;
#ifdef sun
			case 'd':
				dflag++;
				break;
#else sun
#ifdef vax
        /* vax doesn't do nd yet*/
#else vax 
/*        put your machine dependent code here */
#endif vax 
#endif sun
			case 'i':
				interval = atoi(optarg);
				break;
			case 'n':
				nflag++;
				break;
			case 'r':
				rflag++;
				break;
			case 's':
				sflag++;
				break;
			case 'z':
				if (getuid()) {
					fprintf(stderr,
					    "Must be root for z flag\n");
					exit(1);
				}
				zflag++;
				break;
			default:
				usage();
			}
	argc -= optind;
	argv += optind;
	if (argc > 0) {
		vmunix = *argv;
		argv++, argc--;
		if (argc > 0) {
			core = *argv;
			kflag++;
		}
	}
	if (argc > 1 || interval < 0)
		usage();


	setup(zflag);
loop:
	if (interval)		/* Separate the passes */
		printf("----------------------------------------\n");
	getstats();
#ifdef sun
	if (dflag || (dflag + cflag + sflag + nflag + rflag) == 0) {
		d_print(zflag);
	}
#else sun
#ifdef vax
        /* vax doesn't do nd yet*/
#else vax 
       /* put your machine dependent code here  */
#endif vax 
#endif sun
	if (dflag && (sflag + cflag + rflag + nflag) == 0) {
		if (zflag) {
			putstats();
		}
		exit(0);
	}
	if (sflag || (!sflag && !cflag)) {
		if (rflag || (!rflag && !nflag)) {
			sr_print(zflag);
		}
		if (nflag || (!rflag && !nflag)) {
			sn_print(zflag);
		}
	}
	if (cflag || (!sflag && !cflag)) {
		if (rflag || (!rflag && !nflag)) {
			cr_print(zflag);
		}
		if (nflag || (!rflag && !nflag)) {
			cn_print(zflag);
		}
	}
	if (zflag) {
		putstats();
	}
	if (interval) {
		sleep(interval);
		goto loop;
	}
}

getstats()
{
	int size;
	if (klseek(kmem, (long)nl[X_RCSTAT].n_value, 0) == -1) {
		fprintf(stderr, "can't seek in kmem\n");
		exit(1);
	}
	if (read(kmem, &rcstat, sizeof rcstat) != sizeof rcstat) {
		fprintf(stderr, "can't read rcstat from kmem\n");
		exit(1);
	}

	if (klseek(kmem, (long)nl[X_CLSTAT].n_value, 0) == -1) {
		fprintf(stderr, "can't seek in kmem\n");
		exit(1);
	}
 	if (read(kmem, &clstat, sizeof(clstat)) != sizeof (clstat)) {
		fprintf(stderr, "can't read clstat from kmem\n");
		exit(1);
	}
	if (klseek(kmem, (long)nl[X_RSSTAT].n_value, 0) == -1) {
		fprintf(stderr, "can't seek in kmem\n");
		exit(1);
	}
 	if (read(kmem, &rsstat, sizeof(rsstat)) != sizeof (rsstat)) {
		fprintf(stderr, "can't read rsstat from kmem\n");
		exit(1);
	}

	if (klseek(kmem, (long)nl[X_SVSTAT].n_value, 0) == -1) {
		fprintf(stderr, "can't seek in kmem\n");
		exit(1);
	}
 	if (read(kmem, &svstat, sizeof(svstat)) != sizeof (svstat)) {
		fprintf(stderr, "can't read svstat from kmem\n");
		exit(1);
	}

#ifdef sun
	if (klseek(kmem, (long)nl[X_NDSTAT].n_value, 0) == -1) {
		fprintf(stderr, "can't seek in kmem\n");
		exit(1);
	}
 	if (read(kmem, &ndstat, sizeof(ndstat)) != sizeof (ndstat)) {
		fprintf(stderr, "can't read ndstat from kmem\n");
		exit(1);
	}
#else sun
#ifdef vax
        /* vax doesn't do nd yet*/
#else vax 
/*        put your machine dependent code here  */
#endif vax 
#endif sun
}

putstats()
{
	if (klseek(kmem, (long)nl[X_RCSTAT].n_value, 0) == -1) {
		fprintf(stderr, "can't seek in kmem\n");
		exit(1);
	}
	if (write(kmem, &rcstat, sizeof rcstat) != sizeof rcstat) {
		fprintf(stderr, "can't write rcstat to kmem\n");
		exit(1);
	}

	if (klseek(kmem, (long)nl[X_CLSTAT].n_value, 0) == -1) {
		fprintf(stderr, "can't seek in kmem\n");
		exit(1);
	}
 	if (write(kmem, &clstat, sizeof(clstat)) != sizeof (clstat)) {
		fprintf(stderr, "can't write clstat to kmem\n");
		exit(1);
	}

	if (klseek(kmem, (long)nl[X_RSSTAT].n_value, 0) == -1) {
		fprintf(stderr, "can't seek in kmem\n");
		exit(1);
	}
 	if (write(kmem, &rsstat, sizeof(rsstat)) != sizeof (rsstat)) {
		fprintf(stderr, "can't write rsstat to kmem\n");
		exit(1);
	}

	if (klseek(kmem, (long)nl[X_SVSTAT].n_value, 0) == -1) {
		fprintf(stderr, "can't seek in kmem\n");
		exit(1);
	}
 	if (write(kmem, &svstat, sizeof(svstat)) != sizeof (svstat)) {
		fprintf(stderr, "can't write svstat to kmem\n");
		exit(1);
	}

#ifdef sun
	if (klseek(kmem, (long)nl[X_NDSTAT].n_value, 0) == -1) {
		fprintf(stderr, "can't seek in kmem\n");
		exit(1);
	}
 	if (write(kmem, &ndstat, sizeof(ndstat)) != sizeof (ndstat)) {
		fprintf(stderr, "can't write ndstat to kmem\n");
		exit(1);
	}
#else sun
#ifdef vax
        /* vax doesn't do nd yet*/
#else vax 
/*        put your machine dependent code here */
#endif vax 
#endif sun
}

klseek(fd, loc, off)
	int fd;
	long loc;
	int off;
{

	if (kflag) {
		loc = coreadj(loc);
	}
	(void) lseek(fd, (long)loc, off);
}

setup(zflag)
	int zflag;
{
	register struct nlist *nlp;

	nlist(vmunix, nl);
	if (nl[0].n_value == 0) {
		fprintf (stderr, "Variables missing from namelist\n");
		exit (1);
	}
	if (kflag) {
		for (nlp = nl; nlp < &nl[sizeof (nl)/sizeof (nl[0])]; nlp++)
			nlp->n_value = coreadj(nlp->n_value);
	}
	if ((kmem = open(core, zflag ? 2 : 0)) < 0) {
		perror(core);
		exit(1);
	}
}

/*
 * Macro to compute difference between current count and last count in the
 * xxstat structures.
 */
#define DIFF(var) ((var) - (l_##var))

cr_print(zflag)
	int zflag;
{
	fprintf(stdout, "\nClient rpc:\n");
	fprintf(stdout,
	 "calls      badcalls   retrans    badxid     timeout    wait       newcred    badverfs   timers\n");
	fprintf(stdout,
	    "%-11d%-11d%-11d%-11d%-11d%-11d%-11d%-11d%-11d\n",
	    DIFF(rcstat.rccalls),
            DIFF(rcstat.rcbadcalls),
            DIFF(rcstat.rcretrans),
            DIFF(rcstat.rcbadxids),
            DIFF(rcstat.rctimeouts),
            DIFF(rcstat.rcwaits),
            DIFF(rcstat.rcnewcreds),
            DIFF(rcstat.rcbadverfs),
            DIFF(rcstat.rctimers));
	if (zflag) {
		bzero(&rcstat, sizeof rcstat);
	} else
		l_rcstat = rcstat;
}

sr_print(zflag)
	int zflag;
{
	fprintf(stdout, "\nServer rpc:\n");
	fprintf(stdout,
	    "calls      badcalls   nullrecv   badlen     xdrcall\n");
	fprintf(stdout,
	    "%-11d%-11d%-11d%-11d%-11d\n",
           DIFF(rsstat.rscalls),
           DIFF(rsstat.rsbadcalls),
           DIFF(rsstat.rsnullrecv),
           DIFF(rsstat.rsbadlen),
           DIFF(rsstat.rsxdrcall));
	if (zflag) {
		bzero(&rsstat, sizeof rsstat);
	} else
		l_rsstat = rsstat;
}

char *nfsstr[RFS_NPROC] = {
	"null",
	"getattr",
	"setattr",
	"root",
	"lookup",
	"readlink",
	"read",
	"wrcache",
	"write",
	"create",
	"remove",
	"rename",
	"link",
	"symlink",
	"mkdir",
	"rmdir",
	"readdir",
	"statfs" };

cn_print(zflag)
	int zflag;
{
	int i,j;

	fprintf(stdout, "\nClient nfs:\n");
	fprintf(stdout,
	    "calls      badcalls   nclget     nclsleep\n");
	fprintf(stdout,
	    "%-11d%-11d%-11d%-11d\n",
            DIFF(clstat.ncalls),
            DIFF(clstat.nbadcalls),
            DIFF(clstat.nclgets),
            DIFF(clstat.nclsleeps));
	req_print((int *)clstat.reqs, (int *)l_clstat.reqs, DIFF(clstat.ncalls));
#ifdef notdef
	fprintf(stdout,"\nOperation     Mean     Time");
	for (j=0;j<NFS_MAXHIST;j++) 
	  fprintf(stdout,"%3d ",10*j);

	fprintf(stdout,"\n");	

	for (i=0; i<RFS_NPROC;i++) {
	  if (clstat.reqs[i] != 0) {
	    double n,sos,time;
	    n=clstat.reqs[i];
	    sos=clstat.histo[i].sos;
	    time=clstat.histo[i].time;
	    if (n>1)
	      fprintf(stdout,"%-9s %8.2f %8.2f",nfsstr[i],
		      10.0*time/n,
		      time/100.0);
	    else
	      fprintf(stdout,"%-9s %8.2f %8.2f",nfsstr[i],
		      10.0*time/n,0.0);

	    for (j=0;j<NFS_MAXHIST;j++) {
	      fprintf(stdout,"%3d ",clstat.histo[i].data [j]);
	    }
	    fprintf(stdout,"\n");
	  }
	}
#endif

	if (zflag) {
		bzero(&clstat, sizeof clstat);
	} else
		l_clstat = clstat;
}

sn_print(zflag)
	int zflag;
{
	fprintf(stdout, "\nServer nfs:\n");
	fprintf(stdout, "calls      badcalls\n");
	fprintf(stdout, "%-11d%-11d\n", DIFF(svstat.ncalls), DIFF(svstat.nbadcalls));
	req_print((int *)svstat.reqs, (int *)l_svstat.reqs, DIFF(svstat.ncalls));
	if (zflag) {
		bzero(&svstat, sizeof svstat);
	} else
		l_svstat = svstat;
}

#ifdef sun
/* Note - needs work for -i, should anyone ever care. */
d_print(zflag)
	int zflag;
{
	fprintf(stdout, "\nNetwork Disk:\n");
	fprintf(stdout, "rcv %d  snd %d  retrans %d  (%.2f%%)\n",
	    ndstat.ns_rpacks,ndstat.ns_xpacks,ndstat.ns_rexmits,
	    (double)(ndstat.ns_rexmits*100)/ndstat.ns_xpacks);
	fprintf(stdout,
	    "notuser %d  noumatch %d  nobuf %d  lbusy %d  operrs %d\n",
	    ndstat.ns_notuser, ndstat.ns_noumatch, ndstat.ns_nobufs,
	    ndstat.ns_lbusy, ndstat.ns_lbusy);
	fprintf(stdout,
	    "rseq %d  wseq %d  badreq %d  stimo %d  utimo %d  iseq %d\n",
	    ndstat.ns_rseq, ndstat.ns_wseq, ndstat.ns_badreq, ndstat.ns_stimo,
	    ndstat.ns_utimo, ndstat.ns_iseq);
	if (zflag) {
		bzero(&ndstat, sizeof ndstat);
	}
}
#else sun
#ifdef vax
        /* vax doesn't do nd yet*/
#else vax 
/*        put your machine dependent code here */
#endif vax 
#endif sun


req_print(req, l_req, tot)
	int	*req, *l_req;
	int	tot;
{
	int	i, j;
	char	fixlen[128];

	for (i=0; i<=RFS_NPROC / 7; i++) {
		for (j=i*7; j<min(i*7+7, RFS_NPROC); j++) {
			fprintf(stdout, "%-11s", nfsstr[j]);
		}
		fprintf(stdout, "\n");
		for (j=i*7; j<min(i*7+7, RFS_NPROC); j++) {
			if (tot) {
				sprintf(fixlen,
				    "%d %2d%% ", DIFF(req[j]), (DIFF(req[j])*100)/tot);
			} else {
				sprintf(fixlen, "%d 0%% ", DIFF(req[j]));
			}
			fprintf(stdout, "%-11s", fixlen);
		}
		fprintf(stdout, "\n");
	}
}

usage()
{
#ifdef sun
	fprintf(stderr, "nfsstat [-cdnrsz] [vmunix] [core]\n");
#else sun
#ifdef vax
	fprintf(stderr, "nfsstat [-cnrsz] [vmunix] [core]\n");
#else vax
	fprintf(stderr, "nfsstat [-cnrsz] [-i#] [vmunix] [core]\n");
#endif vax
#endif sun
	exit(1);
}

min(a,b)
	int a,b;
{
	if (a<b) {
		return(a);
	}
	return(b);
}

