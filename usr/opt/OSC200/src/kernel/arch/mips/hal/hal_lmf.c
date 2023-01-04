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
static char *rcsid = "@(#)$RCSfile: hal_lmf.c,v $ $Revision: 1.1.3.5 $ (DEC) $Date: 1992/06/24 15:51:38 $";
#endif

/*
 *
 *   Modification history:
 *
 * 18-Oct-91    Carlos Christensen
 *      Port of code from ULTRIX to TIN.  
 *      Changes made to this module as follows:
 *      .  Use lock_write instead of smp_lock for locking kernel cache.
 *      .  Initialize lock for kernel cache (lk_lmf).
 *      .  Create and process exit action for activity licenses.
 *      .  Use of microtime function (instead of timepick variable) for time.
 *      .  Use of kalloc (instead of KM_ALLOC) for storage allocation.
 *      .  Insertion code to reject a P_FAMILY license (conditional).
 *      .  Remove code for P_FAMILY licenses (conditional).
 *      .  Remove code for multiple processor systems (conditional).
 *      .  Remove of certain machine types not available (conditional).
 *      .  Review of code for determination of SMM (no changes).
 *      .  Change of handling of errno and returned values (systemmatic).
 *      .  Remove special code to distinguish between 2100 and 3100
 *      Changes made in other kernel modules as follows:
 *      .  Add calls on {get|set}lminfo to kernel/dec/machine/mips/sys_sysinfo.c
 *      .  Call on lmf_init routine from kernnel/bsd/init_main.c
 *      .  Call on exit_actn routine from kernel/bsd/kern_exit.c
 *      .  Initialize u.u_exitp to NULL in kernel/bsd/kern_fork.c
 *      .  Add kernel variables {lmf|sysinfo}_debug to kernel/dec/machine/mips/kopt.c
 *      .  Add kern_lmf to kernel/conf/files
 *      Changes made in kernel/sys files as follows:
 *      .  Modify OSF header file user.h by adding u_exitp field
 *      .  Add new headers from ULTRIX lmf_smm.h, lmfklic.h, lmf.h, exit_actn.h
 *
 * 01-Jul-91    Carlos Christensen
 *      Copied this file from ULTRIX kern_lmf.c (sccs 5.4 dated 6/19/91)
 *
 * 29-May-91	Paul Grist
 *	Added DS_5000_300 support - 3max+/bigmax.
 *
 * 14-May-91	Joe Szczypek
 *	Added MAXine support.  Use new entries in lmf_smm.h.
 *
 * 06-Sep-90	Randall Brown 
 *	Added DS5000_100 Support. 
 *
 *  3-Aug-90	rafiey (Ali Rafieymehr)
 *	Added VAX9000 support.
 *
 * 05-Jun-90
 *      Added Mariah (VAX65xx) support.
 *
 * 20-Mar-90	Lisa Allgood
 *	Fix old SMM values.
 *
 * 20-Dec-89	Giles Atkinson
 *	Improve support for starting CPUs on a multi-processor
 *	and fix some bugs in VAX SMM recognition.
 *	Reflect new meaning of ws_display_type.
 *
 * 23-Oct-89	Giles Atkinson
 *	SMM determination for MIPS processors
 *
 * 21-Sep-89 	jaw
 *	put in locking in lmf_auth.
 *
 * 29-Jun-89	Giles Atkinson
 *	Added P_FAMILY changes and kernel authorization call.
 *
 * 30-May-89	darrell
 *	Added include of ../../machine/common/cpuconf.h -- cpu types
 *	were moved there.
 *
 *  6 Dec 1988		Lisa Allgood and Giles Atkinson
 *	Original version
 *
 */

/* This module is used only when the kernel is build with _LMF_ */

/* The P_FAMILY option is not implemented at this time.  Code from
 * ULTRIX for p_families is retained, but it is not compiled when
 * the following define sets P_FAMILY to 0. -- carlosc
 */
