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
 * Modification History: machine/alpha/ka_cobra.h
 *
 * 07-Oct-1991 ajb   Initial entry
 *
 */

#ifndef KN430_H
#define KN430_H

#include <sys/types.h>

/*****************************************************************************
 * DEC 4000 HWRPB DEVICE TABLE						     *
 *****************************************************************************
 *
 * This is defined and filled in by the DEC 4000 console.
 */
#define FAST_SCSI 1
#define MAX_DEVICE 16
#define RPB_CONFIG_VERSION 1
enum { 
	NCR53C710_SCSI = 1, 
	TGEC = 2,
	IIC = 9,
	NCR53C710_DSSI = 10,
	NCR53C710_UNKN = 100
	}; 

struct rpb_config {
	u_long size;
	u_long chk_sum;
	u_int version;
	u_int ndevs;
	struct rpb_device {
		u_char hose;
		u_char slot;
		u_short nvect;
		u_short vms_vector;
		u_short osf_vector;
		u_int dev_type;
		u_int dev_version;
		union dev_spec {
			u_long specific_max[2];
			struct scsi_spec {
				u_char bus_id;
				u_char fast;
				u_short speed;
			} scsi;
			struct dssi_spec {
				u_char bus_id;
			} dssi;
			struct tgec_spec {
				u_char sa[6];
				u_char mode;
				u_char burst;
			} tgec;
		} dev;
	} rpb_device[MAX_DEVICE];
};

/*****************************************************************************
 * DEC 4000 CSR PHYSICAL ADDRESSES					     *
 *****************************************************************************
 *
 * Each node's I/O registers on the CBus all fit within one 8k page.
 *
 * Memory on the CBus is laid out as follows:
 *
 *                                                +------------+ 2 0000 0000
 *                                             /  | CPU CSRs   |            
 *                                            /   +------------+ 2 1000 0000
 *  0 0000 0000 +------------------------+   /    | I/O CSRs   |            
 *              | Physical Memory (2 GB) |  /     +------------+ 2 2000 0000
 *  0 8000 0000 +------------------------+ /      | Reserved   |            
 *              | Reserved Memory (6 GB) |/       +------------+            
 *  2 0000 0000 +------------------------+        | Reserved   |            
 *              | Primary I/O (2GB)	 |        +------------+ 2 4000 0000
 *  2 8000 0000 +------------------------+        | Mem 0 CSRs |            
 *              | Reserved (6GB)         |\       +------------+ 2 5000 0000
 *              +------------------------+ \      | Mem 1 CSRs |            
 *                                          \     +------------+ 2 6000 0000
 *                                           \    | Mem 2 CSRs |            
 *                                            \   +------------+ 2 7000 0000
 *                                             \  | Mem 3 CSRs |            
 *                                                +------------+            
 *                                              
 */
#define CPU0_BASE		0x200000000L
#define CPU1_BASE		0x208000000L
#define IO_BASE			0x210000000L
#define MEM0_BASE		0x240000000L
#define MEM1_BASE		0x250000000L
#define MEM2_BASE		0x260000000L
#define MEM3_BASE		0x270000000L
#define KN430_MEM_STRIDE	0x10000000L

/*****************************************************************************
 * DEC 4000 INTERNAL BUS (CBus) CONFIGURATION INFORMATION		     *
 *****************************************************************************/

/*
 * Hard configuration information (maximum number of slots).  The DEC 4000
 * supports a maximum of 2 processors.  For uniprocessor compilation, only
 * provide for one CPU.
 */
#define CBUS_IO_SLOTS 1
#define CBUS_MEM_SLOTS 4

#define CBUS_MAX_CPU_SLOTS 2
#if NCPUS >= CBUS_MAX_CPU_SLOTS
#define CBUS_CPU_SLOTS CBUS_MAX_CPU_SLOTS
#define kn430_cpuid() mfpr_whami();
#else /* NCPUS <= 1 */
#define CBUS_CPU_SLOTS 1
#define kn430_cpuid() 0
#endif /* NCPUS <= 1 */

/*
 * Get id of other CPU, given id of this CPU.
 */
#define KN430_OTHER_CPUID(cpuid) (1 - cpuid)

/*
 * The global Cbus_nodes_alive is set up during system configuration to 
 * indicate which CBus nodes are populated with working hardware.
 * The format of the bitmask is:
 *
 * Bit  Description
 * ---  -----------
 *   0  I/O Module
 *   1  CPU 1 (secondary)	NOTE: These match slot configurations, so the
 *   2  CPU 0 (primary)		      numbering is backwards!
 *   3  Memory 0
 *   4  Memory 1
 *   5  Memory 2
 *   6  Memory 3
 */
extern u_int Cbus_nodes_alive;

#define CBUS_MAX_NODES (CBUS_IO_SLOTS + CBUS_MAX_CPU_SLOTS + CBUS_MEM_SLOTS)
#define CBUS_NODE_IO_SHIFT 0
#define CBUS_NODE_IO (1 << CBUS_NODE_IO_SHIFT)

#define CBUS_NODE_CPU_SHIFT CBUS_IO_SLOTS
#define CBUS_NODE_CPU(id) (1 << (CBUS_NODE_CPU_SHIFT + (1 - (id))))

#define CBUS_NODE_MEM_SHIFT (CBUS_IO_SLOTS + CBUS_MAX_CPU_SLOTS)
#define CBUS_NODE_MEM(slot) (1 << (slot + CBUS_NODE_MEM_SHIFT))

#define CBUS_IO_ALIVE (Cbus_nodes_alive & (CBUS_NODE_IO))
#define CBUS_CPU_ALIVE(id) (Cbus_nodes_alive & (CBUS_NODE_CPU(id)))
#define CBUS_MEM_ALIVE(slot) (Cbus_nodes_alive & (CBUS_NODE_MEM(slot)))


/*
 * Masks for CBus bit fields.
 *
 * CBUS_PA is 30 bits, corresponding to PA<33:4>.
 * CBUS_LK is 29 bits, corresponding to PA<33:5> (MSB is always 0).
 * CBUS_EA is 16 bits, corresponding to PA<33:18>.
 */
#define CBUS_PA	0x3FFFFFFFUL /* CBus PA<33:4> */
#define CBUS_LK	0x1FFFFFFFUL /* CBus Lock Address, PA<33:5> (MSB always 0) */
#define CBUS_EA	    0xFFFFUL /* CBus Exchange Address, PA<33:18> */
#define CBUS_TT	       0x7UL /* CBus Transaction 0=Read,2=Exch,4=Write,7=NUT */
#define CBUS_CID       0x7UL /* CBus Commander ID 1=cpu1,2=cpu2,4=I/O */

/*****************************************************************************
 * DEC400 CPU MODULE CSRS						     *
 *****************************************************************************
 *
 * These are CSRs for hardware other than the CPU chip on the CPU module.
 * The CPU module has Backup Cache control logic, Cbus control logic, and
 * interrupt control logic on it.  There is a duplicate tag store to speed
 * up maintaining cache coherency.
 */

struct cpu_csr {
	u_long bcc;	u_long fill_00[3]; /* Backup Cache Control */

	u_long bcce;	u_long fill_01[3]; /* Backup Cache Correctable Error */
	u_long bccea;	u_long fill_02[3]; /* B-Cache Corr Err Address Latch */

	u_long bcue;	u_long fill_03[3]; /* B-Cache Uncorrectable Error */
	u_long bcuea;	u_long fill_04[3]; /* B-Cache Uncorr Err Addr Latch */


	u_long dter;	u_long fill_05[3]; /* Duplicate Tag Error */


	u_long cbctl;	u_long fill_06[3]; /* CBus Control */

	u_long cbe;	u_long fill_07[3]; /* CBus Error */
	u_long cbeal;	u_long fill_08[3]; /* CBus Error Addr Latch low */
	u_long cbeah;	u_long fill_09[3]; /* CBus Error Addr Latch high */


	u_long pmbx;	u_long fill_10[3]; /* Processor Mailbox */
	u_long ipir;	u_long fill_11[3]; /* Inter-Processor Int Request */
	u_long sic;	u_long fill_12[3]; /* System Interrupt Clear */
	u_long adlk;	u_long fill_13[3]; /* Address Lock (LDxL/STxC) */
	u_long madrl;	u_long fill_14[3]; /* CBus Miss Address */
	u_long rev;	u_long fill_15[3]; /* CMIC Revision */
};
typedef struct cpu_csr * cpu_csrp_t;

/*
 * Masks for B-Cache and Duplicate Tag Fields.
 *
 * The DEC 4000 B-Cache is a direct-mapped physical write back cache, with a
 * fixed 32 byte (hexaword) block size.  It supports the CBus snooping
 * protocol to allow for a dual processor implementation.
 *
 * Each cache block entry is made up of three storage element arrays.  The
 * control store, which is parity protected, contains the binary flags which
 * indicate whether a particular entry is valid, dirty, and/or shared.  The
 * tag store, which is also parity protected, contains the high order address
 * bits of the data currently stored in that particular chache block entry.
 * The data store, which is EDC protected, contains the cached data.
 * 
 * Control Flags:
 *   VALID	The entry is valid (tag and data are meaningful).
 *   DIRTY	The data store contains an updated copy of its main memory
 *		location.
 *   SHARED	The data store contains a copy of a main memory location which
 *		is also cached by another CBus node.  Writes to this location
 * 		must be "write through" to maintain coherency.
 *   PARITY	Contains even parity over the contents of the control store.
 *
 * Tag Store:
 *   Contains the high order address bits <30:20> (<30:22> for 4Mb cache) of
 *   the memory location that currently resides in the cache entry.  Protected
 *   by a parity bit which maintains even parity.  Parity is checked by the
 *   processor during every B-Cache probe cycle, and by the B-Cache controller
 *   during every CBus initiated probe cycle.
 *
 * Data Store:
 *   Contains the actual data of the memory location that is cached.  A cache
 *   entry is made up of 8 longwords, each longword protected by 7 bits of
 *   EDC.  Physically the cache is only 4 longwords wide, so a cache block
 *   consists of 2 consecutive addresses aligned on a 32 byte block boundary.
 *   Error checking is performed whenever the processor hits the cache on a
 *   read and error checking and possibly correcting is performed whenever a
 *   CBus read probe hits dirty, the victimization of a dirty location, or a
 *    masked processor write to a shared location.
 *
 * The B-Cache subsystem is manipulated by two different controllers.  One
 * resides in the processor.  It probes the tag field and, if hit, will
 * read or write data into the B-Cache.  The behavior of the processor
 * relative to the B-Cache is controlled/monitored by the processor registers
 * biu_stat, biu_addr, fill_addr, biu_ctl, and bc_tag.  The other is the
 * B-Cache controller.  It processes all miss traffic and CBus probe activity.
 * The behavior of the B-Cache controller is determined by the settings of the
 * B-Cache CSRs defined below.
 */
#define BC_MO	   0x1FFFFUL /* B-Cache Map Offset (PA<21:5>)) */
#define BC_TV	     0xFFFUL /* B-Cache Tag Value (PA<33:22>)*/
#define BC_EDC_WIDTH	   7 /* B-Cache EDC Syndrome Width */
#define BC_EDC	      0x7FUL /* B-Cache EDC Syndrome */
#define BC_CTL	       0x7UL /* B-Cache Ctrl (Shared,Dirty,Valid) */
#define BC_SZ	       0x3UL /* B-Cache Size: 1x=4Mb, 01=1Mb, 00=undefined */

#define DT_O	      0xFFUL /* Dup Tag Store Offset */
#define DT_DV	   0xFFFFFUL /* Dup Tag Store Value */

#define KN430_REV      0x3UL /* CPU C3 Revision */
#define KN430_MAC     0x3FUL /* Miss Address Counter */

/* 
 * CPU CSR0 - backup cache control register (BCC).
 *
 * Controls the mode of the Backup Cache, for instance whether new entries
 * may be allocated, whether the shared bit should be forced set on fills,
 * whether error checking and error reporting are enabled.  Also has some
 * bits for testing portions of the B-cache hardware.
 *
 * Each half of the CSR is associated with one of two "slices".  Bit masks
 * are defined for bits in the low slice (nameL), the high slice (nameH) 
 * and both slices (name).
 */

/* ********************* Configuration Information ************************* */

/*
 * Cache Size.  Cleared on powerup, set by the console before the cache is
 * enabled.
 *
 *	11 - 4MB Cache
 *	10 - 4MB Cache
 *	01 - 1MB Cache
 *	00 - Reserved
 */
#define BCC_CSZL_SHIFT	30
#define BCC_CSZL (BC_SZ << BCC_CSZL_SHIFT) /* Cache Size Low */
#define BCC_CSZH_SHIFT	62
#define BCC_CSZH (BC_SZ << BCC_CSZH_SHIFT) /* Cache Size High */
#define BCC_CSZ	 (BCC_CSZL|BCC_CSZH)	   /* Cache Size */

/* ****************************** Control ********************************** */

/*
 * Enable Allocate -- When set, enables the filling of the B-Cache when
 * cacheable sycles occur.  When clear, B-Cache allocation is disabled,
 * but victimization of dirty cache lines still occurs.  This means that the
 * cache line associated with the read/write request is written to memory if
 * dirty and then marked invalid; or if not dirty then no change in the cache
 * line state occurs.  In either case the processor is informed not to cache
 * the data in either internal cache.
 *
 * The B-Cache still provides data to the CBus when read cycles probe dirty,
 * CBus write accepting continues.  Invalidation of the primary cache due to
 * CBus writes still occurs.  Because coherence is maintained, flushing the
 * primary and backup caches is not necessary before re-enabling them.  This
 * bit can be cleared directly by writing or by any B-Cache error reported
 * by CSR 3 bits 1 or 3.  This bit will not be settable if CSR 3 bits 1 or 3
 * are set.
 */
#define BCC_EAL	    (1UL << 0)			/* Enable Allocate Low */
#define BCC_EAH	    (1UL << 32)			/* Enable Allocate High */
#define BCC_EA	    (BCC_EAL|BCC_EAH)		/* Enable Allocate */

/*
 * Force Fill Shared -- When set, forces the setting of the SHARED bit in the
 * Tag Control Store when a new entry is allocated in the B-Cache.
 */
#define BCC_FFSL    (1UL << 1)			/* Force Fill Shared Low */
#define BCC_FFSH    (1UL << 33)			/* Force Fill Shared High */
#define BCC_FFS	    (BCC_FFSL|BCC_FFSH)		/* Force Fill Shared */

/*
 * Enable Tag and Duplicate Tag Parity Check -- When set, enables parity
 * checking of B-Cache tag address, tag control, and duplicate tag memory
 * contents whenever the B-CCache controller accesses the B-Cache and/or
 * the duplicate tag store of the primary cache.  When clear, checking of
 * tag address, tag control, and duplicate tag address parity is ignored.
 */
#define BCC_ETPL  (1UL << 2)	      /* Enb Tag & Dup Tag Parity Check Low */
#define BCC_ETPH  (1UL << 34)	      /* Enb Tag & Dup Tag Parity Check High */
#define BCC_ETP	  (BCC_ETPL|BCC_ETPH) /* Enb Tag & Dup Tag Parity Check */

/*
 * Enable Correctable Error Interrupt -- When set, EDC correctable errors
 * detected during any transaction that is logged by CSR1 will result in the
 * assertion of the Hardware Error Interrupt.  When clear, the interrupt is not
 * sent but error information is latched in CSR1.
 */
