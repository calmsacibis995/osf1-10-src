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
static char     *sccsid = "@(#)$RCSfile: auto_mount.c,v $ $Revision: 4.2.9.3 $ (DEC) $Date: 1993/10/19 18:09:11 $";
#endif /* ultrix */
#endif /* lint */

/*
 * Copyright (c) 1987 by Sun Microsystems, Inc.
 */

/*
 *      Modification History:
 *
 *	06 Sep 91 -- condylis
 *		Fixed mount call in remount and added conditional inclusion
 *		of hostname in mount point name.
 *
 *	06 Nov 91 -- condylis
 *		noconn is now default mount option.
 *
 */

#include <stdio.h>
#include <ctype.h>
#include <syslog.h>
#include <sys/param.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/wait.h>
#define _SOCKADDR_LEN
#include <sys/socket.h>
#include <sys/signal.h>
#include <rpc/rpc.h>
#include <rpc/pmap_clnt.h>
#include <netdb.h>
#include <errno.h>
#include "nfs_prot.h"
#include <sys/mount.h>
#ifdef ultrix				/* JFS */
#include <nfs/nfs_clnt.h>
#include <nfs/vfs.h>
#include <sys/fs_types.h>
#include <strings.h>
#include "auto_ultrix.h"
#include <nfs/nfs_gfs.h>
#endif /* ultrix */
#define NFSCLIENT
#include "automount.h"
#include <rpcsvc/mount.h>
#include "mntent.h"

#define MAXHOSTS  20

struct mapfs *find_server();
struct filsys *already_mounted();
void addtomtab();
char *inet_ntoa();
extern int verbose, trace, host_in_mntpnt_name;

/*
 * This routine is a challenge, and is preserved as a monument to all authors
 * of four page for loops.  It mounts both individual and hierarchical mounts.
 * 
 * Everything below which refers to the root of a hierarchy also applies to an
 * individual mount.
 *
 * Returns:
 * routine:	NFS_OK if the root of a hierarchy or the individual mount is
 *		available.  If only a subset of a hierarchy was mounted,
 * 		automount makes no attempt to mount them later.  Bug.
 * rootfs	struct filsys of where the root of the hierarchy is mounted.
 *		Used to setup timeout to unmount the hierarchy and to reset
 *		the timer on later references.
 * linkpath	The pathname where the file system really is mounted, i.e.
 *		what to return as the symbolic link.
 *
 * The root of a hierarchy has its own couple of pages of code.  This is used
 * to find if the root is already mounted.  This used to happen if the link has
 * timed out but do_unmount() hasn't been able to unmount the hierarchy yet.
 * The code now preserves link_fs until the unmount does succeed, so there's
 * a good chance that code can be tossed. The search is done by checking each
 * real NFS mount to see if it matches one of the locations for the root.
 *
 * Next, a couple of special cases are handled for individual mounts:
 * > If it's an individual mount for a local filesystem, return just return
 *   the path referenced in the map.
 * > If find_server reported the file system is already mounted elsewhere,
 *   use that path instead of mounting it again.
 *
 * The mount point is created, if necessary, and the file system mounted.  If
 * that was the root, the link path is created and saved in a new filsys struct
 * that nfsmount() created.  For later mounts, pointers to the root are saved
 * to enable code to treat the hierarchy as a unit.  If the mount fails, that
 * server's entry is marked bad, and we look for another server.
 *
 * Several variables are used in rather strange ways and need explanation:
 * prevhost	This tells find_server() the preferred host to use.  Once
 *		we use a host, we try to stick with him.  The reduces the
 *		chance that a crashed server will do us in in case we
 *		have a hierarchical mount with some paths that traverse
 *		several mount points.
 *
 * tfs		This is set by find_server() only for the root mount when it
 *		finds the directory is already mounted.  The variable is
 *		reused later when a NFS mount is done.
 *
 * mapnext	This is used to avoid a goto.  The goto would have been
 *		more clear.  Grumble.
 *
 * rootmfs	Normally this points to the mapfs for the root.  If we failed
 *		to mount the root, then it points to the last of the root
 *		servers.  This allows to come up with rational path names for
 *		mount points and the link path we'll return.
 *
 * return	The value returned is NFS_OK if file systems are mounted or
 *		if any mounts succeeded.  There are still ways to trick this
 *		code into attempting mounts when the mount have already been
 *		done.  If no mounts succeed, then the status of the last
 *		mount attempt is returned.
 */
