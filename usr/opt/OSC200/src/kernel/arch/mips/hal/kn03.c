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
static char *sccsid = "@(#)$RCSfile: kn03.c,v $ $Revision: 1.2.3.7 $ (DEC) $Date: 1992/10/13 12:14:04 $";
#endif 
/*
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
/* 
 * derived from kn03.c
 */

/* Change History:
 *
 * 27-Jun-91	Scott Cranston
 *     Changed optional compilation of binary error logging support strategy.
 *       - added #include <uerf.h>
 *       - chnaged instances of '#ifdef _UERF' to '#if UERF'
 *
 * 06-June-91	Dave Gerson 
 *	Merged bsd code: added kn03_conf_clk_speed(), additional logic
 *	in kn03_init. Added kn03_delay(), changed name of nv_csr and
 *      nv_diag to add kn03 prefix. 
 *
 * 23-Feb-10	Paul Grist
 *	Created file for support of 3MAX-plus and BIGMAX  (DS_5000_300).
 *
 */

#include <sys/systm.h>
#include <machine/cpu.h>
#include <sys/proc.h>
#include <mach/machine.h>
#include <hal/cpuconf.h>
#include <machine/reg.h>
#include <hal/mc146818clock.h>
#include <sys/syslog.h>

#include <sys/vmmac.h>
#include <sys/presto.h>

#include <sys/table.h>

#include <io/dec/tc/tc.h>
#include <io/common/devdriver.h>

#include <sys/socket.h>
#include <net/if.h>

#include <dec/binlog/errlog.h>

/*
 * KN03 System Registers and Locations
 *
 */
 
#define KN03_SSR_ADDR		0x1F840100	/* System Support Register */
#define	KN03_SIR_ADDR		0x1F840110	/* System Interrupt Register */
#define	KN03_SIRM_ADDR		0x1F840120	/* System Intr Mask Register */
#define	KN03_SIRM_K1ADDR	0xbF840120	/* SIRM in K1 space */

#define KN03_SL_0_ADDR 		0x1E000000	/* TC OPtion 0 */
#define KN03_SL_1_ADDR 		0x1E800000	/* TC Option 1 */
#define KN03_SL_2_ADDR 		0x1F000000	/* TC Option 2 */
#define KN03_SL_3_ADDR 		0x1FB00000	/* Base SCSI   */
#define KN03_SL_4_ADDR 		0x1F8C0000	/* Base Lance  */
#define KN03_SL_5_ADDR 		0x1F900000	/* Base SCC    */
#define KN03_SL_6_ADDR 		0x0
#define KN03_SL_7_ADDR 		0x0

#define KN03_SCSI_ADDR	     	0x1FB00000	/* SCSI Base Address */
#define KN03_LN_ADDR		0x1F8C0000	/* LANCE Base Address */
#define KN03_SCC_ADDR		0x1F900000	/* SCC Base Address */
#define	KN03_CLOCK_ADDR		0x1FA00000	/* Clock Base Address */

#define KN03_SCSI_INDEX		3		/* TC index 3 - Base scsi */
#define KN03_LN_INDEX		4		/* TC index 4 - Base lance */
#define KN03_SCC_INDEX		5		/* TC index 5 - Base comm */

/*
 * MT Chip registers
 */
#define KN03ERR_ADDR		0x1FA40000	/* Error Address Register */
#define KN03CHKSYN_ADDR		0x1FA80000	/* ECC Status Register */
#define KN03CSR_ADDR		0x1FAC0000	/* MT chip, Memory CSR */


/*
 * KN03 SIR Register bit definitions
 *
 */

#define COMM1_XMIT	0x80000000	/* Comm Port 1 Xmit Intr 	*/
#define COMM1_XMIT_DMA	0x40000000	/* Comm Port 1 Xmit DMA Error	*/
#define COMM1_RECV	0x20000000	/* Comm Port 1 Recv Intr 	*/
#define COMM1_RECV_DMA	0x10000000	/* Comm Port 1 Recv DMA Error	*/

#define COMM2_XMIT	0x08000000	/* Comm Port 2 Xmit Intr 	*/
#define COMM2_XMIT_DMA	0x04000000	/* Comm Port 2 Xmit DMA Error	*/
#define COMM2_RECV	0x02000000	/* Comm Port 2 Recv Intr 	*/
#define COMM2_RECV_DMA	0x01000000	/* Comm Port 2 Recv DMA Error	*/

#define	RESERVED_23	0x00800000	/* Reserved Bit 23		*/
#define	RESERVED_22	0x00400000	/* Reserved Bit 22		*/
#define	RESERVED_21	0x00200000	/* Reserved Bit 21		*/
#define	RESERVED_20	0x00100000	/* Reserved Bit 20		*/

#define SCSI_DMA_INTR	0x00080000	/* SCSI DMA buffer ptr loaded	*/
#define SCSI_DMA_ORUN	0x00040000	/* SCSI DMA Overrun Error	*/
#define SCSI_DMA_MEM	0x00020000	/* SCSI DMA Mem Read Error	*/
#define LANCE_DMA_MEM	0x00010000	/* LANCE DMA Mem Read Error	*/

#define ID_3MAXPLUS	0x00008000	/* 3MAX+ Identification Bit	*/
#define	UNSCUR_JMPR	0x00004000	/* Security Mode Jumper		*/
#define TC_SLOT_2_INTR	0x00002000	/* TC Option 2 interrupt	*/
#define TC_SLOT_1_INTR	0x00001000	/* TC Option 1 interrupt	*/

#define TC_SLOT_0_INTR	0x00000800	/* TC Option 0 interrupt	*/
#define NRMOD_JMPR	0x00000400	/* Manufacturing Mode Jumper	*/
#define	SCSI_CHIP_INTR	0x00000200	/* SCSI 53c94 Chip Interrupt	*/
#define	LANCE_CHIP_INTR	0x00000100	/* LANCE Chip Interrupt		*/

#define	SCC1_INTR	0x00000080	/* SCC(1) Intr (Com 2 & kybd)	*/
#define	SCC0_INTR	0x00000040	/* SCC(0) Intr (Com 1 & mouse)	*/
#define RESERVED_5	0x00000020	/* Reserved Bit 5		*/
#define	PSWARN		0x00000010	/* Power Supply Warning		*/

#define	RESERVED_3	0x00000008	/* Reserved Bit 3		*/
#define	SCSI_DATA_RDY	0x00000004	/* SCSI Data Ready		*/
#define PBNC		0x00000002	/* PBNC				*/
#define	PBNO		0x00000001	/* PBNO				*/

#define RESERVED_BITS	(RESERVED_23 | RESERVED_22 | RESERVED_21 | \
			 RESERVED_20 | RESERVED_15 | RESERVED_5 | RESERVED_3)

/*
 * define masks to indicate the various I/O interrupts
 */
#define SLU_INTR	(COMM1_XMIT | COMM1_XMIT_DMA | COMM1_RECV | \
			 COMM1_RECV_DMA | COMM2_XMIT | COMM2_XMIT_DMA | \
			 COMM2_RECV | COMM2_RECV_DMA | SCC1_INTR | \
			 SCC0_INTR )

#define SCSI_INTR	(SCSI_DMA_INTR | SCSI_DMA_ORUN | SCSI_DMA_MEM | \
			 SCSI_CHIP_INTR )

#define LANCE_INTR	(LANCE_DMA_MEM | LANCE_CHIP_INTR)


/*
 *  KN03 IOASIC DMA SLOT Register Definitions  (KSEG1 Addresses)
 */
#define	LANCE_IO_SLOT_REGISTER 		0xbf840160  
#define SCSI_DMA_SLOT_REGISTER		0xbf840170  
#define SCC0_DMA_SLOT_REGISTER		0xbf840180  
#define SCC1_DMA_SLOT_REGISTER		0xbf840190  

#define LANCE_CHIP_SELECTS	  	0x3
#define SCSI_CHIP_SELECTS		0xe
#define SCC0_CHIP_SELECTS		0x14
#define SCC1_CHIP_SELECTS		0x16

/*
 * KN03 MT CSR Bit Definitions
 */
#define KN03CSR_ECCMD	0x0000c000		/* ECC mode bits */
#define KN03CSR_ECCCOR	0x00002000		/* ECC correct bits */
#define KN03CSR_BNK32M	0x00000400		/* 32MB modules if set*/


/*
 * KN03 Error Address Register Bit Definitions (MT Chip EA)
 */
#define ERR_VALID	0x80000000		/* ERRADR valid bit */
#define ERR_TYPE	0x70000000		/* ERRADR error type bits */
#define ERR_WECC	0x70000000		 /* CPU partial mem write ECC*/
#define ERR_WTMO	0x60000000		 /* CPU write timeout */
#define ERR_RECC	0x50000000		 /* CPU memory read ECC */
#define ERR_RTMO	0x40000000		 /* CPU read timeout */
#define ERR_DMAWOVR	0x20000000		 /* DMA write overrun */
#define ERR_DMARECC	0x10000000		 /* DMA memory read ECC */
#define ERR_DMAROVR	0x00000000		 /* DMA read overrun */
#define ERR_UKN		0x00000001		 /* unknown error */
#define ERR_ADDR	0x07ffffff		/* ERRADR error addr bits */
#define ERR_COLADDR	0x00000fff		/* ERRADR column addr bits */

/*
 * KN03 CHKSYN Bit Definitions (MT chip ES)
 */
#define CHKSYN_VLDLO	0x00008000		/* chksyn valid lo bit */
#define CHKSYN_SNGLO	0x00000080		/* chksyn lo-order single bit*/
#define CHKSYN_VLDHI	0x80000000		/* chksyn valid hi bit */
#define CHKSYN_SNGHI	0x00800000		/* chksyn hi-order single bit*/

/*
 * Bits in chksyn_plus (logged in memory error packet)
 * <31>		pc valid bit, set on bus error, cleared on intr error
 * <30:20>	error count for this module (max count for field is 2k)
 * <19:16>	module number in error (0 to 14)
 * <15:0>	half of chksyn that had valid error data
 */
