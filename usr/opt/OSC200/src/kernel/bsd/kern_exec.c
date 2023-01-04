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
static char	*rcsid = "@(#)$RCSfile: kern_exec.c,v $ $Revision: 4.5.25.16 $ (DEC) $Date: 1993/12/17 01:13:41 $";
#endif 
/*
 * 04-Feb-1992	Jeff Denham
 *	Update to POSIX.4/D11 (comments only!)
 *
 * 19-Sep-1991, Jeff Denham
 *	. Uncomment OSF/1.0.1 additions added in last edit.
 *	. Fixed clearing of POSIX.4 timers on an exec with a call to
 *	  clear_psx_timers().
 *
 * 06-Jun-1991, Paula Long, Ed Benson
 *      P1003.4/D10 timers and Process Memory Locking should be inherted across
 *      an exec.  Note: we've used the OSF model for this work, it's a hack
 *      but it should work.
 *
 * 4-Jun-1991	lebel
 *	Added support for > 64 open file descriptors per process.
 *
 * 14-May-1991, afd
 *      First cut at Alpha and 64-bit support
 *
 * 28-Feb-1991, Ken Lesniak
 *	Add support for COFF and ELF shared libraries and ELF executables.
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
 * OSF/1 Release 1.0.1
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
 * Copyright (c) 1982, 1986, 1989 Regents of the University of California.
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

#include <sys/secdefines.h>
#include <sys/exec_incl.h>
#include <rt_pml.h>
#include <rt_timer.h>

#include <sys/dk.h>		/* for SAR counters */
#include <mach/exception.h>
#ifdef __alpha
#include <machine/alpha_ptrace.h>
#endif

#include "bin_compat.h"

extern vm_offset_t stackinc;

#ifdef __hp_osf
int aout_uses_loader = 1;
int coff_uses_loader = 1;
#else
int aout_uses_loader = 0;
int coff_uses_loader = 0;
#endif
int elf_uses_loader = 0;
int macho_uses_loader = 0;

#if	OSF_MACH_O
extern int decode_mach_o_hdr(void *, size_t, unsigned long, mo_header_t *);
#endif


#if	SEC_BASE
#include <sys/security.h>
#include <sys/audit.h>
#endif
#if	SEC_ARCH
#include <sys/secpolicy.h>
#endif
/*
 * exec system call, with and without environments.
 */
struct execa {
	char	*fname;
	char	**argp;
	char	**envp;
};

execv(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	((struct execa *)args)->envp = NULL;
	return (execve(p, args, retval));
}

#ifdef	ibmrt
/* New RXTUnix system call for execve with single step active */
exect(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	int error;

	error = execve(p, args, retval);
	if (!error)
		u.u_ar0[ICSCS] |= ICSCS_INSTSTEP;
	return (error);
}
#endif

extern struct mount *proc_root;
extern struct mount *proc_invlist;

