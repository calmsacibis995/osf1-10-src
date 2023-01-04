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
static char *rcsid = "@(#)$RCSfile: kn16aa.c,v $ $Revision: 1.1.8.18 $ (DEC) $Date: 1993/12/21 21:06:27 $";
#endif

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
#include <sys/table.h>
#include <sys/time.h>

#include <machine/reg.h>
#include <machine/entrypt.h>

#include <hal/cpuconf.h>
#include <io/common/devdriver.h>
#include <io/common/iotypes.h>

#include <io/dec/tc/tc.h>
#include <io/dec/tc/ioasic.h>
#include <hal/kn16aa.h>

#include <dec/binlog/binlog.h>
#include <dec/binlog/errlog.h>
#include <io/cam/sim_common.h>
#include <io/cam/dec_cam.h>
#include <io/cam/scsi_all.h>
#include <io/cam/cam.h>
#include <io/cam/dme.h>
#include <io/cam/sim_cirq.h>
#include <io/cam/sim_target.h>
#include <io/cam/sim.h>

static int kn16aa_mchk_ip = 0;     /* set when machine check in progress */
static int kn16aa_correrr_ip = 0;  /* set when corrected error in progress*/
extern int mcheck_expected;	   /* flag used to communicate between badaddr
				   and Mcheck handling */

extern SIM_SOFTC *softc_directory[];

extern struct cpusw *cpup;	/* pointer to cpusw entry */
extern int cold;		/* for cold restart */
extern int printstate;		/* thru console callback or ULTRIX driver */

/* TURBOchannel slot physical addresses */
vm_offset_t kn16aa_slotaddr[TC_IOSLOTS] =
	{ KN16AA_SLOT_0_ADDR, KN16AA_SLOT_1_ADDR, 
	      /* use bogus address for slots that are not there */
	      SLOT_2_ADDR, SLOT_2_ADDR, SLOT_2_ADDR, SLOT_2_ADDR, 
	      KN16AA_SCSI_ADDR, KN16AA_LANCE_ADDR, 
	      KN16AA_SCC_ADDR, KN16AA_ISDN_ADDR, KN16AA_CFB_ADDR
	};
/*
 * Program the order in which to probe the IO slots on the system.
 * This determines the order in which we assign unit numbers to like devices.
 * It also determines how many slots (and what slot numbers) there are to probe.
 * Terminate the list with a -1.
 * Note: this agrees with the console's idea of unit numbers
 */
int kn16aa_config_order[] = { 6, 7, 8, 9, 10, 0, 1, -1 };

/*
 * Define mapping of interrupt lines with the type of interrupt.
 */
static int kn16aa_interrupt_type[INTR_MAX_LEVEL] = {
		INTR_TYPE_NOSPEC,
		INTR_TYPE_SOFTCLK,
		INTR_TYPE_SOFTCLK,
		INTR_TYPE_DEVICE,
		INTR_TYPE_DEVICE,
		INTR_TYPE_HARDCLK,
		INTR_TYPE_OTHER,
		INTR_TYPE_OTHER
};

extern int confdebug;		/* debug variable for configuration code */

int kn16aa_memerrintvl = (60 * 15);	/* 15 minutes */


/*
 * Prototypes DMA support
 */
u_long  kn16aa_dma_map_alloc();
u_long  kn16aa_dma_map_load();
int     kn16aa_dma_map_unload();
int     kn16aa_dma_map_dealloc();
int     kn16aa_dma_min_boundary();

extern  long    direct_map_alloc();
extern  long    direct_map_load();
extern  int     direct_map_unload();
extern  int     direct_map_dealloc();


/*
 * Prototypes
 */
long    kn16aa_read_io_port();
void    kn16aa_write_io_port();
u_long  kn16aa_iohandle_to_phys();
int     kn16aa_io_copyin();
int     kn16aa_io_copyout();
int     kn16aa_io_copyio();
int     kn16aa_io_zero();
/* to be eliminated in gold; They are just here because the IO Switch
 * table defines them.
 */
vm_offset_t     kn16aa_get_io_handle();
int     kn16aa_io_bcopy();



int kn16aa_coreio_mask; 

/* Define debugging stuff. */
#define DEBUG
#ifdef DEBUG
#define Cprintf if(confdebug)printf
#define Dprintf if(confdebug >= 2)printf
#else
#define Cprintf ;
#define Dprintf ;
#endif

static int kn16aa_palcode_type, kn16aa_palcode_version, kn16aa_palcode_revision;
static int kn16aa_firmware_major, kn16aa_firmware_minor;

/*
 * Pelican initialization routine.
 *
 * Must set up the TURBOchannel bus addresses and structures.
 * A lot of this bus configuration is borrowed from mips/kn02.c
 */

kn16aa_init()
{
        extern int tscsiconf();
	extern volatile int *system_intr_cnts_type_transl;

	int	i;

	Cprintf ("***kn16aa_init() entered\n\r");

	/*
	 * Working on cold restart (for autoconf and kern_cpu)
	 */
	cold = 1;
	/*
	 * Setup global interrupt type array to use our definitions.
	 */
	system_intr_cnts_type_transl = kn16aa_interrupt_type;
    	splextreme();

        /*
         * Setup io access jump/function table
         * Note: get_io_handle & io_bcopy need to be cleaned
         *       out in next release
         */
        ioaccess_callsw_init(kn16aa_read_io_port, kn16aa_write_io_port,
                             kn16aa_iohandle_to_phys, kn16aa_io_copyin,
                             kn16aa_io_copyout, kn16aa_io_copyio,
			     kn16aa_io_zero,
                             kn16aa_get_io_handle, kn16aa_io_bcopy);

        /*
         * Setup dma_map_* jump/function table
         */
        dma_callsw_init(kn16aa_dma_map_alloc,kn16aa_dma_map_load,
                        kn16aa_dma_map_unload,kn16aa_dma_map_dealloc,
                        kn16aa_dma_min_boundary);

	/* Initialize the TURBOchannel slots to empty */
	tc_init();

	/* Read the PALcode version/revision */
	(void) kn16aa_read_palcode_version();

	/* Fill in the TURBOchannel slot addresses */
	for (i = 0; i < TC_IOSLOTS; i++)
		tc_slotaddr[i] = kn16aa_slotaddr[i];;

	/* Fill in the TURBOchannel switch table */
	tc_sw.enable_option = kn16aa_enable_option;
	tc_sw.disable_option = kn16aa_disable_option;
	tc_sw.clear_errors = kn16aa_clear_errors;
	tc_sw.config_order = kn16aa_config_order;
	tc_sw.option_control = kn16aa_option_control;

        /*
         * Set up tc_slot for fixed KN16AA I/O devices.
         */

	/* SCSI */
        strcpy(tc_slot[KN16AA_SCSI_INDEX].devname,"tcds");
        strcpy(tc_slot[KN16AA_SCSI_INDEX].modulename, "PMAZ-DS ");
        tc_slot[KN16AA_SCSI_INDEX].slot = 4;
        tc_slot[KN16AA_SCSI_INDEX].module_width = 1;
        tc_slot[KN16AA_SCSI_INDEX].physaddr = KN16AA_SCSI_ADDR;
        tc_slot[KN16AA_SCSI_INDEX].intr_b4_probe = 0;
        tc_slot[KN16AA_SCSI_INDEX].intr_aft_attach = 1;
	tc_slot[KN16AA_SCSI_INDEX].class = TC_ADPT;
        tc_slot[KN16AA_SCSI_INDEX].adpt_config = 0;

	/* Ethernet */
        strcpy(tc_slot[KN16AA_LN_INDEX].devname,"ln");
        strcpy(tc_slot[KN16AA_LN_INDEX].modulename, "PMAD-BA ");
        tc_slot[KN16AA_LN_INDEX].slot = 5;
        tc_slot[KN16AA_LN_INDEX].module_width = 1;
        tc_slot[KN16AA_LN_INDEX].physaddr = KN16AA_LANCE_ADDR;
        tc_slot[KN16AA_LN_INDEX].intr_b4_probe = 0;
        tc_slot[KN16AA_LN_INDEX].intr_aft_attach = 1;
        tc_slot[KN16AA_LN_INDEX].adpt_config = 0;

	/* SCC */
        strcpy(tc_slot[KN16AA_SCC_INDEX].devname,"scc");
        tc_slot[KN16AA_SCC_INDEX].slot = 5;
        tc_slot[KN16AA_SCC_INDEX].module_width = 1;
        tc_slot[KN16AA_SCC_INDEX].physaddr = KN16AA_SCC_ADDR;
        tc_slot[KN16AA_SCC_INDEX].intr_b4_probe = 0;
        tc_slot[KN16AA_SCC_INDEX].intr_aft_attach = 1;
        tc_slot[KN16AA_SCC_INDEX].adpt_config = 0;

	/* ISDN */
	strcpy(tc_slot[KN16AA_ISDN_INDEX].devname,"bba"); /* Baseboard audio */
	tc_slot[KN16AA_ISDN_INDEX].slot = 5;
	tc_slot[KN16AA_ISDN_INDEX].module_width = 1;
	tc_slot[KN16AA_ISDN_INDEX].physaddr = KN16AA_ISDN_ADDR;
	tc_slot[KN16AA_ISDN_INDEX].intr_b4_probe = 0;
	tc_slot[KN16AA_ISDN_INDEX].intr_aft_attach = 0;
	tc_slot[KN16AA_ISDN_INDEX].adpt_config = 0;

	/* CFB */
        strcpy(tc_slot[KN16AA_CFB_INDEX].devname,"fb");
        strcpy(tc_slot[KN16AA_CFB_INDEX].modulename, "PMAGB-BA");
        tc_slot[KN16AA_CFB_INDEX].slot = 6;
        tc_slot[KN16AA_CFB_INDEX].module_width = 1;
        tc_slot[KN16AA_CFB_INDEX].physaddr = KN16AA_CFB_ADDR;
        tc_slot[KN16AA_CFB_INDEX].intr_b4_probe = 0;
        tc_slot[KN16AA_CFB_INDEX].intr_aft_attach = 0;
        tc_slot[KN16AA_CFB_INDEX].adpt_config = 0;

	/* disable all interrupts until they are configured */
	kn16aa_coreio_mask = 0;
	kn16aa_write_simr(kn16aa_coreio_mask);
	/* Reset all interrupts, then enable them all */
	intrsetvec(IO_INTERRUPT, kn16aa_dispatch_iointr, IO_INTERRUPT);

	{
	    unsigned int val;

	    val = kn16aa_read_ssr();
	    
	    val &= ~(0x2000);
	    kn16aa_write_ssr(val);
	}

	Cprintf ("***kn16aa_init() exiting\n\r");
	return(0);
}

/* Return PALcode version, revision, and type */
int
kn16aa_read_palcode_version()
{
	extern struct rpb *rpb;
	struct rpb_percpu *percpu;

	/* get address of the cpu specific data structures for this cpu */
        percpu = (struct rpb_percpu *) ((long)rpb + rpb->rpb_percpu_off);
	kn16aa_palcode_type     = (percpu->rpb_palrev & 0xff0000) >> 16;
	kn16aa_palcode_version  = (percpu->rpb_palrev & 0xff00) >> 8;
	kn16aa_palcode_revision = percpu->rpb_palrev & 0xff;
	kn16aa_firmware_major = (percpu->rpb_firmrev & 0xff00) >> 8;
	kn16aa_firmware_minor = percpu->rpb_firmrev & 0xff;
	return (0);
}

char DEC_3000_300L_STRING[] = "DEC3000 - M300L";
/*
 * Flamingo configuration routine.
 */
kn16aa_conf()
{
	register struct bus *sysbus;
	extern struct rpb *rpb;
	extern char *platform_string();

	Cprintf ("***kn16aa_conf() entered\n");

	/*
	 * Working on cold restart (for autoconf and kern_cpu)
	 */
	cold = 1;
	enable_spls();

	/*
	 * Calls to configure the TURBOchannel bus
	 */
	Cprintf("kn16aa_conf: Calling get_sys_bus\n");
	system_bus = sysbus = get_sys_bus("tc");

	Cprintf("kn16aa_conf: Calling level 1 bus configuration routine\n");
	(*sysbus->confl1)(-1, 0, sysbus);
	Cprintf("kn16aa_conf: Calling level 2 bus configuration routine\n");
	(*sysbus->confl2)(-1, 0, sysbus);

	/* Pelican bounded has bit 10 set in system variation field */
	switch ((rpb->rpb_sysvar >> 10) & (0x3f)) 
	{
	case 0x0:
		break;
	case 0x1:
	    	cpup->system_string = DEC_3000_300L_STRING;
		break;
	default:
	    	cpup->system_string = platform_string();
	}
	printf("%s system\n", cpup->system_string);

	printf ("Firmware revision: %d.%d\n", kn16aa_firmware_major, kn16aa_firmware_minor);
	printf ("PALcode: %s version %d.%02d\n", kn16aa_palcode_type == PALvar_OSF1 ? "OSF":"VMS",
		kn16aa_palcode_version, kn16aa_palcode_revision);


/*
 * Do we need to call cpu_up() in kern/machine.c? (avail_cpus gets inc'ed there)
 */

	master_cpu = 0;
#if	NCPUS > 1
	printf("Master cpu at slot %d.\n", mfpr_whami());
#endif	/* NCPUS > 1 */
	machine_slot[master_cpu].is_cpu = TRUE;
	machine_slot[master_cpu].cpu_type = CPU_TYPE_ALPHA;
	machine_slot[master_cpu].cpu_subtype = CPU_SUBTYPE_DEC_3000_300;
	machine_slot[master_cpu].running = TRUE;
	machine_slot[master_cpu].clock_freq = hz;

	(void) spl0();

	cold = 0;
	Cprintf ("***kn16aa_conf() exiting\n");
	return(0);
}

/*
 * Map I/O space for the I/O module(s) 
 * This is called thru the CPU switch from startup().
 */

/* This maps in the fixed and option TC slots. Unneeded with KSEG. */
kn16aa_map_io()
{
	Cprintf("***kn16aa_map_io() NOP\n");
	return(0);
}

/*
 * Do we need these routines on a Parity based system ????
 */
kn16aa_crderr() {}
kn16aa_memerr() {}


/*
 * Enable CRD and MEMERR interrupts.
 * This runs at regular (15 minute) intervals, turning on the interrupt.
 * Called initially as cpup->timer_action from startup(), then with timeouts.
 * The interrupt is turned off when 3 error interrupts occur in 1 time period.
 * Thus we report at most once per kn16aa_memerrintvl (15 minutes).
 * NOTE: for now, this is a no-op. It is called once from startup().
 */
kn16aa_memenable()
{
	if (kn16aa_memerrintvl > 0)
		timeout (kn16aa_memenable, (caddr_t) 0, kn16aa_memerrintvl * hz);
	return(0);
}

/*
 *
 *   Name: kn16aa_getinfo(item_list) - Gets system specific information
 *
 *   Inputs:    item_list - List of items the caller wants information about.
 *
 *   Outputs:   returned information or NOT_SUPPORTED for each item requested
 *
 *   Return
 *   Values:    NA.
 */
