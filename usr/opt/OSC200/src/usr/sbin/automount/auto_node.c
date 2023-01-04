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
static char     *sccsid = "@(#)$RCSfile: auto_node.c,v $ $Revision: 4.2.9.2 $ (DEC) $Date: 1993/10/11 17:32:07 $";
#endif /* ultrix */
#endif /* lint */

/*
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 */

/*
 *      Modification History:
 *
 *	06 Sep 91 -- condylis
 *		Changed mod time of directory when link is created in
 *		directory.  Needed for directory caching in TIN nfs client.
 *
 */

#include <stdio.h>
#include <ctype.h>
#include <syslog.h>
#include <values.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/dir.h>
#include <rpc/types.h>
#include <rpc/auth.h>
#include <rpc/auth_unix.h>
#include <rpc/xdr.h>
#include <rpc/clnt.h>
#include <netinet/in.h>
#include <rpcsvc/yp_prot.h>
#include <rpcsvc/ypclnt.h>
#include "nfs_prot.h"
#ifdef ultrix				/* JFS */
typedef quad fsid_t;                    /* file system id type */
#include "auto_ultrix.h"
#endif /* ultrix */
#define NFSCLIENT
#include <sys/mount.h>
#include "automount.h"

struct internal_fh {
	int	fh_pid;
	int	fh_time;
	int	fh_num;
};

static int fh_cnt = 3;
static right_pid = -1;
static time_t right_time;

extern int trace;

struct queue fh_q_hash[FH_HASH_SIZE];

new_fh(vnode)
	struct vnode *vnode;
{
	time_t time();
	register struct internal_fh *ifh =
		(struct internal_fh *)(&vnode->vn_fh);

	if (right_pid == -1) {
		right_pid = getpid();
		(void) time(&right_time);
	}
	ifh->fh_pid = right_pid;
	ifh->fh_time = right_time;
	ifh->fh_num = ++fh_cnt;

	INSQUE(fh_q_hash[ifh->fh_num % FH_HASH_SIZE], vnode);
}

free_fh(vnode)
	struct vnode *vnode;
{
	register struct internal_fh *ifh =
		(struct internal_fh *)(&vnode->vn_fh);

	REMQUE(fh_q_hash[ifh->fh_num % FH_HASH_SIZE], vnode);
}

struct vnode *
fhtovn(fh)
	nfs_fh *fh;
{
	register struct internal_fh *ifh = 
		(struct internal_fh *)fh;
	int num;
	struct vnode *vnode;

	if (ifh->fh_pid != right_pid || ifh->fh_time != right_time)
		return ((struct vnode *)0);
	num = ifh->fh_num;
	vnode = HEAD(struct vnode, fh_q_hash[num % FH_HASH_SIZE]);
	while (vnode) {
		ifh = (struct internal_fh *)(&vnode->vn_fh);
		if (num == ifh->fh_num)
			return (vnode);
		vnode = NEXT(struct vnode, vnode);
	}
	return ((struct vnode *)0);
}

/*
 * Routine to return the name of the file given its file handle.  Used
 * by trace code in the NFS server to print the filename associated with
 * NFS requests.
 */
char *fhtocp(fh)
	nfs_fh *fh;
{
	struct autodir *dir;
	register struct vnode *vnode;
	register struct link *link;

	if (vnode = fhtovn(fh)) {
		if (vnode->vn_type == VN_LINK) {
			link = (struct link *)vnode->vn_data;
			return link->link_name;
		} else {
			dir = (struct autodir *)vnode->vn_data;
			return dir->dir_name;
		}
	}
	return "stale";
}

int
fileid(vnode)
	struct vnode *vnode;
{
	register struct internal_fh *ifh =
		(struct internal_fh *)(&vnode->vn_fh);

	return (ifh->fh_num);
}

