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
static char *rcsid = "@(#)$RCSfile: ldr_exec.c,v $ $Revision: 4.4.10.7 $ (DEC) $Date: 1993/10/19 21:10:08 $";
#endif 
/*
 *
 * 14-May-1991, afd
 *      First cut at Alpha and 64-bit support
 *
 * 28-Feb-1991, Ken Lesniak
 *      Add support for COFF and ELF shared libraries and ELF executables.
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
 * OSF/1 Release 1.0
 */
/*
 * This file implements exec_with_loader().  exec_with_loader()
 * implements, for the most part, the exact same functionality as
 * execve().  All the execve() checks of the file, including #!
 * interpretation, are performed by exec_with_loader(), however,
 * exec_with_loader() loads a user space loader, instead of the file,
 * and passes the name of the file to the loader, with the hope that the
 * loader will load and execute the file.
 */

#include <sys/secdefines.h>
#include <sys/exec_incl.h>
#include <sys/ldr_exec.h>
#include <sys/auxv.h>
#include <mach/machine/thread_status.h>
#include <sys/dk.h>

long exec_args_copyback();

#if	OSF_MACH_O
extern int decode_mach_o_hdr(void *, size_t, unsigned long, mo_header_t *);
#endif

struct exec_with_loader_args {
	int	flags;
	char	*loader;
	char	*file;
	char	**argv;
	char	**envp;
};

/*
 * exec_with_loader()
 *
 * exec_with_loader() implements, for the most part, the exact same
 * functionality as execve().  All the execve() checks of the file,
 * including #!  interpretation, are performed by exec_with_loader(),
 * however, exec_with_loader() loads a user space loader, instead of the
 * file, and passes the name of the file to the loader, with the hope
 * that the loader will load and execute the file.
 */
exec_with_loader(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct exec_with_loader_args *uap;
	int flags;

	uap = (struct exec_with_loader_args *) args;
	flags = uap->flags & LDR_EXEC_USER_MASK;
        ts_sysexec++; /* global table() system call counter (see table.h) */
	return (exec_load_loader(p, flags, uap->loader, uap->file,
		uap->argv, uap->envp, &u.u_file_state));
}

/*
 * exec_load_loader()
 *
 * exec_load_loader() is the common code, that really implements
 * exec_with_loader(), that can be called by both exec_with_loader() and
 * execve(). 
 */
exec_load_loader(p, flags, loader, ufile, uargv, uenvp, ufp)
	struct proc *p;
	long flags;
	char *loader, *ufile, **uargv, **uenvp;
	struct ufile_state *ufp;
{
	enum uio_seg loader_segflg;
	int na, ne, nc, fd = -1;	/* LEAVE THESE AS INTS FOR ALPHA */
	int uid, gid;
	int docoff = 0;
	int do_bsd_a_out = 0;
	int do_o_mach_o = 0;
	int do_elf = 0;
	boolean_t is_priv;
	char *shell_name, *shell_arg;
	char shell_or_file_name_tail[MAXCOMLEN + 1];
	struct vnode *vp;
	struct vattr vattr;
	register struct nameidata *ndp = &u.u_nd;
	vm_offset_t exec_args = 0;
	char line[MAXINTERP];
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
#ifdef	__alpha
	vm_offset_t	user_sp;
	vm_offset_t	save_sp;
#endif	/* __alpha */
#endif
#if	OSF_MACH_O
	long		entry_addr = 0;
	int		conversion_error;
        char		mo_header_buf[MO_SIZEOF_RAW_HDR];
	                              /* buffer for raw canonical version */
        mo_header_t	mo_header;    /* translated version of the header */
#endif
	int audlen = 0;
	char *audptr[4];
	audptr[2] = audptr[3] = NULL;

	/*
	 * Get vnode for file.
	 */
	ndp->ni_nameiop = LOOKUP | FOLLOW;
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = ufile;
	if (error = namei(ndp))
		return (error);
	vp = ndp->ni_vp;
	VOP_GETATTR(vp, &vattr, u.u_cred, error);
	if (error)
		goto bad;

	/*
	 * Get effective uid and gid, then check for proper access to
	 * the file.
	 */
	if (error = exec_get_effective_ids(vp, &vattr, &uid, &gid, &is_priv))
		goto bad;
	if (error = exec_check_access(p, vp, &vattr))
		goto bad;

	/*
	 *	Read in the header to get magic number.
	 *	This magic number is architecture-dependent.
	 */
	exdata.ex_shell[0] = '\0';	/* for zero length files */
	error = vn_rdwr(UIO_READ, vp, (caddr_t)&exdata, sizeof(exdata),
	    (off_t)0, UIO_SYSSPACE, IO_UNIT, u.u_cred, &resid);
	if (error)
		goto bad;

