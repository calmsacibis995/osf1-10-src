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
static char	*sccsid = "@(#)$RCSfile: mountd.c,v $ $Revision: 4.2.11.6 $ (DEC) $Date: 1993/10/05 21:07:39 $";
#endif 
/*
 */
/*
 * OSF/1 Release 1.0
 */

/*
 *	Portions of this software have been licensed to 
 *	Digital Equipment Company, Maynard, MA.
 *	Copyright (c) 1987 Sun Microsystems, Inc.  ALL RIGHTS RESERVED.
 */
/***********************************************************************
 *
 *		Modification History
 *
 * 06/04/91 lebel
 *	Ported over the ULTRIX 4.2 mountd.  Incorporated OSF mountd 
 *	differences.
 *
 ***********************************************************************/

/* NFS mount server */
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/mount.h>
#include <sys/errno.h>
#include <sys/signal.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <netdb.h>
#include <rpc/rpc.h>
#include <sys/mount.h>
#include <sys/time.h>
#include <rpcsvc/ypclnt.h>
#include <rpcsvc/yp_prot.h>
#include <rpcsvc/mount.h>
#include <arpa/nameser.h>
#include <resolv.h>

#define	EXPORTS	"/etc/exports"
#define RMTAB	"/var/adm/mountdtab"
#define _PATH_MOUNTDPID "/var/run/mountd.pid"
#define MAXRMTABLINELEN         (MAXPATHLEN + MAXHOSTNAMELEN + 2)
#define	MAXLINE	2048
#define MAX_LEVELS  10
#define MAXADDRS 8

extern int errno;

int expand_ng();
int innetgr_cache();
int check_ng();
int getkeys();
int mnt();
char *exmalloc();
struct exports *newex();
struct groups *newgroup();
char *goto_char();
char *realpath();
void filladdrs();
void getaddr();
void free_groups();

/*
 * mountd's version of a "struct mountlist". It is the same except
 * for the added ml_pos field.
 */
struct mountdlist {
/* same as XDR mountlist */
        char *ml_name;
        char *ml_path;
        struct mountdlist *ml_nxt;
/* private to mountd */
        long ml_pos;            /* position of mount entry in RMTAB */
};

struct ng {             /* netgroup cache */
        int ng_stamp;
	char *ng_name;
        struct hlist *ng_hosts;
        struct ng *next_ng;
        struct ng *prev_ng;
};

struct hlist {
        char *hname;
        struct hlist *next_host;
};

/*
 * Mount Reply Cache - save replies, and check the cache to catch
 * retransmitted requests.
 */

/*
 *  RPC server duplicate transaction cache flag values
 */
#define DUP_BUSY        0x1     /* transaction in progress */
#define DUP_DONE        0x2     /* transaction was completed */
#define DUP_FAIL        0x4     /* transaction failed */

struct dupreq {
        struct in_addr  rc_addr;        /* client address */
        u_short         rc_flags;       /* DUP_BUSY, DUP_DONE, DUP_FAIL */
        dev_t           rc_dev;         /* device */
        ino_t			rc_ino;         /* inode number */
        uint_t          rc_gen;         /* generation number */
        struct dupreq   *rc_next;       /* linked list of all entries */
        struct dupreq   *rc_chain;      /* hash chain */
	char 		*rc_ancname;    /* name of requested path's most
					   closely related exported ancestor */
};

#define KEYHASH(addr,dev,ino,gen)       ((((addr) ^ (dev)) ^ (ino) ^ (gen)) % drhashszminus1)
#define KEYTOLIST(addr,dev,ino,gen)     ((struct dupreq *)(drhashtbl[KEYHASH(addr,dev,ino,gen)]))
#define REQTOLIST(dr)   KEYTOLIST((dr)->rc_addr.s_addr,(dr)->rc_dev,(dr)->rc_ino, (dr)->rc_gen)
 
/* routine to compare dup cache entries */
#define NOTDUP(dr, addr,dev,ino,gen) (dr->rc_addr.s_addr != addr || \
                            dr->rc_dev != dev || \
                            dr->rc_ino != ino || \
                            dr->rc_gen != gen)

/*
 * dupcache_max is the number of cached items.  It is set
 * based on "system size". It should be large enough to hold
 * transaction history long enough so that a given entry is still
 * around for a few retransmissions of that transaction.
 */
#define MINDUPREQS      1024
#define MAXDUPREQS      4096
struct dupreq **drhashtbl; /* array of heads of hash lists */
int drhashsz;              /* number of hash lists */
int drhashszminus1;        /* hash modulus */
int dupcache_max;
struct dupreq *dupreqcache, *drmru;
struct dupreq *dupcache_check();

struct ng *nglist = NULL;  /* head of netgroup's cache */
char *ng_names[100];
int num_ngs;
static struct mountdlist *mountlist;
int rmtab_load();
long rmtab_insert();
char pathscratch[MAXPATHLEN+1];/* Used to build local path with -m option */
 
char mydomain[MAXDOMNAMELEN];
char *pgmname;
char exportfile[MAXPATHLEN];
struct exports *exports = NULL;
struct exports *flatexports = NULL;
int nfs_portmon = 0;
int ipaddr_check = 0;
int domain_check = 0;
int subdomain_check = 0;
int root_only = 1;
void touch_exports(), send_umntall();

struct timeval rmtab_written, now;
struct timezone tz;
/*
 * resync rmtab no more often than at 30 minute intervals
 * sole purpose is to get rid of commented out (unmounted) entries
 */
int   rmtab_sync=1800;

/*
 * Cached netgroups are assumed correct for at least 15 minutes
 */
int ngtimeout = 900;
 

/*
 * Multi-include exportfile support
 */
struct exportfile_entry {
	struct exportfile_entry *next;   /* ptr to next entry */
	char fn[MAXPATHLEN];             /* full path of filename */
	struct stat statb;               /* stat block */
};

struct exportfile_entry *exportfile_q =  /* exportfile queue head ptr */
	(struct exportfile_entry *)NULL;

int add_exportfile();
void free_exportfile();
void print_exportfile();


/*
 * NFS to mount mapping structures 
 */

struct remotelocalmap_entry {
	struct remotelocalmap_entry *next;  /* ptr to next entry */
	char remotepath[MAXPATHLEN];           /* NFS pathname */
	char localpath[MAXPATHLEN];            /* MNT pathname */
};

struct remotelocalmap_entry *remotelocalmap_q =     /* remotelocalmap queue head ptr */
	(struct remotelocalmap_entry *)NULL;

struct remotelocalmap_entry *remotelocalmap_tail =  /* remotelocalmap queue tail ptr */
	(struct remotelocalmap_entry *)NULL;

int add_remotelocalmap();
void free_remotelocalmap();
void print_remotelocalmap();
char * get_localpath();
char * get_remotepath();