#define BCC_ECEIL   (1UL << 6) 		  /* Enb Corr Err Interrupt */
#define BCC_ECEIH   (1UL << 38) 	  /* Enb Corr Err Interrupt */
#define BCC_ECEI    (BCC_ECEIL|BCC_ECEIH) /* Enb Corr Err Interrupt */

/*
 * Enable EDC Correction -- When clear, correctable single bit errors
 * are reported as uncorrectable.  When set, enables EDC correction of
 * processor write data, victim data, and dirty read hit data.
 */
#define BCC_EEDCCORL (1UL << 7)			 /* Enb EDC Correction Low */
#define BCC_EEDCCORH (1UL << 39)		 /* Enb EDC Correction High */
#define BCC_EEDCCOR  (BCC_EEDCCORL|BCC_EEDCCORH) /* Enb EDC Correction */

/*
 * Enable EDC Check -- When set, the EDC of the B-Cache Data Store and
 * processor written data will be checked.  EDC will be checked by the
 * B-Cache controller only when a CBus READ or EXCHANGE hits Dirty, a
 * masked write to a shared location occurs, or during a victim write.
 * When clear, checking for single and multiple bit errors is disabled,
 * but EDC generation still occurs.
 */
#define BCC_EEDCCHKL (1UL << 8)			 /* Enb EDC Check Low */
#define BCC_EEDCCHKH (1UL << 40)		 /* Enb EDC Check High */
#define BCC_EEDCCHK  (BCC_EEDCCHKL|BCC_EEDCCHKH) /* Enable EDC Check */

/*
 * Enable B-Cache Conditional I/O Updates -- When set, conditional updates
 * of the B-Cache will occur due to CBus writes when the I/O module is the
 * commander.  A CBus write will cause an update if both the contents of the
 * duplicate tag store and the B-Cache indicate a hit.  When clear,
 * unconditional updates of the B-Cache will occur when a CBus write hits a
 * VALID B-Cache location when the I/O module is the CBus commander.
 */
#define BCC_EBCUL (1UL << 9)		/* Enb B-Cache Cond I/O Updates Low */
#define BCC_EBCUH (1UL << 41) 		/* Enb B-Cache Cond I/O Updates High */
#define BCC_EBCU  (BCC_EBCUL|BCC_EBCUH) /* Enb B-Cache Cond I/O Updates */

/*
 * Disable Block Write Around -- When clear, only masked processor writes
 * result in allocation.  Full block unmasked writes will write around the
 * B-Cache and not result in allocation.  When set, processor writes result
 * in allocation regardless of the block mask.
 */
#define BCC_DBWAL   (1UL << 10)		  /* Disable Block Write Around Low */
#define BCC_DBWAH   (1UL << 42)		  /* Disable Block Write Around High */
#define BCC_DBWA    (BCC_DBWAL|BCC_DBWAH) /* Disable Block Write Around */

/* *************************** Diagnostics ********************************* */

/*
 * Enable B-Cache Init -- used to initialize the B-Cache with a known pattern
 * during powerup.
 */
#define BCC_EBCIL   (1UL << 11) 		  /* Enb B-Cache Init Low */
#define BCC_EBCIH   (1UL << 43) 		  /* Enb B-Cache Init High */
#define BCC_EBCI    (BCC_EBCIL|BCC_EBCIH) 	  /* Enb B-Cache Init */

/*
 * Fill Wrong Tag Parity -- When set, wrong parity is forced onto the B-Cache
 * tag address store during the next B-Cache allocation/update (CBus read from
 * this processor or write accept).  This bit is self clearing.
 */
#define BCC_WTPL    (1UL << 3)		/* Fill Wrong Tag Parity Low */
#define BCC_WTPH    (1UL << 35)		/* Fill Wrong Tag Parity High */
#define BCC_WTP	    (BCC_WTPL|BCC_WTPH) /* Fill Wrong Tag Parity */

/*
 * Fill Wrong Control Parity -- When set, wrong parity is forced into the
 * B-Cache tag control store during the next B-Cache allocation/updates
 * (CBus read from this processor or write accept).  This bit is self-clearing.
 */
#define BCC_WCPL    (1UL << 4)		/* Fill Wrong Control Parity Low */
#define BCC_WCPH    (1UL << 36)		/* Fill Wrong Control Parity High */
#define BCC_WCP	    (BCC_WCPL|BCC_WCPH) /* Fill Wrong Control Parity */

/*
 * Fill Wrong Duplicate Tag Store Parity -- When set, the wrong parity value
 * will be written in the primary D-Cache duplicate tag store parity location
 * when a primary D-Cache location is updated during a processor primary
 * D-Cache allocation.  This bit is NOT self clearing.  When this bit is
 * set, the ETP bits should be cleared; otherwise a hardware error interrupt
 * will be signaled every time the duplicate tag store is written.
 */
#define BCC_WDPL    (1UL << 5)		/* Fill Wrong Dup Tag Store Par Low */
#define BCC_WDPH    (1UL << 37)		/* Fill Wrong Dup Tag Store Par High */
#define BCC_WDP	    (BCC_WDPL|BCC_WDPH) /* Fill Wrong Dup Tag Store Par */

/*
 * Force EDC/Control -- NOT self-clearing.  Used to initialize the B-Cache.
 */
#define BCC_FEDCCTLL (1UL << 12)		 /* Force EDC/Control Low */
#define BCC_FEDCCTLH (1UL << 44)		 /* Force EDC/Control High */
#define BCC_FEDCCTL  (BCC_FEDCCTLL|BCC_FEDCCTLH) /* Force EDC/Control */

/*
 * Initialize EDC Control
 *
 * When the FEDCCTL bits are set, the values specified in the IEDC bits
 * are written forced on the EDC field of odd/even longwords.
 */
#define BCC_IEDCL_SHIFT	16
#define BCC_IEDC0 (BC_EDC << BCC_IEDCL_SHIFT)
#define BCC_IEDC2 (BC_EDC << (BCC_IEDCL_SHIFT + BC_EDC_WIDTH))
#define BCC_IEDCL (BCC_IEDC0|BCC_IEDC2)		/* EDC Control Low (Even) */

#define BCC_IEDCH_SHIFT	48
#define BCC_IEDC1 (BC_EDC << BCC_IEDCH_SHIFT)
#define BCC_IEDC3 (BC_EDC << (BCC_IEDCH_SHIFT + BC_EDC_WIDTH))
#define BCC_IEDCH (BCC_IEDC1|BCC_IEDC3)		/* EDC Control High (Odd) */
#define BCC_IEDC  (BCC_IEDCL|BCC_IEDCH)	 	/* EDC Control */

/*
 * B-Cache Control Bits (Shared, Dirty, Valid) -- When FEDCCTL is set during
 * B-Cache initialization, the values specified in here will be filled into
 * the control field of the B-Cache location referenced.  When FEDCCTL is
 * cleared, these bits have no effect.
 */
#define BCC_CTLL_SHIFT	13
#define BCC_CTLL (BC_CTL << BCC_CTLL_SHIFT) /* Shared/Dirty/Valid Low */
#define BCC_CTLH_SHIFT	45
#define BCC_CTLH (BC_CTL << BCC_CTLH_SHIFT) /* Shared/Dirty/Valid High */
#define BCC_CTL	 (BCC_ISDVL|BCC_ISDVH)     /* Shared/Dirty/Valid */

/* 
 * CPU CSR1 - B-Cache Correctable Error register (BCCE).
 *
 * The low bits (CE, MCE) indicate whether a correctable error occurred, and
 * whether a subsequent error was missed.  Additional bits latch error
 * information.  When the error indicators are off, the latches run free.  The
 * CE and MCE bits are write-one-to-clear.  The ISBC bits are like the latches;
 * they are meaningless when the error bits are not set.  Each half of
 * the CSR is associated with one of two "slices".  Bit masks are defined for
 * bits in the low slice (nameL), the high slice (nameH) and both slices
 * (name).
 */

#define BCCE_CEL   (1UL << 3)		 /* Correctable Error Low */
#define BCCE_CEH   (1UL << 35)		 /* Correctable Error High */
#define BCCE_CE	   (BCCE_CEL|BCCE_CEH)	 /* Correctable Error */

#define BCCE_MCEL  (1UL << 2)		 /* Missed Correctable Error Low */
#define BCCE_MCEH  (1UL << 34)		 /* Missed Correctable Error High */
#define BCCE_MCE   (BCCE_MCEL|BCCE_MCEH) /* Missed Correctable Error */

#define BCCE_ISBCL (1UL << 17)		   /* Caused by: 1=B-Cache 0=CPU Low */
#define BCCE_ISBCH (1UL << 49)		   /* Caused by: 1=B-Cache 0=CPU Hi  */
#define BCCE_ISBC  (BCCE_ISBCL|BCCE_ISBCH) /* B-Cache or CPU caused Error */

/*
 * The control bits are only present in the low longword of the CSR.
 */
#define BCCE_CBP  (1UL << 8)		       /* Control Bit: Parity */
#define BCCE_CBS  (1UL << 9)		       /* Control Bit: Shared */
#define BCCE_CBD  (1UL << 10)		       /* Control Bit: Dirty */
#define BCCE_CBV  (1UL << 11)		       /* Control Bit: Valid */
#define BCCE_CTL  (BCCE_CBS|BCCE_CBD|BCCE_CBV) /* Control Bits */
#define BCCE_CTLP (BCCE_CBP|BCCE_CTL) 	       /* Control Bits + Parity */

/*
 * EDC Syndrome Bits for each longword.
 */
#define BCCE_EDCSYN0_SHIFT 18		
#define BCCE_EDCSYN0	(BC_EDC << BCCE_EDCSYN0_SHIFT) /* EDC Syndrome 0 */
#define BCCE_EDCSYN1_SHIFT 50
#define BCCE_EDCSYN1	(BC_EDC << BCCE_EDCSYN1_SHIFT) /* EDC Syndrome 1 */
#define BCCE_EDCSYN2_SHIFT 25
#define BCCE_EDCSYN2	(BC_EDC << BCCE_EDCSYN2_SHIFT) /* EDC Syndrome 2 */
#define BCCE_EDCSYN3_SHIFT 57
#define BCCE_EDCSYN3	(BC_EDC << BCCE_EDCSYN3_SHIFT) /* EDC Syndrome 3 */

#define BCCE_W1TC  (BCCE_MCE|BCCE_CE)  /* Write-one-to-clear bits */

/* 
 * CPU CSR2 - B-Cache Correctable Error Address register (BCCEA)
 *
 * When the B-Cache Correctable Error Register's Correctable Error bits go
 * high (BCCE bits 3 & 35), this register's contents get latched.  Otherwise,
 * its contents change.
 *
 * Each half of the CSR is associated with one of two "slices".  Bit masks
 * are defined for bits in the low slice (nameL), the high slice (nameH) 
 * and both slices (name).
 */

/*
 * B-Cache Map Offset.  If an even/odd longword has a correctable EDC error,
 * the low/high field provides an address pointer into the B-Cache.
 */
#define BCCEA_MOL_SHIFT	0
#define BCCEA_MOL BC_MO			         /* B-Cache Map Offset Low */
#define BCCEA_MOH_SHIFT	32
#define BCCEA_MOH (BC_MO << BCCEA_MOH_SHIFT)     /* B-Cache Map Offset High */
#define BCCEA_MO  (BCCEA_MOL|BCCEA_MOH)		 /* B-Cache Map Offsets */

/*
 * Tag Value.  If an even/odd longword has a parity error, the low/high tag
 * value provides an address pointer into the B-Cache.
 */
#define BCCEA_TVL_SHIFT	19
#define BCCEA_TVL (BC_TV << BCCEA_TVL_SHIFT)  /* B-Cache Tag Value Low */
#define BCCEA_TVH_SHIFT	51
#define BCCEA_TVH (BC_TV << BCCEA_TVH_SHIFT)  /* B-Cache Tag Value High */
#define BCCEA_TV  (BCCEA_TVL|BCCEA_TVH)		/* B-Cache Tag Values */

/*
 * Tag Store Parity.
 */
#define BCCEA_TPL (1UL << 18)	/* B-Cache Tag Parity Low (even parity) */
#define BCCEA_TPH (1UL << 50)	/* B-Cache Tag Parity High (even parity) */
#define BCCEA_TP (BCCEA_TPL|BCCEA_TPH) /* B-Cache Tag Parities (even parity) */


/* 
 * CPU CSR3 - B-Cache Uncorrectable Error register (BCUE).
 *
 * This register reports B-Cache uncorrectable errors, such as parity and
 * uncorrectable errors.  It also latches control and syndrome bits for
 * the error.
 *
 * Each half of the CSR is associated with one of two "slices".  Bit masks
 * are defined for bits in the low slice (nameL), the high slice (nameH) 
 * and both slices (name).
 */
#define BCUE_PEL    (1UL << 1)		  /* Parity Error Low */
#define BCUE_PEH    (1UL << 33)		  /* Parity Error High */
#define BCUE_PE	    (BCUE_PEL|BCUE_PEH)   /* Parity Error */

#define BCUE_MPEL   (1UL << 0)		  /* Missed Parity Error Low */
#define BCUE_MPEH   (1UL << 32L)	  /* Missed Parity Error High */
#define BCUE_MPE    (BCUE_MPEL|BCUE_MPEH) /* Missed Parity Error */

#define BCUE_UEL    (1UL << 3)		  /* B-Cache Uncorr EDC Error Low */
#define BCUE_UEH    (1UL << 35L)	  /* B-Cache Uncorr EDC Error High */
#define BCUE_UE	    (BCUE_UEL|BCUE_UEH)   /* B-Cache Uncorrectable EDC Error */

#define BCUE_MUEL   (1UL << 2)		  /* Missed BCache Uncorr EDC Err Low*/
#define BCUE_MUEH   (1UL << 34L)	  /* Missed BCache Uncorr EDC Error H*/
#define BCUE_MUE    (BCUE_MUEL|BCUE_MUEH) /* Missed BCache Uncorr EDC Error */

/*
 * Control bits for last B-Cache location accessed.  Updated when PE and UE are
 * clear.
 */
#define BCUE_CBP   (1UL << 8)		  /* Control Bit: Parity */
#define BCUE_CBS   (1UL << 9)		  /* Control Bit: Shared */
#define BCUE_CBD   (1UL << 10)		  /* Control Bit: Dirty */
#define BCUE_CBV   (1UL << 11)		  /* Control Bit: Valid */
#define BCUE_CTL   (BCUE_CBS|BCUE_CBD|BCUE_CBV) /* Control Bits */
#define BCUE_CTLP  (BCUE_CBP|BCUE_CTL)	  /* Control Bits + Parity */

/*
 * B-Cache EDC Error -- When set, the B-Cache was the cause of the data
 * error.  This register is updated when the UE and PE bits are clear.
 */
#define BCUE_ISBCL (1UL << 17)		   /* B-Cache (1) or CPU (0) Low */
#define BCUE_ISBCH (1UL << 49L)  	   /* B-Cache (1) or CPU (0) High */
#define BCUE_ISBC  (BCUE_ISBCL|BCUE_ISBCH) /* B-Cache (1) or CPU (0) */

/*
 * Syndrome bits for corresponding B-Cache longwords.
 */