	/*
	 * First check to see if we have a shell script.  If #!
	 * interpretation is being requested, upon return from
	 * exec_get_shell(), shell_name contains the pathname of
	 * the shell to execute and shell_arg contains any additional
	 * argument to the shell.  Note that shell_arg may be NULL.
	 * Throughout the remainder of this function, shell_name is
	 * used to indicate where #! interpretation has been requested.
	 *
	 * Next, if the file is less than MAXINTERP bytes long, it
	 * must be a shell script before we will continue trying to
	 * load it  (i.e. before we will continue trying to load a
	 * loader that will load the shell specified in the script).
	 * Therefore if the file is less than MAXINTERP bytes long and
	 * it isn't a shell script, we return ENOEXEC.
	 */
	if (error = exec_get_shell(exdata.ex_shell, &shell_name, &shell_arg))
		goto bad;

	/*
	 * If #! found, finished with vnode for file.  Now get the
	 * vnode for the shell and make sure we have execute access to
	 * it.  We do not allow set*id shell scripts.
	 *
	 * Next, save the directory entry name of shell or file as
	 * returned by namei().  Now we are finished with either the
	 * shell vnode or the file vnode.
	 */
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

		if(vp->v_usecount) vrele(vp);
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
#if	SYSV_COFF
#ifdef	__alpha
		error = vn_rdwr(UIO_READ, vp, &exdata, sizeof(exdata),
			     (off_t)0, UIO_SYSSPACE, IO_UNIT, u.u_cred, &resid);
		if (error)
			goto bad;
		if (resid) {
			error = ENOEXEC;
			goto bad;
		}
#endif	/* __alpha */
#endif
		uid = u.u_cred->cr_uid;	/* shell scripts can't be setuid */
		gid = u.u_cred->cr_gid;
		is_priv = FALSE;
	}
#if	SYSV_COFF
#ifdef	__alpha
	/*
	 * Capture the user stack pointer before exdata is
	 * overwritten with the loader's header information.
	 */
	user_sp = ahdr.text_start;
#endif	/* __alpha */
#endif
	bcopy((caddr_t)ndp->ni_dent.d_name, (caddr_t)shell_or_file_name_tail,
		MAXCOMLEN);
	shell_or_file_name_tail[MAXCOMLEN] = '\0';

	/*
	 * Get loader name.  Setuid and/or setgid files must use the
	 * LDR_EXEC_DEFAULT_LOADER.  Set appropriate flags.
	 */
	if (loader && !is_priv)
		loader_segflg = UIO_USERSPACE;
	else {
		if (uid != u.u_uid)
			flags |= LDR_EXEC_SETUID_F;
		if (gid != u.u_gid)
			flags |= LDR_EXEC_SETGID_F;
		loader = LDR_EXEC_DEFAULT_LOADER;
		loader_segflg = UIO_SYSSPACE;
	}

	if (strcmp(LDR_EXEC_DEFAULT_LOADER,loader) == 0) 
		if (error = vn_kopen(vp, FREAD, &fd)) goto bad;

	/*
	 * Get vnode for loader.
	 */
	ndp->ni_nameiop = LOOKUP | FOLLOW;
	ndp->ni_segflg = loader_segflg;
	ndp->ni_dirp = loader;
	if (error = namei(ndp))
		return (error);
	vp = ndp->ni_vp;
	VOP_GETATTR(vp, &vattr, u.u_cred, error);
	if (error)
		goto bad;

	/*
	 * Check for execute access to loader.
	 */
	if (error = exec_check_access(p, vp, &vattr))
		goto bad;

	/*
	 *	Read in the header to get magic number.
	 *	This magic number is architecture-dependent.
	 */
	error = vn_rdwr(UIO_READ, vp, (caddr_t)&exdata, sizeof(exdata),
	    (off_t)0, UIO_SYSSPACE, IO_UNIT, u.u_cred, &resid);
	if (error)
		goto bad;
	if (resid) {
		error = ENOEXEC;
		goto bad;
	}