#define CPLUS_VALID	0x80000000
#define CPLUS_EOFF	20
#define CPLUS_EMASK	0x7ff00000
#define CPLUS_MOFF	16
#define CPLUS_MMASK	0x000f0000
#define CPLUS_CHK	0x0000ffff
#define MAXERRCNT	2048

#define MEM_MODULES 15			/* numbered 0..MEM_MODULES-1 */
int kn03memerrs[MEM_MODULES] =		/* mem err count per module */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

#define CRDINTVL (60*15)	/* time delta to enable CRD log - 15 minutes */
int kn03crdintvl = CRDINTVL;	/* global var so we can change it */
int kn03crdlog = 1;		/* is CRD logging currently enabled */

struct kn03_crdcnt {             /* MEM_CRD */
        long   crd_last;	/* time of most recent CRD err */
        long   crd_prev;	/* time of previous CRD err */
} kn03_crdcnt = { 0, 0 };

/*
 * KN03 Memory error-handling definitions: error packet types
 */
#define ESR_INTR_PKT 1
#define ESR_BUS_PKT 2
#define MEMPKT 3

int kn03eccpanic = 0;		/* set true if already panic'ing */
int kn03erradr;			/* software copy of hardware erradr */
int kn03chksyn;			/* software copy of hardware chksyn */

/*
 *	The structure kn03consinfo_*_t can be used to store information
 *	to be printed on a console. This information can then be printed
 *	by kn03_print_consinfo() which is exported  through the cpusw.
 *
 *	Currently this is used only by the bus timeout code(although it 
 *	is a general pupose mechanism).
 */
 
/* error and information to be dumped on a console on a ESR_INTR_PKT type */

struct kn03consinfo_intr_t {
	u_int	cause;	/* from the exception frame */
	u_int	sr;	/* from the exception frame */
	u_int 	sp;	/* from the exception frame */
	u_int	csr;    /* mem csr register */
	u_int	erradr; /* mem error address reg */
	u_int 	ssr;   	/* system support reg */
	u_int 	sir;   	/* system interrupt reg */	
	u_int 	sirm;  	/* system interrupt mask */
};

/* error information to be dumped on a console on a ESR_BUS_PKT type */

struct kn03consinfo_bus_t {
	u_int	cause;		/* from the exception frame */
	u_int	sr;		/* from the exception frame */
	u_int 	sp;		/* from the exception frame */
	u_int	epc; 		/* from the exception frame */
	u_int	badvaddr; 	/* from the exception frame */
	u_int	csr;		/* mem csr register */
	u_int	erradr;		/* mem error address reg */
	u_int	ssr;   		/* system support reg */
	u_int 	sir;    	/* system interrupt reg */	
	u_int 	sirm; 	   	/* system interrupt mask */
};


/* console information to be dumped on a console on a MEMPKT type */

struct kn03consinfo_mem_t {
	u_int	csr;
	u_int	erradr;
	u_int	chksyn_plus;
	u_int	epc;
};

struct kn03consinfo_t {
	int	pkt_type;		/* pkt type */
	union {
	    struct  kn03consinfo_intr_t	intrp;
	    struct  kn03consinfo_bus_t 	busp;
	    struct  kn03consinfo_mem_t 	memp;
	} pkt;
} kn03consinfo;


/*
 *	The structure kn03log_errinfo_t can be used to store information
 *	to be logged to the error log file. This information can be logged 
 *	by kn03_log_errinfo() which is exported  through the cpusw.
 *
 *	Currently this is used only by the bus timeout code(although it 
 *	is a general pupose mechanism).
 */

struct kn03log_errinfo_t {
	int	pkt_type;
	u_int	cause;		/* from the exception frame */
	u_int	sr;		/* from the exception frame */
	u_int 	sp;		/* from the exception frame */
	u_int	epc; 		/* from the exception frame */
	u_int	badvaddr; 	/* from the exception frame */
	u_int	csr;		/* memory control/status reg  */
	u_int	erradr;		/* memory error address reg */
	u_int 	ssr;   		/* system support reg */
	u_int 	sir;   		/* system interrupt reg */	
	u_int 	sirm;   	/* system interrupt mask */

} kn03log_errinfo;


int	kn03_stray(), kn03_iointr(), kn03_halt(), kn03_errintr();
int	kn03_enable_option(), kn03_disable_option();
int	kn03_clear_errors(), kn03_isolate_memerr();
int 	kn03_print_consinfo(), kn03_log_errinfo();

caddr_t vatophys();		/* function  typedef */
int kn03_crdenable();
extern int nofault;		/* to test for parity errors */

extern u_int printstate;	/* how to print to the console */
extern char *rt_clock_addr;	/* addr of the mc146818clock chip */
extern struct timeval time;	/* the system time */
extern int hz;
extern int tick;
extern int tickadj;
extern int fixtick;		/* set to the number of microsec's that the
				   clock looses per second */
#if NCPUS > 1
extern volatile unsigned long  system_intr_cnts_type[NCPUS][INTR_TYPE_SIZE];
#else	/* NCPUS */
extern volatile unsigned long  system_intr_cnts_type[INTR_TYPE_SIZE];
#endif	/* NCPUS */

/*
 * kn03 delay multiplier - fixed for now, must be variable later for
 *			   handling multiple CPU cards.
 */

int kn03_delay_mult = 50;	/* multiplier for DELAY (worst case) */
int kn03_cpu_speed = 0;		/* cpu speed variable */

extern  softclock(), softintr2(), kn_hardclock(), fp_intr();

int     kn03_stray(), kn03_iointr(), kn03_halt(), kn03_errintr();
int     kn03_enable_option(), kn03_disable_option();
int     kn03_clear_errors(); kn03_isolate_memerr();
int     kn03_print_consinfo(), kn03_log_errinfo();

/*
 * NVRAM/Prestoserve support definitions
 *
 */
int kn03_nvram_found = 0;		      /* nvram is present on system*/

#define KN03_NVRAM_DIAG		0x3f8	      /* offset to diag reg on nvram */
#define KN03_NVRAM_ID		0x3fc	      /* offset to NVRAM ID location */
#define KN03_NVRAM_IDENTIFIER   0x03021966    /* NVRAM board ID signature */
#define KN03_NVRAM_START	0x400	      /* offset to NVRAM cache addr */ 
#define KN03_NVRAM_CSR_OFFSET   0x400000      /* offset to NVRAM csr reg */

#define KN03_NVRAM_RO      0x00000002         /* read only test failed */
#define KN03_NVRAM_RW      0x00000004         /* read/write test failed */
#define KN03_NVRAM_FAILED  0x00000008         /* MASK out fail bit */
#define NVRAM_SIZE         0x000000f0         /* MASK out nvram size bits */


#define KN03_NVRAM_BDISC    0x00000002        /* status reg mask for bdisc */
#define KN03_NVRAM_BOK      0x00000001        /* status reg mask for bok   */

unsigned kn03_nvram_start_addr=0;	      /* starting address of NVRAM */
volatile unsigned int kn03_nvram_csr;         /* status/control reg addr*/
volatile unsigned int kn03_nvram_diag;        /* diagnostic reg addr*/

short kn03_test = 0;
short kn03_nvram_debug = 0;
#define Dprintf  if(kn03_nvram_debug)printf

int kn03_sim_ie = 0;		/* Holds tmp SIM value */
int clk_counter;                /* counter value for timng CPU speed */
int kn03_pswarn = 0;		/* set true if we had a ps-warning */
int kn03_psintvl = (60 * 1);	/* time delta to check pswarn bit */


int kn03_pscheck();
caddr_t vatophys();


int kn03_model_number = 0;	/* model number variable */

/*
 *	KN03 R3000A Hardware Interrupt Assignments (5 is highest priority)
 *
 *	0 - IOASIC (Lance,scsi,scc,tc0,tc1,tc2)
 *	1 - RTC    (Clock)
 * 	2 - RES    (Reserved)
 * 	3 - MEM	   (Memory Errors)
 *	4 - HALT   (Push Button Halt)
 *	5 - FPU    (Floating Point)
 *
 */

#define KN03_HALT	SR_IBIT7

/* The routines that correspond to each of the 8 interrupt lines */
int (*kn03intr_vec[IPLSIZE])() = {
	softclock,		/* softint 0	*/
	softintr2,		/* softint 1	*/
	kn03_iointr,		/* hardint 0	*/
	kn_hardclock,		/* hardint 1	*/
	kn03_stray,		/* hardint 2	*/
	kn03_errintr,		/* hardint 3  	*/
	kn03_halt,		/* hardint 4	*/
	fp_intr			/* hardint 5	*/
};


/*
 * Interrupt table types
 */
int     kn03_c0vec_tbl_type[NC0VECS] = {
	/* softint 1 */		INTR_SOFTCLK,	/* AST */
	/* softint 2 */		INTR_SOFTCLK,	/* NETWORK or unused */
	/* hardint 3 */		INTR_NOTCLOCK,	/* IO */
	/* hardint 4 */		INTR_HARDCLK,	/* CLOCK */
	/* hardint 5 */		INTR_NOTCLOCK,	/* STRAY */
	/* hardint 6 */		INTR_NOTCLOCK,	/* ERROR */
	/* hardint 7 */		INTR_NOTCLOCK,	/* HALT */
	/* hardint 8 */		INTR_NOTCLOCK	/* FPU */
};
/*
 * Define mapping of interrupt lines with the type of interrupt.
 * This is basically taken from the table kn03vec_tbl_type declared
 * immediately about this.  Kind of seems like a waste not to be
 * able to use the above table but it is not setup exactly like we
 * need it (NOTCLOCK is not enough info for us).
 */
static int KN03_interrupt_type[INTR_MAX_LEVEL] = {
		INTR_TYPE_SOFTCLK,
		INTR_TYPE_SOFTCLK,
		INTR_TYPE_DEVICE,
		INTR_TYPE_HARDCLK,
		INTR_TYPE_STRAY,
		INTR_TYPE_OTHER,
		INTR_TYPE_OTHER,
		INTR_TYPE_OTHER
};

