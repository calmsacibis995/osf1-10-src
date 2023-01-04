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
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 */
/*	
 *	@(#)$RCSfile: mount.h,v $ $Revision: 4.3.20.6 $ (DEC) $Date: 1993/12/17 01:38:02 $	
 */ 
/*
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */
#ifndef	_SYS_MOUNT_H_
#define _SYS_MOUNT_H_

#ifdef	_KERNEL
#include <rt_preempt.h>
#include <sys/unix_defs.h>
#include <kern/zalloc.h>
#endif

#include <sys/types.h>

#include <sys/secdefines.h>
#if	SEC_ARCH
#include <sys/security.h>
#endif 

/*
 * File system types.
 *
 * WARNING:
 *	These constants serve as indexes for the filesystem name in the
 *	mnt_names[] array in sys/fs_types.h.  Any changes to these
 *	constants should be reflected in the definition of the mnt_names[]
 *	array in this file so that the filesystem name is always indexed
 *	by the associated constant.
 */
#define	MOUNT_NONE	0
#define	MOUNT_UFS	1
#define	MOUNT_NFS	2
#define MOUNT_MFS	3
#define	MOUNT_PC	4
#define MOUNT_S5FS	5
#define MOUNT_CDFS	6
#define MOUNT_DFS       7         /* DCE DFS */
#define MOUNT_EFS       8         /* DCE Episode FS */
#define MOUNT_PROCFS	9
#define	MOUNT_MSFS	10
#define MOUNT_FFM	11
#define MOUNT_FDFS      12
#define MOUNT_ADDON	13	/* Generic file system type */
#define	MOUNT_MAXTYPE	13

#define MNAMELEN 90	/* length of buffer for returned name */

typedef quad fsid_t;			/* file system id type */

/*
 * Size of an NFS fhandle in bytes
 */
#define	NFS_FHSIZE	32

/* Warning!    
 *      NFSMAXFIDDATA   MUST! be a multiple of 4 bytes (32 bits)
 */
#define NFSMAXFIDDATA ((32 - sizeof (fsid_t) - 2 * sizeof(int))/2)
#define NFSMAXFIDLEN ((32 - sizeof (fsid_t))/2)

/*
 *  To support increased NFS export granularity we must stuff the fid of
 *  the export granting directory into the fhandle.  We also need
 *  the fid of the object to be exported. 
 *  So we need to be able to stuff two fids plus a fsid into an fhandle.
 *  The following fid structure replacement allows us to do this.
 */
struct _fh_fid {
	u_short	fid_len;		  /* length of data in bytes */
	u_short fid_reserved;		  /* to force 32-bit alignment */
	char	fid_data[NFSMAXFIDDATA];  /* data (variable length) */
	/* MUST END ON A 4-byte BOUNDARY */
};

typedef struct _fh_fid fh_fid_t;
	
/*
 * Generic file handle to support increase export granularity
 */
struct fhandle {
	fsid_t	fh_fsid;       /* filesystem id */
	fh_fid_t fh_fid;  /* Id of file */
	fh_fid_t fh_efid; /* Id of export granting directory  */
						
};



typedef struct fhandle fhandle_t;

#define fsid_equal(fsid1,fsid2) \
(bcmp((caddr_t)(fsid1), (caddr_t)(fsid2), sizeof(fsid_t))==0)  

#define fid_equal(fid1,fid2) \
((fid1)->fid_len == (fid2)->fid_len &&\
bcmp((fid1)->fid_data, (fid2)->fid_data, (fid1)->fid_len - 2*sizeof(short))==0)

#define fid_copy(fid1,fid2) \
if ((fid1)->fid_len > NFSMAXFIDLEN || (fid1)->fid_len < 2*sizeof(short) )\
	panic("improper fid size");\
(fid2)->fid_len = (fid1)->fid_len;\
bcopy((fid1)->fid_data,(fid2)->fid_data,(fid1)->fid_len - 2*sizeof(short)); 

#define fsid_copy(fsid1,fsid2) \
bcopy((caddr_t)(fsid1), (caddr_t)(fsid2), sizeof(fsid_t))  

#define fsid_print(fs) \
printf("%x,%x\n", *(int *)(fs), *((int *)(fs)+1))

#define fid_print(fid) \
if ((fid)) { \
int i; \
printf("len=%d ", (fid)->fid_len); \
for (i = 0; i < NFSMAXFIDDATA; i++)  \
	printf(",%x",(fid)->fid_data[i]); \
printf("\n"); \
} else printf("can not print null fid\n");




