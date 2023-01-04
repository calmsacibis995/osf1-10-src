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
static char *rcsid = "@(#)$RCSfile: envram.c,v $ $Revision: 1.1.6.7 $ (DEC) $Date: 1993/12/17 20:56:07 $";
#endif

/***************************************************************************
 *
 * envram.c
 *
 * Digital EISA non-volitile ram (DEC2500) device driver
 *
 ***************************************************************************/

#include <data/envram_data.c> 

/*
 * EISA NVRAM I/O register Read/Write Macros
 *
 * Based on Standard bus routines with sc->regbase as base address,
 * simply || the register offset
 */

#define ENVRAM_READIO_D8(a)    READ_BUS_D8((io_handle_t)(sc->regbase | a))
#define ENVRAM_READIO_D16(a)   READ_BUS_D16((io_handle_t)(sc->regbase | a))
#define ENVRAM_READIO_D32(a)   READ_BUS_D32((io_handle_t)(sc->regbase | a))

#define ENVRAM_WRITEIO_D8(a,d) WRITE_BUS_D8((io_handle_t)(sc->regbase | a),(long)d)
#define ENVRAM_WRITEIO_D16(a,d) WRITE_BUS_D16((io_handle_t)(sc->regbase | a),(long)d)
#define ENVRAM_WRITEIO_D32(a,d) WRITE_BUS_D32((io_handle_t)(sc->regbase | a),(long)d)

/*
 * Forward delarations
 */
int	envram_probe(), envram_attach();
int 	eisa_nvram_battery_enable(),  eisa_nvram_battery_disable();
void    envram_zero();
void    envram_read();

/*
 * Driver definitions
 */
caddr_t envramstd[] = { 0 };
struct driver envramdriver = {
    envram_probe, 0, envram_attach, 0, 0, 0, 0, 0, "envram", envram_info};

/*
 * external references
 */
extern void bzero();

#ifdef DEBUG 
#define DEP(x) printf("**** ENVRAM **** "); printf x
#else
#define DEP(x)
#endif

/****************************************************************************
 *
 * ROUTINE NAME:  envram_probe() 
 *									  
 * FUNCTIONAL DESCRIPTION: 						  
 *	This routine checks the presence of the controller, allocates the 
 *	controller's data structures. In addition to filling softc if the
 *	board is found, enable the board memory.
 *									  
 * CALLED BY:  machine autoconfig routines at boot time                   
 *									  
 * FORMAL PARAMETERS:							  
 *		addr -- io_handle_t of the EISA NVRAM registers 
 *		ctlr -- Pointer to the controller structure		  
 *									  
 * IMPLICIT INPUTS:							  
 *		ctlr->conn_priv[0] -- Pointer to 'eisainfo' structure     
 *		ctlr->addr -- KSEG address of controller's base register  
 *		ctlr->physaddr -- Controller's base register physical     
 *				  address				  
 * IMPLICIT OUTPUTS:							  
 *
 *									  
 * RETURN VALUE:							  
 *
 *	Success:	Size of the softc structure.
 *	Failure:	NULL.
 *
 *****************************************************************************/


envram_probe(addr,ctlr)
 caddr_t addr;
 struct controller *ctlr;  

{
    register struct	envram_softc *sc;
    u_int hw_id = 0;
    u_int csr = 0;
    u_char ctrl, cnfg, hibase;
    struct bus_mem mem;
    struct dma dma_p;

    /*
     * Check to see that this is unit 0,  currently only
     * support 1 envram unit.  Can not support multiple until
     * presto interface changes.
     */
    if (ctlr->ctlr_num > 0) return(0);

    /*
     *  Allocate memory for softc structure.
     */
    sc = (struct envram_softc *)kmem_alloc(kernel_map, 
					   sizeof(struct envram_softc));
    if (!sc)
	return(0);
    bzero((char *)sc, sizeof(struct envram_softc));
    envram_softc = sc;

    /*
     * Save the ctrl struct in softc
     */
    sc->ctlr = ctlr;


