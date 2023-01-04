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
static char *rcsid = "@(#)$RCSfile: procfs_vnops.c,v $ $Revision: 1.1.26.18 $ (DEC) $Date: 1994/01/27 14:24:10 $";
#endif

	/*
	 * VNODE Operations.
	 * 
	 * This module contains the Vnode Operations for the /proc
	 * file system.
	 */

#include <kern/lock.h>
#include <vm/vm_map.h>
#include <vm/vm_kern.h>
#include <vm/u_mape_seg.h>
#include <sys/time.h>
#include <sys/mount.h>
#include <sys/vnode.h>
#include <sys/specdev.h>
#include <sys/proc.h>
#include <sys/fcntl.h>
#include <sys/mode.h>
#include <sys/uio.h>
#include <sys/param.h>
#include <sys/map.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <s5fs/s5param.h>
#include <s5fs/s5dir.h>
#include <kern/queue.h>
#include <kern/parallel.h>
#include <kern/thread_swap.h>
#include <mach/vm_param.h>
#include <procfs/procfs.h>
#include <procfs/procfs_l.h>
#include <sys/ptrace.h>
#include <sys/file.h>
#include <sys/siginfo.h>

#ifdef __alpha

#include <machine/trap.h>
#include <mach/alpha/thread_status.h>

#define PROCFS_THREAD_STATE_COUNT	ALPHA_THREAD_STATE_COUNT
#define PROCFS_THREAD_STATE		ALPHA_THREAD_STATE
#define PROCFS_FLOAT_STATE_COUNT	ALPHA_FLOAT_STATE_COUNT
#define PROCFS_FLOAT_STATE		ALPHA_FLOAT_STATE
#define PROCFS_thread_state		alpha_thread_state
#define PR_PC_OFF (sizeof(int))
#define	PROCFS_AST			T_AST

#endif /* __alpha */

#ifdef mips

#include <mach/mips/thread_status.h>

#define PROCFS_THREAD_STATE_COUNT	MIPS_THREAD_STATE_COUNT
#define PROCFS_THREAD_STATE		MIPS_THREAD_STATE
#define PROCFS_FLOAT_STATE_COUNT	MIPS_FLOAT_STATE_COUNT
#define PROCFS_FLOAT_STATE		MIPS_FLOAT_STATE
#define PROCFS_thread_state		mips_thread_state
#define PR_PC_OFF 0
#define	PROCFS_AST			NULL


#endif
#ifdef i386

#include <mach/i386/thread_status.h>

#define PROCFS_THREAD_STATE_COUNT	i386_THREAD_STATE_COUNT
#define PROCFS_THREAD_STATE		i386_THREAD_STATE
#define PROCFS_FLOAT_STATE_COUNT	i386_FLOAT_STATE_COUNT
#define PROCFS_FLOAT_STATE		i386_FLOAT_STATE
#define PROCFS_thread_state		i386_thread_state
#define PR_PC_OFF 0
#define	PROCFS_AST			NULL


#endif
#ifdef multimax

#include <mach/mmax/thread_status.h>

#define PROCFS_THREAD_STATE_COUNT	NS32000_THREAD_STATE_COUNT
#define PROCFS_THREAD_STATE		NS32000_THREAD_STATE
#define PROCFS_FLOAT_STATE_COUNT	NS32000_THREAD_STATE_COUNT
#define PROCFS_FLOAT_STATE		NS32000_THREAD_STATE /* only 1 set
								of registers */
#define PROCFS_thread_state		ns32000_thread_state

#define PR_PC_OFF 0

#endif


#define PR_KERN_MAP	kernel_copy_map

int	procfs_lookup();
int	procfs_open();
int	procfs_close();
int	procfs_access();
int	procfs_getattr();
int	procfs_read();
int	procfs_write();
int	procfs_ioctl();
int	procfs_select();
int	procfs_einval();
int	procfs_seek();
int	procfs_readdir();
int	procfs_abortop();
int	procfs_inactive();
int	procfs_reclaim();
int	procfs_print();
int	procfs_page_read();
int	procfs_page_write();
int	procfs_unsupp();
int	procfs_einval();
int	procfs_bread();
int	procfs_brelse();

struct vnodeops procfs_vnodeops = {
        procfs_lookup,		/* lookup */
        procfs_unsupp,		/* create */
        procfs_unsupp,		/* mknod */
        procfs_open,		/* open */
        procfs_close,		/* close */
        procfs_access,		/* access */
        procfs_getattr,		/* getattr */
        procfs_unsupp,		/* setattr */
        procfs_read,		/* read */
        procfs_write,		/* write */
        procfs_ioctl,		/* ioctl */
        procfs_select,		/* select */
        procfs_einval,		/* mmap */
        procfs_unsupp,		/* fsync */
        procfs_seek,		/* seek */
        procfs_unsupp,		/* remove */
        procfs_unsupp,		/* link */
        procfs_unsupp,		/* rename */
        procfs_unsupp,		/* mkdir */
        procfs_unsupp,		/* rmdir */
        procfs_unsupp,		/* symlink */
        procfs_readdir,		/* readdir */
        procfs_unsupp,		/* readlink */
        procfs_abortop,		/* abortop */
        procfs_inactive,	/* inactive */
        procfs_reclaim,		/* reclaim */
        procfs_unsupp,		/* bmap */
        procfs_unsupp,		/* strategy */
        procfs_print,		/* print */
        procfs_page_read,	/* page_read */
        procfs_page_write,	/* page_write */
	procfs_unsupp,		/* get page added for UBC */
	procfs_unsupp,		/* put page added for UBC */
	procfs_unsupp,		/* swap added for UBC */
	procfs_bread,		/* buffer read */
	procfs_brelse,		/* buffer release */
	procfs_einval,		/* file locking */
	procfs_unsupp,		/* fsync byte range*/
};

extern struct timeval time;
extern struct vnode *proc_root;
extern struct mount *proc_invlist;

extern int strlen();
extern int strcmp();

pr_s5_dirent_t	*pr_s5_dirbuf;
pr_g_dirent_t	*pr_g_dirbuf;

struct timeval pr_s5_timestamp;
struct timeval pr_g_timestamp;


/*
 *	common routine for unsupported vnode ops 
 */
procfs_unsupp()
{
	return(EACCES);
}


/*
 *	common routine for invalid vnode ops 
 */
procfs_einval()
{
	return(EINVAL);
}


/*
 * lookup for procfs
 *
 * Input parameters:
 *
 *	vp		vnode of directory to search (this is always proc_root)
 *   ndp->ni_pnbuf	filename to search for
 *
 * Outputs:
 *
 *   ndp->ni_vp		gets vnode for ascii file name in /proc
 *
 * Return value:
 *			returns 0 for success, non-zero for failure.
 *
 * Side affects:
 *			If there is no vnode on the mp m_mounth list,
 *			one is allocated and it is placed on the list.
 *			If more than nproc vnodes are on the mp m_mounth
 *			list, unused vnodes on the list are freed.f
 */

procfs_lookup(vp, ndp)
	struct vnode *vp;		/* root vnode for /proc */
	struct nameidata *ndp;		/* input and output data */
{
	char *name, *name1;
	struct procnode *pn;
	struct mount *mp;
	struct vnode *start_vp, *free_vp, *wvp;
	int i, vnused, file, error;
	static int vnused_cnt = NULL;

	/*
	 * Set the return vnode, ndp->ni_vp, to NULLVP in case there is an
	 * error.  Otherwise, the kernel will panic.
	 */
	ndp->ni_vp = NULLVP;
	ndp->ni_dvp = vp;

	/*
	 * If the filename is . then return the vnode address we were
	 * called with - after incrementing its usecount.
	 */
	if(strcmp(".", ndp->ni_ptr) == NULL) {
		VREF(vp);
		ndp->ni_vp = vp;
		return(0);
	}

	/*
	 * get the address of the mount struct from the vnode, v_mount
	 */
	mp = vp->v_mount;

	/*
	 * Convert the ascii "file name" in ndp->ni_pnbuf to a binary number.
	 * This is the file name that we want to get a vnode for.
	 * Note: atoi() in dec/machine/mips/machdep.c can return a negative
	 * number and will truncate the number at the first non 0-9 character.
	 * Do some sanity checking before calling atoi().
	 */
	if(strncmp("./", ndp->ni_ptr, 2) == NULL)
		name1 = name = ndp->ni_ptr+2;
	else
		name1 = name = ndp->ni_ptr;

	while( *name != '\0' )
		if( *name > '9' || *name++ < '0')
			return(ENOENT);

	file = atoi(name1);



	/*
	 * Lock the mount struct, and search the m_mounth list for a procnode
	 * with a prc_pid that matches the file name we are looking for. Unlock
	 * the mount struct if a match is found, or at the end of the list.
	 * If a match is found, check that the proc table index in the
	 * procnode, prc_index, is * correct.  If the saved index is wrong,
	 * scan the proc table for the desired p_pid.  If it is found, save
	 * the correct proc table index in the procnode. If it was not found,
	 * return error ENOENT.  Then,  save the vnode address to a working
	 * vnode pointer, and increment its use count.
	 */
	vnused = NULL;
	wvp = NULLVP;
	if( mp->m_mounth != NULLVP)
	{
		MOUNT_LOCK(mp);
		start_vp = mp->m_mounth;
		do
		{
			VN_LOCK(start_vp);
			pn = VNTOPN(start_vp);
			if( pn->prc_pid == file)
			{
			    if( pn->prc_pid != proc[pn->prc_index].p_pid)
			    {
				for(i=0; i< nproc; i++)
					if(proc[i].p_pid == file)
					{
						pn->prc_index = i;
						break;
					}
				if( i == nproc)
				{
					VN_UNLOCK(start_vp);
					MOUNT_UNLOCK(mp);
					return(ENOENT);
				}
				if(proc[i].utask != NULL &&
				 proc[i].p_stat != SZOMB &&
				 proc[i].p_stat != SIDL)
					pn->prc_time = proc[i].utask->uu_start;
			    }
			    wvp = start_vp;
			    VREF(wvp);
			    VN_UNLOCK(start_vp);
			    break;
			}
			VN_UNLOCK(start_vp);
			vnused++;
			start_vp = start_vp->v_mountf;
		} while( start_vp != NULLVP);
		MOUNT_UNLOCK(mp);
	}

	/* If the number of vnodes on the m_mounth list is greater than nproc,
	 * we have to clean up the list. Lock the mount struct, and scan the
	 * m_mounth list.  For each entry, if the proc table index and pid
	 * do not match, and if the prc_time field is 0, call vrele() to free
	 * the vnode.  Checking prc_time guarantees that we do not vrele a
	 * vnode for a process that has terminated, but is still open.
	 * Unlock the mount struct when at the end of the list.
	 */
	if( vnused > (nproc/2))
	{
		vnused_cnt++;
		MOUNT_LOCK(mp);
		start_vp  = mp->m_mounth;
		do
		{
			pn = VNTOPN(start_vp);
			if( (pn->prc_pid != proc[pn->prc_index].p_pid) &&
			    (pn->prc_time.tv_sec == NULL ) )
			{
				free_vp = start_vp;
				start_vp = start_vp->v_mountf;
				vrele(free_vp);
			}
			else
				start_vp = start_vp->v_mountf;
		} while( start_vp != NULLVP);
		MOUNT_UNLOCK(mp);
	}


	/* If a vnode is not found on the list, call getnewvnode to allocate
	 * one.  If getnewvnode fails, return its error status.  If
	 * getnewvnode succeeds, save the new vnode address to a working
	 * vnode pointer. The procnode must be initialized (zero everything
	 * including prc_excl and prc_time); copy the binary pid to prc_pid.
	 * Copy the vnode address to procnode prc_vp, and call insmntque
	 * to put the new vnode on the mount list mp->m_mounth.
	 * (The next code block below fills in prc_index.)
	 */
	if( wvp == NULLVP )
	{
		for(i=0; i< nproc; i++)
			if(proc[i].p_pid == file)
				break;

		if( i == nproc)
			return(ENOENT);

		if(error = getnewvnode(VT_PRFS, &procfs_vnodeops, &wvp))
			return(ENOENT);
		pn = VNTOPN(wvp);
		bzero( pn, sizeof(struct procnode) );
		pn->prc_index = i;
		pn->prc_pid = file;
		pn->prc_vp = wvp;
		if(proc[i].utask != NULL && proc[i].p_stat != SZOMB &&
		 proc[i].p_stat != SIDL)
			pn->prc_time = proc[i].utask->uu_start;
		wvp->v_type = VREG;
		wvp->v_mountedhere = NULLMOUNT;
	        insmntque(wvp, mp);
	}

	/*
	 * Copy the return vnode address to the namei data structure.
	 */
	ndp->ni_vp = wvp;
	return(0);
}


/*
 * Open called.
 *
 * Input parameters:
 *
 *	vpp		vnode
 *	mode		open mode requested
 *	cred		credentials for the tracing process
 *
 * Outputs:
 *
 *  procnode.prc_excl	gets set to the pid of the opening process, if this
 *			is the first exclusive open for write (FEXCL).
 *
 *  procnode.prc_time	is set to the current system time - on first open
 *  procnode.prc_vnptr	is set to the address of the vnode that represents
 *			the file being opened - on the first open
 *
 * Return value:
 *			returns 0 for success, non-zero for failure.
 *	An open for read always succeeds.  An exclusive open for write
 *	succeeds only if the file is not already open for write (i.e. an
 *	exclusive open for write must be the first open for write - even
 *	for root).  Multiple opens for write succeed as long as the file has
 *	not been opened exclusively for write.  An open for write succeeds
 *	for root even if an exclusive open for write had already been done.
 *	(Note: an exclusive open for write by root can fail; a non-exclusive
 *	open for write by root always succeeds.)
 */

procfs_open(vpp, mode, cred)
	struct vnode **vpp;
	int mode;
	struct ucred *cred;
{
	struct procnode *pn;

	/*
	 * If we are opening the /proc directory itself there is nothing
	 * to do, just return success.
	 */
	if( *vpp == proc_root )
		return(0);

	/*
	 * get a pointer to the procnode from the vnode
	 */
	pn = VNTOPN(*vpp);

	/*
	 * If the process is still initializing, return EBUSY
	 */
	if(proc[pn->prc_index].p_stat == SIDL)
		return(EBUSY);

	/*
	 * If the process has terminated, return ENOENT
	 */
	if((proc[pn->prc_index].p_stat == NULL) ||
	   (proc[pn->prc_index].p_stat == SZOMB) )
		return(ENOENT);

	/*
	 * If this is the first open of this file, the vnode address must be
	 * stored in the proc table in the p_vnptr field; this is used by
	 * rexit() to store the exit status should the "traced" process
	 * terminate while it is opened via /proc.  This allows a debugger to
	 * report the exit status. Force execution on the master cpu before
	 * writing the vnode address to the proc table entry.
	 */
	if((proc[pn->prc_index].task->procfs.pr_qflags & PRFS_OPEN) == NULL)
	{
		unix_master();
		proc[pn->prc_index].task->procfs.pr_qflags |= PRFS_OPEN;
		proc[pn->prc_index].task->procfs.pr_flags &= ~PR_RLC;
		proc[pn->prc_index].p_vnptr = *vpp;
		unix_release();
	}

	/*
	 * If the open mode does not have FWRITE set (i.e. it is not being
	 * opened for write), there is nothing to do, return success (i.e. 0).
	 */
	if( !(mode & FWRITE) )
		return(0);

	/*
	 * If the write count in the vnode, v_wrcnt, is 0 we always succeed.
	 * Additionally, if the FEXCL mode bit is set, save the current PID
	 * in the procnode prc_excl. NOTE: the vnode read and write counts
	 * are incremented (as appropriate) by vn_open() and decremented
	 * by vn_close(); both routines are in src/kernel/vfs/vfs_vnops.c.
	 */
	VN_LOCK(vp);
	proc[pn->prc_index].task->procfs.pr_qflags |= PRFS_TRACING;
	proc[pn->prc_index].p_pr_qflags |= PRFS_TRACING;
	if((*vpp)->v_wrcnt == NULL)
	{
		if(mode & FEXCL)
			pn->prc_excl = proc[current_task()->proc_index].p_pid;
		VN_UNLOCK(vp);
		return(0);
	}

	/*
	 * If we get here, the write count is 1 or greater, so check for
	 * and exclusive open.
	 * If the FEXCL mode bit is set, return error EBUSY.
	 * (That is, the file is already opened for write, and someone wants
	 * an exclusive open for write - which is not granted.)
	 */
	if(mode & FEXCL)
	{
		VN_UNLOCK(vp);
		return(EBUSY);
	}

	/*
	 * At this point, 1 or more processes have the file open for write,
	 * but this is not a request for an exclusive open.
	 *
	 * If the file is not already opened exclusively (prc_excl == NULL)
	 * grant access - return success.
	 * If the file is already opened exclusively (prc_excl is not 0) and
	 * we are root, or have SEC privilege, return success, otherwise
	 * return error EBUSY.
	 */
	if( pn->prc_excl == NULL)
	{
		VN_UNLOCK(vp);
		return(0);
	}

	VN_UNLOCK(vp);
#if     SEC_BASE
        if (privileged(SEC_DEBUG, NULL))
#else
        if (cred->cr_uid == NULL)
#endif
                return (0);
	else
		return(EBUSY);

}

/*
 * Close called
 *
 *
 * Input parameters:
 *
 *	vp		vnode
 *	fflag		flag word, unused
 *	cred		credentials for the tracing process, unused
 *
 * Outputs:
 *
 *  procnode.prc_excl	cleared if the calling process is the pid that
 *			had the exclusive open
 *
 * Return value:
 *
 *			returns 0 for success
 *
 *	Note: the vfs layer, vn_close in src/kernel/vfs/vfs_vnops.c,
 *	handles the vnode counters and calls vrele() to free the vnode.
 */
procfs_close(vp, fflag, cred)
	struct vnode *vp;
	int fflag;
	struct ucred *cred;
{
	struct procnode *pn;

	/*
	 * If this is the /proc directory file, just return success.
	 */
	if( vp == proc_root)
		return(0);

	/*
	 * If the procnode prc_excl pid matches ours, we must have had an
	 * exclusive open for write, so set prc_excl, in the procnode, to 0
	 * to indicate that the exclusive write is gone.
	 */
	VN_LOCK(vp);
	pn = VNTOPN(vp);
	if(proc[pn->prc_index].p_stat == SZOMB)
		pn->prc_excl = (pid_t)0;
	else if( pn->prc_excl == proc[current_task()->proc_index].p_pid)
		pn->prc_excl = (pid_t)0;

	VN_UNLOCK(vp);
	return(0);
}

/*
 *	Check Access Permissions:
 *
 * Input parameters:
 *
 *	vp		vnode
 *	mode		open mode requested, unused
 *	cred		credentials for the tracing process
 *
 * Outputs:
 *			none
 *
 * Return value:
 *
 *			returns 0 for success, non-zero for failure.
 */
/*ARGSUSED*/
procfs_access(vp, mode, cred)
	struct vnode *vp;
	int mode;
	struct ucred *cred;
{
	struct proc *p;
	int i, rmode;
	struct procnode *pn;

	/*
	 * If the /proc directory itself is being opened, grant access
	 * unless FWRITE is being requested.
	 */
	if( vp == proc_root)
	{
		if( (mode & FWRITE) )
			return(EACCES);
		else
			return(0);
	}

	/*
	 * Use the PROCFS_VNINVALID macro to verify that the vnode still
	 * represents the traced process.  If the macro evaluates to true,
	 * return ENOENT.  If the vnode is valid, then get a pointer to the
	 * proc table entry for the traced process.
	 */
	pn = VNTOPN(vp);
	if(PROCFS_VNINVALID(pn) )
		return(ENOENT);


	/*
	 * If SEC_BASE and privileged or root, return 0 - success
	 */
#if	SEC_BASE
	if (privileged(SEC_DEBUG, NULL))
#else
	if (cred->cr_uid == NULL)
#endif
		return (0);