/*
 * File identifier.
 * These are unique per filesystem on a single machine.
 */

#define	MAXFIDSZ	20

struct fid {
	u_short		fid_len;		/* length of data in bytes */
	u_short		fid_reserved;		/* force 4-byte alignment */
	char		fid_data[MAXFIDSZ];	/* data (variable length) */
};

/*
 * Arguments to mount UFS
 */
struct ufs_args {
        char    *fspec;         /* block special device to mount */
        int     exflags;        /* export related flags */
        uid_t   exroot;         /* mapping for root uid */
};

/*
 * Arguments to mount CDFS
 */
struct cdfs_args {
        char    *fspec;         /* block special device to mount */
        int     exflags;        /* export related flags */
        uid_t   exroot;         /* mapping for root uid */
	int	flags;		/* CDFS mount options */
};

/*
 * Arguments to mount System V file system
 */
typedef struct ufs_args s5fs_args;

/*
 * Arguments to mount MFS
 */
struct mfs_args {
        char    *name;          /* name to export for statfs */
        caddr_t base;           /* base address of file system in memory */
        u_int size;            /* size of file system */
};

/*
 * Arguments to mount /proc file system
 */
typedef struct ufs_args procfs_args;

/* Arguments for file-on-file mount */
struct ffm_args {
	int   ffm_flags;                      /* operational flags */
	union {
	    char    *ffm_pname;             /* pathname to mount */
	    int     ffm_fdesc;              /* file descriptor to mount */
        } f_un;
};

#define ffm_pathname f_un.ffm_pname     /* shorthand */
#define ffm_filedesc f_un.ffm_fdesc     /* shorthand */

#define FFM_FD          0x000000001     /* file descriptor supplied */
#define FFM_CLONE       0x000000002     /* clone should be performed */



/*
 * File Handle (32 bytes for version 2), variable up to 1024 for version 3
 */
union nfsv2fh {
        fhandle_t       fh_generic;
        u_char          fh_bytes[32];
};
typedef union nfsv2fh nfsv2fh_t;

/*
 * Arguments to mount NFS
 */
struct nfs_args {
        struct sockaddr_in      *addr;          /* file server address */
        nfsv2fh_t               *fh;            /* File handle to be mounted */
        int                     flags;          /* flags */
        int                     wsize;          /* write size in bytes */
        int                     rsize;          /* read size in bytes */
        int                     timeo;          /* initial timeout in .1 secs */
        int                     retrans;        /* times to retry send */
        char                    *hostname;      /* server's name */
        int                     acregmin;       /* min secs for file attrcache*/
        int                     acregmax;       /* max secs for file attrcache*/
        int                     acdirmin;       /* min secs for dir attrcache */
        int                     acdirmax;       /* max secs for dir attrcache */
        char                    *netname;       /* server's netname */
	struct pathcnf		*pathconf;	/* static pathconf kludge */
};

/*
 * NFS mount option flags
 */
#define NFSMNT_SOFT     0x001   /* soft mount (hard is default) */
#define NFSMNT_WSIZE    0x002   /* set write size */
#define NFSMNT_RSIZE    0x004   /* set read size */
#define NFSMNT_TIMEO    0x008   /* set initial timeout */
#define NFSMNT_RETRANS  0x010   /* set number of request retrys */
#define NFSMNT_HOSTNAME 0x020   /* set hostname for error printf */
#define NFSMNT_INT      0x040   /* allow interrupts on hard mount */
#define NFSMNT_NOCONN   0x080   /* no connect on mount - any responder */
#define NFSMNT_NOAC     0x0100  /* don't cache attributes */
#define NFSMNT_ACREGMIN 0x0200  /* set min seconds for file attr cache  */
#define NFSMNT_ACREGMAX 0x0400  /* set max seconds for file attr cache  */
#define NFSMNT_ACDIRMIN 0x0800  /* set min seconds for dir attr cache   */
#define NFSMNT_ACDIRMAX 0x01000 /* set max seconds for dir attr cache   */
#define NFSMNT_NOCTO    0x02000  /* don't freshen attributes on open */
#define NFSMNT_POSIX	0x04000	/* static pathconf kludge info */
#define NFSMNT_AUTO	0x08000	/* automount file system */

/*                   
 * MSFS              
 */                  
                                                               /* msfs */
struct msfs_id {                                               /* msfs */
    int id1;                                                   /* msfs */
    int id2;                                                   /* msfs */
    int tag;                                                   /* msfs */
};                                                             /* msfs */
                                                               /* msfs */