dirinit(mntpnt, map, opts, isdirect)
	char *mntpnt, *map, *opts;
{
	struct autodir *dir;
	struct filsys *fs;
	register fattr *fa;
	struct stat stbuf;
	int mydir = 0;
	struct link *link;
	extern int verbose, yp;
	char *check_hier();
	char *opt_check();
	char *p;
	char *truep, true_name[MAXPATHLEN+1];

	p = mntpnt + (strlen(mntpnt) - 1);
	if (*p == '/')
		*p = '\0';	/* trim trailing / */
	if (*mntpnt != '/') {
		syslog(LOG_ERR, "map %s: dir %s must start with '/'",
		       map, mntpnt);
		return;
	}
	if (realpath(mntpnt, true_name))
		truep = true_name;
	else
		truep = mntpnt;	/* Non existant or dangling link */
	for (dir = HEAD(struct autodir, dir_q); dir;
	    dir = NEXT(struct autodir, dir))
		if (strcmp(dir->dir_truename, true_name) == 0)
			return;
	if (p = check_hier(true_name)) {
		(void) syslog(LOG_ERR, "map %s: hierarchical mountpoints: %s and %s",
			map, p, mntpnt);
		return;
	}
	if (p = opt_check(opts)) {
		syslog(LOG_ERR,
			"map %s: WARNING: default option \"%s\" ignored",
			map, p);
	}

	/*
	 * If it's a direct map then call dirinit
	 * for every map entry. Try first for a local
	 * file, then a NIS map.
	 */
	if (strcmp(mntpnt, "/-") == 0) {
		if (loaddirect_file(map, opts) < 0) {
			(void) loaddirect_yp(map, map, opts);
		}
		return;
	}

	/*
	 * Check whether there's something already mounted here
	 */
	for (fs = HEAD(struct filsys, fs_q); fs;
		fs = NEXT(struct filsys, fs)) {
		if (strcmp(fs->fs_mntpnt, true_name) == 0) {
			(void) syslog(LOG_ERR,
				"map %s: WARNING: %s:%s already mounted on %s",
				map, fs->fs_host, fs->fs_dir, mntpnt);
			break;
		}
	}

	/*
	 * Check whether the map (local file or NIS) exists
	 */
	if (*map != '-' && access(map, R_OK) != 0 && yp) {
		char *val ; int len;

		if (yp_match(mydomain, map, "x",
			1, &val, &len) == YPERR_MAP) {
			if (verbose)
				syslog(LOG_ERR, "map %s: Not found", map);
			return;
		}
	}

	/*
	 * Create a mount point if necessary.  If we worked at it some, we
	 * could create a directory at the end of a dangling symbolic link,
	 * but I think that might require doing readlinks all the way down
	 * the chain if intermediate links are used.  Not worth the effort!
	 */
	if (stat(mntpnt, &stbuf) == 0) {
		if ((stbuf.st_mode & S_IFMT) != S_IFDIR) {
			syslog(LOG_ERR, "map %s: %s: Not a directory",
			       map, mntpnt);
			return;
		}
		if (verbose && !emptydir(mntpnt))
			syslog(LOG_ERR, "map %s: WARNING: %s not empty!",
			       map, mntpnt);
	} else {
		if (lstat(mntpnt, &stbuf) == 0) {
			syslog(LOG_ERR, "map %s: %s: Dangling symbolic link",
			       map, mntpnt);
			return;
		}
		if (mkdir_r(mntpnt)) {
			syslog(LOG_ERR, "map %s: Cannot create directory %s: %m",
			       map, mntpnt);
			return;
		}
		mydir = 1;
	}

	dir = (struct autodir *)malloc(sizeof *dir);
	if (dir == NULL)
		goto alloc_failed;
	bzero((char *)dir, sizeof *dir);
	if (!(dir->dir_name = strdup(mntpnt)))
		goto dir_failed;
	if (!(dir->dir_truename = strdup(true_name)))
		goto dir_failed;
	if (!(dir->dir_map = strdup(map)))
		goto dir_failed;
	if (!(dir->dir_opts = strdup(opts)))
		goto dir_failed;
	if (dir->dir_opts == NULL) {
		goto alloc_failed;
	}
	dir->dir_remove = mydir;
	INSQUE(dir_q, dir);

	new_fh(&dir->dir_vnode);
	dir->dir_vnode.vn_data = (char *)dir;
	fa = &dir->dir_vnode.vn_fattr;
	fa->nlink = 1;
	fa->uid = 0;
	fa->gid = 0;
	fa->size = 512;
	fa->blocksize = 512;
	fa->rdev = 0;
	fa->blocks = 1;
	fa->fsid = 0;
	fa->fileid = fileid(&dir->dir_vnode);
	(void) gettimeofday((struct timeval *)&fa->atime, (struct timezone *)0);
	fa->mtime = fa->atime;
	fa->ctime = fa->atime;

	if (!isdirect) {
		/* The mount point is a directory.
		 * Set up links for it's "." and ".." entries.
		 */
		dir->dir_vnode.vn_type = VN_DIR;
		fa->type = NFDIR;
		fa->mode = NFSMODE_DIR + 0555;
		link = makelink(dir, "." , NULL, "");
		if (link == NULL)
			goto alloc_failed;
		link->link_death = MAXINT;
		link->link_vnode.vn_fattr.fileid = fileid(&link->link_vnode);
		link = makelink(dir, "..", NULL, "");
		if (link == NULL)
			goto alloc_failed;
		link->link_death = MAXINT;
		link->link_vnode.vn_fattr.fileid = fileid(&link->link_vnode);
	} else {
		/* The mount point is direct-mapped. Set it
		 * up as a symlink to the real mount point.
		 */
		dir->dir_vnode.vn_type = VN_LINK;
		fa->type = NFLNK;
		fa->mode = NFSMODE_LNK + 0777;
		fa->size = 20;
		link = (struct link *)malloc(sizeof *link);
		if (link == NULL)
			goto alloc_failed;
		dir->dir_vnode.vn_data = (char *)link;
		link->link_dir = dir;
		link->link_name = strdup(mntpnt);
		if (link->link_name == NULL) {
			FREE((char *)link);
			goto alloc_failed;
		}
		link->link_fs = NULL;
		link->link_path = NULL;
		link->link_death = 0;
	}
	return;

dir_failed:
	if (dir->dir_name)
		FREE((char *)dir->dir_name);
	if (dir->dir_truename)
		FREE((char *)dir->dir_truename);
	if (dir->dir_map)
		FREE((char *)dir->dir_map);
	if (dir)
		FREE((char *)dir);

alloc_failed:
	syslog(LOG_ERR, "dirinit: memory allocation failed: %m");
	return;
}

