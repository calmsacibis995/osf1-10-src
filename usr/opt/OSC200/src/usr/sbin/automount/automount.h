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

/*
 *      @(#)$RCSfile: automount.h,v $ $Revision: 4.2.9.3 $ (DEC) $Date: 1993/11/23 21:47:49 $
 *      %W%  ULTRIX  %G%
 */

/*
 * automount.h 1.9 88/12/14 Copyright 1987 Sun Microsystems, Inc.
 */

/* 3.x compatibility */
#ifdef OLDMOUNT
#define LOG_DAEMON 0
#define MAXHOSTNAMELEN  64
#define MAXNETNAMELEN   255
#endif /* OLDMOUNT */

#define MAXMAPLEN	10000	/* Those hierarchical mounts can get pretty long! */
#define	FH_HASH_SIZE	8

/*
 * General queue structure 
 */
struct queue {
	struct queue	*q_next;
#define	q_head	q_next
	struct queue	*q_prev;
#define	q_tail	q_prev
};

#define	INSQUE(head, ptr) my_insque(&(head), &(ptr)->q)
#define	REMQUE(head, ptr) my_remque(&(head), &(ptr)->q)
#define HEAD(type, head) ((type *)(head.q_head))
#define NEXT(type, ptr)	((type *)(ptr->q.q_next))
#define	TAIL(type, head) ((type *)(head.q_tail))
#define PREV(type, ptr)	((type *)(ptr->q.q_prev))
	

/*
 * Types of filesystem entities (vnodes)
 * We support only one level of DIR; everything else is a symbolic LINK
 */
enum vn_type { VN_DIR, VN_LINK};
struct vnode {
	struct queue q;
	nfs_fh	vn_fh;		/* fhandle */
	struct fattr vn_fattr;	/* file attributes */
	enum vn_type vn_type;	/* type of vnode */
	caddr_t	vn_data;	/* vnode private data */
};

struct vnode *fhtovn();		/* lookup vnode given fhandle */

/*
 * Structure describing a host/filesystem/dir tuple in a NIS map entry
 */
struct mapfs {
	struct mapfs *mfs_next;		/* next in entry */
	int	mfs_ignore;		/* ignore this entry */
	char	*mfs_host;		/* host name */
	struct in_addr mfs_addr;	/* address of host */
	char	*mfs_dir;		/* dir to mount */
	char	*mfs_subdir;		/* subdir of dir */
};

/*
 * NIS entry - lookup of name in DIR gets us this
 */
struct mapent {
	char	*map_root;
	char	*map_mntpnt;
	char	*map_mntopts;
	struct mapfs *map_fs;
	struct mapent *map_next;
};
struct mapent *getmapent();

/*
 * Everthing we know about a mounted filesystem
 * Can include things not mounted by us (fs_mine == 0)
 */
struct filsys {
	struct queue q;			/* next in queue */
	int	fs_death;		/* time when no longer valid */
	int	fs_mine;		/* 1 if we mounted this fs */
	int	fs_present;		/* for checking unmounts */
	int	fs_unmounted;		/* 1 if unmounted OK */
	char	*fs_type;		/* type of filesystem */
	char	*fs_host;		/* host name */
	char	*fs_dir;		/* dir of host mounted */
	char	*fs_mntpnt;		/* local mount point */
	char	*fs_opts;		/* mount options */
	dev_t	fs_mntpntdev;		/* device of mntpnt */
	dev_t	fs_mountdev;		/* device of mount */
	struct nfs_args fs_nfsargs;	/* nfs mount args */
	struct sockaddr_in fs_addr;	/* host address */
	struct filsys *fs_rootfs;	/* root for this hierarchy */
	nfs_fh	fs_rootfh;		/* file handle for nfs mount */
	int	fs_mflags;		/* mount flags */
};
struct queue fs_q;
struct filsys *already_mounted(), *alloc_fs();

/*
 * Structure for recently referenced links
 */
struct link {
	struct queue q;		/* next in queue */
	struct vnode link_vnode;	/* space for vnode */
	struct autodir *link_dir;	/* dir which we are part of */
	char	*link_name;	/* this name in dir */
	struct filsys *link_fs;	/* mounted file system */
	char	*link_path;	/* dir within file system */
	int	link_death;	/* time when no longer valid */
};
struct link *makelink();
struct link *findlink();
	
/*
 * Descriptor for each directory served by the automounter 
 */
struct autodir {
	struct queue q;
	struct	vnode dir_vnode;	/* vnode */
	char	*dir_name;	/* mount point */
	char	*dir_truename;	/* target if dir_name is a symbolic link */
	char	*dir_map;	/* name of map for dir */
	char	*dir_opts;	/* default mount options */
	int	dir_remove;	/* remove mount point */
	int	dir_intercepting; /* umount when shutting down */
	struct queue dir_head;
};
struct queue dir_q;

char self[64];		/* my hostname */
char mydomain[64];	/* my domain name */
char tmpdir[200];	/* real name of /tmp */
struct in_addr my_addr;	/* my inet address */

#ifdef DEBUGOUT
#define FREE(_a_) \
{(void) fprintf(stderr, \
    "in %s:%d  freeing 0x%x\n", __FILE__, __LINE__,&(_a_)); \
    free(_a_);}
#else /* DEBUGOUT */
#define FREE(_a_) free(_a_)
#endif /* DEBUGOUT */

/*void *malloc();*/
char *index(), *strdup();
time_t time_now;		/* set at start of processing of each RPC call */
int mount_timeout;	/* max seconds to wait for mount */
int max_link_time;	/* seconds to keep link around */
int nomounts;		/* don't do any mounts - for cautious servers */
nfsstat lookup(), nfsmount();
char *hasmntopt();