	/*
	 * Get a pointer to the proc table entry that represents
	 * the /proc file we are working with.
	 */
	p = &proc[(pn->prc_index)];

	/* If the process state is SIDL, return EBUSY */
	if( p->p_stat == SIDL)
		return(EBUSY);
	/*
	 * The file permissions of the executable file that represents
	 * the /proc file being opened must allow read access by the tracing
	 * process.  If not, return EACCES.  If yes, do the UID/GID check
	 * for the /proc file below.  Note: check for read access,
	 * irrespective of the access requested by the mode input.
	 */
	rmode = VREAD;
	if(cred->cr_uid != p->task->procfs.pr_uid)
	{
		rmode >>= 3;	/* set to check group permissions */
		if(cred->cr_gid != p->task->procfs.pr_gid)
		{
			/* check if one of the GIDs of the executable file
			 * match the gid of the tracing process
			 */
			for(i=0; i < cred->cr_ngroups; i++)
				if(cred->cr_groups[i] == p->task->procfs.pr_gid)
					break;
			/*if no group match, set to check "other" permissions*/
			if( i == cred->cr_ngroups)
				rmode >>= 3;
		}
	}
	/* when we get here, rmode is set to the position of Owner,
	 * Group, or Other as determined by the above checks.  As
	 * in SysV and the ufs code on OSF/1, if the UID matches,
	 * owner permissions are checked, if the GID matches group
	 * permissions are checked, otherwise Other permissions are
	 * checked.
	 */
	if((rmode & p->task->procfs.pr_protect) == NULL)
		return(EACCES);

	/*
	 * Only root is allowed to open a /proc file if its corresponding
	 * executable has setuid or setgid set - if the uid and gid do not
	 * match.  Therefore if either of these checks fail return EACCES,
	 * otherwise proceed to check the uid and gid.
	 */
	if( ((p->task->procfs.pr_protect & S_ISUID) &&
	   p->task->procfs.pr_uid != cred->cr_uid)  ||
	   ((p->task->procfs.pr_protect & S_ISGID) &&
	   p->task->procfs.pr_gid != cred->cr_gid) )
		return(EACCES);

	/*
	 * The user ID and the group ID of the tracing process must match
	 * the user and group IDs of the /proc file.  If not, return EACCES.
	 * The credentials for the /proc file are in the proc table entry
	 * for the corresponding process.
	 */
	if( (cred->cr_uid == p->p_rcred->cr_uid)
	 && (cred->cr_gid == p->p_rcred->cr_gid) )
		return(0);
	else
		return(EACCES);
}

/*
 *	Get file attributes:
 *
 * Input parameters:
 *
 *	vp		vnode
 *	vap		vattributes structure
 *	cred		credentials for the tracing process
 *
 * Outputs:
 *
 *	vap		the vattr struct is filled in
 *
 * Return value:
 *
 *			returns 0 for success, non-zero for failure.
 */

procfs_getattr(vp, vap, cred)
	struct vnode *vp;
	register struct vattr *vap;
	struct ucred *cred;
{
	int i;
	struct proc *p;
	struct procnode *pn;

	pn = VNTOPN(vp);

	/*
	 * If the input vp is the same as the proc_root vnode, then
	 * fill in info for the /proc directory file.
	 */
	if( vp == proc_root)
	{
		vap->va_type = MOUNT_PROCFS;		/* type is /proc */
		vap->va_mode =
		S_IFDIR|S_IRUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH;
		vap->va_nlink = 2;			/* per SVR4 binary */
		vap->va_uid = (uid_t)0;			/* user id root */
		vap->va_gid = (gid_t)0;			/* group id sys */
		vap->va_fileid = 2;			/* fake vnode number */
#if __alpha
		vap->va_qsize = (nproc+2)*(sizeof(struct s5direct));
#else
		vap->va_qsize.val[0] = (nproc+2)*(sizeof(struct s5direct));
		vap->va_qsize.val[1] = 0;
#endif
	}
	else	/* If we get here, the vnode points to a file in /proc */
	{
		/*
		 * If the file descriptor is invalid, return EAGAIN.  This can
		 * occur if the traced process does an exec of a process that
		 * it does not have read permission for or that has setuid or
		 * setgid set.
		 */
		if(pn->prc_flags & PRC_FDINVLD)
			return(EAGAIN);

		/*
		 * Use the PROCFS_VNINVALID macro to verify that the vnode still
		 * represents the traced process.  If the macro evaluates to
		 * true, return ENOENT.  If the vnode is valid, then get a
		 * pointer to the proc table entry for the traced process.
		 */
		if(PROCFS_VNINVALID(pn) )
			return(ENOENT);

		/*
		 * From the input vnode pointer, get a pointer to the proc
		 * struct for this process - call the pointer p.
		 * Fill in the vattr struct as follows:
		 */
		p = &proc[pn->prc_index];

		vap->va_type = VREG;			/* type regular file*/
		vap->va_mode = S_IFREG|S_IRUSR|S_IWUSR;	/* reg file 0600 perms*/
		vap->va_nlink = 1;			/* 1 file for each pid*/
		vap->va_uid = p->p_ruid;		/* real user id*/
		vap->va_gid = p->p_rgid;		/* real group id*/
		vap->va_fileid = (p->p_pid + 64);	/* fake vnode number */
#if __alpha
		vap->va_qsize =p->task->map->vm_size; /*process size bytes*/
#else
		vap->va_qsize.val[0] =p->task->map->vm_size; /*process size bytes*/
		vap->va_qsize.val[1] = 0;
#endif
	}

	/*
	 * Fields common to /proc dir and files in it
	 */
	vap->va_rdev = (dev_t)0;		/* no special device here */
	vap->va_fsid = (dev_t)0;		/* no special device here */
	vap->va_blocksize = ctob(1);		/* 1 page at a time */
	vap->va_atime = time;			/* set to current system time*/
	vap->va_mtime = time;			/*           "		*/
	vap->va_ctime = time;			/*	     "		*/
	vap->va_flags = NULL;			/* just null */
	vap->va_gen = NULL;			/* never changes */
#if __alpha
	vap->va_qbytes = vap->va_qsize;
#else
	vap->va_qbytes.val[0] = vap->va_qsize.val[0];
	vap->va_qbytes.val[1] = vap->va_qsize.val[1];
#endif
	return (0);
}


/*
 * Vnode op for reading.
 *
 * Input parameters:
 *
 *	vp		vnode
 *	uio		uio structure pointer
 *	ioflag		unused
 *	cred		credentials for the tracing process, unused
 *
 * Outputs:
 *
 *
 *
 * Return value:
 *
 *			returns 0 for success, non-zero for failure.
 */
/* ARGSUSED */
procfs_read(vp, uio, ioflag, cred)
	struct vnode *vp;
	register struct uio *uio;
	int ioflag;
	struct ucred *cred;
{
	register int self;
	int error, nb, s5siz, rd_siz, lost;
	struct procnode *pn;
	struct proc *pr;
	vm_map_t map;
	vm_map_copy_t vmcpy;
	vm_offset_t kern_adr;

	self = 0L;

	/*
	 * Do preliminary sanity checking.
	 */
	if(uio->uio_rw != UIO_READ)
		panic("procfs_read mode");
	if(uio->uio_resid == NULL)
		return(0);

	/*
	 * If vp == the proc_root vnode, the user wants to read the /proc
	 * directory file.
	 */
	if( vp == proc_root)
	{
		/*
		 * Use macro to ensure that data in pr_s5_dirbuf is current.
		 */
		(void)procfs_get_s5_dir();

		/*
		 * If the file offset to read at, uio->uio_offset is greater
		 * than or equal to the size of the s5dir buffer return 0,
		 * (End_OF_File).
		 */
		s5siz = ((nproc+2)*sizeof(struct s5direct));
		if( uio->uio_offset >= s5siz )
			return(0);

		/* Set the number of bytes to copy, nb, to the user buffer to
		 * the minimum of the requested length, uio->uio_resid, or the
		 * amount of data in the s5dir_buffer from the desired offset
		 * to the end of the s5dir_buffer.
		 */
		nb = MIN((s5siz - uio->uio_offset), uio->uio_resid);

		/* call uiomove to copy nb data bytes from the buffer starting
		 * at the desired uio->uio_offset, to the user buffer
		 * return with the status from uiomove
		 */
		error = uiomove(((char *)pr_s5_dirbuf+uio->uio_offset),nb,uio);
		return(error);
	}

	/* We only get here if the user is reading one of the "files"
	 * in the /proc directory.
	 */

#ifdef mips
	/*
	 * The transfer must be a multiple of longwords and begin on a
	 * longword boundary.
	 */
	if( (uio->uio_offset & (sizeof(int)-1) != NULL) ||
	    (uio->uio_resid & (sizeof(int)-1) != NULL) )
		return(EIO);
#endif /* mips */

	/*
	 * Get a procnode pointer from the vnode.
	 * Use the PROCFS_VNINVALID macro to verify that the vnode still
	 * represents the traced process.  If the macro evaluates to true,
	 * return ENOENT.  If the vnode is valid, check that the file
	 * descriptor is still valid, and get a pointer to the proc table
	 * entry for the traced process.
	 */
	pn = VNTOPN(vp);
	if(PROCFS_VNINVALID(pn) )
		return(ENOENT);
	if(pn->prc_flags & PRC_FDINVLD)
		return(EAGAIN);
	/*
	 * get a pointer to the proc table entry for the traced process.
	 */
	pr = &proc[pn->prc_index];

	/*
	 * Determine if/how much of the desired memory area is validly
	 * mapped.  If none is valid, return the error status from
	 * procfs_val_addrs().  nb gets the size of the valid target
	 * area in bytes.
	 */
	if( (error = procfs_val_addrs(pr, &nb, uio)) )
		return(error);

	/*
	 * The target process must be stopped before the read is started,
	 * and resumed after the read completes.  Task suspends are queued,
	 * so it is safe to do a task_suspend() / task_resume() irregardless
	 * of whether the task is currently suspended or running.
	 */
	if(pr->task == current_task()) {
		self = 1;
	} else {
	    if(task_suspend(pr->task) != KERN_SUCCESS)
		return(EAGAIN);
	    if(pr->task == NULL)
		return(ENOENT);
	}


	map = pr->task->map;

	/*
	 *
	 * nb contains the amount of data to read
	 *
	 * call vm_map_reference to map the file area to the kernel
	 * call vm_map_copyin
	 * call vm_map_deallocate - allow the kernel to see "tracing" process?
	 * call vm_map_copyout to get the "file" data into kernel space
	 *
	 * the actual "read" occurs here, it consists of a do while loop
	 * that essentially mimics the code in src/kernel/ufs/ufs_vnops.c
	 * do
	 *   call uiomove to copy from kernel memory space to user buffer
	 *
	 *   decrement the amount of data left to read, nb, by the amount
	 *   of data transferred by uiomove (uio_resid - lost)
	 *
	 * while - loop until error, or uio_resid is 0, or the amount of data
	 * to read is 0
	 */
	vm_map_reference(map);
	if(vm_map_copyin(map,uio->uio_offset,nb,FALSE,&vmcpy) != KERN_SUCCESS)
	{
		vm_map_deallocate(map);
		if(!self)
		    (void)task_resume(pr->task);
		return(EIO);
	}
	vm_map_deallocate(map);
	if(vm_map_copyout(PR_KERN_MAP, &kern_adr,vmcpy) != KERN_SUCCESS)
	{
		vm_map_copy_discard(vmcpy);
		(void)vm_deallocate(PR_KERN_MAP, kern_adr, nb);
		if(!self)
		    (void)task_resume(pr->task);
		return(EIO);
	}

	/*
	 * Save the amount of data requested to be read that was not mapped
	 * (lost), and set the initial amount of data to read (rd_siz) to the
	 * amount of data that procfs_val_addrs() determined was validly
	 * mapped (nb).
	 */
	lost = uio->uio_resid - nb;
	rd_siz = nb;
	do
	{
		error = uiomove((kern_adr +(nb - rd_siz)), rd_siz, uio);
		rd_siz -= (uio->uio_resid - lost);
	} while( (error == NULL) && (rd_siz > NULL) && (uio->uio_resid > NULL));

	/* free the kernel buffer that contained the data to be read */
	(void)vm_deallocate(PR_KERN_MAP, kern_adr, nb);

	/*
	 * Restart the task; if the restart failed and there were no errors
	 * encountered by the last uiomove(), return EAGAIN.
	 */
	if(!self)
	    if(task_resume(pr->task) != KERN_SUCCESS && error == NULL)
		return(EAGAIN);
	return(error);
}

/*
 * Vnode op for writing.
 *
 * Input parameters:
 *
 *	vp		vnode
 *	uio		uio structure pointer
 *	ioflag		unused
 *	cred		credentials for the tracing process, unused
 *
 * Outputs:
 *
 *
 *
 * Return value:
 *
 *			returns 0 for success, non-zero for failure.
 */

/*
 * VM writes are done on page boundaries.  The amount of data to write is
 * expanded to start at the first page boundary before uio_offset, and to
 * end at the next page boundary after uio_resid.  In the first example,
 * the entire area (nb) that the user wants to write is validly mapped and
 * overlaps two memory pages.  In the second example, only part of the area
 * that the user wants to write is validly mapped.  In this case, nb is
 * truncated to the end of the last valid page, and the amount of unmapped
 *  data is saved in variable "lost".
 *
 *
 * buffer layout
 *
 *       |         +++++++     start                +++++++  (start valid page)
 *       v         |     |       |         |        |     |
 *     offset      |     |       |         |        |     |
 *       ^         |     |       |         |        |     |
 *       |         |     |       |         |        |     |
 *  uio_offset     |     |       |         |        |     |
 *       |         |     |       v         |        |     |
 *       v         |     |                 |        |     |
 *    nb (wt_siz)  +++++++     vm_siz      |        +++++++  (page boundary )
 *       ^         |     |                 v        |     |
 *       |         |     |       ^        nb        |     |
 *       |         |     |       |         ^        |     |
 *       |         |     |       |         |        |     |
 *       |         |     |       |         |        |     |
 *   uio_resid     |     |       |         |        |     |
 *                 |     |       |         |        |     |
 *                 +++++++     end                  +++++++  (end valid mapping)
 *                                         |        |     |
 *                                         v        |     |
 *                                       lost       |     |
 *                                         ^        |     |
 *                                         |        |     |
 *                                      uio_resid   |     |
 */


/* ARGSUSED */
procfs_write(vp, uio, ioflag, cred)
	register struct vnode *vp;
	struct uio *uio;
	int ioflag;
	struct ucred *cred;
{
	register int self;
	int error, nb, wt_siz, lost, chprt;
	struct procnode *pn;
	struct proc *pr;
	vm_map_t map;
	vm_map_copy_t vmcpy;
	vm_offset_t kern_adr, start, end, offset;
	vm_size_t vm_siz;

	self = 0L;

	/*
	 * Do preliminary sanity checking.
	 */
	if(uio->uio_rw != UIO_WRITE)
		panic("procfs_write mode");
	if(uio->uio_resid == NULL)
		return(0);

	/*
	 * Sanity check, if the vnode is for the /proc directory itself,
	 * return EBADF.
	 */
	if( vp == proc_root )
		return(EBADF);

#ifdef mips
	/*
	 * The transfer must be a multiple of longwords and begin on a
	 * longword boundary.
	 */
	if( (uio->uio_offset & (sizeof(int)-1) != NULL) ||
	    (uio->uio_resid & (sizeof(int)-1) != NULL) )
		return(EIO);
#endif /* mips */

	/*
	 * Use the PROCFS_VNINVALID macro to verify that the vnode still
	 * represents the traced process.  If the macro evaluates to true,
	 * return ENOENT.  If the vnode is valid, then get a pointer to the
	 * proc table entry for the traced process.
	 */
	pn = VNTOPN(vp);
	if(PROCFS_VNINVALID(pn) )
		return(ENOENT);

	/*
	 * If the file descriptor is invalid, return EAGAIN.  This can
	 * occur if the traced process does an exec of a process that
	 * it does not have read permission for or that has setuid or
	 * setgid set.
	 */
	if(pn->prc_flags & PRC_FDINVLD)
		return(EAGAIN);
	/*
	 * get a pointer to the proc table entry for the traced process
	 */
	pr = &proc[pn->prc_index];


	/*
	 * Determine if/how much of the desired memory area is validly
	 * mapped.  If none is valid, return the error status from
	 * procfs_val_addrs().  nb gets the size of the valid target
	 * area in bytes.
	 */
	if( (error = procfs_val_addrs(pr, &nb, uio)) )
		return(error);


	/*
	 * The target process must be stopped before the write is started,
	 * and resumed after the write completes.  Task suspends are queued,
	 * so it is safe to do a task_suspend() / task_resume() irregardless
	 * of whether the task is currently suspended or running.
	 */
	if(pr->task == current_task()) {
		self = 1;
	} else {
	    if(task_suspend(pr->task) != KERN_SUCCESS)
		return(EAGAIN);
	    if(pr->task == NULL)
		return(ENOENT);
	}


	/*
	 * get a pointer to the map structure for the "traced" process
	 * set the write size for uiomove() to the number of bytes that
	 *    are validly mapped
	 * set lost to the number of bytes at the end of the buffer that
	 *    are not validly mapped
	 * get the va of the start of the page that maps the user requested
	 *    area
	 * get the va of the end of the page that maps the user requested area
	 * set vm_siz to the entire mapped area that contains the user
	 *    requested area
	 * set offset to the start of the user requested area within the
	 *    mapped area
	 */

	map = pr->task->map;
	wt_siz = nb;
	lost = uio->uio_resid - nb;
	start = trunc_page(uio->uio_offset);
	end = round_page( (uio->uio_offset + nb));
	vm_siz = end - start;
	offset = uio->uio_offset - start;

	/*
	 * get a kernel map to the "traced" process memory space
	 */
	vm_map_reference(map);
	if(vm_map_copyin(map, start, vm_siz, FALSE, &vmcpy) != KERN_SUCCESS)
	{
		vm_map_deallocate(map);
		if(!self)
		    (void)task_resume(pr->task);
		return(EIO);
	}

	/*
	 * copy the memory space so the kernel can get to it
	 */
	if(vm_map_copyout(PR_KERN_MAP, &kern_adr,vmcpy) != KERN_SUCCESS)
	{
		vm_map_deallocate(map);
		vm_map_copy_discard(vmcpy);
		(void)vm_deallocate(PR_KERN_MAP, kern_adr, vm_siz);
		if(!self)
		    (void)task_resume(pr->task);
		return(EIO);
	}

	/* copy the user data to the kernel buffer */
	do
	{
		error = uiomove((kern_adr + offset + (nb - wt_siz)),wt_siz,uio);
		wt_siz -= (uio->uio_resid - lost);
	} while( (error == NULL) && (wt_siz > NULL) && (uio->uio_resid > NULL));

	if(error)
	{
                vm_map_deallocate(map);
                (void)vm_deallocate(PR_KERN_MAP, kern_adr, vm_siz);
		if(!self)
                    (void)task_resume(pr->task);
		return(error);
	}

	/* map it back to the "traced" process */
	if(vm_map_copyin(PR_KERN_MAP, kern_adr, vm_siz, TRUE, &vmcpy) !=
	KERN_SUCCESS)
	{
		vm_map_deallocate(map);
		(void)vm_deallocate(PR_KERN_MAP, kern_adr, vm_siz);
		if(!self)
		    (void)task_resume(pr->task);
		return(EIO);
	}

	/*
	 * check memory protection on the "traced" process' memory, and
	 * set it writeable if necessary
	 */
	chprt = NULL;
        if (!vm_map_check_protection(map, start, end, VM_PROT_WRITE))
	{
                chprt = 1;
                (void) vm_map_protect(map, start, end, VM_PROT_ALL, FALSE);
        }