#if	SYSV_ELF
	if (IS_ELF(exdata.ehdr)) {

		/* Make sure we've read all of the ELF header. Beats me
		 * why the initial read doesn't read sizeof(exdata) bytes.
		 */

		if (sizeof(Elf32_Ehdr) > MAXINTERP) {
			error = vn_rdwr(UIO_READ, (caddr_t)&exdata.ehdr,
			    sizeof(Elf32_Ehdr), (off_t)0, UIO_SYSSPACE,
			    (IO_UNIT | IO_NODELOCKED), u.u_cred, &resid);
			if (error)
				goto bad;
		}

		/* Make sure we have an acceptable object:
		 *	little endian header
		 *	little endian text/data
		 *	machine type is MIPS
		 *	program headers are present
		 *	object type is executable or shared lib
		 */

		if ((exdata.ehdr.e_ident[EI_DATA] != ELFDATA2LSB) ||
		    (exdata.ehdr.e_flags & EF_MIPS_OPSEX) ||
		    (exdata.ehdr.e_machine != EM_MIPS) ||
		    (exdata.ehdr.e_phoff == 0) ||
		    (exdata.ehdr.e_phentsize == 0) ||
		    (exdata.ehdr.e_phnum <= 0) ||
		    (exdata.ehdr.e_type != ET_EXEC && exdata.ehdr.e_type != ET_DYN)) {
			error = ENOEXEC;
			goto bad;
		}

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
		do_bsd_a_out = 1;
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
		/*
		 * Now read in the complete file header, 
		 * starting from the beginning.
		 */
		error = vn_rdwr (UIO_READ, vp, (caddr_t)mo_header_buf,
			    sizeof(mo_header_buf), (off_t)0, UIO_SYSSPACE,
			    (IO_UNIT | IO_NODELOCKED), u.u_cred, &resid);
		if (error)
			goto bad;
	
		/* Convert the canonical version of the header so we can
		 * read it. 
		 */

		conversion_error = decode_mach_o_hdr ((void *)mo_header_buf,
				     (size_t)sizeof(mo_header_buf),
				     (unsigned long)MOH_HEADER_VERSION,
				     &mo_header);
		if (conversion_error != MO_HDR_CONV_SUCCESS) {
			error = ENOEXEC;
			goto bad;
		}


		/* Now we have valid header information; see if we 
		 * can load it here.
		 */
		if ((!(mo_header.moh_flags & MOH_EXECABLE_F))
		    || (mo_header.moh_flags & MOH_RELOCATABLE_F) /* needs relocation */
		    || (mo_header.moh_flags & MOH_UNRESOLVED_F)) {
			error = ENOEXEC;
			goto bad;
		}
		/*
		 * The following checks are for whether the file
		 * can be loaded/executed on this system.
		 * We only have to worry about the 
		 * version number (and vendor_type) when
		 * there is a compatibility problem, 
		 * 	e.g. check that version >= N, where
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
		    && (mo_header.moh_first_cmd_off > 0)
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
	/*
	 *	Magic number not recognized.
	 */
	{
		error = ENOEXEC;
		goto bad;
	}

#if	SYSV_COFF
gotcoff:
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

#if	defined(mips) || defined(__hp_osf) || defined(__alpha)
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
#if	defined(mips) || defined(__hp_osf) || defined(__alpha)
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
	if (error = exec_args_collect(loader, loader_segflg, ufile, uargv, 
		    uenvp, shell_name, shell_or_file_name_tail, shell_arg, 
		    &exec_args, &na, &ne, &nc))
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
		save_sp = USRSTACK;
		USRSTACK = user_sp;
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
					  nc + (na+4)*NBPW, uid, gid, 
					  is_priv, u.u_cred);
#endif

#if	BSD_A_OUT
	if (do_bsd_a_out)
#ifdef	sparc
	/*
	 * Make sure user register windows are empty before attempting to
	 * make a new stack.
	 */
		flush_user_windows();
		error = getxfile(p, vp, &exdata.ex_exec, 
				 SA(nc + (na+4)*NBPW + sizeof (struct rwindow)),
				 uid, gid, is_priv, u.u_cred);
#else
		error = getxfile(p, vp, &exdata.ex_exec, nc + (na+4)*NBPW, uid, 
				 gid, is_priv, u.u_cred);
#endif	/* sparc */
#endif	/* BSD_A_OUT*/

	if (error) {
	/*
	 *	NOTE: to prevent a race condition, getxfile had
	 *	to temporarily unlock the inode.  If new code needs to
	 *	be inserted here before the iput below, and it needs
	 *	to deal with the inode, keep this in mind.
	 */
		goto bad;
	}
	if(vp->v_usecount) vrele(vp);
	vp = NULL;

	/*
	 * Check for tracing and copy back arguments and environment
	 * variables.
	 */
	if((p->p_flag & STRC) || (p->p_pr_qflags & PRFS_STOPEXEC))
		flags |= LDR_EXEC_PTRACE_F;
	error = exec_args_copyback((long)flags, loader, exec_args, na, ne, nc, fd, audptr);
	if (error) {
		goto bad;
	}

	unix_master();
	execsigs(p);
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
#if	defined(i386) || defined(mips) || defined(__hp_osf) || defined(__alpha)
		setregs(ahdr.entry);