    /* 
     * I/O register access scheme for envram driver
     *
     * Use a logical addressing scheme:
     *
     *    softc struct will hold io_handle_t for physical base address  
     *	  and I/O acess will be done through R/W macros, passing the 
     *    offset of the target register. The R/W macros are defined
     *    in envram_data.c and use the kernel READ_BUS_D and WRITE_BUS_D
     *	  macros and "or" the offset with the sc->regbase value.
     *	  sc->regbase is the per-option io_handle_t base of the
     *	  EISA NVRAM I/O registers
     */ 

    /* 
     * Get the controller's base address passed as addr. 
     */
    sc->regbase = (u_long)addr;

    /*
     * Read the controller's ID register to ensure it is actually an
     * DEC2500.
     */

    hw_id = ENVRAM_READIO_D32(ENVRAM_ID);

    if (hw_id != ENVRAM_ID_MASK) 
	{

	    printf("envram_probe: Failed to read ID register\n");
	    /* deallocate sc resources */
            kmem_free(kernel_map,sc,sizeof(struct envram_softc));
	    return(0);
        }
    else
	printf("envram_probe: EISA NVRAM present\n");


    /* 
     * EISA Configuration 
     */

    
    /* 
     * Set up softc entries for location and offset of NVRAM cache 
     * for Presto.  
     *
     * The starting io_handle_t of the NVRAM Bus Memory
     * Is available from the bus support information
     *
     * 1MB and 0x400 offset
     */
    sc->cache_offset = ENVRAM_CACHE_OFFSET; 

    /*
     * Get nvram size and io_handle_t of starting address
     */

    if (get_config(ctlr, EISA_MEM, "",&mem,0)) {
        printf("envram probe error\n");
        return(0);
    }

    sc->cache_size = mem.size; 
    sc->cache_base =  (u_long)mem.start_addr;


    sc->cache_phys_start = (u_long)(sc->cache_base + sc->cache_offset);
    sc->cache_kseg_start = (vm_offset_t)(PHYS_TO_KSEG(iohandle_to_phys(sc->cache_phys_start,
							  HANDLE_BUSPHYS_ADDR)));
    sc->saved_mem_sysmap = busphys_to_iohandle(0L,BUS_MEMORY,ctlr);

    /* 
     * account for diag space 
     */
    sc->cache_size = sc->cache_size - ENVRAM_DIAG_RESVED;

    /*
     * Get nvram dma channel
     */

    if (get_config(ctlr, EISA_DMA, "",&dma_p,0)) {
        printf("envram probe error dma channel\n");
	return(0);
    }

    if (dma_p.channel != 7 && dma_p.channel != 5) {
        printf("envram: invalid dma channel %d\n",dma_p.channel);
        return(0);
    }

    /*
     * Enable the module.
     */	
    
    ENVRAM_WRITEIO_D8(ENVRAM_CTRL, EISA_ENABLE_BOARD);
    DEP(("Enableing EISA NVRAM module\n"));
    mb();	

    /*
     * Initialize the CSR
     * Enable the NVRAM memory for writes
     *
     */

    ENVRAM_WRITEIO_D16(ENVRAM_CSR,WRMEM);
    DEP(("Enable ENVRAM for writing\n"));
    mb();	    

    /*
     * Check the console diagnostic results
     */
    envram_read(sc->cache_phys_start-8 ,&sc->diag_status,4);
    DEP(("Diag Register: 0x%x\n",sc->diag_status));

    if (sc->diag_status & BOARD_FAILED) {
        printf("Envram diag reg 0x%x\n",sc->diag_status);
        sc->diag_status = 0;
    }
    else {
        DEP(("diag reg 0x%x\n",sc->diag_status));
        sc->diag_status = 1;
    }

    return(1);
}

