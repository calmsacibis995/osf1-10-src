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
 * Modification History: machine/alpha/ka_ruby.c
 *
 *      14-Jan-92       prm
 *      - fix up ka_ruby_readtodr to do its own seconds calculation, since
 *        toyread_convert doesn't work yet. add support for system specific 
 *        use of watch chip (i.e. am/pm mode.) Same for writetodr routine.
 *
 *	28-oct-91	prm
 *	- Add more mapping for gbus address space.
 *	- Change mchk_logout structure to ruby_mchk_logout, since this a
 *        platform specific structure and does not encompass the whole
 *        alpha architecture.
 *      - Add ruby specific BB_WATCH read and write routines.
 *
 *	4-oct-91	jac	wrote laser mcheck (stripped out ADU stuff),
 *				commented out printf in ruby_init, some
 *				clean up stuff.
 *      13-Sep-91       prm (Peter Mott)
 *         Add support for generic interrupt handling.
 *         Add support for mapping Gbus uart.
 *
 *	12-sep-91	jac	added substancailly more support...
 *
 * May-91	jac: created this file for processor support of Laser/Ruby
 *		from ka_adu.c
 *
 */

#include <sys/types.h>
#include <machine/psl.h>
#include <machine/rpb.h>
#include <machine/scb.h>
#include <machine/clock.h>
#include <machine/cpu.h>
#include <machine/pmap.h>

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
#include <sys/table.h>

#include <sys/presto.h>
#include "presto.h"

#include <machine/reg.h>
#include <machine/entrypt.h>

#include <hal/cpuconf.h>
#include <io/dec/mbox/mbox.h>

#include <hal/kn7aa.h>
#include <io/dec/lsb/lsbreg.h>

#include <io/dec/lsb/lsb_iopreg.h>

#include <vm/vm_kern.h>

#include <dec/binlog/errlog.h>
#include <dec/binlog/binlog.h>

/*
 * Defines used in user mode recovery from machine checks.
 */
#define OKTOTERM(pid)	((pid) != 1)	/* Do not kill init on machine check */

int ruby_mchk_ip = 0;           /* set to 1 when machine check in progress */
int ruby_correrr_ip = 0;        /* set to 1 when corrected error in progress*/
extern int mcheck_expected;	/* flag used to communicate between badaddr
				and Mcheck handling			*/
int mcheck_action;

/* Structure to support machine check processing on individual CPUs */
#if NCPUS > 1
struct ruby_mcheck_control Ruby_mcheck_control[NCPUS];
#else /* NCPUS <= 1 */
struct ruby_mcheck_control Ruby_mcheck_control;
#endif /* NCPUS <= 1 */

#define DBG_MSG(msg)
#define DBG_MSG_OFF(msg)

#ifdef DEBUG
#define DBG_TRACE
#endif

#ifdef DBG_TRACE
#define PARSE_TRACE(msg) DBG_MSG(msg)
#else 
#define PARSE_TRACE(msg)
#endif
#define PARSE_TRACE_OFF(msg)

/* define error action macros */
#define UNDEFINED 0
#define CRASH 1
#define RECOVER 2
#define DISMISS 3
#define ERROR_UNDEFINED mcheck_action = UNDEFINED
#define ERROR_CRASH mcheck_action = CRASH
#define ERROR_RECOVER \
  if (mcheck_action != CRASH) \
    mcheck_action = RECOVER
#define ERROR_DISMISS \
  if ((mcheck_action != CRASH) && \
      (mcheck_action != RECOVER)) \
    mcheck_action = DISMISS

/*
 * define structure to contain addresses where gbus is mapped.
 */
struct gbus_map gbus_map;	/* structure contains ptrs to gbus devices */

extern struct cpusw *cpup;	/* pointer to cpusw entry */
extern int cold;		/* for cold restart */
extern int printstate;		/* thru console callback or ULTRIX driver */

/*
 * Define mapping of interrupt lines with the type of interrupt.
 */
static int ruby_interrupt_type[INTR_MAX_LEVEL] = {
		INTR_TYPE_NOSPEC,
		INTR_TYPE_SOFTCLK,
		INTR_TYPE_SOFTCLK,
		INTR_TYPE_DEVICE,
		INTR_TYPE_DEVICE,
		INTR_TYPE_HARDCLK,
		INTR_TYPE_OTHER,
		INTR_TYPE_OTHER
};

/*
 * Prestoserve support definitions
*/
/* #define PRESTO_DEBUG */
#define CFLUSH_WORKS
					/* these are declared in lsbinit.c */
extern	int	lsb_nvram_present;	/* flag set in lsbconfl1 if nvram found */
extern	int	lsb_nvram_size;		/* adjusted size value		*/
extern	caddr_t	lsb_nvram_start_adr;	/* Adjusted start address	*/

extern void bzero();			/* used for moving data into nvram */
extern void bcopy();

/* Forward declarations. */
vm_offset_t ruby_alloc_elbuf(/* elctl,size */);
vm_offset_t lsb_addr(/* node */);

/*
 * Laser/Ruby initialization routine.
 */

ruby_init()
{
	return(0);
}

/*
 * Laser/Ruby configuration routine.
 */

struct lsbdata *head_lsbdata;

ruby_conf()
{
	struct lsbdata *lsbdata ;
	register struct bus *sysbus;
	struct rpb *rpb;
	int lsbnode;
	extern int npresto_configured ;		/* set in conf.c	*/
	extern volatile int *system_intr_cnts_type_transl;
	extern char *platform_string();

	/* Working on cold restart (for autoconf and kern_cpu)	 	*/
	cold = 1;
	rpb = (struct rpb *)hwrpb_addr;

	switch((rpb->rpb_sysvar & SV_STS_MASK) >> 10) {
		case DEC7000_SYSVAR:
			printf("DEC 7000 system\n");
			machine_slot[master_cpu].cpu_subtype = CPU_SUBTYPE_DEC_7000 ;
			break;
		case DEC10000_SYSVAR:
			printf("DEC 10000 system\n");
			machine_slot[master_cpu].cpu_subtype = CPU_SUBTYPE_DEC_10000 ;
			break;
		default:
			printf("%s\n",platform_string());
			break;
	}

	/*
	 * Setup global interrupt type array to use our definitions.
	 */
	system_intr_cnts_type_transl = ruby_interrupt_type;

	/*
	 * Allocate and setup the lsb definition structure
	*/
	head_lsbdata = (struct lsbdata *) kalloc(sizeof(struct lsbdata));
	lsbdata = head_lsbdata;
	lsbdata->next = 0;		/* in Ruby, there is only 1 lsb	*/
	lsbdata->lsbnum = 0;		/* and its number is 0		*/
	lsbdata->lsbphys = (struct lsb_reg *) LSB_START_PHYS ;

	/*
	 * Assign the virtual space for all lsb nodes
	*/
        for (lsbnode = 0; lsbnode < MAX_LSB_NODE; lsbnode++) {
		lsbdata->lsbvirt[lsbnode] = (struct lsb_reg *)
			PHYS_TO_KSEG((*cpup->nexaddr)(lsbnode));
	}

	/*
	 * Setup lsbdata fields for the CPU
	*/
	
	lsbdata->cpu_lsb_addr = lsbdata->lsbvirt[mfpr_whami()];
	lsbdata->lsbintr_dst = 1 << mfpr_whami();	/* which cpu gets intrpts*/

	master_cpu = 0; /* ??? Should master_cpu be set to id # of CPU	*/
#if	NCPUS > 1
	printf("Master cpu at slot %d.\n", mfpr_whami());
#endif	/* NCPUS > 1 */
	machine_slot[master_cpu].is_cpu = TRUE;
	machine_slot[master_cpu].cpu_type = CPU_TYPE_ALPHA;
	machine_slot[master_cpu].running = TRUE;
	machine_slot[master_cpu].clock_freq = hz;

	/* Map mcheck logout area and initialize data structures */
	ruby_mcheck_init();

        enable_spls();  /* Ready for spl to be lowered */
    	splextreme();

	/*
	 * Configure the laser bus
	*/
        system_bus = sysbus = get_sys_bus("lsb");
	if (sysbus == 0)
		panic("kn7aa: No system bus configured");
        (*sysbus->confl1)(-1, 0, sysbus);

	/*
	 * Set up prestoserve if configured
	*/
	if ( npresto_configured > 0 )
		ruby_presto_config();
	/*
	 * We now have the scb set up enough so we can handle interrupts if any
	 * are pending.
	 */
	(void) spl0();

	/* cold = 0; Now done in machdep.c - is this permanent? */
	return(0);
}
/*
 * Below are the support routines to move data into and out of the NVram.
 * when the CFLUSH pal call is implemented, the additional eviction forceing
 * bcopy should be replaced by the CFLUSH pal call
*/

#ifdef PRESTO_DEBUG
int prdebug_write = 1;
#endif /* PRESTO_DEBUG */

void ruby_presto_write(source, destin, size)
	caddr_t	source, destin ;
	int	size;
{
	int numpfn, loop ;
	caddr_t	evic ;
	long physdestin, physend ;
	volatile long evic_data ;

#ifdef PRESTO_DEBUG
if (prdebug_write) printf("ruby_presto_write: First time through, source = %l016x, destin = %l016x, size = %x\n", source, destin, size);
#endif /* PRESTO_DEBUG */

	bcopy(source, destin, size);	/* move the data into the nvram */

#ifdef CFLUSH_WORKS
/*
 * When the CFLUSH pal call is properly implemented, then this code
 * should be used as it is move efficient then the 'by hand eviction' method
 *
 * NOTE: as of Mar 93, the ruby pal just does a read of the appropriate
 * locations in the low 4 MB of memory for force an eviction. If any of
 * that memory has uncorrectable errors, the machine will halt to console...
*/
	svatophys(destin, &physdestin);
	for ( physend = physdestin + size ;
		physend > physdestin ; physdestin = physdestin + PAGE_SIZE ) {

#ifdef PRESTO_DEBUG
if (prdebug_write) printf("ruby_presto_write: doing a CFLUSH to physdestin = %l016x, (PAGE_SIZE-1) = %x\n", physdestin,(PAGE_SIZE-1));
#endif /* PRESTO_DEBUG */

	cflush((physdestin >> 13L));	/* flush the physical PFN */

#ifdef PRESTO_DEBUG
if (prdebug_write) {
	printf("ruby_presto_write: CFLUSH worked!\n");
	prdebug_write = 0;
	}
#endif /* PRESTO_DEBUG */
	}

	/*
	 * Need one more flush if this transfer crosses a page boundry
	*/
	if ( (physdestin & (PAGE_SIZE-1)) < (physend & (PAGE_SIZE-1)))
		cflush((physdestin >> 13L));

#else /* CFLUSH_WORKS */
#ifdef PRESTO_DEBUG
if (prdebug_write) printf("ruby_presto_write: The eviction adr = %l016x\n", evic);
#endif /* PRESTO_DEBUG */
	if (((long)destin + (long)RUBY_CACHE_SIZE + (long)size) < 
			((long)lsb_nvram_start_adr + MS700_NVRAM_SIZE)) {
		/*
		 * We can just evic forward, there's enough room.
		*/
		evic = destin + RUBY_CACHE_SIZE ;
	}
	else{
		/*
		 * Can't evic forward, there isn't enough room.
		 * evic starting before the destination.
		*/
		evic = destin - RUBY_CACHE_SIZE ;

	}
	for ( ; size > 0 ; size = size - sizeof(int), evic=evic+sizeof(int)) {
	evic_data = (long)*evic ;
	}
	mb();

#ifdef PRESTO_DEBUG
if (prdebug_write)  {
	printf("ruby_presto_write: The first eviction worked!\n");
	prdebug_write = 0 ;
}
#endif /* PRESTO_DEBUG */
#endif /* CFLUSH_WORKS */
}

#ifdef PRESTO_DEBUG
int prdebug_zero = 1;
#endif /* PRESTO_DEBUG */

void ruby_presto_zero(adr,size)
	caddr_t	adr ;
	int	size;
{
	caddr_t evic ;
	volatile long evic_data ;
	long physdestin, physend ;

#ifdef PRESTO_DEBUG
if (prdebug_zero) printf("ruby_presto_zero: First time through, adr = %l016x, size = %x\n", adr, size);
#endif /* PRESTO_DEBUG */

	bzero(adr,size);

#ifdef CFLUSH_WORKS
	svatophys(adr, &physdestin) ;
	for ( physend = physdestin + size ;
		physend > physdestin ; physdestin = physdestin + PAGE_SIZE ) {

#ifdef PRESTO_DEBUG
if (prdebug_zero) printf("ruby_presto_zero: doing a CFLUSH, physdestin = %l016x\n", physdestin);
#endif /* PRESTO_DEBUG */

	cflush((physdestin >> 13L));	/* flush the physical PFN */

#ifdef PRESTO_DEBUG
if (prdebug_zero) {
	printf("ruby_presto_zero: the CFLUSH worked!\n");
	prdebug_zero = 0;
	}
#endif /* PRESTO_DEBUG */
	}

	/*
	 * Need one more flush if this transfer crosses a page boundry
	*/
	if ( (physdestin & (PAGE_SIZE-1)) < (physend & (PAGE_SIZE-1)))
		cflush((physdestin >> 13L));

#else /* CFLUSH_WORKS */
#ifdef PRESTO_DEBUG
if (prdebug_zero) printf("ruby_presto_zero: The eviction adr = %l016x\n", adr);
#endif /* PRESTO_DEBUG */

	if(( (long)adr + (long)RUBY_CACHE_SIZE + (long)size) <
			((long)lsb_nvram_start_adr + MS700_NVRAM_SIZE)) {
		/* We can just evic forward, there's enough room. */
		evic = adr + RUBY_CACHE_SIZE ;
	}
	else {
		/* Can't evic forward, there isn't enough room. */
		evic = adr - RUBY_CACHE_SIZE ;
	}
	for ( ; size > 0 ; size = size - sizeof(int), evic=evic+sizeof(int)) {
	evic_data = (long)*evic ;
	}
	mb();

#ifdef PRESTO_DEBUG
if (prdebug_zero)  {
	printf("ruby_presto_zero: The eviction worked!\n");
	prdebug_zero = 0 ;
	}
#endif /* PRESTO_DEBUG */
#endif /* CFLUSH_WORKS */
}

/*
 * This routine setups up the presto interface and calls presto_init()
*/
ruby_presto_config()
{
	/* these routines are in lsbinit.c	*/
	extern lsb_nvram_status();
	extern lsb_nvram_battery_status();
	extern lsb_nvram_battery_disable();
	extern lsb_nvram_battery_enable();
/*
 * Setup the lsb nvram status routines for the presto driver
*/
	presto_interface0.nvram_status = lsb_nvram_status;
	presto_interface0.nvram_battery_status= lsb_nvram_battery_status;
	presto_interface0.nvram_battery_disable= lsb_nvram_battery_disable;
	presto_interface0.nvram_battery_enable= lsb_nvram_battery_enable;
/*
 * The following interfaces were added to permit hardware dependent
 *  tailoring of presto transfers.
*/
	presto_interface0.nvram_ioreg_read = bcopy ;
	presto_interface0.nvram_ioreg_write = ruby_presto_write ;
	presto_interface0.nvram_block_read = bcopy ;
	presto_interface0.nvram_block_write = ruby_presto_write ;
	presto_interface0.nvram_ioreg_zero = ruby_presto_zero ;
	presto_interface0.nvram_block_zero = ruby_presto_zero ;

	presto_interface0.nvram_min_ioreg = sizeof(int) ;
	presto_interface0.nvram_ioreg_align = sizeof(int) ;
	presto_interface0.nvram_min_block = sizeof(int) ;
	presto_interface0.nvram_block_align = sizeof(int) ;

	/*
	 * Do the presence check after setting the presto_interface routines
	 * to avoid a panic when a presto board is not present (bat status
	 * will be bad and presto not used)
	*/ 
	if (lsb_nvram_present == 0 ) {	/* presto configured, but no NVRAM */
		return(0);
	}
/*
 * Call the presto initialization routine.
 * 
 * The first 1K of reserved NVRAM is for diags, console, etc.
 * The 3rd parameter means mapped, the 4th means cached.
 * The last parameter is an unsigned 32 bit unique 'system identifier'
 * 	used by presto during a power up to determine if the NVRAM has been
 *	moved to a different system.
*/
	presto_init((lsb_nvram_start_adr+1024L),
		(lsb_nvram_size-1024), 1, 1, ruby_ssn());

}

/*
 * Determine an unsigned 32 bit unique number from the Ruby system
 * serial number in the hwrbp. ??? This routine still needs work ???
*/
int
ruby_ssn()
{
	struct rpb *rpb;
	u_int ssn = 0;
	int i;
	char *cp;
	rpb = (struct rpb *)hwrpb_addr;
	cp = rpb->rpb_ssn ;

	cp++ ; cp++ ;		/* skip the first two letters		*/
	for (i = 0 ; i < 8 ; i ++, cp++) {
	ssn += (*cp - 48) << i ; ;
	}
#ifdef PRESTO_DEBUG
	printf("ruby_ssn: rpb->rpb_ssn = %s , id = %x\n", rpb->rpb_ssn, ssn) ;
#endif /* PRESTO_DEBUG */
	return(ssn) ;
}

/*
 *   Map the Gbus I/O space for the console routines
 *   This is called thru the CPU switch from startup.
 * Save the virtual address of Gbus uart for later use.
 *
 * gbus_uart0_base must be set up prior to calling the console init routine.
 */

u_long *gbus_uart0_pg_base;         /* base addr of gbus uart regs */

ruby_map_io()
{
	gbus_uart0_pg_base = (u_long *)PHYS_TO_KSEG(GBUS_LOC_BASE);
	gbus_map.uart0 = (struct gbus_uart *)PHYS_TO_KSEG(GBUS_DUART0);
	gbus_map.uart1 = (struct gbus_uart *)PHYS_TO_KSEG(GBUS_DUART1);
	gbus_map.uart2 = (struct gbus_uart *)PHYS_TO_KSEG(GBUS_DUART2);
	gbus_map.watch = (struct gbus_watch *)PHYS_TO_KSEG(GBUS_WATCH);
	gbus_map.misc = (struct gbus_misc *)PHYS_TO_KSEG(GBUS_MISC);
	return(0);
}

/*
 * Ddefinitions needed to get_info routine 
 */
#define	FBUS_7000_INTR_REQ_REG	(0xfffCA800)

/*
 *
 *   Name: kn430_getinfo(item_list) - Gets system specific information for the requestor
 *
 *   Inputs:	item_list - List of items the caller wants information about.
 *
 *   Outputs:	returned information or NOT_SUPPORTED for each item requested
 *
 *   Return	
 *   Values:	NA.
 */
u_int kn7aa_getinfo(request)
	struct item_list *request;

{
	do {
		request->rtn_status = INFO_RETURNED;
		switch(request->function) {
			case FBUS_INTREQ_REG:
   				request->output_data = (long)FBUS_7000_INTR_REQ_REG;				
				break;
			default: 
				request->rtn_status = NOT_SUPPORTED;
		}
		request = request->next_function;		
	} while (request != NULL);
	return(TRUE);
}

/*
 *
 *   Name: kn7aa_dump_dev() - translate dumpdev into a string the console can understand
 *
 * Translates the SCSI device and target data into a device string which can be passed to
 * prom_open. Which a generic IO console callback. See the SRM for details
 *   Abstract:	This routine returns info specific to the MSCP device
 *		that is needed to perform a dump.
 *
 *   Inputs:	Dump_req - generic dump info 
 *
 *   Outputs:	dump_req->dev_name - device string for the console
 *
 *   Return	
 *   Values:	NA.
 */
kn7aa_dump_dev(dump_req)
	struct dump_request *dump_req;

{
	char *device_string;
	char temp_str[8];

	device_string = dump_req->device_name;

	if (strcmp("MSCP", dump_req->protocol) == 0) {
		strcpy(device_string,"MSCP ");

                /* hose */
                itoa(((mbox_t)(dump_req->device->ctlr_hd->bus_hd->bus_mbox))->hose,temp_str);
                strcpy(&device_string[strlen(device_string)],temp_str);
                strcpy(&device_string[strlen(device_string)]," ");

                /* node */
                if ( dump_req->device->ctlr_hd->bus_hd->bus_type == BUS_XMI )
        	        itoa(dump_req->device->ctlr_hd->slot,temp_str);
                else /* BUS_CI */
			itoa(dump_req->device->ctlr_hd->bus_hd->slot,temp_str);

                strcpy(&device_string[strlen(device_string)],temp_str);
                strcpy(&device_string[strlen(device_string)]," ");

		/* channel */
		strcpy(&device_string[strlen(device_string)],"0 ");

		/* remote bus address */
		if ( dump_req->device->ctlr_hd->bus_hd->bus_type == BUS_XMI )		
			strcpy(&device_string[strlen(device_string)],"0 ");
		else {
                     	itoa(dump_req->device->ctlr_hd->ctlr_num,temp_str);
                     	strcpy(&device_string[strlen(device_string)],temp_str);
                     	strcpy(&device_string[strlen(device_string)]," ");
		}

                /* unit */
                itoa(dump_req->device->unit,temp_str);
                strcpy(&device_string[strlen(device_string)],temp_str);

                /* ??? */
		if (  dump_req->device->ctlr_hd->bus_hd->bus_type == BUS_XMI )
                	strcpy(&device_string[strlen(device_string)]," 102a 0c22");
		else
                     	strcpy(&device_string[strlen(device_string)]," 102a 0c2f");	

	} else if (strcmp("SCSI", dump_req->protocol) == 0) {

		/* protocol */
		strcpy(device_string,"SCSI ");

		/* hose */
		itoa(((mbox_t)(dump_req->device->ctlr_hd->bus_hd->bus_mbox))->hose, temp_str);
		strcpy(&device_string[strlen(device_string)],temp_str);
		strcpy(&device_string[strlen(device_string)]," ");

		/* slot */
		itoa(dump_req->device->ctlr_hd->bus_hd->slot,temp_str);
		strcpy(&device_string[strlen(device_string)],temp_str);
		strcpy(&device_string[strlen(device_string)]," ");

		/* channel */
		itoa(dump_req->device->ctlr_hd->slot,temp_str);
		strcpy(&device_string[strlen(device_string)],temp_str);
		strcpy(&device_string[strlen(device_string)]," ");

		/* remote_address ( target ) */
		itoa(dump_req->unit,temp_str);
		strcpy(&device_string[strlen(device_string)],temp_str);
		strcpy(&device_string[strlen(device_string)]," ");

		/* unit = target * 100 */
		itoa(dump_req->unit*100, temp_str);
		strcpy(&device_string[strlen(device_string)],temp_str);

		strcpy(&device_string[strlen(device_string)]," 102a 0c36");

	}
}


/*
* This routine is called via lsbinit.c when another LEP is found on
* the LSB.
*/
lsb_mplepint()
{
printf("lsb_mplepinit: Processor found in slot %d. MP not currently supported.\n", mfpr_whami());
return(0);
}


/*****************************************************************************
 * KN7AA ERROR HANDLERS 						     *
 *   Machine Check: 670, 660; 						     *
 *   Processor Correctible: 630						     *
 *									     *
 * The error handlers are called through the cpu switch, from	     	     *
 * mach_error.  The basic sequence for handling of errors is as		     *
 * follows:								     *
 *									     *
 * kn7aa_<handler>: machcheck; syscorr; proccorr.			     *
 * 		Copies the error frame and calls kn7aa_parse_xxx.	     *
 * 		Based on severity, brings down machine, kills a process,     *
 * 		or continues execution.					     *
 *    kn7aa_parse_xxx						     	     *
 *		Starts building an error log frame.			     *
 * 		Based on the vector parameter, calls one of the subparse     *
 * 		routines (kn7aa_parse_*).				     *
 *		Completes the error log frame and requests that it be sent.  *
 *****************************************************************************/

/*
 * The severity of a machine check determines what we can do:
 *
 *   System Fatal    - The kernel state is corrupt.  Crash (if you can...).
 *   Processor Fatal - This processor is toast.  For now, crash...
 *   Process Fatal   - A process is corrupt.  Blow it away.  Currently no
 * 		       way to tell the difference between process and system
 *		       fatal -- have to be able to discern user addresses.
 *   Survivable      - Either the error was corrected, or we didn't expect
 *		       to keep running under this error.  Since we're still
 *		       running, it must have been a transient.
 *   Probe	     - The machine check was the result of a bad address probe.
 */
#define MCHECK_SEV_PROBE 0L
#define MCHECK_SEV_SURVIVABLE 1L
#define MCHECK_SEV_SYSTEM_FATAL 2L
#define MCHECK_SEV_PROCESS_FATAL 3L
#define MCHECK_SEV_PROCESSOR_FATAL 4L

/*
 * Reasons for entering this handler:
 *
 * 'LEP hard errors' 
 *	. Mcheck through 0x670 if associated with a synchronous EV4 request.
 *	. interrupt through 0x660 if the error is asynchronous to the EV4.
 * 'LSB Hard errors'
 *	. Interrupt through 0x660 (bits in the LBER are set).
 * 'Mchecks'
 *	. all synchronous I-stream hard errors
 *
 * The format of the error (cmcf) frames are found in kn7aa.h as
 * structure kn7aa_mchk_logout and kn7aa_proccorr_logout
 *
 */

kn7aa_syscorr (type, cmcf, regs)
	long type;		/* This is parameter used to */
				/* distinguish between machine check */
				/* entry types (i.e. 670, 660, 630) */
	caddr_t cmcf;		/* This is a pointer to the machine */
				/* check frame created by PAL, */
				/**** PHYSICAL at time of writing */ 
	register long *regs;	/* pointer to registers saved on stack */
				/* by _XentInt */ 
{

	panic("kn7aa: Unexpected system correctible error interrupt");

}

kn7aa_proccorr (type, cmcf, regs)
	long type;		/* This is parameter used to */
				/* distinguish between machine check */
				/* entry types (i.e. 670, 660, 630) */
	caddr_t cmcf;		/* This is a pointer to the machine */
				/* check frame created by PAL, */
				/**** PHYSICAL at time of writing */ 
	register long *regs;	/* pointer to registers saved on stack */
				/* by _XentInt */ 
{
	struct lsb_lep_reg *nxv;
	struct lsbdata *lsbdata;
	struct kn7aa_proccorr_logout *err_logout;
       	register struct ruby_mcheck_control *mctl;
	register struct ruby_mcheck_control_elbuf *elctl;
	int parse_results;
	int	i;
	struct elr_soft_flags sw_flags;

	/* initialize sw_flags */
	sw_flags.packet_revision = 0;
	sw_flags.rsvd1 = 0;
	sw_flags.error_flags[0] = 0;
	sw_flags.error_flags[1] = 0;
	
	/* set logout pointer to kseg address of cmcf */
        err_logout = (struct kn7aa_proccorr_logout *)cmcf;

	DBG_MSG(("630 cmcf: 0x%l016x\n", err_logout));
	DBG_MSG(("(long *)((char *)cmcf -0x10) 0x%l016x\n", 
		 (long *)((char *)cmcf -0x10)));
	DBG_MSG(("*(long *)((char *)cmcf -0x10) 0x%l016x\n", 
		 *(long *)((char *)cmcf -0x10)));


	/* SCB type 630 is a corrected, single bit read error.
	 * It comes in at IPL 20 and only requires logging,
	 * no corrective action is needed.
	 * 9-oct-91 CVI says that ruby pal does not implement 0630
	 * now and may never...
	 */
	ruby_correrr_ip++;

	/*
	 * The ADU code did this check for the recursive
	 * case. I would think this has the potential
	 * to be an infinite loop...
	 */
	if (ruby_correrr_ip > 1) {
		ruby_correrr_ip--;
		return(0);
	}
	
	ruby_parse_630(err_logout, &sw_flags);
	if (mcheck_action != DISMISS) kn7aa_log_error(err_logout, &sw_flags, type);

	/* Have to clear out error bits                         */
	lsbdata = get_lsb(0);
	nxv = (struct lsb_lep_reg *)lsbdata->lsbvirt[mfpr_whami()];
	nxv->lsb_lep_lmerr = nxv->lsb_lep_lmerr;
	nxv->lsb_lep_lber = nxv->lsb_lep_lber;	/* Clear LSB stuff	*/
	
	DBG_MSG(("(long *)((char *)cmcf -0x10) 0x%l016x\n", 
		 (long *)((char *)cmcf -0x10)));
	DBG_MSG(("*(long *)((char *)cmcf -0x10) 0x%l016x\n", 
		 *(long *)((char *)cmcf -0x10)));
	mces_pce_handle();
	mces_pce_clear();
	DBG_MSG(("(long *)((char *)cmcf -0x10) 0x%l016x\n", 
		 (long *)((char *)cmcf -0x10)));
	DBG_MSG(("*(long *)((char *)cmcf -0x10) 0x%l016x\n", 
		 *(long *)((char *)cmcf -0x10)));

	/*
	 * added to address QAR 12546
	 * decrement the inprogress counter to allow complete processing of
	 * future processor correctable errors.
	 */
	ruby_correrr_ip--;
}