	/*
	 * write the new data into the "traced" process' memory space
	 */
        if (vm_map_copy_overwrite(map, start, vmcpy, FALSE) != KERN_SUCCESS)
                error = EIO;

        vm_map_copy_discard(vmcpy);

        /*
         * Re-protect the victim's memory, if it was changed.
         */
        if (chprt)
            (void)vm_map_protect(map, start, end, VM_PROT_READ|VM_PROT_EXECUTE,
	    FALSE);
#if defined(mips) || defined(__alpha)
        /*
         *      Flush Icache, as we might have changed the victim's code.
	 *	Note: use the ptrace flush code.
         */
#ifdef mips
	ptrace_user_flush(map, start, (end - start));
#else
	imb();
#endif
#endif /* mips || __alpha */

        vm_map_deallocate(map);

        /*
         *      Discard the kernel's copy.
         */
        (void) vm_deallocate(PR_KERN_MAP, kern_adr, vm_siz);

	/*
	 * Restart the task; if the restart failed and there were no errors
	 * encountered by the vm_map_copy_overwrite, return EAGAIN.
	 */
	if(!self)
	    if(task_resume(pr->task) != KERN_SUCCESS && error == NULL)
		return(EAGAIN);
	return(error);
}


/*
 * Seek on a file
 *
 *
 * Input parameters:
 *
 *	vp		vnode
 *	oldoff		current file offset, unused
 *	newoff		new file offset
 *	cred		credentials for the tracing process, unused
 *
 * Outputs:
 *
 *	none, just determine if the new offset is validly mapped
 *
 * Return value:
 *
 *			returns 0 for success, non-zero for failure.
 *
 */
/* ARGSUSED */
procfs_seek(vp, oldoff, newoff, cred)
	struct vnode *vp;
	off_t oldoff, newoff;
	struct ucred *cred;
{
	struct procnode *pn;
	struct proc *pr;
	vm_map_t map;
	struct vm_map_links *link, *lnk_end;


	pn = VNTOPN(vp);

	/* If this is a seek of the /proc directory file, return 0 if
	 * newoff is positive, and less than nproc+2 times the size
	 * of a directory entry.
	 *
	 * If the value is newoff fails the above check, return EINVAL.
	 */
	if( vp == proc_root)
	{
		if(newoff < NULL || (newoff >= (nproc+2)*sizeof(struct s5direct)) )
			return(EINVAL);
		return(0);
	}


	/* This code is for a seek on a file within /proc */
	 
#ifdef mips
	/*
	 * The seek must be to a  longword boundary.
	 */
	if( (newoff & (sizeof(int)-1) != NULL) )
		return(EIO);
#endif /* mips */

	/*
	 * If the file descriptor is invalid, return EAGAIN.  This can
	 * occur if the traced process does an exec of a process that
	 * it does not have read permission for or that has setuid or
	 * setgid set.
	 */
	if(pn->prc_flags & PRC_FDINVLD)
		return(EAGAIN);

	/*
	 * Use the index in the procnode to verify that the proc table
	 * p_pid is the same as the one in the procnode.  Compare the
	 * procnode creation time, prc_time, with the start time in the
	 * proc table entry, proc[index]->utask.uu_start.  If either check
	 * fails, return error ENOENT. 
	 */
	if(PROCFS_VNINVALID(pn) )
		return(ENOENT);


	/*
	 * get a pointer to the proc table entry that represents the traced
	 * process
	 * get a pointer to the map (address map data structure) from
	 * proc->task->map
	 *
	 * lock the map via the vm_map_lock macro
	 *
	 * save the end of the linked list of address_map_entries in lnk_end
	 * from map.vm_links.prev
	 *
	 * set the initial link pointer to follow as link = map.vm_links
	 */
	pr = &proc[pn->prc_index];
	map = pr->task->map;
	vm_map_lock(map);
	lnk_end = &(map->vm_links);
	link = &(map->vm_links.next->vme_links);

	/*
	 * while loop - while link != lnk_end
	 *
	 *   see if the current address_map_entry contains the new address
	 *   ( if newoff >= link.start && newoff <= link.end )
	 *
	 *   if not, advance to the next address_map_entry and check it
	 *   ( link = link.next; continue )
	 *
	 * unlock the map via the vm_map_unlock macro
	 *
	 * if the requested address was not found (link == lnk_end)
	 * return error EINVAL
	 */

	while(link != lnk_end)
	{
		if( (vm_offset_t)newoff >= link->start &&
		(vm_offset_t)newoff <= link->end )
			break;
		else
			link = &(link->next->vme_links);
	}
	vm_map_unlock(map);
	if(link == lnk_end)
		return(EINVAL);
	return(0);
}



/*
 * Vnode op for reading a directory
 *
 *
 * Input parameters:
 *
 *	vp		vnode
 *	uio		pointer to a uio struct, input params & buff addr
 *	cred		credentials for the tracing process, unused
 *	eofflagp	end of file (EOF) output status
 *
 * Outputs:
 *
 *	uio		uio_resid incremented by amt of data read, buff filled
 *	eofflagp	set to 1 if the end of the directory was reached
 *
 * Return value:
 *
 *			returns 0 for success, non-zero for failure.
 *
 */
/* ARGSUSED */
procfs_readdir(vp, uio, cred, eofflagp)
	struct vnode *vp;
	register struct uio *uio;
	struct ucred *cred;
	int *eofflagp;
{
	int lost, nb, error;

/* this initiates from getdirentries() in vfs/vfs_syscalls.c */


/*
 * Note, this routine copies "unused" entries (pid of 0) to the
 * user buffer; this may be incorrect.  It is unclear if the base
 * system call code removes unused entries or not, but the readdir(3)
 * man page says that unused entries are not returned to the user.
 */

	/*
	 * If the vnode is not for the /proc directory, return EINVAL.
	 */
	if(vp != proc_root)
		return(EINVAL);

	/*
	 * If the file offset to read at, uio->uio_offset is greater
	 * than (nproc+2) * sizeof(dirent), then set the end of file
	 * flag and  return 0 (End_OF_File)
	 */
	if(uio->uio_offset > ((nproc+2) * sizeof(pr_g_dirent_t)))
	{
		*eofflagp = 1;
		return(0);
	}
	if(uio->uio_offset + uio->uio_resid >= (nproc+2)*sizeof(pr_g_dirent_t))
		*eofflagp = 1;
	else
		*eofflagp = NULL;

	/*
	 * If the current time, time.tv_secs, is more than 2 seconds
	 * greater than proc_gdirtime, then call proc_getgdir()
	 */
	(void)procfs_get_g_dir();

	/*
	 * If the file offset is not modulo dir size, return error EINVAL.
	 */
	if((uio->uio_offset % sizeof(pr_g_dirent_t)) != NULL)
		return(EINVAL);

	/*
	 * Set the copy_size to be mod dir size of the length, and save any
	 * remainder in "lost" to be added to uio->resid once uiomove is done.
	 */
	lost = uio->uio_resid % sizeof(pr_g_dirent_t);

	/* Set the number of bytes to copy, nb, to the user buffer to the
	 * minimum of the requested length (mod dir size), copy_size, or the
	 * amount of data in the gdir_buffer from the desired offset
	 * to the end of the gdir_buffer (gdir_buf_size - uio->uio_offset).
	 */
	nb = (nproc * sizeof(pr_g_dirent_t)) - uio->uio_offset;
	if (nb > uio->uio_resid)
		nb = uio->uio_resid - lost;

	/* call uiomove to copy nb data  bytes from the gdir_buffer starting
	 * at the desired uio->uio_offset, to the user buffer
	 */
	error = uiomove((pr_g_dirbuf + uio->uio_offset), nb, uio);

	/*
	 * Add the value in "lost" to uio->resid, and
	 * return with the status from uiomove
	 */
	uio->uio_resid += lost;
	return(error);

}


/*
 * This should never be called for procfs, because only "regular files" can
 * exist in the /proc directory.
 */
/* ARGSUSED */
procfs_abortop(ndp)
	register struct nameidata *ndp;
{
	return(0);
}


/*
 * procfs_inactive
 *
 *
 * Input parameters:
 *
 *	vp		vnode
 *
 * Outputs:
 *
 *	no explicit output, the vnode will be freed if its ref count is 1
 *
 * Return value:
 *
 *	Returns 0 for success, non-zero for failure.
 *
 */

procfs_inactive(vp)
        struct vnode *vp;
{
	uid_t tmp_uid;
	gid_t tmp_gid;
	register int tmp_prot, restart_task, restart_thread;
	register thread_t thread;
	register struct vnode *tmp_vp;
	struct procnode *pn;

	/* If vnode for root of /proc file system, check usecount and
	 * vgone it if count is 1
	 */
	if( vp == proc_root)
	{
		if(vp->v_usecount == 1)
		{
			VN_LOCK(vp);
			(void) vgone(vp, VX_NOSLEEP, NULL);
			VN_UNLOCK(vp);
		}
		return(0);
	}

	/*
	 * If the vnode usecount is greater than 1, we just return 0.  If
	 * the usecount is 1, then this is the last open file descriptor,
	 * and the following two code segments must be run.
	 */
	 if(vp->v_usecount > 1)
		return(0);

	/*
	 * This code must check the run-on-last-close flag in the traced
	 * process.  This check can only be done if the process still exists
	 * i.e. if it has not already terminated and the task structure is gone.
	 * If set, all the tracing flags for syscalls, signals, and traps
	 * must be cleared, and if the traced process is stopped it must be
	 * restarted.
	 * In order to clean up the vnode, we must lock the vnode.
	 */
	pn = VNTOPN(vp);
	VN_LOCK(vp);

	restart_task = FALSE;
	if((pn->prc_pid == proc[pn->prc_index].p_pid) &&
		(proc[pn->prc_index].task != NULL) &&
		(proc[pn->prc_index].p_stat != NULL)) {
		unix_master();
		if(proc[pn->prc_index].task->procfs.pr_flags & PR_RLC) {
			if(proc[pn->prc_index].task->procfs.pr_flags & PR_ISTOP)
				restart_task = TRUE;

			/* save uid/gid/perm */

			tmp_uid = proc[pn->prc_index].task->procfs.pr_uid;
			tmp_gid = proc[pn->prc_index].task->procfs.pr_gid;
			tmp_prot = proc[pn->prc_index].task->procfs.pr_protect;
			tmp_vp = proc[pn->prc_index].task->procfs.pr_exvp;
			bzero(&(proc[pn->prc_index].task->procfs), sizeof(procfs_t));
			proc[pn->prc_index].p_pr_qflags = NULL;

			/* reset uid/gid/perm and run on last close*/

			proc[pn->prc_index].task->procfs.pr_uid = tmp_uid;
			proc[pn->prc_index].task->procfs.pr_gid = tmp_gid;
			proc[pn->prc_index].task->procfs.pr_protect = tmp_prot;
			proc[pn->prc_index].task->procfs.pr_exvp = tmp_vp;
		
		}
		for (thread = (thread_t)
			queue_first(&proc[pn->prc_index].task->thread_list);
		    !queue_end(&proc[pn->prc_index].task->thread_list,
				(queue_entry_t) thread);
		    thread = (thread_t) queue_next(&thread->thread_list)) {
			if(thread->t_procfs.pr_flags & PR_RLC) {
				thread_lock(thread);
				restart_thread = FALSE;
				if(thread->t_procfs.pr_flags & PR_ISTOP) {
					restart_thread = TRUE;
				}
				bzero(&thread->t_procfs, sizeof(t_procfs_t));
				if(restart_thread && !restart_task) {
					thread->state &= ~TH_SUSP;
					thread->state |= TH_RUN;
					if(--thread->suspend_count == NULL)
						thread->halted = FALSE;
					thread_unlock(thread);
					unix_release();
					thread_continue(thread);
					unix_master();
					thread_lock(thread);
				}
				thread_unlock(thread);
			}
		}
		if(restart_task)
			task_resume(proc[pn->prc_index].task);
		unix_release();
	}

	/*
	 * Clear the proc table p_vnptr, to prevent exit() from
	 * trying to store the "traced" process exit status there.
	 * Lock the proc table via unix_master before clearing the
	 * vnode pointer in the proc entry.
	 */

	unix_master();
	proc[pn->prc_index].p_vnptr = NULLVP;
	/* make sure the task structure is valid. */
	if((pn->prc_pid == proc[pn->prc_index].p_pid) &&
		(proc[pn->prc_index].task != NULL) &&
		(proc[pn->prc_index].p_stat != NULL)) {
		proc[pn->prc_index].task->procfs.pr_qflags &= ~PRFS_OPEN;
	}
	unix_release();

	/*
	 * In order to clean up the vnode call
	 * vgone if its use count is 1, and unlock the vnode.
	 * vgone takes care of removing the vnode from whatever mount
	 * struct it is on, resetting the vnodeops pointer, etc.
	 */

	if( vp->v_usecount == 1)
		(void) vgone(vp, VX_NOSLEEP, NULL);
	VN_UNLOCK(vp);

	return(0);
}





/*
 * procfs_reclaim
 */
/* ARGSUSED */
procfs_reclaim(vp)
        register struct vnode *vp;
{
	return(0);
}



/*
 * Print out the contents of a vnode that would be useful for debugging.
 *
 *
 * Input parameters:
 *
 *	vp		vnode
 *
 * Outputs:
 *
 *	This routine calls printf() to output data to the console.
 *
 * Return value:
 *
 *	Returns 0 for success, non-zero for failure.
 *
 */
procfs_print(vp)
	struct vnode *vp;
{
	struct procnode *pn;

	pn = VNTOPN(vp);
	/*
	 * call printf to output the following vnode fields:
	 * v_flag %x, v_usecount %d, v_rdcnt %d, v_wrcnt %d,
	 * v_holdcnt %d, v_shlockc %d, v_exlockc %d, and v_mount %x
	 */
	 printf("procfs_print:\n\tvp address = 0x%x, v_flag 0x%x, v_usecount %d\n",
		 vp, vp->v_flag, vp->v_usecount);
	 printf("\tv_wrcnt %d, v_holdcnt %d, v_shlockc %d\n",
		 vp->v_wrcnt, vp->v_holdcnt, vp->v_shlockc);
	 printf("\tv_exlockc %d, v_mount 0x%x, v_rdcnt %d\n",
		 vp->v_exlockc, vp->v_mount, vp->v_rdcnt);
	printf("\t pn address = 0x%x\n\tprc_flags = 0x%x, prc_pid = %d, prc_index = %d\n",
		pn, pn->prc_flags, pn->prc_pid, pn->prc_index);
	printf("\tprc_excl = 0x%x, prc_exitval = %d, *prc_vp = 0x%x\n",
		pn->prc_excl, pn->prc_exitval, pn->prc_vp);
}


/*
 * read a page
 */
/* ARGSUSED */
procfs_page_read(vp, uio, cred)
        struct vnode *vp;
        struct uio *uio;
        struct ucred *cred;
{
	/*
	 * return error EINVAL
	 */
	return(EINVAL);

}

/*
 * write a page
 */
/* ARGSUSED */
procfs_page_write(vp, uio, cred, pager, offset)
        struct vnode    *vp;
        struct uio      *uio;
        struct ucred    *cred;
        memory_object_t pager;
        vm_offset_t     offset;
{
	/*
	 * return error EINVAL
	 */
	return(EINVAL);

}


/* This is for the procfs_select() vnode operation; it was added for
 * multi-thread
 * debugger support.
 */

/*
 * select for procfs
 *
 * Inputs:
 *	vp		vnode of process to search
 *	events		mask of events to search for 
 *	revents		mask of events found 
 *	scanning	? see bsd/sys_generic.c 
 *
 *
 * Output:
 *	copies events to revents if any thread in the process is stopped on
 *	an event of interest
 * Return:
 *	0 if all OK
 *	EBADF if requested to check for read or write activity
 */


procfs_select(vp, events, revents, scanning, cred)
	struct vnode *vp;		/* vnode for process to check */
	short  *events;			/* mask of events to search for */
	short  *revents;		/* mask of events found */
	int    scanning;		/* ? see bsd/sys_generic.c */
	struct ucred *cred;		/* calling process credentials */
{
	struct procnode *pn;
	struct task *task;
	struct thread *thread;


	pn = VNTOPN(vp);

	/*
	 * If the file descriptor is invalid, return EAGAIN.  This can
	 * occur if the traced process does an exec of a process that
	 * it does not have read permission for or that has setuid or
	 * setgid set.
	 */
	if(pn->prc_flags & PRC_FDINVLD)
		return(EAGAIN);

	/*
	 * Use the index in the procnode to verify that the proc table
	 * p_pid is the same as the one in the procnode.  Compare the
	 * procnode creation time, prc_time, with the start time in the
	 * proc table entry, proc[index]->utask.uu_start.  If either check
	 * fails, return error ENOENT. 
	 */
	if(PROCFS_VNINVALID(pn) )
		return(ENOENT);


	/*
	 * If POLLNORM or POLLOUT are set in events,
	 *	return EBADF
	 *
	 * If POLLPRI is not set in events, or scanning is 0,
	 *	return 0
	 *
	 * Using the vnode pointer, get the task pointer and scan the
	 * task thread_list.
	 * For all threads in the process
	 * 	If the PR_ISTOP bit is set in thread->t_procfs.pr_flags)
	 *		copy *events to *revents
	 *		return 0
	 */
	if( (*events & POLLNORM) || (*events & POLLOUT))
		return(EBADF);
	if( !(*events & POLLPRI) || (scanning == NULL))
		return(0);

	task = proc[pn->prc_index].task;

	if(task->procfs.pr_flags & PR_ISTOP)
	{
		*revents = *events;
		return(0);
	}

	task_lock(task);
	for (thread = (thread_t) queue_first(&task->thread_list);
	!queue_end(&task->thread_list, (queue_entry_t) thread);
	thread = (thread_t) queue_next(&thread->thread_list))
		if(thread->t_procfs.pr_flags & PR_ISTOP)
		{
			*revents = *events;
			break;
		}
	task_unlock(task);

	return(0);
}




/*
 * Routine to fill in the s5 dir_buffer with current data from the
 * proc table.
 */