nfsstat
do_mount(dir, me, rootfs, linkpath)
	struct autodir *dir;
	struct mapent *me;
	struct filsys **rootfs;
	char **linkpath;
{
	char mntpnt[MAXPATHLEN];
	static char linkbuf[MAXPATHLEN];
	enum clnt_stat pingmount();
	struct filsys *fs, *tfs;
	struct mapfs *mfs, *rmfs, *rootmfs;
	struct mapent *m, *mapnext;
	nfsstat status = NFSERR_NOENT;
	struct in_addr prevhost;
	int imadeit;		/* I made it; not I'm a deit(y)! */
	struct stat stbuf;
	extern dev_t tmpdev;

	*rootfs = NULL;
	*linkpath = "";
	prevhost.s_addr = (u_int)0;

        for (m = me ; m ; m = mapnext) {
                mapnext = m->map_next;

                tfs = NULL;
                mfs = find_server(m, &tfs, m == me, prevhost);
                if (mfs == NULL)
                        continue;
		if (m == me) {	/* If this is the root of a map */
                        for (fs = HEAD(struct filsys, fs_q); fs;
                                fs = NEXT(struct filsys, fs)) {
				for (rmfs = me->map_fs; rmfs; rmfs = rmfs->mfs_next) {
					/* 
					 * Put together mount point name
					 */
					if (host_in_mntpnt_name)
						(void) sprintf(mntpnt, 
						"%s/%s%s%s%s", 
						tmpdir, rmfs->mfs_host,
						dir->dir_name, 
						me->map_root, me->map_mntpnt);
					else
						(void) sprintf(mntpnt, 
						"%s%s%s%s", 
						tmpdir, dir->dir_name, 
						me->map_root, me->map_mntpnt);
                                	if (strcmp(mntpnt, fs->fs_mntpnt) == 0) {
						/*
						 * Found mounted file system
						 * with mount point name we
						 * would have used.
						 */
						if (host_in_mntpnt_name)
                                        		(void) sprintf(linkbuf, 
							"%s/%s%s%s%s",
                                                	tmpdir,
							rmfs->mfs_host,
                                                	dir->dir_name,
                                                	me->map_root,
                                                	me->map_fs->mfs_subdir);
						else
                                        		(void) sprintf(linkbuf, 
							"%s%s%s%s",
                                                	tmpdir,
                                                	dir->dir_name,
                                                	me->map_root,
                                                	me->map_fs->mfs_subdir);
                                        	if (trace > 1)
                                                	(void) fprintf(stderr,
                                                	"renew link for %s\n",
                                                	linkbuf);
                                        	*linkpath = linkbuf;
						*rootfs = fs;
                                        	return (NFS_OK);
                                	}
				}
                        }
			rootmfs = mfs;
			if (host_in_mntpnt_name)
				(void) sprintf(mntpnt, "%s/%s%s%s%s", tmpdir,
                        	mfs->mfs_host, dir->dir_name, me->map_root, 
				m->map_mntpnt);
			else
				(void) sprintf(mntpnt, "%s%s%s%s", tmpdir,
                        	dir->dir_name, me->map_root, 
				m->map_mntpnt);
		} else {	/* else this is a subdir of hierarchy */
			if (host_in_mntpnt_name)
				(void) sprintf(mntpnt, "%s/%s%s%s%s", tmpdir,
				rootmfs->mfs_host, dir->dir_name, me->map_root, 
				m->map_mntpnt);
			else
				(void) sprintf(mntpnt, "%s%s%s%s", tmpdir,
				dir->dir_name, me->map_root, 
				m->map_mntpnt);
		}		/* (m == me) */

                /*
                 * It may be possible to return a symlink
                 * to an existing mount point without
                 * actually having to do a mount.
                 */
                if (me->map_next == NULL && *me->map_mntpnt == '\0') {

                        /* Is it my host ? */
                        if (mfs->mfs_addr.s_addr == my_addr.s_addr ||
			    mfs->mfs_addr.s_addr == htonl(INADDR_LOOPBACK)) {
                                (void) strcpy(linkbuf, mfs->mfs_dir);
                                (void) strcat(linkbuf, mfs->mfs_subdir);
                                *linkpath = linkbuf;
                                if (trace > 1)
                                        (void) fprintf(stderr,
                                                "It's on my host\n");
				/*
				 * Bug here.  We don't track local mounts,
				 * so can't return a rootfs.  This means
				 * link caching doesn't work on these.
				 */
                                return (NFS_OK);
                        }

                        /*
                         * An existing mount point ?
                         * XXX Note: this is a bit risky - the
                         * mount may belong to another automount
                         * daemon - it could unmount it anytime and
                         * this symlink would then point to an empty
                         * or non-existent mount point.
                         */
                        if (tfs != NULL) {
                                if (trace > 1)
                                        (void) fprintf(stderr,
                                        "already mounted %s:%s on %s (%s)\n",
                                        tfs->fs_host, tfs->fs_dir,
                                        tfs->fs_mntpnt, tfs->fs_opts);

                                (void) strcpy(linkbuf, tfs->fs_mntpnt);
                                (void) strcat(linkbuf, mfs->mfs_subdir);
                                *linkpath = linkbuf;
                                *rootfs = tfs;
                                return (NFS_OK);
                        }
                }

                if (nomounts)
                        return (NFSERR_PERM);

                 /* Create the mount point if necessary */

                imadeit = 0;
                if (stat(mntpnt, &stbuf) != 0) {
                        if (mkdir_r(mntpnt) == 0) {
                                imadeit = 1;
                                if (stat(mntpnt, &stbuf) < 0) {
                                        syslog(LOG_ERR,
                                        "Couldn't stat created mountpoint %s: %m",
                                        mntpnt);
                                        continue;
                                }
                        } else {
                                if (verbose)
                                        syslog(LOG_ERR,
                                        "Couldn't create mountpoint %s: %m",
                                        mntpnt);
                                if (trace > 1)
                                        (void) fprintf(stderr,
                                        "couldn't create mntpnt %s\n",
                                        mntpnt);
                                continue;
                        }
                }
		if (verbose && *rootfs == NULL && tmpdev != stbuf.st_dev)
			syslog(LOG_ERR, "WARNING: %s already mounted on",
				mntpnt);

                /*  Now do the mount */

                tfs = NULL;
                status = nfsmount(mfs->mfs_host, mfs->mfs_addr, mfs->mfs_dir,
                                mntpnt, m->map_mntopts, &tfs);
                if (status == NFS_OK) {
                        if (*rootfs == NULL) {
                                *rootfs = tfs;
				if (host_in_mntpnt_name)
                                	(void) sprintf(linkbuf, "%s/%s%s%s%s",
                                        tmpdir, rootmfs->mfs_host, 
					dir->dir_name, me->map_root,
                                        rootmfs->mfs_subdir);
				else
                                	(void) sprintf(linkbuf, "%s%s%s%s",
                                        tmpdir, dir->dir_name,
                                        me->map_root,
                                        rootmfs->mfs_subdir);
                                *linkpath = linkbuf;
                        }
                        tfs->fs_rootfs = *rootfs;
                        tfs->fs_mntpntdev = stbuf.st_dev;
                        if (stat(mntpnt, &stbuf) < 0) {
                                syslog(LOG_ERR, "Couldn't stat: %s: %m",
                                        mntpnt);
                        } else {
                                tfs->fs_mountdev = stbuf.st_dev;
                        }
                        prevhost.s_addr = mfs->mfs_addr.s_addr;
                } else {
                        if (imadeit)
                                safe_rmdir(mntpnt);
			mfs->mfs_ignore = 1;
			prevhost.s_addr = (u_long)0;
			mapnext = m;
			continue;  /* try another server */
                }
        }			/* for (each entry in map) */
        if (*rootfs != NULL) {
                return (NFS_OK);
        }
        return (status);


}

