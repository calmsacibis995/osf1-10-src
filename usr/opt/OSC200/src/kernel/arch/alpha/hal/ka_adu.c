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
static char *rcsid = "@(#)$RCSfile: ka_adu.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/06/14 18:11:25 $";
#endif

/*
 *	@(#)$RCSfile: ka_adu.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/06/14 18:11:25 $
 */

/************************************************************************
 *									*
 *			Copyright (c) 1990 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any	other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or	reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/*
 * Modification History: machine/alpha/ka_adu.c
 *
 * 08-Aug-91	afd
 *	Added tvbus config code to be compatible with new bus/ctlr structs.
 *
 * 16-May-91	afd
 *	Rewrote the adu_map_io() and tv_map_reg() routines to use
 *	new OSF vm concepts.
 *
 * 13-Dec-90	rjl
 *	Added adu interrupt handling and vector allocation support.
 *
 * 12-Oct-90	Tim Burke
 *	Added machine check support: adu_machcheck() and adu_consprint().
 *
 * 04-Sep-90	afd
 *      Created this file for processor support of the Alpha ADU.
 *
 */

#include <sys/types.h>
#include <machine/psl.h>
#include <machine/rpb.h>
#include <machine/scb.h>
#include <machine/clock.h>
#include <machine/cpu.h>

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/map.h>
#include <sys/buf.h>
#include <sys/dk.h>
#include <sys/vm.h>
#include <sys/conf.h>
#include <sys/reboot.h>
#include <sys/devio.h>
#include <sys/user.h>
#include <sys/kernel.h>
#include <sys/proc.h>
#include <mach/machine.h>
#include <sys/vmmac.h>
#include <sys/conf.h>
#include <sys/table.h>

#include <machine/reg.h>
#include <machine/entrypt.h>

#include <hal/cpuconf.h>
#include <io/common/devdriver.h>

#include <hal/ka_adu.h>

#ifndef	OSF
#include <dec/binlog/errlog.h>
#else /* OSF */
#define EL_PRISEVERE	1
#define EL_PRIHIGH	3
#define EL_PRILOW	5
#define ELMCKT_ADU_COR	1
#define ELMCKT_ADU_ABO	2
#endif /* OSF */

/*
 * Defines used in user mode recovery from machine checks.
 */
#define OKTOTERM(pid)	((pid) != 1)	/* Do not kill init on machine check */

int adu_mchk_ip = 0;            /* set to 1 when machine check in progress */
int adu_correrr_ip = 0;         /* set to 1 when corrected error in progress */

extern struct cpusw *cpup;	/* pointer to cpusw entry */
extern int cold;		/* for cold restart */
extern int printstate;		/* thru console callback or ULTRIX driver */

struct tv_slot 	tv_slot[TV_SLOTS];	/* table with IO device info */

/*
 * adu_tv_cons_base must be set up prior to calling the console init routine.
 */
u_long *adu_tv_cons_base;	/* base addr of regs on console I/O module */
struct tv_config *tv_cnfg_base; /* address of the tv bus configuration */

char *tv_map_reg();

#define TVNSLOTS 16		/* number of possible tv slots		*/

/*
 * Alpha ADU initialization routine.
 */

adu_init()
{
	return(0);
}

/*
 * Alpha ADU configuration routine.
 */

adu_conf()
{
	struct tv_config *tvc;		/* node configuration ptr */
	register struct bus *sysbus;
	int i;

	/*
	 * Working on cold restart (for autoconf and kern_cpu)
	 */
	cold = 1;

	/*
	 * Say what system we are
	 */
	printf("Alpha ADU system\n");

	master_cpu = 0;
#if	NCPUS > 1
	printf("Master cpu at slot %d.\n", master_cpu);
#endif	/* NCPUS > 1 */
	machine_slot[master_cpu].is_cpu = TRUE;
	machine_slot[master_cpu].cpu_type = CPU_TYPE_ALPHA;
	machine_slot[master_cpu].cpu_subtype = CPU_SUBTYPE_ALPHA_ADU;
	machine_slot[master_cpu].running = TRUE;
	machine_slot[master_cpu].clock_freq = hz;

	/*
	 * We now have the scb set up enough so we can handle
	 * interrupts if any are pending.
	 */
	(void) spl0();

	/*
	 * Config the TVbus
	 * The indirect call to (*sysbus->confl1)(-1, 0, sysbus) calls
	 * "tvbusconfl1" in this file.
	 */
	system_bus = sysbus = get_sys_bus("tvbus");
	(*sysbus->confl1)(-1, 0, sysbus);
	(*sysbus->confl2)(-1, 0, sysbus);

	/*
	 * Clear any error bits from I/O probes and return
	 */

	cold = 0;
	return(0);
}

