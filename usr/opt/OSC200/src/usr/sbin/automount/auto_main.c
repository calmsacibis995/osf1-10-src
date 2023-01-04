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
#ifdef ultrix
static char     *sccsid = "%W%  ULTRIX  %G%";
#else /* ultrix */
static char     *sccsid = "@(#)$RCSfile: auto_main.c,v $ $Revision: 4.2.13.5 $ (DEC) $Date: 1993/12/21 20:39:47 $";
#endif /* ultrix */
#endif /* lint */

/*
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 */

/*
 *	Modification History:
 * 
 *	06 Sep 91 -- condylis
 *		Added -h option to include hostname in mount point name.
 *
 *	17 Apr 90 -- condylis
 *		Fixed handling of SIGTERM in catch().  Added changing
 *		of modification time to insure umount succeeds.
 *
 *	06 Nov 91 -- condylis
 *		noconn now is default mount option.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <syslog.h>
#include <errno.h>
#define _SOCKADDR_LEN
#include <rpc/rpc.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include "nfs_prot.h"
#include <netinet/in.h>
#include <netdb.h>
#include <sys/signal.h>
#include <sys/file.h>
#include <rpcsvc/yp_prot.h>
#include <rpcsvc/ypclnt.h>
#ifdef ultrix			/* JFS */
#define KERNEL                  /* suppress gt_names in include */
#include <sys/fs_types.h>
#undef KERNEL
typedef quad fsid_t;		/* file system id type */
#define do_SIGINT
#include "auto_ultrix.h"
#include <nfs/nfs_gfs.h>
#endif /* ultrix */
#define NFSCLIENT
#include <sys/param.h>
#include <sys/mount.h>
#include "automount.h"

extern errno;

#ifdef do_SIGINT
	char **saved_argv;
        char *saved_path;

        void catch1();
        void rerun();
        char * program_run_from();
#endif /* do_SIGINT */
void catch();
void reap();
void set_timeout();
void loadmaster_file();
void loadmaster_yp();

#define	MASTER_MAPNAME	"auto.master"
#define    INADDR_LOOPBACK         (u_int)0x7F000001

int maxwait = 60;
int mount_timeout = 30;
int max_link_time = 5*60;
int verbose, syntaxok, host_in_mntpnt_name;
dev_t tmpdev;
int yp;

u_short myport;

int child;