#endif

#if	OSF_MACH_O
	if (do_o_mach_o)
#ifdef	multimax
#define	MMAX_MOD_START	0x2000
	setregs(entry_addr, MMAX_MOD_START);
#else
	setregs(entry_addr);
#endif
#endif	/* OSF_MACH_O */

#endif	/* SYSV_COFF */
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
	bcopy((caddr_t)shell_or_file_name_tail, (caddr_t)u.u_comm, MAXCOMLEN);

bad:
	/* generate audit record */
	if ( DO_AUDIT(u.u_event,error) ) {
		/* may need to fetch filename */
		if ( (audptr[1] = (char *)exec_args) == NULL ) {
			if ( audptr[0] = (char *)kalloc(MAXPATHLEN) )
				copyinstr ( ufile, audptr[0], MAXPATHLEN, &audlen );
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

		audit_rec_build ( u.u_event, audptr, error, 0, (int *)0, AUD_HPR, (char *)0 );
		if ( audlen ) kfree ( audptr[0], MAXPATHLEN );
	}

	if (exec_args)
		exec_args_free(exec_args);
	if (vp)
		if(vp->v_usecount) vrele(vp);
	if (error == EGETXFILE) {

		/* 
		 *	getxfile failed, kill the current process.
		 *	Send SIGKILL, blow away other pending signals.
		 */
		unix_master();
		p->p_sig = sigmask(SIGKILL);
		p->p_cursig = SIGKILL;
		u.u_sig = 0;
		u.u_cursig = 0;
		psig();		/* Bye */
		unix_release();
	}
	return (error);
}

/*
 * exec_get_effective_ids()
 */
int
exec_get_effective_ids(vp, vap, uidp, gidp, is_privp)
	struct vnode *vp;
	struct vattr *vap;
	int *uidp, *gidp;
	boolean_t *is_privp;
{
	int uid, gid;
	boolean_t is_priv;
	struct mount *mp;

	uid = u.u_cred->cr_uid;
	gid = u.u_cred->cr_gid;
	is_priv = FALSE;
	BM(VN_LOCK(vp));
	mp = vp->v_mount;
	BM(VN_UNLOCK(vp));
	if (mp == DEADMOUNT)
		return(ENODEV);
	BM(MOUNT_LOCK(mp));
	if (mp->m_flag & M_NOEXEC) {
		BM(MOUNT_UNLOCK(mp));
		return (EACCES);
	}
        if ((mp->m_flag & M_NOSUID) == 0) {
		BM(MOUNT_UNLOCK(mp));
		if (vap->va_mode & (VSUID | VSGID)) {
			if (task_secure(current_task())) {
				is_priv = TRUE;
				if (vap->va_mode & VSUID)
					uid = vap->va_uid;
				if (vap->va_mode & VSGID)
					gid = vap->va_gid;
		      } else
		uprintf("%s: privileges disabled because of outstanding IPC access to task\n",
			u.u_comm);
		}
	} else
		BM(MOUNT_UNLOCK(mp));
	*is_privp = is_priv;
	*uidp = uid;
	*gidp = gid;
	return(0);
}

/*
 * exec_check_access()
 */
int
exec_check_access(p, vp, vap)
	struct proc  *p;
	struct vnode *vp;
	struct vattr *vap;
{
	int flag;
	enum vtype type;
	int error;

	VOP_ACCESS(vp, VEXEC, u.u_cred, error);
	if (error)
		return (error);
	PROC_LOCK(p);
	flag = p->p_flag;
	if((flag & STRC) || (p->p_pr_qflags & PRFS_STOPEXEC)) {
		PROC_UNLOCK(p);
		VOP_ACCESS(vp, VREAD, u.u_cred, error);
		if (error)
			return (error);
											}
	else
		PROC_UNLOCK(p);
	VN_LOCK(vp);
	type = vp->v_type;
	VN_UNLOCK(vp);
	if (type != VREG ||
	    (vap->va_mode & (VEXEC|(VEXEC>>3)|(VEXEC>>6))) == 0)
		return (EACCES);
	return (0);
}

