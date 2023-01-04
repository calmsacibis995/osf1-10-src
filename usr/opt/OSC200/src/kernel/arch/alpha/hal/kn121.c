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
static char *rcsid = "@(#)$RCSfile: kn121.c,v $ $Revision: 1.1.13.14 $ (DEC) $Date: 1994/01/21 20:43:24 $";
#endif
/*
 * OLD HISTORY
 * Revision 1.1.2.11  1993/03/08  21:13:33  Gary_Dupuis
 * 	Add definition of bus access widths; Jensen_Widths
 * 			  bus address shifts; Jensen_Addr_Shifts
 * 			  bus low address mask; Jensen_Low_Addr_Masks
 * 			  IO types; Jensen_IO_Types
 * 	[1993/03/08  20:23:41  Gary_Dupuis]
 *
 * Revision 1.1.2.10  93/01/07  15:51:49  Timothy_Burke
 * 	Added call to enable_spls to prepare enable interrupts to be lowered.
 * 	[93/01/07  15:49:47  Timothy_Burke]
 * 
 * Revision 1.1.2.9  93/01/07  14:20:52  Paul_Grist
 * 	Merge fix - remove unecessary syscall tracing stuff
 * 	[93/01/07  14:16:46  Paul_Grist]
 * 
 * Revision 1.1.2.8  92/12/28  16:27:54  Timothy_Burke
 * 	Removed call to parallel port probe.  That is now done out of the
 * 	eisa bus configuration code.
 * 	[92/12/28  16:27:18  Timothy_Burke]
 * 
 * Revision 1.1.2.7  92/12/23  16:24:04  Timothy_Burke
 * 	Somehow the setting of IRQ11 got messed up. ???
 * 	[92/12/23  16:23:28  Timothy_Burke]
 * 
 * Revision 1.1.2.6  92/12/23  15:39:51  Timothy_Burke
 * 	Call parallel port driver probe routine.
 * 	[92/12/23  15:36:38  Timothy_Burke]
 * 
 * Revision 1.1.2.5  92/11/13  16:30:44  Paul_Grist
 * 	Inital EISA scsi support submitted for JTW - modified PIC routine for
 * 	AHA scsi - interrupt (PIC) handler needs work.
 * 	[92/11/13  16:00:18  Paul_Grist]
 * 
 * Revision 1.1.2.4  92/10/30  10:47:57  Timothy_Burke
 * 	Call kbd_init to initialize the keyboard driver.
 * 	[92/10/30  10:41:00  Timothy_Burke]
 * 
 * Revision 1.1.2.3  92/10/28  16:45:45  Paul_Grist
 * 	First-pass cleanup, detailed comments below. To summarize: added
 * 	82357 PIC support, jensen version port routines, split beta code
 * 	out to ka_beta.c. Much, much cleanup and work still needed.
 * 	[92/10/28  15:57:36  Paul_Grist]
 * 
 * Revision 1.1.2.2  92/10/12  07:44:03  Gary_Dupuis
 * 	Initial insertion into live pool.
 * 	[92/10/09  09:53:07  Gary_Dupuis]
 * 
 */

/*
 * Modification History: src/kernel/arch/alpha/hal/ka_jensen.c
 *			 JENSEN Alpha PC - EISA system
 *
 * 24-Sep-92 	Paul Grist
 *	1. Modifications to allow interrupts to work on Jensen
 *	2. Make BETA/JENSEN in/out Vti routines set-up at runtime
 *	   instead of in cpuconf.c target build. Necessary while
 *	   we have no callbacks.
 *	3. Moved i/o addressing routines from isa.c here and hacked
 *	   the ones common to BETA to select the appropriate version
 *	   of the port routine during runtime. This can be cleaned
 *	   up with the forthcoming generic I/O addressing routines.
 *	4. For now, there are 3 routines common to beta and jensen:
 *		a. buslevel-to-irqchan conversion
 *		b. eoi, end-of-intr -- 82357 for jensen, 82380 for beta
 *		c. pic maskon -- interrupt enable for PIC
 *	   Move BETA versions to ka_beta.c, still more seperation needed.
 *	   i.e. eoi and i/o addressing stuff, and later even could
 *	   have sperate conf, etc...
 *
 *	TODO: Need generic I/O routine for EISA, and means for seperate
 *	      vti routines in ace.c.  interrupt handler (82357 PIC) needs
 *	      serious work to be generic, readable, and perform well.
 *
 * 06-Aug-92	Tim Burke
 * 	Added in Theta-2 changes.
 *
 * 03-Aug-92    Tim Burke
 *	For BL7 merge removed #includes from binlog.  This has been defuncted.
 *
 * 17-Jul-92	tml
 *	Changed long to u_long in apc_vector_alloc.
 *
 * 26-May-92	treese
 *	Converted for Alpha/OSF
 *
 * 02-Aug-91	jn
 *      Created this file for processor support of the BETA Alpha PC
 *          hacked from ADU version
 */

#include <sys/types.h>
#include <machine/psl.h>
#include <machine/rpb.h>
#include <machine/scb.h>
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
#include <mach/boolean.h>
#include <sys/vmmac.h>
#include <sys/table.h>
#include <machine/reg.h>
#include <machine/entrypt.h>
#include <hal/cpuconf.h>
#include <io/common/devdriver.h>
#include <io/dec/eisa/eisa.h>
#include <hal/kn121.h>
#include <hal/82357_pic.h>
#include <dec/binlog/binlog.h>
#include <dec/binlog/errlog.h>
#include <hal/mc146818clock.h>

/* #define BEEP_TEST */		/* for beep at single user */
/* #define DISK_LED_FLASH */		/* for flash of disk LED 1/sec */
/* #define DMA_MAP_DEBUG 	/* for printf's in dma_map* code */

/*
 *  Error handling declarations
 */
static int kn121_mchk_ip = 0;     /* set when machine check in progress */
static int kn121_correrr_ip = 0;  /* setwhen corrected error in progress */
extern int mcheck_expected;	  /* flag used to communicate between badaddr
				     and Mcheck handling */
/*
 *	General CPU declarations
 */
extern struct cpusw *cpup;	/* pointer to cpusw entry */
extern int cold;		/* for cold restart status */
extern int printstate;		/* determines origin of prints: console/OSF */
extern struct	eisa_sw	   eisa_sw;	/* Eisa function switch table. */
extern struct	eisa_slot  eisa_slot[];	/* Eisa slot table. */


/*
 *	Definition of bus info structures.
 */
/*----------------------------------------------------------------*/
/* The agreed upon format for bus info structures is		  */
/*								  */
/*	+----------------------------------------+		  */
/*	|  Pointer to common info block  	 |		  */
/*	|  (this is a struct common_bus_info *)  |		  */
/*	+----------------------------------------|		  */
/*	|  Bus specific data block               |		  */
/*	+----------------------------------------+		  */
/*								  */
/* For kn121 machines there is no bus specific data for the ibus. */
/* Therefore we simply use the bus_info_hdr structure defined in  */
/* devdriver.h.							  */
/*----------------------------------------------------------------*/

struct	common_bus_info	kn121_ibus_common_info;
struct	common_bus_info	kn121_eisa_common_info;
struct	bus_info_hdr	bus_info;


/*
 * Define mapping of interrupt lines with the type of interrupt.
 * Needed for the stat programs.
 * see sys/types.h and io/common/handler.c
 */