main(argc, argv)
	int argc;
	char *argv[];
{
	SVCXPRT *transp;
	extern void nfs_program_2();
	extern void check_mtab();
	static struct sockaddr_in sin;	/* static causes zero init */
	struct nfs_args args;
	struct autodir *dir, *dir_next;
	int pid;
	int bad;
	int master_yp = 1;
	char *master_file;
	struct hostent *hp;
        struct stat stbuf;
	extern int trace;
	char pidbuf[64];

	if (geteuid() != 0) {
		fprintf(stderr, "Must be root to use automount\n");
		exit(1);
                /*NOTREACHED*/

	}

#ifdef do_SIGINT
        saved_argv = argv;
        saved_path = program_run_from(argv[0]);
#endif /* do_SIGINT */

	argc--;
	argv++;

#ifdef  ultrix
        openlog("automount", LOG_PID);
#else   /* ultrix */
	openlog("automount", LOG_PID | LOG_NOWAIT | LOG_CONS, LOG_DAEMON);
#endif  /* ultrix */

	(void) setbuf(stdout, (char *)NULL);
	local_init();		/* Set up self, my_addr, name/address list */
	(void) getdomainname(mydomain, sizeof mydomain);
	(void) strcpy(tmpdir, "/tmp_mnt");
	master_file = NULL;
	syntaxok = 1;	/* OK to log map syntax errs to console */

	while (argc && argv[0][0] == '-') switch (argv[0][1]) {
	case 'n':
		nomounts++;
		argc--;
		argv++;
		break;
	case 'm':
		master_yp = 0;
		argc--;
		argv++;
		break;
	case 'f':
		if (argc < 2) {
			usage_arg();
                        /*NOTREACHED*/
		}
		master_file = argv[1];
		argc -= 2;
		argv += 2;
		break;
	case 'M':
		if (argc < 2) {
			usage_arg();
                        /*NOTREACHED*/
		}
		(void) strcpy(tmpdir, argv[1]);
		argc -= 2;
		argv += 2;
		break;
	case 't':			/* timeouts */
		if (argc < 2) {
			(void) fprintf(stderr, "Bad timeout value\n");
			usage();
                        /*NOTREACHED*/
		}
		if (argv[0][2]) {
			set_timeout(argv[0][2], atoi(argv[1]));
		} else {
			char *s;

			for (s = strtok(argv[1], ","); s ;
				s = strtok(NULL, ",")) {
				if (*(s+1) != '=') {
					(void) fprintf(stderr,
						"Bad timeout value\n");
					usage();
                                        /*NOTREACHED*/
				}
				set_timeout(*s, atoi(s+2));
			}
		}
		argc -= 2;
		argv += 2;
		break;

	case 'T':                /* -T gives one level of trace,
				    -T -T gives more detail */
		trace++;
		argc--;
		argv++;
		break;

	case 'D':
		if (argv[0][2])
			(void) putenv(&argv[0][2]);
		else {
			if (argc < 2) {
				usage_arg();
                                /*NOTREACHED*/
			}
			(void) putenv(argv[1]);
			argc--;
			argv++;
		}
		argc--;
		argv++;
		break;

	case 'v':
		verbose++;
		argc--;
		argv++;
		break;

	case 'h':
		host_in_mntpnt_name++;
		argc--;
		argv++;
		break;

	default:
		usage();
                /*NOTREACHED*/
	}

	if (verbose && argc == 0 && master_yp == 0 && master_file == NULL) {
		syslog(LOG_ERR, "no mount maps specified");
		usage();
                /*NOTREACHED*/
	}

	yp = 1;
	if (bad = yp_bind(mydomain)) {
		if (verbose)
			syslog(LOG_ERR, "NIS bind failed: %s",
				yperr_string(bad));
		yp = 0;
	}

	check_mtab();
	/*
	 * Get mountpoints and maps off the command line
	 */
	while (argc >= 2) {
		if (argc >= 3 && argv[2][0] == '-') {
			dirinit(argv[0], argv[1], argv[2]+1, 0);
			argc -= 3;
			argv += 3;
		} else {
			dirinit(argv[0], argv[1], "rw", 0);
			argc -= 2;
			argv += 2;
		}
	}
	if (argc) {
		usage();
		/*NOTREACHED*/
	}

	if (getenv("ARCH") == NULL) {
		char buf[16], str[32];
		int len;
		FILE *f;

		f = popen("machine", "r");
		(void) fgets(buf, 16, f);
		(void) pclose(f);
		if (len = strlen(buf))
			buf[len - 1] = '\0';
		(void) sprintf(str, "ARCH=%s", buf);
		(void) putenv(str);
	}
	
	if (master_file) {
		loadmaster_file(master_file);
	}
	if (master_yp) {
		loadmaster_yp(MASTER_MAPNAME);
	}

	/*
	 * Remove -null map entries
	 */
	for (dir = HEAD(struct autodir, dir_q); dir; dir = dir_next) {
	    	dir_next = NEXT(struct autodir, dir);
		if (strcmp(dir->dir_map, "-null") == 0) {
			REMQUE(dir_q, dir);
		}
	}
	if (HEAD(struct autodir, dir_q) == NULL) {   /* any maps ? */
		syslog(LOG_ERR, "No file systems to serve");
		exit(1);
                /*NOTREACHED*/
	}
	transp = svcudp_create(RPC_ANYSOCK);
	if (transp == NULL) {
		syslog(LOG_ERR, "Cannot create UDP service");
		exit(1);
                /*NOTREACHED*/
	}
	if (!svc_register(transp, NFS_PROGRAM, NFS_VERSION, nfs_program_2, 0)) {
		syslog(LOG_ERR, "svc_register failed");
		exit(1);
                /*NOTREACHED*/
	}
	if (mkdir_r(tmpdir) < 0) {
		syslog(LOG_ERR, "couldn't create %s: %m", tmpdir);
		exit(1);
                /*NOTREACHED*/
	}
	if (stat(tmpdir, &stbuf) < 0) {
		syslog(LOG_ERR, "couldn't stat %s: %m", tmpdir);
		exit(1);
                /*NOTREACHED*/
	}
	tmpdev = stbuf.st_dev;

#ifdef DEBUG
	pid = getpid();
	if (fork()) {
		/* parent */
		signal(SIGTERM, catch);
		signal(SIGHUP,  check_mtab);
		signal(SIGCHLD, reap);
		auto_run();
		syslog(LOG_ERR, "svc_run returned");
		exit(1);
                /*NOTREACHED*/
	}
#else NODEBUG
	switch (pid = fork()) {
	case -1:
		syslog(LOG_ERR, "Cannot fork: %m");
		exit(1);
                /*NOTREACHED*/
	case 0:
		/* child */
		{ int tt = open("/dev/tty", O_RDWR);
		  if (tt > 0) {
			(void) ioctl(tt, TIOCNOTTY, (char *)0);
			(void) close(tt);
		  }
		}
#ifdef do_SIGINT
                signal(SIGINT, rerun);
		signal(SIGTERM, catch1);
#else /* do_SIGINT */
		signal(SIGTERM, catch);
#endif /* do_SIGINT */
		signal(SIGHUP, check_mtab);
		signal(SIGCHLD, reap);
		if (verbose)
			syslog(LOG_ERR, "NFS server starting");
		auto_run();
		syslog(LOG_ERR, "svc_run returned");
		exit(1);
                /*NOTREACHED*/
	}
#endif

	/* parent */
	sin.sin_family = AF_INET;
	sin.sin_port = htons(transp->xp_port);
	myport = transp->xp_port;
	sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	bzero(sin.sin_zero, sizeof(sin.sin_zero));
	args.addr = &sin;
	args.flags = NFSMNT_INT + NFSMNT_TIMEO +
		     NFSMNT_HOSTNAME + NFSMNT_RETRANS + NFSMNT_NOAC +
		     NFSMNT_NOCONN + NFSMNT_AUTO;    
	args.timeo = (mount_timeout + 5) * 10;
	args.retrans = 5;
	bad = 1;

	/*
	 * Mount the daemon at its mount points.
	 * Start at the end of the list because that's
	 * the first on the command line.
	 */
	for (dir = TAIL(struct autodir, dir_q); dir;
	    dir = PREV(struct autodir, dir)) {
		(void) sprintf(pidbuf, "%s:(pid%d)", self, pid);
		if (strlen(pidbuf) >= MAXHOSTNAMELEN-1)
			(void) strcpy(&pidbuf[MAXHOSTNAMELEN-3], "..");
		args.hostname = pidbuf;
		args.fh = (nfsv2fh_t *)&dir->dir_vnode.vn_fh; 
		if (sys_mount(self, dir->dir_truename, M_RDONLY, &args, NULL)) {
			syslog(LOG_ERR, "Can't mount %s: %m", dir->dir_truename);
		} else {
			/* domntent(pid, dir); */
			bad = 0;
		}
	}
	if (bad)		/* No mount succeeded? */
		(void) kill(pid, SIGKILL);
	exit(bad);
	/*NOTREACHED*/
}

