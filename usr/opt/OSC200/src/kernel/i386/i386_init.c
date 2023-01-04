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
static char	*sccsid = "@(#)$RCSfile: i386_init.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:18:16 $";
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
 * Copyright (c) 1988 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */

/*
 *	File:	i386_init.c
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Copyright (C) 1986, Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Basic initialization for I386
 */

#include <cputypes.h>
#include <mach_kdb.h>
#include <mach_rdb.h>

#include <sys/types.h>
#include <sys/systm.h>
#include <sys/table.h>
#include <i386/cpu.h>
#include <i386/pcb.h>
#include <mach/i386/vm_param.h>
#include <i386/handler.h>
#include <i386/dispatcher.h>

#include <mach/vm_param.h>
#include <mach/vm_prot.h>
#include <vm/vm_page.h>

int		loadpt;

vm_size_t	mem_size;
vm_size_t	rawmem_size;
vm_offset_t	first_addr;
vm_offset_t	last_addr;

vm_offset_t	avail_start, avail_end;
vm_offset_t	virtual_avail, virtual_end;

extern char	edata, end;

#if	AT386 || PS2
/* parameters passed from bootstrap loader */
char end;
int cnvmem = 0;		/* must be in .data section */
int extmem = 0;
char *esym = &end;
#endif	/* AT386 || PS2 */

static ihandler_t fp_handler;
static ihandler_id_t *fp_handler_id;
extern fpintr();

i386_init()
{

	/*
	 *	We won't really know our cpu number till after
	 *	we configure... but we still need to have a valid
	 *	cpu number for routines that use it.  Fortunately,
	 *	set_cpu_number will do the right thing for us here
	 *	because cpu_number doesn't know we have multi-port
	 *	memory yet.
	 */
	set_cpu_number();

/*
 *	bzero can't be called at this time since the pmap
 *	system is not initialized yet
 *
 *	bzero((caddr_t)&edata,(unsigned)(&end - &edata));
 */
 	bclear((caddr_t)&edata,(unsigned)(&end - &edata));

/*
 * Initialize the pic prior to any possible call to an spl.
 */
	picinit();
	size_memory();


	/*
	 *	Initialize kernel physical map, mapping the
	 *	region from loadpt to avail_start.
	 *	Kernel virtual address starts at VM_KERNEL_MIN_ADDRESS.
	 */

	avail_start = first_addr;
	avail_end = last_addr;

	printf("386 boot: memory from 0x%x to 0x%x\n", avail_start,
			avail_end);

	pmap_bootstrap(loadpt, &avail_start, &avail_end, &virtual_avail,
				&virtual_end);
	printf("i386_init: virtual_avail = %x, virtual_end = %x\n",
		virtual_avail, virtual_end);


	/*
	 * Install fpintr into the interrupt dispatch table.
	 * The fp unit interrupts on pic line 13 and we use spl1.
	 * Note: intpri[13] is statically initialized to SPL1.
	 */
	fp_handler.ih_level = 13;
	fp_handler.ih_handler = fpintr;
	fp_handler.ih_resolver = 0;
	fp_handler.ih_hparam[0].intparam = 0;
	fp_handler.ih_stats.intr_type = INTR_OTHER;
	if ((fp_handler_id = handler_add(&fp_handler)) != NULL)
		handler_enable(fp_handler_id);
	else
		panic("Unable to add fp interrupt handler");
#if PS2
	abios_init();
#if MACH_RDB
	printf("entering db_kdb\n");
	db_kdb(-1,0,0);
#endif /* MACH_RDB */
#endif /* PS2 */
}


int maxmem = 0;
size_memory()
{
	register vm_offset_t	look;



#if	AT386
	look = 1024*1024 + extmem*1024;    /* extend memory starts at 1MB */
#endif	/* AT386 */
#if	PS2
	look = 1024*1024 + extmem*1024;    /* extend memory starts at 1MB */
	printf("loadpt 0x%x, look 0x%x\n",loadpt,look);
#endif	/* PS2 */

	if (maxmem)
		look = maxmem;
	mem_size = look - loadpt;
#if	MACH_KDB
	if (esym < &end)
		esym = &end;
#else	/* MACH_KDB */
	esym = &end;
#endif	/* MACH_KDB */

	vm_set_page_size();
	first_addr = 0x1000;	/* bios leaves some stuff in low address */
	last_addr = trunc_page(look);
#if PS2
	cnvmem -= 16;		/* drop off last 16k of cnvmem for now XXX */
#endif
}		 	