procfs_get_s5_dir()
{
	register int bcnt, prcnt, i, j;
	int len, shift;
	char  *lptr, lbuf[80];

	/*
	 * Get a write lock on the pr_s5_dirbuf
	 */
	PR_DLOCK(s5);

	/*
	 * If someone has already updated the buffer, return
	 * Add check for current system time >= saved timestamp, so that if
	 * the system time is set to less than its current value, the
	 * pr_s5_dirbuf will never be updated until the system time gets past
	 * the saved value in pr_s5_timestamp.
	 */
	if(time.tv_sec >= pr_s5_timestamp.tv_sec &&
	   time.tv_sec <= (pr_s5_timestamp.tv_sec + PR_LAG_SEC)) {
		PR_DUNLOCK(s5);
		return;
	}

	/*
	 * set the first entry in the s5dir_buffer to be inode 2,
	 * file name . (dot)
	 */
	pr_s5_dirbuf[0].d_ino = 2;
	bcopy(".",pr_s5_dirbuf[0].d_name, 2);

	/*
	 * set the second entry in the s5dir_buffer to be inode 2,
	 * file name .. (dot-dot), 
	 * Note, both file names are NULL terminated.
	 */
	pr_s5_dirbuf[1].d_ino = 2;
	bcopy("..",pr_s5_dirbuf[1].d_name, 3);


	/* To fill in the rest of the s5 dir_buffer, scan the proc table
	 * (We are not locking it, just getting a snapshot) from 0 to
	 * nproc-1 and for each entry, set the s5 dir_buffer inode entry
	 * to proc[n].p_pid +64, set the s5 dir_buffer file name entry to
	 * proc[n]p_pid converted to ascii characters.  A System V filename
	 * does not need to be NULL terminated.  The conversion is done as
	 * follows: 1. if the file name is greater than s5DIRSIZ (historically
	 * 14 characters) it is truncated to s5DIRSIZ.  2. if the file name
	 * is less than PR_MIN_NAME_SIZ (currently 5 characters) it it zero
	 * extended to PR_MIN_NAME_SIZ; pid 0 would be 00000, pid 1 00001, etc.
	 * 3. If neither condition above exists, the file name is just copied
	 * to the pr_s5_dirbuf buffer.
	 * (File names are NULL terminated if less than s5DIRSIZ characters.)
	 *
	 * Note, only proc table entry 0 has an inode value of 64, any other
	 * proc table entry with a p_pid of 0 is considered to be an unused
	 * slot, and its reported inode value is set to 0.
	 */
	pr_s5_dirbuf[2].d_ino = 64;
	bcopy("00000",pr_s5_dirbuf[2].d_name, 6);

	for(bcnt = 3, prcnt = 1; prcnt < nproc; bcnt++, prcnt++){
		if(proc[prcnt].p_pid != NULL )
		{
			pr_s5_dirbuf[bcnt].d_ino =
			(s5ino_t)(proc[prcnt].p_pid+64);

			lptr = &lbuf[PR_MIN_NAME_SZ];
			if(((len = itoa(proc[prcnt].p_pid, lptr))) >= s5DIRSIZ)
			{
				bcopy(lptr, pr_s5_dirbuf[bcnt].d_name,
				s5DIRSIZ);
			}

#if defined(mips) || defined(__alpha)

			else if( len < (PR_MIN_NAME_SZ))
			{
				shift = PR_MIN_NAME_SZ - len;
				for( i=0; i < shift; i++, len++)
					*(--lptr) = '0';
				bcopy(lptr, pr_s5_dirbuf[bcnt].d_name, len+1);

			}
#endif
			else
			{
				bcopy(lptr, pr_s5_dirbuf[bcnt].d_name, len+1);
			}
		}
		else
		{
			pr_s5_dirbuf[bcnt].d_ino = (s5ino_t)0;
			pr_s5_dirbuf[bcnt].d_name[0] = '\0';
		}
	}

	/*
	 * set proc_s5dirtime to the current system time, the global
	 * struct time
	 */
	 pr_s5_timestamp = time;

	/*
	 * give up the write lock on the s5dir_buffer, and return 0
	 */
	PR_DUNLOCK(s5);
	return;
}





/*
 * Routine to fill in the gdir_buffer with current data from the
 * proc table.  This belongs in some procfs_subr.c file or something.
 */

procfs_get_g_dir()
{
	register int bcnt, prcnt, i, j;
	int len, shift;
	char lbuf[80], *lptr;

	/*
	 * Get a write lock on the g_dirbuf
	 */
	PR_DLOCK(g);

	/*
	 * If someone has already updated the buffer, return
	 * Add check for current system time >= saved timestamp, so that if
	 * the system time is set to less than its current value, the
	 * pr_g_dirbuf will never be updated until the system time gets past
	 * the saved value in pr_g_timestamp.
	 */
	if(time.tv_sec >= pr_g_timestamp.tv_sec &&
	   time.tv_sec <= (pr_g_timestamp.tv_sec + PR_LAG_SEC)) {
		PR_DUNLOCK(g);
		return;
	}

	/*
	 * set the first entry in the gdir_buffer to be inode 2,
	 * file name . (dot), p_reclen to sizeof struct pg_dirent,
	 * p_namelen to 1, and set p_fill to 0.
	 */
	pr_g_dirbuf[0].d_ino = 2;
	pr_g_dirbuf[0].d_reclen = sizeof(pr_g_dirent_t);
	pr_g_dirbuf[0].d_namlen = 1;
	bcopy(".",pr_g_dirbuf[0].d_name, 2);

	/*
	 * set the second entry in the gdir_buffer to be inode 2,
	 * file name .. (dot-dot), p_reclen to sizeof struct pg_dirent,
	 * p_namelen to 2, and set p_fill to 0.
	 * Note, both file names are NULL terminated.
	 */
	pr_g_dirbuf[1].d_ino = 2;
	pr_g_dirbuf[1].d_reclen = sizeof(pr_g_dirent_t);
	pr_g_dirbuf[1].d_namlen = 2;
	bcopy("..",pr_g_dirbuf[1].d_name, 3);


	/* To fill in the rest of the gdir_buffer, scan the proc table
	 * (We are not locking it, just getting a snapshot) from 0 to
	 * nproc-1 and for each entry, set the gdir_buffer inode entry
	 * to proc[n].p_pid +64, set the gdir_buffer file name entry to
	 * proc[n]p_pid converted to 5 ascii characters - 0 would be 00000,
	 * 1 would be 00001, etc., * set the record length entry to
	 * sizeof( struct pr_g_dirent), and set the namelength to 5. (Note,
	 * file names are NULL terminated.)
	 *
	 * Note, only proc table entry 0 has an inode value of 64, any other
	 * proc table entry with a p_pid of 0 is considered to be an unused
	 * slot, and its reported inode value is set to 0.  So do it outside
	 * the loop.
	 */
	pr_g_dirbuf[2].d_ino = 64;
	pr_g_dirbuf[2].d_reclen = sizeof(pr_g_dirent_t);
	pr_g_dirbuf[2].d_namlen = PR_MAX_NAME_SZ;
	bcopy("00000",pr_g_dirbuf[2].d_name, 6);


	for(bcnt = 3, prcnt = 1; prcnt < nproc; prcnt++){
		if(proc[prcnt].p_pid != NULL )
		{
			pr_g_dirbuf[bcnt].d_ino = (ino_t)(proc[prcnt].p_pid+64);

			lptr = &lbuf[PR_MIN_NAME_SZ];
			if(((len=itoa(proc[prcnt].p_pid,lptr))) > PR_MAX_NAME_SZ)
			{
				len = PR_MAX_NAME_SZ;
				bcopy(lptr, pr_g_dirbuf[bcnt].d_name,
				PR_MAX_NAME_SZ);
				*(lptr+PR_MAX_NAME_SZ) = '\0';
			}

#if defined(mips) || defined(__alpha)

			else if( len < (PR_MIN_NAME_SZ))
			{
				shift = PR_MIN_NAME_SZ - len;
				for( i=0; i < shift; i++, len++)
					*(--lptr) = '0';
				bcopy(lptr, pr_g_dirbuf[bcnt].d_name, len+1);

			}
#endif
			else
			{
				bcopy(lptr, pr_g_dirbuf[bcnt].d_name, len+1);
			}
			pr_g_dirbuf[bcnt].d_reclen = sizeof(pr_g_dirent_t);
			pr_g_dirbuf[bcnt].d_namlen = len;
			bcnt++;
		}
	}

	/*
	 * If there were any unused entried in the proc table (bcnt < nproc+2),
	 * then zero the rest of the table.  And, set the d_reclen entry of
	 * the first "unused" entry to the remaining size of the buffer.
	 * The buffer is nproc + 2 entries, to allow space for '.' and '..' .
	 */

	if( bcnt < (nproc+2) )
	{
		pr_g_dirbuf[bcnt].d_reclen =
			((nproc+2)-bcnt)*sizeof(struct pr_generic_dir);
		pr_g_dirbuf[bcnt].d_ino = (s5ino_t)0;
		pr_g_dirbuf[bcnt].d_name[0] = '\0';
		pr_g_dirbuf[bcnt].d_namlen = NULL;
	}

	/*
	 * set proc_gdirtime to the current system time, the global
	 * struct time
	 */
	 pr_g_timestamp = time;

	/*
	 * give up the write lock on the gdir_buffer, and return 0
	 */
	PR_DUNLOCK(g);
	return;
}



/*
 * Determine if an address range is validly mapped.
 *
 * Input parameters:
 *
 *	pr		pointer to a procnode structure
 *	nb		pointer to an int
 *	uio		uio structure pointer
 *
 * Outputs:
 *
 *	nb		filled in with the number of validly mapped bytes
 *			in the traced process memory map
 *
 * Return value:
 *
 *			returns 0 for success, non-zero for failure.
 */
int procfs_val_addrs(pr, nb, uio)
struct proc *pr;
int *nb;
struct uio *uio;
{

	long start_found, end_found;
	vm_map_t map;
	struct vm_map_links *link, *lnk_end;
	vm_offset_t end_addr;

	/*
	 * get a pointer to the map (address map data structure) from
	 * pr->task->map
	 *
	 * lock the map via the vm_map_lock macro
	 *
	 * calculate the desired ending read address, end_addr, as
	 * uio_offset + uio_resid
	 *
	 * save the end of the linked list of address_map_entries in lnk_end
	 * from map->vm_links.prev
	 *
	 * set the initial link pointer to follow as link = map->vm_links
	 *
	 * Set the number of bytes to read, nb, to uio_resid for now.  This
	 * value can be reduced if the entire area to read from is not mapped.
	 */
	map = pr->task->map;
	vm_map_lock(map);
	end_addr = (vm_offset_t)(uio->uio_offset + uio->uio_resid);
	lnk_end = &(map->vm_links);
	link = &(map->vm_links.next->vme_links);
	*nb = uio->uio_resid;

	/*
	 * massive while loop - while link != lnk_end
	 *
	 *   see if the current address_map_entry contains our starting address
	 *   ( if uio_offset >= link.start && uio_offset <= link.end )
	 *
	 *   if not, advance to the next address_map_entry and check it
	 *   ( link = link.next; continue )
	 *
	 *   the starting address is valid, check the ending address
	 *   if this address_map_entry contains the ending address, we are done
	 *   ( if end_addr >= link.start && end_addr <= link.end )
	 *	if it does, the amount of data to read is uio_resid, and we
	 *	break out of the while loop
	 *
	 *   at this point, we have a valid starting address, but the current
	 *   address_map_entry does not map all the data that the read wants
	 *   so, we scan the linked list of address_map_entries, for
	 *   contiguous entries, and stop when we either find one that contains
	 *   the desired ending address (the amount of data to read is is
	 *   uio_resid), or we hit one that is not contiguous OR the end of
	 *   the list is reached (the amount of data to read is the end of
	 *   the last contiguous OR valid address_map_entry minus the starting
	 *   address [link.end - uio_offset])
	 */
	start_found = end_found = NULL;

	while(link != lnk_end)
	{
		if(start_found == NULL)
		{
			if( (vm_offset_t)uio->uio_offset >= link->start &&
			    (vm_offset_t)uio->uio_offset <= link->end )
			{
				start_found = 1;
			}
			else
			{
				link = &(link->next->vme_links);
				continue;
			}
		}

		/* if we get here, the starting address is mapped */
		if((vm_offset_t)(uio->uio_offset + uio->uio_resid) <= link->end)
		{
			end_found = 1;
			break;
		}
		else
		{
			/* is the next address map entry contiguous ? or
			 * is this the last entry on the list ?
			 */
			if((link->end != link->next->vme_links.start) ||
			   ( lnk_end == &(link->next->vme_links) ) )
			{
				end_found = 1;
				*nb = (int)(uio->uio_offset - link->end);
				break;
			}
			else
				link = &(link->next->vme_links);
		}
	}

	/*
	 * unlock the map via the vm_map_unlock macro
	 */
	vm_map_unlock(map);

	/*
	 * if the requested address was not found return error EINVAL
	 */
	if(end_found == NULL)
		return(EINVAL);
	return(0);
}

/*
 * procfs_ioctl
 * 
 * This is the /proc filesystem specific IOCTL routine that will be put into
 * the procfs_vnops.c file.  It is called from the generic ioctl routine in
 * sys_generic.c.  The procfs_ioctl routine processes /proc
 * specific IOCTL commands.
 * 
 * Inputs: vp - vnode pointer
 * 	com - ioctl command (see section below
 * 	data - address of data passed to or from kernel
 * 	fflag - flags
 * 	cred - credentials structure
 * 
 * Outputs - Several structures containing status and information may be
 * 	  written into the data area pointed to by data.
 * 
 * Description -
 *	This routine parses the command field, checks the traced process
 *	for certain status types that only matter to the IOCTL work, and
 *	processes the individual commands.
 */
procfs_ioctl(vp, com, data, fflag, cred)
struct vnode *vp;
u_int com;
caddr_t data;
int fflag;
struct ucred *cred;
{
	register int i, tracing, nb, chprt, *spcactptr, fp_stop;
	register char *chptr;
	register struct procnode *pn;
	struct proc *procp;
	register struct thread **threadp, *thread;
	prthreads_t *prthsp;
	struct proc *cp;
	struct prcred *crp;
	struct user *up;
	struct prpsinfo *prpsp;
	struct rusage *lru;
	struct prrun *prrunp;
	prabrun_t *abdata;
	gid_t *gp;
	struct prmap *mp;
	vm_map_t map;
	vm_map_copy_t vmcpy;
	vm_map_entry_t map_entry;
	vm_offset_t kern_adr;
	int mflags, error;
 	struct prstatus *prstatp;
	int s;
	sigqueue_t sigqueue;
	struct sigaction *ap;
	struct sigaction action;
#ifdef __alpha
	struct args {
			 long	pid;
			 long	signo;
	} uap;

	long retval;
#else
	struct args {
			 int	pid;
			 int	signo;
	} uap;

	int retval;
#endif
	int count, runit, last, infotype;
	sigset_t holdsigs;

	char *tdata;
	struct PROCFS_thread_state *tmpstate, *regp;
	long *w_regp;

	vm_offset_t wait_event = (vm_offset_t)NULL;
	retval = NULL;

	/* If the command group is not 'F' ( a procfs ioctl) return EINVAL */
	if( (com & PR_GRP_MASK) != PR_GRP_DEF )
		return(EINVAL);

	cp = proc + current_task()->proc_index;
	pn = VNTOPN(vp);
	infotype = FALSE;
	/*
	 * If the file descriptor is invalid, return EAGAIN.  This can
	 * occur if the traced process does an exec of a process that
	 * it does not have read permission for or that has setuid or
	 * setgid set.
	 */
	if(pn->prc_flags & PRC_FDINVLD)
		return(EAGAIN);

	if((PROCFS_VNINVALID(pn) ) && (com != PIOCSTATUS) && (com != PIOCPSINFO)) {
		return(ENOENT);
	}
	else infotype = TRUE;

	procp = proc + pn->prc_index;
	if((procp > (proc + nproc)) || (procp < proc)) {
		return(EBADF);
	}

	/* check to see if there is a task structure, if not, the task is
	 * still being spawned and is not yet here */
	if((procp->task == NULL) && !infotype) {
		return(ENOENT);
	}

