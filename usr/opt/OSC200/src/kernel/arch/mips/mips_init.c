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
static char	*sccsid = "@(#)$RCSfile: mips_init.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/06/03 11:13:43 $";
#endif 
/*
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
 * OSF/1 Release 1.0
 */
/* 
 * derived from mips_init.c	2.1	(ULTRIX/OSF)	12/3/90";
 */

/*
 *	file:	mips/mips_init.c
 *	author:	A. Forin
 *
 *	Basic initialization for Mips.
 *
 *
 *  Modification History
 *
 * 27-Jun-91	Scott Cranston
 *	added #include <uerf.h>
 */

#include <confdep.h>
#include <mach_kdb.h>

#include <machine/reg.h>
#include <machine/cpu.h>
#include <machine/hwconf.h>

#include <sys/reboot.h>
#include <sys/msgbuf.h>
#include <vm/vm_map.h>
#include <mach/mach_types.h>
#include <mach/vm_param.h>

#if	PROFILING
#include <sys/gprof.h>
extern u_long	s_textsize;
extern struct phdr phdr;
#endif

vm_offset_t	avail_start, avail_end;
vm_offset_t	virtual_avail, virtual_end;
unsigned	zalloc_physical = 0;
vm_size_t	mem_size;		/* 100 ways to cook an egg */
int		maxmem, physmem;
int		memlimit = 0;		/* patchable to run in less memory */

		/* We are supposed to use this */
int		boothowto = RB_SINGLE|RB_ASKNAME|RB_KDB;

extern int	do_virtual_tables;

int	ub_argc;
char	**ub_argv;


mips_init(argc, argv, environ, vector)
int argc;
char *argv[];
char *environ[];
char *vector[];
{
	unsigned fpage;	/* first free physical page */
	extern char end[], edata[];
	extern char utlbmiss[], eutlbmiss[];
	extern char exception[], eexception[];
	extern icache_size, dcache_size;
#if	MACH_KDB
	extern char *kdbesymtab;
	struct thread fake_th;
	struct task fake_tsk;
#endif /* MACH_KDB */
	register thread_t th;
	extern main();
	extern thread_t setup_main();


/* 
 * See note below why this spl call cannot be made here
 */
/*	splhigh();			/* Safety */

	/*
	 * copy down exception vector code
	 */
	bcopy(utlbmiss, UT_VEC, eutlbmiss - utlbmiss);
	bcopy(exception, E_VEC, eexception - exception);
	bzero(edata, end - edata);	/* zero bss */

	/* initialize this cpu as boot processor 
	 * This MUST be done before any spl routines can be called 
	 * (thus before any printfs can be done).
	 *
	 * Pass along all the call params in case consoles, etc.
	 * need to parse, modify, etc. the variables.
	 */
	ub_argc = argc;
	ub_argv = argv;

	cpu_initialize(argc, argv, environ, vector);

	config_cache();

/* TODO? 	icachemask = (icache_size - 1) >> PGSHIFT; */
/* TODO? 	dcachemask = (dcache_size - 1) >> PGSHIFT; */

	flush_cache();

	/*
	 * Acknowledge all interrupts
	 */
	stopclocks();

	/*
	 * First available page of phys mem
	 */
#if	MACH_KDB
	bzero(&fake_th, sizeof fake_th);
	bzero(&fake_tsk, sizeof fake_tsk);
	current_thread() = &fake_th;
	current_thread()->task = &fake_tsk;

	fpage = (unsigned)kdbesymtab;
#else
	fpage = (unsigned)end;
#endif
	fpage = mips_btop(K0_TO_PHYS(mips_round_page(fpage)));

	/*
	 * Parse command line, setting appropriate switches and
	 * configuration values.  Also, set special /etc/init file
	 * and options if requested.
	 * NOTE: Before cnprobe() is called you must do dprintf;
	 * printf won't work.
	 */

/* TODO? 	fpage = xprinit(fpage);	/* initialize XPRBUG buffer */

	/* First set some defaults */
/* Fred -- 6/1/91, memory allocated but never used. */
/*	zalloc_physical = 420*1024;	/* based on 16meg configuration */

	getargs(argc, argv, environ);

	panic_init();		/* even if not a multiprocessor */

	size_memory(fpage);	/* set up memory and msgbuf */

	cninit();
	cnprobe(0);		/* initialize console device */

	/*
	 * We can do mprintfs after kmeminit is done in mapinit.
	 */
/* TODO: 	printstate = printstate | MEMPRINT; */

#if	MACH_KDB
	if ((boothowto & (RB_HALT|RB_KDB)) == (RB_HALT|RB_KDB))
		gimmeabreak();
#endif	/* MACH_KDB */

	printf("Mips boot: memory from 0x%x to 0x%x\n", avail_start,
			avail_end);

#if	PROFILING
	printf("Profiling kernel, s_textsize=%d(%d) [%x..%x]\n",
               s_textsize, phdr.pc_bytes, phdr.lowpc, phdr.highpc);
	printf("got pc_buf at %x\n", phdr.pc_buf);
#endif
        
	if (do_virtual_tables) {
		printf("Will allocate U*x tables in the kseg2.\n");
	} else {
		allocate_unix_tables( &avail_start, FALSE );
		avail_start = round_page(avail_start);
	}

	/*
	 * Initialize the machine-dependent portion of the VM system
	 */
	pmap_bootstrap(&avail_start, &avail_end, &virtual_avail, &virtual_end);
	/*
	 * Used to be that here we returned back to start() for it
	 * to get the first thread and proceed.  Now we do it here,
	 * avoiding some unnecessary assembly language and most importantly
	 * without losing the fake_xx for KDB.
	 */

	th = setup_main();		/* get first thread and */
					/* make it proceed to main() */
	
	th->pcb->pcb_regs[PCB_SP] = ((unsigned int) PCB_WIRED_ADDRESS
						-EF_SIZE);
	th->pcb->pcb_regs[PCB_PC] = (int)main;

	load_context(th);		/* Geronimo!! */
	/*NOTREACHED*/
}