void
set_timeout(letter, t)
	char letter ; int t;
{
	if (t <= 1) {
		(void) fprintf(stderr, "Bad timeout value\n");
		usage();
                /*NOTREACHED*/
	}
	switch (letter) {
	case 'm':
		mount_timeout = t;
		break;
	case 'l':
		max_link_time = t;
		break;
	case 'w':
		maxwait = t;
		break;
	default:
		(void) fprintf(stderr, "automount: bad timeout switch\n");
		usage();
                /*NOTREACHED*/
		break;
	}
}


void
catch()
{
	struct autodir *dir;
	struct filsys *fs, *fs_next;
	struct stat stbuf;
	struct fattr *fa;
	int count;
        union wait *status;

	/*
	 *  The automounter has received a SIGTERM.
	 *  Here it forks a child to carry on servicing
	 *  its mount points in order that those
	 *  mount points can be unmounted.  The child
	 *  checks for symlink mount points and changes them
	 *  into directories to prevent the unmount system
	 *  call from following them.  The struct link pointer
	 *  in the vnode is overwritten with an autodir pointer,
	 *  thereby losing the struct link, but inasmuch as we're
	 *  shutting down anyway, no big deal.
	 */
	signal(SIGTERM, SIG_IGN);
	if (verbose)
		syslog(LOG_ERR, "Caught SIGTERM, shutting down");
        if (trace > 1)
		(void) fprintf(stderr, "Caught SIGTERM, shutting down\n");
	if ((child = fork()) == 0) {
		for (dir = HEAD(struct autodir, dir_q); dir;
		    dir = NEXT(struct autodir, dir)) {
			if (dir->dir_vnode.vn_type != VN_LINK)
				continue;

			dir->dir_vnode.vn_type = VN_DIR;
			dir->dir_vnode.vn_data = (char *)dir;
			fa = &dir->dir_vnode.vn_fattr;
			fa->type = NFDIR;
			fa->mode = NFSMODE_DIR + 0555;
			fa->mtime.seconds++;
		}
		return;
	}

	for (dir = HEAD(struct autodir, dir_q); dir;
	    dir = NEXT(struct autodir, dir)) {
		/*
		 * If we couldn't mount our intercept, some admin probably has
		 * both us and fstab mounting something there and we better
		 * not confuse him more by unmounting the fstab mount!
		 */
		if (!dir->dir_intercepting)
			continue;
		/*  This lstat is a kludge to force the kernel
		 *  to flush the attr cache.  If it was a direct
		 *  mount point (symlink) the kernel needs to
		 *  do a getattr to find out that it has changed
		 *  back into a directory.
		 */
		/*
		 * From thurlow@convex.com:
		 *
		 * The old code, a simple lstat, wasn't reliably convincing
		 * the kernel that the node had changed, so the kernel
		 * readlink requests would get ESTALE.  The kernel must
		 * eventually time out its attribute cache, so we'll wait
		 * until we know we can unmount properly.
		 */
		do {
			if (trace > 1)
				(void) fprintf(stderr,
					       "catch: About to unmount %s\n",
					       dir->dir_truename);
			if (lstat(dir->dir_truename, &stbuf) < 0) {
				if (trace > 1)
					perror("lstat");
				syslog(LOG_ERR, "lstat %s: %m", dir->dir_truename);
				break;
			}

			if ((stbuf.st_mode & S_IFMT) == S_IFLNK) {
				/*
				 * POSIX implicitly blocks signals in a handler
				 */
#ifdef __stdc__
				 sigunblock(sigmask(SIGALRM));
#endif /* __stdc__ */
				 sleep(1);
				 if (!dir->dir_remove) /* If not ours, might really be link */
					 break;
				 continue;
			} else
				break;
		} while (1);

		/*
		 * Someone else may be trying to access the mount point while we
		 * are trying to umount it.  Take the heavy hammer approach and
		 * try several times before giving up.
		 */
		for (count = 0; count < 10000; count++) {
#ifdef ultrix
                    struct fs_data fsd;
                    if (trace > 1)
        		(void) fprintf(stderr, "catch: issuing getmnt\n");
                    if (getmnt(0, &fsd, 0, NOSTAT_ONE, dir->dir_name) < 1)
                        if (trace > 1)
                            (void) perror ("catch: getmnt failed");
		    if (umount(fsd.fd_req.dev) == 0)
#else  /* ultrix */
		    if (umount(dir->dir_truename, MNT_NOFORCE) == 0)
#endif /* ultrix */
		    {
			if (dir->dir_remove)
			    (void) rmdir(dir->dir_truename);
			break; /* Unmounted okay */
		    }
		    if (errno != EBUSY) { /* Strange error? */
			    syslog(LOG_ERR, "umount %s: %m", dir->dir_truename);
                            if (trace > 1)
                                perror("catch umount");
			    break;
		    }
	        }
            }

	(void) kill (child, SIGKILL);

	/*
	 *  Unmount any mounts done by the automounter
	 */
	for (fs = HEAD(struct filsys, fs_q); fs; fs = fs_next) {
		fs_next = NEXT(struct filsys, fs);
		if (fs->fs_mine && fs == fs->fs_rootfs) {
			if (do_unmount(fs))
				fs_next = HEAD(struct filsys, fs_q);
		}
	}
        (void) wait (status);
#ifdef do_SIGINT
}