	if(procp->task != NULL) {
		/* if task or thread is being traced by ptrace, set a flag in
		 * the /proc flags field to indicate this.  Otherwise, turn it
		 * off */
		if (procp->p_flag & STRC)
			procp->task->procfs.pr_flags |= PR_PTRACE;
		else
			procp->task->procfs.pr_flags &= ~PR_PTRACE;

		/* if this is a system task or thread, set a flag in the
		 * /proc flags field to indicate this. */
		if (procp->utask == NULL || procp->utask->uu_tsize == NULL)
			procp->task->procfs.pr_flags |= PR_ISSYS;
	}
	/* command processing */
	switch(com & PR_CMD_MASK) {

	case PIOCCRED & PR_CMD_MASK:		/* get credentials */
		/* copy the credentials for the traced task from the
		 * kernel to the prcred structure in user space */
		crp = (struct prcred *)data;
		crp->pr_euid = procp->p_rcred->cr_uid;
		crp->pr_ruid = procp->p_ruid;
		crp->pr_suid = procp->p_svuid;
		crp->pr_egid = procp->p_rcred->cr_gid;
		crp->pr_rgid = procp->p_rgid;
		crp->pr_sgid = procp->p_svgid;
		crp->pr_ngroups = procp->p_rcred->cr_ngroups;
		break;

	case PIOCGETPR & PR_CMD_MASK:		/* get proc structure */
		/* copy the proc structure for the traced task from the
		 * kernel proc structure to the proc structure in user
		 * space */
		bcopy(procp,(struct proc *)data,sizeof(struct proc));
		break;
#ifdef PROCFS_U
	case PIOCGETTK & PR_CMD_MASK:		/* get Task structure */
		/* copy the Task structure for the traced task from the
		 * kernel Task structure to the Task structure in user
		 * space */
		bcopy(procp->task,(struct task *)data,sizeof(struct task));
		break;
#endif
	case PIOCGETUTK & PR_CMD_MASK:		/* get utask structure */
		/* copy the utask structure for the traced task from the
		 * kernel utask structure to the utask structure in user
		 * space */
		if(procp->utask != NULL)
		  bcopy(procp->utask,(struct utask *)data,sizeof(struct utask));
		break;

	case PIOCGETU & PR_CMD_MASK:		/* get user structure */
		/* copy the user structure information for the traced
		 * task from the corresponding kernel data structures to
		 * the user structure in user space */
		up = (struct user *)data;

		return(EACCES);
#ifdef PROCFS_U
		/* Not reached - unimplimented  just like SVR4 */
		/* Map in the uthread, utask and other user areas */
	/* ZZZ !!! Need to map in pcb command and logname mem maps? !!! ZZZ */
		bcopy(&procp->thread->pcb,&up->u_/*not_a_macro*/pcb,
			sizeof(up->u_/*not_a_macro*/pcb));
		up->u_/*not_a_macro*/procp = procp->utask->uu_procp;
		up->u_/*not_a_macro*/ar0 = &((procp->thread->pcb->pcb_regs)[PCB_A0]);
		bcopy(procp->utask->uu_comm,up->u_/*not_a_macro*/comm,
			sizeof(up->u_/*not_a_macro*/comm));
		bcopy(procp->utask->uu_logname,up->u_/*not_a_macro*/logname,
			sizeof(up->u_/*not_a_macro*/logname));
		up->u_/*not_a_macro*/tsize = procp->u_tsize;
		up->u_/*not_a_macro*/dsize = procp->u_dsize;
		up->u_/*not_a_macro*/ssize = procp->u_ssize;
		up->u_/*not_a_macro*/text_start = procp->u_text_start;
		up->u_/*not_a_macro*/data_start = procp->u_data_start;
		up->u_/*not_a_macro*/stack_start = procp->u_stack_start;
		up->u_/*not_a_macro*/stack_end = procp->u_stack_end;
		up->u_/*not_a_macro*/stack_grows_up = procp->u_stack_grows_up;
		up->u_/*not_a_macro*/outime = procp->u_outime;
		bcopy(procp->utask->uu_signal,up->u_/*not_a_macro*/signal,
			sizeof(up->u_/*not_a_macro*/signal));
		bcopy(procp->utask->uu_sigmask,up->u_/*not_a_macro*/sigmask,
			sizeof(up->u_/*not_a_macro*/sigmask));
		up->u_/*not_a_macro*/sigonstack = procp->utask->uu_sigonstack;
		up->u_/*not_a_macro*/sigintr = procp->utask->uu_sigintr;
		up->u_/*not_a_macro*/oldmask = procp->utask->uu_oldmask;
		up->u_/*not_a_macro*/code =
			procp->thread->u_address.uthread->uu_code;
		bcopy(&procp->utask->uu_sigstack,&up->u_/*not_a_macro*/sigstack,
			sizeof(struct sigstack));
		bcopy(&procp->utask->uu_ru,&up->u_/*not_a_macro*/ru,
			sizeof(struct rusage));
		bcopy(&procp->utask->uu_cru,&up->u_/*not_a_macro*/cru,
			sizeof(struct rusage));
		bcopy(procp->utask->uu_timer,up->u_/*not_a_macro*/timer,
			sizeof(up->u_/*not_a_macro*/timer));
		bcopy(&procp->utask->uu_start,&up->u_/*not_a_macro*/start,
			sizeof(struct timeval));
		bcopy(&procp->utask->uu_acflag,&up->u_/*not_a_macro*/acflag,
			sizeof(struct flag_field));
		bcopy(&procp->utask->uu_/*not_a_macro*/prof,
			&up->u_/*not_a_macro*/prof, sizeof(struct uprof));
		up->u_/*not_a_macro*/prof.pr_base = procp->u_prof.pr_base;
		up->u_/*not_a_macro*/prof.pr_size = procp->u_prof.pr_size;
		up->u_/*not_a_macro*/prof.pr_off = procp->u_prof.pr_off;
		up->u_/*not_a_macro*/prof.pr_scale = procp->u_prof.pr_scale;
		up->u_/*not_a_macro*/maxuprc = procp->utask->uu_maxuprc;
		bcopy(procp->utask->uu_rlimit, up->u_/*not_a_macro*/rlimit,
			sizeof(up->u_/*not_a_macro*/rlimit));
		bcopy(&procp->thread->u_address.uthread->uu_ncache,
			&up->u_/*not_a_macro*/ncache,sizeof(struct nameicache));
		bcopy(&procp->thread->u_address.uthread->uu_nd,
			&up->u_/*not_a_macro*/nd,sizeof(struct nameicache));
		break;
#endif /* PROCFS_U */
	case PIOCGROUPS & PR_CMD_MASK:	/* get group ids */
		/* copy the group ids for the traced task from the
		 * kernel credential structure to the group id array
		 * in user space.  The number of groups to copy is
		 * determined by the value in the cr_ngroups field
		 * in the proc structure which is set when the user
		 * invokes the PIOCCRED ioctl */
		gp = (gid_t *)data;
		for (i = 0; i < procp->p_rcred->cr_ngroups;  i++) {
			gp[i] = procp->p_rcred->cr_groups[i];
		}
		break;

	case PIOCPSINFO & PR_CMD_MASK:	/* get ps info */
		/* copy the ps command information for the traced
		 * task from the corresponding kernel data structures
		 * to the prpsinfo structure in user space */

		prpsp = (struct prpsinfo *)data;

		prpsp->pr_state = procp->p_stat;

		/* set the single character value that corresponds to
		 * the task state; these are SVR4 compatible */
		if(procp->p_stat == SSLEEP) {	/* awaiting an event */
			prpsp->pr_sname = PR_SSLEEP;
		}
		else if(procp->p_stat == SRUN) {	/* running */
			prpsp->pr_sname = PR_SRUN;
		}
		else if(procp->p_stat == SIDL) {   /* intermediate state in */
			prpsp->pr_sname = PR_SIDL;   /*  process creation */
		}
		else if(procp->p_stat == SZOMB) {   /* intermediate state in */
			prpsp->pr_sname = PR_SZOMB;   /*  process termination */
			prpsp->pr_zomb = TRUE;
		}
		else if(procp->p_stat == SSTOP) {   /* task is being traced */
			prpsp->pr_sname = PR_SSTOP;
		}
		else {	/* wait state or the default */
			prpsp->pr_sname = PR_SDEFAULT;
		}
		prpsp->pr_nice = procp->p_nice;
		prpsp->pr_flag = procp->p_flag;
		if(procp->p_rcred != NULL)
		{
		    prpsp->pr_uid = procp->p_rcred->cr_uid;
		    prpsp->pr_gid = procp->p_rcred->cr_gid;
		}
		prpsp->pr_pid = procp->p_pid;
		prpsp->pr_ppid = procp->p_ppid;

		unix_master();

		if(procp->p_pgrp != NULL)
		{
		    prpsp->pr_pgrp = procp->p_pgrp->pg_id;
		    prpsp->pr_ttydev = -1;
		    if(procp->p_session != NULL)
		    {
			if(procp->p_session->s_leader != NULL)  {
				prpsp->pr_sid =
				procp->p_session->s_leader->p_pid;
			}
			if(procp->p_session->s_ttyvp != NULL)  {
				prpsp->pr_ttydev =
				procp->p_session->s_ttyvp->v_rdev;
			}
		    }
		}
		prpsp->pr_rssize = procp->p_rssize;
		if(procp->thread != NULL) {
		       (caddr_t )(procp->thread->pcb->pcb_regs[PCB_PC]);
			count = PROCFS_THREAD_STATE_COUNT;
			if((tmpstate =
			      (struct PROCFS_thread_state *)kalloc(sizeof(struct
			       PROCFS_thread_state))) == NULL) {
				unix_release();
				return(ENOMEM);
			}
			thread_getstatus(procp->thread,
				PROCFS_THREAD_STATE,
				tmpstate, &count);
			prpsp->pr_addr = (caddr_t)tmpstate->pc;
			kfree(tmpstate);
			prpsp->pr_wchan = (caddr_t )(procp->thread->wait_event);
		}

		if(procp->utask != NULL && procp->p_stat != SIDL &&
		   procp->p_stat != SZOMB) {
			prpsp->pr_size = procp->utask->uu_tsize +
				procp->utask->uu_dsize + procp->utask->uu_ssize;
			prpsp->pr_start.tv_sec = procp->utask->uu_start.tv_sec;
			prpsp->pr_start.tv_nsec
				= procp->utask->uu_start.tv_usec;
			bcopy(procp->utask->uu_comm, prpsp->pr_fname,MAXCOMLEN);
		}
		if(procp->utask != NULL && procp->task != NULL &&
		   procp->utask->uu_arg_size != NULL )
		{
			/* the size of the pr_psargs field in the prpsinfo
			 * structure is 80.  Copy either the number of
			 * characters in pr_sargs as specified in uu_arg_size if
			 * this is less than 80 characters or copy 80
			 * characters */
			map = procp->task->map;
			vm_map_reference(map);
			nb = (procp->utask->uu_arg_size >
				sizeof(prpsp->pr_psargs)) ?
			    sizeof(prpsp->pr_psargs) :
				procp->utask->uu_arg_size;

			if(vm_map_copyin(map ,procp->utask->uu_argp, nb, FALSE,
				&vmcpy) != KERN_SUCCESS) {
				vm_map_deallocate(map);
				unix_release();
				return(EIO);
			}
			vm_map_deallocate(map);
			if(vm_map_copyout(PR_KERN_MAP, &kern_adr,vmcpy)
				!= KERN_SUCCESS) {
				vm_map_copy_discard(vmcpy);
				(void)vm_deallocate(PR_KERN_MAP, kern_adr, nb);
				unix_release();
				return(EIO);
			}
			if(eblkcpy((caddr_t) kern_adr,prpsp->pr_psargs,nb)) 
				prpsp->pr_psargs[0] = '\0';
			for(chptr = prpsp->pr_psargs,i = 0; i < nb; chptr++,i++)
				if((*chptr ==NULL) && (*(chptr + 1) == NULL))
					break;
				else if(*chptr ==NULL) *chptr = ' ';
			(void)vm_deallocate(PR_KERN_MAP, kern_adr, nb);
		}
		else
		{
			prpsp->pr_psargs[0] = '\0';
		}
		if(procp->task != NULL) {
			/* The cumulative time of the task is the sum of the
			 * times the threads have run.  This information is not
			 * storeed in the task structure so it must be
			 * retrieved from each thread structure.  The time in
			 * the nanosecond part has to be normalized if it is
			 * more than one second and then converted to useconds
			 * since that is what SVR4 expects. */
			for (thread = (thread_t)
			     queue_first(&procp->task->thread_list);
			  !queue_end(&procp->task->thread_list,
			       (queue_entry_t) thread);
			    thread =
				 (thread_t) queue_next(&thread->thread_list)){
				prpsp->pr_time.tv_sec +=
					thread->user_timer.high_bits +
					thread->system_timer.high_bits;
				prpsp->pr_time.tv_nsec +=
					thread->user_timer.low_bits +
					thread->system_timer.low_bits;
				prpsp->pr_time.tv_sec += (prpsp->pr_time.tv_nsec
					/ 1000000);
				prpsp->pr_time.tv_nsec = (prpsp->pr_time.tv_nsec
					% 1000000);
			}
			thread=(thread_t)queue_first(&procp->task->thread_list);
			prpsp->pr_oldpri = (thread->depress_priority < 0) ?
                       	     thread->priority : thread->depress_priority;
			prpsp->pr_pri = ((NRQS - 1) - prpsp->pr_oldpri);

			/*
			 * Just pick up the policy for the "first" thread in
			 * the process.
			 */
			get_priocntl_clname(prpsp->pr_clname, thread->policy);
		}
		else {
                	strcpy(prpsp->pr_clname, "TS");
		}
		prpsp->pr_time.tv_nsec *= 1000;
		prpsp->pr_cpu = procp->p_cpu;
		unix_release();
		break;

	case PIOCNICE & PR_CMD_MASK:		/* set nice priority */
		/* if user is not root or the value specified by the
		 * user is negative, return errno value EPERM */
#if	SEC_BASE
		if((privileged(SEC_DEBUG, NULL)) && (*(int *)data < NULL)) {
#else
		if((cred->cr_uid == NULL) && (*(int *)data < NULL)) {
#endif
			return(EPERM);
		}

		/* add the value specified by the user to the tasks
		 * nice value */
		procp->p_nice += *(int *)data;
		break;

	case PIOCOPENM & PR_CMD_MASK:		/* get mapped object */
		/* mapping is not currently implemented in OSF
		 * and the proposed algorythm for shared libraries is
		 * changing, so return error invalid for any request but the
		 * default for now */


		if((com & IOC_DIRMASK) != IOC_VOID) {	/* no parameters */
			return(EINVAL);
		}
		else {
			extern struct fileops vnops;
			struct vnode *nvp;
			struct file *nfp;
			int indx;

			unix_master();
			/* get a file descriptor to use, return error
			 * if failure. */
			if( (error = falloc(&nfp, &indx)) ) {
				unix_release();
				return(error);
			}

			ASSERT(U_OFILE(indx, &u.u_file_state) == U_FD_RESERVED);
			ASSERT(nfp->f_count==1 && nfp->f_type==DTYPE_RESERVED);

			/* Get the vnode for the executing process, and
			 * increment its use count and its read count to
			 * make it look as if it had been opened normally.
			 */

			nvp = procp->task->procfs.pr_exvp;
			if(nvp == NULL) {
				unix_release();
				 return (EBADF);
			}
			VN_LOCK(nvp);
			VREF(nvp);
			nvp->v_rdcnt++;
			VN_UNLOCK(nvp);

			/*
			 * Now, do all the setup stuff to the file descriptor so
			 * that it looks like a "normal" open.
			 */

			FP_LOCK(nfp);
			nfp->f_flag = FREAD;
			nfp->f_type = DTYPE_VNODE;
			nfp->f_ops = &vnops;
		/* this next one IS supposed to be the vnode pointer for
		 * the executing process */
			nfp->f_data = (caddr_t)nvp;
#if     SER_COMPAT
			nfp->f_funnel = FUNNEL_NULL;
#endif
			FP_UNLOCK(nfp);
			U_FD_SET(indx, nfp, &u.u_file_state);
			*(caddr_t *)data = (caddr_t )indx;
			unix_release();
		}
		break;

	case PIOCRUN & PR_CMD_MASK:		/* make task runable */
		/* if the task is not stopped, return error busy */
		if(!(procp->task->procfs.pr_flags & (PR_ISTOP | PR_STOPPED)))
			return(EBUSY);

		/* perform the set of actions specified by the user
		 * in the prrun structure */

		if((com & IOC_DIRMASK) != IOC_VOID) {	/* parameters */
			prrunp = (struct prrun *)data;

			/* if PRSHOLD flag is set, set the set of held
			 * signals to the value in the prrun structure */
			if (prrunp->pr_flags & PRSHOLD)
			{
				/* /proc tracemask is 0 to N-1, kernel signals
				 * are 1 to N, so shift right 1 place
				 */
				prrunp->pr_sighold >>= 1;
				/* kill and stop cannot be held, so check */
				if(prrunp->pr_sighold & (sigmask(SIGKILL) |
				    sigmask(SIGSTOP)) )
					return(EINVAL);
				procp->p_sigmask = prrunp->pr_sighold;
			}

			/* if PRSTEP flag is set, set the set single step
			 * indicator in the process control block because
			 * the user wants to single step through the task
			 *
			 * Only allow single stepping for known architectures.
			 */
			if (prrunp->pr_flags & PRSTEP) {
#if defined(mips) || defined(__alpha)
			    if(((procp->task->procfs.pr_why == PR_FAULTED)
#ifdef mips
				&& (procp->task->procfs.pr_what == EXC_BREAK))
#endif /* mips */
#ifdef __alpha
				&& (procp->task->procfs.pr_what==T_IFAULT_BPT))
#endif /* __alpha */
				|| ((procp->task->procfs.pr_why == PR_SIGNALLED)
				&& (procp->task->procfs.pr_what == SIGTRAP))
				 && (procp->thread->pcb->pcb_sstep == NULL)) {
					procp->thread->pcb->pcb_sstep |=
						PT_STEP;
				}
				else
					return(EINVAL);
#else /* not mips or --alpha */
				return(EINVAL);
#endif /* mips or __alpha */
			}

			/* if PRCSIG flag is set, clear the current signal */
			if (prrunp->pr_flags & PRCSIG) {
				sig_lock_simple(procp);
				procp->p_cursig = 0; /*clear current signal*/
				thread = (thread_t)
					queue_first(&procp->task->thread_list);
				thread->u_address.uthread->uu_cursig = 0;
				SIGQ_FREE(
				    thread->u_address.uthread->uu_curinfo);
				sig_unlock(procp);
			}

			/* if PRCFAULT flag is set, clear the current fault */
			if (prrunp->pr_flags & PRCFAULT)
				procp->task->procfs.pr_curflt = -1;

			/* if PRSTRACE flag is set, set the set of traced
			 * signals to the value in the prrun structure */
			if (prrunp->pr_flags & PRSTRACE) {
				procp->task->procfs.pr_sigtrace =
					prrunp->pr_trace;
				if(procp->task->procfs.pr_sigtrace == NULL) {
					procp->task->procfs.pr_qflags &=
						~PRFS_SIGNAL;
				}
				else {
					procp->task->procfs.pr_qflags
						|= PRFS_SIGNAL;
				}
			}

			/* if PRSFAULT flag is set, set the set of traced
			 * faults to the value in the prrun structure */
			if (prrunp->pr_flags & PRSFAULT) {
				procp->task->procfs.pr_flttrace =
					prrunp->pr_fault;
				/* cannot allow trace on preempt fault */
		 		prdelset((fltset_t *)&procp->task->procfs.pr_flttrace,
				PROCFS_AST);

				tracing = FALSE;
				for (i = 0; i < FLTSET_SZ; i++)
				    if(procp->task->procfs.pr_flttrace.word[i]
					!= NULL)
					tracing= TRUE;
				if (tracing)
					procp->task->procfs.pr_qflags |=
						PRFS_FAULT;
				else
					procp->task->procfs.pr_qflags &=
						~PRFS_FAULT;
			}

			/* If PRSADDR flag is set, set the program counter
			 * register in the process control block to the value
			 * in the prrun structure.  The task will resume at
			 * the address that is put into the PC register. */
			if (prrunp->pr_flags & PRSVADDR) {
				count = PROCFS_THREAD_STATE_COUNT;
				if((tmpstate =
				      (struct PROCFS_thread_state *)kalloc(sizeof(struct
				       PROCFS_thread_state))) == NULL) {
					return(ENOMEM);
				}
				thread_getstatus(procp->thread,
					PROCFS_THREAD_STATE,
					tmpstate, &count);
				tmpstate->pc =
					(long )prrunp->pr_vaddr;
				thread_setstatus(procp->thread,
					PROCFS_THREAD_STATE,
					tmpstate, &count);
				kfree(tmpstate);
			}

			/* abort the system call if PRSABORT flag is set and
			 * the system call is traced on entry or the process is
			 * in an interruptible sleep - PR_ASLEEP */
			if(prrunp->pr_flags & PRSABORT) {
			    if(procp->task->procfs.pr_why & PR_SYSENTRY)
				procp->task->procfs.pr_qflags |= PRFS_ABORT;
			    else if( ((procp->thread->interruptible) &&
			    (procp->thread->state & TH_WAIT)))
				wait_event = procp->thread->wait_event;
			}
			/* Note: PRSTOP must be checked last */
			/* if PRSTOP flag is set, set the PR_DSTOP flag in
			 * the pr_flags field in the procfs structure which
			 * directs the task to stop */
			if (prrunp->pr_flags & PRSTOP)
			{
			    /* if the task is stopped on syscall entry, or
			     * stopped on a fault and PRCFAULT had not been
			     * specified, then set PR_DSTOP, otherwise
			     * change pr_why to PR_REQUESTED and skip the
			     * task_resume().  Skipping task_resume() is done
			     * if we are stopped because of a PIOCSTOP,
			     * syscall exit trace, signal trace, or fault
			     * trace - with PRCFAULT specified.
			     */
			    if( (procp->task->procfs.pr_why == PR_SYSENTRY) ||
			    ((procp->task->procfs.pr_why == PR_FAULTED) &&
			    (procp->task->procfs.pr_curflt != -1) ) )
			    {
				procp->task->procfs.pr_flags |= PR_DSTOP;
			    }
			    else
			    {
				procp->task->procfs.pr_why = PR_REQUESTED;
				break;
			    }
			}
		}

		/* reset the /proc stop flags and the /proc reasons for
		 * stopping in the pr_why and pr_what fields of the
		 * procfs structure */
		procp->task->procfs.pr_flags &=
			~(PR_STOPPED | PR_ISTOP | PR_PCINVAL);
		procp->task->procfs.pr_why = NULL;
		procp->task->procfs.pr_what = 0L;

		/* make the task runnable by resuming the task */
		task_resume(procp->task);
		if(wait_event)
		    wakeup(wait_event);
		break;

	case PIOCSTOP & PR_CMD_MASK:		/* stop a task */
		/* if this is a system task, return error busy */
		if (procp->task->procfs.pr_flags & PR_ISSYS)
			return(EBUSY);

	    /* If the task is already stopped on an event of interest, just
	     * fall through to PIOCWSTOP - which will fall through to PIOCSTATUS
	     */
	    task_lock(procp->task);
	    if(!(procp->task->procfs.pr_flags & PR_ISTOP) )
	    {
		task_unlock(procp->task);

		/* Stop the victim by calling task_suspend(). When task_suspend
		 * returns, the victim task is indeed stopped, so set procfs
		 * status to indicate that.  Note, the fall through to
		 * PIOCWSTOP merely falls through to PIOCSTATUS.
		 */
		if(task_suspend(procp->task) != KERN_SUCCESS)
		    return(EAGAIN);
		if(procp->task == NULL)
			return(ENOENT);

		/* If the task->user_stop_count is > 1, it means someone else
		 * did a task suspend type of operation.  Therefore, wait
		 * until the user_stop_count goes to 1 (i.e. we are the one
		 * that is currently causing the task to be stopped) before
		 * setting status to PR_ISTOP.  If the task dies, return
		 * error ENOENT; if the user interrupts the PIOCSTOP, do
		 * a task_resume and return EINTR.
		 */
		task_lock(procp->task);
		if(procp->task->user_stop_count > 1)
		{
		    while ((procp->task != NULL) &&
		    (procp->task->user_stop_count > 1 ) &&
		    (procp->p_stat != SZOMB) && (procp->p_stat != NULL)) {
			task_unlock(procp->task);
			if( ((error = tsleep(&procp->task->procfs,
			(PZERO+1) | PCATCH,"user_stop_count",100))) == EINTR)
			{
				(void)task_resume(procp->task);
				return(error);
			}
			if(procp->task != NULL)
				task_lock(procp->task);
		    }
		    if(procp->task == NULL)
			return(ENOENT);
		}

		/* If we get here, procp->task is still locked. */
		procp->task->procfs.pr_flags |= (PR_STOPPED|PR_ISTOP);
		procp->task->procfs.pr_flags &= ~(PR_PCINVAL|PR_DSTOP);
		procp->task->procfs.pr_why = PR_REQUESTED;
		procp->task->procfs.pr_what = NULL;
		procp->task->procfs.pr_tid = procp->thread;
	    }
	    task_unlock(procp->task);

		/* NO break, fall through! */

	case PIOCWSTOP & PR_CMD_MASK:		/* stop a task with wait*/
		/* if this is a system task, return error busy */
		if (procp->task->procfs.pr_flags & PR_ISSYS)
			return(EBUSY);

		/* wait until the task stops on an event of interest */
		task_lock(procp->task);
		while ((procp->task != NULL) &&
			!(procp->task->procfs.pr_flags & PR_ISTOP) &&
			(procp->p_stat != SZOMB) && (procp->p_stat != NULL)) {
			task_unlock(procp->task);
			if( ((error = tsleep(&procp->task->procfs,
			 (PZERO+1) | PCATCH,"request task stop",100))) == EINTR)
				return(error);
			if(procp->task != NULL)
				task_lock(procp->task);
		}

		/* break; NO! fall through! */
		if (((com & IOC_DIRMASK) == IOC_VOID) ||    /* no parameters */
		   (procp->p_stat == NULL)) {
			if(procp->task != NULL)
				task_unlock(procp->task);
			break;  /* no status struct passed in */
		}
		if(procp->task != NULL)
			task_unlock(procp->task);

	case PIOCSTATUS & PR_CMD_MASK:	/* get task status */
		/* copy the current status information for the task from
		 * the appropriate kernel data structures to the
		 * prstatus structure in user space */
		if((com & IOC_DIRMASK) == IOC_VOID)	/* no parameters */
			break;

		prstatp = (struct prstatus *)data;
		unix_master();
		if((procp->p_stat == NULL) || (PROCFS_VNINVALID(pn)) ||
		   (procp->task == NULL)) {
			prstatp->pr_why = PR_DEAD;
			prstatp->pr_what = pn->prc_exitval;
			prstatp->pr_pid = pn->prc_pid;
			unix_release();
			break;
		}

		prstatp->pr_flags = procp->task->procfs.pr_flags;

		if((procp->thread->interruptible) &&
		 (procp->thread->state & TH_WAIT))
			prstatp->pr_flags |= PR_ASLEEP;

		prstatp->pr_why = procp->task->procfs.pr_why;
		prstatp->pr_what = procp->task->procfs.pr_what;
		prstatp->pr_subcode = procp->task->procfs.pr_subcode;

		/* copy in vp to disk file from which process was exec()ed */
		prstatp->pr_exvp = procp->task->procfs.pr_exvp;

		if (procp->task->procfs.pr_why == PR_SIGNALLED) {
			sig_lock_simple(procp);
			thread = (thread_t)
				queue_first(&procp->task->thread_list);
			if (thread->u_address.uthread->uu_curinfo)
				bcopy(&thread->u_address.uthread->
				      uu_curinfo->siginfo,
				      &prstatp->pr_info,
				      sizeof(k_siginfo_t)); 
			else
				bzero(&prstatp->pr_info, 
				      sizeof(struct siginfo));
			sig_unlock(procp);
		}

		prstatp->pr_cursig = procp->p_cursig;
		prstatp->pr_sigpend = procp->p_sig;
		/* convert kernel signal mask to /proc trace mask */
		prstatp->pr_sighold = (procp->p_sigmask << 1);

		if( procp->utask != NULL && procp->p_stat != SIDL &&
		    procp->p_stat != SZOMB)
		{
			prstatp->pr_altstack = procp->utask->uu_sigstack;
			if (procp->p_cursig != NULL) {
		            if(thread_signal_disposition(procp->p_cursig) )
		            {
				if(procp->thread == NULL) {
				    unix_release();
				    return(EINVAL);
				}
				prstatp->pr_action.sa_handler =
				procp->thread->u_address.uthread->uu_tsignal[procp->p_cursig];
			    }
			    else
			    {
				prstatp->pr_action.sa_handler =
				procp->utask->uu_signal[procp->p_cursig];
			    }
				prstatp->pr_action.sa_mask =
				procp->utask->uu_sigmask[procp->p_cursig];
				prstatp->pr_action.sa_flags |=
				procp->utask->uu_sigonstack &
				sigmask(procp->p_cursig) ? SA_ONSTACK : NULL;
			}
		}

		prstatp->pr_pid = procp->p_pid;
		prstatp->pr_ppid = procp->p_ppid;
		prstatp->pr_pgrp = procp->p_pgrp->pg_id;
		if(procp->p_session->s_leader != NULL)
			prstatp->pr_sid = procp->p_session->s_leader->p_pid;

		for (thread = (thread_t) queue_first(&procp->task->thread_list);
		  !queue_end(&procp->task->thread_list, (queue_entry_t) thread);
		    thread = (thread_t) queue_next(&thread->thread_list)){
		  /* Sum up the per thread system time */
			prstatp->pr_stime.tv_sec +=
				thread->system_timer.high_bits;
			prstatp->pr_stime.tv_nsec +=
				thread->system_timer.low_bits;
			prstatp->pr_stime.tv_sec += (prstatp->pr_stime.tv_nsec
				/ 1000000);
			prstatp->pr_stime.tv_nsec = (prstatp->pr_stime.tv_nsec
				% 1000000);
		  /* Sum up the per thread user time */
			prstatp->pr_utime.tv_sec +=
				thread->user_timer.high_bits;
			prstatp->pr_utime.tv_nsec +=
				thread->user_timer.low_bits;
			prstatp->pr_utime.tv_sec += (prstatp->pr_utime.tv_nsec
				/ 1000000);
			prstatp->pr_utime.tv_nsec = (prstatp->pr_utime.tv_nsec
				% 1000000);
		}
		prstatp->pr_utime.tv_nsec *= 1000;
		prstatp->pr_stime.tv_nsec *= 1000;

		prstatp->pr_cutime.tv_sec = 0L;
		prstatp->pr_cutime.tv_nsec = 0L;

		/* compute the sum of the children's user times */
		sum_cutime(procp,&prstatp->pr_cutime);

		prstatp->pr_cstime.tv_sec = 0L;
		prstatp->pr_cstime.tv_nsec = 0L;

		/* compute the sum of the children's system times */
		sum_cstime(procp,&prstatp->pr_cstime);

                /*
                 * Just pick up the policy for the "first" thread in
                 * the process.
                 */
                thread = (thread_t) queue_first(&procp->task->thread_list);
		if( !queue_end(&procp->task->thread_list, 
		  (queue_entry_t) thread) ) {
                	get_priocntl_clname(prstatp->pr_clname, thread->policy);
		} else {
                	strcpy(prstatp->pr_clname, "TS");
		}

		/* Get the user state (registers) */
		count = PROCFS_THREAD_STATE_COUNT;
		regp = (struct PROCFS_thread_state *)&prstatp->pr_reg;
		thread_getstatus(procp->thread, PROCFS_THREAD_STATE,
			regp, &count);

		/* if the program counter is invalid, set the pr_instr
		 * field in the prstatus structure to zero; else set it
		 * to the current value in the PC register in the
		 * process control block */
		if (procp->task->procfs.pr_flags & PR_PCINVAL)
			prstatp->pr_instr = 0L;
		else {
			/* prstatp->pr_instr = prstatp->pr_reg[PCB_PC]; */
			prstatp->pr_instr = 0L;
			map = procp->task->map;
			vm_map_reference(map);
			nb = sizeof(long);
			if(vm_map_copyin(map, (regp->pc - PR_PC_OFF), nb,
				FALSE, &vmcpy) != KERN_SUCCESS) {
				vm_map_deallocate(map);
				prstatp->pr_instr = 0L;
				break;
			}
			vm_map_deallocate(map);
			if(vm_map_copyout(PR_KERN_MAP, &kern_adr,vmcpy)
				!= KERN_SUCCESS) {
				vm_map_copy_discard(vmcpy);
				(void)vm_deallocate(PR_KERN_MAP, kern_adr, nb);
				prstatp->pr_instr = 0L;
				unix_release();
				break;
			}
            		if(eblkcpy((caddr_t) kern_adr,&prstatp->pr_instr,nb)) {
				prstatp->pr_flags &= PR_PCINVAL;
				prstatp->pr_instr = 0L;
			}
			(void)vm_deallocate(PR_KERN_MAP, kern_adr, nb);
		}
		unix_release();
		break;

	case PIOCGTRACE & PR_CMD_MASK:	/* get traced signals */
		/* get signals traced for the traced task
		 * copy the set of signals to be traced from the procfs
		 * structure to user space */
		bcopy(&procp->task->procfs.pr_sigtrace, (sigset_t *)data,
			sizeof(sigset_t));
		break;

	case PIOCSTRACE & PR_CMD_MASK:	/* set traced signals */
		/* set signals traced for the traced task
		 * copy the set of signals to be traced from user space
		 * into set of signals traced in the procfs structure */
		procp->task->procfs.pr_sigtrace = *(sigset_t *)data;
		if(procp->task->procfs.pr_sigtrace == NULL) {
			procp->task->procfs.pr_qflags &= ~PRFS_SIGNAL;
		}
		else {
			procp->task->procfs.pr_qflags |= PRFS_SIGNAL;
		}
		break;

	case PIOCSSIG & PR_CMD_MASK:		/* set sig to siginfo */
		/*
		 * If the user specifies a zero as the third argument
		 * of the ioctl call, or if the signal specified in the
		 * siginfo structure is NUll, clear the current signal; else
		 * copy the siginfo structure from user space to the
		 * kernel corresponding siginfo structure for the task.
		 */
		if (*(int *)data == NULL ||
		   ((struct siginfo *)data)->si_signo == NULL ) {
			sig_lock_simple(procp);
			thread = (thread_t)
				queue_first(&procp->task->thread_list);
			procp->p_cursig = 0;	/* clear current signal */
			thread->u_address.uthread->uu_cursig = 0;
			sig_unlock(procp);
			SIGQ_FREE(thread->u_address.uthread->uu_curinfo);
		}
		else {
			int sig;
			/* return EINVAL if signal number not a real signal */
			sig = ((struct siginfo *)data)->si_signo;
			if( sig < 0 || sig >= NSIG)
				return(EINVAL);

			/*
			 * Frist, check if the action for the signal to be
			 * forced is SIG_IGN.  If so, just return.
			 */
			sig_lock_simple(procp);
			thread =
				(thread_t) queue_first(&procp->task->thread_list);
			if( thread_signal_disposition(sig) )
			{
			    if(*(thread->u_address.uthread->uu_tsignal+sig) ==
				SIG_IGN)
				break;
			}
			else
			{
			    if(*(procp->utask->uu_signal+sig) == SIG_IGN)
				break;
			}

			/* reset current signal */
			if (!thread->u_address.uthread->uu_curinfo) {
				sigqueue = SIGQ_ALLOC();
				if (sigqueue)
					sigqueue->siginfo = zero_ksiginfo;
				thread->u_address.uthread->uu_curinfo
					= sigqueue;
			}
			if (thread->u_address.uthread->uu_curinfo)
				bcopy((struct siginfo *)data,
				      &thread->u_address.uthread->
				      uu_curinfo->siginfo,
				      sizeof(k_siginfo_t));
			procp->p_cursig = sig;
			procp->task->procfs.pr_ssig = sig;
			if( thread_signal_disposition(sig) )
				thread->u_address.uthread->uu_sig = sig;
			sig_unlock(procp);

			 /* if the signal is a kill, reset the /proc stop
			  * flags and the /proc reasons for stopping in the
			  * pr_why and pr_what fields of the procfs structure */
			 if (procp->p_cursig == SIGKILL) {
				 procp->task->procfs.pr_flags &= ~(PR_STOPPED | PR_ISTOP);
				 procp->task->procfs.pr_why = NULL;
				 procp->task->procfs.pr_what = 0L;

				 /* set the task status to run */
				 procp->p_stat = SRUN;

				 /* make the task runnable by resuming the task */
				 task_resume(procp->task);
			 }
		 }
		 break;

	case PIOCKILL & PR_CMD_MASK:		/* send sig to proc */
		/* If the signal number is not valid, return EINVAL */
		if(*(int *)data <= 0 || *(int *)data >= NSIG)
			return(EINVAL);

		/* send the signal specified by the user to the traced
		 * task with the same semantics as kill */

		/* set up the arguments for the kill routine and call
		 * the kill routine to send the signal */
		uap.pid = procp->p_pid;
#ifdef __alpha
		uap.signo = *(long *)data;
#else
		uap.signo = *(int *)data;
#endif
		kill(cp, &uap, &retval);

		/* if the process is stopped, make it runnable */
		if( uap.signo == SIGKILL &&
		 procp->task->procfs.pr_flags & (PR_STOPPED | PR_ISTOP)) {
			/* reset the /proc stop flags and the /proc
			 * reasons for stopping in the pr_why and
			 * pr_what fields of the procfs structure */
			procp->task->procfs.pr_flags &= ~(PR_STOPPED | PR_ISTOP);
			procp->task->procfs.pr_why = NULL;
			procp->task->procfs.pr_what = 0L;

			/* set the task status to run */
			procp->p_stat = SRUN;

			/* make the task runnable by resuming the task */
			task_resume(procp->task);
		}
		break;

	case PIOCUNKILL & PR_CMD_MASK:	/* delete pending sig */
		/* Can't undo a kill or stop */
		if (*(int *)data == SIGKILL)
			return (EINVAL);

		/* reset the pending signal using the signal number
		 * specified by the user; the sigmask macro converts
		 * the signal number into a signal mask */
		s = splhigh();
		procp->p_sig &= ~(sigmask(*(int *)data));
		(void)sigq_remove_all((&procp->p_sigqueue), *(int *)data);
		if( thread_signal_disposition(*(int *)data) )
		{
		   thread = (thread_t) queue_first(&procp->task->thread_list);

		   (void)sigq_remove_all(&thread->u_address.uthread->uu_sigqueue,
		    *(int *)data);
		   (void)sigq_remove_all((&procp->p_sigqueue), *(int *)data);
		}
		splx(s);
		break;

	case PIOCGHOLD & PR_CMD_MASK:		/* get held signals */
		/* get held signals for the traced task
		 * copy the set of held signals from the procfs
		 * structure to user space
		 * NOTE: see comment in PIOCSHOLD on holdsigs.
		 */
		holdsigs = procp->p_sigmask;
		holdsigs <<= 1;
		bcopy(&holdsigs, (sigset_t *)data, sizeof(procp->p_sigmask));
		break;

	case PIOCSHOLD & PR_CMD_MASK:		/* set held signals */
		/* get signal mask into "kernel" form - the kernel sees
		 * signals 1 to N, the /proc tracemask macros work with
		 * 0 to N-1, therefore, we shift right 1.
		 * NOTE, this only works because sigset_t is a long, if it
		 * is ever changed to an array for MIPS, this will have to
		 * be ifdef-ed.
		 */
		holdsigs = *(sigset_t*)data;
		holdsigs >>= 1;

		/* Can't hold a kill or stop */
		if (holdsigs & (sigmask(SIGKILL) | sigmask(SIGSTOP)))
			return (EINVAL);

		/* set held signals for the traced task
		 * copy the set of held signals from user space
		 * to set of held signals in the procfs structure */
		procp->p_sigmask = holdsigs;
		break;

	case PIOCCFAULT & PR_CMD_MASK:	/* clear current flt */
		/* clear the current fault by resetting the current
		 * fault, and the pr_why and pr_what fields in the
		 * procfs structure */
		procp->task->procfs.pr_curflt = -1;
		break;

	case PIOCGFAULT & PR_CMD_MASK:	/* get traced faults */
		/* get faults traced for the traced task
		 * copy the set of faults to be traced from the procfs
		 * structure to user space */
		bcopy(&procp->task->procfs.pr_flttrace, (fltset_t *)data,
			sizeof(fltset_t)); 
		break;
	case PIOCSFAULT & PR_CMD_MASK:	/* set traced faults */
		/* set faults traced for the traced task
		 * copy the set of faults to be traced from user space
		 * to set of faults traced in the procfs structure */
		procp->task->procfs.pr_flttrace = *(fltset_t *)data;
		/* cannot allow trace on preempt fault */
		prdelset((fltset_t *)&procp->task->procfs.pr_flttrace, PROCFS_AST);

		/* if any faults are being traced, set a flag in
		 * the pr_qflags field in the procfs structure; else
		 * reset the flag.  Process control uses the PRFS_FAULT
		 * flag as a quick check to see if faults are being
		 * traced on entry; this is an optimization for faults */
		tracing = FALSE;
		for (i = 0; i < FLTSET_SZ; i++)
			if (procp->task->procfs.pr_flttrace.word[i] != NULL)
				tracing= TRUE;
		if (tracing)
			procp->task->procfs.pr_qflags |= PRFS_FAULT;
		else
			procp->task->procfs.pr_qflags &= ~PRFS_FAULT;
		break;

	case PIOCGENTRY & PR_CMD_MASK:	/* get traced syscall entry */
		/* get system calls traced on entry for the traced task
		 * copy the set of system calls to be traced on entry
		 * from the set of system calls traced on entry in the
		 * procfs structure to user space */
		bcopy(&procp->task->procfs.pr_sysenter, (sysset_t *)data,
			sizeof(sysset_t)); 
		break;

	case PIOCSENTRY & PR_CMD_MASK:	/* set traced syscall entry */
		/* set system calls traced on entry for the traced task
		 * copy the set of system calls to be traced on entry
		 * from user space to the set of system calls traced on
		 * entry in the procfs structure */
		procp->task->procfs.pr_sysenter = *(sysset_t *)data;

		/* if any system calls are being traced, set a flag in
		 * the pr_qflags field in the procfs structure; else
		 * reset the flag.  Process control uses the PRFS_SCENTER
		 * flag as a quick check to see if system calls are being
		 * traced on entry; this is an optimization for system calls */
		tracing = FALSE;
		for (i = 0; i < SYSSET_SZ; i++)
			if (procp->task->procfs.pr_sysenter.word[i] != NULL)
				tracing= TRUE;
		if (tracing)
			procp->task->procfs.pr_qflags |= PRFS_SCENTER;
		else
			procp->task->procfs.pr_qflags &= ~PRFS_SCENTER;
		break;

	case PIOCGEXIT & PR_CMD_MASK:		/* get traced syscall exit */
		/* get system calls traced on exit for the traced task
		 * copy the set of system calls to be traced on exit
		 * from the set of system calls traced on exit in the
		 * procfs structure to user space */
		bcopy(&procp->task->procfs.pr_sysexit, (sysset_t *)data,
			sizeof(sysset_t));
		break;

	case PIOCSEXIT & PR_CMD_MASK:		/* set traced syscall exit */
		/* set system calls traced on exit for the traced task
		 * copy the set of system calls to be traced from user
		 * space to set of system calls traced on exit in
		 * the procfs structure */
		procp->task->procfs.pr_sysexit = *(sysset_t *)data;

		/* if any system calls are being traced, set a flag in
		 * the pr_qflags field in the procfs structure; else
		 * reset the flag.  Process control uses the PRFS_SCEXIT
		 * flag as a quick check to see if system calls are being
		 * traced on exit; this is an optimization for system calls */
		tracing = FALSE;
		for (i = 0; i < SYSSET_SZ; i++)
			if (procp->task->procfs.pr_sysexit.word[i] != NULL) 
				tracing= TRUE;
		if (tracing)
			procp->task->procfs.pr_qflags |= PRFS_SCEXIT;
		else
			procp->task->procfs.pr_qflags &= ~PRFS_SCEXIT;
		break;

	case PIOCRFORK & PR_CMD_MASK:		/* reset (turn off) inherit-on-fork */
		/* reset inherit-on-fork flag for the traced task */
		procp->task->procfs.pr_flags &= ~PR_FORK;
		break;

	case PIOCSFORK & PR_CMD_MASK:		/* set inherit-on-fork */
		/* set inherit-on-fork flag for the traced task */
		procp->task->procfs.pr_flags |= PR_FORK;
		break;

	case PIOCRRLC & PR_CMD_MASK:		/* reset (turn off) run-on-last-close */
		/* reset run-on-last-close flag for the traced task */
		procp->task->procfs.pr_flags &= ~PR_RLC;
		break;

	case PIOCSRLC & PR_CMD_MASK:		/* set run-on-last-close */
		/* set run-on-last-close flag for the traced task */
		procp->task->procfs.pr_flags |= PR_RLC;
		break;

	case PIOCGFPREG & PR_CMD_MASK:	/* get floating point regs */
		/* copy the set of floating point registers from the
		process control block of the traced task to user
		space.  The pcb is in the victim's process context, so we
		have to map it in.*/

		/* If the primary thread is swapped out, return EAGAIN. */
		thread = (thread_t) queue_first(&procp->task->thread_list);
		if(thread->swap_state != TH_SW_IN)
			return(EAGAIN);

		/* if the primary thread did not use the FP processor, just
		 * return 0's - return the NULL filled buffer.
		 */
		if (thread->pcb->pcb_ownedfp == NULL) {
			bzero( (char *)data, PROCFS_FLOAT_STATE_COUNT );
			break;
		}

		/* If the process is not stopped, stop it and remember that
		 * we did so it can be restarted after the FP regs have been
		 * copied.
		 */
		fp_stop = NULL;
		if(procp->task != current_task() &&
		 !(procp->task->procfs.pr_flags & PR_ISTOP)) {
		    if(task_suspend(procp->task) != KERN_SUCCESS)
			return(EINVAL);
		    fp_stop = 1;
		}
#ifdef __alpha
		/* copy the FP regs from the thread pcb to the ioctl buffer */
		w_regp = (long*)data;
		for(i= 0; i < PROCFS_FLOAT_STATE_COUNT; i++)
			*w_regp++ = thread->pcb->pcb_fpregs[i];
#else
		count = PROCFS_FLOAT_STATE_COUNT;
		if( thread_getstatus(procp->thread, PROCFS_FLOAT_STATE,
			(long *)data, &count) != KERN_SUCCESS ) {
			if(fp_stop == 1)
			    (void)task_resume(procp->task);
			return(EINVAL);
		}
#endif

		/* if we stopped the victim task, restart it. */
		if(fp_stop == 1)
			(void)task_resume(procp->task);
		break;

	case PIOCSFPREG & PR_CMD_MASK:	/* set floating point regs */
		/* if the task is not stopped, return error busy */
		if ( !(procp->task->procfs.pr_flags & PR_ISTOP) )
			return(EBUSY);

		/* copy the set of floating point registers from user
		 * space to the process control block of the traced
		 * task */
#ifdef __alpha
		/* copy the FP regs from the ioctl buffer to the thread pcb */
		w_regp = (long*)data;
		thread = (thread_t) queue_first(&procp->task->thread_list);
		for(i= 0; i < PROCFS_FLOAT_STATE_COUNT; i++)
			thread->pcb->pcb_fpregs[i] = *w_regp++;
#else
		count = PROCFS_FLOAT_STATE_COUNT;
		if( thread_setstatus(procp->thread, PROCFS_FLOAT_STATE,
			(long *)data, &count) != KERN_SUCCESS )
			return(EINVAL);
#endif
		break;

	case PIOCGREG & PR_CMD_MASK:		/* get general regs */
		/* copy the set of general registers from the process
		 * control block of the traced task to user space. */

		count = PROCFS_THREAD_STATE_COUNT;
		thread_getstatus(procp->thread, PROCFS_THREAD_STATE,
			(long *)data, &count);
		break;

	case PIOCSREG & PR_CMD_MASK:		/* set general regs
		/* if the task is not stopped, return error busy */
		if ( !(procp->task->procfs.pr_flags & PR_ISTOP) )
			return(EBUSY);

		/* copy the set of general registers from user space to
		 * the process control block of the traced thread
		 */

		regp = (struct PROCFS_thread_state *)data;
		thread_setstatus(procp->thread, PROCFS_THREAD_STATE, regp,
			&count);
		break;

	case PIOCMAXSIG & PR_CMD_MASK:	/* get max signals */
		/* copy the maximum number of signals to user space */
		*(int *)data = NSIG;
		break;

	case PIOCACTION & PR_CMD_MASK:	/* get sig actions */
		/* copy the signal action structures for the task from
		the kernel to the array of action structures in user
		space */

		if(procp->utask == NULL)
			return(EINVAL);
		ap = (struct sigaction *)data;
		for (i = 1; i < NSIG; i++, ap++) {
		    if(thread_signal_disposition(i) )
		    {
			if(procp->thread == NULL)
				return(EINVAL);
			ap->sa_handler =
			procp->thread->u_address.uthread->uu_tsignal[i];
		    }
		    else
		    {
			ap->sa_handler = procp->utask->uu_signal[i];
		    }
			ap->sa_mask = procp->utask->uu_sigmask[i];
			ap->sa_flags |= procp->utask->uu_sigonstack & sigmask(i)
				? SA_ONSTACK : NULL;
		}
		
		break;

	case PIOCNMAP & PR_CMD_MASK:		/* get # mem mappings */
		/* copy the number of memory mappings specified in the
		 * nentries field of the map structure into user space */
		*(int *)data = procp->task->map->vm_nentries;

		/* the pr_nmaps field in the procfs structure is set
		 * to the nentries field in the map structure for a
		 * subsequent PIOCMAP ioctl that the user may invoke */
		procp->task->procfs.pr_nmap = procp->task->map->vm_nentries + 1;
		break;

	case PIOCMAP & PR_CMD_MASK:		/* get mem mappings */
		/* copy the map information for each memory mapping from
		 * the appropriate map structures in the kernel to the
		 * array of prmap structures in user space */
		mp = (struct prmap *)data;

		/* the number of memory map structures to copy is
		 * determined by the value in the pr_nmap field in the
		 * procfs structure which is set when the user invokes
		 * the PIOCNMAP ioctl */
		unix_master();
		map = procp->task->map;
		vm_map_reference(map);
		map_entry = vm_map_first_entry(map);
		for (i = 0; i < (procp->task->procfs.pr_nmap - 1); i++, mp++) {
			mp->pr_vaddr = (caddr_t)(map_entry->vme_start);

			if (vm_object_type(map_entry->vme_object) == OT_SEG) {
				mp->pr_size = map_entry->vme_seg->seg_size;
				mp->pr_off = (off_t )map_entry->vme_seg->seg_offset;
			}
			else {
				mp->pr_size = map_entry->vme_end - map_entry->vme_start;
				mp->pr_off = (off_t )map_entry->vme_offset;
			}

			mflags = map_entry->vme_protection;
			mp->pr_mflags = NULL;
			if (mflags & VM_PROT_READ)
				mp->pr_mflags |= MA_READ;
			if (mflags & VM_PROT_WRITE)
				mp->pr_mflags |= MA_WRITE;
			if (mflags & VM_PROT_EXECUTE)
				mp->pr_mflags |= MA_EXEC;
			/* MA_SHARED, MA_BREAK, MA_STACK not in OSF */
			map_entry = map_entry->vme_next;
		}
		vm_map_deallocate(map);

		/* add a NULL entry at the end of the list */
		bzero( mp, sizeof(struct prmap) );

		/* zero the number of memory mappings in the pr_nmap
		 * field in the procfs structure */
		procp->task->procfs.pr_nmap = NULL;
		unix_release();
		break;

	case PIOCGSPCACT & PR_CMD_MASK:          /* Get the special action flags */
		/* get the special action flags for the traced task */
		spcactptr = (int *)data;
		*spcactptr = procp->task->procfs.pr_qflags & PRFS_SPCACTMASK;
		break;

	case PIOCSSPCACT & PR_CMD_MASK:          /* set (turn on) special action flags */
		/* set special action flags for the traced task */
		spcactptr = (int *)data;

		procp->task->procfs.pr_qflags &= ~PRFS_SPCACTMASK;
		procp->task->procfs.pr_qflags |= (*spcactptr & PRFS_SPCACTMASK);

		/* set/clear stop-on-exec in the proc p_pr_qflags field */
		procp->p_pr_qflags &= ~PRFS_STOPEXEC;
		procp->p_pr_qflags |= (*spcactptr & PRFS_STOPEXEC);

		break;

	case PIOCNTHR & PR_CMD_MASK:		/* get thread ids */
		/* copy the number of threads specified in the task
		   into user space */
		if((procp->task->procfs.pr_flags & PR_STOPPED))
			return(EBADF);
		*(int *)data = procp->task->thread_count;
		break;

	case PIOCTLIST & PR_CMD_MASK:		/* get thread ids */
		if((procp->task->procfs.pr_flags & PR_STOPPED))
			return(EBADF);
		threadp = (tid_t *)data;
		for(i = 0, thread =
			(thread_t) queue_first(&procp->task->thread_list);
			!queue_end(&procp->task->thread_list,
				(queue_entry_t) thread);
			thread = (thread_t)queue_next(&thread->thread_list),
				i++) {
			threadp[i] = thread;
		}
		break;

#ifdef PROCFS_U
	case PIOCTABRUN & PR_CMD_MASK:
		prthsp = (prthreads_t *)data;
		count = prthsp->pr_count;
		abdata = (prabrun_t *)prthsp->pr_data;
		threadp = abdata->pr_tid;
		if(abdata->pr_flags == -1)
			abdata->pr_run.pr_flags = abdata->pr_flags;
		for (thread = (thread_t) queue_first(&procp->task->thread_list);
		  !queue_end(&procp->task->thread_list, (queue_entry_t) thread);
		    thread = (thread_t) queue_next(&thread->thread_list)){
			runit = TRUE;
			for (i = 0; i < last; i++) {
				if(threadp[i] == thread) {
					runit = FALSE;
					break;
			}
		}
		if(runit)
			if((retval = procfs_tioctl(procp, cp, thread, com,
				abdata->pr_run, cred)) != NULL)
				return((long )thread);
		}
		break;

	case PIOCTRUN & PR_CMD_MASK:
	case PIOCTGETTH & PR_CMD_MASK:
	case PIOCTGETUTH & PR_CMD_MASK:
	case PIOCTSTOP & PR_CMD_MASK:
	case PIOCTSTATUS & PR_CMD_MASK:
	case PIOCTGTRACE & PR_CMD_MASK:
	case PIOCTSTRACE & PR_CMD_MASK:
	case PIOCTSSIG & PR_CMD_MASK:
	case PIOCTKILL & PR_CMD_MASK:
	case PIOCTUNKILL & PR_CMD_MASK:
	case PIOCTCFAULT & PR_CMD_MASK:
	case PIOCTGFAULT & PR_CMD_MASK:
	case PIOCTSFAULT & PR_CMD_MASK:
	case PIOCTGFPREG & PR_CMD_MASK:
	case PIOCTSFPREG & PR_CMD_MASK:
	case PIOCTGREG & PR_CMD_MASK:
	case PIOCTSREG & PR_CMD_MASK:
		nb = IOCPARM_LEN(com);
		prthsp = (prthreads_t *)data;
		count = prthsp->pr_count;
		if(count > procp->task->thread_count)
			count = procp->task->thread_count;
		tdata = prthsp->pr_data;
		for (i = 0; i < count; i++, tdata = tdata + nb) {
			thread = *(tid_t *)(tdata + (nb - sizeof(tid_t)));
			if((retval = procfs_tioctl(procp, cp, thread, com,
				tdata, cred)) != NULL)
				return(retval);
		}
		break;

#endif /* PROCFS_U */
	default:
		/* illegal ioctl command */
		return(EINVAL);
		break;
	}
	return(retval);
}

