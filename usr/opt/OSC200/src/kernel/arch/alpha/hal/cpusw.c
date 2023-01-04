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
static char *rcsid = "@(#)$RCSfile: cpusw.c,v $ $Revision: 1.1.20.7 $ (DEC) $Date: 1993/10/19 21:49:48 $";
#endif


#include <sys/types.h>
#include <machine/cpu.h>
#include <hal/cpuconf.h>
#include <io/common/devdriver.h>

int	mcheck_expected = 0;		/* set to 1 by BADADDR; machine check is expected */
unsigned cpu_systype;                   /* systype word in boot PROM */
int	cpu;				/* ULTRIX system id (see cpuconf.h) */
struct	cpusw *cpup;			/* pointer to cpusw entry    */
struct	cpusw *cpuswitch_entry();
extern long proc_type;			/* processor type */
extern long sys_type;			/* system type (family or platform) */
extern int nocpu();

/*
 * Determine what processor and system we are running on and return
 * the ULTRIX system type.  To be used as the index into the cpu switch
 * (system specific switch table).
 *
 * Parameters:
 *	proc_type		processor type
 *	sys_type		system type (family or platform)
 * 
 * Return:
 *	Value to be stored in cpu, defined in cpuconf.h
 */
system_type(proc_type, sys_type)
	long proc_type;			/* processor type */
	long sys_type;			/* system type (family or platform) */
{
	int ret_val = UNKN_SYSTEM;	/* Assume we don't know yet */

	switch (sys_type) {
		case ST_ADU:
			ret_val = ALPHA_ADU;
			break;
		case ST_DEC_4000:
			ret_val = DEC_4000;
			break;
		case ST_DEC_3000_300:
			ret_val = DEC_3000_300;
			break;
		case ST_DEC_3000_500:
			ret_val = DEC_3000_500;
			break;
		case ST_DEC_7000:
			ret_val = DEC_7000;
			break;
		case ST_DEC_2000_300:
			ret_val = DEC_2000_300;
			break;
	}
if (ret_val == UNKN_SYSTEM)
printf ("WARNING: proc_type %x / sys_type %x unknown\n\r", proc_type, sys_type);
	return(ret_val);
}


/*
 * Get pointer to cpusw table entry for the system we are currently running on.
 *
 * The "cpu" variable (ULTRIX system type) is passed in and compared to the
 * system_type entry in the cpusw table for a match.
 *
 * Parameters:
 *	cpu			the ULTRIX system type
 * 
 * Return:
 * 	cpup			pointer to cpu switch entry for this system
 */
struct cpusw *
cpuswitch_entry(cpu)
	long cpu;
{
	register int i;			/* loop index */

	for (i = 0; cpusw[i].system_type != 0; i++) {
		if (cpusw[i].system_type == cpu)
			return(&cpusw[i]);
	}
	printf("\n\rProcessor type is not configured or not supported.\n\r");
	printf("If you are installing a new version of the operating system,\n\r please refer to the Installation Guide or Hardware Upgrade\n\r release notes (if available).\n\r");
	printf("If you are booting a new kernel, check the 'cpu' keyword\n\r in the kernel configuration file.  Refer to the System\n\r Administration Guide for kernel configuration file information.\n\r");
	halt_cpu();


}


/*
 * Set "cpu" (the ULTRIX system type) based on both
 * processor type and system type.
 *
 * Initialize "cpup" to point to the cpusw table entry for the system
 * that we are currently running on.
 */

set_cpuswitch()
{
	cpu = system_type(proc_type, sys_type);
	cpup = cpuswitch_entry(cpu);
}


mapspace()
{
	return((*(cpup->mapspace))());
}

halt_cpu()
{
	printf("\nHalting cpu... (transfer to monitor)\n\n");
	halt();
}

/*****************************************************************************
 *
 *	This section has machine dependent dispatch routines
 *
 *	The actual routines are entered through the cpu switch, and
 * 	are located in the appropiate cpu dependent file in sys/machine/alpha.
 *
 *****************************************************************************/

/* 
 * Call system specific initialization routine.
 */
cpu_initialize()
{
	extern char *platform_string();
	cpup->system_string = platform_string();
	if ((*(cpup->init))() < 0)
		panic("No initialization routine configured\n");
}

long
read_todclk()
{
	if (cpup->readtodr == nocpu)
		panic("No read TOD routine configured\n");
	else
		return((*(cpup->readtodr))());
}

write_todclk(yrtime)
	u_long yrtime;
{
	if (cpup->writetodr == nocpu)
		panic("No write TOD routine configured\n");
	else
		return((*(cpup->writetodr))(yrtime));
}


/*
 * If we are up and running, block interrupts around the badaddr probe.
 * Do not set mcheck_expected here; let the cpup->badaddr routine do it.
 */