static int kn121_interrupt_type[INTR_MAX_LEVEL] = {
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
 * Forward declarations.
 */
static int kn121_palcode_type, kn121_palcode_version, kn121_palcode_revision;
static int kn121_firmware_major, kn121_firmware_minor;

int  pic_enable_irq(), pic_disable_irq();
int  pic_set_irq_level();
int  pic_set_irq_edge ();
int  irq_to_scb_vector();
int  isp_dma_init ();
int  isp_dma_config ();
int  isp_dma_intr ();

/*
 * Prototypes
 */
u_long	kn121_dma_map_alloc();
u_long	kn121_dma_map_load();
int 	kn121_dma_map_unload();
int 	kn121_dma_map_dealloc();
int 	kn121_dma_min_boundary();
int	kn121_pic_intr();
io_handle_t kn121_busphys_to_iohandle();

extern	long	direct_map_alloc();
extern	int	direct_map_load();
extern	int	direct_map_dealloc();
/* io access prototype decl's */
long	kn121_read_io_port();
void	kn121_write_io_port();
u_long	kn121_iohandle_to_phys();
int	kn121_io_copyin();
int	kn121_io_copyout();
int	kn121_io_copyio();
int     kn121_io_zero();
/* to be eliminated in gold; backward compatibility for now */
vm_offset_t	kn121_get_io_handle();
int	kn121_io_bcopy();

#define WRITE_EISA_D8(a,d)   WRITE_BUS_D8((kn121_eisa_common_info.sparse_io_base|a),d)
#define WRITE_EISA_D16(a,d)  WRITE_BUS_D16((kn121_eisa_common_info.sparse_io_base|a),d)
#define WRITE_EISA_D32(a,d)  WRITE_BUS_D32((kn121_eisa_common_info.sparse_io_base|a),d)
#define READ_EISA_D8(a)      READ_BUS_D8((kn121_eisa_common_info.sparse_io_base|a))
#define READ_EISA_D16(a)     READ_BUS_D16((kn121_eisa_common_info.sparse_io_base|a))
#define READ_EISA_D32(a)     READ_BUS_D32((kn121_eisa_common_info.sparse_io_base|a))

#define VTI_BASE kn121_ibus_common_info.sparse_io_base    

/****************************************************************************
 * 
 * kn121_read_palcode_version 
 *
 * Return PALcode version, revision, and type 
 *
 ****************************************************************************/
int kn121_read_palcode_version()
{
	extern struct rpb *rpb;
	struct rpb_percpu *percpu;

	/* get address of the cpu specific data structures for this cpu */
        percpu = (struct rpb_percpu *) ((long)rpb + rpb->rpb_percpu_off);
	kn121_palcode_type     = (percpu->rpb_palrev & 0xff0000) >> 16;
	kn121_palcode_version  = (percpu->rpb_palrev & 0xff00) >> 8;
	kn121_palcode_revision = percpu->rpb_palrev & 0xff;
	kn121_firmware_major = (percpu->rpb_firmrev & 0xff00) >> 8;
	kn121_firmware_minor = percpu->rpb_firmrev & 0xff;
	return (0);
}



/*****************************************************************************
 *
 * kn121_init()
 *
 * Jensen CPU initialization routine.
 * Called through the cpusw
 *
 *****************************************************************************/
kn121_init()
{
    int edge1reg, edge2reg;
    extern volatile int *system_intr_cnts_type_transl;
    struct controller *ctlr, *get_ctlr_num();
    extern io_handle_t	mc146818clock_rap;
    extern io_handle_t	mc146818clock_rdp;
    extern io_handle_t  pic_base;
    extern io_handle_t  isp_base;
    extern int		eisa_expansion_slots;
    
    /*
     * Block interrupts until we say so when we call enable_spls
     * in kn121_conf()
     */
    splextreme();

    /*
     * Working on cold restart (for autoconf and kern_cpu)
     */
    cold = 1;

    /*
     * Setup io access jump/function table
     * Note: get_io_handle & io_bcopy need to be cleaned
     *	     out in next release
     */
    ioaccess_callsw_init(kn121_read_io_port, kn121_write_io_port,
			 kn121_iohandle_to_phys, kn121_io_copyin,
			 kn121_io_copyout, kn121_io_copyio,
			 kn121_io_zero,
			 kn121_get_io_handle, kn121_io_bcopy);

    /*
     * Setup global interrupt type array to use jensen definitions.
     */
    system_intr_cnts_type_transl = kn121_interrupt_type;

    /* Initialize bus info structures for the ibus and the eisa bus.
     * Sysmaps used are
     *
     * For Jensen and Culzean the sysmap is a 32 bit quantity of which
     * the low order six bits are used.
     *
     *                31                            5    0
     *               +------------------------------------+
     *               |  RESERVED                   |SYSMAP| 
     *               +------------------------------------+
     *
     *  Bits  <31-6> are reserved. Bits <5-0> contain the sysmap
     *  information and can have the following values.
     *
     *    011100    Local bus sparse IO address space.
     *    100000    EISA bus sparse memory address space.
     *    110000    EISA bus sparse IO address space.
     *
     */
    
    kn121_ibus_common_info.intr = (void *)NULL;
    kn121_ibus_common_info.sparse_io_base = 0x1C00000000L;
    kn121_ibus_common_info.sparse_mem_base = 0;
    kn121_ibus_common_info.dense_mem_base = 0;
    
    kn121_eisa_common_info.intr = (void *)NULL;
    kn121_eisa_common_info.sparse_io_base = 0x3000000000L;
    kn121_eisa_common_info.sparse_mem_base = 0x2000000000L;
    kn121_eisa_common_info.dense_mem_base = 0;

    /* 
     * Fill in the eisa switch table.
     */
    eisa_sw.config_base_addr = 0x1a0000000L;
    eisa_sw.config_stride = 0x200;
    eisa_sw.sparse_io_base =  kn121_eisa_common_info.sparse_io_base;
    eisa_sw.sparse_mem_base = kn121_eisa_common_info.sparse_mem_base;
    eisa_sw.dense_mem_base = 0;
    eisa_sw.busphys_to_iohandle = kn121_busphys_to_iohandle;
    eisa_sw.enable_option = pic_enable_irq;
    eisa_sw.disable_option = pic_disable_irq;
    eisa_sw.set_irq_edge = pic_set_irq_edge;
    eisa_sw.set_irq_level = pic_set_irq_level;
    eisa_sw.dma_init = isp_dma_init;
    eisa_sw.dma_config = isp_dma_config;
    eisa_sw.irq_to_scb_vector = irq_to_scb_vector;
    eisa_sw.intr_dispatch = kn121_pic_intr;
    

    /*
     * Fill EISA specific variables.
     */
    eisa_expansion_slots = 8;	/* 0 system, 1-6 expansion, 7 floppy */
    
    /*
     * Initialize the Progrmmable Interrupt Controller (pic) base address
     */
    pic_base =  kn121_eisa_common_info.sparse_io_base;

    /*
     * Initialize the Integrated System Peripheral (isp) base address
     */
    isp_base =  kn121_eisa_common_info.sparse_io_base;


    /*
     * Enable the NMI interrupt
     */
    WRITE_EISA_D8(PC_RAP_BASE,0x0);
    mb();

    /*
     * Initialize global clock register locations for
     * mc146818clock_xxx routines
     */
    mc146818clock_rap = VTI_BASE + KN121_RTC_RAP_BASE;
    mc146818clock_rdp = VTI_BASE + KN121_RTC_RDP_BASE;

    /*
     * Setup dma_map_* jump/function table
     */
    dma_callsw_init(kn121_dma_map_alloc,kn121_dma_map_load,
		      kn121_dma_map_unload,kn121_dma_map_dealloc,
		      kn121_dma_min_boundary);

    /*
     * Intel 82357 PIC Initialization & Jensen implementation
     *
     * Set up the PIC chip which handles all the
     * EISA interupts and the line printer through
     * the IO interrupt entrypoint.
     *
     * Jensen usage of PIC:
     *
     *	o IRQ priority scheme is default: IRQ1 highest to IRQ15 lowest
     *
     *  o Jensen PALcode handles interrupt acknowledge and
     *	  dispatch.
     *
     *
     *	Note: All the 82357 pic routines/defs live in pic.c/h so they will
     *	      be re-usable by other systems using pic chips.
     *
     */

    /*
     * Set Special Mask Mode as an EOI precaution
     */
    WRITE_EISA_D8(CTRL2_OCW3,SET_SPMASK); /*CTLR 2*/ 
    WRITE_EISA_D8(CTRL1_OCW3,SET_SPMASK); /*CTLR 1*/ 

    /*
     * Enable Non Maskable Interrupt (NMI) masks
     */
    WRITE_EISA_D16(NMI_CTRL,NMI_PARITY | NMI_IOCHK);
    WRITE_EISA_D16(NMI_ECTRL,NMI_FAILSAFE | NMI_TIMEOUT);

    /* 
     * Initialize PIC ELR to set all IRQ interrupts to Edge trigger,
     * This should be power-up default, but let's do it any way
     * and let EISA bus code set as needed
     */
    WRITE_EISA_D8( PIC_ELR2,0x0);           
    WRITE_EISA_D8( PIC_ELR1,0x0); 

    /* 
     * Initialize all PIC interrupts to be disabled just in case
     * They will be enabled through EISA bus support
     */
    WRITE_EISA_D8( CTRL2_OCW1,0xff);  /* CTRL2 */    
    WRITE_EISA_D8( CTRL1_OCW1,0xff);  /* CTRL1 */

    /*
     * Read the PALcode and firmware versions
     */
    (void)kn121_read_palcode_version();


    /*
     * Intitalize HAE to zero
     */
    kn121_write_hae(0x0);

    /*
     * Deal with ACE and gpc local I/O configuration.
     *
     * The strategy for dealing with the jensen local-io-specific 
     * swizzle is to encode the vti base address in the i/o address
     * passed to the generic kn121_read/write_port routines. The
     * mechanisim for seamlessly handling this and keeping it from
     * the local i/o drivers is to place the vti base address in
     * the ctlr->physaddr which drivers use as their base I/O address.
     *
     * Unfortunately,  cons_init will be called before we
     * can really configure ace0 and gpc0, where the ctrl->physaddr
     * would normally be initialized
     *
     * Fill-in the physaddr for the ace controller struct so that
     * it will hold the VTI base address during ace_cons_init().
     * If ace0 or gpc0 are not in the config file,  things will
     * eventually break down, so there is a printf indcating so.
     *
     * This is the best place to manage this jensen-specific i/o 
     * problem (local i/o has different swizzle),  vs. putting
     * cpu knowledge into the ace driver which is neither jensen-specific.
     * Nor bus-specific. This also gives the gpc driver the option
     * of getting it's base address from the ctlr list vs a cpu case
     * statement.
     *
     */

    if( (ctlr = get_ctlr_num("ace",0)) == (struct controller *)NULL)
	printf("cons_init: ace0 ctlr struct not found");
    else  
	(u_long)ctlr->physaddr = (u_long)VTI_BASE;

    if( (ctlr = get_ctlr_num("gpc",0)) == (struct controller *)NULL)
	printf("cons_init: gpc0 ctlr struct not found");
    else  
	(u_long)ctlr->physaddr = (u_long)VTI_BASE;

    return(0);
}


/**************************************************************************
 *
 * kn121_pic_intr()
 *
 * This is the central interrupt handler for eisa interrupts on kn121
 * platforms.  This interrupt handler is loaded into the SCB with the
 * slot number of the appropriate option as the argument.  This central
 * dispatch allows for doing the eoi processing in cpu-specific code vs.
 * having the drivers need to call an eoi routine.  The old pic_eoi() code
 * is inline in this function. 
 *
 * The scb vectors are filled in in the eisa bus code,  and the default
 * interrupt handler is stray until the eisa_slot table is filled in.
 *
 **************************************************************************/
int 
kn121_pic_intr( int slot )
{
   int irq;

   if (slot >= 0 && slot < MAX_EISA_SLOTS*2)
      {
      /* Dispatch interrupt */
      (*(eisa_slot[slot].intr))(eisa_slot[slot].unit);

      /* save irq for eoi processing */
      irq = eisa_slot[slot].irq.intr.intr_num;
      }
   else
      printf("kn121_eisa_intr: stray interrupt, slot = 0x%x\n",slot);


   /*
    * Finish PIC interrupt by doing specific EOI to irq level
    */
   switch( irq)
      {
      case PIC_IRQ15:
      case PIC_IRQ14:
      case PIC_IRQ13:                   /* DMA chaining interrupts */
      case PIC_IRQ12:
      case PIC_IRQ11:
      case PIC_IRQ10:
      case PIC_IRQ9:
        /* eoi to cntrl 2 */
        WRITE_EISA_D8(CTRL2_OCW2,SPEC_EOI | (irq&0x7));
      case PIC_IRQ7:
      case PIC_IRQ6:
      case PIC_IRQ5:
      case PIC_IRQ4:
      case PIC_IRQ3:
      case PIC_IRQ1:
      case PIC_IRQ0:
        /* eoi to cntrl 1 */
        WRITE_EISA_D8(CTRL1_OCW2,SPEC_EOI|
                            ((irq>PIC_IRQ7) ? 2:irq));
        break;
      default:
        printf("eoi: unrecognized IRQ = %d\n", irq );
      }

}

/*
 *  kn121_conf:  Jensen configuration routine. 
 *
 */
kn121_conf()
{
    int i;
    register struct bus *sysbus;       /* local bus (ibus), VTI combo I/O */
    register struct bus *eisa_bus;     /* eisa bus  (eisa), peripheral bus*/
    int regb;
    extern char DEC_2000_300_STRING[];   /* Jensen */

    /*
     * Working on cold restart (for autoconf and kern_cpu)
     */
    cold = 1;

    /*
     * Identify system and console revisions
     */
    master_cpu = 0;
    machine_slot[master_cpu].is_cpu = TRUE;
    machine_slot[master_cpu].cpu_type = CPU_TYPE_ALPHA;
    machine_slot[master_cpu].cpu_subtype = CPU_SUBTYPE_DEC_2000_300;
    machine_slot[master_cpu].running = TRUE;
    machine_slot[master_cpu].clock_freq = hz;

    printf("%s system\n", cpup->system_string);
    printf ("Firmware revision: %d.%d\n", kn121_firmware_major, 
	    kn121_firmware_minor);
    printf ("PALcode: %s version %d.%02d\n", 
	    kn121_palcode_type ==  PALvar_OSF1 ? "OSF":"VMS", 
	    kn121_palcode_version, kn121_palcode_revision);

    /*
     * Enable interrupts so bus code can allow drivers to 
     * take interupts during probe/attach if they desire
     */
    enable_spls();
    (void) spl0();

    /*
     * Configure I/O through virtual ibus.  kn121_configure_io() will
     * get called through the ibusconfl1 routine to configure all the
     * local VTI Combo chip devices and the EISA peripheral bus
     *
     */
    if ((system_bus = sysbus = get_sys_bus("ibus")) == (struct bus *)0 )
	panic("kn121_conf:  No ibus bus in config file");
    (*sysbus->confl1)(-1, 0, sysbus);
    (*sysbus->confl2)(-1, 0, sysbus);

    /* 
     * EISA DMA initialization  (Integrated System Peripheral chip)
     *
     * Initialize ISP DMA engine for rotating priority, DREQ sense
     * active high and DACK sense active low. Also enable both DMA
     * controllers. Finally mask off all channels. They will be
     * unmasked individually as needed. Channel 4 is not masked off
     * because channels 0-3 are cascaded into it. Finally load the
     * DMA interrupt handler into the SCB.
     */
    isp_dma_init();
    /* Load eisa slot table entry with dma engines interrupt info. */
    /* We use slot number MAX_EISA_SLOTS for the DMA.  		   */
    eisa_slot[MAX_EISA_SLOTS].intr = isp_dma_intr;
    eisa_slot[MAX_EISA_SLOTS].intr_param = (caddr_t)0;
    eisa_slot[MAX_EISA_SLOTS].irq.intr.intr_num = 13;
    /* Now load the scb vector. */
    intrsetvec (irq_to_scb_vector(13), kn121_pic_intr, MAX_EISA_SLOTS);
    

#ifdef BEEP_TEST
	/*
	 * Put some beeps in the startup sequence.
	 */
	beep_test();
#endif /* BEEP_TEST */


#ifdef DISK_LED_FLASH
	/*
	 * Kick off a thread to cause the HD LED to toggle once a 
	 * second.  this is a debug aid to help identify system hangs.
	 */
	disk_led_flasher(1);
#endif /* DISK_LED_FLASH */


    cold = 0;
    
    return(0);
}


/***************************************************************************
 *
 *  kn121_configure_io
 *
 *   	Jensen configuration routine called through ibusconfl1() routine.  
 *   	We put local devices on a virtual bus called the ibus for the 
 *	purpose of properly setting up the system bus/controller structures.
 *   
 *
 *	Local device bus configuration and i/o access.
 *
 *		probe, attach,  and interrupt all called with  ctrl struct
 *		that contains the base address of the VTI combo chip as
 *		the ctrl->physaddr.  The kn121_read/write_port routines
 *		check for the encoded vti base address to determine if a
 *		jensen "local" byte access is to be done vs. an EISA bus
 *		access.  This method of local i/o configuration allows
 *		local drivers to be more easily portable and usable with
 *		common ISA and EISA devices.
 *
 *	Note:  The local I/O devices that are *not* on any type
 *	       of a bus we can probe,  so their configuration has to be
 *	       wired down here prior to configuring the eisa bus.  Rather
 *	       than essentailly copy ibus code here,  leverage as much from
 *	       the ibus as possible,  and adjust differences here, like the
 *	       need to configure multiple scb locations for ace and gpc.
 *
 *****************************************************************************/
kn121_configure_io(bus)
	struct bus *bus;	/* ibus bus structure */
{
    struct controller *ctlr, *get_ctlr_num();
    struct bus *eisa_bus;
    struct bus  *savebushd;

    int	ret;
    
    /*
     * ACE serial port - 2 lines COMMA/COMMB  
     *
     * Set-up ctlr struct and call probe and attach through
     * the ibus routine,  then get ctlr structure and fill in
     * the two scb vectors with ctlr information
     *
     */
    if (!(ib_config_cont(VTI_BASE,0,0,"ace",bus, 0))) 
	{
	    printf("local serial ports not configured - ace0\n");
	}
    else {
	if( (ctlr = get_ctlr_num("ace", 0)) == (struct controller *)NULL)
	    {
		printf("ace0 ctlr struct not found\n");
		printf("local serial ports interrupts not configured\n");
	    }
	else  
	    {
		intrsetvec(KN121_SCB_COMMA, *ctlr->intr, 0);
		intrsetvec(KN121_SCB_COMMB, *ctlr->intr, 1);
	    }
    }

    /*
     * Mouse and keyboard
     *
     * Set-up ctlr struct and call probe and attach through
     * the ibus routine,  then get ctlr structure and fill in
     * the two scb vectors with ctlr information
     *
     */
    if (!(ib_config_cont(VTI_BASE,0,0,"gpc",bus,0)))
	{
	    printf("PC mouse and keyboard not configured - gpc0\n");
	}
    else 
	{
	if( (ctlr = get_ctlr_num("gpc", 0)) == (struct controller *)NULL)
	    {
		printf("gpc0 ctlr struct not found\n");
		printf("PC mouse and keyboard not configured\n");
	    }
	else  
	    {
		intrsetvec(KN121_SCB_MOUSE, *ctlr->intr, ctlr->ctlr_num);
		intrsetvec(KN121_SCB_KEYBD, *ctlr->intr, ctlr->ctlr_num);
	    }
	}

    /*
     * Configure EISA peripheral bus 
     *
     * EISA bus is configured as a connected bus to the local
     * virtual "ibus",  which is the system_bus.
     */

    if((eisa_bus = get_bus("eisa", 0, bus->bus_name, 
			   bus->bus_num)) ||
       (eisa_bus = get_bus("eisa", 0, bus->bus_name, -1)) ||
       (eisa_bus = get_bus("eisa", -1, bus->bus_name, 
			   bus->bus_num)) ||
       (eisa_bus = get_bus("eisa", -1, bus->bus_name, -1))) 

       {
       savebushd = eisa_bus->bus_hd;
       eisa_bus->bus_hd = bus;	/* Make this connection before calling 	 */
                                /* eisa's confl1 function so that it can */
				/* get to its parent's bus structure for */
				/* any information it needs.		 */
       bus_info.common_infop = &kn121_eisa_common_info;
       ret = 0;
       if (!(*eisa_bus->confl1) || !(*eisa_bus->confl2))
	   printf("kn121_config_io: No eisa configuration code");
       else
	  ret = (*eisa_bus->confl1)(bus->bus_type, &bus_info,
				    eisa_bus);
       if (ret == 1)
	  ret = (*eisa_bus->confl2)(bus->bus_type, &bus_info,
				    eisa_bus);
       if (ret == 1)
	  conn_bus (bus, eisa_bus);
       else
	  {
	  eisa_bus->bus_hd = savebushd;
	  printf("kn121_config_io:  eisa not configured");
	  }
       }
    else 	
       printf("kn121_config_bus: cannot find eisa bus");

    /*
     * parallel printer port 
     *
     * We do the parallel printer after we do the eisa bus because we have
     * to set up an eisa slot table entry for the parallel printer. Since
     * the eisa bus code initializes the slot table we do not want to fill
     * in parallel printer data until that occurs.
     *
     * Set-up ctlr struct and call probe and attach through
     * the ibus routine,  then get ctlr structure and fill in
     * the scb vector with ctlr information
     *
     * The parrallel printer is a local device whose interrupt
     * is attached to the PIC at IRQ1,  so set-up the interrupt 
     * here.
     * 
     */

    ib_config_cont(VTI_BASE,0,0,"lp",bus, 0);
    
    if( (ctlr = get_ctlr_num("lp", 0)) == (struct controller *)NULL)
	{
	    printf("lp0 ctlr struct not found\n");
	    printf("local parallel port interrupt not configured\n");
	}
    else  
	{
	/* Set scb vector to central handler. */
	intrsetvec(KN121_SCB_LP, kn121_pic_intr, 0);
	/* Load eisa slot table entry. Parallel printer goes in system slot */
	/* (0).								    */
	eisa_slot[0].intr = *ctlr->intr;
	eisa_slot[0].intr_param = (caddr_t)0;
	eisa_slot[0].irq.intr.intr_num = 0;
	/* Set trigger sense and enable the interrupt. */
	pic_set_irq_edge(PIC_IRQ1);
/*	pic_enable_irq(PIC_IRQ1); */
	}
	    

}




/* psgfix: system id method needed - ether rom is optional!!!
 * 
 * 	This returns a unique system id by using the 
 *	last 4 bytes of the ethernet station address 
 */ 
int
kn121_system_id()
{
vm_offset_t station_address;
int i;
u_int temp_int;
u_int 	id=0xFACE;

#ifdef NOT
	station_address = PHYS_TO_KSEG(0x01a0080000);
	station_address += 8;	/* just last 4 bytes of ether addres */
	for (i=2; i>=0; i--)
		{
		temp_int = (*(u_int *)station_address);
		id |= temp_int & 0xff;
		id <<= 8;
		station_address += 4;
        }
	temp_int = (*(u_int *)station_address);
	id |= temp_int & 0xff;
#endif /*NOT*/

	return(id);
}


/******************************************************************* 
 *
 *  kn121_read_hae/write_hae
 *
 *  These read/write routines for talking
 *  to the HAE register. The caller of these
 *  routines must raise and lower ipl around
 *  this register to splbio.
 * 
 *
 *  read routine returns HAE value
 *
 *  write routine writes data to HAE
 *
 *********************************************************************/
int kn121_read_hae()
{
   register unsigned int *portp;
   u_int data;

   portp = (unsigned int *)((u_long)KN121_HAE);
   data = *(unsigned int *)PHYS_TO_KSEG(portp) & 0xff;

   return(data);
}

int kn121_write_hae( int data )
{
   register unsigned int *portp;

   portp = (unsigned int *)((u_long)KN121_HAE);
   *(unsigned int *)PHYS_TO_KSEG(portp) = (data & 0xff);
}


/**********************************************************************
 *
 *	KN121 Error handling
 *
 *	Machine Check logout area defined by PALcode consists of 
 *	three parts and is defined in errlog.h:
 *
 *		- Alpha generic header  (Uses flamingo definition)
 *		- EV4-specific logout   (Again use flamingo definition)
 *		- KN121-specfic logout  (KN121 defined platform area)
 * 
 *	All Hardware Machine Checks are currently fatal.
 * 	psgfix: Need to investigate error-handling further.
 *
 *
 ************************************************************************/


/*************************************************************************
 *
 * kn121_macheck()
 *
 * Machine check handler is called from locore thru the cpu switch in 
 * response to a trap at SCB locations: 0x660 (system machine check) 
 * and 0x670 (CPU machine check).
 *
 * Also, correctable errors (0x620 and 0x630) come through here.
 *
 **************************************************************************/
kn121_machcheck (type, cmcf, framep)
	long type;
	caddr_t cmcf;	/* KSEG address */
	long *framep;	/* saved registers - frame pointer */
{
	register int recover = 0;
	register int retry;
	register int byte_count;
	register int clear_check_ip = 0;
	register struct el_kn15aa_logout_header	*mchk_header;
	register struct el_kn15aa_procdata_mcheck *mchk_procdata;
	register struct el_kn121_sysdata_mcheck	*mchk_sysdata;
	struct el_rec *mchk_elrec;
	struct el_kn121_frame_mcheck mchk_logout;
	int  mchk_type, mchk_cputype;
	int i;

	mchk_header = (struct el_kn15aa_logout_header *) cmcf;
	mchk_procdata = (struct el_kn15aa_procdata_mcheck *)(cmcf + 
	  mchk_header->elfl_procoffset - sizeof(mchk_procdata->elfmc_paltemp));
	mchk_sysdata = (struct el_kn121_sysdata_mcheck *)(cmcf + 
			mchk_header->elfl_sysoffset);


	retry = mchk_header->elfl_retry;
	byte_count = mchk_header->elfl_size;

	/* Build the completed machine check frame */
	bcopy (mchk_header, &mchk_logout.elfmc_hdr, 
	       sizeof(mchk_logout.elfmc_hdr));
	bcopy (mchk_procdata, &mchk_logout.elfmc_procdata, 
	       sizeof(mchk_logout.elfmc_procdata));
	bcopy (mchk_sysdata, &mchk_logout.elfmc_sysdata, 
	       sizeof(mchk_logout.elfmc_sysdata));

	/* Allocate an errlog buffer */
	mchk_elrec = ealloc(sizeof(struct el_kn121_frame_mcheck),EL_PRISEVERE);

	/* If we have an errlog record, copy frame in and make it valid */
	if (mchk_elrec != (struct el_rec *)BINLOG_NOBUFAVAIL) {
		switch (type) {
			case SYS_CORR_ERR: /* System correctable error */
				mchk_type = ELEXTYP_SYS_CORR;
				break;
			case PROC_CORR_ERR: /* Processor correctable error */
				mchk_type = ELEXTYP_PROC_CORR;
				break;
			case SYS_MCHECK: /* System machine check abort */
				mchk_type = ELEXTYP_HARD;
				break;
			case PROC_MCHECK: /* Processor machine check abort */
				mchk_type = ELEXTYP_MCK;
				break;
		}
		mchk_cputype = ELMACH_DEC_2000_300;
		bcopy (&mchk_logout, &mchk_elrec->el_body, 
		       sizeof (mchk_logout));
		LSUBID (mchk_elrec, ELCT_EXPTFLT, mchk_type, 
			mchk_cputype, 0, 0, 0);
		binlog_valid(mchk_elrec);
	}

	printstate |= PANICPRINT;


	/* things look bad for the good guys, we weren't planning on a mcheck*/
	kn121_consprint (type, &mchk_logout);

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
	    kn121_correrr_ip++;
	    /*
	     * For the case of multiple entry into this routine for
	     * corrected errors simply return because it is 
	     * possible that something this routine is doing is
	     * causing another correctable error.   Since all this
	     * routine does is log these anyways just ignore the
	     * recursive case.
	     */
	    if (kn121_correrr_ip > 1) {	
		kn121_correrr_ip--;
		return(0);
	    }
	    if (retry == 0) {
		printf("Retry bit not set on corrected err!\n");
	    }
	    recover = 1;
	    break;
	  case SYS_MCHECK: 	/* System machine check abort */
	  case PROC_MCHECK: 	/* Processor machine check abort */
	    kn121_mchk_ip++;
	    /*
	     * If a second machine check is detected while a 
	     * machine check is in progress, a Double Error abort
	     * is generated and the processor enters the restart
	     * sequence.  For this reason the following test should
	     * never be true.  
	     */
	    if (kn121_mchk_ip > 1) {
		panic("Double Error machine check abort.\n");
	    }
	    clear_check_ip = 1;
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
	    kn121_mchk_ip--;
	}	

	/*
	 *  psgfix: Can we give better info than this?
	 *
	 *  for the moment, always panic if an Mcheck happens...
	 *
	 */
	
	panic ("Machine check - Hardware error");
}