/*
 * Examine the TVbus config struct to see what modules are really there
 * map I/O space for the I/O module(s).
 *
 * Save the virtual address of any I/O modules for later use.
 *
 * This is called thru the CPU switch from startup.
 */
adu_map_io()
{
	struct tv_config *tvc;		/* node configuration ptr */
	int io_node=0;			/* I/O node number	*/
	int i;

	/*
	 * Clear the tv_slot table to "safe" initialized values
	 */
	for (i = 0; i < TV_SLOTS; i++) {
		tv_slot[i].module_type = -1;
		tv_slot[i].virtaddr = 0;
	}

	/*
	 * Map the tvbus configuration space.
	 * tv_cnfg_base is allocated in "sys_space"
	 */
	if (pmap_map_io(mdt->rpb_impaddr, NBPG, tv_cnfg_base, VM_PROT_READ,
	    TB_SYNC_LOCAL) != KERN_SUCCESS)
		panic("Can't map TVbus config table");
	/*
	 * Assume that the number of processor slots is the number of
	 * bus slots not the actual number of installed processors.
	 * Find the I/O modules and map their register space.
	 */
	for (i = 0, tvc = tv_cnfg_base; i < TVNSLOTS; i++, tvc++) {
		tv_slot[i].module_type = tvc->tv_type;

		switch (tvc->tv_type) {
		case TV_TYPE_EMPTY:
			break;

		case TV_TYPE_IO:
			tv_slot[i].virtaddr = tv_map_reg(i);
			break;

		case TV_TYPE_MEM64:
			break;

		case TV_TYPE_CPU3:
			break;

		case TV_TYPE_CPU4:
			break;

		default:
			break;
		}
	}
	return(0);
}

/*
 * Map the register space for an I/O module
 * The TVbus slot number that the I/O module resides in, is passed as "io_node"
 */

char *
tv_map_reg(io_node)
	int io_node;			/* I/O node number	*/
{
	unsigned long iobase;		/* Physical I/O base addr */
	char *basevaddr;		/* virtual addr of reg base */

	iobase = TVIOREGBASE | io_node << 28;

	/*
	 * Map the reg space for the (1 and only) I/O module.
	 * adu_tv_cons_base is allocated in "sys_space"
	 */
	if (pmap_map_io(iobase, TVIOMAPSIZE * NBPG, adu_tv_cons_base,
	    VM_PROT_READ|VM_PROT_WRITE, TB_SYNC_LOCAL) != KERN_SUCCESS)
		panic("Can't map TVbus registers");
	basevaddr = (char *) adu_tv_cons_base;

	return(basevaddr);
}


/*
 * Configure the TVbus
 *
 * The ibus config routines call the driver's probe routine.
 */
tvbusconfl1(bustype, binfo, bus)
	int bustype;
	caddr_t binfo;
	struct bus *bus;
{
	register int i, j, k, l;
	int s;
	int found;
	struct tv_config *tvc;		/* node configuration ptr */
	register struct controller *ctlr;
	int savebus;
	char *savebusname;

	/* Only support TVbus as system bus */
	if (bustype != -1)
		panic("TVbus not system bus");

	bus->bus_type = BUS_TV;
	bus->alive = 1;
	printf("%s%d at nexus\n",  bus->bus_name, bus->bus_num);