/** MACHINE CHECK HANDING CODE int 670/660 **/

/*
 * Machine check handler.
 * Called from locore thru the cpu switch in response to a trap at SCB 
 * locations 0x630, 0x660, and 0x670.
 *
 * Reasons for entering this handler:
 *
 * 'LEP hard errors' 
 *	. Mcheck through 0x670 if associated with a synchronous EV4 request
 *	. interrupt through 0x660 if the error is asynchronous to the EV4
 * 'LSB Hard errors'
 *	. Interrupt through 0x660 (bits in the LBER are set)
 * Mchecks
 *	. all synchronous I-stream hard errors
 *
 * In the LEP spec Rev 0.9, the Mcheck frame looks like:
 *
 * 63                 0
 *
 * R    mbz      Number of bytes
 *
 * PAL_temp0
 *  ...
 * PAL_temp31
 * exc_addr
 * exc_sum
 * msk
 * PAL_base
 * HIRR
 * HIER
 * MM_CSR
 * VA
 * BIU_addr
 * BIU_stat
 * DC_addr
 * Fill_addr
 * DC_stat
 * Fill_syndrome
 * BC_tag
 *
 */

ruby_machcheck (type, cmcf, regs)
	long type;		/* This is parameter used to */
				/* distinguish between machine check */
				/* entry types (i.e. 670, 660, 630) */
	caddr_t cmcf;		/* This is a pointer to the machine */
				/* check frame created by PAL, */
				/**** PHYSICAL at time of writing */ 
	register long *regs;	/* pointer to registers saved on stack */
				/* by _XentInt */ 
{
	u_long addr_ptr;
	struct lsb_lep_reg *nxv;
	struct lsbdata *lsbdata;
	register int retry;
	register int clear_check_ip;
	extern int MCheck_ok;
	extern int cold;
	struct ruby_mchk_logout *mchk_logout;
	register struct ruby_mcheck_control *mctl;
	register struct ruby_mcheck_control_elbuf *elctl;
	int parse_results;
	int	i;
	struct elr_soft_flags *sw_flags;

	/* get pointers to machine check control data structures */
	mctl = KN7AA_MCHECK_CONTROL(mfpr_whami());

	/* set logout pointer to kseg address of cmcf. It is finally correct. */
        mchk_logout = (struct ruby_mchk_logout *) cmcf;

	/* get pointer to mcheck error logging control structure */
	elctl = &mctl->mctl_elctl;

#ifdef MCHECK_BUFFER_NOT_IMPLEMENTED
        long    *paddr = (long *)cmcf;
        long    *vaddr = (long *)(&virt_copy);

        /* Copy the logout frame from physical memory into structure */
        for (i = 0; i < sizeof(struct ruby_mchk_logout)/sizeof(long); i++)
                *vaddr++ = ldqp(paddr++);
        mchk_logout = &virt_copy;
#endif /* MCHECK_BUFFER */
	
	/* first see if a Mcheck is expected from badaddr() at this point */
	if (mcheck_expected) {
		mcheck_expected = 0 ;	/* clear the OK flag and indicate an Mcheck occured*/

		/*
		 * Clearing the in progress flag involves writing a 1 
		 * to the mces register.
		 */

 		mces_mcheck_clear();

		/* Have to clear out error bits                         */
		lsbdata = get_lsb(0);
		nxv = (struct lsb_lep_reg *)lsbdata->lsbvirt[mfpr_whami()];
		nxv->lsb_lep_lmerr = nxv->lsb_lep_lmerr;
		nxv->lsb_lep_lber = nxv->lsb_lep_lber;	/* Clear LSB stuff	*/

		/* again, probably superfluous, but make sure the flag is coherent*/
		mb();

		return(0);
	}

	/*
	 * We shouldn't get an unexpected Mcheck, even when cold. However with
	 * P3 modules and the current vm initialization habit of dropping IPL
	 * before we're ready for interrupts, we do see them. This is a quick
	 * fix to get bl10 working.
	*/
	if (cold) {
		printf("kn7aa: Informational only - Mcheck of type = %x during boot\n\r",type);
		return(0);
	}

	/* things look bad for the good guys, we weren't planning on a mcheck*/

	mcheck_action = UNDEFINED;
	clear_check_ip = 0;
	retry = 0;


	/* Test the machine check frame's RETRY_BIT. */

	/*
	 * Work around retry bit bugs...
	 *
	 * - PAL presently uses bit 59 for the retry bit.
	 *
	 * - kn7aa.h is using bit 60.
	 *
	 * - The SRM defines bit 63 as the retry bit.
	 *
	 * To work with the present PAL and prepare for future PAL
	 * releases, which use the correct retry bit, RETRY_BIT is
	 * or'd with bit 63 and 59 to test for retry. 
	 * (PAL only uses one of these bits, so the test forany of the 
	 * three will work until PAL and DEC7000 code syncs up, then 
	 * this work around should be removed.
	 */

	if (mchk_logout != ((struct ruby_mchk_logout *) -1)) {
		retry = (mchk_logout->retry &
			 (RETRY_BIT | 0x8800000000000000L));
	}

	/*
	 * allocate space for software flags. These flags are used
	 * after parsing to determine what data to collect and log.
	 *
	 * cleaned up by ruby_reset_elctl
	 */
	sw_flags =(struct elr_soft_flags *)
		ruby_alloc_elbuf(elctl,(sizeof(struct elr_soft_flags)));
	if (sw_flags == 0)
	{
		panic("kn7aa: Unable to allocate elctl buffer");
	}
	
	sw_flags->packet_revision = 1;
	sw_flags->rsvd1 = 0;
	sw_flags->error_flags[0] = 0;
	sw_flags->error_flags[1] = 0;
	
	switch (type)
	{ /* of machine check */

		case S_MCHECK: 	/* System machine check abort */
		case P_MCHECK: 	/* Processor machine check abort */
			/*
			 * Case 670 are 'EV4' hard errors including double bit
			 * ECC errors on B-cache/memory, or parity errors
			 * on cache tags/internal buses. Wicked bad.
			 *
			 * Case 660 are 'LEP' hard errors including B-cache
			 * errors from the LSB side, Gbus errors, and LEVI
			 * timeouts. These too are wicked bad.
			 *
			 * If a second machine check is detected while a 
			 * machine check is in progress, a Double Error abort
			 * is generated and the processor enters the restart
			 * sequence.  For this reason the following test should
			 * never be true.  
			 */
			ruby_mchk_ip++;
			if (ruby_mchk_ip > 1) {
				panic("Double Error machine check abort.\n");
			}
			clear_check_ip = 1;

			if (type == P_MCHECK)
			{
			  parse_results = ruby_parse_670_mcheck(mchk_logout, sw_flags);
			  if (mcheck_action != DISMISS) kn7aa_log_error(mchk_logout, sw_flags, type);
			}

			if (type == S_MCHECK)
			{
			  parse_results = ruby_parse_660_mcheck(mchk_logout, sw_flags);
			  if (mcheck_action != DISMISS) kn7aa_log_error(mchk_logout, sw_flags, type);
			}
       		break;
		default:
			/*
			 * Unrecognized machine check type.  Assume that these
			 * are not recoverable.  Log what you can before the
			 * system goes down.
			 */
			/* ***PRM: What about logging this one? */
			panic("kn7aa: Unrecognized machine check type %d.\n",type);
			kn7aa_log_error(mchk_logout, sw_flags, type);
			break;

	} /* of machine check */

	/* reset error log control */
	ruby_reset_elctl();
	
	/*
	 * For the case of machine checks the PALcode will set an internal
	 * machine check in progress flag.  This flag must be cleared upon
	 * exit of this handler.  Failure to clear this flag will cause a 
	 * Double Error abort in the event of the next machine check.
	 *
	 * Clearing the in progress flag involves writing a 1 to the mces
	 * register.
	 */

	/*
	 * Clearing the in progress flag involves writing a 1 
	 * to the mces register.
	 */
	
	mces_mcheck_clear();
	
        /*
	 * Have to clear out error bits. This will nuke all errors, is
	 * it necessary to clear bits in any order? Not clear some bits?
	 * Need to clear the  LMERR here too???
	 */
	lsbdata = get_lsb(0);
	nxv = (struct lsb_lep_reg *)lsbdata->lsbvirt[mfpr_whami()];
	nxv->lsb_lep_lmerr = nxv->lsb_lep_lmerr;
	nxv->lsb_lep_lber = nxv->lsb_lep_lber;
	mb();

	if (mcheck_action == CRASH) {
		/* Error parsing says to crash... */
		panic ("Non-recoverable hardware error");
	} 
	else if ((mcheck_action != CRASH) && (retry != 0))
        {
		/* Error parsing doesn't say crash. PAL says retry. */
		/* Let's do it! */
		ruby_mchk_ip--;
		return(0);
	}
	else if (mcheck_action == RECOVER)
	{
		/* Error parsing says retry. */
		ruby_mchk_ip--;
		return(0);
	}
	else
	{
		/* default action. Nothing says recover... */
		panic ("Non-recoverable hardware error");
	}

	ruby_mchk_ip--;
	return(0);
}

/*
 * This is the 'bad address' probe routine for Ruby. It is passed an
 * address and len and will return 0 if the address existes and
 * a 1 if the address is none existant. This routine only works with locally
 * mapped addresses (not mailbox space...) and legal lenghts are:
 *
 * word (len = 4, from BSD days)
 * quad word (len = 8, the bit next to 4).
 *
 * It is assumed that interrupts are not enabled when this routine is entered
 * and (from the LEP spec) that no interrupts are generatored by virtual of
 * a non-existant location reference.
 *
 * A global variable is used to announce that an Machine check might occur.
 * mcheck_expected is set to a 1 if an occurance of an mcheck is OK. The mcheck
 * handler clears mcheck_expected to 0 if an Mcheck does occur. This is the
 * communication mechanism that indicates an Mcheck occured. This also will
 * preclude recursive Mchecks from happening.
*/

ruby_badaddr(addr,len,ptr)
	vm_offset_t addr;
	int len ;
	struct bus_ctlr_common *ptr; /* bus/ctlr this csr access is for */
{
	int word_length, bum_address ;
	long long_length;
	u_int stat = 0;
	struct mbox *mbp = (mbox_t)0;    /* bus/ctlr this csr access is for */

	if (mcheck_expected)
		panic("mcheck_expected set in probe when unexpected"); 

	/*
	 * probably superfluous, but make sure there aren't any unexpected
	 * mchecks about to happen...
	*/
	draina();
	mcheck_expected = 1;			/* enter the mcheck_ok region.	*/
	/* again, probably superfluous, but make sure the flag is coherent*/
	mb();

	if(mbp = (mbox_t)ptr->mbox)
		stat = mbox_badaddr(addr, len, mbp);
	else {
		if (len == 8) {		/* Requesting a quad probe	*/
			addr &= ~(7UL);	/* round address to quadword boundary */
			long_length = *(long *)addr;
		}
		else { /*treat other lens as smallest granularity */
			addr &= ~(3UL);	/* round address to longword boundary */
			word_length = *(int *)addr;
		} 
	}
	mb();
	mb();				/* make sure the flag is coherent*/
	bum_address = ((mbp == 0) ? (!mcheck_expected) : stat);
	mcheck_expected = 0;		/* leave the mcheck_ok region.	*/

	return(bum_address);

}

/*
 * This routine will return physical address of the given lsb node number.
*/
long ruby_nexaddr(lsbnode)
     int lsbnode ;
{
  /* Each lsb node space is 4mb in lenght */
return(LSB_START_PHYS + ((long)lsbnode * 0x400000)) ;
}



/*
 * ka_ruby_readtodr: 
 *      return: (long) number of seconds since the epoch
 * 	  	      (Which was 01-Jan-1970). 
 *
 * This code was modeled after the mc146818clock.c code under
 * ../mips/hal, which uses the same chip model.
 *
 * On this watch chip setting the SET bit (WATCH_CSRB_M_SET) freezes the contents of
 * the time registers read by software, however the hardware maintains a copy
 * which continue to be updated so that when the SET bit is cleared the updates
 * are propagated to the read registers and no time is lost.
 */
