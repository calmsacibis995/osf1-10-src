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
static char *rcsid = "@(#)$RCSfile: if_tra.c,v $ $Revision: 1.1.2.17 $ (DEC) $Date: 1994/01/11 21:44:36 $";
#endif
/*-----------------------------------------------------------------------
 * 28-Oct-92	Uttam Shikarpur
 * 	Started work on the driver.
 *
 *---------------------------------------------------------------------- */
#include "tra.h"
#if NTRA > 0 || defined(BINARY)

#include <data/if_tra_data.c>
#define ADAP "tra"

int	traattach(), traintr(), traoutput(), traprobe();
int	trainit(), traioctl(), trareset(), trastatus(), tratint();
int	traxmit_watchdog(), tra_read_error_log(), tra_error_recovery();
int 	tra_read_error_log_thread();
void	tra_fill_chars(), tra_fill_counters(), tra_fill_mib_entry();
void	tra_fill_mib_stats_entry();

struct  driver tradriver =
	{ traprobe, 0 ,traattach, 0 , 0, 0, 0,0, "tra", trainfo };

extern  int ether_output();

extern task_t first_task;

/*
 * NOTE1: on dealing with MAC commands and the responses.
 * In the rather archahaic TMS380 chip, we can put in only one command at
 * time into the SCB and wait for the SCB_CLEAR or poll till issuing the
 * next command. 
 * For Issuing multiple commands:
 *	if ( SCB is available for use)
 *	    queue the request.
 *	else 
 *	   queue the request onto the SCB Q and wait to be woken
 *	   up when we get the SCB_CLEAR interrupt
 *	   When woken up de-queue the command and send it to the adapter.
 *
 *	After requesting the command the equivalent status block is
 *      queued in the ssb_q array. Each member of this array is for one
 *	command. Indexing is based on the values used for the commands
 *	given on page 4-70 of the manual.
 *
 * Responding to Commands:
 *	When we get a COMMAND.STATUS we check the type of command and wake
 *	up the thread waiting for the response to be completed. Once
 *	woken up the thread de-queues the status block from the ssb_q array
 *	and processes the response.
 */	

/*
 * NOTE2: In TI lingo, bit 0 is MSB and bit 16 is the LSB. In order
 * 	  to be consistent with the manual all such references indicate
 *	  such a bit ordering.
 */

/*
 * NOTE3: All references made to Page x-xx in the code refer to the book
 *	  "TMS380 Second-Generation Token Ring. Users Guide" by TI.
 */

#define TRAMCLGET(m,head) { \
	if(head) { \
		MGETHDR((m), M_DONTWAIT, MT_DATA) \
	} else {\
		MGET((m), M_DONTWAIT, MT_DATA) \
	} \
	if ((m)) { \
		MCLGET((m), M_DONTWAIT) \
		if (((m)->m_flags & M_EXT) == 0) { \
			m_freem(m); \
			(m) = (struct mbuf *)NULL; \
		} \
	} \
}


#define CONVTOPHYS(virtual, physaddr) { \
		if(svatophys((virtual), ((vm_offset_t *)(physaddr))) == KERN_INVALID_ADDRESS) {\
			panic("tra: CONVTOPHYS: Invalid physical address!\n"); \
		} \
}

#define DENSE(x)	((x) - 0x10000000)
#define printdebug printf
extern struct mbuf *mfree;
extern int trn_units;
int tradebug = 0;

/*
 * Name:	traprobe();
 *
 * Arguments:
 *	reg:	The base register of the adapter. In mips this is the
 *	        phyiscal address. On alpha this a KSEG sparse address.
 *	ctlr:	The unit number of the adapter can be got from this.
 *
 * Functional Description:
 *	This probes the adapter to see if it exists. The various register
 *	values are filled in and the descriptors are initialized.
 *
 *
 * Calls:
 *	
 *
 * Return Codes:
 *	Success:	Size of the softc structure.
 *	Failure:	0
 * 
 */
traprobe(reg,ctlr)
	caddr_t reg;
	struct	controller *ctlr;
{
    struct tra_softc *sc = &tra_softc[ctlr->ctlr_num];
    int unit = ctlr->ctlr_num;
    int return_status;
    u_int *map, *loc, *speed;
    vm_offset_t temp[8];
    u_short i, speed_value;
    char buf[TC_ROMNAMLEN + 1];

    tc_addr_to_name(reg, buf);

    printf("tra%d: DEC DETRA Module Name: %s\n", unit, buf);
    /*
     * 'reg' is a KSEG address in the sparse space. All our i/o data is 16 bits
     * wide.
     */
    sc->basereg = (vm_offset_t)reg;

    /*
     * Set up pointers to the EAGLE (TMS380) register.
     */
    sc->tms_sifdat   	= 	(u_long *)(sc->basereg + TRA_TMS_SIFDAT_REG);  
    sc->tms_sifdat_inc	= 	(u_long *)(sc->basereg + TRA_TMS_SIFDAT_INC_REG);
    sc->tms_sifadr1     =	(u_long *)(sc->basereg + TRA_TMS_SIFADR1_REG);
    sc->tms_sifcmd_sts  = 	(u_long *)(sc->basereg + TRA_TMS_SIFCMD_REG);
    sc->tms_sifacl   	= 	(u_long *)(sc->basereg + TRA_TMS_SIFACL_REG);
    sc->tms_sifadr2  	= 	(u_long *)(sc->basereg + TRA_TMS_SIFADR2_REG);
    sc->tms_sifadx   	= 	(u_long *)(sc->basereg + TRA_TMS_SIFADX_REG);
    sc->tms_dmalen      = 	(u_long *)(sc->basereg + TRA_TMS_DMALEN_REG);

    /*
     * Fill out the addresses of the CSR on the card.
     */
    sc->csr0_leds   	= 	(u_long *)(sc->basereg + TRA_CSR0);
    sc->csr1_link_ctrl  = 	(u_long *)(sc->basereg + TRA_CSR1);
    sc->csr2 = 			(u_long *)(sc->basereg + TRA_CSR2);
    sc->csr3_sw_board_reset = 	(u_long *)(sc->basereg + TRA_CSR3);
    sc->csr4_intr_status  = 	(u_long *)(sc->basereg + TRA_CSR4);
    sc->csr5 = 			(u_long *)(sc->basereg + TRA_CSR5);
    sc->csr6 = 			(u_long *)(sc->basereg + TRA_CSR6);
    sc->csr7 = 			(u_long *)(sc->basereg + TRA_CSR7);

    /*
     * The LEDs on the board may be on. Turn them off.
     */
     TRAREGWR(sc->csr0_leds, 0x0000);
     TRAIOSYNC();

    /*
     * Now that we have the registers of the EAGLE lets get the rest
     * of the addresses from the memory map.
     * These are: address of mac code, size of mac code, address of ACE
     * 		  code, size of ACE code, address of ring speed, ring speed
     *		  storage size, address of error storage, size of error
     *		  storage.
     * NOTE: In case of alpha/Flamingo all the addressing is in sparse space.
     *       This effectively doubles the offsets and computations when
     *	     compared to the MIPS platform.
     */
    map = (u_int *)(sc->basereg + 0x0040);

    for (i = 0; i < 8; i++) {
	loc = (u_int *)(TRAREGRD(map) & 0xff); map++; map++;
	loc = (u_int *)((vm_offset_t)(loc) + ((TRAREGRD(map) & 0xff) << 8)); map++; map++;
	loc = (u_int *)((vm_offset_t)(loc) + ((TRAREGRD(map) & 0xff) << 16)); map++; map++;
	temp[i] = (vm_offset_t)((vm_offset_t)(loc) + ((TRAREGRD(map) & 0xff) << 24)); map++; map++;
    }
    /*
     * The addresses in temp[] are in dense space.
     * We calculate things in the dense space here.
     */
    sc->mac_location = (u_int *)((vm_offset_t)(temp[0]) + (vm_offset_t)DENSE(sc->basereg));
    sc->mac_size = temp[1];
    sc->ace_location = (u_int *)((vm_offset_t)(temp[2]) + (vm_offset_t)DENSE(sc->basereg));
    sc->ace_size = temp[3];
    sc->speed_location = (u_int *)((vm_offset_t)(temp[4]) + (vm_offset_t)DENSE(sc->basereg));
    sc->speed_size = temp[5];
    sc->error_location = (u_int *)((vm_offset_t)(temp[6]) + (vm_offset_t)DENSE(sc->basereg));
    sc->error_size = temp[7];

    /*
     * Set the software reset bit to zero.
     */
    TRAREGWR(sc->csr3_sw_board_reset, 0x0);
    TRAIOSYNC();

#ifdef TRADEBUG1  /******* READ/WRITE ring_speed*********/
    printf("Ring speed = 0x%x\n", sc->ring_speed);
    {
	/*
	 * This is a test to read and write the ring speed.
	 * in the EEPROM.
	 * write a zero, read it; write a 2 and read it.
	 */
	tra_save_ring_speed(sc, 0);
    
	/*
	 * Get the ring speed.
	 */
	for (speed = sc->speed_location; 
	     speed < (sc->speed_location + sc->speed_size); speed++) {
	    if ( *(speed + 1) & 0xff == 0xff) {
		sc->ring_speed = *speed & 0xff;
		break;
	    }
	}
	printf("Ring speed = 0x%x\n", sc->ring_speed);

	tra_save_ring_speed(sc, 2);

	for (speed = sc->speed_location; 
	     speed < (sc->speed_location + sc->speed_size); speed++) {
	    if ( *(speed + 1) & 0xff == 0xff) {
		sc->ring_speed = *speed & 0xff;
		break;
	    }
	}
	printf("Ring speed = 0x%x\n;", sc->ring_speed);

    }
#endif /* TRADEBUG1 */    
    bzero(&sc->count, sizeof(struct tra_counts));
    bzero(&sc->count, sizeof(struct tra_counts));
    sc->time_init_flag = 1;
    return_status = tra_bud_reset(sc);
    return(return_status);
}


/*
 * Name:	traattach(ctlr);
 *
 * Arguments:
 *	ctlr:	The unit number of the adapter can be got from this.
 *
 * Functional Description:
 *      At this stage we know the adpater exists and the mac code has been
 *  down loaded. We let the upper layers
 *  know its there (put it onto the ifnet list) and do whatever other 
 *  initialization is required.
 *  	The init block for the driver is initialized.
 *
 *
 * Calls:
 *
 *
 *
 *
 * Return Codes:
 *	None. 
 */
traattach(ctlr)
struct controller *ctlr;
{
    struct tra_softc *sc = &tra_softc[ctlr->ctlr_num];
    struct ifnet *ifp = &sc->is_if;
    struct sockaddr_in *sin;
    short i;
    short count;
    u_char addr[6];
    u_short value, value_tmp;
    u_int *speed, read_ring_speed;

    sc->is_ac.ac_bcastaddr = (u_char *)etherbroadcastaddr;
    sc->is_ac.ac_arphrd = ARPHRD_802 ;		  
    ifp->if_unit = ctlr->ctlr_num;
    ifp->if_name = "tra";
    /*
     * Default to 4Mbs MTU size for IP 
     * Even though the default ring speed is 16Mbs
     * we set the MTU size to 4Mbs because bridges
     * in the real world don't forward MTU of greater
     * than 4K. Ofcourse, the user can change this by
     * using the 'ifconfig' command.
     */
    ifp->if_mtu = TRN4_RFC1042_IP_MTU; /* Default to 4Mbs MTU size for IP */
    ifp->if_mediamtu = TRN16_RFC1042_MAC_MTU; /* Default to 16Mbs MTU size  - RFC 1042*/
    ifp->if_flags |= IFF_SNAP| IFF_BROADCAST| IFF_NOTRAILERS | IFF_SIMPLEX ;
    ifp->if_type = IFT_ISO88025;
    ifp->if_addrlen = 6; 
    ifp->if_hdrlen = sizeof(struct trn_header) + 8 ; /* LLC header is 8 octects */
    ((struct arpcom *)ifp)->ac_ipaddr.s_addr = 0;
    
    /*
     * Point the system to all the calls it needs to know
     * about this driver.
     */
    sin = (struct sockaddr_in *)&ifp->if_addr;
    sin->sin_family = AF_INET;
    ifp->if_output = ether_output;
    ifp->if_start = traoutput;
    ifp->if_init = trainit;
    ifp->if_ioctl = traioctl;
    ifp->if_reset = trareset;
    ifp->if_watchdog = traxmit_watchdog;
    ifp->if_timer = 0;
    
    sc->alloc_index = 0;
    sc->open_status = MIB1231_ROSTATUS_NOOPEN;
    sc->ring_status = MIB1231_RSTATUS_NO_STATUS;
    sc->mon_cntd = MIB1231_ACTMON_TRUE;	/* True = 1; False = 2; see RFC 1231 */
    sc->pad_rif = 1;	/* default, pad routing field is set */

    /* Read the ring speed out of the adapter */
    for (speed = sc->speed_location; 
	 speed < (sc->speed_location + sc->speed_size); speed++) {
	if ( *(speed + 1) & 0xff == 0xff) {
	    read_ring_speed = *speed & 0xff;
	    break;
	}
    }
    if (read_ring_speed == 0)
	    sc->last_speed = sc->ring_speed = 4;
    else 
	    sc->last_speed = sc->ring_speed = 16;
    /*
     * Initialize the locks.
     */
    simple_lock_init(&sc->scb_clear_lock);
    sc->ssb = NULL;
    sc->scb = NULL;

    /*
     * clear the error log counters.
     */

    sc->error_cnts.line_error = 0;
    sc->error_cnts.burst_error = 0;
    sc->error_cnts.ari_fci_error = 0;
    sc->error_cnts.lost_frame_error = 0;
    sc->error_cnts.rcv_congestion_error = 0;
    sc->error_cnts.frame_copied_error = 0;
    sc->error_cnts.token_error = 0;
    sc->error_cnts.dma_bus_error = 0;
    sc->error_cnts.dma_parity_error = 0;
    
    /*
     * Write the various initialization options.
     */
    sc->init_block.init_options = 0x9f00;   
    sc->init_block.cmd_status_vector = 0;
    sc->init_block.xmit_vector = 0;
    sc->init_block.recv_vector = 0;
    sc->init_block.ring_status_vector =  0;
    sc->init_block.scb_clear_vector = 0;
    sc->init_block.adap_chk_vector = 0;
    sc->init_block.recv_burst_size = 0;
    sc->init_block.xmit_burst_size = 0;
    sc->init_block.dma_abort_threshold = 0x0202;

    sc->parm_align = 0;
    /*
     * NULL out the ssb's we use as waiting pods.
     */
    for (i = 0; i < MAXCMD; i++) {
	sc->ssb_q[i].ssb = NULL;
	sc->ssb_q[i].next = NULL;
    }

    /*
     * Get aligned memory for the SCB and the SSB blocks.
     * These have to be 6 bytes and 8 bytes in length
     * respectively and be aligned to a word boundary.
     */
    if (tra_get_physalign_memory(sc) == NULL)
	    return(0);
    
    /*
     * Now we write the initialization block.
     */
    if (tra_write_init_block(sc) == NULL)
	    return(0);

    /*
     * We know that the device has been successfully initialized at this
     * point. Initialize if_version string.
     */
    ifp->if_version = "DEC DETRA 4/16 Mbs Token Ring Interface";
    
    /*
     * Initialize some parameters in the open block.
     */
    sc->grp_addr = 0;
    sc->func_addr = 0;

    /* 
     * Read the hardware address.
     */
    TRAREGWR(sc->tms_sifadx, 0x0);
    TRAREGWR(sc->tms_sifadr1, 0x0);
    TRAIOSYNC();

    for (i = 0; i < 6; i++) {
        addr[i] = (u_char)(ntohs(TRAREGRD(sc->tms_sifdat_inc)) & 0xff);
    }
    /* The addresses being used are in the canonical form */
    haddr_convert(&addr[0]);
    bcopy(&addr[0], sc->is_dpaddr, 6);
    bcopy(&addr[0], sc->is_addr, 6);

    /*
     * Read the microcode revision number.
     * The following is the format. The value given is in EBCDIC.
     *
     * Format         Vendor Code                    Silicon     Microcode
     * 	       Texas Instruments = 0                Revision      Level
     *	     				           (Rev. 3.1)   (Version 2.10)
     *(EBCDIC)   00      00      00     D4    C5    F3    F1    F2    F1    F0
     *(ASCII)    00      00      00     4D    45    33    31    32    31    30
     *(TEXT)     NULL    NULL    NULL   M     E     3     1     2     1     0
     *	                                |     |
     *Microcode Type -------------------+     +---- Device Type
     *M = MAC  EBCDIC: d4                           F = TMS380C26
     *L = LLC  EBCDIC: d3                           E = TMS380C16
     *C = CAF(copy all frames) EBCDIC:C3            T = 1st Generation
     */

    TRAREGWR(sc->tms_sifadx, 0x01);
    TRAIOSYNC();
    TRAREGWR(sc->tms_sifadr1, sc->adap_ptr.software_level);
    TRAIOSYNC();

     TRAREGRD(sc->tms_sifdat_inc);
    /* read microcode  - 1 byte*/
    value = TRAREGRD(sc->tms_sifdat_inc);
    value &= 0x00ff;
    if (value == 0xd4)
	sc->microcode_type = "MAC";
    else if (value == 0xd3)
    	sc->microcode_type = "LLC";
    else if (value == 0xc3)
    	sc->microcode_type = "CAF";
    else
	sc->microcode_type = "Unknown";

    /* read device type  - 1 byte */
    value = TRAREGRD(sc->tms_sifdat_inc);
    value_tmp = value >> 8;
    if (value_tmp == 0xc5)
	sc->device_type = "TMS380C26";
    else if (value_tmp  == 0xc4)
	sc->device_type = "TMS380C16";
    else if (value_tmp == 0x54)
	sc->device_type = "First Generation";
    else 
	sc->device_type = "Unknown";


    /* Read Silicon Revision - 2 bytes. */
    value_tmp = value & 0x00ff;
    value_tmp = (value_tmp & 0x0f) + 0x30; /* convert to ascii */
    sc->si_rev[0] = (u_char)value_tmp;
    sc->si_rev[1] = '.';
    value = TRAREGRD(sc->tms_sifdat_inc);
    value_tmp = value >> 8;
    value_tmp = (value_tmp & 0x0f) + 0x30; /* convert to ascii */
    sc->si_rev[2] = (u_char)value_tmp;

    /* Read Microcode Level - 3 bytes. */
    value_tmp = value & 0x00ff;
    value_tmp = (value_tmp & 0x0f) + 0x30; /* convert to ascii */
    sc->micro_level[0] = (u_char)(value_tmp);
    sc->micro_level[1] = '.';
    value = TRAREGRD(sc->tms_sifdat_inc);
    value_tmp = value >> 8;
    value_tmp = (value_tmp & 0x0f) + 0x30; /* convert to ascii */
    sc->micro_level[2] = (u_char )(value_tmp);
    value_tmp = value & 0x00ff;
    value_tmp = (value_tmp &0x0f) + 0x30; /* convert to ascii */
    sc->micro_level[3] = (u_char )(value_tmp);

    printf("%s%d: %s, Hardware address: %s\n", ADAP, ifp->if_unit,
	   ifp->if_version, ether_sprintf(sc->is_addr));
    printf("%s%d: Token Ring Chip: %s, Microcode Type: %s, Silicon Revision: %c%c%c, Microcode Level: %c%c%c%c\n", ADAP, ifp->if_unit, sc->device_type, sc->microcode_type, sc->si_rev[0],sc->si_rev[1], sc->si_rev[2], sc->micro_level[0], sc->micro_level[1], sc->micro_level[2], sc->micro_level[3]);
    if_attach(ifp);
    trn_units++ ; 		/* 
				 * number of Token Ring adapters 
				 * "token_ring_units" has been defined 
				 * globally in net/if_ethersubr.c
				 */
}	


/*
 * Name:	tra_get_physalign_memory(sc);
 *
 * Arguments:
 *	sc: softc for the adapter.
 *
 * Functional Description:
 *	This returns a word aligned block of memory for the SSB and SCB.
 *	The SSB and SCB addresses must be an even address aligned on a
 *	word boundary.
 *
 *
 * Calls:
 *	None.
 *
 * Return Codes:
 *	Success: 1
 *	Failure: 0
 *
 */
tra_get_physalign_memory(sc)
struct tra_softc *sc;
{
    struct ifnet *ifp = &sc->is_if;
    static u_short i, j;
    u_int physaddr;

    /*
     * Get the SCB address.
     */
    for (i = 0; i < 32; i++) {
	CONVTOPHYS(&scb_pool[ifp->if_unit][i], &physaddr);
	if (physaddr & 0x00000003)
		continue;
	else {
	    sc->scb = (SYSTEM_COMMAND_BLOCK *)(&scb_pool[ifp->if_unit][i]);
     	    sc->init_block.scb_address_lo = (u_short)(physaddr & 0xffff);
     	    sc->init_block.scb_address_hi = (u_short)((physaddr >> 16) & 0xffff);
	    break;
	}
    }	
    if (sc->scb == NULL) {
    	printf("%s%d: Can't get an aligned address for SCB.\n", 
	   ADAP, ifp->if_unit);
	return(NULL);
    }

    /*
     * Get the SSB address. 
     */
    for (j = 0; j < 32; j++) {
	CONVTOPHYS(&ssb_pool[ifp->if_unit][j], &physaddr);
	if (physaddr & 0x00000003)
		continue;
	else {
	    sc->ssb = (SYSTEM_STATUS_BLOCK*)(&ssb_pool[ifp->if_unit][j]);
     	    sc->init_block.ssb_address_lo = (u_short)(physaddr & 0xffff);
     	    sc->init_block.ssb_address_hi = (u_short)((physaddr >> 16) & 0xffff);

	    break;
	}
    }	

    if (sc->ssb == NULL) {
    	printf("%s%d: Cannot get aligned address for System Status Block (SSB)\n", 
	   ADAP, ifp->if_unit);
	return(NULL);
    }
    return(1);
}