/*****************************************************************************
 *
 * kn121_consprint
 *
 * Print error packet to the console.
 * This is only done when we are about to panic on the error.
 *
 * The KN121 machine check logout frame is defined in errlog.h
 *
 ****************************************************************************/

kn121_consprint(type, cmcf)
	int type;			     /* machine check type */
	struct el_kn121_frame_mcheck *cmcf;  /* Pointer to the logout area */
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
	printf("\tEV4 Processor registers\n");
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
	printf("\tfill_syndrome\t= %l016x\n",
	       cmcf->elfmc_procdata.elfmc_fill_syndrome);
	printf("\tfill_addr\t= %l016x\n",cmcf->elfmc_procdata.elfmc_fill_addr);
	printf("\tva\t\t= %l016x\n",cmcf->elfmc_procdata.elfmc_va);
	printf("\tbc_tag\t\t= %l016x\n",cmcf->elfmc_procdata.elfmc_bc_tag);
	/*
	 *  KN121 (Jensen) Platform Specific Fields
	 */
	printf("\tKN121 platform specific registers\n");
	printf("\tident\t= 0x%x\n",cmcf->elfmc_sysdata.elfmc_ident);
        switch (cmcf->elfmc_sysdata.elfmc_ident) {
            case PARITY_ERR:   printf("Parity Error\n"); break;
            case IOCHECK_ERR:  printf("IOcheck Error\n"); break;
            case TIMEOUT_ERR:  printf("Bus Master timeout\n"); break;
            case SLAVE_ERR:    printf("Slave disconnect timeout\n"); break;
            case FAILSAFE_ERR: printf("Failsafe timer timeout\n"); break;
            case SOFT_NMI_ERR: printf("Software NMI\n"); break;
            default: printf("Unknown type\n"); break;
        }
	printf("\tbus master reg\t= 0x%x\n",cmcf->elfmc_sysdata.elfmc_bus_mas);
	printf("\tfw rev\t= 0x%x\n",cmcf->elfmc_sysdata.elfmc_fw_rev);
	printf("\tpic<7:0>msk\t= 0x%x\n",cmcf->elfmc_sysdata.elfmc_pic1_mask);
	printf("\tpic<7:0>irr\t= 0x%x\n",cmcf->elfmc_sysdata.elfmc_pic1_irr);
	printf("\tpic<7:0>isr\t= 0x%x\n",cmcf->elfmc_sysdata.elfmc_pic1_isr);
	printf("\tpic<7:0>elr\t= 0x%x\n",cmcf->elfmc_sysdata.elfmc_pic1_elr);
	printf("\tpic<15:8>msk\t= 0x%x\n",cmcf->elfmc_sysdata.elfmc_pic2_mask);
	printf("\tpic<15:8>irr\t= 0x%x\n",cmcf->elfmc_sysdata.elfmc_pic2_irr);
	printf("\tpic<15:8>isr\t= 0x%x\n",cmcf->elfmc_sysdata.elfmc_pic2_isr);
	printf("\tpic<15:8>elr\t= 0x%x\n",cmcf->elfmc_sysdata.elfmc_pic2_elr);
	printf("\tdma<3:0>msk\t= 0x%x\n",cmcf->elfmc_sysdata.elfmc_dma1_mask);
	printf("\tdma<3:0>stat\t= 0x%x\n",cmcf->elfmc_sysdata.elfmc_dma1_stat);
	printf("\tdma<7:4>msk\t= 0x%x\n",cmcf->elfmc_sysdata.elfmc_dma2_mask);
	printf("\tdma<7:4>stat\t= 0x%x\n",cmcf->elfmc_sysdata.elfmc_dma2_stat);
	printf("\tdma<7:0>chain stat\t= 0x%x\n",cmcf->elfmc_sysdata.elfmc_dma_cstat);
	printf("\tdma<7:0>intr\t= 0x%x\n",cmcf->elfmc_sysdata.elfmc_dma_intr);
	printf("\tdma<7:0>chain mode\t= 0x%x\n",cmcf->elfmc_sysdata.elfmc_dma_cmode);
	printf("\tcomma ie\t= 0x%x\n",cmcf->elfmc_sysdata.elfmc_coma_ie);
	printf("\tcomma ir\t= 0x%x\n",cmcf->elfmc_sysdata.elfmc_coma_ir);
	printf("\tcommb ie\t= 0x%x\n",cmcf->elfmc_sysdata.elfmc_comb_ie);
	printf("\tcommb ir\t= 0x%x\n",cmcf->elfmc_sysdata.elfmc_comb_ir);
	printf("\thae\t= 0x%x\n",cmcf->elfmc_sysdata.elfmc_hae);
	printf("\tsys ctlr ir\t= 0x%x\n",cmcf->elfmc_sysdata.elfmc_sys);
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

/***************************************************************************/
/*                                                                         */
/* kn121_dump_dev  -  Translate dumpdev into a string the console can	   */
/*		      understand.	   				   */
/*                                                                         */
/* SYNOPSIS                                                                */
/*	int  kn121_dump_dev (dump_req)	                 		   */
/*                                                                         */
/*	struct dump_request	*dump_req;                                 */
/*                                                                         */
/*                                                                         */
/* PARAMETERS								   */
/*	dump_req	Pointer to dump information structure.		   */
/*                                                                         */
/*                                                                         */
/* DESCRIPTION								   */
/*	Translates the SCSI device and target data into a device string    */
/*	which can be passed to prom_open, which is a generic IO console    */
/*	callback. See the SRM for more details.	For Jensen the BOOTED_DEV  */
/*	string for a SCSI disk boot is (8 fields):			   */
/*                                                                         */
/*	    SCSI 1 <eisa_slot> <channel> 0 <target*100> 0 JENS-IO 	   */
/*                                                                         */
/*	target*100 eliminates LUN, is that ok?				   */
/*									   */
/*                                                                         */
/***************************************************************************/

kn121_dump_dev(dump_req)
	struct dump_request *dump_req;
{
	char *device_string;
	char temp_str[8];
	u_int scsi_slot;
	int scsi_cntlr;

	device_string = dump_req->device_name;

        /* 
	 * Field 1:  Protocal = SCSI 
	 */
	strcpy(device_string,"SCSI ");

	/* 
	 * Field 2:  eisa_number = 1 
	 */
	strcpy(&device_string[strlen(device_string)],"1 ");

	/* 
	 * Field 3: eisa slot number 
	 */
	scsi_slot = dump_req->device->ctlr_hd->slot;
	itoa(scsi_slot,temp_str);
	strcpy(&device_string[strlen(device_string)],temp_str);

	/* 
	 * space after eisa slot number 
	 */
	strcpy(&device_string[strlen(device_string)]," ");  

	/* 
	 * Field 4: Channel, SCSIA, SCSIB, etc.. 
	 *	    
	 * Jensen console uses A, B, etc.. to logically
	 * number scsi controllers, so A is first, B is
	 * second, and so on.  The letters map to 
	 * their cooresponding number, A = 0, B = 1, etc.
	 * So we can get this equivilent number from
	 * the ctrl->num field,  where we logically assign
	 * numbers starting at 0,  with the first bus we
	 * find.
	 *
	 * There must be a better way than BOOTED_DEV.
	 *
	 */
	scsi_cntlr = dump_req->device->ctlr_hd->ctlr_num;
	itoa(scsi_cntlr,temp_str);
	strcpy(&device_string[strlen(device_string)],temp_str);

	/* 
	 * Field 5: remote address = 0 
	 */
	strcpy(&device_string[strlen(device_string)]," 0 ");

	/* 
	 * Field 6: id and lun(lun always zero) 
	 */
	itoa(dump_req->unit*100,temp_str);
	strcpy(&device_string[strlen(device_string)],temp_str);