static  int     dmsize[13] =
{ 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

long
        ka_ruby_readtodr()
{
        u_long watch_ptr;
        struct gbus_watch *watch;
        u_long secs;
        struct tm tm;
        int s, i, uip_loop, error_exit = 0;
        
        watch = gbus_map.watch; /* get address where watch chip is mapped. */
        
        /*
         * check if the chip has noticed that
         * battery backup has been lost
	 * Change ~watch->csrd_vrt to !watch->csrd_vrt because of a 
	 * union/bit field compiler bug
         */
        if (!(watch->csrd_vrt))
        { 
                printf("WARNING: BB_WATCH invalid time. lost battery backup on clock. csrd.vrt is clear.\n");
                error_exit++;
        }
        
	for (uip_loop=0; ((watch->csra.reg & WATCH_CSRA_M_UIP) &
			(uip_loop < 30)) ; uip_loop++) {
	/* an Update is going to happen within 244 uS, hang out for a while */
	DELAY(10)
	}
	if (uip_loop == 30)
		printf("WARNING: Clock update operation took an unusual amount of time, check date/time\n");
        
        /* if the clock speed is not as expected report it and adjust it */
        if ( (watch->csra.reg & ~WATCH_CSRA_M_UIP) != WATCH_CSRA_SET) { 
                printf("WARNING: BB_WATCH frequencies not as expected. Set to expected value\n");
                printf("                  found csra: 0x%02x set csra to: 0x%02x\n", watch->csra.reg, WATCH_CSRA_SET);
                watch->csra.reg = WATCH_CSRA_SET;
        }
        
        /* if the data mode is BCD return invalid time */
        if ( (watch->csrb.reg & WATCH_CSRB_M_DM) == 0) { 
                printf ("WARNING: BB_WATCH in BCD mode. Invalid time. Setting to Binary mode.\n");
                watch->csrb.reg |= WATCH_CSRB_M_DM;
                error_exit++;
        }
        
        /* if the hour mode is am report it and adjust it */
        if ( (watch->csrb.reg & WATCH_CSRB_M_HM) == 0) { 
                printf ("WARNING: BB_WATCH in 12 hour mode. set to 24 hour mode. Time adjusted.\n");
		watch->csrb.reg |= WATCH_CSRB_M_SET;
                watch->csrb.reg |= WATCH_CSRB_M_HM;
                watch->hours = ((((watch->hours & 0x80)>>7)*12)+(watch->hours & ~0x80));
                watch->csrb.reg &= ~WATCH_CSRB_M_SET;
        }
        
        /* if the DSE mode is set return invalid time */
        if (watch->csrb.reg & WATCH_CSRB_M_DSE)
        { 
                printf("WARNING: BB_WATCH in Daylight savings mode.\n");
                printf("         Setting Standard time mode. Time must be set manually\n");
                watch->csrb.reg &= ~WATCH_CSRB_M_DSE;
                error_exit++;
        }
        
	if (ka_ruby_check_todr_setup())
		{
			error_exit++;
		}

	/* if any errors occurred return NULL (error) status */
        if (error_exit)
                return(0);
        
        s = splhigh();          /* block interrupts */
	watch->csrb.reg |= WATCH_CSRB_M_SET;

        /* Load tm from Ruby's BB_WATCH */
        tm.tm_sec = watch->seconds;
        tm.tm_min = watch->minutes;
        tm.tm_hour = watch->hours;
        tm.tm_mday = watch->day;
        tm.tm_mon = watch->month;
        tm.tm_year = watch->year;
        
        watch->csrb.reg &= ~WATCH_CSRB_M_SET;              /* Time doesn't update when the set bit is set. */
        
        splx( s );              /* re-enable interrupts. */
        
	/*
	 * Map years.  The mapping is as follows:
	 *
	 * 	CHIP	DATE		INTERNAL
	 *   00-69	2000-2069	100-169
	 *   70-99	1970-1999	70-99
	 *
	 * This mapping was chosen to allow the current readtodr() to work
	 * correctly.  Right now, readtodr() always sets the watch to a time
	 * within the year 1970, because it expects the watch hardware to have
	 * a limited range.
	 */
	if (tm.tm_year < 70) {
		tm.tm_year += 100;
	}
	
	/*
	 * If no error was encountered, convert the watch time to the 
	 * time in seconds since the epoch.  The read convert routine 
	 * expects the year field to contain the year - 
	 * 1900 (i.e. 1970 is 70, 2001 is 101), and the month
	 * field to contain a base 0 month index.
	 */
	tm.tm_mon -= 1;		/* Month bases are different. */
        secs = cvt_tm_to_sec(&tm);
        
        /* Need to convert from seconds to tics? */
        
        secs *= UNITSPERSEC;
        secs += TODRZERO;
        
        return(secs); /* actually returning 100 nano second count */
        
}



/*
 *  ka_ruby_writetodr(yrtime):
 *      inputs: 
 *        yrtime - is an unsigned longword which contains the number
 *        of seconds since the beginning of the year.
 *
 *      outputs:
 *          The function returns:
 *            1 - success if the write completes as expected
 *            0 - failure if the write fails to complete as expected
 */
ka_ruby_writetodr(yrtime)
        u_long yrtime;
{
        struct gbus_watch *watch;
        struct tm tm;
        int uip_loop, s;
        
        watch = gbus_map.watch;
        
#ifdef DEBUG
	if (yrtime < 60 ) ka_ruby_debug_hook();

	ruby_print_watch();
#endif
        cvt_sec_to_tm(yrtime, &tm);
	tm.tm_mon += 1;
        
        s = splhigh();

	for (uip_loop=0; ((watch->csra.reg & WATCH_CSRA_M_UIP) &
			(uip_loop < 30)) ; uip_loop++) {
	/* an Update is going to happen within 244 uS, hang out for a while */
	DELAY(10)
	}
	if (uip_loop == 30)
		printf("WARNING: Clock update operation took an unusual amount of time, check date/time\n");
        
	if( ka_ruby_check_todr_setup())
	{
                watch->csra.reg = WATCH_CSRA_SET;
                watch->csrb.reg = WATCH_CSRB_SET;
                watch->csrc.reg = WATCH_CSRC_SET;
        }
        
        watch->csrb.reg |= WATCH_CSRB_M_SET; 
        
        watch->seconds = tm.tm_sec;
        watch->minutes = tm.tm_min;
        watch->hours = tm.tm_hour;
        watch->day = tm.tm_mday;
        watch->month = tm.tm_mon;
        watch->year = tm.tm_year;
        
        watch->csrb.reg &= ~WATCH_CSRB_M_SET; 
        
        splx( s );
}



/*
 *  ka_ruby_check_todr_setup:
 *
 *	Check BB_WATCH CSRs for expected contents, return non-zero
 *	status when unexpected value is detected.
 */

int ka_ruby_check_todr_setup()
{
	struct gbus_watch *watch;
	unsigned char csra_image, csrb_image;

	watch = gbus_map.watch; /* get address where watch chip is mapped. */

	csra_image = (unsigned char) watch->csra.reg ;
	csrb_image = (unsigned char) watch->csrb.reg ;

       /* Make sure clock is setup as expected. */
        if (((csra_image & ~WATCH_CSRA_M_UIP) != WATCH_CSRA_SET) |
            (csrb_image != WATCH_CSRB_SET))
        { 
                printf("kn7aa_readtodr: BB_WATCH not setup properly\n");
                printf("                  expected csra: 0x%02x got csra: 0x%02x\n", WATCH_CSRA_SET, csra_image);
                printf("                  expected csrb: 0x%02x got csrb: 0x%02x\n", WATCH_CSRB_SET, csrb_image);
                printf("                  csrc: 0x%02x\n", watch->csrc.reg);
                printf("                  csrd: 0x%02x\n", (watch->csrd.reg & 0xff));
                return(1);
        }
	return(0);
}        



#ifdef DEBUG
int ka_ruby_debug_hook()
{
	
	char cp_buf[80];
	struct tm tm;
	u_long yrtime;
	
	yrtime = ka_ruby_readtodr();
	cvt_sec_to_tm(yrtime, &tm);
	tm.tm_year = 92;
	yrtime = cvt_tm_to_sec(&tm);
	
	time.tv_sec = yrtime;
	
	printf("\n\n***DBG: kn7aa_debug_hook: entered...\n");
	printf("Valid tests are: 0 - show BB_WATCH\n");
	printf("                 1 - Access LSB Node n (mcheck: 670)\n");
	printf("                 2 - Attempt to do an mcheck error log entry without an mcheck\n");
	printf("                 3 - Access non-exsistant memory, write (mcheck: 660?)\n");
	printf("                 4 - emulate 630\n");
	printf("                 4 - emulate set of 630's\n");
	printf("\nEnter test routine # and parameters: ");
	
	alpha_8530_cons_gets(&cp_buf[0]); 
	
	/*
	  
	  make a timeout routine
	  assert wait
	  printf current_thread() > state
	  Call thread_block
	  
	  */
	
	switch(cp_buf[0]) {
	case '0':
		ruby_print_watch();
		return(0);
		break;
	case '1':
	{/* cause 670 machine check */
		struct bus *lsbbus;
		struct lsbdata *lsbdata;
		struct lsb_reg *nxv;
		u_long addr_ptr;
		int lsbnode;
		
		lsbbus = get_sys_bus("lsb");
		lsbdata = get_lsb(lsbbus->bus_num);
		
		lsbnode = 9;
		while ((lsbnode < 0) || (lsbnode >= 9))
		{
			printf("Enter node to access: ");
			alpha_8530_cons_gets(&cp_buf[0]);  
			lsbnode = cp_buf[0] - '0';
		}
		
		nxv = lsbdata->lsbvirt[lsbnode];
		DBG_MSG(("Virtual Address of node %02d is: 0x%l016x", lsbnode, nxv ));
		if (svatophys(nxv, &addr_ptr) != KERN_SUCCESS)
		{
			DBG_MSG(("Unable to translate nxv"));
		}
		else
		{
			DBG_MSG(("Physical Address of node %02d is: 0x%l016x", lsbnode, addr_ptr ));
		}
		DBG_MSG(("nxv->lsb_ldev: "));
		DBG_MSG(("0x%l016x\n", nxv->lsb_ldev));
		/*				nxv->lsb_ldev = lsbdata->lsberr[lsbnode].plsbsw->lsb_ldev;  */
		
		return(0);
		
	}
		printf("\n 4 \n");
		break;
		
	case '2':
	{ /* Call the machine check logger */
		struct bus *lsbbus;
		struct lsbdata *lsbdata;
		struct lsb_reg *nxv;
		int lsbnode;
		struct elr_soft_flags *sw_flags;
		struct ruby_mchk_logout *mchk_logout;
		register struct ruby_mcheck_control *mctl;
		register struct ruby_mcheck_control_elbuf *elctl;
		
		/* This case hangs... */
		
		elctl = &mctl->mctl_elctl;
		
		/* get pointers to machine check control data structures */
		mctl = KN7AA_MCHECK_CONTROL(mfpr_whami());
		
		/* set logout pointer to virtual address of pal's cmcf, mapped */
		/* at ruby_mcheck_init time. This makes use of cmcf parameter */
		/* unnecessary while its contents stablize. it doesn't */
		/* Contain the right value. */
		mchk_logout = mctl->mctl_lgt_va; 
		
		sw_flags =(struct elr_soft_flags *)
			ruby_alloc_elbuf(elctl,(sizeof(struct elr_soft_flags)));
		if (sw_flags == 0)
		{
			panic("kn7aa: Unable to allocate elctl buffer");
		}
		
		sw_flags->packet_revision = 1;
		sw_flags->rsvd1 = 0;
		sw_flags->error_flags[0] = 0;
		sw_flags->error_flags[1] = 0;
		printf("Calling kn7aa_log_error\n");
		kn7aa_log_error(mchk_logout, sw_flags, 0x670L );
		printf("Returned from kn7aa_log_error\n");
		
		/* reset error log control */
		ruby_reset_elctl();
		
		return(0);
	}
		break;
		
	case '3':
	{/* log 660 machine check */

		struct elr_soft_flags sw_flags;
		struct ruby_mchk_logout *mchk_logout;
		long *addr;

		addr = (long *)PHYS_TO_KSEG(0xffffffff0);
		addr[0] = 0;
/*
		mchk_logout = (struct ruby_mchk_logout *)PHYS_TO_KSEG(0x6620);

		kn7aa_log_error(mchk_logout, &sw_flags, 0x660L);
*/
	}
		break;

	case '4':
	{/* log 630 machine check */

		struct elr_soft_flags sw_flags;
		struct ruby_mchk_logout *mchk_logout;

		/* initialize sw_flags */
		sw_flags.packet_revision = 0;
		sw_flags.rsvd1 = 0;
		sw_flags.error_flags[0] = 0;
		sw_flags.error_flags[1] = 0;
		
		sw_flags.error_flags[0] |= SW_FLAG0_ISTRM_BCACHE_SBE;		
		mchk_logout = (struct ruby_mchk_logout *)PHYS_TO_KSEG(0x6620);

		kn7aa_log_error(mchk_logout, &sw_flags, 0x630L);
	
		break;

	}
	
        case '5':
	{/* exercise PCE routines */

		mces_pce_init(4,2,0);
		mces_pce_handle();
		mces_pce_clear();
		mces_pce_handle();
		mces_pce_clear();
		mces_pce_handle();
		mces_pce_clear();
		mces_pce_handle();
		mces_pce_clear();
		mces_pce_handle();
		mces_pce_clear();

		break;

	}
	return(0);

	}

	
if (0)
{/* cause 630 machine check */
	register struct ruby_mcheck_control *mctl;

	/* fake it for now */

	/* get pointers to machine check control data structures */
	mctl = KN7AA_MCHECK_CONTROL(mfpr_whami());

	ruby_machcheck ( 0x630, mctl->mctl_lgt_pa, 0);

}

if (0)
{/* cause 660 machine check */
	struct bus *lsbbus;
	struct lsbdata *lsbdata;
	int lsbnode;

	lsbbus = get_sys_bus("lsb");
	lsbdata = get_lsb(lsbbus->bus_num);
	for(lsbnode = 0; lsbnode < MAX_LSB_NODE; lsbnode++) {
		lsbdata->lsbvirt[lsbnode]->lsb_ldev =
			lsbdata->lsberr[lsbnode].plsbsw->lsb_ldev;
	}
}

if (0)
{/* cause 670 machine check */
	struct bus *lsbbus;
	struct lsbdata *lsbdata;
	int lsbnode;

	lsbbus = get_sys_bus("lsb");
	lsbdata = get_lsb(lsbbus->bus_num);
	for(lsbnode = 0; lsbnode < MAX_LSB_NODE; lsbnode++) {
		DBG_MSG(("nxv->lsb_ldev: 0x%l016x\n",
			lsbdata->lsbvirt[lsbnode]->lsb_ldev));
	}
}

	return(0);

}
#endif


ruby_print_watch()
{
        struct gbus_watch *watch;
        struct tm tm;
        int s;
        
        watch = gbus_map.watch; /* get address where watch chip is mapped. */
        
        s = splhigh();          /* block interrupts */
        
        /* This blocks updates to clock registers, *
         * while the clock continues to keep time. */
        watch->csrb.reg |= WATCH_CSRB_M_SET;
        
        tm.tm_sec = watch->seconds;
        tm.tm_min = watch->minutes;
        tm.tm_hour = watch->hours;
        tm.tm_mday = watch->day;
        tm.tm_mon = watch->month;
        tm.tm_year = watch->year;
        
        watch->csrb.reg &= ~WATCH_CSRB_M_SET;
        
        splx( s );              /* return to previous IPL */
        
        /* convert from 12 to 24 hour time */
        if ( (watch->csrb.reg & WATCH_CSRB_M_HM) == 0 && tm.tm_hour >= 0x80)
        {
                tm.tm_hour -= 0x80 + 12; /* Change AM/PM bit into 12 hours */
        }
        
        printf ("***DBG: Read from Watch Chip\n");
        printf ("        Time: %l02d:%l02d:%l02d\n", tm.tm_hour, tm.tm_min, tm.tm_sec);
        printf ("        Date: %l02d/%l02d/%l04d\n", tm.tm_mon, tm.tm_mday, tm.tm_year);
        printf ("        csra:%l02x, csrb:%l02x, csrc:%l02x, csrd:%l02x\n", 
                watch->csra.reg, watch->csrb.reg, watch->csrc.reg, (watch->csrd.reg & 0xff));
        printf("\n");
        
        return(0);
        
}


/* ruby_mcheck_init :
 *	This routine is meant to be called at boot time to map the
 * 	machine check logout areas used by pal and required for machine
 * 	check parsing.
 */
int ruby_mcheck_init()
{
	register struct ruby_mcheck_control *mctl;
					/* to contain the pointer to */
					/* the machine check control */
					/* structure for the cpu this */
					/* executes on. */

	register long i;		/* loop counter */

        caddr_t map_logout_pg;		/* to contain the physical */
					/* page address of the PAL's */
					/* mcheck logout frame for cpu */
					/* which thisa executes on */

        u_long map_logout_pg_offset;	/* to contain the offset of */
					/* the mcheck logout frame */
					/* within the above page */

	register long map_logout_len;	/* to contain the size of */
					/* space to be mapped for the */
					/* mcheck logout frame. */

	register struct rpb_percpu *percpu;
					/* pointer, through rpb, to */
					/* cpu specific data */
					/* strucutres fo the cpu this */
					/* executes on. */

	register struct ruby_mcheck_control_elbuf *elctl;
					/* pointer to structure for */
					/* managing error logging data */
					/* structures for cpu on which */
					/* this executes */

	extern struct rpb *rpb;		/* pointer to RPB */

	/* get mctl strucuture for this processor */
	mctl = KN7AA_MCHECK_CONTROL(mfpr_whami());

        /*
	 * Set up the mapping for the logout area.  This is needed for the
	 * VMS PAL machine check handler.  Should be replaced with kseg mapping
	 * when the OSF PAL is available.
	 */
	
	/* get address of the cpu specific data structures for this cpu */
        percpu = (struct rpb_percpu *) ((long)rpb + rpb->rpb_percpu_off +
				(mfpr_whami() * rpb->rpb_slotsize));

	/*
	 * Verify contents of RPB regarding logout area. 
	 */
	if (percpu->rpb_logout == 0) {
		DBG_MSG(("kn7aa_mcheck_init: Uninitialized RPB field: rpb_logout\n"));
		DBG_MSG(("                               Initializing to 0x%l016x\n", KN7AA_MCHECK_FRAME_PA));	
		panic("kn7aa: uninitialized percpu->rpb_logout");
	} else {
		DBG_MSG_OFF(("kn7aa_mcheck_init: RPB field rpb_logout contains: 0x%l016x", percpu->rpb_logout));
	}
        if (percpu->rpb_logout_len == 0) {
		printf("kn7aa_mcheck_init: Uninitialized RPB field: rpb_logout_len\n");
		printf("                               Initializing to 0x%l016x\n", sizeof(struct ruby_mchk_logout));
		panic("kn7aa: uninitialized percpu->rpb_logout_len");
	} else {
		DBG_MSG_OFF(("kn7aa_mcheck_init: RPB field rpb_logout_len contains: 0x%l016x", percpu->rpb_logout_len));
	}


	/* get page address, offset and size for mapping logout area */
	map_logout_pg = (caddr_t)((u_long)percpu->rpb_logout & ~PGOFSET);

	map_logout_pg_offset = (percpu->rpb_logout & PGOFSET);

        map_logout_len = (((percpu->rpb_logout_len + map_logout_pg_offset) +
			    (NBPG-1)) / NBPG) * NBPG; 

	/* setup description of PAL's logout frame in mctl structure */
        mctl->mctl_lgt_len = percpu->rpb_logout_len;

	/* OSF expects data from the PAL logout frame starting at +20 */
	mctl->mctl_lgt_pa = percpu->rpb_logout + 0x20;
	
	/* calculate KSEG address for PAL's logout frame */
	mctl->mctl_lgt_va = (struct ruby_mchk_logout *)
		PHYS_TO_KSEG(map_logout_pg);
	
#ifdef MCHECK_BUFFER_NOT_IMPLEMENTED
	/*
	 * If machine check buffering is configured, set up the buffer area
	 * and controls.
	 */
	printf("Machine check buffering is enabled\n");
	mctl->mctl_lgt_copy =
		(struct el_alpha_ev4_mcheck_logout *)
			kmem_alloc(kernel_map, map_logout_len);
	mctl->mctl_next_copy = 0;
	for (i = 0; i < LGT_COPIES; i++) {
		mctl->mctl_copy_in_use[i] = 0;
	}
#endif /* MCHECK_BUFFER */

	/* initialize BADADDR control parameters */
	mctl->mctl_mcheck_ok = 0;
	mctl->mctl_probe_va = 0;
	mctl->mctl_badaddr_count = 0;
	
	/* initialize error counters */
	bzero(&mctl->error_670_counters,
	      sizeof(mctl->error_670_counters));

	bzero(&mctl->error_660_counters,
	      sizeof(mctl->error_660_counters));

	bzero(&mctl->error_630_counters,
	      sizeof(mctl->error_630_counters));

	bzero(&mctl->error_620_counters,
	      sizeof(mctl->error_620_counters));

	/*
	 * Allocate the scratch area to hold the error log packet.  This takes
	 * another page per cpu.  Eventually, will want to consolidate this
	 * with the memory allocated for logout frame copies. It has been
	 * assumed that an error log packet will not exceed a page here.
	 */
	elctl = &mctl->mctl_elctl;
	elctl->elctl_buf = kmem_alloc(kernel_map, NBPG); 
	elctl->elctl_len = NBPG;
	elctl->elctl_ptr = elctl->elctl_buf;
	elctl->elctl_rmng = elctl->elctl_len;

	/* now initialize and enable corrected error reporting */
	mces_pce_init(4,2,0);

	return 0;
}

/*
 * The severity of a machine check determines what we can do.  There are
 * three severities:
 *
 *   System Fatal    - The kernel state is corrupt.  Crash (if you can...).
 *   Processor Fatal - This processor is toast.  For now, crash...
 *   Process Fatal   - A process is corrupt.  Blow it away.
 *   Survivable      - Either the error was corrected, or we didn't expect
 *		       to keep running under this error.  Since we're still
 *		       running, it must have been a transient.
 *   Probe	   - The machine check was the result of a bad address probe.
 *
 * These severity codes are passed to the logging  routine (in case it cares).
 */
#define MCHECK_SEV_PROBE 0L
#define MCHECK_SEV_SURVIVABLE 1L
#define MCHECK_SEV_SYSTEM_FATAL 2L
#define MCHECK_SEV_PROCESS_FATAL 3L
#define MCHECK_SEV_PROCESSOR_FATAL 4L



int ruby_mcheck_logout_dump(mchk_logout, sw_flags)
 	struct ruby_mchk_logout *mchk_logout;	/* This is the virtual */
						/* address of the */
						/* machine check frame */
						/* used by pal */
     struct elr_soft_flags *sw_flags;
				/* address of the software flags, */
				/* which are used to return parse */
				/* results and as part of the defined */
				/* error packet */
{
	u_long *addr_ptr;

#ifdef DEBUG
        DBG_MSG(("Machine Check Error logout DUMP:"));

	addr_ptr = (u_long *)&mchk_logout->retry;
        svatophys(addr_ptr, &addr_ptr);
        DBG_MSG((" retry:               PA: 0x%l016x,    0x%l016x", addr_ptr, mchk_logout->retry));

	addr_ptr = (u_long *)&mchk_logout->proc_off;
        svatophys(addr_ptr, &addr_ptr);
        DBG_MSG((" proc_off:            PA: 0x%l016x,    0x%l016x", addr_ptr, mchk_logout->proc_off));

	addr_ptr = (u_long *)&mchk_logout->sys_off;
        svatophys(addr_ptr, &addr_ptr);
        DBG_MSG((" sys_off:             PA: 0x%l016x,    0x%l016x", addr_ptr, mchk_logout->sys_off));

	addr_ptr = (u_long *)&mchk_logout->pal_temps[32*2];
        svatophys(addr_ptr, &addr_ptr);
        DBG_MSG_OFF((" pal_temps[32*2]:         PA: 0x%l016x,    0x%l016x", addr_ptr, mchk_logout->pal_temps[32*2]));

	addr_ptr = (u_long *)&mchk_logout->exc_addr;
        svatophys(addr_ptr, &addr_ptr);
        DBG_MSG((" exc_addr:            PA: 0x%l016x,    0x%l016x", addr_ptr, mchk_logout->exc_addr));

	addr_ptr = (u_long *)&mchk_logout->exc_sum;
        svatophys(addr_ptr, &addr_ptr);
        DBG_MSG((" exc_sum:             PA: 0x%l016x,    0x%l016x", addr_ptr, mchk_logout->exc_sum));

	addr_ptr = (u_long *)&mchk_logout->iccsr;
        svatophys(addr_ptr, &addr_ptr);
        DBG_MSG((" iccsr:               PA: 0x%l016x,    0x%l016x", addr_ptr, mchk_logout->iccsr));

	addr_ptr = (u_long *)&mchk_logout->pal_base;
        svatophys(addr_ptr, &addr_ptr);
        DBG_MSG((" pal_base:            PA: 0x%l016x,    0x%l016x", addr_ptr, mchk_logout->pal_base));

	addr_ptr = (u_long *)&mchk_logout->hier;
        svatophys(addr_ptr, &addr_ptr);
        DBG_MSG((" hier:                PA: 0x%l016x,    0x%l016x", addr_ptr, mchk_logout->hier));

	addr_ptr = (u_long *)&mchk_logout->hirr;
        svatophys(addr_ptr, &addr_ptr);
        DBG_MSG((" hirr:                PA: 0x%l016x,    0x%l016x", addr_ptr, mchk_logout->hirr));

	addr_ptr = (u_long *)&mchk_logout->mm_csr;
        svatophys(addr_ptr, &addr_ptr);
        DBG_MSG((" mm_csr:              PA: 0x%l016x,    0x%l016x", addr_ptr, mchk_logout->mm_csr));

	addr_ptr = (u_long *)&mchk_logout->dc_stat;
        svatophys(addr_ptr, &addr_ptr);
        DBG_MSG((" dc_stat:             PA: 0x%l016x,    0x%l016x", addr_ptr, mchk_logout->dc_stat));

	addr_ptr = (u_long *)&mchk_logout->dc_addr;
        svatophys(addr_ptr, &addr_ptr);
        DBG_MSG((" dc_addr:             PA: 0x%l016x,    0x%l016x", addr_ptr, mchk_logout->dc_addr));

	addr_ptr = (u_long *)&mchk_logout->abox_ctl;
        svatophys(addr_ptr, &addr_ptr);
        DBG_MSG((" abox_ctl:            PA: 0x%l016x,    0x%l016x", addr_ptr, mchk_logout->abox_ctl));

	addr_ptr = (u_long *)&mchk_logout->biu_stat;
        svatophys(addr_ptr, &addr_ptr);
        DBG_MSG((" biu_stat:            PA: 0x%l016x,    0x%l016x", addr_ptr, mchk_logout->biu_stat));

	addr_ptr = (u_long *)&mchk_logout->biu_addr;
        svatophys(addr_ptr, &addr_ptr);
        DBG_MSG((" biu_addr:            PA: 0x%l016x,    0x%l016x", addr_ptr, mchk_logout->biu_addr));

	addr_ptr = (u_long *)&mchk_logout->biu_ctl;
        svatophys(addr_ptr, &addr_ptr);
        DBG_MSG((" biu_ctl:             PA: 0x%l016x,    0x%l016x", addr_ptr, mchk_logout->biu_ctl));

	addr_ptr = (u_long *)&mchk_logout->fill_syndrome;
        svatophys(addr_ptr, &addr_ptr);
        DBG_MSG((" fill_syndrome:       PA: 0x%l016x,    0x%l016x", addr_ptr, mchk_logout->fill_syndrome));

	addr_ptr = (u_long *)&mchk_logout->fill_addr;
        svatophys(addr_ptr, &addr_ptr);
        DBG_MSG((" fill_addr:           PA: 0x%l016x,    0x%l016x", addr_ptr, mchk_logout->fill_addr));

	addr_ptr = (u_long *)&mchk_logout->va;
        svatophys(addr_ptr, &addr_ptr);
        DBG_MSG((" va:                  PA: 0x%l016x,    0x%l016x", addr_ptr, mchk_logout->va));

	addr_ptr = (u_long *)&mchk_logout->bc_tag;
        svatophys(addr_ptr, &addr_ptr);
        DBG_MSG((" bc_tag:              PA: 0x%l016x,    0x%l016x", addr_ptr, mchk_logout->bc_tag));

	addr_ptr = (u_long *)&mchk_logout->lep_gbus;
        svatophys(addr_ptr, &addr_ptr);
        DBG_MSG((" lep_gbus:            PA: 0x%l016x,    0x%l016x", addr_ptr, mchk_logout->lep_gbus));

	addr_ptr = (u_long *)&mchk_logout->lep_lmode;
        svatophys(addr_ptr, &addr_ptr);
        DBG_MSG((" lep_lmode:           PA: 0x%l016x,    0x%l016x", addr_ptr, mchk_logout->lep_lmode));

	addr_ptr = (u_long *)&mchk_logout->lep_lmerr;
        svatophys(addr_ptr, &addr_ptr);
        DBG_MSG((" lep_lmerr:           PA: 0x%l016x,    0x%l016x", addr_ptr, mchk_logout->lep_lmerr));

	addr_ptr = (u_long *)&mchk_logout->lep_llock;
        svatophys(addr_ptr, &addr_ptr);
        DBG_MSG((" lep_llock:           PA: 0x%l016x,    0x%l016x", addr_ptr, mchk_logout->lep_llock));

	addr_ptr = (u_long *)&mchk_logout->lep_lber;
        svatophys(addr_ptr, &addr_ptr);
        DBG_MSG((" lep_lber:            PA: 0x%l016x,    0x%l016x", addr_ptr, mchk_logout->lep_lber));

	addr_ptr = (u_long *)&mchk_logout->lep_lcnr;
        svatophys(addr_ptr, &addr_ptr);
        DBG_MSG((" lep_lcnr:            PA: 0x%l016x,    0x%l016x", addr_ptr, mchk_logout->lep_lcnr));

	addr_ptr = (u_long *)&mchk_logout->lep_ldev;
        svatophys(addr_ptr, &addr_ptr);
        DBG_MSG((" lep_ldev:            PA: 0x%l016x,    0x%l016x", addr_ptr, mchk_logout->lep_ldev));

	addr_ptr = (u_long *)&mchk_logout->lep_lbesr0;
        svatophys(addr_ptr, &addr_ptr);
        DBG_MSG((" lep_lbesr0:          PA: 0x%l016x,    0x%l016x", addr_ptr, mchk_logout->lep_lbesr0));

	addr_ptr = (u_long *)&mchk_logout->lep_lbesr1;
        svatophys(addr_ptr, &addr_ptr);
        DBG_MSG((" lep_lbesr1:          PA: 0x%l016x,    0x%l016x", addr_ptr, mchk_logout->lep_lbesr1));

	addr_ptr = (u_long *)&mchk_logout->lep_lbesr2;
        svatophys(addr_ptr, &addr_ptr);
        DBG_MSG((" lep_lbesr2:          PA: 0x%l016x,    0x%l016x", addr_ptr, mchk_logout->lep_lbesr2));

	addr_ptr = (u_long *)&mchk_logout->lep_lbesr3;
        svatophys(addr_ptr, &addr_ptr);
        DBG_MSG((" lep_lbesr3:          PA: 0x%l016x,    0x%l016x", addr_ptr, mchk_logout->lep_lbesr3));

	addr_ptr = (u_long *)&mchk_logout->lep_lbecr0;
        svatophys(addr_ptr, &addr_ptr);
        DBG_MSG((" lep_lbecr0:          PA: 0x%l016x,    0x%l016x", addr_ptr, mchk_logout->lep_lbecr0));

	addr_ptr = (u_long *)&mchk_logout->lep_lbecr1;
        svatophys(addr_ptr, &addr_ptr);
        DBG_MSG((" lep_lbecr1:          PA: 0x%l016x,    0x%l016x", addr_ptr, mchk_logout->lep_lbecr1));

#endif /* DEBUG */
}


/*
 * ruby_alloc_elbuf
 *
 * Allocate space in the machine-specific error logging scratch buffer.  The
 * machine check parser allocates space for packets in this buffer.  The
 * scratch buffer is used to sequentially build up a complete error log
 * packet, before doing an ealloc and sending the packet.
 *
 * This is needed because there is no way to "shrink" a packet once it has
 * been ealloced.
 */
vm_offset_t ruby_alloc_elbuf(elctl, size)
	register struct ruby_mcheck_control_elbuf *elctl;
	register long size;
{
	register vm_offset_t ptr;
	register long rmng;

	/*
	 * If there is enough space left in the buffer, allocate the frame.
	 * If not, barf.
	 */
	rmng = elctl->elctl_rmng - size;
	if (rmng < 0) {
		ptr = 0;
	}
	else {
		ptr = elctl->elctl_ptr;
		elctl->elctl_ptr += size;
		elctl->elctl_rmng = rmng;
	}

	return ptr;
}


/* removed ruby_errlog_valid */

/*
 * Start building the error frame.  Allocate and initialize the Ruby error
 * log header right away.
 */
int ruby_reset_elctl()
{
	register struct ruby_mcheck_control *mctl;
	register struct ruby_mcheck_control_elbuf *elctl;

	/* get pointers to machine check control data structures */
	mctl = KN7AA_MCHECK_CONTROL(mfpr_whami());

	/* get pointer to mcheck error logging control structure */
	elctl = &mctl->mctl_elctl;

	/*
	 * Initialize the scratch buffer as empty.
	 */
	elctl = &mctl->mctl_elctl;
	elctl->elctl_ptr = elctl->elctl_buf;
	elctl->elctl_rmng = elctl->elctl_len;
}




/* #define PRINT_PARSE */

#ifdef PRINT_PARSE
#undef PRINT_PARSE
#define PRINT_PARSE(msg) \
  dprintf ("Error Parse: "); \
  dprintf msg ; \
  dprintf ("\n")
#else
#undef PRINT_PARSE
#define PRINT_PARSE(msg)
#endif



int ruby_670_parse_herr_rdblk(mchk_logout, sw_flags) /* 670 level 1*/
     struct ruby_mchk_logout *mchk_logout;
				/* address of the machine check frame */
				/* to use for parsing */
     struct elr_soft_flags *sw_flags;
				/* address of the software flags, */
				/* which are used to return parse */
				/* results and as part of the defined */
				/* error packet */
{/* biu herr read block */ /* select one */

  PARSE_TRACE(("ruby_670_parse_herr_rdblk"));
  
  if (mchk_logout->lep_lber & LBER_NSES)
				/* if there was an error specific to */
				/* this node. */
    {/* select one */

      if (mchk_logout->lep_lmerr & LMERR_ARBDROP)
	{/* 8.1.26 */
	  PRINT_PARSE((" 8.1.26 "));
	  sw_flags->error_flags[0] |= 
	    SW_FLAG0_670_ARBDROP | SW_FLAG0_CPU_NSES; 
				/* basic mcheck entry */
				/* lsb subpacket */
	  sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT;

	  ERROR_CRASH;
	}
      else if (mchk_logout->lep_lmerr & LMERR_ARBCOL)
	{/* 8.1.27 */
	  PRINT_PARSE((" 8.1.27 "));
	  sw_flags->error_flags[0] = sw_flags->error_flags[0] |
	    SW_FLAG0_670_ARBCOL | SW_FLAG0_CPU_NSES;
				/* basic mcheck entry */
				/* lsb subpacket */
	  sw_flags->error_flags[1] = sw_flags->error_flags[1] |
	    SW_FLAG1_LSB_PRESENT;

	  ERROR_CRASH;
	}
      else
	{/* 8.1.28 */
	  PRINT_PARSE((" 8.1.28 "));
	  sw_flags->error_flags[0] = sw_flags->error_flags[0] |
	    SW_FLAG0_670_INCONSISTENT_1 | SW_FLAG0_CPU_NSES;
				/* basic mcheck entry */
				/* lsb subpacket */	
	  sw_flags->error_flags[1] = sw_flags->error_flags[1] |
	    SW_FLAG1_LSB_PRESENT;

	  ERROR_CRASH;
	}
    }
  
  else if ((mchk_logout->lep_lber & LBER_E) &&
	   (((mchk_logout->lep_lbecr1 & LBECR1_CID) >>
	     LBECR1_CID_SHIFT) == mfpr_whami()))
				/* if there was a bus error and this */
				/* cpu was the bus commander. */
    {
      int found_lber_e_cmdr_err=0;
      
      PARSE_TRACE(("if ((mchk_logout->lep_lber & LBER_E) && \
          (((mchk_logout->lep_lbecr1 & LBECR1_CID) >>\
	     LBECR1_CID_SHIFT) == mfpr_whami()))\n"));

      if (mchk_logout->lep_lber & (LBER_SHE | LBER_DIE))
	{/* 8.1.29 */
	  PRINT_PARSE((" 8.1.29 "));
	  found_lber_e_cmdr_err++;
	  
	  sw_flags->error_flags[0] =
	    (sw_flags->error_flags[0] |
	     SW_FLAG0_LSB_ERR_READ |
	     SW_FLAG0_EV4_READBLOCK);
	  
	  if (mchk_logout->lep_lber & LBER_SHE)
	    sw_flags->error_flags[0] =
	      (sw_flags->error_flags[0] |
	       SW_FLAG0_SHARED_ERROR);
	  
	  if (mchk_logout->lep_lber & LBER_DIE)
	    sw_flags->error_flags[0] =
	      (sw_flags->error_flags[0] |
	       SW_FLAG0_DIRTY_ERROR);
				/* basic mcheck entry */
				/* lsb subpacket */
	  sw_flags->error_flags[1] = sw_flags->error_flags[1] |
	    SW_FLAG1_LSB_PRESENT;

	  ERROR_CRASH;
	}
      
      if (mchk_logout->lep_lber & (LBER_STE | LBER_CNFE | LBER_CAE ))
     
	{/* 8.1.30 */
	  PRINT_PARSE((" 8.1.30 "));
	  found_lber_e_cmdr_err++;
	  sw_flags->error_flags[0] = sw_flags->error_flags[0] |
	    (SW_FLAG0_LSB_ERR_READ | SW_FLAG0_EV4_READBLOCK);
	  
	  if (mchk_logout->lep_lber & LBER_STE)
	    sw_flags->error_flags[0] =
	      sw_flags->error_flags[0] | SW_FLAG0_STALL_ERROR;
	  if (mchk_logout->lep_lber & LBER_CNFE)
	    sw_flags->error_flags[0] =
	      sw_flags->error_flags[0] | SW_FLAG0_CNF_ERROR;
	  if (mchk_logout->lep_lber & LBER_CAE)
	    sw_flags->error_flags[0] =
	      sw_flags->error_flags[0] | SW_FLAG0_CA_ERROR;
	  
				/* basic mcheck entry */
				/* lsb subpacket */
	  sw_flags->error_flags[1] = sw_flags->error_flags[1] |
	    SW_FLAG1_LSB_PRESENT;

	  ERROR_CRASH;
	}
      
      if (mchk_logout->lep_lber & LBER_NXAE)
	{/* NXAE */
	  
	  PARSE_TRACE_OFF(("if (mchk_logout->lep_lber & LBER_NXAE)\n"));

	  if ( (mchk_logout->lep_lbecr1 & LBECR1_CA_CMD ) ==
	      LBECR1_CA_READ_CSR)
	    {/* 8.1.31 */
	      PRINT_PARSE((" 8.1.31 "));
	      found_lber_e_cmdr_err++;
	      sw_flags->error_flags[0] =
		(sw_flags->error_flags[0] |
		 SW_FLAG0_LSB_ERR_RD_CSR |
		 SW_FLAG0_NXM_CSR_READ |
		 SW_FLAG0_EV4_READBLOCK);
	      sw_flags->error_flags[1] =
		(sw_flags->error_flags[1] |
		 SW_FLAG1_LSB_PRESENT);
	      
	      /* ***PRM: Filter this on the address */
	      /* */
	      
	      sw_flags->error_flags[1] =
		(sw_flags->error_flags[1] |
		 SW_FLAG1_LOG_ADAPT_PRES);
	    }
	  
	  if ( (mchk_logout->lep_lbecr1 & LBECR1_CA_CMD ) ==
	      LBECR1_CA_READ)
	    {/* 8.1.32 */
	      PRINT_PARSE((" 8.1.32 "));
	      found_lber_e_cmdr_err++;
	      sw_flags->error_flags[0] =
		(sw_flags->error_flags[0] |
		 SW_FLAG0_LSB_ERR_RD_CSR |
		 SW_FLAG0_NXM_MEM_READ |
		 SW_FLAG0_EV4_READBLOCK);
	      sw_flags->error_flags[1] =
		(sw_flags->error_flags[1] |
		 SW_FLAG1_LMA_PRESENT |
		 SW_FLAG1_LSB_PRESENT);
	    }
	  
	  if ( (mchk_logout->lep_lbecr1 & LBECR1_CA_CMD ) ==
	      LBECR1_CA_PRIVATE)
	    {/* 8.1.33 */
	      PRINT_PARSE((" 8.1.33 "));
	      found_lber_e_cmdr_err++;
	      sw_flags->error_flags[0] =
		(sw_flags->error_flags[0] |
		 SW_FLAG0_LSB_ERR_PRIVATE |
		 SW_FLAG0_NXM_PRIVATE_SPACE |
		 SW_FLAG0_EV4_READBLOCK);
	    }
	}/* NXAE */
      
      if (mchk_logout->lep_lber & LBER_CPE2)
	{/* 8.1.34 */
	  PRINT_PARSE((" 8.1.34 "));
	  sw_flags->error_flags[0] =
	    (sw_flags->error_flags[0] |
	     SW_FLAG0_670_CPE2);
	  
	}
      if (mchk_logout->lep_lber & LBER_CPE)
	{/* 8.1.35 */
	  PRINT_PARSE((" 8.1.35 "));
	  found_lber_e_cmdr_err++;
	  
	  sw_flags->error_flags[0] =
	    (sw_flags->error_flags[0] |
	     SW_FLAG0_670_CPE |
	     SW_FLAG0_LSB_ERR_READ |
	     SW_FLAG0_EV4_READBLOCK);
	  
	  sw_flags->error_flags[1] = sw_flags->error_flags[1] |
	    SW_FLAG1_LSB_PRESENT;
	  
	}
      if (mchk_logout->lep_lber & LBER_CTCE)
	{/* 8.1.36 */
	  PRINT_PARSE((" 8.1.36 "));
	  found_lber_e_cmdr_err++;
	  
	  sw_flags->error_flags[0] =
	    (sw_flags->error_flags[0] |
	     SW_FLAG0_CTCE |
	     SW_FLAG0_LSB_ERR_READ |
	     SW_FLAG0_EV4_READBLOCK);
	  
	  sw_flags->error_flags[1] = sw_flags->error_flags[1] |
	    SW_FLAG1_LSB_PRESENT;
	  
	}
      if (mchk_logout->lep_lber & LBER_UCE2)
	{/* 8.1.37 */
	  PRINT_PARSE((" 8.1.37 "));
	  
	  sw_flags->error_flags[0] |= SW_FLAG0_LSB_UCE2;
	  
	}
      if ((mchk_logout->lep_lber & LBER_CDPE) &&
	  ( (mchk_logout->lep_lbecr1 & LBECR1_CA_CMD ) ==
	   LBECR1_CA_READ_CSR)) 
	{/* 8.1.38 */
	  PRINT_PARSE((" 8.1.38 "));
	  found_lber_e_cmdr_err++;
	  
	  sw_flags->error_flags[0] =
	    (sw_flags->error_flags[0] |
	     SW_FLAG0_LSB_ERR_RD_CSR |
	     SW_FLAG0_LSB_CDPE |
	     SW_FLAG0_EV4_READBLOCK);
	  
	  if (0) /* if cycle type was LSB Private */
	    {
	      sw_flags->error_flags[0] =
		      (sw_flags->error_flags[0] |
		       SW_FLAG0_LSB_ERR_PRIVATE);
	    }	      
	  else if (0) /* if cycle type was LSB READ CSR */
	    {
	      sw_flags->error_flags[0] =
		      (sw_flags->error_flags[0] |
		       SW_FLAG0_LSB_ERR_RD_CSR);
	    }	      

	  sw_flags->error_flags[1] =
	    (sw_flags->error_flags[1] |
	     SW_FLAG1_LSB_PRESENT);
	  
	  if (1) /* ***PRM: if address was IOP's */
	    {
	      sw_flags->error_flags[1] =
		(sw_flags->error_flags[1] |
		 SW_FLAG1_LOG_ADAPT_PRES);
	    }

	  if (1) /* ***PRM: if address was MEM's */
	    {
	      sw_flags->error_flags[1] =
		(sw_flags->error_flags[1] |
		 SW_FLAG1_LMA_PRESENT);
	    }
	}
      
      if ((mchk_logout->lep_lber & LBER_CDPE) &&
	  ( (mchk_logout->lep_lbecr1 & LBECR1_CA_CMD ) ==
	   LBECR1_CA_PRIVATE)) 
	{/* 8.1.38 */
	  PRINT_PARSE((" 8.1.38 "));
	  found_lber_e_cmdr_err++;
	  
	  sw_flags->error_flags[0] =
	    (sw_flags->error_flags[0] |
	     SW_FLAG0_LSB_ERR_PRIVATE |
	     SW_FLAG0_LSB_CDPE |
	     SW_FLAG0_EV4_READBLOCK);
	  
	  sw_flags->error_flags[1] =
	    (sw_flags->error_flags[1] |
	     SW_FLAG1_LSB_PRESENT);
	  
	  if (0) /* ***PRM: if address was IOP's */
	    {
	      sw_flags->error_flags[1] =
		(sw_flags->error_flags[1] |
		 SW_FLAG1_LOG_ADAPT_PRES);
	    }
	  else if (0) /* ***PRM: if address was MEM's */
	    {
	      sw_flags->error_flags[1] =
		(sw_flags->error_flags[1] |
		 SW_FLAG1_LMA_PRESENT);
	    }
	}
      
      if (mchk_logout->lep_lber & LBER_CE2)
	{/* 8.1.39 */
	  PRINT_PARSE((" 8.1.39 "));
	  
	  sw_flags->error_flags[1] =
	    (sw_flags->error_flags[1] |
	     SW_FLAG1_LSB_CE2);
	  
	}
      if (found_lber_e_cmdr_err == 0)
	{/* 8.1.40 */
	  PRINT_PARSE((" 8.1.40 "));
	  
	  sw_flags->error_flags[0] =
	    (sw_flags->error_flags[0] |
	     SW_FLAG0_LSB_ERR_READ |
	     SW_FLAG0_EV4_READBLOCK);
	  
	  sw_flags->error_flags[1] =
	    (sw_flags->error_flags[1] |
	     SW_FLAG1_670_INCONSISTENT_2);
	}
    }
  else if (mchk_logout->lep_lber & LBER_E)
    {/* 8.1.41 */
      PRINT_PARSE((" 8.1.41 "));
      
      sw_flags->error_flags[0] =
	(sw_flags->error_flags[0] |
	 SW_FLAG0_EV4_READBLOCK);
      
      sw_flags->error_flags[1] = sw_flags->error_flags[1] |
	SW_FLAG1_PREVIOUS_SYSTEM_ERROR_LATCHED |
	  SW_FLAG1_LSB_PRESENT;
      
    }
				/* What if parsing fails and comes up */
				/* with none of these? */
}



int ruby_parse_670_mcheck(mchk_logout, sw_flags)
     struct ruby_mchk_logout *mchk_logout;
				/* address of the machine check frame */
				/* */
				/* to use for parsing */
     struct elr_soft_flags *sw_flags;
				/* address of the software flags, */
				/* which are used to return parse */
				/* results and as part of the defined */
				/* error packet */
{
  register struct ruby_mcheck_control *mctl;
  register struct ruby_mcheck_control_elbuf *elctl;
  struct elr_lep_mchk_frame *mchk_frame;
  struct elr_lep_common_header *common_header;
  long *pal_rev;
  int log_packets=0;
  int found_670_error = 0;
  int unexpected_error_found=0;
  
  PRINT_PARSE(("670 Error Parsing: entered..."));
  
  /*
   * Get address for building MCHECK 670 error log data
   */
  mctl = KN7AA_MCHECK_CONTROL(mfpr_whami());
  elctl = &mctl->mctl_elctl;
  
  
  /*
   * Top level 670 Parse decision. Conditional on the state of
   * bits in BIU_STAT. This is coded such that any number of
   * conditions can be detected at the same time.
   */
  
  /* Select all */
  
  DBG_MSG_OFF(("&mchk_logout->biu_stat: 0x%l016x\n", &mchk_logout->biu_stat));
  DBG_MSG_OFF(("mchk_logout->biu_stat: 0x%l016x\n", mchk_logout->biu_stat));

  if (mchk_logout->biu_stat & BIU_SEO)
    {
      DBG_MSG(("  if (mchk_logout->biu_stat & BIU_SEO)\n"));
      ruby_670_biu_seo(mchk_logout, sw_flags);
    }
  
  if (mchk_logout->biu_stat & FILL_SEO)
    {
      DBG_MSG(("  if (mchk_logout->biu_stat & FILL_SEO)\n"));
      ruby_670_fill_seo(mchk_logout, sw_flags);
    }
  
  if (mchk_logout->biu_stat & BC_TPERR)
    {
      DBG_MSG(("  if (mchk_logout->biu_stat & BC_TPERR)\n"));
      found_670_error++;
      ruby_670_bc_tperr(mchk_logout, sw_flags);
    }
  
  if (mchk_logout->biu_stat & BC_TCPERR)
    {
      DBG_MSG(("  if (mchk_logout->biu_stat & BC_TCPERR)\n"));
      found_670_error++;
      ruby_670_bc_tcperr(mchk_logout, sw_flags);
    }
  
  if (mchk_logout->biu_stat & FILL_ECC)
    {
      DBG_MSG(("  if (mchk_logout->biu_stat & FILL_ECC)\n"));
      found_670_error++;
      ruby_670_fill_ecc(mchk_logout, sw_flags);
    }


  DBG_MSG_OFF(("BIU_CMD: 0x%l016x", BIU_CMD));
  DBG_MSG_OFF(("(mchk_logout->biu_stat & BIU_CMD): 0x%l016x", (mchk_logout->biu_stat & BIU_CMD)));
  
  if ((mchk_logout->biu_stat & BIU_HERR) &&
      ((mchk_logout->biu_stat & BIU_CMD) == BIU_CMD_READ_BLOCK))
    {
      DBG_MSG_OFF(("  if ((mchk_logout->biu_stat & BIU_HERR) &&\n"));
      found_670_error++;
      ruby_670_parse_herr_rdblk(mchk_logout, sw_flags);
    }
  
  if ((mchk_logout->biu_stat & BIU_HERR) &&
      ((mchk_logout->biu_stat & BIU_CMD) == BIU_CMD_WRITE_BLOCK))
    {
      DBG_MSG_OFF(("  if ((mchk_logout->biu_stat & BIU_HERR) &&\n"));
      found_670_error++;
      ruby_670_herr_write_block(mchk_logout, sw_flags);
    }
  
  if ((mchk_logout->biu_stat & BIU_HERR) &&
      ((mchk_logout->biu_stat & BIU_CMD) == BIU_CMD_LDxL))
  {
    DBG_MSG_OFF(("  if ((mchk_logout->biu_stat & BIU_HERR) &&      (mchk_logout->biu_stat & BIU_CMD) == BIU_CMD_LDxL)))\n"));
      found_670_error++;
      ruby_670_herr_ldxl(mchk_logout, sw_flags);
    }
  
  if ((mchk_logout->biu_stat & BIU_HERR) &&
      ((mchk_logout->biu_stat & BIU_CMD) == BIU_CMD_STxC))
    {
      DBG_MSG_OFF(("  if ((mchk_logout->biu_stat & BIU_HERR) &&      (mchk_logout->biu_stat & BIU_CMD == BIU_CMD_STxC)) \n"));
      found_670_error++;
      ruby_670_herr_stxc(mchk_logout, sw_flags);
    }
  
				/*
   * test if no errors where found
   */
  if (found_670_error == 0)
    {/* 8.1.91 */
      PRINT_PARSE((" 8.1.91 "));
      
				/* There were no expected bits set !!! */
				/* */
      
      sw_flags->error_flags[1] =
	(sw_flags->error_flags[1] |
	 SW_FLAG1_670_INCONSISTENT_2 |
	 SW_FLAG1_LSB_PRESENT);
      
      ruby_mcheck_logout_dump(mchk_logout, sw_flags);
      
    }
  else
    {
	    /* There is an error */
	    ruby_mcheck_logout_dump(mchk_logout, sw_flags);
    }
}



int ruby_670_parse_lsb_readerr(mchk_logout, sw_flags)
     struct ruby_mchk_logout *mchk_logout;
				/* address of the machine check frame */
				/* to use for parsing */
     struct elr_soft_flags *sw_flags;
				/* address of the software flags, */
				/* which are used to return parse */
				/* results and as part of the defined */
				/* error packet */
{/* LSB Read Error */ /* select all */
  int lsb_data_fetch_error_found=0;

  PARSE_TRACE(("ruby_670_parse_lsb_readerr"));
  
  if (mchk_logout->lep_lber & (LBER_SHE | LBER_DIE))
    {/* 8.1.48 */
      PRINT_PARSE((" 8.1.48 "));
      
      lsb_data_fetch_error_found++;
      
      sw_flags->error_flags[0] =
	(sw_flags->error_flags[0] |
	 SW_FLAG0_EV4_WRITEBLOCK |
	 SW_FLAG0_LSB_ERR_READ);
      
      sw_flags->error_flags[1] = sw_flags->error_flags[1] |
	SW_FLAG1_LSB_PRESENT;
      
      if (mchk_logout->lep_lber & LBER_SHE)
	{
	  sw_flags->error_flags[0] =
	    (sw_flags->error_flags[0] |
	     SW_FLAG0_SHARED_ERROR);
	}
      if (mchk_logout->lep_lber & LBER_DIE);
      {
	sw_flags->error_flags[0] =
	  (sw_flags->error_flags[0] |
	   SW_FLAG0_DIRTY_ERROR);
      }
    }/* 8.1.48 */
  
  if (mchk_logout->lep_lber & (LBER_STE | LBER_CNFE | LBER_CAE ))
    {/* 8.1.49 */
      PRINT_PARSE((" 8.1.49 "));
      
      lsb_data_fetch_error_found++;
      
      sw_flags->error_flags[0] =
	(sw_flags->error_flags[0] |
	 SW_FLAG0_LSB_ERR_READ |
	 SW_FLAG0_EV4_WRITEBLOCK);
      
      sw_flags->error_flags[1] = sw_flags->error_flags[1] |
	SW_FLAG1_LSB_PRESENT;
      
      if (mchk_logout->lep_lber & LBER_STE)
	{
	  sw_flags->error_flags[0] =
	    (sw_flags->error_flags[0] |
	     SW_FLAG0_STALL_ERROR);
	}
      if (mchk_logout->lep_lber & LBER_CNFE)
	{
	  sw_flags->error_flags[0] =
	    (sw_flags->error_flags[0] |
	     SW_FLAG0_CNF_ERROR);
	}
      if (mchk_logout->lep_lber & LBER_CAE)
	{
	  sw_flags->error_flags[0] =
	    (sw_flags->error_flags[0] |
	     SW_FLAG0_CA_ERROR);
	}
    }/* 8.1.49 */
  
  if (mchk_logout->lep_lber & LBER_NXAE)
    {/* 8.1.50 */
      PRINT_PARSE((" 8.1.50 "));
      
      lsb_data_fetch_error_found++;
      sw_flags->error_flags[0] =
	(sw_flags->error_flags[0] |
	 SW_FLAG0_NXM_MEM_READ |
	 SW_FLAG0_LSB_ERR_READ |
	 SW_FLAG0_EV4_WRITEBLOCK);
      sw_flags->error_flags[1] = sw_flags->error_flags[1] |
	SW_FLAG1_LMA_PRESENT | SW_FLAG1_LSB_PRESENT;
      
    }/* 8.1.50 */
  
  if (mchk_logout->lep_lber & LBER_CPE2)
    {/* 8.1.34 */
      PRINT_PARSE((" 8.1.34 "));
      
      sw_flags->error_flags[0] =
	(sw_flags->error_flags[0] |
	 SW_FLAG0_670_CPE2);
    }
  if (mchk_logout->lep_lber & LBER_CPE)
    {/* 8.1.51 */
      PRINT_PARSE((" 8.1.51 "));
      
      lsb_data_fetch_error_found++;
      
      sw_flags->error_flags[0] =
	(sw_flags->error_flags[0] |
	 SW_FLAG0_670_CPE |
	 SW_FLAG0_LSB_ERR_READ |
	 SW_FLAG0_EV4_WRITEBLOCK);
      
      sw_flags->error_flags[1] = sw_flags->error_flags[1] |
	SW_FLAG1_LSB_PRESENT;
    }
  if (mchk_logout->lep_lber & LBER_CTCE)
    {/* 8.1.52 */
      PRINT_PARSE((" 8.1.52 "));
      lsb_data_fetch_error_found++;
      
      sw_flags->error_flags[0] =
	(sw_flags->error_flags[0] |
	 SW_FLAG0_CTCE |
	 SW_FLAG0_LSB_ERR_READ |
	 SW_FLAG0_EV4_WRITEBLOCK);
      
      sw_flags->error_flags[1] = sw_flags->error_flags[1] |
	SW_FLAG1_LSB_PRESENT;
    }
  if (mchk_logout->lep_lber & LBER_UCE2)
    {/* 8.1.37 */
      PRINT_PARSE((" 8.1.37 "));
      
      sw_flags->error_flags[0] =
	(sw_flags->error_flags[0] |
	 SW_FLAG0_LSB_UCE2);
    }
  if (mchk_logout->lep_lber & LBER_CE2)
    {/* 8.1.39 */
      PRINT_PARSE((" 8.1.39 "));
      
      sw_flags->error_flags[1] =
	(sw_flags->error_flags[1] |
	 SW_FLAG1_LSB_CE2);
    }
  if ( lsb_data_fetch_error_found == 0 )
    {/* 8.1.53 */
      PRINT_PARSE((" 8.1.53 "));
      sw_flags->error_flags[0] =
	(sw_flags->error_flags[0] |
	 SW_FLAG0_EV4_WRITEBLOCK);
      
      sw_flags->error_flags[1] =
	(sw_flags->error_flags[1] |
	 SW_FLAG1_670_INCONSISTENT_2 |
	 SW_FLAG1_LSB_PRESENT);
    }
}/* LSB Read Error */



int ruby_670_herr_write_block(mchk_logout, sw_flags)
     struct ruby_mchk_logout *mchk_logout;
				/* address of the machine check frame */
				/* to use for parsing */
     struct elr_soft_flags *sw_flags;
				/* address of the software flags, */
				/* which are used to return parse */
				/* results and as part of the defined */
				/* error packet */
{/* biu_stat.herr && biu_cmd=writeblock */
  
  PARSE_TRACE(("ruby_670_herr_write_block"));

  if (/* There was a Node Specific Error and...*/
				/* LSB transaction was a READ and... */
				/* this CPU was the bus commander */
      (mchk_logout->lep_lber & LBER_NSES) &&
      ( (mchk_logout->lep_lbecr1 & LBECR1_CA_CMD ) == LBECR1_CA_READ) &&
      (((mchk_logout->lep_lbecr1 & LBECR1_CID) >> LBECR1_CID_SHIFT) ==
       mfpr_whami()))
    {/* LSB Read */
      
      if (mchk_logout->lep_lmerr & LMERR_ARBDROP)
	{/* 8.1.42 */
	  PRINT_PARSE((" 8.1.42 "));
	  sw_flags->error_flags[0] =
	    (sw_flags->error_flags[0] |
	     SW_FLAG0_670_ARBDROP |
	     SW_FLAG0_CPU_NSES |
	     SW_FLAG0_LSB_ERR_READ |
	     SW_FLAG0_EV4_WRITEBLOCK);
	  sw_flags->error_flags[1] = sw_flags->error_flags[1] | 
	    SW_FLAG1_LSB_PRESENT;
	}
      else if (mchk_logout->lep_lmerr & LMERR_ARBCOL)
	{/* 8.1.43 */
	  PRINT_PARSE((" 8.1.43 "));
	  sw_flags->error_flags[0] =
	    (sw_flags->error_flags[0] |
	     SW_FLAG0_670_ARBCOL |
	     SW_FLAG0_CPU_NSES |
	     SW_FLAG0_LSB_ERR_READ |
	     SW_FLAG0_EV4_WRITEBLOCK);
	  sw_flags->error_flags[1] = sw_flags->error_flags[1] |
	    SW_FLAG1_LSB_PRESENT;
	}
      else
	{/* 8.1.44 */
	  PRINT_PARSE((" 8.1.44 "));
	  sw_flags->error_flags[0] =
	    (sw_flags->error_flags[0] |
	     SW_FLAG0_670_INCONSISTENT_1 |
	     SW_FLAG0_CPU_NSES |
	     SW_FLAG0_LSB_ERR_READ |
	     SW_FLAG0_EV4_WRITEBLOCK);
	  sw_flags->error_flags[1] = sw_flags->error_flags[1] |
	    SW_FLAG1_LSB_PRESENT;
	}
    }/* LSB Read */
  
  else if (/* There was a Node Specific Error and...*/
				/* LSB transaction was a READ and... */
				/* this CPU was the bus commander */
	   (mchk_logout->lep_lber & LBER_NSES) &&
	   ( (mchk_logout->lep_lbecr1 & LBECR1_CA_CMD ) == LBECR1_CA_WRITE) &&
	   (((mchk_logout->lep_lbecr1 & LBECR1_CID) >> LBECR1_CID_SHIFT) ==
	    mfpr_whami()))
    {/* LSB Write */
      if (mchk_logout->lep_lmerr & LMERR_ARBDROP)
	{/* 8.1.45 */
	  PRINT_PARSE((" 8.1.45 "));
	  sw_flags->error_flags[0] =
	    (sw_flags->error_flags[0] |
	     SW_FLAG0_670_ARBDROP |
	     SW_FLAG0_CPU_NSES |
	     SW_FLAG0_LSB_ERR_WRITE |
	     SW_FLAG0_EV4_WRITEBLOCK);
	  
	  sw_flags->error_flags[1] = sw_flags->error_flags[1] |
	    SW_FLAG1_LSB_PRESENT;
	  
	  
	}
      else if (mchk_logout->lep_lmerr & LMERR_ARBCOL)
	{/* 8.1.46 */
	  PRINT_PARSE((" 8.1.46 "));
	  sw_flags->error_flags[0] =
	    (sw_flags->error_flags[0] |
	     SW_FLAG0_670_ARBCOL |
	     SW_FLAG0_CPU_NSES |
	     SW_FLAG0_LSB_ERR_WRITE |
	     SW_FLAG0_EV4_WRITEBLOCK);
	  
	  sw_flags->error_flags[1] = sw_flags->error_flags[1] |
	    SW_FLAG1_LSB_PRESENT;
	  
	  
	}
      else
	{/* 8.1.47 */
	  PRINT_PARSE((" 8.1.47 "));
	  sw_flags->error_flags[0] =
	    (sw_flags->error_flags[0] |
	     SW_FLAG0_670_INCONSISTENT_1 |
	     SW_FLAG0_CPU_NSES |
	     SW_FLAG0_LSB_ERR_WRITE |
	     SW_FLAG0_EV4_WRITEBLOCK);
	  
	  sw_flags->error_flags[1] = sw_flags->error_flags[1] |
	    SW_FLAG1_LSB_PRESENT;
	  
	  
	}
    }/* LSB Write */
  
  else if (/* There was an LSB error and... */
				/* LSB transaction was a read and... */
				/* this CPU was the bus commander */
	   (mchk_logout->lep_lber & LBER_E) &&
	   ( (mchk_logout->lep_lbecr1 & LBECR1_CA_CMD ) == LBECR1_CA_READ) &&
	   ((mchk_logout->lep_lbecr1 & LBECR1_CID) >>
	    LBECR1_CID_SHIFT) == mfpr_whami())
    {/* LSB Read Error */
      ruby_670_parse_lsb_readerr(mchk_logout, sw_flags);
    }
  else if ((mchk_logout->lep_lber & LBER_E) &&
	   ( (mchk_logout->lep_lbecr1 & LBECR1_CA_CMD ) == LBECR1_CA_WRITE) &&
	   (((mchk_logout->lep_lbecr1 & LBECR1_CID) >>
	     LBECR1_CID_SHIFT) == mfpr_whami()))
    {/* bcache contains shared data */ /* select all */
      int bcache_shr_error_found=0;
      
      if (mchk_logout->lep_lber & (LBER_SHE | LBER_DIE))
	{/* 8.1.54 */
	  PRINT_PARSE((" 8.1.54 "));
	  bcache_shr_error_found++;
	  
	  sw_flags->error_flags[0] =
	    (sw_flags->error_flags[0] |
	     SW_FLAG0_EV4_WRITEBLOCK |
	     SW_FLAG0_LSB_ERR_WRITE);
	  
	  sw_flags->error_flags[1] = sw_flags->error_flags[1] |
	    SW_FLAG1_LSB_PRESENT;
	  
	  if (mchk_logout->lep_lber & LBER_SHE)
	    {
	      sw_flags->error_flags[0] =
		(sw_flags->error_flags[0] |
		 SW_FLAG0_SHARED_ERROR);
	    }
	  if (mchk_logout->lep_lber & LBER_DIE);
	  {
	    sw_flags->error_flags[0] =
	      (sw_flags->error_flags[0] |
	       SW_FLAG0_DIRTY_ERROR);
	  }
	}
      if (mchk_logout->lep_lber & (LBER_STE | LBER_CNFE | LBER_CAE ))
	{/* 8.1.55 */
	  PRINT_PARSE((" 8.1.55 "));
	  bcache_shr_error_found++;
	  
	  sw_flags->error_flags[0] =
	    (sw_flags->error_flags[0] |
	     SW_FLAG0_EV4_WRITEBLOCK |
	     SW_FLAG0_LSB_ERR_WRITE);
	  
	  sw_flags->error_flags[1] = sw_flags->error_flags[1] |
	    SW_FLAG1_LSB_PRESENT;
	  
	  if (mchk_logout->lep_lber & LBER_STE)
	    {
	      sw_flags->error_flags[0] =
		(sw_flags->error_flags[0] |
		 SW_FLAG0_STALL_ERROR);
	    }
	  if (mchk_logout->lep_lber & LBER_CNFE)
	    {
	      sw_flags->error_flags[0] =
		(sw_flags->error_flags[0] |
		 SW_FLAG0_CNF_ERROR);
	    }
	  if (mchk_logout->lep_lber & LBER_CAE)
	    {
	      sw_flags->error_flags[0] =
		(sw_flags->error_flags[0] |
		 SW_FLAG0_CA_ERROR);
	    }
	}
      if (mchk_logout->lep_lber & LBER_NXAE)
	{/* 8.1.56 */
	  PRINT_PARSE((" 8.1.56 "));
	  bcache_shr_error_found++;
	  
	  sw_flags->error_flags[0] =
	    (sw_flags->error_flags[0] |
	     SW_FLAG0_EV4_WRITEBLOCK |
	     SW_FLAG0_LSB_ERR_WRITE);
	  
	  sw_flags->error_flags[1] =
	    ( sw_flags->error_flags[1] |
	     SW_FLAG1_NXM_MEM_WRITE |
	     SW_FLAG1_LMA_PRESENT |
	     SW_FLAG1_LSB_PRESENT);
	}
      if (mchk_logout->lep_lber & LBER_CPE2)
	{/* 8.1.34 */
	  PRINT_PARSE((" 8.1.34 "));
	  sw_flags->error_flags[0] =
	    (sw_flags->error_flags[0] |
	     SW_FLAG0_670_CPE2);
	}
      if (mchk_logout->lep_lber & LBER_CPE)
	{/* 8.1.57 */
	  PRINT_PARSE((" 8.1.57 "));
	  bcache_shr_error_found++;
	  
	  sw_flags->error_flags[0] =
	    (sw_flags->error_flags[0] |
	     SW_FLAG0_670_CPE |
	     SW_FLAG0_EV4_WRITEBLOCK |
	     SW_FLAG0_LSB_ERR_WRITE);
	  
	  sw_flags->error_flags[1] =
	    ( sw_flags->error_flags[1] |
	     SW_FLAG1_LSB_PRESENT);
	}
      if (mchk_logout->lep_lber & LBER_CTCE)
	{/* 8.1.58 */
	  PRINT_PARSE((" 8.1.58 "));
	  bcache_shr_error_found++;
	  
	  sw_flags->error_flags[0] =
	    (sw_flags->error_flags[0] |
	     SW_FLAG0_CTCE |
	     SW_FLAG0_EV4_WRITEBLOCK |
	     SW_FLAG0_LSB_ERR_WRITE);
	  
	  sw_flags->error_flags[1] = sw_flags->error_flags[1] |
	    SW_FLAG1_LSB_PRESENT;
	  
	}
      if (mchk_logout->lep_lber & LBER_UCE2)
	{/* 8.1.37 */
	  PRINT_PARSE((" 8.1.37 "));
	  
	  sw_flags->error_flags[0] =
	    (sw_flags->error_flags[0] |
	     SW_FLAG0_LSB_UCE2);
	  
	}
      if (mchk_logout->lep_lber & LBER_CE2)
	{/* 8.1.39 */
	  PRINT_PARSE((" 8.1.39 "));
	  
	  sw_flags->error_flags[1] =
	    (sw_flags->error_flags[1] |
	     SW_FLAG1_LSB_CE2);
	  
	}
      if ( bcache_shr_error_found == 0 )
	{/* 8.1.59 */
	  PRINT_PARSE((" 8.1.59 "));
	  
	  sw_flags->error_flags[0] =
	    (sw_flags->error_flags[0] |
	     SW_FLAG0_EV4_WRITEBLOCK |
	     SW_FLAG0_LSB_ERR_WRITE);
	  
	  /* ***PRM: PFMS doesn't say to log LSB */
	  sw_flags->error_flags[1] =
	    (sw_flags->error_flags[1] |
	     SW_FLAG1_670_INCONSISTENT_2);
	}
    }/* bcache contains shared data */
  
  else if ((mchk_logout->lep_lber & LBER_E) &&
	   ( (mchk_logout->lep_lbecr1 & LBECR1_CA_CMD ) == LBECR1_CA_WRITE_CSR) &&
	   (((mchk_logout->lep_lbecr1 & LBECR1_CID) >>
	     LBECR1_CID_SHIFT) == mfpr_whami()))
    {/* i/o cycle */ /* select one */
      if (mchk_logout->lep_lber & (LBER_SHE | LBER_DIE))
	{/* 8.1.60 */
	  PRINT_PARSE((" 8.1.60 "));
	  
	  sw_flags->error_flags[0] =
	    (sw_flags->error_flags[0] |
	     SW_FLAG0_EV4_WRITEBLOCK |
	     SW_FLAG0_LSB_ERR_WRT_CSR);
	  
	  sw_flags->error_flags[1] = sw_flags->error_flags[1] |
	    SW_FLAG1_LSB_PRESENT;
	  
	  if (mchk_logout->lep_lber & LBER_SHE)
	    {
	      sw_flags->error_flags[0] =
		(sw_flags->error_flags[0] |
		 SW_FLAG0_SHARED_ERROR);
	    }
	  if (mchk_logout->lep_lber & LBER_DIE);
	  {
	    sw_flags->error_flags[0] =
	      (sw_flags->error_flags[0] |
	       SW_FLAG0_DIRTY_ERROR);
	  }
	}
      else if (mchk_logout->lep_lber & (LBER_STE | LBER_CNFE | LBER_CAE ))
	{/* 8.1.61 */
	  PRINT_PARSE((" 8.1.61 "));
	  
	  sw_flags->error_flags[0] |=
	    (SW_FLAG0_EV4_WRITEBLOCK | SW_FLAG0_LSB_ERR_WRT_CSR);
	 
	  sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT;
	  
	  if (mchk_logout->lep_lber & LBER_STE)
	    {
	      sw_flags->error_flags[0] =
		(sw_flags->error_flags[0] |
		 SW_FLAG0_STALL_ERROR);
	    }
	  if (mchk_logout->lep_lber & LBER_CNFE)
	    {
	      sw_flags->error_flags[0] =
		(sw_flags->error_flags[0] |
		 SW_FLAG0_CNF_ERROR);
	    }
	  if (mchk_logout->lep_lber & LBER_CAE)
	    {
	      sw_flags->error_flags[0] =
		(sw_flags->error_flags[0] |
		 SW_FLAG0_CA_ERROR);
	    }
	}
      else if (mchk_logout->lep_lber & LBER_CPE)
	{/* 8.1.62 */
	  PRINT_PARSE((" 8.1.62 "));
	  
	  sw_flags->error_flags[0] =
	    (sw_flags->error_flags[0] |
	     SW_FLAG0_EV4_WRITEBLOCK |
	     SW_FLAG0_670_CPE |
	     SW_FLAG0_LSB_ERR_WRT_CSR);
	  
	  sw_flags->error_flags[1] =
	    ( sw_flags->error_flags[1] |
	     SW_FLAG1_LSB_PRESENT);
	}
      else if (mchk_logout->lep_lber & LBER_CDPE)
	{/* 8.1.63 */
	  PRINT_PARSE((" 8.1.63 "));
	  
	  sw_flags->error_flags[0] =
	    (sw_flags->error_flags[0] |
	     SW_FLAG0_EV4_WRITEBLOCK |
	     SW_FLAG0_LSB_CDPE |
	     SW_FLAG0_LSB_ERR_WRT_CSR);
	  
	  sw_flags->error_flags[1] =
	    ( sw_flags->error_flags[1] |
	     SW_FLAG1_LSB_PRESENT);
	}
      else if (mchk_logout->lep_lber & LBER_NXAE)
	{/* 8.1.64 */
	  PRINT_PARSE((" 8.1.64 "));
	  
	  sw_flags->error_flags[0] =
	    (sw_flags->error_flags[0] |
	     SW_FLAG0_EV4_WRITEBLOCK |
	     SW_FLAG0_LSB_ERR_WRT_CSR);
	  
	  /* ***PRM: What no NXM */
	  sw_flags->error_flags[1] =
	    ( sw_flags->error_flags[1] |
	     SW_FLAG1_LOG_ADAPT_PRES |
	     SW_FLAG1_LMA_PRESENT |
	     SW_FLAG1_LSB_PRESENT);
	}
      else 
	{/* 8.1.65 */
	  PRINT_PARSE((" 8.1.65 "));
	  
	  sw_flags->error_flags[0] =
	    (sw_flags->error_flags[0] |
	     SW_FLAG0_EV4_WRITEBLOCK |
	     SW_FLAG0_LSB_ERR_WRT_CSR);
	  
	  /* ***PRM: PFMS doesn't say to log LSB */
	  sw_flags->error_flags[1] =
	    (sw_flags->error_flags[1] |
	     SW_FLAG1_670_INCONSISTENT_2);
	}
    }/* i/o cycle */
  
  else if (mchk_logout->lep_lber & LBER_E)
    {/* 8.1.66 */
      PRINT_PARSE((" 8.1.66 "));
      
      sw_flags->error_flags[0] =
	(sw_flags->error_flags[0] |
	 SW_FLAG0_EV4_WRITEBLOCK);
      
      sw_flags->error_flags[1] = sw_flags->error_flags[1] |
	SW_FLAG1_PREVIOUS_SYSTEM_ERROR_LATCHED |
	  SW_FLAG1_LSB_PRESENT;
      
    }
}  /* biu_stat.herr && biu_cmd=writeblock */



int ruby_670_herr_ldxl(mchk_logout, sw_flags) 
     struct ruby_mchk_logout *mchk_logout;
				/* address of the machine check frame */
				/* to use for parsing */
     struct elr_soft_flags *sw_flags;
				/* address of the software flags, */
				/* which are used to return parse */
				/* results and as part of the defined */
				/* error packet */
{/* 670 level 1 */
  
  PARSE_TRACE(("ruby_670_herr_ldxl"));

  if ((mchk_logout->biu_stat & BIU_HERR) &&
      ( (mchk_logout->biu_stat & BIU_CMD ) == BIU_CMD_LDxL))
    { /* load_block */ /* select one */
      if (mchk_logout->lep_lber & LBER_NSES)
	{ /* nses */ /* select one */
	  if (mchk_logout->lep_lmerr & LMERR_ARBDROP)
	    {/* 8.1.67 */
	      PRINT_PARSE((" 8.1.67 "));
	      
		    /* ***PRM: No LSB_ERR_READ? */
	      sw_flags->error_flags[0] =
		(sw_flags->error_flags[0] |
		 SW_FLAG0_670_ARBDROP |
		 SW_FLAG0_CPU_NSES |
		 SW_FLAG0_EV4_LOADLOCK);
	      
	      sw_flags->error_flags[1] = sw_flags->error_flags[1] | 
		SW_FLAG1_LSB_PRESENT;
	      
	    }
	  else if (mchk_logout->lep_lmerr & LMERR_ARBCOL)
	    {/* 8.1.68 */
	      PRINT_PARSE((" 8.1.68 "));
	      sw_flags->error_flags[0] =
		(sw_flags->error_flags[0] |
		 SW_FLAG0_670_ARBCOL |
		 SW_FLAG0_CPU_NSES |
		 SW_FLAG0_EV4_LOADLOCK);
	      
	      sw_flags->error_flags[1] = sw_flags->error_flags[1] |
		SW_FLAG1_LSB_PRESENT;
	    }
	  else if (mchk_logout->lep_lmerr & LMERR_BTAGPE)
	    {/* 8.1.69 */
	      PRINT_PARSE((" 8.1.69 "));
	      
	      sw_flags->error_flags[0] =
		(sw_flags->error_flags[0] |
		 SW_FLAG0_670_BTAGPE |
		 SW_FLAG0_EV4_LOADLOCK);
	      
	    }
	  else if (mchk_logout->lep_lmerr & LMERR_BSTATPE)
	    {/* 8.1.70 */
	      PRINT_PARSE((" 8.1.70 "));
	      
	      sw_flags->error_flags[0] =
		(sw_flags->error_flags[0] |
		 SW_FLAG0_670_BSTATPE |
		 SW_FLAG0_EV4_LOADLOCK);
	      
	    }
	  else 
	    {/* 8.1.71 */
	      PRINT_PARSE((" 8.1.71 "));
	      
	      sw_flags->error_flags[0] =
		(sw_flags->error_flags[0] |
		 SW_FLAG0_670_INCONSISTENT_1 |
		 SW_FLAG0_CPU_NSES |
		 SW_FLAG0_EV4_LOADLOCK);
	      
	      sw_flags->error_flags[1] =
		(sw_flags->error_flags[1] | 
		 SW_FLAG1_LSB_PRESENT);
	      
	    }
	} /* nses */
      else if ((mchk_logout->lep_lber & LBER_E) &&
	       ( (mchk_logout->lep_lbecr1 & LBECR1_CA_CMD ) == LBECR1_CA_READ) &&
	       (((mchk_logout->lep_lbecr1 & LBECR1_CID) >>
		 LBECR1_CID_SHIFT) == mfpr_whami()))
	{ /* getting MEM data */ /* select all */
	  int get_mem_data_error_found=0;
	  
	  if (mchk_logout->lep_lber & (LBER_SHE | LBER_DIE))
	    {/* 8.1.72 */
	      PRINT_PARSE((" 8.1.72 "));
	      get_mem_data_error_found++;
	      
	      sw_flags->error_flags[0] =
		(sw_flags->error_flags[0] |
		 SW_FLAG0_EV4_LOADLOCK | 
		 SW_FLAG0_LSB_ERR_READ);
	      
	      sw_flags->error_flags[1] = sw_flags->error_flags[1] |
		SW_FLAG1_LSB_PRESENT;
	      
	      if (mchk_logout->lep_lber & LBER_SHE)
		{
		  sw_flags->error_flags[0] =
		    (sw_flags->error_flags[0] |
		     SW_FLAG0_SHARED_ERROR);
		}
	      if (mchk_logout->lep_lber & LBER_DIE);
	      {
		sw_flags->error_flags[0] =
		  (sw_flags->error_flags[0] |
		   SW_FLAG0_DIRTY_ERROR);
	      }
	    }
	  if (mchk_logout->lep_lber & (LBER_STE | LBER_CNFE | LBER_CAE ))
	    {/* 8.1.73 */
	      PRINT_PARSE((" 8.1.73 "));
	      get_mem_data_error_found++;
	      
	      sw_flags->error_flags[0] =
		(sw_flags->error_flags[0] |
		 SW_FLAG0_EV4_LOADLOCK | 
		 SW_FLAG0_LSB_ERR_READ);
	      
	      sw_flags->error_flags[1] = sw_flags->error_flags[1] |
		SW_FLAG1_LSB_PRESENT;
	      
	      if (mchk_logout->lep_lber & LBER_STE)
		{
		  sw_flags->error_flags[0] =
		    (sw_flags->error_flags[0] |
		     SW_FLAG0_STALL_ERROR);
		}
	      if (mchk_logout->lep_lber & LBER_CNFE)
		{
		  sw_flags->error_flags[0] =
		    (sw_flags->error_flags[0] |
		     SW_FLAG0_CNF_ERROR);
		}
	      if (mchk_logout->lep_lber & LBER_CAE)
		{
		  sw_flags->error_flags[0] =
		    (sw_flags->error_flags[0] |
		     SW_FLAG0_CA_ERROR);
		}
	    }
	  if (mchk_logout->lep_lber & LBER_NXAE)
	    {/* 8.1.74 */
	      PRINT_PARSE((" 8.1.74 "));
	      get_mem_data_error_found++;
	      
	      
	      sw_flags->error_flags[0] =
		(sw_flags->error_flags[0] |
		 SW_FLAG0_NXM_MEM_READ |
		 SW_FLAG0_EV4_LOADLOCK |
		 SW_FLAG0_LSB_ERR_READ);
	      
	      sw_flags->error_flags[1] =
		( sw_flags->error_flags[1] |
		 SW_FLAG1_LMA_PRESENT |
		 SW_FLAG1_LSB_PRESENT);
	      
	    }
	  if (mchk_logout->lep_lber & LBER_CPE2)
	    {/* 8.1.34 */
	      PRINT_PARSE((" 8.1.34 "));
	      
	      sw_flags->error_flags[0] =
		(sw_flags->error_flags[0] |
		 SW_FLAG0_670_CPE2);
	      
	    }
	  if (mchk_logout->lep_lber & LBER_CPE)
	    {/* 8.1.75 */
	      PRINT_PARSE((" 8.1.75 "));
	      get_mem_data_error_found++;
	      
	      sw_flags->error_flags[0] =
		(sw_flags->error_flags[0] |
		 SW_FLAG0_670_CPE |
		 SW_FLAG0_EV4_LOADLOCK |
		 SW_FLAG0_LSB_ERR_READ);
	      
	      sw_flags->error_flags[1] =
		( sw_flags->error_flags[1] |
		 SW_FLAG1_LSB_PRESENT);
	      
	    }
	  if (mchk_logout->lep_lber & LBER_CTCE)
	    {/* 8.1.76 */
	      PRINT_PARSE((" 8.1.76 "));
	      get_mem_data_error_found++;
	      
	      sw_flags->error_flags[0] =
		(sw_flags->error_flags[0] |
		 SW_FLAG0_CTCE |
		 SW_FLAG0_EV4_LOADLOCK |
		 SW_FLAG0_LSB_ERR_READ);
	      
	      sw_flags->error_flags[1] =
		( sw_flags->error_flags[1] |
		 SW_FLAG1_LSB_PRESENT);
	      
	    }
	  if (mchk_logout->lep_lber & LBER_UCE2)
	    {/* 8.1.37 */
	      PRINT_PARSE((" 8.1.37 "));
	      
	      sw_flags->error_flags[0] =
		(sw_flags->error_flags[0] |
		 SW_FLAG0_LSB_UCE2);
	      
	    }
	  if (mchk_logout->lep_lber & LBER_CE2)
	    {/* 8.1.39 */
	      PRINT_PARSE((" 8.1.39 "));
	      
	      sw_flags->error_flags[1] =
		(sw_flags->error_flags[1] |
		 SW_FLAG1_LSB_CE2);
	      
	    }
	  if (get_mem_data_error_found == 0)
	    {/* 8.1.77 */
	      PRINT_PARSE((" 8.1.77 "));
	      
	      
	      sw_flags->error_flags[0] =
		(sw_flags->error_flags[0] |
		 SW_FLAG0_EV4_LOADLOCK |
		 SW_FLAG0_LSB_ERR_READ);
	      
	      sw_flags->error_flags[1] =
		( sw_flags->error_flags[1] |
		 SW_FLAG1_670_INCONSISTENT_2);
	      
	    }
	} /* getting MEM data */
      else if (mchk_logout->lep_lber & LBER_E) 
	{/* 8.1.78 */
	  PRINT_PARSE((" 8.1.78 "));
	  
	  sw_flags->error_flags[0] =
	    (sw_flags->error_flags[0] |
	     SW_FLAG0_EV4_LOADLOCK);
	  
	  sw_flags->error_flags[1] =
	    ( sw_flags->error_flags[1] |
	     SW_FLAG1_PREVIOUS_SYSTEM_ERROR_LATCHED |
	     SW_FLAG1_LSB_PRESENT);
	  
	}
    } /* load_block */
}/* 670 level 1 */





int ruby_670_herr_stxc(mchk_logout, sw_flags) /* 670 level 1 */
     struct ruby_mchk_logout *mchk_logout;
				/* address of the machine check frame */
				/* to use for parsing */
     struct elr_soft_flags *sw_flags;
				/* address of the software flags, */
				/* which are used to return parse */
				/* results and as part of the defined */
				/* error packet */
{/* biu_stat.herr && biu_cmd=STxC */
  
  PARSE_TRACE(("ruby_670_herr_stxc"));

  if ((mchk_logout->biu_stat & BIU_HERR) &&
      ( (mchk_logout->biu_stat & BIU_CMD ) == BIU_CMD_STxC))
    { /* store_cond */ /* select one */
      
      if (mchk_logout->lep_lber & LBER_NSES)
	{ /* nses */ /* select all */
	  int nses_error_found=0;
	  
	  if (mchk_logout->lep_lmerr & LMERR_ARBDROP)
	    {/* 8.1.79 */
	      PRINT_PARSE((" 8.1.79 "));
	      nses_error_found++;
	      
	      sw_flags->error_flags[0] =
		(sw_flags->error_flags[0] |
		 SW_FLAG0_670_ARBDROP |
		 SW_FLAG0_CPU_NSES |
		 SW_FLAG0_EV4_STORECOND);
	      
	      sw_flags->error_flags[1] = sw_flags->error_flags[1] | 
		SW_FLAG1_LSB_PRESENT;
	      
	    }
	  if (mchk_logout->lep_lmerr & LMERR_ARBCOL)
	    {/* 8.1.80 */
	      PRINT_PARSE((" 8.1.80 "));
	      nses_error_found++;
	      
	      sw_flags->error_flags[0] =
		(sw_flags->error_flags[0] |
		 SW_FLAG0_670_ARBCOL |
		 SW_FLAG0_CPU_NSES |
		 SW_FLAG0_EV4_STORECOND);
	      
	      sw_flags->error_flags[1] = sw_flags->error_flags[1] | 
		SW_FLAG1_LSB_PRESENT;
	      
	    }
	  if (mchk_logout->lep_lmerr & LMERR_BTAGPE)
	    {/* 8.1.81 */
	      PRINT_PARSE((" 8.1.81 "));
	      nses_error_found++;
	      
	      sw_flags->error_flags[0] =
		(sw_flags->error_flags[0] |
		 SW_FLAG0_670_BTAGPE |
		 SW_FLAG0_EV4_STORECOND);
	      
	    }
	  if (mchk_logout->lep_lmerr & LMERR_BSTATPE)
	    {/* 8.1.82 */
	      PRINT_PARSE((" 8.1.82 "));
	      nses_error_found++;
	      
	      sw_flags->error_flags[0] =
		(sw_flags->error_flags[0] |
		 SW_FLAG0_670_BSTATPE |
		 SW_FLAG0_EV4_STORECOND);
	      
	    }
	  
	  if ( nses_error_found == 0 )
	    {/* 8.1.83 */
	      PRINT_PARSE((" 8.1.83 "));
	      
	      sw_flags->error_flags[0] =
		(sw_flags->error_flags[0] |
		 SW_FLAG0_670_INCONSISTENT_1 |
		 SW_FLAG0_CPU_NSES |
		 SW_FLAG0_EV4_STORECOND);
	      
	      sw_flags->error_flags[1] = sw_flags->error_flags[1] | 
		SW_FLAG1_LSB_PRESENT;
	      
	    }
	} /* nses */
      else if ((mchk_logout->lep_lber & LBER_E) &&
	       ( (mchk_logout->lep_lbecr1 & LBECR1_CA_CMD ) == LBECR1_CA_WRITE) &&
	       (((mchk_logout->lep_lbecr1 & LBECR1_CID) >>
		 LBECR1_CID_SHIFT) == mfpr_whami()))
	{ /* CPU WRITE */ /* select all */
	  int lsb_data_write_error_found=0;
	  
	  if (mchk_logout->lep_lber & (LBER_SHE | LBER_DIE))
	    {/* 8.1.84 */
	      PRINT_PARSE((" 8.1.84 "));
	      lsb_data_write_error_found++;
	      
	      sw_flags->error_flags[0] =
		(sw_flags->error_flags[0] |
		 SW_FLAG0_EV4_STORECOND |
		 SW_FLAG0_LSB_ERR_WRITE);
	      
	      sw_flags->error_flags[1] = sw_flags->error_flags[1] |
		SW_FLAG1_LSB_PRESENT;
	      
	      if (mchk_logout->lep_lber & LBER_SHE)
		{
		  sw_flags->error_flags[0] =
		    (sw_flags->error_flags[0] |
		     SW_FLAG0_SHARED_ERROR);
		}
	      if (mchk_logout->lep_lber & LBER_DIE);
	      {
		sw_flags->error_flags[0] =
		  (sw_flags->error_flags[0] |
		   SW_FLAG0_DIRTY_ERROR);
	      }
	    }
	  if (mchk_logout->lep_lber & (LBER_STE | LBER_CNFE | LBER_CAE ))
	    {/* 8.1.85 */
	      PRINT_PARSE((" 8.1.85 "));
	      lsb_data_write_error_found++;
	      
	      sw_flags->error_flags[0] =
		(sw_flags->error_flags[0] |
		 SW_FLAG0_EV4_STORECOND |
		 SW_FLAG0_LSB_ERR_WRITE);
	      
	      sw_flags->error_flags[1] = sw_flags->error_flags[1] |
		SW_FLAG1_LSB_PRESENT;
	      
	      if (mchk_logout->lep_lber & LBER_STE)
		{
		  sw_flags->error_flags[0] =
		    (sw_flags->error_flags[0] |
		     SW_FLAG0_STALL_ERROR);
		}
	      if (mchk_logout->lep_lber & LBER_CNFE)
		{
		  sw_flags->error_flags[0] =
		    (sw_flags->error_flags[0] |
		     SW_FLAG0_CNF_ERROR);
		}
	      if (mchk_logout->lep_lber & LBER_CAE)
		{
		  sw_flags->error_flags[0] =
		    (sw_flags->error_flags[0] |
		     SW_FLAG0_CA_ERROR);
		}
	    }
	  if (mchk_logout->lep_lber & LBER_NXAE)
	    {/* 8.1.86 */
	      PRINT_PARSE((" 8.1.86 "));
	      lsb_data_write_error_found++;
	      
	      sw_flags->error_flags[0] =
		(sw_flags->error_flags[0] |
		 SW_FLAG0_EV4_STORECOND |
		 SW_FLAG0_LSB_ERR_WRITE);
	      
	      sw_flags->error_flags[1] = sw_flags->error_flags[1] |
		SW_FLAG1_NXM_MEM_WRITE |
		  SW_FLAG1_LMA_PRESENT |
		    SW_FLAG1_LSB_PRESENT;
	      
	    }
	  if (mchk_logout->lep_lber & LBER_CPE2)
	    {/* 8.1.34 */
	      PRINT_PARSE((" 8.1.34 "));
	      sw_flags->error_flags[0] =
		(sw_flags->error_flags[0] |
		 SW_FLAG0_670_CPE2);
	    }
	  if (mchk_logout->lep_lber & LBER_CPE)
	    {/* 8.1.87 */
	      PRINT_PARSE((" 8.1.87 "));
	      lsb_data_write_error_found++;
	      
	      sw_flags->error_flags[0] =
		(sw_flags->error_flags[0] |
		 SW_FLAG0_670_CPE |
		 SW_FLAG0_EV4_STORECOND |
		 SW_FLAG0_LSB_ERR_WRITE);
	      
	      sw_flags->error_flags[1] = sw_flags->error_flags[1] |
		SW_FLAG1_LSB_PRESENT;
	      
	    }
	  if (mchk_logout->lep_lber & LBER_CTCE)
	    {/* 8.1.88 */
	      PRINT_PARSE((" 8.1.88 "));
	      lsb_data_write_error_found++;
	      
	      sw_flags->error_flags[0] =
		(sw_flags->error_flags[0] |
		 SW_FLAG0_CTCE |
		 SW_FLAG0_EV4_STORECOND |
		 SW_FLAG0_LSB_ERR_WRITE);
	      
	      sw_flags->error_flags[1] = sw_flags->error_flags[1] |
		SW_FLAG1_LSB_PRESENT;
	      
	    }
	  if (mchk_logout->lep_lber & LBER_UCE2)
	    {/* 8.1.37 */
	      PRINT_PARSE((" 8.1.37 "));
	      
	      sw_flags->error_flags[0] =
		(sw_flags->error_flags[0] |
		 SW_FLAG0_LSB_UCE2);
	      
	    }
	  if (mchk_logout->lep_lber & LBER_CE2)
	    {/* 8.1.39 */
	      PRINT_PARSE((" 8.1.39 "));
	      
	      sw_flags->error_flags[1] =
		(sw_flags->error_flags[1] |
		 SW_FLAG1_LSB_CE2);
	      
	    }
	  if (lsb_data_write_error_found == 0)
	    {/* 8.1.89 */
	      PRINT_PARSE((" 8.1.89 "));
	      
	      sw_flags->error_flags[0] =
		(sw_flags->error_flags[0] |
		 SW_FLAG0_EV4_STORECOND |
		 SW_FLAG0_LSB_ERR_WRITE);
	      
	      sw_flags->error_flags[1] = sw_flags->error_flags[1] |
		SW_FLAG1_670_INCONSISTENT_2 |
		  SW_FLAG1_LSB_PRESENT;
	      
	    }
	  
	} /* CPU WRITE */
      else if (mchk_logout->lep_lber & LBER_E) 
	{/* 8.1.90 */
	  PRINT_PARSE((" 8.1.90 "));
	  
	  sw_flags->error_flags[0] =
	    (sw_flags->error_flags[0] |
	     SW_FLAG0_EV4_STORECOND);
	  
	  sw_flags->error_flags[1] = sw_flags->error_flags[1] |
	    SW_FLAG1_PREVIOUS_SYSTEM_ERROR_LATCHED |
	      SW_FLAG1_LSB_PRESENT;
	  
	}
      else 
	{ /* 8.1.?? ***PRM: none of the above conditions have been meet */
				/* I made this one up... */
	  PRINT_PARSE((" 8.1.?"));
	  
	  sw_flags->error_flags[0] =
	    (sw_flags->error_flags[0] |
	     SW_FLAG0_670_INCONSISTENT_1 |
	     SW_FLAG0_EV4_STORECOND);
	  
	  sw_flags->error_flags[1] = sw_flags->error_flags[1] |
	    SW_FLAG1_670_INCONSISTENT_2 |
	      SW_FLAG1_LSB_PRESENT;
	  
	}
    } /* store_cond */
}



int ruby_670_biu_seo(mchk_logout, sw_flags)
     struct ruby_mchk_logout *mchk_logout;
				/* address of the machine check frame */
				/* to use for parsing */
     struct elr_soft_flags *sw_flags;
				/* address of the software flags, */
				/* which are used to return parse */
				/* results and as part of the defined */
				/* error packet */
{/* 8.1.1 */
  PRINT_PARSE((" 8.1.1 "));
				/* don't set found error, since this only indicates */
				/* multiple, and not the initial error. */
  
  sw_flags->error_flags[1] = sw_flags->error_flags[1] | SW_FLAG1_BIU_SEO;
  ERROR_CRASH;
}



int ruby_670_fill_seo(mchk_logout, sw_flags) /* 670 level 1 */
     struct ruby_mchk_logout *mchk_logout;
				/* address of the machine check frame */
				/* to use for parsing */
     struct elr_soft_flags *sw_flags;
				/* address of the software flags, */
				/* which are used to return parse */
				/* results and as part of the defined */
				/* error packet */
{/* 8.1.2 */
  PRINT_PARSE((" 8.1.2 "));
  
				/* ***PRM: Want to check revision of EV4 to differenciate */
				/* meaning of bit. DC_STAT = 000 => ev2.1; */
				/* DC_STAT = 111 => ev3.0 */ /* where can I get */
				/* revision of chip? */
  
				/* don't set found error, since this only indicates */
				/* multiple, and not the initial error. */
  
  sw_flags->error_flags[1] = sw_flags->error_flags[1] | SW_FLAG1_CACHE_FILL_SEO;
  ERROR_CRASH;
}



int ruby_670_bc_tperr(mchk_logout, sw_flags) /* 670 level 1 */
     struct ruby_mchk_logout *mchk_logout;
				/* address of the machine check frame */
				/* to use for parsing */
     struct elr_soft_flags *sw_flags;
				/* address of the software flags, */
				/* which are used to return parse */
				/* results and as part of the defined */
				/* error packet */
{
  int found_tperr_error=0;

  PARSE_TRACE(("ruby_670_bc_tperr"));
  
				/* Select one */
  if ( (mchk_logout->biu_stat & BIU_CMD ) == BIU_CMD_READ_BLOCK) 
    {/* 8.1.3 */
      PRINT_PARSE((" 8.1.3 "));
      found_tperr_error++;

      sw_flags->error_flags[0] = sw_flags->error_flags[0] | SW_FLAG0_READ_BTAG_APE;
				/* basic mcheck entry */
				/* log biu_stat to eeprom */
				/* log biu_addr to eeprom */
	ERROR_CRASH;
    }
  else if ( (mchk_logout->biu_stat & BIU_CMD ) == BIU_CMD_WRITE_BLOCK) 
    {/* 8.1.4 */
      PRINT_PARSE((" 8.1.4 "));
      found_tperr_error++;
      sw_flags->error_flags[0] = sw_flags->error_flags[0] | SW_FLAG0_WRITE_BTAG_APE;
				/* basic mcheck entry */
				/* log biu_stat to eeprom */
				/* log biu_addr to eeprom */
	ERROR_CRASH;
    }
  
  if (found_tperr_error == 0 )
    {/* 8.1.5 */
      PRINT_PARSE((" 8.1.5 "));
      sw_flags->error_flags[0] = sw_flags->error_flags[0] | SW_FLAG0_BIU_INCON;
				/* basic mcheck entry */
	ERROR_CRASH;
    }
}


int ruby_670_bc_tcperr(mchk_logout, sw_flags) /* 670 level 1 */
     struct ruby_mchk_logout *mchk_logout;
				/* address of the machine check frame */
				/* to use for parsing */
     struct elr_soft_flags *sw_flags;
				/* address of the software flags, */
				/* which are used to return parse */
				/* results and as part of the defined */
				/* error packet */
{
  int found_tcperr_error=0;

  PARSE_TRACE(("ruby_670_bc_tcperr"));

				/* Select one */
  if ( (mchk_logout->biu_stat & BIU_CMD ) == BIU_CMD_READ_BLOCK) 
  {/* 8.1.6 */
    PRINT_PARSE((" 8.1.6 "));
      found_tcperr_error++;
      sw_flags->error_flags[0] = sw_flags->error_flags[0] | SW_FLAG0_READ_BTAG_CTRL_PE;
				/* basic mcheck entry */
				/* log biu_stat to eeprom */
				/* log biu_addr to eeprom */
	ERROR_CRASH;
    }
  else if ( (mchk_logout->biu_stat & BIU_CMD ) == BIU_CMD_WRITE_BLOCK) 
    {/* 8.1.7 */
      PRINT_PARSE((" 8.1.7 "));
      found_tcperr_error++;
      sw_flags->error_flags[0] = sw_flags->error_flags[0] | SW_FLAG0_WRITE_BTAG_CTRL_PE;
				/* basic mcheck entry */
				/* log biu_stat to eeprom */
				/* log biu_addr to eeprom */
	ERROR_CRASH;
    }
  
  if (found_tcperr_error == 0)
    {/* 8.1.5 */
      PRINT_PARSE((" 8.1.5 "));
      sw_flags->error_flags[0] = sw_flags->error_flags[0] | SW_FLAG0_BIU_INCON;
				/* basic mcheck entry */
	ERROR_CRASH;
    }
}

int ruby_670_fill_ecc(mchk_logout, sw_flags) /* 670 level 1 */
     struct ruby_mchk_logout *mchk_logout;
				/* address of the machine check frame */
				/* to use for parsing */
     struct elr_soft_flags *sw_flags;
				/* address of the software flags, */
				/* which are used to return parse */
				/* results and as part of the defined */
				/* error packet */
{/* BIU_STAT.FILL_ECC */
				/* select one */

  PARSE_TRACE(("ruby_670_fill_ecc"));

  if (mchk_logout->biu_stat & BIU_FILL_IRD)
    {
      if (mchk_logout->bc_tag & 1)
	{ /* low bit is hit bit */
				/*			if (mchk_logout->hirr & HIRR_CRR) */
	  if (mchk_logout->hirr & (1<<4))
	    {/* 8.1.8 */
	      PRINT_PARSE((" 8.1.8 "));
	      sw_flags->error_flags[0] = sw_flags->error_flags[0] | SW_FLAG0_ISTRM_BCACHE_SBE;
				/* basic mcheck entry */
	       			/* This should be a 630 on EV4 rev 3 */

	      /* if (mchk_logout->biu_stat & BIU_FILL_CRD) bit 9, distinguishing ev4.3=1 from ev4.2=0 */
	      if (mchk_logout->biu_stat & (1<<9))
		{
		  ERROR_RECOVER;
		} else {		
		  ERROR_CRASH;
		}		
	    }
	  else
	    {/* 8.1.9 */
	      PRINT_PARSE((" 8.1.9 "));
	      sw_flags->error_flags[0] = sw_flags->error_flags[0] | SW_FLAG0_ISTRM_BCACHE_DBE;
				/* basic mcheck entry */
		  ERROR_CRASH;
	    }
	} else {
				/*				if (mchk_logout->hirr & HIRR_CRR) */
	  if (mchk_logout->hirr & (1<<4))
	    {
	      if (mchk_logout->lep_lber & LBER_CE)
		{
		  if (mchk_logout->lep_lbecr1 & LBECR1_DIRTY)
		    {/* 8.1.11 */
		      PRINT_PARSE((" 8.1.11 "));
		      sw_flags->error_flags[0] = sw_flags->error_flags[0] | SW_FLAG0_ISTRM_RD_OTHER_CPU_BCACHE_SBE;
				/* basic mcheck entry */
				/* lsb subpacket */
		      sw_flags->error_flags[1] =
			(sw_flags->error_flags[1] |
			 SW_FLAG1_LSB_PRESENT); 

		      /* PFMS Says this should be a 660 */

		      /* if (mchk_logout->biu_stat & BIU_FILL_CRD) bit 9, distinguishing ev4.3=1 from ev4.2=0 */
		      if (mchk_logout->biu_stat & (1<<9))
			{
			  ERROR_RECOVER;
			} else {		
			  ERROR_CRASH;
			}		
		    }
		  else
		    {/* 8.1.10 */
		      PRINT_PARSE((" 8.1.10 "));
		      sw_flags->error_flags[0] = sw_flags->error_flags[0] | SW_FLAG0_LSB_CE;
				/* basic mcheck entry */
				/* lsb subpacket */
				/* lma subpacket */
		      sw_flags->error_flags[1] =
			(sw_flags->error_flags[1] |
			 SW_FLAG1_LSB_PRESENT |
			 SW_FLAG1_LMA_PRESENT);

		      /* PFMS says this should be 660 */

		      /* if (mchk_logout->biu_stat & BIU_FILL_CRD) bit 9, distinguishing ev4.3=1 from ev4.2=0 */
		      if (mchk_logout->biu_stat & (1<<9))
			{
			  ERROR_RECOVER;
			} else {		
			  ERROR_CRASH;
			}		
		    }
		}
	      else
		{/* 8.1.12 */
		  PRINT_PARSE((" 8.1.12 "));
		  sw_flags->error_flags[0] = sw_flags->error_flags[0] | SW_FLAG0_ISTRM_READ_EDAL_SBE;
				/* basic mcheck entry */

		  /* PFMS says this should be 630 */

		  /* if (mchk_logout->biu_stat & BIU_FILL_CRD) bit 9, distinguishing ev4.3=1 from ev4.2=0 */
		  if (mchk_logout->biu_stat & (1<<9))
		    {
		      ERROR_RECOVER;
		    } else {		
		      ERROR_CRASH;
		    }		
		}
	    }
	  else if (mchk_logout->lep_lber & LBER_UCE)
	    {/* 8.1.13/14/15 */
	      PRINT_PARSE((" 8.1.13/14/15 "));
				/* Scan bus for one: mera.ucer, */
				/* other CPU lmerr.dbatadbe, or */
				/* neither */

	      /* Don't scan for fear of causing double machine check.   */
	      /* Set all possible error bits and leave determination to */
	      /* uerf data interpretation. The LSB subpacket will	*/
	      /* indicate sick node.					*/

	      sw_flags->error_flags[0] |= (7<<12);

	      /* basic mcheck entry */
	      /* lsb subpacket */
	      /* lma subpacket */

	      sw_flags->error_flags[1] =
		      (sw_flags->error_flags[1] |
		       SW_FLAG1_LSB_PRESENT |
		       SW_FLAG1_LMA_PRESENT);
	      
	      ERROR_CRASH;
	    }
	  else
	    {/* 8.1.16 */
	      PRINT_PARSE((" 8.1.16 "));
	      sw_flags->error_flags[0] = sw_flags->error_flags[0] | SW_FLAG0_ISTRM_RD_EDAL_DBE;
				/* basic mcheck entry */
	      ERROR_CRASH;
	    }
	}
    }
  else
    { /* not BIU_FILL_IRD */
      if (mchk_logout->bc_tag & 1) /* BC_HIT */
	{ /* bc_tag.hit */
	  if (mchk_logout->hirr & (1<<4)) /* HIRR_CRR */
	    {/* 8.1.17 */
	      PRINT_PARSE((" 8.1.17 "));
	      sw_flags->error_flags[0] = sw_flags->error_flags[0] | SW_FLAG0_DSTRM_RD_BCACHE_SBE;
				/* basic mcheck entry */

	      /* PFMS says this should be a 630 */

	      /* if (mchk_logout->biu_stat & BIU_FILL_CRD) bit 9, distinguishing ev4.3=1 from ev4.2=0 */
	      if (mchk_logout->biu_stat & (1<<9))
		{
		  ERROR_RECOVER;
		} else {		
		  ERROR_CRASH;
		}		
	    }
	  else
	    {/* 8.1.18 */
	      PRINT_PARSE((" 8.1.18 "));
	      sw_flags->error_flags[0] = sw_flags->error_flags[0] | SW_FLAG0_DSTRM_RD_BCACHE_DBE;
				/* basic mcheck entry */
	      ERROR_CRASH;
	    }
	}
      else
        {/* not bc_tag.hit */
	  if (mchk_logout->hirr & (1<<4)) /* HIRR_CRR */
	    { /* HIRR_CRR */
	      if (mchk_logout->lep_lber & LBER_CE)
		{
		  if (mchk_logout->lep_lbecr1 & LBECR1_DIRTY)
		    {/* 8.1.20 */
		      PRINT_PARSE((" 8.1.20 "));
		      sw_flags->error_flags[0] = sw_flags->error_flags[0] | SW_FLAG0_DSTRM_RD_OTHER_CPU_BCACHE_SBE;
				/* basic mcheck entry */
				/* lsb subpacket */
		      sw_flags->error_flags[1] = sw_flags->error_flags[1] | SW_FLAG1_LSB_PRESENT;

		      /* PFMS says this should be 660 */

		      /* if (mchk_logout->biu_stat & BIU_FILL_CRD) bit 9, distinguishing ev4.3=1 from ev4.2=0 */
		      if (mchk_logout->biu_stat & (1<<9))
			{
			  ERROR_RECOVER;
			} else {		
			  ERROR_CRASH;
			}		
		    }
		  else
		    {/* 8.1.19 */
		      PRINT_PARSE((" 8.1.19 "));
		      sw_flags->error_flags[0] = sw_flags->error_flags[0] | SW_FLAG0_DSTRM_RD_LSB_SBE;
				/* basic mcheck entry */
				/* lma subpacket */
		      sw_flags->error_flags[1] = sw_flags->error_flags[1] | SW_FLAG1_LMA_PRESENT;

		      /* PFMS says this should be 660 */

		      /* if (mchk_logout->biu_stat & BIU_FILL_CRD) bit 9, distinguishing ev4.3=1 from ev4.2=0 */
		      if (mchk_logout->biu_stat & (1<<9))
			{
			  ERROR_RECOVER;
			} else {		
			  ERROR_CRASH;
			}		
		    }
		}
	      else
		{/* 8.1.21 */
		  PRINT_PARSE((" 8.1.21 "));
		  sw_flags->error_flags[0] = sw_flags->error_flags[0] | SW_FLAG0_DSTRM_RD_EDAL_SBE;
				/* basic mcheck entry */

		      /* PFMS says this should be 630 */

		      /* if (mchk_logout->biu_stat & BIU_FILL_CRD) bit 9, distinguishing ev4.3=1 from ev4.2=0 */
		      if (mchk_logout->biu_stat & (1<<9))
			{
			  ERROR_RECOVER;
			} else {		
			  ERROR_CRASH;
			}		
		}
	    } /* HIRR_CRR */
	  else if (mchk_logout->lep_lber & LBER_UCE)
	    {/* 8.1.22/23/24 */
	      PRINT_PARSE((" 8.1.22/23/24 "));
				/* Scan bus for one: mera.ucer, */
				/* other CPU lmerr.dbatadbe, or */
				/* neither */
	      PRINT_PARSE(("Incomplete Parse Path"));

	      /* Don't scan for fear of causing double machine check.   */
	      /* Set all possible error bits and leave determination to */
	      /* uerf data interpretation. The LSB subpacket will	*/
	      /* indicate sick node.					*/

	      sw_flags->error_flags[0] |= (7<<22);

	      /* basic mcheck entry */
	      /* lsb subpacket */
	      /* lma subpacket */

	      sw_flags->error_flags[1] =
		      (sw_flags->error_flags[1] |
		       SW_FLAG1_LSB_PRESENT |
		       SW_FLAG1_LMA_PRESENT);
	      
	      ERROR_CRASH;
	    }
	  else
	    {/* 8.1.25 */
	      PRINT_PARSE((" 8.1.25 "));
	      sw_flags->error_flags[0] = sw_flags->error_flags[0] | SW_FLAG0_DSTRM_RD_EDAL_DBE;
				/* basic mcheck entry */
	      ERROR_CRASH;
	    }
	} /* not bc_tag.hit */
    } /* not BIU_FILL_IRD */
  
} /* BIU_STAT.FILL_ECC */


int ruby_660_fill_ecc(mchk_logout, sw_flags) /* 660 level 1 */
     struct ruby_mchk_logout *mchk_logout;
				/* address of the machine check frame */
				/* to use for parsing */
     struct elr_soft_flags *sw_flags;
				/* address of the software flags, */
				/* which are used to return parse */
				/* results and as part of the defined */
				/* error packet */
{/* BIU_STAT.FILL_ECC */
				/* select one */

  PARSE_TRACE(("ruby_660_fill_ecc"));

  if (mchk_logout->biu_stat & BIU_FILL_IRD)
    {
      if (mchk_logout->bc_tag & 1)
	{ /* low bit is hit bit */
	    /* parse tree says... should be 670 or 630??? */
	    /* go process as 670... */
	    ruby_670_fill_ecc(mchk_logout, sw_flags);
	    mcheck_action = CRASH;
	} else {
	    /* if (mchk_logout->biu_stat & BIU_FILL_CRD) bit 9, distinguishing ev4.3=1 from ev4.2=0 */
	    if (mchk_logout->biu_stat & (1<<9))
	    {
		if (mchk_logout->lep_lber & LBER_CE)
		{
		    if (mchk_logout->lep_lber & LBER_DIE) {
			/* 10.1.3 */
			ERROR_RECOVER;
			sw_flags->error_flags[1] =
				(sw_flags->error_flags[1] |
				 SW_FLAG1_LSB_PRESENT);

			sw_flags->error_flags[1] =
				(sw_flags->error_flags[1] | (1<<(69-64)));
		    } else {
			/* 10.1.2 */
			ERROR_RECOVER;
			sw_flags->error_flags[1] =
				(sw_flags->error_flags[1] |
				 SW_FLAG1_LMA_PRESENT |
				 SW_FLAG1_LSB_PRESENT);

			sw_flags->error_flags[1] =
				(sw_flags->error_flags[1] | (1<<(68-64)));
		    }
		} else {
		    /* 10.1.4 */
		    sw_flags->error_flags[0] = sw_flags->error_flags[0] | SW_FLAG0_ISTRM_READ_EDAL_SBE;
		    ERROR_DISMISS;
		}
	    } else {
		/* 10.1.1 */
		sw_flags->error_flags[1] =
			(sw_flags->error_flags[1] | (1L<<(67-64)));
		ERROR_CRASH;
	    }
	}
    }
  else
    { /* not BIU_FILL_IRD */
	if (mchk_logout->bc_tag & 1) /* BC_HIT */
	{ /* bc_tag.hit */
	    if (mchk_logout->hirr & (1<<4)) /* HIRR_CRR */
		    ruby_670_fill_ecc(mchk_logout, sw_flags);
	    ERROR_CRASH;
	} else {
	    /* if (mchk_logout->biu_stat & BIU_FILL_CRD) bit 9, distinguishing ev4.3=1 from ev4.2=0 */
	    if (mchk_logout->biu_stat & (1<<9))
	    {
		if (mchk_logout->lep_lber & LBER_CE)
		{
		    if (mchk_logout->lep_lber & LBER_DIE) {
			/* 10.1.7 */
			ERROR_RECOVER;
			sw_flags->error_flags[1] =
				(sw_flags->error_flags[1] |
				 SW_FLAG1_LSB_PRESENT);

			sw_flags->error_flags[1] =
				(sw_flags->error_flags[1] | (1<<(79-64)));
		    } else {
			/* 10.1.6 */
			ERROR_RECOVER;
			sw_flags->error_flags[1] =
				(sw_flags->error_flags[1] |
				 SW_FLAG1_LMA_PRESENT);
			sw_flags->error_flags[1] =
				(sw_flags->error_flags[1] | (1<<(78-64)));
		    }
		} else {
		    /* 10.1.8 */
		    sw_flags->error_flags[0] = sw_flags->error_flags[0] | SW_FLAG0_DSTRM_RD_EDAL_SBE;
		    ERROR_DISMISS;
		}
	    } else {
		/* 10.1.5 */
		sw_flags->error_flags[1] =
			(sw_flags->error_flags[1] | (1L<<(77-64)));
		ERROR_CRASH;
	    }
	}	    
    } /* not BIU_FILL_IRD */
  
} /* BIU_STAT.FILL_ECC */



int kn7aa_log_error(mchk_logout, sw_flags, type )
     struct ruby_mchk_logout *mchk_logout;
				/* address of the machine check frame */
				/* to use for parsing */
     struct elr_soft_flags *sw_flags;
				/* address of the software flags, */
				/* which are used to return parse */
				/* results and as part of the defined */
				/* error packet */
     long type;			/* Parameter passed by PAL indicating */
				/* what error type has occurred */

{
  int size_of_frame;
  char *errlog_ptr;
#ifdef NOT_DEFINED
  struct elr_whami *whami_ptr;
#endif
  struct el_rec *elrec;
  int elextyp;
  struct kn7aa_error_counters *error_count_ptr;
  struct ruby_mcheck_control *mctl;
  
  /*
   * Get address for building MCHECK 670 error log data
   */
  mctl = KN7AA_MCHECK_CONTROL(mfpr_whami());

  /*
   * Define error type and setup pointer to error counters
   *
   * Ruby 670        ELCT_EXPTFLT    ELEXTYP_MCK             ELMACH_RUBY
   * Ruby 660        ELCT_EXPTFLT    ELEXTYP_HARD            ELMACH_RUBY
   * Ruby 630        ELCT_EXPTFLT    ELEXTYP_CORR            ELMACH_RUBY
   */

  switch (type)
  { /* type of error being logged */

  case 0x670:
	  elextyp = ELEXTYP_MCK;
	  error_count_ptr = &mctl->error_670_counters;
	  sw_flags->packet_revision = 1;
	  break;
	  
  case 0x660:
	  elextyp = ELEXTYP_HARD;
	  error_count_ptr = &mctl->error_660_counters;
	  sw_flags->packet_revision = 1;
	  break;

  case 0x630:
	  elextyp = ELEXTYP_PROC_CORR;
	  error_count_ptr = &mctl->error_630_counters;
	  sw_flags->packet_revision = 2;
	  break;

  case 0x620:
	  elextyp = ELEXTYP_SYS_CORR;
	  error_count_ptr = &mctl->error_620_counters;
	  sw_flags->packet_revision = 2;
	  break;
  }

  /*
   * Calculate Allocation needed this mchk error log entry
   */ 
  
  size_of_frame = sizeof(struct elr_soft_flags);

  size_of_frame += sizeof(struct elr_lep_common_header);

  if ((type == 0x670) || (type == 0x660)) size_of_frame += sizeof(struct elr_lep_mchk_frame);
  if ((type == 0x630) || (type == 0x620)) size_of_frame += sizeof(struct elr_lep_630_frame);

  size_of_frame += sizeof(struct elr_pal_rev);  

#ifdef NOT_DEFINED
  if ((type == 0x630) || (type == 0x620)) size_of_frame +=
	  sizeof(struct elr_whami);
#endif

  size_of_frame += sizeof(struct elr_error_counters);

  if (sw_flags->error_flags[1] & SW_FLAG1_DLIST_PRESENT)
    {
      size_of_frame += ruby_calc_dlist_size();
    }
  if (sw_flags->error_flags[1] & SW_FLAG1_LSB_PRESENT)
    {
      size_of_frame += ruby_calc_lsb_size();
      DBG_MSG_OFF(("ruby_calc_lsb_size = 0x%l016x", ruby_calc_lsb_size()));
    }
  if (sw_flags->error_flags[1] & SW_FLAG1_LMA_PRESENT)
    {
      size_of_frame += sizeof(struct elr_lma_sub);
    }
  if (sw_flags->error_flags[1] & SW_FLAG1_LOG_ADAPT_PRES)
    {
      size_of_frame += sizeof(struct elr_iop_adp_subp);
    }
  
  
  /*
   * allocate error log frame and setup pointer for filling it.
   */
  
  elrec = (struct el_rec *)binlog_alloc(size_of_frame, 1); 
  DBG_MSG_OFF(("elrec: 0x%l016x", elrec));

  if (elrec == (struct el_rec *)BINLOG_NOBUFAVAIL)
  {
    dprintf("kn7aa: Can't allocate error log buffer/n");
    /* elrec = (struct el_rec *)kalloc(size_of_frame); */
    return(0);
  }

  /*
   * set identifiers in packet for bits to text translation 
   *   parameters - eptr, class, type, ctldev, num, unit, errcode 
   */
  LSUBID(elrec, ELCT_EXPTFLT, elextyp, ELMACH_DEC7000, mfpr_whami(),
	 EL_UNDEF, EL_UNDEF);
  
  errlog_ptr = (char *)&elrec->el_body;

  /*
   * fill error log frame with pieces specific to this error; update
   * errlog_ptr after each fill operation.
   */
  bcopy(sw_flags,errlog_ptr,sizeof(struct elr_soft_flags));
  DBG_MSG(("elr_soft_flags: "));
  ruby_dump_address_range(errlog_ptr, sizeof(struct elr_soft_flags));
  errlog_ptr += sizeof(struct elr_soft_flags);
  
  fill_lep_common_header(errlog_ptr);
  DBG_MSG(("elr_lep_common_header: "));
  ruby_dump_address_range(errlog_ptr, sizeof(struct elr_lep_common_header));
  errlog_ptr += sizeof(struct elr_lep_common_header);
  
  if ((type == 0x670) || (type == 0x660)) {
	  fill_mchk_frame(errlog_ptr, mchk_logout);
	  DBG_MSG(("elr_lep_mchk_frame: "));
	  ruby_dump_address_range(errlog_ptr, sizeof(struct elr_lep_mchk_frame));
	  errlog_ptr += sizeof(struct elr_lep_mchk_frame);
  }
  
  if ((type == 0x630) || (type == 0x620)) {
	  fill_630_frame(errlog_ptr, mchk_logout);
	  DBG_MSG(("elr_lep_mchk_frame: "));
	  ruby_dump_address_range(errlog_ptr, sizeof(struct elr_lep_630_frame));
	  errlog_ptr += sizeof(struct elr_lep_630_frame);
  }
  
  fill_pal_rev(errlog_ptr);
  DBG_MSG(("elr_pal_rev: "));
  ruby_dump_address_range(errlog_ptr, sizeof(struct elr_pal_rev));
  errlog_ptr += sizeof(struct elr_pal_rev);  
  
#ifdef NOT_DEFINED
  /* this requires the PFMS change to include WHAMI in the error log */
  /* packet, which has not been added to ./kernel/dec/binlog/errlog.h */
  /* yet. So remove for the moment */
  if ((type == 0x630) || (type == 0x620)) {
	  whami_ptr = (struct elr_whami *)errlog_ptr;
	  whami_ptr->whami = (long)mfpr_whami();
	  errlog_ptr += sizeof(struct elr_whami);  
  }
#endif

  fill_err_cnts(errlog_ptr, error_count_ptr, sw_flags);
  errlog_ptr += sizeof(struct elr_error_counters);
  
  if (sw_flags->error_flags[1] & SW_FLAG1_DLIST_PRESENT)
    {
      fill_dlist(errlog_ptr);
      DBG_MSG(("dlist: "));
      ruby_dump_address_range(errlog_ptr, ruby_calc_dlist_size());
      errlog_ptr += ruby_calc_dlist_size();
    }
  if (sw_flags->error_flags[1] & SW_FLAG1_LSB_PRESENT)
    {
      fill_lsb(errlog_ptr);
      DBG_MSG(("lsb: "));
      ruby_dump_address_range(errlog_ptr, ruby_calc_lsb_size());
      errlog_ptr += ruby_calc_lsb_size();
    }
  if (sw_flags->error_flags[1] & SW_FLAG1_LMA_PRESENT)
    {
      fill_lma(errlog_ptr, 0);
      DBG_MSG(("elr_lma_sub: "));
      ruby_dump_address_range(errlog_ptr, sizeof(struct elr_lma_sub));
      errlog_ptr += sizeof(struct elr_lma_sub);
    }
  if (sw_flags->error_flags[1] & SW_FLAG1_LOG_ADAPT_PRES)
    {
      fill_adp(errlog_ptr);
      DBG_MSG(("elr_iop_adp_subp: "));
      ruby_dump_address_range(errlog_ptr, sizeof(struct elr_iop_adp_subp));
      errlog_ptr += sizeof(struct elr_iop_adp_subp);
    }
  
  
#ifdef DEBUG
  errlog_ptr = (char *)&elrec->el_body;
  ruby_dump_address_range(errlog_ptr, size_of_frame);
#endif

  /*
   * Log the packet
   */
  binlog_valid(elrec);

  DBG_MSG(("elrec: 0x%l016x", elrec));

}



int ruby_dump_address_range( inp_addr, size)
	int *inp_addr;  /* any valid address */
	int size;	/* in bytes */

	/* No checking is done by this routinte */

{
	int i;
	char inp[80];

#ifdef DEBUG
	printf("Dump Address range: 0x%l016x for: 0x%03x \n", inp_addr, size);
	for(i = 0; i < size/4; )
	{
		if ((i%6) == 0) {
			printf("\n");
			/* printf("%l016x:\n", inp_addr); */
		}
		printf(" %08x ", *inp_addr++);
		i++;
	}
	printf("\n");
	printf("Hit <CR>: ");
	alpha_8530_cons_gets(inp);
	printf("\n");
#endif
	return(0);
}


int ruby_calc_dlist_size()
{
  return(sizeof(struct elr_dec7000_dlist));
}



int ruby_calc_lsb_size()
{
  int subpacket_size;
  int nodes_present;
  int lsb_node_count = 0;
  int node;

  struct bus *lsbbus;
  struct lsbdata *lsbdata;

  subpacket_size = sizeof(struct elr_lsbsnap) - sizeof(struct elr_lsb_req);
    
  /* find out how many LSB nodes there are. */
    
  /*
   * Get bus number for LSB
   */
  lsbbus = get_sys_bus("lsb");
  if (lsbbus == 0)
	  panic("No system bus configured");
      
  /*
   * Get address of lsbdata structure, get_lsb().
   */
  lsbdata = get_lsb(lsbbus->bus_num);

  /*
   * get set of nodes present. presently there is a bug in the lsb config
   * routine, which doesn't recognize cpus. Fix this for packet size
   * calculation.
   */
  nodes_present = (lsbdata->lsbnodes_alive | (1<<mfpr_whami()));

  /*
   * find nodes which are 'alive', lsbdata->lsbnodes_alive & (1<<node).
   * get node's device type, lsbdata-lsberr[lsbnode].plsbsw->lsbldev
   */
  for (node = 0; node < MAX_LSB_NODE; node++)
  {
    if (nodes_present & (1<<node))
    {
      lsb_node_count++;
    }
  }
  
  /* Seven quadwords per node */
  subpacket_size = subpacket_size + (sizeof(struct elr_lsb_req) * lsb_node_count);

  return(subpacket_size);
}



int fill_mchk_frame( mchk_frame_ptr, mchk_logout)
     struct elr_lep_mchk_frame *mchk_frame_ptr;
     struct ruby_mchk_logout *mchk_logout;
				/* address of the machine check frame */
				/* to use for parsing */
{
	int elextyp;

  DBG_MSG(("((short)mchk_logout->retry + 8): 0x%x",((short)mchk_logout->retry + 8)));

  bcopy(&mchk_logout->retry, mchk_frame_ptr, ((short)mchk_logout->retry + 8));

}



int fill_630_frame( lep_630_frame_ptr, lep_630_logout)
     struct elr_lep_630_frame *lep_630_frame_ptr;
     struct elr_lep_630_frame *lep_630_logout;
				/* address of the machine check frame */
				/* to use for parsing */
{
	int elextyp;

  /* ->retry + 8 changed to ->byte_cnt to address QAR 12548 */
  bcopy(&lep_630_logout->byte_cnt, lep_630_frame_ptr, ((short)lep_630_logout->byte_cnt));

}



int fill_pal_rev(pal_rev_ptr)
	struct elr_pal_rev *pal_rev_ptr;
{
	struct rpb *rpb;
	struct rpb_percpu *percpu;
  
	rpb = (struct rpb *)hwrpb_addr;

        percpu = (struct rpb_percpu *) ((long)rpb + rpb->rpb_percpu_off +
				(mfpr_whami() * rpb->rpb_slotsize));

	/* ***PRM: Where do I get the pal revision from ? */
	DBG_MSG(("&percpu->rpb_palrev_avail[0]: &0x%l016x - 0x%l016x", 
		 &percpu->rpb_palrev_avail[0], *(u_long *)&percpu->rpb_palrev_avail[0]));
	pal_rev_ptr->pal_revision = *(u_long *)&percpu->rpb_palrev_avail[0];
}



int fill_err_cnts(errlog_ptr, error_count_ptr, sw_flags)
	char *errlog_ptr;
	struct elr_error_counters *error_count_ptr;
	struct elr_soft_flags *sw_flags;
{
  int i;

  DBG_MSG(("elr_soft_flags: "));
  ruby_dump_address_range(sw_flags, sizeof(struct elr_soft_flags));

  /*
   * When an error is parsed, bits characterizing the error are set in
   * an elr_soft_flags structure. 
   *
   * This routine accumulates counts for these bits in an
   * elr_error_counters structure, and copies the present counts into
   * the error log packet.
   */

  /* for each error being counted...*/
  for (i=0; i < sizeof(struct elr_error_counters); i++)
  {

	  /* add sw_flag bit to corresponding count field */
	  if ( error_count_ptr->error_counters[i] +=
	      (char)(((sw_flags->error_flags[(i/64)]) >> (i%64)) & 1L)) {

		  DBG_MSG(("Error bit set: %l016x", i));

		  /* move present count into errlog packet */
		  errlog_ptr[i] = (char)error_count_ptr->error_counters[i];
		  DELAY(1000);
		  DBG_MSG(("errlog_ptr[i] = error_count_ptr->error_counters[i];"));
		  DELAY(1000);
		  DBG_MSG(("&errlog_ptr[i]: %l016x, errlog_ptr[i]: %l016x", &errlog_ptr[i], errlog_ptr[i]));
		  DELAY(1000);
		  DBG_MSG(("&error_count_ptr->error_counters[i]: %l016x, error_count_ptr->error_counters[i]: %l016x", &error_count_ptr->error_counters[i], error_count_ptr->error_counters[i]));
		  DELAY(1000);
	  }

      /* move present count into errlog packet */
      errlog_ptr[i] = (char)error_count_ptr->error_counters[i];
    }
  DBG_MSG(("error_count: "));
  ruby_dump_address_range(error_count_ptr, sizeof(struct elr_error_counters));
  DBG_MSG(("elr_error_counters: "));
  ruby_dump_address_range(errlog_ptr, sizeof(struct elr_error_counters));
}



int fill_lep_common_header(hdr_ptr)
     struct elr_lep_common_header *hdr_ptr; 
{
  int node,i;
  struct rpb_percpu *rpb_cpu;

  DBG_MSG(("SW_FLAG1_XMI_SUBPACKET_PRES: 0x%x", SW_FLAG1_XMI_SUBPACKET_PRES));

  /* get pointer to this cpu's rpb per-cpu data structure */
  rpb_cpu = (struct rpb_percpu *)( (u_long) rpb  +
				  rpb->rpb_percpu_off +
				  (mfpr_whami() * rpb->rpb_slotsize)); 

  /* log_off is a historical field from previous, similar, processor */
  /* implementations. It is not used for the DEC7000 */ 
  /* zero fill */
  hdr_ptr->log_off[0]=0;
  hdr_ptr->log_off[1]=0;
  
  /* generate the active cpu field from system state. */
  for (node=0; node<NCPUS; node++)
    {
      if (cpu_to_processor(node)->state != PROCESSOR_OFF_LINE)
	{
	  hdr_ptr->active_cpus |= (1<<node);
	}
    }
  
  
  /* get the CPU's revision information */
  DBG_MSG(("rpb_cpu->rpb_procrev: &0x%l016x - 0x%08x", &rpb_cpu->rpb_procrev, rpb_cpu->rpb_procrev));
  hdr_ptr->hw_rev = rpb_cpu->rpb_procrev;
  
  for (i = 0; i < RSVD2_SIZE; i++)
    {
      hdr_ptr->rsvd2[i] = 0;
    }
  
  /* get the system serial number */
  bcopy(rpb->rpb_ssn,hdr_ptr->sys_sn,SN_SIZE);
  
  /* zero fill reserved fields */
  for (i = 0; i < RSVD1_SIZE; i++)
    {
      hdr_ptr->rsvd1[i] = 0;
    }
  
  /* get module serial number */
  bcopy(rpb_cpu->rpb_procsn,hdr_ptr->mod_sn,SN_SIZE);
  
  /* ***PRM: Need to implement a disable resource mask, which will be */
  /* copies here for error logging */
  /* In error logging there are three possible resource states. */
  /* Disable; Not started (implying the resource is not in use but */
  /* can be used); and Active (which is reflected in the active cpu */
  /* field */

  /* In a single processor environment the only processor will not be */
  /* disabled, so this will be zero. In an MP environment two possible */
  /* ways to track processor states exist. the processor_t structure's */
  /* state field, which presently doesn't allow for distinguishing */
  /* between 'not started' and 'disabled'. There is also the processor */
  /* set module, which can be used by convention to hold disabled */
  /* processor's in a specific set. If the 'processor_t' structure */
  /* field 'state' had constants defined to distinguish between 'not */
  /* started' and 'disabled', its use would fit both a single */
  /* processor and multi-processor environments. The 'processor set' */
  /* model can only be applied to the multi-processor environment.

  /* Implement single processor method, where this processor can not */
  /* be disabled. */
  hdr_ptr->dlist_lw.id = mfpr_whami();
  hdr_ptr->dlist_lw.disable_mask = 0;
  hdr_ptr->dlist_lw.mbz = 0;
  
  /* fill system revision field */
  DBG_MSG(("rpb->rpb_sysrev: &0x%l016x - 0x%08x", &rpb->rpb_sysrev, rpb->rpb_sysrev));
  hdr_ptr->spare = rpb->rpb_sysrev;
  
}



int fill_dlist(dlist_ptr)
     struct elr_dec7000_dlist *dlist_ptr;
{
  /* This fill be fine for uni processor */
  dlist_ptr->subpacket_size = ruby_calc_dlist_size() - 4;
  dlist_ptr->dlist_lw[0].disable_mask = 0;
  dlist_ptr->dlist_lw[0].mbz = 0;
  dlist_ptr->dlist_lw[0].id = mfpr_whami();

  /*  Need something special for mp 

      for (i=0; i < num_cpu(); i++) {
	  dlist_ptr->dlist_lw[i].disable_mask = 0;
	  dlist_ptr->dlist_lw[i].mbz = 0;
	  dlist_ptr->dlist_lw[i].id = mfpr_whami();
  }
  */
  
}



int fill_lsb(lsb_ptr)
     struct elr_lsbsnap *lsb_ptr;
{
  int node=0;
  struct bus *lsbbus;
  struct lsbdata *lsbdata;
  int lsb_node_count = 0;

  /* ***PRM: is this supposed to include to cpu from which it is */
  /* executing? If so, isn't the physical address superfluous */
  
  lsb_ptr->nodes_present = 0;

  /*
   * Get bus number for LSB
   */
  lsbbus = get_sys_bus("lsb");
  if (lsbbus == 0)
	  panic("No system bus configured");
  
  /*
   * Get address of lsbdata structure, get_lsb().
   */
  lsbdata = get_lsb(lsbbus->bus_num);
  
  if (svatophys(lsbdata->cpu_lsb_addr, &lsb_ptr->phys_addr) != KERN_SUCCESS)
	  panic("fill_lsb: - can't translate address of CPU.\n");
  
  lsb_ptr->snapshot_size = ruby_calc_lsb_size() - sizeof(lsb_ptr->snapshot_size);

  /*
   * find nodes which are 'alive', lsbdata->lsbnodes_alive & (1<<node).
   * get node's device type, lsbdata-lsberr[lsbnode].plsbsw->lsbldev
   */

  DBG_MSG(("lsbdata->lsbnodes_alive: 0x%l08x", lsbdata->lsbnodes_alive));
  for (node; node < MAX_LSB_NODE; node++)
  {
    /* lsbnode_alive doesn't reflect cpus. This should be corrected in */
    /* lsbinit before SMP to create the proper snapshot */
    if ((lsbdata->lsbnodes_alive & (1<<node)) || (node == mfpr_whami()))
    {
      lsb_ptr->nodes_present |= (1<<node);
      fill_lsb_node(&lsb_ptr->lsb_req[lsb_node_count], node);
      lsb_node_count++;
    }
  }
  DBG_MSG(("lsb_ptr->nodes_present: 0x%l08x", lsb_ptr->nodes_present));
}



int fill_lma( lma_ptr)
     struct elr_lma_sub *lma_ptr;
{
  int node=0;
  struct bus *lsbbus;
  struct lsbdata *lsbdata;
  struct lsb_lma_reg *lsb;

  /*
   * Get bus number for LSB
   */
  lsbbus = get_sys_bus("lsb");
  if (lsbbus == 0)
	  panic("No system bus configured");
  
  /*
   * Get address of lsbdata structure, get_lsb().
   */
  lsbdata = get_lsb(lsbbus->bus_num);
  
  /* Clear/initialize fields */
  lma_ptr->lma_mcr = 0;
  lma_ptr->lma_amr = 0;
  lma_ptr->lma_fadr = 0;
  lma_ptr->lma_mera = 0;
  lma_ptr->lma_msynda = 0;
  lma_ptr->lma_merb = 0;
  lma_ptr->lma_msyndb = 0;
  lma_ptr->lma_spare1 = 0;

  /* search for LMA with error. */
  for (node; node < MAX_LSB_NODE; node++)
  {
    if ((lsbdata->lsbnodes_alive & (1<<node)) &&
	(((lsbdata->lsberr[node].plsbsw->lsb_ldev & LSBLDEV_TYPE) == LSB_MEM) ||
	((lsbdata->lsberr[node].plsbsw->lsb_ldev & LSBLDEV_TYPE) == LSB_BBMEM)))
    {
      lsb = (struct lsb_lma_reg *)lsbdata->lsbvirt[node];

      dprintf("node: %d, LDEV: 0x%x, amr: 0x%x, mera: 0x%x\n", 
	      node, 
	      (lsbdata->lsberr[node].plsbsw->lsb_ldev & LSBLDEV_TYPE),
	      lsb->lma_amr,
	      lsb->lma_mera);

      if (lsb->lma_lber & LSB_TDE)
	{
	  fill_lsb_node(lma_ptr, node);

	  /* check if access to lma succeeded through fill_lsb_node and fill */
	  /* rest of packet if it did. */
	  if (lma_ptr->valid_bit_mask)
	    {
	      lma_ptr->lma_mcr = lsb->lma_mcr;
	      lma_ptr->valid_bit_mask |= (1L<<10);

	      lma_ptr->lma_amr = lsb->lma_amr;
	      lma_ptr->valid_bit_mask |= (1L<<11);

	      lma_ptr->lma_fadr = lsb->lma_fadr;
	      lma_ptr->valid_bit_mask |= (1L<<12);

	      lma_ptr->lma_mera = lsb->lma_mera;
	      lma_ptr->valid_bit_mask |= (1L<<13);

	      lma_ptr->lma_msynda = lsb->lma_msynda;
	      lma_ptr->valid_bit_mask |= (1L<<14);

	      lma_ptr->lma_merb = lsb->lma_merb;
	      lma_ptr->valid_bit_mask |= (1L<<15);

	      lma_ptr->lma_msyndb = lsb->lma_msyndb;
	      lma_ptr->valid_bit_mask |= (1L<<16);
	      lma_ptr->lma_spare1 = 0;
	    }
	}
    }
  }
}



/* 
 * fill_adp( adp_ptr):
 *
 * This routine takes a pointer into an error log packet to a field
 * which begins an elr_iop_adp_subp subpacket. It uses lsb_fill_node to
 * fill the common registers and then fills the IOP specific registers.
 *
 */

int fill_adp( adp_ptr)
	struct elr_iop_adp_subp *adp_ptr;
{
  int node = 0;
  struct bus *lsbbus;
  struct lsbdata *lsbdata;
  struct iopreg *iop;

  /*
   * Get bus number for LSB
   */
  lsbbus = get_sys_bus("lsb");
  if (lsbbus == 0)
	  panic("No system bus configured");
  
  /*
   * Get address of lsbdata structure, get_lsb().
   */
  lsbdata = get_lsb(lsbbus->bus_num);
  
  /* search for IOP with error. */
/*  for (node; node < MAX_LSB_NODE; node++) */
  node = 8; /* IOP is always node 8 */
  {
    if ((lsbdata->lsbnodes_alive & (1<<node)) &&
	((lsbdata->lsberr[node].plsbsw->lsb_ldev & LSBLDEV_TYPE) == LSB_IOP))
    {
      iop = (struct iopreg *)lsbdata->lsbvirt[node];
      fill_lsb_node(adp_ptr, node);

      /* check if access to iop succeeded through fill_lsb_node and fill */
      /* rest of packet if it did. */
      if (adp_ptr->valid)
	{
	  adp_ptr->iop_ipcnse = iop->ipcnse;
	  adp_ptr->valid |= (1L<<10);

	  adp_ptr->iop_ipcvr = iop->ipcvr;
	  adp_ptr->valid |= (1L<<11);

	  adp_ptr->iop_ipcmsr = iop->ipcmsr;
	  adp_ptr->valid |= (1L<<12);

	  adp_ptr->iop_ipchst = iop->ipchst;
	  adp_ptr->valid |= (1L<<13);

	}
    }
  }
}



int fill_lsb_node(lsb_ptr, node)
     struct elr_lsb_req *lsb_ptr;
     int node;
{
  u_long addr_ptr;
  struct bus *lsbbus;
  struct lsbdata *lsbdata;
  struct lsb_lep_reg *lsb;

  /*
   * Get bus number for LSB
   */
  lsbbus = get_sys_bus("lsb");
  if (lsbbus == 0)
	  panic("No system bus configured");
  
  /*
   * Get address of lsbdata structure, get_lsb().
   */
  lsbdata = get_lsb(lsbbus->bus_num);
  
  lsb = (struct lsb_lep_reg *)lsbdata->lsbvirt[node];

  /* set mask to indicate no valid registers */
  lsb_ptr->valid_bit_mask = 0;
  
  /*
   * Need to protect against bad accesses to LSB, in case a node has
   * become inaccessible.
   */
  
  /* Check to see if the lsb node responds to read. To do this machine */
  /* check must be enabled. */
if (BADADDR((int *) lsb,sizeof(int), lsbbus))
/*  if (0) */
  {
    lsb_ptr->lsb_ldev = lsbdata->lsberr[node].plsbsw->lsb_ldev;
  }
  else
  {
    svatophys( lsb, &addr_ptr );
    lsb_ptr->lsb_addr = addr_ptr;

    lsb_ptr->lsb_ldev = lsb->lsb_lep_ldev;
    lsb_ptr->valid_bit_mask |= (1L<<0);

    lsb_ptr->lsb_lber = lsb->lsb_lep_lber;
    lsb_ptr->valid_bit_mask |= (1L<<1);

    lsb_ptr->lsb_lcnr = lsb->lsb_lep_lcnr;
    lsb_ptr->valid_bit_mask |= (1L<<2);

    lsb_ptr->lsb_spare0 = 0;

    if ((lsb_ptr->lsb_ldev & LSBLDEV_TYPE) == LSB_LEP)
    {
      lsb_ptr->lsb_spare0 = lsb->lsb_lep_lmerr;
      lsb_ptr->valid_bit_mask |= (1L<<3);
    }

    lsb_ptr->lsb_lbesr0 = lsb->lsb_lep_lbesr0;
    lsb_ptr->valid_bit_mask |= (1L<<4);

    lsb_ptr->lsb_lbesr1 = lsb->lsb_lep_lbesr1;
    lsb_ptr->valid_bit_mask |= (1L<<5);

    lsb_ptr->lsb_lbesr2 = lsb->lsb_lep_lbesr2;
    lsb_ptr->valid_bit_mask |= (1L<<6);

    lsb_ptr->lsb_lbesr3 = lsb->lsb_lep_lbesr3;
    lsb_ptr->valid_bit_mask |= (1L<<7);

    lsb_ptr->lsb_lbecr0 = lsb->lsb_lep_lbecr0;
    lsb_ptr->valid_bit_mask |= (1L<<8);

    lsb_ptr->lsb_lbecr1 = lsb->lsb_lep_lbecr1;
    lsb_ptr->valid_bit_mask |= (1L<<9);

  }
}


int ruby_parse_660_mcheck(mchk_logout, sw_flags)
 	struct ruby_mchk_logout *mchk_logout;	/* This is the virtual */
						/* address of the */
						/* machine check frame */
						/* used by pal */
	struct elr_soft_flags *sw_flags;
				/* address of the software flags, */
				/* which are used to return parse */
				/* results and as part of the defined */
				/* error packet */
{
	int found_one_level_1;	/* used to count the number of level */
				/* one expected conditions in found */
	int found_660_error=0;

  PRINT_PARSE(("660 Error Parsing: entered..."));

	/* Top level 660 Parse decision. Conditional on the state of */
	/* bits in LEP_LBER. This is coded such that any number of */
	/* conditions can be detected at the same time. */

	if (mchk_logout->biu_stat & FILL_ECC)
	{
	    DBG_MSG(("Machine Check 660: FILL_ECC"));
	    found_660_error++;
	    ruby_660_fill_ecc(mchk_logout, sw_flags);
	}


	if (mchk_logout->lep_lber & LBER_NSES)
	{ /* error specific to CPU */

	    DBG_MSG(("Machine Check 660: LBER_NSES"));
	    DBG_MSG(("	mchk_logout->lep_lber: 0x%l016x", mchk_logout->lep_lber));
	    found_660_error++;
	    ruby_660_lber_nses(mchk_logout, sw_flags);
	  }

	if ((mchk_logout->lep_lber & LBER_E) &&	(mchk_logout->lep_lber & (LBER_SHE | LBER_DIE)))
	{/* 10.1.17 */
		found_660_error++;
		
		sw_flags->error_flags[0] |= SW_FLAG0_LSB_ERR;
		
		sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT;
		
		if (mchk_logout->lep_lber & LBER_SHE)
			sw_flags->error_flags[0] |= SW_FLAG0_SHE;
		
		if (mchk_logout->lep_lber & LBER_DIE)
			sw_flags->error_flags[0] |= SW_FLAG0_DIE;
		ERROR_CRASH;
	}
	
	if ((mchk_logout->lep_lber & LBER_E) &&
	    (((mchk_logout->lep_lbecr1 & LBECR1_CID) >> LBECR1_CID_SHIFT) == 
	     mfpr_whami()))
	{ /* LSB error with CPU as bus commander */

	  DBG_MSG(("Machine Check 660: LBER_E CPU"));
	  DBG_MSG(("	mchk_logout->lep_lber: 0x%l016x", mchk_logout->lep_lber));
	  found_660_error++;
	  ruby_660_lber_e_cpu(mchk_logout, sw_flags);
	}

	if ((mchk_logout->lep_lber & LBER_E) &&
	    (((mchk_logout->lep_lbecr1 & LBECR1_CID) >> LBECR1_CID_SHIFT) == 
	     LSB_IOP_NODE))
	{ /* correctable LSB error when IOP is bus commander */

	  DBG_MSG(("Machine Check 660: LBER_E"));
	  DBG_MSG(("	mchk_logout->lep_lber: 0x%l016x", mchk_logout->lep_lber));
	  found_660_error++;
	  ruby_660_lber_e_iop(mchk_logout, sw_flags);
	}

	if ((mchk_logout->lep_lber & LBER_E) &&
	    (((mchk_logout->lep_lbecr1 & LBECR1_CID) >> LBECR1_CID_SHIFT) != 
	     mfpr_whami()) &&
	    (mchk_logout->lep_lber & LBER_CE))
	{ /* correctable LSB error when CPU is not bus commander */
	  /* 10.1.26 */
	    found_660_error++;
	    sw_flags->error_flags[0] |= SW_FLAG0_BYSTANDER_CE;
            ERROR_DISMISS;
	}

	if ((mchk_logout->lep_lber & LBER_E) &&
	    (((mchk_logout->lep_lbecr1 & LBECR1_CID) >> LBECR1_CID_SHIFT) != 
	     mfpr_whami()) &&
	    (mchk_logout->lep_lber & LBER_UCE))
	{ /* correctable LSB error when CPU is not bus commander */
	  /* 10.1.28 */
	    found_660_error++;
	    sw_flags->error_flags[0] |= SW_FLAG0_BYSTANDER_UCE;
	    ERROR_DISMISS;
	}

	/* test that at least one error was found */
	if (found_660_error != 0)
	{
		/* There is an error */
		DBG_MSG(("ruby_parse_660_mcheck: Found error..."));
		ruby_mcheck_logout_dump(mchk_logout, sw_flags);
	}
	else 
	{/* 10.1.43 */
		/* There were no expected bits set !!! */
		DBG_MSG(("ruby_parse_660_mcheck: No expected error detected by parsing..."));
		ruby_mcheck_logout_dump(mchk_logout, sw_flags);
		sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT;
		sw_flags->error_flags[0] |= SW_FLAG0_660_INCON;
	        ERROR_CRASH;
	}
}



int ruby_660_lber_e_cpu(mchk_logout, sw_flags)
	struct ruby_mchk_logout *mchk_logout;
		/* address of the machine check frame */
		/* to use for parsing */
	struct elr_soft_flags *sw_flags;
		/* address of the software flags, */
		/* which are used to return parse */
		/* results and as part of the defined */
		/* error packet */
{
	int cpu_error_found=0;
	
	if (mchk_logout->lep_lber & (LBER_STE | LBER_CNFE | LBER_CAE ))
	{/* 10.1.18 */
		cpu_error_found++;
		
		sw_flags->error_flags[0] |= SW_FLAG0_LSB_ERR;
		
		sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT;
		
		if (mchk_logout->lep_lber & LBER_STE)
		{
			sw_flags->error_flags[0] =
				(sw_flags->error_flags[0] |
				 SW_FLAG0_STALL_ERROR);
		}
		if (mchk_logout->lep_lber & LBER_CNFE)
		{
			sw_flags->error_flags[0] =
				(sw_flags->error_flags[0] |
				 SW_FLAG0_CNF_ERROR);
		}
		if (mchk_logout->lep_lber & LBER_CAE)
		{
			sw_flags->error_flags[0] =
				(sw_flags->error_flags[0] |
				 SW_FLAG0_CA_ERROR);
		}
		ERROR_CRASH;
	}/* 10.1.18 */
	
	if (mchk_logout->lep_lber & LBER_NXAE)
	{ /* 10.1.19 */
		cpu_error_found++;
		
		sw_flags->error_flags[0] |= SW_FLAG0_NXAE;
		
		sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT
		  | SW_FLAG1_LMA_PRESENT;
		
		if ( ((mchk_logout->lep_lbecr1 & LBECR1_CA_CMD ) ==
		     LBECR1_CA_WRITE_VICTIM) && 
		    (((mchk_logout->lep_lbecr1 & LBECR1_CID) >>
		      LBECR1_CID_SHIFT) == mfpr_whami()))
		{
			/* No specific action */
		}
		
		else if ( ((mchk_logout->lep_lbecr1 & LBECR1_CA_CMD ) ==
			  LBECR1_CA_WRITE) && 
			 (((mchk_logout->lep_lbecr1 & LBECR1_CID) >>
			   LBECR1_CID_SHIFT) == mfpr_whami()))
		{
			/* No specific action */
		}
		else
		{
			/* No specific action */
			sw_flags->error_flags[0] |= SW_FLAG0_INCONSISTENT;
		}
		/* enhance to collect from correct memory */
                ERROR_CRASH;
	}/* 10.1.19 */
	
	if (mchk_logout->lep_lber & LSB_TDE)
	  {/* LSB_TDE. 10.1.20-23 */
		  int tde_error_found=0;
		  
		  cpu_error_found++;
		  
		  if ((mchk_logout->lep_lber & LBER_CPE) &&
		      ( ((mchk_logout->lep_lbecr1 & LBECR1_CA_CMD ) ==
			 LBECR1_CA_WRITE_VICTIM) ||
		       ( (mchk_logout->lep_lbecr1 & LBECR1_CA_CMD ) ==
			LBECR1_CA_WRITE)))
		    {/* 10.1.20 */
			    tde_error_found++;
			    sw_flags->error_flags[0] |= SW_FLAG0_LSB_ERR;
			    
			    sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT;
			    
			    sw_flags->error_flags[0] |= SW_FLAG0_660_CPE;
			    ERROR_CRASH;
		    }
		  
		  if (mchk_logout->lep_lber & LBER_CE)
		    {/* 10.1.21 */
			    tde_error_found++;
			    sw_flags->error_flags[0] |= SW_FLAG0_LSB_ERR;
			    
			    sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT;
			    
			    sw_flags->error_flags[0] |= SW_FLAG0_CE;
			    
			    sw_flags->error_flags[1] |= SW_FLAG1_LOG_ADAPT_PRES;
			    /* enhance to detect if this cpu was not the consumer  */
			    ERROR_RECOVER;
		    }
		  
		  if (mchk_logout->lep_lber & LBER_UCE)
		    {/* 10.1.22 */
			    tde_error_found++;
			    sw_flags->error_flags[0] |= SW_FLAG0_LSB_ERR;
			    
			    sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT;
			    
			    sw_flags->error_flags[0] |= SW_FLAG0_UCE;
			    
			    sw_flags->error_flags[1] |= SW_FLAG1_LOG_ADAPT_PRES;
			    /* enhance to detect if this cpu was not the consumer  */
			    ERROR_CRASH;
		    }
		  
		  if (mchk_logout->lep_lber & LBER_CDPE)
		    {/* 10.1.23 */
			    tde_error_found++;
			    sw_flags->error_flags[0] |= SW_FLAG0_LSB_ERR;
			    
			    sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT;
			    
			    sw_flags->error_flags[0] |= SW_FLAG0_WRT_CDPE;
			    ERROR_CRASH;
		    }
		  
		  if (tde_error_found == 0)
		    {/* inconsistent */
			    sw_flags->error_flags[0] |= SW_FLAG0_LSB_ERR;
			    
			    sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT;
			    
			    sw_flags->error_flags[0] |= SW_FLAG0_INCONSISTENT;
			    
			    sw_flags->error_flags[1] |= SW_FLAG1_LOG_ADAPT_PRES;
			    ERROR_CRASH;
		    }
	  }/* LSB_TDE. 10.1.20-23 */
	
	if ((mchk_logout->lep_lber & LBER_CDPE) && !(mchk_logout->lep_lber & LBER_TDE))
	  {/* 10.1.24 */
		  cpu_error_found++;		
		  sw_flags->error_flags[0] |= SW_FLAG0_LSB_ERR;
		  
		  sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT;
		  
#define SW_FLAG0_660_CDPE SW_FLAG0_WRT_CDPE	/* This flag changed its name in the PFMS. */
		  				/* It has not been reflected in            */
		  				/* kernel/dec/binlog/errlog.h              */

		  sw_flags->error_flags[0] |= SW_FLAG0_660_CDPE;
		  ERROR_CRASH;
	  }
	
	if ((mchk_logout->lep_lber & LBER_CE) && !(mchk_logout->lep_lber & LBER_TDE))
	{ 
		cpu_error_found++;		
		if ((mchk_logout->lep_lbecr1 & LBECR1_CA_CMD) == LBECR1_CA_READ)
		{/* 10.1.25 */
			sw_flags->error_flags[0] |= SW_FLAG0_LSB_ERR |
				SW_FLAG0_BCACHE_FILL_2_3_CE; 
			
			sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT;
                        ERROR_RECOVER;
		}

		if (mchk_logout->lep_lbecr1 & LBECR1_SHARED)
		{
			/* actions not defined - should be dismissed without    */
			/* action, but we've already allocated the error log... */
                        ERROR_DISMISS;
		}
	}
	
	if ((mchk_logout->lep_lber & LBER_UCE) && !(mchk_logout->lep_lber & LBER_TDE))
	{
		cpu_error_found++;		
		
		if ((mchk_logout->lep_lbecr1 & LBECR1_CA_CMD) == LBECR1_CA_READ)
		{/* 10.1.27 */
			sw_flags->error_flags[0] |= SW_FLAG0_LSB_ERR |
				SW_FLAG0_BCACHE_FILL_2_3_UCE; 
			
			sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT |
				SW_FLAG1_LMA_PRESENT;
			/* enhance to collect from correct memory */
                        ERROR_CRASH;
		}
		if (mchk_logout->lep_lbecr1 & LBECR1_SHARED)
		{
			/* actions not defined - should be dismissed without    */
			/* action, but we've already allocated the error log... */
                        ERROR_DISMISS;
		}
	}
	
	if (mchk_logout->lep_lber & LBER_UCE2)
	{/* 10.1.29 */
		sw_flags->error_flags[0] |= SW_FLAG0_UCE2;
	}
	
	if (mchk_logout->lep_lber & LBER_CE2)
	{/* 10.1.29 */
		sw_flags->error_flags[0] |= SW_FLAG0_CE2;
	}
	
	if (mchk_logout->lep_lber & LBER_CPE2)
	{/* 10.1.29 */
		sw_flags->error_flags[0] |= SW_FLAG0_660_CPE2;
	}
	
	if (mchk_logout->lep_lber & LBER_CDPE2)
	{/* 10.1.29 */
		sw_flags->error_flags[0] |= SW_FLAG0_CDPE2;
	}
	
	if ( cpu_error_found == 0) /* LSB Error, commander this node */
	{/* 10.1.30 */
		sw_flags->error_flags[0] |= SW_FLAG0_LSB_ERR |
			SW_FLAG0_LBER_INCON;
		sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT;
		ERROR_CRASH;
	}
}



int ruby_660_lber_e_iop(mchk_logout, sw_flags)
     struct ruby_mchk_logout *mchk_logout;
				/* address of the machine check frame */
				/* to use for parsing */
     struct elr_soft_flags *sw_flags;
				/* address of the software flags, */
				/* which are used to return parse */
				/* results and as part of the defined */
				/* error packet */
{/* LSB err, IOP cmdr */
	
	struct iopreg *iop;
	int iop_error_found=0;
	
	/* get address of IOP */
	iop = (struct iopreg *)lsb_addr(LSB_IOP_NODE);
	
	/* 10.1.31 */
	sw_flags->error_flags[0] |= SW_FLAG0_IOP_CMDR;
	
	if (iop->lber & LSB_STE)
	{/* 10.1.32 */
		iop_error_found++;
		sw_flags->error_flags[0] |= SW_FLAG0_IOP_STE;
		sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT;
		sw_flags->error_flags[1] |= SW_FLAG1_LOG_ADAPT_PRES;
		ERROR_CRASH;
	}
	
	if (iop->lber & LSB_CAE)
	{/* 10.1.32 */
		iop_error_found++;
		sw_flags->error_flags[0] |= SW_FLAG0_IOP_CAE;
		sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT;
		sw_flags->error_flags[1] |= SW_FLAG1_LOG_ADAPT_PRES;
		ERROR_CRASH;
	}
	
	if (iop->lber & LSB_CNFE)
	{/* 10.1.32 */
		iop_error_found++;
		sw_flags->error_flags[0] |= SW_FLAG0_IOP_CNFE;
		sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT;
		sw_flags->error_flags[1] |= SW_FLAG1_LOG_ADAPT_PRES;
		ERROR_CRASH;
	}
	
	if ( (iop->lbecr1 & LBECR1_CA_CMD ) == LBECR1_CA_WRITE)
	{
		int write_error_found=0;
		
		iop_error_found++;
		
		if (iop->lber & LSB_NXAE)
		{/* 10.1.33 */
			write_error_found++;
			sw_flags->error_flags[0] |= SW_FLAG0_IOP_NXAE |
				SW_FLAG0_IOP_LSB_WRITE;
			sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT;
        		ERROR_CRASH;
		}
		if (iop->lber & LSB_CPE)
		{/* 10.1.34 */
			write_error_found++;
			sw_flags->error_flags[0] |= SW_FLAG0_IOP_CPE |
				SW_FLAG0_IOP_LSB_WRITE;
			sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT;
        		ERROR_CRASH;
		}
		if (iop->lber & LSB_CE)
		{/* 10.1.36 */
			write_error_found++;
			sw_flags->error_flags[0] |= SW_FLAG0_IOP_CE |
				SW_FLAG0_IOP_LSB_WRITE;
			sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT;
        		ERROR_RECOVER;
		}
		if (iop->lber & LSB_UCE)
		{/* 10.1.38 */
			write_error_found++;
			sw_flags->error_flags[0] |= SW_FLAG0_IOP_UCE |
				SW_FLAG0_IOP_LSB_WRITE;
			sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT;
        		ERROR_CRASH;
		}
		if (write_error_found == 0)
		{/* 10.1.40 */
			sw_flags->error_flags[0] |= SW_FLAG0_IOP_LSB_INCON_STATE |
				SW_FLAG0_IOP_LSB_WRITE;
			sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT;
        		ERROR_CRASH;
		}
	}
	
	if ( (iop->lbecr1 & LBECR1_CA_CMD ) == LBECR1_CA_READ)
	{
		int read_error_found=0;
		
		iop_error_found++;
		
		if (iop->lber & LSB_NXAE)
		{/* 10.1.33 */
			read_error_found++;
			sw_flags->error_flags[0] |= SW_FLAG0_IOP_NXAE |
				SW_FLAG0_IOP_LSB_READ;
			sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT;
        		ERROR_CRASH;
		}
		if (iop->lber & LSB_CPE)
		{/* 10.1.34 */
			read_error_found++;
			sw_flags->error_flags[0] |= SW_FLAG0_IOP_CPE |
				SW_FLAG0_IOP_LSB_READ;
			sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT;
        		ERROR_CRASH;
		}
		if (iop->lber & LSB_CE)
		{/* 10.1.36-37 */
			read_error_found++;
			sw_flags->error_flags[0] |= SW_FLAG0_IOP_CE |
				SW_FLAG0_IOP_LSB_READ;
			sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT;
        		ERROR_RECOVER;
		}
		if (iop->lber & LSB_UCE)
		{/* 10.1.38-39 */
			read_error_found++;
			sw_flags->error_flags[0] |= SW_FLAG0_IOP_UCE |
				SW_FLAG0_IOP_LSB_READ;
			sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT;
        		ERROR_CRASH;
		}
		if (read_error_found == 0)
		{/* 10.1.40 */
			sw_flags->error_flags[0] |= SW_FLAG0_IOP_LSB_INCON_STATE |
				SW_FLAG0_IOP_LSB_READ;
			sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT;
        		ERROR_CRASH;
		}
	}
	
	if ( (iop->lbecr1 & LBECR1_CA_CMD ) == LBECR1_CA_WRITE_CSR)
	{
		int write_csr_error_found=0;
		
		iop_error_found++;
		
		if (iop->lber & LSB_NXAE)
		{/* 10.1.33 */
			write_csr_error_found++;
			sw_flags->error_flags[0] |= SW_FLAG0_IOP_NXAE |
				SW_FLAG0_IOP_LSB_CSR_WRITE;
			sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT;
        		ERROR_CRASH;
		}
		if (iop->lber & LSB_CPE)
		{/* 10.1.34 */
			write_csr_error_found++;
			sw_flags->error_flags[0] |= SW_FLAG0_IOP_CPE |
				SW_FLAG0_IOP_LSB_CSR_WRITE;
			sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT;
        		ERROR_CRASH;
		}
		if (iop->lber & LSB_CDPE)
		{/* 10.1.35 */
			write_csr_error_found++;
			sw_flags->error_flags[0] |= SW_FLAG0_IOP_CDPE |
				SW_FLAG0_IOP_LSB_CSR_WRITE;
			sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT;
        		ERROR_CRASH;
		}
		if (write_csr_error_found == 0)
		{/* 10.1.40 */
			sw_flags->error_flags[0] |= SW_FLAG0_IOP_LSB_INCON_STATE |
				SW_FLAG0_IOP_LSB_CSR_WRITE;
			sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT;
        		ERROR_CRASH;
		}
	}
	
	if (iop->lber & LSB_CPE2)
	{/* 10.1.41 */
		iop_error_found++;
		sw_flags->error_flags[0] |= SW_FLAG0_IOP_CPE2;
	}
	
	if (iop->lber & LSB_CDPE2)
	{/* 10.1.41 */
		iop_error_found++;
		sw_flags->error_flags[0] |= SW_FLAG0_IOP_CDPE2;
	}
	
	if (iop->lber & LSB_CE2)
	{/* 10.1.41 */
		iop_error_found++;
		sw_flags->error_flags[0] |= SW_FLAG0_IOP_CE2;
	}
	
	if (iop->lber & LSB_UCE2)
	{/* 10.1.41 */
		iop_error_found++;
		sw_flags->error_flags[0] |= SW_FLAG0_IOP_UCE2;
	}
	
	if (iop->lber & LSB_NSES)
	{/* 10.1.42 */
		iop_error_found++;
		sw_flags->error_flags[0] |= SW_FLAG0_IOP_NSES;
		sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT;
		sw_flags->error_flags[1] |= SW_FLAG1_LOG_ADAPT_PRES;
		ERROR_DISMISS;
	}
	
	if (iop_error_found == 0)
	{/* 10.1.40 */
		sw_flags->error_flags[0] |= SW_FLAG0_IOP_LSB_INCON_STATE;
		sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT;
		ERROR_CRASH;
	}
	
}/* LSB err, IOP cmdr */



int ruby_660_lber_nses(mchk_logout, sw_flags)
     struct ruby_mchk_logout *mchk_logout;
				/* address of the machine check frame */
				/* to use for parsing */
     struct elr_soft_flags *sw_flags;
				/* address of the software flags, */
				/* which are used to return parse */
				/* results and as part of the defined */
				/* error packet */
{ /* select all nses */
  int found_660_nses = 0;

  /* add 8 to all the section numbers here. The spec was reordered. ARRrrrggghh!! */

  if (mchk_logout->lep_lmerr & LMERR_ARBDROP)
    {/* 10.1.9 */
      PRINT_PARSE((" 10.1.9 "));
      found_660_nses++;
      sw_flags->error_flags[0] = sw_flags->error_flags[0] |
	SW_FLAG0_660_ARBDRP;
      /* basic int660 entry */
      /* lsb subpacket */
      sw_flags->error_flags[1] = sw_flags->error_flags[1] |
	SW_FLAG1_LSB_PRESENT;
      ERROR_CRASH;
    }

  if (mchk_logout->lep_lmerr & LMERR_ARBCOL)
    {/* 10.1.9 */
      PRINT_PARSE((" 10.1.9 "));
      found_660_nses++;
      sw_flags->error_flags[0] = sw_flags->error_flags[0] |
	SW_FLAG0_660_ARBCOL;
      /* basic mcheck entry */
      /* lsb subpacket */
      sw_flags->error_flags[1] = sw_flags->error_flags[1] |
	SW_FLAG1_LSB_PRESENT;
      ERROR_CRASH;
    }

  if (mchk_logout->lep_lmerr & LMERR_BMAPPE)
    {/* 10.1.10 */
      PRINT_PARSE((" 10.1.10 "));
      found_660_nses++;
      sw_flags->error_flags[0] = sw_flags->error_flags[0] |
	SW_FLAG0_BMAPPE; /* 3 */
      /* basic 660 entry */
      ERROR_CRASH;
    }

  if (mchk_logout->lep_lmerr & LMERR_PMAPPE)
    {/* 10.1.11 */
      PRINT_PARSE((" 10.1.11 "));
      found_660_nses++;
      sw_flags->error_flags[0] = sw_flags->error_flags[0] |
	SW_FLAG0_PMAPPE; /* 4 */
      /* basic 660 entry */
      ERROR_CRASH;
    }

  if (mchk_logout->lep_lmerr & LMERR_BDATASBE)
    {/* BDATASBE, select ? *//* 10.1.12 */
      found_660_nses++;
      ruby_660_bdatasbe(mchk_logout, sw_flags);
    }
  
  if (mchk_logout->lep_lmerr & LMERR_BDATADBE)
    {/* BDATADBE, select ? *//* 10.1.13 */
      found_660_nses++;
      ruby_660_bdatadbe(mchk_logout, sw_flags);
    }
  
  if (mchk_logout->lep_lmerr & LMERR_BTAGPE)
    {/* 10.1.14 */
      found_660_nses++;
      ruby_660_btagpe(mchk_logout, sw_flags);
    }

  if (mchk_logout->lep_lmerr & LMERR_BSTATPE)
    {/* 10.1.15 */
      found_660_nses++;
      ruby_660_bstatpe(mchk_logout, sw_flags);
    }

  if (found_660_nses == 0)
    {/* 10.1.16 */
      PRINT_PARSE((" 10.1.16 "));
      PRINT_PARSE(("Inconstent"));
      /* basic mcheck entry */
      /* lsb subpacket */	
      sw_flags->error_flags[0] |= SW_FLAG0_INCON_NSES; /* 9 */ 
      sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT;
      ERROR_CRASH;
    }

} /* select all nses */



ruby_660_bstatpe(mchk_logout, sw_flags)
     struct ruby_mchk_logout *mchk_logout;
				/* address of the machine check frame */
				/* to use for parsing */
     struct elr_soft_flags *sw_flags;
				/* address of the software flags, */
				/* which are used to return parse */
				/* results and as part of the defined */
				/* error packet */
{
	int bstatpe_found=0;
	
	if ( ((mchk_logout->lep_lbecr1 & LBECR1_CA_CMD ) == LBECR1_CA_READ) &&
	    (((mchk_logout->lep_lbecr1 & LBECR1_CID) >> LBECR1_CID_SHIFT) !=
	     mfpr_whami()))
	{/* 10.1.15 */
	  PRINT_PARSE((" 10.1.15 "));
		bstatpe_found++;
		/* basic 660 entry */
		sw_flags->error_flags[0] |= SW_FLAG0_660_BSTATPE; /* 8 */ 
          ERROR_CRASH;
	}
	
	if ( ((mchk_logout->lep_lbecr1 & LBECR1_CA_CMD ) == LBECR1_CA_WRITE) &&
	    (((mchk_logout->lep_lbecr1 & LBECR1_CID) >> LBECR1_CID_SHIFT) !=
	     mfpr_whami()))
	{/* 10.1.15 */
	  PRINT_PARSE((" 10.1.15 "));
		bstatpe_found++;
		/* basic 660 entry */
		sw_flags->error_flags[0] |= SW_FLAG0_660_BSTATPE; /* 8 */ 
          ERROR_CRASH;

	}

       	if (bstatpe_found == 0)
	{/* 10.1.15 */
	  PRINT_PARSE((" 10.1.15 "));
		sw_flags->error_flags[0] |= SW_FLAG0_660_BSTATPE |
			SW_FLAG0_INCON_NSES; /* 8 | 9 */ 
          ERROR_CRASH;
	}
}



ruby_660_btagpe(mchk_logout, sw_flags)
     struct ruby_mchk_logout *mchk_logout;
				/* address of the machine check frame */
				/* to use for parsing */
     struct elr_soft_flags *sw_flags;
				/* address of the software flags, */
				/* which are used to return parse */
				/* results and as part of the defined */
				/* error packet */
{
	int btagpe_found=0;

	if ( ((mchk_logout->lep_lbecr1 & LBECR1_CA_CMD ) == LBECR1_CA_READ) &&
	(((mchk_logout->lep_lbecr1 & LBECR1_CID) >> LBECR1_CID_SHIFT) !=
	 mfpr_whami()))
	  {/* 10.1.14 */
	    PRINT_PARSE((" 10.1.14 "));
	    btagpe_found++;
	    /* basic 660 entry */
	    sw_flags->error_flags[0] |= SW_FLAG0_660_BTAGPE; /* 6 */ 
          ERROR_CRASH;
	  }
      
	if ( ((mchk_logout->lep_lbecr1 & LBECR1_CA_CMD ) == LBECR1_CA_WRITE) &&
	(((mchk_logout->lep_lbecr1 & LBECR1_CID) >> LBECR1_CID_SHIFT) !=
	 mfpr_whami()))
	  {/* 10.1.14 */
	    PRINT_PARSE((" 10.1.14 "));
	    btagpe_found++;
	    /* basic 660 entry */
	    sw_flags->error_flags[0] |= SW_FLAG0_660_BTAGPE; /* 6 */
          ERROR_CRASH;
	  }
      
      if (btagpe_found == 0)
	{/* 10.1.14 */
	  PRINT_PARSE((" 10.1.14 "));
	  sw_flags->error_flags[0] |= SW_FLAG0_660_BTAGPE |
	    SW_FLAG0_INCON_NSES; /* 5 | 9 */ 
          ERROR_CRASH;
	}

}



ruby_660_bdatadbe(mchk_logout, sw_flags)
     struct ruby_mchk_logout *mchk_logout;
				/* address of the machine check frame */
				/* to use for parsing */
     struct elr_soft_flags *sw_flags;
				/* address of the software flags, */
				/* which are used to return parse */
				/* results and as part of the defined */
				/* error packet */
{
      int bdatadbe_found = 0;
      
      if ( ((mchk_logout->lep_lbecr1 & LBECR1_CA_CMD ) == LBECR1_CA_READ) &&
	(((mchk_logout->lep_lbecr1 & LBECR1_CID) >> LBECR1_CID_SHIFT) !=
	 mfpr_whami()))
	  {/* 10.1.5 */
	    PRINT_PARSE((" 10.1.5 "));
	    bdatadbe_found++;
	    /* basic 660 entry */
	    sw_flags->error_flags[0] |= SW_FLAG0_BDATADBE; /* 6 */ 
	    sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT |
	      SW_FLAG1_LOG_ADAPT_PRES;
	    ERROR_CRASH;
	  }
      
      if ( ((mchk_logout->lep_lbecr1 & LBECR1_CA_CMD ) ==
	   LBECR1_CA_WRITE_VICTIM) && 
	  (((mchk_logout->lep_lbecr1 & LBECR1_CID) >>
	    LBECR1_CID_SHIFT) == mfpr_whami()))
	{/* 10.1.5 */
	  PRINT_PARSE((" 10.1.5 "));
	  bdatadbe_found++;
	  /* basic 660 entry */
	  sw_flags->error_flags[0] |= SW_FLAG0_BDATADBE; /* 6 */ 
	  sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT |
	    SW_FLAG1_LOG_ADAPT_PRES;
          ERROR_CRASH;
	}
      
      if ( ((mchk_logout->lep_lbecr1 & LBECR1_CA_CMD ) == LBECR1_CA_WRITE) &&
	(((mchk_logout->lep_lbecr1 & LBECR1_CID) >> LBECR1_CID_SHIFT) ==
	 mfpr_whami()))
	  {/* 10.1.5 */
	    PRINT_PARSE((" 10.1.5 "));
	    bdatadbe_found++;
	    /* basic 660 entry */
	    sw_flags->error_flags[0] |= SW_FLAG0_BDATADBE; /* 6 */ 
	    sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT |
	      SW_FLAG1_LOG_ADAPT_PRES;
          ERROR_CRASH;
 	  }
      
      if (bdatadbe_found == 0)
	{/* 10.1.5 */
	  PRINT_PARSE((" 10.1.5 "));
	  sw_flags->error_flags[0] |= SW_FLAG0_BDATADBE |
	    SW_FLAG0_INCON_NSES; /* 6 | 9 */ 
	  sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT |
	    SW_FLAG1_LOG_ADAPT_PRES;
          ERROR_CRASH;
	}
}



ruby_660_bdatasbe(mchk_logout, sw_flags)
     struct ruby_mchk_logout *mchk_logout;
				/* address of the machine check frame */
				/* to use for parsing */
     struct elr_soft_flags *sw_flags;
				/* address of the software flags, */
				/* which are used to return parse */
				/* results and as part of the defined */
				/* error packet */
{
      int bdatasbe_found = 0;
      

      PRINT_PARSE((" BDATASBE error"));
      
      if ( ((mchk_logout->lep_lbecr1 & LBECR1_CA_CMD ) == LBECR1_CA_READ) &&
	(((mchk_logout->lep_lbecr1 & LBECR1_CID) >> LBECR1_CID_SHIFT) !=
	 mfpr_whami()))
	  {/* 10.1.12 */
	    PRINT_PARSE((" 10.1.12 "));
	    bdatasbe_found++;
	    /* basic 660 entry */
	    sw_flags->error_flags[0] |= SW_FLAG0_BDATASBE; /* 5 */ 
	    sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT |
	      SW_FLAG1_LOG_ADAPT_PRES;
	    ERROR_RECOVER;
	  }
      
      if ( ((mchk_logout->lep_lbecr1 & LBECR1_CA_CMD ) ==
	   LBECR1_CA_WRITE_VICTIM) && 
	  (((mchk_logout->lep_lbecr1 & LBECR1_CID) >>
	    LBECR1_CID_SHIFT) == mfpr_whami()))
	{/* 10.1.12 */
	  PRINT_PARSE((" 10.1.12 "));
	  bdatasbe_found++;
	  /* basic 660 entry */
	  sw_flags->error_flags[0] |= SW_FLAG0_BDATASBE; /* 5 */ 
	  sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT |
	    SW_FLAG1_LOG_ADAPT_PRES;
	  ERROR_RECOVER;
	}
      
      if ( ((mchk_logout->lep_lbecr1 & LBECR1_CA_CMD ) == LBECR1_CA_WRITE) &&
	(((mchk_logout->lep_lbecr1 & LBECR1_CID) >> LBECR1_CID_SHIFT) ==
	 mfpr_whami()))
	  {/* 10.1.12 */
	    PRINT_PARSE((" 10.1.12 "));
	    bdatasbe_found++;
	    /* basic 660 entry */
	    sw_flags->error_flags[0] |= SW_FLAG0_BDATASBE; /* 5 */ 
	    sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT |
	      SW_FLAG1_LOG_ADAPT_PRES;
	    ERROR_RECOVER;
	  }
      
      if (bdatasbe_found == 0)
	{/* 10.1.12 */
	  PRINT_PARSE((" 10.1.12 "));
	  sw_flags->error_flags[0] |= SW_FLAG0_BDATASBE |
	    SW_FLAG0_INCON_NSES; /* 5 | 9 */ 
	  sw_flags->error_flags[1] |= SW_FLAG1_LSB_PRESENT |
	    SW_FLAG1_LOG_ADAPT_PRES;
          ERROR_RECOVER;
	}
}


vm_offset_t lsb_addr(node)
	int node;
{
  struct bus *lsbbus;
  struct lsbdata *lsbdata;
  struct lsb_reg *lsbnode;

  /*
   * Get bus number for LSB
   */
  lsbbus = get_sys_bus("lsb");
  if (lsbbus == 0)
	  panic("No system bus configured");
  
  /*
   * Get address of lsbdata structure, get_lsb().
   */
  lsbdata = get_lsb(lsbbus->bus_num);
  
  lsbnode = (struct lsb_reg *)lsbdata->lsbvirt[node];

  return((vm_offset_t)lsbnode);

}



int ruby_parse_630(mchk_logout, sw_flags)
	struct ruby_mchk_logout *mchk_logout;
		/* address of the machine check frame */
		/* to use for parsing */
	struct elr_soft_flags *sw_flags;
		/* address of the software flags, */
		/* which are used to return parse */
		/* results and as part of the defined */
		/* error packet */

{


  PRINT_PARSE(("630 Error Parsing: entered..."));

  if (mchk_logout->biu_stat & BIU_FILL_ECC)
    {
	    if (mchk_logout->biu_stat & BIU_FILL_IRD)
	    {
		    if (mchk_logout->bc_tag & 1 )	/* hit */
		    {
				PRINT_PARSE((" 12.1.1 "));
				sw_flags->error_flags[0] |= 
					SW_FLAG0_ISTRM_BCACHE_SBE;
		    }
		    else
		    {
			    PRINT_PARSE((" 12.1.2 "));
				sw_flags->error_flags[0] |= 
					SW_FLAG0_ISTRM_READ_EDAL_SBE;
		    }
	    }
	    else
	    {
		    if (mchk_logout->bc_tag & 1 )	/* hit */
		    { /* 12.1.3 */
				PRINT_PARSE((" 12.1.3 "));
			    sw_flags->error_flags[0] |= 
					SW_FLAG0_DSTRM_RD_BCACHE_SBE;
		    }
			else
			{ /* 12.1.4 */
			      PRINT_PARSE((" 12.1.4 "));
				  sw_flags->error_flags[0] |= 
					SW_FLAG0_DSTRM_RD_EDAL_SBE;
		        }
	    }
    }
  else
  { /* Inconsistent, (no soft error) */
  }
  

}