/*
 * Name:	tra_bud_reset(sc);
 *
 * Arguments:
 *	sc:	The softc structure
 *
 * Functional Description:
 *	This routine asserts the hard reset on the EAGLE. See Pg 4-38 for
 *	the algorithm.
 *
 * Calls:
 *	None.
 *
 * Return Codes:
 *	Success:	1
 *	Failure:	0
 */
tra_bud_reset(sc)
struct tra_softc *sc;
{
    struct ifnet *ifp = &sc->is_if;	    
    u_int *header, header_size, *data_p;
    u_short chapter, offset, data;
    u_short copy, status, num_sd, address;
    short count;
    u_short i;

    copy = TRAREGRD(sc->tms_sifacl);
    copy = copy | SIFACL_ARESET;	/* asssert ARESET bit */
    TRAREGWR(sc->tms_sifacl, copy);
    TRAIOSYNC();
    DELAY(1000);
    copy |= SIFACL_CPHALT;		/* halt adapter CPU */
    TRAREGWR(sc->tms_sifacl, copy);
    TRAIOSYNC();
    DELAY(1000);
    copy = TRAREGRD(sc->tms_sifacl);
    /* clear ARESET bit;disable interrupt; */
    copy &= ~(SIFACL_ARESET | SIFACL_SINTEN);	
    TRAREGWR(sc->tms_sifacl, copy);
    TRAIOSYNC();
    DELAY(1000);

    /*
     * The adapter is in a state to start downloading the
     * code. These addresses are in the dense space.
     * Format of the MAC section of the Rom area...
     * 
     * All values are 16 bit words (read LSB, MSB)...
     *
     *      [       Header Size     ] Size of this header (bytes)
     *      [                       ] \
     *      [                       ]  Segment Descriptor(s)
     *      [                       ] /
     *      [  End of Header Marker ] Value = 7FFE (or some other value)
     *      [ first word of firmware]
     *
     * The header size includes both the size of it's location and the
     * the size of the End of header marker (4 bytes).
     *
     * Segment descriptors are 6 bytes (3 shorts);
     *
     *      [   Chapter Address     ] Chapter address to provide TMS380
     *      [   Offset              ] address with chapter to provide
     *      [   Byte count          ] byte count of segment
     *
     * The Header Size can be used to determine how many segment
     * descriptors are contained in the header.  The formula for deriving
     * this number is;
     *
     *      num_sd = (header_size - 4) / 6
     */

    /*
     * mac_location is in the DENSE space.
     */
    header = sc->mac_location;
    header_size = (u_short)(TRAREGRD(header) & 0xff); header++;
    header_size = header_size + ((TRAREGRD(header) & 0xff) << 8); header++;
    num_sd = (header_size - 4 )/6;

    /*
     * Get the pointer to the start of data.
     * data_p = start_of_header + size_of_header * sizeof a int (4 bytes).
     * NOTE: Each byte in the ROM is word addressable but has only one
     *	     byte of information.
     */
    data_p = (u_int *)((u_char *)(sc->mac_location) + header_size * 4);
    for (i = num_sd; i > 0; i--) {
	while(1) {
	    chapter = (u_short)(TRAREGRD(header) & 0xff); header++;
	    chapter = chapter + ((TRAREGRD(header) & 0xff) << 8); header++;
	    offset = (u_short)(TRAREGRD(header) & 0xff); header++;
	    offset = offset + ((TRAREGRD(header) & 0xff) << 8); header++;
	    count = (u_short)(TRAREGRD(header) & 0xff); header++;
	    count = count + ((TRAREGRD(header) & 0xff) << 8); header++;

	    /*
	     * Check if we have reached the end of header marker.
	     */
	    if (chapter == 0x7ffe)
		break;
	    
	    TRAREGWR(sc->tms_sifadx, chapter);
	    TRAIOSYNC();
	    TRAREGWR(sc->tms_sifadr1, offset);
	    TRAIOSYNC();

	    while (count >= 0) {
		data = (u_short)(TRAREGRD(data_p) & 0xff); data_p++;
		data =  data + ((TRAREGRD(data_p) & 0xff) << 8); data_p++;
		/* Swap the data before writing it out */
		TRAREGWR(sc->tms_sifdat_inc, htons(data));
		TRAIOSYNC();
		count -= 2;
	    }
	}
    }

    /*
     * Now execute the BUD.
     */
    copy = TRAREGRD(sc->tms_sifacl);
    copy &= ~(SIFACL_CPHALT);		/* clear CPHALT bit and start BUD */
    TRAREGWR(sc->tms_sifacl, copy);
    TRAIOSYNC();

    for (i = 0; i < 2; i++) {
	DELAY(4000000);	/* 4 seconds */
	status = TRAREGRD(sc->tms_sifcmd_sts) & CMDSTS_M_READ_MASK;
	if (status == CMDSTS_M_INITIALIZE) {
	    copy = TRAREGRD(sc->tms_sifacl);
	    copy |= SIFACL_SINTEN;		/* Enable interrupts */
    	    TRAREGWR(sc->tms_sifacl, copy);
	    TRAIOSYNC();
	    return(1);
	}
	else {
	    /*
	     * We have to re-try.
	     */
	    TRAREGWR(sc->tms_sifcmd_sts, (CMDSTS_M_SOFTWARE_RESET &
					  TRAREGRD(sc->tms_sifcmd_sts)));
	    TRAIOSYNC();
	}
    }
    printf("tra%d: Bring Up Diagnostic (B.U.D) tests failed, SIFCMD/SIFSTS = 0x%X\n", 
	   ifp->if_unit, status);
    printf("tra%d: Failure Reason: ", ifp->if_unit);
    switch(status & 0x000F) {
	  case 0:
	    printf("Initial Test Error.\n");
	    break;
	  case 1:
	    printf("Adapter Software Checksum Error.\n");
	    break;
	  case 2:
	    printf("Adapter RAM Error\n");
	    break;
	  case 3:
	    printf("Instruction Test Error.\n");
	    break;
	  case 4:
	    printf("Context/Interrupt Test Error.\n");
	    break;
	  case 5:
	    printf("Protocol Handler/RI Hardware Error.\n");
	    break;
	  case 6:
	    printf("System Interface Register Error.\n");
	    break;
	  default:
	    printf("Unknown.\n");
	    break;
    }
    sc->scb_clear_flag = 0;
    return(0);
}


/*
 * Name:	tra_error_recovery();
 *
 * Arguments:
 *	None.
 *
 * Functional Description:
 *
 *	This routine is used for recovering from a error. This calls
 *	tra_mac_close() which use the SCB. Therefore this can not be
 *	invoked when we receive an interrupt and has to be called at
 *	a later time.
 *
 * Calls:
 *	trareset();
 *
 * Return Codes:
 *	None.
 */
tra_error_recovery()
{
    thread_t thread;
    struct tra_softc *sc;
    struct ifnet *ifp;

    thread = current_thread();

    /*
     * Collect the argument left by the kernel_thread_w_arg().
     */
    sc = (struct tra_softc *)thread->reply_port;
    ifp = &sc->is_if;
    thread->reply_port = PORT_NULL;

    for(;;) {
	assert_wait((vm_offset_t)&sc->error_recovery_flag, TRUE);
	thread_block();
	trareset(sc);
    }
}


/*
 * Name:	trareset(sc);
 *
 * Arguments:
 *	sc:	The softc structure
 *
 * Functional Description:
 *
 *	This routine is called at times other than during probe. Here the
 *	adapter is closed before performing a reset.
 *
 * Calls:
 *	tra_bud_reset();
 *
 * Return Codes:
 *	Success:	1
 *	Failure:	0
 */
trareset(sc)
struct tra_softc *sc;
{
    struct ifnet *ifp = &sc->is_if;
    int k;

    ifp->if_timer = 0;
    ifp->if_flags &= ~IFF_UP;
    ifp->if_flags &= ~IFF_RUNNING;
    ifp->if_flags &= ~IFF_OACTIVE;

    k = splimp();
    /*
     * Turn off the LEDs.
     */
    TRAREGWR(sc->csr0_leds, 0x0000);
    TRAIOSYNC();

    if (sc->mac_open) {
	tra_free_buffers(sc);
	sc->mac_open = 0;
    }


    /*
     * Down load the code and initiate the BUD.
     */
    if (tra_bud_reset(sc) == 0)
	return(0);

    splx(k);

#ifdef TRADEBUG1
{
    static int sav_timestamp = 0, reset_first = 1, resets = 0;   
    if (reset_first || time.tv_sec - sav_timestamp >= 400) {
	sav_timestamp = time.tv_sec;
	reset_first = 0;
	resets = 0;
    } else if (time.tv_sec - sav_timestamp <= 210 &&  resets > 8) {
	/* We see more than 8 resets in a period of 3.5 minutes
	 * we will go off line.
	 * This is there to prevent the driver trying to continuously reseting
	 * itself in case of a hardware fault.
	 */
	 reset_first = 1;
	 printf("%s%d: %d resets in %d seconds...\n", 
	       ADAP, ifp->if_unit, resets, (time.tv_sec - sav_timestamp));
	printf("%s%d: Taking adapter off the ring\n", ADAP, ifp->if_unit);
	return (0);
    }
    resets++;
}
#endif /* TRADEBUG1 */

    /*
     * Now we write the initialization block.
     */
    if (tra_write_init_block(sc) == NULL)
	    return(0);

    sc->scb_clear_flag = 0;

    /*
     * Initialize and OPEN the adapter.
     */
    if (trainit(ifp->if_unit))
	ifp->if_flags |= IFF_UP;
    else 
	return(0);

    /*
     * Turn LED2 on.
     */
    TRAREGWR(sc->csr0_leds, TRAREGRD(sc->csr0_leds) | 0x0002);
    TRAIOSYNC();
    sc->count.board_reset++;
    return(1);
}


/*
 * Name:	tra_free_buffers(sc);
 *
 * Arguments:
 *	sc:	The softc structure
 *
 * Functional Description:
 *
 *	This frees up all the buffers that have been allocated on the
 *	receive list.
 *
 * Calls:
 *	None.
 *
 * Return Codes:
 *	Success:	1
 *	Failure:	0
 */
tra_free_buffers(sc)
struct tra_softc *sc;
{
    u_short i;
    
    for (i = 0; i < RCVSZ; i++) {
	
	if (sc->rcvlist[i].rcv_mbuf0) {
	    m_freem(sc->rcvlist[i].rcv_mbuf0);
	    sc->rcvlist[i].rcv_mbuf0 = NULL;
	}
	if (sc->rcvlist[i].rcv_mbuf1) {
	    m_freem(sc->rcvlist[i].rcv_mbuf1);
	    sc->rcvlist[i].rcv_mbuf1 = NULL;
	}
	if (sc->rcvlist[i].rcv_mbuf2) {
	    m_freem(sc->rcvlist[i].rcv_mbuf2);
	    sc->rcvlist[i].rcv_mbuf2 = NULL;
	}
    }
    sc->alloc_index = 0;
    /*
     * Free up any buffers that may have been sitting
     * on the transmit queue.
     */
    while (sc->xmit_done_ptr != sc->xmit_ptr) {
	if (sc->xmit_done_ptr->xmit_mbuf) {
	    m_freem(sc->xmit_done_ptr->xmit_mbuf);
	    sc->xmit_done_ptr->xmit_mbuf = NULL;
	}
	sc->xmit_done_ptr = sc->xmit_done_ptr->next_list;
    }
}


/*
 * Name:	tra_mac_close(sc);
 *
 * Arguments:
 *	sc:	The softc structure for the adapter.
 *
 * Functional Description:
 *
 *	This issues the MAC_CLOSE commands.
 *
 * Calls:
 *	None.
 *
 * Return Codes:
 *	Success:	1
 *	Failure:	0
 */
tra_mac_close(sc)
struct tra_softc *sc;
{
    thread_t thread;
    struct ifnet *ifp;
    u_short s;
    SYSTEM_STATUS_BLOCK *l_ssb;

    ifp = &sc->is_if;
    
    sc->ring_state = MIB1231_RSTATE_CLOSING;
    /*
     * Check if there is already a close command. If so
     * return an error.
     */
    if (sc->ssb_q[MAC_CLOSE].ssb == NULL) {
	/*
	 * See if the scb is available.
	 */
 again:	
	s = splimp();
	simple_lock(&sc->scb_clear_lock);
	if (sc->scb_clear_flag) {
	    /*
	     * Its in use. Block and wait to be woken up.
	     */
	    assert_wait(&sc->scb_clear_flag, TRUE);
	    thread_set_timeout(hz * 45);
	    simple_unlock(&sc->scb_clear_lock);
	    splx(s);
	    thread_block();
	    goto again;
	} 
	/*
	 * Its available for use.
	 */
	sc->scb_clear_flag = 1;		
	simple_unlock(&sc->scb_clear_lock);
	splx(s);
	l_ssb = (SYSTEM_STATUS_BLOCK *)kmem_alloc(kernel_map, 
						  sizeof(SYSTEM_STATUS_BLOCK));
	sc->ssb_q[MAC_CLOSE].ssb = l_ssb;
	/*
	 * Load the SCB with the MAC_CLOSE command.
	 * Issue the close command and block till we get woken
	 * up either by a timeout or by the completion of the
	 * command.
	 */
	sc->scb->scb_cmd = htons(MAC_CLOSE);
	sc->scb->scb_parm_0 = 0;
	sc->scb->scb_parm_1 = 0;
	
	TRAREGWR(sc->tms_sifcmd_sts,
		 TRAREGRD(sc->tms_sifcmd_sts) |
		 (CMDSTS_M_INTR_ADAPTER |	
		  CMDSTS_M_SYS_INTERRUPT |
		  CMDSTS_M_EXECUTE |
		  CMDSTS_M_SCB_REQUEST));
	TRAIOSYNC();
	assert_wait(l_ssb, TRUE);
	thread_set_timeout(hz * 45); /* 45 seconds time out */
	thread_block();
	if (current_thread()->wait_result == THREAD_TIMED_OUT) {
	    /*
	     * We have a serious problem.
	     */
	    printf("%s%d: Time out occurred during MAC_CLOSE command\n", 
		   ADAP, ifp->if_unit);
	    /*
	     * We might as well free this up here.
	     * Because if we timed out there is a good chance we
	     * will never get a response back.
	     */
	    kmem_free(kernel_map,
		      l_ssb,
		      sizeof(SYSTEM_STATUS_BLOCK));
	    sc->ssb_q[MAC_CLOSE].ssb = NULL;
	    return(0);
	} else {
	    /*
	     * Check the return status.
	     */
	    if (ntohs(l_ssb->ssb_parm_0) != MAC_CLOSE_SUCCESS) {
		printf("%s%d: MAC_CLOSE command returned an unknown status: %d\n", ADAP, ifp->if_unit, ntohs(l_ssb->ssb_parm_0));
	    }
	    kmem_free(kernel_map,
		      l_ssb,
		      sizeof(SYSTEM_STATUS_BLOCK));
	    sc->ssb_q[MAC_CLOSE].ssb = NULL;
	    sc->mac_open = 0;
	}
	sc->alloc_index = 0;	
	sc->open_status = MIB1231_ROSTATUS_NOOPEN;
    } else {
	printf("%s%d: MAC_CLOSE request pending\n", ADAP, ifp->if_unit);
    }
    sc->ring_state = MIB1231_RSTATE_CLOSED;
    return(1);
}


/*
 * Name:	tra_write_init_block(sc);
 *
 * Arguments:
 *	sc:	The softc structure for the corresponding adapter.
 *
 * Functional Description:
 *	This function writes the initialization block into the adapter.
 * The algorithm followed here and the values used are given in the
 * TMS380 hand book.
 *
 * Calls:
 *	None.
 *
 * Return Codes:
 *	Success: 1
 *	Failure: 0
 *
 */
tra_write_init_block(sc)
struct tra_softc *sc;
{
    u_short *initblock, i;
    u_short cmdsts_value, delay = 0;
    struct ifnet *ifp = &sc->is_if;

    TRAREGWR(sc->tms_sifadx,  0x0001);
    TRAIOSYNC();
    TRAREGWR(sc->tms_sifadr2, 0x0A00);
    TRAIOSYNC();

    initblock = (u_short *)(&sc->init_block);

    /*
     * Write out 22 bytes of the init block - 16 bits at a time - into
     * the SIFDAT/INC register.
     */
    for (i = 0; i < 11; i++) {
	TRAREGWR(sc->tms_sifdat_inc, initblock[i]);
	TRAIOSYNC();
    }
    /*
     * Write out the addresses.
     */

    /*
     * Write 0x9080 to the SIFCMD/SIFSTS register causing to set the
     * INTERRUPT_ADAPTER and EXCECUTE bits. This prevents resetting the
     * SYSTEM_INTERRUPT bit and clears everything else.
     */
    TRAREGWR(sc->tms_sifcmd_sts, CMDSTS_M_INTR_ADAPTER | 
				 CMDSTS_M_EXECUTE | 
				 CMDSTS_M_SYS_INTERRUPT);
    TRAIOSYNC();
    
    /*
     * Keep checking the value of the SIFCMD/SIFSTS register.
     */
    while(1) {
	DELAY(100);
	cmdsts_value = TRAREGRD(sc->tms_sifcmd_sts);
	if (cmdsts_value & CMDSTS_M_INIT_TEST_ERR) {
	    if (delay++ > 1000) {
		printf("%s%d: Error writing initialization block: SIFCMD/SIFSTS = 0x%X\n", ADAP, ifp->if_unit, cmdsts_value);

		if (cmdsts_value & CMDSTS_M_ERROR)
			print_init_error(ifp->if_unit);
		return(NULL);

	    }
	    continue;
	} 
	break;
    }
#ifdef TRADEBUG1
    /*
     * Otherwise the bits are cleared.
     * SCB should contain: 0x0000C1E2D48B and SSB should contain:
     * 0xFFFD1D7C5D9C3D4
     */
    if (tradebug > 2) {
	printf("%s%d: scb_cmd = 0x%X, scb_parm_0 = 0x%X, scb_parm_1 = 0x%X\n", 
	       ADAP, 
	       ifp->if_unit, 
	       ntohs(sc->scb->scb_cmd),
	       ntohs(sc->scb->scb_parm_0),
	       ntohs(sc->scb->scb_parm_1));
	printf("%s%d: ssb_cmd = 0x%X, ssb_parm_0 = 0x%X, ssb_parm_1 = 0x%X, ssb_parm2 = 0x%X\n", 
	       ADAP, 
	       ifp->if_unit, 
	       ntohs(sc->ssb_copy.ssb_cmd),
	       ntohs(sc->ssb_copy.ssb_parm_0),
	       ntohs(sc->ssb_copy.ssb_parm_1),
	       ntohs(sc->ssb_copy.ssb_parm_2));
    }
#endif TRADEBUG1
    get_internal_ptrs(sc);
    return(1);
}



/*
 * Name:	trainit()
 *
 * Arguments:
 *	unit:	The unit number of the adapter.
 *
 * Functional Description:
 *	This function is responsible for initializing the token ring
 *	adapter to make it operational. At this point we will be able
 * 	allocate mbufs for recieve and will issue the OPEN command.
 *	The xmt and receive buffers will be initialized.
 *
 * Calls:
 *	None.
 *
 * Return Codes:
 *	Success:	1
 *	Failure:	0
 *
 */
