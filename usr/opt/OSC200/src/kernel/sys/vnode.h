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
 *	@(#)$RCSfile: vnode.h,v $ $Revision: 4.3.18.7 $ (DEC) $Date: 1993/12/17 01:38:05 $
 */ 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
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

#ifndef _SYS_VNODE_H_
#define	_SYS_VNODE_H_

#include <sys/secdefines.h>
#include <machine/endian.h>
#ifdef	_KERNEL
#include <sys/unix_defs.h>
#endif
#if	SEC_FSCHANGE
#include <sys/security.h>
#endif
#ifdef	_KERNEL
#include <sys/vp_swap.h>
#endif

/*
 * The vnode is the focus of all file activity in UNIX.
 * There is a unique vnode allocated for each active file,
 * each current directory, each mounted-on file, text file, and the root.
 */

/*
 * vnode types. VNON means no type.
 */
enum vtype 	{ VNON, VREG, VDIR, VBLK, VCHR, VLNK, VSOCK, VFIFO, VBAD };

/*
 * Vnode tag types.
 * These are for the benefit of external programs only (e.g., pstat)
 * and should NEVER be inspected inside the kernel.
 */
enum vtagtype { VT_NON, VT_UFS, VT_NFS, VT_MFS, VT_S5FS, VT_CDFS, VT_DFS,
		  VT_EFS, VT_PRFS, VT_MSFS, VT_FFM, VT_FDFS, VT_ADDON };

/*
 * This defines the maximum size of the private data area
 * permitted for any file system type.  Vn_maxprivate is
 * set early in the boot sequence based on the largest
 * filesystem-specific representation.  The actual size
 * of a vnode will depend on vn_maxprivate at run-time.
 */
extern int	vn_maxprivate;

struct vnode {
	u_int		v_flag;			/* vnode flags (see below) */
	uint_t		v_usecount;		/* reference count of users */
	int		v_holdcnt;		/* page & buffer references */
	u_short		v_shlockc;		/* count of shared locks */
	u_short		v_exlockc;		/* count of exclusive locks */
	off_t		v_lastr;		/* last read (read-ahead) */
	u_int		v_id;			/* capability identifier */
	struct mount	*v_mount;		/* ptr to vfs we are in */
	struct vnodeops	*v_op;			/* vnode operations */
	struct vnode	*v_freef;		/* vnode freelist forward */
	struct vnode	**v_freeb;		/* vnode freelist back */
	struct vnode	*v_mountf;		/* vnode mountlist forward */
	struct vnode	**v_mountb;		/* vnode mountlist back */
	struct buf	*v_cleanblkhd;		/* clean blocklist head */
	struct buf	*v_dirtyblkhd;		/* dirty blocklist head */
	int		v_numoutput;		/* num of writes in progress */
	u_int		v_outflag;		/* output flags */
	enum vtype	v_type;			/* vnode type */
	union {
		struct mount	*vu_mountedhere;/* ptr to mounted vfs (VDIR) */
		struct socket	*vu_socket;	/* unix ipc (VSOCK) */
		struct specinfo	*vu_specinfo;	/* device specinfo structure */
	} v_un;
	enum vtagtype	v_tag;			/* type of underlying data */
	u_short         v_rdcnt;                /* count of readers */
	u_short         v_wrcnt;                /* count of writers */
	struct vm_object
			*v_object;		/* VM object for vnode */
#if SEC_FSCHANGE
	struct vnsecops	*v_secop;		/* vnode security operations */
#endif
#ifdef	_KERNEL
	udecl_simple_lock_data(,v_buflists_lock)/* protect clean/dirty blkhd */
	udecl_simple_lock_data(,v_output_lock)	/* protect numoutput, outflag */
	udecl_simple_lock_data(,v_lock)		/* multiprocessor exclusion */
	lock_data_t	v_aux_lock;		/* auxiliary sleep lock */
	
#endif
	char v_data[1];				/* placeholder, private data */
};
#define v_mountedhere v_un.vu_mountedhere
#define v_socket v_un.vu_socket
#define v_specinfo v_un.vu_specinfo

/*
 * vnode flags.
 */
#define	VROOT		0x0001	 /* root of its file system */
#define	VTEXT		0x0002	 /* vnode is a pure text prototype */
#define	VXLOCK		0x0004	 /* vnode is locked to change underlying type */
#define	VXWANT		0x0008	 /* process is waiting for vnode */
#define	VEXLOCK		0x0010	 /* exclusive lock */
#define	VSHLOCK		0x0020	 /* shared lock */
#define	VLWAIT		0x0040	 /* proc is waiting on shared or excl. lock */
#define	VMOUNTING	0x0100	 /* file system in transistion */
#define	VMOUNTWAIT	0x0200	 /* proc waiting for filesystem to stabilize */
#define VFLOCK		0x0400	 /* vnode is locked for first fifo open */
#define VFWAIT		0x0800	 /* waiting for fifo open */
#define	VSYSTEM		0x1000	 /* vnode being used by kernel */
#define VENF_LOCK       0x2000   /* mandatory file locking enabled */
#define VLOCKS          0x4000   /* fcntl file locks on file */
#define	VSWAP		0x8000	 /* swapping on this vnode */
#define	VCOBJECT     	0x10000  /* creating vnode object */
#define	VCWAIT		0x20000  /* waiting for creation to complete */
#define V_PRIVATE       0x40000  /* private DFS vnode; don't place on free chain */
#define V_CONVERTED     0x80000  /* vnode converted for VFS+ usage */
#define VNOAUDIT        0x100000 /* bypass data access auditing */
#define VIOERROR	0x200000 /* Write error */
#if     SEC_FSCHANGE
#define VMLD	    0x10000000   /* vnode is a multilevel directory */
#endif 
#ifdef i386
#define VXENIX      0x20000000   /* xenix file locks on file */
#endif
#define VDUP      0x40000000  /* duplicate on open(); for FDFS */

/*
 * vnode output flags
 */
#define	VOUTWAIT	0x0001	 /* waiting for output to complete */

/*
 * Operations on vnodes.
 */
struct vnodeops {
	int	(*vn_lookup)(		/* vp, ndp */ );
	int	(*vn_create)(		/* ndp, vap */ );
	int	(*vn_mknod)(		/* ndp, vap, cred */ );
	int	(*vn_open)(		/* vp, fflags, cred */ );
	int	(*vn_close)(		/* vp, fflags, cred */ );
	int	(*vn_access)(		/* vp, fflags, cred */ );
	int	(*vn_getattr)(		/* vp, vap, cred */ );
	int	(*vn_setattr)(		/* vp, vap, cred */ );

	int	(*vn_read)(		/* vp, uiop, ioflag, cred */ );
	int	(*vn_write)(		/* vp, uiop, ioflag, cred */ );
	int	(*vn_ioctl)(		/* vp, com, data, fflag, cred */ );
	int	(*vn_select)(	    /* vp, events, revents, scanning, cred */ );
	int	(*vn_mmap)(		/* vp, off, map, addrp, len,
						prot, maxprot, flags, cred */ );
	int	(*vn_fsync)(		/* vp, fflags, cred, waitfor */ );
	int	(*vn_seek)(		/* vp, (old)offp, off, whence */ );

	int	(*vn_remove)(		/* ndp */ );
	int	(*vn_link)(		/* vp, ndp */ );
	int	(*vn_rename)(		/* fndp, tndp */ );
	int	(*vn_mkdir)(		/* ndp, vap */ );
	int	(*vn_rmdir)(		/* ndp */ );
	int	(*vn_symlink)(		/* ndp, vap, name */ );
	int	(*vn_readdir)(		/* vp, uiop, cred, eofflagp */ );
	int	(*vn_readlink)(		/* vp, uiop, cred */ );

	int	(*vn_abortop)(		/* ndp */ );
	int	(*vn_inactive)(		/* vp */ );
	int	(*vn_reclaim)(		/* vp */ );

	int	(*vn_bmap)(		/* vp, bn, vpp, bnp */ );
	int	(*vn_strategy)(		/* bp */ );

	int	(*vn_print)(		/* vp */ );
	int	(*vn_pgrd)(		/* vp, uiop, cred */ );
	int	(*vn_pgwr)(		/* vp, uiop, cred, pager, offset */ );
	int	(*vn_getpage)(		/* vp, off, len, protp, pl, plsz,
						mape, addr, rw, cred */ );
	int	(*vn_putpage)(		/* vp, pl, pcnt, flags, cred */);
	int	(*vn_swap)(		/* vp, swapop, argp */ );
	int	(*vn_bread)(		/* vp, lbn, bpp, cred */ );
	int	(*vn_brelse)(		/* vp, bp */ );
	int	(*vn_lockctl)(	        /* vp, eld, flag, cred, pid, offset */);
	int	(*vn_syncdata)(		/* vp, flag, offset, length, cred */ );
};

/*
 * Invoking a vnode operation implies funnelling when SER_COMPAT
 * is turned on.
 */

#define	_VOP_(f,v,arg,r)						\
MACRO_BEGIN								\
	struct mount *_vop_mp = (v)->v_mount;				\
									\
	MOUNT_FUNNEL(_vop_mp);						\
	(r) = (*((v)->v_op->f))arg;					\
	MOUNT_UNFUNNEL(_vop_mp);					\
MACRO_END

/* Macros to call the vnode ops */
#define	VOP_LOOKUP(v,n,r)	_VOP_(vn_lookup,(v),((v),(n)),(r))
#define	VOP_CREATE(n,a,r)	_VOP_(vn_create,(n)->ni_dvp,((n),(a)),(r))
#define	VOP_MKNOD(n,a,c,r)	_VOP_(vn_mknod,(n)->ni_dvp,((n),(a),(c)),(r))
#define	VOP_OPEN(vpp,f,c,r)	_VOP_(vn_open,(*vpp),((vpp),(f),(c)),(r))
#define	VOP_CLOSE(v,f,c,r)	_VOP_(vn_close,(v),((v),(f),(c)),(r))
#define	VOP_ACCESS(v,f,c,r)	_VOP_(vn_access,(v),((v),(f),(c)),(r))
#define	VOP_GETATTR(v,a,c,r)	_VOP_(vn_getattr,(v),((v),(a),(c)),(r))
#define	VOP_SETATTR(v,a,c,r)	_VOP_(vn_setattr,(v),((v),(a),(c)),(r))
#define	VOP_READ(v,u,i,c,r)	_VOP_(vn_read,(v),((v),(u),(i),(c)),(r))
#define	VOP_WRITE(v,u,i,c,r)	_VOP_(vn_write,(v),((v),(u),(i),(c)),(r))
#define	VOP_IOCTL(v,o,d,f,c,r,rv) _VOP_(vn_ioctl,(v),((v),(o),(d),(f),(c),(rv)),(r))
#define	VOP_SELECT(v,e,r,s,c,x)	_VOP_(vn_select,(v),((v),(e),(r),(s),(c)),(x))
#define	VOP_MMAP(v,o,m,a,l,p,mp,f,c,r)				\
		_VOP_(vn_mmap,(v),((v),(o),(m),(a),(l),(p),(mp),(f),(c)),r)
#define	VOP_FSYNC(v,f,c,w,r)	_VOP_(vn_fsync,(v),((v),(f),(c),(w)),(r))
#define	VOP_SEEK(v,p,o,w,r)	_VOP_(vn_seek,(v),((v),(p),(o),(w)),(r))
#define	VOP_REMOVE(n,r)		_VOP_(vn_remove,(n)->ni_dvp,((n)),(r))
#define	VOP_LINK(v,n,r)		_VOP_(vn_link,(n)->ni_dvp,((v),(n)),(r))
#define	VOP_RENAME(s,t,r)	_VOP_(vn_rename,(s)->ni_dvp,((s),(t)),(r))
#define	VOP_MKDIR(n,a,r)	_VOP_(vn_mkdir,(n)->ni_dvp,((n),(a)),(r))
#define	VOP_RMDIR(n,r)		_VOP_(vn_rmdir,(n)->ni_dvp,((n)),(r))
#define	VOP_SYMLINK(n,a,m,r)	_VOP_(vn_symlink,(n)->ni_dvp,((n),(a),(m)),(r))
#define	VOP_READDIR(v,u,c,e,r)	_VOP_(vn_readdir,(v),((v),(u),(c),(e)),(r))
#define	VOP_READLINK(v,u,c,r)	_VOP_(vn_readlink,(v),((v),(u),(c)),(r))
#define	VOP_ABORTOP(n,r)	_VOP_(vn_abortop,(n)->ni_dvp,((n)),(r))
#define	VOP_INACTIVE(v,r)	_VOP_(vn_inactive,(v),((v)),(r))
#define	VOP_RECLAIM(v,r)	_VOP_(vn_reclaim,(v),((v)),(r))
#define	VOP_BMAP(v,s,p,n,r)	_VOP_(vn_bmap,(v),((v),(s),(p),(n)),(r))
#define	VOP_STRATEGY(b,r)	_VOP_(vn_strategy,(b)->b_vp,((b)),(r))
#define	VOP_PRINT(v,r)		_VOP_(vn_print,(v),((v)),(r))
#define	VOP_PGRD(v,u,c,r)	_VOP_(vn_pgrd,(v),((v),(u),(c)),(r))
#define	VOP_PGWR(v,u,c,p,o,r)	_VOP_(vn_pgwr,(v),((v),(u),(c),(p),(o)),(r))
#define	VOP_GETPAGE(v, o, l, pt, pl, plsz, m, a, rw, c, r)		\
	_VOP_(vn_getpage,(v),((v),(o),(l),(pt),(pl),(plsz),(m),(a),(rw),(c)),r)
#define VOP_PUTPAGE(v, pl, pcnt, f, c, r)				\
	_VOP_(vn_putpage,(v),((v),(pl),(pcnt),(f),(c)),r)
#define	VOP_SWAP(v,o,a,c,r)	_VOP_(vn_swap,(v),((v),(o),(a),(c)),r)
#define	VOP_BREAD(v,l,b,c,r)	_VOP_(vn_bread,(v),((v),(l),(b),(c)),r)
#define	VOP_BRELSE(v,b,r)	_VOP_(vn_brelse,(v),((v),(b)),r)
#define VOP_LOCKCTL(v,l,cm,cr,p,o,r) \
	_VOP_(vn_lockctl,(v),((v),(l),(cm),(cr),(p),(o)),(r))
#define VOP_SYNCDATA(v,f,o,l,c,r) \
	_VOP_(vn_syncdata,(v),((v),(f),(o),(l),(c)),(r))

/*
 * flags for ioflag
 */
#define IO_UNIT		0x01		/* do I/O as atomic unit */
#define IO_APPEND	0x02		/* append write to end */
#define IO_SYNC		0x04		/* do I/O synchronously */
#define IO_NODELOCKED	0x08		/* underlying node already locked */
#define	IO_NDELAY	0x10		/* FNDELAY flag set in file table */
#define IO_NONBLOCK     0x80            /* FNONBLOCK flag set in file table */
/*
 * DEC/OSF advisory flags for VOP_WRITE calls.
 * These flags are additive to other ioflag values.
 * Local filesystem types need not necessarily recognize them.
 */
#define	IO_DATAONLY	0x20		/* synchronously write data only */
                                        /* (no metadata) for IO_SYNC write */
                                        /* request (additive with IO_SYNC). */
#define IO_DELAYDATA	0x40		/* delay data write as long as */
                                        /* possible for async write request */
                                        /* (additive with "IO_ASYNC", which */
                                        /*  is 0x0). */ 

/*
 * Vnode attributes.  A field value of VNOVAL
 * represents a field whose value is unavailable
 * (getattr) or which is not to be changed (setattr).
 */
struct vattr {
	enum vtype	va_type;	/* vnode type (for create) */
	u_short		va_mode;	/* files access mode and type */
	short		va_nlink;	/* number of references to file */
	uid_t		va_uid;		/* owner user id */
	gid_t		va_gid;		/* owner group id */
	int		va_fsid;	/* file system id (dev for now) */
	int		va_fileid;	/* file id */
#if __alpha
	u_long		va_qsize;	/* file size in bytes */
#else
	quad		va_qsize;	/* file size in bytes */
#endif
	int		va_blocksize;	/* blocksize preferred for i/o */
	struct timeval	va_atime;	/* time of last access */
	struct timeval	va_mtime;	/* time of last modification */
	struct timeval	va_ctime;	/* time file changed */
	u_int		va_gen;		/* generation number of file */
	u_int		va_flags;	/* flags defined for file */
	dev_t		va_rdev;	/* device special file represents */
#if __alpha
	u_long		va_qbytes;	/* bytes of disk space held by file */
#else
	quad		va_qbytes;	/* bytes of disk space held by file */
#endif
	union {
		char	*vau_symlink;	/* name of symlink to be created */
		char	*vau_socket;	/* address of socket (XX make void *) */
	} va_un;
};

#define va_symlink      va_un.vau_symlink
#define va_socket       va_un.vau_socket

#if __alpha
#define	va_size		va_qsize
#define	va_bytes	va_qbytes
#else
#if	BYTE_ORDER == LITTLE_ENDIAN
#define	va_size		va_qsize.val[0]
#define	va_size_rsv	va_qsize.val[1]
#define	va_bytes	va_qbytes.val[0]
#define	va_bytes_rsv	va_qbytes.val[1]
#else
#define	va_size		va_qsize.val[1]
#define	va_size_rsv	va_qsize.val[0]
#define	va_bytes	va_qbytes.val[1]
#define	va_bytes_rsv	va_qbytes.val[0]
#endif
#endif

#if	SEC_FSCHANGE

#define	VHASSECOPS(vp)	((vp)->v_secop)
#define	VSECURE(vp)	((vp)->v_mount->m_flag & M_SECURE)

struct vnsecops {
	int	(*vn_getsecattr)	( /* vp, vsap, cred */ );
	int	(*vn_setsecattr)	( /* vp, vsap, cred */ );
	int	(*vn_dirempty)		( /* vp, dvp, cred */ );
};

#define	VOP_GETSECATTR(v,a,c,r)	(r) = (*((v)->v_secop->vn_getsecattr))((v),(a),(c))
#define	VOP_SETSECATTR(v,a,c,r)	(r) = (*((v)->v_secop->vn_setsecattr))((v),(a),(c))
#define	VOP_DIREMPTY(v,p,c,r)	(r) = (*((v)->v_secop->vn_dirempty))((v),(p),(c))

struct vsecattr {
	u_short		vsa_valid;	/* which fields are valid (see below) */
	u_char		vsa_policy;	/* policy index for vsa_tag */
	u_char		vsa_tagnum;	/* policy-relative tag index */
	struct vnode	*vsa_parent;	/* parent vnode for tag changes */
	tag_t		vsa_tags[SEC_TAG_COUNT];	/* tag pool */
	privvec_t	vsa_gpriv;	/* granted privileges */
	privvec_t	vsa_ppriv;	/* potential privileges */
	u_int		vsa_type_flags;	/* type flags (MLD, 2 person, etc.) */
	uid_t		vsa_uid;	/* POSIX ACL uid(result of tag change)*/
	gid_t		vsa_gid;	/* POSIX ACL group ID (ditto) */
	mode_t		vsa_mode;	/* POSIX ACL mode (ditto) */
};
#define	vsa_tag	vsa_tags[0]

/* Flag values for vsa_valid field */
#define	VSA_TAG		0x01
#define	VSA_GPRIV	0x02
#define	VSA_PPRIV	0x04
#define	VSA_TYPE_FLAGS	0x08
#define	VSA_ALLTAGS	0x10
/*
 * POSIX ACL
 */
#define VSA_UID		0x20
#define VSA_GID		0x40
#define VSA_MODE	0x80

#endif	/* SEC_FSCHANGE */

/*
 *  Modes. Some values same as Ixxx entries from inode.h for now
 */
#define	VSUID	04000		/* set user id on execution */
#define	VSGID	02000		/* set group id on execution */
#define	VSVTX	01000		/* save swapped text even after use */
#define	VREAD	0400		/* read, write, execute permissions */
#define	VWRITE	0200
#define	VEXEC	0100

/*
 * Token indicating no attribute value yet assigned
 */
#define VNOVAL	((unsigned long)0xffffffffffffffff)

#ifdef	_KERNEL
/*
 * Vnode locking constraints.
 *	Field			Comment
 *	-----			-------
 *	v_flag			v_lock
 *	v_usecount		v_lock
 *	v_holdcnt		v_lock
 *	v_shlockc		v_lock
 *	v_exlockc		v_lock
 *	v_lastr			v_lock
 *	v_id			v_lock
 *	v_mount			read-only?	XXX
 *	v_op			v_lock
 *	v_freef			v_free_lock
 *	v_freeb			v_free_lock
 *	v_mountf		m_vlist_lock
 *	v_mountb		m_vlist_lock
 *	v_cleanblkhd		v_buflists_lock
 *	v_dirtyblkhd		v_buflists_lock
 *	v_numoutput		v_output_lock
 *	v_outflag		v_output_lock
 *	v_type			v_lock
 *	vu_mountedhere		v_lock + VMOUNTING (r/o except for force unmnt)
 *	vu_socket		v_lock
 *	vu_nextalias		speclist hashchain lock
 *	v_rdev			v_lock
 *	v_tag			v_lock
 *	v_rdcnt			v_lock
 *	v_wrcnt			v_lock
 *	v_vm_info		read-only
 *	v_*			v_aux_lock (see note below)
 *
 * Special privileges are accorded unparallelized filesystem types,
 * as follows.  Manipulation of globally used fields (e.g. lists) is
 * not allowed by the filesystem, and is done under lock in the proper
 * places.  The private fields are not subject to locking, since when
 * the filesystem is running, it is guaranteed to be funnelled onto the
 * master processor.  It is, however, still subject to normal races, for
 * example, any operation that can sleep may need to provide its own
 * synchronization.
 *
 * The following additional synchronization requirements apply.
 * [expand on VXLOCK, VMOUNTING, etc.].
 *
 * Note about v_vm_info.  Currently, v_vm_info is set at boot-time to
 * point to the vnode's associated v_vm_info structure.  This field is
 * never re-set, not even by vgone, so no lock is needed to use the field.
 *
 * However, it must also be noted that OSF/1 does not currently support
 * the MACH_NBC option.  There are races with the filesystem's use of the
 * vm_info's pager field that must first be resolved before MACH_NBC will
 * work.  The current code does use the pager field but all of these uses
 * are not guaranteed at this time.  There could be problems with the
 * use of the v_vm_info->pager field during mapping and unmapping of
 * files. [XXX]
 *
 * The vm_info structure also has credentials, currently used only by 
 * the paging file code.  These credentials do not change during the
 * life of the paging file.
 *
 * Note about v_aux_lock. This general purpose sleep lock is supplied
 * for use by any kernel subsystem for its own exclusion needs (e.g.
 * between NFS server daemons). It is NOT to be used as an exclusion
 * mechanism between non-cooperating subsystems. *NO* lock semantics
 * should be assumed.
 */

/*
 * The vnode lock protects the contents of the vnode structure;
 * its only purpose is multiprocessor exclusion.  Long-term locking
 * is a privilege reserved for the filesystem-specific layers.
 */
#define	VN_LOCK(vp)		usimple_lock(&(vp)->v_lock)
#define	VN_UNLOCK(vp)		usimple_unlock(&(vp)->v_lock)
#define	VN_LOCK_TRY(vp)		usimple_lock_try(&(vp)->v_lock)
#define	VN_LOCK_INIT(vp)	usimple_lock_init(&(vp)->v_lock)
#define	VN_LOCK_HOLDER(vp)	SLOCK_HOLDER(&(vp)->v_lock)

/*
 * The vnode buffer lists lock protects both the clean and dirty
 * buffer lists in the vnode.
 */
#define	VN_BUFLISTS_LOCK(vp)	usimple_lock(&(vp)->v_buflists_lock)
#define	VN_BUFLISTS_UNLOCK(vp)	usimple_unlock(&(vp)->v_buflists_lock)
#define	VN_BUFLISTS_LOCK_INIT(vp) usimple_lock_init(&(vp)->v_buflists_lock)

/*
 * The vnode output lock protects the number of writes in progress and
 * the output flags.
 */
#define	VN_OUTPUT_LOCK(vp)	usimple_lock(&(vp)->v_output_lock)
#define	VN_OUTPUT_UNLOCK(vp)	usimple_unlock(&(vp)->v_output_lock)
#define	VN_OUTPUT_LOCK_INIT(vp) usimple_lock_init(&(vp)->v_output_lock)

/*
 * The vnode auxiliary lock provides general purpose vnode exclusion.
 * *NO* lock semantics should be assumed.
 * See note above.
 */
#define	VN_READ_LOCK(vp)	lock_read(&(vp)->v_aux_lock)
#define	VN_WRITE_LOCK(vp)	lock_write(&(vp)->v_aux_lock)
#define	VN_WRITE_LOCK_TRY(vp)	lock_try_write(&(vp)->v_aux_lock)
#define	VN_READ_UNLOCK(vp)	lock_read_done(&(vp)->v_aux_lock)
#define	VN_WRITE_UNLOCK(vp)	lock_write_done(&(vp)->v_aux_lock)
#define	VN_SET_RECURSIVE(vp)	lock_set_recursive(&(vp)->v_aux_lock)
#define VN_CLEAR_RECURSIVE(vp)	lock_clear_recursive(&(vp)->v_aux_lock)
#define	VN_AUX_LOCK_INIT(vp)	lock_init2(&(vp)->v_aux_lock, TRUE, \
					   LTYPE_VNODE_AUX)
