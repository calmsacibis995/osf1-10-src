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
static char	*sccsid = "@(#)$RCSfile: autoconf.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:39:39 $";
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
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
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
 * Copyright (c) 1982 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *

 */

#include <cpus.h>

#include <mmax_xpc.h>
#include <mmax_apc.h>
#include <mmax_dpc.h>
#include <mmax_idebug.h>

#include <emc.h>

extern struct devaddr emc_devaddr[];

/*
 * Setup the system to run on the current machine.
 *
 * Configure() is called at boot time and initializes the device tables.
 * Available devices are determined (from possibilities mentioned in ioconf.c),
 * and the drivers are initialized.
 *
 */

#include <sys/types.h>
#include <sys/systm.h>
#include <mach/boolean.h>
#include <mach/machine.h>
#include <kern/assert.h>

#include <mmax/cpu.h>
#include <mmaxio/io.h>
#include <mmaxio/crqdefs.h>
#include <mmax/sccdefs.h>
#include <mmax/boot.h>
#include <mmax/cpudefs.h>

#include <vm/vm_kern.h>
#include <mach/vm_param.h>

/*
 * The following several variables are related to
 * the configuration process, and are used in initializing
 * the machine.
 */
int	cold;		/* if 1, still working on cold-start */
int	cpuspeed = 1;	/* relative cpu speed */

extern	BOOT	Boot;		/* System boot data structure	*/

char	*dev_type_to_name();

#if	MMAX_ULTRA
#define	ALL_SLOTS		TOTAL_SLOTS
#define	MAKE_CPUID(slot,dev)	(((slot)/NUM_SLOT) * NUM_SLOT * NUM_SLOT_DEV +\
				 (((slot)%NUM_SLOT) * NUM_SLOT_DEV) + (dev))
#define	LOG_BOARD(x,t,a)	printf("LAMP %d, Slot %2d:  %s%s\n", \
				       SLOTTOLAMP(x), LOCALSLOT(x), \
				       dev_type_to_name(t), a)
#else
#define	ALL_SLOTS		NUM_SLOT
#define	MAKE_CPUID(slot,dev)	(((slot) * NUM_SLOT_DEV) + (dev))
#define	LOG_BOARD(x,t,a)	printf("Slot %2d:  %s%s\n", \
				       x, dev_type_to_name(t), a)
#endif

/*
 * Determine mass storage and memory configuration for a machine.
 * Get cpu type, and then switch out to machine specific procedures
 * which will probe adaptors to see what is out there.
 */