trainit(unit)
int unit;
{
    struct tra_softc *sc = &tra_softc[unit];
    struct ifnet *ifp = &sc->is_if;
    u_int physaddr, *speed, read_ring_speed;
    SYSTEM_STATUS_BLOCK *l_ssb;
    u_short s, open_option = 0, write_ring_speed;
    static thread_t thread1 = NULL, thread2 = NULL;
    
    /*
     * If we are already open we don't do anything.
     */
    if (ifp->if_flags & IFF_RUNNING)
	    return(1);

    sc->ring_state = MIB1231_RSTATE_OPENING;

    /*
     * Check if there is already an open command. If so
     * return an error.
     */
    if (sc->ssb_q[MAC_OPEN].ssb != NULL) {
	printf("%s%d: MAC_OPEN request pending\n", ADAP, ifp->if_unit);
	return(0);
    }

    /*
     * Get the ring speed.
     * NOTE: This is a value stored in the EEPROM. This has no relation
     *	     to the actual speed the ring may be running at. i.e this
     *	     value of speed does not change when the speed of the ring
     *	     changes. This value is stored by the user and can be modified
     *	     by the user. The default value if 2, which indicates a ring
     *	     speed of 16Mb/s. 
     *	The ring speed can be modified by writing to the sc->csr1_link_ctrl
     *  CSR before issuing the open command.
     */

    for (speed = sc->speed_location; 
	 speed < (sc->speed_location + sc->speed_size); speed++) {
	if ( *(speed + 1) & 0xff == 0xff) {
	    read_ring_speed = *speed & 0xff;
	    break;
	}
    }


    if (sc->ring_speed != 16 && sc->ring_speed != 4)
	sc->ring_speed = sc->last_speed;  /* default it to the last speed */

    if (sc->ring_speed == 4) {
	    write_ring_speed = 0;	/* 4Mbs */
	    sc->etr = 0;		/* Early Token release is not set */
	    ifp->if_baudrate = TRN_BANDWIDTH_4MB;
	    if (ifp->if_mtu > TRN4_RFC1042_IP_MTU || ifp->if_mtu < 1)
		    ifp->if_mtu = TRN4_RFC1042_IP_MTU;
	    ifp->if_mediamtu = TRN4_RFC1042_MAC_MTU; /* set MAC MTU size - RFC 1042*/
    } else {
	    write_ring_speed = 2;	/* 16 Mbs */
	    sc->etr = 1;		/* Early Token release is set */
	    ifp->if_baudrate = TRN_BANDWIDTH_16MB;
	    if (ifp->if_mtu > TRN16_RFC1042_IP_MTU || ifp->if_mtu < 1)
		    ifp->if_mtu = TRN16_RFC1042_IP_MTU;
	    ifp->if_mediamtu = TRN16_RFC1042_MAC_MTU; /* set MAC MTU size - RFC 1042*/
    }

    if (write_ring_speed != (u_short)(read_ring_speed)) {
	if (read_ring_speed == 0)
	    sc->last_speed = 4;
	else
	    sc->last_speed = 16;
	tra_save_ring_speed(sc, write_ring_speed);
    }

    TRAREGWR(sc->csr1_link_ctrl, write_ring_speed);
    TRAIOSYNC();

#ifdef TRADEBUG1
    for (speed = sc->speed_location; 
	 speed < (sc->speed_location + sc->speed_size); speed++) {
	if ( *(speed + 1) & 0xff == 0xff) {
	    read_ring_speed = *speed & 0xff;
	    break;
	}
    }

    printf("read_ring_speed = 0x%x\n", read_ring_speed);
    printf("sc->ring_speed = 0x%x\n", sc->ring_speed);
    printf("write_ring_speed = 0x%x, sc->csr1_link_ctrl = 0x%x\n", write_ring_speed, TRAREGRD(sc->csr1_link_ctrl) & 0xff);
#endif TRADEBUG1


    /*
     * Get the PRODUCT ID.
     */
    bcopy(PRODUCT_ID, sc->product_id, PIDSZ);

    /*
     * Initialize the lists.
     */
    if (tra_buffers_init(unit) == 0)
	    return(0);

    if (sc->pad_rif)
	open_option |= OPEN_PAD_ROUT_FIELD;

    if (sc->mon_cntd == MIB1231_ACTMON_TRUE)
	open_option |= OPEN_CONTENDER;

    if (sc->etr)
	open_option |= OPEN_EARLY_TOKEN_RELEASE;

    /*
     * Set up buffer for the OPEN command.
     * OPEN options.
     * If we are operating in the loopback mode the value will be
     * 0x8502 else the value will be 0x0500.
     */
    if (ifp->if_flags & IFF_LOOPBACK)
	    sc->open_block.open_options = htons(OPEN_WRAP_INTERFACE |
						open_option);
    else 
	    sc->open_block.open_options = htons(open_option);



    /*
     * Set the node address. 
     * Use the non-canonical form.
     */
    {
	u_char tmp_addr[6];
    	bcopy(sc->is_addr, tmp_addr, 6);
    	haddr_convert(tmp_addr);
        /*
         * The MSBit of the MSByte should be zero.
         * Otherwise the open will fail.
         */ 
         if (tmp_addr[0] & 0x80) {
    	     printf("%s%d: Illegal node address: %s\n", ADAP, ifp->if_unit,
						  ether_sprintf(sc->is_addr));
	     printf("%s%d: Setting bit 0 of byte 0 to zero.\n", ADAP, ifp->if_unit);
	     tmp_addr[0] &= 0x7f;
         }
    	bcopy(tmp_addr, sc->open_block.node_address, 6);
    }
   
    /*
     * Set the group address.
     * Initially set to zero.
     */
     bcopy((u_char *)&(sc->grp_addr), sc->open_block.group_address, 4);

    /*
     * Set the functional address.
     * Initially set to zero.
     */
     bcopy((u_char *)&(sc->func_addr), sc->open_block.functional_address, 4);

    /*
     * Set the receive list size.
     * We use 3 data blocks per list.
     */
    sc->open_block.rcv_list_size = htons(20);

    /*
     * Set the transmit list size.
     */
    sc->open_block.xmit_list_size = htons(26);

    /*
     * Buffer size.
     */
    sc->open_block.buffer_size = htons(BUFFER_SIZE);

    /*
     * Transmit buffer count.
     */
    sc->open_block.xmit_buf_min_max = htons((XMIT_BMIN_CNT << 8) | 
					     XMIT_BMAX_CNT);
    /*
     * The product ID.
     */
    CONVTOPHYS(sc->product_id, &physaddr);
    HOST_TO_TMS(physaddr,
		sc->open_block.product_id_addr_hi,
		sc->open_block.product_id_addr_lo);       

    /*
     * See if the scb is available.
     */
  again:
    s = splimp();
    simple_lock(&sc->scb_clear_lock);
    if (sc->scb_clear_flag) {
	/*
	 * Its in use. Block and wait to be woken up.
	 */
	assert_wait(&sc->scb_clear_flag, TRUE);
	thread_set_timeout(hz * 10);
	simple_unlock(&sc->scb_clear_lock);
	splx(s);
	thread_block();
	goto again;
    } 

    /*
     * Its available for use.
     */
    sc->scb_clear_flag = 1;		
    simple_unlock(&sc->scb_clear_lock);
    splx(s);
    l_ssb = (SYSTEM_STATUS_BLOCK *)kmem_alloc(kernel_map, 
					    sizeof(SYSTEM_STATUS_BLOCK));
    sc->ssb_q[MAC_OPEN].ssb = l_ssb;
    /*
     * Load the SCB with the OPEN command.
     * Issue the open command and block till we get woken
     * up either by a timeout or by the completion of the
     * command.
     */
    sc->scb->scb_cmd = htons(MAC_OPEN);
    CONVTOPHYS(&sc->open_block, &physaddr);
    HOST_TO_TMS(physaddr,
		sc->scb->scb_parm_0,       
		sc->scb->scb_parm_1);

    TRAREGWR(sc->tms_sifcmd_sts,
	     TRAREGRD(sc->tms_sifcmd_sts) |
	     (CMDSTS_M_INTR_ADAPTER |	
	      CMDSTS_M_SYS_INTERRUPT |
	      CMDSTS_M_EXECUTE | 
	      CMDSTS_M_SCB_REQUEST));
    TRAIOSYNC();
    assert_wait(l_ssb, TRUE);
    /*
     * This can take a VERY long time. 
     */
    thread_set_timeout(hz * 45); /* 45 seconds time out */
    thread_block();
    if (current_thread()->wait_result == THREAD_TIMED_OUT) {
	/*
	 * We have a serious problem.
	 */
	 printf("%s%d: Time out occurred during MAC_OPEN command\n", ADAP, ifp->if_unit);
	 /*
	  * We might as well free this up here.
	  * Because if we timed out there is a good chance we
	  * will never get a response back.
	  */
	 kmem_free(kernel_map,
	           l_ssb,
		   sizeof(SYSTEM_STATUS_BLOCK));
	 sc->ssb_q[MAC_OPEN].ssb = NULL;
	 /*
	  * If MAC_OPEN fails, we should free up all the
	  * receive buffers that were allocated.
	  */
	 tra_free_buffers(sc);
	 sc->ring_state = MIB1231_RSTATE_OPEN_FAILURE;
	 sc->count.selftest_fail++;
	 return(0);
    } else {
	/*
	 * we got back a response for OPEN.
	 * check if it is a REJECT.
	 */
	if (ntohs(l_ssb->ssb_cmd) == COMMAND_REJECT) {
	    u_short reason; 
	    printf("%s%d: MAC_OPEN command rejected: ", ADAP, ifp->if_unit);
	    reason = ntohs(l_ssb->ssb_parm_0);
	    print_reject_reason(reason);
	    sc->ssb_q[MAC_OPEN].ssb = NULL;
	    kmem_free(kernel_map,
		      l_ssb,
		      sizeof(SYSTEM_STATUS_BLOCK));
	    sc->ring_state = MIB1231_RSTATE_OPEN_FAILURE;
	    return(reason);
	}

	/*
	 * Check the return status.
	 */
	if (ntohs(l_ssb->ssb_parm_0) == OPENC_OPEN_SUCCESS) {
	    kmem_free(kernel_map,
		      l_ssb,
		      sizeof(SYSTEM_STATUS_BLOCK));
	    sc->ssb_q[MAC_OPEN].ssb = NULL;
	    sc->mac_open = 1;
	    sc->open_status = MIB1231_ROSTATUS_OPEN;
	} else {
	    printf("%s%d: MAC_OPEN : ", ADAP, ifp->if_unit);
	    print_open_error(sc, ntohs(l_ssb->ssb_parm_0));
	    kmem_free(kernel_map,
		      l_ssb,
		      sizeof(SYSTEM_STATUS_BLOCK));
	    sc->ssb_q[MAC_OPEN].ssb = NULL;
	    sc->ring_state = MIB1231_RSTATE_OPEN_FAILURE;
	    sc->count.selftest_fail++;
	    return(0);
	}
    }
    /*
     * The only way of knowing that the diagnostics have succeeded
     * is if the OPEN succeeded. So, here we turn the diagnostic
     * LED1.
     */
     TRAREGWR(sc->csr0_leds, TRAREGRD(sc->csr0_leds) | 0x0001);
     TRAIOSYNC();

    /*
     * Start up the threads here.
     * These have to be threads. They have to use the SCB and can
     * be invoked when the SCB is already in use. If these are threads
     * they can be woken up when the SCB is available.
     */
    if (!thread1) {
	thread1 = kernel_thread_w_arg(first_task,
				     tra_read_error_log_thread,
				     sc);
	if (thread1 == NULL) {
	    printf("%s%d: Cannot start read error log thread.\n",
		   ADAP, ifp->if_unit);
	}
    }

    if (!thread2) {
	thread2 = kernel_thread_w_arg(first_task,
				     tra_error_recovery,
				     sc);

	if (thread2 == NULL) {
	    printf("%s%d: Cannot start error recovery thread.\n",
		   ADAP, ifp->if_unit);
	}
    }

    /*
     * We issue a RECEIVE command. In this driver we are using
     * the Valid Method. So, we need to issue the receive command
     * once after the adapter has been initialized. 
     * We get back a status report if a) a transmit was done or b)
     * the command was rejected. If the command was rejected we
     * print it out on the screen. A command rejection points to
     * bad programming and shouldn't happen under normal circumstances.
     * The above comment also applies to the TRANSMIT command.
     */

    /*
     * See if the scb is available.
     */
  again2:
    s = splimp();
    simple_lock(&sc->scb_clear_lock);
    if (sc->scb_clear_flag) {
	/*
	 * Its in use. Block and wait to be woken up.
	 */
	assert_wait(&sc->scb_clear_flag, TRUE);
	thread_set_timeout(hz/2);
	simple_unlock(&sc->scb_clear_lock);
	splx(s);
	thread_block();
	goto again2;
    } 
    /*
     * Its available for use.
     */
    sc->scb_clear_flag = 1;		
    simple_unlock(&sc->scb_clear_lock);
    splx(s);

    /*
     * Load the SCB with the RECEIVE command.
     */
    sc->scb->scb_cmd = htons(MAC_RECEIVE);
    CONVTOPHYS(sc->rcvlist[0].list, &physaddr);
    HOST_TO_TMS(physaddr,
		sc->scb->scb_parm_0,       
		sc->scb->scb_parm_1);

    TRAREGWR(sc->tms_sifcmd_sts,
	     TRAREGRD(sc->tms_sifcmd_sts) |
	     (CMDSTS_M_INTR_ADAPTER |	
	      CMDSTS_M_SYS_INTERRUPT |
	      CMDSTS_M_EXECUTE | 
	      CMDSTS_M_SCB_REQUEST));
    TRAIOSYNC();

    /*
     * Issue the TRANSMIT command. This also needs to be issued only
     * once.
     */

    /*
     * See if the scb is available.
     */
  again3:
    s = splimp();
    simple_lock(&sc->scb_clear_lock);
    if (sc->scb_clear_flag) {
	/*
	 * Its in use. Block and wait to be woken up.
	 */
	assert_wait(&sc->scb_clear_flag, TRUE);
	thread_set_timeout(hz/2);
	simple_unlock(&sc->scb_clear_lock);
	splx(s);
	thread_block();
	goto again3;
    } 

    /*
     * Its available for use.
     */
    sc->scb_clear_flag = 1;		
    simple_unlock(&sc->scb_clear_lock);
    splx(s);

    /*
     * Load the SCB with the TRANSMIT command.
     */
    sc->scb->scb_cmd = htons(MAC_TRANSMIT);
    CONVTOPHYS(sc->xmitlist[0].list, &physaddr);
    HOST_TO_TMS(physaddr,
		sc->scb->scb_parm_0,       
		sc->scb->scb_parm_1);

    TRAREGWR(sc->tms_sifcmd_sts,
	     TRAREGRD(sc->tms_sifcmd_sts) |
	     (CMDSTS_M_INTR_ADAPTER |	
	      CMDSTS_M_SYS_INTERRUPT |
	      CMDSTS_M_EXECUTE |
	      CMDSTS_M_SCB_REQUEST));
    TRAIOSYNC();
    sc->ring_status = MIB1231_RSTATUS_NO_PROB;
    ifp->if_flags |= IFF_RUNNING;
    if (sc->time_init_flag) {
	sc->ztime = time.tv_sec;
	sc->time_init_flag = 0;
    }
    sc->ring_state = MIB1231_RSTATE_OPENED;
    printf("%s%d: Opening the adapter into the ring at %dMb/s\n",
		   ADAP, ifp->if_unit, sc->ring_speed);
    return(1);
}



/*
 * Name:	tra_save_ring_speed(sc, speed_value);
 *
 * Arguments:
 *	sc: The softc structure.
 *	speed_value:   The value to be written out.
 *
 * Functional Description:
 *	 The ring speed is a user programmable value. When the user
 *       changes the value it is written out onto the board. This value,
 *	 by default is set to 16Mb/s.
 *
 * Calls:
 *	None.
 *
 * Return Codes:
 *	Success:	1
 *	Failure:	0
 *
 */
tra_save_ring_speed(sc, speed_value)
struct tra_softc *sc;
u_short speed_value;
{
    u_int *speed;
    u_short stat, current;
    u_short cnt = 0;

    for (speed = sc->speed_location; 
	 speed < (sc->speed_location + sc->speed_size); speed++) {
	if (*(speed + 1) & 0xff == 0xff){
	    current = (*speed & 0xff);
	    if (speed_value == current)
		return(1);
	    reset_rom(sc);
	    /*
	     * Write the the data into the ROM.
	     */
	    TRAREGWR(sc->csr4_intr_status, 0x01);
	    TRAIOSYNC();
	    do {
		u_short rom_data;

		*(speed + 1) = 0x40;		/* write Setup/Program */
		TRAIOSYNC();
		*(speed + 1) = speed_value;     /* write data to location */
		TRAIOSYNC();
		DELAY(2000);
		*speed = 0xc0;			/* write Program/Verify */
		TRAIOSYNC();
		DELAY(2000);
		rom_data = *(speed + 1) & 0xff;
		if(rom_data != speed_value)
		    cnt++;
		else
		    break;
	    } while (cnt < 100);

	    if (cnt == 100) {
		return(0);
	    }
	    TRAREGWR(sc->csr4_intr_status, 0x00);
	    TRAIOSYNC();
	    reset_rom(sc);
	    return(1);
	}
    }
    return(0);
}


/*
 * Name:	reset_rom(sc);
 *
 * Arguments:
 *	sc: 	softc structure for the adpater.
 *
 * Functional Description:
 *	This routine resets the EEPROM. This is used while changing the
 *	"speed_value" stored in the EEPROM.	
 *
 * Calls:
 *	None.
 *
 * Return Codes:
 *	Success:	1
 *	Failure:	0
 *
 */
reset_rom(sc)
struct tra_softc *sc;
{
    /*
     * Reset the ROM.
     */
    TRAREGWR(sc->csr4_intr_status, 0x01);
    TRAIOSYNC();
    TRAREGWR(((u_long *)sc->basereg), 0xff);	/* command reset */
    TRAIOSYNC();
    DELAY(1000);		
    TRAREGWR(((u_long *)sc->basereg), 0xff);
    TRAIOSYNC();
    DELAY(1000);		
    TRAREGWR(((u_long *)sc->basereg), 0x00);	/* commmand read */
    TRAIOSYNC();
    DELAY(1000);		
    TRAREGWR(sc->csr4_intr_status, 0x00);
    TRAIOSYNC();
}


/*
 * Name:	tra_buffers_init(unit)
 *
 * Arguments:
 *	unit:	The unit number of the adapter.
 *
 * Functional Description:
 *	This function initializes the transmit and receive lists
 *	to be used by the driver and adapters.
 *	The lists being used are circular.
 *
 * Calls:
 *	None.
 *
 * Return Codes:
 *	Success:	1
 *	Failure:	0
 *
 */
tra_buffers_init(unit)
int unit;
{
    struct tra_softc *sc = &tra_softc[unit];
    struct ifnet *ifp = &sc->is_if;
    u_short i;
    u_int physaddr, first_xmit_physaddr, first_rcv_physaddr;
    XMIT_PARM_LIST *xmit_parm_list;
    RCV_PARM_LIST *rcv_parm_list;
    u_short count = 0;

    if (!sc->parm_align) {
	/*
	 * Needs to be aligned.
	 */
	xmit_parm_list = (XMIT_PARM_LIST *)(&(tra_xmit_list_pool[unit * (XMITSZ + 1)]));
	rcv_parm_list = (RCV_PARM_LIST *)(&(tra_rcv_list_pool[unit * (RCVSZ + 1)]));
	
	/*
	 * Align the xmit_parm_list to a 16 bit address.
	 */
	CONVTOPHYS(xmit_parm_list, &physaddr);
	while (physaddr & 0x0000000f) {
	    xmit_parm_list = (XMIT_PARM_LIST *)((u_char *)(xmit_parm_list) + 1);
	    CONVTOPHYS(xmit_parm_list, &physaddr);
	}

	/*
	 * Align the rcv_parm_list to an even word boundary address.
	 */
	CONVTOPHYS(rcv_parm_list, &physaddr);
	while (physaddr & 0x0000000f) {
	    rcv_parm_list = (RCV_PARM_LIST *)((u_char *)(rcv_parm_list) + 1);
	    CONVTOPHYS(rcv_parm_list, &physaddr);
	}

	CONVTOPHYS(xmit_parm_list, &first_xmit_physaddr);
	CONVTOPHYS(rcv_parm_list, &first_rcv_physaddr);

	for(i = 0; i < XMITSZ; i++) {
	    sc->xmitlist[i].list = xmit_parm_list;

	    if ((i != (XMITSZ - 1))) {
		xmit_parm_list++;
		CONVTOPHYS(xmit_parm_list, &physaddr);
		sc->xmitlist[i].next_list  = (struct xmit_list *)(&(sc->xmitlist[i + 1]));
		sc->xmitlist[i].next_list->physaddr = physaddr;
	    }
	    HOST_TO_TMS(physaddr,
			sc->xmitlist[i].list->forw_ptr_hi,
			sc->xmitlist[i].list->forw_ptr_lo);
	    sc->xmitlist[i].list->xmit_cstat = 0;
	    sc->xmitlist[i].list->frame_size = 0;
	    sc->xmitlist[i].list->data_count = 0;
	    sc->xmitlist[i].list->data_addr_hi = 0;
	    sc->xmitlist[i].list->data_addr_lo = 0;
	    sc->xmitlist[i].xmit_mbuf  = NULL;
	}

	/*
	 * For the last member of the list point it to the
	 * first one.
	 */
	 HOST_TO_TMS(first_xmit_physaddr,
		     sc->xmitlist[XMITSZ - 1].list->forw_ptr_hi,
		     sc->xmitlist[XMITSZ - 1].list->forw_ptr_lo);
	sc->xmitlist[0].physaddr = first_xmit_physaddr;
	sc->xmitlist[XMITSZ - 1].next_list = (&(sc->xmitlist[0]));

#ifdef TRADEBUG1
	tradebug_dump_xmitlist(sc);
#endif TRADEBUG1
	
	/*
	 * Initialize the receive list.
	 */
	for(i = 0; i < RCVSZ; i++) {
	    sc->rcvlist[i].list = rcv_parm_list;
	    if ((i != (RCVSZ - 1))) {
		rcv_parm_list++;
		CONVTOPHYS(rcv_parm_list, &physaddr);
		sc->rcvlist[i].next_list  = (&(sc->rcvlist[i + 1]));
		sc->rcvlist[i].next_list->physaddr = physaddr;
	    }

	    HOST_TO_TMS(physaddr,
		        sc->rcvlist[i].list->forw_ptr_hi,
		        sc->rcvlist[i].list->forw_ptr_lo);
	    sc->rcvlist[i].list->rcv_cstat = 0;
	    sc->rcvlist[i].list->frame_size = 0;
	    sc->rcvlist[i].list->data_count_0 = 0;
	    sc->rcvlist[i].list->data_addr0_hi = 0;
	    sc->rcvlist[i].list->data_addr0_lo = 0;
	    sc->rcvlist[i].list->data_count_1 = 0;
	    sc->rcvlist[i].list->data_addr1_hi = 0;
	    sc->rcvlist[i].list->data_addr1_lo = 0;
	    sc->rcvlist[i].list->data_count_2 = 0;
	    sc->rcvlist[i].list->data_addr2_hi = 0;
	    sc->rcvlist[i].list->data_addr2_lo = 0;
	    sc->rcvlist[i].rcv_mbuf0 = NULL;
	    sc->rcvlist[i].rcv_mbuf1 = NULL;
	    sc->rcvlist[i].rcv_mbuf2 = NULL;
	    
	}
	
	/*
	 * For the last member of the list point it to the
	 * first one.
	 */
	 HOST_TO_TMS(first_rcv_physaddr,
	             sc->rcvlist[RCVSZ - 1].list->forw_ptr_hi,
		     sc->rcvlist[RCVSZ - 1].list->forw_ptr_lo);
	sc->rcvlist[0].physaddr = first_rcv_physaddr;
	sc->rcvlist[RCVSZ - 1].next_list = (&(sc->rcvlist[0]));
	
	sc->parm_align = 1;
#ifdef TRADEBUG1
	printf("sc->xmit_ptr = 0x%X; sc->xmit_done_ptr = 0x%X; sc->xmitlist = 0x%X\n", sc->xmit_ptr, sc->xmit_done_ptr, sc->xmitlist);
#endif /* TRADEBUG1 */
    } else {
	/*
	 * The circular lists have already been established.
	 * All we need to do is clear out the buffers and zero out the
	 * fields.
	 */
	for(i = 0; i < XMITSZ; i++) {
	    sc->xmitlist[i].list->xmit_cstat = 0;
	    sc->xmitlist[i].list->frame_size = 0;
	    sc->xmitlist[i].list->data_count = 0;
	    sc->xmitlist[i].list->data_addr_hi = 0;
	    sc->xmitlist[i].list->data_addr_lo = 0;
	    sc->xmitlist[i].xmit_mbuf  = NULL;
	}
	
	/*
	 * Initialize the receive list.
	 */
	for(i = 0; i < RCVSZ; i++) {
	    sc->rcvlist[i].list->rcv_cstat = 0;
	    sc->rcvlist[i].list->frame_size = 0;
	    sc->rcvlist[i].list->data_count_0 = 0;
	    sc->rcvlist[i].list->data_addr0_hi = 0;
	    sc->rcvlist[i].list->data_addr0_lo = 0;
	    sc->rcvlist[i].list->data_count_1 = 0;
	    sc->rcvlist[i].list->data_addr1_hi = 0;
	    sc->rcvlist[i].list->data_addr1_lo = 0;
	    sc->rcvlist[i].list->data_count_2 = 0;
	    sc->rcvlist[i].list->data_addr2_hi = 0;
	    sc->rcvlist[i].list->data_addr2_lo = 0;
	    sc->rcvlist[i].rcv_mbuf0 = NULL;
	    sc->rcvlist[i].rcv_mbuf1 = NULL;
	    sc->rcvlist[i].rcv_mbuf2 = NULL;
	}
    }
    /*
     * Initialize the pointers to the first one in the list.
     */
    sc->xmit_ptr = sc->xmit_done_ptr = sc->xmitlist;
    sc->rcv_ptr = sc->rcvlist;
#ifdef TRADEBUG1
tradebug_dump_xmitlist(sc);
#endif /* TRADEBUG1 */
    return(tra_allocate_rcv_buff(ifp));
}

	    