/*
 * Routine to find a promising server for a NFS mount.
 *
 * This accepts a mapent pointer and returns the best looking mapfs to
 * try to use.  Best is determined from these preferences:
 *
 * > Local host, if it is a server, or host referenced by "preferred".
 *   The caller, do_mount(), passes the last host we mounted something from
 *   as the preferred host.  We know we can get to it and there's a good
 *   chance we just mounted something on a higher directory.
 *
 * > If this is for the root of a hierarchy and the file system is already
 *   mounted, use that host.
 *   In this case, we'll return the struct filsys that references the
 *   mount.
 *
 * > The first responder within the local network.
 *   Might be worthwhile to check our subnet.  However, if a subnet server
 *   isn't the first responder, would you want to use it?
 *
 * > The first responder of the rest.
 *
 * If the caller can't mount from our suggestion, it will set the mfs_ignore
 * flag and we won't pay attention to it again until next time do_mount() is
 * called.
 */
struct mapfs *
find_server(me, fsp, rootmount, preferred)
	struct mapent *me;
	struct filsys **fsp;
	int rootmount;
	    struct in_addr preferred;
{
	int entrycount, localcount;
	struct mapfs *mfs, *mfs_one;
	struct hostent *hp;
	struct in_addr addrs[MAXHOSTS], addr, *addrp, trymany();


	/*
	 * get addresses & see if any are myself
	 * or were mounted from previously in a
	 * hierarchical mount.
	 */
	entrycount = localcount = 0;
	for (mfs = me->map_fs; mfs; mfs = mfs->mfs_next) {
		if (mfs->mfs_addr.s_addr == 0) {
			hp = gethostbyname(mfs->mfs_host);
			if (hp)
				bcopy(hp->h_addr, (char *)&mfs->mfs_addr,
					hp->h_length);
			else
				syslog(LOG_ERR, "find_server: gethostbyname failed for %s:%s",
				       mfs->mfs_host, mfs->mfs_dir);
		}
		if (mfs->mfs_addr.s_addr && !mfs->mfs_ignore) {
			entrycount++;
			mfs_one = mfs;
			if (mfs->mfs_addr.s_addr == my_addr.s_addr  ||
			    mfs->mfs_addr.s_addr == htonl(INADDR_LOOPBACK) ||
			    mfs->mfs_addr.s_addr == preferred.s_addr) {
				return (mfs);
			}
		}
	}
	if (entrycount == 0)
		return (NULL);

	/* see if any already mounted */
	if (rootmount)
		for (mfs = me->map_fs; mfs; mfs = mfs->mfs_next) {
			if (mfs->mfs_ignore)
				continue;
			if (*fsp = already_mounted(mfs->mfs_host, mfs->mfs_dir,
					me->map_mntopts, mfs->mfs_addr)) 
				return (mfs);
		}

	if (entrycount == 1) 	/* no replication involved */
		return (mfs_one);

