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

#include "nvtc.h"

/*
 * Digital TC non-volitile ram Interface
*/

#include <data/nvtc_data.c> 
#include <arch/alpha/rpb.h>
extern int cpu;

/*
#define NVTCDEBUG 1 
*/

int	nvtcdebug = 0;			/* debug flag - don't forget NVTCDEBUG */
int	nvtcprobe(), nvtcattach(), nvtcintr();
int	unit;
int	serial = 0;

caddr_t nvtcstd[] = { 0 };

struct	driver nvtcdriver =
	{ nvtcprobe, 0, nvtcattach, 0, 0, 0, 0, 0, "nvtc", nvtcinfo };

extern void bcopy();
extern void bzero();


nvtcprobe(reg,ctlr)
caddr_t reg;
struct controller *ctlr;  
/*
 * Probe the TCNVRAM to see if it's there and initialize data
 * in the softc structure for the driver.
 *
 * Arguments:
 *	reg		Input: 	The base reference register addr. for slot
 *	ctlr		Input:  A controller structure to get the controller
 *				number of the adapter.
 *
 * Return Value:
 *	Success:	Size of the ln_softc structure.
 *	Failure:	NULL.
 */
{
register struct	nvtc_softc *sc;
int unit = ctlr->ctlr_num;

char *c;
caddr_t	temp_compare_buffer, temp_shadow_pointer, temp_source;
int i,cache_len;
u_int compare_value;
register u_int csr;
vm_offset_t cache_pointer;
	
	serial = 0;
	/*
	 *  Allocate memory for softc structure.
	 */
	sc=(struct nvtc_softc *)kmem_alloc(kernel_map, sizeof(struct nvtc_softc));
	if (nvtcdebug)
		printf("nvtc: nvtc_softc is <0x%lx>\n", (vm_offset_t)sc);
	if (!sc)
		return(0);
	bzero((char *)sc, sizeof(struct nvtc_softc));
	nvtc_softc[unit] = sc;

/* We get a sparse space address for the nvram registers when we really want a dense space address */

	sc->regbase = (vm_offset_t)(PHYS_TO_KSEG(DENSE(KSEG_TO_PHYS(reg))));

/* fill out the register addresses in the softc */

	sc->csraddr = (struct nvtcreg *)(sc->regbase + NVTC_CSR);
	sc->daraddr = (struct nvtcreg *)(sc->regbase + NVTC_DAR);
	sc->bataddr = (struct nvtcreg *)(sc->regbase + NVTC_BAT);
	sc->maraddr = (struct nvtcreg *)(sc->regbase + NVTC_MAR);
	sc->bcraddr  = (struct nvtcreg *)(sc->regbase + NVTC_BCR);
	
	/* We will turn on the battery failed and error interrupts. The only time we will enable the dma done
	 * interrupt is when the bcopy to the shadow copy finishes before the dma to nvram.
	 */

	csr = (BURST_128_BYTES | ENBL_ERR_INT | ENBL_BAT_INT); 	/* set only the bits we want */
	NVTCREG_WR(sc->reg_csr,csr);
	mb();	

	/* Check to see if we can support 256 bytes per burst; if so flag for dma routines */
	csr = NVTCREG_RD(sc->reg_csr);

	if(BURST_128_BYTES & csr)
		sc->burst_size = 512;
	else 
		sc->burst_size = 256;
/*
 * We first get the offset to the start of nvram. This is located in the last word of the 4MB NVRAM space which is also an echo of 
 * the word of the 1 MB board. So by reading this one location we are always going to get the proper offset no matter which board
 * we have 1MB or 4MB.
 */

	sc->cache_start = (vm_offset_t)(PHYS_TO_KSEG(DENSE(KSEG_TO_PHYS(reg+TCNVRAM_CACHE_OFFSET))));
	sc->cache_offset = *(u_int *)sc->cache_start;
	sc->cache_start = (vm_offset_t)(PHYS_TO_KSEG(DENSE(KSEG_TO_PHYS(reg+sc->cache_offset))));
	sc->cache_size = *(u_int *)(sc->cache_start + TCNVRAM_DIAG_REGISTER);	/* Fetch all bits in diag register */

	sc->diag_status = (sc->cache_size & BOARD_PASSED) >> 3;	 /* put passed bit in low bit for testing */
	sc->cache_size = (sc->cache_size & BOARD_SIZE) >> 4;
	sc->cache_size *= 0x100000; /* turn from #of MB to # of bytes */
	
	if (sc->cache_size == 0x400000)
		sc->cache_4MB = TRUE; 
	else
		sc->cache_4MB = FALSE;                                                                     

	sc->cache_start += TCNVRAM_DIAG_RESVED;	/* account for diag space */
	sc->cache_size = sc->cache_size - (TCNVRAM_DIAG_RESVED + TCNVRAM_OFFSET_RESVED); /* account for diag space */

	sc->shadow = (caddr_t)kmem_alloc(kernel_map,sc->cache_size); 

	if (!sc->shadow) {
		printf("nvtc%d: Couldn't allocate memory for shadow copy in memory\n", unit);
		return(0);
	}
/* We must init the shadow copy since we have to insure that the NVRAM and the shadow are identical at all times.
 * The reason for this is when there is a write/zero of nvram memory the action is shadowed to main memory, but when there is a
 * read the read is only issued to the shadow copy.
 */
	bzero(sc->shadow,sc->cache_size);
	bcopy(sc->cache_start, sc->shadow, sc->cache_size);

#ifdef NVTCDEBUG
	if(nvtcdebug) {
		sc->compare_buffer = (caddr_t)kmem_alloc(kernel_map,sc->cache_size-TCNVRAM_DIAG_RESVED); 
		bzero(sc->compare_buffer,sc->cache_size);
		bcopy(sc->cache_start,sc->compare_buffer,sc->cache_size);
		temp_compare_buffer = sc->compare_buffer;
	        temp_shadow_pointer = sc->shadow;
		printf("shadow pointer = 0x%x, compare pointer = 0x%x.\n", temp_shadow_pointer, temp_compare_buffer);
		for(i=0; i<=sc->cache_size-1; i++)
			{
	   		if (*temp_compare_buffer != *temp_shadow_pointer) {
				printf("in probe data miscompare on 0x%x, shadow = 0x%x, compare = 0x%x\n",i,temp_shadow_pointer, temp_compare_buffer);
				halt();
			} else {
				temp_compare_buffer++;
				temp_shadow_pointer++;
			}
		}	
	}
#endif /* NVTCDEBUG */
return(1);	
}
/*
 * 	tc_nvram_status
 *
 *     	provide presto with status of diagnostics run on nvram
 *    	 hence, let presto know what to do - recover, etc...
 *
 *	RETURN value:  -1 if no status set, otherwise sys/presto.h defined value
 *
 */

