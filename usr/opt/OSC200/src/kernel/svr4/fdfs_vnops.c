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
 * See the comment explaining this in fdfs_getattr().
 */
#define	STAT_BROKEN	1

#include <sys/types.h>
#include <kern/mfs.h>
#include <sys/time.h>
#include <sys/vnode.h>
#include <sys/namei.h>
#include <sys/errno.h>
#include <sys/kernel.h>
#include <sys/access.h>

#include <mach/mach_types.h>
#include <vm/vm_page.h>
#include <vm/vm_vppage.h>
#include <sys/vfs_ubc.h>
#include <vm/vm_mmap.h>
#include <vm/vm_vp.h>
#include <vm/vm_debug.h>

#include <sys/fdfs.h>

/*
 * Global vfs data structures for fdfs
 */

static int	fdfs_lookup();
static int	fdfs_create();
static int	fdfs_mknod();
static int	fdfs_open();
static int	fdfs_close();
static int	fdfs_access();
static int	fdfs_getattr();
static int	fdfs_setattr();
static int	fdfs_read();
static int	fdfs_write();
static int	fdfs_ioctl();
static int	fdfs_seltrue();
static int	fdfs_mmap();
static int	fdfs_fsync();
static int	fdfs_seek();
static int	fdfs_remove();
static int	fdfs_link();
static int	fdfs_rename();
static int	fdfs_mkdir();
static int	fdfs_rmdir();
static int	fdfs_symlink();
static int	fdfs_readdir();
static int	fdfs_readlink();
static int	fdfs_abortop();
static int	fdfs_inactive();
static int	fdfs_reclaim();
static int	fdfs_bmap();
static int	fdfs_strategy();
static int	fdfs_print();
static int	fdfs_page_read();
static int	fdfs_page_write();
static int	fdfs_getpage();
static int	fdfs_putpage();
static int	fdfs_swap();
static int	fdfs_bread();
static int	fdfs_brelse();
static int	fdfs_lockctl();
static int	fdfs_syncdata();

struct vnodeops fdfs_vnodeops = {
	fdfs_lookup, 		/* lookup */
	fdfs_create, 		/* create */
	fdfs_mknod, 		/* mknod */
	fdfs_open, 		/* open */
	fdfs_close, 		/* close */
	fdfs_access, 		/* access */
	fdfs_getattr, 		/* getattr */
	fdfs_setattr, 		/* setattr */
	fdfs_read, 		/* read */
	fdfs_write, 		/* write */
	fdfs_ioctl, 		/* ioctl */
	fdfs_seltrue, 		/* select */
	fdfs_mmap, 		/* mmap */
	fdfs_fsync, 		/* fsync */
	fdfs_seek, 		/* seek */
	fdfs_remove, 		/* remove */
	fdfs_link, 		/* link */
	fdfs_rename, 		/* rename */
	fdfs_mkdir, 		/* mkdir */
	fdfs_rmdir, 		/* rmdir */
	fdfs_symlink, 		/* symlink */
	fdfs_readdir, 		/* readdir */
	fdfs_readlink, 	/* readlink */
	fdfs_abortop, 		/* abortop */
	fdfs_inactive, 	/* inactive */
	fdfs_reclaim, 		/* reclaim */
	fdfs_bmap, 		/* bmap */
	fdfs_strategy, 	/* strategy */
	fdfs_print, 		/* print */
	fdfs_page_read, 	/* page_read */
	fdfs_page_write, 	/* page_write */
	fdfs_getpage, 		/* get page */
	fdfs_putpage, 		/* put page */
	fdfs_swap, 		/* swap handler */
	fdfs_bread, 		/* buffer read */
	fdfs_brelse, 		/* buffer release */
	fdfs_lockctl, 		/* file locking */
	fdfs_syncdata, 	/* fsync byte range */
};


/*
 * The major number for FDFS is chosen by reserving an entry
 * in cdevsw[] (kernel/io/common/conf.c).  That entry's index
 * is stored in a variable named fdfs_major.  If that file is
 * recompiled, this file should also be recompiled.
 */
extern int	fdfs_major;

/*
 * Convert a component of a pathname into a pointer to a locked inode.
 * The flag argument is LOOKUP, CREATE, RENAME, or DELETE depending on
 * whether the name is to be looked up, created, renamed, or deleted.
 * The only real operation used for FDFS is LOOKUP.  RENAME and DELETE
 * should never be specified.  When CREATE is only, the open() has its
 * OCREAT flag specified.  If the OEXCL is also specified, if the requested
 * file is found, the EEXIST error would be returned.  However, the flags
 * are supposed to be ignored, so this wouldn't do.  To circumvent the
 * problem, whenever the request is CREATE, we pretend to be unable to
 * find the file, whether its name exists or not.  Then, when VOP_CREATE is
 * called, we return the proper vnode.
 */