#define	VN_WRITE_HOLDER(vp)	LOCK_HOLDER(&vp->v_aux_lock)
#define	VN_READ_HOLDER(vp)	((vp)->v_aux_lock.read_count >= 1)

/*
 * public vnode manipulation functions
 */
extern int vn_open();			/* open vnode */
extern int vn_rdwr();			/* read or write vnode */
extern int vn_close();			/* close vnode */
extern int getnewvnode();		/* allocate a new vnode */
extern struct vnode *shadowvnode();	/* get shadow vnode for block device */
extern int bdevvp();			/* allocate a new special dev vnode */
extern int makealias();			/* make special device aliases */
extern int specalloc();			/* allocate specinfo structure */
extern int vgetm();			/* get reference to a vnode */
extern void vref();			/* increase reference to a vnode */
extern void vrele();			/* release vnode */
extern int vgone();			/* completely recycle vnode */
extern void clearalias();		/* recycle vnode and all its aliases */

/*
 * Synchronization notes:
 * Vgone and vgetm will return a 1 if the vnode had VXLOCK set, 0 if it
 * did not.  In the case of a vnode with VXLOCK set, neither routine will
 * successfully complete, and will wait for the lock to clear, depending
 * upon the arguments passed.
 *
 * Args passed to vgone to indicate whether or not to sleep on a VXLOCK
 */