/* The masks to use to look at each of the 8 interrupt lines */
int kn03_iplmask[IPLSIZE] = {
	SR_IMASK1|SR_IEC,
	SR_IMASK2|SR_IEC,
	SR_IMASK3|SR_IEC,
	SR_IMASK4|SR_IEC,
	SR_IMASK5|SR_IEC,
	SR_IMASK6|SR_IEC,
	SR_IMASK7|SR_IEC,
	SR_IMASK8|SR_IEC
};


/*
 * The SR reg masks for splxxx usage, block xxx level and all levels below it.
 * Never Mask HALTs. Masks defined in cpu.h.
 */
int kn03_splm[SPLMSIZE] = {
	SR_IEC | SR_IMASK0,			/* 0 SPLNONE		*/
	SR_IEC | SR_IMASK1,			/* 1 SPLSOFTC		*/
	SR_IEC | SR_IMASK2,			/* 2 SPLNET		*/
	SR_IEC | SR_IMASK3,			/* 3 SPLBIO 		*/
	SR_IEC | SR_IMASK3,			/* 4 SPLIMP 		*/
	SR_IEC | SR_IMASK3,			/* 5 SPLTTY		*/
	SR_IEC | SR_IMASK4,			/* 6 SPLCLOCK		*/
	SR_IEC | SR_IMASK6 | KN03_HALT,		/* 7 SPLMEM		*/
	SR_IEC | SR_IMASK8 | KN03_HALT		/* 8 SPLFPU   		*/
};


u_int kn03_slotaddr[TC_IOSLOTS] = {KN03_SL_0_ADDR, KN03_SL_1_ADDR,
	KN03_SL_2_ADDR, KN03_SL_3_ADDR, KN03_SL_4_ADDR,
	KN03_SL_5_ADDR, KN03_SL_6_ADDR, KN03_SL_7_ADDR};

/*
 * Program the order in which to probe the IO slots on the system.
 * This determines the order in which we assign unit numbers to like devices.
 * It also determines how many slots (and what slot numbers) to probe.
 * Terminate the list with a -1.
 * Note: this agrees with the console's idea of unit numbers
 */


int kn03_config_order [] = { 3, 4, 5, 0, 1, 2, -1 };



kn03_init()
{
        int i;
	extern int c0vec_tblsize;
	extern int c0vec_tbl_type_size;
	extern int c0vec_tbl_type[];
	extern int (*c0vec_tbl[])();
	extern int iplmask[], splm[];
	extern struct cpusw *cpup;
	extern int kn_delay_mult;
	extern volatile int *system_intr_cnts_type_transl;
	
	/* Turn off all interrupts in the SIRM */

	*(u_int *)PHYS_TO_K1(KN03_SIRM_ADDR) = 0;
	wbflush();

	/*
	 * Setup global interrupt type array to use our definitions.
	 */
	system_intr_cnts_type_transl = KN03_interrupt_type;

	/*
	 * Initialize the interrupt dispatch table c0vec_tbl.
	 * Initialize the spl table for the system we are running on.
	 * Initialize the interrupt masks for the system we are running on.
	 * This must be done very early on at boot time and must
	 * be done before any interrupt is allowed.
	 */

	bcopy((int *)kn03intr_vec, c0vec_tbl, c0vec_tblsize);
	bcopy(kn03_c0vec_tbl_type, c0vec_tbl_type, c0vec_tbl_type_size);
	bcopy(kn03_iplmask, iplmask, IPLSIZE * sizeof(int));
	bcopy(kn03_splm, splm, (SPLMSIZE) * sizeof(int));


	/* initialize the dealy loop multiplier */
	kn_delay_mult = kn03_delay_mult;


	/* clear out any ECC errors that may be left around */
	kn03_clear_errors();

	/* disable all DMA's, but don't reset the chips */
	*(u_int *)(PHYS_TO_K1(KN03_SSR_ADDR)) = 0xf00;

	/*
	 * Set up the system specific value for "hz", the number of clock
	 * interrupts per second; and corresponding tick and tickadj values.
	 * Value found in cpusw entry.
	 */
	hz = cpup->HZ;
	tick = 1000000 / hz;
	tickadj = 240000 / (60 * hz);
	fixtick = 1000000 - (tick * hz);

	/*
	 * Assign the rt_clock_addr for this processor
	 */
	rt_clock_addr = (char *)PHYS_TO_K1(KN03_CLOCK_ADDR);

	/*
	 * Dynamically configure the Processor speed 
	 *
	 *  Supported CPU speeds (MHz): 33,36,40,45,50,60
	 * 
	 * kn03_conf_clk_speed counts the number of machine
	 * cycles in a 4ms window, and sets clk_counter to
	 * the count of how many times the test instruction loop
	 * was executed. Using the number of instruction cycles in
	 * the loop, the actual or computed value of clk_counter, 
	 * and the expected CPU speed, all the numbers can be derived.
	 * This algorithm uses the assumption that this MIPS design 
	 * approaches approximately 1 instruction/cycle to calculate 
	 * the CPU speed. 
	 *
	 * To allow for a margin of error, as not all systems
	 * will generate identical numbers, there is a window
	 * of 2MHz on each supported speed, for example, any
	 * CPU between greater than 38MHz, but less than 42MHz
	 * will be sized as 40MHz.
	 *
	 * The delay multiplier is calculated based on the
	 * CPU speed, as a ratio to known delay values for
	 * MIPS processor speeds.
	 *
	 */

	clk_counter = kn03_conf_clk_speed();

	if (clk_counter < 14000) {
	    kn03_cpu_speed = 33;
	    kn03_delay_mult = 19;
	} else if (clk_counter < 15000) {
	    kn03_cpu_speed = 36;
	    kn03_delay_mult = 21;
	} else if (clk_counter < 16000) {
	    kn03_cpu_speed = 40;
	    kn03_delay_mult = 23;	
	} else if (clk_counter < 18800) {
	    kn03_cpu_speed = 45;
	    kn03_delay_mult = 25;	
	} else if (clk_counter < 21000) {
	    kn03_cpu_speed = 50;
	    kn03_delay_mult = 29;	
	} else if (clk_counter < 26000) {
	    kn03_cpu_speed = 60;
	    kn03_delay_mult = 35;	
	} else {
	    kn03_cpu_speed = 0;
	    kn03_delay_mult = 40;
	}

	/*
	 * Clear the memory error counters.
	 */
	for (i = 0; i < MEM_MODULES; i++)
		kn03memerrs[i] = 0;

	/* Initialize the TURBOchannel */
	tc_init();

	/* Fill in the TURBOchannel slot addresses */
	for (i = 0; i < TC_IOSLOTS; i++)
	    tc_slotaddr[i] = kn03_slotaddr[i];

	/* Fill in the TURBOchannel switch table */

	tc_sw.isolate_memerr = kn03_isolate_memerr;
	tc_sw.enable_option = kn03_enable_option;
	tc_sw.disable_option = kn03_disable_option;
	tc_sw.clear_errors = kn03_clear_errors;
	tc_sw.config_order = kn03_config_order;

	/*
	 * Fixed KN03 IO devices
	 */

	strcpy(tc_slot[KN03_SCSI_INDEX].devname,"asc");
	strcpy(tc_slot[KN03_SCSI_INDEX].modulename, "PMAZ-BA ");
	tc_slot[KN03_SCSI_INDEX].slot = 3;
	tc_slot[KN03_SCSI_INDEX].module_width = 1;
	tc_slot[KN03_SCSI_INDEX].physaddr = KN03_SCSI_ADDR;
	tc_slot[KN03_SCSI_INDEX].intr_b4_probe = 0;
	tc_slot[KN03_SCSI_INDEX].intr_aft_attach = 1;
	tc_slot[KN03_SCSI_INDEX].adpt_config = 0;

	strcpy(tc_slot[KN03_LN_INDEX].devname,"ln");
	strcpy(tc_slot[KN03_LN_INDEX].modulename, "PMAD-BA ");
	tc_slot[KN03_LN_INDEX].slot = 3;
	tc_slot[KN03_LN_INDEX].module_width = 1;
	tc_slot[KN03_LN_INDEX].physaddr = KN03_LN_ADDR;
	tc_slot[KN03_LN_INDEX].intr_b4_probe = 0;
	tc_slot[KN03_LN_INDEX].intr_aft_attach = 1;
	tc_slot[KN03_LN_INDEX].adpt_config = 0;

	strcpy(tc_slot[KN03_SCC_INDEX].devname,"scc");
	tc_slot[KN03_SCC_INDEX].slot = 3;
	tc_slot[KN03_SCC_INDEX].module_width = 1;
	tc_slot[KN03_SCC_INDEX].physaddr = KN03_SCC_ADDR;
	tc_slot[KN03_SCC_INDEX].intr_b4_probe = 0;
	tc_slot[KN03_SCC_INDEX].intr_aft_attach = 1;
	tc_slot[KN03_SCC_INDEX].adpt_config = 0;

	/*
	 *  Enable chip selects on IOASIC DMA SLOT Registers
	 */

	*(u_int *)LANCE_IO_SLOT_REGISTER =  LANCE_CHIP_SELECTS;
	*(u_int *)SCSI_DMA_SLOT_REGISTER =  SCSI_CHIP_SELECTS;
	*(u_int *)SCC0_DMA_SLOT_REGISTER =  SCC0_CHIP_SELECTS;
	*(u_int *)SCC1_DMA_SLOT_REGISTER =  SCC1_CHIP_SELECTS;

	return (0);
}

/*
 * Routine to return value for global variable cpu_subtype, which
 * can be used to discern between 3MAX+ and BigMAX on a KN03 system.
 */
get_kn03_subtype()
{
    /*
     * System ID bit definition
     *
     *	gpi[15] = 1, system is 3MAX+
     *	gpi[15] = 0, system is BigMAX
     *
     */
	if ( *(int *)PHYS_TO_K1(KN03_SIR_ADDR) & ID_3MAXPLUS )
	    return(VARIANT_3MAXPLUS);
	else
	    return(VARIANT_BIGMAX);
}


