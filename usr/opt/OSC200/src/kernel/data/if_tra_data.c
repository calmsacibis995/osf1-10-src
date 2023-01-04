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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: if_tra_data.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/05/25 22:06:29 $";
#endif
*/
#include "tra.h"
#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/buf.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <sys/kernel.h>  
#include <vm/vm_kern.h>  
#include <sys/syslog.h>  
#include <dec/binlog/errlog.h>

#include <net/if.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/in_systm.h>
#include <netinet/if_ether.h>
#include <netinet/if_trn.h>
#include <net/ether_driver.h>
#include <hal/cpuconf.h>
#include <io/common/devdriver.h>
#include <io/dec/uba/ubavar.h>
#include <io/dec/tc/tc.h>

#include <kern/thread.h>
#include <kern/sched_prim.h>

#include <io/dec/netif/if_trareg.h>


#define TRAIOSYNC()	mb()
#define TRAREGWR(csr, value)	(*(csr)) = ((unsigned long)((0x03L<<32)|((unsigned long)(value))));
/*
 *  This macro reads a DETRA register from sparse space.  We don't bother
 *  reading the bytemask longword as its read value is unpredictable.
 */
#define TRAREGRD(csr) (*(volatile unsigned short *)(csr))
#define PAGESZ	8192

/*
 * This the maximum number of transmit and receive parameter lists that are
 * preallocated.
 */
#define XMITSZ	25 /* Total 25*3 buffers can be transmitted at a time.*/
#define RCVSZ	16 /* 16 pairs of a small and a cluster mbuf */

/*
 * For the OPEN command.
 *			The board has 512K of RAM of which 392K is useable
 *			for receive and transmit.
 */
#define BUFFER_SIZE	2048
#define XMIT_BMIN_CNT	10
#define XMIT_BMAX_CNT	20

/*
 * Memory reserve for SSB and SCB blocks.
 * We pre-allocate some memory that will be use for 
 * allocating memory for the SSB and SCB blocks. These blocks
 * have to be aligned on an even word boundary. So, we allocate some
 * extra memory in order to find the alignment.
 */

u_char ssb_pool[NTRA][32];
u_char scb_pool[NTRA][32];
/*
 * Structure to read data into the driver.
 */
ERROR_LOG error_log;
/*
 * The error log table.
 * Used to maintain the counts by the driver.
 */

struct error_log_table {
    u_int	line_error;
    u_int	burst_error;
    u_int	ari_fci_error;
    u_int	lost_frame_error;
    u_int	rcv_congestion_error;		/* receive overrun */
    u_int	frame_copied_error;
    u_int	token_error;
    u_int	dma_bus_error;
    u_int	dma_parity_error;
};

struct tra_counts {
    u_short	ring_poll_error;
    u_short	transmit_timeout;
    u_short	board_reset;
    u_short	small_packet;
    u_short	access_pri_error;
    u_short	unenabled_mac_frame;
    u_short	illegal_frame_format;
    u_short	no_sysbuffers;
    u_short	signal_loss;
    u_short	hard_error;
    u_short	soft_error;
    u_short	xmit_beacon;
    u_short	xmit_fail;
    u_short	lobe_wire_fault;
    u_short	auto_rem_error;
    u_short	remove_received;
    u_short	counter_overflow;
    u_short	single_station;
    u_short	ring_recovery;
    u_short	dio_parity;
    u_short	read_timeout_abort;
    u_short	read_parity_error_abort;
    u_short	read_bus_error_abort;
    u_short	write_timeout_abort;
    u_short	write_parity_error_abort;
    u_short	write_bus_error_abort;
    u_short	illegal_opcode;
    u_short	parity_error;
    u_short	ram_data_error;
    u_short	ram_parity_error;
    u_short	ring_underrun;
    u_short	invalid_internal_interrupt;
    u_short	invalid_error_interrupt;
    u_short	invalid_xop_request;
    u_short	fatal_error;
    u_short	selftest_fail;
};