int tc_nvram_status()
{
	register struct nvtc_softc *sc = nvtc_softc[unit];

	if (!sc->diag_status) 
		return(NVRAM_RDWR); /* Passed read write diags/ we can't determine which kind of test it did */
	else 
		return(NVRAM_BAD); /* failed diags */
}

/*
 * 	tc_nvram_battery_status
 *
 *     	update the global battery information structure for Prestoserve
 *     
 *	RETURN value:  0, if batteries are ok;  1, if problem
 */
int tc_nvram_battery_status()
{
	register struct nvtc_softc *sc = nvtc_softc[unit];
	
#ifdef  NVTCDEBUG
	printf("tc_nvram_battery_status: entering ...\n");
#endif /* NVTCDEBUG */
	nvram_batteries0.nv_nbatteries = 1;	   /* one battery */
	nvram_batteries0.nv_minimum_ok = 1;	   /* it must be good */
	nvram_batteries0.nv_primary_mandatory = 1; /* primary must be OK */
	nvram_batteries0.nv_test_retries = 1;	   /* call this routine 1 times
						    * for each "test" */
	
	if (!(NVTCREG_RD(sc->reg_csr) & BAT_FAIL)) {
		nvram_batteries0.nv_status[0] = BATT_OK;
#ifdef  NVTCDEBUG
		printf("tc_nvram_battery_status: exiting ...\n");	
#endif /* NVTCDEBUG */
		return(0);
	} else {
#ifdef  NVTCDEBUG
		printf("tc_nvram_battery_status: exiting ...\n");	
#endif /* NVTCDEBUG */
		return(1); 
	}
}

/*
 *	 tc_nvram_battery_enable
 *
 *  	- arms the battery 
 *  	- required action is to zero BDISC control bit, this disables the 
 *    	  battery disconect circuit.
 *
 *	RETURN value:  1, if batteries successfully enabled, 0 if not  
 */