#ifdef PROCFS_U
/* Special processing for the thread related ioctls */
procfs_tioctl(procp, cp, thread, com, data, cred)
struct proc *procp, *cp;
tid_t thread;
int com;
caddr_t data;
struct ucred *cred;
{
	register int i, nb, tracing;
	int count, error, retval = NULL;
	struct prrun *prrunp;
	struct prstatus *prstatp;
	struct args {
			 int	pid;
			 int	signo;
	} uap;
	struct sigaction *ap;
	struct sigaction action;
	thread_t ckth;
	struct PROCFS_thread_state *tmpstate, *regp;
	vm_map_t map;
	vm_map_copy_t vmcpy;
	vm_map_entry_t map_entry;
	vm_offset_t kern_adr;

	tracing = FALSE;
	for (ckth = (thread_t) queue_first(&procp->task->thread_list);
	  !queue_end(&procp->task->thread_list, (queue_entry_t) ckth);
	    ckth = (thread_t) queue_next(&ckth->thread_list)){
		if(thread == ckth) {
			tracing = TRUE;
			break;
		}
	}
	if(!tracing) {
		return(EBADF);
	}

	switch(com & PR_CMD_MASK) {
	case PIOCTRUN & PR_CMD_MASK:		/* make thread runable */
	case PIOCTABRUN & PR_CMD_MASK:		/* make thread runable */
		/* if the thread is not stopped, return error busy */
		if(!(thread->t_procfs.pr_flags & (PR_ISTOP | PR_STOPPED)))
			return(EBUSY);

		/* perform the set of actions specified by the user
		 * in the prrun structure, if any */

		/* if PRCSIG flag is set, clear the current signal */
		prrunp = (struct prrun *)data;
		if (prrunp->pr_flags != -1) {

			/* if PRSHOLD flag is set, set the set of held
			 * signals to the value in the prrun structure */
			if (prrunp->pr_flags & PRSHOLD)
			{
				/* /proc tracemask is 0 to N-1, kernel signals
				 * are 1 to N, so shift right 1 place
				 */
				prrunp->pr_sighold >>= 1;
				/* kill and stop cannot be held, so check */
				if(prrunp->pr_sighold & (sigmask(SIGKILL) |
				    sigmask(SIGSTOP)) )
					return(EINVAL);
				procp->p_sigmask = prrunp->pr_sighold;
			}

			/* if PRSTEP flag is set, set the set single step
			 * indicator in the process control block because
			 * the user wants to single step through the task
			 *
			 * Only allow single stepping for known architectures.
			 */
			if (prrunp->pr_flags & PRSTEP) {
#if defined(mips) || defined(__alpha)
			    if(((thread->t_procfs.pr_why == PR_FAULTED)
#ifdef mips
				&& (thread->t_procfs.pr_what == EXC_BREAK))
#endif /* mips */
#ifdef __alpha
				&& (thread->t_procfs.pr_what==T_IFAULT_BPT))
#endif /* __alpha */
				|| ((thread->t_procfs.pr_why == PR_SIGNALLED)
				&& (thread->t_procfs.pr_what == SIGTRAP))
				 && (thread->pcb->pcb_sstep == NULL)) {
					procp->thread->pcb->pcb_sstep |=
						PT_STEP;
				}
				else
					return(EINVAL);
#else /* not mips or --alpha */
				return(EINVAL);
#endif /* mips or __alpha */
			}

			if (prrunp->pr_flags & PRCSIG) {
				procp->p_cursig = NULL;
			}
			/* if PRCFAULT flag is set, clear the current fault */
			if (prrunp->pr_flags & PRCFAULT)
				thread->t_procfs.pr_curflt = -1;
			/* if PRSTRACE flag is set, set the set of traced
			 * signals to the value in the prrun structure */
			if (prrunp->pr_flags & PRSTRACE) {
				thread->t_procfs.pr_sigtrace = prrunp->pr_trace;
				if(thread->t_procfs.pr_sigtrace == NULL) {
					thread->t_procfs.pr_qflags &=
						~PRFS_SIGNAL;
				}
				else {
					thread->t_procfs.pr_qflags |= PRFS_SIGNAL;
				}
			}
			/* if PRSFAULT flag is set, set the set of traced
			 * faults to the value in the prrun structure */
			if (prrunp->pr_flags & PRSFAULT) {
				thread->t_procfs.pr_flttrace = prrunp->pr_fault;
				for (i = 0; i < FLTSET_SZ; i++)
				    if(thread->t_procfs.pr_flttrace.word[i] != NULL)
					tracing= TRUE;
				if (tracing)
					thread->t_procfs.pr_qflags |= PRFS_FAULT;
				else
					thread->t_procfs.pr_qflags &= ~PRFS_FAULT;
			}

			/* If PRSADDR flag is set, set the program counter
			 * register in the process control block to the value
			 * in the prrun structure.  The task will resume at
			 * the address that is put into the PC register. */
			if (prrunp->pr_flags & PRSVADDR) {
				count = PROCFS_THREAD_STATE_COUNT;
				if((tmpstate =
				      (struct PROCFS_thread_state *)kalloc(sizeof(struct
				       PROCFS_thread_state))) == NULL) {
					return(ENOMEM);
				}
				thread_getstatus(thread,
					PROCFS_THREAD_STATE,
					tmpstate, &count);
				tmpstate->pc =
					(long )prrunp->pr_vaddr;
				thread_setstatus(thread,
					PROCFS_THREAD_STATE,
					tmpstate, &count);
				kfree(tmpstate);
			}

			/* if PRSTOP flag is set, set the PR_DSTOP flag in
			 * the pr_flags field in the procfs structure which
			 * directs the thread to stop */
			if (prrunp->pr_flags & PRSTOP)
				thread->t_procfs.pr_flags |= PR_DSTOP;
			/* abort the system call if PRSABORT flag is set and
			 * the system call is traced on entry or the process is
			 * in an interruptible sleep */
			if ((prrunp->pr_flags & PRSABORT) &&
			 ((thread->t_procfs.pr_flags & PR_SYSENTRY) ||
				 ((thread->interruptible) &&
				 (thread->state & TH_WAIT))))
				thread->t_procfs.pr_qflags |= PRFS_ABORT;
		}