struct trastat {
    u_long  tra_bytercvd;           /* bytes received */
    u_long  tra_bytesent;           /* bytes sent */
    u_long  tra_pdurcvd;            /* data blocks received */
    u_long  tra_pdusent;            /* data blocks sent */
    u_long  tra_mbytercvd;          /* multicast bytes received */
    u_long  tra_mpdurcvd;           /* multicast blocks received */
    u_long  tra_mbytesent;          /* multicast bytes sent */
    u_long  tra_mpdusent;           /* multicast blocks sent */
    u_long  tra_pduunrecog;	    /* frame unrecognized */
};

struct scb_wait_q {
    SYSTEM_COMMAND_BLOCK       	*scb;
    struct scb_wait_q 		*next;
};

struct ssb_wait_q {
    SYSTEM_STATUS_BLOCK       	*ssb;
    struct ssb_wait_q 		*next;
};

struct xmit_list {
    XMIT_PARM_LIST	*list;		/* The xmit list seen by the adapter */
    u_int		physaddr;	/* The physical address of "*list" */
    struct xmit_list	*next_list;	/* next pointer - used by driver. */
    struct mbuf		*xmit_mbuf;	/* The address of the data buffer */
};

#define XMIT_LIST struct xmit_list

struct rcv_list {
    RCV_PARM_LIST	*list;		/* The rcv list seen by adapter */
    u_int		physaddr;	/* lower portion of param phys. addr */
    struct rcv_list	*next_list;	/* next pointer */
    struct mbuf		*rcv_mbuf0;	/* Address of the mbuf */
    struct mbuf		*rcv_mbuf1;	/* Address of the mbuf */
    struct mbuf		*rcv_mbuf2;	/* Address of the mbuf */
};

#define RCV_LIST struct rcv_list
#define TRN_MAC_HDR_SIZE 32	/* Size of mac header 14 + rif(18) */
/*
 * The following union is used for making our life easier in using the 
 * TMS380 chip which needs things in terms of 16 bits and in the network
 * byte order for which we use htons() whereever required. This ain't an 
 * chip designed to keep you sane ;-).
 * For example: ABCDEF01 will be broken into:
 *	value = ABCDEF01
 *	lo_short = htons(ABCD);
 *	hi_short = htons(EF01);
 *	
 */
union converter {
    struct {
	u_short lo_bytes;
	u_short	hi_bytes;
    } segment;
    u_int	value_int;
};

/*
 * This macro converts a host 32 bit value to the TMS format of 2 16
 * bit values.
 */
#define HOST_TO_TMS(value, hi, lo){\
        union converter convert;\
	convert.value_int = value;\
	lo = htons(convert.segment.lo_bytes);\
	hi = htons(convert.segment.hi_bytes);\
}

/*
 * This macro converts a TMS two 16 bit values to the HOST format of 32
 * bit value.
 */
#define TMS_TO_HOST(value, hi, lo){\
        union converter convert;\
	convert.segment.lo_bytes = ntohs(lo);\
	convert.segment.hi_bytes = ntohs(hi);\
	value = convert.value_int;\
}

/*
 * The product ID.
 */
#define PRODUCT_ID	"DEC DETRA 4/16"
#define PIDSZ 		15

#define MAXCMD	16
/*
 * Token Ring software status per interface.
 *
 * Each interface is referenced by a network interface structure,
 * ds_if, which the routing code uses to locate the interface.
 */