configure()
{
	int	emc_num, slot_num;
	int	i, j;
	int	cpuid;
	int	ncpus, slottype, type, subtype, wanted;
#if	MMAX_ULTRA
	int	uic_num;
#endif
#include <lv.h>
#if NLV > 0
	extern int lvprobe();
#endif

	extern int	numcpus;

#if	MMAX_IDEBUG
	ASSERT(!icu_ints_on());
#endif

	/*
	 * Initialize CRQ communications.  Also inititalizes interrupt vector
	 * table before spl0 below.
	 */
	initcrq();

	/*
	 * On the MULTIMAX, initialize the SCC (interrupts must
	 * be enabled and the physical-to-virtual memory pool must
	 * be set up).
	 */
	init_scc();
#if	MMAX_ULTRA
	init_remote_sccs();
#endif

#if	MMAX_XPC
	type = CPU_TYPE_NS32532;
	subtype = (*XPCREG_CSR & XPCCSR_FPA_PRES) ?
		CPU_SUBTYPE_MMAX_XPC_FPU :
		CPU_SUBTYPE_MMAX_XPC_FPA;
	ncpus = 2;
	wanted = XPC;
#endif
#if	MMAX_APC
	type = CPU_TYPE_NS32332;
	subtype = (*APCREG_CSR & APCCSR_32081_PRES) ?
		    CPU_SUBTYPE_MMAX_APC_FPU :
		    CPU_SUBTYPE_MMAX_APC_FPA;

	ncpus = 2;
	wanted = APC;
#endif
#if	MMAX_DPC
	type = CPU_TYPE_NS32032;
	subtype = CPU_SUBTYPE_MMAX_DPC;
	ncpus = 2;
	wanted = DPCII;
#endif

	emc_num = 0;
	numcpus = 0;
	for (i = 0; i < ALL_SLOTS; i++) {
		slottype = Boot.boot_slotid[i];
		if (slottype == wanted) {
			for (j = 0; j < ncpus; j++) {
				cpuid = MAKE_CPUID(i, j);
				machine_slot[cpuid].is_cpu = TRUE;
				machine_slot[cpuid].cpu_type = type;
				machine_slot[cpuid].cpu_subtype = subtype;
			}
			numcpus += ncpus;
			LOG_BOARD(i, slottype, " configured");
			continue;
		}

	if (i == SCC_SLOT) {	/* cheat */
			LOG_BOARD(i, SCC, "");
			continue;
		}

		switch (slottype) {
		    case UNDEF_BD:	/* nothing there */
			break;
		    default:
			LOG_BOARD(i, slottype, " (ignored)");
			break;
		    case DPC:
		    case DPCII:
		    case APC:
		    case XPC:
			LOG_BOARD(i, slottype, " (ignored)");
			break;
		    case EMC:
		    case EMCII:
		    case EMCDIF:
		    case MSC:
			/* Found an I/O card.  Initialize it. */
			if (emc_num < NEMC) {
				if (init_emc(i, emc_num) == KERN_SUCCESS) {
					LOG_BOARD(i, slottype, " configured");
#if	MMAX_ULTRA
					emc_devaddr[emc_num].v_lamp = SLOTOLAMP(i);
#endif
					emc_devaddr[emc_num].v_chan = i;
					emc_devaddr[emc_num].v_dev = i;
					emc_devaddr[emc_num].v_valid = DEV_INITIALIZED;
					msd_config(emc_num);
					if (slottype != MSC)
						enattach(emc_num);
				} else {
					LOG_BOARD(i, slottype, " FAILED INITIALIZATION!");
				}
				emc_num++;
			} else {
				/* device in backplane but not expectedd */
				LOG_BOARD(i, slottype, " unconfigured and ignored");
			}
			break;
		    case UIC:
#if	MMAX_ULTRA
			if (uic_num < NUIC) {
				if (init_uic(i, uic_num) == KERN_SUCCESS)
					LOG_BOARD(i, slottype, " configured");
				uic_num++;
			} else
				LOG_BOARD(i, slottype, " unconfigured and ignored");
#else
			LOG_BOARD(i, slottype, " (ignored)");
#endif
			break;
		}
	}
	machine_slot[master_cpu].running = TRUE;

	spl0();

	slcinit();	/* Initialize the scc serial port driver */

#if NLV > 0
	lvprobe(0);
#endif
	/*
	 * Configure swap area and related system
	 * parameter based on device(s) used.
	 */
	swapconf();
	cold = 0;

	printf("Master cpu at port %d.\n", master_cpu);

	return;
}

slave_config()
{
	register int i;
	int	this_cpu;

	this_cpu = cpu_number();
	machine_slot[this_cpu].running = TRUE;

}

/*
 * Configure swap space and related parameters.
 */

#define PAR_MASK 0xff
#define DEV_MASK 0xffffff00
#define DEV_SHIFT 8

swapconf()
{
	dev_t	ms_bootdev();
	int	partn;
	long    dev;

	partn = Boot.boot_rootdev & PAR_MASK;
	rootdev = ms_bootdev(Boot.boot_unitid, partn);

	/*
	 *	Dump device is specified in boot structure. (from param file)
	 */
	partn = Boot.boot_dumpdev & PAR_MASK;
	dev = (Boot.boot_dumpdev & DEV_MASK) >> DEV_SHIFT;
	dumpdev = ms_bootdev(dev, partn);

	/*
	 *	We swap to normal file systems for new VM.
	 */

	dumplo = 0;		/* make it easy to look in /dev/xxb */
}

/*
 * Start the remaining (non-boot) processors in the system by sending
 * (and waiting for) execute program messages.
 */

/* Startup messages are statically allocated here so virt=phys */
#define NUMSTARTMSGS	4
crq_exec_msg_t	start_cpu_msgs[NUMSTARTMSGS];

extern BOOT	Boot;		/* boot configuration data */

