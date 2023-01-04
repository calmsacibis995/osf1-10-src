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

#include <sys/types.h>
#include <sys/user.h>
#include <sys/time.h>
#include <sys/vnode.h>
#include <sys/mount.h>
#include <sys/errno.h>
#include <kern/mfs.h>
#include <kern/lock.h>
#include <sys/namei.h>
#include <sys/fdfs.h>

extern struct vnodeops fdfs_vnodeops;

/*
 * Storage definitions for fdfs static data structures.
 */
struct vnode *fdfs_root_directory;
decl_simple_lock_data(,fdfs_lock)

/*
 * fdfs vfs operations.
 */
static int	fdfs_mount();
static int	fdfs_start();
static int	fdfs_unmount();
static int	fdfs_root();
static int	fdfs_quotactl();
static int	fdfs_statfs();
static int	fdfs_sync();
static int	fdfs_fhtovp();
static int	fdfs_vptofh();
static int	fdfs_init();
static int	fdfs_mountroot();
static int	fdfs_swapvp();

struct vfsops fdfs_vfsops = {
	fdfs_mount,
	fdfs_start,
	fdfs_unmount,
	fdfs_root,
	fdfs_quotactl,
	fdfs_statfs,
	fdfs_sync,
	fdfs_fhtovp,
	fdfs_vptofh,
	fdfs_init,
	fdfs_mountroot,
	fdfs_swapvp
};

struct fdfs_dirent  *fdfs_dirbuf = NULL;
static int	fdfs_already_mounted;

/*
 * This external routine is used only by copen() in vfs_syscalls.c to extract the
 * file descriptor from the FDFS vnode.  By using it, copen() doesn't need to know
 * the exact layout of the FDFS vnode.  Only FDFS vnodes have VDUP set in the v_flags
 * field of the vnode.
 */
int
fdfs_file_descriptor(struct vnode *vnode_ptr)
{
	return (FILE_DESCRIPTOR(vnode_ptr));
}


/*
 * These routines are called by the fdfs_mount() and fdfs_unmount() routines.
 * mount() should prevent other attempts to mount on the current directory, but
 * we have to provide protection to make sure the FDFS file system isn't
 * concurrently being mounted on a different directory.
 */

static int	
fdfs_mark_mounted(void)
{
	simple_lock(&fdfs_lock);
	if (fdfs_already_mounted) {
		simple_unlock(&fdfs_lock);
		return (EBUSY);
	}
	ASSERT(fdfs_already_mounted == 0);
	fdfs_already_mounted++;
	ASSERT(fdfs_already_mounted == 1);
	simple_unlock(&fdfs_lock);
	return (0);
}


static int	
fdfs_mark_unmounted(void)
{
	ASSERT(fdfs_already_mounted == 1);
	fdfs_already_mounted--;
	ASSERT(fdfs_already_mounted == 0);
	return (0);
}


/*
 * This routine allocates a "file system block" to be returned
 * when the dirent calls are made.  It is one file system block long
 * to simplify the code, as that precludes having to figure out *which*
 * block of the directory to send.  To this end, the number of entries
 * normally visible in a directory is a function of the number of entries
 * that can fit in a file system block.  I say "normally visible", since
 * doing a stat on any positive decimal number spontaneously "creates"
 * that entry even though it won't show up on an "ls" (just the entries in
 * this block show up on a simple "ls").
 */

/*
 * This defines the number of entries reported in an "ls".  This number is
 * based upon the file system block size (FDFS_DIRBLKSZ) and the size of an
 * FDFS directory entry record (struct fdfs_dirent).  The value does NOT
 * include "." and ".." which must have space in the directory block.
 */
#define	FDFS_FILE_VNODES	((FDFS_DIRBLKSZ - (2 * sizeof(struct fdfs_dirent)))\
				 / sizeof(struct fdfs_dirent))