/***************************************************************************
 *
 * ROUTINE NAME:  envram_nvram_status() 
 *									  
 * FUNCTIONAL DESCRIPTION: 						  
 *     	Provide Prestoserve with status of diagnostics run on nvram.
 *
 * CALLED BY:  Prestoserve through interface
 *									  
 * FORMAL PARAMETERS:							  
 *			None
 *									  
 * IMPLICIT INPUTS:							  
 *		sc->diag_status -- diag flag set in probe routine
 *									  
 * RETURN VALUE: (sys/presto.h defined status values)
 *
 *	   NVRAM_RDONLY   -- Passed RO diags
 *	   NVRAM_BAD  -- Failed diags
 *		   
 ***************************************************************************/

int eisa_nvram_status()
{
    register struct envram_softc *sc = envram_softc;
    
    if (sc->diag_status) 
        return(NVRAM_RDONLY);    /* Passed RO diags */ 
    else 
	return(NVRAM_BAD);       /* failed diags */

}	


/******************************************************************************
 *
 * ROUTINE NAME:  eisa_nvram_battery_status()
 *									  
 * FUNCTIONAL DESCRIPTION: 						  
 *     	Provide Prestoserve with status of nvram battery.
 *
 * CALLED BY:  Prestoserve through interface
 *									  
 * FORMAL PARAMETERS:							  
 *			None
 *									  
 * IMPLICIT OUTPUTS:
 *	Fills in nvram_batteries global Presto interface
 *
 * RETURN VALUE: (sys/presto.h defined status values)
 *
 *	   0 -- Batteries OK
 *	   1 -- Battery Problem
 *
 *****************************************************************************/

int eisa_nvram_battery_status()
{
    register struct envram_softc *sc = envram_softc;

    nvram_batteries0.nv_nbatteries = 1;	       /* always one battery */
    nvram_batteries0.nv_minimum_ok = 1;	       /* it must be good */
    nvram_batteries0.nv_primary_mandatory = 1; /* primary must be OK */
    nvram_batteries0.nv_test_retries = 1;      /* call this routine 1 time*/

    if ((ENVRAM_READIO_D16(ENVRAM_CSR) & BAT_FAIL)) 
	{
	    nvram_batteries0.nv_status[0] = BATT_OK;
	    return(0);
	} 
    else 
	    return(1); 
}

/******************************************************************************
 *
 * ROUTINE NAME:  eisa_nvram_battery_enable() 
 *									  
 * FUNCTIONAL DESCRIPTION: 						  
 *     	Provide Prestoserve with the ability to enable the battery on
 *	the eisa nvram. 
 *
 * CALLED BY:  Prestoserve through interface
 *									  
 * FORMAL PARAMETERS:							  
 *			None
 *
 * RETURN VALUE: 0 -- Enabled successfully 
 *		 1 -- Not enabled
 *
 *****************************************************************************/
int eisa_nvram_battery_enable()
{
    register struct envram_softc *sc = envram_softc;

    /*
     * required action is to zero BDISC control bit, this disables the 
     * battery disconect circuit.
     */
    ENVRAM_WRITEIO_D16(ENVRAM_CSR,WRMEM|SET_LED);
    ENVRAM_WRITEIO_D8(ENVRAM_BAT,!BAT_DISCON_BIT);
    mb();

    return(0);
}	

/******************************************************************************
 *
 * ROUTINE NAME:  eisa_nvram_battery_disable()
 *									  
 * FUNCTIONAL DESCRIPTION: 						  
 *     	Provide Prestoserve with the ability to disable the battery on
 *	the eisa nvram. 
 *
 * CALLED BY:  Prestoserve through interface
 *									  
 * FORMAL PARAMETERS:							  
 *			None
 *
 * RETURN VALUE: 0 -- Disabled successfully
 *		 1 -- Not Disabled
 *
 *****************************************************************************/