execve(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct execa *uap;
	int na, ne, nc;		/* LEAVE THESE AS INTS FOR ALPHA */
	int uid, gid;
	int	docoff = 0;
	int	do_bsd_a_out = 0;
	int	do_o_mach_o = 0;
	int	do_elf = 0;
	boolean_t is_priv;
	char *shell_name, *shell_arg;
	char shell_name_tail[MAXCOMLEN + 1];
	struct vnode *vp;
	int vpsaved;
	struct vattr vattr;
	register struct nameidata *ndp = &u.u_nd;
	vm_offset_t exec_args = 0;
	char line[MAXINTERP];
	struct ufile_state *ufp = &u.u_file_state;
	union {
		char	ex_shell[MAXINTERP];	/* #! and interpreter name */
#if	SYSV_ELF
		Elf32_Ehdr ehdr;
#endif
#if	SYSV_COFF
		/* 
		 * Coff fileheader structures.
		 */
		struct {
			struct filehdr fhdr;
			struct aouthdr ohdr;
#define ahdr exdata.coff.ohdr
		} coff;
		struct {
			short	magic;
		} coff_hdr;
#endif
#if	OSF_MACH_O
		long	omo_magic;
#endif
#if	BSD_A_OUT
		struct	exec ex_exec;
#endif
	} exdata;
	int resid, error;
#if	SYSV_COFF
	int	aouthdr_offset;
#endif
#if	OSF_MACH_O
	long		entry_addr = 0;
	int		conversion_error;
        char		mo_header_buf[MO_SIZEOF_RAW_HDR];
	                              /* buffer for raw canonical version */
        mo_header_t	mo_header;    /* translated version of the header */
#endif
        int i,s;
        register queue_head_t   *list;
        register thread_t       thread, cur_thread, prev_thread;
        u_int audlen = 0;
        char *audptr[4];

        audptr[2] = audptr[3] = NULL;
	uap = (struct execa *) args;

#if BIN_COMPAT
	/* We may end up running an OSF/1 program or a program that is 
	 * supported by a different compatability module. */
	if (u.u_compat_mod) {
		cm_terminate(u.u_compat_mod);
		u.u_compat_mod = 0;
	}
#endif /*BIN_COMPAT*/

	vpsaved = NULL;
        ts_sysexec++; /* global table() system call counter (see table.h) */

	ndp->ni_nameiop = LOOKUP | FOLLOW;
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = uap->fname;
	if (error = namei(ndp))
		return (error);
	vp = ndp->ni_vp;
	VOP_GETATTR(vp, &vattr, u.u_cred, error);
	if (error)
		goto bad;
	if (error = exec_get_effective_ids(vp, &vattr, &uid, &gid, &is_priv))
		goto bad;
	if (error = exec_check_access(p, vp, &vattr))
		goto bad;

	/* begin /proc code */
	/* If someone has this task open, we got work */
	if( (p->task->procfs.pr_qflags & PRFS_OPEN)) {
		int rmode, a_error;
		struct procnode *pn;
		pn = VNTOPN(p->p_vnptr);

		PROC_LOCK(p);
		/*
		 * If the object is set UID, set GID, or if the current
		 * UID is not root and the object's uid is different or
		 * the object is not readable, put the vnode on the
		 * invalid list, remove the proccess table reference
		 * to the vnode, mark the vnode invalid. If the vnode
		 * has a writer, request a stop and set
		 * run-on-last-close, otherwise, just clear out the
		 * task procfs structure and all thread procfs
		 * structures.
		 */
		rmode = VREAD;
		VOP_ACCESS(vp, rmode, u.u_cred, a_error);

		/* if the vnode is already on the invalid list, skip all */
		if( !(pn->prc_flags & PRC_FDINVLD) &&
		  ((vattr.va_mode & (VSUID | VSGID)) || (a_error != NULL) ) )
		{
			struct vnode *lvp, *tlvp;

			/* Get the vnode pointer */
			lvp = p->p_vnptr;
			p->p_vnptr = 0;
			PROC_UNLOCK(p);

			/* move the vnode off the valid list and onto the
			 * invalid list */
			insmntque(lvp, proc_invlist);
			pn->prc_flags |= PRC_FDINVLD;
	    
			if(lvp->v_wrcnt > 0) {
				/* if there is a writer, but stop on exec is
				 * not set, stop the task, and set RLC */
				unix_master();
				p->task->procfs.pr_flags |= (PR_RLC);
				p->task->procfs.pr_flags |= PR_STOPPED |PR_ISTOP;
				p->task->procfs.pr_flags &= ~(PR_PCINVAL);
				p->task->procfs.pr_why = PR_REQUESTED;
				p->task->procfs.pr_what = NULL;
				p->task->procfs.pr_tid = p->thread;
				p->task->procfs.pr_qflags &= ~PRFS_OPEN;
				wakeup(&(p->task->procfs));
				task_suspend(p->task);
				unix_release();
			}
			else {
				/* clear the task and thread procfs structs */
				uid_t tmp_uid;
				gid_t tmp_gid;
				struct vnode *tmp_vp;
				register int tmp_prot;
				thread_t thread;
	
				task_lock(p->task);
				/* save the uid, gid, and protection info */
				tmp_uid = p->task->procfs.pr_uid;
				tmp_gid = p->task->procfs.pr_gid;
				tmp_prot = p->task->procfs.pr_protect;
				tmp_vp = p->task->procfs.pr_exvp;
				/* clear the struct */
				bzero(&p->task->procfs, sizeof(procfs_t));
				/* reset uid/gid/perm */
				p->task->procfs.pr_uid = tmp_uid;
				p->task->procfs.pr_gid = tmp_gid;
				p->task->procfs.pr_protect = tmp_prot;
				p->task->procfs.pr_exvp = tmp_vp;
				/* clear out the procfs in all threads */
				for (thread = (thread_t) queue_first(&p->task->thread_list);
				  !queue_end((queue_entry_t *)&p->task->thread_list, (queue_entry_t *) thread);
				  thread = (thread_t) queue_next(&p->thread->thread_list)){
					bzero(&thread->t_procfs,
					sizeof(struct t_procfs));
				}
				task_unlock(p->task);
			}
			VN_UNLOCK(lvp);
		}
		PROC_UNLOCK(p);
	}

	/* save the new UID GID and protections for the executable file */
	p->task->procfs.pr_uid = vattr.va_uid;
	p->task->procfs.pr_gid = vattr.va_gid;
	p->task->procfs.pr_protect = vattr.va_mode;
	/* end /proc code */

       /*  
	* Is this a multi-threaded process ?
	* If yes, terminate all but the current thread and reset
	* the "prime thread" in the proc structure to the current 
	* thread.
	*
	* Iterate through all the threads.
	* While waiting for each thread, we gain a reference to it
	* to prevent it from going away on us.  This guarantees
	* that the "next" thread in the list will be a valid thread.
	*
	*/

        task_lock(p->task);

        if (p->task->thread_count > 1) {
		p->task->suspend_count++;
		cur_thread = current_thread();
		list = &p->task->thread_list;
        	thread = (thread_t) queue_first(list);
        	prev_thread = THREAD_NULL;
        	while (!queue_end(list, (queue_entry_t) thread)) {
                	if (thread != cur_thread) {
                        	(void) thread_reference(thread);
                        	task_unlock(p->task);
                        	if (prev_thread != THREAD_NULL)
                        		(void) thread_deallocate(prev_thread); /* may block */
                        	(void) thread_terminate(thread);
                        	prev_thread = thread;
                        	task_lock(p->task);
                	}
         	       thread = (thread_t) queue_next(&thread->thread_list);
     		}	
		p->thread = cur_thread;
		p->task->suspend_count--;
		task_unlock(p->task);
		if (prev_thread != THREAD_NULL)
                	(void) thread_deallocate(prev_thread);         /* may block */
	}
	else
		task_unlock(p->task);	

#if	SEC_BASE
	/*
	 * If not on NOSUID filesystem, check for EXECSUID privilege
	 * for SUID programs.  Audit change of IDs.
	 */
	if (security_is_on) {
		int mflag;
		MOUNT_LOCK(vp->v_mount);
		mflag = vp->v_mount->m_flag;
		MOUNT_UNLOCK(vp->v_mount);
		if (!(mflag & M_NOSUID) && (error = exec_allowed(&vattr)))
			goto bad;
	}
#endif

	/*
	 *	Read in the header to get magic number.
	 *	This magic number is architecture-dependent.
	 */
	exdata.ex_shell[0] = '\0';	/* for zero length files */
	error = vn_rdwr(UIO_READ, vp, (caddr_t)&exdata, sizeof(exdata),
	    (off_t)0, UIO_SYSSPACE, IO_UNIT, u.u_cred, &resid);
	if (error)
		goto bad;
	if (error = exec_get_shell(exdata.ex_shell, &shell_name, &shell_arg))
		goto bad;

	if (shell_name) {

		/*
		 * Save the shell_name and shell_arg because we are about
		 * to overwrite exdata.ex_shell.  Also adjust shell_name
		 * and shell_arg pointers to point to the new copies of
		 * the strings.
		 */
		bcopy((caddr_t)exdata.ex_shell, (caddr_t)line, MAXINTERP);
		shell_name = line + (shell_name - exdata.ex_shell);
		if (shell_arg)
			shell_arg = line + (shell_arg - exdata.ex_shell);

		/*
		 * Switch over to loading the shell.  Finished with vnode
		 * for the script.  Get a vnode for the shell.  Make sure
		 * the shell has the proper access permissions.  Read in
		 * the header.  The file must be at least MAXINTERP byte
		 * long.  If not, just as above, we assume it must be shell
		 * script.  However, at this point, we won't allow shell
		 * that indirectly refer to other shell scripts.  Save the
		 * directory entry name as returned by namei() to use later
		 * as argv[0].  Lastly, we do not allow shell scripts to be
		 * setuid or setgid.
		 */
		vrele(vp);
		ndp->ni_nameiop = LOOKUP | FOLLOW; 
		ndp->ni_segflg = UIO_SYSSPACE;
		ndp->ni_dirp = shell_name;
		if (error = namei(ndp))
			return (error);
		vp = ndp->ni_vp;
		VOP_GETATTR(vp, &vattr, u.u_cred, error);
		if (error)
			goto bad;
		if (exec_check_access(p, vp, &vattr) == -1)
			goto bad;
		error = vn_rdwr(UIO_READ, vp, &exdata, sizeof(exdata),
			     (off_t)0, UIO_SYSSPACE, IO_UNIT, u.u_cred, &resid);
		if (error)
			goto bad;
		if (resid) {
			error = ENOEXEC;
			goto bad;
		}
		bcopy((caddr_t)ndp->ni_dent.d_name, (caddr_t)shell_name_tail,
		    MAXCOMLEN);
		shell_name_tail[MAXCOMLEN] = '\0';
		is_priv = FALSE;	/* shell scripts can't be setid */
		uid = u.u_cred->cr_uid;	/* shell scripts can't be setuid */
		gid = u.u_cred->cr_gid; /* shell scripts can't be setgid */
	}

	/* Save the vnode being exec-ed; this is used for PIOCOPENM and to
	 * return in prstatus pr_exvp for use by utilities that need to find
	 * the physical file that a running process was created from.
	 */
	if(p->task->procfs.pr_exvp != NULLVP)
		vrele(p->task->procfs.pr_exvp);
	p->task->procfs.pr_exvp = vp;
	VREF(p->task->procfs.pr_exvp);
	vpsaved = 1;

	/* See if we can recognize the file format.  Those that we
	 * recognize but don't know how to load will be sent off to
	 * the user space loader.
	 */

#if	SYSV_ELF
	if (IS_ELF(exdata.ehdr)) {
		if (elf_uses_loader) goto call_exec_loader;

		/* Make sure we've read all of the ELF header. Beats me
		 * why the initial read doesn't read sizeof(exdata) bytes.
		 */

		if (sizeof(Elf32_Ehdr) > MAXINTERP) {
			error = vn_rdwr(UIO_READ, vp, (caddr_t)&exdata.ehdr,
			    sizeof(Elf32_Ehdr), (off_t)0, UIO_SYSSPACE,
			    IO_UNIT, u.u_cred, &resid);
			if (error)
			    goto bad;
		}

		/* If this object uses or is a shared library, exec the loader */

		if (exdata.ehdr.e_type == ET_DYN)
			goto call_exec_loader;

		/* Make sure we have an acceptable object:
		 *	little endian header
		 *	little endian text/data
		 *	machine type is MIPS
		 *	program headers are present
		 *	object type is executable
		 */

		if ((exdata.ehdr.e_ident[EI_DATA] != ELFDATA2LSB) ||
		    (exdata.ehdr.e_flags & EF_MIPS_OPSEX) ||
		    (exdata.ehdr.e_machine != EM_MIPS) ||
		    (exdata.ehdr.e_phoff == 0) ||
		    (exdata.ehdr.e_phentsize == 0) ||
		    (exdata.ehdr.e_phnum <= 0) ||
		    (exdata.ehdr.e_type != ET_EXEC)) {
			error = ENOEXEC;
			goto bad;
		}

		/* Looks like we have a static-linked ELF executable */

		do_elf = 1;
		goto gotobject;
	}
#endif

#if	BSD_A_OUT
	/*
	 * Read in first few bytes of file for segment sizes, magic number:
	 *	OMAGIC = plain executable
	 *	NMAGIC = RO text
	 *	ZMAGIC = demand paged RO text
	 * Also an ASCII line beginning with #! is
	 * the file name of a ``shell'' and arguments may be prepended
	 * to the argument list if given here.
	 *
	 * SHELL NAMES ARE LIMITED IN LENGTH.
	 *
	 * ONLY ONE ARGUMENT MAY BE PASSED TO THE SHELL FROM
	 * THE ASCII LINE.
	 */
	if (exdata.ex_exec.a_magic == ZMAGIC ||
	    exdata.ex_exec.a_magic == NMAGIC) {
		if (exdata.ex_exec.a_text == 0) {
			error = ENOEXEC;
			goto bad;
		}
		do_bsd_a_out= 1;
		goto gotobject;
	} else
	if (exdata.ex_exec.a_magic == OMAGIC) {
		exdata.ex_exec.a_data += exdata.ex_exec.a_text;
		exdata.ex_exec.a_text = 0;
		do_bsd_a_out = 1;
		goto gotobject;
	} else
#ifdef	balance
	if (exdata.ex_exec.a_magic == 0x10ea) {	/* ZMAGIC: 0@0 */
						/* no XMAGIC yet */
		exdata.ex_exec.a_magic = ZMAGIC;	/* make other code easier */
		if (exdata.ex_exec.a_text == 0) {
			error = ENOEXEC;
			goto bad;
		}
		do_bsd_a_out = 1;
		goto gotobject;
	} else
#endif
#endif	/* BSD_A_OUT */
#if	OSF_MACH_O
	if (exdata.omo_magic == OUR_MOH_MAGIC) {
		if (macho_uses_loader) goto call_exec_loader;

		/*
		 * Now read in the complete file header, 
		 * starting from the beginning.
		 */
		error = vn_rdwr (UIO_READ, vp, (caddr_t)mo_header_buf,
			    sizeof(mo_header_buf), (off_t)0, UIO_SYSSPACE,
			    IO_UNIT, u.u_cred, &resid);
		if (error)
			goto bad;

		/* Convert the canonical version of the header so we can
		 * read it.  If we can't convert it here, send the file to
		 * the user space loader -- maybe it can load the file.
		 */

		conversion_error = decode_mach_o_hdr ((void *)mo_header_buf,
				     (size_t)sizeof(mo_header_buf),
				     (unsigned long)MOH_HEADER_VERSION,
				     &mo_header);
		if (conversion_error != MO_HDR_CONV_SUCCESS) 
			goto call_exec_loader;

		/* Now we have valid header information; see if we 
		 * can load it here.
		 */
		if ((!(mo_header.moh_flags & MOH_EXECABLE_F))
		    || (mo_header.moh_flags & MOH_RELOCATABLE_F) /* needs relocation */
		    || (mo_header.moh_flags & MOH_UNRESOLVED_F)) {
			goto call_exec_loader;
		}
		/*
		 * The following checks are for whether the file
		 * can be loaded/executed on this system.
		 * We only have to worry about the 
		 * version number (and vendor_type) when
		 * there is a compatibility problem, 
		 * e.g. check that version >= N, where
		 * versions < N are not supported or are
		 * supported differently.  Then this 
		 * program only has to be updated for
		 * those changes it would find incompatible.
		 */
		if ((mo_header.moh_byte_order == OUR_BYTE_ORDER)
		    && (mo_header.moh_data_rep_id == OUR_DATA_REP_ID)
		    && (mo_header.moh_cpu_type == OUR_CPU_TYPE)
		    && (mo_header.moh_max_page_size==PAGE_SIZE)
		    && (mo_header.moh_sizeofcmds > 0)
		    && (mo_header.moh_load_map_cmd_off >= 
			mo_header.moh_first_cmd_off)
		    && (mo_header.moh_load_map_cmd_off < 
			(mo_header.moh_first_cmd_off +
			 mo_header.moh_sizeofcmds))) {
			        do_o_mach_o = 1;
				goto gotobject;
		} else {
			error = ENOEXEC;
			goto bad;
		}
	} else
#endif	/* OSF-MACH-O */
#if	SYSV_COFF
#ifdef	i386
	if (exdata.coff_hdr.magic == I386MAGIC) {
		aouthdr_offset = sizeof(struct filehdr);
		goto gotcoff;
	} else
#endif
#ifdef	multimax
	if (exdata.coff_hdr.magic == N16WRMAGIC ||
	    exdata.coff_hdr.magic == N16ROMAGIC) {
		aouthdr_offset = N16FILHSZ;
		goto gotcoff;
	} else if (exdata.coff_hdr.magic == NS32GMAGIC ||
		   exdata.coff_hdr.magic == NS32SMAGIC) {
		aouthdr_offset = FILHSZ;
		goto gotcoff;
	} else
#endif
#ifdef	mips
	if (exdata.coff_hdr.magic == MIPSMAGIC) {
		aouthdr_offset = FILHSZ;
		goto gotcoff;
	} else
#endif
#ifdef	__alpha
	if (exdata.coff_hdr.magic == ALPHAMAGIC) {
		aouthdr_offset = FILHSZ;
		goto gotcoff;
	} else
#endif
#ifdef	__mc68000
	if (exdata.coff_hdr.magic == MC68MAGIC) {
		aouthdr_offset = FILHSZ;
		goto gotcoff;
	} else
#endif
#endif	/* SYSV_COFF */

	{
		/* not recognized; may be ascii */
		error = ENOEXEC;
		goto bad;
	}

	/* we only came here via explicit goto; if the magic number was
	 * recognized, there was always a goto
	 */
call_exec_loader:
		/*
		 * Either one of the debugging flags was on, instructing
		 * us to use the user-space loader, or the file was too
		 * complicated to load in the kernel.  Sigh, must finish
		 * with vnode and eventually re-do all the 
		 * processing we've done so far, until we get a 
		 * better interface into the internals of
		 * exec_load_loader().  It does everything, so 
		 * simply return when it completes.
		 */
			    
		vrele(vp);
                error = exec_load_loader(p, 0, (char *)0, uap->fname,
			uap->argp, uap->envp, &u.u_file_state);

		/* map exec_load_loader() errors */
		switch (error) {

		default:
		case ENOENT:
			error = ENOEXEC;
			break;

		case ESUCCESS:
/*
                 * OSF/1 version of Clocks and Timers piggyback the 
                 * BSD ITIMER_REAL timer so they implemented a flag to
                 * distingish between the 2. The OSF implementation is to D9.
                 * Digital has implemented P1003.4 Timers as separate timers
                 * than those supplied by BSD.
*/              
     			if (p->p_realtimer_coe)	/* ITIMER_REAL_COE HACK XXXXX */
				(void)clear_p_realtimer(p);

#if RT_TIMER
	         /* 
                  * P1003.4 specifies timers shouldn't be inherited across
                  * an exec.
                  */
			(void) clear_psx4_timers(p);
#endif /* RT_TIMER */

#if RT_PML
	         /* 
                  * P1003.4 specifies proces memory locking shouldn't be 
                  * inherited an exec.
                  */

           	        u.u_lflags &= ~(UL_ALL_FUTURE|UL_STKLOCK|
			              UL_DATLOCK|UL_TXTLOCK|UL_PROLOCK);
#endif /* RT_PML */

		case E2BIG:
			/* leave error as is */
			break;

		}

		if( error != ESUCCESS && vpsaved != NULL)
		{
			vrele(p->task->procfs.pr_exvp);
			p->task->procfs.pr_exvp = NULLVP;
		}


		return(error);

#if	SYSV_COFF
gotcoff:
	/*
	 * check for entry point in text segment.
	 */
	if ((ahdr.entry < ahdr.text_start) ||
	    (ahdr.entry > ahdr.text_start+ahdr.tsize)) {
		error = ENOEXEC;
		goto bad;
	}
	if (coff_uses_loader) goto call_exec_loader;

#if defined mips || defined __alpha
	/* See if this object uses or is a shared library */

	if (exdata.coff.fhdr.f_flags & F_MIPS_SHARABLE)
		goto call_exec_loader;
#endif

	/*
	 * Now check the second (a.out) header for segment sizes
	 * and magic number:
	 *	OMAGIC = plain executable
	 *	NMAGIC = RO text
	 *	ZMAGIC = demand paged RO text
	 *
	 * XXX On some machines, OMAGIC here does not
	 * XXX mean what OMAGIC means under BSD_A_OUT.  This code
	 * XXX may need to be fixed for those machines.
	 */
	docoff = 1;

#if	defined(mips) || defined(__hp_osf) || defined (__alpha)
	/*
	 * check for unaligned entry point
	 */
	if (ahdr.entry & (sizeof(int)-1)) {
		error = ENOEXEC;
		goto bad;
	}
#endif

	switch (ahdr.magic) {
	    case OMAGIC: /* XXX */
#if	defined(mips) || defined(__hp_osf) || defined (__alpha)
		/* We do it right:
		 * Do not make text read-only, e.g. put it in the data section
		 * Note that by definition text and data are contiguous both
		 * in the file and in memory.
		 */
		ahdr.data_start = ahdr.text_start;
		ahdr.dsize += ahdr.tsize;
		ahdr.tsize = 0;
		break;
#endif
	    case NMAGIC:
	    case ZMAGIC:
		if (ahdr.tsize == 0) {
			error = ENOEXEC;
			goto bad;
		}
		break;

	    default:
		error = ENOEXEC;
		goto bad;
	}
#if defined(mips) || defined(__alpha)
	/*
	 * MIPS:
	 * Enforce (artificial) addressability limit: this covers
	 * a chip bug.
	 * Alpha:
	 * Linking to first 64KB is prohibited.
	 */
	if ((ahdr.text_start < VM_MIN_ADDRESS) || 
	      (ahdr.data_start < VM_MIN_ADDRESS)) {
		error = ENOEXEC;
		goto bad;
	}
#endif
#ifdef	multimax
	/*
	 *	XXX Alignment flags don't get set in N16 fileheaders.
	 */
	if (aouthdr_offset == N16FILHSZ)
		ahdr.flags |= U_AL_1024;
#endif
#endif	/* SYSV_COFF */

gotobject:

	/*
	 * Collect arguments and lock vnode/inode.
	 */
	if (error = exec_args_collect((char *)0, UIO_SYSSPACE, 
		uap->fname, uap->argp, uap->envp, shell_name, shell_name_tail, 
		shell_arg, &exec_args, &na, &ne, &nc))
			goto bad;

#ifdef	sun
	/*
	 *	Save a.out header for Sun debuggers
	 */
	current_thread()->pcb->pcb_exec = exdata.ex_exec;
#endif
	/*
	 * We need to account for our reference on the credentials before
	 * calling the getxfile routine.  The getxfile functions wind up
	 * doing three decrements on the credentials.  So we bump the 
	 * reference count here.
	 */
	 crhold(u.u_cred);

#if	SYSV_ELF
	if (do_elf)
		error = elf_getxfile(p, vp, &exdata.ehdr,
		    nc + (na+4)*NBPW, uid, gid, is_priv, u.u_cred);
#endif

#if	SYSV_COFF
	if (docoff) {
#ifdef	__alpha
		/* Now committed to the next image; set the stack pointer. */
		U_HANDY_LOCK();
		USRSTACK = ahdr.text_start;
		U_HANDY_UNLOCK();
#endif	/* __alpha */
		error = coff_getxfile(p, vp, &exdata.coff.fhdr, &ahdr,
			 (long) (nc + (na+4)*NBPW),
			 (long) uid, (long) gid, is_priv, u.u_cred);
	}
#endif

#if	OSF_MACH_O
	if (do_o_mach_o) 
  		error = o_mach_o_getxfile(p, vp, &mo_header, &entry_addr,
			nc + (na+4)*NBPW, uid, gid, is_priv, u.u_cred);
#endif

#if	BSD_A_OUT
	if (do_bsd_a_out)
#ifdef	sparc
	/*
	 * Make sure user register windows are empty before attempting to
	 * make a new stack.
	 */
		{
			flush_user_windows();
			error = getxfile(p, vp, &exdata.ex_exec,
				SA(nc + (na+4)*NBPW + sizeof(struct rwindow)), 
				 uid, gid, is_priv, u.u_cred);
		}
#else
		error = getxfile(p, vp, &exdata.ex_exec, nc + (na+4)*NBPW, uid, 
				 gid, is_priv, u.u_cred);
#endif	/* sparc */
#endif	/* BSD_A_OUT */

	if (error)
		goto bad;
	vrele(vp);
	vp = NULL;

	/*
	 * Copy back arglist.
	 */
	error = exec_args_copyback((long)0, (char *)0, exec_args, na, ne, nc, 0, audptr);
	if (error)
		goto bad;

	unix_master();
	execsigs(u.u_procp);
	unix_release();

	/*
	 * We take a lock here to be extra safe.  We didn't kill all
	 * of the other threads in the task.  This semantic is a bit
	 * strange; we should look into killing all the other threads
	 * on exec.
	 */
	U_FDTABLE_LOCK(ufp);
	for (nc = ufp->uf_lastfile; nc >= 0; --nc) {
		if (U_POFILE(nc, ufp) & UF_EXCLOSE) {
			(void)closef(U_OFILE(nc, ufp));
			U_OFILE_SET(nc, NULL, ufp);
			U_POFILE_SET(nc, 0, ufp);
		}
		U_POFILE_SET(nc, U_POFILE(nc, ufp) & ~UF_MAPPED, ufp);
	}
	while (ufp->uf_lastfile >= 0 && (U_OFILE(ufp->uf_lastfile, ufp)== NULL))
		ufp->uf_lastfile--;
	U_FDTABLE_UNLOCK(ufp);
#if	SYSV_ELF
	if (do_elf)
		setregs(exdata.ehdr.e_entry);
#endif	/* SYSV_ELF */

#if	SYSV_COFF
	if (docoff)
#ifdef	multimax
		setregs(ahdr.entry, ahdr.mod_start);
#endif
#if	defined(i386) || defined(mips) || defined (__hp_osf) || defined (__alpha)
		setregs(ahdr.entry);
#endif

#endif	/* SYSV_COFF */

#if	OSF_MACH_O
	if (do_o_mach_o)
#ifdef	multimax
#define	MMAX_MOD_START	0x2000
	setregs(entry_addr, MMAX_MOD_START);
#else
	setregs(entry_addr);
#endif
#endif	/* OSF_MACH_O */

#if	BSD_A_OUT
	if (do_bsd_a_out)
	        setregs(exdata.ex_exec.a_entry);
#endif

#ifdef	vax
	{
		/*
		 *	This belongs in vax.setregs()
		 */
		extern int nsigcode[5];

		bcopy((caddr_t)nsigcode,
		      (caddr_t)(VM_MAX_ADDRESS - sizeof(nsigcode)),
		      sizeof(nsigcode));
	}
#endif
#ifdef	ibmrt
	{
	    	/*
		 *	sigcode[] must agree with declaration in pcb.h
		 *
		 *	sigcode goes at the bottom of the user_stack,
		 *	where, of course, the user's stack can grow
		 *	down on top of it, but this seems unlikely.
		 *	Putting it at the top makes ps(1) unhappy.
		 */
		extern int sigcode[3];
		bcopy((caddr_t)sigcode,
		      (caddr_t)SIGCODE_ADDRESS,
		      sizeof(sigcode));
	}
#endif

	/*
	 * Remember file name for accounting.
	 */
	u.u_acflag.fi_flag &= ~AFORK;
	if (shell_name)
		bcopy((caddr_t)shell_name_tail, (caddr_t)u.u_comm, MAXCOMLEN);
	else {
		if (ndp->ni_dent.d_namlen > MAXCOMLEN)
			ndp->ni_dent.d_namlen = MAXCOMLEN;
		bcopy((caddr_t)ndp->ni_dent.d_name, (caddr_t)u.u_comm,
		    (unsigned)(ndp->ni_dent.d_namlen + 1));
	}
/*
                 * OSF/1 version of Clocks and Timers piggyback the 
                 * BSD ITIMER_REAL timer so they implemented a flag to
                 * distingish between the 2. The OSF implementation is to D9.
                 * Digital has implemented P1003.4 Timers as separate timers
                 * than those supplied by BSD.
*/
     			if (p->p_realtimer_coe)	/* ITIMER_REAL_COE HACK XXXXX */
				(void)clear_p_realtimer(p);

#if RT_TIMER
	         /* 
                  * P1003.4 specifies timers shouldn't be inherited across
                  * an exec.
                  */
			(void) clear_psx4_timers(p);
#endif /* RT_TIMER */

#if RT_PML
	         /* 
                  * P1003.4 specifies proces memory locking shouldn't be 
                  * inherited an exec.
                  */

           	        u.u_lflags &= ~(UL_ALL_FUTURE|UL_STKLOCK|
			              UL_DATLOCK|UL_TXTLOCK|UL_PROLOCK);
#endif /* RT_PML */

#if BIN_COMPAT
	/* 
	 * See if this executable is serviced by a compatability module
	 */
	cm_recognizer(&exdata.coff.fhdr,&ahdr, vp);
#endif /*BIN_COMPAT*/
bad:
	/* generate audit record */
	if ( DO_AUDIT(u.u_event,error) ) {
		/* may need to fetch filename */
		if ( (audptr[1] = (char *)exec_args) == NULL ) {
			if ( audptr[0] = (char *)kalloc(MAXPATHLEN) )
				copyinstr ( uap->fname, audptr[0], MAXPATHLEN, &audlen );
		}

		/* option to record arglist */
		if ( audptr[2] && audptr[1] ) {
			/* empty arglist */
			if ( audptr[2] == audptr[1] ) audptr[1] = NULL;
			else if ( (audstyle&AUD_EXEC_ARGP) == 0 ) audptr[1] = NULL;
			else if ( audptr[2]-audptr[1] > AUD_BUF_SIZ )
				*(audptr[1]+AUD_BUF_SIZ) = '\0';
		}

		/* option to record environment variables */
		if ( audptr[0] && audptr[2] ) {
			/* empty environment list */
			if ( audptr[0] == audptr[2] ) audptr[2] = NULL;
			else if ( (audstyle&AUD_EXEC_ENVP) == 0 ) audptr[2] = NULL;
			else if ( audptr[3] && audptr[3]-audptr[2] > AUD_BUF_SIZ )
				*(audptr[2]+AUD_BUF_SIZ) = '\0';
		}

		audit_rec_build ( u.u_event, audptr, error, *retval, (int *)0, AUD_HPR, (char *)0 );
		if ( audlen ) kfree ( audptr[0], MAXPATHLEN );
	}

	if (exec_args)
		exec_args_free(exec_args);
	if(vp)
		vrele(vp);

	if(error && vpsaved != NULL)
	{
		vrele(p->task->procfs.pr_exvp);
		p->task->procfs.pr_exvp = NULLVP;
	}

	if (error == EGETXFILE) {
		struct proc	*p;
		/* 
		 *	getxfile failed, kill the current process.
		 *	Send SIGKILL, blow away other pending signals.
		 */
		p = u.u_procp;
		unix_master();
		s = splhigh();
		p->p_sig = sigmask(SIGKILL);
		p->p_cursig = SIGKILL;
		u.u_sig = (sigset_t)0;
		u.u_cursig = 0;
		splx(s);
		psig();		/* Bye */
		unix_release();
		return (error);
	}
	return (error);
}