struct msfs_args {                                             /* msfs */
    struct msfs_id id;                                         /* msfs */
};                                                             /* msfs */

/*
 * file system statistics
 */

struct statfs {
	short	f_type;			/* type of filesystem (see below) */
	short	f_flags;		/* copy of mount flags */
	int	f_fsize;		/* fundamental filesystem block size */
	int	f_bsize;		/* optimal transfer block size */
	int	f_blocks;		/* total data blocks in file system */
	int	f_bfree;		/* free blocks in fs */
	int	f_bavail;		/* free blocks avail to non-su */
	int	f_files;		/* total file nodes in file system */
	int	f_ffree;		/* free file nodes in fs */
	fsid_t	f_fsid;			/* file system id */
	int	f_spare[9];		/* spare for later */
	char	f_mntonname[MNAMELEN];	/* directory on which mounted */
	char	f_mntfromname[MNAMELEN];/* mounted filesystem */
	union mount_info {		/* mount options */
		struct ufs_args ufs_args;
		struct nfs_args nfs_args;
		struct mfs_args mfs_args;
		s5fs_args s5fs_args;
		struct cdfs_args cdfs_args;
		procfs_args procfs_args;
                struct msfs_args msfs_args;                     /* msfs */
		struct ffm_args ffm_args;			/* ffm */
	} mount_info;
};

/*
 * Structure per mounted file system.
 * Each mounted file system has an array of
 * operations and an instance record.
 * The file systems are put on a doubly linked list.
 */
struct mount {
	struct mount	*m_next;		/* next in mount list */
	struct mount	*m_prev;		/* prev in mount list */
	struct vfsops	*m_op;			/* operations on fs */
	struct vnode	*m_vnodecovered;	/* vnode we mounted on */
	struct vnode	*m_mounth;		/* list of vnodes this mount */
	int		m_flag;			/* flags */
	uid_t		m_exroot;		/* exported mapping for uid 0 */
	uid_t		m_uid;			/* uid that performed mount */
	struct statfs	m_stat;			/* cache of fs stats */
	qaddr_t		m_data;			/* private data */
	/* information used to control NFS server console error messages */
	struct {
		int n_noexport;	/* # of no export messages */
		int last_noexport;	/* secs. since last no export msg */
		int n_stalefh;	/* # of stale fh messages */
		int last_stalefh;	/* secs. since last stale fh msg */
	} m_nfs_errmsginfo;
#if	SEC_ARCH
	tag_t		m_tag[SEC_TAG_COUNT];	/* for unlabeled filesystems */
#endif
#ifdef	_KERNEL
	lock_data_t	m_lookup_lock;		/* pathname/filehandle synch */
/*	udecl_funnel_data(,m_funnel)  */   	/* uniprocessor code compatibility */
       /* This gives a preprocessor error so we replace it with it's expan */
#if     SER_COMPAT || RT_PREEMPT
			int m_funnel   ;	/* serial code compatibility */
#endif /* SER_COMPAT */
	udecl_simple_lock_data(,m_lock)		/* multiprocessor exclusion */
	udecl_simple_lock_data(,m_vlist_lock)	/* protect vnode list */
#endif
};

/*
 * Mount flags.
 */
#define	M_RDONLY	0x00000001	/* read only filesystem */
#define	M_SYNCHRONOUS	0x00000002	/* file system written synchronously */
#define	M_NOEXEC	0x00000004	/* can't exec from filesystem */
#define	M_NOSUID	0x00000008	/* don't honor setuid bits on fs */
#define	M_NODEV		0x00000010	/* don't interpret special files */
/*
 * exported mount flags.
 */
#define	M_EXPORTED	0x00000100	/* file system is exported */
#define	M_EXRDONLY	0x00000200	/* exported read only */
#define M_EXRDMOSTLY    0x00000400      /* exported ro to most */
#if	SEC_ARCH
#define	M_SECURE	0x00000800	/* file system is labeled */
#endif
/*
 * Flags set by internal operations.
 */
#define	M_LOCAL		0x00001000	/* filesystem is stored locally */
#define	M_QUOTA		0x00002000	/* quotas are enabled on filesystem */
/*
 * Mask of flags that are visible to statfs()
 */
#define	M_VISFLAGMASK	0x0000ffff
/*
 * filesystem control flags.
 */
#define	M_UPDATE	0x00010000	/* not a real mount, just an update */
#define	M_SYNCING	0x00020000	/* sync in progress */
#define M_FMOUNT	0x00040000	/* forcibly mount, even if not clean */
#if	MACH
/*
 * MACH swap info
 */