kn03_conf()
{
        extern int cold;
	extern u_int cpu_systype;
        extern int icache_size, dcache_size;
	register struct bus *sysbus;
	extern cpu_subtype;
	int s;

	/* 
	 * Report what system we are on
	 */

	if ( cpu_subtype == VARIANT_BIGMAX)
	    {
		if (kn03_cpu_speed != 0)
		    printf("DECsystem 5900, %dMHz Processor\n",kn03_cpu_speed);
		else
		    printf("DECsystem 5900\n"); 
	    }
	else
	    {
		if ((strlen(rex_getenv("osconsole"))) > 1) 
		    {

		     if (kn03_cpu_speed != 0)
		       printf("DECstation 5000 Model 2%d\n", kn03_cpu_speed);
		     else
		       printf("DECstation 5000 Model 2xx, unknown model\n"); 
	            }
	        else
		     if (kn03_cpu_speed != 0)
		       printf("DECsystem 5000 Model 2%d\n", kn03_cpu_speed);
		     else
		       printf("DECsystem 5000 Model 2xx, unknown model\n"); 

	    }
	printf("KN03 - system rev %d\n", (GETHRDREV(cpu_systype)));
	printf("%dK Instruction Cache, %dK Data Cache\n",
		icache_size/1024, dcache_size/1024);

	coproc_find();
	
	/* Turn off all interrupts in the SIRM */
	*(u_int *)PHYS_TO_K1(KN03_SIRM_ADDR) = 0;
	wbflush();

	/*
	 * dgd -- Stuff from generic osf 'configure' routine.
	 *	  same stuff in kn01.c; 
	 *     -- Some other machine_slot var values set, which
	 * 	  are needed, though.
	 */
	master_cpu = 0;
#if	NCPUS > 1
	printf("Master cpu at slot %d.\n", master_cpu);
#endif	/* NCPUS > 1 */
	machine_slot[master_cpu].is_cpu = TRUE;
        machine_slot[master_cpu].cpu_type = CPU_TYPE_MIPS;
/* dgd  machine_slot[master_cpu].cpu_subtype = subtype; */
        machine_slot[master_cpu].cpu_subtype = CPU_SUBTYPE_MIPS_R3000a;
	machine_slot[master_cpu].running = TRUE;
        machine_slot[master_cpu].clock_freq = hz;

	/*
	 * Get the system bus structure and call the bus configuration code.
	 * system_bus is global pointer used by loadable code when walking
	 * the hardware topology tree.  It is extern'd via devdriver.h.
	 */
	system_bus = sysbus = get_sys_bus("tc");
	if (sysbus == 0) 
		panic("No system bus configured");
	/*
	 * Probe the TURBOchannel and find all devices
	 */
	(*sysbus->confl1)(-1, 0, sysbus); /* call level one configuration */
	(*sysbus->confl2)(-1, 0, sysbus); /* call level two configuration */
 
	/* 
	 * Start timers for  ecc and power-supply errors
	 */
	timeout (kn03_crdenable, (caddr_t) 0, kn03crdintvl * hz);
	timeout (kn03_pscheck, (caddr_t) 0, kn03_psintvl * hz);

	/*
	 * Probe for NVRAM, and configure it, if it is present.
	 */
	 kn03_config_nvram();

	s = splnone(); /* TODO redundent?  done upon return to configure */
	return (0);
}


kn03_iointr(ep)
u_int *ep;
{
	register u_int sir;
	register u_int current_sirm = *(u_int *)PHYS_TO_K1(KN03_SIRM_ADDR);
	
	
	sir = *(u_int *)PHYS_TO_K1(KN03_SIR_ADDR);
	
	/* mask out bits that are disabled thru SIRM register */
	sir &= current_sirm;
	
	if (sir & SLU_INTR) {
	    (*(tc_slot[KN03_SCC_INDEX].intr))();
	} else if (sir & LANCE_INTR) {
	    (*(tc_slot[KN03_LN_INDEX].intr))();
	} else if (sir & SCSI_INTR) {
	    (*(tc_slot[KN03_SCSI_INDEX].intr))();
	} else if (sir & TC_SLOT_2_INTR) {
	    (*(tc_slot[2].intr))(tc_slot[2].param);
	} else if (sir & TC_SLOT_1_INTR) {
	    (*(tc_slot[1].intr))(tc_slot[1].param);
	} else if (sir & TC_SLOT_0_INTR) {
	    (*(tc_slot[0].intr))(tc_slot[0].param);
	} else {
	    incr_interrupt_counter_type(INTR_TYPE_STRAY);
	    printf("Stray interrupt from System Interrupt Register");

	}
}


kn03_enable_option(index)
    register int index;
{
        register int i;
	
	switch (index) {
	  case KN03_SCC_INDEX:
	    kn03_sim_ie |= (SLU_INTR);
	    break;
	    
	  case KN03_LN_INDEX:
	    kn03_sim_ie |= (LANCE_INTR);
	    break;
	    
	  case KN03_SCSI_INDEX:
	    kn03_sim_ie |= (SCSI_INTR);
	    break;
	    
	  case TC_OPTION_SLOT_0:
	    kn03_sim_ie |= (TC_SLOT_0_INTR);
	    break;
	    
	  case TC_OPTION_SLOT_1:
	    kn03_sim_ie |= (TC_SLOT_1_INTR);
	    break;
	    
	  case TC_OPTION_SLOT_2:
	    kn03_sim_ie |= (TC_SLOT_2_INTR);
	    break;
	    
	  default:
	    printf("Enable_option call to non existent slot");
	    break;
	}

	*(u_int *)PHYS_TO_K1(KN03_SIRM_ADDR) = kn03_sim_ie;
	
}

kn03_disable_option(index)
    register int index;
{
        register int i;

	switch (index) {
	  case KN03_SCC_INDEX:
	    kn03_sim_ie &= ~(SLU_INTR);
	    break;
	    
	  case KN03_LN_INDEX:
	    kn03_sim_ie &= ~(LANCE_INTR);
	    break;
	    
	  case KN03_SCSI_INDEX:
	    kn03_sim_ie &= ~(SCSI_INTR);
	    break;
	    
	  case TC_OPTION_SLOT_0:
	    kn03_sim_ie &= ~(TC_SLOT_0_INTR);
	    break;
	    
	  case TC_OPTION_SLOT_1:
	    kn03_sim_ie &= ~(TC_SLOT_1_INTR);
	    break;
	    
	  case TC_OPTION_SLOT_2:
	    kn03_sim_ie &= ~(TC_SLOT_2_INTR);
	    break;
	    
	  default:
	    printf("disable_option call to non existent slot");
	    break;
	}
	
	*(u_int *)PHYS_TO_K1(KN03_SIRM_ADDR) = kn03_sim_ie;
}

/*
 * Routine to handle trap errors: user-mode ibe & dbe, & all kernel mode traps.
 * We try to recover from user-mode errors and panic on kernel mode errors.
 *
 * Entry conditions:
 *	kn03erradr and kn03chksyn are set in locore on bus errors
 *	(VEC_dbe and VEC_ibe) to be a copy of the hardware registers.
 */
