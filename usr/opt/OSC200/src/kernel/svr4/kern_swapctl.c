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
static char *rcsid = "@(#)$RCSfile: kern_swapctl.c,v $ $Revision: 1.1.2.5 $ (DEC) $Date: 1993/08/13 20:17:27 $";
#endif
/*
 *  Module Name:
 *	kern_swapctl.c
 *  Description:
 *	implements the SVR4 swapctl() system call.   This system call
 *	add, list and count the swap devices on system.  The SC_REMOVE
 *	command is unsupported now.
 *  Origin:
 *	swapctl() system call was designed and developed 
 *	based on AT&T SVID Third Edition Volume III, 
 *	man pages on machine "linder" running SVR4, and OSF/1.
 *  Algorithm:
 *	It makes two passes through the command. In the first
 *	pass, user errors are checked. In the second pass
 *	the command is tried to be executed.   If the command 
 *	is to add a new swap device existing vm_swapon() function is
 *	called with the path name of swap to added.  Removing a swap
 *	is not supported now and returns ENOSYS if attempted.
 *	To list the swap devices it goes through linked list of the 
 *	swap file entries from vm_swap_head and the values are
 *	listed for the user specified number of entries.
 *	The count of the entries are returned by counting the number
 *	of the entries in the linked list starting vm_swap_head.
 *	
 */

#include <sys/param.h>
#include <sys/errno.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <vm/vm_page.h>
#include <vm/vm_swap.h>
#include <sys/swap.h>

#if SEC_BASE
#include <sys/security.h>
#include <sys/audit.h>
#endif

#define BLOCK_SHIFT  (9)		/* SVR4 block size is 512 bytes */