#define P_FAMILY 0

/* TIN doesn't support any multi-processor systems.  Thus the number of 
 * cpus running is always 1.  The ULTRIX code for multi-processor systems
 * has been left in place, but some of it has not been ported and is
 * conditional under the MULTI_PROCESSOR flag.  The following #define
 * sets that flag to 0 and effectively removes the code. -- carlosc
 */
#define MULTI_PROCESSOR 0

/* Some CPU types are not defined in TIN.  The ULTRIX code for these
 * cpus has been left in place, but it is conditional under the
 * EXTRA_CPU_TYPE flag.  The following define sets that flag to 0 and
 * effectively removes the code. -- carlosc
 */
#define EXTRA_CPU_TYPE 0

#if P_FAMILY
/* This is the ULTRIX lock for process queues.  It must be replaced
 * by the corresponding Silver lock when family licenses are implemented.
 * -- carlosc
 */
extern lock_data_t lk_procqs;
#endif /*P_FAMILY*/

/* The variable lmf_debug is set to 1 by mentioning it in the
 * boot command; otherwise, it is zero.
 * The constant LMF_DEBUG must be defined here as 0 or 1. -- carlosc
 */
#define LMF_DEBUG 0
int lmf_debug;

#include <sys/param.h>      /* from OSF, unchanged */
#include <sys/systm.h>      /* from OSF, unchanged */
#include <sys/exit_actn.h>  /* from ULTRIX for LMF */
#include <sys/user.h>       /* from OSF, add u_exitp field */
#include <sys/errno.h>      /* from OSF, unchanged */
#include <sys/lmfklic.h>    /* from ULTRIX for LMF -- includes sys/lmf.h */
#include <hal/lmf_smm.h>    /* from ULTRIX for LMF */
#include <sys/proc.h>       /* from OSF, unchanged */
#include <kern/lock.h>      /* from OSF, unchanged */
#include <io/common/devio.h> /* from OSF, unchanged */
#include <hal/cpuconf.h>    /* from OSF, unchanged */

/*
 *	Machine dependent part of the Kernel part of the Ultrix LMF.  
 *	The get/setlminfo functions are called from the get/setinfo 
 *	system calls.
 */

/* Permanent LMF data */

extern int lmf_smm;			/* System marketing model */
extern klic_t *klic_head;		/* Head of klic_t chain */
extern int lic_cpus;			/* Number of CPUs currently allowed */
extern unsigned short *smm_tablep;	/* Pointer to table of MP SMMs */

/* External references */

extern int smp_debug;

/* Functions */

void lmf_start_cpu();
void set_smp_smm();
int set_lmf_smm();
int count_cpus();

/* Functions associated with changes to the SMM caused by starting and
 * stopping processors in a multi-processor machine.
 * See also the SETSMM sub-function.
 */

#if MULTI_PROCESSOR

/* This is called whenever a processor starts up.
 * If the current SMM does not reflect the use of a new processor
 * then disable all products whose unit charge depends on the SMM
 * and whose licenses are now too small.
 * Unfortunately we cannot re-enable on CPU shutdown as that happens
 * asynchronously.
 */

void
lmf_start_cpu() {
	register int max_cpus;
	register klic_t *kp;                /* Points into unit cache */
	register int act_cpus;

	if (smm_tablep && (max_cpus = *smm_tablep) > 1 &&
	    lic_cpus < max_cpus &&
	    lic_cpus < (act_cpus = count_cpus())) {

		/* Block licenses */

		for (kp = klic_head; kp; kp = kp->kl_next) {
			if (kp->kl_flags.fl_used_smm &&
			    kp->kl_max_cpus < act_cpus)
				kp->kl_flags.fl_blocked = 1;
		}
	}
}

#endif /*MULTI_PROCESSOR*/