int eisa_nvram_battery_disable()
{
    register struct envram_softc *sc = envram_softc;

    /* The required action is to send sequence "11001" to 
     * the battery disconnect register.  This enables the battery 
     * disconnect circuit. 
     */

    ENVRAM_WRITEIO_D16(ENVRAM_CSR,WRMEM);
    ENVRAM_WRITEIO_D8(ENVRAM_BAT,BAT_DISCON_BIT);
    mb();
    ENVRAM_WRITEIO_D8(ENVRAM_BAT,BAT_DISCON_BIT);
    mb();
    ENVRAM_WRITEIO_D8(ENVRAM_BAT,!BAT_DISCON_BIT);
    mb();
    ENVRAM_WRITEIO_D8(ENVRAM_BAT,!BAT_DISCON_BIT);
    mb();
    ENVRAM_WRITEIO_D8(ENVRAM_BAT,BAT_DISCON_BIT);
    mb();

    return(0);

}

/******************************************************************************
 *                                                                        
 * ROUTINE NAME:  envram_write()
 *                                                                        
 * FUNCTIONAL DESCRIPTION:  
 *
 *	Presto WRITE operation:	Write to NVRAM from Main Memory
 *
 *	write the length block of data pointed to by the source address 
 *      parameter to the EISA NVRAM destination paramter.  This routine
 *	provides the DMA slave capability to write to the NVRAM,  to Program
 *	I/O or just copy to the NVRAM the io_copy() routine can be
 *	used.
 *
 * ASSUMPTIONS:
 *	1. The destination is *always* the NVRAM
 *	2. The source is from Host (Main) memory
 *
 * FORMAL PARAMETERS:  
 *	source -- address of the source of the data to be written 
 *	       Because this is passed by presto interface, the format
 *	       will be a KSEG'd logical physical address.
 *	dest -- destination for the data
 *	       Because this is passed by presto interface, the format
 *	       will be a KSEG'd logical physical address.
 *	len -- length of the block
 *
 * RETURN VALUE:  None
 *             
 *****************************************************************************/

void envram_write(source,dest,len)
	caddr_t source,dest;
	u_int	len;
{
    register struct envram_softc *sc = envram_softc;
    vm_offset_t destptr;
    register int xfer;                  /* size of each partial transfer */
    int retry;                          /* retry counter */
    char *ddest = dest;                 /* destination pointer */

    /* Presto WRITE operation:	Write to NVRAM from Main Memory */

    /* Use DMA if size is larger than 32 bytes */
    if (len > 32) { 
    
        /*
         * Set up destination address passed from Presto,
         * the dest is a main memory virtual address
         */
        destptr = KSEG_TO_PHYS(dest) - sc->cache_base;
        ddest = (char *)destptr;

        /*
         *  Allign destination to 1K
         */
        if (!(xfer = ENVRAM_XFER_SIZE - ((int)ddest & (ENVRAM_XFER_SIZE-1))))
            xfer = ENVRAM_XFER_SIZE;

        if (xfer > len)
            xfer = len;

        /*
         * Allign source to 8K 
         */
        if ((u_int)source/ENVRAM_ALLIGN != ((u_int)source+xfer)/ENVRAM_ALLIGN) 
            xfer = xfer - (((u_int)source+xfer) & (ENVRAM_ALLIGN-1));

        while (1) {
        
	    /*
	     * Set up the 82357 dma controller
	     */
            if (!(dma_map_load(xfer,(vm_offset_t)source,(struct proc *)0,sc->ctlr,&sc->sglp,0,DMA_OUT)))
                panic("envram: dma_map_load failure\n");

            /*
             * Set up NVRAM source address
             */
            ENVRAM_WRITEIO_D16(ENVRAM_DMA0,((u_int)(ddest-4) << 6));
            ENVRAM_WRITEIO_D16(ENVRAM_DMA1,((u_int)ddest >> 5));

            /*
             * Start NVRAM transfer
             */        

            ENVRAM_WRITEIO_D16(ENVRAM_CSR,SET_DREQ|WRMEM|SET_LED);
	    mb();

            /*
             * Bookeeping, bury behind DMA
             */
            len -= xfer;
            source += xfer;
            ddest += xfer;

            /*
             *  Set up for next, allign destination to 1K, NVRAM only handles DMAs
             *  inside of a 1k alligned address range.
             */
            if (!(xfer = ENVRAM_XFER_SIZE - (((int)ddest & (ENVRAM_XFER_SIZE-1)))))
                xfer = ENVRAM_XFER_SIZE;

            if (xfer > len)
                xfer = len;

            /*
             * Allign source to 8K, source will be memory hence 8K for OSF pages.
             */
            if ((u_int)source/ENVRAM_ALLIGN != ((u_int)source+xfer)/ENVRAM_ALLIGN) 
                xfer = xfer - (((u_int)source+xfer) & (ENVRAM_ALLIGN-1));

            /*
             * Spin on ENVRAM DREQ bit. If the hardware works we should never see 
             * this bit set.
             */
            retry = 10;
            while (--retry) 
                if (!(ENVRAM_READIO_D16(ENVRAM_CSR) & SET_DREQ)) 
                    break;

            if (!len)
                break;

            /*
             * If retry expires the hardware is broken.
             */
            if (!retry)
                panic("envram: DMA retry expired\n");

        }

        return;
     }

    io_copyout((vm_offset_t)source,
	       (io_handle_t)(KSEG_TO_PHYS((u_long)dest)|sc->saved_mem_sysmap),
	       len);

}	