	/* "probe" the TVbus */
	for (i = 0, tvc = tv_cnfg_base; i < TVNSLOTS; i++, tvc++) {
		/*
		 * What type of module is this?
		 */

		switch (tvc->tv_type) {

		case TV_TYPE_EMPTY:
			break;

		case TV_TYPE_MEM64:
			printf("\tMemory\tnode %d Status=%b\n", i, tvc->tv_flag,
				TVSTATUSBITS);
			break;

		case TV_TYPE_CPU3:
			printf("\tEV3 CPU\tnode %d Status=%b\n", i, tvc->tv_flag,
				TVSTATUSBITS);
			break;

		case TV_TYPE_CPU4:
			printf("\tEV4 CPU\tnode %d Status=%b\n", i, tvc->tv_flag,
				TVSTATUSBITS);
			break;

		default:
			printf("\tUNKNOWN\tnode %d type=%x\n", i, tvc->tv_type);
			break;

		case TV_TYPE_IO:
			printf("\tI/O\tnode %d Status=%b\n", i, tvc->tv_flag,
				TVSTATUSBITS);
			/*
			 * For each I/O module found on the system we "know"
			 * that there are three devices: serial line, lance,
			 * and scsi controller.
			 *
			 * Call tv_config_cont for serial line, lance and scsi.
			 *
			 * The tv_config_cont routine will in turn call
			 * the probe and attach routines for each controller.
			 */
			for (j = 0; j < 3; j++) {
				switch (j) {
				case 0:
					strcpy(tv_slot[j].name, "aducn");
					break;
				case 1:
					strcpy(tv_slot[j].name, "adusz");
					break;
				case 2:
					strcpy(tv_slot[j].name, "aduln");
					break;
				}
				if (!(ctlr = get_ctlr(tv_slot[j].name, -1, bus->bus_name, bus->bus_num)))
					printf("ctlr struct for %s not found\n",tv_slot[j].name);
				else {
					savebus = ctlr->bus_num;
					savebusname = ctlr->bus_name;
					ctlr->bus_name = bus->bus_name;
					ctlr->bus_num = bus->bus_num;
					if (!(tv_config_cont(tv_slot[i].virtaddr, (long)i, tv_slot[j].name, ctlr))) {
					    printf("%s in slot %d not probed\n", tv_slot[j].name, i);
					    ctlr->bus_num = savebus;
					    ctlr->bus_name = savebusname;
					} else
					    conn_ctlr(bus, ctlr);
				}
			}
		} /* end switch */
	}

#ifdef notdef
Do we need this for the ADU? Maybe not since we set up the scb in
adu_vector_alloc(ipl,handler,parameter)
	/*
	 * Install the static interrupt handlers
	 */
	handler_install();
#endif notdef
	return(0);
}

tvbusconfl2(bustype, binfo, bus)
	int bustype;
	caddr_t binfo;
	struct bus *bus;
{
	return(0);
}

extern  struct device   device_list[];

tv_config_cont(nxv, slot, name, ctlr)
	char *nxv;
	u_long slot;
	char *name;
	struct controller *ctlr;
{
	register struct driver *drp;
	register struct device *device;
	int savectlr;
	char *savectname;
	int (**ivec)();
	int i;
	int found = 0;

	if (ctlr->alive)
		return(0);

	drp = ctlr->driver;

	i = (*drp->probe)(nxv, ctlr);
	if (i == 0)
	    return(0);
	ctlr->slot = slot;
	ctlr->alive = 1;
	ctlr->addr = (char *)nxv;
	(void)svatophys(ctlr->addr, &ctlr->physaddr);
	drp->ctlr_list[ctlr->ctlr_num] = ctlr;
	config_fillin(ctlr);
	printf("\n");
	if (drp->cattach)
		(*drp->cattach)(ctlr);