void
rerun() {
	signal(SIGINT, SIG_IGN); /* one signal at a time */
        catch();
        if (child == 0)
		return;		/* just continue from child process */
	syslog(LOG_ERR, "exec'ing myself");
        if (trace > 1)
                (void)fprintf(stderr, "rerun: just before execve, child=%d\n",
			      child);
	(void)sigsetmask(~(sigmask(SIGALRM)|sigmask(SIGINT)|sigmask(SIGTERM))
			 & sigblock(0)); /* clear blocking for SIG{ALRM INT TERM} */
#ifdef ultrix
#define AUTOMOUNT "/usr/etc/automount"
#else
#define AUTOMOUNT "/usr/sbin/automount"
#endif
        if (execve((saved_path == NULL) ? AUTOMOUNT : saved_path,
		   saved_argv, NULL) == -1) {
		if (trace>1)
			(void)perror("rerun: cannot exec:");
		syslog(LOG_ERR, "cannot exec: %m");
		exit(1);
	}
}

void
catch1() {
        catch();
        if (child == 0)
		return;		/* just continue from child process */
#endif				/* do_SIGINT */
	syslog(LOG_ERR, "exiting");
	exit(0);
        /*NOTREACHED*/
}

void
reap()
{
	while (wait3((union wait *)0, WNOHANG, (struct rusage *)0) > 0)
		;
}