#define BCUE_EDCSYN0_SHIFT 18
#define BCUE_EDCSYN0	(BC_EDC << BCUE_EDCSYN0_SHIFT) /* Syndrome 0 */
#define BCUE_EDCSYN1_SHIFT 50
#define BCUE_EDCSYN1	(BC_EDC << BCUE_EDCSYN1_SHIFT) /* Syndrome 1 */
#define BCUE_EDCSYN2_SHIFT 25
#define BCUE_EDCSYN2	(BC_EDC << BCUE_EDCSYN2_SHIFT) /* Syndrome 2 */
#define BCUE_EDCSYN3_SHIFT 57
#define BCUE_EDCSYN3	(BC_EDC << BCUE_EDCSYN3_SHIFT) /* Syndrome 3 */

#define BCUE_W1TC		(BCUE_MPE|BCUE_PE|BCUE_MUE|BCUE_UE)

/*
 * CPU CSR4 - B-cache uncorrectable error address register (BCUEA).
 *
 * When a B-Cache tag store or control store parity error, or an 
 * uncorrectable EDC error has been detected, the B-Cache Uncorrectable
 * Error Address Register contains the index of the B-Cache location that
 * contains the error.
 *
 * Each half of the CSR is associated with one of two "slices".  Bit masks
 * are defined for bits in the low slice (nameL), the high slice (nameH) 
 * and both slices (name).
 */
#define BCUEA_MOL_SHIFT 0
#define BCUEA_MOL BC_MO			          /* B-Cache Map Offset Low */
#define BCUEA_MOH_SHIFT	32
#define BCUEA_MOH (BC_MO << BCUEA_MOH_SHIFT)      /* B-Cache Map Offset High */
#define BCUEA_MO  (BCUEA_MOL|BCUEA_MOH)		  /* B-Cache Map Offset */

#define BCUEA_PTPL  (1UL << 17)		    /* B-Cache Predicted Tag Par Low */
#define BCUEA_PTPH  (1UL << 49)		    /* B-Cache Predicted Tag Par High*/
#define BCUEA_PTP   (BCUEA_PTPL|BCUEA_PTPH) /* B-Cache Predicted Tag Parity */

#define BCUEA_TPL   (1UL << 18)		    /* B-Cache Tag Parity Low */
#define BCUEA_TPH   (1UL << 50)		    /* B-Cache Tag Parity High */
#define BCUEA_TP    (BCUEA_TPL|BCUEA_TPH)   /* B-Cache Tag Parity */

#define BCUEA_TVL_SHIFT	19
#define BCUEA_TVL   (BC_TV << BCUEA_TVL_SHIFT) /* B-Cache Tag Value Low */
#define BCUEA_TVH_SHIFT	51
#define BCUEA_TVH   (BC_TV << BCUEA_TVH_SHIFT) /* B-Cache Tag Value High */
#define BCUEA_TV    (BCUEA_TVL|BCUEA_TVH)	 /* B-Cache Tag Value */

/*
 * CPU CSR5 - Duplicate Tag Error Register (DTER).
 *
 * Every DEC 4000 CPU module provides a mechanism to perform "conditional
 * invalidation" of the cache memory subsystems.  This is accomplished through
 * the use of a copy of the Primary D-Cache tag store.  As updates to
 * cacheable locations occur, invalidation of the internal data cache is 100%
 * accurate.  The duplicate tag store also provides a means to "selectively"
 * accept updates to the B-Cache.  This allows optimal system performance for
 * accesses to truly shared locations; however protects the integrity of the
 * write-back system by invalidating "shared cold" locations caused by
 * phenomena such as process migration.
 *
 * The Duplicate Tag Error Register is updated after each access to the
 * duplicate tag store RAM in both slices.  After a parity error is detected,
 * the contents of the register are not updated while the error flag is set,
 * the lost error flag does not inhibit error logging.  The missed error bit
 * in this register, when set, suppresses further error interrupts from
 * duplicate tag parity errors.  When these parity errors are detected,
 * invalidation of the primary and/or secondary caches result.
 *
 * Filling the duplicate tag RAM with bad parity causes the coherence policy
 * to degenerate from update to invalidate.
 *
 * Each half of the CSR is associated with one of two "slices".  Bit masks
 * are defined for bits in the low slice (nameL), the high slice (nameH) 
 * and both slices (name).
 */
#define DTER_PEL    (1UL << 1)		  /* Dup Tag Parity Error Low */
#define DTER_PEH    (1UL << 33)		  /* Dup Tag Parity Error High */
#define DTER_PE	    (DTER_PEL|DTER_PEH)   /* Dup Tag Parity Error */

#define DTER_MPEL   (1UL << 0)		  /* Missed Dup Tag Parity Err Low */
#define DTER_MPEH   (1UL << 32)		  /* Missed Dup Tag Parity Err High */
#define DTER_MPE    (DTER_MPEL|DTER_MPEH) /* Missed Dup Tag Parity Error */

/*
 * Duplicate Tag Store Offset -- If a parity occurs in the duplicate tag store,
 * then the offset will be latched until the PE bits have been cleared.
 * This field is only updated when memory space references occur while the
 * register is not frozen.
 */
#define DTER_DTSOL_SHIFT 2
#define DTER_DTSOL (DT_O << DTER_DTSOL_SHIFT)	/* Dup Tag Store Offset Low */
#define DTER_DTSOH_SHIFT 34
#define DTER_DTSOH (DT_O << DTER_DTSOH_SHIFT)	/* Dup Tag Store Offset High */
#define DTER_DTSO  (DTER_DTSOL|DTER_DTSOH)	/* Dup Tag Store Offsets */

/*
 * Duplicate Tag -- If a parity error occurs in the P-Cache duplicate tag
 * store, then the contents of this register will be frozen until the PE bits
 * have been cleared.  This field is only updated when memory space references
 * occur while the register is not frozen.
 */
#define DTER_DTL_SHIFT 10
#define DTER_DTL (DT_V << DTER_DTL_SHIFT)	/* Duplicate Tag Low */
#define DTER_DTH_SHIFT 42
#define DTER_DTH (DT_V << DTER_DTH_SHIFT)	/* Duplicate Tag High */
#define DTER_DT	 (DTER_DTL|DTER_DTH)		/* Duplicate Tag */

/*
 * Duplicate Tag Parity -- Contains the most recent Primary D-Cache duplicate
 * tag store parity.  There are two cases to consider, probes from the CBus and
 * read misses from the processor.  If a parity error occurs due to a probe 
 * then this bit shows that bad parity as calculated from the DT bits in this
 * register and is a fatal error.  If a parity error occurs due to a processor
 * read miss which allocates into the duplicate tag, then this bit shows the
 * parity of the allocated address and does not reflect the calculated parity
 * of DT.  The contents of this field are frozen until the PE bits have been
 * cleared.  This field is only updated when memory space references occur
 * while the register is not frozen.
 */
#define DTER_PL	(1UL << 30)	    /* Dup Tag Parity Low */
#define DTER_PH	(1UL << 62)	    /* Dup Tag Parity High */
#define DTER_P	(DTER_PL|DTER_PH)   /* Dup Tag Parity */

#define DTER_W1TC   (DTER_MPE|DTER_PE)

/*
 * CPU CSR6 - CBus control register (CBCTL).
 *
 * The CBus interface is the DEC 4000 CPU module's "window to the world."
 * All standard data processed by a DEC 4000 processor is obtained over the
 * CBus from either a memory module, the I/O module, or another CPU module.
 * Refer to the "Cobra System Bus Specification."  All CBus registers are
 * visible to all CBus commanders.
 *
 * Each half of the CSR is associated with one of two "slices".  Bit masks
 * are defined for bits in the low slice (nameL), the high slice (nameH) 
 * and both slices (name).
 */

/* ********************************* Control ******************************* */

/*
 * Enable Parity Check -- When set, longword parity checking on the CBus for
 * both the C/A and data portion of the cycle (if responder) is enabled.
 */
#define CBCTL_EPCL (1UL << 3)  /* Enb Par Chk: C/A LW 1,0 & Data LW 6,4,2,0 */
#define CBCTL_EPCH (1UL << 35) /* Enb Par Chk: C/A LW 3,2 & Data LW 7,5,3,1 */
#define CBCTL_EPC  (CBCTL_EPCL|CBCTL_EPCH) /* Enable Parity Check */

/*
 * Force Shared -- When set, asserts CSHARED_L on all CBus transactions.
 */
#define CBCTL_FSDL (1UL << 4)			/* Force Shared Low */
#define CBCTL_FSDH (1UL << 36)			/* Force Shared High */
#define CBCTL_FSD  (CBCTL_FSDL|CBCTL_FSDH) 	/* Force Shared */

/*
 * ARB Control Mask -- When the bit corresponding to a particular subsystem
 * is set, that subsystem will be granted the bus when requested.  A processor
 * can not clear its own ARB Control Mask bit.  This field is non-functional in
 * CPU 2.
 */
#define CBCTL_ACM_CPU1L	(1UL << 8)	/* Arb Control CPU 1 Low */
#define CBCTL_ACM_CPU1H	(1UL << 40L)	/* Arb Control CPU 1 High */
#define CBCTL_ACM_CPU1	(CBCTL_ACM_CPU1L|CBCTL_ACM_CPU1H) /* Arb Ctl CPU 1 */

#define CBCTL_ACM_CPU2L	(1UL << 9)	/* Arb Control CPU 2 Low */
#define CBCTL_ACM_CPU2H	(1UL << 41)	/* Arb Control CPU 2 High */
#define CBCTL_ACM_CPU2	(CBCTL_ACM_CPU2L|CBCTL_ACM_CPU2H) /* Arb Ctl CPU 2 */

#define CBCTL_ACM_IOL	(1UL << 10)	/* Arb Control I/O Low */
#define CBCTL_ACM_IOH	(1UL << 42)	/* Arb Control I/O High */
#define CBCTL_ACM_IO	(CBCTL_ACM_IOL|CBCTL_ACM_IOH) /* Arb Ctl CPU 2 */

/*
 * Enable CBus Error Interrupt -- When clear, disables the CBus C_ERR_L
 * interrupt signal from being driven due to errors encountered by this node.
 * This bit does not disable reception of this interrupt signal.
 */
#define CBCTL_ECERRL  (1UL << 11)	/* Enable CBus Error Interrupt Low */
#define CBCTL_ECERRH  (1UL << 43)	/* Enable CBus Error Interrupt High */
#define CBCTL_ECERR   (CBCTL_ECERRL|CBCTL_ECERRH) /* Enable CBus Error Int */

/*
 * Reserved Diagnostic -- Reserved for diagnostic testing.  When set, the
 * behavior of the CBus arbiter is modified to regulate the flow of back-to-
 * back transactions.
 */
#define CBCTL_RSDL (1UL << 12)		   /* Reserved Diagnostic Low */
#define CBCTL_RSDH (1UL << 44)		   /* Reserved Diagnostic High */
#define CBCTL_RSD  (CBCTL_RSDL|CBCTL_RSDH) /* Reserved Diagnostic */

/* ******************************* Diagnostics ***************************** */

/*
 * Data Wrong Parity -- When set, forces wrong parity during both data
 * portions of the next CBus cycle to which this node responds.  Once this
 * cycle has occurred, the bits are cleared.  This should not be set when any
 * of the C/A Wrong Parity bits are set as a responder is not able to log both
 * wrong C/A parity and wrong data parity in the same CBus transaction.
 */
#define CBCTL_DWPL (1UL << 0)	      /* Data Wrong Parity Low (LW 6,4,2,0) */
#define CBCTL_DWPH (1UL << 32)	      /* Data Wrong Parity High (LW 7,5,3,1) */
#define CBCTL_DWP  (CBCTL_DWPL|CBCTL_DWPH) /* Data Wrong Parity */

/*
 * C/A Wrong Parity -- When set forces wrong parity during the C/A portion of
 * the next C/A cycle from this node to the CBus.  Once this cycle has occurred
 * the C/A Wrong Parity bits are automatically cleared.
 */
#define CBCTL_CAWP0 (1UL << 1)	/* C/A Wrong Parity on LW 0 */
#define CBCTL_CAWP1 (1UL << 33) /* C/A Wrong Parity on LW 1 */
#define CBCTL_CAWP2 (1UL << 2)  /* C/A Wrong Parity on LW 2 */
#define CBCTL_CAWP3 (1UL << 34) /* C/A Wrong Parity on LW 3 */

#define CBCTL_CAWPL (CBCTL_CAWP0|CBCTL_CAWP2) /* C/A Wrong Parity Low */
#define CBCTL_CAWPH (CBCTL_CAWP1|CBCTL_CAWP3) /* C/A Wrong Parity High */
#define CBCTL_CAWP  (CBCTL_CAWPL|CBCTL_CAWPH) /* C/A Wrong Parity */


/*
 * CPU CSR7 - CBus error register (CBE).
 *
 * The CBus Error Register is updated every CBus cycle.  Whenever a CBus
 * error is detected, the contents of this register are frozen until the
 * error bits are cleared.  The contents of the register are not updated while
 * the error flags are set, the lost error flags do not inhibit error logging.
 *
 * If a tag control or tag store parity error is detected then the C/A Not
 * Acked and/or the Write Data Not Acked bits are set.
 *
 * Each half of the CSR is associated with one of two "slices".  Bit masks
 * are defined for bits in the low slice (nameL), the high slice (nameH) 
 * and both slices (name).
 */

/*
 * Reserved Diagnostic -- Set as a result of an error event being communicated
 * between the even and odd interface chips.  This bit is for chip debug and
 * should not be used by system software.  The address is not held in CSR 8
 * and 9 when this bit is set.  Write 1 to clear.
 */
#define CBE_RDL	(1UL << 1)	  /* Reserved Diagnostic Low */
#define CBE_RDH	(1UL << 33)	  /* Reserved Diagnostic High */
#define CBE_RD	(CBE_RDL|CBE_RDH) /* Reserved Diagnostic */

/*
 * C/A Parity Error -- Set when a parity error was detected on a C/A cycle,
 * regardless of the address or which node was the commander.  A CPU node
 * checks its own parity as a commander.  Write 1 to clear.
 */
#define CBE_CAPEL (1UL << 2)		/* C/A Parity Error LW 2,0 */
#define CBE_CAPEH (1UL << 34)		/* C/A Parity Error LW 3,1 */
#define CBE_CAPE  (CBE_CAPEL|CBE_CAPEH) /* C/A Parity Error */

/*
 * Missed C/A Parity Error -- Set when a parity error was detected or the
 * C/A cycle was not acknowledged from this commander and the error command
 * and address could not be saved in the CBEAL register.  Write 1 to clear.
 */
#define CBE_MCAPEL (1UL << 3)		   /* Missed C/A Parity Err LW 2,0 */
#define CBE_MCAPEH (1UL << 35)		   /* Missed C/A Parity Err LW 3,1 */
#define CBE_MCAPE  (CBE_MCAPEL|CBE_MCAPEH) /* Missed C/A Parity Error */

/*
 * Parity Error on Write Data (Responder) -- Set when a parity error was
 * detectd as a responder or bystander in the case of accepting write data to
 * update the B-Cache.  Write 1 to clear.
 */
#define CBE_WDPEL (1UL << 4)		/* Parity Error on Write Data LW 2,0 */
#define CBE_WDPEH (1UL << 36)		/* Parity Error on Write Data LW 3,1 */
#define CBE_WDPE  (CBE_WDPEL|CBE_WDPEH) /* Parity Error on Write Data */

/*
 * Missed Parity Error on Write Data (Responder).
 */