int tc_nvram_battery_enable()
{
	register struct nvtc_softc *sc = nvtc_softc[unit];

#ifdef  NVTCDEBUG
	printf("tc_nvram_battery_enable: entering ...\n");	
#endif /* NVTCDEBUG */
	NVTCREG_WR(sc->reg_bat,0);
	mb();
	NVTCREG_WR(sc->reg_bat,0);
	mb();
	NVTCREG_WR(sc->reg_bat,0);
	mb();
	NVTCREG_WR(sc->reg_bat,0);
	mb();
#ifdef  NVTCDEBUG
	printf("tc_nvram_battery_enable: exiting ...\n");	
#endif /* NVTCDEBUG */

}	

/*
 * 	tc_nvram_battery_disable
 *
 *	- unarms battery
 *	- required action is to send sequence "11001" to control register
 *	- this enables the battery disconnect circuit
 *	- The excessive wbflushes are used to protect against merged writes
 *
 *	RETURN value:  0, if batteries successfully disabled, 1 if not  
 */
int tc_nvram_battery_disable()
{
	register struct nvtc_softc *sc = nvtc_softc[unit];

#ifdef  NVTCDEBUG
	printf("tc_nvram_battery_disable: exiting ...\n");	
#endif /* NVTCDEBUG */
	NVTCREG_WR(sc->reg_bat,1);
	mb();
	NVTCREG_WR(sc->reg_bat,1);
	mb();
	NVTCREG_WR(sc->reg_bat,0);
	mb();
	NVTCREG_WR(sc->reg_bat,1);
	mb();
#ifdef  NVTCDEBUG
	printf("tc_nvram_battery_disable: exiting ...\n");	
#endif /* NVTCDEBUG */
}	
/*
 * 	tc_nvram_write
 *             
 */