/*
 *  Check whether the mount point is a
 *  subdirectory or a parent directory
 *  of any previously mounted automount
 *  mount point.
 */
char *
check_hier(mntpnt)
	char *mntpnt;
{
	register struct autodir *dir;
	register char *p, *q;

	for (dir = TAIL(struct autodir, dir_q) ; dir ; 
		dir = PREV(struct autodir, dir)) {
		if (strcmp(dir->dir_map, "-null") == 0)
			continue;
		p = dir->dir_name;
		q = mntpnt;
		for (; *p == *q ; p++, q++)
			if (*p == '\0')
				break;
		if (*p == '/' && *q == '\0')
			return (dir->dir_name);
		if (*p == '\0' && *q == '/')
			return (dir->dir_name);
		if (*p == '\0' && *q == '\0')
			return (dir->dir_name);
	}
	return (NULL);	/* it's not a subdir or parent */
}

emptydir(name)
	char *name;
{
	DIR *dirp;
	struct dirent *d;

	dirp = opendir(name);
	if (dirp == NULL) 
		return (0);
	while (d = readdir(dirp)) {
		if (strcmp(d->d_name, ".") == 0)
			continue;
		if (strcmp(d->d_name, "..") == 0)
			continue;
		break;
	}
	(void) closedir(dirp);
	if (d)
		return (0);
	return (1);
}

loaddirect_file(map, opts)
	char *map, *opts;
{
	FILE *fp;
	int done = 0;
	char *line, *p1, *p2;
	extern char *get_line();
	char linebuf[MAXMAPLEN];

	if ((fp = fopen(map, "r")) == NULL) {
		return -1;
	}

	while ((line = get_line(fp, linebuf, sizeof linebuf)) != NULL) {
		p1 = line;
		while (*p1 && isspace(*(u_char *)p1)) p1++;
		if (*p1 == '\0')
			continue;
		p2 = p1;
		while (*p2 && !isspace(*(u_char *)p2)) p2++;
		*p2 = '\0';
		if (*p1 == '+')
			(void) loaddirect_yp(p1+1, map, opts);
		else
			dirinit(p1, map, opts, 1);
		done++;
	}
	
	(void) fclose(fp);
	return done;
}

loaddirect_yp(ypmap, localmap, opts)
	char *ypmap, *localmap, *opts;
{
	int first, err;
	char *key, *nkey, *val;
	int kl, nkl, vl;
	char dir[100];
	extern int yp;

	if (!yp)
		return;

	first = 1;
	key  = NULL; kl  = 0;
	nkey = NULL; nkl = 0;
	val  = NULL; vl  = 0;

	for (;;) {
		if (first) {
			first = 0;
			err = yp_first(mydomain, ypmap, &nkey, &nkl, &val, &vl);
		} else {
			err = yp_next(mydomain, ypmap, key, kl, &nkey, &nkl,
				&val, &vl);
		}
		if (err) {
			if (err != YPERR_NOMORE && err != YPERR_MAP)
				syslog(LOG_ERR, "%s: %s",
					ypmap, yperr_string(err));
			return;
		}
		if (key)
			FREE(key);
		key = nkey;
		kl = nkl;

		if (kl < 2 || kl >= 100)
			continue;
		if (isspace(*(u_char *)key) || *key == '#')
			continue;
		(void) strncpy(dir, key, kl);
		dir[kl] = '\0';

		dirinit(dir, localmap, opts, 1);

		FREE(val);
	}
}