#define CBE_MWDPEL (1UL << 5)		   /* Missed Par Err on Wr D LW 2,0 */
#define CBE_MWDPEH (1UL << 37)		   /* Missed Par Err on Wr D LW 2,0 */
#define CBE_MWDPE  (CBE_MWDPEL|CBE_MWDPEH) /* Missed Par Err on Write Data */

/*
 * Parity Error on Read Data (Requestor) -- Set when a parity error was
 * detected on returned read data as a commander.
 */
#define CBE_RDPEL (1UL << 6)		/* Par Err on Read Data, Even LW */
#define CBE_RDPEH (1UL << 38)		/* Par Err on Read Data, Odd LW */
#define CBE_RDPE  (CBE_RDPEL|CBE_RDPEH) /* Parity Error on Read Data */

/*
 * Missed Parity Error on Read Data (Requestor).
 */
#define CBE_MRDPEL (1UL << 7)		  /* Missed Par Err on Rd D, Even LW */
#define CBE_MRDPEH (1UL << 39)		  /* Missed Par Err on Rd D, Odd LW */
#define CBE_MRDPE (CBE_MRDPEL|CBE_MRDPEH) /* Missed Par Error on Read Data */

/*
 * C/A Parity Error Longword Indicators -- When a parity error is detected on
 * a particular C/A longword, the corresponding C/A Parity Error bit is set.
 * These bits have meaning when the CAPE bits are set.
 */
#define CBE_CAPE0	(1UL << 8)		/* C/A Par Err LW 0 */
#define CBE_CAPE1	(1UL << 40)		/* C/A Par Err LW 1 */
#define CBE_CAPE2	(1UL << 9)		/* C/A Par Err LW 2 */
#define CBE_CAPE3	(1UL << 41)		/* C/A Par Err LW 3 */
#define CBE_CAPEEVEN	(CBE_CAPE2 | CBE_CAPE0) /* C/A Par Err Even LW */
#define CBE_CAPEODD	(CBE_CAPE3 | CBE_CAPE1) /* C/A Par Err Odd LW */
#define CBE_CAPESUM	(CBE_CAPEODD | CBE_CAPEEVEN) /* C/A Par Err Bits */

/*
 * Data Parity Error Longword Indicators -- When a data parity error is
 * detected by this module, these bits indicate that a parity error was
 * observed in the corresponding longword. (DPE4 means LW 0 during the second
 * data cycle portion, etc).  These bits are latched when the RDPE or WDPE bits
 * are set.
 */
#define CBE_DPE0	(1UL << 10) /* Data Par Err LW 0 Cycle 1 */
#define CBE_DPE1	(1UL << 42) /* Data Par Err LW 1 Cycle 1 */
#define CBE_DPE2	(1UL << 11) /* Data Par Err LW 2 Cycle 1 */
#define CBE_DPE3	(1UL << 43) /* Data Par Err LW 3 Cycle 1 */
#define CBE_DPE4	(1UL << 12) /* Data Par Err LW 4 (LW 0 Cycle 2) */
#define CBE_DPE5	(1UL << 44) /* Data Par Err LW 5 (LW 1 Cycle 2) */
#define CBE_DPE6	(1UL << 13) /* Data Par Err LW 6 (LW 2 Cycle 2) */
#define CBE_DPE7	(1UL << 45) /* Data Par Err LW 7 (LW 3 Cycle 2) */
#define CBE_DPEODD	(CBE_DPE7 | CBE_DPE5 | CBE_DPE3 | CBE_DPE1)
#define CBE_DPEEVEN	(CBE_DPE6 | CBE_DPE4 | CBE_DPE2 | CBE_DPE0)
#define CBE_DPESUM	(CBE_DPEODD | CBE_DPEEVEN)

/*
 * C/A Not Acked -- Set if the C/A portion of a CBus cycle generated by this
 * commander was not acknowledged.  The error address is logged in fields
 * associated with the lower 32 bits of the CBEAL and CBEAH.  The upper 32
 * bits of these registers have no meaning when this error has been detected.
 * This error may cause subsequent lost errors.  Write 1 to clear.
 */
#define CBE_CANA (1UL << 14)		/* Cmd/Addr not acknowledged */

/*
 * Write Data Not Acked -- Set if either octaword portion of a CBus write
 * cycle generated by this commander was not acknowledged.  The error address
 * is logged in CBEAL and CBEAH, hence this error may cause subsequent lost
 * errors.  Write 1 to clear. This error could be flagged if a double bit
 * error occurs in the tag store and the exchange address of the dirty victim
 * places its address outside the physical address space of the currently
 * configured system.
 */
#define CBE_WDNA (1UL << 15)		/* Write Data not acknowledged */

/*
 * Undefined bits, correspond to WDNA and CANA in the low longword.
 */
#define CBE_UNDEF ((1UL << 46) | (1UL << 47))

/*
 * Miss Count -- A six bit miss address counter.
 */
#define CBE_MCL_SHIFT 25
#define CBE_MCL (KN430_MAC << CBE_MCL_SHIFT)	/* Miss Address Counter Low */
#define CBE_MCH_SHIFT 57
#define CBE_MCH (KN430_MAC << CBE_MCH_SHIFT)	/* Miss Address Counter High */
#define CBE_MC  (CBE_MCL|CBE_MCH)		/* Miss Address Counters */

/*
 * Miss Address Valid -- This bit will be set when the miss counter overflows
 * and holds sampled miss address in the MADRL register.  The miss address is
 * sampled every 64th B-Cache miss by a free running miss transaction counter.
 * When this bit is set, MADRL has valid miss contents.  If this bit is set,
 * capture of a new sample is disabled but the counter continues to run.  A
 * copy of this bit is readable from bit 0 of the MADRL register.  Write 1
 * to clear.
 */
#define CBE_MAVL (1UL << 31)		/* Miss Address Valid Low */
#define CBE_MAVH (1UL << 63)		/* Miss Address Valid High */
#define CBE_MAV  (CBE_MAVL|CBE_MAVH)	/* Miss Address Valid */

/* All write-one-to-clear bits. */
#define CBE_W1TC		(CBE_RD|CBE_CAPE|CBE_MCAPE| \
				 CBE_WDPE|CBE_MWDPE|CBE_RDPE|CBE_MRDPE| \
				 CBE_CANA|CBE_WDNA|CBE_UNDEF)

/*
 * CPU CSR8 - CBus error address low (CBEAL).
 *
 * Contains the actual data found on the CBus <63:0> during the latest C/A
 * cycle.  Whenever a CBus error is detected and logged in the CBE register,
 * the contents of this register are frozen until all of the error indications,
 * not the missed error indications, in the CBE Register are cleared.
 *
 * For CBus command/address cycles that are not acknowledged, the failing
 * address is latched only in the error logging bits <31:0>.  Bits <63:34> do
 * not contain valid data when this type of error is logged.
 */
#define CBEAL_AL_WIDTH 30
#define CBEAL_AL_SHIFT 2
#define CBEAL_AL (CBUS_PA << CBEAL_AL_SHIFT) /* PA<33:4> from CAD<31:2> */
#define CBEAL_AL_ADDR(x) (((unsigned long)(x) & CBEAL_AH) << 2)

#define CBEAL_AH_WIDTH CBEAL_AL_WIDTH
#define CBEAL_AH_SHIFT 34
#define CBEAL_AH (CBUS_PA << CBEAL_AH_SHIFT) /* PA<33:4> from CAD<64:34> */
#define CBEAL_AH_ADDR(x) (((unsigned long)(x) & CBEAL_AH) >> 30)

#define CBEAL_A	(CBEAL_AL|CBEAL_AH) /* All C/A bits */

/*
 * CPU CSR9 - CBus error address high (CBEAH).
 *
 * This register is updated by this nodes commander transactions or by CBus
 * errors and contains the actual data found on the CBus <127:64> during the
 * latest C/A cycle.  Whenever a CBus error is detected and logged in the CBE
 * register, the contents of this register are frozen until all of the error,
 * not missed error, indications in the CBE register are cleared.
 *
 * For CBus command/address cycles that are not acknowledged, the failing
 * address is latched only in the error logging bits <31:0>.  Bits <63:34>
 * do not contain valid information when this type of error is logged.
 */

/*
 * Exchange Address -- Contains the tag of the victimized cache location.
 */
#define CBEAH_EAL_SHIFT	2
#define CBEAH_EAL (CBUS_EA << CBEAH_EAL_SHIFT)
#define CBEAH_EAH_SHIFT		34
#define CBEAH_EAH		(CBUS_EA << CBEAH_EAH_SHIFT)
#define CBEAH_EA		(CBEAH_EAL|CBEAH_EAH)

/*
 * Transaction Type.  The commander transaction request, encoded as:
 *
 *  000 READ         100 WRITE     
 *  001 RESERVED     101 RESERVED  
 *  010 EXCHANGE     110 RESERVED  
 *  011 RESERVED     111 NUT       
 */
#define CBEAH_TTL_SHIFT 18
#define CBEAH_TTL (CBUS_TT << CBEAH_TTL_SHIFT)
#define CBEAH_TTH_SHIFT		50
#define CBEAH_TTH		(CBUS_TT << CBEAH_TTH_SHIFT)
#define CBEAH_TT		(CBEAH_TTL|CBEAH_TTH)

/*
 * Commander ID -- Contains the commander identification code, encoded as:
 *
 *  000 RESERVED     100 I/O     
 *  001 CPU 1	     101 RESERVED
 *  010 CPU 2	     110 RESERVED
 *  011 RESERVED     111 RESERVED
 */
#define CBEAH_CIDL_SHIFT 21
#define CBEAH_CIDL (CBUS_CID << CBEAH_CIDL_SHIFT)
#define CBEAH_CIDH_SHIFT	53
#define CBEAH_CIDH		(CBUS_CID << CBEAH_CIDH_SHIFT)
#define CBEAH_CID		(CBEAH_CIDL|CBEAH_CIDH)

/*
 * CPU CSR10 - Processor mailbox (PMBX).  
 * These are simply 64-bit ports between processors, and 
 * shouldn't be confused with I/O mailboxes.  They are intended to be used
 * for communication in the absence of any memory subsystem, during system
 * initialization and exception processing.
 */





/*
 * CPU CSR11 - Inter-Processor Interrupt Request register (IPIR).
 *
 * Implements the interprocessor interrupt IPR specified in the Alpha SRM.
 * As these are CBus visible, any CBus commander including the owner can
 * post an interprocessor interrupt to any CPU.  The interrupt is driven
 * into the processor signal identified internally as HIRR(3).
 */
#define IPIR_RI	(1UL << 32) /* Request CPU Interrupt */

/*
 * CPU CSR12 - System Interrupt Clear register (SIC).
 *
 * The System Interrupt Clear register provides a path for the CPU to clear
 * the edge triggered interrupts from the CBus C_ERR_L signal, the SYS_EVENT_L
 * signal, and the interval timer interrupt, CINT_TIM signal.  The C_ERR_L and
 * SYS_EVENT_L signals are broadcast to both CPU modules.  The interval timer
 * clock is received from the CBus and used to generate an Interval Timer
 * Interrupt to each processor.  The Interval Timer Interrupt is local to each
 * CPU so that this interrupt will occur 180 degrees out of phase with the
 * other processor node.  The system generic SYSTEM EVENT INTERRUPT is
 * generated by an I/O halt request, an Operator Control Panel HALT request,
 * an enclosure event, or a power supply event.  The generic transaction error
 * signal C_ERR_L is generated by soft or hard errors related to data
 * transactions.
 */

/*
 * CBus Error Interrupt Clear -- A read of this register returns the state
 * of the interrupt signal to the processor.  A STQ to this register with a
 * 1 in this bit position will clear the latched C_ERR_L interrupt signal
 * driven to the local CPU.  This interrupt can be masked in the 21064 chip.
 */
#define SIC_EIC	 (1UL << 2)	/* CBus Error Interrupt Clear */

/*
 * Interval Timer Interrupt Clear -- A read of this register returns the state
 * of the interrupt signal to the processor. A STQ to this register with a 1
 * in this bit position will clear the latched CINT_TIM interrupt signal driven
 * to the local CPU.  This interrupt can be masked in the 21064 chip.
 */
#define SIC_ITIC (1UL << 32)	/* Interval Timer Interrupt Clear */

/*
 * System Event Clear -- A read of this register returns the state of the
 * request signal to the processor.  A STQ to this register witha 1 in this bit
 * position will clear the latched CSYS_EVENT_L interrupt signal driven to the
 * local CPU.  This interrupt can be masked in the 21064 chip.
 */
#define SIC_SEIC (1UL << 33)	/* System Event Clear */

/*
 * CPU CSR13 - address lock register (ADLK).
 *
 * This register is required by the Alpha SRM to support the LDxL and STxC
 * instructions.  This is supported in "memory like" regions only (Address
 * <33> is 0).  This register will latch the address and set the Lock Address
 * Valid bit when a LDxL instruction to memory address space is executed.  The
 * Lock Address Valid bit will be cleared when a STxC to any location or a
 * CBus write to the locked location occurs even if the write is from this
 * node, or by explicitly clearing it by writing bit <0>.  If the Lock Address
 * Valid bit is set, and a LDxL to a different location occurs, the contents of
 * the Lock Address field will be updated with the new address.
 *
 * The resolution of the address lock in the DEC 4000 system is a single
 * aligned 32 byte block.
 *
 * Each half of the CSR is associated with one of two "slices".  Bit masks
 * are defined for bits in the low slice (nameL), the high slice (nameH) 
 * and both slices (name).
 */
#define ADLK_AVL (1UL << 0)		/* Lock Address Valid Low */
#define ADLK_AVH (1UL << 32)		/* Lock Address Valid High */
#define ADLK_AV  (ADLK_AVL|ADLK_AVH)	/* Lock Address Valid */

#define ADLK_AL_SHIFT 3
#define ADLK_AL (CBUS_LK << ADLK_LAL_SHIFT) /* Lock Address Low */
#define ADLK_AH_SHIFT 35
#define ADLK_AH (CBUS_LK << ADLK_LAH_SHIFT) /* Lock Address High */
#define ADLK_A  (ADLK_AL|ADLK_AH) /* Lock Addresses */

/*
 * CPU CSR14 - miss address register (MADRL).
 *
 * The Miss Address Register captures the CBus read or exchange address on
 * every miss, and holds the 64th sample until a sample valid flag is cleared.
 * The read or exchange may have resulted from a 21064 read or write
 * transaction.  The latching strobe is skewed by 32 counts between the low
 * and high longwords of CSR 14.
 */

/*
 * Miss Address Valid -- Read-only copy of CSR 7 bit 31.
 */
#define MADRL_MAVL (1UL << 0)		   /* Miss Address Valid Low */
#define MADRL_MAVH (1UL << 32)		   /* Miss Address Valid High */
#define MADRL_MAV (MADRL_MAVL|MADRL_MAVH)  /* Miss Address Valie */

/*
 * Transaction Type -- Set indicates Read, clear indicates Exchange.
 */
#define MADRL_TTL (1UL << 1)  /* Transaction Type Low (1=Read, 0=Exchange) */
#define MADRL_TTH (1UL << 33) /* Transaction Type High (1=Read, 0=Exchange) */
#define MADRL_TT  (MADRL_TTL|MADRL_TTH) /* Transaction Type */

/*
 * Address.
 */