#ifdef	mips
	vm_offset_t ws_eventQueue;
#endif

size_memory(fpage)
vm_offset_t fpage;
{
	register int i;
#if PROFILING
	caddr_t tmp_addr;
	extern char eprol[], etext[];
	extern char *s_lowpc;
#endif
	extern caddr_t dsa_physical_bg; /* DSA physically contiguous begin*/
	extern caddr_t dsa_physical_end;/* DSA physically contiguous end */
	extern int nNUQ, nNCI;

	/*
	 * Invalidate entire tlb (including wired entries)
	 */
	for (i = 2; i < NTLBENTRIES; i++)
		tlb_zero(i);

	/*
	 * Size memory through cpu dependent memory sizer
	 */
	i = machine_memsize(fpage);

	/*
	 * If we should run in less memory then do it.
	 */
	if (memlimit && (i > mips_btop(memlimit)))
		i = mips_btop(memlimit);
	/* many names for the same thing: */
	maxmem = physmem = i;
	mem_size = mips_ptob(maxmem);

	/*
	 * If we used adb (or something) on the kernel and set "Physmem"
	 *    to some value, then use that value for "physmem".
	 * This allows us to run in less physical memory than the
	 *   machine really has. (rr's idea).
	 */
/* TODO: 	if (Physmem >= MINMEM_PGS) { */
/* TODO: 		physmem = Physmem; */
/* TODO: 	} */
	/*
	 * Save the real amount of memory (or the above artificial
	 * amount) to allow the sizer to accurately configure physmem
	 * in the config file.  `Physmem' was originally added for the
	 * sizer utility - tomt
	 */
/* TODO: 	Physmem = physmem;	/* Let sizer know the size */

	/*
	 * Initialize error message buffer (at end of core).
	 * This is needed as soon as possible so that printf
	 * will stand a chance of working.
	 */
	maxmem -= btoc(sizeof(struct msgbuf));
	pmsgbuf = (struct msgbuf *)PHYS_TO_K0((unsigned)mips_ptob(maxmem));
	if (pmsgbuf->msg_magic != MSG_MAGIC)
		for (i = 0; i < MSG_BSIZE; i++)
			pmsgbuf->msg_bufc[i] = 0;

#if	PROFILING
#define palloc(name,type,num) \
	name = (type *) PHYS_TO_K0((unsigned)mips_ptob(fpage)); \
        fpage += btoc(num);

	s_textsize = 0;
	s_lowpc = phdr.lowpc = (char *)
	    ROUNDDOWN((unsigned)eprol, HISTFRACTION*sizeof(HISTCOUNTER));
	phdr.highpc = (char *)
	    ROUNDUP((unsigned)etext, HISTFRACTION*sizeof(HISTCOUNTER));
	s_textsize = phdr.highpc - phdr.lowpc;
	phdr.pc_bytes = (s_textsize / HISTFRACTION);
	palloc(phdr.pc_buf, char, phdr.pc_bytes);
#if	PROFTYPE == 2 || PROFTYPE == 3
	phdr.bb_bytes = s_textsize / BB_SCALE;
	palloc(phdr.bb_buf, char, phdr.bb_bytes);
#endif
#endif	/* PROFILING */
        
	/*
	 * Notify the VM system of what mem looks like
	 */
	vm_set_page_size();

	avail_start = (vm_offset_t)mips_ptob(fpage);
	avail_end   = (vm_offset_t)mips_ptob(maxmem);
#ifdef mips	

    	if( nNUQ || nNCI ) {
	    avail_start += 0x1fff;		/* make on 8k boundry */
	    avail_start &= ~0x1fff;		/* make on 8k boundry */
	    dsa_physical_bg = (caddr_t)PHYS_TO_K0(avail_start);
	    avail_start += (nNUQ * 8192);
	    avail_start += (nNCI * 4 * 8192);
	    dsa_physical_end = (caddr_t)PHYS_TO_K0(avail_start);
    	}

	/*
	 * Binary logger needs a page aligned page to map shared 
	 * screen control info.
	 * And it's needed now.  
	 * A cnprobe is expected to be done after returning.
	 */
	ws_eventQueue = avail_start;
	avail_start += PAGE_SIZE;
#endif /* mips */
	if (zalloc_physical) {
		extern vm_offset_t zalloc_next_space, zalloc_end_of_space;
		zalloc_next_space = PHYS_TO_K0(avail_start);
		avail_start += round_page(zalloc_physical);
		zalloc_end_of_space = PHYS_TO_K0(avail_start);
	}
}