u_int
kn16aa_getinfo(request)
struct item_list *request;
{
	do {
		request->rtn_status = INFO_RETURNED;
		switch(request->function) {
			case GET_TC_SPEED:
				request->output_data = KN16AA_TC_CLK_SPEED;
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
 *   Name: kn16aa_dump_dev() - translate dumpdev into a string the console can understand
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
kn16aa_dump_dev(dump_req)
	struct dump_request *dump_req;

{
	char *device_string;
	char temp_str[8];
	u_int scsi_slot;
	int scsi_cntlr;


	device_string = dump_req->device_name;
/*	sim94_reset(dump_req->device->ctlr_num); */
/*
 *  For Pelican the BOOTED_DEV string is:
 * 	SCSI <tc_no> <slot> <channel> 0 <target*100> 0 <devtype>
 *
 */
	strcpy(device_string,"SCSI ");
	strcpy(&device_string[strlen(device_string)],"0 ");		/* tc_number */

	scsi_slot = dump_req->device->ctlr_hd->bus_hd->slot;
	itoa(scsi_slot,temp_str);
	strcpy(&device_string[strlen(device_string)],temp_str);		/* tc slot number */

	strcpy(&device_string[strlen(device_string)]," ");  

	scsi_cntlr = dump_req->device->ctlr_hd->slot;			/* SCSI A or SCSI B relative to the slot */
	itoa(scsi_cntlr,temp_str);
	strcpy(&device_string[strlen(device_string)],temp_str);
	strcpy(&device_string[strlen(device_string)]," 0 ");

	itoa(dump_req->unit*100,temp_str); 				/* id and lun(lun always zero) */
	strcpy(&device_string[strlen(device_string)],temp_str);

	strcpy(&device_string[strlen(device_string)]," 0 ");  		/* 0=disk; 1=tape */

	if (dump_req->device->ctlr_hd->slot == 4)	/* if its no the internal then tell console that fact */
		strcpy(&device_string[strlen(device_string)],"FLAMG-IO");	/* flaming-io = on board scsi */
	else                                                                                     
		strcpy(&device_string[strlen(device_string)],tc_slot[scsi_slot].modulename);	/* devtype */
 	

}


/* This returns a unique system id by using the last 4 bytes of the ethernet station address */ 
int
kn16aa_system_id()
{
vm_offset_t station_address;
int i;
u_int temp_int;
u_int 	id=0;

	station_address = PHYS_TO_KSEG(0x01a0080000);
	station_address += 8;				/* just last 4 bytes of ether addres */
	for (i=2; i>=0; i--)
		{
		temp_int = (*(u_int *)station_address);
		id |= temp_int & 0xff;
		id <<= 8;
		station_address += 4;
        }
	temp_int = (*(u_int *)station_address);
	id |= temp_int & 0xff;
	return(id);
}

kn16aa_option_control(index, flags)
{
    return (0);
}

kn16aa_enable_option(index)
int index;
{
    switch (index) {
      case KN16AA_SCSI_INDEX:
	/* no masks for SCSI */
	break;
	
      case KN16AA_SCC_INDEX:
	kn16aa_coreio_mask |= SCC_INTR;
	break;

      case KN16AA_LN_INDEX:
	kn16aa_coreio_mask |= LANCE_INTR;
	break;
	
      case KN16AA_ISDN_INDEX:
	kn16aa_coreio_mask |= ISDN_INTR;
	break;

      case TC_OPTION_SLOT_0:
	kn16aa_coreio_mask |= KN16AA_TC_0_INTR;
	break;
	
      case TC_OPTION_SLOT_1:
	kn16aa_coreio_mask |= KN16AA_TC_1_INTR;
	break;

      case KN16AA_CFB_INDEX:
	/* no masks for CFB intr */
	break;
	
      default:
	panic("Enable option called with a non existent index");
	break;
    }

    kn16aa_write_simr(kn16aa_coreio_mask);
    return (0);
}

kn16aa_disable_option(index)
int index;
{
    switch (index) {
      case KN16AA_SCSI_INDEX:
	/* no masks for SCSI */
	break;
	
      case KN16AA_SCC_INDEX:
	kn16aa_coreio_mask &= ~(SCC_INTR);
	break;

      case KN16AA_LN_INDEX:
	kn16aa_coreio_mask &= ~(LANCE_INTR);
	break;
	
      case KN16AA_ISDN_INDEX:
	kn16aa_coreio_mask &= ~(ISDN_INTR);
	break;

      case TC_OPTION_SLOT_0:
	kn16aa_coreio_mask &= ~(KN16AA_TC_0_INTR);
	break;
	
      case TC_OPTION_SLOT_1:
	kn16aa_coreio_mask &= ~(KN16AA_TC_1_INTR);
	break;

      case KN16AA_CFB_INDEX:
	/* no masks for CFB intr */
	break;
	
      default:
	panic("Enable option called with a non existent index");
	break;
    }

    kn16aa_write_simr(kn16aa_coreio_mask);
    return (0);
}

kn16aa_clear_errors()
{
	return (0);
}


unsigned int kn16aa_read_ssr()
{
	mb();
	return (*(unsigned int *)PHYS_TO_KSEG(KN16AA_SSR));
}

void kn16aa_write_ssr(val)
unsigned int val;
{
	*(unsigned int *)PHYS_TO_KSEG(KN16AA_SSR) = val;
	mb();
}

unsigned int kn16aa_read_sir()
{
	mb();
	return (*(unsigned int *)PHYS_TO_KSEG(KN16AA_SIR));
}

void kn16aa_write_sir(val)
unsigned int val;
{
	*(unsigned int *)PHYS_TO_KSEG(KN16AA_SIR) = val;
	mb();
}

unsigned int kn16aa_read_simr()
{
	mb();
	return (*(unsigned int *)PHYS_TO_KSEG(KN16AA_SIMR));
}

void kn16aa_write_simr(val)
unsigned int val;
{
	*(unsigned int *)PHYS_TO_KSEG(KN16AA_SIMR) = val;
	mb();
}

unsigned int kn16aa_read_ir()
{
	mb();
	return ((*(unsigned int *)PHYS_TO_KSEG(KN16AA_IR)) & TCIR_MASK);
}

/* Take a vector 0x800 I/O interrupt and decide who it is for */
void kn16aa_dispatch_iointr()
{
        register unsigned int sir;
	register unsigned int ir;
	register unsigned int found = 0;
	
	/* Read IR to find out who interrupted us */
	ir = kn16aa_read_ir();
	if (ir & 0x4) {
	    found++;
	    (*(tc_slot[KN16AA_CFB_INDEX].intr))(tc_slot[KN16AA_CFB_INDEX].unit);
	}
	if (ir & 0x10) {
	    /* Read SIR and mask out bits that we don't want */
	    sir = kn16aa_read_sir();
	    sir &= kn16aa_read_simr();
	    if (sir & SCC_INTR) {
		found++;
		sccintr(0);
	    }
	    if (sir & LANCE_INTR) {
		found++;
		(*(tc_slot[KN16AA_LN_INDEX].intr))(tc_slot[KN16AA_LN_INDEX].unit);
	    }
	    if (sir & ISDN_INTR) {
		found++;
		(*(tc_slot[KN16AA_ISDN_INDEX].intr))(tc_slot[KN16AA_ISDN_INDEX].unit);
	    }
	    if (sir & KN16AA_TC_0_INTR) {
		found++;
		(*(tc_slot[0].intr))(tc_slot[0].unit);
	    }
	    if (sir & KN16AA_TC_1_INTR) {
		found++;
		(*(tc_slot[1].intr))(tc_slot[1].unit);
	    }
	}
	if (ir & 0x8) {
	    found++;
	    (*(tc_slot[KN16AA_SCSI_INDEX].intr))
					(tc_slot[KN16AA_SCSI_INDEX].unit);
	}
	if (!found) {
	    printf ("Stray interrupt - IR:%x", ir);
	    if (ir & 0x80)
		printf (" ; SIR:%x", sir);
	    printf ("\n");
	}
}


/*
 * Machine check handler.
 * Called from locore thru the cpu switch in response to a trap at SCB 
 * locations: 0x660 (system machine check) and 0x670 (CPU machine check).
 * Also, correctable errors (0x620 and 0x630) come through here...
 * Some support for the structures needed by binary error logging.
 */
kn16aa_machcheck (type, cmcf, framep)
	long type;
	caddr_t cmcf;	/* PHYSICAL!!! */
	long *framep;	/* saved registers - frame pointer */
{
	register int recover = 0;
	register int retry;
	register int byte_count;
	register int clear_check_ip = 0;
	register struct el_kn15aa_logout_header		*mchk_header;
	register struct el_kn15aa_procdata_mcheck	*mchk_procdata;
	register struct el_kn16aa_sysdata_mcheck	*mchk_sysdata;
	struct el_rec					*mchk_elrec;
	struct el_kn16aa_frame_mcheck			 mchk_logout;
	int						 mchk_type, mchk_cputype;
	int	i;

	mchk_header = (struct el_kn15aa_logout_header *) cmcf;
	mchk_procdata = (struct el_kn15aa_procdata_mcheck *)(cmcf + mchk_header->elfl_procoffset - sizeof(mchk_procdata->elfmc_paltemp));
	mchk_sysdata = (struct el_kn16aa_sysdata_mcheck *)(cmcf + mchk_header->elfl_sysoffset);

	/* first see if a Mcheck is expected from badaddr() at this point */
	if (mcheck_expected && mchk_sysdata->elfmc_ident == TC_TIMEOUT_RD) {
		mcheck_expected = 0;	/* Reset flag to indicate Mcheck */
		mtpr_mces(1);		/* Clear MCES register */
		mb ();			/* Just in case, sync things up */
		return(0);
	}

	retry = mchk_header->elfl_retry;
	byte_count = mchk_header->elfl_size;

	/* Build the completed machine check frame */
	bcopy (mchk_header, &mchk_logout.elfmc_hdr, sizeof(mchk_logout.elfmc_hdr));
	bcopy (mchk_procdata, &mchk_logout.elfmc_procdata, sizeof(mchk_logout.elfmc_procdata));
	bcopy (mchk_sysdata, &mchk_logout.elfmc_sysdata, sizeof(mchk_logout.elfmc_sysdata));

	/* Allocate an errlog buffer */
	mchk_elrec = ealloc(sizeof(struct el_kn16aa_frame_mcheck), EL_PRISEVERE);

	/* If we have an errlog record, copy frame in and make it valid */
	if (mchk_elrec != (struct el_rec *)BINLOG_NOBUFAVAIL) {
		switch (type) {
			case SYS_CORR_ERR:	/* System correctable error */
				mchk_type = ELEXTYP_SYS_CORR;
				break;
			case PROC_CORR_ERR:	/* Processor correctable error */
				mchk_type = ELEXTYP_PROC_CORR;
				break;
			case SYS_MCHECK:	/* System machine check abort */
				mchk_type = ELEXTYP_HARD;
				break;
			case PROC_MCHECK:	/* Processor machine check abort */
				mchk_type = ELEXTYP_MCK;
				break;
		}
		mchk_cputype = ELMACH_DEC_3000_300;
		bcopy (&mchk_logout, &mchk_elrec->el_body, sizeof (mchk_logout));
		LSUBID (mchk_elrec, ELCT_EXPTFLT, mchk_type, mchk_cputype, 0, 0, 0);
		binlog_valid(mchk_elrec);
	}

	printstate |= PANICPRINT;


	/* things look bad for the good guys, we weren't planning on a mcheck*/
	kn16aa_consprint (type, &mchk_logout);

	switch (type) {
			/*
			 * The first 2 cases are for errors that have
			 * been corrected.  For this reason the retry
			 * bit in the logout area should be set.  The
			 * purpose of these interrupts is to log the
			 * frequency of occurance.
			 */
		case SYS_CORR_ERR: 	/* System correctable error interrupt */
		case PROC_CORR_ERR:	/* Processor correctable error interrupt */
			kn16aa_correrr_ip++;
			/*
			 * For the case of multiple entry into this routine for
			 * corrected errors simply return because it is 
			 * possible that something this routine is doing is
			 * causing another correctable error.   Since all this
			 * routine does is log these anyways just ignore the
			 * recursive case.
			 */
			if (kn16aa_correrr_ip > 1) {	
				kn16aa_correrr_ip--;
				return(0);
			}
			if (retry == 0) {
				printf("Retry bit not set on corrected err!\n");
			}
			recover = 1;
			/* Log the error */
#ifdef do_not_include
			logcorrected((long *)cmcf, ELMCKT_ADU_COR, cpunum);
#endif do_not_include
			break;
		case SYS_MCHECK: 	/* System machine check abort */
		case PROC_MCHECK: 	/* Processor machine check abort */
			kn16aa_mchk_ip++;
			/*
			 * If a second machine check is detected while a 
			 * machine check is in progress, a Double Error abort
			 * is generated and the processor enters the restart
			 * sequence.  For this reason the following test should
			 * never be true.  
			 */
			if (kn16aa_mchk_ip > 1) {
				panic("Double Error machine check abort.\n");
			}
			clear_check_ip = 1;
			/* Log the error */
#ifdef do_not_include
			logmck((long *)cmcf, ELMCKT_ADU_ABO, cpunum, retry);
#endif do_not_include
       			break;
		default:
			/*
			 * Unrecognized machine check type.  Assume that these
			 * are not recoverable.  Log what you can before the
			 * system goes down.
			 */
			/* Log something here ? */
			recover = 0;
			printf("Unrecognized machine check type %lx.\n",type);
			break;
	}
	/*
	 * For the case of machine checks the PALcode will set an internal
	 * machine check in progress flag.  This flag must be cleared upon
	 * exit of this handler.  Failure to clear this flag will cause a 
	 * Double Error abort in the event of the next machine check.
	 */
	if (clear_check_ip) {
		kn16aa_mchk_ip--;
	}
#ifdef do_not_include
	else {
		kn16aa_correrr_ip--;
	}
#endif

/* for the moment, always panic if an Mcheck happens... */

	panic ("Machine check - Hardware error");
}

/*
 * This is the 'bad address' probe routine for Flamingo. It is passed an
 * address and len and will return 0 if the address exists and
 * a 1 if the address is not existant. This routine only works with locally
 * mapped addresses. Legal lengths are word (4 bytes) or quadword (8 bytes).
 *
 * It is assumed that interrupts are not enabled when this routine is entered
 * and (from the LEP spec) that no interrupts are generated by a non-existant
 * location reference.
 *
 * A global variable is used to announce that an Machine check might occur.
 * mcheck_expected is set to a 1 if an occurance of an mcheck is OK. The mcheck
 * handler clears mcheck_expected to 0 if an Mcheck does occur. This is the
 * communication mechanism that indicates an Mcheck occured. This also will
 * preclude recursive Mchecks from happening.
 */
kn16aa_badaddr(addr,len,ptr)
	vm_offset_t addr;
	int len;
        struct bus_ctlr_common *ptr;
{
	int word_value, bum_address;
	long long_value;
        int stat;

	Cprintf ("***kn16aa_badaddr(addr,len) entered!, addr = %lx, len = %x\n", addr,len);

	if (mcheck_expected)
		panic("mcheck_expected set in probe when unexpected");

	draina();			/* wait for any outstanding aborts */
	mcheck_expected = 1;		/* enter the mcheck_ok region.	*/
	mb ();				/* insure mcheck_expected is written */
	


        if (ptr && ptr->bus_hd && ptr->badaddr) 
         {
            (ptr->badaddr)(addr, len);
         }
        else {

        switch(len) {

        case 4:         /* Requesting a longword probe  */
                addr &= ~(3UL);         /* round address to longword boundary */
                word_value = *(int *)addr;
                break;
        case 8:         /* Requesting a quad probe      */
                addr &= ~(7UL);         /* round address to quadword boundary */
                long_value = *(long *)addr;
                break;
        default:
                addr &= ~(3UL);         /* round address to longword boundary */
                word_value = *(int *)addr;
                break;
           }
       }

	mb ();				/* two mb()'s force things out XXX */
        mb ();				/* make sure mchecks occur prior to
					   clearing flags                  */
        bum_address = !mcheck_expected; /* save if an mcheck occured    */
        mcheck_expected = 0;            /* leave the mcheck_ok region.  */

	if (bum_address) {
		Cprintf ("Address %lx is *not* there!\n", addr);
	} else {
		Cprintf ("Address %lx is really there!\n", addr);
	}

	return(bum_address);
}

/*
 * Print error packet to the console.
 * This is only done when we are about to panic on the error.
 * See also arch/alpha/errlog.c which will replace this code
 * when we have "real" error logging enabled.
 */

kn16aa_consprint(type, cmcf)
	int type;				/* machine check type */
	struct el_kn16aa_frame_mcheck *cmcf; 	/* Pointer to the logout area */
{
	register int i;

	printf("MACHINE CHECK type 0x%x  ",type);

	switch (type) {
		case SYS_MCHECK:     /* System machine check abort */
		case PROC_MCHECK:     /* Processor machine check abort */
			printf("Machine check abort\n");
			/* Print ALPHA fields */
			for (i = 0; i < 32; i+=2) {
				printf("\tptr[%d-%d]\t= %l016x %l016x\n",
				       i, i+1, cmcf->elfmc_procdata.elfmc_paltemp[i], 
				       cmcf->elfmc_procdata.elfmc_paltemp[i+1]);
			}
			/* Print EV4 fields */
			printf("\texc_addr\t= %l016x\n",cmcf->elfmc_procdata.elfmc_exc_addr);
			printf("\texc_sum\t\t= %l016x\n",cmcf->elfmc_procdata.elfmc_exc_sum);
			printf("\texc_mask\t= %l016x\n",cmcf->elfmc_procdata.elfmc_exc_mask);
			printf("\ticcsr\t\t= %l016x\n",cmcf->elfmc_procdata.elfmc_iccsr);
			printf("\tpal_base\t= %l016x\n",cmcf->elfmc_procdata.elfmc_pal_base);
			printf("\thier\t\t= %l016x\n",cmcf->elfmc_procdata.elfmc_hier);
			printf("\thirr\t\t= %l016x\n",cmcf->elfmc_procdata.elfmc_hirr);
			printf("\tmm_csr\t\t= %l016x\n",cmcf->elfmc_procdata.elfmc_mm_csr);
			printf("\tdc_stat\t\t= %l016x\n",cmcf->elfmc_procdata.elfmc_dc_stat);
			printf("\tdc_addr\t\t= %l016x\n",cmcf->elfmc_procdata.elfmc_dc_addr);
			printf("\tabox_ctl\t= %l016x\n",cmcf->elfmc_procdata.elfmc_abox_ctl);
			printf("\tbiu_stat\t= %l016x\n",cmcf->elfmc_procdata.elfmc_biu_stat);
			printf("\tbiu_addr\t= %l016x\n",cmcf->elfmc_procdata.elfmc_biu_addr);
			printf("\tbiu_ctl\t\t= %l016x\n",cmcf->elfmc_procdata.elfmc_biu_ctl);
			printf("\tfill_syndrome\t= %l016x\n",cmcf->elfmc_procdata.elfmc_fill_syndrome);
			printf("\tfill_addr\t= %l016x\n",cmcf->elfmc_procdata.elfmc_fill_addr);
			printf("\tva\t\t= %l016x\n",cmcf->elfmc_procdata.elfmc_va);
			printf("\tbc_tag\t\t= %l016x\n",cmcf->elfmc_procdata.elfmc_bc_tag);

			/* Print Pelican fields (really 32-bit registers) */
        		printf("\tident\t\t= %lx\n", (unsigned int)cmcf->elfmc_sysdata.elfmc_ident);
        		printf("\tmcr_stat\t= %l08x\n", (unsigned int)cmcf->elfmc_sysdata.elfmc_mcr_stat);
        		printf("\tintr\t\t= %l08x\n", (unsigned int)cmcf->elfmc_sysdata.elfmc_intr);
        		printf("\ttc_status\t= %l08x\n", (unsigned int)cmcf->elfmc_sysdata.elfmc_tc_status);
        		printf("\tconfig\t\t= %l08x\n", (unsigned int)cmcf->elfmc_sysdata.elfmc_config);
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
 *  Read a standard Alpha TODR
 */
long
kn16aa_readtodr()
{
	struct tm tm;
	long	rc = 0;
	int	i, s;
	volatile struct	kn16aa_rtc	*rtc;

	Cprintf ("***kn16aa_readtodr() entered\n");

	s = splextreme();	/* Can this be splhigh? */

	rtc = (struct kn16aa_rtc *)PHYS_TO_KSEG(KN16AA_TOY_ADDR);

	while (rtc->rega & TOY_REGA_UIP) {	/* update in progress */
		splx(s);
		DELAY(10);
		s = splextreme();
	}
	if ((rtc->regb & TOY_REGB_DM) == 0) {
		printf ("WARNING: Resetting TOY from BCD mode - must set time manually\n");
		rtc->regb |= TOY_REGB_DM;
		return (0);
	}
	if ((rtc->regb & TOY_REGB_24) == 0) {
		printf ("WARNING: Resetting TOY from 12-hour mode - must set time manually\n");
		rtc->regb |= TOY_REGB_24;
		return (0);
	}

	Cprintf ("Fields are: %d:%d:%d %d/%d/%d\n",
		rtc->hours, rtc->minutes, rtc->seconds,
		rtc->month, rtc->dom, rtc->year);

	if (rtc->year != YRREF-1900) {
		return (0);
	}

	tm.tm_year = rtc->year;
	tm.tm_mon = rtc->month - 1;	/* 0..11, not 1..12 */
	tm.tm_mday = rtc->dom;
	tm.tm_hour = rtc->hours;
	tm.tm_min = rtc->minutes;
	tm.tm_sec = rtc->seconds;
	rc = cvt_tm_to_sec(&tm);

	rc *= UNITSPERSEC;	/* Scale to 100ns ticks */
	rc += TODRZERO;		/* Indicate a valid time */

	Cprintf ("***kn16aa_readtodr() exited\n");
	splx(s);
        return (rc);
}

/*
 *  Write a standard Alpha TODR
 */
kn16aa_writetodr(yrtime)
	long yrtime;	/* Was u_int, but it seems that it should be long */
{
	struct tm tm;
	volatile struct	kn16aa_rtc	*rtc;

	Cprintf ("***kn16aa_writetodr(%lx) entered\n", yrtime);

	cvt_sec_to_tm (yrtime, &tm);
	rtc = (struct kn16aa_rtc *)PHYS_TO_KSEG(KN16AA_TOY_ADDR);
	rtc->year = tm.tm_year;
	rtc->month = tm.tm_mon + 1;	/* 1..12, not 0..11 */
	rtc->dom = tm.tm_mday;
	rtc->hours = tm.tm_hour;
	rtc->minutes = tm.tm_min;
	rtc->seconds = tm.tm_sec;
	Cprintf ("Fields are: %d:%d:%d %d/%d/%d\n",
		rtc->hours, rtc->minutes, rtc->seconds,
		rtc->month, rtc->dom, rtc->year);

	Cprintf ("***kn16aa_writetodr() exited\n");
}

#define MY_PHYS_TO_KSEG(addr)	((vm_offset_t)(addr) + 0xfffffc0000000000)
#define MY_DELAY(n)	{ volatile int i = 30 * (n); while (i--); }

kn16aa_write_leds(n)
unsigned char n;
{
    int *leds_addr;
    int temp = 0;

    leds_addr = (int *)MY_PHYS_TO_KSEG(0x1f0000018);

    n = ~n;
    temp = (n & 0xf) << 20;
    temp |= (n & 0xf0) << 12;

    *leds_addr = 0xff0000;
    mb();

    *leds_addr = temp;
    mb();

    MY_DELAY(1000000);
}






/*
 *  These functions are being removed from the Alpha IO Architecture.
 *  They exist here only as place holders.
 */



vm_offset_t
kn16aa_get_io_handle (dev_addr, width, type)

io_handle_t     dev_addr;
int             width;
int             type;
{  /* Begin kn16aa_get_io_handle */
        panic("DEC3000_300 will not implement get_io_handle()");
}  /* End kn16aa_get_io_handle */




/***************************************************************************/
/*                                                                         */
/* kn16aa_io_zero   -  Zero fill at the bus io destination address for     */
/*                     length byte_count.                                  */
/*                                                                         */
/* SYNOPSIS                                                                */
/*      int  kn16aa_io_zero (dstaddr, byte_count)                          */
/*                                                                         */
/*      io_handle_t     dstaddr;                                           */
/*      int             byte_count;                                        */
/*                                                                         */
/*                                                                         */
/* PARAMETERS                                                              */
/*                                                                         */
/*      dstaddr         This is the SYSMAP IO handle of the base system    */
/*                      combined with the natural address of the IO space  */
/*                      the copy is destined for.                          */
/*                                                                         */
/*      byte_count      Number of bytes in block to be copied.             */
/*                                                                         */
/*                                                                         */
/*                                                                         */
/* DESCRIPTION                                                             */
/*                                                                         */
/*  IO space at the destination address supplied is zeroed for the         */
/*  byte count specified.                                                  */
/*                                                                         */
/***************************************************************************/
#define PATTERN_ZERO 0x00

int
kn16aa_io_zero( dstaddr, byte_count)
   io_handle_t  dstaddr;
   register u_long       byte_count;
{
   register u_int      ret_val;
   register unsigned long  cur_io_p;
   register unsigned long  dstaddr_alignment;
   union  buffer tbuffer;

   ret_val = IOA_OKAY;
   /* Convert iohandle_t into a sparse space address */
   /* For birds iohandle sparse address walks ints by two */
   cur_io_p = ((dstaddr & KN16_IOHANDLE_MASK) |
                      (dstaddr & MASK_LONG_ADDRESS & KN16_DENSE_ADDRESS_MASK)*2);
   dstaddr_alignment = dstaddr & ALIGNMENT_MASK;
   /* -------------------------------------------------- */
   /* Source Address Aligned Destination Address Aligned */
   /* -------------------------------------------------- */
     if(dstaddr_alignment == ALIGNED ) {
       while(byte_count >= TC_LONGWORD) {
         (*((int *)cur_io_p)) = PATTERN_ZERO;
         byte_count -= 4;
         (int *)cur_io_p += 2;
         }
       /* clean up the remaining buffer transfer of 1 to 3 bytes. */
       if(byte_count == TRIBYTE_REMAINDER) {
         (*(long *)(cur_io_p)) = PATTERN_ZERO | TC_TRIBYTE_REMAINDER_MASK_LOW;
         }
       else {
         if (byte_count == WORD_REMAINDER ){
           (*(long *)(cur_io_p)) = PATTERN_ZERO | TC_WORD_REMAINDER_MASK_WORD1;
           }
         else {
           if (byte_count == BYTE_REMAINDER ){
             (*(long *)(cur_io_p)) = PATTERN_ZERO | TC_BYTE_REMAINDER_MASK_CHAR1;
             }
           }
         }
       }
     /* ----------------------------------------------------*/
     /* Source Address Aligned Destination Address !Aligned */
     /* ----------------------------------------------------*/
     else {
       /* Do the copy based on the destination TC alignment */
       switch(dstaddr_alignment) {

         case TRIBYTE_OFFSET: /* Source aligned !destination on byte offset  */
           /* Align the Destination TC address */
           (*((long *)(cur_io_p))) =  PATTERN_ZERO | TC_BYTE_REMAINDER_MASK_CHAR4;
           (int *)cur_io_p += 2;
           byte_count -= 1;
           tbuffer.LW = 0;
           /* Write LW to TC IO space until done */
           while(byte_count >= TC_LONGWORD) {
             (*((int *)(cur_io_p))) = tbuffer.INT.IW = PATTERN_ZERO;
             (int *)cur_io_p += 2;
             byte_count -= 4;
             }
           /* Write copy remainer to TC IO space */
           /* clean up the remaining buffer transfer of 1 to 3 bytes. */
           if(byte_count == TRIBYTE_REMAINDER) {
             (*((long *)(cur_io_p))) = PATTERN_ZERO | TC_TRIBYTE_REMAINDER_MASK_LOW ;
             }
           else {
             if (byte_count == WORD_REMAINDER ){
               (*((long *)(cur_io_p))) =  PATTERN_ZERO | TC_WORD_REMAINDER_MASK_WORD1;
                }
             else {
                if(byte_count == BYTE_REMAINDER) {
                 (*((long *)(cur_io_p))) = PATTERN_ZERO | TC_BYTE_REMAINDER_MASK_CHAR1;
                 }
               }
             }
           break;
         case WORD_OFFSET: /* Source address aligned !destination... */
           /* Align the Destination TC address */
           (*((long *)(cur_io_p))) = PATTERN_ZERO | TC_WORD_REMAINDER_MASK_WORD2;
           (int *)cur_io_p += 2;
           byte_count -= 2;
           /* Write LW to TC IO space until done */
           while(byte_count >= TC_LONGWORD) {
             (*((int *)(cur_io_p))) = tbuffer.INT.IW = PATTERN_ZERO;
             (int *)cur_io_p += 2;
             byte_count -= 4;
             }
           /* Write copy remainer to TC IO space */
           /* clean up the remaining buffer transfer of 1 to 3 bytes. */
           if(byte_count == TRIBYTE_REMAINDER) {
             (*((long *)(cur_io_p))) =  PATTERN_ZERO | TC_TRIBYTE_REMAINDER_MASK_LOW;
             }
           else {
             if (byte_count == WORD_REMAINDER ){
               (*((long *)(cur_io_p))) = PATTERN_ZERO | TC_WORD_REMAINDER_MASK_WORD1;
               }
             else {
               if(byte_count == BYTE_REMAINDER){
                 (*((long *)(cur_io_p))) = PATTERN_ZERO | TC_BYTE_REMAINDER_MASK_CHAR1;
                 }
               }
             }
           break;

         case BYTE_OFFSET: /* Source address aligned !destination... */
           /* Align the Destination TC address */
           *((long *)(cur_io_p)) = PATTERN_ZERO | TC_TRIBYTE_REMAINDER_MASK_HIGH;
           (int *)cur_io_p += 2;
           byte_count -= 3;
           /* Write LW to TC IO space until done */
           while(byte_count >= TC_LONGWORD) {
             (*((int *)(cur_io_p))) = tbuffer.INT.IW = PATTERN_ZERO;
             (int *)cur_io_p += 2;
             byte_count -= 4;
             }
           /* Write copy remainer to TC IO space */
           /* clean up the remaining buffer transfer of 1 to 3 bytes. */
           if(byte_count == TRIBYTE_REMAINDER) {
             (*((long *)(cur_io_p))) =  PATTERN_ZERO | TC_TRIBYTE_REMAINDER_MASK_LOW;
             }
           else {
             if (byte_count == WORD_REMAINDER ){
               (*((long *)(cur_io_p))) =  PATTERN_ZERO | TC_WORD_REMAINDER_MASK_WORD1;
               }
             else {
               if(byte_count == BYTE_REMAINDER) {
                 (*((long *)(cur_io_p))) = PATTERN_ZERO | TC_BYTE_REMAINDER_MASK_CHAR1;
                 }
               }
             }
           break;
         default: /* Here for good form... aYup! */
           break;
         }
       } /*  Dstaddr !Aligned */
   return (ret_val);

}  /* End kn16aa_io_zero  */

int
kn16aa_io_bcopy (srcaddr, dstaddr, byte_count)
caddr_t srcaddr;
caddr_t dstaddr;
int     byte_count;
{  /* Begin kn16aa_io_bcopy */
        panic("DEC3000_300 will not implement io_bcopy()");
}  /* End kn16aa_io_bcopy */





 /*
  * Alpha OSF IO Architecture Support.
  *
  */


/***************************************************************************/
/*                                                                         */
/* kn16_iohandle_to_phys()  This routine returns the address passed to it. */
/*                                                                         */
/*                                                                         */
/* SYNOPSIS                                                                */
/*      u_long   kn16_iohandle_to_phys (dev_addr, width, type)             */
/*                                                                         */
/*      io_handle_t     io_handle;                                         */
/*      long            flags;                                             */
/*                                                                         */
/*                                                                         */
/* PARAMETERS                                                              */
/*      io_handle       io_handle to be translated into a dense space      */
/*                      address.                                           */
/*                                                                         */
/*      flags           tbd                                                */  
/*                                                                         */
/* DESCRIPTION                                                             */
/*       This function always returns a dense space address for the        */
/*   bird machines.                                                        */
/*                                                                         */
/***************************************************************************/ 

u_long 
kn16aa_iohandle_to_phys(io_handle, flags)

   io_handle_t  io_handle;
   long         flags;
{
   return((io_handle - KN16_DENSE_SPACE));
}


/***************************************************************************/
/*                                                                         */
/* kn16_read_io_port   - Reads data from device register located in turbo  */
/*                       channel "Sparse" address space.                   */
/*                                                                         */
/* SYNOPSIS                                                                */
/*      long  kn16_read_io_port (dev_addr, width, type)                    */
/*                                                                         */
/*      io_handle_t     devaddr;                                           */
/*      int             width;                                             */
/*      int             type;                                              */
/*                                                                         */
/*                                                                         */
/* PARAMETERS                                                              */
/*      devaddr         IO handle from the device driver.                  */
/*                                                                         */
/*      width           Specifies the width in bytes of the data to be     */
/*                      read. Supported values are 1, 2 and 4.             */
/*                      Note: The turbo channel cannot do an Atomic        */
/*                      Quadword data operation and therfore does not      */
/*                      support this feature. Inaddition, Tribyte          */
/*                      operations are not not supported on the Flamingo   */
/*                      Sandpiper or the Pelican.                          */
/*                                                                         */
/*      type            This field is not used on the Flamingo, Sandpiper  */
/*                      or the Pelican.                                    */
/*                                                                         */
/* NOTE: Values for type are defined in <io/common/devdriver.h>            */
/*       For performance reasons no checking of the dev_adr is done.       */
/***************************************************************************/

long 
kn16aa_read_io_port (dev_addr, width, type)

io_handle_t    dev_addr;
int             width;
int             type;

{  /* Begin kn16_read_io_port */

    volatile long Final_Address;
    volatile long Mask_Register;
    register int Mask_Register_Temp;
    register u_int loc_data;
    register u_int Mask_Value;
    register u_int Mask_Value_Temp;
    register u_int offset;
    register u_int s;

    offset = dev_addr & 3;
    /* For birds iohandle sparse address * 2 */
    Final_Address = ( (dev_addr & KN16_IOHANDLE_MASK) |
                       ((dev_addr & MASK_LONG_ADDRESS & KN16_DENSE_ADDRESS_MASK)*2));

    if (width == LONG_ACCESS) {  /* Longword Operation, no mask register necessary */
       loc_data = *(int *)(Final_Address);
       return (loc_data);
      }
    else{ if (width == WORD_ACCESS)  /* Word Operation */
            if (offset == 0)
              Mask_Value_Temp = KN16_READ_W_MASK0;
            else
              Mask_Value_Temp = KN16_READ_W_MASK1;
          else                       /* Byte Operation */
            switch (offset) {
              case 00:
                Mask_Value_Temp = KN16_READ_B_MASK0;
                break;
              case 01:
                Mask_Value_Temp = KN16_READ_B_MASK1;
                break;
              case 02:
                Mask_Value_Temp = KN16_READ_B_MASK2;
                break;
              case 03:
                Mask_Value_Temp = KN16_READ_B_MASK3;
                break;
              default:
                break;
              }

          Mask_Register =  KN16_MASK_REGISTER_ADDRESS;
          s = splextreme();
          Mask_Value = (*(int *)Mask_Register |  Mask_Value_Temp | 16 );  
          *(int *) Mask_Register = Mask_Value;
          mb();

          loc_data = *(int *)Final_Address;
          Mask_Value = *(int *)Mask_Register;
          *(int *)Mask_Register = (Mask_Value & KN16_CLEAR_MASK );
          mb();
          splx(s);

          /* Return Data in C structure definition requested by caller. */
          return (BUILD_RETURN_STRUCTURE(loc_data,offset,width));
      }

}  /* End kn16_read_io_port */


/***************************************************************************/
/*                                                                         */
/* kn16_write_io_port   - Write data to a turbo channel device register.   */
/*                                                                         */
/*                                                                         */
/* SYNOPSIS                                                                */
/*      long  kn16_write_io_port (dev_addr, width, type, data )            */
/*                                                                         */
/*      io_handle_t     devaddr;                                           */
/*      int             width;                                             */
/*      int             type;                                              */
/*      long            data;                                              */
/*                                                                         */
/*                                                                         */
/* PARAMETERS                                                              */
/*      devaddr         IO handle to be written.                           */
/*                                                                         */
/*      width           Specifies the width in bytes of the data to be     */
/*                      written. Supported values are 1, 2 and 4.          */
/*                      Octal and tri byte operations not supported.       */
/*                                                                         */
/*      type            This field is not used on the Flamingo, Sandpiper  */
/*                      and Pelican.                                       */
/*                                                                         */
/*      data            The data value to be written.                      */
/*                                                                         */
/* NOTE: Values for type are defined in <io/common/devdriver.h>            */
/*       For performance reasons no checking of the dev_adr is done.       */
/***************************************************************************/

void
kn16aa_write_io_port (dev_addr, width, type, data)
io_handle_t     dev_addr;
int             width;
int             type;
long            data;

{  /* Begin kn16_write_io_port */

    register long Final_Address;
    register long loc_data;
    register int offset;
    register int s;

    offset = dev_addr & 3;
    /* For birds iohandle sparse address * 2 */
    Final_Address = ( (dev_addr & KN16_IOHANDLE_MASK) |
                       (dev_addr & MASK_LONG_ADDRESS & KN16_DENSE_ADDRESS_MASK)*2);

    if (width == LONG_ACCESS) {  /* Longword Operation, No mask necessary */
       data = ((data & 0xffffffff) | KN16_WRITE_LW_MASK);
      }
    else{ if (width == WORD_ACCESS) {      /* Word Operation */
           data = (((data & 0xffff) <<
                      (offset*8)) | (KN16_WRITE_W_MASK << (offset)));
            }
          else {                            /* Byte Operation */
            data = (((data & 0xff) <<
                      (offset*8)) |  (KN16_WRITE_B_MASK << (offset)));
            }
    }
    *(long*)Final_Address = data;
}  /* End kn16_write_io_port */









/***************************************************************************/
/*                                                                         */
/* kn16aa_io_copyout  -  Copy a block of byte contigous Memory to IO       */
/*                       space.                                            */
/*                                                                         */
/* SYNOPSIS                                                                */
/*      int  kn16aa_io_copyout (srcaddr, dstaddr, byte_count)              */
/*                                                                         */
/*      vm_address_t    srcaddr;                                           */
/*      io_handle_t     dstaddr;                                           */
/*      int             byte_count;                                        */
/*                                                                         */
/*                                                                         */
/* PARAMETERS                                                              */
/*      srcaddr         Kernel Virtual address of the Memory block to      */
/*                      be copied from.                                    */
/*                                                                         */
/*      dstaddr         This is the  IO handle of the base system          */
/*                      combined with the physical address of the IO space */
/*                      the copy is destined for.                          */
/*                                                                         */
/*      byte_count      Number of bytes in block to be copied.             */
/*                                                                         */
/*   Note: Neither the srcaddr or the dstaddr assume any alignment.        */
/*                                                                         */
/*                                                                         */
/* DESCRIPTION                                                             */
/*    Pcode: io_copyout() - Trick, getting to "lw" IO write operations     */
/*                          for the bulk of the transfer.                  */
/*                                                                         */
/***************************************************************************/

int
kn16aa_io_copyout(srcaddr, dstaddr, byte_count)
   vm_offset_t  srcaddr;
   io_handle_t  dstaddr;
   u_long       byte_count;
{
   int      ret_val;
   unsigned long  cur_mem_p;
   unsigned long  cur_io_p;
   unsigned long  srcaddr_alignment, dstaddr_alignment;
   union  buffer tbuffer;

   tbuffer.LW  = 0;
   ret_val = IOA_OKAY ; 
   /* Convert iohandle_t into a sparse space address */
   /* For birds iohandle sparse address walks ints by two */
   cur_io_p = ((dstaddr & KN16_IOHANDLE_MASK) |
                      (dstaddr & MASK_LONG_ADDRESS & KN16_DENSE_ADDRESS_MASK)*2);
   srcaddr_alignment = srcaddr & ALIGNMENT_MASK;
   dstaddr_alignment = dstaddr & ALIGNMENT_MASK;
   cur_mem_p = srcaddr;
   /* -------------------------------------------------- */
   /* Source Address Aligned Destination Address Aligned */
   /* -------------------------------------------------- */
   if(srcaddr_alignment == ALIGNED ) {
     if(dstaddr_alignment == ALIGNED ) {
       while(byte_count >= TC_LONGWORD) {
         (*((int *)cur_io_p)) = (*((int *)cur_mem_p));
         byte_count -= 4;
         (int *)cur_io_p += 2;
         (int *)cur_mem_p += 1;
         }
       /* clean up the remaining buffer transfer of 1 to 3 bytes. */
       if(byte_count == TRIBYTE_REMAINDER) {
         tbuffer.WORD_BYTES.WORD1 = *((short *)(cur_mem_p));
         (unsigned char *)cur_mem_p += 2;
         tbuffer.WORD_BYTES.CHAR1 = *((char *)(cur_mem_p));
         tbuffer.LW = tbuffer.LW | TC_TRIBYTE_REMAINDER_MASK_LOW;
         (*(long *)(cur_io_p)) = tbuffer.LW;
         }
       else {
         if (byte_count == WORD_REMAINDER ){
           tbuffer.WORDS.WORD1 = *((short *)(cur_mem_p));
           tbuffer.LW = tbuffer.LW | TC_WORD_REMAINDER_MASK_WORD1;
           (*(long *)(cur_io_p)) = tbuffer.LW;
           }
         else {
           if (byte_count == BYTE_REMAINDER ){

             tbuffer.BYTES.CHAR1 = *((char *)(cur_mem_p));
             tbuffer.LW = tbuffer.LW | TC_BYTE_REMAINDER_MASK_CHAR1;
             (*(long *)(cur_io_p)) = tbuffer.LW;
             }
           }
         }
       }
     /* ----------------------------------------------------*/
     /* Source Address Aligned Destination Address !Aligned */
     /* ----------------------------------------------------*/
     else {
       /* Do the copy based on the destination TC alignment */
       switch(dstaddr_alignment) {

         case TRIBYTE_OFFSET: /* Source aligned !destination on byte offset  */
           /* Align the Destination TC address */
           tbuffer.BYTES.CHAR4 = (*((char *)(cur_mem_p)));
           (char *)cur_mem_p += 1;
           tbuffer.LW = tbuffer.LW | TC_BYTE_REMAINDER_MASK_CHAR4;
           (*((long *)(cur_io_p))) = tbuffer.LW;
           (int *)cur_io_p += 2;
           byte_count -= 1;
           tbuffer.LW = 0;

           /* Write LW to TC IO space until done */
           while(byte_count >= TC_LONGWORD) {
             tbuffer.BYTES.CHAR1 = (*((char *)(cur_mem_p)));
             (char *)cur_mem_p += 1;
             tbuffer.BYTES.CHAR2 = (*((char *)(cur_mem_p)));
             (char *)cur_mem_p += 1;
             tbuffer.BYTES.CHAR3 = (*((char *)(cur_mem_p)));
             (char *)cur_mem_p += 1;
             tbuffer.BYTES.CHAR4 = (*((char *)(cur_mem_p)));
             (char *)cur_mem_p += +1;
             (*((int *)(cur_io_p))) = tbuffer.INT.IW;
             (int *)cur_io_p += 2;
             byte_count -= 4;
             }
           /* Write copy remainer to TC IO space */
           tbuffer.LW = 0;
           /* clean up the remaining buffer transfer of 1 to 3 bytes. */
           if(byte_count == TRIBYTE_REMAINDER) {
             tbuffer.BYTES.CHAR1 = (*((char *)(cur_mem_p)));
             (char *)cur_mem_p += 1;
             tbuffer.BYTES.CHAR2 = (*((char *)(cur_mem_p)));
             (char *)cur_mem_p += 1;
             tbuffer.BYTES.CHAR3 = (*((char *)(cur_mem_p)));
             tbuffer.LW = tbuffer.LW | TC_TRIBYTE_REMAINDER_MASK_LOW;
             (*((long *)(cur_io_p))) = tbuffer.LW;
             }
           else {
             if (byte_count == WORD_REMAINDER ){
               tbuffer.BYTES.CHAR1 = (*((char *)(cur_mem_p)));
               (char *)cur_mem_p += 1;
               tbuffer.BYTES.CHAR2 = (*((char *)(cur_mem_p)));
               tbuffer.LW = tbuffer.LW | TC_WORD_REMAINDER_MASK_WORD1;
               (*((long *)(cur_io_p))) = tbuffer.LW;
                }
             else {
                if(byte_count == BYTE_REMAINDER) {
                 tbuffer.BYTES.CHAR1 = (*((char *)(cur_mem_p)));
                 tbuffer.LW = tbuffer.LW | TC_BYTE_REMAINDER_MASK_CHAR1 ;
                 (*((long *)(cur_io_p))) = tbuffer.LW;
                 }
               }
             }
           break;
         case WORD_OFFSET: /* Source address aligned !destination... */
           /* Align the Destination TC address */
           tbuffer.WORDS.WORD2 = (*((short *)(cur_mem_p)));
           (char *)cur_mem_p += 2;
           tbuffer.LW = tbuffer.LW | TC_WORD_REMAINDER_MASK_WORD2;
           (*((long *)(cur_io_p))) = tbuffer.LW;
           (int *)cur_io_p += 2;
           byte_count -= 2;
           tbuffer.LW = 0;
           /* Write LW to TC IO space until done */
           while(byte_count >= TC_LONGWORD) {
             tbuffer.WORDS.WORD1 = (*((short *)(cur_mem_p)));
             (char *)cur_mem_p += 2;
             tbuffer.WORDS.WORD2 = (*((short *)(cur_mem_p)));

             (char *)cur_mem_p += 2;
             (*((int *)(cur_io_p))) = tbuffer.INT.IW;
             (int *)cur_io_p += 2;
             byte_count -= 4;
             }
           /* Write copy remainer to TC IO space */
           tbuffer.LW = 0;
           /* clean up the remaining buffer transfer of 1 to 3 bytes. */
           if(byte_count == TRIBYTE_REMAINDER) {
             tbuffer.BYTES.CHAR1 = (*((char *)(cur_mem_p)));
             (char *)cur_mem_p += 1;
             tbuffer.BYTES.CHAR2 = (*((char *)(cur_mem_p)));
             (char *)cur_mem_p += 1;
             tbuffer.BYTES.CHAR3 = (*((char *)(cur_mem_p)));
             tbuffer.LW = tbuffer.LW | TC_TRIBYTE_REMAINDER_MASK_LOW;
             (*((long *)(cur_io_p))) = tbuffer.LW;
             }
           else {
             if (byte_count == WORD_REMAINDER ){
               tbuffer.WORDS.WORD1 = (*((short *)(cur_mem_p)));
               tbuffer.LW = tbuffer.LW | TC_WORD_REMAINDER_MASK_WORD1;
               (*((long *)(cur_io_p))) = tbuffer.LW;
               }
             else {
               if(byte_count == BYTE_REMAINDER){
                 (char *)cur_mem_p += 1;
                 tbuffer.BYTES.CHAR1 = (*((char *)(cur_mem_p)));
                 tbuffer.LW = tbuffer.LW | TC_BYTE_REMAINDER_MASK_CHAR1;
                 (*((long *)(cur_io_p))) = tbuffer.LW;
                 }
               }
             }
           break;

         case BYTE_OFFSET: /* Source address aligned !destination... */
           /* Align the Destination TC address */
           tbuffer.BYTES.CHAR2 = (*((char *)(cur_mem_p)));
           (char *)cur_mem_p += 1;
           tbuffer.WORDS.WORD2 = (*((short *)(cur_mem_p)));
           (char *)cur_mem_p += 2;
           tbuffer.LW = tbuffer.LW | TC_TRIBYTE_REMAINDER_MASK_HIGH;
           *((long *)(cur_io_p)) = tbuffer.LW;
           (int *)cur_io_p += 2;
           byte_count -= 3;
           tbuffer.LW = 0;
           /* Write LW to TC IO space until done */
           while(byte_count >= TC_LONGWORD) {
             tbuffer.BYTES.CHAR1 = (*((char *)(cur_mem_p)));
             (char *)cur_mem_p += 1;
             tbuffer.BYTES.CHAR2 = (*((char *)(cur_mem_p)));
             (char *)cur_mem_p += 1;
             tbuffer.BYTES.CHAR3 = (*((char *)(cur_mem_p)));
             (char *)cur_mem_p += 1;
             tbuffer.BYTES.CHAR4 = (*((char *)(cur_mem_p)));
             (char *)cur_mem_p += 1;
             (*((int *)(cur_io_p))) = tbuffer.INT.IW;
             (int *)cur_io_p += 2;

             byte_count -= 4;
             }
           /* Write copy remainer to TC IO space */
           tbuffer.LW = 0;
           /* clean up the remaining buffer transfer of 1 to 3 bytes. */
           if(byte_count == TRIBYTE_REMAINDER) {
             tbuffer.BYTES.CHAR1 = *((char *)(cur_mem_p));
             (char *)cur_mem_p += 1;
             tbuffer.BYTES.CHAR2 = *((char *)(cur_mem_p));
             (char *)cur_mem_p += 1;
             tbuffer.BYTES.CHAR3 = *((char *)(cur_mem_p));
             tbuffer.LW = tbuffer.LW | TC_TRIBYTE_REMAINDER_MASK_LOW;
             (*((long *)(cur_io_p))) = tbuffer.LW;
             }
           else {
             if (byte_count == WORD_REMAINDER ){
               tbuffer.BYTES.CHAR1 = (*((char *)(cur_mem_p)));
               (char *)cur_mem_p += 1;
               tbuffer.BYTES.CHAR2 = (*((char *)(cur_mem_p)));
               tbuffer.LW = tbuffer.LW | TC_WORD_REMAINDER_MASK_WORD1;
               (*((long *)(cur_io_p))) = tbuffer.LW;
               }
             else {
               if(byte_count == BYTE_REMAINDER) {
                 tbuffer.BYTES.CHAR1 = (*((char *)(cur_mem_p)));
                 tbuffer.LW = tbuffer.LW | TC_BYTE_REMAINDER_MASK_CHAR1;
                 (*((long *)(cur_io_p))) = tbuffer.LW;
                 }
               }
             }
           break;
         default: /* Here for good form... aYup! */
           break;
         }
       } /* Srcaddr Aligned Dstaddr !Aligned */
     }
   else { /* srcaddr is !aligned  */

     /* --------------------------------------------------- */
     /* Source Address !Aligned Destination Address Aligned */
     /* --------------------------------------------------- */
     if(dstaddr_alignment == ALIGNED) {
       /* Do the copy based on the destination TC alignment */
       switch(srcaddr_alignment) {

         case BYTE_OFFSET: /* Source !Aligned destination Aligned */
           /* Align the Destination TC address, not necessary */
           /* Write LW to TC IO space until done */
           while(byte_count >= TC_LONGWORD) {
             tbuffer.BYTES.CHAR1 = *((char *)(cur_mem_p));
             (char *)cur_mem_p += 1;
             tbuffer.BYTES.CHAR2 = *((char *)(cur_mem_p));
             (char *)cur_mem_p += 1;
             tbuffer.BYTES.CHAR3 = *((char *)(cur_mem_p));
             (char *)cur_mem_p += 1;
             tbuffer.BYTES.CHAR4 = *((char *)(cur_mem_p));
             (char *)cur_mem_p += 1;
             *((int *)(cur_io_p)) = tbuffer.INT.IW;
             (int *)cur_io_p += 2;
             byte_count -= 4;
             }
           /* Write copy remainer to TC IO space */
           tbuffer.LW = 0;
           /* clean up the remaining buffer transfer of 1 to 3 bytes. */
           if(byte_count == TRIBYTE_REMAINDER) {
             tbuffer.BYTES.CHAR1 = *((char *)(cur_mem_p));
             (char *)cur_mem_p += 1;
             tbuffer.BYTES.CHAR2 = *((char *)(cur_mem_p));
             (char *)cur_mem_p += 1;
             tbuffer.BYTES.CHAR3 = *((char *)(cur_mem_p));
             (char *)cur_mem_p += 1;
             tbuffer.LW = tbuffer.LW | TC_TRIBYTE_REMAINDER_MASK_LOW;
             *((long *)(cur_io_p)) = tbuffer.LW;
             }
           else {
             if (byte_count == WORD_REMAINDER ){
               tbuffer.BYTES.CHAR1 = *((char *)(cur_mem_p));
               (char *)cur_mem_p += 1;
               tbuffer.BYTES.CHAR2 = *((char *)(cur_mem_p));
               tbuffer.LW = tbuffer.LW | TC_WORD_REMAINDER_MASK_WORD1;
               *((long *)(cur_io_p)) = tbuffer.LW;
                }
             else {
               if(byte_count == BYTE_REMAINDER) {
                 tbuffer.BYTES.CHAR1 = *((char *)(cur_mem_p));
                 tbuffer.LW = tbuffer.LW | TC_BYTE_REMAINDER_MASK_CHAR1;
                 *((long *)(cur_io_p)) = tbuffer.LW;
                 }
               }
             }
           break;

         case WORD_OFFSET: /* Source address !Aligned destination Aligned... */
           /* Align the Destination TC address, not necessary */
           /* Write LW to TC IO space until done */
           while(byte_count >= TC_LONGWORD) {
             tbuffer.WORDS.WORD1 = *((short *)(cur_mem_p));
             (char *)cur_mem_p += 2;
             tbuffer.WORDS.WORD2 = *((short *)(cur_mem_p));
             (char *)cur_mem_p += 2;
             *((int *)(cur_io_p)) = tbuffer.INT.IW;
             (int *)cur_io_p += 2;
             byte_count -= 4;
             }
           /* Write copy remainer to TC IO space */
           tbuffer.LW = 0;
           /* clean up the remaining buffer transfer of 1 to 3 bytes. */
           if(byte_count == TRIBYTE_REMAINDER) {
             tbuffer.WORDS.WORD1 = *((short *)(cur_mem_p));
             (char *)cur_mem_p += 2;
             tbuffer.BYTES.CHAR3 = *((char *)(cur_mem_p));
             tbuffer.LW = tbuffer.LW | TC_TRIBYTE_REMAINDER_MASK_LOW;
             *((long *)(cur_io_p)) = tbuffer.LW;
             }
           else {
             if (byte_count == WORD_REMAINDER ){
               tbuffer.WORDS.WORD1 = *((short *)(cur_mem_p));
               tbuffer.LW = tbuffer.LW | TC_WORD_REMAINDER_MASK_WORD1;
               *((long *)(cur_io_p)) = tbuffer.LW;
               }
             else {
               if(byte_count == BYTE_REMAINDER) {
                 (char *)cur_mem_p += 1;
                 tbuffer.BYTES.CHAR1 = *((char *)(cur_mem_p));
                 tbuffer.LW = tbuffer.LW | TC_BYTE_REMAINDER_MASK_CHAR1;
                 *((long *)(cur_io_p)) = tbuffer.LW;
                 }
               }

             }

           break;
         case TRIBYTE_OFFSET: /* Source address !Aligned destination Aligned... */
           /* Align the Destination TC address, not necessary  */
           /* Write LW to TC IO space until done */
           while(byte_count >= TC_LONGWORD) {
             tbuffer.BYTES.CHAR1 = (*((char *)(cur_mem_p)));
             (char *)cur_mem_p += 1;
             tbuffer.BYTES.CHAR2 = (*((char *)(cur_mem_p)));
             (char *)cur_mem_p += 1;
             tbuffer.BYTES.CHAR3 = (*((char *)(cur_mem_p)));
             (char *)cur_mem_p += 1;
             tbuffer.BYTES.CHAR4 = (*((char *)(cur_mem_p)));
             (char *)cur_mem_p += 1;
             (*(int *)(cur_io_p)) = tbuffer.INT.IW;
             (int *)cur_io_p += 2;
             byte_count -= 4;
             }
           /* Write copy remainer to TC IO space */
           tbuffer.LW = 0;
           /* clean up the remaining buffer transfer of 1 to 3 bytes. */
           if(byte_count == TRIBYTE_REMAINDER) {
             tbuffer.BYTES.CHAR1 = *((char *)(cur_mem_p));
             (char *)cur_mem_p += 1;
             tbuffer.BYTES.CHAR2 = *((char *)(cur_mem_p));
             (char *)cur_mem_p += 1;
             tbuffer.BYTES.CHAR3 = *((char *)(cur_mem_p));
             tbuffer.LW = tbuffer.LW | TC_TRIBYTE_REMAINDER_MASK_LOW ;
             (*((long *)(cur_io_p))) = tbuffer.LW;
             }
           else {
             if (byte_count == WORD_REMAINDER ){
               tbuffer.BYTES.CHAR1 = *((char *)(cur_mem_p));
               (char *)cur_mem_p += 1;
               tbuffer.BYTES.CHAR2 = *((char *)(cur_mem_p));
               tbuffer.LW = tbuffer.LW | TC_WORD_REMAINDER_MASK_WORD1;
               (*((long *)(cur_io_p))) = tbuffer.LW;
               }
             else {
               if(byte_count == BYTE_REMAINDER) {
                 tbuffer.BYTES.CHAR1 = *((char *)(cur_mem_p));
                 tbuffer.LW = tbuffer.LW | TC_BYTE_REMAINDER_MASK_CHAR1;
                 (*((long *)(cur_io_p))) = tbuffer.LW;
                 }
               }
             }
           break;
         default: /* Here for good form... aYup! */
           break;
         }
       }

     else {  /* dstaddr is !Aligned */

       /* ---------------------------------------------------- */
       /* Source Address !Aligned Destination Address !Aligned */
       /* ---------------------------------------------------- */
       /* Do the copy based on the destination TC alignment    */
       switch(dstaddr_alignment) {

         case TRIBYTE_OFFSET: /* Source !aligned destination !aligned */
           /* Align the Destination TC address */
           tbuffer.BYTES.CHAR4 = *((char *)(cur_mem_p));
           (char *)cur_mem_p += 1;
           tbuffer.LW = tbuffer.LW | TC_BYTE_REMAINDER_MASK_CHAR4;
           *((long *)(cur_io_p)) = tbuffer.LW;
           (int *)cur_io_p += 2;
           byte_count -= 1;
           tbuffer.LW = 0;
           /* Write LW to TC IO space until done */
           while(byte_count >= TC_LONGWORD) {
             tbuffer.BYTES.CHAR1 = *((char *)(cur_mem_p));
             (char *)cur_mem_p += 1;
             tbuffer.BYTES.CHAR2 = *((char *)(cur_mem_p));
             (char *)cur_mem_p += 1;
             tbuffer.BYTES.CHAR3 = *((char *)(cur_mem_p));
             (char *)cur_mem_p += 1;
             tbuffer.BYTES.CHAR4 = *((char *)(cur_mem_p));
             (char *)cur_mem_p += +1;
             *((int *)(cur_io_p)) = tbuffer.INT.IW;
             (int *)cur_io_p += 2;
             byte_count -= 4;
             }
           /* Write copy remainer to TC IO space */
           tbuffer.LW = 0;
           /* clean up the remaining buffer transfer of 1 to 3 bytes. */
           if(byte_count == TRIBYTE_REMAINDER) {
             tbuffer.BYTES.CHAR1 = *((char *)(cur_mem_p));
             (char *)cur_mem_p += 1;
             tbuffer.BYTES.CHAR2 = *((char *)(cur_mem_p));
             (char *)cur_mem_p += 1;
             tbuffer.BYTES.CHAR3 = *((char *)(cur_mem_p));
             tbuffer.LW = tbuffer.LW | TC_TRIBYTE_REMAINDER_MASK_LOW ;
             *((long *)(cur_io_p)) = tbuffer.LW;
             }
           else {
             if (byte_count == WORD_REMAINDER ){

               tbuffer.BYTES.CHAR1 = *((char *)(cur_mem_p));
               (char *)cur_mem_p += 1;
               tbuffer.BYTES.CHAR2 = *((char *)(cur_mem_p));
               tbuffer.LW = tbuffer.LW | TC_WORD_REMAINDER_MASK_WORD1;
               *((long *)(cur_io_p)) = tbuffer.LW;
                }
             else {
               if(byte_count == BYTE_REMAINDER){
                 tbuffer.BYTES.CHAR1 = *((char *)(cur_mem_p));
                 tbuffer.LW = tbuffer.LW | TC_BYTE_REMAINDER_MASK_CHAR1;
                 *((long *)(cur_io_p)) = tbuffer.LW;
                 }
               }
             }
           break;

         case WORD_OFFSET: /* Source address aligned !destination... */
           /* Align the Destination TC address */
           tbuffer.BYTES.CHAR3 = *((char *)(cur_mem_p));
           (char *)cur_mem_p += 1;
           tbuffer.BYTES.CHAR4 = *((char *)(cur_mem_p));
           (char *)cur_mem_p += 1;
           tbuffer.LW = tbuffer.LW | TC_WORD_REMAINDER_MASK_WORD2;
           *((long *)(cur_io_p)) = tbuffer.LW;
           (int *)cur_io_p += 2;
           byte_count -= 2;
           tbuffer.LW = 0;
           /* Write LW to TC IO space until done */
           while(byte_count >= TC_LONGWORD) {
             tbuffer.BYTES.CHAR1 = *((char *)(cur_mem_p));
             (char *)cur_mem_p += 1;
             tbuffer.BYTES.CHAR2 = *((char *)(cur_mem_p));
             (char *)cur_mem_p += 1;
             tbuffer.BYTES.CHAR3 = *((char *)(cur_mem_p));
             (char *)cur_mem_p += 1;
             tbuffer.BYTES.CHAR4 = *((char *)(cur_mem_p));
             (char *)cur_mem_p += +1;
             *((int *)(cur_io_p)) = tbuffer.INT.IW;
             (int *)cur_io_p += 2;
             byte_count -= 4;
             }
           /* Write copy remainer to TC IO space */
           tbuffer.LW = 0;
           /* clean up the remaining buffer transfer of 1 to 3 bytes. */
           if(byte_count == TRIBYTE_REMAINDER) {
             tbuffer.BYTES.CHAR1 = *((char *)(cur_mem_p));
             (char *)cur_mem_p += 1;
             tbuffer.BYTES.CHAR2 = *((char *)(cur_mem_p));
             (char *)cur_mem_p += 1;
             tbuffer.BYTES.CHAR3 = *((char *)(cur_mem_p));
             tbuffer.LW = tbuffer.LW | TC_TRIBYTE_REMAINDER_MASK_LOW;
             *((long *)(cur_io_p)) = tbuffer.LW;
             }
           else {
             if (byte_count == WORD_REMAINDER ){
               tbuffer.BYTES.CHAR1 = *((char *)(cur_mem_p));
               (char *)cur_mem_p += 1;

               tbuffer.BYTES.CHAR2 = *((char *)(cur_mem_p));
               tbuffer.LW = tbuffer.LW | TC_WORD_REMAINDER_MASK_WORD1;
               *((long *)(cur_io_p)) = tbuffer.LW;
               }
             else {
               if(byte_count == BYTE_REMAINDER) {
                 tbuffer.BYTES.CHAR1 = *((char *)(cur_mem_p));
                 tbuffer.LW = tbuffer.LW | TC_BYTE_REMAINDER_MASK_CHAR1;
                 *((long *)(cur_io_p)) = tbuffer.LW;
                 }
               }
             }
           break;

         case BYTE_OFFSET: /* Source address aligned !destination... */
           /* Align the Destination TC address */
           tbuffer.BYTES.CHAR2 = *((char *)(cur_mem_p));
           (char *)cur_mem_p += 1;
           tbuffer.BYTES.CHAR3 = *((char *)(cur_mem_p));
           (char *)cur_mem_p += 1;
           tbuffer.BYTES.CHAR4 = *((char *)(cur_mem_p));
           (char *)cur_mem_p += 1;
           tbuffer.LW = tbuffer.LW | TC_TRIBYTE_REMAINDER_MASK_HIGH ;
           *((long *)(cur_io_p)) = tbuffer.LW;
           (int *)cur_io_p += 2;
           byte_count -= 3;
           tbuffer.LW = 0;
           /* Write LW to TC IO space until done */
           while(byte_count >= TC_LONGWORD) {
             tbuffer.BYTES.CHAR1 = *((char *)(cur_mem_p));
             (char *)cur_mem_p += 1;
             tbuffer.BYTES.CHAR2 = *((char *)(cur_mem_p));
             (char *)cur_mem_p += 1;
             tbuffer.BYTES.CHAR3 = *((char *)(cur_mem_p));
             (char *)cur_mem_p += 1;
             tbuffer.BYTES.CHAR4 = *((char *)(cur_mem_p));
             (char *)cur_mem_p += 1;
             *((int *)(cur_io_p)) = tbuffer.INT.IW;
             (int  *)cur_io_p += 2;
             byte_count -= 4;
             }
           /* Write copy remainer to TC IO space */
           tbuffer.LW = 0;
           /* clean up the remaining buffer transfer of 1 to 3 bytes. */
           if(byte_count == TRIBYTE_REMAINDER) {
             tbuffer.BYTES.CHAR1 = *((char *)(cur_mem_p));
             (char *)cur_mem_p += 1;
             tbuffer.BYTES.CHAR2 = *((char *)(cur_mem_p));
             (char *)cur_mem_p += 1;
             tbuffer.BYTES.CHAR3 = *((char *)(cur_mem_p));
             (char *)cur_mem_p += 1;
             tbuffer.LW = tbuffer.LW | TC_TRIBYTE_REMAINDER_MASK_LOW ;
             *((long *)(cur_io_p)) = tbuffer.LW;
             }
           else {
             if (byte_count == WORD_REMAINDER ){
               tbuffer.BYTES.CHAR1 = *((char *)(cur_mem_p));

               (char *)cur_mem_p += 1;
               tbuffer.BYTES.CHAR2 = *((char *)(cur_mem_p));
               tbuffer.LW = tbuffer.LW | TC_WORD_REMAINDER_MASK_WORD1;
               *((long *)(cur_io_p)) = tbuffer.LW;
               }
             else {
               if(byte_count == BYTE_REMAINDER){
                 tbuffer.BYTES.CHAR1 = *((char *)(cur_mem_p));
                 tbuffer.LW = tbuffer.LW | TC_BYTE_REMAINDER_MASK_CHAR1;
                 *((long *)(cur_io_p)) = tbuffer.LW;
                 }
               }
             }
           break;
         default: /* Here for good form... aYup! */
           break;
         }
       }
     }
   return (ret_val);

}  /* End kn16aa_io_copyout */


/***************************************************************************/
/*                                                                         */
/* kn16aa_io_copyin   -  Copy a block of Memory from IO address space      */
/*                       into system Memory.                               */
/*                                                                         */
/* SYNOPSIS                                                                */
/*      int  kn16aa_io_copyin (srcaddr, dstaddr, byte_count)               */
/*                                                                         */
/*      io_handle_t     srcaddr;                                           */
/*      vm_offset_t     dstaddr;                                           */
/*      int             byte_count;                                        */
/*                                                                         */
/*                                                                         */
/* PARAMETERS                                                              */
/*      srcaddr         This is the IO handle of the base system combined  */
/*                      with the physical address of the IO space the      */
/*                      copy originates from.                              */
/*                                                                         */
/*      dstaddr         This is the Kerenl Virtual Memory address that the */
/*                      copy operation is destined for.                    */
/*                                                                         */
/*      byte_count      Number of bytes in block to be copied. This        */
/*                      buffer is assumed to be physically contigous.      */
/*                                                                         */
/*   Note: Neither the srcaddr or the dstaddr assume any alignment.        */
/*                                                                         */
/*                                                                         */
/* DESCRIPTION                                                             */
/*                                                                         */
/*    Pcode: io_copyin () - Trick, getting to "lw" IO read operations      */
/*                          for the bulk of the transfer.                  */
/*                                                                         */
/***************************************************************************/

int
kn16aa_io_copyin(srcaddr, dstaddr, byte_count )
   io_handle_t  srcaddr;
   vm_offset_t  dstaddr;
   u_long       byte_count;

{  /* Begin kn16aa_io_copyin */

   register u_int   ret_val;
   register u_long  cur_mem_p;
   register u_long  cur_io_p;
   register u_long  srcaddr_alignment, dstaddr_alignment;
   union  buffer tbuffer;

   ret_val = IOA_OKAY; 
   tbuffer.LW  = 0;
   /* Convert io_handle_t into sparse space address */
   /* For birds iohandle sparse address walks ints by two */
   cur_io_p = ((srcaddr & KN16_IOHANDLE_MASK) |
                      (srcaddr & MASK_LONG_ADDRESS & KN16_DENSE_ADDRESS_MASK)*2);
   srcaddr_alignment = srcaddr & ALIGNMENT_MASK;
   dstaddr_alignment = dstaddr & ALIGNMENT_MASK;
   cur_mem_p = dstaddr;
   /* -------------------------------------------------- */
   /* Source Address Aligned Destination Address Aligned */
   /* -------------------------------------------------- */
   if(srcaddr_alignment == ALIGNED ) {
     if(dstaddr_alignment == ALIGNED ) {
       while(byte_count >= TC_LONGWORD) {
         (*((int *)cur_mem_p)) = (*((int *)cur_io_p));
         byte_count -= 4;
         (int *)cur_io_p += 2;
         (int *)cur_mem_p += 1;
         }
       /* clean up the remaining buffer transfer of 1 to 3 bytes. */
       if(byte_count == TRIBYTE_REMAINDER) {
         tbuffer.INT.IW = (*((int *)cur_io_p));
         *((short *)(cur_mem_p)) = tbuffer.WORD_BYTES.WORD1;
         (unsigned char *)cur_mem_p += 2;
         *((char *)(cur_mem_p)) = tbuffer.WORD_BYTES.CHAR1;
         }
       else {
         if (byte_count == WORD_REMAINDER){
           tbuffer.INT.IW = (*((int *)cur_io_p));
           *((short *)(cur_mem_p)) = tbuffer.WORD_BYTES.WORD1;
           }

         else {
           if (byte_count == BYTE_REMAINDER){
             tbuffer.INT.IW = (*((int *)cur_io_p));
             *((char  *)(cur_mem_p)) = tbuffer.BYTES.CHAR1;
             }
           }

         }
       }
     /* ----------------------------------------------------*/
     /* Source Address Aligned Destination Address !Aligned */
     /* ----------------------------------------------------*/
     else {
       /* Do the copy based on the destination TC alignment */
       switch(dstaddr_alignment) {

         case TRIBYTE_OFFSET: /* Source aligned !destination on byte offset  */
           /* Align the Destination memory address */
           tbuffer.INT.IW = (*((int *)cur_io_p));
           (int *)cur_io_p += 2;
           (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR1;
           (char *)cur_mem_p += 1;
           byte_count -= 1;
           /* Write LW to Memory space until done */
           while(byte_count >= TC_LONGWORD) {
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR2;
             (char *)cur_mem_p += 1;
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR3;
             (char *)cur_mem_p += 1;
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR4;
             (char *)cur_mem_p += +1;
             tbuffer.INT.IW = (*((int *)cur_io_p));
             (int *)cur_io_p += 2;
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR1;
             (char *)cur_mem_p += 1;
             byte_count -= 4;
             }
           /* Write copy remainer to Memory space */
           /* clean up the remaining buffer transfer of 1 to 3 bytes. */
           if(byte_count == TRIBYTE_REMAINDER) {
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR2;
             (char *)cur_mem_p += 1;
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR3;
             (char *)cur_mem_p += 1;
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR4;
             }
           else {
             if (byte_count == WORD_REMAINDER ){
               (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR2;
               (char *)cur_mem_p += 1;
               (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR3;
                }
             else {
                if(byte_count == BYTE_REMAINDER) {
                  (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR2;
                 }
               }
             }
           break;
         case WORD_OFFSET: /* Source address aligned !destination... */
           /* Align the Destination Memory address */
           tbuffer.INT.IW = (*((int *)cur_io_p));

           (*((short *)(cur_mem_p))) = tbuffer.WORDS.WORD1;
           (char *)cur_mem_p += 2;
           (int *)cur_io_p += 2;
           byte_count -= 2;
           /* Write LW to Memory space until done */
           while(byte_count >= TC_LONGWORD) {
             (*((short *)(cur_mem_p))) =  tbuffer.WORDS.WORD2;
             (char *)cur_mem_p += 2;
             tbuffer.INT.IW = (*((int *)cur_io_p));
             (int *)cur_io_p += 2;
             (*((short *)(cur_mem_p))) =  tbuffer.WORDS.WORD1;
             (char *)cur_mem_p += 2;
             byte_count -= 4;
             }
           /* Write copy remainer to Memory space */
           /* clean up the remaining buffer transfer of 1 to 3 bytes. */
           if(byte_count == TRIBYTE_REMAINDER) {
             (*((short *)(cur_mem_p))) =  tbuffer.WORDS.WORD2;
             (char *)cur_mem_p += 2;
             tbuffer.INT.IW = (*((int *)cur_io_p));
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR1;
             }
           else {
             if (byte_count == WORD_REMAINDER ){
               (*((short *)(cur_mem_p))) =  tbuffer.WORDS.WORD2;
               }
             else {
               if(byte_count == BYTE_REMAINDER){
                 (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR3;
                 }
               }
             }
           break;

         case BYTE_OFFSET: /* Source address aligned !destination... */
           /* Align the Destination Memory address */
           tbuffer.INT.IW = (*((int *)cur_io_p));
           (int *)cur_io_p += 2;
           (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR1;
           (char *)cur_mem_p += 1;

           (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR2;
           (char *)cur_mem_p += 1;
           (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR3;
           (char *)cur_mem_p += +1;
           byte_count -= 3;
           /* Write LW to Memory space until done */
           while(byte_count >= TC_LONGWORD) {
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR4;
             (char *)cur_mem_p += 1;
             tbuffer.INT.IW = (*((int *)cur_io_p));
             (int *)cur_io_p += 2;
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR1;
             (char *)cur_mem_p += 1;
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR2;
             (char *)cur_mem_p += 1;
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR3;

             (char *)cur_mem_p += +1;
             byte_count -= 4;
             }

           /* Write copy remainer to Memory space */
           /* clean up the remaining buffer transfer of 1 to 3 bytes. */
           if(byte_count == TRIBYTE_REMAINDER) {
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR4;
             (char *)cur_mem_p += 1;
             tbuffer.INT.IW = (*((int *)cur_io_p));
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR1;
             (char *)cur_mem_p += 1;
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR2;
             }
           else {
             if (byte_count == WORD_REMAINDER ){
               (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR4;
               (char *)cur_mem_p += 1;
               tbuffer.INT.IW = (*((int *)cur_io_p));
               (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR1;
               }
             else {
               if(byte_count == BYTE_REMAINDER) {
                 (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR4;
                 }
               }
            }
           break;
         default: /* Here for good form... aYup! */
           break;
         }
       } /* Srcaddr Aligned Dstaddr !Aligned */
     }
   else { /* srcaddr is !aligned  */

     /* --------------------------------------------------- */
     /* Source Address !Aligned Destination Address Aligned */
     /* --------------------------------------------------- */

     if(dstaddr_alignment == ALIGNED) {
       /* Do the copy based on the destination TC alignment */
       switch(srcaddr_alignment) {

         case BYTE_OFFSET: /* Source !Aligned destination Aligned */
           tbuffer.INT.IW = (*((int *)cur_io_p));
           (int *)cur_io_p += 2;
           /* Write LW to Memory space until done */
           while(byte_count >= TC_LONGWORD) {
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR2;
             (char *)cur_mem_p += 1;
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR3;
             (char *)cur_mem_p += 1;
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR4;
             (char *)cur_mem_p += +1;
             tbuffer.INT.IW = (*((int *)cur_io_p));
             (int *)cur_io_p += 2;
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR1;

             (char *)cur_mem_p += 1;
             byte_count -= 4;
             }
           /* Write copy remainer to TC IO space */
           /* clean up the remaining buffer transfer of 1 to 3 bytes. */
           if(byte_count == TRIBYTE_REMAINDER) {
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR2;
             (char *)cur_mem_p += 1;
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR3;
             (char *)cur_mem_p += 1;
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR4;
             }
           else {
             if (byte_count == WORD_REMAINDER ){
               (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR2;
               (char *)cur_mem_p += 1;
               (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR3;
                }
             else {
               if(byte_count == BYTE_REMAINDER) {
                 (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR2;
                 }
               }
             }
           break;

         case WORD_OFFSET: /* Source address !Aligned destination Aligned... */
           /* Write to Memory space until done */
           tbuffer.INT.IW = (*((int *)cur_io_p));
           (int *)cur_io_p += 2;
           while(byte_count >= TC_LONGWORD) {
             (*((short *)(cur_mem_p))) =  tbuffer.WORDS.WORD2;

             (char *)cur_mem_p += 2;
             tbuffer.INT.IW = (*((int *)cur_io_p));
             (int *)cur_io_p += 2;
             (*((short *)(cur_mem_p))) =  tbuffer.WORDS.WORD1;
             (char *)cur_mem_p += 2;
             byte_count -= 4;
             }
           /* Write copy remainer to TC IO space */
           /* clean up the remaining buffer transfer of 1 to 3 bytes. */
           if(byte_count == TRIBYTE_REMAINDER) {
             (*((short *)(cur_mem_p))) =  tbuffer.WORDS.WORD2;
             (char *)cur_mem_p += 2;
             tbuffer.INT.IW = (*((int *)cur_io_p));
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR1;
             }
           else {
             if (byte_count == WORD_REMAINDER ){
               (*((short *)(cur_mem_p))) =  tbuffer.WORDS.WORD2;
               }
             else {
               if(byte_count == BYTE_REMAINDER) {
                 (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR3;
                 }
               }

             }

           break;
         case TRIBYTE_OFFSET: /* Source address !Aligned destination Aligned... */
           /* Align the Destination Memory address, not necessary  */
           /* Write to Memory  IO space until done */
           tbuffer.INT.IW = (*((int *)cur_io_p));
           (int *)cur_io_p += 2;
           while(byte_count >= TC_LONGWORD) {
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR1;
             (char *)cur_mem_p += 1;
             tbuffer.INT.IW = (*((int *)cur_io_p));
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR2;
             (char *)cur_mem_p += 1;
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR3;
             (char *)cur_mem_p += 1;
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR4;
             (char *)cur_mem_p += 1;
             byte_count -= 4;
             }
           /* Write copy remainer to TC IO space */
           /* clean up the remaining buffer transfer of 1 to 3 bytes. */
           if(byte_count == TRIBYTE_REMAINDER) {
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR1;
             (char *)cur_mem_p += 1;
             tbuffer.INT.IW = (*((int *)cur_io_p));
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR2;
             (char *)cur_mem_p += 1;

             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR3;
             }
           else {
             if (byte_count == WORD_REMAINDER ){
               (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR1;
               (char *)cur_mem_p += 1;
               tbuffer.INT.IW = (*((int *)cur_io_p));
               (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR2;
               }
             else {
               if(byte_count == BYTE_REMAINDER) {
                 (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR1;
                 }
               }
             }
           break;
         default: /* Here for good form... aYup! */
           break;
         }
       }

     else {  /* dstaddr is !Aligned */

       /* ---------------------------------------------------- */
       /* Source Address !Aligned Destination Address !Aligned */
       /* ---------------------------------------------------- */
       /* Do the copy based on the destination TC alignment    */
       switch(srcaddr_alignment) {


         case TRIBYTE_OFFSET: /* Source !aligned destination !aligned */
           /* Align the Destination Memory address */
           tbuffer.INT.IW = (*((int *)cur_io_p));
           (int *)cur_io_p += 2;
           (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR4;
           (char *)cur_mem_p += 1;
           byte_count -= 1;
           /* Write to Memory IO space until done */
           while(byte_count >= TC_LONGWORD) {
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR1;
             (char *)cur_mem_p += 1;
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR2;
             (char *)cur_mem_p += 1;
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR3;
             (char *)cur_mem_p += +1;
             tbuffer.INT.IW = (*((int *)cur_io_p));
             (int *)cur_io_p += 2;
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR4;
             (char *)cur_mem_p += 1;
             byte_count -= 4;
             }
           /* Write copy remainer to TC IO space */
           /* clean up the remaining buffer transfer of 1 to 3 bytes. */

           if(byte_count == TRIBYTE_REMAINDER) {
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR1;
             (char *)cur_mem_p += 1;
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR2;
             (char *)cur_mem_p += 1;
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR3;
             }
           else {
             if (byte_count == WORD_REMAINDER ){
               (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR1;
               (char *)cur_mem_p += 1;
               (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR2;
               }
             else {
               if(byte_count == BYTE_REMAINDER){
                 (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR1;
                 (char *)cur_mem_p += 1;
                 }
               }
             }
           break;

         case WORD_OFFSET: /* Source address ! aligned !destination... */
           /* Align the Destination TC address */
           tbuffer.INT.IW = (*((int *)cur_io_p));
           (int *)cur_io_p += 2;
           (*((short *)(cur_mem_p))) =  tbuffer.WORDS.WORD2;
           (char *)cur_mem_p += 2;
           byte_count -= 2;
           /* Write LW to TC IO space until done */

           while(byte_count >= TC_LONGWORD) {
             (*((short *)(cur_mem_p))) =  tbuffer.WORDS.WORD1;
             (char *)cur_mem_p += 2;
             tbuffer.INT.IW = (*((int *)cur_io_p));
             (int *)cur_io_p += 2;
             (*((short *)(cur_mem_p))) = tbuffer.WORDS.WORD2;
             (char *)cur_mem_p += 2;
             byte_count -= 4;
             }
           /* Write copy remainer to TC IO space */
           /* clean up the remaining buffer transfer of 1 to 3 bytes. */
           if(byte_count == TRIBYTE_REMAINDER) {
             (*((short *)(cur_mem_p))) = tbuffer.WORDS.WORD1;
             (char *)cur_mem_p += 2;
             tbuffer.INT.IW = (*((int *)cur_io_p));
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR3;
             }
           else {
             if (byte_count == WORD_REMAINDER ){
               (*((short *)(cur_mem_p))) =  tbuffer.WORDS.WORD1;
               }
             else {

               if(byte_count == BYTE_REMAINDER) {
                 (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR1;
                 }
               }
             }
           break;

         case BYTE_OFFSET: /* Source address aligned !destination... */
           /* Align the Destination TC address */
           tbuffer.INT.IW = (*((int *)cur_io_p));
           (int *)cur_io_p += 2;
           (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR2;
           (char *)cur_mem_p += 1;
           (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR3;
           (char *)cur_mem_p += 1;
           (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR4;
           (char *)cur_mem_p += +1;
           byte_count -= 3;
           /* Write LW to TC IO space until done */
           while(byte_count >= TC_LONGWORD) {
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR1;
             (char *)cur_mem_p += 1;
             tbuffer.INT.IW = (*((int *)cur_io_p));
             (int *)cur_io_p += 2;
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR2;
             (char *)cur_mem_p += 1;
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR3;
             (char *)cur_mem_p += 1;
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR4;
             (char *)cur_mem_p += +1;
             byte_count -= 4;
             }
           /* Write copy remainer to TC IO space */
           /* clean up the remaining buffer transfer of 1 to 3 bytes. */

           if(byte_count == TRIBYTE_REMAINDER) {
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR1;
             (char *)cur_mem_p += 1;
             tbuffer.INT.IW = (*((int *)cur_io_p));
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR2;
             (char *)cur_mem_p += 1;
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR3;
             }
           else {
             if (byte_count == WORD_REMAINDER ){
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR1;
             (char *)cur_mem_p += 1;
             tbuffer.INT.IW = (*((int *)cur_io_p));
             (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR2;

               }
             else {
               if(byte_count == BYTE_REMAINDER){
                 (*((char *)(cur_mem_p))) = tbuffer.BYTES.CHAR1;
                 }
               }
             }
           break;
         default: /* Here for good form... aYup! */
           break;
         }
       }
     }
   return (ret_val);
}  /* End kn16aa_io_copyin */






/***************************************************************************/
/*                                                                         */
/* kn16aa_io_copyio  -  Copy a block of IO space to another                */
/*                      block of IO space.                                 */
/*                                                                         */
/* SYNOPSIS                                                                */
/*      int  kn16aa_io_copyio (srcaddr, dstaddr, byte_count)               */
/*                                                                         */
/*      io_handle_t     srcaddr;                                           */
/*      io_handle_t     dstaddr;                                           */
/*      int             byte_count;                                        */
/*                                                                         */
/*                                                                         */
/* PARAMETERS                                                              */
/*                                                                         */
/*    srcaddr         This is the IO handle of the device driver combined  */
/*                    with the IO offset the copy originates from.         */
/*                                                                         */
/*    dstaddr         This is the IO handle of the device driver combined  */
/*                    with the IO offset the copy is destined for.         */
/*                                                                         */
/*    byte_count      Number of bytes to be copied.                        */
/*                    This buffer is assumed to be physically contigous.   */
/*                                                                         */
/*   Note: Neither the srcaddr or the dstaddr assume any alignment of      */
/*         the data.                                                       */
/*                                                                         */
/*                                                                         */
/* DESCRIPTION                                                             */
/*                                                                         */
/*    Pcode: io_copyin()  - Trick, getting to "lw" IO read operations      */
/*                          and "lw" IO write opernation for the BULK      */
/*                          of the transfer.                               */
/*                                                                         */
/***************************************************************************/

int
kn16aa_io_copyio(srcaddr, dstaddr, byte_count)
   io_handle_t  srcaddr;
   io_handle_t  dstaddr;
   u_long       byte_count;

{  /* Begin kn16aa_io_copyio */
   register u_int          ret_val;
   register u_int          item_count;
   register u_long         cur_io_in_p;
   register u_long         cur_io_out_p;
   register u_long         srcaddr_alignment, dstaddr_alignment;
   union  buffer iobuffer_in,iobuffer_out;

   ret_val = IOA_OKAY;
   /* Convert srcaddr io_handle_t into sparse space address */
   /* For birds iohandle sparse address walks ints by two */
   cur_io_in_p = ((srcaddr & KN16_IOHANDLE_MASK) |
                      (srcaddr & LONGWORD_ALIGNMENT_MASK & KN16_DENSE_ADDRESS_MASK)*2);
   /* TC IO Pointer is Long Word Aligned */
   cur_io_out_p = ((dstaddr & KN16_IOHANDLE_MASK) |
                      (dstaddr & LONGWORD_ALIGNMENT_MASK & KN16_DENSE_ADDRESS_MASK)*2);
   srcaddr_alignment = srcaddr & ALIGNMENT_MASK;
   dstaddr_alignment = dstaddr & ALIGNMENT_MASK;
   /* -------------------------------------------------- */
   /* Source Address Aligned Destination Address Aligned */
   /* -------------------------------------------------- */

   if(srcaddr_alignment == ALIGNED ) {
     if(dstaddr_alignment == ALIGNED ) {
       while(byte_count >= TC_LONGWORD) {
         (*((int *)cur_io_out_p)) = (*((int *)cur_io_in_p));
         byte_count -= 4;
         (int *)cur_io_in_p += 2;
         (int *)cur_io_out_p += 2;
         }
       /* clean up the remaining buffer transfer of 1 to 3 bytes. */
       if(byte_count == TRIBYTE_REMAINDER) {
         iobuffer_in.LW = (*((int *)cur_io_in_p));
         iobuffer_in.INT.PAD = 0;
         (*(long *)(cur_io_out_p)) = iobuffer_in.LW | TC_TRIBYTE_REMAINDER_MASK_LOW;
         }
       else {
         if (byte_count == WORD_REMAINDER){
           iobuffer_in.LW = (*((int *)cur_io_in_p));
           iobuffer_in.INT.PAD = 0;
           (*(long *)(cur_io_out_p)) = iobuffer_in.LW | TC_WORD_REMAINDER_MASK_WORD1;
           }
         else {
           if (byte_count == BYTE_REMAINDER){
             iobuffer_in.LW = (*((int *)cur_io_in_p));
             iobuffer_in.INT.PAD = 0;
             (*(long *)(cur_io_out_p)) = iobuffer_in.LW | TC_BYTE_REMAINDER_MASK_CHAR1;
             }
           }
         }
       }
     /* ----------------------------------------------------*/
     /* Source Address Aligned Destination Address !Aligned */
     /* ----------------------------------------------------*/
     else {
       /* Do the copy based on the destination TC alignment */
       switch(dstaddr_alignment) {

         case TRIBYTE_OFFSET: /* Source aligned !destination on byte offset  */
           iobuffer_in.INT.IW  = (*((int *)cur_io_in_p));
           (int *)cur_io_in_p += 2;

           iobuffer_out.BYTES.CHAR4 = iobuffer_in.BYTES.CHAR1;
           iobuffer_out.INT.PAD = 0;
           (*((long *)(cur_io_out_p))) = iobuffer_out.LW  | TC_BYTE_REMAINDER_MASK_CHAR4;
           (int *)cur_io_out_p += 2;
           byte_count -= 1;
           /* Write LW to TC IO space until done */
           while(byte_count >= TC_LONGWORD) {
             iobuffer_out.BYTES.CHAR1 = iobuffer_in.BYTES.CHAR2;
             iobuffer_out.BYTES.CHAR2 = iobuffer_in.BYTES.CHAR3;
             iobuffer_out.BYTES.CHAR3 = iobuffer_in.BYTES.CHAR4;
             iobuffer_in.INT.IW  = (*((int *)cur_io_in_p));
             (int *)cur_io_in_p += 2;
             iobuffer_out.BYTES.CHAR4 = iobuffer_in.BYTES.CHAR1;
             (*((int *)(cur_io_out_p))) = iobuffer_out.INT.IW;
             (int *)cur_io_out_p += 2;
             byte_count -= 4;
             }
           /* Write copy remainer to TC IO space */
           /* clean up the remaining buffer transfer of 1 to 3 bytes. */
           if(byte_count == TRIBYTE_REMAINDER) {
             iobuffer_out.BYTES.CHAR1 = iobuffer_in.BYTES.CHAR2;
             iobuffer_out.BYTES.CHAR2 = iobuffer_in.BYTES.CHAR3;
             iobuffer_out.BYTES.CHAR3 = iobuffer_in.BYTES.CHAR4;
             iobuffer_out.INT.PAD = 0;
             (*((long *)(cur_io_out_p))) = iobuffer_out.LW | TC_TRIBYTE_REMAINDER_MASK_LOW;
             }
           else {
             if (byte_count == WORD_REMAINDER ){
               iobuffer_out.BYTES.CHAR1 = iobuffer_in.BYTES.CHAR2;
               iobuffer_out.BYTES.CHAR2 = iobuffer_in.BYTES.CHAR3;
               iobuffer_out.INT.PAD = 0;
               (*((long *)(cur_io_out_p))) = iobuffer_out.LW | TC_WORD_REMAINDER_MASK_WORD1;
               }
             else {
                if(byte_count == BYTE_REMAINDER) {
                  iobuffer_out.BYTES.CHAR1 = iobuffer_in.BYTES.CHAR2;
                  iobuffer_out.INT.PAD = 0;
                  (*((long *)(cur_io_out_p))) = iobuffer_out.LW | TC_BYTE_REMAINDER_MASK_CHAR1;
                 }
               }
             }
           break;
         case WORD_OFFSET: /* Source address aligned !destination... */
           /* Align the Destination TC address */
           iobuffer_in.INT.IW  = (*((int *)cur_io_in_p));
           (int *)cur_io_in_p += 2;
           iobuffer_out.WORDS.WORD2 = iobuffer_in.WORDS.WORD1;
           iobuffer_out.INT.PAD = 0;
           (*((long *)(cur_io_out_p))) = iobuffer_out.LW  | TC_WORD_REMAINDER_MASK_WORD2;
           (int *)cur_io_out_p += 2;
           byte_count -= 2;
           /* Write LW to TC IO space until done */
           while(byte_count >= TC_LONGWORD) {
             iobuffer_out.WORDS.WORD1 = iobuffer_in.WORDS.WORD2;
             iobuffer_in.INT.IW  = (*((int *)cur_io_in_p));
             (int *)cur_io_in_p += 2;
             iobuffer_out.WORDS.WORD2 = iobuffer_in.WORDS.WORD1;
             (*((int *)(cur_io_out_p))) = iobuffer_out.INT.IW;
             (int *)cur_io_out_p += 2;
             byte_count -= 4;

             }
           /* Write copy remainer to TC IO space */
           /* clean up the remaining buffer transfer of 1 to 3 bytes. */
           if(byte_count == TRIBYTE_REMAINDER) {
             iobuffer_out.WORDS.WORD1 = iobuffer_in.WORDS.WORD2;
             iobuffer_in.INT.IW  = (*((int *)cur_io_in_p));
             iobuffer_out.BYTES.CHAR3 = iobuffer_in.BYTES.CHAR1;
             iobuffer_out.INT.PAD = 0;
             (*((long *)(cur_io_out_p))) = iobuffer_out.LW | TC_TRIBYTE_REMAINDER_MASK_LOW;
             }
           else {
             if (byte_count == WORD_REMAINDER ){
               iobuffer_out.WORDS.WORD1 = iobuffer_in.WORDS.WORD2;
               iobuffer_out.INT.PAD = 0;
               (*((long *)(cur_io_out_p))) = iobuffer_out.LW | TC_WORD_REMAINDER_MASK_WORD1;
               }
             else {
               if(byte_count == BYTE_REMAINDER){
                 iobuffer_out.BYTES.CHAR1 = iobuffer_in.BYTES.CHAR3;
                 iobuffer_out.INT.PAD = 0;
                 (*((long *)(cur_io_out_p))) = iobuffer_out.LW | TC_BYTE_REMAINDER_MASK_CHAR1;
                 }
               }
             }
           break;

         case BYTE_OFFSET: /* Source address aligned !destination... */
           /* Align the Destination TC address */
           iobuffer_in.INT.IW  = (*((int *)cur_io_in_p));
           (int *)cur_io_in_p += 2;
           iobuffer_out.BYTES.CHAR2 = iobuffer_in.BYTES.CHAR1;
           iobuffer_out.BYTES.CHAR3 = iobuffer_in.BYTES.CHAR2;
           iobuffer_out.BYTES.CHAR4 = iobuffer_in.BYTES.CHAR3;
                 iobuffer_out.INT.PAD = 0;
           (*((long *)(cur_io_out_p))) = iobuffer_out.LW  | TC_TRIBYTE_REMAINDER_MASK_HIGH;
           (int *)cur_io_out_p += 2;
           byte_count -= 3;
           /* Write LW to TC IO space until done */
           while(byte_count >= TC_LONGWORD) {
             iobuffer_out.BYTES.CHAR1 = iobuffer_in.BYTES.CHAR4;
             iobuffer_in.INT.IW  = (*((int *)cur_io_in_p));
             (int *)cur_io_in_p += 2;

             iobuffer_out.BYTES.CHAR2 = iobuffer_in.BYTES.CHAR1;
             iobuffer_out.BYTES.CHAR3 = iobuffer_in.BYTES.CHAR2;
             iobuffer_out.BYTES.CHAR4 = iobuffer_in.BYTES.CHAR3;
             (*((int *)(cur_io_out_p))) = iobuffer_out.INT.IW;
             (int *)cur_io_out_p += 2;
             byte_count -= 4;
             }
           /* Write copy remainer to TC IO space */
           /* clean up the remaining buffer transfer of 1 to 3 bytes. */
           if(byte_count == TRIBYTE_REMAINDER) {
             iobuffer_out.BYTES.CHAR1 = iobuffer_in.BYTES.CHAR4;
             iobuffer_in.INT.IW  = (*((int *)cur_io_in_p));
             iobuffer_out.BYTES.CHAR2 = iobuffer_in.BYTES.CHAR1;
             iobuffer_out.BYTES.CHAR3 = iobuffer_in.BYTES.CHAR2;
             iobuffer_out.INT.PAD = 0;
             (*((long *)(cur_io_out_p))) = iobuffer_out.LW | TC_TRIBYTE_REMAINDER_MASK_LOW;
             }
           else {
             if (byte_count == WORD_REMAINDER ){
               iobuffer_out.BYTES.CHAR1 = iobuffer_in.BYTES.CHAR4;
               iobuffer_in.INT.IW  = (*((int *)cur_io_in_p));
               iobuffer_out.BYTES.CHAR2 = iobuffer_in.BYTES.CHAR1;
               iobuffer_out.INT.PAD = 0;
               (*((long *)(cur_io_out_p))) = iobuffer_out.LW | TC_WORD_REMAINDER_MASK_WORD1;
               }
             else {
               if(byte_count == BYTE_REMAINDER) {
                 iobuffer_out.BYTES.CHAR1 = iobuffer_in.BYTES.CHAR4;
                 iobuffer_out.INT.PAD = 0;
                 (*((long *)(cur_io_out_p))) = iobuffer_out.LW | TC_BYTE_REMAINDER_MASK_CHAR1;
                 }
               }
             }
           break;
         default: /* Here for good form... aYup! */
           break;
         }
       } /* Srcaddr Aligned Dstaddr !Aligned */
     }
   else { /* srcaddr is !aligned  */

     /* --------------------------------------------------- */
     /* Source Address !Aligned Destination Address Aligned */
     /* --------------------------------------------------- */
     if(dstaddr_alignment == ALIGNED) {
       /* Do the copy based on the destination TC alignment */
       switch(srcaddr_alignment) {

         case BYTE_OFFSET: /* Source !Aligned destination Aligned */
           /* Align the Destination TC address, not necessary */
           /* Write LW to TC IO space until done */
           iobuffer_in.INT.IW  = (*((int *)cur_io_in_p));
           (int *)cur_io_in_p += 2;
           while(byte_count >= TC_LONGWORD) {
             iobuffer_out.BYTES.CHAR1 = iobuffer_in.BYTES.CHAR2;
             iobuffer_out.BYTES.CHAR2 = iobuffer_in.BYTES.CHAR3;
             iobuffer_out.BYTES.CHAR3 = iobuffer_in.BYTES.CHAR4;
             iobuffer_in.INT.IW  = (*((int *)cur_io_in_p));
             (int *)cur_io_in_p += 2;
             iobuffer_out.BYTES.CHAR4 = iobuffer_in.BYTES.CHAR1;
             *((int *)(cur_io_out_p)) = iobuffer_out.INT.IW;
             (int *)cur_io_out_p += 2;
             byte_count -= 4;
             }
           /* Write copy remainer to TC IO space */
           /* clean up the remaining buffer transfer of 1 to 3 bytes. */
           if(byte_count == TRIBYTE_REMAINDER) {
             iobuffer_out.BYTES.CHAR1 = iobuffer_in.BYTES.CHAR2;
             iobuffer_out.BYTES.CHAR2 = iobuffer_in.BYTES.CHAR3;
             iobuffer_out.BYTES.CHAR3 = iobuffer_in.BYTES.CHAR4;
             iobuffer_out.INT.PAD = 0;
             iobuffer_out.LW = iobuffer_out.LW | TC_TRIBYTE_REMAINDER_MASK_LOW;
             }
           else {
             if (byte_count == WORD_REMAINDER ){
               iobuffer_out.BYTES.CHAR1 = iobuffer_in.BYTES.CHAR2;
               iobuffer_out.BYTES.CHAR2 = iobuffer_in.BYTES.CHAR3;
               iobuffer_out.INT.PAD = 0;
               iobuffer_out.LW = iobuffer_out.LW | TC_WORD_REMAINDER_MASK_WORD1;
                }
             else {
               if(byte_count == BYTE_REMAINDER) {
                 iobuffer_out.BYTES.CHAR1 = iobuffer_in.BYTES.CHAR2;
                 iobuffer_out.INT.PAD = 0;
                 iobuffer_out.LW = iobuffer_out.LW | TC_BYTE_REMAINDER_MASK_CHAR1;
                 }
               }
             }
           break;

         case WORD_OFFSET: /* Source address !Aligned destination Aligned... */
           /* Align the Destination TC address, not necessary */
           /* Write LW to TC IO space until done */
           iobuffer_in.INT.IW  = (*((int *)cur_io_in_p));
           (int *)cur_io_in_p += 2;
           while(byte_count >= TC_LONGWORD) {
             iobuffer_out.WORDS.WORD1 = iobuffer_in.WORDS.WORD2;
             iobuffer_in.INT.IW  = (*((int *)cur_io_in_p));
             (int *)cur_io_in_p += 2;
             iobuffer_out.WORDS.WORD2 = iobuffer_in.WORDS.WORD1;
             (*((int *)(cur_io_out_p))) = iobuffer_out.INT.IW;
             (int *)cur_io_out_p += 2;
             byte_count -= 4;
             }
           /* Write copy remainer to TC IO space */
           /* clean up the remaining buffer transfer of 1 to 3 bytes. */
           if(byte_count == TRIBYTE_REMAINDER) {
             iobuffer_out.WORDS.WORD1 = iobuffer_in.WORDS.WORD2;
             iobuffer_in.INT.IW  = (*((int *)cur_io_in_p));
             iobuffer_out.BYTES.CHAR3 = iobuffer_in.BYTES.CHAR1;
             iobuffer_out.INT.PAD = 0;
             (*((long *)(cur_io_out_p))) = iobuffer_out.LW | TC_TRIBYTE_REMAINDER_MASK_LOW;
             }
           else {
             if (byte_count == WORD_REMAINDER ){
               iobuffer_out.WORDS.WORD1 = iobuffer_in.WORDS.WORD2;
               iobuffer_out.INT.PAD = 0;
               (*((long *)(cur_io_out_p))) = iobuffer_out.LW | TC_WORD_REMAINDER_MASK_WORD1;
               }
             else {
               if(byte_count == BYTE_REMAINDER) {
                 iobuffer_out.BYTES.CHAR1 = iobuffer_in.BYTES.CHAR3;
                 iobuffer_out.INT.PAD = 0;
                 (*((long *)(cur_io_out_p))) = iobuffer_out.LW | TC_BYTE_REMAINDER_MASK_CHAR1;
                 }
               }
             }

           break;
         case TRIBYTE_OFFSET: /* Source address !Aligned destination Aligned... */
           /* Align the Destination TC address, not necessary  */
           /* Write LW to TC IO space until done */
           iobuffer_in.INT.IW  = (*((int *)cur_io_in_p));
           (int *)cur_io_in_p += 2;
           while(byte_count >= TC_LONGWORD) {
             iobuffer_out.BYTES.CHAR1 = iobuffer_in.BYTES.CHAR4;
             iobuffer_in.INT.IW  = (*((int *)cur_io_in_p));
             (int *)cur_io_in_p += 2;
             iobuffer_out.BYTES.CHAR2 = iobuffer_in.BYTES.CHAR1;
             iobuffer_out.BYTES.CHAR3 = iobuffer_in.BYTES.CHAR2;
             iobuffer_out.BYTES.CHAR4 = iobuffer_in.BYTES.CHAR3;
             (*((int *)(cur_io_out_p))) = iobuffer_out.INT.IW;
             (int *)cur_io_out_p += 2;
             byte_count -= 4;
             }
           /* Write copy remainer to TC IO space */
           /* clean up the remaining buffer transfer of 1 to 3 bytes. */
           if(byte_count == TRIBYTE_REMAINDER) {
             iobuffer_out.BYTES.CHAR1 = iobuffer_in.BYTES.CHAR4;
             iobuffer_in.INT.IW  = (*((int *)cur_io_in_p));
             (int *)cur_io_in_p += 2;
             iobuffer_out.BYTES.CHAR2 = iobuffer_in.BYTES.CHAR1;
             iobuffer_out.BYTES.CHAR3 = iobuffer_in.BYTES.CHAR2;
             iobuffer_out.INT.PAD = 0;
             (*((long *)(cur_io_out_p))) = iobuffer_out.LW | TC_TRIBYTE_REMAINDER_MASK_LOW;
             }
           else {
             if (byte_count == WORD_REMAINDER ){
               iobuffer_out.BYTES.CHAR1 = iobuffer_in.BYTES.CHAR4;
               iobuffer_in.INT.IW  = (*((int *)cur_io_in_p));
               (int *)cur_io_in_p += 2;
               iobuffer_out.BYTES.CHAR2 = iobuffer_in.BYTES.CHAR1;
               iobuffer_out.INT.PAD = 0;
               (*((long *)(cur_io_out_p))) = iobuffer_out.LW | TC_WORD_REMAINDER_MASK_WORD1;
               }
             else {
               if(byte_count == BYTE_REMAINDER) {
                 iobuffer_out.BYTES.CHAR1 = iobuffer_in.BYTES.CHAR4;
                 iobuffer_out.INT.PAD = 0;
                 (*((long *)(cur_io_out_p))) = iobuffer_out.LW | TC_BYTE_REMAINDER_MASK_CHAR1;
                 }
               }
             }
           break;
         default: /* Here for good form... aYup! */
           break;
         }
       }
     else {  /* dstaddr is !Aligned */

       /* ---------------------------------------------------- */
       /* Source Address !Aligned Destination Address !Aligned */
       /* ---------------------------------------------------- */
       panic("io_copy from unaligned address to unaligned address not supported.");
       }
     }
   return (ret_val);
}  /* End kn16aa_io_copyio */



/*
 * dma_map_* Bus Bridge DMA routines for kn16aa platform
 */
/************************************************************************/
/*                                                                      */
/* kn16aa_dma_map_alloc - kn16 platform specific Bus Bridge DMA map     */
/*                        alloc. fcn.                                   */
/*                                                                      */
/* SYNOPSIS                                                             */
/*      long kn16_dma_map_alloc(long bc, struct controller *cntrlrp,    */
/*                              sglist_t *sglistp, int flags)           */
/*                                                                      */
/* PARAMETERS                                                           */
/*      bc      no. of bytes that system resources are being req. for   */
/*      cntrlrp ptr to controller structure for the driver that is      */
/*              requesting resources.                                   */
/*      sglistp Pointer to a ptr to data structure (that will be        */
/*              alloc'd) that will contain info on scatter-gather list  */
/*              for "bc-sized" DMA xfer.                                */
/*      flags   flag word indicating caller's desire to sleep if        */
/*              resources are busy; return w/failure if unable to       */
/*              satisfy entire "bc"-sized request                       */
/*                                                                      */
/* DESCRIPTION                                                          */
/*                                                                      */
/*      This function follows the descriptions given in the "OSF ALPHA  */
/*      Architecture Bus Bridge DMA Mapping Software Support"           */
/*      document.                                                       */
/*                                                                      */
/* RETURN                                                               */
/*      Returns "rtn_bc", the number of bytes the allocat(able) system  */
/*      resources are able to support wrt request byte count (bc).      */
/*      Never exceeds "bc".  Returns 0 on failure/no-resources.         */
/*      If rtn_bc != 0, then sglistp updated to point to allocated      */
/*      sglist structure.                                               */
/*                                                                      */
/************************************************************************/
#define NO_OPTIONAL_DMA_MAPPING_FOUND 0
#define OPTIONAL_DMA_MAPPING_DETECTED 1 
#define NOT_SG_HEAD -1
#define SG_HEAD -2

   /* To be generated by tc.c confl1 functionality and
      filled in by the confl bus support of bus-adapters
      that support DMA bus mapping. */

u_long
kn16aa_dma_map_alloc(bc, cntrlrp, sglistp, flags)

   u_long                  bc;
   struct  controller      *cntrlrp;
   sglist_t                *sglistp;
   int                     flags;
{

   struct bus           *bus_ptr;
   struct ovrhd  *sgl_ovrhdp;
   u_long                   base,rtn_bc;
   register u_int           bus_index;
   register sglist_t        sglistp_curr = (sglist_t)(NULL);
   register sglist_t        sglistp_last = (sglist_t)(NULL);
   register sg_entry_t      sg_entryp= (sg_entry_t)(NULL);
   register long            num_ents;
   register int             num_guard_pages = 0;
   struct  bus_bridge_dma_callsw  *current_hw_dma_bus_mapping;
   struct bus *optional_bus_mapping[5] = {0,0,0,0,0};

#ifdef DMA_MAP_DEBUG 
    printf("\n ENTERED kn16aa_dma_map_alloc \n");
    printf("Calling args are: \n");
    printf("\t bc      = 0x%lx \n", bc);
    printf("\t cntrlrp = 0x%lx \n", cntrlrp);
    printf("\t &sglistp = 0x%lx \n", sglistp);
    printf("\t sglistp = 0x%lx \n", *sglistp);
    printf("\t flags   = 0x%x \n", flags);
#endif 


    /*  bus_index represents wheather this system has DMA bus
     *  mapping support and if so how many are available to
     *  this device driver.
     *  0 == No hardware DMA mapping
     * >0 == # Buses that have DMA mapping in the system
     */
    bus_index = NO_OPTIONAL_DMA_MAPPING_FOUND;
    /* Bus walk from the controller to the system bus.
     * Start with the bus the controller is connected to and walk back
     * until the sytem bus is reached.
     */
    bus_ptr = cntrlrp->bus_hd;
    while(bus_ptr != NULL) {
      /* Check for DMA mapping support in this bus!  */
      if(bus_ptr->bus_bridge_dma != 0x00) { /* This bus has sg hardware support */

#ifdef DMA_MAP_DEBUG 
        printf("bus_name = %s DMA Mapping support found\n",bus_ptr->bus_name);
#endif 
        /*
         * optional_bus_mapping is an array of the bus structures in the system
         * that supports DMA sg mapping hardware. This array represents the
         * bus mappaing detected while walking from the controller to the
         * system bus.
         */
        optional_bus_mapping[bus_index] = bus_ptr;
        bus_index++;
        }
      bus_ptr = bus_ptr->bus_hd;
      } /* while bus-> */ 

      /* No optional mapping located then use direct_map_alloc() */
      /* Pelican class machine do not have TC sg DMA hardware */
      if(bus_index == NO_OPTIONAL_DMA_MAPPING_FOUND){
        rtn_bc = direct_map_alloc(bc, cntrlrp, sglistp, flags);
        /* Indicate direct map done for future reference */  
        if(rtn_bc != 0) (*sglistp)->flags |=  DMA_DIRECT_MAP; 
        return(rtn_bc);
        }

#ifdef DMA_MAP_DEBUG 
      printf(" %x Hardware sg hardware mapping buses found \n",bus_index);
#endif 

     /*
      * This structure is always allocated for the kn15aa.c file because
      * DMA scatter gather hardware is know to always exist. This will not
      * be true for other bird machines like the Pelican which will
      * use dma_direct_map() for sglist allocations when optional
      * DMA scatter gather hardware support is not detected.
      *
      * Alloc first "super" sglist struct = 2X sizeof(sglist);
      * use "super" space to store overhead: load's va & bc; zone size;
      * This sglist structure will not have a babc list allocated and
      * will contain all the calling paramaters of the kn15aa_dma_alloc
      * and kn15aa_dma_load routines.
      *
      * System Bus Scatter Gather Software Header Structure
      *     +----------------------------------+
      *     |           "sgp"=Null             |
      *     +----------------+-----------------+
      *     |   val_ents     |"num_ents"= Null |
      *     +----------------+-----------------+
      *     |     "flags"    |      index      |
      *     +----------------+--------+--------+
      *     |               next               |
      *     +----------------------------------+
      *     |             "bus_ptr"            |
      *     +----------------------------------+
      *     |               va                 |
      *     +----------------------------------+
      *     |              "bc"                |
      *     +----------------------------------+
      *     |           "procp "               |
      *     +----------------------------------+
      *  "" - Indicates initialized at this time
      */
     sglistp_curr = (sglist_t)dma_zalloc_super_sglist(flags);
     if (sglistp_curr  == (sglist_t )NULL) {

             return(0L);             /* no mem. resources! */
       }
     sgl_ovrhdp = (struct ovrhd *)(&sglistp[1]);
     sglistp_curr->flags = flags;         /* copy flags */
     sglistp_curr->next =  NULL;   /* No more sglistp...  */
     sgl_ovrhdp->cntrlrp = cntrlrp;  /* cntrlrp for dma_unload_dealloc */
     sgl_ovrhdp->procp = NULL;      /* Mark this as ancilary sglist */

     /* Terminate the babc list pointer with a Null entry! This is not the sg
        superlist pointer that will be returned to the caller. */
     sglistp_curr->sgp =  (sg_entry_t)NULL;

     /* ba = number of babc elements alloc/supported in zone, indicating that
        this is a "System SG Header".   */
     sglistp_curr->num_ents = (unsigned int)NULL;
     /* must zero 1st babc list entry for load's concat alg. to work */
     /*
      *   This first sglist structure will have the following fields
      * initialized: Captive sglist header kept for internal reference.
      * Its use does not manifest itself until a system with more than
      * one set of DMA sg hardware available.
      *
      *  sgp() - "Null" Not used in this sglist instance
      *  val_ents -  "Null" Not used in this sglist instance
      *  num_ents - "Null" Not used in this sglist instance
      *  flags -  flags
      *  index - Not Used
      *  next -  Null
      *  cntrlrp -  cntrlrp
      *  va -  "Will be filled in by the dma load call..."
      *  bc -  bc (private field )
      *  procp -  Not Used
      *
      * "optional_bus_mapping" refers to hardware scatter gather
      * DMA mapping functionality detected in the system "bus" that is
      * not part to the base kn15aa.c hardware platform support.
      *
      * In the kn15aa.c file the DMA sg mapping is treated as optional
      * although it is not "physically optional"  on this platform.
      *
      * This was done for the following reasons:
      *    . TC sg hardware allocates via the bus bridge mapping scheme
      *    . It was a waste of code space to allocate the tc sg resource
      *      outside of the following (bus bridge mapping) loop.
      *
      * Check the array for a nonzero entry, this indicates optional
      * sg DMA hardware support detected on the platform. This array
      * is constructed during the bus probe.
      */
     while((optional_bus_mapping[bus_index] != NO_OPTIONAL_DMA_MAPPING_FOUND)&&
           (bus_index >= OPTIONAL_DMA_MAPPING_DETECTED )) {

       /* Set pointer to bus bridge DMA mapping table for this resource */
       bus_ptr = optional_bus_mapping[bus_index];
       current_hw_dma_bus_mapping = bus_ptr->bus_bridge_dma;

       /* The last optional_bus_mapping[] entry is nearest to the system bus */
       /* printf(" Optional DMA bus mapping detected, Help! Help! should not be here!!!\n"); */ 
       /* save the current sglistp pointer for the allocation. */
       sglistp_last = sglistp_curr;
       /* Allocate a "super" sglist struct  = 2X sizeof(sglist);
        * use "super" space to store overhead: DMA Bus Bridge Mapping
        * This sglist structure will not have a babc list allocated and
        * will contain all the calling paramaters of the kn15aa_dma_alloc
        * and kn15aa_dma_load routines.
        *
        * Third Party Bus DMA Scatter Gather Software Header Structure
        *  ((((Base System sglistp) <- (TC map sglist)) <- (Third Party sglist)...)
        *                                                  ++++++++++++++++ sglistp_curr!
        *     +----------------------------------+
        *     |              "sgp(0)"            |
        *     +----------------+-----------------+
        *     |   val_ents     |     num_ents(1) |
        *     +----------------+-----------------+
        *     |      flags()   |      index      |
        *     +----------------+--------+--------+
        *     |               next(0)            |
        *     +----------------------------------+
        *     |             "cntrlrp(0)"         |
        *     +----------------------------------+
        *     |              va                  |
        *     +----------------------------------+
        *     |              bc                  |
        *     +----------------------------------+
        *     |              procp               |
        *     +----------------------------------+
        *  "" - Indicates initialized at this time
        *
        *  sgp() - sg_entryp
        *  val_ents - TBD
        *  num_ents - 1
        *  flags - flags
        *  index - 0
        *  next - Null
        *  cntrlrp - cntrlrp
        *  va - sglistp_curr - link back to previous sglist_t
        *  bc - base "dma load inits"
        *  procp - base
        */
       sglistp_curr = (sglist_t)dma_zalloc_super_sglist(flags);
       if (sglistp_curr == (sglist_t )NULL) {
         dma_zfree_super_sglist((vm_offset_t)sglistp_curr); /* free System Bus sglist */
         return(0L);      /* no mem. resources! */
         }
       /* Fill in the second sglist superheader with the tc sg list info */
       sgl_ovrhdp = (struct ovrhd *)(&sglistp_curr[1]);
       sglistp_curr->flags = flags;         /* copy flags */
       sglistp_curr->next =  (sglist_t)NULL;
       sgl_ovrhdp->cntrlrp = (struct controller *)(optional_bus_mapping[bus_index]); /*BusPtr*/
       /* sgl_ovrhdp->private = NULL;  /* Should probably not mess with this... */
       sglistp_curr->sgp =  (sg_entry_t)NULL;

       /* Always will be NULL, First sglistp in DMA bus bridge num_ents = NULL
        * indicates end of chain when walked in.
        */
       sglistp_curr->num_ents = (unsigned int)NULL;

       /* Allocate Turbo Channel scatter gather mapping registers */

       /* base = tc_map_alloc(bc,flags); */
       base = ((*current_hw_dma_bus_mapping->dma_alloc)(bc, flags));
       if (base == (-1)) { /* Failure */
         dma_zfree_super_sglist((vm_offset_t)sglistp_curr); /* free TC0 sglist */
         dma_zfree_super_sglist((vm_offset_t)sglistp); /* free System Bus sglist */
         return(0L);      /* no mem. resources! */
         }
       sgl_ovrhdp->va = (vm_offset_t)sglistp;  /* Link back in sglist chain  */
       sgl_ovrhdp->procp = (struct proc *)base;      /* save hardware map base offset */
       sglistp_curr->val_ents = 0;
       sglistp_curr->index = 0;             /* just in case */

       /* Try to fill babc list for first sglist struct;
        * for the majority of DMA transfers, this alloc is sufficient,
        * so optimize code path for this case.
        */

       /* num_ents = 1 unless discontinous allocation is supported */
       sg_entryp =
          (sg_entry_t)dma_zalloc(sizeof(struct sg_entry), flags);
       if(sg_entryp == (sg_entry_t)NULL) {                   /* fail if not all */
         dma_zfree_super_sglist((vm_offset_t)sglistp_curr); /* free TC0 sglist */
         dma_zfree_super_sglist((vm_offset_t)sglistp);     /* free System Bus sglist */
         /* tc_map_free(base,bc,flags); */
         ((*current_hw_dma_bus_mapping->dma_dealloc)(base, bc, flags));
         return(0L);      /* no mem. resources! */
         }
       sglistp_curr->sgp = sg_entryp;
       sglistp_curr->num_ents = 1; /* True until discontinous allocation supported */
       bzero(sg_entryp, sizeof(struct sg_entry));
       bus_index--;
       } /* while(optional_bus_mapping) */
     *sglistp = sglistp_curr;  /* set return value to the bus DMA is finally mapped*/
     return(bc);
}

/************************************************************************/
/*                                                                      */
/* kn16aa_dma_map_load - kn16aa platform specific dma map load fcn.     */
/*                                                                      */
/* SYNOPSIS                                                             */
/*      long kn15aa_dma_map_load(long bc, vm_offset_t va, struct proc   */
/*                               *procp, struct controller *cntrlrp,    */
/*                               sglist_t *sglistp, long max_bc,        */
/*                               int flags)                             */
/*                                                                      */
/* PARAMETERS                                                           */
/*      bc      no. of bytes that system resources are being req. for   */
/*      va      starting virtual address of DMA buffer to map.          */
/*      procp   process that va is valid in; procp = 0 for kernel pmap. */
/*      cntrlrp ptr to controller structure for the driver that is      */
/*              requesting resources.                                   */
/*      sglistpp Pointer to a ptr to data structure that will was       */
/*              previously alloc'd in a dma_map_alloc() call; if        */
/*              *sglistpp = 0, then an alloc needs to be done (for the  */
/*              lazy at-heart).                                         */
/*      max_bc  maximum size contiguous buffer that a ba-bc (sg_entry)  */
/*              pair should be. ba-bc list generator tries to min. no.  */
/*              of ba-bc pairs by increasing "bc" when ba's are         */
/*              contiguous.                                             */
/*      flags   flag word indicating caller's desire to sleep if        */
/*              resources are busy; return failure if unable to satisfy */
/*              entire "bc"-sized request; direction of DMA xfer        */
/*                                                                      */
/* DESCRIPTION                                                          */
/*                                                                      */
/*      This function follows the descriptions given in the "OSF ALPHA  */
/*      Architecture Bus Bridge DMA Mapping Software Support"           */
/*      document.                                                       */
/*                                                                      */
/* RETURN                                                               */
/*      Returns "rtn_bc", the number of bytes that were mapped          */
/*      wrt request byte count (bc). Never exceeds "bc".  Returns 0 on  */
/*      failure/no-available-resources.  If rtn_bc != 0, and            */
/*      *sglistpp = 0 on call-entry, then *sglistp updated to point to  */
/*      allocated sglist structure.                                     */
/*                                                                      */
/************************************************************************/
u_long
kn16aa_dma_map_load(bc, va, procp, cntrlrp, sglistpp, max_bc, flags)
   long                    bc;
   vm_offset_t             va;
   struct  proc            *procp;
   struct  controller      *cntrlrp;
   sglist_t                *sglistpp;
   long                    max_bc;
   int                     flags;
{
   long    rtn_bc = bc;
   int     base, bus_index,current_index;
   u_long  bus_address;
   struct  bus     *bus_ptr;
   struct  sglist  *sglistp = *sglistpp;
   struct  sglist  *sglistp_last = *sglistpp;
   struct  sglist  *sglistp_curr,*sglistp_next;
   struct  ovrhd  *sgl_ovrhdp_curr, *sgl_ovrhdp_next;
   struct  ovrhd  *sgl_ovrhdp_last;
   sg_entry_t      sg_entryp= (sg_entry_t)(NULL);
   struct  bus_bridge_dma_callsw  *current_hw_dma_bus_mapping;
   struct bus *optional_bus_mapping[5] = {0,0,0,0,0};

#ifdef DMA_MAP_DEBUG 
    printf("\n ENTERED kn116aa_dma_map_load \n");
    printf("Calling args are: \n");
    printf("\t bc      = 0x%lx \n", bc);
    printf("\t va      = 0x%lx \n", va);
    printf("\t procp   = 0x%lx \n", procp);
    printf("\t cntrlrp = 0x%lx \n", cntrlrp);
    printf("\t sglistp = 0x%lx \n", sglistpp);
    printf("\t flags   = 0x%x \n", flags);
#endif 

    rtn_bc = bc;
    if(bc != max_bc) return(0L);
    /* sglistp in Null so make effort to call dma_map_alloc... */
    if (sglistp_last == (sglist_t)0) {
           rtn_bc = kn16aa_dma_map_alloc(bc,cntrlrp,sglistpp,flags);
           if (rtn_bc == 0L)       {   /* drop-out on failure */
                   *sglistpp = (sglist_t)0;
                   return(0L);
             }
       }

    sglistp = *sglistpp;

#ifdef DMA_MAP_DEBUG 
    printf("\n ENTERED kn15aa_dma_map_load again...  \n");
    printf("Calling args are: \n");
    printf("\t bc      = 0x%lx \n", bc);
    printf("\t rtn_bc  = 0x%lx \n", rtn_bc );
    printf("\t va      = 0x%lx \n", va);
    printf("\t procp   = 0x%lx \n", procp);
    printf("\t cntrlrp = 0x%lx \n", cntrlrp);
    printf("\t sglistp = 0x%lx \n", sglistp);
    printf("\t flags   = 0x%lx \n", (*sglistpp)->flags );
    printf("\t flags       = 0x%lx \n", (sglistp)->flags );
    printf("\t sgp         = 0x%lx \n", (sglistp)->sgp );
    printf("\t sgp.ba      = 0x%lx \n", (sglistp)->sgp[0].ba);
    printf("\t sgp.bc      = 0x%lx \n", (sglistp)->sgp[0].bc);
    printf("\t val_ents    = 0x%lx \n", (sglistp)->val_ents);
    printf("\t mum_ents    = 0x%lx \n", (sglistp)->num_ents);
    printf("\t index       = 0x%lx \n", (sglistp)->index );
#endif 

    /*  bus_index represents wheather this system has DMA bus
     *  bridge mapping support and if so how many are available to
     *  this device driver.
     *  0 == No hardware DMA mapping
     * >0 == # Buses that have DMA mapping in the system
     *  1 == TC sg DMA hardware present on all platforms...
     */
    bus_index = NO_OPTIONAL_DMA_MAPPING_FOUND;
    /* Bus walk from the controller to the system bus.
     * Start with the bus the controller is connected to and walk back
     * until the system bus is reached.
     */
    bus_ptr = cntrlrp->bus_hd;
    while(bus_ptr != NULL) {
      /* Check for DMA mapping support in this bus!  */
      if(bus_ptr->bus_bridge_dma != 0x00) { /* This bus has sg hardware support */
        /*
         * optional_bus_mapping is an array of the bus structures in the system
         * that supports DMA sg mapping hardware. This array represents the
         * bus mapping detected while walking from the controller to the
         * system bus.
         */
        optional_bus_mapping[bus_index] = bus_ptr;
        bus_index++;
        }
      bus_ptr = bus_ptr->bus_hd;
      }

      /* No optional mapping located in system then use direct_map_load  */
      /* Pelican class machines do not have TC sg DMA hardware */
      if(bus_index == NO_OPTIONAL_DMA_MAPPING_FOUND){
        /* now, load the allocated map(s); failure = 0 */ 
        if(direct_map_load(rtn_bc, va, procp, sglistp, max_bc))
          return(rtn_bc);
        else
          return(0L);
        }

      /*
       * System Bus Scatter Gather Software Header Structure
       *     +----------------------------------+
       *     |           "sgp"=Null             |
       *     +----------------+-----------------+
       *     |   val_ents     |"num_ents"= Null |
       *     +----------------+-----------------+
       *     |     "flags"    |      index      |
       *     +----------------+--------+--------+
       *     |               next               |
       *     +----------------------------------+
       *     |             "bus_ptr"            |
       *     +----------------------------------+
       *     |               va                 |
       *     +----------------------------------+
       *     |              "bc"                |
       *     +----------------------------------+
       *     |            "procp"               |
       *     +----------------------------------+
       *  "" - Indicates initialized at this time
       *
       *   This FIRST sglist structure will have the following fields
       * initialized: Captive sglist header kept for internal reference.
       * Its use does not manifest itself until a system with more than
       * one set of DMA sg hardware available.
       *
       *  sgp() - Null
       *  val_ents - 0
       *  num_ents - Null
       *  flags -  flags - do not mean anything to tc_load...
       *  index - 0
       *  next -  Null
       *  cntrlrp -  cntrlrp
       *  va - va from the dma load call
       *  bc -  private
       *  procp -  procp from the dma map load call
       *  kn15aa_dma_map_load(bc, va, procp, cntrlrp, sglistpp, max_bc, flags)
       */

       sgl_ovrhdp_last->va = va;            /* save for dma_kmem_buffer     */
       sgl_ovrhdp_last->procp = procp;      /* save for dma_kmem_buffer     */
       sglistp_last->val_ents = 0;
       sglistp_last->index = 0;             /* just in case */


      /* There is hardware map loading to be done */
       while(bus_index >= OPTIONAL_DMA_MAPPING_DETECTED )
         {
         /* Make sglistp_curr point to the current Bus Bridge DMA
          * sglistp structure.
          *
          *  bus_index represents wheather this system has DMA bus
          *  bridge mapping support and if so how many are available to
          *  this device driver.
          *  0 == No hardware DMA mapping
          * >0 == # Buses that have DMA mapping in the system
          *  1 == TC sg DMA hardware present on all platforms...
          */
          sglistp_curr = *sglistpp;
          current_index = bus_index - 1;
          while(current_index >=  0x01) {
                              /* Walk the sglistpp index back to the correct point */
                              /* It sure would be nice to have forward and backward */
                              /* links!!!                                           */
            sgl_ovrhdp_curr = (struct ovrhd *)(&sglistp_curr[1]);
            sglistp_curr = (sglist_t)sgl_ovrhdp_curr->va;
            current_index--;
            }
          sgl_ovrhdp_curr = (struct ovrhd *)(&sglistp_curr[1]);
          /*
           *   These following sglist structure will have the following fields
           * initialized: Captive sglist header kept for internal reference.
           * Its use does not manifest itself until a system with more than
           * one set of DMA sg hardware available.
           *
           *  sgp() - Pointer to single pair...
           *  val_ents - 0
           *  num_ents - 1
           *  flags -  flags - do not mean anything to tc_dmaload...  Fix!!
           *  index - 0
           *  next -  Null
           *  cntrlrp -  Bus structure pointer
           *  va - pointer to the previous sglist  , setup during alloc
           *  bc -  private
           *  procp -  base of the hardware maps  , setup during alloc
           *
           *  kn15aa_dma_map_load(bc, va, procp, cntrlrp, sglistpp, max_bc, flags)
           */
           sglistp_curr->val_ents = 0;
           sglistp_curr->index = 0;             /* just in case */
           sg_entryp = sglistp_last->sgp;
           base = (u_long)sgl_ovrhdp_curr->procp;

           /* get a pointer to the current DMA sg hardware call table */
           bus_ptr = (struct bus *) sgl_ovrhdp_curr->cntrlrp;
           current_hw_dma_bus_mapping = bus_ptr->bus_bridge_dma;

           /* load the tc DMA sg mapping registers.
            * tc_loadmap(procp,base,va,bc,flags); Not!! a standard interface...
            * Flags definition needs to be changed...   This will be handled in
            * the Jacket Routines for the tc sg hardware. 
            */
           bus_address = ((*current_hw_dma_bus_mapping->dma_load)(procp, base, va, bc, flags));

           sg_entryp = sglistp_curr->sgp;
           sg_entryp->ba = (bus_addr_t)bus_address;
           sg_entryp->bc = bc;
           bus_index--;
           }
   return(rtn_bc);
}


/************************************************************************/
/*                                                                      */
/* kn16aa_dma_map_unload  - invalidate address mapping                  */
/*                                                                      */
/* SYNOPSIS                                                             */
/*      int kn15aa_dma_map_unload(int flags, struct sglist *sglistp)    */
/*                                                                      */
/* PARAMETERS                                                           */
/*      sglistp Pointer to data structure that was previously alloc'd   */
/*              in a dma_map_alloc() call;                              */
/*      flags   flag word indicating caller's desire to have system     */
/*              resources deallocated, as well as mapping invalidated.  */
/*                                                                      */
/* DESCRIPTION                                                          */
/*                                                                      */
/*      This function follows the descriptions given in the "OSF ALPHA  */
/*      Architecture Bus Bridge DMA Mapping Software Support"           */
/*      document.                                                       */
/*                                                                      */
/* RETURN                                                               */
/*      A successful return is one (1). A failure a zero (0).           */
/*                                                                      */
/************************************************************************/
int
kn16aa_dma_map_unload(flags, sglistp)
   int  flags;
   struct  sglist *sglistp;
{
   int     bc;
   long  base,rtn_status;
   struct bus  *bus_ptr;
   struct  sglist  *sglistp_curr,*sglistp_next;
   struct  ovrhd  *sgl_ovrhdp;
   sg_entry_t      sg_entryp= (sg_entry_t)(NULL);
   struct  bus_bridge_dma_callsw  *current_hw_dma_bus_mapping;
   struct bus *optional_bus_mapping[5] = {0,0,0,0,0};

#ifdef DMA_MAP_DEBUG 
    printf("\n ENTERED kn16aa_dma_map_unload \n");
    printf("Calling args are: \n");
    printf("\t flags    = 0x%lx \n", flags);
    printf("\t sglistp = 0x%lx \n", sglistp);
    printf("\t flags       = 0x%lx \n", (sglistp)->flags );
    printf("\t sgp         = 0x%lx \n", (sglistp)->sgp );
    printf("\t sgp.ba      = 0x%lx \n", (sglistp)->sgp[0].ba);
    printf("\t sgp.bc      = 0x%lx \n", (sglistp)->sgp[0].bc);
    printf("\t val_ents    = 0x%lx \n", (sglistp)->val_ents);
    printf("\t mum_ents    = 0x%lx \n", (sglistp)->num_ents);
    printf("\t index       = 0x%lx \n", (sglistp)->index );
#endif 

   if (sglistp == (sglist_t)NULL)
       return(0);

   /* This function walks the bus bridge DMA sglistp structure chain
    * unloading the hardware DMA mapping functionality.
    */
   sgl_ovrhdp = (struct ovrhd *)(&sglistp[1]);
   sglistp_curr = sglistp;

   /*
    * if flags has DMA_DEALLOC set, dealloc resources;
    * mirror of dma_map_load where sglistp == 0 means do an alloc
    */
   if(sglistp->flags & DMA_DIRECT_MAP) {
     if (flags & DMA_DEALLOC ) {      /* dealloc as well*/
        rtn_status = direct_map_unload(sglistp);
        if(rtn_status == 0x1)
           return(direct_map_dealloc(sglistp));
        else
          return(0L); 
        }
     else
        return(direct_map_unload(sglistp));
     } 

   /* The head of the sglist "bridge" structure chain has num_ents = 0 */
   /* or now the flags field has a Primary bit...                      */ 
   while(sglistp_curr->num_ents != 0 ){
      sg_entryp = sglistp_curr->sgp;
      sglistp_next =  (sglist_t)sgl_ovrhdp->va; /* Pointer back bus bridge sglist */ 

      /* Recover the bus structure pointer for this DMA Mapping */
      bus_ptr = (struct bus *)sgl_ovrhdp->cntrlrp;
      current_hw_dma_bus_mapping = bus_ptr->bus_bridge_dma;

      /* Unload the Bus mapping hardware */
      base =  (u_long)sgl_ovrhdp->procp; /* Mapping register base address */
      /* Should check for multiple ba-bc pair and sglist but for now */ 
      bc = sg_entryp->bc;   /* I know there is only a single ba-bc pair */
      if(((*current_hw_dma_bus_mapping->dma_unload)(base, bc, sglistp->flags)))
        { /* exit on error condition returned */ 
        } 

      sglistp_curr = sglistp_next;
      sgl_ovrhdp = (struct ovrhd *)(&sglistp[1]);
      }
    if(sglistp->flags & DMA_DEALLOC) {
      if(kn16aa_dma_map_dealloc(sglistp)) { /* exit on error condition returned */ 
        }
      } 
    return(1);
}



/************************************************************************/
/*                                                                      */
/* kn16aa_dma_map_dealloc - deallocates system DMA mapping resources    */
/*                                                                      */
/* SYNOPSIS                                                             */
/*      int kn15aa_dma_map_dealloc(struct sglist *sglistp)              */
/*                                                                      */
/* PARAMETERS                                                           */
/*      sglistp ptr to data structure that was previously alloc'd       */
/*      in a dma_map_alloc() call;                                      */
/*                                                                      */
/* DESCRIPTION                                                          */
/*                                                                      */
/*      This function follows the descriptions given in the "OSF ALPHA  */
/*      Architecture Bus Bridge DMA Mapping Software Support"           */
/*      document.                                                       */
/*                                                                      */
/*                                                                      */
/* RETURN                                                               */
/*      A successful return is one (1). A failure a zero (0).           */
/*                                                                      */
/************************************************************************/
int
kn16aa_dma_map_dealloc(sglistp)
        struct  sglist *sglistp;
{
   int     bc;
   long  base;
   struct bus  *bus_ptr;
   struct  sglist  *sglistp_curr,*sglistp_next;
   struct  ovrhd  *sgl_ovrhdp;
   sg_entry_t      sg_entryp= (sg_entry_t)(NULL);
   struct  bus_bridge_dma_callsw  *current_hw_dma_bus_mapping;

#ifdef DMA_MAP_DEBUG
    printf("\n ENTERED kn16aa_dma_map_dealloc \n"); 
    printf("\t flags       = 0x%lx \n", (sglistp)->flags );
    printf("\t sgp         = 0x%lx \n", (sglistp)->sgp );
    printf("\t sgp.ba      = 0x%lx \n", (sglistp)->sgp[0].ba);
    printf("\t sgp.bc      = 0x%lx \n", (sglistp)->sgp[0].bc);
    printf("\t val_ents    = 0x%lx \n", (sglistp)->val_ents);
    printf("\t mum_ents    = 0x%lx \n", (sglistp)->num_ents);
    printf("\t index       = 0x%lx \n", (sglistp)->index );
#endif

   /* Null sglistp to this function is an error */
    if (sglistp == (sglist_t)NULL)
        return(0);

   /* This function walks the bus bridge DMA sglistp structure chain
    * releasing/deallocating all the memory and hardware DMA resources.
    * It is the responsibility of a bus adapters dealloc function to not
    * just dealloc but to reinit/shutoff the mapping registers.
    */
   sgl_ovrhdp = (struct ovrhd *)(&sglistp[1]);
   sglistp_curr = sglistp;

   if (sglistp->flags  & DMA_DIRECT_MAP) {    
      return(direct_map_dealloc(sglistp));
      }

   /* The head of the sglist "bridge" structure chain has num_ents = 0 */
   while(sglistp_curr->num_ents != 0 ){
      sg_entryp = sglistp_curr->sgp;
      sglistp_next =  (sglist_t)sgl_ovrhdp->va;

      /* Recover the bus structure pointer for this DMA Mapping */
      bus_ptr = (struct bus *)sgl_ovrhdp->cntrlrp;

      current_hw_dma_bus_mapping = bus_ptr->bus_bridge_dma;

      /* Free the Bus mapping hardware */
      base =  (u_long)sgl_ovrhdp->procp; /* Mapping register base address */
      bc = sg_entryp->bc;
      ((*current_hw_dma_bus_mapping->dma_dealloc)(base, bc, sglistp->flags ));

      /* free the ba-bc pair (1) */
      dma_zfree(sizeof(struct sg_entry)*sglistp->num_ents,
                       (vm_offset_t)sglistp->sgp );

      /* free the super sglist structure */
      dma_zfree_super_sglist((vm_offset_t)sglistp_curr);

      sglistp_curr = sglistp_next;
      sgl_ovrhdp = (struct ovrhd *)(&sglistp[1]);
      }
    dma_zfree_super_sglist((vm_offset_t)sglistp_curr);
    return(1);
}

/************************************************************************/
/*                                                                      */
/* kn16aa_dma_min_boundary  -  system DMA atomicity transfer size.      */
/*                                                                      */
/* SYNOPSIS                                                             */
/*      int kn15_dma_min_boundary(struct controller *cntrlrp)           */
/*                                                                      */
/* PARAMETERS                                                           */
/*      cntrlrp ptr to controller structure for the driver that is      */
/*              requesting resources.                                   */
/*                                                                      */
/* DESCRIPTION                                                          */
/*      This function returns the number of bytes a DMA transfer        */
/*      is guaranteed to be atomic on a particular system &/or io bus   */
/*      attached to a system.
/*                                                                      */
/* RETURN                                                               */
/*      Always returns a "4" on Pelican, for longword-level DMA         */
/*      atomicity.                                                      */
/*                                                                      */
/************************************************************************/
int
kn16aa_dma_min_boundary(cntrlrp)
        struct controller *cntrlrp;
{
#ifdef DMA_MAP_DEBUG
        printf("\n ENTERED kn15aa_dma_min_boundary \n");
#endif
        return((int)(4));
}