/*
 * Name:	tra_allocate_rcv_buff();
 *
 * Arguments:
 *	ifp:	  	The ifnet pointer.
 *			
 * Functional Description:
 * 	This allocates receive buffers.
 *
 *	Note on setting up of receive buffers:
 *		The data that is passed on to the the higher layers should
 *    	be aligned to a 4 byte boundary. 
 *			--------- 0
 *		       |   AC	|
 *			--------- 1
 *		       |   FC	|
 *			--------- 2
 *		       | Destn.	|
 *			--------- 8
 *		       | Source.|
 *			--------- 14
 *		       |  RIF	|
 *		       | 0-18   |
 *		       | bytes 	|
 *			---------  <------- This should be on a 4 byte
 *		       | Data	|	    boundary
 *		       |	|
 *
 *
 *  	In order to achieve this we use a small mbuf to receive the 
 *	MAC header and a 8K cluster mbuf to receive the data. The data
 *	includes the LLC frame.
 *	
 *
 * Calls:
 *	None.
 * 
 * Return Codes:
 *	Success:	1
 *	Failure:	0
 *
 */
tra_allocate_rcv_buff(ifp)
struct ifnet *ifp;
{
    struct tra_softc *sc = &tra_softc[ifp->if_unit];
    struct mbuf *m0, *m1;
    u_short i;
    u_int physaddr;

    /*
     * Allocate RCV Buffers.
     * If the ring speed is 4Mbs we use 1 buffer per list, if
     * the ring speed is 16Mbs we use 2 buffers per list.
     * NOTE: As per RFC 1042, the largest MAC MTU for 
     *	     a 4 Mbs token ring is 4136 (TRN4_RFC1042_MAC_MTU) bytes and
     *	     for 16 Mbs token ring it is 8232 (TRN16_RFC1042_MAC_MTU) bytes.
     */

    for (i = sc->alloc_index; i < RCVSZ; i++) {
	TRAMCLGET(m0, 1);
	if (m0) { 
	    MGET((m1), M_DONTWAIT, MT_DATA);
	    if (!m1) {
		m_freem(m0);
		break;
	    }
	} else {
	    break;
	}

	/* Get the physical address. */
	CONVTOPHYS((mtod(m0, u_long *)), (&physaddr));

	/*
	 * Use the "converter" to get the data in the right format.
	 */
	HOST_TO_TMS(physaddr,
		    sc->rcvlist[i].list->data_addr0_hi,
		    sc->rcvlist[i].list->data_addr0_lo);
	
	CONVTOPHYS((mtod(m1, u_long *)), (&physaddr));
	HOST_TO_TMS(physaddr,
		    sc->rcvlist[i].list->data_addr1_hi,
		    sc->rcvlist[i].list->data_addr1_lo);
	
	/*
	 * As per TI manual, Page 4-104, we have to initialize the
	 * data count field values to something other than zero, even
	 * if that descriptor is not being used.
	 */
	sc->rcvlist[i].rcv_mbuf0 = m0;
	sc->rcvlist[i].list->data_count_0 = htons(MCLBYTES | 0x8000);
	sc->rcvlist[i].rcv_mbuf1 = m1;
	sc->rcvlist[i].list->data_count_1 = htons(MLEN);
	sc->rcvlist[i].rcv_mbuf2 = NULL; 		/* NOT USED */
	sc->rcvlist[i].list->data_count_2 = htons(1);	/* NOT USED */

	/*
	 * Initialize CSTAT with frame interrupt and valid frame.
	 */
	sc->rcvlist[i].list->rcv_cstat = htons(RCVCSTAT_INITIALIZE);
    }

    if (i < 0)	{	
	printf("%s%d: Unable to get memory for receiving data.\n", 
	       ADAP, ifp->if_unit);    
	return(0);
    }
    
#ifdef TRADEBUG1
    tradebug_dump_rcvlist(sc);
#endif TRADEBUG1

    sc->alloc_index = i;
    return(1);
}


/*
 * Name:	traoutput();
 *
 * Arguments:
 *	ifp:	  	The ifnet pointer.
 *			
 * Functional Description:
 *	This services the transmit queue. The data passed down from the
 * higher layers, in mbufs, is put in the DMA area and the adapter is
 * informed by writing out the TRANSMIT command in the SCB.
 *
 * Calls:
 *	None.
 * 
 * Return Codes:
 *	None.
 *
 */
traoutput(ifp)
struct ifnet *ifp;
{
    struct mbuf *m;
    short unit = ifp->if_unit;
    struct tra_softc *sc = &tra_softc[unit];
    XMIT_PARM_LIST  *xmit_parm, *first_xmit_parm;
    XMIT_LIST *tmp_xmit_ptr, *sav_xmit_ptr;
    XMIT_LIST *first_list = NULL;
    struct mbuf *m0, *mprev;
    u_int physaddr;
    u_short s, j, oversize = 0, totlen;

#ifdef  TRADEBUG1
	if (tradebug > 4)
	printf("traoutput: sc->xmit_ptr = 0x%X; sc->xmit_done_ptr = 0x%X; sc->xmitlist = 0x%X\n", sc->xmit_ptr, sc->xmit_done_ptr, sc->xmitlist);
#endif /* TRADEBUG1 */
    s = splimp();


    xmit_parm = NULL;
    /*
     * Now to produce some buffers to be transmitted.
     * Process the transmit queue.
     * The check to see if this is a list we can use : a) the valid bit
     * is reset to zero and one of the fields (for example, frame size)
     * is not zero. The reason for the second test is to prevent us from
     * overwriting a list we have already filled but have not yet turned on
     * the valid bit. Remember, the valid bit is turned on only after all
     * the packets have been queued and are ready to be transmitted.
     */
     while (sc->xmit_ptr->next_list != sc->xmit_done_ptr) {
	u_short *data_count;

	if (sc->is_if.if_snd.ifq_head) {
	    IF_DEQUEUE(&sc->is_if.if_snd, m0);
	} else { /* There is nothing queued up. 
		  */
	    break;/* From the while loop */
	}
#ifdef TRADEBUG1
	{
	    int i;
	    struct trn_header *eptr;
	    eptr = mtod(m0, struct trn_header *);
	    printf("eptr->trn_ac = 0x%X\n ", eptr->trn_ac);
	    printf("eptr->trn_fc = 0x%X\n ", eptr->trn_fc);
	    for (i = 0; i < 6; i++)
		printf("eptr->trn_dhost = 0x%X: ", eptr->trn_dhost[i] & 0xff);
	    printf("\n");
	    for (i = 0; i < 6; i++)
		printf("eptr->trn_shost = 0x%X: ", eptr->trn_shost[i] & 0xff);

	}
#endif /* TRADEBUG1 */
	/*
	 * Check for packet header.
	 */
	if (!(m0->m_flags & M_PKTHDR)) {
	    /*
	     * drop this packet as it does not have
	     * the packet header.
	     */
	    printf("%s%d: No packet header in mbuf.\n",
		   ADAP, ifp->if_unit);
	    m_freem(m0);
	    continue;
	}
	/*
	 * Check the packet length.
	 * This should not be greater than the MTU size less the
	 * 7 bytes of SDEL(1), CRC(4), FS(1) and EDEL(1) that is attached by 
	 * the adapter.
	 */
	totlen = m0->m_pkthdr.len;
	if (sc->ring_speed == 16) {
	    if (totlen > TRN16_RFC1042_MAC_MTU - 7)
		oversize = 1;
	} else {
	    if (totlen > TRN4_RFC1042_MAC_MTU - 7)
		oversize = 1;
	}

	if(oversize) {
	    /*
	     * drop this packet, the size exceeds the MTU.
	     */
	    printf("%s%d: Output packet size too big: %d\n",
		   ADAP, ifp->if_unit, totlen);
	    m_freem(m0);
	    continue;
	}

	/*
         * All addresses that are passed to the driver are in the 
	 * canonical form. The source and destination address should
	 * be converted to the non-canonical form before transmitting.
	 * Long live TMS380!! :-)
	 */
	{
	    struct trn_header *eptr;
	    eptr = mtod(m0, struct trn_header *);
	    haddr_convert(eptr->trn_dhost);
	    haddr_convert(eptr->trn_shost);
	}
#ifdef TRADEBUG1
	if (tradebug > 3) {
	    struct mbuf *mtst;
	    int i = 0, sum = 0;
	    
	    mtst = m0;
	    while(mtst) {
		i++;
		printf("m_len%d = %d ", i, mtst->m_len);
		sum += mtst->m_len;
		mtst = mtst->m_next;
	    }
	    printf("\nsum of m_len = %d, totlen = %d\n", sum, totlen);
	}

	if (tradebug > 5) {
	    u_char *tmp;
	    u_short i;
	    struct mbuf *mtmp;
	    mtmp = m0;
	    printf("xmit: data xmitted: ");
	    while(mtmp) {
		if (mtmp->m_len == 0) {
		    mtmp = mtmp->m_next;
		    continue;
		}
		tmp = (u_char *)(mtmp->m_data);
		for (i = 0; i < mtmp->m_len; i++, tmp++)
		    printf(" 0x%X: ", *tmp & 0xff);
		mtmp = mtmp->m_next;
	    }
	}
#endif TRADEBUG1

	/*
	 * Strategy for xmting:
	 * 	Once the above tests have passed, we blindly start
	 * 	filling in the transmit descriptors - one descriptor
	 *	per mbuf. If we hit the last descriptor
	 *	and we have more than one mbuf left, we will not
	 *	transmit that packet now. We will instead clean up
	 * 	and pre-pend this back onto the data link queue.
	 *
	 * Even though the frame size is valid only for the first list
	 * the TMS380 wants a non-zero frame size for each
	 * list.
	 */

	first_list =  sc->xmit_ptr;
	/*
	 * Save the current xmit pointer.
	 */
	sav_xmit_ptr = tmp_xmit_ptr = sc->xmit_ptr;

	/* point to the list */
	first_xmit_parm  = tmp_xmit_ptr->list;
	j = 1;
	for (m = m0; m; mprev = m, m = m->m_next) {
	    u_short len, split_mbuf;
	    u_long addr;
	    u_int physaddr;
	    u_int clsize;
	    u_short *data_addr_hi, *data_addr_lo;
	    
	    addr = mtod(m, u_long);
	    len = m->m_len;

	    split_mbuf = 0;

	    while (len > 0) {
		int slen, tmp_len = 0;
		u_short *data_count1;

	        clsize = MCLBYTES - (((u_long) addr) & (MCLBYTES - 1));
		if (j == 1) {
		    xmit_parm = tmp_xmit_ptr->list;
		    data_count1 = (u_short *)(&xmit_parm->data_count);
		}
		    
		if ((tmp_xmit_ptr->next_list->next_list == sc->xmit_done_ptr) && 
		   !split_mbuf) {
		    struct mbuf *mnew, *msav;
		    caddr_t mdata, mdata_new;

		    if (tradebug > 2)
			printf("%s%d: Adapter descriptors may get filled.\n",
			       ADAP, ifp->if_unit);
		    /*
		     * If this packet has more than 3 mbufs or more we will run
		     * out of space. So, we roll the rest of the mbuf
		     * into one buffer and transmit it.
		     * NOTE: This assumes that the packet being transmitted
		     *	     is not greater than 8K. Because the size of
		     *	     the cluster we use is 8K.
		     */
		    msav = m;

		    while(m) {
			tmp_len += m->m_len;
			m = m->m_next;
		    }
		    m = msav;

		    TRAMCLGET(mnew, 0);
		    /*
		     * If no mbuf is  or if the total length will not fit in a
		     * cluster we will try later.
		     */
		    if (mnew == NULL || tmp_len > MCLBYTES) {
			XMIT_LIST  *xmit_p;
			XMIT_PARM_LIST *xmit_parm_p;
			if (mnew == NULL) {
			    if (tradebug > 2)
				printf("%s%d: Could not get mbuf.\n",
				       ADAP, ifp->if_unit);
			    sc->count.no_sysbuffers++;
			}
			/* cleanup and prepend */

			for (xmit_p = sav_xmit_ptr; 
			     xmit_p != tmp_xmit_ptr; 
			     xmit_p = xmit_p->next_list) {

			    xmit_p->xmit_mbuf = NULL;
			    xmit_parm_p  = xmit_p->list;
			    xmit_parm_p->xmit_cstat = 0;
			    xmit_parm_p->frame_size = 0;
			    xmit_parm_p->data_count = 0;
			    xmit_parm_p->data_addr_hi = 0;
			    xmit_parm_p->data_addr_lo = 0;
			    xmit_parm_p->data_count1 = 0;
			    xmit_parm_p->data_addr1_hi = 0;
			    xmit_parm_p->data_addr1_lo = 0;
			    xmit_parm_p->data_count2 = 0;
			    xmit_parm_p->data_addr2_hi = 0;
			    xmit_parm_p->data_addr2_lo = 0;
			}
			IF_PREPEND(&sc->is_if.if_snd, m0);
			goto traoutput_out;
		    }
		    mdata_new = mtod(mnew, caddr_t);
		    len = 0;
		    while(m) {
			mdata = mtod(m, caddr_t);
			bcopy(mdata, mdata_new, m->m_len);
			mdata_new += m->m_len;
			len += m->m_len;
			m = m->m_next;
		    }
		    mnew->m_len = len;
		    mnew->m_next = NULL;
		    /* set up the new values */
		    if (msav != m0) /*link up the chain with the new buffer */
			mprev->m_next = mnew;
		    clsize = MCLBYTES;
		    addr = mtod(mnew, u_long);
		    m = mnew;
		    m_freem(msav);/* free the old buffer */
		}

		slen = min(len, clsize);
		
		if (slen != len)
                    split_mbuf = 1;


		CONVTOPHYS(addr, &physaddr);
		data_count = data_count1 + 3 * (j - 1);
		data_addr_hi = data_count + 1;
		data_addr_lo = data_count + 2;
		HOST_TO_TMS(physaddr,
			    *data_addr_hi,
			    *data_addr_lo);
		*data_count = htons(slen | 0x8000);
		xmit_parm->frame_size = htons(totlen);
		addr += slen;
		len -= slen;
		if ((j == 1) && (xmit_parm == first_xmit_parm)) {
		    xmit_parm->xmit_cstat |=  htons(XMITCSTAT_REQ_START_FRAME |
						    XMITCSTAT_REQ_FRAME_INTR |
						    XMITCSTAT_REQ_PASS_SRC_ADDR);
		    tmp_xmit_ptr->xmit_mbuf = m;
		} else {
		    if (j == 1) {
			xmit_parm->xmit_cstat = 0;
		    	tmp_xmit_ptr->xmit_mbuf = NULL;
		    }
		}


		if (++j == 4) {
		    *data_count &= htons(0x7fff);
		    tmp_xmit_ptr = tmp_xmit_ptr->next_list;
		    j = 1;
		}
	    }
	}
	/*
	 * Mark the last parm. of the packet.
	 */
	xmit_parm->xmit_cstat |=  htons(XMITCSTAT_REQ_END_FRAME);
	/*
	 * Turn off the MSB  bit in the last data count field.
	 */
	*data_count &= htons(0x7fff);
        /*
         * Update the pointer to the next free parameter list we can
         * use for transmitting.
         */
	 if (j != 1) {
	     sc->xmit_ptr = tmp_xmit_ptr->next_list;
	 } else {
	     sc->xmit_ptr = tmp_xmit_ptr;
	 }
       /*
	* Turn on the valid bit.
	*/
	xmit_parm = first_list->list;
	xmit_parm->xmit_cstat |= htons(XMITCSTAT_REQ_VALID);
    } /* while (sc->xmit_ptr->next_list != sc->xmit_done_ptr) */

traoutput_out:

#ifdef TRADEBUG1
    if (tradebug > 6)
	tradebug_dump_xmitlist(sc);
#endif /* TRADEBUG1 */
    /*
     * Tell the adapter to transmit.
     */
    TRAREGWR(sc->tms_sifcmd_sts, 
	     TRAREGRD(sc->tms_sifcmd_sts) |
	     (CMDSTS_M_INTR_ADAPTER |
	      CMDSTS_M_RCV_VALID | 
	      CMDSTS_M_SYS_INTERRUPT |
	      CMDSTS_M_XMIT_VALID));
    TRAIOSYNC();
    /*
     * Tell the upper layer we are transmitting.
     */
    sc->is_if.if_flags |= IFF_OACTIVE;
    ifp->if_timer = 3;
    splx(s);
    return;
}	

#ifdef TRADEBUG1
tradebug_dump_xmitlist(sc)
struct tra_softc *sc;
{
    struct ifnet *ifp = &sc->is_if;
    int i;
    XMIT_PARM_LIST *xmit_parm_list;
    u_int physaddr;

    xmit_parm_list = (XMIT_PARM_LIST *)(&(tra_xmit_list_pool[ifp->if_unit * (XMITSZ + 1)]));
    /*
     * Dump the entire xmit list.
     */

    for (i = 0; i < XMITSZ; i++) {
	printf("&xmit_list[%d] = 0x%X\n", i, &sc->xmitlist[i]);
	printf("xmit_list[%d].list = 0x%X\n", i, sc->xmitlist[i].list);
	printf("xmit_list[%d].physaddr = 0x%X\n", i, sc->xmitlist[i].physaddr);
	printf("xmit_list[%d].next_list = 0x%X\n", i, sc->xmitlist[i].next_list);
	printf("xmit_list[%d].xmit_mbuf = 0x%X\n", i, sc->xmitlist[i].xmit_mbuf);

	printf("-------- xmit_list[%d].list breakdown ---------\n", i);
	CONVTOPHYS(xmit_parm_list, &physaddr);
	printf("xmit_parm_list[%d] = 0x%X, physaddr = 0x%X\n",
		   i, xmit_parm_list, physaddr);
	xmit_parm_list++;
	printf("xmit_list[%d].list->forw_ptr_hi = 0x%X\n", i, sc->xmitlist[i].list->forw_ptr_hi);
	printf("xmit_list[%d].list->forw_ptr_lo = 0x%X\n", i, sc->xmitlist[i].list->forw_ptr_lo);
	printf("xmit_list[%d].list->xmit_cstat = 0x%X\n", i, sc->xmitlist[i].list->xmit_cstat);
	printf("xmit_list[%d].list->frame_size = 0x%X\n", i, sc->xmitlist[i].list->frame_size);
	printf("xmit_list[%d].list->data_count = 0x%X\n", i, sc->xmitlist[i].list->data_count);
	printf("xmit_list[%d].list->data_addr_hi = 0x%X\n", i, sc->xmitlist[i].list->data_addr_hi);
	printf("xmit_list[%d].list->data_addr_lo = 0x%X\n", i, sc->xmitlist[i].list->data_addr_lo);
	printf("xmit_list[%d].list->data_count1 = 0x%X\n", i, sc->xmitlist[i].list->data_count1);
	printf("xmit_list[%d].list->data_addr1_hi = 0x%X\n", i, sc->xmitlist[i].list->data_addr1_hi);
	printf("xmit_list[%d].list->data_addr1_lo = 0x%X\n", i, sc->xmitlist[i].list->data_addr1_lo);
	printf("xmit_list[%d].list->data_count2 = 0x%X\n", i, sc->xmitlist[i].list->data_count2);
	printf("xmit_list[%d].list->data_addr2_hi = 0x%X\n", i, sc->xmitlist[i].list->data_addr2_hi);
	printf("xmit_list[%d].list->data_addr2_lo = 0x%X\n", i, sc->xmitlist[i].list->data_addr2_lo);
	printf("**********************************************\n\n");
    }
	
}

tradebug_dump_rcvlist(sc)
struct tra_softc *sc;
{
    struct ifnet *ifp = &sc->is_if;
    int i;
    RCV_PARM_LIST *rcv_parm_list;
    u_int physaddr;

    rcv_parm_list = (RCV_PARM_LIST *)(&(tra_rcv_list_pool[ifp->if_unit * (RCVSZ + 1)]));
    /*
     * Dump the entire rcv list.
     */