#if	BSD_A_OUT
/*
 * Read in and set up memory for executed file.
 */
getxfile(p, vp, ep, nargc, uid, gid, is_priv, cred)
	register struct proc *p;
	register struct vnode *vp;
	struct exec *ep;
	long nargc, uid, gid;
	boolean_t is_priv;
	struct ucred *cred;
{
	size_t ts, ds, ss;
	int pagi;
	vm_size_t	text_size, data_size;
	struct ucred    *newcr;
	int		pflag, error;

	if (ep->a_magic == ZMAGIC)
		pagi = SPAGV;
	else
		pagi = 0;

	/*
	 * Compute text and data sizes and make sure not too large.
	 * NB - Check data and bss separately as they may overflow 
	 * when summed together.
	 */
	text_size = loader_round_page(ep->a_text);	/* bytes */
	ts = btoc(text_size);				/* machine pages */
#ifdef __hp_osf
        /* This fixes a bug.  However, I know of no other machine where
        the loader page size is larger than the VM page size.  So, I can't
        properly test this.  So, for now the bug is fixed only on HP */
	data_size = round_page(ep->a_data + ep->a_bss);
#else
	data_size = loader_round_page(ep->a_data + ep->a_bss);
#endif

							/* bytes */
	ds = btoc(data_size);				/* machine pages */
	ss = SSIZE + btoc(loader_round_page(nargc));
	PROC_LOCK(p);
	p->p_flag &= ~(SPAGV|SSEQL|SUANOM|SXONLY);
	PROC_UNLOCK(p);
	VOP_ACCESS(vp, VREAD, u.u_cred, error);
	PROC_LOCK(p);
	if (error) {
		p->p_flag |= SXONLY;
		p->p_flag &= ~STRC;
		error = 0;
	}
	p->p_flag |= pagi | SEXEC;
	PROC_UNLOCK(p);

	(void) vm_exec(is_priv);

	error = program_loader(vp, ep, pagi);
#if	SEC_BASE
	if (security_is_on)
#if	SEC_PRIV
	compute_subject_privileges(vp, p->p_flag & STRC);
#else	/* SEC_PRIV */
	compute_subject_privileges(vp, 
		(p->p_flag & STRC || (! SEC_SUSER(uid,gid))));
#endif	/* SEC_PRIV */
#endif	/* SEC_BASE */
	/*
	 * set SUID/SGID protections, if no tracing
	 */
#if	SEC_ARCH
	BM(PROC_LOCK(p));
	pflag = p->p_flag;
	BM(PROC_UNLOCK(p));
	if ((pflag&STRC)==0) {
		if (uid != u.u_cred->cr_uid || gid != u.u_cred->cr_gid) {
			newcr = crcopy(u.u_cred);
			newcr->cr_uid = uid;
			newcr->cr_gid = gid;
			substitute_real_creds(p, NOCRED, newcr->cr_uid, NOCRED,
				      newcr->cr_gid, newcr);
			SP_CHANGE_SUBJECT();
		}
		if(p->p_pr_qflags & PRFS_STOPEXEC) {
		   	/* If the stop-on-exec bit is set, stop the task */
			if(!prismember(&p->task->procfs.pr_sigtrace, SIGTRAP)) {
				praddset(&p->task->procfs.pr_sigtrace, SIGTRAP);
				p->task->procfs.pr_qflags |= PRFS_SIGNAL;
			}
			unix_master();
			psignal(p, SIGTRAP);
			unix_release();
		}
	} else {
		/*
		 * exec gave us an additional reference on u.u_cred
		 * We should in fact be using the passed in creds but
		 * for historical reasons we aren't.
		 */
		crfree(u.u_cred);

		/*
		 * This software exception will be mapped to a SIGTRAP
		 * signal with the siginfo si_code field of TRAP_TRACE.
		 */
		thread_doexception(current_thread(), EXC_BREAKPOINT, 0L,
				   EXC_ALPHA_TRACE );
	}
#else /* SEC_ARCH */
	newcr = crcopy(u.u_cred);
	BM(PROC_LOCK(p));
	pflag = p->p_flag;
	BM(PROC_UNLOCK(p));
	if ((pflag&STRC)==0) {
		newcr->cr_uid = uid;
		newcr->cr_gid = gid;
		if(p->p_pr_qflags & PRFS_STOPEXEC) {
		   	/* If the stop-on-exec bit is set, stop the task */
			if(!prismember(&p->task->procfs.pr_sigtrace, SIGTRAP)) {
				praddset(&p->task->procfs.pr_sigtrace, SIGTRAP);
				p->task->procfs.pr_qflags |= PRFS_SIGNAL;
			}
			unix_master();
			psignal(p, SIGTRAP);
			unix_release();
		}
	} else {

		/*
		 * This software exception will be mapped to a SIGTRAP
		 * signal with the siginfo si_code field of TRAP_TRACE.
		 */
		thread_doexception(current_thread(), EXC_BREAKPOINT, 0L,
				   EXC_ALPHA_TRACE);
	}
	substitute_real_creds(p, NOCRED, newcr->cr_uid, NOCRED,
			      newcr->cr_gid, newcr);
#endif	/* SEC_ARCH */
	U_HANDY_LOCK();
	u.u_tsize = ts;
	u.u_dsize = ds;
	u.u_ssize = ss;
	u.u_prof.pr_scale = 0;
	U_HANDY_UNLOCK();
bad:
	return (error);
}
#endif	/* BSD_A_OUT */