struct link *
makelink(dir, name, fs, linkpath)
	struct autodir *dir;
	char *name;
	struct filsys *fs;
	char *linkpath;
{
	struct link *link;
	register fattr *fa;

	link = findlink(dir, name);
	if (link == NULL) {
		link = (struct link *)malloc(sizeof *link);
		if (link == NULL)
			goto alloc_failed;
		link->link_name = strdup(name);
		if (link->link_name == NULL) {
			FREE((char *)link);
			goto alloc_failed;
		}
		INSQUE(dir->dir_head, link);
		link->link_fs = NULL;
		link->link_path = NULL;
		new_fh(&link->link_vnode);
		link->link_vnode.vn_data = (char *)link;
		link->link_vnode.vn_type = VN_LINK;
	}
	link->link_dir = dir;
	link->link_fs = NULL;
	if (link->link_path) {
		FREE(link->link_path);
		link->link_path = NULL;
	}
	if (fs) {
		link->link_fs = fs;
		fs->fs_rootfs->fs_death = time_now + max_link_time;
	}
	if (linkpath) {
		link->link_path = strdup(linkpath);
		if (link->link_path == NULL) {
			REMQUE(link->link_dir->dir_head, link);
			if (link->link_name)
				FREE(link->link_name);
			FREE((char *)link);
			goto alloc_failed;
		}
	}
	link->link_death = time_now + max_link_time;

	fa = &link->link_vnode.vn_fattr;
	fa->type = NFLNK;
	fa->mode = NFSMODE_LNK + 0777;
	fa->nlink = 1;
	fa->uid = 0;
	fa->gid = 0;
	fa->size = strlen(linkpath);
/* 
	if (fs) 
		fa->size += strlen(fs->fs_dir) + 1;
*/
	fa->blocksize = 512;
	fa->rdev = 0;
	fa->blocks = 1;
	fa->fsid = 0;
	fa->fileid = fileid(&link->link_vnode);
	(void) gettimeofday((struct timeval *)&fa->atime, (struct timezone *)0);
	fa->mtime = fa->atime;
	fa->ctime = fa->atime;
	if (dir)
		(void) gettimeofday((struct timeval *)&dir->dir_vnode.vn_fattr.mtime, (struct timezone *)0);

	return (link);

alloc_failed:
	syslog(LOG_ERR, "Memory allocation failed: %m");
	return (NULL);
}

zero_link(link)
	struct link *link;
{
	link->link_death = 0;
	link->link_fs = (struct filsys *)0;
	(void) gettimeofday((struct timeval *)&link->link_vnode.vn_fattr.mtime, (struct timezone *)0);
	if (link->link_dir)
	(void) gettimeofday((struct timeval *)&link->link_dir->dir_vnode.vn_fattr.mtime, (struct timezone *)0);
}

/*
 * Interesting.  The only caller of this is nfsproc_readlink_2(), which is
 * called only for direct maps.  That means we never free links!  For
 * indirect maps, freeing links could lead to glitches when multiple
 * readdir operations are needed to read a whole directory, so that may make
 * sense.  I haven't looked into why we don't free direct links.
 */
free_link(link)
	struct link *link;
{

	/* Don't remove a direct link - just zero it */

	if (link->link_dir->dir_vnode.vn_type == VN_LINK) {
		zero_link(link);
		return;
	}

	REMQUE(link->link_dir->dir_head, link);
	free_fh(&link->link_vnode);
	if (link->link_name)
		FREE(link->link_name);
	if (link->link_path)
		FREE(link->link_path);
	FREE((char *)link);
}

struct link *
findlink(dir, name)
	struct autodir *dir;
	char *name;
{
	struct link *link;

	for (link = HEAD(struct link, dir->dir_head); link;
	    link = NEXT(struct link, link))
		if (strcmp(name, link->link_name) == 0)
			return (link);
	return ((struct link *)0);
}