	/* 
	 * Field 7: device type: 0=disk; 1=tape 
	 */
	strcpy(&device_string[strlen(device_string)]," 0 ");

	/* 
	 * Field 8: system type = JENS-IO 
	 */
	strcpy(&device_string[strlen(device_string)],"JENS-IO");
}




/***************************************************************************/
/*									   */
/*	Bell-related Routines						   */
/*									   */
/***************************************************************************/

#define MAX_BELL_QUEUE 16
int bell_users = 0;
int bell_next = 0;
int bell_cur = 0;
int bell_pitch[MAX_BELL_QUEUE];
int bell_duration[MAX_BELL_QUEUE];

/*
 * Turn on the bell, given a pitch
 *
 * Input:
 *		pitch		frequency in beats per second
 */
int
kn121_bell_on(pitch)
	int pitch;
{
	long data;

	/* !!! MAGIC !!! */
	/*
	 * oscillator freq is 1.193 MHz, so dividing that by the desired
	 * frequency (cycles/sec) gives the counter value to load as the 
	 * square wave period:
	 *
	 *	(CLKS/sec) / (cycles/sec) -> CLKS/cycle
	 */
	pitch = 1193180/pitch;

	/*
	 * Turn on gate and speaker data for counter 2.
	 *   NMI Status Register:
	 *	GATE set (enable Ctr 2) - bit 0
	 *	SPKR set (AND w/ Ctr 2) - bit 1
	 */
	data = READ_EISA_D8(0x61);
	WRITE_EISA_D8(0x61,data | 0x03);
	mb();
	
	/*
	 * Set Counter 2 to "pitch".
	 *   Timer 1 Control reg (@0x43):
	 *	OUT pin is 1                -  bit 7
	 *	Counter Latch LSB then MSB  -  bits 4 & 5
	 *	Mode 3 (Square Wave Mode)   -  bits 1 & 2
	 *   Timer 1 Counter 2 (@0x42): LSB
	 *   Timer 1 Counter 2 (@0x42): MSB
	 */
	WRITE_EISA_D8(0x43,0xb6L);
	mb();
	WRITE_EISA_D8(0x42,(long)(pitch&0xff));
	mb();
	WRITE_EISA_D8(0x42,(long)((pitch>>8)&0xff));
	mb();

	/*
	 * Mode 3 loops indefinitely, so tone goes on until "bell_off"
	 *  is called...
	 */
	return(0);
}

/*
 * routine to turn off the bell
 */
int
kn121_bell_off()
{	
	long data;
	int s;

	/*
	 * Turn off gate and speaker data for counter 2.
	 *   NMI Status Register (@0x61):
	 *	GATE clr (disable Ctr 2) - bit 0
	 *	SPKR clr (AND w/ Ctr 2)  - bit 1
	 */
	data = READ_EISA_D8(0x61);
	WRITE_EISA_D8(0x61,data & ~0x03);
	mb();

	/*
	 * Reset Counter 2 to zeros.
	 *   Timer 1 Control reg (@0x43):
	 *	OUT pin is 1                -  bit 7
	 *	Counter Latch LSB then MSB  -  bits 4 & 5
	 *	Mode 1 (One-Shot)           -  bit 1
	 *   Timer 1 Counter 2 (@0x42): LSB
	 *   Timer 1 Counter 2 (@0x42): MSB
	 */
	WRITE_EISA_D8(0x43,0xb2L);
	mb();
        WRITE_EISA_D8(0x42,0L);
	mb();
	WRITE_EISA_D8(0x42,0L);
	mb();

	/*
	 * see if any bell requests are queued up...
	 */
	s = spltty();
	if (--bell_users > 0) {
		kn121_bell_on(bell_pitch[bell_cur]);
		timeout(kn121_bell_off, 0,
			bell_duration[bell_cur] * hz / 1000);
		bell_cur = (bell_cur + 1) % MAX_BELL_QUEUE;
	}
	splx(s);
}

/*
 * kn121 CPU (Jensen) routine to ring the bell.
z *
 * This routine is called via the arch/alpha/hal/cpusw.c routine "ring_bell"
 *  entry point, using the function pointer from arch/alpha/hal/cpuconf.c.
 *
 * Input:
 *	pitch		cycles per second (beat frequency)
 *	duration	milliseconds
 */
int
kn121_ring_bell(pitch, duration)
	int pitch, duration;
{
	int s;

	/*
	 * check pitch for too low or too high
	 */
	if (pitch < 220 || pitch > 2000) /* FIXME - is this reasonable? */
		pitch = 440;

	/*
	 * check duration:
	 *	duration less than 20msecs gets changed to 20 msecs
	 *	larger than 1000 msecs gets changed to 1000 msecs
	 */
	if (duration < 20)
		duration = 20;
	else if (duration > 1000)
		duration = 1000;

	/*
	 * check for bell already being rung...
	 */
	s = spltty();
	if (++bell_users > 1) {
		bell_pitch[bell_next] = pitch;
		bell_duration[bell_next] = duration;
		bell_next = (bell_next + 1) % MAX_BELL_QUEUE;
		splx(s);
		return(0);
	}
	splx(s);


	/*
	 * start the bell up with pitch, and then
	 *  convert duration to clock ticks, and cause
	 *  bell turn-off at that time.
	 */
	kn121_bell_on(pitch);
	timeout(kn121_bell_off, 0, duration * hz / 1000);

	return(0);
}


/*
 * dma_map_* routines for kn121 platform 
 */
/************************************************************************/
/*                                                                      */
/* kn121_dma_map_alloc  -  kn121 platform specific dma map alloc. fcn.	*/
/*                                                                      */
/* SYNOPSIS								*/
/*    u_long kn121_dma_map_alloc(u_long bc, struct controller *cntrlrp, */
/*				 sglist_t *sglistp, int flags)		*/
/*                                                                      */
/* PARAMETERS								*/
/*	bc	no. of bytes that system resources are being req. for	*/
/*	cntrlrp	ptr to controller structure for the driver that is	*/
/*		requesting resources.					*/
/*	sglistp Pointer to a ptr to data structure (that will be 	*/
/*		alloc'd) that will contain info on scatter-gather list 	*/
/*		for "bc-sized" DMA xfer.				*/
/*	flags	flag word indicating caller's desire to sleep if 	*/
/*		resources are busy; return w/failure if unable to 	*/
/*		satisfy entire "bc"-sized request			*/
/*                                                                      */
/* DESCRIPTION								*/
/*	This function will call down to the direct-map alloc function 	*/
/*	to allocate kernel data structures, as well as check for 	*/
/*	necessary EISA resources: allocate a controller-channel if EISA */
/*	card is a DMA slave; if EISA bus master device, will do nothing */
/*	that is EISA-specific.						*/
/*                                                                      */
/* RETURN								*/
/*	Returns "rtn_bc", the number of bytes the allocat(able) system	*/
/*	resources are able to support wrt request byte count (bc).	*/
/*	Never exceeds "bc".  Returns 0 on failure/no-resources.		*/
/*	If rtn_bc != 0, then sglistp updated to point to allocated 	*/
/*	sglist structure.						*/
/*                                                                      */
/************************************************************************/
u_long
kn121_dma_map_alloc(bc, cntrlrp, sglistp, flags)
	unsigned long		bc;
	struct	controller 	*cntrlrp;
	sglist_t		*sglistp;
	int 			flags;
{
	int		isp_status;
	unsigned long	rtn_bc;

#ifdef DMA_MAP_DEBUG
	printf("\n ENTERED kn121_dma_map_alloc \n");
	printf("Calling args are: \n");
	printf("\t bc      = 0x%lx \n", bc);
	printf("\t cntrlrp = 0x%lx \n", cntrlrp);
	printf("\t &sglistp = 0x%lx \n", sglistp);
	printf("\t sglistp = 0x%lx \n", *sglistp);
	printf("\t flags   = 0x%x \n", flags);
#endif	/* DMA_MAP_DEBUG */

	/*
 	 * Check for EISA channel availability if DMA slave device;
 	 * otherwise, succeed;
	 *
	 * NOTE: Multiple alloc's to the same controller (channel) 
	 *	 is ok --> driver just wants more mapping resources;
	 *	 let lower-level, controller-specific layer keep track
	 *	 of this info.
 	 * cntrlrp -- needed for DMA channel-based devices
 	 * flags   -- needed to indicate RD/WR direction;
	 *	     if SLEEP set, wait on channel if shared & busy
	 */

	/* The generic calling i/f would look something like:
	 *	(*cntrlrp->bus_hd->dma_framework->dma_alloc)()
	 * but we know the only bus support on JENSEN is EISA-isp,
	 * so lets go right to it!
	 */
#ifdef DMA_MAP_DEBUG
	 printf("Calling isp_dma_alloc \n");
#endif	/* DMA_MAP_DEBUG */

	 isp_status = isp_dma_alloc(cntrlrp, flags);

#ifdef DMA_MAP_DEBUG
	 /* printf("Return value of isp_dma_alloc is 0x%x \n", isp_status); */
#endif	/* DMA_MAP_DEBUG */

	 /*
	  * Now, do (s-g) mapping; if DMA slave device, hand the
	  * list to the channel intr. routine on dma_map_load(); if 
	  * bus dma device, it will get the list via the driver.
	  */
	 if (isp_status == 0) {  /* success */
		/* call direct_map since no s-g support on JENSEN;
		 * returned list of bus_address will = system mem. address;
		 */
#ifdef DMA_MAP_DEBUG
		printf("Calling direct_map_alloc w/following args: \n");
		printf("\t bc       = 0x%lx \n", bc);
		printf("\t cntrlrp  = 0x%lx \n", cntrlrp);
		printf("\t sglistp = 0x%lx \n", *sglistp);
		printf("\t flags   = 0x%x \n", flags);
#endif	/* DMA_MAP_DEBUG */

		rtn_bc = direct_map_alloc(bc,cntrlrp,sglistp,flags);

#ifdef DMA_MAP_DEBUG
		printf("\t Return values of direct_map_alloc: \n");
		printf("\t\t sglistp = 0x%lx \n", *sglistp);
		if (rtn_bc)
			printf("\t\t rtn_bc  = 0x%lx \n", rtn_bc);
		else
			printf("\t\t rtn_bc is zero \n");
#endif	/* DMA_MAP_DEBUG */

		return(rtn_bc);
	} else	{ /* channel is shared & busy */
		*sglistp = (sglist_t)0;
		return(0L);
	}
}

/************************************************************************/
/*                                                                      */
/* kn121_dma_map_load  -  kn121 platform specific dma map load fcn.	*/
/*                                                                      */
/* SYNOPSIS								*/
/*    u_long kn121_dma_map_load(u_long bc, vm_offset_t va, struct proc  */
/*				*procp,	struct controller *cntrlrp, 	*/
/*				sglist_t *sglistp, u_long max_bc, 	*/
/*				int flags)				*/
/*                                                                      */
/* PARAMETERS								*/
/*	bc	no. of bytes that system resources are being req. for	*/
/*	va	starting virtual address of DMA buffer to map.		*/
/*	procp	process that va is valid in; procp = 0 for kernel pmap. */
/*	cntrlrp	ptr to controller structure for the driver that is	*/
/*		requesting resources.					*/
/*	sglistpp Pointer to a ptr to data structure that will was 	*/
/*		previously alloc'd in a dma_map_alloc() call; if 	*/
/*		*sglistpp = 0, then an alloc needs to be done (for the  */
/*		lazy at-heart).						*/
/*	max_bc	maximum size contiguous buffer that a ba-bc (sg_entry)  */
/*		pair should be. ba-bc list generator tries to min. no. 	*/
/*		of ba-bc pairs by increasing "bc" when ba's are 	*/
/*		contiguous.						*/
/*	flags	flag word indicating caller's desire to sleep if 	*/
/*		resources are busy; return failure if unable to satisfy */
/*		entire "bc"-sized request; direction of DMA xfer 	*/
/*		(for EISA slave	DMA devices); 				*/
/*                                                                      */
/* DESCRIPTION								*/
/*	This function will call down to the direct-map alloc function 	*/
/*	if a previous dma_map_alloc() was not done, i.e., *sglistpp = 0;*/
/*	otherwise, it calls the direct-map function to map va's to bus	*/
/*	phys. addr's per JENSEN one-to-one mapping. For an EISA slave	*/
/*	DMA interface, the s-g list generated is handed to the isp code */
/*	to perform chained DMA (if necessary).				*/
/*                                                                      */
/* RETURN								*/
/*	Returns "rtn_bc", the number of bytes that were mapped		*/
/*	wrt request byte count (bc). Never exceeds "bc".  Returns 0 on  */
/*	failure/no-available-resources.  If rtn_bc != 0, and 		*/
/*	*sglistpp = 0 on call-entry, then *sglistp updated to point to 	*/
/*	allocated sglist structure.					*/
/*                                                                      */
/************************************************************************/
u_long
kn121_dma_map_load(bc, va, procp, cntrlrp, sglistpp, max_bc, flags)
	unsigned long		bc;
	vm_offset_t		va;
	struct	proc		*procp;
	struct	controller	*cntrlrp;
	sglist_t		*sglistpp;
	unsigned long		max_bc;
	int			flags;
{
	u_long	rtn_bc = bc;
	struct	sglist	*sglistp = *sglistpp;

#ifdef DMA_MAP_DEBUG
	printf("\n ENTERED kn121_dma_map_load \n");
	printf("Calling args are: \n");
	printf("\t bc      = 0x%lx \n", bc);
	printf("\t va      = 0x%lx \n", va);
	printf("\t procp   = 0x%lx \n", procp);
	/* printf("\t cntrlrp = 0x%lx \n", cntrlrp); */
	printf("\t sglistp = 0x%lx \n", sglistp);
	printf("\t flags   = 0x%x \n", flags);
#endif	/* DMA_MAP_DEBUG */

	if (sglistp == (sglist_t)0) {	/* no sglist */
		rtn_bc = kn121_dma_map_alloc(bc,cntrlrp,sglistpp,flags);
		if (rtn_bc == 0L)	{  /* drop-out on failure */
			*sglistpp = (sglist_t)0;
			return(0L);
		}
	}
	sglistp = *sglistpp;

	/* now, load the allocated map(s); failure = 0 */
	if(direct_map_load(rtn_bc,va,procp,sglistp,max_bc)) {
		/* give DMA channel hw s-g map, if DMA slave device */
		isp_dma_load(cntrlrp,sglistp,flags);
	}
	else {
		return(0L);
	}

	return(rtn_bc);
}

