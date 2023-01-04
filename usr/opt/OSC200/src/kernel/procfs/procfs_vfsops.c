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
static char *rcsid = "@(#)$RCSfile: procfs_vfsops.c,v $ $Revision: 1.1.3.6 $ (DEC) $Date: 1993/01/20 14:23:53 $";
#endif

	/*
	 * VFS Operations.
	 * 
	 * This module contains the Virtual File System Operations for
	 * the /proc file system.
	 */

#include <rt_preempt.h>
#include <sys/time.h>
#include <sys/mount.h>
#include <sys/vnode.h>
#include <sys/param.h>
#include <sys/errno.h>
#include <sys/specdev.h>
#include <sys/proc.h>
#include <procfs/procfs.h>
#include <procfs/procfs_l.h>
#include <vm/vm_kern.h>

#if   MACH

#define MOUNT_ALLOCATE(mp)      ZALLOC(mount_zone, mp, struct mount *);

#else

#define MOUNT_ALLOCATE(mp)      MALLOC(mp, (struct mount *),            \
                                       (u_long)sizeof(struct mount),   \
                                       M_MOUNT, M_WAITOK)
#endif

int	procfs_mount();
int	procfs_start();
int	procfs_unmount();
int	procfs_root();
int	procfs_quotactl();
int	procfs_statfs();
int	procfs_sync();
int	procfs_fhtovp();
int	procfs_vptofh();
int	procfs_init();
int	procfs_badop();

struct vfsops procfs_vfsops = {
        procfs_mount,
        procfs_start,
        procfs_unmount,
        procfs_root,
        procfs_quotactl,
        procfs_statfs,
        procfs_sync,
        procfs_fhtovp,
        procfs_vptofh,
        procfs_init,
	procfs_badop,	/* mountroot  added for UBC changes */
	procfs_badop	/* swapvp  added for UBC changes */
};


extern struct vnodeops procfs_vnodeops;

struct vnode *proc_root = NULLVP; /* vnode if /proc is mounted */
struct mount *proc_invlist = (struct mount *)0;	/* mount point with list of invalid vnodes */

extern	pr_s5_dirent_t	*pr_s5_dirbuf;
extern	pr_g_dirent_t	*pr_g_dirbuf;


/*
 *
 * code executed for VFS mountroot and swapvp calls
 *
 */
procfs_badop()
{
	return(EINVAL);
}



/*
 *
 * proc specific mount system call code
 *
 *
 * Inputs parameters:
 *
 *	mp	mount structure: allocated by VFS layer, filled in here
 *	path	directory name (in user space) on which to mount file system
 *	data	pointer to args in user space
 * 	ndp	pointer to nameidata struct, unused
 *
 * Outputs:
 *
 *	mp	the statfs structure in mp gets filled in
 *  proc_root	gets the address of the vnode for the root of the /proc fs
 *
 * Return value:
 *
 *	Returns 0 for success, non-zero for failure.
 *
 */


/*ARGSUSED*/
procfs_mount(mp, path, data, ndp)
	register struct mount *mp;	/* allocated by vfs layer */
	char *path;			/* mounted on directory */
	caddr_t data;			/* mount "device" */
	struct nameidata *ndp;		/* unused */
{

	procfs_args args;
	int error;
	u_int size;

	/*
	 * /proc can only be mounted once; if proc_root contains a valid
	 * vnode address, then /proc is already mounted and we return
	 * error EBUSY
	 */

	if( proc_root != NULLVP )
		return(EBUSY);


	/*
	 * Get the parameters from user space, used later when filling in
	 * statfs struct data.  Do it now, so if there is an error we don't
	 * have to put back the vnode that we are about to allocate.
	 */

	if( error = copyin(data, (caddr_t)&args, sizeof(procfs_args)))
		return(error);