auto_run()
{
	int read_fds, n;
	time_t time();
	int last;
	struct timeval tv;

	last = time((int *)0);
	tv.tv_sec = maxwait;
	tv.tv_usec = 0;
	for (;;) {
		read_fds = svc_fds;
		n = select(32, &read_fds, (int *)0, (int *)0, &tv);
		time_now = time((int *)0);
		if (n)
			svc_getreq(read_fds);
		if (time_now >= last + maxwait) {
			last = time_now;
			do_timeouts();
		}
	}
}

usage_arg() {
	fprintf(stderr, "White space is required after options that take arguments.\n");
	usage();
        /*NOTREACHED*/
}

usage()
{
	fprintf(stderr, "Usage: automount\n%s%s%s%s%s%s%s%s%s%s%s",
		"\t[-n]\t\t(no mounts)\n",
		"\t[-m]\t\t(ignore NIS auto.master)\n",
		"\t[-T]\t\t(trace nfs requests)\n",
		"\t[-v]\t\t(verbose error msgs)\n",
		"\t[-h]\t\t(insert server host name in mount point names)\n",
		"\t[-tl n]\t\t(mount duration)\n",
		"\t[-tm n]\t\t(attempt interval)\n",
		"\t[-tw n]\t\t(unmount interval)\n",
		"\t[-M dir]\t(temporary mount dir)\n",
		"\t[-D n=s]\t(define env variable)\n",
		"\t[-f file]\t(get mntpnts from file)\n",
		"\t[ dir map [-mntopts] ] ...\n");
	exit(1);
        /*NOTREACHED*/
}

void
loadmaster_yp(mapname)
	char *mapname;
{
	int first, err;
	char *key, *nkey, *val;
	int kl, nkl, vl;
	char dir[100], map[100];
	char *p, *opts;


	if (!yp)
		return;

	first = 1;
	key  = NULL; kl  = 0;
	nkey = NULL; nkl = 0;
	val  = NULL; vl  = 0;

	for (;;) {
		if (first) {
			first = 0;
			err = yp_first(mydomain, mapname, &nkey, &nkl, &val, &vl);
		} else {
			err = yp_next(mydomain, mapname, key, kl, &nkey, &nkl,
				&val, &vl);
		}
		if (err) {
			if (err != YPERR_NOMORE && err != YPERR_MAP)
				syslog(LOG_ERR, "%s: %s",
					mapname, yperr_string(err));
			return;
		}
		if (key)
			FREE(key);
		key = nkey;
		kl = nkl;

		if (kl >= 100 || vl >= 100)
			return;
		if (kl < 2 || vl < 1)
			return;
		if (isspace(*(u_char *)key) || *key == '#')
			return;
		(void) strncpy(dir, key, kl);
		dir[kl] = '\0';
		(void) strncpy(map, val, vl);
		map[vl] = '\0';
		p = map;
		while (*p && !isspace(*(u_char *)p))
			p++;
		opts = "rw";
		if (*p) {
			*p++ = '\0';
			while (*p && isspace(*(u_char *)p))
				p++;
			if (*p == '-')
				opts = p+1;
		}

		dirinit(dir, map, opts, 0);

		FREE(val);
	}
}