kn03_trap_error(ep, code, sr, cause, signo)
	register u_int *ep;		/* exception frame ptr */
	u_int code;			/* trap code (trap type) */
	u_int sr, cause;		/* status and cause regs */
	int *signo;			/* set if we want to kill process */
{
	caddr_t pa;			/* the physical addr of the error */	
	register struct proc *p;	/* ptr to current proc struct */
	register u_int kn03csr;		/* copy of csr reg */
	register u_int erradr;		/* copy of erradr reg */
	register u_int chksyn;		/* copy of chksyn reg */
	register u_int chksyn_plus;	/* chksyn + pc valid bit & err count */
	int errtype;			/* local record of error type */
	int module;			/* module number with error */
	unsigned ssr;			/* System Support reg */
	unsigned sir;			/* System Inter reg */
	unsigned sirm;			/* System Inter Mask reg */
	int kn03_nvram_end=0;		/* Calculate ending address*/
	/*
	 * Snapshot System registers
	 */
	ssr = *(u_int *)(PHYS_TO_K1(KN03_SSR_ADDR));
	sir = *(u_int *)(PHYS_TO_K1(KN03_SIR_ADDR));
	sirm = *(u_int *)(PHYS_TO_K1(KN03_SIRM_ADDR));
	kn03csr = *(u_int *)PHYS_TO_K1(KN03CSR_ADDR);

	/*
	 * These registers caught and set in locore
	 */
	erradr = kn03erradr;
	chksyn = kn03chksyn;

	p = u.u_procp;

	if ((erradr & ERR_VALID) && ((erradr & ERR_TYPE) == ERR_RECC) &&
	    (((chksyn & CHKSYN_VLDLO) && ((chksyn & CHKSYN_SNGLO) == 0))||
	     ((chksyn & CHKSYN_VLDHI) && ((chksyn & CHKSYN_SNGHI) == 0))))
		errtype = ERR_RECC;	/* Multibit memory read ECC */
	else if ((erradr & ERR_VALID) && ((erradr & ERR_TYPE) == ERR_RTMO))
		errtype = ERR_RTMO;	/* CPU read timeout */
	else errtype = ERR_UKN;
	/*
	 * If we have NVRAM, need to adjust valid phys address ranges
	 * for errors, will need to know ending address for checking
	 * ECC errors, use CSR as last valid location.
	 */
	if (kn03_nvram_found)
	    kn03_nvram_end = kn03_nvram_start_addr + KN03_NVRAM_CSR_OFFSET;



	if (USERMODE(sr)) {
		switch (errtype) {
		case ERR_RECC:
			pa = vatophys(ep[EF_BADVADDR]);

			if ( ((int)pa != -1 && (btop((int)pa) < physmem)) || (((int)pa != -1) && (kn03_nvram_start_addr <= (int)pa <= kn03_nvram_end)) )

			    if (kn03csr & KN03CSR_BNK32M)
				    module = (int)pa / (32*(1024*1024));
			    else    module = (int)pa / ( 8*(1024*1024));
			else
				module = -1;

			if (module >= 0 && module < MEM_MODULES) {
				kn03memerrs[module]++;
				if (kn03memerrs[module] > MAXERRCNT) {
					kn03memerrs[module] = 0;
					printf("Error count on memory module %d reached %d, resetting count to zero.\n", module, MAXERRCNT);
				}
			} else
				module = -1;
			erradr = (erradr & (~ERR_COLADDR)) | (((int)erradr -5) & ERR_COLADDR);
			chksyn_plus = 0;
			if (chksyn & CHKSYN_VLDLO) {
				chksyn_plus = (chksyn & CPLUS_CHK);
			} else
				chksyn_plus = ((chksyn >> CPLUS_MOFF) & CPLUS_CHK);
			if (module == -1)
				chksyn_plus |= (0 << CPLUS_EOFF)
					| (0 < CPLUS_MOFF) | CPLUS_VALID;
			else
				chksyn_plus |= (kn03memerrs[module] << CPLUS_EOFF)
					| (module < CPLUS_MOFF) | CPLUS_VALID;
#ifdef nosharedpageinfo /* OHMS */
			if (SHAREDPG(pa)) {
#endif nosharedpageinfo 
				kn03logmempkt(EL_PRISEVERE, ep, ELMETYP_RDS, kn03csr, erradr, chksyn_plus);
				kn03_consprint(MEMPKT, ep, kn03csr, erradr, chksyn_plus, ssr, sir, sirm); 
				kn03eccpanic = 1;
				if (module == -1)
					panic("multibit ECC error reported on non-existent memory module");
				else 
					panic("multibit memory ECC error in shared page");
#ifdef nosharedpageinfo /* OHMS */
			} else {
				kn03logmempkt(EL_PRISEVERE, ep, ELMETYP_RDS, kn03csr, erradr, chksyn_plus);
				printf("pid %d (%s) was killed on multibit memory ECC error\n",
					p->p_pid, u.u_comm);
				uprintf("pid %d (%s) was killed on multibit memory ECC error\n",
					p->p_pid, u.u_comm);
			}
#endif nosharedpageinfo
			break;
			
		case ERR_RTMO:
			uprintf("pid %d (%s) was killed on CPU read bus timeout\n",
				p->p_pid, u.u_comm);
			
			break;
		case ERR_UKN:
		default:
			uprintf("pid %d (%s) was killed on unknown bus error\n",
				p->p_pid, u.u_comm);
			break;
		}
	} else {
		/*
		 * Kernel mode errors.
		 * They all panic, its just a matter of what we log
		 * and what panic message we issue.
		 */
		switch (code) {

		case EXC_DBE:
			/*
			 * If we are already dumping from an ECC error
			 * and we get a data bus CPU read ECC error, then
			 * just ignore this error.
			 */
			if ((errtype == ERR_RECC) && kn03eccpanic) {
				return(0);
			}
			/* fall thru */
		case EXC_IBE:
			switch (errtype) {
			case ERR_RECC:	/* Multibit memory read ECC */
				pa = vatophys(ep[EF_BADVADDR]);

				if ( ((int)pa != -1 && (btop((int)pa) < physmem)) || (((int)pa != -1) && (kn03_nvram_start_addr <= (int)pa <= kn03_nvram_end)) ) {
				
				    if (kn03csr & KN03CSR_BNK32M)
					module = (int)pa / (32*(1024*1024));
				    else    module = (int)pa / ( 8*(1024*1024));

				} else
					module = -1;

				if (module >= 0 && module < MEM_MODULES) {
					kn03memerrs[module]++;
					if (kn03memerrs[module] > MAXERRCNT) {
						kn03memerrs[module] = 0;
						printf("Error count on memory module %d reached %d, resetting count to zero.\n", module, MAXERRCNT);
					}
				} else
					module = -1;
				erradr = (erradr & (~ERR_COLADDR)) | (((int)erradr -5) & ERR_COLADDR);
				chksyn_plus = 0;
				if (chksyn & CHKSYN_VLDLO) {
					chksyn_plus = (chksyn & CPLUS_CHK);
				} else
					chksyn_plus = ((chksyn >> CPLUS_MOFF) & CPLUS_CHK);
				if (module == -1)
					chksyn_plus |= (0 << CPLUS_EOFF)
						| (0 < CPLUS_MOFF) | CPLUS_VALID;
				else
					chksyn_plus |= (kn03memerrs[module] << CPLUS_EOFF)
						| (module < CPLUS_MOFF) | CPLUS_VALID;
				kn03logmempkt(EL_PRISEVERE, ep, ELMETYP_RDS, kn03csr, erradr, chksyn_plus);
				kn03_consprint(MEMPKT, ep, kn03csr, erradr, chksyn_plus, ssr, sir, sirm); 
				kn03eccpanic = 1;
				if (module == -1)
					panic("multibit ECC error reported on non-existent memory module");
				else 
					panic("multibit memory ECC error");
				break;
				
			case ERR_RTMO:
				kn03logesrpkt(EL_PRISEVERE, ep, ssr, sir, sirm, kn03csr, erradr);
				kn03_consprint(ESR_BUS_PKT, ep, kn03csr, erradr, 0, ssr, sir, sirm);
				panic("CPU read bus timeout");
				break;
			case ERR_UKN:
			default:
				kn03logesrpkt(EL_PRISEVERE, ep, ssr, sir, sirm, kn03csr, erradr);
				kn03_consprint(ESR_BUS_PKT, ep, kn03csr, erradr, 0, ssr, sir, sirm);
				panic("Unknown bus timeout");
				break;
			}
			break;
		case EXC_CPU:
			kn03logesrpkt(EL_PRISEVERE, ep, ssr, sir, sirm, kn03csr, 0);
			kn03_consprint(ESR_BUS_PKT, ep, kn03csr, 0, 0, ssr, sir, sirm);
			panic("coprocessor unusable");
			break;
		case EXC_RADE:
		case EXC_WADE:
			kn03logesrpkt(EL_PRISEVERE, ep, ssr, sir, sirm, kn03csr, 0);
			kn03_consprint(ESR_BUS_PKT, ep, kn03csr, 0, 0, ssr, sir, sirm);
			panic("unaligned access");
			break;
		default:
			kn03logesrpkt(EL_PRISEVERE, ep, ssr, sir, sirm, kn03csr, 0);
			kn03_consprint(ESR_BUS_PKT, ep, kn03csr, 0, 0, ssr, sir, sirm);
			panic("trap");
			break;
		}
	}
	/*
	 * Default user-mode action is to terminate the process
	 */
	*signo = SIGBUS;
	return(0);
}

/*
 * Error interrupt.
 * With buffered writes, these are not reported synchronously, thus
 * there is no process context to terminate a user process, so we panic.
 *
 * Causes:
 *   Memory Errors:
 *	CPU read single bit ECC
 *	DMA read overrun, DMA write overrun
 *	CPU partial memory write ECC, DMA memory read ECC
 *   Timeout Errors:
 *	CPU I/O write timeout
 */