swapctl(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		long cmd;
		long arg;
	} *uap = (struct args *) args;
	
	int cmd, ret;
	void *arg;
	long lowat, hiwat;
	int n_swap, swt_n;
	struct vm_swap *swap;
	struct swapres swapres;
	struct swaptable swaptab, *u_swaptab;
	struct swapent swapent, *u_swapent;
	char   c_null[1];
	int len_copied;
	extern lock_data_t vm_swap_modify_lock;
	long audit_arg[4];

	/*
	 * On alpha, syscalls always have argumets as long. Convert 'em
	 * to match the user interface as in SVID.
	 */

	cmd = (int) uap->cmd;
	arg = (void *)uap->arg;
	c_null[0] = (char)0;
	ret = ESUCCESS;

	/* 
	 * Check for possible user errors. Only root can add/remove
	 * a swap file
	 */

	switch (cmd) {
	case SC_ADD:
#if     SEC_BASE
        	if (!privileged(SEC_LIMIT, EPERM)) {
			ret = EPERM;
			break;
		}
#else
        	if (ret = suser(u.u_cred, &u.u_acflag))
			break;
#endif
		if (ret=copyin(arg, &swapres, sizeof(struct swapres)))
			break;
		/*
		 * We force the sr_start to be zero as OSF/1 expects
		 * all start addresses to be zero. 
		 */
		if (swapres.sr_start != (off_t)0) {
			ret = ENOSYS;
			break;
		}
		/* 
		 * adding zero blocks returns EINVAL in SVR4
		 */
		if (swapres.sr_length == (off_t)0) {
			ret = EINVAL;
			break;
		}
		if (swapres.sr_name == (char *)0) {
			ret = ENOENT;
			break;
		}
		break;

	case SC_REMOVE:
		/* 
		 * NOTE: Removing swap file is *UNSUPPORTED* now !
		 */
		ret = ENOSYS;
		break;

	case SC_LIST:
		u_swaptab = (struct swaptable *)arg;
		if (ret=copyin(u_swaptab, &swaptab, sizeof(swaptab)))
			break;
		/*
		 * This check should probably be done after making sure
		 * there is at least one swap file. But SVR4 returns
	 	 * ENOMEM if swt_n == 0, even if there are no
		 * swap files to report on.
		 */
		if (swaptab.swt_n == 0) {
			ret = ENOMEM;
			break;
		}
		break;

	case SC_GETNSWP:
		break;

	default:
		ret = EINVAL;
		break;

	}
	
	if (ret != ESUCCESS)
		goto out;
	
	/*
	 * Now that many possible user errors are checked, try doing
	 * add, list or count swap files.
	 */

	switch (cmd) {
	case SC_ADD:
		/*
		 * lowat is 1 so that the file is shrunk as paging space
		 * freed.  (This is valid only for regular files but
		 * unsupported currently in OSF/1)
		 * sr_start cannot be specified in OSF/1 now.
		 * sr_length is in terms of 512 byte blocks in SVR4
		 */
		lowat = (long)1;
		hiwat = (long)swapres.sr_length << BLOCK_SHIFT;
        	if ((ret=vm_swapon(swapres.sr_name, 0, lowat,
		     hiwat, UIO_USERSPACE)) != KERN_SUCCESS) {
			/*
	 		* We are limited with SVR4 on return values, make
	 		* the best erffort to return the most suitable.
	 		*/
			switch(ret) {
			case ENOMEM:
			case ENOTDIR:
			case ENAMETOOLONG:
			case ENOENT:
			case ELOOP:
			case EISDIR:
			case EPERM:
			case EROFS:
			case EINVAL:
			case EFAULT:
				break;
			case ENXIO:
			case ENODEV:
				ret = ENOENT;
				break;
			case EACCES:
				ret = EPERM;
				break;
			case EBUSY:
				ret = EEXIST;
				break;
			default:
				ret = EINVAL;
				break;
			}
		}
		break;

	case SC_LIST:
		swt_n = swaptab.swt_n;
		u_swapent = &u_swaptab->swt_ent[0];
		swap = vm_swap_head;
		lock_read(&vm_swap_modify_lock);

		/*
		 * Go through each swap entry from vm_swap_head and list
		 * the info to the user. 
		 */

		for (n_swap=0; swap ; swap=swap->vs_fl, u_swapent++) {
			n_swap++;
			/*
			 * Copy in entire swapent, although we just need
		         * the pointer to name of swap file for completeness.
			 */
			if (ret=copyin(u_swapent, &swapent, sizeof(swapent))) {
				lock_done(&vm_swap_modify_lock);
				goto out;
			}

			if (swapent.ste_path == (char *)NULL) {
				ret = ENOMEM;
				lock_done(&vm_swap_modify_lock);
				goto out;
			}
			/* 
			 * In OSF/1 swap always start at block 0.
			 * ste_length is in terms of 512 byte blocks
			 * ste_flags is zero as SC_REMOVE is unsupported.
			 */

			swapent.ste_start = (off_t)0;
			swapent.ste_length = (off_t)(swap->vs_swapsize <<
					     (swap->vs_oshift - BLOCK_SHIFT));
			swapent.ste_pages = (long)swap->vs_swapsize;
			swapent.ste_free = (long)swap->vs_freespace;
			swapent.ste_flags = (long)0;

			if (ret=copyout(&swapent, u_swapent, sizeof(swapent))) {
				lock_done(&vm_swap_modify_lock);
				goto out;
			}

			if (ret=copyoutstr(swap->vs_path, swapent.ste_path,
			    MAXPATHLEN, &len_copied)) {
				lock_done(&vm_swap_modify_lock);
				goto out;
			}

			if (swap->vs_fl == vm_swap_head)
				break;
			/*
			 * If there are more entries to be listed and no
			 * more user space return error 
			 */
			if (n_swap == swt_n) {
				ret = ENOMEM;
				lock_done(&vm_swap_modify_lock);
				goto out;
			}
		}
		lock_done(&vm_swap_modify_lock);

		/*
		 * Give the number of swapents actually listed too
   		 */
		*retval = n_swap;

		break;

	case SC_GETNSWP:
		swap = vm_swap_head;

		lock_read(&vm_swap_modify_lock);
		for (n_swap = 0; swap; swap = swap->vs_fl) {
			n_swap++;
			if (swap->vs_fl == vm_swap_head)
				break;
		}
		lock_done(&vm_swap_modify_lock);

		*retval = n_swap;
		ret = ESUCCESS;
		break;
	}
	
out:
	if( DO_AUDIT(SYS_swapctl,ret) ) {

	    if (cmd == SC_ADD) {
		audit_arg[0] = (long) cmd;
		audit_arg[1] = (long) swapres.sr_name;
		audit_arg[2] = (long) swapres.sr_start;
		audit_arg[3] = (long) swapres.sr_length;
		    AUDIT_CALL(SYS_swapctl, ret, audit_arg, *retval, AUD_VHPR, "ar3300000" );
	    }
	}

	return(ret);
}