#if	SYSV_COFF
/*
 * Version of getxfile for machines that use the Common Object File Format.
 * Some vendors have their own idea of what COFF is supposed to be, so
 * we have still machine-dependencies here.
 */
coff_getxfile(p, vp, fhd, ap, nargc, uid, gid, is_priv, cred)
	register struct proc *p;
	register struct vnode *vp;
	struct filehdr *fhd;
	struct aouthdr *ap;
	long nargc, uid, gid;
	boolean_t is_priv;
	struct ucred *cred;
{
	size_t 		ts, ds, ss;
	int 		pagi;
	vm_offset_t	addr;
	vm_size_t	size;
	vm_map_t	my_map;
	vm_offset_t	offset;
	vm_offset_t	vm_text_start, vm_text_end;
	vm_offset_t	vm_data_start, vm_data_end;
	vm_offset_t	vm_end;
	struct ucred	*newcr;
	int		pflag, error;

#ifdef	multimax
	vm_offset_t	sectalign;

	/*
	 *	Page size used by loader is encoded in flags field.
	 *	Only 1024 and 4096 are supported currently.
	 */
	if (ap->flags & U_AL_1024)
		sectalign = 1024;
	else if (ap->flags & U_AL_4096)
		sectalign = 4096;
	else {
		error = ENOEXEC;
		goto bad;
	}

#define SECTALIGN		sectalign
#define NOPAGI_TEXT_OFFSET	SECTALIGN
#define NOPAGI_DATA_OFFSET	(SECTALIGN + ap->tsize)
#define PAGI_TEXT_OFFSET	SECTALIGN
#define PAGI_DATA_OFFSET	(SECTALIGN + loader_round_page(ap->tsize))
#endif	/* multimax */

#ifdef	i386
#define NOPAGI_TEXT_OFFSET	((fhd->f_nscns * sizeof(struct scnhdr)) + \
				 sizeof(struct filehdr) + fhd->f_opthdr)
#define NOPAGI_DATA_OFFSET	(NOPAGI_TEXT_OFFSET + ap->tsize)
#define PAGI_TEXT_OFFSET	0
#define PAGI_DATA_OFFSET	trunc_page(ap->text_start + ap->tsize)
#endif	/* i386 */

#ifdef	__hp_osf
#define NOPAGI_TEXT_OFFSET	((fhd->f_nscns * sizeof(struct scnhdr)) + \
				 sizeof(struct filehdr) + fhd->f_opthdr)
#define NOPAGI_DATA_OFFSET	(NOPAGI_TEXT_OFFSET + ap->tsize)
#define PAGI_TEXT_OFFSET	SECTALIGN
#define PAGI_DATA_OFFSET	(PAGI_TEXT_OFFSET + ap->tsize)
#endif	/* __hp_osf */

#ifdef	mips
#define SECTALIGN		((vm_offset_t)4096)
#define NOPAGI_TEXT_OFFSET	N_TXTOFF(*fhd, *ap)
#define NOPAGI_DATA_OFFSET	(N_TXTOFF(*fhd, *ap) + ap->tsize)
#define PAGI_TEXT_OFFSET	NOPAGI_TEXT_OFFSET
#define PAGI_DATA_OFFSET	NOPAGI_DATA_OFFSET
#endif	/* mips */

#ifdef	__alpha
#define SECTALIGN		((vm_offset_t)8192)
#define NOPAGI_TEXT_OFFSET	N_TXTOFF(*fhd, *ap)
#define NOPAGI_DATA_OFFSET	(N_TXTOFF(*fhd, *ap) + ap->tsize)
#define PAGI_TEXT_OFFSET	NOPAGI_TEXT_OFFSET
#define PAGI_DATA_OFFSET	MAX(0, trunc_page(NOPAGI_DATA_OFFSET))
#endif	/* alpha */

	/*
	 *	Check pageability.
	 */
	if (ap->magic == ZMAGIC)
		pagi = SPAGV;
	else
		pagi = 0;

	/*
	 * Compute text, data and stack sizes.
	 */

	ts = btoc(loader_round_page(ap->tsize));
#ifdef __hp_osf
        /* This fixes a bug.  However, I know of no other machine where
        the loader page size is larger than the VM page size.  So, I can't
        properly test this.  So, for now the bug is fixed only on HP */
	ds = btoc(round_page(ap->bsize + ap->dsize));
#else
	ds = btoc(loader_round_page(ap->bsize + ap->dsize));
#endif
	ss = SSIZE + btoc(loader_round_page(nargc));

	PROC_LOCK(p);
	p->p_flag &= ~(SPAGV|SSEQL|SUANOM|SXONLY);
	PROC_UNLOCK(p);
	VOP_ACCESS(vp, VREAD, u.u_cred, error);
	PROC_LOCK(p);
	if (error) {
		p->p_flag |= SXONLY;
		p->p_flag &= ~STRC;
		error = 0;
	}
	p->p_flag |= pagi | SEXEC;
	PROC_UNLOCK(p);


	my_map = current_task()->map;
	/*
	 *	Even if we are execing the same image (the RFS server
	 *	does this, for example), we do not have to unlock the
	 *	vnode; deallocating it does not require it to be locked.
	 */
	(void) vm_exec(is_priv);
	
	/*
	 *	Allocate low-memory stuff: text, data, bss.
	 *	Read text and data into lowest part, then make text read-only.
	 */

	/*
	 *	Remember where text and data start.
	 */
	U_HANDY_LOCK();
	u.u_text_start = (caddr_t) ap->text_start;
	u.u_data_start = (caddr_t) trunc_page(ap->data_start);
	U_HANDY_UNLOCK();

	/*
	 *	Note vm boundaries for data and text segments.  If data
	 *	and text overlap a page, that is considered data.
	 *
	 */
	vm_text_start = trunc_page(ap->text_start);
	vm_text_end = round_page(ap->text_start + ap->tsize);
	vm_data_start = trunc_page(ap->data_start);
	vm_data_end = round_page(ap->data_start + ap->dsize);
	vm_end = round_page(ap->data_start + ap->dsize + ap->bsize);
#ifdef	i386
	ds = btoc(round_page(ap->data_start + ap->dsize + ap->bsize) - vm_data_start);
#endif
	if (vm_text_end > vm_data_start)
		vm_text_end = vm_data_start;

	error = 0;

	if (pagi == 0) {
		/*
		 * Not demand paged.
		 * In OMAGIC images the code is not read-protected.
		 */
		/*
		 * Allocate and read in the data segment (OMAGIC & NMAGIC).
		 */
		if (ap->dsize <= 0)
			goto read_text;	/* pure text and sanity */
		addr = vm_data_start;
		size = vm_end - vm_data_start;	/* include bss */
		if (vm_allocate(my_map, &addr, size, FALSE) != KERN_SUCCESS) {
			uprintf("%s: Data section too big.\n", u.u_comm);
			goto suicide;
		}

		/*
		 *	Read in the data segment (OMAGIC & NMAGIC).  It goes on
		 *	the next loader_page boundary after the text.
		 */
		error = vn_rdwr(UIO_READ, vp,
				(caddr_t) ap->data_start, (int) ap->dsize,
				NOPAGI_DATA_OFFSET, UIO_USERSPACE, IO_UNIT,
				u.u_cred, (int *)0);
		if (error) {
			uprintf("%s: Cannot read data.\n", u.u_comm);
			goto suicide;
		}
		if (ap->magic == OMAGIC) /* Must explicitly allow execute. */
			(void)vm_protect(my_map, vm_data_start,
					 vm_data_end - vm_data_start, FALSE,
					 VM_PROT_ALL);
		/*
		 *	Allocate and read in text segment, and read-protect it
		 *	if necessary (NMAGIC).
		 */
read_text:
		if (ap->tsize > 0) {
			addr = vm_text_start;
			size = vm_text_end - vm_text_start;
			if (vm_allocate(my_map, &addr, size, FALSE)
			    != KERN_SUCCESS) {
				uprintf("%s: Text section too big.\n",
					u.u_comm);
				goto suicide;
			}
			error = vn_rdwr(UIO_READ, vp,
				(caddr_t) ap->text_start, (int) ap->tsize,
				NOPAGI_TEXT_OFFSET, UIO_USERISPACE, IO_UNIT,
				u.u_cred, (int *) 0);
			if (error) {
				uprintf("%s: Cannot read text.\n", u.u_comm);
				goto suicide;
			}
			if (ap->magic != OMAGIC) {
				(void)vm_protect(my_map, vm_text_start,
					vm_text_end - vm_text_start, FALSE,
					VM_PROT_READ|VM_PROT_EXECUTE);
			}
		}
	} else {


		/*
		 *	Map the text segment.
		 */
		if (ap->tsize <= 0)
			goto map_data;	/* sanity */
		VOP_MMAP(vp, PAGI_TEXT_OFFSET, my_map, &vm_text_start,
			vm_text_end - vm_text_start, 
			VM_PROT_READ|VM_PROT_EXECUTE,
			VM_PROT_ALL,
			MAP_PRIVATE|MAP_FIXED, u.u_cred, error); 
		if (error) {
				uprintf("%s: Cannot map text.\n",u.u_comm);
				goto suicide;
		}
		/*
		 *	Map the data segment, if any.
		 */
map_data:
		if (vm_data_end > vm_data_start) {
			VOP_MMAP(vp, PAGI_DATA_OFFSET, my_map,
				&vm_data_start,
			    	vm_data_end - vm_data_start,
			    	VM_PROT_READ|VM_PROT_WRITE,
			    	VM_PROT_ALL,
				MAP_PRIVATE|MAP_FIXED, u.u_cred, error); 
			if (error) {
				uprintf("%s: Cannot map data.\n", u.u_comm);
				goto suicide;
			}
		}


		/*
		 *	Allocate bss.  First check whether any more is needed.
		 */

		
		size =  vm_end - vm_data_end;
		if ( (long)size > 0 ) {			/* == missing     */
			addr = vm_data_end;
			if (vm_allocate(my_map, &addr, size, FALSE)
			    != KERN_SUCCESS) {
				    uprintf("%s: Cannot allocate space for bss.\n", u.u_comm);
				    goto suicide;
			}
			vm_protect(my_map, addr, size, FALSE, VM_PROT_READ|VM_PROT_WRITE);
		}

		/*
		 *	If the data segment does not end on a VM page
		 *	boundary, we have to clear the remainder of the VM
		 *	page it ends on so that the bss segment will
		 *	(correctly) be zero.
		 */
		addr = ap->data_start + ap->dsize;
		if (vm_data_end > addr) {
			{ vm_size_t num_zeroes;
			  do {
				num_zeroes = MIN( PAGE_SIZE, vm_data_end - addr);
				if (copyout(vm_kern_zero_page, (caddr_t)addr,  num_zeroes)) {
					uprintf("%s: Cannot zero front of BSS.\n", u.u_comm);
					goto suicide;
				}
				addr += num_zeroes;
			  } while (addr < vm_data_end);
			}
		}
	}

	/*
	 *	Create the stack.
	 */

	size = round_page(stackinc);
	U_HANDY_LOCK();

#ifdef  __alpha
        /*
         *  VM_MAX_ADDRESS is not the correct thing here.  It appears to
         *  work in non-alpha implementations because VM_MAX_ADDRESS and
         *  USRSTACK are more or less the same.  For alpha they are
         *  very different.
         */
        u.u_stack_start = (caddr_t) (addr = trunc_page(USRSTACK - size));
#else
	u.u_stack_start = (caddr_t) (addr = trunc_page(VM_MAX_ADDRESS - size));
	u.u_stack_end = u.u_stack_start + size;