#define MADRL_AL_SHIFT 2
#define MADRL_AL (CBUS_PA << MADRL_AL_SHIFT)/* Addr<33:4> from CAD<31:2> */
#define MADRL_AH_SHIFT 34
#define MADRL_AH (CBUS_PA << MADRL_AH_SHIFT)/* Addr<33:4> from CAD<63:34>*/
#define MADRL_A (MADRL_AL|MADRL_AH)

/*
 * CPU CRS15 - C^3 Revision Register (REV).
 *
 * Read-only register containing the revision of the CPU module C^3 chips.
 *
 *  00  Rev 1
 *  00  Rev 2 (If the bits 14 and 13 of the CBCTL Register can be set and
 *	       cleared and the revision in the CRR field indicate a 0 then the
 *	       bus interface slice is rev. 2)
 *  11  Rev 3
 */
#define CRR_REVL_SHIFT 0
#define CRR_REVL (KN430_REV << CRR_REVL_SHIFT)
#define CRR_REVH_SHIFT 32
#define CRR_REVH (KN430_REV << CRR_REVH_SHIFT)
#define CRR_REV (CRR_REVL|CRR_REVH)

/*****************************************************************************
 * I/O MODULE CSRS							     *
 *****************************************************************************
 *
 * The I/O module has two gate arrays which contain the CBus interface logic
 * and other functionality.  These gate arrays, referred to as IONICs, are
 * arranged in slices to handle even and odd longwords on the CBus.
 *
 * The IONICs interface to both the Futurebus+ (FBus) and a 32-bit local I/O
 * bus (LBus) which has:
 *
 *	5 NCR 710 SCSI Interface Controllers.
 *  128MB Script RAM for use by the NCR Controllers.
 *	1 or 2 TGEC Ethernet Interface Controllers
 *	1 Dallas-style clock chip with 50 bytes of NVRAM.
 *	1 UART controller (2 ports).
 *
 * The control bits for the even (low) IONIC are located in the low longword
 * of each CSR.  The corresponding control bits for the odd (high) IONIC are
 * located in the corresponding bits of the high longword of each CSR.  The
 * control bits must always be changed simultaneously -- setting the slices
 * independently will cause undefined behavior.
 */

struct io_csr {
	u_long iocsr;	u_long fill_00[3]; /* I/O Control/Status */
	u_long cerr1;	u_long fill_02[3]; /* Cbus Error Register */
	u_long cerr2;	u_long fill_04[3]; /* Cbus Cmd/Adr latch <63:0> */
	u_long cerr3;	u_long fill_06[3]; /* Cbus Cmd/Adr latch <127:64> */
	u_long lmbpr;	u_long fill_08[3]; /* Lbus Mbox Ptr (bits <31:6>) */
	u_long fmbpr;	u_long fill_0a[3]; /* Fbus Mbox Ptr (bits <31:6>) */
	u_long diagcsr;	u_long fill_0c[3]; /* Diagnostic CSR */
	u_long fivect;	u_long fill_0e[3]; /* FBus Interrupt Vector (FIFO) */
	u_long fhvect;	u_long fill_10[3]; /* FBus Halt Vector (FIFO 0=empty)*/
	u_long ferr1;	u_long fill_12[3]; /* FBus Error Register */
	u_long ferr2;	u_long fill_14[3]; /* FBus Error Address Even/Odd */
	u_long io_lint;	u_long fill_16[3]; /* Local Interrupt Register */

	u_long lerr1;	u_long fill_18[3]; /* LBus Error Register */
	u_long lerr2;	u_long fill_1a[3]; /* LBus Error Address Even/Odd */
			u_long fill_1c[4];
			u_long fill_1e[4];

/*
 * LBus and FBus Diagnostic Mode CSRs.  
 *
 * Used to access the LBus cache line buffers during powerup diagnostics.  When
 * bits 0 and 32 in the DIAGCSR registers are set, the Diagnostic Mode Data
 * registers are enabled.  When the bits are clear, the registers will respond
 * to transactions but the data returned will be unpredictable.  These
 * registers are used in conjunction with the Merge Select bits in the DIAGCSR
 * register to steer the data through the internal data paths to provide
 * diagnostic coverage.  Switching from normal mode to diagnostic mode may
 * destroy data currently in the cache line buffers.
 *
 * Naming is as follows:  Dwxyz
 *
 *        w: f = FBus, l = LBus
 *        x: c = cache line data location, m = mailbox data location
 *        y: a = 1st cache line buffer, b = 2nd cache line buffer
 *        z: Quadword within cache line
 */
	u_long dlca0;	u_long fill_20[3];
	u_long dlca1;	u_long fill_22[3];
	u_long dlca2;	u_long fill_24[3];
	u_long dlca3;	u_long fill_26[3];
	u_long dlcb0;	u_long fill_28[3];
	u_long dlcb1;	u_long fill_2a[3];
	u_long dlcb2;	u_long fill_2c[3];
	u_long dlcb3;	u_long fill_2e[3];
	u_long dlma0;	u_long fill_30[3];
	u_long dlma1;	u_long fill_32[3];
	u_long dlma2;	u_long fill_34[3];
	u_long dlma3;	u_long fill_36[3];
	u_long dlmb0;	u_long fill_38[3];
	u_long dlmb1;	u_long fill_3a[3];
	u_long dlmb2;	u_long fill_3c[3];
	u_long dlmb3;	u_long fill_3e[3];
/*
 * FBus Diagnostic Mode CSRs
 */
	u_long dfca0;	u_long fill_40[3];
	u_long dfca1;	u_long fill_42[3];
	u_long dfca2;	u_long fill_44[3];
	u_long dfca3;	u_long fill_46[3];
	u_long dfcb0;	u_long fill_48[3];
	u_long dfcb1;	u_long fill_4a[3];
	u_long dfcb2;	u_long fill_4c[3];
	u_long dfcb3;	u_long fill_4e[3];
	u_long dfma0;	u_long fill_50[3];
	u_long dfma1;	u_long fill_52[3];
	u_long dfma2;	u_long fill_54[3];
	u_long dfma3;	u_long fill_56[3];
	u_long dfmb0;	u_long fill_58[3];
	u_long dfmb1;	u_long fill_5a[3];
	u_long dfmb2;	u_long fill_5c[3];
	u_long dfmb3;	u_long fill_5e[3];

	/*
	 * Diagnostic Address CSRs
	 *
	 * These CSRs allow access to the address pointers used by the FBus
	 * and LBus interfaces.  They are enabled by DIAGCSR bits 0 and 32.
	 * These CSRs should not be acessed when FBus or LBus activity is
	 * present.  These are read only registers.
	 */
	u_long dlaa;	u_long fill_60[3+3*4];
	u_long dlab;	u_long fill_68[3+3*4];
	u_long dfaa;	u_long fill_70[3+3*4];
	u_long dfab;	u_long fill_78[3+3*4];
};
typedef struct io_csr * io_csrp_t;

/*
 * I/O Control/Status Register (IOCSR).
 *
 * This CSR has some bits which are not duplicated (they're wired to devices
 * other than the two IONIC slices).
 */

/*
 * Ethernet Halt Enable (EHE), Ethernet Halt (EH)
 *
 * EHE allows remote trigger messages from the local Ethernet interfaces to
 * assert the System Event interrupt on the CBus.  Use the individual
 * TGEC device registers to enable/disable a single device.  EH is asserted
 * if either TGEC has requested a CPU halt.
 */
#define IOCSR_EHE  (1UL << 0)		   /* Ethernet Halt Enable. */
#define IOCSR_EH   (1UL << 1)		   /* Ethernet Halt */

/*
 * Futurebus+ Halt Enable (FHE), Futurebus+ Halt (FH)
 *
 * FHE allows writes by a Futurebus+ I/O device to the FBus Halt Request
 * register to assert the System Event interrupt on the CBus.  FH is
 * asserted if an FBus device is requesting a CPU halt.  Reading the FBus
 * Halt Vector will provide the vector for the requesting device.
 */
#define IOCSR_FHE  (1UL << 4)		   /* FBus Halt Enable */
#define IOCSR_FH   (1UL << 5)		   /* FBus Halt */

/*
 * Futurebus+ Interrupt Status (FIS), Futurebus+ Power Fail
 */
#define IOCSR_FIS  (1UL << 6)		   /* FBus Int Queue has data */
#define IOCSR_FPF  (1UL << 8)		   /* FBus Power Fail */

/*
 * **********************  Slice-specific control ****************************
 */

#define IOCSR_MEL  (1UL << 9)		   /* Mailbox Enable Low (even) */
#define IOCSR_MEH  (1UL << 41)		   /* Mailbox Enable High (odd) */
#define IOCSR_ME   (IOCSR_MEL|IOCSR_MEH)   /* Mailbox Enable */

#define IOCSR_FDEL (1UL << 10)		   /* FBus DMA Enable Low (even) */
#define IOCSR_FDEH (1UL << 42)		   /* FBus DMA Enable High (odd) */
#define IOCSR_FDE  (IOCSR_FDEL|IOCSR_FDEH) /* FBus DMA Enable */

#define IOCSR_LDEL (1UL << 11)		   /* LBus DMA Enable Low (even) */
#define IOCSR_LDEH (1UL << 43)		   /* LBus DMA Enable High (odd) */
#define IOCSR_LDE  (IOCSR_LDEL|IOCSR_LDEH) /* LBus DMA Enable */

/*
 * The processor must time the assertion of FR for a minimum of 100msec and a
 * maximum of 200msec.  Should clear the FBus and the FMPBR at the same time.
 */
#define IOCSR_FRL  (1UL << 7)		   /* Assert FBus reset (RE_L) */
#define IOCSR_FRH  (1UL << 39)		   /* Clear Odd FBus intfc logic */
#define IOCSR_FR   (IOCSR_FRL|IOCSR_FRH)   /* Assert FBus reset */

#define IOCSR_FMRL (1UL << 15)		   /* FBus MBX Ptr Reset Low (even) */
#define IOCSR_FMRH (1UL << 47)		   /* FBus MBX Ptr Reset High (odd) */
#define IOCSR_FMR  (IOCSR_FMRL|IOCSR_FMRH) /* FBus MBX Ptr (FMBPR) Reset */

/*
 * Should clear the LMBPR and the LBus interface logic at the same time.
 */
#define IOCSR_LMRL (1UL << 16)		   /* LBus MBX Ptr Reset Low (even) */
#define IOCSR_LMRH (1UL << 48)		   /* LBus MBX Ptr Reset High (odd) */
#define IOCSR_LMR  (IOCSR_LMRL|IOCSR_LMRH) /* LBus MBX Ptr (LMBPR) Reset */

#define IOCSR_LRL  (1UL << 17)		   /* LBus Intfc Reset Low (even) */
#define IOCSR_LRH  (1UL << 49)		   /* LBus Intfc Reset High (odd) */
#define IOCSR_LR   (IOCSR_LRL|IOCSR_LRH)   /* LBus Interface Reset */

/* ***************************** Diagnostics ******************************* */

#define IOCSR_CAWWP0	(1UL << 12)	/* Cmd/Adr Write Wrong Par 0. */
#define IOCSR_CAWWP1	(1UL << 44)	/* Cmd/Adr Write Wrong Par 1. */
#define IOCSR_CAWWP2	(1UL << 13)	/* Cmd/Adr Write Wrong Par 2. */
#define IOCSR_CAWWP3	(1UL << 45)	/* Cmd/Adr Write Wrong Par 3. */

#define IOCSR_DWWPL	(1UL << 14)     /* Data Write Wrong Parity Low (even) */
#define IOCSR_DWWPH	(1UL << 46)    /* Data Write Wrong Parity High (odd) */
#define IOCSR_DWWP	(IOCSR_DWWPL|IOCSR_DWWPH) /* Data Write Wrong Parity */

#define IOCSR_FCPL	(1UL << 18)	/* FBus Complement Parity Low (even) */
#define IOCSR_FCPH	(1UL << 50)	/* FBus Complement Parity High (odd) */
#define IOCSR_FCP	(IOCSR_FCPL|IOCSR_FCPH) /* FBus Complement Parity */

#define IOCSR_LCPL	(1UL << 19)	/* LBus Complement Parity Low (even) */
#define IOCSR_LCPH	(1UL << 51)	/* LBus Complement Parity High (odd) */
#define IOCSR_LCP	(IOCSR_LCPL|IOCSR_LCPH) /* LBus Complement Parity */

/*
 * I/O CBus Error Register (CERR1).
 *
 * Each half of the CSR is associated with one of two "slices".  Bit masks
 * are defined for bits in the low slice (nameL), the high slice (nameH) 
 * and both slices (name).
 */

/*
 * FBus Mailbox Error -- Asserted when an error is detected during the
 * processing of a mailbox operation directed to the FBus.  The contents of the
 * FMBPR are frozen until this bit is cleared.  Write 1 to clear.  Cleared on
 * powerup or after a CBus reset.
 */
#define CERR1_FME (1UL << 15)	/* FBus MBX Error */

/*
 * LBus Mailbox Error -- Asserted when an error is detected during the
 * processing of a mailbox operation directed to the LBus.  The contents of the
 * LMBPR are frozen until this bit is cleared.  Write 1 to clear.  Cleared on
 * powerup or after a CBus reset.
 *
 * NOTE: This condition will not currently be generated by the I/O module and
 *       all LBus mailbox errors will be indicated within the status field of
 *       the mailbox data structure.
 */
#define CERR1_LME (1UL << 47)	/* LBus MBX Error */

/*
 * Bus Synchronization Error -- This bit is set when the I/O module detects
 * command fields on the upper and lower quadwords of the CBus that do not
 * match.  This error is an indication that the gate array devices that make
 * up the interface for the current bus master are out of sync.  This error
 * condition is checked for all CBus transactions whether or not the I/O
 * module is the CBus master.  When this bit is set the failing command/address
 * is saved in CERR2 and CERR3.  Write 1 to clear.  Cleared on power up or
 * after a CBus reset.
 */
#define CERR1_BSE (1UL << 17)	/* Bus Synchronization Error */

/*
 * Uncorrectable Read Error -- Set when, while acting as the CBus master, the
 * I/O module receives an uncorrectable read data error from the selected
 * device.  The failing command/address is saved in CERR2 and CERR3.  Write 1
 * to clear.  Cleared at powerup or after a CBus reset.
 */
#define CERR1_UREL (1UL << 0)		   /* Uncorr Read Error Low (even) */
#define CERR1_UREH (1UL << 32)		   /* Uncorr Read Error High (odd) */
#define CERR1_URE  (CERR1_UREL|CERR1_UREH) /* Uncorr Read Error */

/*
 * Command/Address No Acknowledge Error -- Set when, while acting as the CBus
 * master, the I/O module fails to receive an acknowledge for the C/A that it
 * has placed on the bus.  This error indicates that the selected device does
 * not exist, the selected device is broken, the I/O module is broken, or a
 * C/A Parity Error has occurred during the cycle.  The failing C/A is saved
 * in CERR 2 and CERR3.  Write 1 to clear.  Cleared on powerup or after a CBus
 * reset.
 */
#define CERR1_CANAL (1UL << 1)			/* Cmd/Adr NoAck Low (even) */
#define CERR1_CANAH (1UL << 33)			/* Cmd/Adr NoAck High (odd) */
#define CERR1_CANA  (CERR1_CANAL|CERR1_CANAH)	/* Command/Address No-Ackn */

/*
 * Command/Address Parity Error -- Set when a parity error is detected on a
 * Command/Address transfer regardless of whether the transfer is directed
 * to this module or not.  When this bit is set, the error status in the
 * CERR2 and CERR3 registers and in CERR1[9:8] is available to help locate
 * the source of the error.  Write 1 to clear.  Cleared on powerup.
 */