/************************************************************************/
/*                                                                      */
/* kn121_dma_map_unload  - invalidate address mapping			*/
/*                                                                      */
/* SYNOPSIS								*/
/*	int kn121_dma_map_unload(int flags, struct sglist *sglistp)	*/
/*                                                                      */
/* PARAMETERS								*/
/*	sglistp Pointer to data structure that was previously alloc'd 	*/
/*		in a dma_map_alloc() call; 				*/
/*	flags	flag word indicating caller's desire to have system	*/
/*		resources deallocated, as well as mapping invalidated.	*/
/*                                                                      */
/* DESCRIPTION								*/
/*	This function simply sets the sg_entry index to 0, since a	*/
/*	one-to-one map is always valid on a system's io bus.		*/
/*	If flag is set to DMA_DEALLOC, the system resources associated	*/
/*	with the DMA mapping are deallocated/free'd.			*/
/*                                                                      */
/* RETURN								*/
/*      A successful return is one (1). A failure a zero (0).  		*/
/*                                                                      */
/************************************************************************/
int
kn121_dma_map_unload(flags, sglistp)
	int flags;
	struct	sglist *sglistp;
{
	struct	ovrhd	*sgl_ovrhdp;
	
	if (sglistp == (sglist_t)NULL)
		return(0);

	sgl_ovrhdp = (struct ovrhd *)(&sglistp[1]);

#ifdef DMA_MAP_DEBUG
	printf("\n ENTERED kn121_dma_map_unload \n");
	printf("Calling args are: \n");
	printf("\t flag    = 0x%lx \n", flags);
	printf("\t sglistp = 0x%lx \n", sglistp);
#endif	/* DMA_MAP_DEBUG */

	/*
	 * for DMA slave devices, dealloc any internal version
	 * of s-g maps made by EISA isp chaining intr engine; if none
	 * to dealloc, then it'll be back... real soon.
	 */
#ifdef DMA_MAP_DEBUG
	printf("\n CALLING isp_dma_unload; calling args: \n");
	printf("\t sglistp = 0x%lx \n", sglistp);
#endif	/* DMA_MAP_DEBUG */

	isp_dma_unload(sgl_ovrhdp->cntrlrp, sglistp);

#ifdef DMA_MAP_DEBUG
	printf("\t RETURNED from isp_dma_unload \n");
#endif	/* DMA_MAP_DEBUG */

	/*
	 * if flags has DMA_DEALLOC set, dealloc resources;
	 * mirror of dma_map_load where sglistp == 0 means do an alloc
	 */
 	if (flags & DMA_DEALLOC)	{ /* dealloc as well*/
		kn121_dma_map_dealloc(sglistp);
	}

	return(1);	/* always succeed on kn121 */
}

/************************************************************************/
/*                                                                      */
/* kn121_dma_map_dealloc  -  deallocates system DMA mapping resources	*/
/*                                                                      */
/* SYNOPSIS								*/
/*	int kn121_dma_map_dealloc(struct sglist *sglistp)		*/
/*                                                                      */
/* PARAMETERS								*/
/*	sglistp ptr to data structure that was previously alloc'd 	*/
/*		in a dma_map_alloc() call; 				*/
/*                                                                      */
/* DESCRIPTION								*/
/*	This function simply deallocates the memory resources used	*/
/*	to map the DMA buffer space to the io bus address space(s).	*/
/*                                                                      */
/* RETURN								*/
/*      A successful return is one (1). A failure a zero (0).  		*/
/*                                                                      */
/************************************************************************/
int
kn121_dma_map_dealloc(sglistp)
	struct  sglist *sglistp;
{
	struct	ovrhd	*sgl_ovrhdp;

	if (sglistp == (sglist_t)NULL)
		return(0);

	sgl_ovrhdp = (struct ovrhd *)(&sglistp[1]);

#ifdef DMA_MAP_DEBUG
	printf("\n ENTERED kn121_dma_map_dealloc \n");
#endif
	/* free DMA channel if DMA slave device */
	isp_dma_dealloc(sgl_ovrhdp->cntrlrp);

	/* it's been nice knowing you */
	direct_map_dealloc(sglistp);

	return(1);	/* always succeed on kn121 */
}

/************************************************************************/
/*                                                                      */
/* kn121_dma_min_boundary  -  system DMA atomicity transfer size.	*/
/*                                                                      */
/* SYNOPSIS								*/
/*	int kn121_dma_min_boundary(struct controller *cntrlrp)		*/
/*                                                                      */
/* PARAMETERS								*/
/*	cntrlrp	ptr to controller structure for the driver that is	*/
/*		requesting resources.					*/
/*                                                                      */
/* DESCRIPTION								*/
/*	This function returns the number of bytes a DMA transfer	*/
/*	is guaranteed to be atomic on a particular system &/or io bus	*/
/*	attached to a system.  For JENSEN, this value is 1, for 	*/
/*	byte-level atomicity.						*/
/*                                                                      */
/* RETURN								*/
/*	Always returns a "1" on JENSEN, for byte-level DMA atomicity.	*/
/*                                                                      */
/************************************************************************/
int
kn121_dma_min_boundary(cntrlrp)
	struct controller *cntrlrp;
{
/* JENSEN only supports one I/O bus (EISA) thus
 * no-need to call the (EISA) bus-specific routine to check its limits.
 */
#ifdef DMA_MAP_DEBUG
	printf("\n ENTERED kn121_dma_min_boundary \n");
#endif
	return((int)(1));
}

#ifdef BEEP_TEST

/*
 * even entries are frequencies, odd entries are durations
 * can have no sound period by specifying 0 frequency and positive duration
 * MUST have -1 entries to end the table
 */
static int beep_index = 0;

/* NOTE: following defines should be mutex */
#define KODALY
/* #define BEETHOVEN */

static int beep_data[] =
{
#ifdef KODALY 
/* 5 note sequence from "Close Encounters" (orig. GAFFC) */
	 994, 250,	/* B2 */
	1117, 250,	/* C#3 */
	 885, 250,	/* A2 */
	 440, 250,	/* A1 */
	 660, 500,	/* E2 */
#endif /* KODALY */

#ifdef BEETHOVEN
/* first 4 notes of Beethoven's Fifth Symphony */
	738, 100,
	000, 050,
	738, 100,
	000, 050,
	738, 100,
	000, 050,
	586, 300,
	000, 050,
#ifdef NOTNOW
/* next 4 notes of Beethoven's Fifth Symphony */
	658, 100,
	000, 050,
	658, 100,
	000, 050,
	658, 100,
	000, 050,
	553, 300,
#endif /* NOTNOW */
#endif /* BEETHOVEN */

	-1, -1		/* must be on the end */
};

beep_test()
{
	beep_index = 0;
	start_next_beep(0);
}

end_last_beep()
{
	kn121_bell_off();

	beep_index += 2;
	if (beep_data[beep_index] < 0)
		beep_index = 0;
	else
		start_next_beep(beep_index);
	
}

start_next_beep(index)
	int index;
{
	if (beep_data[index] > 0)
		kn121_bell_on(beep_data[index]);

	if (beep_data[index + 1] > 0)
		timeout(end_last_beep, 0, beep_data[index + 1] * hz / 1000);
}

/*
 * This routine can be called externally to cause a beep.  
 * (wicked awesome!)
 */
Beep(freq, msecs)
int freq, msecs;
{
	kn121_bell_on(freq);
	timeout(kn121_bell_off, 0, msecs * hz / 1000);
}

#endif /* BEEP_TEST */

#ifdef DISK_LED_FLASH
/* FIXME FIXME - this is not currently working, so do not activate it!!! */
/***************************************************************************/
/*									   */
/*	Disk LED Control Routines					   */
/*									   */
/***************************************************************************/

/*
 * This routine is called to either turn on or turn off the disk LED on the
 * front pannel of the Jensen.  The action parameter controls this operation
 * as follows:
 *              action = 0      Turns off the LED
 *              action = 1      Turns on the LED
 *              action = 2      Toggles the LED
 * Returns: 0 = success, -1 = failure.
 */

#define HDLED 0x0002

void gpc_ctl_cmd();
void gpc_ctl_output();

#include <sys/workstation.h>
#include <sys/inputdevice.h>
#include <sys/wsdevice.h>
#include <io/dec/ws/pcxal.h>

int
disk_led(action)
        int action;
{
        int d;
	int s;

        /*
         * This is driven from an unused port on the 82C106.
         *
         * First a command is sent which causes the P2 output port contents
         * to be placed in the data buffer.  The data buffer is read into
         * the variable "d" to obtain its current setting.  Next a command is
         * written to specify that the next byte of data to be written will
         * be written to the controller's output port.  
         */
	s = spltty();
        gpc_ctl_cmd(PK_CTL_RDOUT);
        d = gpc_ctl_input();
        switch (action) {
                case 0: d &= (~HDLED); break;
                case 1: d |= HDLED; break;
                case 2: if (d & HDLED)
                                d &= (~HDLED);
                        else
                                d |= HDLED;
                        break;
                default: return(-1);
        }
        gpc_ctl_cmd(PK_CTL_WROUT);
        gpc_ctl_output(d);
	splx(s);
        return(0);
}

/*
 * This is a form of debug routine.  The intent is that it get called
 * periodically (say, once per second) to toggle the disk led on the
 * front pannel.  This may give an indication if the system is hung.
 *
 * The parameter passed is the fraction of a second to sleep between
 * flashes.  For example a value of 1 is 1/1 or 1 second.  A value of
 * 2 is 1/2 second, etc.
 */
int
disk_led_flasher(seconds_divisor)
        int seconds_divisor; 
{
        /* Only continue this process if the led toggle succeeds.  No
         * point in keeping around a no-op process.
         *
         * Although it shouldn't be necessary, out of paranoia perform
         * an untimeout just to insure that the callout queue doesn't 
         * accidentally get overrun with these threads.
         */
        /* tim - what if the previous call had a different parameter;
         * would it still untimeout?
	 */
        untimeout(disk_led_flasher, seconds_divisor);
        if (disk_led(2) == 0) {
                timeout(disk_led_flasher, 1, hz/seconds_divisor);
        }
        else {
                printf("disk_led_flasher: terminating on error.\n");
        }
}
#endif /* DISK_LED_FLASH */


/*
 * Old HISTORY from io_access.c
 * Revision 1.1.4.3  1993/07/30  18:25:29  Randall_Brown
 * 	Revision 1.1.2.8  1993/07/12  00:24:28  Paul_Grist
 * 	Add embedded support for Jensen local VTI I/O in
 * 	kn121_read/write routines. Add HAE support to read/write
 * 	port routines.
 * 	[1993/07/08  08:03:59  Paul_Grist]
 *
 * Revision 1.1.4.2  1993/06/24  22:28:49  Randall_Brown
 * 	Submit of AGOSHW pool into Sterling
 * 	[1993/06/24  21:08:07  Randall_Brown]
 * 
 * Revision 1.1.2.7  1993/05/24  20:13:32  Gary_Dupuis
 * 	Added some documentation.
 * 	[1993/05/24  19:37:48  Gary_Dupuis]
 * 
 * Revision 1.1.2.6  1993/05/04  21:33:14  Gary_Dupuis
 * 	Rewrote kn121_get_io_handle, kn121_read_io_port and
 * 	kn121_write_io_port.
 * 	[1993/05/04  21:28:33  Gary_Dupuis]
 * 
 * Revision 1.1.2.5  1993/04/12  20:05:54  Gary_Dupuis
 * 	Added code in kn121_io_bcopy to do option to option copy.
 * 	[1993/04/12  20:05:21  Gary_Dupuis]
 * 
 * Revision 1.1.2.4  1993/04/06  15:56:06  Gary_Dupuis
 * 	Merge prior to submit.
 * 	[1993/04/06  15:54:09  Gary_Dupuis]
 * 
 * 	Add kn121_read_io_port, kn121_write_io_port and kn121_io_bcopy.
 * 	[1993/04/06  15:18:08  Gary_Dupuis]
 * 
 * Revision 1.1.2.3  1993/03/17  13:56:43  Paul_Grist
 * 	Jensen header file name changes from ka_jensen.h to kn121.h
 * 	[1993/03/17  13:56:25  Paul_Grist]
 * 
 * Revision 1.1.2.2  1993/03/08  21:13:32  Gary_Dupuis
 * 	Initial coding.
 * 	[1993/03/08  20:23:38  Gary_Dupuis]
 * 
 * $EndLog$
 * $EndLog$
 */

int	errno;

#define IOA_OKAY	0
#define	IOA_ERROR	-1

#define HAESET() if (hae) { ipl=splbio(); kn121_write_hae(hae);mb();}
#define HAECLR() if (hae) {kn121_write_hae(0x0);mb(); splx(ipl);} 

/*------------------------------------------------------------------------*/
/* For performance reasons define this as a MACRO rather than a function. */
/* Arguments are bus address, access width, access type.		  */
/*------------------------------------------------------------------------*/
#define KN121_ADDR_TRANSLATE(x,y,z)	\
                                        \
           ((((vm_offset_t)(x) & KN121_LOW_ADDR_MASK) <<  \
	    KN121_EISA_ADDR_SHIFT) | KN121_EISA_BIT |     \
            ((vm_offset_t)z << KN121_EISA_IO_SHIFT) | 		  \
	    ((y - 1) << KN121_WIDTH_SHIFT))

/******************************************
*
*	read_io_port  Functions
*
*******************************************/

/***************************************************************************/
/*                                                                         */
/* kn121_read_io_port	-  Reads data from device register located in EISA */
/* 			   address space.				   */
/*                                                                         */
/* SYNOPSIS                                                                */
/*	long  kn121_read_io_port (dev_addr, width, type)                   */
/*                                                                         */
/*	vm_offset_t	devaddr;	                                   */
/*	int		width;						   */
/*	int		type;						   */
/*                                                                         */
/*                                                                         */
/* PARAMETERS								   */
/*	devaddr		Device physical address to read from. This address */
/*			is a raw device address, e.g. to read the id of an */
/*			option in slot 3 this address would be 0x3c80.	   */
/*									   */
/*		        To hide the fact that local I/O on Jensen requires */
/*			a different swizzle,  local I/O drivers call this  */
/*			routine with the KN121_VTI_BASE address encoded in */
/*			the addr, if it's there we do a local i/o access,  */
/*			otherwise we do an EISA access.			   */
/*                                                                         */
/*	width		Specifies the width in bytes of the data to be     */
/*			read. Supported values are 1, 2, 3, 4 and 8.	   */
/*                                                                         */
/* 	type		Specifies the type of read. Legal values are.	   */
/*			BUS_MEMORY, and BUS_IO.				   */
/*									   */
/* NOTE: Values for type are defined in <io/common/devdriver.h>		   */
/***************************************************************************/

long
kn121_read_io_port (dev_addr, width, type)

io_handle_t	dev_addr;
int		width;
int		type;