        /*
         * Process export requests.
         */
	MOUNT_LOCK(mp);
        if ((args.exflags & M_EXPORTED) || (mp->m_flag & M_EXPORTED)) {
                if (args.exflags & M_EXPORTED)
                        mp->m_flag |= M_EXPORTED;
                else
                        mp->m_flag &= ~M_EXPORTED;
                if (args.exflags & M_EXRDONLY)
                        mp->m_flag |= M_EXRDONLY;
                else
                        mp->m_flag &= ~M_EXRDONLY;
                mp->m_exroot = args.exroot;
        }
        if ((mp->m_flag & M_UPDATE) == NULL) {
#if     SER_COMPAT || RT_PREEMPT
                mp->m_funnel = FUNNEL_NULL;
#endif
	}


	MOUNT_UNLOCK(mp);

	/*
	 * Allocate a vnode for the root of the /proc file system.  And,
	 * save the /proc root vnode in global location proc_root.  Zero
	 * the procnode part of the vnode.
	 */
	if (error = getnewvnode(VT_PRFS, &procfs_vnodeops, &proc_root))
		return(error);
        bzero( (VNTOPN(proc_root)), sizeof(struct procnode) );
	(VNTOPN(proc_root))->prc_vp = proc_root;

	/*
	 * allocate memory for /proc directory buffers, free root vnode and
	 * return ENOMEM if no memory is available.
	 */
	if( ( ((pr_g_dirbuf = (pr_g_dirent_t *)kmem_alloc(kernel_map,
	      ((nproc+2) * sizeof(struct pr_generic_dir))) )) == NULL) ||
	    ( ((pr_s5_dirbuf = (pr_s5_dirent_t *)kmem_alloc(kernel_map,
	      ((nproc+2) * sizeof(struct pr_sys5_dir))) )) == NULL) )
	{
		vrele(proc_root);
		proc_root = NULLVP;
		return(ENOMEM);
	}



	/*
	 * Set the mount point, and file type.
	 */
	proc_root->v_mount = mp;
	proc_root->v_mountedhere = NULLMOUNT;
	proc_root->v_type = VDIR;
	proc_root->v_flag = VROOT;


	/*
	 * Fill in the statfs struct, which is contained in the mount struct.
	 * This data is static; setting it here speeds up procfs_statfs.
	 */

	mp->m_stat.f_type = MOUNT_PROCFS;
	mp->m_stat.f_flags = mp->m_flag;
	mp->m_stat.f_fsize = ctob(1);
	mp->m_stat.f_bsize = (ctob(1))/2;
	mp->m_stat.f_blocks = 0;
	mp->m_stat.f_bfree = 0;
	mp->m_stat.f_bavail = 0;
	mp->m_stat.f_files = nproc+2;
	mp->m_stat.f_fsid.val[0] = NODEV;
	mp->m_stat.f_fsid.val[1] = MOUNT_PROCFS;

	(void) copyinstr(args.fspec, mp->m_stat.f_mntfromname, MNAMELEN - 1,
		&size);
        bzero(mp->m_stat.f_mntfromname + size, MNAMELEN - size);

	(void) copyinstr(path, mp->m_stat.f_mntonname,
		sizeof(mp->m_stat.f_mntonname) - 1, &size);

	bzero(mp->m_stat.f_mntonname + size,
		sizeof(mp->m_stat.f_mntonname) - size);


	/*
	 * Call procfs_statfs() to fill in mp->m_stat.f_ffree
	 */
	(void)procfs_statfs(mp);
	
	
	return (0);
}


/*
 * start
 *
 * Input parameters:
 *
 *	mp	pointer to a mount struct, unused
 *	flags	flags word, unused
 *
 * This routine must be present and must return success (0).  It is called
 * by mount() in src/kernel/vfs/vfs_syscalls.c.
 *
 */

/*ARGSUSED*/
procfs_start(mp, flags)
	struct mount *mp;
	int flags;
{
	return (0);
}

/*
 * unmount system call
 *
 * Input parameters:
 *
 *	mp		pointer to a mount struct
 *	mntflags	flags word
 *
 * Outputs:
 *
 *	mp		all vnodes on m_mounth list are freed
 *	proc_root	cleared to 0 to indicate /proc not mounted
 *
 * Return value:
 *
 *	Returns 0 for success, non-zero for failure.
 *
 * Synchronization assumptions:
 *	-- other unmounts are locked out.
 *	-- mp is still on global list, and is accessible.
 */