/******************************************************************************
 *                                                                        
 * ROUTINE NAME:  envram_read()
 *                                                                        
 * FUNCTIONAL DESCRIPTION:  
 *
 *	Presto READ operation:	Read from NVRAM and write to Main Memory
 *
 *	read the length block of data pointed to by the source address 
 *      parameter to the EISA NVRAM destination paramter.  This routine
 *	provides the DMA slave capability to read to the NVRAM,  to Program
 *	I/O or just copy to the NVRAM the io_copy() routine can be
 *	used.
 *
 * ASSUMPTIONS:
 *	1. The source is *always* from the NVRAM
 *	2. The destination is to Host (Main) memory
 *
 * FORMAL PARAMETERS:  
 *	source -- address of the source of the data to written
 *	       Because this is passed by presto interface, the format
 *	       will be a KSEG'd logical physical address.
 *	dest -- destination for data to go to
 *	       Because this is passed by presto interface, the format
 *	       will be a KSEG'd logical physical address.
 *	len -- length of the block
 *
 * RETURN VALUE:  None
 *             
 *****************************************************************************/

void envram_read(source,dest,len)
	caddr_t source,dest;
	u_int	len;
{
    register struct envram_softc *sc = envram_softc;

    io_copyin((io_handle_t)KSEG_TO_PHYS((u_long)source|sc->saved_mem_sysmap),
	      (vm_offset_t)dest,len);

}	


/******************************************************************************
 *                                                                        
 * ROUTINE NAME:  envram_zero()
 *                                                                        
 * FUNCTIONAL DESCRIPTION:  
 *	Zero the "len" bytes of EISA NVRAM memory starting at "addr" 
 *
 * FORMAL PARAMETERS:
 *	addr - starting address of EISA NVRAM to zero
 *	       Because this is passed by presto interface, the format
 *	       will be a KSEG'd logical physical address.
 *	len - number of bytes to zero
 *
 * RETURN VALUE:
 *
 *	None.
 *
 ****************************************************************************/
void envram_zero(addr,len)
	caddr_t addr;
	u_int	len;
{
    register struct envram_softc *sc = envram_softc;

    io_zero((io_handle_t)KSEG_TO_PHYS((u_long)addr|sc->saved_mem_sysmap),len);

}	

/****************************************************************************
 *                                                                        
 * ROUTINE NAME:  envram_attach()
 *                                                                        
 * FUNCTIONAL DESCRIPTION:  
 *     	Interface exists: make available by defining presto interface 
 *    	routines and calling presto_init.
 *
 * FORMAL PARAMETERS:
 *	ctlr -- struct controller 
 *									  
 * IMPLICIT INPUTS:							  
 *		ctlr->conn_priv[0] -- Pointer to 'eisainfo' structure     
 *		ctlr->addr -- KSEG address of controller's base register  
 *		ctlr->physaddr -- Controller's base register physical     
 *				  address				  
 *
 *		softc struct is available with all EISA NVRAM values
 *
 * IMPLICIT OUTPUTS:
 *
 * RETURN VALUE:
 *
 *	None.
 *
 *****************************************************************************/