{  /* Begin kn121_read_io_port */

   register long	loc_data;
   register long        bus = (dev_addr >> 4) & KN121_EISA_BUS_MASK;
   register long        hae;
   register int 	ipl;

   /* check for io_handle_t types first */
   switch (bus) {

   case KN121_EISA_MEM :

       hae = (dev_addr >> KN121_HAE_SHIFT) & KN121_HAE_MASK;
       HAESET();
       loc_data =*(u_int *)PHYS_TO_KSEG((bus) 
	   | ((dev_addr & 0xfffffff)<<KN121_EISA_ADDR_SHIFT) 
	   | ((width-1) << KN121_WIDTH_SHIFT));
 
       HAECLR();
       return((loc_data >> ((dev_addr&3) << 3)) & ((1L<<(8*(width)))-1)); 

   case KN121_EISA_IO :

       return(((*(u_int *)PHYS_TO_KSEG((bus) 
           | ((dev_addr & 0xfffffff)<<KN121_EISA_ADDR_SHIFT) 
           | ((width-1) << KN121_WIDTH_SHIFT)))
           >> ((dev_addr&3)<<3)) & ((1L<<((width) << 3))-1)); 
   
   case KN121_VTI_BASE :

       /* VTI combo local port read*/
       return( (*(u_int *)PHYS_TO_KSEG((bus)
           | ((dev_addr & 0xfffffff)<<KN121_SHIFT))) & 0xff);
   }
   

   /* now handle the old interface */
   /* These will go away, don't use them!!!!! */
   if ( !((dev_addr & KN121_VTI_BASE) == KN121_VTI_BASE)) {

       /* EISA Bus Read */
       return (((*(u_int *)PHYS_TO_KSEG (KN121_ADDR_TRANSLATE(dev_addr,width,type))) 
           >> ((dev_addr & 3) << 3)) & ((1L<<((width) << 3))-1));

   }
   else

       printf("VTI Read Interface without io_handle_t no longer exists dev_addr:0x%lx \n",dev_addr);

}  /* End kn121_read_io_port */



/******************************************
*
*	write_io_port  Functions
*
*******************************************/

void
kn121_write_io_port (dev_addr, width, type, data)

io_handle_t	dev_addr;
int		width;
int		type;
long		data;


{  /* Begin kn121_write_io_port */

   register long        bus = (dev_addr >> 4) & KN121_EISA_BUS_MASK;
   register long        hae;
   register int 	ipl;

   /* check for io_handle_t type first */
   switch (bus) {

   case KN121_EISA_MEM :
       hae = (dev_addr >> KN121_HAE_SHIFT) & KN121_HAE_MASK;
       HAESET();
       *(u_int *)PHYS_TO_KSEG((bus) 
           | ((dev_addr & 0xfffffffL)<<KN121_EISA_ADDR_SHIFT) 
	   | ((width-1) << KN121_WIDTH_SHIFT)) 
	   = data << ((dev_addr & 3) << 3);
       HAECLR();
       return;
   
   case KN121_EISA_IO :
       *(u_int *)PHYS_TO_KSEG((bus) 
           | ((dev_addr & 0xfffffffL)<<KN121_EISA_ADDR_SHIFT) 
	   | ((width-1) << KN121_WIDTH_SHIFT)) 
           = data << ((dev_addr & 3) << 3);
       return;

   case KN121_VTI_BASE :

       *(u_int *)PHYS_TO_KSEG((bus)
	   | ((dev_addr & 0xfffffffL)<<KN121_SHIFT)) 
           = data & 0xff;
       return;
   }

   /* These will go away, don't use them!!!!!! */
   if ( !((dev_addr & KN121_VTI_BASE) == KN121_VTI_BASE)) {
       
       /* EISA bus write */
       *(u_int *)PHYS_TO_KSEG (KN121_ADDR_TRANSLATE(dev_addr,width,type)) 
           = data << ((dev_addr & 3) << 3); 
   }
   else 
       printf("VTI Write Interface without io_handle_t no longer exists dev_addr:0x%lx \n",dev_addr);

}  /* End kn121_write_io_port */




/*****************************************************************
 *  The following table describes the shifts and masks required when
 *  optimizing for 32bit transfers to/from IO and MEM if source
 *  and destination addresses are not naturally aligned. There are
 *  two cases to consider. For either case, the inner loops will
 *  do one 32bit read for each 32bit write. The only overhead from
 *  the naturally aligned case being register shifts, masks, and
 *  merges.
 *
 *
 *  Case 1 - Left Shift First
 * 
 *  SourceAddr%4 < DestAddr%4 
 *
 *    read fd(first data)
 *    save fd
 *    shift fd left
 *    write fd to dest
 *    restore saved fd to fd
 *    shift fd right<---------+
 *    mask fd                 |
 *    read sd(second data)    |
 *    save sd                 | while (count-- > 4)
 *    shift sd left           | 
 *    mask sd                 |
 *    write fd | sd to dest   |
 *    restore saved sd to fd -+
 *    finish remainder
 *
 *  Case 2 - Right Shift First
 *
 *  SourceAddr%4 > DestAddr%4
 *
 *    read fd
 *    shift fd right
 *    mask fd       
 *    read sd       
 *    save sd       
 *    shift sd left 
 *    mask sd       
 *    write fd | sd to dest  
 *    restore saved sd to fd<-+
 *    shift fd right          |
 *    mask fd                 |
 *    read sd                 | while (count-- > 4)
 *    save sd                 |
 *    shift sd left           |
 *    mask sd                 |
 *    write fd| sd to dest----+
 *    finish remainder
 *
 * ROW SOURCE COL   DEST SHIFTa  MASKa  SHIFTb  MASKb SIZE
 *  0   1111   1    1110   L1   M(1110)  R3    M(0001)  3
 *  0   1111   2    1100   L2   M(1100)  R2    M(0011)  2 
 *  0   1111   3    1000   L3   M(1000)  R1    M(0111)  1 
 *
 *  1   1110   0    1111   R1   M(0111)  L3    M(1000)  4  
 *  1   1110   2    1100   L1   M(1110)  R3    M(0001)  2 
 *  1   1110   3    1000   L2   M(1100)  R2    M(0011)  1 
 *
 *  2   1100   0    1111   R2   M(0011)  L2    M(1100)  4 
 *  2   1100   1    1110   R1   M(0111)  L3    M(1000)  3 
 *  2   1100   3    1000   L1   M(1110)  R3    M(0001)  1 
 *
 *  3   1000   0    1111   R3   M(0001)  L1    M(1110)  4
 *  3   1000   1    1110   R2   M(0011)  L2    M(1100)  3
 *  3   1000   2    1100   R1   M(0111)  L3    M(1000)  2
 **************************************************************/

#define BYTE0 0x000000ff /* masks for byte fields */
#define BYTE1 0x0000ff00
#define BYTE2 0x00ff0000
#define BYTE3 0xff000000

#define sLEFT  0	/* shift left first */
#define sRIGHT 1        /* shift right first */

struct eisa_byte_merge {
   unsigned int ashift; /* shift sizes b = 32-a*/
   unsigned int amask;  /* masks, b = ~a*/
   unsigned int LRflag; /* LEFT = Lshift before Rshift */
}eb_merge[4][4] = {
 {{ 0, 00000|00000|00000|00000, 0},
  { 8, BYTE3|BYTE2|BYTE1|00000, sLEFT},
  {16, BYTE3|BYTE2|00000|00000, sLEFT},
  {24, BYTE3|00000|00000|00000, sLEFT}},

 {{ 8, 00000|BYTE2|BYTE1|BYTE0, sRIGHT},
  { 0, 00000|00000|00000|00000, 0},
  { 8, BYTE3|BYTE2|BYTE1|00000, sLEFT},
  {16, BYTE3|BYTE2|00000|00000, sLEFT}},

 {{16, 00000|00000|BYTE1|BYTE0, sRIGHT},
  { 8, 00000|BYTE2|BYTE1|BYTE0, sRIGHT},
  { 0, 00000|00000|00000|00000, 0},
  { 8, BYTE3|BYTE2|BYTE1|00000, sLEFT}},

 {{24, 00000|00000|00000|BYTE0, sRIGHT},
  {16, 00000|00000|BYTE1|BYTE0, sRIGHT},
  { 8, 00000|BYTE2|BYTE1|BYTE0, sRIGHT},
  { 0, 00000|00000|00000|00000, 0}}
};

/* dmask[DestAlign][count%4] */
/* used when performaing a read modify write on the destination */
unsigned int dmask[4][4] = {
    {00000|00000|00000|00000, 
     BYTE3|BYTE2|BYTE1|00000, 
     BYTE3|BYTE2|00000|00000, 
     BYTE3|00000|00000|00000},

    {00000|00000|00000|BYTE0, 
     BYTE3|BYTE2|00000|BYTE0, 
     BYTE3|00000|00000|BYTE0, 
     00000|00000|00000|BYTE0},

    {00000|00000|BYTE1|BYTE0, 
     BYTE3|00000|BYTE1|BYTE0, 
     00000|00000|BYTE1|BYTE0, 
     00000|00000|BYTE1|BYTE0},

    {00000|BYTE2|BYTE1|BYTE0, 
     00000|BYTE2|BYTE1|BYTE0, 
     00000|BYTE2|BYTE1|BYTE0, 
     00000|BYTE2|BYTE1|BYTE0}
};


/******************************************************************************
 *                                                                        
 * ROUTINE NAME:  kn121_io_copyin()
 * 
 * SYNOPSIS
 *    long kn121_io_copyin(src, dest, count)
 * 
 *    io_handle_t src;
 *    vm_offset_t dest;
 *    u_long count;
 * 
 * PARAMETERS
 *    src 	io_handle_t for source address of copy
 *    dest      kernel vertual address of destination
 *    count     byte count
 * 
 * FUNCTIONAL DESCRIPTION:  
 *    Copys data from EISA I/O or MEMORY spaces to system memory.
 *    Optimizes for 32bit transfers.
 *
 * RETURN VALUE:
 *
 *      IOA_OKAY on succes -1 on failure
 *
 ****************************************************************************/


int kn121_io_copyin(src, dest, count)
io_handle_t src;
vm_offset_t dest;
register u_long count;
{
    /* only needed for nonaligned cases */
    register unsigned int a,b,m,savedb,amask,bmask,mmask,ashift,bshift; 
    struct eisa_byte_merge *ebm;

    /* needed for all cases */
    register char *EisaAddr;
    register char *source = (char *)((long)src & KN121_LOW_ADDR_MASK);
    register unsigned long bus = (src >> 4) & KN121_EISA_BUS_MASK;
    register unsigned long SourceAlign = (unsigned long)source & 3;
    register unsigned long DestAlign = (unsigned long)dest & 3;
    
    /* only needed if hae other than 0x0 */
    int ipl;
    int hae = (src >> KN121_HAE_SHIFT) & KN121_HAE_MASK;

    /* No support for busses other than mem or io */
    if (bus != KN121_EISA_MEM && bus != KN121_EISA_IO){
	printf("kn121_io_copyin() unsupported bus io_handle_t source = 0x%lx\n",src);
	return(-1);
    }

    /* someone may do this */
    if (count == 0)
	return(IOA_OKAY);

    /* Check for naturally aligned */
    if (SourceAlign == DestAlign) {

	EisaAddr = (char *)PHYS_TO_KSEG(((((long)source & ~3L)
	     << KN121_EISA_ADDR_SHIFT) | bus));

	dest = (vm_offset_t)((long )dest & ~3L);

	/* Check for partial first word*/
	if (SourceAlign) {

	    /* mask for read modify write */       
	    amask = dmask[DestAlign][(count >=4) ? 0:count%4];

	    /* we will need this to merge with */
	    a = *(int *)dest & amask;

	    /* See if very small transfer, less than SourceAlign */
	    if (count <= (4-SourceAlign)) {

		/* set eisa byte mask and swizzle */
		EisaAddr = (char *)((long)EisaAddr 
		    | (((long)count-1) << KN121_WIDTH_SHIFT) 
		    | ((long)SourceAlign << KN121_EISA_ADDR_SHIFT));

		/* Read modify Write, Thank alpha for this! */
		/* We could let the compiler handle this, but this performs the same    */
		/* function, and lets us see exactly what's going on. Also, regardless  */
		/* of the optimization level we should get the same code. */

		HAESET();

		*(int *)dest = (*(int *)EisaAddr & ~amask) | a;

		HAECLR();

		return(IOA_OKAY);
	    }
	    else {

		/* set eisa byte mask and swizzle */
		EisaAddr = (char *)((long)EisaAddr 
		    | ((long)(3-SourceAlign) << KN121_WIDTH_SHIFT) 
		    | ((long)SourceAlign << KN121_EISA_ADDR_SHIFT));

		HAESET();
		*(int *)dest = (*(int *)EisaAddr & ~amask) | a;

		/* Do this here so we don't get two sets in a row */
		HAECLR(); 

		(char *)dest += 4;
		(char *)source += (4-SourceAlign);
		count -= (4-SourceAlign);
		EisaAddr = (char *)(((long )EisaAddr & ~KN121_WIDTH_MASK) + KN121_EISA_STRIDE);
	    }
	}

	/* Naturally word alligned from here on */
	EisaAddr = (char *)(((long)EisaAddr 
	    | (3 << KN121_WIDTH_SHIFT)) & KN121_EISA_ALIGN_MASK);
	
	HAESET();

	/* do all the 32 bit aligned cases */
	while (count >= 4) {

	    *(int *)dest = *(int *)EisaAddr;

	    count -= 4;
	    EisaAddr += KN121_EISA_STRIDE;
	    (char *)dest += 4;
	}

	/* check for remainder */
	if (count) {

	    a = *(int *)dest;
	    amask = dmask[0][count];
	    a = a & amask;

	    EisaAddr = (char *)(((long)EisaAddr & (~KN121_WIDTH_MASK&KN121_EISA_ALIGN_MASK)) |
			       ((count -1) << KN121_WIDTH_SHIFT));

	    *(int *)dest = (*(int *)EisaAddr & ~amask) | a;

	}
	HAECLR();
	return(IOA_OKAY);
    }

    /* Nonaligned case */
    ebm = &eb_merge[SourceAlign][DestAlign];
    amask = ebm->amask;
    bmask = ~amask;
    ashift = ebm->ashift;
    bshift = 32-ashift;

    /* align to int */
    dest = (vm_offset_t)((long)dest & ~3L); 

    EisaAddr = (char *)PHYS_TO_KSEG(((((long)source) << KN121_EISA_ADDR_SHIFT) | bus 
	    | ((3-SourceAlign) << KN121_WIDTH_SHIFT)));     
    
    /* LFflag tells which shift first */
    if (ebm->LRflag) {

	HAESET();

	/* correct byte lanes and masked */
	a = ((*(int *)EisaAddr) >> ashift) & amask;

	/* get next data and copy for later*/
	/* eisa will be naturally aligned from now on */
	EisaAddr = (char *)(((long)EisaAddr & KN121_EISA_ALIGN_MASK) 
	    | (3 << KN121_WIDTH_SHIFT)) + KN121_EISA_STRIDE;

	savedb = b = *(int *)EisaAddr;

	/* correct byte lanes and merged */
	a = a | ((b << bshift) & bmask);

	mmask = dmask[DestAlign][(count >=4) ? 0:count%4];

	/*
	 * We will need this to merge with.
	 *
	 * Note: we don't really need this for the DestAlign 0 case, but becasue 
	 * of the way Jensen's cache fill works, this doesn't cost anything.
	 * Jensen can either do the read now and throw the data away, or it
	 * will do it latter if we don't do it now.
	 */
	m = *(int *)dest & mmask;

	/* check for small transfers */
	if (count <= (4-DestAlign)) {
	
	    /* write partial to mem */
	    *(int *)dest = (a & ~mmask) | m;

	    HAECLR();

	    return(IOA_OKAY);
	}

	/* write partial to mem */
	*(int *)dest = (a & ~mmask) | m;

	/* eisa will be naturally aligned from now on */
	EisaAddr = (char *)((long)EisaAddr + KN121_EISA_STRIDE);

	/* update byte counter */
	count -=  (4 - DestAlign);

	/* for all longword(32b) transfers */
	while (count >= 4){

	    /* resore old b, extract, and shift */
	    a = (savedb >> ashift) & amask;

	    /* read new b from source */
	    (char *)dest += 4;

	    savedb = b = *(int *)EisaAddr;

	    /* shift, mask, and merge with a */
	    a = a | ((b << bshift) & bmask);

	    *(int *)dest = a;

	    EisaAddr = EisaAddr + KN121_EISA_STRIDE;
	    count -= 4;
	}


	/* check for remainder */
	if (count) {
	    a = (savedb >> ashift) & amask;
	    (char *)dest += 4;

	    EisaAddr = (char *)(((long)EisaAddr & ~KN121_WIDTH_MASK)
		| ((count-1)<<KN121_WIDTH_SHIFT));

	    a = a | (( *(int *)EisaAddr << bshift) & bmask);

	    mmask = dmask[0][count];
	    m = *(int *)dest & mmask;

	    *(int *)dest = (a & ~mmask) | m;

	}
	HAECLR();
	return(IOA_OKAY); 
    }

    /* LEFT shift first case */
    else {

	HAESET();

	savedb = a = *(int *)EisaAddr;

	a = (a << ashift) & amask;

	mmask = dmask[DestAlign][(count >=4) ? 0 : count%4];

	m = *(int *)dest & mmask;

	/* check for small transfers */
	if (count <= (4-DestAlign)) {

	    /* write to the memory */
	    *(int *)dest = (a & ~mmask) | m;
	    HAECLR();
	    return(IOA_OKAY);
	}
	
	/* write to mem */
	*(int *)dest = (a & ~mmask) | m;


	/* eisa will be naturally aligned from now on */
	EisaAddr = (char *)(((long)EisaAddr & KN121_EISA_ALIGN_MASK) 
	    | (3 << KN121_WIDTH_SHIFT)) + KN121_EISA_STRIDE;

	/* update byte counter */
	count -=  (4 - DestAlign);
	(char *)dest += 4;

	/* for all longword(32b) transfers */
	while (count >= 4){

	    /* resore old b, extract, and shift */
	    b = (savedb >> bshift) & bmask;

	    savedb = a = *(int *)EisaAddr;

	    a = (a << ashift) & amask;

	    *(int *)dest = a | b;

	    EisaAddr = EisaAddr + KN121_EISA_STRIDE;
	    count -= 4;
	    (char *)dest += 4;
	}

	/* check for remainder */
	if (count) {
	    b = (savedb >> bshift) & bmask;
	    EisaAddr = (char *)(((long)EisaAddr & ~KN121_WIDTH_MASK)
		| ((count-1)<<KN121_WIDTH_SHIFT));
	    a = ((*(int *)EisaAddr << ashift) & amask) | b;

	    mmask = dmask[0][count];
	    m = *(int *)dest & mmask;

	    *(int *)dest = (a & ~mmask) | m;

	}
	HAECLR();
	return(IOA_OKAY); 

    }
}