void tc_nvram_write(source,dest,len)
	caddr_t source,dest;
	u_int	len;
{
	register struct nvtc_softc *sc = nvtc_softc[unit];
	u_long	temp_a33_a32,temp_a2_a28,temp_a31_a29, phys, cache_offset;
	vm_offset_t cache_pointer;
	u_int	saved_burst_data;
        u_int	dar_load_value,burst_count,temp_u_int;
	u_int temp_csr;
	int	unaligned;

	caddr_t temp_compare_buffer, temp_shadow_pointer, temp_source;
	int i, cache_len,sw;
	u_int compare_value;


#ifdef  NVTCDEBUG
	printf("tc_nvram_write: entering ...\n");	
#endif /* NVTCDEBUG */
		
	if(len < sc->burst_size) {
     		cache_pointer = sc->cache_start + (vm_offset_t)(dest - sc->shadow);
		bcopy(source,dest,len);
		bcopy(source,cache_pointer,len);
		mb();

#ifdef NVTCDEBUG
		if(nvtcdebug) {
			printf("cache pointer = %lx, len = %lx, cache_start= %lx .\n",(ulong)cache_pointer,len, sc->cache_start);
			printf("source = %lx, dest = %lx, sahdow start = %lx.\n",source,dest,sc->shadow);
		}
#endif /* NVTCDEBUG */

	} else { /* transfer is greater than 256 or 512 bytes which ever is the min size for a DMA transfer */
		cache_offset = dest - sc->shadow;
		cache_offset +=  TCNVRAM_DIAG_RESVED; 			/* Account for diag resved space */
		if (!sc->cache_4MB)
			cache_offset += 0x100000;			/* point to where the 1MB addresses start */

#ifdef NVTCDEBUG
		if(nvtcdebug)
			printf("cache offset = %lx, len = %lx.\n",cache_offset,len);
#endif /* NVTCDEBUG */

	    	if((svatophys(source, &phys)) == KERN_INVALID_ADDRESS) {
			printf("nvtc: FATAL error; source = %x, dest = %x, len = %x.\n",source,dest,len);
			panic("nvtc: tc_nvram_write : Invalid physical address!\n");
		}	
        	/* DMA requirements disallow DMA xfers to cross 2K boundary. So we need to check to make sure our transfer
		 * will not cross one. How we do this is to make sure we have burst mode alignment. We bcopy data over
		 * to get the data to good alignment.
		 */
		if(unaligned = phys % sc->burst_size) {
			bcopy(source,cache_offset+sc->cache_start,unaligned);
			phys += unaligned;
			len -= unaligned;
			cache_offset += unaligned;
		}

		/* The DAR register requires a TC address for the source as follows:
		 * A28:A2,A33:A29 in the register's bits 31-0.		
		 */
		temp_a33_a32 = (phys >> 29) & 0x0000000000000018;
		temp_a2_a28 = ((phys >> 2) << 5);
		temp_a31_a29 = ((phys & 0x00000000E0000000) >> 29);
		dar_load_value = temp_a33_a32 | temp_a2_a28 | temp_a31_a29;

		/* Load up CSRs and punch the GO bit. We can load up DMA registers in any order
		 * so we won't do a mb till just before to GO bit. All must be loaded before GO
		 * bit is set.
		 */
		NVTCREG_WR(sc->reg_dar,dar_load_value);
		NVTCREG_WR(sc->reg_mar,((cache_offset >> 2) << 5));
		burst_count = (len / sc->burst_size) << 6;
		NVTCREG_WR(sc->reg_bcr,burst_count);
		mb();		
		temp_csr = NVTCREG_RD(sc->reg_csr) | DMA_GO;
		NVTCREG_WR(sc->reg_csr,temp_csr);
		mb();		

		/* If transfer isn't a multiple of the DMA burst size then transfer the leftover via a bcopy */
		if (temp_u_int = len % sc->burst_size > 0) {
			bcopy(source+temp_u_int,cache_pointer+temp_u_int,temp_u_int);
			mb();		
		} 

		bcopy(source,dest,len);				/* copy to shadow copy */

		/* try twice before forcing an interrupt, the second time issue a mb to verify we get the correct csr contents */
		temp_csr = NVTCREG_RD(sc->reg_csr);	  	/* if DMA takes longer than bcopy then we wait for an interrupt */
		if (!(temp_csr & DMA_DONE)) {
			mb();
			temp_csr = NVTCREG_RD(sc->reg_csr);
			if (!(temp_csr & DMA_DONE)) {                                                                          
				temp_csr |= ENBL_DONE_INT;
				temp_csr &= ~(DMA_DONE | DMA_GO); /* We don't want to clear the interrupting condition */
				i = splbio();
				NVTCREG_WR(sc->reg_csr, temp_csr); 
				mb();
     				sleep(sc->cache_start,PZERO+1);		
				splx(i);
			}
		}
	}	

#ifdef NVTCDEBUG
	if (nvtcdebug) {
		sw = 1;
		bzero(sc->compare_buffer,sc->cache_size);
		bcopy(sc->cache_start,sc->compare_buffer,sc->cache_size);
		temp_compare_buffer = sc->compare_buffer;
	        temp_shadow_pointer = sc->shadow;
		for(i=0; i<=sc->cache_size-1; i += 8) {
			if (*(long *)temp_compare_buffer != *(long *)temp_shadow_pointer) {
				if (sw) {
					sw = 0;
					printf("\nin write data miscompare on 0x%x, shadow = 0x%x, compare = 0x%x\n",
						i,temp_shadow_pointer, temp_compare_buffer);
					printf("tcnv:ioreg_write source = 0x%x, dest = 0x%x, len 0x%x.\n", source,dest,len); 
					printf("cache_offset = 0x%x, cache_pointer = %x.\n",cache_offset,cache_pointer);
					printf("offset %x, got %x, expected %x.\n",
						i, *(long *)temp_compare_buffer,*(long *)temp_shadow_pointer);
			} else {
					printf("offset %x, got %x, expected %x.\n",
						i, *(long *)temp_compare_buffer,*(long *)temp_shadow_pointer);
					temp_compare_buffer += 8;
					temp_shadow_pointer += 8;
				}
			} else {
				temp_compare_buffer += 8;
				temp_shadow_pointer += 8;
				
			}
		}
	}
	printf("tc_nvram_write: exiting ...\n");	
#endif /* NVTCDEBUG */

}	
/*
 * 	tc_nvram_zero
 *
 */
void tc_nvram_zero(dest,len)
	caddr_t dest;
	u_int	len;
{
	register struct nvtc_softc *sc = nvtc_softc[unit];
	vm_offset_t cache_pointer;
	caddr_t temp_compare_buffer, temp_shadow_pointer;
	int i,cache_len;

#ifdef  NVTCDEBUG
	printf("tc_nvram_zero: entering ...\n");	
#endif /* NVTCDEBUG */

	cache_pointer = sc->cache_start + (vm_offset_t)(dest - sc->shadow);
	bzero(dest,len);
	bzero(cache_pointer,len);
	mb(); 

#ifdef  NVTCDEBUG
	if (nvtcdebug) {
		bzero(sc->compare_buffer,sc->cache_size);
		bcopy(sc->cache_start,sc->compare_buffer,sc->cache_size);
		temp_compare_buffer = sc->compare_buffer;
	        temp_shadow_pointer = sc->shadow;
		for(i=0; i<=sc->cache_size-1; i += 8) {
			if (*(u_long *)temp_compare_buffer != *(u_long *)temp_shadow_pointer) {
				printf("after zero data miscompare on 0x%x, shadow = 0x%x, compare = 0x%x\n",i,temp_shadow_pointer, temp_compare_buffer);
				printf("\ntcnv: nvram_zero dest = 0x%x, len 0x%x.", dest,len); 
			}
			temp_compare_buffer += 8;
			temp_shadow_pointer += 8;
		}
	}
	printf("tc_nvram_zero: exiting ...\n");	
#endif /* NVTCDEBUG */

}	