procfs_unmount(mp, mntflags)
	struct mount *mp;
	int mntflags;
{
	struct vnode *mlist, *list_end;
	int error;
	int flags = 0;


	/*
	 * do not allow a forced unmount
	 */
        if (mntflags & MNT_FORCE)
                return(EINVAL);

	/*
	 * We need to ensure that no one has done a cd to /proc.  If the
	 * proc vnode usecount is > 1, then /proc is still busy and/or
	 * someone has done a cd to /proc - so return EBUSY.
	 */
	if( proc_root->v_usecount > 1)
		return(EBUSY);

	/*
	 * If there are any vnodes on the list of invalid
	 * file descriptors, call vflush() to free them.
	 * If there are any vnodes on the list with a reference count
	 * greater than 0, vflush() returns EBUSY.
	 */
	if( proc_invlist->m_mounth != NULLVP )
		if( error = vflush(proc_invlist, NULLVP, flags))
			return(error);

	/*
	 * If there are any vnodes on the mount point's m_mounth list,
	 * we need to check if any of them are busy, and to clear them out.
	 */
	if( mp->m_mounth != NULLVP )
	{
		/* Call vflush to free any vnodes that may be on the
		 * m_mounth list from anyone having done an ls on /proc.
		 * If there are any vnodes on the list with a reference count
		 * greater than 0, vflush() returns EBUSY.
		 */
		if( error = vflush(mp, NULLVP, flags))
			return(error);
	}

	/*
	 * Call vrele to free the root vnode, then clear the global proc_root
	 * to indicate that /proc is no longer mounted.
	 */
        vrele(proc_root);
	proc_root = NULLVP;


	/*
	 * free memory for /proc directory buffers
	 */
	(void)kmem_free(kernel_map, pr_g_dirbuf,
	      ((nproc+2) * sizeof(struct pr_generic_dir)));
	(void)kmem_free(kernel_map, pr_s5_dirbuf,
	      ((nproc+2) * sizeof(struct pr_sys5_dir)));

	return(0);
}


/*
 * Return root of a filesystem
 *
 * Input parameters:
 *
 *	mp		pointer to a mount struct, unused
 *	vpp		vnode pointer to fill in
 *
 * Outputs:
 *
 *	vpp		gets the vnode address in proc_root
 *
 * Return value:
 *
 *	Returns 0 for success
 *
 *
 * Synchronization assumptions:
 *	-- it's safe, and file system isn't going anywhere.
 */
procfs_root(mp, vpp)
	struct mount *mp;
	struct vnode **vpp;
{

	/* increment the vnode ref count because namei() does a vrele() on it */
	VREF(proc_root);

	/* get vnode pointer from proc_root */
	*vpp = proc_root;
	return (0);
}



/*
 * Do operations associated with quotas
 *
 * This is invoked from quotactl() in src/kernel/vfs/vfs_syscalls.c.
 * It does not apply to /proc, so just return success (0) i.e. there
 * is nothing that we need to do.
 */
/*ARGSUSED*/
procfs_quotactl(mp, cmds, uid, arg)
	struct mount *mp;
	int cmds;
	uid_t uid;
	caddr_t arg;
{
	return(0);
}

/*
 * Get file system statistics.
 *
 * Input parameters:
 *
 *	mp	pointer to a mount struct
 *
 * Outputs:
 *
 *	mp	mp->m_stat.f_ffree gets the # of free proc table entries
 *
 * Return value:
 *
 *	Returns 0 for success
 *
 *
 * Most of the statfs information is static, and is filled in at mount time.
 * All this does is calculate the number of free proc table slots, and
 * returns it as the number of free "files" in the /proc file
 * system.  While it would be faster to scan the allproc and zombproc lists,
 * that would require locking those lists.  Additionally, scanning the proc
 * table takes longer and may not be completely accurate; but this will not
 * slow down other cpus in a multi-cpu system.  The number of free slots in
 * the system proc table is/(or can be) constantly changing, therefore just
 * give a reasonable snapshot of what that value is.
 *
 */