start_cpus(startaddr)
long	startaddr;
{
	register crq_exec_msg_t	*cmd = &start_cpu_msgs[0];
	crq_exec_msg_t		*rsp;
	register int		i;
	register crq_t		*crq;
	cpu_type_t		type;
	cpu_subtype_t		subtype;
	unsigned		s;
	int			cpus = 1;
#if	MMAX_XPC
	xpccsr_t		sts;
#endif
#if	MMAX_APC
	apccsr_t		sts;
#endif
#if	MMAX_DPC
	dpcsts_t		sts;
#endif

	type = machine_slot[cpu_number()].cpu_type;
	subtype = machine_slot[cpu_number()].cpu_subtype;
	printf("Cpu %d starting other cpus.\n", cpu_number());

	for (i = 0; i < NCPUS; i++) {
		if ((Boot.boot_maxcpus) && (cpus == Boot.boot_maxcpus))
			break;
		if ((!machine_slot[i].is_cpu) || (i == master_cpu))
				continue;

		type = machine_slot[i].cpu_type;
		subtype = machine_slot[i].cpu_subtype;

		/* Fill out start message */
		cmd->exec_start_addr = startaddr;
		cmd->exec_hdr.crq_msg_code = CRQOP_EXEC_PROG;
		sts.l = (long)i;
		cmd->exec_hdr.crq_msg_unitid =
			MAKEUNITID(0, sts.f.s_slotid, sts.f.s_cpuid, REQ_LUN);

		/* Aim the start message at the target cpu's command queue. */
		crq = REQ_CRQ(sts.f.s_slotid, sts.f.s_cpuid);
		s = spl7();
		crq_lock(&crq->crq_slock);
		insque(cmd, crq->crq_cmd.dbl_bwd);
		cmd->exec_hdr.crq_msg_status = STS_QUEUED;
		crq_unlock(&crq->crq_slock);
		splx(s);

		/* Wait for response or time out */
		rsp=(crq_exec_msg_t *)rec_polled_rsp(REQ_CRQ(sts.f.s_slotid,
							     sts.f.s_cpuid));
		if (rsp != NULL) {
			switch (rsp->exec_hdr.crq_msg_status) {
			    case STS_SUCCESS:
				printf("Cpu %d started.\n", i);
				cpus++;
				break;
				
			    default:
				printf("Cpu %d FAILED to start, sts 0x%x\n",
				       i, rsp->exec_hdr.crq_msg_status);
				break;
			}
		} else {
			/*
			 * Cpu failed to acknowledge, abandon original message
			 * because heaven only knows what the hardware is
			 * doing with it.
			 */
			printf("Cpu %d FAILED to acknowledge\n", i);
			cmd++;
			if(cmd == &start_cpu_msgs[NUMSTARTMSGS]) {
				printf("Too many acknowledge failures -- Aborting cpu startup.\n");
				return;
			}
		}
	}

	printf("Cpu startup done, %d cpus running.\n", cpus);
}

/*
 * Acknowlege a cpu start
 */
ack_cpustart()
{
	register int	i;
	register crq_t	*cpu_crq;
	crq_msg_t	*rsp;
	unsigned	s;

	cpu_crq = REQ_CRQ(GETCPUSLOT, GETCPUDEV);
	/*
	 * Turn the execute program command into a respone
	 */
	s = spl7();
	crq_lock(&cpu_crq->crq_slock);
	if (cpu_crq->crq_cmd.dbl_fwd == (dbl_link_t *)(&cpu_crq->crq_cmd.dbl_fwd))
		panic("\nAck_cpustart: no command queued");
	rsp = (crq_msg_t *)remque(cpu_crq->crq_cmd.dbl_fwd);
	if (rsp->crq_msg_code != CRQOP_EXEC_PROG)
		panic("\nAck_cpustart: no execute program queued");
	rsp->crq_msg_status = STS_SUCCESS;
	insque(rsp, cpu_crq->crq_rsp.dbl_bwd);
	crq_unlock(&cpu_crq->crq_slock);
	splx(s);
}


static char	unknown[] = "Unknown";
static char	*device_names[] = {
	"Ethernet / Mass Storage Card (SE/NCR/LCS)",
	"Dual Processor Card",
	"System Control Card",
	unknown,
	"Ethernet / Mass Storage Card (SE/DTC)",
	"Dual Processor Card II",
	"Shared Memory Card (4MB)",
	"Shared Memory Card II (4MB)",
	"System Control Card II",
	"Advanced Dual Processor Card",
	unknown,
	unknown,
	unknown,
	"Mass Storage Card",
	"Shared Memory Card (16MB)",
	unknown,
	unknown,
	unknown,
	unknown,
	"Mass Storage Card modified for real-time CPU tracing",
	"Gigamax Interconnect Card",
	"Extended Dual Processor Card",
	unknown,
	"Ethernet / Mass Storage Card (DE)"
};

char *
dev_type_to_name(type)
int	type;
{
	char	*name;

	if (type > UNDEF_BD && type < UNKNOWN_BD)
		return device_names[type-1];
	else
		return "unrecognized device type";
}