#define	M_SWAP_PREFER	0x00080000	/* prefer to swap here */
#define	M_SWAP_NEVER	0x00100000	/* never swap here */
#endif

#define M_IOERROR	0x00200000	/* I/O error */

/* 
 * SVR3 and SVID-3 flags
 */

#define MS_DATA		0x000000020	/* called with 6 arguments */
#define MS_RDONLY	M_RDONLY
#define MS_NOSUID	M_NOSUID
#define MS_REMOUNT	M_UPDATE

/*
 * Operations supported on mounted file system.
 */
struct vfsops {
	int	(*vfs_mount)(	/* mp, path, data, ndp */ );
	int	(*vfs_start)(	/* mp, flags */ );
	int	(*vfs_unmount)(	/* mp, forcibly */ );
	int	(*vfs_root)(	/* mp, vpp */ );
	int	(*vfs_quotactl)(/* mp, cmd, uid, arg */ );
	int	(*vfs_statfs)(	/* mp */ );
	int	(*vfs_sync)(	/* mp, waitfor */ );
	int	(*vfs_fhtovp)(	/* mp, fidp, vpp */ );
	int	(*vfs_vptofh)(	/* vp, fidp */ );
	int	(*vfs_init)(	/* */ );
	int	(*vfs_mountroot)();
	int	(*vfs_swapvp)();
};

#if RT_PREEMPT
#define	MOUNT_FUNNEL(m)		RT_FUNNEL((m)->m_funnel)
#define	MOUNT_UNFUNNEL(m)	RT_UNFUNNEL((m)->m_funnel)
#else
#define	MOUNT_FUNNEL(m)		FUNNEL((m)->m_funnel)
#define	MOUNT_UNFUNNEL(m)	UNFUNNEL((m)->m_funnel)
#endif

#define _VFSOP_(f,mp,args,ret)						\
MACRO_BEGIN								\
	MOUNT_FUNNEL(mp);						\
	(ret) = (*(mp)->m_op->f)args;					\
	MOUNT_UNFUNNEL(mp);						\
MACRO_END

#define	_VFSOPV_(f,vp,args,ret)						\
MACRO_BEGIN								\
	MOUNT_FUNNEL((vp)->v_mount);					\
	(ret) = (*((vp)->v_mount->m_op->f))args;			\
	MOUNT_UNFUNNEL((vp)->v_mount);					\
MACRO_END

#define VFS_MOUNT(M,P,D,N,r)		_VFSOP_(vfs_mount,(M),((M),(P),(D),(N)),(r))
#define	VFS_START(MP,FL,r)		_VFSOP_(vfs_start,(MP),((MP),(FL)),(r))
#define	VFS_UNMOUNT(MP,F,r)		_VFSOP_(vfs_unmount,(MP),((MP),(F)),(r))
#define	VFS_ROOT(MP,VPP,r)		_VFSOP_(vfs_root,(MP),((MP),(VPP)),(r))
#define	VFS_QUOTACTL(MP,C,U,A,r)	_VFSOP_(vfs_quotactl,(MP),((MP),(C),(U),(A)),r)
#define	VFS_STATFS(MP,r)		_VFSOP_(vfs_statfs,(MP),(MP),(r))
#define	VFS_SYNC(MP,WF,r)		_VFSOP_(vfs_sync,(MP),((MP),(WF)),(r))
#define	VFS_FHTOVP(MP,FP,VP,r)		_VFSOP_(vfs_fhtovp,(MP),((MP),(FP),(VP)),(r))
#define	VFS_VPTOFH(VP,FIDP,r)		_VFSOPV_(vfs_vptofh,(VP),((VP),(FIDP)),(r))
#define	VFS_MOUNTROOT(MP,VPP,r)		_VFSOP_(vfs_mountroot,(MP),((MP),(VPP)),(r))
#define	VFS_SWAPVP(MP,VPP,r)		_VFSOP_(vfs_swapvp,(MP),((MP),(VPP)),(r))


/*
 * forcibly flags for vfs_umount().
 * waitfor flags to vfs_sync() and getfsstat()
 */
#define MNT_FORCE	0x1
#define MNT_NOFORCE	0x2
#define	MNT_SKIPSYSTEM	0x4
#define MNT_WAIT	0x1
#define MNT_NOWAIT	0x2