procfs_statfs(mp)
	struct mount *mp;
{
	register int i;
	register int free;
	free = 0;


	/*
	 * Ignore pid 0, the swapper process, and count any proc table entry
	 * with no parent proc structure as being free. This is one of the
	 * fields in the proc struct cleared by rexit().
	 */
	for( i= 1; i < nproc; i++)
	{
		if(proc[i].p_pptr == (struct proc *)0)
			free++;
	}

	/*
	 * Lock the mount point, and copy the number of free proc table entries
	 * to the statfs structure.
	 */
	MOUNT_LOCK(mp);
	mp->m_stat.f_ffree = free;
	MOUNT_UNLOCK(mp);

	return (0);
}

/*
 * procfs specific sync call
 *
 * This is invoked from sync() and dounmount() in src/kernel/vfs/vfs_syscalls.c.
 * It does not apply to /proc, so just return success (0) i.e. there
 * is nothing that we need to do.
 *
 */
/*ARGSUSED*/
procfs_sync(mp, waitfor)
	struct mount *mp;
	int waitfor;
{

	return(0);
}


/*
 * File handle to vnode
 *
 * Input parameters:
 *
 *	mp	pointer to a mount struct, unused
 *	fhp	pointer to a file handle structure
 *	vpp	pointer to a pointer to a vnode, used as return value
 *
 * Outputs:
 *
 *	vpp	gets the vnode address in the file handle
 *
 * Return value:
 *
 *	Returns 0 for success
 *
 */
/*ARGSUSED*/
procfs_fhtovp(mp, fhp, vpp)
	register struct mount *mp;
	struct fid *fhp;
	struct vnode **vpp;
{

	/* copy vnode address stored in fh to vpp */
	*vpp = PROCFS_FHTOVP(fhp);
	return (0);
}

/*
 * Vnode pointer to File handle
 * Input parameters:
 *
 *	mp	pointer to a mount struct
 *
 * Outputs:
 *
 *	mp	mp->m_stat.f_ffree gets the # of free proc table entries
 *
 * Return value:
 *
 *	Returns 0 for success
 *
 * Synchronization assumptions:
 *	-- i_gen and i_number are read-only
 */
procfs_vptofh(vp, fhp)
	struct vnode *vp;
	struct fid *fhp;
{
	register struct pfid *pfh;

	/*
	 * cast the generic fhp as a pfid (procfs file handle) pointer
	 * fill in the length as the actual length of the structure,
	 * copy the vnode pointer, vp, to the struct, and
	 * set the generation number to 0
	 */
	pfh = (struct pfid *)fhp;
	pfh->pfid_len = sizeof( struct pfid);
	pfh->pfid_vnode = vp;
	pfh->pfid_gen = 0L;

	return (0);
}


/*
 * Init the proc filesystem.
 *
 * Inputs:
 *	none
 *
 * Outputs:
 *	proc_invlist:	gets the address of a new mount structure
 *
 * Return value:
 *			always returns 0 for success
 *
 * This sets up a mount point with nothing but the correct vnode op address,
 * and a list of invalid vnodes.  This is done to invalidate the file
 * descriptors of any process that does an exec of a setuid or setgid
 * program while it is open via /proc.
 */
procfs_init()
{
	/*
	 * Allocate a new mount structure, and initialize it.
	 */
	MOUNT_ALLOCATE(proc_invlist);
        proc_invlist->m_op = vfssw[MOUNT_PROCFS];
        proc_invlist->m_flag = 0;
        proc_invlist->m_exroot = (uid_t)0;
        proc_invlist->m_uid = (uid_t)0;
        proc_invlist->m_mounth = NULLVP;
        proc_invlist->m_next = (struct mount *)0;

	return (0);
}