static int	
fdfs_lookup(vp, ndp)
struct vnode *vp;
register struct nameidata *ndp;
{
	/*
   * Ensure that the directory is FDFS root.
   * Also ensure that we are not attempting a RENAME or DELETE operation.
   */
	ASSERT(vp == fdfs_root_directory);
	ASSERT(!(ndp->ni_nameiop & DELETE));
	ASSERT(!(ndp->ni_nameiop & RENAME));
	ASSERT(vp->v_usecount);

	/*
   * If there is success, we return the vnode in ni_vp.  If not, this field is required to
   * be NULL.
   */
	ndp->ni_dvp = vp;
	ndp->ni_vp = NULL;

	/*
   * If the requested operation is CREATE, we already know that we "can't" find
   * the requested entry.  See the comment at the beginning of fdfs_lookup() for
   * rationale.
   */
	if (ndp->ni_nameiop & CREATE) {
		return (ENOENT);
	}

	/*
   * We can be asked for the value of '.'.  '..' should be taken care of
   * by namei(), and so we needn't worry about it.
   */
	if ((ndp->ni_namelen == 1) && (*(ndp->ni_ptr) == '.')) {
		ndp->ni_vp = vp;
		VREF(vp);		/* XXX is this necessary? */
	} else
	 {
		char	*nameptr;
		ino_t fd_number;
		int char_cnt, error;
		struct vnode *fdfs_ptr;

		/*
     * Cycle through the file name, allowing the minor number to wrap.  This is
     * exactly how SVR4 does it.
     */
		for (nameptr = ndp->ni_ptr, fd_number = 0, char_cnt = ndp->ni_namelen; 
		    char_cnt--; nameptr++) {
			if ((*nameptr < '0') || (*nameptr > '9')) {
				return (ENOENT);
			} else
			 {
				fd_number = (fd_number * 10) + (int)(*nameptr - '0');
			}
		}

		/*
     * Allocate a new vnode and initialize its content.  It will be freed once
     * again in the open() code because VDUP is set.
     */
		if (error = getnewvnode(VT_FDFS, &fdfs_vnodeops, &fdfs_ptr)) {
			return (error);
		}

		/*
     * See the comment explaining this in fdfs_getattr().
     */
#if STAT_BROKEN == 0
		fdfs_ptr->v_type = VREG;
#else    
		fdfs_ptr->v_type = VCHR;
#endif
		fdfs_ptr->v_flag |= VDUP;
		fdfs_ptr->v_object = VM_OBJECT_NULL;
		FILE_DESCRIPTOR(fdfs_ptr) = minor(makedev(0, fd_number));
		insmntque(fdfs_ptr, fdfs_root_directory->v_mount);

		ndp->ni_vp = fdfs_ptr;
	}
	return (0);
}


static int	
fdfs_create(ndp, vap)
struct nameidata *ndp;
struct vattr *vap;
{
	short	old_nameiop;
	int	error;

	old_nameiop = ndp->ni_nameiop;
	ndp->ni_nameiop = LOOKUP | FOLLOW;
	error = namei(ndp);
	ndp->ni_nameiop = old_nameiop;
	return (error);
}


/*
 * Mknod vnode call
 */
static int	
fdfs_mknod(ndp, vap, cred)
struct nameidata *ndp;
struct ucred *cred;
register struct vattr *vap;
{
	return (ENOSYS);
}


/*
 * Open called.
 *
 * Nothing to do.
 */
static int	
fdfs_open(vpp, mode, cred)
struct vnode **vpp;
int	mode;
struct ucred *cred;
{
	return (0);
}


static int	
fdfs_close(vp, fflag, cred)
struct vnode *vp;
int	fflag;
struct ucred *cred;
{
	return (ENOSYS);
}


static int	
fdfs_access(vp, mode, cred)
struct vnode *vp;
int	mode;
struct ucred *cred;
{
	return (0);
}


static int	
fdfs_getattr(vp, vap, cred)
struct vnode *vp;
register struct vattr *vap;
struct ucred *cred;
{
	int	s;

	bzero(vap, sizeof(struct vattr ));

	/*
   * Normally, the vnode would have to be protected
   * while getting the type, but we assume that the file system cannot be
   * dismounted while fdfs_getvattr() is operating and don't worry about it.
   */