    for (i = 0; i < RCVSZ; i++) {
	printf("&rcv_list[%d] = 0x%X\n", i, &sc->rcvlist[i]);
	printf("rcv_list[%d].list = 0x%X\n", i, sc->rcvlist[i].list);
	printf("rcv_list[%d].physaddr = 0x%X\n", i, sc->rcvlist[i].physaddr);
	printf("rcv_list[%d].next_list = 0x%X\n", i, sc->rcvlist[i].next_list);
	printf("rcv_list[%d].rcv_mbuf0 = 0x%X\n", i, sc->rcvlist[i].rcv_mbuf0);
	printf("rcv_list[%d].rcv_mbuf1 = 0x%X\n", i, sc->rcvlist[i].rcv_mbuf1);
	printf("rcv_list[%d].rcv_mbuf2 = 0x%X\n", i, sc->rcvlist[i].rcv_mbuf2);

	printf("-------- rcv_list[%d].list breakdown ---------\n", i);
	CONVTOPHYS(rcv_parm_list, &physaddr);
	printf("rcv_parm_list[%d] = 0x%X, physaddr = 0x%X\n",
		   i, rcv_parm_list, physaddr);
	rcv_parm_list++;
	printf("rcv_list[%d].list->forw_ptr_hi = 0x%X\n", i, sc->rcvlist[i].list->forw_ptr_hi);
	printf("rcv_list[%d].list->forw_ptr_lo = 0x%X\n", i, sc->rcvlist[i].list->forw_ptr_lo);
	printf("rcv_list[%d].list->rcv_cstat = 0x%X\n", i, sc->rcvlist[i].list->rcv_cstat);
	printf("rcv_list[%d].list->frame_size = 0x%X\n", i, sc->rcvlist[i].list->frame_size);
	printf("rcv_list[%d].list->data_count0 = 0x%X\n", i, sc->rcvlist[i].list->data_count_0);
	printf("rcv_list[%d].list->data_addr0_hi = 0x%X\n", i, sc->rcvlist[i].list->data_addr0_hi);
	printf("rcv_list[%d].list->data_addr0_lo = 0x%X\n", i, sc->rcvlist[i].list->data_addr0_lo);
	printf("rcv_list[%d].list->data_count_1 = 0x%X\n", i, sc->rcvlist[i].list->data_count_1);
	printf("rcv_list[%d].list->data_addr1_hi = 0x%X\n", i, sc->rcvlist[i].list->data_addr1_hi);
	printf("rcv_list[%d].list->data_addr1_lo = 0x%X\n", i, sc->rcvlist[i].list->data_addr1_lo);
	printf("rcv_list[%d].list->data_count_2 = 0x%X\n", i, sc->rcvlist[i].list->data_count_2);
	printf("rcv_list[%d].list->data_addr2_hi = 0x%X\n", i, sc->rcvlist[i].list->data_addr2_hi);
	printf("rcv_list[%d].list->data_addr2_lo = 0x%X\n", i, sc->rcvlist[i].list->data_addr2_lo);

	printf("**********************************************\n\n");
    }
	
}
#endif /* TRADEBUG1 */


/*
 * Name:	trarint()
 *
 * Arguments:
 *	unit:	The unit number of the adpater.
 *
 * Functional Description:
 *	This is responsible for receiving packets and passing it on to
 * the higher layers for processing.
 *
 * On a single interrupt we can get multiple packets(frames). See
 * Page 4-107 and 4-108. We service all the frames.
 *
 *	
 *
 * Calls:
 *	traread()
 *
 * Return Codes:
 *	None.
 *
 *
 */
trarint(unit)
u_short unit;
{
    struct tra_softc *sc =  &tra_softc[unit];
    struct ifnet *ifp = &(sc->is_if);
    u_short rcv_cstat, len, cmd, rcv_status;
    u_short last = 0;
    u_int addr, physaddr;
    struct mbuf *m0, *m1, *mp;

    cmd = ntohs(sc->ssb_copy.ssb_cmd);
    rcv_status = ntohs(sc->ssb_copy.ssb_parm_0);

    if (RCVSSB_RCV_SUSPEND & rcv_status) {
	/*
	 * We don't use the odd forward pointer method. So,
	 * this should not be generated.
	 */
	printf("%s%d: Receive suspended\n", ADAP, ifp->if_unit);
	thread_wakeup((vm_offset_t)&sc->error_recovery_flag);
    }


    TMS_TO_HOST(addr, sc->ssb_copy.ssb_parm_1, sc->ssb_copy.ssb_parm_2);
    rcv_cstat = ntohs(sc->rcv_ptr->list->rcv_cstat);
    /*
     * Each list contains one frame.
     * We start processing from where ever we left of last.
     * This is pointed to by the 'rcv_ptr'.
     * After the adapter has consumed one of the mbufs it will turn off
     * the valid bit in the cstat field.
     */
    while (!(rcv_cstat & RCVCSTAT_COMP_VALID) && 
	   (rcv_cstat & RCVCSTAT_COMP_FRAME_COMPLETE) && !last) {
	if (sc->rcv_ptr->physaddr == addr) {
	    /*
	     * This is the last parameter block.
	     */
	    last = 1;
	}
	/*
	 * Process the packet.
	 * Sanity check first: Check if FRAME_START and FRAME_END are set
	 * to one.
	 */
	if (!((rcv_cstat) & RCVCSTAT_START_END_FRAME)) {
	    printf("%s%d: Receive Parameter Block Is Incorrect. cstat: 0x%x\n",
		   ADAP, ifp->if_unit, rcv_cstat);
	    sc->rcv_ptr->list->rcv_cstat = 0;
	    sc->rcv_ptr->list->rcv_cstat = htons(RCVCSTAT_COMP_VALID);
	    sc->rcv_ptr = sc->rcv_ptr->next_list;
	    rcv_cstat = ntohs(sc->rcv_ptr->list->rcv_cstat);
	    sc->tracount.tra_pduunrecog++;
	    continue;
	}
	
	len = ntohs(sc->rcv_ptr->list->frame_size);

	if (len <= 32) {
	    /*
	     * We have a bogus packet.
	     */
	    sc->count.small_packet++;
	    sc->tracount.tra_pduunrecog++;
	    goto reuse;
	}
		

	/*
	 * NOTE: When we fall short of mbufs we are creating a live-lock
	 *	 situation by reusing the buffer for the next packet. There
	 *	 is no way to let the driver go to sleep for sometime like
	 *	 we do in case of the DEFTA driver which allows the system
	 *	 to recover by making some buffers available.
	 */
	if (len <= MCLBYTES) {
	    TRAMCLGET(m0, 1);

	    if (m0 == NULL) {
		sc->count.no_sysbuffers++;
		goto reuse;
	    }
	    mp = sc->rcv_ptr->rcv_mbuf0;
	    mp->m_len = len;
	    mp->m_next = NULL;

	    traread(sc, sc->rcv_ptr->rcv_mbuf0, len);

	    /* Get the physical address. */
	    CONVTOPHYS((mtod(m0, u_long *)), (&physaddr));
	    /*
	     * Use the "converter" to get the data in the right format.
	     */
	    HOST_TO_TMS(physaddr,
			sc->rcv_ptr->list->data_addr0_hi,
			sc->rcv_ptr->list->data_addr0_lo);

	    sc->rcv_ptr->rcv_mbuf0 = m0;
	    sc->rcv_ptr->list->data_count_0 = htons(MCLBYTES | 0x8000);

	} else {
	    TRAMCLGET(m0, 1);
	    if (m0) { 
		MGET((m1), M_DONTWAIT, MT_DATA);
		if (m1 == NULL) {
		    m_freem(m0);
		    sc->count.no_sysbuffers++;
		    goto reuse;
		}
	    } else {
		sc->count.no_sysbuffers++;
		goto reuse;
	    }

	    mp = sc->rcv_ptr->rcv_mbuf0;
	    mp->m_len = MCLBYTES;
	    mp->m_next = sc->rcv_ptr->rcv_mbuf1;
	    mp = mp->m_next;
	    mp->m_len = len - MCLBYTES;
	    mp->m_next = NULL;
	    
	    traread(sc, sc->rcv_ptr->rcv_mbuf0, len);
	    
	    /* Replace the buffers */
	    /* Get the physical address. */
	    CONVTOPHYS((mtod(m0, u_long *)), (&physaddr));
	    /*
	     * Use the "converter" to get the data in the right format.
	     */
	    HOST_TO_TMS(physaddr,
			sc->rcv_ptr->list->data_addr0_hi,
			sc->rcv_ptr->list->data_addr0_lo);

	    CONVTOPHYS((mtod(m1, u_long *)), (&physaddr));

	    HOST_TO_TMS(physaddr,
			sc->rcv_ptr->list->data_addr1_hi,
			sc->rcv_ptr->list->data_addr1_lo);

	    sc->rcv_ptr->rcv_mbuf0 = m0;
	    sc->rcv_ptr->list->data_count_0 = htons(MCLBYTES | 0x8000);
	    sc->rcv_ptr->rcv_mbuf1 = m1;
	    sc->rcv_ptr->list->data_count_1 = htons(MLEN);
	}
	/*
	 * Initialize CSTAT with frame interrupt and valid frame.
	 */
reuse:	
	sc->rcv_ptr->list->frame_size = 0;
	sc->rcv_ptr->list->rcv_cstat = htons(RCVCSTAT_INITIALIZE);
	sc->rcv_ptr = sc->rcv_ptr->next_list;
	rcv_cstat = ntohs(sc->rcv_ptr->list->rcv_cstat);
#ifdef TRADEBUG1
	if (tradebug > 3) {
	    printf("rcv_cstat = 0x%X\n", rcv_cstat);
	    printf("addr = 0x%X\n", addr);
	    printf("sc->rcv_ptr->physaddr = 0x%X\n", sc->rcv_ptr->physaddr);
	    printf("RCVSZ = %d, sc->alloc_index = %d\n", RCVSZ, sc->alloc_index);
	}
#endif TRADEBUG1
	continue;
    }  /* while (!(rcv_cstat & RCVCSTAT_COMP_VALID)) */

    /*
     * Check if we need to allocate more mbufs.
     */
    if (sc->alloc_index < RCVSZ) {
	tra_allocate_rcv_buff(ifp);
    }
}



/*
 * Name:	traread();
 *
 * Arguments:
 *	sc:	The softc structure.
 *  	m:  The mbuf to pass onto the higher layers.
 *  	len:The length of the received mbuf.
 *
 * Functional Description:
 *	This passes on the received packet, which is in the form of an mbuf
 *	to the higher layers.
 *
 * Calls:
 *	ether_input()
 *
 * Return Codes:
 *
 */
traread(sc, m, len)
struct tra_softc *sc;
struct mbuf *m;
int len;
{
    struct trn_header *eptr;
    struct mbuf *mtmp;
    
    eptr = mtod(m, struct trn_header *);

    /*
     * All addresses that are passed from the adapter are in the
     * non-canonical form. Convert it to the canonical form before
     * passing the addresses to the higher layer.
     */
     haddr_convert(eptr->trn_dhost);
     haddr_convert(eptr->trn_shost);

#ifdef TRADEBUG1
    {
	int i, k;
	u_char *tmp;
	struct mbuf *mtmp;
	
	if (tradebug > 4) {
	    printf("eptr->trn_ac = 0x%X\n ", eptr->trn_ac);
	    printf("eptr->trn_fc = 0x%X\n ", eptr->trn_fc);
	    printf("eptr->trn_dhost: ");
	    for (i = 0; i < 6; i++)
		printf(" 0x%X: ", eptr->trn_dhost[i] & 0xff);
	    printf("\n");
	    printf("eptr->trn_shost: ");
	    for (i = 0; i < 6; i++)
		printf(" 0x%X: ", eptr->trn_shost[i] & 0xff);
	    printf("\n");
	    mtmp = m;
	    printf("m->m_len = %d; len = %d\n", m->m_len, len);
	    while (mtmp) {					
		tmp = (u_char *)(mtmp->m_data);
		for (i = 0, k = 0; i < mtmp->m_len; i++, tmp++, k++) {
		    printf(" 0x%X: ", *tmp & 0xff);
		    if (k == 7) printf(" ");
		    if (k == 15) {
			printf("\n");
			k = 0;
		    }
		}
		mtmp = mtmp->m_next;
	    }
	    printf("\n");
	}
    }
#endif /* TRADEBUG1 */

    /*
     * Subtract length of MAC header from len
     * The RIF field in the header is padded to 18 bytes.
     */
    len -= sizeof (struct trn_header);
    m->m_len -= sizeof (struct trn_header);
    m->m_data += sizeof (struct trn_header);
    m->m_pkthdr.rcvif = &sc->is_if;
    m->m_pkthdr.len = len;
    
    /*
     * Bump up the DECnet counter.
     */
    sc->is_if.if_ipackets++;
    
    sc->tracount.tra_bytercvd += len;

    if ( sc->tracount.tra_pdurcvd != (unsigned) 0xffffffffffffffff)
	sc->tracount.tra_pdurcvd++;
      
    if( eptr->trn_dhost[0] & 0x80 ) {
	sc->tracount.tra_mbytercvd += len;
	if(sc->tracount.tra_mpdurcvd != (unsigned) 0xffffffffffffffff)
	    sc->tracount.tra_mpdurcvd++;
    }
    /*
     * Dispatch this packet.
     */
    ether_input(&(sc->is_if), (struct ether_header *)eptr, m);
}


/*
 * Name:	tratint()
 *
 * Arguments:
 *	unit:	The unit number of the adpater.
 *
 * Functional Description:
 *	This function frees up buffers that have already been transmitted.
 *	We take an interrupt for each packet that has been transmitted.
 *	
 *
 * Calls:
 *	traoutput()
 *
 * Return Codes:
 *	None.
 *
 *
 */
tratint(unit)
u_short unit;
{
    struct tra_softc *sc =  &tra_softc[unit];
    struct ifnet *ifp = &(sc->is_if);
    XMIT_LIST	*sav_xmit_ptr;
    XMIT_PARM_LIST *xmit_parm;
    u_short status_code, s;
    u_int physaddr, cur_physaddr;
    u_short count1 = 0, count2 = 0, xmit_fail = 0;
    struct trn_header *trnh;

    /*
     * Check the transmit status.
     */
#ifdef TRADEBUG1
    if (tradebug > 3)
    printf("tratint: sc->xmit_ptr = 0x%X; sc->xmit_done_ptr = 0x%X; sc->xmitlist = 0x%X\n", sc->xmit_ptr, sc->xmit_done_ptr, sc->xmitlist);
#endif /* TRADEBUG1 */

    status_code = ntohs(sc->ssb_copy.ssb_parm_0);
    TMS_TO_HOST(physaddr, sc->ssb_copy.ssb_parm_1, sc->ssb_copy.ssb_parm_2);

#ifdef TRADEBUG1
    if (tradebug > 3)
    printf("tratint: physaddr = 0x%X, status code = 0x%X\n", physaddr, status_code);
#endif /* TRADEBUG1 */
    if (status_code & XSTAT_FRAME_COMPLETE) {
	do {
	    u_short xmitcstat;

	    cur_physaddr = sc->xmit_done_ptr->physaddr;

	    xmit_parm = sc->xmit_done_ptr->list;
	
	    /*
	     * Check if there were any errors on transmission.
	     */
	    xmitcstat = ntohs(xmit_parm->xmit_cstat);

	    if (!(xmitcstat & XMITCSTAT_COMP_VALID)) {
		/*
		 * This frame has been transmitted.
		 * check for any errors.
		 */
		if (xmitcstat & XMITCSTAT_COMP_XMIT_ERROR)
		    ifp->if_oerrors++;
	    }

	    if (xmitcstat & XMITCSTAT_COMP_FRAME_END) {
		/*
		 * Bump up the packet count. 
		 */
		sc->is_if.if_opackets++;
		sc->tracount.tra_pdusent++;
		sc->tracount.tra_bytesent += ntohs(xmit_parm->frame_size);
	   }
	    /*
	     * Free up the mbuf.
	     */
	    if (sc->xmit_done_ptr->xmit_mbuf) {
		trnh = mtod(sc->xmit_done_ptr->xmit_mbuf, struct trn_header *);
		if (trnh->trn_dhost[0] & 0x80) {
		    sc->tracount.tra_mbytesent += ntohs(xmit_parm->frame_size);
		    sc->tracount.tra_mpdusent++;
		}
		m_freem(sc->xmit_done_ptr->xmit_mbuf);
		sc->xmit_done_ptr->xmit_mbuf = NULL;
	    }
	    xmit_parm->xmit_cstat = 0;
	    xmit_parm->frame_size = 0;
	    xmit_parm->data_count = 0;
	    xmit_parm->data_addr_hi = 0;
	    xmit_parm->data_addr_lo = 0;
	    xmit_parm->data_count1 = 0;
	    xmit_parm->data_addr1_hi = 0;
	    xmit_parm->data_addr1_lo = 0;
	    xmit_parm->data_count2 = 0;
	    xmit_parm->data_addr2_hi = 0;
	    xmit_parm->data_addr2_lo = 0;
	    sc->xmit_done_ptr = sc->xmit_done_ptr->next_list;
	} while ((cur_physaddr != physaddr) &&
		 (sc->xmit_done_ptr != sc->xmit_ptr)); 
    }


    if (status_code & XSTAT_COMMAND_COMPLETE) {
	printf("%s%d: Transmit suspended\n",
	       ADAP, ifp->if_unit);
    }

    if (status_code & XSTAT_ACCESS_PRI_ERROR) {
	sc->count.access_pri_error++;
	printf("%s%d: Access Priority Error In Transmit.\n",
	       ADAP, ifp->if_unit);
	xmit_fail = 1;
    }

    if (status_code & XSTAT_UNENBL_MAC_FRAME) {
	sc->count.unenabled_mac_frame++;
	printf("%s%d: Unenabled MAC Frame In Transmit.\n",
	       ADAP, ifp->if_unit);
	xmit_fail = 1;
    }

    if (status_code & XSTAT_ILL_FRAME_FORMAT) {
	sc->count.illegal_frame_format++;
	printf("%s%d: Illegal Frame In Transmit.\n",
	       ADAP, ifp->if_unit);
	xmit_fail = 1;
    }

    if (xmit_fail) 
    	sc->count.xmit_fail++;

    /*
     * If any of the following happen we will reset the adapter
     * and start over. These should not happen under normal operation.
     */
    if (status_code & XSTAT_LIST_ERROR) {
	sc->count.xmit_fail++;
	printf("%s%d: List Error in transmit. Status code = 0x%X.\n",
	       ADAP, ifp->if_unit, status_code);
	printf("%s%d: Frame address = 0x%X.\n", 
	       ADAP, ifp->if_unit, physaddr);
#ifdef TRADEBUG1
	tradebug_dump_xmitlist(sc);
#endif /* TRADEBUG1 */

	if (status_code & XSTAT_FRAME_SIZE_ERROR) {
	    printf("%s%d: Frame Size Error In Transmit.\n",
		   ADAP, ifp->if_unit);
	}

	if (status_code & XSTAT_XMIT_THRESHOLD) {
	    printf("%s%d: Transmit Threshold Error.\n",
		   ADAP, ifp->if_unit);
	}

	if (status_code & XSTAT_FRAME_ERROR) {
	    printf("%s%d: Frame Error in transmit.\n",
		   ADAP, ifp->if_unit);
	}

	if (status_code & XSTAT_ODD_ADDRESS) {
	    printf("%s%d: Error in transmit - Odd Address.\n",
		   ADAP, ifp->if_unit);
	}
	thread_wakeup((vm_offset_t)&sc->error_recovery_flag);
    }
    /*
     * Check if there is anything to transmit.
     */
    if (sc->is_if.if_snd.ifq_head) {
	if(tradebug > 3)
	    printf("tratint: More on the queue\n");
	traoutput(ifp);
    } else if (sc->xmit_done_ptr == sc->xmit_ptr) {
        ifp->if_timer = 0;
	sc->is_if.if_flags &= ~IFF_OACTIVE;
    }
}


/*
 * Name:	traxmit_watchdog()
 *
 * Arguments:
 *	unit:	The unit number of the adpater.
 *
 * Functional Description:
 *	If we don't get a transmit interrupt in a specific time interval
 *	this routine is invoked. Sometimes the clear SSB interrupt doesn't
 *	get to the board. This causes the xmit process to lock up the
 *	entire driver. This is rectified by issuing another SSB clear.
 *	If the cause for this time out is something else, the board is
 *	reset.
 *
 * Calls:
 *
 * Return Codes:
 *	None.
 *
 *
 */
traxmit_watchdog(unit)
u_short unit;
{
    struct tra_softc *sc = &tra_softc[unit];
    u_short sifcmdsts;
    u_short x;
#ifdef TRADEBUG1
    printf("watchdog: Xmit time out.\n");
    printf("watchdog: sc->xmit_ptr = 0x%X; sc->xmit_done_ptr = 0x%X; sc->xmitlist = 0x%X\n", sc->xmit_ptr, sc->xmit_done_ptr, sc->xmitlist);
    printf("watchdog: sc->rcv_ptr = 0x%X;\n", sc->rcv_ptr);
    printf("sc->tms_sifcmd_sts = 0x%X\n", TRAREGRD(sc->tms_sifcmd_sts));
    printf("watchdog: sc->ssb_copy.ssb_parm_0 = 0x%X; sc->ssb_copy.ssb_parm_1 = 0x%X; sc->ssb_copy.ssb_parm_2 = 0x%X\n", ntohs(sc->ssb_copy.ssb_parm_0), ntohs(sc->ssb_copy.ssb_parm_1), ntohs(sc->ssb_copy.ssb_parm_2));
    printf("watchdog: sc->ssb->ssb_parm_0 = 0x%X; sc->ssb->ssb_parm_1 = 0x%X; sc->ssb->ssb_parm_2 = 0x%X\n", ntohs(sc->ssb->ssb_parm_0), ntohs(sc->ssb->ssb_parm_1), ntohs(sc->ssb->ssb_parm_2));
#endif /* TRADEBUG1 */
    printf("%s%d: Transmit timeout: Resetting the adapter...\n", 
	    ADAP, unit);
    thread_wakeup((vm_offset_t)&sc->error_recovery_flag);
}
		   

/*
 * Name:	traintr();
 *
 * Arguments:
 *	unit:	  	The unit number the interrupt was received on.
 *			
 * Functional Description:
 *	This is the interrupt service routine for the driver. All the
 *	interrupts from the adapter are vectored into this routine.
 *
 * Calls:
 *	None.
 * 
 * Return Codes:
 *	None.
 *
 */