nvtcattach(ctlr)
	struct controller *ctlr;
/*
 * Interface exists: make available by defining presto interface routines and calling presto_init.
 * 
 * 
 *
 * Arguments:
 *	ctlr		struct controller containing the controller number.
 *
 * Return Value:
 *	None.
 */
{
	u_long nvtc_id;
	int i;
	u_int csr;
  	register struct nvtc_softc *sc = nvtc_softc[ctlr->ctlr_num];

	unit = ctlr->ctlr_num;

	/* Save out slot number */
	
	sc->slot = ctlr->slot;	

        /* Turn on parity for our TC slot in the IOSLOT register only if rev of board will support(all 512 burst boards do) */

	if (sc->burst_size == 512) {
		if (tc_option_control(ctlr,SLOT_PARITY) == SLOT_PARITY) {
			csr = NVTCREG_RD(sc->reg_csr);
		       	csr |= (ENBL_PARITY);
			NVTCREG_WR(sc->reg_csr,csr);
			mb();
		}
	}

	/* setup preto interface entry points for presto access to the NVRAM cache */

	/* 
	 * Initialize the structure that will point to the routines that 
	 * touch both the shadow copy and the NVRAM for Prestoserve
	 */
	presto_interface0.nvram_status = tc_nvram_status;
	presto_interface0.nvram_battery_status= tc_nvram_battery_status;
	presto_interface0.nvram_battery_disable= tc_nvram_battery_disable;
	presto_interface0.nvram_battery_enable= tc_nvram_battery_enable;

	presto_interface0.nvram_ioreg_read = bcopy;
	presto_interface0.nvram_ioreg_write = tc_nvram_write;
	presto_interface0.nvram_block_read = bcopy;
	presto_interface0.nvram_block_write = tc_nvram_write;
	presto_interface0.nvram_ioreg_zero = tc_nvram_zero;
	presto_interface0.nvram_block_zero = tc_nvram_zero;

	presto_interface0.nvram_min_ioreg = sizeof(int);	/* ON Alpha machines 4 bytes is the smallest IO write allowed */
	presto_interface0.nvram_ioreg_align = sizeof(int);
	presto_interface0.nvram_min_block = sc->burst_size;	/* Min size is determined by the HW */
	presto_interface0.nvram_block_align = PRFSIZE;		/* alignment so xfer won't cross 2K boundary on DMA bursts*/

/* We must now get a system ID so we can ID a nvram board which was not created on this system */

	printf("system id of = %x.\n",unique_sysid());
	presto_init(sc->shadow, sc->cache_size, 
			NVTC_NOTMAPPED, NVTC_CACHED, 
		        unique_sysid());

}

nvtcintr(unit)
/*
 * Interface exists: make available by 
 * 
 * 
 *
 * Arguments:
 *	unit - The unit which caused the interrupt
 *
 * Return Value:
 *	None.
 */
	int unit;
{
    register struct nvtc_softc *sc = nvtc_softc[unit];
    register u_int csr;   
                                                           
	mb();
	csr = NVTCREG_RD(sc->reg_csr);
	if (csr & ERROR_SUM) {				/* Determine the error we got, they are all fatal */
		printf("nvtc: WARNING - fatal error was detected on transfer, csr = 0x%x.\n",csr);
		panic("nvtc: FATAL IO error while NVRAM transaction was in progress.\n");
	} else if (csr & BAT_FAIL)			/* See if the battery is failing, its fatal */ 
		panic("nvtc: FATAL battery failed on the nvram module.\n");
	else if (csr & DMA_DONE) {		
		csr |= DMA_DONE;			/* Clear intrrupting condition */
		csr ^= ENBL_DONE_INT;			/* Disable DONE interrupts */
		NVTCREG_WR(sc->reg_csr, csr);
		mb();
		wakeup(sc->cache_start);		/* wakeup process */
	}
#ifdef  NVTCDEBUG
	 else 
		if(nvtcdebug) 
			printf("nvtc: Stray interrupt csr = %x.\n",csr);
#endif /* NVTCDEBUG */

	return;
}