#define CERR1_CAPEL (1UL << 2)		      /* Cmd/Adr Par Err Low (even) */
#define CERR1_CAPEH (1UL << 34)		      /* Cmd/Adr Par Err High (odd) */
#define CERR1_CAPE  (CERR1_CAPEL|CERR1_CAPEH) /* Cmd/Adr Parity Error */

#define CERR1_MCAPEL (1UL << 3)		    /* Missed C/A Par Err Low (even) */
#define CERR1_MCAPEH (1UL << 35)	    /* Missed C/A Par Err High (odd) */
#define CERR1_MCAPE  (CERR1_MCAPEL|CERR1_MCAPEH)  /* Missed C/A Parity Err */

/*
 * Responder Write Data Parity Error -- Set when a longword parity error is
 * detected during the transfer of write data from a commander node on the CBus
 * to an I/O module primary I/O space register (I/O module is the CBus
 * responder).  If this indicator is set, the full error status (including the
 * failing Command/Address, and Data Parity Error bits) is available to help
 * identify the source of this error.  Write 1 to clear.  Cleared on powerup.
 */
#define CERR1_WDPEL (1UL << 4)		/* Rsp Wr Data Par Err Low (even) */
#define CERR1_WDPEH (1UL << 36)		/* Rsp Wr Data Par Err High (odd) */
#define CERR1_WDPE  (CERR1_WDPEL|CERR1_WDPEH)  /* Rspdr Wrt Data Par Err */

#define CERR1_MWDPEL (1UL << 5)	    /* Missed Rsp Wr Data Par Err Low (even) */
#define CERR1_MWDPEH (1UL << 37)    /* Missed Rsp Wr Data Par Err High (odd) */
#define CERR1_MWDPE (CERR1_MWDPEL|CERR1_MWDPEH) /*Missed Rsp Wr Data Parr Err*/

/*
 * Read Data Parity Error -- Set when a longword parity error is detected
 * during the transfer of read data from a responder node when the I/O module
 * is the commander.  If this indicator is set, the full error status
 * (including the failing Command/Address, and Data Parity Error bits) is
 * available to help identify the source of this error.  Write 1 to clear.
 * Cleared on powerup.
 */
#define CERR1_RDPEL (1UL << 6)		     /* Read Data Par Err Low (even) */
#define CERR1_RDPEH (1UL << 38)		     /* Read Data Par Err High (odd) */
#define CERR1_RDPE  (CERR1_RDPEL|CERR1_RDPEH)/* Read Data Parity Error */

#define CERR1_MRDPEL (1UL << 7)	     /* Missed Read Data Par Err Low (even) */
#define CERR1_MRDPEH (1UL << 39)     /* Missed Read Data Par Err High (odd) */
#define CERR1_MRDPE  (CERR1_MRDPEL|CERR1_MRDPEH) /* Missed Rd Data par Err */

/*
 * Commander Write Data Parity Error -- Set if a longword write data parity
 * error is detected while the I/O module is acting as the CBus commander.
 * When this bit is set, the failing command/address is saved in  CERR2 and
 * CERR3.  Write 1 to clear.  Cleared on powerup or after a CBus reset.
 */
#define CERR1_CWDPEL		(1UL << 16)
#define CERR1_CWDPEH		(1UL << 48)
#define CERR1_CWDPE		(CERR1_CWDPEL|CERR1_CWDPEH)

/*
 * CSTALL Sync Error -- Set if a CSTALL out of sync error is detected.
 */
#define CERR1_CSEL (1UL << 14)		   /* CSTALL Sync Err Low */
#define CERR1_CSEH (1UL << 46) 		   /* CSTALL Sync Err High */
#define CERR1_CSE  (CERR1_CSEL|CERR1_CSEH) /* CSTALL Sync Error */

/*
 * CA Parity Error Longword Identifiers -- These bits are set when a parity
 * error on the corresponding longword is detected, during a CBus transaction.
 * Write 1 to clear.  Undefined on powerup.
 */
#define CERR1_CAPE0 (1UL << 8)  /* Cmd/Adr Par Err LW 0 */
#define CERR1_CAPE1 (1UL << 40) /* Cmd/Adr Par Err LW 1 */
#define CERR1_CAPE2 (1UL << 9)  /* Cmd/Adr Par Err LW 2 */
#define CERR1_CAPE3 (1UL << 41) /* Cmd/Adr Par Err LW 3 */
#define CERR1_CAPEODD (CERR1_CAPE3|CERR1_CAPE1) /* CA Par Odd */
#define CERR1_CAPEEVEN (CERR1_CAPE2|CERR1_CAPE0) /* CA Par Even */
#define CERR1_CAPESUM (CERR1_CAPEODD|CERR1_CAPEEVEN) /* CA Par Err Bits */

/*
 * Data Parity Error Longword Identifiers -- These bits are set when a parity
 * error is detected on the corresponding longword during the data portion of
 * a CBus transation.
 */
#define CERR1_DPE0 (1UL << 10) /* Data Par Err LW 0 */
#define CERR1_DPE1 (1UL << 42) /* Data Par Err LW 1 */
#define CERR1_DPE2 (1UL << 11) /* Data Par Err LW 2 */
#define CERR1_DPE3 (1UL << 43) /* Data Par Err LW 3 */
#define CERR1_DPE4 (1UL << 12) /* Data Par Err LW 4 */
#define CERR1_DPE5 (1UL << 44) /* Data Par Err LW 5 */
#define CERR1_DPE6 (1UL << 13) /* Data Par Err LW 6 */
#define CERR1_DPE7 (1UL << 45) /* Data Par Err LW 7 */
#define CERR1_DPEODD (CERR1_DPE7|CERR1_DPE5|CERR1_DPE3|CERR1_DPE1)/*DPE Odd*/
#define CERR1_DPEEVEN (CERR1_DPE6|CERR1_DPE4|CERR1_DPE2|CERR1_DPE0)/*DPE Even*/
#define CERR1_DPESUM (CERR1_DPEODD|CERR1_DPEEVEN) /* Data Par Err bits */

/*
 * All the parity error bits.
 */
#define CERR1_PARERR (CERR1_DPESUM|CERR1_CAPESUM|CERR1_MRDPE|CERR1_RDPE|\
		      CERR1_MWDPE|CERR1_WDPE|CERR1_MCAPE|CERR1_CAPE)

				 
/*
 * I/O FERR1
 *
 * Each half of the CSR is associated with one of two "slices".  Bit masks
 * are defined for bits in the low slice (nameL), the high slice (nameH) 
 * and both slices (name).
 */

/*
 * FBus Data Parity Error -- Set when a parity error is detected during a FBus
 * write transaction when the I/O module is operating as the FBus slave.
 * When this bit is set the C_ERR hardware error interrupt will be asserted
 * on the CBus.  Write 1 to clear, cleared by the FBus reset bit in the IOCSR
 * or by a CBus reset.
 */
#define FERR1_DPEL (1UL << 0)		    /* FBus Data Par Err Low (even) */
#define FERR1_DPEH (1UL << 32)		    /* FBus Data Par Err High (odd) */
#define FERR1_DPE  (FERR1_DPEL|FERR1_DPEH)  /* FBus Data Parity Error */

/*
 * FBus Address Parity Error -- Set when an address parity error is detected
 * during the connection phase of an FBus transaction.  The I/O module is
 * operating as the potential FBus slave during this operation.  WEhen this
 * bit is set the C_ERR hardware error interrupt will be asserted on the CBus.
 * Since the address is latched in both the even and odd array slices either
 * one or both slices may detect the parity error.  Write 1 to clear, cleared
 * by the FBus Reset bit in the IOCSR or by a CBus reset.
 */
#define FERR1_APEL		(1UL << 1)
#define FERR1_APEH		(1UL << 33)
#define FERR1_APE		(FERR1_APEL|FERR1_APEH)

/*
 * I/O Local Interrupt Register (LINT)
 *
 */
#define LINT_SCSI0_IRQ (1UL << 0)	/* SCSI Bus 0 Interrupt */
#define LINT_SCSI1_IRQ (1UL << 1)	/* SCSI Bus 1 Interrupt */
#define LINT_SCSI2_IRQ (1UL << 2)	/* SCSI Bus 2 Interrupt */
#define LINT_SCSI3_IRQ (1UL << 3)	/* SCSI Bus 3 Interrupt */
#define LINT_SCSI4_IRQ (1UL << 4)	/* SCSI Bus 4 Interrupt */
#define LINT_SLU_IRQ   (1UL << 32)	/* Serial Line Controller Interrupt */
#define LINT_NI0_IRQ   (1UL << 33)	/* Ethernet 0 Interrupt */
#define LINT_NI1_IRQ   (1UL << 34)	/* Ethernet 1 Interrupt */
#define LINT_SBC_IRQ   (1UL << 35)	/* Serial (IIC) Bus Interrupt */

/*
 * I/O LBUS Error Register (LERR1)
 *
 * Each half of the CSR is associated with one of two "slices".  Bit masks
 * are defined for bits in the low slice (nameL), the high slice (nameH) 
 * and both slices (name).
 */

/*
 * Data Parity Error -- Indicates that there was a data parity error on the
 * LBus detected by the gate array.  Data parity errors are indicated during
 * the following transactions:
 *
 *	TGEC DMA Write Cycle
 *	SCSI Controller DMA Write Cycle
 *
 * Parity errors during a mailbox read of the SCSI Script RAM or TGEC CSRs are
 * reported in the status field of the mailbox data structure.  Parity errors
 * during a TGEC or SCSI controller DMA read cycle are reported by the
 * specific device.
 *
 * When this bit is set, the C_ERR hardware interrupt will be asserted on the
 * CBus.  Write 1 to clear.  Cleared on powerup or by setting the LBus reset
 * bits in the IOCSR register.
 */
#define LERR1_DPEL (1UL << 0)
#define LERR1_DPEH (1UL << 32)
#define LERR1_DPE  (LERR1_DPEL|LERR1_DPEH)

/*****************************************************************************
 * MEMORY MODULE CSRS							     *
 *****************************************************************************
 *
 * The DEC 4000 accomodates up to four memory modules.  Each memory module has
 * two or four banks of DRAMs, and two ASIC slices to handle memory control and
 * CBus traffic.  There are no configuration rules for ordering memories in
 * their slots; the slot ID is picked up from the backplane when a module is
 * plugged in.
 *
 * Memory modules may be interleaved to improve performance.  Interleaving
 * increases performance on sequential memory accesses, and also increases
 * the chance that memory refresh cycles can be hidden behind accesses to
 * another board.  Cache lines are never split accross boards, even when they
 * are interleaved.
 *
 * Two or four identical boards may be interleaved, using two-way or four-way
 * interleaving.  Two-way interleaving may also be used to interleave boards
 * with different sizes, if multiple boards can be combined to have the same
 * capacity as the board which they are interleaved with.  For example, a
 * 256Mb board may be two-way interleaved with two 128Mb boards.
 *
 * The interleave mode, interleave unit, module size, and module base address
 * determine which addresses a module responds to.  The revision field
 * indicates the revision of the memory module ASICs.  The slot field is used
 * for error reporting, to correlate errors with the physical board location.
 * The only way that the slot number comes in to play in system setup is the
 * choice of a board to load the console.  When the system comes up, the
 * largest memory which is closest to the CPU (lowest slot number) is chosen
 * to load the console.
 */

struct mem_csr {
	u_long error;	u_long fill_00[3];  /* CSR0: Error Register */
	u_long ctrap1;	u_long fill_02[3];  /* CSR1: CBus Command Trap 1 */
	u_long ctrap2;	u_long fill_04[3];  /* CSR2: CBus Command Trap 2 */
	u_long config;	u_long fill_06[3];  /* CSR3: Memory Configuration */
	u_long edcstat1; u_long fill_08[3]; /* CSR4: EDC Status 1 */
	u_long edcstat2; u_long fill_0A[3]; /* CSR5: EDC Status 2 */
	u_long edcctl;	u_long fill_0C[3];  /* CSR6: EDC Control */
	u_long stream;	u_long fill_0E[3];  /* CSR7: Stream Buffer Control */
	u_long refresh;	u_long fill_10[3];  /* CSR8: Refresh Control */
	u_long filter;	u_long fill_12[3];  /* CSR9: CRD Filter Control */
			u_long fill_14[4];
			u_long fill_16[4];
			u_long fill_18[4];
			u_long fill_1A[4];
			u_long fill_1C[4];
	u_long force;	u_long fill_1E[3];  /* CSRF: Tester Forced Refresh */
};
typedef struct mem_csr * mem_csrp_t;

/*
 * Memory module field masks.
 * 
 */
#define MEM_REV	       0xFUL /* Memory Revision */
#define MEM_SYN	     0xFFFUL /* Memory EDC Syndrome */
#define MEM_ILVM       0x3UL /* Memory Interleave Mode */
#define MEM_SZ	       0x7UL /* Memory Size (bank is 2, DRAM size is <1:0>) */

#define MEM_SLOT       0x3UL /* Memory Slot (IO Space PA<28:29>)*/

#define MEM_BASE     0x1FFUL /* Memory Base (CAD<30:22>,PA<32:24>) */
#define MEM_ILVU       0x3UL /* Mem Intrlv Unit Selct (CAD<[4:]3>,PA<[6:]5>) */
#define MEM_BANK       0x3UL /* Memory Bank Select (PA<[19:]18>) */


/*
 * Memory Error Register (CSR0)
 *
 * Contains the error identification information for a failed CBus
 * transaction.  This register includes the Error Summary bits, along with any
 * lower-level error flags as may be provided by the CBus node.  Only the
 * first error detected is saved.  If subsequent errors are detected the
 * command and address information for those errors are not saved, but a bit
 * in CSR0 will be set to indicate that an error was missed.
 *
 * Certain bits within CSR0 are specific to functions wholly contained within
 * CMIC1 (the master ASIC on the memory module), while other bits reflect the
 * status from functions wholly contained within CMIC2 (the slave ASIC on the
 * memory module).  The bits provided by CMIC1 are located in the even/low LW
 * of CSR0, while the bits provided by CMIC2 are located in the odd/high
 * longword of CSR0.
 */

/*
 * Error Summary -- This is the "OR" of other bits within CSR0.  When CSR0 is
 * set the contents of CSR0 are frozen, with the exception of the missed error
 * bits, which are updated in the event of a subsequent error.  This bit
 * cannot be cleared directly, but must be cleared by clearing the error bits
 * which caused it to be asserted.
 */
#define MERR_ESL (1UL << 0)		/* Mem Error Summary Low */
#define MERR_ESH (1UL << 32)		/* Mem Error Summary High */
#define MERR_ES  (MERR_ESL|MERR_ESH)	/* Memory Error Summary */
#define MERR_ESUM MERR_ES

/*
 * Synchronization Error -- Set when the two CMICs are out of synch.
 * Write 1 to clear.  Cleared on powerup.  Does NOT contribute to the state
 * of MERR_ES.
 */
#define MERR_SYNCL (1UL << 1)		   /* Mem Synchronization Error Low */
#define MERR_SYNCH (1UL << 33)		   /* Mem Synchronization Error High */
#define MERR_SYNC  (MERR_SYNCL|MERR_SYNCH) /* Memory Synchronization Error */