#define VX_NOSLEEP      0
#define VX_SLEEP        1


/*
 * Two flavors of vget (both return 0 if you get the reference):
 *	vget(vp) -- sleep on VXLOCK; always gets the reference.
 *	vget_nowait(vp) -- do not sleep on VXLOCK; return 1 immediately if set.
 */
#define vget(vp)	vgetm(vp, 1)	/* get reference, sleep on VXLOCK */
#define vget_nowait(vp)	vgetm(vp, 0)	/* if VXLOCK set, return 1, no ref. */

/*
 * Flags to various vnode functions.
 */
#define	SKIPSYSTEM	0x0001		/* vflush: skip VSYSTEM vnodes */
#define	FORCECLOSE	0x0002		/* vflush: force file closure */
#define	DOCLOSE		0x0004		/* vclean: close active files */

/*
 * Inline references for non-debug kernels
 */
#if	MACH_ASSERT
extern void vattr_null();		/* set attributes to null */
#define	VREF(vp)	vref(vp)	/* increase reference to a vnode */
#define VHOLD(vp)	vhold(vp)	/* increase buf or page ref to vnode */
#define HOLDRELE(vp)	holdrele(vp)	/* decrease buf or page ref to vnode */
#define VATTR_NULL(vap) vattr_null(vap) /* initialize a vattr structure */