/*
 * exec_get_shell()
 *
 * exec_get_shell() parses a character string checking if #!
 * interpretation is requested.  It only examines the first MAXINTERP
 * characters of the string, line.  If line, begins with #!,
 * exec_get_shell() performs the rest of the normal #! processing.
 * exec_get_shell() returns 0 if successful.  If successful,
 * *shell_namep indicates whether #! interpretation was requested.  If
 * *shell_namep is NULL, no #! interpretation was requested.  If
 * *shell_namep is non-NULL, then #! interpretation was requested and
 * *shell_namep is a pointer to the pathname string of the shell that
 * should be loaded and *shell_argp contains a pointer to the optional
 * shell argument string that should be passed to that shell.  Not
 * that *shell_argp may be NULL exec_get_shell() returns -1 if an
 * error occurred and sets u.u_error  to  indicate the error. 
 */
int
exec_get_shell(line, shell_namep, shell_argp)
	char *line, **shell_namep, **shell_argp;
{    
	char *cp;

	*shell_namep = *shell_argp = (char *)0;
	if ((line[0] == '#') && (line[1] == '!')) {
		for (cp = &line[2];; ++cp) {
			if (cp >= &line[MAXINTERP])
				return (ENOEXEC);
			if (*cp == '\n') {
				*cp = '\0';
				break;
			}
			if (*cp == '\t')
				*cp = ' ';
		}
		cp = &line[2];
		while (*cp == ' ')
			cp++;
		*shell_namep = cp;
		while (*cp && *cp != ' ')
			cp++;
		if (*cp) {
			*cp++ = '\0';
			while (*cp == ' ')
				cp++;
			if (*cp)
				*shell_argp = cp;
		}
	}
	return (0);
}

/*
 * exec_args_collect()
 *
 * exec_args_collect() collects the argument variables, the environment
 * variables, the file name string and the loader name string if present,
 * placing them in a chunk of pageable kernel memory.
 * exec_args_collect() returns 0 if successful.  It returns -1 and sets
 * u.u_error to indicate the error if it fails.
 * 
 * shell_name is a pointer to the file name of the shell if #!
 * interpretation was  requested.  shell_name_tail contains the tail
 * of the canonicallized form of that file name, as returned by
 * namei().  Note well, shell_name indicates whether #! interpretation
 * was requested.  Therefore shell_name is equivalent to the old indir
 * variable.  If shell_name is non-NULL, the arguments are massaged
 * such that argv[0] is the tail of the canonicallized form of the
 * shell file name, as returned by namei(), argv[1] is the argument
 * string to the shell (e.g. shell_arg) if present, argv[2] is the
 * original file name, and the  remaining arguments come from the
 * original argv[], skipping the original argv[0]. 
 */