/*
 * As a mounted file system, inode 2 is taken for the root.  By convention, inodes
 * 0 and 1 cannot be used.  100 is an arbitrary inode offset value to skip the
 * unavailable offsets, but makes it easy to see what the actual file descriptor
 * number is.
 */
#define	INODE_OFFSET		100
#define	BASE10			10

static int	
allocate_fdfs_dirent_block(struct mount *mp)
{
	struct fdfs_dirent *dptr;
	struct vattr attributes;
	struct vnode *saved_ni_cdir;
	int	error, fd_number;
	struct nameidata *ndp = &u.u_nd;

	saved_ni_cdir = ndp->ni_cdir;
	ndp->ni_cdir = mp->m_vnodecovered;
	ndp->ni_segflg = UIO_SYSSPACE;
	ndp->ni_dirp = "..";
	ndp->ni_nameiop = LOOKUP | FOLLOW;
	error = namei(ndp);
	ndp->ni_cdir = saved_ni_cdir;
	if (error) {
		return (error);
	}

	VOP_GETATTR(ndp->ni_vp, &attributes, u.u_cred, error);
	if (error) {
		return (error);
	}

	/*
   * Get a block of memory, FDFS_DIRBLKSZ long.
   * readdir() always returns a full directory block.
   */
	fdfs_dirbuf = (struct fdfs_dirent * )kalloc(FDFS_DIRBLKSZ);
	bzero((char *)fdfs_dirbuf, FDFS_DIRBLKSZ);	/* make sure unset bytes are zero */
	dptr = fdfs_dirbuf;

	/*
   * Initialize "." first.
   */
	dptr->d_ino = 2;
	dptr->d_reclen = sizeof(struct fdfs_dirent);
	dptr->d_namlen = 1;
	strcpy(dptr->d_name, ".");
	dptr++;

	/*
   * Initialize ".." next.
   */
	dptr->d_ino = attributes.va_fileid;
	dptr->d_reclen = sizeof(struct fdfs_dirent);
	dptr->d_namlen = 2;
	strcpy(dptr->d_name, "..");

	/*
   * Now the rest of the entries.
   */
	for (fd_number = 0; fd_number < FDFS_FILE_VNODES; fd_number++) {
		int	length, fd_tmp;

		++dptr;
		dptr->d_ino = fd_number + INODE_OFFSET;
		for (length = 1, fd_tmp = fd_number; fd_tmp /= BASE10; length++)
			;
		for (dptr->d_namlen = length, fd_tmp = fd_number; length--; fd_tmp /= BASE10) {
			dptr->d_name[length] = '0' + (fd_tmp % BASE10);
		}
		dptr->d_reclen = sizeof(struct fdfs_dirent);
	}

	/*
   * The last record must contain any extra space within the file block.
   */
	dptr->d_reclen += FDFS_DIRBLKSZ - ((FDFS_FILE_VNODES + 2) * sizeof(struct fdfs_dirent));

	return (0);
}



/************************************************************************
 * VFS operations.							*
 ************************************************************************/

/*
 * Synchronization assumptions:
 *	-- Other attempted mounts on this directory are locked out by
 *	   mount().
 * N.B.  Expect ndp to contain a pointer to the vnode to be covered in ni_vp.
 *
 * mount structure
 * Upon entry or after return, the m_next, m_prev, m_op, m_vnodecovered,
 * m_mounth, m_flag, m_exroot, m_uid, m_stat, m_lookup_lock, and m_funnel
 * fields of the mount structure are filled in.  This leaves us to fill in
 * only m_data.
 */
static int	
fdfs_mount(mp, path, data, ndp)
register struct mount *mp;
char	*path;
caddr_t data;
struct nameidata *ndp;
{
	int	error, current_vnode, size;
	struct vnode *fdfs_ptr;
	struct statfs *fs = &(mp->m_stat);

	/*
   * Disable user mounts for FDFS
   */
	if (mp->m_uid) {
		return (EPERM);
	}

#if     SER_COMPAT || RT_PREEMPT
        mp->m_funnel = TRUE;
#endif