#endif /* alpha */

	u.u_stack_grows_up = FALSE;
	U_HANDY_UNLOCK();
	if (vm_allocate(my_map, &addr, size, FALSE) != KERN_SUCCESS) {
		uprintf("%s: Cannot find space for stack.\n", u.u_comm);
		goto suicide;
	}

#if	SEC_BASE
	if (security_is_on)
#if	SEC_PRIV
	compute_subject_privileges(vp, p->p_flag & STRC);
#else	/* SEC_PRIV */
	compute_subject_privileges(vp, 
		(p->p_flag & STRC || (! SEC_SUSER(uid,gid))));
#endif	/* SEC_PRIV */
#endif	/* SEC_BASE */
	/*
	 * set SUID/SGID protections, if no tracing
	 */
#if	SEC_ARCH
	BM(PROC_LOCK(p));
	pflag = p->p_flag;
	BM(PROC_UNLOCK(p));
	if ((pflag&STRC)==0) {
	    if (uid != u.u_cred->cr_uid || gid != u.u_cred->cr_gid) {
		newcr = crcopy(u.u_cred);
		newcr->cr_uid = uid;
		newcr->cr_gid = gid;
		substitute_real_creds(p, NOCRED, newcr->cr_uid, NOCRED,
			      newcr->cr_gid, newcr);
		SP_CHANGE_SUBJECT();
	    }
		if(p->p_pr_qflags & PRFS_STOPEXEC) {
#ifdef	mips
			register thread_t th = current_thread();
			th->pcb->trapcause = CAUSEEXEC;
#endif
		   	/* If the stop-on-exec bit is set, stop the task */
			if(!prismember(&p->task->procfs.pr_sigtrace, SIGTRAP)) {
				praddset(&p->task->procfs.pr_sigtrace, SIGTRAP);
				p->task->procfs.pr_qflags |= PRFS_SIGNAL;
			}
			unix_master();
			psignal(p, SIGTRAP);
			unix_release();
		}
	} else {
#ifdef	mips
		current_thread()->pcb->trapcause = CAUSEEXEC;
#endif
		/*
		 * exec gave us an additional reference on u.u_cred
		 * We should in fact be using the passed in creds but
		 * for historical reasons we aren't.
		 */
		crfree(u.u_cred);

		/*
		 * This software exception will be mapped to a SIGTRAP
		 * signal with the siginfo si_code field of TRAP_TRACE.
		 */
		thread_doexception(current_thread(), EXC_BREAKPOINT, 0L,
				   EXC_ALPHA_TRACE);
	}
#else /* SEC_ARCH */
	newcr = crcopy(cred);
	BM(PROC_LOCK(p));
	pflag = p->p_flag;
	BM(PROC_UNLOCK(p));
	if ((pflag&STRC)==0) {
		newcr->cr_uid = uid;
		newcr->cr_gid = gid;
	    if(p->p_pr_qflags & PRFS_STOPEXEC) {
#ifdef mips
		register thread_t th = current_thread();
		th->pcb->trapcause = CAUSEEXEC;
#endif
	   	/* If the stop-on-exec bit is set, stop the task */
		if(!prismember(&p->task->procfs.pr_sigtrace, SIGTRAP)) {
			praddset(&p->task->procfs.pr_sigtrace, SIGTRAP);
			p->task->procfs.pr_qflags |= PRFS_SIGNAL;
		}
		unix_master();
		psignal(p, SIGTRAP);
		unix_release();
	    }
	} else {
#ifdef	mips
		current_thread()->pcb->trapcause = CAUSEEXEC;
#endif

		/*
		 * This software exception will be mapped to a SIGTRAP
		 * signal with the siginfo si_code field of TRAP_TRACE.
		 */
		thread_doexception(current_thread(), EXC_BREAKPOINT, 0L,
				   EXC_ALPHA_TRACE);
	}
	substitute_real_creds(p, NOUID, newcr->cr_uid, NOGID,
			      newcr->cr_gid, newcr);
#endif	/* SEC_ARCH */
	U_HANDY_LOCK();
	u.u_tsize = ts;
	u.u_dsize = ds;
	u.u_ssize = ss;
	u.u_prof.pr_scale = 0;
	U_HANDY_UNLOCK();
bad:
	return (error);
suicide:
	return (EGETXFILE);
}
#endif	/* SYSV_COFF */

#if	OSF_MACH_O
/* Version of getxfile for the OSF/mach-o object file format. */

o_mach_o_getxfile (p, vp, mhp, entryp, nargc, uid, gid, is_priv, cred)
	register struct proc *p;
        register struct vnode	*vp;
        mo_header_t		*mhp;
        long			*entryp;
        long			nargc, uid, gid;
        boolean_t		is_priv;
        struct ucred		*cred;
{
	caddr_t		lc_bufferp = NULL;
	load_cmd_map_command_t	*ld_map_cmdp;
	ldc_header_t	*lcp;
	entry_command_t *ent_lcp;
	long		entry_tmp=0;
	region_command_t  *reg_lcp;
	int		i;
	int		error=0;
	int		resid;
	int		nentries;
	int		region_type;
	int		last_region_id;
	int		nregions;
	int		bad_omo_format;
	int		saw_writeable_region;
	int		saw_nonwriteable_region;
	char		*p1;
	char		*p2;
	vm_offset_t	highest_addr = 0;
	vm_offset_t	lowest_addr = 0;
	vm_offset_t	lowest_w_addr = 0;
	vm_offset_t	highest_nonw_addr = 0;
	vm_prot_t	region_prot;
	vm_size_t	vm_region_size;
	vm_size_t	file_size;
	vm_size_t	vm_mapped_size;
	int		pagi, pflag;
	vm_offset_t	addr;
	vm_size_t	size;
	vm_map_t	my_map;
	vm_offset_t	offset;
	vm_offset_t	vm_region_start;
	vm_offset_t	vm_region_end;
	struct vattr	vattr;
	struct ucred	*newcr;

#define LOAD_CMD_P(cmd_off) (lc_bufferp - mhp->moh_first_cmd_off + cmd_off)


	/* 
	 * Check that the page size linked for will work here.
	 * Eventually, the linked page size must be an integral
	 * multiple of the current VM page size.
	 * However, for now they must be equal so that object files
	 * converted from other formats will be mapped correctly.
	 */

	if (mhp->moh_max_page_size != PAGE_SIZE) {
		error = ENOEXEC;
		goto out;
	}

	pagi = SPAGV;		/* assume all mach-o files can be mapped */

	PROC_LOCK(p);
	p->p_flag &= ~(SPAGV|SSEQL|SUANOM|SXONLY);
	PROC_UNLOCK(p);
	VOP_ACCESS(vp, VREAD, u.u_cred, error);
	PROC_LOCK(p);
	if (error) {
		p->p_flag |= SXONLY;
		p->p_flag &= ~STRC;
		error = 0;
	}
	p->p_flag |= pagi | SEXEC;
	PROC_UNLOCK(p);


	/* read in load commands */

	lc_bufferp = kalloc((long)(mhp->moh_sizeofcmds));
	if (lc_bufferp == NULL) {
		error = ENOEXEC;
		goto out;
	}

	error = vn_rdwr(UIO_READ, vp, lc_bufferp, 
			    (long)(mhp->moh_sizeofcmds), 
			    (off_t)(mhp->moh_first_cmd_off),
			    UIO_SYSSPACE, IO_UNIT, u.u_cred, &resid);

	if (error)
		goto out;
	if (resid) {
		error = ENOEXEC;
		goto out;
	}

	ld_map_cmdp = (load_cmd_map_command_t *)
		(LOAD_CMD_P(mhp->moh_load_map_cmd_off));

	/* Check that ld_map_cmdp is valid and that the load map command
	   has  reasonable values */

	if ((!VALID_LDC_HEADER_PTR(ld_map_cmdp))
	|| (ld_map_cmdp->ldc_cmd_size > (mhp->moh_sizeofcmds -
					     (mhp->moh_load_map_cmd_off -
					     mhp->moh_first_cmd_off)))) {
		error = ENOEXEC;
		goto clean_up_less;
	}

	/* check that nentries map entries fit in the space occupied by
	 * the load command map */

	nentries = ld_map_cmdp->lcm_nentries;
	p1 = (char *) ld_map_cmdp;
	p2 = p1 + ld_map_cmdp->ldc_cmd_size;

	if ((char *)(&ld_map_cmdp->lcm_map[nentries]) > p2) {
		error = ENOEXEC;
		goto clean_up_less;
	}

	/*
	 * Loop through the load commands looking for the entry point
	 * and the region commands.
	 *
	 * Look for an entry command.  If we dont find
	 * one, return ENOEXEC, since all execable files
	 * should have an entry.  If you want to change the code
	 * to call exec_load_loader instead, you should
	 * read in the load commands and check for the
	 * entry at the time you check for magic numbers,
	 * so you can call exec_load_loader BEFORE you
	 * collect the arguments.
	 *
	 * Find all the region load commands and check that we can load them
	 * all.  We need to check in a separate pass from mapping so that
	 * we can return ENOEXEC if there are format problems.  Once we
	 * have cleared our address space and begun mapping, the only error
	 * that should be returned is EGETXFILE.  In order to simplify
	 * the second pass, the first pass zeroes out all the load command
	 * map entries that don't correspond to region commands.
	 */

	/* Initialize stuff */
	/* already initialized entry_tmp and vm address range variables */

	bad_omo_format = FALSE;
	last_region_id = -1;	
	nregions = 0;
	saw_writeable_region = FALSE;
	saw_nonwriteable_region = FALSE;
	VOP_GETATTR(vp, &vattr, u.u_cred, error);	/* get file size */
	if (error != 0)
		 goto clean_up_less;

	for (i = 0; i < nentries; i++) {
		if (ld_map_cmdp->lcm_map[i] == 0) continue;
		lcp = (ldc_header_t *)(LOAD_CMD_P(ld_map_cmdp->lcm_map[i]));

		/* check the validity of the load command pointer */
		if ((ld_map_cmdp->lcm_map[i] >= (mhp->moh_sizeofcmds +
						 mhp->moh_first_cmd_off))
		    || (!VALID_LDC_HEADER_PTR(lcp))) {
			bad_omo_format = TRUE;
			break;
		}

		if (lcp->ldci_cmd_type == LDC_ENTRY) {
			ent_lcp = (entry_command_t *)lcp;
			if ((ent_lcp->entc_flags & ENT_VALID_ABSADDR_F)) {
				entry_tmp = (long)(ent_lcp->entc_absaddr);
			}
		}
		if (lcp->ldci_cmd_type != LDC_REGION) {
			ld_map_cmdp->lcm_map[i] = 0;
			continue;
		}
		reg_lcp = (region_command_t *)lcp;

		if (reg_lcp->regc_vm_size == 0) {   /* ignore null region */
			ld_map_cmdp->lcm_map[i]  = 0;
			continue;
		}

		/* do not bother to check whether region offset and size are
		 * within the file bounds -- if they aren't, they will
		 * only affect the process
		 */

		vm_region_start = (vm_offset_t)(reg_lcp->regc_vm_addr);
		vm_region_size = (vm_size_t)(round_page(reg_lcp->regc_vm_size));
		vm_region_end = vm_region_start + vm_region_size - 1;

		if ((!(reg_lcp->regc_flags & REG_ABS_ADDR_F))

			/* region MUST start on a page boundary */
		    || (vm_region_start != trunc_page(vm_region_start))

		    || (reg_lcp->regc_initprot == MO_PROT_NONE)

			/* must have read or write or execute */
		    || ((!(reg_lcp->regc_initprot & MO_PROT_READ)) &&
			(!(reg_lcp->regc_initprot & MO_PROT_WRITE)) &&
			(!(reg_lcp->regc_initprot & MO_PROT_EXECUTE)))) {
			    bad_omo_format = TRUE;
			    break;
		    }

		last_region_id = i;
		nregions++;

		 /* Now accumulate information for u.u_text and u.u_data. */

		if (nregions == 1) {
			lowest_addr = vm_region_start;
			highest_addr = vm_region_end;
		} else {if (vm_region_start < lowest_addr)
				lowest_addr = vm_region_start;
			if (vm_region_end > highest_addr)
				highest_addr = vm_region_end;
		}

		if ((reg_lcp->regc_initprot & MO_PROT_WRITE) != 0) {
			if (saw_writeable_region == FALSE) {
				lowest_w_addr = vm_region_start;
				saw_writeable_region = TRUE;
			}
			else { if (vm_region_start < lowest_w_addr)
				       lowest_w_addr = vm_region_start;
		       }
		}
		else {  /* have a non-writeable region */
			if (saw_nonwriteable_region == FALSE) {
				highest_nonw_addr = vm_region_end;
				saw_nonwriteable_region = TRUE;
			}
			else { if (vm_region_end > highest_nonw_addr)
				       highest_nonw_addr = vm_region_end;
		       }
		}

	}		/* end of first load command loop */


	if ((entry_tmp == 0) 
	    || (last_region_id < 0) 
	    || (bad_omo_format == TRUE)) {
		error = ENOEXEC;
		goto clean_up_less;
	}

	/* Now that we're done with the format checks, we can clear the
	 * address space and map the regions.
	 */