	/* try local net */
	addrp = addrs;
	for (mfs = me->map_fs; mfs; mfs = mfs->mfs_next) {
		if (mfs->mfs_ignore)
			continue;
		if (inet_netof(mfs->mfs_addr) == inet_netof(my_addr)) {
			localcount++;
			*addrp++ = mfs->mfs_addr;
		}
	}
	if (localcount > 0) {	/* got some */
		(*addrp).s_addr = 0;	/* terminate list */
		addr = trymany(addrs, mount_timeout / 2);
		if (addr.s_addr) {	/* got one */
			for (mfs = me->map_fs; mfs; mfs = mfs->mfs_next)
			if (addr.s_addr == mfs->mfs_addr.s_addr)
				return (mfs);
		}
	}
	if (entrycount == localcount)
		return (NULL);

	/* now try them all */
	addrp = addrs;
	for (mfs = me->map_fs; mfs; mfs = mfs->mfs_next)
		if (!mfs->mfs_ignore)
			*addrp++ = mfs->mfs_addr;
	(*addrp).s_addr = 0;	/* terminate list */
	addr = trymany(addrs, mount_timeout / 2);
	if (addr.s_addr) {	/* got one */
		for (mfs = me->map_fs; mfs; mfs = mfs->mfs_next)
		if (addr.s_addr == mfs->mfs_addr.s_addr)
				return (mfs);
	}
	return (NULL);
}

/*
 *  If mount table has changed update internal fs info
 */