		/* reset the /proc stop flags and the /proc reasons for
		 * stopping in the pr_why and pr_what fields of the
		 * procfs structure */
		thread->t_procfs.pr_flags &= ~(PR_STOPPED | PR_ISTOP);
		if(procp->task->procfs.pr_flags & PR_ISTOP)
			procp->task->procfs.pr_flags &= ~PR_ISTOP;
		thread->t_procfs.pr_why = NULL;
		thread->t_procfs.pr_what = 0L;
		/* set the thread status to run */
		procp->p_stat = SRUN;
		/* make the thread runnable by resuming the thread */
		thread->state &= ~TH_SUSP;
		thread->state |= TH_RUN;
		if(--thread->suspend_count == NULL)
			thread->halted = TRUE;
		thread_continue(thread);
		break;

	case PIOCTSTOP & PR_CMD_MASK:		/* stop a thread */
		/* if this is a system thread, return error busy */
		prstatp = (struct prstatus *)data;
		thread = (struct thread *)prstatp->pr_tid;

		if (thread->t_procfs.pr_flags & PR_ISSYS)
			return(EBUSY);

		/* set the PR_DSTOP flag in the pr_flags field in the
		 * procfs structure which directs the thread to stop */
		thread->t_procfs.pr_flags |= PR_DSTOP;

		/* wait until the thread stops on an event of interest */
		thread_lock(thread);
		while ((procp->task != NULL) &&
			!(thread->t_procfs.pr_flags & PR_ISTOP) &&
			(procp->p_stat != SZOMB) && (procp->p_stat != NULL)) {
			thread_unlock(thread);
			if( ((error = tsleep(&thread->task->procfs,
			    (PZERO+1) | PCATCH, "thread stop",200))) == EINTR )
				return(error);
			if(procp->task != NULL)
				thread_lock(thread);
		}

		/* break; NO! fall through! */
		if ((procp->task == NULL) || (*(int *)data == -1) ||
			(procp->p_stat == SZOMB) || (procp->p_stat == NULL)) {
			if(procp->task != NULL)
				thread_unlock(thread);
			break;  /* no status struct passed in */
		}
		if(procp->task != NULL)
			thread_unlock(thread);

	case PIOCTSTATUS & PR_CMD_MASK:	/* get thread status */
		/* copy the current status information for the thread
		 * form the appropriate kernel data structures to the
		 * prstatus structure in user space */
		prstatp = (struct prstatus *)data;
		thread = (struct thread *)prstatp->pr_tid;
		prstatp->pr_flags = thread->t_procfs.pr_flags;
		prstatp->pr_why = thread->t_procfs.pr_why;
		prstatp->pr_what = thread->t_procfs.pr_what;