	for (device = device_list; device->dev_name; device++) {
		if (((device->ctlr_num != ctlr->ctlr_num) &&
			 (device->ctlr_num !=-1) && (device->ctlr_num != -99)) ||
			((strcmp(device->ctlr_name, ctlr->ctlr_name)) &&
			 (strcmp(device->ctlr_name, "*"))) ||
			(device->alive) )

			continue;

		savectlr = device->ctlr_num;
		savectname = device->ctlr_name;
		device->ctlr_num = ctlr->ctlr_num;
		device->ctlr_name = ctlr->ctlr_name;

		if ((drp->slave) && (*drp->slave)(device, nxv)) {
			device->alive = 1;
			conn_device(ctlr, device);
			drp->dev_list[device->logunit] = device;
			if(device->unit >= 0)
				printf("%s%d at %s%d unit %d",
				    device->dev_name, device->logunit,
				    drp->ctlr_name, ctlr->ctlr_num, device->unit);
			else
				printf("%s%d at %s%d",
				    device->dev_name, device->logunit,
				    drp->ctlr_name, ctlr->ctlr_num);
			if (drp->dattach)
				(*drp->dattach)(device);
			printf("\n");
		}
		else {
			device->ctlr_num = savectlr;
			device->ctlr_name = savectname;
		}
	}
    return(1);
}

/*
 * This routine sets the cache to the state passed:  enabled/disabled.
 * Handle 1st and 2nd level cache.
 */

adu_setcache(state)
	int state;
{
	return(0);
}

/*
 * Enable cache
 */

adu_cachenbl()
{
	return(0);
}

/*
 * Enable CRD interrupts.
 * This runs at regular (15 min) intervals, turning on the interrupt.
 * It is called by the timeout call in memenable in machdep.c
 * The interrupt is turned off, in adu_crderr(), when 3 error interrupts
 *   occur in 1 time period.  Thus we report @ most once per memintvl (15 mins).
 */
adu_memenable()
{
	return(0);
}

/*
 * Delay routine for ADU
Note: this should work on ALL Alpha systems, therefor it can go
	into machdep.c as a common delay routine
 */
adu_delay(n)
	int n;		/* number of micro-seconds to delay for */
{
	register long cnt;
	long ns_per_tic;
	long tics_per_usec;
	long start_scc;

	start_scc = scc();
	ns_per_tic = 1000000000 / rpb->rpb_counter; /* 1billion ns per sec */
	tics_per_usec = 1000 / ns_per_tic;	/* 1000 ns per usec */

	while (((scc() - start_scc) / tics_per_usec) < n)
		;
}


/*
 * Machine check handler.
 * Called from locore thru the cpu switch in response to a trap at SCB 
 * locations 0x620, 0x630, 0x660, 0x670.
 * We recover from any that we can if hardware "retry" is possible.
 *
 * tim fix - remaining to do:
 *	-) Take into account cache and other related issues.
 */

