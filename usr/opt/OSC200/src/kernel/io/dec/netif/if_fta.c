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
static char *rcsid = "@(#)$RCSfile: if_fta.c,v $ $Revision: 1.1.26.13 $ (DEC) $Date: 1993/12/20 21:48:10 $";
#endif
/*-----------------------------------------------------------------------
 * Modification History: 
 *
 * 27-Mar-92	Uttam Shikarpur
 * 	Submited to the pool
 *
 *---------------------------------------------------------------------- */
#include "fta.h"
/*
 * NOTE ON USE OF wbflush(): wbflush() - write buffer flush - blocks till
 * 			     buffer has been written out. It does not imply
 *			     an immediate flushing of the buffer or serve as
 *			     memory barrier operation. A wbflush is
 *			     required when we write something to the adapter
 *			     and need to read it, or a value related to the
 *			     written out variable, immediately.
 *			     This comment is applicable to the MIPS platform.
 *			     The memory barrier operation used on the ALPHA
 *			     platform aids in gauranteeing sequentiality of
 *			     operations while writing out to the hardware
 *			     registers.
 */
 /*
  * NOTE FOR MBUF USAGE ON ALPHA MACHINES:
  *	This driver while running on a alpha assumes that the mbuf cluster
  *	size is greater that FDDIMTU size. In this implementation the mbuf
  *	cluster has a size of 8K. In future if this size is reduced to
  *	less than FDDIMTU, parts of receive and transmit paths will have to be
  *	rewritten.
  */
#if NFTA > 0 || defined(BINARY)

#include <data/if_fta_data.c>

#define ADAP "fta"

extern	struct timeval	time;
extern	struct protosw *iftype_to_proto(), *iffamily_to_proto();

int	ftaattach(), ftaintr(), ftaoutput(), ftaprobe();
int	ftainit(), ftaioctl(), ftareset(), ftastatus(), ftatint();
int	fta_service_rcv_q(), fta_service_unsol_q();
int	fta_service_cmd_rsp_q(), fta_rcv_mbuf_alloc(), fta_error_recovery();
short	fta_cmd_req(), fta_poll_cmd_req();

int init_defea();	/* REMOVE ME FTW */

#ifdef SMT
int	fta_service_smt_q();
#endif /* SMT */

struct  driver ftadriver =
	{ ftaprobe, 0 ,ftaattach, 0 , 0, 0, 0,0, "fta", ftainfo };
extern  int ether_output();
u_char  fta_invalid[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

extern int fddi_units;
extern task_t first_task;

#define FTAMCLGET(m,head) { \
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
			panic("fta: CONVTOPHYS: Invalid physical address!\n"); \
		} \
}


#ifdef __alpha
#define FILLRCVDESCR(m, data_physaddr, phymbuf, rcvmbuf, mdata) { \
	(rcvmbuf) = (m);\
	CONVTOPHYS((mtod((m), u_long *)), ((data_physaddr))); \
	(phymbuf) = (*(data_physaddr));\
	mdata = (char *)(PHYS_TO_KSEG(phymbuf));\
}
/*
 * This takes a buffer from the top of the queue and puts it at
 * the end of the queue. This is used when we discard a packet
 * due to some error and want to reuse the mbufs.
 */

#define MOVE2END(top, end, bp, rcv_blk) {\
                bp[end].rmbuf = bp[top].rmbuf;\
		bp[end].phymbuf = bp[top].phymbuf;\
		bp[end].mdata = bp[top].mdata; \
		rcv_blk[end].buff_lo = (u_int)bp[top].phymbuf; \
		rcv_blk[end].long_1 |= (u_int)((bp[top].phymbuf>> 32) & PI_RCV_DESCR_M_BUFF_HI); \
		bp[top].rmbuf = NULL;\
}
#else
#define FILLRCVDESCR(m, data_physaddr, phymbuf, rcvmbuf, mdata) { \
	(rcvmbuf) = (m);\
	CONVTOPHYS((mtod((m), u_long *)), (data_physaddr)); \
	(phymbuf) = (*(data_physaddr));\
	mdata = (char *)(PHYS_TO_K0(phymbuf));\
}

#define MOVE2END(top, end, bp, rcv_blk) {\
                bp[end].rmbuf = bp[top].rmbuf;\
		bp[end].phymbuf = bp[top].phymbuf;\
		bp[end].mdata = bp[top].mdata; \
		rcv_blk[end].buff_lo = bp[top].phymbuf; \
		bp[end + 1].rmbuf = bp[top + 1].rmbuf;\
		bp[end + 1].phymbuf = bp[top + 1].phymbuf;\
		bp[end + 1].mdata = bp[top + 1].mdata;\
		rcv_blk[end + 1].buff_lo = bp[top + 1].phymbuf; \
		bp[top].rmbuf = NULL;\
		bp[top + 1].rmbuf = NULL; \
}
#endif /* __alpha */

#define printdebug printf
extern struct mbuf *mfree;
int ftadebug = 0;

/*
 * Name:	ftaprobe();
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
 *	fta_transition_state();
 *
 * Return Codes:
 *	Success:	Size of the softc structure.
 *	Failure:	0
 * 
 */
ftaprobe(reg,ctlr)
	caddr_t reg;
	struct	controller *ctlr;
{
    struct fta_softc *sc = &fta_softc[ctlr->ctlr_num];
    int unit = ctlr->ctlr_num;
    int return_status, off, i;
    extern int cpu;
#ifdef __alpha
    char buf[TC_ROMNAMLEN + 1];
    vm_offset_t io_handle;
    unsigned int slot_id;

    /*
     * 'reg' is a KSEG address in the sparse space. All our i/o data is 32 bits
     * wide.
     */
    sc->basereg = (vm_offset_t)reg;
#else
    sc->basereg = (u_long)PHYS_TO_K1((u_long)reg);
#endif /* __alpha */

    sc->pdqsw = turbosw;
    off = sc->pdqsw->mode_base;
    sc->basereg += off;
    if (cpu == DEC_2000_300) {
        slot_id = READ_BUSIO_D32((vm_offset_t)reg + BOARD_ID_OFFSET);
        if ((slot_id & DEFEA_ID_MASK) != DEFEA_ID)
            return 0;
        else
            printf("fta%d DEC DEFEA FDDI Module, Hardware revision %d\n", unit, (slot_id & DEFEA_REV_MASK) >> DEFEA_REV_SHIFT);
        sc->isa_irq = ((struct eisa_info *)(ctlr->eisainfo))->irq.intr.intr_num;
        if (sc->isa_irq != 15 && (sc->isa_irq < 9 || 11 < sc->isa_irq)) {
            printf("fta%d: illegal IRQ: %d, must be one of 9, 10, 11, or 15\n", unit, sc->isa_irq);
            return 0;
        }
    sc->basereg -= off;
    init_defea(reg, sc, &((struct eisa_info *)(ctlr->eisainfo))->phys_mem);	/* throw away when ECU arrives */
    io_handle = get_io_handle(((struct eisa_info *)(ctlr->eisainfo))->phys_mem.start_addr, 3, 0);
    sc->addr_portreset   =      (u_int *)PHYS_TO_KSEG((io_handle + (BI_KR_PORT_RESET << 6)));
    sc->addr_hostdata    =      (u_int *)PHYS_TO_KSEG((io_handle + (BI_KR_HOST_DATA << 6)));
    sc->addr_portctrl    =      (u_int *)PHYS_TO_KSEG((io_handle + (BI_KR_PORT_CTRL << 6)));
    sc->addr_portdataA   =      (u_int *)PHYS_TO_KSEG((io_handle + (BI_KR_PORT_DATA_A << 6)));
    sc->addr_portdataB   =      (u_int *)PHYS_TO_KSEG((io_handle + (BI_KR_PORT_DATA_B << 6)));
    sc->addr_portstatus  =      (u_int *)PHYS_TO_KSEG((io_handle + (BI_KR_PORT_STATUS << 6)));
    sc->addr_intrtype0   =      (u_int *)PHYS_TO_KSEG((io_handle + (BI_KR_HOST_INT_TYPE_0 << 6)));
    sc->addr_intenbX     =      (u_int *)PHYS_TO_KSEG((io_handle + (BI_KR_HOST_INT_ENB_X << 6)));
    sc->addr_type2prod   =      (u_int *)PHYS_TO_KSEG((io_handle + (BI_KR_TYPE_2_PROD << 6)));
    sc->addr_cmdrspprod  =      (u_int *)PHYS_TO_KSEG((io_handle + (BI_KR_CMD_RSP_PROD << 6)));
    sc->addr_cmdreqprod  =      (u_int *)PHYS_TO_KSEG((io_handle + (BI_KR_CMD_REQ_PROD << 6)));
    sc->addr_hostsmtprod =      (u_int *)PHYS_TO_KSEG((io_handle + (BI_KR_SMT_HOST_PROD << 6)));
    sc->addr_unsolprod   =      (u_int *)PHYS_TO_KSEG((io_handle + (BI_KR_UNSOL_PROD << 6)));
    sc->is_defea = 1;
    } else {
    /* 
     * fill out the register address
     */
    sc->addr_portreset   = 	(u_int *)(sc->basereg + BI_KR_PORT_RESET);  
    sc->addr_hostdata    = 	(u_int *)(sc->basereg + BI_KR_HOST_DATA);
    sc->addr_portctrl    =	(u_int *)(sc->basereg + BI_KR_PORT_CTRL);
    sc->addr_portdataA   = 	(u_int *)(sc->basereg + BI_KR_PORT_DATA_A);
    sc->addr_portdataB   = 	(u_int *)(sc->basereg + BI_KR_PORT_DATA_B);
    sc->addr_portstatus  = 	(u_int *)(sc->basereg + BI_KR_PORT_STATUS);
    sc->addr_intrtype0   = 	(u_int *)(sc->basereg + BI_KR_HOST_INT_TYPE_0);
    sc->addr_intenbX     = 	(u_int *)(sc->basereg + BI_KR_HOST_INT_ENB_X);
    sc->addr_type2prod   = 	(u_int *)(sc->basereg + BI_KR_TYPE_2_PROD);
    sc->addr_cmdrspprod  = 	(u_int *)(sc->basereg + BI_KR_CMD_RSP_PROD);
    sc->addr_cmdreqprod  = 	(u_int *)(sc->basereg + BI_KR_CMD_REQ_PROD);
    sc->addr_hostsmtprod = 	(u_int *)(sc->basereg + BI_KR_SMT_HOST_PROD);
    sc->addr_unsolprod   = 	(u_int *)(sc->basereg + BI_KR_UNSOL_PROD);
    sc->is_defea = 0;
    tc_addr_to_name(reg, buf);
    printf("fta%d: DEC DEFTA (PDQ) Module Name: %s\n", unit, buf);
    }

    /*
     * Initialize the CAM entries, local copies. The first two entries are
     * filled by the adapter.	
     */
    sc->is_inuse = 0; /* The # of slots we have filled up in the array.*/
    for (i = PI_CMD_ADDR_FILTER_K_FIRST; i < PI_CMD_ADDR_FILTER_K_SIZE; i++) {
	bcopy(fta_invalid, &sc->is_multi[i][0], 8);
	sc->is_muse[i] = 0;
    }

    /*
     * Initialize the directed beacon info structure. This
     * should contain no info during start up.
     */
    bzero(sc->dbeacon.db_srcaddr, 6);
    bzero(sc->dbeacon.db_unaddr, 6);

    /*
     * Initialize filters in the softc structure. We will write this
     * to the adapter once the DMA engine up and running in the PORTINIT
     * state.
     */
    /* Promiscuous mode disabled */
    sc->filters[IND_GROUP].item_code = PI_ITEM_K_IND_GROUP_PROM;
    sc->filters[IND_GROUP].value = PI_FSTATE_K_BLOCK; 
	
    /* All multicast receives disabled */
    sc->filters[GROUP].item_code = PI_ITEM_K_GROUP_PROM;
    sc->filters[GROUP].value = PI_FSTATE_K_BLOCK; 
    
    /* Broadcasts enabled. */
    sc->filters[BROADCAST].item_code = PI_ITEM_K_BROADCAST;
    sc->filters[BROADCAST].value = PI_FSTATE_K_PASS; 
    
    /* SMT Promiscuous disabled */
    sc->filters[SMT_PROM].item_code = PI_ITEM_K_SMT_PROM;
    sc->filters[SMT_PROM].value = PI_FSTATE_K_BLOCK; 
    
    /* SMT User disabled */
    sc->filters[SMT_USER].item_code = PI_ITEM_K_SMT_USER;
    sc->filters[SMT_USER].value = PI_FSTATE_K_BLOCK; 
    
    /* Reserved frames Disabled */
    sc->filters[RESERVED].item_code = PI_ITEM_K_RESERVED;
    sc->filters[RESERVED].value = PI_FSTATE_K_BLOCK;
    
    /* Implementor frames disabled */
    sc->filters[IMPLEMENTOR].item_code = PI_ITEM_K_IMPLEMENTOR;
    sc->filters[IMPLEMENTOR].value = PI_FSTATE_K_BLOCK; 
    
    /* The last item in the list  - EOL */
    sc->filters[EOL].item_code = PI_ITEM_K_EOL; 
    for (i = 0; i < NFTA; i++) {
        sc->rsp_thread_started = FALSE;
	sc->reinit_thread_started = FALSE;
    }
    ifqmaxlen_seen = 0;
    /*
     * Clear the arrays we use to manage the queues locally within
     * the driver.
     */
    fta_init_arrays(sc);

    sc->driver_state = PI_DRIVERINIT;    
    return_status = fta_transition_state(sc, unit, PI_DRIVERINIT);
    return(return_status);
}

/* 32 memory at 0xd0000
 * enable interrupts
 */
int
init_defea(slot, sc, mem)
int slot;
struct fta_softc *sc;
struct bus_mem *mem;
{
    int base_port = slot;
    char iccsr = 0;
    unsigned int start_addr, end_addr;
    unsigned char dip, mask;

    /* need to know which slot we're in */
    slot >>= EISA_SLOT_ADDR_SHIFT;

    /* enable bit, along with our IRQ */
    if (sc->isa_irq == 15)
        iccsr = 0xb;
    else
        iccsr = 8 | (sc->isa_irq - 9);

    /* register windows to set up PDQ registers in EISA memory space */
    start_addr = (mem->start_addr >> 8) & 0xfffffc;
    end_addr = ((mem->start_addr + mem->size - 1) >> 8) & 0xfffffc;
    mask = ((mem->size - 1) >> 8) & 0xfc;
    dip = ((mem->start_addr & 0x38000) >> 15) | (mem->size >> 16);

    write_io_port(base_port + 0xc84, 1, BUS_IO, 0);
    write_io_port(base_port + 0xcae, 1, BUS_IO, 0);
    write_io_port(base_port + 0x40, 1, BUS_IO, 0);
    write_io_port(base_port + 0xc8c, 1, BUS_IO, 0);
    write_io_port(base_port + 0xc8d, 1, BUS_IO, 0);
    write_io_port(base_port + 0xc90, 1, BUS_IO, 0);
    write_io_port(base_port + 0xc91, 1, BUS_IO, 0);
    write_io_port(base_port + 0xc92, 1, BUS_IO, 0);
    write_io_port(base_port + 0xc93, 1, BUS_IO, 0x40);
    write_io_port(base_port + 0xc94, 1, BUS_IO, 0);

    write_io_port(base_port + 0xc95, 1, BUS_IO, 0);
    write_io_port(base_port + 0xc96, 1, BUS_IO, 0);
    write_io_port(base_port + 0xc97, 1, BUS_IO, 0);
    write_io_port(base_port + 0xc98, 1, BUS_IO, 0);
    write_io_port(base_port + 0xc99, 1, BUS_IO, 0x3c);
    write_io_port(base_port + 0xc9a, 1, BUS_IO, 0);
    write_io_port(base_port + 0xc9b, 1, BUS_IO, 0);
    write_io_port(base_port + 0xc9c, 1, BUS_IO, 0);
    write_io_port(base_port + 0xc9d, 1, BUS_IO, 0);
    write_io_port(base_port + 0xc9e, 1, BUS_IO, 0);
    write_io_port(base_port + 0xc9f, 1, BUS_IO, 0);
    write_io_port(base_port + 0xca0, 1, BUS_IO, 0);

    write_io_port(base_port + 0xca1, 1, BUS_IO, 0x20);
    write_io_port(base_port + 0xca2, 1, BUS_IO, 0x20);
    write_io_port(base_port + 0xca3, 1, BUS_IO, 0x20);
    write_io_port(base_port + 0xca4, 1, BUS_IO, 0);
    write_io_port(base_port + 0xca5, 1, BUS_IO, 0);
    write_io_port(base_port + 0xca6, 1, BUS_IO, 0);
    write_io_port(base_port + 0xca7, 1, BUS_IO, 0);
    write_io_port(base_port + 0xcaa, 1, BUS_IO, 0x0);
    write_io_port(base_port + 0xcab, 1, BUS_IO, 0);

    write_io_port(base_port + 0xc85, 1, BUS_IO, start_addr & 0xff);
    write_io_port(base_port + 0xc86, 1, BUS_IO, (start_addr & 0xff00) >> 8);
    write_io_port(base_port + 0xc87, 1, BUS_IO, (start_addr & 0xff00) >> 16);

    write_io_port(base_port + 0xc88, 1, BUS_IO, end_addr & 0xff);
    write_io_port(base_port + 0xc89, 1, BUS_IO, (end_addr & 0xff00) >> 8);
    write_io_port(base_port + 0xc8a, 1, BUS_IO, (end_addr & 0xff0000) >> 16);

    write_io_port(base_port + 0xc8b, 1, BUS_IO, mask);

    write_io_port(base_port + 0xc8e, 1, BUS_IO, start_addr & 0xff);
    write_io_port(base_port + 0xc8f, 1, BUS_IO, (start_addr & 0xff00) >> 8);

    write_io_port(base_port + 0xca8, 1, BUS_IO, dip);

    FTAIOSYNC();
    write_io_port(base_port + 0xcae, 1, BUS_IO, 0x23);
    write_io_port(base_port + 0xc94, 1, BUS_IO, slot * 16);
    write_io_port(base_port + 0xc92, 1, BUS_IO, slot * 16);
    write_io_port(base_port + 0xc84, 1, BUS_IO, 1);
    write_io_port(base_port + 0x40, 1, BUS_IO, 0x9);
    write_io_port(base_port + 0xca9, 1, BUS_IO, iccsr);
    FTAIOSYNC();
}



/*
 * Name:	ftaattach(ctlr);
 *
 * Arguments:
 *	ctlr:	The unit number of the adapter can be got from this.
 *
 * Functional Description:
 *      At this stage we know the adpater exists. We let the upper layers
 *  know its there (put it onto the ifnet list) and do whatever other 
 *  initialization is required.
 *  The system initializes the interface when it is ready to accept packets.
 *
 *
 * Calls:
 *      fta_transition_state();
 *	fta_get_physalign_memory();
 *	fta_index_init();
 *
 * Return Codes:
 *	None. 
 */
ftaattach(ctlr)
struct controller *ctlr;
{
	struct fta_softc *sc = &fta_softc[ctlr->ctlr_num];
	struct ifnet *ifp = &sc->is_if;
	struct sockaddr_in *sin;
	short i;
	short count;
	u_char link_addr[6], *fw;
	u_int fw_val;

	/*
	 * Initialize locks.
	 */
	simple_lock_init(&sc->cmd_rsp_enb_lock);
	simple_lock_init(&sc->alloc_mbufs_lock);
	simple_lock_init(&sc->xmt_indexes_lock);
	simple_lock_init(&sc->counter_update_lock);
	lock_init(&sc->cmd_buf_q_lock, TRUE);

	sc->is_ac.ac_bcastaddr = (u_char *)etherbroadcastaddr;
	sc->is_ac.ac_arphrd = ARPHRD_ETHER ;
	ifp->if_unit = ctlr->ctlr_num;
	ifp->if_name = "fta";
	ifp->if_mtu = FDDIMTU;
	ifp->if_mediamtu = FDDIMAX;
	ifp->if_flags |= IFF_SNAP| IFF_BROADCAST|IFF_MULTICAST| IFF_NOTRAILERS | IFF_SIMPLEX ;
	ifp->if_type = IFT_FDDI;
	ifp->if_addrlen = 6; 
	ifp->if_hdrlen = sizeof(struct fddi_header) + 8 ; /* LLC header is 8 octects */
	((struct arpcom *)ifp)->ac_ipaddr.s_addr = 0;

	/*
	 * Write the maximum TRT for the ring to be reported back to
	 * the user. This is a read only value and remains constant.
	 */
	 sc->t_max = 173;

	/*
	 * Point the system to all the calls it needs to know
	 * about this driver.
	 */

	sin = (struct sockaddr_in *)&ifp->if_addr;
	sin->sin_family = AF_INET;
	ifp->if_output = ether_output;
	ifp->if_start = ftaoutput;
	ifp->if_init = ftainit;
	ifp->if_ioctl = ftaioctl;
	ifp->if_reset = ftareset;
	ifp->if_watchdog = NULL;
	ifp->if_timer = 0;
        if (sc->is_defea)
            ifp->if_sysid_type = 162;
        else
            ifp->if_sysid_type = 160;
	ifp->if_baudrate = FDDI_BANDWIDTH_100MB;


	/*
	 * Get aligned memory for the the consumer and 
	 * descriptor blocks.
	 */
	if (fta_get_physalign_memory(sc) == NULL)
		return(0);

	/*
	 * Get a memory block for counter updates. We replace the
	 * buffer we get, on the unsolicited queue, by replacing it with 
	 * the buffer in "sc->cntr_update."  This way, we don't have to 
	 * allocate memory in the Interrupt service routine. 
	 * This memory block should be
	 * physically contiguous and aligned memory of 512 bytes. But, 
	 * we can't garauntee that unless we allocate a page. 
	 * This should be fixed when VM has this capability. *SNU*
	 */

         sc->cntr_update = (u_long *)kmem_alloc(kernel_map, PAGESZ);

	 if (sc->cntr_update == NULL) {
	    printf("%s%d: Can't allocate memory.\n", ADAP, ifp->if_unit);
		return(0);
	 }

	/*
	 * Initialize the status field of the softc structure to 
	 * NULL. Initialize the counters field as well.
	 */
	sc->fta_status = NULL;
	sc->ctrblk = NULL;

    	/*
     	 * Initialize all the indexes.
     	 */
     	fta_index_init(sc);

	/*
	 * Transition state to PORTINIT. We need the DMA engine up and
	 * running. At the end of the PORTINIT state the adapter is in
	 * the DMA Available state.
	 */
	if (fta_transition_state(sc, ifp->if_unit, PI_PORTINIT) == NULL) {
	    printf("%s%d: Could not initialize network adapter.\n",
		   ADAP, ifp->if_unit);
	    return(0);
	}

	/*
	 * Initialize the number of mbufs the driver requires.
	 * NOTE: A pair of cluster mbufs is used for receiving a
	 *	 single packet on mips but a single mbuf is used in alpha.
	 */
	sc->rcv_mbuf_alloc_thread = NULL;
	sc->rcv_mbufs_required = RCV_DESCR_PROD; /* see if_fta_data.c */
	sc->rcv_mbufs_allocated = 0;
	sc->xmt_prod_thresh = XMT_PROD_THRESH;	 /* see if_fta_data.c */

	/*
	 * We know that the device has been successfully initialized at this
	 * point. Initialize if_version string.
	 */
        if (sc->is_defea)
            ifp->if_version = "DEC DEFEA (PDQ) FDDI Interface";
        else
            ifp->if_version = "DEC DEFTA (PDQ) FDDI Interface";


	/* 
	 * Hardware address using port control command 
	 * Lower word then the higer word.
	 */
	
	FTAREGWR(sc->addr_portdataA, 1); /* bits 47:32 */
	FTAIOSYNC();
	FTAREGWR(sc->addr_portctrl, PI_PCTRL_M_MLA | PI_PCTRL_M_CMD_ERROR);
	FTAIOSYNC();
	count = 0;
	while(FTAREGRD(sc->addr_portctrl) & PI_PCTRL_M_MLA) {
	    DELAY(100); 
	    count++;	
	    if (count == 10000) {
		printf("%s%d: Command timeout while reading low MLA.\n", ADAP, ifp->if_unit);
		return(0);
	    }	
	}
	if (FTAREGRD(sc->addr_portctrl) & PI_PCTRL_M_CMD_ERROR) {
	    printf("%s%d: Error while reading low MLA.\n", ADAP, ifp->if_unit);
	    return(0);
	}
	{
	    u_int hostdata;
	    u_char *data;
	    hostdata = FTAREGRD(sc->addr_hostdata);
	    data = (u_char *)(&hostdata);
	    bcopy(&data[0], &link_addr[4], 2);
	}
	FTAREGWR(sc->addr_portdataA, 0); /* bits 31:00 */
	FTAIOSYNC();
	FTAREGWR(sc->addr_portctrl, PI_PCTRL_M_MLA | PI_PCTRL_M_CMD_ERROR);
	FTAIOSYNC();
	count = 0;
	while(FTAREGRD(sc->addr_portctrl) & PI_PCTRL_M_MLA) {
	    DELAY(100); 
	    count++;	
	    if (count == 10000) {
		printf("%s%d: Command timeout while reading high MLA.\n", ADAP, ifp->if_unit);
		return(0);
	    }	
	}
	if (FTAREGRD(sc->addr_portctrl) & PI_PCTRL_M_CMD_ERROR) {
	    printf("%s%d: Error while reading high MLA.\n", ADAP, ifp->if_unit);
	    return(0);
	} 
	{
	    u_int hostdata;
	    u_char *data;
	    hostdata = FTAREGRD(sc->addr_hostdata);
	    data = (u_char *)(&hostdata);
	    bcopy(&data[0], &link_addr[0], 4);
	}
	bcopy(&link_addr[0], sc->is_addr, 6);
	bcopy(sc->is_addr, sc->is_dpaddr, 6);
	
	/*
	 * Get the firmware revision.
	 */
	FTAREGWR(sc->addr_portctrl, PI_PCTRL_M_FW_REV_READ | PI_PCTRL_M_CMD_ERROR);
	FTAIOSYNC();
	count = 0;
	while(FTAREGRD(sc->addr_portctrl) & PI_PCTRL_M_FW_REV_READ) {
	    DELAY(100); 
	    count++;	
	    if (count == 10000) {
		printf("%s%d: Command timeout while reading firware rev.\n", ADAP, ifp->if_unit);
		return(0);
	    }	
	}

	if (FTAREGRD(sc->addr_portctrl) & PI_PCTRL_M_CMD_ERROR) {
	    printf("%s%d: Error while reading low MLA.\n", ADAP, ifp->if_unit);
	    return(0);
	} 
	fw_val = FTAREGRD(sc->addr_hostdata);
	fw = (u_char *)(&fw_val);
	
	printf("%s%d: %s, Hardware address: %s , Firmware rev: %c%c%c%c \n", 
		sc->is_if.if_name, ifp->if_unit,
		ifp->if_version, ether_sprintf(sc->is_dpaddr),
		fw[3], fw[2], fw[1], fw[0]);

	attachpfilter(&(sc->is_ed));
	if_attach(ifp);
	fddi_units ++ ; 	/* 
				 * number of FDDI adapter 
				 * "fddi_units" has been defined globally in
				 * net/if_ethersubr.c
				 */
}


#ifdef PDQ_VM_HACK

struct memory_list {
    u_char *ptr;
    struct memory_list *next;
};

/*
 * Name:	fta_get_physalign_memory(sc);
 *
 * Arguments:
 *	sc:	The softc structure.
 *
 * Functional Description:
 *	This gets memory aligned to a 64 bytes boundary and 8K boundary.
 *	Currently OSF doesn't support such memory allocation schemes.
 *	Note that is memory has to be physically aligned, it is not
 *	sufficient if it virtually aligned, because virtual alignment
 *	doesn't garauntee physical alignment.
 *	Getting a 64 byte alignment:
 *	 	We get a page of memory and keep searching till we get
 *	a block that is 64 byte aligned (i.e the lower 6 bits are zero).
 *	Getting a 8K alignment:
 *		This is tougher. Each time we alloc a page of memory
 *	there is a good possibility that it is not 8K aligned, even though
 *	it might be 4K aligned(8K aligned means the lower 13 bits are all
 *	zeros). We keep allocating 2 4K pages of memory till we get a page
 *	that is 8K aligned and then we check if the next block is
 *	contiguous. If so, we free up the pages that we will not
 *	be using.
 *	It gets uglier... When we allocate an 8K chunk of virtual memory
 *	it may be contiguous in the virtual space and physical space, BUT
 *	in the physical space the 2 4K chunks are backwards. See the diagram
 *	below. 
 *	  ---------------------	
 *	 |		      |
 *       |     -------  ___   |------> -------
 *	 |     |     |	   |	       |     | 4K physical, pointing to lower
 *	 |     |8K   |	   |	       |     | half of the virtual space.
 *	 |     |vitually   |	       -------
 *	 |_____|cont-|     |---------> -------
 *	       |iguous		       |     | 4K physical, pointing to the
 *	       |     |		       |     | upper half of the virtual space.
 *	       ------		       -------
 *
 * 	So, the adapter is given the second virtual address, and we do some
 *	management to address the right space. This happens most of the time.
 *	This is a HACK. But, given the circumstances, this is the
 *	best hack I can think off. Any suggestions, improvements are 
 *	welcome.
 *	The 64 byte aligned memory has to be 64 bytes in length and the
 *	8K byte aligned memory has to have a length of 8192 bytes.
 *
 *
 * Calls:
 *	None.
 *
 * Return Codes:
 *	Success:	1
 *	Failure:	0 
 */
fta_get_physalign_memory(sc)
struct fta_softc *sc;
{

    struct ifnet *ifp = &sc->is_if;
    struct memory_list *curr_ptr, *curr_ptr_sav, *new_ptr, *first_ptr;
    u_char *tmp_ptr, *tmp_ptr2;
    short got_memory;
    u_long physaddr, physaddr2;
    int physical_mem_flipped;
    u_long mem1;
     
    first_ptr = NULL;

    while(1) {

	new_ptr = (struct memory_list *)kmem_alloc(kernel_map, 
						   sizeof(struct memory_list));
	/* NOTE: PAGESZ is defined in data/if_fta_data.c */
	tmp_ptr = (u_char *)kmem_alloc(kernel_map, PAGESZ*2);

	if (tmp_ptr == NULL) {
	    got_memory = FALSE;
	    break;
	}
	CONVTOPHYS(tmp_ptr, &physaddr);/* The first part */
	tmp_ptr2 = tmp_ptr + PAGESZ; /* The second part */
	CONVTOPHYS(tmp_ptr2, &physaddr2); /* Phys. address of the 2nd part*/
	/*
         * Note: We check for 16K alignment. If the blocks are backward
     	 *       we are garaunteed that the top page is 8K aligned. We
	 *	     use that page as the starting page for the adapter.
	 */

	if(physaddr & 0x3FFF) {
	    if(!(physaddr2 & 0x1FFF) && (physaddr2 == physaddr - PAGESZ)) {
		/*
		 * We got what we wanted.
		 */
		physical_mem_flipped = TRUE;
		goto get_memory;
	    }
	    if(!(physaddr & 0x1FFF) && (physaddr2 == physaddr + PAGESZ)) {
		/*
		 * We got what we wanted.
		 */
		physical_mem_flipped = FALSE;
		goto get_memory;
	    }
	    /* 
	     *  save up the buffers for freeing later; after we
	     *  have got a buffer that is 16K aligned.
	     */
	    if( first_ptr == NULL) {
		curr_ptr = first_ptr = new_ptr;
		curr_ptr->ptr = tmp_ptr;
		curr_ptr->next = NULL;
	    } else {
		curr_ptr->next = new_ptr;
		curr_ptr = new_ptr;
		curr_ptr->ptr = tmp_ptr;
		curr_ptr->next = NULL;
	    }
	    continue;
	} else {
	    if(physaddr2 == physaddr - PAGESZ) {
		if (physaddr2 & 0x1FFF) {
	    	    /* 
	     	     *  save up the buffers for freeing later; after we
	             *  have got a second buffer that is 8K aligned.
	             */
	            if( first_ptr == NULL) {
		        curr_ptr = first_ptr = new_ptr;
		        curr_ptr->ptr = tmp_ptr;
		        curr_ptr->next = NULL;
	            } else {
		        curr_ptr->next = new_ptr;
		        curr_ptr = new_ptr;
		        curr_ptr->ptr = tmp_ptr;
		        curr_ptr->next = NULL;
	            }

		    continue;
		} else {
		    physical_mem_flipped = TRUE;
		}
	    } else {
		physical_mem_flipped = FALSE;
	    }
       }
get_memory:	
      /*
       * we got our address free up the the memory list structure 
       */
       kmem_free(kernel_map, new_ptr, sizeof(struct memory_list));
       got_memory = TRUE;
       break;
    }
    /*
     * Free up memory that didn't meet our criteria.
     */
    curr_ptr = first_ptr;
    while (curr_ptr != NULL) {
	kmem_free(kernel_map, curr_ptr->ptr, PAGESZ);
	curr_ptr_sav = curr_ptr;
	curr_ptr = curr_ptr->next;
	kmem_free(kernel_map, curr_ptr_sav, sizeof(struct memory_list));
    }

    if (got_memory == FALSE)
	    return(0);

    tmp_ptr2 = (u_char *)(PHYS_TO_K1(physaddr2));
    tmp_ptr = (u_char *)(PHYS_TO_K1(physaddr));

    if( physical_mem_flipped == TRUE) {
	/*
	 * First the lower block virtually, but the higher block physically.
	 */
	sc->q_info.rcv_data = (PI_RCV_DESCR *)(tmp_ptr2); /* 2K */
	tmp_ptr2 = tmp_ptr2 + 2048;
	sc->q_info.xmt_data = (PI_XMT_DESCR *)(tmp_ptr2); /* 2K */
	/*
	 * Next the higher block virtually, but the lower block physically.
	 */
	sc->q_info.smt_host = (PI_RCV_DESCR *)(tmp_ptr); /* 512 */
	tmp_ptr = tmp_ptr + 512;
	sc->q_info.unsol = (PI_RCV_DESCR *)(tmp_ptr); /* 128 */
	tmp_ptr = tmp_ptr + 128;
	sc->q_info.cmd_rsp = (PI_RCV_DESCR *)(tmp_ptr); /* 128 */
	tmp_ptr = tmp_ptr + 128;
	sc->q_info.cmd_req = (PI_XMT_DESCR *)(tmp_ptr); /* 128 */
	/*
	 * Assign the physical address to be that of the higher block.
	 * NOTE: This actually corresponds to the second half of the
	 *	 virtual space. But because of the mapping done above
	 * 	 the rest of the code won't get effected.
	 */
	sc->q_info.phys_descr_blk = physaddr2;
    } else {
	sc->q_info.rcv_data = (PI_RCV_DESCR *)(tmp_ptr); /* 2K */
	tmp_ptr = tmp_ptr + 2048;
	sc->q_info.xmt_data = (PI_XMT_DESCR *)(tmp_ptr); /* 2K */
	/*
	 * Next the higher block virtually, but the lower block physically.
	 */
	sc->q_info.smt_host = (PI_RCV_DESCR *)(tmp_ptr2); /* 512 */
	tmp_ptr2 = tmp_ptr2 + 512;
	sc->q_info.unsol = (PI_RCV_DESCR *)(tmp_ptr2); /* 128 */
	tmp_ptr2 = tmp_ptr2 + 128;
	sc->q_info.cmd_rsp = (PI_RCV_DESCR *)(tmp_ptr2); /* 128 */
	tmp_ptr2 = tmp_ptr2 + 128;
	sc->q_info.cmd_req = (PI_XMT_DESCR *)(tmp_ptr2); /* 128 */
	/*
	 * Assign the physical address. 
	 */
	sc->q_info.phys_descr_blk = physaddr;
    }

    /*
     * Get the consumer block address. Walk through the page of memory
     * and find a physical space that is 64 byte aligned.
     */
    tmp_ptr = (u_char *)kmem_alloc(kernel_map, PAGESZ);
    if (tmp_ptr == NULL) {
	printf("%s%d: Can not get memory for the consumer block.\n", 
		sc->is_if.if_name, ifp->if_unit);
	return(0);
    }
    while(1) {
	CONVTOPHYS(tmp_ptr, &physaddr);
	if(physaddr & 0x003F) {
	    tmp_ptr++;
	    continue;
	} else {
	    break;
	}
    }
    mem1 = PHYS_TO_K1(physaddr);
    tmp_ptr = (u_char *)(mem1);
    sc->q_info.cons_blk = (PI_CONSUMER_BLOCK *)tmp_ptr;
    sc->q_info.phys_cons_blk = physaddr;
    return(1);
}
#else PDQ_VM_HACK
/*
 * This is used for alpha.
 */
fta_get_physalign_memory(sc)
struct fta_softc *sc;
{
    struct ifnet *ifp = &sc->is_if;
    vm_offset_t physaddr;
    u_char *mem_blk;

    mem_blk = (u_char *)kmem_alloc(kernel_map, PAGESZ);

    if (mem_blk == NULL) {
	printf("%s%d: Can not get memory for the descriptor block.\n", 
		sc->is_if.if_name, ifp->if_unit);
	return(0);
    }
    CONVTOPHYS(mem_blk, &physaddr);/* The physical address */

    if (physaddr & 0x1FFF) {
	printf("%s%d: Descriptor block not aligned to 8K.\n", 
		sc->is_if.if_name, ifp->if_unit);
	return(0);
    }
    sc->q_info.descr_blk = (PI_DESCR_BLOCK *)mem_blk;
    sc->q_info.phys_descr_blk = physaddr;

    /*
     * Get the consumer block address. Walk through the page of memory
     * and find a physical space that is 64 byte aligned.
     * NOTE: This should be changed once we have the ability to
     *       allocate contigous, aligned memory. *SNU*
     */
    mem_blk = (u_char *)kmem_alloc(kernel_map, PAGESZ);
    if (mem_blk == NULL) {
	printf("%s%d: Can not get memory for the consumer block.\n", 
		sc->is_if.if_name, ifp->if_unit);
	return(0);
    }
    while(1) {
	CONVTOPHYS(mem_blk, &physaddr);
	if(physaddr & 0x003F) {
	    mem_blk++;
	    continue;
	} else {
	    break;
	}
    }
    sc->q_info.cons_blk = (PI_CONSUMER_BLOCK *)mem_blk;
    sc->q_info.phys_cons_blk = physaddr;
    return(1);
}
#endif /* PDQ_VM_HACK */

#define ASCPRINT(reason) printf("%s%d: reason.\n", ADAP, ifp->if_unit);
/*
 * Name:	ftaintr(unit);
 *
 * Arguments:
 *	unit:	The unit number of the adapter.
 *
 * Functional Description:
 *	This is the interrupt service routine.
 *	It checks the interrupts in the following order: 1) If it was
 * an interrupt indicating a state change 2) Receive Interrupt 3) SMT
 * Receive interrupt 4) Unsolicited receive interrupt.
 * 	We do not enable 1) command request interrupt
 *	We enable xmt interrupt conditionally, and the command response
 * interrupt only after the command response thread has been started.
 *
 * Calls:	
 *	fta_print_halt_reason();
 *	fta_service_unsol_q();
 *	fta_service_rcv_q();
 *	fta_service_smt_q();
 *	fta_transition_state();
 *
 * Return Codes:
 *	None. 
 */
ftaintr(unit)
int unit;
{
    register struct fta_softc *sc = &fta_softc[unit];
    struct ifnet *ifp = &sc->is_if;
    int s;
    u_int status_reg;

    s = splimp();
    status_reg = FTAREGRD(sc->addr_portstatus);
    /*
     * Service the interrupt.
     */
    if (status_reg & PI_PSTATUS_M_TYPE_0_PENDING) {
	u_int type0_reg = FTAREGRD(sc->addr_intrtype0);
	/*
	 * Acknowledge the interrupt by writing a "1" to the
	 * appropriate bit.
	 * NOTE: CSR CMD done interrupts are ignored.
	 */

	if (type0_reg & PI_HOST_INT_0_M_STATE_CHANGE) {
	    switch((status_reg & PI_PSTATUS_M_STATE) >> PI_PSTATUS_V_STATE) {
	      case PI_STATE_K_RESET:
		ASCPRINT(Reset);
		break;
	      case PI_STATE_K_UPGRADE:
		ASCPRINT(Upgrade);
		break;
	      case PI_STATE_K_DMA_UNAVAIL:
		ASCPRINT(DMA Unavailable);
		break;
	      case PI_STATE_K_DMA_AVAIL:
		ASCPRINT(DMA Available);
		break;
	      case PI_STATE_K_LINK_AVAIL:
		ASCPRINT(Link Available);
		if (sc->driver_state == PI_OPERATIONAL)
		    ifp->if_flags |= IFF_UP;
		break;
	      case PI_STATE_K_LINK_UNAVAIL:
		ASCPRINT(Link Unavailable);
		ifp->if_flags &= ~IFF_UP;
		break;
	      case PI_STATE_K_HALTED:
		ASCPRINT(Halted);
		ifp->if_flags &= ~IFF_UP;
		fta_print_halt_reason(sc, status_reg & PI_PSTATUS_M_HALT_ID);
		/*
		 * If there is anything on the unsolicited queue,
		 * service it. This will log in information if
		 * a PC trace was received.
		 */
		fta_service_unsol_q(unit);
#ifdef __ERRLOG__
		/*
		 * The following is a grossly inefficient method of accessing
		 * the error logger. But, at this stage in the driver we
		 * don't have much of a choice.
		 */
		{
		    struct el_rec *elrp;
		    short i, j;
		    /*
		     * Issue a start command to the error log.
		     */
		    FTAREGWR((sc->addr_portctrl), 
			     PI_PCTRL_M_ERROR_LOG_START | 
			     PI_PCTRL_M_CMD_ERROR);
		    FTAIOSYNC();
		    /*
		     * Check if the command has
		     * been completed. If not
		     * check if the error bit is
		     * set.
		     */
		    i = 0;
		    while (FTAREGRD(sc->addr_portctrl) & 
			   PI_PCTRL_M_ERROR_LOG_START) {
			DELAY(1);
			i++;
			if (i == PI_PCTRL_K_TIMEOUT) {
			    printf("%s%d: Port command timeout while initializing error log.\n",
				   ADAP, ifp->if_unit);
			    goto get_out;
			}
		    }
		    if (FTAREGRD(sc->addr_portctrl) & PI_PCTRL_M_CMD_ERROR) {
			printf("%s%d: Error while initializing error log.\n",
			       ADAP, ifp->if_unit);
			goto get_out;
		    }
		    
		    /*
		     * There might have been 'N' events recorded.
		     * Each event has 512 bytes of data that needs to
		     * be dumped out. We know we have reached the
		     * end of the error log if the command fails.
		     */
		    while(1) {
			struct el_fta *elbod;
			u_int *ptr;
			u_int hostdata;

			elrp = ealloc(sizeof(struct el_fta), EL_PRILOW);
			if (elrp == NULL)
				break;
			elbod = &elrp->el_body.el_fta;
			ptr = (u_int *)elbod;
			
			/* 4 bytes read through each iteration */
			for(j = 0; j < 128; j++) {
			    FTAREGWR((sc->addr_portctrl), 
				     PI_PCTRL_M_ERROR_LOG_READ | 
				     PI_PCTRL_M_CMD_ERROR);
			    FTAIOSYNC();
			    /*
			     * Check if the command has
			     * been completed. If not
			     * check if the error bit is
			     * set.
			     */
			    i = 0;
			    while (FTAREGRD(sc->addr_portctrl) & 
				   PI_PCTRL_M_ERROR_LOG_READ) {
				DELAY(100);
				i++;
				if (i == 1000) {
				    printf("%s%d: Port command timeout while reading error log.\n",
					   ADAP, ifp->if_unit);
				    goto get_out;
				}
			    }
			    if (FTAREGRD(sc->addr_portctrl) &
				PI_PCTRL_M_CMD_ERROR) {
				printf("%s%d: Error while initializing error log.\n",
				       ADAP, ifp->if_unit);
				goto get_out;
			    }

			    /*
			     * Read data from the Host Data reg.
			     */
			    if (j < 11) {
				hostdata = FTAREGRD(sc->addr_hostdata);
				bcopy(&hostdata, ptr, 4);
				ptr++;
			    } else if (j > 10 && j < 17) {
				/*
				 * reserved data - 24 bytes.
				 */
				hostdata = FTAREGRD(sc->addr_hostdata);
				bcopy(&hostdata, ptr, 4);
				ptr++;
			    } else { /* fw_info - 111 bytes */
				hostdata = FTAREGRD(sc->addr_hostdata);
				bcopy(&hostdata, ptr, 4);
				ptr++;
			    }
			}
			/*
			 * Copy this information into the error logger.
			 */
			if (j != 0) { /* some data was made available */
			    LSUBID(elrp, 
				   ELCT_DCNTL, 
				   ELFTA, 0, 0, 
				   unit, 
				   elbod->caller_id);
			    EVALID(elrp);
			}
		    }
		}
 get_out:
#endif /* __ERRLOG__ */
				    
		/*
		 * Reset the adapter and make it go through
		 * self test.
		 */
		printf("%s%d: Reinitializing network adapter...\n",
		       ADAP, ifp->if_unit);
		FTAREGWR((sc->addr_intenbX), PI_TYPE_ALL_INT_DISABLE);
		FTAIOSYNC();
		ifp->if_flags &= ~IFF_UP;
		thread_wakeup((vm_offset_t)&sc->error_recovery_flag);
		splx(s);
		return(0);
		break;
	      case PI_STATE_K_RING_MEMBER:
		/*
		 * This state is used if the driver goes of line but
		 * the adapter remains on line, as might happen when
		 * dealing with dynamically loadable drivers. This situation
		 * is not possible in this driver.
		 */
		ASCPRINT(Ring Member);
		break;
	      default:
		ASCPRINT(Unknown State!);
		break;
	    } 
	    FTAREGWR((sc->addr_intrtype0), 
		     FTAREGRD(sc->addr_intrtype0) | 
		     PI_HOST_INT_0_M_STATE_CHANGE);
	    FTAIOSYNC();
	}

	if (type0_reg & PI_HOST_INT_0_M_XMT_DATA_FLUSH) {
	    printf("%s%d: Flushing Transmit queue.\n",
		   ADAP, ifp->if_unit);

	    while (xmt_cindex != xmt_pindex) {
		if (sc->xmt_vaddr[xmt_cindex].vaddr) {
		    m_freem(sc->xmt_vaddr[xmt_cindex].vaddr);
		    sc->xmt_vaddr[xmt_cindex].vaddr = NULL;
		}
		xmt_cindex = (xmt_cindex + 1) & NPDQXMT_MASK;
	    }
	    /*
	     * Write out the new value of the index to the adapter.
	     */
	    FTAREGWR((sc->addr_type2prod), 
		     (FTAREGRD(sc->addr_type2prod) & 
		      ~PI_TYPE_2_PROD_M_XMT_DATA_COMP)| 
		     (xmt_cindex << PI_TYPE_2_PROD_V_XMT_DATA_COMP));
	    FTAIOSYNC();
	    /*
	     * Notify the adapter that the flushing has been completed.
	     */
	    FTAREGWR((sc->addr_portctrl), 
		     PI_PCTRL_M_XMT_DATA_FLUSH_DONE |
		     PI_PCTRL_M_CMD_ERROR);
	    FTAIOSYNC();
	    FTAREGWR((sc->addr_intrtype0),
		     FTAREGRD(sc->addr_intrtype0)| 
		     PI_HOST_INT_0_M_XMT_DATA_FLUSH);
	    FTAIOSYNC();
	}

	if (type0_reg & PI_HOST_INT_0_M_NXM) {
	    /*
	     * Host accessed a non-existent location on the adapter.
	     */
	    printf("%s%d: Unknown location accessed on the network adapter.\n",
		   ADAP, ifp->if_unit);
	    thread_wakeup((vm_offset_t)&sc->error_recovery_flag);
	    FTAREGWR((sc->addr_intrtype0), 
		     FTAREGRD(sc->addr_intrtype0) | 
		     PI_HOST_INT_0_M_NXM);
	    FTAIOSYNC();
	    splx(s);
	    return(0);
	}

	if ((type0_reg & PI_HOST_INT_0_M_PM_PAR_ERR) ||
	    (type0_reg & PI_HOST_INT_0_M_BUS_PAR_ERR)) {

	    if (type0_reg & PI_HOST_INT_0_M_PM_PAR_ERR) {
		    FTAREGWR((sc->addr_intrtype0), 
			     FTAREGRD(sc->addr_intrtype0) | 
			     PI_HOST_INT_0_M_PM_PAR_ERR);
		    FTAIOSYNC();
		    printf("%s%d: Packet memory parity error.\n",
			   ADAP, ifp->if_unit);
	    }

	    if (type0_reg & PI_HOST_INT_0_M_BUS_PAR_ERR) {
		    FTAREGWR((sc->addr_intrtype0), 
			     FTAREGRD(sc->addr_intrtype0) | 
			     PI_HOST_INT_0_M_BUS_PAR_ERR);
		    FTAIOSYNC();
		    printf("%s%d: Host bus parity error.\n",
			   ADAP, ifp->if_unit);
	    }

	    /*
	     * We go into the driverinit state and bring the 
	     * driver back into operational state after initialization..
	     */
	    FTAREGWR((sc->addr_intenbX), PI_TYPE_ALL_INT_DISABLE);
	    FTAIOSYNC();
	    ifp->if_flags &= ~IFF_UP;

	    printf("%s%d: Reinitializing network adapter.\n",
		   ADAP, ifp->if_unit);	
	    if (++(sc->halt_count) > MAX_HALTS) {
		printf("%s%d: Halts threshold exceeded.\n",
		       ADAP, ifp->if_unit);	
		fta_transition_state(sc, unit, PI_BROKEN);
	        splx(s);
		return(0);
	    }
	    thread_wakeup((vm_offset_t)&sc->error_recovery_flag);
	    splx(s);
	    return(0);
	}

	if (type0_reg &  PI_HOST_INT_0_M_20MS &&
	    (FTAREGRD(sc->addr_intenbX) & PI_HOST_INT_0_M_20MS)) {
	    FTAREGWR((sc->addr_intrtype0), 
		     FTAREGRD(sc->addr_intrtype0) | 
		     PI_HOST_INT_0_M_20MS);
	    FTAIOSYNC();
	    FTAREGWR((sc->addr_intenbX), 
		     FTAREGRD(sc->addr_intenbX) & 
		     ~PI_HOST_INT_0_M_20MS);
	    FTAIOSYNC();
	    /*
	     * Enable the receive interrupt.
	     */
	    FTAREGWR((sc->addr_intenbX), 
		     FTAREGRD(sc->addr_intenbX) | 
		     PI_TYPE_X_M_RCV_DATA_ENB);
	    FTAIOSYNC();
	    fta_service_rcv_q(unit);
	}
    }

    if (status_reg & PI_PSTATUS_M_RCV_DATA_PENDING) {
	fta_service_rcv_q(unit);
    }

    if (status_reg & PI_PSTATUS_M_XMT_DATA_PENDING) {
	ftatint(unit);
    }

    if (status_reg & PI_PSTATUS_M_CMD_RSP_PENDING) {
	struct fta_softc *sc = &fta_softc[unit];
	/*
	 * This interrupt gets acknowledged by writing out the
	 * completion index. Since we will be servicing this
	 * queue later, we will disable this interrupt.
	 * Obtain a lock before disabling this.
	 */
	FTAREGWR((sc->addr_intrtype0), 
		 FTAREGRD(sc->addr_intrtype0) & 
		 ~PI_PSTATUS_M_CMD_RSP_PENDING);
	FTAIOSYNC();
	simple_lock(&sc->cmd_rsp_enb_lock);
	FTAREGWR((sc->addr_intenbX), 
		 FTAREGRD(sc->addr_intenbX) & 
		 ~PI_TYPE_X_M_CMD_RSP_ENB);
	FTAIOSYNC();
	/*
	 * Wake up the thread to process the response(s).
	 */
	thread_wakeup((vm_offset_t)sc);
	simple_unlock(&sc->cmd_rsp_enb_lock);
    }
	
    if (status_reg & PI_PSTATUS_M_UNSOL_PENDING) {
	struct fta_softc *sc =  &fta_softc[unit];
	fta_service_unsol_q(unit);
    }

#ifdef SMT
    if (status_reg & PI_PSTATUS_M_SMT_HOST_PENDING) {	    
	fta_service_smt_q(unit);
    }
#endif /* SMT */
    splx(s);
    return;
}



/*
 * Name:	ftaioctl();
 *
 * Arguments:
 *	ifp:	Pointer to the ifnet structure.
 *	cmd:	The command to execute.
 *	data:	The data associated with the command.
 *
 * Functional Description:
 *	On being issued an ioctl command this routine determines the
 *	type of request and executes, returning data in the structure
 *	provided.
 *	TODO: We currently allow only internal loop back. It should be
 *	      possible to allow both internal and external loop back.**SNU**
 * Calls:
 *	fta_cmd_req();
 *	fta_transition_state();
 *	fta_print_comp_status();
 *
 * Return Codes:
 *	
 */
ftaioctl(ifp, cmd, data)
register struct ifnet *ifp;
#ifdef __alpha
unsigned int cmd;
#else
int cmd;
#endif /* __alpha */
caddr_t data;
{
    struct fta_softc *sc = &fta_softc[ifp->if_unit];
    struct protosw *pr;
    struct ifreq *ifr = (struct ifreq *)data;
    struct ifdevea *ifd = (struct ifdevea *)data;
    register struct ifaddr *ifa = (struct ifaddr *)data;
    struct ctrreq *ctr = (struct ctrreq *)data; /* MIB counters */
    struct ifeeprom *ife = (struct ifeeprom *)data;
    struct ifchar *ifc = (struct ifchar *)data;
    u_long *rsp_buf;
    struct cmd_buf *cmdbuf = NULL;
    short retval;
    int i, k, s, return_value, count=0;
    
    
    switch (cmd) {
      case SIOCENABLBACK: {
	  PI_CMD_CHARS_SET_REQ *data;
	  /*
	   * We just enable internal loop back.
	   * External loopback is not applicable to SAS stations.
	   */
	  if(ftadebug > 1) 
		  printf("SIOCENABLBACK");

	  /*
	   * In order to do a internal loop back the adapter should be
	   * made to go through a change from link available to link
	   * unavailable and back to link available. If the it is not
	   * in link available, we just transition to link available,
	   * after issuing the loopback command.
	   */

	  if (ifp->if_flags & IFF_RUNNING) {
	      lock_write(&sc->cmd_buf_q_lock);
	      s = splimp();
	      simple_lock(&sc->cmd_rsp_enb_lock);
	      simple_lock(&sc->alloc_mbufs_lock);
	      simple_lock(&sc->xmt_indexes_lock);
	      simple_lock(&sc->counter_update_lock);

	      if (!fta_transition_state(sc, ifp->if_unit, PI_PORTINIT)) {
		  printf("%s%d: Error while transitioning to PORTINIT state.\n",
			 ADAP, ifp->if_unit);
		  simple_unlock(&sc->counter_update_lock);
		  simple_unlock(&sc->xmt_indexes_lock);
		  simple_unlock(&sc->alloc_mbufs_lock);
		  simple_unlock(&sc->cmd_rsp_enb_lock);

		  splx(s);
		  lock_done(&sc->cmd_buf_q_lock);
		  return(EIO);
	      }
	      simple_unlock(&sc->counter_update_lock);
	      simple_unlock(&sc->xmt_indexes_lock);
	      simple_unlock(&sc->alloc_mbufs_lock);
	      simple_unlock(&sc->cmd_rsp_enb_lock);
	      splx(s);
	      lock_done(&sc->cmd_buf_q_lock);
	  } else {
	      /*
	       * If the interface was not operational
	       * this is not allowed by the driver.
	       */
	      ftainit(ifp->if_unit);
	      if (fta_set_initial_chars(ifp->if_unit) == NULL) 
		      return(EIO);
	  }

	  /*
	   * Issue the loop back command.
	   * We do not use threads here. This could cause a timing
	   */
	  data = (PI_CMD_CHARS_SET_REQ *)kmem_alloc(kernel_map, 
						sizeof(PI_CMD_CHARS_SET_REQ));

	  cmdbuf = (struct cmd_buf *)kmem_alloc(kernel_map, 
						sizeof(struct cmd_buf));
	  if (cmdbuf == NULL || data == NULL) {
	      if (data)
		      kmem_free(kernel_map, 
				data, 
				sizeof(PI_CMD_CHARS_SET_REQ));
	      if (cmdbuf)
		      kmem_free(kernel_map, 
				cmdbuf, 
				sizeof(struct cmd_buf));
	      return(ENOMEM);
	  }

	  cmdbuf->timeout = TRUE; /* set to FALSE if serviced by rsp. thread */
	  data->cmd_type = PI_CMD_K_CHARS_SET;
	  data->item[0].item_code = PI_ITEM_K_LOOPBACK_MODE;
	  data->item[0].value = PI_ITEM_K_LOOPBACK_INT;
	  data->item[0].item_index = 0;
	  data->item[1].item_code = PI_ITEM_K_EOL;
	  cmdbuf->req_buf = (u_long *)(data);
	  cmdbuf->rsp_buf = NULL;
	  
	  if (sc->rsp_thread_started == TRUE) {
	      retval = fta_cmd_req(cmdbuf,
				   sc,
				   PI_CMD_K_CHARS_SET);
	  } else {
	      retval = fta_poll_cmd_req(cmdbuf,
					sc,
					PI_CMD_K_CHARS_SET);
	  }
	  if (retval == NULL) {
	      kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
	      kmem_free(kernel_map, 
			data, 
			sizeof(PI_CMD_CHARS_SET_REQ));
	      return(EIO);
	  }

	  if (cmdbuf->rsp_buf == NULL) {
	      return(EIO);
	  }

	  if (((PI_RSP_HEADER *)cmdbuf->rsp_buf)->status != PI_RSP_K_SUCCESS) {
	      /*
	       * The request could not go onto the queue or we
	       * got a bad response.
	       */
	      printf("%s%d: Enable Internal Loop Back Failed.\n",
		     ADAP, ifp->if_unit);
	      fta_print_comp_status(((PI_RSP_HEADER *)cmdbuf->rsp_buf)->status);
	      kmem_free(kernel_map, cmdbuf->rsp_buf, PAGESZ);/*SNU*/
	      kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
	      kmem_free(kernel_map, 
			data, 
			sizeof(PI_CMD_CHARS_SET_REQ));
	      return(EINVAL);
	  }
	  k = splimp();
	  sc->char_val.loopback = PI_ITEM_K_LOOPBACK_INT;
	  ifp->if_flags |= IFF_LOOPBACK;
	  splx(k);
	  kmem_free(kernel_map, cmdbuf->rsp_buf, PAGESZ);/*SNU*/
	  kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
	  kmem_free(kernel_map, 
		    data, 
		    sizeof(PI_CMD_CHARS_SET_REQ));
	  if (!fta_transition_state(sc, ifp->if_unit, PI_OPERATIONAL)) {
	      printf("%s%d: Error while transitioning to OPERATIONAL state.\n",
		     ADAP, ifp->if_unit);
	      return(EIO);
	  }
	  break;
      }

      case SIOCDISABLBACK: {
	  PI_CMD_CHARS_SET_REQ *data;

	  if(ftadebug > 1) 
		  printf("SIOCDISABLBACK");
	  /*
	   * Check if loop back is not enabled. If not just return.
	   */
	  if (!(ifp->if_flags & IFF_LOOPBACK))
		  break;

	  /*
	   * In order to do a disable internal loop back get the adapter
	   * back to DMA AVAILABLE state.
	   */
	  lock_write(&sc->cmd_buf_q_lock);
	  s = splimp();
	  simple_lock(&sc->cmd_rsp_enb_lock);
	  simple_lock(&sc->alloc_mbufs_lock);
	  simple_lock(&sc->xmt_indexes_lock);
	  simple_lock(&sc->counter_update_lock);
	  
	  if (!fta_transition_state(sc, ifp->if_unit, PI_PORTINIT)) {
	      printf("%s%d: Error while transitioning to PORTINIT state.\n",
		     ADAP, ifp->if_unit);
	      simple_unlock(&sc->counter_update_lock);
	      simple_unlock(&sc->xmt_indexes_lock);
	      simple_unlock(&sc->alloc_mbufs_lock);
	      simple_unlock(&sc->cmd_rsp_enb_lock);
	      
	      splx(s);
	      lock_done(&sc->cmd_buf_q_lock);
	      return(EIO);
	  }
	  simple_unlock(&sc->counter_update_lock);
	  simple_unlock(&sc->xmt_indexes_lock);
	  simple_unlock(&sc->alloc_mbufs_lock);
	  simple_unlock(&sc->cmd_rsp_enb_lock);
	  splx(s);
	  lock_done(&sc->cmd_buf_q_lock);

	  /*
	   * Issue the disable loop back command.
	   * We do not use threads here. This could cause a timing
	   */
	  data = (PI_CMD_CHARS_SET_REQ *)kmem_alloc(kernel_map, 
						sizeof(PI_CMD_CHARS_SET_REQ));

	  cmdbuf = (struct cmd_buf *)kmem_alloc(kernel_map, 
						sizeof(struct cmd_buf));
	  if (cmdbuf == NULL || data == NULL) {
	      if (data)
		      kmem_free(kernel_map, 
				data, 
				sizeof(PI_CMD_CHARS_SET_REQ));
	      if (cmdbuf)
		      kmem_free(kernel_map, 
				cmdbuf, 
				sizeof(struct cmd_buf));
	      return(ENOMEM);
	  }

	  cmdbuf->timeout = TRUE; /* set to FALSE if serviced by rsp. thread */
	  data->cmd_type = PI_CMD_K_CHARS_SET;
	  data->item[0].item_code = PI_ITEM_K_LOOPBACK_MODE;
	  data->item[0].value = PI_ITEM_K_LOOPBACK_NONE;
	  data->item[0].item_index = 0;
	  data->item[1].item_code = PI_ITEM_K_EOL;

	  cmdbuf->req_buf = (u_long *)(&data);
	  cmdbuf->rsp_buf = NULL;

	  if (sc->rsp_thread_started == TRUE) {
	      retval = fta_cmd_req (cmdbuf,
				    sc,
				    PI_CMD_K_CHARS_SET);
	  } else {
	      retval = fta_poll_cmd_req(cmdbuf,
					sc,
					PI_CMD_K_CHARS_SET);
	  }

	  if (retval == NULL) {
	      kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
	      kmem_free(kernel_map, 
			data, 
			sizeof(PI_CMD_CHARS_SET_REQ));
	      return(EIO);
	  }

	  if (cmdbuf->rsp_buf == NULL) {
	      return(EIO);
	  }

	  if (((PI_RSP_HEADER *)cmdbuf->rsp_buf)->status != PI_RSP_K_SUCCESS) {
	      printf("%s%d: Disable Internal Loop Back Failed.\n",
		     ADAP, ifp->if_unit);
	      fta_print_comp_status(((PI_RSP_HEADER *)cmdbuf->rsp_buf)->status);
	      kmem_free(kernel_map, cmdbuf->rsp_buf, PAGESZ);/*SNU*/
	      kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
	      kmem_free(kernel_map, 
			data, 
			sizeof(PI_CMD_CHARS_SET_REQ));
	      return(EINVAL);
	  }
	  k = splimp();
	  sc->char_val.loopback = PI_ITEM_K_LOOPBACK_NONE;
	  ifp->if_flags &= ~IFF_LOOPBACK;
	  splx(k);
	  kmem_free(kernel_map, cmdbuf->rsp_buf, PAGESZ);/*SNU*/
	  kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
	  kmem_free(kernel_map, 
		    data, 
		    sizeof(PI_CMD_CHARS_SET_REQ));
	  /*
	   * Bring it back on the network.
	   */
	  if (!fta_transition_state(sc, ifp->if_unit, PI_OPERATIONAL)) {
		  printf("%s%d: Error while transitioning to OPERATIONAL state.\n",
			 ADAP, ifp->if_unit);
		  return(EIO);
	  }
	 ifp->if_flags |= IFF_UP;
	  break;
      }
	
      case SIOCRPHYSADDR: {
	  /*
	   * Read the default hardware address.
	 */
	  bcopy(sc->is_dpaddr, ifd->default_pa, 6);
	  bcopy(sc->is_addr, ifd->current_pa, 6);
	  break;
      }	
	
      case SIOCADDMULTI:
      case SIOCSPHYSADDR:
	
	if(ftadebug > 1 )
		printf("SIOCADDMULTI || SIOCSPHYSADDR");
	/*
	 * We can not change the physical
	 * station address; we just add an entry to the CAM as an
	 * alias address.
	 */
	if (cmd == SIOCSPHYSADDR) {
		bcopy(ifr->ifr_addr.sa_data, sc->is_addr, 6);
                pfilt_newaddress(sc->is_ed.ess_enetunit, ifr->ifr_addr.sa_data);
	}

	return_value = fta_addr_filter_set(sc, 
					   PDQ_ADD_ADDR, 
					   ifr->ifr_addr.sa_data);
	/* If an IP address has been configured then an ARP packet
	 * must be broadcast to tell all hosts which currently have
	 * our address in their ARP tables to update their information.
	 */
	if (cmd == SIOCSPHYSADDR) {
		if(((struct arpcom *)ifp)->ac_ipaddr.s_addr)
			rearpwhohas((struct arpcom *)ifp,
				  &((struct arpcom *)ifp)->ac_ipaddr);
        }
	break;

      case SIOCDELMULTI:
	if(ftadebug > 1 )
		printf("SIOCDELMULTI\n");
	return_value = fta_addr_filter_set(sc, 
					   PDQ_DEL_ADDR, 
					   ifr->ifr_addr.sa_data);
	return(return_value);
	break;

      case SIOCRDZCTRS: { /* should this be allowed?? *SNU* */
	  PI_CMD_CNTRS_SET_REQ data;
	  /*
	   * Reset the counter values.
	   */
	  if (ftadebug > 1)
		  printf("SIOCRDZCTRS\n");
	  if (sc->driver_state != PI_UPGRADE) {
	      data.cmd_type = PI_CMD_K_CNTRS_SET;
	      bzero(&data.cntrs, sizeof(PI_CNTR_BLK));
	      cmdbuf = (struct cmd_buf *)kmem_alloc(kernel_map, 
						    sizeof(struct cmd_buf));
	      if (cmdbuf == NULL) {
		  return(ENOMEM);
	      }
	      cmdbuf->timeout = TRUE; /*set FALSE if serviced by rsp. thread */
	      cmdbuf->req_buf = (u_long *)(&data);
	      cmdbuf->rsp_buf = NULL;
	      retval = fta_cmd_req(cmdbuf,
				   sc,
				   PI_CMD_K_CNTRS_SET);
	      if (retval == NULL) {
		  kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
		  return(EIO);
	      }

	      /* 
	       * We get back a null if the adapter was reset even before we
	       *  were able to get a response.
	       */
	      if (cmdbuf->rsp_buf == NULL) {
		  return(EIO);
	      }

	      if (((PI_RSP_HEADER *)cmdbuf->rsp_buf)->status != PI_RSP_K_SUCCESS) {
		  printf("%s%d: Zeroing counters failed.\n",
			 ADAP, ifp->if_unit);
		  fta_print_comp_status(((PI_RSP_HEADER *)cmdbuf->rsp_buf)->status);
		  kmem_free(kernel_map, cmdbuf->rsp_buf, PAGESZ);/*SNU*/
		  kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
		  return(EINVAL);
	      }
	      kmem_free(kernel_map, cmdbuf->rsp_buf, PAGESZ);/*SNU*/
	      kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
	  } else {
	      return(EACCES);
	  }
      }

      case SIOCRDCTRS:
	/*
	 * Get the counter values.
	 */
	if (ftadebug > 1)
		printf("SIOCRDCTRS");
	if (sc->driver_state != PI_UPGRADE || 
	    sc->driver_state != PI_DRIVERINIT) {
	    switch(ctr->ctr_type) {
	      case FDDIMIB_SMT:
	      case FDDIMIB_MAC:
	      case FDDIMIB_PATH:
	      case FDDIMIB_PORT:
	      case FDDIMIB_ATTA:
		if (fta_fmib_fill(sc, ctr, ctr->ctr_type) == NULL)
			return(EIO);
		break;
	      case FDDISMT_MIB_SMT:
	      case FDDISMT_MIB_MAC:
	      case FDDISMT_MIB_PORT:
	      case FDDISMT_MIB_PATH:
		if (fta_smtmib_fill(sc, ctr, ctr->ctr_type) == NULL)
			return(EIO);
		break;
		
	      case FDDIDECEXT_MIB:
		if (fta_get_decext_mib(ifp->if_unit) != ESUCCESS)
			return(EIO);
		if (sc->dec_ext != NULL) {
		    /* copy out the structure */
		    bcopy(&sc->dec_ext->esmt_station_type, 
			  ctr->decmib_ext, 
			  sizeof(struct decext_mib));
		}
		break;

	      case FDDI_STATUS:
		return(fta_get_status(sc, &ctr->sts_fddi));
		break;
	      default:
	      case CTR_FDDI:
		ctr->ctr_type = CTR_FDDI;
		fta_get_cntrs(sc, &ctr->ctr_fddi);
		break;
	    }
	} else {
	    return(EACCES);
	}
	break;

      case SIOCSIFADDR:
	/*
	 * This brings the interface to the operational state. 
	 * If the interface is
	 * not running, it is initialized and then brought into
	 * the operational state.
	 */
        if (ftadebug > 1)
		printf("SIOCSIFADDR\n");

	if (!(ifp->if_flags & IFF_RUNNING)) {
	    ftainit(ifp->if_unit);
	    if (fta_set_initial_chars(ifp->if_unit) == NULL) 
		    return(EIO);
	}
	
	if (!(ifp->if_flags & IFF_UP)) {
	    int state;
	    state = ((FTAREGRD(sc->addr_portstatus) & PI_PSTATUS_M_STATE) 
		                                       >> PI_PSTATUS_V_STATE);
	    if (state != PI_STATE_K_LINK_AVAIL || 
		state != PI_STATE_K_LINK_UNAVAIL) {
		if(!fta_transition_state(sc, ifp->if_unit, PI_OPERATIONAL)) {
		    printf("%s%d: Error changing state to operational state\n",
			   ADAP, ifp->if_unit);
		    return(EIO);
		}
	    }
	    ifp->if_flags |= IFF_UP;
	}
	
	switch(ifa->ifa_addr->sa_family) {
	  case AF_INET:
	    ((struct arpcom *)ifp)->ac_ipaddr = IA_SIN(ifa)->sin_addr;
	    break;
	}
	break;

      case SIOCSIFFLAGS: {
	  PI_CMD_FILTERS_SET_REQ *req_buff;

	  if (ftadebug > 1)
		  printf("SIOCSIFFLAGS flag 0x%x adapter state 0x%x\n",
			 ifp->if_flags, 
		         ((FTAREGRD(sc->addr_portstatus) & PI_PSTATUS_M_STATE) 
		                                       >> PI_PSTATUS_V_STATE));
	  if (ifr->ifr_flags & IFF_UP) {
	      int state;
	      state = ((FTAREGRD(sc->addr_portstatus) & PI_PSTATUS_M_STATE) 
		                                       >> PI_PSTATUS_V_STATE);

	      if (state != PI_STATE_K_LINK_AVAIL && 
	          state != PI_STATE_K_LINK_UNAVAIL) {

		  if (!(ifp->if_flags & IFF_RUNNING)) {
		      ftainit(ifp->if_unit);
		      if (fta_set_initial_chars(ifp->if_unit) == NULL) 
			      return(EIO);
		  }
		  if(!fta_transition_state(sc, ifp->if_unit, PI_OPERATIONAL)) {
		      printf("%s%d: Error changing state to operational state\n",
			     ADAP, ifp->if_unit);
		      return(EIO);
		  }
		  ifp->if_flags |= IFF_UP;
	      }

	      /*
	       * Here we can program the adapter to:
	       * a) Receive ALL multicast addresses only (IFF_ALLMULTI)
	       * b) Receive ALL addresses (IFF_PROMISC)
	       */
	      if (ifr->ifr_flags & IFF_PROMISC) {
		  sc->filters[IND_GROUP].value = PI_FSTATE_K_PASS;
	      } else {
		  sc->filters[IND_GROUP].value = PI_FSTATE_K_BLOCK;
	      }
	      
	      if (ifr->ifr_flags & IFF_ALLMULTI) {
		  sc->filters[GROUP].value = PI_FSTATE_K_PASS; 
	      } else {
		  sc->filters[GROUP].value = PI_FSTATE_K_BLOCK;
	      }

	      req_buff = (PI_CMD_FILTERS_SET_REQ *)kmem_alloc(kernel_map, 
					       sizeof(PI_CMD_FILTERS_SET_REQ));
	      cmdbuf = (struct cmd_buf *)kmem_alloc(kernel_map, 
						    sizeof(struct cmd_buf));
	      if (cmdbuf == NULL || req_buff == NULL) {
		  if (req_buff)
			  kmem_free(kernel_map, 
				    req_buff, 
				    sizeof(PI_CMD_FILTERS_SET_REQ));
	      	  if (cmdbuf)
		          kmem_free(kernel_map, 
				    cmdbuf, 
				    sizeof(struct cmd_buf));
		  return(ENOMEM);
	      }


	      req_buff->cmd_type = PI_CMD_K_FILTERS_SET;
	      for (i = 0; i < PI_CMD_FILTERS_SET_K_ITEMS_MAX; i++) {
		  req_buff->item[i].item_code = sc->filters[i].item_code;
		  req_buff->item[i].value = sc->filters[i].value;
	      }
	      cmdbuf->timeout = TRUE; 
	      cmdbuf->req_buf = (u_long *)(req_buff);
	      cmdbuf->rsp_buf = NULL;
	      if (sc->rsp_thread_started == FALSE) {
		  retval = fta_poll_cmd_req(cmdbuf,
					    sc,
					    PI_CMD_K_FILTERS_SET);
	      } else {
		  retval = fta_cmd_req(cmdbuf,
				       sc,
				       PI_CMD_K_FILTERS_SET);
	      }		    
		  
	      if (retval == NULL) {
		  kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
		  kmem_free(kernel_map, 
			    req_buff, 
			    sizeof(PI_CMD_FILTERS_SET_REQ));
		  return(EIO);
	      }
	      
	      if (cmdbuf->rsp_buf == NULL) {
		  kmem_free(kernel_map, 
			    req_buff, 
			    sizeof(PI_CMD_FILTERS_SET_REQ));
		  return(EIO);
	      }

	      if (((PI_RSP_HEADER *)cmdbuf->rsp_buf)->status != PI_RSP_K_SUCCESS) {
		  printf("%s%d: Can not set Filters.\n",
			 ADAP, ifp->if_unit);
		  fta_print_comp_status(((PI_RSP_HEADER *)cmdbuf->rsp_buf)->status);
		  kmem_free(kernel_map, cmdbuf->rsp_buf, PAGESZ);
		  kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
		  kmem_free(kernel_map, 
			    req_buff, 
			    sizeof(PI_CMD_FILTERS_SET_REQ));
		  return(EINVAL);
	      } else {
		  if (ifr->ifr_flags & IFF_ALLMULTI) {
		      ifp->if_flags |= IFF_ALLMULTI;
		  } else {
		      ifp->if_flags &= ~IFF_ALLMULTI;
		  }
		  if (ifr->ifr_flags & IFF_PROMISC) {
		      ifp->if_flags |= IFF_PROMISC;
		  } else {
		      ifp->if_flags &= ~IFF_PROMISC;
		  }
	      }
	      kmem_free(kernel_map, cmdbuf->rsp_buf, PAGESZ);
	      kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
	      kmem_free(kernel_map, 
			req_buff, 
			sizeof(PI_CMD_FILTERS_SET_REQ));
	  } else {
	      /*
	       * Shut the interface down.
	       */
	      int state;
	      state = ((FTAREGRD(sc->addr_portstatus) & PI_PSTATUS_M_STATE) 
		                                       >> PI_PSTATUS_V_STATE);
	      if (state == PI_STATE_K_LINK_AVAIL || 
	          state == PI_STATE_K_LINK_UNAVAIL) {
		  lock_write(&sc->cmd_buf_q_lock);
		  s = splimp();
		  simple_lock(&sc->cmd_rsp_enb_lock);
		  simple_lock(&sc->alloc_mbufs_lock);
		  simple_lock(&sc->xmt_indexes_lock);
		  simple_lock(&sc->counter_update_lock);
		  
		  if (!fta_transition_state(sc, ifp->if_unit, PI_PORTINIT)) {
		      printf("%s%d: Error while transitioning to PORTINIT state.\n",
			     ADAP, ifp->if_unit);
		      simple_unlock(&sc->counter_update_lock);
		      simple_unlock(&sc->xmt_indexes_lock);
		      simple_unlock(&sc->alloc_mbufs_lock);
		      simple_unlock(&sc->cmd_rsp_enb_lock);
		      
		      splx(s);
		      lock_done(&sc->cmd_buf_q_lock);
		      return(EIO);
		  }
		  simple_unlock(&sc->counter_update_lock);
		  simple_unlock(&sc->xmt_indexes_lock);
		  simple_unlock(&sc->alloc_mbufs_lock);
		  simple_unlock(&sc->cmd_rsp_enb_lock);
		  splx(s);
		  lock_done(&sc->cmd_buf_q_lock);
	      }
	  }
	  break;
      }

      case SIOCIFRESET: { /* The adapter needs to be reset */	
	  if (ftadebug > 1)
		  printf("SIOCIFRESET\n");
	  if (fta_reinitialize(ifp, TRUE) == EIO)
		  return(EIO);
	  break;
      }

      case SIOCEEUPDATE:	/* EEPROM update */
#define SWAP_INT(x)  ((x & 0xff000000)>>24)|((x & 0x00ff0000)>>8) \
		    |((x & 0x0000ff00)<<8) | ((x & 0x000000ff)<<24)
	/*
	 * First change the state to UPGRADE, if not already
	 * in the upgrade state.
	 */
	ifp->if_flags &= ~IFF_UP;
	s = splimp();	

	if (PI_STATE_K_UPGRADE != ((FTAREGRD(sc->addr_portstatus) & 
			     PI_PSTATUS_M_STATE) >> PI_PSTATUS_V_STATE)) {
	    /*
	     * Grab all the locks.
	     * NOTE: These locks become relevant in an MP system.
	     */

	    simple_lock(&sc->cmd_rsp_enb_lock);
	    simple_lock(&sc->alloc_mbufs_lock);
	    simple_lock(&sc->xmt_indexes_lock);
	    simple_lock(&sc->counter_update_lock);
	    if (fta_transition_state(sc, ifp->if_unit, PI_UPGRADE) == NULL) {
		printf("%s%d: Could not get network adapter to upgrade state.\n",
		       ADAP, ifp->if_unit);
		simple_unlock(&sc->counter_update_lock);
		simple_unlock(&sc->xmt_indexes_lock);
		simple_unlock(&sc->alloc_mbufs_lock);
		simple_unlock(&sc->cmd_rsp_enb_lock);
		splx(s);
		return(EIO);
	    }
      
	    /*
	     * Now we can let go off the locks.
	     */
	    simple_unlock(&sc->counter_update_lock);
	    simple_unlock(&sc->xmt_indexes_lock);
	    simple_unlock(&sc->alloc_mbufs_lock);
	    simple_unlock(&sc->cmd_rsp_enb_lock);
	}
	/*
	 * Perform the upgrade 
	 */
	for (i = ife->ife_blklen, k = 0; i > 0; i = i - 8, k = k + 8) {
	    /*
	     * We copy eight bytes of data at a time into the adapter.
	     */
	    /*
	     * STEP 1: Copy longword (4 bytes) to load into PORT Data A
	     *	       and a longword into PORT Data B.
	     * NOTE: The data being copied to the adapter has to be swapped.
	     */
	    (*(u_int *)(&ife->ife_data[k])) = SWAP_INT((*(u_int *)(&ife->ife_data[k])));
	    FTAREGWR(sc->addr_portdataA, (*(u_int *)(&ife->ife_data[k])));
	    FTAIOSYNC();

	    (*(u_int *)(&ife->ife_data[k + 4])) = SWAP_INT((*(u_int *)(&ife->ife_data[k + 4])));
	    FTAREGWR(sc->addr_portdataB, (*(u_int *)(&ife->ife_data[k + 4])));
	    FTAIOSYNC();
	    /*
	     * STEP 2: Write the command CMD_COPY_DATA to the PCR
	     */
	    FTAREGWR((sc->addr_portctrl), 
		     PI_PCTRL_M_COPY_DATA | 
		     PI_PCTRL_M_CMD_ERROR);
	    FTAIOSYNC();
	    	    
	    /*
	     * STEP 3: Check for the CSR command done.
	     */
	    count = 0;
	    while(FTAREGRD(sc->addr_portctrl) & PI_PCTRL_M_COPY_DATA) {
		int state;
		DELAY(100);
		if (count++ > 1000) {
		    printf("%s%d: Port control command did not succeed.\n",
			   ADAP, ifp->if_unit);
		    if (ftadebug > 2) {
		    	state = ((FTAREGRD(sc->addr_portstatus) & 
			     PI_PSTATUS_M_STATE) >> PI_PSTATUS_V_STATE);
		    	printf("port control = 0x%X, state = %d\n", 
				*(sc->addr_portctrl), state);
		    }
		    fta_transition_state(sc, ifp->if_unit, PI_BROKEN);
		    splx(s);
		    return(EIO);
		}	
	    }
	
	    if(FTAREGRD(sc->addr_portctrl) & PI_PCTRL_M_CMD_ERROR) {
		    printf("%s%d: Error while performing upgrade.\n",
			   ADAP, ifp->if_unit);
		    fta_transition_state(sc, ifp->if_unit, PI_BROKEN);
		    splx(s);
		    return(EIO);
	    }
	}

	/*
	 * Check if this was the last block written. If so, BLAST FLASH
	 */
	if(ife->ife_lastblk == IFE_LASTBLOCK ) {
	    /* write out the total length into port data A register 
	     * This value should be provided in the ife_offset field
	     * when this is the last block being written out.
	     */
	    FTAREGWR(sc->addr_portdataA, ife->ife_offset);
	    FTAIOSYNC();
	    FTAREGWR((sc->addr_portctrl), 
		     PI_PCTRL_M_BLAST_FLASH | 
		     PI_PCTRL_M_CMD_ERROR);
	    FTAIOSYNC();
	    /*
	     * Wait atleast 90 seconds for blast FLASH 
	     */
	    count = 0;
	    while(FTAREGRD(sc->addr_portctrl) & PI_PCTRL_M_BLAST_FLASH) {
		DELAY(10000);
		if (count++ > 10000) {
		    printf("%s%d: Blast port control command did not succeed.\n",
			   ADAP, ifp->if_unit);
		    {
			int state;
		    	state = ((FTAREGRD(sc->addr_portstatus) & 
			     PI_PSTATUS_M_STATE) >> PI_PSTATUS_V_STATE);
		    	printf("port control = 0x%X, state = %d\n", 
				*(sc->addr_portctrl), state);
		    }
		    fta_transition_state(sc, ifp->if_unit, PI_BROKEN);
		    splx(s);
		    return(EIO);
		}	
	    }
	
	    if(FTAREGRD(sc->addr_portctrl) & PI_PCTRL_M_CMD_ERROR) {
		    printf("%s%d: Error while blasting the flash.\n",
			   ADAP, ifp->if_unit);
		    {
			int state;
		    	state = ((FTAREGRD(sc->addr_portstatus) & 
			     PI_PSTATUS_M_STATE) >> PI_PSTATUS_V_STATE);
		    	printf("port control = 0x%X, state = %d\n", 
				*(sc->addr_portctrl), state);
		    }
		    fta_transition_state(sc, ifp->if_unit, PI_BROKEN);
		    splx(s);
		    return(EIO);
	    }
	    printf("%s%d: Blast command succeeded. Total length blasted = %d bytes\n", ADAP, ifp->if_unit, ife->ife_offset);
	}
	splx(s);
	break;

      case SIOCIFSETCHAR:

	if (ifp->if_flags & IFF_RUNNING) {
	    int index = 0;
	    PI_CMD_CHARS_SET_REQ *char_set;
	    PI_CMD_SNMP_SET_REQ *snmp_set;
	    short tmp;
	    /*
	     * If we have all resources successfully allocated.
	     */	    

	    char_set = (PI_CMD_CHARS_SET_REQ *)kmem_alloc(kernel_map, 
						sizeof(PI_CMD_CHARS_SET_REQ));
	    if (char_set == NULL)
		return(ENOMEM);
	    
	    if (ftadebug > 1) {
	        printf("treq = %d\n", ifc->ifc_treq);
	        printf("tvx = %d\n", ifc->ifc_tvx);
	        printf("rtoken = %d\n", ifc->ifc_rtoken);
	        printf("ring purger = %d\n", ifc->ifc_ring_purger);
	        printf("count interval = %d\n", ifc->ifc_cnt_interval);
	        printf("lem = %d\n", ifc->ifc_lem);
	        printf("full_duplex_mode = %d\n", ifc->ifc_full_duplex_mode);
	    }

	    char_set->cmd_type = PI_CMD_K_CHARS_SET;
	    if ((int)(ifc->ifc_treq) > -1) {
	    	char_set->item[index].item_code = PI_ITEM_K_T_REQ;
	    	char_set->item[index].value = ifc->ifc_treq;
	    	char_set->item[index++].item_index = 0;
	    }
	    if ((int)(ifc->ifc_tvx) > -1) {
	    	char_set->item[index].item_code = PI_ITEM_K_TVX;
	    	char_set->item[index].value = ifc->ifc_tvx;
	    	char_set->item[index++].item_index = 0;
	    }
	    if ((int)(ifc->ifc_rtoken) > -1) {
	    	char_set->item[index].item_code = PI_ITEM_K_RESTRICTED_TOKEN;
	    	char_set->item[index].value = ifc->ifc_rtoken;
	    	char_set->item[index++].item_index = 0;
	    }
	    if ((int)(ifc->ifc_ring_purger) > -1) {
	    	char_set->item[index].item_code = PI_ITEM_K_RING_PURGER;
	    	char_set->item[index].value = ifc->ifc_ring_purger;
	    	char_set->item[index++].item_index = 0;
	    }
	    if ((int)(ifc->ifc_cnt_interval) > -1) {
	    	char_set->item[index].item_code = PI_ITEM_K_CNTR_INTERVAL;
	    	char_set->item[index].value = ifc->ifc_cnt_interval;
	    	char_set->item[index++].item_index = 0;
	    }
	    if ((int)(ifc->ifc_lem) > -1) {
	    	char_set->item[index].item_code = PI_ITEM_K_LEM_THRESHOLD;
	    	char_set->item[index].value = ifc->ifc_lem;
	    	char_set->item[index++].item_index = 0;
	    }

	    char_set->item[index].item_code = PI_ITEM_K_EOL;

	    cmdbuf = (struct cmd_buf *)kmem_alloc(kernel_map, 
						sizeof(struct cmd_buf));
	    if (cmdbuf == NULL) {
		kmem_free(kernel_map, char_set, sizeof(PI_CMD_CHARS_SET_REQ));
		return(ENOMEM);
	    }
	    cmdbuf->timeout = TRUE; /* set FALSE if serviced by rsp. thread */
	    cmdbuf->req_buf = (u_long *)(char_set);
	    cmdbuf->rsp_buf = NULL;

	    /*
	     * Write into the adapter.
	     */
	    retval = fta_cmd_req(cmdbuf,
				 sc,
				 PI_CMD_K_CHARS_SET);

	    if (retval == NULL) {
		kmem_free(kernel_map, char_set, sizeof(PI_CMD_CHARS_SET_REQ));
		kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
		return(EIO);
	    }


	    /* 
	     * We get back a null if the adapter was reset even before we
	     *  were able to get a response.
	     */
	    if (cmdbuf->rsp_buf == NULL) {
		kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
		kmem_free(kernel_map, char_set, sizeof(PI_CMD_CHARS_SET_REQ));
		return(EIO);
	    }

	    if (((PI_RSP_HEADER *)cmdbuf->rsp_buf)->status != PI_RSP_K_SUCCESS){
		printf("%s%d: Can not set characteristic value.\n",
		       ADAP, ifp->if_unit);
		fta_print_comp_status(((PI_RSP_HEADER *)cmdbuf->rsp_buf)->status);
		kmem_free(kernel_map, char_set, sizeof(PI_CMD_CHARS_SET_REQ));
		kmem_free(kernel_map, cmdbuf->rsp_buf, PAGESZ);/*SNU*/
		kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
		return(EINVAL);
	    }

	    /*
	     * If this was successful, update the values we have.
	     */
 	    if ((int)(ifc->ifc_treq) > -1)
 		    sc->char_val.treq = ifc->ifc_treq;
	    if ((int)(ifc->ifc_tvx) > -1)
 		    sc->char_val.tvx = ifc->ifc_tvx;
	    if ((int)(ifc->ifc_rtoken) > -1)
 		    sc->char_val.rtoken = ifc->ifc_rtoken;
	    if ((int)(ifc->ifc_ring_purger) > -1)
 		    sc->char_val.ring_purger = ifc->ifc_ring_purger;
	    if ((int)(ifc->ifc_cnt_interval) > -1)
 		    sc->char_val.cnt_interval = ifc->ifc_cnt_interval;
	    if ((int)(ifc->ifc_lem) > -1)
 		    sc->char_val.lem0 = ifc->ifc_lem;

	    kmem_free(kernel_map, char_set, sizeof(PI_CMD_CHARS_SET_REQ));
	    kmem_free(kernel_map, cmdbuf->rsp_buf, PAGESZ);/*SNU*/
	    kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));

	    /*
	     * Now check if we have to enable/disable full duplex
	     * mode of operation for the DEFTA.
	     */
	    index = 0;
	    tmp = (short)(ifc->ifc_full_duplex_mode);
	    if ((tmp == FDX_DIS || tmp == FDX_ENB)) {
		snmp_set = (PI_CMD_SNMP_SET_REQ *)kmem_alloc(kernel_map, 
						sizeof(PI_CMD_SNMP_SET_REQ));
		if (snmp_set == NULL) {
		    printf("%s%d: Could not get memory.\n",
			   ADAP, ifp->if_unit);
		    return(ENOMEM);
		}
		snmp_set->cmd_type = PI_CMD_K_SNMP_SET;
		snmp_set->item[index].item_code = PI_ITEM_K_FDX_ENB_DIS;
		snmp_set->item[index].value = tmp;
		snmp_set->item[index++].item_index = 0;

		snmp_set->item[index].item_code = PI_ITEM_K_EOL;

		cmdbuf = (struct cmd_buf *)kmem_alloc(kernel_map, 
						sizeof(struct cmd_buf));
		if (cmdbuf == NULL) {
		    kmem_free(kernel_map, snmp_set, sizeof(PI_CMD_SNMP_SET_REQ));
		    printf("%s%d: Could not get memory.\n",
			   ADAP, ifp->if_unit);
		    return(ENOMEM);
		}
		cmdbuf->timeout = TRUE; /* FALSE if serviced by rsp. thread */
		cmdbuf->req_buf = (u_long *)(snmp_set);
		cmdbuf->rsp_buf = NULL;

		
		/*
		 * Write into the adapter.
		 */
		retval = fta_cmd_req(cmdbuf,
				     sc,
				     PI_CMD_K_SNMP_SET);

		if (retval == NULL) {
		    kmem_free(kernel_map, snmp_set, sizeof(PI_CMD_SNMP_SET_REQ));
		    kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
		    sc->char_val.full_duplex_mode = FDX_DIS; /* disabled by default */
		    break;
		}

		if (cmdbuf->rsp_buf == NULL) {
		    kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
		    kmem_free(kernel_map, snmp_set, sizeof(PI_CMD_SNMP_SET_REQ));
		    break;
		}

		if (((PI_RSP_HEADER *)cmdbuf->rsp_buf)->status != PI_RSP_K_SUCCESS){
		    printf("%s%d: Can not enable/disable duplex mode.\n",
		       ADAP, ifp->if_unit);
		    fta_print_comp_status(((PI_RSP_HEADER *)cmdbuf->rsp_buf)->status);
		    kmem_free(kernel_map, snmp_set, sizeof(PI_CMD_SNMP_SET_REQ));
		    kmem_free(kernel_map, cmdbuf->rsp_buf, PAGESZ);/*SNU*/
		    kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
		    break;
		}
		if (ftadebug > 1)	
		    printf("full_duplex_mode = %d\n",tmp);
		/*
		 * If this was successful, update the values we have.
		 */
		sc->char_val.full_duplex_mode = tmp;
		kmem_free(kernel_map, snmp_set, sizeof(PI_CMD_SNMP_SET_REQ));
		kmem_free(kernel_map, cmdbuf->rsp_buf, PAGESZ);/*SNU*/
		kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
	    }
	}
	break;

	case SIOCSIPMTU:
		{
		u_short ifmtu;

		bcopy(ifr->ifr_data, (u_char *)(&ifmtu), sizeof(u_short));	

		if (ifmtu > FDDIMTU || ifmtu < IP_MINMTU)
			return(EINVAL);
		else
			ifp->if_mtu = ifmtu;
		}
	break;

	default:
		return(EINVAL);
    }
    return(ESUCCESS);
}


/*
 * Name:	ftareset();
 *
 * Arguments:
 *	unit:	The unit number.
 *
 * Functional Description:
 *	Resets the adapter.
 *
 * Calls:
 *	fta_reinitialize()
 * Return Codes:
 *
 */
ftareset(unit)
int unit;
{
    struct fta_softc *sc = &fta_softc[unit];
    struct ifnet *ifp = &sc->is_if;
    fta_reinitialize(ifp, TRUE);
}


/*
 * Name:	fta_reinitialize();
 *
 * Arguments:
 *	ifp:	The ifnet pointer.
 *	reset_only: Is it only a reset only or is it reallocating of all
 *	            resources once the reset is done? If this is for an 
 *		    upgrade this will have the value of PI_UPGRADE.
 *
 * Functional Description:
 *	This routine initializes the adapter and brings it back to the
 * DMA available state. This routine is invoked ONLY if the adapter is
 * up and is fully operational.
 *
 * Calls:
 *
 * Return Codes:
 *
 */
fta_reinitialize(ifp, reset_only)
struct ifnet *ifp;
int reset_only;
{
    struct fta_softc *sc = &fta_softc[ifp->if_unit];
    struct cmd_buf *tmp_ptr;
    int s;

    lock_write(&sc->cmd_buf_q_lock);

    s = splimp();
    /*
     * Shut off the interface.
     */
    ifp->if_flags &= ~IFF_UP;
    ifp->if_flags &= ~IFF_RUNNING;

    /*
     * Grab all the locks.
     * NOTE: These locks become relevant in an MP system.
     */

    simple_lock(&sc->cmd_rsp_enb_lock);
    simple_lock(&sc->alloc_mbufs_lock);
    simple_lock(&sc->xmt_indexes_lock);
    simple_lock(&sc->counter_update_lock);

    if (fta_transition_state(sc, ifp->if_unit, PI_DRIVERINIT) == NULL) {
	printf("%s%d: Could not initialize network adapter.\n",
	       ADAP, ifp->if_unit);
	simple_unlock(&sc->counter_update_lock);
	simple_unlock(&sc->xmt_indexes_lock);
	simple_unlock(&sc->alloc_mbufs_lock);
	simple_unlock(&sc->cmd_rsp_enb_lock);
	return(EIO);
    }

    /*
     * Now we can let go off the locks.
     */
    simple_unlock(&sc->counter_update_lock);
    simple_unlock(&sc->xmt_indexes_lock);
    simple_unlock(&sc->alloc_mbufs_lock);
    simple_unlock(&sc->cmd_rsp_enb_lock);

    /*
     * Wake up threads that might be waiting for responses.
     */
    while(sc->q_first != NULL) {
	tmp_ptr = sc->q_first;
	sc->q_first = sc->q_first->next;
	/* wake up the thread */
	tmp_ptr->timeout = FALSE;
	tmp_ptr->rsp_buf = NULL;
	thread_wakeup_one((vm_offset_t)tmp_ptr);
    }
    sc->q_first = sc->q_last = NULL;

    /*
     * Free up all buffers allocated.
     */
    fta_free_buff(ifp->if_unit);

    /*
     * Initialize all the indexes.
     */
    fta_index_init(sc);
    splx(s);
    lock_done(&sc->cmd_buf_q_lock);

    /*
     * If there is going to be an upgrade, do not get the adapter
     * into the DMA Available state.
     */
    if (reset_only == PI_UPGRADE) {
	ftanreset++;
	return(ESUCCESS);
    }

    /*
     * Transition state to PORTINIT. We need the DMA engine up and
     * running. At the end of the PORTINIT state the adapter is in
     * the DMA Available state.
     */
    if (fta_transition_state(sc, ifp->if_unit, PI_PORTINIT) == NULL) {
	printf("%s%d: Could not initialize network adapter.\n",
	       ADAP, ifp->if_unit);
	return(EIO);
    }


    
    /*
     * Initialize the arrays.
     */
    fta_init_arrays(sc);

    /*
     * we have no mbufs that have been allocated.
     * When the thread wakes up it will allocate more
     * buffers.
     */
    sc->rcv_mbufs_allocated = 0;

    if (reset_only == FALSE) {
	/*
	 * Initialize the driver with all the required buffers.
	 */
	ftainit(ifp->if_unit);
	
	/*
	 * Wake up the thread in case it is sleeping for a timeout.
	 */
	thread_wakeup((vm_offset_t)&sc->mbuf_alloc_flag);
	
	FTAREGWR((sc->addr_intenbX), 
		 FTAREGRD(sc->addr_intenbX) | 
		 PI_TYPE_X_M_CMD_RSP_ENB);
	FTAIOSYNC();
	
	/*
	 * Set all the initial characteristics.
	 */
	if (fta_set_initial_chars(ifp->if_unit) == NULL) 
		return(EIO);
    }
    ftanreset++;
    return(ESUCCESS);
}

/*
 * Name:	fta_init_arrays();
 *
 * Arguments:
 *	sc:	The softc structure.
 *
 * Functional Description:
 *	Initializes all the arrays used to manage the rcv, xmt and smt
 *	queues.
 *
 * Calls:
 *
 * Return Codes:
 *
 */
fta_init_arrays(sc)
struct fta_softc *sc;
{
    int i;

    /*
     * Initialize.
     */
    
    for (i = 0; (i < NPDQRCV ); i++) {
	sc->rcvmbuf[i].rmbuf = NULL;
	sc->rcvmbuf[i].phymbuf = 0;
	sc->rcvmbuf[i].mdata = NULL;
    }
#ifdef SMT
    for (i = 0; (i < NPDQHSMT ); i++) {
	sc->smtmbuf[i].rmbuf = NULL;
	sc->smtmbuf[i].phymbuf = 0;
	sc->smtmbuf[i].mdata = NULL;
    }
#endif /* SMT */
    /*
     * Initialize the array to keep track of transmit buffers.
     */
    for (i = 0; i < NPDQXMT; i++) {
	sc->xmt_vaddr[i].vaddr = NULL;
    }
    
}


/*
 * Name:	ftaread();
 *
 * Arguments:
 *	sc:	The softc structure.
 *  m:  The mbuf to pass onto the higher layers.
 *  len:The length of the received mbuf.
 *
 * Functional Description:
 *	This passes on the received packet, which is in the form of an mbuf
 *	to the higher layers. Before it does that, though, it moves the data
 *	pointers past the fddi header information.
 *
 * Calls:
 *	ether_input()
 *
 * Return Codes:
 *
 */
ftaread(sc, m, len)
struct fta_softc *sc;
struct mbuf *m;
int len;
{
	struct fddi_header *eptr;

	eptr = mtod(m, struct fddi_header *);
	
	/*
	 * Trim off the fddi header.
	 */
	m->m_data += sizeof (struct fddi_header);
	m->m_len -= sizeof (struct fddi_header);


	/*
	 * Subtract length of header from len
	 */
	len -= sizeof (struct fddi_header);


	m->m_pkthdr.rcvif = &sc->is_if;
	m->m_pkthdr.len = len;

	/*
	 * Bump up the DECnet counter.
	 */
	sc->is_if.if_ipackets++;
	fstc_bytercvd += len;
	if ( fstc_pdurcvd != (unsigned) 0xffffffff)
		fstc_pdurcvd++;
	
	if( eptr->fddi_dhost[0] & 1 ) {
		fstc_mbytercvd += len;
		if(fstc_mpdurcvd != 0xffffffff)
			fstc_mpdurcvd++;
	}

	/*
	 * Dispatch this packet.
	 */
   	ether_input(&(sc->is_ed), (struct ether_header *)eptr, m, 0);
}


/*
 * Name:	ftastatus();
 *
 * Arguments:
 *	unit:	The unit number.
 *
 * Functional Description:
 *	It schedules read status command which updates the fta_status 
 *	field in the softc structure.
 *
 * Calls:
 *	None.
 *
 * Return Codes:
 *	ESUCESS, ENOMEM, EIO.
 * 
 */
ftastatus(unit)
int unit;
{
    register struct fta_softc *sc = &fta_softc[unit];
    register struct ifnet *ifp = &sc->is_if;
    NODATA_CMD *req_buff;
    int s;
    struct cmd_buf *cmdbuf;
    short retval;

    /*
     * If the adapter is in the halt state we will not be able
     * to get information using the DMA engine.
     */
    if (FTAREGRD(sc->addr_portctrl) & PI_PCTRL_M_HALT) {
    	if (sc->fta_status == NULL)
	    return(EIO);
        s = splimp();
        sc->fta_status->phy_led[0] = 2;   /* Red */
        sc->fta_status->link_state = 1;   /* Off Ready */
        sc->fta_status->dup_addr_flag = 0;/* Unknown */
        sc->fta_status->purger_state = 0; /* Purger Off */
        sc->fta_status->phy_state[0] = 2; /* Off Ready */
	if (sc->dec_ext == NULL) {
	    splx(s);
	    return(EIO);
	}
	sc->dec_ext->efdx_state = FDX_DIS; 	/* disabled */
        splx(s);
        return(ESUCCESS);
    }

    req_buff = (NODATA_CMD *)kmem_alloc(kernel_map, sizeof(NODATA_CMD));

    cmdbuf = (struct cmd_buf *)kmem_alloc(kernel_map, 
					  sizeof(struct cmd_buf));
    
    if (cmdbuf == NULL || req_buff == NULL) {
	if (req_buff)
		kmem_free(kernel_map, req_buff, sizeof(NODATA_CMD));
	if (cmdbuf)
	        kmem_free(kernel_map, 
		    	  cmdbuf, 
			  sizeof(struct cmd_buf));
	return(ENOMEM);
    }

    req_buff->cmd_type = PI_CMD_K_STATUS_CHARS_GET;

    cmdbuf->timeout = TRUE; /* set to FALSE if serviced by rsp. thread */
    cmdbuf->req_buf = (u_long *)req_buff;
    cmdbuf->rsp_buf = NULL;
    if (sc->rsp_thread_started == TRUE) {
	retval = fta_cmd_req(cmdbuf,
			     sc,
			     PI_CMD_K_STATUS_CHARS_GET);
    } else { /* we poll for a response */
	retval = fta_poll_cmd_req(cmdbuf,
				  sc,
				  PI_CMD_K_STATUS_CHARS_GET);
    }
    if (retval == NULL) {
	kmem_free(kernel_map, req_buff, sizeof(NODATA_CMD));
	kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
	return(ENOMEM);
    }
	
    /* 
     * We get back a null if the adapter was reset even before we
     *  were able to get a response.
     */
    if (cmdbuf->rsp_buf == NULL || cmdbuf->timeout == TRUE) {
	    return(EIO);
    }

    if (((PI_RSP_HEADER *)cmdbuf->rsp_buf)->status != PI_RSP_K_SUCCESS) {
	printf("%s%d: ftastatus() - Can not get status characteristics\n",
	       ADAP, ifp->if_unit);
	kmem_free(kernel_map, req_buff, sizeof(NODATA_CMD));
	kmem_free(kernel_map, cmdbuf->rsp_buf, PAGESZ);/*SNU*/
	kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
	return(EIO);
    }

    /*
     * Lock the softc structure while we update it.
     */
    s = splimp();
    if (sc->fta_status != NULL)  /* this is not the first update */
	kmem_free(kernel_map, sc->fta_status, PAGESZ);/*SNU to be changedto 512*/
    sc->fta_status = (PI_CMD_STATUS_CHARS_GET_RSP *)(cmdbuf->rsp_buf);
    splx(s);
    kmem_free(kernel_map, req_buff, sizeof(NODATA_CMD));
    kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));

    /*
     * We need to get the state of the duplex mode. This is available
     * in the DEC extended mib.
     */
    if (fta_get_decext_mib(unit) != ESUCCESS)
	    printf("%s%d: Can not get DEC Extended MIB attributes.\n",
		   ADAP, ifp->if_unit);
    return(ESUCCESS);
}


/*
 * Name:	fta_get_decext_mib();
 *
 * Arguments:
 *	unit:	The unit number.
 *
 * Functional Description:
 *	This gets the values of the DEC Extended MIB attributes.
 *
 * Calls:
 *	None.
 *
 * Return Codes:
 *	ESUCESS, ENOMEM, EIO.
 * 
 */
fta_get_decext_mib(unit)
int unit;
{
    register struct fta_softc *sc = &fta_softc[unit];
    register struct ifnet *ifp = &sc->is_if;
    NODATA_CMD *ext_req_buff;
    int s;
    struct cmd_buf *ext_cmdbuf;
    short retval;

    /*
     * Get the command buffer.
     */
    ext_req_buff = (NODATA_CMD *)kmem_alloc(kernel_map, sizeof(NODATA_CMD));

    ext_cmdbuf = (struct cmd_buf *)kmem_alloc(kernel_map, 
					  sizeof(struct cmd_buf));
    
    if (ext_cmdbuf == NULL || ext_req_buff == NULL) {
	if (ext_req_buff)
		kmem_free(kernel_map, ext_req_buff, sizeof(NODATA_CMD));
	if (ext_cmdbuf)
	        kmem_free(kernel_map, 
		    	  ext_cmdbuf, 
			  sizeof(struct cmd_buf));
	return(ENOMEM);
    }

    ext_req_buff->cmd_type = PI_CMD_K_DEC_EXT_MIB_GET;

    ext_cmdbuf->timeout = TRUE; /* set to FALSE if serviced by rsp. thread */
    ext_cmdbuf->req_buf = (u_long *)ext_req_buff;
    ext_cmdbuf->rsp_buf = NULL;
    if (sc->rsp_thread_started == TRUE) {
	retval = fta_cmd_req(ext_cmdbuf,
			     sc,
			     PI_CMD_K_DEC_EXT_MIB_GET);
    } else { /* we poll for a response */
	retval = fta_poll_cmd_req(ext_cmdbuf,
				  sc,
				  PI_CMD_K_DEC_EXT_MIB_GET);
    }
    if (retval == NULL) {
	kmem_free(kernel_map, ext_req_buff, sizeof(NODATA_CMD));
	kmem_free(kernel_map, ext_cmdbuf, sizeof(struct cmd_buf));
	return(EIO);
    }
	
    /* 
     * We get back a null if the adapter was reset even before we
     *  were able to get a response.
     */
    if (ext_cmdbuf->rsp_buf == NULL || ext_cmdbuf->timeout == TRUE) {
	kmem_free(kernel_map, ext_req_buff, sizeof(NODATA_CMD));
	kmem_free(kernel_map, ext_cmdbuf, sizeof(struct cmd_buf));
	return(EIO);
    }

    if (((PI_RSP_HEADER *)ext_cmdbuf->rsp_buf)->status != PI_RSP_K_SUCCESS) {
	printf("%s%d: Can not DEC EXT MIB values\n",
	       ADAP, ifp->if_unit);
	kmem_free(kernel_map, ext_req_buff, sizeof(NODATA_CMD));
	kmem_free(kernel_map, ext_cmdbuf->rsp_buf, PAGESZ);/*SNU*/
	kmem_free(kernel_map, ext_cmdbuf, sizeof(struct cmd_buf));
	return(ESUCCESS);
    }

    /*
     * Lock the softc structure while we update it.
     */
    s = splimp();
    if (sc->dec_ext != NULL)  /* this is not the first update */
	kmem_free(kernel_map, sc->dec_ext, PAGESZ);/*SNU to be changedto 512*/
    sc->dec_ext = (PI_CMD_DEC_EXT_MIB_GET_RSP *)(ext_cmdbuf->rsp_buf);
    splx(s);
    kmem_free(kernel_map, ext_req_buff, sizeof(NODATA_CMD));
    kmem_free(kernel_map, ext_cmdbuf, sizeof(struct cmd_buf));
}

	
/*
 * Name:	fta_selftest();
 *
 * Arguments:
 *	sc:	The softc structure.
 *
 * Functional Description:
 *	This resets the adapter.	
 *
 * Calls:
 *	fta_print_halt_reason()
 *
 * Return Codes:
 *	Success:	1
 *	Failure:	0
 */
fta_selftest(sc)
struct fta_softc *sc;
{
    register struct ifnet *ifp = &sc->is_if;
    short adap_state = 0;
    short halt_reason;
    short count = 0, foo;

    /*
     * Write portdataA register.
     */
    FTAREGWR(sc->addr_portdataA, PI_PDATA_A_RESET_M_SKIP_ST);
    FTAIOSYNC();

    /*
     * Hold the LSB in the port reset register to a value of 
     * 1 for atleast 1micro second. We use a thread timeout
     * to wake us up after 2 micro seconds. The finest 
     * granularity we can get using DELAY is 1 ms, which 
     * means a lot of cycles wasted, especially when we have
     * fast clocks.
     */
    FTAREGWR(sc->addr_portreset, PI_RESET_M_ASSERT_RESET);
    FTAIOSYNC();
    sc->ztime = time.tv_sec;	/* Resetting the board will reset counters */
				/* so save the time counters were zeroed. */
    if (sc->rsp_thread_started == FALSE) {
	/* wait for a few seconds */
	DELAY(1000000);
    } else {
    	assert_wait((vm_offset_t)&foo, TRUE);
    	thread_set_timeout(hz * 2);
	thread_block();
    }
    FTAREGWR(sc->addr_portreset, 0);
    FTAIOSYNC();
    /*
     * If this was successful, then the adapter should
     * transition to DMA_Unavailable state.
     */
    while (adap_state != PI_STATE_K_DMA_UNAVAIL) {
        adap_state = ((FTAREGRD(sc->addr_portstatus) & 
    		       PI_PSTATUS_M_STATE) >> PI_PSTATUS_V_STATE);
	if (sc->rsp_thread_started== FALSE) {
	   /* check every few milli seconds */
	   DELAY(1000);
    	   count++;	
	} else {    
    	    assert_wait((vm_offset_t)&foo, TRUE);
    	    thread_set_timeout(hz * 2);
    	    thread_block();
	    count++;
	}
	if (count > 10000) { /* we couldn't initialize the adapter */
	    halt_reason = FTAREGRD(sc->addr_portstatus) & PI_PSTATUS_M_HALT_ID;
	    fta_print_halt_reason(sc, halt_reason); 
	    return(0);
    	} else {
	    continue;
        }
    }
    return(1);
}




/*
 * Name:	fta_print_halt_reason()
 *
 * Arguments:
 *	sc:	   The softc structure.
 *      reason:    The reason why the adapter is in a halt state.
 *		
 * Functional Description:
 *	Prints to the console why the adapter came to a halt or a "crashing
 *      halt".
 *
 * Calls:
 *	none.
 * 
 * Return Codes:
 *      none.
 */
fta_print_halt_reason(sc, reason)
struct fta_softc *sc;
short reason;
{
    struct ifnet *ifp = &sc->is_if;
    switch(reason) {
      case PI_HALT_ID_K_SELFTEST_TIMEOUT:
	printf("%s%d: Halt Reason (%d): Self Test Timeout.\n", ADAP, ifp->if_unit, reason);
	break;
      case PI_HALT_ID_K_PARITY_ERROR:
	printf("%s%d: Halt Reason (%d): Host bus parity error.\n", ADAP, ifp->if_unit, reason);
	break;
      case PI_HALT_ID_K_HOST_DIR_HALT:
	printf("%s%d: Halt Reason (%d): Host Directed Halt\n", ADAP, ifp->if_unit, reason);
	break;
      case PI_HALT_ID_K_SW_FAULT:
	printf("%s%d: Halt Reason (%d):Firmware Fault\n", ADAP, ifp->if_unit, reason);
	break;
      case PI_HALT_ID_K_HW_FAULT:
	printf("%s%d: Halt Reason (%d): Network Hardware Fault\n", ADAP, ifp->if_unit, reason);
	break;
      case PI_HALT_ID_K_PC_TRACE:
	printf("%s%d: Halt Reason (%d): PC Trace\n", ADAP, ifp->if_unit, reason);
	break;
      case PI_HALT_ID_K_DMA_ERROR:
 	printf("%s%d: Halt Reason (%d): DMA Error\n", ADAP, ifp->if_unit, reason);
 	break;

      case PI_HALT_ID_K_IMAGE_CRC_ERROR:
 	printf("%s%d: Halt Reason (%d): Image CRC Error\n", ADAP, ifp->if_unit, reason);
 	break;

      default:
	printf("%s%d: Halt Reason (%d): Unknown.\n", ADAP, ifp->if_unit, reason);
	break;
    }
}




/*
 * Name:	fta_print_comp_status();
 *
 * Arguments:
 *	status:	The status
 *
 * Functional Description:
 *	Prints the completion status of a command response.	
 *
 * Calls:
 *	None.
 *
 * Return Codes:
 *	None.
 */
fta_print_comp_status(status)
int status;
{
    char *s = "Completion Status ";

    switch(status) {
      case PI_RSP_K_SUCCESS:
	printf("%s(%d) : Success.\n", s, PI_RSP_K_SUCCESS);
	break;
      case PI_RSP_K_FAILURE:
	printf("%s(%d) : Failure.\n", s, PI_RSP_K_FAILURE);
	break;
      case PI_RSP_K_WARNING:
	printf("%s(%d) : Warning.\n", s, PI_RSP_K_WARNING);
	break;
      case PI_RSP_K_LOOP_MODE_BAD :
	printf("%s(%d) : Loopback Mode Invalid.\n", s, PI_RSP_K_LOOP_MODE_BAD);
	break;
      case PI_RSP_K_ITEM_CODE_BAD:
	printf("%s(%d) : Bad Item Code.\n", s, PI_RSP_K_ITEM_CODE_BAD);
	break;
      case PI_RSP_K_TVX_BAD:
	printf("%s(%d): Invalid TVX Given.\n", s, PI_RSP_K_TVX_BAD);
	break;
      case PI_RSP_K_TREQ_BAD:
	printf("%s(%d): Invalid TREQ Given.\n", s, PI_RSP_K_TREQ_BAD); 
	break;
      case PI_RSP_K_TOKEN_BAD:
	printf("%s(%d): Invalid Token.\n", s, PI_RSP_K_TOKEN_BAD);
	break;
      case PI_RSP_K_NO_EOL:
	printf("%s(%d): No EOL Given.\n", s, PI_RSP_K_NO_EOL);
	break;
      case PI_RSP_K_FILTER_STATE_BAD:
	printf("%s(%d): Invalid Filter State.\n", s, PI_RSP_K_FILTER_STATE_BAD);
	break;
      case PI_RSP_K_CMD_TYPE_BAD:
	printf("%s(%d): Invalid Command Type.\n", s, PI_RSP_K_CMD_TYPE_BAD);
	break;
      case PI_RSP_K_ADAPTER_STATE_BAD:
	printf("%s(%d): Invalid Adapter State.\n", s, PI_RSP_K_ADAPTER_STATE_BAD);
	break;
      case PI_RSP_K_RING_PURGER_BAD:
	printf("%s(%d): Invalid Ring Purger.\n", s, PI_RSP_K_RING_PURGER_BAD);
	break;
      case PI_RSP_K_LEM_THRESHOLD_BAD:
	printf("%s(%d): Invalid LEM Threshold A.\n", s, PI_RSP_K_LEM_THRESHOLD_BAD);
	break;
      case PI_RSP_K_LOOP_NOT_SUPPORTED:
	printf("%s(%d): Loopback not supported.\n", s, PI_RSP_K_LOOP_NOT_SUPPORTED);
	break;
      case PI_RSP_K_FLUSH_TIME_BAD:
 	printf("%s(%d): Bad Flush Time.\n", s, PI_RSP_K_FLUSH_TIME_BAD);
 	break;

      case PI_RSP_K_NOT_IMPLEMENTED:
 	printf("%s(%d): Not Implemented.\n", s, PI_RSP_K_NOT_IMPLEMENTED);
 	break;

      case PI_RSP_K_CONFIG_POLICY_BAD:
 	printf("%s(%d): Bad Configuration Policy.\n", s, PI_RSP_K_CONFIG_POLICY_BAD);
 	break;

      case PI_RSP_K_STATION_ACTION_BAD:
 	printf("%s(%d): Bad Station Action.\n", s, PI_RSP_K_STATION_ACTION_BAD);
 	break;

      case PI_RSP_K_MAC_ACTION_BAD:
 	printf("%s(%d): Bad MAC Action.\n", s, PI_RSP_K_MAC_ACTION_BAD);
 	break;

      case PI_RSP_K_CON_POLICIES_BAD:
 	printf("%s(%d): Bad Connection Policy.\n", s, PI_RSP_K_CON_POLICIES_BAD);
 	break;

      case PI_RSP_K_MAC_LOOP_TIME_BAD:
 	printf("%s(%d): MAC Loop Time Bad.\n", s, PI_RSP_K_MAC_LOOP_TIME_BAD);
 	break;

      case PI_RSP_K_TB_MAX_BAD:
 	printf("%s(%d): TB Max Bad.\n", s, PI_RSP_K_TB_MAX_BAD);
 	break;

      case PI_RSP_K_LER_CUTOFF_BAD:
 	printf("%s(%d): LER Cuttoff Bad.\n", s, PI_RSP_K_LER_CUTOFF_BAD);
 	break;

      case PI_RSP_K_LER_ALARM_BAD:
 	printf("%s(%d): LER Alarm Bad.\n", s, PI_RSP_K_LER_ALARM_BAD);
 	break;

      case PI_RSP_K_MAC_PATHS_REQ_BAD:
 	printf("%s(%d): MAC Paths Request Bad.\n", s, PI_RSP_K_MAC_PATHS_REQ_BAD);
 	break;

      case PI_RSP_K_MAC_T_REQ_BAD:
 	printf("%s(%d): MAC T Request Bad.\n", s, PI_RSP_K_MAC_T_REQ_BAD);
 	break;

      case PI_RSP_K_EMAC_RING_PURGER_BAD:
 	printf("%s(%d): Ring Purger Bad.\n", s, PI_RSP_K_EMAC_RING_PURGER_BAD);
 	break;

      case PI_RSP_K_EMAC_RTOKEN_TIME_BAD:
 	printf("%s(%d): Token Time Bad.\n", s, PI_RSP_K_EMAC_RTOKEN_TIME_BAD);
 	break;

      case PI_RSP_K_NO_SUCH_ENTRY:
 	printf("%s(%d): No Such Entry.\n", s, PI_RSP_K_NO_SUCH_ENTRY);
 	break;

      default:
	printf("%s(%d) : Unknown completion status.\n", s, status);
	break;
    }
}


/*
 * Name:	fta_index_init();
 *
 * Arguments:
 *	sc:	The softc structure.
 *
 * Functional Description:
 *	This initializes all the producer register for the six queues.	
 *	This routine also initializes the consumer block. The consumer
 * 	block should be initialized before the adapter transitions to the
 *	DMA Available state, at which point the consumer block becomes
 *	read only by the driver.
 *	All indicies are set to zero.
 *
 * Calls:
 *	None.
 *
 * Return Codes:
 *	None.	
 */
fta_index_init(sc)
register struct	fta_softc *sc;
{
	/*
	 * The completion and producer registers.
	 */

	xmt_cindex = 0;
	rcv_cindex =  0;
	xmt_pindex =  0;
	rcv_pindex =  0;
    	FTAREGWR((sc->addr_type2prod), 0);
	unsol_pindex = 	unsol_cindex = 0;
	FTAREGWR((sc->addr_unsolprod), 0);
	cmdreq_pindex = cmdreq_cindex = 0;
	FTAREGWR((sc->addr_cmdreqprod), 0);
	cmdrsp_pindex = cmdrsp_cindex = 0;
	FTAREGWR((sc->addr_cmdrspprod), 0);
	smt_pindex = smt_cindex = 0;
	FTAREGWR((sc->addr_hostsmtprod), 0);
	/*
	 * The consumer block.
	 */
	sc->cons_req_index = 0;
	sc->cons_rsp_index = 0;
	sc->cons_unsol_index = 0;
	sc->cons_rcv_index = 0;
	sc->cons_xmt_index = 0;
	sc->cons_hostsmt_index = 0;
	FTAIOSYNC();
	return(1);
}


/*
 * Name:	fta_transition_state()
 *
 * Arguments:
 *	sc:   The softc structure.
 *	unit: The unit # to which the above softc belongs.
 *	state:The state to go into.
 *
 * Functional Description:
 *	This changes the state of the driver, by performing certain fixed
 * functions for any given paticular state.
 *
 * Calls:
 *	fta_selftest();
 *	fta_cmd_req();
 *	fta_addr_filter_set();
 *	fta_print_comp_status();
 *
 * Return Codes:
 *	Success:	       1
 *      Failure:	       0
 * 
 */
fta_transition_state(sc,
		 unit,
		 state)
struct fta_softc *sc;
short unit;
short state;
{
    register struct ifnet *ifp = &sc->is_if;
    int to_index;
    int i;
    struct cmd_buf *cmdbuf;
    short  retval, foo;

    switch(state) {
	/*
	 * A change to this state is requested whenever the adapter
	 * needs to initialize totally.
	 */
      case PI_DRIVERINIT:

	/*
	 * perform the adapter reset. This performs an
	 * adapter self test also.
	 */

	if(fta_selftest(sc) == 0) {
		printf("%s%d: Self test failure.\n", ADAP, unit);
		return(0);
	}

	/*
	 * Disable all interrupts.
	 */
	 FTAREGWR((sc->addr_intenbX), PI_TYPE_ALL_INT_DISABLE);
	 FTAIOSYNC();

 	/*
 	 * Set the DMA burst size to 16
 	 */
 	fta_set_dma_burst_size(sc, PI_PDATA_B_DMA_BURST_SIZE_16);
	sc->prev_state = sc->driver_state;
	sc->driver_state = PI_DRIVERINIT;
	return(sizeof(struct fta_softc));
	break;

      case PI_PORTINIT: {
	  u_int status_reg;
	  short delay;
	  struct cmd_buf *q_first;
	  int count;

	  /*
	   * Disable all interrupts and clear interrupt type 0 register.
	   */
	   FTAREGWR((sc->addr_intrtype0), 0x3F);
	   FTAIOSYNC();
	   FTAREGWR((sc->addr_intrtype0), 0);
	   FTAIOSYNC();
	   FTAREGWR((sc->addr_intenbX), PI_TYPE_ALL_INT_DISABLE);
	   FTAIOSYNC();
	  /*
	   * Switch the interface off if we came here from the operational
	   * state.
	   */
	  if (sc->driver_state == PI_OPERATIONAL) {

	      /* enable state change */
	      FTAREGWR((sc->addr_intenbX), PI_HOST_INT_0_M_STATE_CHANGE);
	      FTAIOSYNC();
	      FTAREGWR((sc->addr_portdataA), PI_SUB_CMD_K_LINK_UNINIT);
	      FTAIOSYNC();

	      FTAREGWR((sc->addr_portctrl), 
		       PI_PCTRL_M_SUB_CMD | 
		       PI_PCTRL_M_CMD_ERROR);
	      FTAIOSYNC();
	      /*	
	       * Check if the CSR command done bit is set and the 
	       * command error bit is not set.
	       */
	      count = 0;
	      while(FTAREGRD(sc->addr_portctrl) & PI_PCTRL_M_SUB_CMD) {
		      assert_wait((vm_offset_t)&foo, TRUE);
		      thread_set_timeout(hz/10);
		      thread_block();
		      count++;
		      if (count > 1000) {
			      printf("%s%d: Command timeout while transitioning to DMA Available state.\n", ADAP, ifp->if_unit);
			      return(0);
		      }
	      }
	      if (FTAREGRD(sc->addr_portctrl) & PI_PCTRL_M_CMD_ERROR) {
		  printf("%s%d: Error while transitioning to DMA Available state.\n", ADAP, ifp->if_unit);
		  return(0);
	      } 
	      /*
	       * Enable the response done interrupt, because if the interface
	       * was operational, the response thread is up and running and
	       * the driver can take command response done interrupts.
	       */

	      FTAREGWR((sc->addr_intenbX), FTAREGRD(sc->addr_intenbX) |
		                           PI_HOST_INT_0_M_XMT_DATA_FLUSH |
				           PI_HOST_INT_0_M_NXM |
				           PI_HOST_INT_0_M_PM_PAR_ERR |
				           PI_HOST_INT_0_M_BUS_PAR_ERR |
				           PI_TYPE_X_M_CMD_RSP_ENB);
	      FTAIOSYNC();

	      return(1);
	  }
	  /*
	   * We now arm Type 0 interrupt.
	   * We are not yet ready to handle types 1 & 2 interrupts.
	   * FYI - Type 1 interrupt is for rcv frames on host SMT queue,
	   *	 rcv. frames on command response queue or recv. frames
	   *	 on the command request queue. Type 2 interrupt is for
	   *	 work done on the xmt and rcv data queues.
	   */
	  FTAREGWR((sc->addr_intenbX), FTAREGRD(sc->addr_intenbX) |
                                       PI_HOST_INT_0_M_STATE_CHANGE |
		                       PI_HOST_INT_0_M_XMT_DATA_FLUSH |
				       PI_HOST_INT_0_M_NXM |
				       PI_HOST_INT_0_M_PM_PAR_ERR |
				       PI_HOST_INT_0_M_BUS_PAR_ERR);
	  FTAIOSYNC();

	  /*
	   * Initialize the DMA engine.
	   */
	  /*
	   * Write out the addresses for the consumer block.
	   * The address should be aligned to a 64 byte boundary.
	   */
		  
	  FTAREGWR((sc->addr_portdataB), 0); 	/* Higher bits are NULL */

	  /* The 32 bit address */
	  FTAREGWR(sc->addr_portdataA, sc->q_info.phys_cons_blk);
	  FTAIOSYNC();

	  FTAREGWR((sc->addr_portctrl), 
		   PI_PCTRL_M_CONS_BLOCK | PI_PCTRL_M_CMD_ERROR);
	  FTAIOSYNC();

	  /*
	   * Wait for 2 seconds,max. It takes 1 second for the consumer block
	   * to be written out. We set a timer and go to sleep for 2 seconds,
	   * allowing the system to do other functions.
	   */
	  /*	
	   * Check if the CSR command done bit is set and the 
	   * command error bit is not set.
	   */
	  count = 0;
	  while(FTAREGRD(sc->addr_portctrl) & PI_PCTRL_M_CONS_BLOCK) {
	      if (sc->prev_state == PI_OPERATIONAL || sc->driver_state == PI_OPERATIONAL) {
		  assert_wait((vm_offset_t)&foo, TRUE);
		  thread_set_timeout(hz/10);
		  thread_block();
		  count++;
	      } else {
	          /* wait for sometime */
	          DELAY(100);
		  count++;
	      }
	      if (count > 1000) {
	          printf("%s%d: Command timeout while initializing consumer block!\n", ADAP, ifp->if_unit);
	          return(0);
	       }
	  }
	  if (FTAREGRD(sc->addr_portctrl) & PI_PCTRL_M_CMD_ERROR) {
	      printf("%s%d: Error while initializing the consumer block.\n", ADAP, ifp->if_unit);
	      return(0);
	  } 
	  /*
	   * Time to initialize DMA and get the adapter to the DMA
	   * available state. The descriptor block is 8K aligned in
	   * memory. 
	   */
	  FTAREGWR((sc->addr_portdataB), 0); 	/* Higher bits are NULL */
	  FTAIOSYNC();

	  /* 
	   * The 32 bit values 
	   * 31:12 The base address which is 8K aligned.
	   * 11:02 Reserved - not used.
	   * 01:01 Longword_Bswap_Data	 - set to 1
	   * 00:00 Longword_Bswap_Literal - set to 0
	   */
	  FTAREGWR((sc->addr_portdataA), 
		   sc->q_info.phys_descr_blk | 0x0000002);
	  FTAIOSYNC();

	  /*
	   * Start the DMA engine.
	   */
	  FTAREGWR((sc->addr_portctrl),
		   PI_PCTRL_M_INIT | PI_PCTRL_M_CMD_ERROR);
	  FTAIOSYNC();

	  /*	
	   * Check if the CSR command done bit is set and the command error
	   * bit is not set.
	   */
	  count = 0;
	  while(FTAREGRD(sc->addr_portctrl) & PI_PCTRL_M_INIT) {
	      if (sc->prev_state == PI_OPERATIONAL || sc->driver_state == PI_OPERATIONAL) {
		  assert_wait((vm_offset_t)&foo, TRUE);
		  thread_set_timeout(hz/10);
		  thread_block();
		  count++;
	      } else {
		  /* wait for some time */
		  DELAY(100000);
		  count++;
	      }
	      if (count == 1000) {
                  printf("%s%d: Command timeout while initializing DMA engine.\n", ADAP, ifp->if_unit);
		  return(0);
	      }
	  }
	if (FTAREGRD(sc->addr_portctrl) & PI_PCTRL_M_CMD_ERROR) {
	    printf("%s%d: Error while initializing DMA engine!\n", ADAP, ifp->if_unit);
	    return(0);
	}
	status_reg = FTAREGRD(sc->addr_portstatus);
	if (((status_reg & PI_PSTATUS_M_STATE) >> PI_PSTATUS_V_STATE) != 
						PI_STATE_K_DMA_AVAIL) {
	    printf("%s%d: Cannot change adapter state to DMA Available.\n",
		     ADAP, ifp->if_unit);
	    return(0);
	}

	  /*
	   * Change state.
	   */
	  sc->prev_state = sc->driver_state;
	  sc->driver_state = PI_PORTINIT;
	  return(1);
	  break;
      }

      case PI_OPERATIONAL: {
	  int s;
	  NODATA_CMD *req_buff;
	  thread_t thread;

	  if (sc->reinit_thread_started == FALSE) {
	      thread = kernel_thread_w_arg(first_task,
					   fta_error_recovery,
					   sc);
	      if (thread == NULL) {
		  printf("%s%d: Cannot start error recovery thread.\n",
			 ADAP, ifp->if_unit);
	      }
	      sc->reinit_thread_started = TRUE;
	  }
	
	  /*
	   * Enable interrupts for RCV Data and unsol.
	   */
	  s = splimp();
	  FTAREGWR((sc->addr_intenbX), FTAREGRD(sc->addr_intenbX) | 
		                       PI_TYPE_X_M_RCV_DATA_ENB |
				       PI_TYPE_X_M_UNSOL_ENB |
				       PI_TYPE_X_M_XMT_DATA_ENB);
	  FTAIOSYNC();
	  splx(s);
	  /*
	   * In order to make it operational, issue the start 
	   * command.
	   */

	  req_buff = (NODATA_CMD *)kmem_alloc(kernel_map, sizeof(NODATA_CMD));

	  cmdbuf = (struct cmd_buf *)kmem_alloc(kernel_map, 
						sizeof(struct cmd_buf));
	  
	  if (cmdbuf == NULL || req_buff == NULL) {
	      if (req_buff)
		      kmem_free(kernel_map, req_buff, sizeof(NODATA_CMD));
	      if (cmdbuf)
	              kmem_free(kernel_map, 
		    	        cmdbuf, 
			        sizeof(struct cmd_buf));
	      return(NULL);
	  }

	  req_buff->cmd_type = PI_CMD_K_START;
	  cmdbuf->timeout = TRUE; /*set FALSE if serviced by rsp. thread */  
	  cmdbuf->req_buf = (u_long *)(req_buff);
	  cmdbuf->rsp_buf = NULL;
	
	  if (sc->rsp_thread_started == FALSE) {
	      retval = fta_poll_cmd_req(cmdbuf,
					sc,
					PI_CMD_K_START);
	  } else {
	      retval = fta_cmd_req(cmdbuf,
				   sc,
				   PI_CMD_K_START);
	  }

	  if (retval == NULL) {
	      kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
	      kmem_free(kernel_map, req_buff, sizeof(NODATA_CMD));
	      return(NULL);
	  }

	  if (cmdbuf->rsp_buf == NULL) {
	      kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
	      kmem_free(kernel_map, req_buff, sizeof(NODATA_CMD));
	      return(NULL);
	  }

	  if (((PI_RSP_HEADER *)cmdbuf->rsp_buf)->status != PI_RSP_K_SUCCESS){
	      printf("%s%d: fta_transition_state() - Start Command Failed.\n",
			ADAP, ifp->if_unit);
	      kmem_free(kernel_map, cmdbuf->rsp_buf, PAGESZ);/*SNU*/
	      kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
	      kmem_free(kernel_map, req_buff, sizeof(NODATA_CMD));
	      return(NULL);
	  }
	  kmem_free(kernel_map, cmdbuf->rsp_buf, PAGESZ);/*SNU*/
	  kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
	  kmem_free(kernel_map, req_buff, sizeof(NODATA_CMD));
	  /*
	   * Indicate that no transmission is in progress.
	   */
	  sc->fta_debug.cmdcnt.start++;
	  sc->is_if.if_flags &= ~IFF_OACTIVE;
	  sc->prev_state = sc->driver_state;
	  sc->driver_state = PI_OPERATIONAL;
	  return(1);
      }
	
      case PI_UPGRADE: {
	  u_int status_reg = FTAREGRD(sc->addr_portstatus);
	  short adap_state = 0, count = 0;

	  /*
	   * Bring the adapter into DMA Unavailable state.
	   */
	  if (fta_reinitialize(ifp, PI_UPGRADE) != ESUCCESS)
		  return(0);
	  /*
	   * Set the upgrade bit in the port data A register.
	   */
	  FTAREGWR((sc->addr_portdataA), PI_PDATA_A_RESET_M_UPGRADE);
	  FTAIOSYNC();

	  /*
	   * Reset the adapter.
	   * Hold the LSB in the port reset register to a value of 
	   * 1 for atleast 1micro second.
	   */
	  FTAREGWR((sc->addr_portreset), PI_RESET_M_ASSERT_RESET);
	  FTAIOSYNC();
	  DELAY(10000);
	  FTAREGWR((sc->addr_portreset), 0); /* reset the bit */
	  FTAIOSYNC();

	  while (adap_state != PI_STATE_K_UPGRADE) {
	      adap_state = ((FTAREGRD(sc->addr_portstatus) & 
			     PI_PSTATUS_M_STATE) >> PI_PSTATUS_V_STATE);
	      DELAY(1000);
	      count++;
	      if (count > 100000) { 
		  return(0);
	      } else {
		  continue;
	      }
	  }
	  sc->prev_state = sc->driver_state;
	  sc->driver_state = PI_UPGRADE;
	  return(1);
	  break;
      }

      case PI_BROKEN: {
	  /*
	   * Disable all interrupts.
	   */
	  FTAREGWR((sc->addr_intenbX), PI_TYPE_ALL_INT_DISABLE);
	  FTAIOSYNC();
	  sc->is_if.if_flags &= ~IFF_OACTIVE | ~IFF_UP | ~IFF_RUNNING;
	  printf("%s%d: Driver disabled. Check the network adapter.\n",
			ADAP, ifp->if_unit);
	  sc->prev_state = sc->driver_state;
	  sc->driver_state = PI_BROKEN;
	  break;
      }
    }
}


/*
 * Name:	ftainit()
 *
 * Arguments:
 *	unit:	The unit number of the adapter.
 *			
 * Functional Description:
 *    	This routine is used for initializing all descriptors on all
 *	queues.
 *
 * Calls:
 * 	None.
 *
 * Return Code:
 *	Success:	1
 *	Failure:	0
 */
ftainit(unit)
int unit;
{
    struct fta_softc *sc = &fta_softc[unit];
    struct ifnet *ifp = &sc->is_if;

    /*
     * Initialize descriptors for :
     * 1) XMT data queue 
     * 2) RCV data queue
     * 3) Command request queue
     * 4) Command response queue
     * 5) Unsolicited queue
     * 6) Host SMT queue
     *
     * "fta_rcv_descr_init()" initializes descriptors of all the receive
     * queues - rcv data queue, command response queue, unsolicited
     * queue and smt host queue.
     *
     * "fta_xmt_descr_init()" initializes descriptors for the command
     * request and transmit data queues.
     * The command response queue and the unolicited queue point to 
     * 512 byte buffers. 
     * The receive data and SMT host descriptors point to 8K byte buffer 
     * segmented into 2 halves.
     * NOTE1: In the command response queue the buffer size is zero.
     *      Reason: We do not use interrupts to service the command 
     *              response queue. i.e. when a command needs to be
     *              serviced we place a request on the command request
     *              queue and then poll for the response on the command
     *              response queue. Thus, we will allocate buffers as
     *              and when required. This scheme was chosen in order
     *              to avoid the overhead of maintaining driver queues
     *              which will map onto each of the request sent out
     *              so as to be able to pair the response as they come
     *              back up from the adapter. Given the infrequent 
     *              command request when compared to the frequencies
     *              on the other queues, this trade off seems to be worth
     *              while.
     */


    /* command response queue descriptors */
    fta_rcv_descr_init(sc,
		       CMDRSP_Q);
	  
    /* unsolicited queue descriptors */
    fta_rcv_descr_init(sc,
		       UNSOL_Q);
	  
#ifdef SMT	  
			     
    /* SMT Host queue descriptors */
    fta_rcv_descr_init(sc,
		       SMT_Q);
#endif /* SMT */
    /* Data receive queue descriptors */
    fta_rcv_descr_init(sc,
		       RCV_Q);
	  
    /* 
     * We  initialize command request queue.
     * Note, that we do not initialize the transmit queue
     * here. This is done as needed before transmitting packets
     * because we can estimate the number of descriptors we will
     * need per packet then.
     */
	  
    /* The command request queue */
    fta_xmt_descr_init(sc->cmd_req_blk, NPDQCREQ);

    /* Allocate the buffers */
    fta_alloc_buffs(unit);
    ifp->if_flags |= IFF_RUNNING;

    /* If ztime is zero, then it was reset prior to clock starting. */
    /* We now give it a valid value. */
    if(!sc->ztime)
    	sc->ztime = time.tv_sec;
}


/*
 * Name:	fta_alloc_buffs()
 *
 * Arguments:
 *	unit:	The unit number of the adapter.
 *
 * Functional Description:
 *	This allocates the buffers required by the driver. This gets
 *	called by the system during startup and then subsequently from
 *	within driver whenever the adapter needs to be reset.
 *
 * Calls:
 *	fta_rcv_buff_produce();
 *
 * Return Codes:
 *	None.	
 */
fta_alloc_buffs(unit)
short unit;
{
    struct fta_softc *sc = &fta_softc[unit];
    struct ifnet *ifp = &sc->is_if;
    int to_index;
    thread_t thread;

    /*
     * Produce buffers.
     */
    fta_rcv_buff_produce(sc,
			 NULL,
			 UNSOL_Q);
    to_index = sc->rcv_mbufs_required - 1;
    if (sc->rcv_mbuf_alloc_thread == NULL) {
	fta_rcv_buff_produce(sc,
			     &to_index,
			     RCV_Q);
    }
    fta_rcv_buff_produce(sc, 
                         NULL,
		         CMDRSP_Q);
#ifdef SMT	  
    to_index = SMT_DESCR_PROD - 1;
    fta_rcv_buff_produce(sc,
			 &to_index, 
			 SMT_Q);
#endif /* SMT */
    /*
     * Start up the thread to process the command response 
     * queues. We have one thread per interface. The third
     * argument is the argument that is passed when the thread
     * is woken up. We use this to identify which interface we
     * are to service.
     */

    if (sc->rsp_thread_started == FALSE) {
	thread = kernel_thread_w_arg(first_task,
				     fta_service_cmd_rsp_q,
				     sc);
	if (thread == NULL) {
	    printf("%s%d: Cannot start the response queue thread.\n",
		   ADAP, ifp->if_unit);
	    sc->rsp_thread_started = FALSE;
	} else {
	    sc->rsp_thread_started = TRUE;
	    sc->cmd_rsp_thread = thread;
	}
    }

    if (sc->rcv_mbuf_alloc_thread == NULL) {
	thread = kernel_thread_w_arg(first_task,
				     fta_rcv_mbuf_alloc,
				     sc);
	if (thread == NULL) {
	    printf("%s%d: Cannot start rcv. mbuf allocation thread.\n",
		   ADAP, ifp->if_unit);
	}
	sc->rcv_mbuf_alloc_thread = thread;
    }
	
    /*
     * Enable the command response done interrupt.
     */
    FTAREGWR((sc->addr_intenbX), FTAREGRD(sc->addr_intenbX) |
	                         PI_TYPE_X_M_CMD_RSP_ENB);
    FTAIOSYNC();

}

/*
 * Name:	fta_error_recovery()
 *
 * Arguments:
 *	None.
 *
 * Functional Description:
 *	This is a thread that gets started up when the adapter becomes
 *	operational. 
 *	This thread is responsible for resetting the adapter in the event
 *	of a "fatal" error. A fatal error requires restting the adapter.
 *	Such an error is discovered during an interrrupt. While resetting
 *	adapter it is required to block and since it is illegal to block
 *	during an interrupt routine this thread is woken up to do the
 *	reset.
 *
 * Calls:
 *	fta_reinitialize()
 *
 * Return Codes:
 *	None.	
 */
fta_error_recovery()
{
    thread_t thread;
    struct fta_softc *sc;
    struct ifnet *ifp;

    thread = current_thread();

    /*
     * Collect the argument left by the kernel_thread_w_arg().
     */
    sc = (struct fta_softc *)thread->reply_port;
    ifp = &sc->is_if;
    thread->reply_port = PORT_NULL;

    for(;;) {
	assert_wait((vm_offset_t)&sc->error_recovery_flag, TRUE);
	thread_block();
	fta_reinitialize(ifp, FALSE);
	if (fta_transition_state(sc, ifp->if_unit, PI_OPERATIONAL) == NULL) {
		printf("%s%d: Could not get the adapter on line.\n",
	       			ADAP, ifp->if_unit);
		fta_transition_state(sc, ifp->if_unit, PI_BROKEN);
		continue;
    	}
	ifp->if_flags |= IFF_UP;
   }
}

#define SEG_LEN_DIVISOR	0x000080	/* 128 bytes to be used for dividing
					 */
/*
 * Name:	fta_rcv_descr_init();
 *
 * Arguments:
 *	sc:	The softc for the adapter unit.
 *	q_type: The type queue we are going to initialize.
 *
 * Functional Description:
 *	This initializes all the receive descriptor for the  command rsp,
 *	data rcv, unsolicited and host SMT queues.
 *
 *	After being initialized free receive buffers are produced.
 *
 * Calls:
 *	None.
 *
 * Return Codes:
 *	None.	
 */
fta_rcv_descr_init(sc,
		   q_type)
register struct fta_softc *sc;
register short q_type;
{
    PI_RCV_DESCR *rcv_blk;
    int buff_size;
    short seg_count;
    short ring_size;
    short entry;
    
    switch(q_type) {
      case CMDRSP_Q:
	rcv_blk = sc->cmd_rsp_blk;
	buff_size = RSP_BUFSIZ;
	seg_count = PDQ_HOST_SEG_VALUE_1;
	ring_size = NPDQCRSP;
	break;
      case UNSOL_Q:
	rcv_blk = sc->unsol_blk;
	buff_size = UNSOL_BUFSIZ;
	seg_count = PDQ_HOST_SEG_VALUE_1;
	ring_size = NPDQUNSOL;
	break;
      case RCV_Q:
	rcv_blk = sc->rcv_data_blk;
	buff_size = RCV_BUFSIZ;
#ifdef __alpha
	seg_count = PDQ_HOST_SEG_VALUE_1;
#else
	seg_count = PDQ_HOST_SEG_VALUE_2;
#endif /* __alpha */
	ring_size = NPDQRCV;
	break;
#ifdef SMT
      case SMT_Q:
	rcv_blk = sc->smt_data_blk;
	buff_size = SMT_BUFSIZ;
#ifdef __alpha
	seg_count = PDQ_HOST_SEG_VALUE_1;
#else
	seg_count = PDQ_HOST_SEG_VALUE_2;
#endif /* __alpha */
	ring_size = NPDQHSMT;
	break;
#endif /* SMT */

    }
    
    switch(q_type) {
      case UNSOL_Q:
      case CMDRSP_Q:
	/*
	 * Initialize the descriptors. We use only one segment
	 * per response.
	 */
	for (entry = 0; entry < ring_size; entry++) {
	    /* 
	     * The higher bits first - longword 1;
	     */
	    rcv_blk[entry].long_1 = 0; /* initialize */
	    rcv_blk[entry].long_1 = PI_RCV_DESCR_M_SOP |
		    ((buff_size/SEG_LEN_DIVISOR)
		     << PI_RCV_DESCR_V_SEG_LEN) |
			     (seg_count << PI_RCV_DESCR_V_SEG_CNT);
	}
	break;
      case RCV_Q:
#ifdef SMT
      case SMT_Q:
#endif /* SMT */
	/*
	 * Initialize the descriptors. We use two segments per
	 * response. Each segment is 4K bytes wide. Two contiguous
	 * descriptors are used to read in a packet which can have
	 * at most 4352 bytes of data.
	 * For alpha we use only ONE 8K buffer.
	 */
	for (entry = 0; entry < ring_size; entry++) {
	    /* 
	     * The higher bits first - longword 1;
	     */
	    rcv_blk[entry].long_1 = 0; /* initialize */
	    rcv_blk[entry].long_1 = PI_RCV_DESCR_M_SOP |
		    ((buff_size/SEG_LEN_DIVISOR)
		     << PI_RCV_DESCR_V_SEG_LEN) |
			     (seg_count << PI_RCV_DESCR_V_SEG_CNT);
#ifndef __alpha
	    /*
	     * The next descriptor.
	     */
	    entry++;
	    rcv_blk[entry].long_1 = 0; /* initialize */
	    rcv_blk[entry].long_1 =  ((buff_size/SEG_LEN_DIVISOR)
		     << PI_RCV_DESCR_V_SEG_LEN) |
			     (seg_count << PI_RCV_DESCR_V_SEG_CNT);
#endif /* __alpha */
	}
	break;
    }
}



/*
 * Name:	fta_xmt_descr_init();
 *
 * Arguments:
 *	xmt_blk:	The array of xmt decriptors needed to be
 *			initialized.
 *	ring_size:	The number of descriptors to be initialized.
 *
 * Functional Description:
 *	This initializes all the transmit descriptors for the  command req,
 *	data xmt queues.
 *	They do not point to any buffers. These buffers are allocated 
 *	dynamically just before transmit. It is then that the seg_len
 *	field also gets filled in.
 *
 * Calls:
 *	none.
 *
 * Return Codes:
 *	None.	
 */
fta_xmt_descr_init(xmt_blk,
		   ring_size)
register PI_XMT_DESCR *xmt_blk;
register short ring_size;
{
    short entry;
    
    /*
     * Initialize the descriptors.
     */
    for (entry = 0; entry < ring_size; entry++) {
	/* 
	 * The higher bits first - longword 1;
	 */
	xmt_blk[entry].long_1 = 0; /* initialize */
	xmt_blk[entry].buff_lo = 0; /* no address yet */
    }
    wbflush();
}	


	
/*
 * Name:	fta_rcv_buff_produce();
 *
 * Arguments:
 *	sc:     	The softc structure.
 *	to_index:	The index till which to produce. This could
 *			be equal to one less than the service index, which
 *			is the maximum one can produce. Or it could be less
 *			than that.
 *	q_type:		The type of queue on which to produce buffers.
 *	
 * Functional Description:
 *	This produces free buffers on a specified receive queue. As it
 *	produces the buffers it allocates the required memory for it
 *
 * Calls:
 *	none.
 * 
 * Return Codes:
 *      none.
 */
fta_rcv_buff_produce(sc,
		     to_index,
		     q_type)
register struct fta_softc *sc;
register short *to_index;
register short q_type;
{
    PI_RCV_DESCR *rcv_blk;
    u_short prod_index;	/* the producer index */
    u_short compl_index;/* the completion index */
    u_long *buffer_addr;
    int ring_size, buff_size, mask;
    struct rmbuf *bp;
    vm_offset_t physaddr;

    switch (q_type) {
      case RCV_Q:
#ifdef SMT
      case SMT_Q:
	if (q_type == SMT_Q) {
	    ring_size = NPDQHSMT;
	    mask = NPDQHSMT_MASK;
	    bp = sc->smtmbuf;
		rcv_blk = sc->smt_data_blk;
		prod_index = smt_pindex;
		compl_index = smt_cindex;
	} else
#endif /* SMT */
	{
	    /*
	     * NOTE: It may not be possible for all the buffers requested.
	     *	     we will allocate these buffers later. So, long as
	     *	     we can allocate atleast one pair of buffers we will
	     *	     inform the DMA engine of their availability. 
	     *	     For the rest of the allocation a thread is started up
	     * 	     which checks to see if buffers need to be allocated.
	     */	     
	    ring_size = NPDQRCV;
	    mask = NPDQRCV_MASK;
	    bp = sc->rcvmbuf;
	    rcv_blk = sc->rcv_data_blk;
	    prod_index = rcv_pindex;
	    compl_index = rcv_cindex;
	}
#ifdef __alpha
	while (((prod_index + 1) & mask) != compl_index) {
	    struct mbuf *m0;
	    if (to_index != NULL) {
		if (prod_index == *to_index & mask) {
		    /*
		     * We have reached the 
		     * specified number
		     * of descriptors to allocate.
		     */
		    break;
		}
	    }
	    if (bp[prod_index].rmbuf == NULL) {
		FTAMCLGET(m0, 1); /* mbuf with header */
		if (m0 == NULL) {
		    if (prod_index >= 1)
			    goto write_reg;
		    else 
			    return;
		}
	    }
	    /*
	     * The lower bits - The buffer address.
	     */
	    FILLRCVDESCR(m0, 
			 &physaddr,
			 bp[prod_index].phymbuf,
			 bp[prod_index].rmbuf,
			 bp[prod_index].mdata);
	    rcv_blk[prod_index].buff_lo = (u_int)physaddr;
	    rcv_blk[prod_index].long_1 |= (u_int)((physaddr >> 32) & PI_RCV_DESCR_M_BUFF_HI);
	    /*
	     * For the queue, produce the buffers
	     */
	    prod_index = (prod_index + 1) % ring_size;
	    sc->rcv_mbufs_allocated += 1;
	}
#else
	/*
	 * In both these cases we allocate a pair of
	 * 4K mbufs.
	 */
	while (((prod_index + 2) & mask) != compl_index) {
	    struct mbuf *m0, *m1;
	    if (to_index != NULL) {
		if (prod_index == (*to_index - 1) & mask) {
		    /*
		     * We have reached the 
		     * specified number
		     * of descriptors to allocate.
		     */
		    break;
		}
	    }
	    if (bp[prod_index].rmbuf == NULL) {
		FTAMCLGET(m0, 1); /* mbuf with header */
		if (m0 == NULL) {
		    if (prod_index >= 2)
			    goto write_reg;
		    else 
			    return;
		}
	    }
	    if (bp[prod_index + 1].rmbuf == NULL) {
		FTAMCLGET(m1, 0); /* mbuf without header */
		if (m1 == NULL) {
		    m_freem(m0);
		    if (prod_index >= 2)
			    goto write_reg;
		    else 
			    return;
		}
	    }

	    /*
	     * The lower bits - The buffer address.
	     */
	    FILLRCVDESCR(m0, 
			 &rcv_blk[prod_index].buff_lo,
			 bp[prod_index].phymbuf,
			 bp[prod_index].rmbuf,
			 bp[prod_index].mdata);
	    /*
	     * Fill in the address of the next buffer also.
	     */

	    FILLRCVDESCR(m1, 
			 &rcv_blk[prod_index + 1].buff_lo, 
			 bp[prod_index + 1].phymbuf,
			 bp[prod_index + 1].rmbuf,
			 bp[prod_index + 1].mdata);
	    /*
	     * For the queue, produce the buffers
	     */
	    prod_index = (prod_index + 2) % ring_size;
	    sc->rcv_mbufs_allocated += 2;
	}
#endif /* __alpha */
write_reg:

	/*
	 * write out the value of the producer index.
	 */
#ifdef SMT
	if (q_type == SMT_Q) {
		smt_pindex = prod_index;
		FTAREGWR((sc->addr_hostsmtprod), 
			 (FTAREGRD(sc->addr_hostsmtprod) & 
			  ~PI_TYPE_1_PROD_M_PROD) | 
			 (smt_pindex << PI_TYPE_1_PROD_V_PROD));
	} else
#endif /* SMT */
       {
	rcv_pindex = prod_index;
	FTAREGWR((sc->addr_type2prod),
		 (FTAREGRD(sc->addr_type2prod) & 
		  ~PI_TYPE_2_PROD_M_RCV_DATA_PROD) | 
		 (rcv_pindex << PI_TYPE_2_PROD_V_RCV_DATA_PROD));
       }
	FTAIOSYNC();
	break;
	
      case UNSOL_Q:
      case CMDRSP_Q:
	if (q_type == UNSOL_Q) {
	    ring_size = NPDQUNSOL;
	    mask = NPDQUNSOL_MASK;
	    buff_size = UNSOL_BUFSIZ;
	    rcv_blk = sc->unsol_blk;
	    prod_index = unsol_pindex;
	    compl_index = unsol_cindex;
	} else {
	    ring_size = NPDQCRSP;
	    mask = NPDQCRSP_MASK;
	    buff_size = UNSOL_BUFSIZ;
	    rcv_blk = sc->cmd_rsp_blk;
	    prod_index = cmdrsp_pindex;
	    compl_index = cmdrsp_cindex;
	}



	while (((prod_index + 1) & mask) != compl_index) {
	    if (to_index != NULL) {
		if (prod_index == *to_index & mask) {
		    /*
		     * We have reached the 
		     * specified number
		     * of descriptors to allocate.
		     */
		    break;
		}
	    }
	    /*
	     * The lower bits - The buffer address.
	     * These have to be aligned to 512 bytes. We are
	     * going to allocate 4K bytes of memory which will
	     * be aligned to 4K, thus aligned to 512 byte boundary.
	     * This will be changed once the VM gets fixed.
	     */
	    /* buffer_addr = (caddr_t)kmem_alloc(kernel_map, 
					      buff_size); ***SNU**/
	    buffer_addr = (u_long *)kmem_alloc(kernel_map, PAGESZ);
	    if (buffer_addr == NULL)
		    return(NULL);
	    CONVTOPHYS(buffer_addr, &physaddr);
	    rcv_blk[prod_index].buff_lo = (u_int)physaddr;
#ifdef __alpha
	    rcv_blk[prod_index].long_1 |= (u_int)((physaddr >> 32) & PI_RCV_DESCR_M_BUFF_HI);
#endif /* __alpha */
	    /*
	     * Store the virtual address for later use.
	     */
	    if (q_type == UNSOL_Q) 
		    sc->unsol_buf[prod_index] = buffer_addr;
	    else
		    sc->rsp_buf[prod_index] = buffer_addr;

	    /*
	     * For the queue, produce the buffer
	     */
	    prod_index = (prod_index + 1) & mask;
	    
	}

	/*
	 * write out the value of the producer index.
	 * local copy         = adpater's copy       = updated index value
	 */
	if (q_type == UNSOL_Q) {
	    unsol_pindex = prod_index;
	    FTAREGWR((sc->addr_unsolprod), (FTAREGRD(sc->addr_unsolprod) & 
				     ~PI_TYPE_1_PROD_M_PROD)| 
				      (unsol_pindex  << PI_TYPE_1_PROD_V_PROD));
	} else {
	    cmdrsp_pindex = prod_index;
	    FTAREGWR((sc->addr_cmdrspprod), (FTAREGRD(sc->addr_cmdrspprod) & 
				     ~PI_TYPE_1_PROD_M_PROD)| 
				      (cmdrsp_pindex << PI_TYPE_1_PROD_V_PROD));
	}
	FTAIOSYNC();
	break;
    }
    return(1);
}


/*
 * Name:	fta_rcv_mbuf_alloc()
 *
 * Arguments:
 *  	None.
 *			
 * Functional Description:
 *	This thread allocates receive mbufs. The intial mbufs are allocated
 *	in the ftainit routine. It normally is unable to allocate all the
 *	mbufs required.
 *
 * Calls:
 *	None.
 * 
 * Return Codes:
 *	None.
 *
 */
fta_rcv_mbuf_alloc()
{
    thread_t thread;
    struct fta_softc *sc;
    struct ifnet *ifp;
    PI_RCV_DESCR *rcv_blk;
    int mbufs_allocated; 
    int ring_size = NPDQRCV;
    struct rmbuf *bp;
    int to_index, saved_prod, s, foo;
    vm_offset_t physaddr;

    thread = current_thread();

    /*
     * Collect the argument left by the kernel_thread_w_arg().
     */
    sc = (struct fta_softc *)thread->reply_port;
    ifp = &sc->is_if;
    rcv_blk = sc->rcv_data_blk;
    thread->reply_port = PORT_NULL;
    bp = sc->rcvmbuf;
    for(;;) {
	if ((sc->rcv_mbufs_allocated == sc->rcv_mbufs_required) ||
	    sc->rcv_mbufs_allocated >= 256)
		goto take_a_nap;
	s = splimp();
	simple_lock(&sc->alloc_mbufs_lock);
	/*
	 * We need to get more mbufs.
	 */
	to_index = (rcv_pindex + (sc->rcv_mbufs_required - sc->rcv_mbufs_allocated)) & NPDQRCV_MASK;
#ifdef __alpha
	while (((rcv_pindex + 1) & NPDQRCV_MASK) != rcv_cindex) {
	    struct mbuf *m0, *m1;
	    saved_prod = rcv_pindex;
	    if (rcv_pindex == to_index) {
		if (ftadebug > 2)
		    printf("Limit reached: rcv_pindex = %d; to_index = %d\n", rcv_pindex, to_index);
		/*
		 * We have reached the 
		 * specified number
		 * of descriptors to allocate.
		 */
		break; /* out of while() loop */
	    }

	    if (bp[rcv_pindex].rmbuf == NULL) {
		FTAMCLGET(m0, 1); /* mbuf with header */
		if (m0 == NULL) {
		    if (rcv_pindex == saved_prod) /* if nothing is changed */
			    goto take_a_nap;
		    else 			  /* got some buffers */
			    goto write_to_reg;
		}
	    }
	    /*
	     * The lower bits - The buffer address.
	     */
	    FILLRCVDESCR(m0, 
			 &physaddr,
			 bp[rcv_pindex].phymbuf,
			 bp[rcv_pindex].rmbuf,
			 bp[rcv_pindex].mdata);
	    rcv_blk[rcv_pindex].buff_lo = (u_int)physaddr;
	    rcv_blk[rcv_pindex].long_1 |= (u_int)((physaddr >> 32) & PI_RCV_DESCR_M_BUFF_HI);
	    /*
	     * For the queue, produce the buffers
	     */
	    rcv_pindex = (rcv_pindex + 1) & NPDQRCV_MASK;
	    sc->rcv_mbufs_allocated += 1;
	}
#else
	/*
	 * We produce only in pairs.
	 */
	while (((rcv_pindex + 2) & NPDQRCV_MASK) != rcv_cindex) {
	    struct mbuf *m0, *m1;
	    saved_prod = rcv_pindex;
	    if (rcv_pindex == to_index) {
		if (ftadebug > 2)
		    printf("Limit reached: rcv_pindex = %d; to_index = %d\n", rcv_pindex, to_index);
		/*
		 * We have reached the 
		 * specified number
		 * of descriptors to allocate.
		 */
		break; /* out of while() loop */
	    }

	    if (bp[rcv_pindex].rmbuf == NULL) {
		FTAMCLGET(m0, 1); /* mbuf with header */
		if (m0 == NULL) {
		    if (rcv_pindex == saved_prod) /* if nothing is changed */
			    goto take_a_nap;
		    else 			  /* got some buffers */
			    goto write_to_reg;
		}
	    }

	    if (bp[rcv_pindex + 1].rmbuf == NULL) {
		FTAMCLGET(m1, 0); /* mbuf without header */
		if (m1 == NULL) {
		    m_freem(m0);
		    if (rcv_pindex == saved_prod)
			    goto take_a_nap;
		    else 
			    goto write_to_reg;

		}
	    }
	    /*
	     * The lower bits - The buffer address.
	     */
	    FILLRCVDESCR(m0, 
			 &rcv_blk[rcv_pindex].buff_lo,
			 bp[rcv_pindex].phymbuf,
			 bp[rcv_pindex].rmbuf,
			 bp[rcv_pindex].mdata);

	    /*
	     * Fill in the address of the next buffer also.
	     */
	    FILLRCVDESCR(m1, 
			 &rcv_blk[rcv_pindex + 1].buff_lo, 
			 bp[rcv_pindex + 1].phymbuf,
			 bp[rcv_pindex + 1].rmbuf,
			 bp[rcv_pindex + 1].mdata);
	    /*
	     * For the queue, produce the buffers
	     */
	    rcv_pindex = (rcv_pindex + 2) & NPDQRCV_MASK;
	    sc->rcv_mbufs_allocated += 2;
	}
#endif /* __alpha */
write_to_reg:
	if (ftadebug > 2) {
	    printf("thread: producer index = %d\n",rcv_pindex);
	    printf("thread: rcv mbufs allocated = %d\n",sc->rcv_mbufs_allocated);
	}
	/*
	 * write out the value of the producer index.
	 */
	FTAREGWR((sc->addr_type2prod), (FTAREGRD(sc->addr_type2prod) & ~PI_TYPE_2_PROD_M_RCV_DATA_PROD) | (rcv_pindex << PI_TYPE_2_PROD_V_RCV_DATA_PROD));
	FTAIOSYNC();
take_a_nap:	
	/*
	 * Once we have serviced all the responses go back sleep.
	 */
	assert_wait((vm_offset_t)&sc->mbuf_alloc_flag, TRUE);
	thread_set_timeout(hz * 10);
	simple_unlock(&sc->alloc_mbufs_lock);
	splx(s);
	thread_block();
    } /* for(;;) */
}


/*
 * Name:	fta_poll_cmd_req()
 *
 * Arguments:
 *      req_buff: The request buffer to post.
 *      sc:   	  Pointer to softc structure.
 *      command:  The command requested
 *	
 * Functional Description:
 *      This puts in a dma requests and polls for a response in the dma 
 * response queue. 
 *      It produces a response buffer on the cmdrsp queue by incrementing the
 * producer index for the cmdrsp queue. Buffer space for the command response
 * queue is allocated before a request is posted.
 *      Copy the request buffer into the 128-byte aligned buffer on the cmdreq
 * queue. Produce a request buffer, by incrementing the producer index. Next,
 * poll the cmd request for completion. When the request is gone through
 * increment the service index. Then, poll the command response queue. When
 * you get a response back send it back the buffer to the higher layer. Then,
 * increment the service index for the cmdrsp queue.
 * 	NOTE: This is used during start up time, when threads are not
 *	      yet available. Once the system is up, fta_cmd_req() is used.
 *
 * Calls:
 *	None.
 * 
 * Return Codes:
 *       Success:	A pointer to the structure containing status info.
 *	 Failure: 	A pointer to the status info containing reason for
 *			failure.
 *
 */
short
fta_poll_cmd_req(cmdbuf,
	         sc,
	         command)
struct cmd_buf *cmdbuf;
register struct fta_softc *sc;
register short command;
{
    struct ifnet *ifp = &sc->is_if;
    u_long *rsp_buff;
    u_long *req_buf = cmdbuf->req_buf;
    u_long seg_size;
    short delay;
    vm_offset_t physaddr;
   
    switch(command) {
	
      case PI_CMD_K_START:           /* The start command */
      case PI_CMD_K_FILTERS_GET:     /* filters get */
      case PI_CMD_K_STATUS_CHARS_GET:/* characteristics get */
      case PI_CMD_K_ADDR_FILTER_GET: /* filters get */
      case PI_CMD_K_ERROR_LOG_CLEAR: /* error log clear */
      case PI_CMD_K_FDDI_MIB_GET:	/* Get the FDDI MIB attributes */
      case PI_CMD_K_DEC_EXT_MIB_GET: /* Get the DEC ext. mib */
      case PI_CMD_K_SMT_MIB_GET:     /* Get the SMT MIB */
	seg_size = sizeof(NODATA_CMD);
	break;
	
      case PI_CMD_K_FILTERS_SET:     /* filters set */
	seg_size = sizeof(PI_CMD_FILTERS_SET_REQ);
	break;
	
      case PI_CMD_K_CHARS_SET:       /* characteristics set */
	seg_size = sizeof(PI_CMD_CHARS_SET_REQ);
	break;
	
      case PI_CMD_K_SNMP_SET:       /* characteristics set */
	seg_size = sizeof(PI_CMD_SNMP_SET_REQ);
	break;

      case PI_CMD_K_ADDR_FILTER_SET: /* Address filters set */
	seg_size = sizeof(PI_CMD_ADDR_FILTER_SET_REQ);
	break;
	
      case PI_CMD_K_ERROR_LOG_GET:   /* error log get */
	seg_size = sizeof(PI_CMD_ERROR_LOG_GET_REQ);
	break;

      case PI_CMD_K_CNTRS_SET:	     /* counters set */
	seg_size = sizeof(PI_CMD_CNTRS_SET_REQ);
	break;

      case PI_CMD_K_CNTRS_GET:       /* counters get */
	/*
	 * We don't get counters through this mechanism.
	 * It is read through the unsolicited queue every 1 second.
	 * and the internal driver buffer is updated.
	 */
	break;

      default:
	printf("fta_cmd_req: Unknown command requested.\n");
	return(NULL);
	break;
    }
   
    /* First produce a buffer for the command response queue after 
     * allocating data space for it (atleast 512 bytes). The buffer 
     * has to be aligned to a 512 byte boundary. Since the kernel currently 
     * does not support dynamic allocation on a byte boundary we will use
     * a cluster mbuf. For SILVER plans are not in to support such an 
     * allocation. This will change sometime in the future.
     ****SNU** Remember to change it then.*****
     * Note that we don't
     * need to initialize any other part of the descriptor as it has
     * already been initialized during start up time.
     * Fill in the descriptor with the segment length and the address of
     * the data. Then produce this descriptor and  wait for a response.
     */
     rsp_buff = (u_long *)kmem_alloc(kernel_map, PAGESZ); /*SNU** change to 512*/

     if (rsp_buff == NULL)
	     return(NULL);
    /*
     * Initialize the descriptor block - the lower bits first with address
     * cmd_rsp_descriptor_block[indexed_to_the_curr_descr_to_be_produced]
     */
    CONVTOPHYS(rsp_buff, &physaddr);
    sc->cmd_rsp_blk[cmdrsp_pindex].buff_lo = (u_int)physaddr;
    sc->cmd_rsp_blk[cmdrsp_pindex].long_1 =  PI_RCV_DESCR_M_SOP |
					    ((512/SEG_LEN_DIVISOR) << PI_RCV_DESCR_V_SEG_LEN);
#ifdef __alpha
    /*
     * In alpha the physical addresses are more that 32 bits wide.
     * We will have to write out the higher bits of the physical address
     * into the descriptor.
     */
    sc->cmd_rsp_blk[cmdrsp_pindex].long_1 |= (u_int)((physaddr >> 32) & PI_RCV_DESCR_M_BUFF_HI);
#endif /* __alpha */
    cmdbuf->rsp_buf = rsp_buff;
    /*
     * produce a buffer on the response queue
     * update our local copy of the index and write it out to the adapter.
     */
    cmdrsp_pindex = (cmdrsp_pindex + 1) % PI_CMD_RSP_ENTRIES;
    FTAREGWR((sc->addr_cmdrspprod), 
	    (FTAREGRD(sc->addr_cmdrspprod) & ~PI_TYPE_1_PROD_M_PROD)| 
	    (cmdrsp_pindex << PI_TYPE_1_PROD_V_PROD));
    /* wait for it to be written out */
    FTAIOSYNC();


    /*
     * Now fill up the data in the command request descriptor
     * and then produce it. Note, that there is no need to initialize
     * it as it was done during startup time.
     */
    CONVTOPHYS(req_buf, &physaddr);
    sc->cmd_req_blk[cmdreq_pindex].buff_lo = (u_int)physaddr;

    /* fill in the descriptor values */
    sc->cmd_req_blk[cmdreq_pindex].long_1 = PI_XMT_DESCR_M_SOP |
	                                     PI_XMT_DESCR_M_EOP |
					     (seg_size << PI_XMT_DESCR_V_SEG_LEN);
#ifdef __alpha
    /*
     * In alpha the physical addresses are more that 32 bits wide.
     * We will have to write out the higher bits of the physical address
     * into the descriptor.
     */
    sc->cmd_req_blk[cmdreq_pindex].long_1 |= (u_int)((physaddr >> 32) & PI_XMT_DESCR_M_BUFF_HI);
#endif /* __alpha */
    /*
     * produce the buffer on the request queue
     * update our local copy of the index and write it out to the adapter.
     */
    cmdreq_pindex = (cmdreq_pindex + 1) % PI_CMD_REQ_ENTRIES;
    FTAREGWR((sc->addr_cmdreqprod),
	    (FTAREGRD(sc->addr_cmdreqprod) & ~PI_TYPE_1_PROD_M_PROD)| 
	    (cmdreq_pindex << PI_TYPE_1_PROD_V_PROD));
    FTAIOSYNC();

    delay = 0;
    while(sc->cons_req_index != (cmdreq_pindex) % PI_CMD_REQ_ENTRIES){
	if (delay > 1000) {
	    printf("Timeout in command request.\n");
	    break;
	}
	DELAY(50);
	delay++;
    }

    /*
     * Check for command response completion. 
     */
    delay = 0;
    if ((cmdreq_pindex + 1) % NPDQCRSP == sc->cons_req_index) {
	printf("%s%d: Warning: Command response queue is stalled!\n",
		ADAP, ifp->if_unit);
    }

    while(sc->cons_rsp_index != ((cmdrsp_pindex) % PI_CMD_RSP_ENTRIES)){
	if (delay > 3000) {
	    printf("Timeout in command response.\n");
	    ((PI_RSP_HEADER *)rsp_buff)->status = PI_RSP_K_FAILURE;
    	    cmdbuf->timeout = TRUE;
	    return(1);
	}
	DELAY(50);
	delay++;
    }

    cmdbuf->timeout = FALSE;
    /*
     * Make the completion index equal to the producer index for the request
     * and the response queues.
     */
    cmdrsp_cindex = cmdrsp_pindex;
    cmdreq_cindex = cmdreq_pindex;

    FTAREGWR((sc->addr_cmdrspprod),
	     (FTAREGRD(sc->addr_cmdrspprod) &  ~PI_TYPE_1_PROD_M_COMP) | 
	     (cmdrsp_cindex << PI_TYPE_1_PROD_V_COMP));
    FTAIOSYNC();
    FTAREGWR((sc->addr_cmdreqprod), 
	     (FTAREGRD(sc->addr_cmdreqprod) &  ~PI_TYPE_1_PROD_M_COMP) | 
	     (cmdreq_cindex << PI_TYPE_1_PROD_V_COMP));
    FTAIOSYNC();
    /*
     * Now return the buffer containing the data 
     * back.
     */
    return(1);
}


/*
 * Name:	fta_cmd_req()
 *
 * Arguments:
 *      req_buff: The request buffer to post.
 *      sc:   	  Pointer to softc structure.
 *      command:  The command requested
 *	
 * Functional Description:
 *      This puts in a dma requests onto the request queue of the adapter.
 * The response for each of these requests is got from the response queue. An
 * interrupt informs us of the pending response. The order of the response
 * is the order in which the requests were sent.
 *
 * Calls:
 *	None.
 * 
 * Return Codes:
 *       Success:	A pointer to the structure containing status info.
 *	 Failure: 	A pointer to the status info containing reason for
 *			failure.
 *
 */
short
fta_cmd_req(cmdbuf,
	    sc,
	    command)
struct cmd_buf *cmdbuf;
struct fta_softc *sc;
short command;
{
    struct ifnet *ifp = &sc->is_if;
    short seg_size;
    short delay;
    vm_offset_t physaddr;
    struct cmd_buf *sav_ptr, *tmp_ptr;

    switch(command) {
	
      case PI_CMD_K_START:           /* The start command */
      case PI_CMD_K_FILTERS_GET:     /* filters get */
      case PI_CMD_K_STATUS_CHARS_GET:/* characteristics get */
      case PI_CMD_K_ADDR_FILTER_GET: /* filters get */
      case PI_CMD_K_ERROR_LOG_CLEAR: /* error log clear */
      case PI_CMD_K_FDDI_MIB_GET:	/* Get the FDDI MIB attributes */
      case PI_CMD_K_DEC_EXT_MIB_GET: /* Get the DEC extended MIB */
      case PI_CMD_K_SMT_MIB_GET:     /* Get the SMT MIB */
	seg_size = sizeof(NODATA_CMD);
	break;
	
      case PI_CMD_K_FILTERS_SET:     /* filters set */
	seg_size = sizeof(PI_CMD_FILTERS_SET_REQ);
	break;
	
      case PI_CMD_K_CHARS_SET:       /* characteristics set */
	seg_size = sizeof(PI_CMD_CHARS_SET_REQ);
	break;

      case PI_CMD_K_SNMP_SET:       /* characteristics set */
	seg_size = sizeof(PI_CMD_SNMP_SET_REQ);
	break;
	
      case PI_CMD_K_ADDR_FILTER_SET: /* Address filters set */
	seg_size = sizeof(PI_CMD_ADDR_FILTER_SET_REQ);
	break;
	
      case PI_CMD_K_ERROR_LOG_GET:   /* error log get */
	seg_size = sizeof(PI_CMD_ERROR_LOG_GET_REQ);
	break;

      case PI_CMD_K_CNTRS_SET:	     /* counters set */
	seg_size = sizeof(PI_CMD_CNTRS_SET_REQ);
	break;
	
      case PI_CMD_K_CNTRS_GET:       /* counters get */
	/*
	 * We don't get counters through this mechanism.
	 * It is read through the unsolicited queue every 1 second.
	 * and the internal driver buffer is updated.
	 */
	break;

      default:
	printf("fta_cmd_req: Unknown command requested!\n");
	break;
    }

    lock_write(&sc->cmd_buf_q_lock);
    /*
     * Check if there is space on the adapter's queue.
     * If not we return indicating a failure.
     */
    if ((cmdreq_pindex + 1) == cmdreq_cindex)
	    return(NULL);

    /*
     * There is space on the adapter. Queue it onto the request queue.
     * At this point we are assured of atleast a place on the
     * adapter queue - because of the locks we hold. We do not
     * put anything on this queue without those locks.
     */
    sav_ptr = cmdbuf;
    if (sc->q_first == NULL) {
	sc->q_first = sc->q_last = cmdbuf;
    } else {
	sc->q_last->next = cmdbuf;
	sc->q_last = cmdbuf;
	sc->q_last->next = NULL;
    }

    /*
     * Now fill up the data in the command request descriptor
     * and then produce it.
     */
    CONVTOPHYS(sav_ptr->req_buf, &physaddr);
    sc->cmd_req_blk[cmdreq_pindex].buff_lo = (u_int)physaddr;

    /* fill in the descriptor values */
    sc->cmd_req_blk[cmdreq_pindex].long_1 = PI_XMT_DESCR_M_SOP |
	                                     PI_XMT_DESCR_M_EOP |
					     (seg_size << PI_XMT_DESCR_V_SEG_LEN);
#ifdef __alpha
    /*
     * In alpha the physical addresses are more that 32 bits wide.
     * We will have to write out the higher bits of the physical address
     * into the descriptor.
     */
    sc->cmd_req_blk[cmdreq_pindex].long_1 |= (u_int)((physaddr >> 32) & PI_XMT_DESCR_M_BUFF_HI);
#endif /* __alpha */

    /* Produce the descriptor */
    cmdreq_pindex = (cmdreq_pindex + 1) & NPDQCREQ_MASK;

    /* write it out to the adapter */
    FTAREGWR((sc->addr_cmdreqprod),
	     (FTAREGRD(sc->addr_cmdreqprod) & ~PI_TYPE_1_PROD_M_PROD)|
	     (cmdreq_pindex  << PI_TYPE_1_PROD_V_PROD));
    FTAIOSYNC();
    /*
     * We check for command request completion.
     */
    delay = 0;
    while(sc->cons_req_index != (cmdreq_pindex) % PI_CMD_REQ_ENTRIES){
	if (delay > 1000) {
	    printf("Timeout in command request.\n");
	    lock_done(&sc->cmd_buf_q_lock);
	    return(NULL);
	    break;
	}
	DELAY(50);
	delay++;
    }

    /*
     * Bump up the completion index.
     */
    cmdreq_cindex = cmdreq_pindex;
    /* write it out to the adapter */
    FTAREGWR((sc->addr_cmdreqprod),
	     (FTAREGRD(sc->addr_cmdreqprod) & ~PI_TYPE_1_PROD_M_COMP)|
	     (cmdreq_pindex  << PI_TYPE_1_PROD_V_COMP));
    FTAIOSYNC();
    /*
     * Now block for a response.
     */
    assert_wait((vm_offset_t)cmdbuf, TRUE);
    /* unlock the queue */
    lock_done(&sc->cmd_buf_q_lock);
    thread_block();
    return(1);
}


/*
 * Name:	fta_service_cmd_rsp_q()
 *
 * Arguments:
 *  	None.
 *			
 * Functional Description:
 *	This is a thread which is woken up each time there is a command
 *	response interrupt. This thread services all the responses before
 *	going back to sleep again.
 *	There is one such thread per interface.
 *
 * Calls:
 *	None.
 * 
 * Return Codes:
 *	None.
 *
 */
fta_service_cmd_rsp_q()
{
    thread_t thread;
    struct fta_softc *sc;
    struct ifnet *ifp;
    PI_RCV_DESCR *rcv_blk;
    vm_offset_t physaddr;

    thread = current_thread();

    /*
     * Collect the argument left by the kernel_thread_w_arg().
     * See ftainit();
     */
    sc = (struct fta_softc *)thread->reply_port;
    ifp = &sc->is_if;
    rcv_blk = sc->cmd_rsp_blk;
    thread->reply_port = PORT_NULL;
    for(;;) {
	u_long *buffer_addr;
	struct cmd_buf *tmp_ptr;
	int k;
	/*
	 * Check if we have anything on the response queue.
	 * If this is a startup there should be nothing on it.
	 */
	lock_write(&sc->cmd_buf_q_lock);
	while (cmdrsp_cindex != sc->cons_rsp_index) {
	    if (sc->q_first == NULL) {
		/*
		 * This can happen if the adapter was reset and all the
		 * buffers were freed even before this thread got woken	
		 * up. We have a timing problem. In this case we reset 
		 * the adapter and go back to sleep. 
		 */
		printf("%s%d: Nothing on the command response queue.\n",
		       ADAP, ifp->if_unit);
		fta_reinitialize(ifp, FALSE);
		break; /* out of the while loop */
	    }	
	    /*
	     * The first response on the adpater queue will correspond
	     * to the first request on our local queue sc->q_first. We
	     * are assured of this because of the locking we use on the
	     * queues.
	     */
	    /* dequeue the request from our local queue */
	    tmp_ptr = sc->q_first;
	    if (sc->q_first->next == NULL)
		    sc->q_first = sc->q_last = NULL;
	    else
		    sc->q_first = sc->q_first->next;

	    /* Get memory to replace the one we are going to pull out */

	    buffer_addr = (u_long *)kmem_alloc(kernel_map, PAGESZ); /*SNU*/
	    if (buffer_addr == NULL) {
		/* 
		 * We will just drop this packet and reuse the 
		 * descriptor.
		 */
		printf("%s%d: Can not allocate memory on cmd. receive queue.\n",
		       ADAP, ifp->if_unit);
		tmp_ptr->rsp_buf = NULL;
		bzero(sc->rsp_buf[cmdrsp_cindex], PAGESZ); /*SNU*/
		sc->rsp_buf[cmdrsp_pindex] = sc->rsp_buf[cmdrsp_cindex];
		rcv_blk[cmdrsp_pindex].buff_lo = rcv_blk[cmdrsp_cindex].buff_lo;
	    } else {
		tmp_ptr->rsp_buf = sc->rsp_buf[cmdrsp_cindex];

		CONVTOPHYS(buffer_addr, &physaddr);
		rcv_blk[cmdrsp_pindex].buff_lo = (u_int)physaddr;
#ifdef __alpha
    		rcv_blk[cmdrsp_pindex].long_1 |= 
			    (u_int)((physaddr >> 32) & PI_XMT_DESCR_M_BUFF_HI);
#endif /* __alpha */
		/* allocate new memory onto the command response descriptor*/
		sc->rsp_buf[cmdrsp_pindex] = buffer_addr;
		sc->rsp_buf[cmdrsp_cindex] = NULL;
	    }
	    /* bump up the service and producer index */
	    cmdrsp_cindex = (cmdrsp_cindex + 1) & NPDQCRSP_MASK;
	    cmdrsp_pindex = (cmdrsp_pindex + 1) & NPDQCRSP_MASK;
	    /* write out to the adapter registers */

	    FTAREGWR((sc->addr_cmdrspprod),
		     (FTAREGRD(sc->addr_cmdrspprod) & ~PI_TYPE_1_PROD_M_COMP) |
		     (cmdrsp_cindex << PI_TYPE_1_PROD_V_COMP));
	    FTAIOSYNC();
	    FTAREGWR((sc->addr_cmdrspprod), 
		     (FTAREGRD(sc->addr_cmdrspprod) & ~PI_TYPE_1_PROD_M_PROD)| 
		     (cmdrsp_pindex << PI_TYPE_1_PROD_V_PROD));
	    FTAIOSYNC();

	    /* wake up the thread waiting for this response */
	    tmp_ptr->timeout = FALSE;
	    thread_wakeup_one((vm_offset_t)tmp_ptr);
	} /* while (cmdrsp_cindex != sc->cons_rsp_index) */
	/*
	 * Once we have serviced all the responses go back sleep.
	 */
	assert_wait((vm_offset_t)sc, TRUE);
	lock_done(&sc->cmd_buf_q_lock);
	/* We should enable the command response interrupt */
	k = splimp();
	simple_lock(&sc->cmd_rsp_enb_lock);	
	FTAREGWR((sc->addr_intenbX),
		 FTAREGRD(sc->addr_intenbX) | PI_TYPE_X_M_CMD_RSP_ENB);
	FTAIOSYNC();
	simple_unlock(&sc->cmd_rsp_enb_lock);
	thread_block();
	splx(k);
    } /* for(;;) */
}



#define einfo	unsol_report->info.report
/*
 * Name:	fta_service_unsol_q()
 *
 * Arguments:
 *	unit:	  	The unit which has to have the queue serviced.
 *			
 * Functional Description:
 *	This services the unsolicited queue. The information is extracted
 * from the descriptor and passed on to the callee. A new memory is attached
 * to the descriptor and the the buffer is produced. If more than one buffer
 * is serviced then all of them are produced.
 * 	After servicing the queue, the producer register is written out and
 * the interrupt is rearmed.
 *
 * Calls:
 *	None.
 * 
 * Return Codes:
 *	None.
 *
 */
fta_service_unsol_q(unit)
register int unit;
{
    struct fta_softc *sc = &fta_softc[unit];
    struct ifnet *ifp = &sc->is_if;
    PI_UNSOL_REPORT *unsol_report;
    short i;
    vm_offset_t physaddr;

    while (sc->cons_unsol_index != unsol_cindex) {
#ifndef __alpha
	clean_dcache((u_long *)(PHYS_TO_K0(sc->unsol_blk[unsol_cindex].buff_lo)), 512);
#endif /* __alpha */
	unsol_report = (PI_UNSOL_REPORT *)(sc->unsol_buf[unsol_cindex]);

	if (unsol_report->event_type == PI_UNSOL_TYPE_K_EVENT_REPORT) {
	    switch (einfo.event_header.entity) {
	      case PI_UNSOL_ENTITY_K_STATION:
		if (einfo.event_header.event_code == PI_UNSOL_STAT_K_PC_TRACE_RCVD) {
		    printf("%s%d: PC Trace received.\n",
			    ADAP, ifp->if_unit);
		} else
			printf("%s%d: Unknown event received!\n",
				ADAP, ifp->if_unit);
		break;
	      case PI_UNSOL_ENTITY_K_LINK:
	        switch (einfo.event_header.event_code) {
		  case PI_UNSOL_LINK_K_XMT_UNDERRUN:
		    printf("%s%d: Link transmit underrun.\n",
			   ADAP, ifp->if_unit);
		    break;
		  case PI_UNSOL_LINK_K_XMT_FAILURE:
		    printf("%s%d: Link transmit failure.\n",
			   ADAP, ifp->if_unit);
		    break;
		  case PI_UNSOL_LINK_K_BLOCK_CHECK_ERR:
		    printf("%s%d: Block check error.\n",
			   ADAP, ifp->if_unit);
		    break;
		  case PI_UNSOL_LINK_K_FRAME_STAT_ERR:
		    printf("%s%d: Frame status error.\n",
			   ADAP, ifp->if_unit);
		    break;
		  case PI_UNSOL_LINK_K_LENGTH_ERR:
		    printf("%s%d: Length error.\n",
			  ADAP, ifp->if_unit);
		    break;
		  case PI_UNSOL_LINK_K_BAD_IND_DST:
		    printf("%s%d: Bad destination.\n",
			   ADAP, ifp->if_unit);
		    break;
		  case PI_UNSOL_LINK_K_BAD_MCAST_DST:
		    printf("%s%d: Bad multicast destination.\n",
			   ADAP, ifp->if_unit);
		    break;
		  case PI_UNSOL_LINK_K_RCV_OVERRUN:
		    printf("%s%d: Receive Overrun.\n",
			   ADAP, ifp->if_unit);
		    break;
		  case PI_UNSOL_LINK_K_NO_LINK_BUFFER:
		    printf("%s%d: No link buffer.\n",
			   ADAP, ifp->if_unit);
		    break;
		  case PI_UNSOL_LINK_K_NO_USER_BUFFER:
		    printf("%s%d: No user buffer.\n",
			   ADAP, ifp->if_unit);
		    break;
		  case PI_UNSOL_LINK_K_INIT_INITD:
		    printf("%s%d: Link initialization initiated.\n",
			   ADAP, ifp->if_unit);
		    break;
		  case PI_UNSOL_LINK_K_RING_INIT_RCVD:
		    printf("%s%d: Ring initialized.\n",
			   ADAP, ifp->if_unit);
		    break;
		  case PI_UNSOL_LINK_K_BEACON_INITD:
		    printf("%s%d: Ring beacon initiated.\n",
			   ADAP, ifp->if_unit);
		    break;
		  case PI_UNSOL_LINK_K_DUP_ADDR_FOUND:
		    printf("%s%d: Duplicate address failure.\n",
			   ADAP, ifp->if_unit);
		    break;
		  case PI_UNSOL_LINK_K_DUP_TOKEN_FOUND:
		    printf("%s%d: Duplicate token found.\n",
			    ADAP, ifp->if_unit);
		    break;
		  case PI_UNSOL_LINK_K_RING_PURGE_ERR:
		    printf("%s%d: Ring purge error.\n",
			   ADAP, ifp->if_unit);
		    break;
		  case PI_UNSOL_LINK_K_FCI_STRIP_ERR:
		    printf("%s%d: FCI strip error.\n",
			  ADAP, ifp->if_unit);
		    break;
		  case PI_UNSOL_LINK_K_PC_TRACE_INITD:
		    printf("%s%d: PC Trace initiated.\n",
			   ADAP, ifp->if_unit);
		    break;
		  case PI_UNSOL_LINK_K_BEACON_RCVD:
		    printf("%s%d: Directed beacon received.\n",
			   ADAP, ifp->if_unit);
		    break;
		}
		break;
	      case PI_UNSOL_ENTITY_K_PHY:
		switch(einfo.event_header.event_code) {
		  case PI_UNSOL_PHY_K_LEM_REJECT:
		    printf("%s%d: LEM error monitor reject.\n",
			    ADAP, ifp->if_unit);
		    break;
		  case PI_UNSOL_PHY_K_EBUFF_ERR:
		    printf("%s%d: Elasticity buffer error.\n",
			   ADAP, ifp->if_unit);
		    break;
		  case PI_UNSOL_PHY_K_LCT_REJECT:
		    printf("%s%d: LCT rejects.\n",
			  ADAP, ifp->if_unit);
		    break;
		}
		break;
	    }
	    print_event_args(unsol_report, sc);
	    /*
	     * We will reuse the same memory. Rather than 
	     * freeing this and reallocating we will just 
	     * zero out the contents and produce the buffer 
	     * at the end of the loop.
	     */
	    bzero(unsol_report, UNSOL_BUFSIZ);
	    sc->unsol_buf[unsol_pindex] = sc->unsol_buf[unsol_cindex];
	    sc->unsol_buf[unsol_cindex] = NULL;
	    sc->unsol_blk[unsol_pindex].buff_lo = sc->unsol_blk[unsol_cindex].buff_lo;
	} else { /* This is a counter update 
		  * Free the old block and attach the
		  * new block to the softc structure.
		  * Increase IPL while doing that.
		  */
	    simple_lock(&sc->counter_update_lock);
	    sc->ctrblk = (PI_CNTR_BLK *)(&(unsol_report->info.cntrs.cntrs));
	    simple_unlock(&sc->counter_update_lock);
	    bzero(sc->cntr_update, UNSOL_BUFSIZ);
	    CONVTOPHYS(sc->cntr_update, &physaddr);
	    sc->unsol_blk[unsol_pindex].buff_lo = (u_int)physaddr;
#ifdef __alpha
    /*
     * In alpha the physical addresses are more that 32 bits wide.
     * We will have to write out the higher bits of the physical address
     * into the descriptor.
     */
    sc->unsol_blk[unsol_pindex].long_1 |= (u_int)((physaddr >> 32) & PI_XMT_DESCR_M_BUFF_HI);
#endif /* __alpha */
	    sc->unsol_buf[unsol_pindex] = sc->cntr_update;
	    sc->unsol_buf[unsol_cindex] = NULL;
	    sc->cntr_update = (u_long *)unsol_report;
	}
	/*
	 * Increment the completion index and produce the new buffer.
	 */
	unsol_cindex = (unsol_cindex + 1) & NPDQUNSOL_MASK;
	unsol_pindex = (unsol_pindex + 1) & NPDQUNSOL_MASK;
    }
    /*
     * write out the producer and completion indexes to the adapter.
     */
    FTAREGWR((sc->addr_unsolprod),
	     ((FTAREGRD(sc->addr_unsolprod) &  ~PI_TYPE_1_PROD_M_PROD) | 
	      (unsol_pindex << PI_TYPE_1_PROD_V_PROD)));
    FTAIOSYNC();
    FTAREGWR((sc->addr_unsolprod), 
	     ((FTAREGRD(sc->addr_unsolprod) &  ~PI_TYPE_1_PROD_M_COMP) | 
	      (unsol_cindex << PI_TYPE_1_PROD_V_COMP)));
    FTAIOSYNC();
}


/*
 * Name:	print_event_args();
 *
 * Arguments:
 *	einfo:	  	Event info
 *			
 * Functional Description:
 *	This prints out arguments of a received event.
 *
 * Calls:
 *	None.
 * 
 * Return Codes:
 *	None.
 *
 */
print_event_args(unsol_report, sc)
PI_UNSOL_REPORT *unsol_report;
struct fta_softc *sc;
{
    struct ifnet *ifp = &sc->is_if;
    u_int *data;
    PI_UNSOL_ARG_REASON_DESC *station_data;
    PI_UNSOL_ARG_DL_HEADER_DESC *dl;
    PI_UNSOL_ARG_NET_ADDR_DESC *src;
    PI_UNSOL_ARG_NET_ADDR_DESC *upnbr;

    data = unsol_report->info.report.event_data;

    station_data = (PI_UNSOL_ARG_REASON_DESC *)data;

    /*
     * Since the first field is always going to be arg_code, we type
     * cast it to the station data to read the information. This
     * allows us to check up front if there is any data contained
     * in the structure that we should be printing out.
     */
    if (station_data->arg_code == PI_UNSOL_ARG_K_EOL) {
	if (ftadebug > 2)
		printf("station_data->arg_code == PI_UNSOL_ARG_K_EOL\n");
	    return;
    }
    switch (unsol_report->info.report.event_header.event_code) {
      case PI_UNSOL_ENTITY_K_STATION:
	printf("%s%d: Self Test Failure Reason: 0x%X\n",
	       ADAP, ifp->if_unit, 
	       ((PI_UNSOL_ARG_REASON_DESC *)data)->reason_code);
	break;
	
      case PI_UNSOL_ENTITY_K_LINK:

	while (((PI_UNSOL_ARG_REASON_DESC *)data)->arg_code != PI_UNSOL_ARG_K_EOL) {
	    switch (((PI_UNSOL_ARG_REASON_DESC *)data)->arg_code) {
	      case PI_UNSOL_LINK_ARG_K_REASON:
		printf("%s%d: Ring Initialization Reason: 0x%X\n",
		       ADAP, ifp->if_unit, 
		       ((PI_UNSOL_ARG_REASON_DESC *)data)->reason_code);
		station_data = (PI_UNSOL_ARG_REASON_DESC *)data;
		station_data = station_data + sizeof(PI_UNSOL_ARG_REASON_DESC);
		data = (u_int *)station_data;
		break;
		
	      case PI_UNSOL_LINK_ARG_K_DL_HEADER:	
		printf("%s%d: Data Link Header Information:\n",
		       ADAP, ifp->if_unit);
		printf("\tDSAP: %d\n",
		       ((PI_UNSOL_ARG_DL_HEADER_DESC *)(data))->llc.dsap);
		printf("\tSSAP: %d\n",
		       ((PI_UNSOL_ARG_DL_HEADER_DESC *)(data))->llc.ssap);
		printf("\tControl: %d\n",
		       ((PI_UNSOL_ARG_DL_HEADER_DESC *)(data))->llc.control);
		printf("\tPID1: %d\n",
		       ((PI_UNSOL_ARG_DL_HEADER_DESC *)(data))->llc.pid_1);
		printf("\tPID2: %d\n",
		       ((PI_UNSOL_ARG_DL_HEADER_DESC *)(data))->llc.pid_2);
		dl = (PI_UNSOL_ARG_DL_HEADER_DESC *)data;
		dl = dl + sizeof(PI_UNSOL_ARG_DL_HEADER_DESC);
		data = (u_int *)dl;
		break;
		
	      case PI_UNSOL_LINK_ARG_K_SOURCE:
		printf("%s%d: Source Address:\n",
		       ADAP, ifp->if_unit,
		       ether_sprintf((char *)(&((PI_UNSOL_ARG_NET_ADDR_DESC *)(data))->net_addr)));
		bcopy(&((PI_UNSOL_ARG_NET_ADDR_DESC *)(data))->net_addr,
		      sc->dbeacon.db_srcaddr,
		      6);
		src = (PI_UNSOL_ARG_NET_ADDR_DESC *)data;
		src = src + sizeof(PI_UNSOL_ARG_NET_ADDR_DESC);
		data = (u_int *)src;
		break;
		
	      case PI_UNSOL_LINK_ARG_K_UP_NBR:
		printf("%s%d: Upstream Neighbor Address:\n",
		       ADAP, ifp->if_unit,
		       ether_sprintf((char *)(&((PI_UNSOL_ARG_NET_ADDR_DESC *)(data))->net_addr)));
		bcopy(&((PI_UNSOL_ARG_NET_ADDR_DESC *)(data))->net_addr,
		      sc->dbeacon.db_unaddr,
		      6);
		upnbr = (PI_UNSOL_ARG_NET_ADDR_DESC *)data;
		upnbr = upnbr + sizeof(PI_UNSOL_ARG_NET_ADDR_DESC);
		data = (u_int *)upnbr;
		break;
	    }
	}
	break;
      case PI_UNSOL_ENTITY_K_PHY:
	printf("%s%d: Link confidence test failure in",
	       ADAP, ifp->if_unit);
	if (((PI_UNSOL_ARG_DIRECTION_DESC *)(data))->direction == PI_LCT_DIRECTION_K_LOCAL) {
	    printf("local direction.\n");
	} else if (((PI_UNSOL_ARG_DIRECTION_DESC *)(data))->direction == PI_LCT_DIRECTION_K_LOCAL) {
	    printf("remote direction.\n");
	} else {
	    printf("both (local and remote) directions.\n");
	}
	break;
    }
}
	    
		   

/*
 * Name:	ftaoutput();
 *
 * Arguments:
 *	unit:	  	The unit number to transmit on.
 *			
 * Functional Description:
 *	This services the transmit queue. The data passed down from the
 * higher layers, in mbufs, is put in the DMA area and the adapter is
 * informed by writing out the service and producer registers.
 * 	Transmission is always garaunteed. Therefore there is no need to
 * check for a transmit finish status. So, what happens if some one unplugs
 * the cable? Well, we will get a type 0 interrupt informing us of a state
 * change. If the state is  Link Unavailable, (which is read from the Port 
 * Status register) then we know something is wrong and most probably the 
 * cable was pulled out. In such an event we don't transmit.
 * 	Developmental note: It was attempted to transmit with the transmit
 *	interrupt not enabled. This works fine except for process, like
 *	NFS, that must know when the transmit has completed. Needless to
 *	say, this caused NFS to hang and caused us to drop that scheme.
 *
 * Calls:
 *	None.
 * 
 * Return Codes:
 *	None.
 *
 */
ftaoutput(ifp)
register struct ifnet *ifp;
{
    register struct mbuf *m;
    short unit = ifp->if_unit;
    register struct fta_softc *sc = &fta_softc[unit];
    PI_XMT_DESCR *xmt_blk;
    struct mbuf *m0;
    int totlen, n, s;
    caddr_t mptr;
    int consumer_index;
    vm_offset_t physaddr;
    short descr_count = 0;

    s = splimp();
    simple_lock(&sc->xmt_indexes_lock);
    /*
     * Tell the upper layer we are transmitting.
     */
     sc->is_if.if_flags |= IFF_OACTIVE;

    if (ifqmaxlen_seen < ifp->if_snd.ifq_len)
	    ifqmaxlen_seen = ifp->if_snd.ifq_len;
    /*
     * Now to produce some buffers to be transmitted.
     * Process the transmit queue.
     */
    while (((xmt_pindex + 1) & NPDQXMT_MASK) != xmt_cindex) {
	short saved_prod;

	if (sc->is_if.if_snd.ifq_head) {
	    IF_DEQUEUE(&sc->is_if.if_snd, m0);
	} else { /* There is nothing queued up. 
		  */
	    break;	/* From the while loop */
	}
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
	 * Check the packet length
	 */
	if((totlen = m0->m_pkthdr.len) > FDDIMAX) {
	    /*
	     * drop this packet, the size exceeds the MTU.
	     */
	    printf("%s%d: Output packet size too big: %d\n",
			   ADAP, ifp->if_unit, totlen);
	    m_freem(m0);
	    continue;
	}
	if (ftadebug > 3) {
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

	/*
	 * Strategy for xmting:
	 * 	Once the above tests have passed, we blindly start
	 * 	filling in the transmit descriptors - one descriptor
	 *	per mbuf. If we hit the last descriptor
	 *	and we have more than one mbuf left, we will not
	 *	transmit that packet now. We will instead clean up
	 * 	and prepend this back onto the data link queue.
	 *
	 * TAKING CARE OF ARBITRARILY ALIGNED DATA:
	 * 	We could get mbufs that cross page boundaries and
	 * do not start at a page boundary. So, the algorithm implemented
	 * assumes nothing. A mbuf can be aligned to any boundary and
	 * cross a page boundary. In mips the worse case is a mbuf
	 * which is 4098 bytes long. The first byte is in the first
	 * page, 4096 bytes is in another page and the last byte is
	 * on another page. So, we will have atmost 3 pages. But, in
	 * alpha, we will have at most 2 pages, because the page size
	 * is 8192 bytes and since the max MTU size is 4352 bytes it
	 * won't be a problem.
	 *
	 * THE ALGORITHM FOR MIPS:
	 *	if(m_len <= 4K ) {
	 *	    if (it does not cross page boundary)
	 *		put into one descriptor and transmit.
	 *	    else 
	 *		put into two descriptors and transmit only if
	 *		pages are not contiguous.
	 *	else if (m_len > 4K && it starts on a 4K)
	 *		put into two descriptors and transmit.
	 *	else {	  * it crosses page boundary and doesn't
	 *                * start on a 4K boundary and len > 4096
	 *		  *
	 *		if(second page < 4096 bytes)
	 *		   put into two descriptors and transmit.
	 *		if(second page > 4096 bytes)
	 *		   put into three descriptors and transmit.
	 *
	 * THE ALGORITHM FOR ALPHA:
	 *	if(m_len <= FDDIMAX && does not cross page boundary)
	 *		put into one descriptor and transmit.
	 *	else 	  * it crosses page boundary and doesn't
	 *                * start on a 8K boundary.
	 *		  *
	 *		   put into two descriptors and transmit.
	 *       
	 */

	/*
	 * Save the internal producer index
	 */
	saved_prod = xmt_pindex;

	m = m0;
	/*
	 * Get the first mbuf out.
	 */
	mptr = mtod(m, caddr_t);	/* point to the data */

	xmt_blk = &sc->xmt_data_blk[xmt_pindex];
	
	CONVTOPHYS(mptr, &physaddr);
#ifdef __alpha
	{
	    u_long hi_boundary;
	    vm_offset_t physaddr2;
	    /* NOTE: PAGESZ is defined in data/if_fta_data.c */
	    hi_boundary = (physaddr + PAGESZ) & 0xFFFFFFFFFFFFE000;
	    if (physaddr + m->m_len <= hi_boundary) {
		xmt_blk->buff_lo = (u_int)physaddr;
		xmt_blk->long_1 = (m->m_len) << PI_XMT_DESCR_V_SEG_LEN;
		xmt_blk->long_1 |= (u_int)((physaddr >> 32) & PI_XMT_DESCR_M_BUFF_HI);
		xmt_blk->long_1 |=  PI_XMT_DESCR_M_SOP;
		/* save the address for later use, while freeing up mbufs */
		sc->xmt_vaddr[xmt_pindex].vaddr = m;
		xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
		descr_count++;
	    } else {
		int first_page_len = 0;
		first_page_len = hi_boundary - physaddr;
		CONVTOPHYS(mptr + first_page_len, &physaddr2);
		if (physaddr2 == physaddr + first_page_len) {
		    /* This is 2 contiguous pages */
		    xmt_blk->buff_lo = (u_int)physaddr;
		    xmt_blk->long_1 = (m->m_len) << PI_XMT_DESCR_V_SEG_LEN;
		    xmt_blk->long_1 |= (u_int)((physaddr >> 32) & PI_XMT_DESCR_M_BUFF_HI);
		    xmt_blk->long_1 |=  PI_XMT_DESCR_M_SOP;
		    /* save the addr. for later use, while freeing up mbufs */
		    sc->xmt_vaddr[xmt_pindex].vaddr = m;
		    xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
		    descr_count++;
		} else {
		    /*
		     * We have to produce 2 descriptors, for this mbuf,
		     * check if we have space.
		     */
		    if (((xmt_pindex + 2) & NPDQXMT_MASK) != xmt_cindex) {
			xmt_blk->buff_lo = (u_int)physaddr;
			xmt_blk->long_1 = (first_page_len) << PI_XMT_DESCR_V_SEG_LEN;
			xmt_blk->long_1 |= (u_int)((physaddr >> 32) & PI_XMT_DESCR_M_BUFF_HI);
			xmt_blk->long_1 |=  PI_XMT_DESCR_M_SOP;
			sc->xmt_vaddr[xmt_pindex].vaddr = m;
			xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
			/* get the next descriptor for the rest of the mbuf */
			xmt_blk = &sc->xmt_data_blk[xmt_pindex];
			mptr = mptr + first_page_len;
			CONVTOPHYS(mptr, &physaddr);
			xmt_blk->buff_lo = (u_int)physaddr;
			xmt_blk->long_1 = (m->m_len - first_page_len) << PI_XMT_DESCR_V_SEG_LEN;
			xmt_blk->long_1 |= (u_int)((physaddr >> 32) & PI_XMT_DESCR_M_BUFF_HI);
			sc->xmt_vaddr[xmt_pindex].vaddr = NULL;
			xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
		        descr_count++;
		    } else {
			sc->xmt_vaddr[saved_prod].vaddr = NULL;
			xmt_pindex = saved_prod;
		        goto ftaoutput_out;
		    }
		}
	    }
	}
#else
	{
	    u_int hi_boundary;
	    vm_offset_t physaddr2;
	    int first_page_len = 0;

	    /* NOTE: PAGESZ is defined in data/if_fta_data.c */
	    hi_boundary = ((u_int)physaddr + PAGESZ) & 0xFFFFF000;

	    if (m->m_len <= PAGESZ) {
		if (physaddr + m->m_len <= hi_boundary) {
		    xmt_blk->buff_lo = (u_int)physaddr;
		    xmt_blk->long_1 = (m->m_len) << PI_XMT_DESCR_V_SEG_LEN;
		    xmt_blk->long_1 |=  PI_XMT_DESCR_M_SOP;
		    /* save the addr. for later use, while freeing up mbufs */
		    sc->xmt_vaddr[xmt_pindex].vaddr = m;
		    xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
		    descr_count++;
		} else {
		    first_page_len = hi_boundary - physaddr;
		    CONVTOPHYS(mptr + first_page_len, &physaddr2);
		    if (physaddr2 == physaddr + first_page_len) {
			/* This is 2 contiguous pages */
			xmt_blk->buff_lo = (u_int)physaddr;
			xmt_blk->long_1 = (m->m_len) << PI_XMT_DESCR_V_SEG_LEN;
			xmt_blk->long_1 |=  PI_XMT_DESCR_M_SOP;
			/* save the addr. for later use, while freeing mbufs */
			sc->xmt_vaddr[xmt_pindex].vaddr = m;
			xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
		        descr_count++;
		    } else {
			/*
			 * We have to produce 2 descriptors, for this mbuf,
			 * check if we have space.
			 */
			if (((xmt_pindex + 2) & NPDQXMT_MASK) != xmt_cindex) {
			    xmt_blk->buff_lo = (u_int)physaddr;
			    xmt_blk->long_1 = (first_page_len) << PI_XMT_DESCR_V_SEG_LEN;
			    xmt_blk->long_1 |=  PI_XMT_DESCR_M_SOP;
			    sc->xmt_vaddr[xmt_pindex].vaddr = m;
			    xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
		            descr_count++;
			    /* get the next descriptor for the rest */
			    xmt_blk = &sc->xmt_data_blk[xmt_pindex];
			    mptr = mptr + first_page_len;
			    CONVTOPHYS(mptr, &physaddr);
			    xmt_blk->buff_lo = (u_int)physaddr;
			    xmt_blk->long_1 = (m->m_len - first_page_len) << PI_XMT_DESCR_V_SEG_LEN;
			    sc->xmt_vaddr[xmt_pindex].vaddr = NULL;
			    xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
		            descr_count++;
			} else {
			    sc->xmt_vaddr[saved_prod].vaddr = NULL;
			    xmt_pindex = saved_prod;
			    goto ftaoutput_out;
			}
		    }
		}
	    } else {
		int first_page_len = 0, second_page_len = 0, third_page_len = 0;
		/*
		 * This crosses and doesn't start on a page boundary.
		 */
		first_page_len = hi_boundary - physaddr;
		second_page_len = m->m_len - first_page_len;
		
		if (second_page_len <= PAGESZ) { /* no third page */
		    CONVTOPHYS(mptr + first_page_len, &physaddr2);
		    if (physaddr2 == physaddr + first_page_len) {
			/* This is 2 contiguous pages */
			xmt_blk->buff_lo = (u_int)physaddr;
			xmt_blk->long_1 = (m->m_len) << PI_XMT_DESCR_V_SEG_LEN;
			xmt_blk->long_1 |=  PI_XMT_DESCR_M_SOP;
			/* save addr for later use, while freeing up mbufs */
			sc->xmt_vaddr[xmt_pindex].vaddr = m;
			xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
		        descr_count++;
		    } else {
			/*
			 * We have to produce 2 descriptors, for this mbuf,
			 * check if we have space.
			 */
			if (((xmt_pindex + 2) & NPDQXMT_MASK) != xmt_cindex) {
			    xmt_blk->buff_lo = (u_int)physaddr;
			    xmt_blk->long_1 = first_page_len << PI_XMT_DESCR_V_SEG_LEN;
			    xmt_blk->long_1 |=  PI_XMT_DESCR_M_SOP;
			    sc->xmt_vaddr[xmt_pindex].vaddr = m;
			    xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
		            descr_count++;
			    
			    /* get the next descriptor for the rest of the mbuf */
			    xmt_blk = &sc->xmt_data_blk[xmt_pindex];
			    mptr = mptr + first_page_len;
			    CONVTOPHYS(mptr, &physaddr);
			    xmt_blk->buff_lo = (u_int)physaddr;
			    xmt_blk->long_1 = (m->m_len - first_page_len) << PI_XMT_DESCR_V_SEG_LEN;
			    sc->xmt_vaddr[xmt_pindex].vaddr = NULL;
			    xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
		            descr_count++;
			} else {
			    sc->xmt_vaddr[saved_prod].vaddr = NULL;
			    xmt_pindex = saved_prod;
			    goto ftaoutput_out;
			}
		    }
		} else { /* there are 3 pages. */
		    third_page_len = m->m_len - first_page_len - PAGESZ;
		    if (((xmt_pindex + 3) & NPDQXMT_MASK) != xmt_cindex) {
			xmt_blk->buff_lo = (u_int)physaddr;
			xmt_blk->long_1 = first_page_len << PI_XMT_DESCR_V_SEG_LEN;
			xmt_blk->long_1 |=  PI_XMT_DESCR_M_SOP;
			sc->xmt_vaddr[xmt_pindex].vaddr = m;
			xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
		        descr_count++;
			/* get the next descriptor for the rest of the mbuf */
			xmt_blk = &sc->xmt_data_blk[xmt_pindex];
			mptr = mptr + first_page_len;
			CONVTOPHYS(mptr, &physaddr);
			xmt_blk->buff_lo = (u_int)physaddr;
			xmt_blk->long_1 = (PAGESZ) << PI_XMT_DESCR_V_SEG_LEN;
			sc->xmt_vaddr[xmt_pindex].vaddr = NULL;
			xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
			descr_count++;
			/* get the 3rd descriptor for the rest of the mbuf */
			xmt_blk = &sc->xmt_data_blk[xmt_pindex];
			mptr = mptr + first_page_len + PAGESZ;
			CONVTOPHYS(mptr, &physaddr);
			xmt_blk->buff_lo = (u_int)physaddr;
			xmt_blk->long_1 = (third_page_len) << PI_XMT_DESCR_V_SEG_LEN;
			sc->xmt_vaddr[xmt_pindex].vaddr = NULL;
			xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
			descr_count++;
		    } else {
			sc->xmt_vaddr[saved_prod].vaddr = NULL;
			xmt_pindex = saved_prod;
			goto ftaoutput_out;
		    }
		}
	    }
	}
#endif /* __alpha */

	m = m->m_next;

	while (m) {
	    if (m->m_len == 0) {
		/* 
		 * The adapter will halt if zero length mbufs
		 * are transmitted.
		 */
		m = m->m_next;
	        continue;
	    }

	    if (((xmt_pindex + 1) & NPDQXMT_MASK) == xmt_cindex) {
		int descr;
		if (ftadebug > 2)
			printf("%s%d: Adapter descriptors full.\n",
			        ADAP, ifp->if_unit);
		if ((descr = xmt_pindex - saved_prod) < 0)
			descr = descr + NPDQXMT;
		if (descr >= 254) {
#ifdef __alpha
		/*
		 * If this packet has 254 mbufs or more,
		 * roll the entire packet into one large
		 * cluster mbuf and prepend it back.
		 * We will transmit this the next time around.
		 */
		/*
		 * NOTE: For alpha we have currently an 8K cluster and
		 *	 the packet size has to be less than that. So,
		 *	 we can safely copy this into the cluster.
		 *	 But, in case of MIPs this has to be dealt with
		 *	 differently and requires a slightly different
		 *	 algorithm which hasn't been implemented. In future, 
		 *	 if this problem is encountered
		 *	 on a MIPS platform, or in case of ALPHA if the 
		 *	 cluster size is changed to less than FDDIMTU,the 
		 *	 algorithm will will have to be changed.
		 */
		    struct mbuf *mnew;
		    FTAMCLGET(mnew, 1); /* mbuf with header */
		    if (mnew != NULL) {
			u_short len = 0;
			caddr_t mdata, mdata_new;

			m = m0;
			mdata_new = mtod(mnew, caddr_t);
			while(m) {
			    mdata = mtod(m, caddr_t);
			    bcopy(mdata, mdata_new, m->m_len);
			    mdata_new += m->m_len;
			    len += m->m_len;
			    m = m->m_next;
			}
			mnew->m_len = len;	
			mnew->m_next = NULL;
			mnew->m_pkthdr.len = m0->m_pkthdr.len;
			m_freem(m0);
			m0 = mnew;
		    }
		    IF_PREPEND(&sc->is_if.if_snd, m0);
		    xmt_pindex = saved_prod;
		    sc->xmt_vaddr[saved_prod].vaddr = NULL;
		    if (mnew != NULL)
		    	goto ftaoutput_redo;
		    else
		    	goto ftaoutput_out;
#else 
		/*
		 * If this packet has 254 mbufs or more,
		 * free the packet. We don't transmit it.
		 * This algorithm should be rewritten for the
		 * MIPS platform.
		 */
	    	printf("%s%d: Too many buffers in xmit packet. Dropping it.: %d\n",
			   ADAP, ifp->if_unit, descr);
		 m_freem(m0);
		 xmt_pindex = saved_prod;
		 sc->xmt_vaddr[saved_prod].vaddr = NULL;
		 goto ftaoutput_redo;
#endif /* __alpha */
		} else {
		    IF_PREPEND(&sc->is_if.if_snd, m0);
		    xmt_pindex = saved_prod;
		    sc->xmt_vaddr[saved_prod].vaddr = NULL;
		}
		/*
		 * If we reach this point it indicates that the adapter
		 * is full and we should give the driver a chance to
		 * service the descriptors which have been transmitted.
		 * This is done by relinquishing this IPL and returning.
		 * This will allow the xmt done interrupt to be serviced
		 * and will allow and receives to be processed.
		 */
		goto ftaoutput_out;
	    } else {
		mptr = mtod(m, caddr_t);	/* point to the data */
		xmt_blk = &sc->xmt_data_blk[xmt_pindex];
		CONVTOPHYS(mptr, &physaddr);
#ifdef __alpha
		{
		    u_long hi_boundary;
		    vm_offset_t physaddr2;
		    hi_boundary = (physaddr + PAGESZ) & 0xFFFFE000;
		    if (physaddr + m->m_len <= hi_boundary) {
			xmt_blk->buff_lo = (u_int)physaddr;
			xmt_blk->long_1 = (m->m_len) << PI_XMT_DESCR_V_SEG_LEN;
			xmt_blk->long_1 |= (u_int)((physaddr >> 32) & PI_XMT_DESCR_M_BUFF_HI);
			/* save the address for later use, 
			 * while freeing up mbufs.
			 */
			sc->xmt_vaddr[xmt_pindex].vaddr = NULL;
			xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
			descr_count++;
		    } else {
			int first_page_len = 0;
			first_page_len = hi_boundary - physaddr;
			CONVTOPHYS(mptr + first_page_len, &physaddr2);
			if (physaddr2 == physaddr + first_page_len) {
			    /* This is 2 contiguous pages */
			    xmt_blk->buff_lo = (u_int)physaddr;
			    xmt_blk->long_1 = (m->m_len) << PI_XMT_DESCR_V_SEG_LEN;
			    xmt_blk->long_1 |= (u_int)((physaddr >> 32) & PI_XMT_DESCR_M_BUFF_HI);
			    /* save addr. for later use, while freeing mbufs */
			    sc->xmt_vaddr[xmt_pindex].vaddr = NULL;
			    xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
			    descr_count++;
			} else {

			    /*
			     * We have to produce 2 descriptors, for this mbuf,
			     * check if we have space.
			     */
			    if (((xmt_pindex + 2) & NPDQXMT_MASK) != xmt_cindex) {
				xmt_blk->buff_lo = (u_int)physaddr;
				xmt_blk->long_1 = (first_page_len) << PI_XMT_DESCR_V_SEG_LEN;
				xmt_blk->long_1 |= (u_int)((physaddr >> 32) & PI_XMT_DESCR_M_BUFF_HI);
				sc->xmt_vaddr[xmt_pindex].vaddr = NULL;
				xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
			        descr_count++;
				/* get the next descriptor for the rest 
				 * of the mbuf 
				 */
				xmt_blk = &sc->xmt_data_blk[xmt_pindex];
				mptr = mptr + first_page_len;
				CONVTOPHYS(mptr, &physaddr);
				xmt_blk->buff_lo = (u_int)physaddr;
				xmt_blk->long_1 = (m->m_len - first_page_len) << PI_XMT_DESCR_V_SEG_LEN;
				xmt_blk->long_1 |= (u_int)((physaddr >> 32) & PI_XMT_DESCR_M_BUFF_HI);
				sc->xmt_vaddr[xmt_pindex].vaddr = NULL;
				xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
			        descr_count++;
			    } else {
				sc->xmt_vaddr[saved_prod].vaddr = NULL;
				xmt_pindex = saved_prod;
				goto ftaoutput_out;
			    }
			}
		    }
		}
#else
		{
		    u_int hi_boundary;
		    vm_offset_t physaddr2;
		    int first_page_len = 0;

		    hi_boundary = ((u_int)physaddr + PAGESZ) & 0xFFFFF000;
		    
		    if (m->m_len <= PAGESZ) {
			if (physaddr + m->m_len <= hi_boundary) {
			    xmt_blk->buff_lo = (u_int)physaddr;
			    xmt_blk->long_1 = (m->m_len) << PI_XMT_DESCR_V_SEG_LEN;
			    /* save the addr. for later use, while freeing up mbufs */
			    sc->xmt_vaddr[xmt_pindex].vaddr = NULL;
			    xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
			    descr_count++;
			} else {
			    first_page_len = hi_boundary - physaddr;
			    CONVTOPHYS(mptr + first_page_len, &physaddr2);
			    if (physaddr2 == physaddr + first_page_len) {
				/* This is 2 contiguous pages */
				xmt_blk->buff_lo = (u_int)physaddr;
				xmt_blk->long_1 = (m->m_len) << PI_XMT_DESCR_V_SEG_LEN;
				/* save the addr. for later use, while freeing mbufs */
				sc->xmt_vaddr[xmt_pindex].vaddr = NULL;
				xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
				descr_count++;
			    } else {
				/*
				 * We have to produce 2 descriptors, for this mbuf,
				 * check if we have space.
				 */
				if (((xmt_pindex + 2) & NPDQXMT_MASK) != xmt_cindex) {
				    xmt_blk->buff_lo = (u_int)physaddr;
				    xmt_blk->long_1 = (first_page_len) << PI_XMT_DESCR_V_SEG_LEN;
				    sc->xmt_vaddr[xmt_pindex].vaddr = NULL;
				    xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
				    descr_count++;
				    /* get the next descriptor for the rest */
				    xmt_blk = &sc->xmt_data_blk[xmt_pindex];
				    mptr = mptr + first_page_len;
				    CONVTOPHYS(mptr, &physaddr);
				    xmt_blk->buff_lo = (u_int)physaddr;
				    xmt_blk->long_1 = (m->m_len - first_page_len) << PI_XMT_DESCR_V_SEG_LEN;
				    sc->xmt_vaddr[xmt_pindex].vaddr = NULL;
				    xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
				    descr_count++;
				} else {
				    sc->xmt_vaddr[saved_prod].vaddr = NULL;
				    xmt_pindex = saved_prod;
				    goto ftaoutput_out;
				}
			    }
			}
		    } else {
			int first_page_len = 0, second_page_len = 0, third_page_len = 0;
			first_page_len = hi_boundary - physaddr;
			second_page_len = m->m_len - first_page_len;
			
			if (second_page_len <= PAGESZ) { /* no third page */
			    CONVTOPHYS(mptr + first_page_len, &physaddr2);
			    if (physaddr2 == physaddr + first_page_len) {
				/* This is 2 contiguous pages */
				xmt_blk->buff_lo = (u_int)physaddr;
				xmt_blk->long_1 = (m->m_len) << PI_XMT_DESCR_V_SEG_LEN;
				/* save addr for later use, while freeing up mbufs */
				sc->xmt_vaddr[xmt_pindex].vaddr = NULL;
				xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
				descr_count++;
			    } else {
				/*
				 * We have to produce 2 descriptors, for this mbuf,
				 * check if we have space.
				 */
				if (((xmt_pindex + 2) & NPDQXMT_MASK) != xmt_cindex) {
				    xmt_blk->buff_lo = (u_int)physaddr;
				    xmt_blk->long_1 = first_page_len << PI_XMT_DESCR_V_SEG_LEN;
				    sc->xmt_vaddr[xmt_pindex].vaddr = NULL;
				    xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
				    descr_count++;
				    
				    /* get the next descriptor for the rest of the mbuf */
				    xmt_blk = &sc->xmt_data_blk[xmt_pindex];
				    mptr = mptr + first_page_len;
				    CONVTOPHYS(mptr, &physaddr);
				    xmt_blk->buff_lo = (u_int)physaddr;
				    xmt_blk->long_1 = (m->m_len - first_page_len) << PI_XMT_DESCR_V_SEG_LEN;
				    sc->xmt_vaddr[xmt_pindex].vaddr = NULL;
				    xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
				    descr_count++;
				} else {
				    sc->xmt_vaddr[saved_prod].vaddr = NULL;
				    xmt_pindex = saved_prod;
				    goto ftaoutput_out;
				}
			    }
			} else { /* there are 3 pages. */
			    third_page_len = m->m_len - first_page_len - PAGESZ;
			    if (((xmt_pindex + 3) & NPDQXMT_MASK) != xmt_cindex) {
				xmt_blk->buff_lo = (u_int)physaddr;
				xmt_blk->long_1 = first_page_len << PI_XMT_DESCR_V_SEG_LEN;
				sc->xmt_vaddr[xmt_pindex].vaddr = NULL;
				xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
				descr_count++;
				/* get the next descriptor for the rest of the mbuf */
				xmt_blk = &sc->xmt_data_blk[xmt_pindex];
				mptr = mptr + first_page_len;
				CONVTOPHYS(mptr, &physaddr);
				xmt_blk->buff_lo = (u_int)physaddr;
				xmt_blk->long_1 = (PAGESZ) << PI_XMT_DESCR_V_SEG_LEN;
				sc->xmt_vaddr[xmt_pindex].vaddr = NULL;
				xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
				descr_count++;
				/* get the 3rd descriptor for the rest of the mbuf */
				xmt_blk = &sc->xmt_data_blk[xmt_pindex];
				mptr = mptr + first_page_len + PAGESZ;
				CONVTOPHYS(mptr, &physaddr);
				xmt_blk->buff_lo = (u_int)physaddr;
				xmt_blk->long_1 = (third_page_len) << PI_XMT_DESCR_V_SEG_LEN;
				sc->xmt_vaddr[xmt_pindex].vaddr = NULL;
				xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
				descr_count++;
			    } else {
				sc->xmt_vaddr[saved_prod].vaddr = NULL;
				xmt_pindex = saved_prod;
				goto ftaoutput_out;
			    }
			}	
		    }
		}
#endif /* __alpha */
	    }
	    m = m->m_next;
	}
	/*
	 * Mark the last descriptor of the packet.
	 */
	xmt_blk->long_1 |= PI_XMT_DESCR_M_EOP;

	/*
	 * Write out the producer index if we have crossed the XMT_PROD_THRESH.
	 */
	 if (descr_count > sc->xmt_prod_thresh) {
	     FTAREGWR((sc->addr_type2prod),
		     (FTAREGRD(sc->addr_type2prod) & ~PI_TYPE_2_PROD_M_XMT_DATA_PROD)| (xmt_pindex << PI_TYPE_2_PROD_V_XMT_DATA_PROD));;
	     FTAIOSYNC();
	     descr_count = 0;
	 }
ftaoutput_redo:
	continue;
    }/* while (((xmt_pindex + 1) & NPDQXMT_MASK) != xmt_cindex) */

ftaoutput_out:
    if (descr_count) {
	FTAREGWR((sc->addr_type2prod),
		(FTAREGRD(sc->addr_type2prod) & ~PI_TYPE_2_PROD_M_XMT_DATA_PROD)| (xmt_pindex << PI_TYPE_2_PROD_V_XMT_DATA_PROD));;
	FTAIOSYNC();
    }
    sc->is_if.if_flags &= ~IFF_OACTIVE;
    simple_unlock(&sc->xmt_indexes_lock);
    splx(s);
    return;
}	


/*
 * Name:	ftaoutput_one_pkt();
 *
 * Arguments:
 *	unit:	  	The unit number to transmit on.
 *			
 * Functional Description:
 *	This services the transmit queue to transmit only ONE packet. This
 * is called from the receive routine while processing received packets. In
 * order to achieve fairness if we receive more than a certain number of
 * packets we allow one packet to the transmitted. The rationale here is that
 * the packet we are transmitting in all probability will be an ack. from the
 * higher layers. So, this won't cause the higher layer protocols to time out
 * if the driver is very busy receiving packets.
 *
 * Calls:
 *	None.
 * 
 * Return Codes:
 *	None.
 *
 */
ftaoutput_one_pkt(ifp)
register struct ifnet *ifp;
{
    register struct mbuf *m;
    short unit = ifp->if_unit;
    register struct fta_softc *sc = &fta_softc[unit];
    PI_XMT_DESCR *xmt_blk;
    struct mbuf *m0;
    int totlen, n, s; 
    caddr_t mptr;
    vm_offset_t physaddr;

    s = splimp();
    simple_lock(&sc->xmt_indexes_lock);
    /*
     * Tell the upper layer we are transmitting.
     */
     sc->is_if.if_flags |= IFF_OACTIVE;

    /*
     * Now to produce some buffers to be transmitted.
     * Process the transmit queue.
     */
    if (((xmt_pindex + 1) & NPDQXMT_MASK) != xmt_cindex) {
	short saved_prod;

	if (sc->is_if.if_snd.ifq_head) {
	    IF_DEQUEUE(&sc->is_if.if_snd, m0);
	} else {
	    return;
	}
	
	/*
	 * Check for packet header.
	 */
	if (!(m0->m_flags & M_PKTHDR)) {
	    /*
	     * drop this packet as it does not have
	     * the packet header.
	     */
	    if(ftadebug > 2)
		    printf("%s%d: No packet header in mbuf!\n",
			   ADAP, ifp->if_unit);
	    m_freem(m0);
	    return;
	}

	/*
	 * Check the packet length
	 */
	if((totlen = m0->m_pkthdr.len) > FDDIMAX) {
	    /*
	     * drop this packet, the size exceeds the MTU.
	     */
	    if (ftadebug > 2)
		    printf("%s%d: Output packet size too big: %d\n",totlen,
			   ADAP, ifp->if_unit);
	    m_freem(m0);
	    return;
	}

	/*
	 * Strategy for xmting:
	 * 	Once the above tests have passed, we blindly start
	 * 	filling in the transmit descriptors - one descriptor
	 *	per mbuf. If we hit the last descriptor
	 *	and we have more than one mbuf left, we will not
	 *	transmit that packet now. We will instead clean up
	 * 	and prepend this back onto the data link queue.
	 */
	
	/*
 	 * Save the internal producer index
	 */
	saved_prod = xmt_pindex;

	m = m0;
	/*
	 * Get the first mbuf out.
	 */
	mptr = mtod(m, caddr_t);	/* point to the data */

	xmt_blk = &sc->xmt_data_blk[xmt_pindex];
	CONVTOPHYS(mptr, &physaddr);
#ifdef __alpha
	{
	    u_long hi_boundary;
	    vm_offset_t physaddr2;
	    hi_boundary = (physaddr + PAGESZ) & 0xFFFFFFFFFFFFE000;
	    if (physaddr + m->m_len <= hi_boundary) {
		xmt_blk->buff_lo = (u_int)physaddr;
		xmt_blk->long_1 = (m->m_len) << PI_XMT_DESCR_V_SEG_LEN;
		xmt_blk->long_1 |= (u_int)((physaddr >> 32) & PI_XMT_DESCR_M_BUFF_HI);
		xmt_blk->long_1 |=  PI_XMT_DESCR_M_SOP;
		/* save the address for later use, while freeing up mbufs */
		sc->xmt_vaddr[xmt_pindex].vaddr = m;
		xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
	    } else {
		int first_page_len = 0;
		first_page_len = hi_boundary - physaddr;
		CONVTOPHYS(mptr + first_page_len, &physaddr2);
		if (physaddr2 == physaddr + first_page_len) {
		    /* This is 2 contiguous pages */
		    xmt_blk->buff_lo = (u_int)physaddr;
		    xmt_blk->long_1 = (m->m_len) << PI_XMT_DESCR_V_SEG_LEN;
		    xmt_blk->long_1 |= (u_int)((physaddr >> 32) & PI_XMT_DESCR_M_BUFF_HI);
		    xmt_blk->long_1 |=  PI_XMT_DESCR_M_SOP;
		    /* save the addr. for later use, while freeing up mbufs */
		    sc->xmt_vaddr[xmt_pindex].vaddr = m;
		    xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
		} else {
		    /*
		     * We have to produce 2 descriptors, for this mbuf,
		     * check if we have space.
		     */
		    if (((xmt_pindex + 2) & NPDQXMT_MASK) != xmt_cindex) {
			xmt_blk->buff_lo = (u_int)physaddr;
			xmt_blk->long_1 = (first_page_len) << PI_XMT_DESCR_V_SEG_LEN;
			xmt_blk->long_1 |= (u_int)((physaddr >> 32) & PI_XMT_DESCR_M_BUFF_HI);
			xmt_blk->long_1 |=  PI_XMT_DESCR_M_SOP;
			sc->xmt_vaddr[xmt_pindex].vaddr = m;
			xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
			/* get the next descriptor for the rest of the mbuf */
			xmt_blk = &sc->xmt_data_blk[xmt_pindex];
			mptr = mptr + first_page_len;
			CONVTOPHYS(mptr, &physaddr);
			xmt_blk->buff_lo = (u_int)physaddr;
			xmt_blk->long_1 = (m->m_len - first_page_len) << PI_XMT_DESCR_V_SEG_LEN;
			xmt_blk->long_1 |= (u_int)((physaddr >> 32) & PI_XMT_DESCR_M_BUFF_HI);
			sc->xmt_vaddr[xmt_pindex].vaddr = NULL;
			xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
		    } else {
			sc->xmt_vaddr[saved_prod].vaddr = NULL;
			xmt_pindex = saved_prod;
			return;
		    }
		}
	    }
	}
#else
	{
	    u_int hi_boundary;
	    vm_offset_t physaddr2;
	    int first_page_len = 0;

	    hi_boundary = ((u_int)physaddr + PAGESZ) & 0xFFFFF000;

	    if (m->m_len <= PAGESZ) {
		if (physaddr + m->m_len <= hi_boundary) {
		    xmt_blk->buff_lo = (u_int)physaddr;
		    xmt_blk->long_1 = (m->m_len) << PI_XMT_DESCR_V_SEG_LEN;
		    xmt_blk->long_1 |=  PI_XMT_DESCR_M_SOP;
		    /* save the addr. for later use, while freeing up mbufs */
		    sc->xmt_vaddr[xmt_pindex].vaddr = m;
		    xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
		} else {
		    first_page_len = hi_boundary - physaddr;
		    CONVTOPHYS(mptr + first_page_len, &physaddr2);
		    if (physaddr2 == physaddr + first_page_len) {
			/* This is 2 contiguous pages */
			xmt_blk->buff_lo = (u_int)physaddr;
			xmt_blk->long_1 = (m->m_len) << PI_XMT_DESCR_V_SEG_LEN;
			xmt_blk->long_1 |=  PI_XMT_DESCR_M_SOP;
			/* save the addr. for later use, while freeing mbufs */
			sc->xmt_vaddr[xmt_pindex].vaddr = m;
			xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
		    } else {
			/*
			 * We have to produce 2 descriptors, for this mbuf,
			 * check if we have space.
			 */
			if (((xmt_pindex + 2) & NPDQXMT_MASK) != xmt_cindex) {
			    xmt_blk->buff_lo = (u_int)physaddr;
			    xmt_blk->long_1 = (first_page_len) << PI_XMT_DESCR_V_SEG_LEN;
			    xmt_blk->long_1 |=  PI_XMT_DESCR_M_SOP;
			    sc->xmt_vaddr[xmt_pindex].vaddr = m;
			    xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
			    /* get the next descriptor for the rest */
			    xmt_blk = &sc->xmt_data_blk[xmt_pindex];
			    mptr = mptr + first_page_len;
			    CONVTOPHYS(mptr, &physaddr);
			    xmt_blk->buff_lo = (u_int)physaddr;
			    xmt_blk->long_1 = (m->m_len - first_page_len) << PI_XMT_DESCR_V_SEG_LEN;
			    sc->xmt_vaddr[xmt_pindex].vaddr = NULL;
			    xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
			} else {
			    sc->xmt_vaddr[saved_prod].vaddr = NULL;
			    xmt_pindex = saved_prod;
			    return;
			}
		    }
		}
	    } else {
		int first_page_len = 0, second_page_len = 0, third_page_len = 0;
		/*
		 * This crosses and doesn't start on a page boundary.
		 */
		first_page_len = hi_boundary - physaddr;
		second_page_len = m->m_len - first_page_len;
		
		if (second_page_len <= PAGESZ) { /* no third page */
		    CONVTOPHYS(mptr + first_page_len, &physaddr2);
		    if (physaddr2 == physaddr + first_page_len) {
			/* This is 2 contiguous pages */
			xmt_blk->buff_lo = (u_int)physaddr;
			xmt_blk->long_1 = (m->m_len) << PI_XMT_DESCR_V_SEG_LEN;
			xmt_blk->long_1 |=  PI_XMT_DESCR_M_SOP;
			/* save addr for later use, while freeing up mbufs */
			sc->xmt_vaddr[xmt_pindex].vaddr = m;
			xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
		    } else {
			/*
			 * We have to produce 2 descriptors, for this mbuf,
			 * check if we have space.
			 */
			if (((xmt_pindex + 2) & NPDQXMT_MASK) != xmt_cindex) {
			    xmt_blk->buff_lo = (u_int)physaddr;
			    xmt_blk->long_1 = first_page_len << PI_XMT_DESCR_V_SEG_LEN;
			    xmt_blk->long_1 |=  PI_XMT_DESCR_M_SOP;
			    sc->xmt_vaddr[xmt_pindex].vaddr = m;
			    xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
			    
			    /* get the next descriptor for the rest of the mbuf */
			    xmt_blk = &sc->xmt_data_blk[xmt_pindex];
			    mptr = mptr + first_page_len;
			    CONVTOPHYS(mptr, &physaddr);
			    xmt_blk->buff_lo = (u_int)physaddr;
			    xmt_blk->long_1 = (m->m_len - first_page_len) << PI_XMT_DESCR_V_SEG_LEN;
			    sc->xmt_vaddr[xmt_pindex].vaddr = NULL;
			    xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
			} else {
			    sc->xmt_vaddr[saved_prod].vaddr = NULL;
			    xmt_pindex = saved_prod;
			    return;
			}
		    }
		} else { /* there are 3 pages. */
		    third_page_len = m->m_len - first_page_len - PAGESZ;
		    if (((xmt_pindex + 3) & NPDQXMT_MASK) != xmt_cindex) {
			xmt_blk->buff_lo = (u_int)physaddr;
			xmt_blk->long_1 = first_page_len << PI_XMT_DESCR_V_SEG_LEN;
			xmt_blk->long_1 |=  PI_XMT_DESCR_M_SOP;
			sc->xmt_vaddr[xmt_pindex].vaddr = m;
			xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
			/* get the next descriptor for the rest of the mbuf */
			xmt_blk = &sc->xmt_data_blk[xmt_pindex];
			mptr = mptr + first_page_len;
			CONVTOPHYS(mptr, &physaddr);
			xmt_blk->buff_lo = (u_int)physaddr;
			xmt_blk->long_1 = (PAGESZ) << PI_XMT_DESCR_V_SEG_LEN;
			sc->xmt_vaddr[xmt_pindex].vaddr = NULL;
			xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
			/* get the 3rd descriptor for the rest of the mbuf */
			xmt_blk = &sc->xmt_data_blk[xmt_pindex];
			mptr = mptr + first_page_len + PAGESZ;
			CONVTOPHYS(mptr, &physaddr);
			xmt_blk->buff_lo = (u_int)physaddr;
			xmt_blk->long_1 = (third_page_len) << PI_XMT_DESCR_V_SEG_LEN;
			sc->xmt_vaddr[xmt_pindex].vaddr = NULL;
			xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
		    } else {
			sc->xmt_vaddr[saved_prod].vaddr = NULL;
			xmt_pindex = saved_prod;
			return;
		    }
		}
	    }
	}
#endif /* __alpha */
	m = m->m_next;

	while (m) {
	   if (m->m_len == 0) {
	    /*
	     * The adapter will halt if zero length mbufs
	     * are transmitted.
	     */
	     m = m->m_next;
	     continue;
	   }

	    /*
	     * Check if we have filled up all the descriptors.
	     */
	    if (((xmt_pindex + 1) & NPDQXMT_MASK) == xmt_cindex) {
		int descr;
		/*
		 * If this packet has 255 mbufs or more,
		 * drop this packet. This is highly unlikely,
		 * but it is possible.
		 */
		if ((descr = xmt_pindex - saved_prod) < 0)
			descr = descr + NPDQXMT;
		if (descr == 255) {
		    m_freem(m0);
		    sc->xmt_vaddr[saved_prod].vaddr = NULL;
		} else {
		    /*
		     * Clean up the descriptors we filled in 
		     * and put the packet back on the higher layer's
		     * queue.
		     */
		    IF_PREPEND(&sc->is_if.if_snd, m0);
		}
		/*
		 * Decrement this count here as will be incrementing
		 * this later, even though we will not be xmt'ng any
		 * packets.
		 */
		sc->is_if.if_opackets--;
		xmt_pindex = saved_prod;
		break; /* out of the while(m) loop */
	    } else {

		mptr = mtod(m, caddr_t);	/* point to the data */
		xmt_blk = &sc->xmt_data_blk[xmt_pindex];
	    	CONVTOPHYS(mptr, &physaddr);
#ifdef __alpha
		{
		    u_long hi_boundary;
		    vm_offset_t physaddr2;
		    hi_boundary = (physaddr + PAGESZ) & 0xFFFFE000;
		    if (physaddr + m->m_len <= hi_boundary) {
			xmt_blk->buff_lo = (u_int)physaddr;
			xmt_blk->long_1 = (m->m_len) << PI_XMT_DESCR_V_SEG_LEN;
			xmt_blk->long_1 |= (u_int)((physaddr >> 32) & PI_XMT_DESCR_M_BUFF_HI);
			/* save the address for later use, 
			 * while freeing up mbufs.
			 */
			sc->xmt_vaddr[xmt_pindex].vaddr = NULL;
			xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
		    } else {
			int first_page_len = 0;
			first_page_len = hi_boundary - physaddr;
			CONVTOPHYS(mptr + first_page_len, &physaddr2);
			if (physaddr2 == physaddr + first_page_len) {
			    /* This is 2 contiguous pages */
			    xmt_blk->buff_lo = (u_int)physaddr;
			    xmt_blk->long_1 = (m->m_len) << PI_XMT_DESCR_V_SEG_LEN;
			    xmt_blk->long_1 |= (u_int)((physaddr >> 32) & PI_XMT_DESCR_M_BUFF_HI);
			    sc->xmt_vaddr[xmt_pindex].vaddr = NULL;
			    xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
			} else {

			    /*
			     * We have to produce 2 descriptors, for this mbuf,
			     * check if we have space.
			     */
			    if (((xmt_pindex + 2) & NPDQXMT_MASK) != xmt_cindex) {
				xmt_blk->buff_lo = (u_int)physaddr;
				xmt_blk->long_1 = (first_page_len) << PI_XMT_DESCR_V_SEG_LEN;
				xmt_blk->long_1 |= (u_int)((physaddr >> 32) & PI_XMT_DESCR_M_BUFF_HI);
				sc->xmt_vaddr[xmt_pindex].vaddr = NULL;
				xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
				/* get the next descriptor for the rest 
				 * of the mbuf 
				 */
				xmt_blk = &sc->xmt_data_blk[xmt_pindex];
				mptr = mptr + first_page_len;
				CONVTOPHYS(mptr, &physaddr);
				xmt_blk->buff_lo = (u_int)physaddr;
				xmt_blk->long_1 = (m->m_len - first_page_len) << PI_XMT_DESCR_V_SEG_LEN;
				xmt_blk->long_1 |= (u_int)((physaddr >> 32) & PI_XMT_DESCR_M_BUFF_HI);
				sc->xmt_vaddr[xmt_pindex].vaddr = NULL;
				sc->xmt_vaddr[xmt_pindex].vaddr = m;
				xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
			    } else {
				sc->xmt_vaddr[saved_prod].vaddr = NULL;
				xmt_pindex = saved_prod;
				return;
			    }
			}
		    }
		}
#else
		{
		    u_int hi_boundary;
		    vm_offset_t physaddr2;
		    int first_page_len = 0;

		    hi_boundary = ((u_int)physaddr + PAGESZ) & 0xFFFFF000;
		    
		    if (m->m_len <= PAGESZ) {
			if (physaddr + m->m_len <= hi_boundary) {
			    xmt_blk->buff_lo = (u_int)physaddr;
			    xmt_blk->long_1 = (m->m_len) << PI_XMT_DESCR_V_SEG_LEN;
			    /* save the addr. for later use, while freeing up mbufs */
			    sc->xmt_vaddr[xmt_pindex].vaddr = NULL;
			    xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
			} else {
			    first_page_len = hi_boundary - physaddr;
			    CONVTOPHYS(mptr + first_page_len, &physaddr2);
			    if (physaddr2 == physaddr + first_page_len) {
				/* This is 2 contiguous pages */
				xmt_blk->buff_lo = (u_int)physaddr;
				xmt_blk->long_1 = (m->m_len) << PI_XMT_DESCR_V_SEG_LEN;
				/* save the addr. for later use, while freeing mbufs */
				sc->xmt_vaddr[xmt_pindex].vaddr = NULL;
				xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
			    } else {
				/*
				 * We have to produce 2 descriptors, for this mbuf,
				 * check if we have space.
				 */
				if (((xmt_pindex + 2) & NPDQXMT_MASK) != xmt_cindex) {
				    xmt_blk->buff_lo = (u_int)physaddr;
				    xmt_blk->long_1 = (first_page_len) << PI_XMT_DESCR_V_SEG_LEN;
				    sc->xmt_vaddr[xmt_pindex].vaddr = NULL;
				    xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
				    /* get the next descriptor for the rest */
				    xmt_blk = &sc->xmt_data_blk[xmt_pindex];
				    mptr = mptr + first_page_len;
				    CONVTOPHYS(mptr, &physaddr);
				    xmt_blk->buff_lo = (u_int)physaddr;
				    xmt_blk->long_1 = (m->m_len - first_page_len) << PI_XMT_DESCR_V_SEG_LEN;
				    sc->xmt_vaddr[xmt_pindex].vaddr = NULL;
				    xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
				} else {
				    sc->xmt_vaddr[saved_prod].vaddr = NULL;
				    xmt_pindex = saved_prod;
				    return;
				}
			    }
			}
		    } else {
			int first_page_len = 0, second_page_len = 0, third_page_len = 0;
			first_page_len = hi_boundary - physaddr;
			second_page_len = m->m_len - first_page_len;
			
			if (second_page_len <= PAGESZ) { /* no third page */
			    CONVTOPHYS(mptr + first_page_len, &physaddr2);
			    if (physaddr2 == physaddr + first_page_len) {
				/* This is 2 contiguous pages */
				xmt_blk->buff_lo = (u_int)physaddr;
				xmt_blk->long_1 = (m->m_len) << PI_XMT_DESCR_V_SEG_LEN;
				/* save addr for later use, while freeing up mbufs */
				sc->xmt_vaddr[xmt_pindex].vaddr = NULL;
				xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
			    } else {
				/*
				 * We have to produce 2 descriptors, for this mbuf,
				 * check if we have space.
				 */
				if (((xmt_pindex + 2) & NPDQXMT_MASK) != xmt_cindex) {
				    xmt_blk->buff_lo = (u_int)physaddr;
				    xmt_blk->long_1 = first_page_len << PI_XMT_DESCR_V_SEG_LEN;
				    sc->xmt_vaddr[xmt_pindex].vaddr = NULL;
				    xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
				    
				    /* get the next descriptor for the rest of the mbuf */
				    xmt_blk = &sc->xmt_data_blk[xmt_pindex];
				    mptr = mptr + first_page_len;
				    CONVTOPHYS(mptr, &physaddr);
				    xmt_blk->buff_lo = (u_int)physaddr;
				    xmt_blk->long_1 = (m->m_len - first_page_len) << PI_XMT_DESCR_V_SEG_LEN;
				    sc->xmt_vaddr[xmt_pindex].vaddr = NULL;
				    xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
				} else {
				    sc->xmt_vaddr[saved_prod].vaddr = NULL;
				    xmt_pindex = saved_prod;
				    return;
				}
			    }
			} else { /* there are 3 pages. */
			    third_page_len = m->m_len - first_page_len - PAGESZ;
			    if (((xmt_pindex + 3) & NPDQXMT_MASK) != xmt_cindex) {
				xmt_blk->buff_lo = (u_int)physaddr;
				xmt_blk->long_1 = first_page_len << PI_XMT_DESCR_V_SEG_LEN;
				sc->xmt_vaddr[xmt_pindex].vaddr = NULL;
				xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
				/* get the next descriptor for the rest of the mbuf */
				xmt_blk = &sc->xmt_data_blk[xmt_pindex];
				mptr = mptr + first_page_len;
				CONVTOPHYS(mptr, &physaddr);
				xmt_blk->buff_lo = (u_int)physaddr;
				xmt_blk->long_1 = (PAGESZ) << PI_XMT_DESCR_V_SEG_LEN;
				sc->xmt_vaddr[xmt_pindex].vaddr = NULL;
				xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
				/* get the 3rd descriptor for the rest of the mbuf */
				xmt_blk = &sc->xmt_data_blk[xmt_pindex];
				mptr = mptr + first_page_len + PAGESZ;
				CONVTOPHYS(mptr, &physaddr);
				xmt_blk->buff_lo = (u_int)physaddr;
				xmt_blk->long_1 = (third_page_len) << PI_XMT_DESCR_V_SEG_LEN;
				sc->xmt_vaddr[xmt_pindex].vaddr = NULL;
				xmt_pindex = (xmt_pindex + 1) & NPDQXMT_MASK;
			    } else {
				sc->xmt_vaddr[saved_prod].vaddr = NULL;
				xmt_pindex = saved_prod;
				return;
			    }
			}	
		    }
		}
#endif /* __alpha */
	    }
	    m = m->m_next;
	}
	/*
	 * Mark the last descriptor of the packet.
	 */
	xmt_blk->long_1 |=  PI_XMT_DESCR_M_EOP;
	
	/*
	 * Bump up the packet count. if_opackets, is the total number of
	 * packets xmt'd by the system.
	 */
	sc->is_if.if_opackets++;

	/*
	 * write out the producer index to the adapter register.
	 */
	FTAREGWR((sc->addr_type2prod),
		 (FTAREGRD(sc->addr_type2prod) & ~PI_TYPE_2_PROD_M_XMT_DATA_PROD)| 
		(xmt_pindex  << PI_TYPE_2_PROD_V_XMT_DATA_PROD));
	FTAIOSYNC();	
    }/* if (((xmt_pindex + 1) & NPDQXMT_MASK) != xmt_cindex) */

    sc->is_if.if_flags &= ~IFF_OACTIVE;
    simple_unlock(&sc->xmt_indexes_lock);
    splx(s);
    return;
}


/*
 * Name:	ftatint()
 *
 * Arguments:
 *  	unit:		The unit to recive data on.
 *			
 * Functional Description:
 * 	This function frees up buffers that have already been transmitted. 
 *
 * Calls:
 *	ftaoutput();
 * 
 * Return Codes:
 *	None.
 *
 */
ftatint(unit)
short unit;
{
    struct fta_softc *sc =  &fta_softc[unit];
    struct ifnet *ifp = &(sc->is_if);
    int n, consumer_index;
    struct fddi_header *fh;
    struct mbuf *mtmp;

    simple_lock(&sc->xmt_indexes_lock);
    consumer_index = (sc->cons_xmt_index & PI_CONS_M_XMT_INDEX) >> 16;
    while (consumer_index != xmt_cindex) {
	if (sc->xmt_vaddr[xmt_cindex].vaddr) {
	    if (sc->xmt_vaddr[xmt_cindex].vaddr) {
		mtmp = sc->xmt_vaddr[xmt_cindex].vaddr;
		sc->is_if.if_opackets++;
		/*
 		 * Less  3 bytes because we don't transmit the 3 bytes
		 * of FDDI header.
		 */
		fstc_bytesent += mtmp->m_pkthdr.len - 3;
		if (fstc_pdusent != 0xffffffff)
			fstc_pdusent++;
		fh = mtod(mtmp, struct fddi_header *);
		/*
		 * multicast packets.
		 */
		if (fh->fddi_dhost[0] & 0x1) {
		    fstc_mbytesent += mtmp->m_pkthdr.len - 3;
		    if (fstc_mpdusent != 0xffffffff)
			    fstc_mpdusent++;
		}
		m_freem(mtmp);
		sc->xmt_vaddr[xmt_cindex].vaddr = NULL;
	    }
	} 
	xmt_cindex = (xmt_cindex + 1) & NPDQXMT_MASK;
    } 

    /*
     * write out the completion index to the adapter register.
     */
    /* adapter copy     = updated service index */
    FTAREGWR((sc->addr_type2prod),
	    (FTAREGRD(sc->addr_type2prod) &  ~PI_TYPE_2_PROD_M_XMT_DATA_COMP)| 
	     (xmt_cindex << PI_TYPE_2_PROD_V_XMT_DATA_COMP));
    FTAIOSYNC();
    simple_unlock(&sc->xmt_indexes_lock);
    /*
     * Check if there is anything to transmit.
     */
    if (sc->is_if.if_snd.ifq_head)
	ftaoutput(ifp);
}


/*
 * Name:	fta_service_rcv_q()
 *
 * Arguments:
 *  	unit:		The unit to recive data on.
 *			
 * Functional Description:
 *	This services the receive queue and is called whenever we receive
 * a receive interrupt for the RCV_DATA queue. There is a lot of common
 * code between the servicing of the SMT queue and this routine. For the
 * sake of speed these routines have been seperated.
 *
 *	The rcv data queue contains a total of 256 descriptors, each of which 
 * can point to a maximum of 8K buffer space. For this driver each descriptor
 * points to a 4K buffer and not ALL the descriptors are used. Only a block of
 * descriptors are produced (RCV_DESCR_PROD in if_fta_data.c file)
 * This avoids allocating 256 * 4K = 1024K = 1M of memory.
 * For MIPS:
 * 	If the length of the packet received is less that 108 bytes, the data
 * is copied into a small mbuf, in order to enable us to reuse the cluster
 * mbuf. If the length of the dat is less than or equal to 4096 bytes, and we
 * get a cluster mbuf to replace the one we are pulling out, we pass on the
 * mbuf to the higher layers. If the length of the data is greater than 4096
 * bytes we attempt to replace both the mbufs. Only if we are able to replace
 * the mbufs, do we take the mbufs off the adapter's queue and pass it on to
 * the protocol layers.
 *
 *	If we lack mbuf resources, we arm an interrupt that will wake us up
 * at most after 20ms. At this point we will try and service the receive 
 * queue once again.
 *
 * Fairness to transmit during prolong receives: This is achieved by
 * allowing a transmit after 'N' packets have been received. The value 
 * chosen for 'N' is up for debate. For this implementation the value is
 * 30, chosen for no particular reason. The overhead introduced to achieve
 * this "fairness" involves incrementing a counter and doing a comparision
 * for each packet received. We believe this to be about 3 to 4 instructions
 * and not an extensive amount of overhead.
 *	NOTE: There is no need to clean_dcache() because we are using
 *	uncached and unmapped memory space.
 *
 * Calls:
 *	None.
 * 
 * Return Codes:
 *	None.
 *
 */

fta_service_rcv_q(unit)
short unit;
{
    struct fta_softc *sc =  &fta_softc[unit];
    struct ifnet *ifp = &(sc->is_if);
    struct rmbuf *bp;
    PI_RCV_DESCR *rcv_blk = sc->rcv_data_blk;
    int len;		    /* The length of the packet rcv. */
    int pkts_rcvd;	    /* pkts. received while servicing this interrupt */
    int new_intr_enabled = 0; /* set if we enable the 20ms interrupt from adap*/
    pkts_rcvd = 0;
    bp = &sc->rcvmbuf[0];

    while ((sc->cons_rcv_index & PI_CONS_M_RCV_INDEX) != rcv_cindex) {
	u_int fmc_descr;
	char *data_a, *data_b;
	struct rmbuf *bp_a, *bp_b;
	struct mbuf *m, *mp, *m1;
	int len;
	vm_offset_t physaddr;

	bp_a = &sc->rcvmbuf[rcv_cindex];
	bp_b = &sc->rcvmbuf[rcv_cindex + 1];
	data_a = bp_a->mdata;
	data_b = bp_b->mdata;
#ifdef __alpha
	fmc_descr = *((u_int *)(data_a));
#else
	fmc_descr = *((u_int *)(K0_TO_K1(data_a)));
#endif /* __alpha */

	len = fmc_descr & PI_FMC_DESCR_M_LEN;

	/* point to the fddi header */
	data_a = data_a + 4;

	    
	/*
	 * start processing the queued packets.
	 */
	/*
	 * Check if the packet needs to be flushed.
	 * If not process it.
	 */

	if(!(PI_FMC_DESCR_M_RCC_FLUSH & fmc_descr)) {
	    /*
	     * Check if the frame is too long or too short.
	     */
	    if (len > FDDIMAX || len < FDDILLCMIM) { 
		if (fstc_pdulen != 0xffffffff)
			fstc_pdulen++;
		sc->is_if.if_ierrors++;
		printf("%s%d: Illegal length, packet dropped; len = %d\n", ADAP, ifp->if_unit, len);
		/*
		 * These buffers are of no use, we will 
		 * use them again.
		 */
    		simple_lock(&sc->alloc_mbufs_lock);
		MOVE2END(rcv_cindex, rcv_pindex, bp, rcv_blk);
#ifdef __alpha
		rcv_pindex = (rcv_pindex + 1) & NPDQRCV_MASK;
		rcv_cindex = (rcv_cindex + 1) & NPDQRCV_MASK;
#else
		/*
		 * Produce the 2 decriptors.
		 */
		rcv_pindex = (rcv_pindex + 2) & NPDQRCV_MASK;
		rcv_cindex = (rcv_cindex + 2) & NPDQRCV_MASK;
#endif /* __alpha */
		FTAREGWR((sc->addr_type2prod),
			 (FTAREGRD(sc->addr_type2prod) & ~PI_TYPE_2_PROD_M_RCV_DATA_PROD) | 
			 (rcv_pindex <<  PI_TYPE_2_PROD_V_RCV_DATA_PROD));
		FTAIOSYNC();
    		simple_unlock(&sc->alloc_mbufs_lock);
		continue;
	    }
           /*
            * The length reported by FTA includes one
            * byte Frame Control, real data and 4 bytes CRC.
            * The driver interprets the frame as 4 bytes
            * FDDI header ( including one byte Frame Control)
            * and real data. So, we need to decrement the
            * length by one.
            */
            len--;

#ifdef __alpha
	    FTAMCLGET(mp, 1);
	    /* point to the mbuf containing the data received */
	    m = bp_a->rmbuf;
	    if (mp) {
		/* fill in the length field of the mbuf */
		m->m_pkthdr.len = m->m_len = len;
		m->m_data = data_a;
		simple_lock(&sc->alloc_mbufs_lock);
		/*
		 * Initialize info. for the new mbuf to be produced.
		 */
		FILLRCVDESCR(mp, 
			     &physaddr,
			     bp[rcv_pindex].phymbuf,
			     bp[rcv_pindex].rmbuf,
			     bp[rcv_pindex].mdata);
	    	rcv_blk[rcv_pindex].buff_lo = (u_int)physaddr;
	    	rcv_blk[rcv_pindex].long_1 |= 
			    (u_int)((physaddr >> 32) & PI_RCV_DESCR_M_BUFF_HI);
		rcv_pindex = (rcv_pindex + 1) & NPDQRCV_MASK;
		FTAREGWR((sc->addr_type2prod),
			 (FTAREGRD(sc->addr_type2prod) & 
			  ~PI_TYPE_2_PROD_M_RCV_DATA_PROD)| 
			 (rcv_pindex << PI_TYPE_2_PROD_V_RCV_DATA_PROD));
		FTAIOSYNC();
		simple_unlock(&sc->alloc_mbufs_lock);
		ftaread(sc, m, len);
		bp[rcv_cindex].phymbuf = 0;
		bp[rcv_cindex].rmbuf = NULL;
		bp[rcv_cindex].mdata = NULL;
		rcv_cindex = (rcv_cindex + 1) & NPDQRCV_MASK;
		pkts_rcvd++;
		continue;
#else 
	    /* We do not have limitless supply of cluster
	     * mbufs. So, if the packet is smaller that 108 bytes
	     * we copy it into a small mbuf.
	     * In case of ULTRIX we don't have such a problem.
	     */
	    if (len <= MHLEN) {
		MGETHDR((mp), M_DONTWAIT, MT_DATA);
		if (mp) { /* There is mbuf available */
		    clean_dcache(data_a, len);
		    bcopy(data_a, mtod(mp, caddr_t), len);
		    mp->m_pkthdr.len = mp->m_len = len;
    		    simple_lock(&sc->alloc_mbufs_lock);
		    MOVE2END(rcv_cindex, rcv_pindex, bp, rcv_blk);
		    rcv_pindex = (rcv_pindex + 2) & NPDQRCV_MASK;
	            FTAREGWR((sc->addr_type2prod), 
			     (FTAREGRD(sc->addr_type2prod) & 
			      ~PI_TYPE_2_PROD_M_RCV_DATA_PROD)| 
			     (rcv_pindex <<  PI_TYPE_2_PROD_V_RCV_DATA_PROD));
		    FTAIOSYNC();
    		    simple_unlock(&sc->alloc_mbufs_lock);
		    ftaread(sc, mp, len); /* pass the packet up */
		    bp[rcv_cindex].phymbuf = 0;
		    bp[rcv_cindex].rmbuf = NULL;
		    bp[rcv_cindex].mdata = NULL;
		    /* we just increment the local counter
		     */
		    rcv_cindex = (rcv_cindex + 2) & NPDQRCV_MASK;
		    pkts_rcvd++;
		} else {
		    /*
		     * No buffer available. Update producer and
		     * completion indexes and enable 20ms wake up interrupt.
		     */
		    u_int indexes = 0;
		    
    		    simple_lock(&sc->alloc_mbufs_lock);
		    indexes = ((rcv_pindex << PI_TYPE_2_PROD_V_RCV_DATA_PROD) |
			       (rcv_cindex << PI_TYPE_2_PROD_V_RCV_DATA_COMP));
			    FTAREGWR((sc->addr_type2prod),
				     (FTAREGRD(sc->addr_type2prod) & 
				      ~(PI_TYPE_2_PROD_M_RCV_DATA_COMP | PI_TYPE_2_PROD_M_RCV_DATA_PROD)) | indexes);
		    FTAIOSYNC();
		    FTAREGWR((sc->addr_intenbX),
			    FTAREGRD(sc->addr_intenbX)| PI_HOST_INT_0_M_20MS);
		    FTAIOSYNC();
    		    simple_unlock(&sc->alloc_mbufs_lock);
		    if (ftadebug > 2)
		        printf("20ms interrupt enabled 1\n");
		    new_intr_enabled = 1;
		    /*
		     * Disable the receive interrupt.
		     */
	  	    FTAREGWR((sc->addr_intenbX), 
			     FTAREGRD(sc->addr_intenbX) &
			     ~PI_TYPE_X_M_RCV_DATA_ENB);
		    FTAIOSYNC();
		    ftannombuf++;
		    break;
		}
	    } else { /* length is greater than MHLEN( = 100) */
		FTAMCLGET(mp, 1);
		/* point to the mbuf containing the data received */
		m = bp_a->rmbuf;
		if (mp) {
		    if (len <= MCLBYTES - 4) {
			clean_dcache(data_a, len);
			/* fill in the length field of the mbuf */
			m->m_pkthdr.len = m->m_len = len;
			m->m_data = data_a;
    		        simple_lock(&sc->alloc_mbufs_lock);
			/*
			 * Initialize info. for the new mbuf to be produced.
			 */
			FILLRCVDESCR(mp, 
				     &rcv_blk[rcv_pindex].buff_lo,
				     bp[rcv_pindex].phymbuf,
				     bp[rcv_pindex].rmbuf,
				     bp[rcv_pindex].mdata);
		        /*
			 * Move the unused mbuf to the end.
			 */
			bp[rcv_pindex + 1].rmbuf = 
				bp[rcv_cindex + 1].rmbuf;
			bp[rcv_pindex + 1].phymbuf = 
				bp[rcv_cindex + 1].phymbuf;
			bp[rcv_pindex + 1].mdata = 
				bp[rcv_cindex + 1].mdata;
			rcv_blk[rcv_pindex + 1].buff_lo = 
					rcv_blk[rcv_cindex + 1].buff_lo;
			/* for sanity - initialize old data areas */
			bp[rcv_cindex].rmbuf = NULL;
			bp[(rcv_cindex + 1)].rmbuf = NULL;
			rcv_pindex = (rcv_pindex + 2) & NPDQRCV_MASK;
			FTAREGWR((sc->addr_type2prod),
				 (FTAREGRD(sc->addr_type2prod) & 
				  ~PI_TYPE_2_PROD_M_RCV_DATA_PROD)| 
				 (rcv_pindex << PI_TYPE_2_PROD_V_RCV_DATA_PROD));
			FTAIOSYNC();
    		        simple_unlock(&sc->alloc_mbufs_lock);
			ftaread(sc, m, len);
			bp[rcv_cindex].phymbuf = 0;
			bp[rcv_cindex].rmbuf = NULL;
			bp[rcv_cindex].mdata = NULL;
			rcv_cindex = (rcv_cindex + 2) & NPDQRCV_MASK;
			pkts_rcvd++;
			continue;
		    } else {
			FTAMCLGET(m1, 0);			
			if (m1) {
			    m->m_pkthdr.len = len;
			    clean_dcache(data_a, MCLBYTES - 4);
			    clean_dcache(data_b, len - MCLBYTES + 4);
			    m->m_data = data_a;
			    m->m_len = MCLBYTES - 4;
			    m->m_next = bp_b->rmbuf;
			    m->m_next->m_len = len - MCLBYTES + 4;

    		            simple_lock(&sc->alloc_mbufs_lock);
			    /*
			     * Init. info. for the new mbufs being produced.
			     */
			    FILLRCVDESCR(mp, 
					 &rcv_blk[rcv_pindex].buff_lo,
					 bp[rcv_pindex].phymbuf,
					 bp[rcv_pindex].rmbuf,
					 bp[rcv_pindex].mdata);
			    FILLRCVDESCR(m1,
					 &rcv_blk[rcv_pindex + 1].buff_lo,
					 bp[rcv_pindex + 1].phymbuf,
					 bp[rcv_pindex + 1].rmbuf,
					 bp[rcv_pindex + 1].mdata);
			    rcv_pindex = (rcv_pindex + 2) & NPDQRCV_MASK;
			    FTAREGWR((sc->addr_type2prod),
				     (FTAREGRD(sc->addr_type2prod) & 
				      ~PI_TYPE_2_PROD_M_RCV_DATA_PROD)| 
				     (rcv_pindex <<  PI_TYPE_2_PROD_V_RCV_DATA_PROD));
			    FTAIOSYNC();
    		            simple_unlock(&sc->alloc_mbufs_lock);
			    ftaread(sc, m, len);
			    /* for sanity - initialize old data areas */
			    bp[rcv_cindex].rmbuf = NULL;
			    bp[rcv_cindex + 1].rmbuf = NULL;
			    rcv_cindex = (rcv_cindex + 2) & NPDQRCV_MASK;
			    pkts_rcvd++;	
			    continue;
			} else {
			    u_int indexes = 0;
			    /* 
			     * We could not get the second cluster mbuf.
			     */
			    m_freem(mp);
			    /*
			     * No buffer available. Update producer and
			     * compl indexes and enable 20ms wake up interrupt.
			     */
			    simple_lock(&sc->alloc_mbufs_lock);
			    indexes = (((rcv_pindex & NPDQRCV_MASK) << PI_TYPE_2_PROD_V_RCV_DATA_PROD) | ((rcv_cindex & NPDQRCV_MASK) << PI_TYPE_2_PROD_V_RCV_DATA_COMP));
			    FTAREGWR((sc->addr_type2prod),
				     (FTAREGRD(sc->addr_type2prod) & 
				      ~(PI_TYPE_2_PROD_M_RCV_DATA_COMP | PI_TYPE_2_PROD_M_RCV_DATA_PROD)) | indexes);
			    FTAIOSYNC();
    		            simple_unlock(&sc->alloc_mbufs_lock);
			    FTAREGWR((sc->addr_intenbX),
				     FTAREGRD(sc->addr_intenbX)| 
				     PI_HOST_INT_0_M_20MS);
			    FTAIOSYNC();
			    if (ftadebug > 2)
			        printf("20ms interrupt enabled 2\n");
			    new_intr_enabled = 1;
		    	    /*
		     	     * Disable the receive interrupt.
		             */
			    FTAREGWR((sc->addr_intenbX), 
				     FTAREGRD(sc->addr_intenbX) &
				     ~PI_TYPE_X_M_RCV_DATA_ENB);
			    FTAIOSYNC();
			    ftannombuf++;
			    break;
			}
		    }
#endif /* __alpha */
		} else {
		    u_int indexes = 0;
		    /*
		     * We could not get the first cluster we requested.
		     * No buffer available. Update producer and
		     * compl indexes and enable 20ms wake up interrupt.
		     */
		    simple_lock(&sc->alloc_mbufs_lock);
		    indexes = (((rcv_pindex & NPDQRCV_MASK) << PI_TYPE_2_PROD_V_RCV_DATA_PROD) | ((rcv_cindex & NPDQRCV_MASK) << PI_TYPE_2_PROD_V_RCV_DATA_COMP));
		    FTAREGWR((sc->addr_type2prod),
			     (FTAREGRD(sc->addr_type2prod) & 
			      ~(PI_TYPE_2_PROD_M_RCV_DATA_COMP | PI_TYPE_2_PROD_M_RCV_DATA_PROD)) | indexes);
		    FTAIOSYNC();
    		     simple_unlock(&sc->alloc_mbufs_lock);
		    FTAREGWR((sc->addr_intenbX),
			     FTAREGRD(sc->addr_intenbX)| 
			     PI_HOST_INT_0_M_20MS);
		    if (ftadebug > 2)
		        printf("20ms interrupt enabled 3\n");
		    new_intr_enabled = 1;
		    /*
		     * Disable the receive interrupt.
		     */
		    FTAREGWR((sc->addr_intenbX), 
			     FTAREGRD(sc->addr_intenbX) &
			     ~PI_TYPE_X_M_RCV_DATA_ENB);
		    FTAIOSYNC();
		    ftannombuf++;
		    break;
		}
#ifndef __alpha
	    }
#endif /* __alpha */
	} else { /* if(!(PI_FMC_DESCR_M_RCC_FLUSH & fmc_descr)) { */
	    /*
	     * There was an error.
	     * Parse the descriptor to see why we should flush
	     * the packet.
	     */

	    if (ftadebug > 2) 
		    printf("%s%d: Error while receiving.\n",
			   ADAP, ifp->if_unit);		

	    if (fmc_descr == 0x01AA) {  /* 0x01AA = 110101010 */
		thread_wakeup((vm_offset_t)&sc->error_recovery_flag);
		return;
	    }

	    switch ((fmc_descr & PI_FMC_DESCR_M_RCC_RRR) >> PI_FMC_DESCR_V_RCC_RRR) {
	      case PI_FMC_DESCR_K_RRR_SUCCESS:
		/*
		 * RCC is zero. Check for other problems.
		 * CHECK FSC before doing anything!!
		 */
		if (PI_FMC_DESCR_M_RCC_CRC & fmc_descr) {
		    printf("%s%d: MAC CRC Error.\n",
			   ADAP, ifp->if_unit);
		    fstc_fcserror++;
		} else if (fmc_descr & PI_FMC_DESCR_M_FSB_ERROR) {
		    printf("%s%d: E bit set\n",
			   ADAP, ifp->if_unit);
			 /* The E bit was set */
			fstc_fseerror++;
		}
		break;
	      case PI_FMC_DESCR_K_RRR_LENGTH_BAD:
		printf("%s%d: Invalid length.\n",
		       ADAP, ifp->if_unit);
		fstc_pdualig++;
		break;
	      case PI_FMC_DESCR_K_RRR_FRAGMENT:
		/* What on earth does this mean??? **SNU***/
		printf("%s%d: Fragment Error.\n",
		       ADAP, ifp->if_unit);
		break;
	      case PI_FMC_DESCR_K_RRR_FORMAT_ERR:
		printf("%s%d: Frame Format Error!\n",
		       ADAP, ifp->if_unit);
		break;
	      case PI_FMC_DESCR_K_RRR_MAC_RESET:
		/* Ya so what if MAC reset? CHECK!! ***SNU*** */
		printf("%s%d: MAC Reset Error.\n",
		       ADAP, ifp->if_unit);
		break;
	      case PI_FMC_DESCR_K_RRR_SA_MATCH: 
		/**So what if there is a source address match **SNU** Check */
	      case PI_FMC_DESCR_K_RRR_DA_MATCH:
	      case PI_FMC_DESCR_K_RRR_FMC_ABORT:
		printf("%s%d: Network adapter hardware problem.\n",
		       ADAP, ifp->if_unit);
		/*
		 * Halt the adapter and bump up the count.
		 */
		ifp->if_flags &= ~IFF_UP;
		printf("%s%d: Network Adapter halted for reinitialization.\n",
		       ADAP, ifp->if_unit);	
		if (++(sc->halt_count) > MAX_HALTS) {
		    printf("%s%d: Halts threshold exceeded.\n",
			   ADAP, ifp->if_unit);	
		    fta_transition_state(sc, unit, PI_BROKEN);
		    return(0);
		}
		thread_wakeup((vm_offset_t)&sc->error_recovery_flag);
		return(0);
	    }
	    /*
	     * Discard the frame.
	     * Put the mbufs to the end of the list so
	     * that we can reuse them.
	     */
	    /*
	     * We did not go to the halt.
	     */
	    simple_lock(&sc->alloc_mbufs_lock);
	    MOVE2END(rcv_cindex, rcv_pindex, bp, rcv_blk);
#ifdef __alpha
	    rcv_pindex = (rcv_pindex + 1) & NPDQRCV_MASK;
#else 
	    rcv_pindex = (rcv_pindex + 2) & NPDQRCV_MASK;
#endif /* __alpha */
	    FTAREGWR((sc->addr_type2prod),
		     (FTAREGRD(sc->addr_type2prod) &  
		      ~PI_TYPE_2_PROD_M_RCV_DATA_PROD)| 
		     (rcv_pindex <<  PI_TYPE_2_PROD_V_RCV_DATA_PROD));
	    FTAIOSYNC();
	    simple_unlock(&sc->alloc_mbufs_lock);
#ifdef __alpha
	    rcv_cindex = (rcv_cindex + 1) & NPDQRCV_MASK;
#else 
	    rcv_cindex = (rcv_cindex + 2) & NPDQRCV_MASK;
#endif /* __alpha */
	}

	if (pkts_rcvd > PKTS_RCVD_THRESH) {
	    /*
	     * Keep a local counter to see how many times this
	     * happens.
	     */
	    ftafairness++;
	    if (ftadebug > 2)
	        printf("Rcv. Packet threshold reached; pkts_rcvd = %d\n", pkts_rcvd);
	    /*
	     * Check if there are any packets queued up.
	     */

	    if (sc->is_if.if_snd.ifq_head)
		    ftaoutput_one_pkt(ifp);
	    pkts_rcvd = 0;
	}
    } /* while ((sc->cons_rcv_index & PI_CONS_M_RCV_INDEX) != rcv_cindex) */
    /*
     * We are done here. Update the adapter service and producer
     * indices only if we have not enabled the 20ms interrupt.
     */
    /* adapter copy    = local copy */
    if(!new_intr_enabled) {
	u_int indexes = 0;
	simple_lock(&sc->alloc_mbufs_lock);
	indexes = (((rcv_pindex & NPDQRCV_MASK) << PI_TYPE_2_PROD_V_RCV_DATA_PROD) |
		   ((rcv_cindex & NPDQRCV_MASK) << PI_TYPE_2_PROD_V_RCV_DATA_COMP));
	FTAREGWR((sc->addr_type2prod),
		 (FTAREGRD(sc->addr_type2prod) & 
		  ~(PI_TYPE_2_PROD_M_RCV_DATA_COMP |  PI_TYPE_2_PROD_M_RCV_DATA_PROD)) | indexes);
#ifdef __alpha
        FTAIOSYNC();
#endif /* __alpha */
       simple_unlock(&sc->alloc_mbufs_lock);
    }
}


#ifdef SMT
/** NOTE: This code is untested and uncompiled. In essence this is identical
          to the fta_service_rcv_q() routine.***********/
/*
 * Name:	fta_service_smt_q()
 *
 * Arguments:
 *  	unit:		The unit to recive data on.
 *			
 * Functional Description:
 *	This services the SMT queue and is called whenever we receive
 * a host SMT interrupt. This code is similar (almost identical)
 * to fta_service_rcv_q(), but has been seperated out to make things
 * work faster.
 *	Unlike the receive path we service ONLY ONE packet and exit. We do not
 * service the entire queue.
 *
 * VERY IMPORTANT NOTE:  This code path hasn't been tested. We don't have
 *			a DLI equivalent to handle SMT packets. So, till
 *			we don't get that layer in place, this code will
 *			remain untested.
 *
 * Calls:
 *	None.
 * 
 * Return Codes:
 *	None.
 *
 */
fta_service_smt_q(unit)
short unit;
{
    struct fta_softc *sc =  &fta_softc[unit];
    struct ifnet *ifp = &(sc->is_if);
    PI_RCV_DESCR *smt_blk = sc->smt_data_blk;
    int len;		    /* The length of the packet rcv. */
    int orig_service;	    /* copy of the service index which isn't modified*/
    int saved_producer;	    /* copy of the producer index written out */
    int new_intr_enabled = 0;
    orig_service = smt_cindex;
    saved_producer = smt_pindex;

    /* Uncomment the following if the entire queue is to be serviced */
    /* while (sc->cons_hostsmt_index != smt_cindex) { */
    while(orig_service + 2 != smt_cindex) { /* process only one packet */
	FMC_RCV_DESCR fmc_descr; 
	char *data_a, *data_b;
	struct rmbuf *bp_a, *bp_b, *bp;
	struct mbuf *m, *mp, *m1;
	    
	bp = &sc->rcvmbuf[0];
	bp_a = &sc->rcvmbuf[smt_cindex];
	bp_b = &sc->rcvmbuf[smt_cindex + 1];
	data_a = bp_a->mdata;
	data_b = bp_b->mdata;
	fmc_descr.descr = ((FMC_RCV_DESCR *)data_a)->descr;
	data_a++;	/* point to the FC in the header */
	    
	/*
	 * start processing the queued packets.
	 */
	/*
	 * Check if the packet needs to be flushed.
	 * If not process it.
	 */
	if(!(PI_FMC_DESCR_M_RCC_FLUSH & fmc_descr.descr)) {
	    int len;
	    len = fmc_descr.descr & PI_FMC_DESCR_M_LEN;
	    /*
	     * Check if the frame is too long or too short.
	     */
	    if (len > FDDIMAX || len < FDDILLCMIM) { 
		if (fstc_pdulen != 0xffffffff)
			fstc_pdulen++;
		sc->is_if.if_ierrors++;
		/*
		 * These buffers are of no use, we will 
		 * use them again.
		 */
		MOVE2END(smt_cindex, smt_pindex, bp, smt_blk);
		/*
		 * Produce the 2 decriptors.
		 */
		smt_pindex = (smt_pindex + 2) & NPDQRCV_MASK;
		*(sc->addr_hostsmtprod) = 
			(*(sc->addr_hostsmtprod) & ~PI_TYPE_1_PROD_M_PROD)| 
			((smt_pindex & PI_TYPE_1_PROD_M_PROD) << 
			 PI_TYPE_1_PROD_V_PROD);
		continue;
	    } 
	    /*
	     * NOTE:
	     * We DO NOT check for the type of frame here.
	     * We can safely assume that the frame we received
	     * is an LLC because we have set filters
	     * to deliver us only the LLC frames and filter
	     * out everything else. In the future if we do
	     * have to start processing reserved and implementor
	     * specific frames this will have to be changed.
	     * Also, MAC frames never come up to the driver.
	     */

	    /* In OSF we do not have limitless supply of cluster
	     * mbufs. So, if the packet is smaller that 108 bytes
	     * we copy it into a small mbuf.
	     * In case of ULTRIX we don't have such a problem.
	     */
	    if (len <= 108) {
		MGETHDR((mp), M_DONTWAIT, MT_DATA);
		if (mp) { /* There is mbuf available */
		    bcopy(datat_a, mtod(mp, caddr_t), len);
		    mp->m_pkthdr.len = mp->m_len = len;
		    MOVE2END(smt_cindex, smt_pindex, bp, smt_blk);
		    smt_pindex = (smt_pindex + 2) & NPDQRCV_MASK;
		    *(sc->addr_hostsmtprod) = 
			(*(sc->addr_hostsmtprod) & ~PI_TYPE_1_PROD_M_PROD)| 
			((smt_pindex & PI_TYPE_1_PROD_M_PROD) << 
			 PI_TYPE_1_PROD_V_PROD);

		    ftaread(sc, mp, len); /* pass the packet up */
		    /* NOTE: we just increment the local counter and
		     * perform a modulo operation just before writing
		     * it out.
		     */
		    smt_cindex = (smt_cindex + 2);
		} else {
		    u_int indexes = 0;
		    /*
		     * No buffer available. Update producer and
		     * compl indexes and enable 20ms wake up interrupt.
		     */

		    indexes = ((rcv_pindex << PI_TYPE_2_PROD_V_RCV_DATA_PROD) |
			       (rcv_cindex << PI_TYPE_2_PROD_V_RCV_DATA_COMP));
		    *(sc->addr_type2prod) = indexes;
			    
		    *(sc->addr_intenbX) = *(sc->addr_intenbX) | 
			    PI_HOST_INT_0_M_20MS;
		    new_intr_enabled = 1;
		}
	    } else { /* length is greater than 108 */
		FTAMCLGET(mp, 1);
		/* point to the mbuf containing the data received */
		m = bp_a->rmbuf;

		if (mp) {
		    if (len <= MCLBYTES) {
#ifndef __alpha
			clean_dcache(data_a, len);
#endif /* __alpha */
			/* fill in the length field of the mbuf */
			m->m_pkthdr.len = m->m_len = len;
			/*
			 * Initialize info. for the new mbuf to be produced.
			 */
			FILLRCVDESCR(mp, 
				     &smt_blk[smt_pindex].buff_lo,
				     bp[smt_pindex].phymbuf,
				     bp[smt_pindex].rmbuf,
				     bp[smt_pindex].mdata);
		        /*
			 * Move the unused mbuf to the end.
			 */
			bp[smt_pindex + 1].rmbuf = bp[smt_cindex + 1].rmbuf;
			bp[smt_pindex + 1].phymbuf = bp[smt_cindex + 1].phymbuf;
			bp[smt_pindex + 1].mdata = bp[smt_cindex + 1].mdata;
			/* for sanity - initialize old data areas */
			bp[smt_cindex].rmbuf = NULL;
			bp[(smt_cindex + 1) & NPDQRCV_MASK].rmbuf = NULL;
			smt_pindex = (smt_pindex + 2) & NPDQRCV_MASK;
			*(sc->addr_hostsmtprod) = 
			   (*(sc->addr_hostsmtprod) & ~PI_TYPE_1_PROD_M_PROD)| 
			     ((smt_pindex & PI_TYPE_1_PROD_M_PROD) << 
			      PI_TYPE_1_PROD_V_PROD);
			ftaread(sc, m, len);
			smt_cindex = smt_cindex + 2;
		    } else {
			FTAMCLGET(m1, 0);			
			if (m1) {
#ifndef __alpha
			    clean_dcache(data_a, MCLBYTES);
			    clean_dcache(data_b, len - MCLBYTES);
#endif /* __alpha */
			    m->m_pkthdr.len = len;
			    m->m_len = MCLBYTES;
			    m->m_next = bp_b->rmbuf;
			    m->m_next->m_len = len - MCLBYTES;
			    m->m_next->m_data = data_b;
			    /*
			     * Init. info. for the new mbufs being produced.
			     */
			    FILLRCVDESCR(mp, 
					 &smt_blk[smt_pindex].buff_lo,
					 bp[smt_pindex].phymbuf,
					 bp[smt_pindex].rmbuf,
					 bp[smt_pindex].mdata);
			    FILLRCVDESCR(m1, 
					 &smt_blk[smt_pindex + 1].buff_lo,
					 bp[smt_pindex + 1].phymbuf,
					 bp[smt_pindex + 1].rmbuf,
					 bp[smt_pindex + 1].mdata);
			    smt_pindex = (smt_pindex + 2) & NPDQRCV_MASK;
			    *(sc->addr_hostsmtprod) = 
			    (*(sc->addr_hostsmtprod) & ~PI_TYPE_1_PROD_M_PROD)| 				(smt_pindex << PI_TYPE_1_PROD_V_PROD);

			    /* dli_ftaread(sc, m, len); */
			    smt_cindex = smt_cindex + 2;
			} else {
			    u_int indexes = 0;
			    /* 
			     * We could not get the second cluster mbuf.
			     * No buffer available. Update producer and
			     * compl indexes and enable 20ms wake up interrupt.
			     */
			    m_freem(mp);
			    indexes = ((rcv_pindex << PI_TYPE_2_PROD_V_RCV_DATA_PROD) |
				       (rcv_cindex << PI_TYPE_2_PROD_V_RCV_DATA_COMP));
			    *(sc->addr_type2prod) = indexes;
			    
			    *(sc->addr_intenbX) = *(sc->addr_intenbX) | 
				    PI_HOST_INT_0_M_20MS;
			    new_intr_enabled = 1;
			}
		    }
		} else {
		    u_int indexes = 0;
		    /*
		     * We could not get the first cluster we requested.
		     * No buffer available. Update producer and
		     * compl indexes and enable 20ms wake up interrupt.
		     */
		    indexes = ((rcv_pindex << PI_TYPE_2_PROD_V_RCV_DATA_PROD) |
			       (rcv_cindex << PI_TYPE_2_PROD_V_RCV_DATA_COMP));
		    *(sc->addr_type2prod) = indexes;
		    
		    *(sc->addr_intenbX) = *(sc->addr_intenbX) | 
			    PI_HOST_INT_0_M_20MS;
		    new_intr_enabled = 1;
		}
#ifdef OSF
	    }
#endif /* OSF */
	} else {
	    /*
	     * There was an error.
	     * Parse the descriptor to see why we should flush
	     * the packet.
	     */
	    if (ftadebug > 2) 
		    printf("%s%d: Error while receiving.\n",
			   ADAP, ifp->if_unit);		

	    if (fmc_descr.descr == 0x01AA) {  /* 0x01AA = 110101010 */
		fta_reinitialize(ifp, FALSE);
    		if (fta_transition_state(sc, ifp->if_unit, PI_OPERATIONAL) == NULL) {
			printf("%s%d: Could not get the adapter on line.\n",
	       			ADAP, ifp->if_unit);
		        return(1);
    		}
		ifp->if_flags |= IFF_UP;
		return(0);
	    }

	    switch ((fmc_descr.descr & PI_FMC_DESCR_M_RCC_RRR) >> 16) {
	      case PI_FMC_DESCR_K_RRR_SUCCESS:
		/*
		 * RCC is zero. Check for other problems.
		 * CHECK FSC before doing anything!!
		 * This part has to be rewritten ***SNU***
		 */
		if (PI_FMC_DESCR_M_RCC_CRC & fmc_descr.descr) {
		    printf("%s%d: MAC CRC Error.\n",
			   ADAP, ifp->if_unit);
		    fstc_fcserror++;
		} else if (fmc_descr.fmc_descr.rcv_fsb_e) /* The E bit was set */
			fstc_fseerror++;
		break;
	      case PI_FMC_DESCR_K_RRR_LENGTH_BAD:
		printf("%s%d: Invalid length.\n",
		       ADAP, ifp->if_unit);
		fstc_pdualig++;
		break;
	      case PI_FMC_DESCR_K_RRR_FRAGMENT:
		/* What on earth does this mean??? **SNU***/
		printf("%s%d: Fragment Error.\n",
		       ADAP, ifp->if_unit);
		break;
	      case PI_FMC_DESCR_K_RRR_FORMAT_ERR:
		printf("%s%d: Frame Format Error.\n",
		       ADAP, ifp->if_unit);
		break;
	      case PI_FMC_DESCR_K_RRR_MAC_RESET:
		/* Ya so what if MAC reset? CHECK!! ***SNU*** */
		printf("%s%d: MAC Reset Error.\n",
		       ADAP, ifp->if_unit);
		break;
	      case PI_FMC_DESCR_K_RRR_SA_MATCH: 
		/**So what if there is a source address mactch **SNU** Check */
	      case PI_FMC_DESCR_K_RRR_DA_MATCH:
	      case PI_FMC_DESCR_K_RRR_FMC_ABORT:
		printf("%s%d: Network adapter hardware problem!\n",
		       ADAP, ifp->if_unit);
		/*
		 * Halt the adapter and bump up the count.
		 */
		ifp->if_flags &= ~IFF_UP;
		*(sc->addr_portctrl) = PI_PCTRL_M_HALT | PI_PCTRL_M_CMD_ERROR;
		FTAIOSYNC();
		printf("%s%d:  Reinitializing network adapter...\n",
		       ADAP, ifp->if_unit);

		ifp->if_flags &= ~IFF_UP;
		
		fta_reinitialize(ifp, FALSE);
    		if (fta_transition_state(sc, ifp->if_unit, PI_OPERATIONAL) == NULL) {
			printf("%s%d: Could not get the adapter on line.\n",
	       			ADAP, ifp->if_unit);
			return(1);
		}
		ifp->if_flags |= IFF_UP;
		return(0);
	    }

	    /*
	     * Discard the frame.
	     * Put the mbufs to the end of the list so
	     * that we can reuse them.
	     */
	    if (sc->prev_state != PI_PORTINIT) {
		/*
		 * We did not go to the halt.
		 */
		MOVE2END(smt_cindex, smt_pindex, bp, smt_blk);
		smt_pindex = (smt_pindex + 2) & NPDQRCV_MASK;
		*(sc->addr_hostsmtprod) = 
			(*(sc->addr_hostsmtprod) & ~PI_TYPE_1_PROD_M_PROD)| 
				((smt_pindex << PI_TYPE_1_PROD_V_PROD);
	    }
	}
	continue;	
    } /* while (sc->cons_hostsmtindex != smt_cindex) */
    /*
     * We are done here. Update the adapter service and producer
     * indices.
     */
    if (!new_intr_enabled) {
	u_int indexes = 0;
	indexes = (((smt_pindex & NPDQRCV_MASK) << PI_TYPE_2_PROD_V_RCV_DATA_PROD) |
		   ((smt_cindex & NPDQRCV_MASK) << PI_TYPE_2_PROD_V_RCV_DATA_COMP));
	*(sc->addr_hostmstprod) = indexes;
    }
    return(1);
}
#endif /* SMT */



/*
 * Name:	fta_fmib_fill()
 *
 * Arguments:
 *  	sc:		The softc structure.
 *	fmib:		The counter structure.
 *	type:		The type of counter.
 *			
 * Functional Description:
 *	This fetches the SNMP FDDI mib attributes from the local driver
 *	data structer and copies over the needed attributes to the known
 *	user data structure.
 *
 * Calls:
 * 	
 *
 * Return Code:
 *	None.
 */
fta_fmib_fill(sc, fmib, type)
register struct fta_softc *sc;
struct ctrreq *fmib;
int type;
{
    NODATA_CMD *req_buff;
    struct ifnet *ifp = &sc->is_if;
    PI_CMD_FDDI_MIB_GET_RSP *mib_value;
    struct cmd_buf *cmdbuf;
    short retval;

    /*
     * First post a request and get the information.
     */
    req_buff = (NODATA_CMD *)kmem_alloc(kernel_map, sizeof(NODATA_CMD));

    cmdbuf = (struct cmd_buf *)kmem_alloc(kernel_map, 
					  sizeof(struct cmd_buf));
    if (cmdbuf == NULL || req_buff == NULL) {
	if (req_buff)
		kmem_free(kernel_map, req_buff, sizeof(NODATA_CMD));
	if (cmdbuf)
		kmem_free(kernel_map, 
			  cmdbuf, 
			  sizeof(struct cmd_buf));
	return(NULL);
    }

    
    req_buff->cmd_type = PI_CMD_K_FDDI_MIB_GET;
    cmdbuf->timeout = TRUE; /*set FALSE if serviced by rsp. thread */  
    cmdbuf->req_buf = (u_long *)(req_buff);
    cmdbuf->rsp_buf = NULL;

    if (sc->rsp_thread_started == FALSE) {
	retval = fta_poll_cmd_req(cmdbuf,
				  sc,
				  PI_CMD_K_FDDI_MIB_GET);
    } else {
	retval = fta_cmd_req(cmdbuf,
			     sc,
			     PI_CMD_K_FDDI_MIB_GET);
    }

    if (retval == NULL) {
	kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
	kmem_free(kernel_map, req_buff, sizeof(NODATA_CMD));
	return(NULL);
    }


    /* 
     * We get back a null if the adapter was reset even before we
     *  were able to get a response.
     */
    if (cmdbuf->rsp_buf == NULL) {
	kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
	kmem_free(kernel_map, req_buff, sizeof(NODATA_CMD));
	return(NULL);
    }

    if (((PI_RSP_HEADER *)cmdbuf->rsp_buf)->status != PI_RSP_K_SUCCESS){
	printf("%s%d: fta_fmib_fill() - Can't get MIB variables.\n",
			ADAP, ifp->if_unit);
	kmem_free(kernel_map, cmdbuf->rsp_buf, PAGESZ);/*SNU*/
	kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
	kmem_free(kernel_map, req_buff, sizeof(NODATA_CMD));
	return(0);
    }

    mib_value = (PI_CMD_FDDI_MIB_GET_RSP *)cmdbuf->rsp_buf;

    switch (type) {
      case FDDIMIB_SMT:       /* SMT group */
	/* 
	 * adapter will provide this number 
	 */
	fmib->fmib_smt.smt_number = fddi_units;
	fmib->fmib_smt.smt_index = sc->is_if.if_unit + 1;
        itoa(mib_value->smt_station_id.octet_7_4, &fmib->fmib_smt.smt_stationid[0]);
        itoa(mib_value->smt_station_id.octet_3_0, &fmib->fmib_smt.smt_stationid[4]);
	fmib->fmib_smt.smt_opversionid = (u_short)mib_value->smt_op_version_id;
	fmib->fmib_smt.smt_hiversionid = (u_short)mib_value->smt_hi_version_id;
	fmib->fmib_smt.smt_loversionid = (u_short)mib_value->smt_lo_version_id;
	fmib->fmib_smt.smt_macct = (short)mib_value->smt_mac_ct;
	fmib->fmib_smt.smt_nonmasterct = (short)mib_value->smt_non_master_ct;
	fmib->fmib_smt.smt_masterct = (short)mib_value->smt_master_ct;
	fmib->fmib_smt.smt_pathsavail = (short)mib_value->smt_paths_available;
	fmib->fmib_smt.smt_configcap = (short)mib_value->smt_config_capabilities;
	fmib->fmib_smt.smt_configpolicy = (short)mib_value->smt_config_policy;
	fmib->fmib_smt.smt_connectpolicy = (u_short)mib_value->smt_connection_policy;
	fmib->fmib_smt.smt_timenotify = (u_short)mib_value->smt_t_notify;
	fmib->fmib_smt.smt_statusreport = (short)mib_value->smt_status_reporting;
	fmib->fmib_smt.smt_ecmstate = (short)mib_value->smt_ecm_state;
	fmib->fmib_smt.smt_cfstate = (short)mib_value->smt_cf_state;
	fmib->fmib_smt.smt_holdstate = (short)mib_value->smt_hold_state;
	fmib->fmib_smt.smt_remotedisconn = (short)mib_value->smt_remote_disconnect_flag;
	fmib->fmib_smt.smt_action = (short)mib_value->smt_station_action;
	break;

      case FDDIMIB_MAC:       /* MAC group */
	fmib->fmib_mac.mac_number = fddi_units;
	fmib->fmib_mac.mac_smt_index = sc->is_if.if_unit + 1;
	fmib->fmib_mac.mac_fsc = mib_value->mac_frame_status_capabilities;
	/* greatest lower bound == 167.77224*1000/80*/
	fmib->fmib_mac.mac_gltmax = mib_value->mac_t_max_greatest_lower_bound;
	/* greatest lower bound for tvx ==  2.5*1000/80*/
	fmib->fmib_mac.mac_gltvx = mib_value->mac_tvx_greatest_lower_bound; 
	fmib->fmib_mac.mac_paths = mib_value->mac_paths_available;
	fmib->fmib_mac.mac_current = mib_value->mac_current_path;
        itoa(sc->fta_status->una.lwrd_1, &fmib->fmib_mac.mac_upstream[0]);
        itoa(sc->fta_status->una.lwrd_0, &fmib->fmib_mac.mac_upstream[4]);
        itoa(sc->fta_status->una_old.lwrd_1, &fmib->fmib_mac.mac_oldupstream[0]);
        itoa(sc->fta_status->una_old.lwrd_0, &fmib->fmib_mac.mac_oldupstream[4]);
	fmib->fmib_mac.mac_dupaddrtest = mib_value->mac_dup_addr_test;
	fmib->fmib_mac.mac_pathsreq = mib_value->mac_paths_requested;
	fmib->fmib_mac.mac_downstreamtype = mib_value->mac_downstream_port_type;
	bcopy(sc->is_dpaddr,&fmib->fmib_mac.mac_smtaddress[0],6);
	fmib->fmib_mac.mac_treq = mib_value->mac_t_req;
	fmib->fmib_mac.mac_tneg = mib_value->mac_t_neg;
	fmib->fmib_mac.mac_tmax = sc->t_max;
	fmib->fmib_mac.mac_tvx = mib_value->mac_tvx_value;
	fmib->fmib_mac.mac_tmin = mib_value->mac_t_min; /* 40000/80 */
	fmib->fmib_mac.mac_framestatus = mib_value->mac_current_frame_status;

	if (sc->ctrblk) {
	    if (sc->ctrblk->frame_cnt.ms)
		    fmib->fmib_mac.mac_counter = 0xffffffff;
	    else 
		    fmib->fmib_mac.mac_counter = sc->ctrblk->frame_cnt.ls;
	    
	    if(sc->ctrblk->error_cnt.ms)
		    fmib->fmib_mac.mac_error = 0xffffffff;
	    else
		    fmib->fmib_mac.mac_error = sc->ctrblk->error_cnt.ls;
	    
	    if(sc->ctrblk->lost_cnt.ms)
		    fmib->fmib_mac.mac_lost = 0xffffffff;
	    else
		    fmib->fmib_mac.mac_lost = sc->ctrblk->lost_cnt.ls;
	} else {	
	    fmib->fmib_mac.mac_counter = 0;	    
	    fmib->fmib_mac.mac_error = 0;
	    fmib->fmib_mac.mac_lost = 0;
	}
	 fmib->fmib_mac.mac_frame_error_thresh = mib_value->mac_frame_error_threshold;
	 fmib->fmib_mac.mac_frame_error_ratio = mib_value->mac_frame_error_ratio;
	fmib->fmib_mac.mac_rmtstate = mib_value->mac_rmt_state;
	fmib->fmib_mac.mac_dupaddr = mib_value->mac_da_flag;
	fmib->fmib_mac.mac_updupaddr = mib_value->mac_una_da_flag;
	fmib->fmib_mac.mac_condition = mib_value->mac_frame_condition;
	fmib->fmib_mac.mac_chip_set = mib_value->mac_chip_set;
	fmib->fmib_mac.mac_action = mib_value->mac_action;
	break;

      case  FDDIMIB_PORT:   /* PORT group - we are an SAS, thus only 1 port */
	fmib->fmib_port.port_number = fddi_units;
	fmib->fmib_port.port_smt_index = sc->is_if.if_unit + 1 ;
	fmib->fmib_port.port_index = 1 ; /* always one */
	fmib->fmib_port.port_pctype = mib_value->port_pc_type[0];
	fmib->fmib_port.port_pcneighbor = mib_value->port_pc_neighbor[0];
	fmib->fmib_port.port_connpolicy = mib_value->port_connection_policies[0];
	fmib->fmib_port.port_remoteind = mib_value->port_remote_mac_indicated[0];
	fmib->fmib_port.port_CEstate = mib_value->port_ce_state[0];
	fmib->fmib_port.port_pathreq = mib_value->port_paths_requested[0];
	fmib->fmib_port.port_placement = mib_value->port_mac_placement[0];
	fmib->fmib_port.port_availpaths = mib_value->port_available_paths[0];
	fmib->fmib_port.port_looptime = mib_value->port_mac_loop_time[0]; 
	fmib->fmib_port.port_TBmax = mib_value->port_tb_max[0];/* 50*1000/80 */
	fmib->fmib_port.port_BSflag = mib_value->port_bs_flag[0];
	fmib->fmib_port.port_LerrEst = mib_value->port_ler_estimate[0];

	if (sc->ctrblk) {
	    if(sc->ctrblk->lct_rejects[0].ms)
		    fmib->fmib_port.port_LCTfail = 0xffffffff;
	    else
		    fmib->fmib_port.port_LCTfail = sc->ctrblk->lct_rejects[0].ms;

	    if(sc->ctrblk->lem_rejects[0].ms)
		    fmib->fmib_port.port_Lemreject = 0xffffffff;
	    else 
		    fmib->fmib_port.port_Lemreject = sc->ctrblk->lem_rejects[0].ls;
	    /*
	     * LEM Event count seems to be missing *SNU*
	     */
	    /* fmib->fmib_port.port_Lem = sc->ctrblk->lem_event.lo; */
	} else {
	    fmib->fmib_port.port_LCTfail = 0;
	    fmib->fmib_port.port_Lemreject = 0;
	}
	fmib->fmib_port.port_Lercutoff = mib_value->port_ler_cutoff[0];
	fmib->fmib_port.port_alarm = mib_value->port_ler_alarm[0];
	fmib->fmib_port.port_connectstate = mib_value->port_connect_state[0];
	fmib->fmib_port.port_PCMstate = mib_value->port_pcm_state[0];
	fmib->fmib_port.port_PCwithhold = mib_value->port_pc_withhold[0];
	fmib->fmib_port.port_Lercondition = mib_value->port_ler_condition[0];
	fmib->fmib_port.port_action = mib_value->port_action[0];
	fmib->fmib_port.port_chip_set = mib_value->port_chip_set[0];
	break;

      case FDDIMIB_ATTA:
	fmib->fmib_atta.atta_number = fddi_units;
	fmib->fmib_atta.atta_smt_index = sc->is_if.if_unit + 1 ;
	fmib->fmib_atta.atta_index = 1 ;
	fmib->fmib_atta.atta_class = mib_value->attachment_class;
	fmib->fmib_atta.atta_bypass = mib_value->attachment_ob_present;
	fmib->fmib_atta.atta_IMaxExpiration = mib_value->attachment_imax_expiration;
	fmib->fmib_atta.atta_InsertedStatus = mib_value->attachment_inserted_status;
	fmib->fmib_atta.atta_InsertPolicy = mib_value->attachment_insert_policy;
	break;
    }
    kmem_free(kernel_map, cmdbuf->rsp_buf, PAGESZ);/*SNU*/
    kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
    kmem_free(kernel_map, req_buff, sizeof(NODATA_CMD));
    return(1);
}


/*
 * Name:	fta_smtmib_fill()
 *
 * Arguments:
 *  	sc:		The softc structure.
 *	fmib:		The counter structure.
 *	type:		The type of request.
 *			
 * Functional Description:
 *	This fetches the SNMP SMT mib attributes from the adapter and
 *	copies it into the user structure.
 *
 * Calls:
 * 	
 *
 * Return Code:
 *	None.
 */
fta_smtmib_fill(sc, mibreq, type)
register struct fta_softc *sc;
struct ctrreq *mibreq;
u_short type;
{
    NODATA_CMD *req_buff;
    struct ifnet *ifp = &sc->is_if;
    struct smtmib_smt *smt_value;
    struct smtmib_mac *mac_value;
    struct smtmib_port *port_value;
    struct smtmib_path *path_value;
    struct cmd_buf *cmdbuf;
    short retval;

    /*
     * First post a request and get the information.
     */
    req_buff = (NODATA_CMD *)kmem_alloc(kernel_map, sizeof(NODATA_CMD));

    cmdbuf = (struct cmd_buf *)kmem_alloc(kernel_map, 
					  sizeof(struct cmd_buf));
    if (cmdbuf == NULL || req_buff == NULL) {
	if (req_buff)
		kmem_free(kernel_map, req_buff, sizeof(NODATA_CMD));
	if (cmdbuf)
		kmem_free(kernel_map, 
			  cmdbuf, 
			  sizeof(struct cmd_buf));
	return(NULL);
    }

    
    req_buff->cmd_type = PI_CMD_K_SMT_MIB_GET;
    cmdbuf->timeout = TRUE; /* set FALSE if serviced by rsp. thread */  
    cmdbuf->req_buf = (u_long *)(req_buff);
    cmdbuf->rsp_buf = NULL;

    if (sc->rsp_thread_started == FALSE) {
	retval = fta_poll_cmd_req(cmdbuf,
				  sc,
				  PI_CMD_K_SMT_MIB_GET);
    } else {
	retval = fta_cmd_req(cmdbuf,
			     sc,
			     PI_CMD_K_SMT_MIB_GET);
    }

    if (retval == NULL) {
	kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
	kmem_free(kernel_map, req_buff, sizeof(NODATA_CMD));
	return(NULL);
    }


    /* 
     * We get back a null if the adapter was reset even before we
     *  were able to get a response.
     */
    if (cmdbuf->rsp_buf == NULL) {
	kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
	kmem_free(kernel_map, req_buff, sizeof(NODATA_CMD));
	return(NULL);
    }

    if (((PI_RSP_HEADER *)cmdbuf->rsp_buf)->status != PI_RSP_K_SUCCESS){
	printf("%s%d: fta_smtmib_fill() - Can't get MIB variables.\n",
			ADAP, ifp->if_unit);
	kmem_free(kernel_map, cmdbuf->rsp_buf, PAGESZ);/*SNU*/
	kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
	kmem_free(kernel_map, req_buff, sizeof(NODATA_CMD));
	return(NULL);
    }

    switch (type) {
	case FDDISMT_MIB_SMT:
     	     smt_value = (struct smtmib_smt *)(&((PI_CMD_SMT_MIB_GET_RSP *)(cmdbuf->rsp_buf))->smt_station_id);
    	    bcopy(smt_value, &mibreq->smib_smt, sizeof(struct smtmib_smt));
	    break;

	case FDDISMT_MIB_MAC:
     	    mac_value = (struct smtmib_mac *)(&((PI_CMD_SMT_MIB_GET_RSP *)(cmdbuf->rsp_buf))->mac_frame_status_functions);
    	    bcopy(mac_value, &mibreq->smib_mac, sizeof(struct smtmib_mac));
	    break;

	case FDDISMT_MIB_PATH:
     	    path_value = (struct smtmib_path *)(&((PI_CMD_SMT_MIB_GET_RSP *)(cmdbuf->rsp_buf))->path_configuration);
    	    bcopy(path_value, &mibreq->smib_path, sizeof(struct smtmib_path));
	    break;

	case FDDISMT_MIB_PORT:
     	    port_value = (struct smtmib_port *)(((PI_CMD_SMT_MIB_GET_RSP *)(cmdbuf->rsp_buf))->port_my_type);
    	    bcopy(port_value, &mibreq->smib_port, sizeof(struct smtmib_port));
	    break;
    }
    return(1);
}

/*
 * Name:	fta_get_status()
 *
 * Arguments:
 *  	sc:		The softc structure.
 *	ctr:		The counter structure pointing to the status structure.
 *			This structure needs to filled for the user.
 *			
 * Functional Description:
 *	This fills in the fstatus structure for the user read status 
 *	information.
 *
 * Calls:
 * 	
 *
 * Return Code:
 *	None.
 */
fta_get_status(sc, ctr)
register struct fta_softc *sc;
struct fstatus *ctr;
{
    PI_CMD_STATUS_CHARS_GET_RSP *fs;
    PI_CMD_DEC_EXT_MIB_GET_RSP *dec_ext;
    u_int status_reg;
    struct ifnet *ifp = &sc->is_if;
    int return_value;

    status_reg = FTAREGRD(sc->addr_portstatus); 
    /*
     * Zero out the status structure.
     */
    bzero(ctr, sizeof(struct fstatus));
	
    /*
     * Get status information from the adapter.
     */
    if ((return_value = ftastatus(ifp->if_unit)) != ESUCCESS)
	    return(return_value);

    fs = sc->fta_status;
    dec_ext = sc->dec_ext;
    /* 
     * assign the default characterists value 
     * and revision number
     */
    ctr->t_req = fs->t_req;
    ctr->t_max = sc->t_max;
    ctr->tvx   = fs->tvx;
    ctr->lem_threshold = fs->lem_threshold[0];
    ctr->rtoken_timeout = fs->token_timeout; 
    ctr->pmd_type = fs->pmd_type[0];
    ctr->smt_version = fs->smt_ver_id;
    bcopy(&fs->module_rev[0], &ctr->phy_rev[0], 4);
    bcopy(&fs->firmware_rev[0], &ctr->fw_rev[0], 4);
	
    ctr->led_state = fs->phy_led[0];
    ctr->link_state = fs->link_state;
    ctr->phy_state = fs->phy_state[0];
    ctr->dup_add_test = fs->dup_addr_flag;
    ctr->ring_purge_state = fs->purger_state;
    ctr->state = ((status_reg & PI_PSTATUS_M_STATE) >> PI_PSTATUS_V_STATE);
    /*ctr->rmt_state = fs->rmt_state; ** MISSING!!! **SNU**/
    ctr->neg_trt = fs->tneg;
    bcopy(&(((u_char *)(&fs->una))[0]),&ctr->upstream[0],6);
    bcopy(&(((u_char *)(&fs->dna))[0]),&ctr->downstream[0],6);
    /*ctr->una_timed_out = fs->una_timed_out; MISSSING!!! *SNU**/
    ctr->frame_strip_mode = fs->fci_mode;
    /*ctr->claim_token_mode = fs->claim_token_mode; MISSING!!! *SNU**/
    ctr->neighbor_phy_type = fs->nbor_phy_type[0];
    ctr->rej_reason = fs->reject_reason[0];
    ctr->phy_link_error = fs->link_error_est[0];/*IS THIS CORRECT?? *SNU*/
    bcopy(sc->is_dpaddr, &ctr->mla[0], 6);
    ctr->ring_error = fs->error_reason & 0x0f;
    bcopy(&(((u_char *)(&fs->last_dir_beacon_sa))[0]),&ctr->dir_beacon[0], 6);
    bcopy(&(((u_char *)(&fs->last_dir_beacon_una))[0]),&ctr->dir_beacon_una[0], 6);
    ctr->cnt_interval = fs->cntr_interval;
    if (dec_ext != NULL) {
	ctr->full_duplex_mode = dec_ext->efdx_enable;
	ctr->full_duplex_state = dec_ext->efdx_state;
    } else {
	ctr->full_duplex_mode = FDX_DIS;
	ctr->full_duplex_state = 0;
    }
    return(ESUCCESS);
}	


/*
 * Name:	fta_get_cntrs()
 *
 * Arguments:
 *  	sc:		The softc structure.
 *	ctr:		The counter structure pointing to the counter info.
 *			This structure needs to be filled for the user.
 *			
 * Functional Description:
 *	This fills in the latest counter values. NOTE: Since we are only
 *	reading the counters we do not lock the structure. So, there is a
 *	possibility that the counter block could get update while we read
 *	in the values.
 *
 * Calls:
 * 	
 *
 * Return Code:
 *	None.
 */
fta_get_cntrs(sc, ctr)
register struct fta_softc *sc;
struct fstat *ctr;
{
    PI_CNTR_BLK *cntrs = sc->ctrblk;
    register int seconds;

    /*
     * Fill out the fddi counters through the ethernet counter
     * "estat" structure. It is  based upon the information
     * returned by the CMD_{RDC,RCC}CNTR command and driver  
     * maintained counter. 
     */
    bzero(ctr, sizeof(struct fstat));

    seconds = fstc_second = time.tv_sec - sc->ztime;
    if (seconds & 0xffff0000)
	    ctr->fst_second = 0xffff;
    else
	    ctr->fst_second = seconds & 0xffff;

    /* driver counter */

    ctr->fst_bytercvd   = fstc_bytercvd;
    ctr->fst_bytesent   = fstc_bytesent;
    ctr->fst_pdurcvd    = fstc_pdurcvd;
    ctr->fst_pdusent    = fstc_pdusent;
    ctr->fst_mbytercvd  = fstc_mbytercvd;
    ctr->fst_mpdurcvd   = fstc_mpdurcvd;
    ctr->fst_mbytesent  = fstc_mbytesent;
    ctr->fst_mpdusent   = fstc_mpdusent;
    ctr->fst_pduunrecog  = fstc_pduunrecog = sc->is_ctrblk.est_unrecog;
    ctr->fst_mpduunrecog = fstc_mpduunrecog;
    ctr->fst_fcserror    = fstc_fcserror ;
    ctr->fst_fseerror    = fstc_fseerror ;
    ctr->fst_pdulen      = fstc_pdulen;
    ctr->fst_pdualig     = fstc_pdualig;

    if (cntrs == NULL) {
	ctr->fst_ringbeaconrecv = 0;
	ctr->fst_frame =  fstc_frame = 0;
	ctr->fst_error = fstc_error = 0;
	ctr->fst_lost =  fstc_lost = 0;
	ctr->fst_sendfail = fstc_sendfail = 0;
	ctr->fst_underrun = fstc_underrun = 0;
	ctr->fst_overrun = fstc_overrun = 0;
	ctr->fst_sysbuf = fstc_sysbuf = 0;
	ctr->fst_ringinit = fstc_ringinitrcv = 0;
	ctr->fst_ringbeacon = fstc_ringbeacon = 0;
	ctr->fst_dupaddfail = fstc_dupaddfail = 0;
	ctr->fst_ringpurge = fstc_ringpurge = 0;
	ctr->fst_duptoken = fstc_duptoken = 0;
	ctr->fst_bridgestrip= fstc_bridgestrip= 0;
	ctr->fst_traceinit= fstc_traceinit= 0;
	ctr->fst_tracerecv= fstc_tracerecv= 0;
	ctr->fst_lem_rej= fstc_lem_rej= 0;
	ctr->fst_lct_rej= fstc_lct_rej= 0;
	ctr->fst_lem_events = fstc_lem_events = 0;
	ctr->fst_connection= fstc_connection= 0;
	ctr->fst_ebf_error = fstc_ebf_error = 0;
	return;
    }
    if (cntrs->directed_beacons_rcvd.ms ||(cntrs->directed_beacons_rcvd.ls & 0xffff0000))
	    ctr->fst_ringbeaconrecv = 0xffff;
    else
	    ctr->fst_ringbeaconrecv = *(u_short*)(&cntrs->directed_beacons_rcvd.ls);

    /* adapter counter */ 
    if (cntrs->frame_cnt.ms) 
	    ctr->fst_frame =  fstc_frame = 0xffffffff;
    else
	    ctr->fst_frame = fstc_frame = cntrs->frame_cnt.ls;

    if (cntrs->error_cnt.ms)
		ctr->fst_error = fstc_error = 0xffffffff;
	else
		ctr->fst_error = fstc_error = cntrs->error_cnt.ls;

    if (cntrs->lost_cnt.ms)
	    ctr->fst_lost =  fstc_lost = 0xffffffff;
    else
	    ctr->fst_lost =  fstc_lost = cntrs->lost_cnt.ls;

    if (cntrs->xmt_failures.ms || (cntrs->xmt_failures.ls & 0xffff0000))
	    ctr->fst_sendfail = fstc_sendfail = 0xffff;
	else
	    ctr->fst_sendfail =  fstc_sendfail = *(u_short*)(&cntrs->xmt_failures.ls);

    if (cntrs->xmt_underruns.ms || (cntrs->xmt_underruns.ls & 0xffff0000))
		ctr->fst_underrun = fstc_underrun = 0xffff;
	else
		ctr->fst_underrun = fstc_underrun = *(u_short*)(&cntrs->xmt_underruns.ls);

    if ( cntrs->rcv_overruns.ms || (cntrs->rcv_overruns.ls & 0xffff0000))  
	    ctr->fst_overrun = fstc_overrun = 0xffff;
    else
	    ctr->fst_overrun = fstc_overrun = *(u_short*)(&cntrs->rcv_overruns.ls);

    if (cntrs->user_buff_unavailable.ms || 
	(cntrs->user_buff_unavailable.ls & 0xffff0000))
	    ctr->fst_userbuf = fstc_userbuf = 0xffff;
    else
	    ctr->fst_userbuf = fstc_userbuf = *(u_short*)(&cntrs->user_buff_unavailable.ls); 

    ctr->fst_sysbuf = fstc_sysbuf = (u_short)ftannombuf;

    if (cntrs->inits_initiated.ms || (cntrs->inits_initiated.ls & 0xffff0000))
		ctr->fst_ringinit = fstc_ringinitrcv = 0xffff;
	else
		ctr->fst_ringinit = fstc_ringinitrcv = *(u_short*)(&cntrs->inits_initiated.ls);

    if (cntrs->inits_rcvd.ms ||	(cntrs->inits_rcvd.ls & 0xffff0000))
	    ctr->fst_ringinitrcv = fstc_ringinitrcv = 0xffff;
    else
	    ctr->fst_ringinitrcv = *(u_short*)(&cntrs->inits_rcvd.ls);

    if (cntrs->beacons_initiated.ms || 
	(cntrs->beacons_initiated.ls & 0xffff0000))
	    ctr->fst_ringbeacon = fstc_ringbeacon = 0xfffff;
	else
		ctr->fst_ringbeacon = fstc_ringbeacon = *(u_short*)(&cntrs->beacons_initiated.ls);

    if (cntrs->dup_addrs.ms || (cntrs->dup_addrs.ls & 0xffff0000))
	    ctr->fst_dupaddfail = fstc_dupaddfail = 0xffff;
    else	
	    ctr->fst_dupaddfail = fstc_dupaddfail = *(u_short*)(&cntrs->dup_addrs.ls);

    if (cntrs->purge_errors.ms ||
			(cntrs->purge_errors.ls & 0xffff0000))
	    ctr->fst_ringpurge = fstc_ringpurge = 0xffff;
    else
	    ctr->fst_ringpurge = fstc_ringpurge = *(u_short*)(&cntrs->purge_errors.ls);

    if (cntrs->dup_tokens.ms || (cntrs->dup_tokens.ls & 0xffff0000))	
		ctr->fst_duptoken = fstc_duptoken = 0xffff;
    else
	    ctr->fst_duptoken = fstc_duptoken = *(u_short*)(&cntrs->dup_tokens.ls);

    if (cntrs->fci_strip_errors.ms || (cntrs->fci_strip_errors.ls & 0xffff0000))
	    ctr->fst_bridgestrip= fstc_bridgestrip= 0xffff;
    else
	    ctr->fst_bridgestrip= fstc_bridgestrip= *(u_short*)(&cntrs->fci_strip_errors.ls);

    if (cntrs->traces_initiated.ms || (cntrs->traces_initiated.ls & 0xffff0000))
	    ctr->fst_traceinit= fstc_traceinit= 0xffff;
    else
	    ctr->fst_traceinit= fstc_traceinit= *(u_short*)(&cntrs->traces_initiated.ls);
    /* directed beacons received is missing ***SNU**/
    if (cntrs->traces_rcvd.ms || 
	(cntrs->traces_rcvd.ls & 0xffff0000))
	    ctr->fst_tracerecv= fstc_tracerecv= 0xffff;
    else
	    ctr->fst_tracerecv= fstc_tracerecv= *(u_short*)(&cntrs->traces_rcvd.ls);

    if (cntrs->lem_rejects[0].ms || (cntrs->lem_rejects[0].ls & 0xffff0000))
	    ctr->fst_lem_rej= fstc_lem_rej= 0xffff;
    else
	    ctr->fst_lem_rej= fstc_lem_rej= *(u_short*)(&cntrs->lem_rejects[0].ls);
	
    if (cntrs->lct_rejects[0].ms || (cntrs->lct_rejects[0].ls & 0xffff0000))
	    ctr->fst_lct_rej= fstc_lct_rej= 0xffff;
    else
	    ctr->fst_lct_rej = fstc_lct_rej= *(u_short*)(&cntrs->lct_rejects[0].ls);
/************************************* NOT AVAILABLE ??? ***SNU**
    if (cntrs->tne_exp_rej.ms || ****** NOT IN ARCH SPEC ******
                        (cntrs->tne_exp_rej.ls & 0xffff0000))
                ctr->fst_tne_exp_rej= fstc_tne_exp_rej= 0xffff;
        else
                ctr->fst_tne_exp_rej = fstc_tne_exp_rej= *(u_short*)(&cntrs->tne_exp_rej.ls);
**************************************************************/
    /* maps to link errors */
    if (cntrs->link_errors[0].ms ||
                        (cntrs->link_errors[0].ls & 0xffff0000))
                ctr->fst_lem_events = fstc_lem_events = 0xffff;
        else
                ctr->fst_lem_events = fstc_lem_events = *(u_short*)(&cntrs->link_errors[0].ls);

        if (cntrs->connections[0].ms ||
                        (cntrs->connections[0].ls & 0xffff0000))
                ctr->fst_connection= fstc_connection= 0xffff;
        else
                ctr->fst_connection= fstc_connection= *(u_short*)(&cntrs->connections[0].ls) ;

    if (cntrs->ebuff_errors[0].ms || (cntrs->ebuff_errors[0].ls & 0xffff0000))
	    ctr->fst_ebf_error = fstc_ebf_error = 0xffff;
    else	
	    ctr->fst_ebf_error = fstc_ebf_error = *(u_short*)(&cntrs->ebuff_errors[0].ls);

}


/*
 * Name:	fta_addr_filter_set()
 *
 * Arguments:
 *  	sc:		The softc structure.
 *	cmd:		The command is either ADD or DELETE.
 *	e_addr:		The ethernet address in the canonical form.
 *			
 * Functional Description:
 *	This routine adds or deletes ethernet addresses to the CAM. The
 *	ethernet address to be added must be in the non-canonical form. This
 *	routine converts the address to the canonical form before adding it
 *	to CAM. If addition or deletion to the CAM is successful we update
 *	the local copy in the softc structure.
 *
 * Calls:
 * 	
 *
 * Return Code:
 *	Success:	1
 *	Failure:	0
 */
fta_addr_filter_set(sc, 
		    cmd,
		    e_addr)
struct fta_softc *sc;
short cmd;
u_char *e_addr;
{
    struct ifnet *ifp = &sc->is_if;
    int s;
    int i;
    u_long *rsp_buf;
    PI_CMD_ADDR_FILTER_SET_REQ *req_buff;
    struct cmd_buf *cmdbuf;
    short retval;

    s = splimp();
    switch(cmd) {
      case PDQ_ADD_ADDR: {
	  int i, k;
	  int j = -1;
	  /*
	   * If e_addr is NULL, we just write the current table into the 
	   * adapter. This happens when we are recovering from a fault or
	   * during boot up time.
	   */
	  if (e_addr == NULL)
		  goto add;

	  for (i = PI_CMD_ADDR_FILTER_K_FIRST + 1; 
	       i < PI_CMD_ADDR_FILTER_K_SIZE - 1; i++) {
	      if (bcmp(sc->is_multi[i], e_addr, 6) == 0) {
		  sc->is_muse[i]++;
		  splx(s);
		  return(ESUCCESS);
	      }
	      /*
	       * Mark the first available entry to use, in
	       * case we need it.
	       */
	      if ((j < 0) && (bcmp(sc->is_multi[i], fta_invalid, 8) == 0))
		      j = i;
	  }
	  if (sc->is_inuse > PI_CMD_ADDR_FILTER_K_SIZE) {
	      splx(s);
	      return(ENOBUFS);
	  } else {
	      bcopy(e_addr, &(sc->is_multi[j][0]), 6);
	      sc->is_muse[j]++;
	      sc->is_inuse++;
	  }

	  /*
	   * Now add it to the CAM
	   */
  add:
	  req_buff = (PI_CMD_ADDR_FILTER_SET_REQ *)kmem_alloc(kernel_map, 
				       sizeof(PI_CMD_ADDR_FILTER_SET_REQ));
	  cmdbuf = (struct cmd_buf *)kmem_alloc(kernel_map, 
						sizeof(struct cmd_buf));
	  if (cmdbuf == NULL || req_buff == NULL) {
	      if (req_buff)
		      kmem_free(kernel_map, 
				req_buff, 
				sizeof(PI_CMD_ADDR_FILTER_SET_REQ));
	      if (cmdbuf)
	              kmem_free(kernel_map, 
		    	        cmdbuf, 
			        sizeof(struct cmd_buf));
	      return(NULL);
	  }	  

	  req_buff->cmd_type = PI_CMD_K_ADDR_FILTER_SET;

	  for (i = 2, k = 0; i < PI_CMD_ADDR_FILTER_K_SIZE; i++, k++)
		  bcopy(&(sc->is_multi[i][0]), &(req_buff->entry[k]), 8);
	  cmdbuf->timeout = TRUE; /*set FALSE if serviced by rsp. thread */
	  cmdbuf->req_buf = (u_long *)(req_buff);
	  cmdbuf->rsp_buf = NULL;

	  if (sc->rsp_thread_started== TRUE) {
	      retval = fta_cmd_req(cmdbuf,
				   sc,
				   PI_CMD_K_ADDR_FILTER_SET);
	      if (retval == NULL) {
		  kmem_free(kernel_map, 
			    req_buff, 
			    sizeof(PI_CMD_ADDR_FILTER_SET_REQ));
		  kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
		  return(EIO);
	      }

	      /* 
	       * We get back a null if the adapter was reset even before we
	       *  were able to get a response.
	       */
	      if (cmdbuf->rsp_buf == NULL)
		      return(EIO);
	  } else { /* we poll for a response */
	      retval = fta_poll_cmd_req(cmdbuf,
					sc,
					PI_CMD_K_ADDR_FILTER_SET);
	      if (retval == NULL) {
		  kmem_free(kernel_map, 
			    req_buff, 
			    sizeof(PI_CMD_ADDR_FILTER_SET_REQ));
	          kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
	          return(0);
	       }
	   }

	  if (((PI_RSP_HEADER *)cmdbuf->rsp_buf)->status != PI_RSP_K_SUCCESS){
	      printf("%s%d: Can not add to the CAM.\n",
		     ADAP, ifp->if_unit);
	      fta_print_comp_status(((PI_RSP_HEADER *)cmdbuf->rsp_buf)->status);
	      kmem_free(kernel_map, cmdbuf->rsp_buf, PAGESZ);/*SNU*/
	      kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
	      kmem_free(kernel_map, 
			req_buff, 
			sizeof(PI_CMD_ADDR_FILTER_SET_REQ));
	      if (e_addr != NULL)
		      /*
		       * Zero out the entry we just put in and make
		       * it invalid.
		       */
		      bzero(sc->is_multi[j], 8);
	      splx(s);
	      return(EINVAL);
	  }
	  kmem_free(kernel_map, cmdbuf->rsp_buf, PAGESZ);/*SNU*/
	  kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
	  kmem_free(kernel_map, 
		    req_buff, 
		    sizeof(PI_CMD_ADDR_FILTER_SET_REQ));
	  break;
      }
      case PDQ_DEL_ADDR: {
	  int i, k;
	  for (i = PI_CMD_ADDR_FILTER_K_FIRST + 1; 
	       i < PI_CMD_ADDR_FILTER_K_SIZE - 1; i++) {
	      if (bcmp(sc->is_multi[i], e_addr ,6) == 0)
		      break;
	  }
	  if ((i < PI_CMD_ADDR_FILTER_K_SIZE - 1) && (--sc->is_muse[i] == 0)) {
		  bcopy(fta_invalid, sc->is_multi[i], 8);
		  sc->is_inuse--;
	  } else {
	      splx(s);
	      return(EINVAL);
	  }

	  /*
	   * Now rewrite the CAM table.
	   */
	  req_buff = (PI_CMD_ADDR_FILTER_SET_REQ *)kmem_alloc(kernel_map, 
				       sizeof(PI_CMD_ADDR_FILTER_SET_REQ));
	  cmdbuf = (struct cmd_buf *)kmem_alloc(kernel_map, 
						sizeof(struct cmd_buf));
	  if (cmdbuf == NULL || req_buff == NULL) {
	      if (req_buff)
		      kmem_free(kernel_map, 
				req_buff, 
				sizeof(PI_CMD_ADDR_FILTER_SET_REQ));
	      if (cmdbuf)
	              kmem_free(kernel_map, 
		    	        cmdbuf, 
			        sizeof(struct cmd_buf));
	      return(NULL);
	  }	  

	  req_buff->cmd_type = PI_CMD_K_ADDR_FILTER_SET;

	  for (i = 2, k = 0; i < PI_CMD_ADDR_FILTER_K_SIZE; i++, k++)
		  bcopy(&(sc->is_multi[i][0]), &(req_buff->entry[k]), 8);

	  cmdbuf->timeout = TRUE; /*set FALSE if serviced by rsp. thread */   
	  cmdbuf->req_buf = (u_long *)(req_buff);
	  cmdbuf->rsp_buf = NULL;
	  if (sc->rsp_thread_started == TRUE) {
	      retval = fta_cmd_req(cmdbuf,
				   sc,
				   PI_CMD_K_ADDR_FILTER_SET);
	      if (retval == NULL) {
		  kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
		  kmem_free(kernel_map, 
			    req_buff, 
			    sizeof(PI_CMD_ADDR_FILTER_SET_REQ));
		  return(EIO);
	      }
	      
	      /* 
	       * We get back a null if the adapter was reset even before we
	       *  were able to get a response.
	       */
	      if (cmdbuf->rsp_buf == NULL)
		      return(EIO);
	  } else { /* we poll for a response */
	      retval = fta_poll_cmd_req(cmdbuf,
					sc,
					PI_CMD_K_ADDR_FILTER_SET);
	      if (retval == NULL) {
	          kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
		  kmem_free(kernel_map, 
			    req_buff, 
			    sizeof(PI_CMD_ADDR_FILTER_SET_REQ));
	          return(0);
	       }
	   }

	  if (((PI_RSP_HEADER *)cmdbuf->rsp_buf)->status != PI_RSP_K_SUCCESS){
	      printf("%s%d: Can not add to the CAM.\n",
		     ADAP, ifp->if_unit);
	      fta_print_comp_status(((PI_RSP_HEADER *)cmdbuf->rsp_buf)->status);
	      kmem_free(kernel_map, cmdbuf->rsp_buf, PAGESZ);/*SNU*/
	      kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
	      kmem_free(kernel_map, 
			req_buff, 
			sizeof(PI_CMD_ADDR_FILTER_SET_REQ));
	      splx(s);
	      return(EINVAL);
	  }
	  kmem_free(kernel_map, cmdbuf->rsp_buf, PAGESZ);/*SNU*/
	  kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
	  kmem_free(kernel_map, 
		    req_buff, 
		    sizeof(PI_CMD_ADDR_FILTER_SET_REQ));
      }
    }
    splx(s);
    return(ESUCCESS);
}


/*
 * Name:	fta_free_buff()
 *
 * Arguments:
 *	unit:	The unit number of the adapter.
 *			
 * Functional Description:
 *	This routine is called if for some reason we want to initialize
 * the adapter when it is already up and runnning. In such an event
 * all the buffers have to freed before they get reinitialized.
 *
 * Calls:
 * 	None.
 *
 * Return Code:
 *	Success:	1
 *	Failure:	0
 */
fta_free_buff(unit)
int unit;
{
    int i, retval;
    struct fta_softc *sc = &fta_softc[unit];
    struct ifnet *ifp = &sc->is_if;


    /*
     * Free up all the transmit buffers.
     */
    while (xmt_cindex != xmt_pindex) {
	if (sc->xmt_vaddr[xmt_cindex].vaddr) {
	    m_freem(sc->xmt_vaddr[xmt_cindex].vaddr);
	    sc->xmt_vaddr[xmt_cindex].vaddr = NULL;
	}
	xmt_cindex = (xmt_cindex + 1) & NPDQXMT_MASK;
    }

    /*
     * Free up all the receive buffers.
     */
    while (rcv_cindex != rcv_pindex) {
	if (sc->rcvmbuf[rcv_cindex].rmbuf) {
	    m_freem(sc->rcvmbuf[rcv_cindex].rmbuf);
	    sc->rcvmbuf[rcv_cindex].rmbuf = NULL;
	}
	rcv_cindex = (rcv_cindex + 1) & NPDQRCV_MASK;
    }
    
    if (ftadebug > 2) 
        printf("Finished freeing up rcv buffers.\n");

    for (i = 0; i < NPDQCRSP; i++) {
       if (sc->rsp_buf[i] != NULL) {
    	    kmem_free(kernel_map, sc->rsp_buf[i], PAGESZ);
	    sc->rsp_buf[i] = NULL;
       }
     }
    
    if (ftadebug > 2) 
        printf("Finished freeing up cmd rsp buffers.\n");

    for (i = 0; i < NPDQUNSOL; i++) {
	if (sc->unsol_buf[i] != NULL) {
	    kmem_free(kernel_map, sc->unsol_buf[i], PAGESZ);/*SNU*/
	    sc->unsol_buf[i] = NULL;
	}
    }
    if (ftadebug > 2) 
        printf("Finished freeing up buffers.\n");

}



/*
 * Name:	fta_set_inital_chars()
 *
 * Arguments:
 *	unit:	The unit number of the adapter.
 *			
 * Functional Description:
 *	This routine set the initial characteristics on the adapter. This
 *	routine if called during boot time will set default characteristics
 *	otherwise will restore characteristics where ever possible.
 *	The following commands are issued:
 *	1) PI_CMD_K_FILTERS_SET - set filters for rcv. packets.
 *	2) PI_CMD_K_CHARS_SET - set the timer interval for receiving counters.
 *	3) PI_CMD_ADDR_FILTER_SET - add addresses to the CAM
 *	4) If the previous state of the driver was DRIVERINIT get current
 *	   characteristics set on the adapter (PI_CMD_K_STATUS_CHARS_GET).
 *	   Otherwise the characteristics that were set are restored 
 *	   (PI_CMD_K_STATUS_CHARS_SET) and the counters are also 
 *	   restored (PI_CMD_K_CNTRS_SET).
 *
 * Calls:
 * 	None.
 *
 * Return Code:
 *	Success:	1
 *	Failure:	0
 */
fta_set_initial_chars(unit)
int unit;
{
    int i, retval;
    struct cmd_buf *cmdbuf;
    struct fta_softc *sc = &fta_softc[unit];
    struct ifnet *ifp = &sc->is_if;
    PI_CMD_FILTERS_SET_REQ *req_buff;
    PI_CMD_CHARS_SET_REQ *char_set;
    u_long *rsp_buf;

    /*
     * Set the filters for receiving packets,
     * and set the timer for receiving unsolicited counters
     * to one second.
     */
    req_buff = (PI_CMD_FILTERS_SET_REQ *)kmem_alloc(kernel_map, 
				       sizeof(PI_CMD_FILTERS_SET_REQ));

    cmdbuf = (struct cmd_buf *)kmem_alloc(kernel_map, 
					  sizeof(struct cmd_buf));

    if (cmdbuf == NULL || req_buff == NULL) {
	if (req_buff)
		kmem_free(kernel_map, 
			  req_buff, 
			  sizeof(PI_CMD_FILTERS_SET_REQ));
	if (cmdbuf)
	        kmem_free(kernel_map, 
	  	          cmdbuf, 
		          sizeof(struct cmd_buf));
	return(NULL);
    }	  

    req_buff->cmd_type = PI_CMD_K_FILTERS_SET;

    for (i = 0; i < PI_CMD_FILTERS_SET_K_ITEMS_MAX; i++) {
	req_buff->item[i].item_code = sc->filters[i].item_code;
	req_buff->item[i].value = sc->filters[i].value;
    }
    cmdbuf->timeout = TRUE;
    cmdbuf->req_buf = (u_long *)(req_buff);
    cmdbuf->rsp_buf = NULL;
    if (sc->rsp_thread_started == TRUE) {
	retval = fta_cmd_req(cmdbuf,
			     sc,
			     PI_CMD_K_FILTERS_SET);

    } else { 
	retval = fta_poll_cmd_req(cmdbuf,
				  sc,
				  PI_CMD_K_FILTERS_SET);
    }

    if (retval == NULL) {
    	kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
    	kmem_free(kernel_map, 
		  req_buff, 
		  sizeof(PI_CMD_FILTERS_SET_REQ));
	return(NULL);
     }

     if (cmdbuf->rsp_buf == NULL)
	return(NULL);

    if (((PI_RSP_HEADER *)cmdbuf->rsp_buf)->status != PI_RSP_K_SUCCESS){
	printf("%s%d: Can not set receive filters on network adapter.\n",
	       ADAP, ifp->if_unit);
	fta_print_comp_status(((PI_RSP_HEADER *)cmdbuf->rsp_buf)->status);
	kmem_free(kernel_map, cmdbuf->rsp_buf, PAGESZ);
	kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
	kmem_free(kernel_map, 
		  req_buff, 
		  sizeof(PI_CMD_FILTERS_SET_REQ));
	return(NULL);
    }
    sc->fta_debug.cmdcnt.set_addr_filter++;

    kmem_free(kernel_map, cmdbuf->rsp_buf, PAGESZ);
    kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
    kmem_free(kernel_map, 
	      req_buff, 
	      sizeof(PI_CMD_FILTERS_SET_REQ));

    char_set = (PI_CMD_CHARS_SET_REQ *)kmem_alloc(kernel_map, 
				       sizeof(PI_CMD_CHARS_SET_REQ));

    cmdbuf = (struct cmd_buf *)kmem_alloc(kernel_map, 
					  sizeof(struct cmd_buf));
    if (cmdbuf == NULL || char_set == NULL) {
	if (char_set)
		kmem_free(kernel_map, 
			  char_set, 
			  sizeof(PI_CMD_CHARS_SET_REQ));
	if (cmdbuf)
	        kmem_free(kernel_map, 
	  	          cmdbuf, 
		          sizeof(struct cmd_buf));
	return(NULL);
    }	  

    char_set->cmd_type = PI_CMD_K_CHARS_SET;
    char_set->item[0].item_code = PI_ITEM_K_CNTR_INTERVAL;
    char_set->item[0].value = 1;	
    char_set->item[1].item_code = PI_ITEM_K_EOL;

    cmdbuf->timeout = TRUE; /* set FALSE if serviced by rsp. thread */
    cmdbuf->req_buf = (u_long *)(char_set);
    cmdbuf->rsp_buf = NULL;

    if (sc->rsp_thread_started == TRUE) {
	retval = fta_cmd_req(cmdbuf,
			     sc,
			     PI_CMD_K_CHARS_SET);
    } else {
	retval = fta_poll_cmd_req(cmdbuf,
				  sc,
				  PI_CMD_K_CHARS_SET);
    }
    if (retval == NULL) {
	kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
	kmem_free(kernel_map, 
		  char_set, 
		  sizeof(PI_CMD_CHARS_SET_REQ));
	return(NULL);
    }

    /* 
     * We get back a null if the adapter was reset even before we
     *  were able to get a response.
     */
    if (cmdbuf->rsp_buf == NULL)
	    return(NULL);

    if (((PI_RSP_HEADER *)cmdbuf->rsp_buf)->status != PI_RSP_K_SUCCESS){
	printf("%s%d: Can not set timer value.\n",
	       ADAP, ifp->if_unit);
	fta_print_comp_status(((PI_RSP_HEADER *)cmdbuf->rsp_buf)->status);
	kmem_free(kernel_map, cmdbuf->rsp_buf, PAGESZ);/*SNU*/
	kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
	kmem_free(kernel_map, 
		  char_set, 
		  sizeof(PI_CMD_CHARS_SET_REQ));
	return(NULL);
    }
    kmem_free(kernel_map, cmdbuf->rsp_buf, PAGESZ);/*SNU*/
    kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
    kmem_free(kernel_map, 
	      char_set, 
	      sizeof(PI_CMD_CHARS_SET_REQ));
    sc->fta_debug.cmdcnt.set_char++;

    /*
     * Add the multicast  addresses to the CAM.
     * from the default list. The list is maintained
     * by fta_addr_filter_set() routine. By passing
     * NULL as its third argument it uses the default
     * list.
     */
    if (fta_addr_filter_set(sc, PDQ_ADD_ADDR, NULL) != ESUCCESS) {
	printf("%s%d: Can not add addresses to the CAM.\n",
	       ADAP, ifp->if_unit);
    }
    if (sc->prev_state == PI_DRIVERINIT) {
	/*
	 * Get the default characteristics set on the
	 * adapter currently.
	 */
	if (ftastatus(ifp->if_unit) == EIO) {
	    sc->char_val.valid = 0;
	    printf("%s%d: Can not get characteristic values.\n",
		   ADAP, ifp->if_unit);
	} else if (sc->fta_status != NULL) {
	    /*
	     * Copy in the default characteristic values.
	     */
	    sc->char_val.treq = sc->fta_status->t_req;
	    sc->char_val.tvx = sc->fta_status->tvx;
	    sc->char_val.rtoken = sc->fta_status->token_timeout;
	    sc->char_val.ring_purger = sc->fta_status->purger_enb;
	    sc->char_val.cnt_interval = sc->fta_status->cntr_interval;
	    sc->char_val.lem0 = sc->fta_status->link_error_est[0];
	    sc->char_val.loopback = sc->fta_status->loopback;
	    if (sc->dec_ext == NULL)
	        sc->char_val.full_duplex_mode = FDX_DIS; /* disabled */
	    else 
	        sc->char_val.full_duplex_mode = sc->dec_ext->efdx_enable;
	    sc->char_val.valid = 1;
	}
    } else { /* (sc->prev_state != PI_DRIVERINIT) */
	/*
	 * We need to restore values from the stored values
	 * in struct char_val.
	 * Prepare the list.
	 */

	char_set = (PI_CMD_CHARS_SET_REQ *)kmem_alloc(kernel_map, 
				       sizeof(PI_CMD_CHARS_SET_REQ));

	cmdbuf = (struct cmd_buf *)kmem_alloc(kernel_map, 
					      sizeof(struct cmd_buf));
	if (cmdbuf == NULL || char_set == NULL) {
	    if (char_set)
		    kmem_free(kernel_map, 
			      char_set, 
			      sizeof(PI_CMD_CHARS_SET_REQ));
	    if (cmdbuf)
	            kmem_free(kernel_map, 
	  	              cmdbuf, 
		              sizeof(struct cmd_buf));
	    return(NULL);
	}	  

	if (sc->char_val.valid) {
	    char_set->cmd_type = PI_CMD_K_CHARS_SET;
	    char_set->item[0].item_code = PI_ITEM_K_T_REQ;
	    char_set->item[0].value = sc->char_val.treq;
	    char_set->item[0].item_index = 0;
	    char_set->item[1].item_code = PI_ITEM_K_TVX;
	    char_set->item[1].value = sc->char_val.tvx;
	    char_set->item[1].item_index = 0;
	    char_set->item[2].item_code = PI_ITEM_K_RESTRICTED_TOKEN;
	    char_set->item[2].value = sc->char_val.rtoken;
	    char_set->item[2].item_index = 0;
	    char_set->item[3].item_code = PI_ITEM_K_RING_PURGER;
	    char_set->item[3].value = sc->char_val.ring_purger;
	    char_set->item[3].item_index = 0;
	    char_set->item[4].item_code = PI_ITEM_K_CNTR_INTERVAL;
	    char_set->item[4].value = sc->char_val.cnt_interval;
	    char_set->item[4].item_index = 0;
	    char_set->item[5].item_code = PI_ITEM_K_LEM_THRESHOLD;
	    char_set->item[5].value = sc->char_val.lem0;
	    char_set->item[5].item_index = 0;
	    char_set->item[6].item_code = PI_ITEM_K_LOOPBACK_MODE;
	    char_set->item[6].value = sc->char_val.loopback;
	    char_set->item[6].item_index = 0;
	    char_set->item[7].item_code = PI_ITEM_K_EOL;
	    
	    cmdbuf->timeout = TRUE; /* set FALSE if serviced by rsp. thread */		  
	    cmdbuf->req_buf = (u_long *)(char_set);
	    cmdbuf->rsp_buf = NULL;
	    
	    if (sc->rsp_thread_started == TRUE) {
		retval = fta_cmd_req(cmdbuf,
				     sc,
				     PI_CMD_K_CHARS_SET);
	    } else {
		retval = fta_poll_cmd_req(cmdbuf,
					  sc,
					  PI_CMD_K_CHARS_SET);
	    }

	    if (retval == NULL) {
		kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
		kmem_free(kernel_map, 
			  char_set, 
			  sizeof(PI_CMD_CHARS_SET_REQ));
		return(NULL);
	    }
	    
	    
	    /* 
	     * We get back a null if the adapter was reset even before we
	     *  were able to get a response.
	     */

	    if (cmdbuf->rsp_buf == NULL)
		    return(NULL);
	    if (((PI_RSP_HEADER *)cmdbuf->rsp_buf)->status != PI_RSP_K_SUCCESS){
		printf("%s%d: Can not set characteristic value.\n",
		       ADAP, ifp->if_unit);
		fta_print_comp_status(((PI_RSP_HEADER *)rsp_buf)->status);
		kmem_free(kernel_map, cmdbuf->rsp_buf, PAGESZ);/*SNU*/
		kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
		kmem_free(kernel_map, 
			  char_set, 
			  sizeof(PI_CMD_CHARS_SET_REQ));
		return(NULL);
	    }
	    kmem_free(kernel_map, cmdbuf->rsp_buf, PAGESZ);/*SNU*/
	    kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
	    kmem_free(kernel_map, 
		      char_set, 
		      sizeof(PI_CMD_CHARS_SET_REQ));
	} else {
	    printf("%s%d: Can not restore characteristic values.\n",
		   ADAP, ifp->if_unit);
	}
	return(1);
	/*
	 * Restore the counters.
	 */
	{
	    PI_CMD_CNTRS_SET_REQ *data;

	    data = (PI_CMD_CNTRS_SET_REQ *)kmem_alloc(kernel_map, 
				       sizeof(PI_CMD_CNTRS_SET_REQ));

	    cmdbuf = (struct cmd_buf *)kmem_alloc(kernel_map, 
						  sizeof(struct cmd_buf));
	    if (cmdbuf == NULL || data == NULL) {
		if (data)
			kmem_free(kernel_map, 
				  char_set, 
				  sizeof(PI_CMD_CNTRS_SET_REQ));
	        if (cmdbuf)
	                kmem_free(kernel_map, 
	  	                  cmdbuf, 
		                  sizeof(struct cmd_buf));
		printf("%s%d: Can not restore counters.\n",
		       ADAP, ifp->if_unit);
		return(NULL);
	    }	  

	    data->cmd_type = PI_CMD_K_CNTRS_SET;
	    bcopy(sc->ctrblk, &data->cntrs, sizeof(PI_CNTR_BLK));

	    cmdbuf->timeout = TRUE; /*set FALSE if serviced by rsp. thread */
	    cmdbuf->req_buf = (u_long *)(data);
	    cmdbuf->rsp_buf = NULL;

	    if (sc->rsp_thread_started == TRUE) {
		retval = fta_cmd_req(cmdbuf,
				     sc,
				     PI_CMD_K_CNTRS_SET);
	    } else {
		retval = fta_poll_cmd_req(cmdbuf,
					  sc,
					  PI_CMD_K_CNTRS_SET);
	    }

	    if (retval == NULL) {
		kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
		kmem_free(kernel_map, 
			  char_set, 
			  sizeof(PI_CMD_CNTRS_SET_REQ));
		printf("%s%d: Can not restore counters.\n",
		       ADAP, ifp->if_unit);

		return(NULL);
	    }


	    /* 
	     * We get back a null if the adapter was reset even before we
	     *  were able to get a response.
	     */
	    if (cmdbuf->rsp_buf == NULL)
		    return(NULL);

	    if (((PI_RSP_HEADER *)cmdbuf->rsp_buf)->status != 
		PI_RSP_K_SUCCESS){
		printf("%s%d: Could not restore counters!\n",
		       ADAP, ifp->if_unit);
		fta_print_comp_status(((PI_RSP_HEADER *)rsp_buf)->status);
	    }
	    kmem_free(kernel_map, cmdbuf->rsp_buf, PAGESZ);/*SNU*/
	    kmem_free(kernel_map, cmdbuf, sizeof(struct cmd_buf));
	    kmem_free(kernel_map, 
		      char_set, 
		      sizeof(PI_CMD_CNTRS_SET_REQ));	    
	}
    }
    return(1);
}

/*
 * Name:	fta_set_dma_burst_size()
 *
 * Arguments:
 *	sc	The softc structure.
 *	size:	The burst size (0, 8, 16 or 32)
 *			
 * Functional Description:
 *	This sets the DMA burst size for the adapter.
 *
 * Calls:
 * 	None.
 *
 * Return Code:
 *	Success:	1
 *	Failure:	0
 */
fta_set_dma_burst_size(sc, size)
struct fta_softc *sc;
int size;
{
    int count;
    struct ifnet *ifp = &sc->is_if;
    
    FTAREGWR(sc->addr_portdataA, PI_SUB_CMD_K_BURST_SIZE_SET);
    FTAREGWR(sc->addr_portdataB, size);
    FTAIOSYNC();

    FTAREGWR((sc->addr_portctrl), PI_PCTRL_M_SUB_CMD | PI_PCTRL_M_CMD_ERROR);
    FTAIOSYNC();

    /*	
     * Check if the CSR command done bit is set and the 
     * command error bit is not set.
     */
    count = 0;
    while(FTAREGRD(sc->addr_portctrl) & PI_PCTRL_M_SUB_CMD) {
 	DELAY(1000);
 	count++;
 	if (count > 1000) {
 	    printf("%s%d: Command timeout while setting DMA burst size\n", ADAP, ifp->if_unit);
 	    return(0);
 	}	
    }
    if (FTAREGRD(sc->addr_portctrl) & PI_PCTRL_M_CMD_ERROR) {
 	printf("%s%d: Error while setting DMA burst size.\n", ADAP, ifp->if_unit);
 	return(0);
    } 
    return(1);
}
#endif 