main(argc, argv)
char	*argv[];
{
	SVCXPRT *transp;
	char *strchr ();
	int fd; /* open fd of rmtab */
	extern char *optarg;
	extern int optind;
	char ch;

	pgmname = argv[0];

	/*
	 * must be superuser to run 
	 */
	if (geteuid() != 0){
		(void) fprintf(stderr, "%s:  must be super user\n", pgmname);
		(void) fflush(stderr);
		exit(1);
	}
	while ((ch = getopt(argc, argv, "idsn")) != EOF) {
		switch (ch) {
		case 'i':
			ipaddr_check++;
			break;
		case 'd':
			ipaddr_check++;
			domain_check++;
			break;
		case 's':
			ipaddr_check++;
			subdomain_check++;
			break;
		case 'n':
			root_only = 0;
			break;
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;
	if (argc == 1) {
		strncpy(exportfile, argv[0], MAXPATHLEN-1);
		exportfile[MAXPATHLEN-1] = '\0';
	} else
		strcpy(exportfile, EXPORTS); 
	add_exportfile(exportfile);
	touch_exports();

	if (openlog("mountd", LOG_PID, LOG_DAEMON) < 0)
		fprintf(stderr, "mountd: openlog failed\n");
	syslog(LOG_ERR, "startup");
	if(getdomainname(mydomain, sizeof(mydomain)) < 0){      /* jaa */
		syslog(LOG_ERR, "getdomainname: %m");
		exit(1);
	}

	/*
	 * Read rmtab and exports files, build netgroups cache
	 */
	fd = rmtab_load();
	if (mydomain[0] != NULL)
        	build_ngcache();
#ifdef DEBUG
        print_ngcache();
#endif DEBUG
	set_exports();
#ifdef DEBUG
	(void) fprintf(stderr, "*** finished loading export list ***\n");
#endif DEBUG
	gettimeofday(&rmtab_written, &tz);

	/*
	 * Remove this chunk of code if we ever run under inetd.
         * (Make sure rmtab - fd - does not get closed)
	 * Also remove the mtd_abort() routine if running under inetd.
	 */
	{
		int s, t;
		struct sockaddr_in addr;
		int len = sizeof(struct sockaddr_in);
		int pid;

#ifndef DEBUG
		/*
		 * This used to close all fd's 0-20 (except for the one
		 * associated with rmtab).  This would end up closing the
		 * socket descriptor associated with syslog (obtained from
		 * the call to openlog).  As a result, none of the syslog
		 * messages after this point would be logged.  If the call
		 * to openlog() was coded properly it would return the fd
		 * associated with syslog; however since it doesn't there's
		 * no good way of knowing which fd is for syslog.  So only
		 * close 0,1,2 to be safe.
		 */
		for (t = 0; t < 3; t++)
			if (t != fd)
				(void) close(t);
	 	open("/", 0);
	 	dup2(0, 1);
	 	dup2(0, 2);
#endif
		if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
			syslog(LOG_ERR, "socket: %m");
			exit(1);
		}
		bzero(&addr, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = INADDR_ANY;
		addr.sin_port = 0;
		if (bind(s, &addr, sizeof(addr)) < 0) {
			syslog(LOG_ERR, "bind: %m");
			exit(1);
		}
		if (getsockname(s, &addr, &len) != 0) {
			syslog(LOG_ERR, "getsockname: %m");
			(void) close(s);
			exit(1);
		}
		pmap_unset(MOUNTPROG, MOUNTVERS);
		/* 
		 * register with portmapper if not started from inetd
		 */
		pmap_set(MOUNTPROG, MOUNTVERS, IPPROTO_UDP,
		    ntohs(addr.sin_port));
		if (dup2(s, 0) < 0) {
			syslog(LOG_ERR, "dup2: %m");
			exit(1);
		}
#ifndef DEBUG
		pid = fork();
		if (pid < 0) {
			syslog(LOG_ERR, "Cannot fork: %m");
			exit(1);
		}
		if (pid != 0)
			exit(0);
#endif
	}
	/* End chunk to remove if running under inetd. */

	(void)setsid();
#ifndef DEBUG
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
#endif
	signal(SIGHUP, touch_exports);
	signal(SIGTERM, send_umntall);
	{ FILE *pidfile = fopen(_PATH_MOUNTDPID, "w");
	  if (pidfile != NULL) {
		fprintf(pidfile, "%d\n", getpid());
		fclose(pidfile);
	  }
	}

	/*
	 * Create UDP service
	 */
	if ((transp = svcudp_create(0)) == NULL) {
		syslog(LOG_ERR, "couldn't create UDP transport");
		exit(1);
	}
	if (!svc_register(transp, MOUNTPROG, MOUNTVERS, mnt, 0)) {
		syslog(LOG_ERR, "couldn't register MOUNTPROG");
		exit(1);
	}

	/*
	 * Create TCP service
	 */
	if ((transp = svctcp_create(RPC_ANYSOCK, 0, 0)) == NULL) {
		syslog(LOG_ERR, "couldn't create TCP transport");
		exit(1);
	}
	if (!svc_register(transp, MOUNTPROG, MOUNTVERS, mnt, IPPROTO_TCP)) {
		syslog(LOG_ERR, "couldn't register MOUNTPROG");
		exit(1);
	}

	dupcache_init();  /* dup req cache */
	/*
	 * Start serving
	 */

	while(1) {
		svc_run();
		syslog(LOG_ERR, "Error: svc_run shouldn't have returned");
		mtd_abort();
	}
}

/*
 * Server procedure switch routine
 */
mnt(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
#ifdef DEBUG
	char *machine;

	machine = ((struct authunix_parms *) rqstp->rq_clntcred)->aup_machname;
#endif DEBUG
	switch(rqstp->rq_proc) {
		case NULLPROC:
			if (!svc_sendreply(transp, xdr_void, NULL)) {
				syslog(LOG_ERR, "couldn't reply to NULL rpc call");
				mtd_abort();
			}
			return;
		case MOUNTPROC_MNT:
#ifdef DEBUG
			(void) fprintf(stderr, "about to do a mount from %s\n", machine);
#endif
			set_exports();
			(void) mtd_mount(rqstp, transp);
			return;
		case MOUNTPROC_DUMP:
#ifdef DEBUG
			(void) fprintf(stderr, "about to do a dump from %s\n", machine);
#endif
			if (!svc_sendreply(transp, xdr_mountlist, &mountlist)) {
				syslog(LOG_ERR, "couldn't reply to DUMP rpc call");
				mtd_abort();
			}
			return;
		case MOUNTPROC_UMNT:
#ifdef DEBUG
			(void) fprintf(stderr, "about to do an unmount from %s\n", machine);
#endif
			(void) mtd_umount(rqstp, transp);
			return;
		case MOUNTPROC_UMNTALL:
#ifdef DEBUG
			(void) fprintf(stderr, "about to do an unmountall from %s\n", machine);
#endif
			mtd_umountall(rqstp, transp);
			return;
		case MOUNTPROC_EXPORT:
		case MOUNTPROC_EXPORTALL:
#ifdef DEBUG
			(void) fprintf(stderr, "about to do an export from %s\n", machine);
#endif
			set_exports();
			mtd_export(transp);
			return;
		default:
			svcerr_noproc(transp);
			return;
	}
}

struct hostent *
getclientsname(rqstp,transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
	struct sockaddr_in actual;
	struct hostent *hp;
	static struct hostent h;
	static char hostbuf_save[MAXHOSTNAMELEN+1];
	int hostlen;
	static struct in_addr prev;
	static char *null_alias;
	char *cp;
	char *strchr ();
	struct authunix_parms *ucr;
	uid_t uid = -2;

	if (rqstp->rq_cred.oa_flavor != AUTH_UNIX) {
		return(NULL);
	}
	actual = *svc_getcaller(transp);
	if (nfs_portmon) {
		if (ntohs(actual.sin_port) >= IPPORT_RESERVED) {
			return(NULL);
		}
	}
	/*
	 * Check root-only user authorization
	 */
	ucr = (struct authunix_parms *)rqstp->rq_clntcred;
	uid = ucr->aup_uid;
	if (uid != 0 && root_only)
		return(NULL);

        /*
         * Don't use the unix credentials to get the machine name,
         * instead use the source IP address.  Use cached hostent
         * if previous request was from same client.
         */
        if (bcmp(&actual.sin_addr, &prev, sizeof(struct in_addr)) == 0) {
#ifdef DEBUG
                (void)fprintf(stderr, "Found addr in hostent cache! \n");
#endif DEBUG
                return (&h);
	}

        hp = gethostbyaddr((char *) &actual.sin_addr, sizeof(actual.sin_addr),
                           AF_INET);
        if (hp == NULL) {
		if (ipaddr_check) {
                	svcerr_auth(transp, AUTH_BADCRED);
                	return(NULL);
		} /* else dummy one up */
		h.h_name = (char *)inet_ntoa(actual.sin_addr);
#ifdef DEBUG
                (void)fprintf(stderr, "Dummy hostent name %s\n", h.h_name);
#endif DEBUG
                h.h_aliases = &null_alias;
                h.h_addrtype = AF_INET;
                h.h_length = sizeof (uint_t);
                hp = &h;
        } else {
		/*
		 * Stash the contents of the host name into a stable
		 * location.  This is necessary because a subsequent call to
		 * gethostby* will end up altering the contents of what is 
		 * pointed to by "hp". 
		 */
		hostlen = strlen(hp->h_name);
		if (hostlen > MAXHOSTNAMELEN)		/* paranoia check */
			hostlen = MAXHOSTNAMELEN;
		strncpy(hostbuf_save, hp->h_name, hostlen);
		hostbuf_save[hostlen] = '\0';
        	bcopy(hp, &h, sizeof(struct hostent));
		h.h_name = hostbuf_save;
	}

        prev = actual.sin_addr;

#ifdef DEBUG
	(void)fprintf(stderr, "getclientsname %s\n", hp->h_name);
#endif DEBUG

	/*
	 *	If domain_check or subdomain_check is set and
	 *	if BIND enabled, check the requester's domain spec
	 *	to be sure it is local.  Check if:
	 *		1) domain spec matches that of local host or
	 *		2) host name is unqualified (local)
	 */
	if ((domain_check || subdomain_check) && (bindup()) != NULL) {
		if ((cp = strchr (hp->h_name, '.')) != NULL) {

			/* qualified name */
			if (strcasecmp (cp+1, _res.defdname) != NULL) {
				/*
				 * Not in local domain, check  
				 * if in subdomain
				 */
				if (subdomain_check) {
#ifdef DEBUG
					(void) fprintf (stderr, "mountd: client, %s not local domain member, check if %s is a subdomain of %s\n", hp->h_name, cp+1, _res.defdname);
#endif DEBUG
					if ((strfind (cp+1, _res.defdname)) == NULL) {
						syslog(LOG_ERR, "warning: (u)mount attempt from client %s, not a local subdomain member", hp->h_name);
						return (NULL);
					}
				}
				else {
					syslog(LOG_ERR, "warning: (u)mount attempt from client %s, not a local domain member", hp->h_name);
					return(NULL);
				}
			}
		}
	}
#ifdef DEBUG
	(void) fprintf(stderr, "%s: end ipaddr_check successful\n", pgmname);
#endif DEBUG
	return(hp);
}

/*
 * Check mount requests, add to mounted list if ok
 */
mtd_mount(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
	fhandle_t fh;
	struct fhstatus fhs;
	char *remotepath;
	char *path;
	int fd, anc_fd;
	struct mountdlist *ml;
	struct stat statbuf;
	struct exports *ex, *ex2, *ancestors[MAX_LEVELS];
	struct exports *saveex = NULL;
	int found=0, success=0, bestposs=1, index, i;
	int first = 1;
	struct dupreq *dr;
	char *anc_name;
	char rpath[MAXPATHLEN];
	struct hostent *client;
	struct sockaddr_in addr;

        if ((client = getclientsname(rqstp,transp))== NULL) {
                svcerr_weakauth(transp);
                return;
        }
	remotepath = NULL;
	if (!svc_getargs(transp, xdr_path, &remotepath)) {
		svcerr_decode(transp);
		mtd_abort();
		return;
	}
	path = get_localpath(remotepath);
#ifdef DEBUG
	(void) fprintf(stderr, "remotepath is %s\n",remotepath);
	(void) fprintf(stderr, "local path is %s\n",path);
#endif

	if ((fd = open(path, O_RDONLY, 0)) < 0) {
		fhs.fhs_status = errno;
		perror("mountd: open");
		goto done;
	}
	if (fstat(fd, &statbuf) < 0) {
		fhs.fhs_status = errno;
		perror("mountd: stat");
		(void) close(fd);
		goto done;
	}

        /*
         * Now, before we spend a lot of time looking through the
         * exports list, lets check to see if we have already handed out
         * a filehandle to this requestor. . .
         */
	addr = *svc_getcaller(transp);
        if (dr = dupcache_check(addr.sin_addr, statbuf.st_dev, statbuf.st_ino,
                        statbuf.st_gen)) {
		if ((anc_fd = open(dr->rc_ancname, O_RDONLY, 0)) < 0) {
			fhs.fhs_status = errno;
			perror("mountd: open");
			goto done;
		}
		if (getfh(fd, &fh, anc_fd) < 0) {
			fhs.fhs_status = errno;
			syslog(LOG_ERR, "getfh: %m");
			(void)close(anc_fd);
			goto done;
		}
		fhs.fhs_status = 0;
                fhs.fhs_fh = fh;
		(void)close(anc_fd);
                goto done;
        }

	/*
	 * Expand path into its canonical form, resolving symbolic link
	 * and ../ components. 
	 */
	if (realpath(path, rpath) == NULL) {
#ifdef DEBUG
		perror("mountd");
		(void)fprintf(stderr, "Realpath failed on %s\n", path);
#endif DEBUG
		fhs.fhs_status = EACCES;
		goto done;
	}
	/*
	 * Search exports list for a directory with an entry with the
	 * same dev_t number.  Search down that dev's list looking for 
	 * an exact match.  If none found, look for entries that are
	 * an ancestor of the requested path.  Check the access on the
	 * closest match.  If access is not allowed, check the next
	 * closest, ... until either no more exported ancestors exist
	 * or an ancestor is found with allowable access for the client.
	 * The export options used will be those on the ancestor found.
	 */
loop_search:
	for (ex = exports; ex != NULL; ex = ex->ex_devptr) {
#ifdef DEBUG
		(void) fprintf(stderr, "checking %o for %o\n", ex->ex_dev, statbuf.st_dev);
#endif
		if (ex->ex_dev == statbuf.st_dev) {
			for (ex2=ex; ex2 != NULL; ex2 = ex2->ex_next) {
#ifdef DEBUG
				(void) fprintf(stderr, "checking %s with %s for exact match dir\n", ex2->ex_name, rpath);
#endif
				if (strcmp(ex2->ex_name, rpath) == NULL) {
#ifdef DEBUG
					(void) fprintf(stderr, "got exact match dir\n");
#endif
					if (ex2->ex_groups == NULL) {
						success = 1;
						break;
					}
					else {
						if ((check_groups(ex2->ex_groups, client->h_name, client->h_aliases))!=NULL) {
							success = 1;
							break;
						}
						else {
							/*
							 * Exact match found but requester
							 * is not an allowable host.  Break
							 * to search for an ancestor match.
							 */
							saveex = ex;
							break;
						}
					}
				}
			}
			if (success) {
				if ((anc_fd = open(ex2->ex_name, O_RDONLY, 0)) < 0) {
					fhs.fhs_status = errno;
					perror("mountd: open");
					goto done;
				}
				anc_name = ex2->ex_name;
				goto hit;
			}
			if (saveex != NULL)
				break;
			if (ex2 == NULL) {
				/*
				 * Exact matches failed.  
				 * Break to best match search.
				 */
#ifdef DEBUG
				(void) fprintf(stderr,"exact match for %s failed\n", rpath);
#endif
				saveex = ex;
				break;
			}
		}
	}
	if (!saveex) {
		if (first) {
			first = 0;
			touch_exports();
			set_exports();
			goto loop_search;
		}
		else {
#ifdef DEBUG
			(void) fprintf(stderr, "%s: filesystem %s not found\n", pgmname, rpath);
#endif DEBUG
			fhs.fhs_status = EACCES;
			goto done;
		}
	}

	/*
	 * Using pointer to directory exports record with same
	 * dev_t number, try to find best match.
	 */
	ex = saveex;
	if (ex->ex_dev == statbuf.st_dev) {
		/*
		 *  initialize ancestors
		 */
		for (i=0; i< MAX_LEVELS; i++)
			ancestors[i] = NULL;
		for (ex2 = ex; ex2 != NULL; ex2 = ex2->ex_next) {
			if ((strcmp(ex2->ex_name, rpath) != NULL) && (index = path_check(ex2->ex_name, rpath)) > bestposs) {
#ifdef DEBUG
				(void) fprintf(stderr, "ancestor: %s found, level:%d is > bestposs\n", ex2->ex_name, index);
#endif
				found = 1;
				ancestors[index-2] = ex2;
			}
			else if (index == bestposs) {
#ifdef DEBUG
				(void) fprintf(stderr, "best match %s found, checking access\n", ex2->ex_name);
#endif
				if (ex2->ex_groups == NULL) {
					success = 1;
					break;
				}
				else {
					if ((check_groups(ex2->ex_groups, client->h_name, client->h_aliases)) != NULL) {
						success = 1;
						break;
					}
					else
						++bestposs;
				}
			}
		}
		if (success) {
			if ((anc_fd = open(ex2->ex_name, O_RDONLY, 0)) < 0) {
				fhs.fhs_status = errno;
				perror("mountd: open");
				goto done;
			}
			anc_name = ex2->ex_name;
			goto hit;
		}
		if (found) {
			for (i= ((bestposs==1)? 0:bestposs-2); i< MAX_LEVELS-1; i++)
				if (ancestors[i] != NULL) {
#ifdef DEBUG
					(void) fprintf(stderr, "Checking ancestor: %s for allowable access\n", ancestors[i]->ex_name); 
#endif DEBUG
					if (ancestors[i]->ex_groups == NULL) {
						success = 1;
						break;
					}
					else {
						if ((check_groups(ancestors[i]->ex_groups, client->h_name, client->h_aliases)) != NULL) {
							success = 1;
							break;
						}
					}
				}
			if (success) {
				if ((anc_fd = open(ancestors[i]->ex_name, O_RDONLY, 0)) < 0) {
					fhs.fhs_status = errno;
					perror("mountd: open");
					goto done;
				}
				anc_name = ancestors[i]->ex_name;
				goto hit;
			}
		}
	}
	fhs.fhs_status = EACCES;
	goto done;
  hit:
	if (getfh(fd, &fh, anc_fd) < 0) {
		fhs.fhs_status = errno;
		syslog(LOG_ERR, "getfh: %m");
		(void)close(anc_fd);
		goto done;
	}
	fhs.fhs_status = 0;
	fhs.fhs_fh = fh;
	(void)close(anc_fd);
        /*
         * Now that we have a "New" mount request, lets tuck it into the
         * dupreq cache so that if the client retransmits, we can reply
	 * with the filehandle immediately.
         */
        dupcache_enter(addr.sin_addr, statbuf.st_dev, statbuf.st_ino,
                    statbuf.st_gen, anc_name);
done:
#ifdef DEBUG
        (void) fprintf(stderr, "*** ng cache and export list ***\n");
        print_ngcache();
        print_exports(exports);
#endif DEBUG
	(void) close(fd);
	errno = 0;

	if (!svc_sendreply(transp, xdr_fhstatus, &fhs)) {
		syslog(LOG_ERR, "couldn't reply to MOUNT rpc call");
		mtd_abort();
	} 
	else if (fhs.fhs_status == 0) {
		for (ml = mountlist; ml != NULL; ml = ml->ml_nxt) {
			if (strcmp(ml->ml_path, remotepath) == 0 &&
		    	    strcmp(ml->ml_name, client->h_name) == 0)
				break;
		}
		if (ml == NULL) {
			ml = (struct mountdlist *)exmalloc(sizeof(struct mountdlist));
			ml->ml_path = (char *)exmalloc(strlen(remotepath) + 1);
			(void) strcpy(ml->ml_path, remotepath);
			ml->ml_name = (char *)exmalloc(strlen(client->h_name) + 1);
			(void) strcpy(ml->ml_name, client->h_name);
			ml->ml_nxt = mountlist;
			mountlist = ml;
			ml->ml_pos = rmtab_insert(client->h_name, remotepath);
		}
	}
	svc_freeargs(transp, xdr_path, &remotepath);
}

/*
 * Check if machine is in groups structure.  If not, call YP.
 */
check_groups(headgl, mach, aliases)
	struct groups *headgl;
	char *mach;
	char **aliases;
{
	struct groups *gl;
	size_t length;
	char **aliasptr;

	/* 
	 * first check for exact match on machine name
	 */
	for (gl = headgl; gl != NULL; gl = gl->g_next){ 
#ifdef DEBUG
		(void) fprintf(stderr, "checking machines: %s for %s\n", gl->g_name, mach);
#endif
		if (strcasecmp(gl->g_name, mach) == 0) {
			gl->g_hbit = 1;
			return(TRUE);
		}
		else if (((length = (size_t) local_hostname_length(mach)) != NULL) && 
			  (strlen(gl->g_name) == length) &&
			  (strncasecmp(gl->g_name, mach, length) == 0)) {
			gl->g_hbit = 1;
			return(TRUE);
		}
		for (aliasptr = aliases; *aliasptr != NULL; aliasptr++) {
			if (strcasecmp(gl->g_name, *aliasptr) == 0)
				return(TRUE);
		}
	}
	/*
	 * now check netgroups
	 */
	for (gl = headgl; gl != NULL; gl = gl->g_next){ 
#ifdef DEBUG
		(void) fprintf(stderr, "checking innetgr: %s for %s\n", gl->g_name, mach);
#endif
		if (gl->g_hbit)
			continue;
		if (innetgr_cache (gl->g_name, mach, &gl->g_hbit, aliases)) {
			return(TRUE);
		}
	}
	return(FALSE);
}

/*
 * Check the pathnames of the export name and the desired mount
 * pathname.
 */
path_check(list_name, name)
 	char *list_name, *name;
{
 	char *ch1, *ch2;
 	char *ch11, *ch22;
 	char s1, s2;
 	int done, level=0;
 
#ifdef DEBUG
	(void) fprintf(stderr, "path_check %s with %s\n", list_name, name);
#endif
 	ch1 = list_name;
 	ch2 = name;
 	if (!strcmp(list_name, "/")) {
		/*
		 * just return the number of levels in "name"
		 */
		for (; ch2 && *ch2!='\0'; ch2++)
			if (*ch2 == '/') level++;
		if (*--ch2 == '/') level--; /* trailing slash */
		return(level);
	}
 	done = 0;
 	while (!done) {
 		if ((ch1 = goto_char(ch1)) != NULL)
 			ch11 = index(ch1, '/');
 		if ((ch2 = goto_char(ch2)) != NULL)
 			ch22 = index(ch2, '/');
 		if (ch11 && *ch11 != '\0') {
 			s1 = *ch11;
 			*ch11 = '\0';
 			ch11++;
 		}
 		if (ch22 && *ch22 != '\0') {
 			s2 = *ch22;
 			*ch22 = '\0';
 			ch22++;
 		}
 		if ((ch1 && ch2) && !strcmp(ch1, ch2)) {
 			if (ch11 && *ch11 != '\0') {
 				*--ch11 = s1;
 				ch11++;
 			}
 			if (ch22 && *ch22 != '\0') {
 				*--ch22 = s2;
 				ch22++;
 			}
 			ch1 = ch11;
 			ch2 = ch22;
 			if ((ch11 && *ch11 == '\0') || !ch11) {
				/*
				 * count number of slashes remaining in ancestor
				 */
				if (ch2) {
					for (--ch2; ch2 && *ch2!='\0'; ch2++)
						if (*ch2 == '/') level++;
					if (*--ch2 == '/')
						level--;
				}
 				return(level);
			}
 			if ((ch11 && *ch11 != '\0') && ((ch22 && *ch22 == '\0') || !ch22))
 				done = 1;
 		}
 		else {
 			if (ch11 && *ch11 != '\0')
 				*--ch11 = s1;
 			if (ch22 && *ch22 != '\0')
 				*--ch22 = s2;
 			done = 1;
 		}
 	}
	return(level);
}

/*
 * Skip over slashes (/) and go to first character
 */
char *
goto_char(ch)
 	char *ch;
{
 	for (;ch && *ch == '/'; ch++)
 		;
	if (ch)
 		return(ch);
	else
		return(NULL);
}
 
/*
 * Remove an entry from mounted list
 */
mtd_umount(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
	char *path;
	struct mountdlist *ml, *oldml;
	struct hostent *client;
	long pos = -1;

        if ((client = getclientsname(rqstp,transp))== NULL) {
                svcerr_weakauth(transp);
                return;
        }
	path = NULL;
	if (!svc_getargs(transp, xdr_path, &path)) {
		svcerr_decode(transp);
		mtd_abort();
		return;
	}
#ifdef DEBUG
	(void) fprintf(stderr, "umounting %s for %s\n", path, client->h_name);
#endif
	oldml = mountlist;
	for (ml = mountlist; ml != NULL;
	    oldml = ml, ml = ml->ml_nxt) {
		if (strcmp(ml->ml_path, path) == 0 &&
		    strcmp(ml->ml_name, client->h_name) == 0) {
			if (ml == mountlist)
				mountlist = ml->ml_nxt;
			else
				oldml->ml_nxt = ml->ml_nxt;
#ifdef DEBUG
			(void) fprintf(stderr, "freeing %s\n", path);
#endif
			pos = ml->ml_pos;
			free(ml->ml_path);
			free(ml->ml_name);
			free((char *)ml);
			break;
		    }
	}
	if (!svc_sendreply(transp,xdr_void, NULL)) {
		syslog(LOG_ERR, "couldn't reply to UMOUNT rpc call");
		mtd_abort();
	} else {
		gettimeofday(&now, &tz);
                if ((now.tv_sec - rmtab_written.tv_sec) > rmtab_sync) {
                        dumptofile();
                        rmtab_written = now;
                }
                else if (pos >= 0) {
                        rmtab_delete(pos);
                }
	}
	svc_freeargs(transp, xdr_path, &path);
}

/*
 * Remove all entries for one machine from mounted list
 */
mtd_umountall(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
	char *machine;
	struct mountdlist *ml, *oldml;

	if (!svc_getargs(transp, xdr_void, NULL)) {
		svcerr_decode(transp);
		return;
	}
	/*
	 * We assume that this call is asynchronous and made via the 
	 * portmapper callit routine.  Therefore return control immediately.
	 * The error causes the portmapper to remain silent, as opposed to
	 * every machine on the net blasting the requester with a response.
	 */
	svcerr_systemerr(transp);
        if (rqstp->rq_cred.oa_flavor == AUTH_UNIX) {
                machine =
                   ((struct authunix_parms *)rqstp->rq_clntcred)->aup_machname;
        }
        else
                return;

	oldml = mountlist;
	for (ml = mountlist; ml != NULL; ml = ml->ml_nxt) {
		if (strncmp(ml->ml_name, machine, sizeof(machine)) == 0) {
#ifdef DEBUG
			(void) fprintf(stderr, "got a hit\n");
#endif
			if (ml == mountlist) {
				mountlist = ml->ml_nxt;
				oldml = mountlist;
			}
			else
				oldml->ml_nxt = ml->ml_nxt;
			rmtab_delete(ml->ml_pos);
			free(ml->ml_path);
			free(ml->ml_name);
			free((char *)ml);
		}
		else
			oldml = ml;
	}
	svc_freeargs(transp, xdr_void, NULL);
}

FILE *f;
/*
 * Save current mount state info so we
 * can attempt to recover in case of a crash.
 */
dumptofile()
{
	static char *t1 = "/var/adm/zzXXXXXX";
	static char *t2 = "/var/adm/zzXXXXXX";
	FILE *fp;
	struct mountdlist *ml;
	char *mktemp();
	int mf;
	
	(void) fclose(f);
	(void) strcpy(t2, t1);
	t2 = mktemp(t2);
	if ((mf = creat(t2, 0644)) < 0) {
		syslog(LOG_ERR, "creat: %m, cannot dump mountlist to %s", RMTAB);
		return;
	}
	if ((fp = fdopen(mf, "w")) == NULL) {
		syslog(LOG_ERR, "fdopen: %m, cannot dump mountlist to %s", RMTAB);
		return;
	}
	for (ml = mountlist; ml != NULL; ml = ml->ml_nxt) {
		ml->ml_pos = ftell(fp);
		(void) fprintf(fp, "%s:%s\n", ml->ml_name, ml->ml_path);
	}
	if (rename(t2, RMTAB) < 0) 
		syslog(LOG_ERR, "rename: %m, cannot dump mountlist to %s", RMTAB);
	(void) fclose(fp);

	 f = fopen(RMTAB, "r+");
}

/*
 * send current export list
 */
mtd_export(transp)
	SVCXPRT *transp;
{
	struct exports *ex;

	if (!svc_getargs(transp, xdr_void, NULL)) {
		svcerr_decode(transp);
		mtd_abort();
	} else {
		/*
		 * Send exported request the list of flattened exports
		 * to conform with other os's.  Otherwise, all 
		 * exported directories would not be seen.
		 */
		ex = flatexports;
		if (!svc_sendreply(transp, xdr_exports, &ex)) {
			syslog(LOG_ERR, "couldn't reply to EXPORT rpc call");
			mtd_abort();
		}
	}
}

int exported_to_all;     /* keeps track of whether current exports 
			    entry is exported to the world */

/*
 * Parse exports file.  If this is the first call or the file exportfile
 * has changed, it is opened and parsed to create an exports list.
 * File should look like:
 *
 * pathname [options] [name1 name2 ... namen]
 *   or
 * .INCLUDE exportfile
 *   or
 * #anything
 *
 * pathname:	name of a mounted local file system
 *		name of a directory of a mounted local filesystem
 * optional options
 *	 -r(oot)=0	for entire export, root maps to 0;
 *			overrides anon=uid for client superusers
 *	 -r(oot)=hlist  maps root users on specified hosts to 0;
 *			hlist is of the form client{:client} where
 *			client can be a host name or IP address;
 *			overrides anon=uid for client superusers
 *	 -anon=uid	maps anonymous users to uid; root users and AUTH_NULL
 *			requests are anonymous; setting to -1 disables
 *			anonymous access
 *	 -(r)o		entire filesystem exported readonly
 *	 -rw=hlist	limits rd-wr access to hosts specified;
 *			hlist is of the form client{:client} where
 *			client can be a host name or IP address
 *	 -access=hlist  list of hosts to grant mount access to; provided for 
 *			SunOS compat; use of whitespace separated name list
 *			following options is still accepted; 
 *			hlist is of the form client{:client} where
 *			client can be a host name, IP address, or netgroup
 * 	 -m=path	specifies name of local filesystem to export; the
 *			first pathname in column 1 is interpreted as the 
 *			path the clients specify when (u)mounting but is 
 *			not an actual path on the system (for ASE)
 *
 * name:	        netgroup, hostname, IP address or a list of 
 *			whitespace separated names (no names implies everyone)
 *
 * .INCLUDE exportfile: specifies the name of an additional exports file
 *			to parse (for ASE)
 *
 * A '#' anywhere in the line marks a comment to the end of that line
 *
 * NOTE: a non-white character in column 1 indicates a new export
 *	specification.
 */
set_exports()
{
	int bol;	/* begining of line */
	int eof;	/* end of file */
	int opt;	/* beginning of option */
	struct exports *ex=NULL, *ex2;
	int newdev();
	char ch;
	char *str;
	char *l;
	char line[MAXLINE];	/* current line */
	char rpath[MAXPATHLEN];
	struct stat statb;
	struct statfs statfsb;
	FILE *fp;
	int isdev;
	int found;
	int bad_entry=0;  /* true if current entry does not exist or is a duplicate */
	struct exportfile_entry *exfn_entry;
	int modified=0;
	char excmd[MAXLINE];
	char exfn[MAXPATHLEN];
	char remotepath[MAXPATHLEN];
	char *remotepath_l;
	int remotepath_flag;

	exported_to_all = 0;
	/*
	 * Check to see if any of the export files have
	 * changed.  Exit if not changed!
	 */
	for (exfn_entry = exportfile_q; 
	     exfn_entry != NULL;
	     exfn_entry = exfn_entry->next) {
		if (stat(exfn_entry->fn,&statb) == 0) {
			if (exfn_entry->statb.st_mtime != statb.st_mtime) {
				modified = 1;
				break;
			}
		} else {
			if (!strcmp(exportfile,exfn_entry->fn)) {
#ifdef DEBUG
				(void) fprintf(stderr, "%s: stat of exportfile failed", pgmname);
				perror("mountd: exportfile");
#endif
				freeex(exports);
				exports = NULL;
				freeex(flatexports);
				flatexports = NULL;
				free_remotelocalmap();
				return;
			}
			if (exfn_entry->statb.st_mtime != 0) {
				/* alt exportfile no longer exists */
				modified = 1;
				break;
			}
		}
	}
	
	if (!modified) {
		return;
	}
	

	if ((fp = fopen(exportfile, "r")) == NULL) {
                syslog(LOG_ERR, "fopen of %s: %m", exportfile);
		freeex(exports);
		exports = NULL;
		freeex(flatexports);
		flatexports = NULL;
		free_remotelocalmap();
		return;
	}

        dupcache_inval();               /* invalidate the dup req cache */
        freeex(exports);
        exports = NULL;
        freeex(flatexports);
        flatexports = NULL;
	free_remotelocalmap();  

        free_exportfile();              /* reset exportfile queue */
        add_exportfile(exportfile);

	(void) fclose(fp);

	/*
	 * Parse each export file
	 */
	for (exfn_entry = exportfile_q; 
	     exfn_entry != NULL;
	     exfn_entry = exfn_entry->next) {

#ifdef DEBUG_FULL
		(void)fprintf(stderr, "parsing exportfile %s\n", 
				exfn_entry->fn);
#endif
		if ((fp = fopen(exfn_entry->fn, "r")) == NULL) {
			syslog(LOG_ERR, "fopen failure %s: %m", exfn_entry->fn);
			(void) fclose(fp);
			continue;
		}
		
		l = line;
		*l = '\0';
		eof = 0;
		while (!eof) {
			switch (*l) {
			case '\0':
			case '\n': /* End of line reached */
				/* 
				 * If previous entry was exported to the
				 * world, free the groups list.  Done so
				 * that we do not have to force users to
				 * make the access list a superset of 
				 * the rw= and root= lists.
				 */
				if (exported_to_all) {
					if (isdev) 
					    free_groups(ex);
					else 
					    free_groups(ex->ex_next);
					exported_to_all = 0;
				}

				/*
				 * Read next line and set state vars
				 */
				if (fgets(line, MAXLINE, fp) == NULL) {
					eof = 1;
				} else {
					bol = 1;
					opt = 0;
					l = line;
				}
				break;
			case ' ':
			case '	':
				/*
				 * If this is the continuation of a bad entry, skip the line
				 */
				if (bad_entry) {
					*l = '\0';
					break;
				}
				/*
				 * White space, skip to first non-white
				 */
				while (*l == ' ' || *l == '	') {
					l++;
				}
				bol = 0;
				break;
			case '#':
				/*
				 * Comment, skip to end of line.
				 */
				*l = '\0';
				break;
			case '.':
				/*
				 * Export command.  The only command for now
				 * is .INCLUDE which specifies an
				 * additional exports file to parse.
				 */
				if ((sscanf(l,"%s%s",excmd,exfn) == 2) &&
				    (!strcmp(".INCLUDE",excmd))) {
					add_exportfile(exfn);
				}
				*l = '\0';
				break;
			case '-':
				/*
				 * option of the form: -option=value or -option
				 */
				if (bol) {
					syslog(LOG_ERR, "Cannot parse '%s'", l);
					*l = '\0';
					break;
				}
				opt = 1;
				l++;
				break;
			default:
				/*
				 * normal character: if col one get dir else name or opt
				 */
				str = l;
				while (*l != ' ' && *l != '	' &&
				       *l != '\0' && *l != '\n') {
					l++;
				}
				ch = *l;
				*l = '\0';
				if (bol) {
					/*
					 * Logic for building export list:
					 *  The list is organized by dev horizontally,
					 *  all entries of the same dev hang off vertically.
					 *  The top dev entry is just the first one read.
					 *
					 *  Try to find an exports list entry with the
					 *  same dev number.  If not found, this entry
					 *  represents a new top entry.  Connect its
					 *  devptr to the front of the exports list.
					 *  If a dev match was found, tack this entry on
					 *  to the "next" pointer of the top entry for this 
					 *  dev.  Always check if this is a duplicate entry.
					 */
#ifdef DEBUG_FULL
					(void) fprintf(stderr, "--- next /etc/exports entry to add is %s ---\n", str);
#endif DEBUG_FULL
					
					bad_entry = 0;  /* this is a new export entry */
					remotepath_flag = 0;

					remotepath_l = strstr(l+1,"-m=");
					if (remotepath_l != NULL) {
						strcpy(remotepath,remotepath_l+3);
						remotepath_l = remotepath;
						while (*remotepath_l != ' ' && *remotepath_l != '	' &&
						       *remotepath_l != '\0' && *remotepath_l != '\n') {
							remotepath_l++;
						}
						*remotepath_l = '\0';
						remotepath_flag = 1;
					} else {
						strcpy(remotepath,str);
					}

					if (stat(remotepath, &statb) < 0) {
						syslog(LOG_ERR, "stat: %m, Cannot stat %s", remotepath);
						bad_entry = 1;
						break;
					}
					/*
					 * Expand path into its canonical form, cache the result
					 */
					if (realpath(remotepath, rpath) == NULL) {
						syslog(LOG_ERR, 
						       "set_exports: realpath failed on %s: %m", remotepath);
						bad_entry = 1;
						break;
					}
					
					for (ex = exports; ex != NULL; ex = ex->ex_devptr) 
						if (ex->ex_dev == statb.st_dev) break;
					
					if (ex == NULL) {
						/* check if local fs */
						if (statfs(rpath, &statfsb) < 0) {
							syslog(LOG_ERR, "Cannot statfs %s: %m", rpath);
							bad_entry = 1;
							break;
						}
						if (statfsb.f_type == MOUNT_NFS) {
							syslog(LOG_ERR, "Cannot export non-local filesystem: %s", rpath);
							bad_entry = 1;
							break;
						}
						
						isdev = TRUE;
#ifdef DEBUG_FULL
						(void) fprintf(stderr, "adding new export %s\n", rpath);
#endif DEBUG_FULL
						/* 
						 * assume exported to all
						 * until we find out o.w.
						 */
						exported_to_all = 1;
						newdev(rpath, statb.st_dev, statb.st_ino, statb.st_gen);
						if (remotepath_flag == 1) {
							add_remotelocalmap(str,rpath);
						}
						ex = exports;
					}
					else {
						found = FALSE;
						for (ex2=ex; ex2 != NULL; ex2 = ex2->ex_next)
							if (ex2->ex_ino == statb.st_ino) {
							syslog(LOG_ERR, "Duplicate directory entry for %s - duplicate ignored", str); 
							found = TRUE;
							bad_entry = 1;
							break;
						}
						if (!found) {
							isdev = FALSE;
							/*
							  initialize rootmap to nobody 
						*/
#ifdef DEBUG_FULL
							(void) fprintf(stderr, "adding new export %s\n", rpath);
#endif DEBUG_FULL
							/* 
							 * assume exported to
							 * all until we find
							 * out o.w.
							 */
							exported_to_all = 1;
							ex->ex_next = newex(rpath, ex->ex_next, statb.st_dev, statb.st_ino, statb.st_gen, (uid_t)-2, 0, (uid_t)-2, NULL, NULL);
							if (remotepath_flag == 1) {
								add_remotelocalmap(str,rpath);
							}
						}
						else 
							break;  /* skip rest of entry */
					}
				}
				else {
					if (opt) {
						opt = 0;
						if (isdev)
							setopt(str, ex);
						else
							setopt(str, ex->ex_next);
					}
					else {
						if (!isdev) {
#ifdef DEBUG_FULL
							(void) fprintf(stderr, "adding new groups %s to %s\n", str, ex->ex_next->ex_name);
#endif DEBUG_FULL
							ex->ex_next->ex_groups = newgroup(str, ex->ex_next->ex_groups);
						}
						else {
#ifdef DEBUG_FULL
							(void) fprintf(stderr, "adding new groups %s to %s\n", str, ex->ex_name);
#endif DEBUG_FULL
							ex->ex_groups = newgroup(str, ex->ex_groups);
						}
						exported_to_all = 0;
					}
				}
				*l = ch;
				bol = 0;
				break;
			}
		}
		(void) fclose(fp);
	}
#ifdef DEBUG
	(void) fprintf(stderr, "*** export list final results ***\n");
	print_exports(exports);
	(void) fprintf(stderr, "\n*** exportfile list ***\n");
	print_exportfile();
	(void) fprintf(stderr, "\n*** remotelocalmap list ***\n");
	print_remotelocalmap();
#endif DEBUG

	/*
	 * Update the kernel's exportfsdata list so that it is in sync with
	 * the exports file and the mountd's list.
	 */

	update_exportfsdata();
	flatten_exports();
#ifdef DEBUG_FULL
	(void) fprintf(stderr, "*** export list final results flattened ***\n");
	print_exports(flatexports);
#endif DEBUG_FULL
	return;
}

/*
 * Make exportfs calls necessary to update kernel's view of exports.
 */
update_exportfsdata()
{
	struct exports *ex, *ex2, *k_ex=NULL, *k_ptr;
	struct exportfsdata kbuf;
	u_int cookie=0;
	int naddrs, i;
	struct sockaddr_in *sin;

	/*
	 * Build a list equivalent to kernel's list.
	 */
	for (;;) {
		if ((exportfs (EXPORTFS_READ, &cookie, &kbuf)) < 0) {
			if (errno == ENOENT) break;
			syslog(LOG_ERR, "exportfs READ: %m");
			exit (1);
		}
		k_ex = newex(kbuf.e_path, k_ex, kbuf.e_dev, kbuf.e_ino, kbuf.e_gen, kbuf.e_rootmap, kbuf.e_flags, kbuf.e_anon, &kbuf.e_rootaddrs, &kbuf.e_writeaddrs);
		if (kbuf.e_more == 0) break;
	}
#ifdef DEBUG_FULL 
	(void) fprintf (stderr, "*** Kernel exportfsdata before changes ***\n");
	print_exports(k_ex);
#endif DEBUG_FULL  

	/*
	 * Loop through mountd's export list.  Find matching entry in
	 * local view of kernel's exportfsdata list.  If one is not found,
	 * add this export to kernel's list.  If they are different,
	 * update the kernel's data.  Mark the k_ex entry that has 
	 * been looked at so that we know what hasn't been checked yet.
	 */

	 for (ex=exports; ex!=NULL; ex=ex->ex_devptr) {
		for (ex2=ex; ex2!=NULL; ex2=ex2->ex_next) {

			/*
			   find matching kernel entry
			*/
			for (k_ptr=k_ex; k_ptr!=NULL; k_ptr=k_ptr->ex_next)
				if ((strcmp(k_ptr->ex_name, ex2->ex_name))==0) break;

			/*
			   if export is not in kernel's exportfsdata, add it 
			*/
			if (k_ptr == NULL) {
#ifdef DEBUG_FULL
				(void) fprintf(stderr, "Adding new export %s to kernel\n", ex2->ex_name);
#endif DEBUG_FULL 
				extokex(ex2, &kbuf);
				if ((exportfs(EXPORTFS_CREATE, NULL, &kbuf)) < 0) {
					syslog(LOG_ERR, "exportfs CREATE of %s: %m", kbuf.e_path);
					exit (1);
				}
			}
			/*
			 * if kernel's exportfsdata struct is not uptodate, replace it 
			 */
			else if (!(ex_same(k_ptr, ex2))) {
#ifdef DEBUG_FULL
				(void) fprintf(stderr, "Fixing kernel export %s\n", ex2->ex_name);
#endif DEBUG_FULL 
				extokex(k_ptr, &kbuf);
				if ((exportfs (EXPORTFS_REMOVE, NULL, &kbuf))< 0) {
					syslog(LOG_ERR, "exportfs REMOVE of %s: %m", kbuf.e_path);
					exit (1);
				}
				extokex(ex2, &kbuf);
				if ((exportfs(EXPORTFS_CREATE, NULL, &kbuf)) < 0) {
					syslog(LOG_ERR, "exportfs CREATE(2) of %s: %m", kbuf.e_path);
					exit (1);
				}
				k_ptr->ex_name= "CHECKED";
			}
			else k_ptr->ex_name= "CHECKED";
		}
	}
	/*
	 * Loop through k_ex list looking for entries that need to be 
	 * removed from kernel's exportfsdata list.
	 */

	for (k_ptr=k_ex; k_ptr!=NULL; k_ptr=k_ptr->ex_next) {
		if (strcmp(k_ptr->ex_name, "CHECKED") != 0) {
#ifdef DEBUG_FULL
			(void)fprintf(stderr,"Removing kernel export %s\n", k_ptr->ex_name);
#endif DEBUG_FULL 
			extokex(k_ptr, &kbuf);
			if ((exportfs (EXPORTFS_REMOVE, NULL, &kbuf)) < 0) {
				syslog(LOG_ERR, "exportfs REMOVE(2) of %s: %m", kbuf.e_path);
				exit (1);
			}
		}
	}
	freeex(k_ex);

#ifdef DEBUG_FULL 
	(void) fprintf (stderr, "*** Kernel exportfsdata after changes ***\n");
	cookie = 0;
	for (;;) {
		if ((exportfs (EXPORTFS_READ, &cookie, &kbuf)) < 0) {
			syslog(LOG_ERR, "exportfs READ(2): %m");
			exit (1);
		}
		(void)fprintf(stderr,"export name %s\n", kbuf.e_path);
		(void)fprintf(stderr,"\tdev: %d ino: %d rootmap=%d anon=%d", 
			kbuf.e_dev, kbuf.e_ino, kbuf.e_rootmap, kbuf.e_anon);
		if (kbuf.e_flags & M_EXRDONLY)
			(void)fprintf(stderr," (M_EXRDONLY)");
		if (kbuf.e_flags & M_EXRDMOSTLY)
			(void)fprintf(stderr," (M_EXRDMOSTLY)");
		(void)fprintf(stderr, "\n");

		naddrs = kbuf.e_rootaddrs.naddrs;
		(void)fprintf(stderr, "\tRootaddrs (%d): ", naddrs);
		for (i=0; i<naddrs; i++) {
			sin = (struct sockaddr_in *)
				(kbuf.e_rootaddrs.addrvec + i);
			(void)fprintf(stderr, "%s ", inet_ntoa(sin->sin_addr));
		}
		(void)fprintf(stderr, "\n");

		naddrs = kbuf.e_writeaddrs.naddrs;
		(void)fprintf(stderr, "\tWriteaddrs (%d): ", naddrs);
		for (i=0; i<naddrs; i++) {
			sin = (struct sockaddr_in *)
				(kbuf.e_writeaddrs.addrvec + i);
			(void)fprintf(stderr, "%s ", inet_ntoa(sin->sin_addr));
		}
		(void)fprintf(stderr, "\n");

		if (kbuf.e_more == 0) break;
	}
	(void)fprintf(stderr, "\n");
#endif DEBUG_FULL
}

/*
 * Compare kernel's export entry to mountd's
 */

ex_same (k_ex, ex)
	struct exports *k_ex, *ex;
{
	int i;

	if ((k_ex->ex_rootmap != ex->ex_rootmap) || 
	    (k_ex->ex_flags != ex->ex_flags) ||
	    (k_ex->ex_ino != ex->ex_ino) || 
	    (k_ex->ex_gen != ex->ex_gen) || 
	    (k_ex->ex_dev != ex->ex_dev) || 
	    (k_ex->ex_anon != ex->ex_anon) ||
	    (k_ex->ex_rootaddrs.naddrs != ex->ex_rootaddrs.naddrs) ||
	    (k_ex->ex_writeaddrs.naddrs != ex->ex_writeaddrs.naddrs))
		return (FALSE);

	for (i = 0; i < ex->ex_rootaddrs.naddrs; i++) {
	     if ((u_int)((struct sockaddr_in *)
			 (&k_ex->ex_rootaddrs.addrvec[i]))->sin_addr.s_addr !=
	         (u_int)((struct sockaddr_in *)
			 (&ex->ex_rootaddrs.addrvec[i]))->sin_addr.s_addr)
	    	 return (FALSE);
	}

        for (i = 0; i < ex->ex_writeaddrs.naddrs; i++) {
	     if ((u_int)((struct sockaddr_in *)
			 (&k_ex->ex_writeaddrs.addrvec[i]))->sin_addr.s_addr !=
	         (u_int)((struct sockaddr_in *)
			 (&ex->ex_writeaddrs.addrvec[i]))->sin_addr.s_addr)
	         return (FALSE);
	}

	return (TRUE);
}


newdev (name, dev, ino, gen)
	char *name;
	dev_t dev;
	ino_t ino;
	uint_t gen;
{
	struct exports *new;
	char *newname;

	new = (struct exports *)exmalloc(sizeof(*new));
	newname = (char *)exmalloc(strlen(name) + 1);
	(void) strcpy(newname, name);

	new->ex_name = newname;
	new->ex_dev = dev;
	new->ex_ino = ino;
	new->ex_gen = gen;
	new->ex_next = NULL;
	new->ex_rootmap = -2;		/* Initialize to nobody */
	new->ex_flags = M_EXPORTED;
	new->ex_anon = -2;
	new->ex_rootaddrs.naddrs = 0;
	new->ex_rootaddrs.addrvec = NULL;
	new->ex_writeaddrs.naddrs = 0;
	new->ex_writeaddrs.addrvec = NULL;
	new->ex_groups = NULL;		/* Groups ptr */
	new->ex_devptr = exports;
	exports = new;
}

struct exports *
newex(name, next, dev, ino, gen, rootmap, flags, anon, rootaddrs, writeaddrs)
	char *name;
	struct exports *next;
	dev_t dev;
	ino_t ino;
	uint_t gen;
	uid_t rootmap;
	int flags;
	uid_t anon;
	struct exportfsaddrlist *rootaddrs;
	struct exportfsaddrlist *writeaddrs;
{
	struct exports *new;
	char *newname;
	int naddrs;

	new = (struct exports *)exmalloc(sizeof(*new));
	newname = (char *)exmalloc(strlen(name) + 1);
	(void) strcpy(newname, name);

	if (rootaddrs == NULL) {
		new->ex_rootaddrs.naddrs = 0;
		new->ex_rootaddrs.addrvec = NULL;
	} else {
		naddrs = rootaddrs->naddrs;
		new->ex_rootaddrs.naddrs = naddrs;
		if (naddrs > 0) {
			new->ex_rootaddrs.addrvec = (struct sockaddr *)
				exmalloc(naddrs * sizeof(struct sockaddr));
        		bcopy(rootaddrs->addrvec, new->ex_rootaddrs.addrvec, 
				naddrs * sizeof(struct sockaddr));
		}
	}

	if (writeaddrs == NULL) {
		new->ex_writeaddrs.naddrs = 0;
		new->ex_writeaddrs.addrvec = NULL;
	} else {
		naddrs = writeaddrs->naddrs;
		new->ex_writeaddrs.naddrs = naddrs;
		if (naddrs > 0) {
			new->ex_writeaddrs.addrvec = (struct sockaddr *)
				exmalloc(naddrs * sizeof(struct sockaddr));
        		bcopy(writeaddrs->addrvec, new->ex_writeaddrs.addrvec, 
				naddrs * sizeof(struct sockaddr));
		}
	}

	new->ex_name = newname;
	new->ex_dev = dev;
	new->ex_ino = ino;
	new->ex_gen = gen;
	new->ex_rootmap = rootmap;
	new->ex_flags = flags|M_EXPORTED;
	new->ex_anon = anon;
	new->ex_next = next;
	new->ex_groups = NULL;		/* Groups ptr */
	new->ex_devptr = NULL;
	return (new);
}

extokex(ex, kex)
	struct exports *ex;
	struct exportfsdata *kex;
{
	(void)strcpy (kex->e_path, ex->ex_name);
	kex->e_dev = ex->ex_dev;
	kex->e_ino = ex->ex_ino;
	kex->e_gen = ex->ex_gen;
	kex->e_rootmap = ex->ex_rootmap;
	kex->e_flags = ex->ex_flags;
	kex->e_anon = ex->ex_anon;
	kex->e_more = 0;
	kex->e_rootaddrs.naddrs = ex->ex_rootaddrs.naddrs;
	kex->e_writeaddrs.naddrs = ex->ex_writeaddrs.naddrs;
        bcopy(ex->ex_rootaddrs.addrvec, kex->e_rootaddrs.addrvec, 
			kex->e_rootaddrs.naddrs * sizeof(struct sockaddr));
        bcopy(ex->ex_writeaddrs.addrvec, kex->e_writeaddrs.addrvec, 
			kex->e_writeaddrs.naddrs * sizeof(struct sockaddr));
}

struct groups *
newgroup(name, next)
	char *name;
	struct groups *next;
{
	struct groups *new;
	char *newname;
	u_int saddr;
	struct hostent *hp;

	new = (struct groups *)exmalloc(sizeof(*new));
	/*
	 * Can specify host by IP address
	 */
	if (isdigit(*name)) {
		saddr = inet_addr(name);
		if ((saddr != -1) && ((hp = gethostbyaddr((caddr_t)&saddr, 
			  sizeof(saddr), AF_INET)) != NULL)) {
				newname = (char *)exmalloc(strlen(hp->h_name) + 1);
				(void)strcpy(newname, hp->h_name);
		}
		else {
			newname = (char *)exmalloc(strlen(name) + 1);
			(void) strcpy(newname, name);
		}
	} else {
		newname = (char *)exmalloc(strlen(name) + 1);
		(void) strcpy(newname, name);
	}

	new->g_name = newname;
	new->g_next = next;
	new->g_hbit = 0;
	return (new);
}

setopt(str, ex)
	char *str;
	struct exports *ex;
{
	char *arg, *optend, *m, *n;

	/*
	 * Parse options
	 */
	while (str && *str) {
		if (optend = index(str, ','))
			*optend++ = '\0';
		if (arg = index(str, '='))
			*arg++ = '\0';

		if (!strcmp(str, "ro") || !strcmp(str, "o")) {
			if (!(ex->ex_flags & M_EXRDMOSTLY))
				ex->ex_flags |= M_EXRDONLY;
		} 
		else if (!strcmp(str, "rw")) {
			if (!arg) {
				syslog(LOG_ERR, 
				   "Bad rw=hostlist syntax for path %s in %s", 
				    ex->ex_name, exportfile);
				str = optend;
				continue;
			}
			ex->ex_flags = M_EXRDMOSTLY|M_EXPORTED;
			filladdrs(&ex->ex_writeaddrs, arg, ex, 
				  ex->ex_name, "rw=\0");
		} 
		else if (!strcmp(str, "root") || !strcmp(str, "r")) {
			if (!arg) {
				syslog(LOG_ERR, 
				   "Bad root= syntax for path %s in %s", 
				    ex->ex_name, exportfile);
				str = optend;
				continue;
			}
			m = arg;
			if ((*m++ == '0') && (*m == '\0'))
				ex->ex_rootmap=0;
			else {
				filladdrs(&ex->ex_rootaddrs, arg, 
					ex, ex->ex_name, "root=\0");
			}
		}
		else if (!strcmp(str, "anon")) {
			if (!arg) {
				syslog(LOG_ERR, 
				   "Bad anon=uid syntax for path %s in %s", 
				    ex->ex_name, exportfile);
				str = optend;
				continue;
			}
			m = arg;
			if (*m == '-')
				n = m+1;
			else
				n = m;
			for (; *n ; n++) {
				if (!isdigit(*n)) {
					syslog(LOG_ERR, "%c invalid anon= mapping integer for path %s in %s", *m, ex->ex_name, exportfile);
					break;
				}
			}
			if (*n=='\0') ex->ex_anon = atoi(m);
		} 
		else if (!strcmp(str, "access")) {
			if (!arg) {
				syslog(LOG_ERR, 
				   "Bad access= syntax for path %s in %s", 
				    ex->ex_name, exportfile);
				str = optend;
				continue;
			}
			exported_to_all = 0; /* this is not a global export */
			while (arg && *arg) {
				if (m = index(arg, ':'))
					*m++ = '\0';
				ex->ex_groups = newgroup(arg, ex->ex_groups);
				arg = m;
			}
		}
		else if (!strcmp(str, "m")) {
			/*
			 * Ignore the -m= option here.  It has already
			 * been parsed in set_exports().
			 */
		} else if (strcmp(str, "n") != 0) {
			/*
		 	 * If n, ignore (ULTRIX pre 4.0 no filehandle option 
			 * obsolete); Otherwise, bad option
		 	 */
			syslog(LOG_ERR, 
				"%s unknown export option for path %s in %s", 
				str, ex->ex_name, exportfile);
		}
		str = optend;
	}
}

/* 
 * Build the exaddrlist given the hlist string from the exports file
 */
void
filladdrs(addrlist, hlist, ex, path, ex_option)
	struct exaddrlist *addrlist;	/* ptr to rootaddrs or writeaddrs */
	char *hlist; 			/* list of host names */
	struct exports *ex;		/* for groups list */
	char *path;  			/* exported path */
	char ex_option[6];  		/* either root= or rw= */
{
	char *c;

	if (addrlist->addrvec == NULL) {
	      addrlist->addrvec = 
	         (struct sockaddr *)exmalloc(EXMAXADDRS * sizeof(struct sockaddr));
	}

	while (hlist && *hlist && (addrlist->naddrs < EXMAXADDRS)) {
		if (c = index(hlist, ':'))
			*c++ = '\0';
		getaddr(addrlist, hlist, ex, path, ex_option);
		hlist = c;
	}

	if (hlist && *hlist) {
	    syslog(LOG_ERR, 
		"Warning: only %d hostnames read from %s list for path %s in %s", 
		EXMAXADDRS, ex_option, path, exportfile);
	}
}

void
getaddr(addrlist, hname, ex, path, ex_option)
	struct exaddrlist *addrlist;	/* ptr to rootaddrs or writeaddrs */
	char *hname; 			/* host name or IP address */
	struct exports *ex;		/* for groups list */
	char *path;  			/* exported path */
	char *ex_option;  		/* either root= or rw= */
{
	struct hostent *hp;
	u_int saddr;
	struct sockaddr_in *sin;
	int i, j;

	i = addrlist->naddrs;

	/* Is it an IP address? */
	if (isdigit(*hname)) {
		saddr = inet_addr(hname);
		if ((saddr != -1) && ((hp = gethostbyaddr((caddr_t)&saddr, 
			  sizeof(saddr), AF_INET)) != NULL)) {

			/* Add the IP address to the exaddrlist */
			bzero((char *)&addrlist->addrvec[i],
				sizeof(struct sockaddr));
			addrlist->addrvec[i].sa_family = AF_INET;
			sin = (struct sockaddr_in *)(addrlist->addrvec + i);
			bcopy(&saddr, &sin->sin_addr, sizeof(saddr));
			addrlist->naddrs++;

			/* 
			 * Copy this IP address to the generic access list 
			 * so that it doesn't have to be a superset of 
			 * root=hlist and rw=hlist.
			 */
			ex->ex_groups = newgroup(hname, ex->ex_groups);
			return;
		}
	} 
	/* Is it a hostname? */
	if ((hp = gethostbyname(hname)) != NULL) {
		switch (hp->h_addrtype) {
		case AF_INET:
		     /*
		      * Create an entry for each IP address associated with
		      * this hostname.
		      */
		     for (j=0; hp->h_addr_list[j] && (i < EXMAXADDRS); j++) {
				bzero((char *)&addrlist->addrvec[i],
					sizeof(struct sockaddr));
				addrlist->addrvec[i].sa_family = AF_INET;
				sin = (struct sockaddr_in *) 
					(addrlist->addrvec + i);
				bcopy(hp->h_addr_list[j], &sin->sin_addr, 
					sizeof(sin->sin_addr));
				i = ++addrlist->naddrs;
		     }
		     /* 
		      * Copy this hostname to the generic access list 
		      * so that it doesn't have to be a superset of 
		      * root=hlist and rw=hlist.
		      */
		     ex->ex_groups = newgroup(hname, ex->ex_groups);
		     break;
		default:
		     syslog(LOG_ERR, 
		         "%s=%s for path %s has unsupported address type %d",
			 ex_option, hname, path, hp->h_addrtype);
		     break;
		}
	}
	/*
	 * Note: we don't support netgroups as part of these lists.  We
	 * could, it would mean checking the ng_cache, expanding the ng if
	 * it's timed out or not found, and then looping through each 
	 * hostname and doing a gethostbyname.  This would be very 
	 * expensive (lots of NIS calls).  In addition, the same host 
	 * can be part of many netgroups.  If we ever decide we want to 
	 * support this, we should probably add an IP address translation 
	 * cache.  (Remember the Intel CLD...) Btw, SunOS does not support 
	 * netgroups as part of these lists.
	 */
	else {
		syslog(LOG_ERR,
			"Unknown host %s in %s option for path %s in %s",
			hname, ex_option, path, exportfile);
	}
}


/*
 * Flatten exports list to show directory entries.
 */
flatten_exports()
{
	struct exports *ex, *ex2;
	struct exports *exnew = NULL;
	struct groups *groups;
	struct groups *groupsnew = NULL;

#ifdef DEBUG_FULL
	(void) fprintf(stderr, "\n******* Flattening exports: *******\n");
#endif DEBUG_FULL
	for (ex = exports; ex != NULL; ex = ex->ex_devptr) {
		for (ex2 = ex; ex2 != NULL; ex2 = ex2->ex_next) {
#ifdef DEBUG_FULL
			(void) fprintf(stderr, "making newex for %s\n", ex2->ex_name);
#endif DEBUG_FULL
			exnew = newex(get_remotepath(ex2->ex_name), exnew, (dev_t)0, (ino_t)0, 0, (uid_t)0, 0, (uid_t)0, NULL, NULL);
			for (groups = ex2->ex_groups, groupsnew = NULL; groups != NULL; groups = groups->g_next) {
#ifdef DEBUG_FULL
				(void) fprintf(stderr, "making newgroup for %s\n", groups->g_name);
#endif DEBUG_FULL
				groupsnew = newgroup(groups->g_name, groupsnew);
			}
			exnew->ex_groups = groupsnew;
		}
	}
	flatexports = exnew;
}

char *
exmalloc(size)
	int size;
{
	char *ret;

	if ((ret = (char *)malloc((unsigned) size)) == 0) {
		syslog(LOG_ERR, "Memory allocation failed in exmalloc: %m");
		exit(1);
	}
	return (ret);
}

/*
 * Free entire ex list
 */
freeex(ex)
	struct exports *ex;
{
	struct exports *next_ex, *next_dev;
	struct groups *group_ptr, *next_group;

	while (ex) {
		next_dev = ex->ex_devptr;
		while (ex) {
			next_ex = ex->ex_next;
			group_ptr = ex->ex_groups;
			while (group_ptr) {
				next_group = group_ptr->g_next;
				free (group_ptr->g_name);
				free ((char *)group_ptr);
				group_ptr = next_group;
			}
			free (ex->ex_name);
			if (ex->ex_rootaddrs.naddrs)
				free (ex->ex_rootaddrs.addrvec);
			if (ex->ex_writeaddrs.naddrs)
				free (ex->ex_writeaddrs.addrvec);
			free ((char *)ex);
			ex = next_ex;
		}
		ex = next_dev;
	}
}


/*
 * Free groups list if any.
 */
void
free_groups(ex)
	struct exports *ex;
{
	struct groups *group_ptr, *next_group;

	group_ptr = ex->ex_groups;
	while (group_ptr) {
		next_group = group_ptr->g_next;
		free (group_ptr->g_name);
		free ((char *)group_ptr);
		group_ptr = next_group;
	}
	ex->ex_groups = NULL;
}

#ifdef DEBUG
print_exports(ex)
	struct exports *ex;
{
	struct groups *groups;
	struct exports *ex2;
	int naddrs, i;
	struct sockaddr_in *sin;

	(void) fprintf(stderr, "\nexport list begin:\n");
	for (; ex != NULL; ex = ex->ex_devptr) {
		for (ex2 = ex; ex2 != NULL; ex2 = ex2->ex_next) {

		    (void)fprintf(stderr, "dir: %s\n", ex2->ex_name);
		    (void)fprintf(stderr, "\tdev %d ino %d rootmap %d anon %d",
		                  ex2->ex_dev, ex2->ex_ino, ex2->ex_rootmap, 
				  ex2->ex_anon);
		    if (ex2->ex_flags & M_EXRDONLY)
			(void) fprintf(stderr, " ( M_EXRDONLY )");
		    if (ex2->ex_flags & M_EXRDMOSTLY)
			(void) fprintf(stderr, " ( M_EXRDMOSTLY )");
		    (void)fprintf(stderr, "\n");

		    naddrs = ex2->ex_rootaddrs.naddrs;
		    (void)fprintf(stderr, "\tRootaddrs (%d): ", naddrs);
		    for (i=0; i<naddrs; i++) {
			sin = (struct sockaddr_in *)
				(ex2->ex_rootaddrs.addrvec + i);
			(void)fprintf(stderr, "%s ", inet_ntoa(sin->sin_addr));
		    }
		    (void)fprintf(stderr, "\n");
		    
		    naddrs = ex2->ex_writeaddrs.naddrs;
		    (void)fprintf(stderr, "\tWriteaddrs (%d): ", naddrs);
		    for (i=0; i<naddrs; i++) {
			sin = (struct sockaddr_in *)
				(ex2->ex_writeaddrs.addrvec + i);
			(void)fprintf(stderr, "%s ", inet_ntoa(sin->sin_addr));
		    }
		    (void)fprintf(stderr, "\n");

		    (void)fprintf(stderr, "\tGroups: ");
		    for (groups= ex2->ex_groups; groups!= NULL; 
			 groups= groups->g_next) {
				(void)fprintf(stderr, "%s hbit: %d ", 
					groups->g_name, groups->g_hbit);
		    }
		    (void) fprintf(stderr, "\n");
		}
		(void) fprintf(stderr, "finish exports for this dev\n");
	}
	(void) fprintf(stderr, "export list end:\n\n");
}
#endif DEBUG

usage()
{
	(void) fprintf(stderr,"usage: mountd [-n] [-i -d -s] [exportsfile]\n");
	(void) fflush(stderr);
	exit(1);
}

mtd_abort()
{
	/*
	 * This routine is a do-nothing to replace the libc abort, to prevent
	 * mountd from crashing all the time.  It is safe to remove this stub
	 * if running under inetd, since inetd will just restart us when we
	 * crash and the core dumps may be useful.
	 */
}


/*
 * Restore saved mount state; rewrite file without commented out
 * entries
 */
rmtab_load()
{
        char *path;
        char *name;
        char *end;
        struct mountdlist *ml;
        char line[MAXRMTABLINELEN];
	int fd;

        f = fopen(RMTAB, "r");
        if (f != NULL) {
                while (fgets(line, sizeof(line), f) != NULL) {
                        name = line;
                        path = strchr(name, ':');
                        if (*name != '#' && path != NULL) {
                                *path = 0;
                                path++;
                                end = strchr(path, '\n');
                                if (end != NULL) {
                                        *end = 0;
                                }
                                ml = (struct mountdlist *)
                                        exmalloc(sizeof(struct mountdlist));
                                ml->ml_path = (char *)
                                        exmalloc(strlen(path) + 1);
                                (void)strcpy(ml->ml_path, path);
                                 ml->ml_name = (char *)
                                        exmalloc(strlen(name) + 1);
                                (void)strcpy(ml->ml_name, name);
                                ml->ml_nxt = mountlist;
                                mountlist = ml;
                        }
                }
                (void)fclose(f);
        }
	fd = open(RMTAB, O_CREAT|O_TRUNC|O_RDWR, 0644);
	f = fdopen(fd, "r+");
        if (f != NULL) {
                for (ml = mountlist; ml != NULL; ml = ml->ml_nxt) {
                        ml->ml_pos = rmtab_insert(ml->ml_name, ml->ml_path);
                }
		return(fd);
        }
	return(-1);
}

long
rmtab_insert(name, path)
        char *name;
        char *path;
{
        long pos;

        if (f == NULL || fseek(f, (off_t)0, 2) == -1) {
                return (-1);
        }
        pos = ftell(f);
        if (fprintf(f, "%s:%s\n", name, path) == EOF) {
                return (-1);
        }
	fflush(f);
        return (pos);
}

rmtab_delete(pos)
        long pos;
{
        if (f != NULL && pos != -1 && fseek(f, (off_t)pos, 0) == 0) {
                (void) fprintf(f, "#");
                fflush(f);
	}
}


build_ngcache()
{
	int err, i;
	struct ypall_callback cbinfo;

	cbinfo.foreach = getkeys;
	cbinfo.data = NULL;

	err = yp_all(mydomain, "netgroup", &cbinfo);
	if (err) {
		if (err != YPERR_MAP)
			syslog(LOG_ERR, "YP error building netgroup cache: %s",
				yperr_string(err));
		return;
	}
	for (i=0; i< num_ngs; i++) {
		(void)expand_ng(ng_names[i]);
		free(ng_names[i]);
	}	
}

/*
 * Called for each netgroup in yp database.  Returns 0 to have yp_all
 * call it again to process the next netgroup.
 */
static int
getkeys(status, key, kl, val, vl, data)
	int status;
	char *key;
	int kl;
	char *val;
	int vl;
	char *data;
{
	int size;

	if (status == YP_TRUE) {
		size = kl + 1;
		ng_names[num_ngs] = (char *)malloc((unsigned) size);
		strncpy(ng_names[num_ngs], key, kl);
		ng_names[num_ngs++][kl] = '\0';
		/*
		 * initial cache size is limited to 100 netgroups
		 */
		if (num_ngs == 100)  
			return(1);
		return(0);
	}
	if (status != YP_NOMORE)
		syslog(LOG_ERR, "YP error expanding netgroups: %s",
			yperr_string(ypprot_err(status)));
	return(1);
}


int
expand_ng(ngname)
	char *ngname;
{
	char *machp, *userp, *domainp;
	int is_a_ng=0;

	setnetgrent(ngname);
	while (getnetgrent(&machp, &userp, &domainp)) {
		if (is_a_ng++ == 0)  
			new_ng(ngname);
		if ((domainp == NULL) || (strcmp(domainp, mydomain) == 0)) {
			if (machp == NULL) {
				new_ng_host("everyone");
				break;
			}
			else if (isalnum(*machp) || (*machp == '_'))
				new_ng_host(machp);
		}
	}
	endnetgrent();
#ifdef DEBUG
	(void)fprintf(stderr,"Expand ng of %s returning %d\n", ngname, is_a_ng);
#endif DEBUG
	return(is_a_ng);
}


/* Is "export_gname" a netgroup and if so is client "mach" in it ??
 * Searches netgroup cache, rebuilds the ng entry if it is timed out,
 * adds a new ng entry if need be.  Sets ng_or_host to 0 if export_gname
 * is a netgroup, 1 if it is a hostname.
 */
int 
innetgr_cache(export_gname, mach, ng_or_host, aliases)
	char *export_gname, *mach;
	int *ng_or_host;
	char **aliases;
{
	struct ng *ngp;
	int success, match;
	static int variance;

#ifdef DEBUG
	(void)fprintf(stderr, "innetgr_cache called with gname: %s, mach: %s\n",
		export_gname, mach);
#endif DEBUG

	/* Try to find netgroup in cache */
	for (ngp = nglist; ngp != NULL; ngp = ngp->next_ng) {
		if (strcmp(export_gname, ngp->ng_name) == 0) {
			/*
			 * In cache, is it timed out?  Netgroups time out
			 * after 15-30 minutes.
			 */
			gettimeofday(&now, &tz);
			if (variance > 900) 
				variance=0;
			if (now.tv_sec - ngp->ng_stamp > (ngtimeout+variance)) {
			/*
			 * Timed out, re-expand it
			 */
#ifdef DEBUG
				(void)fprintf(stderr, 
				    "timed out: variance is %d\n", variance);
#endif DEBUG
				success = expand_ng(ngp->ng_name);
				if (success) {
					free_ng(ngp);
					ngp = nglist;
					/* add 1 minute to variance */
					variance += 60;
				}
				else {
					syslog(LOG_ERR, 
					  "Could not expand netgroup %s", 
					   ngp->ng_name);
					*ng_or_host = 1;
					return(FALSE);
				}
			}
			/* Is requesting host in this ng? */
			match = check_ng(ngp, mach, aliases);
			return(match);
		}
	}
	/* 
	 * export_gname not found in cache, try to expand it
	 */
	success = expand_ng(export_gname);
	if (! success) {
		*ng_or_host = 1;  /* its a hostname */
		return(FALSE);
	}
	match = check_ng(nglist, mach, aliases);
	return(match);
}
	

/*
 * returns true if mach is in the cached netgroup pointed to by ngp
 */
int
check_ng(ngp, mach, aliases)
	struct ng *ngp;
	char *mach;
	char **aliases;
{
	struct hlist *hptr;
	size_t length;
	char **aliasptr;

	length = (size_t) local_hostname_length(mach);
	for (hptr = ngp->ng_hosts; hptr != NULL; hptr = hptr->next_host) {
		if ((strcmp(hptr->hname, mach) == 0) || 
		    (strcmp(hptr->hname, "everyone") == 0))
			return(TRUE);
		else if ((length != NULL) && 
			 (strlen(hptr->hname) == length) &&
			 (strncasecmp(hptr->hname, mach, length) == 0))
			return(TRUE);
		for (aliasptr = aliases; *aliasptr != NULL; aliasptr++) {
			if (strcmp(hptr->hname, *aliasptr) == 0)
				return(TRUE);
		}
	}
#ifdef DEBUG
	(void)fprintf(stderr, "Check ng returning FALSE\n");
#endif DEBUG
	return(FALSE);
}

new_ng(ngname)
	char *ngname;
{
	struct ng *new;
	char *newname;

	new = (struct ng *)exmalloc(sizeof(*new));
	newname = (char *)exmalloc(strlen(ngname) + 1);
	(void)strcpy(newname, ngname);

	new->ng_name = newname;
	new->ng_hosts = NULL;
	gettimeofday(&now, &tz);
	new->ng_stamp = now.tv_sec;
	new->next_ng = nglist;
	new->prev_ng = NULL;
	if (nglist != NULL)
		nglist->prev_ng = new;
	nglist = new;
}

new_ng_host(hname)
	char *hname;
{
	struct hlist *new;
	char *newname;

	new = (struct hlist *)exmalloc(sizeof(*new));
	newname = (char *)exmalloc(strlen(hname) + 1);
	(void)strcpy(newname, hname);

	new->hname = newname;
	new->next_host = nglist->ng_hosts;
	nglist->ng_hosts = new;
}

free_ng(ngp)
	struct ng *ngp;
{
	struct hlist *hptr, *tmphptr;

	hptr = ngp->ng_hosts;
	while (hptr) {
		tmphptr = hptr->next_host;
		free(hptr->hname);
		free(hptr);
		hptr = tmphptr;
	}
	if (nglist == ngp) {
		nglist = ngp->next_ng;
		ngp->next_ng->prev_ng = NULL;
	}
	else {
		ngp->prev_ng->next_ng = ngp->next_ng;
		if (ngp->next_ng != NULL)
			ngp->next_ng->prev_ng = ngp->prev_ng;
	}
	free(ngp->ng_name);
	free(ngp);
}

#ifdef DEBUG
print_ngcache()
{
	struct ng *ngp;
	struct hlist *hptr;

	(void) fprintf(stderr, "NETGROUPS CACHE begin: \n");
	for (ngp = nglist; ngp != NULL; ngp = ngp->next_ng) {
		(void) fprintf(stderr, "\t ng_name: %s  stamp: %d\n",
			ngp->ng_name, ngp->ng_stamp);
		(void) fprintf(stderr, "\t HOSTS:\n");
		for (hptr=ngp->ng_hosts; hptr!=NULL; hptr=hptr->next_host) {
			(void)fprintf(stderr, "\t\t %s\n", hptr->hname);
		}
	}
	(void) fprintf(stderr, "NETGROUPS CACHE end.\n");
}
#endif DEBUG

/*
 * Initialize the duplicate reply cache. . .
 */
dupcache_init() 
{
	register struct dupreq *dr;
	register struct dupreq **dt;
	int i;

	dupcache_max = 1024;
	drhashsz = dupcache_max / 16;
	drhashszminus1 = drhashsz - 1;

	dr= (struct dupreq *) malloc(sizeof(*dr) * dupcache_max);

	dt= (struct dupreq **) malloc(sizeof(struct dupreq *) * drhashsz);

	for (i = 0; i < dupcache_max; i++)
		dr[i].rc_next = &(dr[i + 1]);
	dr[dupcache_max - 1].rc_next = dr;
	dupreqcache = dr;
	drmru = dr;

	for (i = 0; i < drhashsz; i++)
		dt[i] = NULL;
	drhashtbl = dt;
}


dupcache_inval()
{
	int i;
#ifdef DEBUG
	(void) fprintf(stderr, "Invalidating duplicate requests cache\n");
#endif DEBUG
	for (i = 0; i < dupcache_max; i++)
		unhash(&dupreqcache[i]);
}


dupcache_enter(addr,dev,ino,gen, anc_name)
	struct in_addr addr;
	dev_t  dev;
	ino_t  ino;	
	uint_t	gen;	
	char *anc_name;
{
	register struct dupreq *dr;
	char *newname;

#ifdef DEBUG
	(void) fprintf(stderr,"Entering addr:%x dev:%x ino:%d %d anc:%s\n",
		addr.s_addr,dev,ino,gen, anc_name);
#endif DEBUG
	dr = drmru->rc_next;
	unhash(dr);
	drmru = dr;

	newname = (char *)exmalloc(strlen(anc_name) + 1);
	(void) strcpy(newname, anc_name);

	dr->rc_addr = addr;
	dr->rc_dev = dev;
	dr->rc_ino = ino;
	dr->rc_gen = gen;
	dr->rc_ancname = newname;

	dr->rc_chain = drhashtbl[((addr.s_addr ^ (int) dev ^ (int) ino ^ (int) gen) % drhashszminus1)];
	drhashtbl[((addr.s_addr ^ (int)dev ^ (int)ino ^ (int) gen) % drhashszminus1)] = dr;
#ifdef DEBUG
	print_dupcache();
#endif DEBUG
}


/*
 * returns a pointer to the dup req cache entry if it exists
 */
struct dupreq *
dupcache_check(addr,dev,ino,gen)
	struct in_addr addr;
	dev_t  dev;
	ino_t ino;	
	uint_t	gen;	
{
	register struct dupreq *dr;
#ifdef DEBUG
	(void) fprintf(stderr,"CHECK DUP CACHE for addr:%x dev:%x ino:%d %d\n",
			addr.s_addr,dev,ino,gen);
#endif DEBUG
	dr = KEYTOLIST(addr.s_addr,dev,ino,gen); 
	while (dr != NULL) { 
		if (NOTDUP(dr,addr.s_addr,dev,ino,gen)) {
			dr = dr->rc_chain;
			continue;
		}
#ifdef DEBUG
		(void)fprintf(stderr,"\t Got it!\n");
#endif DEBUG
		return(dr);
	}
#ifdef DEBUG
	(void)fprintf(stderr,"\t Nope\n");
#endif DEBUG
	return((struct dupreq *) 0);
}

static
unhash(dr)
	struct dupreq *dr;
{
	struct dupreq *drt;
	struct dupreq *drtprev = NULL;
	 
	drt = REQTOLIST(dr); 
	while (drt != NULL) { 
		if (drt == dr) { 
			if (drtprev == NULL) {
				REQTOLIST(dr) = drt->rc_chain;
			} else {
				drtprev->rc_chain = drt->rc_chain;
			}
			free(dr->rc_ancname);
			return; 
		}	
		drtprev = drt;
		drt = drt->rc_chain;
	}	
}

#ifdef DEBUG
print_dupcache()
{
	struct dupreq *dr, **dt;
	int i;

	(void) fprintf(stderr, "DUP REQS CACHE begin: \n");
	dt = drhashtbl;
	for (i=0; i < drhashsz; i++) {
		(void)fprintf(stderr, "hash list[%d]\n", i);
		for (dr = dt[i]; dr != NULL; dr = dr->rc_chain) {
		    (void)fprintf(stderr,"\t addr:%x dev:%x ino:%d %d anc:%s\n",
			dr->rc_addr, dr->rc_dev, dr->rc_ino, dr->rc_gen,
			dr->rc_ancname);
		}
	}
	(void) fprintf(stderr, "DUP REQS CACHE end.\n");
}
#endif DEBUG

/*
 * Input name in raw, canonicalized pathname output to canon.  If dosymlinks
 * is nonzero, resolves all symbolic links encountered during canonicalization
 * into an equivalent symlink-free form.  Returns 0 on success, -1 on failure.
 *
 * Sets errno on failure.
 */
int
pathcanon(raw, canon, dosymlinks)
    char	*raw,
		*canon;
    int		dosymlinks;
{
    register char	*s,
			*d;
    register char	*limit = canon + MAXPATHLEN;
    char		*modcanon;
    int			nlink = 0;

    /*
     * Make sure that none of the operations overflow the corresponding buffers.
     * The code below does the copy operations by hand so that it can easily
     * keep track of whether overflow is about to occur.
     */
    s = raw;
    d = canon;
    modcanon = canon;

    while (d < limit && *s)
	*d++ = *s++;

    /* Add a trailing slash to simplify the code below. */
    s = "/";
    while (d < limit && (*d++ = *s++))
	continue;
	
    /*
     * Canonicalize the path.  The strategy is to update in place, with
     * d pointing to the end of the canonicalized portion and s to the
     * current spot from which we're copying.  This works because
     * canonicalization doesn't increase path length, except as discussed
     * below.  Note also that the path has had a slash added at its end.
     * This greatly simplifies the treatment of boundary conditions.
     */
    d = s = modcanon;
    while (d < limit && *s) {
	if ((*d++ = *s++) == '/' && d > canon + 1) {
	    register char  *t = d - 2;

	    switch (*t) {
	    case '/':
		/* Found // in the name. */
		d--;
		continue;
	    case '.': 
		switch (*--t) {
		case '/':
		    /* Found /./ in the name. */
		    d -= 2;
		    continue;
		case '.': 
		    if (*--t == '/') {
			/* Found /../ in the name. */
			while (t > canon && *--t != '/')
			    continue;
			d = t + 1;
		    }
		    continue;
		default:
		    break;
		}
		break;
	    default:
		break;
	    }
	    /*
	     * We're at the end of a component.  If dosymlinks is set
	     * see whether the component is a symbolic link.  If so,
	     * replace it by its contents.
	     */
	    if (dosymlinks) {
		char		link[MAXPATHLEN + 1];
		register int	llen;

		/*
		 * See whether it's a symlink by trying to read it.
		 *
		 * Start by isolating it.
		 */
		*(d - 1) = '\0';
		if ((llen = readlink(canon, link, sizeof link)) >= 0) {
		    /* Make sure that there are no circular links. */
		    nlink++;
		    if (nlink > MAXSYMLINKS) {
			errno = ELOOP;
			return (-1);
		    }
		    /*
		     * The component is a symlink.  Since its value can be
		     * of arbitrary size, we can't continue copying in place.
		     * Instead, form the new path suffix in the link buffer
		     * and then copy it back to its proper spot in canon.
		     */
		    t = link + llen;
		    *t++ = '/';
		    /*
		     * Copy the remaining unresolved portion to the end
		     * of the symlink. If the sum of the unresolved part and
		     * the readlink exceeds MAXPATHLEN, the extra bytes
		     * will be dropped off. Too bad!
		     */
		    (void) strncpy(t, s, sizeof link - llen - 1);
		    link[sizeof link - 1] = '\0';
		    /*
		     * If the link's contents are absolute, copy it back
		     * to the start of canon, otherwise to the beginning of
		     * the link's position in the path.
		     */
		    if (link[0] == '/') {
			/* Absolute. */
			(void) strcpy(canon, link);
			d = s = canon;
		    }
		    else {
			/*
			 * Relative: find beginning of component and copy.
			 */
			--d;
			while (d > canon && *--d != '/')
			    continue;
			s = ++d;
			/*
			 * If the sum of the resolved part, the readlink
			 * and the remaining unresolved part exceeds
			 * MAXPATHLEN, the extra bytes will be dropped off.
			*/
			if (strlen(link) >= (limit - s)) {
				(void) strncpy(s, link, limit - s);
				*(limit - 1) = '\0';
			} else {
				(void) strcpy(s, link);
			}
		    }
		    continue;
		} else {
		   /*
		    * readlink call failed. It can be because it was
		    * not a link (i.e. a file, dir etc.) or because the
		    * the call actually failed.
		    */
		    if (errno != EINVAL)
			return (-1);
		    *(d - 1) = '/';	/* Restore it */
		}
	    } /* if (dosymlinks) */
	}
    } /* while */

    /* Remove the trailing slash that was added above. */
    if (*(d - 1) == '/' && d > canon + 1)
	    d--;
    *d = '\0';
    return (0);
}

/*
 * Canonicalize the path given in raw, resolving away all symbolic link
 * components.  Store the result into the buffer named by canon, which
 * must be long enough (MAXPATHLEN bytes will suffice).  Returns NULL
 * on failure and canon on success.
 *
 * The routine indirectly invokes the readlink() system call 
 * so it inherits the possibility of hanging due to inaccessible file 
 * system resources.
 */
char *
realpath(raw, canon)
    char	*raw;
    char	*canon;
{
    return (pathcanon(raw, canon, 1) < 0 ? NULL : canon);
}

/*
 * This function is called via SIGTERM when the system is going down.
 * It sends a broadcast MOUNTPROC_UMNTALL.  Stupid since 1) it assumes
 * clients will be running mountds too and 2) it assumes that the mountd
 * is only killed when the system is going down but OSF/1 has this...
 */
void
send_umntall()
{
        (void) clnt_broadcast(MOUNTPROG, MOUNTVERS, MOUNTPROC_UMNTALL,
                xdr_void, (caddr_t)0, xdr_void, (caddr_t)0, (caddr_t)0);
        exit();
}

/* 
 * Force a re-read of exports file on next mtd_mount or mtd_export
 * by making saved /etc/exports modification time older.
 */
void
touch_exports()
{
	struct exportfile_entry *entry;

	if (exportfile_q != NULL) {
		for (entry = exportfile_q; entry != NULL; entry = entry->next) {
			entry->statb.st_mtime--;
		}
	}	
}

/*
 * The routine will insert the filename and stat information on a queue.
 * If the file cannot be "stat"ed, the structure will contain zeroes.
 * It will not be an error.  The queue is expected to be pointing to
 * NULL or a valid queue entry.  The exit (not return) if there is a
 * problem allocating memory.
 */
int
add_exportfile( char *fn )
{
	struct exportfile_entry *last_entry;
	struct exportfile_entry *entry;

	/*
	 * Check for duplicates.  The routine will exit if a
	 * duplicate is found by returning a 0.  Duplicates
	 * are not reports as an error.
	 */

	if (exportfile_q != NULL) {
		last_entry = exportfile_q;	
		entry = exportfile_q; 
		while (entry != NULL ) {
			if (strcmp(entry->fn,fn) == 0) {
				return(0);
			}
			last_entry = entry;
			entry = entry->next;
		}
	}

	/*
	 * Create an entry.  Note, the stat block is filled
	 * with zeroes if stat error.  This is neccessary
	 * to insure that the modification time for the file
	 * is compared correctly.
	 */

	entry = (struct exportfile_entry *)
		exmalloc(sizeof(struct exportfile_entry));
	entry->next = NULL;
	strcpy(entry->fn,fn);
	if (stat(fn,&entry->statb) < 0) {
		bzero(&entry->statb,sizeof(struct stat));
	}

	/*
	 * Insert entry on queue
	 */

	if (exportfile_q != NULL) {
		last_entry->next = entry;
	} else {
		exportfile_q = entry;
	}
}

/*
 * This routine will free all memory allocated for the exportfile queue
 * and leave the exportfile queue head pointing to NULL.  There are
 * no error return values.
 */
void
free_exportfile()
{
	struct exportfile_entry *entry;

	while (exportfile_q != NULL) {
		entry = exportfile_q;
		exportfile_q = exportfile_q->next;
		free(entry);
	}
}

/*
 * This routine is a debug routine that will print the filenames of all
 * the exportfiles maintained by the current mountd process.  There are
 * no return values.
 */
void
print_exportfile()
{
	struct exportfile_entry *entry;
	
	if (exportfile_q == NULL)
		return;

	for (entry = exportfile_q; entry != NULL; entry = entry->next) {
		fprintf(stderr,"\t%s\n",entry->fn);
	}
}


/*
 * The routine will insert the remote pathname and local pathname on a
 * queue.  This list will provide a mapping between the remote pathname
 * and the local pathname.  The queue is expected to be pointing to NULL
 * or a valid queue entry.  Note, the routine will *exit* (not return) if
 * there is a problem allocating memory.
 */
int
add_remotelocalmap( char *remotepath, char *localpath )
{
	struct remotelocalmap_entry *last_entry;
	struct remotelocalmap_entry *entry;


	/* Check for null pointers */

	if ((remotepath == NULL) || (localpath == NULL))
		return;

	/*
	 * Create an entry.
	 */

	entry = (struct remotelocalmap_entry *)
		exmalloc(sizeof(struct remotelocalmap_entry));
	entry->next = NULL;
	strcpy(entry->remotepath,remotepath);
	strcpy(entry->localpath,localpath);

	/*
	 * Insert entry on queue
	 */

	if (remotelocalmap_q != NULL) {
		remotelocalmap_tail->next = entry;
		remotelocalmap_tail = entry;
	} else {
		remotelocalmap_q = entry;
		remotelocalmap_tail = entry;
	}
}

/*
 * This routine will free all memory allocated for the remote to local 
 * mapping queue.  The head pointer will be pointing to NULL.  There are
 * no return values.
 */
void
free_remotelocalmap()
{
	struct remotelocalmap_entry *entry;

	while (remotelocalmap_q != NULL) {
		entry = remotelocalmap_q;
		remotelocalmap_q = remotelocalmap_q->next;
		free(entry);
	}
	remotelocalmap_tail = NULL;
}

/*
 * Added for the -m option, this routine will return the local pathname
 * for the remote pathname by searching the remote-local mapping queue.
 * If a valid entry is found the local pathname will be returned.
 * Otherwise, the remote pathname will be returned.  This routine 
 * provides the complement functionality of get_remotepath().
 */
char *
get_localpath(char *remotepath)
{
	struct remotelocalmap_entry *entry;

	if (remotelocalmap_q == NULL)
		return(remotepath);

	for (entry = remotelocalmap_q; entry != NULL; entry = entry->next) {
		/*
		 * erlen is length of this entry's remote path.
		 */
		int erlen = strlen(entry->remotepath);
		/*
		 * Does the remote path passed match the entry remote path at 
		 * least up to the length of the entry remote path.
		 */
		if (strncmp(entry->remotepath,remotepath, erlen) == 0) {
			/*
			 * rlen is length of passed remote path.
			 */
			int rlen = strlen(remotepath);
			/*
			 * If passed remote path is longer than entry 
			 * remote path.
			 */
			if (rlen > erlen) {
				/*
				 * Copy the entry local path into the scratch
				 * char array and then concatenate the
				 * remainder of the passed path to that.
				 * Return the string we just put together.
				 */
				strcpy(pathscratch, entry->localpath);
				strcat(pathscratch+strlen(pathscratch), 
					remotepath+erlen);
				return(pathscratch);
			/*
			 * rlen has to be equal to erlen.
			 * Just return entry localpath.
			 */
			} else
				return(entry->localpath);
		}
	}

	return(remotepath);
}

/*
 * Added for the -m option, this routine will return the remote pathname
 * for the local pathname by searching the remote-local mapping queue.
 * If a valid entry is found the remote pathname will be returned.
 * Otherwise, the local pathname will be returned.  This routine provides
 * the complement functionality of get_localpath().
 */
char *
get_remotepath(char *localpath)
{
	struct remotelocalmap_entry *entry;

	if (remotelocalmap_q == NULL)
		return(localpath);

	for (entry = remotelocalmap_q; entry != NULL; entry = entry->next) {
		if (strcmp(entry->localpath,localpath) == 0) {
			return(entry->remotepath);
		}
	}
	
	return(localpath);
}

/*
/*
 * This routine is a debug routine that will print the local and remote
 * pathnames maintained by the current mountd process.  There are
 * no return values.
 */
void
print_remotelocalmap()
{
	struct remotelocalmap_entry *entry;
	
	if (remotelocalmap_q == NULL)
		return;
	
	for (entry = remotelocalmap_q; entry != NULL; entry = entry->next) {
		fprintf(stderr,"\tremotepath: %s\n",entry->remotepath);
		fprintf(stderr,"\tlocalpath: %s\n",entry->localpath);
		fprintf(stderr,"\n");
	}
}