	my_map = current_task()->map;
	/*
	 *	Even if we are execing the same image (the RFS server
	 *	does this, for example), we do not have to unlock the
	 *	vnode; deallocating it does not require it to be locked.
	 */
	(void) vm_exec(is_priv);


	/* Loop through load commands again, mapping all the regions. */

	for (i = 0; i <= last_region_id; i++) {
		if (ld_map_cmdp->lcm_map[i] == 0) continue;   /* only regions are left in map */
		reg_lcp = (region_command_t *)
			(LOAD_CMD_P(ld_map_cmdp->lcm_map[i]));

		/*
		 * Calculate VM boundaries in case those specified are
		 * not on page boundaries.
		 */

		vm_region_start = (vm_offset_t)(reg_lcp->regc_vm_addr);
		file_size = (vm_size_t)(reg_lcp->ldc_header.ldci_section_len);
		vm_mapped_size = round_page(file_size);
		vm_region_size = (vm_size_t)(round_page(reg_lcp->regc_vm_size));

		/* Translate the protection attributes */

		region_prot = VM_PROT_NONE;
		if (reg_lcp->regc_initprot & MO_PROT_READ) 
			region_prot |= VM_PROT_READ;
		if (reg_lcp->regc_initprot & MO_PROT_WRITE) 
			region_prot |= VM_PROT_WRITE;
		if (reg_lcp->regc_initprot & MO_PROT_EXECUTE) 
			region_prot |= VM_PROT_EXECUTE;

		/* Map the file part of the region. */

		if (vm_mapped_size > 0) {
			VOP_MMAP(vp, (vm_offset_t)
				(reg_lcp->ldc_header.ldci_section_off),
				my_map, &vm_region_start,
				vm_mapped_size, region_prot, VM_PROT_ALL,
				MAP_PRIVATE|MAP_FIXED, u.u_cred, error); 
			if (error) {
					uprintf("%s: Cannot map region with load map id %d.\n", u.u_comm, i);
					error = EGETXFILE;
					goto clean_up_more;
			}

			/*
			 * Map the bss part of the region, if any.  If the
			 * bss part does not start on a page boundary, we
			 * first zero out the remainder of the last mapped
			 * page; this will cause a copy-on-write of that 
			 * page to be triggered.  Then any remaining bss is
			 * allocated from anonymous memory.
			 */
			if ((vm_size_t)(reg_lcp->regc_vm_size) > file_size) {			                               /* have trailing bss */
				size = vm_mapped_size - file_size;
				if (size > 0) {	    /* VOP is still unlocked */
					if (copyout(vm_kern_zero_page,
						    (caddr_t)(vm_region_start + file_size),
						    size)) {	/* no write access?? */
						uprintf("%s: Cannot zero partial page of region with id %d.\n", u.u_comm, i);
						error = EGETXFILE;
						goto clean_up_more;
					}
				/* Leave VOP unlocked until done with regions. 	*/
				}	/* end of padding last mapped page with zeroes */
			}
		}			/* end of mapping from file */

		if (vm_region_size > vm_mapped_size) {
			addr = vm_region_start + vm_mapped_size;
			size = vm_region_size - vm_mapped_size;
			if (vm_allocate(my_map, &addr, size, FALSE)
			    != KERN_SUCCESS) {
				    uprintf ("%s: Cannot allocate space for bss in region with id %d.\n", u.u_comm, i);
                                    error = EGETXFILE;
                                    goto clean_up_more;
			    }
                        vm_protect(my_map, addr, size, FALSE, region_prot);
                 }		   	/* end of allocating bss */
        }     	                   	/* end of second load command loop */


        /* Fill in fields in u_area.
	 * The approach here is to consider data as everything
	 * between the lowest writeable address and the highest 
	 * address used, and to consider text as everything 
	 * non-writeable that is lower than data.
	 *
	 * This may result in one of several anomalies, including:
	 * - the real text section not showing up in
	 *   the u-area (if it is above a writeable region),
	 * - "data" being nonwriteable (if there are no writeable regions).
	 * These are OK.  The purpose of the u-area is no longer
	 * to accurately reflect object sections, but rather
	 * 1) to make malloc (sbrk) work, which uses dsize, and
	 * 2) to make core files work well enough to be useable
	 * (even is extra stuff is copied).
	 */

	if ((saw_nonwriteable_region == FALSE) 
	    || (lowest_addr == lowest_w_addr)) {	/* no "text" */
		u.u_text_start = (caddr_t) 0;
		u.u_tsize = (size_t)(btoc(0));
	} else {
		u.u_text_start = (caddr_t) lowest_addr;
		(highest_nonw_addr < lowest_w_addr) ?
			(u.u_tsize = (size_t)(btoc(highest_nonw_addr - lowest_addr + 1))) :
				(u.u_tsize = (size_t)(btoc(lowest_w_addr - lowest_addr)));
	}

	if (saw_writeable_region == FALSE)
		lowest_w_addr = lowest_addr;	/* set it so we can use it */
	u.u_data_start = (caddr_t) lowest_w_addr;
	u.u_dsize = (size_t)(btoc(highest_addr - lowest_w_addr + 1));
		
		/* ******** */
/*		uprintf("u t start = %lx, u t size = %ld (0x%lx)\n", (long)
			(u.u_text_start), (long)(u.u_tsize),(long)(u.u_tsize));
		uprintf("u d start = %lx, u d size = %ld (0x%lx)\n", (long)
			(u.u_data_start), (long)(u.u_dsize),(long)(u.u_dsize));
*/

        /* Free memory allocated for loading (not args). */

	kfree(lc_bufferp, (long)(mhp->moh_sizeofcmds));

	/*
	 *	Create the stack.  (Deallocate the old one and create a 
	 *	new one).
	 */

	size = round_page(stackinc);
        addr = trunc_page(VM_MAX_ADDRESS - size);
	U_HANDY_LOCK();
	u.u_stack_start = (caddr_t) addr;
	u.u_stack_end = u.u_stack_start + size;
	u.u_stack_grows_up = FALSE;
	U_HANDY_UNLOCK();
	(void) vm_deallocate(my_map, addr, size);
	if (vm_allocate(my_map, &addr, size, FALSE) != KERN_SUCCESS) {
		uprintf("%s: Cannot find space for stack.\n", u.u_comm);
		error = EGETXFILE;
                goto out;
	}

#if	SEC_BASE
	if (security_is_on)
#if	SEC_PRIV
	compute_subject_privileges(vp, p->p_flag & STRC);
#else	/* SEC_PRIV */
	compute_subject_privileges(vp, 
		(p->p_flag & STRC || (! SEC_SUSER(uid,gid))));
#endif	/* SEC_PRIV */
#endif	/* SEC_BASE */
	/*
	 * set SUID/SGID protections, if no tracing
	 */
#if	SEC_ARCH
	BM(PROC_LOCK(p));
	pflag = p->p_flag;
	BM(PROC_UNLOCK(p));
	if ((pflag&STRC)==0) {
	    if (uid != u.u_cred->cr_uid || gid != u.u_cred->cr_gid) {
		newcr = crcopy(u.u_cred);
		newcr->cr_uid = uid;
		newcr->cr_gid = gid;
		substitute_real_creds(p, NOCRED, newcr->cr_uid, NOCRED,
			      newcr->cr_gid, newcr);
		SP_CHANGE_SUBJECT();
	    }
		if(p->p_pr_qflags & PRFS_STOPEXEC) {
#ifdef	mips
			register thread_t th = current_thread();
			th->pcb->trapcause = CAUSEEXEC;
#endif
		   	/* If the stop-on-exec bit is set, stop the task */
			if(!prismember(&p->task->procfs.pr_sigtrace, SIGTRAP)) {
				praddset(&p->task->procfs.pr_sigtrace, SIGTRAP);
				p->task->procfs.pr_qflags |= PRFS_SIGNAL;
			}
			unix_master();
			psignal(p, SIGTRAP);
			unix_release();
		}
	} else {
#ifdef	mips
		current_thread()->pcb->trapcause = CAUSEEXEC;
#endif
		/*
		 * exec gave us an additional reference on u.u_cred
		 * We should in fact be using the passed in creds but
		 * for historical reasons we aren't.
		 */
		crfree(u.u_cred);

		/*
		 * This software exception will be mapped to a SIGTRAP
		 * signal with the siginfo si_code field of TRAP_TRACE.
		 */
		thread_doexception(current_thread(), EXC_BREAKPOINT, 0L,
				   EXC_ALPHA_TRACE);
	}
#else	/* SEC_ARCH */
	newcr = crcopy(cred);
	BM(PROC_LOCK(p));
	pflag = p->p_flag;
	BM(PROC_UNLOCK(p));
	if ((pflag&STRC)==0) {
		newcr->cr_uid = uid;
		newcr->cr_gid = gid;
		if(p->p_pr_qflags & PRFS_STOPEXEC) {
#ifdef	mips
			register thread_t th = current_thread();
			th->pcb->trapcause = CAUSEEXEC;
#endif
		   	/* If the stop-on-exec bit is set, stop the task */
			if(!prismember(&p->task->procfs.pr_sigtrace, SIGTRAP)) {
				praddset(&p->task->procfs.pr_sigtrace, SIGTRAP);
				p->task->procfs.pr_qflags |= PRFS_SIGNAL;
			}
			unix_master();
			psignal(p, SIGTRAP);
			unix_release();
		}
	} else {
#ifdef	mips
		current_thread()->pcb->trapcause = CAUSEEXEC;
#endif

		/*
		 * This software exception will be mapped to a SIGTRAP
		 * signal with the siginfo si_code field of TRAP_TRACE.
		 */
		thread_doexception(current_thread(), EXC_BREAKPOINT, 0L,
				   EXC_ALPHA_TRACE);

	}
	substitute_real_creds(p, NOUID, newcr->cr_uid, NOGID,
			      newcr->cr_gid, newcr);
#endif	/* SEC_ARCH */
	U_HANDY_LOCK();
	u.u_ssize = SSIZE + btoc(round_page(nargc));
	u.u_prof.pr_scale = 0;
	U_HANDY_UNLOCK();

        goto out;

clean_up_more:
clean_up_less:
	kfree(lc_bufferp, (long)(mhp->moh_sizeofcmds));
out:
	*entryp = entry_tmp;
        return (error);
}
#endif	/* OSF_MACH_O */

/*
 *	BSD_A_OUT program loaders follow.
 */

#if	BSD_A_OUT
#if	defined(vax) || defined(sun) || defined(ibmrt) || defined(i386) || defined(__hp_osf)

/*
 *	FILE_OFFSET is how much of the file to skip when the page
 *	containing the a.out header is not part of the loaded image.
 *	This is not true on the SUN.
 */
#if	defined(vax) || defined(ibmrt) || defined(__hp_osf)
#define	FILE_OFFSET	LOADER_PAGE_SIZE
#endif

#if	defined(sun) || defined(i386)
#define	FILE_OFFSET	0	/* beware - not LOADER_PAGE_SIZE on SUN */
int allow_0_reference = 0;
#endif