kn03_errintr(ep)
	u_int *ep;			/* exception frame ptr */
{
	register int s;
	register u_int kn03csr;		/* copy of csr reg */
	register u_int erradr;		/* copy of erradr reg */
	register u_int chksyn;		/* copy of chksyn reg */
	register u_int chksyn_plus;	/* valid half of chksyn+errcnt+pc */
	int errtype;			/* local record of error type */
	int pa;				/* the physical address of the error */
	int module;			/* module number with error */
	long currtime;			/* current time value */
        struct kn03consinfo_t *pcons;  /* pointer to console info */
        struct kn03log_errinfo_t *plog; /* pointer to log info */
	unsigned ssr;
	unsigned sir;
	unsigned sirm;

	/*
	 * Snapshot system registers
	 */
	ssr = *(u_int *)(PHYS_TO_K1(KN03_SSR_ADDR));
	sir = *(u_int *)(PHYS_TO_K1(KN03_SIR_ADDR));
	sirm = *(u_int *)(PHYS_TO_K1(KN03_SIRM_ADDR));
	kn03csr = *(u_int *)PHYS_TO_K1(KN03CSR_ADDR);
	erradr = *(u_int *)PHYS_TO_K1(KN03ERR_ADDR);
	chksyn = *(u_int *)PHYS_TO_K1(KN03CHKSYN_ADDR);

	/*
	 * Decode Error Type
	 */
	if ((erradr & ERR_VALID) && ((erradr & ERR_TYPE) == ERR_RECC) &&
	    (((chksyn & CHKSYN_VLDLO) && (chksyn & CHKSYN_SNGLO)) ||
	     ((chksyn & CHKSYN_VLDHI) && (chksyn & CHKSYN_SNGHI))))
		errtype = ERR_RECC;	/* singlebit memory read ECC */
	else if ((erradr & ERR_VALID) && ((erradr & ERR_TYPE) == ERR_DMARECC) &&
	    ((chksyn & CHKSYN_VLDLO) || (chksyn & CHKSYN_VLDHI)))
		errtype = ERR_DMARECC;	/* DMA memory read ECC */
	else if ((erradr & ERR_VALID) && ((erradr & ERR_TYPE) == ERR_WECC) &&
	    ((chksyn & CHKSYN_VLDLO) || (chksyn & CHKSYN_VLDHI)))
		errtype = ERR_WECC;	/* CPU partial mem write ECC */
	else if ((erradr & ERR_VALID) && ((erradr & ERR_TYPE) == ERR_DMAROVR))
		errtype = ERR_DMAROVR;	/* DMA read overrun */
	else if ((erradr & ERR_VALID) && ((erradr & ERR_TYPE) == ERR_DMAWOVR))
		errtype = ERR_DMAWOVR;	/* DMA write overrun */
	else if ((erradr & ERR_VALID) && ((erradr & ERR_TYPE) == ERR_WTMO))
		errtype = ERR_WTMO;	/* CPU write timeout */
	else errtype = ERR_UKN;

	/*
	 * Handle according to error type
	 */
	switch (errtype) {
	case ERR_RECC:
	case ERR_DMARECC:
	case ERR_WECC:
		erradr = (erradr & (~ERR_COLADDR)) | (((int)erradr -5) & ERR_COLADDR);
		pa = (erradr & ERR_ADDR) << 2;
		if (kn03csr & KN03CSR_BNK32M)
			module = pa / (32*(1024*1024));
		else	module = pa / (8*(1024*1024));
		if (module >= 0 && module < MEM_MODULES) {
			kn03memerrs[module]++;
			if (kn03memerrs[module] > MAXERRCNT) {
				kn03memerrs[module] = 0;
				printf("Error count on memory module %d reached %d, resetting count to zero.\n", module, MAXERRCNT);
			}
		} else
			module = -1;
		/*
		 * Build chksyn_plus.
		 */
		chksyn_plus = 0;
		if (chksyn & CHKSYN_VLDLO) {
			chksyn_plus = (chksyn & CPLUS_CHK);
		} else
			chksyn_plus = ((chksyn >> CPLUS_MOFF) & CPLUS_CHK);
		if (module == -1) {
			chksyn_plus |= (0 << CPLUS_EOFF);
			chksyn_plus |= (0 << CPLUS_MOFF);
		} else {
			chksyn_plus |= (kn03memerrs[module] << CPLUS_EOFF);
			chksyn_plus |= (module << CPLUS_MOFF);
		}
		chksyn_plus &= ~CPLUS_VALID;
		if (((chksyn & CHKSYN_VLDLO) && (chksyn & CHKSYN_SNGLO)) ||
	            ((chksyn & CHKSYN_VLDHI) && (chksyn & CHKSYN_SNGHI))) {
			/*
			 * Single bit ECC error (CRD)
			 * If we get 3 or more in 1 second then disable logging
			 * them for 15 minutes.  The variable "kn03stopcrdlog"
			 * is cleared by the kn03crdenable routine.
			 */
			if (kn03crdlog) {
				currtime = time.tv_sec;
				if (currtime == kn03_crdcnt.crd_prev) {
					kn03crdlog = 0;
					printf("High rate of corrected single-bit ECC errors, logging disabled for 15 minutes\n");
					kn03_crdcnt.crd_last = 0;
					currtime = 0;
				}
				kn03logmempkt(EL_PRIHIGH, ep, ELMETYP_CRD, kn03csr, erradr, chksyn_plus);
				kn03_crdcnt.crd_prev = kn03_crdcnt.crd_last;
				kn03_crdcnt.crd_last = currtime;
			}
			/*
			 * Scrub the single bit error
			 */
			*(u_int *)PHYS_TO_K0(pa) = *(u_int *)PHYS_TO_K0(pa);
			*(u_int *)PHYS_TO_K1(KN03ERR_ADDR) = 0;
			wbflush();
		} else {
			/*
			 * Multibit error, panic.
			 */
			kn03logmempkt(EL_PRISEVERE, ep, ELMETYP_RDS, kn03csr, erradr, chksyn_plus);
			kn03_consprint(MEMPKT, ep, kn03csr, erradr, chksyn_plus, ssr, sir, sirm);
			*(u_int *)PHYS_TO_K1(KN03ERR_ADDR) = 0;
			kn03eccpanic = 1;
			wbflush();
			panic("multibit memory ECC error");
		}
		break;
	case ERR_DMAWOVR:
	case ERR_DMAROVR:
		kn03logesrpkt(EL_PRISEVERE, ep, ssr,sir,sirm, kn03csr,erradr);
		kn03_consprint(ESR_INTR_PKT, ep, kn03csr, erradr, 0, ssr, sir, sirm);
		*(u_int *)PHYS_TO_K1(KN03ERR_ADDR) = 0;
		wbflush();
		panic("DMA overrun");
		break;
	case ERR_WTMO:
                kn03logesrpkt(EL_PRISEVERE, ep, ssr,sir,sirm, kn03csr,erradr);
	        kn03_consprint(ESR_INTR_PKT, ep, kn03csr, erradr, 0, ssr, sir, sirm);
		*(u_int *)PHYS_TO_K1(KN03ERR_ADDR) = 0;
		wbflush();
		panic("CPU write timeout");
		break;
	case ERR_UKN:
	default:
		kn03logesrpkt(EL_PRISEVERE, ep, ssr,sir,sirm, kn03csr, erradr);
		kn03_consprint(ESR_BUS_PKT, ep, kn03csr, erradr, 0, ssr, sir, sirm);
		printf("\tChecksyn register\t= 0x%x\n", chksyn);
		*(u_int *)PHYS_TO_K1(KN03ERR_ADDR) = 0;
		wbflush();
		panic("Unknown memory error interrupt");
		break;
	}
	return(0);
}


/*
 * Log a memory error packet, so uerf can find it as a kn03 main memory error.
 */
kn03logmempkt(priority, ep, type, kn03_mem_cs, erradr, chksyn_plus)
	int priority;		/* pkt priority: panic: severe; else: high */
	register u_int *ep;	/* exception frame ptr */
	int type;		/* error type: RDS, CRD, DMAOVR */
	u_int kn03_mem_cs;	/* copy of kn03csr to log */
	u_int erradr;		/* copy of erradr to log */
	u_int chksyn_plus;	/* chksyn + error count + pc valid bit */
{

	struct el_rec *elrp;
	register struct el_mem *mrp;

	elrp = ealloc(EL_MEMSIZE, priority);
	if (elrp != NULL) {
		LSUBID(elrp,ELCT_MEM,EL_UNDEF,ELMCNTR_kn02,EL_UNDEF,EL_UNDEF,EL_UNDEF);
		mrp = &elrp->el_body.elmem;
		mrp->elmem_cnt = 1;
		mrp->elmemerr.cntl = 1;
		mrp->elmemerr.type = type;
		mrp->elmemerr.numerr = 1;
		mrp->elmemerr.regs[0] = ep[EF_EPC];
		mrp->elmemerr.regs[1] = kn03_mem_cs;
		mrp->elmemerr.regs[2] = erradr;
		mrp->elmemerr.regs[3] = chksyn_plus;
		EVALID(elrp);
	}
	log(LOG_ERR, "kn03 error: Cause 0x%x PC 0x%x Status 0x%x Bad VA 0x%x\n",
	    ep[EF_CAUSE], ep[EF_EPC], ep[EF_SR], ep[EF_BADVADDR]);
	log(LOG_ERR, "Sp 0x%x kn03 csr 0x%x Error adr 0x%x\n",
	    ep[EF_SP], kn03_mem_cs, erradr);
	
}



#define KN03_LOG_ESRPKT(elrp, cause,epc,sr,badva,sp,ssr,sir,sirm,csr,erradr) \
	elrp->el_body.elesr.elesr.el_esrkn03.esr_cause = cause;	\
	elrp->el_body.elesr.elesr.el_esrkn03.esr_epc = epc;		\
	elrp->el_body.elesr.elesr.el_esrkn03.esr_status = sr;		\
	elrp->el_body.elesr.elesr.el_esrkn03.esr_badva = badva;	\
	elrp->el_body.elesr.elesr.el_esrkn03.esr_sp = sp;		\
	elrp->el_body.elesr.elesr.el_esrkn03.esr_ssr = ssr;		\
	elrp->el_body.elesr.elesr.el_esrkn03.esr_sir = sir;		\
	elrp->el_body.elesr.elesr.el_esrkn03.esr_sirm = sirm;		\
	elrp->el_body.elesr.elesr.el_esrkn03.esr_cs = csr;		\
	elrp->el_body.elesr.elesr.el_esrkn03.esr_erradr = erradr;	

/*
 * Log Error & Status Registers to the error log buffer
 */
kn03logesrpkt(priority, ep, ssr, sir, sirm, csr, erradr)
	int priority;		/* for pkt priority */
	register u_int *ep;	/* exception frame ptr */
	u_int ssr;
	u_int sir;
	u_int sirm;
        u_int csr;
        u_int erradr;
{
	struct el_rec *elrp;
	
	elrp = ealloc(sizeof(struct el_esr), priority);
	if (elrp != NULL) {
	    LSUBID(elrp,ELCT_MCK,ELESR_KN03,EL_UNDEF,EL_UNDEF,EL_UNDEF,
		   EL_UNDEF);
	    KN03_LOG_ESRPKT(elrp, ep[EF_CAUSE], ep[EF_EPC], ep[EF_SR], 
			    ep[EF_BADVADDR], ep[EF_SP], ssr, sir, sirm, csr, erradr);
	    
	    EVALID(elrp);
	}
	log(LOG_ERR, "kn03 error: Cause 0x%x PC 0x%x Status 0x%x Bad VA 0x%x\n",
	    ep[EF_CAUSE], ep[EF_EPC], ep[EF_SR], ep[EF_BADVADDR]);
	log(LOG_ERR, "Sp 0x%x kn03 csr 0x%x Error adr 0x%x\n",
	    ep[EF_SP],csr, erradr);

}


/*
 * 	Logs error information to the error log buffer.
 *	Exported through the cpu switch.
 */

kn03_log_errinfo(esrp)
struct kn03log_errinfo_t *esrp;
{
	struct el_rec *elrp;
	
	switch (esrp->pkt_type) {
	    
	  case ESR_INTR_PKT:
	    elrp = ealloc(sizeof(struct el_esr), EL_PRISEVERE);
	    if (elrp != NULL) {
		LSUBID(elrp,ELCT_MCK,ELESR_KN03,EL_UNDEF,EL_UNDEF,EL_UNDEF
		       ,EL_UNDEF);
		KN03_LOG_ESRPKT(elrp, esrp->cause, esrp->epc, esrp->sr, 
				esrp->badvaddr, esrp->sp, esrp->ssr, 
				esrp->sir, esrp->sirm, esrp->csr, 
				esrp->erradr);
		EVALID(elrp);
	    }
	    break;
	    
	  default: 
	    printf("bad pkt type\n");
	    return;
	}
	log(LOG_ERR, "kn03 error: Cause 0x%x PC 0x%x Status 0x%x Bad VA 0x%x\n",
	    esrp->cause, esrp->epc, esrp->sr, esrp->badvaddr);
	log(LOG_ERR, "Sp 0x%x kn03csr 0x%x erradr 0x%x\n",
	    esrp->sp, esrp->csr, esrp->erradr);

}


/*
 * Check Power Supply over-heat Warning.
 * If its overheating, warn to shut down the system.
 * If its gone from overheat to OK, cancel the warning.
 */