void
loadmaster_file(masterfile)
	char *masterfile;
{
	FILE *fp;
	char *line, *dir, *map, *opts;
	extern char *get_line();
	char linebuf[MAXMAPLEN];

	if ((fp = fopen(masterfile, "r")) == NULL) {
		syslog(LOG_ERR, "%s:%m", masterfile);
		return;
	}

	while ((line = get_line(fp, linebuf, sizeof linebuf)) != NULL) {
		dir = line;
		while (*dir && isspace(*(u_char *)dir)) dir++;
		if (*dir == '\0')
			continue;
		map = dir;
		while (*map && !isspace(*(u_char *)map)) map++;
		if (*map)
			*map++ = '\0';
		if (*dir == '+') {
			loadmaster_yp(dir+1);
		} else {
			while (*map && isspace(*(u_char *)map)) map++;
			if (*map == '\0')
				continue;
			opts = map;
			while (*opts && !isspace(*(u_char *)opts)) opts++;
			if (*opts) {
				*opts++ = '\0';
				while (*opts && isspace(*(u_char *)opts)) opts++;
			}
			if (*opts != '-')
				opts = "-rw";
			
			dirinit(dir, map, opts+1, 0);
		}
	}
	(void) fclose(fp);
}

#ifdef do_SIGINT
/*
 * char *program_run_from(char *argc0)
 *
 * This function returns the complete path and file specification
 * for the currently running program.
 *
 * argc0 is the argc[0] value from the main routine.
 *
 * If getcwd or getenv return errors, or no file is found return
 * a null pointer.
 *
 *
 * Trying to find yourself is like trying to bite your own teeth.
 *
 *								-- Alan Watts
 *
 *  Jim Williams 16-Jun-1993
 *      original type-in
 */

#define ROOT "/"		/* root directory name */
#define DIR_SEP "/"		/* directory separator */
#define CURR_DIR "./"		/* refer to current directory */
#define PARENT_DIR "../"	/* refer to parent directory */
#define PATH_SEP ":"		/* token separator in PATH */

#ifdef __STDC__
char           *getcwd(char *buf, int bufsize);	/* get working directory path */

char *
program_run_from(char *argc0)

#else	/* __STDC__ */
char *getcwd(); /* get working directory path */

char *
program_run_from(argc0)
	char *argc0;
#endif	/* __STDC__ */

{
	static char     return_value[MAXPATHLEN]; /* This must persist after
						     return */
	char            working_dir[MAXPATHLEN];
	char           *env_path;
	char           *token_ptr;
	const char     *slash = DIR_SEP;
	int             file_id;

	if (strncmp(ROOT, argc0, sizeof(ROOT) - 1) == 0) /* abs path? */
		return argc0;	/* path was in argc[0] */

	if ((strncmp(CURR_DIR, argc0, sizeof(CURR_DIR) - 1) == 0) || /* rel path? */
	    (strncmp(PARENT_DIR, argc0, sizeof(PARENT_DIR) - 1) == 0)) {
		/* return <curr dir> DIR_SEP argc[0] */
		if ((token_ptr = getcwd(working_dir, sizeof working_dir)) == NULL)
			return NULL;
		sprintf(return_value, "%s%s%s", token_ptr, slash, argc0);
		return return_value;
	}

	/* not a special case, do it the hard way, namely scan the PATH */
	if ((env_path = getenv("PATH")) == NULL)
		return NULL;

	(void) strncpy(working_dir, env_path, MAXPATHLEN); /* copy it because we
							      mung it */

	if (strcmp(working_dir, env_path) != 0)
		return NULL;	/* check for truncation */

	for (token_ptr = strtok(working_dir, PATH_SEP);	/* find each token */
	     token_ptr != NULL;	/* until there aren't more */
	     token_ptr = strtok(NULL, PATH_SEP)) { /* how to get next one */
		/* build <token> DIR_SEP argc[0] */
		sprintf(return_value, "%s%s%s", token_ptr, slash, argc0);
		if ((file_id = open(return_value, O_RDONLY)) < 0) /* try to open it */
			continue; /* failed, we're not there */

		close(file_id);	/* did open it, we've found ourselves */
		return return_value;
	}			/* end of for loop */

	return NULL;		/* Ran out of tokens */
}
#endif