#ifdef	_KERNEL
/*
 * Mount structure locking constraints.
 *	Field		Constraint
 *	-----		----------
 *	m_next		mountlist_lock
 *	m_prev		mountlist_lock
 *	m_op		m_lock
 *	m_vnodecovered	read-only?	XXX
 *	m_mounth	m_vlist_lock
 *	m_flag		m_lock
 *	m_exroot	m_lock
 *	m_stat		m_lock
 *	m_data		read-only
 *
 * Additional mount synchronization constraints.  [XXX]
 */

/*
 * The mount lock protects the contents of the mount structure from
 * other processors.  Only needed for NCPUS > 1.
 */
#define	MOUNT_LOCK(mp)		usimple_lock(&(mp)->m_lock)
#define	MOUNT_UNLOCK(mp)	usimple_unlock(&(mp)->m_lock)
#define	MOUNT_LOCK_INIT(mp)	usimple_lock_init(&(mp)->m_lock)
#define	MOUNT_LOCK_HOLDER(mp)	SLOCK_HOLDER(&(mp)->m_lock)

/*
 * The mount vnode list lock guards the list of vnodes associated
 * with a mount point.  Only needed for NCPUS > 1.
 */
#define	MOUNT_VLIST_LOCK(mp)	usimple_lock(&(mp)->m_vlist_lock)
#define	MOUNT_VLIST_UNLOCK(mp)	usimple_unlock(&(mp)->m_vlist_lock)
#define	MOUNT_VLIST_LOCK_INIT(mp) usimple_lock_init(&(mp)->m_vlist_lock)

/*
 * The mount lookup lock synchronizes pathname and file handle
 * translation with mount and unmount operations.
 */
#define	MOUNT_LOOKUP_START(mp)		lock_read(&(mp)->m_lookup_lock)
#define	MOUNT_LOOKUP_TRY(mp)		lock_try_read(&(mp)->m_lookup_lock)
#define	MOUNT_LOOKUP(mp)						\
MACRO_BEGIN								\
	if (!MOUNT_LOOKUP_TRY(mp))					\
		panic("m_lookup_lock botch");				\
MACRO_END

#define	MOUNT_LOOKUP_DONE(mp)		lock_read_done(&(mp)->m_lookup_lock);
#define	MOUNT_DISABLE_LOOKUPS(mp)	lock_write(&(mp)->m_lookup_lock)
#define	MOUNT_ENABLE_LOOKUPS(mp)	lock_write_done(&(mp)->m_lookup_lock)
#define	MOUNT_LOOKUP_LOCK_INIT(mp)	lock_init2(&(mp)->m_lookup_lock,TRUE,\
						   LTYPE_MOUNT_LOOKUP);
#define	MOUNT_LOOKUPS_DISABLED(mp)	LOCK_HOLDER(&(mp)->m_lookup_lock)

/*
 * Mount list lock operations.
 */
#define	MOUNTLIST_LOCK()		usimple_lock(&mountlist_lock)
#define	MOUNTLIST_UNLOCK()		usimple_unlock(&mountlist_lock)
#define	MOUNTLIST_LOCK_INIT()		usimple_lock_init(&mountlist_lock)

/*
 * Lock operations on the vfssw.  Used for addition/deletion of
 * file systems.
 */
#define VFSSW_READ_LOCK()	lock_read(&vfssw_lock);
#define VFSSW_WRITE_LOCK()	lock_write(&vfssw_lock);
#define VFSSW_WRITE_UNLOCK()	lock_done(&vfssw_lock);
#define VFSSW_READ_UNLOCK()	lock_done(&vfssw_lock);
#define VFSSW_LOCK_INIT()	lock_init2(&vfssw_lock, TRUE, LTYPE_VFSSW);

#define NULLMOUNT	((struct mount *) 0)
#define DEADMOUNT	(&dead_mount)

/*
 * exported vnode operations and globals
 */
extern void	vfs_remove();		/* remove a vfs from mount list */
extern struct	mount *getvfs();	/* return vfs given fsid */
extern struct	mount *rootfs;		/* ptr to root mount structure */
extern struct	mount dead_mount; 	/* dead mount structure */
extern struct	vfsops *vfssw[];	/* mount filesystem type switch */
extern lock_data_t vfssw_lock;		/* guard vfssw */
udecl_simple_lock_data(extern,mountlist_lock)	/* guard mount list */

#if	MACH
extern zone_t	mount_zone;		/* dynamically allocated mounts */
#endif
extern int	nmount;			/* number of ufs mount structs */
extern int	nmount_max;		/* number of entries in mount_zone */
					/* XXX should be combined XXX */
#endif	/* _KERNEL */
#endif	/* _SYS_MOUNT_H_ */