kn03_pscheck()
{
	register u_int sir;		/* a copy of the real csr */
	
	sir = *(u_int *)PHYS_TO_K1(KN03_SIR_ADDR);
	
	if (sir & PSWARN) {
	    printf("System Overheating - suggest shutdown and power-off\n");
	    kn03_pswarn = 1;
	} else {
	    if (kn03_pswarn) {
		printf("System OK - cancel overheat shutdown\n");
		kn03_pswarn = 0;
	    }
	}
	timeout (kn03_pscheck, (caddr_t) 0, kn03_psintvl * hz);
}


/*
 * Enable CRD (single bit ECC) error logging
 */
kn03_crdenable()
{
	kn03crdlog = 1;
	timeout (kn03_crdenable, (caddr_t) 0, kn03crdintvl * hz);
}


/*
 * Print error packet to the console.
 * This is only done when we are about to panic on the error.
 * It calls kn03_print_consinfo to actually print the information.
 *
 */
kn03_consprint(pkt, ep, kn03csr, erradr, chksyn_plus, ssr, sir, sirm)
	int pkt;		/* error pkt: Error & Stat Regs / memory pkt */
	register u_int *ep;	/* exception frame ptr */
	u_int kn03csr;		/* kn03csr to print */
	u_int erradr;		/* kn03 erradr to print */
	u_int chksyn_plus;	/* chksyn + error count + pc valid bit */
	unsigned ssr;
	unsigned sir;
	unsigned sirm;
{
	register int i;
	struct kn03consinfo_t p;
	

	p.pkt_type = pkt;
	switch (pkt) {
	case ESR_INTR_PKT:
		p.pkt.intrp.cause	= ep[EF_CAUSE];
		p.pkt.intrp.sr		= ep[EF_SR];
		p.pkt.intrp.sp		= ep[EF_SP];
		p.pkt.intrp.csr		= kn03csr;
		p.pkt.intrp.erradr	= erradr;
		p.pkt.intrp.ssr		= ssr;
		p.pkt.intrp.sir		= sir;
		p.pkt.intrp.sirm	= sirm;
		break;

	case ESR_BUS_PKT:
		p.pkt.busp.cause	= ep[EF_CAUSE];
		p.pkt.busp.epc		= ep[EF_EPC];
		p.pkt.busp.sr		= ep[EF_SR];
		p.pkt.busp.badvaddr	= ep[EF_BADVADDR];
		p.pkt.busp.sp		= ep[EF_SP];
		p.pkt.busp.csr		= kn03csr;
		p.pkt.busp.erradr	= erradr;
		p.pkt.busp.ssr		= ssr;
		p.pkt.busp.sir		= sir;
		p.pkt.busp.sirm       	= sirm;
		break;

	case MEMPKT:
		if (chksyn_plus & 0x80000000)
		    p.pkt.memp.epc	= ep[EF_EPC];

		p.pkt.memp.csr		= kn03csr;
		p.pkt.memp.erradr	= erradr;
		p.pkt.memp.chksyn_plus	= chksyn_plus;
		break;

	default:
		printf("bad consprint\n");
		return;
	}
	kn03_print_consinfo(&p);
	
}


/*
 *	This routine is similar to kn03consprint().
 *	This is exported through the cpusw structure. 
 *	
 */

kn03_print_consinfo(p)
struct kn03consinfo_t *p;
{
	/*
	 * If console is a graphics device,
	 * force printf messages directly to screen.
	 */
	printstate |= PANICPRINT;

	switch (p->pkt_type) {
	case ESR_INTR_PKT:
		printf("\nException condition\n");
		printf("\tCause reg\t\t\t= 0x%x\n", p->pkt.intrp.cause);
		printf("\tException PC\t\t\t= invalid for this error\n");
		printf("\tStatus reg\t\t\t= 0x%x\n", p->pkt.intrp.sr);
		printf("\tBad virt addr\t\t\t= invalid for this error\n");
		printf("\tStack ptr\t\t\t= 0x%x\n", p->pkt.intrp.sp);
		printf("\tSystem CSR\t\t\t= 0x%x\n", p->pkt.intrp.csr);

		if ( p->pkt.busp.erradr) 
		    {
			printf("\tError Address reg\t\t\t= 0x%x\n", 
				p->pkt.intrp.erradr);
			printf("\t  Phys addr\t\t\t= 0x%x\n", 
				(p->pkt.intrp.erradr&ERR_ADDR) << 2);
		    }
		printf("\tSystem Support reg\t\t = 0x%x\n", p->pkt.intrp.ssr);
		printf("\tSystem Interrupt reg\t\t = 0x%x\n", p->pkt.intrp.sir);
		printf("\tSystem Interrupt Mask reg\t = 0x%x\n", 
			p->pkt.intrp.sirm);
		break;

	case ESR_BUS_PKT:
		printf("\nException condition\n");
		printf("\tCause reg\t\t\t= 0x%x\n", p->pkt.busp.cause);
		printf("\tException PC\t\t\t= 0x%x\n", p->pkt.busp.epc);
		printf("\tStatus reg\t\t\t= 0x%x\n", p->pkt.busp.sr);
		printf("\tBad virt addr\t\t\t= 0x%x\n", p->pkt.busp.badvaddr);
		printf("\tStack ptr\t\t\t= 0x%x\n", p->pkt.busp.sp);
		printf("\tSystem CSR\t\t\t= 0x%x\n", p->pkt.busp.csr);

		if ( p->pkt.busp.erradr) 
		    {
			printf("\tError Address reg\t\t= 0x%x\n", 
				p->pkt.busp.erradr);
			printf("\t  Phys addr\t\t\t= 0x%x\n", 
				(p->pkt.busp.erradr&ERR_ADDR) << 2);
		    }
		printf("\tSystem Support reg\t\t = 0x%x\n", p->pkt.busp.ssr);
		printf("\tSystem Interrupt reg\t\t = 0x%x\n", p->pkt.busp.sir);
		printf("\tSystem Interrupt Mask reg\t = 0x%x\n", 
			p->pkt.busp.sirm);
		break;

	case MEMPKT:
		printf("\nMemory Error\n");
		if (p->pkt.memp.chksyn_plus & 0x80000000)
			printf("\tException PC\t\t= 0x%x\n", 
				p->pkt.memp.chksyn_plus);
		else
		    printf("\tException PC\t\t= invalid for this error\n");
		printf("\tSystem CSR\t\t= 0x%x\n", p->pkt.memp.csr);
		printf("\tError Address reg\t= 0x%x\n", p->pkt.memp.erradr);
		printf("\t\t\t  Phys addr\t= 0x%x\n", 
			(p->pkt.memp.erradr&ERR_ADDR) << 2);
		printf("\nError count for module %d = %d\n",
		    ((p->pkt.memp.chksyn_plus & CPLUS_MMASK) >> CPLUS_MOFF),
		    ((p->pkt.memp.chksyn_plus & CPLUS_EMASK) >> CPLUS_EOFF));
		break;

	default:
		printf("bad print_consinfo \n");
		break;
	}
}

/*
 * kn03 delay routine.
 *
 *  delay multiplier assigned in kn03_init, and is sized according
 *  to processor speed, which is determined by conf_clk_speed().
 *
 */
kn03_delay(n)
	int n;
{
	register int N = kn03_delay_mult*(n); 
	while (--N > 0); 
	return(0);
}

/*
 * kn03_stray handler,
 *
 *  should stray interrupts occur, log the interrupt info.
 *		      
 */
int kn03_stray_cnt=0;

kn03_stray(ep)
u_int *ep;
{

    printf("kn03: No interrupt pending\n");
    printf("\nPC:\t0x%x\nSP:\t0x%x\nEP:\t0x%x\n\n", ep[EF_EPC], ep[EF_SP], ep);
   
}


/*
 * kn03 halt handler
 *
 *  Push-button halt results in a high halt interrupt
 *  signal while the button is depressed, so we spin
 *  on the cause bit before servicing.
 * 
 */
kn03_halt(ep)
u_int *ep;
{

    /*
     * Spin on cause - waiting for button to be released
     */
    while (get_cause() & KN03_HALT) ;

    /*
     * For Debugging: print out value of PC, SP, and EP with labels 
     */
    rex_printf("\nPC:\t0x%x\nSP:\t0x%x\nEP:\t0x%x\n\n",
	       ep[EF_EPC], ep[EF_SP], ep);
    /*
     * Call PROM to execute system halt
     */
     rex_halt(0,0);

}

/*
 *	Clears any pending errors in the error register
 */
kn03_clear_errors()
{
        *(u_int *)PHYS_TO_K1(KN03ERR_ADDR) = 0;
	wbflush();
}


/*
 * Error isolation routine
 *
 * TURBOchannel devices getting DMA errors need cpu-specific info,
 * this routine passes error information which can be used to
 * better handle error conditions within the I/O drivers.
 *
 */
kn03_isolate_memerr(memerr_status)
	struct tc_memerr_status *memerr_status;
{

/* psgfix - This is a dummy routine, and we need to add ECC support
            to this new facility */

    return (-1);

}

kn03_conf_clk_speed()
{
    register volatile struct rt_clock *rt =(struct rt_clock *)rt_clock_addr;
    register volatile int dummy;
    register int s, counter = 0;
    int save_rega, save_regb;

    /*
     * Block everything.  Changed from splextreme() in bsd pool. 
     */

    s = splfpu();

    /* enable periodic interrupt */
    save_rega = rt->rt_rega;
    save_regb = rt->rt_regb;
    rt->rt_rega = RTA_DV32K|RTA_4ms;
    rt->rt_regb = RTB_DMBINARY|RTB_24HR|RTB_PIE;

    /* clear any old interrupts */
    dummy = rt->rt_regc;

    /* wait for start */
    while ((get_cause() & SR_IBIT4) == 0);

    dummy = rt->rt_regc;

    /* wait for finish and count */
    while ((get_cause() & SR_IBIT4) == 0)
	counter++;

    dummy = rt->rt_regc;
    rt->rt_rega = save_rega;
    rt->rt_regb = save_regb;

    splx(s);
    return (counter);
}