adu_machcheck (type, cmcf)
	long type;
	caddr_t cmcf;
{
	register int recover;
	register int retry;
	register int clear_check_ip;
	struct mchk_logout *mchk_logout;
	u_long	psl = 0;	/* tim fix - temp, assign appropriately later */
	int	cpunum = 0;	/* tim fix - figure this out on multiprocessor*/

	recover = 0;
	clear_check_ip = 0;
	retry = 0;
	/*
	 * Granted this may not be a logout area of type mchk_logout but
	 * cast the pointer to that anyways because all this routine does
	 * is look at the first longword which is the same for all the 
	 * logout formats.
	 *
	 * The cmcf field will be set to -1 if there is no logout in memory.
 	 */
	mchk_logout = (struct mchk_logout *) cmcf;
	if (mchk_logout != ((struct mchk_logout *) -1)) {
		retry = mchk_logout->retry & RETRY_BIT;
	}

	switch (type) {
			/*
			 * The first 2 cases are for errors that have
			 * been corrected.  For this reason the retry
			 * bit in the logout area should be set.  The
			 * purpose of these interrupts is to log the
			 * frequency of occurance.
			 */
		case S_CORR_ERR: 	/* System correctable error interrupt */
		case P_CORR_ERR:	/* Processor correctable error interrupt */
			adu_correrr_ip++;
			/*
			 * For the case of multiple entry into this routine for
			 * corrected errors simply return because it is 
			 * possible that something this routine is doing is
			 * causing another correctable error.   Since all this
			 * routine does is log these anyways just ignore the
			 * recursive case.
			 */
			if (adu_correrr_ip > 1) {	
				adu_correrr_ip--;
				return(0);
			}
			if (retry == 0) {
				printf("Retry bit not set on corrected err!\n");
			}
			recover = 1;
			/* Log the error */
			break;
		case S_MCHECK: 	/* System machine check abort */
		case P_MCHECK: 	/* Processor machine check abort */
			adu_mchk_ip++;
			/*
			 * If a second machine check is detected while a 
			 * machine check is in progress, a Double Error abort
			 * is generated and the processor enters the restart
			 * sequence.  For this reason the following test should
			 * never be true.  
			 */
			if (adu_mchk_ip > 1) {
				panic("Double Error machine check abort.\n");
			}
			clear_check_ip = 1;
			/* Log the error */
			/*
			 * If the logout area indicates that a retry is 
			 * possible give it a shot.
			 */
			if (retry) {
				recover = 1;
			}
			/*
			 * The logout area indicates that a retry is not
			 * possible.  Try to salvage the system by not issuing
			 * a panic if in user mode.
			 *
			 * Note: We may not want to retry all cases of usermode
			 * machine checks.  Since the adu spec does not yet
			 * detail sub-cases of machine checks it is not
			 * possible to differentiate.
			 *
			 * We need the psl to determine if the context is user
			 * mode.  I assume that the psl is included in the
			 * machine check frame; if not it will have to be
			 * passed into this routine as a parameter.  If the
			 * psl is part of the logout area then make sure the
			 * logout pointer is not null before testing for 
			 * usermode.
			 */
			else {
				if (USERMODE(psl) && 
				    OKTOTERM(u.u_procp->p_pid)) {
					recover = 1;
				}
				else {
					adu_consprint(type, cmcf);
				}
			}
			break;
		default:
			/*
			 * Unrecognized machine check type.  Assume that these
			 * are not recoverable.  Log what you can before the
			 * system goes down.
			 */
			/* Log something here ? */
			printf("Unrecognized machine check type %d.\n",type);
			adu_consprint(type, cmcf);
			break;
	}
	if (recover == 0) {
		panic("Non-recoverable adu machine check.\n");
	}
	/*
	 * For the case of machine checks the PALcode will set an internal
	 * machine check in progress flag.  This flag must be cleared upon
	 * exit of this handler.  Failure to clear this flag will cause a 
	 * Double Error abort in the event of the next machine check.
	 *
	 * Clearing the in progress flag involves writing a 1 to the mces
	 * register.
	 */
	if (clear_check_ip) {
		adu_mchk_ip--;
	}
	else {
		adu_correrr_ip--;
	}
	return(0);
}

/*
 * Log memory errors in kernel buffer:
 *
 * Traps through SCB vector ??: uncorrectable memory errors.
 */
adu_memerr()
{
	register int recover;	/* set to 1 if we can recover from this error */
	register u_int time;	/* from TODR */

	recover = 0;
	/*
	 * First note the time; then determine if hardware retry is
	 * possible, which will be used for the recoverable cases.
	 */
	time = 0;
	printf("memerr interrupt\n");
	panic ("memory error");
}

/*
 * Log CRD memory errors in kernel buffer:
 *
 * Traps through SCB vector ??: correctable memory errors.
 *
 * These errors are recoverable.
 */
adu_crderr()
{
	int recover;	/* set to 1 if we can recover from this error */
	u_int time;

	recover = 0;
	time = 0;
	return(0);
}

/*
 * Print error packet to the console.
 * This is only done when we are about to panic on the error.
 */