		if (procp->task->procfs.pr_why == PR_SIGNALLED) {
			sig_lock_simple(procp);
			if( thread->u_address.uthread->uu_cursig ) {
				bcopy(
				  &thread->u_address.uthread->uu_cursig->siginfo,
				  &prstatp->pr_info, sizeof(struct siginfo));
			} else {
				bzero(&prstatp->pr_info, 
				  sizeof(struct siginfo));
			}
			sig_unlock(procp);
		}
		prstatp->pr_cursig = thread->u_address.uthread->uu_cursig;
		prstatp->pr_sigpend = thread->u_address.uthread->uu_sig;
		/* convert kernel signal mask to /proc trace mask */
		prstatp->pr_sighold = (procp->p_sigmask <<1);
		if(procp->utask != NULL)
		{
			prstatp->pr_altstack = procp->utask->uu_sigstack;
			if (thread->u_address.uthread->uu_cursig != NULL) {
		            if(thread_signal_disposition(procp->p_cursig) )
		            {
				if(procp->thread == NULL)
				    return(EINVAL);
				prstatp->pr_action.sa_handler =
				procp->thread->u_address.uthread->uu_tsignal[procp->p_cursig];
			    }
			    else
			    {
				prstatp->pr_action.sa_handler =
				procp->utask->uu_signal[procp->p_cursig];
			    }
				prstatp->pr_action.sa_mask =
				procp->utask->uu_sigmask[procp->p_cursig];
				prstatp->pr_action.sa_flags |=
				procp->utask->uu_sigonstack &
				sigmask(procp->p_cursig) ? SA_ONSTACK : NULL;
			}
			prstatp->pr_utime.tv_sec =
			procp->utask->uu_ru.ru_utime.tv_sec;
			prstatp->pr_utime.tv_nsec =
			procp->utask->uu_ru.ru_utime.tv_usec;
			prstatp->pr_stime.tv_sec =
			procp->utask->uu_ru.ru_stime.tv_sec;
			prstatp->pr_stime.tv_nsec =
			procp->utask->uu_ru.ru_stime.tv_usec;
		}
		prstatp->pr_pid = procp->p_pid;
		prstatp->pr_ppid = procp->p_ppid;
		prstatp->pr_pgrp = procp->p_pgrp->pg_id;
		if(procp->p_session->s_leader != NULL)
			prstatp->pr_sid = procp->p_session->s_leader->p_pid;
		prstatp->pr_cutime.tv_sec = 0L;
		prstatp->pr_cutime.tv_nsec = 0L;
		/* compute the sum of the children's user times */
		sum_cutime(procp,&prstatp->pr_cutime);
		prstatp->pr_cstime.tv_sec = 0L;
		prstatp->pr_cstime.tv_nsec = 0L;
		/* compute the sum of the children's system times */
		sum_cstime(procp,&prstatp->pr_cstime);
		prstatp->pr_clname[0] = NULL;

		/* Get the current registers */
		count = PROCFS_THREAD_STATE_COUNT;
		thread_getstatus(thread, PROCFS_THREAD_STATE,
			(int *)&prstatp->pr_reg, &count);

		prstatp->pr_nthreads = procp->task->thread_count;
		prstatp->pr_tid = (tid_t)thread;

		/* if the program counter is invalid, set the pr_instr
		 * field in the prstatus structure to zero; else set it
		 * to the current value in the PC register in the
		 * process control block */
		if (thread->t_procfs.pr_flags & PR_PCINVAL)
			prstatp->pr_instr = 0L;
		else {
			/* prstatp->pr_instr = prstatp->pr_reg[PCB_PC]; */
			prstatp->pr_instr = 0L;
			map = procp->task->map;
			vm_map_reference(map);
			nb = sizeof(long);
			if(vm_map_copyin(map, prstatp->pr_reg.regs[PCB_PC], nb,
				FALSE, &vmcpy) != KERN_SUCCESS) {
				vm_map_deallocate(map);
				prstatp->pr_instr = 0L;
				break;
			}
			vm_map_deallocate(map);
			if(vm_map_copyout(PR_KERN_MAP, &kern_adr,vmcpy)
				!= KERN_SUCCESS) {
				vm_map_copy_discard(vmcpy);
				(void)vm_deallocate(PR_KERN_MAP, kern_adr, nb);
				prstatp->pr_instr = 0L;
				break;
			}
            		if(eblkcpy((caddr_t) kern_adr,&prstatp->pr_instr,nb)) {
				prstatp->pr_flags &= PR_PCINVAL;
				prstatp->pr_instr = 0L;
			}
			(void)vm_deallocate(PR_KERN_MAP, kern_adr, nb);
		}
		break;

	case PIOCTGTRACE & PR_CMD_MASK:	/* get thread traced signals */
		/* get signals traced for the traced thread
		copy the set of signals to be traced from the procfs
		structure to user space */
		bcopy(&thread->t_procfs.pr_sigtrace, (sigset_t *)data,
			sizeof(sigset_t));
		break;

	case PIOCTSTRACE & PR_CMD_MASK:	/* set thread traced signals */
		/* set signals traced for the traced thread
		 * copy the set of signals to be traced from user space
		 * into set of signals traced in the procfs structure */
		thread->t_procfs.pr_sigtrace = *(sigset_t *)data;
		if(thread->t_procfs.pr_sigtrace == NULL)
			thread->t_procfs.pr_qflags &= ~PRFS_SIGNAL;
		else
			thread->t_procfs.pr_qflags |= PRFS_SIGNAL;
		break;

	case PIOCTSSIG & PR_CMD_MASK:		/* set thread sig to siginfo */
		/* if the user specifies a zero as the third argument
		 * of the ioctl call, clear the current signal; else
		 * copy the siginfo structure from user space to the
		 * kernel corresponding siginfo structure for the thread */

		if (*(int *)data == NULL)
		{
			sigqueue_t curinfop;
			s = splhigh();
			/*
			 * p_cursig and p_curinfo should always be in
			 * agreement with each other, so protect changes
			 * with a splhigh().
			 */
			procp->p_cursig = 0;	/* clear current signal */
			curinfop = procp->p_curinfo;
			SIGQ_FREE(procp->p_curinfo);

			task_lock(&procp->task);
			sig_lock_simple(procp);
			/*
			 * Loop thru thread list.
			 */
			for (thread = (thread_t) queue_first(&procp->task->thread_list);
			    !queue_end(&procp->task->thread_list, (queue_entry_t) thread);
			    thread = (thread_t) queue_next(&thread->thread_list)) {
				if (procp->p_curinfo == thread->u_address.uthread->uu_curinfo) {
					thread->u_address.uthread->uu_cursig  = NULL;
					thread->u_address.uthread->uu_curinfo = NULL;
				}
			}
			sig_unlock(procp);
			task_unlock(&p->task);
			splx(s);
		} else {
			s = splhigh();
			sigqueue = SIGQ_ALLOC();
			splx(s);
			if (sigqueue)
				*sigqueue->siginfo = zero_ksiginfo;

			/*
			 * There is a problem with trying to distinguish
			 * between thread and process signals.  Once a
			 * signal is being handled (in issig()), the proc 
			 * p_cursig and thread uu_cursig are set to the same
			 * value. Only if a thread handling a signal in the
			 * kernel (in issig() or psig()) is interrupted by
			 * another thread from the same process is it
			 * possible that the value in p_cursig represents
			 * a different signal than the value in a thread's
			 * uu_cursig.  If the signal is set only in uu_cursig,
			 * but is currently being handled by a thread in
			 * issig() or psig(), then changing the uu_cursig
			 * value will have no affect, because the
			 * issig()/psig() routines only look at the
			 * p_cursig values.
			 *
			 * In order to deal with this problem, we will
			 * set both uu_cursig and p_cursig to the new
			 * signal when uu_cursig and p_cursig are currently
			 * equal.  If they are not equal, we'll assume that
			 * some other thread is currently using p_cursig,
			 * and we won't change it.
			 */
			for (thread = (thread_t) queue_first(&procp->task->thread_list);
			    !queue_end(&procp->task->thread_list, (queue_entry_t) thread);
			    thread = (thread_t) queue_next(&thread->thread_list)) {

				if (thread->u_address.uthread->uu_cursig ==
				    procp->p_cursig) {
					sig_lock_simple(procp);
					s = splhigh();
					/* reset current signal */
					if (!procp->p_curinfo) {
						/* no current siginfo, use the
						 * allocated one 
						 */
						procp->p_curinfo = sigqueue;
						sigqueue = NULL;
					}
					bcopy((struct siginfo *)data,
					      &procp->p_curinfo->siginfo,
					      sizeof(k_siginfo_t));
					procp->p_cursig =
						((struct siginfo)data).si_signo;
					splx(s);

					task_lock(&procp->task);
					/*
					 * Keep thread info up to date with
					 * proc info.
					 */
					if (thread->u_address.uthread->uu_cursig) {
						thread->u_address.uthread->uu_cursig = 
							procp->p_cursig;
						thread->u_address.uthread->uu_curinfo = 
							procp->p_curinfo;
					}
					sig_unlock(procp);
					task_unlock(&procp->task);
				} else {
					task_lock(&procp->task);
					sig_lock_simple(procp);
					if (!thread->u_address.uthread->uu_curinfo) {
						/*
						 * no current siginfo, use the
						 * allocated one 
						 */
						thread->u_address.uthread->uu_curinfo
							= sigqueue;
						sigqueue = NULL;
					}
					bcopy((struct siginfo *)data,
					      thread->u_address.uthread->uu_curinfo->siginfo,
					      sizeof(struct siginfo));
					thread->u_address.uthread->uu_cursig = 
					       ((struct siginfo)data).si_signo;
					sig_unlock(procp);
					task_unlock(&procp->task);
				}
				/* free alloc'ed sigqueue if not needed */
				s = splhigh();
				SIGQ_FREE(siginfo);
				splx(s);
			}
		}

		/* if the user specified a siginfo structure as the
		 * third argument in the ioctl call, the signal
		 * specified in the siginfo structure is sent
		 * to the task */

		 /* if the signal is a kill, reset the /proc stop
		  * flags and the /proc reasons for stopping in the
		  * pr_why and pr_what fields of the procfs structure */
		 if (procp->p_cursig == SIGKILL) {
			 procp->task->procfs.pr_flags &=
				~(PR_STOPPED | PR_ISTOP);
			 thread->t_procfs.pr_flags &=
				~(PR_STOPPED | PR_ISTOP);
			 thread->t_procfs.pr_why = NULL;
			 thread->t_procfs.pr_what = 0L;

			 /* set the task status to run */
			 procp->p_stat = SRUN;

			 /* make the task runnable by resuming the task
			  * and thread */
			 thread->state &= ~TH_SUSP;
			 thread->state |= TH_RUN;
			 if(--thread->suspend_count == NULL)
				thread->halted = TRUE;
			 thread_continue(thread);
			 task_resume(procp->task);
		 }

	case PIOCTKILL & PR_CMD_MASK:		/* send sig to thread */
		/* send the signal specified by the user to the traced
		 * thread with the same semantics as kill */

		/* set up the arguments for the kill routine and call
		 * the kill routine to send the signal */
		uap.pid = procp->p_pid;
		uap.signo = *(int *)data;
		kill(cp, &uap, &retval);
		/* if the process is stopped, make it runnable */
		if (thread->t_procfs.pr_flags & (PR_STOPPED | PR_ISTOP)) {
			/* reset the /proc stop flags and the /proc
			 * reasons for stopping in the pr_why and
			 * pr_what fields of the t_procfs structure */
			thread->t_procfs.pr_flags &=
				~(PR_STOPPED | PR_ISTOP);
			thread->t_procfs.pr_why = NULL;
			thread->t_procfs.pr_what = 0L;
			/* make the thread runnable by resuming the thread */
			thread->state &= ~TH_SUSP;
			thread->state |= TH_RUN;
			if(--thread->suspend_count == NULL)
				thread->halted = TRUE;
			thread_continue(procp->thread);
		}
		if (procp->task->procfs.pr_flags & (PR_STOPPED | PR_ISTOP)) {
			/* reset the /proc stop flags and the /proc
			 * reasons for stopping in the pr_why and
			 * pr_what fields of the procfs structure */
			procp->task->procfs.pr_flags &=
				~(PR_STOPPED | PR_ISTOP);
			procp->task->procfs.pr_why = NULL;
			procp->task->procfs.pr_what = 0L;
			/* set the thread status to run */
			procp->p_stat = SRUN;
			/* make the thread runnable by resuming the thread */
			task_resume(procp->thread);
		}
		break;

	case PIOCTUNKILL & PR_CMD_MASK:	/* delete pending sig */
		/* reset the pending signal using the signal number
		specified by the user; the sigmask macro converts
		the signal number into a signal mask */
		procp->p_sig &= ~(sigmask(*(int *)data));
		break;

	case PIOCTGFAULT & PR_CMD_MASK:	/* get thread traced faults */
		/* get faults traced for the traced thread
		 * copy the set of faults to be traced from the procfs
		 * structure to user space */
		bcopy(&thread->t_procfs.pr_flttrace, (fltset_t *)data,
			sizeof(fltset_t)); 
		break;

	case PIOCTSFAULT & PR_CMD_MASK:	/* set thread traced faults */
		/* set faults traced for the traced thread
		 * copy the set of faults to be traced from user space
		 * to set of faults traced in the procfs structure */
		bcopy((fltset_t *)data, &thread->t_procfs.pr_flttrace,
			sizeof(fltset_t));
		break;

	case PIOCTGFPREG & PR_CMD_MASK:	/* get thread floating point regs */
		/* copy the set of floating point registers from the
		 * process control block of the traced thread to user
		 * space */
		count = PROCFS_FLOAT_STATE_COUNT;
		thread_getstatus(thread, PROCFS_FLOAT_STATE,
			(long *)data, &count);
		break;

	case PIOCTSFPREG & PR_CMD_MASK:	/* set thread floating point regs */
		/* if the thread is not stopped, return error busy */
		if ( !(thread->t_procfs.pr_flags & PR_ISTOP) )
			return(EBUSY);

		/* copy the set of floating point registers from user
		 * space to the process control block of the traced
		 * thread */
		count = PROCFS_FLOAT_STATE_COUNT;
		thread_setstatus(thread, PROCFS_FLOAT_STATE,
			(long *)data, &count);
		break;

	case PIOCTGREG & PR_CMD_MASK:		/* get thread general regs */
		/* copy the set of general registers from the process
		 * control block of the traced thread to user space */
		count = PROCFS_THREAD_STATE_COUNT;
		thread_getstatus(thread, PROCFS_THREAD_STATE,
			(long *)data, &count);
		break;
	case PIOCTSREG & PR_CMD_MASK:		/* set thread general regs */
		/* if the thread is not stopped, return error busy */
		if ( !(thread->t_procfs.pr_flags & PR_ISTOP) )
			return(EBUSY);

		/* copy the set of general registers from user space to
		 * the process control block of the traced thread
		 */
		count = PROCFS_THREAD_STATE_COUNT;
		regp = (struct PROCFS_thread_state *)data;
		thread_setstatus(thread, PROCFS_THREAD_STATE, regp, &count);
		break;

	case PIOCTACTION & PR_CMD_MASK:	/* get thread sig actions */
		/* copy the signal action structures for the task from
		 * the kernel to the array of action structures in user
		 * space */
		if(procp->utask == NULL)
			return(EINVAL);
		ap = (struct sigaction *)data;
		for (i = 0; i < NSIG; i++, ap++) {
			ap->sa_handler = procp->utask->uu_signal[i];
			ap->sa_mask = procp->utask->uu_sigmask[i];
			ap->sa_flags |= procp->utask->uu_sigonstack & sigmask(i)
				? SA_ONSTACK : NULL;
		}
		break;

	case PIOCTTERM & PR_CMD_MASK:		/* terminate a thread */
		thread_force_terminate(thread);
		break;
	default:
		/* illegal ioctl command */
		return(EINVAL);
		break;
	}
	return(retval);
}

#endif /* PROCFS_U */

#define	PRFS_ADD_TIMES(timer, tp)					 \
	MACRO_BEGIN							 \
	{							 	 \
	(tp)->tv_sec += (timer).high_bits + ( (timer).low_bits/1000000); \
	(tp)->tv_nsec += ((timer).low_bits%1000000)*1000;		 \
	}								 \
	MACRO_END



/*
sum_cutime - routine called by various status IOCTL commands to compute the
	 sum of the childrens' user times.

Inputs -  p: a pointer to a process table entry.

Outputs - t: a pointer to a time structure and is the output parameter.
		 It is filled in by the routine.

Description - The children of the task referred to by p are surveyed and the
	  current  user execution time is summed up into the given structure.
*/
sum_cutime(p,tp)
register struct proc *p;
struct timestruct *tp;
{
	struct thread *thread;

	/* if this task has any children, go sum the youngest */
	if((p->p_cptr != NULL) && (p->p_cptr->p_stat != SIDL &&
	    p->p_cptr->p_stat != SZOMB))
		sum_cutime(p->p_cptr, tp);

	/* if this task has any siblings, go sum its oldest sibling */
	if((p->p_osptr != NULL) && (p->p_osptr->p_stat != SIDL &&
	    p->p_osptr->p_stat != SZOMB))
		sum_cutime(p->p_osptr, tp);

	/*
	 * Add up the user times for this task.  User time is only
	 * currently saved in the thread structure; scan all threads for
	 * this task.
	 */

	task_lock(p->task);
	for (thread = (thread_t) queue_first(&p->task->thread_list);
	!queue_end(&p->task->thread_list, (queue_entry_t) thread);
	thread = (thread_t) queue_next(&thread->thread_list))
		PRFS_ADD_TIMES(thread->user_timer, tp);

	task_unlock(p->task);

	/* NOTE: reaped user time is currently not implemented in the OSF/1
	 * kernel.  */
}


/*
sum_cstime - routine called by various status IOCTL commands to compute the
	 sum of the childrens' system times.

Inputs -  p: a pointer to a process table entry.

Outputs - t: a pointer to a time structure and is the output parameter.
	     It is filled in by the routine.

Description - The children of the task referred to by p are surveyed and the
	  current  system execution time is summed up into the given structure.
*/

sum_cstime(p,tp)
register struct proc *p;
struct timestruct *tp;
{
	struct thread *thread;

	/* if this task has any children, go sum the youngest */
	if((p->p_cptr != NULL) && (p->p_cptr->p_stat != SIDL &&
	    p->p_cptr->p_stat != SZOMB))
		sum_cstime(p->p_cptr, tp);

	/* if this task has any siblings, go sum its oldest sibling */
	if((p->p_osptr != NULL) && (p->p_osptr->p_stat != SIDL &&
	    p->p_osptr->p_stat != SZOMB))
		sum_cstime(p->p_osptr, tp);

	/*
	 * Add up the system times for this task.  System time is only
	 * currently saved in the thread structure; scan all threads for
	 * this task.
	 */

	task_lock(p->task);
	for (thread = (thread_t) queue_first(&p->task->thread_list);
	!queue_end(&p->task->thread_list, (queue_entry_t) thread);
	thread = (thread_t) queue_next(&thread->thread_list))
		PRFS_ADD_TIMES(thread->system_timer, tp);

	task_unlock(p->task);

	/* NOTE: reaped system time is currently not implemented in the OSF/1
	 * kernel.  */
}



procfs_bread(vp, lbn, bpp, cred)
	register struct vnode *vp;
	off_t lbn;
	struct buf **bpp;
	struct ucred *cred;
{
	return (EOPNOTSUPP);
}

procfs_brelse(vp, bp)
	register struct vnode *vp;
	register struct buf *bp;
{
	return (EOPNOTSUPP);
}