	/*
   * FDFS vnodes are labeled VREG to avoid some problems in vn_open() and to
   * prevent our calling specfs.  But SVR4 says they should be character devices,
   * so the deception is carried out here.
   *
   * Unfortunately, in vn_stat() they check the *vnode's* type, not the the type
   * returned by this fdfs_getattr() call, and use it to set the S_IFMT field of
   * mode.  Thus, stat() returns the /dev/fd/XXX files as being regular files
   * instead of character devices.  *sigh*
   *
   * If resetting p_flag in vn_open() for bsd4.3 compatibility is a real problem,
   * this can all be fixed by looking at the *correct* field, namely va_type.  In
   * that case, you can see what the code should be as it is "#if STAT_BROKEN"
   * conditionalized.
   */
	if (vp->v_type == VDIR) {
		vap->va_fileid = 2;
		vap->va_mode = 0555;
		vap->va_type = VDIR;
		vap->va_nlink = 2;
		vap->va_size = FDFS_DIRBLKSZ;
	} else
	 {
		vap->va_fileid = 100 + FILE_DESCRIPTOR(vp);
		vap->va_fsid = makedev(fdfs_major, FILE_DESCRIPTOR(vp));
		vap->va_rdev = makedev(fdfs_major, FILE_DESCRIPTOR(vp));
		vap->va_mode = 0666;
#if STAT_BROKEN == 0
		vap->va_type = VCHR;
#else
		vap->va_type = vp->v_type;
#endif  
		vap->va_nlink = 1;
	}

	vap->va_blocksize = FDFS_DIRBLKSZ;

	/*
   * All times are the current time.  This code was copied in from utimes().  There
   * should probably be some system service that does exactly this.
   */
	s = splhigh();
	TIME_READ_LOCK();
	vap->va_atime = vap->va_mtime = vap->va_ctime = time;
	TIME_READ_UNLOCK();
	splx(s);

	return (0);
}


/*
 * From this point onward, these routines should never be called.  These
 * routines are available only after a file has been opened.  When an
 * FDFS vnode is opened, the real node that it represents is inserted in
 * place of the FDFS vnode.
 */

static int	
fdfs_setattr(vp, vap, cred)
register struct vnode *vp;
register struct vattr *vap;
register struct ucred *cred;
{
	return (ENOSYS);
}


/*
 * Vnode op for reading.
 */
static int	
fdfs_read(vp, uio, ioflag, cred)
struct vnode *vp;
register struct uio *uio;
int	ioflag;
struct ucred *cred;
{
	return (ENOSYS);
}


/*
 * Vnode op for writing.
 */
static int	
fdfs_write(vp, uio, ioflag, cred)
register struct vnode *vp;
struct uio *uio;
int	ioflag;
struct ucred *cred;
{
	return (ENOSYS);
}


static int	
fdfs_ioctl(vp, com, data, fflag, cred)
struct vnode *vp;
int	com;
caddr_t data;
int	fflag;
struct ucred *cred;
{
	return (ENOSYS);
}


/*
 * Mmap a file
 *
 * NB Currently unsupported.
 */
static int	
fdfs_mmap(vp, fflags, cred)
struct vnode *vp;
int	fflags;
struct ucred *cred;
{
	return (ENOSYS);
}


static int	
fdfs_sync(mp, waitfor)
struct mount *mp;
int	waitfor;
{
	return (ENOSYS);
}


/*
 * Synch an open file.
 */
static int	
fdfs_fsync(vp, fflags, cred, waitfor)
struct vnode *vp;
int	fflags;
struct ucred *cred;
int	waitfor;
{
	return (ENOSYS);
}


/*
 * Seek on a file
 */
static int	
fdfs_seek(vp, oldoff, newoff, cred)
struct vnode *vp;
off_t oldoff, newoff;
struct ucred *cred;
{
	return (ENOSYS);
}


/*
 * fdfs remove
 */
static int	
fdfs_remove(ndp)
struct nameidata *ndp;
{
	return (ENOSYS);
}


/*
 * link vnode call
 */
static int	
fdfs_link(vp, ndp)
register struct vnode *vp;
register struct nameidata *ndp;
{
	return (ENOSYS);
}


static int	
fdfs_rename(fndp, tndp)
register struct nameidata *fndp, *tndp;
{
	return (ENOSYS);
}


/*
 * Mkdir system call
 */
static int	
fdfs_mkdir(ndp, vap)
struct nameidata *ndp;
struct vattr *vap;
{
	return (ENOSYS);
}


/*
 * Rmdir system call.
 */
static int	
fdfs_rmdir(ndp)
register struct nameidata *ndp;
{
	return (ENOSYS);
}


/*
 * symlink -- make a symbolic link
 */