/******************************************************************************
 *                                                                        
 * ROUTINE NAME:  kn121_io_copyout()
 * 
 * SYNOPSIS
 *    long kn121_io_copyout(source, dst, count)
 * 
 *    vm_offset_t source;
 *    io_handle_t dst;
 *    u_long count;
 * 
 * PARAMETERS
 *    source    kernel vertual address of source
 *    dst	io_handle_t for destination address of copy
 *    count     byte count
 * 
 * FUNCTIONAL DESCRIPTION:  
 *    Copys data from system memory to  EISA I/O or MEMORY spaces.
 *    Optimizes for 32bit transfers.
 *
 * RETURN VALUE:
 *
 *      IOA_OKAY on succes -1 on failure
 *
 ****************************************************************************/

int kn121_io_copyout(source, dst, count)
vm_offset_t source;
io_handle_t dst;
register u_long count;
{
    /* only needed for nonaligned cases */
    register unsigned int a,b,savedb,amask,bmask,ashift,bshift; 
    struct eisa_byte_merge *ebm;

    /* needed for all cases */
    register char *EisaAddr;
    register char *dest = (char *)((long)dst & KN121_LOW_ADDR_MASK);
    register unsigned long bus = (dst >> 4) & KN121_EISA_BUS_MASK;
    register long SourceAlign = (unsigned long)source & 3;
    register long DestAlign = (unsigned long)dest & 3;

    /* only needed if hae other than 0x0 */
    int ipl;
    int hae = (dst >> KN121_HAE_SHIFT) & KN121_HAE_MASK;

    /* No support for busses other than mem or io */
    if (bus != KN121_EISA_MEM && bus != KN121_EISA_IO){
	printf("kn121_io_copyin() unsupported bus io_handle_t destination = 0x%lx\n",dst);
	return(-1);
    }

    /* someone may do this */
    if (count == 0)
	return(IOA_OKAY);

    /* Check for naturally aligned */
    if (SourceAlign == DestAlign) {

	EisaAddr = (char *)PHYS_TO_KSEG(((((long)dest & ~3L) 
		<< KN121_EISA_ADDR_SHIFT) | bus));

	source = (vm_offset_t)((long )source & ~3L);

	/* Check for partial first word*/
	if (SourceAlign) {
	   
	    /* See if very small transfer, less than SourceAlign */
	    if (count <= (4-SourceAlign)) {

		/* set eisa byte mask and swizzle */
		EisaAddr = (char *)((long)EisaAddr 
		    | (((long)count-1) << KN121_WIDTH_SHIFT) 
		    | ((long)SourceAlign << KN121_EISA_ADDR_SHIFT));

		HAESET();

		*(int *)EisaAddr = *(int *)source;

		HAECLR();
		return(IOA_OKAY);
	    }
	    else {

		/* set eisa byte mask and swizzle */
		EisaAddr = (char *)((long)EisaAddr 
		    | ((long)(3-SourceAlign) << KN121_WIDTH_SHIFT) 
		    | ((long)SourceAlign << KN121_EISA_ADDR_SHIFT));

		HAESET();

		*(int *)EisaAddr = *(int *)source;

		HAECLR();
		source += 4;
		dest += (4-SourceAlign);
		count -= (4-SourceAlign);
		EisaAddr = (char *)(((long )EisaAddr & ~KN121_WIDTH_MASK) + KN121_EISA_STRIDE);
	    }
	}
	
	/* Naturally word alligned from here on */
	EisaAddr = (char *)(((long)EisaAddr 
	    | (3 << KN121_WIDTH_SHIFT)) & KN121_EISA_ALIGN_MASK);
	
	HAESET();
	while (count >= 4) {

	    *(int *)EisaAddr = *(int *)source;

	    count -= 4;
	    EisaAddr += KN121_EISA_STRIDE;
	    source += 4;
	}

	/* check for remainder */
	if (count) {

	    EisaAddr = (char *)(((long)EisaAddr & (~KN121_WIDTH_MASK&KN121_EISA_ALIGN_MASK))
				| ((count -1) << KN121_WIDTH_SHIFT));

	    *(int *)EisaAddr = *(int *)source;

	}
	HAECLR();
	return(IOA_OKAY);
    }

    /* Nonaligned case*/
    ebm = &eb_merge[SourceAlign][DestAlign];
    amask = ebm->amask;
    bmask = ~amask;
    ashift = ebm->ashift;
    bshift = 32-ashift;

    /* align to int */
    source = (vm_offset_t)((long)source & ~3L); 

    EisaAddr = (char *)PHYS_TO_KSEG(((((long)dest) << KN121_EISA_ADDR_SHIFT) | bus 
	    | ((3-DestAlign) << KN121_WIDTH_SHIFT)));     
    
    /* LFflag tells which shift first */
    if (ebm->LRflag) {

	/* correct byte lanes and masked */
	a = ((*(int *)source) >> ashift) & amask;

	/* get next data and copy for later*/
	source = (vm_offset_t)((long)source +4);
	savedb = b = *(int *)source;

	/* correct byte lanes and merged */
	a |= ((b << bshift) & bmask);

	/* check for small transfers */
	if (count <= (4-DestAlign)) {
	    EisaAddr = (char *)(((long)EisaAddr & ~KN121_WIDTH_MASK) 
		| ((count-1) << KN121_WIDTH_SHIFT));

	    HAESET();

	    /* write to the eisa bus */
	    *(int *)EisaAddr = a;

	    HAECLR();

	    return(IOA_OKAY);
	}

	HAESET();

	/* write to the eisa bus */
	*(int *)EisaAddr = a;

	/* eisa will be naturally aligned from now on */
	EisaAddr = (char *)(((long)EisaAddr & KN121_EISA_ALIGN_MASK) 
	    | (3 << KN121_WIDTH_SHIFT)) + KN121_EISA_STRIDE;

	/* update byte counter */
	count -=  (4 - DestAlign);

	/* for all longword(32b) transfers */
	while (count >= 4){

	    /* resore old b, extract, and shift */
	    a = (savedb >> ashift) & amask;

	    /* read new b from source */
	    source += 4;
	    savedb = b = *(int *)source;

	    /* shift, mask, and merge with a */
	    a |= ((b << bshift) & bmask);

	    *(int *)EisaAddr = a;

	    EisaAddr = EisaAddr + KN121_EISA_STRIDE;
	    count -= 4;
	}

	/* check for remainder */
	if (count) {
	    a = (savedb >> ashift) & amask;
	    source += 4;
	    a = a | (( *(int *)source << bshift) & bmask);
	    EisaAddr = (char *)(((long)EisaAddr & ~KN121_WIDTH_MASK)
		| ((count-1)<<KN121_WIDTH_SHIFT));

	    *(int *)EisaAddr = a;

	}

	HAECLR();

	return(IOA_OKAY); 
    }

    /* LEFT shift first case */
    else {

	savedb = a = *(int *)source;

	a = (a << ashift) & amask;

	/* check for small transfers */
	if (count <= (4-DestAlign)) {
	    EisaAddr = (char *)(((long)EisaAddr & ~KN121_WIDTH_MASK) 
		| ((count-1) << KN121_WIDTH_SHIFT));


	    HAESET();

	    /* write to the eisa bus */
	    *(int *)EisaAddr = a;

	    HAECLR();

	    return(IOA_OKAY);
	}
	
	HAESET();

	/* write to the eisa bus */
	*(int *)EisaAddr = a;

	/* eisa will be naturally aligned from now on */
	EisaAddr = (char *)(((long)EisaAddr & KN121_EISA_ALIGN_MASK) 
	    | (3 << KN121_WIDTH_SHIFT)) + KN121_EISA_STRIDE;

	/* update byte counter */
	count -=  (4 - DestAlign);

	source += 4;

	/* for all longword(32b) transfers */
	while (count >= 4){

	    /* restore old b, extract, and shift */
	    b = (savedb >> bshift) & bmask;

	    savedb = a = *(int *)source;

	    a = (a << ashift) & amask;

	    *(int *)EisaAddr = a | b;
 
	    EisaAddr = EisaAddr + KN121_EISA_STRIDE;
	    count -= 4;
	    source += 4;
	}

	/* check for remainder */
	if (count) {

	    EisaAddr = (char *)(((long)EisaAddr & ~KN121_WIDTH_MASK)
		| ((count-1)<<KN121_WIDTH_SHIFT));

	    *(int *)EisaAddr = (((*(int *)source) << ashift) & amask) 
		    | ((savedb >> bshift) & bmask);

	}

	HAECLR();

	return(IOA_OKAY); 

    }
}

/******************************************************************************
 *                                                                        
 * ROUTINE NAME:  kn121_io_zero()
 * 
 * SYNOPSIS
 *    long kn121_io_zero(dst, count)
 * 
 *    io_handle_t dst;
 *    u_long count;
 * 
 * PARAMETERS
 *    dst	io_handle_t for destination address of zero
 *    count     byte count
 * 
 * FUNCTIONAL DESCRIPTION:  
 *    Zeros data starting at dst for count bytes in EISA I/O or MEMORY spaces.
 *    Optimizes for 32bit writes.
 *
 * RETURN VALUE:
 *
 *      IOA_OKAY on succes -1 on failure
 *
 ****************************************************************************/