	/*
   * Allow FDFS to be mounted only once at a time.
   */
	if (error = fdfs_mark_mounted()) {
		return (error);
	}

	/*
   * Create the root directory of FDFS.
   */

	if (error = getnewvnode(VT_FDFS, &fdfs_vnodeops, &fdfs_root_directory)) {
		fdfs_mark_unmounted();
		return (error);
	}

	fdfs_ptr = fdfs_root_directory;
	fdfs_ptr->v_type = VDIR;
	fdfs_ptr->v_flag |= VROOT;		/* mark as the root of the file system */
	fdfs_ptr->v_object = VM_OBJECT_NULL;
	FILE_DESCRIPTOR(fdfs_ptr) = -1;
	insmntque(fdfs_ptr, mp);

	/*
   * The directory buffer is always the same for this instance of mount().  They
   * differ between mounts only by the inode of "..".
   */
	if (error = allocate_fdfs_dirent_block(mp)) {
		fdfs_unmount(mp, 0);
	}

	/*
   * Prime the statfs structure.  Otherwise, someone will have to call getfsstat()
   * with MNT_NOWAIT before the information will show up.  fdfs_statfs does nothing
   * because none of this information changes.
   */
	bzero((char *)fs, sizeof(*fs));
	fs->f_type = MOUNT_FDFS;
	fs->f_files = FDFS_FILE_VNODES + 2;	/* Don't forget '.' and '..'. */
	(void) copyinstr(path, fs->f_mntonname, sizeof(fs->f_mntonname) - 1, &size);
	strcpy(fs->f_mntfromname, "/dev/fd");

	return (error);
}


static int	
fdfs_start(mp, flags)
struct mount *mp;
int	flags;
{
	return (0);
}


static int	
fdfs_unmount(mp, mntflags)
struct mount *mp;
int	mntflags;
{
	/*
   * Release the root FDFS directory.
   */
	vrele(fdfs_root_directory);
	fdfs_root_directory = (struct vnode *)0;

	/*
   * Mark FDFS unmounted and allow subsequent mounts.
   */
	if (fdfs_dirbuf != NULL) {
		kfree(fdfs_dirbuf, FDFS_DIRBLKSZ);
		fdfs_dirbuf = NULL;
	}
	fdfs_mark_unmounted();
	return (0);
}


static int	
fdfs_root(mp, vpp)
struct mount *mp;
struct vnode **vpp;
{
	VREF(fdfs_root_directory);
	*vpp = fdfs_root_directory;
	return (0);
}


static int	
fdfs_quotactl(mp, cmds, uid, arg)
struct mount *mp;
int	cmds;
uid_t uid;
caddr_t arg;
{
	return (ENOSYS);
}


/*
 * The statfs structure for /dev/fd is primed in fdfs_mount.  It never changes
 * from then on, so when this is called, we simply return success.
 */
static int	
fdfs_statfs(mp)
struct mount *mp;
{
	return (0);
}


static int	
fdfs_sync(mp, waitfor)
struct mount *mp;
int	waitfor;
{
	return (0);
}


static int	
fdfs_fhtovp(mp, fhp, vpp)
register struct mount *mp;
struct fid *fhp;
struct vnode **vpp;
{
	return (ENOSYS);
}


static int	
fdfs_vptofh(vp, fhp)
struct vnode *vp;
struct fid *fhp;
{
	return (ENOSYS);
}


static int	
fdfs_init()
{
	fdfs_already_mounted = 0;
	simple_lock_init(&fdfs_lock);
	simple_unlock(&fdfs_lock); /* start out unlocked */
}


static int	
fdfs_swapvp(mp, vpp, path)
struct mount *mp;
struct vnode **vpp;
char	*path;
{
	return(ENOSYS);
}


static int	
fdfs_mountroot(mp, vpp)
struct mount *mp;
struct vnode **vpp;
{
	return (ENOSYS);
}