static int	
fdfs_symlink(ndp, vap, target)
struct nameidata *ndp;
struct vattr *vap;
char	*target;
{
	return (ENOSYS);
}


/*
 * Vnode op for read and write
 */
static int	
fdfs_readdir(vp, uio, cred, eofflagp)
struct vnode *vp;
register struct uio *uio;
struct ucred *cred;
int	*eofflagp;
{
	int	lost, error;

	if ((uio->uio_offset != 0) || (uio->uio_resid < FDFS_DIRBLKSZ)) {
		return (EINVAL);
	}

	/*
   * Copy the block out.
   */
	lost = uio->uio_resid - FDFS_DIRBLKSZ;
	uio->uio_resid = uio->uio_iov->iov_len = FDFS_DIRBLKSZ;

	error = uiomove((caddr_t)fdfs_dirbuf, FDFS_DIRBLKSZ, uio);

	uio->uio_resid += lost;

	/*
   * Update the uio block.
   */
	uio->uio_resid -= FDFS_DIRBLKSZ;
	*eofflagp = 1;
	return (error);
}


/*
 * Return target name of a symbolic link
 */
static int	
fdfs_readlink(vp, uiop, cred)
struct vnode *vp;
struct uio *uiop;
struct ucred *cred;
{
	return (ENOSYS);
}


/*
 * Fdfs abort op, called after namei() when a CREATE/DELETE isn't actually
 * done. Iff ni_vp/ni_dvp not null and locked, unlock.
 */
static int	
fdfs_abortop(ndp)
register struct nameidata *ndp;
{
	return (ENOSYS);
}


/*
 * Last reference to an inode.
 */
static int	
fdfs_inactive(vp)
struct vnode *vp;
{
	return (0);
}


/*
 * Reclaim a device inode so that it can be used for other purposes.
 */
static int	
fdfs_reclaim(vp)
register struct vnode *vp;
{
	return (0);
}


/*
 * Get access to bmap
 */
static int	
fdfs_bmap(vp, bn, vpp, bnp)
struct vnode *vp;
daddr_t bn;
struct vnode **vpp;
daddr_t *bnp;
{
	return (ENOSYS);
}


/*
 * Just call the device strategy routine
 */
static int	
fdfs_strategy(bp)
register struct buf *bp;
{
	return (ENOSYS);
}


/*
 * Print out the contents of an inode.
 */
static int	
fdfs_print(vp)
struct vnode *vp;
{
	return (ENOSYS);
}


static int	
fdfs_page_read(vp, uio, cred)
struct vnode *vp;
struct uio *uio;
struct ucred *cred;
{
	return (ENOSYS);
}


static int	
fdfs_page_write(vp, uio, cred, pager, offset)
struct vnode *vp;
struct uio *uio;
struct ucred *cred;
memory_object_t pager;
vm_offset_t	offset;
{
	return (ENOSYS);
}


static int	
fdfs_getpage(struct vnode *vp,
register vm_offset_t offset,
vm_size_t len,
vm_prot_t *protp,
register vm_page_t *pl,
register int plsz,
vm_map_entry_t ep,
vm_offset_t addr,
int rwflg,
struct ucred *cred)
{
	return (ENOSYS);
}


static int	
fdfs_putpage(register struct vnode *vp,
register vm_page_t *pl,
register int pcnt,
int flags,
struct ucred *cred)
{
	return (ENOSYS);
}


static int	
fdfs_swap(register struct vnode *vp,
vp_swap_op_t swop,
vm_offset_t args)
{
	return (ENOSYS);
}


static int	
fdfs_bread(register struct vnode *vp,
off_t lbn,
struct buf **bpp,
struct ucred *cred)
{
	return (ENOSYS);
}


static int	
fdfs_brelse(register struct vnode *vp,
register struct buf *bp)
{
	return (ENOSYS);
}


static int	
fdfs_lockctl(vp, eld, flag, cred, clid, offset)
struct vnode *vp;
struct eflock *eld;
int	flag;
struct ucred *cred;
pid_t clid;
off_t offset;
{
	return (ENOSYS);
}


static int	
fdfs_syncdata(vp, fflags, start, length, cred)
struct vnode *vp;
int	fflags;
vm_offset_t start;
vm_size_t length;
struct ucred *cred;
{
	return (ENOSYS);
}


/*
 * This is an exact copy of seltrue() from kernel/bsd/sys_generic.c.
 * It is copied because the link step places seltrue() too far and and
 * 16-bit offset is too small for the distance.
 */
static int	
fdfs_seltrue(dev, events, revents, scanning)
int	scanning;
dev_t dev;
short	*events, *revents;
{
	if (scanning) {
		*revents = *events;
	}
	return (0);
}