adu_consprint(type, cmcf)
	int type;		/* machine check type */
	caddr_t cmcf; 		/* Pointer to the logout area */
{
	register struct mchk_logout *mc_logout;
	register struct sce_logout *sc_logout;
	register struct pce_logout *pc_logout;
	register int i;

	printstate |= PANICPRINT;

	printf("adu machine check type 0x%x.\n",type);

	switch (type) {
			/*
			 * The first 2 cases are for correctable errors.
			 * Since the retry flag should always be set for the
			 * the correctable errors this routine should never
			 * be called (since no panic is about to be done).
			 * Just to be on the safe side include the cases for
			 * these anyways.
			 */
                case S_CORR_ERR:     /* System correctable error interrupt */
			printf("System correctable error \n");
			sc_logout = (struct sce_logout *) cmcf;
			if (sc_logout == ((struct sce_logout *)-1)) {
			    printf("No logout area.\n");
			}
			else {
			    printf("\tretry\t= %x\n",sc_logout->retry);
			    printf("\tremaining fields undefined...\n");
			}
			break;
                case P_CORR_ERR:     /* Processor correctable error interrupt */
			printf("Processor correctable error \n");
			pc_logout = (struct pce_logout *) cmcf;
			if (pc_logout == ((struct pce_logout *)-1)) {
			    printf("No logout area.\n");
			}
			else {
			    printf("\tretry\t= %x\n",sc_logout->retry);
			    printf("\tremaining fields undefined...\n");
			}
			break;
                case S_MCHECK:     /* System machine check abort */
                case P_MCHECK:     /* Processor machine check abort */
			printf("Machine check abort\n");
			mc_logout = (struct mchk_logout *) cmcf;
			if (mc_logout == ((struct mchk_logout *)-1)) {
			    printf("No logout area.\n");
			}
			else {
			    printf("\tretry\t= 0x%x\n",mc_logout->retry);
			    for (i = 0; i < 32; i++) {
			        printf("\tpt[%i]\t=0x%x\n",i,mc_logout->pt[i]);
			    }
			    printf("\texc_addr\t= 0x%x\n",mc_logout->exc_addr);
			    printf("\texc_sum\t= 0x%x\n",mc_logout->exc_sum);
			    printf("\tmsk\t= 0x%x\n",mc_logout->msk);
			    printf("\tpal_base\t= 0x%x\n",mc_logout->pal_base);
			    printf("\thirr\t= 0x%x\n",mc_logout->hirr);
			    printf("\thier\t= 0x%x\n",mc_logout->hier);
			    printf("\tmm_csr\t= 0x%x\n",mc_logout->mm_csr);
			    printf("\tva\t= 0x%x\n",mc_logout->va);
			    printf("\tbui_addr\t= 0x%x\n",mc_logout->bui_addr);
			    printf("\tbui_stat\t= 0x%x\n",mc_logout->bui_stat);
			    printf("\tdc_addr\t= 0x%x\n",mc_logout->dc_addr);
			    printf("\tfill_adr\t=0x%x\n",mc_logout->fill_addr);
			    printf("\tdc_stat\t=0x%x\n",mc_logout->dc_stat);
			    printf("\tfill_syndrome\t=0x%x\n",
					mc_logout->fill_syndrome);
			    printf("\tbc_tag\t=0x%x\n",mc_logout->bc_tag);
			}
			break;
		default:
			/*
			 * Since the length is given in the first word of the
			 * logout area it would be possible to just do a hex
			 * dump of the raw packet.
			 */
			printf("Unrecognized machine check type.\n");
			break;
	}
}

/*
 * Log Error & Status Registers
 */

adu_logesrpkt(priority)
	int priority;		/* for pkt priority */
{
	return(0);
}

/*
 * Log Memory CSRs
 */

adu_logmempkt(recover)
	int recover;		/* for pkt priority */
{
	return(0);
}

/*
 * Generic ADU interrupt vector allocation support
 *
 * The alpha SCB has 512 vectors of which the first 128 belong to the
 * system.  The remaining 384 are reserved for I/O devices.
 *
 * On an ADU system the processor can be in anyone of the middle 8 slots.
 * On a 16 node tvbus this would mean that a cpu module can be in slots 4-12.
 * On a 14 node tvbus this would mean that a cpu module can be in slots 3-11.
 *
 * The SCB vectors starting with #129 (0x800) are allocated to each of the
 * possible cpu slots in groups of 32. The group of 32 is further subdivided
 * according the the IPL that the interrupt is granted at. Slots 0-23 are
 * for ipl 20 interrupts, slots 24-25 are for ipl 21, slots 26-29 are for
 * ipl 22, and slots 30-31 are for ipl 23 requests.  Slot 28 and 29 are not
 * available to system software as they are used by palcode for other
 * functions.
 *
 * The slot number is called the interrupt request channnel and is provided
 * to each of the device interfaces.  It therefore selects the device ipl
 * and the scb vector to use.  The following routine manages this allocation.
 * Each driver calls this routine to install it's interrupt handler and
 * obtain an interrupt chan and interrupt request node.
 */