program_loader(vp, ep, pagi)
	struct vnode	*vp;
	struct exec	*ep;
	int		pagi;
{
	vm_map_t	my_map;
	vm_offset_t	addr;
	vm_size_t	size;
	vm_offset_t	text_start;
	vm_offset_t	data_start;
	vm_size_t	loader_text_size;
	vm_size_t	loader_data_size;
	vm_size_t	text_size;
	vm_size_t	data_size;
	vm_size_t	data_bss_size;
	vm_size_t	real_text_size;
	vm_size_t	real_data_size;
	int		error = 0;

	my_map = current_task()->map;

	/*
	 *	Make copy of exec header fields that we might want
	 *	to modify.
	 */
	real_text_size = ep->a_text;
	real_data_size = ep->a_data;
#ifdef	i386
	real_text_size += sizeof (struct exec);
#define	BELIEVE_A_OUT 1
#endif

	/*
	 *	On the VAX, the loader aligns the text and data so
	 *	that we cannot protect all of the text.  Therefore,
	 *	we adjust the text/data sizes so that the text size
	 *	corresponds to the part we can protect and the data
	 *	size includes the text we cannot protect.
	 */
#if	defined(vax)
	if (pagi) {
		size = loader_round_page(real_text_size)-
		    trunc_page(real_text_size);
		real_text_size -= size;
		real_data_size += size;
	}
#endif
#ifdef	BELIEVE_A_OUT
	loader_text_size = real_text_size;
	loader_data_size = real_data_size;
#else
	loader_text_size = loader_round_page(real_text_size);
	loader_data_size = loader_round_page(real_data_size);
#endif

	/*
	 *	Since the data starts immediately on the next loader
	 *	page boundary, we cannot round the text size up to
	 *	the next memory size on the VAX.
	 */
#ifdef	vax
	text_size = loader_text_size;
#endif
#ifdef	__hp_osf
	text_size = loader_text_size;
#endif
#if	defined(sun) || defined(ibmrt) || defined(i386)
	text_size = round_page(real_text_size);
#endif

	data_size = round_page(real_data_size);
	data_bss_size = round_page(real_data_size+ep->a_bss);
#ifdef	i386
#undef	USRTEXT
#define USRTEXT	0x10000
#endif
	text_start = USRTEXT;
	if (pagi == 0 && ep->a_text == 0)
		data_start = text_start;
	else {
#ifdef	vax
		/* data immediately after text */
		data_start = text_size;
#endif
#ifdef	__hp_osf
		/* data immediately after text */
		data_start = text_size;
#endif
#ifdef	sun3
		/* data into next segment boundary after text */
		data_start = ((text_start+text_size+SGOFSET)&~SGOFSET);
#endif
#ifdef	sun4
		if (pagi == 0)
			data_start = ((text_start+text_size+SGOFSET)&~SGOFSET);
		else
			data_start = round_page(text_start + text_size);
#endif
#ifdef	ibmrt
		/* data into fixed data segment */
		data_start = 0x10000000;
#endif
#ifdef	i386
		data_start = ((text_start+text_size+(PAGE_SIZE-1))&~(PAGE_SIZE-1));
#endif
	}

#ifdef	sun
	if (pagi && trunc_page(text_start) != text_start) {
		uprintf("text doesn't start on page boundary.\n");
		goto suicide;
	}
#endif
#ifdef	__hp_osf
	if (pagi && trunc_page(text_start) != text_start) {
		uprintf("text doesn't start on page boundary.\n");
		goto suicide;
	}
#endif

	/*
	 *	Remember text and data starting points.
	 */
	U_HANDY_LOCK();
	u.u_text_start = (caddr_t)text_start;
	u.u_data_start = (caddr_t)data_start;
	U_HANDY_UNLOCK();
#ifdef	vax
	if (pagi) u.u_data_start += size; /* adjust for change above */
#endif

	error = 0;

	if (pagi == 0) {
		/*
		 *	Not demand paged.
		 *
		 *	Allocate space for image.
		 */
		if (text_size + data_bss_size > 0) {
			addr = trunc_page(data_start);
			size = data_bss_size + data_start - addr;
			if (size > 0 &&
			    vm_allocate(my_map, &addr, size, FALSE)
			    != KERN_SUCCESS) {
				uprintf("Cannot find space for data+bss.\n");
				goto suicide;
			}
			addr = text_start;
			size = trunc_page(text_size);
			if (size > 0 &&
			    vm_allocate(my_map, &addr, size, FALSE)
			    != KERN_SUCCESS) {
				uprintf("Cannot find space for text.\n");
				goto suicide;
			}
		}

		/*
		 *	Read in the data segment (OMAGIC & NMAGIC).
		 */
		if (real_data_size) {
			error = vn_rdwr(UIO_READ, vp,
				(caddr_t)data_start, (int)real_data_size,
				(off_t)(sizeof(struct exec)+ real_text_size),
				UIO_USERSPACE, IO_UNIT, u.u_cred, (int *)0);
		}

		/*
		 *	Read in text segment if necessary (NMAGIC),
		 *	and read-protect it.
		 */
		if ((error == 0) && (real_text_size > 0)) {
			error = vn_rdwr(UIO_READ, vp,
				(caddr_t) text_start, (int)real_text_size,
				(off_t)sizeof(struct exec), UIO_USERISPACE, 
				IO_UNIT, u.u_cred, (int *) 0);
			if (error == 0 && trunc_page(text_size) > 0) {
				(void) vm_protect(my_map,
					 text_start,
					 trunc_page(text_size),
					 FALSE,
					 VM_PROT_READ|VM_PROT_EXECUTE);
			}
		}
	}
	else {

		/*
		 *	Map the text segment.
		 */
		addr = text_start;
		if (text_size != 0) {
			VOP_MMAP(vp, (vm_offset_t) FILE_OFFSET,
				my_map, &addr, text_size,
				VM_PROT_READ|VM_PROT_EXECUTE, VM_PROT_ALL,
				MAP_FIXED|MAP_PRIVATE, u.u_cred, error); 
			if (error) {
				uprintf("Cannot map text into user address space\n");
				goto suicide;
			}
		}

		/*
		 *	Now map in the data segment.
		 */
		addr = data_start;
		if (data_size != 0) {
			VOP_MMAP(vp, (vm_offset_t)
				loader_text_size + FILE_OFFSET,
				my_map, &addr, data_size,
				VM_PROT_ALL, VM_PROT_ALL, 
				MAP_PRIVATE|MAP_FIXED, u.u_cred, error); 
			if (error) {
				uprintf("Cannot map data into user space.\n");
				goto suicide;
			}
			addr += data_size;
		}


		/*
		 *	Allocate the remainder of the BSS segment.
		 */
		size = data_bss_size - data_size;
		if (size != 0 &&
		    vm_allocate(my_map, &addr, size, FALSE)!= KERN_SUCCESS) {
		    	uprintf("Cannot allocate BSS in user address space\n");
			goto suicide;
		}

		/*
		 *	If the data segment does not end on a VM page
		 *	boundary, we have to clear the remainder of the VM
		 *	page it ends on so that the bss segment will
		 *	(correctly) be zero.
		 *	The loader has already guaranteed that the (text+data)
		 *	segment ends on a loader_page boundary.
		 */
		addr = data_start + loader_data_size;
		size = data_size - loader_data_size;
		if (size > 0) {
			if (copyout(vm_kern_zero_page, (caddr_t)addr, size)) {
				uprintf("Cannot zero partial data page\n");
				goto suicide;
			}
		}
	}

	/*
	 *	Reprotect just text page 0 to NONE
	 */
#if	defined(sun) || defined(i386)
#define PROT (allow_0_reference ? VM_PROT_READ : VM_PROT_NONE)
	addr = 0;
	if (vm_protect(my_map, addr, VM_PROT_NONE, FALSE) != KERN_SUCCESS) {
		uprintf("Cannot reserve user page 0.\n");
		goto suicide;
	}
#endif

	/*
	 *	Create the stack.
	 *	(Deallocate the old one and create a new one).
	 */
	size = round_page(stackinc);
	addr = trunc_page(VM_MAX_ADDRESS - size);
	U_HANDY_LOCK();
	u.u_stack_start = (caddr_t) addr;
	u.u_stack_end = u.u_stack_start + size;
	u.u_stack_grows_up = FALSE;
	U_HANDY_UNLOCK();
	if (vm_allocate(my_map, &addr, size, FALSE) != KERN_SUCCESS) {
		uprintf("Cannot find space for stack.\n");
		goto suicide;
	}

	return (error);

suicide:
	return (EGETXFILE);
}
#endif	/* vax || sun || ibmrt || i386 || __hp_osf */

#ifdef	balance
/*
 * balance_getxfile()
 *	Loader for Sequent Balance (ns32000) object files.
 *
 * Code derived from VAX version in getxfile().
 *
 * Magic number ZMAGIC has already been converted to ZMAGIC.  No XMAGIC yet.
 * Assumes "pagi".
 *
 * Balance a.out's assume the code is loaded at 0x800, and text/data are
 * rounded to 2k boundaries.  The header is loaded at 0x800 as part of the
 * text, to avoid wasting space.  0 -> 0x7ff are filled with read-only
 * zero pages (ZMAGIC).  When/if there is XMAGIC, 0->0x7ff needs to
 * be totally invalid.
 *
 * ep->a_text includes the low 2k of address space; the file doesn't, however.
 */

#define LOADER_LOWBYTES	LOADER_PAGE_SIZE	/* start of address space not */
						/* backed by file */

program_loader(vp, ep, pagi)
	struct vnode	*vp;
	struct exec	*ep;
	int		pagi;
{
	register vm_map_t	my_map;
	register kern_return_t	ret;
	register vm_size_t	copy_size;
	register vm_offset_t	copy_end;
	register vm_offset_t	data_end;
	vm_size_t	bss_size;
	vm_offset_t	addr;
	vm_offset_t	low_delta;
	long		size;
	int		error = 0;

#ifdef	lint
	pagi++;
#endif

	/*
	 * Need to know how much larger MACH page is than LOADER_LOWBYTES.
	 */

	if (PAGE_SIZE > LOADER_LOWBYTES)
		low_delta = PAGE_SIZE - LOADER_LOWBYTES;
	else
		low_delta = 0;

	my_map = current_task()->map;

	/*
	 *	Remember text and data starting points
	 */
	U_HANDY_LOCK();
	u.u_text_start = USRTEXT;
	u.u_data_start = (caddr_t) loader_round_page(ep->a_text);
	U_HANDY_UNLOCK();

	error = 0;

	/*
	 * Allocate a region backed by the exec'ed vnode.
	 *
	 * copy_size is set to that part of the file that will be page-aligned
	 * in the addresss space (ie, after LOADER_LOWBYTES).  Thus, if
	 * LOADER_LOWBYTES < PAGE_SIZE, the beginning of the file is not
	 * part of the "copy" map.
	 */

	copy_size = round_page(ep->a_text + ep->a_data - PAGE_SIZE);
	if (ep->a_text+ep->a_data > PAGE_SIZE) {
		addr = VM_MIN_ADDRESS + round_page(LOADER_LOWBYTES),
		VOP_MMAP(vp, low_delta, my_map, &addr, copy_size,
			VM_PROT_ALL, VM_PROT_ALL, MAP_PRIVATE|MAP_FIXED,
			u.u_cred, error);

		if (error) {
			uprintf("Unable to map text/data.\n");
			goto suicide;
		}
	}

	/*
	 *	Allocate the blank area preceding the text
	 */

	addr = VM_MIN_ADDRESS;
	if (vm_allocate(my_map, &addr, round_page(LOADER_LOWBYTES), FALSE)
	    != KERN_SUCCESS) {
	    	uprintf("Cannot allocate low bytes region\n");
		goto suicide;
	}

	/*
	 * If the loader page-size < PAGE_SIZE, need to read the
	 * first part of the file into place.  Do this before write-protext
	 * the text, since we must write on it.
	 */

	int	resid;
	if (low_delta) {
		ret = vn_rdwr(UIO_READ, vp,
				(caddr_t) LOADER_LOWBYTES, (int) low_delta,
				(off_t) 0, UIO_USERISPACE,
			        IO_UNIT, u.u_cred, &resid);
		if (ret != KERN_SUCCESS) {
			uprintf("Could not read first page of text.\n");
			goto suicide;
		}
	}

	/*
	 * Read-protect just the text region.  Do this before we zero
	 * the bss area, so that we have only one copy of the text.
	 */

	(void) vm_protect(my_map,
		 VM_MIN_ADDRESS,
		 trunc_page(ep->a_text),
		 FALSE,
		 VM_PROT_READ|VM_PROT_EXECUTE);

	/*
	 * If the data segment does not end on a VM page boundary,
	 * we have to clear the remainder of the VM page it ends
	 * on so that the bss segment will (correctly) be zero.
	 * The loader has already guaranteed that the (text+data)
	 * segment ends on a loader_page boundary.
	 */

	data_end = VM_MIN_ADDRESS + loader_round_page(ep->a_text + ep->a_data);
	copy_end = VM_MIN_ADDRESS + round_page(LOADER_LOWBYTES) + copy_size;
	if (copy_end > data_end) {
		if(copy_end-data_end > PAGE_SIZE) {
			uprintf("Cannot clear front of bss segment.\n");
			goto suicide;
		}
		if (copyout(vm_kern_zero_page, (caddr_t)data_end, copy_end - data_end)) {
			uprintf("Cannot zero partial data page\n");
			goto suicide;
		}
	}

	/*
	 *	Allocate the BSS region
	 */

	bss_size = round_page(ep->a_text + ep->a_data + ep->a_bss) - copy_end;
	addr = copy_end;
	if (bss_size != 0) {
		if (vm_allocate(my_map, &addr, bss_size, FALSE) != KERN_SUCCESS) {
			uprintf("Cannot allocate BSS region\n");
			goto suicide;
		}
	}

	/*
	 * Create the stack.  (Deallocate the old one and create a new one).
	 *
	 * Is it really necessary to deallocate the old stack?  The
	 * vm_map_remove() done above should have deleted the entire
	 * address space.  This one might make some data/bss dissappear,
	 * though.
	 */

	size = round_page(stackinc);
	U_HANDY_LOCK();
	u.u_stack_start = (caddr_t) (addr = trunc_page(VM_MAX_ADDRESS - size));
	u.u_stack_end = u.u_stack_start + size;
	u.u_stack_grows_up = FALSE;
	U_HANDY_UNLOCK();
	(void) vm_deallocate(my_map, addr, size);
	if (vm_allocate(my_map, &addr, size, FALSE) != KERN_SUCCESS) {
		uprintf("Cannot create stack.\n");
		goto suicide;
	}
	return (error);

suicide:
	return (EGETXFILE);
}
#endif	/* balance */

#endif	/* BSD_A_OUT */

#if SYSV_ELF
/* Version of getxfile for ELF object file format */