struct	tra_softc {
	struct	   ether_driver is_ed;		/* driver */
#define	is_ac	is_ed.ess_ac			/* common part */
#define	ztime	is_ed.ess_ztime			/* Time since last zeroed */
#define	is_if	is_ac.ac_if			/* network-visible interface */
#define	is_addr	is_ac.ac_enaddr			/* hardware address */
	u_char	is_dpaddr[6];			/* default physical address */
	vm_offset_t basereg;			/* base register  */
	INIT_PARM_BLOCK   init_block;           /* initialization options */
	SYSTEM_COMMAND_BLOCK 	*scb;		/* system command block */
	SYSTEM_STATUS_BLOCK	*ssb;		/* system status block */
	SYSTEM_STATUS_BLOCK	ssb_copy;	/* copy of the ssb */
	XMIT_LIST		xmitlist[XMITSZ];/* The xmit list */
	XMIT_LIST		*xmit_ptr;	/* buffer avail. for xmit */
	XMIT_LIST		*xmit_done_ptr; /* buffer thats been xmitted */
	RCV_LIST		rcvlist[RCVSZ];/* The rcv. list */
	RCV_LIST		*rcv_ptr;	/* rcv buffer to process */
	OPEN_PARM_BLOCK		open_block;	/* The open parm. block */
	u_short			time_init_flag; 
	u_short			mac_open;	/* 1 == open; 0 == close */
	u_short			alloc_index;
	u_char			product_id[PIDSZ];/* The product ID */
	u_short			parm_align;
	u_int			grp_addr;	
	u_int			func_addr;	/* Functional address	*/
	u_short			ring_speed;	/* The current ring speed */
	u_short			last_speed;	/* The last ring speed */
	u_short			ring_status;	/* Ring status */
	u_short			ring_state;	/* Ring state */
	u_short			open_status;	/* Open status */
	u_short			etr;	  /* Early Token Release true/false */
	u_short			mon_cntd; /* Monitor Contender: true/false */
	u_short			pad_rif;	/* pad routing field */
	struct error_log_table	error_cnts;	/* The error log counters */
	struct tra_counts	count;		/* some local counts */
	struct trastat		tracount;	/* packet counters */
	vm_offset_t		error_log_read_flag;
	vm_offset_t		error_recovery_flag;
	u_short			rcv_ring_size;
	lock_data_t		scb_clear_lock;
	u_short			scb_clear_flag;	/* Is SCB available? */
	struct scb_wait_q	*scb_q;
	struct ssb_wait_q	ssb_q[MAXCMD];
	u_long		 	*tms_sifdat;	/* addresses of registers */
	u_long		 	*tms_sifdat_inc;/* these are in sparse space */
	u_long		 	*tms_sifadr1;
	u_long		 	*tms_sifcmd_sts;
	u_long		 	*tms_sifacl;
	u_long		 	*tms_sifadr2;
	u_long		 	*tms_sifadx;
	u_long		 	*tms_dmalen;
	u_long		 	*csr0_leds;
	u_long		 	*csr1_link_ctrl;
	u_long			*csr2;
	u_long		 	*csr3_sw_board_reset;
	u_long		 	*csr4_intr_status;
	u_long			*csr5;
	u_long			*csr6;
	u_long			*csr7;
	u_int			*mac_location;
	u_int			mac_size;
	u_int			*ace_location;
	u_int			ace_size;
	u_int			*speed_location;
	u_int			speed_size;
	u_int			*error_location;
	u_int			error_size;
	ADAP_INTERNAL_POINTERS	adap_ptr;
	char                    *microcode_type;
	char                    *device_type;
	u_char                  si_rev[3];
	u_char                  micro_level[4];
};

#ifdef BINARY
extern	struct 	tra_softc tra_softc[];
extern	struct	controller *trainfo[];
extern  XMIT_PARM_LIST tra_xmit_list_pool[];
extern  RCV_PARM_LIST tra_rcv_list_pool[];
#else BINARY
#if NTRA > 0
struct 	tra_softc tra_softc[NTRA];
struct	controller *trainfo[NTRA];
RCV_PARM_LIST	tra_rcv_list_pool[NTRA * (RCVSZ + 1)];
XMIT_PARM_LIST	tra_xmit_list_pool[NTRA * (XMITSZ + 1)];
#else 
struct 	tra_softc *tra_softc;
struct	controller *trainfo;
XMIT_PARM_LIST *tra_xmit_list_pool;
RCV_PARM_LIST *tra_rcv_list_pool;
#endif	
#endif	BINARY