int
exec_args_collect(loader, loader_segflg, ufile, uargp, uenvp, shell_name,
    shell_name_tail, shell_arg, exec_argsp, nap, nep, ncp)
	char *loader, *ufile, **uargp, **uenvp;
	char *shell_name, *shell_name_tail, *shell_arg;
	enum uio_seg loader_segflg;
	int *nap, *nep, *ncp;
	vm_offset_t *exec_argsp;
{
	int na, ne, nc, cc, error;
	vm_offset_t ucp, ap;
	vm_offset_t exec_args, exec_args_allocate();
	char *cp, *sharg;
	unsigned len;
	char **from;

	na = 0;
	ne = 0;
	nc = 0;
	exec_args = exec_args_allocate();
	cp = (char *) exec_args;	/* running pointer for copy */
	cc = ARG_MAX;			/* size of exec_args */
					/*   (actually only ARG_MAX to limit */
					/*   arguments and environment */
					/*   variables) */

	/*
	 * Copy arguments and environment variables.  Note if no
	 * arguments, then no environment variables are copied.
	 */
	if (uargp) {

		from = uargp;

		if (shell_name) {

			/* argv[0] */
#ifdef __alpha
			if ((ap = fuqword((caddr_t)from)) == -1) {
#else
			if ((ap = fuword((caddr_t)from)) == -1) {
#endif
				error = EFAULT;
				goto bad;
			}
			if (ap != NULL)
				from++;		/* skip argv[0] */
			error = exec_copystr(shell_name_tail, &cp, &cc, &len, &nc);
			if (error)
				goto bad;
			na++;

			/* optional argv[1] */
			if (shell_arg) {
				error = exec_copystr(shell_arg, &cp, &cc, &len, &nc);
				if (error)
					goto bad;
				na++;
			}

			/* argv[shell_arg ? 2 : 1] */
			error = exec_copyinstr(ufile, &cp, &cc, &len, &nc);
			if (error)
				goto bad;
			na++;
		}

#ifdef __alpha
		for ( ; ((ap = fuqword((caddr_t)from)) != NULL); from++) {
#else
		for ( ; ((ap = fuword((caddr_t)from)) != NULL); from++) {
#endif
			if (ap == -1) {
				error = EFAULT;
				goto bad;
			}
			error = exec_copyinstr((caddr_t)ap, &cp, &cc, &len, &nc);
			if (error)
				goto bad;
			na++;
		}

		if (uenvp) {
#ifdef __alpha
			for (from = uenvp; ((ap = fuqword((caddr_t)from)) != NULL); from++) {
#else
			for (from = uenvp; ((ap = fuword((caddr_t)from)) != NULL); from++) {
#endif
				if (ap == -1) {
					error = EFAULT;
					goto bad;
				}
				error = exec_copyinstr((caddr_t)ap, &cp, &cc, &len, &nc);
				if (error)
					goto bad;
				na++;
				ne++;
			}
		}
	}

	/* additional space for file name and loader name */
	cc += (NCARGS - ARG_MAX);

	/* copy file name */
	if (shell_name)
		error = exec_copystr(shell_name, &cp, &cc, &len, &nc);
	else
		error = exec_copyinstr((caddr_t)ufile, &cp, &cc, &len, &nc);
	if (error)
		goto bad;

	/* copy loader name if present */
	if (loader) {
		if (loader_segflg == UIO_USERSPACE)
			error = exec_copyinstr((caddr_t)loader, &cp, &cc, &len, &nc);
		else
			error = exec_copystr(loader, &cp, &cc, &len, &nc);
		if (error)
			goto bad;
	}

	/* align character count */
	nc = (nc + NBPW-1) & ~(NBPW-1);

	/*
	 * Success
	 */
	*exec_argsp = exec_args;
	*nap = na;
	*nep = ne;
	*ncp = nc;
	return (0);

	/*
	 * Failure
	 */
bad:
	exec_args_free(exec_args);
	return (error);
}

/*
 * exec_args_copyback()
 *
 * exec_args_copyback() copies the argument count, the argument vector,
 * the environment variables and the new auxiliary vector back to the
 * user stack.  The loader name string, if present, and the file name
 * string are the first strings in the block holding the arguments and
 * the environment variables.
 *
 * There are two forms of auxv[], depending on whether
 * exec_args_copyback() was called from exec_with_loader() or execve().
 * A non-zero value in loader indicates the former.  auxv[] will always
 * the filename.  If a loader is present, then auxv[] will also
 * contain the loader filename and the loader flags.  Therefore,
 * including the AT_NULL entry, if loader is set, auxv[] will contain
 * 4 entries, otherwise it will only contain 2 entries.  Note that
 * each element occupies 8 bytes.
 */
long
exec_args_copyback(flags, loader, exec_args, na, ne, nc, fd, audp)
	vm_offset_t exec_args;
	long flags;		/* LEAVE THIS AS A LONG FOR ALPHA */
	int na, ne, nc, fd;	/* LEAVE THESE AS INTS FOR ALPHA */
	char *loader;
	char *audp[];
{
	int i, nav, cc, error, *auxv;
	vm_offset_t ap, ucp, uloader, ufile;
	unsigned len;
	char *cp;
#ifdef __alpha
	vm_offset_t save_ap;
#else
	vm_offset_t save_ar0;
#endif

	/* 
	 * Compute number of elements in auxv[]
	 */
	
	if (loader)
		nav = (fd != -1) ? 5 : 4;
	else
		nav = 2;
		
	/*
	 * Compute locations for strings, vectors and data in user
	 * address space.
	 */
#ifdef mips
	ucp = USRSTACK - nc - NBPW - EA_SIZE;
	ap = ucp - na*NBPW - 3*NBPW - (nav*2)*NBPW;
	if (ap & 0xf) {
		int fudge;
		fudge = ap - (ap & ~0xf);
		ap -= fudge;
		ucp -= fudge;
	}
	save_ar0 = u.u_ar0[EF_SP];
	u.u_ar0[EF_SP] = ap;
#else
#ifdef __alpha
	ucp = USRSTACK - nc - NBPW - EA_SIZE;
	ap = ucp - na*NBPW - 3*NBPW - (nav*2)*NBPW;
	if (ap & 0x1f) {
		int fudge;
		fudge = ap - (ap & ~0x1f);
		ap -= fudge;
		ucp -= fudge;
	}
	save_ap = mfpr_usp();
	mtpr_usp(ap);	/* load up new user stack pointer into mtpr register */
#else
#ifdef sparc
	ucp = USRSTACK - nc - NBPW;
        /*
         * Keep stack aligned and leave room for initial reg window on sparc.
         */
        ap = USRSTACK - SA(nc + (na+4+(nav*2))*NBPW);
        u.u_ar0[SP] = ap - sizeof (struct rwindow);
#else	/* sparc */
	ucp = USRSTACK - nc - NBPW;
	ap = ucp - na*NBPW - 3*NBPW - (nav*2)*NBPW;
#ifdef	i386
	u.u_ar0[UESP] = ap;
#else
	u.u_ar0[SP] = ap;
#endif
#endif	/* sparc */
#endif	/* alpha */
#endif	/* mips */

	/*
	 * Initialize pointers and counters.
	 */
	nc = 0;
	cc = 0;
	cp = (char *) exec_args;
	cc = NCARGS;

	/*
	 * Process argc, argv[] and envp[]
	 */
#ifdef __alpha
	(void) suqword((caddr_t)ap, na-ne);
#else
	(void) suword((caddr_t)ap, na-ne);
#endif
	ap += NBPW;
        u.u_argp = (char *)ucp;
	for (i = 0; i < (na - ne); i++) {
#ifdef __alpha
		(void) suqword((caddr_t)ap, ucp);
#else
		(void) suword((caddr_t)ap, ucp);
#endif
		error = exec_copyoutstr(&cp, (caddr_t)ucp, &cc, &len, &nc);
		*(cp-1) = ' ';
		ucp += len;
		if (error == EFAULT) {
#ifdef __alpha
			mtpr_usp(save_ap);
#else
			u.u_ar0[EF_SP] = save_ar0;
#endif
			return(1);
		}
		ap += NBPW;
	}
	if ( cp != (char *)exec_args ) *(cp-1) = '\0';
        u.u_arg_size = ucp - (unsigned long)u.u_argp;


	audp[2] = cp; /* start of environment */
#ifdef __alpha
	(void) suqword((caddr_t)ap, 0);
#else
	(void) suword((caddr_t)ap, 0);
#endif
	ap += NBPW;
        u.u_envp = (char *)ucp;
	for (i = 0; i < ne; i++) {
#ifdef __alpha
		(void) suqword((caddr_t)ap, ucp);
#else
		(void) suword((caddr_t)ap, ucp);
#endif
		error = exec_copyoutstr(&cp, (caddr_t)ucp, &cc, &len, &nc);
		ucp += len;
		*(cp-1) = ' ';
		if (error == EFAULT) {
#ifdef __alpha
			mtpr_usp(save_ap);
#else
			u.u_ar0[EF_SP] = save_ar0;
#endif
			return(1);
		}
		ap += NBPW;
	}
	if ( cp != (char *)exec_args ) *(cp-1) = '\0';
        u.u_env_size = ucp - (unsigned long)u.u_envp;


	audp[0] = cp; /* filename */
#ifdef __alpha
	(void) suqword((caddr_t)ap, 0);
#else
	(void) suword((caddr_t)ap, 0);
#endif
	ap += NBPW;

	/*
	 * Copyout file name string if present and remember address
	 */
	ufile = ucp;
	error = exec_copyoutstr(&cp, (caddr_t)ucp, &cc, &len, &nc);
	ucp += len;
		if (error == EFAULT) {
#ifdef __alpha
			mtpr_usp(save_ap);
#else
			u.u_ar0[EF_SP] = save_ar0;
#endif
			return(1);
		}
	audp[3] = cp; /* end */

	/*
	 * Copyout loader name string if present and remember address
	 */
	if (loader) {
		uloader = ucp;
		error = exec_copyoutstr(&cp, (caddr_t)ucp, &cc, &len, &nc);
		ucp += len;
		if (error == EFAULT) {
#ifdef __alpha
			mtpr_usp(save_ap);
#else
			u.u_ar0[EF_SP] = save_ar0;
#endif
			return(1);
		}
	}


	/*
	 * Process auxv[].  See above comments.
	 */
	
	switch(nav) {

	   case(5): /* Loader is LDR_EXEC_DEFAULT_LOADER */
#ifdef __alpha
		(void) suqword((caddr_t)ap, AT_EXEC_FD); ap += NBPW;
		(void) suqword((caddr_t)ap, fd); ap += NBPW;
#else
		(void) suword((caddr_t)ap, AT_EXEC_FD); ap += NBPW;
		(void) suword((caddr_t)ap, fd); ap += NBPW;
#endif
	   case(4): /* Loader is not LDR_EXEC_DEFAULT_LOADER */
#ifdef __alpha
		(void) suqword((caddr_t)ap, AT_EXEC_LOADER_FILENAME);ap += NBPW;
		(void) suqword((caddr_t)ap, uloader); ap += NBPW;
		(void) suqword((caddr_t)ap, AT_EXEC_LOADER_FLAGS); ap += NBPW;
		(void) suqword((caddr_t)ap, flags); ap += NBPW;
#else
		(void) suword((caddr_t)ap, AT_EXEC_LOADER_FILENAME); ap += NBPW;
		(void) suword((caddr_t)ap, uloader); ap += NBPW;
		(void) suword((caddr_t)ap, AT_EXEC_LOADER_FLAGS); ap += NBPW;
		(void) suword((caddr_t)ap, flags); ap += NBPW;
#endif
	   case(2): /* No Loader */
#ifdef __alpha
		(void) suqword((caddr_t)ap, AT_EXEC_FILENAME); ap += NBPW;
		(void) suqword((caddr_t)ap, ufile); ap += NBPW;
		(void) suqword((caddr_t)ap, AT_NULL); ap += NBPW;
		(void) suqword((caddr_t)ap, 0); ap += NBPW;
#else
		(void) suword((caddr_t)ap, AT_EXEC_FILENAME); ap += NBPW;
		(void) suword((caddr_t)ap, ufile); ap += NBPW;
		(void) suword((caddr_t)ap, AT_NULL); ap += NBPW;
		(void) suword((caddr_t)ap, 0); ap += NBPW;
#endif
	}
	
	return(0);
}

/*
 * exec_args_allocate()
 *
 * exec_args_allocate() allocates memory to hold argument
 * variables and environment variables.  The size is always
 * a constant, NCARGS.
 */
vm_offset_t
exec_args_allocate()
{
	vm_offset_t exec_args;

	exec_args = kmem_alloc_pageable(kernel_pageable_map, NCARGS);
	if (exec_args == 0)
		panic("exec_args_allocate: cannot allocate arguments");
	return(exec_args);
}

/*
 * exec_args_free()
 *
 * exec_args_free() frees the memory allocated to hold argument
 * variables and environment variables.  The size is always a
 * constant, NCARGS.
 */
exec_args_free(exec_args)
	vm_offset_t exec_args;
{
	if (exec_args)
		kmem_free(kernel_pageable_map, exec_args, NCARGS);
}

/*
 * exec_copystr()
 * exec_copyinstr()
 * exec_copyoutstr()
 *
 * These are exec() versions of copystr(), copyinstr() and copyoutstr().
 * They not only do the obvious copy operation, but they also bump the
 * character pointer, decrement the maximum count and increment the
 * current count.  They also map ENAMETOOLONG, which means string exceeded
 * maximum length, to E2BIG, which is what exec() would like to return.
 */
int
exec_copystr(src, destp, maxlengthp, lencopiedp, ncp)
	char *src, **destp;
	int *maxlengthp, *ncp;
	unsigned *lencopiedp;
{
	unsigned len;
	int error;

	error = copystr(src, *destp, (unsigned)*maxlengthp, &len);

	if (error == ENAMETOOLONG)
		error = E2BIG;

	*lencopiedp = len;
	*destp += len;
	*ncp += len;
	*maxlengthp -= len;

	return(error);
}

int
exec_copyinstr(user_src, kernel_destp, maxlengthp, lencopiedp, ncp)
	caddr_t user_src;
	char **kernel_destp;
	int *maxlengthp, *ncp;
	unsigned *lencopiedp;
{
	unsigned len;
	int error;

	error = copyinstr(user_src, *kernel_destp, (unsigned)*maxlengthp, &len);

	if (error == ENAMETOOLONG)
		error = E2BIG;

	*lencopiedp = len;
	*kernel_destp += len;
	*ncp += len;
	*maxlengthp -= len;

	return(error);
}

int
exec_copyoutstr(kernel_srcp, user_dest, maxlengthp, lencopiedp, ncp)
	char **kernel_srcp;
	caddr_t user_dest;
	int *maxlengthp, *ncp;
	unsigned *lencopiedp;
{
	unsigned len;
	int error;

	error = copyoutstr(*kernel_srcp, user_dest, (unsigned)*maxlengthp, &len);

	if (error == ENAMETOOLONG)
		error = E2BIG;

	*lencopiedp = len;
	*kernel_srcp += len;
	*ncp += len;
	*maxlengthp -= len;

	return(error);
}