traintr(unit)
int unit;
{
    struct tra_softc *sc = &tra_softc[unit];
    struct ifnet *ifp = &sc->is_if;
    u_short	s, intr_status, cmdsts, commmand, copy;
  
    s = splimp();

    /*
     * Make a copy of the SSB.
     */
    sc->ssb_copy.ssb_cmd = sc->ssb->ssb_cmd;
    sc->ssb_copy.ssb_parm_0 = sc->ssb->ssb_parm_0;
    sc->ssb_copy.ssb_parm_1 = sc->ssb->ssb_parm_1;
    sc->ssb_copy.ssb_parm_2 = sc->ssb->ssb_parm_2;

    /*
     * Read the status register.
     */
    intr_status = TRAREGRD(sc->tms_sifcmd_sts);

    /*
     * Clear the SYSTEM_INTERRUPT bit of the SIFCMD/SIFSTS register.
     */
    TRAREGWR(sc->tms_sifcmd_sts, 0x0000);
    TRAIOSYNC();

    /*
     * We are now done with the SSB.
     */
    TRAREGWR(sc->tms_sifcmd_sts, 
	     (CMDSTS_M_INTR_ADAPTER |
	      CMDSTS_M_SYS_INTERRUPT |
	      CMDSTS_M_SSB_CLEAR));
    TRAIOSYNC();

    /*
     * First check if the Error bit is set. This bit gets set if BUD
     * detected an error or there was an error during the initialization
     * process. If this bit is set the error condition is specified in 
     * bits 12-15. This is a fatal error and means something is wrong with
     * the adapter. See page 4-40.
     */

    if (CMDSTS_M_ERROR & intr_status) {
	if (CMDSTS_M_TEST & intr_status) 
		printf("%s%d: Diagnostics Error.\n",
		       ADAP, ifp->if_unit);
	if (CMDSTS_M_INITIALIZE & intr_status)
		printf("%s%d: Initialization Error.\n",
		       ADAP, ifp->if_unit);

	switch (intr_status & CMDSTS_M_ERROR_REASON) {
	  case  BUD_INITIAL_TEST_ERROR:
	    printf("%s%d: BUD Initial Test Error.\n",
		   ADAP, ifp->if_unit);
	    break;

	  case  BUD_ADAP_SOFT_CKSUM_ERROR:
	    printf("%s%d: BUD Adapter Software Checksum Error.\n",
		   ADAP, ifp->if_unit);
	    break;

	  case  BUD_ADAP_RAM_ERROR:
	    printf("%s%d: BUD Adapter RAM Error.\n",
		   ADAP, ifp->if_unit);
	    break;

	  case  BUD_ADAP_INSTR_ERROR:
	    printf("%s%d: BUD Intruction Test Error.\n",
		   ADAP, ifp->if_unit);
	    break;

	  case  BUD_CTXT_INTR_TEST_ERROR:
	    printf("%s%d: BUD Context/Interrupt Test Error.\n",
		   ADAP, ifp->if_unit);
	    break;

	  case  BUD_PH_HARDWARE_ERROR:
	    printf("%s%d: BUD Protocol Handler Hardware Error.\n",
		   ADAP, ifp->if_unit);
	    break;

	  case  BUD_SYS_INTF_REG_ERROR:
	    printf("%s%d: BUD System Interface Register Error.\n",
		   ADAP, ifp->if_unit);
	    break;
	}

	ifp->if_flags &= ~IFF_RUNNING;
	ifp->if_flags &= ~IFF_UP;
	/*
	 * Turn of the LEDs.
	 */
     	TRAREGWR(sc->csr0_leds, 0x0000);
	TRAIOSYNC();
	thread_wakeup((vm_offset_t)&sc->error_recovery_flag);
    } else if (intr_status & CMDSTS_M_SYS_INTERRUPT) {
	/*
	 * Now check for the regular interrupts.
	 */
	switch(intr_status & CMDSTS_M_INTR_TYPE_MASK) {	   
	/*
	 * ADAPTER.CHECK interrupt.
	 */
	  case CMDSTS_INTR_ADAPTER_CHECK:
	    {
		ADAP_CHECK_BLOCK *ac;
		u_short	acb_status, acb_p0, acb_p1, acb_p2;

		printf("%s%d: Adapter check interrupt: ",
		       ADAP, ifp->if_unit);

		TRAREGWR(sc->tms_sifadx, 0x0001);
		TRAREGWR(sc->tms_sifadr1, 0x05e0);
		TRAIOSYNC();

		acb_status = TRAREGRD(sc->tms_sifdat_inc);
		acb_p0 = TRAREGRD(sc->tms_sifdat_inc);
		acb_p1 = TRAREGRD(sc->tms_sifdat_inc);
		acb_p2 = TRAREGRD(sc->tms_sifdat_inc);
		
		if (acb_status & ADCHECK_DIO_PARITY) {
		    printf("DIO Parity Error.\n");
		    sc->count.dio_parity++;
		}
		
		if (acb_status & ADCHECK_DMA_RD_ABORT) {
		    printf("DMA Read Abort - ");
		    if (acb_p0 == 0) {
			printf("Timeout Abort.\n");
			sc->count.read_timeout_abort++;
		    }

		    if (acb_p0 == 1) {
			printf("Parity Error Abort.\n");
			sc->count.read_parity_error_abort++;
		    }
		    if (acb_p0 == 2) {
			printf("Bus Error Abort.\n");
			sc->count.read_bus_error_abort++;
		    }

		    printf("\tHost system address: p1:0x%X p2:0x%X\n",
			   acb_p1, acb_p2);
		}
		
		if (acb_status & ADCHECK_DMA_WR_ABORT ) {
		    printf("DMA Write Error - ");

		    if (acb_p0 == 0) {
			printf("Timeout Abort.\n");
			sc->count.write_timeout_abort++;
		    }
		    if (acb_p0 == 1) {
			printf("Parity Error Abort.\n");
			sc->count.write_parity_error_abort++;
		    }

		    if (acb_p0 == 2) {
			printf("Bus Error Abort.\n");
			sc->count.write_parity_error_abort++;
		    }
		    printf("\tHost system address: p1:0x%X p2:0x%X\n",
			   acb_p1, acb_p2);
		}
		
		if (acb_status & ADCHECK_ILL_OP_CODE) {
		    printf("Illegal Opcode.\n");
		    printf("\tCP registers -  R13:0x%X R14:0x%X R15:0x%X\n",
			   acb_p0, acb_p1, acb_p2);
		    sc->count.illegal_opcode++;
		}
		
		if (acb_status & ADCHECK_PARITY_ERRORS) {
		    printf("Parity Errors.\n");
		    printf("\tCP registers -  R13:0x%X R14:0x%X R15:0x%X\n",
			   acb_p0, acb_p1, acb_p2);
		    sc->count.parity_error++;
		}
		
		if (acb_status & ADCHECK_RAM_DATA_ERROR) {
		    printf("RAM Data Error.\n");
		    printf("\tAddress -  MSB:0x%X LSB:0x%X\n",
			   acb_p0, acb_p1);
		    sc->count.ram_data_error++;
		}

		
		if (acb_status & ADCHECK_RAM_PRT_ERROR) {
		    printf("RAM Parity Error.\n");
		    printf("\tAddress -  MSB:0x%X LSB:0x%X\n",
			   acb_p0, acb_p1);
		    sc->count.ram_parity_error++;
		}
		
		if (acb_status & ADCHECK_RING_UNDERRUN) {
		    printf("Ring Underrun.\n");
		    printf("\tCP registers -  R13:0x%X R14:0x%X R15:0x%X\n",
			   acb_p0, acb_p1, acb_p2);
		    sc->count.ring_underrun++;
		}
		
		if (acb_status & ADCHECK_INV_INTR) {
		    printf("Invalid Internal Interrupt.\n");
		    printf("\tCP registers -  R13:0x%X R14:0x%X R15:0x%X\n",
			   acb_p0, acb_p1, acb_p2);
		    sc->count.invalid_internal_interrupt++;
		}
		
		if (acb_status & ADCHECK_INV_ERROR_INTR) {
		    printf("Invalid Error Interrupt.\n");
		    printf("\tCP registers -  R13:0x%X R14:0x%X R15:0x%X\n",
			   acb_p0, acb_p1, acb_p2);
		    sc->count.invalid_error_interrupt++;
		}
		
		if (acb_status & ADCHECK_INV_XOP) {
		    printf("Invalid XOP Request.\n");
		    sc->count.invalid_xop_request++;
		}
		/*
		 * We have a serious problem
		 * The adapter is taken of line, so might as
		 * well take the driver of line also.
		 */
		ifp->if_flags &= ~IFF_RUNNING;
		ifp->if_flags &= ~IFF_UP;
		/*
		 * Turn off the LEDs.
		 */
		TRAREGWR(sc->csr0_leds, 0x0000);
		TRAIOSYNC();
		/* Try to reset the adapter a couple of times */
		if (++(sc->count.fatal_error) < 3) {
		    thread_wakeup((vm_offset_t)&sc->error_recovery_flag);
		}
		break;
	    }

	    /*
	     * RING.STATUS Interrupt.
	     */
	  case CMDSTS_INTR_RING_STATUS:
	    {
		u_short ssb_p0;
		
		sc->ring_status = 0;
		ssb_p0 = ntohs(sc->ssb_copy.ssb_parm_0);

		if (ssb_p0 & TRA_RSTAT_SIGNAL_LOSS) {
		    printf("%s%d: Ring status: ", ADAP, ifp->if_unit);
		    printf("Signal loss detected on the fing.\n");
		    sc->ring_status += MIB1231_RSTATUS_SIGNAL_LOSS;
		    sc->count.signal_loss++;
		}
		    
		if (ssb_p0 & TRA_RSTAT_HARD_ERROR) {
		    printf("%s%d: Ring status: ", ADAP, ifp->if_unit);
		    printf("Transmitting/Receiving beacon frames.\n");
		    sc->ring_status +=  MIB1231_RSTATUS_HARD_ERROR;
		    sc->count.hard_error++;

		}

		if (ssb_p0 & TRA_RSTAT_SOFT_ERROR) {
		    printf("%s%d: Ring status: ", ADAP, ifp->if_unit);
		    printf("Transmitted report error MAC frame.\n");
		    sc->ring_status +=  MIB1231_RSTATUS_SOFT_ERROR;
		    sc->count.soft_error++;
		}

		if (ssb_p0 & TRA_RSTAT_XMIT_BEACON) {
		    printf("%s%d: Ring status: ", ADAP, ifp->if_unit);
		    printf("Transmitting beacon frames.\n");
		    sc->count.xmit_beacon++;
		}

		if (ssb_p0 & TRA_RSTAT_LOBE_WIRE_FAULT) {
		    printf("%s%d: Ring status: ", ADAP, ifp->if_unit);
		    printf("Lobe wire fault.\n");
		    sc->ring_status +=  MIB1231_RSTATUS_LOBE_WIRE_FAULT;
		    sc->count.lobe_wire_fault++;
		}

		if (ssb_p0 & TRA_RSTAT_AUTO_REM_ERROR) {
		    printf("%s%d: Ring status: ", ADAP, ifp->if_unit);
		    printf("Auto removal error.\n");
		    sc->ring_status += MIB1231_RSTATUS_AUTO_REM_ERROR;
		    sc->count.auto_rem_error++;
		}

		if (ssb_p0 & TRA_RSTAT_REMOVE_RCVD) {
		    printf("%s%d: Ring status: ", ADAP, ifp->if_unit);
		    printf("Remove ring station MAC request received.\n");
		    sc->ring_status +=  MIB1231_RSTATUS_REMOVE_RCVD;
		    sc->count.remove_received++;
		}

		if (ssb_p0 & TRA_RSTAT_COUNT_OVERFLOW) {
		    printf("%s%d: Ring status: ", ADAP, ifp->if_unit);
		    printf("Adapter Error Counter Overflow.\n");
		    sc->count.counter_overflow++;
		    thread_wakeup((vm_offset_t)&sc->error_log_read_flag);
		}

		if (ssb_p0 & TRA_RSTAT_SINGLE_STATION) {
		    printf("%s%d: Ring status: ", ADAP, ifp->if_unit);
		    printf("Single station on ring.\n");
		    sc->ring_status += MIB1231_RSTATUS_SINGLE_STATION;;
		    sc->count.single_station++;
		}

		if (ssb_p0 & TRA_RSTAT_RING_RECOVERY) {
		    printf("%s%d: Ring status: ", ADAP, ifp->if_unit);
		    printf("Ring recovery.\n");
		    sc->ring_status += MIB1231_RSTATUS_RING_RECOVERY;
		    sc->count.ring_recovery++;
		}


		if (ssb_p0 & (TRA_RSTAT_AUTO_REM_ERROR |
			      TRA_RSTAT_REMOVE_RCVD |
			      TRA_RSTAT_LOBE_WIRE_FAULT)) {
		    sc->ring_state = MIB1231_RSTATE_CLOSED;
		    /*
		     * We have a serious problem
		     * The adapter is taken of line, so might as
		     * well take the driver of line also.
		     */
		    ifp->if_flags &= ~IFF_RUNNING;
		    ifp->if_flags &= ~IFF_UP;
		    /*
		     * Turn off the LEDs.
		     */
		    TRAREGWR(sc->csr0_leds, 0x0000);
		    TRAIOSYNC();
		    printf("%s%d: Resetting adapter.\n", ADAP, ifp->if_unit);
		    thread_wakeup((vm_offset_t)&sc->error_recovery_flag);
		}	
		break;
	    }

	  /*
	   * SCB.CLEAR. This doesn't use the SSB. It only indicates
	   * that the SCB is ready to be used again.
	   */

	  case CMDSTS_INTR_SCB_CLEAR:
	    {
		/*
		 * Wake up a thread waiting for using the
		 * SCB.
		 */
		simple_lock(&sc->scb_clear_lock);
		sc->scb_clear_flag = 0;
		simple_unlock(&sc->scb_clear_lock);
		thread_wakeup_one(&sc->scb_clear_flag);
		break;
	    }
	    
	    /*
	     * SCB.RECEIVE.STATUS interrupt. 
	     */
	  case CMDSTS_INTR_RECV_STATUS:
	    {
#ifdef TRADEBUG1
		tradebug_dump_rcvlist(sc);
#endif TRADEBUG1
		trarint(unit);

		TRAREGWR(sc->tms_sifcmd_sts, 
			 TRAREGRD(sc->tms_sifcmd_sts) |
			 (CMDSTS_M_INTR_ADAPTER |
			  CMDSTS_M_SYS_INTERRUPT |
			  CMDSTS_M_RCV_CONTINUE |
			  CMDSTS_M_RCV_VALID));
			  
		TRAIOSYNC();	    
#ifdef TRADEBUG1
		printf("*******AFTER SERVICING RCV Q*********\n");
		tradebug_dump_rcvlist(sc);
#endif TRADEBUG1
		break;
	    }

	    /*
	     * SCB.TRANSMIT interrupt. Generated after a frame(s)
	     * has been transmitted.
	     */
	  case CMDSTS_INTR_XMIT_STATUS:
	    {
		tratint(unit);
		break;
	    }

	    /*
	     * COMMAND.STATUS Interrupt for commands other
	     * than TRANSMIT or RECEIVE. This also includes
	     * COMMAND_REJECT. See page 4-36 of the manual.
	     */
	  case CMDSTS_INTR_COMMAND_STATUS:
	    {
		u_short command;
		/*
		 * If this is a command reject, read the
		 * command from parm_1 of the SSB.
		 * Wake up the thread that is waiting for this.
		 */
		if ((ntohs(sc->ssb_copy.ssb_cmd) == COMMAND_REJECT)) {
		    command = ntohs(sc->ssb_copy.ssb_parm_1);
		} else {
		    command = ntohs(sc->ssb_copy.ssb_cmd);
		}

		if (command < 3 || command > 18) {
		    printf("%s%d: Response For An Unknown Command:  0x%X\n",
			   ADAP, ifp->if_unit, command);
		    goto out;
		} 

		if (command == MAC_TRANSMIT) {
		    printf("%s%d: Command Reject For MAC.TRANSMIT.\n",
			   ADAP, ifp->if_unit);
		    goto out;
		}

		if (command == MAC_RECEIVE) {
		    printf("%s%d: Command Reject For MAC.RECEIVE.\n",
			   ADAP, ifp->if_unit);
		    goto out;
		}
		/*
		 * Wake up the process waiting for this completion.
		 * Before waking up we update the ssb with the info
		 * we have.
		 * These could be any of the following:
		 * OPEN, CLOSE, READ.ERROR.LOG, READ.ADAPTER and
		 * MODIFY.OPEN.PARAMETERS.
		 * We currently don't use any of the others.
		 */
		if (sc->ssb_q[command].ssb == NULL) {
		    /*
		     * There is no one queued up for this response.
		     */
		    printf("%s%d: No command for the response. Command: 0x%X\n",
			   ADAP, ifp->if_unit, command);
		} else {
		    sc->ssb_q[command].ssb->ssb_cmd = sc->ssb_copy.ssb_cmd;
		    sc->ssb_q[command].ssb->ssb_parm_0 = sc->ssb_copy.ssb_parm_0;
		    sc->ssb_q[command].ssb->ssb_parm_1 = sc->ssb_copy.ssb_parm_1;
		    sc->ssb_q[command].ssb->ssb_parm_2 = sc->ssb_copy.ssb_parm_2;
		    thread_wakeup_one(sc->ssb_q[command].ssb);
		}
	   
	      out:
		break;
	    }

	  default:
	    printf("%s%d: Driver received an unknown interrupt\n", ADAP, ifp->if_unit);
	    printf("\tintr_status: 0x%x; ssb: 0x%x 0x%x 0x%x 0x%x\n",
		   intr_status, sc->ssb_copy.ssb_cmd, sc->ssb_copy.ssb_parm_0,
		   sc->ssb_copy.ssb_parm_1, sc->ssb_copy.ssb_parm_2);
	    break;	
	}
    }

    splx(s);
}

/*
 * Name:	tra_read_error_log_thread()
 *
 * Arguments:
 *	None.
 *
 * Functional Description:
 *	This is a thread that gets woken up if the error log has to be
 *	read. This is needed if we have a counter overflow and the
 *	errorlog needs to be read. 
 *	The counter overflow is indicated when the SCB is in use. We
 *	need the SCB to issue the error log read command. Waking up a 
 *	thread to do this at later time to do this avoids any deadlock.
 *
 * Calls:
 *	None.
 *
 * Return Codes:
 *	None.	
 */
tra_read_error_log_thread()
{
    thread_t thread;
    struct tra_softc *sc;
    struct ifnet *ifp;


    thread = current_thread();

    /*
     * Collect the argument left by the kernel_thread_w_arg().
     */
    sc = (struct tra_softc *)thread->reply_port;
    ifp = &sc->is_if;
    thread->reply_port = PORT_NULL;

    for(;;) {
	assert_wait((vm_offset_t)&sc->error_log_read_flag, TRUE);
	thread_block();
	tra_read_error_log(sc);
    }
}

/*
 * Name:	tra_read_error_log()
 *
 * Arguments:
 *	sc:	The softc structure.
 *
 * Functional Description:
 *	This reads the error log.
 *
 * Calls:
 *	None.
 *
 * Return Codes:
 *	None.	
 */
tra_read_error_log(sc)
struct tra_softc *sc;
{
    struct ifnet *ifp = &sc->is_if;
    u_short s;

    if (!(ifp->if_flags & IFF_UP))
	return;

    /*
     * Check if there is already an READ_ERROR_LOG. If so
     * don't issue another command.
     */
    if (sc->ssb_q[MAC_READ_ERROR_LOG].ssb == NULL) {
	u_int physaddr;
	SYSTEM_STATUS_BLOCK *l_ssb;
 again4:
        s = splimp();
	simple_lock(&sc->scb_clear_lock);
	if (sc->scb_clear_flag) {
	    /*
	     * Its in use. Block and wait to be woken up.
	     */
	    assert_wait(&sc->scb_clear_flag, TRUE);
	    thread_set_timeout(hz * 10);
	    simple_unlock(&sc->scb_clear_lock);
	    splx(s);
	    thread_block();
	    goto again4;
	} 

	/*
	 * Its available for use.
	 */
	sc->scb_clear_flag = 1;		
	simple_unlock(&sc->scb_clear_lock);
	l_ssb = (SYSTEM_STATUS_BLOCK *)kmem_alloc(kernel_map, 
						  sizeof(SYSTEM_STATUS_BLOCK));
	sc->ssb_q[MAC_READ_ERROR_LOG].ssb = l_ssb;
	/*
	 * Load the SCB with the MAC_READ_ERROR_LOG command.
	 * Issue the open command and block till we get woken
	 * up either by a timeout or by the completion of the
	 * command.
	 */
	sc->scb->scb_cmd = htons(MAC_READ_ERROR_LOG);
	
	CONVTOPHYS(&error_log, &physaddr);
	HOST_TO_TMS(physaddr,
		    sc->scb->scb_parm_0,       
		    sc->scb->scb_parm_1);
	
	TRAREGWR(sc->tms_sifcmd_sts,
		 TRAREGRD(sc->tms_sifcmd_sts) |
		 (CMDSTS_M_INTR_ADAPTER |	
		  CMDSTS_M_SYS_INTERRUPT |
		  CMDSTS_M_EXECUTE | 
		  CMDSTS_M_SCB_REQUEST));
	TRAIOSYNC();	
	assert_wait(l_ssb, TRUE);
	/*
	 * Wait some seconds.
	 */
	thread_set_timeout(hz * 25); /* 25 seconds time out */
	thread_block();
	if (current_thread()->wait_result == THREAD_TIMED_OUT) {
	    /*
	     * We have a serious problem.
	     */
	    printf("%s%d: Time out occurred during MAC_READ_ERROR_LOG command\n", ADAP, ifp->if_unit);
	} else {
	    /*
	     * we got back a response for READ_ERROR_LOG
	     */
	    u_short count1;
	    u_short count2;
	    /*
	     * Read the counters.
	     */
	    count1 = ntohs(error_log.line_error);
	    count1 = count1 >> 8;
	    sc->error_cnts.line_error += count1;
		
	    count1 = ntohs(error_log.burst_arifci_error);
	    count2 = count1 >> 8;
	    count1 &= 0x00ff;
	    sc->error_cnts.burst_error += count2;
	    sc->error_cnts.ari_fci_error += count1;
	    
	    count1 = ntohs(error_log.lostframe_rcvcong_error);
	    count2 = count1 >> 8;
	    count1 &= 0x00ff;
	    sc->error_cnts.lost_frame_error += count2;
	    sc->error_cnts.rcv_congestion_error += count1;
	    
	    count1 = ntohs(error_log.frame_copied_error);
	    count1 = count1 >> 8;
	    sc->error_cnts.frame_copied_error += count1;
	    
	    count1 = ntohs(error_log.token_error);
	    count1 = count1 >> 8;
	    sc->error_cnts.token_error += count1;
	    
	    count1 = ntohs(error_log.dma_bus_parity_error);
	    count2 = count1 >> 8;
	    count1 &= 0x00ff;
	    sc->error_cnts.dma_bus_error += count2;
	    sc->error_cnts.dma_parity_error += count1;
	}
	kmem_free(kernel_map,
		  l_ssb,
		  sizeof(SYSTEM_STATUS_BLOCK));
	sc->ssb_q[MAC_READ_ERROR_LOG].ssb = NULL;
        splx(s);
    } else {
	printf("%s%d: READ_ERROR_LOG command pending\n", 
	       ADAP, ifp->if_unit);
    }
}	