/*
 *
 * kn03_config_nvram
 *
 *    - initialize pointers to cpu specific routines for Prestoserve
 *    - Check for nvram presence
 *    - initialize status and control register location according to 
 *      nvram location
 *    - make call to presto_init to initialize Prestoserve 
 *
 *
 */
int kn03_cache_nvram = 1;
int kn03_nvram_mapped = 0;
unsigned kn03_nvcsr = 0x1;
unsigned kn03_nvdiag = 0x13;

kn03_config_nvram()
{
  	extern kn03_nvram_status();
	extern kn03_nvram_battery_status();
	extern kn03_nvram_battery_enable();
	extern kn03_nvram_battery_disable();
	extern u_int kn03_machineid();

	extern void bcopy();
	extern void bzero();

	extern int memlimit;
	unsigned kn03_nvram_start_addr;
	unsigned int kn03_nvram_location;
	unsigned kn03_nvram_size;

	unsigned kn03_nvram_physaddr;
	int kn03_nvram_found = 0;
	unsigned nvram_id = 0;
	short i = 0;

	static unsigned addr[] = { 0x800000,   0x1000000,  0x1800000, 
				   0x2000000,  0x2800000,  0x3000000,
				   0x3800000,  0x4000000,  0x4800000, 
				   0x5000000,  0x5800000,  0x6000000, 
				   0x6800000,  0x7000000,  0x8000000,
				   0xa000000,  0xc000000,  0xe000000,  
				   0x10000000, 0x12000000, 0x14000000, 
				   0x16000000, 0x18000000, 
				   0x1a000000, 0x1c000000 }; 

	/*
	 * For testing, using main memory instead of NVRAM
	 * Use memlimit to stop memory at 32Mb, memlimit=0x2000000
	 * Set up necessary registers, to simulate NVRAM interface
	 *
	 */

	if (kn03_test)
	  {
	   if (memlimit) 
	    {
       	     printf("kn03 NVRAM debug: Simmulating NVRAM in manin memory\n");
	     printf("kn03 NVRAM debug: Starting address = 0x%x\n",memlimit);

	     *(int *)PHYS_TO_K1((memlimit + 0x3f8)) = kn03_nvdiag; /* diag */
	     *(int *)PHYS_TO_K1((memlimit + 0x3fc)) = 0x03021966;  /* id */
	     *(int *)PHYS_TO_K1((memlimit + 0x400000)) = kn03_nvcsr; /* csr */

	    }
	  }

	/* 
	 * Initialize the structure that will point to the cpu specific
	 * functions for Prestoserve
	 */
	presto_interface0.nvram_status = kn03_nvram_status;
	presto_interface0.nvram_battery_status= kn03_nvram_battery_status;
	presto_interface0.nvram_battery_disable= kn03_nvram_battery_disable;
	presto_interface0.nvram_battery_enable= kn03_nvram_battery_enable;

	/*
	 * presto_interface modified to allow for "programming" of
	 * Presto transfers
	 */
	presto_interface0.nvram_ioreg_read = bcopy ;
	presto_interface0.nvram_ioreg_write = bcopy ;
	presto_interface0.nvram_block_read = bcopy ;
	presto_interface0.nvram_block_write = bcopy ;
	presto_interface0.nvram_ioreg_zero = bzero ;
	presto_interface0.nvram_block_zero = bzero ;

	presto_interface0.nvram_min_ioreg = sizeof(int) ;
	presto_interface0.nvram_ioreg_align = sizeof(int) ;
	presto_interface0.nvram_min_block = sizeof(int) ;
	presto_interface0.nvram_block_align = sizeof(int) ;

	/* 
	 * configure the NVRAM option if it is present
	 *
	 * Manually probe the possible locations where nvram address
	 * space can begin, if there is memory there, then look for
	 * the known signature at the id location.
	 *
	 */
	for ( i=0 ; i <= 24 ; i+=1)
	    {
		if ( bbadaddr(PHYS_TO_K1(addr[i]+KN03_NVRAM_ID),4, 0) &&
		     bbadaddr(PHYS_TO_K1(addr[i]+KN03_NVRAM_ID),4, 0))
		    {
		     Dprintf("kn03: No memory board present at 0x%x, i=%d\n",
			       PHYS_TO_K1(addr[i]+KN03_NVRAM_ID),i);
		    }
		else
		    {
			nvram_id = *(int *)PHYS_TO_K1(addr[i]+KN03_NVRAM_ID);

			Dprintf("%d: ID from memory = 0x%x\n",i, nvram_id);

			if ( nvram_id == KN03_NVRAM_IDENTIFIER)
			    {
				kn03_nvram_found = 1;
				Dprintf("kn03: Found NVRAM board\n");
				kn03_nvram_physaddr = addr[i];
				break;
			    }
			else Dprintf("kn03: Not an NVRAM board\n");

		    }
	    }


	if (kn03_nvram_found)

	  {

	    kn03_nvram_location = PHYS_TO_K1(kn03_nvram_physaddr) ;
	    kn03_nvram_start_addr = kn03_nvram_physaddr + KN03_NVRAM_START ;
 
	    kn03_nvram_diag = PHYS_TO_K1(kn03_nvram_location+KN03_NVRAM_DIAG);

	    kn03_nvram_csr = kn03_nvram_location + 0x400000; 

	    kn03_nvram_size = (*(int *)kn03_nvram_diag & NVRAM_SIZE) >> 4;
	    kn03_nvram_size *= 0x100000;  /* convert to bytes */
	    kn03_nvram_size -= 0x400; 	  /* - reserved space */

	    Dprintf("NVRAM: Starting address = 0x%x\n", kn03_nvram_start_addr);
	    Dprintf("NVRAM: Diag reg address = 0x%x\n", kn03_nvram_diag);
	    Dprintf("NVRAM: Diag reg contents = 0x%x\n",
		    *(int *)PHYS_TO_K1(kn03_nvram_diag));

	    presto_init(kn03_nvram_start_addr, kn03_nvram_size, 
			kn03_nvram_mapped, kn03_cache_nvram,
		        kn03_machineid());
	  }
}

/*
 *
 * 	kn03_nvram_status
 *
 *     	provide presto with status of diagnostics run on nvram
 *    	 hence, let presto know what to do - recover, etc...
 *
 *	RETURN value:  -1 if no status set, otherwise pr.h defined value
 *
 */

kn03_nvram_status()
{
	if ( *(int *)kn03_nvram_diag & KN03_NVRAM_FAILED)
		return(NVRAM_BAD);
	else if ( *(int *)kn03_nvram_diag & KN03_NVRAM_RO)
		return(NVRAM_RDONLY);
	else if ( *(int *)kn03_nvram_diag & KN03_NVRAM_RW)
		return(NVRAM_RDWR);
	else {
		printf("kn03_nvram_status: No nvram diag bits set for status");
		return(-1);
	}
}

/*
 * 	kn03_nvram_battery_status
 *
 *     	update the global battery information structure for Prestoserve
 *     
 *	RETURN value:  0, if batteries are ok;  1, if problem
 */

kn03_nvram_battery_status()
{

	nvram_batteries0.nv_nbatteries = 1;	   /* one battery */
	nvram_batteries0.nv_minimum_ok = 1;	   /* it must be good */
	nvram_batteries0.nv_primary_mandatory = 1; /* primary must be OK */
	nvram_batteries0.nv_test_retries = 3;	   /* call this routine 3 times
						    * for each "test" */

	/* for 3max simulation - always return true, enable zeros BOK */

	if ( (*(int *)kn03_nvram_csr & KN03_NVRAM_BOK) || (kn03_test) )
		{
			nvram_batteries0.nv_status[0] = BATT_OK;
			return(0);
		}
	else
	    return(1);
	  
}

/*
 *	 kn03_nvram_battery_enable
 *
 *  	- arms the battery 
 *  	- required action is to zero BDISC control bit, this disables the 
 *    	  battery disconect circuit.
 *
 *	RETURN value:  1, if batteries successfully enabled, 0 if not  
 */
kn03_nvram_battery_enable()
{
	
	*(int *)kn03_nvram_csr = 0x0;
	wbflush();

	if ( (*(int *)kn03_nvram_csr & KN03_NVRAM_BDISC))
		return(1);
	else
		return(0);
}

/*
 * 	kn03_nvram_battery_disable
 *
 *	- unarms battery
 *	- required action is to send sequence "11001" to control register
 *	- this enables the battery disconnect circuit
 *	- The excessive wbflushes are used to protect against merged writes
 *
 *	RETURN value:  0, if batteries successfully disabled, 1 if not  
 */

kn03_nvram_battery_disable()
{

/*psgfix - may be able to do dummy writes inbetween  writes, and 1 wbflush*/

	*(int *)kn03_nvram_csr = 0x1;
	wbflush();
	
	*(int *)kn03_nvram_csr = 0x1;
	wbflush();
	
	*(int *)kn03_nvram_csr = 0x0;
	wbflush();
	
	*(int *)kn03_nvram_csr = 0x0;
	wbflush();

	*(int *)kn03_nvram_csr = 0x1;
	wbflush();
	
	
	if ( *(int *)kn03_nvram_csr & KN03_NVRAM_BDISC )
		return(0);
	else
		return(1);
	
}


/*
 * Get the hardware E_net address for on-board ln0 and use it
 * to build a poor man's version of a unique processor ID.
 */
u_int
kn03_machineid()
{
	register struct ifnet *ifp = ifnet;
	struct ifdevea ifr;
	u_int i = 0;
	int error;

	if (ifnet == NULL) {
		printf("kn220: ifnet NULL\n");
		return (i);
	}

	while (ifp != NULL) {
		if (ifp->if_name[0] == 'l' && ifp->if_name[1] == 'n' &&
		    ifp->if_unit == 0) {	/* found ln0 */
			error = (*ifp->if_ioctl)(ifp, SIOCRPHYSADDR, &ifr);
			if (error)
				return (i);
			i = (u_int)ifr.default_pa[2];
			i = i << 8;
			i += (u_int)ifr.default_pa[3];
			i = i << 8;
			i += (u_int)ifr.default_pa[4];
			i = i << 8;
			i += (u_int)ifr.default_pa[5];
			return (i);
		}
		ifp = ifp->if_next;
	}
	return (i);
}