envram_attach(ctlr)
	struct controller *ctlr;

{
    u_long envram_id;
    u_int csr;
    register struct envram_softc *sc = envram_softc;

    if (dma_map_alloc(ENVRAM_XFER_SIZE,sc->ctlr,&sc->sglp,0) == 0) 
      panic("envram: dma_map_alloc error\n");

    /* Initialize presto interface entry points which allow
     * Prestoserve access to the NVRAM cache 
     */

    presto_interface0.nvram_status = eisa_nvram_status;
    presto_interface0.nvram_battery_status= eisa_nvram_battery_status;
    presto_interface0.nvram_battery_disable= eisa_nvram_battery_disable;
    presto_interface0.nvram_battery_enable= eisa_nvram_battery_enable;

    /*
     * The following define access routines for the Presto
     * driver to use for its access to the EISA NVRAM.
     * Note that the ioreg routines and the block routines
     * are all expected to be of the format with src,dest,count
     * parameters. The zero routines take arguments of addr,length
     *
     */
    presto_interface0.nvram_ioreg_read = envram_read; 
    presto_interface0.nvram_ioreg_write = envram_write; 
    presto_interface0.nvram_block_read = envram_read;  
    presto_interface0.nvram_block_write = envram_write;
    presto_interface0.nvram_ioreg_zero = envram_zero; 
    presto_interface0.nvram_block_zero = envram_zero; 

    /*
     * The EISA granularity is a byte,  but force the
     * use of 32bit quantities for performance reasons
     */

    /* Min size of a "small" ioreg data block */
    presto_interface0.nvram_min_ioreg = sizeof(int); 

    /* byte alignment restriction for ioreg block */
    presto_interface0.nvram_ioreg_align = sizeof(int);
  
    /* min size of a "large" block data transfer (in bytes) */
    presto_interface0.nvram_min_block = PRFSIZE;	

    /* byte alignment restriction for a block data transfers */
    presto_interface0.nvram_block_align = PRFSIZE;	

    /* PRFSIZE = smallest fragment size for buffer (1K) */

    /*
     * Call presto_init to initialize the Prestoserve driver
     * interfaces
     */
    
    /* RMA - fix Need unique sysid without etherrom!! */
    presto_init(sc->cache_kseg_start, sc->cache_size,
		ENVRAM_NOTMAPPED, ENVRAM_CACHED,
                envram_ssn()); 
}


/*
 *      Determine an unsigned 32 bit unique number from the system
 *      serial number in the hwrbp.   Convert the serial number from ascii
 *      to a hex number.  Any letter over 'F' (or f) is converted to 0xf
 *      modulo the letter. If Jensen, just generate something.
 *
 */
envram_ssn()
{
        extern struct rpb *rpb;
        u_int ssn = 0;
        int i;
        char *cp;

        cp = rpb->rpb_ssn + 9;

        if (*cp == '\0') {
            cp = "NO System Serial Number"+8; 
            printf("envram_ssn:  %s\n",cp-8);
	}
        else {
            DEP(("envram_ssn: %s\n", rpb->rpb_ssn));
        }
	for (i = 0 ; i < 8 ; i++, cp--){
                if (*cp < '9')
                        ssn += (*cp - '0' ) << (i*4);
                else if (*cp < 'G')
                        ssn += (*cp - 'A' + 0xa ) << (i*4);
                else if (*cp < 'a')
                        ssn += ( *cp % 0xf ) << (i*4);
                else if (*cp < 'g')
                        ssn += (*cp - 'a' + 0xa ) << (i*4);
                else
                        ssn += ( *cp % 0xf ) << (i*4);
        }
        DEP(("envram_ssn: id = %x\n", ssn));
        return(ssn);
}