badaddr(addr,len, ptr)
	caddr_t addr;
	int len;
	struct bus_ctlr_common *ptr; 
{
	int status, s;
	extern int cold;	/* booting in progress */


	if (cold)
		status =  (*(cpup->badaddr))(addr,len,ptr);
	else {
		s = splextreme();	/* Disable interrupts */
		status = (*(cpup->badaddr))(addr,len,ptr);
		splx(s);		/* Restore interrupts */
	}
	return(status);
}

/*
 * wrapper routine to badaddr(); no real use/reason
 * except lots of historical use.
 */
BADADDR(addr,len,ptr)
        caddr_t addr;
        int     len;
	struct bus_ctlr_common *ptr; 
{
        return(badaddr(addr,len,ptr));
}

/*
 * This routine sets the cache to the state passed:  enabled/disabled.
 */

setcache(state)
	int state;
{
#ifdef rjlfix
	if ((*cpup->setcache)(state) < 0 )
		panic("No setcache routine configured\n");
#endif rjlfix
}

/*
 * Flush the system caches.
 * In "critical path code", don't check return status.
 */
flush_cache()
{
	(*(cpup->flush_cache))();
}

/*
 * Common dispatch point for machine (hardware) errors
 */
#ifndef MCHECK_REGS
#define MCHECK_REGS 1
#endif /* !defined(MCHECK_REGS) */
#if MCHECK_REGS
#define MCHECK_REG_PARAM , regs
#define MCHECK_REG_DECL long *regs
#else /* MCHECK_REGS */
#define MCHECK_REG_PARAM
#define MCHECK_REG_DECL
#endif /* MCHECK_REGS */
mach_error(type, phys_logout MCHECK_REG_PARAM)
	long type;
	char *phys_logout;
	MCHECK_REG_DECL;
{
	/*
	 * Dispatch the appropriate fault handler as specified in the cpu
	 * switch table for the processor type.  The type fields come from
	 * the parameter field as specified in the SCB entry.
	 *
 	 * tim fix - These case values should perhaps be macros.
	 * Al thought that it may be a good idea to call the softerr_intr
	 * routine from the cpu switch on the correctable error types.
	 * For now send all these to the same handler.  If things get too
	 * ugly there then it may be useful to vector to different handlers.
	 */
	switch (type) {
		case S_CORR_ERR: 	/* System correctable error interrupt */
			if ((*cpup->harderr_intr)(type, phys_logout
					       MCHECK_REG_PARAM) < 0 ) {
				panic("No memory error handler configured\n");
			}
			break;
		case P_CORR_ERR:	/* Processor correctable error interrupt */
			if ((*cpup->softerr_intr)(type, phys_logout
					       MCHECK_REG_PARAM) < 0 ) {
				panic("No correctable error handler configured\n");
			}
			break;
		case S_MCHECK: 	/* System machine check abort */
		case P_MCHECK: 	/* Processor machine check abort */

			if ((*cpup->machcheck)(type, phys_logout
					       MCHECK_REG_PARAM) < 0 ) {
				panic("No machine check handler configured\n");
			}
			break;
		default:
			panic("Invalid case of machine error in mach_error\n");
	}
	mces_mcheck_clear();
}

machine_configure()
{
        if (cpup->config == nocpu)
                panic("No configuration routine configured\n");
        else
		/* get zoned memory for dma-subsystem, before 
		 * config-ing drivers. 
		 * Assumption: console drivers used before config-setup
		 * 		do not depend on dma_map_*() functions
		 */
		dma_zones_init();
                return((*(cpup->config))());
}

/*
 * Returns a unique system id 
 */
unique_sysid()

{
	(*(cpup->unique_sysid))();
}

/* 
 * Returns a device string that can be passed to the console
 * firmware to do generic console io callbacks. We will use
 * these callbacks to do the dump IO.
 */

void trans_dumpdev(dump_req)
	struct dump_request *dump_req;
{
	((*cpup->trans_dumpdev)(dump_req));

}

/*
 * Ring bell with pitch and duration.
 *
 * NOTE: this operation is often performed by a keyboard, such as the LK401;
 *  however, some keyboards, notably the PCXAL, have no built-in sound ability,
 *  so those drivers would depend on the CPU code (here) to get to the CPU's
 *  bell ring routine.
 */
int
ring_bell(pitch, duration)
int	pitch;
int	duration;
{
	if (cpup->ring_bell)
		return ((*cpup->ring_bell)(pitch, duration));
	else
		return(0);
}
u_int get_info(item_list)
struct item_list *item_list;
{
	if (cpup->get_info)
		return((*cpup->get_info)(item_list));
	else
		return(NOT_SUPPORTED);
}