#define TVVCHANS 32		/* number of vectors per tvbus slot	*/
#define NIPLS 4			/* number of ipls available		*/
#define TVCHANINIT (3<<29 | 1<<26) /* Unuseable channels		*/
/*
 * The tviovec is a shadow scb that mirrors the relevant portion of the 
 * system scb.
 */
struct scbentry tviovec[TVVCHANS];	/* array of tvbus io vectors */
extern struct scbentry _scb[];		/* system scb			*/
unsigned int tviochan=TVCHANINIT; /* array of allocd channels */ 
struct ipl_channel {
	int ipls,iple;		/* start and end channels */
} ipl_chan[4]={ {0, 23},
		{24, 25},
		{26, 29},
		{30, 31}
};


/*
 * This routine allocates an scb vector at the appropriate ipl on this
 * cpu.  The handler and parameter are registered for the general interrupt
 * handler to use when the interrupt occurs and the caller is given a cookie
 * that specifies the proper irq channel and node as follows.
 *
 *       bits 8-4 = irqchan
 *            3-0 = irqnode
 */
adu_vector_alloc(ipl,handler,parameter)
int ipl;				/* ipl to interrupt at		*/
u_long (*handler)();			/* interrupt handler		*/
long parameter;				/* parameter to pass handler	*/
{
	register int ipls, iple;	/* chan ranges for each ipl	*/
	register int irqchan;		/* returned channel		*/
	int irqnode = mfpr_whami();	/* cpu node			*/
	int scbvec;			/* scb index			*/
	register int bit;		/* bit mask			*/
	int i;

	if( ipl < 20 || ipl > 23 ){
		printf("bad ipl requested\n");
		panic("adu_vector_alloc");
	}

	/*
	 * zero base the ipl and initialize the range
	 */
	ipl -= 20;
	ipls = ipl_chan[ipl].ipls;
	iple = ipl_chan[ipl].iple;

	/*
	 * Find the next unused channel at this ipl
	 */
	for( irqchan = -1 ; ipls <= iple ; ipls++ ){
		bit = 1<<ipls;
		if( !( tviochan & bit ) ){
			tviochan |= bit;
			irqchan = ipls;
			break;
		}
	}
	/*
	 * If there is an available channel then record the handler,
	 * it's parameter, stuff the scb, and return the cookie.
	 */
	if( irqchan >= 0 ){
		scbvec = 128 + 32*(irqnode-3) + irqchan;
		tviovec[irqchan].scb_vector = handler;
		tviovec[irqchan].scb_param = parameter;
		_scb[scbvec].scb_vector = handler;
		_scb[scbvec].scb_param = parameter;
		return (irqchan<<4 | irqnode);
	} else
		return 0;
}

/*
 *  Read a standard Alpha TODR
 */
long
alpha_readtodr()
{
#ifdef notdef
	long t;
#define EPOCH 0x2d8539c80
	/*
	 * by private agreement the todr from the 3-max console is
	 * at 8160 physical and has been fudged at (time+EPOCH)*10000000
	 */
	t=ldqp(8160);
	if( t )	  		/* right now we don't get anything sane */
		t = (t/10000000)-EPOCH;
	else
		t = 1;		/* force system to use date in fs */

	return t;
#else
	/* return(mfpr_at()); */
	return 1L;	/* force system to use date in fs */
#endif

}


/*
 *  Write a standard Alpha TODR
 */
alpha_writetodr(yrtime)
	u_long yrtime;
{
}