void
check_mtab()
{
	struct filsys *fs, *fsnext;
	char *index();
	int found;
       	struct hostent *hp;
#ifdef ultrix
	struct v_fs_data tfs;
	struct v_fs_data *buffer = &tfs;
	int start=0;
#else /* ultrix */
	struct statfs *buffer, *stptr;
	int mntsize;
#endif /* ultrix */

	/* reset the present flags */
	for (fs = HEAD(struct filsys, fs_q); fs;
	    fs = NEXT(struct filsys, fs))
		    fs->fs_present = 0;

#ifdef ultrix
	/* now see what's been mounted */
	while ((getmnt(&start, buffer, sizeof(struct fs_data),
		       NOSTAT_MANY, NULL)) > 0) {
		char *tmphost, *tmppath, *p, tmpc;

		if (buffer->fd_fstype != GT_NFS)
			continue;
		p = index(buffer->fd_devname, ':');
		if (p == NULL)
			continue;
		tmpc = *p;
		*p = '\0';
		tmphost = buffer->fd_devname;
		tmppath = p+1;
		if (tmppath[0] != '/') /* Automount point? */
			continue;
		found = 0;
		for (fs = HEAD(struct filsys, fs_q); fs;
			fs = NEXT(struct filsys, fs)) {
			if (strcmp(buffer->fd_path, fs->fs_mntpnt) == 0 &&
			    strcmp(tmphost, fs->fs_host) == 0 &&
			    strcmp(tmppath, fs->fs_dir) == 0 &&
			    (fs->fs_mine ||
			     (buffer->fd_un.gvfs.vfs.vfs_flag == fs->fs_mflags)))
#else /* ultrix */
	if ((mntsize = getmntinfo(&stptr, MNT_NOWAIT)) == 0) {
		syslog(LOG_ERR, "check_mtab: getmntinfo failed");
		return;
	}
	/* now see what's been mounted */
	for (buffer=stptr; buffer<&stptr[mntsize]; buffer++) {
		char *tmphost, *tmppath, *p, tmpc;
		if (buffer->f_type != MOUNT_NFS)
			continue;
		p = index(buffer->f_mntfromname, ':');
		if (p == NULL)
			continue;
		tmpc = *p;
		*p = '\0';
		tmphost = buffer->f_mntfromname;
		tmppath = p+1;
		if (tmppath[0] != '/') /* Automount point? */
			continue;
		found = 0;
		for (fs = HEAD(struct filsys, fs_q); fs;
			fs = NEXT(struct filsys, fs)) {
			if (strcmp(buffer->f_mntonname, fs->fs_mntpnt) == 0 &&
			    strcmp(tmphost, fs->fs_host) == 0 &&
			    strcmp(tmppath, fs->fs_dir) == 0 &&
			    (fs->fs_mine ||
				same_bopts(buffer->f_flags, fs->fs_mflags,
					   &buffer->mount_info,
					   &fs->fs_nfsargs)))
#endif /* ultrix */
			{
				fs->fs_present = 1;
				found++;
				break;
			}
		}
		if (!found) {
			fs = alloc_fs(tmphost, tmppath,
#ifdef ultrix
				      buffer->fd_path,
				      buffer->fd_un.gvfs.mi.mi_optstr
#else /* ultrix */
				      buffer->f_mntonname,
				      &buffer->mount_info.nfs_args,
				      buffer->f_flags
#endif /* ultrix */
				      );
			if (fs == NULL)
				break;
			fs->fs_present = 1;
			hp = gethostbyname(tmphost);
			if (hp != NULL) {
				bcopy(hp->h_addr, &fs->fs_addr.sin_addr, hp->h_length);
			}
		}
		*p = tmpc;
	}

	/* free fs's that are no longer present */
	for (fs = HEAD(struct filsys, fs_q); fs; fs = fsnext) {
		fsnext = NEXT(struct filsys, fs);
		if (!fs->fs_present) {
			if (fs->fs_mine)
				syslog(LOG_ERR,
					"%s:%s no longer mounted",
					fs->fs_host, fs->fs_dir);
			if (trace > 1)
				(void) fprintf(stderr,
					"%s:%s no longer mounted\n",
					fs->fs_host, fs->fs_dir);
			flush_links(fs);
			free_filsys(fs);
		}
	}
}

/*
 * Search the mount table to see if the given file system is already
 * mounted. 
 */
struct filsys *
already_mounted(host, fsname, opts, hostaddr)
	char *host;
	char *fsname;
	char *opts;
	struct in_addr hostaddr;
{
	struct filsys *fs;
	struct mntent m1;
	struct autodir *dir;
	int mydir;
	extern int verbose;

	check_mtab();
	m1.mnt_opts = opts;
	for (fs = HEAD(struct filsys, fs_q); fs; fs = NEXT(struct filsys, fs)){
		if (strcmp(fsname, fs->fs_dir) != 0)
			continue;
		if (hostaddr.s_addr != fs->fs_addr.sin_addr.s_addr)
			continue;

		/*
		 * Check it's not on one of my mount points.
		 * I might be mounted on top of a previous 
		 * mount of the same file system.
		 */

		for (mydir = 0, dir = HEAD(struct autodir, dir_q); dir;
			dir = NEXT(struct autodir, dir)) {
			if (strcmp(dir->dir_name, fs->fs_mntpnt) == 0) {
				mydir = 1;
				if (verbose)
					syslog(LOG_ERR,
					"%s:%s already mounted on %s",
					host, fsname, fs->fs_mntpnt);
				break;
			}
		}
		if (mydir)
			continue;

		if (same_opts(opts, fs))
			return (fs);
		else 
			continue;
	}
	return(0);
}

/*
 * It seems odd that nfsmount() caches a client handle and this guy
 * doesn't use it.  However, if we did there would be a very good chance
 * that it would be out-of-date by now, so we'd have to destroy and recreate
 * it anyway.  So why does nfsmount() cache the handle?  I guess because
 * it can speed up mounting a hierarchical mount and reduces the time
 * the user has to wait.  We get here at timeout or shutdown time and
 * no one is anxious to see this complete (except the next requestor, if
 * any).
 */
nfsunmount(fs)
	struct filsys *fs;
{
	struct sockaddr_in sin;
	struct timeval timeout;
	CLIENT *cl;
	enum clnt_stat rpc_stat;
	int s;

	sin = fs->fs_addr;
	/*
	 * Port number of "fs->fs_addr" is NFS port number; make port
	 * number 0, so "clntudp_create" finds port number of mount
	 * service.
	 */
	sin.sin_port = 0;
	s = RPC_ANYSOCK;
	timeout.tv_sec = 2; /* retry interval */
	timeout.tv_usec = 0;
	if ((cl = clntudp_create(&sin, MOUNTPROG, 1,
	    timeout, &s)) == NULL) {
		syslog(LOG_WARNING, "%s:%s %s", fs->fs_host, fs->fs_dir,
		    clnt_spcreateerror("server not responding"));
		return;
	}
#ifdef OLDMOUNT
	if (bindresvport(s, NULL))
		syslog(LOG_ERR, "Warning: cannot do local bind");
#endif
	cl->cl_auth = authunix_create_default();
	timeout.tv_sec = 10;
	rpc_stat = clnt_call(cl, MOUNTPROC_UMNT, xdr_path, &fs->fs_dir,
	    xdr_void, (char *)NULL, timeout);
	(void) close(s);
	AUTH_DESTROY(cl->cl_auth);
	clnt_destroy(cl);
	if (rpc_stat != RPC_SUCCESS)
		syslog(LOG_WARNING, "%s", clnt_sperror(cl, "mountd unmount"));
}

enum clnt_stat
pingmount(hostaddr)
	struct in_addr hostaddr;
{
	struct timeval timeout;
	struct sockaddr_in server_addr;
	enum clnt_stat clnt_stat = RPC_TIMEDOUT;
	CLIENT *cl;
	static struct in_addr goodhost, deadhost;
	static time_t goodtime, deadtime;
	int cache_time = 60;  /* sec */
	int sock = RPC_ANYSOCK;

	if (goodtime > time_now && hostaddr.s_addr == goodhost.s_addr)
			return (RPC_SUCCESS);
	if (deadtime > time_now && hostaddr.s_addr == deadhost.s_addr)
			return (RPC_TIMEDOUT);

	if (trace > 1)
		(void) fprintf(stderr, "ping %s ", inet_ntoa(hostaddr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr = hostaddr;
	server_addr.sin_port = htons(NFS_PORT);
	timeout.tv_sec = 2;	/* retry interval */
	timeout.tv_usec = 0;
	cl = clntudp_create(&server_addr, NFS_PROGRAM, NFS_VERSION,
		timeout, &sock);
	if (cl) {
		timeout.tv_sec = 10;
		clnt_stat = clnt_call(cl, NULLPROC, xdr_void, 0, xdr_void, 0,
			timeout);
		clnt_destroy(cl);
	}

	if (clnt_stat == RPC_SUCCESS) {
		goodhost = hostaddr;
		goodtime = time_now + cache_time;
	} else {
		deadhost = hostaddr;
		deadtime = time_now + cache_time;
	}

	if (trace > 1)
		(void) fprintf(stderr, "%s\n", clnt_stat == RPC_SUCCESS ?
			"OK" : "NO RESPONSE");

	return (clnt_stat);
}

struct in_addr gotaddr;

/* ARGSUSED */
catchfirst(raddr)
	struct sockaddr_in *raddr;
{
	gotaddr = raddr->sin_addr;
	return (1);	/* first one ends search */
}

/*
 * ping a bunch of hosts at once and find out who
 * responds first
 */
struct in_addr
trymany(addrs, timeout)
	struct in_addr *addrs;
	int timeout;
{
	enum clnt_stat nfs_cast();
	enum clnt_stat clnt_stat;

	if (trace > 1) {
		register struct in_addr *a;

		(void) fprintf(stderr, "nfs_cast: ");
		for (a = addrs ; a->s_addr ; a++)
			(void) fprintf(stderr, "%s ", inet_ntoa(*a));
		(void) fprintf(stderr, "\n");
	}
		
	gotaddr.s_addr = 0;
	clnt_stat = nfs_cast(addrs, catchfirst, timeout);
	if (trace > 1) {
		(void) fprintf(stderr, "nfs_cast: got %s\n",
			(int) clnt_stat ? "no response" : inet_ntoa(gotaddr));
	}
	if (clnt_stat)
		syslog(LOG_ERR, "trymany: servers not responding: %s",
			clnt_sperrno(clnt_stat));
	return (gotaddr);
}

/*
 * Return 1 if the path s2 is a subdirectory of s1.
 */
subdir(s1, s2)
	char *s1, *s2;
{
	while (*s1 == *s2)
		s1++, s2++;
	return (*s1 == '\0' && *s2 == '/');
}

nfsstat
nfsmount(host, hostaddr, dir, mntpnt, opts, fsp)
	char *host, *dir, *mntpnt, *opts;
	struct in_addr hostaddr;
	struct filsys **fsp;
{
	struct filsys *fs;
	char remname[MAXPATHLEN];
	struct mntent m;
	struct nfs_args args;
	int flags;
	static struct sockaddr_in sin;
	static struct fhstatus fhs;
	static int s = -1;
	struct timeval timeout;
	static CLIENT *cl = NULL;
	static time_t time_valid, deadtime;
	static struct in_addr deadhost;
	int cache_time = 60;  /* sec */
	enum clnt_stat rpc_stat;
	enum clnt_stat pingmount();
	u_short port;
	nfsstat status;
	struct stat stbuf;
	static u_int prev_vers = 0;
	u_int vers;
	char *topts;

	/*
	 * If portmap or mountd just timed out on us, additional NFS requests
	 * have piled up on our socket.  Therefore, we cache such failures to
	 * dispense with subsequent calls quicky.
	 */
	if (deadtime > time_now && hostaddr.s_addr == deadhost.s_addr)
		return (NFSERR_NOENT);

	/* Make sure the mountpoint is safe to mount on */
	if (lstat(mntpnt, &stbuf) < 0) {
		syslog(LOG_ERR, "Couldn't stat %s: %m", mntpnt);
		return (NFSERR_NOENT);
	}
	/* A symbolic link could be dangerous e.g. -> /usr */
	if ((stbuf.st_mode & S_IFMT) == S_IFLNK) {
		if (realpath(mntpnt, remname) == NULL) {
			syslog(LOG_ERR, "%s: realpath failed: %m",
				mntpnt);
			return (NFSERR_NOENT);
		}
		if (!subdir(tmpdir, remname)) {
			syslog(LOG_ERR,
				"%s:%s -> %s : dangerous symbolic link",
				host, dir, remname);
			return (NFSERR_NOENT);
		}
	}

	if (pingmount(hostaddr) != RPC_SUCCESS) {
		syslog(LOG_ERR, "host %s not responding", host);
		return (NFSERR_NOENT);
	}
	(void) sprintf(remname, "%s:%s", host, dir);
	m.mnt_opts = opts;
	vers = MOUNTVERS;
	/*
	 * Get a new client handle if the host is different
	 */
	if (cl == NULL || hostaddr.s_addr != sin.sin_addr.s_addr ||
		vers != prev_vers || time_now > time_valid) {
		prev_vers = vers;
		if (cl) {
			if (cl->cl_auth)
				AUTH_DESTROY(cl->cl_auth);
			clnt_destroy(cl);
		}
		bzero((char *)&sin, sizeof(sin));
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = hostaddr.s_addr;
		timeout.tv_usec = 0;
		timeout.tv_sec = 2;  /* retry interval */
		s = RPC_ANYSOCK;
		if ((cl = clntudp_create(&sin, MOUNTPROG, vers,
		    timeout, &s)) == NULL) {
			syslog(LOG_ERR, "%s: %s", remname,
			clnt_spcreateerror("server's portmap not responding"));
			goto dead;
		}
#ifdef OLDMOUNT
		if (bindresvport(s, NULL))
			syslog(LOG_ERR, "Warning: cannot do local bind");
#endif
		cl->cl_auth = authunix_create_default();
		timeout.tv_sec = 10;
		if (clnt_call(cl, NULLPROC, xdr_void, 0, xdr_void, 0,
			      timeout) != RPC_SUCCESS) {
			syslog(LOG_ERR, "Can't ping mountd at server %s%s",
			       host, clnt_sperror(cl, ""));
			goto dead; /* cl gets destroyed next call */
		}
		time_valid = time_now + cache_time;
	}

	/*
	 * Get fhandle of remote path from server's mountd.
	 */
	timeout.tv_usec = 0;
	timeout.tv_sec = mount_timeout;
	rpc_stat = clnt_call(cl, MOUNTPROC_MNT, xdr_path, &dir,
	    xdr_fhstatus, &fhs, timeout);
	if (rpc_stat != RPC_SUCCESS) {
		/*
		 * Given the way "clnt_sperror" works, the "%s" immediately
		 * following the "not responding" is correct.
		 */
		syslog(LOG_ERR, "%s: mountd not responding%s", remname,
		    clnt_sperror(cl, ""));
		goto dead;
	}

	if (errno = fhs.fhs_status)  {
                if (errno == EACCES) {
			if (verbose)
				syslog(LOG_ERR,
				"can't mount %s: no permission", remname);
			status = NFSERR_ACCES;
                } else {
			syslog(LOG_ERR, "can't mount %s: %m", remname);
			status = NFSERR_IO;
                }
                return (status);
        }        

	/*
	 * set mount args
	 */
	args.fh = (nfsv2fh_t *)&fhs.fhs_fh;
	args.hostname = remname;
	args.flags = 0;
	args.flags |= (NFSMNT_HOSTNAME | NFSMNT_INT | NFSMNT_NOCONN);
	args.addr = &sin;
	m.mnt_opts = opts;
	flags = 0;

	/* ignore sun secure mount option */
	if (hasmntopt(&m, MNTOPT_SECURE) != NULL) {
		syslog(LOG_ERR, "Mount of %s on %s; secure option specified: %m", remname, mntpnt);
		return (NFSERR_IO);
	}

	topts = strdup(opts);
	getstdopts(topts, &flags);
	if (getnfsopts(topts, &args) == 0) {
		syslog(LOG_ERR, "Problem with mount options for %s on %s %s\n", remname, mntpnt, opts);
		FREE(topts);
		return(NFSERR_IO);
	}
	FREE(topts);

	if (port = nopt(&m, "port")) {
		sin.sin_port = htons(port);
	} else {
		sin.sin_port = htons(NFS_PORT);	/* XXX should use portmapper */
	}

	if (trace > 1) {
		(void) fprintf(stderr, "mount %s %s (%s)\n",
			remname, mntpnt, opts);
	}
	if (sys_mount(host, mntpnt, flags, &args, opts)) {
		syslog(LOG_ERR, "Mount of %s on %s: %m", remname, mntpnt);
		return (NFSERR_IO);
	}
	if (trace > 1) {
		(void) fprintf(stderr, "mount %s OK\n", remname);
	}
	if (*fsp)
		fs = *fsp;
	else {
#ifdef ultrix
                fs = alloc_fs(host, dir, mntpnt, opts);
#else
		fs = alloc_fs(host, dir, mntpnt, &args, flags);
#endif /* ultrix */
		if (fs == NULL)
			return (NFSERR_NOSPC);
	}
#ifdef ultrix
        {
                struct fs_data fsd;
                if (getmnt(0, &fsd, 0, NOSTAT_ONE, mntpnt) < 1) {
                        /* Couldn't get dev number; we'll try later */
                        fs->fs_mountdev = -1;
                } else {
                        /* Got it; save it away for the umount      */
                        fs->fs_mountdev = fsd.fd_req.dev;
                }
        }
#endif /* ultrix */

	fs->fs_type = MNTTYPE_NFS;
	fs->fs_mine = 1;
	fs->fs_nfsargs = args;
	fs->fs_mflags = flags;
	fs->fs_nfsargs.hostname = fs->fs_host;
	fs->fs_nfsargs.addr = &fs->fs_addr;
	fs->fs_nfsargs.fh = (nfsv2fh_t *)&fs->fs_rootfh;
	fs->fs_addr = sin;
	bcopy(&fhs.fhs_fh, &fs->fs_rootfh, sizeof fs->fs_rootfh);
        *fsp = fs;
	return (NFS_OK);

dead:
	deadhost = hostaddr;
	deadtime = time_now + 60;
	return (NFSERR_NOENT);
}


nfsstat
remount(fs)
	struct filsys *fs;
{
	char remname[1024];
	struct stat stbuf;

	if (fs->fs_nfsargs.fh == 0) 
		return nfsmount(fs->fs_host, fs->fs_addr.sin_addr, fs->fs_dir,
				fs->fs_mntpnt, fs->fs_opts, &fs);
	(void) sprintf(remname, "%s:%s", fs->fs_host, fs->fs_dir);
	if (trace > 1) {
		(void) fprintf(stderr, "remount %s %s (%s)\n",
			remname, fs->fs_mntpnt, fs->fs_opts);
	}
	if (pingmount(fs->fs_addr.sin_addr) != RPC_SUCCESS) {
		if (verbose || fs->fs_unmounted++ < 5)
			syslog(LOG_ERR,
				"remount %s on %s: server not responding",
				remname, fs->fs_mntpnt);
		if (trace > 1)
			(void) fprintf(stderr, "remount FAILED: server not responding\n");
		return (NFSERR_IO);
	}
	if (stat(fs->fs_mntpnt, &stbuf) < 0) {
		syslog(LOG_ERR, "remount: couldn't stat: %s: %m", fs->fs_mntpnt);
		return (NFSERR_IO);
	}
	fs->fs_nfsargs.hostname = remname;
        if (sys_mount(fs->fs_host, fs->fs_mntpnt, fs->fs_mflags,
		      &fs->fs_nfsargs, fs->fs_opts)) {
		if (verbose || fs->fs_unmounted++ < 5)
			syslog(LOG_ERR, "remount of %s on %s: %m", remname,
		    fs->fs_mntpnt);
		if (trace > 1)
			perror("remount FAILED ");
		return (NFSERR_IO);
	}

	fs->fs_mntpntdev = stbuf.st_dev;
	if (stat(fs->fs_mntpnt, &stbuf) < 0) {
		syslog(LOG_ERR, "remount: couldn't stat: %s: %m", fs->fs_mntpnt);
		return (NFSERR_IO);
	}
	fs->fs_mountdev = stbuf.st_dev;

	if (trace > 1)
		(void) fprintf(stderr, "remount OK\n");
	return (NFS_OK);

}

sys_mount(host, mount_point, mflags, argp, opts)
	char *host;
	char *mount_point;
	int mflags;
	struct nfs_args *argp;
	char *opts;		/* Ultrix only, though OS doesn't use.... */
{
#ifdef ultrix			/* JFS */
	struct nfs_gfs_mount args_ult;

	/* Load OSF nfs_args into ULTRIX nfs_gfs_mount */
	args_ult.gfs_flags = mflags; /* M_ flags */
	args_ult.addr = argp->addr;
	args_ult.fh = (fhandle_t *) argp->fh;
	args_ult.flags = argp->flags; /* NFSMNT_ flags */
	args_ult.wsize = argp->wsize;
	args_ult.rsize = argp->rsize;
	args_ult.timeo = argp->timeo;
	args_ult.retrans = argp->retrans;
	args_ult.hostname = host;
	args_ult.optstr = opts;
	args_ult.pg_thresh = SIXTYFOUR;
	args_ult.acregmin = argp->acregmin;
	args_ult.acregmax = argp->acregmax;
	args_ult.acdirmin = argp->acdirmin;
	args_ult.acdirmax = argp->acdirmax;
	if (trace > 1) {
		fprintf(stderr, "sys_mount: args_ult.optstr = %s\n",
			args_ult.optstr);
		fprintf(stderr, "sys_mount: args_ult.gfs_flags = %x\n",
			args_ult.gfs_flags);
		fprintf(stderr, "sys_mount: args_ult.flags = %x\n",
			args_ult.flags);
		fprintf(stderr, "sys_mount: type = %x\n", GT_NFS);
		fprintf(stderr, "sys_mount: mount fields: %s %s %x\n",
			argp->hostname, mount_point, mflags & M_RONLY);
	}
	return mount(argp->hostname, mount_point, mflags & M_RONLY,
		     GT_NFS, &args_ult);
#else				/* ultrix */
#ifdef OLDMOUNT
#define MOUNT_NFS 1
#endif
	return mount(MOUNT_NFS, mount_point, mflags, argp);
#endif				/* ultrix */
}

/*
 * Return the value of a numeric option of the form foo=x, if
 * option is not found or is malformed, return 0.
 */
nopt(mnt, opt)
	struct mntent *mnt;
	char *opt;
{
	int val = 0;
	char *equal;
	char *str;

	if (str = hasmntopt(mnt, opt)) {
		if (equal = index(str, '=')) {
			val = atoi(&equal[1]);
		} else {
			syslog(LOG_ERR, "Bad numeric option '%s'", str);
		}
	}
	return (val);
}

#ifdef ultrix
char *strdup(s1)
	char * s1;
{
        char *s2;

        if (s1 == NULL || (s2 = (char *)malloc(strlen(s1) + 1)) == NULL)
                return NULL;
        return (strcpy(s2, s1));
}
#endif /* ultrix */