/* Set up the SMM manipulation in an MP system.
 * Assumes that all available CPUs will be used.
 * Argument is array with first entry a count of SMM values which follow.
 * **** This must not have more than MAX_SMMD-1 elements (including count) ****
 */

static void
set_smp_smm(table)
unsigned short table[];
	{
	register int cpus;
	extern int cpu_avail;		/* Number of CPUs autoconfig found */

	smm_tablep = table;		/* Save table pointer */
	cpus = table[0] < cpu_avail ? table[0] : cpu_avail;
	lic_cpus = cpus;
	lmf_smm = table[cpus];
}

/*  NOTE.  At the time this module is incorporated in a release, this
 *  routine must be updated to reflect the current product line. -- carlosc
 */

/* Function to set the SMM when the system is booted
 *
 * The first #define below sets the threshold for distinguishing
 * DECxxx2100 from 3100.   It is the geometric mean of the observed
 * values for the two processors.   If it needs to be recalculated,
 * compile this code with -DLMF_T_TEST to turn on the printf()s.
 */

#define THRESH 2307
#define WLIM   100000			/* Maximum wait for clock tick */

set_lmf_smm() {
	register unsigned int slow, gt, i;
	register unsigned char *ws_typep;
	extern int cpu;
	extern int ws_display_type, ws_display_units; 

	/* SMM table for PMAX variants */

	static unsigned short pmax_tbl[][3] = {
					SMM_M3100T, SMM_M3100M, SMM_M3100C,
					SMM_M2100T, SMM_M2100M, SMM_M2100C};

	/* SMM table for ISIS variants */

	static unsigned short isis_tbl[] = {4,
					    SMM_M5810, SMM_M5820,
					    SMM_M5830, SMM_M5840};

	/* Dermine the type of graphics on this system even if it
	 * is multi-headed.  If there are multiple display types
	 * on the system, the most interesting is the one with the
	 * highest type code.
	 *
	 * ******* This may not always be true. ********
	 *
	 * There are up to four values stored as an array of bytes
	 * in ws_display_type with bits to show which are meaningful in
	 * ws_display_units.
	 */

	gt = 0;
	ws_typep = (unsigned char *) &ws_display_type; 
	i = ws_display_units & 0xf;
	while (i) {
		if ((i & 1) && gt < *ws_typep)
			gt = *ws_typep;
		++ws_typep;
		i >>= 1;
	}

	switch (cpu) {
	case DS_3100:			/* PMAX = DECstation/system x100 */ 

		/* Is this the slow or fast CPU?
		 * The only way to find out is by timing, and the ULTRIX
                 * original handles the decision this way:
		 *    Wait for timepick to switch values, then count
		 *    loop executions until it switches back.
		 *    The counting function is in assembler to protect
		 *    against compiler and optimisation level changes.
                 * HOWEVER, this Hercules version handles the decision
                 * in this expedient manner:
                 *    Always assume a slow CPU
                 * Therefore, software will be charged the lesser of the
                 * two possible availability charges.  -- carlosc
		 */

                slow = 1;

		/* Convert graphics type to an array index */

		if (gt) {			/* Workstation */
			if (gt == PMC_DTYPE)	/* Colour */
				gt = 2;
			else
				gt = 1;
		}
		
		/* Now look up the SMM value */

		lmf_smm = pmax_tbl[slow][gt];
		break;

	case DS_5400:			/* MIPSfair = DECsystem 5400 */
		lmf_smm = SMM_M5400;
		break;

	case DS_5800:			/* ISIS = DECsystem 58xx */
		set_smp_smm(isis_tbl);
		break;

	case DS_5000:			/* 3MAX = DECstation/system 50xx ? */
		/* This is one of the 4 types of 3MAX workstation/timeshare */

		switch (gt) {
		case 0:	lmf_smm = SMM_M5000T;	/* No graphics - timeshare */
			break;
		case CFB_DTYPE:
			lmf_smm = SMM_M5000C;	/* Ordinary colour */
			break;
#if EXTRA_CPU_TYPE
		case PX_DTYPE:
			lmf_smm = SMM_M5000_2;	/* 2D accelerator */
			break;
		case PXG_DTYPE:
			lmf_smm = SMM_M5000_3;	/* 3D accelerator */
			break;
#endif /*EXTRA_CPU_TYPE*/

		default:
			lmf_smm = SMM_M5000C;	/* Assume ordinary colour */
			break;
		}
		break;

	case DS_5000_100: /* 3MIN = DECstation/system 5000 Model 100 */
		/* This is one of the 4 types of 3MAX workstation/timeshare */

		switch (gt) {
		case 0:	lmf_smm = SMM_M5000_100T; /* No graphics - timeshare */
			break;
		case CFB_DTYPE:
			lmf_smm = SMM_M5000_100C;	/* Ordinary colour */
			break;
#if EXTRA_CPU_TYPE
		case PX_DTYPE:
			lmf_smm = SMM_M5000_100_2;	/* 2D accelerator */
			break;
		case PXG_DTYPE:
			lmf_smm = SMM_M5000_100_3;	/* 3D accelerator */
			break;
#endif /*EXTRA_CPU_TYPE*/

		default:
			lmf_smm = SMM_M5000_100C; /* Assume ordinary colour */
			break;
		}
		break;

	case DS_5000_300: /* 3MAX+ and BIGMAX - KN03 Processor systems */

		switch (gt) {
		case 0:	lmf_smm = SMM_M5000_300T; /* No graphics - timeshare */
			break;
		case CFB_DTYPE:
			lmf_smm = SMM_M5000_300C;	/* Ordinary colour */
			break;
#if EXTRA_CPU_TYPE
		case PX_DTYPE:
			lmf_smm = SMM_M5000_300_2;	/* 2D accelerator */
			break;
		case PXG_DTYPE:
			lmf_smm = SMM_M5000_300_3;	/* 3D accelerator */
			break;
#endif /*EXTRA_CPU_TYPE*/

		default:
			lmf_smm = SMM_M5000_300C; /* Assume ordinary colour */
			break;
		}
		break;

#if EXTRA_CPU_TYPE
	case DS_MAXINE:	   /* MAXine = DECstation/system 5000 Model 50 */

		switch (gt) {
		case 0:	lmf_smm = SMM_MAXINET;	/* No graphics - timeshare */
			break;
		case CFB_DTYPE:
			lmf_smm = SMM_MAXINEC;	/* Ordinary colour */
			break;
		case PX_DTYPE:
			lmf_smm = SMM_MAXINE_2;	/* 2D accelerator */
			break;
		case PXG_DTYPE:
			lmf_smm = SMM_MAXINE_3;	/* 3D accelerator */
			break;
		default:
			lmf_smm = SMM_MAXINEC;	/* Assume ordinary colour */
			break;
		}
		break;
#endif /*EXTRA_CPU_TYPE*/

	case DS_5100:			/* MIPSMATE = DECsystem 5100  */
		lmf_smm = SMM_M5100;
		break;

	case DS_5500:			/* MIPSfair 2 = DECsystem 5500  */
		lmf_smm = SMM_M5500;
		break;

	case DS_CMAX:			/* CMAX = ? */
		lmf_smm = SMM_MCMAX;
		break;
	}

#ifdef LMF_T_TEST
	printf("LMF smm = %d\n", lmf_smm);
#endif
}

/* Count the number of active processors */

int
count_cpus() {

#if MULTI_PROCESSOR
	register struct cpudata *cp, **cpp;
	register int count = 0;

	for (cpp = cpudata; cpp < cpudata + MAXCPU; ++cpp) {
		if ((cp = *cpp) && (cp->cpu_state & CPU_RUN))
			++count;
	}
	return count;
#elseif
        return 1;
#endif /*MULTI_PROCESSOR*/
}