/*
 * Command/Address Parity Error -- Set when a CBus Parity Error is detected
 * during the transfer of Command/Address from the commander node to the
 * selected memory module.  These bits are set by any memory module on the CBus
 * when a parity error occurs, regardless of whether a module is the intended
 * responder.  MERR_CAPESUM indicates which longwords contained the error(s).
 */
#define MERR_CAPEL (1UL << 2)		   /* Mem Cmd/Adr Parity Error Low */
#define MERR_CAPEH (1UL << 34)		   /* Mem Cmd/Adr Parity Error High */
#define MERR_CAPE  (MERR_CAPEL|MERR_CAPEH) /* Mem Command/Address Parity Err */

/*
 * Missed Command/Address Parity Error -- Set when a CBus Parity Error is
 * detected after a previous error has been detected.  Write 1 to clear.
 */
#define MERR_MCAPEL (1UL << 3)		  /* Mem Missed Cmd/Adr Par Err Low */
#define MERR_MCAPEH (1UL << 35)		  /* Mem Missed Cmd/Adr Par Err High */
#define MERR_MCAPE  (MERR_MCAPEL|MERR_MCAPEH) /* Mem Missed Cmd/Adr Par Err */

/*
 * Write Data Parity Error -- Set when a CBus Parity Error is detected during
 * the transfer of data from the commander node to the selected memory module.
 * This bit is set only by the module that is the current responder.
 * MERR_DPESUM indicates which longwords actually contained the error(s).
 * Write 1 to clear.  Cleared on powerup.

 */
#define MERR_WDPEL (1UL << 4)		   /* Mem Write Data Par Err Low */
#define MERR_WDPEH (1UL << 36)		   /* Mem Write Data Par Err High */
#define MERR_WDPE  (MERR_WDPEL|MERR_WDPEH) /* Mem Write Data Par Error */

/*
 * Missed Write Data Parity Error -- Set when a CBus Parity Error is detected
 * during the transfer of data from the commander node to the selected memory
 * module, after a previous error has been detected.  Write 1 to clear.
 */
#define MERR_MWDPEL (1UL << 5)		  /* Mem Missed Wr Data Par Err Low */
#define MERR_MWDPEH (1UL << 37)		  /* Mem Missed Wr Data Par Err High */
#define MERR_MWDPE  (MERR_MWDPEL|MERR_MWDPEH) /* Mem Missed Wrt Data Par Err */

/*
 * EDC Uncorrectable Error -- Set when an EDC Uncorrectable Error is detected
 * on data read from the DRAM array.  The full error status is available in
 * other CSRs.  Write 1 to clear.
 */
#define MERR_EDCUEL (1UL << 16) 		/* Mem EDC Uncorr Err Low */
#define MERR_EDCUEH (1UL << 48) 		/* Mem EDC Uncorr Err High */
#define MERR_EDCUE  (MERR_EDCUEL|MERR_EDCUEH)   /* Mem EDC Uncorrectable Err */

/*
 * Missed EDC Uncorrectable Error -- Set when an EDC Uncorrectable Error is
 * detected on data read from the DRAM array, after some other error has
 * already been set in this CSR.  Write 1 to clear.  Cleared on powerup.
 */
#define MERR_MEDCUEL (1UL << 17) 	   /* Mem Missed EDC Uncorr Err Low */
#define MERR_MEDCUEH (1UL << 49) 	   /* Mem Missed EDC Uncorr Err High */
#define MERR_MEDCUE (MERR_MEDCUEL|MERR_MEDCUEH) /* Mem Missed EDC Uncorr Err */

/*
 * EDC Correctable Error -- Set on detection of an EDC Correctable Error on
 * data read from the DRAM array.  Write 1 to clear.  Cleared on powerup.
 */
#define MERR_EDCCEL (1UL << 18)		      /* Mem EDC Corr Err Low */
#define MERR_EDCCEH (1UL << 50)		      /* Mem EDC Corr Err High */
#define MERR_EDCCE  (MERR_EDCCEL|MERR_EDCCEH) /* Mem EDC Corr Err */

/*
 * Missed EDC Correctable Error -- Set on detection of an EDC Correctable
 * Error on data read from the DRAM array, after some other error has been set
 * into this CSR.  Write 1 to clear.  Cleared on powerup.
 */
#define MERR_MEDCCEL		(1UL << 19)
#define MERR_MEDCCEH		(1UL << 51)
#define MERR_MEDCCE		(MERR_MEDCCEL|MERR_MEDCCEH)


/*
 * Command/Address Parity Error Longword Indicators -- When MERR_CAPE is
 * asserted, these bits indicate which longwords had parity errors.
 * Cleared by writing 1 to MERR_CAPE.
 */
#define MERR_CAPE0 (1UL << 8)	/* Mem Cmd/Adr Parity Err LW 0 */
#define MERR_CAPE1 (1UL << 40)	/* Mem Cmd/Adr Parity Err LW 1 */
#define MERR_CAPE2 (1UL << 9)	/* Mem Cmd/Adr Parity Err LW 2 */
#define MERR_CAPE3 (1UL << 41)	/* Mem Cmd/Adr Parity Err LW 3 */
#define MERR_CAPEE (MERR_CAPE0|MERR_CAPE2)
#define MERR_CAPEO (MERR_CAPE1|MERR_CAPE3)
#define MERR_CAPESUM (MERR_CAPEL|MERR_CAPEH)

/*
 * Data Parity Error Longword Indicators -- When MERR_WDPE is set, these bits
 * indicate which longwords had parity errors.  Cleared by writing a 1 to
 * MERR_WDPE.  
 */
#define MERR_DPE0 (1UL << 10)	/* Mem Data Parity Err LW 0 */
#define MERR_DPE1 (1UL << 42)	/* Mem Data Parity Err LW 1 */
#define MERR_DPE2 (1UL << 11)	/* Mem Data Parity Err LW 2 */
#define MERR_DPE3 (1UL << 43)	/* Mem Data Parity Err LW 3 */
#define MERR_DPE4 (1UL << 12)	/* Mem Data Parity Err LW 4 */
#define MERR_DPE5 (1UL << 44)	/* Mem Data Parity Err LW 5 */
#define MERR_DPE6 (1UL << 13)	/* Mem Data Parity Err LW 6 */
#define MERR_DPE7 (1UL << 45)	/* Mem Data Parity Err LW 7 */
#define MERR_DPEE (MERR_DPE0|MERR_DPE2|MERR_DPE4|MERR_DPE6)
#define MERR_DPEO (MERR_DPE1|MERR_DPE3|MERR_DPE5|MERR_DPE7)
#define MERR_DPESUM (MRR_DPEE|MERR_DPEO)

/*
 * Memory Command Trap 1 (CSR1)
 *
 * Read-only register that contains a complete copy of the CBus Command/Ad-
 * dress cycle information present on CBus CAD<63:0> during CBus State 1.  In
 * the event that an error is detected by the memory module, the contents of
 * this element will be frozen by the assertion of MERR_ES, and will remain in
 * this condition until subsequently unfrozen by resetting all of the
 * individual error bits in CSR0.
 *
 * When error recording is enabled, but no error has been previously detected,
 * a read to this element will provide the Command/Address information from
 * the current operation.  If an error has been detected and recorded, a read
 * to this element will provide the Command/Address from the failed operation.
 *
 * Memory Command Trap 2 (CSR2) is similar to CSR1, holding a copy of the
 * CBUS Command/Address cycle informatino present on CBus CAD<127:0> during
 * CBus State 1.
 *
 * The format of the two Command Trap registers is similar, so the masks
 * defined here may be used with either Command Trap register.
 */

#define MCMD_ADDR_SHIFT 2
#define MCMD_ADDR (CBUS_PA << MCMD_ADDR_SHIFT) /* PA bits <33:4> */

#define MCMD_ECHADR_SHIFT 66
#define MCMD_ECHADR (CBUS_EA << MCMD_ECHADR_SHIFT) /* Exchange Addr bits <> */

#define MCMD_TRANS_SHIFT 82
#define MCMD_TRANS (CBUS_TT << MCMD_TRANS_SHIFT) /* Transaction Type */

#define MCMD_CID_SHIFT 85
#define MCMD_CID (CBUS_CID << MCMD_CID_SHIFT) /* Commander ID */

/*
 * Convert PA bits in CSR to a physical address.  PA bits <33:4> are stored in
 * CSR bits <2:31>.  Shifting left two yields a PA.
 */
#define MCMD_PA_SHIFT (4 - MCMD_ADDR_SHIFT)
#define MCMD_PA(mcmd) (((mcmd) & MCMD_ADDR) << MCMD_PA_SHIFT)

/*
 * The bank select bits are PA 19..18 if four banks are present, or PA 18 if
 * two banks are present.  PA bit 18 corresponds to CSR bit 16.
 */
#define MCMD_BANK_SHIFT (18 - MCMD_PA_SHIFT)
#define MCMD_BANK (MEM_BANK << MCMD_BANK_SHIFT)

/*
 * For IIC logging.
 */
#define  MCMD1_ADR6_5_SHIFT     3
#define  MCMD1_ADR6_5           (0x3UL << MCMD1_ADR6_5_SHIFT)
#define  MCMD1_ADR13PLUS_SHIFT  11
#define  MCMD1_ADR28TO13        (0x0FFFFUL << MCMD1_ADR13PLUS_SHIFT)
#define  MCMD1_ADR27TO13        (0x07FFFUL << MCMD1_ADR13PLUS_SHIFT)
#define  MCMD1_ADR26TO13        (0x03FFFUL << MCMD1_ADR13PLUS_SHIFT)
#define  MCMD1_ADR25TO13        (0x01FFFUL << MCMD1_ADR13PLUS_SHIFT)
#define  MCMD1_ADR24TO13        (0x00FFFUL << MCMD1_ADR13PLUS_SHIFT)
#define  MCMD1_ADR23TO13        (0x007FFUL << MCMD1_ADR13PLUS_SHIFT)
#define  ADR13_TO_OFFSET2_SHIFT 9

/*
 * Memory Config register -- CSR3.
 *
 * Read/write register that provides global memory space configuration control
 * and status for the memory module.  I/O write operations affect both CMIC1
 * and CMIC2 simultaneously.  CSR3 bits<31:0> are used by CMIC1, while bits
 * <63:32> are used by CMIC2.  The contents of the two longwords of this
 * register must always be equal to avoid unpredictable results.  This
 * register must be written prior to the first access to module memory space.
 */

/*
 * Module Slot Identification -- Slot 0 is closest to the CPU modules.  Slot 3
 * is furthest away.
 */
#define MCONF_SLOTL_SHIFT 0
#define MCONF_SLOTL (MEM_SLOT << MCONF_SLOTL_SHIFT) /* Mem Slot Low */
#define MCONF_SLOTH_SHIFT 32
#define MCONF_SLOTH (MEM_SLOT << MCONF_SLOTH_SHIFT) /* Mem Slot High */
#define MCONF_SLOT (MCONF_SLOTL|MCONF_SLOTH)	    /* Mem Slot */

/*
 * Module Size -- The firmware writes this field during powerup.  The DRAM
 * type used and the number of DRAM banks populating the array are controlled
 * by this field.  The configuration data is obtained from the EEPROM on the
 * memory module via the IIC bus.  Cleared on powerup.
 *
 * The DRAM type is encoded in MSIZE<1:0>, and the number of banks is encoded
 * in MSIZE<2>:
 *
 * MSIZE<1,0>   DRAM TYPE		MSIZE<2>   NUMBER OF BANKS
 * ----------   ---------               --------   -------------------------
 *    0, 0      RESERVED		    0      2 (half populated module)
 *    0, 1      4 Mbit (1MW x 4b)           1	   4 (fully populated module)
 *    1, 0      16 Mbit (4MW x 4b)
 *    1, 1      RESERVED
 */
#define MCONF_MSIZEL_SHIFT 4
#define MCONF_MSIZEL (MEM_SZ << MCONF_MSIZEL_SHIFT)	/* Mem Size Low */
#define MCONF_MSIZEH_SHIFT 36
#define MCONF_MSIZEH (MEM_SZ << MCONF_MSIZEH_SHIFT)	/* Mem Size High */
#define MCONF_MSIZE (MCONF_MSIZEL|MCONF_MSIZEH)		/* Mem Size */

#define MCONF_DSIZEL_SHIFT MCONF_MSIZEL_SHIFT
#define MCONF_DSIZEL (MEM_SZ << MCONF_DSIZEL_SHIFT)	/* DRAM Size Low */
#define MCONF_DSIZEH_SHIFT MCONF_MSIZEH_SHIFT
#define MCONF_DSIZEH (MEM_SZ << MCONF_DSIZEH_SHIFT)	/* DRAM Size High */
#define MCONF_DSIZE (MCONF_DSIZEL|MCONF_DSIZEH)		/* DRAM Size */

#define MCONF_BANKSL (1UL << (MCONF_MSIZEL_SHIFT + 2))	/* Mem Banks Low */
#define MCONF_BANKSH (1UL << (MCONF_MSIZEH_SHIFT + 2))	/* Mem Banks High */
#define MCONF_BANKS (MCONF_BANKSL|MCONF_BANKSH)		/* Mem Banks */

/* (For compatability with log_iic_error.c and test_iic.c */
#define MCONF_SIZEL_SHIFT MCONF_MSIZEL_SHIFT
#define MCONF_SIZEL MCONF_MSIZEL
#define MCONF_BANK MCONF_BANKS
#define MCONF_BANKL MCONF_BANKSL
#define MCONF_BANKH MCONF_BANKSH

/*
 * Enable Memory Diagnostic Mode -- Used to place the module into diagnostic
 * mode.  Allows multiple memory modules to be configured to occupy the same
 * memory space, but have only a single module drive the CBus during Read Data
 * transfers.  Used to implement parallel memory test.
 */
#define MCONF_DIAGL (1UL << 8)			/* Mem Diag Mode Low */
#define MCONF_DIAGH (1UL << 40)			/* Mem Diag Mode High */
#define MCONF_DIAG (MCONF_DIAGL|MCONF_DIAGH)	/* Mem Diag Mode */

/*
 * CMIC Hardware Revision Level -- Read-only bits indicating the revision of
 * the Memory Hardware:
 *
 * VALUE  HARDWARE/
 * (bin)  DESCRIPTION
 * -----  -----------
 *  0000  DC7226A	No correctable EDC interrupts, errors are corrected.
 *			No stream buffers.  Not shipped in "real" systems.
 *			Severe limitations in back-to-back CBus transactions.
 *			False reporting of EDC errors, and failure to report
 *			some that did occur.
 *  0001  DC7226B	Pass 2.  No stream buffers.  On MP system, no EDC/CRD
 *			correction, but still have reporting (need to crash
 *			if EDC/CRD is reported).  Shipped in all systems.
 *			Stream buffers did work in some configurations, but
 *			V2.8 console doesn't check for them.  There are lots of
 *			these parts in the pipe; we may need to support them.
 *  0010  DC7226D	(Pass 3/C doesn't exist -- jumped to Pass 4/D).
 *			Should be fully functional.
 *  0011  RESERVED
 *  X1XX  RESERVED
 *  1XXX  RESERVED
 */
#define MCONF_REVL_SHIFT 12
#define MCONF_REVL (MEM_REV << MCONF_REVL_SHIFT)   /* Mem CMIC Revision Low */
#define MCONF_REVH_SHIFT 44
#define MCONF_REVH (MEM_REV << MCONF_REVH_SHIFT)   /* Mem CMIC Revision High */
#define MCONF_REV (MCONF_REVL|MCONF_REVH)	   /* Mem CMIC Revision */