int kn121_io_zero(dst, count)
io_handle_t dst;
register u_long count;
{
    /* needed for all cases */
    register char *EisaAddr;
    register char *dest = (char *)((long)dst & KN121_LOW_ADDR_MASK);
    register unsigned long bus = (dst >> 4) & KN121_EISA_BUS_MASK;
    register long DestAlign = (unsigned long)dest & 3;

    /* only needed if hae other than 0x0 */
    int ipl;
    int hae = (dst >> KN121_HAE_SHIFT) & KN121_HAE_MASK;

    /* No support for busses other than mem or io */
    if (bus != KN121_EISA_MEM && bus != KN121_EISA_IO){
	printf("kn121_io_zero() unsupported bus io_handle_t destination = 0x%lx\n",dst);
	return(-1);
    }

    /* someone may do this */
    if (count == 0)
	return(IOA_OKAY);

    /* aligned to longword case */
    EisaAddr = (char *)PHYS_TO_KSEG(((((long)dest) 
		<< KN121_EISA_ADDR_SHIFT) | bus));


    /* Check for partial first word*/
    if (DestAlign) {
	   
        /* See if very small transfer, less than SourceAlign */
        if (count <= (4-DestAlign)) {

	    /* set eisa byte mask and swizzle */
	    EisaAddr = (char *)((long)EisaAddr 
		    | (((long)count-1) << KN121_WIDTH_SHIFT) 
		    | ((long)DestAlign << KN121_EISA_ADDR_SHIFT));

	    HAESET();

	    *(int *)EisaAddr = 0;

	    HAECLR();
	    return(IOA_OKAY);
	}
	else {

	    /* set eisa byte mask and swizzle */
	    EisaAddr = (char *)((long)EisaAddr 
		    | ((long)(3-DestAlign) << KN121_WIDTH_SHIFT) 
		    | ((long)DestAlign << KN121_EISA_ADDR_SHIFT));

	    HAESET();

	    *(int *)EisaAddr = 0;

	    HAECLR();
	    dest += (4-DestAlign);
	    count -= (4-DestAlign);
	    EisaAddr = (char *)(((long )EisaAddr & ~KN121_WIDTH_MASK) + KN121_EISA_STRIDE);
	}
    }
	
    /* Naturally word alligned from here on */
    EisaAddr = (char *)(((long)EisaAddr 
	    | (3 << KN121_WIDTH_SHIFT)) & KN121_EISA_ALIGN_MASK);
	
    HAESET();
    while (count >= 4) {

	*(int *)EisaAddr = 0;

	count -= 4;
	EisaAddr += KN121_EISA_STRIDE;
    }

    /* check for remainder */
    if (count) {

	EisaAddr = (char *)((long)EisaAddr & (~KN121_WIDTH_MASK&KN121_EISA_ALIGN_MASK)
				| ((count -1) << KN121_WIDTH_SHIFT));

	*(int *)EisaAddr = 0;

    }
    HAECLR();
    return(IOA_OKAY);
}

int
kn121_io_copyio(src, dst, len)
   io_handle_t	src;
   io_handle_t	dst;
   u_long	len;
{
	panic("DEC2000_300 does not implement io_copyio yet");
}

io_handle_t
kn121_busphys_to_iohandle(u_int addr, int flags, struct controller *ctlr_p)
{

    if (flags & DENSE_MEMORY){
        return(eisa_sw.dense_mem_base | addr); /* NULL on Jensen */
    }
    else if (flags & BUS_IO) {
        return(eisa_sw.sparse_io_base | addr);
    }
    else if (!(flags | BUS_MEMORY)) { /* because flag BUS_MEMORY = 0 */
        return(eisa_sw.sparse_mem_base | addr);
    }
    else {
        printf("No Flags: busphys_to_iohandle(addr:0x%lx,flags:0x%x,ctlr_p:0x%lx\n",
            addr,flags,ctlr_p);
    }
}


u_long
kn121_iohandle_to_phys(io_handle, flags)
   io_handle_t	io_handle;
   long		flags;
{
#define IOHANDLE_TRANSLATE(handle, width)			\
       ( ((((vm_offset_t)(handle) & KN121_SYSMAP_MASK) >> 4)& \
	  KN121_EISA_BUS_MASK)|	\
         (((vm_offset_t)(handle) & KN121_LOW_ADDR_MASK) 	\
				<< KN121_EISA_ADDR_SHIFT) | 	\
	    ((width - 1) << KN121_WIDTH_SHIFT) )
		

	int	width;
	u_long	phys_addr;

	/* JENSEN/kn121/DEC2000_300 only has sparse space */
	if (flags & HANDLE_SPARSE_SPACE) {
	   if (flags & HANDLE_BYTE)
		width = 1;
	   if (flags & HANDLE_WORD)
		width = 2;
	   if (flags & HANDLE_TRIBYTE)
		width = 3;
	   if (flags & HANDLE_LONGWORD)
		width = 4;
	   if (flags & HANDLE_QUADWORD)
		return((u_long)NULL); /* QW's not supported */

	   phys_addr = (u_long)(IOHANDLE_TRANSLATE(io_handle, width));
	   return (phys_addr);
	} else if (flags & HANDLE_BUSPHYS_ADDR) {
           return(io_handle & ~KN121_SYSMAP_MASK);
        } else {
	        /* No dense space on JENSEN/kn121/DEC2000_300 */
	   return((u_long)NULL);
	}
}



/******************************************
*
*	get_io_handle 
*
*	Given a device address and the width of the data to be accessed,
*	this function returns the handle to be used to access the device.
*	This handle may be an address or a cookie to be handed to some IO
*	handler. What handle is returned is dependent on the platform and
*	bus.
*
*******************************************/

/***************************************************************************/
/*                                                                         */
/* kn121_get_io_handle	-  Returns a translated physical address that can  */
/* 			   be used to access bus address space. 	   */
/*                                                                         */
/* SYNOPSIS                                                                */
/*	vm_offset_t  kn121_get_io_handle (dev_addr, width, type)           */
/*                                                                         */
/*	vm_offset_t	devaddr;	                                   */
/*	int		width;						   */
/*	int		type;						   */
/*                                                                         */
/*                                                                         */
/* PARAMETERS								   */
/*	devaddr		Device physical address to translate. This address */
/*			is a raw device address, e.g. to translate the     */
/*			address for the id of an option in slot 3 this     */
/*			address would be 0x3c80.			   */
/*                                                                         */
/*	width		Specifies the width in bytes of the data to be     */
/*			read. Supported values are 1, 2, 3, and 4.	   */
/*                                                                         */
/* 	type		Specifies the type of read. Legal values are	   */
/*			BUS_MEMORY, and BUS_IO.	NOTE: Values for type are  */
/*			defined in <io/common/devdriver.h>.		   */
/*									   */
/* DESCRIPTION								   */
/*	This function generates a translated physical address that can be  */
/*	used to access the given bus address. If address bits <31:25> are  */
/*	non zero (HAE would be required) the translation fails and an      */
/*	address value  of all 1s returned. Otherwise the translated address*/
/*	is returned. The following code segment shows the use of 	   */
/*	get_io_handle to read the product id of an option in slot 3.	   */
/*									   */
/*	caddr = kn121_get_io_handle (0x3c80, 4, BUS_IO);		   */
/*	pr_id = *(int *)PHYS_TO_KSEG(caddr);				   */
/*									   */
/***************************************************************************/


vm_offset_t
kn121_get_io_handle (dev_addr, width, type)

io_handle_t	dev_addr;
int		width;
int		type;


{  /* Begin kn121_get_io_handle */

    if ((dev_addr & KN121_HIGH_ADDR_MASK) == 0)
	return (KN121_ADDR_TRANSLATE(dev_addr,width,type));
    else
	/* High address bits set. */
	{
#ifdef HAE_SUPPORT
	   printf("get_io_handle: BUSMEM access > 32MB not supported\n");
	   return (~0);
#else /* HAE_SUPPORT */

	   return (KN121_ADDR_TRANSLATE(dev_addr,width,type));


#endif
	}


}  /* End kn121_get_io_handle */



/********************************************/
/*                                          */
/*	io_bcopy  Functions                 */
/*                                          */
/********************************************/

/***************************************************************************/
/*                                                                         */
/* kn121_io_bcopy  -  Copy a block of memory to or from IO space.	   */
/*                                                                         */
/* SYNOPSIS                                                                */
/*	int  kn121_io_bcopy (srcaddr, dstaddr, byte_count)                 */
/*                                                                         */
/*	caddr_t		srcaddr;	                                   */
/*	caddr_t		dstaddr;					   */
/*	int		byte_count;					   */
/*                                                                         */
/*                                                                         */
/* PARAMETERS								   */
/*	srcaddr		Physical address of begining of block to copy from.*/
/*			This can point to system memory or to IO space.    */
/*			If it points to IO space then it must be a 	   */
/*			translated (swizzled) address. This is required    */
/*			so that kn121_io_bcopy can determine which address */
/*			points to IO space. The IO address should be set   */
/*			up for long-word (4 bytes) access.		   */
/*                                                                         */
/*	dstaddr		Physical address of beginning of block to copy to. */
/*			This parameter has the same characteristics as 	   */
/*			'srcaddr'. 					   */
/*                                                                         */
/* 	byte_count	Number of bytes in block to be copied.		   */
/***************************************************************************/


int
kn121_io_bcopy (srcaddr, dstaddr, byte_count)

caddr_t	srcaddr;
caddr_t	dstaddr;
int	byte_count;

{  /* Begin kn121_io_bcopy */

   int		ret_val;
   int		i;
   int		item_count;
   int		length;
   int		increment;
   caddr_t	cur_mem_p;
   caddr_t	cur_io_p;
   
   ret_val = IOA_OKAY;
   /* We determine which address is io by checking to see which one is */
   /* swizzled. Swizzled address has one or both of bits <33:32> set.  */
   if ((ulong_t)srcaddr & KN121_IO)
      {  /* srcaddr has high bits on so it is swizzled and therefore is */
	 /* the IO address. We are copying from an option board.   */
      if ((ulong_t)dstaddr & KN121_IO)
	 {  /* Destination address is swizzled also. Therefore we are */
	    /* copying from one option board to another. Sort of a poor */
	    /* man's peer to peer transfer.  */

	 length = (((ulong_t)srcaddr &
		    KN121_WIDTH_MASK)>>KN121_WIDTH_SHIFT) + 1;
	 increment = length << KN121_EISA_ADDR_SHIFT;
	 item_count = byte_count/length + byte_count%length;
	 
	 /* Now do copy */
	 switch (length)
	    {
	    case  1:	/* Bytes */
	      for (cur_io_p = srcaddr, cur_mem_p = dstaddr, i=0;
		   i<item_count;
		   i++, cur_io_p += increment, cur_mem_p += increment)
		 {
		 *(caddr_t)PHYS_TO_KSEG(cur_mem_p) =
		    *(caddr_t)PHYS_TO_KSEG(cur_io_p);
		 }
	      break;
	      
	    case 2:	/* Words 2bytes */
	      for (cur_io_p = srcaddr, cur_mem_p = dstaddr, i=0;
		   i<item_count;
		   i++, cur_io_p += increment, cur_mem_p += increment)
		 {
		 *(short *)PHYS_TO_KSEG(cur_mem_p) = 
		    *(short *)PHYS_TO_KSEG(cur_io_p);
		 }
	      break;
	      
	    case 4:	/* Long-words  4bytes */
	      for (cur_io_p = srcaddr, cur_mem_p = dstaddr, i=0;
		   i<item_count;
		   i++, cur_io_p += increment, cur_mem_p += increment)
		 {
		 *(int *)PHYS_TO_KSEG(cur_mem_p) = 
		    *(int *)PHYS_TO_KSEG(cur_io_p);
		 }
	      break;
	    default:
	      ret_val = IOA_ERROR;
	      errno = EINVAL;
	      break;
	    }  /* End Switch */

	 }  /* End IO to IO copy */
      else
	 {  /* Destination is not swizzled and therefore is a memory */
	 /* address. We are copying from an option board to memory. */
	 /* Determine count of items to be copied. Total copy size is */
	 /* given in bytes. Item size is generated from the width     */
	 /* specified  in the IO address. Tri-bytes are not supported.*/
	 
	 length = (((ulong_t)srcaddr &
		    KN121_WIDTH_MASK)>>KN121_WIDTH_SHIFT) + 1;
	 increment = length << KN121_EISA_ADDR_SHIFT;
	 item_count = byte_count/length + byte_count%length;
	 /* Now do copy */
	 switch (length)
	    {
	    case  1:	/* Bytes */
	      for (cur_io_p = srcaddr, cur_mem_p = dstaddr, i=0;
		   i<item_count;
		   i++, cur_io_p += increment, cur_mem_p += length)
		 {
		 *(caddr_t)PHYS_TO_KSEG(cur_mem_p) =
		    *(caddr_t)PHYS_TO_KSEG(cur_io_p);
		 }
	      break;
	      
	    case 2:	/* Words 2bytes */
	      for (cur_io_p = srcaddr, cur_mem_p = dstaddr, i=0;
		   i<item_count;
		   i++, cur_io_p += increment, cur_mem_p += length)
		 {
		 *(short *)PHYS_TO_KSEG(cur_mem_p) = 
		    *(short *)PHYS_TO_KSEG(cur_io_p);
		 }
	      break;
	      
	    case 4:	/* Long-words  4bytes */
	      for (cur_io_p = srcaddr, cur_mem_p = dstaddr, i=0;
		   i<item_count;
		   i++, cur_io_p += increment, cur_mem_p += length)
		 {
		 *(int *)PHYS_TO_KSEG(cur_mem_p) = 
		    *(int *)PHYS_TO_KSEG(cur_io_p);
		 }
	      break;
	    default:
	      ret_val = IOA_ERROR;
	      errno = EINVAL;
	      break;
	    }  /* End Switch */
	 }  /* End IO to memory copy */
      }  /* End source is IO */
   else  if ((ulong_t)dstaddr & KN121_IO)
      {  /* dstaddr has high bits on so it is swizzled and therefore is    */
	 /* the IO address. We are copying from memory to an option board. */
	 /* Determine count of items to be copied. Total copy size is */
	 /* given in bytes. Item size is generated from the width     */
	 /* specified  in the IO address. Tri-bytes are not supported.*/
      
      length = (((ulong_t)dstaddr & KN121_WIDTH_MASK)>>KN121_WIDTH_SHIFT) + 1;
      increment = length << KN121_EISA_ADDR_SHIFT;
      item_count = byte_count/length + byte_count%length;
      /* Now do copy */
      switch (length)
	 {
         case  1:	/* Bytes */
	    for (cur_mem_p = srcaddr, cur_io_p = dstaddr, i=0;
		 i<item_count;
		 i++, cur_io_p += increment, cur_mem_p += length)
	       {
	       *(caddr_t)PHYS_TO_KSEG(cur_io_p) =
		  *(caddr_t)PHYS_TO_KSEG(cur_mem_p);
	       }
	    break;
	    
	 case 2:	/* Words 2bytes */
	    for (cur_mem_p = srcaddr, cur_io_p = dstaddr, i=0;
		 i<item_count;
		 i++, cur_io_p += increment, cur_mem_p += length)
	       {
	       *(short *)PHYS_TO_KSEG(cur_io_p) =
		  *(short *)PHYS_TO_KSEG(cur_mem_p);
	       }
	    break;
	    
	 case 4:	/* Long-words  4bytes */
	    for (cur_mem_p = srcaddr, cur_io_p = dstaddr, i=0;
		 i<item_count;
		 i++, cur_io_p += increment, cur_mem_p += length)
	       {
	       *(int *)PHYS_TO_KSEG(cur_io_p) =
		  *(int *)PHYS_TO_KSEG(cur_mem_p);
	       }
	    break;
	  default:
	    ret_val = IOA_ERROR;
	    errno = EINVAL;
	    break;
	 }  /* End switch */
      }  /* End memory to IO copy */
   else
      /* Neither address is swizzled. As a result we cannot determine */
      /* which address is the IO address. Return an error to caller.  */
      ret_val = EFAULT;
   
   return (ret_val);

}  /* End kn121_io_bcopy */