int elf_getxfile(p, vp, ehdr, nargc, uid, gid, is_priv, cred)
	register struct proc *p;
	register struct vnode *vp;
	Elf32_Ehdr *ehdr;
	long nargc, uid, gid;
	long is_priv;
	struct ucred *cred;
{
	int i;
	vm_map_t my_map;
	unsigned long text_start, text_size, text_offset;
	unsigned long data_start, data_size, data_offset;
	unsigned long bss_start, bss_size;
	vm_offset_t vm_text_start, vm_text_end;
	vm_offset_t vm_data_start, vm_data_end;
	vm_offset_t vm_bss_end;
	int error;
	int resid;
	vm_size_t size;
	vm_offset_t addr;
	int pflag;
	struct ucred *newcr;

	/*
	 * Determine the start address and size of the segments by
	 * reading the program headers looking for load commands
	 */

	text_size = 0;
	data_size = 0;
	bss_size = 0;

	i = ehdr->e_phnum;
	addr = ehdr->e_phoff;
	do {
		Elf32_Phdr phdr;

		error = vn_rdwr(UIO_READ, vp, (caddr_t)&phdr, sizeof(phdr),
		    addr, UIO_SYSSPACE, IO_UNIT, u.u_cred, &resid);
		if (error) {
			return error;
		} else if (resid) {
			return ENOEXEC;
		}

		if (phdr.p_type == PT_LOAD) {
			switch (phdr.p_flags & (PF_R + PF_W)) {
			case PF_R:		/* TEXT */
				text_start = phdr.p_vaddr;
				text_size = phdr.p_filesz;
				text_offset = phdr.p_offset;
				break;

			case PF_R + PF_W:	/* DATA */
				data_offset = phdr.p_offset;
				data_start = phdr.p_vaddr;
				data_size = phdr.p_filesz;
				bss_start = data_start + data_size;
				bss_size = phdr.p_memsz - data_size;
				break;
			}
		}

		addr += ehdr->e_phentsize;
	} while (--i);

	/* Determine the actual VM addresses we'll use to map the
	 * the regions
	 */

	vm_text_start = trunc_page(text_start);
	vm_text_end = round_page(text_start + text_size);
	vm_data_start = trunc_page(data_start);
	vm_data_end = round_page(data_start + data_size);
	vm_bss_end = round_page(bss_start + bss_size);

	/* If the adjust text end overlaps the start of data,
	 * adjust the text end. The real end of the text will be
	 * mapped as data.
	 */

	if (vm_text_end > vm_data_start)
		vm_text_end = vm_data_start;

	/* beats me... */

	PROC_LOCK(p);
	p->p_flag &= ~(SPAGV | SSEQL | SUANOM | SXONLY);
	PROC_UNLOCK(p);
	VOP_ACCESS(vp, VREAD, u.u_cred, error);
	PROC_LOCK(p);
	if (error) {
		p->p_flag |= SXONLY;
		p->p_flag &= ~STRC;
	}
	p->p_flag |= SPAGV | SEXEC;
	PROC_UNLOCK(p);


	my_map = current_task()->map;

	/*
	 * Even if we're exec'ing the same image (the RFS server does
	 * this, for example), we do not have to unlock the vnode;
	 * deallocating it does not require it to be locked
	 */

	(void)vm_exec(is_priv);

	/* Remember where the text and data start */

	U_HANDY_LOCK();
	u.u_text_start = (caddr_t)vm_text_start;
	u.u_data_start = (caddr_t)vm_data_start;
	U_HANDY_UNLOCK();


	if (vm_text_end != vm_text_start) {
		VOP_MMAP(vp, text_offset - (text_start - vm_text_start),
			my_map, &vm_text_start, vm_text_end - vm_text_start,
			VM_PROT_READ|VM_PROT_EXECUTE, VM_PROT_ALL,
			MAP_FIXED|MAP_PRIVATE, u.u_cred, error); 
		if (error) {
			uprintf("%s: cannot map text\n", u.u_comm);
			return EGETXFILE;
		}
	}

	if (vm_data_end != vm_data_start) {
		VOP_MMAP(vp, data_offset - (data_start - vm_data_start),
			my_map, &vm_data_start, vm_data_end - vm_data_start,
			VM_PROT_READ|VM_PROT_WRITE, VM_PROT_ALL,
			MAP_FIXED|MAP_PRIVATE, u.u_cred, error); 
		if (error) {
			uprintf("%s: cannot map data\n", u.u_comm);
			return EGETXFILE;
		}
	}


	if (size = vm_data_end - bss_start) {

		/* Zero out the part of the last data page which is
		 * actually the beginning of bss
		 */

		if (copyout(vm_kern_zero_page, (caddr_t)bss_start, size)) {
			uprintf("%s: cannot zero beginning of bss\n", u.u_comm);
			return EGETXFILE;
		}
	}

	if (vm_bss_end > vm_data_end) {
		if (vm_allocate(my_map, &vm_data_end, vm_bss_end - vm_data_end,
		    FALSE) != KERN_SUCCESS) {
			uprintf("%s: cannot allocate bss\n", u.u_comm);
			return EGETXFILE;
		}
		vm_protect(my_map, bss_start, bss_size, FALSE,
		    VM_PROT_READ | VM_PROT_WRITE);
	}

	/* Create stack by deallocating the old one and creating a new one */

	size = round_page(stackinc);
	U_HANDY_LOCK();
	u.u_stack_start = (caddr_t)(addr = trunc_page(VM_MAX_ADDRESS - size));
	u.u_stack_end = u.u_stack_start + size;
	u.u_stack_grows_up = FALSE;
	U_HANDY_UNLOCK();
	if (vm_allocate(my_map, &addr, size, FALSE) != KERN_SUCCESS) {
		uprintf("%s: cannot allocate stack\n", u.u_comm);
		return EGETXFILE;
	}

#if	SEC_BASE
	if (security_is_on)
#if	SEC_PRIV
	compute_subject_privileges(vp, p->p_flag & STRC);
#else	/* SEC_PRIV */
	compute_subject_privileges(vp, 
		(p->p_flag & STRC || (! SEC_SUSER(uid,gid))));
#endif	/* SEC_PRIV */
#endif	/* SEC_BASE */

	/* Set SUID/SGID protections, if no tracing */

#if	SEC_ARCH
	BM(PROC_LOCK(p));
	pflag = p->p_flag;
	BM(PROC_UNLOCK(p));
	if ((pflag & STRC)==0) {
	    if (uid != u.u_cred->cr_uid || gid != u.u_cred->cr_gid) {
		newcr = crcopy(u.u_cred);
		newcr->cr_uid = uid;
		newcr->cr_gid = gid;
		substitute_real_creds(p, NOCRED, newcr->cr_uid, NOCRED,
			      newcr->cr_gid, newcr);
		SP_CHANGE_SUBJECT();
		if(p->p_pr_qflags & PRFS_STOPEXEC) {
#ifdef	mips
			register thread_t th = current_thread();
			th->pcb->trapcause = CAUSEEXEC;
#endif
		   	/* If the stop-on-exec bit is set, stop the task */
			if(!prismember(&p->task->procfs.pr_sigtrace, SIGTRAP)) {
				praddset(&p->task->procfs.pr_sigtrace, SIGTRAP);
				p->task->procfs.pr_qflags |= PRFS_SIGNAL;
			}
			unix_master();
			psignal(p, SIGTRAP);
			unix_release();
		}
	    }
	} else {
#ifdef	mips
		current_thread()->pcb->trapcause = CAUSEEXEC;
#endif
		/*
		 * exec gave us an additional reference on u.u_cred
		 * We should in fact be using the passed in creds but
		 * for historical reasons we aren't.
		 */

		crfree(u.u_cred);

		/*
		 * This software exception will be mapped to a SIGTRAP
		 * signal with the siginfo si_code field of TRAP_TRACE.
		 */
		thread_doexception(current_thread(), EXC_BREAKPOINT, 0L,
				   EXC_ALPHA_TRACE);
	}
#else /* SEC_ARCH */
	newcr = crcopy(cred);
	BM(PROC_LOCK(p));
	pflag = p->p_flag;
	BM(PROC_UNLOCK(p));
	if ((pflag & STRC)==0) {
		newcr->cr_uid = uid;
		newcr->cr_gid = gid;
		if(p->p_pr_qflags & PRFS_STOPEXEC) {
#ifdef	mips
			register thread_t th = current_thread();
			th->pcb->trapcause = CAUSEEXEC;
#endif
		   	/* If the stop-on-exec bit is set, stop the task */
			if(!prismember(&p->task->procfs.pr_sigtrace, SIGTRAP)) {
				praddset(&p->task->procfs.pr_sigtrace, SIGTRAP);
				p->task->procfs.pr_qflags |= PRFS_SIGNAL;
			}
			unix_master();
			psignal(p, SIGTRAP);
			unix_release();
		}
	} else {
#ifdef	mips
		current_thread()->pcb->trapcause = CAUSEEXEC;
#endif

		/*
		 * This software exception will be mapped to a SIGTRAP
		 * signal with the siginfo si_code field of TRAP_TRACE.
		 */
		thread_doexception(current_thread(), EXC_BREAKPOINT, 0L,
				   EXC_ALPHA_TRACE);
	}
	substitute_real_creds(p, NOUID, newcr->cr_uid, NOGID,
			      newcr->cr_gid, newcr);
#endif	/* SEC_ARCH */

	/* Fill in segment sizes */

	U_HANDY_LOCK();
	u.u_tsize = btoc(vm_text_end - vm_text_start);
	u.u_dsize = btoc(vm_bss_end - vm_data_start);
	u.u_ssize = SSIZE + btoc(loader_round_page(nargc));
	u.u_prof.pr_scale = 0;
	U_HANDY_UNLOCK();

	return 0;
}
#endif /* SYSV_ELF */

#include <sys/reboot.h>

char		init_program_name[128] = "/sbin/mach_init";
char		init_args[128] = "-sa\0";
struct execa	init_exec_args;
int		init_attempts = 0;


void load_init_program()
{
	vm_offset_t	init_addr;
	vm_size_t	init_size;
	int		*old_ap;
	char		*argv[3];
	long		retval[2];
	int		error = 0;

	unix_master();

#ifdef __alpha
	/* Don't merge this change.  See ../arch/alpha/exec_subr.h */
	pmap_scavenge_boot();
#endif __alpha


#if !defined mips && !defined __alpha
	init_args[1] = (boothowto & RB_SINGLE) ? 's' : 'x';
	init_args[2] = (boothowto & RB_ASKNAME) ? 'a' : 'x';
#endif
	
	do {
#if	defined(balance) || defined(multimax) || defined(i386)
		if (init_attempts == 4)
			panic("Can't load init");
#else
		if (boothowto & RB_INITNAME) {
			printf("init program? ");
			gets(init_program_name);
		}
#endif

		if (error && ((boothowto & RB_INITNAME) == 0) &&
		    (init_attempts == 1)) {
			static char other_init1[] = "/sbin/init";
			error = 0;
			bcopy(other_init1, init_program_name,
				sizeof(other_init1));
		}

		if (error && ((boothowto & RB_INITNAME) == 0) &&
		    (init_attempts == 2)) {
			static char other_init2[] = "/etc/mach_init";
			error = 0;
			bcopy(other_init2, init_program_name,
				sizeof(other_init2));
		}

		if (error && ((boothowto & RB_INITNAME) == 0) &&
		    (init_attempts == 3)) {
			static char other_init3[] = "/etc/init";
			error = 0;
			bcopy(other_init3, init_program_name,
				sizeof(other_init3));
		}


		init_attempts++;

		if (error) {
			printf("Load of %s failed, errno %d\n",
					init_program_name, error);
			error = 0;
			boothowto |= RB_INITNAME;
		}

		/*
		 *	Copy out program name.
		 */

		init_size = round_page(sizeof(init_program_name) + 1);
		init_addr = VM_MIN_ADDRESS;
		(void) vm_allocate(current_task()->map, &init_addr, init_size, TRUE);
		if (init_addr == 0)
			init_addr++;
		(void) copyout((caddr_t) init_program_name,
				(caddr_t) (init_addr),
				(unsigned) sizeof(init_program_name));

		argv[0] = (char *) init_addr;

		/*
		 *	Put out first (and only) argument, similarly.
		 */

		init_size = round_page(sizeof(init_args) + 1);
		init_addr = VM_MIN_ADDRESS;
		(void) vm_allocate(current_task()->map, &init_addr, init_size, TRUE);
		if (init_addr == 0)
			init_addr++;
		(void) copyout((caddr_t) init_args,
				(caddr_t) (init_addr),
				(unsigned) sizeof(init_args));

		argv[1] = (char *) init_addr;

		/*
		 *	Null-end the argument list
		 */

		argv[2] = (char *) 0;
		
		/*
		 *	Copy out the argument list.
		 */
		
		init_size = round_page(sizeof(argv));
		init_addr = VM_MIN_ADDRESS;
		(void) vm_allocate(current_task()->map, &init_addr, init_size, TRUE);
		(void) copyout((caddr_t) argv,
				(caddr_t) (init_addr),
				(unsigned) sizeof(argv));

		/*
		 *	Set up argument block for fake call to execve.
		 */

		init_exec_args.fname = argv[0];
		init_exec_args.argp = (char **) init_addr;
		init_exec_args.envp = 0;

		ASSERT(u.u_cred != NOCRED);
		error = execve(u.u_procp, (void *) &init_exec_args, retval);
	} while (error);

	unix_release();
}