free_filsys(fs)
	struct filsys *fs;
{
	flush_links(fs);
	REMQUE(fs_q, fs);
	FREE(fs->fs_host);
	FREE(fs->fs_dir);
	FREE(fs->fs_mntpnt);
	if (fs->fs_opts)
		FREE(fs->fs_opts);
	FREE(fs);
}

struct filsys *
#ifdef ultrix
alloc_fs(host, dir, mntpnt, opts)
        char *host, *dir, *mntpnt, *opts;
#else
alloc_fs(host, dir, mntpnt, na, flags)
	char *host, *dir, *mntpnt;
	struct nfs_args *na;
	int flags;
#endif /* ultrix */
{
	struct filsys *fs;

	fs = (struct filsys *)malloc(sizeof *fs);
	if (fs == NULL)
		goto alloc_failed;
	bzero((char *)fs, sizeof *fs);
	fs->fs_rootfs = fs;
	fs->fs_host = strdup(host);
	if (fs->fs_host == NULL) {
		FREE((char *)fs);
		goto alloc_failed;
	}
	fs->fs_dir = strdup(dir);
	if (fs->fs_dir == NULL) {
		FREE(fs->fs_host);
		FREE((char *)fs);
		goto alloc_failed;
	}
	fs->fs_mntpnt = strdup(mntpnt);
	if (fs->fs_mntpnt == NULL) {
		FREE(fs->fs_dir);
		FREE(fs->fs_host);
		FREE((char *)fs);
		goto alloc_failed;
	}
#ifdef ultrix
        if (opts != NULL) {
                fs->fs_opts = strdup(opts);
                if (fs->fs_opts == NULL) {
                        FREE(fs->fs_mntpnt);
                        FREE(fs->fs_dir);
                        FREE(fs->fs_host);
                        FREE((char *)fs);
                        goto alloc_failed;
                }
        } else
                fs->fs_opts = NULL;
#else
	fs->fs_nfsargs = *na;
	fs->fs_mflags = flags;
#endif /* ultrix */
	INSQUE(fs_q, fs);
	return (fs);

alloc_failed:
	syslog(LOG_ERR, "Memory allocation failed: %m");
	return (NULL);
}

my_insque(head, item)
	struct queue *head, *item;
{
	item->q_next = head->q_head;
	item->q_prev = NULL;
	head->q_head = item;
	if (item->q_next)
		item->q_next->q_prev = item;
	if (head->q_tail == NULL)
		head->q_tail = item;
}

my_remque(head, item)
	struct queue *head, *item;
{
	if (item->q_prev)
		item->q_prev->q_next = item->q_next;
	else
		head->q_head = item->q_next;
	if (item->q_next)
		item->q_next->q_prev = item->q_prev;
	else
		head->q_tail = item->q_prev;
	item->q_next = item->q_prev = NULL;
}

/*
 * I ripped out the link timeout code to fix a problem with hierarchical
 * mounts where do_mount wasn't able to mount the root filesystem and
 * do_unmount can't unmount one of the remaining file systems.  Zero_link
 * meant the the next lookup would call do_mount which would think none of
 * the hierarchy was mounted, attempt to mount everything, get EBUSY on all
 * of them and return an error.  Instead, free_filsys is calling flush_links,
 * and that calls zero_link.
 */
do_timeouts()
{
	struct filsys *fs, *nextfs;
	extern int trace, syntaxok;
	extern void check_mtab();

	if (trace > 1)
		(void) fprintf(stderr, "do_timeouts: enter\n");

	check_mtab();
	syntaxok = 1;

	for (fs = HEAD(struct filsys, fs_q); fs; fs = nextfs) {
		nextfs = NEXT(struct filsys, fs);
		if (fs->fs_mine) {
			if (fs != fs->fs_rootfs)
				continue;
			if (fs->fs_death > time_now)
				continue;
			if (!do_unmount(fs))
				fs->fs_death = time_now + max_link_time;
		}
	}

	if (trace > 1)
		(void) fprintf(stderr, "do_timeouts: exit\n");
}

flush_links(fs)
	struct filsys *fs;
{
	struct link *link;
	struct vnode *vnode;
	int i;

	for (i = 0; i < FH_HASH_SIZE; i++) {
		vnode = HEAD(struct vnode, fh_q_hash[i]);
		for (; vnode; vnode = NEXT(struct vnode, vnode)) {
			if (vnode->vn_type != VN_LINK)
				continue;
			link = (struct link *)vnode->vn_data;
			if (link->link_fs == fs)
				zero_link(link);
		}
	}
}