/*
 * Interleaving Mode Select
 *	 0=No Interleaving	 	2=Four Way interleaving
 *	 1=Two Way Interleaving	 	3=undefined
 *
 * Two Way Interleaving requires a pair of modules with identical capacity.
 * Two pairs can be set up with two way interleaving, as long as the larger
 * pair is configured to be first in memory.  Also, modules with different
 * capacities may be combined into a two way interleave set.  For example,
 * a 512Mb board could be interleaved with a set of three boards with
 * capacities of 128Mb, 128Mb, and 256Mb.
 *
 * Four Way Interleaving requires four modules with identical capacity.
 */
#define MCONF_ILVML_SHIFT 18
#define MCONF_ILVML (MEM_ILVM << MCONF_ILVML_SHIFT) /* Mem Intrlv Mode Low */
#define MCONF_ILVMH_SHIFT 50
#define MCONF_ILVMH (MEM_ILVM << MCONF_ILVMH_SHIFT) /* Mem Intrlv Mode High */
#define MCONF_ILVM (MCONF_ILVML|MCONF_ILVMH)	    /* Mem Interleave Mode */

/*
 * Interleaving Unit Select -- Defines the order in which modules will respond
 * under the interleaved module selection process (there is no required slot
 * arangement for memories).
 *
 * Modules are interleaved in hexaword chunks
 */
#define MCONF_ILVUL_SHIFT 20
#define MCONF_ILVUL (MEM_ILVU << MCONF_ILVUL_SHIFT) /* Mem Intrlv Unit Low */
#define MCONF_ILVUH_SHIFT 52
#define MCONF_ILVUH (MEM_ILVU << MCONF_ILVUH_SHIFT) /* Mem Intrlv Unit High */
#define MCONF_ILVU (MCONF_ILVUL|MCONF_ILVUH)	    /* Mem Interleave Unit */

/*
 * Base CBus Memory Address -- These fields specify the base address for the
 * board (first address which the board responds to).  The base address
 * represents PA bits 32:24, aligning the board on a 16Mb boundary.  Boards
 * are further constrained to be aligned on a boundary which is a multiple
 * of their size:  This requires that largest boards be configured first.
 */
#define MCONF_BASEL_SHIFT 22
#define MCONF_BASEL (MEM_BASE << MCONF_BASEL_SHIFT) /* Mem Base Addr Low */
#define MCONF_BASEH_SHIFT 54
#define MCONF_BASEH (MEM_BASE << MCONF_BASEH_SHIFT) /* Mem Base Addr High */
#define MCONF_BASE (MCONF_BASEL|MCONF_BASEH)	    /* Mem Base Address */

/*
 * Memory Enable -- When deasserted, the board does not respond to CBus
 * commands.
 */
#define MCONF_ENBL (1U << 31)			/* Mem Enable Low */
#define MCONF_ENBH (1UL << 63)			/* Mem Enable High */
#define MCONF_ENB (MCONF_ENBL|MCONF_ENBH)	/* Mem Enable */

/*
 * Memory EDC Status 1 Register -- CSR4
 *
 * Contains the EDC Write Check bits last written to memory, along with the
 * EDC Read Check bits last writen to memory.  Updated for each valid memory
 * space access made to the particular module.  When a hard error occurs, the
 * bits in this register are frozen.
 *
 */
#define EDCSTAT1_RCHKL_SHIFT 0
#define EDCSTAT1_RCHKL (MEM_SYN << EDCSTAT1_RCHKL_SHIFT)
#define EDCSTAT1_RCHKH_SHIFT 16
#define EDCSTAT1_RCHKH (MEM_SYN << EDCSTAT1_RCHKH_SHIFT)
#define EDCSTAT1_RCHK (EDCSTAT1_RCHKL|EDCSTAT1_RCHKH)

#define EDCSTAT1_WCHKL_SHIFT 32
#define EDCSTAT1_WCHKL (MEM_SYN << EDCSTAT1_WCHKL_SHIFT)
#define EDCSTAT1_WCHKH_SHIFT 48
#define EDCSTAT1_WCHKH (MEM_SYN << EDCSTAT1_WCHKH_SHIFT)
#define EDCSTAT1_WCHK (EDCSTAT1_WCHKL|EDCSTAT1_WCHKH)

/*
 * Memory EDC Status 2 Register -- CSR5
 *
 * Contains state information from the EDC (Error Detection and Correction)
 * logic within both CMIC1 and CMIC2.  Updated for each valid memory space
 * access made to the particular module.  When an EDC error is detected, the
 * contents of this register are frozen.
 */
#define EDCSTAT2_SYNL_SHIFT 0
#define EDCSTAT2_SYNL MEM_SYN			       /* Mem Syndrome Low */
#define EDCSTAT2_SYNH_SHIFT 32
#define EDCSTAT2_SYNH (MEM_SYN << EDCSTAT2_SYNH_SHIFT) /* Mem Syndrome High */

/*
 * Memory EDC Control Register -- CSR 6.
 *
 * Read/write reigster that is used to control EDC and parity checking
 * functions for both CMIC1 and CMIC2.
 */

/*
 * Enable CRD Reporting -- When set, CRD reporting is enabled for the
 * corresponding CMIC(s).  When disabled, logging of correctable errors to
 * CSRs 0, 1, 2, 4, and 5 will not be performed.
 */
#define EDCCTL_ENACRDL (1UL << 29)		/* Enb CRD Reporting Low */
#define EDCCTL_ENACRDH (1UL << 61)		/* Enb CRD Reporting High */
#define EDCCTL_ENACRD (EDCCTL_ENACRDL | EDCCTL_ENACRDH) /* Enb CRD Reporting */

/*
 * Disable EDC Correction -- When set, correction of EDC errors is suppressed.
 * However, reporting of EDC errors will occur to the appropriate EDC status
 * CSR bits.
 *
 * Pass 2 memory CMICs have EDC correction disabled when running in a dual CPU
 * configuration, because EDC correction does not work properly when two CPUs
 * are present.  It is necessary to crash if an EDC error is reported on such
 * a system.
 */
#define EDCCTL_DISEDCCL (1UL << 30)		/* Dis EDC Correction Low */
#define EDCCTL_DISEDCCH (1UL << 62)		/* Dis EDC Correction High */
#define EDCCTL_DISEDCC (EDCCTL_DISEDCCL | EDCCTL_DISEDCCH) /* Dis EDC Corrn */

/*
 * Disable EDC Reporting -- When set, reporting of EDC Uncorrectible errors
 * to the CBus CUCERR_L signal is disabled.  Also, logging of state to CSRs
 * 0, 1, 2, 4, or 5 is disabled.  Useful for filtering if more than one cell
 * is bad on a board.
 */
#define EDCCTL_DISEDCRL (1UL << 31)		/* Disable EDC Reporting */
#define EDCCTL_DISEDCRH (1UL << 63)		/* Disable EDC Reporting */
#define EDCCTL_DISEDCR (EDCCTL_DISEDCRL | EDCCTL_DISEDCRH) /* Dis EDC Report */

/*
 * Substitute Read Check Bits -- When USE_RB is set, the Substitute Read Check
 * Bits are used in place of the Read Check Bits normally fetched from the
 * DRAMs and input to the EDC Check And Correct function.  These are used for
 * generating errors to test the correctable and uncorrectable error logic.
 */
#define EDC1_RBL_SHIFT 0
#define EDC1_RBL (0xFFFL << EDC1_RBL_SHIFT)
#define EDC1_RBH_SHIFT 32
#define EDC1_RBH (0xFFFL << EDC1_RBH_SHIFT)
#define EDC1_WB (EDC1_WBL | EDC1_WBH)

#define EDCCTL_USE_RBL (1L << 12)
#define EDCCTL_USE_RBH (1L << 44)
#define EDCCTL_USE_RB (EDCCTL_USE_RBL | EDCCTL_USE_RBH)


/*
 * Substitute Write Check Bits -- When USE_WB is set, the Substitute Write
 * Check Bits are used in place of the Write Check Bits normally fetched from
 * the DRAMs and input to the EDC Check And Correct function.  These are used
 * for generating errors to test the correctable and uncorrectable error logic.
 */
#define EDC1_WBL_SHIFT 16
#define EDC1_WBL (0xFFFL << EDC1_WBL_SHIFT)
#define EDC1_WBH_SHIFT 48
#define EDC1_WBH (0xFFFL << EDC1_WBH_SHIFT)
#define EDC1_RB (EDC1_RBL | EDC1_RBH)

#define EDCCTL_USE_WBL (1L << 13)
#define EDCCTL_USE_WBH (1L << 45)
#define EDCCTL_USE_WB (EDCCTL_USE_WBL | EDCCTL_USE_WBH)

/*
 * Memory EDC CRD Filter Control Register -- CSR 9.
 *
 * Provides a facility for masking of a selected correctable error from
 * setting any bits in CSRs 0, 1, 2, and 5.  This allows the filtering of one
 * particular, possibly frequently occurring, correctable error in favor of
 * logging other, possibly infrequent, correctable errors into the error
 * registers on the CMM.
 *
 * The CRD filter operates on EDC symbols from a single DRAM in a single bank.
 * Once CSR9 is loaded with the specified syndrome pattern and the specific
 * bank id and the enable bit is set, correctable errors with the same
 * syndrome bits in the same bank cease to be logged.  Note that this does not
 * preclude correction of the CRD error being masked.  Both single-bit and
 * double-bit correctable errors can be masked using the CRD filter logic.
 *
 * Unlik most of the CSRs, the two halves of CSR9 can be set independently
 * without any spurious effects to the CMM operation.  A particular symbol
 * error can be masked for CMIC1, while a different symbol error is masked for
 * CMIC2.
 */

/*
 * Syndrome masks.
 */
#define EDCF_SYNDROMEL_SHIFT 0
#define EDCF_SYNDROMEL (MEM_SYN << EDCF_SYNDROMEL_SHIFT) /* CRD Fltr Syn Low */
#define EDCF_SYNDROMEH_SHIFT 32
#define EDCF_SYNDROMEH (MEM_SYN << EDCF_SYNDROMEH_SHIFT) /* CRD Fltr Syn Hi */

/*
 * Syndrome Bank Select.
 */
#define EDCF_BANKL_SHIFT 12
#define EDCF_BANKL (MEM_BANK << EDCF_BANKL_SHIFT) /* CRD Filter Bank Sel Low */
#define EDCF_BANKH_SHIFT 44
#define EDCF_BANKH (MEM_BANK << EDCF_BANKH_SHIFT) /* CRD Filtr Bank Sel High */

/*
 * Filter Enable.
 */
#define EDCF_ENBL (1UL << 14)	/* Enable CRD Filter Low */
#define EDCF_ENBH (1UL << 46)	/* Enable CRD Filter High */
#define EDCF_ENB (EDCF_ENBL|EDCF_ENBH)	/* Enable CRD Filters */


/*****************************************************************************
 * KN430 WATCH CHIP DEFINITIONS.					     *
 *****************************************************************************
 *
 * V0.6 of the I/O spec shows the watch chip as being mapped to longword
 * boundaries.  The actual implementation aligns the chip registers on
 * byte boundaries.
 */

struct kn430_watch {
	u_char	seconds;	/* Seconds in minute (0-59) */
	u_char	sec_alarm;	   /* Alarm seconds */
	u_char	minutes;	/* Minutes in hour (0-59) */
	u_char	min_alarm;	   /* Alarm minute */
	u_char	hours;		/* Hour in day (0-23 or 0-12) */
	u_char	hrs_alarm;	   /* Alarm hour */
	u_char	day_of_wk;	/* Day in week (1-7) */
	u_char	day_of_mon;	/* Day in month (1-31) */
	u_char	month;		/* Month in year (1-12) */
	u_char	year;		/* Year (0-99) */
	u_char	reg_a;		/* Update in progress */
	u_char	reg_b;		/* Mode */
	u_char	reg_c;		/* */
	u_char	reg_d;		/* */
	u_char	ram[50];	/* NVRAM (used by console) */
};

/*
 * Calculate offset of watch chip CSR, for LBus address.
 */
#define KN430_WATCH_OFFSET(x) ((long)&(((struct kn430_watch *)0)->x))

/* Watch Chip Register A constants. */
#define LBUS_TOY_UIP	0x80	/* Update in Progress */

/* Watch Chip Register B constants. */
#define LBUS_TOY_SET	0x80	/* Hold values while setting */
#define LBUS_TOY_PIE	0x40	/* Enable Periodic Interrupts */
#define LBUS_TOY_AIE	0x20	/* Enable Alarm */
#define LBUS_TOY_UIE	0x10	/* Update Ended Interrupt Enable */
#define LBUS_TOY_SQWE	0x08	/* Square Wave Enable */
#define LBUS_TOY_DM	0x04	/* Data Mode (0 BCD, 1 BINARY) */
#define LBUS_TOY_24	0x02	/* Hours Format (1 24, 0 12) */
#define LBUS_TOY_DSE	0x01	/* Daylight Savings Enable */

/* Watch Chip Register D constants. */
#define LBUS_TOY_VRT	0x80	/* */

/*
 * The expected watch chip mode is:
 *
 *	Bit	Description				State
 *	---	-----------				-----
 *      SET	Prevents updates when on.		0 but don't care.
 *      PIE	Periodic Interrupt Enable.		Don't care.
 *      AIE	Alarm Interrupt Enable.			Don't care.
 *	UIE	Updated Ended interrupt Enable.		Don't care.
 *	SQWE	Square Wave Enable.			1
 *	DM	Data Mode (0 BCD, 1 BINARY).		1
 *	24/12	Hours Format (1 24, 0 12).		1
 *	DSE	Daylight Savings Enable.		0
 *
 */
#define LBUS_TOY_EXPECTED_MODE (LBUS_TOY_SQWE|LBUS_TOY_DM|LBUS_TOY_24)

/*****************************************************************************
 * DEC 4000 MAILBOX/LBUS DEFINITIONS.					     *
 *****************************************************************************/

/*
 * Parameters for mailbox access to LBus (used by the watch chip access
 * routines in kn430.c).
 */
#define LBUS_TIMEOUT 20000

/* Basic DEC 4000 LBus commands */
#define LBUS_WRITE          (0x00000008 | WRT_CSR)
#define LBUS_READ           0x00000000
#define LBUS_EE             0x00000010
#define LBUS_UART           0x00000020
#define LBUS_SCSI_SCRPT     0x00000040
#define LBUS_SCSI_CTLR      0x00000080
#define LBUS_TOY_CLK        0x00000100
#define LBUS_TGEC           0x00000200
#define LBUS_ENET_ADDR_ROM  0x00000400
#define LBUS_SRL_BUS        0x00000800
#define LBUS_FLASH          0x00001000

/*
 * Machine Check SCB Vectors.
 *
 * These values are specified by the SRM.  KN430_VECTOR_FRU is a special
 * case.
 */
#define KN430_VECTOR_PROC_FATAL 0x670
#define KN430_VECTOR_SYS_FATAL 0x660
#define KN430_VECTOR_PROC_CORR 0x630
#define KN430_VECTOR_SYS_CORR 0x620  /* unused */
#define KN430_VECTOR_FRU 0x0

/*
 * External entries for testing purposes.
 */
extern void kn430_edc_reset();
extern int kn430_checkpoint_footprints();
#endif /* KN430_H */