#else	/* MACH_ASSERT */

#define VREF(vp)						\
MACRO_BEGIN							\
	VN_LOCK(vp);						\
	(vp)->v_usecount++;					\
	VN_UNLOCK(vp);						\
MACRO_END

#define VHOLD(vp)						\
MACRO_BEGIN							\
	VN_LOCK(vp);						\
	(vp)->v_holdcnt++;					\
	VN_UNLOCK(vp);						\
MACRO_END

#define HOLDRELE(vp)						\
MACRO_BEGIN							\
	VN_LOCK(vp);						\
	(vp)->v_holdcnt--;					\
	VN_UNLOCK(vp);						\
MACRO_END

#define VATTR_NULL(vap) *(vap) = va_null /* initialize a vattr structure */
#define vattr_null(vap) VATTR_NULL(vap);

#endif	/* MACH_ASSERT */

#define	VUNREF(vp)	vrele(vp)	 /* decrease reference to a vnode */
#define NULLVP	((struct vnode *)0)


/*
 * Global vnode data.
 */
extern	struct vnode *rootdir;		/* root (i.e. "/") vnode */

extern	struct vnode *vnode;		/* The vnode table itself */
extern	struct vnode *vnodeNVNODE;	/* The end of the vnode table */
extern	int nvnode;			/* number of slots in the table */
extern	long desiredvnodes;		/* number of vnodes desired */
extern	struct vattr va_null;		/* predefined null vattr structure */
#endif	/* _KERNEL */
#endif	/* _SYS_VNODE_H_ */
