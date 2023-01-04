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
static char *rcsid = "@(#)$RCSfile: dump.c,v $ $Revision: 1.1.3.5 $ (DEC) $Date: 1992/09/29 09:15:17 $";
#endif
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
 * derived from machdep.c	2.1	(ULTRIX/OSF)	12/3/90";
 */
/* from static char *sccsid = "@(#)machdep.c	4.12      (ULTRIX)  11/14/90"; */
/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */


/*
 * Some old history from machdep.c:
 *
 * 
 * Revision 1.1.3.5  91/12/20  14:28:48  William_Burns
 * 	revision 3.14.9.4
 * 	date: 91/12/20 11:59:53;  author: devrcs;  
 *	state: Exp;  lines added/del: 28/1
 * 	sccs rev: 3.17;  orig date: 91/11/10 14:44:06;  orig author: fred
 * 	Fred Canter - Darrell's crash dump fixes.
 * 	[91/12/20  14:07:12  William_Burns]
 * 
 * 	91/12/06	Gary Dupuis
 * 	Added support for Maxine (PERSONAL_DECstation)
 * 	1) In netbootchk() add a case for DS_MAXINE in the switch on
 * 	   cpu. It uses the same code as the DS_5000_100.
 * 	2) In badaddr() do the same.
 * 	[91/12/12  10:11:36  Gary_Dupuis]
 * 
 */
/************************************************************************
 *
 *	Modification History: machdep.c
 *
 * 08-Nov-91	Darrell Dunnuck
 *	Fixed crashdumps for serial line DS5000.
 *
 * 12-Jun-1989 -- gg
 *	In dumpsys() added a check for the presence dumpdev.
 *
 * 20-Apr-1989	afd
 * 	In dumpsys(), if console is a graphics device, force printf 
 *	messages directly to screen.  This is needed here for the case 
 *	when we manually start the dump routine from console mode.
 *
 * 29-Dec-1988  afd
 *	In dumpsys(), set dumpsize BEFORE calling netdump().
 *	Otherwise, dumpsize is zero and savecore saves zero bytes.
 *
 *
 **********************************************************************/


#include <machine/reg.h>
#include <mach/machine/thread_status.h>

#include <sys/systm.h>
#include <sys/user.h>
#include <sys/types.h>

#include <sys/proc.h>
#include <sys/buf.h>
#include <sys/conf.h>

#include <ufs/inode.h>

#include <sys/mount.h>
#include <ufs/ufsmount.h>
#include <ufs/fs.h>

#include <kern/xpr.h>

#include <machine/cpu.h>
#include <machine/fpu.h>
#include <hal/cpuconf.h>

#include <sys/lock.h>

extern int cpu;
extern struct cpusw *cpup;


int	dumpmag = 0x8fca0101;	/* magic number for savecore */
int	dumpsize = 0;		/* also for savecore */
int	dont_dump = 0;		/* if nonzero, skip the dump */
int     partial_dump = 0;       /* if nonzero, do a partial dump */
int	partial_dumpmag = 0x8fca0104;	/* partial dump magic number */

/*
 * Doadump comes here after turning off memory management and
 * getting on the dump stack, either when called above, or by
 * the auto-restart code.
 */
dumpsys()
{
	extern int rex_base;
	extern int physmem;

	if (dont_dump)
		return;
	if (dumpdev == NODEV)
		return;
	if (dumpsize) {
		return;
	}
	if(partial_dump) dumpsize = num_kernel_pages();
	else dumpsize = physmem;	/* Phys # pages */

	/* TODO: sve_tlb UTRIX dumpsys code
	 * Save away the TLB information needed to analyze MIPS dumps.
	save_tlb(tlb_dump);
	*/

	/*
	 * Reset the SII SCSI controllers before taking a crash
	 * dump. Needed for the dumb PROM drivers on MIPS to work.
	 */
	switch(cpu) {
	    case DS_3100:
		siireset();
		break;
	    case DS_5100:
		siireset();
		break;
	    default:
		break;
	}
	/* TODO: printstate set to panic print in ULTRIX dumpsys
	 * If console is a graphics device, force printf messages 
	 * directly to screen. This is needed here for the case when 
	 * we manually start the dump routine from console mode.
	printstate |= PANICPRINT;
	 */

	/* TODO: no netboot check in dumpsys */

/* TOOD: partition table stuff from ULTRIX out for now (dumpsys) */
#ifndef OSF
	/* The part_tbl has the dump partition info set in 
	 * init_main() on the way up. This will insure the
	 * IPL stays high and the data is good.
	 */
	parttbl = &part_tbl;	
	if (parttbl->pt_part[GETDEVS(dumpdev)].pi_nblocks <= 0) {
		cprintf("dump area improper\n");
		return(-1);
	}

	/* Set blkoffs to the beginning of the dump device in blocks. */
	dumpinfo.blkoffs = parttbl->pt_part[GETDEVS(dumpdev)].pi_blkoff;

	/* Get the size of the dump device in blocks */
	maxsize = parttbl->pt_part[GETDEVS(dumpdev)].pi_nblocks - dumplo;
	if(maxsize < dumpsize)
		dumpsize = maxsize;

	/*
	 * Check if the dump offset is valid.
	 */
	if(dumplo < 0) {
		printf("dump offset improper\n");
		return(-1);
	}
#endif	/* !OSF */

	if (cpup->flags & CONSINIT_OK)
		if (rex_base)
			rex_console_init();

	dprintf("\ndumping to dev %x, offset %d, numpages %d\n",
		dumpdev, dumplo, dumpsize);
	dprintf("dump ");
again:
	switch ((*bdevsw[major(dumpdev)].d_dump)(dumpdev)) {

	case ENXIO:
		dprintf("device bad\n");
		break;

	case EFAULT:
		dprintf("device not ready\n");
	        break;

	case EINVAL:
		dprintf("area improper\n");
		break;

	case EIO:
		dprintf("i/o error\n");
		break;

        case ENOSPC:
		/* If dump device is too small and we are doing a full
		 * dump, then try to do a partial dump
		 */
		if(partial_dump == 0){
		        dprintf("Dump device too small for full dump.\n");
			dumpsize = num_kernel_pages();
			dprintf("Trying partial dump (%d pages)\n", dumpsize);
		        partial_dump = 1;
			goto again;
		}
		else dprintf("Dump device too small\n");
		break;
	default:
		dprintf("succeeded\n");
		break;
	}
}


/*************************************************************
 *
 * dumpsetupvectors()
 *
 * Reset the operating system exception vector handling code
 * after a RESET. This is needed to get dumps from MIPS machines
 * that are hung.
 *
 ************************************************************/
dumpsetupvectors()
{
	extern  char utlbmiss[], eutlbmiss[];
	extern  char exception[], eexception[];

	bcopy(utlbmiss, UT_VEC, eutlbmiss - utlbmiss);
	bcopy(exception, E_VEC, eexception - exception);
	flush_cache();
}