/*
 * Name:	traioctl()
 *
 * Arguments:
 *	ifp:	Pointer to the ifnet structure.
 *	cmd:	The command to execute
 *	data:	The data associated with the command.
 *
 * Functional Description:
 * 	This routine determines the type of request and executes it, returning
 *	data, if any, in the structure provided.
 *
 * Calls:
 *	
 *
 * Return Codes:
 *	
 *
 */
traioctl(ifp, cmd, data)
struct ifnet *ifp;
unsigned int cmd;
caddr_t data;
{
    struct tra_softc *sc = &tra_softc[ifp->if_unit];
    struct ifreq *ifr = (struct ifreq *)data;
    struct ifdevea *ifd = (struct ifdevea *)data;
    struct ifaddr *ifa = (struct ifaddr *)data;
    struct ctrreq *trnctr = (struct ctrreq *)data;

    switch (cmd) {
      case SIOCENABLBACK:
	if (tradebug > 1)
		printf ("SIOCENABLBACK\n");
	ifp->if_flags &= ~IFF_UP;
	/* 
	 * Turn the loopback flag on so that in trainit() the right
	 * options get loaded.
	 */
	ifp->if_flags |= IFF_LOOPBACK;
	if(!trareset(sc))
	    return(EIO);
	break;

      case SIOCDISABLBACK:
	if (tradebug > 1)
		printf ("SIOCDISABLBACK\n");
	ifp->if_flags &= ~IFF_UP;
	/* 
	 * Turn off the loopback flag so that in trainit() the right
	 * options get loaded.
	 */
	ifp->if_flags &= ~IFF_LOOPBACK;
	if(!trareset(sc))
	    return(EIO);
	break;

      case SIOCRPHYSADDR:
	if (tradebug > 1)
		printf ("SIOCRPHYSADDR\n");
	  /*
	   * Read the default hardware address.
	   */
	  bcopy(sc->is_dpaddr, ifd->default_pa, 6);
	  bcopy(sc->is_addr, ifd->current_pa, 6);

	break;

      case SIOCSPHYSADDR:
	if (tradebug > 1)
		printf ("SIOCSPHYSADDR\n");
	/*
	 * The LSBit of the MSByte (for canonical addresses) must be zero.
	 */
	if (ifr->ifr_addr.sa_data[0] & 0x01) {
	    printf("%s%d: Illegal node address: %s\n", ADAP, ifp->if_unit,
				  ether_sprintf(ifr->ifr_addr.sa_data));
	    return(EINVAL);
	}
	bcopy(ifr->ifr_addr.sa_data, sc->is_addr, 6);
	if(!trareset(sc))
	    return(EIO);
	if(((struct arpcom *)ifp)->ac_ipaddr.s_addr)
	    	rearpwhohas((struct arpcom *)ifp,
			  &((struct arpcom *)ifp)->ac_ipaddr);
	break;

      case SIOCADDMULTI:
      case SIOCDELMULTI:
	{
	    /*
	     * NOTE: The received canonical address must be in the
	     *       canonical form.
	     */
	    u_short func_addr1;	/* The first 2 bytes - should be 0xC000 */
	    u_int func_addr2;	/* The last 4 bytes */
	    if (tradebug > 1)
		printf ("SIOCDELMULTI\n");

	    haddr_convert(ifr->ifr_addr.sa_data);

	    /* The first 2 bytes should be 0xC000 */
	    if ( ((u_char)ifr->ifr_addr.sa_data[0] != 0xC0) || 
			((u_char)ifr->ifr_addr.sa_data[1] != 0x00) )
		return(EINVAL);

	    bcopy((u_char *)&(ifr->ifr_addr.sa_data[2]), &func_addr2, 4);

	    if (cmd == SIOCDELMULTI) {
		    u_int tmp;
		    /*
		     * First check if the functional address provided
		     * are effective. i.e if they haven't been enabled
	             * the address can't be deleted.
		     */
		    tmp = func_addr2 & sc->func_addr; 
		    if (tmp != func_addr2)
			return(EINVAL);
		    else
		      sc->func_addr ^= func_addr2;
	    } else 
		    sc->func_addr |= func_addr2;

	    if (ifp->if_flags & IFF_RUNNING)
		if(!trareset(sc))
		    return(EIO);
	    break;
	}
      case SIOCRDZCTRS :
	if (tradebug > 1)
		printf ("SIOCRDZCTRS\n");
	break;

      case SIOCRDCTRS :
	if (tradebug > 1)
		printf ("SIOCRDCTRS\n");
	switch (trnctr->ctr_type) {
	  case TRN_CHAR:
	    tra_fill_chars(sc, trnctr);
	    break;
	  case TRN_MIB_ENTRY:
	    tra_fill_mib_entry(sc, trnctr);
	    break;
	  case TRN_MIB_STAT_ENTRY:
	    tra_fill_mib_stats_entry(sc, trnctr);
	    break;
	  case TRN_MIB_TIMER_ENTRY:	/* This is optional and not supported */
	    return(ENOTSUP);
	    break;
	  default:
	  case CTR_TRN:
	    tra_fill_counters(sc, trnctr);
	    trnctr->ctr_type = CTR_TRN;
	    break;
	}
	break;

      case SIOCSIFADDR:
	if (tradebug > 1)
		printf ("SIOCSIFADDR\n");

	if (!(ifp->if_flags & IFF_RUNNING)) {
	    u_short return_val;

	    return_val = trainit(ifp->if_unit);
	    if (return_val == COMM_REJ_SAME_COMMAND ||
		return_val == COMM_REJ_ADAPTER_OPEN) {
		printf("%s%d: Retrying...", ADAP, ifp->if_unit);
		/* reset the adapter and try again */
		return_val = trareset(ifp->if_unit);
		if (!return_val)
		    return(EIO);
	    } else {
		if (return_val == 0)
		    return(EIO);
	    }
	}
	ifp->if_flags |= IFF_UP;
	/*
	 * Turn LED2 on.
	 */
         TRAREGWR(sc->csr0_leds, TRAREGRD(sc->csr0_leds) | 0x0002);
	 TRAIOSYNC();
	switch(ifa->ifa_addr->sa_family) {
	  case AF_INET:
	    ((struct arpcom *)ifp)->ac_ipaddr = IA_SIN(ifa)->sin_addr;
	    break;
	}
#ifdef TRADEBUG1
	read_adap_values(sc);
#endif TRADEBUG1
	break;

      case SIOCSIFFLAGS:
	if (tradebug > 1)
		printf ("SIOCSIFFLAGS\n");

	if (ifr->ifr_flags & IFF_UP) {
	    if (sc->mac_open)	/* The interface is already up and running */
		break;
	    else
		if(!trareset(sc))
		    return(EIO);
	} else {
	    if (!sc->mac_open) /* The interface is already down */
		break;
	    else {
		ifp->if_flags &= ~IFF_UP;
		ifp->if_flags &= ~IFF_RUNNING;
		if (!tra_mac_close(sc))
		    return(EIO);
		/*
		 * Turn LED2 off.
		 */
		TRAREGWR(sc->csr0_leds, TRAREGRD(sc->csr0_leds) & 0x0001);
		TRAIOSYNC();
	    }
	}
	break;

      case SIOCIFRESET:
	if (tradebug > 1)
		printf ("SIOCIFRESET\n");
	if(!trareset(sc))
	    return(EIO);
	break;

      case SIOCIFSETCHAR:
	if (tradebug > 1)
		printf ("SIOCIFSETCHAR\n");
	break;

      case SIOCSMACSPEED:
      {
	u_short speed;
	if (tradebug > 1)
		printf ("SIOCSMACSPEED\n");

	bcopy(ifr->ifr_data, (u_char *)(&speed), sizeof(u_short));
	if (speed == sc->ring_speed)
	     break;
	if (speed == 4)
	    sc->ring_speed = 4;
	else if (speed == 16)
	    sc->ring_speed = 16;
	else
	    return(EINVAL);

	if (ifp->if_flags & IFF_RUNNING)
	    if(!trareset(sc))
	        return(EIO);
	break;
      }

      case SIOCRMACSPEED:
	ifr->ifr_value = (int)sc->ring_speed;
	break;

      case SIOCSIPMTU:
      {
	u_short ifmtu;
	if (tradebug > 1)
		printf ("SIOCSIPMTU\n");

	bcopy(ifr->ifr_data, (u_char *)(&ifmtu), sizeof(u_short));

	if (sc->ring_speed == 16) { 
	    if (ifmtu > TRN16_RFC1042_IP_MTU || ifmtu < IP_MINMTU)
	        return(EINVAL);
	    else
	        ifp->if_mtu = ifmtu;
	}

	if (sc->ring_speed == 4) { 
	    if (ifmtu > TRN4_RFC1042_IP_MTU || ifmtu < IP_MINMTU)
	        return(EINVAL);
	    else
	        ifp->if_mtu = ifmtu;
	}

	break;
      }

      default:
	if (tradebug > 1)
		printf ("Unknown ioctl command.\n");
	return(EINVAL);
     }
     return(ESUCCESS);
}


/*
 * Name:	tra_fill_chars(sc, trnctr)
 *
 * Arguments:
 *	sc:	The softc structure.
 *	trnctr: The structure to be filled in.
 *
 * Functional Description:
 * 	Gets the characteristics and fill it into the user structure.
 *
 * Calls:
 *	None.
 *
 * Return Codes:
 *	None.
 *
 */
void
tra_fill_chars(sc, trnctr)
struct tra_softc *sc;
struct ctrreq *trnctr;
{

    union mac_addr addr;
    u_int *tmp;
    u_short value, i, k;
    u_char tmp_addr[6];

    k = splimp();

    /* MAC address */
    /* convert the address to its canonical form */
    bcopy(sc->is_dpaddr, tmp_addr, 6);
    bcopy(tmp_addr, trnctr->ctr_ctrs.trnchar.mac_addr, 6);

    /* Group Address */
    TRAREGWR(sc->tms_sifadx, 0x01); TRAIOSYNC();
    TRAREGWR(sc->tms_sifadr1, sc->adap_ptr.grp_addr); TRAIOSYNC();
    addr.saddr[0] = 0x00C0;
    for (i = 1; i < 3; i++) {
	addr.saddr[i] =  ntohs(TRAREGRD(sc->tms_sifdat_inc));
    }
    /* sc->grp_addr = *((u_int *)(&addr.saddr[1]));  SNU: Do we need this? */
    bcopy(addr.caddr, trnctr->ctr_ctrs.trnchar.grp_addr, 6);
    haddr_convert(trnctr->ctr_ctrs.trnchar.grp_addr); 

    /* Functional address */
    TRAREGWR(sc->tms_sifadx, 0x01); TRAIOSYNC();
    TRAREGWR(sc->tms_sifadr1, sc->adap_ptr.func_addr); TRAIOSYNC();
    addr.saddr[0] = 0x00C0;
    for (i = 1; i < 3; i++) {
	addr.saddr[i] =  ntohs(TRAREGRD(sc->tms_sifdat_inc));
    }
    /* sc->func_addr = *((u_int *)(&addr.saddr[1])); SNU: Do we need this? */
    bcopy(addr.caddr, trnctr->ctr_ctrs.trnchar.func_addr, 6);
    haddr_convert(trnctr->ctr_ctrs.trnchar.func_addr);

    /* Drop number */
    TRAREGWR(sc->tms_sifadx, 0x01); TRAIOSYNC();
    TRAREGWR(sc->tms_sifadr1, sc->adap_ptr.phy_drop_numb); TRAIOSYNC();

    trnctr->ctr_ctrs.trnchar.drop_numb = TRAREGRD(sc->tms_sifdat_inc);
    
    /* UNA */
    TRAREGWR(sc->tms_sifadx, 0x01); TRAIOSYNC();
    TRAREGWR(sc->tms_sifadr1, sc->adap_ptr.upstream_neighbor); TRAIOSYNC();

    for (i = 0; i < 3; i++) {
	addr.saddr[i] =  ntohs(TRAREGRD(sc->tms_sifdat_inc));
    }
    haddr_convert(addr.caddr);
    bcopy(addr.caddr, trnctr->ctr_ctrs.trnchar.upstream_nbr, 6);

    /* Upstream drop number */
    TRAREGWR(sc->tms_sifadx, 0x01); TRAIOSYNC();
    TRAREGWR(sc->tms_sifadr1, sc->adap_ptr.upstream_drop_numb); TRAIOSYNC();
    trnctr->ctr_ctrs.trnchar.upstream_drop_numb = TRAREGRD(sc->tms_sifdat_inc);
    
    /* Transmit access priority */
    TRAREGWR(sc->tms_sifadx, 0x01); TRAIOSYNC();
    TRAREGWR(sc->tms_sifadr1, sc->adap_ptr.xmit_access_priority); TRAIOSYNC();
    trnctr->ctr_ctrs.trnchar.transmit_access_pri = TRAREGRD(sc->tms_sifdat_inc);

    /* Last Major Vector */
    TRAREGWR(sc->tms_sifadx, 0x01); TRAIOSYNC();
    TRAREGWR(sc->tms_sifadr1, sc->adap_ptr.last_major_vector); TRAIOSYNC();
    trnctr->ctr_ctrs.trnchar.last_major_vector = TRAREGRD(sc->tms_sifdat_inc);

    /* Ring Status */
    TRAREGWR(sc->tms_sifadx, 0x01); TRAIOSYNC();
    TRAREGWR(sc->tms_sifadr1, sc->adap_ptr.ring_status); TRAIOSYNC();
    value = TRAREGRD(sc->tms_sifdat_inc);

    trnctr->ctr_ctrs.trnchar.ring_status = 0;
    
    if (value ==  MIB1231_RSTATUS_NO_PROB && sc->ring_status != value) {
	value = sc->ring_status;
	sc->ring_status =  MIB1231_RSTATUS_NO_PROB;
    } 
    trnctr->ctr_ctrs.trnchar.ring_status = value;

    /* Monitor Contender */
    trnctr->ctr_ctrs.trnchar.monitor_contd = sc->mon_cntd;

    /* Soft Error Timer
     * The timer value reported back is in units of
     * 10ms. So we multiply whatever we read by 10.
     */
    TRAREGWR(sc->tms_sifadx, 0x01); TRAIOSYNC();
    TRAREGWR(sc->tms_sifadr1, sc->adap_ptr.soft_err_timer_val); TRAIOSYNC();
    trnctr->ctr_ctrs.trnchar.soft_error_timer = 10 * TRAREGRD(sc->tms_sifdat_inc);

    /* Local ring number */
    TRAREGWR(sc->tms_sifadx, 0x01); TRAIOSYNC();
    TRAREGWR(sc->tms_sifadr1, sc->adap_ptr.local_ring_numb); TRAIOSYNC();
    trnctr->ctr_ctrs.trnchar.ring_number = TRAREGRD(sc->tms_sifdat_inc);

    /* Monitor Error Code */
    TRAREGWR(sc->tms_sifadx, 0x01); TRAIOSYNC();
    TRAREGWR(sc->tms_sifadr1, sc->adap_ptr.monitor_error_code); TRAIOSYNC();
    trnctr->ctr_ctrs.trnchar.monitor_error_code = TRAREGRD(sc->tms_sifdat_inc);

    /* Beacon Transmit Type */
    TRAREGWR(sc->tms_sifadx, 0x01); TRAIOSYNC();
    TRAREGWR(sc->tms_sifadr1, sc->adap_ptr.beacon_xmit_type); TRAIOSYNC();
    trnctr->ctr_ctrs.trnchar.beacon_transmit_type = TRAREGRD(sc->tms_sifdat_inc);
    /* Beacon Receive Type */
    TRAREGWR(sc->tms_sifadx, 0x01); TRAIOSYNC();
    TRAREGWR(sc->tms_sifadr1, sc->adap_ptr.beacon_rcv_type); TRAIOSYNC();
    trnctr->ctr_ctrs.trnchar.beacon_receive_type = TRAREGRD(sc->tms_sifdat_inc);

    /* Beacon UNA */
    TRAREGWR(sc->tms_sifadx, 0x01); TRAIOSYNC();
    TRAREGWR(sc->tms_sifadr1, sc->adap_ptr.beaconing_stn_una); TRAIOSYNC();
    for (i = 0; i < 3; i++) {
	addr.saddr[i] =  ntohs(TRAREGRD(sc->tms_sifdat_inc));
    }
    haddr_convert(addr.caddr); /* convert to canonical */
    bcopy(addr.caddr, trnctr->ctr_ctrs.trnchar.beacon_una, 6);

    /* Beacon station drop number */
    TRAREGWR(sc->tms_sifadx, 0x01); TRAIOSYNC();
    TRAREGWR(sc->tms_sifadr1, sc->adap_ptr.beaconing_stn_phy_drop_numb); 
    TRAIOSYNC();
    trnctr->ctr_ctrs.trnchar.beacon_stn_drop_numb = TRAREGRD(sc->tms_sifdat_inc);

    /* Ring Speed */
    if (sc->ring_speed == 16)
	trnctr->ctr_ctrs.trnchar.ring_speed = MIB1231_RSPEED_16_MEG;
    else
	 trnctr->ctr_ctrs.trnchar.ring_speed = MIB1231_RSPEED_4_MEG;

    /* Early Token Release */
    trnctr->ctr_ctrs.trnchar.etr = sc->etr;

    /* Open status */
    trnctr->ctr_ctrs.trnchar.open_status = sc->open_status;

    /* Token ring chip details */
    bcopy(sc->device_type, trnctr->ctr_ctrs.trnchar.token_ring_chip,
      sizeof(sc->device_type) + 1);
    splx(k);
}


/*
 * Name:	tra_fill_counters(sc, trnctr)
 *
 * Arguments:
 *	sc:	The softc structure.
 *	trnctr: The structure to be filled in.
 *
 * Functional Description:
 * 	Fills in the counters.
 *
 * Calls:
 *	None.
 *
 * Return Codes:
 *	None.
 *
 */
void
tra_fill_counters(sc, trnctr)
struct tra_softc *sc;
struct ctrreq *trnctr;
{
    /* Read the error log counters */
    tra_read_error_log(sc);


    trnctr->ctr_ctrs.trncount.trn_second = time.tv_sec - sc->ztime;
    trnctr->ctr_ctrs.trncount.trn_bytercvd = sc->tracount.tra_bytercvd;
    trnctr->ctr_ctrs.trncount.trn_bytesent = sc->tracount.tra_bytesent;
    trnctr->ctr_ctrs.trncount.trn_pdurcvd = sc->tracount.tra_pdurcvd;
    trnctr->ctr_ctrs.trncount.trn_pdusent = sc->tracount.tra_pdusent;
    trnctr->ctr_ctrs.trncount.trn_mbytercvd = sc->tracount.tra_mbytercvd;
    trnctr->ctr_ctrs.trncount.trn_mbytesent = sc->tracount.tra_mbytesent;
    trnctr->ctr_ctrs.trncount.trn_mpdurcvd = sc->tracount.tra_mpdurcvd;
    trnctr->ctr_ctrs.trncount.trn_mpdusent = sc->tracount.tra_mpdusent;
    trnctr->ctr_ctrs.trncount.trn_pduunrecog = sc->tracount.tra_pduunrecog;
    trnctr->ctr_ctrs.trncount.trn_mpduunrecog =  0;	/* Not available */
    trnctr->ctr_ctrs.trncount.trn_nosysbuf = sc->count.no_sysbuffers;
    trnctr->ctr_ctrs.trncount.trn_xmit_fail = sc->count.xmit_fail;
    trnctr->ctr_ctrs.trncount.trn_xmit_underrun = 0;	/* Not available */
    trnctr->ctr_ctrs.trncount.trn_line_error = sc->error_cnts.line_error;
    trnctr->ctr_ctrs.trncount.trn_internal_error = sc->count.hard_error;
    trnctr->ctr_ctrs.trncount.trn_burst_error = sc->error_cnts.burst_error;
    trnctr->ctr_ctrs.trncount.trn_ari_fci_error = sc->error_cnts.ari_fci_error;
    trnctr->ctr_ctrs.trncount.trn_ad_trans = 0;	/* Not available */
    trnctr->ctr_ctrs.trncount.trn_lost_frame_error = sc->error_cnts.lost_frame_error;
    trnctr->ctr_ctrs.trncount.trn_rcv_congestion_error = sc->error_cnts.rcv_congestion_error;
    trnctr->ctr_ctrs.trncount.trn_frame_copied_error = sc->error_cnts.frame_copied_error;
    trnctr->ctr_ctrs.trncount.trn_frequency_error = 0;	/* Not available */
    trnctr->ctr_ctrs.trncount.trn_token_error = sc->error_cnts.token_error;
    trnctr->ctr_ctrs.trncount.trn_hard_error = sc->count.hard_error;
    trnctr->ctr_ctrs.trncount.trn_soft_error = sc->count.soft_error;
    trnctr->ctr_ctrs.trncount.trn_adapter_reset = sc->count.board_reset;
    trnctr->ctr_ctrs.trncount.trn_signal_loss = sc->count.signal_loss;
    trnctr->ctr_ctrs.trncount.trn_xmit_beacon = sc->count.xmit_beacon;
    trnctr->ctr_ctrs.trncount.trn_ring_recovery = sc->count.ring_recovery;
    trnctr->ctr_ctrs.trncount.trn_lobe_wire_fault = sc->count.lobe_wire_fault;
    trnctr->ctr_ctrs.trncount.trn_remove_received = sc->count.remove_received;
    trnctr->ctr_ctrs.trncount.trn_single_station = sc->count.single_station;
    trnctr->ctr_ctrs.trncount.trn_selftest_fail = sc->count.selftest_fail;
}



