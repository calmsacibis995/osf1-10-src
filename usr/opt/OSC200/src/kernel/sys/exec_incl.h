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
 *	@(#)$RCSfile: exec_incl.h,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/07/15 18:49:56 $
 */ 
/*
 *
 * 28-Feb-1991, Ken Lesniak
 *	Add support for ELF executables and shared libraries
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
 * This file contains common definitions and include files used by
 *  both kern_exec.c and ldr_exec.c
 */

#ifndef _SYS_EXEC_INCL_H_
#define _SYS_EXEC_INCL_H_

#include <cputypes.h>
#include <bsd_a_out.h>
#include <sysv_coff.h>
#include <sysv_elf.h>
#include <osf_mach_o.h>

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/map.h>
#include <sys/user.h>
#include <sys/kernel.h>
#include <sys/proc.h>
#include <sys/mount.h>
#include <sys/ucred.h>
#include <sys/buf.h>
#include <sys/vnode.h>
#include <sys/mman.h>
#include <sys/vm.h>
#include <sys/file.h>
#include <sys/uio.h>
#include <sys/acct.h>
#include <sys/exec.h>

#include <machine/reg.h>
#if	defined(vax) || defined(i386)
#include <machine/psl.h>
#endif

#if	SYSV_ELF
/*
 * File headers for ELF files.
 */
#include <sysV/elf_abi.h>
#ifdef mips
#include <sysV/elf_mips.h>
#endif
#endif

#if	SYSV_COFF
/*
 *	Fileheaders for coff files.
 */
#ifdef __hp_osf
/* On HP, get coff header files copied from usr/include/<machine> source rather than
   those in the kernel/sysv source.  This is probably what needs to be done
   for the other platforms as well (right now, mips has 2 versions of each
   header - in usr/include/<machine> and in kernel/sysv).  The makefile at
   kernel/include/Makefile does the copying of these (and other) headers. */
#include <scnhdr.h>
#include <aouthdr.h>
#include <filehdr.h>
#else
#include <sysV/scnhdr.h>
#include <sysV/aouthdr.h>
#include <sysV/filehdr.h>
#endif /* hp_osf */
#endif

#if	OSF_MACH_O
/*
 *	header files for processing OSF/mach-o.
 */
#include <mach_o_header.h>
#include <mach_o_format.h>
#include <kern/kalloc.h>
#endif

#ifdef	mips
#include <sys/ptrace.h>
#endif


/*
 *  Force all namei() calls to permit remote names since this module has
 *  been updated.
 */
#if	MACH_RFS
#undef	namei
#define namei	rnamei
#endif

#ifdef	ibmrt
#include <ca/debug.h>
#endif

#ifdef	sun4
#include <sun4/asm_linkage.h>
#endif

#include <sys/signal.h>
#include <kern/task.h>
#include <kern/thread.h>

#include <mach/vm_param.h>
#include <vm/vm_map.h>
#include <vm/vm_kern.h>
#include <vm/vm_user.h>
#include <kern/zalloc.h>

#include <kern/parallel.h>



#if	BSD_A_OUT
/*
 * All of these silly loader_page_size's should be moved into a machine
 * dependent directory, for obvious reasons.  -BB
 */
#ifdef	vax
#define LOADER_PAGE_SIZE	(1024)
#endif
#if	defined(ibmrt) || defined(balance)
#define LOADER_PAGE_SIZE	(2048)
#endif
#ifdef	sun
#define LOADER_PAGE_SIZE	(8192)
#endif
#ifdef	__hp_osf
/* warning, this may be changed to 4096 at a later date */
#define LOADER_PAGE_SIZE	(8192)
#endif
#ifdef	i386
#define	LOADER_PAGE_SIZE	(4096)
#endif
#endif	/* BSD_A_OUT */

#if	SYSV_COFF
/*
 *	Corresponding definitions are in coff_getxfile because the
 *	section size (SECTALIGN, corresponds to LOADER_PAGE_SIZE)
 *	must be obtained from the file header for some architectures.
 */

#ifdef	LOADER_PAGE_SIZE
#define SECTALIGN		LOADER_PAGE_SIZE
#else
#define LOADER_PAGE_SIZE	SECTALIGN
#endif
#endif

#define loader_round_page(x)	((vm_offset_t)((((vm_offset_t)(x)) \
						+ LOADER_PAGE_SIZE - 1) \
					& ~(LOADER_PAGE_SIZE-1)))
#define loader_trunc_page(x)	((vm_offset_t)(((vm_offset_t)(x)) \
					& ~(LOADER_PAGE_SIZE-1)))


/*
 *	A corrupted fileheader can cause getxfile to decide to bail
 *	out without setting up the address space correctly.  It is
 *	essential in this case that control never get back to the
 *	user.  The following error code is used by getxfile to tell
 *	execve that the process must be killed.
 */

#define EGETXFILE	126

#endif