/*
 * Name:	tra_fill_mib_entry(sc, trnctr)
 *
 * Arguments:
 *	sc:	The softc structure.
 *	trnctr: The structure to be filled in.
 *
 * Functional Description:
 * 	Fills in the counters.
 *
 * Calls:
 *	None.
 *
 * Return Codes:
 *	None.
 *
 */
void
tra_fill_mib_entry(sc, trnctr)
struct tra_softc *sc;
struct ctrreq *trnctr;
{
    struct ifnet *ifp = &sc->is_if;
    u_short value, i, k;
    union mac_addr addr;

    k = splimp(); 

    trnctr->ctr_ctrs.dot5Entry.dot5TrnNumber = trn_units;
    trnctr->ctr_ctrs.dot5Entry.dot5IfIndex = ifp->if_unit + 1;
    trnctr->ctr_ctrs.dot5Entry.dot5Commands = MIB1231_COMM_NO_OP;

    /* Ring Status */
    TRAREGWR(sc->tms_sifadx, 0x01); TRAIOSYNC();
    TRAREGWR(sc->tms_sifadr1, sc->adap_ptr.ring_status); TRAIOSYNC();
    value = TRAREGRD(sc->tms_sifdat_inc);
    trnctr->ctr_ctrs.dot5Entry.dot5RingStatus = 0;
    if (value ==  MIB1231_RSTATUS_NO_PROB && sc->ring_status != value) {
	value = sc->ring_status;
	sc->ring_status =  MIB1231_RSTATUS_NO_PROB;
    } 
    trnctr->ctr_ctrs.dot5Entry.dot5RingStatus = value;
    trnctr->ctr_ctrs.dot5Entry.dot5RingState = sc->ring_state;
    trnctr->ctr_ctrs.dot5Entry.dot5RingOpenStatus = sc->open_status;
    trnctr->ctr_ctrs.dot5Entry.dot5RingSpeed = sc->ring_speed;

    /* UNA */
    TRAREGWR(sc->tms_sifadx, 0x01); TRAIOSYNC();
    TRAREGWR(sc->tms_sifadr1, sc->adap_ptr.upstream_neighbor); TRAIOSYNC();

    for (i = 0; i < 3; i++) {
	addr.saddr[i] =  TRAREGRD(sc->tms_sifdat_inc);
    }

    bcopy(addr.caddr, trnctr->ctr_ctrs.dot5Entry.dot5UpStream, 6);
    trnctr->ctr_ctrs.dot5Entry.dot5ActMonParticipate = sc->mon_cntd;

    /* Functional address */
    TRAREGWR(sc->tms_sifadx, 0x01); TRAIOSYNC();
    TRAREGWR(sc->tms_sifadr1, sc->adap_ptr.func_addr); TRAIOSYNC();
    addr.saddr[0] = 0xC000;
    for (i = 1; i < 3; i++) {
	addr.saddr[i] =  TRAREGRD(sc->tms_sifdat_inc);
    }
    bcopy(addr.caddr, trnctr->ctr_ctrs.dot5Entry.dot5Functional, 6);
    splx(k);
}



/*
 * Name:	tra_fill_mib_stats_entry(sc, trnctr)
 *
 * Arguments:
 *	sc:	The softc structure.
 *	trnctr: The structure to be filled in.
 *
 * Functional Description:
 * 	Fills in the counters.
 *
 * Calls:
 *	None.
 *
 * Return Codes:
 *	None.
 *
 */
void
tra_fill_mib_stats_entry(sc, trnctr)
struct tra_softc *sc;
struct ctrreq *trnctr;
{
    struct ifnet *ifp = &sc->is_if;
    u_short value, i;
    union mac_addr addr;

    /* Read the error log counters */
    tra_read_error_log(sc);

    trnctr->ctr_ctrs.dot5StatsEntry.dot5StatsIfIndex = ifp->if_unit + 1;
    trnctr->ctr_ctrs.dot5StatsEntry.dot5StatsLineErrors = sc->error_cnts.line_error;
    trnctr->ctr_ctrs.dot5StatsEntry.dot5StatsBurstErrors = sc->error_cnts.burst_error;
    trnctr->ctr_ctrs.dot5StatsEntry.dot5StatsACErrors = sc->error_cnts.ari_fci_error;
    /* Not available */
    trnctr->ctr_ctrs.dot5StatsEntry.dot5StatsAbortTransErrors = 0;

    trnctr->ctr_ctrs.dot5StatsEntry.dot5StatsInternalErrors = sc->count.hard_error;;
    trnctr->ctr_ctrs.dot5StatsEntry.dot5StatsLostFrameErrors = sc->error_cnts.lost_frame_error;;
    trnctr->ctr_ctrs.dot5StatsEntry.dot5StatsReceiveCongestions = sc->error_cnts.rcv_congestion_error;
    trnctr->ctr_ctrs.dot5StatsEntry.dot5StatsFrameCopiedErrors = sc->error_cnts.frame_copied_error;
    trnctr->ctr_ctrs.dot5StatsEntry.dot5StatsTokenErrors = sc->error_cnts.token_error;
    trnctr->ctr_ctrs.dot5StatsEntry.dot5StatsSoftErrors = sc->count.soft_error;
    trnctr->ctr_ctrs.dot5StatsEntry.dot5StatsHardErrors = sc->count.hard_error;
    trnctr->ctr_ctrs.dot5StatsEntry.dot5StatsSignalLoss = sc->count.signal_loss;
    trnctr->ctr_ctrs.dot5StatsEntry.dot5StatsTransmitBeacons = sc->count.xmit_beacon;
    trnctr->ctr_ctrs.dot5StatsEntry.dot5StatsRecoverys = sc->count.ring_recovery;
    trnctr->ctr_ctrs.dot5StatsEntry.dot5StatsLobeWires = sc->count.lobe_wire_fault;
    trnctr->ctr_ctrs.dot5StatsEntry.dot5StatsRemoves = sc->count.remove_received;
    trnctr->ctr_ctrs.dot5StatsEntry.dot5StatsSingles = sc->count.single_station;
    /* Not available */
    trnctr->ctr_ctrs.dot5StatsEntry.dot5StatsFreqErrors = 0;
}


/*
 * Name:	print_reject_reason(reason);
 *
 * Arguments:
 *	reason:	The reject reason.
 *
 * Functional Description:
 * 	Prints the reason for the command rejection.
 *
 * Calls:
 *	None.
 *
 * Return Codes:
 *	None.
 *
 */
print_reject_reason(reason)
u_short reason;
{
    switch (reason) {
	/*
	 * Find out the reason for the command reject.
	 */
      case COMM_REJ_ILL_COMMAND:
	printf("Unkown Command Issued To Adapter.\n");
	break;
	
      case COMM_REJ_ADDR_ERROR:
	printf("SCB Address Field Not Word Aligned.\n");
	break;
	
      case COMM_REJ_ADAPTER_OPEN:
	printf("The Adapter is Open.\n");
	break;
	
      case COMM_REJ_ADAPTER_CLOSED:
	printf("The Adapter Is Closed.\n");
	break;
	
      case COMM_REJ_SAME_COMMAND:
	printf("The Command Is Already Being Executed.\n");
	break;
    }
}


/*
 * Name:	print_open_error(sc, reason);
 *
 * Arguments:
 *	sc:	The softc structure.
 *	reason:	The error reason.
 *	
 * Functional Description:
 * 	Prints the error on MAC_OPEN.
 *
 * Calls:
 *	None.
 *
 * Return Codes:
 *	Success:	1
 *
 */
print_open_error(sc, reason)
struct tra_softc *sc;
u_short reason;
{
    if (reason & OPENC_NODE_ADDR_ERROR)
	    printf("Node Address Error.\n");

    if (reason & OPENC_LIST_SIZE_ERROR)
	    printf("Transmit or Receive List Size Error.\n");

    if (reason & OPENC_BUFF_SIZE_ERROR)
	    printf("Buffer Size Error.\n");

    if (reason & OPENC_XMIT_BUFF_COUNT_ERROR)
	    printf("Transmit Buffer Count Error.\n");

    if (reason & OPENC_INV_OPEN_OPTION)
	    printf("Invalid Open Option.\n");

    sc->open_status = MIB1231_ROSTATUS_BADPARM;

    if (reason & OPENC_OPEN_ERROR) {
	u_short phase = (reason & OPENC_PHASE_MASK) >> 4;
	u_short	error = reason & OPENC_ERROR_MASK;
	printf("Open error:  ");
	switch (phase) {
	  case OPENC_LOBE_MEDIA_TEST:
	    printf("Phase: Lobe Media Test; ");
	    break;

	  case OPENC_PHYS_INSERTION:
	    printf("Phase: Physical insertion; ");
	    break;

	  case OPENC_ADDR_VERIFY:
	    printf("Phase: Address verification; ");
	    break;

	  case OPENC_PART_POLL_RING:
	    printf("Phase: Participation in ring poll; ");
	    sc->count.ring_poll_error++;
	    break;

	  case OPENC_REQ_INIT:
	    printf("Phase: Request initialization; ");
	    break;
	}

	switch (error) {
	  case OPENC_FUNCT_FAILURE:
	    printf("Error: Function failure\n");
	    sc->open_status = MIB1231_ROSTATUS_LOBEFAILED;
	    break;

	  case OPENC_SIGNAL_LOSS:
	    printf("Error: Signal loss\n");
	    sc->count.signal_loss++;
	    sc->open_status = MIB1231_ROSTATUS_SIG_LOSS;
	    break;

	  case OPENC_TIMEOUT:
	    printf("Error: Insertion timeout\n");
	    sc->open_status = MIB1231_ROSTATUS_INS_TIMEOUT;
	    break;

	  case OPENC_RING_FAIL:
	    printf("Error: Ring purge timeout\n");
	    sc->open_status = MIB1231_ROSTATUS_RING_FAILED;
	    break;

	  case OPENC_DUP_NODE_ADDR:
	    printf("Error: Duplicate node address\n");
	    sc->open_status = MIB1231_ROSTATUS_DUPLICATE_MAC;
	    break;

	  case OPENC_REQ_PARMS:
	    printf("Error: Ring parameter server did not respond\n");
	    sc->open_status = MIB1231_ROSTATUS_REQ_FAILED;
	    break;

	  case OPENC_REMOVE_RCVD:
	    printf("Error: Remove received\n");
	    sc->open_status = MIB1231_ROSTATUS_REM_RECVD;
	    break;
	  default:
	    printf("\n");
	    sc->open_status = MIB1231_ROSTATUS_NOOPEN;
	    break;
	}
    }
    return(1);
}

/*
 * Name:	get_internal_ptrs(unit);
 *
 * Arguments:
 *	sc:	The softc structure for this interface.
 *
 * Functional Description:
 *	This copies the adapter internal pointers into the softc
 *	structure.
 *
 * Calls:
 *	None.
 *
 * Return Codes:
 *	Success:	1
 *
 *
 */
get_internal_ptrs(sc)
struct tra_softc *sc;
{

    /*
     * Load the address 0x010a00. See Page 4-48.
     */
    TRAREGWR(sc->tms_sifadx, 0x01);
    TRAIOSYNC();
    TRAREGWR(sc->tms_sifadr1, 0x0A00);
    TRAIOSYNC();
    sc->adap_ptr.bia = TRAREGRD(sc->tms_sifdat_inc);
    sc->adap_ptr.software_level = TRAREGRD(sc->tms_sifdat_inc);
    sc->adap_ptr.node_addr =  TRAREGRD(sc->tms_sifdat_inc);
    sc->adap_ptr.grp_addr = sc->adap_ptr.node_addr + 6;
    sc->adap_ptr.func_addr = sc->adap_ptr.node_addr + 10;

    TRAREGWR(sc->tms_sifadr1, 0x0A06);
    TRAIOSYNC();
    sc->adap_ptr.phy_drop_numb = TRAREGRD(sc->tms_sifdat_inc);
    sc->adap_ptr.upstream_neighbor = sc->adap_ptr.phy_drop_numb + 4;
    sc->adap_ptr.upstream_drop_numb = sc->adap_ptr.phy_drop_numb + 10;
    sc->adap_ptr.last_ring_poll_addr = sc->adap_ptr.phy_drop_numb + 14;
    sc->adap_ptr.xmit_access_priority = sc->adap_ptr.phy_drop_numb + 22;
    sc->adap_ptr.src_class_auth = sc->adap_ptr.phy_drop_numb + 24;
    sc->adap_ptr.last_attn_code = sc->adap_ptr.phy_drop_numb + 26;
    sc->adap_ptr.last_src_addr =  sc->adap_ptr.phy_drop_numb + 28;	
    sc->adap_ptr.last_beacon_type =  sc->adap_ptr.phy_drop_numb + 34;
    sc->adap_ptr.last_major_vector = sc->adap_ptr.phy_drop_numb + 36;
    sc->adap_ptr.ring_status =  sc->adap_ptr.phy_drop_numb + 38;
    sc->adap_ptr.soft_err_timer_val = sc->adap_ptr.phy_drop_numb + 40;
    sc->adap_ptr.ring_intf_err_cnt =  sc->adap_ptr.phy_drop_numb + 42;
    sc->adap_ptr.local_ring_numb = sc->adap_ptr.phy_drop_numb + 44;
    sc->adap_ptr.monitor_error_code = sc->adap_ptr.phy_drop_numb + 46;
    sc->adap_ptr.beacon_xmit_type =sc->adap_ptr.phy_drop_numb + 48;
    sc->adap_ptr.beacon_rcv_type = sc->adap_ptr.phy_drop_numb + 50;
    sc->adap_ptr.frame_corr_sav = sc->adap_ptr.phy_drop_numb + 52;
    sc->adap_ptr.beaconing_stn_una = sc->adap_ptr.phy_drop_numb + 54;
    sc->adap_ptr.beaconing_stn_phy_drop_numb = sc->adap_ptr.phy_drop_numb + 64;
    sc->adap_ptr.mac_buf_ptr = TRAREGRD(sc->tms_sifdat_inc);

    TRAREGWR(sc->tms_sifadr1, 0x0A0c);
    TRAIOSYNC();
    sc->adap_ptr.speed_flag =  TRAREGRD(sc->tms_sifdat_inc);
    sc->adap_ptr.total_ram = TRAREGRD(sc->tms_sifdat_inc);
}

#ifdef TRADEBUG1

/*
 * Name:	read_adap_values(sc, addr, count);
 *
 * Arguments:
 *	sc:	The softc structure for the adapter.
 *	addr: 	The contents of which needs to be printed out.
 *	count:	How many words to print.
 *
 * Functional Description:
 *	Prints contents of the register.
 *
 * Calls:
 *	None.
 *
 * Return Codes:
 *
 *
 *
 */
read_adap_values(sc)
struct tra_softc *sc;
{
    printf("*******OUTPUT OF ADAPTER INTERNAL PARAMETERS*******\n");

    printf("bia: ");
    print_adap_value(sc, sc->adap_ptr.bia, 3);
    printf("software level: ");
    print_adap_value(sc, sc->adap_ptr.software_level, 5);
    printf("node address: ");
    print_adap_value(sc, sc->adap_ptr.node_addr, 3);
    printf("group address: ");
    print_adap_value(sc, sc->adap_ptr.grp_addr, 2);
    printf("functional address: ");
    print_adap_value(sc, sc->adap_ptr.func_addr, 2);
    printf("physical drop number: ");
    print_adap_value(sc, sc->adap_ptr.phy_drop_numb, 2);
    printf("upstream neighbor address: ");
    print_adap_value(sc, sc->adap_ptr.upstream_neighbor, 3);
    printf("upstream drop number: ");
    print_adap_value(sc, sc->adap_ptr.upstream_drop_numb, 2);
    printf("last ring poll address: ");
    print_adap_value(sc, sc->adap_ptr.last_ring_poll_addr, 3);
    printf("transmit access priority: ");
    print_adap_value(sc, sc->adap_ptr.xmit_access_priority, 1);
    printf("source class authorization: ");
    print_adap_value(sc, sc->adap_ptr.src_class_auth, 1);
    printf("last attention code: ");
    print_adap_value(sc, sc->adap_ptr.last_attn_code, 1);
    printf("last source address: ");
    print_adap_value(sc, sc->adap_ptr.last_src_addr, 3);
    printf("last beacon type: ");
    print_adap_value(sc, sc->adap_ptr.last_beacon_type, 1);
    printf("last major vector: ");
    print_adap_value(sc, sc->adap_ptr.last_major_vector, 1);
    printf("Ring Status: ");
    print_adap_value(sc, sc->adap_ptr.ring_status, 1);
    printf("soft error timer value: ");
    print_adap_value(sc, sc->adap_ptr.soft_err_timer_val, 1);
    printf("ring interface error counter: ");
    print_adap_value(sc, sc->adap_ptr.ring_intf_err_cnt, 1);
    printf("local ring number: ");
    print_adap_value(sc, sc->adap_ptr.local_ring_numb, 1);
    printf("mointor error code: ");
    print_adap_value(sc, sc->adap_ptr.monitor_error_code, 1);
    printf("beacon xmit type: ");
    print_adap_value(sc, sc->adap_ptr.beacon_xmit_type, 1);
    printf("beacon rcv type: ");
    print_adap_value(sc, sc->adap_ptr.beacon_rcv_type, 1);
    printf("Frame correlator save: ");
    print_adap_value(sc, sc->adap_ptr.frame_corr_sav, 1);
    printf("beaconing station UNA: ");
    print_adap_value(sc, sc->adap_ptr.beaconing_stn_una, 3);
    printf("beaconing station physical drop number: ");
    print_adap_value(sc, sc->adap_ptr.beaconing_stn_phy_drop_numb, 1);
    printf("Speed flag: ");
    print_adap_value(sc, sc->adap_ptr.speed_flag, 1);
    printf("Total adapter RAM found:  ");
    print_adap_value(sc, sc->adap_ptr.total_ram, 1);
}


/*
 * Name:	print_adap_value(sc, addr, count);
 *
 * Arguments:
 *	sc:	The softc structure for the adapter.
 *	addr: 	The contents of which needs to be printed out.
 *	count:	How many words to print.
 *
 * Functional Description:
 *	Prints contents of the register.
 *
 * Calls:
 *	None.
 *
 * Return Codes:
 *
 *
 *
 */
print_adap_value(sc, addr, count)
struct tra_softc *sc;
u_short addr;
u_short count;
{
    u_short value, i;

    TRAREGWR(sc->tms_sifadx, 0x01);
    TRAIOSYNC();
    TRAREGWR(sc->tms_sifadr1, addr);
    TRAIOSYNC();

    for (i = 0; i < count; i++) {
	value =  TRAREGRD(sc->tms_sifdat_inc);
	printf("0x%X ", value);
    }
    
    printf("\n");
}
#endif TRADEBUG1

/*
 * Name:	print_init_error(unit);
 *
 * Arguments:
 *	unit:	The unit number of the adpater.
 *
 * Functional Description:
 *	This function prints the error encountered  during initialization.
 *
 * Calls:
 *	None.
 *
 * Return Codes:
 *	Success:	1
 *
 *
 */
print_init_error(unit)
u_short unit;
{
    struct tra_softc *sc = &tra_softc[unit];
    struct ifnet *ifp = &sc->is_if;
    u_short value;

    value = TRAREGRD(sc->tms_sifcmd_sts);

    value = value & CMDSTS_M_ERROR_REASON;

    printf("%s%d: Initialization Error: ", ADAP, ifp->if_unit);

    switch(value) {

      case INITOP_INVALID_BLOCK_ERR:
	printf("Invalid initialization block.\n");
	break;

      case INITOP_INVALID_OPTION_ERR:
	printf("Invalid options.\n");
	break;

      case INITOP_INVALID_RCV_BURST_SZ_ERR:
	printf("Invalid receive burst size.\n");
	break;

      case INITOP_INVALID_XMT_BURST_SZ_ERR:
	printf("Invalid transmit burst count.\n");
	break;

      case INITOP_INVALID_DMA_THRESH_ERR:
	printf("Invalid DMA abort threshold.\n");
	break;

      case INITOP_INVALID_SCB_ERR:
	printf("Invalid System Command Block address.\n");
	break;

      case INITOP_INVALID_SSB_ERR:
	printf("Invalid System Status Block address.\n");
	break;

      case INITOP_DIO_PARITY_ERR:
	printf("DIO parity error.\n");
	break;

      case INITOP_DMA_TIMEOUT_ERR:
	printf("DMA timeout.\n");
	break;

      case INITOP_DMA_PARITY_ERR:
	printf("DMA parity error.\n");
	break;

      case INITOP_DMA_BUS_ERR:
	printf("DMA bus error.\n");
	break;

      case INITOP_DMA_DATA_ERR:
	printf("DMA data error.\n");
	break;

      case INITOP_ADAPTER_CHECK_ERR:
	printf("Adapter check error.\n");
	break;

      default:
	printf("Unknown error.\n");
	break;
    }
    return(1);
}
#endif 
